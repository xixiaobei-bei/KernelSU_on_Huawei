/*
 * usb_extra_modem.h
 *
 * header file for usb_extra_modem driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef _USB_EXTRA_MODEM_H_
#define _USB_EXTRA_MODEM_H_

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>

#define UEM_BASE_DEC                      10
#define UEM_UEVENT_SIZE                   3
#define UEM_VID                           0x3426
#define UEM_PID                           0x2999
#define UEM_ATTACH_WORK_DELAY             100
#define UEM_VBUS_INSERT_DELAY             100
#define UEM_OTG_INSERT_DELAY              100
#define UEM_CHARGE_INFO_DELAY             150
#define UEM_NCM_ENUMERATION_TIME_OUT      10000

enum uem_hwuvdm_command {
	UEM_HWUVDM_CMD_CMOS_Q2_CLOSE = 1,
	UEM_HWUVDM_CMD_VBUS_INSERT = 2,
	UEM_HWUVDM_CMD_CHARGE_READY = 3,
	UEM_HWUVDM_CMD_OTG_INSERT = 4,
	UEM_HWUVDM_CMD_OTG_DISCONNECT = 5,
	UEM_HWUVDM_CMD_CMOS_Q3_OPEN = 6,
	UEM_HWUVDM_CMD_PMU_ENABLE = 7,
	UEM_HWUVDM_CMD_PMU_DISABLE = 8,
	UEM_HWUVDM_CMD_REQUEST_CHARGE_INFO = 9,
	UEM_HWUVDM_CMD_USB_RESET = 10,
	UEM_HWUVDM_CMD_MODEM_WAKEUP = 11,
	UEM_HWUVDM_CMD_USB_POWERON = 12,
	UEM_HWUVDM_CMD_USB_POWEROFF = 13,
};

struct uem_dev_info {
	struct device *dev;
	struct notifier_block nb;
	struct delayed_work attach_work;
	struct delayed_work vbus_insert_work;
	struct delayed_work otg_insert_work;
	struct delayed_work charge_info_work;
	struct delayed_work ncm_enumeration_work;
	struct wakeup_source *uem_lock;
	u32 charge_resistance;
	u32 charge_leak_current;
	bool attach_status;
	bool otg_status;
	bool vbus_status;
	bool modem_active_status;
};

struct uem_dev_info *uem_get_dev_info(void);

#ifdef CONFIG_USB_EXTRA_MODEM
unsigned int uem_get_charge_resistance(void);
unsigned int uem_get_charge_leak_current(void);
void uem_handle_pr_swap_end(void);
bool uem_check_online_status(void);
void uem_handle_detach_event(void);
void uem_set_product_id_info(unsigned int vid, unsigned int pid);
#else
static inline unsigned int uem_get_charge_resistance(void)
{
	return 0;
}

static inline unsigned int uem_get_charge_leak_current(void)
{
	return 0;
}

static inline void uem_handle_pr_swap_end(void)
{
}

static inline bool uem_check_online_status(void)
{
	return false;
}

static inline void uem_handle_detach_event(void)
{
}

static inline void uem_set_product_id_info(unsigned int vid, unsigned int pid)
{
}
#endif /* CONFIG_USB_EXTRA_MODEM */

#endif /* _USB_EXTRA_MODEM_H_ */
