/*
 * pericom30216.c
 *
 * driver file for Pericom pericom30216 typec chip
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
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "pericom30216.h"
#include <huawei_platform/usb/switch/switch_ap/switch_usb_class.h>
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG typec_pericom30216
HWLOG_REGIST();

struct typec_device_info *g_pericom30216_dev;
static int input_current = -1;

static int pericom30216_detect_current_mode(void);
static int pericom30216_detect_port_mode(void);

static int pericom30216_read_reg(int regnr)
{
	u8 buf[PERICOM30216_REGISTER_NUM];
	struct typec_device_info *di = g_pericom30216_dev;
	struct i2c_msg msgs[1] = {
		{
			.addr = di->client->addr,
			.flags = I2C_M_RD,
			.len = sizeof(u8) * PERICOM30216_REGISTER_NUM,
			.buf = (void *)buf
		},
	};

	/* i2c_transfer returns number of messages transferred */
	if (i2c_transfer(di->client->adapter, msgs, 1) != 1) {
		hwlog_err("read transfer error reg=%x\n", regnr);
		return -EIO;
	}

	di->reg[regnr - 1] = buf[regnr - 1];

	return 0;
}

static int pericom30216_read_control_reg(void)
{
	int i;
	u8 buf[PERICOM30216_REGISTER_NUM - 2];
	struct typec_device_info *di = g_pericom30216_dev;
	struct i2c_msg msgs[1] = {
		{
			.addr = di->client->addr,
			.flags = I2C_M_RD,
			.len = sizeof(u8) * (PERICOM30216_REGISTER_NUM - 2),
			.buf = (void *)buf
		},
	};

	/* i2c_transfer returns number of messages transferred */
	if (i2c_transfer(di->client->adapter, msgs, 1) != 1)
		return -EIO;

	for (i = 0; i < PERICOM30216_REGISTER_NUM - 2; i++)
		di->reg[i] = buf[i];

	return 0;
}

static int pericom30216_read_all_reg(void)
{
	int i;
	u8 buf[PERICOM30216_REGISTER_NUM];
	struct typec_device_info *di = g_pericom30216_dev;
	struct i2c_msg msgs[1] = {
		{
			.addr = di->client->addr,
			.flags = I2C_M_RD,
			.len = sizeof(u8) * PERICOM30216_REGISTER_NUM,
			.buf = (void *)buf
		},
	};

	/* i2c_transfer returns number of messages transferred */
	if (i2c_transfer(di->client->adapter, msgs, 1) != 1)
		return -EIO;

	for (i = 0; i < PERICOM30216_REGISTER_NUM; i++)
		di->reg[i] = buf[i];

	return 0;
}

static int pericom30216_write_reg(u8 reg, u8 value)
{
	int i;
	u8 buf[PERICOM30216_REGISTER_NUM];
	struct typec_device_info *di = g_pericom30216_dev;
	struct i2c_msg msgs[1] = {
		{
			.addr = di->client->addr,
			.len = sizeof(u8) * PERICOM30216_REGISTER_NUM,
			.buf = (void *)buf
		},
	};

	pericom30216_read_control_reg();

	for (i = 0; i < PERICOM30216_REGISTER_NUM; i++)
		buf[i] = di->reg[i];

	buf[reg - 1] = value;

	/* i2c_transfer returns number of messages transferred */
	if (i2c_transfer(di->client->adapter, msgs, 1) != 1) {
		hwlog_err("write transfer error reg=%x\n", reg);
		return -EIO;
	}

	return 0;
}

static int pericom30216_clean_int_mask(void)
{
	u8 reg_val;
	struct typec_device_info *di = g_pericom30216_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	reg_val = di->reg[PERICOM30216_REG_CONTROL - 1];
	if (reg_val & PERICOM30216_REG_CONTROL_INT_MASK) {
		reg_val &= ~PERICOM30216_REG_CONTROL_INT_MASK;
		pericom30216_write_reg(PERICOM30216_REG_CONTROL, reg_val);
	}

	return 0;
}

static int pericom30216_device_check(void)
{
	return pericom30216_read_reg(PERICOM30216_REG_DEVICE_ID);
}

static int pericom30216_current_status_setting(u8 val)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	u8 mask_val;
	u8 current_set_val;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = pericom30216_read_reg(PERICOM30216_REG_CONTROL);
	if (ret < 0)
		return ret;

	reg_val = di->reg[PERICOM30216_REG_CONTROL - 1];
	mask_val = reg_val & (~PERICOM30216_REG_SET_CURRENT_STATUES);
	current_set_val = reg_val & PERICOM30216_REG_SET_CURRENT_STATUES;

	if (val == current_set_val) {
		hwlog_info("current mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = pericom30216_write_reg(PERICOM30216_REG_CONTROL, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int pericom30216_ctrl_output_current(int value)
{
	switch (value) {
	case TYPEC_HOST_CURRENT_DEFAULT:
		hwlog_info("set to default current\n");
		pericom30216_current_status_setting(
			PERICOM30216_REG_CONTROL_DEFAULT_CURRENT);
		break;

	case TYPEC_HOST_CURRENT_MID:
		hwlog_info("set to medium current\n");
		pericom30216_current_status_setting(
			PERICOM30216_REG_CONTROL_MID_CURRENT);
		break;

	case TYPEC_HOST_CURRENT_HIGH:
		hwlog_info("set to high current\n");
		pericom30216_current_status_setting(
			PERICOM30216_REG_CONTROL_HIGH_CURRENT);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static int pericom30216_port_status_setting(u8 val)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	u8 mask_val;
	u8 port_set_val;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = pericom30216_read_reg(PERICOM30216_REG_CONTROL);
	if (ret < 0)
		return ret;

	reg_val = di->reg[PERICOM30216_REG_CONTROL - 1];
	mask_val = reg_val & (~PERICOM30216_REG_SET_PORT_STATUS);
	port_set_val = reg_val & PERICOM30216_REG_SET_PORT_STATUS;

	if (val == port_set_val) {
		hwlog_info("port mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = pericom30216_write_reg(PERICOM30216_REG_CONTROL, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int pericom30216_port_set_disabled(int set)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = pericom30216_read_reg(PERICOM30216_REG_CONTROL);
	if (ret < 0)
		return ret;

	reg_val = di->reg[PERICOM30216_REG_CONTROL - 1];

	if (set == DISABLE_SET) {
		hwlog_info("port_set_disabled set\n");
		reg_val |= PERICOM30216_REG_CONTROL_POWER_SAVING;
	} else {
		hwlog_info("port_set_disabled clear\n");
		reg_val &= ~((u8)PERICOM30216_REG_CONTROL_POWER_SAVING);
	}

	ret = pericom30216_write_reg(PERICOM30216_REG_CONTROL, reg_val);
	if (ret < 0)
		return ret;

	return 0;
}

static void pericom30216_soft_reset(void)
{
	pericom30216_port_status_setting(PERICOM30216_REG_CONTROL_SET_DRP);
	mdelay(PERICOM_RESET_DURATION_MS);
	pericom30216_port_status_setting(PERICOM30216_REG_CONTROL_SET_UFP);
	mdelay(PERICOM_RESET_DURATION_MS);
	pericom30216_port_status_setting(PERICOM30216_REG_CONTROL_SET_DRP);
	mdelay(PERICOM_DURATION_MS);
}

static int pericom30216_ctrl_port_mode(int value)
{
	switch (value) {
	case TYPEC_HOST_PORT_MODE_DFP:
		hwlog_info("set to dfp mode\n");
		pericom30216_port_status_setting(
			PERICOM30216_REG_CONTROL_SET_DFP);
		pericom30216_port_set_disabled(DISABLE_SET);
		pericom30216_soft_reset();
		pericom30216_port_status_setting
			(PERICOM30216_REG_CONTROL_SET_DFP);
		pericom30216_port_set_disabled(DISABLE_CLEAR);
		break;

	case TYPEC_HOST_PORT_MODE_UFP:
		hwlog_info("set to ufp mode\n");
		pericom30216_port_status_setting(
			PERICOM30216_REG_CONTROL_SET_UFP);
		pericom30216_port_set_disabled(DISABLE_SET);
		pericom30216_soft_reset();
		pericom30216_port_status_setting(
			PERICOM30216_REG_CONTROL_SET_UFP);
		pericom30216_port_set_disabled(DISABLE_CLEAR);
		break;

	case TYPEC_HOST_PORT_MODE_DRP:
		hwlog_info("set to drp mode\n");
		pericom30216_port_status_setting(
			PERICOM30216_REG_CONTROL_SET_DRP);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static int pericom30216_detect_attachment_status(void)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	u8 reg_cc_status;
	u8 reg_ctrl;
	int reg_status;
	u8 port_mode;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* read control reg and mask interrupt */
	pericom30216_read_control_reg();
	pericom30216_write_reg(PERICOM30216_REG_CONTROL,
		PERICOM30216_REG_CONTROL_INT_MASK |
		(di->reg[PERICOM30216_REG_CONTROL - 1]));

	/* read all reg status and clear the reg3 attachment interrupt status */
	pericom30216_read_all_reg();
	mdelay(1); /* 1ms */

	/* unmask the interrupt */
	pericom30216_write_reg(PERICOM30216_REG_CONTROL,
		(~PERICOM30216_REG_CONTROL_INT_MASK) &
		(di->reg[PERICOM30216_REG_CONTROL - 1]));
	reg_val = di->reg[PERICOM30216_REG_INTERRUPT - 1];
	reg_cc_status = di->reg[PERICOM30216_REG_CC_STATUS - 1];
	reg_ctrl = di->reg[PERICOM30216_REG_CONTROL - 1];
	hwlog_info("reg reg1=0x%x, reg2=0x%x, reg3=0x%x, reg4=0x%x\n",
		di->reg[0], di->reg[1], di->reg[2], di->reg[3]);

	/* pericom30216 chip bug workaround */
	if (reg_cc_status == 0x97) {
		mdelay(300);
		pericom30216_write_reg(PERICOM30216_REG_CONTROL, 0x01);
		pericom30216_write_reg(PERICOM30216_REG_CONTROL, 0x05);
		pericom30216_read_reg(PERICOM30216_REG_CC_STATUS);
		pericom30216_write_reg(PERICOM30216_REG_CONTROL, reg_ctrl);
	}

	reg_status = pericom30216_detect_current_mode();
	if (reg_val & PERICOM30216_REG_ATTACH) {
		port_mode = pericom30216_detect_port_mode();
		if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
			input_current = reg_status;
			hwlog_info("attach different attach status\n");
		}
		di->dev_st.attach_status = TYPEC_ATTACH;
	} else if (reg_val & PERICOM30216_REG_DETACH) {
		hwlog_info("pericom detach\n");
		di->dev_st.attach_status = TYPEC_DETACH;
		input_current = -1;
	} else {
		port_mode = pericom30216_detect_port_mode();
		if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
			if ((input_current != reg_status) && reg_status != -1) {
				input_current = reg_status;
				di->dev_st.attach_status =
					TYPEC_CUR_CHANGE_FOR_FSC;
				hwlog_info("typec current change\n");
			}
		}
		/*
		 * there will be two interrupts when a DFP device is detected.
		 * the first is attach and the next is a none-type interrupt
		 * as a chip feature.
		 */
	}

	return di->dev_st.attach_status;
}

static int pericom30216_detect_current_mode(void)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	u8 current_detect_val;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	reg_val = di->reg[PERICOM30216_REG_CC_STATUS - 1];
	current_detect_val = reg_val & PERICOM30216_REG_DEV_CURRENT_MODE;

	switch (current_detect_val) {
	case PERICOM30216_REG_HIGH_CURRENT:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_HIGH;
		hwlog_info("detected type c current is 3A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case PERICOM30216_REG_MID_CURRENT:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_MID;
		hwlog_info("detected type c current is 1.5A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case PERICOM30216_REG_DEFAULT_CURRENT:
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

static int pericom30216_detect_cc_orientation(void)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	u8 cc_val;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	reg_val = di->reg[PERICOM30216_REG_CC_STATUS - 1];
	cc_val = reg_val & PERICOM30216_REG_CC_CONNECTED_STATUS;

	if (cc_val == PERICOM30216_REG_CC1_CONNECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_CC1;
		hwlog_info("cc1 detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == PERICOM30216_REG_CC2_CONNECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_CC2;
		hwlog_info("cc2 detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == PERICOM30216_REG_CC_UNDERTERMINED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_NOT_READY;
		hwlog_info("undetermined status is detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else {
		di->dev_st.cc_orient = TYPEC_ORIENT_DEFAULT;
		hwlog_err("detect cc orientation error, cc_orient=%d\n",
			di->dev_st.cc_orient);
	}

	return di->dev_st.cc_orient;
}

static int pericom30216_detect_port_mode(void)
{
	struct typec_device_info *di = g_pericom30216_dev;
	u8 reg_val;
	u8 port_mode_val;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	reg_val = di->reg[PERICOM30216_REG_CC_STATUS - 1];
	port_mode_val = reg_val & PERICOM30216_REG_PORT_MODE;

	if (port_mode_val == PERICOM30216_REG_STATUS_UFP) {
		hwlog_info("ufp otg detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DFP;
	} else if (port_mode_val == PERICOM30216_REG_STATUS_DFP) {
		hwlog_info("dfp host detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_UFP;
	} else if (port_mode_val == PERICOM30216_REG_STATUS_AUDIO) {
		hwlog_info("audio accessory detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_AUDIOACC;
	} else if (port_mode_val == PERICOM30216_REG_STATUS_DEBUG) {
		hwlog_info("debug accessory detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DEBUGACC;
	} else {
		hwlog_err("detect port mode error\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_NOT_READY;
	}

	return di->dev_st.port_mode;
}

struct typec_device_ops pericom30216_ops = {
	.clean_int_mask = pericom30216_clean_int_mask,
	.detect_attachment_status = pericom30216_detect_attachment_status,
	.detect_cc_orientation = pericom30216_detect_cc_orientation,
	.detect_input_current = pericom30216_detect_current_mode,
	.detect_port_mode = pericom30216_detect_port_mode,
	.ctrl_output_current = pericom30216_ctrl_output_current,
	.ctrl_port_mode = pericom30216_ctrl_port_mode,
};

static ssize_t dump_regs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct typec_device_info *di = g_pericom30216_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return scnprintf(buf, PAGE_SIZE, "0x%02X,0x%02X,0x%02X,0x%02x\n",
		di->reg[0], di->reg[1], di->reg[2], di->reg[3]);
}

static DEVICE_ATTR(dump_regs, 0444, dump_regs_show, NULL);

static struct attribute *pericom30216_attributes[] = {
	&dev_attr_dump_regs.attr,
	NULL,
};

static const struct attribute_group pericom30216_attr_group = {
	.attrs = pericom30216_attributes,
};

static int pericom30216_create_sysfs(void)
{
	int ret = 0;
	struct class *typec_class = NULL;
	struct device *new_dev = NULL;

	typec_class = hw_typec_get_class();
	if (typec_class) {
		new_dev = device_create(typec_class, NULL, 0, NULL,
			"pericom30216");
		if (IS_ERR(new_dev)) {
			hwlog_err("sysfs device create failed\n");
			return PTR_ERR(new_dev);
		}

		ret = sysfs_create_group(&new_dev->kobj,
			&pericom30216_attr_group);
		if (ret)
			hwlog_err("sysfs group create failed\n");
	}

	return ret;
}

static void pericom30216_remove_sysfs(struct typec_device_info *di)
{
	device_remove_file(&di->client->dev, &dev_attr_dump_regs);
}

static irqreturn_t pericom30216_irq_handler(int irq, void *dev_id)
{
	int gpio_value_intb;
	struct typec_device_info *di = dev_id;

	hwlog_info("pericom30216 int happened\n");

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

static void pericom30216_initialization(void)
{
	pericom30216_write_reg(PERICOM30216_REG_CONTROL,
		PERICOM30216_REG_CONTROL_INT_MASK);

	/* delay 30ms */
	msleep(30);

	pericom30216_write_reg(PERICOM30216_REG_CONTROL,
		PERICOM30216_REG_CONTROL_SET_DRP);
}

static int pericom30216_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	unsigned int gpio_enb_val = 1;
	struct typec_device_info *di = NULL;
	struct typec_device_info *pdi = NULL;
	struct device_node *node = NULL;
	unsigned int typec_trigger_otg = 0;
	unsigned int mdelay = 0;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_pericom30216_dev = di;
	di->dev = &client->dev;
	di->client = client;
	node = di->dev->of_node;
	i2c_set_clientdata(client, di);

	di->gpio_enb = of_get_named_gpio(node,
		"pericom30216_typec,gpio_enb", 0);
	hwlog_info("gpio_enb=%d\n", di->gpio_enb);

	if (!gpio_is_valid(di->gpio_enb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_check_i2c;
	}

	ret = gpio_request(di->gpio_enb, "pericom30216_en");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_check_i2c;
	}

	ret = of_property_read_u32(node, "pericom30216_gpio_enb",
		&gpio_enb_val);
	if (ret) {
		hwlog_err("pericom30216_gpio_enb dts read failed\n");
		goto fail_free_gpio_enb;
	}
	hwlog_info("gpio_enb_val=%d\n", gpio_enb_val);

	ret = gpio_direction_output(di->gpio_enb, gpio_enb_val);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto fail_free_gpio_enb;
	}

	ret = of_property_read_u32(node, "pericom30216_mdelay", &mdelay);
	if (ret) {
		hwlog_err("pericom30216_mdelay dts read failed\n");
		mdelay = 0;
	}
	hwlog_info("mdelay=%d\n", mdelay);

	if (mdelay) {
		hwlog_info("wait for %d ms before i2c access\n", mdelay);
		msleep(mdelay);
	}

	ret = pericom30216_device_check();
	if (ret) {
		hwlog_err("the chip is not pericom30216\n");
		goto fail_free_gpio_enb;
	}

	of_property_read_u32(node, "typec_trigger_otg", &typec_trigger_otg);
	hwlog_info("typec_trigger_otg=%d\n", typec_trigger_otg);
	di->typec_trigger_otg = !!typec_trigger_otg;

	di->gpio_intb = of_get_named_gpio(node,
		"pericom30216_typec,gpio_intb", 0);
	hwlog_info("gpio_intb=%d\n", di->gpio_intb);

	if (!gpio_is_valid(di->gpio_intb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	pdi = typec_chip_register(di, &pericom30216_ops, THIS_MODULE);
	if (!pdi) {
		hwlog_err("typec register chip error\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	ret = gpio_request(di->gpio_intb, "pericom30216_int");
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

	ret = request_irq(di->irq_intb, pericom30216_irq_handler,
			IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING,
			"pericom30216_int", pdi);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_intb = -1;
		goto fail_free_gpio_intb;
	}

	ret = pericom30216_create_sysfs();
	if (ret)
		goto fail_create_sysfs;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_TYPEC);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	pericom30216_initialization();

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* set current advertisement to high */
	pericom30216_ctrl_output_current(TYPEC_HOST_CURRENT_HIGH);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
	pericom30216_remove_sysfs(di);
	free_irq(di->gpio_intb, di);
fail_free_gpio_intb:
	gpio_free(di->gpio_intb);
fail_free_gpio_enb:
	gpio_free(di->gpio_enb);
fail_check_i2c:
	devm_kfree(&client->dev, di);
	g_pericom30216_dev = NULL;

	return ret;
}

static int pericom30216_remove(struct i2c_client *client)
{
	struct typec_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	pericom30216_remove_sysfs(di);
	free_irq(di->irq_intb, di);
	gpio_set_value(di->gpio_enb, 1);
	gpio_free(di->gpio_enb);
	gpio_free(di->gpio_intb);
	g_pericom30216_dev = NULL;
	devm_kfree(&client->dev, di);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id typcc_pericom30216_ids[] = {
	{ .compatible = "huawei,pericom30216" },
	{},
};
MODULE_DEVICE_TABLE(of, typcc_pericom30216_ids);

static const struct i2c_device_id pericom30216_i2c_id[] = {
	{ "pericom30216", 0 },
	{},
};

static struct i2c_driver pericom30216_i2c_driver = {
	.driver = {
		.name = "pericom30216",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(typcc_pericom30216_ids),
	},
	.probe = pericom30216_probe,
	.remove = pericom30216_remove,
	.id_table = pericom30216_i2c_id,
};

static __init int pericom30216_i2c_init(void)
{
	return i2c_add_driver(&pericom30216_i2c_driver);
}

static __exit void pericom30216_i2c_exit(void)
{
	i2c_del_driver(&pericom30216_i2c_driver);
}

module_init(pericom30216_i2c_init);
module_exit(pericom30216_i2c_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei pericom30216 typec module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
