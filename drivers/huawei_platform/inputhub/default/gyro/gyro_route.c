/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "gyro_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <securec.h>

#include "contexthub_route.h"

#define MAX_TIMEOFFSET_VAL 100000000

unsigned long ungyro_timestamp_offset;

ssize_t show_gyro_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(GYRO),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}

ssize_t show_gyro_position_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gyro_platform_data *pf_data = NULL;

	pf_data = gyro_get_platform_data(TAG_GYRO);
	if (pf_data == NULL)
		return -1;
	hwlog_info("%s: gyro_position is %d\n", __func__, pf_data->position);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		pf_data->position);
}

ssize_t store_ungyro_time_offset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	long val = 0;

	if (!dev || !attr || !buf)
		return -EINVAL;

	if (strict_strtol(buf, 10, &val)) { /* change to 10 type */
		hwlog_err("%s: read uni val %lu invalid", __func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: set ungyro timestamp offset val %ld\n", __func__, val);
	if ((val < -MAX_TIMEOFFSET_VAL) || (val > MAX_TIMEOFFSET_VAL)) {
		hwlog_err("%s:set ungyro timestamp offset fail, invalid val\n",
			__func__);
		return size;
	}

	ungyro_timestamp_offset = val;
	return size;
}

ssize_t show_ungyro_time_offset(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf)
		return -EINVAL;

	hwlog_info("%s: unigyro_time_offset is %lu\n",
		__func__, ungyro_timestamp_offset);
	if (memcpy_s(buf, MAX_STR_SIZE, &ungyro_timestamp_offset,
		sizeof(ungyro_timestamp_offset)) != EOK)
		return -1;
	return sizeof(ungyro_timestamp_offset);
}

ssize_t show_gyro_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"gyro offset:%d %d %d\nsensitivity:%d %d %d\nxis_angle:%d %d %d %d %d %d %d %d %d\nuser calibrated offset:%d %d %d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1],
		gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5],
		gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9],
		gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13],
		gyro_sensor_offset[14], gyro_sensor_offset[15],
		gyro_sensor_offset[16], gyro_sensor_offset[17]);
}

ssize_t show_gyro1_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"gyro1 offset:%d %d %d\n online calibrated offset:%d %d %d\n",
		gyro1_sensor_offset[0], gyro1_sensor_offset[1],
		gyro1_sensor_offset[2], gyro1_sensor_offset[15],
		gyro1_sensor_offset[16], gyro1_sensor_offset[17]);
}

