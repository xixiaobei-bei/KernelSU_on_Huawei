/*
 * hung_wp_screen.h
 *
 * This file is the header file for hung wp screen
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LINUX_HUNG_WP_SCREEN_H__
#define __LINUX_HUNG_WP_SCREEN_H__

#define WP_SCREEN_PWK_PRESS 0
#define WP_SCREEN_PWK_RELEASE 1
#define WP_SCREEN_PWK_LONGPRESS 2
#define WP_SCREEN_VDOWN_KEY 114
#define WP_SCREEN_VUP_KEY 115

void *hung_wp_screen_qcom_pkey_press(int type, int state);
void hung_wp_screen_powerkey_ncb(unsigned long event);
void hung_wp_screen_setbl(int level);
void hung_wp_screen_vkeys_cb(unsigned int knum, unsigned int value);
#endif
