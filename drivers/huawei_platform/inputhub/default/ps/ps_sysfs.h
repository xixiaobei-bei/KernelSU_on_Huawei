/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ps sysfs header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __PS_SYSFS_H__
#define __PS_SYSFS_H__

#include "ps_channel.h"
#include "ps_detect.h"
#include "ps_route.h"

#define PS_BIG_DATA_OFFSET         6
#define PS_PULSE_LEN_64            64

enum ps_nv_context {
	PS_XTALK0 = 0,
	PS_5CM_PDATA0,
	PS_3CM_PDATA0,
	PS_OFFSET_NEW0,
	PS_OFFSET_OLD0,
	PS_XTALK_OLD0,
	PS_XTALK1,
	PS_5CM_PDATA1,
	PS_3CM_PDATA1,
	PS_OFFSET_NEW1,
	PS_OFFSET_OLD1,
	PS_XTALK_OLD1,
	PS_PULSE_LEN
};

int get_stop_auto_ps(void);
ssize_t attr_ps_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
/* temp: buffer to write, length: bytes to write */
int write_ps_offset_to_nv(int *temp, uint16_t length);
ssize_t attr_ps_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_set_stop_ps_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_dt_ps_sensor_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t sensors_calibrate_show_ps(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
