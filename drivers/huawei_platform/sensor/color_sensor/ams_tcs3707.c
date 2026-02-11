/*
 * ams_tcs3707.c
 *
 * code for AMS tcs3707 sensor
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#include "ams_tcs3707.h"

#define HWLOG_TAG color_sensor
HWLOG_REGIST();

static bool fd_gain_adjusted;
static bool fd_timer_stopped;
static bool color_calibrate_result = true;
static bool report_calibrate_result;
static u16 ring_buffer[MAX_BUFFER_SIZE] = {0};
static u32 read_out_buffer[MAX_BUFFER_SIZE] = {0};
static u16 *buffer_ptr = &ring_buffer[0];
static u16 *head_ptr = &ring_buffer[0];
static u16 *tail_ptr = &ring_buffer[0];
static u8 first_circle_end;
static DEFINE_MUTEX(ring_buffer_mutex);
static DEFINE_MUTEX(enable_handle_mutex);
static int report_value[AMS_REPORT_DATA_LEN] = {0};
static struct at_color_sensor_cali_nv_t color_nv_para;
static struct color_chip *p_chip;
static int read_nv_first_in;
static int enable_status_before_cali;
static UINT8 report_logcount;
static UINT32 algo_support;

extern int color_report_val[MAX_REPORT_LEN];
extern int (*color_default_enable)(bool enable);
extern UINT32 flicker_support;

int tcs3707_port_get_byte(struct i2c_client *handle, uint8_t reg, uint8_t *data,
	uint8_t len)
{
	int ret;

	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EPERM;
	}
	ret = i2c_smbus_read_i2c_block_data(handle, reg, len, data);
	if (ret < 0)
		hwlog_err("%s: failed\n", __func__);

	return ret;
}

int tcs3707_port_set_byte(struct i2c_client *handle, uint8_t reg, uint8_t *data,
	uint8_t len)
{
	int ret;

	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EPERM;
	}
	ret = i2c_smbus_write_i2c_block_data(handle, reg, len, data);

	if (ret < 0)
		hwlog_err("%s: failed\n", __func__);

	return ret;
}

static int tcs3707_get_byte(struct i2c_client *handle, UINT8 reg,
	UINT8 *read_data)
{
	int read_count;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	read_count = tcs3707_port_get_byte(handle, reg, read_data, ONE_BYTE);
	return read_count;
}

static int tcs3707_set_byte(struct i2c_client *handle, UINT8 reg, UINT8 data)
{
	int ret;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = tcs3707_port_set_byte(handle, reg, &data, ONE_BYTE);
	return ret;
}

static int tcs3707_get_buf(struct i2c_client *handle, UINT8 reg,
	UINT8 *read_data, UINT8 length)
{
	int read_count;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	read_count = tcs3707_port_get_byte(handle, reg, read_data, length);
	return read_count;
}

static int tcs3707_get_field(struct i2c_client *handle, UINT8 reg, UINT8 *data,
	UINT8 mask)
{
	int ret;

	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = tcs3707_port_get_byte(handle, reg, data, ONE_BYTE);
	*data &= mask;
	return ret;
}

static int tcs3707_set_field(struct i2c_client *handle, UINT8 reg, UINT8 data,
	UINT8 mask)
{
	int write_count = 0;
	UINT8 original_data = 0;
	UINT8 new_data;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	(void)tcs3707_get_byte(handle, reg, &original_data);

	new_data = original_data & (~mask);
	new_data |= (data & mask);

	if (new_data != original_data)
		write_count = tcs3707_set_byte(handle, reg, new_data);

	return write_count;
}

static int tcs3707_rgb_report_type(void)
{
	return AWB_SENSOR_RAW_SEQ_TYPE_C_R_G_B_W;
}

static int tcs3707_report_data(int value[])
{
	return ap_color_report(value, AMS_REPORT_DATA_LEN * sizeof(int));
}

static int tcs3707_set_enable(struct i2c_client *handle, UINT8 reg,
	UINT8 data, UINT8 mask)
{
	int ret = 0;
	UINT8 ori_data = 0;
	UINT8 new_data;

	mutex_lock(&enable_handle_mutex);
	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		mutex_unlock(&enable_handle_mutex);
		return -EFAULT;
	}
	(void)tcs3707_get_byte(handle, TCS3707_ENABLE_REG, &ori_data);
	new_data = ori_data & (~mask);
	new_data |= (data & mask);
	if (new_data != ori_data)
		ret = tcs3707_set_byte(handle, TCS3707_ENABLE_REG, new_data);

	mutex_unlock(&enable_handle_mutex);
	return ret;
}

static enum tcs3707_sensor_id_t tcs3707_get_chipid(struct i2c_client *handle)
{
	UINT8 chipid = 0;
	UINT8 i;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return AMS_UNKNOWN_DEVICE;
	}
	tcs3707_get_byte(handle, TCS3707_ID_REG, &chipid);

	for (i = 0; i < ARRAY_SIZE(tcs3707_ids); i++)
		if (tcs3707_ids[i] == chipid)
			return AMS_TCS3707_REV0;

	return AMS_UNKNOWN_DEVICE;
}

static void tcs3707_get_revid(struct i2c_client *handle)
{
	UINT8 revid = AMS_UNKNOWN_REV;

	if (algo_support == AMS_UNSUPPORT_ALGO)
		return;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	tcs3707_get_byte(handle, TCS3707_REVID_REG, &revid);
	hwlog_info("%s get revid = 0x%x\n", __func__, revid);
	revid = revid & AMS_3408_3707_DIST;
	if (revid == AMS_TCS3707_REVID)
		algo_support = AMS_TCS3707_NOR_ALGO;
	else if (revid == AMS_TCS3408_REVID)
		algo_support = AMS_TCS3408_NOR_ALGO;
}

static void tcs3707_reset_regs(struct i2c_client *handle)
{
	UINT8 i;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(tcs3707_settings); i++)
		tcs3707_set_byte(handle, tcs3707_settings[i].reg,
			tcs3707_settings[i].value);
	hwlog_warn("%s\n", __func__);
}

static UINT8 tcs3707_gain_to_reg(UINT32 x)
{
	UINT8 i;

	if ((ARRAY_SIZE(tcs3707_als_gain_conv) - 1) < 0)
		return 0;

	for (i = ARRAY_SIZE(tcs3707_als_gain_conv) - 1; i != 0; i--)
			if (x >= tcs3707_als_gain_conv[i])
				break;

	return i;
}

static INT32 tcs3707_get_gain(const void *ctx)
{
	UINT8 cfg1_reg = 0;
	INT32 gain = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	tcs3707_get_field(((struct tcs3707_ctx *)ctx)->handle,
		TCS3707_CFG1_REG, &cfg1_reg, AGAIN_MASK);

	if (cfg1_reg < ARRAY_SIZE(tcs3707_als_gain))
		gain = tcs3707_als_gain[cfg1_reg];

	hwlog_info("now the gain val = %d\n", gain);
	return gain;
}

static INT32 tcs3707_set_als_gain(void *ctx, int gain)
{
	INT32 ret;
	UINT8 cfg1 = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	switch (gain) {
	case ALS_GAIN_VALUE_1:
		cfg1 = AGAIN_0_5X;
		break;
	case ALS_GAIN_VALUE_2 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_1X;
		break;
	case ALS_GAIN_VALUE_3 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_2X;
		break;
	case ALS_GAIN_VALUE_4 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_4X;
		break;
	case ALS_GAIN_VALUE_5 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_8X;
		break;
	case ALS_GAIN_VALUE_6 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_16X;
		break;
	case ALS_GAIN_VALUE_7 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_32X;
		break;
	case ALS_GAIN_VALUE_8 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_64X;
		break;
	case ALS_GAIN_VALUE_9 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_128X;
		break;
	case ALS_GAIN_VALUE_10 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_256X;
		break;
	case ALS_GAIN_VALUE_11 * TCS3707_GAIN_SCALE:
		cfg1 = AGAIN_512X;
		break;
	default:
		break;
	}

	ret = tcs3707_set_field(((struct tcs3707_ctx *)ctx)->handle,
		TCS3707_CFG1_REG, cfg1, AGAIN_MASK);
	((struct tcs3707_ctx *)ctx)->alg_ctx.gain = (uint32_t)gain;

	hwlog_warn("%s gain = %d, cfg1 = %d\n", __func__, gain, cfg1);
	return ret;
}

static INT32 tcs3707_set_itime(struct tcs3707_ctx *ctx, int itime_ms)
{
	INT32 ret;
	INT32 itime_us;
	INT32 atime;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	itime_us = itime_ms * MS_2_US;
	atime = ((itime_us + DEFAULT_ASTEP / 2) / DEFAULT_ASTEP) - 1;
	if (atime < 0)
		atime = 0;
	ret = tcs3707_set_byte(ctx->handle, TCS3707_ATIME_REG, (UINT8)atime);

	return ret;
}

static INT32 tcs3707_set_fd_gain(struct tcs3707_ctx *ctx, UINT32 gain)
{
	UINT8 cfg1 = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	switch (gain) {
	case ALS_GAIN_VALUE_1:
		cfg1 = FD_GAIN_0_5X;
		break;
	case ALS_GAIN_VALUE_2:
		cfg1 = FD_GAIN_1X;
		break;
	case ALS_GAIN_VALUE_3:
		cfg1 = FD_GAIN_2X;
		break;
	case ALS_GAIN_VALUE_4:
		cfg1 = FD_GAIN_4X;
		break;
	case ALS_GAIN_VALUE_5:
		cfg1 = FD_GAIN_8X;
		break;
	case ALS_GAIN_VALUE_6:
		cfg1 = FD_GAIN_16X;
		break;
	case ALS_GAIN_VALUE_7:
		cfg1 = FD_GAIN_32X;
		break;
	case ALS_GAIN_VALUE_8:
		cfg1 = FD_GAIN_64X;
		break;
	case ALS_GAIN_VALUE_9:
		cfg1 = FD_GAIN_128X;
		break;
	case ALS_GAIN_VALUE_10:
		cfg1 = FD_GAIN_256X;
		break;
	case ALS_GAIN_VALUE_11:
		cfg1 = FD_GAIN_512X;
		break;
	default:
		break;
	}
	// close FDEN
	tcs3707_set_enable(ctx->handle, TCS3707_ENABLE_REG, 0, FDEN);
	tcs3707_set_field(ctx->handle, TCS3707_CTRL_REG, CLEAR_FIFO, CLEAR_FIFO);
	// open FDEN
	tcs3707_set_field(ctx->handle, TCS3707_FD_CFG3, cfg1, FD_GAIN_MASK);
	tcs3707_set_enable(ctx->handle, TCS3707_ENABLE_REG, FDEN, FDEN);

	ctx->flc_ctx.fd_gain = gain;
	hwlog_info("%s fd_gain = %d, cfg1 = 0x%02x\n", __func__, gain, cfg1);
	return 0;
}

static void tcs3707_als_reg_update(struct tcs3707_ctx *ctx, bool cal_mode)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	// For first integration after AEN
	ctx->alg_ctx.first_inte = false;
	if (!cal_mode) {
		ctx->alg_ctx.gain = TCS3707_AGAIN_FOR_FIRST_DATA;
		ctx->alg_ctx.itime_ms = TCS3707_ITIME_FOR_FIRST_DATA;
		ctx->alg_ctx.first_inte = true;
	}
	tcs3707_set_itime(ctx, ctx->alg_ctx.itime_ms);
	tcs3707_set_als_gain(ctx, ctx->alg_ctx.gain);
	hwlog_info("%s mode=%d, update gain=%d, Itime=%d, first_inte=%d\n",
		__func__, cal_mode, ctx->alg_ctx.gain,
		ctx->alg_ctx.itime_ms, ctx->alg_ctx.first_inte);
}

static void tcs3707_set_als_onoff(struct tcs3707_ctx *ctx, UINT8 data,
	bool cal_mode)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (data == 0) {
		tcs3707_set_enable(ctx->handle, TCS3707_ENABLE_REG, 0, AEN);
		tcs3707_set_byte(ctx->handle, TCS3707_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
	} else {
		tcs3707_als_reg_update(ctx, cal_mode);
		ctx->alg_ctx.als_update = 0;
		tcs3707_set_byte(ctx->handle, TCS3707_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
		tcs3707_set_enable(ctx->handle, TCS3707_ENABLE_REG, AEN, AEN);
	}
}

// als saturation check
static bool tcs3707_als_satu_check(struct tcs3707_ctx *ctx,
	struct tcs3707_adc_data_t *curr_raw)
{
	UINT16 astep_inte;
	UINT32 saturation;
	UINT8 atime = 0;
	UINT8 astep[2] = {0};

	if (!ctx || !curr_raw)
		return true;

	tcs3707_get_byte(ctx->handle, TCS3707_ATIME_REG, &atime);
	tcs3707_get_buf(ctx->handle, TCS3707_ASTEPL_REG, astep, DOUBLE_BYTE);
	astep_inte = COMB_WORD(astep[1], astep[0]); // get real astep_inte

	saturation = (atime + 1) * (astep_inte + 1); // calc saturation value
	if (saturation > MAX_SATURATION)
		saturation = MAX_SATURATION;

	// get saturation x 80%
	saturation = (saturation * SATURATION_CHECK_PCT) / 10;

	if ((curr_raw->c > saturation) || (curr_raw->w > saturation))
		return true;

	return false;
}

static bool tcs3707_als_insuff_check(struct tcs3707_ctx *ctx,
	struct tcs3707_adc_data_t *curr_raw)
{
	if (!curr_raw || !ctx) {
		hwlog_err("%s NULL poniter\n", __func__);
		return false;
	}
	if (curr_raw->c < AMS_TCS3707_LOW_LEVEL)
		return true;
	return false;
}

static bool tcs3707_fd_satu_check(struct tcs3707_ctx *ctx, UINT16 value)
{
	UINT8 low_fd_atime = 0;
	UINT8 high_fd_atime = 0;
	UINT16 fd_atime;
	UINT16 fd_time;
	UINT32 saturation;

	if (!ctx) {
		hwlog_err("%s NULL poniter\n", __func__);
		return true;
	}
	tcs3707_get_byte(ctx->handle, TCS3707_FD_CFG1, &low_fd_atime);
	tcs3707_get_field(ctx->handle, TCS3707_FD_CFG3, &high_fd_atime,
		FD_TIME_HIGH_3_BIT_MASK);
	fd_atime = COMB_WORD(high_fd_atime, low_fd_atime); // get fd_atime
	// step values is 2.78us. so astep value is 0.
	// set fd_time = (ATIME+1) * (ASTEP+1)
	fd_time = (fd_atime + 1) * (NORMAL_ASTEP + 1); // algo ratio cal for fd
	saturation = (fd_time * 8) / 10; // get real 0.8 saturation

	if (value >= saturation)
		return true;
	return false;
}

static bool tcs3707_fd_insuff_check(struct tcs3707_ctx *ctx, uint16_t value)
{
	if (value < TCS3707_FD_LOW_LEVEL)
		return true;

	return false;
}

static void tcs3707_reset_als_gain(struct tcs3707_ctx *ctx, UINT16 c_raw)
{
	if (c_raw <= GAIN_QUICKLY_FIX_LEVEL_1)
		// use 256 gain
		tcs3707_set_als_gain(ctx, (GAIN_LEVEL_5 * TCS3707_GAIN_SCALE));
	else if (c_raw <= GAIN_QUICKLY_FIX_LEVEL_2)
		// use 64 gain
		tcs3707_set_als_gain(ctx, (GAIN_LEVEL_4 * TCS3707_GAIN_SCALE));
	else if (c_raw <= GAIN_QUICKLY_FIX_LEVEL_3)
		// use 16 gain
		tcs3707_set_als_gain(ctx, (GAIN_LEVEL_3 * TCS3707_GAIN_SCALE));
	else if (c_raw <= GAIN_QUICKLY_FIX_LEVEL_4)
		// use 4 gain
		hwlog_info("use the init 4x gain\n");
	else
		// use 1 gain
		tcs3707_set_als_gain(ctx, (GAIN_LEVEL_1 * TCS3707_GAIN_SCALE));

}

static bool tcs3707_als_gain_adjust(struct tcs3707_ctx *ctx,
	struct tcs3707_adc_data_t *curr_raw)
{
	bool satu_check = false;
	bool insuff_check = false;
	bool re_enable = true;
	UINT32 curr_gain;
	UINT32 new_gain;

	curr_gain = ctx->alg_ctx.gain / TCS3707_GAIN_SCALE;
	satu_check = tcs3707_als_satu_check(ctx, curr_raw);
	insuff_check = tcs3707_als_insuff_check(ctx, curr_raw);

	if (satu_check && (curr_gain == GAIN_LEVEL_2)) {
		new_gain = GAIN_LEVEL_1;
	} else if ((satu_check && (curr_gain == GAIN_LEVEL_3)) ||
			(insuff_check && (curr_gain == GAIN_LEVEL_1))) {
		new_gain = GAIN_LEVEL_2;
	} else if ((satu_check && (curr_gain == GAIN_LEVEL_4)) ||
			(insuff_check && (curr_gain == GAIN_LEVEL_2))) {
		new_gain = GAIN_LEVEL_3;
	} else if ((satu_check && (curr_gain == GAIN_LEVEL_5)) ||
			(insuff_check && (curr_gain == GAIN_LEVEL_3))) {
		new_gain = GAIN_LEVEL_4;
	} else if (insuff_check && (ctx->alg_ctx.gain == GAIN_LEVEL_4)) {
		new_gain = GAIN_LEVEL_5;
	} else {
		re_enable = false;
		ctx->alg_ctx.als_update |= AMS_TCS3707_FEATURE_ALS;
	}
	if (re_enable)
		tcs3707_set_als_gain(ctx, (new_gain * TCS3707_GAIN_SCALE));

	return re_enable;
}

static bool tcs3707_handle_als(struct tcs3707_ctx *ctx, bool cal_mode)
{
	UINT8 adc_data[TCS3707_ADC_BYTES] = {0};
	bool re_enable = false;
	struct tcs3707_adc_data_t curr_raw;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	memset(&curr_raw, 0, sizeof(curr_raw));
	// read 10 raw ADC data
	tcs3707_get_buf(ctx->handle, TCS3707_ADATA0L_REG, adc_data, 10);
	curr_raw.c = COMB_WORD(adc_data[1], adc_data[0]);
	curr_raw.r = COMB_WORD(adc_data[3], adc_data[2]);
	curr_raw.g = COMB_WORD(adc_data[5], adc_data[4]);
	curr_raw.b = COMB_WORD(adc_data[7], adc_data[6]);
	curr_raw.w = COMB_WORD(adc_data[9], adc_data[8]);

	if (cal_mode) {
		ctx->alg_ctx.als_update |= AMS_TCS3707_FEATURE_ALS;
		goto handle_als_exit;
	}

	// use a short timer to quickly fix a proper gain for first data
	if (ctx->alg_ctx.first_inte) {
		hwlog_info("before gain re_set, ITIME = %d, AGAIN = %d\n",
			ctx->alg_ctx.itime_ms, ctx->alg_ctx.gain);

		tcs3707_reset_als_gain(ctx, curr_raw.c);
		ctx->alg_ctx.itime_ms = TCS3707_ITIME_DEFAULT;
		tcs3707_set_itime(ctx, ctx->alg_ctx.itime_ms);
		ctx->alg_ctx.first_inte = false;
		re_enable = true;
		goto handle_als_exit;
	}
	re_enable = tcs3707_als_gain_adjust(ctx, &curr_raw);

handle_als_exit:
	if (!re_enable) {
		ctx->alg_ctx.data.c = curr_raw.c;
		ctx->alg_ctx.data.r = curr_raw.r;
		ctx->alg_ctx.data.g = curr_raw.g;
		ctx->alg_ctx.data.b = curr_raw.b;
		ctx->alg_ctx.data.w = curr_raw.w;
	}
	return re_enable;
}

static bool tcs3707_handle_als_event(struct tcs3707_ctx *ctx, bool cal_mode)
{
	bool ret = false;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return false;
	}

	tcs3707_get_byte(ctx->handle, TCS3707_STATUS_REG, &ctx->alg_ctx.status);
	if (ctx->alg_ctx.status & AINT_MASK) {
		// only get available staus, then read data
		ret = tcs3707_handle_als(ctx, cal_mode);
		// clear the soft interrupt
		tcs3707_set_byte(ctx->handle, TCS3707_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
	}
	return ret;
}

static UINT8 tcs3707_get_als_update(struct tcs3707_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	return ctx->alg_ctx.als_update;
}

static void tcs3707_get_als_data(struct tcs3707_ctx *ctx,
	struct export_alsdata_t *export)
{
	if (!ctx || !export) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx->alg_ctx.als_update &= ~(AMS_TCS3707_FEATURE_ALS);

	export->raw_c = ctx->alg_ctx.data.c;
	export->raw_r = ctx->alg_ctx.data.r;
	export->raw_g = ctx->alg_ctx.data.g;
	export->raw_b = ctx->alg_ctx.data.b;
	export->raw_w = ctx->alg_ctx.data.w;
}

void tcs3707_als_timer_wrk(uintptr_t data)
{
	struct color_chip *chip = (struct color_chip *)data;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->als_work);
}

void tcs3707_flc_timer_wrk(uintptr_t data)
{
	struct color_chip *chip = (struct color_chip *)data;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->fd_work);
}

static UINT8 get_rgb_fd_enable_status(struct i2c_client *handle)
{
	UINT8 enable_sta = 0;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	tcs3707_port_get_byte(handle, TCS3707_ENABLE_REG, &enable_sta, 1);

	return enable_sta;
}

static ssize_t tcs3707_als_set_enable(struct color_chip *chip, uint8_t en)
{
	UINT8 rgb_enable_status;
	UINT32 i;

	if (!chip) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return 0;
	}
	hwlog_info("%s = %d\n", __func__, en);

	rgb_enable_status = get_rgb_fd_enable_status(chip->client);
	if (((rgb_enable_status & AEN) == AEN) && (en == 1)) {
		hwlog_warn("%s, rgb already enabled, return\n", __func__);
		return 0;
	}

	if (((rgb_enable_status & AEN) != AEN) && (en == 0)) {
		hwlog_warn("%s, rgb already disabled, return\n", __func__);
		return 0;
	}

	tcs3707_set_als_onoff(chip->device_ctx, en, chip->in_cal_mode);
	if (en == 0) {
		hwlog_info("%s: close rgb, del_timer\n", __func__);
		return 0;
	}

	// set default report val to -1 when enable
	color_report_val[0] = AMS_REPORT_DATA_LEN;
	for (i = 1; i < MAX_REPORT_LEN; i++)
		color_report_val[i] = -1;

	if (chip->in_cal_mode == false) {
		mod_timer(&chip->work_timer, jiffies + msecs_to_jiffies(12));
		hwlog_info("%s set 12 for a proper gain quickly\n", __func__);
		report_logcount = AMS_REPORT_LOG_COUNT_NUM;
	} else {
		mod_timer(&chip->work_timer,
			jiffies + msecs_to_jiffies(CALI_RGB_TIMER));
		hwlog_info("in calibrate mode timer set as 120ms\n");
	}
	return 0;
}
static void get_cal_para_from_nv(void)
{
	UINT32 i;
	int ret;

	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if (ret < 0) {
		hwlog_err("%s: fail,use default para\n", __func__);
		return;
	}

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		if (!color_nv_para.cal_c_ratio[i] ||
			!color_nv_para.cal_r_ratio[i] ||
			!color_nv_para.cal_g_ratio[i] ||
			!color_nv_para.cal_b_ratio[i] ||
			!color_nv_para.cal_w_ratio[i]) {
			color_nv_para.cal_c_ratio[i] = TCS_DEFAULT_CAL_RAT;
			color_nv_para.cal_r_ratio[i] = TCS_DEFAULT_CAL_RAT;
			color_nv_para.cal_g_ratio[i] = TCS_DEFAULT_CAL_RAT;
			color_nv_para.cal_b_ratio[i] = TCS_DEFAULT_CAL_RAT;
			color_nv_para.cal_w_ratio[i] = TCS_DEFAULT_CAL_RAT;
		}
		hwlog_info("%s gain[%d]: [%d, %d, %d, %d, %d]\n", __func__, i,
			color_nv_para.cal_c_ratio[i],
			color_nv_para.cal_r_ratio[i],
			color_nv_para.cal_g_ratio[i],
			color_nv_para.cal_b_ratio[i],
			color_nv_para.cal_w_ratio[i]);
	}
}

static int save_cal_para_to_nv(struct color_chip *chip)
{
	UINT32 i;
	int ret = 0;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}

	color_nv_para.nv_c = chip->cali_ctx.cal_c_tar;
	color_nv_para.nv_r = chip->cali_ctx.cal_r_tar;
	color_nv_para.nv_g = chip->cali_ctx.cal_g_tar;
	color_nv_para.nv_b = chip->cali_ctx.cal_b_tar;
	color_nv_para.nv_w = chip->cali_ctx.cal_w_tar;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		color_nv_para.cal_c_ratio[i] = chip->cali_ctx.cal_c_rst[i];
		color_nv_para.cal_r_ratio[i] = chip->cali_ctx.cal_r_rst[i];
		color_nv_para.cal_g_ratio[i] = chip->cali_ctx.cal_g_rst[i];
		color_nv_para.cal_b_ratio[i] = chip->cali_ctx.cal_b_rst[i];
		color_nv_para.cal_w_ratio[i] = chip->cali_ctx.cal_w_rst[i];
		hwlog_info("%s gain[%d]: [%d, %d, %d, %d,%d]\n", __func__, i,
			color_nv_para.cal_c_ratio[i],
			color_nv_para.cal_r_ratio[i],
			color_nv_para.cal_g_ratio[i],
			color_nv_para.cal_b_ratio[i],
			color_nv_para.cal_w_ratio[i]);
	}

	ret = write_color_calibrate_data_to_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if (ret < 0)
		hwlog_err("%s: fail\n", __func__);
	return 1;
}

static int tcs3707_als_cali_rgbc(struct rgb_cali_tar_t *cali_val)
{
	uint32_t result;

	result = cali_val->rawdata / AMS_TCS3707_CAL_AVERAGE;

	// cant devide by zero, use the default calibration scaling factor
	if (result == 0) {
		// just return defalut ratio
		hwlog_err("%s, raw data = 0\n", __func__);
		return TCS3707_CAL_RATIO;
	}

	result = cali_val->target_val * (cali_val->again *
		AMS_TCS3707_FLOAT_TO_FIX / AMS_TCS3707_GAIN_OF_GOLDEN) / result;

	if ((result > cali_val->high_th) || (result < cali_val->low_th)) {
		hwlog_info("%s ratio is out bound[%d, %d]! result = %d\n",
			__func__, cali_val->low_th, cali_val->high_th, result);
		color_calibrate_result = false;
	}
	return result;
}

static void tcs3707_als_cali_all_rgbc(struct color_sensor_cali_t *cal_raw,
	enum color_sensor_cal_states cal_idx, struct rgb_cali_tar_t *cali_tar)
{
	cali_tar->high_th = tcs3707_gain_thr[cal_idx].high_thr;
	cali_tar->low_th = tcs3707_gain_thr[cal_idx].low_thr;

	// R cali
	hwlog_info("%s, cali R, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_r_raw, cal_raw->cal_r_tar);
	cali_tar->rawdata = cal_raw->cal_r_raw;
	cali_tar->target_val = cal_raw->cal_r_tar;
	cal_raw->cal_r_rst[cal_idx] = tcs3707_als_cali_rgbc(cali_tar);

	// G cali
	hwlog_info("%s, cali G, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_g_raw, cal_raw->cal_g_tar);
	cali_tar->rawdata = cal_raw->cal_g_raw;
	cali_tar->target_val = cal_raw->cal_g_tar;
	cal_raw->cal_g_rst[cal_idx] = tcs3707_als_cali_rgbc(cali_tar);

	// B cali
	hwlog_info("%s, cali B, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_b_raw, cal_raw->cal_b_tar);
	cali_tar->rawdata = cal_raw->cal_b_raw;
	cali_tar->target_val = cal_raw->cal_b_tar;
	cal_raw->cal_b_rst[cal_idx] = tcs3707_als_cali_rgbc(cali_tar);

	// C cali
	hwlog_info("%s, cali C, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_c_raw, cal_raw->cal_c_tar);
	cali_tar->rawdata = cal_raw->cal_c_raw;
	cali_tar->target_val = cal_raw->cal_c_tar;
	cal_raw->cal_c_rst[cal_idx] = tcs3707_als_cali_rgbc(cali_tar);

	// W cali
	hwlog_info("%s, cali W, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_w_raw, cal_raw->cal_w_tar);
	cali_tar->rawdata = cal_raw->cal_w_raw;
	cali_tar->target_val = cal_raw->cal_w_tar;
	cal_raw->cal_w_rst[cal_idx] = tcs3707_als_cali_rgbc(cali_tar);

	hwlog_warn("%s, result r = %d, g = %d, b = %d, c = %d, w = %d\n",
		__func__, cal_raw->cal_c_rst[cal_idx],
		cal_raw->cal_r_rst[cal_idx], cal_raw->cal_g_rst[cal_idx],
		cal_raw->cal_b_rst[cal_idx], cal_raw->cal_w_rst[cal_idx]);
}

static void tcs3707_cal_als(struct color_chip *chip)
{
	struct export_alsdata_t out;
	struct tcs3707_ctx *ctx = NULL;
	UINT32 curr_gain;
	struct rgb_cali_tar_t cali_temp;

	hwlog_info("%s, comes into cali step\n", __func__);
	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->device_ctx;
	curr_gain = (ctx->alg_ctx.gain / TCS3707_GAIN_SCALE);

	memset(&out, 0, sizeof(out));
	tcs3707_get_als_data(chip->device_ctx, &out);

	hwlog_info("%s: state = %d, count = %d, curr_gain = %d\n",
		__func__, chip->cali_ctx.cal_state,
		chip->cali_ctx.cal_raw_count, curr_gain);

	tcs3707_get_gain(ctx);

	if ((chip->cali_ctx.cal_state < CAL_STATE_GAIN_LAST) &&
		(chip->cali_ctx.cal_state >= 0)) {
		chip->cali_ctx.cal_raw_count++;
		chip->cali_ctx.cal_c_raw += out.raw_c;
		chip->cali_ctx.cal_r_raw += out.raw_r;
		chip->cali_ctx.cal_g_raw += out.raw_g;
		chip->cali_ctx.cal_b_raw += out.raw_b;
		chip->cali_ctx.cal_w_raw += out.raw_w;

		cali_temp.high_th =
			tcs3707_gain_thr[chip->cali_ctx.cal_state].high_thr;
		cali_temp.low_th =
			tcs3707_gain_thr[chip->cali_ctx.cal_state].low_thr;
		cali_temp.again = curr_gain;
		tcs3707_als_cali_all_rgbc(&chip->cali_ctx,
			chip->cali_ctx.cal_state, &cali_temp);

		chip->cali_ctx.cal_state++;
		chip->cali_ctx.cal_raw_count = 0;
		chip->cali_ctx.cal_c_raw = 0;
		chip->cali_ctx.cal_r_raw = 0;
		chip->cali_ctx.cal_g_raw = 0;
		chip->cali_ctx.cal_b_raw = 0;
		chip->cali_ctx.cal_w_raw = 0;
		if (chip->cali_ctx.cal_state < CAL_STATE_GAIN_LAST) {
			tcs3707_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
			ctx->alg_ctx.gain =
				tcs3707_als_gain_conv[chip->cali_ctx.cal_state];
			tcs3707_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
		}
	} else {
		if (color_calibrate_result == true) {
			save_cal_para_to_nv(chip);
			report_calibrate_result = true;
		} else {
			hwlog_err("color_calibrate_result fail\n");
			report_calibrate_result = false;
		}
		chip->in_cal_mode = false;
		tcs3707_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);

		if (enable_status_before_cali == 1)
			tcs3707_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
		else
			hwlog_info("color sensor disabled before calibrate\n");
		hwlog_info("%s done\n", __func__);
	}
}

static void tcs3707_report_als(struct color_chip *chip)
{
	struct export_alsdata_t out;
	UINT8 curr_gain_idx;
	UINT8 curr_gain_radio_idx;
	UINT32 curr_gain;
	UINT8 j;
	struct tcs3707_ctx *ctx = NULL;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = chip->device_ctx;

	memset(&out, 0, sizeof(out));
	tcs3707_get_als_data(chip->device_ctx, &out);
	curr_gain = ctx->alg_ctx.gain;

	if (curr_gain < tcs3707_als_gain_conv[CAL_STATE_GAIN_5])
		curr_gain_idx = tcs3707_gain_to_reg(curr_gain);
	else
		curr_gain_idx = CAL_STATE_GAIN_5;

	if (curr_gain < tcs3707_als_gain_conv[CAL_STATE_GAIN_3])
		curr_gain_radio_idx =
			tcs3707_gain_to_reg(tcs3707_als_gain_conv[CAL_STATE_GAIN_3]);
	else if (curr_gain < tcs3707_als_gain_conv[CAL_STATE_GAIN_5])
		curr_gain_radio_idx = tcs3707_gain_to_reg(curr_gain);
	else
		curr_gain_radio_idx = CAL_STATE_GAIN_5;

	// adjust the report data when the calibrate ratio is acceptable
	out.raw_c *= color_nv_para.cal_c_ratio[curr_gain_radio_idx];
	out.raw_c /= AMS_TCS3707_FLOAT_TO_FIX;

	out.raw_r *= color_nv_para.cal_r_ratio[curr_gain_radio_idx];
	out.raw_r /= AMS_TCS3707_FLOAT_TO_FIX;

	out.raw_g *= color_nv_para.cal_g_ratio[curr_gain_radio_idx];
	out.raw_g /= AMS_TCS3707_FLOAT_TO_FIX;

	out.raw_b *= color_nv_para.cal_b_ratio[curr_gain_radio_idx];
	out.raw_b /= AMS_TCS3707_FLOAT_TO_FIX;

	out.raw_w *= color_nv_para.cal_w_ratio[curr_gain_radio_idx];
	out.raw_w /= AMS_TCS3707_FLOAT_TO_FIX;

	report_value[0] = (int)out.raw_c;
	report_value[1] = (int)out.raw_r;
	report_value[2] = (int)out.raw_g;
	report_value[3] = (int)out.raw_b;
	report_value[4] = (int)out.raw_w;
	color_report_val[0] = AMS_REPORT_DATA_LEN;

	for (j = 0; j < 5; j++) {
		report_value[j] *= AMS_TCS3707_GAIN_OF_GOLDEN;
		report_value[j] /= tcs3707_als_gain_lvl[curr_gain_idx];
		color_report_val[j + 1] = report_value[j];
	}

	tcs3707_report_data(report_value);
	report_logcount++;
	if (report_logcount < AMS_REPORT_LOG_COUNT_NUM)
		return;
	hwlog_info("report data %d, %d, %d, %d, %d, currentGain[%d]\n",
		report_value[0], report_value[1], report_value[2],
		report_value[3], report_value[4],
		tcs3707_als_gain_lvl[curr_gain_idx]);
	hwlog_info("curr gain[%d], gain idx=[%d], gain_radio idx[%d]\n",
		tcs3707_als_gain_lvl[curr_gain_idx],
		curr_gain_idx, curr_gain_radio_idx);
	hwlog_info("cal ratio c = %d, r = %d, g = %d, b = %d, w = %d\n",
		color_nv_para.cal_c_ratio[curr_gain_radio_idx],
		color_nv_para.cal_r_ratio[curr_gain_radio_idx],
		color_nv_para.cal_g_ratio[curr_gain_radio_idx],
		color_nv_para.cal_b_ratio[curr_gain_radio_idx],
		color_nv_para.cal_w_ratio[curr_gain_radio_idx]);
	report_logcount = 0;
}

int tcs3707_enable_rgb(bool enable)
{
	struct color_chip *chip = p_chip;

	if (enable)
		tcs3707_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
	else
		tcs3707_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
	return 1;
}
EXPORT_SYMBOL_GPL(tcs3707_enable_rgb);

void tcs3707_show_calibrate(struct color_chip *chip,
	struct at_color_sensor_output_t *out)
{
	UINT32 i = 0;

	if (!out || !chip) {
		hwlog_err("%s input para NULL\n", __func__);
		return;
	}

	if (chip->in_cal_mode == false)
		hwlog_err("%s not in cali mode\n", __func__);

	out->result = (UINT32)report_calibrate_result;
	hwlog_info("%s result = %d\n", __func__, out->result);
	out->gain_arr = CAL_STATE_GAIN_LAST;
	out->color_arr = AMS_REPORT_DATA_LEN;
	memcpy(out->cali_gain, tcs3707_als_gain_lvl, sizeof(out->cali_gain));
	memcpy(out->cali_rst[0], chip->cali_ctx.cal_c_rst,
		sizeof(out->cali_rst[0]));
	memcpy(out->cali_rst[1], chip->cali_ctx.cal_r_rst,
		sizeof(out->cali_rst[1]));
	memcpy(out->cali_rst[2], chip->cali_ctx.cal_g_rst,
		sizeof(out->cali_rst[2]));
	memcpy(out->cali_rst[3], chip->cali_ctx.cal_b_rst,
		sizeof(out->cali_rst[3]));
	memcpy(out->cali_rst[4], chip->cali_ctx.cal_w_rst,
		sizeof(out->cali_rst[4]));

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++)
		hwlog_info("%s i = %d: cali= %d,%d,%d,%d,%d\n", __func__, i,
		out->cali_rst[0][i], out->cali_rst[1][i], out->cali_rst[2][i],
		out->cali_rst[3][i], out->cali_rst[4][i]);

	return;
}

void tcs3707_store_calibrate(struct color_chip *chip,
	struct at_color_sensor_input_t *in)
{
	struct tcs3707_ctx *ctx = NULL;
	struct tcs3707_cal_tar_t input_para;
	UINT8 rgb_enable_status;

	if (!chip || !chip->device_ctx || !in) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	AMS_MUTEX_LOCK(&chip->lock);
	report_calibrate_result = false;
	input_para.enable = in->enable;
	input_para.tar_c = in->reserverd[0];
	input_para.tar_r = in->reserverd[1];
	input_para.tar_g = in->reserverd[2];
	input_para.tar_b = in->reserverd[3];
	input_para.tar_w = in->reserverd[4];

	if (input_para.enable && chip->in_cal_mode) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		hwlog_err("%s already in cali mode\n", __func__);
		return;
	}

	if (input_para.enable) {
		ctx = chip->device_ctx;
		hwlog_info("%s start cali mode\n", __func__);
		chip->cali_ctx.cal_raw_count = 0;
		chip->cali_ctx.cal_c_raw = 0;
		chip->cali_ctx.cal_r_raw = 0;
		chip->cali_ctx.cal_g_raw = 0;
		chip->cali_ctx.cal_b_raw = 0;
		chip->cali_ctx.cal_w_raw = 0;
		chip->cali_ctx.cal_c_tar = input_para.tar_c;
		// calculate targer for gain 1x (assuming its set at 64x)
		chip->cali_ctx.cal_r_tar = input_para.tar_r;
		chip->cali_ctx.cal_g_tar = input_para.tar_g;
		chip->cali_ctx.cal_b_tar = input_para.tar_b;
		chip->cali_ctx.cal_w_tar = input_para.tar_w;
		chip->cali_ctx.cal_state = CAL_STATE_GAIN_1;
		hwlog_info("%s tar_c_r_g_b_w = %d, %d, %d, %d, %d\n", __func__,
			input_para.tar_c, input_para.tar_r, input_para.tar_g,
			input_para.tar_b, input_para.tar_w);

		ctx->alg_ctx.gain = tcs3707_als_gain_conv[CAL_STATE_GAIN_1];
		ctx->alg_ctx.itime_ms = TCS3707_ITIME_DEFAULT;
		rgb_enable_status = get_rgb_fd_enable_status(ctx->handle);
		if ((rgb_enable_status & AEN) == AEN) {
			// enabled before calibrate
			enable_status_before_cali = 1;
			hwlog_info("%s: enabled before calibrate\n", __func__);
			tcs3707_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
			msleep(10); // sleep 10 ms to make sure disable timer
		} else {
			// disabled before calibrate
			enable_status_before_cali = 0;
			hwlog_info("%s: disabled before calibrate\n", __func__);
		}
		chip->in_cal_mode = true;
		// make the calibrate_result true for calibrate again
		color_calibrate_result = true;
		tcs3707_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
	} else {
		hwlog_info("%s stopp calibrate mode\n", __func__);
		chip->in_cal_mode = false;
	}

	AMS_MUTEX_UNLOCK(&chip->lock);
}

void tcs3707_show_enable(struct color_chip *chip, int *state)
{
	UINT8 rgb_enable_status;

	if (!chip || !state) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	rgb_enable_status = get_rgb_fd_enable_status(chip->client);

	if (rgb_enable_status & AEN)
		*state = 1;
	else
		*state = 0;
}

void tcs3707_store_enable(struct color_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	if (state)
		tcs3707_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
	else
		tcs3707_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
}

static void fd_enable_set(struct color_chip *chip, UINT8 en)
{
	struct tcs3707_ctx *ctx = NULL;
	UINT8 fd_enable_status;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = (struct tcs3707_ctx *)chip->device_ctx;

	hwlog_info("%s = %d\n", __func__, en);
	fd_enable_status = get_rgb_fd_enable_status(ctx->handle);
	if (en) {
		if ((fd_enable_status & FDEN) != FDEN) {
			tcs3707_set_fd_gain(ctx, DEFAULT_FD_GAIN);
			ctx->flc_ctx.first_fd_inte = true;
			// first enable flicker timer
			mod_timer(&chip->fd_timer,
				jiffies + msecs_to_jiffies(FIRST_FLK_TIMER));
			fd_timer_stopped = false;
			hwlog_info("fd_enable 6ms for a gain quickly\n");
		} else {
			hwlog_warn("fd_enable fd already been enabled\n");
		}
	} else {
		if ((fd_enable_status & FDEN) == FDEN) {
			tcs3707_set_enable(ctx->handle, TCS3707_ENABLE_REG,
				0, FDEN);
			fd_timer_stopped = true;
			//clear ring_buffer when close the fd
			memset(ring_buffer, 0, MAX_BUFFER_SIZE * 2);
			buffer_ptr = &ring_buffer[0];
			head_ptr = &ring_buffer[0];
			tail_ptr = &ring_buffer[0];
			hwlog_info("fd_enable now disable fd sensor\n");
		} else {
			hwlog_warn("fd_enable has already been disabled\n");
		}
	}
}

void tcs3707_fd_show_enable(struct color_chip *chip, int *state)
{
	UINT8 fd_enable_status;

	if (!chip || !state) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	fd_enable_status = get_rgb_fd_enable_status(chip->client);
	if ((fd_enable_status & FDEN) == FDEN)
		*state = 1;
	else
		*state = 0;
}

void tcs3707_fd_store_enable(struct color_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	if (state)
		fd_enable_set(chip, AMSDRIVER_FD_ENABLE);
	else
		fd_enable_set(chip, AMSDRIVER_FD_DISABLE);
	hwlog_info("%s enable = %d success\n", __func__, state);
}

static void tcs3707_als_work(struct work_struct *work)
{
	bool re_enable = false;
	UINT8 rgb_data_status;
	UINT8 rgb_enable_status;
	struct tcs3707_ctx *ctx = NULL;
	struct color_chip *chip = NULL;

	if (!work) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct color_chip, als_work);
	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer chip is NULL\n", __func__);
		return;
	}
	ctx = (struct tcs3707_ctx *)chip->device_ctx;
	AMS_MUTEX_LOCK(&chip->lock);
	if (read_nv_first_in == 0) {
		get_cal_para_from_nv();
		read_nv_first_in = -1; // -1: do not read nv again
	}

	re_enable = tcs3707_handle_als_event(ctx, chip->in_cal_mode);

	rgb_data_status = tcs3707_get_als_update(ctx);
	if (rgb_data_status & AMS_TCS3707_FEATURE_ALS) {
		if (chip->in_cal_mode == false && !re_enable) {
			tcs3707_report_als(chip);
		} else {
			hwlog_info("%s cali mode\n", __func__);
			tcs3707_cal_als(chip);
		}
	}

	rgb_enable_status = get_rgb_fd_enable_status(ctx->handle);
	if ((rgb_enable_status & AEN) != AEN) {
		hwlog_warn("%s: rgb already disabled, del timer\n", __func__);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return;
	}

	if (chip->in_cal_mode == false) {
		if (re_enable) {
			mod_timer(&chip->work_timer,
				jiffies + msecs_to_jiffies(RE_ENABLE_RGB_TIMER));
			hwlog_info("crgb re_enable timer set 101ms\n");
		} else {
			// timer set as 100ms
			mod_timer(&chip->work_timer, jiffies + HZ / 10);
		}
	} else {
		// calibrate mode set timer for 120ms
		mod_timer(&chip->work_timer, jiffies + msecs_to_jiffies(120));
		hwlog_info("in calib mode mod timer set as 120ms\n");
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
}

void get_read_out_buffer(void)
{
	UINT32 i;
	uint32_t head_ptr_delta = 0;
	u16 *temp_ptr = &ring_buffer[0];
	u16 *temp_head_ptr = head_ptr;

	head_ptr_delta = (uint32_t)((uintptr_t)temp_head_ptr -
		(uintptr_t)(&ring_buffer[0]));
	head_ptr_delta = head_ptr_delta / 2;

	if (head_ptr_delta > MAX_BUFFER_SIZE - 1)
		head_ptr_delta = MAX_BUFFER_SIZE - 1;

	memset(read_out_buffer, 0, sizeof(read_out_buffer));

	for (i = 0; i < (MAX_BUFFER_SIZE - head_ptr_delta); i++) {
		read_out_buffer[i] = *temp_head_ptr;
		temp_head_ptr++;
	}

	for (i = (MAX_BUFFER_SIZE - head_ptr_delta); i < MAX_BUFFER_SIZE; i++) {
		read_out_buffer[i] = *temp_ptr;
		temp_ptr++;
	}

	for (i = 0; i < MAX_BUFFER_SIZE; i++)
		hwlog_debug("get out buffer read_out_buffer[%d] = %d\n",
			i, read_out_buffer[i]);
}

void ring_buffer_process(uint8_t fifo_lvl, uint16_t *buf_16)
{
	UINT32 i = 0;

	mutex_lock(&ring_buffer_mutex);
	for (i = 0; i < fifo_lvl; i++) {
		*buffer_ptr = buf_16[i];
		tail_ptr = buffer_ptr;
		buffer_ptr++;
		if (buffer_ptr == &ring_buffer[MAX_BUFFER_SIZE]) {
			buffer_ptr = &ring_buffer[0];
			first_circle_end = 1;
		}
	}
	if ((tail_ptr == &ring_buffer[MAX_BUFFER_SIZE - 1]) ||
		(first_circle_end == 0))
		head_ptr = &ring_buffer[0];

	if ((first_circle_end == 1) &&
		(tail_ptr != &ring_buffer[MAX_BUFFER_SIZE - 1]))
		head_ptr = (tail_ptr + 1);

	mutex_unlock(&ring_buffer_mutex);
}

static bool fd_auto_gain_control(struct tcs3707_ctx *ctx, uint16_t *buf_16_bit,
	int len)
{
	UINT8 j;
	UINT8 k;
	UINT16 buf_16_bit_temp;
	UINT16 buf_16_bit_max;
	bool fd_saturation_check = false;
	bool fd_insufficience_check = false;

	if (len < MAX_AUTOGAIN_CHECK_CNT)
		return false;

	fd_gain_adjusted = false;
	// sort the 10 buf[i] data, and find the max data
	for (j = 0; j < 9; j++) { // min 9 val
		for (k = j + 1; k < 10; k++) { // max 10 val
			if (buf_16_bit[j] > buf_16_bit[k]) {
				buf_16_bit_temp = buf_16_bit[j];
				buf_16_bit[j] = buf_16_bit[k];
				buf_16_bit[k] = buf_16_bit_temp;
			}
		}
	}
	buf_16_bit_max = buf_16_bit[9]; // get max bit
	// first flicker detect data is special
	if (ctx->flc_ctx.first_fd_inte) {
		if (buf_16_bit_max <= BUF_16_MAX_LEVEL5)
			//if min 9x gain auto lev5
			tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
		else if (buf_16_bit_max <= BUF_16_MAX_LEVEL4)
			//if min 18x gain auto lev4
			tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
		else if (buf_16_bit_max <= BUF_16_MAX_LEVEL3)
			//if min 36x gain auto lev3
			tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
		else if (buf_16_bit_max <= BUF_16_MAX_LEVEL2)
			hwlog_info("%s, keep the init 4x gain\n", __func__);
		else
			tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_1);

		hwlog_info("%s, come into first_fd_inte, gain = %d\n", __func__,
			ctx->flc_ctx.fd_gain);
		ctx->flc_ctx.first_fd_inte = false;
		fd_gain_adjusted = true;
		goto handle_fliker_exit;
	}

	fd_saturation_check = tcs3707_fd_satu_check(ctx, buf_16_bit_max);
	fd_insufficience_check = tcs3707_fd_insuff_check(ctx, buf_16_bit_max);

	// Adjust gain setting for flicker detect
	if (fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_2)) {
		tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_1);
		fd_gain_adjusted = true;
	} else if ((fd_saturation_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_3)) ||
		(fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_1))) {
		tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_2);
		fd_gain_adjusted = true;
	} else if ((fd_saturation_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_4)) ||
		(fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_2))) {
		tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
		fd_gain_adjusted = true;
	} else if ((fd_saturation_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_5)) ||
		(fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_3))) {
		tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
		fd_gain_adjusted = true;
	} else if (fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_4)) {
		tcs3707_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
		fd_gain_adjusted = true;
	}

handle_fliker_exit:
	return fd_gain_adjusted;
}

static void read_fd_data(struct color_chip *chip)
{
	UINT8 fifo_level = 0;
	UINT16 i;
	UINT8 buf[MAX_BUFFER_SIZE] = {0};
	UINT16 fd_report_value[REPORT_FIFO_LEN] = {0};
	UINT16 buf_16_bit[REPORT_FIFO_LEN] = {0};
	bool ret_auto_gain = false;
	UINT8 fd_enable_status;
	struct tcs3707_ctx *ctx = NULL;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s, pointer is null\n", __func__);
		return;
	}
	ctx = (struct tcs3707_ctx *)chip->device_ctx;
	fd_enable_status = get_rgb_fd_enable_status(ctx->handle);
	(void)tcs3707_get_byte(ctx->handle, TCS3707_FIFO_STATUS, &fifo_level);
	if (fd_timer_stopped == false)
		mod_timer(&chip->fd_timer, jiffies + msecs_to_jiffies(22));
	else
		hwlog_info("fd has been disabled, del fd work timer\n");

	// cal fd_ratio
	if ((fifo_level == 0) || (ctx->flc_ctx.fd_gain == 0))
		goto handle_fd_data;

	color_sensor_read_fifo(ctx->handle, TCS3707_FDADAL, buf,
		fifo_level * FIFO_LEVEL_RATIO);

	for (i = 0; i < fifo_level; i++)
		buf_16_bit[i] = COMB_WORD(buf[i * 2 + 1], buf[i * 2]);

	for (i = 0; i < fifo_level; i++)
		fd_report_value[i] = (buf_16_bit[i] *
			AMS_TCS3707_FD_GAIN_OF_GOLDEN) / ctx->flc_ctx.fd_gain;

handle_fd_data:
	ret_auto_gain = fd_auto_gain_control(ctx, buf_16_bit, REPORT_FIFO_LEN);
	if (ret_auto_gain == false) {
		// to forbid the case when the sensor is closed,
		// but still process the data
		if ((fd_enable_status & FDEN) == FDEN)
			ring_buffer_process(fifo_level, fd_report_value);
	} else if (ret_auto_gain == true) {
		// if fd_gain is adjusted
		// clear the ringbuffer and reset the related pointer.
		mutex_lock(&ring_buffer_mutex);
		memset(ring_buffer, 0, MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT);
		// clear ring_buffer when close the fd
		buffer_ptr = &ring_buffer[0];
		head_ptr = &ring_buffer[0];
		tail_ptr = &ring_buffer[0];
		mutex_unlock(&ring_buffer_mutex);
	}
}


static void tcs3707_fd_work(struct work_struct *work)
{
	struct color_chip *chip = NULL;

	if (!work) {
		hwlog_err("%s: Pointer work is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct color_chip, fd_work);
	if (!chip) {
		hwlog_err("%s: Pointer chip is NULL\n", __func__);
		return;
	}
	AMS_MUTEX_LOCK(&chip->lock);
	read_fd_data(chip);
	AMS_MUTEX_UNLOCK(&chip->lock);
}

static void tcs3707_get_fd_data(struct color_chip *chip, char *ams_fd_data)
{
	if (!chip || !ams_fd_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&ring_buffer_mutex);
	get_read_out_buffer();
	memcpy(ams_fd_data, read_out_buffer, sizeof(read_out_buffer));
	mutex_unlock(&ring_buffer_mutex);
}

static char *tcs3707_chip_name(void)
{
	return "ams_tcs3707";
}

static char *tcs3707_algo_type(void)
{
	if (algo_support == AMS_UNSUPPORT_ALGO) {
		hwlog_info("not support hal algo\n");
		return "unsupport";
	} else if (algo_support == AMS_TCS3707_NOR_ALGO) {
		hwlog_info("support tcs3707_nor algo\n");
		return "tcs3707_nor";
	} else if (algo_support == AMS_TCS3408_NOR_ALGO) {
		hwlog_info("support tcs3408_nor algo\n");
		return "tcs3408_nor";
	}
	hwlog_info("support hal default algo\n");
	return "tcs3408_nor";
}

int tcs3707_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
	int ret = -1;
	UINT32 i;
	int rc;
	struct device *dev = NULL;
	static struct color_chip *chip = NULL;
	enum tcs3707_sensor_id_t device_id;

	hwlog_info("enter %s\n", __func__);
	if (!client) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	dev = &client->dev;
	color_notify_support(); // declare support 3707

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		hwlog_err("%s: i2c smbus byte data unsupported\n", __func__);
		ret = -EOPNOTSUPP;
		goto init_failed;
	}

	chip = kzalloc(sizeof(struct color_chip), GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto malloc_failed;
	}

	mutex_init(&chip->lock);
	chip->client = client;
	chip->pdata = dev->platform_data;
	i2c_set_clientdata(chip->client, chip);

	chip->in_suspend = 0;
	chip->in_cal_mode = false;
	chip->cali_ctx.cal_state = 0;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		chip->cali_ctx.cal_c_rst[i] = TCS3707_CAL_RATIO;
		chip->cali_ctx.cal_r_rst[i] = TCS3707_CAL_RATIO;
		chip->cali_ctx.cal_g_rst[i] = TCS3707_CAL_RATIO;
		chip->cali_ctx.cal_b_rst[i] = TCS3707_CAL_RATIO;
		chip->cali_ctx.cal_w_rst[i] = TCS3707_CAL_RATIO;
		color_nv_para.cal_c_ratio[i] = TCS3707_CAL_RATIO;
		color_nv_para.cal_r_ratio[i] = TCS3707_CAL_RATIO;
		color_nv_para.cal_g_ratio[i] = TCS3707_CAL_RATIO;
		color_nv_para.cal_b_ratio[i] = TCS3707_CAL_RATIO;
		color_nv_para.cal_w_ratio[i] = TCS3707_CAL_RATIO;
	}

	device_id = tcs3707_get_chipid(chip->client);
	if (device_id == AMS_UNKNOWN_DEVICE) {
		hwlog_info("%s failed: AMS_UNKNOWN_DEVICE\n", __func__);
		goto id_failed;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_AP_COLOR_SENSOR);
#endif
	rc = of_property_read_u32(dev->of_node, "flicker_support",
		&flicker_support);
	if (rc < 0) {
		hwlog_warn("%s, get flicker_support failed\n", __func__);
		flicker_support = 1; // default support flk
	}
	rc = of_property_read_u32(dev->of_node, "algo_support",
		&algo_support);
	if (rc < 0) {
		hwlog_warn("%s, get algo_support failed\n", __func__);
		algo_support = AMS_UNSUPPORT_ALGO; // default not support algo
	}

	hwlog_info("%s flicker_support = %d, algo_support = %d\n", __func__,
		flicker_support, algo_support);
	tcs3707_get_revid(chip->client);

	chip->device_ctx = kzalloc(sizeof(struct tcs3707_ctx), GFP_KERNEL);
	if (!chip->device_ctx) {
		hwlog_info("ams_tcs3707 kzalloc failed\n");
		goto id_failed;
	}

	((struct tcs3707_ctx *)(chip->device_ctx))->handle = chip->client;
	tcs3707_reset_regs(chip->client);
	tcs3707_set_enable(chip->client, TCS3707_ENABLE_REG, PON, PON);
	msleep(3); // delay for mode device ready
	tcs3707_set_field(chip->client,	TCS3707_CTRL_REG,
		ALS_MANUAL_AZ, ALS_MANUAL_AZ);
	tcs3707_set_enable(chip->client, TCS3707_ENABLE_REG, 0, AEN);
	tcs3707_set_enable(chip->client, TCS3707_ENABLE_REG, 0, FDEN);

	init_timer(&chip->work_timer);
	setup_timer(&chip->work_timer, tcs3707_als_timer_wrk, (uintptr_t)chip);
	INIT_WORK(&chip->als_work, tcs3707_als_work);

	init_timer(&chip->fd_timer);
	setup_timer(&chip->fd_timer, tcs3707_flc_timer_wrk, (uintptr_t)chip);
	INIT_WORK(&chip->fd_work, tcs3707_fd_work);

	chip->at_color_show_calibrate_state = tcs3707_show_calibrate;
	chip->at_color_store_calibrate_state = tcs3707_store_calibrate;
	chip->color_enable_show_state = tcs3707_show_enable;
	chip->color_enable_store_state = tcs3707_store_enable;
	chip->color_sensor_get_gain = tcs3707_get_gain;
	chip->color_sensor_set_gain = tcs3707_set_als_gain;
	chip->get_flicker_data = tcs3707_get_fd_data;
	chip->flicker_enable_show_state = tcs3707_fd_show_enable;
	chip->flicker_enable_store_state = tcs3707_fd_store_enable;
	chip->color_report_type = tcs3707_rgb_report_type;
	chip->color_chip_name = tcs3707_chip_name;
	chip->color_algo_type = tcs3707_algo_type;

	p_chip = chip;
	ret = color_register(chip);
	if (ret < 0)
		hwlog_err("%s color_register fail\n", __func__);

	color_default_enable = tcs3707_enable_rgb;
	hwlog_info("%s ok\n", __func__);
	return 0;

id_failed:
	if (chip->device_ctx)
		kfree(chip->device_ctx);
	i2c_set_clientdata(client, NULL);
malloc_failed:
	kfree(chip);

init_failed:
	hwlog_err("%s Probe failed\n", __func__);
	color_notify_absent();
	return ret;
}

int tcs3707_suspend(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}

	hwlog_info("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	chip->in_suspend = 1;
	if (chip->wake_irq)
		irq_set_irq_wake(chip->client->irq, 1);
	else if (!chip->unpowered)
		hwlog_info("powering off\n");
	AMS_MUTEX_UNLOCK(&chip->lock);

	return 0;
}

int tcs3707_resume(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	return 0;
}

int ams_tcs3707_remove(struct i2c_client *client)
{
	struct color_chip *chip = NULL;

	if (!client) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = i2c_get_clientdata(client);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}

	free_irq(client->irq, chip);
	i2c_set_clientdata(client, NULL);
	kfree(chip->device_ctx);
	kfree(chip);
	return 0;
}

static struct i2c_device_id amsdriver_idtable[] = {
	{ "ams_tcs3707", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, amsdriver_idtable);

static const struct dev_pm_ops ams_tcs3707_pm_ops = {
	.suspend = tcs3707_suspend,
	.resume = tcs3707_resume,
};

static const struct of_device_id amsdriver_of_id_table[] = {
	{ .compatible = "ams,tcs3707" },
	{},
};

static struct i2c_driver ams_tcs3707_driver = {
	.driver = {
		.name = "ams_tcs3707",
		.owner = THIS_MODULE,
		.of_match_table = amsdriver_of_id_table,
	},
	.id_table = amsdriver_idtable,
	.probe = tcs3707_probe,
	.remove = ams_tcs3707_remove,
};

static int __init tcs3707_init(void)
{
	int rc;

	hwlog_info("%s\n", __func__);

	rc = i2c_add_driver(&ams_tcs3707_driver);

	hwlog_info("%s %d\n", __func__, rc);
	return rc;
}

static void __exit tcs3707_exit(void)
{
	hwlog_info("%s\n", __func__);
	i2c_del_driver(&ams_tcs3707_driver);
}

module_init(tcs3707_init);
module_exit(tcs3707_exit);

MODULE_AUTHOR("AMS AOS Software<cs.americas@ams.com>");
MODULE_DESCRIPTION("AMS tcs3707 ALS, XYZ color sensor driver");
MODULE_LICENSE("GPL");
