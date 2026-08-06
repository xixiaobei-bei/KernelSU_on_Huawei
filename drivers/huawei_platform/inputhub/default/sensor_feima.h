/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: sensor feima header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __SENSOR_FEIMA_H__
#define __SENSOR_FEIMA_H__

#include "sensor_config.h"

struct sensor_cookie {
	int tag;
	const char *name;
	const struct attribute_group *attrs_group;
	struct device *dev;
};

typedef struct {
	uint16_t sub_cmd;
	uint16_t sar_info;
} rpc_ioctl_t;

enum {
	CALL_START = 0xa2,
	CALL_STOP,
};

void send_lcd_freq_to_sensorhub(uint32_t lcd_freq);
void save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level);
int posture_sensor_enable(void);
void report_fold_status_when_poweroff_charging(int status);

#endif /* __SENSOR_FEIMA_H__ */
