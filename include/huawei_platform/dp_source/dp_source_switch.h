/*
 * dp_source_switch.h
 *
 * dp source switch interface
 *
 * Copyright (c) 2008-2019 Huawei Technologies Co., Ltd.
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

#ifndef __DP_SOURCE_SWITCH_H__
#define __DP_SOURCE_SWITCH_H__

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

enum dp_event_type {
	DP_LINK_STATE_BAD = 0,
	DP_LINK_STATE_GOOD,
};

enum dp_source_mode {
	DIFF_SOURCE = 0,
	SAME_SOURCE,
};

#ifdef CONFIG_HW_DP_SOURCE
int get_current_dp_source_mode(void);
int update_external_display_timming_info(uint32_t width,
					 uint32_t high,
					 uint32_t fps);
void dp_send_event(enum dp_event_type event);
#else
static inline int get_current_dp_source_mode(void) { return SAME_SOURCE; }

static inline int update_external_display_timming_info(uint32_t width,
						       uint32_t high,
						       uint32_t fps)
{
	return 0;
}

static inline void dp_send_event(enum dp_event_type event) {}
#endif // !CONFIG_HW_DP_SOURCE

#endif // __DP_SOURCE_SWITCH_H__

