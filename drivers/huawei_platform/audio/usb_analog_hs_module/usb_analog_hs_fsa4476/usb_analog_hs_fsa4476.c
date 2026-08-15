/*
 * usb_analog_hs_fsa4476.c
 *
 * usb analog headset fsa4476 driver
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

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/hisi/hi64xx/hi64xx_mbhc.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_SUPERSWITCH_FSC
#include "../../../usb/superswitch/fsc/Platform_Linux/fusb3601_global.h"
#endif
#include "huawei_platform/audio/usb_analog_hs_fsa4476.h"

#define SD_GPIO_SCTRL_REG    0xfff0a314

#define HWLOG_TAG usb_analog_hs_fsa4476
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define MAX_BUF_SIZE                    32
#define DEFAULT_MIC_SWITCH_DELAY_TIME   0
#define HIGH                            1
#define LOW                             0
#define PLUG_IN_STATUS_STABLE_DELAY     800
#define WAKE_LOCK_TIMEOUT               1000

struct usb_ana_hs_fsa4476_data {
	int gpio_type;
	int gpio_en1;
	int gpio_en2;
	int gpio_enn;
	/* H: sbu1/sbu2 hung up; L: sbu1/sbu2 connected to mic/gnd */
	int mic_gnd_np;
	/* H: sbu1-->mic, sbu2-->gnd; L: sbu1-->gnd, sbu2-->mic */
	int mic_gnd_switch;
	int mic_switch_delay_time;
	/* usb analog headset dev register flag */
	int registed;
	int sd_gpio_used;
	struct wake_lock wake_lock;
	struct mutex mutex;
	struct workqueue_struct *analog_hs_plugin_delay_wq;
	struct delayed_work analog_hs_plugin_delay_work;

	struct workqueue_struct *analog_hs_plugout_delay_wq;
	struct delayed_work analog_hs_plugout_delay_work;

	struct usb_analog_hs_dev *codec_ops_dev;
	/* store codec description data */
	void *private_data;
	int usb_analog_hs_in;
	bool using_superswitch;
	bool support_cc;
	bool connect_linein_r;
	/*
	 * HUAWEI USB-C TO 3.5MM AUDIO ADAPTER has TDD noise,
	 * when usb analog hs plug in, need switch to upper antenna.
	 */
	bool switch_antenna;
};

enum {
	FSA4776_ENABLE     = 0,
	FSA4776_DISABLE    = 1,
};

static struct usb_ana_hs_fsa4476_data *fsa4476_pdata;

static inline int hs_gpio_get_value(int gpio)
{
	if (fsa4476_pdata->gpio_type == USB_ANALOG_HS_GPIO_CODEC)
		return gpio_get_value_cansleep(gpio);
	return gpio_get_value(gpio);
}

static void hs_gpio_set_value(int gpio, int value)
{
	if (gpio <= 0)
		return;
	if (fsa4476_pdata->gpio_type == USB_ANALOG_HS_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

void usb_analog_hs_fsa4476_set_gpio_state(int enn, int en1, int en2)
{
	if (fsa4476_pdata->using_superswitch)
		hs_gpio_set_value(fsa4476_pdata->gpio_enn, enn);

	hs_gpio_set_value(fsa4476_pdata->gpio_en1, en1);
	/* invert mic_gnd_np */
	hs_gpio_set_value(fsa4476_pdata->mic_gnd_np, en1 ? 0 : 1);
	hs_gpio_set_value(fsa4476_pdata->gpio_en2, en2);
	hs_gpio_set_value(fsa4476_pdata->mic_gnd_switch, en2);
}

static void usb_analog_hs_fsa4476_enable(int enable)
{
	if (fsa4476_pdata->using_superswitch)
		hs_gpio_set_value(fsa4476_pdata->gpio_enn, enable);
}

#ifdef CONFIG_SUPERSWITCH_FSC
static int set_superswitch_sbu_switch(SbuSwitch_t sbu_switch)
{
	if (fsa4476_pdata->using_superswitch) {
		struct fusb3601_chip *chip = fusb3601_GetChip();

		if (!chip) {
			hwlog_err("%s fusb3601_chip is NULL\n", __func__);
			return -1;
		}
		FUSB3601_set_sbu_switch(&chip->port, sbu_switch);
	}
	return 0;
}
#endif

static void usb_analog_hs_plugin_work(struct work_struct *work)
{
	IN_FUNCTION;

	wake_lock(&fsa4476_pdata->wake_lock);
	/*
	 * change codec hs resistence from 70ohm to 3Kohm,
	 * to reduce the pop sound in hs when usb analog hs plug in.
	 */
	fsa4476_pdata->codec_ops_dev->ops.hs_high_resistence_enable(
		fsa4476_pdata->private_data, true);
#ifdef CONFIG_SUPERSWITCH_FSC
	/* SBU2 connect to HS_FB */
	if (set_superswitch_sbu_switch(Sbu_Cross_Close_Aux) < 0) {
		wake_unlock(&fsa4476_pdata->wake_lock);
		return;
	}
#endif
	/* to avoid TDD-noise */
	if (fsa4476_pdata->switch_antenna)
		pd_dpm_send_event(ANA_AUDIO_IN_EVENT);

	usb_analog_hs_fsa4476_enable(FSA4776_ENABLE);
	hs_gpio_set_value(fsa4476_pdata->gpio_en1, 1);
	hs_gpio_set_value(fsa4476_pdata->mic_gnd_np, 0);

	msleep(20);
	fsa4476_pdata->codec_ops_dev->ops.plug_in_detect(
		fsa4476_pdata->private_data);
	mutex_lock(&fsa4476_pdata->mutex);
	fsa4476_pdata->usb_analog_hs_in = USB_ANA_HS_PLUG_IN;

	mutex_unlock(&fsa4476_pdata->mutex);
	/* recovery codec hs resistence to 70ohm */
	fsa4476_pdata->codec_ops_dev->ops.hs_high_resistence_enable(
		fsa4476_pdata->private_data, false);
	wake_unlock(&fsa4476_pdata->wake_lock);

	OUT_FUNCTION;
}

static void usb_analog_hs_plugout_work(struct work_struct *work)
{
	IN_FUNCTION;

	wake_lock(&fsa4476_pdata->wake_lock);
	if (fsa4476_pdata->analog_hs_plugin_delay_wq) {
		hwlog_info("%s remove plugin work, insert-remov too fast\n",
			__func__);
		cancel_delayed_work(
			&fsa4476_pdata->analog_hs_plugin_delay_work);
		flush_workqueue(fsa4476_pdata->analog_hs_plugin_delay_wq);
	}
	if (fsa4476_pdata->usb_analog_hs_in == USB_ANA_HS_PLUG_IN) {
		hwlog_info("%s usb analog hs plug out act\n", __func__);
		fsa4476_pdata->codec_ops_dev->ops.plug_out_detect(
			fsa4476_pdata->private_data);
		mutex_lock(&fsa4476_pdata->mutex);
		fsa4476_pdata->usb_analog_hs_in = USB_ANA_HS_PLUG_OUT;
		mutex_unlock(&fsa4476_pdata->mutex);
		hs_gpio_set_value(fsa4476_pdata->gpio_en1, 0);
		hs_gpio_set_value(fsa4476_pdata->mic_gnd_np, 1);
		hs_gpio_set_value(fsa4476_pdata->gpio_en2, 0);
		hs_gpio_set_value(fsa4476_pdata->mic_gnd_switch, 0);
		if (fsa4476_pdata->switch_antenna)
			/* notify the phone: usb analog hs plug out. */
			pd_dpm_send_event(ANA_AUDIO_OUT_EVENT);

#ifdef CONFIG_SUPERSWITCH_FSC
		/* HS_FB disconnect */
		if (set_superswitch_sbu_switch(Sbu_None) < 0) {
			wake_unlock(&fsa4476_pdata->wake_lock);
			return;
		}
#endif
		usb_analog_hs_fsa4476_enable(FSA4776_DISABLE);
	}
	wake_unlock(&fsa4476_pdata->wake_lock);

	OUT_FUNCTION;
}

static int set_mic_sel_val(int sel_value)
{
	int ret = sel_value ? 0 : 1;

#ifdef CONFIG_SUPERSWITCH_FSC
	if (sel_value) {
		if (set_superswitch_sbu_switch(Sbu_Cross_Close_Aux) < 0)
			return -ENODEV;
	} else {
		if (set_superswitch_sbu_switch(Sbu_Close_Aux) < 0)
			return -ENODEV;
	}
#endif
	return ret;
}

int usb_ana_hs_fsa4476_dev_register(struct usb_analog_hs_dev *dev,
				    void *codec_data)
{
	/* usb analog headset driver not be probed, just return */
	if (!fsa4476_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return -ENODEV;
	}

	/* only support one codec to be registed */
	if (fsa4476_pdata->registed == USB_ANALOG_HS_ALREADY_REGISTER) {
		hwlog_err("%s codec has registed, no more permit\n", __func__);
		return -EEXIST;
	}

	if (!dev->ops.plug_in_detect || !dev->ops.plug_out_detect) {
		hwlog_err("%s codec ops function must be all registed\n",
			__func__);
		return -EINVAL;
	}

	mutex_lock(&fsa4476_pdata->mutex);
	fsa4476_pdata->codec_ops_dev = dev;
	fsa4476_pdata->private_data = codec_data;
	fsa4476_pdata->registed = USB_ANALOG_HS_ALREADY_REGISTER;
	mutex_unlock(&fsa4476_pdata->mutex);

	hwlog_info("%s usb analog hs register sucessful\n", __func__);

	return 0;
}

int usb_ana_hs_fsa4476_check_hs_pluged_in(void)
{
	int analog_hs_state = pd_dpm_get_analog_hs_state();

	hwlog_info("%s analog_hs_state =%d\n", __func__, analog_hs_state);

	if (analog_hs_state)
		return USB_ANA_HS_PLUG_IN;
	return USB_ANA_HS_PLUG_OUT;
}

void usb_ana_hs_fsa4476_mic_switch_change_state(void)
{
	int mic_sel_val;
	int mic_gnd_switch;

	/* usb analog headset driver not be probed, just return */
	if (!fsa4476_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}

	if (fsa4476_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}

	IN_FUNCTION;

	mic_sel_val = hs_gpio_get_value(fsa4476_pdata->gpio_en2);
	hwlog_info("%s gpio mic sel is %d\n", __func__, mic_sel_val);
	if (fsa4476_pdata->mic_gnd_switch > 0) {
		mic_gnd_switch = hs_gpio_get_value(
			fsa4476_pdata->mic_gnd_switch);
		hwlog_info("%s gpio mic gnd switch is %d\n",
			__func__, mic_gnd_switch);
	}

	mic_sel_val = set_mic_sel_val(mic_sel_val);
	if (mic_sel_val < 0)
		return;

	hwlog_info("%s gpio mic sel will set to %d\n", __func__, mic_sel_val);

	hs_gpio_set_value(fsa4476_pdata->gpio_en2, mic_sel_val);
	hs_gpio_set_value(fsa4476_pdata->mic_gnd_switch, mic_sel_val);
	if (fsa4476_pdata->mic_switch_delay_time > 0)
		msleep(fsa4476_pdata->mic_switch_delay_time);
	mic_sel_val = hs_gpio_get_value(fsa4476_pdata->gpio_en2);
	hwlog_info("%s gpio mic sel change to %d\n", __func__, mic_sel_val);
	mic_gnd_switch = hs_gpio_get_value(fsa4476_pdata->mic_gnd_switch);
	hwlog_info("%s gpio mic gnd switch change to %d\n",
		__func__, mic_gnd_switch);

	OUT_FUNCTION;
}

void usb_ana_hs_fsa4476_plug_in_out_handle(int hs_state)
{
	/* usb analog headset driver not be probed, just return */
	if (!fsa4476_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}
	if (fsa4476_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}
	IN_FUNCTION;

	wake_lock_timeout(&fsa4476_pdata->wake_lock,
		msecs_to_jiffies(WAKE_LOCK_TIMEOUT));

	switch (hs_state) {
	case USB_ANA_HS_PLUG_IN:
		queue_delayed_work(fsa4476_pdata->analog_hs_plugin_delay_wq,
			&fsa4476_pdata->analog_hs_plugin_delay_work,
			msecs_to_jiffies(PLUG_IN_STATUS_STABLE_DELAY));
		break;
	case USB_ANA_HS_PLUG_OUT:
		queue_delayed_work(fsa4476_pdata->analog_hs_plugout_delay_wq,
			&fsa4476_pdata->analog_hs_plugout_delay_work, 0);
		break;
	case DP_PLUG_IN:
		usb_analog_hs_fsa4476_set_gpio_state(FSA4776_ENABLE, 0, 0);
		break;
	case DP_PLUG_IN_CROSS:
		usb_analog_hs_fsa4476_set_gpio_state(FSA4776_ENABLE, 0, 1);
		break;
	case DP_PLUG_OUT:
		usb_analog_hs_fsa4476_set_gpio_state(FSA4776_DISABLE, 0, 0);
		break;
	case DIRECT_CHARGE_IN:
		usb_analog_hs_fsa4476_set_gpio_state(FSA4776_ENABLE, 0, 0);
		break;
	case DIRECT_CHARGE_OUT:
		usb_analog_hs_fsa4476_set_gpio_state(FSA4776_DISABLE, 0, 0);
		break;
	default:
		break;
	}
	hwlog_info("%s hs_state is %d\n", __func__, hs_state);
	OUT_FUNCTION;
}

bool check_usb_analog_hs_fsa4476_support(void)
{
	if (!fsa4476_pdata)
		return false;

	if (fsa4476_pdata->registed == USB_ANALOG_HS_NOT_REGISTER)
		return false;
	return true;
}

static const struct of_device_id usb_ana_hs_fsa4476_of_match[] = {
	{ .compatible = "huawei,usb_ana_hs_fsa4476", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_ana_hs_fsa4476_of_match);

/* load dts config for board difference */
static void load_gpio_type_config(struct device_node *node)
{
	unsigned int temp = USB_ANALOG_HS_GPIO_SOC;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		fsa4476_pdata->gpio_type = temp;
	else
		fsa4476_pdata->gpio_type = USB_ANALOG_HS_GPIO_SOC;
}

static int usb_ana_hs_load_gpio(struct device *dev, int *gpio_index,
			int out_value, const char *gpio_name)
{
	int gpio;
	int ret;

	gpio = of_get_named_gpio(dev->of_node, gpio_name, 0);
	if (gpio < 0) {
		hwlog_info("%s:Looking up %s property in node %s failed %d\n",
			__func__, dev->of_node->full_name, gpio_name, gpio);
		return -ENOENT;
	}
	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s mic_gnd_switch is unvalid\n", __func__);
		return -ENOENT;
	}
	ret = gpio_request(gpio, gpio_name);
	if (ret < 0)
		hwlog_err("%s request GPIO for %s fail %d\n",
			__func__, gpio_name, ret);
	else
		gpio_direction_output(gpio, out_value);
	*gpio_index = gpio;
	return ret;
}

static void usb_analog_hs_free_gpio(void)
{
	IN_FUNCTION;

	if (!fsa4476_pdata)
		return;

	if (fsa4476_pdata->gpio_enn > 0)
		gpio_free(fsa4476_pdata->gpio_enn);

	if (fsa4476_pdata->gpio_en1 > 0)
		gpio_free(fsa4476_pdata->gpio_en1);

	if (fsa4476_pdata->gpio_en2 > 0)
		gpio_free(fsa4476_pdata->gpio_en2);

	if (fsa4476_pdata->mic_gnd_np > 0)
		gpio_free(fsa4476_pdata->mic_gnd_np);

	if (fsa4476_pdata->mic_gnd_switch > 0)
		gpio_free(fsa4476_pdata->mic_gnd_switch);
}

static int usb_ana_hs_fsa4476_load_gpio_pdata(struct device *dev)
{
	if (usb_ana_hs_load_gpio(dev, &(fsa4476_pdata->gpio_enn), LOW,
		"swtich_enn") < 0)
		goto fsa4476_get_gpio_err;

	if (usb_ana_hs_load_gpio(dev, &(fsa4476_pdata->gpio_en1), LOW,
		"swtich_en1") < 0)
		goto fsa4476_get_gpio_err;

	if (usb_ana_hs_load_gpio(dev, &(fsa4476_pdata->gpio_en2), LOW,
		"swtich_en2") < 0)
		goto fsa4476_get_gpio_err;

	if (usb_ana_hs_load_gpio(dev, &(fsa4476_pdata->mic_gnd_np), HIGH,
		"mic_gnd_np") < 0) {
		hwlog_info("%s: mic_gnd_np is unvalid\n", __func__);
	}

	if (usb_ana_hs_load_gpio(dev, &(fsa4476_pdata->mic_gnd_switch), HIGH,
		"mic_gnd_switch") < 0) {
		hwlog_info("%s: mic_gnd_switch is unvalid\n", __func__);
	}

	/* get sd gpio use config */
	fsa4476_pdata->sd_gpio_used = of_get_named_gpio(dev->of_node,
		"sd_gpio_used", 0);
	if (fsa4476_pdata->sd_gpio_used < 0) {
		hwlog_err("%s:can not get sd_gpio_used , use default\n",
			__func__);
		fsa4476_pdata->sd_gpio_used = 0;
	}

	return 0;
fsa4476_get_gpio_err:
	usb_analog_hs_free_gpio();
	return -ENOENT;
}

#ifdef USB_ANALOG_HEADSET_DEBUG
static ssize_t usb_ana_hs_fsa4476_mic_switch_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int value;
	int count = 0;

	if (!fsa4476_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return count;
	}

	if (fsa4476_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s pdata is not registed\n", __func__);
		return count;
	}

	value = hs_gpio_get_value(fsa4476_pdata->gpio_en2);

	return scnprintf(buf, MAX_BUF_SIZE, "%d\n", value);
}

static ssize_t usb_ana_hs_fsa4476_mic_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val;

	if (!fsa4476_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return count;
	}

	if (fsa4476_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s pdata is not registed\n", __func__);
		return count;
	}

	ret = kstrtol(buf, 10, &val);
	if (ret < 0) {
		hwlog_err("%s input error\n", __func__);
		return count;
	}

	if (val) {
		hs_gpio_set_value(fsa4476_pdata->gpio_en2, 1);
		hs_gpio_set_value(fsa4476_pdata->mic_gnd_switch, 1);
	} else {
		hs_gpio_set_value(fsa4476_pdata->gpio_en2, 0);
		hs_gpio_set_value(fsa4476_pdata->mic_gnd_switch, 0);
	}

	return count;
}

static DEVICE_ATTR(mic_switch, 0660, usb_ana_hs_fsa4476_mic_switch_show,
	usb_ana_hs_fsa4476_mic_switch_store);

static struct attribute *usb_ana_hs_fsa4476_attributes[] = {
	&dev_attr_mic_switch.attr,
	NULL
};

static const struct attribute_group usb_ana_hs_fsa4476_attr_group = {
	.attrs = usb_ana_hs_fsa4476_attributes,
};
#endif

static long usb_ana_hs_fsa4476_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	int ret = 0;
	int gpio_mic_sel_val;
	unsigned int __user *p_user = (unsigned int __user *)arg;

	if (!fsa4476_pdata)
		return -EBUSY;

	if (fsa4476_pdata->registed == USB_ANALOG_HS_NOT_REGISTER)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_USB_ANA_HS_GET_MIC_SWITCH_STATE:
		gpio_mic_sel_val = hs_gpio_get_value(
			fsa4476_pdata->gpio_en2);
		hwlog_info("%s gpio_mic_sel_val = %d\n",
			__func__, gpio_mic_sel_val);
		ret = put_user((__u32)(gpio_mic_sel_val), p_user);
		break;
	case IOCTL_USB_ANA_HS_GET_CONNECT_LINEIN_R_STATE:
		hwlog_info("%s connect_linein_r = %d\n",
			__func__, fsa4476_pdata->connect_linein_r);
		ret = put_user((__u32)(fsa4476_pdata->connect_linein_r),
			p_user);
		break;
	case IOCTL_USB_ANA_HS_GND_FB_CONNECT:
		hwlog_info("%s gnd fb connect\n", __func__);
#ifdef CONFIG_SUPERSWITCH_FSC
		/* SBU2 connect to HS_FB */
		if (set_superswitch_sbu_switch(Sbu_Cross_Close_Aux) < 0)
			return -EINVAL;
#endif
		usb_analog_hs_fsa4476_enable(FSA4776_ENABLE);
		hs_gpio_set_value(fsa4476_pdata->gpio_en1, 1);
		hs_gpio_set_value(fsa4476_pdata->mic_gnd_np, 0);
		break;
	case IOCTL_USB_ANA_HS_GND_FB_DISCONNECT:
		hwlog_info("%s ioctl gnd fb disconnect\n", __func__);
#ifdef CONFIG_SUPERSWITCH_FSC
		/* HS_FB disconnect */
		if (set_superswitch_sbu_switch(Sbu_None) < 0)
			return -EINVAL;
#endif
		usb_analog_hs_fsa4476_enable(FSA4776_DISABLE);
		hs_gpio_set_value(fsa4476_pdata->gpio_en1, 0);
		hs_gpio_set_value(fsa4476_pdata->mic_gnd_np, 1);
		break;
	default:
		hwlog_err("%s unsupport cmd\n", __func__);
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations usb_ana_hs_fsa4476_fops = {
	.owner               = THIS_MODULE,
	.open                = simple_open,
	.unlocked_ioctl      = usb_ana_hs_fsa4476_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl        = usb_ana_hs_fsa4476_ioctl,
#endif
};

static struct miscdevice usb_ana_hs_fsa4476_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "usb_analog_hs",
	.fops   = &usb_ana_hs_fsa4476_fops,
};

struct usb_analog_hs_ops usb_ana_hs_fsa4476_ops = {
	.usb_ana_hs_check_headset_pluged_in =
				usb_ana_hs_fsa4476_check_hs_pluged_in,
	.usb_ana_hs_dev_register = usb_ana_hs_fsa4476_dev_register,
	.check_usb_ana_hs_support = check_usb_analog_hs_fsa4476_support,
	.usb_ana_hs_plug_in_out_handle = usb_ana_hs_fsa4476_plug_in_out_handle,
	.usb_ana_hs_mic_switch_change_state =
				usb_ana_hs_fsa4476_mic_switch_change_state,
};

static int sd_gpio_config(void)
{
	unsigned int tmp_reg_value;
	unsigned long *virtual_addr = (unsigned long *)ioremap(
		SD_GPIO_SCTRL_REG, sizeof(unsigned long));

	if (fsa4476_pdata->sd_gpio_used != 1)
		return 0;
	if (!virtual_addr) {
		hwlog_err("%s sd gpio config fail\n", __func__);
		return -1;
	}
	/* BIT2: reset sd gpio status */
	tmp_reg_value = *(unsigned long *)virtual_addr | 0x4;
	*(unsigned long *)virtual_addr = tmp_reg_value;

	iounmap(virtual_addr);

	return 0;
}

static int read_property_value(struct device *dev,
	const char *property_name, int default_value)
{
	int ret;
	int value;

	ret = of_property_read_u32(dev->of_node, property_name, &value);
	if (ret) {
		hwlog_info("%s: missing %s, set default value %d\n",
			__func__, property_name, default_value);
		value = default_value;
	}
	hwlog_info("%s %s =%d\n", __func__, property_name, value);
	return value;
}

static bool read_property_setting(struct device *dev,
	const char *property_name, bool default_setting)
{
	bool setting = false;
	int value;

	if (!of_property_read_u32(dev->of_node, property_name, &value)) {
		if (value)
			setting = true;
		else
			setting = false;
	} else {
		hwlog_info("%s: missing %s, set default value %s\n", __func__,
			property_name, default_setting ? "true" : "false");
		setting = default_setting;
	}
	return setting;
}

static int usb_ana_hs_create_workqueue(void)
{
	int ret = 0;

	/* create workqueue */
	fsa4476_pdata->analog_hs_plugin_delay_wq =
		create_singlethread_workqueue("usb_analog_hs_plugin_delay_wq");
	if (!(fsa4476_pdata->analog_hs_plugin_delay_wq)) {
		hwlog_err("%s plugin wq create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_DELAYED_WORK(&fsa4476_pdata->analog_hs_plugin_delay_work,
		usb_analog_hs_plugin_work);

	fsa4476_pdata->analog_hs_plugout_delay_wq =
	    create_singlethread_workqueue("usb_analog_hs_plugout_delay_wq");
	if (!(fsa4476_pdata->analog_hs_plugout_delay_wq)) {
		hwlog_err("%s plugout wq create failed\n", __func__);
		ret = -ENOMEM;
		goto err_plugin_delay_wq;
	}
	INIT_DELAYED_WORK(&fsa4476_pdata->analog_hs_plugout_delay_work,
		usb_analog_hs_plugout_work);
	return ret;

err_plugin_delay_wq:
	if (fsa4476_pdata->analog_hs_plugin_delay_wq) {
		cancel_delayed_work(
			&fsa4476_pdata->analog_hs_plugin_delay_work);
		flush_workqueue(fsa4476_pdata->analog_hs_plugin_delay_wq);
		destroy_workqueue(fsa4476_pdata->analog_hs_plugin_delay_wq);
	}
	return ret;
}

static void usb_ana_hs_remove_workqueue(void)
{
	if (fsa4476_pdata->analog_hs_plugout_delay_wq) {
		cancel_delayed_work(
			&fsa4476_pdata->analog_hs_plugout_delay_work);
		flush_workqueue(fsa4476_pdata->analog_hs_plugout_delay_wq);
		destroy_workqueue(fsa4476_pdata->analog_hs_plugout_delay_wq);
	}
	if (fsa4476_pdata->analog_hs_plugin_delay_wq) {
		cancel_delayed_work(
			&fsa4476_pdata->analog_hs_plugin_delay_work);
		flush_workqueue(fsa4476_pdata->analog_hs_plugin_delay_wq);
		destroy_workqueue(fsa4476_pdata->analog_hs_plugin_delay_wq);
	}
}

static int usb_ana_hs_fsa4476_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node =  dev->of_node;
	int ret;

	IN_FUNCTION;
	fsa4476_pdata = kzalloc(sizeof(*fsa4476_pdata), GFP_KERNEL);
	if (!fsa4476_pdata) {
		hwlog_err("%s can not allocate ana hs dev data\n", __func__);
		return -ENOMEM;
	}
	ret = usb_ana_hs_fsa4476_load_gpio_pdata(dev);
	if (ret < 0) {
		hwlog_err("%s get gpios failed, ret =%d\n", __func__, ret);
		goto fsa4476_err_out;
	}
	ret = sd_gpio_config();
	if (ret < 0)
		goto fsa4476_err_gpio;
	fsa4476_pdata->mic_switch_delay_time = read_property_value(dev,
		"mic_switch_delay", DEFAULT_MIC_SWITCH_DELAY_TIME);
	fsa4476_pdata->connect_linein_r = read_property_setting(dev,
		"connect_linein_r", true);
	fsa4476_pdata->support_cc = read_property_setting(dev,
		"support_cc", false);
	fsa4476_pdata->using_superswitch = read_property_setting(dev,
		"using_superswitch", false);
	fsa4476_pdata->switch_antenna = read_property_setting(dev,
		"switch_antenna", false);
	wake_lock_init(&fsa4476_pdata->wake_lock, WAKE_LOCK_SUSPEND,
		"usb_analog_hs");
	mutex_init(&fsa4476_pdata->mutex);
	/* load dts config for board difference */
	load_gpio_type_config(node);
	fsa4476_pdata->registed = USB_ANALOG_HS_NOT_REGISTER;
	fsa4476_pdata->usb_analog_hs_in = USB_ANA_HS_PLUG_OUT;

	hs_gpio_set_value(fsa4476_pdata->gpio_en1, 0);
	hs_gpio_set_value(fsa4476_pdata->gpio_en2, 0);
	hs_gpio_set_value(fsa4476_pdata->mic_gnd_np, 1);
	hs_gpio_set_value(fsa4476_pdata->mic_gnd_switch, 0);
	if (fsa4476_pdata->using_superswitch)
		hs_gpio_set_value(fsa4476_pdata->gpio_enn, FSA4776_DISABLE);
	else
		hs_gpio_set_value(fsa4476_pdata->gpio_enn, FSA4776_ENABLE);
	/* create workqueue */
	ret = usb_ana_hs_create_workqueue();
	if (ret < 0)
		goto fsa4476_err_lock;
	ret = usb_analog_hs_ops_register(&usb_ana_hs_fsa4476_ops);
	if (ret) {
		hwlog_err("%s register usb_ana_hs_fsa4476_ops ops failed\n",
			__func__);
		goto fsa4476_err_misc_register;
	}
	/* register misc device for userspace */
	ret = misc_register(&usb_ana_hs_fsa4476_device);
	if (ret) {
		hwlog_err("%s misc device register failed\n", __func__);
		goto fsa4476_err_misc_register;
	}
#ifdef USB_ANALOG_HEADSET_DEBUG
	ret = sysfs_create_group(&dev->kobj, &usb_ana_hs_fsa4476_attr_group);
	if (ret < 0)
		hwlog_err("%s failed to register sysfs\n", __func__);
#endif
	hwlog_info("%s usb_analog_hs probe success\n", __func__);
	return 0;

fsa4476_err_misc_register:
	usb_ana_hs_remove_workqueue();
fsa4476_err_lock:
	wake_lock_destroy(&fsa4476_pdata->wake_lock);
fsa4476_err_gpio:
	usb_analog_hs_free_gpio();
fsa4476_err_out:
	kfree(fsa4476_pdata);
	fsa4476_pdata = NULL;

	return ret;
}

static int usb_ana_hs_fsa4476_remove(struct platform_device *pdev)
{
	hwlog_info("%s in remove\n", __func__);

	if (!fsa4476_pdata)
		return 0;

	usb_ana_hs_remove_workqueue();
	usb_analog_hs_free_gpio();
	misc_deregister(&usb_ana_hs_fsa4476_device);

#ifdef USB_ANALOG_HEADSET_DEBUG
	sysfs_remove_group(&pdev->dev.kobj, &usb_ana_hs_fsa4476_attr_group);
#endif

	kfree(fsa4476_pdata);
	fsa4476_pdata = NULL;

	return 0;
}

static struct platform_driver usb_ana_hs_fsa4476_driver = {
	.driver = {
		.name   = "usb_ana_hs_fsa4476",
		.owner  = THIS_MODULE,
		.of_match_table = usb_ana_hs_fsa4476_of_match,
	},
	.probe  = usb_ana_hs_fsa4476_probe,
	.remove = usb_ana_hs_fsa4476_remove,
};

static int __init usb_ana_hs_fsa4476_init(void)
{
	return platform_driver_register(&usb_ana_hs_fsa4476_driver);
}

static void __exit usb_ana_hs_fsa4476_exit(void)
{
	platform_driver_unregister(&usb_ana_hs_fsa4476_driver);
}

subsys_initcall_sync(usb_ana_hs_fsa4476_init);
module_exit(usb_ana_hs_fsa4476_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb analog headset switch fsa4476 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
