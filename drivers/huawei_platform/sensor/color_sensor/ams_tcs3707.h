/*
 * ams_tcs3707.h
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

#ifndef __AMS_TCS3707_H__
#define __AMS_TCS3707_H__
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

// Register map
#define TCS3707_ENABLE_REG              0x80
#define TCS3707_ATIME_REG               0x81
#define TCS3707_PTIME_REG               0x82
#define TCS3707_WTIME_REG               0x83
#define TCS3707_AILTL_REG               0x84
#define TCS3707_AILTH_REG               0x85
#define TCS3707_AIHTL_REG               0x86
#define TCS3707_AIHTH_REG               0x87
#define TCS3707_PILT0L_REG              0x88
#define TCS3707_PILT0H_REG              0x89
#define TCS3707_PILT1L_REG              0x8A
#define TCS3707_PILT1H_REG              0x8B
#define TCS3707_PIHT0L_REG              0x8C
#define TCS3707_PIHT0H_REG              0x8D
#define TCS3707_PIHT1L_REG              0x8E
#define TCS3707_PIHT1H_REG              0x8F
#define TCS3707_AUXID_REG               0x90
#define TCS3707_REVID_REG               0x91
#define TCS3707_ID_REG                  0x92
#define TCS3707_STATUS_REG              0x93
#define TCS3707_ASTATUS_REG             0x94
#define TCS3707_ADATA0L_REG             0x95
#define TCS3707_ADATA0H_REG             0x96
#define TCS3707_ADATA1L_REG             0x97
#define TCS3707_ADATA1H_REG             0x98
#define TCS3707_ADATA2L_REG             0x99
#define TCS3707_ADATA2H_REG             0x9A
#define TCS3707_ADATA3L_REG             0x9B
#define TCS3707_ADATA3H_REG             0x9C
#define TCS3707_ADATA4L_REG             0x9D
#define TCS3707_ADATA4H_REG             0x9E
#define TCS3707_ADATA5L_REG             0x9F
#define TCS3707_ADATA5H_REG             0xA0
#define TCS3707_PDATAL_REG              0xA1
#define TCS3707_PDATAH_REG              0xA2
#define TCS3707_STATUS2_REG             0xA3
#define TCS3707_STATUS3_REG             0xA4
#define TCS3707_STATUS5_REG             0xA6
#define TCS3707_STATUS6_REG             0xA7
#define TCS3707_CFG0_REG                0xA9
#define TCS3707_CFG1_REG                0xAA
#define TCS3707_CFG3_REG                0xAC
#define TCS3707_CFG4_REG                0xAD
#define TCS3707_CFG8_REG                0xB1
#define TCS3707_CFG10_REG               0xB3
#define TCS3707_CFG11_REG               0xB4
#define TCS3707_CFG12_REG               0xB5
#define TCS3707_CFG14_REG               0xB7
#define TCS3707_PCFG1_REG               0xB8
#define TCS3707_PCFG2_REG               0xB9
#define TCS3707_PCFG4_REG               0xBB
#define TCS3707_PCFG5_REG               0xBC
#define TCS3707_PERS_REG                0xBD
#define TCS3707_GPIO_REG                0xBE
#define TCS3707_POFFSETL_REG            0xC7
#define TCS3707_POFFSETH_REG            0xC8
#define TCS3707_ASTEPL_REG              0xCA
#define TCS3707_ASTEPH_REG              0xCB
#define TCS3707_AGC_GAIN_MAX_REG        0xCF
#define TCS3707_PXAVGL_REG              0xD0
#define TCS3707_PXAVGH_REG              0xD1
#define TCS3707_PBSLNL_REG              0xD2
#define TCS3707_PBSLNH_REG              0xD3
#define TCS3707_AZ_CONFIG_REG           0xD6
#define TCS3707_FD_CFG0                 0xD7
#define TCS3707_FD_CFG1                 0xD8
#define TCS3707_FD_CFG3                 0xDA
#define TCS3707_CALIB_REG               0xEA
#define TCS3707_CALIBCFG0_REG           0xEB
#define TCS3707_CALIBCFG1_REG           0xEC
#define TCS3707_CALIBCFG2_REG           0xED
#define TCS3707_CALIBSTAT_REG           0xEE
#define TCS3707_INTENAB_REG             0xF9
#define TCS3707_CTRL_REG                0xFA
#define TCS3707_FIFO_MAP                0xFC
#define TCS3707_FIFO_STATUS             0xFD
#define TCS3707_FDADAL                  0xFE
#define TCS3707_FDADAH                  0xFF

/* Register bits map */
// ENABLE @ 0x80
#define PON                             (0x01 << 0)
#define AEN                             (0x01 << 1)
#define PEN                             (0x01 << 2)
#define WEN                             (0x01 << 3)
#define FDEN                            (0x01 << 6)

// AUXID @ 0x90
#define AUXID_MASK                      (0x0F << 0)

// REVID @ 0x91
#define REVID_MASK                      (0x07 << 0)

// ID_MASK @ 0x92
#define ID_MASK                         (0x3F << 2)

// STATUS @ 0x93
#define SINT                            (0x01 << 0)
#define CINT                            (0x01 << 1)
#define AINT                            (0x01 << 3)
#define PINT0                           (0x01 << 4)
#define PINT1                           (0x01 << 5)
#define PSAT                            (0x01 << 6)
#define ASAT                            (0x01 << 7)

#define AINT_MASK                       (0x01 << 3)
#define ASAT_MASK                       (0x01 << 7)
#define SINT_MASK                       (0x01 << 0)

// ASTATUS @0x94
#define AGAIN_STATUS_MASK               (0x0F << 0)
#define ASAT_STATUS                     (0x01 << 7)

// STATUS2 @0xA3
#define ASAT_ANALOG                     (0x01 << 3)
#define ASAT_DIGITAL                    (0x01 << 4)
#define PVALID                          (0x01 << 5)
#define AVALID                          (0x01 << 6)

// STATUS3 @0xA4
#define PSAT_AMBIENT                    (0x01 << 0)
#define PSAT_REFLECTIVE                 (0x01 << 1)
#define PSAT_ADC                        (0x01 << 2)
#define STATUS3_RVED                    (0x01 << 3)
#define AINT_AILT                       (0x01 << 4)
#define AINT_AIHT                       (0x01 << 5)

// STATUS4 @0xA5
#define PINT0_PILT                      (0x01 << 0)
#define PINT0_PIHT                      (0x01 << 1)
#define PINT1_PILT                      (0x01 << 2)
#define PINT1_PIHT                      (0x01 << 3)

// STATUS6 @0xA7
#define INIT_BUSY                       (0x01 << 0)
#define SAI_ACTIVE                      (0x01 << 1)
#define ALS_TRIGGER_ERROR               (0x01 << 2)
#define PROX_TRIGGER_ERROR              (0x01 << 3)
#define OVTEMP_DETECTED                 (0x01 << 5)

// CFG0 @0xA9
#define ALS_TRIGGER_LONG                (0x01 << 2)
#define PROX_TRIGGER_LONG               (0x01 << 3)
#define LOWPOWER_IDLE                   (0x01 << 5)

// CFG1 @0xAA
#define AGAIN_0_5X                      (0x00 << 0)
#define AGAIN_1X                        (0x01 << 0)
#define AGAIN_2X                        (0x02 << 0)
#define AGAIN_4X                        (0x03 << 0)
#define AGAIN_8X                        (0x04 << 0)
#define AGAIN_16X                       (0x05 << 0)
#define AGAIN_32X                       (0x06 << 0)
#define AGAIN_64X                       (0x07 << 0)
#define AGAIN_128X                      (0x08 << 0)
#define AGAIN_256X                      (0x09 << 0)
#define AGAIN_512X                      (0x0A << 0)
#define AGAIN_MASK                      (0x1F << 0)

// CFG3 @0xAC
#define CFG3_RVED                       (0x0C << 0)
#define SAI                             (0x01 << 4)
#define HXTALK_MODE1                    (0x01 << 5)

// CFG4 @0xAD
#define GPIO_PINMAP_DEFAULT             (0x00 << 0)
#define GPIO_PINMAP_RVED                (0x01 << 0)
#define GPIO_PINMAP_AINT                (0x02 << 0)
#define GPIO_PINMAP_PINT0               (0x03 << 0)
#define GPIO_PINMAP_PINT1               (0x04 << 0)
#define GPIO_PINMAP_MASK                (0x07 << 0)
#define INT_INVERT                      (0x01 << 3)
#define INT_PINMAP_NORMAL               (0x00 << 4)
#define INT_PINMAP_RVED                 (0x01 << 4)
#define INT_PINMAP_AINT                 (0x02 << 4)
#define INT_PINMAP_PINT0                (0x03 << 4)
#define INT_PINMAP_PINT1                (0x04 << 4)
#define INT_PINMAP_MASK                 (0x07 << 4)

// CFG8_REG @0xB1
#define SWAP_PROX_ALS5                  (0x01 << 0)
#define ALS_AGC_ENABLE                  (0x01 << 2)
#define CONCURRENT_PROX_AND_ALS         (0x01 << 4)

// CFG10_REG @0xB3
#define ALS_AGC_LOW_HYST_12_5           (0x00 << 4)
#define ALS_AGC_LOW_HYST_25             (0x01 << 4)
#define ALS_AGC_LOW_HYST_37_5           (0x02 << 4)
#define ALS_AGC_LOW_HYST_50             (0x03 << 4)
#define ALS_AGC_LOW_HYST_MASK           (0x03 << 4)
#define ALS_AGC_HIGH_HYST_50            (0x00 << 6)
#define ALS_AGC_HIGH_HYST_62_5          (0x01 << 6)
#define ALS_AGC_HIGH_HYST_75            (0x02 << 6)
#define ALS_AGC_HIGH_HYST_87_5          (0x03 << 6)
#define ALS_AGC_HIGH_HYST_MASK          (0x03 << 6)

// CFG11_REG @0xB4
#define PINT_DIRECT                     (0x01 << 6)
#define AINT_DIRECT                     (0x01 << 7)

//CFG12_REG @0xB5
#define ALS_TH_CHANNEL_0                (0x00 << 0)
#define ALS_TH_CHANNEL_1                (0x01 << 0)
#define ALS_TH_CHANNEL_2                (0x02 << 0)
#define ALS_TH_CHANNEL_3                (0x03 << 0)
#define ALS_TH_CHANNEL_4                (0x04 << 0)
#define ALS_TH_CHANNEL_MASK             (0x07 << 0)

//CFG14_REG @0xB7
#define PROX_OFFSET_COARSE_MASK         (0x1F << 0)
#define EN_PROX_OFFSET_RANGE            (0x01 << 5)
#define AUTO_CO_CAL_EN                  (0x01 << 6)

//PCFG1_REG @0xB8
#define PROX_FILTER_1                   (0x00 << 0)
#define PROX_FILTER_2                   (0x01 << 0)
#define PROX_FILTER_4                   (0x02 << 0)
#define PROX_FILTER_8                   (0x03 << 0)
#define PROX_FILTER_MASK                (0x03 << 0)
#define PROX_FILTER_DOWNSAMPLE          (0x01 << 2)
#define HXTALK_MODE2                    (0x01 << 7)

// PCFG2_REG @0xB9
#define PLDRIVE0_MASK                   (0x7F << 0)

// PCFG4_REG @0xBB
#define PGAIN_1X                        (0x00 << 0)
#define PGAIN_2X                        (0x01 << 0)
#define PGAIN_4X                        (0x02 << 0)
#define PGAIN_8X                        (0x03 << 0)
#define PGAIN_MASK                      (0x03 << 0)

// PCFG5_REG @0xBC
#define PPULSE_MASK                     (0x3F << 0)
#define PPULSE_LEN_4US                  (0x00 << 6)
#define PPULSE_LEN_8US                  (0x01 << 6)
#define PPULSE_LEN_16US                 (0x02 << 6)
#define PPULSE_LEN_32US                 (0x03 << 6)
#define PPULSE_LEN_MASK                 (0x03 << 6)

// PERS_REG @0xBD
#define APERS_MASK                      (0x0F << 0)
#define PPERS_MASK                      (0x0F << 4)

// GPIO_REG @0xBE
#define GPIO_IN                         (0x01 << 0)
#define GPIO_OUT                        (0x01 << 1)
#define GPIO_IN_EN                      (0x01 << 2)
#define GPIO_INVERT                     (0x01 << 3)

// GAIN_MAX_REG @0xCF
#define AGC_AGAIN_MAX_MASK              (0x0F << 0) // 2^(AGC_AGAIN_MAX)

// CALIB_REG @0xEA
#define START_OFFSET_CALIB              (0x01 << 0)

// CALIBCFG0_REG @0xEB
#define DCAVG_ITERATIONS_MASK           (0x07 << 0) // 0 is skip, 2^(ITERATIONS)
#define BINSRCH_SKIP                    (0x01 << 3)
#define DCAVG_AUTO_OFFSET_ADJUST        (0x01 << 6)
#define DCAVG_AUTO_BSLN                 (0x01 << 7)

// CALIBCFG1_REG @0xEC
#define PXAVG_ITERATIONS_MASK           (0x07 << 0) // 0 is skip, 2^(ITERATIONS)
#define PXAVG_AUTO_BSLN                 (0x01 << 3)
#define PROX_AUTO_OFFSET_ADJUST         (0x01 << 6)

// CALIBCFG1_REG @0xED
#define BINSRCH_TARGET_3                (0x00 << 5)
#define BINSRCH_TARGET_7                (0x01 << 5)
#define BINSRCH_TARGET_15               (0x02 << 5)
#define BINSRCH_TARGET_31               (0x03 << 5)
#define BINSRCH_TARGET_63               (0x04 << 5)
#define BINSRCH_TARGET_127              (0x05 << 5)
#define BINSRCH_TARGET_255              (0x06 << 5)
#define BINSRCH_TARGET_511              (0x07 << 5)
#define BINSRCH_TARGET_MASK             (0x07 << 5)

// CALIBSTAT_REG @0xEE
#define CALIB_FINISHED                  (0x01 << 0)
#define OFFSET_ADJUSTED                 (0x01 << 1)
#define BASELINE_ADJUSTED               (0x01 << 2)

// INTENAB_REG @0xF9
#define SIEN                            (0x01 << 0)
#define CIEN                            (0x01 << 1)
#define AIEN                            (0x01 << 3)
#define PIEN0                           (0x01 << 4)
#define PIEN1                           (0x01 << 5)
#define PSIEN                           (0x01 << 6)
#define ASIEN                           (0x01 << 7)

// CONTROL_REG @0xFA
#define CLEAR_FIFO                      (0x01 << 1)
#define ALS_MANUAL_AZ                   (0x01 << 2)

#define TCS3707_CAL_RATIO               10000

// FD_CFG3@0xDA
#define FD_GAIN_0_5X                    (0x00 << 3)
#define FD_GAIN_1X                      (0x01 << 3)
#define FD_GAIN_2X                      (0x02 << 3)
#define FD_GAIN_4X                      (0x03 << 3)
#define FD_GAIN_8X                      (0x04 << 3)
#define FD_GAIN_16X                     (0x05 << 3)
#define FD_GAIN_32X                     (0x06 << 3)
#define FD_GAIN_64X                     (0x07 << 3)
#define FD_GAIN_128X                    (0x08 << 3)
#define FD_GAIN_256X                    (0x09 << 3)
#define FD_GAIN_512X                    (0x0A << 3)
#define FD_GAIN_MASK                    (0x1F << 3)
#define FD_TIME_HIGH_3_BIT_MASK         (0x07 << 0)

#define AMS_TCS3707_DEVICE_ID           0x18
#define AMS_TCS3707_REV_ID              0x11

#define TCS3707_GAIN_SCALE	        1000
#define TCS3707_ITIME_DEFAULT           100
#define AMS_TCS3707_GAIN_OF_GOLDEN      128
#define AMS_TCS3707_FD_GAIN_OF_GOLDEN   512

#define AMS_REPORT_LOG_COUNT_NUM        20

#define TCS3707_ITIME_FOR_FIRST_DATA    3
#define TCS3707_AGAIN_FOR_FIRST_DATA    (4 * TCS3707_GAIN_SCALE)
#define ONE_BYTE_LENGTH_8_BITS          8
#define GAIN_QUICKLY_FIX_LEVEL_1        13
#define GAIN_QUICKLY_FIX_LEVEL_2        50
#define GAIN_QUICKLY_FIX_LEVEL_3        200
#define GAIN_QUICKLY_FIX_LEVEL_4        800
#define AMS_TCS3707_LUX_AVERAGE_COUNT   8
#define AMS_TCS3707_CAL_AVERAGE         1
#define AMS_TCS3707_HIGH                0xFF
#define AMS_TCS3707_LOW                 0x00

#define GAIN_LEVEL_1                    1   // CHOOSE_GAIN_1
#define GAIN_LEVEL_2                    4   // CHOOSE_GAIN_4
#define GAIN_LEVEL_3                    16  // CHOOSE_GAIN_16
#define GAIN_LEVEL_4                    64  // CHOOSE_GAIN_64
#define GAIN_LEVEL_5                    256 // CHOOSE_GAIN_256

#define GAIN_CALI_LEVEL_1               1
#define GAIN_CALI_LEVEL_2               4
#define GAIN_CALI_LEVEL_3               16
#define GAIN_CALI_LEVEL_4               64
#define GAIN_CALI_LEVEL_5               256

#define FD_GAIN_LEVEL_1                 1
#define FD_GAIN_LEVEL_2                 4
#define FD_GAIN_LEVEL_3                 32
#define FD_GAIN_LEVEL_4                 128
#define FD_GAIN_LEVEL_5                 512

#define ALS_GAIN_VALUE_1                0 // actual gain is 0.5
#define ALS_GAIN_VALUE_2                1
#define ALS_GAIN_VALUE_3                2
#define ALS_GAIN_VALUE_4                4
#define ALS_GAIN_VALUE_5                8
#define ALS_GAIN_VALUE_6                16
#define ALS_GAIN_VALUE_7                32
#define ALS_GAIN_VALUE_8                64
#define ALS_GAIN_VALUE_9                128
#define ALS_GAIN_VALUE_10               256
#define ALS_GAIN_VALUE_11               512

#define FIFO_LEVEL_RATIO                2
#define TCS3707_GETBUF_LEN              32
#define FD_POLLING_TIME                 22

#define DEFAULT_ASTEP                   2780
#define CONFIG_AMS_FLICKER_DETECT

#define DEFAULT_FD_GAIN                 4
#define FD_TIME_RATIO                   1000
#define FD_RATIO_SCL                    10000
#define MAX_AMS_CALI_THRESHOLD          500000
#define TCS_DEFAULT_CAL_RAT             10000

#define AMS_REPORT_DATA_LEN             5
#define TCS3707_ADC_BYTES               10
#define RGBAP_CALI_DATA_NV_NUM          383
#define RGBAP_CALI_DATA_SIZE            96
#define AMS_TCS3707_FLOAT_TO_FIX        10000
#define FLOAT_TO_FIX_LOW                100
#define AMS_TCS3707_CAL_THR             2
#define AP_COLOR_DMD_DELAY_TIME_MS      30000

#define AMS_TCS3707_LOW_LEVEL           1000
#define TCS3707_FD_LOW_LEVEL            30
#define SATURATION_CHECK_PCT            8

#define AMSDRIVER_ALS_ENABLE            1
#define AMSDRIVER_ALS_DISABLE           0

#define AMSDRIVER_FD_ENABLE             1
#define AMSDRIVER_FD_DISABLE            0

#define REPORT_FIFO_LEN                 128
#define FIFO_LEVEL_16                   16
#define FIFO_LEVEL_32                   32
#define FIFO_LEVEL_48                   48
#define FIFO_LEVEL_64                   64
#define FIFO_LEVEL_80                   80
#define FIFO_LEVEL_96                   96
#define FIFO_LEVEL_112                  112

#define MAX_AUTOGAIN_CHECK_CNT          10
#define BUF_16_MAX_LEVEL5               2
#define BUF_16_MAX_LEVEL4               9
#define BUF_16_MAX_LEVEL3               36
#define BUF_16_MAX_LEVEL2               288
#define BUF_RATIO_8_16BIT               2
#define ONE_BYTE                        1
#define MS_2_US                         1000
#define DOUBLE_BYTE                     2
#define MAX_SATURATION                  65535
#define FIRST_RGB_TIMER                 12
#define RE_ENABLE_RGB_TIMER             105
#define CALI_RGB_TIMER                  120
#define FIRST_FLK_TIMER                 6
#define MAX_BUFFER_SIZE                 256
#define BUFSIZ                          512
#define NORMAL_ASTEP                    0
#define AMS_3408_3707_DIST              0xF0

#define AMS_MUTEX_LOCK(m) { \
	mutex_lock(m); \
}
#define AMS_MUTEX_UNLOCK(m) { \
	mutex_unlock(m); \
}

#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(x) ((sizeof(x)) / (sizeof((x)[0])))
#endif

struct tcs3707_adc_data_t { // do not change the order of elements
	UINT16 c;
	UINT16 r;
	UINT16 g;
	UINT16 b;
	UINT16 w;
};

struct tcs3707_flc_ctx_t {
	UINT32 fd_gain;
	UINT16 fd_time_ms;
	bool first_fd_inte;
};

struct tcs3707_als_ctx_t {
	UINT16 saturation;
	UINT32 gain;
	UINT16 itime_ms;
	UINT8 status;
	UINT8 als_update;
	bool first_inte;
	struct tcs3707_adc_data_t data;
};

enum tcs3707_sensor_id_t {
	AMS_UNKNOWN_DEVICE,
	AMS_TCS3707_REV0,
	AMS_TCS3707_LAST_DEVICE
};

enum tcs_rev_id_t {
	AMS_UNKNOWN_REV,
	AMS_TCS3707_REVID = 0x10,
	AMS_TCS3408_REVID = 0x50
};

enum tcs_hal_algo_t {
	AMS_UNSUPPORT_ALGO,
	AMS_TCS3707_NOR_ALGO,
	AMS_TCS3408_NOR_ALGO,
};

struct tcs3707_reg_setting_t {
	UINT8 reg;
	UINT8 value;
};

struct tcs3707_gain_thr_t {
	UINT32 low_thr;
	UINT32 high_thr;
};

enum tcs3707_power_mode_t {
	AMS_TCS3707_FEATURE_ALS = (1 << 0),
	AMS_TCS3707_FEATURE_LAST
};

struct tcs3707_ctx {
	enum tcs3707_sensor_id_t device_id;
	struct i2c_client *handle;
	struct tcs3707_als_ctx_t alg_ctx;
	struct tcs3707_flc_ctx_t flc_ctx;
};

struct export_alsdata_t {
	UINT32 raw_c;
	UINT32 raw_r;
	UINT32 raw_g;
	UINT32 raw_b;
	UINT32 raw_w;
};

struct tcs3707_cal_tar_t {
	UINT32 enable;
	UINT32 tar_c;
	UINT32 tar_r;
	UINT32 tar_g;
	UINT32 tar_b;
	UINT32 tar_w;
	UINT32 reserverd[3];
};

static UINT8 const tcs3707_ids[] = {
	0x18, // tcs3707 chip id val
};

static const struct tcs3707_reg_setting_t tcs3707_settings[] = {
	{ TCS3707_ATIME_REG,   0x3b },
	{ TCS3707_CFG0_REG,    0x00 },
	{ TCS3707_CFG1_REG,    AGAIN_4X }, //default again is 4X
	{ TCS3707_CFG3_REG,    0x0C },
	{ TCS3707_CFG4_REG,    0x80 }, // use fd_gain as the adata5's gain value
	{ TCS3707_CFG8_REG,    0x08 },
	{ TCS3707_CFG10_REG,   0xf2 },
	{ TCS3707_CFG11_REG,   0x40 },
	{ TCS3707_CFG12_REG,   0x00 },
	{ TCS3707_CFG14_REG,   0x00 },
	{ TCS3707_PERS_REG,    0x00 },
	{ TCS3707_GPIO_REG,    0x02 },
	{ TCS3707_ASTEPL_REG,  0xe7 }, //default astep is 2.78ms
	{ TCS3707_ASTEPH_REG,  0x03 },
	{ TCS3707_AGC_GAIN_MAX_REG, 0x99 },
	{ TCS3707_AZ_CONFIG_REG, 0x00 },
	{ TCS3707_FD_CFG0, 0x80 },
	{ TCS3707_FD_CFG1, 0x67 }, // fd_time=0.5ms, rate is 1khz, 0xb3 => 0x67
	{ TCS3707_FD_CFG3, 0x31 }, // default fd_gain = 32x, from 0x30 to 0x31
	{ TCS3707_FIFO_MAP, 0x00 }
};

static UINT32 const tcs3707_als_gain_conv[] = {
	(GAIN_CALI_LEVEL_1 * TCS3707_GAIN_SCALE),
	(GAIN_CALI_LEVEL_2 * TCS3707_GAIN_SCALE),
	(GAIN_CALI_LEVEL_3 * TCS3707_GAIN_SCALE),
	(GAIN_CALI_LEVEL_4 * TCS3707_GAIN_SCALE),
	(GAIN_CALI_LEVEL_5 * TCS3707_GAIN_SCALE)
};

static UINT32 const tcs3707_als_gain_lvl[] = {
	GAIN_LEVEL_1,
	GAIN_LEVEL_2,
	GAIN_LEVEL_3,
	GAIN_LEVEL_4,
	GAIN_LEVEL_5
};

static UINT16 const tcs3707_als_gain[] = {
	0, // actual gain is 0.5
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512
};

static const struct tcs3707_gain_thr_t tcs3707_gain_thr[CAL_STATE_GAIN_LAST] = {
	{ 0, MAX_AMS_CALI_THRESHOLD }, // set threshold 1x to 0~100
	{ 0, MAX_AMS_CALI_THRESHOLD }, // set threshold 1x to 0~100
	{ 5000, 20000 },
	{ 5000, 20000 },
	{ 5000, 20000 },
};

int amsdriver_remove(struct i2c_client *client);
int amsdriver_resume(struct device *dev);
int amsdriver_suspend(struct device *dev);
int amsdriver_probe(struct i2c_client *client, const struct i2c_device_id *idp);

#endif
#endif /* __AMS_TCS3707_H__ */
