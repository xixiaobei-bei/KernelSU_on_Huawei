/*
 * hal_common.h
 *
 * common config
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include "hi_type.h"
#include "drv_venc_ioctl.h"

#define LCU_SIZE_H265 64
#define LCU_SIZE_H264 16
#define BASE_BLOCK_SIZE 8
#define VENC_MASK 0xcf
#define CMDLIST_MASK 0x202
#define CMDLIST_LAST_FRAME_MASK 0x102
#define CMDLIST_CLR_INT 0x105
#define dist_protocol(protocol, value1, value2) (((protocol) == VEDU_H265) ? (value1) : (value2))

void vedu_hal_request_bus_idle(uint32_t *reg_base);
void vedu_hal_cfg_reg(struct encode_info *channel_cfg, uint32_t core_id);
void vedu_hal_cfg_reg_simple(struct encode_info *channel_cfg, uint32_t core_id);

#endif
