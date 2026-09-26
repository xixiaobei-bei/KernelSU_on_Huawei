/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_H__
#define __ALS_PARA_TABLE_H__

#include "als_detect.h"

#define VCNL36832_PARA_SIZE 31
#define STK3338_PARA_SIZE 9
#define LTR2568_PARA_SIZE 10
#define VEML32185_PARA_SIZE 12
#define STK32670_PARA_SIZE 10
#define STK32671_PARA_SIZE 9
#define LTR311_PARA_SIZE 13
#define SYH399_PARA_SIZE 16
#define PINHOLE_PARA_SIZE 10
#define TMD2745_PARA_SIZE 10
#define RPR531_PARA_SIZE 16
#define APDS9999_PARA_SIZE 24
#define TMD3702_PARA_SIZE 29
#define TSL2540_PARA_SIZE 11
#define TCS3701_PARA_SIZE 32
#define TCS3707_PARA_SIZE 29
#define VCNL36658_PARA_SIZE 31
#define TSL2591_PARA_SIZE 22
#define BH1726_PARA_SIZE 16
#define MAX_PARA_SIZE 33
#define BH1749_PARA_SIZE 27
#define BU27006MUC_PARA_SIZE 27
#define VD6281_PARA_SIZE 8
#define LTR2594_PARA_SIZE 19
#define STK3638_PARA_SIZE 30

#define BH1745_MAX_THRESHOLD_NUM 23
#define BH1745_MIN_THRESHOLD_NUM 24

#define APDS9251_MAX_THRESHOLD_NUM 17
#define APDS9251_MIN_THRESHOLD_NUM 18

#define TMD3725_MAX_THRESHOLD_NUM 27
#define TMD3725_MIN_THRESHOLD_NUM 28

#define LTR582_MAX_THRESHOLD_NUM 24
#define LTR582_MIN_THRESHOLD_NUM 25

#define LTR578_APDS9922_MAX_THRESHOLD_NUM 8
#define LTR578_APDS9922_MIN_THRESHOLD_NUM 9

#define RPR531_MAX_THRESHOLD_NUM 14
#define RPR531_MIN_THRESHOLD_NUM 15

#define TMD2745_MAX_THRESHOLD_NUM 8
#define TMD2745_MIN_THRESHOLD_NUM 9

#define APDS9999_MAX_THRESHOLD_NUM 22
#define APDS9999_MIN_THRESHOLD_NUM 23

#define TMD3702_MAX_THRESHOLD_NUM 27
#define TMD3702_MIN_THRESHOLD_NUM 28

#define TCS3707_MAX_THRESHOLD_NUM 27
#define TCS3707_MIN_THRESHOLD_NUM 28

#define TCS3701_MAX_THRESHOLD_NUM 30
#define TCS3701_MIN_THRESHOLD_NUM 31

#define TSL2540_MAX_THRESHOLD_NUM 9
#define TSL2540_MIN_THRESHOLD_NUM 10

#define VCNL36658_MAX_THRESHOLD_NUM 28
#define VCNL36658_MIN_THRESHOLD_NUM 29

#define TSL2591_MAX_THRESHOLD_NUM 15
#define TSL2591_MIN_THRESHOLD_NUM 16

#define BH1726_MAX_THRESHOLD_NUM 14
#define BH1726_MIN_THRESHOLD_NUM 15

#define BH1749_MAX_THRESHOLD_NUM 25
#define BH1749_MIN_THRESHOLD_NUM 26

#define BU27006MUC_MAX_THRESHOLD_NUM 25
#define BU27006MUC_MIN_THRESHOLD_NUM 26

#define VD6281_MAX_THRESHOLD_NUM 6
#define VD6281_MIN_THRESHOLD_NUM 7

#define LTR2594_MAX_THRESHOLD_NUM 17
#define LTR2594_MIN_THRESHOLD_NUM 18

#define STK3638_MAX_THRESHOLD_NUM 28
#define STK3638_MIN_THRESHOLD_NUM 29

typedef struct _bh1745_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/* give to bh1745 rgb sensor use,output lux and cct will use these para */
	s16 bh745_para[25];
} bh1745_als_para_table;

typedef struct _apds9251_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to apds251 rgb sensor use,output lux and cct will use these para
	 * the apds251_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 apds251_para[21];
} apds9251_als_para_table;

typedef struct _tmd3725_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to tmd3725 rgb sensor use,output lux and cct will use these para
	 * the tmd3725_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 tmd3725_para[33];
} tmd3725_als_para_table;

typedef struct _ltr582_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to ltr582 rgb sensor use,output lux and cct will use these para
	 * the ltr582_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 ltr582_para[26];
} ltr582_als_para_table;

typedef struct _pinhole_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	/*
	 * modify the size of the array to pass more data
	 * the ph_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 pinhole_para[PINHOLE_PARA_SIZE];
} pinhole_als_para_table;

typedef struct _tmd2745_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	/*
	 * modify the size of the array to pass more data
	 * keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 als_para[TMD2745_PARA_SIZE];
} tmd2745_als_para_table;

typedef struct _rpr531_als_para_table{
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	s16 rpr531_para[RPR531_PARA_SIZE];
} rpr531_als_para_table;

typedef struct _apds9999_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to apds9999 rgb sensor use,
	 * output lux and cct will use these para.
	 * the apds9999_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 apds9999_para[APDS9999_PARA_SIZE];
} apds9999_als_para_table;

typedef struct _tmd3702_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to tmd3702 rgb sensor use,output lux and cct will use these para
	 * the tmd3702_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 tmd3702_para[TMD3702_PARA_SIZE];
} tmd3702_als_para_table;

typedef struct _tsl2540_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to tsl2540 rgb sensor use,output lux and cct will use these para
	 * the tsl2540_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 tsl2540_para[TSL2540_PARA_SIZE];
} tsl2540_als_para_table;

typedef struct _tcs3707_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tcs3707_para[TCS3707_PARA_SIZE];
} tcs3707_als_para_table;

typedef struct _tcs3701_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to tmd3701 rgb sensor use,output lux and cct will use these para
	 * the tmd3701_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 tcs3701_para[TCS3701_PARA_SIZE];
} tcs3701_als_para_table;

typedef struct _vcnl36658_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	/*
	 * give to vcnl36658 rgb sensor use,
	 * output lux and cct will use these para.
	 * the vcnl36658_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 vcnl36658_para[VCNL36658_PARA_SIZE];
} vcnl36658_als_para_table;

typedef struct _tsl2591_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 tsl2591_para[TSL2591_PARA_SIZE];
} tsl2591_als_para_table;

typedef struct _bh1726_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 bh1726_para[BH1726_PARA_SIZE];
} bh1726_als_para_table;

typedef struct _bh1749_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 bh1749_para[BH1749_PARA_SIZE];
} bh1749_als_para_table_t;

typedef struct _bu27006muc_als_para_table{
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 bu27006muc_para[BU27006MUC_PARA_SIZE];
} bu27006muc_als_para_table_t;

typedef struct _vd6281_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 vd6281_para[VD6281_PARA_SIZE];
} vd6281_als_para_table;

typedef struct _ltr2594_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	/* give to ltr2594 rgb sensor use,output lux and cct will use these para */
	s16 ltr2594_para[LTR2594_PARA_SIZE];
} ltr2594_als_para_table;

typedef struct _stk3638_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	/*
	 * give to STK3638 rgb sensor use,output lux and cct will use these para
	 * the STK3638_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE
	 */
	s16 stk3638_para[STK3638_PARA_SIZE];
} stk3638_als_para_table;

typedef struct {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t len;
	s16 als_para[MAX_PARA_SIZE];
} als_para_normal_table;

void set_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	als_para_normal_table *tp_color_table,
	int arraysize);
void select_stk3638_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_liteon_ltr2594_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void set_vd6281_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void set_tsl2591_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void set_bh1726_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void set_rpr531_als_extend_prameters(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void set_pinhole_als_extend_parameters(
	struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void set_tmd2745_als_extend_parameters(
	struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_rohm_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_rohmbh1749_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_rohmbu27006muc_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_avago_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_apds9999_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tmd3725_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tmd3702_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tcs3707_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tcs3701_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tcs3708_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tcs3718_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tmd2702_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_ams_tsl2540_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_vishay_vcnl36658_als_data(
	struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_liteon_ltr582_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);

#endif

