/*
 * sc8502.h
 *
 * charge-pump sc8502 macro, addr etc. (bp: bypass mode, cp: charge-pump mode)
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

#ifndef _SC8502_H_
#define _SC8502_H_

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

#define SC8502_CHARGING_FWD               1
#define SC8502_CHARGING_REV               2
#define SC8502_CHARGING_REV_CP            3

#define SC8502_CP_EN                      1
#define SC8502_CP_DIS                     0

#define SC8502_QB_EN                      1
#define SC8502_QB_DIS                     0

#define SC8502_REV_CP_RETRY_TIME          150
#define SC8502_REV_CP_SLEEP_TIME          15

/* device id register */
#define SC8502_DEVICE_ID_REG              0x16
#define SC8502_DEVICE_ID_VAL              0xD0

/*
 * VOUT_OVP reg = 0x00
 * VOUT_OVP = 8V + VOUT_OVP[6:0]*50mV, default = 12V [b101 0000]
 */
#define SC8502_VOUT_OVP_REG                0x00
#define SC8502_VOUT_OV_REG_FWD_VAL         0xE4
#define SC8502_VOUT_OV_REG_REV_VAL         0x64

/*
 * PMID_OVP reg = 0x01
 * PMID_OVP = 12V + PMID_OVP[6:0]*100mV, default = 22V [b110 0100]
 */
#define SC8502_PMID_OVP_REG                0x01
#define SC8502_PMID_OV_REG_FWD_VAL         0xE4
#define SC8502_PMID_OV_REG_REV_VAL         0x64

/*
 * PMID2OUT_OVP_UVP reg = 0x03
 * PMID2OUT_OVP = 200mV + PMID2OUT_OVP[2:0]*50mV, default = 350mV [b011]
 * PMID2OUT_UVP = -100mV - PMID2OUT_UVP[5:3]*50mV, default = -100mV [b000]
 */
#define SC8502_PMID2OUT_OVP_UVP_REG        0x03
#define SC8502_PMID_VOUT_FWD_VAL           0x3F
#define SC8502_PMID_VOUT_FWD_CP_VAL        0x7F
#define SC8502_PMID_VOUT_REV_VAL           0xBF

/*
 * PMID2OUT_OCP_RCP reg = 0x04
 * PMID2OUT_OCP = 175mV + PMID2OUT_OCP[2:0]*25mV, default = 250mV [b011]
 * PMID2OUT_RCP = -75mV - PMID2OUT_RCP[5:3]*25mV, default = -100mV [b001]
 */
#define SC8502_PMID2OUT_OCP_RCP_REG        0x04
#define SC8502_PMID2OUT_OCP_RCP_INIT_VAL   0x1C

/*
 * IBUS_OCP reg = 0x05
 * IBUS_OCP = 1.2A + IBUS_OCP[2:0]*300mA, default = 2.4A [b100]
 */
#define SC8502_IBUS_OCP_REG                0x05
#define SC8502_IBUS_OCP_FWD_VAL            0x3C
#define SC8502_IBUS_OCP_REV_VAL            0xB6

/* CONTROL1 reg=0x07 */
#define SC8502_CTRL1_REG                   0x07
#define SC8502_CHARGE_EN_MASK              BIT(7)
#define SC8502_CHARGE_EN_SHIFT             7
#define SC8502_QB_EN_MARK                  BIT(6)
#define SC8502_QB_EN_SHITF                 6
#define SC8502_CTRL1_REG_FWD_INIT          0xC0
#define SC8502_CTRL1_REG_REV_INIT          0x80

/* CONTROL2 reg=0x08 */
#define SC8502_CTRL2_REG                   0x08
#define SC8502_CHANGE_MODE_MASK            (BIT(0) | BIT(1) | BIT(2))
#define SC8502_CHANGE_MODE_SHIFT           0
#define SC8502_CTRL2_REG_FWD_BP_INIT       0x80
#define SC8502_CTRL2_REG_REV_BP_INIT       0x82
#define SC8502_CTRL2_REG_REV_CP_INIT       0x83
#define SC8502_FWD_BP_MODE_VAL             0x00
#define SC8502_FWD_CP_MODE_VAL             0x01
#define SC8502_REV_BP_MODE_VAL             0x02
#define SC8502_REV_CP_MODE_VAL             0x03

/* CONTROL3 reg=0x09 */
#define SC8502_CTRL3_REG                   0x09
#define SC8502_CTRL3_REG_INIT              0x02

/* INT_STAT reg=0x0B */
#define SC8502_INT_STAT_REG                0x0B

/* FLT1_FLAG reg=0x0F */
#define SC8502_FLT1_FLAG_REG               0x0F

/* FLT2_FLAG reg=0x11 */
#define SC8502_FLT2_FLAG_REG               0x11

struct sc8502_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	u8 chip_id;
	int gpio_int;
	int irq_int;
};

#endif /* _SC8502_H_ */
