/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MAG_DETECT_H__
#define __MAG_DETECT_H__

#include "sensor_sysfs.h"

#define MAG_DEV_COUNT_MAX 1

#define PDC_SIZE 27
#define CHARGER_TYPE_NUM 4
// EVE: 0:10V 1:9V
struct mag_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t mag_dev_index;
	uint8_t mag_first_start_flag;
	uint8_t mag_folder_function;
	int32_t mag_threshold_for_als_calibrate;
	int32_t akm_cal_algo;
	int32_t akm_need_charger_current;
	int32_t akm_current_x_fac[CHARGER_TYPE_NUM];
	int32_t akm_current_y_fac[CHARGER_TYPE_NUM];
	int32_t akm_current_z_fac[CHARGER_TYPE_NUM];
};


struct mag_device_info *mag_get_device_info(int32_t tag);
void read_magn_charger_current_data_from_dts(struct device_node *dn);
void mag_detect_init(struct sensor_detect_manager *sm, uint32_t len);
#endif
