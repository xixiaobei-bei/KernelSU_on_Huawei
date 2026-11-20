/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __CAP_PROX_DETECT_H__
#define __CAP_PROX_DETECT_H__

#include "sensor_sysfs.h"

#define CALIBRATE_THRED_0        0
#define CALIBRATE_THRED_1        1
#define CALIBRATE_THRED_2        2
#define CALIBRATE_THRED_3        3

char *cap_prox_get_calibrate_order_string(void);
void read_capprox_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
int cap_prox_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index);
void cap_prox_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
