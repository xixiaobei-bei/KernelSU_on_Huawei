/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: sensor config header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "sensor_detect.h"

#define EXTEND_DATA_TYPE_IN_DTS_BYTE        0
#define EXTEND_DATA_TYPE_IN_DTS_HALF_WORD   1
#define EXTEND_DATA_TYPE_IN_DTS_WORD        2

#define HALL_COVERD 1

#define SENSOR_VOLTAGE_3_3V  3300000
#define SENSOR_VOLTAGE_3_2V  3200000
#define SENSOR_VOLTAGE_3_1V  3100000
#define SENSOR_VOLTAGE_3V    3000000
#define SENSOR_VOLTAGE_1V8   1800000

#define NV_READ_TAG 1
#define NV_WRITE_TAG 0

#define PS_CALIDATA_NV_NUM 334
#define PS_CALIDATA_NV_SIZE 52
#define TOF_CALIDATA_NV_SIZE 47
#define TOF_ZERO_CALIBRATE_ADDR 28
#define TOF_6CM_CALIBRATE_ADDR 32
#define TOF_10CM_CALIBRATE_ADDR 36
#define TOF_60CM_CALIBRATE_ADDR 40
#define ALS_CALIDATA_L_R_NUM 175
#define ALS_CALIDATA_L_R_SIZE 12
#define ALS_CALIDATA_NV_NUM 339
#define ALS_CALIDATA_NV_TOTAL_SIZE 72
#define ALS_CALIDATA_NV_ONE_SIZE 24
#define ALS_CALIDATA_NV_ALS1_ADDR ALS_CALIDATA_NV_ONE_SIZE
#define ALS_CALIDATA_NV_ALS2_ADDR (ALS_CALIDATA_NV_ONE_SIZE * 2)
#define ALS_CALIDATA_NV_SIZE 12
#define ALS_CALIDATA_NV_SIZE_WITH_DARK_NOISE_OFFSET 14
#define GYRO_CALIDATA_NV_NUM 341
#define GYRO_TEMP_CALI_NV_NUM 377
#define GYRO_CALIDATA_NV_SIZE 72
#define GYRO_TEMP_CALI_NV_SIZE 56
#define HANDPRESS_CALIDATA_NV_NUM 354
#define HANDPRESS_CALIDATA_NV_SIZE 24
#define AIRPRESS_CALIDATA_NV_NUM 332
#define AIRPRESS_CALIDATA_NV_SIZE 4
#define AIRPRESS_CALIDATA_NV_SIZE_WITH_AIRTOUCH 100
#define CAP_PROX_CALIDATA_NV_NUM 310
#define CAP_PROX_CALIDATA_NV_SIZE 28
#define ALS_TP_CALIDATA_NV1_NUM 403
#define ALS_TP_CALIDATA_NV1_SIZE 104
#define ALS_TP_CALIDATA_NV2_NUM 404
#define ALS_TP_CALIDATA_NV2_SIZE 104
#define ALS_TP_CALIDATA_NV3_NUM 405
#define ALS_TP_CALIDATA_NV3_SIZE 104
#define ACC_OFFSET_NV_NUM 307
#define ACC_OFFSET_NV_SIZE 60
#define MAG_CALIBRATE_DATA_NV_NUM 233
#define MAG_CALIBRATE_DATA_NV_SIZE (MAX_MAG_CALIBRATE_DATA_LENGTH)
#define MAG_FOLDER_CALIBRATE_DATA_NV_SIZE (MAX_MAG_FOLDER_CALIBRATE_DATA_LENGTH)
#define MAG_AKM_CALIBRATE_DATA_NV_SIZE (MAX_MAG_AKM_CALIBRATE_DATA_LENGTH)
#define MAG_MAX_CALIB_NV_SIZE MAG_AKM_CALIBRATE_DATA_NV_SIZE
#define VIB_CALIDATA_NV_NUM 337
#define VIB_CALIDATA_NV_SIZE 12
#define VIB_CALIDATA_NV_NAME "VIBCAL"
#define ALS_UNDER_TP_CALDATA_LEN 59
#define ALS_UNDER_TP_A_INDEX 4
#define ALS_UNDER_TP_A_COUNT 25
#define ALS_UNDER_TP_B_INDEX 29
#define ALS_UNDER_TP_B_COUNT 30
#define MAX_HP_OFFSET_DATA_LENGTH 24

#define ACC1_OFFSET_NV_NUM 410
#define ACC1_OFFSET_NV_SIZE 60
#define ACC1_NV_NAME  "GSENSOR1"

#define GYRO1_OFFSET_NV_NUM 411
#define GYRO1_OFFSET_NV_SIZE 72
#define GYRO1_NV_NAME  "GYRO1"

#define MAG1_OFFSET_NV_NUM 412
#define MAG1_OFFSET_NV_SIZE 12
#define MAG1_NV_NAME  "MSENSOR1"

#define CAP_PROX1_CALIDATA_NV_NUM 413
#define CAP_PROX1_CALIDATA_NV_SIZE 28
#define CAP_PROX1_NV_NAME  "CSENSOR1"

#define ALS_UD_CALI_LEN 4
#define ALS_UNDER_TP_RAWDATA_LEN 4
#define ALS_UNDER_TP_RGB_DATA_LEN 16

#define ALS_MMI_PARA_LEN 2
#define ALS_MMI_LUX_MIN_ID 0
#define ALS_MMI_LUX_MIN_VAL 30
#define ALS_MMI_LUX_MAX_ID 1
#define ALS_MMI_LUX_MAX_VAL 320
#define ALS_RES_SIZE 6
#define ALS_SUB_BLOCK_SIZE 13
#define ALS_DBV_SIZE 60
#define ALS_GAMMA_SIZE 32
#define ALS_TP_CALIDATA_NV4_NUM 442
#define ALS_TP_CALIDATA_NV4_SIZE 104
#define ALS_TP_CALIDATA_NV5_NUM 443
#define ALS_TP_CALIDATA_NV5_SIZE 104
#define ALS_TP_CALIDATA_NV6_NUM 444
#define ALS_TP_CALIDATA_NV6_SIZE 104
#define ALS_UNDER_TP_CALDATA_GAMMA_LEN 115

enum ALS_SENSNAME {
	APDS9922 = 1,
	LTR578 = 2,
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

struct press_alg_result {
	int slope; /* calibrated para */
	int base_press; /* based airpress value */
	short max_press; /* max airpress value in down event */
	short raise_press; /* airpress value without touch */
	short min_press; /* min value of airpress */
	short temp; /* temperature */
	short speed; /* time of down */
	char result_flag; /* calibrated result */
};

struct airpress_touch_calibrate_data {
	struct press_alg_result c_result; /* calibrated result */
	struct press_alg_result t_result; /* tested result */
};

struct als_under_tp_nv_type{
	unsigned int a[ALS_UNDER_TP_A_COUNT]; /* 25 area para */
	int b[ALS_UNDER_TP_B_COUNT]; /* 30 algrothm para */
};

struct als_under_tp_nv_type_gamma{
	uint32_t sub_block[ALS_SUB_BLOCK_SIZE];
	uint32_t res[ALS_RES_SIZE];
	uint32_t dbv[ALS_DBV_SIZE];
	uint32_t gamma[ALS_GAMMA_SIZE];
};

struct als_under_tp_calidata {
	uint16_t x; /* left_up x-aix */
	uint16_t y; /* left_up y-aix */
	uint16_t width;
	uint16_t length;
	union {
		struct als_under_tp_nv_type nv_type_defult;
		struct als_under_tp_nv_type_gamma nv_type_gamma;
	};
};

#define als_ud_nv_size(nv_type) \
	((nv_type == 1) ? (sizeof(struct als_under_tp_calidata)) : \
	(sizeof(struct als_under_tp_nv_type) + 8)) /* more 8 char */


enum {
	ALS_CHIP_NONE = 0,
	ALS_CHIP_APDS9922,
	ALS_CHIP_LTR578,
	ALS_CHIP_AVAGO_RGB,
	ALS_CHIP_VD6281,
	ALS_CHIP_ROHM_RGB,
	ALS_CHIP_BH1749,
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
	ALS_CHIP_AMS_TCS3701_RGB,
	ALS_CHIP_LTR2568,
	ALS_CHIP_STK3338,
	ALS_CHIP_VISHAY_VCNL36832,
	ALS_CHIP_LTR2594,
	ALS_CHIP_STK3638,
	ALS_CHIP_AMS_TCS3707_RGB,
	ALS_CHIP_VEML32185,
	ALS_CHIP_BU27006MUC,
	ALS_CHIP_STK32670,
	ALS_CHIP_STK32671,
	ALS_CHIP_LTR311,
	ALS_CHIP_SYH399,
	ALS_CHIP_AMS_TCS3708_RGB,
	ALS_CHIP_AMS_TCS3718_RGB,
	ALS_CHIP_AMS_TMD2702_RGB,
	ALS_CHIP_AMS_TSL2540_RGB,
};

struct als_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t als_dev_index;
	uint8_t als_first_start_flag;
	uint8_t als_opened_before_calibrate;
	uint8_t is_cali_supported;
	uint32_t chip_type;
	uint32_t table_id;
	s16 min_thres;
	s16 max_thres;
	uint8_t extend_para_set;
	uint8_t als_support_under_screen_cali;
	uint8_t tp_color_from_nv_flag;
	uint8_t als_under_tp_first_start_flag;
	uint8_t send_para_flag;
	uint8_t sem_als_ud_rgbl_block_flag;
	uint8_t als_ud_rgbl_block;
	uint8_t als_ud_need_reset;
	uint64_t als_rgb_pa_to_sh;
	uint16_t als_offset[ALS_CALIBRATE_DATA_LENGTH];
	int32_t als_always_on;
	uint16_t als_ud_cali_xy[ALS_UD_CALI_LEN];
	int32_t als_under_tp_cal_rawdata[ALS_UNDER_TP_RAWDATA_LEN];
	int32_t als_under_tp_rgb_data[ALS_UNDER_TP_RGB_DATA_LEN];
	int32_t als_mmi_para[ALS_MMI_PARA_LEN];
	uint8_t als_sensor_calibrate_data[ALS_CALIDATA_NV_ONE_SIZE];
	uint8_t als_ud_nv_type;
	enum ret_type als_calibration_res;
	als_run_stop_para_t als_ud_data_upload;
	struct als_under_tp_calidata als_under_tp_cal_data;
};

extern struct hisi_nve_info_user user_info;
extern u8 tp_manufacture;
extern int gsensor_offset[ACC_CALIBRATE_DATA_LENGTH];
extern uint8_t gsensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
extern int gsensor1_offset[ACC1_OFFSET_DATA_LENGTH];
extern uint8_t gsensor1_calibrate_data[ACC1_CALIBRATE_DATA_LENGTH];
extern char gyro_temperature_offset[GYRO_TEMP_CALI_NV_SIZE];
extern uint8_t gyro_temperature_calibrate_data[GYRO_TEMP_CALI_NV_SIZE];
extern int gyro_sensor_offset[GYRO_CALIBRATE_DATA_LENGTH];
extern uint8_t gyro_sensor_calibrate_data[GYRO_CALIDATA_NV_SIZE];
extern int gyro1_sensor_offset[GYRO1_CALIBRATE_DATA_LENGTH];
extern uint8_t gyro1_sensor_calibrate_data[GYRO1_OFFSET_NV_SIZE];
extern int32_t ps_sensor_offset[PS_CALIBRATE_DATA_LENGTH];
extern uint8_t ps_sensor_calibrate_data[MAX_SENSOR_CALIBRATE_DATA_LENGTH];
extern uint8_t msensor_calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
extern uint8_t msensor_folder_calibrate_data[MAX_MAG_FOLDER_CALIBRATE_DATA_LENGTH];
extern uint8_t msensor1_calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
extern uint8_t msensor_akm_calibrate_data[MAX_MAG_AKM_CALIBRATE_DATA_LENGTH];

int fill_extend_data_in_dts(struct device_node *dn, const char *name,
		unsigned char *dest, size_t max_size, int flag);
int mcu_i3c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx,
		uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
int mcu_i2c_rw(uint8_t i2c_i3c, uint8_t bus_num, uint8_t i2c_add, uint8_t *tx,
		uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
int mcu_spi_rw(uint8_t bus_num, union spi_ctrl ctrl, uint8_t *tx,
		uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
int combo_bus_trans(const struct sensor_combo_cfg *p_cfg, const uint8_t *tx,
		uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
void __dmd_log_report(int dmd_mark, const char *err_func, const char *err_msg);
void reset_calibrate_data(void);
int send_handpress_calibrate_data_to_mcu(void);
int mag_current_notify(void);
void read_tp_color_cmdline(void);
int write_calibrate_data_to_nv(int nv_number, int nv_size,
		const char *nv_name, const char *temp);
int read_calibrate_data_from_nv(int nv_number, int nv_size,
	const char *nv_name);
int send_calibrate_data_to_mcu(int tag, uint32_t subcmd, const void *data,
	int length, bool is_recovery);
int send_subcmd_data_to_mcu(int32_t tag, uint32_t subcmd, const void *data,
	uint32_t length, int32_t *err_no);
int send_subcmd_data_to_mcu_lock(int32_t tag, uint32_t subcmd,
	const void *data, uint32_t length, int32_t *err_no);
int mcu_save_calidata_to_nv(int tag, const int *para);
int open_send_current(int (*send) (int));
int close_send_current(void);

uint8_t *get_hp_offset(void);
u8 get_phone_color(void);
void set_phone_color(u8 color);
char *get_sensor_chip_info_address(enum sensor_detect_list index);
uint8_t *get_tof_sensor_calibrate_data(void);
int get_handpress_first_start_flag(void);
int send_vibrator_calibrate_data_to_mcu(void);
int send_tof_calibrate_data_to_mcu(void);

#endif /* __SENSORS_H__ */
