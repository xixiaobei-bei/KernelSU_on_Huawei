/*
 * boxid.c
 *
 * boxid driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "boxid.h"

#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/ioctl.h>

#define LOG_TAG "boxid"

#define DEVICEBOX_ID_NAME          "deviceboxID"
#define PAGE_SIZE_MAX              1024
#define NAME_LEN_MAX               32
#define ELECTRIC_LEVEL_LOW         0
#define ELECTRIC_LEVEL_HIGH        1
#define VENDOR                     2

#define SUPPORT_GPIO_NUM_MAX       2
#define GPIO_ID_FAIL               (-1)
#define COEFF_OF_READ_ARRAY_NUM    2
#define BOXID_VDD_MIN_UV           1800000
#define BOXID_VDD_MAX_UV           1800000

enum box_vendor {
	BOX_NAME_DEFAULT = 0,
	BOX_NAME_AAC,
	BOX_NAME_GOER,
	BOX_NAME_GD,
	BOX_NAME_LC,
	BOX_NAME_LX,
	BOX_NAME_XW,

	// not rcv or spk device, from qcom projects
	BOX_NAME_PU,
	BOX_NAME_PD,
	BOX_NAME_NP,

	BOX_NAME_QS,
	BOX_VENDOR_MAX
};

static char *boxtable[BOX_VENDOR_MAX] = {
	"", "AAC", "GOER", "GD ", "LC ", "LX ", "XW ",
	"PU ", "PD ", "NP ", "QS "
};

enum {
	DEVICEBOX_ID_MODE_USE_GPIO = 0x0,
	DEVICEBOX_ID_MODE_USE_ADC = 0x1,
};

enum {
	USE_ONE_GPIO = 1, /* default value */
	USE_TWO_GPIO = 2,
};

enum {
	BOX_ID_DISABLED = 0,
	BOX_ID_ENABLED = 1,
};

enum {
	GPIO_REQ_FAIL = -1,
	GPIO_REQ_SUCCESS = 0,
};

/* define gpio and map name,which is the same as dtsi config */
static const char * const gpio_name_table[BOX_NUM_MAX] = {
	[SPEAKER_ID] = "gpio_speakerID",
	[RECEIVER_ID] = "gpio_receiverID",
	[BOX_3RD_ID] = "gpio_3rdboxID",
	[BOX_4TH_ID] = "gpio_4thboxID",
};

static const char * const box_map_table[BOX_NUM_MAX] = {
	[SPEAKER_ID] = "speaker_map",
	[RECEIVER_ID] = "receiver_map",
	[BOX_3RD_ID] = "box3rd_map",
	[BOX_4TH_ID] = "box4th_map",
};

struct box_info_st {
	int box_enable;
	int gpio_num;
	int gpio_id[SUPPORT_GPIO_NUM_MAX];
	int gpio_req_flag[SUPPORT_GPIO_NUM_MAX];
	int gpio_status[SUPPORT_GPIO_NUM_MAX];
	int box_map[VENDOR][VENDOR];
};

struct out_audio_device_id {
	int check_mode;
	int box_num;
	bool gpio_extra_pull_up_enable;
	struct regulator *pull_up_vdd;
	struct box_info_st box_info[BOX_NUM_MAX];
};

struct out_audio_device_id device_boxid = {-1};

int boxid_read(int id)
{
	struct box_info_st *info = device_boxid.box_info;
	int vendor = BOX_NAME_DEFAULT;
	int status0, status1;

	if (id > BOX_4TH_ID) {
		pr_err("%s: out_id is invalid\n", __func__);
		return -EINVAL;
	}

	if (device_boxid.check_mode == DEVICEBOX_ID_MODE_USE_GPIO) {
		if (info[id].gpio_num == USE_ONE_GPIO) {
			status0 = info[id].gpio_status[0];
			vendor = info[id].box_map[0][status0];
		} else if (info[id].gpio_num == USE_TWO_GPIO) {
			status0 = info[id].gpio_status[0];
			status1 = info[id].gpio_status[1];
			vendor = info[id].box_map[status0][status1];
		}
	}

	pr_info("%s box_id:%d vendor:%d\n", __func__, id, vendor);
	return vendor;
}
EXPORT_SYMBOL(boxid_read);

static inline int boxid_put_user(int id, unsigned int __user *p_user)
{
	unsigned int value = BOX_NAME_DEFAULT;

	if (device_boxid.box_info[id].box_enable)
		value = boxid_read(id);

	return put_user(value, p_user);
}

static long boxid_do_ioctl(struct file *file, unsigned int cmd,
			   void __user *p, int compat_mode)
{
	unsigned int __user *p_user = (unsigned int __user *)p;
	int ret;

	if (p_user == NULL) {
		pr_err("%s: pUser is null\n", __func__);
		return -EINVAL;
	}

	switch (cmd) {
	case DEVICEBOX_ID_GET_SPEAKER:
		ret = boxid_put_user(SPEAKER_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_RECEIVER:
		ret = boxid_put_user(RECEIVER_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_3RDBOX:
		ret = boxid_put_user(BOX_3RD_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_4THBOX:
		ret = boxid_put_user(BOX_4TH_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_BOX_NUM:
		ret = put_user(device_boxid.box_num, p_user);
		break;
	default:
		pr_err("unsupport boxid cmd\n");
		ret = -EINVAL;
		break;
	}
	return (long)ret;
}

static long boxid_ioctl(struct file *file, unsigned int command,
			unsigned long arg)
{
	/*
	 * The use of parameters "0" is to meet format of linux driver,
	 * it has no practical significance.
	 */
	return boxid_do_ioctl(file, command, (void __user *)arg, 0);
}

static long boxid_compat_ioctl(struct file *file, unsigned int command,
			       unsigned long arg)
{
	/*
	 * The use of parameters "0" is to meet format of linux driver,
	 * it has no practical significance.
	 */
	return boxid_do_ioctl(file, command, (void __user *)compat_ptr(arg), 0);
}

static const struct of_device_id boxid_match[] = {
	{ .compatible = "huawei,deviceboxID", },
	{},
};
MODULE_DEVICE_TABLE(of, boxid_match);

static const struct file_operations boxid_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = boxid_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = boxid_compat_ioctl,
#endif
};

static struct miscdevice boxid_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICEBOX_ID_NAME,
	.fops = &boxid_fops,
};

char *get_box_name(int vendor)
{
	if ((vendor > BOX_NAME_DEFAULT) && (vendor < BOX_VENDOR_MAX))
		return boxtable[vendor];

	return "NA ";
}

#ifdef DEVICEBOXID_DEBUG
static ssize_t boxid_info_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct box_info_st *info = device_boxid.box_info;
	int i, j;
	int vendor;

	if (buf == NULL) {
		pr_err("%s: buf is null\n", __func__);
		return 0;
	}

	snprintf(buf, PAGE_SIZE_MAX, "---boxid info begin---\n");

	snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
		 "GPIO_STATUS:\n");
	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		for (j = 0; j < info[i].gpio_num; j++) {
			snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
				 "	  box[%d].gpio[%d].status:%d\n", i, j,
				 info[i].gpio_status[j]);
		}
	}

	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "Box[%d]_MAP:\n", i);
		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "	  -----------------\n");
		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "	  |  %s  |  %s	|\n",
			 get_box_name(info[i].box_map[0][0]),
			 get_box_name(info[i].box_map[0][1]));
		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "	  -----------------\n");
		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "	  |  %s  |  %s	|\n",
			 get_box_name(info[i].box_map[1][0]),
			 get_box_name(info[i].box_map[1][1]));
		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "	  -----------------\n");
	}

	snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf), "Box_NAME:\n");
	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		vendor = boxid_read(i);
		snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			 "	  box[%d]  :  %s\n", i, get_box_name(vendor));
	}

	snprintf(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
		 "---boxid info end---\n");

	return strlen(buf);
}

static DEVICE_ATTR(deviceboxID_info, 0660, boxid_info_show, NULL);

static struct attribute *boxid_attributes[] = {
	&dev_attr_deviceboxID_info.attr,
	NULL,
};

static const struct attribute_group boxid_attr_group = {
	.attrs = boxid_attributes,
};
#endif /* DEVICEBOXID_DEBUG */

static int get_check_mode(struct device_node *node)
{
	const char *mode = NULL;
	int rc;

	/* get check mode */
	rc = of_property_read_string(node, "check_mode", &mode);
	if (rc) {
		pr_info("%s: not find dev_node ,rc=%d\n", __func__, rc);
		return DEVICEBOX_ID_MODE_USE_ADC;
	}
	pr_info("%s: mode: %s\n", __func__, mode);

	if (!strncmp(mode, "gpio", strlen("gpio")))
		return DEVICEBOX_ID_MODE_USE_GPIO;

	return DEVICEBOX_ID_MODE_USE_ADC;
}

static int get_box_num(struct device_node *node)
{
	int box_num = 0;
	int rc;

	/* get box number */
	rc = of_property_read_u32(node, "box_num", &box_num);
	if (rc < 0)
		pr_info("%s: not find dev_node ,rc=%d\n", __func__, rc);
	else
		pr_info("%s: box_num: %d\n", __func__, box_num);

	return box_num;
}

static int get_gpio_status(struct device_node *node, char *propname, int boxid,
			   int index)
{
	struct box_info_st *info = device_boxid.box_info;
	int ret;

	pr_debug("%s, propname=%s, box_index=%d, gpio_index=%d\n", __func__,
		 propname, boxid, index);
	info[boxid].gpio_id[index] = of_get_named_gpio(node, propname, index);
	if (info[boxid].gpio_id[index] < 0) {
		pr_err("%s: box_info[%d].gpio_id[%d] is unvalid\n", __func__,
		       boxid, index);
		return -ENOENT;
	}

	if (!gpio_is_valid(info[boxid].gpio_id[index])) {
		pr_err("%s: box_info[%d].gpio_id[%d] is unvalid\n", __func__,
		       boxid, index);
		return -ENOENT;
	}

	ret = gpio_request(info[boxid].gpio_id[index], "gpio_id_check");
	if (ret) {
		pr_err("%s: boxid[%d] index[%d] gpio_id_check req fail %d\n",
		       __func__, boxid, index, ret);
		return -ENOENT;
	}
	info[boxid].gpio_req_flag[index] = ret;

	/* set gpio to input status */
	ret = gpio_direction_input(info[boxid].gpio_id[index]);
	if (ret) {
		pr_err("%s: set gpio to input status error! ret is %d\n",
		       __func__, ret);
	}

	info[boxid].gpio_status[index] =
	    gpio_get_value_cansleep(info[boxid].gpio_id[index]);

	if (info[boxid].gpio_status[index] < ELECTRIC_LEVEL_LOW ||
	    info[boxid].gpio_status[index] > ELECTRIC_LEVEL_HIGH) {
		/* if get status failed, set default value to avoid overrange */
		info[boxid].gpio_status[index] = ELECTRIC_LEVEL_LOW;
		pr_err("%s:box_info[%d].gpio_status[%d] is invalid\n", __func__,
		       boxid, index);
		return -ENOENT;
	}
	pr_info("%s:read box_info[%d].gpio_status[%d] is %d\n", __func__, boxid,
		index, info[boxid].gpio_status[index]);

	return 0;
}

static bool get_boxid_extra_pull_up_enable(struct device_node *node)
{
	if (of_property_read_bool(node, "boxID_extra_pull_up_enable")) {
		pr_info("%s: boxID extra pull up regulator enabled\n",
			__func__);
		return true;
	}

	pr_info("%s: boxID extra pull up regulator not set\n", __func__);
	return false;
}

static struct regulator *get_boxid_gpio_vdd(struct device *dev)
{
	struct regulator *vdd = NULL;

	vdd = regulator_get(dev, "boxID_pull_up_vdd");
	if (IS_ERR(vdd)) {
		pr_err("%s: boxID regulator get failed\n", __func__);
		return NULL;
	}
	return vdd;
}

static int get_boxid_info(struct device_node *node)
{
	struct box_info_st *info = device_boxid.box_info;
	int temp = BOX_ID_DISABLED;
	int i;

	pr_debug("%s\n", __func__);
	device_boxid.check_mode = get_check_mode(node);
	device_boxid.box_num = get_box_num(node);
	if (device_boxid.box_num > BOX_NUM_MAX) {
		pr_err("%s: box_num is out of range\n", __func__);
		return -ENOENT;
	}

	device_boxid.gpio_extra_pull_up_enable =
	    get_boxid_extra_pull_up_enable(node);

	for (i = 0; i < device_boxid.box_num; i++) {
		if (!of_property_read_u32_index(node, "enable_boxID", i, &temp))
			info[i].box_enable = temp;
		else
			info[i].box_enable = BOX_ID_DISABLED;
		pr_debug("%s, box_info[%d].box_enable=%d\n", __func__, i,
			 info[i].box_enable);
	}

	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable == BOX_ID_ENABLED) {
			if (!of_property_read_u32_index
			    (node, "gpio_num", i, &temp))
				info[i].gpio_num = temp;
			else
				info[i].gpio_num = USE_ONE_GPIO;
		}
		pr_debug("%s, box_info[%d].gpio_num=%d\n", __func__, i,
			 info[i].gpio_num);
	}

	return 0;
}

static int get_boxid_map_priv(struct device_node *node, char *propname, int id)
{
	struct box_info_st *info = device_boxid.box_info;
	int ret = 0;

	pr_debug("%s, propname=%s, box_index=%d, gpio_num=%d\n", __func__,
		 propname, id, info[id].gpio_num);
	if (info[id].gpio_num == USE_ONE_GPIO) {
		int sz = sizeof(info[id].box_map) / sizeof(int) /
		     COEFF_OF_READ_ARRAY_NUM;

		ret = of_property_read_u32_array(node, propname,
			info[id].box_map[0], sz);
		if (ret) {
			pr_err("one gpio box_id_map read err, boxid:%d\n", id);
			ret = -ENOENT;
		} else {
			pr_debug("%s, box_index=%d\n", __func__, id);
		}
	} else if (info[id].gpio_num == USE_TWO_GPIO) {
		int sz = sizeof(info[id].box_map) / sizeof(int);

		ret = of_property_read_u32_array(node, propname,
			info[id].box_map[0], sz);
		if (ret) {
			pr_err("two gpio box_id_map read err, boxid:%d\n", id);
			ret = -ENOENT;
		} else {
			pr_debug("%s, box_index=%d\n", __func__, id);
		}
	} else {
		pr_err("gpio_num out of range,box[%d].gpio_num:%d\n", id,
		       info[id].gpio_num);
		ret = -ENOENT;
	}

	return ret;
}

static int get_boxid_map(struct device_node *node)
{
	char map_name[NAME_LEN_MAX] = {0};
	int ret = 0;
	int i;

	pr_debug("%s\n", __func__);
	for (i = 0; i < device_boxid.box_num; i++) {
		if (device_boxid.box_info[i].box_enable != BOX_ID_ENABLED)
			continue;

		strncpy(map_name, box_map_table[i], NAME_LEN_MAX - 1);
		ret = get_boxid_map_priv(node, map_name, i);
		if (ret)
			return ret;
		memset(map_name, 0, NAME_LEN_MAX);
	}

	return ret;
}

static int get_boxid_gpio_status(struct device_node *node)
{
	char gpio_name[NAME_LEN_MAX] = {0};
	int i, j;
	int ret;

	if (node == NULL) {
		pr_err("%s: node is null\n", __func__);
		return -ENOENT;
	}

	for (i = 0; i < device_boxid.box_num; i++) {
		if (device_boxid.box_info[i].box_enable != BOX_ID_ENABLED)
			continue;

		strncpy(gpio_name, gpio_name_table[i], NAME_LEN_MAX - 1);
		for (j = 0; j < device_boxid.box_info[i].gpio_num; j++) {
			ret = get_gpio_status(node, gpio_name, i, j);
			if (ret != 0)
				return ret;
		}
		memset(gpio_name, 0, NAME_LEN_MAX);
	}

	return 0;
}

static int boxid_regulator_config_on(struct regulator *vdd)
{
	int rc = 0;

	if (regulator_count_voltages(vdd) > 0) {
		rc = regulator_set_voltage(vdd, BOXID_VDD_MIN_UV,
			BOXID_VDD_MAX_UV);
		if (rc) {
			pr_err("%s: vdd set_vtg on failed rc=%d\n",
				__func__, rc);
			regulator_put(vdd);
		}
	}

	return rc;
}

static int boxid_regulator_config_off(struct regulator *vdd)
{
	int rc = 0;

	if (regulator_count_voltages(vdd) > 0) {
		rc = regulator_set_voltage(vdd, 0, BOXID_VDD_MAX_UV);
		if (rc)
			pr_err("%s: vdd set_vtg off failed rc=%d\n",
			     __func__, rc);
	}
	regulator_put(vdd);

	return rc;
}

static int boxid_regulator_config(struct regulator *vdd, bool on)
{
	if (vdd == NULL) {
		pr_err("%s: vdd is null\n", __func__);
		return -EINVAL;
	}

	if (on)
		return boxid_regulator_config_on(vdd);

	return boxid_regulator_config_off(vdd);
}

static int boxid_regulator_set_on(struct regulator *vdd)
{
	int rc = 0;

	if (regulator_count_voltages(vdd) > 0) {
		rc = regulator_enable(vdd);
		if (rc)
			pr_err("%s: vdd enable failed rc=%d\n", __func__, rc);
	}

	return rc;
}

static int boxid_regulator_set_off(struct regulator *vdd)
{
	int rc = 0;

	if (regulator_count_voltages(vdd) > 0) {
		rc = regulator_disable(vdd);
		if (rc)
			pr_err("%s: vdd disable failed rc=%d\n", __func__, rc);
	}
	return rc;
}

static int boxid_regulator_set(struct regulator *vdd, bool on)
{
	if (vdd == NULL) {
		pr_err("%s: vdd is null\n", __func__);
		return -EINVAL;
	}

	if (on)
		return boxid_regulator_set_on(vdd);

	return boxid_regulator_set_off(vdd);
}

static void boxid_gpio_free(void)
{
	struct box_info_st *info = device_boxid.box_info;
	int i, j;

	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		for (j = 0; j < info[i].gpio_num; j++) {
			if (info[i].gpio_req_flag[j] != GPIO_REQ_SUCCESS)
				continue;

			gpio_free(info[i].gpio_id[j]);
			info[i].gpio_id[j] = GPIO_ID_FAIL;
			info[i].gpio_req_flag[j] = GPIO_REQ_FAIL;
		}
	}
}

static int boxid_pinctrl_select_state(struct pinctrl *p, const char *name)
{
	struct pinctrl_state *pinctrl_def = NULL;
	int ret;

	pinctrl_def = pinctrl_lookup_state(p, name);
	if (IS_ERR(pinctrl_def)) {
		pr_err("%s: could not get %s defstate\n", __func__, name);
		return -ENOENT;
	}

	ret = pinctrl_select_state(p, pinctrl_def);
	if (ret) {
		pr_err("%s: could not set pins to %s state\n", __func__, name);
		return -ENOENT;
	}

	return 0;
}

static int boxid_probe_init_fs(struct platform_device *pdev)
{
	int ret;

#ifdef DEVICEBOXID_DEBUG
	/* create sysfs for debug function */
	ret = sysfs_create_group(&pdev->dev.kobj, &boxid_attr_group);
	if (ret < 0) {
		pr_err("%s: failed to register sysfs\n", __func__);
		return -ENOENT;
	}
#endif

	ret = misc_register(&boxid_device);
	if (ret) {
		pr_err("%s: boxid_device register failed", __func__);
		ret = -ENOENT;
		goto err_out;
	}

	return 0;

err_out:
#ifdef DEVICEBOXID_DEBUG
	sysfs_remove_group(&pdev->dev.kobj, &boxid_attr_group);
#endif
	return ret;
}

static int boxid_probe_use_gpio(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct pinctrl *p = NULL;
	int ret;

	p = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(p)) {
		pr_err("%s: could not get pinctrl\n", __func__);
		return -ENOENT;
	}

	ret = boxid_pinctrl_select_state(p, "default");
	if (ret)
		return -ENOENT;

	ret = boxid_regulator_config(device_boxid.pull_up_vdd, true);
	if (ret == 0) {
		ret = boxid_regulator_set(device_boxid.pull_up_vdd, true);
		if (ret) {
			pr_err("%s: failed to enable vdd regulator\n", __func__);
			boxid_regulator_config(device_boxid.pull_up_vdd, false);
			device_boxid.pull_up_vdd = NULL;
		}
	} else {
		device_boxid.pull_up_vdd = NULL;
	}

	ret = get_boxid_gpio_status(node);
	if (ret) {
		pr_err("%s: read gpio_status fail\n", __func__);
		boxid_regulator_set(device_boxid.pull_up_vdd, false);
		boxid_regulator_config(device_boxid.pull_up_vdd, false);
		ret = -ENOENT;
		goto err_get_gpio_status;
	}

	boxid_regulator_set(device_boxid.pull_up_vdd, false);
	boxid_regulator_config(device_boxid.pull_up_vdd, false);

	/* reset gpio to NP status for saving power */
	ret = boxid_pinctrl_select_state(p, "idle");
	if (ret)
		goto err_get_gpio_status;

	ret = boxid_probe_init_fs(pdev);
	if (ret)
		goto err_get_gpio_status;

	return 0;

err_get_gpio_status:
	boxid_gpio_free();
	return ret;
}

static int boxid_probe(struct platform_device *pdev)
{
	const struct of_device_id *match = NULL;
	struct device_node *node = NULL;
	int ret;

	pr_info("%s ++\n", __func__);
	if (pdev == NULL) {
		pr_err("%s: pdev is null\n", __func__);
		return -ENOENT;
	}

	match = of_match_device(boxid_match, &pdev->dev);
	if (!match) {
		pr_err("%s: get boxid device info err\n", __func__);
		return -ENOENT;
	}
	node = pdev->dev.of_node;
	if (node == NULL) {
		pr_err("%s: node is null\n", __func__);
		return -ENOENT;
	}

	/* get boxid info from dts.such as check_mode,gpio_num,box_enable etc */
	ret = get_boxid_info(node);
	if (ret) {
		pr_err("%s: get boxid_info err\n", __func__);
		return -ENOENT;
	}

	device_boxid.pull_up_vdd = device_boxid.gpio_extra_pull_up_enable ?
	    get_boxid_gpio_vdd(&pdev->dev) : NULL;

	/* read boxID map info from dts */
	ret = get_boxid_map(node);
	if (ret) {
		pr_err("%s: get boxID_map info err\n", __func__);
		return -ENOENT;
	}

	if (device_boxid.check_mode == DEVICEBOX_ID_MODE_USE_GPIO) {
		ret = boxid_probe_use_gpio(pdev);
		if (ret < 0) {
			pr_info("%s failed\n", __func__);
			return ret;
		}
	}

	pr_info("%s --\n", __func__);
	return 0;
}

static int boxid_remove(struct platform_device *pdev)
{
	if (pdev == NULL) {
		pr_err("%s: pdev is null\n", __func__);
		return 0;
	}

	if (device_boxid.check_mode == DEVICEBOX_ID_MODE_USE_GPIO) {
		boxid_gpio_free();
#ifdef DEVICEBOXID_DEBUG
		sysfs_remove_group(&pdev->dev.kobj, &boxid_attr_group);
#endif
		misc_deregister(&boxid_device);
	}
	return 0;
}

static struct platform_driver boxid_driver = {
	.driver = {
		.name = DEVICEBOX_ID_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(boxid_match),
	},
	.probe = boxid_probe,
	.remove = boxid_remove,
};

static int __init boxid_init(void)
{
	return platform_driver_register(&boxid_driver);
}

static void __exit boxid_exit(void)
{
	platform_driver_unregister(&boxid_driver);
}

module_init(boxid_init);
module_exit(boxid_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("boxid driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

