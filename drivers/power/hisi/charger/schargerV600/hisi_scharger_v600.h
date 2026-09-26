/*
 * hisi_scharger_v600.h
 *
 * HI6526 charger driver header
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#ifndef __HI6526_CHARGER_H__
#define __HI6526_CHARGER_H__
#include "hisi_scharger_v600_regs.h"
#include <linux/i2c.h>       /* for struct device_info */
#include <linux/device.h>    /* for struct device_info */
#include <linux/workqueue.h> /* for struct evice_info */
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/rtmutex.h>
#include <linux/pm_wakeup.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/huawei_charger.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/hisi/usb/hisi_tcpc_ops.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/i2c.h>
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
#include <linux/power/hisi/charger/hisi_charger_fcp.h>
#include <linux/power/hisi/charger/hisi_charger_scp.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/bci_battery.h>
#include <linux/power_supply.h>
#include <linux/raid/pq.h>
#include <linux/slab.h>
#include <linux/usb/otg.h>
#include <pmic_interface.h>
#include <securec.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/usb/switch/switch_ap/switch_chip.h>
#include <soc_schargerV600_interface.h>

struct hi6526_device_info *get_hi6526_device(void);

#define HI6526_MODULE_NAME              hi6526
#define HI6526_MODULE_NAME_STR          "hi6526"
#define HI6526_I2C_SLAVE_ADDRESS        0x6B
#define CHG_ENABLE                      1
#define CHG_DISABLE                     0
#define ENABLE_DBG_CHECK                1
#define DISABLE_DBG_CHECK               0

#define WEAKSOURCE_FLAG_REG             PMIC_HRST_REG12_ADDR(0)
#define WAEKSOURCE_FLAG                 BIT(2)
#define HI6526_DBG_VAL_SIZE             9

#define HI6526_REG_MAX                  (SOC_SCHARGER_OTG_RO_REG_9_ADDR(0) + 1)

#define RET_SIZE_21                     21
#define REGISTER_RET_SIZE               8
#define LEN_OF_TRUNCATE                 26
#define CHG_NONE_REG                    0x00
#define CHG_NONE_MSK                    0xFF
#define CHG_NONE_SHIFT                  0x00

#define CHIP_VERSION_0                  (SOC_SCHARGER_VERSION0_RO_REG_0_ADDR(0))
#define CHIP_VERSION_2                  (SOC_SCHARGER_VERSION2_RO_REG_0_ADDR(0))
#define CHIP_VERSION_4                  (SOC_SCHARGER_VERSION4_RO_REG_0_ADDR(0))
#define CHG_INPUT_SOURCE_REG            (SOC_SCHARGER_BUCK_CFG_REG_0_ADDR(0))
#define CHG_ILIMIT_SHIFT                (SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_ilimit_START)

struct reg_page {
	u8 page_addr;
	u8 page_val;
};

#define CHG_VBATT_CV_103(x)             ((x) * 103 / 100)
#define scharger_err(fmt, args...) pr_err("[hisi_scharger]" fmt, ## args)
#define scharger_evt(fmt, args...) pr_warn("[hisi_scharger]" fmt, ## args)
#define scharger_inf(fmt, args...) pr_info("[hisi_scharger]" fmt, ## args)
#define scharger_dbg(fmt, args...) pr_debug("[hisi_scharger]" fmt, ## args)

#define FCP_ADAPTER_CNTL_REG            (SOC_SCHARGER_FCP_ADAP_CTRL_ADDR(0))
#define HI6526_ACCP_CHARGER_DET         (1 << SOC_SCHARGER_FCP_ADAP_CTRL_fcp_set_d60m_r_START)

/* struct define area */
struct dc_regulator_info {
	int ibus;
	int ibat;
	int vout;
	int vbat;
};

struct param {
	int bat_comp;
	int vclamp;
	int fcp_support;
	int scp_support;
	int r_coul_mohm;
	int dp_th_res;
	u32 dbg_check_en;
	int lvc_vusb2vbus_drop_en;
	int sc_vusb2vbus_drop_en;
	int mask_pmic_irq_support;
	int adc_single_mode_en;
	int adc_tsbat_support;
	int batt_temp_compensation_en;
	int batt_temp_compensation_r; /* momh */
	int vusb_uv_flag;
	struct dc_regulator_info lvc_regulator;
	struct dc_regulator_info sc_regulator;
};

enum chg_mode_state {
	NONE = 0,
	BUCK,
	LVC,
	SC,
	OTG_MODE,
	SOH_MODE
};

struct chip_debug_info {
	u64 ts_nsec; /* timestamp in nanoseconds */
	int vusb;
	int vbus;
	int ibus;
	int ibat;
	int vout;
	int vbat;
	int ibus_ref;
};

#define INFO_LEN         10
#define DBG_WORK_TIME    40
#define WAIT_PD_IRQ_TIME 10

struct hi6526_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work reverbst_work;
	struct delayed_work dc_ucp_work;
	struct param param_dts;
	unsigned int charger_type;
	struct notifier_block   usb_nb;
	enum chg_mode_state chg_mode;
	struct mutex fcp_detect_lock;
	struct rt_mutex i2c_lock;
	struct mutex otg_lock;
	struct mutex adc_conv_lock;
	struct mutex accp_adapter_reg_lock;
	struct mutex ibias_calc_lock;
	struct nty_data dc_nty_data;
	struct delayed_work dbg_work;
	struct wakeup_source hi6526_wake_lock;
	struct chip_debug_info dbg_info[INFO_LEN];
	unsigned int hi6526_version;
	unsigned int is_board_type; /* 0:sft 1:udp 2:asic */
	int buck_vbus_set;
	int input_current;
	int input_limit_flag;
	int gpio_cd;
	int gpio_int;
	int irq_int;
	int term_vol_mv;
	unsigned int adaptor_support;
	int ucp_work_first_run;
	int dc_ibus_ucp_happened;
	unsigned int dbg_index;
	unsigned long reverbst_begin;
	int delay_cunt;
	int reverbst_cnt;
	u8 batt_ovp_cnt_30s;
	u8 sysfs_fcp_reg_addr;
	u8 sysfs_fcp_reg_val;
	u8 dbg_work_stop;
	u8 abnormal_happened;
	int chip_temp;
	u8 i2c_reg_page;
	int reset_n_efuse_dis;
};

#define CHG_IRQ_ADDR                        (SOC_SCHARGER_IRQ_FLAG_ADDR(0))
#define CHG_FCP_IRQ                         (BIT(0))
#define CHG_OTHERS_IRQ                      (BIT(1))
#define CHG_PD_IRQ                          (BIT(2))
#define CHG_LVC_SC_IRQ                      (BIT(3))
#define CHG_BUCK_IRQ                        (BIT(4))

/* buck irq */
#define CHG_BUCK_IRQ_ADDR                   (SOC_SCHARGER_IRQ_FLAG_0_ADDR(0))
#define BUCK_IRQ_MASK                       0xE0FFFF
#define CHG_BUCK_IRQ_ADDR2                  (SOC_SCHARGER_IRQ_FLAG_2_ADDR(0))
#define CHG_RBOOST_IRQ_SHIFT                7
#define CHG_RBOOST_IRQ_MSK                  (1 << CHG_RBOOST_IRQ_SHIFT)

/* lcv/scc irq */
#define CHG_LVC_SC_IRQ_ADDR                 (SOC_SCHARGER_IRQ_FLAG_2_ADDR(0))
#define LVC_SC_IRQ_MASK                     0xFFFF1F
#define LVC_REGULATOR_IRQ_MASK              0x000F00
#define IRQ_IBUS_DC_UCP_MASK                0x000001

/* pd irq */
#define CHG_PD_IRQ_ADDR                     (SOC_SCHARGER_IRQ_FLAG_5_ADDR(0))
#define PD_IRQ_MASK                         0xE0

/* others irq */
#define CHG_OTHERS_IRQ_ADDR                 (SOC_SCHARGER_IRQ_FLAG_5_ADDR(0))
#define OTHERS_IRQ_MASK                     0xFFFF1D
#define OTHERS_SOH_IRQ_MASK                 0x00000C

#define OTHERS_VDROP_IRQ_MASK               0x300000
#define OTHERS_OTP_IRQ_MASK                 0x000200
#define OTHERS_VBAT_LV_IRQ_MASK             0x000100

#define IRQ_VDROP_MIN_MASK                  0x200000
#define IRQ_VDROP_OVP_MASK                  0x100000

/* fcp irq */
#define CHG_FCP_IRQ_ADDR1                   (SOC_SCHARGER_ISR1_ADDR(0))
#define OTHERS_IRQ_MASK1                    0x1FFE
#define CHG_FCP_IRQ_ADDR2                   (SOC_SCHARGER_FCP_IRQ3_ADDR(0))
#define OTHERS_IRQ_MASK2                    0xFFFF
#define CHG_FCP_IRQ_ADDR3                   (SOC_SCHARGER_FCP_IRQ5_ADDR(0))
#define OTHERS_IRQ_MASK3                    0x03

#define CHG_FCP_IRQ5_MASK_ADDR              (SOC_SCHARGER_FCP_IRQ5_MASK_ADDR(0))
#define CHG_IRQ_MASK_ALL_ADDR               (SOC_SCHARGER_IRQ_MASK_ADDR(0))
#define CHG_IRQ_MASK_ALL_SHIFT              0
#define CHG_IRQ_MASK_ALL_MSK                (0x1B << CHG_IRQ_MASK_ALL_SHIFT)
#define CHG_IRQ_MASK_PD_MSK                 (1 << 2)

#define CHG_IRQ_UNMASK_DEFAULT              0x01

#define CHG_IRQ_MASK_0                      (SOC_SCHARGER_IRQ_MASK_0_ADDR(0))
#define CHG_IRQ_MASK_1                      (SOC_SCHARGER_IRQ_MASK_1_ADDR(0))
#define CHG_IRQ_MASK_2                      (SOC_SCHARGER_IRQ_MASK_2_ADDR(0))
#define CHG_IRQ_MASK_3                      (SOC_SCHARGER_IRQ_MASK_3_ADDR(0))
#define CHG_IRQ_MASK_4                      (SOC_SCHARGER_IRQ_MASK_4_ADDR(0))
#define CHG_IRQ_MASK_5                      (SOC_SCHARGER_IRQ_MASK_5_ADDR(0))
#define CHG_IRQ_MASK_6                      (SOC_SCHARGER_IRQ_MASK_6_ADDR(0))
#define CHG_IRQ_MASK_7                      (SOC_SCHARGER_IRQ_MASK_7_ADDR(0))

#define IRQ_SLEEP_MOD_MASK                  BIT(0)
#define IRQ_OTG_OCP_MASK                    BIT(7)

#define CHG_IRQ_MASK_0_VAL                  0x00
#define CHG_IRQ_MASK_1_VAL                  (IRQ_OTG_OCP_MASK | IRQ_SLEEP_MOD_MASK)

#define EFUSE_SEL_REG                       (SOC_SCHARGER_EFUSE_SEL_ADDR(0))
#define EFUSE_SEL_SHIFT                     (SOC_SCHARGER_EFUSE_SEL_sc_efuse_sel_START)
#define EFUSE_SEL_MASK                      (3 << EFUSE_SEL_SHIFT)

#define EFUSE1_SEL_REG                      (SOC_SCHARGER_EFUSE_TESTBUS_CFG_ADDR(0))
#define EFUSE1_SEL_SHIFT                                                       \
	(SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_START)
#define EFUSE1_SEL_MASK                     (0x0f << EFUSE1_SEL_SHIFT)

#define EFUSE2_SEL_REG                      (SOC_SCHARGER_EFUSE_TESTBUS_SEL_ADDR(0))
#define EFUSE2_SEL_SHIFT                                                       \
	(SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse2_testbus_sel_START)
#define EFUSE2_SEL_MASK                     (0x0f << EFUSE2_SEL_SHIFT)

#define EFUSE3_SEL_REG                      (SOC_SCHARGER_EFUSE_TESTBUS_SEL_ADDR(0))
#define EFUSE3_SEL_SHIFT                                                       \
	(SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse3_testbus_sel_START)
#define EFUSE3_SEL_MASK                     (0x0f << EFUSE3_SEL_SHIFT)

#define EFUSE4_SEL_REG                      (SOC_SCHARGER_EFUSE_TESTBUS_SEL_ADDR(0))
#define EFUSE4_SEL_SHIFT                                                       \
	(SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse3_testbus_sel_START)
#define EFUSE4_SEL_MASK                     (0x0f << EFUSE4_SEL_SHIFT)

#define EFUSE4_CFG                          SOC_SCHARGER_EFUSE_CFG_0_ADDR(0)
#define EFUSE4_CFG_RD_MODE_SHIFT                                               \
	(SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_rd_mode_sel_START)
#define EFUSE4_CFG_RD_MODE_SEL_MASK         (1 << EFUSE4_CFG_RD_MODE_SHIFT)
#define EFUSE4_CFG_RD_MODE_BIT_FLUSH        0
#define EFUSE4_CFG_RD_MODE_64BIT_FLUSH      1
#define EFUSE4_CFG_RD_CTRL_SHIFT                                               \
	(SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_rd_ctrl_START)
#define EFUSE4_CFG_RD_CTRL_SEL_MASK         (1 << EFUSE4_CFG_RD_CTRL_SHIFT)
#define EFUSE4_CFG_RD_CTRL_EN               1
#define EFUSE4_CFG_RD_CTRL_DIS              0

#define EFUSE4_3_4_TESTBUS_SEL_SHIFT                                           \
	(SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_END)
#define EFUSE4_3_4_TESTBUS_SEL_MASK         (1 << EFUSE4_3_4_TESTBUS_SEL_SHIFT)
#define EFUSE4_3_TESTBUS_SEL                0
#define EFUSE4_4_TESTBUS_SEL                1

#define EFUSE_EN_REG                        (SOC_SCHARGER_EFUSE_TESTBUS_CFG_ADDR(0))
#define EFUSE_EN_SHIFT                                                         \
	(SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse_testbus_en_START)
#define EFUSE_EN_MASK                       (1 << EFUSE_EN_SHIFT)
#define EFUSE_EN                            1
#define EFUSE_DIS                           0

#define EFUSE1_RDATA_REG                    (SOC_SCHARGER_EFUSE1_TESTBUS_RDATA_ADDR(0))
#define EFUSE2_RDATA_REG                    (SOC_SCHARGER_EFUSE2_TESTBUS_RDATA_ADDR(0))
#define EFUSE3_RDATA_REG                    (SOC_SCHARGER_EFUSE3_TESTBUS_RDATA_ADDR(0))
#define EFUSE4_RDATA_REG                    (SOC_SCHARGER_EFUSE3_TESTBUS_RDATA_ADDR(0))

#define EFUSE_PDOB_SEL_SHIFT                (SOC_SCHARGER_EFUSE_SEL_sc_efuse_pdob_sel_START)
#define EFUSE_PDOB_SEL_MASK                 (1 << EFUSE_PDOB_SEL_SHIFT)
#define EFUSE_PDOB_SEL_CALI_EN              1
#define EFUSE_PDOB_SEL_CALI_DIS             0

#define EFUSE_PDOB_PRE_ADDR_WE              (SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_ADDR(0))
#define EFUSE_PDOB_PRE_ADDR_SHIFT                                              \
	(SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_addr_START)
#define EFUSE_PDOB_PRE_ADDR_MASK            (0x7 << EFUSE_PDOB_PRE_ADDR_SHIFT)
#define EFUSE_PDOB_PRE_WE_SHIFT                                                \
	(SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_we_START)
#define EFUSE_PDOB_PRE_WE_MASK              (1 << EFUSE_PDOB_PRE_WE_SHIFT)
#define EFUSE_PDOB_PRE_WE_EN                1
#define EFUSE_PDOB_PRE_WE_DIS               0

#define EFUSE_PDOB_PRE_WDATA                (SOC_SCHARGER_EFUSE_PDOB_PRE_WDATA_ADDR(0))
#define EFUSE_PDOB_PRE_WDATA_SHIFT                                             \
	(SOC_SCHARGER_EFUSE_PDOB_PRE_WDATA_sc_efuse_pdob_pre_wdata_START)
#define EFUSE_PDOB_PRE_WDATA_MASK           (0xFF << EFUSE_PDOB_PRE_WDATA_SHIFT)

#define TCPC_CC_RESET_ADDR		(SOC_SCHARGER_TCPC_RO_REG_5_ADDR(0))
#define CC_RESET_MASK			0x01
#define CC_RESET_MASK_DIS		1

#define LDO33_EN_REG 	SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_ADDR(0)
#define LDO33_EN_SHITF 	SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_ldo33_en_START
#define LDO33_EN_MASK 	(1 << LDO33_EN_SHITF)

#define CV_ATE_TRIM_MASK                    0xF8
#define CV_ATE_TRIM_SHITF                   3

#define ATE_TRIM_LIST_0                     0x0F
#define ATE_TRIM_LIST_1                     0x0E
#define ATE_TRIM_LIST_2                     0x0D
#define ATE_TRIM_LIST_3                     0x0C
#define ATE_TRIM_LIST_4                     0x0B
#define ATE_TRIM_LIST_5                     0x0A
#define ATE_TRIM_LIST_6                     0x09
#define ATE_TRIM_LIST_7                     0x08
#define ATE_TRIM_LIST_8                     0x07
#define ATE_TRIM_LIST_9                     0x06
#define ATE_TRIM_LIST_10                    0x05
#define ATE_TRIM_LIST_11                    0x04
#define ATE_TRIM_LIST_12                    0x03
#define ATE_TRIM_LIST_13                    0x02
#define ATE_TRIM_LIST_14                    0x01
#define ATE_TRIM_LIST_15                    0x00
#define ATE_TRIM_LIST_16                    0x10
#define ATE_TRIM_LIST_17                    0x11
#define ATE_TRIM_LIST_18                    0x12
#define ATE_TRIM_LIST_19                    0x13
#define ATE_TRIM_LIST_20                    0x14
#define ATE_TRIM_LIST_21                    0x15
#define ATE_TRIM_LIST_22                    0x16
#define ATE_TRIM_LIST_23                    0x17
#define ATE_TRIM_LIST_24                    0x18
#define ATE_TRIM_LIST_25                    0x19
#define ATE_TRIM_LIST_26                    0x1A
#define ATE_TRIM_LIST_27                    0x1B
#define ATE_TRIM_LIST_28                    0x1C
#define ATE_TRIM_LIST_29                    0x1D
#define ATE_TRIM_LIST_30                    0x1E
#define ATE_TRIM_LIST_31                    0x1F

#define PAGE0_NUM                           (0x7F + 1)
#define PAGE1_NUM                           (0x75 + 1)
#define PAGE2_NUM                           (0x76 + 1)

#define REVERBST_DELAY_ON                   1000
#define VBUS_2600_MV                        2600

#define IBUS_OCP_START_VAL                  500
#define IBUS_ABNORMAL_VAL                   200
#define IBUS_ABNORMAL_CNT                   5
#define IBUS_ABNORMAL_TIME                  20

#define BUF_LEN                             80
#define DELAY_TIMES                         3

/* direct charger regulator register */
#define DC_REGULATOR_IBAT_REG               (SOC_SCHARGER_DC_IBAT_REGULATOR_ADDR(0))
#define DC_REGULATOR_IBAT_MIN               3000 /* 3000mA */
#define DC_REGULATOR_IBAT_MAX               13000 /* 13000mA */
#define DC_REGULATOR_IBAT_SETP              50   /* 50mA */

#define DC_REGULATOR_VBAT_REG               (SOC_SCHARGER_DC_VBAT_REGULATOR_ADDR(0))
#define DC_REGULATOR_VBAT_MIN               4200 /* 4.2v */
#define DC_REGULATOR_VBAT_MAX               5000 /* 5V */
#define DC_REGULATOR_VBAT_SETP              10   /* 10mV */

#define DC_REGULATOR_VOUT_REG               (SOC_SCHARGER_DC_VOUT_REGULATOR_ADDR(0))
#define DC_REGULATOR_VOUT_MIN               4200 /* 4.2v */
#define DC_REGULATOR_VOUT_MAX               5000 /* 5V */
#define DC_REGULATOR_VOUT_SETP              10   /* 10mV */

#define DC_REGULATOR_IBUS_REG               (SOC_SCHARGER_DC_TOP_CFG_REG_2_ADDR(0))
#define DC_REGULATOR_IBUS_MIN               800 /* 800mA */
#define DC_REGULATOR_IBUS_MAX               56000 /* 5600mA */
#define DC_REGULATOR_IBUS_SETP              100   /* 100mA */
#define DC_REGULATOR_IBUS_SHIFT             (SOC_SCHARGER_DC_TOP_CFG_REG_2_da_dc_ibus_regl_sel_START)
#define DC_REGULATOR_IBUS_MASK              (0x3F << DC_REGULATOR_IBUS_SHIFT)

/*
 * flag2 bit3/4  vbus ovp   LVC     DIRECT_CHARGE_FAULT_VBUS_OVP
 * flag2 bit2    vbus uv    LVC     none
 * flag2 bit1    ibus ocp   LVC/SC  DIRECT_CHARGE_FAULT_INPUT_OCP /
 *                                 DIRECT_CHARGE_FAULT_IBUS_OCP
 * flag2 bit0    ibus ucp   close

 * flag3 bit7    ibus rcp   LVC/SC  DIRECT_CHARGE_FAULT_REVERSE_OCP /
 * flag3 bit6    vdrop ov   LVC/SC  DIRECT_CHARGE_FAULT_VDROP_OVP  /
 * flag3 bit5    vbat ovp   LVC/SC  DIRECT_CHARGE_FAULT_VBAT_OVP  /
 * flag3 bit4    vusb ovp    SC     DIRECT_CHARGE_FAULT_VBUS_OVP

 * flag4 bit7    vbus ov    SC      DIRECT_CHARGE_FAULT_VBUS_OVP
 * flag4 bit6    vbus uv     SC
 * flag4 bit5    Vin-2Vout   SC     DIRECT_CHARGE_FAULT_VDROP_OVP
 * flag4 bit4    vbat uvp    SC
 * flag4 bit3    ibat ocp   LVC/SC   DIRECT_CHARGE_FAULT_IBAT_OCP /
 * flag4 bit2    ibat ucp   LVC/SC   DIRECT_CHARGE_FAULT_IBAT_OCP /
 * flag4 bit1    ibus ocp   SC       DIRECT_CHARGE_FAULT_IBUS_OCP  opt1
 * flag4 bit0    ibus ocp   SC       DIRECT_CHARGE_FAULT_IBUS_OCP
*/
#define FAULT_VBUS_OVP                      0x801018
#define FAULT_IBUS_OCP                      0x030002
#define FAULT_REVERSE_OCP                   0x008000
#define FAULT_VDROP_OVP                     0x204000
#define FAULT_VBAT_OVP                      0x002000
#define FAULT_IBAT_OCP                      0x0C0000

/* BUCK IRQ HANDLE */
#define FAULT_BUCK_VBAT_OVP                 0x000010
#define FAULT_OTG_OCP                       0x008000
#define FAULT_REVERSBST                     0x800000
#define FAULT_RECHG                         0x000200
#define FAULT_CHG_FAULT                     0x000800
#define FAULT_CHG_DONE                      0x000080

#define REVERBST_RETRY                      300

#define R_MOHM_1                            1
#define R_MOHM_2                            2
#define R_MOHM_5                            5
#define R_MOHM_10                           10

#define R_MOHM_DEFAULT                      R_MOHM_2 /* resisitance mohm */

#define TRIM_A_MIN                          0
#define TRIM_A_MAX                          2

/* NTC Table length */
#define T_V_ARRAY_LENGTH	31

#define V_REF_V600		2500 /* mV */
#define R_PU			18000L /* omh */
#define MA_2_NA			1000000L
#define V_REF_V600		2500 /* mV */
#define V_PU			1800 /* mV */
#define MV_2_NV			1000000L
#define MA_2_NA			1000000L
#define UV_2_NV			1000L

struct opt_regs {
	u16 reg;
	u8 mask;
	u8 shift;
	u8 val;
	unsigned long before; /* before delay ms */
	unsigned long after; /* after delay ms */
};

#define reg_cfg(r, m, s, v, b, a)  { .reg = (r), .mask = (m), .shift = (s), .val = (v), .before = (b), .after = (a), }

int hi6526_write(u16 reg, u8 value);
int hi6526_read(u16 reg, u8 *value);
void hi6526_set_adc_acr_mode(int enable);
int hi6526_get_chip_temp(int *temp);
int hi6526_read_block(u16 reg, u8 *value, unsigned int num_bytes);
int hi6526_write_block(u16 reg, u8 *value, unsigned int num_bytes);
int hi6526_fcp_adapter_reg_read(u8 *val, u8 reg);
int hi6526_read_mask(u16 reg, u8 mask, u8 shift, u8 *value);
int hi6526_write_mask(u16 reg, u8 mask, u8 shift, u8 value);
int hi6526_fcp_adapter_reg_write(u8 val, u8 reg);
int hi6526_switch_to_buck_mode(void);
int hi6526_lvc_is_close(void *dev_data);
int hi6526_get_loadswitch_id(void *dev_data);
int hi6526_sc_is_close(void);
int hi6526_get_switchcap_id(void);
int hi6526_batinfo_get_ibus_ma(int *vbus_mv);
int hi6526_get_vbus_mv(unsigned int *vbus_mv);
int hi6526_get_ibat(int *ibat_ma);
int hi6526_get_vout(int *vout_mv, void *dev_data);
int hi6526_get_vbat(void);
int hi6526_get_ibus_ma(void);
int hi6526_reset_watchdog_timer(void);
int hi6526_get_dp_res(void);
int __hi6526_get_chip_temp(int *temp);
int hi6526_get_adc_value(u32 chan, u32 *data);
void hi6526_direct_charge_fault_handle(u32 lvc_sc_irq_state);
int hi6526_lvc_enable(int enable, void *dev_data);
int hi6526_sc_enable(int enable);
int hi6526_set_charge_enable(int enable);
int hi6526_config_opt_param(int vbus_vol);
int hi6526_set_fast_safe_timer(u32 safe_timer);
int hi6526_dpm_init(void);
int hi6526_set_term_enable(int enable);
void hi6526_set_input_current_limit(int enable);
void hi6526_set_anti_reverbst_reset(void);
void set_boot_weaksource_flag(void);
void hi6526_set_input_current_limit(int enable);
void hi6526_vbat_lv_handle(void);
int hi6526_get_vbus_uvp_state(void);
int hi6526_set_vbus_vset(u32 value);
int hi6526_set_vbus_ovp(int vbus);
int hi6526_scp_master_init(void);
int set_buck_mode_enable(int enable);
int hi6526_set_input_current(int cin_limit);
int hi6526_set_charge_current(int charge_current);
int hi6526_set_terminal_voltage(int charge_voltage);
int hi6526_set_terminal_current(int term_current);
int hi6526_set_watchdog_timer(int value);
void hi6526_opt_regs_set(struct opt_regs *opt, unsigned int len);
void hi6526_opt_regs_set(struct opt_regs *opt, unsigned int len);
int hi6526_adc_loop_enable(int enable);
int hi6526_dummy_fun_1(void);
int hi6526_dummy_fun_2(int val);
int hi6526_set_watchdog_timer_ms(int ms);
int hi6526_get_vbus_mv2(int *vbus_mv);
int hi6526_set_batfet_ctrl(u32 status);
int hi6526_set_otg_current(int value);
int hi6526_set_otg_enable(int enable);
int hi6526_ibat_res_sel(int resisitance);
int hi6526_set_vbus_uvp_ovp(int vbus);
void hi6526_vusb_uv_det_enable(u8 enable);
int hi6526_set_recharge_vol(u8 rechg);
int hi6526_set_precharge_current(int precharge_current);
int hi6526_set_charge_current(int charge_current);
int hi6526_set_precharge_voltage(u32 pre_vchg);
void hi6526_set_input_current_limit(int enable);
int hi6526_set_charge_current(int charge_current);
int hi6526_set_bat_comp(int value);
int hi6526_set_vclamp(int value);
void hi6526_dbg_work(struct work_struct *work);
void hi6526_dc_ucp_delay_work(struct work_struct *work);
void hi6526_unmask_all_irq(void);
void hi6526_irq_work(struct work_struct *work);
int hi6526_irq_init(struct hi6526_device_info *di, struct device_node *np);
int hi6526_get_vusb(int *vusb_mv);
int hi6526_efuse_write(int efuse_id, u8 offset, u8 value, int pre_cali);
int hi6526_efuse_read(int efuse_id, u8 offset, u8 *value, u8 pre_cali);
int hi6526_chip_init(struct chip_init_crit *init_crit);
int hi6526_get_dieid_str(char *dieid, unsigned int len);
int hi6526_get_tsbat(int *ts_bat);
int hi6526_get_vusb_force(int *vusb_mv);
int hi6526_efuse_get_dieid(u8 *dieid, unsigned int len);
int hi6526_fcp_reg_read_block(int reg, int *val, int num, void *dev_data);
int hi6526_fcp_reg_write_block(int reg, const int *val, int num, void *dev_data);
int hi6526_fcp_detect_adapter(void *dev_data);
int hi6526_fcp_master_reset(void *dev_data);
int hi6526_fcp_adapter_reset(void *dev_data);
int hi6526_fcp_read_switch_status(void *dev_data);
int hi6526_fcp_stop_charge_config(void *dev_data);
int hi6526_is_fcp_charger_type(void *dev_data);
int hi6526_scp_reg_read_block(int reg, int *val, int num, void *dev_data);
int hi6526_scp_reg_write_block(int reg, const int *val, int num, void *dev_data);
int hi6526_fcp_adapter_reg_read_block(u8 reg, u8 *val, u8 num, void *dev_data);
int hi6526_scp_detect_adapter(void *dev_data);
int hi6526_scp_chip_reset(void *dev_data);
int hi6526_scp_adaptor_reset(void *dev_data);
int hi6526_pre_init(void *dev_data);
int hi6526_is_support_fcp(void);
int hi6526_lock_mutex_init(struct hi6526_device_info *di);
void parse_dts(struct device_node *np, struct hi6526_device_info *di);
int hi6526_is_support_scp(void);
void hi6526_reset_n_efuse_dis(int dis);
void hi6526_sc_reg_enable(int enable);
void hi6526_lvc_reg_enable(int enable);

#endif
