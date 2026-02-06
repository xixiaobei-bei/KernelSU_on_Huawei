/*
 * power_delay.h
 *
 * delay time operation for power module
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

#ifndef _POWER_DELAY_H_
#define _POWER_DELAY_H_

#define DT_USLEEP_100US    100
#define DT_USLEEP_500US    500
#define DT_USLEEP_1MS      1000
#define DT_USLEEP_2MS      2000
#define DT_USLEEP_5MS      5000
#define DT_USLEEP_10MS     10000
#define DT_USLEEP_15MS     15000
#define DT_USLEEP_20MS     20000

#define DT_MSLEEP_20MS     20
#define DT_MSLEEP_25MS     25
#define DT_MSLEEP_30MS     30
#define DT_MSLEEP_50MS     50
#define DT_MSLEEP_100MS    100
#define DT_MSLEEP_200MS    200
#define DT_MSLEEP_250MS    250
#define DT_MSLEEP_500MS    500
#define DT_MSLEEP_1S       1000
#define DT_MSLEEP_2S       2000

void power_usleep(unsigned int delay);
bool power_msleep(unsigned int delay, unsigned int interval, bool (*exit)(void));

#endif /* _POWER_DELAY_H_ */
