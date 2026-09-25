/*
 * hisi_pcie_idle_sleep.h
 *
 * PCIe idle sleep vote interfaces for EP
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef _HISI_PCIE_IDLE_SLEEP_H_
#define _HISI_PCIE_IDLE_SLEEP_H_

enum {
	PCIE_DEVICE_WLAN = 0,
	/* Add more pcie devices here */
	PCIE_DEVICE_MAX,
};

#if defined(CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP)
void kirin_pcie_refclk_device_vote(u32 ep_type, u32 rc_id, u32 vote);
#else
static inline void kirin_pcie_refclk_device_vote(u32 ep_type, u32 rc_id, u32 vote)
{
}
#endif /* CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP */

#endif /* _HISI_PCIE_IDLE_SLEEP_H_ */
