/*
 * stwlc68.h
 *
 * stwlc68 macro, addr etc.
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

#ifndef _STWLC68_H_
#define _STWLC68_H_

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
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include "stwlc68_chip.h"

struct stwlc68_chip_info {
	u16 chip_id;
	u8 chip_rev;
	u8 cust_id;
	u16 sram_id;
	u16 svn_rev;
	u16 cfg_id;
	u16 pe_id;
	u16 patch_id;
	u8 cut_id;
};

struct stwlc68_global_val {
	bool otp_skip_soak_recovery_flag;
	int ram_rom_status;
	int sram_bad_addr;
	bool sram_chk_complete;
};

struct stwlc68_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct mutex mutex_irq;
	struct stwlc68_global_val g_val;
	u8 rx_fod_5v[STWLC68_RX_FOD_LEN];
	u8 pu_rx_fod_5v[STWLC68_RX_FOD_LEN];
	u8 rx_fod_9v[STWLC68_RX_FOD_LEN];
	u8 rx_fod_9v_cp60[STWLC68_RX_FOD_LEN];
	u8 rx_fod_9v_cp39s[STWLC68_RX_FOD_LEN];
	u8 rx_fod_12v[STWLC68_RX_FOD_LEN];
	u8 rx_fod_15v[STWLC68_RX_FOD_LEN];
	u8 rx_fod_15v_cp39s[STWLC68_RX_FOD_LEN];
	u8 rx_ldo_cfg_5v[STWLC68_LDO_CFG_LEN];
	u8 rx_ldo_cfg_9v[STWLC68_LDO_CFG_LEN];
	u8 rx_ldo_cfg_12v[STWLC68_LDO_CFG_LEN];
	u8 rx_ldo_cfg_sc[STWLC68_LDO_CFG_LEN];
	u8 rx_offset_9v;
	u8 rx_offset_15v;
	unsigned int ic_type;
	bool pu_shell_flag;
	bool need_chk_pu_shell;
	int max_iout;
	int rx_ss_good_lth;
	int tx_fod_th_5v;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_sleep_en;
	int gpio_int;
	int gpio_pwr_good;
	int irq_int;
	int irq_active;
	u16 irq_val;
	u16 ept_type;
	int irq_cnt;
	int support_cp;
	int dev_type;
	int tx_ocp_val;
	int tx_ovp_val;
	int tx_uvp_th;
	struct delayed_work sram_scan_work;
};

/* stwlc68 i2c */
int stwlc68_read_block(struct stwlc68_dev_info *di, u16 reg, u8 *data, u8 len);
int stwlc68_write_block(struct stwlc68_dev_info *di, u16 reg, u8 *data, u8 data_len);
int stwlc68_4addr_read_block(struct stwlc68_dev_info *di, u32 addr, u8 *data, u8 len);
int stwlc68_4addr_write_block(struct stwlc68_dev_info *di, u32 addr, u8 *data, u8 data_len);
int stwlc68_read_byte(u16 reg, u8 *data);
int stwlc68_write_byte(u16 reg, u8 data);
int stwlc68_read_word(u16 reg, u16 *data);
int stwlc68_write_word(u16 reg, u16 data);

/* stwlc68 common */
void stwlc68_enable_irq(struct stwlc68_dev_info *di);
void stwlc68_disable_irq_nosync(struct stwlc68_dev_info *di);
void stwlc68_chip_enable(bool enable, void *dev_data);

/* stwlc68 chip_info */
int stwlc68_get_cfg_id(u16 *cfg_id);
int stwlc68_get_patch_id(u16 *patch_id);
int stwlc68_get_cut_id(struct stwlc68_dev_info *di, u8 *cut_id);
int stwlc68_get_chip_info(struct stwlc68_chip_info *info);
int stwlc68_get_chip_info_str(char *info_str, int len, void *dev_data);

/* stwlc68 tx */
int stwlc68_sw2tx(void);

/* stwlc68 fw */
void stwlc68_fw_sram_scan_work(struct work_struct *work);
int stwlc68_fw_sram_update(struct stwlc68_dev_info *di, u32 sram_mode);
int stwlc68_fw_ops_register(struct stwlc68_dev_info *di);

#endif /* _STWLC68_H_ */
