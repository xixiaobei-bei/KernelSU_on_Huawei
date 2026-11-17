/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "acc_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <securec.h>

#include "acc_sysfs.h"
#include "contexthub_route.h"

#define MAX_TIMEOFFSET_VAL 100000000

struct acc_ioctl_t {
	uint32_t sub_cmd;
	uint8_t filter_flag;
};

int send_acc_filter_flag(unsigned long value)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	struct acc_ioctl_t pkg_ioctl;
	hwlog_info("%s: value %lu\n", __func__, value);

	pkg_ap.tag = TAG_ACCEL;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ioctl.sub_cmd = SUB_CMD_ACC_FILTER_REQ;
	pkg_ioctl.filter_flag = value;
	pkg_ap.wr_buf = &pkg_ioctl;
	pkg_ap.wr_len = sizeof(pkg_ioctl);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send acc filter flag %d to mcu fail, ret=%d\n", value, ret);
		return ret;
	}
	return ret;
}

ssize_t show_acc_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(ACC),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}

static unsigned long unacc_timestamp_offset;
ssize_t store_unacc_time_offset(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	long val = 0;

	if (!dev || !attr || !buf)
		return -EINVAL;

	if (strict_strtol(buf, 10, &val)) { /* change to 10 type */
		hwlog_err("%s: read unacc_timestamp_offset val %lu invalid",
			__func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: set acc timestamp offset val %ld\n", __func__, val);
	if ((val < -MAX_TIMEOFFSET_VAL) || (val > MAX_TIMEOFFSET_VAL)) {
		hwlog_err("%s:set acc timestamp offset fail, invalid val\n",
			__func__);
		return size;
	}

	unacc_timestamp_offset = val;
	return size;
}

ssize_t show_unacc_time_offset(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf)
		return -EINVAL;

	hwlog_info("%s: acc_time_offset is %lu\n",
		__FUNCTION__, unacc_timestamp_offset);
	if (memcpy_s(buf, MAX_STR_SIZE, &unacc_timestamp_offset,
		sizeof(unacc_timestamp_offset)) != EOK)
		return -1;
	return sizeof(unacc_timestamp_offset);
}

ssize_t show_acc_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"acc offset:%d %d %d\nsensitivity:%d %d %d\nxis_angle:%d %d %d %d %d %d %d %d %d\n",
		gsensor_offset[0], gsensor_offset[1], gsensor_offset[2],
		gsensor_offset[3], gsensor_offset[4], gsensor_offset[5],
		gsensor_offset[6], gsensor_offset[7], gsensor_offset[8],
		gsensor_offset[9], gsensor_offset[10], gsensor_offset[11],
		gsensor_offset[12], gsensor_offset[13], gsensor_offset[14]);
}

ssize_t show_acc1_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"acc offset:%d %d %d\n",
		gsensor1_offset[0], gsensor1_offset[1], gsensor1_offset[2]);
}
