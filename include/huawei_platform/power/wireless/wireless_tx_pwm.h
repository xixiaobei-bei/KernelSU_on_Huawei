/*
 * wireless_tx_pwm.h
 *
 * pwm head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_PWM_H_
#define _WIRELESS_TX_PWM_H_

#ifdef CONFIG_WIRELESS_CHARGER
void wltx_pwm_close(unsigned int ic_type);
int wltx_pwm_set_volt(unsigned int ic_type, int vset);
#else
static inline void wltx_pwm_close(unsigned int ic_type)
{
}

static inline int wltx_pwm_set_volt(unsigned int ic_type, int vset)
{
	return -ENOTSUPP;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_PWM_H_ */
