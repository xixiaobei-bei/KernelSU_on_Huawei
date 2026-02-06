/*
 * cps4057.h
 *
 * cps4057 macro, addr etc.
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

#ifndef _CPS4057_H_
#define _CPS4057_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>

#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/hardware_ic/wireless_ic_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_hw.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>

#include "cps4057_chip.h"

#define CPS4057_SW2TX_SLEEP_TIME              25 /* ms */
#define CPS4057_SW2TX_RETRY_TIME              500 /* ms */

#define CPS4057_SHUTDOWN_SLEEP_TIME           200
#define CPS4057_RCV_MSG_SLEEP_TIME            100
#define CPS4057_RCV_MSG_SLEEP_CNT             10
#define CPS4057_WAIT_FOR_ACK_SLEEP_TIME       50
#define CPS4057_WAIT_FOR_ACK_RETRY_CNT        10
#define CPS4057_SEND_MSG_RETRY_CNT            2
#define CPS4057_RX_TMP_BUFF_LEN               32

/* coil test */
#define CPS4057_COIL_TEST_PING_INTERVAL       0
#define CPS4057_COIL_TEST_PING_FREQ           115

struct cps4057_chip_info {
	u16 chip_id;
	u16 mtp_ver;
};

struct cps4057_global_val {
	bool mtp_latest;
	bool mtp_chk_complete;
	bool rx_stop_chrg_flag;
	bool tx_stop_chrg_flag;
	bool irq_abnormal_flag;
	struct qi_protocol_handle *qi_hdl;
};

struct cps4057_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct cps4057_tx_fod_para {
	u16 ploss_th0;
	u16 ploss_th1;
	u16 ploss_th2;
	u16 ploss_th3;
	u16 hp_ploss_th0;
	u16 hp_ploss_th1;
	u16 hp_ploss_th2;
	u16 hp_cur_th0;
	u16 hp_cur_th1;
	u8 ploss_cnt;
	u16 q_en;
	u16 q_coil_th;
	u16 q_th;
};

struct cps4057_rx_mod_cfg {
	u8 low_volt;
	u8 high_volt;
	u8 fac_high_volt;
};

struct cps4057_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct mutex mutex_irq;
	struct cps4057_global_val g_val;
	struct cps4057_tx_init_para tx_init_para;
	struct cps4057_tx_fod_para tx_fod;
	struct cps4057_rx_mod_cfg mod_cfg;
	unsigned int ic_type;
	bool irq_active;
	u8 rx_ldo_cfg_5v[CPS4057_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_9v[CPS4057_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_12v[CPS4057_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_sc[CPS4057_RX_LDO_CFG_LEN];
	int rx_ss_good_lth;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int gpio_pwr_good;
	int irq_int;
	int full_bridge_ith;
	u16 irq_val;
	int irq_cnt;
	u16 ept_type;
};

/* cps4057 i2c */
int cps4057_read_byte(u16 reg, u8 *data);
int cps4057_read_word(u16 reg, u16 *data);
int cps4057_write_byte(u16 reg, u8 data);
int cps4057_write_word(u16 reg, u16 data);
int cps4057_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data);
int cps4057_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data);
int cps4057_read_block(u16 reg, u8 *data, u8 len);
int cps4057_write_block(u16 reg, u8 *data, u8 data_len);
int cps4057_aux_write_word(u16 reg, u16 data);
/* cps4057 common */
void cps4057_chip_enable(bool enable, void *dev_data);
bool cps4057_is_chip_enable(void *dev_data);
void cps4057_enable_irq(struct cps4057_dev_info *di);
void cps4057_disable_irq_nosync(struct cps4057_dev_info *di);
void cps4057_get_dev_info(struct cps4057_dev_info **di);
struct device_node *cps4057_dts_dev_node(void *dev_data);
int cps4057_get_mode(u8 *mode);
bool cps4057_is_pwr_good(void);
/* cps4057 chip_info */
int cps4057_get_chip_id(u16 *chip_id);
u8 *cps4057_get_die_id(void);
int cps4057_get_chip_info(struct cps4057_chip_info *info);
int cps4057_get_chip_info_str(char *info_str, int len, void *dev_data);

/* cps4057 rx */
void cps4057_rx_mode_irq_handler(struct cps4057_dev_info *di);
void cps4057_rx_abnormal_irq_handler(struct cps4057_dev_info *di);
void cps4057_rx_shutdown_handler(struct cps4057_dev_info *di);
void cps4057_rx_probe_check_tx_exist(struct cps4057_dev_info *di);
int cps4057_rx_ops_register(struct cps4057_dev_info *di);
bool cps4057_rx_is_tx_exist(void *dev_data);

/* cps4057 tx */
void cps4057_tx_mode_irq_handler(struct cps4057_dev_info *di);
struct wireless_tx_device_ops *cps4057_get_tx_ops(void);
int cps4057_tx_ops_register(struct cps4057_dev_info *di);

/* cps4057 fw */
void cps4057_fw_mtp_check_work(struct work_struct *work);
int cps4057_fw_sram_update(void *dev_data);
int cps4057_fw_ops_register(struct cps4057_dev_info *di);

/* cps4057 qi */
struct qi_protocol_ops *cps4057_get_qi_ops(void);
int cps4057_qi_ops_register(struct cps4057_dev_info *di);

/* cps4057 dts */
int cps4057_parse_dts(struct device_node *np, struct cps4057_dev_info *di);

/* cps4057 hw_test */
int cps4057_hw_test_ops_register(void);

#endif /* _CPS4057_H_ */
