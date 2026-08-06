/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: Sensor feima driver
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

#include <securec.h>

#include "als_detect.h"
#include "als_para_debug.h"
#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_route.h"
#include "sensor_config.h"
#include "sensor_detect.h"
#include "sensor_feima.h"
#include "sensor_sysfs.h"

#define MIN_CAP_PROX_MODE 0
#define MAZ_CAP_PROX_MODE 2
#define MAR_PHONE_TYPE 47
#define ONE_MILLION 1000000

#define ABOV_WRITE_DATA_LENGTH      36
#define ABOV_CHECKSUM_DATA_LENGTH   2
#define ABOV_ENTER_BOOTLOADER_CMD   5
#define ABOV_STR_TO_HEX             10
#define ABOV_STR_TO_HEX_HIGH_ORDER  4

struct class *sensors_class;
int sleeve_test_enabled = 0;

static uint8_t  abov_device_id;
static bool rpc_motion_request;
static time_t get_data_last_time;
static unsigned long sar_service_info = 0;
extern volatile int vibrator_shake;
extern volatile int hall_value;
static enum ret_type airpress_calibration_res = RET_INIT; /* airpress calibrate result */
extern uint8_t gyro_position;
extern struct als_platform_data als_data;
extern struct sar_sensor_detect semtech_sar_detect;
extern struct sar_sensor_detect adi_sar_detect;
extern struct sar_sensor_detect cypress_sar_detect;
extern struct sar_sensor_detect g_abov_sar_detect;
extern struct sar_sensor_detect aw9610_sar_detect;

extern struct compass_platform_data mag_data;
extern int gsensor_offset[ACC_CALIBRATE_DATA_LENGTH]; /* g-sensor calibrate data */
extern int gyro_sensor_offset[GYRO_CALIBRATE_DATA_LENGTH];
extern int ps_sensor_offset[PS_CALIBRATE_DATA_LENGTH];
extern int mag_threshold_for_als_calibrate;

static bool camera_set_rpc_flag = false;

static int rpc_commu(unsigned int cmd, unsigned int pare, uint16_t motion)
{
	int ret;
	struct write_info pkg_ap;
	rpc_ioctl_t pkg_ioctl;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	pkg_ap.tag = TAG_RPC;
	pkg_ap.cmd = cmd;
	pkg_ioctl.sub_cmd = pare;
	pkg_ioctl.sar_info = motion;
	pkg_ap.wr_buf = &pkg_ioctl;
	pkg_ap.wr_len = sizeof(pkg_ioctl);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send rpc cmd%d to mcu fail,ret=%d\n", cmd, ret);
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

static ssize_t show_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.status[data->tag]);
}

static ssize_t store_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	const char *operation = NULL;
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (ap_sensor_enable(data->tag, (val == 1)))
		return size;

	operation = ((val == 1) ? "enable" : "disable");
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = data->tag;
	pkg_ap.cmd = (val == 1) ? CMD_CMN_OPEN_REQ : CMD_CMN_CLOSE_REQ;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret != 0) {
		hwlog_err("%s %s failed, ret = %d in %s\n", operation, data->name, ret, __func__);
		return size;
	}

	if (pkg_mcu.errno != 0)
		hwlog_err("%s %s failed errno = %d in %s\n", operation, data->name, pkg_mcu.errno, __func__);
	else
		hwlog_info("%s %s success\n", operation, data->name);

	return size;
}
static int rpc_status_change(void)
{
	int ret;

	sar_service_info = (sar_service_info & ~BIT(9)) | ((bool)rpc_motion_request << 9);
	hwlog_info("sar_service_info is %lu\n", sar_service_info);
	ret = rpc_motion(sar_service_info);
	if (ret) {
		hwlog_err("rpc motion fail: %d\n", ret);
		return ret;
	}
	return ret;
}

/* add for Txx front&wide camera radio frequency interference */
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

	hwlog_info("status %d, sar_service_info is %lu\n", status, sar_service_info);
	ret = rpc_status_change();
	if (ret) {
		hwlog_err("rpc status change fail: %d\n", ret);
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(rpc_status_change_for_camera);

static ssize_t store_rpc_motion_req(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long value = 0;

	if (strict_strtoul(buf, TO_DECIMALISM, &value))
		hwlog_err("%s: rpc motion request val %lu invalid", __func__, value);

	hwlog_info("%s: rpc motion request val %lu\n", __func__, value);
	if ((value != 0) && (value != 1)) {
		hwlog_err("%s: set enable fail, invalid val\n", __func__);
		return size;
	}
	rpc_motion_request = value;
	rpc_status_change();
	return size;
}
static ssize_t store_rpc_sar_service_req(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long sar_service = 0;

	if (strict_strtoul(buf, TO_DECIMALISM, &sar_service))
		hwlog_err("rpc_sar_service_req strout error.\n");
	if (sar_service > 0xFFFF) {
		hwlog_err("%s: set enable fail, invalid val\n", __func__);
		return size;
	}
	hwlog_info("%s: rpc sar service request val %lu, buf is %s.\n", __func__, sar_service, buf);
	if (camera_set_rpc_flag) {
		sar_service = sar_service | 0x400; /* camera set bit10 */
		hwlog_info("%s: camera_set_rpc_flag, rpc sar service val %lu\n", __func__, sar_service);
	}
	sar_service_info = sar_service;
	rpc_status_change();
	return size;
}
static ssize_t show_rpc_motion_req(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", rpc_motion_request);
}
static ssize_t show_set_delay(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.delay[data->tag]);
}

static ssize_t store_set_delay(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_cmn_interval_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	struct sensor_cookie *data =
		(struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&cpkt, 0, sizeof(cpkt));
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (ap_sensor_setdelay(data->tag, val))
		return size;

	if (val >= 10 && val <= 1000) { /* range [10, 1000] */
		pkg_ap.tag = data->tag;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		cpkt.param.period = val;
		pkg_ap.wr_buf = &hd[1];
		pkg_ap.wr_len = sizeof(cpkt.param);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
		if (ret != 0) {
			hwlog_err("set %s delay cmd to mcu fail, ret = %d in %s\n", data->name, ret, __func__);
			return ret;
		}
		if (pkg_mcu.errno != 0) {
			hwlog_err("set %s delay failed errno %d in %s\n", data->name, pkg_mcu.errno, __func__);
			return -EINVAL;
		}
		hwlog_info("set %s delay %ld success\n", data->name, val);
	} else {
		hwlog_err("set %s delay_ms %d ms range error in %s\n", data->name, (int)val, __func__);
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

extern uint8_t tag_to_hal_sensor_type[TAG_SENSOR_END];
static ssize_t show_get_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	unsigned int hal_sensor_tag = tag_to_hal_sensor_type[data->tag];

	CHECK_SENSOR_COOKIE(data);
	{
		struct t_sensor_get_data *get_data = &sensor_status.get_data[hal_sensor_tag];
		unsigned int offset = 0;
		int i = 0;
		int mem_num;

		atomic_set(&get_data->reading, 1);
		reinit_completion(&get_data->complete);
		/* return -ERESTARTSYS if interrupted, 0 if completed. */
		if (wait_for_completion_interruptible(&get_data->complete) == 0) {
			for (mem_num = get_data->data.length / sizeof(get_data->data.value[0]); i < mem_num; i++) {
				if ((data->tag == TAG_ALS) && (i == 0))
					get_data->data.value[0] = get_data->data.value[0] / ALS_MCU_HAL_CONVER;
				if (data->tag == TAG_ACCEL)
					/* need be divided by 1000.0 for high resolution */
					get_data->data.value[i] = get_data->data.value[i] / ACC_CONVERT_COFF;
				offset += sprintf(buf + offset, "%d\t", get_data->data.value[i]);
			}
			offset += sprintf(buf + offset, "\n");
		}

		return offset;
	}

	return 0;
}

static ssize_t store_get_data(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	{
		struct timeval tv;
		struct sensor_data event;
		int arg;
		int argc = 0;
		time_t get_data_current_time;

		memset(&tv, 0, sizeof(struct timeval));
		do_gettimeofday(&tv);
		get_data_current_time = tv.tv_sec;
		if ((get_data_current_time - get_data_last_time) < 1) {
			hwlog_info("%s:time interval is less than 1s(from %u to %u), skip\n",
				__func__, (uint32_t)get_data_last_time, (uint32_t)get_data_current_time);
			return size;
		}
		get_data_last_time = get_data_current_time;

		/* parse cmd buffer */
		for (; (buf = get_str_begin(buf)) != NULL; buf = get_str_end(buf)) {
			if (get_arg(buf, &arg)) {
				if (argc < (sizeof(event.value) / sizeof(event.value[0])))
					event.value[argc++] = arg;
				else
					hwlog_err("too many args, %d will be ignored\n", arg);
			}
		}

		/* fill sensor event and write to sensor event buffer */
		report_sensor_event(data->tag, event.value, sizeof(event.value[0]) * argc);
	}

	return size;
}

static ssize_t show_selftest(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%s\n", sensor_status.selftest_result[data->tag]);
}

static ssize_t store_selftest(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	pkt_subcmd_req_t cpkt;
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);
	struct pkt_header_resp resp_pkt;

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
		if (wait_for_mcu_resp_data_after_send(&cpkt, inputhub_mcu_write_cmd(&cpkt,
			sizeof(cpkt)), 4000, &resp_pkt, sizeof(resp_pkt)) == 0) { /* 4000 ms */
			hwlog_err("wait for %s selftest timeout\n", data->name);
			/* SUC-->"0", OTHERS-->"1", size is 2 */
			memcpy(sensor_status.selftest_result[data->tag], "1", 2);
			return size;
		}
		if (resp_pkt.errno != 0) {
			hwlog_err("%s selftest fail\n", data->name);
			/* SUC-->"0", OTHERS-->"1", size is 2 */
			memcpy(sensor_status.selftest_result[data->tag], "1", 2);
		} else {
			hwlog_info("%s selftest success\n", data->name);
			/* SUC-->"0", OTHERS-->"1", size is 2 */
			memcpy(sensor_status.selftest_result[data->tag], "0", 2);
		}
	}

	return size;
}

static ssize_t show_read_airpress(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_sensor_read_airpress_common(dev, attr, buf);
}

static ssize_t show_calibrate(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return sensors_calibrate_show(data->tag, dev, attr, buf);
}

static ssize_t store_calibrate(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return sensors_calibrate_store(data->tag, dev, attr, buf, size);
}

/* modify als para online */
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

static ssize_t show_selftest_timeout(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", 3000); /* 3000ms */
}

static ssize_t show_calibrate_timeout(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", 3000); /* 3000 ms */
}

static ssize_t show_mag_calibrate_method(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", mag_data.calibrate_method);
}

static ssize_t show_calibrate_method(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_mag_calibrate_method(dev, attr, buf); /* none:0, DOE:1, DOEaG:2 */
}
static ssize_t show_cap_prox_calibrate_type(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_cap_prox_calibrate_method(dev, attr, buf); /* non auto:0, auto:1 */
}
static ssize_t show_cap_prox_calibrate_order(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sensor_cookie *data = (struct sensor_cookie *)dev_get_drvdata(dev);

	CHECK_SENSOR_COOKIE(data);
	return show_cap_prox_calibrate_orders(dev, attr, buf);
}

#define SAR_SENSOR_DEDECT_LENGTH 10
static int sar_sensor_i2c_detect(struct sar_sensor_detect sar_detect)
{
	int detect_result;
	int ret;
	uint8_t bus_num;
	uint8_t i2c_address;
	uint8_t reg_add;
	uint8_t buf_temp[SAR_SENSOR_DEDECT_LENGTH] = { 0 };

	/* ##(bus_num)##(i2c_addr)##(reg_addr)##(len) */
	bus_num = sar_detect.cfg.bus_num;
	i2c_address = sar_detect.cfg.i2c_address;
	reg_add = sar_detect.chip_id;
	buf_temp[0] = 0;
	hwlog_info("In %s! bus_num = 0x%x, i2c_address = 0x%x, reg_add = 0x%x , chip_id_value = 0x%x 0x%x\n",
		__func__, bus_num, i2c_address, reg_add, sar_detect.chip_id_value[0], sar_detect.chip_id_value[1]);
	ret = mcu_i2c_rw(bus_num, i2c_address, &reg_add, 1, &buf_temp[0], 1);
	if (ret < 0)
		hwlog_err("In %s! i2c read reg fail!\n", __func__);

	if ((buf_temp[0] == sar_detect.chip_id_value[0]) || (buf_temp[0] == sar_detect.chip_id_value[1])) {
		detect_result = 1;
		hwlog_info("sar_sensor_detect_succ 0x%x\n", buf_temp[0]);
	} else {
		detect_result = 0;
		hwlog_info("sar_sensor_detect_fail 0x%x\n", buf_temp[0]);
	}
	return detect_result;
}
static ssize_t show_sar_sensor_detect(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int final_detect_result;
	int semtech_detect_result = 0;
	int adi_detect_result = 0;
	int cypress_detect_result = 0;
	int abov_detect_result = 0;
	int aw9610_detect_result = 0;

	if (semtech_sar_detect.detect_flag == 1) {
		hwlog_info("semtech_sar_detect\n");
		semtech_detect_result = sar_sensor_i2c_detect(semtech_sar_detect);
	}
	if (adi_sar_detect.detect_flag == 1) {
		hwlog_info("adi_sar_detect\n");
		adi_detect_result = sar_sensor_i2c_detect(adi_sar_detect);
	}
	if (cypress_sar_detect.detect_flag == 1) {
		hwlog_info("cypress_sar_detect\n");
		cypress_detect_result = sar_sensor_i2c_detect(cypress_sar_detect);
	}
	if (g_abov_sar_detect.detect_flag == 1) {
		hwlog_info("abov_sar_detect\n");
		abov_detect_result = sar_sensor_i2c_detect(g_abov_sar_detect);
	}
	if (aw9610_sar_detect.detect_flag == 1) {
		hwlog_info("aw9610_sar_detect\n");
		aw9610_detect_result = sar_sensor_i2c_detect(aw9610_sar_detect);
	}

	final_detect_result = semtech_detect_result || adi_detect_result ||
		cypress_detect_result || abov_detect_result || aw9610_detect_result;
	hwlog_info("In %s! final_detect_result=%d, semtech_detect_result=%d, adi_detect_result=%d , cypress_detect_result=%d\n",
		__func__, final_detect_result, semtech_detect_result, adi_detect_result, cypress_detect_result);
	hwlog_info("abov_detect_result = %d\n", abov_detect_result);
	hwlog_info("aw9610_detect_result = %d\n", aw9610_detect_result);

	return snprintf(buf, MAX_STR_SIZE, "%d\n", final_detect_result);
}
static ssize_t store_fingersense_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	int *enabled = get_fingersense_enabled();

	if (strict_strtoul(buf, TO_DECIMALISM, &val)) {
		hwlog_err("%s: finger sense enable val%lu invalid", __func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: finger sense enable val %ld\n", __func__, val);
	if ((val != 0) && (val != 1)) {
		hwlog_err("%s:finger sense set enable fail, invalid val\n", __func__);
		return size;
	}

	if (*enabled == val) {
		hwlog_info("%s:finger sense already at seted state\n", __func__);
		return size;
	}

	hwlog_info("%s: finger sense set enable\n", __func__);
	ret = fingersense_enable(val);
	if (ret) {
		hwlog_err("%s: finger sense enable fail: %d\n", __func__, ret);
		return size;
	}
	*enabled = val;
	return size;
}

static ssize_t show_fingersense_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	int *enabled = get_fingersense_enabled();

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"%d\n", *enabled);
}

static ssize_t show_fingersense_data_ready(struct device *dev, struct device_attribute *attr, char *buf)
{
	bool *ready = get_fingersense_data_ready();

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n", *ready);
}

static ssize_t show_fingersense_latch_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	int size;
	int *enabled = get_fingersense_enabled();
	bool *ready = get_fingersense_data_ready();
	s16 *data = get_fingersense_data();

	size = ((sizeof(*data) * FINGERSENSE_DATA_NSAMPLES) < MAX_STR_SIZE) ?
		(sizeof(*data) * FINGERSENSE_DATA_NSAMPLES) : MAX_STR_SIZE;
	if ((!(*ready)) || (!(*enabled))) {
		hwlog_err("%s:fingersense zaxix not ready %d or not enable%d\n",
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
	if ((vibrator_shake == 1) || (HALL_COVERD & hall_value)) {
		hwlog_err("coverd, vibrator shaking, not send fingersense req data cmd to mcu\n");
		return -1;
	}
#endif

	if (!(*enabled)) {
		hwlog_err("%s: finger sense not enable,  dont req data\n", __func__);
		return size;
	}

	*ready = false;
	ret = fingersense_commu(sub_cmd, sub_cmd, RESP, true);
	if (ret) {
		hwlog_err("%s: finger sense send requst data failed\n", __func__);
		return size;
	}
	return size;
}

static ssize_t show_ois_ctrl(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "%d\n", sensor_status.gyro_ois_status);
}

static ssize_t store_ois_ctrl(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int source;
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	source = simple_strtol(buf, NULL, TO_DECIMALISM);
	if (source == 1) {
		ret = ois_commu(TAG_OIS, CMD_CMN_OPEN_REQ, source, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: ois open gyro fail\n", __func__);
			return size;
		}
		/* delay 200 ms */
		ret = ois_commu(TAG_OIS, CMD_CMN_INTERVAL_REQ, 200, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: set delay fail\n", __func__);
			return size;
		}

		ret = ois_commu(TAG_GYRO, SUB_CMD_GYRO_OIS_REQ, source, RESP, true);
		if (ret) {
			hwlog_err("%s: ois enable fail\n", __func__);
			return size;
		}
		hwlog_info("%s:ois enable succsess\n", __func__);
	} else {
		ret = ois_commu(TAG_GYRO, SUB_CMD_GYRO_OIS_REQ, source, RESP, true);
		if (ret) {
			hwlog_err("%s:ois close fail\n", __func__);
			return size;
		}

		ret = ois_commu(TAG_OIS, CMD_CMN_CLOSE_REQ, source, NO_RESP, false);
		if (ret) {
			hwlog_err("%s: ois close gyro fail\n", __func__);
			return size;
		}
		hwlog_info("%s:ois close succsess\n", __func__);
	}
	return size;
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
static DEVICE_ATTR(cap_prox_calibrate_type, 0440, show_cap_prox_calibrate_type, NULL); /* only for magnetic */
static DEVICE_ATTR(cap_prox_calibrate_order, 0440, show_cap_prox_calibrate_order, NULL);
static DEVICE_ATTR(sar_sensor_detect, 0440, show_sar_sensor_detect, NULL);

static DEVICE_ATTR(set_fingersense_enable, 0660, show_fingersense_enable, store_fingersense_enable);
static DEVICE_ATTR(fingersense_data_ready, 0440, show_fingersense_data_ready, NULL);
static DEVICE_ATTR(fingersense_latch_data, 0440, show_fingersense_latch_data, NULL);
static DEVICE_ATTR(fingersense_req_data, 0220, NULL, store_fingersense_req_data);
static DEVICE_ATTR(rpc_motion_req, 0660, show_rpc_motion_req, store_rpc_motion_req);
static DEVICE_ATTR(rpc_sar_service_req, 0660, NULL, store_rpc_sar_service_req);
static DEVICE_ATTR(ois_ctrl, 0660, show_ois_ctrl, store_ois_ctrl);

static ssize_t show_acc_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(ACC),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(acc_sensorlist_info, 0664, show_acc_sensorlist_info, NULL);

static ssize_t show_mag_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(MAG),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(mag_sensorlist_info, 0664, show_mag_sensorlist_info, NULL);

static ssize_t show_gyro_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(GYRO),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}

static ssize_t show_gyro_position_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: gyro_position is %d\n", __func__, gyro_position);
	return snprintf(buf, MAX_STR_SIZE, "%d\n", gyro_position);
}

static DEVICE_ATTR(gyro_sensorlist_info, 0664, show_gyro_sensorlist_info, NULL);
static DEVICE_ATTR(gyro_position_info, 0660, show_gyro_position_info, NULL);

unsigned long ungyro_timestamp_offset = 0;
#define MAX_TIMEOFFSET_VAL 100000000
static ssize_t store_ungyro_time_offset(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	long val = 0;

	if (strict_strtol(buf, 10, &val)) { /* change to 10 type */
		hwlog_err("%s: read uni val %lu invalid", __func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: set ungyro timestamp offset val %ld\n", __func__, val);
	if ((val < -MAX_TIMEOFFSET_VAL) || (val > MAX_TIMEOFFSET_VAL)) {
		hwlog_err("%s:set ungyro timestamp offset fail, invalid val\n", __func__);
		return size;
	}

	ungyro_timestamp_offset = val;
	return size;
}
static ssize_t show_ungyro_time_offset(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: unigyro_time_offset is %d\n", __func__, ungyro_timestamp_offset);
	memcpy(buf, &ungyro_timestamp_offset, sizeof(ungyro_timestamp_offset));
	return sizeof(ungyro_timestamp_offset);
}
static DEVICE_ATTR(ungyro_time_offset, 0664, show_ungyro_time_offset, store_ungyro_time_offset);

unsigned long unacc_timestamp_offset = 0;
static ssize_t store_unacc_time_offset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	long val = 0;

	if (strict_strtol(buf, 10, &val)) { /* change to 10 type */
		hwlog_err("%s: read unacc_timestamp_offset val%lu invalid", __func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: set acc timestamp offset val %ld\n", __func__, val);
	if ((val < -MAX_TIMEOFFSET_VAL) || (val > MAX_TIMEOFFSET_VAL)) {
		hwlog_err("%s:set acc timestamp offset fail, invalid val\n", __func__);
		return size;
	}

	unacc_timestamp_offset = val;
	return size;
}
static ssize_t show_unacc_time_offset(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s: acc_time_offset is %d\n", __func__, unacc_timestamp_offset);
	memcpy(buf, &unacc_timestamp_offset, sizeof(unacc_timestamp_offset));
	return sizeof(unacc_timestamp_offset);
}
static DEVICE_ATTR(unacc_time_offset, 0664, show_unacc_time_offset, store_unacc_time_offset);
static ssize_t show_ps_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(PS),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(ps_sensorlist_info, 0664, show_ps_sensorlist_info, NULL);

static ssize_t show_als_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(ALS),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(als_sensorlist_info, 0664, show_als_sensorlist_info, NULL);

static ssize_t calibrate_threshold_from_mag_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = mag_threshold_for_als_calibrate;

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static DEVICE_ATTR(calibrate_threshold_from_mag, 0664, calibrate_threshold_from_mag_show, NULL);

static ssize_t show_airpress_sensorlist_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(AIRPRESS),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}
static DEVICE_ATTR(airpress_sensorlist_info, 0664, show_airpress_sensorlist_info, NULL);

static ssize_t show_als_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct als_device_info *dev_info = NULL;

	dev_info = als_get_device_info(TAG_ALS);
	if (!dev_info)
		return -1;

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"als OFFSET:%u  %u  %u  %u  %u  %u\n",
		dev_info->als_offset[0], dev_info->als_offset[1],
		dev_info->als_offset[2], dev_info->als_offset[3],
		dev_info->als_offset[4], dev_info->als_offset[5]);
}
static DEVICE_ATTR(als_offset_data, 0444, show_als_offset_data, NULL);

static ssize_t show_ps_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "ps OFFSET:%d  %d  %d\n",
		ps_sensor_offset[0], ps_sensor_offset[1], ps_sensor_offset[2]);
}
static DEVICE_ATTR(ps_offset_data, 0444, show_ps_offset_data, NULL);

static ssize_t show_acc_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE,
		"acc offset:%d  %d  %d\nsensitivity:%d  %d  %d\nxis_angle:%d  %d  %d  %d  %d  %d  %d  %d  %d\n",
		gsensor_offset[0], gsensor_offset[1], gsensor_offset[2], gsensor_offset[3], gsensor_offset[4],
		gsensor_offset[5], gsensor_offset[6], gsensor_offset[7], gsensor_offset[8], gsensor_offset[9],
		gsensor_offset[10], gsensor_offset[11], gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);
}
static DEVICE_ATTR(acc_offset_data, 0444, show_acc_offset_data, NULL);

static ssize_t show_gyro_offset_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, MAX_STR_SIZE, "gyro offset:%d  %d  %d\nsensitivity:%d  %d  %d\nxis_angle:%d  %d  %d  %d  %d  %d  %d  %d  %d\nuser calibrated offset:%d  %d  %d\n",
		gyro_sensor_offset[0], gyro_sensor_offset[1], gyro_sensor_offset[2], gyro_sensor_offset[3],
		gyro_sensor_offset[4], gyro_sensor_offset[5], gyro_sensor_offset[6], gyro_sensor_offset[7],
		gyro_sensor_offset[8], gyro_sensor_offset[9], gyro_sensor_offset[10], gyro_sensor_offset[11],
		gyro_sensor_offset[12], gyro_sensor_offset[13], gyro_sensor_offset[14], gyro_sensor_offset[15],
		gyro_sensor_offset[16], gyro_sensor_offset[17]);
}
static DEVICE_ATTR(gyro_offset_data, 0444, show_gyro_offset_data, NULL);

static ssize_t attr_airpress_calibrate_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	struct read_info read_pkg;

	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if ((val != 1) && (val != 0)) {
		hwlog_err("airpress calibrate para error, val=%d\n", val);
		return count;
	}

	/* send calibrate command */
	read_pkg = send_airpress_calibrate_cmd(TAG_PRESSURE, val, &airpress_calibration_res);

	return count;
}

static ssize_t attr_airpress_calibrate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = airpress_calibration_res;

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static DEVICE_ATTR(airpress_calibrate, 0660, attr_airpress_calibrate_show, attr_airpress_calibrate_write);

static ssize_t attr_cap_prox_data_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	int ret;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	memset(&spkt, 0, sizeof(spkt));
	if (!buf) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	if (val < MIN_CAP_PROX_MODE || val > MAZ_CAP_PROX_MODE) {
		hwlog_err("cap_prox_data_mode error, val=%d\n", val);
		return -1;
	}

	spkt.subcmd = SUB_CMD_SET_DATA_MODE;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
	memcpy(spkt.para, &val, sizeof(val));

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send tag %d sar mode to mcu fail,ret=%d\n", pkg_ap.tag, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("send sar mode to mcu fail\n");
			ret = -1;
		} else {
			hwlog_info("send sar mode to mcu succes\n");
			ret = count;
		}
	}

	return ret;
}

static DEVICE_ATTR(cap_prox_data_mode, 0220, NULL, attr_cap_prox_data_mode_store);

static int abov_str_to_hex(const char *string, uint8_t *buf, int len)
{
	uint8_t high, low;
	int index;
	int i = 0;

	for (index = 0; index < len; index += ABOV_CHECKSUM_DATA_LENGTH) {
		high = string[index];
		low = string[index + 1];

		if (high >= '0' && high <= '9') {
			high = high - '0';
		} else if (high >= 'A' && high <= 'F') {
			high = high - 'A' + ABOV_STR_TO_HEX;
		} else if (high >= 'a' && high <= 'f') {
			high = high - 'a' + ABOV_STR_TO_HEX;
		} else {
			hwlog_info("high = %x\n", high);
			return -1;
		}
		if (low >= '0' && low <= '9') {
			low = low - '0';
		} else if (low >= 'A' && low <= 'F') {
			low = low - 'A' + ABOV_STR_TO_HEX;
		} else if (low >= 'a' && low <= 'f') {
			low = low - 'a' + ABOV_STR_TO_HEX;
		} else {
			hwlog_info("low = %x\n", low);
			return -1;
		}
		buf[i++] = (high << ABOV_STR_TO_HEX_HIGH_ORDER) | low;
	}
	return 0;
}

static ssize_t attr_cap_prox_abov_data_write(
	struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	uint8_t data[ABOV_WRITE_DATA_LENGTH] = { 0 };
	int ret;

	memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap));
	memset_s(&pkg_mcu, sizeof(pkg_mcu), 0, sizeof(pkg_mcu));
	memset_s(&spkt, sizeof(spkt), 0, sizeof(spkt));

	if (!buf) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	/* 2 letters one byte, include '\0' */
	if (count != (ABOV_WRITE_DATA_LENGTH * 2 + 1)) {
		hwlog_err("%s:wrong input,count is %d\n", __func__, (int)count);
		return -EINVAL;
	}
	/* data length is 72 */
	ret = abov_str_to_hex(buf, data, ABOV_WRITE_DATA_LENGTH * 2);
	if (ret) {
		hwlog_err("%s: str2hex failed\n", __func__);
		return count;
	}

	spkt.subcmd = SUB_CMD_SET_ADD_DATA_REQ;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(data) + SUBCMD_LEN;
	if (memcpy_s(spkt.para, sizeof(data), data, sizeof(data)) != EOK) {
		hwlog_err("%s memcpy data error\n", __func__);
		return -EINVAL;
	}

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send tag %d sar mode to mcu fail,ret=%d\n",
			pkg_ap.tag, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s send abov data to mcu fail\n", __func__);
			ret = -1;
		} else {
			hwlog_info("%s send abov data to mcu succes\n", __func__);
			ret = count;
		}
	}
	return ret;
}

static DEVICE_ATTR(abov_data_write, 0220, NULL, attr_cap_prox_abov_data_write);

static int32_t abov_bootloader_verify_result;
static ssize_t attr_abov_bootloader_verify(
	struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	uint8_t data[ABOV_CHECKSUM_DATA_LENGTH] = {0};
	int ret;

	memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap));
	memset_s(&pkg_mcu, sizeof(pkg_mcu), 0, sizeof(pkg_mcu));
	memset_s(&spkt, sizeof(spkt), 0, sizeof(spkt));

	if (!buf) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	/* 2 letters one byte, include '\0' */
	if (count != (ABOV_CHECKSUM_DATA_LENGTH * 2 + 1)) {
		hwlog_err("%s wrong input, count is %d\n", __func__, (int)count);
		return -EINVAL;
	}
	/* 2 letters one byte */
	ret = abov_str_to_hex(buf, data, ABOV_CHECKSUM_DATA_LENGTH * 2);
	if (ret) {
		hwlog_err("%s: str2hex failed\n", __func__);
		return count;
	}

	spkt.subcmd = SUB_CMD_ADDITIONAL_INFO;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(data) + SUBCMD_LEN;
	if (memcpy_s(spkt.para, sizeof(data), data, sizeof(data)) != EOK) {
		hwlog_err("%s memcpy data error\n", __func__);
		return -EINVAL;
	}

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret != 0) {
		hwlog_err("%s send tag %d sar mode to mcu fail,ret=%d\n",
			__func__, pkg_ap.tag, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s send abov data to mcu return fail\n", __func__);
			abov_bootloader_verify_result = -1;
			ret = -1;
		} else {
			abov_bootloader_verify_result = 0;
			hwlog_info("%s mcu return succes,result is %d\n",
				__func__, abov_bootloader_verify_result);
			ret = count;
		}
	}
	return ret;
}

static ssize_t attr_abov_bootloader_verify_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf)
		return -1;
	return scnprintf(buf, MAX_STR_SIZE, "%d\n", abov_bootloader_verify_result);
}

static DEVICE_ATTR(abov_bootloader_verify, 0660, attr_abov_bootloader_verify_result_show, attr_abov_bootloader_verify);

static ssize_t attr_abov_bootloader_enter(
	struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_parameter_req_t spkt;
	struct pkt_header *shd = (struct pkt_header *)&spkt;
	unsigned long val = 0;
	int ret;

	memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap));
	memset_s(&pkg_mcu, sizeof(pkg_mcu), 0, sizeof(pkg_mcu));
	memset_s(&spkt, sizeof(spkt), 0, sizeof(spkt));

	if (!buf) {
		hwlog_err("%s: buf is NULL\n", __func__);
		return -EINVAL;
	}
	if (kstrtoul(buf, ABOV_STR_TO_HEX, &val)) {
		hwlog_err("%s: strtoul fail\n", __func__);
		return -EINVAL;
	}
	if (val != ABOV_ENTER_BOOTLOADER_CMD) {
		hwlog_err("%s: wrong value input\n", __func__);
		return -EINVAL;
	}

	spkt.subcmd = SUB_CMD_SELFCALI_REQ;
	pkg_ap.tag = TAG_CAP_PROX;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
	if (memcpy_s(spkt.para, sizeof(val), &val, sizeof(val)) != EOK) {
		hwlog_err("%s memcpy data error\n", __func__);
		return -EINVAL;
	}

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("%s send tag %d sar mode to mcu fail,ret=%d\n",
			__func__, pkg_ap.tag, ret);
		ret = -1;
	} else {
		if (pkg_mcu.errno != 0) {
			hwlog_err("%s send abov data to mcu return fail\n",
				__func__);
			ret = -1;
		} else {
			if (memcpy_s(&abov_device_id, sizeof(abov_device_id),
				pkg_mcu.data, sizeof(abov_device_id)) != EOK)
				hwlog_err("%s memcpy abov_device_id error\n",
					__func__);
			hwlog_info("%s mcu return succes,device is %d\n",
				__func__, abov_device_id);
			ret = count;
		}
	}
	return ret;
}

static ssize_t attr_abov_bootloader_enter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf)
		return -1;
	return scnprintf(buf, MAX_STR_SIZE, "%x\n", abov_device_id);
}


static DEVICE_ATTR(abov_bootloader_enter, 0660, attr_abov_bootloader_enter_show, attr_abov_bootloader_enter);

static ssize_t abov_reg_dump_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t reg = 0;
	uint32_t val = 0;
	uint8_t buf_temp[DEBUG_DATA_LENGTH] = { 0 };
	int res;

	if (count < 5) { /* input length must > 5 */
		hwlog_err("%s, invalid input", __func__);
		return -EINVAL;
	}
	hwlog_info("%s,buf 0x%x,0x%x,0x%x\n", __func__, buf[0], buf[1], buf[ABOV_CHECKSUM_DATA_LENGTH]);

	if (sscanf(buf, "%x,%x", &reg, &val) == ABOV_CHECKSUM_DATA_LENGTH) {
		hwlog_info("%s,reg = 0x%02x, val = 0x%02x\n", __func__,
			*(uint8_t *)&reg, *(uint8_t *)&val);
		buf_temp[0] = reg;
		buf_temp[1] = val;
		res = mcu_i2c_rw(sar_pdata.cfg.bus_num,
			(uint8_t)(sar_pdata.cfg.i2c_address),
			buf_temp, ABOV_CHECKSUM_DATA_LENGTH, NULL, 0);
		if (res < 0)
			hwlog_err("%s: i2c write fail\n", __func__);
	}
	return count;
}


static ssize_t abov_reg_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint8_t reg_value = 0;
	uint8_t i;
	int ret = 0;
	int res;

	if (!dev || !attr || !buf)
		return -1;

	for (i = 0; i < 0x26; i++) { /* converter channel selection register */
		res = mcu_i2c_rw(sar_pdata.cfg.bus_num,
			(uint8_t)(sar_pdata.cfg.i2c_address),
			&i, 1, &reg_value, 1);
		if (res < 0)
			hwlog_err("%s: i2c read fail, i=%x\n", __func__, i);
		ret += scnprintf(buf + ret, PAGE_SIZE, "(0x%02x)=0x%02x\n",
			i, reg_value);
	}
	for (i = 0x46; i < 0x4c; i++) { /* flash status register) */
		res = mcu_i2c_rw(sar_pdata.cfg.bus_num,
			(uint8_t)(sar_pdata.cfg.i2c_address),
			&i, 1, &reg_value, 1);
		if (res < 0)
			hwlog_err("%s: i2c read fail, i=%x\n", __func__, i);
		ret += scnprintf(buf + ret, PAGE_SIZE, "(0x%02x)=0x%02x\n",
			i, reg_value);
	}

	for (i = 0x80; i < 0x8C; i++) { /* basic interval timer register */
		res = mcu_i2c_rw(sar_pdata.cfg.bus_num,
			(uint8_t)(sar_pdata.cfg.i2c_address),
			&i, 1, &reg_value, 1);
		if (res < 0)
			hwlog_err("%s: i2c read fail, i=%x\n", __func__, i);
		ret += scnprintf(buf + ret, PAGE_SIZE, "(0x%02x)=0x%02x\n",
			i, reg_value);
	}
	return ret;
}

static DEVICE_ATTR(abov_reg_rw, 0660, abov_reg_dump_show, abov_reg_dump_store);
static struct attribute *acc_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_acc_sensorlist_info.attr,
	&dev_attr_acc_offset_data.attr,
	&dev_attr_unacc_time_offset.attr,
	NULL,
};

static const struct attribute_group acc_sensor_attrs_grp = {
	.attrs = acc_sensor_attrs,
};

static int sleeve_test_ps_prepare(int ps_config)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_PS;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &ps_config;
	pkg_ap.wr_len = sizeof(ps_config);
	ret = write_customize_cmd(&pkg_ap,  &pkg_mcu, true);
	if (ret) {
		hwlog_err("send sleeve_test ps config cmd to mcu fail,ret=%d\n", ret);
		return ret;
	}
	if (pkg_mcu.errno != 0)
		hwlog_err("sleeve_test ps config fail%d\n", pkg_mcu.errno);
	return ret;
}

static ssize_t store_sleeve_test_prepare(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;

	if (strict_strtoul(buf, TO_DECIMALISM, &val)) {
		hwlog_err("%s: sleeve_test enable val invalid", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: sleeve_test enable val %ld\n", __func__, val);
	if ((val != 0) && (val != 1)) {
		hwlog_err("%s:sleeve_test set enable fail, invalid val\n", __func__);
		return -EINVAL;
	}

	if (sleeve_test_enabled == val) {
		hwlog_info("%s:sleeve_test already at seted state\n", __func__);
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
			hwlog_info("sleeve_test threshhold %d, phone_color%d\n",
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

int send_als_ud_data_to_mcu(int tag, uint32_t subcmd, const void *data,
	int length, bool is_recovery)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu;
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;

	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = length + SUBCMD_LEN;
	memcpy(cpkt.para, data, length);

	if (is_recovery)
		return write_customize_cmd(&pkg_ap, NULL, false);

	ret = write_customize_cmd(&pkg_ap,  &pkg_mcu, true);
	if (ret) {
		hwlog_err("send tag %d calibrate data to mcu fail,ret=%d\n", tag, ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("send tag %d  calibrate data fail,err=%d\n", tag, pkg_mcu.errno);
		return -1;
	}

	return 0;
}

void save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level)
{
	uint64_t timestamp;
	struct timespec64 ts;
	uint32_t para[3];

	if (als_data.is_bllevel_supported) {
		get_monotonic_boottime64(&ts);
		timestamp = ((unsigned long long)(ts.tv_sec * NSEC_PER_SEC) +
			(unsigned long long)ts.tv_nsec) / ONE_MILLION;
		para[0] = (uint32_t)mipi_level;
		para[1] = (uint32_t)bl_level;
		para[2] = (uint32_t)timestamp;
		send_als_ud_data_to_mcu(TAG_ALS, SUB_CMD_UPDATE_BL_LEVEL,
			(const void *)&(para), sizeof(para), false);
	}
}

inline void send_lcd_freq_to_sensorhub(uint32_t lcd_freq)
{
	return;
}

static DEVICE_ATTR(sleeve_test_prepare, 0220, NULL, store_sleeve_test_prepare);
static DEVICE_ATTR(sleeve_test_threshhold, 0440, show_sleeve_test_threshhold, NULL);

static struct attribute *ps_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_sleeve_test_prepare.attr,
	&dev_attr_ps_sensorlist_info.attr,
	&dev_attr_ps_offset_data.attr,
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
	NULL,
};

static const struct attribute_group als_sensor_attrs_grp = {
	.attrs = als_sensor_attrs,
};

static struct attribute *mag_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_timeout.attr,
	&dev_attr_calibrate_method.attr,
	&dev_attr_mag_sensorlist_info.attr,
	NULL,
};

static const struct attribute_group mag_sensor_attrs_grp = {
	.attrs = mag_sensor_attrs,
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
	NULL,
};

static const struct attribute_group gyro_sensor_attrs_grp = {
	.attrs = gyro_sensor_attrs,
};

static struct attribute *airpress_sensor_attrs[] = {
	&dev_attr_get_data.attr,
	&dev_attr_read_airpress.attr,
	&dev_attr_set_calidata.attr,
	&dev_attr_airpress_calibrate.attr,
	&dev_attr_airpress_sensorlist_info.attr,
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
	&dev_attr_abov_data_write.attr,
	&dev_attr_abov_bootloader_verify.attr,
	&dev_attr_abov_bootloader_enter.attr,
	&dev_attr_abov_reg_rw.attr,
	NULL,
};
static const struct attribute_group cap_prox_sensor_attrs_grp = {
	.attrs = cap_prox_sensor_attrs,
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
};

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

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		atomic_set(&sensor_status.get_data[tag].reading, 0);
		init_completion(&sensor_status.get_data[tag].complete);
		/* 1 means fail , 2 set the length of buf */
		memcpy(sensor_status.selftest_result[tag], "1", 2);
	}
}

/* device_create->device_register->device_add->device_add_attrs->device_add_attributes */
static int sensors_register(void)
{
	int i;

	for (i = 0; i < sizeof(all_sensors) / sizeof(all_sensors[0]); ++i) {
		all_sensors[i].dev = device_create(sensors_class, NULL, 0, &all_sensors[i], all_sensors[i].name);
		if (!all_sensors[i].dev) {
			hwlog_err("[%s] Failed", __func__);
			return -1;
		}
		if (all_sensors[i].attrs_group) {
			if (sysfs_create_group(&all_sensors[i].dev->kobj, all_sensors[i].attrs_group))
				hwlog_err("create files failed in %s\n", __func__);
		}
	}
	return 0;
}

static void sensors_unregister(void)
{
	device_destroy(sensors_class, 0);
}

static int sensors_feima_init(void)
{
	if (is_sensorhub_disabled())
		return -1;
	sensors_class = class_create(THIS_MODULE, "sensors");
	if (IS_ERR(sensors_class))
		return PTR_ERR(sensors_class);

	sensors_class->dev_groups = sensors_attr_groups;
	sensors_register();
	init_sensors_get_data();
	create_debug_files();
	return 0;
}

static void sensors_feima_exit(void)
{
	sensors_unregister();
	class_destroy(sensors_class);
}

late_initcall_sync(sensors_feima_init);
module_exit(sensors_feima_exit);

MODULE_AUTHOR("SensorHub <smartphone@huawei.com>");
MODULE_DESCRIPTION("SensorHub feima driver");
MODULE_LICENSE("GPL");
