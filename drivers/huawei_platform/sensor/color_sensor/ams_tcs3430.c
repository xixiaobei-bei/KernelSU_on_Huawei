/*
 * ams_tcs3430.c
 *
 * code for AMS tcs3430 sensor
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
#include "ams_tcs3430.h"

#define HWLOG_TAG color_sensor
HWLOG_REGIST();

static struct color_chip *p_chip;
static bool color_calibrate_result = true;
static bool report_calibrate_result;
static bool color_devcheck_dmd_result = true;
static struct color_sensor_cali_nv_t color_nv_para;
static int read_nv_first_in;
static int enable_status_before_calibrate;
extern int (*color_default_enable)(bool enable);
static uint8_t report_logcount;
static int report_value[AMS_REPORT_DATA_LEN] = {0};
struct delayed_work ams_dmd_work;
#ifdef CONFIG_HUAWEI_DSM
extern struct dsm_client *shb_dclient;
#endif

#if defined(CONFIG_AMS_OPTICAL_SENSOR_ALS)
int tcs3430_port_get_byte(struct i2c_client *handle, uint8_t reg, uint8_t *data,
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

int tcs3430_port_set_byte(struct i2c_client *handle, uint8_t reg, uint8_t *data,
	uint8_t len)
{
	int ret;

	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = i2c_smbus_write_i2c_block_data(handle, reg, len, data);
	if (ret < 0)
		hwlog_err("%s: failed\n", __func__);

	return ret;
}

static int tcs3430_get_byte(struct i2c_client *handle, enum tcs3430_regs_t reg,
	UINT8 *read_data)
{
	int ret;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	if ((reg < AMS_TCS3430_DEVREG_ENABLE) ||
		(reg >= AMS_TCS3430_DEVREG_REG_MAX))
		return -EFAULT;

	ret = tcs3430_port_get_byte(handle, tcs3430_reg_def[reg].address,
		read_data, 1);
	return ret;
}

static int tcs3430_set_byte(struct i2c_client *handle, enum tcs3430_regs_t reg,
	UINT8 data)
{
	int ret = 0;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return ret;
	}

	ret = tcs3430_port_set_byte(handle, tcs3430_reg_def[reg].address,
		&data, 1);
	return ret;
}

static int tcs3430_get_buf(struct i2c_client *handle, enum tcs3430_regs_t reg,
	UINT8 *read_data, UINT8 length)
{
	int ret;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = tcs3430_port_get_byte(handle, tcs3430_reg_def[reg].address,
		read_data, length);
	return ret;
}

static int ams_tcs3430_report_data(int value[])
{
	hwlog_debug("ams tcs3430 report data\n");
	return ap_color_report(value, AMS_REPORT_DATA_LEN * sizeof(int));
}

static int tcs3430_set_word(struct i2c_client *handle, enum tcs3430_regs_t reg,
	UINT16 data)
{
	int ret;
	UINT8 length = sizeof(UINT16);
	UINT8 buffer[sizeof(UINT16)] = {0};

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	buffer[0] = ((data >> AMS_ENDIAN_1) & 0xff);
	buffer[1] = ((data >> AMS_ENDIAN_2) & 0xff);

	ret = tcs3430_port_set_byte(handle, tcs3430_reg_def[reg].address,
		&buffer[0], length);
	return ret;
}

static int tcs3430_get_field(struct i2c_client *handle, enum tcs3430_regs_t reg,
	UINT8 *read_data, enum regmask_t mask)
{
	int ret;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = tcs3430_port_get_byte(handle, tcs3430_reg_def[reg].address,
		read_data, 1);

	*read_data &= mask;

	return ret;
}

static int tcs3430_set_field(struct i2c_client *handle, enum tcs3430_regs_t reg,
	UINT8 data, enum regmask_t mask)
{
	int ret = 0;
	UINT8 original_data = 0;
	UINT8 new_data;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	tcs3430_get_byte(handle, reg, &original_data);

	new_data = original_data & (~mask);
	new_data |= (data & mask);

	if (new_data != original_data)
		ret = tcs3430_set_byte(handle, reg, new_data);

	return ret;
}

static int tcs3430_rgb_report_type(void)
{
	return AWB_SENSOR_RAW_SEQ_TYPE_R_G_B_IR;
}

static enum tcs3430_device_id_t tcs3430_test_id(struct i2c_client *handle)
{
	UINT8 chip_id = 0;
	UINT8 rev_id = 0;
	UINT8 i = 0;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return AMS_UNKNOWN_DEVICE;
	}

	tcs3430_get_byte(handle, AMS_TCS3430_DEVREG_ID, &chip_id);
	tcs3430_get_byte(handle, AMS_TCS3430_DEVREG_REVID, &rev_id);

	do {
		if (((chip_id & tcs3430_devids[i].dev_id_mask) ==
			(tcs3430_devids[i].dev_id &
			tcs3430_devids[i].dev_id_mask)) &&
			((rev_id & tcs3430_devids[i].dev_ref_mask) ==
			(tcs3430_devids[i].dev_ref &
			tcs3430_devids[i].dev_ref_mask)))
			return tcs3430_devids[i].device;

		i++;
	} while (tcs3430_devids[i].device != AMS_TCS3430_LAST_DEVICE);

	return AMS_UNKNOWN_DEVICE;
}

static void tcs3430_reset_regs(struct i2c_client *handle)
{
	enum tcs3430_regs_t i;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	for (i = AMS_TCS3430_DEVREG_ENABLE; i <= AMS_TCS3430_DEVREG_CFG1; i++)
		tcs3430_set_byte(handle, i, tcs3430_reg_def[i].reset_val);

	for (i = AMS_TCS3430_DEVREG_STATUS; i < AMS_TCS3430_DEVREG_REG_MAX; i++)
		tcs3430_set_byte(handle, i, tcs3430_reg_def[i].reset_val);
}

static UINT8 tcs3430_gain_to_reg(UINT32 x)
{
	UINT8 i;

	for (i = sizeof(tcs3430_als_gain_conv) / sizeof(UINT32) - 1; i != 0; i--)
		if (x >= tcs3430_als_gain_conv[i])
			break;

	hwlog_info("tcs3430_gain_to reg: %d %d\n", x, i);
	return i;
}


static INT32 tcs3430_get_gain(const void *ctx)
{
	UINT8 cfg1_reg_data = 0;
	UINT8 cfg2_reg_data = 0;
	INT32 gain;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	tcs3430_get_field(((struct tcs3430_ctx *)ctx)->handle,
		AMS_TCS3430_DEVREG_CFG1, &cfg1_reg_data, MASK_AGAIN);
	tcs3430_get_field(((struct tcs3430_ctx *)ctx)->handle,
		AMS_TCS3430_DEVREG_CFG2, &cfg2_reg_data, MASK_HGAIN);
	if (cfg1_reg_data >= AMS_TCS3430_GAIN_COUNT) // set max gain val to
		return 0;

	if (cfg2_reg_data) {
		if (cfg1_reg_data == AGAIN_64)
			gain = tcs3430_als_gains[cfg1_reg_data + 1];
		else
			gain = tcs3430_als_gains[cfg1_reg_data];
	} else {
		gain = tcs3430_als_gains[cfg1_reg_data];
	}
	return gain;
}

static INT32 tcs3430_set_gain(void *dev_ctx, int gain)
{
	INT32 ret = 0;
	UINT32 gain_len;
	struct tcs3430_ctx *ctx = NULL;

	if (!dev_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	ctx = dev_ctx;

	hwlog_info("tcs3430 set gain: %d\n", gain);
	gain_len = ARRAY_SIZE(tcs3430_als_gain_conv);
	if (gain_len == 0)
		return 0;

	if (gain >= tcs3430_als_gain_conv[gain_len - 1]) {
		// First, set AGAIN to 64x in CFG1
		ret += tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_CFG1,
			AGAIN_64, MASK_AGAIN);
		// Second set the HGAIN bit in CFG2
		ret += tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_CFG2,
			HGAIN, MASK_HGAIN);
	} else {
		// Make sure HGAIN bit is clear in CFG2
		ret += tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_CFG2,
			0, MASK_HGAIN);

		// Set AGAIN in CFG1
		ret += tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_CFG1,
			tcs3430_gain_to_reg((UINT32)gain), MASK_AGAIN);
	}
	ctx->alg_ctx.als_data.gain = (UINT32)gain;
	return ret;
}

static INT32 tcs3430_set_atime_ms(struct tcs3430_ctx *ctx, int atime_ms)
{
	INT32 ret;
	UINT8 atime_reg_data;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	atime_reg_data = AMS_TCS3430_MS_TO_ATIME(atime_ms);
	ret = tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_ATIME,
		atime_reg_data);

	return ret;
}

static bool tcs3430_als_reg_update(struct tcs3430_ctx *ctx, bool in_cal_mode)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	// For first integration after AEN
	ctx->first_inte = false;
	if (!(ctx->mode & AMS_TCS3430_MODE_ALS) && !in_cal_mode) {
		ctx->alg_ctx.als_data.gain = 16 * AMS_TCS3430_GAIN_SCALE;
		ctx->alg_ctx.als_data.atime_ms = 2;
		ctx->first_inte = true;
	}

	tcs3430_set_atime_ms(ctx, ctx->alg_ctx.als_data.atime_ms);
	tcs3430_set_gain(ctx, ctx->alg_ctx.als_data.gain);
	tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_PERS, 0, MASK_APERS);
	// set thresholds such that it would generate an interrupt
	// 2 bytes, AILTL and AILTH
	tcs3430_set_word(ctx->handle, AMS_TCS3430_DEVREG_AILTL, 0xffff);
	// 2 bytes, AIHTL and AIHTH
	tcs3430_set_word(ctx->handle, AMS_TCS3430_DEVREG_AIHTL, 0x0000);

	return false;
}

static bool tcs3430_dev_cfg(struct tcs3430_ctx *ctx,
	enum tcs3430_feature_t feature, enum tcs3430_cfg_op_t option,
	UINT32 data, bool in_cal_mode)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}
	if (feature != AMS_TCS3430_FEATURE_ALS) {
		hwlog_info("%s not support feature\n", __func__);
		return 0;
	}
	if (option != AMS_TCS3430_CONFIG_ENABLE) {
		hwlog_info("%s not support option\n", __func__);
		return 0;
	}
	if (data == 0) {
		tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_ENABLE, PON);
		tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_INTENAB, 0x00);
		tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_STATUS,
			(MASK_ASAT | MASK_AINT));
		ctx->mode = AMS_TCS3430_MODE_OFF;
	} else {
		tcs3430_als_reg_update(ctx, in_cal_mode);
		tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_INTENAB, AIEN);
		tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_ENABLE,
			(AEN | PON));
		tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_STATUS,
			(MASK_ASAT | MASK_AINT));
		ctx->mode |= AMS_TCS3430_MODE_ALS;
	}

	return 0;
}

static void tcs3430_get_max_min_raw(struct tcs3430_adc_data_t *curr_raw,
	uint32_t *max, uint32_t *min)
{
	*max = curr_raw->x;
	if (curr_raw->y > *max)
		*max = curr_raw->y;

	if (curr_raw->z > *max)
		*max = curr_raw->z;

	if (curr_raw->ir1 > *max)
		*max = curr_raw->ir1;

	*min = curr_raw->x;
	if (curr_raw->y < *min)
		*min = curr_raw->y;

	if (curr_raw->z < *min)
		*min = curr_raw->z;
}

static bool tcs3430_satu_check(struct tcs3430_ctx *ctx,
	struct tcs3430_adc_data_t *curr_raw)
{
	uint32_t satu;
	UINT16 atime;

	atime = ctx->alg_ctx.als_data.atime_ms;
	// calculate saturation value
	satu = ((AMS_TCS3430_MS_TO_ATIME(atime) + 1) << 10) - 1;
	satu = (satu * 9) / 10; // threadhold ratio 0.9 for saturation

	if (curr_raw->x > satu || curr_raw->y > satu ||	curr_raw->z > satu ||
		curr_raw->ir1 > satu)
		return true;

	return false;
}

static bool tcs3430_insuff_check(struct tcs3430_ctx *ctx,
	struct tcs3430_adc_data_t *curr_raw)
{
	if (curr_raw->x < AMS_TCS3430_LOW_LEVEL ||
		curr_raw->y < AMS_TCS3430_LOW_LEVEL ||
		curr_raw->z < AMS_TCS3430_LOW_LEVEL)
		return true;

	return false;
}

static bool tcs3430_handle_als(struct tcs3430_ctx *ctx, bool in_cal_mode)
{
	UINT8 adc_data[AMS_TCS3430_ADC_BYTES] = {0};
	struct tcs3430_adc_data_t curr_raw;
	uint8_t amux_data = 0;
	bool re_enable = false;
	uint32_t max_raw = 0;
	uint32_t min_raw = 0;
	UINT32 gain;

	memset(&curr_raw, 0, sizeof(curr_raw));
	// read ADC data
	tcs3430_get_buf(ctx->handle, AMS_TCS3430_DEVREG_CH0DATAL, adc_data,
		AMS_TCS3430_ADC_BYTES);
	tcs3430_get_field(ctx->handle, AMS_TCS3430_DEVREG_CFG1, &amux_data,
		MASK_AMUX);

	// extract raw X/IR2 channel data
	switch (ctx->device_id) {
	case AMS_TCS3430_REV0:
		curr_raw.x = (adc_data[0] << 0) | (adc_data[1] << 8);
		curr_raw.y = (adc_data[2] << 0) | (adc_data[3] << 8);
		curr_raw.ir2 = (adc_data[4] << 0) | (adc_data[5] << 8);
		if (amux_data == 0)
			curr_raw.z = (adc_data[6] << 0) | (adc_data[7] << 8);
		else
			curr_raw.ir1 = (adc_data[6] << 0) | (adc_data[7] << 8);
		break;
	case AMS_TCS3430_REV1:
		curr_raw.z = (adc_data[0] << 0) | (adc_data[1] << 8);
		curr_raw.y = (adc_data[2] << 0) | (adc_data[3] << 8);
		curr_raw.ir1 = (adc_data[4] << 0) | (adc_data[5] << 8);
		if (amux_data == 0)
			curr_raw.x = (adc_data[6] << 0) | (adc_data[7] << 8);
		else
			curr_raw.ir2 = (adc_data[6] << 0) | (adc_data[7] << 8);
		break;
	default:
		break;
	}
	if (in_cal_mode) {
		ctx->update_avai |= AMS_TCS3430_FEATURE_ALS;
		goto handle_als_exit;
	}

	// First integration is special
	if (ctx->first_inte) {
		// Get max and min raw data
		tcs3430_get_max_min_raw(&curr_raw, &max_raw, &min_raw);
		hwlog_info("als data: max =%d, min = %d\n", max_raw, min_raw);

		// Decide the proper gain setting
		if (max_raw < 100) // raw threadhold of 128x, set to 128
			tcs3430_set_gain(ctx, (128 * AMS_TCS3430_GAIN_SCALE));
		else if (max_raw < 200) //raw threadhold of 64x, set to 64
			tcs3430_set_gain(ctx, (64 * AMS_TCS3430_GAIN_SCALE));
		else if (max_raw < 800)// raw threadhold of 16x, set to 16
			hwlog_debug("keep 16x gain\n");
		else if (min_raw > 60) // raw threadhold of 1x, Set Gain to 1x
			tcs3430_set_gain(ctx, (1 * AMS_TCS3430_GAIN_SCALE));
		else // Set Gain to 4x
			tcs3430_set_gain(ctx, (4 * AMS_TCS3430_GAIN_SCALE));

		ctx->alg_ctx.als_data.atime_ms =
			AMS_TCS3430_NOMINAL_ATIME_DEFAULT;
		tcs3430_set_atime_ms(ctx, ctx->alg_ctx.als_data.atime_ms);
		re_enable = true;
		ctx->first_inte = false;
		goto handle_als_exit;
	}
	gain = ctx->alg_ctx.als_data.gain;
	// Adjust gain setting
	if (tcs3430_satu_check(ctx, &curr_raw) &&
		(gain == (4 * AMS_TCS3430_GAIN_SCALE))) {
		// Set Gain to 1x
		tcs3430_set_gain(ctx, (1 * AMS_TCS3430_GAIN_SCALE));
		re_enable = true;
	} else if ((tcs3430_satu_check(ctx, &curr_raw) &&
		(gain == (16 * AMS_TCS3430_GAIN_SCALE))) ||
		(tcs3430_insuff_check(ctx, &curr_raw) &&
		(gain == (1 * AMS_TCS3430_GAIN_SCALE)))) {
		// between 1x and 16x, Set Gain to 4x
		tcs3430_set_gain(ctx, (4 * AMS_TCS3430_GAIN_SCALE));
		re_enable = true;
	} else if ((tcs3430_satu_check(ctx, &curr_raw) &&
		(gain == (64 * AMS_TCS3430_GAIN_SCALE))) ||
		(tcs3430_insuff_check(ctx, &curr_raw) &&
		(gain == (4 * AMS_TCS3430_GAIN_SCALE)))) {
		// between 4x and 64x, set Gain to 16x
		tcs3430_set_gain(ctx, (16 * AMS_TCS3430_GAIN_SCALE));
		re_enable = true;
	} else if ((tcs3430_satu_check(ctx, &curr_raw) &&
		(gain == (128 * AMS_TCS3430_GAIN_SCALE))) ||
		(tcs3430_insuff_check(ctx, &curr_raw) &&
		(gain == (16 * AMS_TCS3430_GAIN_SCALE)))) {
		// between 16x and 128x, set Gain to 64x
		tcs3430_set_gain(ctx, (64 * AMS_TCS3430_GAIN_SCALE));
		re_enable = true;
	} else if (tcs3430_insuff_check(ctx, &curr_raw) &&
		(gain == (64 * AMS_TCS3430_GAIN_SCALE))) {
		// 64x, Set Gain to 128x
		tcs3430_set_gain(ctx, (128 * AMS_TCS3430_GAIN_SCALE));
		re_enable = true;
	} else {
		ctx->update_avai |= AMS_TCS3430_FEATURE_ALS;
	}

handle_als_exit:
	if (!re_enable) {
		ctx->alg_ctx.als_data.data_array.x = curr_raw.x;
		ctx->alg_ctx.als_data.data_array.y = curr_raw.y;
		ctx->alg_ctx.als_data.data_array.z = curr_raw.z;
		ctx->alg_ctx.als_data.data_array.ir1 = curr_raw.ir1;
		ctx->alg_ctx.als_data.data_array.ir2 = curr_raw.ir2;
	}

	hwlog_debug("tcs3430 handle_als: ATIME = %d, AGAIN = %d\n",
		(UINT16)ctx->alg_ctx.als_data.atime_ms,
		(UINT32)ctx->alg_ctx.als_data.gain);

	return re_enable;
}

static bool tcs3430_als_event_handler(struct tcs3430_ctx *ctx, bool in_cal_mode)
{
	bool ret = false;

	tcs3430_get_byte(ctx->handle, AMS_TCS3430_DEVREG_STATUS,
		&ctx->alg_ctx.als_data.status);

	if (ctx->alg_ctx.als_data.status & (MASK_AINT)) {
		if (ctx->mode & AMS_TCS3430_MODE_ALS) {
			ret = tcs3430_handle_als(ctx, in_cal_mode);
			tcs3430_set_byte(ctx->handle,
				AMS_TCS3430_DEVREG_STATUS,
				(MASK_ASAT | MASK_AINT));
		}
	}
	return ret;
}

static bool tcs3430_get_mode(struct tcs3430_ctx *ctx, enum tcs3430_mode_t *mode)
{
	if (!ctx || !mode) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	*mode = ctx->mode;
	return false;
}

static uint32_t tcs3430_get_avai(struct tcs3430_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	return ctx->update_avai;
}

static bool tcs3430_get_als_data(struct tcs3430_ctx *ctx,
	struct export_alsdata_t *export)
{
	ctx->update_avai &= ~(AMS_TCS3430_FEATURE_ALS);

	export->raw_x = ctx->alg_ctx.als_data.data_array.x;
	export->raw_y = ctx->alg_ctx.als_data.data_array.y;
	export->raw_z = ctx->alg_ctx.als_data.data_array.z;
	export->raw_ir = ctx->alg_ctx.als_data.data_array.ir1;
	export->raw_ir2 = ctx->alg_ctx.als_data.data_array.ir2;

	return false;
}

static void tcs3430_get_def_cal_data(struct _calibration_t *cal_data)
{
	if (!cal_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	cal_data->timebase_us = TCS3430_PER_TICK;
	cal_data->calibration.nominal_atime = AMS_TCS3430_NOMINAL_ATIME_DEFAULT;
	cal_data->calibration.nominal_again = AMS_TCS3430_NOMINAL_AGAIN_DEFAULT;
}

static bool tcs3430_device_init(struct tcs3430_ctx *ctx,
	struct i2c_client *handle)
{
	if (!ctx || !handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	hwlog_info("ams_deviceInit start\n");
	memset(ctx, 0, sizeof(struct tcs3430_ctx));
	ctx->handle = handle;
	ctx->mode = AMS_TCS3430_MODE_OFF;
	ctx->device_id = tcs3430_test_id(ctx->handle);
	tcs3430_reset_regs(ctx->handle);
	ctx->alg_ctx.als_data.atime_ms = AMS_TCS3430_NOMINAL_ATIME_DEFAULT;
	ctx->alg_ctx.als_data.gain = AMS_TCS3430_AGAIN_DEFAULT;
	tcs3430_set_gain(ctx, AMS_TCS3430_AGAIN_DEFAULT);
	tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_ENABLE, (AEN | PON));
	msleep(8); // sleep 8ms start the AZ_MODE
	tcs3430_set_byte(ctx->handle, AMS_TCS3430_DEVREG_ENABLE, PON);
	hwlog_info("ams_deviceInit end\n");
	return false;
}

static void tcs3430_get_dev_info(struct tcs3430_dev_info_t *info)
{
	if (!info) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	memset(info, 0, sizeof(struct tcs3430_dev_info_t));
	info->memory_size = sizeof(struct tcs3430_ctx);
	tcs3430_get_def_cal_data(&info->default_cal_data);
}

void tcs3430_als_timer_handle(uintptr_t data)
{
	struct color_chip *chip = (struct color_chip *)data;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->als_work);
}

static ssize_t osal_als_enable_set(struct color_chip *chip, uint8_t en)
{
	ssize_t rc = 0;
	enum tcs3430_mode_t mode = AMS_TCS3430_MODE_OFF;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	tcs3430_get_mode(chip->device_ctx, &mode);

	rc |= tcs3430_dev_cfg(chip->device_ctx, AMS_TCS3430_FEATURE_ALS,
		AMS_TCS3430_CONFIG_ENABLE, en, chip->in_cal_mode);
	hwlog_info("%s = %d\n", __func__, en);

	if (en) {
		if ((mode & AMS_TCS3430_MODE_ALS) == AMS_TCS3430_MODE_ALS) {
			hwlog_info("osal als enable: timer already running\n");
			return 0;
		}
		// if not enable before now set enable
		if (chip->in_cal_mode == false) {
			// first enable sleep 3ms for auto gain
			msleep(3);
			hwlog_info("first enable sleep 3ms\n");
			schedule_work(&chip->als_work);
			report_logcount = AMS_REPORT_LOG_COUNT_NUM;
		} else {
			// first enable set the timer as 120ms
			mod_timer(&chip->work_timer,
				jiffies + msecs_to_jiffies(120));
			hwlog_info("in cali mode timer set as 120ms\n");
		}
		hwlog_info("osal als enable: add_timer\n");
	} else {
		if ((mode & AMS_TCS3430_MODE_ALS) == AMS_TCS3430_MODE_ALS)
			hwlog_info("osal als enable: del_timer\n");
	}

	return 0;
}

static int get_cal_para_from_nv(void)
{
	int i;
	int ret;

	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if (ret < 0) {
		for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
			hwlog_err("%s: fail,use default para\n", __func__);
			hwlog_err("get cal gain[%d]: [%d, %d, %d, %d]\n", i,
				color_nv_para.cal_x_ratio[i],
				color_nv_para.cal_y_ratio[i],
				color_nv_para.cal_z_ratio[i],
				color_nv_para.cal_ir_ratio[i]);
		}
		return 0;
	}

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		hwlog_info("get cal gain[%d]: [%d, %d, %d, %d]\n", i,
			color_nv_para.cal_x_ratio[i],
			color_nv_para.cal_y_ratio[i],
			color_nv_para.cal_z_ratio[i],
			color_nv_para.cal_ir_ratio[i]);
		if (!color_nv_para.cal_x_ratio[i] ||
			!color_nv_para.cal_y_ratio[i] ||
			!color_nv_para.cal_z_ratio[i] ||
			!color_nv_para.cal_ir_ratio[i]) {
			color_nv_para.cal_x_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
			color_nv_para.cal_y_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
			color_nv_para.cal_z_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
			color_nv_para.cal_ir_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
		} else if ((color_nv_para.cal_x_ratio[i] >= TCS_LOW_THR) &&
			(color_nv_para.cal_x_ratio[i] <= TCS_HIGH_THR) &&
			(color_nv_para.cal_y_ratio[i] >= TCS_LOW_THR) &&
			(color_nv_para.cal_y_ratio[i] <= TCS_HIGH_THR) &&
			(color_nv_para.cal_z_ratio[i] >= TCS_LOW_THR) &&
			(color_nv_para.cal_z_ratio[i] <= TCS_HIGH_THR) &&
			(color_nv_para.cal_ir_ratio[i] >= TCS_LOW_THR) &&
			(color_nv_para.cal_ir_ratio[i] <= TCS_HIGH_THR)) {
			color_nv_para.cal_x_ratio[i] *= FLOAT_TO_FIX_LOW;
			color_nv_para.cal_y_ratio[i] *= FLOAT_TO_FIX_LOW;
			color_nv_para.cal_z_ratio[i] *= FLOAT_TO_FIX_LOW;
			color_nv_para.cal_ir_ratio[i] *= FLOAT_TO_FIX_LOW;
			hwlog_info("low_level nv calibrate data\n");
		}
	}

	for (i = 0; i <= CAL_STATE_GAIN_2; i++) {
		color_nv_para.cal_x_ratio[i] =
			color_nv_para.cal_x_ratio[CAL_STATE_GAIN_3];
		color_nv_para.cal_y_ratio[i] =
			color_nv_para.cal_y_ratio[CAL_STATE_GAIN_3];
		color_nv_para.cal_z_ratio[i] =
			color_nv_para.cal_z_ratio[CAL_STATE_GAIN_3];
		color_nv_para.cal_ir_ratio[i] =
			color_nv_para.cal_ir_ratio[CAL_STATE_GAIN_3];
		if (i == CAL_STATE_GAIN_1)
			hwlog_info("1x Gain equal to 16x for nv cali data\n");
		else if (i == CAL_STATE_GAIN_2)
			hwlog_info("4x Gain equal to 16x for nv cali data\n");
	}
	return 1;
}

static int save_cal_para_to_nv(struct color_chip *chip)
{
	unsigned int i = 0;
	int ret;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}

	color_nv_para.nv_x = chip->cali_ctx.cal_x_tar;
	color_nv_para.nv_y = chip->cali_ctx.cal_y_tar;
	color_nv_para.nv_z = chip->cali_ctx.cal_z_tar;
	color_nv_para.nv_ir = chip->cali_ctx.cal_ir_tar;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		color_nv_para.cal_x_ratio[i] = chip->cali_ctx.cal_x_rst[i];
		color_nv_para.cal_y_ratio[i] = chip->cali_ctx.cal_y_rst[i];
		color_nv_para.cal_z_ratio[i] = chip->cali_ctx.cal_z_rst[i];
		color_nv_para.cal_ir_ratio[i] = chip->cali_ctx.cal_ir_rst[i];
		hwlog_info("save nv: gain[%d]: [%d, %d, %d, %d]\n", i,
			color_nv_para.cal_x_ratio[i],
			color_nv_para.cal_y_ratio[i],
			color_nv_para.cal_z_ratio[i],
			color_nv_para.cal_ir_ratio[i]);
	}

	ret = write_color_calibrate_data_to_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if (ret < 0)
		hwlog_err("%s: fail\n", __func__);
	return 1;
}

static int tcs3430_als_cali_rgbc(struct rgb_cali_tar_t *cali_val)
{
	uint32_t result;

	result = cali_val->rawdata / AMS_TCS3430_CAL_AVERAGE;

	// cant devide by zero, use the default calibration scaling factor
	if (result == 0) {
		// just return defalut ratio
		hwlog_err("%s, raw data = 0\n", __func__);
		return AMS_TCS3430_FLOAT_TO_FIX;
	}

	result = cali_val->target_val * (cali_val->again *
		AMS_TCS3430_FLOAT_TO_FIX / AMS_TCS3430_GAIN_OF_GOLDEN) / result;

	if ((result > cali_val->high_th) || (result < cali_val->low_th)) {
		hwlog_info("%s ratio is out bound[%d, %d]! result = %d\n",
			__func__, cali_val->low_th, cali_val->high_th, result);
		color_calibrate_result = false;
	}
	return result;
}

static void tcs3430_als_cali_all_rgbc(struct color_sensor_cali_t *cal_raw,
	enum color_sensor_cal_states cal_idx, struct rgb_cali_tar_t *cali_tar)
{
	cali_tar->high_th = tcs3430_gain_thr[cal_idx].high_thr;
	cali_tar->low_th = tcs3430_gain_thr[cal_idx].low_thr;

	// X cali
	hwlog_info("%s cali R channel\n", __func__);
	cali_tar->rawdata = cal_raw->cal_r_raw;
	cali_tar->target_val = cal_raw->cal_r_tar;
	cal_raw->cal_r_rst[cal_idx] = tcs3430_als_cali_rgbc(cali_tar);

	// Y cali
	hwlog_info("%s cali G channel\n", __func__);
	cali_tar->rawdata = cal_raw->cal_g_raw;
	cali_tar->target_val = cal_raw->cal_g_tar;
	cal_raw->cal_g_rst[cal_idx] = tcs3430_als_cali_rgbc(cali_tar);

	// Z cali
	hwlog_info("%s cali B channel\n", __func__);
	cali_tar->rawdata = cal_raw->cal_b_raw;
	cali_tar->target_val = cal_raw->cal_b_tar;
	cal_raw->cal_b_rst[cal_idx] = tcs3430_als_cali_rgbc(cali_tar);

	// IR cali
	hwlog_info("%s cali C channel\n", __func__);
	cali_tar->rawdata = cal_raw->cal_c_raw;
	cali_tar->target_val = cal_raw->cal_c_tar;
	cal_raw->cal_c_rst[cal_idx] = tcs3430_als_cali_rgbc(cali_tar);

	hwlog_warn("%s, rawdat x = %d, y = %d, z = %d, ir = %d\n",
		__func__, cal_raw->cal_x_raw, cal_raw->cal_y_raw,
		cal_raw->cal_z_raw, cal_raw->cal_ir_raw);

	hwlog_warn("%s, result x = %d, y = %d, z = %d,ir = %d\n",
		__func__, cal_raw->cal_x_rst[cal_idx],
		cal_raw->cal_y_rst[cal_idx], cal_raw->cal_z_rst[cal_idx],
		cal_raw->cal_ir_rst[cal_idx]);
}

static void tcs3430_cal_handle_als(struct color_chip *chip)
{
	struct export_alsdata_t out;
	struct tcs3430_ctx *ctx = NULL;
	struct rgb_cali_tar_t cali_temp;
	uint32_t curr_gain;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->device_ctx;
	curr_gain = (ctx->alg_ctx.als_data.gain / AMS_TCS3430_GAIN_SCALE);

	tcs3430_get_als_data(chip->device_ctx, &out);

	hwlog_info("cali state %d\n", chip->cali_ctx.cal_state);
	hwlog_info("cali count %d\n", chip->cali_ctx.cal_raw_count);

	if ((chip->cali_ctx.cal_state < CAL_STATE_GAIN_LAST) &&
		(chip->cali_ctx.cal_state >= 0)) {
		chip->cali_ctx.cal_raw_count++;
		chip->cali_ctx.cal_x_raw += out.raw_x;
		chip->cali_ctx.cal_y_raw += out.raw_y;
		chip->cali_ctx.cal_z_raw += out.raw_z;
		chip->cali_ctx.cal_ir_raw += out.raw_ir;

		cali_temp.high_th =
			tcs3430_gain_thr[chip->cali_ctx.cal_state].high_thr;
		cali_temp.low_th =
			tcs3430_gain_thr[chip->cali_ctx.cal_state].low_thr;
		cali_temp.again = curr_gain;
		tcs3430_als_cali_all_rgbc(&chip->cali_ctx,
			chip->cali_ctx.cal_state, &cali_temp);

		chip->cali_ctx.cal_state++;
		chip->cali_ctx.cal_raw_count = 0;
		chip->cali_ctx.cal_x_raw = 0;
		chip->cali_ctx.cal_y_raw = 0;
		chip->cali_ctx.cal_z_raw = 0;
		chip->cali_ctx.cal_ir_raw = 0;
		if (chip->cali_ctx.cal_state < CAL_STATE_GAIN_LAST) {
			osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
			ctx->alg_ctx.als_data.gain =
				tcs3430_als_gain_conv[chip->cali_ctx.cal_state];
			osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
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
		osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
		tcs3430_set_gain(chip->device_ctx, AMS_TCS3430_AGAIN_DEFAULT);
		if (enable_status_before_calibrate == 1)
			osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
		else
			hwlog_info("color sensor disabled before calibrate\n");
		hwlog_info("osal_calHandl_als: done\n");
	}
}

static void osal_report_als(struct color_chip *chip)
{
	struct export_alsdata_t out = {0};
	uint8_t curr_gain_idx = 0;
	uint32_t curr_gain = 0;
	struct tcs3430_ctx *ctx = NULL;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = chip->device_ctx;

	tcs3430_get_als_data(chip->device_ctx, &out);
	curr_gain = ctx->alg_ctx.als_data.gain;

	if (curr_gain < tcs3430_als_gain_conv[CAL_STATE_GAIN_5])
		curr_gain_idx = tcs3430_gain_to_reg(curr_gain);
	else
		curr_gain_idx = CAL_STATE_GAIN_5;

	// adjust the report data when the calibrate ratio is acceptable

	out.raw_ir *= color_nv_para.cal_ir_ratio[curr_gain_idx];
	out.raw_ir /= AMS_TCS3430_FLOAT_TO_FIX;
	out.raw_x *= color_nv_para.cal_x_ratio[curr_gain_idx];
	out.raw_x /= AMS_TCS3430_FLOAT_TO_FIX;
	out.raw_y *= color_nv_para.cal_y_ratio[curr_gain_idx];
	out.raw_y /= AMS_TCS3430_FLOAT_TO_FIX;
	out.raw_z *= color_nv_para.cal_z_ratio[curr_gain_idx];
	out.raw_z /= AMS_TCS3430_FLOAT_TO_FIX;

	report_value[0] = (int)out.raw_x;
	report_value[1] = (int)out.raw_y;
	report_value[2] = (int)out.raw_z;
	report_value[3] = (int)out.raw_ir;

	report_value[0] *= AMS_TCS3430_GAIN_OF_GOLDEN;
	report_value[0] /= tcs3430_als_gains[curr_gain_idx];
	report_value[1] *= AMS_TCS3430_GAIN_OF_GOLDEN;
	report_value[1] /= tcs3430_als_gains[curr_gain_idx];
	report_value[2] *= AMS_TCS3430_GAIN_OF_GOLDEN;
	report_value[2] /= tcs3430_als_gains[curr_gain_idx];
	report_value[3] *= AMS_TCS3430_GAIN_OF_GOLDEN;
	report_value[3] /= tcs3430_als_gains[curr_gain_idx];

	ams_tcs3430_report_data(report_value);
	report_logcount++;
	if (report_logcount < AMS_REPORT_LOG_COUNT_NUM)
		return;

	hwlog_info("tcs3430 report data %d, %d, %d, %d\n",
		report_value[0], report_value[1], report_value[2],
		report_value[3]);
	hwlog_info("currentGain[%d]: [%d, %d, %d, %d]\n",
		tcs3430_als_gains[curr_gain_idx],
		color_nv_para.cal_x_ratio[curr_gain_idx],
		color_nv_para.cal_y_ratio[curr_gain_idx],
		color_nv_para.cal_z_ratio[curr_gain_idx],
		color_nv_para.cal_ir_ratio[curr_gain_idx]);
	report_logcount = 0;
}

int ams_tcs3430_setenable(bool enable)
{
	struct color_chip *chip = p_chip;

	if (enable)
		osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
	else
		osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);

	hwlog_info("ams tcs3430_setenable success\n");
	return 1;
}
EXPORT_SYMBOL_GPL(ams_tcs3430_setenable);

void ams_show_calibrate(struct color_chip *chip,
	struct color_sensor_output_t *out)
{
	unsigned int i;

	if (!out || !chip) {
		hwlog_err("ams show calibrate input para NULL\n");
		return;
	}

	if (chip->in_cal_mode == false)
		hwlog_err("ams show_calibrate not in calibration mode\n");

	out->result = (UINT32)report_calibrate_result;
	hwlog_info("color_calibrate_show result = %d\n", out->result);
	memcpy(out->report_ir, chip->cali_ctx.cal_ir_rst,
		sizeof(out->report_ir));
	memcpy(out->report_x, chip->cali_ctx.cal_x_rst, sizeof(out->report_x));
	memcpy(out->report_y, chip->cali_ctx.cal_y_rst, sizeof(out->report_y));
	memcpy(out->report_z, chip->cali_ctx.cal_z_rst, sizeof(out->report_z));

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		hwlog_info("color_calibrate_show i = %d: %d,%d,%d,%d\n", i,
		out->report_x[i], out->report_y[i],
		out->report_z[i], out->report_ir[i]);
		hwlog_info(" cali_ctx i = %d: %d,%d,%d,%d.\n", i,
		chip->cali_ctx.cal_x_rst[i], chip->cali_ctx.cal_y_rst[i],
		chip->cali_ctx.cal_z_rst[i], chip->cali_ctx.cal_ir_rst[i]);

	}
}
void ams_store_calibrate(struct color_chip *chip,
	struct color_sensor_input_t *in)
{
	struct tcs3430_ctx *ctx = NULL;
	enum tcs3430_mode_t mode = AMS_TCS3430_MODE_OFF;

	if (!chip || !in || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (in->enable && chip->in_cal_mode) {
		hwlog_err("Already in calibration mode\n");
		return;
	}

	ctx = chip->device_ctx;
	if (in->enable) {
		hwlog_info("ams store_calibrate: starting calibration mode\n");
		chip->cali_ctx.cal_raw_count = 0;
		chip->cali_ctx.cal_x_raw = 0;
		chip->cali_ctx.cal_y_raw = 0;
		chip->cali_ctx.cal_z_raw = 0;
		chip->cali_ctx.cal_ir_raw = 0;
		// calculate targer for gain 1x (assuming its set at 64x)
		chip->cali_ctx.cal_x_tar = in->tar_x;
		chip->cali_ctx.cal_y_tar = in->tar_y;
		chip->cali_ctx.cal_z_tar = in->tar_z;
		chip->cali_ctx.cal_ir_tar = in->tar_ir;
		chip->cali_ctx.cal_state = CAL_STATE_GAIN_1;
		ctx->alg_ctx.als_data.gain =
			tcs3430_als_gain_conv[CAL_STATE_GAIN_1];

		tcs3430_get_mode(chip->device_ctx, &mode);
		if ((mode & AMS_TCS3430_MODE_ALS) == AMS_TCS3430_MODE_ALS) {
			// enabled before calibrate
			enable_status_before_calibrate = 1;
			hwlog_info("%s: enabled before calibrate\n", __func__);
			osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
			msleep(10); // sleep 10 ms to make sure disable timer
		} else {
			// disabled before calibrate
			enable_status_before_calibrate = 0;
			hwlog_info("%s: disabled before calibrate\n", __func__);
		}
		chip->in_cal_mode = true;
		// make the calibrate_result true for calibrate again
		color_calibrate_result = true;
		osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
	} else {
		hwlog_info("ams store_calibrate: stopping calibration mode\n");
		chip->in_cal_mode = false;
	}
}

void ams_show_enable(struct color_chip *chip, int *state)
{
	enum tcs3430_mode_t mode = AMS_TCS3430_MODE_OFF;

	if (!chip || !state) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	tcs3430_get_mode(chip->device_ctx, &mode);
	if (mode & AMS_TCS3430_MODE_ALS)
		*state = 1;
	else
		*state = 0;
}

void ams_store_enable(struct color_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (state)
		osal_als_enable_set(chip, AMSDRIVER_ALS_ENABLE);
	else
		osal_als_enable_set(chip, AMSDRIVER_ALS_DISABLE);
}
#endif

static void amsdriver_work(struct work_struct *work)
{
	bool re_enable = false;
	enum tcs3430_mode_t mode = AMS_TCS3430_MODE_OFF;
	struct color_chip *chip = NULL;
	struct tcs3430_ctx *ctx = NULL;

	if (!work) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct color_chip, als_work);
	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer chip is NULL\n", __func__);
		return;
	}
	ctx = chip->device_ctx;

	AMS_MUTEX_LOCK(&chip->lock);
	if (read_nv_first_in == 0) {
		get_cal_para_from_nv();
		read_nv_first_in = -1; // -1: do not read again.
	}
	hwlog_info("amsdriver work\n");
	re_enable = tcs3430_als_event_handler(ctx, chip->in_cal_mode);
	if (re_enable) {
		tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_ENABLE, 0,
			MASK_AEN);
		tcs3430_set_field(ctx->handle, AMS_TCS3430_DEVREG_ENABLE, AEN,
			MASK_AEN);
	}

	if (tcs3430_get_avai(ctx) & AMS_TCS3430_FEATURE_ALS) {
		if ((chip->in_cal_mode == false) && !re_enable) {
			hwlog_info("amsdriver work: osal_report_als\n");
			osal_report_als(chip);
		} else {
			hwlog_info("amsdriver work: calibration mode\n");
			tcs3430_cal_handle_als(chip);
		}
	}

	tcs3430_get_mode(ctx, &mode);
	if (((mode & AMS_TCS3430_MODE_ALS) == AMS_TCS3430_MODE_ALS)) {
		if (chip->in_cal_mode == false) {
			if (re_enable) {
				mod_timer(&chip->work_timer,
					jiffies + msecs_to_jiffies(106));
				hwlog_info("timer set as 106ms\n");
			} else {
				mod_timer(&chip->work_timer, jiffies + HZ / 10);
				hwlog_info("timer set as 100ms\n");
			}
		} else {
			mod_timer(&chip->work_timer,
				jiffies + msecs_to_jiffies(120));
			hwlog_info("in calibrate mode, timer set as 120ms\n");
		}
	} else {
		hwlog_info("not mod timer mode = %d\n", mode);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
}

#ifdef CONFIG_HUAWEI_DSM
static void amsdriver_dmd_work(struct work_struct *work)
{
	if (color_devcheck_dmd_result == false) {
		if (dsm_client_ocuppy(shb_dclient))
			return;
		dsm_client_record(shb_dclient,
			"ap_color_sensor_detected fail\n");
		dsm_client_notify(shb_dclient,
			DSM_AP_ERR_COLORSENSOR_DETECT);
		hwlog_err("%s: tcs3430 detect fail\n", __func__);
	}
}
#endif

int amsdriver_probe(struct i2c_client *client,
	const struct i2c_device_id *idp)
{
	int ret = -1;
	unsigned int i;
	struct device *dev = NULL;
	static struct color_chip *chip;
	struct tcs3430_dev_info_t ams_dev_info;
	enum tcs3430_device_id_t device;

	if (!client) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}

	hwlog_info("amsdriver probe\n");
	dev = &client->dev;

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
		chip->cali_ctx.cal_x_rst[i] = AMS_TCS3430_FLOAT_TO_FIX;
		chip->cali_ctx.cal_y_rst[i] = AMS_TCS3430_FLOAT_TO_FIX;
		chip->cali_ctx.cal_z_rst[i] = AMS_TCS3430_FLOAT_TO_FIX;
		chip->cali_ctx.cal_ir_rst[i] = AMS_TCS3430_FLOAT_TO_FIX;
		color_nv_para.cal_x_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
		color_nv_para.cal_y_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
		color_nv_para.cal_z_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
		color_nv_para.cal_ir_ratio[i] = AMS_TCS3430_FLOAT_TO_FIX;
	}

#ifdef CONFIG_HUAWEI_DSM
	INIT_DELAYED_WORK(&ams_dmd_work, amsdriver_dmd_work);
#endif
	device = tcs3430_test_id(chip->client);

	if (device == AMS_UNKNOWN_DEVICE) {
		hwlog_err("tcs3430_test_id failed: AMS_UNKNOWN_DEVICE\n");
#ifdef CONFIG_HUAWEI_DSM
		color_devcheck_dmd_result = false;
		schedule_delayed_work(&ams_dmd_work,
			msecs_to_jiffies(AP_COLOR_DMD_DELAY_TIME_MS));
#endif
		goto id_failed;
	}

	hwlog_info("tcs3430_test_id ok\n");
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_AP_COLOR_SENSOR);
#endif
	tcs3430_get_dev_info(&ams_dev_info);
	hwlog_info("ams dev info ok\n");

	chip->device_ctx = kzalloc(ams_dev_info.memory_size, GFP_KERNEL);
	if (chip->device_ctx) {
		ret = tcs3430_device_init(chip->device_ctx, chip->client);
		if (ret == false) {
			hwlog_info("ams deviceInit ok\n");
		} else {
			hwlog_info("ams deviceInit failed\n");
			goto id_failed;
		}
	} else {
		hwlog_info("ams_tcs3430 kzalloc failed\n");
		goto id_failed;
	}

	init_timer(&chip->work_timer);
	setup_timer(&chip->work_timer, tcs3430_als_timer_handle,
		(uintptr_t)chip);
	INIT_WORK(&chip->als_work, amsdriver_work);

	chip->color_show_calibrate_state = ams_show_calibrate;
	chip->color_store_calibrate_state = ams_store_calibrate;
	chip->color_enable_show_state = ams_show_enable;
	chip->color_enable_store_state = ams_store_enable;
	chip->color_sensor_get_gain = tcs3430_get_gain;
	chip->color_sensor_set_gain = tcs3430_set_gain;
	chip->color_report_type = tcs3430_rgb_report_type;
	p_chip = chip;

	ret = color_register(chip);
	if (ret < 0)
		hwlog_err("color_register fail\n");

	color_default_enable = ams_tcs3430_setenable;
	hwlog_info("tcs3430 Probe ok\n");
	return 0;

id_failed:
	if (chip->device_ctx)
		kfree(chip->device_ctx);
	i2c_set_clientdata(client, NULL);

malloc_failed:
	kfree(chip);

init_failed:
	hwlog_err("tcs3430 Probe failed\n");
	return ret;
}

int amsdriver_suspend(struct device *dev)
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

int amsdriver_resume(struct device *dev)
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

int amsdriver_remove(struct i2c_client *client)
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
	{ "ams_tcs3430", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, amsdriver_idtable);

static const struct dev_pm_ops amsdriver_pm_ops = {
	.suspend = amsdriver_suspend,
	.resume = amsdriver_resume,
};

static const struct of_device_id amsdriver_of_id_table[] = {
	{ .compatible = "ams,tcs3430" },
	{},
};


static struct i2c_driver amsdriver_driver = {
	.driver = {
		.name = "ams_tcs3430",
		.owner = THIS_MODULE,
		.of_match_table = amsdriver_of_id_table,
	},
	.id_table = amsdriver_idtable,
	.probe = amsdriver_probe,
	.remove = amsdriver_remove,
};

static int __init amsdriver_init(void)
{
	int rc;

	hwlog_info("ams_tcs3430: init\n");
	rc = i2c_add_driver(&amsdriver_driver);
	hwlog_info("ams_tcs3430: %d", rc);
	return rc;
}

static void __exit amsdriver_exit(void)
{
	hwlog_info("ams_tcs3430: exit\n");
	i2c_del_driver(&amsdriver_driver);
}

module_init(amsdriver_init);
module_exit(amsdriver_exit);

MODULE_AUTHOR("AMS AOS Software<cs.americas@ams.com>");
MODULE_DESCRIPTION("AMS tcs3430 ALS, XYZ color sensor driver");
MODULE_LICENSE("GPL");
