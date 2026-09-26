// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_rx_common.c
 *
 * rx common interface of wireless charging
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/accessory/wireless_lightstrap.h>

enum wlrx_scene wlrx_get_scene(void)
{
	if (lightstrap_online_state())
		return WLRX_SCN_LIGHTSTRAP;

	return WLRX_SCN_NORMAL;
}
