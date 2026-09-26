/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: thermometer detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __THERM_DETECT_H__
#define __THERM_DETECT_H__

#include "sensor_sysfs.h"

#define THERM_DEV_COUNT_MAX 1
#define THERMO_CAL_NUM                    28
#define THERMO_THRESHOLD_NUM              28
#define THERMO_TATO_LEN                   10
#define THERMO_BIG_DATA_LEN               30
#define THERMO_INDEX                      2

#ifdef SENSOR_DATA_ACQUISITION
#define THERMO_TEST_CAL             "THERMO"
#define sensor_cal_result(x)    (((x) == SUC) ? "pass" : "fail")

#define THERMO_VERIFY_TEMP_DIFF           703027001
#define THERMO_LOW_TEMP_1                 703027002
#define THERMO_LOW_TEMP_2                 703027003
#define THERMO_LOW_TEMP_3                 703027004
#define THERMO_LOW_TEMP_4                 703027005
#define THERMO_LOW_TEMP_5                 703027006
#define THERMO_LOW_TEMP_6                 703027007
#define THERMO_LOW_TEMP_7                 703027008
#define THERMO_LOW_TEMP_8                 703027009
#define THERMO_LOW_TEMP_9                 703027010
#define THERMO_LOW_TEMP_10                703027011
#define THERMO_HIGH_TEMP_1                703027012
#define THERMO_HIGH_TEMP_2                703027013
#define THERMO_HIGH_TEMP_3                703027014
#define THERMO_HIGH_TEMP_4                703027015
#define THERMO_HIGH_TEMP_5                703027016
#define THERMO_HIGH_TEMP_6                703027017
#define THERMO_HIGH_TEMP_7                703027018
#define THERMO_HIGH_TEMP_8                703027019
#define THERMO_HIGH_TEMP_9                703027020
#define THERMO_HIGH_TEMP_10               703027021
#define THERMO_CALI_HA                    703027022
#define THERMO_CALI_HB                    703027023
#define THERMO_VERIFY_TEMP                703027024
#define THERMO_LOW_TA_MAX                 703027025
#define THERMO_LOW_TO_AVG                 703027026
#define THERMO_HIGH_TA_MAX                703027027
#define THERMO_HIGH_TO_AVG                703027028
#endif

struct thermo_threshold_cfg {
	int min_verify_diff_th;
	int max_verify_diff_th;
	int min_low_threshold;
	int max_low_threshold;
	int min_high_threshold;
	int max_high_threshold;
	int min_ha_threshold;
	int max_ha_threshold;
	int min_hb_threshold;
	int max_hb_threshold;
	int min_verify_threshold;
	int max_verify_threshold;
	int ta_max_threshold;
	int to_avg_threshold;
};

struct therm_platform_data {
	struct sensor_combo_cfg cfg;
	uint8_t blackbody_count;
	uint8_t ar_mode;
	int k1;
	int k2;
	int c1;
	int c2;
	int c3;
	int c4;
	int c5;
	struct thermo_threshold_cfg th_cfg;
};

struct therm_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t therm_dev_index;
	enum ret_type therm_calibration_res;
	int therm_upload_data_flag;
};

struct therm_platform_data *therm_get_platform_data(int32_t tag);
struct therm_device_info *therm_get_device_info(int32_t tag);
char *therm_get_sensors_id_string(void);
void therm_get_sensors_id_from_dts(struct device_node *dn);
char *therm_get_calibrate_index_string(void);
void read_thermometer_data_from_dts(struct device_node *dn);
int thermometer_data_from_mcu(const struct pkt_header *head);
void therm_detect_init(struct sensor_detect_manager *sm, uint32_t len);
void read_thermo_thre_from_dts1(struct device_node *dn);
void read_thermo_thre_from_dts2(struct device_node *dn);
void thermometer_big_data_from_mcu(const struct pkt_header *head,
	struct therm_device_info *dev_info);
void thermometer_set_big_data_th(void);
#endif
