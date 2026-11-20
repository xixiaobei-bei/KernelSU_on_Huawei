/*
 * stwlc88.h
 *
 * stwlc88 macro, addr etc.
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

#ifndef _STWLC88_H_
#define _STWLC88_H_

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
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>

#include "stwlc88_chip.h"

#define ST88_SW2TX_SLEEP_TIME              25 /* ms */
#define ST88_SW2TX_RETRY_TIME              500 /* ms */

#define ST88_SHUTDOWN_SLEEP_TIME           200
#define ST88_RCV_MSG_SLEEP_TIME            100
#define ST88_RCV_MSG_SLEEP_CNT             10
#define ST88_WAIT_FOR_ACK_SLEEP_TIME       50
#define ST88_WAIT_FOR_ACK_RETRY_CNT        10
#define ST88_WAIT_FOR_SEND_SLEEP_TIME      50
#define ST88_WAIT_FOR_SEND_RETRY_CNT       10
#define ST88_SNED_MSG_RETRY_CNT            2
#define ST88_RX_TMP_BUFF_LEN               32

/* coil test */
#define ST88_COIL_TEST_PING_INTERVAL       0
#define ST88_COIL_TEST_PING_FREQ           115

struct stwlc88_chip_info {
	u16 chip_id;
	u8 chip_rev;
	u8 cust_id;
	u16 rom_id;
	u16 ftp_patch_id;
	u16 ram_patch_id;
	u16 cfg_id;
	u16 pe_id;
};

struct stwlc88_global_val {
	bool ftp_chk_complete;
	bool rx_stop_chrg_flag;
	bool tx_stop_chrg_flag;
	bool irq_abnormal_flag;
	unsigned int ftp_status;
	struct qi_protocol_handle *qi_hdl;
};

struct stwlc88_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct stwlc88_rx_mod_cfg {
	u8 low_volt;
	u8 high_volt;
	u8 fac_high_volt;
};

struct stwlc88_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work ftp_check_work;
	struct mutex mutex_irq;
	struct stwlc88_global_val g_val;
	struct stwlc88_tx_init_para tx_init_para;
	struct stwlc88_rx_mod_cfg mod_cfg;
	unsigned int ic_type;
	bool irq_active;
	u8 rx_ldo_cfg_5v[ST88_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_9v[ST88_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_12v[ST88_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_sc[ST88_RX_LDO_CFG_LEN];
	u8 tx_fod_para[ST88_TX_FOD_LEN];
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
	u8 tx_fod_cnt;
};

/* stwlc88 common */
int stwlc88_read_byte(u16 reg, u8 *data);
int stwlc88_read_word(u16 reg, u16 *data);
int stwlc88_write_byte(u16 reg, u8 data);
int stwlc88_write_word(u16 reg, u16 data);
int stwlc88_read_byte_mask(u16 reg, u8 mask, u8 shift, u8 *data);
int stwlc88_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data);
int stwlc88_read_word_mask(u16 reg, u16 mask, u16 shift, u16 *data);
int stwlc88_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data);
int stwlc88_read_block(u16 reg, u8 *data, u8 len);
int stwlc88_write_block(u16 reg, u8 *data, u8 data_len);
int stwlc88_hw_read_block(u32 addr, u8 *data, u8 len);
int stwlc88_hw_write_block(u32 addr, u8 *data, u8 data_len);
void stwlc88_chip_reset(void *dev_data);
void stwlc88_chip_enable(bool enable, void *dev_data);
bool stwlc88_is_chip_enable(void *dev_data);
void stwlc88_enable_irq(struct stwlc88_dev_info *di);
void stwlc88_disable_irq_nosync(struct stwlc88_dev_info *di);
void stwlc88_get_dev_info(struct stwlc88_dev_info **di);
struct device_node *stwlc88_dts_dev_node(void *dev_data);
int stwlc88_get_mode(u8 *mode);
bool stwlc88_is_pwr_good(void);

/* stwlc88 chip_info */
int stwlc88_get_chip_id(u16 *chip_id);
int stwlc88_get_ftp_patch_id(u16 *ftp_patch_id);
int stwlc88_get_cfg_id(u16 *cfg_id);
int stwlc88_get_die_id(u8 *die_id, int len);
int stwlc88_get_chip_info_str(char *info_str, int len, void *dev_data);
int stwlc88_get_chip_fw_version(u8 *data, int len, void *dev_data);

/* stwlc88 rx */
int stwlc88_rx_ops_register(struct stwlc88_dev_info *di);
void stwlc88_rx_mode_irq_handler(struct stwlc88_dev_info *di);
void stwlc88_rx_abnormal_irq_handler(struct stwlc88_dev_info *di);
void stwlc88_rx_shutdown_handler(struct stwlc88_dev_info *di);
void stwlc88_rx_probe_check_tx_exist(struct stwlc88_dev_info *di);
bool stwlc88_rx_is_tx_exist(void *dev_data);

/* stwlc88 tx */
int stwlc88_tx_ops_register(struct stwlc88_dev_info *di);
void stwlc88_tx_mode_irq_handler(struct stwlc88_dev_info *di);
struct wireless_tx_device_ops *stwlc88_get_tx_ops(void);
struct wlps_tx_ops *stwlc88_get_txps_ops(void);

/* stwlc88 fw */
int stwlc88_fw_ops_register(struct stwlc88_dev_info *di);
void stwlc88_fw_ftp_check_work(struct work_struct *work);
int stwlc88_fw_sram_update(void *dev_data);

/* stwlc88 qi */
int stwlc88_qi_ops_register(struct stwlc88_dev_info *di);

/* stwlc88 dts */
int stwlc88_parse_dts(struct device_node *np, struct stwlc88_dev_info *di);

/* stwlc88 hw_test */
int stwlc88_hw_test_ops_register(void);

#endif /* _STWLC88_H_ */
