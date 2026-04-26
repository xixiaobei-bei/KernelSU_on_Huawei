/*
 * hl1512.h
 *
 * charge-pump hl1512 macro, addr etc. (bp: bypass mode, cp: charge-pump mode)
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

#ifndef _HL1512_H_
#define _HL1512_H_

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

#define HL1512_ADDR_LEN                1
#define HL1512_REG_BIT_AI              BIT(7)

#define HL1512_CHARGING_FWD            1
#define HL1512_CHARGING_REV            2
#define HL1512_CHARGING_REV_CP         3

#define HL1512_REV_CP_RETRY_TIME       150
#define HL1512_REV_CP_SLEEP_TIME       15

/* device id register */
#define HL1512_DEVICE_ID_REG           0x10
#define HL1512_DEVICE_ID               0xA0
#define HL1512Y_DEVICE_ID              0xA8

/* init a7 register */
#define HL1512_INIT_REGA7              0xA7
#define HL1512_INIT_REGA7_VAL          0xF9

/* init 53 register */
#define HL1512_INIT_REG53              0x53
#define HL1512_INIT_REG53_VAL          0x05

/* init 58 register */
#define HL1512_INIT_REG58              0x58
#define HL1512_INIT_REG58_VAL          0x82

/* mode register */
#define HL1512_MOD_REG                 0x00
#define HL1512_FWD_MOD_INIT_VAL        0x80
#define HL1512_REV_MOD_INIT_VAL        0x20
#define HL1512_REV_PMID_INIT_VAL       0x11
#define HL1512_REV_WPC_INIT_VAL        0x21
#define HL1512_REV_QRB_DIS_INIT_VAL    0x19
#define HL1512_REV_QRB_EN_INIT_VAL     0x29

/* force bypass / force cp register */
#define HL1512_FORCE_BPCP_MASK         (BIT(0) | BIT(1))
#define HL1512_FORCE_BPCP_SHIFT        0
#define HL1512_FORCE_BP_EN             0x00
#define HL1512_FORCE_CP_EN             0x01
#define HL1512_AUTO_BPCP_EN            0x02

/* control register */
#define HL1512_CONTROL_REG             0x01
#define HL1512_CONTROL_REG_VAL         0x06
#define HL1512_CONTROL_REV_CP_VAL      0x46
#define HL1512Y_CONTROL_REG_VAL        0x0A

/* pmid vout ov/uv register */
#define HL1512_PMID_VOUT_OV_UV_REG     0x02
#define HL1512_PMID_VOUT_FWD_VAL       0xFC
#define HL1512_PMID_VOUT_REV_VAL       0xFD
#define HL1512_PMID_VOUT_CP_VAL        0xFE

/* vout ov register */
#define HL1512_VOUT_OV_REG             0x03
#define HL1512_VOUT_OV_REG_FWD_VAL     0xFE
#define HL1512_VOUT_OV_REG_REV_VAL     0xFC
#define HL1512Y_VOUT_OV_REG_FWD_VAL    0xFA

/* watch dog & clock register */
#define HL1512_WD_CLOCK_REG            0x04
#define HL1512_WD_CLOCK_VAL            0x17
#define HL1512_WD_CLOCK_FREQ_VAL       0x1F

/* clock sync register */
#define HL1512_CLK_SYNC_REG            0x0B
#define HL1512_CLK_SYNC_OVP_VAL        0x02

/* current limit register */
#define HL1512_CUR_LIMIT_REG           0x0D
#define HL1512_CUR_LIMIT_MASK          (BIT(2) | BIT(3))
#define HL1512_CUR_LIMIT_QRB_DIS_VAL   0x0C
#define HL1512_CUR_LIMIT_DISABLE_VAL   0
#define HL1512_CUR_LIMIT_DEFAULT       3000 /* mA */
#define HL1512_CUR_LIMIT_FWD_VAL       0x9D
#define HL1512_CUR_LIMIT_REV_VAL       0x99
#define HL1512_CUR_LIMIT_REV_CP_VAL    0x98
#define HL1512Y_CUR_LIMIT_FWD_VAL      0x9D

/* temp alarm threshold register */
#define HL1512_TEMP_ALARM_TH_REG       0x0F
#define HL1512_TEMP_ALARM_TH_REG_VAL   0xC0

/* mode status register */
#define HL1512_STATUS_REG              0x06
#define HL1512_STATUS_REG_BP_VAL       0x02
#define HL1512_STATUS_REG_CP_VAL       0xE3
#define HL1512_STATUS_REG_REV_CP_VAL   0xFD
#define HL1512_STATUS_00_MASK          BIT(0)
#define HL1512_STATUS_01_MASK          BIT(1)
#define HL1512_STATUS_02_MASK          BIT(2)
#define HL1512_STATUS_00_SHIFT         1
#define HL1512_STATUS_01_SHIFT         2
#define HL1512_STATUS_02_SHIFT         3
#define HL1512_STATUS_MASK             0x07

struct hl1512_init_data {
	u8 mode_val;
	u8 control_val;
	u8 pmid_vout_ov_uv_val;
	u8 vout_ov_val;
	u8 current_limit_val;
};

struct hl1512_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	u8 chip_id;
	int gpio_int;
	int irq_int;
	unsigned int current_limit_val;
};

#endif /* _HL1512_H_ */
