/*
 * sc200x.h
 *
 * sc200x driver
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#ifndef _SC200X_H_
#define _SC200X_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>

enum sc200x_status {
	SC200X_FAILED = -1,
	SC200X_SUCCESS = 0,
	SC200X_STA_NACK,
	SC200X_STA_NORSP,
	SC200X_RETRY
};

enum sc200x_device_id {
	SC200X_ID_SC2001 = 0,
	SC200X_ID_SC2004
};

struct sc200x_device_info {
	struct device *dev;
	struct device *sysfs_dev;
	struct i2c_client *client;
	struct work_struct irq_work;
	struct wakeup_source wakelock;
	struct mutex accp_detect_lock;
	struct mutex accp_adpt_reg_lock;
	struct completion accp_rsp_completion;
	struct completion accp_detect_completion;
	int irq_int;
	int irq_gpio;
	int device_id;
	int power_role;
	unsigned int rp_value;
	unsigned int src_enable;
	unsigned int fcp_support;
	unsigned int scp_support;
	unsigned int pd_support;
};

/* reg=0x11, r, device type */
#define SC200X_REG_DEV_TYPE             0x11

#define SC200X_DEV_TYPE_MASK            (BIT(7) | BIT(6))
#define SC200X_DEV_TYPE_SHIFT           6

#define SC200X_DEV_TYPE_SDP             0x01
#define SC200X_DEV_TYPE_CDP             0x02
#define SC200X_DEV_TYPE_DCP             0x03

/* reg=0x1E, r, version info */
#define SC200X_REG_FW_VER               0x1E
#define SC200X_FW_MAJOR_VER_MASK        (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define SC200X_FW_MAJOR_VER_SHIFT       4
#define SC200X_FW_MINOR_VER_MASK        (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SC200X_FW_MINOR_VER_SHIFT       0

/* reg=0x1F, r, device id */
#define SC200X_REG_ID                   0x1F
#define SC200X_DEVICE_ID_MASK           (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3))
#define SC200X_DEVICE_ID_SHIFT          3
#define SC200X_VENDOR_ID_MASK           (BIT(2) | BIT(1) | BIT(0))
#define SC200X_VENDOR_ID_SHIFT          0

/* reg=0x20, rw, chip alert */
#define SC200X_REG_ALERT                0x20

#define SC200X_ALERT_ACCP_DETECT_MASK   BIT(2)
#define SC200X_ALERT_ACCP_DETECT_SHIFT  2
#define SC200X_ALERT_ACCP_MSG_MASK      BIT(1)
#define SC200X_ALERT_ACCP_MSG_SHIFT     1
#define SC200X_ALERT_CC_STATUS_MASK     BIT(0)
#define SC200X_ALERT_CC_STATUS_SHIFT    0

#define SC200X_ALERT_ACCP_MASK          (BIT(2) | BIT(1))
#define SC200X_ALERT_ACCP_SHIFT         1

/* reg=0x21, rw, device control */
#define SC200X_REG_DEV_CTRL             0x21

#define SC200X_ACCP_EN_MASK             BIT(4)
#define SC200X_ACCP_EN_SHIFT            4
#define SC200X_RST_CTRL_MASK            (BIT(3) | BIT(2))
#define SC200X_RST_CTRL_SHIFT           2
#define SC200X_PD_CMD_MASK              BIT(1)
#define SC200X_PD_CMD_SHIFT             1
#define SC200X_ACCP_CMD_MASK            BIT(0)
#define SC200X_ACCP_CMD_SHIFT           0

#define SC200X_RST_CTRL_ACCP_MSTR       0x01
#define SC200X_RST_CTRL_ACCP_SLAVE      0x02

/* reg=0x0F, rw, pmu control */
#define SC200X_REG_PMU_CTRL             0x0F

#define SC200X_PMU_CTRL_VALUE0          0x5D
#define SC200X_PMU_CTRL_VALUE1          0x0F

#endif /* _SC200X_H_ */
