/*
 * dp_aux_switch.h
 *
 * dp aux switch driver
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

#ifndef __DP_AUX_SWITCH_H__
#define __DP_AUX_SWITCH_H__

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

enum aux_switch_channel_type {
	CHANNEL_FSA4476 = 0,
	CHANNEL_SUPERSWITCH,
};

#ifdef CONFIG_DP_AUX_SWITCH
void dp_aux_switch_op(uint32_t value);
void dp_aux_uart_switch_enable(void);
void dp_aux_uart_switch_disable(void);
enum aux_switch_channel_type get_aux_switch_channel(void);
#else
static inline void dp_aux_switch_op(uint32_t value) {}
static inline void dp_aux_uart_switch_enable(void) {}
static inline void dp_aux_uart_switch_disable(void) {}
static inline enum aux_switch_channel_type get_aux_switch_channel(void)
{
	return CHANNEL_SUPERSWITCH;
}
#endif // !CONFIG_DP_AUX_SWITCH

#endif // __DP_AUX_SWITCH_H__
