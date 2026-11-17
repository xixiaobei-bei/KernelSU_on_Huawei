/*
 * mt5735.h
 *
 * mt5735 macro, addr etc.
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

#ifndef _MT5735_H_
#define _MT5735_H_

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
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/hardware_ic/wireless_ic_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
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
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common.h>

#include "mt5735_chip.h"

#define MT5735_SW2TX_SLEEP_TIME              25 /* ms */
#define MT5735_SW2TX_RETRY_TIME              500 /* ms */

#define MT5735_SHUTDOWN_SLEEP_TIME           200
#define MT5735_RCV_MSG_SLEEP_TIME            100
#define MT5735_RCV_MSG_SLEEP_CNT             10
#define MT5735_WAIT_FOR_ACK_SLEEP_TIME       100
#define MT5735_WAIT_FOR_ACK_RETRY_CNT        5
#define MT5735_SNED_MSG_RETRY_CNT            2
#define MT5735_RX_TMP_BUFF_LEN               32

/* coil test */
#define MT5735_COIL_TEST_PING_INTERVAL       0
#define MT5735_COIL_TEST_PING_FREQ           115

struct mt5735_chip_info {
	u16 chip_id;
	u8 cust_id;
	u8 hw_id;
	u16 minor_ver;
	u16 major_ver;
};

struct mt5735_global_val {
	bool mtp_chk_complete;
	bool mtp_latest;
	bool rx_stop_chrg_flag;
	bool tx_stop_chrg_flag;
	bool irq_abnormal_flag;
	struct qi_protocol_handle *qi_hdl;
};

struct mt5735_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct mt5735_tx_fod_para {
	u16 ploss_th0;
	u16 ploss_th1;
	u16 ploss_th2;
	u16 ploss_th3;
	u16 hp_ploss_th0;
	u16 hp_ploss_th1;
	u8 ploss_cnt;
};

struct mt5735_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct mutex mutex_irq;
	struct mt5735_global_val g_val;
	struct mt5735_tx_init_para tx_init_para;
	struct mt5735_tx_fod_para tx_fod;
	unsigned int ic_type;
	bool irq_active;
	u8 rx_ldo_cfg_5v[MT5735_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_9v[MT5735_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_12v[MT5735_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_sc[MT5735_RX_LDO_CFG_LEN];
	int rx_ss_good_lth;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int gpio_pwr_good;
	int irq_int;
	u32 irq_val;
	int irq_cnt;
	u32 ept_type;
	int full_bridge_ith;
};

/* mt5735 common */
int mt5735_read_byte(u16 reg, u8 *data);
int mt5735_write_byte(u16 reg, u8 data);
int mt5735_read_word(u16 reg, u16 *data);
int mt5735_write_word(u16 reg, u16 data);
int mt5735_read_dword(u16 reg, u32 *data);
int mt5735_write_dword(u16 reg, u32 data);
int mt5735_read_byte_mask(u16 reg, u8 mask, u8 shift, u8 *data);
int mt5735_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data);
int mt5735_read_word_mask(u16 reg, u16 mask, u16 shift, u16 *data);
int mt5735_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data);
int mt5735_read_dword_mask(u16 reg, u32 mask, u32 shift, u32 *data);
int mt5735_write_dword_mask(u16 reg, u32 mask, u32 shift, u32 data);
int mt5735_read_block(u16 reg, u8 *data, u8 len);
int mt5735_write_block(u16 reg, u8 *data, u8 data_len);
void mt5735_chip_reset(void *dev_data);
void mt5735_chip_enable(bool enable, void *dev_data);
bool mt5735_is_chip_enable(void *dev_data);
void mt5735_enable_irq(struct mt5735_dev_info *di);
void mt5735_disable_irq_nosync(struct mt5735_dev_info *di);
void mt5735_get_dev_info(struct mt5735_dev_info **di);
struct device_node *mt5735_dts_dev_node(void *dev_data);
int mt5735_get_mode(u16 *mode);
bool mt5735_is_pwr_good(void);

/* mt5735 chip_info */
int mt5735_get_chip_id(u16 *chip_id);
int mt5735_get_chip_info_str(char *info_str, int len, void *dev_data);
int mt5735_get_chip_fw_version(u8 *data, int len, void *dev_data);

/* mt5735 rx */
int mt5735_rx_ops_register(struct mt5735_dev_info *di);
void mt5735_rx_mode_irq_handler(struct mt5735_dev_info *di);
void mt5735_rx_abnormal_irq_handler(struct mt5735_dev_info *di);
void mt5735_rx_shutdown_handler(struct mt5735_dev_info *di);
void mt5735_rx_probe_check_tx_exist(struct mt5735_dev_info *di);
bool mt5735_rx_is_tx_exist(void *dev_data);

/* mt5735 tx */
int mt5735_tx_ops_register(struct mt5735_dev_info *di);
void mt5735_tx_mode_irq_handler(struct mt5735_dev_info *di);
struct wireless_tx_device_ops *mt5735_get_tx_ops(void);
struct wlps_tx_ops *mt5735_get_txps_ops(void);

/* mt5735 fw */
int mt5735_fw_ops_register(struct mt5735_dev_info *di);
void mt5735_fw_mtp_check_work(struct work_struct *work);
int mt5735_fw_sram_update(void *dev_data);

/* mt5735 qi */
int mt5735_qi_ops_register(struct mt5735_dev_info *di);

/* mt5735 dts */
int mt5735_parse_dts(const struct device_node *np, struct mt5735_dev_info *di);

/* mt5735 hw_test */
int mt5735_hw_test_ops_register(void);

#endif /* _MT5735_H_ */
