/*
 * ams_as7341.c
 *
 * code for AMS as7341 sensor
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * Description: code for AMS as7341 sensor
 * Author: sunwendong
 * Create: 2019-10-18
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
#include "ams_as7341.h"

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


// defines for special tests
// define EXTENDED_LOG for log test
// define TEST_COMPARE_SMUX_CONFIG for Test

#define HWLOG_TAG color_sensor
HWLOG_REGIST();

static bool fd_gain_adjusted;
static bool fd_timer_stopped;
static bool color_calibrate_result = true;
static bool report_calibrate_result;
static bool skip_first_rgb_data;
static u16 ring_buffer[MAX_BUFFER_SIZE];
static u32 read_out_buffer[MAX_BUFFER_SIZE];
static u16 *buffer_ptr = &ring_buffer[0];
static u16 *head_ptr = &ring_buffer[0];
static u16 *tail_ptr = &ring_buffer[0];
static u8 first_circle_end;
static DEFINE_MUTEX(ring_buffer_mutex);
static DEFINE_MUTEX(enable_handle_mutex);
static struct color_chip *p_chip;
static int enable_status_before_cali;
static UINT8 report_logcount;
static UINT32 algo_support;
static u32 max_flicker;
static UINT8 channel_block;
static UINT8 smux_buffer[SMUX_NUM][SMUX_BUF_LEN];
static UINT16 highest_digits[SMUX_NUM];

extern int color_report_val[MAX_REPORT_LEN];
extern int (*color_default_enable)(bool enable);
extern UINT32 flicker_support;
static void read_fd_data(struct color_chip *chip);

static uint8_t find_highest_bit(uint32_t value)
{
	uint8_t i;

	if (value == 0)
		return 0;

	for (i = INTEGER_BIT_SIZE - 1; i >= 0; i--)
		if ((value >> i) & 1)
			return i;
	return 0;
}

static void as7341_get_optimized_gain(UINT32 maximum_adc, UINT32 highest_adc,
	UINT8 *p_gain, UINT8 *p_saturation)
{
	UINT32 gain_change;

	if (highest_adc == 0)
		highest_adc = 1;

	if (highest_adc >= maximum_adc) {
		/* saturation detected */
		if (*p_gain > LOW_AUTO_GAIN_VALUE)
			*p_gain /= AUTO_GAIN_DIVIDER;
		else
			*p_gain = LOWEST_GAIN_ID;
		*p_saturation = IS_SATURATION;
	} else {
		/* value too low, increase the gain to 80% maximum */
		gain_change = (SATURATION_LOW_PERCENT * maximum_adc) /
			(SATURATION_HIGH_PERCENT * highest_adc);
		if (gain_change == 0 && *p_gain != 0) {
			(*p_gain)--;
		} else {
			gain_change = find_highest_bit(gain_change);
			if (((uint32_t)(*p_gain) + gain_change) > MAX_GAIN_ID)
				*p_gain = MAX_GAIN_ID;
			else
				*p_gain += (uint8_t)gain_change;
		}
		*p_saturation = !IS_SATURATION;
	}
}

static void as7341_get_field(struct i2c_client *handle, UINT8 reg, UINT8 *data,
	UINT8 mask)
{
	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	color_sensor_get_byte(handle, reg, data);
	*data &= mask;
}

static void as7341_set_field(struct i2c_client *handle, UINT8 reg, UINT8 data,
	UINT8 mask)
{
	int write_count;
	UINT8 original_data = 0;
	UINT8 new_data;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&enable_handle_mutex);
	color_sensor_get_byte(handle, reg, &original_data);
	new_data = original_data & (~mask);
	new_data |= (data & mask);
	write_count = color_sensor_set_byte(handle, reg, new_data);
	mutex_unlock(&enable_handle_mutex);
}

static int as7341_rgb_report_type(void)
{
	return AWB_SENSOR_RAW_SEQ_TYPE_MULTISPECTRAL;
}

static int as7341_report_data(int value[])
{
	return ap_color_report(value, AMS_REPORT_DATA_LEN * sizeof(int));
}

static enum as7341_sensor_id_t as7341_get_chipid(struct i2c_client *handle)
{
	UINT8 chipid = 0;
	UINT8 i;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return AMS_UNKNOWN_DEVICE;
	}
	color_sensor_get_byte(handle, AS7341_ID_REG, &chipid);

	for (i = 0; i < ARRAY_SIZE(as7341_ids); i++) {
		if (as7341_ids[i] == chipid) {
			hwlog_debug("get this chip info\n");
			return AMS_AS7341_REV0;
		}
	}
	return AMS_UNKNOWN_DEVICE;
}

static void as7341_log_revid(struct i2c_client *handle)
{
	UINT8 revid = AMS_UNKNOWN_REV;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	color_sensor_get_byte(handle, AS7341_REVID_REG, &revid);
	hwlog_debug("%s get revid before = 0x%x\n", __func__, revid);
}

static INT32 as7341_als_get_gain(const void *ctx)
{
	UINT8 cfg1_reg = 0;
	INT32 gain = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	as7341_get_field(((struct as7341_ctx *)ctx)->handle,
		AS7341_CFG1_REG, &cfg1_reg, AGAIN_MASK);

	if (cfg1_reg < ARRAY_SIZE(as7341_als_gain))
		gain = as7341_als_gain[cfg1_reg];

	hwlog_info("now the gain val = %d\n", gain);
	return gain;
}

static void as7341_als_set_gain(struct as7341_ctx *p_as7341, UINT8 gain_id)
{
	UINT32 gain_value = 0;

	if (!p_as7341) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (gain_id > MAX_GAIN_ID) {
		hwlog_err("%s: gain_id out of range: %u\n", __func__, gain_id);
		return;
	}

	if (gain_id == LOWEST_GAIN_ID)
		gain_value = LOWEST_GAIN_VALUE;
	else
		gain_value = (1 << (gain_id - 1)) * AS7341_GAIN_SCALE;

	color_sensor_set_byte(p_as7341->handle, AS7341_CFG1_REG, gain_id);
	p_as7341->alg_ctx.gain_id = gain_id;
	p_as7341->alg_ctx.gain = gain_value;
	hwlog_info("%s gain = %d, id = %u\n", __func__, gain_value, gain_id);
}

static UINT8 as7341_als_get_saved_gain(struct as7341_ctx *p_as7341)
{
	if (!p_as7341) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	return p_as7341->alg_ctx.gain_id;
}

static void as7341_als_set_itime(struct as7341_ctx *p_as7341, int itime_ms)
{
	INT32 atime;

	if (!p_as7341) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	// NOTE: ITIME_MS = (ATIME + 1) * (ASTEP + 1) * 2.78us
	// 2.78 means 2000 / 720
	// included rounding  ((value * 10) + 5) / 10
	atime = ((((itime_ms * MS_2_US * ITIME_DIVIDER / ITIME_MULTIPLIER /
		(p_as7341->alg_ctx.astep + 1)) * ROUNDING_SCALAR) +
		ROUNDING_ADD) / ROUNDING_SCALAR) - 1;
	if (atime < 0) {
		hwlog_err("%s: atime out of range: %d\n", __func__, atime);
		return;
	} else if (atime > FULL_BYTE) {
		hwlog_err("%s: atime out of range: %d\n", __func__, atime);
		return;
	}
	p_as7341->alg_ctx.atime = (UINT8)atime;

	color_sensor_set_byte(p_as7341->handle, AS7341_ATIME_REG,
		p_as7341->alg_ctx.atime);
	p_as7341->alg_ctx.itime_ms = itime_ms;
	hwlog_debug("%s als_itime: %dms', als_atime: %d\n", __func__, itime_ms,
		p_as7341->alg_ctx.atime);
}

static void as7341_als_set_astep(struct as7341_ctx *p_as7341, UINT16 astep)
{
	if (!p_as7341) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	p_as7341->alg_ctx.astep = astep;
	color_sensor_write_fifo(p_as7341->handle, AS7341_ASTEPL_REG,
		&astep, ASTEP_LEN);
}

static UINT16 as7341_als_get_fullscale(struct as7341_ctx *p_as7341)
{
	UINT32 value;

	if (!p_as7341) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}

	value = ((UINT32)(p_as7341->alg_ctx.astep) + 1) *
		((UINT32)(p_as7341->alg_ctx.atime) + 1);
	if (value > MAX_ADC_COUNT)
		value = MAX_ADC_COUNT;
	return (uint16_t)value;
}

static void as7341_log_buffer(UINT8 *p_name, UINT32 *p_buf, UINT8 size)
{
	if (!p_name || !p_buf) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (size != AMS_REPORT_DATA_LEN)
		return;

	hwlog_debug("%s : %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", p_name,
		p_buf[0], p_buf[1], p_buf[2], p_buf[3],
		p_buf[4], p_buf[5], p_buf[6], p_buf[7],
		p_buf[8], p_buf[9]);
}

static void calculate_fd_sat_threshold(struct as7341_ctx *ctx)
{
	UINT8 low_fd_atime = 0;
	UINT8 high_fd_atime = 0;
	UINT32 fd_time;

	if (!ctx) {
		hwlog_err("%s NULL poniter\n", __func__);
		return;
	}

	color_sensor_get_byte(ctx->handle, AS7341_FD_CFG1, &low_fd_atime);
	as7341_get_field(ctx->handle, AS7341_FD_CFG3, &high_fd_atime,
		FD_TIME_HIGH_3_BIT_MASK);
	fd_time = COMB_WORD(high_fd_atime, low_fd_atime);
	max_flicker = (fd_time * SATURATION_LOW_PERCENT) /
		SATURATION_HIGH_PERCENT;
	hwlog_info("%s max_gain: %u, threshold: %u\n", __func__, fd_time,
		max_flicker);
}

static void as7341_reset_regs(struct as7341_ctx *ctx)
{
	UINT8 i;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(as7341_settings); i++)
		color_sensor_set_byte(ctx->handle, as7341_settings[i].reg,
			as7341_settings[i].value);

	as7341_set_field(ctx->handle, AS7341_ENABLE_REG, PON, PON | AEN | FDEN);
	as7341_set_field(ctx->handle, AS7341_CTRL_REG, ALS_MANUAL_AZ,
		ALS_MANUAL_AZ);

	as7341_als_set_gain(ctx, DEFAULT_ALS_GAIN_ID);
	as7341_als_set_astep(ctx, DEFAULT_ASTEP);
	as7341_als_set_itime(ctx, AS7341_ITIME_DEFAULT);

	calculate_fd_sat_threshold(ctx);

	if (color_sensor_set_byte(ctx->handle, AS7341_CFG0_REG, 0) < 0)
		hwlog_err("%s: Failed writing register\n", __func__);

	ctx->alg_ctx.load_calibration = true;
	// initialize calibration default values
	for (i = 0; i < AMS_REPORT_DATA_LEN; i++) {
		ctx->cal_ctx.ratio[i] = AS7341_CAL_RATIO;
		ctx->cal_ctx.target[i] = 0;
	}
}

static UINT8 as7341_get_fd_gain_reg_value(int gain)
{
	UINT8 cfg1;

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
	default:
		cfg1 = FD_GAIN_512X;
		break;
	}

	return cfg1;
}

static INT32 as7341_set_fd_gain(struct as7341_ctx *ctx, int gain)
{
	UINT8 cfg1;
	UINT32 old_gain;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	old_gain = ctx->flc_ctx.fd_gain;
	cfg1 = as7341_get_fd_gain_reg_value(gain);

	// close FDEN
	as7341_set_field(ctx->handle, AS7341_ENABLE_REG, 0, FDEN);
	as7341_set_field(ctx->handle, AS7341_CTRL_REG, CLEAR_FIFO,
		CLEAR_FIFO);
	// open FDEN
	as7341_set_field(ctx->handle, AS7341_FD_CFG3, cfg1,
		FD_GAIN_MASK);
	as7341_set_field(ctx->handle, AS7341_ENABLE_REG, FDEN, FDEN);

	ctx->flc_ctx.fd_gain = gain;
	hwlog_debug("%s old = %u, new = %d, cfg1 = 0x%02x\n", __func__,
		old_gain, gain, cfg1);
	return 0;
}

#ifdef TEST_COMPARE_SMUX_CONFIG
static void as7341_get_smux_via_ram(struct as7341_ctx *ctx, uint8_t *p_data,
	uint8_t size)
{
	uint8_t regval;
	UINT8 i = 0;

	if (size != SMUX_BUF_LEN) {
		hwlog_err("%s: Length error: %u\n", __func__, size);
		return;
	}

	// read smux conf from smux chain to ram
	regval = SMUX_DIR_WRITE;
	color_sensor_set_byte(ctx->handle, AS7341_CFG6_REG, regval);

	// start smux command
	regval = PON | SMUXEN;
	color_sensor_set_byte(ctx->handle, AS7341_ENABLE_REG, regval);

	while (i++ < MAX_SMUX_WAIT_CYCLE) {
		color_sensor_get_byte(ctx->handle, AS7341_ENABLE_REG, &regval);
		if ((regval & SMUXEN) == 0) {
			break;
		} else {
			hwlog_info("%s Poll AS7341_ENABLE_REG: 0x%02x\n",
				__func__, regval);
			mdelay(1);
		}
	}

	// read smux config from ram
	color_sensor_read_fifo(ctx->handle, 0x00, p_data, SMUX_BUF_LEN);

	// clear Smux Set command
	regval = 0x00;
	color_sensor_set_byte(ctx->handle, AS7341_CFG6_REG, regval);
}
#endif

static void as7341_write_smux_with_ram(struct as7341_ctx *ctx,
	uint8_t *p_smux)
{
	uint8_t regval;

	// write smux conf from ram to smux chain
	regval = SMUX_DIR_READ;
	color_sensor_set_byte(ctx->handle, AS7341_CFG6_REG, regval);

	// write smux config to ram
	color_sensor_write_fifo(ctx->handle, 0x00, p_smux,
		SMUX_BUF_LEN);

	// power on, start smux command
	regval = PON | SMUXEN;
	color_sensor_set_byte(ctx->handle, AS7341_ENABLE_REG, regval);
}

static void as7341_write_smux_without_ram(struct as7341_ctx *ctx,
	uint8_t *p_smux)
{
	struct i2c_msg msgs[SMUX_WITHOUT_RAM_MSG_SIZE * SMUX_NO_RAM_I2C_NUM];
	int ret;
	uint8_t i, j, msg_num;
	uint8_t buffer[SMUX_NO_RAM_I2C_NUM] = {
		AS7341_REGADDR_CHAINCMD, CHAIN_CMD_SHIFT_ALS_6BITS };
	UINT8 config[SMUX_WITHOUT_RAM_MSG_SIZE];

	for (i = 0; i < SMUX_WITHOUT_RAM_MSG_SIZE; i += SMUX_NO_RAM_I2C_NUM) {
		config[i] = AS7341_REGADDR_CHAIN_SMUX;
		config[i + 1] = p_smux[SMUX_BUF_LEN - 1 -
			(i / SMUX_NO_RAM_I2C_NUM)];
	}

	for (i = 0, j = 0; i < SMUX_WITHOUT_RAM_MSG_SIZE; i++) {
		msgs[j].flags = I2C_M_STOP;
		msgs[j].addr  = ctx->handle->addr;
		msgs[j].len   = SMUX_NO_RAM_I2C_NUM;
		if (i % SMUX_NO_RAM_I2C_NUM)
			msgs[j].buf = buffer;
		else
			msgs[j].buf = config + i;
		j++;

		// read data because no stop condition is supported
		msgs[j].flags = I2C_M_RD;
		msgs[j].addr  = ctx->handle->addr;
		msgs[j].len   = 1;
		msgs[j].buf   = &i;
		j++;
	}

	msg_num = (SMUX_WITHOUT_RAM_MSG_SIZE * SMUX_NO_RAM_I2C_NUM) - 1;

	ret = i2c_transfer(ctx->handle->adapter, msgs, msg_num);
	if (ret != msg_num) {
		if (ret < 0)
			hwlog_err("%s: Multi I2C Xfer failed with %d",
				__func__, -ret);
		else
			hwlog_err("%s: Multi I2C Xfer only %d of %u mesgs",
				__func__, ret, msg_num);
		return;
	}

	if (color_sensor_set_byte(ctx->handle, AS7341_REGADDR_CHAINCMD, 0) < 0)
		hwlog_err("%s: I2C register write failed", __func__);
}

static int8_t as7341_create_smux_buffer(
	const uint16_t *p_conf, uint8_t channel_num,
	uint8_t *p_smux_config, uint8_t smux_size)
{
	uint16_t smux_value;
	uint8_t i, pixel;
	bool para_check;

	para_check = !p_conf || !p_smux_config || (smux_size != SMUX_BUF_LEN) ||
		(channel_num != SMUX_CHANNELS);
	if (para_check)
		return -EFAULT;

	memset(p_smux_config, 0, smux_size);

	for (i = 1; i <= SMUX_CHANNELS; i++) {
		pixel = (p_conf[i - 1] >> ONE_BYTE_LENGTH) & FULL_BYTE;
		if (pixel > PIXEL_MAX)
			return -EINVAL;
		if (p_conf[i - 1] > FULL_BYTE && pixel != FULL_BYTE) {
			smux_value = p_smux_config[pixel / PIXEL_RATIO];
			if (((pixel % PIXEL_RATIO) > 0) &&
				((smux_value & HIGH_HALF_BYTE) == 0)) {
				smux_value |= (((uint16_t)i) <<
					HALF_BYTE_SHIFT) & HIGH_HALF_BYTE;
			} else if (((pixel % PIXEL_RATIO) == 0) &&
				(smux_value & LOW_HALF_BYTE) == 0) {
				smux_value |= ((uint16_t)i) & LOW_HALF_BYTE;
			} else {
				return -EINVAL;
			}
			p_smux_config[pixel / PIXEL_RATIO] = smux_value;
		}

		pixel = p_conf[i - 1] & FULL_BYTE;
		if (pixel > PIXEL_MAX)
			return -EINVAL;
		if (pixel != FULL_BYTE) {
			smux_value = p_smux_config[pixel / PIXEL_RATIO];
			if (((pixel % PIXEL_RATIO) > 0) &&
				((smux_value & HIGH_HALF_BYTE) == 0)) {
				smux_value |= (((uint16_t)i) << HALF_BYTE_SHIFT) &
					HIGH_HALF_BYTE;
			} else if (((pixel % PIXEL_RATIO) == 0) &&
				(smux_value & LOW_HALF_BYTE) == 0) {
				smux_value |= ((uint16_t)i) & LOW_HALF_BYTE;
			} else {
				return -EINVAL;
			}
			p_smux_config[pixel / PIXEL_RATIO] = smux_value;
		}
	}
	return 0;
}

static void switch_smux(struct as7341_ctx *ctx, enum smux_type smux)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (smux >= SMUX_NUM)
		return;
	as7341_write_smux_without_ram(ctx, smux_buffer[smux]);
}

static UINT8 get_rgb_fd_enable_status(struct i2c_client *handle)
{
	UINT8 enable_sta = 0;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	color_sensor_get_byte(handle, AS7341_ENABLE_REG, &enable_sta);
	return enable_sta;
}

static INT32 as7341_set_als_onoff(struct as7341_ctx *ctx, UINT8 data,
	bool cal_mode)
{
	INT32 result = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	hwlog_info("%s set_als_onoff data is %u\n", __func__, data);

	// reset interrupt status
	color_sensor_set_byte(ctx->handle, AS7341_STATUS_REG,
		(ASAT_MASK | AINT_MASK));

	if (data == 0) {
		// disable als measurement
		as7341_set_field(ctx->handle, AS7341_ENABLE_REG, 0, AEN);
	} else {
		// For first integration after AEN
		ctx->alg_ctx.meas_steps = STEP_DEFAULT_MEASURE;
		ctx->alg_ctx.als_update = 0;
		if (!cal_mode) {
			ctx->alg_ctx.meas_steps = STEP_FAST_AUTO_GAIN;
			as7341_als_set_itime(ctx, AS7341_ITIME_FOR_FIRST_DATA);
			as7341_als_set_gain(ctx, AS7341_AGAIN_FOR_FIRST_DATA);
		} else {
			as7341_als_set_itime(ctx, AS7341_ITIME_DEFAULT);
			as7341_als_set_gain(ctx, DEFAULT_CALIB_GAIN_ID);
		}

		// starts manual auto zero if flicker is not running
		if (FDEN != (get_rgb_fd_enable_status(ctx->handle) & FDEN)) {
			hwlog_info("%s starts manual autozero\n", __func__);
			as7341_set_field(ctx->handle, AS7341_CTRL_REG,
				ALS_MANUAL_AZ, ALS_MANUAL_AZ);
			result = 1;
		}

		// enable als measurement
		channel_block = 0;
		as7341_set_field(ctx->handle, AS7341_ENABLE_REG, AEN, AEN);
	}
	hwlog_info("%s set_als_onoff smux_mode is  %u\n", __func__,
		ctx->alg_ctx.smux_mode);
	return result;
}

static void as7341_handle_first_integration(struct as7341_ctx *ctx)
{
	UINT8 old_gain_id;
	UINT8 gain_id;
	UINT8 saturation;

	gain_id = as7341_als_get_saved_gain(ctx);
	old_gain_id = gain_id;
	if (ctx->alg_ctx.smux_mode > 1)
		return;
	as7341_get_optimized_gain(as7341_als_get_fullscale(ctx),
		highest_digits[ctx->alg_ctx.smux_mode],
		&gain_id, &saturation);
	if (old_gain_id != gain_id) {
		hwlog_info("first gain adjust - old gain: %u, saturation: %u\n",
			old_gain_id, saturation);
		as7341_als_set_gain(ctx, gain_id);
	}

	if (!saturation) {
		as7341_als_set_itime(ctx, AS7341_ITIME_FAST);
		ctx->alg_ctx.meas_steps = STEP_FAST_MEASURE;
	}
}

static bool as7341_handle_smux_block(struct as7341_ctx *ctx, bool cal_mode)
{
	UINT8 old_gain_id;
	UINT8 gain_id;
	UINT8 saturation;
	bool re_enable = false;
	UINT16 maximum;

	if (channel_block == 0) {
		ctx->alg_ctx.smux_mode = !ctx->alg_ctx.smux_mode;

		// read FIFO buffer length
		mutex_lock(&ctx->flc_ctx.lock);
		color_sensor_get_byte(ctx->handle, AS7341_FIFO_LVL,
			&ctx->flc_ctx.fd_smux_impact_start);

		switch_smux(ctx, ctx->alg_ctx.smux_mode);

		// read FIFO buffer length
		color_sensor_get_byte(ctx->handle, AS7341_FIFO_LVL,
			&ctx->flc_ctx.fd_smux_impact_end);
		mutex_unlock(&ctx->flc_ctx.lock);
	} else {
		if (cal_mode) {
			ctx->alg_ctx.als_update |= AMS_AS7341_FEATURE_ALS;
		} else {
			if (highest_digits[0] > highest_digits[1])
				maximum = highest_digits[0];
			else
				maximum = highest_digits[1];

			gain_id = as7341_als_get_saved_gain(ctx);
			old_gain_id = gain_id;
			as7341_get_optimized_gain(as7341_als_get_fullscale(ctx),
				maximum, &gain_id, &saturation);
			if (old_gain_id != gain_id) {
				hwlog_info("gain adjust - old: %u, sat: %u\n",
					old_gain_id, saturation);
				as7341_als_set_gain(ctx, gain_id);
			}

			if (saturation)
				re_enable = true;
			// in case gain change but als just output raw
			else if (old_gain_id == gain_id)
				ctx->alg_ctx.als_update |=
					AMS_AS7341_FEATURE_ALS;
		}
	}
	channel_block = !channel_block;

	return re_enable;
}

static void adjust_raw_values_on_itime(struct as7341_ctx *ctx)
{
	int i;

	for (i = 0; i < AMS_REPORT_DATA_LEN; i++)
		ctx->alg_ctx.data[i] *= AS7341_ITIME_DEFAULT /
			AS7341_ITIME_FAST;
	as7341_log_buffer("INT_ADJUST", ctx->alg_ctx.data,
		AMS_REPORT_DATA_LEN);
	as7341_als_set_itime(ctx, AS7341_ITIME_DEFAULT);
	ctx->alg_ctx.meas_steps = STEP_DEFAULT_MEASURE;
}

static void as7341_save_data(struct as7341_ctx *ctx, UINT8 old_smux,
	UINT16 *p_adc_data)
{
	if (old_smux == SMUX_OTHER) {
		ctx->alg_ctx.data[CHAN_F1] = p_adc_data[0];
		ctx->alg_ctx.data[CHAN_F2] = p_adc_data[1];
		ctx->alg_ctx.data[CHAN_F3] = p_adc_data[2];
		ctx->alg_ctx.data[CHAN_F8] = p_adc_data[3];
		ctx->alg_ctx.data[CHAN_NR] = p_adc_data[4];
	} else {
		ctx->alg_ctx.data[CHAN_F4] = p_adc_data[0];
		ctx->alg_ctx.data[CHAN_F5] = p_adc_data[1];
		ctx->alg_ctx.data[CHAN_F6] = p_adc_data[2];
		ctx->alg_ctx.data[CHAN_F7] = p_adc_data[3];
		ctx->alg_ctx.data[CHAN_CL] = p_adc_data[4];
	}
}

static bool as7341_handle_als(struct as7341_ctx *ctx, bool cal_mode)
{
	UINT16 adc_data[SMUX_CHANNELS] = {0};
	bool re_enable = false;
	UINT8 i;
	UINT8 old_smux = ctx->alg_ctx.smux_mode;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	color_sensor_read_fifo(ctx->handle, AS7341_ADATA0L_REG,
		(UINT8 *)adc_data, RAW_DATA_BYTE_NUM);

	as7341_set_field(ctx->handle, AS7341_ENABLE_REG, 0, AEN);

	if (ctx->alg_ctx.smux_mode > 1)
		return false;
	highest_digits[ctx->alg_ctx.smux_mode] = adc_data[0];
	for (i = 1; i < SMUX_CHANNELS - 1; i++)
		if (highest_digits[ctx->alg_ctx.smux_mode] < adc_data[i])
			highest_digits[ctx->alg_ctx.smux_mode] = adc_data[i];

#ifdef EXTENDED_LOG
	if (ctx->alg_ctx.smux_mode == SMUX_DEFAULT) {
		hwlog_debug("CH0:F4 %u, F5 %u, F6 %u, F7 %u, CL %u, GAIN %u\n",
			adc_data[0], adc_data[1], adc_data[2],
			adc_data[3], adc_data[4],
			((struct as7341_ctx *)ctx)->alg_ctx.gain_id);
	} else {
		hwlog_debug("CH1:F1 %u, F2 %u, F3 %u, F8 %u, NR %u, GAIN %u\n",
			adc_data[0], adc_data[1], adc_data[2],
			adc_data[3], adc_data[4],
			((struct as7341_ctx *)ctx)->alg_ctx.gain_id);
	}
#endif

	if (ctx->alg_ctx.meas_steps == STEP_FAST_AUTO_GAIN) {
		as7341_handle_first_integration(ctx);
		re_enable = true;
	} else {
		re_enable = as7341_handle_smux_block(ctx, cal_mode);
	}

	if (!re_enable) {
		as7341_save_data(ctx, old_smux, adc_data);
		if ((ctx->alg_ctx.meas_steps == STEP_FAST_MEASURE) &&
			ctx->alg_ctx.als_update)
			adjust_raw_values_on_itime(ctx);
	}

	as7341_set_field(ctx->handle, AS7341_ENABLE_REG, AEN, AEN);
	hwlog_debug("%s als_update: %u, re_enable: %d\n", __func__,
		ctx->alg_ctx.als_update, re_enable);
	return re_enable;
}

static bool as7341_handle_als_event(struct as7341_ctx *ctx, bool cal_mode)
{
	bool ret = false;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return false;
	}
	color_sensor_get_byte(ctx->handle, AS7341_STATUS_REG,
		&ctx->alg_ctx.status);
	if (ctx->alg_ctx.status & AINT_MASK) {
		// clear the soft interrupt
		color_sensor_set_byte(ctx->handle, AS7341_STATUS_REG,
			(ASAT_MASK | AINT_MASK));
		// only get available staus, then read data
		ret = as7341_handle_als(ctx, cal_mode);
	} else {
		hwlog_warn("%s: No new als data were available\n", __func__);
	}
	return ret;
}

static UINT8 as7341_get_als_update(struct as7341_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	return ctx->alg_ctx.als_update;
}

static void as7341_get_als_data(struct as7341_ctx *ctx,
	UINT32 *p_data, UINT32 num)
{
	if (!ctx || !p_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (num != AMS_REPORT_DATA_LEN) {
		hwlog_err("%s: length out of range: %u\n", __func__, num);
		return;
	}

	memcpy(p_data, ctx->alg_ctx.data, num * sizeof(UINT32));
	ctx->alg_ctx.als_update &= ~(AMS_AS7341_FEATURE_ALS);
}

void as7341_als_timer_wrk(uintptr_t data)
{
	struct color_chip *chip = (struct color_chip *)data;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->als_work);
}

void as7341_flc_timer_wrk(uintptr_t data)
{
	struct color_chip *chip = (struct color_chip *)data;

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	schedule_work(&chip->fd_work);
}

static ssize_t as7341_als_set_enable(struct color_chip *chip, uint8_t en)
{
	UINT8 rgb_enable_status;
	UINT32 i;
	UINT32 autozero_offset = 0;

	if (!chip) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return 0;
	}
	hwlog_info("%s = %u\n", __func__, en);

	rgb_enable_status = get_rgb_fd_enable_status(chip->client);
	if (((rgb_enable_status & AEN) == AEN) && (en == 1)) {
		hwlog_warn("%s, rgb already enabled, return\n", __func__);
		return 0;
	}

	if (((rgb_enable_status & AEN) != AEN) && (en == 0)) {
		hwlog_warn("%s, rgb already disabled, return\n", __func__);
		return 0;
	}

	if (as7341_set_als_onoff(chip->device_ctx, en, chip->in_cal_mode))
		autozero_offset = AUTOZERO_TIMER_OFFSET;

	if (en == 0) {
		hwlog_info("%s: close rgb, del_timer\n", __func__);
		return 0;
	}

	// set default report val to -1 when enable
	color_report_val[0] = AMS_REPORT_DATA_LEN;
	for (i = 1; i < MAX_REPORT_LEN; i++)
		color_report_val[i] = -1;

	mod_timer(&chip->work_timer,
		jiffies + msecs_to_jiffies(autozero_offset
		+ ((struct as7341_ctx *)chip->device_ctx)->alg_ctx.itime_ms
		+ HIGH_TIMER_OFFSET));

	report_logcount = 0;

	return 0;
}
static int get_cal_para_from_nv(struct as7341_calib_ctx_t *p_calib)
{
	int ret, i;

	if (!p_calib) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s: read calib data with size %d\n", __func__,
		sizeof(struct as7341_calib_ctx_t));
	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM,
		sizeof(struct as7341_calib_ctx_t), "RGBAP", (char *)p_calib);
	if (ret < 0) {
		hwlog_err("%s: failed with error code %d\n", __func__, ret);
		return -ENODATA;
	}
	for (i = 0; i < AMS_REPORT_DATA_LEN; i++) {
		if (p_calib->ratio[i] == 0) {
			hwlog_err("%s: read calib ratio from mem %d\n",
				__func__);
			return -EILSEQ;
		}
	}
	return ret;
}

static int save_cal_para_to_nv(struct as7341_calib_ctx_t *p_calib)
{
	int ret;

	if (!p_calib) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s: write calib data with size %d\n", __func__,
		sizeof(struct as7341_calib_ctx_t));
	ret = write_color_calibrate_data_to_nv(RGBAP_CALI_DATA_NV_NUM,
		sizeof(struct as7341_calib_ctx_t), "RGBAP", (char *)p_calib);

	if (ret < 0)
		hwlog_err("%s: fail, error code\n", __func__, ret);
	return ret;
}

static int as7341_cal_als(struct color_chip *chip)
{
	UINT32 raw_data[AMS_REPORT_DATA_LEN];
	int i;
	struct as7341_ctx *ctx = chip->device_ctx;
	UINT32 curr_gain;

	hwlog_debug("%s, comes into cali step\n", __func__);
	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	curr_gain = (ctx->alg_ctx.gain / AS7341_GAIN_SCALE);

	as7341_get_als_data(chip->device_ctx, raw_data, AMS_REPORT_DATA_LEN);

	as7341_log_buffer("cal_target", ctx->cal_ctx.target,
		AMS_REPORT_DATA_LEN);
	as7341_log_buffer("cal_raw", raw_data, AMS_REPORT_DATA_LEN);

	hwlog_info("%s: state = %d, count = %u, curr_gain = %u\n",
		__func__, chip->cali_ctx.cal_state,
		chip->cali_ctx.cal_raw_count, curr_gain);

	for (i = 0; i < AMS_REPORT_DATA_LEN; i++)
		if (raw_data[i] == 0) {
			// can"t devide by zero, use the default scaling factor
			ctx->cal_ctx.ratio[i] = AS7341_CAL_RATIO;
			color_calibrate_result = false;
			hwlog_err("%s, raw[%d] == 0, can not devide by zero\n",
				__func__, i);
		} else {
			ctx->cal_ctx.ratio[i] = ctx->cal_ctx.target[i] *
				(curr_gain * AMS_AS7341_FLOAT_TO_FIX /
				AMS_AS7341_GAIN_OF_GOLDEN) /
				raw_data[i];
		}

	as7341_log_buffer("cal_ratio", ctx->cal_ctx.ratio, AMS_REPORT_DATA_LEN);
	hwlog_info("cal_result : %d", color_calibrate_result);

	if (color_calibrate_result)
		save_cal_para_to_nv(&(ctx->cal_ctx));

	report_calibrate_result = true;
	chip->in_cal_mode = false;

	if (enable_status_before_cali != 1)
		as7341_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
	else
		hwlog_info("color sensor disabled before calibrate\n");

	hwlog_info("%s done\n", __func__);

	return 0;
}

static void as7341_report_als(struct color_chip *chip)
{
	UINT32 raw_data[AMS_REPORT_DATA_LEN];
	UINT32 curr_gain;
	UINT8 i;
	struct as7341_ctx *ctx = NULL;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = chip->device_ctx;
	as7341_get_als_data(chip->device_ctx, raw_data, AMS_REPORT_DATA_LEN);
	curr_gain = ctx->alg_ctx.gain / AS7341_GAIN_SCALE;
	if (curr_gain == 0)
		return;
	// adjust the report data when the calibrate ratio is acceptable
	for (i = 0; i < AMS_REPORT_DATA_LEN; i++)
		raw_data[i] = raw_data[i] * ctx->cal_ctx.ratio[i] /
			AMS_AS7341_FLOAT_TO_FIX;

	hwlog_debug("raw data whitebalanced: %u, %u, %u\n",
		raw_data[0], raw_data[1], raw_data[2]);

	// send data without length information
	if (skip_first_rgb_data) {
		skip_first_rgb_data = false;
	} else {
		color_report_val[0] = AMS_REPORT_DATA_LEN;
		for (i = 0; i < AMS_REPORT_DATA_LEN; i++)
			color_report_val[i + 1] = raw_data[i] *
				AMS_AS7341_GAIN_OF_GOLDEN / curr_gain;
		as7341_report_data(color_report_val + 1);
	}
	report_logcount++;
	if ((report_logcount > AMS_REPORT_DATA_LEN) &&
		(report_logcount < AMS_REPORT_LOG_COUNT_NUM))
		return;
	if (report_logcount == AMS_REPORT_LOG_COUNT_NUM)
		report_logcount = MAX_LOG_COUNT;
	hwlog_info("color_report_val: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		color_report_val[1], color_report_val[2],
		color_report_val[3], color_report_val[4],
		color_report_val[5], color_report_val[6],
		color_report_val[7], color_report_val[8],
		color_report_val[9], color_report_val[10]);
	hwlog_info("curr gain: %u\n", curr_gain);
}

int as7341_enable_rgb(bool enable)
{
	struct color_chip *chip = p_chip;

	hwlog_info("%s: enter\n", __func__);

	if (!chip) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return 0;
	}

	if (enable)
		as7341_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
	else
		as7341_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);

	return 1;
}
EXPORT_SYMBOL_GPL(as7341_enable_rgb);
void as7341_show_calibrate(struct color_chip *chip,
	struct at_color_sensor_output_t *out)
{
	UINT32 i;
	UINT32 j;
	struct as7341_ctx *ctx = chip->device_ctx;

	if (!out || !chip || !ctx) {
		hwlog_err("%s input para NULL\n", __func__);
		return;
	}

	if (chip->in_cal_mode == false)
		hwlog_err("%s not in cali mode\n", __func__);

	out->result = (UINT32)report_calibrate_result;
	hwlog_info("%s result = %u\n", __func__, out->result);

	out->gain_arr = CAL_STATE_GAIN_LAST;
	out->color_arr = AMS_REPORT_DATA_LEN;
	memcpy(out->cali_gain, as7341_als_gain_lvl, sizeof(out->cali_gain));

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		for (j = 0; j < MAX_RAW_DATA_LEN; j++)
			if (j < AMS_REPORT_DATA_LEN)
				out->cali_rst[j][i] = ctx->cal_ctx.ratio[j];
			else
				out->cali_rst[j][i] = 0;

		hwlog_info("%s i = %d: cali= %u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
			__func__,
			as7341_als_gain_lvl[i], out->cali_rst[0][i],
			out->cali_rst[1][i], out->cali_rst[2][i],
			out->cali_rst[3][i], out->cali_rst[4][i],
			out->cali_rst[5][i], out->cali_rst[6][i],
			out->cali_rst[7][i], out->cali_rst[8][i],
			out->cali_rst[9][i]);
	}
}

void as7341_store_calibrate(struct color_chip *chip,
	struct at_color_sensor_input_t *in)
{
	struct as7341_ctx *ctx = NULL;
	UINT8 rgb_enable_status;

	if (!chip || !chip->device_ctx || !in) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	report_calibrate_result = false;

	hwlog_info("%s get tar_f1_f2_f3_clear_nir = %u, %u, %u, %u, %u\n",
		__func__, in->reserverd[0], in->reserverd[1],
		in->reserverd[2], in->reserverd[8],
		in->reserverd[9]);

	if (in->enable && chip->in_cal_mode) {
		hwlog_err("%s already in cali mode\n", __func__);
		return;
	}

	if (in->enable) {
		ctx = chip->device_ctx;
		hwlog_info("%s start calibration mode\n", __func__);
		chip->cali_ctx.cal_raw_count = 0;

		// copy target value
		memcpy(ctx->cal_ctx.target, in->reserverd,
			AMS_REPORT_DATA_LEN * sizeof(UINT32));

		// disable running measurement and safe state
		rgb_enable_status = get_rgb_fd_enable_status(ctx->handle);
		if ((rgb_enable_status & AEN) == AEN) {
			// enabled before calibrate
			enable_status_before_cali = 1;
			hwlog_info("%s: enabled before calibrate\n", __func__);
			as7341_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
			mdelay(10); // sleep 10 ms to make sure disable timer
		} else {
			// disabled before calibrate
			enable_status_before_cali = 0;
			hwlog_info("%s: disabled before calibrate\n", __func__);
		}
		chip->in_cal_mode = true;
		// make the calibrate_result true, it will be reset on any error
		color_calibrate_result = true;

		// start calibration process
		as7341_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
	} else {
		hwlog_info("%s stop calibration mode\n", __func__);
		chip->in_cal_mode = false;
	}
}

void as7341_show_enable(struct color_chip *chip, int *state)
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

void as7341_store_enable(struct color_chip *chip, int state)
{
	hwlog_err("%s: enter\n", __func__);

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	skip_first_rgb_data = true;
	if (state)
		as7341_als_set_enable(chip, AMSDRIVER_ALS_ENABLE);
	else
		as7341_als_set_enable(chip, AMSDRIVER_ALS_DISABLE);
}

static void fd_enable_set(struct color_chip *chip, UINT8 en)
{
	struct as7341_ctx *ctx = NULL;
	UINT8 fd_enable_status;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = (struct as7341_ctx *)chip->device_ctx;

	hwlog_info("%s = %u\n", __func__, en);
	fd_enable_status = get_rgb_fd_enable_status(ctx->handle);
	if (en) {
		if ((fd_enable_status & FDEN) != FDEN) {

			// set default gain
			as7341_set_fd_gain(ctx, DEFAULT_FD_GAIN);

			// enable flicker
			as7341_set_field(ctx->handle, AS7341_ENABLE_REG, FDEN,
				FDEN);

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

			// disable flicker
			as7341_set_field(ctx->handle, AS7341_ENABLE_REG,
				0, FDEN);

			fd_timer_stopped = true;
			// clear ring_buffer when close the fd
			memset(ring_buffer, 0,
				MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT);
			buffer_ptr = &ring_buffer[0];
			head_ptr = &ring_buffer[0];
			tail_ptr = &ring_buffer[0];
			hwlog_info("fd_enable now disable fd sensor\n");
		} else {
			hwlog_warn("fd_enable has already been disabled\n");
		}
	}
}

void as7341_fd_show_enable(struct color_chip *chip, int *state)
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

void as7341_fd_store_enable(struct color_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	if (state)
		// check the calibration process, disable fd function
		fd_enable_set(chip, AMSDRIVER_FD_ENABLE);
	else
		fd_enable_set(chip, AMSDRIVER_FD_DISABLE);

	hwlog_info("%s enable = %d success\n", __func__, state);
}

static void as7341_initialize_calibration(struct as7341_ctx *ctx)
{
	UINT32 i;

	if (get_cal_para_from_nv(&(ctx->cal_ctx)) < 0) {
		hwlog_warn("set default calibration data\n");
		// initialize calibration default values
		for (i = 0; i < AMS_REPORT_DATA_LEN; i++)
			ctx->cal_ctx.ratio[i] = AS7341_CAL_RATIO;
	} else {
		hwlog_debug("successful reading of calibration data\n");
		hwlog_info("CALIB %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
			ctx->cal_ctx.ratio[0],
			ctx->cal_ctx.ratio[1],
			ctx->cal_ctx.ratio[2],
			ctx->cal_ctx.ratio[3],
			ctx->cal_ctx.ratio[4],
			ctx->cal_ctx.ratio[5],
			ctx->cal_ctx.ratio[6],
			ctx->cal_ctx.ratio[7],
			ctx->cal_ctx.ratio[8],
			ctx->cal_ctx.ratio[9]);
	}
}

static void as7341_als_work(struct work_struct *work)
{
	bool re_enable = false;
	UINT8 rgb_data_status;
	UINT8 rgb_enable_status;
	struct as7341_ctx *ctx = NULL;
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
	ctx = (struct as7341_ctx *)chip->device_ctx;

	// read persistent data here, because it is not available during probe
	if (ctx->alg_ctx.load_calibration) {
		as7341_initialize_calibration(ctx);
		ctx->alg_ctx.load_calibration = false;
	}

	re_enable = as7341_handle_als_event(ctx, chip->in_cal_mode);

	rgb_data_status = as7341_get_als_update(ctx);
	if ((rgb_data_status & AMS_AS7341_FEATURE_ALS) && !re_enable) {
		hwlog_debug("%s: handle raw_data: cal=%d\n", __func__,
			chip->in_cal_mode);
		if (chip->in_cal_mode == false)
			as7341_report_als(chip);
		else
			as7341_cal_als(chip);
	}

	rgb_enable_status = get_rgb_fd_enable_status(ctx->handle);
	if ((rgb_enable_status & AEN) != AEN) {
		hwlog_warn("%s: rgb already disabled, del timer\n", __func__);
		return;
	}

	mod_timer(&chip->work_timer, jiffies +
		msecs_to_jiffies(ctx->alg_ctx.itime_ms + HIGH_TIMER_OFFSET));
}

void as7341_get_read_out_buffer(void)
{
	UINT32 i;
	uint32_t head_ptr_delta;
	u16 *temp_ptr = &ring_buffer[0];
	u16 *temp_head_ptr = head_ptr;

	head_ptr_delta = (uint32_t)((uintptr_t)temp_head_ptr -
		(uintptr_t)(&ring_buffer[0]));
	head_ptr_delta = head_ptr_delta / ASTEP_LEN;

	if (head_ptr_delta > MAX_BUFFER_SIZE - 1)
		head_ptr_delta = MAX_BUFFER_SIZE - 1;

	memset(read_out_buffer, 0, sizeof(read_out_buffer));

	for (i = 0; i < (MAX_BUFFER_SIZE - head_ptr_delta); i++) {
		read_out_buffer[i] = *temp_head_ptr;
		temp_head_ptr++;
	}

	for (i = (MAX_BUFFER_SIZE - head_ptr_delta); i < MAX_BUFFER_SIZE;
		i++) {
		read_out_buffer[i] = *temp_ptr;
		temp_ptr++;
	}

#ifdef EXTENDED_LOG
	for (i = 0; i < MAX_BUFFER_SIZE; i++)
		hwlog_debug("FD_Buffer[%d],%u\n", i, read_out_buffer[i]);
#endif
}

void as7341_ring_buffer_process(uint8_t fifo_lvl, uint16_t *buf_16)
{
	UINT32 i;

	if (!buf_16) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

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

static void fd_auto_gain_first(struct as7341_ctx *ctx, uint16_t buf_16_bit_max)
{
	if (buf_16_bit_max <= BUF_16_MAX_LEVEL5)
		// if min 9x gain auto lev5
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
	else if (buf_16_bit_max <= BUF_16_MAX_LEVEL4)
		// if min 18x gain auto lev4
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
	else if (buf_16_bit_max <= BUF_16_MAX_LEVEL3)
		// if min 36x gain auto lev3
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
	else if (buf_16_bit_max <= BUF_16_MAX_LEVEL2)
		hwlog_info("%s, keep the init 4x gain\n", __func__);
	else
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_1);

	hwlog_info("%s, come into first_fd_inte, gain = %u\n",
		__func__, ctx->flc_ctx.fd_gain);
}

static bool fd_auto_gain_normal(struct as7341_ctx *ctx, UINT16 buf_16_bit_max)
{
	bool fd_saturation_check;
	bool fd_insufficience_check;
	bool gain_adjusted = false;

	fd_saturation_check = buf_16_bit_max >= max_flicker;
	fd_insufficience_check = buf_16_bit_max < AS7341_FD_LOW_LEVEL;
	if (fd_saturation_check)
		hwlog_warn("%s, FD sat: %d, max_fd: %u, thresh: %u\n", __func__,
			fd_saturation_check, buf_16_bit_max, max_flicker);

	if (fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_2)) {
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_1);
		gain_adjusted = true;
	} else if ((fd_saturation_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_3)) ||
		(fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_1))) {
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_2);
		gain_adjusted = true;
	} else if ((fd_saturation_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_4)) ||
		(fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_2))) {
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
		gain_adjusted = true;
	} else if ((fd_saturation_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_5)) ||
		(fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_3))) {
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
		gain_adjusted = true;
	} else if (fd_insufficience_check &&
		(ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_4)) {
		as7341_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
		gain_adjusted = true;
	}
	return gain_adjusted;
}

static bool fd_auto_gain_control(struct as7341_ctx *ctx, uint16_t *buf_16_bit,
	int len)
{
	UINT8 i;
	UINT16 buf_16_bit_max;

	if (!ctx || !buf_16_bit) {
		hwlog_err("%s, pointer is null\n", __func__);
		return false;
	}

	if (len == 0)
		return false;

	if (len > MAX_AUTOGAIN_CHECK)
		len = MAX_AUTOGAIN_CHECK;

	fd_gain_adjusted = false;
	buf_16_bit_max = buf_16_bit[0];
	for (i = 1; i < len; i++)
		if (buf_16_bit_max < buf_16_bit[i])
			buf_16_bit_max = buf_16_bit[i];

	// first flicker detect data is special
	if (ctx->flc_ctx.first_fd_inte) {
		fd_auto_gain_first(ctx, buf_16_bit_max);
		ctx->flc_ctx.first_fd_inte = false;
		fd_gain_adjusted = true;
	} else {
		if (fd_auto_gain_normal(ctx, buf_16_bit_max))
			fd_gain_adjusted = true;
	}

	return fd_gain_adjusted;
}

static bool check_flicker_status(struct as7341_ctx *ctx)
{
	UINT8 fd_status6;
	bool error_detected = false;

	color_sensor_get_byte(ctx->handle, AS7341_STATUS6_REG, &fd_status6);
	if (fd_status6 & FD_TRIG) {
		error_detected = true;
		hwlog_err("ERROR: Flicker error detected");
	} else if (fd_status6 & OVTEMP) {
		error_detected = true;
		hwlog_err("ERROR: Flicker overtemperature detected");
	} else if (fd_status6 & FIFO_OV) {
		error_detected = true;
		hwlog_err("ERROR: Flicker FIFO overflow detected");
	}
	return error_detected;
}

static void repair_corrupted_fd_data(struct as7341_ctx *ctx,
	UINT16 *buf, UINT8 num)
{
	UINT16 start_idx;
	int slope;
	UINT8 i;

	if ((num > REPORT_FIFO_LEN) ||
		(ctx->flc_ctx.fd_smux_impact_end >= REPORT_FIFO_LEN)) {
		hwlog_err("num is %u, impact_end is %u overflow\n",
			num, ctx->flc_ctx.fd_smux_impact_end);
		ctx->flc_ctx.fd_smux_impact_start = 0;
		ctx->flc_ctx.fd_smux_impact_end = 0;
		return;
	}
	if (ctx->flc_ctx.fd_smux_impact_end != 0) {
		if (ctx->flc_ctx.fd_smux_impact_start > 0)
			start_idx = ctx->flc_ctx.fd_smux_impact_start - 1;
		else
			start_idx = 0;

		if ((ctx->flc_ctx.fd_smux_impact_end - start_idx) > 0)
			slope = ((buf[ctx->flc_ctx.fd_smux_impact_end] -
				buf[start_idx]) << ONE_BYTE_LENGTH) /
				(ctx->flc_ctx.fd_smux_impact_end - start_idx);
		else
			slope = 0;

		for (i = 0; i < num; i++)
			if ((i > start_idx) &&
				(i < ctx->flc_ctx.fd_smux_impact_end))
				buf[i] = ((slope * (i - start_idx)) >>
					ONE_BYTE_LENGTH) +
					buf[start_idx];

		ctx->flc_ctx.fd_smux_impact_start = 0;
		ctx->flc_ctx.fd_smux_impact_end = 0;
	}
}

static void pull_fd_data_to_buffer(struct as7341_ctx *ctx,
	UINT16 *buf, UINT8 num)
{
	UINT8 fd_enable_status, i;

	fd_enable_status = get_rgb_fd_enable_status(ctx->handle);
	hwlog_debug("%s, fd_enable_status = %u, fd_timer_stopped is %d\n",
		__func__, fd_enable_status, fd_timer_stopped);

	if ((fd_enable_status & FDEN) == FDEN) {
		// normalization
		for (i = 0; i < num; i++)
			buf[i] = (UINT16)(((UINT32)(buf[i]) *
				AMS_AS7341_FD_GAIN_OF_GOLDEN) /
				ctx->flc_ctx.fd_gain);
		as7341_ring_buffer_process(num, buf);
	}
}

static void reset_fd_buffer(struct as7341_ctx *ctx, bool error_detected)
{
	// if fd_gain is adjusted
	// clear the ringbuffer and reset the related pointer.
	mutex_lock(&ring_buffer_mutex);
	memset(ring_buffer, 0, MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT);
	// clear ring_buffer when close the fd
	buffer_ptr = &ring_buffer[0];
	head_ptr = &ring_buffer[0];
	tail_ptr = &ring_buffer[0];
	mutex_unlock(&ring_buffer_mutex);

	// clear FIFO
	ctx->flc_ctx.fd_smux_impact_start = 0;
	ctx->flc_ctx.fd_smux_impact_end = 0;

	if (error_detected) {
		as7341_set_field(ctx->handle, AS7341_ENABLE_REG, 0, FDEN);
		as7341_set_field(ctx->handle, AS7341_CTRL_REG, CLEAR_FIFO,
			CLEAR_FIFO);
		as7341_set_field(ctx->handle, AS7341_ENABLE_REG, FDEN, FDEN);
	}
}

static void read_fd_data(struct color_chip *chip)
{
	UINT16 buf_16_bit[REPORT_FIFO_LEN] = {0};
	bool auto_gain = false;
	UINT8 fifo_level;
	struct as7341_ctx *ctx = NULL;
	bool error_detected = false;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s, pointer is null\n", __func__);
		return;
	}
	ctx = (struct as7341_ctx *)chip->device_ctx;

	mutex_lock(&ctx->flc_ctx.lock);

	error_detected = check_flicker_status(ctx);
	color_sensor_get_byte(ctx->handle, AS7341_FIFO_LVL, &fifo_level);
	if (fifo_level > MAX_FIFO_LEVEL)
		error_detected = true;

	if (fd_timer_stopped == false)
		mod_timer(&chip->fd_timer,
			jiffies + msecs_to_jiffies(FD_POLLING_TIME));
	else
		hwlog_info("fd has been disabled, del fd work timer\n");

	// cal fd_ratio
	if (fifo_level > 0 && !error_detected) {
		color_sensor_read_fifo(ctx->handle, AS7341_FDADAL, buf_16_bit,
			fifo_level * FIFO_LEVEL_RATIO);
		// repair damaged data by SMUX reconfiguration
		repair_corrupted_fd_data(ctx, buf_16_bit, fifo_level);

		auto_gain = fd_auto_gain_control(ctx, buf_16_bit, fifo_level);
		if (auto_gain == false)
			pull_fd_data_to_buffer(ctx, buf_16_bit, fifo_level);
	} else {
		hwlog_warn("%s: FIFO_LVL: %u, error: %d\n", __func__,
			fifo_level, error_detected);
	}

	if (auto_gain == true || error_detected) {
		hwlog_warn("%s: FD reset, auto_gain: %d, error: %d\n",
			__func__, auto_gain, error_detected);
		reset_fd_buffer(ctx, error_detected);
	}

	mutex_unlock(&ctx->flc_ctx.lock);
}

static void as7341_fd_work(struct work_struct *work)
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
	ams_mutex_lock(&chip->lock);
	read_fd_data(chip);
	ams_mutex_unlock(&chip->lock);
}

static void as7341_get_fd_data(struct color_chip *chip, char *ams_fd_data)
{

	if (!chip || !ams_fd_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&ring_buffer_mutex);
	as7341_get_read_out_buffer();

	memcpy(ams_fd_data, read_out_buffer, sizeof(read_out_buffer));
	mutex_unlock(&ring_buffer_mutex);
}

static char *as7341_chip_name(void)
{
	return "ams_as7341";
}

int as7341_set_external_gain(void *ctx, int gain)
{
	if (gain > FULL_BYTE)
		return -EINVAL;

	as7341_als_set_gain(ctx, (UINT8)gain);
	return 0;
}

static void smux_init(struct as7341_ctx *ctx)
{
	uint16_t channels[SMUX_CHANNELS];

#ifdef TEST_COMPARE_SMUX_CONFIG
	uint8_t read_channels[SMUX_CHANNELS];
	uint8_t i;
#endif

	channels[0] = SMUX_F4;
	channels[1] = SMUX_F5;
	channels[2] = SMUX_F6;
	channels[3] = SMUX_F7;
	channels[4] = SMUX_CLEAR;
	channels[5] = SMUX_FLICKER;
	if (as7341_create_smux_buffer(channels, SMUX_CHANNELS, smux_buffer[0],
		SMUX_BUF_LEN))
		hwlog_info("failed as7341_create_smux_buffer 0\n");

	channels[0] = SMUX_F1;
	channels[1] = SMUX_F2;
	channels[2] = SMUX_F3;
	channels[3] = SMUX_F8;
	channels[4] = SMUX_NIR;
	channels[5] = SMUX_FLICKER;
	if (as7341_create_smux_buffer(channels, SMUX_CHANNELS, smux_buffer[1],
		SMUX_BUF_LEN))
		hwlog_info("failed as7341_create_smux_buffer 1\n");

#ifdef TEST_COMPARE_SMUX_CONFIG
	as7341_write_smux_without_ram(ctx, smux_buffer[0]);
	// read it twice because function fails on first time
	as7341_get_smux_via_ram(ctx, read_channels, SMUX_BUF_LEN);
	as7341_get_smux_via_ram(ctx, read_channels, SMUX_BUF_LEN);
	for (i = 0; i < SMUX_BUF_LEN; i++)
		hwlog_debug("c_smux_0 %d: 0x%02X - 0x%02X - %d -\n", i,
			smux_buffer[0][i], read_channels[i],
			smux_buffer[0][i] == read_channels[i]);

	as7341_write_smux_without_ram(ctx, smux_buffer[1]);
	as7341_get_smux_via_ram(ctx, read_channels, SMUX_BUF_LEN);
	for (i = 0; i < SMUX_BUF_LEN; i++)
		hwlog_debug("c_smux_1 %d: 0x%02X - 0x%02X - %d -\n", i,
			smux_buffer[1][i], read_channels[i],
			smux_buffer[1][i] == read_channels[i]);
#endif

	// configure first channels
	ctx->alg_ctx.smux_mode = SMUX_DEFAULT;
	channel_block = 0;
	as7341_write_smux_with_ram(ctx, smux_buffer[ctx->alg_ctx.smux_mode]);
}

static void configure_functions(struct color_chip *chip)
{
	chip->at_color_show_calibrate_state = as7341_show_calibrate;
	chip->at_color_store_calibrate_state = as7341_store_calibrate;
	chip->color_enable_show_state = as7341_show_enable;
	chip->color_enable_store_state = as7341_store_enable;
	chip->color_sensor_get_gain = as7341_als_get_gain;
	chip->color_sensor_set_gain = as7341_set_external_gain;
	chip->get_flicker_data = as7341_get_fd_data;
	chip->flicker_enable_show_state = as7341_fd_show_enable;
	chip->flicker_enable_store_state = as7341_fd_store_enable;
	chip->color_report_type = as7341_rgb_report_type;
	chip->color_chip_name = as7341_chip_name;

	color_default_enable = as7341_enable_rgb;
}

static void read_options_parameter(struct device *dev)
{
	int rc;

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

	hwlog_info("%s flicker_support = %u, algo_support = %u\n", __func__,
		flicker_support, algo_support);
}

static void init_library(struct color_chip *chip)
{
	int ret;

	init_timer(&chip->work_timer);
	setup_timer(&chip->work_timer, as7341_als_timer_wrk, (uintptr_t)chip);
	INIT_WORK(&chip->als_work, as7341_als_work);

	init_timer(&chip->fd_timer);
	setup_timer(&chip->fd_timer, as7341_flc_timer_wrk, (uintptr_t)chip);
	INIT_WORK(&chip->fd_work, as7341_fd_work);

	configure_functions(chip);

	p_chip = chip;
	ret = color_register(chip);
	if (ret < 0)
		hwlog_err("%s color_register fail\n", __func__);

	hwlog_info("rgb sensor %s ok\n", __func__);
}

int as7341_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
	int ret = -1;
	struct device *dev = NULL;
	static struct color_chip *chip = NULL;
	struct as7341_ctx *ctx = NULL;

	if (!client)
		return -EFAULT;

	color_notify_support(); // declare support as7341

	dev = &client->dev;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto init_failed;

	chip = kzalloc(sizeof(struct color_chip), GFP_KERNEL);
	if (!chip)
		goto malloc_failed;

	mutex_init(&chip->lock);
	chip->client = client;
	chip->pdata = dev->platform_data;
	i2c_set_clientdata(chip->client, chip);

	chip->in_cal_mode = false;
	chip->cali_ctx.cal_state = 0;

	if (as7341_get_chipid(chip->client) == AMS_UNKNOWN_DEVICE) {
		hwlog_info("%s failed: AMS_UNKNOWN_DEVICE\n", __func__);
		goto id_failed;
	}

	read_options_parameter(dev);
	as7341_log_revid(chip->client);

	chip->device_ctx = kzalloc(sizeof(struct as7341_ctx), GFP_KERNEL);
	if (!chip->device_ctx)
		goto id_failed;

	ctx = chip->device_ctx;
	ctx->handle = chip->client;
	mutex_init(&(ctx->flc_ctx.lock));

	as7341_reset_regs(ctx);
	smux_init(ctx);
	init_library(chip);
	return 0;

id_failed:
	if (chip->device_ctx)
		kfree(chip->device_ctx);
	i2c_set_clientdata(client, NULL);
	hwlog_err("%s id_failed\n", __func__);
malloc_failed:
	if (chip)
		kfree(chip);
init_failed:
	hwlog_err("%s Probe failed\n", __func__);
	color_notify_absent();
	return ret;
}

int as7341_suspend(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s\n", __func__);
	return 0;
}

int as7341_resume(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	hwlog_info("%s\n", __func__);
	return 0;
}

int ams_as7341_remove(struct i2c_client *client)
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
	{ "ams_as7341", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, amsdriver_idtable);

static const struct dev_pm_ops ams_as7341_pm_ops = {
	.suspend = as7341_suspend,
	.resume = as7341_resume,
};

static const struct of_device_id amsdriver_of_id_table[] = {
	{ .compatible = "ams,as7341" },
	{},
};

static struct i2c_driver ams_as7341_driver = {
	.driver = {
		.name = "ams_as7341",
		.owner = THIS_MODULE,
		.of_match_table = amsdriver_of_id_table,
	},
	.id_table = amsdriver_idtable,
	.probe = as7341_probe,
	.remove = ams_as7341_remove,
};

static int __init as7341_init(void)
{
	int rc;

	hwlog_info("%s\n", __func__);

	rc = i2c_add_driver(&ams_as7341_driver);

	hwlog_info("%s %d\n", __func__, rc);
	return rc;
}

static void __exit as7341_exit(void)
{
	hwlog_info("%s\n", __func__);
	i2c_del_driver(&ams_as7341_driver);
}

module_init(as7341_init);
module_exit(as7341_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("color_sensor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
