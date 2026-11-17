/*
 * wireless_lightstrap.h
 *
 * wireless lightstrap driver
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

#ifndef _WIRELESS_LIGHTSTRAP_H_
#define _WIRELESS_LIGHTSTRAP_H_

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <huawei_platform/power/wireless/wireless_tx_pwr_src.h>

#define LIGHTSTRAP_MAX_RX_SIZE           6
#define LIGHTSTRAP_INFO_LEN              16
#define LIGHTSTRAP_ENVP_OFFSET4          4
#define LIGHTSTRAP_PRODUCT_TYPE          7
#define LIGHTSTRAP_OFF                   0
#define LIGHTSTRAP_TIMEOUT               3600
#define LIGHTSTRAP_DELAY                 500
#define LIGHTSTRAP_PING_FREQ_DEFAULT     0
#define LIGHTSTRAP_WORK_FREQ_DEFAULT     0
#define LIGHTSTRAP_RESET_TX_PING_FREQ    135

enum lightstrap_status_dmd_type {
	LIGHTSTRAP_ATTACH_DMD,
	LIGHTSTRAP_DETACH_DMD,
};

enum lightstrap_sysfs_type {
	LIGHTSTRAP_SYSFS_DEV_PRODUCT_TYPE,
};

enum lightstrap_status_type {
	LIGHTSTRAP_STATUS_DEF,  /* default */
	LIGHTSTRAP_STATUS_INIT, /* initialized */
	LIGHTSTRAP_STATUS_WWE,  /* waiting wlrx end */
	LIGHTSTRAP_STATUS_WPI,  /* waiting product info */
	LIGHTSTRAP_STATUS_DEV,  /* device on */
};

struct lightstrap_di {
	struct device *dev;
	struct notifier_block event_nb;
	struct work_struct event_work;
	struct delayed_work check_work;
	struct delayed_work tx_ping_work;
	struct mutex lock;
	enum lightstrap_status_type status;
	u8 product_type;
	u8 product_id;
	unsigned long event_type;
	bool is_opened_by_hall;
	bool tx_status_ping;
	bool hall_status;
	unsigned int ping_freq;
	unsigned int work_freq;
};

#ifdef CONFIG_WIRELESS_ACCESSORY
bool lightstrap_online_state(void);
enum wltx_pwr_src lightstrap_specify_pwr_src(void);
void lightstrap_reinit_tx_chip(void);
#else
static inline bool lightstrap_online_state(void)
{
	return false;
}

static inline enum wltx_pwr_src lightstrap_specify_pwr_src(void)
{
	return PWR_SRC_NULL;
}

static inline void lightstrap_reinit_tx_chip(void)
{
}
#endif /* CONFIG_WIRELESS_ACCESSORY */

#endif /* _WIRELESS_LIGHTSTRAP_H_ */