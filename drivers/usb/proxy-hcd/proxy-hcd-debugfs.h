/*
 * proxy-hcd-debugfs.h
 *
 * utilityies for proxy-hcd debugging
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

#ifndef _PROXY_HCD_DEBUGFS_H_
#define _PROXY_HCD_DEBUGFS_H_

#include "proxy-hcd.h"

#ifdef CONFIG_HISI_DEBUG_FS
int phcd_debugfs_init(struct proxy_hcd *phcd);
void phcd_debugfs_exit(struct proxy_hcd *phcd);
#else
static inline int phcd_debugfs_init(struct proxy_hcd *phcd) {return 0; }
static inline void phcd_debugfs_exit(struct proxy_hcd *phcd) {}
#endif /* CONFIG_HISI_DEBUG_FS */

#endif
