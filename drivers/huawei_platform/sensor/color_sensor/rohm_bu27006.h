/*
 * rohm_bu27006.h
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

#ifndef __ROHM_BU27006_H__
#define __ROHM_BU27006_H__

#include "color_sensor.h"

// BU27006 REGESTER
#define BU27006_SYSTEMCONTROL      0x40
#define BU27006_MODECONTROL1       0x41
#define BU27006_MODECONTROL2       0x42
#define BU27006_MODECONTROL3       0x43
#define BU27006_RED_DATA           0x50
#define BU27006_GREEN_DATA         0x52
#define BU27006_BLUE_DATA          0x54
#define BU27006_IR_DATA            0x56
#define BU27006_FLICKER_DATA       0x58
#define BU27006_FLICKER_COUNTER    0x5A
#define BU27006_FIFO_LEVEL         0x5B
#define BU27006_FIFO_DATA_LOW      0x5C
#define BU27006_FIFO_DATA_HIGH     0x5D
#define BU27006_MANUFACT_ID        0x92
#define BU27006_WHO_AM_I           0xE0
#define BU27006_I2C_ADDRESS        0x38

// alias
#define BU27006_RESET              BU27006_SYSTEMCONTROL
#define BU27006_RGB_GAIN_TIME      BU27006_MODECONTROL1
#define BU27006_FLC_GAIN           BU27006_MODECONTROL2
#define BU27006_ENABLE             BU27006_MODECONTROL3

// FIFO LEVEL (5Bh)
#define BU27006_FIFO_LEVEL_EMPTY   0   // FIFO Length = 0   (Empty)
#define BU27006_FIFO_LEVEL_WTM     64  // FIFO Length = 64  (Watermark)
#define BU27006_FIFO_LEVEL_FULL    100 // FIFO Length = 100 (Full)

// REG_MODECONTROL1(0x41)
#define MEASURE_55MS               0x01 // 0b01
#define MEASURE_100MS              0x02 // 0b10

#define RGB_GAIN_SHIFT_VALUE       4
#define RGB_GAIN_X1                0x00 // (0b00 << RGB_GAIN_SHIFT_VALUE)
#define RGB_GAIN_X4                0x10 // (0b01 << RGB_GAIN_SHIFT_VALUE)
#define RGB_GAIN_X32               0x20 // (0b10 << RGB_GAIN_SHIFT_VALUE)
#define RGB_GAIN_X128              0x30 // (0b11 << RGB_GAIN_SHIFT_VALUE)

// 55 is not used for calculate lux, 50 will be used actually
#define TIME_55MS                  50
#define TIME_100MS                 100

#define FLC_MODE_1KHZ              0x00
#define FLC_MODE_2KHZ              0x01
#define FLC_GAIN_SHIFT_VALUE       3
#define FLC_GAIN_X1                0x00 // (0b000 << FLC_GAIN_SHIFT_VALUE)
#define FLC_GAIN_X2                0x08 // (0b001 << FLC_GAIN_SHIFT_VALUE)
#define FLC_GAIN_X4                0x10 // (0b010 << FLC_GAIN_SHIFT_VALUE)
#define FLC_GAIN_X8                0x18 // (0b011 << FLC_GAIN_SHIFT_VALUE)
#define FLC_GAIN_X16               0x20 // (0b100 << FLC_GAIN_SHIFT_VALUE)
#define FLC_GAIN_X32               0x28 // (0b101 << FLC_GAIN_SHIFT_VALUE)

#define GAIN_X1                    1
#define GAIN_X2                    2
#define GAIN_X4                    4
#define GAIN_X8                    8
#define GAIN_X16                   16
#define GAIN_X32                   32
#define GAIN_X128                  128

#define BU27006_1K_MODE	           0
#define BU27006_2K_MODE	           1

#define RGB_GAIN_DEFAULT           GAIN_X4
#define RGB_TIME_DEFAULT           TIME_55MS

#define FLC_GAIN_DEFAULT           GAIN_X32
#define FLC_MODE_DEFAULT           BU27006_1K_MODE

// define parameter for register
#define SW_RESET                   (1 << 7)
// REG_MODECONTROL2(0x43)
#define RGBC_EN_OFF                (0 << 1) // RGBC measurement is inactive
#define RGBC_EN_ON                 (1 << 1) // RGBC measurement is active
#define RGBC_VALID_HIGH            (1 << 7)
#define FLC_VALID_HIGH             (1 << 6)
#define FLC_EN_OFF                 (0 << 0) // FLC measurement is inactive
#define FLC_EN_ON                  (1 << 0) // FLC measurement is active

// GAIN change automaticly according to the current rgb ir raw data
#define TIME_GAIN_MASK             0xF8 // reg41 bit0-2 is 0
#define RGB_GAIN_MASK              0xCF // reg41 bit4-5 is 0
#define FLC_GAIN_MASK              0xC7 // reg42 bit3-5 is 0
#define FLC_MODE_MASK              0xF7 // reg42 bit3-5 is 0

// define for lux count when cal rgbc
#define BU27006_CAL_AVG            1
#define ROHM_REPORT_DATA_LEN       4
#define BU27006_ADC_BYTES          8

#define RGBAP_CALI_DATA_NV_NUM     383
#define RGBAP_CALI_DATA_SIZE       96
#define ROHM_BU27006_FLOAT_TO_FIX  10000
#define BU27006_DEFAULT_CAL_RATIO  10000
#define FLOAT_TO_FIX_LOW           100

#define ROHM_BU27006_FACTOR        1000
#define ROHM_BU27006_LOW_SUM_LEVEL 500
#define ROHM_BU27006_LOW_LEVEL     100
#define ROHM_BU27006_HIGH_LEVEL    60000

#define BU27006_FLC_LOW_LEVEL      100
#define BU27006_FLC_1K_HIGH_LEVEL  6000 // 0x17FF(1Khz); 0x07FF(2Khz)
#define BU27006_FLC_2K_HIGH_LEVEL  2000 // 0x17FF(1Khz); 0x07FF(2Khz)

#define ROHM_REPORT_LOG_COUNT_NUM  20
#define FLC_LOG_REPORT_NUM         20000
#define LEFT_SHIFT_8               8
#define DATA_TRANSFER_COFF         1600 // 32X gain, 50ms atime
#define FLC_TRANSFER_COFF          512  // normalization

#define ROHM_FLC_POLL_TIME         20
#define ROHM_ALS_FIRST_POLL_TIME   60
#define ROHM_ALS_NORMAL_POLL_TIME  105
#define ROHMDRIVER_ALS_ENABLE      1
#define ROHMDRIVER_ALS_DISABLE     0
#define ROHMDRIVER_FLC_ENABLE      1
#define ROHMDRIVER_FLC_DISABLE     0
#define BU27006_RGB_GAIN_SIZE      4
#define RGB_MAX_CALI_THR           500000
#define ROHM_FLICKER_FIFO_SIZE     256
#define BU27006_DEF_GAIN_THR_LOW   5000
#define BU27006_DEF_GAIN_THR_HIGH  20000

#define TIME_GAIN_VALUE(a)         ((a) & (~TIME_GAIN_MASK)) // get bit0-2
// get bit4-5
#define RGB_GAIN_VALUE(a)          (((a) & (~RGB_GAIN_MASK)) >> 4)
// get bit3-5
#define FLC_GAIN_VALUE(a)          (((a) & (~FLC_GAIN_MASK)) >> 3)

// define diff cali threshold in diff gain
const struct cali_gain_thr_t bu27006_gain_thr[CAL_STATE_GAIN_5] = {
	{ 1, 0, RGB_MAX_CALI_THR },
	{ 4, 20, RGB_MAX_CALI_THR },
	{ 32, BU27006_DEF_GAIN_THR_LOW, BU27006_DEF_GAIN_THR_HIGH },
	{ 128, BU27006_DEF_GAIN_THR_LOW, BU27006_DEF_GAIN_THR_HIGH },
};

// define a common struct for rgb cali para
struct rgb_cali_tar_t {
	uint32_t rawdata;
	uint32_t high_th;
	uint32_t low_th;
	uint32_t again;
	uint32_t atime_ms;
	uint32_t target_val;
};

// structure to read data value from sensor
struct rawdata_arg_t {
	unsigned int red;   // data value of red data after calc
	unsigned int green; // data value of green data after calc
	unsigned int blue;  // data value of blue data after calc
	unsigned int ir;    // data value of ir data after calc
};

// define a rgb raw data struct
struct adc_data_t {
	uint16_t r;  // data value of red data from sensor
	uint16_t g;  // data value of green data from sensor
	uint16_t b;  // data value of blue data from sensor
	uint16_t ir; // data value of ir data from sensor
};

// define a flk raw data struct
struct bu27006_flk_data_t {
	unsigned int fifo_tmp[ROHM_FLICKER_FIFO_SIZE];
	unsigned int fifo_data[ROHM_FLICKER_FIFO_SIZE];
};

// define a common rgb all para stuct
struct bu27006_als_ctx_t {
	bool rgbc_valid;
	bool gain_changed;
	uint16_t gain;
	uint32_t gain_ir;
	uint16_t atime_ms;
	struct adc_data_t data;
};

// define a common flc all para stuct
struct bu27006_flc_ctx_t {
	bool fifo_full_flag;
	bool gain_changed;
	uint32_t gain_flc;
	uint32_t mode_flc;
	uint32_t fifo_cnt;
	struct bu27006_flk_data_t flc_data;
};

enum sensor_id_t {
	ROHM_UNKNOWN_DEVICE,
	ROHM_BU27006_REV0,
	ROHM_BU27006_REV1,
	ROHM_BU27006_LAST_DEVICE
};

enum bu27006_feature {
	BU27006_RGBC_READY = 1,
	BU27006_RGBC_NOT_READY
};

enum bu27006_power_mode_t {
	BU27006_MODE_OFF = 0,
	BU27006_MODE_ON = 1,
	BU27006_MODE_UNKNOWN // must be in last position
};

// define a struct for bu27006 sensor
struct bu27006_ctx {
	enum sensor_id_t device_id;
	struct i2c_client *handle;
	enum bu27006_power_mode_t als_mode;
	enum bu27006_power_mode_t flc_mode;
	struct bu27006_als_ctx_t alg_ctx;
	struct bu27006_flc_ctx_t flc_ctx;
	uint32_t update; // define for check whether rgbc data is ready
};

static void bu27006_get_als_mode(struct bu27006_ctx *ctx,
	enum bu27006_power_mode_t *mode);
static void bu27006_get_flc_mode(struct bu27006_ctx *ctx,
	enum bu27006_power_mode_t *mode);
static int bu27006_get_flc_data(struct bu27006_ctx *ctx);

#endif /* __ROHM_BU27006_H__ */
