/*
 * dp_debug.h
 *
 * debug for dp module
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
#ifndef __DP_DEBUG_H__
#define __DP_DEBUG_H__

enum dp_link_lanes {
	DP_LINK_LANES_1 = 1,
	DP_LINK_LANES_2 = 2,
	DP_LINK_LANES_4 = 4,
	DP_LINK_LANES_MASK = 0xF,
};

enum dp_link_rate {
	DP_LINK_RATE_RBR = 1,
	DP_LINK_RATE_HBR2,
	DP_LINK_RATE_HBR3,
	DP_LINK_RATE_MAX,
	DP_LINK_RATE_MASK = 0xF,
};

enum dp_video_format {
	DP_VIDEO_FMT_VCEA = 0,
	DP_VIDEO_FMT_CVT,
	DP_VIDEO_FMT_DMT,
	DP_VIDEO_FMT_MAX,
};

#define DP_VIDEO_MODE_MASK 0xFF

void dp_debug_init_combophy_pree_swing(uint32_t *pv, int count);
int dp_debug_append_info(char *buf, int size, char *fmt, ...);
int dp_debug_get_vs_pe_force(uint8_t *vs_force, uint8_t *pe_force);
int dp_debug_get_lanes_rate_force(uint8_t *lanes_force, uint8_t *rate_force);
int dp_debug_get_resolution_force(uint8_t *user_mode, uint8_t *user_format);

#endif // __DP_DEBUG_H__

