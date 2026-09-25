/*
 * hl1512.h
 *
 * charge-pump hl1512 macro, addr etc. (bp: bypass mode, cp: charge-pump mode)
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#define HL1512_ADDR_LEN                1
#define HL1512_REG_BIT_AI              BIT(7)

#define HL1512_CHARGING_FWD            1
#define HL1512_CHARGING_REV            2

/* Device ID reg */
#define HL1512_DEVICE_ID_REG           0x10

/* init A7 register */
#define HL1512_INIT_REGA7              0xA7
#define HL1512_INIT_REGA7_VAL          0xF9
/* init 58 register */
#define HL1512_INIT_REG58              0x58
#define HL1512_INIT_REG58_VAL          0x82
/* mode register */
#define HL1512_MOD_REG                 0x00
#define HL1512_MOD_INIT_REG_VAL        0x80
#define HL1512_MOD_REG_VAL             0x80
#define HL1512_MOD_REV_REG_VAL         0x20
/* control register */
#define HL1512_CONTROL_REG             0x01
#define HL1512_CONTROL_REG_VAL         0x06
/* pmid vout ov/uv register */
#define HL1512_PMID_VOUT_OV_UV_REG     0x02
#define HL1512_PMID_VOUT_FWD_VAL       0xFC
#define HL1512_PMID_VOUT_REV_VAL       0xFD
#define HL1512_PMID_VOUT_CP_VAL        0xFE
/* vout ov register */
#define HL1512_VOUT_OV_REG             0x03
#define HL1512_VOUT_OV_REG_FWD_VAL     0xFF
#define HL1512_VOUT_OV_REG_REV_VAL     0xFC
/* watch dog & clock */
#define HL1512_WD_CLOCK_REG            0x04
#define HL1512_WD_CLOCK_VAL            0x17
/* current limit register */
#define HL1512_CURRENT_LIMIT_REG       0x0D
#define HL1512_CURRENT_LIMIT_FWD_VAL   0x9D
#define HL1512_CURRENT_LIMIT_REV_VAL   0x99
/* temp alarm th register */
#define HL1512_TEMP_ALARM_TH_REG       0x0F
#define HL1512_TEMP_ALARM_TH_REG_VAL   0xC0

/* Force Bypass / Force CP */
#define HL1512_FORCE_BPCP_MASK         (BIT(7) | BIT(6) | BIT(1) | BIT(0))
#define HL1512_FORCE_BPCP_SHIFT        0
#define HL1512_FORCE_BP_EN             0x80
#define HL1512_FORCE_CP_EN             0x81

/* mode status */
#define HL1512_STATUS_REG              0x06
#define HL1512_STATUS_REG_BP_VAL       0x02
#define HL1512_STATUS_REG_CP_VAL       0x03
#define HL1512_STATUS_00_MASK          BIT(0)
#define HL1512_STATUS_01_MASK          BIT(1)
#define HL1512_STATUS_02_MASK          BIT(2)
#define HL1512_STATUS_00_SHIFT         1
#define HL1512_STATUS_01_SHIFT         2
#define HL1512_STATUS_02_SHIFT         3
#define HL1512_STATUS_MASK             0x07

struct hl1512_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	u8 chip_id;
	int gpio_int;
	int irq_int;
};

#endif /* _HL1512_H_ */
