/*
 * rt9759.h
 *
 * rt9759 driver
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

#ifndef _RT9759_H_
#define _RT9759_H_

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

struct rt9759_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data nty_data;
	int gpio_int;
	int irq_int;
	int irq_active;
	int chip_already_init;
	int device_id;
	int tsbus_high_r_kohm;
	int tsbus_low_r_kohm;
	int switching_frequency;
	int special_freq_shift;
	u32 ic_role;
	int sense_r_actual;
	int sense_r_config;
	u32 chrg_ctl_reg_init;
	u32 adc_fn_reg_init;
	int get_id_time;
	int init_finish_flag;
	int int_notify_enable_flag;
	u32 adc_accuracy;
	int adc_v_ref;
	int v_pullup;
	int r_pullup;
	int r_comp;
	int ibus_low_deglitch;
	int ucp_rise_threshold;
	int close_vbat_protection;
};

#define RT9759_INIT_FINISH                1
#define RT9759_NOT_INIT                   0
#define RT9759_ENABLE_INT_NOTIFY          1
#define RT9759_DISABLE_INT_NOTIFY         0

#define RT9759_NOT_USED                   0
#define RT9759_USED                       1
#define RT9759_DEVICE_ID_GET_FAIL         (-1)
#define RT9759_REG_RESET                  1
#define RT9759_SWITCHCAP_DISABLE          0

#define RT9759_LOW_BYTE_INIT              0xFF
#define RT9759_LENTH_OF_BYTE              8
#define RT9759_TDIE_SCALE                 2

#define RT9759_ADC_DISABLE                0
#define RT9759_ADC_ENABLE                 1

#define RT9759_ALM_DISABLE                0
#define RT9759_ALM_ENABLE                 1

#define RT9759_AC_OVP_THRESHOLD_INIT      12000 /* 12v */
#define RT9759_VBUS_OVP_THRESHOLD_INIT    12000 /* 12v */
#define RT9759_IBUS_OCP_THRESHOLD_INIT    4750 /* 4.75a */
#define RT9759_VBAT_OVP_THRESHOLD_INIT    5000 /* 5v */
#define RT9759_IBAT_OCP_THRESHOLD_INIT    10000 /* 10a */

#define RT9759_RESISTORS_10KOHM           10 /* for Rhi and Rlo */
#define RT9759_RESISTORS_100KOHM          100 /* for Rhi and Rlo */
#define RT9759_RESISTORS_KILO             1000 /* kilo */

/*
 * BAT_OVP reg=0x00
 * default=4.35v [b10 0010], BAT_OVP=3.5v + bat_ovp[5:0]*25mv
 */
#define RT9759_BAT_OVP_REG                0x00
#define RT9759_BAT_OVP_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5))
#define RT9759_BAT_OVP_SHIFT              0
#define RT9759_BAT_OVP_MAX_5075MV         5075 /* 5.075v */
#define RT9759_BAT_OVP_BASE_3500MV        3500 /* 3.5v */
#define RT9759_BAT_OVP_DEFAULT            4350 /* 4.35v */
#define RT9759_BAT_OVP_STEP               25 /* 25mv */

#define RT9759_BAT_OVP_DIS_MASK           BIT(7)
#define RT9759_BAT_OVP_DIS_SHIFT          7

/*
 * BAT_OVP_ALM reg=0x01
 * default=4.20v [b01 1100], BAT_OVP_ALM=3.5v + bat_ovp_alm[5:0]*25mv
 */
#define RT9759_BAT_OVP_ALM_REG            0x01
#define RT9759_BAT_OVP_ALM_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5))
#define RT9759_BAT_OVP_ALM_SHIFT          0
#define RT9759_BAT_OVP_ALM_MAX            5075 /* 5.075v */
#define RT9759_BAT_OVP_ALM_BASE           3500 /* 3.5v */
#define RT9759_BAT_OVP_ALM_DEFAULT        4200 /* 4.2v */
#define RT9759_BAT_OVP_ALM_STEP           25 /* 25mv */

#define RT9759_BAT_OVP_ALM_DIS_MASK       BIT(7)
#define RT9759_BAT_OVP_ALM_DIS_SHIFT      7

/*
 * BAT_OCP reg=0x02
 * default=8.1a [b011 1101], BAT_OCP=2a + bat_ocp[6:0]*100ma
 */
#define RT9759_BAT_OCP_REG                0x02
#define RT9759_BAT_OCP_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define RT9759_BAT_OCP_SHIFT              0
#define RT9759_BAT_OCP_MAX_10000MA        10000 /* 10.0a */
#define RT9759_BAT_OCP_BASE_2000MA        2000 /* 2.0a */
#define RT9759_BAT_OCP_DEFAULT            8100 /* 8.1a */
#define RT9759_BAT_OCP_STEP               100 /* 100ma */

#define RT9759_BAT_OCP_DIS_MASK           BIT(7)
#define RT9759_BAT_OCP_DIS_SHIFT          7

/*
 * BAT_OCP_ALM reg=0x03
 * default=8.0a [b011 1100], BAT_OCP_ALM=2a + bat_ocp_alm[6:0]*100ma
 */
#define RT9759_BAT_OCP_ALM_REG            0x03
#define RT9759_BAT_OCP_ALM_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define RT9759_BAT_OCP_ALM_SHIFT          0
#define RT9759_BAT_OCP_ALM_MAX            10000 /* 10.0a */
#define RT9759_BAT_OCP_ALM_BASE           2000 /* 2.0a */
#define RT9759_BAT_OCP_ALM_DEFAULT        8000 /* 8.0a */
#define RT9759_BAT_OCP_ALM_STEP           100 /* 100ma */

#define RT9759_BAT_OCP_ALM_DIS_MASK       BIT(7)
#define RT9759_BAT_OCP_ALM_DIS_SHIFT      7

/*
 * BAT_UCP_ALM reg=0x04
 * default=2.0a [b010 1000], BAT_UCP_ALM=bat_ucp_alm[6:0]*50ma
 */
#define RT9759_BAT_UCP_ALM_REG            0x04
#define RT9759_BAT_UCP_ALM_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define RT9759_BAT_UCP_ALM_SHIFT          0
#define RT9759_BAT_UCP_ALM_MAX            6350 /* 6.35a */
#define RT9759_BAT_UCP_ALM_BASE           0 /* 0a */
#define RT9759_BAT_UCP_ALM_DEFAULT        2000 /* 2a */
#define RT9759_BAT_UCP_ALM_STEP           50 /* 50ma */

#define RT9759_BAT_UCP_ALM_DIS_MASK       BIT(7)
#define RT9759_BAT_UCP_ALM_DIS_SHIFT      7

/*
 * AC_PROTECTION reg=0x05
 * default=18.0v [b111], AC_OVP=11v + ac_ovp[2:0]*1v
 */
#define RT9759_AC_OVP_REG                 0x05
#define RT9759_AC_OVP_MASK                (BIT(0) | BIT(1) | BIT(2))
#define RT9759_AC_OVP_SHIFT               0
#define RT9759_AC_OVP_MAX_18000MV         18000 /* 18v */
#define RT9759_AC_OVP_BASE_11000MV        11000 /* 11v */
#define RT9759_AC_OVP_DEFAULT             18000 /* 18v */
#define RT9759_AC_OVP_STEP                1000 /* 1v */

#define RT9759_AC_OVP_STAT_MASK           BIT(7)
#define RT9759_AC_OVP_STAT_SHIFT          7
#define RT9759_AC_OVP_FLAG_MASK           BIT(6)
#define RT9759_AC_OVP_FLAG_SHIFT          6
#define RT9759_AC_OVP_MASK_MASK           BIT(5)
#define RT9759_AC_OVP_MASK_SHIFT          5

/*
 * BUS_OVP reg=0x06
 * default=8.9v [b011 1010], BUS_OVP=6v + bus_ovp[6:0]*50mv
 */
#define RT9759_BUS_OVP_REG                0x06
#define RT9759_VBUS_PD_EN_MASK            BIT(7)
#define RT9759_VBUS_PD_EN_SHIFT           7
#define RT9759_BUS_OVP_MASK               (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define RT9759_BUS_OVP_SHIFT              0

#define RT9759_VBUS_PD_EN_DISABLE         0

#define RT9759_BUS_OVP_MAX_12350MV        12350 /* 12.35v */
#define RT9759_BUS_OVP_BASE_6000MV        6000 /* 6v */
#define RT9759_BUS_OVP_DEFAULT            8900 /* 8.9v */
#define RT9759_BUS_OVP_STEP               50 /* 50mv */

/*
 * BUS_OVP_ALM reg=0x07
 * default=8.8v [b011 1000], BUS_OVP_ALM=6v + bus_ovp_alm[6:0]*50mv
 */
#define RT9759_BUS_OVP_ALM_REG            0x07
#define RT9759_BUS_OVP_ALM_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define RT9759_BUS_OVP_ALM_SHIFT          0
#define RT9759_BUS_OVP_ALM_MAX            12350 /* 12.35v */
#define RT9759_BUS_OVP_ALM_BASE           6000 /* 6v */
#define RT9759_BUS_OVP_ALM_DEFAULT        8800 /* 8.8v */
#define RT9759_BUS_OVP_ALM_STEP           50 /* 50mv */

#define RT9759_BUS_OVP_ALM_DIS_MASK       BIT(7)
#define RT9759_BUS_OVP_ALM_DIS_SHIFT      7

/*
 * BUS_OCP_UCP reg=0x08
 * default=4.25a [b1101], BUS_OCP_UCP=1a + bus_ocp[3:0]*250ma
 */
#define RT9759_BUS_OCP_UCP_REG            0x08
#define RT9759_BUS_OCP_MASK               (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define RT9759_BUS_OCP_SHIFT              0
#define RT9759_BUS_OCP_MAX_4750MA         4750 /* 4.75a */
#define RT9759_BUS_OCP_BASE_1000MA        1000 /* 1a */
#define RT9759_BUS_OCP_DEFAULT            4250 /* 4.25a */
#define RT9759_BUS_OCP_STEP               250 /* 250ma */

#define RT9759_BUS_OCP_DIS_MASK           BIT(7)
#define RT9759_BUS_OCP_DIS_SHIFT          7
#define RT9759_IBUS_UCP_RISE_FLAG_MASK    BIT(6)
#define RT9759_IBUS_UCP_RISE_FLAG_SHIFT   6
#define RT9759_IBUS_UCP_RISE_MASK_MASK    BIT(5)
#define RT9759_IBUS_UCP_RISE_MASK_SHIFT   5
#define RT9759_IBUS_UCP_FALL_FLAG_MASK    BIT(4)
#define RT9759_IBUS_UCP_FALL_FLAG_SHIFT   4

/*
 * BUS_OCP_ALM reg=0x09
 * default=4.0a [b101 0000], BUS_OCP_ALM=bus_ocp_alm[6:0]*50ma
 */
#define RT9759_BUS_OCP_ALM_REG            0x09
#define RT9759_BUS_OCP_ALM_MASK           (BIT(0) | BIT(1) | BIT(2) | \
	BIT(3) | BIT(4) | BIT(5) | BIT(6))
#define RT9759_BUS_OCP_ALM_SHIFT          0
#define RT9759_BUS_OCP_ALM_MAX            6350 /* 6.35a */
#define RT9759_BUS_OCP_ALM_BASE           0 /* 0a */
#define RT9759_BUS_OCP_ALM_DEFAULT        4000 /* 4a */
#define RT9759_BUS_OCP_ALM_STEP           50 /* 50ma */

#define RT9759_BUS_OCP_ALM_DIS_MASK       BIT(7)
#define RT9759_BUS_OCP_ALM_DIS_SHIFT      7

/* CONVERTER_STATE reg=0x0a */
#define RT9759_CONVERTER_STATE_REG        0x0A

#define RT9759_TSHUT_FLAG_MASK            BIT(7)
#define RT9759_TSHUT_FLAG_SHIFT           7
#define RT9759_TSHUT_STAT_MASK            BIT(6)
#define RT9759_TSHUT_STAT_SHIFT           6
#define RT9759_VBUS_ERRORLO_STAT_MASK     BIT(5)
#define RT9759_VBUS_ERRORLO_STAT_SHIFT    5
#define RT9759_VBUS_ERRORHI_STAT_MASK     BIT(4)
#define RT9759_VBUS_ERRORHI_STAT_SHIFT    4
#define RT9759_SS_TIMEOUT_FLAG_MASK       BIT(3)
#define RT9759_SS_TIMEOUT_FLAG_SHIFT      3
#define RT9759_WD_TIMEOUT_FLAG_MASK       BIT(2)
#define RT9759_WD_TIMEOUT_FLAG_SHIFT      2
#define RT9759_CONV_OCP_FLAG_MASK         BIT(1)
#define RT9759_CONV_OCP_FLAG_SHIFT        1
#define RT9759_PIN_DIAG_FAIL_FLAG_MASK    BIT(0)
#define RT9759_PIN_DIAG_FAIL_FLAG_SHIFT   0

/* CONTROL reg=0x0b */
#define RT9759_CONTROL_REG                0x0B
/* 0x46=disable watchdog, switching freq 550khz */
#define RT9759_CONTROL_REG_INIT           0x46

#define RT9759_REG_RST_MASK               BIT(7)
#define RT9759_REG_RST_SHIFT              7
#define RT9759_FSW_SET_MASK               (BIT(5) | BIT(6))
#define RT9759_FSW_SET_SHIFT              5
#define RT9759_WATCHDOG_DIS_MASK          BIT(2)
#define RT9759_WATCHDOG_DIS_SHIFT         2
#define RT9759_WATCHDOG_CONFIG_MASK       (BIT(0) | BIT(1))
#define RT9759_WATCHDOG_CONFIG_SHIFT      0

#define RT9759_REG_RST_ENABLE             1

#define RT9759_SW_FREQ_450KHZ             450
#define RT9759_SW_FREQ_500KHZ             500
#define RT9759_SW_FREQ_550KHZ             550
#define RT9759_SW_FREQ_675KHZ             675
#define RT9759_SW_FREQ_750KHZ             750
#define RT9759_SW_FREQ_825KHZ             825

#define RT9759_FSW_SET_SW_FREQ_250KHZ     0x0
#define RT9759_FSW_SET_SW_FREQ_375KHZ     0x1
#define RT9759_FSW_SET_SW_FREQ_500KHZ     0x2
#define RT9759_FSW_SET_SW_FREQ_750KHZ     0x3

#define RT9759_WTD_CONFIG_TIMING_500MS    500
#define RT9759_WTD_CONFIG_TIMING_1000MS   1000
#define RT9759_WTD_CONFIG_TIMING_5000MS   5000
#define RT9759_WTD_CONFIG_TIMING_30000MS  30000

#define RT9759_WTD_SET_500MS              0
#define RT9759_WTD_SET_1000MS             1
#define RT9759_WTD_SET_5000MS             2
#define RT9759_WTD_SET_30000MS            3

/* CHRG_CTRL reg=0x0c */
#define RT9759_CHRG_CTL_REG               0x0C
#define RT9759_CHRG_CTL_REG_INIT          0x0e

#define RT9759_CHARGE_EN_MASK             BIT(7)
#define RT9759_CHARGE_EN_SHIFT            7
#define RT9759_MS_MASK                    (BIT(5) | BIT(6))
#define RT9759_MS_SHIFT                   5
#define RT9759_FREQ_SHIFT_MASK            (BIT(3) | BIT(4))
#define RT9759_FREQ_SHIFT_SHIFT           3
#define RT9759_TSBUS_DIS_MASK             BIT(2)
#define RT9759_TSBUS_DIS_SHIFT            2
#define RT9759_TSBAT_DIS_MASK             BIT(1)
#define RT9759_TSBAT_DIS_SHIFT            1
#define RT9759_TDIE_DIS_MASK              BIT(0)
#define RT9759_TDIE_DIS_SHIFT             0

#define RT9759_SW_FREQ_SHIFT_NORMAL       0
#define RT9759_SW_FREQ_SHIFT_P_P10        1 /* +10% */
#define RT9759_SW_FREQ_SHIFT_M_P10        2 /* -10% */
#define RT9759_SW_FREQ_SHIFT_MP_P10       3 /* +/-10% */

/* INT_STAT reg=0x0d */
#define RT9759_INT_STAT_REG               0x0D

#define RT9759_BAT_OVP_ALM_STAT_MASK      BIT(7)
#define RT9759_BAT_OVP_ALM_STAT_SHIFT     7
#define RT9759_BAT_OCP_ALM_STAT_MASK      BIT(6)
#define RT9759_BAT_OCP_ALM_STAT_SHIFT     6
#define RT9759_BUS_OVP_ALM_STAT_MASK      BIT(5)
#define RT9759_BUS_OVP_ALM_STAT_SHIFT     5
#define RT9759_BUS_OCP_ALM_STAT_MASK      BIT(4)
#define RT9759_BUS_OCP_ALM_STAT_SHIFT     4
#define RT9759_BAT_UCP_ALM_STAT_MASK      BIT(3)
#define RT9759_BAT_UCP_ALM_STAT_SHIFT     3
#define RT9759_ADAPTER_INSERT_STAT_MASK   BIT(2)
#define RT9759_ADAPTER_INSERT_STAT_SHIFT  2
#define RT9759_VBAT_INSERT_STAT_MASK      BIT(1)
#define RT9759_VBAT_INSERT_STAT_SHIFT     1
#define RT9759_ADC_DONE_STAT_MASK         BIT(0)
#define RT9759_ADC_DONE_STAT_SHIFT        0

/* INT_FLAG reg=0x0e */
#define RT9759_INT_FLAG_REG               0x0E

#define RT9759_BAT_OVP_ALM_FLAG_MASK      BIT(7)
#define RT9759_BAT_OVP_ALM_FLAG_SHIFT     7
#define RT9759_BAT_OCP_ALM_FLAG_MASK      BIT(6)
#define RT9759_BAT_OCP_ALM_FLAG_SHIFT     6
#define RT9759_BUS_OVP_ALM_FLAG_MASK      BIT(5)
#define RT9759_BUS_OVP_ALM_FLAG_SHIFT     5
#define RT9759_BUS_OCP_ALM_FLAG_MASK      BIT(4)
#define RT9759_BUS_OCP_ALM_FLAG_SHIFT     4
#define RT9759_BAT_UCP_ALM_FLAG_MASK      BIT(3)
#define RT9759_BAT_UCP_ALM_FLAG_SHIFT     3
#define RT9759_ADAPTER_INSERT_FLAG_MASK   BIT(2)
#define RT9759_ADAPTER_INSERT_FLAG_SHIFT  2
#define RT9759_VBAT_INSERT_FLAG_MASK      BIT(1)
#define RT9759_VBAT_INSERT_FLAG_SHIFT     1
#define RT9759_ADC_DONE_FLAG_MASK         BIT(0)
#define RT9759_ADC_DONE_FLAG_SHIFT        0

/* INT_MASK reg=0x0f */
#define RT9759_INT_MASK_REG               0x0F
#define RT9759_INT_MASK_REG_INIT          0xF8

#define RT9759_BAT_OVP_ALM_MASK_MASK      BIT(7)
#define RT9759_BAT_OVP_ALM_MASK_SHIFT     7
#define RT9759_BAT_OCP_ALM_MASK_MASK      BIT(6)
#define RT9759_BAT_OCP_ALM_MASK_SHIFT     6
#define RT9759_BUS_OVP_ALM_MASK_MASK      BIT(5)
#define RT9759_BUS_OVP_ALM_MASK_SHIFT     5
#define RT9759_BUS_OCP_ALM_MASK_MASK      BIT(4)
#define RT9759_BUS_OCP_ALM_MASK_SHIFT     4
#define RT9759_BAT_UCP_ALM_MASK_MASK      BIT(3)
#define RT9759_BAT_UCP_ALM_MASK_SHIFT     3
#define RT9759_ADAPTER_INSERT_MASK_MASK   BIT(2)
#define RT9759_ADAPTER_INSERT_MASK_SHIFT  2
#define RT9759_VBAT_INSERT_MASK_MASK      BIT(1)
#define RT9759_VBAT_INSERT_MASK_SHIFT     1
#define RT9759_ADC_DONE_MASK_MASK         BIT(0)
#define RT9759_ADC_DONE_MASK_SHIFT        0

/* FLT_STAT reg=0x10 */
#define RT9759_FLT_STAT_REG               0x10

#define RT9759_BAT_OVP_FLT_STAT_MASK      BIT(7)
#define RT9759_BAT_OVP_FLT_STAT_SHIFT     7
#define RT9759_BAT_OCP_FLT_STAT_MASK      BIT(6)
#define RT9759_BAT_OCP_FLT_STAT_SHIFT     6
#define RT9759_BUS_OVP_FLT_STAT_MASK      BIT(5)
#define RT9759_BUS_OVP_FLT_STAT_SHIFT     5
#define RT9759_BUS_OCP_FLT_STAT_MASK      BIT(4)
#define RT9759_BUS_OCP_FLT_STAT_SHIFT     4
#define RT9759_TSBUS_TSBAT_ALM_STAT_MASK  BIT(3)
#define RT9759_TSBUS_TSBAT_ALM_STAT_SHIFT 3
#define RT9759_TSBAT_FLT_STAT_MASK        BIT(2)
#define RT9759_TSBAT_FLT_STAT_SHIFT       2
#define RT9759_TSBUS_FLT_STAT_MASK        BIT(1)
#define RT9759_TSBUS_FLT_STAT_SHIFT       1
#define RT9759_TDIE_ALM_STAT_MASK         BIT(0)
#define RT9759_TDIE_ALM_STAT_SHIFT        0

/* FLT_FLAG reg=0x11 */
#define RT9759_FLT_FLAG_REG               0x11

#define RT9759_BAT_OVP_FLT_FLAG_MASK      BIT(7)
#define RT9759_BAT_OVP_FLT_FLAG_SHIFT     7
#define RT9759_BAT_OCP_FLT_FLAG_MASK      BIT(6)
#define RT9759_BAT_OCP_FLT_FLAG_SHIFT     6
#define RT9759_BUS_OVP_FLT_FLAG_MASK      BIT(5)
#define RT9759_BUS_OVP_FLT_FLAG_SHIFT     5
#define RT9759_BUS_OCP_FLT_FLAG_MASK      BIT(4)
#define RT9759_BUS_OCP_FLT_FLAG_SHIFT     4
#define RT9759_TSBUS_TSBAT_ALM_FLAG_MASK  BIT(3)
#define RT9759_TSBUS_TSBAT_ALM_FLAG_SHIFT 3
#define RT9759_TSBAT_FLT_FLAG_MASK        BIT(2)
#define RT9759_TSBAT_FLT_FLAG_SHIFT       2
#define RT9759_TSBUS_FLT_FLAG_MASK        BIT(1)
#define RT9759_TSBUS_FLT_FLAG_SHIFT       1
#define RT9759_TDIE_ALM_FLAG_MASK         BIT(0)
#define RT9759_TDIE_ALM_FLAG_SHIFT        0

/* FLT_MASK reg=0x12 */
#define RT9759_FLT_MASK_REG               0x12
#define RT9759_FLT_MASK_REG_INIT          0x0F

#define RT9759_BAT_OVP_FLT_MASK_MASK      BIT(7)
#define RT9759_BAT_OVP_FLT_MASK_SHIFT     7
#define RT9759_BAT_OCP_FLT_MASK_MASK      BIT(6)
#define RT9759_BAT_OCP_FLT_MASK_SHIFT     6
#define RT9759_BUS_OVP_FLT_MASK_MASK      BIT(5)
#define RT9759_BUS_OVP_FLT_MASK_SHIFT     5
#define RT9759_BUS_OCP_FLT_MASK_MASK      BIT(4)
#define RT9759_BUS_OCP_FLT_MASK_SHIFT     4
#define RT9759_TSBUS_TSBAT_ALM_MASK_MASK  BIT(3)
#define RT9759_TSBUS_TSBAT_ALM_MASK_SHIFT 3
#define RT9759_TSBAT_FLT_MASK_MASK        BIT(2)
#define RT9759_TSBAT_FLT_MASK_SHIFT       2
#define RT9759_TSBUS_FLT_MASK_MASK        BIT(1)
#define RT9759_TSBUS_FLT_MASK_SHIFT       1
#define RT9759_TDIE_ALM_MASK_MASK         BIT(0)
#define RT9759_TDIE_ALM_MASK_SHIFT        0

/* PART_INFO reg=0x13 */
#define RT9759_PART_INFO_REG              0x13

#define RT9759_DEVICE_REV_MASK            (BIT(4) | BIT(5) | BIT(6) | BIT(7))
#define RT9759_DEVICE_REV_SHIFT           4
#define RT9759_DEVICE_ID_MASK             (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define RT9759_DEVICE_ID_SHIFT            0

#define RT9759_DEVICE_ID_RT9759           8

/* ADC_CTRL reg=0x14 */
#define RT9759_ADC_CTRL_REG               0x14
#define RT9759_ADC_CTRL_REG_INIT          0xA0

#define RT9759_ADC_CTRL_EN_MASK           BIT(7)
#define RT9759_ADC_CTRL_EN_SHIFT          7
#define RT9759_ADC_RATE_MASK              BIT(6)
#define RT9759_ADC_RATE_SHIFT             6
#define RT9759_IBUS_ADC_DIS_MASK          BIT(0)
#define RT9759_IBUS_ADC_DIS_SHIFT         0

/* ADC_FN_DISABLE reg=0x15 */
#define RT9759_ADC_FN_DIS_REG             0x15
#define RT9759_ADC_FN_DIS_REG_INIT        0x02

#define RT9759_VBUS_ADC_DIS_MASK          BIT(7)
#define RT9759_VBUS_ADC_DIS_SHIFT         7
#define RT9759_VAC_ADC_DIS_MASK           BIT(6)
#define RT9759_VAC_ADC_DIS_SHIFT          6
#define RT9759_VOUT_ADC_DIS_MASK          BIT(5)
#define RT9759_VOUT_ADC_DIS_SHIFT         5
#define RT9759_VBAT_ADC_DIS_MASK          BIT(4)
#define RT9759_VBAT_ADC_DIS_SHIFT         4
#define RT9759_IBAT_ADC_DIS_MASK          BIT(3)
#define RT9759_IBAT_ADC_DIS_SHIFT         3
#define RT9759_TSBUS_ADC_DIS_MASK         BIT(2)
#define RT9759_TSBUS_ADC_DIS_SHIFT        2
#define RT9759_TSBAT_ADC_DIS_MASK         BIT(1)
#define RT9759_TSBAT_ADC_DIS_SHIFT        1
#define RT9759_TDIE_ADC_DIS_MASK          BIT(0)
#define RT9759_TDIE_ADC_DIS_SHIFT         0

/* IBUS_ADC1 reg=0x16 IBUS=ibus_adc[8:0]*1ma */
/* IBUS_ADC0 reg=0x17 */
#define RT9759_IBUS_ADC1_REG              0x16
#define RT9759_IBUS_ADC0_REG              0x17

#define RT9759_IBUS_ADC_STEP              1 /* 1ma */

/* VBUS_ADC1 reg=0x18 VBUS=vbus_adc[8:0]*1mv */
/* VBUS_ADC0 reg=0x19 */
#define RT9759_VBUS_ADC1_REG              0x18
#define RT9759_VBUS_ADC0_REG              0x19

#define RT9759_VBUS_ADC_STEP              1 /* 1mv */

/* VAC_ADC1 reg=0x1A VAC=vac_adc[8:0]*1mv */
/* VAC_ADC0 reg=0x1B */
#define RT9759_VAC_ADC1_REG               0x1A
#define RT9759_VAC_ADC0_REG               0x1B

#define RT9759_VAC_ADC_STEP               1 /* 1mv */

/* VOUT_ADC1 reg=0x1C VOUT=vout_adc[8:0]*1mv */
/* VOUT_ADC0 reg=0x1D */
#define RT9759_VOUT_ADC1_REG              0x1C
#define RT9759_VOUT_ADC0_REG              0x1D

#define RT9759_VOUT_ADC_STEP              1 /* 1mv */

/* VBAT_ADC1 reg=0x1E VBAT=vbat_adc[8:0]*1mv */
/* VBAT_ADC0 reg=0x1F */
#define RT9759_VBAT_ADC1_REG              0x1E
#define RT9759_VBAT_ADC0_REG              0x1F

#define RT9759_VBAT_ADC_STEP              1 /* 1mv */

/* IBAT_ADC1 reg=0x20 IBAT=ibat_adc[8:0]*1ma */
/* IBAT_ADC0 reg=0x21 */
#define RT9759_IBAT_ADC1_REG              0x20
#define RT9759_IBAT_ADC0_REG              0x21

#define RT9759_IBAT_ADC_STEP              1 /* 1ma */

/* TSBUS_ADC1 reg=0x22 TSBUS=tsbus_adc[8:0]*0.09766% */
/* TSBUS_ADC0 reg=0x23 */
#define RT9759_TSBUS_ADC1_REG             0x22
#define RT9759_TSBUS_ADC1_MASK            (BIT(0) | BIT(1))

#define RT9759_TSBUS_ADC0_REG             0x23

#define RT9759_TSBUS_ADC_STEP             9766 /* 0.09766% */
#define RT9759_TSBUS_PER_MAX              10000000

/* TSBAT_ADC1 reg=0x24 TSBAT=tsbat_adc[8:0]*0.09766% */
/* TSBAT_ADC0 reg=0x25 */
#define RT9759_TSBAT_ADC1_REG             0x24
#define RT9759_TSBAT_ADC1_MASK            (BIT(0) | BIT(1) | BIT(7))

#define RT9759_TSBAT_ADC0_REG             0x25

#define RT9759_TSBAT_ADC_STEP             9766 /* 0.09766% */

/* TDIE_ADC1 reg=0x26 TDIE=tdie_adc[8:0]*0.5c */
/* TDIE_ADC0 reg=0x27 */
#define RT9759_TDIE_ADC1_REG              0x26
#define RT9759_TDIE_ADC1_MASK             (BIT(0) | BIT(7))

#define RT9759_TDIE_ADC0_REG              0x27

#define RT9759_TDIE_ADC_STEP              5 /* 0.5c */

/*
 * TSBUS_FLT1 reg=0x28
 * default=4.1% [b0001 0101], TSBUS_FLT=tsbus_flt[7:0] * 0.19531%
 */
#define RT9759_TSBUS_FLT_REG              0x28

#define RT9759_TSBUS_FLT_MAX              4980405  /* 49.80405% */
#define RT9759_TSBUS_FLT_BASE             0
#define RT9759_TSBUS_FLT_DEFAULT          410151   /* 4.10151% */
#define RT9759_TSBUS_FLT_STEP             19531    /* 0.19531% */

/*
 * TSBAT_FLT0 reg=0x29
 * default=4.1% [b0001 0101],  TSBUS_FLT=tsbus_flt[7:0] * 0.19531%
 */
#define RT9759_TSBAT_FLT_REG              0x29

#define RT9759_TSBAT_FLT_MAX              4980405 /* 49.80405% */
#define RT9759_TSBAT_FLT_BASE             0
#define RT9759_TSBAT_FLT_DEFAULT          410151 /* 4.10151% */
#define RT9759_TSBAT_FLT_STEP             19531 /* 0.19531% */

/*
 * TDIE_ALM reg=0x2a
 * default=125c [b1100 1000], TDIE_ALM=25+tdie_alm[7:0]*0.5c
 */
#define RT9759_TDIE_ALM_REG               0x2A

#define RT9759_TDIE_ALM_MAX               1525 /* 152.5c */
#define RT9759_TDIE_ALM_BASE              250 /* 25c */
#define RT9759_TDIE_ALM_DEFAULT           1250 /* 12c */
#define RT9759_TDIE_ALM_STEP              5 /* 0.5c */

/* REG_CTRL reg=0x2b */
#define RT9759_REG_CTRL                   0x2B
#define RT9759_UCP_RISE_MASK              BIT(2)
#define RT9759_UCP_RISE_SHIFT             2
#define RT9759_UCP_RISE_500MA             1
#define RT9759_UCP_RISE_300MA             0
#define RT9759_IBAT_RSEN_MASK             BIT(1)
#define RT9759_IBAT_RSEN_SHIFT            1
#define RT9759_IBAT_RSEN_2MOHM            0
#define RT9759_IBAT_RSEN_5MOHM            1

/* REG_STAT_FLAG reg=0x2e */
#define RT9759_STAT_FLAG_REG              0x2E
#define RT9759_VBUS_LOW_DEGLITCH_MASK     BIT(4)
#define RT9759_VBUS_LOW_DEGLITCH_SHIFT    4
#define RT9759_VBUS_LOW_DEGLITCH_10US     0
#define RT9759_VBUS_LOW_DEGLITCH_10MS     1
#define RT9759_IBUS_LOW_DEGLITCH_MASK     BIT(3)
#define RT9759_IBUS_LOW_DEGLITCH_SHIFT    3
#define RT9759_IBUS_LOW_DEGLITCH_10US     0
#define RT9759_IBUS_LOW_DEGLITCH_5MS      1

/* DEGLITCH reg=0x2f */
#define RT9759_DEGLITCH_REG               0x2F

#endif /* _RT9759_H_ */
