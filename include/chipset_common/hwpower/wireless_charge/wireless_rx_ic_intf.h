/*
 * wireless_rx_ic_intf.h
 *
 * common interface, variables, definition etc for wireless_rx_ic_intf
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

#ifndef _WIRELESS_RX_IC_INTF_H_
#define _WIRELESS_RX_IC_INTF_H_

#include <linux/device.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

#define WLRX_IC_FOD_COEF_LEN          128
#define WLRX_IC_DFT_IOUT_MAX          1350

struct wlrx_ic_ops {
	void *dev_data;
	struct device_node *(*get_dev_node)(void *);
	int (*fw_update)(void *);
	int (*chip_init)(unsigned int, unsigned int, void *);
	void (*chip_reset)(void *);
	void (*chip_enable)(bool, void *);
	bool (*is_chip_enable)(void *);
	void (*sleep_enable)(bool, void *);
	bool (*is_sleep_enable)(void *);
	void (*ext_pwr_prev_ctrl)(bool, void *);
	void (*ext_pwr_post_ctrl)(bool, void *);
	void (*pmic_vbus_handler)(bool, void *);
	bool (*is_tx_exist)(void *);
	int (*kick_watchdog)(void *);
	void (*stop_charging)(void *);
	void (*send_ept)(int, void *);
	int (*get_chip_info)(char *, int, void *);
	int (*get_vrect)(int *, void *);
	int (*get_vout)(int *, void *);
	int (*get_iout)(int *, void *);
	int (*get_imax)(int *, void *);
	int (*get_vout_reg)(int *, void *);
	int (*get_vfc_reg)(int *, void *);
	int (*get_fop)(int *, void *);
	int (*get_temp)(int *, void *);
	int (*get_cep)(int *, void *);
	void (*get_bst_delay_time)(int *, void *);
	int (*get_fod_coef)(char *, int, void *);
	int (*set_vfc)(int, void *);
	int (*set_vout)(int, void *);
	int (*set_fod_coef)(const char *, void *);
	void (*set_pu_shell_flag)(bool, void *);
	bool (*need_chk_pu_shell)(void *);
	char *(*read_nvm_info)(int, void *);
};

#ifdef CONFIG_WIRELESS_CHARGER
int wlrx_ic_ops_register(struct wlrx_ic_ops *ops, unsigned int type);
bool wlrx_ic_is_ops_registered(unsigned int type);
struct device_node *wlrx_ic_get_dev_node(unsigned int type);
int wlrx_ic_fw_update(unsigned int type);
int wlrx_ic_chip_init(unsigned int ic_type, unsigned int init_type, unsigned int tx_type);
void wlrx_ic_chip_reset(unsigned int type);
void wlrx_ic_chip_enable(unsigned int type, bool flag);
bool wlrx_ic_is_chip_enable(unsigned int type);
void wlrx_ic_sleep_enable(unsigned int type, bool flag);
bool wlrx_ic_is_sleep_enable(unsigned int type);
void wlrx_ic_ext_pwr_prev_ctrl(unsigned int type, bool flag);
void wlrx_ic_ext_pwr_post_ctrl(unsigned int type, bool flag);
void wlrx_ic_pmic_vbus_handler(unsigned int type, bool flag);
bool wlrx_ic_is_tx_exist(unsigned int type);
int wlrx_ic_kick_watchdog(unsigned int type);
void wlrx_ic_stop_charging(unsigned int type);
void wlrx_ic_send_ept(unsigned int type, int ept_type);
int wlrx_ic_get_chip_info(unsigned int type, char *buf, int len);
int wlrx_ic_get_vrect(unsigned int type, int *vrect);
int wlrx_ic_get_vout(unsigned int type, int *vout);
int wlrx_ic_get_iout(unsigned int type, int *iout);
int wlrx_ic_get_imax(unsigned int type, int *rx_imax);
int wlrx_ic_get_vout_reg(unsigned int type, int *vreg);
int wlrx_ic_get_vfc_reg(unsigned int type, int *vfc_reg);
int wlrx_ic_get_fop(unsigned int type, int *fop);
int wlrx_ic_get_temp(unsigned int type, int *temp);
int wlrx_ic_get_cep(unsigned int type, int *cep);
void wlrx_ic_get_bst_delay_time(unsigned int type, int *time);
int wlrx_ic_get_fod_coef(unsigned int type, char *fod_str, int len);
int wlrx_ic_set_vfc(unsigned int type, int vfc);
int wlrx_ic_set_vout(unsigned int type, int vout);
int wlrx_ic_set_fod_coef(unsigned int type, const char *fod_str);
void wlrx_ic_set_pu_shell_flag(unsigned int type, bool flag);
bool wlrx_ic_need_chk_pu_shell(unsigned int type);
char *wlrx_ic_read_nvm_info(unsigned int type, int sec);
#else
static inline int wlrx_ic_ops_register(struct wlrx_ic_ops *ops, unsigned int type)
{
	return 0;
}

static inline bool wlrx_ic_is_ops_registered(unsigned int type)
{
	return false;
}

static inline struct device_node *wlrx_ic_get_dev_node(unsigned int type)
{
	return NULL;
}

static inline int wlrx_ic_fw_update(unsigned int type)
{
	return -EPERM;
}

static inline int wlrx_ic_chip_init(unsigned int ic_type,
	unsigned int init_type, unsigned int tx_type)
{
	return -EPERM;
}

static inline void wlrx_ic_chip_reset(unsigned int type)
{
}

static inline void wlrx_ic_chip_enable(unsigned int type, bool flag)
{
}

static inline bool wlrx_ic_is_chip_enable(unsigned int type)
{
	return false;
}

static inline void wlrx_ic_sleep_enable(unsigned int type, bool flag)
{
}

static inline bool wlrx_ic_is_sleep_enable(unsigned int type)
{
	return false;
}

static inline void wlrx_ic_ext_pwr_prev_ctrl(unsigned int type, bool flag)
{
}

static inline void wlrx_ic_ext_pwr_post_ctrl(unsigned int type, bool flag)
{
}

static inline void wlrx_ic_pmic_vbus_handler(unsigned int type, bool flag)
{
}

static inline bool wlrx_ic_is_tx_exist(unsigned int type)
{
	return false;
}

static inline int wlrx_ic_kick_watchdog(unsigned int type)
{
	return -EPERM;
}

static inline void wlrx_ic_stop_charging(unsigned int type)
{
}

static inline void wlrx_ic_send_ept(unsigned int type, int ept_type)
{
}

static inline int wlrx_ic_get_chip_info(unsigned int type, char *buf, int len)
{
	return -EPERM;
}

static inline int wlrx_ic_get_vrect(unsigned int type, int *vrect)
{
	return -EPERM;
}

static inline int wlrx_ic_get_vout(unsigned int type, int *vout)
{
	return -EPERM;
}

static inline int wlrx_ic_get_iout(unsigned int type, int *iout)
{
	return -EPERM;
}

static inline int wlrx_ic_get_imax(unsigned int type, int *rx_imax)
{
	return -EPERM;
}

static inline int wlrx_ic_get_vout_reg(unsigned int type, int *vreg)
{
	return -EPERM;
}

static inline int wlrx_ic_get_vfc_reg(unsigned int type, int *vfc_reg)
{
	return -EPERM;
}

static inline int wlrx_ic_get_fop(unsigned int type, int *fop)
{
	return -EPERM;
}

static inline int wlrx_ic_get_temp(unsigned int type, int *temp)
{
	return -EPERM;
}

static inline int wlrx_ic_get_cep(unsigned int type, int *cep)
{
	return -EPERM;
}

static inline void wlrx_ic_get_bst_delay_time(unsigned int type, int *time)
{
}

static inline int wlrx_ic_get_fod_coef(unsigned int type, char *fod_str, int len)
{
	return -EPERM;
}

static inline int wlrx_ic_set_vfc(unsigned int type, int vfc)
{
	return -EPERM;
}

static inline int wlrx_ic_set_vout(unsigned int type, int vout)
{
	return -EPERM;
}

static inline int wlrx_ic_set_fod_coef(unsigned int type, const char *fod_str)
{
	return -EPERM;
}

static inline void wlrx_ic_set_pu_shell_flag(unsigned int type, bool flag)
{
}

static inline bool wlrx_ic_need_chk_pu_shell(unsigned int type)
{
	return false;
}

static inline char *wlrx_ic_read_nvm_info(unsigned int type, int sec)
{
	return NULL;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_IC_INTF_H_ */
