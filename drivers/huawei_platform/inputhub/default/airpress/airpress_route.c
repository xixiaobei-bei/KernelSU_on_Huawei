/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: airpress route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "airpress_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <securec.h>

#include "airpress_sysfs.h"
#include "contexthub_route.h"

#define AIRPRESS_CALIBRATE_DIVISOR 100
#define AIRPRESS_PERIOD_MAX 20

enum {
	AIRPRESS_CALIBRATE_ENTER = 3,
	AIRPRESS_CALIBRATE_WRITE = 9,
	AIRPRESS_CALIBRATE_EXIT = 10,
};

/* airpress calibrate result */
static enum ret_type airpress_calibration_res = RET_INIT;
static int airpress_close_after_touch_calibrate = true;

struct airpress_touch_calibrate_data pressure_touch_calibrate_data;

static int airpress_calibrate_process(unsigned long val,
	struct read_info *read_info);

ssize_t show_airpress_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(AIRPRESS),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}

ssize_t attr_airpress_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret;
	struct read_info read_pkg;
	enum ret_type calibration_result = RET_INIT;

	if (!buf) {
		hwlog_err("attr_airpress_calibrate_write: invalid input\n");
		return -EINVAL;
	}
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	/* send calibrate command */
	read_pkg = send_airpress_calibrate_cmd(TAG_PRESSURE, val,
			&calibration_result);
	if (calibration_result != SUC) {
		hwlog_err("attr_airpress_calibrate_write: send_airpress_calibrate_cmd fail\n");
		return -1;
	}
	/* calibration process */
	ret = airpress_calibrate_process(val, &read_pkg);
	return ret;
}

static int airpress_calibrate_on_write(struct read_info *read_info)
{
	struct airpress_platform_data *pf_data = NULL;
	char temp[AIRPRESS_CALIDATA_NV_SIZE_WITH_AIRTOUCH] = { 0 };

	pf_data = airpress_get_platform_data(TAG_PRESSURE);
	if (pf_data == NULL)
		return -1;
	if (memcpy_s(&pressure_touch_calibrate_data,
		sizeof(pressure_touch_calibrate_data), read_info->data,
		sizeof(pressure_touch_calibrate_data)) != EOK)
		return -1;
	if (memcpy_s(temp, sizeof(temp), &pf_data->offset,
		sizeof(pf_data->offset)) != EOK)
		return -1;
	if (memcpy_s(temp + sizeof(pf_data->offset),
		sizeof(temp) - sizeof(pf_data->offset),
		&pressure_touch_calibrate_data,
		sizeof(pressure_touch_calibrate_data)) != EOK)
		return -1;
	if (write_calibrate_data_to_nv(AIRPRESS_CALIDATA_NV_NUM,
		sizeof(pf_data->offset) +
		sizeof(pressure_touch_calibrate_data), "AIRDATA", temp)) {
		hwlog_err("attr_airpress_calibrate_write: write_calibrate_data_to_nv fail\n");
		return -1;
	}
	hwlog_info("temp[0~7] = %x %x %x %x %x %x %x %x\n",
		temp[0], temp[1], temp[2], temp[3],
		temp[4], temp[5], temp[6], temp[7]);
	hwlog_info("attr_airpress_calibrate_write: write_calibrate_data_to_nv success, cResult: %d %d %d %d %d %d %d %d, tResult: %d %d %d %d %d %d %d %d\n",
		pressure_touch_calibrate_data.c_result.slope,
		pressure_touch_calibrate_data.c_result.base_press,
		pressure_touch_calibrate_data.c_result.max_press,
		pressure_touch_calibrate_data.c_result.raise_press,
		pressure_touch_calibrate_data.c_result.min_press,
		pressure_touch_calibrate_data.c_result.temp,
		pressure_touch_calibrate_data.c_result.speed,
		pressure_touch_calibrate_data.c_result.result_flag,
		pressure_touch_calibrate_data.t_result.slope,
		pressure_touch_calibrate_data.t_result.base_press,
		pressure_touch_calibrate_data.t_result.max_press,
		pressure_touch_calibrate_data.t_result.raise_press,
		pressure_touch_calibrate_data.t_result.min_press,
		pressure_touch_calibrate_data.t_result.temp,
		pressure_touch_calibrate_data.t_result.speed,
		pressure_touch_calibrate_data.t_result.result_flag);
#ifdef SENSOR_DATA_ACQUISITION
	airpress_touch_data_acquisition(&pressure_touch_calibrate_data);
#endif
	return 1;
}

static int airpress_calibrate_on_enter(void)
{
	int ret;
	interval_param_t param = { 0 };

	if (sensor_status.opened[TAG_PRESSURE] == 0) { /* if airpress is not opened, open first */
		hwlog_info("send airpress open cmd(during calibrate) to mcu\n");
		airpress_close_after_touch_calibrate = true;
		ret = inputhub_sensor_enable(TAG_PRESSURE, true);
		if (ret) {
			hwlog_err("send airpress open cmd(during calibrate) to mcu fail,ret=%d\n", ret);
			return -1;
		}
	} else {
		airpress_close_after_touch_calibrate = false;
	}
	/* period must equals 20 ms */
	if ((sensor_status.delay[TAG_PRESSURE] == 0) ||
		(sensor_status.delay[TAG_PRESSURE] > AIRPRESS_PERIOD_MAX)) {
		hwlog_info("send airpress setdelay cmd(during calibrate) to mcu\n");
		param.period = AIRPRESS_PERIOD_MAX;
		ret = inputhub_sensor_setdelay(TAG_PRESSURE, &param);
		if (ret) {
			hwlog_err("send airpress set delay cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	}
	return 1;
}

static int airpress_calibrate_on_exit(void)
{
	int ret;

	if (airpress_close_after_touch_calibrate == true) {
		airpress_close_after_touch_calibrate = false;
		hwlog_info("send airpress close cmd(during calibrate) to mcu\n");
		ret = inputhub_sensor_enable(TAG_PRESSURE, false);
		if (ret) {
			hwlog_err("send airpress close cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	}
	return 1;
}

static int airpress_calibrate_process(unsigned long val,
	struct read_info *read_info)
{
	uint32_t cmd;
	int ret = -1;

	cmd = (val % AIRPRESS_CALIBRATE_DIVISOR);
	switch (cmd) {
	case AIRPRESS_CALIBRATE_WRITE:
		ret = airpress_calibrate_on_write(read_info);
		break;
	case AIRPRESS_CALIBRATE_ENTER:
		ret = airpress_calibrate_on_enter();
		break;
	case AIRPRESS_CALIBRATE_EXIT:
		ret = airpress_calibrate_on_exit();
		break;
	default:
		if (memcpy_s(&ret, sizeof(ret),
			read_info->data, sizeof(ret)) != EOK)
			return -1;
		break;
	}

	hwlog_info("attr_airpress_calibrate_write: ret = %d.\n", ret);
	return ret;
}

ssize_t attr_airpress_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int val = airpress_calibration_res;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", val);
}

ssize_t attr_airpress_set_tp_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int32_t err_no = 0;

	if (buf == NULL) {
		hwlog_err("%s buf is NULL\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s input buf_size is %lu\n", __func__, count);
	ret = send_subcmd_data_to_mcu_lock(TAG_PRESSURE,
		SUB_CMD_SET_TP_COORDINATE, (const void *)buf, count, &err_no);
	if (ret) {
		hwlog_err("send tag %d tp touch info to mcu fail,ret=%d\n",
			(int32_t)TAG_PRESSURE, ret);
		ret = -1;
	} else {
		if (err_no != 0) {
			hwlog_err("send tp touch info to mcu fail\n");
			ret = -1;
		} else {
			hwlog_info("send tp touch info to mcu succes\n");
			ret = count;
		}
	}
	return (ssize_t)ret;
}

