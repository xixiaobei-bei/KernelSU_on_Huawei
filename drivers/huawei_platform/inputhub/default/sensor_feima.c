/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: sensor feima source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#include "sensor_feima.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/semaphore.h>
#include <linux/time64.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <securec.h>

#include "acc_sysfs.h"
#include "airpress_sysfs.h"
#include "als_sysfs.h"
#include "cap_prox_sysfs.h"
#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_route.h"
#include "gyro_sysfs.h"
#include "mag_sysfs.h"
#include "ps_sysfs.h"
#include "sensor_config.h"
#include "sensor_detect.h"
#include "sensor_info.h"
#include "sensor_sysfs.h"
#include "therm_sysfs.h"

#define ALS_MCU_HAL_CONVER 10
#define ACC_CONVERT_COFF 1000

struct class *sensors_class;
int sleeve_test_enabled;

static bool rpc_motion_request;
static time_t get_data_last_time;
static unsigned long sar_service_info;
extern volatile int vibrator_shake;
extern volatile int hall_value;

static bool camera_set_rpc_flag; /* default value: false */
static uint8_t g_stepcountdocm;

static int rpc_commu(unsigned int cmd, unsigned int pare, uint16_t motion)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	rpc_ioctl_t pkg_ioctl;

	pkg_ap.tag = TAG_RPC;
	pkg_ap.cmd = cmd;
	pkg_ioctl.sub_cmd = pare;
	pkg_ioctl.sar_info = motion;
	pkg_ap.wr_buf = &pkg_ioctl;
	pkg_ap.wr_len = sizeof(pkg_ioctl);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send rpc cmd %d to mcu fail,ret=%d\n", cmd, ret);
		return ret;
	}
	return ret;
}

static int rpc_motion(uint16_t motion)
{
	unsigned int cmd;
	unsigned int sub_cmd;
	unsigned int ret;

	if (rpc_motion_request) {
		cmd = CMD_CMN_CONFIG_REQ;
		sub_cmd = SUB_CMD_RPC_START_REQ;
		ret = rpc_commu(cmd, sub_cmd, motion);
		if (ret) {
			hwlog_err("%s: rpc motion enable fail\n", __func__);
			return ret;
		}
		hwlog_info("%s: rpc motion start succsess\n", __func__);
	} else {
		cmd = CMD_CMN_CONFIG_REQ;
		sub_cmd = SUB_CMD_RPC_STOP_REQ;
		ret = rpc_commu(cmd, sub_cmd, motion);
		if (ret) {
			hwlog_info("%s: rpc motion close fail\n", __func__);
			return ret;
		}
		hwlog_info("%s: rpc motion stop succsess\n", __func__);
	}

	return ret;
}

#define CHECK_SENSOR_COOKIE(data) \
do {\
	if ((!(data)) || (!((data)->tag >= TAG_SENSOR_BEGIN && \
		((data)->tag) < TAG_SENSOR_END)) || (!((data)->name))) {\
		hwlog_err("error in %s\n", __func__);\
		return -EINVAL;\
	} \
} while (0)

static ssize_t show_enable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);
	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n",
		sensor_status.status[data->tag]);
}

static ssize_t store_enable(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	const char *operation = NULL;
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);

	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (ap_sensor_enable(data->tag, (val == 1)))
		return size;

	operation = ((val == 1) ? "enable" : "disable");
	pkg_ap.tag = data->tag;
	pkg_ap.cmd = (val == 1) ? CMD_CMN_OPEN_REQ : CMD_CMN_CLOSE_REQ;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret != 0) {
		hwlog_err("%s %s failed, ret = %d in %s\n",
			operation, data->name, ret, __func__);
		return size;
	}

	if (pkg_mcu.errno != 0)
		hwlog_err("%s %s failed errno = %d in %s\n",
			operation, data->name, pkg_mcu.errno, __func__);
	else
		hwlog_info("%s %s success\n", operation, data->name);

	return size;
}

static int rpc_status_change(void)
{
	int ret;

	sar_service_info = (sar_service_info & ~BIT(9)) | /* clean bit 9 */
		((unsigned long)rpc_motion_request << 9); /* set bit 9 */
	hwlog_info("sar_service_info is %lu\n", sar_service_info);
	ret = rpc_motion(sar_service_info);
	if (ret) {
		hwlog_err("rpc motion fail: %d\n", ret);
		return ret;
	}
	return ret;
}

/*
 * Description: add for Txx front&wide camera radio frequency interference
 *              status:1--enable camera
 *              status:0--disable camera
 */
int rpc_status_change_for_camera(unsigned int status)
{
	int ret = 0;

	if (status == 1) {
		sar_service_info = sar_service_info | 0x400; /* set bit10 */
		camera_set_rpc_flag = true;
	} else if (status == 0) {
		sar_service_info = sar_service_info & 0xFBFF; /* release bit10 */
		camera_set_rpc_flag = false;
	} else {
		camera_set_rpc_flag = false;
		hwlog_err("error status\n");
		return ret;
	}

	hwlog_info("status %d, sar_service_info is %lu\n",
		status, sar_service_info);
	ret = rpc_status_change();
	if (ret) {
		hwlog_err("rpc status change fail: %d\n", ret);
		return ret;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(rpc_status_change_for_camera);

static ssize_t store_rpc_motion_req(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long value = 0;

	if (strict_strtoul(buf, TO_DECIMALISM, &value))
		hwlog_err("%s: rpc motion request val %lu invalid",
			__func__, value);

	hwlog_info("%s: rpc motion request val %lu\n", __func__, value);

	if (value == CALL_START || value == CALL_STOP)
		send_acc_filter_flag(value);

	if ((value != 0) && (value != 1)) {
		hwlog_err("%s: set enable fail, invalid val\n", __func__);
		return size;
	}
	rpc_motion_request = value;
	ps_telecall_status_change(value);
	rpc_status_change();
	return size;
}

static ssize_t store_rpc_sar_service_req(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long sar_service = 0;

	if (strict_strtoul(buf, TO_DECIMALISM, &sar_service))
		hwlog_err("rpc_sar_service_req strout error\n");
	if (sar_service > 0xFFFF) {
		hwlog_err("%s: set enable fail, invalid val\n", __func__);
		return size;
	}
	hwlog_info("%s: rpc sar service request val (%lu), buf is %s\n",
		__func__, sar_service, buf);
	if (camera_set_rpc_flag) {
		sar_service = sar_service | 0x400; /* camera set bit10 */
		hwlog_info("%s: camera_set_rpc_flag, rpc sar service val (%lu)\n",
			__func__, sar_service);
	}
	sar_service_info = sar_service;
	rpc_status_change();
	return size;
}

static ssize_t show_rpc_motion_req(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", rpc_motion_request);
}

static ssize_t show_set_delay(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.delay[data->tag]);
}

static ssize_t store_set_delay(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	pkt_cmn_interval_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	memset(&cpkt, 0, sizeof(cpkt));
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (ap_sensor_setdelay(data->tag, val))
		return size;

	if (val >= 1 && val <= 1000) { /* range [1, 1000] */
		pkg_ap.tag = data->tag;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		cpkt.param.period = val;
		pkg_ap.wr_buf = &hd[1];
		pkg_ap.wr_len = sizeof(cpkt.param);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
		if (ret != 0) {
			hwlog_err("set %s delay cmd to mcu fail, ret = %d in %s\n",
				data->name, ret, __func__);
			return ret;
		}
		if (pkg_mcu.errno != 0) {
			hwlog_err("set %s delay failed errno %d in %s\n",
				data->name, pkg_mcu.errno, __func__);
			return -EINVAL;
		} else {
			hwlog_info("set %s delay (%ld)success\n", data->name, val);
		}
	} else {
		hwlog_err("set %s delay_ms %d ms range error in %s\n",
			data->name, (int)val, __func__);
		return -EINVAL;
	}

	return size;
}

static const char *get_sensor_info_by_tag(int tag)
{
	enum sensor_detect_list sname;

	sname = get_id_by_sensor_tag(tag);
	return (sname != SENSOR_MAX) ? get_sensor_chip_info_address(sname) : "";
}

static ssize_t show_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%s\n", get_sensor_info_by_tag(data->tag));
}

static ssize_t show_get_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	uint8_t *tag_type = get_tag_to_hal_sensor_type();
	unsigned int hal_sensor_tag = tag_type[data->tag];
	struct t_sensor_get_data *get_data = &sensor_status.get_data[hal_sensor_tag];
	unsigned int offset = 0;
	int i = 0;
	int mem_num;

	CHECK_SENSOR_COOKIE(data);

	atomic_set(&get_data->reading, 1);
	reinit_completion(&get_data->complete);
	/* return -ERESTARTSYS if interrupted, 0 if completed */
	if (wait_for_completion_interruptible(&get_data->complete) == 0) {
		mem_num = get_data->data.length / sizeof(get_data->data.value[0]);
		for (; i < mem_num; i++) {
			if (((data->tag == TAG_ALS) || (data->tag == TAG_ALS1) ||
				(data->tag == TAG_ALS2)) && (i == 0))
				get_data->data.value[0] =
					get_data->data.value[0] / ALS_MCU_HAL_CONVER;

			if (data->tag == TAG_ACCEL || data->tag == TAG_ACC1)
				/* need be devicdd by 1000.0 for high resolu */
				get_data->data.value[i] =
					get_data->data.value[i] / ACC_CONVERT_COFF;

			offset += sprintf(buf + offset, "%d\t",
				get_data->data.value[i]);
		}
		offset += sprintf(buf + offset, "\n");
	}
	return offset;
}

static ssize_t store_get_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	struct timeval tv;
	struct sensor_data event;
	int arg;
	int argc = 0;
	time_t get_data_current_time;

	CHECK_SENSOR_COOKIE(data);

	memset(&tv, 0, sizeof(struct timeval));
	do_gettimeofday(&tv);
	get_data_current_time = tv.tv_sec;
	if ((get_data_current_time - get_data_last_time) < 1) {
		hwlog_info("%s:time interval is less than 1s(from %u to %u), skip\n",
			__func__, (uint32_t)get_data_last_time,
			(uint32_t)get_data_current_time);
		return size;
	}
	get_data_last_time = get_data_current_time;

	/* parse cmd buffer */
	for (; (buf = get_str_begin(buf)) != NULL; buf = get_str_end(buf)) {
		if (get_arg(buf, &arg)) {
			if (argc < (sizeof(event.value) / sizeof(event.value[0])))
				event.value[argc++] = arg;
			else
				hwlog_err("too many args, %d will be ignored\n",
					arg);
		}
	}
	/* fill sensor event and write to sensor event buffer */
	report_sensor_event(data->tag, event.value,
		sizeof(event.value[0]) * argc);

	return size;
}

static ssize_t show_selftest(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%s\n",
		sensor_status.selftest_result[data->tag]);
}

static ssize_t store_selftest(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	uint32_t errno;
	unsigned long val = 0;
	pkt_subcmd_req_t cpkt;
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
#ifdef CONFIG_INPUTHUB_30
	struct write_info wr;
	struct read_info rd;
#endif

	CHECK_SENSOR_COOKIE(data);
	/* SUC-->"0", OTHERS-->"1", size is 2 */
	memcpy(sensor_status.selftest_result[data->tag], "1", 2);
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	if (val == 1) {
		cpkt.hd.tag = data->tag;
		cpkt.hd.cmd = CMD_CMN_CONFIG_REQ;
		cpkt.subcmd = SUB_CMD_SELFTEST_REQ;
		cpkt.hd.resp = RESP;
		cpkt.hd.length = SUBCMD_LEN;
#ifndef CONFIG_INPUTHUB_30
		{
			struct pkt_header_resp resp_pkt;
			ret = wait_for_mcu_resp_data_after_send(&cpkt,
				inputhub_mcu_write_cmd(&cpkt, sizeof(cpkt)),
				4000, &resp_pkt, sizeof(resp_pkt)); /* 4000 ms */
			errno = resp_pkt.errno;
		}
#else
		wr.tag = cpkt.hd.tag;
		wr.cmd = cpkt.hd.cmd;
		wr.wr_buf = &cpkt.subcmd;
		wr.wr_len = cpkt.hd.length;
		ret = write_customize_cmd(&wr, &rd, false);
		ret = !ret;
		errno = rd.errno;
#endif
		if (ret == 0) {
			hwlog_err("wait for %s selftest timeout\n", data->name);
			/* SUC-->"0", OTHERS-->"1", size is 2 */
			memcpy(sensor_status.selftest_result[data->tag], "1", 2);
			return size;
		} else {
			if (errno != 0) {
				hwlog_err("%s selftest fail\n", data->name);
				/* flyhorse k : SUC-->"0", OTHERS-->"1" */
				memcpy(sensor_status.selftest_result[data->tag],
					"1", 2); /* src size is 2 */
			} else {
				hwlog_info("%s selftest success\n", data->name);
				/* flyhorse k : SUC-->"0", OTHERS-->"1" */
				memcpy(sensor_status.selftest_result[data->tag],
					"0", 2); /* src size is 2 */
			}
		}
	}
	return size;
}

static ssize_t show_read_airpress(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_sensor_read_airpress_common(dev, attr, buf);
}

static ssize_t show_get_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	hwlog_info("feima %s tag=%d\n", __func__, data->tag);
	return show_get_sensors_id(data->tag, dev, attr, buf);
}

static ssize_t show_calibrate(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	hwlog_info("feima %s tag=%d\n", __func__, data->tag);

	return sensors_calibrate_show(data->tag, dev, attr, buf);
}

static ssize_t store_calibrate(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return sensors_calibrate_store(data->tag, dev, attr, buf, size);
}

/*
 * modify als para online
 * als pattern : vendor1--bh1745,  vendor2-- apds9251
 * step 1:   write data to node in ap. eg:echo xx,xx, ... xx  >  /sys/class/sensors/als_debug_data
 * step 2:   reboot sensorhub. eg: echo 1 > /sys/devices/platform/huawei_sensor/iom3_recovery
 * then sensorhub restart and read als parameter into itself. so als para refreshed.
 */
static ssize_t show_als_debug_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_debug_data_show(data->tag, dev, attr, buf);
}

static ssize_t store_als_debug_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_debug_data_store(data->tag, dev, attr, buf, size);
}

static ssize_t show_selftest_timeout(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	if (data->tag == TAG_THERMOMETER)
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n", 2000); /* 2000 ms */
	else
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n", 1000); /* 1000 ms */
}

static ssize_t show_calibrate_timeout(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		1000); /* 1000 ms */
}

static ssize_t show_calibrate_method(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	/* none:0, DOE:1, DOEaG:2 */
	return show_mag_calibrate_method(dev, attr, buf);
}

static ssize_t show_cap_prox_calibrate_type(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	/* non auto:0, auto:1 */
	return show_cap_prox_calibrate_method(dev, attr, buf);
}

static ssize_t show_cap_prox_calibrate_order(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_cap_prox_calibrate_orders(data->tag, dev, attr, buf);
}

static ssize_t show_sar_sensor_detect(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int final_detect_result;
	unsigned int sar_detect_result;
	unsigned int sar_aux_detect_result;
	char *id_string = NULL;

	id_string = cap_prox_get_sensors_id_string();
	if (id_string == NULL)
		return -1;

	sar_detect_result = get_sar_detect_result();
	sar_aux_detect_result = get_sar_aux_detect_result();

	final_detect_result = sar_detect_result;
	if (!strcmp(id_string, "65541 65572")) {
		final_detect_result = sar_detect_result &&
			sar_aux_detect_result;
		hwlog_info("there two sar sensor\n");
	}

	if (!strcmp(id_string, "65572")) {
		final_detect_result = sar_aux_detect_result;
		hwlog_info("only has sar1 sensor\n");
	}

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%u\n",
		final_detect_result);
}

static ssize_t store_fingersense_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	int *enabled = get_fingersense_enabled();

	if (strict_strtoul(buf, TO_DECIMALISM, &val)) {
		hwlog_err("%s: finger sense enable val %lu invalid",
			__func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: finger sense enable val %ld\n", __func__, val);
	if ((val != 0) && (val != 1)) {
		hwlog_err("%s:finger sense set enable fail, invalid val\n",
			__func__);
		return size;
	}

	if (*enabled == val) {
		hwlog_info("%s:finger sense already at seted state\n",
			__func__);
		return size;
	}

	hwlog_info("%s: finger sense set enable\n", __func__);
	ret = fingersense_enable(val);
	if (ret) {
		hwlog_err("%s: finger sense enable fail: %d\n",
			__func__, ret);
		return size;
	}
	*enabled = val;
	return size;
}

static ssize_t show_fingersense_enable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int *enabled = get_fingersense_enabled();

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"%d\n", *enabled);
}

static ssize_t show_fingersense_data_ready(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	bool *ready = get_fingersense_data_ready();

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n", *ready);
}

static ssize_t show_fingersense_latch_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int size;
	int *enabled = get_fingersense_enabled();
	bool *ready = get_fingersense_data_ready();
	s16 *data = get_fingersense_data();

	size = (MAX_STR_SIZE > (sizeof(*data) * FINGERSENSE_DATA_NSAMPLES)) ?
		(sizeof(*data) * FINGERSENSE_DATA_NSAMPLES) : MAX_STR_SIZE;

	if ((!(*ready)) || (!(*enabled))) {
		hwlog_err("%s:fingersense zaxix not ready %d or not enable %d\n",
			__func__, *ready, *enabled);
		return size;
	}
	if (memcpy_s(buf, MAX_STR_SIZE, (char *)data, size) != EOK) {
		hwlog_err("%s memcpy_s error\n", __func__);
		return -1;
	}

	return size;
}

static ssize_t store_fingersense_req_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	unsigned int sub_cmd = SUB_CMD_ACCEL_FINGERSENSE_REQ_DATA_REQ;
	int *enabled = get_fingersense_enabled();
	bool *ready = get_fingersense_data_ready();

#if defined(CONFIG_HISI_VIBRATOR)
	if ((vibrator_shake == 1) ||
		((HALL_COVERD & (unsigned int)hall_value) &&
		(get_hall_sen_type() == 0))) {
		hwlog_err("coverd, vibrator shaking, not send fingersense req data cmd to mcu\n");
		return -1;
	}
#endif

	if (!(*enabled)) {
		hwlog_err("%s: finger sense not enable,  dont req data\n",
			__func__);
		return size;
	}

	*ready = false;
	ret = fingersense_commu(sub_cmd, sub_cmd, RESP, true);
	if (ret) {
		hwlog_err("%s: finger sense send requst data failed\n",
			__func__);
		return size;
	}
	return size;
}

static ssize_t show_ois_ctrl(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n",
		sensor_status.gyro_ois_status);
}

static ssize_t store_ois_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int source;
	int ret;
	unsigned int cmd;
	unsigned int delay = 200;

	source = simple_strtol(buf, NULL, TO_DECIMALISM);
	if (source == 1) {
		cmd = CMD_CMN_OPEN_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: ois open gyro fail\n", __func__);
			return size;
		}

		cmd = CMD_CMN_INTERVAL_REQ;
		ret = ois_commu(TAG_OIS, cmd, delay, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: set delay fail\n", __func__);
			return size;
		}

		cmd = SUB_CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP, true);
		if (ret) {
			hwlog_err("%s: ois enable fail\n", __func__);
			return size;
		}
		hwlog_info("%s:ois enable succsess\n", __func__);
	} else {
		cmd = SUB_CMD_GYRO_OIS_REQ;
		ret = ois_commu(TAG_GYRO, cmd, source, RESP, true);
		if (ret) {
			hwlog_err("%s:ois close fail\n", __func__);
			return size;
		}

		cmd = CMD_CMN_CLOSE_REQ;
		ret = ois_commu(TAG_OIS, cmd, source, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: ois close gyro fail\n", __func__);
			return size;
		}
		hwlog_info("%s:ois close succsess\n", __func__);
	}
	return size;
}

static ssize_t show_als_ud_rgbl_status(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_ud_rgbl_status_show(data->tag, dev, attr, buf);
}

static ssize_t store_rgb_status(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_rgb_status_store(data->tag, dev, attr, buf, size);
}

static ssize_t show_als_ud_rgbl_block_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_ud_rgbl_block_data_show(data->tag, dev, attr, buf);
}

/* files create for every sensor */
DEVICE_ATTR(enable, 0660, show_enable, store_enable);
DEVICE_ATTR(set_delay, 0660, show_set_delay, store_set_delay);
DEVICE_ATTR(info, 0440, show_info, NULL);

static struct attribute *sensors_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_set_delay.attr,
	&dev_attr_info.attr,
	NULL,
};
static const struct attribute_group sensors_attr_group = {
	.attrs = sensors_attributes,
};

static const struct attribute_group *sensors_attr_groups[] = {
	&sensors_attr_group,
	NULL,
};

static ssize_t store_set_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return therm_store_set_mode(dev, attr, buf, size);
}

static ssize_t show_get_calibrate_index(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return attr_get_therm_calibrate_index(dev, attr, buf);
}

/* files create for specific sensor */
static DEVICE_ATTR(get_data, 0660, show_get_data, store_get_data);
static DEVICE_ATTR(self_test, 0660, show_selftest, store_selftest);
static DEVICE_ATTR(self_test_timeout, 0440, show_selftest_timeout, NULL);
static DEVICE_ATTR(read_airpress, 0440, show_read_airpress, NULL); /* only for airpress */
static DEVICE_ATTR(set_calidata, 0660, show_calibrate, store_calibrate); /* only for airpress */
static DEVICE_ATTR(calibrate, 0660, show_calibrate, store_calibrate);
static DEVICE_ATTR(als_debug_data, 0660, show_als_debug_data, store_als_debug_data);
static DEVICE_ATTR(calibrate_timeout, 0440, show_calibrate_timeout, NULL);
static DEVICE_ATTR(calibrate_method, 0440, show_calibrate_method, NULL); /* only for magnetic */
static DEVICE_ATTR(cap_prox_calibrate_type, 0440, show_cap_prox_calibrate_type, NULL);
static DEVICE_ATTR(cap_prox_calibrate_order, 0440, show_cap_prox_calibrate_order, NULL);
static DEVICE_ATTR(sar_sensor_detect, 0440, show_sar_sensor_detect, NULL);
static DEVICE_ATTR(get_sensor_id, 0440, show_get_sensor_id, NULL);
static DEVICE_ATTR(set_fingersense_enable, 0660, show_fingersense_enable,
	store_fingersense_enable);
static DEVICE_ATTR(fingersense_data_ready, 0440, show_fingersense_data_ready, NULL);
static DEVICE_ATTR(fingersense_latch_data, 0440, show_fingersense_latch_data, NULL);
static DEVICE_ATTR(fingersense_req_data, 0220, NULL, store_fingersense_req_data);
static DEVICE_ATTR(rpc_motion_req, 0660, show_rpc_motion_req, store_rpc_motion_req);
static DEVICE_ATTR(rpc_sar_service_req, 0660, NULL, store_rpc_sar_service_req);
static DEVICE_ATTR(ois_ctrl, 0660, show_ois_ctrl, store_ois_ctrl);
static DEVICE_ATTR(als_ud_rgbl, 0660, show_als_ud_rgbl_status, store_rgb_status);
static DEVICE_ATTR(als_ud_rgbl_block, 0440, show_als_ud_rgbl_block_data, NULL);
static DEVICE_ATTR(set_mode, 0660, NULL, store_set_mode);
static DEVICE_ATTR(get_calibrate_index, 0440, show_get_calibrate_index, NULL);
static DEVICE_ATTR(acc_sensorlist_info, 0664, show_acc_sensorlist_info, NULL);
static DEVICE_ATTR(mag_sensorlist_info, 0664, show_mag_sensorlist_info, NULL);
static DEVICE_ATTR(gyro_sensorlist_info, 0664, show_gyro_sensorlist_info, NULL);
static DEVICE_ATTR(gyro_position_info, 0660, show_gyro_position_info, NULL);
static DEVICE_ATTR(ungyro_time_offset, 0664, show_ungyro_time_offset,
	store_ungyro_time_offset);
static DEVICE_ATTR(unacc_time_offset, 0664, show_unacc_time_offset,
	store_unacc_time_offset);

int posture_sensor_enable(void)
{
	int ret;
	interval_param_t param;

	memset_s(&param, sizeof(param), 0, sizeof(param));
	param.period = 50; /* 50 ms */

	inputhub_sensor_enable(TAG_POSTURE, true);

	ret = inputhub_sensor_setdelay(TAG_POSTURE, &param);
	if (ret)
		hwlog_err("set delay cmd to mcu fail,ret=%d\n", ret);
	return 0;
}

static ssize_t show_judge_sensor_in_board_status(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_sensor_in_board_status_sysfs(dev, attr, buf);
}
static DEVICE_ATTR(judge_sensor_in_board_status, 0440,
	show_judge_sensor_in_board_status, NULL);

static DEVICE_ATTR(ps_sensorlist_info, 0664, show_ps_sensorlist_info, NULL);

static ssize_t show_als_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_sensorlist_info_show(data->tag, dev,	attr, buf);
}
static DEVICE_ATTR(als_sensorlist_info, 0664,
	show_als_sensorlist_info, NULL);
static DEVICE_ATTR(calibrate_threshold_from_mag, 0664,
	calibrate_threshold_from_mag_show, NULL);
static DEVICE_ATTR(airpress_sensorlist_info, 0664,
	show_airpress_sensorlist_info, NULL);

static ssize_t show_als_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_offset_data_show(data->tag, dev, attr, buf);
}
static DEVICE_ATTR(als_offset_data, 0444, show_als_offset_data, NULL);

static DEVICE_ATTR(ps_offset_data, 0444, show_ps_offset_data, NULL);
static DEVICE_ATTR(acc_offset_data, 0444, show_acc_offset_data, NULL);
static DEVICE_ATTR(acc1_offset_data, 0444, show_acc1_offset_data, NULL);
static DEVICE_ATTR(gyro_offset_data, 0444, show_gyro_offset_data, NULL);
static DEVICE_ATTR(gyro1_offset_data, 0444, show_gyro1_offset_data, NULL);

static ssize_t attr_set_als_always_on_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_always_on_store(data->tag, dev, attr, buf, size);
}
static DEVICE_ATTR(always_on, 0660, NULL, attr_set_als_always_on_store);

static ssize_t show_als_mmi_para(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_mmi_para_show(data->tag, dev, attr, buf);
}
static DEVICE_ATTR(als_mmi_para, 0444, show_als_mmi_para, NULL);

static DEVICE_ATTR(airpress_calibrate, 0660, attr_airpress_calibrate_show,
	attr_airpress_calibrate_write);

static DEVICE_ATTR(airpress_set_tp_info, 0220,
	NULL, attr_airpress_set_tp_info_store);

static ssize_t attr_als_calibrate_under_tp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_calibrate_under_tp_store(data->tag, dev, attr, buf, count);
}

static ssize_t attr_als_calibrate_under_tp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_calibrate_under_tp_show(data->tag, dev, attr, buf);
}

static DEVICE_ATTR(als_calibrate_under_tp, 0660,
	attr_als_calibrate_under_tp_show, attr_als_calibrate_under_tp_store);

static ssize_t attr_als_rgb_data_under_tp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_rgb_data_under_tp_store(data->tag, dev, attr, buf, count);
}

static ssize_t attr_als_rgb_data_under_tp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_rgb_data_under_tp_show(data->tag, dev, attr, buf);
}

static DEVICE_ATTR(als_rgb_data_under_tp, 0660,
	attr_als_rgb_data_under_tp_show, attr_als_rgb_data_under_tp_store);

static ssize_t attr_als_under_tp_calidata_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_under_tp_calidata_store(data->tag, dev, attr, buf, count);
}

/* return underscreen als to node file */
static ssize_t attr_als_under_tp_calidata_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_under_tp_calidata_show(data->tag, dev, attr, buf);
}

static DEVICE_ATTR(set_als_under_tp_calidata, 0660,
	attr_als_under_tp_calidata_show, attr_als_under_tp_calidata_store);

/* return als_support_under_screen_cali to node file */
static ssize_t attr_als_calibrate_after_sale_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return als_calibrate_after_sale_show(data->tag, dev, attr, buf);
}
static DEVICE_ATTR(als_calibrate_after_sale, 0440,
	attr_als_calibrate_after_sale_show, NULL);

static DEVICE_ATTR(ps_calibrate_after_sale, 0440,
	attr_ps_calibrate_after_sale_show, NULL);
static DEVICE_ATTR(cap_prox_data_mode, 0220, NULL, attr_cap_prox_data_mode_store);
static DEVICE_ATTR(abov_data_write, 0220, NULL, attr_cap_prox_abov_data_write);
static DEVICE_ATTR(abov_bootloader_verify, 0660,
	attr_abov_bootloader_verify_result_show, attr_abov_bootloader_verify);
static DEVICE_ATTR(abov_bootloader_enter, 0660,
	attr_abov_bootloader_enter_show, attr_abov_bootloader_enter);
static DEVICE_ATTR(abov_reg_rw, 0660, abov_reg_dump_show, abov_reg_dump_store);
static DEVICE_ATTR(ultra_ps_feature, 0440, attr_ultra_ps_feature_show, NULL);
static struct attribute *acc_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_acc_sensorlist_info.attr,
	&dev_attr_acc_offset_data.attr,
	&dev_attr_unacc_time_offset.attr,
	&dev_attr_judge_sensor_in_board_status.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group acc_sensor_attrs_grp = {
	.attrs = acc_sensor_attrs,
};

static struct attribute *acc1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_acc_sensorlist_info.attr,
	&dev_attr_acc1_offset_data.attr,
	&dev_attr_unacc_time_offset.attr,
	NULL,
};

static const struct attribute_group acc1_sensor_attrs_grp = {
	.attrs = acc1_sensor_attrs,
};

static ssize_t store_sleeve_test_prepare(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;

	if (strict_strtoul(buf, TO_DECIMALISM, &val)) {
		hwlog_err("%s: sleeve_test enable val invalid", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: sleeve_test enable val %ld\n", __func__, val);
	if ((val != 0) && (val != 1)) {
		hwlog_err("%s:sleeve_test set enable fail, invalid val\n",
			__func__);
		return -EINVAL;
	}

	if (sleeve_test_enabled == val) {
		hwlog_info("%s:sleeve_test already at seted state\n",
			__func__);
		return size;
	}

	ret = sleeve_test_ps_prepare(val);
	if (ret) {
		hwlog_err("%s: sleeve_test enable fail: %d\n", __func__, ret);
		return -EINVAL;
	}
	sleeve_test_enabled = val;
	hwlog_info("%s: sleeve_test set enable success\n", __func__);
	return size;
}

static ssize_t show_sleeve_test_threshhold(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	u8 phone_color = get_phone_color();
	struct sleeve_detect_pare *sleeve_para = get_sleeve_detect_parameter();

	for (i = 0; i < MAX_PHONE_COLOR_NUM; i++) {
		if (phone_color == sleeve_para[i].tp_color) {
			hwlog_info("sleeve_test threshhold %d, phone_color %d\n",
				sleeve_para[i].sleeve_detect_threshhold,
				phone_color);
			return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
				"%d\n",
				sleeve_para[i].sleeve_detect_threshhold);
		}
	}
	hwlog_info("sleeve_test get threshhold fail, phone_color %d\n", phone_color);
	return -1;
}

static DEVICE_ATTR(sleeve_test_prepare, 0220, NULL, store_sleeve_test_prepare);
static DEVICE_ATTR(sleeve_test_threshhold, 0440, show_sleeve_test_threshhold, NULL);

static ssize_t show_support_absolute_threshold(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ps_device_info *dev_info = NULL;

	if (!dev || !attr || !buf)
		return -1;
	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return -1;
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		dev_info->ps_support_abs_threshold);
}

static DEVICE_ATTR(support_absolute_threshold, 0664,
	show_support_absolute_threshold, NULL);

static struct attribute *ps_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_prepare.attr,
	&dev_attr_ps_sensorlist_info.attr,
	&dev_attr_ps_offset_data.attr,
	&dev_attr_support_absolute_threshold.attr,
	&dev_attr_ps_calibrate_after_sale.attr,
	&dev_attr_ultra_ps_feature.attr,
	NULL,
};

static const struct attribute_group ps_sensor_attrs_grp = {
	.attrs = ps_sensor_attrs,
};

static struct attribute *als_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_als_debug_data.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_threshhold.attr,
	&dev_attr_als_sensorlist_info.attr,
	&dev_attr_calibrate_threshold_from_mag.attr,
	&dev_attr_als_offset_data.attr,
	&dev_attr_als_calibrate_under_tp.attr,
	&dev_attr_set_als_under_tp_calidata.attr,
	&dev_attr_als_rgb_data_under_tp.attr,
	&dev_attr_als_ud_rgbl.attr,
	&dev_attr_als_ud_rgbl_block.attr,
	&dev_attr_als_calibrate_after_sale.attr,
	&dev_attr_always_on.attr,
	&dev_attr_als_mmi_para.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static struct attribute *als1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_als_debug_data.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_threshhold.attr,
	&dev_attr_als_sensorlist_info.attr,
	&dev_attr_calibrate_threshold_from_mag.attr,
	&dev_attr_als_offset_data.attr,
	&dev_attr_als_calibrate_under_tp.attr,
	&dev_attr_set_als_under_tp_calidata.attr,
	&dev_attr_als_rgb_data_under_tp.attr,
	&dev_attr_als_ud_rgbl.attr,
	&dev_attr_als_ud_rgbl_block.attr,
	&dev_attr_als_calibrate_after_sale.attr,
	&dev_attr_always_on.attr,
	&dev_attr_als_mmi_para.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static struct attribute *als2_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_als_debug_data.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_threshhold.attr,
	&dev_attr_als_sensorlist_info.attr,
	&dev_attr_calibrate_threshold_from_mag.attr,
	&dev_attr_als_offset_data.attr,
	&dev_attr_als_calibrate_under_tp.attr,
	&dev_attr_set_als_under_tp_calidata.attr,
	&dev_attr_als_rgb_data_under_tp.attr,
	&dev_attr_als_ud_rgbl.attr,
	&dev_attr_als_ud_rgbl_block.attr,
	&dev_attr_als_calibrate_after_sale.attr,
	&dev_attr_always_on.attr,
	&dev_attr_als_mmi_para.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group als_sensor_attrs_grp = {
	.attrs = als_sensor_attrs,
};

static const struct attribute_group als1_sensor_attrs_grp = {
	.attrs = als1_sensor_attrs,
};

static const struct attribute_group als2_sensor_attrs_grp = {
	.attrs = als2_sensor_attrs,
};

static struct attribute *mag_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate_method.attr,
	&dev_attr_mag_sensorlist_info.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group mag_sensor_attrs_grp = {
	.attrs = mag_sensor_attrs,
};

static struct attribute *mag1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate_method.attr,
	&dev_attr_mag_sensorlist_info.attr,
	NULL,
};

static const struct attribute_group mag1_sensor_attrs_grp = {
	.attrs = mag1_sensor_attrs,
};


static struct attribute *hall_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	NULL,
};

static const struct attribute_group hall_sensor_attrs_grp = {
	.attrs = hall_sensor_attrs,
};

static struct attribute *gyro_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_gyro_sensorlist_info.attr,
	&dev_attr_gyro_position_info.attr,
	&dev_attr_gyro_offset_data.attr,
	&dev_attr_ungyro_time_offset.attr,
	&dev_attr_get_sensor_id.attr,
	NULL,
};

static const struct attribute_group gyro_sensor_attrs_grp = {
	.attrs = gyro_sensor_attrs,
};

static struct attribute *gyro1_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_gyro_sensorlist_info.attr,
	&dev_attr_gyro_position_info.attr,
	&dev_attr_gyro1_offset_data.attr,
	&dev_attr_ungyro_time_offset.attr,
	NULL,
};

static const struct attribute_group gyro1_sensor_attrs_grp = {
	.attrs = gyro1_sensor_attrs,
};


static struct attribute *airpress_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_read_airpress.attr,
	&dev_attr_set_calidata.attr,
	&dev_attr_airpress_calibrate.attr,
	&dev_attr_airpress_sensorlist_info.attr,
	&dev_attr_airpress_set_tp_info.attr,
	NULL,
};

static const struct attribute_group airpress_sensor_attrs_grp = {
	.attrs = airpress_sensor_attrs,
};

static struct attribute *finger_sensor_attrs[] = {
	&dev_attr_set_fingersense_enable.attr,
	&dev_attr_fingersense_data_ready.attr,
	&dev_attr_fingersense_latch_data.attr,
	&dev_attr_fingersense_req_data.attr,
	NULL,
};

static const struct attribute_group finger_sensor_attrs_grp = {
	.attrs = finger_sensor_attrs,
};

static struct attribute *handpress_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	NULL,
};
static const struct attribute_group handpress_sensor_attrs_grp = {
	.attrs = handpress_sensor_attrs,
};

static struct attribute *ois_sensor_attrs[] = {
	&dev_attr_ois_ctrl.attr,
	NULL,
};

static const struct attribute_group ois_sensor_attrs_grp = {
	.attrs = ois_sensor_attrs,
};
static struct attribute *cap_prox_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_cap_prox_calibrate_type.attr,
	&dev_attr_cap_prox_calibrate_order.attr,
	&dev_attr_sar_sensor_detect.attr,
	&dev_attr_cap_prox_data_mode.attr,
	&dev_attr_get_sensor_id.attr,
	&dev_attr_abov_data_write.attr,
	&dev_attr_abov_bootloader_verify.attr,
	&dev_attr_abov_bootloader_enter.attr,
	&dev_attr_abov_reg_rw.attr,
	NULL,
};
static const struct attribute_group cap_prox_sensor_attrs_grp = {
	.attrs = cap_prox_sensor_attrs,
};

static struct attribute *cap_prox1_sensor_attrs[] = {
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_cap_prox_calibrate_type.attr,
	&dev_attr_cap_prox_calibrate_order.attr,
	&dev_attr_sar_sensor_detect.attr,
	&dev_attr_cap_prox_data_mode.attr,
	NULL,
};
static const struct attribute_group cap_prox1_sensor_attrs_grp = {
	.attrs = cap_prox1_sensor_attrs,
};


static struct attribute *orientation_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	NULL,
};

static const struct attribute_group orientation_attrs_grp = {
	.attrs = orientation_sensor_attrs,
};
static struct attribute *rpc_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_rpc_motion_req.attr,
	&dev_attr_rpc_sar_service_req.attr,
	NULL,
};
static const struct attribute_group rpc_sensor_attrs_grp = {
	.attrs = rpc_sensor_attrs,
};

static struct attribute *thermometer_sensor_attrs[] = {
	&dev_attr_calibrate.attr,
	&dev_attr_set_mode.attr,
	&dev_attr_get_sensor_id.attr,
	&dev_attr_get_calibrate_index.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	NULL,
};

static const struct attribute_group thermometer_sensor_attrs_grp = {
	.attrs = thermometer_sensor_attrs,
};

static ssize_t show_step_counter_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", g_stepcountdocm);
}

static DEVICE_ATTR(step_counter_info, 0664, show_step_counter_info, NULL);

static struct attribute *g_stepcounterattrs[] = {
	&dev_attr_step_counter_info.attr,
	NULL,
};

static const struct attribute_group step_counter_attrs_grp = {
	.attrs = g_stepcounterattrs,
};

static ssize_t show_hinge_status_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!buf)
		return 0;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		get_fold_hinge_status());
}

static DEVICE_ATTR(hinge_status_info, 0664, show_hinge_status_info, NULL);

static struct attribute *hinge_sensor_attrs[] = {
	&dev_attr_hinge_status_info.attr,
	NULL,
};

static const struct attribute_group g_hinge_sensor_attrs_grp = {
	.attrs = hinge_sensor_attrs,
};

static struct sensor_cookie all_sensors[] = {
	{
		.tag = TAG_ACCEL,
		.name = "acc_sensor",
		.attrs_group = &acc_sensor_attrs_grp,
	},
	{
		.tag = TAG_PS,
		.name = "ps_sensor",
		.attrs_group = &ps_sensor_attrs_grp,
	},
	{
		.tag = TAG_ALS,
		.name = "als_sensor",
		.attrs_group = &als_sensor_attrs_grp,
	},
	{
		.tag = TAG_MAG,
		.name = "mag_sensor",
		.attrs_group = &mag_sensor_attrs_grp,
	},
	{
		.tag = TAG_HALL,
		.name = "hall_sensor",
		.attrs_group = &hall_sensor_attrs_grp,
	},
	{
		.tag = TAG_GYRO,
		.name = "gyro_sensor",
		.attrs_group = &gyro_sensor_attrs_grp,
	},
	{
		.tag = TAG_PRESSURE,
		.name = "airpress_sensor",
		.attrs_group = &airpress_sensor_attrs_grp,
	},
	{
		.tag = TAG_FINGERSENSE,
		.name = "fingersense_sensor",
		.attrs_group = &finger_sensor_attrs_grp,
	},
	{
		.tag = TAG_HANDPRESS,
		.name = "handpress_sensor",
		.attrs_group = &handpress_sensor_attrs_grp,
	},
	{
		.tag = TAG_OIS,
		.name = "ois_sensor",
		.attrs_group = &ois_sensor_attrs_grp,
	},
	{
		.tag = TAG_CAP_PROX,
		.name = "cap_prox_sensor",
		.attrs_group = &cap_prox_sensor_attrs_grp,
	},
	{
		.tag = TAG_ORIENTATION,
		.name = "orientation_sensor",
		.attrs_group = &orientation_attrs_grp,
	},
	{
		.tag = TAG_RPC,
		.name = "rpc_sensor",
		.attrs_group = &rpc_sensor_attrs_grp,
	},
	{
		.tag = TAG_ACC1,
		.name = "acc1_sensor",
		.attrs_group = &acc1_sensor_attrs_grp,
	},
	{
		.tag = TAG_GYRO1,
		.name = "gyro1_sensor",
		.attrs_group = &gyro1_sensor_attrs_grp,
	},
	{
		.tag = TAG_ALS1,
		.name = "als1_sensor",
		.attrs_group = &als1_sensor_attrs_grp,
	},
	{
		.tag = TAG_MAG1,
		.name = "mag1_sensor",
		.attrs_group = &mag1_sensor_attrs_grp,
	},
	{
		.tag = TAG_ALS2,
		.name = "als2_sensor",
		.attrs_group = &als2_sensor_attrs_grp,
	},
	{
		.tag = TAG_CAP_PROX1,
		.name = "cap_prox1_sensor",
		.attrs_group = &cap_prox1_sensor_attrs_grp,
	},
	{
		.tag = TAG_STEP_COUNTER,
		.name = "step_counter",
		.attrs_group = &step_counter_attrs_grp,
	 },
	 {
		.tag = TAG_THERMOMETER,
		.name = "thermometer_sensor",
		.attrs_group = &thermometer_sensor_attrs_grp,
	},
	{
		.tag = TAG_POSTURE,
		.name = "hinge_sensor",
		.attrs_group = &g_hinge_sensor_attrs_grp,
	},
};

void send_lcd_freq_to_sensorhub(uint32_t lcd_freq)
{
	ps_send_lcd_freq_to_sensorhub(lcd_freq);
}

void save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level)
{
	als_save_light_to_sensorhub(mipi_level, bl_level);
}

void report_fold_status_when_poweroff_charging(int status)
{
	static int pre_status = -1;
	char *fold_on[] = {"FOLDSTAUS=0", NULL};
	char *fold_off[] = {"FOLDSTAUS=1", NULL};
	int pos_type = 21; // define posture register sensor type

	if (is_power_off_charging_posture() != 1) // off charge mode
		return;

	if (status == pre_status)
		return;
	pre_status = status;
	hwlog_info("%s name = %s, status = %d\n", __func__,
		all_sensors[pos_type].name, status);
	if (status == 0)
		kobject_uevent_env(&all_sensors[pos_type].dev->kobj, KOBJ_CHANGE,
			fold_on);
	else
		kobject_uevent_env(&all_sensors[pos_type].dev->kobj, KOBJ_CHANGE,
			fold_off);
}

struct device *get_sensor_device_by_name(const char *name)
{
	int i;

	if (!name)
		return NULL;

	for (i = 0; i < sizeof(all_sensors) / sizeof(all_sensors[0]); ++i) {
		if (all_sensors[i].name && (strcmp(name, all_sensors[i].name) == 0))
			return all_sensors[i].dev;
	}

	return NULL;
}

static void init_sensors_get_data(void)
{
	int tag;

	for (tag = SENSORHUB_TYPE_BEGIN; tag < SENSORHUB_TYPE_END; ++tag) {
		atomic_set(&sensor_status.get_data[tag].reading, 0);
		init_completion(&sensor_status.get_data[tag].complete);
	}

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		/* 1 means fail */
		if (memcpy_s(sensor_status.selftest_result[tag],
			sizeof(sensor_status.selftest_result[tag]),
			"1", sizeof("1")) != EOK) {
			hwlog_err("%s memcpy_s error tag=%d\n", __func__, tag);
			return;
		}
	}
}

/* device_create->device_register->device_add->device_add_attrs->device_add_attributes */
static int sensors_register(void)
{
	int i;

	for (i = 0; i < sizeof(all_sensors) / sizeof(all_sensors[0]); ++i) {
		all_sensors[i].dev = device_create(sensors_class, NULL, 0,
			&all_sensors[i], all_sensors[i].name);
		if (!all_sensors[i].dev) {
			hwlog_err("[%s] Failed", __func__);
			return -1;
		} else {
			if (all_sensors[i].attrs_group &&
				sysfs_create_group(&all_sensors[i].dev->kobj,
					all_sensors[i].attrs_group))
				hwlog_err("create files failed in %s\n",
					__func__);
		}
	}
	return 0;
}

static void sensors_unregister(void)
{
	device_destroy(sensors_class, 0);
}

static void get_docom_step_counter(void)
{
	char *stepcountty = NULL;
	struct device_node *sensorhubnode = NULL;

	g_stepcountdocm = 0;
	sensorhubnode = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (of_property_read_string(sensorhubnode, "docom_step_counter",
			(const char **)&stepcountty) == 0) {
		hwlog_info("get_docom_step_counter step_count_ty is %s\n",
						stepcountty);
		if (!strcmp("enabled", stepcountty))
			g_stepcountdocm = 1;
	}
	hwlog_info("%s : docom_step_counter status is %d\n", __func__,
					g_stepcountdocm);
}

static int sensors_feima_init(void)
{
	if (is_sensorhub_disabled())
		return -1;
	get_docom_step_counter();
	sensors_class = class_create(THIS_MODULE, "sensors");
	if (IS_ERR(sensors_class))
		return PTR_ERR(sensors_class);
	sensors_class->dev_groups = sensors_attr_groups;
	sensors_register();
	init_sensors_get_data();
	create_debug_files();
	if (ps_ud_workqueue_init())
		return -1;
	return 0;
}

static void sensors_feima_exit(void)
{
	sensors_unregister();
	class_destroy(sensors_class);
}

late_initcall_sync(sensors_feima_init);
module_exit(sensors_feima_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SensorHub feima driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
