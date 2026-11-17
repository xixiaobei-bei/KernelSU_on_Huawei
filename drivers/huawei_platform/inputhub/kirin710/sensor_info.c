/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: sensor info source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "sensor_info.h"

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/hwspinlock.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/types.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>

#include "contexthub_route.h"
#include "motion_channel.h"
#include "sensor_sysfs.h"

static int is_gsensor_gather_enable;
static int stop_auto_motion;
static int flag_for_sensor_test; /* fordden sensor cmd from HAL */
static int stop_auto_accel;

int get_stop_auto_motion(void)
{
	return stop_auto_motion;
}

int get_flag_for_sensor_test(void)
{
	return flag_for_sensor_test;
}

int get_stop_auto_accel(void)
{
	return stop_auto_accel;
}

ssize_t attr_set_gsensor_gather_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	unsigned long enable = 0;
	unsigned int delay = 50;
	interval_param_t delay_param = {
		.period = delay,
		.batch_count = 1,
		.mode = AUTO_MODE,
		.reserved[0] = TYPE_STANDARD /* for step counter only */
	};

	if (strict_strtoul(buf, TO_DECIMALISM, &enable))
		return -EINVAL;

	if ((enable != 0) && (enable != 1))
		return -EINVAL;

	if (is_gsensor_gather_enable == enable) {
		hwlog_info("gsensor gather already seted to state, is_gsensor_gather_enable %d\n",
			is_gsensor_gather_enable);
		return size;
	}
	ret = inputhub_sensor_enable(TAG_GPS_4774_I2C, enable);
	if (ret) {
		hwlog_err("send GSENSOR GATHER enable cmd to mcu fail,ret=%d\n", ret);
		return -EINVAL;
	}

	if (enable == 1) {
		ret = inputhub_sensor_setdelay(TAG_GPS_4774_I2C, &delay_param);
		if (ret) {
			hwlog_err("send GSENSOR GATHER set delay cmd to mcu fail,ret=%d\n", ret);
			return -EINVAL;
		}
	}

	is_gsensor_gather_enable = enable;
	hwlog_info("GSENSOR GATHER set to state(%lu) success\n", enable);

	return size;
}

ssize_t attr_set_sensor_test_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;

	hwlog_info("%s +\n", __func__);
	if (strict_strtoul(buf, TO_DECIMALISM, &val)) {
		hwlog_err("In %s! val = %lu\n", __func__, val);
		return -EINVAL;
	}
	if (val == 1)
		flag_for_sensor_test = 1;
	else
		flag_for_sensor_test = 0;
	return size;
}

/*
 * buf: motion value, 2byte,
 * motion type, 0-11
 * second status, 0-4
 */
#define MOTION_DT_STUP_LENGTH 5
#define RADIX_16 16
ssize_t attr_set_dt_motion_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	char dt_motion_value[MOTION_DT_STUP_LENGTH] = { };
	int i = 0;
	unsigned long source;

	source = simple_strtoul(buf, NULL, RADIX_16);
	hwlog_err("%s buf %s, source %lu, size %lu\n", __func__, buf, source, size);

	for (; i < MOTION_DT_STUP_LENGTH - 1; i++) {
		dt_motion_value[i] = source % ((i + 1) * RADIX_16);
		source = source / RADIX_16;
	}

	dt_motion_value[MOTION_DT_STUP_LENGTH - 1] = '\0';
	hwlog_err("%s motion %x %x %x %x\n", __func__, dt_motion_value[0],
		dt_motion_value[1], dt_motion_value[2], dt_motion_value[3]);
	inputhub_route_write(ROUTE_MOTION_PORT, dt_motion_value,
		MOTION_DT_STUP_LENGTH - 1);

	return size;
}

ssize_t attr_set_stop_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	stop_auto_accel = simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	hwlog_err("%s stop_auto_accel %d\n", __func__, stop_auto_accel);
	return size;
}

static int tell_cts_test_to_mcu(int status)
{
	struct read_info pkg_mcu;
	struct write_info winfo;
	pkt_sys_statuschange_req_t pkt;

	if (status == 1) {
		inputhub_sensor_enable(TAG_AR, false);
		hwlog_info("close ar in %s\n", __func__);
	}
	if ((status == 0) || (status == 1)) {
		winfo.tag = TAG_SYS;
		winfo.cmd = CMD_SYS_CTS_RESTRICT_MODE_REQ;
		winfo.wr_len = sizeof(pkt) - sizeof(pkt.hd);
		pkt.status = status;
		winfo.wr_buf = &pkt.status;
		return write_customize_cmd(&winfo, &pkg_mcu, true);
	} else {
		hwlog_err("error status %d in %s\n", status, __func__);
		return -EINVAL;
	}
	return 0;
}

ssize_t attr_stop_auto_motion_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		stop_auto_motion);
}

ssize_t attr_set_stop_auto_motion(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val;

	val = simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	if (val == 1) { /* cts test,disable motion */
		disable_motions_when_sysreboot();
		stop_auto_motion = 1;
		hwlog_err("%s stop_auto_motion =%d, val = %lu\n",
			__func__, stop_auto_motion, val);
		tell_cts_test_to_mcu(1);
	}
	if (val == 0) {
		stop_auto_motion = 0;
		enable_motions_when_recovery_iom3();
		hwlog_err("%s stop_auto_motion =%d, val = %lu\n",
			__func__, stop_auto_motion, val);
		tell_cts_test_to_mcu(0);
	}

	return size;
}

ssize_t attr_set_sensor_stepcounter_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long source;
	struct sensor_data event;

	source = simple_strtoul(buf, NULL, TO_DECIMALISM);
	event.type = TAG_STEP_COUNTER;
	event.length = 12; /* 12: int x 3 */
	event.value[0] = source;
	event.value[1] = 0;
	event.value[2] = 0;

	report_sensor_event(TAG_STEP_COUNTER, event.value, event.length);
	return size;
}

