/*
 * tps63810.h
 *
 * tps63810 header
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

#ifndef _TPS63810_H_
#define _TPS63810_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/i2c.h>

#define TPS_I2C_RETRY_CNT            3

/* register control */
#define TPS63810_CONTROL_ADDR        0x01
#define TPS63810_CONTROL_MASK        (BIT(5) | BIT(6))
#define TPS63810_CONTROL_SHIFT       5
#define TPS63810_LOW_RANGE           0
#define TPS63810_HIGH_RANGE          1
#define TPS63810_CONTROL_ENABLE      1

/* register status */
#define TPS63810_STATUS_ADDR         0x02
#define TPS63810_STATUS_HD_MASK      BIT(4)
#define TPS63810_STATUS_HD_SHIFT     4
#define TPS63810_STATUS_UV_MASK      BIT(3)
#define TPS63810_STATUS_UV_SHIFT     3
#define TPS63810_STATUS_OC_MASK      BIT(2)
#define TPS63810_STATUS_OC_SHIFT     2
#define TPS63810_STATUS_TSD_MASK     BIT(1)
#define TPS63810_STATUS_TSD_SHIFT    1
#define TPS63810_STATUS_PG_MASK      BIT(0)
#define TPS63810_STATUS_PG_SHIFT     0

/* register devid */
#define TPS63810_DEVID_ADDR          0x03
#define TPS63810_DEVID_MFR_MASK      (BIT(4) | BIT(5) | BIT(6) | BIT(7))
#define TPS63810_DEVID_MFR_SHIFT     4
#define TPS63810_DEVID_MAJOR_MASK    (BIT(2) | BIT(3))
#define TPS63810_DEVID_MAJOR_SHIFT   2
#define TPS63810_DEVID_MINOR_MASK    (BIT(0) | BIT(1))
#define TPS63810_DEVID_MINOR_SHIFT   0
#define TPS63810_DEVID_TI            0x04
#define TPS63810_DEVID_RT            0xa8

/* register vout1 */
#define TPS63810_VOUT1_ADDR          0x04
#define TPS63810_VOUT1_LR_DEFAULT    3300
#define TPS63810_VOUT1_LR_MIN        1800
#define TPS63810_VOUT1_LR_MAX        4975
#define TPS63810_VOUT1_HR_DEFAULT    3525
#define TPS63810_VOUT1_HR_MIN        2025
#define TPS63810_VOUT1_HR_MAX        5200
#define TPS63810_VOUT1_STEP          25

#define RT6160_VOUT1_DEFAULT         3300
#define RT6160_VOUT1_MIN             2025
#define RT6160_VOUT1_MAX             5200
#define RT6160_VOUT1_STEP            25

/* register vout2 */
#define TPS63810_VOUT2_ADDR          0x05
#define TPS63810_VOUT2_LR_DEFAULT    3450
#define TPS63810_VOUT2_LR_MIN        1800
#define TPS63810_VOUT2_LR_MAX        4975
#define TPS63810_VOUT2_HR_DEFAULT    3675
#define TPS63810_VOUT2_HR_MIN        2025
#define TPS63810_VOUT2_HR_MAX        5200
#define TPS63810_VOUT2_STEP          25

#define RT6160_VOUT2_DEFAULT         3450
#define RT6160_VOUT2_MIN             2025
#define RT6160_VOUT2_MAX             5200
#define RT6160_VOUT2_STEP            25

struct tps63810_device_info {
	struct i2c_client *client;
	struct device *dev;
	u32 vsel_state;
	u32 vsel_range;
	u8 dev_id;
	int gpio_no;
};

#endif /* _TPS63810_H_ */
