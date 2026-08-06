/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: vibrator detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __VIBRATOR_DETECT_H__
#define __VIBRATOR_DETECT_H__

#include "sensor_sysfs.h"

void read_vibrator_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
void vibrator_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
