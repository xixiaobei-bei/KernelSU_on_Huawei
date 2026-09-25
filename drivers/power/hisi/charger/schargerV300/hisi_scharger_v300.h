/*
 * hisi_scharger_v300.h
 *
 * HI6523 charger driver header
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#ifndef __HI6523_CHARGER_H__
#define __HI6523_CHARGER_H__
#include <linux/printk.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <soc_schargerV300_interface.h>
#include <linux/hisi/usb/hisi_usb.h>

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>

#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/power_supply.h>
#include <linux/raid/pq.h>
#include <linux/slab.h>
#include <linux/usb/otg.h>
#include <pmic_interface.h>

#define HI6523_MODULE_NAME                  hi6523
#define HI6523_MODULE_NAME_STR              "hi6523"

#define SCHARGER_V300_INFO
#ifndef SCHARGER_V300_INFO
#define scharger_err(fmt, args...) do {} while (0)
#define scharger_evt(fmt, args...) do {} while (0)
#define scharger_inf(fmt, args...) do {} while (0)
#else
#define scharger_err(fmt, args...) do { pr_err("[hisi_scharger]" fmt, ## args); } while (0)
#define scharger_evt(fmt, args...) do { pr_warn("[hisi_scharger]" fmt, ## args); } while (0)
#define scharger_inf(fmt, args...) do { pr_info("[hisi_scharger]" fmt, ## args); } while (0)
#endif

#define FCP_ADAPTER_CNTL_REG                (SOC_SCHARGER_FCP_ADAP_CTRL_ADDR(0))
#define HI6523_ACCP_CHARGER_DET             (1 <<  SOC_SCHARGER_FCP_ADAP_CTRL_fcp_set_d60m_r_START)

#define WATER_VOLT_PARA_LEVEL               6
#define WATER_VOLT_PARA                     2

/* struct define area */
struct check_vol {
	unsigned int vol_min;
	unsigned int vol_max;
};

struct scharger_check_voltage {
	struct check_vol dm_vol_data[WATER_VOLT_PARA_LEVEL];
	struct check_vol dp_vol_data[WATER_VOLT_PARA_LEVEL];
};

struct param {
	int bat_comp;
	int vclamp;
	int fcp_support;
	int dpm_en;
	int scp_support;
	struct scharger_check_voltage scharger_check_vol;
};
struct hi6523_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct notifier_block   usb_nb;
	struct delayed_work plugout_check_work;
	struct work_struct irq_work;
	struct param param_dts;
	struct mutex fcp_detect_lock;
	struct mutex adc_conv_lock;
	struct mutex accp_adapter_reg_lock;
	struct mutex ibias_calc_lock;
	enum hisi_charger_type charger_type;
	int gpio_cd;
	int gpio_int;
	int irq_int;
	int term_vol_mv;
	int boost_5v_use_common_gpio;
	int bst_ctrl_use_common_gpio;
	int scp_power_en;
	int is_need_bst_ctrl;
	int bst_ctrl;
	int scp_need_extra_power;
	unsigned int adaptor_support;
	u8 sysfs_fcp_reg_addr;
	u8 sysfs_fcp_reg_val;
};
struct charge_cv_vdpm_data {
	int cv;
	int vdpm;
	int vdpm_set_val;
};

/* HI6523 Registers' Definitions */
#define CHIP_VERSION_0                  (SOC_SCHARGER_CHIP_VERSION_0_ADDR(0))
#define CHIP_VERSION_1                  (SOC_SCHARGER_CHIP_VERSION_1_ADDR(0))
#define CHIP_VERSION_2                  (SOC_SCHARGER_CHIP_VERSION_2_ADDR(0))
#define CHIP_VERSION_3                  (SOC_SCHARGER_CHIP_VERSION_3_ADDR(0))
#define CHIP_VERSION_4                  (SOC_SCHARGER_CHIP_VERSION_4_ADDR(0))
#define CHIP_VERSION_5                  (SOC_SCHARGER_CHIP_VERSION_5_ADDR(0))
#define CHIP_ID0                        0x03
#define CHIP_ID1                        0x01
#define CHIP_ID2                        0x02
#define CHG_STATUS0_REG                 (SOC_SCHARGER_STATUS0_ADDR(0))
#define CHG_BUCK_OK                     (1 << SOC_SCHARGER_STATUS0_buck_ok_START)
#define CHG_VERSION_REG5                (SOC_SCHARGER_CHIP_VERSION_5_ADDR(0))
#define WEAKSOURCE_FLAG_REG             PMIC_HRST_REG12_ADDR(0)

/* BUCK_REG0 tuning register */
/*
 * buck the current set in the average limit current ring.
 * 0000: 100mA; 00001: 150mA; 00010: 900mA; 00100: 1A; 00101: 1.2A; 00110: 1.3A;
 * 00111: 1.4A; 01000: 1.5A; 01001: 1.6A; 01010: 1.7A; 01011: 1.8A; 01100: 1.9A;
 * 01101: 2A; 01110: 2.1A; 01111: 2.2A; 10000: 2.4A; 10001: 2.6A; 10010: 2.8A;
 * 10011: 3A; 10100: 3.2A; 10101~11111: 3.2A.
 */
#define CHG_INPUT_SOURCE_REG            (SOC_SCHARGER_BUCK_REG0_ADDR(0))
#define CHG_ILIMIT_SHIFT                (SOC_SCHARGER_BUCK_REG0_buck_ilimit_START)
#define CHG_ILIMIT_MSK                  (0x1f << CHG_ILIMIT_SHIFT)
#define CHG_FCP_SOFT_RST_REG            (SOC_SCHARGER_FCP_SOFT_RST_CTRL_ADDR(0))

/* CHG_REG1 tuning register */
#define CHG_FAST_CURRENT_REG            (SOC_SCHARGER_CHG_REG1_ADDR(0))
#define CHG_FAST_ICHG_SHIFT             (SOC_SCHARGER_CHG_REG1_chg_fast_ichg_START)
#define CHG_FAST_ICHG_MSK               (0x1f << CHG_FAST_ICHG_SHIFT)

/* DET_TOP_REG0 tuning register */
#define CHG_VBUS_VSET_REG               (SOC_SCHARGER_DET_TOP_REG0_ADDR(0))
/* gear config for adapter, 0: 5V; 01: 9V; 10&11: 12V */
#define VBUS_VSET_SHIFT                 (SOC_SCHARGER_DET_TOP_REG0_vbus_vset_START)
#define VBUS_VSET_MSK                   (0x03 << VBUS_VSET_SHIFT)
#define VBUS_VSET_5V                    5
#define VBUS_VSET_9V                    9
#define VBUS_VSET_12V                   12

/* BUCK_REG2 tuning register */
#define CHG_DPM_SEL_REG                 (SOC_SCHARGER_BUCK_REG2_ADDR(0))
/* DPM input voltage set: 000: 82.5%; 001: 85%; 010:87.5%; 011: 90%; 100: 92.5%; 101: 95% */
#define CHG_DPM_SEL_SHIFT               (SOC_SCHARGER_BUCK_REG2_buck_dpm_sel_START)
#define CHG_DPM_SEL_MSK                 (0x07 << CHG_DPM_SEL_SHIFT)
#define CHG_DPM_SEL_82_5                0
#define CHG_DPM_SEL_85_0                1
#define CHG_DPM_SEL_87_5                2
#define CHG_DPM_SEL_90_0                3
#define CHG_DPM_SEL_92_5                4
#define CHG_DPM_SEL_95_0                5
#define CHG_DPM_SEL_BASE                825
#define CHG_DPM_SEL_STEP                25
#define VINDPM_4520                     4520
#define VINDPM_4700                     4700
/* CHG_REG0 tuning register */
/* charger anable signal, 0: disable; 1：enable */
#define CHG_ENABLE_REG                  (SOC_SCHARGER_CHG_REG0_ADDR(0))
#define CHG_EN_SHIFT                    (SOC_SCHARGER_CHG_REG0_chg_en_START)
#define CHG_EN_MSK                      (1 <<  CHG_EN_SHIFT)

#define BATFET_CTRL_CFG_REG             (SOC_SCHARGER_CHG_REG0_ADDR(0))
/*
 * batfet_ctrl config register: this register is controlled not only by the
 * pwr_rst_n reset signal, but also by dc_plug_n signal delivered by analogal
 * register: 0: batfet off; 1: batfet on
 */
#define BATFET_CTRL_CFG_SHIFT           (SOC_SCHARGER_CHG_REG0_batfet_ctrl_cfg_START)
#define BATFET_CTRL_CFG_MSK             (1 << BATFET_CTRL_CFG_SHIFT)
#define CHG_BATFET_DIS                  0
#define CHG_BATFET_EN                   1

#define CHG_PRE_ICHG_REG                (SOC_SCHARGER_CHG_REG0_ADDR(0))
/* precharge current gear tuning : 00: 100mA; 01: 200mA; 10 300mA; 11: 400mA */
#define CHG_PRE_ICHG_SHIFT              (SOC_SCHARGER_CHG_REG0_chg_pre_ichg_START)
#define CHG_PRE_ICHG_MSK                (0x03 << CHG_PRE_ICHG_SHIFT)
#define CHG_PRG_ICHG_STEP               100
#define CHG_PRG_ICHG_MIN                100
#define CHG_PRG_ICHG_100MA              100
#define CHG_PRG_ICHG_200MA              200
#define CHG_PRG_ICHG_300MA              300
#define CHG_PRG_ICHG_400MA              400
#define CHG_PRG_ICHG_MAX                400

#define CHG_PRE_VCHG_REG                (SOC_SCHARGER_CHG_REG0_ADDR(0))
/* precharge voltage threshold tuning: 00: 2.8V; 01: 3.0V; 10: 3.1V; 11: 3.2V */
#define CHG_PRE_VCHG_SHIFT              (SOC_SCHARGER_CHG_REG0_chg_pre_vchg_START)
#define CHG_PRE_VCHG_MSK                (0x03 << CHG_PRE_VCHG_SHIFT)
#define CHG_PRG_VCHG_2800               2800
#define CHG_PRG_VCHG_3000               3000
#define CHG_PRG_VCHG_3100               3100
#define CHG_PRG_VCHG_3200               3200

/* CHG_REG3 tuning register */
/* fastcharge satefy timer(hour): 00: 5h; 01: 8h; 10: 12h; 11: 20h */
#define CHG_FASTCHG_TIMER_REG           (SOC_SCHARGER_CHG_REG3_ADDR(0))
#define CHG_FASTCHG_TIMER_SHIFT         (SOC_SCHARGER_CHG_REG3_chg_fastchg_timer_START)
#define CHG_FASTCHG_TIMER_MSK           (0x03 << CHG_FASTCHG_TIMER_SHIFT)
#define CHG_FASTCHG_TIMER_5H            0
#define CHG_FASTCHG_TIMER_8H            1
#define CHG_FASTCHG_TIMER_12H           2
#define CHG_FASTCHG_TIMER_20H           3

/* CHG_REG4 tuning register */
#define CHG_EN_TERM_REG                 (SOC_SCHARGER_CHG_REG4_ADDR(0))
#define CHG_EN_TERM_SHIFT               (SOC_SCHARGER_CHG_REG4_chg_en_term_START)
#define CHG_EN_TERM_MSK                 (0x01 << CHG_EN_TERM_SHIFT)
#define CHG_TERM_EN                     1
#define CHG_TERM_DIS                    0

/* CHG_REG2 tuning register */
/*
 * fast charge completion voltage threshold tuning bits: 0000: 3.8V;
 * 0001: 3.85V; 0010: 3.9V; 0011: 3.95V; 0100: 4V; 0101: 4.05V; 0110: 4.1V;
 * 0111: 4.15V; 1000: 4.2V; 1001: 4.25V; 1010: 4.3V; 1011: 4.35V; 1100: 4.4V;
 * 1101: 4.45V; 1110: 4.5V; 1111:4.5V.
 */
#define CHG_FAST_VCHG_REG               (SOC_SCHARGER_CHG_REG2_ADDR(0))
#define CHG_FAST_VCHG_SHIFT             (SOC_SCHARGER_CHG_REG2_chg_fast_vchg_START)
#define CHG_FAST_VCHG_MSK               (0x0f << CHG_FAST_VCHG_SHIFT)

/*
 * fast charge completion current threshold tuning bits: 000: 50mA;
 * 0001: 100mA; 010: 150mA; 011: 200mA; 100: 300mA; 101: 400mA; 110: 500mA; 111: 600mA.
 */
#define CHG_TERM_ICHG_REG               (SOC_SCHARGER_CHG_REG2_ADDR(0))
#define CHG_TERM_ICHG_SHIFT             (SOC_SCHARGER_CHG_REG2_chg_term_ichg_START)
#define CHG_TERM_ICHG_MSK               (0x07 << CHG_TERM_ICHG_SHIFT)
#define CHG_TERM_ICHG_50MA              50
#define CHG_TERM_ICHG_100MA             100
#define CHG_TERM_ICHG_150MA             150
#define CHG_TERM_ICHG_200MA             200
#define CHG_TERM_ICHG_300MA             300
#define CHG_TERM_ICHG_400MA             400
#define CHG_TERM_ICHG_500MA             500
#define CHG_TERM_ICHG_600MA             600

#define WATCHDOG_CTRL_REG               (SOC_SCHARGER_WATCHDOG_CTRL_ADDR(0))
#define WATCHDOG_TIMER_SHIFT            (SOC_SCHARGER_WATCHDOG_CTRL_watchdog_timer_START)
#define WATCHDOG_TIMER_MSK              (0x03 << WATCHDOG_TIMER_SHIFT)
#define WATCHDOG_TIMER_10_S             10
#define WATCHDOG_TIMER_20_S             20
#define WATCHDOG_TIMER_40_S             40

#define WATCHDOG_SOFT_RST_REG           (SOC_SCHARGER_WATCHDOG_SOFT_RST_ADDR(0))
#define WD_RST_N_SHIFT                  (SOC_SCHARGER_WATCHDOG_SOFT_RST_wd_rst_n_START)
#define WD_RST_N_MSK                    (1 << WD_RST_N_SHIFT)
#define WATCHDOG_TIMER_RST              (1 << WD_RST_N_SHIFT)

#define CHG_OTG_REG0                    (SOC_SCHARGER_OTG_REG0_ADDR(0))
#define CHG_OTG_LIM_SHIFT               (SOC_SCHARGER_OTG_REG0_otg_lim_set_START)
#define CHG_OTG_LIM_MSK                 (0x03 << CHG_OTG_LIM_SHIFT)
#define BOOST_LIM_MIN                   500
#define BOOST_LIM_500                   500
#define BOOST_LIM_1000                  1000
#define BOOST_LIM_1500                  1500
#define BOOST_LIM_2000                  2000
#define BOOST_LIM_MAX                   2000
#define CHG_OTG_EN_SHIFT                (SOC_SCHARGER_OTG_REG0_otg_en_int_START)
#define CHG_OTG_EN_MSK                  (0x1 << CHG_OTG_EN_SHIFT)
#define APPLE_DETECT_ENABLE             1
#define APPLE_DETECT_DISABLE            0
#define APPLE_DETECT_SHIFT              2
#define APPLE_DETECT_MASK               (1 << 2)

#define CHG_STATUS0_REG                 (SOC_SCHARGER_STATUS0_ADDR(0))
#define HI6523_CHG_BUCK_OK              (0x01 << SOC_SCHARGER_STATUS0_buck_ok_START)
#define HI6523_CHG_STAT_CHARGE_DONE     (0x03 << SOC_SCHARGER_STATUS0_chg_chgstate_START)
#define CHG_TERM_ABLE_STATE		(CHG_IN_ACL_STATE | CHG_IN_THERM_STATE)
#define CHG_IN_DPM_STATE                (0x01 << SOC_SCHARGER_STATUS0_chg_dpm_state_START)
#define HI6523_CHG_OTG_ON               (0x01 << SOC_SCHARGER_STATUS0_otg_on_START)
#define CHG_IN_ACL_STATE                (0x01 << SOC_SCHARGER_STATUS0_chg_acl_state_START)
#define CHG_IN_THERM_STATE              (0x01 << SOC_SCHARGER_STATUS0_chg_therm_state_START)
#define HI6523_CHG_STAT_SHIFT           (SOC_SCHARGER_STATUS0_chg_chgstate_START)
#define HI6523_CHG_STAT_MASK            (0x03 << HI6523_CHG_STAT_SHIFT)
#define CHG_STAT_ENABLE                 1
#define CHG_STAT_DISABLE                0

#define CHG_STATUS1_REG                 (SOC_SCHARGER_STATUS1_ADDR(0))
#define HI6523_WATCHDOG_OK              (0x01 << SOC_SCHARGER_STATUS1_wdt_time_out_n_START)

#define CHG_ADC_CTRL_REG                (SOC_SCHARGER_ADC_CTRL_ADDR(0))
#define CHG_ADC_EN_SHIFT                (SOC_SCHARGER_ADC_CTRL_hkadc_en_START)
#define CHG_ADC_EN_MSK                  (1 << CHG_ADC_EN_SHIFT)
#define CHG_ADC_RST_SHIFT               (SOC_SCHARGER_ADC_CTRL_hkadc_reset_START)
#define CHG_ADC_RST_MSK                 (1 << CHG_ADC_RST_SHIFT)
#define CHG_ADC_EN                      1
#define CHG_ADC_DIS                     0

#define CHG_ADC_VBUS_RDY_REG            (SOC_SCHARGER_BUCK_RESERVE1_STATE_ADDR(0))
#define CHG_ADC_VBUS_RDY_SHIFT          7
#define CHG_ADC_VBUS_RDY_MSK            (1 << CHG_ADC_VBUS_RDY_SHIFT)
#define CHG_ADC_VBUS_RDY                1

#define CHG_ADC_CH_SHIFT                (SOC_SCHARGER_ADC_CTRL_hkadc_ch_sel_START)
#define CHG_ADC_CH_MSK                  (0x03 << CHG_ADC_CH_SHIFT)
#define CHG_ADC_CH_DET                  0x03
#define CHG_ADC_CH_VBUS                 0x02
#define CHG_ADC_CH_IBUS_REF             0x01
#define CHG_ADC_CH_IBUS                 0
#define CHG_ADC_BUFF_LEN                10

#define CHG_ADC_START_REG               (SOC_SCHARGER_ADC_START_ADDR(0))
#define CHG_ADC_START_SHIFT             (SOC_SCHARGER_ADC_START_hkadc_start_START)
#define CHG_ADC_START_MSK               (1 << CHG_ADC_START_SHIFT)

#define CHG_ADC_CONV_STATUS_REG         (SOC_SCHARGER_ADC_CONV_STATUS_ADDR(0))
#define CHG_ADC_CONV_STATUS_SHIFT       (SOC_SCHARGER_ADC_CONV_STATUS_hkadc_valid_START)
#define CHG_ADC_CONV_STATUS_MSK         (1 << CHG_ADC_CONV_STATUS_SHIFT)

#define CHG_ADC_APPDET_REG              (SOC_SCHARGER_SYS_RESVO2_ADDR(0))
#define CHG_ADC_APPDET_EN_SHIFT         2
#define CHG_ADC_APPDET_EN_MSK           (1 << CHG_ADC_APPDET_EN_SHIFT)
#define CHG_ADC_APPDET_EN               1
#define CHG_ADC_APPDET_DIS              0
#define CHG_ADC_APPDET_CHSEL_SHIFT      3
#define CHG_ADC_APPDET_CHSEL_MSK        (1 << CHG_ADC_APPDET_CHSEL_SHIFT)
#define CHG_ADC_APPDET_DPLUS            1
#define CHG_ADC_APPDET_DMINUS           0

#define CHG_ADC_DATA_REG                (SOC_SCHARGER_ADC_DATA1_ADDR(0))

#define CHG_HIZ_CTRL_REG                (SOC_SCHARGER_SCHG_LOGIC_CTRL_ADDR(0))
#define CHG_HIZ_ENABLE_SHIFT            (SOC_SCHARGER_SCHG_LOGIC_CTRL_buck_enb_START)
#define CHG_HIZ_ENABLE_MSK              (1 << CHG_HIZ_ENABLE_SHIFT)

#define CHG_IRQ_STATUS1                        (SOC_SCHARGER_IRQ_STATUS1_ADDR(0))

#define CHG_IRQ0                        (SOC_SCHARGER_IRQ0_ADDR(0))
#define CHG_OTG_OCP                     (1 << SOC_SCHARGER_IRQ0_otg_ocp_r_START)
#define CHG_OTG_SCP                     (1 << SOC_SCHARGER_IRQ0_otg_scp_r_START)
#define CHG_OTG_UVP                     (1 << SOC_SCHARGER_IRQ0_otg_uvp_r_START)
#define CHG_IRQ1                        (SOC_SCHARGER_IRQ1_ADDR(0))
#define CHG_VBUS_UVP                    (1 << SOC_SCHARGER_IRQ1_vbus_uvp_r_START)
#define CHG_VBAT_OVP                    (1 << SOC_SCHARGER_IRQ1_vbat_ovp_r_START)
#define CHG_IRQ2                        (SOC_SCHARGER_IRQ2_ADDR(0))
#define CHG_WDT_IRQ                     (SOC_SCHARGER_WATCHDOG_IRQ_ADDR(0))
#define CHG_RBOOST_IRQ_SHIFT            4
#define CHG_RBOOST_IRQ_MSK              (1 << CHG_RBOOST_IRQ_SHIFT)
#define CHG_RBOOST_IRQ                  1
#define CHG_IRQ0_MASK_ADDR              (SOC_SCHARGER_IRQ0_MASK_ADDR(0))
#define CHG_IRQ1_MASK_ADDR              (SOC_SCHARGER_IRQ1_MASK_ADDR(0))
#define CHG_IRQ2_MASK_ADDR              (SOC_SCHARGER_IRQ2_MASK_ADDR(0))
#define CHG_WDT_IRQ_MASK_ADDR           (SOC_SCHARGER_WATCHDOG_IRQ_MASK_ADDR(0))
#define CHG_FCP_ISR1_MASK_ADDR          (SOC_SCHARGER_IMR1_ADDR(0))
#define CHG_FCP_ISR2_MASK_ADDR          (SOC_SCHARGER_IMR2_ADDR(0))
#define CHG_FCP_IRQ5_MASK_ADDR          (SOC_SCHARGER_FCP_IRQ5_MASK_ADDR(0))
#define CHG_IRQ0_MASK_DEFAULT           0x48
#define CHG_IRQ1_MASK_DEFAULT           0x0
#define CHG_IRQ2_MASK_DEFAULT           0x30
#define CHG_WDT_IRQ_DEFAULT             0x0
#define CHG_IRQ_MASK_ALL                0xFF

#define CHG_IR_COMP_REG                 (SOC_SCHARGER_CHG_REG5_ADDR(0))
#define CHG_IR_COMP_SHIFT               (SOC_SCHARGER_CHG_REG5_chg_ir_set_START)
#define CHG_IR_COMP_MSK                 (0x07 << CHG_IR_COMP_SHIFT)
#define CHG_IR_COMP_MIN                 0
#define CHG_IR_COMP_0MOHM               0
#define CHG_IR_COMP_15MOHM              15
#define CHG_IR_COMP_30MOHM              30
#define CHG_IR_COMP_45MOHM              45
#define CHG_IR_COMP_60MOHM              60
#define CHG_IR_COMP_75MOHM              75
#define CHG_IR_COMP_95MOHM              95
#define CHG_IR_COMP_110MOHM             110
#define CHG_IR_COMP_MAX                 110
#define CHG_IR_COMP_NUM                 8
#define CHG_LMT_GEAR_NUM                28

#define CHG_IR_VCLAMP_REG               (SOC_SCHARGER_CHG_REG5_ADDR(0))
#define CHG_IR_VCLAMP_SHIFT             (SOC_SCHARGER_CHG_REG5_chg_vclamp_set_START)
#define CHG_IR_VCLAMP_MSK               (0x07 << CHG_IR_VCLAMP_SHIFT)
#define CHG_IR_VCLAMP_STEP              50
#define CHG_IR_VCLAMP_MIN               0
#define CHG_IR_VCLAMP_MAX               350

#define CHG_UVP_OVP_VOLTAGE_REG         (SOC_SCHARGER_SYS_RESVO1_ADDR(0))
#define CHG_UVP_OVP_VOLTAGE_MSK         0xf
#define CHG_UVP_OVP_VOLTAGE_SHIFT       1
#define CHG_UVP_OVP_VOLTAGE_5V          0
#define CHG_UVP_OVP_VOLTAGE_9V          0x05
#define CHG_UVP_OVP_VOLTAGE_12V         0x0a
#define CHG_UVP_OVP_VOLTAGE_MAX         0x02

/* fcp detect */
#define CHG_FCP_ADAPTER_DETECT_FAIL     1
#define CHG_FCP_ADAPTER_DETECT_SUCC     0
#define CHG_FCP_ADAPTER_DETECT_OTHER    (-1)
#define CHG_FCP_POLL_TIME               100 /* 100ms */
#define CHG_FCP_DETECT_MAX_COUT         20 /* fcp detect MAX COUT */

/* fcp adapter status */
#define FCP_ADAPTER_5V                  5000
#define FCP_ADAPTER_9V                  9000
#define FCP_ADAPTER_12V                 12000
#define FCP_ADAPTER_MAX_VOL             (FCP_ADAPTER_12V)
#define FCP_ADAPTER_MIN_VOL             (FCP_ADAPTER_5V)
#define FCP_ADAPTER_RST_VOL             (FCP_ADAPTER_5V)
#define FCP_ADAPTER_VOL_CHECK_ERROR     500
#define FCP_ADAPTER_VOL_CHECK_POLLTIME  100
#define FCP_ADAPTER_VOL_CHECK_TIMEOUT   10

#define CHG_FCP_CMD_REG                 (SOC_SCHARGER_CMD_ADDR(0))
#define CHG_FCP_ADDR_REG                (SOC_SCHARGER_ADDR_ADDR(0))
#define CHG_FCP_WDATA_REG               (SOC_SCHARGER_DATA0_ADDR(0))
#define CHG_FCP_RDATA_REG               (SOC_SCHARGER_FCP_RDATA_ADDR(0))

#define CHG_FCP_CTRL_REG                (SOC_SCHARGER_CNTL_ADDR(0))
#define CHG_FCP_EN_SHIFT                (SOC_SCHARGER_CNTL_enable_START)
#define CHG_FCP_EN_MSK                  (1 << CHG_FCP_EN_SHIFT)
#define CHG_FCP_MSTR_RST_SHIFT          (SOC_SCHARGER_CNTL_mstr_rst_START)
#define CHG_FCP_MSTR_RST_MSK            (1 << CHG_FCP_MSTR_RST_SHIFT)
#define CHG_FCP_SNDCMD_SHIFT            (SOC_SCHARGER_CNTL_sndcmd_START)
#define CHG_FCP_SNDCMD_MSK              (1 << CHG_FCP_SNDCMD_SHIFT)
#define CHG_FCP_EN                      1
#define CHG_FCP_DIS                     0

#define CHG_FCP_DET_CTRL_REG            (SOC_SCHARGER_FCP_CTRL_ADDR(0))
#define CHG_FCP_DET_EN_SHIFT            (SOC_SCHARGER_FCP_CTRL_fcp_det_en_START)
#define CHG_FCP_DET_EN_MSK              (1 << CHG_FCP_DET_EN_SHIFT)
#define CHG_FCP_CMP_EN_SHIFT            (SOC_SCHARGER_FCP_CTRL_fcp_cmp_en_START)
#define CHG_FCP_CMP_EN_MSK              (1 << CHG_FCP_CMP_EN_SHIFT)
#define CHG_FCP_DET_EN                  1

#define CHG_FCP_STATUS_REG              (SOC_SCHARGER_STATUIS_ADDR(0))
#define CHG_FCP_DVC_SHIFT               (SOC_SCHARGER_STATUIS_dvc_START)
#define CHG_FCP_DVC_MSK                 (0x03 << CHG_FCP_DVC_SHIFT)
#define CHG_FCP_ATTATCH_SHIFT           (SOC_SCHARGER_STATUIS_attach_START)
#define CHG_FCP_ATTATCH_MSK             (1 << CHG_FCP_ATTATCH_SHIFT)
#define CHG_FCP_SLAVE_GOOD              (CHG_FCP_DVC_MSK | CHG_FCP_ATTATCH_MSK)

#define CHG_FCP_SET_STATUS_REG          (SOC_SCHARGER_FCP_ADAP_CTRL_ADDR(0))
#define CHG_FCP_SET_STATUS_SHIFT        (SOC_SCHARGER_FCP_ADAP_CTRL_fcp_set_d60m_r_START)
#define CHG_FCP_SET_STATUS_MSK          (1 << CHG_FCP_SET_STATUS_SHIFT)

#define CHG_FCP_ISR1_REG                (SOC_SCHARGER_ISR1_ADDR(0))
#define CHG_FCP_CMDCPL                  (1 << SOC_SCHARGER_ISR1_cmdcpl_START)
#define CHG_FCP_ACK                     (1 << SOC_SCHARGER_ISR1_ack_START)
#define CHG_FCP_NACK                    (1 << SOC_SCHARGER_ISR1_nack_START)
#define CHG_FCP_CRCPAR                  (1 << SOC_SCHARGER_ISR1_crcpar_START)

#define CHG_FCP_ISR2_REG                (SOC_SCHARGER_ISR2_ADDR(0))
#define CHG_FCP_CRCRX                   (1 << SOC_SCHARGER_ISR2_crcrx_START)
#define CHG_FCP_PARRX                   (1 << SOC_SCHARGER_ISR2_parrx_START)
#define CHG_FCP_PROTSTAT                (1 << SOC_SCHARGER_ISR2_protstat_START)

#define CHG_FCP_IRQ3_REG                (SOC_SCHARGER_FCP_IRQ3_ADDR(0))
#define CHG_FCP_TAIL_HAND_FAIL          (1 << SOC_SCHARGER_FCP_IRQ3_tail_hand_fail_irq_START)
#define CHG_FCP_INIT_HAND_FAIL          (1 << SOC_SCHARGER_FCP_IRQ3_init_hand_fail_irq_START)

#define CHG_FCP_IRQ4_REG                (SOC_SCHARGER_FCP_IRQ4_ADDR(0))
#define CHG_FCP_ENABLE_HAND_FAIL        (1 << SOC_SCHARGER_FCP_IRQ4_enable_hand_fail_irq_START)

#define CHG_FCP_IRQ5_REG                (SOC_SCHARGER_FCP_IRQ5_ADDR(0))
#define CHG_FCP_SET_DET_SHIFT           (SOC_SCHARGER_FCP_IRQ5_fcp_set_d60m_r_START)
#define CHG_FCP_SET_DET_MSK             (1 << SOC_SCHARGER_FCP_IRQ5_fcp_set_d60m_r_START)

/* adapter cmd */
#define CHG_FCP_CMD_SBRRD                   0x0c
#define CHG_FCP_CMD_SBRWR                   0x0b

/* adapter registers */
#define CHG_FCP_SLAVE_ID_OUT0               0x04
#define CHG_FCP_SLAVE_DISCRETE_CAPABILITIES 0x21
#define CHG_FCP_SLAVE_MAX_PWR               0x22
#define CHG_FCP_SLAVE_OUTPUT_CONTROL        0x2b
#define CHG_FCP_SLAVE_VOUT_CONFIG           0x2c
#define CHG_FCP_SLAVE_REG_DISCRETE_OUT_V(n) (0x30 + (n))
/* Register FCP_SLAVE_OUTPUT_CONTROL (0x2b) */
#define CHG_FCP_SLAVE_SET_VOUT              (1 << 0)
#define CHG_FCP_VOL_STEP                    10
#define CHG_FCP_OUTPUT_VOL_5V               5
#define CHG_FCP_OUTPUT_VOL_9V               9
#define CHG_FCP_OUTPUT_VOL_12V              12

/* fcp_adapter_transfer_status */
#define FCP_TRANSFER_SUCC                   0
#define FCP_TRANSFER_FAIL                   (-1)

/* anti-reverbst */
#define CHG_ANTI_REVERBST_REG               (SOC_SCHARGER_BUCK_RESERVE2_ADDR(0))
#define CHG_ANTI_REVERBST_EN_SHIFT          7
#define CHG_ANTI_REVERBST_EN_MSK            (1 << CHG_ANTI_REVERBST_EN_SHIFT)
#define CHG_ANTI_REVERBST_EN                1
#define CHG_ANTI_REVERBST_DIS               0
#define CHG_DPM_MIN_VOLTAGE                 4500
#define CHG_DPM_MAX_VOLTAGE                 4625

#define CHG_DPM_VOL_4000_MV                 4000
#define CHG_DPM_VOL_4835_MV                 4835
#define CHG_DPM_VOL_4855_MV                 4855
#define CHG_BAT_VOL_3850_MV                 3850
#define CHG_BAT_VOL_3750_MV                 3750
#define CHG_BUCK_REG3_REG                   (SOC_SCHARGER_BUCK_REG3_ADDR(0))
#define CHG_BUCK_GAP_90MV_REG               0x07
#define CHG_BUCK_GAP_0MV_REG                0x00

/* 5V DPM modify registrs */
#define CHG_OTG_RESERVE2                    (SOC_SCHARGER_OTG_RESERVE2_ADDR(0))
#define CHG_5V_DPM_SHIFT                    1
#define CHG_5V_DPM_SHIFT_MSK                (0x0f << CHG_5V_DPM_SHIFT)
#define DPM_VAL_4460                        3
#define DPM_VAL_4675                        4

/* set current reg value */
#define CURRENT_STEP_0                      0
#define CURRENT_STEP_9                      9
#define CURRENT_STEP_14                     14
#define CURRENT_STEP_24                     24
#define CURRENT_STEP_30                     30
#define CURRENT_STEP_31                     31

/* for single-phase synchronous BUCK */
#define CHG_BUCK_REG5_REG                   (SOC_SCHARGER_BUCK_REG5_ADDR(0))

#define CHG_BUCK_OCP_HALVED_SHIFT           5
#define CHG_BUCK_OCP_HALVED_MASK            (1 << CHG_BUCK_OCP_HALVED_SHIFT)

int hi6523_get_vbus_mv(unsigned int *vbus_mv);
int hi6523_apple_adapter_detect(int enable);
int hi6523_set_vbus_vset(u32 value);
int hi6523_config_opt_param(int vbus_vol);
#endif
