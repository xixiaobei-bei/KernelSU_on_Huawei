/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: some functions of sensorhub power
 * Author: qindiwen
 */

#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "sensor_detect.h"

#define EXTEND_DATA_TYPE_IN_DTS_BYTE        0
#define EXTEND_DATA_TYPE_IN_DTS_HALF_WORD   1
#define EXTEND_DATA_TYPE_IN_DTS_WORD        2
#define HALL_COVERD                         1
#define SENSOR_VOLTAGE_3V                   3000000
#define NV_READ_TAG                         1
#define NV_WRITE_TAG                        0
#define PS_CALIDATA_NV_NUM                  334
#define PS_CALIDATA_NV_SIZE                 12
#define TOF_CALIDATA_NV_SIZE                28
#define ALS_CALIDATA_NV_NUM                 339
#define ALS_CALIDATA_NV_SIZE                12
#define GYRO_CALIDATA_NV_NUM                341
#define GYRO_TEMP_CALI_NV_NUM               377
#define GYRO_CALIDATA_NV_SIZE               72
#define GYRO_TEMP_CALI_NV_SIZE              56
#define HANDPRESS_CALIDATA_NV_NUM           354
#define HANDPRESS_CALIDATA_NV_SIZE          24
#define AIRPRESS_CALIDATA_NV_NUM            332
#define AIRPRESS_CALIDATA_NV_SIZE           4
#define CAP_PROX_CALIDATA_NV_NUM            310
#define CAP_PROX_CALIDATA_NV_SIZE           28
#define PINHOLE_PARA_SIZE                   10
#define LTR578_PARA_SIZE                    10
#define STK3321_PARA_SIZE                   10
#define STK3235_PARA_SIZE                   9
#define SY3079_PARA_SIZE                    10
#define TMD2745_PARA_SIZE                   10
#define TMD2755_PARA_SIZE                   10
#define MN78911_PARA_SIZE                   13
#define SY3133_PARA_SIZE                    30
#define STK33562_PARA_SIZE                  15
#define RPR531_PARA_SIZE                    16
#define APDS9999_PARA_SIZE                  24
#define TMD3702_PARA_SIZE                   29
#define VCNL36658_PARA_SIZE                 31
#define TSL2591_PARA_SIZE                   15
#define BH1726_PARA_SIZE                    16
#define MAX_PARA_SIZE                       33
#define APDS9308_PARA_SIZE                  14
#define ACC_OFFSET_NV_NUM                   307
#define ACC_OFFSET_NV_SIZE                  60
#define MAG_CALIBRATE_DATA_NV_NUM           233
#define MAG_CALIBRATE_DATA_NV_SIZE          (MAX_MAG_CALIBRATE_DATA_LENGTH)
#define MAG_AKM_CALIBRATE_DATA_NV_SIZE      (MAX_MAG_AKM_CALIBRATE_DATA_LENGTH)
#define VIB_CALIDATA_NV_NUM                 337
#define VIB_CALIDATA_NV_SIZE                3
#define VIB_CALIDATA_NV_NAME                "VIBCAL"
#define SAR_ABOV_CH_NUM                     2
#define MAX_HP_OFFSET_DATA_LENGTH           24
#define SAR_ABOV_CH_0                       0
#define SAR_ABOV_CH_1                       1

enum ALS_SENSNAME {
	APDS9922 = 1,
	LTR578 = 2,
	STK3321 = 3,
	STK3235 = 4,
	SY3079 = 5,
	SYH399 = 6,
};

enum ret_type {
	RET_INIT = 0,
	SUC = 1,
	EXEC_FAIL,
	NV_FAIL,
	COMMU_FAIL,
	POSITION_FAIL,
	RET_TYPE_MAX
};

enum detect_state {
	DET_INIT = 0,
	DET_FAIL,
	DET_SUCC
};

enum {
	ALS_CHIP_NONE = 0,
	ALS_CHIP_APDS9922,
	ALS_CHIP_LTR578,
	ALS_CHIP_STK3321,
	ALS_CHIP_STK3235,
	ALS_CHIP_SY3079,
	ALS_CHIP_APDS9308,
	ALS_CHIP_AVAGO_RGB,
	ALS_CHIP_ROHM_RGB,
	ALS_CHIP_TMD2745,
	ALS_CHIP_RPR531,
	ALS_CHIP_AMS_TMD3725_RGB,
	ALS_CHIP_LITEON_LTR582,
	ALS_CHIP_APDS9999_RGB,
	ALS_CHIP_AMS_TMD3702_RGB,
	ALS_CHIP_VISHAY_VCNL36658,
	ALS_CHIP_TSL2591,
	ALS_CHIP_BH1726,
	ALS_CHIP_APDS9253_RGB,
	ALS_CHIP_LTR2568,
	ALS_CHIP_STK3338,
	ALS_CHIP_VISHAY_VCNL36832,
	ALS_CHIP_SYH399,
	ALS_CHIP_BU27006,
	ALS_CHIP_TCS3707,
	ALS_CHIP_LTR311,
	ALS_CHIP_TMD2755,
	ALS_CHIP_MN78911,
	ALS_CHIP_STK33562,
	ALS_CHIP_SY3133,
};

struct als_device_info {
	uint8_t als_first_start_flag;
	uint8_t is_cali_supported;
	uint32_t chip_type;
	uint32_t table_id;
	s16 min_thres;
	s16 max_thres;
	uint8_t tp_color_from_nv_flag;
	uint8_t send_para_flag;
	uint16_t als_offset[ALS_CALIBRATE_DATA_LENGTH];
	uint8_t als_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
};

enum {
	PS_CHIP_NONE = 0,
	PS_CHIP_PA224,
	PS_CHIP_TMD2620,
	PS_CHIP_APDS9110,
	PS_CHIP_TMD3725,
	PS_CHIP_LTR582,
	PS_CHIP_APDS9999,
	PS_CHIP_TMD3702,
	PS_CHIP_VCNL36658,
	PS_CHIP_LTR2568,
	PS_CHIP_TMD2755,
	PS_CHIP_MN78911,
	PS_CHIP_STK33562,
};

struct ps_device_info {
	uint8_t ps_first_start_flag;
	uint32_t chip_type;
};

struct sar_cap_proc_calibrate_data {
	uint16_t swap_flag[3];
	uint16_t cal_fact_base[3];
	uint16_t cal_offset[3];
	uint16_t digi_offset[3];
	uint16_t cap_prox_extend_data[2];
};

struct sar_cypress_calibrate_data {
	uint16_t sar_idac;
	uint16_t raw_data;
	uint16_t near_signaldata;
	uint16_t far_signaldata;
};

struct sar_semtech_calibrate_data {
	uint16_t offset;
	uint16_t diff;
};

struct sar_aw9610_calibrate_data {
	uint16_t offset;
	uint16_t diff;
};

struct sar_abov_calibrate_data {
	uint16_t offset[SAR_ABOV_CH_NUM];
	uint16_t diff[SAR_ABOV_CH_NUM];
};

union sar_calibrate_data {
	struct sar_cap_proc_calibrate_data cap_cali_data;
	struct sar_cypress_calibrate_data cypres_cali_data;
	struct sar_semtech_calibrate_data semtech_cali_data;
	struct sar_abov_calibrate_data abov_cali_data;
	struct sar_aw9610_calibrate_data aw9610_cali_data;
};

typedef struct _bh1745_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 bh745_para[25]; /* give to bh1745 rgb sensor use,output lux and cct will use these para */
} bh1745_als_para_table;

typedef struct _sy3133_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 sy3133_para[SY3133_PARA_SIZE]; /* give to sy3133 rgb sensor use,output lux and cct will use these para */
} sy3133_als_para_table;

typedef struct _bu27006_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 bu27006_para[27]; /* give to bu27006 rgb sensor use,output lux and cct will use these para */
} bu27006_als_para_table;

typedef struct _tcs3707_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tcs3707_para[29]; /* give to tcs3707 rgb sensor use,output lux and cct will use these para */
} tcs3707_als_para_table;

typedef struct _apds9251_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 apds251_para[21]; /* give to apds251 rgb sensor use,output lux and cct will use these par */
} apds9251_als_para_table; /* the apds251_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _tmd3725_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tmd3725_para[33]; /* give to tmd3725 rgb sensor use,output lux and cct will use these par */
} tmd3725_als_para_table; /* the tmd3725_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */


typedef struct _ltr582_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 ltr582_para[26]; /* give to ltr582 rgb sensor use,output lux and cct will use these par */
} ltr582_als_para_table; /* the ltr582_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _pinhole_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	s16 pinhole_para[PINHOLE_PARA_SIZE]; /* modify the size of the array to pass more data */
} pinhole_als_para_table; /* the ph_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _ltr578_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 para[LTR578_PARA_SIZE];
} ltr578_als_para_table;

typedef struct _stk3321_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 para[STK3321_PARA_SIZE];
} stk3321_als_para_table;

typedef struct _stk3235_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 para[STK3235_PARA_SIZE];
} stk3235_als_para_table;

typedef struct _sy3079_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t sens_name;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 para[SY3079_PARA_SIZE];
} sy3079_als_para_table;

typedef struct _tmd2745_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 als_para[TMD2745_PARA_SIZE]; /* modify the size of the array to pass more data */
} tmd2745_als_para_table; /* keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _tmd2755_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	float als_para[TMD2755_PARA_SIZE]; /* modify the size of the array to pass more data */
} tmd2755_als_para_table; /* keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _tmd2755_ps_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 ps_para[TMD2755_PARA_SIZE]; /* modify the size of the array to pass more data */
} tmd2755_ps_para_table;

typedef struct _mn78911_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	float als_para[MN78911_PARA_SIZE]; /* modify the size of the array to pass more data */
} mn78911_als_para_table; /* keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _mn78911_ps_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 ps_para[MN78911_PARA_SIZE]; /* modify the size of the array to pass more data */
} mn78911_ps_para_table; /* keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _stk33562_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	float als_para[STK33562_PARA_SIZE]; /* modify the size of the array to pass more data */
} stk33562_als_para_table; /* keep als_para size smaller than SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _stk33562_ps_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t tp_color;
	s16 ps_para[STK33562_PARA_SIZE]; /* modify the size of the array to pass more data */
} stk33562_ps_para_table;

typedef struct _rpr531_als_para_table {
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
	s16 apds9999_para[APDS9999_PARA_SIZE]; /* give to apds9999 rgb sensor use,output lux and cct will use these par */
} apds9999_als_para_table; /* the apds9999_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _tmd3702_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 tmd3702_para[TMD3702_PARA_SIZE]; /* give to tmd3702 rgb sensor use,output lux and cct will use these par */
} tmd3702_als_para_table; /* the tmd3702_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _vcnl36658_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_lcd_manufacture;
	uint8_t tp_color;
	s16 vcnl36658_para[31]; /* give to vcnl36658 rgb sensor use,output lux and cct will use these par */
} vcnl36658_als_para_table; /* the vcnl36658_para size must small SENSOR_PLATFORM_EXTEND_DATA_SIZE */

typedef struct _tsl2591_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	s16 tsl2591_para[TSL2591_PARA_SIZE];
} tsl2591_als_para_table;

typedef struct _bh1726_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	s16 bh1726_para[BH1726_PARA_SIZE];
} bh1726_als_para_table;

typedef struct _apds9308_als_para_table {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	s16 apds9308_para[APDS9308_PARA_SIZE];
} apds9308_als_para_table;

typedef struct {
	uint8_t phone_type;
	uint8_t phone_version;
	uint8_t tp_manufacture;
	uint8_t len;
	s16 als_para[MAX_PARA_SIZE];
} als_para_normal_table;

extern int fill_extend_data_in_dts(struct device_node *dn, const char *name, unsigned char *dest, size_t max_size, int flag);
extern int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern int mcu_spi_rw(uint8_t bus_num, union spi_ctrl ctrl, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern int combo_bus_trans(struct sensor_combo_cfg *p_cfg, uint8_t *tx, uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
extern void __dmd_log_report(int dmd_mark, const char *err_func, const char *err_msg);
extern int write_gyro_sensor_offset_to_nv(char *temp, int length);
extern int write_ps_sensor_offset_to_nv(char *temp, int length);
extern void reset_calibrate_data(void);
extern void reset_add_data(void);
extern int send_gsensor_calibrate_data_to_mcu(void);
extern int send_airpress_calibrate_data_to_mcu(void);
extern int send_gyro_calibrate_data_to_mcu(void);
extern int send_handpress_calibrate_data_to_mcu(void);
extern int mag_current_notify(void);
extern void read_tp_color_cmdline(void);
extern int write_calibrate_data_to_nv(int nv_number, int nv_size, const char *nv_name, const char *temp);
extern int write_gsensor_offset_to_nv(char *temp, int length);
extern int write_gyro_temperature_offset_to_nv(char *temp, int length);
extern int get_tpcolor_from_nv(void);

int read_calibrate_data_from_nv(int nv_number, int nv_size, char *nv_name);
struct ps_device_info *ps_get_device_info(int32_t tag);
int send_calibrate_data_to_mcu(int tag, uint32_t subcmd,
	const void *data, int length, bool is_recovery);
int send_subcmd_data_to_mcu(int32_t tag, uint32_t subcmd, const void *data,
	uint32_t length, int32_t *err_no);
uint8_t *get_hp_offset(void);
u8 get_phone_color(void);
char *get_sensor_chip_info_address(enum sensor_detect_list index);
uint8_t *get_ps_sensor_calibrate_data(void);
uint8_t *get_tof_sensor_calibrate_data(void);
int send_app_config_cmd(int tag, void *app_config, bool use_lock);
int send_vibrator_calibrate_data_to_mcu(void);
int send_cap_prox_calibrate_data_to_mcu(void);
int send_ps_calibrate_data_to_mcu(void);
int send_tof_calibrate_data_to_mcu(void);
int send_als_calibrate_data_to_mcu(void);
int send_gyro_temperature_offset_to_mcu(void);
int send_sar_add_data_to_mcu(void);
void select_ps_para(struct sensor_detect_manager *sm);

int open_send_current(int (*send) (int));
int close_send_current(void);
#endif /* __SENSORS_H__ */
