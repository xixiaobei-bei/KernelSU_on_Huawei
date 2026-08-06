/*
 * tusb320.c
 *
 * driver file for TI tusb320 typec chip
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
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "tusb320.h"
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG typec_tusb320
HWLOG_REGIST();

struct typec_device_info *g_tusb320_dev;
static int input_current = -1;
static u8 attach_status;

static int tusb320_detect_port_mode(void);
static int tusb320_detect_input_current(void);

static int tusb320_i2c_read(struct typec_device_info *di,
	unsigned char *rxdata, int length)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = di->client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxdata,
		},
	};

	/* i2c_transfer returns number of messages transferred */
	ret = i2c_transfer(di->client->adapter, msgs, 1);
	if (ret < 0)
		hwlog_err("read transfer error %d\n", ret);

	return ret;
}

static int tusb320_i2c_write(struct typec_device_info *di,
	unsigned char *txdata, int length)
{
	int ret;
	struct i2c_msg msg[] = {
		{
			.addr = di->client->addr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	/* i2c_transfer returns number of messages transferred */
	ret = i2c_transfer(di->client->adapter, msg, 1);
	if (ret < 0)
		hwlog_err("write transfer error reg=%x,%d\n", txdata[0], ret);

	return ret;
}

static int tusb320_read_reg(u8 reg, u8 *val)
{
	int ret;
	u8 buf[1];
	struct typec_device_info *di = g_tusb320_dev;

	buf[0] = reg;
	ret = tusb320_i2c_write(di, buf, 1);
	if (ret < 0)
		return ret;

	return tusb320_i2c_read(di, val, 1);
}

static int tusb320_write_reg(u8 reg, u8 val)
{
	u8 buf[2]; /* 2 byte */
	struct typec_device_info *di = g_tusb320_dev;

	buf[0] = reg;
	buf[1] = val;

	return tusb320_i2c_write(di, buf, 2);
}

static int tusb320_clean_mask(void)
{
	u8 reg_val = 0;
	int ret;

	ret = tusb320_read_reg(TUSB320_REG_ATTACH_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	reg_val |= TUSB320_REG_STATUS_INT;
	ret = tusb320_write_reg(TUSB320_REG_ATTACH_STATUS, reg_val);
	if (ret < 0)
		return ret;

	ret = tusb320_read_reg(TUSB320_REG_ATTACH_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	if (reg_val & TUSB320_REG_STATUS_INT) {
		ret = tusb320_write_reg(TUSB320_REG_ATTACH_STATUS, reg_val);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int tusb320_device_check(void)
{
	u8 reg_val = 0;

	return tusb320_read_reg(TUSB320_REG_DEVICE_ID, &reg_val);
}

static int tusb320_host_current_mode(u8 val)
{
	u8 reg_val = 0;
	u8 mask_val;
	u8 current_set_val;
	int ret;

	ret = tusb320_read_reg(TUSB320_REG_CURRENT_MODE, &reg_val);
	if (ret < 0)
		return ret;

	mask_val = reg_val & (~TUSB320_REG_HOST_CUR_MODE);
	current_set_val = reg_val & TUSB320_REG_HOST_CUR_MODE;

	if (val == current_set_val) {
		hwlog_info("current mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = tusb320_write_reg(TUSB320_REG_CURRENT_MODE, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int tusb320_ctrl_output_current(int value)
{
	switch (value) {
	case TYPEC_HOST_CURRENT_DEFAULT:
		hwlog_info("set to default current\n");
		tusb320_host_current_mode(TUSB320_REG_HOST_CUR_DEFAULT);
		break;

	case TYPEC_HOST_CURRENT_MID:
		hwlog_info("set to medium current\n");
		tusb320_host_current_mode(TUSB320_REG_HOST_CUR_MID);
		break;

	case TYPEC_HOST_CURRENT_HIGH:
		hwlog_info("set to high current\n");
		tusb320_host_current_mode(TUSB320_REG_HOST_CUR_HIGH);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static void tusb320_soft_reset(void)
{
	u8 reg_val = 0;
	int ret;

	ret = tusb320_read_reg(TUSB320_REG_MODE_SET, &reg_val);
	if (ret < 0)
		hwlog_err("read reg_mode_set error\n");

	reg_val |= TUSB320_REG_SET_SOFT_RESET;

	ret = tusb320_write_reg(TUSB320_REG_MODE_SET, reg_val);
	if (ret < 0)
		hwlog_err("write reg_mode_set error\n");
}

static int tusb320_host_port_mode(u8 val)
{
	u8 reg_val = 0;
	u8 mask_val;
	u8 port_set_val;
	int ret;

	ret = tusb320_read_reg(TUSB320_REG_MODE_SET, &reg_val);
	if (ret < 0)
		return ret;

	mask_val = reg_val & (~TUSB320_REG_SET_PORT_MODE);
	port_set_val = reg_val & TUSB320_REG_SET_PORT_MODE;

	if (val == port_set_val) {
		hwlog_info("port mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = tusb320_write_reg(TUSB320_REG_MODE_SET, val);
	if (ret < 0)
		return ret;

	return 0;
}

static void tusb320_rd_rp_disable(int set)
{
	u8 reg_val = 0;
	int ret;

	ret = tusb320_read_reg(TUSB320_REG_DISABLE, &reg_val);
	if (ret < 0)
		hwlog_err("read reg_disable error\n");

	if (set == DISABLE_SET) {
		hwlog_info("rd_rp_disable set\n");
		reg_val |= TUSB320_REG_SET_DISABLE_RD_RP;
	} else {
		hwlog_info("rd_rp_disable clear\n");
		reg_val &= ~((u8)TUSB320_REG_SET_DISABLE_RD_RP);
	}

	ret = tusb320_write_reg(TUSB320_REG_DISABLE, reg_val);
	if (ret < 0)
		hwlog_err("write reg_disable error\n");
}

static int tusb320_ctrl_port_mode(int value)
{
	switch (value) {
	case TYPEC_HOST_PORT_MODE_DFP:
		hwlog_info("set to dfp mode\n");
		tusb320_host_port_mode(TUSB320_REG_SET_UFP);
		tusb320_rd_rp_disable(DISABLE_SET);
		tusb320_soft_reset();
		mdelay(TUSB320_RESET_DURATION_MS);
		tusb320_host_port_mode(TUSB320_REG_SET_DFP);
		tusb320_rd_rp_disable(DISABLE_CLEAR);
		break;

	case TYPEC_HOST_PORT_MODE_UFP:
		hwlog_info("set to ufp mode\n");
		tusb320_write_reg(TUSB320_REG_MODE_SET,
			TUSB320_REG_SET_UFP |
			TUSB320_REG_SET_SOFT_RESET |
			TUSB320_REG_SET_DISABLE_TERM);
		mdelay(TUSB320_UFP_RESET_DURATION_MS);
		tusb320_write_reg(TUSB320_REG_MODE_SET, TUSB320_REG_SET_UFP);
		break;

	case TYPEC_HOST_PORT_MODE_DRP:
		hwlog_info("set to drp mode\n");
		tusb320_host_port_mode(TUSB320_REG_SET_DRP);
		tusb320_rd_rp_disable(DISABLE_SET);
		tusb320_soft_reset();
		mdelay(TUSB320_RESET_DURATION_MS);
		tusb320_host_port_mode(TUSB320_REG_SET_DRP);
		tusb320_rd_rp_disable(DISABLE_CLEAR);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static int tusb320_read_attachment_reg(void)
{
	u8 reg_val = 0;

	tusb320_read_reg(TUSB320_REG_ATTACH_STATUS, &reg_val);

	hwlog_info("register value of 09h is 0x%x\n", reg_val);

	tusb320_clean_mask();

	return reg_val;
}

static int tusb320_detect_attachment_status(void)
{
	u8 reg_val;
	u8 mode_val;
	int reg_current;
	int port_mode;
	struct typec_device_info *di = g_tusb320_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	reg_val = tusb320_read_attachment_reg();
	mode_val = reg_val & TUSB320_REG_STATUS_MODE;
	reg_current = tusb320_detect_input_current();

	if (mode_val) {
		hwlog_info("attach,reg_val=%d,reg_cur=%d\n",
			reg_val, reg_current);
		port_mode = tusb320_detect_port_mode();
		if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
			if (reg_val != attach_status) {
				attach_status = reg_val;
				input_current = reg_current;
				di->dev_st.attach_status = TYPEC_ATTACH;
				hwlog_info("attach different attach status\n");
			} else {
				if ((reg_current != input_current) &&
					reg_current >= 0) {
					input_current = reg_current;
					di->dev_st.attach_status =
						TYPEC_CUR_CHANGE_FOR_FSC;
					hwlog_info("typec current change\n");
				} else {
					di->dev_st.attach_status = TYPEC_ATTACH;
					hwlog_info("tusb320 attach anyway\n");
				}
			}
		} else {
			di->dev_st.attach_status = TYPEC_ATTACH;
		}
	} else {
		hwlog_info("tusb320 detach\n");
		di->dev_st.attach_status = TYPEC_DETACH;
		input_current = -1;
		attach_status = 0;
	}

	return di->dev_st.attach_status;
}

static int tusb320_detect_cc_orientation(void)
{
	u8 reg_val = 0;
	u8 cc_val;
	u8 mode_val;
	struct typec_device_info *di = g_tusb320_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	tusb320_read_reg(TUSB320_REG_ATTACH_STATUS, &reg_val);
	cc_val = reg_val & TUSB320_REG_STATUS_CC;
	mode_val = reg_val & TUSB320_REG_STATUS_MODE;

	if (mode_val == TUSB320_REG_STATUS_DFP ||
		mode_val == TUSB320_REG_STATUS_UFP) {
		if (cc_val) {
			di->dev_st.cc_orient = TYPEC_ORIENT_CC2;
			hwlog_info("cc2 detected, cc_orient=%d\n",
				di->dev_st.cc_orient);
		} else {
			di->dev_st.cc_orient = TYPEC_ORIENT_CC1;
			hwlog_info("cc1 detected, cc_orient=%d\n",
				di->dev_st.cc_orient);
		}
	} else {
		di->dev_st.cc_orient = TYPEC_ORIENT_DEFAULT;
		hwlog_info("cc not attached, reg=0x%02X\n", reg_val);
	}

	return di->dev_st.cc_orient;
}

static int tusb320_detect_port_mode(void)
{
	u8 reg_val = 0;
	u8 mode_val;
	struct typec_device_info *di = g_tusb320_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	tusb320_read_reg(TUSB320_REG_ATTACH_STATUS, &reg_val);
	mode_val = reg_val & TUSB320_REG_STATUS_MODE;

	if (mode_val == TUSB320_REG_STATUS_DFP) {
		hwlog_info("ufp otg detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DFP;
	} else if (mode_val == TUSB320_REG_STATUS_UFP) {
		hwlog_info("dfp host detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_UFP;
	} else {
		hwlog_err("detect port mode error\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_NOT_READY;
	}

	return di->dev_st.port_mode;
}

static int tusb320_detect_input_current(void)
{
	u8 reg_val = 0;
	u8 current_detect_val;
	struct typec_device_info *di = g_tusb320_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	tusb320_read_reg(TUSB320_REG_CURRENT_MODE, &reg_val);

	current_detect_val = reg_val & TUSB320_REG_DEV_CUR_MODE;

	switch (current_detect_val) {
	case TUSB320_REG_DEV_CUR_HIGH:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_HIGH;
		hwlog_info("detected type c current is 3A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case TUSB320_REG_DEV_CUR_MID:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_MID;
		hwlog_info("detected type c current is 1.5A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case TUSB320_REG_DEV_CUR_DEFAULT:
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

struct typec_device_ops tusb320_ops = {
	.clean_int_mask = tusb320_clean_mask,
	.detect_attachment_status = tusb320_detect_attachment_status,
	.detect_cc_orientation = tusb320_detect_cc_orientation,
	.detect_input_current = tusb320_detect_input_current,
	.detect_port_mode = tusb320_detect_port_mode,
	.ctrl_output_current = tusb320_ctrl_output_current,
	.ctrl_port_mode = tusb320_ctrl_port_mode,
};

static ssize_t dump_regs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	u8 reg[TUSB320_REGISTER_NUM] = {0};

	/* from reg08 to reg11 */
	for (i = 8; i < 11; i++)
		tusb320_read_reg(i, &reg[i]);

	return scnprintf(buf, PAGE_SIZE, "0x%02X,0x%02X,0x%02X\n",
		reg[8], reg[9], reg[10]);
}

static DEVICE_ATTR(dump_regs, 0444, dump_regs_show, NULL);

static struct attribute *tusb320_attributes[] = {
	&dev_attr_dump_regs.attr,
	NULL,
};

static const struct attribute_group tusb320_attr_group = {
	.attrs = tusb320_attributes,
};

static int tusb320_create_sysfs(void)
{
	int ret = 0;
	struct class *typec_class = NULL;
	struct device *new_dev = NULL;

	typec_class = hw_typec_get_class();
	if (typec_class) {
		new_dev = device_create(typec_class, NULL, 0, NULL, "tusb320");
		if (IS_ERR(new_dev)) {
			hwlog_err("sysfs device create failed\n");
			return PTR_ERR(new_dev);
		}

		ret = sysfs_create_group(&new_dev->kobj, &tusb320_attr_group);
		if (ret)
			hwlog_err("sysfs group create failed\n");
	}

	return ret;
}

static void tusb320_remove_sysfs(struct typec_device_info *di)
{
	device_remove_file(&di->client->dev, &dev_attr_dump_regs);
}

static irqreturn_t tusb320_irq_handler(int irq, void *dev_id)
{
	int gpio_value_intb;
	struct typec_device_info *di = dev_id;

	hwlog_info("tusb320 int happened\n");

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

static void tusb320_initialization(void)
{
	u8 reg_val = 0;

	tusb320_read_reg(TUSB320_REG_MODE_SET, &reg_val);
	hwlog_info("reg_mode_set value=%d\n", reg_val);

	reg_val |= TUSB320_REG_SET_SOFT_RESET;
	tusb320_write_reg(TUSB320_REG_MODE_SET, reg_val);

	tusb320_host_port_mode(TUSB320_REG_SET_DRP);
}

static int tusb320_probe(struct i2c_client *client,
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

	g_tusb320_dev = di;
	di->dev = &client->dev;
	di->client = client;
	node = di->dev->of_node;
	i2c_set_clientdata(client, di);

	di->gpio_enb = of_get_named_gpio(node, "tusb320_typec,gpio_enb", 0);
	hwlog_info("gpio_enb=%d\n", di->gpio_enb);

	if (!gpio_is_valid(di->gpio_enb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_check_i2c;
	}

	ret = gpio_request(di->gpio_enb, "tusb320_en");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_check_i2c;
	}

	ret = of_property_read_u32(node, "tusb320_gpio_enb", &gpio_enb_val);
	if (ret) {
		hwlog_err("tusb320_gpio_enb dts read failed\n");
		goto fail_free_gpio_enb;
	}
	hwlog_info("gpio_enb_val=%d\n", gpio_enb_val);

	ret = gpio_direction_output(di->gpio_enb, gpio_enb_val);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto fail_free_gpio_enb;
	}

	/*
	 * t_i2c_en in the spec
	 * time from enable signal active to i2c access available
	 */
	ret = of_property_read_u32(node, "tusb320_mdelay", &mdelay);
	if (ret) {
		hwlog_err("tusb320_mdelay dts read failed\n");
		mdelay = 0;
	}
	hwlog_info("mdelay=%d\n", mdelay);

	if (mdelay) {
		hwlog_info("wait for %d ms before i2c access\n", mdelay);
		msleep(mdelay);
	}

	ret = tusb320_device_check();
	if (ret) {
		hwlog_err("the chip is not tusb320\n");
		goto fail_free_gpio_enb;
	}

	of_property_read_u32(node, "typec_trigger_otg", &typec_trigger_otg);
	hwlog_info("typec_trigger_otg=%d\n", typec_trigger_otg);
	di->typec_trigger_otg = !!typec_trigger_otg;

	di->gpio_intb = of_get_named_gpio(node, "tusb320_typec,gpio_intb", 0);
	hwlog_info("gpio_intb=%d\n", di->gpio_intb);

	if (!gpio_is_valid(di->gpio_intb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	pdi = typec_chip_register(di, &tusb320_ops, THIS_MODULE);
	if (!pdi) {
		hwlog_err("typec register chip error\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	ret = gpio_request(di->gpio_intb, "tusb320_int");
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

	ret = request_irq(di->irq_intb, tusb320_irq_handler,
			IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING,
			"tusb320_int", pdi);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_intb = -1;
		goto fail_free_gpio_intb;
	}

	ret = tusb320_clean_mask();
	if (ret) {
		hwlog_err("clear interrupt mask error %d\n", ret);
		goto fail_free_gpio_irq;
	}

	ret = tusb320_create_sysfs();
	if (ret)
		goto fail_create_sysfs;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_TYPEC);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	tusb320_initialization();

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	/* set current advertisement to high */
	tusb320_ctrl_output_current(TYPEC_HOST_CURRENT_HIGH);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
	tusb320_remove_sysfs(di);
fail_free_gpio_irq:
	free_irq(di->gpio_intb, di);
fail_free_gpio_intb:
	gpio_free(di->gpio_intb);
fail_free_gpio_enb:
	gpio_free(di->gpio_enb);
fail_check_i2c:
	devm_kfree(&client->dev, di);
	g_tusb320_dev = NULL;

	return ret;
}

static int tusb320_remove(struct i2c_client *client)
{
	struct typec_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	tusb320_remove_sysfs(di);
	free_irq(di->irq_intb, di);
	gpio_set_value(di->gpio_enb, 1);
	gpio_free(di->gpio_enb);
	gpio_free(di->gpio_intb);
	g_tusb320_dev = NULL;
	devm_kfree(&client->dev, di);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id typec_tusb320_ids[] = {
	{ .compatible = "huawei,tusb320" },
	{},
};
MODULE_DEVICE_TABLE(of, typec_tusb320_ids);

static const struct i2c_device_id tusb320_i2c_id[] = {
	{ "tusb320", 0 },
	{},
};

static struct i2c_driver tusb320_i2c_driver = {
	.driver = {
		.name = "tusb320",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(typec_tusb320_ids),
	},
	.probe = tusb320_probe,
	.remove = tusb320_remove,
	.id_table = tusb320_i2c_id,
};

static __init int tusb320_i2c_init(void)
{
	return i2c_add_driver(&tusb320_i2c_driver);
}

static __exit void tusb320_i2c_exit(void)
{
	i2c_del_driver(&tusb320_i2c_driver);
}

module_init(tusb320_i2c_init);
module_exit(tusb320_i2c_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei tusb320 typec module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
