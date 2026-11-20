/*
 * camerafs.c
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 *
 * camera utile class driver
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include "cam_log.h"
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include "securec.h"

// Sensor MIPI frequency gear for fore and rear, default: 0
static int g_fore_frequency_gear;
static int g_rear_frequency_gear;

struct camerafs_class {
	struct class *classptr;
	struct device *p_device;
};

struct camerafs_ois_class {
	struct class *classptr;
	struct device *p_device;
};

static struct camerafs_ois_class g_camerafs_ois;

static struct camerafs_class g_camerafs;

static dev_t g_devnum;
static dev_t g_osi_devnum;

#define CAMERAFS_NODE "node"
#define CAMERAFS_OIS_NODE "ois"
#define CAMERAFS_ID_MAX 3
#define CAMERAFS_ID_MIN 0

wait_queue_head_t g_ois_que;
static int g_ois_check;
#define OIS_TEST_TIMEOUT ((HZ) * 8)
#ifdef DEBUG_HISI_CAMERA
static int g_ois_done;
static int g_cross_width = -1;
static int g_cross_height = -1;
static int g_ic_num = -1;
#endif
spinlock_t pix_lock = __SPIN_LOCK_UNLOCKED("camerafs");

#define LDO_NUM_MAX 6
#define LDO_RUN_COUNT 20
#define LDO_NAME_LEN 32
#define LDO_DROP_CURRENT_COUNT 2
enum {
	REAR_POS = 0,
	FRONT_POS,
	CAM_POS_MAX,
};
struct cam_ldo {
	int ldo_num;
	int ldo_channel[LDO_NUM_MAX];
	int ldo_current[LDO_NUM_MAX];
	int ldo_threshold[LDO_NUM_MAX];
	char ldo_name[LDO_NUM_MAX][LDO_NAME_LEN];
};
static struct cam_ldo g_camerafs_ldo[CAM_POS_MAX];
static int g_rt_ldo_detect_pos = REAR_POS;

static struct mutex g_ldo_lock;

static ssize_t rear_sensor_freq_gear_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;

	rc = sscanf_s(buf, "%d", &g_rear_frequency_gear);
	if (rc != 1) {
		g_rear_frequency_gear = 0; // set default value(0)
		cam_err("%s store g_rear_frequency_gear error, set default value 0",
			__func__);
		return -1;
	}

	if ((g_rear_frequency_gear < 0) || (g_rear_frequency_gear > 1))
		g_rear_frequency_gear = 0;

	return count;
}

static ssize_t rear_sensor_freq_gear_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;

	cam_info("%s buf = %s", __func__, buf);
	rc = scnprintf(buf, PAGE_SIZE, "%d\n", g_rear_frequency_gear);
	return rc;
}

static ssize_t fore_sensor_freq_gear_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;

	rc = sscanf_s(buf, "%d", &g_fore_frequency_gear);
	if (rc != 1) {
		g_fore_frequency_gear = 0; // set default value 0
		cam_err("%s store g_fore_frequency_gear error, set default value 0",
			__func__);
		return -1;
	}

	if ((g_fore_frequency_gear < 0) || (g_fore_frequency_gear > 1))
		g_fore_frequency_gear = 0;

	return count;
}

static ssize_t fore_sensor_freq_gear_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;

	cam_info("%s buf = %s", __func__, buf);
	rc = scnprintf(buf, PAGE_SIZE, "%d\n", g_fore_frequency_gear);
	return rc;
}

static struct device_attribute g_fore_sensor_frequency_ctrl =
	__ATTR(fore_frequency_node, 0660,
	fore_sensor_freq_gear_show,
	fore_sensor_freq_gear_store);
static struct device_attribute g_rear_sensor_frequency_ctrl =
	__ATTR(rear_frequency_node, 0660,
	rear_sensor_freq_gear_show,
	rear_sensor_freq_gear_store);

static int g_thermal_meter[CAMERAFS_ID_MAX];
static ssize_t hw_sensor_thermal_meter_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int cam_id = -1;
	int offset;

	offset = sscanf_s(buf, "%d", &cam_id);
	if (offset > 0 && cam_id < CAMERAFS_ID_MAX && cam_id >= CAMERAFS_ID_MIN)
		offset = sscanf_s(buf + offset, "%d", &g_thermal_meter[cam_id]);

	return count;
}
static ssize_t hw_sensor_thermal_meter_show0(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%d", g_thermal_meter[0]);
	return ret;
}
static ssize_t hw_sensor_thermal_meter_show1(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%d",  g_thermal_meter[1]);
	return ret;
}
static ssize_t hw_sensor_thermal_meter_show2(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%d",  g_thermal_meter[2]);
	return ret;
}

static struct device_attribute g_sensor_thermal_meter0 =
	__ATTR(thermal_meter0, 0664,
	hw_sensor_thermal_meter_show0,
	hw_sensor_thermal_meter_store);
static struct device_attribute g_sensor_thermal_meter1 =
	__ATTR(thermal_meter1, 0664,
	hw_sensor_thermal_meter_show1,
	hw_sensor_thermal_meter_store);
static struct device_attribute g_sensor_thermal_meter2 =
	__ATTR(thermal_meter2, 0664,
	hw_sensor_thermal_meter_show2,
	hw_sensor_thermal_meter_store);

int register_camerafs_ois_attr(struct device_attribute *attr);
#ifdef DEBUG_HISI_CAMERA
static ssize_t hw_ois_aging_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int done_flag;

	if (sscanf_s(buf, "%d", &done_flag) <= 0) {
		cam_info("write data done_flag error");
		return -1;
	}
	cam_info("%s: done_flag = %d", __func__, done_flag);
	g_ois_done = done_flag;
	wake_up_interruptible(&g_ois_que);
	return count;
}
#endif
#ifdef DEBUG_HISI_CAMERA
static ssize_t hw_ois_aging_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = -1;

	cam_info("Enter: %s", __func__);
	g_ois_done = 0;
	// wait for start command
	msleep(50);
	ret = wait_event_interruptible_timeout(g_ois_que, g_ois_done != 0,
		OIS_TEST_TIMEOUT);
	if (ret <= 0)
		cam_warn("%s: wait ois signal timeout", __func__);

	ret = scnprintf(buf, PAGE_SIZE, "%d", g_ois_done);

	return ret;
}
#endif
#ifdef DEBUG_HISI_CAMERA
static struct device_attribute g_hw_ois_aging =
	__ATTR(ois_aging, 0664, hw_ois_aging_show, hw_ois_aging_store);
#endif
static ssize_t hw_ois_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int done_flag;

	if (sscanf_s(buf, "%d", &done_flag) <= 0) {
		cam_info("%s: write data done_flag error", __func__);
		return -1;
	}
	cam_info("%s: done_flag = %d", __func__, done_flag);
	g_ois_check = done_flag;
	wake_up_interruptible(&g_ois_que);
	return count;
}

static ssize_t hw_ois_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	cam_info("Enter: %s", __func__);
	g_ois_check = 0;
	// wait for start command
	msleep(50);
	ret = wait_event_interruptible_timeout(g_ois_que, g_ois_check != 0,
		OIS_TEST_TIMEOUT);
	if (ret <= 0)
		cam_warn("%s: wait ois signal timeout", __func__);

	ret = scnprintf(buf, PAGE_SIZE, "%d", g_ois_check);

	return ret;
}

static struct device_attribute g_hw_ois_check =
	__ATTR(g_ois_check, 0664, hw_ois_check_show, hw_ois_check_store);

// add for ois mmi test
#ifdef DEBUG_HISI_CAMERA
static ssize_t hw_ois_test_mmi_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = -1;

	cam_info("Enter: %s", __func__);
	spin_lock(&pix_lock);
	ret = scnprintf(buf, PAGE_SIZE, "%d,%d\n",
		g_cross_width, g_cross_height);
	g_cross_width = -1;
	g_cross_height = -1;
	spin_unlock(&pix_lock);

	return ret;
}
#endif
#ifdef DEBUG_HISI_CAMERA
static ssize_t hw_ois_test_mmi_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int width, height;

	spin_lock(&pix_lock);

	if (sscanf_s(buf, "%d%d", &width, &height) <= 0) {
		cam_info("%s: write data width height error", __func__);
		spin_unlock(&pix_lock);
		return -1;
	}

	g_cross_width = width;
	g_cross_height = height;
	spin_unlock(&pix_lock);
	cam_info("Enter: %s %d, %d", __func__, g_cross_width, g_cross_height);

	return count;
}
#endif
#ifdef DEBUG_HISI_CAMERA
static struct device_attribute g_hw_ois_pixel =
	__ATTR(ois_pixel, 0664, hw_ois_test_mmi_show, hw_ois_test_mmi_store);
#endif
#ifdef DEBUG_HISI_CAMERA
static ssize_t hw_ois_ic_num_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = -1;

	cam_info("Enter: %s", __func__);
	spin_lock(&pix_lock);
	ret = scnprintf(buf, PAGE_SIZE, "%d\n", g_ic_num);
	spin_unlock(&pix_lock);

	return ret;
}
#endif
#ifdef DEBUG_HISI_CAMERA
static ssize_t hw_ois_ic_num_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int icnum;

	spin_lock(&pix_lock);

	if (sscanf_s(buf, "%d", &icnum) <= 0) {
		cam_info("%s: write data icnum error", __func__);
		spin_unlock(&pix_lock);
		return -1;
	}

	g_ic_num = icnum;
	spin_unlock(&pix_lock);
	cam_info("Enter: %s %d", __func__, g_ic_num);

	return count;
}
#endif
#ifdef DEBUG_HISI_CAMERA
static struct device_attribute g_hw_ois_icnum =
	__ATTR(ois_icnum, 0664, hw_ois_ic_num_show, hw_ois_ic_num_store);
#endif

extern int hisi_adc_get_current(int adc_channel);

static ssize_t hw_cam_ldo_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cam_ldo *p_ldo = NULL;
	int buflen;
	int ret;

	cam_info("Enter : %s", __func__);
	mutex_lock(&g_ldo_lock);
	if (!buf) {
		cam_err("%s, buf is NULL\n", __func__);
		mutex_unlock(&g_ldo_lock);
		return -1;
	}
	buflen = (int)sizeof(struct cam_ldo);
	if (g_rt_ldo_detect_pos == REAR_POS)
		p_ldo = &(g_camerafs_ldo[REAR_POS]);
	else
		p_ldo = &(g_camerafs_ldo[FRONT_POS]);

	ret = memcpy_s((struct cam_ldo *)buf, buflen, p_ldo, buflen);
	if (ret != 0) {
		cam_err("%s camera ldo copy fail\n", __func__);
		mutex_unlock(&g_ldo_lock);
		return -1;
	}
	mutex_unlock(&g_ldo_lock);
	cam_info("Exit : %s\n", __func__);
	return buflen;
}

static ssize_t hw_cam_ldo_detect_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int i, j;
	int sum_cur;
	int cur_val;
	int drop_count = LDO_DROP_CURRENT_COUNT;
	int min_cur;
	int max_cur;
	struct cam_ldo *p_ldo = NULL;

	mutex_lock(&g_ldo_lock);
	if (!buf) {
		cam_err("%s, buf is NULL\n", __func__);
		mutex_unlock(&g_ldo_lock);
		return -1;
	}
	cam_info("Enter: %s\n", __func__);
	if (buf[0] == '0') {
		g_rt_ldo_detect_pos = REAR_POS;
		p_ldo = &(g_camerafs_ldo[REAR_POS]);
	} else if (buf[0] == '1') {
		g_rt_ldo_detect_pos = FRONT_POS;
		p_ldo = &(g_camerafs_ldo[FRONT_POS]);
	} else {
		cam_err("%s, buf param is error\n", __func__);
		mutex_unlock(&g_ldo_lock);
		return -1;
	}
	for (i = 0; i < p_ldo->ldo_num; i++) {
		sum_cur = 0;
		for (j = 0; j < LDO_RUN_COUNT; j++) {
			cur_val = hisi_adc_get_current(p_ldo->ldo_channel[i]);
			if (cur_val < 0) {
				cam_err("%s ldo read data error cur_val=%d\n",
					__func__, cur_val);
				sum_cur = -1;
				break;
			}
			if (j == 0) {
				min_cur = cur_val;
				max_cur = cur_val;
			} else {
				if (min_cur > cur_val)
					min_cur = cur_val;

				if (max_cur < cur_val)
					max_cur = cur_val;
			}
			sum_cur = sum_cur + cur_val;

			msleep(5);
		}
		if (sum_cur == -1) {
			p_ldo->ldo_current[i] = sum_cur;
		} else {
			p_ldo->ldo_current[i] = (sum_cur - min_cur - max_cur) /
				(LDO_RUN_COUNT - drop_count);
		}
	}
	mutex_unlock(&g_ldo_lock);
	cam_info("Exit: %s\n", __func__);
	return count;
}

static struct device_attribute g_hw_cam_ldo_detect =
	__ATTR(struct cam_ldo, 0660, hw_cam_ldo_detect_show, hw_cam_ldo_detect_store);

int register_camerafs_attr(struct device_attribute *attr);

static int hw_rt_get_ldo_data(void)
{
	struct cam_ldo *p_ldo = NULL;
	const char *pldoname = NULL;
	int ret;

	mutex_init(&g_ldo_lock);
	ret = memset_s(g_camerafs_ldo, sizeof(struct cam_ldo) * CAM_POS_MAX,
		0, sizeof(struct cam_ldo) * CAM_POS_MAX);
	if (ret != 0)
		cam_err("%s memset_s return fail\n", __func__);

	if (runmode_is_factory()) {
		struct device_node *of_node = NULL;
		int i;

		of_node = of_find_node_by_path("/huawei,camera_ldo");
		if (!of_node)
			return -1;

		p_ldo = &(g_camerafs_ldo[REAR_POS]);
		p_ldo->ldo_num = of_property_count_elems_of_size(of_node,
			"rear-ldo-channel", sizeof(u32));
		if (p_ldo->ldo_num > 0) {
			ret = of_property_read_u32_array(of_node,
				"rear-ldo-channel",
				(u32 *)&p_ldo->ldo_channel, p_ldo->ldo_num);
			if (ret < 0) {
				cam_err("%s failed %d\n", __func__, __LINE__);
				return ret;
			}
			ret = of_property_read_u32_array(of_node,
				"rear-ldo-threshold",
				(u32 *)&p_ldo->ldo_threshold, p_ldo->ldo_num);
			if (ret < 0) {
				cam_err("%s failed %d\n", __func__, __LINE__);
				return ret;
			}
			for (i = 0; i < p_ldo->ldo_num; i++) {
				ret = of_property_read_string_index(of_node,
					"rear-ldo", i, &pldoname);
				if (ret < 0) {
					cam_err("%s failed %d\n",
						__func__, __LINE__);
					return ret;
				}
				ret = strncpy_s(p_ldo->ldo_name[i],
					LDO_NAME_LEN - 1, pldoname,
					strlen(pldoname));
				if (ret != 0)
					cam_err("%s strncpy_s return error\n",
						__func__);
			}
		}

		p_ldo = &(g_camerafs_ldo[FRONT_POS]);
		p_ldo->ldo_num = of_property_count_elems_of_size(of_node,
			"front-ldo-channel", sizeof(u32));
		if (p_ldo->ldo_num > 0) {
			ret = of_property_read_u32_array(of_node,
				"front-ldo-channel",
				(u32 *)&p_ldo->ldo_channel, p_ldo->ldo_num);
			if (ret < 0) {
				cam_err("%s failed %d\n", __func__, __LINE__);
				return ret;
			}
			ret = of_property_read_u32_array(of_node,
				"front-ldo-threshold",
				(u32 *)&p_ldo->ldo_threshold, p_ldo->ldo_num);
			if (ret < 0) {
				cam_err("%s failed %d\n", __func__, __LINE__);
				return ret;
			}
			for (i = 0; i < p_ldo->ldo_num; i++) {
				ret = of_property_read_string_index(of_node,
					"front-ldo", i, &pldoname);
				if (ret < 0) {
					cam_err("%s failed %d\n",
						__func__, __LINE__);
					return ret;
				}
				ret = strncpy_s(p_ldo->ldo_name[i],
					LDO_NAME_LEN - 1,
					pldoname,
					strlen(pldoname));
				if (ret != 0) {
					cam_err("%s copy fail\n", __func__);
					return ret;
				}
			}
		}
		g_rt_ldo_detect_pos = REAR_POS; // default detect rear camera
		ret = register_camerafs_attr(&g_hw_cam_ldo_detect);
	}
	return ret;
}

static int __init camerafs_module_init(void)
{
	int ret;

	g_camerafs.classptr = NULL;
	g_camerafs.p_device = NULL;
	spin_lock_init(&pix_lock);

	ret = alloc_chrdev_region(&g_devnum, 0, 1, CAMERAFS_NODE);
	ret = alloc_chrdev_region(&g_osi_devnum, 0, 1, CAMERAFS_OIS_NODE);
	if (ret) {
		printk("error %s fail to alloc a dev_t\n", __func__);
		return -1;
	}

	g_camerafs.classptr = class_create(THIS_MODULE, "camerafs");
	g_camerafs_ois.classptr = g_camerafs.classptr;
	if (IS_ERR(g_camerafs.classptr)) {
		cam_err("class_create failed %d\n", ret);
		ret = PTR_ERR(g_camerafs.classptr);
		return -1;
	}

	g_camerafs.p_device = device_create(g_camerafs.classptr, NULL, g_devnum,
		NULL, "%s", CAMERAFS_NODE);
	g_camerafs_ois.p_device = device_create(g_camerafs_ois.classptr, NULL,
		g_osi_devnum, NULL, "%s", CAMERAFS_OIS_NODE);

	if (IS_ERR(g_camerafs.p_device)) {
		cam_err("class_device_create failed %s\n", CAMERAFS_NODE);
		ret = PTR_ERR(g_camerafs.p_device);
		return -1;
	}

#ifdef DEBUG_HISI_CAMERA
	register_camerafs_ois_attr(&g_hw_ois_aging);
	register_camerafs_ois_attr(&g_hw_ois_pixel);
	register_camerafs_ois_attr(&g_hw_ois_icnum);
#endif
	register_camerafs_attr(&g_sensor_thermal_meter0);
	register_camerafs_attr(&g_sensor_thermal_meter1);
	register_camerafs_attr(&g_sensor_thermal_meter2);
	register_camerafs_ois_attr(&g_hw_ois_check);

	// for camera mipi adaption
	register_camerafs_attr(&g_rear_sensor_frequency_ctrl);
	register_camerafs_attr(&g_fore_sensor_frequency_ctrl);

	init_waitqueue_head(&g_ois_que);
	hw_rt_get_ldo_data();
	cam_info("%s end", __func__);
	return 0;
}

int register_camerafs_attr(struct device_attribute *attr)
{
	int ret;

	ret = device_create_file(g_camerafs.p_device, attr);
	if (ret < 0) {
		cam_err("camera fs creat dev attr[%s] fail", attr->attr.name);
		return -1;
	}
	cam_info("camera fs creat dev attr[%s] OK", attr->attr.name);
	return 0;
}
EXPORT_SYMBOL(register_camerafs_attr);

int register_camerafs_ois_attr(struct device_attribute *attr)
{
	int ret;

	ret = device_create_file(g_camerafs_ois.p_device, attr);
	if (ret < 0) {
		cam_err("camera oiscreat dev attr[%s] fail", attr->attr.name);
		return -1;
	}
	cam_info("camera ois creat dev attr[%s] OK", attr->attr.name);
	return 0;
}

static void __exit camerafs_module_deinit(void)
{
	device_destroy(g_camerafs.classptr, g_devnum);
	device_destroy(g_camerafs_ois.classptr, g_osi_devnum);
	class_destroy(g_camerafs.classptr);
	unregister_chrdev_region(g_devnum, 1);
	unregister_chrdev_region(g_osi_devnum, 1);
}

module_init(camerafs_module_init);
module_exit(camerafs_module_deinit);
MODULE_AUTHOR("Jiezhou");
MODULE_DESCRIPTION("Camera fs virtul device");
MODULE_LICENSE("GPL");
