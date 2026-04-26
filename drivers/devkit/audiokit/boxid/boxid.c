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
#include "securec.h"

#define LOG_TAG "boxid"

#define DEVICEBOX_ID_NAME          "deviceboxID"
#define PAGE_SIZE_MAX              1024
#define NAME_LEN_MAX               32
#define ELECTRIC_LEVEL_INVALID     (-1)
#define ELECTRIC_LEVEL_LOW         0
#define ELECTRIC_LEVEL_HIGH        1
#define ELECTRIC_LEVEL_TRISTATE    2
#define VENDOR                     3

#define SUPPORT_GPIO_NUM_MAX       2
#define GPIO_ID_FAIL               (-1)
#define BOXID_VDD_MIN_UV           1800000
#define BOXID_VDD_MAX_UV           1800000

#define SPKID_CHANNEL_DEFAULT      12
#define HKADC_MIN_VOL_DEFAULT      900
#define HKADC_MID_VOL_DEFAULT      1500
#define HKADC_MAX_VOL_DEFAULT      1900

enum box_vendor {
	BOX_NAME_DEFAULT = 0,
	BOX_NAME_AAC,
	BOX_NAME_GOER,
	BOX_NAME_GD,
	BOX_NAME_LC,
	BOX_NAME_LX,
	BOX_NAME_XW,
	BOX_NAME_SSI,

	BOX_VENDOR_MAX
};

static char *boxtable[BOX_VENDOR_MAX] = {
	"", "AAC", "GOER", "GD ", "LC ", "LX ", "XW ", "SSI "
};

enum {
	DEVICEBOX_ID_MODE_USE_GPIO = 0x0,
	DEVICEBOX_ID_MODE_USE_ADC = 0x1,
	DEVICEBOX_ID_MODE_USE_HKADC = 0x2,
};

enum {
	USE_NO_GPIO = 0, /* default value */
	USE_ONE_GPIO = 1,
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
	[BOX_5FIF_ID] = "gpio_5fifboxID",
	[BOX_6SIX_ID] = "gpio_6sixboxID",
	[BOX_7SEV_ID] = "gpio_7sevboxID",
	[BOX_8EIG_ID] = "gpio_8eigboxID",
};

static const char * const box_map_table[BOX_NUM_MAX] = {
	[SPEAKER_ID] = "speaker_map",
	[RECEIVER_ID] = "receiver_map",
	[BOX_3RD_ID] = "box3rd_map",
	[BOX_4TH_ID] = "box4th_map",
	[BOX_5FIF_ID] = "box5fif_map",
	[BOX_6SIX_ID] = "box6six_map",
	[BOX_7SEV_ID] = "box7sev_map",
	[BOX_8EIG_ID] = "box8eig_map",
};

struct box_hkadc_info {
	unsigned int hkadc_channel;
	unsigned int hkadc_min_voltage;
	unsigned int hkadc_mid_voltage;
	unsigned int hkadc_max_voltage;
};

struct actual_box_group {
	unsigned int num;
	int *share_spkid;
};

struct boxid_share_info {
	bool share_support;
	unsigned int actual_boxid_num;
	unsigned int *actual_boxid;
	struct actual_box_group box_group[BOX_NUM_MAX];
};

enum consistent_test_result {
	BOX_CONSISTENT_TEST_FAILED = 0,
	BOX_CONSISTENT_TEST_PASS
};

enum {
	BOX_CONSISTENT_NOT_SUPPORT = 0,
	BOX_CONSISTENT_SUPPORT,
};

struct boxs_vendor_test_groups {
	unsigned int num;
	int *boxid;
};

struct box_consistent_test {
	bool test_support;
	unsigned int group_num;
	struct boxs_vendor_test_groups groups[BOX_NUM_MAX];
};

struct box_info_st {
	int box_enable;
	int gpio_num;
	int gpio_id[SUPPORT_GPIO_NUM_MAX];
	int gpio_req_flag[SUPPORT_GPIO_NUM_MAX];
	int gpio_status[SUPPORT_GPIO_NUM_MAX];
	int voltage_status;
	int box_map[VENDOR][VENDOR];
};

struct out_audio_device_id {
	int check_mode;
	int box_num;
	int tristate_mode;
	bool gpio_extra_pull_up_enable;
	bool is_ext_gpio;
	struct regulator *pull_up_vdd;
	struct box_info_st box_info[BOX_NUM_MAX];
	struct box_hkadc_info spkbox_hkadc_info;
	struct box_consistent_test consistent_test;
};

struct out_audio_device_id device_boxid = {-1};

#define boxid_kfree_ops(p) \
do {\
	if (p != NULL) { \
		kfree(p); \
		p = NULL; \
	} \
} while (0)

int boxid_read(int id)
{
	struct box_info_st *info = device_boxid.box_info;
	int vendor = BOX_NAME_DEFAULT;
	int status0, status1;

	if (id > BOX_NUM_MAX) {
		pr_err("%s: out_id:%d is invalid\n", __func__, id);
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

	if (device_boxid.check_mode == DEVICEBOX_ID_MODE_USE_HKADC &&
		id == SPEAKER_ID) {
		status0 = info[id].voltage_status;
		vendor = status0 < 0 ? BOX_NAME_DEFAULT : info[id].box_map[0][status0];
	}

	pr_info("%s box_id:%d vendor:%d\n", __func__, id, vendor);
	return vendor;
}
EXPORT_SYMBOL(boxid_read);

static inline int boxid_put_user(int id, unsigned int __user *p_user)
{
	int value = BOX_NAME_DEFAULT;

	if (device_boxid.box_info[id].box_enable)
		value = boxid_read(id);

	return put_user(value, p_user);
}

static int box_consistent_support_put_user(unsigned int __user *p_user)
{
	unsigned int value = BOX_CONSISTENT_NOT_SUPPORT;

	if (device_boxid.consistent_test.test_support)
		value = BOX_CONSISTENT_SUPPORT;

	return put_user(value, p_user);
}

static int box_consistent_put_user(unsigned int __user *p_user)
{
	unsigned int value;
	unsigned int i;
	unsigned int j;
	int value1;
	int value2;
	unsigned int boxid1;
	unsigned int boxid2;

	struct box_consistent_test *test =
		(struct box_consistent_test *)&(device_boxid.consistent_test);

	struct boxs_vendor_test_groups *group = NULL;
	struct box_info_st *info = device_boxid.box_info;

	if (!test->test_support) {
		pr_err("%s: not support consistent test\n", __func__);
		value = BOX_CONSISTENT_TEST_FAILED;
		return put_user(value, p_user);
	}

	for (i = 0; i < test->group_num; i++) {
		group = (struct boxs_vendor_test_groups *)(&test->groups[i]);
		for (j = 1; j < group->num; j++) {
			boxid1 = group->boxid[j - 1];
			boxid2 = group->boxid[j];
			if (info[boxid1].box_enable != BOX_ID_ENABLED ||
				info[boxid2].box_enable != BOX_ID_ENABLED) {
				pr_err("%s:group[%d] boxid[%d],[%d] disabled\n",
					__func__, i, boxid1, boxid2);
				value = BOX_CONSISTENT_TEST_FAILED;
				return put_user(value, p_user);
			}

			value1 = boxid_read(boxid1);
			value2 = boxid_read(boxid2);
			if (value1 != value2) {
				pr_err("%s:group[%d],[%d:%d],[%d:%d] is diff\n",
					__func__, i, boxid1, value1,
					boxid2, value2);
				value = BOX_CONSISTENT_TEST_FAILED;
				return put_user(value, p_user);
			}
		}
	}

	value = BOX_CONSISTENT_TEST_PASS;
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
	case DEVICEBOX_ID_GET_5THBOX:
		ret = boxid_put_user(BOX_5FIF_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_6THBOX:
		ret = boxid_put_user(BOX_6SIX_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_7THBOX:
		ret = boxid_put_user(BOX_7SEV_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_8THBOX:
		ret = boxid_put_user(BOX_8EIG_ID, p_user);
		break;
	case DEVICEBOX_ID_GET_CONSISTENT_SUPPORT:
		ret = box_consistent_support_put_user(p_user);
		break;
	case DEVICEBOX_ID_GET_CONSISTENT_TEST:
		ret = box_consistent_put_user(p_user);
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
	return boxid_do_ioctl(file, command, (void __user *)(uintptr_t)arg, 0);
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

	if (snprintf_s(buf, PAGE_SIZE_MAX, PAGE_SIZE_MAX - 1,
		"---boxid info begin---\n") < 0)
		pr_err("%s: snprintf_s is failed\n", __func__);

	if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
		PAGE_SIZE_MAX - strlen(buf) - 1, "GPIO_STATUS:\n") < 0)
		pr_err("%s: snprintf_s is failed\n", __func__);

	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		for (j = 0; j < info[i].gpio_num; j++) {
			if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
				PAGE_SIZE_MAX - strlen(buf) - 1,
				"   box[%d].gpio[%d].status:%d\n", i, j,
				 info[i].gpio_status[j]) < 0)
				pr_err("%s: snprintf_s is failed\n", __func__);
		}
	}

	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		if (i > BOX_4TH_ID)
			continue;

		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "Box[%d]_MAP:\n", i) < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "   -----------------\n") < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "   |  %s  |  %s  |  %s  |\n",
			 get_box_name(info[i].box_map[0][0]),
			 get_box_name(info[i].box_map[0][1]),
			 get_box_name(info[i].box_map[0][2])) < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "   -----------------\n") < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "   |  %s  |  %s  |  %s  |\n",
			 get_box_name(info[i].box_map[1][0]),
			 get_box_name(info[i].box_map[1][1]),
			 get_box_name(info[i].box_map[1][2])) < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "   -----------------\n") < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
	}

	if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
		PAGE_SIZE_MAX - strlen(buf) - 1, "Box_NAME:\n") < 0)
		pr_err("%s: snprintf_s is failed\n", __func__);
	for (i = 0; i < device_boxid.box_num; i++) {
		if (info[i].box_enable != BOX_ID_ENABLED)
			continue;

		vendor = boxid_read(i);
		if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
			PAGE_SIZE_MAX - strlen(buf) - 1, "   box[%d]  :  %s\n",
			i, get_box_name(vendor)) < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);
	}

	if (snprintf_s(buf + strlen(buf), PAGE_SIZE_MAX - strlen(buf),
		PAGE_SIZE_MAX - strlen(buf) - 1, "---boxid info end---\n") < 0)
		pr_err("%s: snprintf_s is failed\n", __func__);

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

	if (!strncmp(mode, "hkadc", strlen("hkadc")))
		return DEVICEBOX_ID_MODE_USE_HKADC;

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

static void get_gpio_tristate(struct pinctrl *pin, struct device_node *node,
			      int *gpio_status, unsigned int gpio_id)
{
	int status = *gpio_status;
	int ret;
	int value;

	if (!device_boxid.tristate_mode)
		return;

	if (status == ELECTRIC_LEVEL_LOW)
		return;
	if (!device_boxid.is_ext_gpio) {
		ret = boxid_pinctrl_select_state(pin, "default");
		if (ret) {
			pr_info("%s: select state error %d\n", __func__, ret);
			return;
		}
	}
	mdelay(1);
	value = gpio_get_value_cansleep(gpio_id);
	if (value == ELECTRIC_LEVEL_HIGH)
		*gpio_status = ELECTRIC_LEVEL_HIGH;
	else if (value == ELECTRIC_LEVEL_LOW)
		*gpio_status = ELECTRIC_LEVEL_TRISTATE;
	else
		pr_info("%s: tristate mode is invalid\n", __func__);
	pr_info("%s: tristate mode read %d\n", __func__, *gpio_status);
}

static int get_gpio_status(struct pinctrl *pin, struct device_node *node,
			   const char *propname, int boxid, int index)
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
	if (ret)
		pr_err("%s: set gpio to input status error! ret is %d\n",
		       __func__, ret);

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
	get_gpio_tristate(pin, node, &(info[boxid].gpio_status[index]),
			  info[boxid].gpio_id[index]);

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

static bool get_boxid_is_extgpio(struct device_node *node)
{
	if (of_property_read_bool(node, "boxid_is_extgpio")) {
		pr_info("%s: boxid is extgpio from pca953x IC, without pinctrl\n",
			__func__);
		return true;
	}

	pr_info("%s: boxid is soc gpio\n", __func__);
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

	device_boxid.is_ext_gpio = get_boxid_is_extgpio(node);

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

static int get_boxid_map_priv(struct device_node *node, const char *propname,
			      int id)
{
	struct box_info_st *info = device_boxid.box_info;
	int ret = 0;
	int temp = 0;
	int i;
	unsigned int index1;
	unsigned int index2;

	pr_debug("%s, propname=%s, box_index=%d, gpio_num=%d\n", __func__,
		 propname, id, info[id].gpio_num);

	if (info[id].gpio_num == USE_ONE_GPIO ||
		info[id].gpio_num == USE_TWO_GPIO ||
		info[id].gpio_num == USE_NO_GPIO) {
		for (i = 0; i < (VENDOR * VENDOR); i++) {
			if (!of_property_read_u32_index (node,
				propname, i, &temp)) {
				index1 = i / VENDOR;
				index2 = i % VENDOR;
				info[id].box_map[index1][index2] = temp;
				pr_info("%s, read index=%d, value=%d\n",
					__func__, i, temp);
			} else {
				break;
			}
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

		if (strncpy_s(map_name, NAME_LEN_MAX, box_map_table[i],
			NAME_LEN_MAX - 1) != EOK)
			pr_err("%s: strncpy_s is failed\n", __func__);
		ret = get_boxid_map_priv(node, map_name, i);
		if (ret)
			return ret;
		if (memset_s(map_name, NAME_LEN_MAX, 0, NAME_LEN_MAX) != EOK)
			pr_err("%s: memset_s is failed\n", __func__);
	}

	return ret;
}

static int get_boxid_gpio_status(struct pinctrl *pin, struct device_node *node)
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

		if (strncpy_s(gpio_name, NAME_LEN_MAX, gpio_name_table[i],
			NAME_LEN_MAX - 1) != EOK)
			pr_err("%s: strncpy_s is failed\n", __func__);
		for (j = 0; j < device_boxid.box_info[i].gpio_num; j++) {
			ret = get_gpio_status(pin, node, gpio_name, i, j);
			if (ret != 0)
				return ret;
		}
		if (memset_s(gpio_name, NAME_LEN_MAX, 0, NAME_LEN_MAX) != EOK)
			pr_err("%s: memset_s is failed\n", __func__);
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

			if (gpio_is_valid(info[i].gpio_id[j])) {
				gpio_free(info[i].gpio_id[j]);
				info[i].gpio_id[j] = GPIO_ID_FAIL;
				info[i].gpio_req_flag[j] = GPIO_REQ_FAIL;
			}
		}
	}
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

	if (!device_boxid.is_ext_gpio) {
		p = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR(p)) {
			pr_err("%s: could not get pinctrl\n", __func__);
			return -ENOENT;
		}

		if (!device_boxid.tristate_mode) {
			ret = boxid_pinctrl_select_state(p, "default");
			if (ret)
				return -ENOENT;
		}
	}

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

	ret = get_boxid_gpio_status(p, node);
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
	if (!device_boxid.is_ext_gpio) {
		ret = boxid_pinctrl_select_state(p, "idle");
		if (ret)
			goto err_get_gpio_status;
	}

	ret = boxid_probe_init_fs(pdev);
	if (ret)
		goto err_get_gpio_status;

	return 0;

err_get_gpio_status:
	boxid_gpio_free();
	return ret;
}

static void update_spkbox_voltage_status(int voltage)
{
	struct box_info_st *info = device_boxid.box_info;
	struct box_hkadc_info *box_hkadc_info = &device_boxid.spkbox_hkadc_info;

	if (voltage >= box_hkadc_info->hkadc_min_voltage &&
		voltage < box_hkadc_info->hkadc_mid_voltage)
		info[SPEAKER_ID].voltage_status = ELECTRIC_LEVEL_LOW;
	else if (voltage >= box_hkadc_info->hkadc_mid_voltage &&
		voltage <= box_hkadc_info->hkadc_max_voltage)
		info[SPEAKER_ID].voltage_status = ELECTRIC_LEVEL_HIGH;
	else
		info[SPEAKER_ID].voltage_status = ELECTRIC_LEVEL_INVALID;

	pr_info("hkadc voltage = %d, spkbox_voltage_status = %d\n",
		voltage, info[SPEAKER_ID].voltage_status);
}

__attribute__((weak))int hisi_adc_get_value(int adc_channel)
{
	pr_info("%s:use weak func, channel = %d\n", __func__, adc_channel);
	return 0;
}

static int get_hkadc_voltage(unsigned int hkadc_channel)
{
	return hisi_adc_get_value(hkadc_channel);
}

static int boxid_probe_use_hkadc(struct platform_device *pdev)
{
	unsigned int temp = 0;
	int hkadc_voltage;
	struct device_node *node = pdev->dev.of_node;
	struct box_hkadc_info *box_hkadc_info = &device_boxid.spkbox_hkadc_info;

	box_hkadc_info->hkadc_channel = SPKID_CHANNEL_DEFAULT;
	box_hkadc_info->hkadc_min_voltage = HKADC_MIN_VOL_DEFAULT;
	box_hkadc_info->hkadc_mid_voltage = HKADC_MID_VOL_DEFAULT;
	box_hkadc_info->hkadc_max_voltage = HKADC_MAX_VOL_DEFAULT;

	if (!of_property_read_u32(node, "spkid_hkadc_channel", &temp))
		box_hkadc_info->hkadc_channel = temp;

	if (!of_property_read_u32(node, "hkadc_min_voltage", &temp))
		box_hkadc_info->hkadc_min_voltage = temp;

	if (!of_property_read_u32(node, "hkadc_mid_voltage", &temp))
		box_hkadc_info->hkadc_mid_voltage = temp;

	if (!of_property_read_u32(node, "hkadc_max_voltage", &temp))
		box_hkadc_info->hkadc_max_voltage = temp;

	pr_debug("hkadc_spk_channel %u, hkadc_min_voltage %u,"
		" hkadc_mid_voltage %u, hkadc_max_voltage %u\n",
		box_hkadc_info->hkadc_channel,
		box_hkadc_info->hkadc_min_voltage,
		box_hkadc_info->hkadc_mid_voltage,
		box_hkadc_info->hkadc_max_voltage);

	hkadc_voltage = get_hkadc_voltage(box_hkadc_info->hkadc_channel);
	if (hkadc_voltage < 0) {
		pr_err("hkadc_voltage = %d, out of range\n", hkadc_voltage);
		return -ERANGE;
	}

	update_spkbox_voltage_status(hkadc_voltage);
	return boxid_probe_init_fs(pdev);
}

int boxid_get_prop_of_u32_array(struct device_node *node,
	const char *propname, u32 **value, unsigned int *num)
{
	u32 *array = NULL;
	int ret;
	int count = 0;

	if ((node == NULL) || (propname == NULL) || (value == NULL) ||
		(num == NULL)) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (of_property_read_bool(node, propname))
		count = of_property_count_elems_of_size(node,
				propname, (int)sizeof(u32));

	if (count == 0) {
		pr_info("%s: %s not existed, skip\n", __func__, propname);
		*num = 0;
		return 0;
	}

	array = kzalloc(sizeof(u32) * count, GFP_KERNEL);
	if (array == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(node, propname, array,
			(size_t)(long)count);
	if (ret < 0) {
		pr_err("%s: get %s array failed\n", __func__, propname);
		ret = -EFAULT;
		goto get_prop_of_u32_array_err;
	}

	*value = array;
	*num = count;
	return 0;

get_prop_of_u32_array_err:
	boxid_kfree_ops(array);
	return ret;
}

static void boxid_copy_actual_box_info(struct actual_box_group *box_group,
	int actual_id)
{
	unsigned int i;
	int copy_id;
	struct box_info_st *copy_info = NULL;
	struct box_info_st *actual_info = NULL;

	if (box_group == NULL)
		return;

	actual_info =
		(struct box_info_st *)&(device_boxid.box_info[actual_id]);
	for (i = 0; i < box_group->num; i++) {
		copy_id = box_group->share_spkid[i];
		pr_info("%s: copy id:%d to id:%d\n", __func__,
			actual_id, copy_id);
		copy_info =
			(struct box_info_st *)&(device_boxid.box_info[copy_id]);
		if (memcpy_s(copy_info, sizeof(*copy_info), actual_info,
			sizeof(*copy_info)) != EOK)
			pr_err("%s: memcpy_s is failed\n", __func__);
	}
}

static void boxid_get_share_box_info(struct boxid_share_info *share_info)
{
	struct actual_box_group *box_group = NULL;
	unsigned int i;
	int actual_id;

	for (i = 0; i < share_info->actual_boxid_num; i++) {
		box_group =
			(struct actual_box_group *)&(share_info->box_group[i]);

		actual_id = share_info->actual_boxid[i];
		pr_info("%s: actual_boxid num = %d, box_grup->num =%d\n",
			__func__, share_info->actual_boxid_num, box_group->num);
		boxid_copy_actual_box_info(box_group, actual_id);
	}
}

static int boxid_check_share_spkid(struct actual_box_group *box_group)
{
	unsigned int i;

	if (box_group == NULL) {
		pr_err("%s:box_group is NULL\n", __func__);
		return -EFAULT;
	}

	for (i = 0; i < box_group->num; i++) {
		if (box_group->share_spkid[i] < 0 ||
			box_group->share_spkid[i] >= BOX_NUM_MAX) {
			pr_err("%s: invail spkid%d = %d\n",
				__func__, i,
				box_group->share_spkid[i]);
			return -EFAULT;
		}
	}

	return 0;
}

static int boxid_parse_actual_boxid_groups(struct device_node *node,
	struct boxid_share_info *share_info)
{
	int ret;
	unsigned int i;
	char group_name[NAME_LEN_MAX] = { 0 };
	struct actual_box_group *box_group = NULL;

	if (share_info == NULL || node == NULL) {
		pr_err("%s: node or share_info is NULL\n", __func__);
		return -EFAULT;
	}

	if (share_info->actual_boxid_num > BOX_NUM_MAX) {
		pr_err("%s: actual_boxid_num: %d is error\n",
			__func__, share_info->actual_boxid_num);
		return -EFAULT;
	}

	for (i = 0; i < share_info->actual_boxid_num; i++) {
		box_group =
			(struct actual_box_group *)&(share_info->box_group[i]);
		if (memset_s(group_name, NAME_LEN_MAX, 0, NAME_LEN_MAX) != EOK)
			pr_err("%s: memset_s is failed\n", __func__);

		if (snprintf_s(group_name,
			(unsigned long)NAME_LEN_MAX, (unsigned long)NAME_LEN_MAX - 1,
			"actual_boxid%d_group", i) < 0)
			pr_err("%s: snprintf_s is failed\n", __func__);

		ret = boxid_get_prop_of_u32_array(node, group_name,
			(u32 **)&(box_group->share_spkid), &box_group->num);
		/* not return negative value, actual group may be not exist */
		if ((box_group->num == 0) || (ret < 0)) {
			box_group->num = 0;
			pr_info("%s: get actual boxid:%d group:%d error\n",
				__func__, i, box_group->num);
		}
	}

	for (i = 0; i < share_info->actual_boxid_num; i++) {
		box_group =
			(struct actual_box_group *)&(share_info->box_group[i]);
		ret = boxid_check_share_spkid(box_group);
		if (ret) {
			pr_err("%s: invaild spkid in boxid_group num:%d\n",
				__func__, box_group->num);
			return ret;
		}
	}
	return 0;
}

static int boxid_parse_actual_boxid_num(struct device_node *node,
	struct boxid_share_info *share_info)
{
	int ret;
	unsigned int i;
	const char *actual_boxid_num_str = "actual_boxid_num";

	if (share_info == NULL || node == NULL) {
		pr_err("%s: node or share_info is NULL\n", __func__);
		return -EFAULT;
	}

	ret = boxid_get_prop_of_u32_array(node, actual_boxid_num_str,
		(u32 **)&(share_info->actual_boxid),
		&share_info->actual_boxid_num);
	if ((share_info->actual_boxid_num == 0 ||
		share_info->actual_boxid_num > BOX_NUM_MAX) || (ret < 0)) {
		pr_err("%s: actual_boxid_num:%d, or ret:%d, is error\n",
			__func__, share_info->actual_boxid_num, ret);
		return -EFAULT;
	}

	pr_info("%s: actual_boxid_num = %d\n",
		__func__, share_info->actual_boxid_num);
	for (i = 0; i < share_info->actual_boxid_num; i++) {
		if (share_info->actual_boxid[i] >= BOX_NUM_MAX) {
			pr_info("%s: actual_boxid[%d] = %d is invaild\n",
				__func__, i, share_info->actual_boxid[i]);
			return -EFAULT;
		}
	}
	return 0;
}


static void boxid_free_share_info(
	struct boxid_share_info *share_info)
{
	unsigned int i;
	struct actual_box_group *box_group = NULL;

	if (share_info == NULL)
		return;

	for (i = 0; i < share_info->actual_boxid_num; i++) {
		if (i >= BOX_NUM_MAX)
			break;

		box_group =
			(struct actual_box_group *)&(share_info->box_group[i]);
		if (box_group != NULL)
			boxid_kfree_ops(box_group->share_spkid);
	}

	boxid_kfree_ops(share_info->actual_boxid);
	boxid_kfree_ops(share_info);
	share_info = NULL;
}


static int boxid_parse_dt_share_info(struct platform_device *pdev)
{
	const char *share_info_str = "boxid_share_info";
	const char *support_share_str = "hardware_support_share";
	struct device_node *node = NULL;
	struct boxid_share_info *share_info = NULL;
	int ret;

	if (pdev == NULL) {
		pr_err("%s: pdev is NULL\n", __func__);
		return -EFAULT;
	}

	node = of_get_child_by_name(pdev->dev.of_node, share_info_str);
	if (node == NULL) {
		pr_info("%s: boxid_share_info not existed, skip\n", __func__);
		return 0;
	}

	share_info = kzalloc(sizeof(*share_info), GFP_KERNEL);
	if (share_info == NULL)
		return -ENOMEM;

	share_info->actual_boxid_num = 0;
	share_info->share_support = of_property_read_bool(node,
		support_share_str);
	if (!share_info->share_support) {
		pr_info("%s: boxID not support share\n", __func__);
		goto err_parse_dt_share_info;
	}

	ret = boxid_parse_actual_boxid_num(node, share_info);
	if (ret < 0)
		goto err_parse_dt_share_info;

	ret = boxid_parse_actual_boxid_groups(node, share_info);
	if (ret < 0)
		goto err_parse_dt_share_info;

	boxid_get_share_box_info(share_info);

err_parse_dt_share_info:
	boxid_free_share_info(share_info);
	return 0;
}

static void boxid_free_consistent_test_info(
	struct box_consistent_test *test)
{
	unsigned int i;
	struct boxs_vendor_test_groups *groups = NULL;

	/* not support consistent, it not kzalloc box_consistent_test */
	if (device_boxid.consistent_test.test_support == false)
		return;

	if (test == NULL)
		return;

	for (i = 0; i < test->group_num; i++) {
		if (i >= BOX_NUM_MAX)
			break;

		groups = (struct boxs_vendor_test_groups *)&(test->groups[i]);
		if (groups != NULL)
			boxid_kfree_ops(groups->boxid);
	}

	test = NULL;
}

static void box_consistent_test_check(struct box_consistent_test *test,
			struct device_node *node)
{
	unsigned int i;
	unsigned int j;
	int ret;
	const char *test_groups = "consistent_test_groups";

	if (test == NULL || node == NULL) {
		pr_err("%s: test or share_info is NULL\n", __func__);
		return;
	}

	for (i = 0; i < test->group_num; i++) {
		ret = boxid_get_prop_of_u32_array(node, test_groups,
			(u32 **)&(test->groups[i].boxid),
			&(test->groups[i].num));
		if ((test->groups[i].num == 0 ||
			test->groups[i].num > BOX_NUM_MAX) || (ret < 0)) {
			pr_err("%s: groups[i].num:%d, or ret:%d, is error\n",
				__func__, test->groups[i].num, ret);
			boxid_free_consistent_test_info(test);
			test->group_num = 0;
			test->test_support = false;
			return;
		}

		for (j = 0; j < test->groups[i].num; j++) {
			if (test->groups[i].boxid[j] < 0 ||
				test->groups[i].boxid[j] >= BOX_NUM_MAX) {
				pr_err("%s:groups[%d].boxid[%d]:%d, is error\n",
					__func__, i, j,
					test->groups[i].boxid[j]);
				boxid_free_consistent_test_info(test);
				test->group_num = 0;
				test->test_support = false;
				return;
			}
		}
	}
}

static void box_parse_dt_consistent_test_info(struct platform_device *pdev)
{
	const char *test_info = "box_consistent_test_info";
	const char *consistent_test_support = "consistent_test_support";

	struct device_node *node = NULL;
	struct box_consistent_test *test = NULL;
	int ret;

	if (pdev == NULL) {
		pr_err("%s: pdev is NULL\n", __func__);
		return;
	}

	test = (struct box_consistent_test *)&(device_boxid.consistent_test);
	if (memset_s(test, sizeof(*test), 0, sizeof(*test)) != EOK)
		pr_err("%s: memset_s is failed\n", __func__);

	node = of_get_child_by_name(pdev->dev.of_node, test_info);
	if (node == NULL) {
		pr_info("%s:consistent test info not exist, skip\n", __func__);
		return;
	}

	test->test_support = of_property_read_bool(node,
		consistent_test_support);
	if (!test->test_support) {
		pr_info("%s: consistent_test not support\n", __func__);
		return;
	}

	ret = of_property_read_u32(node, "test_group_num",
		(u32 *)(&test->group_num));
	if (ret < 0 || test->group_num > BOX_NUM_MAX) {
		pr_err("%s: get test_group_num:%d or ret:%d failed\n",
			__func__, test->group_num, ret);
		test->group_num = 0;
		test->test_support = false;
		return;
	}

	pr_info("%s: test->group_num = %d\n", __func__, test->group_num);
	box_consistent_test_check(test, node);
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

	/* read tristate_mode */
	ret = of_property_read_u32(node, "tristate_mode",
				   &(device_boxid.tristate_mode));
	if (ret) {
		device_boxid.tristate_mode = 0;
		pr_info("%s: tristate_mode is disable\n", __func__);
	} else {
		pr_info("%s: tristate_mode is %d\n", __func__,
			device_boxid.tristate_mode);
	}

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

	if (device_boxid.check_mode == DEVICEBOX_ID_MODE_USE_HKADC) {
		ret = boxid_probe_use_hkadc(pdev);
		if (ret < 0) {
			pr_err("%s failed\n", __func__);
			return ret;
		}
	}
	boxid_parse_dt_share_info(pdev);
	box_parse_dt_consistent_test_info(pdev);
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

	boxid_free_consistent_test_info(
		(struct box_consistent_test *)&device_boxid.consistent_test);

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

