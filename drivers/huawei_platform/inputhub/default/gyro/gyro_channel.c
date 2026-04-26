/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "gyro_channel.h"

#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"
#include "gyro_sysfs.h"

int send_gyro1_calibrate_data_to_mcu(void)
{
	struct gyro_device_info *dev_info = NULL;

	dev_info = gyro_get_device_info(TAG_GYRO1);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(GYRO1_OFFSET_NV_NUM,
		GYRO1_OFFSET_NV_SIZE, GYRO1_NV_NAME))
		return -1;

	dev_info->gyro_first_start_flag = 1;
	/* copy to gyro1 offset by pass */
	if (memcpy_s(gyro1_sensor_offset, sizeof(gyro1_sensor_offset),
		user_info.nv_data, sizeof(gyro1_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro1 offset: %d %d %d online offset:%d %d %d\n",
		gyro1_sensor_offset[0], gyro1_sensor_offset[1],
		gyro1_sensor_offset[2], gyro1_sensor_offset[15],
		gyro1_sensor_offset[16], gyro1_sensor_offset[17]);

	if (memcpy_s(&gyro1_sensor_calibrate_data,
		sizeof(gyro1_sensor_calibrate_data),
		gyro1_sensor_offset,
		(sizeof(gyro1_sensor_calibrate_data) < GYRO1_OFFSET_NV_SIZE) ?
		sizeof(gyro1_sensor_calibrate_data) : GYRO1_OFFSET_NV_SIZE) !=
		EOK)
		return -1;
	if (send_calibrate_data_to_mcu(TAG_GYRO1, SUB_CMD_SET_OFFSET_REQ,
		gyro1_sensor_offset, GYRO1_OFFSET_NV_SIZE, false))
		return -1;

	return 0;
}


int send_gyro_calibrate_data_to_mcu(void)
{
	struct gyro_device_info *dev_info = NULL;

	dev_info = gyro_get_device_info(TAG_GYRO);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(GYRO_CALIDATA_NV_NUM,
		GYRO_CALIDATA_NV_SIZE, "GYRO"))
		return -1;

	dev_info->gyro_first_start_flag = 1;
	/* copy to gsensor_offset by pass */
	if (memcpy_s(gyro_sensor_offset, sizeof(gyro_sensor_offset),
		user_info.nv_data, sizeof(gyro_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro_sensor offset: %d %d %d sensitity:%d %d %d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1],
		gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5]);
	hwlog_info("nve_direct_access read gyro_sensor xis_angle: %d %d %d %d %d %d %d %d %d\n",
		gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9],
		gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13],
		gyro_sensor_offset[14]);
	hwlog_info("nve_direct_access read gyro_sensor online offset: %d %d %d\n",
		gyro_sensor_offset[15], gyro_sensor_offset[16],
		gyro_sensor_offset[17]);

	if (memcpy_s(&gyro_sensor_calibrate_data,
		sizeof(gyro_sensor_calibrate_data),
		gyro_sensor_offset,
		(sizeof(gyro_sensor_calibrate_data) < GYRO_CALIDATA_NV_SIZE) ?
		sizeof(gyro_sensor_calibrate_data) : GYRO_CALIDATA_NV_SIZE) !=
		EOK)
		return -1;
	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
		gyro_sensor_offset, GYRO_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

int send_gyro_temperature_offset_to_mcu(void)
{
	if (read_calibrate_data_from_nv(GYRO_TEMP_CALI_NV_NUM,
		GYRO_TEMP_CALI_NV_SIZE, "GYTMP"))
		return -1;

	/* copy to gsensor_offset by pass */
	if (memcpy_s(gyro_temperature_offset, sizeof(gyro_temperature_offset),
		user_info.nv_data, sizeof(gyro_temperature_offset)) != EOK)
		return -1;

	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_GYRO_TMP_OFFSET_REQ,
		gyro_temperature_offset, sizeof(gyro_temperature_offset), false))
		return -1;

	if (memcpy_s(&gyro_temperature_calibrate_data,
		sizeof(gyro_temperature_calibrate_data),
		gyro_temperature_offset,
		(sizeof(gyro_temperature_calibrate_data) <
		sizeof(gyro_temperature_offset)) ?
		sizeof(gyro_temperature_calibrate_data) :
		sizeof(gyro_temperature_offset)) != EOK)
		return -1;
	return 0;
}

int write_gyro1_sensor_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro1_sensor_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(GYRO1_OFFSET_NV_NUM, length,
		GYRO1_NV_NAME, temp))
		return -1;

	if (memcpy_s(gyro1_sensor_calibrate_data,
		sizeof(gyro1_sensor_calibrate_data), temp,
		(sizeof(gyro1_sensor_calibrate_data) < length) ?
		sizeof(gyro1_sensor_calibrate_data) : length) != EOK)
		return -1;
	if (memcpy_s(gyro1_sensor_offset, sizeof(gyro1_sensor_offset),
		gyro1_sensor_calibrate_data,
		(sizeof(gyro1_sensor_offset) < GYRO1_OFFSET_NV_SIZE) ?
		sizeof(gyro1_sensor_offset) : GYRO1_OFFSET_NV_SIZE) != EOK)
		return -1;
	hwlog_info("gyro calibrate data %d %d %d %d %d %d lens=%d\n",
		gyro1_sensor_offset[0], gyro1_sensor_offset[1],
		gyro1_sensor_offset[2], gyro1_sensor_offset[15],
		gyro1_sensor_offset[16], gyro1_sensor_offset[17], length);
	return ret;
}

int write_gyro_sensor_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro_sensor_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (write_calibrate_data_to_nv(GYRO_CALIDATA_NV_NUM, length, "GYRO", temp))
		return -1;

	if (memcpy_s(gyro_sensor_calibrate_data,
		sizeof(gyro_sensor_calibrate_data), temp,
		(sizeof(gyro_sensor_calibrate_data) < length) ?
		sizeof(gyro_sensor_calibrate_data) : length) != EOK)
		return -1;
	if (memcpy_s(gyro_sensor_offset, sizeof(gyro_sensor_offset),
		gyro_sensor_calibrate_data,
		(sizeof(gyro_sensor_offset) < GYRO_CALIDATA_NV_SIZE) ?
		sizeof(gyro_sensor_offset) : GYRO_CALIDATA_NV_SIZE) != EOK)
		return -1;
	hwlog_info("gyro calibrate data %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d,online offset %d %d %d lens=%d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1],
		gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5],
		gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9],
		gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13],
		gyro_sensor_offset[14], gyro_sensor_offset[15],
		gyro_sensor_offset[16], gyro_sensor_offset[17], length);
	return ret;
}

int write_gyro_temperature_offset_to_nv(const char *temp, int length)
{
	int ret = 0;

	if (!temp) {
		hwlog_err("write_gyro_temp_offset_to_nv fail, invalid para\n");
		return -1;
	}
	if (write_calibrate_data_to_nv(GYRO_TEMP_CALI_NV_NUM, length,
		"GYTMP", temp))
		return -1;

	if (memcpy_s(gyro_temperature_calibrate_data,
		sizeof(gyro_temperature_calibrate_data), temp,
		(sizeof(gyro_temperature_calibrate_data) < length) ?
		sizeof(gyro_temperature_calibrate_data) : length) != EOK)
		return -1;
	hwlog_info("write_gyro_temp_offset_to_nv suc len=%d\n", length);
	return ret;
}

void reset_gyro_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(GYRO)) == 0)
		return;
	send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
		gyro_sensor_calibrate_data, GYRO_CALIDATA_NV_SIZE, true);
	send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_GYRO_TMP_OFFSET_REQ,
		gyro_temperature_calibrate_data, GYRO_TEMP_CALI_NV_SIZE,
		true);
}

void reset_gyro1_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(GYRO1)) == 0)
		return;
	send_calibrate_data_to_mcu(TAG_GYRO1, SUB_CMD_SET_OFFSET_REQ,
		gyro1_sensor_calibrate_data, GYRO1_OFFSET_NV_SIZE, true);
}

