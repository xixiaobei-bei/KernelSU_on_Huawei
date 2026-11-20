/*
 * rohm_bh1749.h
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

#ifndef __ROHM_BH1749_H__
#define __ROHM_BH1749_H__

#include "color_sensor.h"
#ifndef ROHM_PLATFORM_H
#define ROHM_PLATFORM_H

/* BH1749 REGESTER */
#define ROHM_BH1749_SYSTEMCONTROL             0x40
#define ROHM_BH1749_MODECONTROL1              0x41
#define ROHM_BH1749_MODECONTROL2              0x42
#define ROHM_BH1749_RED_DATA                  0x50
#define ROHM_BH1749_GREEN_DATA                0x52
#define ROHM_BH1749_BLUE_DATA                 0x54
#define ROHM_BH1749_IR_DATA                   0x58
#define ROHM_BH1749_GREEN2_DATA               0x5A
#define ROHM_BH1749_INTERRUPT                 0x60
#define ROHM_BH1749_INT_PERSISTENCE           0x61
#define ROHM_BH1749_THRED_HIGH                0x62
#define ROHM_BH1749_THRED_LOW                 0x64
#define ROHM_BH1749_MANUFACT_ID               0x92
#define ROHM_BH1749_ID                        0xE0

#define ROHM_BH1749_INT_SOURCE_GREEN          (1 << 2)  // Green channel
#define ROHM_BH1749_INT_ENABLE                1         // INT pin enable
#define ROHM_BH1749_INT_DISABLE               0         // INT pin disable
#define ROHM_BH1749_INT_STATUS_ACTIVE         (1 << 7)  // active status

#define SW_RESET                              (1 << 7)
#define INT_RESET                             (1 << 6)

#define MEASURE_120MS                         0x02
#define MEASURE_240MS                         0x03
#define MEASURE_35MS                          0x05
#define TIME_120MS                            120
#define TIME_240MS                            240
// 35 is not used for calculate lux, 30 will be used actually
#define TIME_35MS                             30

#define RGB_GAIN_SHIFT_VALUE                  3
#define RGB_GAIN_X1                           (0x01 << RGB_GAIN_SHIFT_VALUE)
#define RGB_GAIN_X32                          (0x03 << RGB_GAIN_SHIFT_VALUE)

#define IR_GAIN_SHIFT_VALUE                   5
// IR_GAIN_X1 0x20
#define IR_GAIN_X1                            (0x01 << IR_GAIN_SHIFT_VALUE)
// IR_GAIN_X32 0x60
#define IR_GAIN_X32                           (0x03 << IR_GAIN_SHIFT_VALUE)

#define GAIN_X1                               1
#define GAIN_X32                              32
// RGBC measurement is active
#define RGBC_EN_ON                            (1 << 4)
// RGBC measurement is inactive and becomes power down
#define RGBC_EN_OFF                           (0 << 4)
#define RGBC_VALID_HIGH                       (1 << 7)
#define BH1749_I2C_ADDRESS                    0x38 // 7 bits slave addr 011 1001

// GAIN change automaticly according to the current rgb ir raw data
#define AUTO_GAIN                             1
#define BH1749_GAIN_CHANGE_MAX                60000
#define BH1749_GAIN_CHANGE_MIN                100
#define TIME_GAIN_MASK                        0xF8 // reg41 bit0-2 is 0
#define RGB_GAIN_MASK                         0xE7 // reg41 bit3-4 is 0
#define IR_GAIN_MASK                          0x9F // reg41 bit5-6 is 0

#define TIME_GAIN_VALUE(a)                    ((a) & 0x07) // get bit0-2
// get bit3-4
#define RGB_GAIN_VALUE(a)                     (((a) & 0x18) >> RGB_GAIN_SHIFT_VALUE)
// get bit5-6
#define IR_GAIN_VALUE(a)                      (((a) & 0x60) >> IR_GAIN_SHIFT_VALUE)

// max and abs define
#define MY_MAX(a, b)                          (((a) > (b)) ? (a) : (b))
#define MY_ABS(a)                             (((a) >= 0) ? (a): (-a))

#define CONFIG_ROHM_LITTLE_ENDIAN             1
#ifdef CONFIG_ROHM_LITTLE_ENDIAN
#define ROHM_ENDIAN_1                         0
#define ROHM_ENDIAN_2                         8
#else
#define ROHM_ENDIAN_2                         0
#define ROHM_ENDIAN_1                         8
#endif

#define ROHM_BH1749_LUX_AVERAGE_COUNT         8
#define ROHM_BH1749_CAL_AVERAGE               1

#define ROHM_BH1749_HIGH                      0xFF
#define ROHM_BH1749_LOW                       0x00
#define BH1749_MATRIX_ROW_SIZE                3
#define BH1749_MATRIX_COL_SIZE                4

#define ROHM_BH1749_DEVICE_ID                 0xDC
#define ROHM_BH1749_DEVICE_ID_MASK            0xFC
#define ROHM_BH1749_REV_ID                    0x01
#define ROHM_BH1749_REV_ID_MASK               0x07
#define ROHM_REPORT_DATA_LEN                  4
#define ROHM_BH1749_ADC_BYTES                 6
#define ROHM_BH1749_ADC_IR_BYTES              2
#define RGBAP_CALI_DATA_NV_NUM                383
#define RGBAP_CALI_DATA_SIZE                  96
#define ROHM_BH1749_FLOAT_TO_FIX              10000
#define FLOAT_TO_FIX_LOW                      100
#define ROHM_BH1749_CAL_THR                   2
#define AP_COLOR_DMD_DELAY_TIME_MS            30000
#define HIGH_THRESHOLD                        100000
#define LOW_THRESHOLD                         1
#define ROHM_BH1749_LOW_LEVEL                 100
#define ROHM_BH1749_HIGH_LEVEL                60000
#define ROHM_BH1749_GAIN_OF_GOLDEN            32
#define ROHM_REPORT_LOG_COUNT_NUM             20
#define BH1749_GAIN_SCALE                     1000
#define LEFT_SHIFT_8                          8
#define DATA_TRANSFER_COFF                    (32 * 120)
#define ROHM_POLL_TIME                        150
#define ROHM_GAIN_SIZE                        4

#ifndef TRUE
#define TRUE                                  1
#define FALSE                                 0
#endif

#define ROHM_MUTEX_LOCK(m) { \
	mutex_lock(m); \
}
#define ROHM_MUTEX_UNLOCK(m) { \
	mutex_unlock(m); \
}

#define GAIN_COV(a)                           ((a) * BH1749_GAIN_SCALE)
#define RAW_DATA_TRANS(a, b, c) \
	(((a) * DATA_TRANSFER_COFF) / ((b) / BH1749_GAIN_SCALE) / (c))

#ifdef CONFIG_ROHM_OPTICAL_FLOAT_SUPPORT
struct bh1749_matrix_data_t {
	float high_ir[BH1749_MATRIX_ROW_SIZE][BH1749_MATRIX_COL_SIZE];
	float low_ir[BH1749_MATRIX_ROW_SIZE][BH1749_MATRIX_COL_SIZE];
};
#endif

struct raw_data_arg_t {
	unsigned int red;   // data value of red data from sensor
	unsigned int green; // data value of green data from sensor
	unsigned int blue;  // data value of blue data from sensor
	unsigned int ir;    // data value of ir data from sensor
};

struct coefficients_t {
#ifdef CONFIG_ROHM_OPTICAL_FLOAT_SUPPORT
	struct bh1749_matrix_data_t *matrix;
	float kx;
	float ky;
	float kz;
	float kir1;
	float high_ir_thresh;
	float left_edge;
	float right_edge;
#endif
	UINT16 nominal_atime;
	UINT8 nominal_again;
};

#ifdef CONFIG_ROHM_OPTICAL_FLOAT_SUPPORT
struct cie_tristimulus {
	float x;
	float y;
	float z;
};
#endif
struct bh1749_adc_data_t { /* do not change the order of elements */
	UINT16 z;
	UINT16 y;
	UINT16 ir1;
	UINT16 x;
	UINT16 ir2;
};

struct bh1749_als_xyz_data_t {
	struct bh1749_adc_data_t adc;
#ifdef CONFIG_ROHM_OPTICAL_FLOAT_SUPPORT
	struct cie_tristimulus tristimulus;
	float chromaticity_x;
	float chromaticity_y;
	float lux;
	float ir;
	UINT32 cct;
#endif
};

struct bh1749_als_data_set_t {
	struct bh1749_adc_data_t data_array;
	UINT32 gain;
	UINT32 gain_ir;
	UINT16 atime_ms;
	UINT8 status;
};

struct bh1749_alg_ctx_t {
	struct bh1749_als_data_set_t als_data;
	struct bh1749_als_xyz_data_t results;
	UINT16 saturation;
};

enum bh1749_device_id_t {
	ROHM_UNKNOWN_DEVICE,
	ROHM_BH1749_REV0,
	ROHM_BH1749_REV1,
	ROHM_BH1749_LAST_DEVICE
};


enum bh1749_reg_masks_t {
	MASK_PON              = 0x01, /* register 0x80 */
	MASK_AEN              = 0x02,
	MASK_WEN              = 0x08,
	MASK_ATIME            = 0xff, /* register 0x81 */
	MASK_WTIME            = 0xff, /* register 0x83 */
	MASK_AILTL            = 0xFF, /* register 0x84 */
	MASK_AILTH            = 0xFF, /* register 0x85 */
	MASK_AIHTL            = 0xFF, /* register 0x86 */
	MASK_AIHTH            = 0xFF, /* register 0x87 */
	MASK_APERS            = 0x0F, /* register 0x8c */
	MASK_WLONG            = 0x04, /* register 0x8d */
	MASK_AMUX             = 0x08, /* register 0x90 */
	MASK_AGAIN            = 0x03,
	MASK_REVID            = 0x07, /* register 0x91 */
	MASK_ID               = 0xFC, /* register 0x92 */
	MASK_ASAT             = 0x80, /* register 0x93 */
	MASK_AINT             = 0x10,
	MASK_CINT             = 0x08,
	MASK_CH0DATAL         = 0xFF, /* register 0x94 */
	MASK_CH0DATAH         = 0xFF, /* register 0x95 */
	MASK_CH1DATAL         = 0xFF, /* register 0x96 */
	MASK_CH1DATAH         = 0xFF, /* register 0x97 */
	MASK_CH2DATAL         = 0xFF, /* register 0x98 */
	MASK_CH2DATAH         = 0xFF, /* register 0x99 */
	MASK_CH3DATAL         = 0xFF, /* register 0x9A */
	MASK_CH3DATAH         = 0xFF, /* register 0x9B */
	MASK_HGAIN            = 0x10, /* register 0x9F */
	MASK_INT_RD_CLEAR     = 0x80, /* register 0xAB */
	MASK_SAI              = 0x10,
	MASK_AZ_MODE          = 0x80, /* register 0xD6 */
	MASK_AZ_NTH_ITERATION = 0x7F,
	MASK_ASIEN            = 0x80, /* register 0xDD */
	MASK_AIEN             = 0x10,
	MASK_LAST_IN_ENUMLIST
};

enum bh1749_power_feature_t {
	BH1749_FEATURE_ALS = (1 << 0),
	BH1749_FEATURE_LAST
};

enum bh1749_rohm_mode_t {
    ROHM_BH1749_MODE_OFF = 0,
    ROHM_BH1749_MODE_ALS = (1 << 0),
    ROHM_BH1749_MODE_UNKNOWN  // must be in last position
} ;

struct _calibration_data_t {
	UINT32 timebase_us;
	struct coefficients_t cali_data;
};

struct bh1749_ctx_t {
	enum bh1749_device_id_t device_id;
	struct i2c_client *handle;
	enum bh1749_rohm_mode_t mode;
	struct _calibration_data_t *system_cal_data;
	struct _calibration_data_t default_cal_data;
	struct bh1749_alg_ctx_t alg_ctx;
	UINT32 rgb_available;
	bool first_inte;
};

struct bh1749_device_info_t {
	UINT32 memory_size;
	struct _calibration_data_t default_cal_data;
};


#define ROHMDRIVER_ALS_ENABLE 1
#define ROHMDRIVER_ALS_DISABLE 0

struct export_als_data_t {
	uint32_t raw_x;
	uint32_t raw_y;
	uint32_t raw_z;
	uint32_t raw_ir;
	uint32_t raw_ir2;
};

static UINT32 const bh1749_als_gain_conv[] = {
	1 * BH1749_GAIN_SCALE,
	32 * BH1749_GAIN_SCALE
};

static UINT8 const bh1749_als_gains[] = {
	1,
	32
};

#define ROHM_PORT_get_timestamp_usec(x)

int rohmdriver_remove(struct i2c_client *client);
int rohmdriver_resume(struct device *dev);
int rohmdriver_suspend(struct device *dev);
int rohmdriver_probe(struct i2c_client *client,
	const struct i2c_device_id *idp);

#endif
#endif /* __ROHM_BH1749_H__ */
