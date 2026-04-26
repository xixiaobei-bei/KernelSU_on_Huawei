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

#include "acc_sysfs.h"
#include "airpress_sysfs.h"
#include "als_sysfs.h"
#include "cap_prox_sysfs.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "gyro_sysfs.h"
#include "mag_sysfs.h"
#include "motion_channel.h"
#include "protocol.h"
#include "ps_sysfs.h"
#include "sensor_sysfs.h"
#include "therm_sysfs.h"

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

static void attr_set_selftest_write_cmd(enum obj_tag tag,
	char *selftest_result, struct write_info *pkg_ap)
{
	int err;
	struct read_info pkg_mcu;

	if (memset_s(&pkg_mcu, sizeof(pkg_mcu), 0, sizeof(pkg_mcu)) != EOK)
		hwlog_err("%s pkg_mcu memset_s error\n", __func__);
	err = write_customize_cmd(pkg_ap, &pkg_mcu, true);
	if (err) {
		hwlog_err("send %d selftest cmd to mcu fail, ret %d\n",
			tag, err);
		if (memcpy_s(selftest_result, SELFTEST_RESULT_MAXLEN,
			"0", strlen("0") + 1) != EOK)
			hwlog_err("%s memcpy_s error\n", __func__);
		return;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("%d selftest fail\n", tag);
		if (memcpy_s(selftest_result, SELFTEST_RESULT_MAXLEN,
			"0", strlen("0") + 1) != EOK)
			hwlog_err("%s memcpy_s error\n", __func__);
	} else {
		hwlog_info("send %d selftest success, data len %d\n",
			tag, pkg_mcu.data_length);
		if (memcpy_s(selftest_result, SELFTEST_RESULT_MAXLEN,
			"1", strlen("1") + 1) != EOK)
			hwlog_err("%s memcpy_s error\n", __func__);
	}
}

#ifdef CONFIG_HUAWEI_DSM
ssize_t attr_set_selftest(enum obj_tag tag, char *selftest_result,
	const char *buf, size_t size)
{
	unsigned long val = 0;
	struct write_info pkg_ap;
	uint32_t subcmd;

	if (memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap)) != EOK)
		hwlog_err("%s pkg_ap memset_s error\n", __func__);
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	/* val 1 express enable */
	if (val == 1) {
		pkg_ap.tag = tag;
		pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
		subcmd = SUB_CMD_SELFTEST_REQ;
		pkg_ap.wr_buf = &subcmd;
		pkg_ap.wr_len = SUBCMD_LEN;
		attr_set_selftest_write_cmd(tag, selftest_result, &pkg_ap);
	}

	return size;
}
#else
ssize_t attr_set_selftest(enum obj_tag tag, unsigned int cmd,
	char *selftest_result, const char *buf, size_t size)
{
	unsigned long val = 0;
	struct write_info pkg_ap;

	if (memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap)) != EOK)
		hwlog_err("%s pkg_ap memset_s error\n", __func__);
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	/* val 1 express enable */
	if (val == 1) {
		pkg_ap.tag = tag;
		pkg_ap.cmd = cmd;
		pkg_ap.wr_buf = NULL;
		pkg_ap.wr_len = 0;
		attr_set_selftest_write_cmd(tag, selftest_result, &pkg_ap);
	}

	return size;
}
#endif

ssize_t attr_set_enable(enum obj_tag tag,
	enum obj_cmd cmd1, enum obj_cmd cmd2, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	char *str = NULL;

	if (memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap)) != EOK)
		hwlog_err("%s pkg_ap memset_s error\n", __func__);
	if (memset_s(&pkg_mcu, sizeof(pkg_mcu), 0, sizeof(pkg_mcu)) != EOK)
		hwlog_err("%s pkg_mcu memset_s error\n", __func__);
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	pkg_ap.tag = tag;
	/* val 1 express enable, 0 express disable */
	if (val == 1) {
		str = "enable";
		pkg_ap.cmd = cmd1;
	} else {
		str = "disable";
		pkg_ap.cmd = cmd2;
	}
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send tag %d %s cmd to mcu fail, ret = %d\n",
			tag, str, ret);
		return size;
	}
	if (pkg_mcu.errno != 0)
		hwlog_err("set tag %d %s fail\n", tag, str);
	else
		hwlog_info("set tag %d %s success\n", tag, str);
	if ((val == 1) && (tag == TAG_ACCEL)) {
		struct acc_device_info *dev_info = NULL;

		dev_info = acc_get_device_info(tag);
		if (dev_info != NULL &&
			dev_info->acc_opened_before_calibrate == 0) {
			dev_info->acc_opened_before_calibrate = 1;
			hwlog_info("open cmd during calib not close after calib\n");
		}
	}
	return size;
}

ssize_t attr_set_delay(enum obj_tag tag, const char *buf, size_t size)
{
	unsigned long val = 0;
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	pkt_cmn_interval_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;

	if (memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap)) != EOK)
		hwlog_err("%s pkg_ap memset_s error\n", __func__);
	if (memset_s(&pkg_mcu, sizeof(pkg_mcu), 0, sizeof(pkg_mcu)) != EOK)
		hwlog_err("%s pkg_mcu memset_s error\n", __func__);
	if (memset_s(&cpkt, sizeof(cpkt), 0, sizeof(cpkt)) != EOK)
		hwlog_err("%s cpkt memset_s error\n", __func__);
	if (sensor_status.opened[tag] == 0) {
		hwlog_err("send tag %d delay must be opend first\n", tag);
		return -EINVAL;
	}
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	/* delay time min 10 ms, less than 1000 ms */
	if (val >= 10 && val < 1000) {
		pkg_ap.tag = tag;
		pkg_ap.cmd = CMD_CMN_INTERVAL_REQ;
		cpkt.param.period = val;
		/* hd[1] express the address of cpkt.param */
		pkg_ap.wr_buf = &hd[1];
		pkg_ap.wr_len = sizeof(cpkt.param);
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
		if (ret) {
			hwlog_err("send tag %d delay cmd to mcu fail,ret=%d\n",
				tag, ret);
			return size;
		}
		if (pkg_mcu.errno != 0)
			hwlog_err("set tag %d delay fail\n", tag);
		else
			hwlog_info("set tag %d delay %ld ms success\n",
				tag, val);
	}

	return size;
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
	ret = inputhub_sensor_enable(TAG_CONNECTIVITY, enable);
	if (ret) {
		hwlog_err("send GSENSOR GATHER enable cmd to mcu fail,ret=%d\n",
			ret);
		return -EINVAL;
	}

	if (enable == 1) {
		ret = inputhub_sensor_setdelay(TAG_CONNECTIVITY, &delay_param);
		if (ret) {
			hwlog_err("send GSENSOR GATHER set delay cmd to mcu fail,ret=%d\n",
				ret);
			return -EINVAL;
		}
	}
	is_gsensor_gather_enable = enable;
	hwlog_info("GSENSOR GATHER set to state %lu success\n", enable);

	return size;
}

static int pdr_write_customize_cmd(struct write_info *pkg_ap)
{
	int ret;
	struct read_info pkg_mcu = { 0 };

	ret = write_customize_cmd(pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("pdr write cmd to mcu fail, ret=%d\n", ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("set pdr delay fail\n");
		return -1;
	}

	return 0;
}

ssize_t attr_set_pdr_delay(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned long val = 0;
	int start_update_flag;
	int precise;
	int interval;
	struct write_info pkg_ap = { 0 };
	pdr_ioctl_t pkg_ioctl;

	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	/* val define: xyyzzz x:0 start 1 update yy:precise zzz:interval */
	if (val == 0)
		val = 1010; /* 1010: start, precise 1, interval 10 */
	start_update_flag = (val / 100000); /* 100000: to start or update */
	/* 1000: precise to unit */
	precise = ((val / 1000) %
		100); /* 100: just precise */
	interval = val % 1000; /* 1000: to interval */

	hwlog_info("val = %lu start_update_flag = %d precise = %d interval= %d\n",
		val, start_update_flag, precise, interval);
	if (precise == 0)
		precise = 1;
	if (interval == 0)
		interval = 240; /* interval 240 */

	pkg_ap.tag = TAG_PDR;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	if (val >= 1000) { /* 1000: not only interval */
		pkg_ioctl.sub_cmd = (start_update_flag == 0) ?
			SUB_CMD_FLP_PDR_START_REQ : SUB_CMD_FLP_PDR_UPDATE_REQ;
		/* multiply 1000 */
		pkg_ioctl.start_param.report_interval = interval * 1000;
		/* multiply 1000 */
		pkg_ioctl.start_param.report_precise = precise * 1000;
		pkg_ioctl.start_param.report_count = interval / precise;
		pkg_ioctl.start_param.report_times = 0;
		pkg_ap.wr_buf = &pkg_ioctl;
		pkg_ap.wr_len = sizeof(pkg_ioctl);
	} else if (val == 2) { /* 2: stop command */
		pkg_ioctl.sub_cmd = SUB_CMD_FLP_PDR_STOP_REQ;
		pkg_ioctl.stop_param = 30000; /* stop param 30000 */
		pkg_ap.wr_buf = &pkg_ioctl;
		pkg_ap.wr_len = sizeof(pkg_ioctl);
	}
	hwlog_info(" pkg_ioctl.sub_cmd = %d\n ", pkg_ioctl.sub_cmd);
	if (pdr_write_customize_cmd(&pkg_ap) == 0)
		hwlog_info("set pdr delay (%ld)success\n", val);

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
 * second status 0-4
 */
#define MOTION_DT_STUP_LENGTH 5
#define RADIX_16 16
ssize_t attr_set_dt_motion_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	char dt_motion_value[MOTION_DT_STUP_LENGTH] = { 0 };
	int i;
	unsigned long source;

	source = simple_strtoul(buf, NULL, RADIX_16);
	hwlog_err("%s buf %s, source %lu size %lu\n", __func__, buf, source, size);

	for (i = 0; i < MOTION_DT_STUP_LENGTH - 1; i++) {
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
	stop_auto_accel = simple_strtoul(buf, NULL, 16);
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

ssize_t attr_set_sensor_motion_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int i;
	unsigned long source;
	struct sensor_data event;

	source = simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	if (source) { /* 1: landscape */
		hwlog_err("%s landscape\n", __func__);
		event.type = TAG_ACCEL;
		event.length = 12; /* 12: int x 3 */
		event.value[0] = 1000; /* 1000: x axis val */
		event.value[1] = 0;
		event.value[2] = 0;
	} else { /* 0: portial */
		hwlog_err("%s portial\n", __func__);
		event.type = TAG_ACCEL;
		event.length = 12; /* 12: int x 3 */
		event.value[0] = 0;
		event.value[1] = 1000; /* 1000: y axis val */
		event.value[2] = 0;
	}

	for (i = 0; i < 20; i++) { /* 20 times */
		msleep(100); /* sleep 100 ms */
		report_sensor_event(TAG_ACCEL, event.value, event.length);
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

ssize_t attr_set_dt_hall_sensor_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_data event;
	unsigned long source;

	source = simple_strtoul(buf, NULL, TO_HEXADECIMAL);

	hwlog_err("%s buf %s, source %lu\n", __func__, buf, source);
	event.type = TAG_HALL;
	event.length = 4; /* 4: int size */
	event.value[0] = (int)source;

	inputhub_route_write(ROUTE_SHB_PORT, (char *)&event, 8); /* 8 bytes */
	return size;
}

ssize_t show_sensor_in_board_status_sysfs(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s is %s\n", __func__, sensor_in_board_status);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
		sensor_in_board_status);
}

ssize_t show_sensor_read_temperature(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		get_temperature_data);
}

ssize_t show_dump_sensor_status(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int tag;

	hwlog_info("-------------------------------------\n");
	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		/* ps and step counter need always on, just skip */
		if (unlikely((tag == TAG_PS) || (tag == TAG_STEP_COUNTER) ||
			(tag == TAG_MAGN_BRACKET)))
			continue;

		hwlog_info(" %s\t %s\t %d\n", obj_tag_str[tag],
			sensor_status.opened[tag] ? "open" : "close",
			sensor_status.delay[tag]);
	}
	hwlog_info("-------------------------------------\n");
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"please check log %d\n", get_temperature_data);
}

ssize_t store_set_data_type(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct g_sensor_platform_data *pf_data = NULL;
	int32_t set_data_type[2] = { 0 };
	const int32_t *set_type = NULL;
	int ret;
	int32_t err_no = 0;

	set_type = (const int32_t *)buf;
	if (size == sizeof(set_data_type)) {
		set_data_type[0] = *set_type;
		set_data_type[1] = *(set_type + 1);
	} else {
		hwlog_err("%s:size %lu is not equal to 8\n",  __func__, size);
		return -1;
	}

	/* set_data_type[0]: 1.ACC 2.GYRO 35.ACC1 36.GYRO1 */
	/* set_data_type[1]: 1.raw_data 2.cali_data 4.nor_data */
	hwlog_info("%s: data type tag is %d, type is %d\n",
		__func__, set_data_type[0], set_data_type[1]);

	if (set_data_type[0] == TAG_ACCEL || set_data_type[0] == TAG_GYRO) {
		pf_data = acc_get_platform_data(TAG_ACCEL);
		if (pf_data == NULL)
			return -1;
		if (pf_data->calibrate_way == 0) {
			hwlog_info("%s: tag %d now is self_calibreate\n",
				__func__, set_data_type[0]);
			return size;
		}
	} else if (set_data_type[0] == TAG_ACC1 ||
		set_data_type[0] == TAG_GYRO1) {
		pf_data = acc_get_platform_data(TAG_ACC1);
		if (pf_data == NULL)
			return -1;
		if (pf_data->calibrate_way == 0) {
			hwlog_info("%s: tag %d now is self_calibreate\n",
				__func__, set_data_type[0]);
			return size;
		}
	} else {
		hwlog_err("%s: set sensor tag if fail\n", __func__);
		return -1;
	}

	if (set_data_type[1] < 1 || set_data_type[1] > 4) { /* max 4 */
		hwlog_err("%s:set data type is fail, invalid val\n", __func__);
		return -1;
	}

	ret = send_subcmd_data_to_mcu_lock(set_data_type[0],
		SUB_CMD_SET_DATA_TYPE_REQ, (const void *)&set_data_type[1],
		sizeof(set_data_type[1]), &err_no);
	if (ret != 0 || err_no != 0)
		hwlog_err("%s fail tag=%d type=%d ret=%d err_no=%d\n",
			__func__, set_data_type[0], set_data_type[1], ret,
			err_no);
	return size;
}

ssize_t show_get_sensors_id(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	switch (tag) {
	case TAG_ACCEL:
		hwlog_info("show_get_sensors_id acc\n");
		return attr_get_acc_sensor_id(dev, attr, buf);
	case TAG_GYRO:
		return attr_get_gyro_sensor_id(dev, attr, buf);
	case TAG_ALS:
	case TAG_ALS1:
	case TAG_ALS2:
		return attr_get_als_sensor_id(dev, attr, buf);
	case TAG_MAG:
		return attr_get_mag_sensor_id(dev, attr, buf);
	case TAG_CAP_PROX:
		return attr_get_cap_sensor_id(dev, attr, buf);
	case TAG_THERMOMETER:
		return attr_get_therm_sensor_id(dev, attr, buf);
	default:
		hwlog_err("%s tag %d get_sensors_id not implement\n", __func__, tag);
		break;
	}

	return 0;
}

ssize_t sensors_calibrate_show(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	switch (tag) {
	case TAG_ACCEL:
	case TAG_ACC1:
		return sensors_calibrate_show_acc(tag, dev, attr, buf);
	case TAG_PS:
		return sensors_calibrate_show_ps(tag, dev, attr, buf);
	case TAG_ALS:
	case TAG_ALS1:
	case TAG_ALS2:
		return sensors_calibrate_show_als(tag, dev, attr, buf);
	case TAG_GYRO:
	case TAG_GYRO1:
		return sensors_calibrate_show_gyro(tag, dev, attr, buf);
	case TAG_PRESSURE:
		return show_airpress_set_calidata(dev, attr, buf);
	case TAG_HANDPRESS:
		return sensors_calibrate_show_handpress(tag, dev, attr, buf);
	case TAG_CAP_PROX:
	case TAG_CAP_PROX1:
		return sensors_calibrate_show_cap_prox(tag, dev, attr, buf);
	case TAG_THERMOMETER:
		return sensors_calibrate_show_therm(tag, dev, attr, buf);
	default:
		hwlog_err("tag %d calibrate not implement in %s\n", tag, __func__);
		break;
	}

	return 0;
}

ssize_t sensors_calibrate_store(int tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	switch (tag) {
	case TAG_ACCEL:
		return attr_acc_calibrate_write(dev, attr, buf, count);
	case TAG_PS:
		return attr_ps_calibrate_write(dev, attr, buf, count);
	case TAG_ALS:
	case TAG_ALS1:
	case TAG_ALS2:
		return attr_als_calibrate_write_by_tag(tag, dev, attr, buf,
			count);
	case TAG_GYRO:
		return attr_gyro_calibrate_write(dev, attr, buf, count);
	case TAG_PRESSURE:
		return store_airpress_set_calidata(dev, attr, buf, count);
	case TAG_HANDPRESS:
		return attr_handpress_calibrate_write(dev, attr, buf, count);
	case TAG_CAP_PROX:
		return attr_cap_prox_calibrate_write(dev, attr, buf, count);
	case TAG_ACC1:
		return attr_acc1_calibrate_write(dev, attr, buf, count);
	case TAG_GYRO1:
		return attr_gyro1_calibrate_write(dev, attr, buf, count);
	case TAG_CAP_PROX1:
		return attr_cap_prox1_calibrate_write(dev, attr, buf, count);
	case TAG_THERMOMETER:
		if (calibrate_processing == 0)
			calibrate_processing = 1;
		else
			return count;
		return attr_thermometer_calibrate_write(dev, attr, buf, count);
	default:
		hwlog_err("tag %d calibrate not implement in %s\n", tag, __func__);
		break;
	}

	return count;
}

ssize_t show_hifi_supported(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		get_hifi_supported());
}

static enum detect_state sensor_detect_flag = DET_FAIL;
ssize_t show_sensor_detect(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_detect_manager *sensor_manager = get_sensor_manager();
	int i;
	int detect_result = 0;

	for (i = 0; i < SENSOR_MAX; i++) {
		detect_result = sensor_manager[i].detect_result;
		if (detect_result == sensor_detect_flag)
			if (snprintf_s(&buf[i * MAX_SENSOR_NAME_LENGTH],
				MAX_SENSOR_NAME_LENGTH,
				MAX_SENSOR_NAME_LENGTH - 1, "%s ",
				sensor_manager[i].sensor_name_str) < 0)
				hwlog_err("%s error i=%d\n", __func__, i);
	}

	return MAX_STR_SIZE;
}

ssize_t store_sensor_detect(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int flag;

	flag = simple_strtol(buf, NULL, TO_DECIMALISM);
	sensor_detect_flag = flag ? DET_SUCC : DET_FAIL;
	hwlog_info("sensor detect value %d\n", (int)sensor_detect_flag);

	return size;
}

