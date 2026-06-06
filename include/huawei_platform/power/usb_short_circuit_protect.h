/*
 * usb_short_circuit_protect.h
 *
 * usb port short circuit protect (control by ap) monitor driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _USB_SHORT_CURRENT_PROTECT_H_
#define _USB_SHORT_CURRENT_PROTECT_H_

#define HIZ_MODE                1
#define NOT_HIZ_MODE            0
#define DMD_HIZ_DISABLE         0
#define USCP_ENABLE_PAR         0644
#define USB_TEMP_PAR            0444
#define USCP_ENFORCE_PAR        0644
#define USB_TEMP_NUM            25
#define GET_TEMP_VAL_NUM        2
#define CHECK_NTC_TEMP_MAX      100
#define CHECK_NTC_TEMP_MIN      (-30)
#define SLEEP_200MS             200
#define SLEEP_10MS              10
#define CHECK_INTERVAL_SLOW     10000
#define CHECK_INTERVAL_FAST     300
#define CHECK_CNT_INIT          1100
#define CHECK_CNT_LIMIT         1001
#define DEFAULT_TUSB_THRESHOLD  40
#define NTC_SAMPLES             3
#define NTC_INVALID_TEMP_THRE   10

int is_in_uscp_mode(void);
int is_uscp_hiz_mode(void);
int is_in_rt_uscp_mode(void);
int get_usb_ntc_temp(void);

#endif /* _USB_SHORT_CURRENT_PROTECT_H_ */
