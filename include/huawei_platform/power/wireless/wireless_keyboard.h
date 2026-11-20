/*
 * wireless_keyboard.h
 *
 * wireless keyboard detect driver
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

#ifndef _WIRELESS_KEYBOARD_H_
#define _WIRELESS_KEYBOARD_H_

#define WLC_START_CHARING  1
#define WLC_STOP_CHARING   0

#ifdef CONFIG_WIRELESS_ACCESSORY
void wlc_update_kb_control(int status);
#else
inline void wlc_update_kb_control(int status)
{
}
#endif /* CONFIG_WIRELESS_ACCESSORY */

#endif /* _WIRELESS_KEYBOARD_H_ */
