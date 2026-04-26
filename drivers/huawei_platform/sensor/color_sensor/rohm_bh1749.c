/*
 * rohm_bh1749.c
 *
 * code for rohm bh1749 sensor
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "rohm_bh1749.h"

#define HWLOG_TAG color_sensor
HWLOG_REGIST();

static int report_value[ROHM_REPORT_DATA_LEN] = {0};
static struct color_chip *p_chip;
static bool color_calibrate_result = true;
static bool report_calibrate_result;
static struct color_sensor_cali_nv_t color_nv_para;
static int read_nv_first_in;
static int enable_status_before_calibrate;
static struct delayed_work rohm_dmd_work;
static uint8_t report_logcount;

#ifdef CONFIG_HUAWEI_DSM
static bool color_devcheck_dmd_result = true;
extern struct dsm_client *shb_dclient;
#endif
extern int (*color_default_enable)(bool enable);

#if defined(CONFIG_ROHM_OPTICAL_SENSOR_ALS)
int bh1749_port_get_byte(struct i2c_client *handle, uint8_t reg, uint8_t *data,
	uint8_t len)
{
	int ret;

	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = i2c_smbus_read_i2c_block_data(handle, reg, len, data);
	if (ret < 0)
		hwlog_err("%s: failed\n", __func__);

	return ret;
}

int bh1749_port_set_byte(struct i2c_client *handle, uint8_t reg, uint8_t *data,
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

static int bh1749_get_byte(struct i2c_client *handle, uint8_t reg,
	UINT8 *read_data)
{
	int read_count;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	read_count = bh1749_port_get_byte(handle, reg, read_data, 1);

	return read_count;
}

static int bh1749_set_byte(struct i2c_client *handle, uint8_t reg, UINT8 data)
{
	int write_count;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	write_count = bh1749_port_set_byte(handle, reg,	&data, 1);
	return write_count;
}

static int bh1749_get_buf(struct i2c_client *handle, uint8_t reg,
	UINT8 *read_data, UINT8 length)
{
	int ret;

	if (!handle || !read_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ret = bh1749_port_get_byte(handle, reg, read_data, length);

	return ret;
}

static int bh1749_rgb_report_type(void)
{
	return AWB_SENSOR_RAW_SEQ_TYPE_R_G_B_IR;
}

static int bh1749_report_data(int value[])
{
	hwlog_debug("rohm_bh1749_report_data\n");
	return ap_color_report(value, ROHM_REPORT_DATA_LEN * sizeof(int));
}

static INT32 bh1749_set_gain(void *ctx, int gain)
{
	UINT8 cfg1_data = 0;

	if (!ctx) {
		hwlog_err("ROHM_Driver: %s: Pointer is NULL\n", __func__);
		return -1;
	}

	hwlog_info("%s: %d\n", __func__, gain);
	bh1749_get_byte(((struct bh1749_ctx_t *)ctx)->handle,
		ROHM_BH1749_MODECONTROL1, &cfg1_data);
	if ((uint32_t)gain == GAIN_X1) {
		cfg1_data = (cfg1_data & RGB_GAIN_MASK) | RGB_GAIN_X1;
		((struct bh1749_ctx_t *)ctx)->alg_ctx.als_data.gain =
			GAIN_X1 * BH1749_GAIN_SCALE;
	} else if ((uint32_t)gain == GAIN_X32) {
		cfg1_data = (cfg1_data & RGB_GAIN_MASK) | RGB_GAIN_X32;
		((struct bh1749_ctx_t *)ctx)->alg_ctx.als_data.gain =
			GAIN_X32 * BH1749_GAIN_SCALE;
	} else {
		hwlog_err("Invalid gain\n");
		return 0;
	}
	bh1749_set_byte(((struct bh1749_ctx_t *)ctx)->handle,
		ROHM_BH1749_MODECONTROL1, cfg1_data);

	return 0;
}

static UINT8 bh1749_get_devid(struct i2c_client *handle)
{
	UINT8 chip = 0;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return ROHM_UNKNOWN_DEVICE;
	}

	bh1749_get_byte(handle, ROHM_BH1749_MANUFACT_ID, &chip);
	return chip;
}

static INT32 bh1749_get_gain(const void *ctx)
{
	UINT8 cfg1_data = 0;
	INT32 gain;

	// intalize gain original val
	unsigned char again[ROHM_GAIN_SIZE] = { 0, 1, 0, 32 };

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	bh1749_get_byte(((struct bh1749_ctx_t *)ctx)->handle,
		ROHM_BH1749_MODECONTROL1, &cfg1_data);

	gain = again[RGB_GAIN_VALUE(cfg1_data)];
	return gain;
}

static UINT8 bh1749_gain_to_reg(UINT32 x)
{
	UINT8 i;

	for (i = sizeof(bh1749_als_gain_conv) / sizeof(UINT32) - 1; i != 0; i--)
		if (x >= bh1749_als_gain_conv[i])
			break;

	hwlog_info("bh1749 gain to reg: %d %d\n", x, i);
	return i;
}

static void bh1749_set_ir_gain(void *ctx, uint32_t gain)
{
	UINT8 cfg1_data = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	hwlog_info("%s: %d\n", __func__, gain);
	bh1749_get_byte(((struct bh1749_ctx_t *)ctx)->handle,
		ROHM_BH1749_MODECONTROL1, &cfg1_data);
	if (gain == GAIN_X1) {
		cfg1_data = (cfg1_data & IR_GAIN_MASK) | IR_GAIN_X1;
		((struct bh1749_ctx_t *)ctx)->alg_ctx.als_data.gain_ir =
			GAIN_X1 * BH1749_GAIN_SCALE;
	} else if (gain == GAIN_X32) {
		cfg1_data = (cfg1_data & IR_GAIN_MASK) | IR_GAIN_X32;
		((struct bh1749_ctx_t *)ctx)->alg_ctx.als_data.gain_ir =
			GAIN_X32 * BH1749_GAIN_SCALE;
	} else {
		hwlog_err("Invalid ir_gain\n");
		return;
	}
	bh1749_set_byte(((struct bh1749_ctx_t *)ctx)->handle,
		ROHM_BH1749_MODECONTROL1, cfg1_data);
}


static void bh1749_set_atime(struct bh1749_ctx_t *ctx, int atime_ms)
{
	UINT8 atime_data = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	bh1749_get_byte(ctx->handle, ROHM_BH1749_MODECONTROL1, &atime_data);

	if (atime_ms == TIME_120MS) {
		atime_data = (atime_data & TIME_GAIN_MASK) | MEASURE_120MS;
		ctx->alg_ctx.als_data.atime_ms = TIME_120MS;
	} else if (atime_ms == TIME_240MS) {
		atime_data = (atime_data & TIME_GAIN_MASK) | MEASURE_240MS;
		ctx->alg_ctx.als_data.atime_ms = TIME_240MS;
	} else if (atime_ms == TIME_35MS) {
		atime_data = (atime_data & TIME_GAIN_MASK) | MEASURE_35MS;
		ctx->alg_ctx.als_data.atime_ms = TIME_35MS;
	} else {
		hwlog_err("Invalid ir_gain\n");
	}
	bh1749_set_byte(ctx->handle, ROHM_BH1749_MODECONTROL1, atime_data);
}

static void bh1749_reset_all_regs(struct bh1749_ctx_t *ctx)
{
	UINT8 tmp;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	hwlog_info("_bh1749_resetAllRegisters\n");
	// reset
	tmp = SW_RESET | INT_RESET;
	bh1749_set_byte(ctx->handle, ROHM_BH1749_SYSTEMCONTROL, tmp);

	// set 32X default
	bh1749_set_gain(ctx, GAIN_X32);
	bh1749_set_ir_gain(ctx, GAIN_X32);

	// set 35ms default
	bh1749_set_atime(ctx, TIME_120MS);
}

static bool bh1749_saturation_check(struct bh1749_ctx_t *ctx,
	struct raw_data_arg_t *curr_raw)
{
	if (!curr_raw) {
		hwlog_err("saturation_check NULL pointer!\n");
		return false;
	}
	if (curr_raw->red > ROHM_BH1749_HIGH_LEVEL ||
		curr_raw->green > ROHM_BH1749_HIGH_LEVEL ||
		curr_raw->blue > ROHM_BH1749_HIGH_LEVEL)
		return true;

	return false;
}

static bool bh1749_insuff_check(struct bh1749_ctx_t *ctx,
	struct raw_data_arg_t *curr_raw)
{
	if (!curr_raw) {
		hwlog_err("insuff_check NULL pointer!\n");
		return false;
	}
	if (curr_raw->red < ROHM_BH1749_LOW_LEVEL ||
		curr_raw->green < ROHM_BH1749_LOW_LEVEL ||
		curr_raw->blue < ROHM_BH1749_LOW_LEVEL)
		return true;

	return false;
}

static bool bh1749_saturation_ir_check(struct bh1749_ctx_t *ctx,
	struct raw_data_arg_t *curr_raw)
{
	if (!curr_raw) {
		hwlog_err("saturation_ir_check NULL pointer\n");
		return false;
	}

	if (curr_raw->ir > ROHM_BH1749_HIGH_LEVEL)
		return true;

	return false;
}

static bool bh1749_insufficience_ir_check(struct bh1749_ctx_t *ctx,
	struct raw_data_arg_t *curr_raw)
{
	if (!curr_raw) {
		hwlog_err("insufficience_ir_check NULL pointer\n");
		return false;
	}

	if (curr_raw->ir < ROHM_BH1749_LOW_LEVEL)
		return true;

	return false;
}

static bool bh1749_handle_als(struct bh1749_ctx_t *ctx, bool in_cal_mode)
{
	UINT8 adc_data[ROHM_BH1749_ADC_BYTES] = {0};
	struct raw_data_arg_t rgb_rawdata = {0};
	uint8_t valid_data = 0;
	bool re_enable = false;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return false;
	}

	// read valid status
	bh1749_get_byte(ctx->handle, ROHM_BH1749_MODECONTROL2, &valid_data);
	hwlog_info("%s: BH1749_MODECONTROL2 = 0x%x\n", __func__, valid_data);

	if (valid_data & RGBC_VALID_HIGH) {
		// read 6 ADC data
		bh1749_get_buf(ctx->handle, ROHM_BH1749_RED_DATA, adc_data, 6);
		rgb_rawdata.red = (adc_data[1] << 8) | adc_data[0];
		rgb_rawdata.green = (adc_data[3] << 8) | adc_data[2];
		rgb_rawdata.blue = (adc_data[5] << 8) | adc_data[4];

		bh1749_get_buf(ctx->handle, ROHM_BH1749_IR_DATA, adc_data, 2);
		rgb_rawdata.ir = (adc_data[1] << 8) | adc_data[0];
		hwlog_info("%s: r=%d, g = %d, b = %d, ir = %d\n", __func__,
			rgb_rawdata.red, rgb_rawdata.green,
			rgb_rawdata.blue, rgb_rawdata.ir);

		ctx->alg_ctx.als_data.data_array.x = rgb_rawdata.red;
		ctx->alg_ctx.als_data.data_array.y = rgb_rawdata.green;
		ctx->alg_ctx.als_data.data_array.z = rgb_rawdata.blue;
		ctx->alg_ctx.als_data.data_array.ir1 = rgb_rawdata.ir;
		ctx->alg_ctx.als_data.data_array.ir2 = rgb_rawdata.ir;
	} else {
		hwlog_err("%s Invalid status\n", __func__);
		return true;
	}

	if (in_cal_mode) {
		ctx->rgb_available |= BH1749_FEATURE_ALS;
		hwlog_warn("%s: ATIME = %d, AGAIN = %d, inCalMode = %d\n",
			__func__, ctx->alg_ctx.als_data.atime_ms,
			ctx->alg_ctx.als_data.gain, in_cal_mode);
		return false;
	}

	// Adjust rgb gain setting
	if (bh1749_saturation_check(ctx, &rgb_rawdata)) {
		if (ctx->alg_ctx.als_data.gain == GAIN_COV(GAIN_X32)) {
			bh1749_set_gain(ctx, GAIN_X1); // Set Gain to 1x
			re_enable = true;
		}

	} else if (bh1749_insuff_check(ctx, &rgb_rawdata)) {
		if (ctx->alg_ctx.als_data.gain == GAIN_COV(GAIN_X1)) {
			bh1749_set_gain(ctx, GAIN_X32); // Set Gain to 32x
			re_enable = true;
		}
	}

	// Adjust ir gain setting
	if (bh1749_saturation_ir_check(ctx, &rgb_rawdata)) {
		if (ctx->alg_ctx.als_data.gain_ir == GAIN_COV(GAIN_X32)) {
			bh1749_set_ir_gain(ctx, GAIN_X1); // Set ir Gain to 1x
			re_enable = true;
		}
	} else if (bh1749_insufficience_ir_check(ctx, &rgb_rawdata)) {
		if (ctx->alg_ctx.als_data.gain_ir == GAIN_COV(GAIN_X1)) {
			bh1749_set_ir_gain(ctx, GAIN_X32); // Set ir Gain to 32
			re_enable = true;
		}
	}

	// set available every time, because timer is more than 50ms,
	// our mesaure time is 35ms
	ctx->rgb_available |= BH1749_FEATURE_ALS;

	return re_enable;
}

static bool bh1749_event_handler(struct bh1749_ctx_t *ctx, bool in_cal_mode)
{
	bool ret = false;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return false;
	}

	ret = bh1749_handle_als(ctx, in_cal_mode);
	return ret;
}

static bool bh1749_get_mode(struct bh1749_ctx_t *ctx,
	enum bh1749_rohm_mode_t *mode)
{
	if (!ctx || !mode) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	*mode = ctx->mode;
	return false;
}

static bool bh1749_set_mode(struct bh1749_ctx_t *ctx,
	enum bh1749_rohm_mode_t mode)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	ctx->mode = mode;
	return false;
}

static uint32_t bh1749_get_rgb_avai(struct bh1749_ctx_t *ctx)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	return ctx->rgb_available;
}

static bool bh1749_get_als_data(struct bh1749_ctx_t *ctx,
	struct export_als_data_t *export)
{
	if (!ctx || !export) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	ctx->rgb_available &= ~(BH1749_FEATURE_ALS);

	export->raw_x = ctx->alg_ctx.als_data.data_array.x;
	export->raw_y = ctx->alg_ctx.als_data.data_array.y;
	export->raw_z = ctx->alg_ctx.als_data.data_array.z;
	export->raw_ir = ctx->alg_ctx.als_data.data_array.ir1;
	export->raw_ir2 = ctx->alg_ctx.als_data.data_array.ir1;

	return false;
}

static void bh1749_device_init(struct bh1749_ctx_t *ctx,
	struct i2c_client *handle)
{
	if (!ctx || !handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	memset(ctx, 0, sizeof(struct bh1749_ctx_t));
	ctx->handle = handle;
	ctx->mode = ROHM_BH1749_MODE_OFF;
	ctx->device_id = bh1749_get_devid(ctx->handle);
	bh1749_reset_all_regs(ctx);
}

static bool bh1749_get_dev_info(struct bh1749_device_info_t *info)
{
	if (!info) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	memset(info, 0, sizeof(struct bh1749_device_info_t));
	info->memory_size = sizeof(struct bh1749_ctx_t);
	return false;
}

static void bh1749_als_timer_handle(uintptr_t data)
{
	struct color_chip *chip = (struct color_chip *)data;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	schedule_work(&chip->als_work);
}

static ssize_t bh1749_als_set_enable(struct color_chip *chip, uint8_t en)
{
	enum bh1749_rohm_mode_t mode = ROHM_BH1749_MODE_OFF;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	bh1749_get_mode(chip->device_ctx, &mode);
	if (en) {
		bh1749_set_byte(chip->client, ROHM_BH1749_MODECONTROL2,
			RGBC_EN_ON);
		// first enable set the timer as 150ms
		report_logcount = ROHM_REPORT_LOG_COUNT_NUM;
		mod_timer(&chip->work_timer,
			jiffies + msecs_to_jiffies(ROHM_POLL_TIME));
		bh1749_set_mode(chip->device_ctx, ROHM_BH1749_MODE_ALS);
	} else {
		bh1749_set_byte(chip->client, ROHM_BH1749_MODECONTROL2,
			RGBC_EN_OFF);
		bh1749_set_mode(chip->device_ctx, ROHM_BH1749_MODE_OFF);
	}

	hwlog_info("%s = %d\n", __func__, en);
	return 0;
}

static int get_cal_para_from_nv(void)
{
	unsigned int i;
	int ret;

	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if (ret < 0) {
		hwlog_err("%s: fail,use default para!!\n", __func__);
		return 0;
	}
	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		hwlog_warn("%s: gain[%d]: [%d, %d, %d, %d]\n", __func__, i,
		color_nv_para.cal_x_ratio[i], color_nv_para.cal_y_ratio[i],
		color_nv_para.cal_z_ratio[i], color_nv_para.cal_ir_ratio[i]);
		if (!color_nv_para.cal_x_ratio[i] ||
			!color_nv_para.cal_y_ratio[i] ||
			!color_nv_para.cal_z_ratio[i] ||
			!color_nv_para.cal_ir_ratio[i]) {
			color_nv_para.cal_x_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
			color_nv_para.cal_y_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
			color_nv_para.cal_z_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
			color_nv_para.cal_ir_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
		}
	}
	// copy calibration coefficient 32X to 1X as ROHM suggestion
	color_nv_para.cal_x_ratio[CAL_STATE_GAIN_1] =
		color_nv_para.cal_x_ratio[CAL_STATE_GAIN_2];
	color_nv_para.cal_y_ratio[CAL_STATE_GAIN_1] =
		color_nv_para.cal_y_ratio[CAL_STATE_GAIN_2];
	color_nv_para.cal_z_ratio[CAL_STATE_GAIN_1] =
		color_nv_para.cal_z_ratio[CAL_STATE_GAIN_2];
	color_nv_para.cal_ir_ratio[CAL_STATE_GAIN_1] =
		color_nv_para.cal_ir_ratio[CAL_STATE_GAIN_2];

	return 1;
}

static int save_cal_para_to_nv(struct color_chip *chip)
{
	unsigned int i;
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
		hwlog_warn("save cal nv: gain[%d]: [%d, %d, %d, %d]\n", i,
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

static int bh1749_als_cali_rgb(struct rgb_cali_tar_t *cali_val)
{
	uint32_t result;

	result = cali_val->rawdata / ROHM_BH1749_CAL_AVERAGE;

	// cant devide by zero, use the default calibration scaling factor
	if (result == 0) {
		hwlog_err("%s raw data = 0\n", __func__);
		return ROHM_BH1749_FLOAT_TO_FIX;
	}
	if ((cali_val->again == 0) || (cali_val->atime_ms == 0)) {
		color_calibrate_result = false;
		hwlog_err("%s atime_ms again data = 0\n", __func__);
		return ROHM_BH1749_FLOAT_TO_FIX;
	}

	// transfer to 32X gain 120ms atime
	result = cali_val->rawdata * DATA_TRANSFER_COFF / cali_val->again /
		cali_val->atime_ms;
	if (result == 0) {
		hwlog_err("%s cali rst data = 0\n", __func__);
		color_calibrate_result = false;
		return ROHM_BH1749_FLOAT_TO_FIX;
	}
	result = cali_val->target_val * ROHM_BH1749_FLOAT_TO_FIX / result;
	if ((result > cali_val->high_th) || (result < cali_val->low_th)) {
		hwlog_info("%s: ratio is out bound[%d, %d]! result = %d\n",
			__func__, cali_val->low_th, cali_val->high_th, result);
		color_calibrate_result = false;
	}
	return result;
}

static void bh1749_als_cali_all_rgbc(struct color_sensor_cali_t *cal_raw,
	enum color_sensor_cal_states cal_idx, struct rgb_cali_tar_t *cali_tar)
{
	// X cali
	hwlog_info("%s, cali X chanel, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_x_raw, cal_raw->cal_x_tar);
	cali_tar->rawdata = cal_raw->cal_x_raw;
	cali_tar->target_val = cal_raw->cal_x_tar;
	cal_raw->cal_x_rst[cal_idx] = bh1749_als_cali_rgb(cali_tar);

	// Y cali
	hwlog_info("%s, cali Y chanel, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_y_raw, cal_raw->cal_y_tar);
	cali_tar->rawdata = cal_raw->cal_y_raw;
	cali_tar->target_val = cal_raw->cal_y_tar;
	cal_raw->cal_y_rst[cal_idx] = bh1749_als_cali_rgb(cali_tar);

	// Z cali
	hwlog_info("%s, cali Z chanel, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_z_raw, cal_raw->cal_z_tar);
	cali_tar->rawdata = cal_raw->cal_z_raw;
	cali_tar->target_val = cal_raw->cal_z_tar;
	cal_raw->cal_z_rst[cal_idx] = bh1749_als_cali_rgb(cali_tar);

	// IR cali
	hwlog_info("%s, cali IR chanel, raw = %d, tar = %d\n", __func__,
		cal_raw->cal_ir_raw, cal_raw->cal_ir_tar);
	cali_tar->rawdata = cal_raw->cal_ir_raw;
	cali_tar->target_val = cal_raw->cal_ir_tar;
	cal_raw->cal_ir_rst[cal_idx] = bh1749_als_cali_rgb(cali_tar);

	hwlog_warn("%s, result X = %d, Y = %d, Z = %d, IR = %d\n", __func__,
		cal_raw->cal_x_rst[cal_idx], cal_raw->cal_y_rst[cal_idx],
		cal_raw->cal_z_rst[cal_idx], cal_raw->cal_ir_rst[cal_idx]);
}

static void bh1749_cal_als(struct color_chip *chip)
{
	struct export_als_data_t out;
	struct bh1749_ctx_t *ctx = NULL;
	uint32_t curr_gain = 0;
	struct rgb_cali_tar_t cali_tar_temp;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->device_ctx;
	curr_gain = ctx->alg_ctx.als_data.gain / BH1749_GAIN_SCALE;

	bh1749_get_als_data(chip->device_ctx, &out);

	hwlog_info("cal state = %d, count = %d, gain = %d\n",
		chip->cali_ctx.cal_state, chip->cali_ctx.cal_raw_count,
		ctx->alg_ctx.als_data.gain);

	if ((ctx->alg_ctx.als_data.gain == 0) ||
		(ctx->alg_ctx.als_data.atime_ms == 0) ||
		(ctx->alg_ctx.als_data.gain_ir == 0)) {
		hwlog_err("%s, gain atime gain_ir err para\n", __func__);
		return;
	}
	if (chip->cali_ctx.cal_state < CAL_STATE_GAIN_3) {
		chip->cali_ctx.cal_raw_count++;
		chip->cali_ctx.cal_x_raw += out.raw_x;
		chip->cali_ctx.cal_y_raw += out.raw_y;
		chip->cali_ctx.cal_z_raw += out.raw_z;
		chip->cali_ctx.cal_ir_raw += out.raw_ir;
		if (chip->cali_ctx.cal_raw_count < ROHM_BH1749_CAL_AVERAGE)
			return;
		// init cali para
		cali_tar_temp.high_th = HIGH_THRESHOLD;
		cali_tar_temp.low_th = LOW_THRESHOLD;
		cali_tar_temp.again = curr_gain;
		cali_tar_temp.atime_ms = ctx->alg_ctx.als_data.atime_ms;
		bh1749_als_cali_all_rgbc(&chip->cali_ctx,
			chip->cali_ctx.cal_state, &cali_tar_temp);

		// here add cal process
		chip->cali_ctx.cal_state++;
		chip->cali_ctx.cal_raw_count = 0;
		chip->cali_ctx.cal_x_raw = 0;
		chip->cali_ctx.cal_y_raw = 0;
		chip->cali_ctx.cal_z_raw = 0;
		chip->cali_ctx.cal_ir_raw = 0;

		if (chip->cali_ctx.cal_state >= CAL_STATE_GAIN_3)
			return;

		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_DISABLE);
		ctx->alg_ctx.als_data.gain =
			bh1749_als_gain_conv[chip->cali_ctx.cal_state];
		ctx->alg_ctx.als_data.gain_ir =
			bh1749_als_gain_conv[chip->cali_ctx.cal_state];
		bh1749_set_gain(chip->device_ctx,
			ctx->alg_ctx.als_data.gain / BH1749_GAIN_SCALE);
		bh1749_set_ir_gain(chip->device_ctx,
			ctx->alg_ctx.als_data.gain_ir / BH1749_GAIN_SCALE);
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_ENABLE);

	} else {
		if (color_calibrate_result == true) {
			save_cal_para_to_nv(chip);
			report_calibrate_result = true;
		} else {
			hwlog_err("color_calibrate_result fail\n");
			report_calibrate_result = false;
		}
		chip->in_cal_mode = false;
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_DISABLE);
		bh1749_set_gain(chip->device_ctx, GAIN_X32);
		bh1749_set_ir_gain(chip->device_ctx, GAIN_X32);
		if (enable_status_before_calibrate == 1)
			bh1749_als_set_enable(chip, ROHMDRIVER_ALS_ENABLE);
		else
			hwlog_info("color sensor disabled before calibrate\n");
		hwlog_info("osal_calHandl_als: done\n");
	}
}

static void osal_report_als(struct color_chip *chip)
{
	struct export_als_data_t out;
	uint8_t curr_gain_idx = 0;
	uint32_t curr_gain = 0;
	struct bh1749_ctx_t *ctx = NULL;

	if (!chip || !chip->device_ctx) {
		hwlog_err("ROHM_Driver: %s: Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->device_ctx;
	bh1749_get_als_data(ctx, &out);
	hwlog_debug("raw data: x = %d, y = %d, z = %d, ir = %d\n",
		out.raw_x, out.raw_y, out.raw_z, out.raw_ir);
	curr_gain = ctx->alg_ctx.als_data.gain;

	if (curr_gain < bh1749_als_gain_conv[CAL_STATE_GAIN_2])
		curr_gain_idx = bh1749_gain_to_reg(curr_gain);
	else
		curr_gain_idx = CAL_STATE_GAIN_2;

	out.raw_ir *= color_nv_para.cal_ir_ratio[curr_gain_idx];
	out.raw_ir /= ROHM_BH1749_FLOAT_TO_FIX;
	out.raw_x *= color_nv_para.cal_x_ratio[curr_gain_idx];
	out.raw_x /= ROHM_BH1749_FLOAT_TO_FIX;
	out.raw_y *= color_nv_para.cal_y_ratio[curr_gain_idx];
	out.raw_y /= ROHM_BH1749_FLOAT_TO_FIX;
	out.raw_z *= color_nv_para.cal_z_ratio[curr_gain_idx];
	out.raw_z /= ROHM_BH1749_FLOAT_TO_FIX;

	hwlog_debug("report data before cali x = %d, y = %d, z = %d, ir = %d\n",
		out.raw_x, out.raw_y, out.raw_z, out.raw_ir);
	if ((ctx->alg_ctx.als_data.gain == 0) ||
		(ctx->alg_ctx.als_data.gain_ir == 0) ||
		(ctx->alg_ctx.als_data.atime_ms == 0))
		return;
	// rohm raw data transform
	out.raw_x = RAW_DATA_TRANS(out.raw_x, ctx->alg_ctx.als_data.gain,
		ctx->alg_ctx.als_data.atime_ms);
	out.raw_y = RAW_DATA_TRANS(out.raw_y, ctx->alg_ctx.als_data.gain,
		ctx->alg_ctx.als_data.atime_ms);
	out.raw_z = RAW_DATA_TRANS(out.raw_z, ctx->alg_ctx.als_data.gain,
		ctx->alg_ctx.als_data.atime_ms);
	out.raw_ir = RAW_DATA_TRANS(out.raw_ir, ctx->alg_ctx.als_data.gain,
		ctx->alg_ctx.als_data.atime_ms);

	// adjust the report data when the calibrate ratio is acceptable
	hwlog_debug("report data after tran: x = %d, y = %d, z = %d, ir = %d\n",
		out.raw_x, out.raw_y, out.raw_z, out.raw_ir);

	// copy to report_value
	report_value[0] = (int)out.raw_x;
	report_value[1] = (int)out.raw_y;
	report_value[2] = (int)out.raw_z;
	report_value[3] = (int)out.raw_ir;

	// report to up level
	bh1749_report_data(report_value);
	report_logcount++;
	if (report_logcount < ROHM_REPORT_LOG_COUNT_NUM)
		return;
	hwlog_info("bh1749 report data %d, %d, %d, %d\n", report_value[0],
		report_value[1], report_value[2], report_value[3]);
	hwlog_info("bh1749: als_gains[%d], x y z ir = %d, %d, %d, %d\n",
		bh1749_als_gains[curr_gain_idx],
		color_nv_para.cal_x_ratio[curr_gain_idx],
		color_nv_para.cal_y_ratio[curr_gain_idx],
		color_nv_para.cal_z_ratio[curr_gain_idx],
		color_nv_para.cal_ir_ratio[curr_gain_idx]);
	hwlog_info("bh1749: atime[%d], rgb_gain[%d], ir_gain[%d]\n",
		(UINT16)ctx->alg_ctx.als_data.atime_ms,
		(UINT32)ctx->alg_ctx.als_data.gain,
		(UINT32)ctx->alg_ctx.als_data.gain_ir);
	report_logcount = 0;
}

int rohm_bh1749_setenable(bool enable)
{
	struct color_chip *chip = p_chip;

	if (!chip)
		return 0;

	if (enable)
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_ENABLE);
	else
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_DISABLE);

	return 1;
}
EXPORT_SYMBOL_GPL(rohm_bh1749_setenable);

void rohm_show_calibrate(struct color_chip *chip,
	struct color_sensor_output_t *out)
{
	unsigned int i;

	if (!out || !chip) {
		hwlog_err("rohm show_calibrate input para NULL\n");
		return;
	}

	if (chip->in_cal_mode == false)
		hwlog_err("rohm show_calibrate not in calibration mode\n");

	out->result = (UINT32)report_calibrate_result;
	hwlog_warn("color_calibrate_show result = %d\n", out->result);
	memcpy(out->report_ir, chip->cali_ctx.cal_ir_rst,
		sizeof(out->report_ir));
	memcpy(out->report_x, chip->cali_ctx.cal_x_rst, sizeof(out->report_x));
	memcpy(out->report_y, chip->cali_ctx.cal_y_rst, sizeof(out->report_y));
	memcpy(out->report_z, chip->cali_ctx.cal_z_rst, sizeof(out->report_z));

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		hwlog_warn("color_calibrate_show i = %d: %d,%d,%d,%d\n", i,
			out->report_x[i], out->report_y[i],
			out->report_z[i], out->report_ir[i]);
		hwlog_warn("cali_ctx i = %d: %d,%d,%d,%d\n", i,
			chip->cali_ctx.cal_x_rst[i],
			chip->cali_ctx.cal_y_rst[i],
			chip->cali_ctx.cal_z_rst[i],
			chip->cali_ctx.cal_ir_rst[i]);
	}
}

void rohm_store_calibrate(struct color_chip *chip,
	struct color_sensor_input_t *in)
{
	struct bh1749_ctx_t *ctx = NULL;
	enum bh1749_rohm_mode_t mode = 0;

	if (!chip || !in || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (in->enable && chip->in_cal_mode) {
		hwlog_err("store_calibrate: Already in calibration mode\n");
		return;
	}

	if (in->enable) {
		ctx = chip->device_ctx;
		hwlog_warn("rohm store_calibrate: starting calibration mode\n");
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
		hwlog_warn("cal target cali x = %d, y = %d, z = %d, ir = %d\n",
			in->tar_x, in->tar_y, in->tar_z, in->tar_ir);

		ctx->alg_ctx.als_data.gain =
			bh1749_als_gain_conv[CAL_STATE_GAIN_1];
		ctx->alg_ctx.als_data.gain_ir =
			bh1749_als_gain_conv[CAL_STATE_GAIN_1];

		bh1749_get_mode(chip->device_ctx, &mode);
		if ((mode & ROHM_BH1749_MODE_ALS) == ROHM_BH1749_MODE_ALS) {
			// enabled before calibrate
			enable_status_before_calibrate = 1;
			hwlog_warn("%s: enabled before calibrate\n", __func__);
			bh1749_als_set_enable(chip, ROHMDRIVER_ALS_DISABLE);
			msleep(10); // sleep 10 ms to make sure disable timer
		} else {
			// disabled before calibrate
			enable_status_before_calibrate = 0;
			hwlog_warn("%s: disabled before calibrate\n", __func__);
		}
		chip->in_cal_mode = true;
		// make the calibrate_result true for calibrate again
		color_calibrate_result = true;
		bh1749_set_gain(chip->device_ctx,
			ctx->alg_ctx.als_data.gain / BH1749_GAIN_SCALE);
		bh1749_set_ir_gain(chip->device_ctx,
			ctx->alg_ctx.als_data.gain_ir / BH1749_GAIN_SCALE);
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_ENABLE);
	} else {
		hwlog_warn("rohm store_calibrate: stopping calibration mode\n");
		chip->in_cal_mode = false;
	}
}

void rohm_show_enable(struct color_chip *chip, int *state)
{
	enum bh1749_rohm_mode_t mode = ROHM_BH1749_MODE_OFF;

	if (!chip || !state) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	bh1749_get_mode(chip->device_ctx, &mode);
	if (mode & ROHM_BH1749_MODE_ALS)
		*state = 1;
	else
		*state = 0;
}

void rohm_store_enable(struct color_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (state)
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_ENABLE);
	else
		bh1749_als_set_enable(chip, ROHMDRIVER_ALS_DISABLE);
}
#endif

static void rohmdriver_work(struct work_struct *work)
{
	bool re_enable = false;
	enum bh1749_rohm_mode_t mode = ROHM_BH1749_MODE_OFF;
	struct color_chip *chip = NULL;

	if (!work) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct color_chip, als_work);
	if (!chip) {
		hwlog_err("%s: Pointer chip is NULL\n", __func__);
		return;
	}

	ROHM_MUTEX_LOCK(&chip->lock);
	if (read_nv_first_in == 0) {
		(void)get_cal_para_from_nv();
		read_nv_first_in = -1; // -1: do not read again.
	}

	hwlog_info("rohm driver_work\n");
	re_enable = bh1749_event_handler((struct bh1749_ctx_t *)chip->device_ctx,
		chip->in_cal_mode);
	hwlog_info("rohm_bh1749: re_enable = %d\n", re_enable);

	if (bh1749_get_rgb_avai((struct bh1749_ctx_t *)chip->device_ctx) &
		BH1749_FEATURE_ALS) {
		if (chip->in_cal_mode == false) {
			hwlog_info("rohm driver_work: osal_report_als\n");
			osal_report_als(chip);
		} else {
			hwlog_warn("rohm driver_work: calibration mode\n");
			bh1749_cal_als(chip);
		}
	}

	bh1749_get_mode(chip->device_ctx, &mode);
	if (mode)
		mod_timer(&chip->work_timer,
			jiffies + msecs_to_jiffies(ROHM_POLL_TIME));

	ROHM_MUTEX_UNLOCK(&chip->lock);
}

#ifdef CONFIG_HUAWEI_DSM
static void rohmdriver_dmd_work(struct work_struct *work)
{
	if (color_devcheck_dmd_result == false) {
		if (dsm_client_ocuppy(shb_dclient))
			return;
		dsm_client_record(shb_dclient,
			"ap_color_sensor_detected fail\n");
		dsm_client_notify(shb_dclient,
			DSM_AP_ERR_COLORSENSOR_DETECT);
		hwlog_err("color sensor bh1749 detected fail\n");
	}
}
#endif

int rohmdriver_probe(struct i2c_client *client,
	const struct i2c_device_id *idp)
{
	int ret = -1;
	unsigned int i = 0;
	UINT8 device = 0;
	struct bh1749_device_info_t rohm_info;
	struct device *dev = NULL;
	static struct color_chip *chip = NULL;
	struct driver_i2c_platform_data *pdata = NULL;

	if (!client) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	dev = &client->dev;
	if (!dev) {
		hwlog_err("%s: dev Pointer is NULL\n", __func__);
		return -1;
	}
	pdata = dev->platform_data;
	hwlog_warn("rohm_bh1749: rohmdriver probe\n");
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "%s: i2c smbus byte data unsupported\n", __func__);
		ret = -EOPNOTSUPP;
		goto init_failed;
	}

	chip = kzalloc(sizeof(struct color_chip), GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto init_failed;
	}

	mutex_init(&chip->lock);
	chip->client = client;
	chip->pdata = pdata;
	i2c_set_clientdata(chip->client, chip);

	chip->in_suspend = 0;
	chip->in_cal_mode = false;
	chip->cali_ctx.cal_state = 0;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		chip->cali_ctx.cal_x_rst[i] = ROHM_BH1749_FLOAT_TO_FIX;
		chip->cali_ctx.cal_y_rst[i] = ROHM_BH1749_FLOAT_TO_FIX;
		chip->cali_ctx.cal_z_rst[i] = ROHM_BH1749_FLOAT_TO_FIX;
		chip->cali_ctx.cal_ir_rst[i] = ROHM_BH1749_FLOAT_TO_FIX;
		color_nv_para.cal_x_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
		color_nv_para.cal_y_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
		color_nv_para.cal_z_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
		color_nv_para.cal_ir_ratio[i] = ROHM_BH1749_FLOAT_TO_FIX;
	}

#ifdef CONFIG_HUAWEI_DSM
	INIT_DELAYED_WORK(&rohm_dmd_work, rohmdriver_dmd_work);
#endif
	device = bh1749_get_devid(chip->client);

	if (device == ROHM_BH1749_ID) {
		hwlog_info("bh1749_get_devid success\n");
	} else {
#ifdef CONFIG_HUAWEI_DSM
		color_devcheck_dmd_result = false;
		schedule_delayed_work(&rohm_dmd_work,
			msecs_to_jiffies(AP_COLOR_DMD_DELAY_TIME_MS));
#endif
		hwlog_err("bh1749_get_devid FAILED: ROHM_UNKNOWN_DEVICE\n");
		goto  malloc_failed;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_AP_COLOR_SENSOR);
#endif

	bh1749_get_dev_info(&rohm_info);
	chip->device_ctx = kzalloc(rohm_info.memory_size, GFP_KERNEL);
	if (chip->device_ctx) {
		bh1749_device_init(chip->device_ctx, chip->client);
	} else {
		hwlog_err("rohm bh1749 kzalloc failed\n");
		ret = -ENOMEM;
		goto malloc_failed;
	}

	hwlog_info("Setup for ALS\n");
	init_timer(&chip->work_timer);
	setup_timer(&chip->work_timer, bh1749_als_timer_handle,
		(uintptr_t)chip);
	INIT_WORK(&chip->als_work, rohmdriver_work);

	chip->color_show_calibrate_state = rohm_show_calibrate;
	chip->color_store_calibrate_state = rohm_store_calibrate;
	chip->color_enable_show_state = rohm_show_enable;
	chip->color_enable_store_state = rohm_store_enable;
	chip->color_sensor_get_gain = bh1749_get_gain;
	chip->color_sensor_set_gain = bh1749_set_gain;
	chip->color_report_type = bh1749_rgb_report_type;

	p_chip = chip;
	ret = color_register(chip);
	if (ret < 0) {
		hwlog_err("rohm_bh1749: color_register fail \n");
	}
	color_default_enable = rohm_bh1749_setenable;
	hwlog_info("rohm Probe ok.\n");
	return 0;

malloc_failed:
	kfree(chip);
init_failed:
	hwlog_err("rohm Probe failed.\n");
	return ret;
}

int rohmdriver_suspend(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return -1;
	}
	hwlog_info("%s\n", __func__);
	ROHM_MUTEX_LOCK(&chip->lock);
	chip->in_suspend = 1;
	if (chip->wake_irq)
		irq_set_irq_wake(chip->client->irq, 1);
	else if (!chip->unpowered)
		hwlog_info("powering off\n");
	ROHM_MUTEX_UNLOCK(&chip->lock);
	return 0;
}

int rohmdriver_resume(struct device *dev)
{
	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	dev_get_drvdata(dev);
	return 0;
}

int rohmdriver_remove(struct i2c_client *client)
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
	i2c_set_clientdata(client, NULL);
	if (chip->device_ctx)
		kfree(chip->device_ctx);
	if (chip)
		kfree(chip);
	return 0;
}

static struct i2c_device_id rohmdriver_idtable[] = {
	{ "rohm_bh1749", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, rohmdriver_idtable);

static const struct dev_pm_ops rohmdriver_pm_ops = {
	.suspend = rohmdriver_suspend,
	.resume = rohmdriver_resume,
};

static const struct of_device_id rohmdriver_of_id_table[] = {
	{ .compatible = "rohm,bh1749" },
	{},
};


static struct i2c_driver rohmdriver_driver = {
	.driver = {
		.name = "rohm_bh1749",
		.owner = THIS_MODULE,
		.of_match_table = rohmdriver_of_id_table,
	},
	.id_table = rohmdriver_idtable,
	.probe = rohmdriver_probe,
	.remove = rohmdriver_remove,
};

static int __init rohmdriver_init(void)
{
	int rc;

	hwlog_info("rohm_rohm bh1749: init\n");

	rc = i2c_add_driver(&rohmdriver_driver);

	hwlog_info("rohm_bh1749: %d", rc);
	return rc;
}

static void __exit rohmdriver_exit(void)
{
	hwlog_info("rohm_rohm bh1749: exit\n");
	i2c_del_driver(&rohmdriver_driver);
}

module_init(rohmdriver_init);
module_exit(rohmdriver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei rohm color sensor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
