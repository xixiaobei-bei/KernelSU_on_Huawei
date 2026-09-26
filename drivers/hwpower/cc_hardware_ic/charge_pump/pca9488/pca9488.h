/*
 * pca9488.h
 *
 * charge-pump pca9488 macro, addr etc. (bp: bypass mode, cp: charge-pump mode)
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _PCA9488_H_
#define _PCA9488_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#define PCA9488_ADDR_LEN                1
#define PCA9488_REG_BIT_AI              BIT(7)

/* device id register */
#define PCA9488_DEVICE_ID_REG           0x00
#define PCA9488_CHIP_ID_A0              0x00

/* int device 0 mask register */
#define PCA9488_INT_DEV0_MASK_REG       0x05
#define PCA9488_INT_DEV0_MASK_VAL       0xC0

/* int device 1 mask register */
#define PCA9488_INT_DEV1_MASK_REG       0x06
#define PCA9488_INT_DEV1_MASK_VAL       0xA1
#define PCA9488_INT_DEV1_MASK_EN        0x00

/* int hv sc 0 mask register */
#define PCA9488_INT_HV_SC0_MASK_REG     0x07
#define PCA9488_INT_HV_SC0_MASK_VAL     0x80

/* int hv sc 1 mask register */
#define PCA9488_INT_HV_SC1_MASK_REG     0x08
#define PCA9488_INT_HV_SC1_MASK_VAL     0xF0

/* int device 0 status register */
#define PCA9488_DEV_STS_REG0            0x09
#define PCA9488_VBUS_IN_MIN_OK          BIT(0)
#define PCA9488_WPC_IN_MIN_OK           BIT(1)
#define PCA9488_WPC_IN_OK_PIN_HIGH      BIT(2)
#define PCA9488_VBUS_IN_SWITCH_OK       BIT(3)
#define PCA9488_VBUS_IN_SWITCH_NOT_OK   BIT(4)
#define PCA9488_SC_OUT_MIN_OK           BIT(5)

/* int hv sc 0 status register */
#define PCA9488_HV_SC_STS_REG0          0x0B
#define PCA9488_BYPASS_EN_MASK          BIT(1)
#define PCA9488_BYPASS_EN_SHIFT         1
#define PCA9488_SWITCHING_EN_MASK       BIT(2)
#define PCA9488_SWITCHING_EN_SHIFT      2

/* device control register 0 */
#define PCA9488_DEV_CTRL_REG0           0x0D
#define PCA9488_DEV_CTRL_REG0_VAL       0x01
#define PCA9488_INFET_EN_MASK           BIT(0)
#define PCA9488_INFET_EN_SHIFT          0
#define PCA9488_INFET_EN                1
#define PCA9488_INFET_DIS               0
#define PCA9488_INFET_FAULT_MASK        BIT(7)
#define PCA9488_INFET_FAULT_SHIFT       7
#define PCA9488_INFET_FAULT_VAL         0

/* switched capacitor converter control register 0 */
#define PCA9488_HV_SC_CTRL_REG0         0x0F
#define PCA9488_SC_OP_MODE_MASK         (BIT(1) | BIT(0))
#define PCA9488_SC_OP_MODE_SHIFT        0
#define PCA9488_SC_OP_OFF               0
#define PCA9488_SC_OP_BP                1
#define PCA9488_SC_OP_2TO1              2
#define PCA9488_SC_OP_1TO2              3
#define PCA9488_STANDBY_EN_MASK         BIT(2)
#define PCA9488_STANDBY_EN_SHIFT        2
#define PCA9488_STANDBY_EN              1
#define PCA9488_STANDBY_DIS             0

/* switched capacitor converter control register 1 */
#define PCA9488_HV_SC_CTRL_REG1         0x10
#define PCA9488_HV_SC_CTRL_REG1_VAL     0x3f
#define PCA9488_HV_SC_CTRL_MASK         (BIT(2) | BIT(1) | BIT(0))
#define PCA9488_HV_SC_CTRL_SHIFT        0
#define PCA9488_HV_SC_CTRL_VAL          7

/* switched capacitor converter control register 2 */
#define PCA9488_HV_SC_CTRL_REG2         0x11

/* pre-charge */
#define PCA9488_PC_SC_MASK              (BIT(5) | BIT(4) | BIT(3))
#define PCA9488_PC_SC_SHIFT             3
#define PCA9488_PC_SC_VAL               5 /* 101b, 260ma */

/* switched capacitor converter control register 3 */
#define PCA9488_HV_SC_CTRL_REG3         0x12
#define PCA9488_CP_MODE_MASK            BIT(7)
#define PCA9488_CP_MODE_SHIFT           7
#define PCA9488_CP_MODE_EN              1
#define PCA9488_CP_MODE_DIS             0

/* fault control register */
#define PCA9488_FAULT_CTRL_REG          0x14
#define PCA9488_FAULT_CTRL_VAL          0x50
#define PCA9488_VIN_SW_OK_EN_MASK       BIT(3)
#define PCA9488_VIN_SW_OK_EN_SHIFT      3
#define PCA9488_VIN_SW_OK_EN            0
#define PCA9488_VIN_SW_OK_DIS           1

/* track comparator control register */
#define PCA9488_TRACK_CTRL_REG          0x15
#define PCA9488_TRACK_CTRL_VAL          0x00
#define PCA9488_TRACK_CTRL_TIME_MASK    BIT(0)
#define PCA9488_TRACK_CTRL_TIME_SHIFT   0
#define PCA9488_TRACK_CTRL_TIME_VAL     1

/* drive capability register */
#define PCA9488_DRIVE_CAPABILITY_REG    0x16
#define PCA9488_DRIVE_CAPABILITY_VAL    0x01

/* enable hidden register */
#define PCA9488_ENABLE_HIDDEN_REG       0x19
#define PCA9488_ENABLE_HIDDEN_VAL       0x19

struct pca9488_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	u8 chip_id;
	int gpio_int;
	int irq_int;
};

#endif /* _PCA9488_H_ */
