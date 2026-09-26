/*
 * ams_as7341.h
 *
 * code for AMS as7341 sensor
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * Description: code for AMS as7341 sensor
 * Author: hujianglei
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

#ifndef __AMS_AS7341_H__
#define __AMS_AS7341_H__
#include "color_sensor.h"

#ifndef AMS_PLATFORM_H
#define AMS_PLATFORM_H

#define CONFIG_AMS_LITTLE_ENDIAN        1
#ifdef CONFIG_AMS_LITTLE_ENDIAN
#define AMS_ENDIAN_1                    0
#define AMS_ENDIAN_2                    8
#else
#define AMS_ENDIAN_2                    0
#define AMS_ENDIAN_1                    8
#endif

#define P2RAM_OTP_1                   0x01
// Register map
#define AS7341_ENABLE_REG             0x80
#define AS7341_ATIME_REG              0x81
#define AS7341_WTIME_REG              0x83
#define AS7341_AILTL_REG              0x84
#define AS7341_AILTH_REG              0x85
#define AS7341_AIHTL_REG              0x86
#define AS7341_AIHTH_REG              0x87
#define AS7341_AUXID_REG              0x90
#define AS7341_REVID_REG              0x91
#define AS7341_ID_REG                 0x92
#define AS7341_STATUS_REG             0x93
#define AS7341_ASTATUS_REG            0x94
#define AS7341_ADATA0L_REG            0x95
#define AS7341_ADATA0H_REG            0x96
#define AS7341_ADATA1L_REG            0x97
#define AS7341_ADATA1H_REG            0x98
#define AS7341_ADATA2L_REG            0x99
#define AS7341_ADATA2H_REG            0x9A
#define AS7341_ADATA3L_REG            0x9B
#define AS7341_ADATA3H_REG            0x9C
#define AS7341_ADATA4L_REG            0x9D
#define AS7341_ADATA4H_REG            0x9E
#define AS7341_ADATA5L_REG            0x9F
#define AS7341_ADATA5H_REG            0xA0
#define AS7341_STATUS2_REG            0xA3
#define AS7341_STATUS3_REG            0xA4
#define AS7341_STATUS5_REG            0xA6
#define AS7341_STATUS6_REG            0xA7
#define AS7341_CFG0_REG               0xA9
#define AS7341_CFG1_REG               0xAA
#define AS7341_CFG3_REG               0xAC
#define AS7341_CFG4_REG               0xAD
#define AS7341_CFG6_REG               0xAF
#define AS7341_CFG8_REG               0xB1
#define AS7341_CFG9_REG               0xB2
#define AS7341_CFG10_REG              0xB3
#define AS7341_CFG11_REG              0xB4
#define AS7341_CFG12_REG              0xB5
#define AS7341_CFG14_REG              0xB7
#define AS7341_PERS_REG               0xBD
#define AS7341_GPIO_REG               0xBE
#define AS7341_ASTEPL_REG             0xCA
#define AS7341_ASTEPH_REG             0xCB
#define AS7341_AGC_GAIN_MAX_REG       0xCF
#define AS7341_ALS_CHANNEL_CTRL       0xD5
#define AS7341_AZ_CONFIG_REG          0xD6
#define AS7341_FD_CFG0                0xD7
#define AS7341_FD_CFG1                0xD8
#define AS7341_FD_CFG3                0xDA
#define AS7341_FD_STATUS              0xDB
#define AS7341_INTENAB_REG            0xF9
#define AS7341_CTRL_REG               0xFA
#define AS7341_FIFO_MAP               0xFC
#define AS7341_FIFO_LVL               0xFD
#define AS7341_FDADAL                 0xFE
#define AS7341_FDADAH                 0xFF

#define AS7341_CHAINCMD               0xDC
#define AS7341_CHAIN_SMUX             0xDF

#define AS7351_AUX_ID_VAL1            0x01
#define AS7351_AUX_ID_VAL2            0x08

// Register bits map
// ENABLE @ 0x80
#define PON                                      (0x01 << 0)
#define AEN                                      (0x01 << 1)
#define PEN                                      (0x01 << 2)
#define WEN                                      (0x01 << 3)
#define SMUXEN                                   (0x01 << 4)
#define FDEN                                     (0x01 << 6)

// AUXID @ 0x90
#define AUXID_MASK                               (0x0F << 0)

// REVID @ 0x91
#define REVID_MASK                               (0x07 << 0)

// ID_MASK @ 0x92
#define ID_MASK                                  (0x3F << 2)

// STATUS @ 0x93
#define SINT                                     (0x01 << 0)
#define CINT                                     (0x01 << 1)
#define FINT                                     (0x01 << 2)
#define AINT                                     (0x01 << 3)
#define ASAT                                     (0x01 << 7)

#define AINT_MASK                                (0x01 << 3)
#define ASAT_MASK                                (0x01 << 7)
#define SINT_MASK                                (0x01 << 0)
#define FINT_MASK                                (0x01 << 2)

// ASTATUS @0x94
#define AGAIN_STATUS_MASK                        (0x0F << 0)
#define ASAT_STATUS                              (0x01 << 7)

// STATUS2 @0xA3
#define ASAT_ANALOG                              (0x01 << 3)
#define ASAT_DIGITAL                             (0x01 << 4)
#define AVALID                                   (0x01 << 6)

// STATUS5 @0xA6
#define SINT_SMUX                                (0x01 << 2)
#define SINT_FD                                  (0x01 << 3)

// STATUS6 @0xA7
#define ALS_TRIGGER_ERROR                        (0x01 << 2)
#define FD_TRIG                                  (0x01 << 4)
#define OVTEMP                                   (0x01 << 5)
#define FIFO_OV                                  (0x01 << 7)

// CFG1 @0xAA
#define AGAIN_0_5X                               (0x00 << 0)
#define AGAIN_1X                                 (0x01 << 0)
#define AGAIN_2X                                 (0x02 << 0)
#define AGAIN_4X                                 (0x03 << 0)
#define AGAIN_8X                                 (0x04 << 0)
#define AGAIN_16X                                (0x05 << 0)
#define AGAIN_32X                                (0x06 << 0)
#define AGAIN_64X                                (0x07 << 0)
#define AGAIN_128X                               (0x08 << 0)
#define AGAIN_256X                               (0x09 << 0)
#define AGAIN_512X                               (0x0A << 0)
#define AGAIN_MASK                               (0x1F << 0)

// INTENAB_REG @0xF9
#define SIEN                                     (0x01 << 0)
#define CIEN                                     (0x01 << 1)
#define AIEN                                     (0x01 << 3)
#define ASIEN                                    (0x01 << 7)

// CONTROL_REG @0xFA
#define CLEAR_FIFO                               (0x01 << 1)
#define ALS_MANUAL_AZ                            (0x01 << 2)

#define AS7341_CAL_RATIO                         10000

// FD_CFG3@0xDA
#define FD_GAIN_0_5X                             (0x00 << 3)
#define FD_GAIN_1X                               (0x01 << 3)
#define FD_GAIN_2X                               (0x02 << 3)
#define FD_GAIN_4X                               (0x03 << 3)
#define FD_GAIN_8X                               (0x04 << 3)
#define FD_GAIN_16X                              (0x05 << 3)
#define FD_GAIN_32X                              (0x06 << 3)
#define FD_GAIN_64X                              (0x07 << 3)
#define FD_GAIN_128X                             (0x08 << 3)
#define FD_GAIN_256X                             (0x09 << 3)
#define FD_GAIN_512X                             (0x0A << 3)
#define FD_GAIN_MASK                             (0x1F << 3)
#define FD_TIME_HIGH_3_BIT_MASK                  (0x07 << 0)

#define AS7341_GAIN_SCALE                               1000
#define AS7341_ITIME_DEFAULT                              50
#define AS7341_CAL_ITIME_DEFAULT                         250

#define AS7341_ITIME_FAST                                 15
#define AMS_AS7341_GAIN_OF_GOLDEN                        256
#define AMS_AS7351_GAIN_OF_GOLDEN                        512

#define AMS_AS7341_FD_GAIN_OF_GOLDEN                     512
#define DEFAULT_ALS_GAIN_ID                                4
#define AMS_REPORT_LOG_COUNT_NUM                          30

#define AS7341_ITIME_FOR_FIRST_DATA                        3
#define AS7341_AGAIN_FOR_FIRST_DATA                        6
#define ONE_BYTE_LENGTH                                    8
#define INTEGER_BIT_SIZE                                  32

#define GAIN_LEVEL_1                                       1
#define GAIN_LEVEL_2                                       4
#define GAIN_LEVEL_3                                      16
#define GAIN_LEVEL_4                                      64
#define GAIN_LEVEL_5                                     256
#define GAIN_LEVEL_6                                     512

#define FD_GAIN_LEVEL_1                                    1
#define FD_GAIN_LEVEL_2                                    4
#define FD_GAIN_LEVEL_3                                   32
#define FD_GAIN_LEVEL_4                                  128
#define FD_GAIN_LEVEL_5                                  512

#define ALS_GAIN_VALUE_1                                   0 // 0.5
#define ALS_GAIN_VALUE_2                                   1
#define ALS_GAIN_VALUE_3                                   2
#define ALS_GAIN_VALUE_4                                   4
#define ALS_GAIN_VALUE_5                                   8
#define ALS_GAIN_VALUE_6                                  16
#define ALS_GAIN_VALUE_7                                  32
#define ALS_GAIN_VALUE_8                                  64
#define ALS_GAIN_VALUE_9                                 128
#define ALS_GAIN_VALUE_10                                256
#define ALS_GAIN_VALUE_11                                512

#define SMUX_DIR_WRITE                                  0x08
#define SMUX_DIR_READ                                   0x10
#define SMUX_WITHOUT_RAM_MSG_SIZE                         40
#define SMUX_NO_RAM_I2C_NUM                                2

#define FIFO_LEVEL_RATIO                                   2
#define FD_POLLING_TIME                                   22

#define DEFAULT_ASTEP                                    499
#define DEFAULT_FD_GAIN                                    4

#define AMS_REPORT_DATA_LEN                               10
#define RGBAP_CALI_DATA_NV_NUM                           383
#define AMS_AS7341_FLOAT_TO_FIX                        10000

#define AS7341_FD_LOW_LEVEL                               30
#define SATURATION_CHECK_PCT                               8

#define AMSDRIVER_ALS_ENABLE                               1
#define AMSDRIVER_ALS_DISABLE                              0

#define AMSDRIVER_FD_ENABLE                                1
#define AMSDRIVER_FD_DISABLE                               0

#define REPORT_FIFO_LEN                                  128

#define MAX_GAIN_ID                                       10
#define MAX_AUTOGAIN_CHECK_CNT                            10
#define MIN_AUTOGAIN_CHECK                                 9
#define MAX_AUTOGAIN_CHECK                                10
#define BUF_16_MAX_LEVEL5                                  2
#define BUF_16_MAX_LEVEL4                                  9
#define BUF_16_MAX_LEVEL3                                 36
#define BUF_16_MAX_LEVEL2                                288
#define BUF_RATIO_8_16BIT                                  2
#define ONE_BYTE                                           1
#define MS_2_US                                         1000
#define DOUBLE_BYTE                                        2
#define MAX_SATURATION                                 65535
#define MAX_ADC_COUNT                                  65535

#define HIGH_TIMER_OFFSET                                  5
#define LOW_TIMER_OFFSET                                   2
#define AUTOZERO_TIMER_OFFSET                             12
#define ITIME_TIMER_OFFSET                                40
#define AUTO_GAIN_ITIME                                    3

#define CALI_RGB_TIMER                                   120
#define FIRST_FLK_TIMER                                    6
#define MAX_BUFFER_SIZE                                  256
#define BUFSIZ                                           512
#define AMS_7341_DIST                                   0xF0
#define PIXEL_MAX                                         39
#define PIXEL_RATIO                                        2
#define FULL_BYTE                                       0xFF
#define HIGH_HALF_BYTE                                  0xF0
#define LOW_HALF_BYTE                                   0x0F
#define HALF_BYTE_SHIFT                                    4
#define SMUX_CHANNELS                                      6
#define SMUX_BUF_LEN                                      20
#define ASTEP_LEN                                          2
#define CHECK_F1F2_KVAL                                    2
#define DEFAULT_CALIB_AS7341_GAIN_ID                       9
#define DEFAULT_CALIB_AS7351_GAIN_ID                      10
// kun change the default GAIN from 9 to 10
#define LOW_AUTO_GAIN_VALUE                                3
#define AUTO_GAIN_DIVIDER                                  2
#define LOWEST_GAIN_ID                                     0
#define LOWEST_GAIN_VALUE                                500
#define SATURATION_LOW_PERCENT                            80
#define SATURATION_HIGH_PERCENT                          100
#define RAW_DATA_BYTE_NUM                                 10
#define MAX_LOG_COUNT                                     10

#define AS7341_REGADDR_CHAIN_SMUX                       0xDF
#define AS7341_REGADDR_CHAINCMD                         0xDC
#define CHAIN_CMD_SHIFT_ALS_6BITS                       0x46
#define MAX_FIFO_LEVEL                                   126
#define MAX_SMUX_WAIT_CYCLE                                5
#define CHIP_ID                                         0x24

#define ITIME_DIVIDER                                    720
#define ITIME_MULTIPLIER                                2000
#define ROUNDING_SCALAR                                   10
#define ROUNDING_ADD                                       5
#define IS_SATURATION                                      1
#define REG_BANK0                                       0x40
#define REG_BANK1                                       0x50
#define P2RAM_OPT_MASK                           (0x01 << 6)

// SMUX config
#define SMUX_F1_1     2
#define SMUX_F1_2     32
#define SMUX_F1       (uint16_t)(((uint16_t)SMUX_F1_2<<8) | (uint16_t)SMUX_F1_1)
#define SMUX_F2_1     10
#define SMUX_F2_2     25
#define SMUX_F2       (uint16_t)(((uint16_t)SMUX_F2_2<<8) | (uint16_t)SMUX_F2_1)
#define SMUX_F3_1      1
#define SMUX_F3_2     31
#define SMUX_F3       (uint16_t)(((uint16_t)SMUX_F3_2<<8) | (uint16_t)SMUX_F3_1)
#define SMUX_F4_1     11
#define SMUX_F4_2     26
#define SMUX_F4       (uint16_t)(((uint16_t)SMUX_F4_2<<8) | (uint16_t)SMUX_F4_1)
#define SMUX_F5_1     13
#define SMUX_F5_2     19
#define SMUX_F5       (uint16_t)(((uint16_t)SMUX_F5_2<<8) | (uint16_t)SMUX_F5_1)
#define SMUX_F6_1      8
#define SMUX_F6_2     29
#define SMUX_F6       (uint16_t)(((uint16_t)SMUX_F6_2<<8) | (uint16_t)SMUX_F6_1)
#define SMUX_F7_1     14
#define SMUX_F7_2     20
#define SMUX_F7       (uint16_t)(((uint16_t)SMUX_F7_2<<8) | (uint16_t)SMUX_F7_1)
#define SMUX_F8_1      7
#define SMUX_F8_2     28
#define SMUX_F8       (uint16_t)(((uint16_t)SMUX_F8_2<<8) | (uint16_t)SMUX_F8_1)
#define SMUX_CLEAR_1  17
#define SMUX_CLEAR_2  35
#define SMUX_CLEAR    (uint16_t)(((uint16_t)SMUX_CLEAR_2<<8) | SMUX_CLEAR_1)
#define SMUX_NIR      38
#define SMUX_FLICKER  39
#define SMUX_DARK     37

#define AS7341_CALGAIN  CAL_STATE_GAIN_5

enum channel_names {
	CHAN_F1 = 0,
	CHAN_F2 = 1,
	CHAN_F3 = 2,
	CHAN_F4 = 3,
	CHAN_F5 = 4,
	CHAN_F6 = 5,
	CHAN_F7 = 6,
	CHAN_F8 = 7,
	CHAN_NR = 8,
	CHAN_CL = 9
};

enum measurement_steps {
	STEP_FAST_AUTO_GAIN,
	STEP_FAST_MEASURE,
	STEP_DEFAULT_MEASURE
};

#define ams_mutex_lock(m) { \
	mutex_lock(m); \
}
#define ams_mutex_unlock(m) { \
	mutex_unlock(m); \
}

struct as7341_flc_ctx_t {
	UINT32 fd_gain;
	UINT16 fd_time_ms;
	UINT8  fd_smux_impact_start;
	UINT8  fd_smux_impact_end;
	struct mutex lock;
	bool first_fd_inte;
};

struct as7341_als_ctx_t {
	UINT16 saturation;
	UINT8 gain_id;
	UINT32 gain;
	UINT32 itime_ms;
	UINT16 astep;
	UINT8 atime;
	UINT8 status;
	UINT8 status2;
	UINT8 als_update;
	enum measurement_steps meas_steps;
	bool load_calibration;
	UINT8 smux_mode;
	UINT32 data[AMS_REPORT_DATA_LEN];
	UINT8 enabled;
};

struct as7341_calib_ctx_t {
	uint32_t target[AMS_REPORT_DATA_LEN];
	uint32_t ratio[AMS_REPORT_DATA_LEN];
	uint8_t reserved[16];
};

enum as7341_sensor_id_t {
	AMS_UNKNOWN_DEVICE,
	AMS_AS7341_REV0,
	AMS_AS7351_REV0,
	AMS_AS7341_LAST_DEVICE
};

enum tcs_rev_id_t {
	AMS_UNKNOWN_REV,
	AMS_AS7341_REVID = 0x00
};

enum smux_type {
	SMUX_DEFAULT = 0,
	SMUX_OTHER = 1,

	SMUX_NUM = 2
};

enum tcs_hal_algo_t {
	AMS_UNSUPPORT_ALGO,
};


struct as7341_reg_setting_t {
	UINT8 reg;
	UINT8 value;
};

enum as7341_power_mode_t {
	AMS_AS7341_FEATURE_ALS = (1 << 0),
	AMS_AS7341_FEATURE_LAST
};

struct as7341_ctx {
	enum as7341_sensor_id_t device_id;
	struct i2c_client *handle;
	struct as7341_als_ctx_t alg_ctx;
	struct as7341_flc_ctx_t flc_ctx;
	struct as7341_calib_ctx_t cal_ctx;
};

static UINT8 const as7341_ids[] = {
	CHIP_ID, // as7341 chip id val
};

static const struct as7341_reg_setting_t as7341_settings[] = {
	{ AS7341_ENABLE_REG, 0x00 }, // disable enable register
	{ AS7341_CTRL_REG, 0x00 },
	{ AS7341_CFG1_REG, AGAIN_4X },
	{ AS7341_CFG3_REG, 0x0C },
	{ AS7341_CFG8_REG, 0x90 },
	{ AS7341_CFG10_REG, 0xf2 },
	{ AS7341_CFG12_REG, 0x00 },
	{ AS7341_PERS_REG, 0x00 },
	{ AS7341_AZ_CONFIG_REG, 0x00 }, // disable az
	{ AS7341_FD_CFG0, 0xA1 }, // enable FD-FIO
	{ AS7341_FD_CFG1, 0x67 }, // fd_time=0.5ms, rate is 1khz, 0xb3 => 0x67
	{ AS7341_FD_CFG3, 0x31 }, // default fd_gain = 32x, from 0x30 to 0x31
	{ AS7341_FIFO_MAP, 0x00 }
};

static UINT32 const as7341_als_gain_lvl[] = {
	GAIN_LEVEL_1,
	GAIN_LEVEL_2,
	GAIN_LEVEL_3,
	GAIN_LEVEL_4,
	GAIN_LEVEL_5
};

static UINT16 const as7341_als_gain[] = {
	ALS_GAIN_VALUE_1, // actual gain is 0.5
	ALS_GAIN_VALUE_2,
	ALS_GAIN_VALUE_3,
	ALS_GAIN_VALUE_4,
	ALS_GAIN_VALUE_5,
	ALS_GAIN_VALUE_6,
	ALS_GAIN_VALUE_7,
	ALS_GAIN_VALUE_8,
	ALS_GAIN_VALUE_9,
	ALS_GAIN_VALUE_10,
	ALS_GAIN_VALUE_11
};

#endif
#endif // __AMS_AS7341_H__
