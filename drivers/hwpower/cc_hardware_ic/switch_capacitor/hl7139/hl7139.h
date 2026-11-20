/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hl7139.h
 *
 * hl7139 header file
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

#ifndef _HL7139_H_
#define _HL7139_H_

#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

#define HL7139_DEV_NAME_LEN                            32

struct hl7139_param {
	int scp_support;
	int fcp_support;
};

struct hl7139_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data nty_data;
	struct hl7139_param param_dts;
	struct dc_ic_ops sc_ops;
	struct dc_ic_ops lvc_ops;
	struct dc_batinfo_ops batinfo_ops;
	struct power_log_ops log_ops;
	char name[HL7139_DEV_NAME_LEN];
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
	int vbat_ovp_para;
	int ibat_ocp_para;
	int vin_ovp_para;
	int iin_ocp_para;
	int ignore_discharge_flag;
	int sc_ibus_ocp;
	struct mutex scp_detect_lock;
	struct mutex accp_adapter_reg_lock;
	unsigned int fcp_support;
	unsigned int adapter_support;
	unsigned int scp_isr_backup[2]; /* datasheet:scp_isr1,scp_isr2 */
	unsigned int close_ibat_ocp;
};

#define HL7139_INIT_FINISH                             1
#define HL7139_NOT_INIT                                0
#define HL7139_ENABLE_INT_NOTIFY                       1
#define HL7139_DISABLE_INT_NOTIFY                      0

#define HL7139_NOT_USED                                0
#define HL7139_USED                                    1
#define HL7139_DEVICE_ID_GET_FAIL                      (-1)
#define HL7139_REG_RESET                               1
#define HL7139_SWITCHCAP_DISABLE                       0

#define HL7139_LENGTH_OF_BYTE                          8

#define HL7139_ADC_RESOLUTION_4                        4
#define HL7139_ADC_STANDARD_TDIE                       40

#define HL7139_ADC_DISABLE                             0
#define HL7139_ADC_ENABLE                              1

/* DEVICE_ID reg=0x00 */
#define HL7139_DEVICE_ID_REG                           0x00
#define HL7139_DEVICE_ID_DEV_REV_MASK                  (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define HL7139_DEVICE_ID_DEV_REV_SHIFT                 4
#define HL7139_DEVICE_ID_DEV_ID_MASK                   (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define HL7139_DEVICE_ID_DEV_ID_SHIFT                  0
#define HL7139_DEVICE_ID_HL7139                        10

/* INT reg=0x01 */
#define HL7139_INT_REG                                 0x01
#define HL7139_INT_REG_INIT                            0xFF
#define HL7139_INT_STATE_CHG_I_MASK                    BIT(7)
#define HL7139_INT_STATE_CHG_I_SHIFT                   7
#define HL7139_INT_REG_I_MASK                          BIT(6)
#define HL7139_INT_REG_I_SHIFT                         6
#define HL7139_INT_TS_TEMP_I_MASK                      BIT(5)
#define HL7139_INT_TS_TEMP_I_SHIFT                     5
#define HL7139_INT_V_OK_I_MASK                         BIT(4)
#define HL7139_INT_V_OK_I_SHIFT                        4
#define HL7139_INT_CUR_I_MASK                          BIT(3)
#define HL7139_INT_CUR_I_SHIFT                         3
#define HL7139_INT_SHORT_I_MASK                        BIT(2)
#define HL7139_INT_SHORT_I_SHIFT                       2
#define HL7139_INT_WDOG_I_MASK                         BIT(1)
#define HL7139_INT_WDOG_I_SHIFT                        1
#define HL7139_INT_PROTOCOL_I_MASK                     BIT(0)
#define HL7139_INT_PROTOCOL_I_SHIFT                    0

/* INT_MSK reg=0x02 */
#define HL7139_INT_MSK_REG                             0x02
#define HL7139_INT_MSK_STATE_CHG_M_MASK                BIT(7)
#define HL7139_INT_MSK_STATE_CHG_M_SHIFT               7
#define HL7139_INT_MSK_REG_M_MASK                      BIT(6)
#define HL7139_INT_MSK_REG_M_SHIFT                     6
#define HL7139_INT_MSK_TS_TEMP_M_MASK                  BIT(5)
#define HL7139_INT_MSK_TS_TEMP_M_SHIFT                 5
#define HL7139_INT_MSK_V_OK_M_MASK                     BIT(4)
#define HL7139_INT_MSK_V_OK_M_SHIFT                    4
#define HL7139_INT_MSK_CUR_M_MASK                      BIT(3)
#define HL7139_INT_MSK_CUR_M_SHIFT                     3
#define HL7139_INT_MSK_SHORT_M_MASK                    BIT(2)
#define HL7139_INT_MSK_SHORT_M_SHIFT                   2
#define HL7139_INT_MSK_WDOG_M_MASK                     BIT(1)
#define HL7139_INT_MSK_WDOG_M_SHIFT                    1
#define HL7139_INT_MSK_PROTOCOL_M_MASK                 BIT(0)
#define HL7139_INT_MSK_PROTOCOL_M_SHIFT                0

/* INT_STS_A reg=0x03 */
#define HL7139_INT_STS_A_REG                           0x03
#define HL7139_INT_STS_A_STATE_CHG_STS_MASK            (BIT(7) | BIT(6))
#define HL7139_INT_STS_A_STATE_CHG_STS_SHIFT           6
#define HL7139_INT_STS_A_REG_STS_MASK                  (BIT(5) | BIT(4) | BIT(3) | BIT(2))
#define HL7139_INT_STS_A_REG_STS_SIFT                  2
#define HL7139_INT_STS_A_TS_TEMP_STS_MASK              BIT(1)
#define HL7139_INT_STS_A_TS_TEMP_STS_SHIFT             1

/* INT_STS_A reg=0x04 */
#define HL7139_INT_STS_B_REG                           0x04
#define HL7139_INT_STS_B_V_NOT_OK_STS_MASK             BIT(4)
#define HL7139_INT_STS_B_V_NOT_OK_STS_SHIFT            4
#define HL7139_INT_STS_B_CUR_STS_MASK                  BIT(3)
#define HL7139_INT_STS_B_CUR_STS_SIFT                  3
#define HL7139_INT_STS_B_SHORT_STS_MASK                BIT(2)
#define HL7139_INT_STS_B_SHORT_STS_SHIFT               2
#define HL7139_INT_STS_B_WDOG_STS_MASK                 BIT(1)
#define HL7139_INT_STS_B_WDOG_STS_SHIFT                1

/* STATUS_A reg=0x05 */
#define HL7139_STATUS_A_REG                            0x05
#define HL7139_STATUS_A_REG_INIT                       0x77
#define HL7139_STATUS_A_VIN_OVP_STS_MASK               BIT(7)
#define HL7139_STATUS_A_VIN_OVP_STS_SHIFT              7
#define HL7139_STATUS_A_VIN_UVLO_STS_MASK              BIT(6)
#define HL7139_STATUS_A_VIN_UVLO_STS_SHIFT             6
#define HL7139_STATUS_A_TRACK_OV_STS_MASK              BIT(5)
#define HL7139_STATUS_A_TRACK_OV_STS_SHIFT             5
#define HL7139_STATUS_A_TRACK_UV_STS_MASK              BIT(4)
#define HL7139_STATUS_A_TRACK_UV_STS_SHIFT             4
#define HL7139_STATUS_A_VBAT_OVP_STS_MASK              BIT(3)
#define HL7139_STATUS_A_VBAT_OVP_STS_SHIFT             3
#define HL7139_STATUS_A_VOUT_OVP_STS_MASK              BIT(2)
#define HL7139_STATUS_A_VOUT_OVP_STS_SHIFT             2
#define HL7139_STATUS_A_PMID_QUAL_STS_MASK             BIT(1)
#define HL7139_STATUS_A_PMID_QUAL_STS_SHIFT            1
#define HL7139_STATUS_A_VBUS_UV_STS_MASK               BIT(0)
#define HL7139_STATUS_A_VBUS_UV_STS_SHIFT              0

/* STATUS_B reg=0x06 */
#define HL7139_STATUS_B_REG                            0x06
#define HL7139_STATUS_B_REG_INIT                       0x3E
#define HL7139_STATUS_B_IIN_OCP_STS_MASK               BIT(7)
#define HL7139_STATUS_B_IIN_OCP_STS_SHIFT              7
#define HL7139_STATUS_B_IBAT_OCP_STS_MASK              BIT(6)
#define HL7139_STATUS_B_IBAT_OCP_STS_SHIFT             6
#define HL7139_STATUS_B_IIN_UCP_STS_MASK               BIT(5)
#define HL7139_STATUS_B_IIN_UCP_STS_SHIFT              5
#define HL7139_STATUS_B_FET_SHORT_STS_MASK             BIT(4)
#define HL7139_STATUS_B_FET_SHORT_STS_SHIFT            4
#define HL7139_STATUS_B_CFLY_SHORT_STS_MASK            BIT(3)
#define HL7139_STATUS_B_CFLY_SHORT_STS_SHIFT           3
#define HL7139_STATUS_B_DEV_MODE_STS_MASK              (BIT(2) | BIT(1))
#define HL7139_STATUS_B_DEV_MODE_STS_SHIFT             1
#define HL7139_STATUS_B_THSD_STS_MASK                  BIT(0)
#define HL7139_STATUS_B_THSD_STS_SHIFT                 0

/* STATUS_C reg=0x07 */
#define HL7139_STATUS_C_REG                            0x07
#define HL7139_STATUS_C_QPUMP_STS_MASK                 BIT(7)
#define HL7139_STATUS_C_QPUMP_STS_SHIFT                7
#define HL7139_STATUS_C_VBUS_GOOD_S_MASK               BIT(0)
#define HL7139_STATUS_C_VBUS_UVP_MODE                  1

/* VBAT_REG reg=0x08 */
#define HL7139_VBAT_REG_REG                            0x08
#define HL7139_VBAT_REG_VBAT_OVP_DIS_MASK              BIT(7)
#define HL7139_VBAT_REG_VBAT_OVP_DIS_SHIFT             7
#define HL7139_VBAT_REG_TVBAT_OVP_DEB_MASK             BIT(6)
#define HL7139_VBAT_REG_TVBAT_OVP_DEB_SHIFT            6
#define HL7139_VBAT_REG_VBAT_REG_TH_MASK               (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define HL7139_VBAT_REG_VBAT_REG_TH_SHIFT              0

/* VBAT_REG_TH(V) = 4.0V + DEC[5:0] * 10mV */
#define HL7139_VBAT_REG_TH_MIN                         4000
#define HL7139_VBAT_REG_TH_MAX                         4600
#define HL7139_VBAT_REG_TH_STEP                        10
#define HL7139_VBAT_OVP_INIT                           4430

/* STBY_CTRL reg=0x09 */
#define HL7139_STBY_CTRL_REG                           0x09
#define HL7139_STBY_CTRL_DIG_CLK_DIS_MASK              BIT(1)
#define HL7139_STBY_CTRL_DIG_CLK_DIS_SHIFT             1
#define HL7139_STBY_CTRL_DEEP_STBY_EN_MASK             BIT(0)
#define HL7139_STBY_CTRL_DEEP_STBY_EN_SHIFT            0

/* IBAT_REG reg=0x0A */
#define HL7139_IBAT_REG_REG                            0x0A
#define HL7139_IBAT_REG_IBAT_OCP_DIS_MASK              BIT(7)
#define HL7139_IBAT_REG_IBAT_OCP_DIS_SHIFT             7
#define HL7139_IBAT_REG_TIBAT_OCP_DEB_MASK             BIT(6)
#define HL7139_IBAT_REG_TIBAT_OCP_DEB_SHIFT            6
#define HL7139_IBAT_REG_IBAT_REG_TH_MASK               (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define HL7139_IBAT_REG_IBAT_REG_TH_SHIFT              0

/* IBAT_REG(A) = 2A + DEC[5:0] * 0.1A */
#define HL7139_IBAT_REG_TH_MIN                         2000
#define HL7139_IBAT_REG_TH_MAX                         6600
#define HL7139_IBAT_REG_TH_STEP                        100
#define HL7139_IBAT_OCP_INIT                           7100

/* VBUS_OVP reg=0x0B */
#define HL7139_VBUS_OVP_REG                            0x0B
#define HL7139_VBUS_OVP_EXT_NFET_USE_MASK              BIT(7)
#define HL7139_VBUS_OVP_EXT_NFET_USE_SHIFT             7
#define HL7139_VBUS_OVP_VBUS_PD_EN_MASK                BIT(6)
#define HL7139_VBUS_OVP_VBUS_PD_EN_SHIFT               6
#define HL7139_VBUS_OVP_VGS_SEL_MASK                   (BIT(5) | BIT(4))
#define HL7139_VBUS_OVP_VGS_SEL_SHIFT                  4
#define HL7139_VBUS_OVP_VBUS_OVP_TH_MASK               (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define HL7139_VBUS_OVP_VBUS_OVP_SHIFT                 0

/* VGS_SEL(V) = 7V + DEC[5:4] * 0.5V */
#define HL7139_VGS_SEL_MIN                             7000
#define HL7139_VGS_SEL_MAX                             8500
#define HL7139_VGS_SEL_BASE_STEP                       500
#define HL7139_VGS_SEL_INIT                            7000

/* VBUS_OVP(V) = 4V + DEC[3:0] * 1V */
#define HL7139_VBUS_OVP_MIN                            4000
#define HL7139_VBUS_OVP_MAX                            19000
#define HL7139_VBUS_OVP_STEP                           1000
#define HL7139_VBUS_OVP_INIT                           12000

/* VIN_OVP reg=0x0C */
#define HL7139_VIN_OVP_REG                             0x0C
#define HL7139_VIN_OVP_VIN_PD_CFG_MASK                 BIT(5)
#define HL7139_VIN_OVP_VIN_PD_CFG_SHIFT                5
#define HL7139_VIN_OVP_VIN_OVP_DIS_MASK                BIT(4)
#define HL7139_VIN_OVP_VIN_OVP_DIS_SHIFT               4
#define HL7139_VIN_OVP_VIN_OVP_MASK                    (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define HL7139_VIN_OVP_VIN_OVP_SHIFT                   0

/* CPMODE VIN_OVP(V)= 10.2V + DEC[3:0] * 0.1V */
#define HL7139_VIN_OVP_CP_MIN                          10200
#define HL7139_VIN_OVP_CP_MAX                          11700
#define HL7139_VIN_OVP_CP_STEP                         100
#define HL7139_VIN_OVP_CP_INIT                         10500

/* BPMODE VIN_OVP(V)= 5.1V + DEC[3:0] * 50mV */
#define HL7139_VIN_OVP_BP_MIN                          5100
#define HL7139_VIN_OVP_BP_MAX                          5850
#define HL7139_VIN_OVP_BP_STEP                         50
#define HL7139_VIN_OVP_BP_INIT                         5250

/* IIN_REG reg=0x0E */
#define HL7139_IIN_REG_REG                             0x0E
#define HL7139_IIN_REG_IIN_OCP_DIS_MASK                BIT(7)
#define HL7139_IIN_REG_IIN_OCP_DIS_SHIFT               7
#define HL7139_IIN_REG_IIN_REG_TH_MASK                 (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define HL7139_IIN_REG_IIN_REG_TH_SHIFT                0

/* CPMODE IIN_REG_TH(A) = 1A + DEC[5:0] * 50mA */
#define HL7139_IIN_REG_TH_CP_MIN                       1000
#define HL7139_IIN_REG_TH_CP_MAX                       3500
#define HL7139_IIN_REG_TH_CP_STEP                      50
#define HL7139_IIN_OCP_CP_INIT                         3100

/* BPMODE IIN_REG_TH(A) = 2A + DEC[5:0] * 100mA */
#define HL7139_IIN_REG_TH_BP_MIN                       2000
#define HL7139_IIN_REG_TH_BP_MAX                       7000
#define HL7139_IIN_REG_TH_BP_STEP                      100
#define HL7139_IIN_OCP_BP_INIT                         6100

/* IIN_OC reg=0x0F */
#define HL7139_IIN_OC_REG                              0x0F
#define HL7139_IIN_OC_TIIN_OC_DEB_MASK                 BIT(7)
#define HL7139_IIN_OC_TIIN_OC_DEB_SHIFT                7
#define HL7139_IIN_OC_IIN_OCP_TH_MASK                  (BIT(6) | BIT(5))
#define HL7139_IIN_OC_IIN_OCP_TH_SHIFT                 5

/* IIN_OCP(A) = 0.1A + DEC[6:5] * 50mA */
#define HL7139_IIN_OCP_TH_ABOVE_100MA                  100
#define HL7139_IIN_OCP_TH_ABOVE_150MA                  150
#define HL7139_IIN_OCP_TH_ABOVE_200MA                  200
#define HL7139_IIN_OCP_TH_ABOVE_250MA                  250
#define HL7139_IIN_OCP_TH_ABOVE_STEP                   50

/* REG_CTRL_0 reg=0x10 */
#define HL7139_REG_CTRL0_REG                           0x10
#define HL7139_REG_CTRL0_TDIE_REG_DIS_MASK             BIT(7)
#define HL7139_REG_CTRL0_TDIE_REG_DIS_SHIFT            7
#define HL7139_REG_CTRL0_IIN_REG_DIS_MASK              BIT(6)
#define HL7139_REG_CTRL0_IIN_REG_DIS_SHIFT             6
#define HL7139_REG_CTRL0_IIN_REGULATION_OFF            1
#define HL7139_REG_CTRL0_IIN_REGULATION_ON             0
#define HL7139_REG_CTRL0_TDIE_REG_TH_MASK              (BIT(5) | BIT(4))
#define HL7139_REG_CTRL0_TDIE_REG_TH_SHIFT             4

/* REG_CTRL_1 reg=0x11 */
#define HL7139_REG_CTRL1_REG                           0x11
#define HL7139_REG_CTRL1_VBAT_REG_DIS_MASK             BIT(7)
#define HL7139_REG_CTRL1_VBAT_REG_DIS_SHIFT            7
#define HL7139_REG_CTRL1_VBAT_REGULATION_OFF           1
#define HL7139_REG_CTRL1_VBAT_REGULATION_ON            0
#define HL7139_REG_CTRL1_IBAT_REG_DIS_MASK             BIT(6)
#define HL7139_REG_CTRL1_IBAT_REG_DIS_SHIFT            6
#define HL7139_REG_CTRL1_IBAT_REGULATION_OFF           1
#define HL7139_REG_CTRL1_IBAT_REGULATION_ON            0
#define HL7139_REG_CTRL1_VBAT_OVP_TH_MASK              (BIT(5) | BIT(4))
#define HL7139_REG_CTRL1_VBAT_OVP_TH_SHIFT             4
#define HL7139_REG_CTRL1_IBAT_OCP_TH_MASK              (BIT(3) | BIT(2))
#define HL7139_REG_CTRL1_IBAT_OCP_TH_SHIFT             2

/* VBAT_OVP(V) = 80mV + DEC[5:4] * 10mV */
#define HL7139_VBAT_OVP_TH_ABOVE_80MV                  80
#define HL7139_VBAT_OVP_TH_ABOVE_90MV                  90
#define HL7139_VBAT_OVP_TH_ABOVE_100MV                 100
#define HL7139_VBAT_OVP_TH_ABOVE_110MV                 110
#define HL7139_VBAT_OVP_TH_ABOVE_STEP                  10

/* IBAT_OVP(A) = 200mA + DEC[3:2] * 100mA */
#define HL7139_IBAT_OCP_TH_ABOVE_200MA                 200
#define HL7139_IBAT_OCP_TH_ABOVE_300MA                 300
#define HL7139_IBAT_OCP_TH_ABOVE_400MA                 400
#define HL7139_IBAT_OCP_TH_ABOVE_500MA                 500
#define HL7139_IBAT_OCP_TH_ABOVE_STEP                  100

/* CTRL_0 reg=0x12 */
#define HL7139_CTRL0_REG                               0x12
#define HL7139_CTRL0_CHG_EN_MASK                       BIT(7)
#define HL7139_CTRL0_CHG_EN_SHIFT                      7
#define HL7139_CTRL0_CHG_EN                            1
#define HL7139_CTRL0_CHG_OFF                           0
#define HL7139_CTRL0_FSW_SET_MASK                      (BIT(6) | BIT(5) | BIT(4) | BIT(3))
#define HL7139_CTRL0_FSW_SET_SHIFT                     3
#define HL7139_CTRL0_UNPLUG_DET_EN_MASK                BIT(2)
#define HL7139_CTRL0_UNPLUG_DET_EN_SHIFT               2
#define HL7139_CTRL0_IIN_UCP_TH_MASK                   (BIT(1) | BIT(0))
#define HL7139_CTRL0_IIN_UCP_TH_SHIFT                  0

#define HL7139_SW_FREQ_500KHZ                          500
#define HL7139_SW_FREQ_700KHZ                          700
#define HL7139_SW_FREQ_900KHZ                          900
#define HL7139_SW_FREQ_1100KHZ                         1100
#define HL7139_SW_FREQ_1300KHZ                         1300
#define HL7139_SW_FREQ_1600KHZ                         1600

#define HL7139_FSW_SET_SW_FREQ_500KHZ                  0x00
#define HL7139_FSW_SET_SW_FREQ_600KHZ                  0x01
#define HL7139_FSW_SET_SW_FREQ_700KHZ                  0x02
#define HL7139_FSW_SET_SW_FREQ_800KHZ                  0x03
#define HL7139_FSW_SET_SW_FREQ_900KHZ                  0x04
#define HL7139_FSW_SET_SW_FREQ_1000KHZ                 0x05
#define HL7139_FSW_SET_SW_FREQ_1100KHZ                 0x06
#define HL7139_FSW_SET_SW_FREQ_1200KHZ                 0x07
#define HL7139_FSW_SET_SW_FREQ_1300KHZ                 0x08
#define HL7139_FSW_SET_SW_FREQ_1400KHZ                 0x09
#define HL7139_FSW_SET_SW_FREQ_1500KHZ                 0x0a
#define HL7139_FSW_SET_SW_FREQ_1600KHZ                 0x0b

/* CPMODE IIN_UCP(A) = 0.1A + DEC[1:0] * 50mA */
#define HL7139_IIN_UCP_TH_CP_MIN                       100
#define HL7139_IIN_UCP_TH_CP_MAX                       250
#define HL7139_IIN_UCP_TH_CP_STEP                      50

/* BPMODE IIN_OVP(A) = 0.2A + DEC[1:0] * 100mA */
#define HL7139_IIN_UCP_TH_BP_MIN                       200
#define HL7139_IIN_UCP_TH_BP_MAX                       500
#define HL7139_IIN_UCP_TH_BP_STEP                      100

/* CTRL_1 reg=0x13 */
#define HL7139_CTRL1_REG                               0x13
#define HL7139_CTRL1_UCP_DEB_SEL_MASK                  (BIT(7) | BIT(6))
#define HL7139_CTRL1_UCP_DEB_SEL_SHIFT                 6
#define HL7139_CTRL1_VOUT_OVP_DIS_MASK                 BIT(5)
#define HL7139_CTRL1_VOUT_OVP_DIS_SHIFT                5
#define HL7139_CTRL1_TS_PROT_EN_MASK                   BIT(4)
#define HL7139_CTRL1_TS_PROT_EN_SHIFT                  4
#define HL7139_CTRL1_AUTO_V_REC_EN_MASK                BIT(2)
#define HL7139_CTRL1_AUTO_V_REC_EN_SHIFT               2
#define HL7139_CTRL1_AUTO_I_REC_EN_MASK                BIT(1)
#define HL7139_CTRL1_AUTO_I_REC_EN_SHIFT               1
#define HL7139_CTRL1_AUTO_UCP_REC_EN_MASK              BIT(0)
#define HL7139_CTRL1_AUTO_UCP_REC_EN_SHIFT             0

#define HL7139_UCP_DEB_SEL_10MS                        10
#define HL7139_UCP_DEB_SEL_100MS                       100
#define HL7139_UCP_DEB_SEL_500MS                       500
#define HL7139_UCP_DEB_SEL_1000MS                      1000

/* CTRL_2 reg=0x14 */
#define HL7139_CTRL2_REG                               0x14
#define HL7139_CTRL2_SFT_RST_MASK                      (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define HL7139_CTRL2_SFT_RST_SHIFT                     4
#define HL7139_CTRL2_WD_DIS_MASK                       BIT(3)
#define HL7139_CTRL2_WD_DIS_SHIFT                      3
#define HL7139_CTRL2_WD_TMR_MASK                       (BIT(2) | BIT(1) | BIT(0))
#define HL7139_CTRL2_WD_TMR_SHIFT                      0

#define HL7139_CTRL2_SFT_RST_ENABLE                    0x0c
#define HL7139_WD_TMR_200MS                            200
#define HL7139_WD_TMR_500MS                            500
#define HL7139_WD_TMR_1000MS                           1000
#define HL7139_WD_TMR_2000MS                           2000
#define HL7139_WD_TMR_5000MS                           5000
#define HL7139_WD_TMR_10000MS                          10000
#define HL7139_WD_TMR_20000MS                          20000
#define HL7139_WD_TMR_40000MS                          40000

/* CTRL_3 reg=0x15 */
#define HL7139_CTRL3_REG                               0x15
#define HL7139_CTRL3_DEV_MODE_MASK                     BIT(7)
#define HL7139_CTRL3_DEV_MODE_SHIFT                    7
#define HL7139_CTRL3_DPDM_CFG_MASK                     BIT(2)
#define HL7139_CTRL3_DPDM_CFG_SHIFT                    2
#define HL7139_CTRL3_GPP_CFG_MASK                      (BIT(1) | BIT(0))
#define HL7139_CTRL3_GPP_CFG_SHIFT                     0

#define HL7139_CTRL3_CPMODE                            0
#define HL7139_CTRL3_BPMODE                            1

/* TRACK_OV_UV reg=0x16 */
#define HL7139_TRACK_OV_UV_REG                         0x16
#define HL7139_TRACK_OV_UV_TRACK_OV_DIS_MASK           BIT(7)
#define HL7139_TRACK_OV_UV_TRACK_OV_DIS_SHIFT          7
#define HL7139_TRACK_OV_UV_TRACK_UV_DIS_MASK           BIT(6)
#define HL7139_TRACK_OV_UV_TRACK_UV_DIS_SHIFT          6
#define HL7139_TRACK_OV_UV_TRACK_OV_MASK               (BIT(5) | BIT(4) | BIT(3))
#define HL7139_TRACK_OV_UV_TRACK_OV_SHIFT              3
#define HL7139_TRACK_OV_UV_TRACK_UV_MASK               (BIT(2) | BIT(1) | BIT(0))
#define HL7139_TRACK_OV_UV_TRACK_UV_SHIFT              0

/* TRACK_OV(V) = 200mV + DEC[5:3] * 100mV */
#define HL7139_TRACK_OV_UV_TRACK_OV_MIN                200
#define HL7139_TRACK_OV_UV_TRACK_OV_MAX                900
#define HL7139_TRACK_OV_UV_TRACK_OV_INIT               600
#define HL7139_TRACK_OV_UV_TRACK_OV_STEP               100

/* TRACK_UV(V) = 50mV + DEC[2:0] * 50mV */
#define HL7139_TRACK_OV_UV_TRACK_UV_MIN                50
#define HL7139_TRACK_OV_UV_TRACK_UV_MAX                400
#define HL7139_TRACK_OV_UV_TRACK_UV_INIT               250
#define HL7139_TRACK_OV_UV_TRACK_UV_STEP               50

/* TS_TH reg=0x17 */
#define HL7139_TS_TH_REG                               0x17
#define HL7139_TS_TH_TS_TH_MASK                        0xFF
#define HL7139_TS_TH_TS_TH_SHIFT                       0

/* ADC_CTRL0 reg=0x40 */
#define HL7139_ADC_CTRL0_REG                           0x40
#define HL7139_ADC_CTRL0_ADC_REG_COPY_MASK             BIT(7)
#define HL7139_ADC_CTRL0_ADC_REG_COPY_SHIFT            7
#define HL7139_ADC_CTRL0_ADC_MAN_COPY_MASK             BIT(6)
#define HL7139_ADC_CTRL0_ADC_MAN_COPY_SHIFT            6
#define HL7139_ADC_CTRL0_ADC_MODE_CFG_MASK             (BIT(4) | BIT(3))
#define HL7139_ADC_CTRL0_ADC_MODE_CFG_SHIFT            3
#define HL7139_ADC_CTRL0_ADC_AVG_TIME_MASK             (BIT(2) | BIT(1))
#define HL7139_ADC_CTRL0_ADC_AVG_TIME_SHIFT            1
#define HL7139_ADC_CTRL0_ADC_READ_EN_MASK              BIT(0)
#define HL7139_ADC_CTRL0_ADC_READ_EN_SHIFT             0

/* ADC_CTRL1 reg=0x41 */
#define HL7139_ADC_CTRL1_REG                           0x41
#define HL7139_ADC_CTRL1_VIN_ADC_DIS_MASK              BIT(7)
#define HL7139_ADC_CTRL1_VIN_ADC_DIS_SHIFT             7
#define HL7139_ADC_CTRL1_IIN_ADC_DIS_MASK              BIT(6)
#define HL7139_ADC_CTRL1_IIN_ADC_DIS_SHIFT             6
#define HL7139_ADC_CTRL1_VBAT_ADC_DIS_MASK             BIT(5)
#define HL7139_ADC_CTRL1_VBAT_ADC_DIS_SHIFT            5
#define HL7139_ADC_CTRL1_IBAT_ADC_DIS_MASK             BIT(4)
#define HL7139_ADC_CTRL1_IBAT_ADC_DIS_SHIFT            4
#define HL7139_ADC_CTRL1_TS_ADC_DIS_MASK               BIT(3)
#define HL7139_ADC_CTRL1_TS_ADC_DIS_SHIFT              3
#define HL7139_ADC_CTRL1_TDIE_ADC_DIS_MASK             BIT(2)
#define HL7139_ADC_CTRL1_TDIE_ADC_DIS_SHIFT            2
#define HL7139_ADC_CTRL1_VOUT_ADC_DIS_MASK             BIT(1)
#define HL7139_ADC_CTRL1_VOUT_ADC_DIS_SHIFT            1

/* ADC_VIN_0 reg=0x42 */
#define ADC_VIN_0_REG                                  0x42
#define ADC_VIN_0_ADC_VIN_MASK                         0xFF
#define ADC_VIN_0_ADC_VIN_SHIFT                        0

/* ADC_VIN_1 reg=0x43 */
#define ADC_VIN_1_REG                                  0x43
#define ADC_VIN_1_ADC_VIN_MASK                         (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_VIN_1_ADC_VIN_SHIFT                        0

/* ADC_IIN_0 reg=0x44 */
#define ADC_IIN_0_REG                                  0x44
#define ADC_IIN_0_ADC_IIN_MASK                         0xFF
#define ADC_IIN_0_ADC_IIN_SHIFT                        0

/* ADC_IIN_1 reg=0x45 */
#define ADC_IIN_1_REG                                  0x45
#define ADC_IIN_1_ADC_IIN_MASK                         (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_IIN_1_ADC_IIN_SHIFT                        0

#define ADC_REG_NUM                                    2
#define ADC_REG_HIGH                                   0
#define ADC_REG_LOW                                    1
#define ADC_GET_VALID_NUM                              0x0F
#define ADC_INVALID_BIT                                4

/* ADC_VBAT_0 reg=0x46 */
#define ADC_VBAT_0_REG                                 0x46
#define ADC_VBAT_0_ADC_VBAT_MASK                       0xFF
#define ADC_VBAT_0_ADC_VBAT_SHIFT                      0

/* ADC_VBAT_1 reg=0x47 */
#define ADC_VBAT_1_REG                                 0x47
#define ADC_VBAT_1_ADC_VBAT_MASK                       (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_VBAT_1_ADC_VBAT_SHIFT                      0

/* ADC_IBAT_0 reg=0x48 */
#define ADC_IBAT_0_REG                                 0x48
#define ADC_IBAT_0_ADC_IBAT_MASK                       0xFF
#define ADC_IBAT_0_ADC_IBAT_SHIFT                      0

/* ADC_IBAT_1 reg=0x49 */
#define ADC_IBAT_1_REG                                 0x49
#define ADC_IBAT_1_ADC_IBAT_MASK                       (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_IBAT_1_ADC_IBAT_SHIFT                      0

/* ADC_VTS_0 reg=0x4A */
#define ADC_VTS_0_REG                                  0x4A
#define ADC_VTS_0_ADC_VTS_MASK                         0xFF
#define ADC_VTS_0_ADC_VTS_SHIFT                        0

/* ADC_VTS_1 reg=0x4B */
#define ADC_VTS_1_REG                                  0x4B
#define ADC_VTS_1_ADC_VTS_MASK                         (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_VTS_1_ADC_VTS_SHIFT                        0

/* ADC_VOUT_0 reg=0x4C */
#define ADC_VOUT_0_REG                                 0x4C
#define ADC_VOUT_0_ADC_VOUT_MASK                       0xFF
#define ADC_VOUT_0_ADC_VOUT_SHIFT                      0

/* ADC_VOUT_1 reg=0x4D */
#define ADC_VOUT_1_REG                                 0x4D
#define ADC_VOUT_1_ADC_VOUT_MASK                       (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_VOUT_1_ADC_VOUT_SHIFT                      0

/* ADC_TDIE_0 reg=0x4E */
#define ADC_TDIE_0_REG                                 0x4E
#define ADC_TDIE_0_ADC_TDIE_MASK                       0xFF
#define ADC_TDIE_0_ADC_TDIE_SHIFT                      0

/* ADC_TDIE_1 reg=0x4F */
#define ADC_TDIE_1_REG                                 0x4F
#define ADC_TDIE_1_ADC_TDIE_MASK                       (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define ADC_TDIE_1_ADC_TDIE_SHIFT                      0

int hl7139_config_vbuscon_ovp_ref_mv(int ovp_threshold, void *dev_data);
int hl7139_config_vbus_ovp_ref_mv(int ovp_threshold, void *dev_data);
int hl7139_get_vbus_mv(int *vbus, void *dev_data);
int hl7139_get_vbus_uvp_status(void *dev_data);
int hl7139_is_support_fcp(void *dev_data);

#endif /* _HL7139_H_ */
