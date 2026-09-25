/*
 * bd99954_charger.h
 *
 * bd99954 driver
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

#ifndef _BD99954_CHARGER_H_
#define _BD99954_CHARGER_H_

#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/workqueue.h>

#define BD99954_REG_TOTAL_NUM                 0x80

/* command select */
#define BD99954_REG_PROTECT                   0x3E
#define BD99954_REG_MAP_SET_CHARGE            0x3F

/* charge status */
#define BD99954_REG_CHGSTM_STATUS             0x00
#define BD99954_CURR_STATE_MASK               (BIT(0) | BIT(1) | \
	BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define BD99954_CHG_DONE_VALUE                5
#define BD99954_FAST_CHARGE_MODE              3
#define BD99954_NOT_CHARGE                    0
#define BD99954_REG_VBAT_VSYS_STATUS          0x01
#define BD99954_VBAT_OVP_MASK                 BIT(3)
#define BD99954_REG_VBUS_VCC_STATUS           0x02
#define BD99954_VBUS_OVP_MASK                 BIT(3)
#define BD99954_VBUS_CLPS_MASK                BIT(2)
#define BD99954_VBUS_ONLINE_MASK              BIT(0)
#define BD99954_REG_CHGOP_STATUS              0x03
#define BD99954_BATT_TEMP_MASK                (BIT(8) | BIT(9) | BIT(10))
#define BD99954_REG_WDT_STATUS                0x04

/* actual input current limit */
#define BD99954_REG_CUR_IBUS_LIM_SET          0x05
#define BD99954_CUR_IBUS_LIM_SET_STEP         1

/* input curr limit */
#define BD99954_REG_IBUS_LIM_SET              0x07
#define BD99954_IBUS_LIMIT_SHIFT              5
#define BD99954_IBUS_LIMIT_STEP               32
#define BD99954_IBUS_LIMIT_MAX                16352
#define BD99954_IBUS_LIMIT_MIN                512
#define BD99954_IBUS_LIMIT_DEFAULT            500

/* otg set */
#define BD99954_REG_IOTG_LIM_SET              0x09
#define BD99954_IOTG_STEP                     32
#define BD99954_IOTG_SHIFT                    5
#define BD99954_IOTG_MAX                      16352
#define BD99954_IOTG_MIN                      0
#define BD99954_REG_VIN_CTRL_SET              0x0A
#define BD99954_VCC_INPUT_EN_MASK             BIT(5)
#define BD99954_VCC_INPUT_EN_SHIFT            5
#define BD99954_DISABLE_VCC_INPUT             0x0

/* charge ctrl set */
#define BD99954_REG_CHGOP_SET1                0x0B
#define BD99954_DIS_AUTO_LIMIIN_MASK          BIT(13)
#define BD99954_DIS_AUTO_LIMIIN_SHIFT         13
#define BD99954_EN_AUTO_LIMIIN                0
#define BD99954_REG_AUTO_TOP_MASK             BIT(6)
#define BD99954_REG_AUTO_TOP_SHIFT            6
#define BD99954_DISABLE_AUTO_TOP              0
#define BD99954_REG_CHGOP_SET2                0x0C
#define BD99954_ILIMIT_RESET_STEP_MASK        (BIT(12) | BIT(13) | BIT(14))
#define BD99954_ILIMIT_RESET_STEP_SHIFT       12
#define BD99954_ILIMIT_RESET_STEP_VALUE       0x1
#define BD99954_REG_CHG_EN_MASK               BIT(7)
#define BD99954_REG_CHG_EN_SHIFT              7
#define BD99954_REG_USB_SUS_MASK              BIT(6)
#define BD99954_REG_USB_SUS_SHIFT             6
#define BD99954_REG_CHOP_ALL_MASK             BIT(0)
#define BD99954_REG_CHOP_ALL_SHIFT            0

/* vbus limit set */
#define BD99954_REG_VBUS_SET                  0x0D
#define BD9995_VBUS_SHIFT                     7
#define BD9995_VBUS_MIN                       3840
#define BD9995_VBUS_MAX                       25088
#define BD9995_VBUS_STEP                      128

/* charge watchdog set */
#define BD99954_REG_CHGWDT_SET                0x0F

/* system voltage limit */
#define BD99954_REG_VSYSREG_SET               0x11
#define BD99954_VSYSREG_SHIFT                 6
#define BD99954_VSYSREG_MAX                   19200
#define BD99954_VSYSREG_MIN                   2560
#define BD99954_VSYSREG_STEP                  64

/* trickle-charging current set */
#define BD99954_REG_ITRICH_SET                0x14
#define BD99954_REG_ITRICH_SET_SHIFT          6

/* pre-charging current set */
#define BD99954_REG_IPRECH_SET                0x15
#define BD99954_IPRECH_STEP                   64
#define BD99954_IPRECH_SHIFT                  6
#define BD99954_IPRECH_MAX                    1024
#define BD99954_IPRECH_MIN                    0

/* charge current set */
#define BD99954_REG_ICHG_SET                  0x16
#define BD99954_ICHG_SHIFT                    6
#define BD99954_ICHG_STEP                     64
#define BD99954_ICHG_MAX                      16320
#define BD99954_ICHG_MIN                      0

/* termination current set */
#define BD99954_REG_ITERM_SET                 0x17
#define BD99954_ITERM_SHIFT                   6
#define BD99954_ITERM_STEP                    64
#define BD99954_ITERM_MAX                     1024
#define BD99954_ITERM_MIN                     0

/* otg voltage set */
#define BD99954_REG_VRBOOST_SET               0x19
#define BD99954_VRBOOST_SET_STEP              64
#define BD99954_VRBOOST_SET_MAX               22016
#define BD99954_VRBOOST_SET_MIN               4032
#define BD99954_VRBOOST_SET_SHIFT             6

/* charge voltage set */
#define BD99954_REG_VFASTCHG_REG_SET1         0x1A
#define BD99954_VFASTCHG_SHIFT                4
#define BD99954_VCHARGE_MAX                   19200
#define BD99954_VCHARGE_MIN                   2560
#define BD99954_VCHARGE_STEP                  16

/* recharge voltage */
#define BD99954_REG_VRECHG_SET                0x1D
#define BD99954_VRECHG_SHITF                  4
#define BD99954_VRECHG_STEP                   16
#define BD99954_VRECHG_DEFAULT                100

#define BD99954_REG_VBATOVP_SET               0x1E
#define BD99954_REG_IBATSHORT_SET             0x1F
#define BD99954_REG_0X20                      0x20
#define BD99954_REG_0X21                      0x21
#define BD99954_REG_0X22                      0x22
#define BD99954_REG_0X23                      0x23
#define BD99954_REG_0X24                      0x24
#define BD99954_REG_0X25                      0x25
#define BD99954_REG_0X28                      0x28
#define BD99954_REG_0X30                      0x30
#define BD99954_REG_VALUE_0X00                0

/* reset register */
#define BD99954_REG_SYSTEM_CTRL               0x3D
#define BD99954_RESET_ALL_REG_MASK            BIT(0)
#define BD99954_RESET_ALL_REG_SHIFT           0

/* adc value */
#define BD99954_REG_IBATP_VAL                 0x50
#define BD99954_REG_IBATM_VAL                 0x52
#define BD99954_REG_VBAT_VAL                  0x54
#define BD99954_REG_VBAT_AVE_VAL              0x55
#define BD99954_REG_VBAT_VAL_STEP             1
#define BD99954_REG_THERM_VAL                 0x56
#define BD99954_THERM_VAL_OFFSET              (-55)
#define BD99954_THERM_VAL_STEP                1
#define BD99954_REG_IBUS_VAL                  0x58
#define BD99954_REG_IBUS_AVE_VAL              0x59
#define BD99954_IBUS_VAL_STEP                 1
#define BD99954_REG_VACP_VAL                  0x5A
#define BD99954_REG_VBUS_VAL                  0x5C
#define BD99954_REG_VBUS_AVE_VAL              0x5D
#define BD99954_VBUS_VAL_STEP                 1
#define BD99954_REG_VSYS_VAL                  0x60

/* interrupt setting */
#define BD99954_REG_INT1_SET                  0x69
#define BD99954_REG_INT3_SET                  0x6B
#define BD99954_REG_INT4_SET                  0x6C

/* interrupt status */
#define BD99954_REG_INT1_STATUS               0x71
#define BD99954_REG_VBUS_OV_DET_MASK          BIT(5)
#define BD99954_REG_VBUS_OV_DET_SHIFT         5
#define BD99954_REG_INT3_STATUS               0x73
#define BD99954_REG_VBAT_OV_DET_MASK          BIT(5)
#define BD99954_REG_VBAT_OV_DET_SHIFT         5

#define BD99954_REG_ILIM_DECREASE_SET         0x7A
#define BD99954_ILIM_DECREASE_VALUE           200

#define BD99954_REG_SMBREG                    0x7C
#define BD99954_REG_SMBREG_0H                 0x0
#define BD99954_REG_SMBREG_5H                 0x5

#endif /* _BD99954_CHARGER_H_ */
