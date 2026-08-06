/*
 * usb_analog_hs_isl54405.c
 *
 * usb analog headset isl54405 driver
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
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/wakelock.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/hisi/hi64xx/hi64xx_mbhc.h>
#include <huawei_platform/log/hw_log.h>
#include "huawei_platform/audio/usb_analog_hs_isl54405.h"

#define HWLOG_TAG usb_ana_hs_isl54405
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define MAX_BUF_SIZE                    32
#define HIGH                            1
#define LOW                             0
#define WAKE_LOCK_TIMEOUT               1000 /* ms */

struct usb_ana_hs_isl54405_data {
	int gpio_type;
	int gpio_switch_1v8_en;
	int gpio_mic_switch;
	int gpio_usb_hs_switch;
	/* usb analog headset dev register flag */
	int registed;
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
};

static struct usb_ana_hs_isl54405_data *isl54405_pdata;

static inline int usb_analog_hs_gpio_get_value(int gpio)
{
	if (isl54405_pdata->gpio_type == USB_ANALOG_HS_GPIO_CODEC)
		return gpio_get_value_cansleep(gpio);
	return gpio_get_value(gpio);
}

static inline void usb_analog_hs_gpio_set_value(int gpio, int value)
{
	if (isl54405_pdata->gpio_type == USB_ANALOG_HS_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static void usb_analog_hs_plugin_work(struct work_struct *work)
{
	enum hisi_jack_states hs_type = HISI_JACK_NONE;

	IN_FUNCTION;

	wake_lock(&isl54405_pdata->wake_lock);
	isl54405_pdata->codec_ops_dev->ops.plug_in_detect(
		isl54405_pdata->private_data);
	mutex_lock(&isl54405_pdata->mutex);
	isl54405_pdata->usb_analog_hs_in = USB_ANA_HS_PLUG_IN;
	hs_type = isl54405_pdata->codec_ops_dev->ops.get_headset_type(
		isl54405_pdata->private_data);

	hwlog_info("%s hs_type =%d\n", __func__, hs_type);
	if (hs_type == HISI_JACK_HEADSET || hs_type == HISI_JACK_HEADPHONE ||
		hs_type == HISI_JACK_INVERT)
		usb_analog_hs_gpio_set_value(
			isl54405_pdata->gpio_usb_hs_switch, 0);

	mutex_unlock(&isl54405_pdata->mutex);
	wake_unlock(&isl54405_pdata->wake_lock);

	OUT_FUNCTION;
}

static void usb_analog_hs_plugout_work(struct work_struct *work)
{
	IN_FUNCTION;

	wake_lock(&isl54405_pdata->wake_lock);
	if (isl54405_pdata->usb_analog_hs_in == USB_ANA_HS_PLUG_IN) {
		hwlog_info("%s usb analog hs plug out act\n", __func__);
		isl54405_pdata->codec_ops_dev->ops.plug_out_detect(
			isl54405_pdata->private_data);
		mutex_lock(&isl54405_pdata->mutex);
		isl54405_pdata->usb_analog_hs_in = USB_ANA_HS_PLUG_OUT;
		usb_analog_hs_gpio_set_value(
			isl54405_pdata->gpio_usb_hs_switch, 1);
		mutex_unlock(&isl54405_pdata->mutex);
	}
	wake_unlock(&isl54405_pdata->wake_lock);

	OUT_FUNCTION;
}

int usb_ana_hs_isl54405_dev_register(struct usb_analog_hs_dev *dev,
					void *codec_data)
{
	/* usb analog headset driver not be probed, just return */
	if (!isl54405_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return -ENODEV;
	}

	/* only support one codec to be registed */
	if (isl54405_pdata->registed == USB_ANALOG_HS_ALREADY_REGISTER) {
		hwlog_err("%s more than one codec regist permit\n", __func__);
		return -EEXIST;
	}

	if (!dev->ops.plug_in_detect || !dev->ops.plug_out_detect ||
		!dev->ops.get_headset_type) {
		hwlog_err("%s codec ops function check fail\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&isl54405_pdata->mutex);
	isl54405_pdata->codec_ops_dev = dev;
	isl54405_pdata->private_data = codec_data;
	isl54405_pdata->registed = USB_ANALOG_HS_ALREADY_REGISTER;
	mutex_unlock(&isl54405_pdata->mutex);

	hwlog_err("%s usb analog hs has been register sucessful\n", __func__);

	return 0;
}

int usb_ana_hs_isl54405_check_hs_pluged_in(void)
{
	int typec_detach = PD_DPM_TYPEC_UNATTACHED;

	pd_dpm_get_typec_state(&typec_detach);

	hwlog_info("%s typec_detach =%d\n", __func__, typec_detach);

	if (typec_detach == PD_DPM_USB_TYPEC_AUDIO_ATTACHED)
		return USB_ANA_HS_PLUG_IN;
	return USB_ANA_HS_PLUG_OUT;
}

void usb_ana_hs_isl54405_mic_switch_change_state(void)
{
	int mic_sel_val;

	/* usb analog headset driver not be probed, just return */
	if (!isl54405_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}

	if (isl54405_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}

	IN_FUNCTION;

	mic_sel_val = usb_analog_hs_gpio_get_value(
		isl54405_pdata->gpio_mic_switch);
	hwlog_info("%s gpio mic sel is %d\n", __func__, mic_sel_val);
	mic_sel_val = mic_sel_val ? 0 : 1;
	hwlog_info("%s gpio mic sel will set to %d\n", __func__, mic_sel_val);

	usb_analog_hs_gpio_set_value(
		isl54405_pdata->gpio_mic_switch, mic_sel_val);
	mic_sel_val = usb_analog_hs_gpio_get_value(
		isl54405_pdata->gpio_mic_switch);
	hwlog_info("%s gpio mic sel change to %d\n", __func__, mic_sel_val);

	OUT_FUNCTION;
}

void usb_ana_hs_isl54405_plug_in_out_handle(int hs_state)
{
	/* usb analog headset driver not be probed, just return */
	if (!isl54405_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return;
	}

	if (isl54405_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s codec_ops_dev is not registed\n", __func__);
		return;
	}
	IN_FUNCTION;

	hwlog_info("%s hs_state is %d[%s]\n", __func__, hs_state,
		(hs_state == USB_ANA_HS_PLUG_IN) ? "PLUG_IN" : "PLUG_OUT");

	wake_lock_timeout(&isl54405_pdata->wake_lock,
		msecs_to_jiffies(WAKE_LOCK_TIMEOUT));

	if (hs_state == USB_ANA_HS_PLUG_IN)
		queue_delayed_work(isl54405_pdata->analog_hs_plugin_delay_wq,
			&isl54405_pdata->analog_hs_plugin_delay_work,
			msecs_to_jiffies(800)); /* delay for status stable */
	else
		queue_delayed_work(isl54405_pdata->analog_hs_plugout_delay_wq,
			&isl54405_pdata->analog_hs_plugout_delay_work, 0);
	OUT_FUNCTION;
}

bool check_usb_analog_hs_isl54405_support(void)
{
	if (!isl54405_pdata ||
		(isl54405_pdata->registed == USB_ANALOG_HS_NOT_REGISTER))
		return false;

	return true;
}

static const struct of_device_id usb_ana_hs_isl54405_of_match[] = {
	{ .compatible = "huawei,usb_ana_hs_isl54405", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_ana_hs_isl54405_of_match);

/* load dts config for board difference */
static void load_gpio_type_config(struct device_node *node)
{
	unsigned int temp = USB_ANALOG_HS_GPIO_SOC;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		isl54405_pdata->gpio_type = temp;
	else
		isl54405_pdata->gpio_type = USB_ANALOG_HS_GPIO_SOC;
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

	if (!isl54405_pdata)
		return;

	if (isl54405_pdata->gpio_switch_1v8_en > 0)
		gpio_free(isl54405_pdata->gpio_switch_1v8_en);

	if (isl54405_pdata->gpio_mic_switch > 0)
		gpio_free(isl54405_pdata->gpio_mic_switch);

	if (isl54405_pdata->gpio_usb_hs_switch > 0)
		gpio_free(isl54405_pdata->gpio_usb_hs_switch);
}

static int usb_ana_hs_isl54405_load_gpio_pdata(struct device *dev)
{
	/* get switch 1v8 control gpio */
	if (usb_ana_hs_load_gpio(dev, &(isl54405_pdata->gpio_switch_1v8_en),
		HIGH, "swtich_1v8_en") < 0)
		goto isl54405_get_gpio_err;

	/* get mic sel control gpio */
	if (usb_ana_hs_load_gpio(dev, &(isl54405_pdata->gpio_mic_switch),
		HIGH, "gpio_mic_switch") < 0)
		goto isl54405_get_gpio_err;

	/* get usb hs switch control gpio */
	if (usb_ana_hs_load_gpio(dev, &(isl54405_pdata->gpio_usb_hs_switch),
		HIGH, "gpio_usb_hs_switch") < 0)
		goto isl54405_get_gpio_err;

	return 0;

isl54405_get_gpio_err:
	usb_analog_hs_free_gpio();
	return -ENOMEM;
}

#ifdef USB_ANALOG_HS_ISL54405_DEBUG
static ssize_t usb_ana_hs_isl54405_mic_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int value;

	if (!isl54405_pdata) {
		hwlog_err("%s pdata is NULL\n", __func__);
		return 0;
	}

	if (isl54405_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s pdata is not registed\n", __func__);
		return 0;
	}

	value = usb_analog_hs_gpio_get_value(isl54405_pdata->gpio_mic_switch);

	return scnprintf(buf, MAX_BUF_SIZE, "%d\n", value);
}

static ssize_t usb_ana_hs_isl54405_mic_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long val;

	if (!isl54405_pdata) {
		hwlog_err("%s pdata is NULL or not registed\n", __func__);
		return count;
	}

	if (isl54405_pdata->registed == USB_ANALOG_HS_NOT_REGISTER) {
		hwlog_err("%s pdata is NULL or not registed\n", __func__);
		return count;
	}

	ret = kstrtol(buf, 10, &val);
	if (ret < 0) {
		hwlog_err("%s input error\n", __func__);
		return count;
	}

	if (val)
		usb_analog_hs_gpio_set_value(
			isl54405_pdata->gpio_mic_switch, 1);
	else
		usb_analog_hs_gpio_set_value(
			isl54405_pdata->gpio_mic_switch, 0);

	return count;
}

static DEVICE_ATTR(mic_switch, 0660, usb_ana_hs_isl54405_mic_switch_show,
		usb_ana_hs_isl54405_mic_switch_store);

static struct attribute *usb_ana_hs_isl54405_attributes[] = {
	&dev_attr_mic_switch.attr,
	NULL
};

static const struct attribute_group usb_ana_hs_isl54405_attr_group = {
	.attrs = usb_ana_hs_isl54405_attributes,
};
#endif

static long usb_ana_hs_isl54405_ioctl(struct file *file, unsigned int cmd,
					unsigned long arg)
{
	int ret;
	int gpio_mic_sel_val;
	unsigned int __user *p_user = (unsigned int __user *) arg;

	if (!isl54405_pdata)
		return -EBUSY;

	if (isl54405_pdata->registed == USB_ANALOG_HS_NOT_REGISTER)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_USB_ANA_HS_GET_MIC_SWITCH_STATE:
		gpio_mic_sel_val = usb_analog_hs_gpio_get_value(
			isl54405_pdata->gpio_mic_switch);
		hwlog_info("%s gpio_mic_sel_val = %d\n",
			__func__, gpio_mic_sel_val);
		ret = put_user((__u32)(gpio_mic_sel_val), p_user);
		break;
	default:
		hwlog_err("%s unsupport cmd\n", __func__);
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static void usb_ana_hs_isl54405_remove_workqueue(void)
{
	if (isl54405_pdata->analog_hs_plugout_delay_wq) {
		cancel_delayed_work(
			&isl54405_pdata->analog_hs_plugout_delay_work);
		flush_workqueue(isl54405_pdata->analog_hs_plugout_delay_wq);
		destroy_workqueue(isl54405_pdata->analog_hs_plugout_delay_wq);
	}
	if (isl54405_pdata->analog_hs_plugin_delay_wq) {
		cancel_delayed_work(
			&isl54405_pdata->analog_hs_plugin_delay_work);
		flush_workqueue(isl54405_pdata->analog_hs_plugin_delay_wq);
		destroy_workqueue(isl54405_pdata->analog_hs_plugin_delay_wq);
	}
}

static int usb_ana_hs_isl54405_create_workqueue(void)
{
	int ret = 0;

	/* create workqueue */
	isl54405_pdata->analog_hs_plugin_delay_wq =
		create_singlethread_workqueue("usb_analog_hs_plugin_delay_wq");
	if (!(isl54405_pdata->analog_hs_plugin_delay_wq)) {
		hwlog_err("%s plugin wq create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_DELAYED_WORK(&isl54405_pdata->analog_hs_plugin_delay_work,
		usb_analog_hs_plugin_work);

	isl54405_pdata->analog_hs_plugout_delay_wq =
		create_singlethread_workqueue("usb_analog_hs_plugout_delay_wq");
	if (!(isl54405_pdata->analog_hs_plugout_delay_wq)) {
		hwlog_err("%s plugout wq create failed\n", __func__);
		ret = -ENOMEM;
		goto isl54405_err_create_wq;
	}
	INIT_DELAYED_WORK(&isl54405_pdata->analog_hs_plugout_delay_work,
		usb_analog_hs_plugout_work);
	return ret;
isl54405_err_create_wq:
	usb_ana_hs_isl54405_remove_workqueue();
	return ret;
}

static const struct file_operations usb_ana_hs_isl54405_fops = {
	.owner               = THIS_MODULE,
	.open                = simple_open,
	.unlocked_ioctl      = usb_ana_hs_isl54405_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl        = usb_ana_hs_isl54405_ioctl,
#endif
};

static struct miscdevice usb_ana_hs_isl54405_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "usb_analog_hs",
	.fops   = &usb_ana_hs_isl54405_fops,
};

struct usb_analog_hs_ops usb_ana_hs_isl54405_ops = {
	.usb_ana_hs_check_headset_pluged_in =
		usb_ana_hs_isl54405_check_hs_pluged_in,
	.usb_ana_hs_dev_register = usb_ana_hs_isl54405_dev_register,
	.check_usb_ana_hs_support = check_usb_analog_hs_isl54405_support,
	.usb_ana_hs_plug_in_out_handle =
		usb_ana_hs_isl54405_plug_in_out_handle,
	.usb_ana_hs_mic_switch_change_state =
		usb_ana_hs_isl54405_mic_switch_change_state,
};

static int usb_ana_hs_isl54405_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node =  dev->of_node;
	int ret;

	IN_FUNCTION;

	isl54405_pdata = kzalloc(sizeof(*isl54405_pdata), GFP_KERNEL);
	if (!isl54405_pdata)
		return -ENOMEM;

	ret = usb_ana_hs_isl54405_load_gpio_pdata(dev);
	if (ret < 0) {
		hwlog_err("%s get gpios failed, ret =%d\n", __func__, ret);
		goto isl54405_err_out;
	}

	wake_lock_init(&isl54405_pdata->wake_lock, WAKE_LOCK_SUSPEND,
		"usb_analog_hs");
	mutex_init(&isl54405_pdata->mutex);

	/* load dts config for board difference */
	load_gpio_type_config(node);
	isl54405_pdata->registed = USB_ANALOG_HS_NOT_REGISTER;
	isl54405_pdata->usb_analog_hs_in = USB_ANA_HS_PLUG_OUT;
	usb_analog_hs_gpio_set_value(isl54405_pdata->gpio_switch_1v8_en, 1);
	usb_analog_hs_gpio_set_value(isl54405_pdata->gpio_usb_hs_switch, 1);

	/* create workqueue */
	ret = usb_ana_hs_isl54405_create_workqueue();
	if (ret)
		goto isl54405_err_lock;

	ret = usb_analog_hs_ops_register(&usb_ana_hs_isl54405_ops);
	if (ret) {
		hwlog_err("%s register ops failed\n", __func__);
		goto isl54405_err_misc_register;
	}

	/* register misc device for userspace */
	ret = misc_register(&usb_ana_hs_isl54405_device);
	if (ret) {
		hwlog_err("%s misc device register failed\n", __func__);
		goto isl54405_err_misc_register;
	}

#ifdef USB_ANALOG_HS_ISL54405_DEBUG
	ret = sysfs_create_group(&dev->kobj, &usb_ana_hs_isl54405_attr_group);
	if (ret < 0)
		hwlog_err("%s failed to register sysfs\n", __func__);
#endif

	hwlog_info("%s usb_analog_hs probe success\n", __func__);
	return 0;

isl54405_err_misc_register:
	usb_ana_hs_isl54405_remove_workqueue();
isl54405_err_lock:
	wake_lock_destroy(&isl54405_pdata->wake_lock);
isl54405_err_out:
	kfree(isl54405_pdata);
	isl54405_pdata = NULL;

	return ret;
}

static int usb_ana_hs_isl54405_remove(struct platform_device *pdev)
{
	hwlog_info("%s in remove\n", __func__);

	if (!isl54405_pdata)
		return 0;

	usb_ana_hs_isl54405_remove_workqueue();

	usb_analog_hs_free_gpio();
	misc_deregister(&usb_ana_hs_isl54405_device);

#ifdef USB_ANALOG_HS_ISL54405_DEBUG
	sysfs_remove_group(&pdev->dev.kobj, &usb_ana_hs_isl54405_attr_group);
#endif

	kfree(isl54405_pdata);
	isl54405_pdata = NULL;

	return 0;
}

static struct platform_driver usb_ana_hs_isl54405_driver = {
	.driver = {
		.name   = "usb_ana_hs_isl54405",
		.owner  = THIS_MODULE,
		.of_match_table = usb_ana_hs_isl54405_of_match,
	},
	.probe  = usb_ana_hs_isl54405_probe,
	.remove = usb_ana_hs_isl54405_remove,
};

static int __init usb_ana_hs_isl5405_init(void)
{
	return platform_driver_register(&usb_ana_hs_isl54405_driver);
}

static void __exit usb_ana_hs_isl5405_exit(void)
{
	platform_driver_unregister(&usb_ana_hs_isl54405_driver);
}

subsys_initcall_sync(usb_ana_hs_isl5405_init);
module_exit(usb_ana_hs_isl5405_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb analog headset switch isl5405 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
