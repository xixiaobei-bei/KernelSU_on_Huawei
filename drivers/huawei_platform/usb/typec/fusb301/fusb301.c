/*
 * fusb301.c
 *
 * driver for Fairchild fusb301 typec chip
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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/param.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "fusb301.h"
#include <linux/reboot.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG typec_fusb301
HWLOG_REGIST();

static struct notifier_block reboot_nb;
struct typec_device_info *g_fusb301_dev;

static int fusb301_i2c_read(struct typec_device_info *di, u8 reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(di->client, reg);
	if (ret < 0)
		hwlog_err("read byte error reg=%x,%d\n", reg, ret);

	return ret;
}

static int fusb301_i2c_write(struct typec_device_info *di, u8 reg, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(di->client, reg, val);
	if (ret < 0)
		hwlog_err("write byte error reg=%x,%d\n", reg, ret);

	return ret;
}

static int fusb301_read_reg(u8 reg, u8 *val)
{
	int ret;
	struct typec_device_info *di = g_fusb301_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fusb301_i2c_read(di, reg);
	if (ret < 0)
		return ret;

	*val = ret;
	return ret;
}

static int fusb301_write_reg(u8 reg, u8 val)
{
	struct typec_device_info *di = g_fusb301_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return fusb301_i2c_write(di, reg, val);
}

static int fusb301_clean_mask(void)
{
	u8 reg_val = 0;
	int ret;

	ret = fusb301_read_reg(FUSB301_REG_CONTROL, &reg_val);
	if (ret < 0)
		return ret;

	reg_val &= ~FUSB301_REG_INT_MASK;
	ret = fusb301_write_reg(FUSB301_REG_CONTROL, reg_val);
	if (ret < 0)
		return ret;

	return 0;
}

static int fusb301_device_check(void)
{
	u8 reg_val = 0;

	return fusb301_read_reg(FUSB301_REG_DEVICE_ID, &reg_val);
}

static int fusb301_host_current_mode(u8 val)
{
	u8 reg_val = 0;
	u8 mask_val;
	u8 current_set_val;
	int ret;

	ret = fusb301_read_reg(FUSB301_REG_CONTROL, &reg_val);
	if (ret < 0)
		return ret;

	mask_val = reg_val & (~FUSB301_REG_HOST_CUR_MODE);
	current_set_val = reg_val & FUSB301_REG_HOST_CUR_MODE;

	if (val == current_set_val) {
		hwlog_info("current mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = fusb301_write_reg(FUSB301_REG_CONTROL, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int fusb301_ctrl_output_current(int value)
{
	switch (value) {
	case TYPEC_HOST_CURRENT_DEFAULT:
		hwlog_info("set to default current\n");
		fusb301_host_current_mode(FUSB301_REG_HOST_CUR_DEFAULT);
		break;

	case TYPEC_HOST_CURRENT_MID:
		hwlog_info("set to medium current\n");
		fusb301_host_current_mode(FUSB301_REG_HOST_CUR_MID);
		break;

	case TYPEC_HOST_CURRENT_HIGH:
		hwlog_info("set to high current\n");
		fusb301_host_current_mode(FUSB301_REG_HOST_CUR_HIGH);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static int fusb301_host_port_mode(u8 val)
{
	u8 reg_val = 0;
	int ret;

	ret = fusb301_read_reg(FUSB301_REG_PORT_MODE, &reg_val);
	if (ret < 0)
		return ret;

	if (val == reg_val) {
		hwlog_info("port mode is same as setting\n");
		return 0;
	}

	ret = fusb301_write_reg(FUSB301_REG_PORT_MODE, val);
	if (ret < 0)
		return ret;

	return 0;
}

static void fusb301_set_port_state(int value)
{
	mdelay(1); /* delay 1ms */

	switch (value) {
	case TYPEC_STATE_UNATTACHED_SINK:
		hwlog_info("set to unattached.snk\n");
		fusb301_write_reg(FUSB301_REG_MANUAL,
			FUSB301_REG_MANUAL_UNATTACHED_SINK);
		break;

	case TYPEC_STATE_UNATTACHED_SOURCE:
		hwlog_info("set to unattached.src\n");
		fusb301_write_reg(FUSB301_REG_MANUAL,
			FUSB301_REG_MANUAL_UNATTACHED_SOURCE);
		break;

	case TYPEC_STATE_UNATTACHED_DISABLED:
		hwlog_info("set to disabled\n");
		fusb301_write_reg(FUSB301_REG_MANUAL,
			FUSB301_REG_MANUAL_UNATTACHED_DISABLED);
		break;

	case TYPEC_STATE_UNATTACHED_ERRORRECOVERY:
		hwlog_info("set to error recovery\n");
		fusb301_write_reg(FUSB301_REG_MANUAL,
			FUSB301_REG_MANUAL_UNATTACHED_ERRORRECOVERY);
		break;

	default:
		hwlog_err("wrong input action\n");
		break;
	}
}

static int fusb301_ctrl_port_mode(int value)
{
	switch (value) {
	case TYPEC_HOST_PORT_MODE_DFP:
		hwlog_info("set to dfp mode\n");
		fusb301_host_port_mode(FUSB301_REG_SET_DFP);
		fusb301_set_port_state(TYPEC_STATE_UNATTACHED_SOURCE);
		break;

	case TYPEC_HOST_PORT_MODE_UFP:
		hwlog_info("set to ufp mode\n");
		fusb301_host_port_mode(FUSB301_REG_SET_UFP);
		fusb301_set_port_state(TYPEC_STATE_UNATTACHED_DISABLED);
		mdelay(DISABLE_DURATION_MS);
		fusb301_host_port_mode(FUSB301_REG_SET_UFP);
		fusb301_set_port_state(TYPEC_STATE_UNATTACHED_SINK);
		break;

	case TYPEC_HOST_PORT_MODE_DRP:
		hwlog_info("set to drp mode\n");
		fusb301_host_port_mode(FUSB301_REG_SET_DRP);
		fusb301_set_port_state(TYPEC_STATE_UNATTACHED_ERRORRECOVERY);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static int fusb301_detect_attachment_status(void)
{
	int ret;
	u8 reg_val = 0;
	struct typec_device_info *di = g_fusb301_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fusb301_read_reg(FUSB301_REG_INT, &reg_val);
	if (ret < 0)
		return ret;

	if (reg_val & FUSB301_REG_INT_ATTACH) {
		hwlog_info("fusb301 attach\n");
		di->dev_st.attach_status = TYPEC_ATTACH;
	} else if (reg_val & FUSB301_REG_INT_DETACH) {
		hwlog_info("fusb301 detach\n");
		di->dev_st.attach_status = TYPEC_DETACH;
	} else if (reg_val & FUSB301_REG_INT_CC_CUR_CH) {
		hwlog_info("interrupt of bc_lvl\n");
		di->dev_st.attach_status = TYPEC_CUR_CHANGE_FOR_FSC;
	} else if (reg_val & FUSB301_REG_INT_ACC_CH) {
		hwlog_info("interrupt of accessory change\n");
		di->dev_st.attach_status = TYPEC_ACC_MODE_CHANGE;
	} else {
		hwlog_err("wrong interrupt\n");
		di->dev_st.attach_status = TYPEC_STATUS_NOT_READY;
	}

	return di->dev_st.attach_status;
}

static int fusb301_detect_cc_orientation(void)
{
	int ret;
	u8 reg_val = 0;
	u8 cc_val;
	struct typec_device_info *di = g_fusb301_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fusb301_read_reg(FUSB301_REG_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	cc_val = reg_val & FUSB301_REG_CC_STATUS;

	if (cc_val == FUSB301_REG_CC2_DETECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_CC2;
		hwlog_info("cc2 detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == FUSB301_REG_CC1_DETECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_CC1;
		hwlog_info("cc1 detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == FUSB301_REG_NO_CC_DETECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_DEFAULT;
		hwlog_info("no cc has been detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == FUSB301_REG_CC_FAULT) {
		di->dev_st.cc_orient = TYPEC_ORIENT_NOT_READY;
		hwlog_info("there is a cc detection fault, cc_orient=%d\n",
			di->dev_st.cc_orient);
	}

	return di->dev_st.cc_orient;
}

static int fusb301_detect_port_mode(void)
{
	int ret;
	u8 reg_val = 0;
	struct typec_device_info *di = g_fusb301_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fusb301_read_reg(FUSB301_REG_TYPE, &reg_val);
	if (ret < 0)
		return ret;

	if (reg_val & FUSB301_REG_STATUS_DFP) {
		hwlog_info("ufp otg detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DFP;
	} else if (reg_val & FUSB301_REG_STATUS_UFP) {
		hwlog_info("dfp host detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_UFP;
	} else if (reg_val & FUSB301_REG_STATUS_POWERACC) {
		hwlog_info("power accessory otg is detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DFP;
	} else if (reg_val & FUSB301_REG_STATUS_DEBUGACC) {
		hwlog_info("debug accessory is detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DEBUGACC;
	} else if (reg_val & FUSB301_REG_STATUS_AUDIOACC) {
		hwlog_info("audio accessory is detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_AUDIOACC;
	} else {
		hwlog_err("detect port mode error\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_NOT_READY;
	}

	return di->dev_st.port_mode;
}

static int fusb301_detect_input_current(void)
{
	int ret;
	u8 reg_val = 0;
	u8 current_detect_val;
	struct typec_device_info *di = g_fusb301_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = fusb301_read_reg(FUSB301_REG_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	current_detect_val = reg_val & FUSB301_REG_DEV_CUR_MODE;

	switch (current_detect_val) {
	case FUSB301_REG_DEV_CUR_HIGH:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_HIGH;
		hwlog_info("detected type c current is 3A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case FUSB301_REG_DEV_CUR_MID:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_MID;
		hwlog_info("detected type c current is 1.5A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case FUSB301_REG_DEV_CUR_DEFAULT:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_DEFAULT;
		hwlog_info("detected type c current is default, iin=%d\n",
			di->dev_st.input_current);
		break;

	default:
		hwlog_err("detect typec c current error, reg_val=0x%x\n",
			reg_val);
		return -1;
	}

	return di->dev_st.input_current;
}

struct typec_device_ops fusb301_ops = {
	.clean_int_mask = fusb301_clean_mask,
	.detect_attachment_status = fusb301_detect_attachment_status,
	.detect_cc_orientation = fusb301_detect_cc_orientation,
	.detect_input_current = fusb301_detect_input_current,
	.detect_port_mode = fusb301_detect_port_mode,
	.ctrl_output_current = fusb301_ctrl_output_current,
	.ctrl_port_mode = fusb301_ctrl_port_mode,
};

static ssize_t dump_regs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i, index;
	u8 reg_val = 0;
	const int regaddr[FUSB301_DUMP_REG_NUM] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x10, 0x11, 0x12, 0x13
	};
	const char str[] = "0123456789abcdef";

	/* if there is no register value, replace it with xx */
	for (i = 0; i < 3 * FUSB301_REGISTER_NUM; i++) {
		if (2 == (i % 3))
			buf[i] = ' ';
		else
			buf[i] = 'x';
	}

	buf[0x2f] = '\n'; /* reg 0x2f change line for better print type */
	buf[0x5f] = '\0'; /* reg 0x5f to end */

	for (i = 0; i < FUSB301_DUMP_REG_NUM; i++) {
		index = regaddr[i];
		fusb301_read_reg(index, &reg_val);
		buf[3 * (long)index] = str[(reg_val & 0xf0) >> 4];
		buf[3 * (long)index + 1] = str[reg_val & 0x0f];
		buf[3 * (long)index + 2] = ' ';
	}

	return 0x60; /* 0x60 is default size */
}

static DEVICE_ATTR(dump_regs, 0444, dump_regs_show, NULL);

static struct attribute *fusb301_attributes[] = {
	&dev_attr_dump_regs.attr,
	NULL,
};

static const struct attribute_group fusb301_attr_group = {
	.attrs = fusb301_attributes,
};

static int fusb301_create_sysfs(void)
{
	int ret = 0;
	struct class *typec_class = NULL;
	struct device *new_dev = NULL;

	typec_class = hw_typec_get_class();
	if (typec_class) {
		new_dev = device_create(typec_class, NULL, 0, NULL, "fusb301");
		if (IS_ERR(new_dev)) {
			hwlog_err("sysfs device create failed\n");
			return PTR_ERR(new_dev);
		}

		ret = sysfs_create_group(&new_dev->kobj, &fusb301_attr_group);
		if (ret)
			hwlog_err("sysfs group create failed\n");
	}

	return ret;
}

static void fusb301_remove_sysfs(struct typec_device_info *di)
{
	device_remove_file(&di->client->dev, &dev_attr_dump_regs);
}

static irqreturn_t fusb301_irq_handler(int irq, void *dev_id)
{
	int gpio_value_intb;
	struct typec_device_info *di = dev_id;

	hwlog_info("fusb301 int happened\n");

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	gpio_value_intb = gpio_get_value(di->gpio_intb);
	if (gpio_value_intb == 1) /* 1: gpio is high */
		hwlog_err("intb=%d when interrupt occured\n", gpio_value_intb);

	typec_wake_lock(di);
	schedule_work(&di->g_intb_work);

	return IRQ_HANDLED;
}

static void fusb301_initialization(void)
{
	u8 reg_val = 0;

	/* reset */
	fusb301_write_reg(FUSB301_REG_RESET, FUSB301_RESET);

	/* must delay 10ms */
	mdelay(10);

	/* check manual register */
	fusb301_read_reg(FUSB301_REG_MANUAL, &reg_val);
	hwlog_info("reg_manual value=%d\n", reg_val);

	/* read FUSB301_REG_INT register to clear the irq first */
	fusb301_detect_attachment_status();
	fusb301_ctrl_port_mode(TYPEC_HOST_PORT_MODE_DRP);
	fusb301_clean_mask();
}

static int fusb301_shutdown_prepare(struct notifier_block *nb,
	unsigned long event, void *data)
{
	switch (event) {
	/* fall through */
	case SYS_DOWN:
	case SYS_HALT:
	case SYS_POWER_OFF:
		hwlog_err("prepare to shutdown, event=%ld\n", event);
		fusb301_ctrl_port_mode(TYPEC_HOST_PORT_MODE_UFP);
		break;

	default:
		hwlog_err("error event, ignore, event=%ld\n", event);
		break;
	}

	return NOTIFY_OK;
}

static int fusb301_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	unsigned int gpio_enb_val = 1;
	struct typec_device_info *di = NULL;
	struct typec_device_info *pdi = NULL;
	struct device_node *node = NULL;
	unsigned int typec_trigger_otg = 0;

	hwlog_info("probe begin\n");

	reboot_nb.notifier_call = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_fusb301_dev = di;
	di->dev = &client->dev;
	di->client = client;
	node = di->dev->of_node;
	i2c_set_clientdata(client, di);

	di->gpio_enb = of_get_named_gpio(node, "fusb301_typec,gpio_enb", 0);
	hwlog_info("gpio_enb=%d\n", di->gpio_enb);

	if (!gpio_is_valid(di->gpio_enb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_check_i2c;
	}

	ret = gpio_request(di->gpio_enb, "fusb301_en");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_check_i2c;
	}

	ret = of_property_read_u32(node, "fusb301_gpio_enb", &gpio_enb_val);
	if (ret) {
		hwlog_err("fusb301_gpio_enb dts read failed\n");
		goto fail_free_gpio_enb;
	}
	hwlog_info("gpio_enb_val=%d\n", gpio_enb_val);

	ret = gpio_direction_output(di->gpio_enb, gpio_enb_val);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto fail_free_gpio_enb;
	}

	ret = fusb301_device_check();
	if (ret) {
		hwlog_err("the chip is not fusb301\n");
		goto fail_free_gpio_enb;
	}

	reboot_nb.notifier_call = fusb301_shutdown_prepare;
	register_reboot_notifier(&reboot_nb);

	of_property_read_u32(node, "typec_trigger_otg", &typec_trigger_otg);
	hwlog_info("typec_trigger_otg=%d\n", typec_trigger_otg);
	di->typec_trigger_otg = !!typec_trigger_otg;

	di->gpio_intb = of_get_named_gpio(node, "fusb301_typec,gpio_intb", 0);
	hwlog_info("gpio_intb=%d\n", di->gpio_intb);

	if (!gpio_is_valid(di->gpio_intb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	pdi = typec_chip_register(di, &fusb301_ops, THIS_MODULE);
	if (!pdi) {
		hwlog_err("typec register chip error\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	ret = gpio_request(di->gpio_intb, "fusb301_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_free_gpio_enb;
	}

	di->irq_intb = gpio_to_irq(di->gpio_intb);
	if (di->irq_intb < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto fail_free_gpio_intb;
	}

	ret = gpio_direction_input(di->gpio_intb);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		goto fail_free_gpio_intb;
	}

	fusb301_initialization();

	ret = request_irq(di->irq_intb, fusb301_irq_handler,
			IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING,
			"fusb301_int", pdi);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_intb = -1;
		goto fail_free_gpio_intb;
	}

	ret = fusb301_create_sysfs();
	if (ret)
		goto fail_create_sysfs;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_TYPEC);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* set current advertisement to high */
	fusb301_ctrl_output_current(TYPEC_HOST_CURRENT_HIGH);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
	fusb301_remove_sysfs(di);
fail_free_gpio_intb:
	gpio_free(di->gpio_intb);
fail_free_gpio_enb:
	if (reboot_nb.notifier_call)
		unregister_reboot_notifier(&reboot_nb);

	gpio_free(di->gpio_enb);
fail_check_i2c:
	devm_kfree(&client->dev, di);
	g_fusb301_dev = NULL;

	return ret;
}

static int fusb301_remove(struct i2c_client *client)
{
	struct typec_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	fusb301_remove_sysfs(di);
	free_irq(di->irq_intb, di);
	gpio_set_value(di->gpio_enb, 1);
	gpio_free(di->gpio_enb);
	gpio_free(di->gpio_intb);
	g_fusb301_dev = NULL;
	devm_kfree(&client->dev, di);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id typec_fusb301_ids[] = {
	{ .compatible = "huawei,fusb301" },
	{},
};
MODULE_DEVICE_TABLE(of, typec_fusb301_ids);

static const struct i2c_device_id fusb301_i2c_id[] = {
	{ "fusb301", 0 },
	{},
};

static struct i2c_driver fusb301_i2c_driver = {
	.driver = {
		.name = "fusb301",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(typec_fusb301_ids),
	},
	.probe = fusb301_probe,
	.remove = fusb301_remove,
	.id_table = fusb301_i2c_id,
};

static __init int fusb301_i2c_init(void)
{
	return i2c_add_driver(&fusb301_i2c_driver);
}

static __exit void fusb301_i2c_exit(void)
{
	i2c_del_driver(&fusb301_i2c_driver);
}

module_init(fusb301_i2c_init);
module_exit(fusb301_i2c_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei fusb301 typec module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
