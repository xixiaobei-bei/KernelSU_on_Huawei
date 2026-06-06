/*
 * ptn5150h.c
 *
 * driver file for Nxp ptn5150h typec chip
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
#include "ptn5150h.h"
#include <huawei_platform/usb/hw_typec_dev.h>
#include <huawei_platform/usb/hw_typec_platform.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG typec_ptn5150h
HWLOG_REGIST();

struct typec_device_info *g_ptn5150h_dev;
static int input_current = -1;

static int ptn5150h_detect_input_current(void);
static int ptn5150h_detect_port_mode(void);

static int ptn5150h_i2c_read(struct typec_device_info *di, u8 reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(di->client, reg);
	if (ret < 0)
		hwlog_err("read byte error reg=%x,%d\n", reg, ret);

	return ret;
}

static int ptn5150h_i2c_write(struct typec_device_info *di, u8 reg, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(di->client, reg, val);
	if (ret < 0)
		hwlog_err("write byte error reg=%x,%d\n", reg, ret);

	return ret;
}

static int ptn5150h_read_reg(u8 reg, u8 *val)
{
	int ret;
	struct typec_device_info *di = g_ptn5150h_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = ptn5150h_i2c_read(di, reg);
	if (ret < 0)
		return ret;

	*val = ret;
	return ret;
}

static int ptn5150h_write_reg(u8 reg, u8 val)
{
	struct typec_device_info *di = g_ptn5150h_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	return ptn5150h_i2c_write(di, reg, val);
}

static int ptn5150h_clean_mask(void)
{
	u8 reg_val = 0;
	int ret;

	ret = ptn5150h_read_reg(PTN5150H_REG_CONTROL, &reg_val);
	if (ret < 0)
		return ret;

	reg_val &= ~PTN5150H_REG_INT_MASK_DETACHED_ATTACHED;
	ret = ptn5150h_write_reg(PTN5150H_REG_CONTROL, reg_val);
	if (ret < 0)
		return ret;

	return 0;
}

static int ptn5150h_device_check(void)
{
	u8 reg_val = 0;

	return ptn5150h_read_reg(PTN5150H_REG_DEVICE_ID, &reg_val);
}

static int ptn5150h_host_current_mode(u8 val)
{
	u8 reg_val = 0;
	u8 mask_val;
	u8 current_set_val;
	int ret;

	ret = ptn5150h_read_reg(PTN5150H_REG_CONTROL, &reg_val);
	if (ret < 0)
		return ret;

	mask_val = reg_val & (~PTN5150H_REG_RP_MODE);
	current_set_val = reg_val & PTN5150H_REG_RP_MODE;

	if (val == current_set_val) {
		hwlog_info("current mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = ptn5150h_write_reg(PTN5150H_REG_CONTROL, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int ptn5150h_ctrl_output_current(int value)
{
	switch (value) {
	case TYPEC_HOST_CURRENT_DEFAULT:
		hwlog_info("set to default current\n");
		ptn5150h_host_current_mode(PTN5150H_REG_RP_DEFAULT);
		break;

	case TYPEC_HOST_CURRENT_MID:
		hwlog_info("set to medium current\n");
		ptn5150h_host_current_mode(PTN5150H_REG_RP_MEDIUM);
		break;

	case TYPEC_HOST_CURRENT_HIGH:
		hwlog_info("set to high current\n");
		ptn5150h_host_current_mode(PTN5150H_REG_RP_HIGH);
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static int ptn5150h_host_port_mode(u8 val)
{
	u8 reg_val;
	u8 mask_val;
	u8 current_set_val;
	int ret;

	ret = ptn5150h_read_reg(PTN5150H_REG_CONTROL, &reg_val);
	if (ret < 0)
		return ret;

	mask_val = reg_val & (~PTN5150H_REG_PORT_MODE);
	current_set_val = reg_val & PTN5150H_REG_PORT_MODE;

	if (val == current_set_val) {
		hwlog_info("port mode is same as setting\n");
		return 0;
	}

	val |= mask_val;
	ret = ptn5150h_write_reg(PTN5150H_REG_CONTROL, val);
	if (ret < 0)
		return ret;

	return 0;
}

static void ptn5150h_detect_rp_rd(void)
{
	u8 val = 0;
	u8 val_1 = 0;
	u8 val3 = 0;
	u8 val19 = 0;
	int rp_miss_count = 0;
	int delay_count = 0;

	do {
		hwlog_info("goto do-while loop\n");

		/* detect Rp on CC line */
		ptn5150h_read_reg(PTN5150H_REG_DETECT_RP, &val_1);
		val_1 &= (PTN5150H_REG_RP1_DETECT | PTN5150H_REG_RP2_DETECT);
		/*
		 * only one Rp on CC line,
		 * one DFP is connected to us,wait for Vbus
		 */
		if ((val_1 == PTN5150H_REG_RP1_DETECT) ||
			(val_1 == PTN5150H_REG_RP2_DETECT)) {
			rp_miss_count = 0;
			hwlog_info("only one Rp\n");
		} else if ((val_1 == (PTN5150H_REG_RP1_DETECT |
			PTN5150H_REG_RP2_DETECT)) || (val_1 == 0x00)) {
			/* no Rp on CC line. detect 3 times. */
			rp_miss_count++;
			hwlog_info("no Rp or 2 Rp, %d\n", rp_miss_count);
		}

		ptn5150h_read_reg(PTN5150H_REG_CC_STATUS, &val);
		val = val & (PTN5150H_REG_PORT_ATTACH_STATUS_DA_ATTACHED |
			PTN5150H_REG_PORT_ATTACH_STATUS_UFP_ATTACHED |
			PTN5150H_REG_PORT_ATTACH_STATUS_DFP_ATTACHED);

		hwlog_info("check register 04 %d\n", val);

		/*
		 * Vbus comes,
		 * reg 0x04[4:2]=001 means one DFP is connected to us.
		 */
		if (val == PTN5150H_REG_PORT_ATTACH_STATUS_DFP_ATTACHED) {
			rp_miss_count = RP_MISS_COUNT_MAX;
			hwlog_info("we are UFP\n");
		}

		/*
		 * if 3 times no Rp, there is a UFP connect to us
		 */
		if (rp_miss_count == (RP_MISS_COUNT_MAX - 1)) {
			hwlog_info("we are DFP\n");
			/* enable accessory detection */
			ptn5150h_write_reg(PTN5150H_REG_ACC1,
				PTN5150H_ENABLE_ACC1);
			/* enable accessory detection */
			ptn5150h_write_reg(PTN5150H_REG_ACC2,
				PTN5150H_ENABLE_ACC2);

			/* set to DFP mode,
			 * disable interrupt attach/detach
			 */
			ptn5150h_write_reg(PTN5150H_REG_CONTROL,
				PTN5150H_REG_RP_DEFAULT |
				PTN5150H_REG_MODE_DFP |
				PTN5150H_REG_INT_MASK_DETACHED_ATTACHED);
			/*
			 * because from UFP to DFP,
			 * there would be a period that Vbus=0.
			 * no connection
			 */
			mdelay(PTN5150H_VBUS_DETECT_TIME);
			ptn5150h_read_reg(PTN5150H_REG_INT_STATUS, &val3);
			ptn5150h_read_reg(PTN5150H_REG_INT_REG_STATUS, &val19);
		} else if (rp_miss_count < RP_MISS_COUNT_MAX - 1) {
			/* delay 10ms to loop detect Rp */
			mdelay(PTN5150H_RP_DETECT_TIME);
			delay_count++;
		}
	} while ((rp_miss_count < RP_MISS_COUNT_MAX - 1) &&
		(delay_count < DELAY_COUNT_MAX));
}

static int ptn5150h_ctrl_port_mode(int value)
{
	u8 val = 0;
	u8 val3 = 0;
	u8 val19 = 0;

	switch (value) {
	case TYPEC_HOST_PORT_MODE_DFP:
		hwlog_info("set to dfp mode\n");
		/* set to DFP and disable interrupt */
		ptn5150h_write_reg(PTN5150H_REG_INT_MASK,
			PTN5150H_REG_ORIENTATION_FOUND_MASK |
			PTN5150H_REG_ROLE_CHANGE_MASK |
			PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK);
		/* mask interrupt */
		ptn5150h_write_reg(PTN5150H_REG_CONTROL,
			PTN5150H_REG_INT_MASK_DETACHED_ATTACHED |
			PTN5150H_REG_MODE_DFP |
			PTN5150H_REG_RP_DEFAULT);
		/* delay 50ms for disconnection interrupt */
		mdelay(PTN5150H_DISCONNECTION_INTERRUPT_DELAY);
		/* clear interrupt */
		ptn5150h_read_reg(PTN5150H_REG_INT_REG_STATUS, &val);
		/* clear interrupt */
		ptn5150h_read_reg(PTN5150H_REG_INT_STATUS, &val);
		/* delay 50ms for attach */
		mdelay(PTN5150H_ATTACH_DELAY);
		/* enable the interrupt */
		ptn5150h_write_reg(PTN5150H_REG_CONTROL,
			PTN5150H_REG_RP_DEFAULT | PTN5150H_REG_MODE_DFP);
		break;

	case TYPEC_HOST_PORT_MODE_UFP:
		ptn5150h_read_reg(PTN5150H_REG_INT_MASK, &val);
		hwlog_info("REG_INT_MASK = %d\n", val);

		/* after try sink, reg 0x18 equals 0x1C */
		if (val != (PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK |
			PTN5150H_REG_ROLE_CHANGE_MASK |
			PTN5150H_REG_ORIENTATION_FOUND_MASK)) {
			hwlog_info("set to UFP mode\n");

			/* disable accessory detech */
			ptn5150h_write_reg(PTN5150H_REG_ACC1,
				PTN5150H_DISABLE_ACC1);
			/* disable accessory detech */
			ptn5150h_write_reg(PTN5150H_REG_ACC2,
				PTN5150H_DISABLE_ACC2);
			/*
			 * force to ufp mode,
			 * disable interrupt-- atttach /detach
			 */
			ptn5150h_write_reg(PTN5150H_REG_CONTROL,
				PTN5150H_REG_RP_DEFAULT |
				PTN5150H_REG_INT_MASK_DETACHED_ATTACHED);
			/* only enable oriental interrupt */
			ptn5150h_write_reg(PTN5150H_REG_INT_MASK,
				PTN5150H_REG_ORIENTATION_FOUND_MASK |
				PTN5150H_REG_ROLE_CHANGE_MASK |
				PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK);
			/* delay 50ms for disconnect interrupt */
			mdelay(PTN5150H_DISCONNECTION_INTERRUPT_DELAY);
			/* clear interrupt */
			ptn5150h_read_reg(PTN5150H_REG_INT_STATUS, &val3);
			/* clear interrupt */
			ptn5150h_read_reg(PTN5150H_REG_INT_REG_STATUS, &val19);
			/* wait for debounce time */
			mdelay(PTN5150H_DEBOUNCE_DELAY);

			ptn5150h_detect_rp_rd();
		} else {
			hwlog_info("reverse here\n");
			ptn5150h_write_reg(PTN5150H_REG_CONTROL,
				PTN5150H_REG_INT_MASK_DETACHED_ATTACHED |
				PTN5150H_REG_RP_DEFAULT);
			/* delay 50ms for disconnect interrupt */
			mdelay(PTN5150H_DISCONNECTION_INTERRUPT_DELAY);
			/* clear interrupt */
			ptn5150h_read_reg(PTN5150H_REG_INT_REG_STATUS,
				&val19);
			/* clear interrupt */
			ptn5150h_read_reg(PTN5150H_REG_INT_STATUS, &val3);

			mdelay(PTN5150H_UFP_DELAY);/* 20160510 */
			/* force to ufp */
			ptn5150h_write_reg(PTN5150H_REG_CONTROL,
			PTN5150H_REG_RP_DEFAULT);
		}
		break;

	case TYPEC_HOST_PORT_MODE_DRP:
		hwlog_info("set to drp mode\n");
		ptn5150h_write_reg(PTN5150H_REG_ACC1, PTN5150H_ENABLE_ACC1);
		ptn5150h_write_reg(PTN5150H_REG_ACC2, PTN5150H_ENABLE_ACC2);
		ptn5150h_host_port_mode(PTN5150H_REG_MODE_DRP);
		ptn5150h_write_reg(PTN5150H_REG_INT_MASK,
			PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK |
			PTN5150H_REG_ROLE_CHANGE_MASK |
			PTN5150H_REG_ORIENTATION_FOUND_MASK |
			PTN5150H_REG_DA_FOUND_MASK |
			PTN5150H_REG_AA_FOUND_MASK); /* change to 0x1f */
		break;

	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0;
}

static void ptn5150h_mask_current_interrupt(bool enable)
{
	u8 reg_val = 0;

	ptn5150h_read_reg(PTN5150H_REG_INT_MASK, &reg_val);
	reg_val = enable ?
		(reg_val & (~PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK)) :
		(reg_val | PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK);

	ptn5150h_write_reg(PTN5150H_REG_INT_MASK, reg_val);
}

static int ptn5150h_detect_attachment_status(void)
{
	int ret;
	u8 reg_val = 0;
	u8 reg_val2 = 0;
	u8 port_mode;
	int reg_status;
	struct typec_device_info *di = g_ptn5150h_dev;

	ptn5150h_read_reg(PTN5150H_REG_CONTROL, &reg_val2);
	ptn5150h_write_reg(PTN5150H_REG_CONTROL,
		reg_val2 | PTN5150H_REG_INT_MASK_DETACHED_ATTACHED); /* 0x01 */
	/* read and clear the attach/detach interrupt */
	ret = ptn5150h_read_reg(PTN5150H_REG_INT_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	/* read and clear the other cc status interrupt */
	ret = ptn5150h_read_reg(PTN5150H_REG_INT_REG_STATUS, &reg_val2);
	if (ret < 0)
		return ret;

	reg_status = ptn5150h_detect_input_current();
	if (reg_val & PTN5150H_REG_CABLE_ATTACH_INT) {
		port_mode = ptn5150h_detect_port_mode();
		if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
			input_current = reg_status;
			ptn5150h_mask_current_interrupt(true);
			hwlog_info("attach different attach status\n");
		}
		di->dev_st.attach_status = TYPEC_ATTACH;
	} else if (reg_val & PTN5150H_REG_CABLE_DETACH_INT) {
		hwlog_info("ptn5150h detach\n");
		di->dev_st.attach_status = TYPEC_DETACH;
		ptn5150h_mask_current_interrupt(false);
		input_current = -1;
	} else {
		port_mode = ptn5150h_detect_port_mode();
		if (port_mode == TYPEC_DEV_PORT_MODE_UFP) {
			if ((input_current != reg_status) && reg_status != -1) {
				input_current = reg_status;
				di->dev_st.attach_status =
					TYPEC_CUR_CHANGE_FOR_FSC;
				ptn5150h_mask_current_interrupt(true);
				hwlog_info("typec current change\n");
			} else {
				di->dev_st.attach_status =
					TYPEC_STATUS_NOT_READY;
			}
		} else {
			hwlog_err("wrong interrupt\n");
			di->dev_st.attach_status = TYPEC_STATUS_NOT_READY;
		}
	}

	return di->dev_st.attach_status;
}

static int ptn5150h_detect_cc_orientation(void)
{
	int ret;
	u8 reg_val = 0;
	u8 cc_val;
	struct typec_device_info *di = g_ptn5150h_dev;

	ret = ptn5150h_read_reg(PTN5150H_REG_CC_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	cc_val = reg_val & PTN5150H_REG_CC_POPARITY_CC_STATUS;

	if (cc_val == PTN5150H_REG_CC_POPARITY_CC2_CONNECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_CC2;
		hwlog_info("cc2 detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == PTN5150H_REG_CC_POPARITY_CC1_CONNECTED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_CC1;
		hwlog_info("cc1 detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else if (cc_val == PTN5150H_REG_CC_POPARITY_CABLE_NOT_ATTACHED) {
		di->dev_st.cc_orient = TYPEC_ORIENT_DEFAULT;
		hwlog_info("no cc has been detected, cc_orient=%d\n",
			di->dev_st.cc_orient);
	} else{
		di->dev_st.cc_orient = TYPEC_ORIENT_NOT_READY;
		hwlog_info("there is a cc detection fault, cc_orient=%d\n",
			di->dev_st.cc_orient);
	}

	return di->dev_st.cc_orient;
}

static int ptn5150h_detect_port_mode(void)
{
	int ret;
	u8 reg_val = 0;
	u8 port_val;
	struct typec_device_info *di = g_ptn5150h_dev;

	ret = ptn5150h_read_reg(PTN5150H_REG_CC_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	port_val = reg_val & PTN5150H_REG_PORT_ATTACH_STATUS_AAA_ATTACHED;

	if (port_val == PTN5150H_REG_PORT_ATTACH_STATUS_UFP_ATTACHED) {
		hwlog_info("ufp otg detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DFP;
	} else if (port_val == PTN5150H_REG_PORT_ATTACH_STATUS_DFP_ATTACHED) {
		hwlog_info("dfp host detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_UFP;
	} else if (port_val == PTN5150H_REG_PORT_ATTACH_STATUS_DA_ATTACHED) {
		hwlog_info("debug accessory is detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_DEBUGACC;
	} else if (port_val == PTN5150H_REG_PORT_ATTACH_STATUS_AAA_ATTACHED) {
		hwlog_info("audio accessory is detected\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_AUDIOACC;
	} else {
		hwlog_info("detect port mode error\n");
		di->dev_st.port_mode = TYPEC_DEV_PORT_MODE_NOT_READY;
	}

	return di->dev_st.port_mode;
}

static int ptn5150h_detect_input_current(void)
{
	int ret;
	u8 reg_val = 0;
	u8 current_detect_val;
	struct typec_device_info *di = g_ptn5150h_dev;

	ret = ptn5150h_read_reg(PTN5150H_REG_CC_STATUS, &reg_val);
	if (ret < 0)
		return ret;

	current_detect_val = reg_val & PTN5150H_REG_RP_DETECTION_CUR_MODE;

	switch (current_detect_val) {
	case PTN5150H_REG_RP_DETECTION_3A:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_HIGH;
		hwlog_info("detected type c current is 3A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case PTN5150H_REG_RP_DETECTION_1_5A:
		di->dev_st.input_current = TYPEC_DEV_CURRENT_MID;
		hwlog_info("detected type c current is 1.5A, iin=%d\n",
			di->dev_st.input_current);
		break;

	case PTN5150H_REG_RP_DETECTION_STDUSB:
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

struct typec_device_ops ptn5150h_ops = {
	.clean_int_mask = ptn5150h_clean_mask,
	.detect_attachment_status = ptn5150h_detect_attachment_status,
	.detect_cc_orientation = ptn5150h_detect_cc_orientation,
	.detect_input_current = ptn5150h_detect_input_current,
	.detect_port_mode = ptn5150h_detect_port_mode,
	.ctrl_output_current = ptn5150h_ctrl_output_current,
	.ctrl_port_mode = ptn5150h_ctrl_port_mode,
};

static ssize_t dump_regs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i, index;
	u8 reg_val = 0;
	const int regaddr[PTN5150H_DUMP_REG_NUM] = {
		0x01, 0x02, 0x03, 0x04, 0x09, 0x0A, 0x10, 0x18, 0x19
	};
	const char str[] = "0123456789abcdef";

	/* if there is no register value, replace it with xx */
	for (i = 0; i < 3 * PTN5150H_REGISTER_NUM; i++) {
		if (2 == (i % 3))
			buf[i] = ' ';
		else
			buf[i] = 'x';
	}

	buf[0x2f] = '\n'; /* reg 0x2f change line for better print type */
	buf[0x5f] = '\0'; /* reg 0x5f to end */

	for (i = 0; i < PTN5150H_DUMP_REG_NUM; i++) {
		index = regaddr[i];
		ptn5150h_read_reg(index, &reg_val);
		buf[3 * (long)index] = str[(reg_val & 0xf0) >> 4];
		buf[3 * (long)index + 1] = str[reg_val & 0x0f];
		buf[3 * (long)index + 2] = ' ';
	}

	return 0x60; /* 0x60 is default size */
}

static DEVICE_ATTR(dump_regs, 0444, dump_regs_show, NULL);

static struct attribute *ptn5150h_attributes[] = {
	&dev_attr_dump_regs.attr,
	NULL,
};

static const struct attribute_group ptn5150h_attr_group = {
	.attrs = ptn5150h_attributes,
};

static int ptn5150h_create_sysfs(void)
{
	int ret = 0;
	struct class *typec_class = NULL;
	struct device *new_dev = NULL;

	typec_class = hw_typec_get_class();
	if (typec_class) {
		new_dev = device_create(typec_class, NULL, 0, NULL, "ptn5150h");
		if (IS_ERR(new_dev)) {
			hwlog_err("sysfs device create failed\n");
			return PTR_ERR(new_dev);
		}

		ret = sysfs_create_group(&new_dev->kobj, &ptn5150h_attr_group);
		if (ret)
			hwlog_err("sysfs group create failed\n");
	}

	return ret;
}

static void ptn5150h_remove_sysfs(struct typec_device_info *di)
{
	device_remove_file(&di->client->dev, &dev_attr_dump_regs);
}

static irqreturn_t ptn5150h_irq_handler(int irq, void *dev_id)
{
	int gpio_value_intb;
	struct typec_device_info *di = dev_id;

	hwlog_info("ptn5150h int happened\n");

	gpio_value_intb = gpio_get_value(di->gpio_intb);
	if (gpio_value_intb == 1) /* 1: gpio is high */
		hwlog_err("intb=%d when interrupt occured\n", gpio_value_intb);

	typec_wake_lock(di);
	schedule_work(&di->g_intb_work);

	return IRQ_HANDLED;
}

static void ptn5150h_initialization(void)
{
	u8 reg_val = 0;
	u8 reg_val04 = 0;

	ptn5150h_read_reg(PTN5150H_INTERNAL_REG_SW_TDRPSWAP_RP, &reg_val);
	hwlog_info("duty cycle reg 4f %d\n", reg_val);

	ptn5150h_read_reg(PTN5150H_INTERNAL_REG_SW_TDRPSWAP_RD, &reg_val);
	hwlog_info("duty cycle reg 51 %d\n", reg_val);

	ptn5150h_write_reg(PTN5150H_INTERNAL_REG_SW_TDRPSWAP_RP,
		PTN5150H_SET_DUTY_CYCLE_RP_PRESENT_TIME);
	ptn5150h_write_reg(PTN5150H_INTERNAL_REG_SW_TDRPSWAP_RD,
		PTN5150H_SET_DUTY_CYCLE_RD_PRESENT_TIME);
	ptn5150h_read_reg(PTN5150H_INTERNAL_REG_SW_TDRPSWAP_RP, &reg_val);
	hwlog_info("duty cycle reg 4f again %d\n", reg_val);

	ptn5150h_read_reg(PTN5150H_INTERNAL_REG_SW_TDRPSWAP_RD, &reg_val);
	hwlog_info("duty cycle reg 51 again %d\n", reg_val);

	ptn5150h_read_reg(PTN5150H_REG_CC_STATUS, &reg_val04);
	hwlog_info("duty cycle reg 04 again %d\n", reg_val04);

	ptn5150h_read_reg(PTN5150H_REG_INT_STATUS, &reg_val);
	hwlog_info("duty cycle reg 03 again %d\n", reg_val);

	ptn5150h_read_reg(PTN5150H_REG_INT_REG_STATUS, &reg_val);
	hwlog_info("duty cycle reg 19 again %d\n", reg_val);

	ptn5150h_read_reg(PTN5150H_REG_CONTROL, &reg_val);
	hwlog_info("duty cycle reg 02 again %d\n", reg_val);

	/*
	 * if power on there is no dfp detach,
	 * then set port to drp else keep as ufp
	 */
	if ((reg_val04 & (PTN5150H_REG_PORT_ATTACH_STATUS_DFP_ATTACHED |
		PTN5150H_REG_PORT_ATTACH_STATUS_UFP_ATTACHED |
		PTN5150H_REG_PORT_ATTACH_STATUS_DA_ATTACHED)) !=
		PTN5150H_REG_PORT_ATTACH_STATUS_DFP_ATTACHED)
		ptn5150h_ctrl_port_mode(TYPEC_HOST_PORT_MODE_DRP);

	ptn5150h_clean_mask();
	ptn5150h_write_reg(PTN5150H_REG_ACC1, PTN5150H_ENABLE_ACC1);
	ptn5150h_write_reg(PTN5150H_REG_ACC2, PTN5150H_ENABLE_ACC2);
	ptn5150h_write_reg(PTN5150H_REG_INT_MASK,
		PTN5150H_REG_CC_COMPARATOR_CHANGE_MASK |
		PTN5150H_REG_ROLE_CHANGE_MASK |
		PTN5150H_REG_ORIENTATION_FOUND_MASK |
		PTN5150H_REG_DA_FOUND_MASK |
		PTN5150H_REG_AA_FOUND_MASK); /* change to 0x1f */
}

static int ptn5150h_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	unsigned int gpio_enb_val = 1;
	struct typec_device_info *di = NULL;
	struct typec_device_info *pdi = NULL;
	struct device_node *node = NULL;
	unsigned int typec_trigger_otg = 0;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_ptn5150h_dev = di;
	di->dev = &client->dev;
	di->client = client;
	node = di->dev->of_node;
	i2c_set_clientdata(client, di);

	di->gpio_enb = of_get_named_gpio(node, "ptn5150h_typec,gpio_enb", 0);
	hwlog_info("gpio_enb=%d\n", di->gpio_enb);

	if (!gpio_is_valid(di->gpio_enb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_check_i2c;
	}

	ret = gpio_request(di->gpio_enb, "ptn5150h_en");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto fail_check_i2c;
	}

	ret = of_property_read_u32(node, "ptn5150h_gpio_enb", &gpio_enb_val);
	if (ret) {
		hwlog_err("ptn5150h_gpio_enb dts read failed\n");
		goto fail_free_gpio_enb;
	}
	hwlog_info("gpio_enb_val=%d\n", gpio_enb_val);

	ret = gpio_direction_output(di->gpio_enb, gpio_enb_val);
	if (ret) {
		hwlog_err("gpio set output fail\n");
		goto fail_free_gpio_enb;
	}

	ret = ptn5150h_device_check();
	if (ret) {
		hwlog_err("the chip is not ptn5150h\n");
		goto fail_free_gpio_enb;
	}

	of_property_read_u32(node, "typec_trigger_otg", &typec_trigger_otg);
	hwlog_info("typec_trigger_otg=%d\n", typec_trigger_otg);
	di->typec_trigger_otg = !!typec_trigger_otg;

	di->gpio_intb = of_get_named_gpio(node, "ptn5150h_typec,gpio_intb", 0);
	hwlog_info("gpio_intb=%d\n", di->gpio_intb);

	if (!gpio_is_valid(di->gpio_intb)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	pdi = typec_chip_register(di, &ptn5150h_ops, THIS_MODULE);
	if (!pdi) {
		hwlog_err("typec register chip error\n");
		ret = -EINVAL;
		goto fail_free_gpio_enb;
	}

	ret = gpio_request(di->gpio_intb, "ptn5150h_int");
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

	ptn5150h_initialization();

	ret = request_irq(di->irq_intb, ptn5150h_irq_handler,
			IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING,
			"ptn5150h_int", pdi);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_intb = -1;
		goto fail_free_gpio_intb;
	}

	ret = ptn5150h_create_sysfs();
	if (ret)
		goto fail_create_sysfs;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_TYPEC);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	hwlog_info("probe end\n");
	return 0;

fail_create_sysfs:
	ptn5150h_remove_sysfs(di);
	free_irq(di->gpio_intb, di);
fail_free_gpio_intb:
	gpio_free(di->gpio_intb);
fail_free_gpio_enb:
	gpio_free(di->gpio_enb);
fail_check_i2c:
	devm_kfree(&client->dev, di);
	g_ptn5150h_dev = NULL;

	return ret;
}

static int ptn5150h_remove(struct i2c_client *client)
{
	struct typec_device_info *di = i2c_get_clientdata(client);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	ptn5150h_remove_sysfs(di);
	free_irq(di->irq_intb, di);
	gpio_set_value(di->gpio_enb, 1);
	gpio_free(di->gpio_enb);
	gpio_free(di->gpio_intb);
	g_ptn5150h_dev = NULL;
	devm_kfree(&client->dev, di);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id typec_ptn5150h_ids[] = {
	{ .compatible = "huawei,ptn5150h" },
	{},
};
MODULE_DEVICE_TABLE(of, typec_ptn5150h_ids);

static const struct i2c_device_id ptn5150h_i2c_id[] = {
	{ "ptn5150h", 0 },
	{},
};

static struct i2c_driver ptn5150h_i2c_driver = {
	.driver = {
		.name = "ptn5150h",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(typec_ptn5150h_ids),
	},
	.probe = ptn5150h_probe,
	.remove = ptn5150h_remove,
	.id_table = ptn5150h_i2c_id,
};

static __init int ptn5150h_i2c_init(void)
{
	return i2c_add_driver(&ptn5150h_i2c_driver);
}

static __exit void ptn5150h_i2c_exit(void)
{
	i2c_del_driver(&ptn5150h_i2c_driver);
}

module_init(ptn5150h_i2c_init);
module_exit(ptn5150h_i2c_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei ptn5150h typec module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
