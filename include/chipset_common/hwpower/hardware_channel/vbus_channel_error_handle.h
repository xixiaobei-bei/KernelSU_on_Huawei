/*
 * vbus_channel_error_handle.h
 *
 * error handle for vbus channel driver
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

#ifndef _VBUS_CHANNEL_ERROR_HANDLE_H_
#define _VBUS_CHANNEL_ERROR_HANDLE_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>

enum vbus_ch_eh_dmd_type {
	BOOST_GPIO_SC_DMD_REPORT,
	BOOST_GPIO_OTG_OCP_DMD_REPORT,
};

struct otg_sc_para {
	u32 vol_mv;
	u32 check_times;
	u32 delayed_time;
};

struct otg_sc_info {
	struct delayed_work work;
	u32 fault_count;
	bool work_flag;
	struct otg_sc_para *para;
};

struct vbus_ch_eh_dev {
	struct notifier_block nb;
	struct otg_sc_info boost_gpio_otg_sc_info;
	int usb_state_count;
};

#ifdef CONFIG_VBUS_CHANNEL
bool vbus_ch_eh_get_otg_sc_flag(void);
#else
static inline bool vbus_ch_eh_get_otg_sc_flag(void)
{
	return false;
}
#endif /* CONFIG_VBUS_CHANNEL */

#endif /* _VBUS_CHANNEL_ERROR_HANDLE_H_ */
