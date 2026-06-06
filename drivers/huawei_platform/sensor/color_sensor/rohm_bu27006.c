/*
 * rohm_bu27006.c
 *
 * code for rohm bu27006 sensor
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include "rohm_bu27006.h"
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/version.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG color_sensor
HWLOG_REGIST();

extern int color_report_val[MAX_REPORT_LEN];
extern int (*color_default_enable)(bool enable);
extern UINT32 g_flicker_support;

static DEFINE_MUTEX(fifo_mutex);
static int als_first_data_cnt;
static uint8_t als_report_logcount;
static uint32_t flc_report_logcount;
static struct colorDriver_chip *p_chip;
static int report_value[ROHM_REPORT_DATA_LEN];
static int read_nv_first_in;
static bool color_calibrate_result;
static bool report_calibrate_result;
static int enable_status_before_calibrate;
static color_sensor_cali_para_nv color_nv_para;

static uint32_t const bu27006_als_gains[BU27006_RGB_GAIN_SIZE] = {
	1,
	4,
	32,
	128
};

static int bu27006_rgb_report_type(void)
{
	return AWB_SENSOR_RAW_SEQ_TYPE_R_G_B_IR;
}

static void bu27006_als_report_fac_data(void)
{
	uint8_t i;

	// copy to fac color_report_val
	color_report_val[0] = BU27006_RGB_GAIN_SIZE;
	for (i = 1; i <= ROHM_REPORT_DATA_LEN; i++)
		color_report_val[i] = report_value[i - 1];
}

static void bu27006_als_reset_report_data(void)
{
	uint8_t i;

	color_report_val[0] = BU27006_RGB_GAIN_SIZE;
	for (i = 1; i < MAX_REPORT_LEN; i++)
		color_report_val[i] = -1; // set -1 for invalid rgbc data
}

static int32_t bu27006_set_als_gain(void *p, int gain)
{
	int ret;
	uint8_t gain_data = 0;
	struct bu27006_ctx *ctx = NULL;

	if (!p)
		return -EFAULT;
	ctx = p;

	hwlog_info("%s = %d\n", __func__, gain);

	ret = color_sensor_get_byte(ctx->handle, BU27006_RGB_GAIN_TIME,
		&gain_data);
	if (ret < 0)
		return ret;

	if (gain == GAIN_X1) {
		gain_data = (gain_data & RGB_GAIN_MASK) | RGB_GAIN_X1;
		ctx->alg_ctx.gain = GAIN_X1;
		ctx->alg_ctx.gain_ir = GAIN_X1;
	} else if (gain == GAIN_X4) {
		gain_data = (gain_data & RGB_GAIN_MASK) | RGB_GAIN_X4;
		ctx->alg_ctx.gain = GAIN_X4;
		ctx->alg_ctx.gain_ir = GAIN_X4;
	} else if (gain == GAIN_X32) {
		gain_data = (gain_data & RGB_GAIN_MASK) | RGB_GAIN_X32;
		ctx->alg_ctx.gain = GAIN_X32;
		ctx->alg_ctx.gain_ir = GAIN_X32;
	} else if (gain == GAIN_X128) {
		gain_data = (gain_data & RGB_GAIN_MASK) | RGB_GAIN_X128;
		ctx->alg_ctx.gain = GAIN_X128;
		ctx->alg_ctx.gain_ir = GAIN_X128;
	} else {
		hwlog_warn("bu27006 Invalid gain\n");
		return -EFAULT;
	}

	color_sensor_set_byte(ctx->handle, BU27006_RGB_GAIN_TIME, gain_data);
	return 0;
}

static uint8_t bu27006_get_chip_id(struct i2c_client *client)
{
	uint8_t chip_id = 0;

	if (!client) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return 0;
	}

	(void)color_sensor_get_byte(client, BU27006_MANUFACT_ID, &chip_id);
	return chip_id;
}

static int bu27006_get_als_gain(void *ctx)
{
	uint8_t gain_data = 0;
	uint8_t gain_idx;

	if (!ctx)
		return -EFAULT;

	(void)color_sensor_get_byte(((struct bu27006_ctx *)ctx)->handle,
		BU27006_RGB_GAIN_TIME, &gain_data);

	gain_idx = RGB_GAIN_VALUE(gain_data);
	if (gain_idx < BU27006_RGB_GAIN_SIZE) {
		hwlog_info("%s = %d\n", __func__, bu27006_als_gains[gain_idx]);
		return bu27006_als_gains[gain_idx];
	}
	return 0;
}

static void bu27006_flc_fifo_init(struct bu27006_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	hwlog_info("%s\n", __func__);

	memset(&ctx->flc_ctx.flc_data, 0, sizeof(ctx->flc_ctx.flc_data));
	ctx->flc_ctx.fifo_full_flag = false;
	ctx->flc_ctx.gain_changed = false;
	ctx->flc_ctx.fifo_cnt = 0;
}

static void bu27006_set_flc_gain(struct bu27006_ctx *ctx, uint8_t gain)
{
	uint8_t flc_gain = 0;
	int rc;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	hwlog_info("%s = %d\n", __func__, gain);
	rc = color_sensor_get_byte(ctx->handle, BU27006_FLC_GAIN, &flc_gain);
	if (rc < 0)
		return;

	if (gain == GAIN_X1) {
		flc_gain = (flc_gain & FLC_GAIN_MASK) | FLC_GAIN_X1;
		ctx->flc_ctx.gain_flc = GAIN_X1;
	} else if (gain == GAIN_X2) {
		flc_gain = (flc_gain & FLC_GAIN_MASK) | FLC_GAIN_X2;
		ctx->flc_ctx.gain_flc = GAIN_X2;
	} else if (gain == GAIN_X4) {
		flc_gain = (flc_gain & FLC_GAIN_MASK) | FLC_GAIN_X4;
		ctx->flc_ctx.gain_flc =  GAIN_X4;
	} else if (gain == GAIN_X8) {
		flc_gain = (flc_gain & FLC_GAIN_MASK) | FLC_GAIN_X8;
		ctx->flc_ctx.gain_flc = GAIN_X8;
	} else if (gain == GAIN_X16) {
		flc_gain = (flc_gain & FLC_GAIN_MASK) | FLC_GAIN_X16;
		ctx->flc_ctx.gain_flc = GAIN_X16;
	} else if (gain == GAIN_X32) {
		flc_gain = (flc_gain & FLC_GAIN_MASK) | FLC_GAIN_X32;
		ctx->flc_ctx.gain_flc = GAIN_X32;
	} else {
		hwlog_err("bu27006 Invalid flc_gain\n");
		return;
	}

	(void)color_sensor_set_byte(ctx->handle, BU27006_FLC_GAIN, flc_gain);
}

static void bu27006_set_flc_hz_mode(struct bu27006_ctx *ctx,uint8_t work_mode)
{
	uint8_t mode_data = 0;
	int rc;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	hwlog_info("%s = %d\n", __func__, work_mode);
	rc = color_sensor_get_byte(ctx->handle, BU27006_FLC_GAIN, &mode_data);
	if (rc < 0)
		return;

	if (work_mode == BU27006_1K_MODE) {
		mode_data = (mode_data & FLC_MODE_MASK) | FLC_MODE_1KHZ;
		ctx->flc_ctx.mode_flc = BU27006_1K_MODE;
	} else if (work_mode == BU27006_2K_MODE) {
		mode_data = (mode_data & FLC_MODE_MASK) | FLC_MODE_2KHZ;
		ctx->flc_ctx.mode_flc = BU27006_2K_MODE;
	} else {
		hwlog_err("bu27006 Invalid flc_work_mode\n");
		return;
	}

	(void)color_sensor_set_byte(ctx->handle, BU27006_FLC_GAIN, mode_data);
}

static void bu27006_set_als_atime(struct bu27006_ctx *ctx, int atime_ms)
{
	uint8_t atime = 0;

	if (!ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	color_sensor_get_byte(ctx->handle, BU27006_RGB_GAIN_TIME, &atime);

	if (atime_ms == TIME_55MS) {
		atime = (atime & TIME_GAIN_MASK) | MEASURE_55MS;
		ctx->alg_ctx.atime_ms = TIME_55MS;
	} else if (atime_ms == TIME_100MS) {
		atime = (atime & TIME_GAIN_MASK) | MEASURE_100MS;
		ctx->alg_ctx.atime_ms = TIME_100MS;
	} else {
		hwlog_err("bu27006 Invalid atime_ms\n");
		return;
	}

	color_sensor_set_byte(ctx->handle, BU27006_RGB_GAIN_TIME, atime);
}

static void bu27006_reset_regs(struct bu27006_ctx *ctx,
	struct i2c_client *handle)
{
	if (!handle || !ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	hwlog_info("%s\n", __func__);

	// reset all register
	color_sensor_set_byte(handle, BU27006_RESET, SW_RESET);

	// set rgb agin to default 4X
	bu27006_set_als_gain(ctx, RGB_GAIN_DEFAULT);

	// set 32x and 1khz for flicker
	bu27006_set_flc_gain(ctx, FLC_GAIN_DEFAULT);
	bu27006_set_flc_hz_mode(ctx, FLC_MODE_DEFAULT);

	// set 55ms default
	bu27006_set_als_atime(ctx, RGB_TIME_DEFAULT);
}

static bool bu27006_flc_saturation_check(struct bu27006_ctx *ctx, int curr_fifo)
{
	uint32_t max_thr = BU27006_FLC_1K_HIGH_LEVEL;

	if (!ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return false;
	}

	if (ctx->flc_ctx.mode_flc == BU27006_1K_MODE)
		max_thr = BU27006_FLC_1K_HIGH_LEVEL;
	else if (ctx->flc_ctx.mode_flc == BU27006_2K_MODE)
		max_thr = BU27006_FLC_2K_HIGH_LEVEL;

	if (curr_fifo > max_thr)
		return true;

	return false;
}

static bool bu27006_flc_insufficience_check(int curr_fifo)
{
	if (curr_fifo < BU27006_FLC_LOW_LEVEL)
		return true;

	return false;
}

static bool bu27006_als_saturation_check(struct adc_data_t *curr_raw)
{
	if (!curr_raw) {
		hwlog_err("bu27006 NULL pointer!\n");
		return false;
	}

	if ((curr_raw->r > ROHM_BU27006_HIGH_LEVEL) ||
		(curr_raw->g > ROHM_BU27006_HIGH_LEVEL) ||
		(curr_raw->b > ROHM_BU27006_HIGH_LEVEL))
		return true;

	return false;
}

static bool bu27006_als_insufficience_check(struct adc_data_t *curr_raw)
{
	if (!curr_raw) {
		hwlog_err("bu27006 NULL pointer\n");
		return false;
	}

	if (((curr_raw->r < ROHM_BU27006_LOW_LEVEL) ||
		(curr_raw->g < ROHM_BU27006_LOW_LEVEL) ||
		(curr_raw->b < ROHM_BU27006_LOW_LEVEL)) &&
		((curr_raw->r + curr_raw->g + curr_raw->b) <
		ROHM_BU27006_LOW_SUM_LEVEL))
		return true;

	return false;
}

static void bu27006_als_agc(struct bu27006_ctx *ctx,
	struct adc_data_t *adc_data)
{
	uint8_t target_gain = 0;
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;

	if (!ctx || !adc_data) {
		hwlog_err("%s NULL pointer\n", __func__);
		return;
	}

	// AGC: Adjust rgb gain setting
	if (bu27006_als_saturation_check(adc_data)) {
		if (ctx->alg_ctx.gain > GAIN_X32) // current is 128X
			target_gain = GAIN_X32;
		else if (ctx->alg_ctx.gain > GAIN_X4) // current is 32X
			target_gain = GAIN_X4;
		else if (ctx->alg_ctx.gain > GAIN_X1) // current is 4X
			target_gain = GAIN_X1;
	} else if (bu27006_als_insufficience_check(adc_data)) {
		if (ctx->alg_ctx.gain < GAIN_X4) // curr is 128X
			target_gain = GAIN_X4;
		else if (ctx->alg_ctx.gain < GAIN_X32) // curr is 32X
			target_gain = GAIN_X32;
		else if (ctx->alg_ctx.gain < GAIN_X128) // curr is 4X
			target_gain = GAIN_X128;
	}
	if (target_gain) {
		bu27006_get_flc_mode(ctx, &mode);
		if (mode & BU27006_MODE_ON) {
			hwlog_info("read fifo before als_gain change\n");
			bu27006_get_flc_data(ctx);
		}

		hwlog_info("%s curr gain = %d change to %d\n", __func__,
			ctx->alg_ctx.gain, target_gain);
		bu27006_set_als_gain(ctx, target_gain);
	}

	// return true if gain changed
	ctx->alg_ctx.gain_changed = ((target_gain == 0) ? false : true);
}

static void bu27006_flc_agc(struct bu27006_ctx *ctx)
{
	uint8_t target_gain = 0;
	uint8_t tmp[2] = {0};
	uint16_t flc_data;
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;

	if (!ctx) {
		hwlog_err("%s NULL pointer!\n", __func__);
		return;
	}

	// fix enable interruput bug for als first data
	bu27006_get_als_mode(ctx, &mode);
	if ((mode & BU27006_MODE_ON) && (als_first_data_cnt == 0)) {
		hwlog_info("stop flc_gain change before als first data report");
		return;
	}
	color_sensor_read_fifo(ctx->handle, BU27006_FLICKER_DATA, tmp, 2);
	flc_data = COMB_WORD(tmp[1], tmp[0]);
	if ((flc_report_logcount % FLC_LOG_REPORT_NUM) == 0)
		hwlog_info("bu27006 flc_data=%d, gain_flc = %d\n",
			flc_data, ctx->flc_ctx.gain_flc);

	// AGC: Adjust flicker gain setting
	if (bu27006_flc_saturation_check(ctx, flc_data)) {
		if (ctx->flc_ctx.gain_flc > GAIN_X16) // current is 32X
			target_gain = GAIN_X16;
		else if (ctx->flc_ctx.gain_flc > GAIN_X8) // current is 16X
			target_gain = GAIN_X8;
		else if (ctx->flc_ctx.gain_flc > GAIN_X4) // current is 8X
			target_gain = GAIN_X4;
		else if (ctx->flc_ctx.gain_flc > GAIN_X2) // current is 4X
			target_gain = GAIN_X2;
		else if (ctx->flc_ctx.gain_flc > GAIN_X1) // current is 2X
			target_gain = GAIN_X1;
	} else if (bu27006_flc_insufficience_check(flc_data)) {
		if (ctx->flc_ctx.gain_flc < GAIN_X2) // current is 1X
			target_gain = GAIN_X2;
		else if (ctx->flc_ctx.gain_flc < GAIN_X4) // current is 2X
			target_gain = GAIN_X4;
		else if (ctx->flc_ctx.gain_flc < GAIN_X8) // current is 128X
			target_gain = GAIN_X8;
		else if (ctx->flc_ctx.gain_flc < GAIN_X16) // current is 32X
			target_gain = GAIN_X16;
		else if (ctx->flc_ctx.gain_flc < GAIN_X32) // current is 4X
			target_gain = GAIN_X32;
	}

	if (target_gain) {
		hwlog_info("bu27006 curr_flc_gain %d, change to %d\n",
			ctx->flc_ctx.gain_flc, target_gain);
		bu27006_set_flc_gain(ctx, target_gain);
	}

	// return true if gain changed
	ctx->flc_ctx.gain_changed = ((target_gain == 0) ? false : true);
}

static void bu27006_handle_als(struct bu27006_ctx *ctx)
{
	uint8_t adc[BU27006_ADC_BYTES] = {0};
	uint8_t valid_data = 0;

	// read valid status
	color_sensor_get_byte(ctx->handle, BU27006_ENABLE, &valid_data);

	if (valid_data & RGBC_VALID_HIGH)
		ctx->alg_ctx.rgbc_valid |= true;

	if (ctx->alg_ctx.rgbc_valid) {
		// read ADC data
		(void)color_sensor_read_fifo(ctx->handle, BU27006_RED_DATA,
			adc, BU27006_ADC_BYTES);

		ctx->alg_ctx.data.r = COMB_WORD(adc[1], adc[0]);
		ctx->alg_ctx.data.g = COMB_WORD(adc[3], adc[2]);
		ctx->alg_ctx.data.b = COMB_WORD(adc[5], adc[4]);
		ctx->alg_ctx.data.ir = COMB_WORD(adc[7], adc[6]);
		ctx->alg_ctx.rgbc_valid = false;

		hwlog_debug("%s r=%d, g=%d, b=%d, ir=%d\n", __func__,
			ctx->alg_ctx.data.r, ctx->alg_ctx.data.g,
			ctx->alg_ctx.data.b, ctx->alg_ctx.data.ir);
	} else {
		hwlog_warn("%s als data not valid\n", __func__);
		return;
	}

	// set available every time
	// because timer is more than 55ms, our mesaure time is 55ms
	ctx->update |= BU27006_RGBC_READY;
}

static int bu27006_get_flc_data(struct bu27006_ctx *ctx)
{
	uint8_t fifo_level = 0;
	uint32_t i = 0;
	uint32_t tmp;
	uint16_t fifo_cnt;
	unsigned char read_fifo[BU27006_FIFO_LEVEL_FULL << 1] = {0};
	unsigned short raw_fifo[BU27006_FIFO_LEVEL_FULL] = {0};

	// read fifo_level
	color_sensor_get_byte(ctx->handle, BU27006_FIFO_LEVEL, &fifo_level);
	if ((fifo_level == 0) || (fifo_level > BU27006_FIFO_LEVEL_FULL)) {
		hwlog_err("bu27006 fifo_level = %d, return\n", fifo_level);
		return -EINVAL;
	}

	// read fifo data, one fifo data has 2 byte
	color_sensor_read_fifo(ctx->handle, BU27006_FIFO_DATA_LOW, read_fifo,
		fifo_level << 1);
	memcpy((unsigned char *)raw_fifo, &read_fifo, sizeof(read_fifo));
	if (unlikely(ctx->flc_ctx.gain_flc == 0))
		return -EINVAL;

	mutex_lock(&fifo_mutex);
	// update to fifo_tmp
	fifo_cnt = ctx->flc_ctx.fifo_cnt;
	while (i < fifo_level) {
		ctx->flc_ctx.flc_data.fifo_tmp[fifo_cnt] =
			(int)raw_fifo[i] * FLC_TRANSFER_COFF /
				ctx->flc_ctx.gain_flc;
		fifo_cnt++;
		i++;
		if (fifo_cnt >= ROHM_FLICKER_FIFO_SIZE) {
			fifo_cnt = fifo_cnt % ROHM_FLICKER_FIFO_SIZE;
			ctx->flc_ctx.fifo_full_flag = true;
		}
	}
	ctx->flc_ctx.fifo_cnt = fifo_cnt;
	// update to fifo_data
	if (ctx->flc_ctx.fifo_full_flag == false) {
		// fifo_tmp is not full, it just copy to fifo_data directly
		memcpy((uint8_t *)ctx->flc_ctx.flc_data.fifo_data,
			(uint8_t *)ctx->flc_ctx.flc_data.fifo_tmp,
			sizeof(ctx->flc_ctx.flc_data.fifo_tmp));
	} else {
		// fifo_tmp is already full, it should copy one by one
		i = 0;
		while (i < ROHM_FLICKER_FIFO_SIZE) {
			tmp = (fifo_cnt + i) % ROHM_FLICKER_FIFO_SIZE;
			ctx->flc_ctx.flc_data.fifo_data[i] =
			ctx->flc_ctx.flc_data.fifo_tmp[tmp];
			i++;
		}
	}
	mutex_unlock(&fifo_mutex);

	// debug flc data
	if ((flc_report_logcount % FLC_LOG_REPORT_NUM) == 0) {
		for (i = 0; i < ROHM_FLICKER_FIFO_SIZE; i++)
			hwlog_info("bu27006 fifo_data[i=%3d] = %d\n",
				i, ctx->flc_ctx.flc_data.fifo_data[i]);
	}

	return 0;
}

static void bu27006_handle_flc(struct bu27006_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	// flc agc
	bu27006_flc_agc(ctx);

	// re-initialize fifo data
	if (ctx->flc_ctx.gain_changed) {
		ctx->flc_ctx.gain_changed = false;
		hwlog_warn("bu27006 flc_gain changed, reset fifo\n");

		mutex_lock(&fifo_mutex);
		bu27006_flc_fifo_init(ctx);
		mutex_unlock(&fifo_mutex);
	}

	// handle fifo
	bu27006_get_flc_data(ctx);
}

static void bu27006_get_als_mode(struct bu27006_ctx *ctx,
	enum bu27006_power_mode_t *mode)
{
	if (!ctx || !mode) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	*mode = ctx->als_mode;
}

static void bu27006_set_als_mode(struct bu27006_ctx *ctx,
	enum bu27006_power_mode_t mode)
{
	if (!ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	ctx->als_mode = mode;
}

static void bu27006_get_flc_mode(struct bu27006_ctx *ctx,
	enum bu27006_power_mode_t *mode)
{
	if (!ctx || !mode) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	*mode = ctx->flc_mode;
}

static void bu27006_set_flc_mode(struct bu27006_ctx *ctx,
	enum bu27006_power_mode_t mode)
{
	if (!ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	ctx->flc_mode = mode;
}

static uint32_t bu27006_get_als_avail(struct bu27006_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return 0;
	}
	return ctx->update;
}

static void bu27006_get_raw_als(struct bu27006_ctx *ctx,
	struct adc_data_t *export_data)
{
	if (!ctx || !export_data) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}
	ctx->update &= ~(BU27006_RGBC_READY);

	memcpy(export_data, &ctx->alg_ctx.data, sizeof(ctx->alg_ctx.data));
}

static void bu27006_device_init(struct bu27006_ctx *ctx,
	struct i2c_client *port)
{
	if (!port || !ctx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	ctx->handle = port;
	ctx->als_mode = BU27006_MODE_OFF;
	ctx->flc_mode = BU27006_MODE_OFF;
	ctx->device_id = bu27006_get_chip_id(ctx->handle);
	bu27006_reset_regs(ctx, port);
}

static void bu27006_als_timer_wrk(unsigned long data)
{
	struct colorDriver_chip *chip = (struct colorDriver_chip *)data;

	if (!chip) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	schedule_work(&chip->als_work);
}

static void bu27006_flc_timer_wrk(unsigned long data)
{
	struct colorDriver_chip *chip = (struct colorDriver_chip *)data;

	if (!chip) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->fd_work);
}

static ssize_t bu27006_set_als_enable(struct colorDriver_chip *chip,
	uint8_t enable)
{
	uint8_t reg_en = 0;
	struct bu27006_ctx *ctx = NULL;

	if (!chip || !chip->deviceCtx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return -EINVAL;
	}

	ctx = chip->deviceCtx;
	color_sensor_get_byte(ctx->handle, BU27006_ENABLE, &reg_en);

	if (enable) {
		reg_en = reg_en | RGBC_EN_ON;
		color_sensor_set_byte(chip->client, BU27006_ENABLE, reg_en);

		// first enable set the timer as 60ms
		mod_timer(&chip->work_timer,
			jiffies + msecs_to_jiffies(ROHM_ALS_FIRST_POLL_TIME));
		bu27006_set_als_mode(ctx, BU27006_MODE_ON);
		bu27006_als_reset_report_data();
		ctx->alg_ctx.rgbc_valid = false;
		ctx->update = 0;
		als_report_logcount = ROHM_REPORT_LOG_COUNT_NUM;
		als_first_data_cnt = 0;
	} else {
		reg_en = reg_en & (~RGBC_EN_ON);
		color_sensor_set_byte(chip->client, BU27006_ENABLE, reg_en);
		bu27006_set_als_mode(ctx, BU27006_MODE_OFF);
	}

	hwlog_info("%s = %d\n", __func__, enable);
	return 0;
}

static void bu27006_set_flc_enable(struct colorDriver_chip *chip, uint8_t en)
{
	UINT8 reg_en = 0;
	struct bu27006_ctx *ctx = NULL;

	if (!chip || !chip->deviceCtx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}
	ctx = chip->deviceCtx;

	color_sensor_get_byte(chip->client, BU27006_ENABLE, &reg_en);
	if (en) {
		reg_en = reg_en | FLC_EN_ON;
		color_sensor_set_byte(chip->client, BU27006_ENABLE, reg_en);

		mod_timer(&chip->fd_timer,
			jiffies + msecs_to_jiffies(ROHM_FLC_POLL_TIME));
		bu27006_set_flc_mode(ctx, BU27006_MODE_ON);
		bu27006_flc_fifo_init(ctx);
		flc_report_logcount = 0;
	} else {
		reg_en = reg_en & (~FLC_EN_ON);
		color_sensor_set_byte(chip->client, BU27006_ENABLE, reg_en);
		bu27006_set_flc_mode(ctx, BU27006_MODE_OFF);
	}

	hwlog_info("%s = %d\n", __func__, en);
}

static void get_cal_para_from_nv(void)
{
	uint32_t i = 0;
	int ret = 0;
	uint32_t cal_ratio[ROHM_REPORT_DATA_LEN];

	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
	if (ret < 0)
		hwlog_err("%s fail, use default para\n", __func__);

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		hwlog_info("%s gain[%d]: [%d, %d, %d, %d]\n", __func__, i,
			color_nv_para.calXratio[i], color_nv_para.calYratio[i],
			color_nv_para.calZratio[i], color_nv_para.calIratio[i]);
	}

	// only use 32x gain val for cal rgbc
	cal_ratio[0] = color_nv_para.calXratio[CAL_STATE_GAIN_3];
	cal_ratio[1] = color_nv_para.calYratio[CAL_STATE_GAIN_3];
	cal_ratio[2] = color_nv_para.calZratio[CAL_STATE_GAIN_3];
	cal_ratio[3] = color_nv_para.calIratio[CAL_STATE_GAIN_3];

	for (i = 0; i < ROHM_REPORT_DATA_LEN; i++) {
		if ((cal_ratio[i] > BU27006_DEF_GAIN_THR_HIGH) ||
			(cal_ratio[i] < BU27006_DEF_GAIN_THR_LOW)) {
			color_nv_para.calXratio[i] = BU27006_DEFAULT_CAL_RATIO;
			color_nv_para.calYratio[i] = BU27006_DEFAULT_CAL_RATIO;
			color_nv_para.calZratio[i] = BU27006_DEFAULT_CAL_RATIO;
			color_nv_para.calIratio[i] = BU27006_DEFAULT_CAL_RATIO;
		}
	}
}

static void save_cal_para_to_nv(struct colorDriver_chip *chip)
{
	int i;

	if (!chip) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	color_nv_para.nv_Xtarget = chip->calibrationCtx.calXtarget;
	color_nv_para.nv_Ytarget = chip->calibrationCtx.calYtarget;
	color_nv_para.nv_Ztarget = chip->calibrationCtx.calZtarget;
	color_nv_para.nv_IRtarget = chip->calibrationCtx.calIRtarget;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		color_nv_para.calXratio[i] = chip->calibrationCtx.calXresult[i];
		color_nv_para.calYratio[i] = chip->calibrationCtx.calYresult[i];
		color_nv_para.calZratio[i] = chip->calibrationCtx.calZresult[i];
		color_nv_para.calIratio[i] = chip->calibrationCtx.calIRresult[i];
		hwlog_info("%s gain[%d]: [%d, %d, %d, %d]\n", __func__, i,
			color_nv_para.calXratio[i], color_nv_para.calYratio[i],
			color_nv_para.calZratio[i], color_nv_para.calIratio[i]);
	}

	write_color_calibrate_data_to_nv(RGBAP_CALI_DATA_NV_NUM,
		RGBAP_CALI_DATA_SIZE, "RGBAP", (char *)&color_nv_para);
}

static int bu27006_als_cali_rgb(struct rgb_cali_tar_t *cali_val)
{
	uint32_t result;

	result = cali_val->rawdata / BU27006_CAL_AVG;

	// cant devide by zero, use the default calibration scaling factor
	if (result == 0) {
		color_calibrate_result = false;
		return BU27006_DEFAULT_CAL_RATIO;
	}

	// transfer to 32X gain 50ms atime
	result = cali_val->rawdata * DATA_TRANSFER_COFF / cali_val->again /
		cali_val->atime_ms;
	if (result == 0) {
		color_calibrate_result = false;
		return BU27006_DEFAULT_CAL_RATIO;
	}
	result = cali_val->target_val * ROHM_BU27006_FLOAT_TO_FIX / result;
	if ((result > cali_val->high_th) || (result < cali_val->low_th)) {
		hwlog_info("%s(): ratio is out bound[%d, %d]! result = %d\n",
			__func__, cali_val->low_th, cali_val->high_th, result);
		color_calibrate_result = false;
	}
	return result;
}

static void bu27006_als_cali_all_rgbc(struct color_sensor_calibration *cal_raw,
	enum color_sensor_cal_states cal_idx, struct rgb_cali_tar_t *cali_tar)
{
	cali_tar->high_th = bu27006_gain_thr[cal_idx].high_thr;
	cali_tar->low_th = bu27006_gain_thr[cal_idx].low_thr;

	// X cali
	cali_tar->rawdata = cal_raw->calXsample;
	cali_tar->target_val = cal_raw->calXtarget;
	cal_raw->calXresult[cal_idx] = bu27006_als_cali_rgb(cali_tar);

	// Y cali
	cali_tar->rawdata = cal_raw->calYsample;
	cali_tar->target_val = cal_raw->calYtarget;
	cal_raw->calYresult[cal_idx] = bu27006_als_cali_rgb(cali_tar);

	// Z cali
	cali_tar->rawdata = cal_raw->calZsample;
	cali_tar->target_val = cal_raw->calZtarget;
	cal_raw->calZresult[cal_idx] = bu27006_als_cali_rgb(cali_tar);

	// IR cali
	cali_tar->rawdata = cal_raw->calIRsample;
	cali_tar->target_val = cal_raw->calIRtarget;
	cal_raw->calIRresult[cal_idx] = bu27006_als_cali_rgb(cali_tar);

	hwlog_warn("%s, result X = %d, Y = %d, Z = %d, IR = %d\n", __func__,
		cal_raw->calXresult[cal_idx], cal_raw->calYresult[cal_idx],
		cal_raw->calZresult[cal_idx], cal_raw->calIRresult[cal_idx]);
}

static void bu27006_als_cali(struct colorDriver_chip *chip)
{
	struct adc_data_t out_data;
	struct bu27006_ctx *ctx = NULL;
	uint32_t curr_gain;
	enum color_sensor_cal_states cal_idx;
	struct rgb_cali_tar_t cali_tar_temp;

	if (!chip || !chip->deviceCtx) {
		hwlog_info("%s Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->deviceCtx;
	curr_gain = ctx->alg_ctx.gain;
	cal_idx = chip->calibrationCtx.calState;
	bu27006_get_raw_als(chip->deviceCtx, &out_data);

	if ((ctx->alg_ctx.gain == 0) ||	(ctx->alg_ctx.atime_ms == 0)) {
		hwlog_info("%s gain atime err para\n", __func__);
		return;
	}
	hwlog_info("%s: cal_idx = %d, count = %d, gain = %d\n", __func__,
		cal_idx, chip->calibrationCtx.calSampleCounter, curr_gain);

	if (cal_idx < CAL_STATE_GAIN_5) {
		chip->calibrationCtx.calSampleCounter++;
		chip->calibrationCtx.calXsample += out_data.r;
		chip->calibrationCtx.calYsample += out_data.g;
		chip->calibrationCtx.calZsample += out_data.b;
		chip->calibrationCtx.calIRsample += out_data.ir;
		if (chip->calibrationCtx.calSampleCounter < BU27006_CAL_AVG)
			return;

		// init cali para
		cali_tar_temp.high_th = bu27006_gain_thr[cal_idx].high_thr;
		cali_tar_temp.low_th = bu27006_gain_thr[cal_idx].low_thr;
		cali_tar_temp.again = ctx->alg_ctx.gain;
		cali_tar_temp.atime_ms = ctx->alg_ctx.atime_ms;

		bu27006_als_cali_all_rgbc(&chip->calibrationCtx, cal_idx,
			&cali_tar_temp);

		chip->calibrationCtx.calState++;
		chip->calibrationCtx.calSampleCounter = 0;
		chip->calibrationCtx.calXsample = 0;
		chip->calibrationCtx.calYsample = 0;
		chip->calibrationCtx.calZsample = 0;
		chip->calibrationCtx.calIRsample = 0;

		if (chip->calibrationCtx.calState < CAL_STATE_GAIN_5) {
			hwlog_warn("%s continue calibrate\n", __func__);
			bu27006_set_als_enable(chip, ROHMDRIVER_ALS_DISABLE);
			msleep(10);
			ctx->alg_ctx.gain =
				bu27006_als_gains[chip->calibrationCtx.calState];
			bu27006_set_als_gain(ctx, ctx->alg_ctx.gain);
			bu27006_set_als_enable(chip, ROHMDRIVER_ALS_ENABLE);
		}
	} else {
		if (color_calibrate_result == true) {
			save_cal_para_to_nv(chip);
			report_calibrate_result = true;
		} else {
			hwlog_err("%s calibrate fail\n", __func__);
			report_calibrate_result = false;
		}
		chip->inCalMode = false;
		bu27006_set_als_enable(chip, ROHMDRIVER_ALS_DISABLE);
		bu27006_set_als_gain(chip->deviceCtx, GAIN_X32);
		if (enable_status_before_calibrate == 1)
			bu27006_set_als_enable(chip, ROHMDRIVER_ALS_ENABLE);
		else
			hwlog_warn("bu27006 disabled before calibrate\n");
		hwlog_warn("bu27006 osal_cali_als: done\n");
	}
}

static void bu27006_als_report(struct colorDriver_chip *chip)
{
	struct adc_data_t out_data;
	struct rawdata_arg_t rgb_rawdata;
	struct bu27006_ctx *ctx = NULL;

	if (!chip || !chip->deviceCtx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}
	ctx = chip->deviceCtx;
	bu27006_get_raw_als(ctx, &out_data);

	// this is used AGC
	rgb_rawdata.red = out_data.r;
	rgb_rawdata.green = out_data.g;
	rgb_rawdata.blue = out_data.b;
	rgb_rawdata.ir = out_data.ir;

	rgb_rawdata.red *= color_nv_para.calXratio[CAL_STATE_GAIN_3];
	rgb_rawdata.red /= ROHM_BU27006_FLOAT_TO_FIX;
	rgb_rawdata.green *= color_nv_para.calYratio[CAL_STATE_GAIN_3];
	rgb_rawdata.green /= ROHM_BU27006_FLOAT_TO_FIX;
	rgb_rawdata.blue *= color_nv_para.calZratio[CAL_STATE_GAIN_3];
	rgb_rawdata.blue /= ROHM_BU27006_FLOAT_TO_FIX;
	rgb_rawdata.ir *= color_nv_para.calIratio[CAL_STATE_GAIN_3];
	rgb_rawdata.ir /= ROHM_BU27006_FLOAT_TO_FIX;

	if ((ctx->alg_ctx.gain == 0) || (ctx->alg_ctx.atime_ms == 0))
		return;

	// rohm raw data transform
	rgb_rawdata.red = rgb_rawdata.red * DATA_TRANSFER_COFF /
		ctx->alg_ctx.gain / ctx->alg_ctx.atime_ms;
	rgb_rawdata.green = rgb_rawdata.green * DATA_TRANSFER_COFF /
		ctx->alg_ctx.gain / ctx->alg_ctx.atime_ms;
	rgb_rawdata.blue = rgb_rawdata.blue * DATA_TRANSFER_COFF /
		ctx->alg_ctx.gain / ctx->alg_ctx.atime_ms;
	rgb_rawdata.ir = rgb_rawdata.ir * DATA_TRANSFER_COFF /
		ctx->alg_ctx.gain / ctx->alg_ctx.atime_ms;

	// copy to report_value
	report_value[0] = (int)rgb_rawdata.red;
	report_value[1] = (int)rgb_rawdata.green;
	report_value[2] = (int)rgb_rawdata.blue;
	report_value[3] = (int)rgb_rawdata.ir;

	// report to up level
	bu27006_als_report_fac_data();
	ap_color_report(report_value, ROHM_REPORT_DATA_LEN * sizeof(int));

	if (als_first_data_cnt == 0)
		als_first_data_cnt = 1; // report first rgbc data

	als_report_logcount++;
	if (als_report_logcount >= ROHM_REPORT_LOG_COUNT_NUM) {
		hwlog_info("bu27006 report data %d, %d, %d, %d\n",
			report_value[0], report_value[1], report_value[2],
			report_value[3]);
		hwlog_info("gain[%d], X[%d], Y[%d], Z[%d], Ir[%d], atime[%d]\n",
			ctx->alg_ctx.gain,
			color_nv_para.calXratio[CAL_STATE_GAIN_3],
			color_nv_para.calYratio[CAL_STATE_GAIN_3],
			color_nv_para.calZratio[CAL_STATE_GAIN_3],
			color_nv_para.calIratio[CAL_STATE_GAIN_3],
			ctx->alg_ctx.atime_ms);
		als_report_logcount = 0;
	}

	// als agc
	bu27006_als_agc(ctx, &out_data);
}

int bu27006_enable_rgbc(bool enable)
{
	struct colorDriver_chip *chip = p_chip;

	if (!chip)
		return -EINVAL;

	if (enable) {
		// set 4X default
		bu27006_set_als_gain(chip->deviceCtx, RGB_GAIN_DEFAULT);
		// set 55ms default
		bu27006_set_als_atime(chip->deviceCtx, RGB_TIME_DEFAULT);
		// enable sensor
		bu27006_set_als_enable(chip, ROHMDRIVER_ALS_ENABLE);
	} else {
		// disable sensor
		bu27006_set_als_enable(chip, ROHMDRIVER_ALS_DISABLE);
	}

	hwlog_info("%s = %d success\n", __func__, enable);
	return 0;
}
EXPORT_SYMBOL_GPL(bu27006_enable_rgbc);

static void bu27006_als_show_calibrate(struct colorDriver_chip *chip,
	at_color_sensor_output_para *out_para)
{
	int32_t i;

	if (!out_para || !chip) {
		hwlog_err("%s input para NULL\n", __func__);
		return;
	}

	if (chip->inCalMode == false)
		hwlog_warn("%s, but not in calibration mode\n", __func__);

	out_para->result = (uint32_t)report_calibrate_result;
	hwlog_info("%s result = %d\n", __func__, out_para->result);
	out_para->gain_arr = CAL_STATE_GAIN_5; // is 4 gain type
	out_para->color_arr = ROHM_REPORT_DATA_LEN;

	memcpy(out_para->report_gain, bu27006_als_gains,
		sizeof(bu27006_als_gains));
	memcpy(out_para->report_raw[0], chip->calibrationCtx.calIRresult,
		sizeof(out_para->report_raw[0]));
	memcpy(out_para->report_raw[1], chip->calibrationCtx.calXresult,
		sizeof(out_para->report_raw[1]));
	memcpy(out_para->report_raw[2], chip->calibrationCtx.calYresult,
		sizeof(out_para->report_raw[2]));
	memcpy(out_para->report_raw[3], chip->calibrationCtx.calZresult,
		sizeof(out_para->report_raw[3]));

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		hwlog_info("%s i = %d: %d, %d, %d, %d,%d\n", __func__, i,
		out_para->report_raw[0][i], out_para->report_raw[1][i],
		out_para->report_raw[2][i], out_para->report_raw[3][i],
		out_para->report_raw[4][i]);
	}
}

static void bu27006_als_store_calibrate(struct colorDriver_chip *chip,
	at_color_sensor_input_para *at_in_para)
{
	struct bu27006_ctx *ctx = NULL;
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;
	struct color_sensor_input in_para;

	if (!chip || !at_in_para || !chip->deviceCtx) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	in_para.enable = at_in_para->enable;
	in_para.tar_x = at_in_para->reserverd[0];
	in_para.tar_y = at_in_para->reserverd[1];
	in_para.tar_z = at_in_para->reserverd[2];
	in_para.tar_ir = at_in_para->reserverd[3];

	if (in_para.enable && chip->inCalMode) {
		hwlog_err("%s already in calibration mode\n", __func__);
		return;
	}
	if (!in_para.enable) {
		hwlog_warn("%s stopping calibration mode\n", __func__);
		chip->inCalMode = false;
		return;
	}

	ctx = chip->deviceCtx;
	hwlog_info("%s start calibration mode\n", __func__);
	memset(&chip->calibrationCtx, 0, sizeof(chip->calibrationCtx));
	// calculate targer for gain 1x (assuming its set at 64x)
	chip->calibrationCtx.calXtarget = in_para.tar_x;
	chip->calibrationCtx.calYtarget = in_para.tar_y;
	chip->calibrationCtx.calZtarget = in_para.tar_z;
	chip->calibrationCtx.calIRtarget = in_para.tar_ir;
	chip->calibrationCtx.calState = CAL_STATE_GAIN_1;
	hwlog_info("%s cali target x = %d, y = %d, z = %d, ir = %d\n",
		__func__, in_para.tar_x, in_para.tar_y,
		in_para.tar_z, in_para.tar_ir);

	ctx->alg_ctx.gain = bu27006_als_gains[CAL_STATE_GAIN_1];

	bu27006_get_als_mode(chip->deviceCtx, &mode);
	if ((mode & BU27006_MODE_ON) == BU27006_MODE_ON) {
		enable_status_before_calibrate = 1; // disable before calibrate
		hwlog_info("%s enabled before calibrate\n", __func__);
		bu27006_set_als_enable(chip, ROHMDRIVER_ALS_DISABLE);
		msleep(10); // sleep 10 ms to make sure disable timer
	} else {
		enable_status_before_calibrate = 0; // disabled before calibrate
		hwlog_info("%s disabled before calibrate\n", __func__);
	}
	chip->inCalMode = true;
	// make the calibrate_result true for calibrate again
	color_calibrate_result = true;
	bu27006_set_als_gain(chip->deviceCtx, ctx->alg_ctx.gain);
	bu27006_set_als_enable(chip, ROHMDRIVER_ALS_ENABLE);
}

static void bu27006_als_show_enable(struct colorDriver_chip *chip, int *state)
{
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;

	if (!chip || !state) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	bu27006_get_als_mode(chip->deviceCtx, &mode);
	if (mode & BU27006_MODE_ON)
		*state = 1;
	else
		*state = 0;
}

static void bu27006_als_store_enable(struct colorDriver_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&chip->lock);
	if (state) {
		// set 32X default
		bu27006_set_als_gain(chip->deviceCtx, RGB_GAIN_DEFAULT);

		// set 55ms default
		bu27006_set_als_atime(chip->deviceCtx, RGB_TIME_DEFAULT);

		// enable rgb sensor
		bu27006_set_als_enable(chip, ROHMDRIVER_ALS_ENABLE);
	} else {
		bu27006_set_als_enable(chip, ROHMDRIVER_ALS_DISABLE);
	}
	mutex_unlock(&chip->lock);
}

static void bu27006_flc_show_enable(struct colorDriver_chip *chip, int *state)
{
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;

	if (!chip || !state) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	bu27006_get_flc_mode(chip->deviceCtx, &mode);
	if (mode & BU27006_MODE_ON)
		*state = 1;
	else
		*state = 0;
}

static void bu27006_flc_store_enable(struct colorDriver_chip *chip, int en)
{
	if (!chip) {
		hwlog_err("%s pointer is NULL\n", __func__);
		return;
	}

	if (en) {
		// set 32x and 1khz for flicker as default enable
		bu27006_set_flc_gain(chip->deviceCtx, FLC_GAIN_DEFAULT);
		bu27006_set_flc_hz_mode(chip->deviceCtx, FLC_MODE_DEFAULT);
		bu27006_set_flc_enable(chip, ROHMDRIVER_FLC_ENABLE);
	} else {
		bu27006_set_flc_enable(chip, ROHMDRIVER_FLC_DISABLE);
	}
}

static void bu27006_als_work(struct work_struct *work)
{
	struct bu27006_ctx *ctx = NULL;
	struct colorDriver_chip *chip = NULL;
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;

	if (!work) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	chip = container_of(work, struct colorDriver_chip, als_work);
	if (!chip || !chip->deviceCtx) {
		hwlog_err("%s Pointer chip is NULL\n", __func__);
		return;
	}
	ctx = chip->deviceCtx;

	if (read_nv_first_in == 0) {
		get_cal_para_from_nv();
		read_nv_first_in = -1; // -1: do not read again.
	}

	bu27006_handle_als(ctx);

	if (bu27006_get_als_avail(ctx) & BU27006_RGBC_READY) {
		if (chip->inCalMode == false)
			bu27006_als_report(chip);
		else
			bu27006_als_cali(chip);
	}

	bu27006_get_als_mode(ctx, &mode);
	if (mode)
		// timer set as 105ms
		mod_timer(&chip->work_timer,
			jiffies + msecs_to_jiffies(ROHM_ALS_NORMAL_POLL_TIME));
	else
		hwlog_warn("bu27006 timer already stopped\n");
}

static void bu27006_flc_work(struct work_struct *work)
{
	enum bu27006_power_mode_t mode = BU27006_MODE_OFF;
	struct colorDriver_chip *chip = NULL;

	if (!work) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	chip = container_of(work, struct colorDriver_chip, fd_work);
	if (!chip) {
		hwlog_err("%s chip Pointer is NULL\n", __func__);
		return;
	}

	flc_report_logcount++;
	// print raw_fifo
	if ((flc_report_logcount % FLC_LOG_REPORT_NUM) == 0)
		hwlog_info("rohm_bu27006_flc_work\n");

	mutex_lock(&chip->lock);
	bu27006_handle_flc(chip->deviceCtx);
	bu27006_get_flc_mode(chip->deviceCtx, &mode);
	if (mode)
		mod_timer(&chip->fd_timer,
			jiffies + msecs_to_jiffies(ROHM_FLC_POLL_TIME));
	else
		hwlog_warn("bu27006 flc_timer stopped\n");
	mutex_unlock(&chip->lock);

	flc_report_logcount = flc_report_logcount % FLC_LOG_REPORT_NUM;
}

static void bu27006_get_fifo_data(struct colorDriver_chip *chip,
	char *fifo_data)
{
	struct bu27006_ctx *ctx = NULL;

	if (!chip || !chip->deviceCtx || !fifo_data) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->deviceCtx;
	mutex_lock(&fifo_mutex);
	memcpy(fifo_data, (uint8_t *)ctx->flc_ctx.flc_data.fifo_data,
		sizeof(ctx->flc_ctx.flc_data.fifo_data));
	mutex_unlock(&fifo_mutex);
}

static char *bu27006_chip_name(void)
{
	return "rohm_bu27006";
}

static int bu27006_probe(struct i2c_client *client,
	const struct i2c_device_id *idp)
{
	int ret;
	int i;
	uint8_t device_id;
	struct device *dev = NULL;
	//static struct colorDriver_chip *chip;
	struct driver_i2c_platform_data *pdata = NULL;

	hwlog_info("rohm_bu27006_probe\n");
	if (!client) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return -EINVAL;
	}

	dev = &client->dev;
	if (!dev) {
		hwlog_err("%s dev Pointer is NULL\n", __func__);
		return -EINVAL;
	}
	pdata = dev->platform_data;
	color_notify_support();
	if (!i2c_check_functionality(client->adapter,
		I2C_FUNC_SMBUS_BYTE_DATA)) {
		hwlog_err("%s i2c smbus byte data unsupported\n", __func__);
		ret = -EOPNOTSUPP;
		goto init_failed;
	}

	p_chip = kzalloc(sizeof(struct colorDriver_chip), GFP_KERNEL);
	if (!p_chip) {
		ret = -ENOMEM;
		goto init_failed;
	}

	mutex_init(&p_chip->lock);
	p_chip->client = client;
	p_chip->pdata = pdata;
	i2c_set_clientdata(p_chip->client, p_chip);

	p_chip->in_suspend = 0;
	p_chip->inCalMode = false;
	p_chip->calibrationCtx.calState = 0;

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		color_nv_para.calXratio[i] = BU27006_DEFAULT_CAL_RATIO;
		color_nv_para.calYratio[i] = BU27006_DEFAULT_CAL_RATIO;
		color_nv_para.calZratio[i] = BU27006_DEFAULT_CAL_RATIO;
		color_nv_para.calIratio[i] = BU27006_DEFAULT_CAL_RATIO;
	}

	device_id = bu27006_get_chip_id(p_chip->client);
	if (device_id == BU27006_WHO_AM_I) {
		hwlog_info("bu27006 check success\n");
	} else {
		hwlog_err("%s FAILED: ROHM_UNKNOWN_DEVICE\n", __func__);
		goto malloc_failed;
	}

	p_chip->deviceCtx = kzalloc(sizeof(struct bu27006_ctx), GFP_KERNEL);
	if (p_chip->deviceCtx) {
		bu27006_device_init(p_chip->deviceCtx, p_chip->client);
	} else {
		hwlog_err("rohm bu27006 kzalloc failed\n");
		ret = -ENOMEM;
		goto id_failed;
	}

	init_timer(&p_chip->work_timer);
	setup_timer(&p_chip->work_timer, bu27006_als_timer_wrk,
		(unsigned long)p_chip);
	INIT_WORK(&p_chip->als_work, bu27006_als_work);

	init_timer(&p_chip->fd_timer);
	setup_timer(&p_chip->fd_timer, bu27006_flc_timer_wrk,
		(unsigned long)p_chip);
	INIT_WORK(&p_chip->fd_work, bu27006_flc_work);

	// init common func for rgb & flk
	p_chip->at_color_show_calibrate_state = bu27006_als_show_calibrate;
	p_chip->at_color_store_calibrate_state = bu27006_als_store_calibrate;
	p_chip->color_enable_show_state = bu27006_als_show_enable;
	p_chip->color_enable_store_state = bu27006_als_store_enable;
	p_chip->flicker_enable_show_state = bu27006_flc_show_enable;
	p_chip->flicker_enable_store_state = bu27006_flc_store_enable;
	p_chip->color_sensor_getGain = bu27006_get_als_gain;
	p_chip->color_sensor_setGain = bu27006_set_als_gain;
	p_chip->get_flicker_data = bu27006_get_fifo_data;
	p_chip->color_report_type = bu27006_rgb_report_type;
	p_chip->color_chip_name = bu27006_chip_name;

	ret = color_register(p_chip);
	if (ret < 0)
		hwlog_err("%s color_register fail\n", __func__);

	color_default_enable = bu27006_enable_rgbc;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_AP_COLOR_SENSOR);
#endif
	g_flicker_support = 1; // support flicker
	hwlog_info("%s success\n", __func__);
	return 0;

id_failed:
	if (p_chip->deviceCtx)
		kfree(p_chip->deviceCtx);
	p_chip->deviceCtx = NULL;
	i2c_set_clientdata(client, NULL);

malloc_failed:
	kfree(p_chip);
	p_chip = NULL;
init_failed:
	hwlog_err("%s Probe failed\n", __func__);
	color_notify_absent();
	return ret;
}

static int bu27006_suspend(struct device *dev, pm_message_t mesg)
{
	struct colorDriver_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return -EINVAL;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s chip Pointer is NULL\n", __func__);
		return -EINVAL;
	}
	hwlog_warn("%s\n", __func__);
	return 0;
}

static int bu27006_resume(struct device *dev)
{
	struct colorDriver_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return -EINVAL;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s chip Pointer is NULL\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s\n", __func__);

	return 0;
}

static int bu27006_remove(struct i2c_client *client)
{
	struct colorDriver_chip *chip = NULL;

	if (!client) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return -EINVAL;
	}

	chip = i2c_get_clientdata(client);
	if (!chip) {
		hwlog_err("%s Pointer is NULL\n", __func__);
		return -EINVAL;
	}

	i2c_set_clientdata(client, NULL);
	if (chip->deviceCtx)
		kfree(chip->deviceCtx);

	kfree(chip);

	return 0;
}

static struct i2c_device_id rohm_idtable[] = {
	{ "rohm_bu27006", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, rohm_idtable);

static const struct of_device_id rohm_of_id_table[] = {
	{ .compatible = "rohm,bu27006" },
	{},
};

static struct i2c_driver rohm_bu27006_driver = {
	.driver = {
		.suspend = bu27006_suspend,
		.resume = bu27006_resume,
		.name = "rohm_bu27006_drv",
		.owner = THIS_MODULE,
		.of_match_table = rohm_of_id_table,
	},
	.id_table = rohm_idtable,
	.probe = bu27006_probe,
	.remove = bu27006_remove,
};

static int __init rohm_bu27006_init(void)
{
	int rc;
	struct device_node *np = NULL;

	hwlog_warn("%s\n", __func__);
	np = of_find_node_by_name(NULL, "bu27006");
	if (!np) {
		hwlog_err("bu27006 node NOT found\n");
		return -EINVAL;
	}

	rc = i2c_add_driver(&rohm_bu27006_driver);
	hwlog_info("%s %d\n", __func__, rc);
	return rc;
}

static void __exit rohm_bu27006_exit(void)
{
	hwlog_warn("%s\n", __func__);
	i2c_del_driver(&rohm_bu27006_driver);
}

module_init(rohm_bu27006_init);
module_exit(rohm_bu27006_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei rohm color sensor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
