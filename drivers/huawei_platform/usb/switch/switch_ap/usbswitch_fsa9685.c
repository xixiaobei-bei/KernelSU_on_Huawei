/*
 * usbswitch_fsa9685.c
 *
 * usbswitch with fsa9685 driver
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

#include <huawei_platform/usb/switch/switch_fsa9685.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/param.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <huawei_platform/log/hw_log.h>
#include "switch_chip.h"
#include "switch_chip_i2c.h"

#define HWLOG_TAG usbswitch_fsa9685
HWLOG_REGIST();

static const int usbswitch_fsa9685_regaddrs[] = {
	FSA9685_REG_DEVICE_ID,
	FSA9685_REG_CONTROL,
	/* FSA9685_REG_INTERRUPT, */
	FSA9685_REG_INTERRUPT_MASK,
	FSA9685_REG_ADC,
	FSA9685_REG_TIMING_SET_1,
	FSA9685_REG_DETACH_CONTROL,
	FSA9685_REG_DEVICE_TYPE_1,
	FSA9685_REG_DEVICE_TYPE_2,
	FSA9685_REG_DEVICE_TYPE_3,
	FSA9685_REG_MANUAL_SW_1,
	FSA9685_REG_TIMING_SET_2,
};

static void usbswitch_fsa9685_detach_work(void)
{
	int ret;
	struct fsa9685_device_info *di = switch_get_dev_info();

	if (!di)
		return;

	hwlog_info("detach_work entry\n");

	ret = switch_read_reg(di->client, FSA9685_REG_DETACH_CONTROL);
	if (ret < 0)
		return;

	ret = switch_write_reg(di->client, FSA9685_REG_DETACH_CONTROL, 1);
	if (ret < 0)
		return;

	hwlog_info("detach_work end\n");
}

static int usbswitch_fsa9685_manual_switch(int input_select)
{
	int value;
	int ret;
	struct fsa9685_device_info *di = switch_get_dev_info();

	if (!di)
		return -EINVAL;

	switch (input_select) {
	case FSA9685_USB1_ID_TO_IDBYPASS:
		value = REG_VAL_FSA9685_USB1_ID_TO_IDBYPASS;
		break;
	case FSA9685_USB2_ID_TO_IDBYPASS:
		value = REG_VAL_FSA9685_USB2_ID_TO_IDBYPASS;
		break;
	case FSA9685_UART_ID_TO_IDBYPASS:
		value = REG_VAL_FSA9685_UART_ID_TO_IDBYPASS;
		break;
	case FSA9685_USB1_ID_TO_VBAT:
		value = REG_VAL_FSA9685_USB1_ID_TO_VBAT;
		break;
	case FSA9685_OPEN:
	default:
		value = REG_VAL_FSA9685_OPEN;
		break;
	}

	ret = switch_write_reg(di->client, FSA9685_REG_MANUAL_SW_1, value);
	if (ret < 0) {
		ret = -ERR_FSA9685_REG_MANUAL_SW_1;
		return ret;
	}

	value = switch_read_reg(di->client, FSA9685_REG_CONTROL);
	if (value < 0) {
		ret = -ERR_FSA9685_READ_REG_CONTROL;
		return ret;
	}

	value &= (~FSA9685_MANUAL_SW); /* 0: manual switching */

	ret = switch_write_reg(di->client, FSA9685_REG_CONTROL, value);
	if (ret < 0) {
		ret = -ERR_FSA9685_WRITE_REG_CONTROL;
		return ret;
	}

	return 0;
}

static int usbswitch_fsa9685_switchctrl_store(struct i2c_client *client,
	int action)
{
	switch (action) {
	case MANUAL_DETACH:
		hwlog_info("manual_detach\n");
		usbswitch_fsa9685_detach_work();
		break;
	case MANUAL_SWITCH:
		hwlog_info("manual_switch(usb1_id_to_vbat)\n");
		usbswitch_fsa9685_manual_switch(FSA9685_USB1_ID_TO_VBAT);
		break;
	default:
		hwlog_err("wrong input action\n");
		return -1;
	}

	return 0x60; /* 0x60 is default store size */
}

static int usbswitch_fsa9685_switchctrl_show(char *buf)
{
	int device_type1;
	int device_type2;
	int device_type3;
	int mode = -1;
	unsigned int tmp;
	struct fsa9685_device_info *di = switch_get_dev_info();

	if (!buf || !di) {
		hwlog_err("buf is null\n");
		return -1;
	}

	device_type1 = switch_read_reg(di->client, FSA9685_REG_DEVICE_TYPE_1);
	if (device_type1 < 0)
		goto fail_read_reg;

	device_type2 = switch_read_reg(di->client, FSA9685_REG_DEVICE_TYPE_2);
	if (device_type2 < 0)
		goto fail_read_reg;

	device_type3 = switch_read_reg(di->client, FSA9685_REG_DEVICE_TYPE_3);
	if (device_type3 < 0)
		goto fail_read_reg;

	hwlog_info("type1=0x%x type2=0x%x type3=0x%x\n",
		device_type1, device_type2, device_type3);

	/*
	 * because device_type1 device_type2 device_type3 are nonnegative val
	 * so it can be used in bit operation
	 * 16: word, 8: byte
	 */
	tmp = (device_type3 << 16) | (device_type2 << 8) | device_type1;
	mode = 0;
	while (tmp >> mode)
		mode++;

fail_read_reg:
	return scnprintf(buf, PAGE_SIZE, "%d\n", mode);
}

static int usbswitch_fsa9685_dump_regs(char *buf)
{
	int i;
	int val;
	char rd_buf[FSA9685_RD_BUF_SIZE] = {0};
	int size = ARRAY_SIZE(usbswitch_fsa9685_regaddrs);
	struct fsa9685_device_info *di = switch_get_dev_info();

	if (!di)
		return -EINVAL;

	scnprintf(rd_buf, FSA9685_RD_BUF_SIZE, "dump_regs: fsa9685\n");
	strncat(buf, rd_buf, strlen(rd_buf));

	for (i = 0; i < size; i++) {
		val = switch_read_reg(di->client, usbswitch_fsa9685_regaddrs[i]);

		memset(rd_buf, 0, FSA9685_RD_BUF_SIZE);
		scnprintf(rd_buf, FSA9685_RD_BUF_SIZE, "[0x%02x]: 0x%02x\n",
			usbswitch_fsa9685_regaddrs[i], val);
		strncat(buf, rd_buf, strlen(rd_buf));
	}

	return strlen(buf);
}

static struct fsa9685_device_ops usbswitch_fsa9685_ops = {
	.dump_regs = usbswitch_fsa9685_dump_regs,
	.switchctrl_store = usbswitch_fsa9685_switchctrl_store,
	.switchctrl_show = usbswitch_fsa9685_switchctrl_show,
	.manual_switch = usbswitch_fsa9685_manual_switch,
	.detach_work = usbswitch_fsa9685_detach_work,
};

struct fsa9685_device_ops *usbswitch_fsa9685_get_device_ops(void)
{
	return &usbswitch_fsa9685_ops;
}
