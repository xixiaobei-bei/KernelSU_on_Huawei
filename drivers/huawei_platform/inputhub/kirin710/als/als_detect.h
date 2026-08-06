/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_DETECT_H__
#define __ALS_DETECT_H__

#include "sensor_sysfs.h"

#define ALS_DEV_COUNT_MAX 1

struct als_platform_data {
	struct sensor_combo_cfg cfg;
	GPIO_NUM_TYPE gpio_int1;
	uint8_t atime;
	uint8_t again;
	uint16_t poll_interval;
	uint16_t init_time;
	s16 threshold_value;
	s16 ga1;
	s16 ga2;
	s16 ga3;
	s16 coe_b;
	s16 coe_c;
	s16 coe_d;
	uint8_t als_phone_type;
	uint8_t als_phone_version;
	uint8_t als_gain_dynamic;
	uint8_t als_phone_tp_colour;
	uint8_t als_extend_data[SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE];
	uint8_t is_bllevel_supported;
	uint8_t als_always_open;
};

struct als_platform_data *als_get_platform_data(int32_t tag);
struct als_device_info *als_get_device_info(int32_t tag);
void read_als_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
void resend_als_parameters_to_mcu(void);
void select_als_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn);
void als_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
