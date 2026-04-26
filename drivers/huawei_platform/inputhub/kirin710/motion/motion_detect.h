/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: motion detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MOTION_DETECT_H__
#define __MOTION_DETECT_H__

#include "sensor_sysfs.h"

struct motion_platform_data {
	uint8_t motion_horizontal_pickup_flag;
	uint8_t angle_gap;
};

void read_motion_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
int motion_set_cfg_data(void);
void motion_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
