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

#ifndef __AMS_TCS3430_H__
#define __AMS_TCS3430_H__
#include "color_sensor.h"

#ifndef AMS_PLATFORM_H
#define AMS_PLATFORM_H

#define CONFIG_AMS_LITTLE_ENDIAN                   1
#ifdef CONFIG_AMS_LITTLE_ENDIAN
#define AMS_ENDIAN_1                               0
#define AMS_ENDIAN_2                               8
#else
#define AMS_ENDIAN_2                               0
#define AMS_ENDIAN_1                               8
#endif

#define AMS_TCS3430_LUX_AVERAGE_COUNT              8
#define AMS_TCS3430_CAL_AVERAGE                    1
#define AMS_TCS3430_HIGH                           0xFF
#define AMS_TCS3430_LOW                            0x00
#define TCS3430_INT_TIME_PER_CYCLE                 2778
#define TCS3430_INT_FACTOR                         1000
#define AMS_TCS3430_ATIME_TO_MS(x) \
	(((((x) + 1) * TCS3430_INT_TIME_PER_CYCLE) / TCS3430_INT_FACTOR))
#define AMS_TCS3430_MS_TO_ATIME(x) \
	((UINT8)(((x) * TCS3430_INT_FACTOR) / TCS3430_INT_TIME_PER_CYCLE))
#define AMS_TCS3430_ATIME_TO_CYCLES(x)             ((x) + 1)

#define AMS_TCS3430_MATRIX_ROW_SIZE                3
#define AMS_TCS3430_MATRIX_COL_SIZE                4
#define AMS_TCS3430_DEVICE_OFF_TO_IDLE_MS          10
#define TCS3430_PER_TICK                           (TCS3430_INT_TIME_PER_CYCLE)
#define AMS_TCS3430_ACTUAL_USEC(x) \
	((((x) + TCS3430_PER_TICK / 2) / TCS3430_PER_TICK) * TCS3430_PER_TICK)
#define AMS_TCS3430_ALS_USEC_TO_REG(x)             ((x) / TCS3430_PER_TICK)
#define AMS_TCS3430_ADC_COUNT_HIGH                 24000
#define AMS_TCS3430_ADC_COUNT_LOW                  100
#ifndef AMS_TCS3430_UINT_MAX_VALUE
#define AMS_TCS3430_UINT_MAX_VALUE                 (-1)
#endif
#define AMS_TCS3430_DEVICE_ID                      0xDC
#define AMS_TCS3430_DEVICE_ID_MASK                 0xFC
#define AMS_TCS3430_REV_ID                         0x01
#define AMS_TCS3430_REV_ID_MASK                    0x07
#define AMS_REPORT_DATA_LEN                        4
#define AMS_TCS3430_ADC_BYTES                      8
#define RGBAP_CALI_DATA_NV_NUM                     383
#define RGBAP_CALI_DATA_SIZE                       96
#define AMS_TCS3430_FLOAT_TO_FIX                   10000
#define FLOAT_TO_FIX_LOW                           100
#define AMS_TCS3430_CAL_THR                        2
#define TCS_LOW_THR                                50
#define TCS_HIGH_THR                               200
#define AP_COLOR_DMD_DELAY_TIME_MS                 30000
#define AMS_TCS3430_LOW_LEVEL                      100
#define AMS_TCS3430_NOMINAL_ATIME_DEFAULT          100
#define AMS_TCS3430_NOMINAL_AGAIN_DEFAULT          16
#define AMS_TCS3430_ATIME_DEFAULT                  700
#define AMS_TCS3430_AGAIN_DEFAULT                  (64 * 1000)
#define AMS_TCS3430_GAIN_OF_GOLDEN                 64
#define AMS_REPORT_LOG_COUNT_NUM                   20
#define AMS_TCS3430_GAIN_COUNT                     5
#define AMS_TCS3430_GAIN_SCALE                     1000
#define AMSDRIVER_ALS_ENABLE                       1
#define AMSDRIVER_ALS_DISABLE                      0

#define AMS_MUTEX_LOCK(m) { \
	mutex_lock(m);\
}
#define AMS_MUTEX_UNLOCK(m) { \
	mutex_unlock(m);\
}

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

struct export_alsdata_t {
	uint32_t raw_x;
	uint32_t raw_y;
	uint32_t raw_z;
	uint32_t raw_ir;
	uint32_t raw_ir2;
};

#ifdef CONFIG_AMS_OPTICAL_FLOAT_SUPPORT
struct tcs3430_matrix_data_t {
	float high_ir[AMS_TCS3430_MATRIX_ROW_SIZE][AMS_TCS3430_MATRIX_COL_SIZE];
	float low_ir[AMS_TCS3430_MATRIX_ROW_SIZE][AMS_TCS3430_MATRIX_COL_SIZE];
};
#endif

struct coefficients_t {
#ifdef CONFIG_AMS_OPTICAL_FLOAT_SUPPORT
	struct ams_tcs3430_matrix_data_t *matrix;
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

#ifdef CONFIG_AMS_OPTICAL_FLOAT_SUPPORT
struct cie_tristimulus {
	float x;
	float y;
	float z;
};
#endif
struct tcs3430_adc_data_t { // do not change the order of elements
	UINT16 z;
	UINT16 y;
	UINT16 ir1;
	UINT16 x;
	UINT16 ir2;
};

struct als_xyz_data_t {
	struct tcs3430_adc_data_t adc;
#ifdef CONFIG_AMS_OPTICAL_FLOAT_SUPPORT
	struct cie_tristimulus tristimulus;
	float chromaticity_x;
	float chromaticity_y;
	float lux;
	float ir;
	UINT32 cct;
#endif
};

enum tcs3430_als_status_t {
	AMS_3430_ALS_STATUS_IRQ  = (1 << 0),
	AMS_3430_ALS_STATUS_RDY  = (1 << 1),
	AMS_3430_ALS_STATUS_OVFL = (1 << 2)
};

struct tcs3430_als_data_set_t {
	struct tcs3430_adc_data_t data_array;
	UINT32 gain;
	UINT16 atime_ms;
	UINT8 status;
} ams_tcs3430_AlsDataSet_t;

struct tcs3430_als_ctx_t {
	struct tcs3430_als_data_set_t als_data;
	struct als_xyz_data_t results;
	UINT16 saturation;
};

enum tcs3430_device_id_t {
	AMS_UNKNOWN_DEVICE,
	AMS_TCS3430_REV0,
	AMS_TCS3430_REV1,
	AMS_TCS3430_LAST_DEVICE
};

struct tcs_3430_devid_t {
	UINT8 dev_id;
	UINT8 dev_id_mask;
	UINT8 dev_ref;
	UINT8 dev_ref_mask;
	enum tcs3430_device_id_t device;
};

enum tcs3430_regs_t {
	AMS_TCS3430_DEVREG_ENABLE,    /* 0x80 */
	AMS_TCS3430_DEVREG_ATIME,
	AMS_TCS3430_DEVREG_WTIME,
	AMS_TCS3430_DEVREG_AILTL,
	AMS_TCS3430_DEVREG_AILTH,
	AMS_TCS3430_DEVREG_AIHTL,
	AMS_TCS3430_DEVREG_AIHTH,
	AMS_TCS3430_DEVREG_PERS,
	AMS_TCS3430_DEVREG_CFG0,
	AMS_TCS3430_DEVREG_CFG1,
	AMS_TCS3430_DEVREG_REVID,
	AMS_TCS3430_DEVREG_ID,
	AMS_TCS3430_DEVREG_STATUS,    /* 0x93 */
	AMS_TCS3430_DEVREG_CH0DATAL,
	AMS_TCS3430_DEVREG_CH0DATAH,
	AMS_TCS3430_DEVREG_CH1DATAL,
	AMS_TCS3430_DEVREG_CH1DATAH,
	AMS_TCS3430_DEVREG_CH2DATAL,
	AMS_TCS3430_DEVREG_CH2DATAH,
	AMS_TCS3430_DEVREG_CH3DATAL,
	AMS_TCS3430_DEVREG_CH3DATAH,
	AMS_TCS3430_DEVREG_CFG2,
	AMS_TCS3430_DEVREG_CFG3,
	AMS_TCS3430_DEVREG_AZ_CONFIG,
	AMS_TCS3430_DEVREG_INTENAB,

	AMS_TCS3430_DEVREG_REG_MAX
};

enum tcs3430_reg_op_t {
	PON                   = 0x01,  /* register 0x80 */
	AEN                   = 0x02,
	WEN                   = 0x08,
	WLONG                 = 0x04,  /* register 0x8d */
	AMUX                  = 0x08,  /* register 0x90 */
	AGAIN_1               = 0x00,
	AGAIN_4               = 0x01,
	AGAIN_16              = 0x02,
	AGAIN_64              = 0x03,
	ASAT                  = 0x80,  /* register 0x93 */
	AINT                  = 0x10,
	CINT                  = 0x08,
	HGAIN                 = 0x10,  /* register 0x9F */
	INT_READ_CLEAR        = 0x80,  /* register 0xAB */
	SAI                   = 0x10,
	AZ_MODE               = 0x80,  /* register 0xD6 */
	ASIEN                 = 0x80,  /* register 0xDD */
	AIEN                  = 0x10,
	LAST_IN_ENUM_LIST
};

enum regmask_t {
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

struct reg_table_t {
	UINT8 address;
	UINT8 reset_val;
};

struct tcs3430_gain_thr_t {
	UINT32 low_thr;
	UINT32 high_thr;
};

enum tcs3430_feature_t {
	AMS_TCS3430_FEATURE_ALS   = (1 << 0),
	AMS_TCS3430_FEATURE_LAST
};

enum tcs3430_cfg_op_t {
	AMS_TCS3430_CONFIG_ENABLE,
	AMS_TCS3430_CONFIG_LAST
};

enum tcs3430_mode_t {
	AMS_TCS3430_MODE_OFF      = (0),
	AMS_TCS3430_MODE_ALS      = (1 << 0),
	AMS_TCS3430_MODE_UNKNOWN  /* must be in last position */
};

struct _calibration_t {
	UINT32 timebase_us;
	struct coefficients_t calibration;
};

struct tcs3430_ctx {
	enum tcs3430_device_id_t device_id;
	struct i2c_client *handle;
	enum tcs3430_mode_t mode;
	struct _calibration_t *system_cali_data;
	struct _calibration_t default_cali_data;
	struct tcs3430_als_ctx_t alg_ctx;
	UINT32 update_avai;
	bool first_inte;
};

struct tcs3430_dev_info_t {
	UINT32 memory_size;
	struct _calibration_t default_cal_data;
};

static struct tcs_3430_devid_t const tcs3430_devids[] = {
	{ AMS_TCS3430_DEVICE_ID, MASK_ID, AMS_TCS3430_REV_ID, MASK_REVID,
		AMS_TCS3430_REV1 },
	{ AMS_TCS3430_DEVICE_ID, MASK_ID, 0, MASK_REVID, AMS_TCS3430_REV0 },
	{ 0, 0, 0, 0, AMS_TCS3430_LAST_DEVICE }
};

static UINT32 const tcs3430_als_gain_conv[] = {
	(1 * AMS_TCS3430_GAIN_SCALE),
	(4 * AMS_TCS3430_GAIN_SCALE),
	(16 * AMS_TCS3430_GAIN_SCALE),
	(64 * AMS_TCS3430_GAIN_SCALE),
	(128 * AMS_TCS3430_GAIN_SCALE)
};

static UINT8 const tcs3430_als_gains[] = {
	1,
	4,
	16,
	64,
	128
};

static struct tcs3430_gain_thr_t const tcs3430_gain_thr[CAL_STATE_GAIN_LAST] = {
	{ 0, (100 * AMS_TCS3430_FLOAT_TO_FIX) }, // set threshold 1x to 0~100
	{ 5000, 20000 },
	{ 5000, 20000 },
	{ 5000, 20000 },
	{ 5000, 20000 },
};

static struct reg_table_t const tcs3430_reg_def[AMS_TCS3430_DEVREG_REG_MAX] = {
	{ 0x80, 0x01 },          /* DEVREG_ENABLE */
	{ 0x81, 0x23 },          /* DEVREG_ATIME */
	{ 0x83, 0x00 },          /* DEVREG_WTIME */
	{ 0x84, 0x00 },          /* DEVREG_AILTL */
	{ 0x85, 0x00 },          /* DEVREG_AILTH */
	{ 0x86, 0x00 },          /* DEVREG_AIHTL */
	{ 0x87, 0x00 },          /* DEVREG_AIHTH */
	{ 0x8C, 0x00 },          /* DEVREG_PERS */
	{ 0x8D, 0x80 },          /* DEVREG_CFG0 */
	{ 0x90, 0x00 },          /* DEVREG_CFG1 */
	{ 0x91, AMS_TCS3430_REV_ID },    /* DEVREG_REVID */
	{ 0x92, AMS_TCS3430_DEVICE_ID }, /* DEVREG_ID */
	{ 0x93, 0x00 },          /* DEVREG_STATUS */
	{ 0x94, 0x00 },          /* DEVREG_CH0DATAL */
	{ 0x95, 0x00 },          /* DEVREG_CH0DATAH */
	{ 0x96, 0x00 },          /* DEVREG_CH1DATAL */
	{ 0x97, 0x00 },          /* DEVREG_CH1DATAH */
	{ 0x98, 0x00 },          /* DEVREG_CH2DATAL */
	{ 0x99, 0x00 },          /* DEVREG_CH2DATAH */
	{ 0x9A, 0x00 },          /* DEVREG_CH3DATAL */
	{ 0x9B, 0x00 },          /* DEVREG_CH3DATAH */
	{ 0x9F, 0x04 },          /* DEVREG_CFG2 */
	{ 0xAB, 0x00 },          /* DEVREG_CFG3 */
	{ 0xD6, 0x7F },          /* DEVREG_AZ_CONFIG */
	{ 0xDD, 0x00 },          /* DEVREG_INTENAB */
};

int amsdriver_remove(struct i2c_client *client);
int amsdriver_resume(struct device *dev);
int amsdriver_suspend(struct device *dev);
int amsdriver_probe(struct i2c_client *client, const struct i2c_device_id *idp);

#endif
#endif /* __AMS_TCS3430_H__ */
