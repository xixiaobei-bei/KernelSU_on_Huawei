/*
 * hw_vote_private.h
 *
 * Privare head file of Device driver for regulators in IC
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __HW_VOTE_PRIVATE_H__
#define __HW_VOTE_PRIVATE_H__

#define CHANNEL_NAME_MAX	30
#define VSRC_NAME_MAX		30

extern struct hv_channel *g_hv_channel_array;
extern unsigned int g_hv_channel_size;

extern int hv_reg_read(const struct hv_reg_cfg *cfg, u32 *val);
extern void hw_vote_mutex_lock(void);
extern void hw_vote_mutex_unlock(void);
#endif /* __HW_VOTE_PRIVATE_H__ */
