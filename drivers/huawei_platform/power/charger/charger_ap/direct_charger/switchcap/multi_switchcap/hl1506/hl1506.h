/*
 * hl1506.h
 *
 * hl1506 driver
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

#ifndef _HL1506_H_
#define _HL1506_H_

#define HL1506_INVALID_ID            -1
#define HL1506_MAX_REGS              4
#define HL1506_DEV_MAX_NUM           5
#define HL1506_CHIP_DISABLE          0
#define HL1506_CHIP_ENABLE           1
#define HL1506_5V_DISABLE            0
#define HL1506_5V_ENABLE             1

#define HL1506_ADDR_LEN              1
#define I2C_WR_MSG_LEN               1
#define I2C_RD_MSG_LEN               2
#define BYTE_LEN                     1

/* Reg0x00: PMID Voltage Limit ADC */
#define HL1506_REG_00                0x00
#define HL1506_00_INIT_VAL           0xff
/* Reg0x01: iin Limit / Force Bypass / Force CP */
#define HL1506_REG_01                0x01
#define HL1506_01_INIT_VAL           0xd1
#define HL1506_01_IIN_LIMI_MASK      0xf8 /* bit[3:7] */
#define HL1506_01_IIN_LIMI_SHIFT     3
#define HL1506_01_FORCE_BP_MASK      BIT(1)
#define HL1506_01_FORCE_BP_SHIFT     1
#define HL1506_01_FORCE_BP_EN        1
#define HL1506_01_FORCE_BP_DIS       0
#define HL1506_01_FORCE_CP_MASK      BIT(0)
#define HL1506_01_FORCE_CP_SHIFT     0
#define HL1506_01_FORCE_CP_EN        1
#define HL1506_01_FORCE_CP_DIS       0
/* Reg0x02: CP-CK/ Disable LDO5 */
#define HL1506_REG_02                0x02
#define HL1506_02_INIT_VAL           0x47
/* Reg0x03: host enable */
#define HL1506_REG_03                0x03
#define HL1506_03_INIT_VAL           0x80
#define HL1506_03_DISABLE_CHIP       0X02
/* Reg0x04: status */
#define HL1506_REG_04                0x04
#define HL1506_04_BPCP_MODE_MASK     BIT(7)
#define HL1506_04_BPCP_MODE_SHIFT    7
/* Reg0x061: i2c init */
#define HL1506_I2C_INIT_REG          0x61
#define HL1506_I2C_INIT_RST          0x00
#define HL1506_I2C_INIT_CLR          0x80

struct hl1506_device_info {
	struct i2c_client *client;
	struct device *dev;
	int gpio_en;
	int gpio_wchg_5v_en;
	u32 dev_id;
};

#endif /* _HL1506_H_ */
