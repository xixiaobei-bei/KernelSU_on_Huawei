/*
 * cps4029.h
 *
 * cps4029 macro, addr etc.
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

#ifndef _CPS4029_H_
#define _CPS4029_H_

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
#include <chipset_common/hwpower/wireless_charge/wireless_trx_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_hw.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/wireless_charge/wireless_firmware.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>

#include "cps4029_chip.h"

#define CPS4029_SW2TX_SLEEP_TIME              25 /* ms */
#define CPS4029_SW2TX_RETRY_TIME              500 /* ms */

#define CPS4029_SHUTDOWN_SLEEP_TIME           200
#define CPS4029_RCV_MSG_SLEEP_TIME            100
#define CPS4029_RCV_MSG_SLEEP_CNT             10
#define CPS4029_WAIT_FOR_ACK_SLEEP_TIME       50
#define CPS4029_WAIT_FOR_ACK_RETRY_CNT        10
#define CPS4029_SEND_MSG_RETRY_CNT            2
#define CPS4029_RX_TMP_BUFF_LEN               32

struct cps4029_chip_info {
	u16 chip_id;
	u16 mtp_ver;
};

struct cps4029_global_val {
	bool sram_i2c_ready;
	bool mtp_chk_complete;
	bool mtp_latest;
	bool tx_stop_chrg_flag;
	struct qi_protocol_handle *qi_hdl;
};

struct cps4029_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct cps4029_tx_fod_para {
	u16 ploss_th0;
	u8 ploss_cnt;
};

struct cps4029_tx_fop_para {
	int tx_max_fop;
	int tx_min_fop;
};

struct cps4029_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct mutex mutex_irq;
	struct wakeup_source wakelock;
	struct cps4029_global_val g_val;
	struct cps4029_tx_init_para tx_init_para;
	struct cps4029_tx_fod_para tx_fod;
	struct cps4029_tx_fop_para tx_fop;
	unsigned int ic_type;
	bool irq_active;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_int;
	int irq_int;
	u16 irq_val;
	int irq_cnt;
	u16 ept_type;
};

/* cps4029 i2c */
int cps4029_read_byte(struct cps4029_dev_info *di, u16 reg, u8 *data);
int cps4029_read_word(struct cps4029_dev_info *di, u16 reg, u16 *data);
int cps4029_write_byte(struct cps4029_dev_info *di, u16 reg, u8 data);
int cps4029_write_word(struct cps4029_dev_info *di, u16 reg, u16 data);
int cps4029_write_byte_mask(struct cps4029_dev_info *di, u16 reg, u8 mask, u8 shift, u8 data);
int cps4029_write_word_mask(struct cps4029_dev_info *di, u16 reg, u16 mask, u16 shift, u16 data);
int cps4029_read_block(struct cps4029_dev_info *di, u16 reg, u8 *data, u8 len);
int cps4029_write_block(struct cps4029_dev_info *di, u16 reg, u8 *data, u8 data_len);
int cps4029_aux_write_word(struct cps4029_dev_info *di, u16 reg, u16 data);

/* cps4029 common */
void cps4029_chip_enable(bool enable, void *dev_data);
void cps4029_enable_irq(struct cps4029_dev_info *di);
void cps4029_disable_irq_nosync(struct cps4029_dev_info *di);
struct device_node *cps4029_dts_dev_node(void *dev_data);
int cps4029_get_mode(struct cps4029_dev_info *di, u8 *mode);

/* cps4029 chip_info */
int cps4029_get_chip_id(struct cps4029_dev_info *di, u16 *chip_id);
u8 *cps4029_get_die_id(void);
int cps4029_get_chip_info(struct cps4029_dev_info *di, struct cps4029_chip_info *info);
int cps4029_get_chip_info_str(struct cps4029_dev_info *di, char *info_str, int len);

/* cps4029 tx */
void cps4029_tx_mode_irq_handler(struct cps4029_dev_info *di);
int cps4029_tx_ops_register(struct wltrx_ic_ops *ops, struct cps4029_dev_info *di);

/* cps4029 fw */
void cps4029_fw_mtp_check_work(struct work_struct *work);
int cps4029_fw_sram_update(void *dev_data);
int cps4029_fw_ops_register(struct wltrx_ic_ops *ops, struct cps4029_dev_info *di);

/* cps4029 qi */
int cps4029_qi_ops_register(struct wltrx_ic_ops *ops, struct cps4029_dev_info *di);

/* cps4029 dts */
int cps4029_parse_dts(struct device_node *np, struct cps4029_dev_info *di);

/* cps4029 hw_test */
int cps4029_hw_test_ops_register(void);
int cps4029_fw_sram_i2c_init(struct cps4029_dev_info *di, u8 inc_mode);

#endif /* _CPS4029_H_ */
