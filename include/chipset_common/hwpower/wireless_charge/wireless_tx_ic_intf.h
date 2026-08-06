/*
 * wireless_tx_ic_intf.h
 *
 * common interface, variables, definition etc for wireless_tx_ic_intf
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

#ifndef _WIRELESS_TX_IC_INTF_H_
#define _WIRELESS_TX_IC_INTF_H_

#include <linux/device.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

struct wltx_ic_ops {
	void *dev_data;
	struct device_node *(*get_dev_node)(void *);
	int (*fw_update)(void *);
	int (*prev_psy)(bool, void *);
	int (*chip_init)(unsigned int, void *);
	void (*chip_reset)(void *);
	void (*chip_enable)(bool, void *);
	int (*mode_enable)(bool, void *);
	int (*activate_chip)(void *);
	int (*set_open_flag)(bool, void *);
	int (*set_stop_cfg)(void *);
	bool (*is_rx_discon)(void *);
	bool (*is_in_tx_mode)(void *);
	bool (*is_in_rx_mode)(void *);
	int (*get_vrect)(u16 *, void *);
	int (*get_vin)(u16 *, void *);
	int (*get_iin)(u16 *, void *);
	int (*get_temp)(s16 *, void *);
	int (*get_fop)(u16 *, void *);
	int (*get_duty)(u8 *, void *);
	int (*get_cep)(s8 *, void *);
	int (*get_ptx)(u32 *, void *);
	int (*get_prx)(u32 *, void *);
	int (*get_ploss)(s32 *, void *);
	int (*get_ploss_id)(u8 *, void *);
	int (*get_ping_freq)(u16 *, void *);
	int (*get_ping_interval)(u16 *, void *);
	int (*get_min_fop)(u16 *, void *);
	int (*get_max_fop)(u16 *, void *);
	int (*get_full_bridge_ith)(u16 *, void *);
	int (*set_ping_freq)(u16, void *);
	int (*set_ping_interval)(u16, void *);
	int (*set_min_fop)(u16, void *);
	int (*set_max_fop)(u16, void *);
	int (*set_ilim)(u16, void *);
	int (*set_fod_coef)(u16, u8, void *);
	int (*set_rp_dm_to)(u8, void *);
	int (*set_vset)(int, void *);
	int (*set_bridge)(unsigned int, unsigned int, void *);
};

#ifdef CONFIG_WIRELESS_CHARGER
int wltx_ic_ops_register(struct wltx_ic_ops *ops, unsigned int type);
bool wltx_ic_is_ops_registered(unsigned int type);
struct device_node *wltx_ic_get_dev_node(unsigned int type);
int wltx_ic_fw_update(unsigned int type);
int wltx_ic_prev_power_supply(unsigned int type, bool flag);
int wltx_ic_chip_init(unsigned int type, unsigned int client);
void wltx_ic_chip_reset(unsigned int type);
void wltx_ic_chip_enable(unsigned int type, bool flag);
int wltx_ic_mode_enable(unsigned int type, bool flag);
int wltx_ic_activate_chip(unsigned int type);
int wltx_ic_set_open_flag(unsigned int type, bool flag);
int wltx_ic_set_stop_config(unsigned int type);
bool wltx_ic_is_rx_disconnect(unsigned int type);
bool wltx_ic_is_in_tx_mode(unsigned int type);
bool wltx_ic_is_in_rx_mode(unsigned int type);
int wltx_ic_get_vrect(unsigned int type, u16 *vrect);
int wltx_ic_get_vin(unsigned int type, u16 *vin);
int wltx_ic_get_iin(unsigned int type, u16 *iin);
int wltx_ic_get_temp(unsigned int type, s16 *temp);
int wltx_ic_get_fop(unsigned int type, u16 *fop);
int wltx_ic_get_duty(unsigned int type, u8 *duty);
int wltx_ic_get_cep(unsigned int type, s8 *cep);
int wltx_ic_get_ptx(unsigned int type, u32 *ptx);
int wltx_ic_get_prx(unsigned int type, u32 *prx);
int wltx_ic_get_ploss(unsigned int type, s32 *ploss);
int wltx_ic_get_ploss_id(unsigned int type, u8 *id);
int wltx_ic_get_ping_freq(unsigned int type, u16 *freq);
int wltx_ic_get_ping_interval(unsigned int type, u16 *interval);
int wltx_ic_get_min_fop(unsigned int type, u16 *min_fop);
int wltx_ic_get_max_fop(unsigned int type, u16 *max_fop);
int wltx_ic_get_full_bridge_ith(unsigned int type, u16 *ith);
int wltx_ic_set_ping_freq(unsigned int type, u16 freq);
int wltx_ic_set_ping_interval(unsigned int type, u16 interval);
int wltx_ic_set_min_fop(unsigned int type, u16 min_fop);
int wltx_ic_set_max_fop(unsigned int type, u16 max_fop);
int wltx_ic_set_ilim(unsigned int type, u16 ilim);
int wltx_ic_set_fod_coef(unsigned int type, u16 ploss_th, u8 ploss_cnt);
int wltx_ic_set_rp_dm_to(unsigned int type, u16 timeout);
int wltx_ic_set_vset(unsigned int type, int vset);
int wltx_ic_set_bridge(unsigned int type, unsigned int v_ask, unsigned int bridge);
#else
static inline int wltx_ic_ops_register(struct wltx_ic_ops *ops, unsigned int type)
{
	return 0;
}

static inline bool wltx_ic_is_ops_registered(unsigned int type)
{
	return false;
}

static inline struct device_node *wltx_ic_get_dev_node(unsigned int type)
{
	return NULL;
}

static inline int wltx_ic_fw_update(unsigned int type)
{
	return -EPERM;
}

static inline int wltx_ic_prev_power_supply(unsigned int type, bool flag)
{
	return -EPERM;
}

static inline int wltx_ic_chip_init(unsigned int type, unsigned int client)
{
	return -EPERM;
}

static inline void wltx_ic_chip_reset(unsigned int type)
{
}

static inline void wltx_ic_chip_enable(unsigned int type, bool flag)
{
}

static inline int wltx_ic_mode_enable(unsigned int type, bool flag)
{
	return -EPERM;
}

static inline int wltx_ic_activate_chip(unsigned int type)
{
	return -EPERM;
}

static inline int wltx_ic_set_open_flag(unsigned int type, bool flag)
{
	return -EPERM;
}

static inline int wltx_ic_set_stop_config(unsigned int type)
{
	return -EPERM;
}

static inline bool wltx_ic_is_rx_disconnect(unsigned int type)
{
	return true;
}

static inline bool wltx_ic_is_in_tx_mode(unsigned int type)
{
	return false;
}

static inline bool wltx_ic_is_in_rx_mode(unsigned int type)
{
	return false;
}

static inline int wltx_ic_get_vrect(unsigned int type, u16 *vrect)
{
	return -EPERM;
}

static inline int wltx_ic_get_vin(unsigned int type, u16 *vin)
{
	return -EPERM;
}

static inline int wltx_ic_get_iin(unsigned int type, u16 *iin)
{
	return -EPERM;
}

static inline int wltx_ic_get_temp(unsigned int type, s16 *temp)
{
	return -EPERM;
}

static inline int wltx_ic_get_fop(unsigned int type, u16 *fop)
{
	return -EPERM;
}

static inline int wltx_ic_get_duty(unsigned int type, u8 *duty)
{
	return -EPERM;
}

static inline int wltx_ic_get_cep(unsigned int type, s8 *cep)
{
	return -EPERM;
}

static inline int wltx_ic_get_ptx(unsigned int type, u32 *ptx)
{
	return -EPERM;
}

static inline int wltx_ic_get_prx(unsigned int type, u32 *prx)
{
	return -EPERM;
}

static inline int wltx_ic_get_ploss(unsigned int type, s32 *ploss)
{
	return -EPERM;
}

static inline int wltx_ic_get_ploss_id(unsigned int type, u8 *id)
{
	return -EPERM;
}

static inline int wltx_ic_get_ping_freq(unsigned int type, u16 *freq)
{
	return -EPERM;
}

static inline int wltx_ic_get_ping_interval(unsigned int type, u16 *interval)
{
	return -EPERM;
}

static inline int wltx_ic_get_min_fop(unsigned int type, u16 *min_fop)
{
	return -EPERM;
}

static inline int wltx_ic_get_max_fop(unsigned int type, u16 *max_fop)
{
	return -EPERM;
}

static inline int wltx_ic_get_full_bridge_ith(unsigned int type, u16 *ith)
{
	return -EPERM;
}

static inline int wltx_ic_set_ping_freq(unsigned int type, u16 freq)
{
	return -EPERM;
}

static inline int wltx_ic_set_ping_interval(unsigned int type, u16 interval)
{
	return -EPERM;
}

static inline int wltx_ic_set_min_fop(unsigned int type, u16 min_fop)
{
	return -EPERM;
}

static inline int wltx_ic_set_max_fop(unsigned int type, u16 max_fop)
{
	return -EPERM;
}

static inline int wltx_ic_set_ilim(unsigned int type, u16 ilim)
{
	return -EPERM;
}

static inline int wltx_ic_set_fod_coef(unsigned int type, u16 ploss_th, u8 ploss_cnt)
{
	return -EPERM;
}

static inline int wltx_ic_set_rp_dm_to(unsigned int type, u16 timeout)
{
	return -EPERM;
}

static inline int wltx_ic_set_vset(unsigned int type, int vset)
{
	return -EPERM;
}

static inline int wltx_ic_set_bridge(unsigned int type, unsigned int v_ask, unsigned int bridge)
{
	return -EPERM;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_IC_INTF_H_ */
