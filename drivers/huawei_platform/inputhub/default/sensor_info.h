/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: sensor info header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __SENSOR_INFO_H__
#define __SENSOR_INFO_H__

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "sensor_sysfs.h"

int get_stop_auto_motion(void);
int get_flag_for_sensor_test(void);
int get_stop_auto_accel(void);
#ifdef CONFIG_HUAWEI_DSM
ssize_t attr_set_selftest(enum obj_tag tag, char *selftest_result,
	const char *buf, size_t size);
#else
ssize_t attr_set_selftest(enum obj_tag tag, unsigned int cmd,
	char *selftest_result, const char *buf, size_t size);
#endif
ssize_t attr_set_enable(enum obj_tag tag,
	enum obj_cmd cmd1, enum obj_cmd cmd2, const char *buf, size_t size);
ssize_t attr_set_delay(enum obj_tag tag, const char *buf, size_t size);
ssize_t attr_set_gsensor_gather_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_pdr_delay(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_pdr_delay(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_sensor_test_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_dt_motion_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_stop_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_stop_auto_motion_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_set_stop_auto_motion(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_sensor_motion_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_sensor_stepcounter_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t attr_set_dt_hall_sensor_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t show_sensor_in_board_status_sysfs(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_sensor_read_temperature(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_dump_sensor_status(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_set_data_type(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t show_get_sensors_id(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t sensors_calibrate_show(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t sensors_calibrate_store(int tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t show_hifi_supported(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_sensor_detect(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t store_sensor_detect(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);

#endif
