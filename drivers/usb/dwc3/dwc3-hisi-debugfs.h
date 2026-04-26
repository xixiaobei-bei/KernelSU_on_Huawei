/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Headfile for debugfs.c
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _DWC3_CHIP_DEBUGFS_H_
#define _DWC3_CHIP_DEBUGFS_H_

#include <linux/fs.h>
#include "core.h"

#ifdef CONFIG_HISI_DEBUG_FS
void dwc3_chip_debugfs_init(struct dwc3 *dwc, struct dentry *root);
void dwc3_chip_debugfs_exit(void);
int dwc3_is_test_noc_err(void);
uint32_t dwc3_get_noc_err_addr(uint32_t addr);
#else
static inline void dwc3_chip_debugfs_init(struct dwc3 *dwc, struct dentry *root) {}
static inline void dwc3_chip_debugfs_exit(void) {}
static inline int dwc3_is_test_noc_err(void)
{
	return 0;
}

static inline uint32_t dwc3_get_noc_err_addr(uint32_t addr)
{
	return 0;
}
#endif

#endif /* _DWC3_CHIP_DEBUGFS_H_ */
