/*
 * hw_usb_hub.c
 *
 * usb hub driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/usb.h>
#include <linux/hisi/usb/tca.h>
#include <linux/io.h>
#include <linux/device.h>
#include "hw_usb_hub.h"
#include <linux/pinctrl/consumer.h>

#define USBHUB_BPWR_HOLD_TIME 200
#define DELAT_TIME          10
#define MAX_HUB_RESET_TIMES 3
#define MAX_INPUT_SIZE 10

#define EC_ADDR	0x02B2
#define EC_ADDR_PV	0x06B2
#define HUB_ON	0x65
#define PORT1_ON	0x5F
#define PORT2_ON	0x61
#define EC_VDD_USBHUB_LGA_EN_ON	0x66

#define HUB_OFF 0x64
#define PORT1_OFF	0x5E
#define PORT2_OFF	0x60
#define EC_VDD_USBHUB_LGA_EN_OFF	0x67

enum {
	PRODUCT_KU = 0,  //Kelvinu
	PRODUCT_PV_0 = 1,//M536 HW V1-V3
	PRODUCT_PV_1 = 2,//M536 HW V4 and later
};

static int g_hub_gpio = 0;
static int g_hub_reset_times = 0;
static int g_product_type = 0;
static int g_back_hub_reset_gpio = 0;

static struct class *usb_hub_port_class;
static struct device *usb_hub_port_dev;

extern int echub_i2c_write_regulator(unsigned short regoffset, char regdata);

static void reset_usb_hub(void)
{
	gpio_direction_output(g_hub_gpio, 1);
}

static void dereset_usb_hub(void)
{
	gpio_direction_output(g_hub_gpio, 0);
}

static void hw_usb_hub_power_on(void)
{
	int ret, port1, port2;
	ret = echub_i2c_write_regulator(EC_ADDR, HUB_ON);
	if (ret) {
		usbhub_err("hub power on error\n");
	} else {
		usbhub_err("hub power on succeed\n");
	}

	port1 = echub_i2c_write_regulator(EC_ADDR, PORT1_ON);
	mdelay(DELAT_TIME);
	if (port1) {
		usbhub_err("echub_i2c_write_5V_port1_on error!\n");
	} else {
		usbhub_err("echub_i2c_write_5V_port1_on succeed!\n");
	}

	/* USB Port_2 5V power on */
	port2 = echub_i2c_write_regulator(EC_ADDR, PORT2_ON);
	if (port2) {
		usbhub_err("echub_i2c_write_5V_port2_on error!\n");
	} else {
		usbhub_err("echub_i2c_write_5V_port2_on succeed!\n");
	}

	usbhub_err("usb-hub-port power_on_probe -\n");
	return;
}

static void hw_usb_hub_power_on_pv(void)
{
	int lga;
	lga = echub_i2c_write_regulator(EC_ADDR_PV, EC_VDD_USBHUB_LGA_EN_ON);
	if (lga) {
		usbhub_err("EC_+VDD_USBHUB_LGA_EN_ON pv error!\n");
	} else {
		usbhub_err("EC_+VDD_USBHUB_LGA_EN_ON pv succeed!\n");
	}

	usbhub_err("usb-hub-LGA power_on_probe -\n");
	return;
}

static void hw_usb_hub_power_off(void)
{
	int ret, port1, port2;
	ret = echub_i2c_write_regulator(EC_ADDR, HUB_OFF);
	if (ret) {
		usbhub_info("hub power off error\n");
	} else {
		usbhub_info("hub power off succeed\n");
	}

	port1 = echub_i2c_write_regulator(EC_ADDR, PORT1_OFF);
	mdelay(DELAT_TIME);
	if (port1) {
		usbhub_info("echub_i2c_write_5V_port1_off error!\n");
	} else {
		usbhub_info("echub_i2c_write_5V_port1_off succeed!\n");
	}

	/* USB Port_2 5V power on */
	port2 = echub_i2c_write_regulator(EC_ADDR, PORT2_OFF);
	if (port2) {
		usbhub_info("echub_i2c_write_5V_port2_off error!\n");
	} else {
		usbhub_info("echub_i2c_write_5V_port2_off succeed!\n");
	}

	usbhub_info("usb-hub-port power_off_probe -\n");
	return;
}

static void hw_usb_hub_power_off_pv(void)
{
	int lga;
	lga = echub_i2c_write_regulator(EC_ADDR_PV, EC_VDD_USBHUB_LGA_EN_OFF);
	if (lga) {
		usbhub_info("EC_+VDD_USBHUB_LGA_EN_OFF pv error\n");
	} else {
		usbhub_info("EC_+VDD_USBHUB_LGA_EN_OFF pv succeed\n");
	}

	usbhub_info("usb-hub-LGA power_off_probe -\n");
	return;
}

static void force_usb_reset(void)
{
	/* set high to reset usb hub device.
	* because of different hw design,ku reset:0;pv reset:1.
	*/
	if (g_product_type == PRODUCT_KU) {
		hw_usb_hub_power_off();
		reset_usb_hub();
	}else if ((g_product_type == PRODUCT_PV_1) || (g_product_type == PRODUCT_PV_0)) {
		hw_usb_hub_power_off_pv();
		gpio_direction_output(g_back_hub_reset_gpio,0);
	}
	mdelay(USBHUB_BPWR_HOLD_TIME);

	// plug out
	usbhub_info("force usb hub plug out\n");
	pd_event_notify(TCA_IRQ_HPD_OUT, TCPC_USB31_AND_DP_2LINE, TCA_ID_RISE_EVENT, TYPEC_ORIEN_POSITIVE);
	pd_event_notify(TCA_IRQ_HPD_OUT, TCPC_USB31_AND_DP_2LINE, TCA_DP_OUT, TYPEC_ORIEN_POSITIVE);

	// plug in
	usbhub_info("force usb hub plug in\n");
	pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_AND_DP_2LINE, TCA_ID_FALL_EVENT, TYPEC_ORIEN_POSITIVE);
	pd_event_notify(TCA_IRQ_HPD_IN, TCPC_USB31_AND_DP_2LINE, TCA_DP_IN, TYPEC_ORIEN_POSITIVE);
}


static int usbhub_dev_notify(struct notifier_block *self, unsigned long action, void *dev)
{
	struct usb_bus *bus = NULL;

	switch (action) {
	case USB_BUS_ADD:
		bus = (struct usb_bus *)dev;
		usbhub_info("USB_BUS_ADD busnum=%d\n", bus->busnum);

		/* ku:set low to de-reset usb hub device
		*/
		if ((g_product_type == PRODUCT_KU) && (bus->busnum == 1)) {
		    dereset_usb_hub();
		    hw_usb_hub_power_on();
		}else if ((g_product_type == PRODUCT_PV_0) || (g_product_type == PRODUCT_PV_1) && (bus->busnum == 3)) {
		    gpio_direction_output(g_back_hub_reset_gpio,1);
		    hw_usb_hub_power_on_pv();
		}
		break;
	case USB_BUS_REMOVE:
		bus = (struct usb_bus *)dev;
		usbhub_info("USB_BUS_REMOVE busnum=%d\n", bus->busnum);
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block g_usbhub_dev_nb = {
	.notifier_call = usbhub_dev_notify,
};


static ssize_t hub_port_reset_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t val;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, MAX_INPUT_SIZE, &val))) {
		usbhub_info("invaild parameters\n");
		return -EINVAL;
	}

	if (val == 1) {
		if (g_hub_reset_times < MAX_HUB_RESET_TIMES) {
			force_usb_reset();
			g_hub_reset_times++;
		}

		usbhub_info("hub_port_reset\n");
	}

	return count;
}

static DEVICE_ATTR(hub_port_reset, 0664, NULL, hub_port_reset_store);


static struct attribute *usb_hub_port_ctrl_attributes[] = {
	&dev_attr_hub_port_reset.attr,
	NULL
};

static const struct attribute_group usb_hub_port_attr_group = {
	.attrs = usb_hub_port_ctrl_attributes,
};

static int back_hub_gpio_pin_init(struct platform_device *pdev)
{
	int ret = 0;
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrl_default;
	struct pinctrl_state *pinctrl_idle;

	pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(pinctrl);
		usbhub_err("pinctrl get error ++\n");
		return ret;
	}

	pinctrl_default =pinctrl_lookup_state(pinctrl, "default");
	if (IS_ERR(pinctrl_default))
		usbhub_err("could not get default pinstate\n");

	pinctrl_idle = pinctrl_lookup_state(pinctrl, "idle");
	if (IS_ERR(pinctrl_idle))
		usbhub_err("could not get sleep pinstate\n");

	ret = pinctrl_select_state(pinctrl, pinctrl_default);
	if (ret)
		usbhub_err("could not set pins to default state\n");

	return ret;
}

static int back_hub_gpio_init(struct platform_device *pdev, struct device_node *np)
{
	int ret = 0;

	back_hub_gpio_pin_init(pdev);

	ret = of_property_read_u32(np, "back_hub_reset_gpio", (u32 *)&g_back_hub_reset_gpio);
	if (ret) {
		usbhub_err("get back hub reset_gpio failed\n");
		return -EINVAL;
	}

	ret = gpio_request(g_back_hub_reset_gpio, "back_hub_reset_gpio");
	if (ret) {
		usbhub_err("request back hub reset_gpio=%d failed\n", g_back_hub_reset_gpio);
		return -EINVAL;
	}

	usbhub_info("get back_hub_reset_gpio=%d succ\n", g_back_hub_reset_gpio);
	return ret;
}

static int usb_hub_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	usbhub_info("usb hub probe ++\n");
	ret = of_property_read_u32(np, "product_type", (u32 *)&g_product_type);
	if (ret) {
		usbhub_err("get product_type failed\n");
		g_product_type = 0;
	}

	ret = of_property_read_u32(np, "hub_bpwr_gpio", (u32 *)&g_hub_gpio);
	if (ret) {
		usbhub_err("get pd_rst_gpio failed\n");
		return -EINVAL;
	}

	ret = gpio_request(g_hub_gpio, "hub_bpwr_gpio");
	if (ret) {
		usbhub_err("get hub_bpwr_gpio=%d failed\n", g_hub_gpio);
		return -EINVAL;
	}

	usbhub_info("get hub_bpwr_gpio=%d product_type=%d succ\n", g_hub_gpio, g_product_type);

	/* set high to reset usb hub device.
	 * because of different hw design,ku reset:0;pv reset:1.
	 */
	if (g_product_type == PRODUCT_KU) {
		hw_usb_hub_power_off();
		reset_usb_hub();
	}else if ((g_product_type == PRODUCT_PV_1) || (g_product_type == PRODUCT_PV_0)) {
		hw_usb_hub_power_off_pv();
		ret = back_hub_gpio_init(pdev,np);
		if (ret) {
			usbhub_err("get g_back_hub_reset_gpio failed\n");
			return -EINVAL;
		}
		gpio_direction_output(g_back_hub_reset_gpio,0);
	}

	usb_register_notify(&g_usbhub_dev_nb);

	usb_hub_port_class = class_create(THIS_MODULE, "usb_hub_port");
	if (IS_ERR(usb_hub_port_class)) {
		usbhub_err("cannot create class\n");
		ret = PTR_ERR(usb_hub_port_class);
		goto fail_create_class;
	}

	if (usb_hub_port_class) {
		usb_hub_port_dev = device_create(usb_hub_port_class, NULL, 0, NULL, "port");
		if (IS_ERR(usb_hub_port_dev)) {
			usbhub_err("sysfs device create failed\n");
			ret = PTR_ERR(usb_hub_port_dev);
			goto fail_create_device;
		}

		ret = sysfs_create_group(&usb_hub_port_dev->kobj, &usb_hub_port_attr_group);
		if (ret) {
			usbhub_err("sysfs group create failed\n");
			goto fail_create_sysfs;
		}
	}

	usbhub_info("usb hub probe --\n");

fail_create_sysfs:
	usb_hub_port_dev = NULL;

fail_create_device:
	usb_hub_port_class = NULL;

fail_create_class:

	return ret;
}

static int usb_hub_remove(struct platform_device *pdev)
{
	usbhub_info("usb hub remove\n");
	if (g_hub_gpio)
		gpio_free(g_hub_gpio);

	if (g_back_hub_reset_gpio)
		gpio_free(g_back_hub_reset_gpio);

	usb_unregister_notify(&g_usbhub_dev_nb);

	sysfs_remove_group(&usb_hub_port_dev->kobj, &usb_hub_port_attr_group);
	usb_hub_port_dev = NULL;
	usb_hub_port_class = NULL;

	return 0;
}

static const struct of_device_id hw_usb_hub_match[] = {
	{ .compatible = "huawei,hw_usb_hub" },
	{},
};


static struct platform_driver usb_hub_driver = {
	.probe          = usb_hub_probe,
	.remove         = usb_hub_remove,
	.driver         = {
		.name   = "hw_usb_hub",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_usb_hub_match),
		},
};

static int __init hw_usb_hub_init(void)
{
	return platform_driver_register(&usb_hub_driver);
}

static void __exit hw_usb_hub_exit(void)
{
	platform_driver_unregister(&usb_hub_driver);
}

fs_initcall(hw_usb_hub_init);
module_exit(hw_usb_hub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei usb hub module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
