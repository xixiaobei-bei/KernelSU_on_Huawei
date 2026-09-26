/*
 * vsim.h
 *
 * monitor vsim traffic
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

#ifndef __HWRIL_VSIM_H
#define __HWRIL_VSIM_H
#include <net/genetlink.h>

int vsim_set_ifname(struct sk_buff *skb, struct genl_info *info);
int vsim_reset_ifname(struct sk_buff *skb, struct genl_info *info);
int vsim_reset_counter(struct sk_buff *skb, struct genl_info *info);
int vsim_get_flow(struct sk_buff *skb, struct genl_info *info);
int vsim_init(void);
#endif	/* __HWRIL_VSIM_H */
