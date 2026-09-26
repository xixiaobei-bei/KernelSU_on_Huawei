/*
 * sm5450.h
 *
 * sm5450 header file
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
#ifndef _SM5450_H_
#define _SM5450_H_

#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>

struct sm5450_param {
	int scp_support;
	int fcp_support;
};

struct sm5450_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data nty_data;
	struct sm5450_param param_dts;
	int gpio_int;
	int irq_int;
	int irq_active;
	int chip_already_init;
	int device_id;
	int rev_id;
	int dc_ibus_ucp_happened;
	u32 ic_role;
	int get_id_time;
	int get_rev_time;
	int init_finish_flag;
	int int_notify_enable_flag;
	int switching_frequency;
	int sense_r_actual;
	int sense_r_config;
	struct mutex scp_detect_lock;
	struct mutex accp_adapter_reg_lock;
	unsigned int fcp_support;
	unsigned int adapter_support;
	unsigned int scp_isr_backup[2]; /* datasheet:scp_isr1,scp_isr2 */
};

#define SM5450_INIT_FINISH                             1
#define SM5450_NOT_INIT                                0
#define SM5450_ENABLE_INT_NOTIFY                       1
#define SM5450_DISABLE_INT_NOTIFY                      0

#define SM5450_NOT_USED                                0
#define SM5450_USED                                    1
#define SM5450_DEVICE_ID_GET_FAIL                      (-1)
#define SM5450_REG_RESET                               1
#define SM5450_SWITCHCAP_DISABLE                       0

#define SM5450_LENGTH_OF_BYTE                          8

#define SM5450_ADC_RESOLUTION_2                        2
#define SM5450_ADC_RESOLUTION_4                        4
#define SM5450_ADC_STANDARD_TDIE                       40

#define SM5450_ADC_DISABLE                             0
#define SM5450_ADC_ENABLE                              1

#define SM5450_VBAT_OVP_REF_INIT                       4350 /* 4.35v */
#define SM5450_IBAT_OCP_REF_INIT                       7200 /* 7.2a */
#define SM5450_VBUS_OVP_REF_RESET                      6000 /* 6v */
#define SM5450_VBUS_OVP_REF_INIT                       11500 /* 11.5v */
#define SM5450_IBUS_OCP_REF_INIT                       3000 /* 3a */
#define SM5450_VBUSCON_OVP_REF_RESET                   6500 /* 6v */
#define SM5450_VBUSCON_OVP_REF_INIT                    12000 /* 12v */

/* CONTROL1 reg=0x00 */
#define SM5450_CONTROL1_REG                            0x00
#define SM5450_CONTROL1_RESET_MASK                     BIT(7)
#define SM5450_CONTROL1_RESET_SHIFT                    7
#define SM5450_CONTROL1_OPMODE_MASK                    (BIT(6) | BIT(5) |  BIT(4))
#define SM5450_CONTROL1_OPMODE_SHIFT                   4
#define SM5450_CONTROL1_NEN_WATCHDOG_MASK              BIT(3)
#define SM5450_CONTROL1_NEN_WATCHDOG_SHIFT             3
#define SM5450_CONTROL1_WATCHDOG_REF_MASK              (BIT(2) | BIT(1) |  BIT(0))
#define SM5450_CONTROL1_WATCHDOG_REF_SHIFT             0

#define SM5450_CONTROL1_RST_ENABLE                     1
#define SM5450_CONTROL1_OFFMODE                        0
#define SM5450_CONTROL1_FBYPASSMODE                    1
#define SM5450_CONTROL1_FCHGPUMPMODE                   2

#define SM5450_WTD_CONFIG_TIMING_500MS                 500
#define SM5450_WTD_CONFIG_TIMING_1000MS                1000
#define SM5450_WTD_CONFIG_TIMING_2000MS                2000
#define SM5450_WTD_CONFIG_TIMING_5000MS                5000
#define SM5450_WTD_CONFIG_TIMING_10000MS               10000
#define SM5450_WTD_CONFIG_TIMING_20000MS               20000
#define SM5450_WTD_CONFIG_TIMING_40000MS               40000
#define SM5450_WTD_CONFIG_TIMING_80000MS               80000

/* CONTROL2 reg=0x01 */
#define SM5450_CONTROL2_REG                            0x01
#define SM5450_CONTROL2_FREQ_MASK                      (BIT(7) | BIT(6) | BIT(5))
#define SM5450_CONTROL2_FREQ_SHIFT                     5
#define SM5450_CONTROL2_ADJUST_FREQ_MASK               (BIT(4) | BIT(3))
#define SM5450_CONTROL2_ADJUST_FREQ_SHIFT              3

/* CONTROL3 reg=0x02 */
#define SM5450_CONTROL3_REG                            0x02
#define SM5450_CONTROL3_EN_FCAP_SCP_MASK               BIT(7)
#define SM5450_CONTROL3_EN_FCAP_SCP_SHIFT              7
#define SM5450_CONTROL3_EN_COMP_MASK                   BIT(6)
#define SM5450_CONTROL3_EN_COMP_SHIFT                  6
#define SM5450_CONTROL3_EN_RLTVUVP_MASK                BIT(5)
#define SM5450_CONTROL3_EN_RLTVUVP_SHIFT               5
#define SM5450_CONTROL3_EN_RLTVOVP_MASK                BIT(4)
#define SM5450_CONTROL3_EN_RLTVOVP_SHIFT               4
#define SM5450_CONTROL3_RLTVUVP_REF_MASK               (BIT(3) | BIT(2))
#define SM5450_CONTROL3_RLTVUVP_REF_SHIFT              2
#define SM5450_CONTROL3_RLTVOVP_REF_MASK               (BIT(1) | BIT(0))
#define SM5450_CONTROL3_RLTVOVP_REF_SHIFT              1

#define SM5450_RLTVUVP_REF_1P01                        0x0
#define SM5450_RLTVUVP_REF_1P02                        0x1
#define SM5450_RLTVUVP_REF_1P03                        0x2
#define SM5450_RLTVUVP_REF_1P04                        0x3

#define SM5450_RLTVOVP_REF_1P10                        0x0
#define SM5450_RLTVOVP_REF_1P15                        0x1
#define SM5450_RLTVOVP_REF_1P20                        0x2
#define SM5450_RLTVOVP_REF_1P25                        0x3

#define SM5450_SW_FREQ_450KHZ                          450
#define SM5450_SW_FREQ_500KHZ                          500
#define SM5450_SW_FREQ_550KHZ                          550
#define SM5450_SW_FREQ_675KHZ                          675
#define SM5450_SW_FREQ_750KHZ                          750
#define SM5450_SW_FREQ_825KHZ                          825
#define SM5450_SW_FREQ_1000KHZ                         1000
#define SM5450_SW_FREQ_1250KHZ                         1250
#define SM5450_SW_FREQ_1500KHZ                         1500

#define SM5450_FSW_SET_SW_FREQ_200KHZ                  0x0
#define SM5450_FSW_SET_SW_FREQ_375KHZ                  0x1
#define SM5450_FSW_SET_SW_FREQ_500KHZ                  0x2
#define SM5450_FSW_SET_SW_FREQ_750KHZ                  0x3
#define SM5450_FSW_SET_SW_FREQ_1000KHZ                 0x4
#define SM5450_FSW_SET_SW_FREQ_1250KHZ                 0x5
#define SM5450_FSW_SET_SW_FREQ_1500KHZ                 0x6

#define SM5450_SW_FREQ_SHIFT_NORMAL                    0 /* norminal frequency */
#define SM5450_SW_FREQ_SHIFT_P_P10                     1 /* +%10 */
#define SM5450_SW_FREQ_SHIFT_M_P10                     2 /* -%10 */

/* DEVICE_INFO reg=0x03 */
#define SM5450_DEVICE_INFO_REG                         0x03
#define SM5450_DEVICE_INFO_REVID_MASK                  (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define SM5450_DEVICE_INFO_REVID_SHIFT                 4
#define SM5450_DEVICE_INFO_DEVID_MASK                  (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_DEVICE_INFO_DEVID_SHIFT                 0
#define SM5450_DEVICE_ID_SM5450                        1
#define SY69636_DEVICE_ID_SY69636                      2

/* VBUSCON_OVP reg=0x04 */
#define SM5450_VBUSCON_OVP_REG                         0x04
#define SM5450_VBUSCON_OVP_EN_VBUSCON_OVP_MASK         BIT(4)
#define SM5450_VBUSCON_OVP_EN_VBUSCON_OVP_SHIFT        4
#define SM5450_VBUSCON_OVP_VBUSCON_OVP_REF_MASK        (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_VBUSCON_OVP_VBUSCON_OVP_REF_SHIFT       0

#define SM5450_VBUSCON_OVP_MAX_19000MV                 19000 /* 19v */
#define SM5450_VBUSCON_OVP_BASE_4000MV                 4000 /* 4v */
#define SM5450_VBUSCON_OVP_DEFAULT                     12000 /* 12v */
#define SM5450_VBUSCON_OVP_STEP                        1000 /* 1v */

/* PULLDOWN reg=0x05 */
#define SM5450_PULLDOWN_REG                            0x05
#define SM5450_PULLDOWN_EN_VBUSCON_PD_MASK             BIT(7)
#define SM5450_PULLDOWN_EN_VBUSCON_PD_SHIFT            7
#define SM5450_PULLDOWN_EN_VBUS_PD_MASK                BIT(6)
#define SM5450_PULLDOWN_EN_VBUS_PD_SHIFT               6

/* VBUSCON_OVP reg=0x06 */
#define SM5450_VBUSOVP_REG                             0x06
#define SM5450_VBUSOVP_EN_VBUSOVP_MASK                 BIT(7)
#define SM5450_VBUSOVP_EN_VBUSOVP_SHIFT                7
#define SM5450_VBUSOVP_VBUSOVP_REF_MASK                (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_VBUSOVP_VBUSOVP_REF_SHIFT               0

#define SM5450_VBUS_OVP_MAX_14000MV                    14000 /* 14v */
#define SM5450_VBUS_OVP_BASE_4000MV                    4000 /* 4v */
#define SM5450_VBUS_OVP_DEFAULT                        11500 /* 11.5v */
#define SM5450_VBUS_OVP_STEP                           100 /* 100mv */

/* IBUS_OCP_UCP reg=0x07 */
#define SM5450_IBUS_OCP_UCP_REG                        0x07
#define SM5450_IBUS_OCP_UCP_EN_IBUSUCP_MASK            BIT(7)
#define SM5450_IBUS_OCP_UCP_EN_IBUSUCP_SHIFT           7
#define SM5450_IBUS_OCP_UCP_IBUSUCP_REF_MASK           BIT(6)
#define SM5450_IBUS_OCP_UCP_IBUSUCP_REF_SHIFT          6
#define SM5450_IBUS_OCP_UCP_EN_IBUSOCP_MASK            BIT(5)
#define SM5450_IBUS_OCP_UCP_EN_IBUSOCP_SHIFT           5
#define SM5450_IBUS_OCP_UCP_IBUSOCP_REF_MASK           (BIT(4) | BIT(3) | BIT(2) | \
	BIT(1) | BIT(0))
#define SM5450_IBUS_OCP_UCP_IBUSOCP_REF_SHIFT          0

#define SM5450_BUS_UCP_BASE_300MA_150MA                300
#define SM5450_BUS_UCP_BASE_500MA_250MA                500

#define SM5450_IBUS_OCP_CP_BASE_500MA                  500
#define SM5450_IBUS_OCP_CP_MAX_3600MA                  3600

#define SM5450_IBUS_OCP_BP_BASE_2500MA                 2500
#define SM5450_IBUS_OCP_BP_MAX_5600MA                  5600

#define SM5450_IBUS_OCP_STEP                           100

/* VBATOVP reg=0x08 */
#define SM5450_VBATOVP_REG                             0x08
#define SM5450_VBATOVP_EN_VBATOVP_MASK                 BIT(7)
#define SM5450_VBATOVP_EN_VBATOVP_SHIFT                7
#define SM5450_VBATOVP_VBATOVP_REF_MASK                (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SM5450_VBATOVP_VBATOVP_REF_SHIFT               0

#define SM5450_BAT_OVP_BASE_4000MV                     4000
#define SM5450_BAT_OVP_BASE_5575MV                     5575
#define SM5450_BAT_OVP_STEP                            25

/* IBATOCP reg=0x09 */
#define SM5450_IBATOCP_REG                             0x09
#define SM5450_IBATOCP_EN_IBATOCP_MASK                 BIT(7)
#define SM5450_IBATOCP_EN_IBATOCP_SHIFT                7
#define SM5450_IBATOCP_IBATOCP_REF_MASK                (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SM5450_IBATOCP_IBATOCP_REF_SHIFT               0

#define SM5450_BAT_OCP_BASE_2000MA                     2000
#define SM5450_BAT_OCP_BASE_8300MA                     8300
#define SM5450_BAT_OCP_STEP                            100

/* REGULATION reg=0x0A */
#define SM5450_REGULATION_REG                          0x0A
#define SM5450_REGULATION_EN_IBATREG_MASK              BIT(5)
#define SM5450_REGULATION_EN_IBATREG_SHIFT             5
#define SM5450_REGULATION_IBATREG_REF_MASK             (BIT(4) | BIT(3))
#define SM5450_REGULATION_IBATREG_REF_SHIFT            3
#define SM5450_REGULATION_EN_VBATREG_MASK              BIT(2)
#define SM5450_REGULATION_EN_VBATREG_SHIFT             2
#define SM5450_REGULATION_VBATREG_REF_MASK             (BIT(1) | BIT(0))
#define SM5450_REGULATION_VBATREG_REF_SHIFT            0

#define SM5450_IBATREG_REF_BELOW_200MA                 200
#define SM5450_IBATREG_REF_BELOW_300MA                 300
#define SM5450_IBATREG_REF_BELOW_400MA                 400
#define SM5450_IBATREG_REF_BELOW_500MA                 500
#define SM5450_IBATREG_REF_BELOW_STEP                  100

#define SM5450_VBATREG_REF_BELOW_50MV                  50
#define SM5450_VBATREG_REF_BELOW_100MV                 100
#define SM5450_VBATREG_REF_BELOW_150MV                 150
#define SM5450_VBATREG_REF_BELOW_200MV                 200
#define SM5450_VBATREG_REF_BELOW_STEP                  50

/* FLAG1 reg=0x0B */
#define SM5450_FLAG1_REG                               0x0B
#define SM5450_FLAG1_VBUSCON_OVP_FLAG_MASK             BIT(7)
#define SM5450_FLAG1_VBUSCON_OVP_FLAG_SHIFT            7
#define SM5450_FLAG1_VBUSCON_PD_FLAG_MASK              BIT(6)
#define SM5450_FLAG1_VBUSCON_PD_FLAG_SHIFT             6
#define SM5450_FLAG1_VBUS_PD_FLAG_MASK                 BIT(5)
#define SM5450_FLAG1_VBUS_PD_FLAG_SHIFT                5
#define SM5450_FLAG1_VBUSOVP_FLAG_MASK                 BIT(3)
#define SM5450_FLAG1_VBUSOVP_FLAG_SHIFT                3
#define SM5450_FLAG1_IBUSOCP_FLAG_MASK                 BIT(2)
#define SM5450_FLAG1_IBUSOCP_FLAG_SHIFT                2
#define SM5450_FLAG1_CHGON_FLAG_MASK                   BIT(1)
#define SM5450_FLAG1_CHGON_FLAG_SHIFT                  1
#define SM5450_FLAG1_IBUSUCP_FALLING_FLAG_MASK         BIT(0)
#define SM5450_FLAG1_IBUSUCP_FALLING_FLAG_SHIFT        0

/* FLAG_MASK1 reg=0x0C */
#define SM5450_FLAG_MASK1_REG                          0x0C
#define SM5450_FLAG_MASK1_INIT_REG                     0x73
#define SM5450_FLAG_MASK1_VBUSCON_OVP_MSK_MASK         BIT(7)
#define SM5450_FLAG_MASK1_VBUSCON_OVP_MSK_SHIFT        7
#define SM5450_FLAG_MASK1_VBUSCON_PD_MSK_MASK          BIT(6)
#define SM5450_FLAG_MASK1_VBUSCON_PD_MSK_SHIFT         6
#define SM5450_FLAG_MASK1_VBUS_PD_MSK_MASK             BIT(5)
#define SM5450_FLAG_MASK1_VBUS_PD_MSK_SHIFT            5
#define SM5450_FLAG_MASK1_VBUSOVP_MSK_MASK             BIT(3)
#define SM5450_FLAG_MASK1_VBUSOVP_MSK_SHIFT            3
#define SM5450_FLAG_MASK1_IBUSOCP_MSK_MASK             BIT(2)
#define SM5450_FLAG_MASK1_IBUSOCP_MSK_SHIFT            2
#define SM5450_FLAG_MASK1_CHGON_MSK_MASK               BIT(1)
#define SM5450_FLAG_MASK1_CHGON_MSK_SHIFT              1
#define SM5450_FLAG_MASK1_IBUSUCP_FALLING_MSK_MASK     BIT(0)
#define SM5450_FLAG_MASK1_IBUSUCP_FALLING_MSK_SHIFT    0

/* FLAG2 reg=0x0D */
#define SM5450_FLAG2_REG                               0x0D
#define SM5450_FLAG2_VBATOVP_FLAG_MASK                 BIT(7)
#define SM5450_FLAG2_VBATOVP_FLAG_SHIFT                7
#define SM5450_FLAG2_IBATOCP_FLAG_MASK                 BIT(6)
#define SM5450_FLAG2_IBATOCP_FLAG_SHIFT                6
#define SM5450_FLAG2_VBATREG_FLAG_MASK                 BIT(5)
#define SM5450_FLAG2_VBATREG_FLAG_SHIFT                5
#define SM5450_FLAG2_IBATREG_FLAG_MASK                 BIT(4)
#define SM5450_FLAG2_IBATREG_FLAG_SHIFT                4
#define SM5450_FLAG2_TSD_FLAG_MASK                     BIT(3)
#define SM5450_FLAG2_TSD_FLAG_SHIFT                    3
#define SM5450_FLAG2_RLTVUVP_FLAG_MASK                 BIT(2)
#define SM5450_FLAG2_RLTVUVP_FLAG_SHIFT                2
#define SM5450_FLAG2_RLTVOVP_FLAG_MASK                 BIT(1)
#define SM5450_FLAG2_RLTVOVP_FLAG_SHIFT                1
#define SM5450_FLAG2_CONOCP_FLAG_MASK                  BIT(0)
#define SM5450_FLAG2_CONOCP_FLAG_SHIFT                 0

/* FLAG_MASK2 reg=0x0E */
#define SM5450_FLAG_MASK2_REG                          0x0E
#define SM5450_FLAG_MASK2_INIT_REG                     0x37
#define SM5450_FLAG_MASK2_VBATOVP_MSK_MASK             BIT(7)
#define SM5450_FLAG_MASK2_VBATOVP_MSK_SHIFT            7
#define SM5450_FLAG_MASK2_IBATOCP_MSK_MASK             BIT(6)
#define SM5450_FLAG_MASK2_IBATOCP_MSK_SHIFT            6
#define SM5450_FLAG_MASK2_VBATREG_MSK_MASK             BIT(5)
#define SM5450_FLAG_MASK2_VBATREG_MSK_SHIFT            5
#define SM5450_FLAG_MASK2_IBATREG_MSK_MASK             BIT(4)
#define SM5450_FLAG_MASK2_IBATREG_MSK_SHIFT            4
#define SM5450_FLAG_MASK2_TSD_MSK_MASK                 BIT(3)
#define SM5450_FLAG_MASK2_TSD_MSK_SHIFT                3
#define SM5450_FLAG_MASK2_RLTVUVP_MSK_MASK             BIT(2)
#define SM5450_FLAG_MASK2_RLTVUVP_MSK_SHIFT            2
#define SM5450_FLAG_MASK2_RLTVOVP_MSK_MASK             BIT(1)
#define SM5450_FLAG_MASK2_RLTVOVP_MSK_SHIFT            1
#define SM5450_FLAG_MASK2_CONOCP_MAK_MASK              BIT(0)
#define SM5450_FLAG_MASK2_CONOCP_MAK_SHIFT             0

/* FLAG3 reg=0x0F */
#define SM5450_FLAG3_REG                               0x0F
#define SM5450_FLAG3_INIT_REG                          0xFF
#define SM5450_FLAG3_VBUSPOK_FLAG_MASK                 BIT(7)
#define SM5450_FLAG3_VBUSPOK_FLAG_SHIFT                7
#define SM5450_FLAG3_VBATPOK_FLAG_MASK                 BIT(6)
#define SM5450_FLAG3_VBATPOK_FLAG_SHIFT                6
#define SM5450_FLAG3_WATCHDOG_FLAG_MASK                BIT(5)
#define SM5450_FLAG3_WATCHDOG_FLAG_SHIFT               5
#define SM5450_FLAG3_VBUSCON_UVLO_FLAG_MASK            BIT(4)
#define SM5450_FLAG3_VBUSCON_UVLO_FLAG_SHIFT           4
#define SM5450_FLAG3_VBUSUVLO_FLAG_MASK                BIT(3)
#define SM5450_FLAG3_VBUSUVLO_FLAG_SHIFT               3
#define SM5450_FLAG3_CHGON_TMR_FLAG_MASK               BIT(2)
#define SM5450_FLAG3_CHGON_TMR_FLAG_SHIFT              2
#define SM5450_FLAG3_ADCDONE_FLAG_MASK                 BIT(1)
#define SM5450_FLAG3_ADCDONE_FLAG_SHIFT                1
#define SM5450_FLAG3_FCAP_SCP_FLAG_MASK                BIT(0)
#define SM5450_FLAG3_FCAP_SCP_FLAG_SHIFT               0

/* FLAG_MASK3 reg=0x10 */
#define SM5450_FLAG_MASK3_REG                          0x10
#define SM5450_FLAG_MASK3_INIT_REG                     0xFF
#define SM5450_FLAG_MASK3_VBUSPOK_FLAG_MSK_MASK        BIT(7)
#define SM5450_FLAG_MASK3_VBUSPOK_FLAG_MSK_SHIFT       7
#define SM5450_FLAG_MASK3_VBATPOK_FLAG_MSK_MASK        BIT(6)
#define SM5450_FLAG_MASK3_VBATPOK_FLAG_MSK_SHIFT       6
#define SM5450_FLAG_MASK3_WATCHDOG_FLAG_MSK_MASK       BIT(5)
#define SM5450_FLAG_MASK3_WATCHDOG_FLAG_MSK_SHIFT      5
#define SM5450_FLAG_MASK3_VBUSCON_UVLO_FLAG_MSK_MASK   BIT(4)
#define SM5450_FLAG_MASK3_VBUSCON_UVLO_FLAG_MSK_SHIFT  4
#define SM5450_FLAG_MASK3_VBUSUVLO_FLAG_MSK_MASK       BIT(3)
#define SM5450_FLAG_MASK3_VBUSUVLO_FLAG_MSK_SHIFT      3
#define SM5450_FLAG_MASK3_CHGON_TMR_FLAG_MSK_MASK      BIT(2)
#define SM5450_FLAG_MASK3_CHGON_TMR_FLAG_MSK_SHIFT     2
#define SM5450_FLAG_MASK3_ADCDONE_FLAG_MSK_MASK        BIT(1)
#define SM5450_FLAG_MASK3_ADCDONE_FLAG_MSK_SHIFT       1
#define SM5450_FLAG_MASK3_FCAP_SCP_FLAG_MSK_MASK       BIT(0)
#define SM5450_FLAG_MASK3_FCAP_SCP_FLAG_MSK_SHIFT      0

/* ADCCTRL reg=0x11 */
#define SM5450_ADCCTRL_REG                             0x11
#define SM5450_ADCCTRL_ENADC_MASK                      BIT(7)
#define SM5450_ADCCTRL_ENADC_SHIFT                     7
#define SM5450_ADCCTRL_ADCMODE_MASK                    BIT(6)
#define SM5450_ADCCTRL_ADCMODE_SHIFT                   6
#define SM5450_ADCCTRL_DIS_VBUSADC_MASK                BIT(5)
#define SM5450_ADCCTRL_DIS_VBUSADC_SHIFT               5
#define SM5450_ADCCTRL_DIS_IBUSADC_MASK                BIT(4)
#define SM5450_ADCCTRL_DIS_IBUSADC_SHIFT               4
#define SM5450_ADCCTRL_DIS_VBATADC_MASK                BIT(3)
#define SM5450_ADCCTRL_DIS_VBATADC_SHIFT               3
#define SM5450_ADCCTRL_DIS_IBATADC_MASK                BIT(2)
#define SM5450_ADCCTRL_DIS_IBATADC_SHIFT               2
#define SM5450_ADCCTRL_DIS_TDIEADC_MASK                BIT(1)
#define SM5450_ADCCTRL_DIS_TDIEADC_SHIFT               1

/* VBUSADC_H reg=0x12 */
#define SM5450_VBUSADC_H_REG                           0x12
#define SM5450_VBUSADC_H_VBUSADC_MASK                  (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_VBUSADC_H_VBUSADC_SHIFT                 0

/* VBUSADC_L reg=0x13 */
#define SM5450_VBUSADC_L_REG                           0x13
#define SM5450_VBUSADC_L_VBUSADC_MASK                  0xFF
#define SM5450_VBUSADC_L_VBUSADC_SHIFT                 0

/* IBUSADC_H reg=0x14 */
#define SM5450_IBUSADC_H_REG                           0x14
#define SM5450_IBUSADC_H_IBUSADC_MASK                  (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_IBUSADC_H_IBUSADC_SHIFT                 0

/* IBUSADC_L reg=0x15 */
#define SM5450_IBUSADC_L_REG                           0x15
#define SM5450_IBUSADC_L_IBUSADC_MASK                  0xFF
#define SM5450_IBUSADC_L_IBUSADC_SHIFT                 0

/* VBAT_H reg=0x16 */
#define SM5450_VBATADC_H_REG                           0x16
#define SM5450_VBATADC_H_VBAT_MASK                     (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_VBATADC_H_VBAT_SHIFT                    0

/* VBAT_L reg=0x17 */
#define SM5450_VBATADC_L_REG                           0x17
#define SM5450_VBATADC_L_VBAT_MASK                     0xFF
#define SM5450_VBAT_L_VBAT_SHIFT                       0

/* IBAT_H reg=0x18 */
#define SM5450_IBATADC_H_REG                           0x18
#define SM5450_IBATADC_H_IBAT_MASK                     (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SM5450_IBATADC_H_IBAT_SHIFT                    0

/* IBAT_L reg=0x19 */
#define SM5450_IBATADC_L_REG                           0x19
#define SM5450_IBATADC_L_IBAT_MASK                     0xFF
#define SM5450_IBATADC_L_IBAT_SHIFT                    0

/* TDIEADC reg=0x1A */
#define SM5450_TDIEADC_REG                             0x1A
#define SM5450_TDIEADC_TDIEADC_MASK                    0xFF
#define SM5450_TDIEADC_TDIEADC_SHIFT                   0

int sm5450_config_vbuscon_ovp_ref_mv(int ovp_threshold, void *dev_data);
int sm5450_config_vbus_ovp_ref_mv(int ovp_threshold, void *dev_data);
int sm5450_get_vbus_mv(int *vbus, void *dev_data);
int sm5450_is_support_fcp(void *dev_data);

#endif /* _SM5450_H_ */
