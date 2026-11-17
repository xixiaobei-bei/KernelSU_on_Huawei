/*
 * aw32280.h
 *
 * aw32280 header file
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _AW32280_H_
#define _AW32280_H_

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
#include <linux/raid/pq.h>
#include <linux/bitops.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>

struct aw32280_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data nty_data;
	int gpio_int;
	int gpio_enable;
	int gpio_reset;
	int irq_int;
	int chip_already_init;
	int device_id;
	u32 ic_role;
	int get_id_time;
	int init_finish_flag;
	int int_notify_enable_flag;
	int sense_r_actual;
	int sense_r_config;
	u32 parallel_mode;
	u32 slave_mode;
	u8 charge_mode;
};

#define AW32280_NOT_USED                   0
#define AW32280_USED                       1
#define AW32280_NOT_INIT                   0
#define AW32280_INIT_FINISH                1
#define AW32280_CHIP_ID_REG_NUM            6
#define AW32280_HOST_MODE                  0
#define AW32280_STANDALONE_MODE            0
#define AW32280_ENABLE_INT_NOTIFY          1
#define AW32280_DISABLE_INT_NOTIFY         0
#define AW32280_BUF_LEN                    80
#define AW32280_DEVICE_ID_GET_FAIL         (-1)

#define AW32280_CHIP_ID_0_REG              0x00
#define AW32280_CHIP_ID_1_REG              0x01
#define AW32280_CHIP_ID_2_REG              0x02
#define AW32280_CHIP_ID_3_REG              0x03
#define AW32280_CHIP_ID_4_REG              0x04
#define AW32280_CHIP_ID_5_REG              0x05
#define AW32280_CHIP_ID_0                  0x36
#define AW32280_CHIP_ID_1                  0x35
#define AW32280_CHIP_ID_2                  0x32
#define AW32280_CHIP_ID_3                  0x36
#define AW32280_CHIP_ID_4                  0x00
#define AW32280_CHIP_ID_5                  0x60

#define AW32280_SC_OVP_EN_REG              0x12
#define AW32280_SC_OVP_EN_SHIFT            0
#define AW32280_SC_OVP_EN_MASK             BIT(0)

#define AW32280_SC_OVP_MODE_REG            0x13
#define AW32280_SC_OVP_MODE_SHIFT          0
#define AW32280_SC_OVP_MODE_MASK           BIT(0)

#define AW32280_SC_PSW_EN_REG              0x14
#define AW32280_SC_PSW_EN_SHIFT            0
#define AW32280_SC_PSW_EN_MASK             BIT(0)

#define AW32280_SC_PSW_MODE_REG            0x15
#define AW32280_SC_PSW_MODE_SHIFT          0
#define AW32280_SC_PSW_MODE_MASK           BIT(0)

#define AW32280_SC_SC_EN_REG               0x16
#define AW32280_SC_SC_EN_SHIFT             0
#define AW32280_SC_SC_EN_MASK              BIT(0)
#define AW32280_SC_SC_EN                   1
#define AW32280_SC_SC_DISABLE              0

#define AW32280_SC_SC_MODE_REG             0x18
#define AW32280_SC_SC_MODE_SHIFT           0
#define AW32280_SC_SC_MODE_MASK            (BIT(2) | BIT(1) | BIT(0))
#define AW32280_CHG_FBYPASS_MODE           0
#define AW32280_CHG_F21SC_MODE             1
#define AW32280_CHG_F41SC_MODE             2

#define AW32280_SC_WDT_EN_REG              0x20
#define AW32280_SC_WDT_EN_SHIFT            0
#define AW32280_SC_WDT_EN_MASK             BIT(0)

#define AW32280_WDT_SOFT_RESET_REG         0x21
#define AW32280_WD_RST_N_SHIFT             0
#define AW32280_WD_RST_N_MASK              BIT(0)

#define AW32280_WDT_CONFIG_REG             0x22
#define AW32280_WDT_CONFIG_SHIFT           0
#define AW32280_WDT_CONFIG_MASK            (BIT(1) | BIT(0))
#define AW32280_WDT_CONFIG_5S              0x3
#define AW32280_WDT_CONFIG_2S              0x2
#define AW32280_WDT_CONFIG_1S              0x1
#define AW32280_WDT_CONFIG_500MS           0x0
#define AW32280_WTD_CONFIG_TIMING_5000MS   5000
#define AW32280_WTD_CONFIG_TIMING_2000MS   2000
#define AW32280_WTD_CONFIG_TIMING_1000MS   1000
#define AW32280_WTD_CONFIG_TIMING_500MS    500

#define AW32280_DIG_STATUS0_REG            0x23
#define AW32280_WDT_TIME_OUT_SHIFT         0
#define AW32280_WDT_TIME_OUT_MASK          BIT(0)

#define AW32280_HKADC_EN_REG               0x100
#define AW32280_SC_HKADC_EN_SHIFT          0
#define AW32280_SC_HKADC_EN_MASK           BIT(0)
#define AW32280_SC_HKADC_EN                1

#define AW32280_HKADC_START_REG            0x101
#define AW32280_HKADC_START_SHIFT          0
#define AW32280_HKADC_START_MASK           BIT(0)

#define AW32280_HKADC_CTRL1_REG            0x102
#define AW32280_SC_HKADC_AVG_TIMES_SHIFT   0
#define AW32280_SC_HKADC_AVG_TIMES_MASK    (BIT(1) | BIT(0))
#define AW32280_HKADC_CTRL1_INIT           0x53

#define AW32280_HKADC_SEQ_CH_H_REG         0x103
#define AW32280_HKADC_SEQ_CH_H_INIT        0x3F

#define AW32280_HKADC_SEQ_CH_L_REG         0x104
#define AW32280_HKADC_SEQ_CH_L_INIT        0xFF

#define AW32280_HKADC_DATA_VALID_REG       0x106
#define AW32280_HKADC_DATA_VALID_SHIFT     0
#define AW32280_HKADC_DATA_VALID_MASK      BIT(0)

#define AW32280_VUSB_ADC_L_REG             0x107
#define AW32280_VUSB_ADC_H_REG             0x108

#define AW32280_VPSW_ADC_L_REG             0x109
#define AW32280_VPSW_ADC_H_REG             0x10A

#define AW32280_VBUS_ADC_L_REG             0x10B
#define AW32280_VBUS_ADC_H_REG             0x10C

#define AW32280_VOUT1_ADC_L_REG            0x10D
#define AW32280_VOUT1_ADC_H_REG            0x10E

#define AW32280_VOUT2_ADC_L_REG            0x10F
#define AW32280_VOUT2_ADC_H_REG            0x110

#define AW32280_VBAT1_ADC_L_REG            0x111
#define AW32280_VBAT1_ADC_H_REG            0x112

#define AW32280_VBAT2_ADC_L_REG            0x113
#define AW32280_VBAT2_ADC_H_REG            0x114

#define AW32280_IBAT1_ADC_L_REG            0x115
#define AW32280_IBAT1_ADC_H_REG            0x116

#define AW32280_IBAT2_ADC_L_REG            0x117
#define AW32280_IBAT2_ADC_H_REG            0x118

#define AW32280_IBUS_REF_ADC_L_REG         0x119
#define AW32280_IBUS_REF_ADC_H_REG         0x11A

#define AW32280_IBUS_ADC_L_REG             0x11B
#define AW32280_IBUS_ADC_H_REG             0x11C

#define AW32280_TDIE_ADC_L_REG             0x11D
#define AW32280_TDIE_ADC_H_REG             0x11E

#define AW32280_TBAT1_ADC_L_REG            0x11F
#define AW32280_TBAT1_ADC_H_REG            0x120

#define AW32280_TBAT2_ADC_L_REG            0x121
#define AW32280_TBAT2_ADC_H_REG            0x122

#define AW32280_OTP_EN_REG                 0x127
#define AW32280_OTP_EN_SHIFT               0
#define AW32280_OTP_EN_MASK                BIT(0)

#define AW32280_IBAT1_RES_PLACE_REG        0x136
#define AW32280_IBAT2_RES_PLACE_REG        0x137
#define AW32280_IBAT_RES_PLACE_MASK        BIT(0)
#define AW32280_IBAT_RES_PLACE_SHIFT       0
#define AW32280_IBAT_RES_HIGHSIDE          0
#define AW32280_IBAT_RES_LOWSIDE           1

#define AW32280_IRQ_FLAG_REG               0x200

#define AW32280_IRQ_FLAG_2_REG             0x203
#define AW32280_VBUS_OVP_FLAG_MASK         BIT(2)
#define AW32280_VOUT_OVP_FLAG_MASK         BIT(1)

#define AW32280_IRQ_FLAG_3_REG             0x204
#define AW32280_IBAT_OVP_FLAG_MASK         (BIT(5) | BIT(4))

#define AW32280_IRQ_FLAG_4_REG             0x205
#define AW32280_IBUS_OVP_FLAG_MASK         BIT(7)
#define AW32280_VBAT_OVP_FLAG_MASK         (BIT(2) | BIT(1))

#define AW83220_IRQ_FLAG_5_REG             0x206

#define AW32280_USB_OVP_CFG_0_REG          0x302
#define AW32280_USB_OVP_CFG_0_REG_INIT     0x3F

#define AW32280_SC_PRO_TOP_CFG_0_REG       0x304
#define AW32280_SC_PRO_TOP_CFG_0_REG_INIT  0xA4

#define AW22803_SC_PRO_TOP_CFG_2_REG       0x306
#define AW22803_SC_PRO_TOP_CFG_2_REG_INIT  0x00

#define AW32280_SC_PRO_TOP_CFG_3_REG       0x307
#define AW32280_SC_PRO_TOP_CFG_3_REG_INIT  0x0F

#define AW32280_SC_DET_TOP_CFG_0_REG       0x308
#define AW32280_SC_DET_TOP_CFG_0_REG_INIT  0xFF

#define AW32280_SC_TOP_CFG_4_REG           0x316
#define AW32280_SC_HOST_SEL_MASK           BIT(0)
#define AW32280_SC_HOST_SEL_SHIFT          0

#define AW32280_SC_TOP_CFG_6_REG           0x318
#define AW32280_SC_PARALLEL_SEL_MASK       BIT(5)
#define AW32280_SC_PARALLEL_SEL_SHIFT      5

#define AW32280_SC_TOP_CFG_7_REG           0x319
#define AW32280_SC_TOP_CFG_7_REG_INIT      0x7F

#define AW32280_SC_TOP_CFG_8_REG           0x31A
#define AW32280_SC_TOP_CFG_8_REG_INIT      0xFF

#define AW32280_USB_OVP_CFG_2_REG          0x31F
#define AW32280_USB_OVP_MASK               (BIT(7) | BIT(6) | BIT(5))
#define AW32280_USB_OVP_SHIFT              5
#define AW32280_USB_OVP_F41SC_VAL          6
#define AW32280_USB_OVP_F21SC_VAL          2
#define AW32280_USB_OVP_FBPSC_VAL          0

#define AW32280_PSW_OVP_CFG_2_REG          0x321
#define AW32280_PSW_OVP_MASK               (BIT(7) | BIT(6) | BIT(5))
#define AW32280_PSW_OVP_SHIFT              5
#define AW32280_PSW_OVP_F41SC_VAL          6
#define AW32280_PSW_OVP_F21SC_VAL          2
#define AW32280_PSW_OVP_FBPSC_VAL          0

#define AW32280_SC_PRO_TOP_CFG_4_REG       0x322
#define AW32280_FWD_IBAT1_OVP_MASK         (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define AW32280_FWD_IBAT1_OVP_SHIFT        0

#define AW32280_SC_PRO_TOP_CFG_5_REG       0x323
#define AW32280_FWD_IBAT2_OVP_MASK         (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define AW32280_FWD_IBAT2_OVP_SHIFT        4
#define AW32280_FWD_IBAT_OVP_MIN           5000
#define AW32280_FWD_IBAT_OVP_MAX           15000
#define AW32280_FWD_IBAT_OVP_STEP          500
#define AW32280_FWD_IBAT_OVP_INIT          13000

#define AW32280_SC_PRO_TOP_CFG_6_REG       0x324
#define AW32280_IBAT_RES_SEL_SHIFT         2
#define AW32280_IBAT_RES_SEL_MASK          BIT(2)
#define AW32280_IBAT_SNS_RES_1MOHM         0
#define AW32280_IBAT_SNS_RES_2MOHM         1

#define AW32280_SC_PRO_TOP_CFG_7_REG       0x325
#define AW32280_PRO_TOP_CFG_7_REG_INIT     0x64

#define AW32280_SC_PRO_TOP_CFG_8_REG       0x326
#define AW32280_PRO_TOP_CFG_8_REG_INIT     0xC0
#define AW32280_IBUS_OVP_F21SC_MIN         6000
#define AW32280_IBUS_OVP_F41SC_MIN         3000

#define AW32280_SC_PRO_TOP_CFG_12_REG      0x32A
#define AW32280_SC_PRO_TOP_CFG_12_REG_INIT 0x36

#define AW32280_SC_DET_TOP_CFG_2_REG       0x32B
#define AW32280_DET_TOP_CFG_2_REG_INIT     0x31

#define AW32280_REF_TOP_CFG_0_REG          0x300

#define AW32280_BLK_TOP_CFG_0_REG          0x330
#define AW32280_VBUS_PULLDOWN_MARK         BIT(0)
#define AW32280_VBUS_PULLDOWN_SHIFT        0

#define AW32280_VBUS_OVP_F41SC_REG         0x334
#define AW32280_VBUS_OVP_F21SC_REG         0x335
#define AW32280_VBUS_OVP_FBPSC_REG         0x336
#define AW32280_VBUS_OVP_MARK              BIT(0)
#define AW32280_VBUS_OVP_SHIFT             0
#define AW32280_VBUS_OVP_F41SC_MIN         22000
#define AW32280_VBUS_OVP_F21SC_MIN         12000
#define AW32280_VBUS_OVP_FBPSC_MIN         5600
#define AW32280_VBUS_OVP_HIGH_VAL          1
#define AW32280_VBUS_OVP_LOW_VAL           0

#define AW32280_SC_VOUT_OVP_REG            0x337
#define AW32280_SC_VOUT_OVP_REG_INIT       0x02

#define AW32280_FWD_VBAT1_OVP_SEL_REG      0x338
#define AW32280_FWD_VBAT2_OVP_SEL_REG      0x339
#define AW32280_FWD_VBAT_OVP_MASK          (BIT(2) | BIT(1) | BIT(0))
#define AW32280_FWD_VBAT_OVP_SHIFT         0
#define AW32280_FWD_VBAT_OVP_MIN           4500
#define AW32280_FWD_VBAT_OVP_MAX           5200
#define AW32280_FWD_VBAT_OVP_STEP          100
#define AW80322_FWD_VBAT_OVP_INIT          4800

#endif /* _AW32280_H_ */
