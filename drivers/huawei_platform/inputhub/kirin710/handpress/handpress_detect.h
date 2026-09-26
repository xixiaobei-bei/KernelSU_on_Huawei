/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: handpress detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __HANDPRESS_DETECT_H__
#define __HANDPRESS_DETECT_H__

#include "sensor_sysfs.h"

struct handpress_platform_data {
	struct sensor_combo_cfg cfg;
	uint8_t bootloader_type;
	uint8_t id[CYPRESS_CHIPS];
	uint8_t i2c_address[CYPRESS_CHIPS];
	uint8_t t_pionts[CYPRESS_CHIPS];
	uint16_t poll_interval;
	uint32_t irq[CYPRESS_CHIPS];
	uint8_t handpress_extend_data[SENSOR_PLATFORM_EXTEND_DATA_SIZE];
};

void read_handpress_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
int handpress_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index);
void handpress_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
