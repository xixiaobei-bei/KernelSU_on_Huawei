/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _HISI_DSS_MNTN_H_
#define _HISI_DSS_MNTN_H_

#include "hisi_dss_mntn_log.h"
#include "mntn_public_interface.h"

#define DSS_LOG_NAME "dss_dump_reg.txt"
#define DSS_LOG_PATH_MAXLEN 128
#define DSS_FILESYS_DEFAULT_MODE 0770
#define DSS_REG_NAME_LEN 15
// max reg dump regions
#define DSS_MAX_REG_DUMP_REGIONS 10
// max dump number in a region
#define DSS_MAX_REG_DUMP_NUM 100

struct dss_regs_info {
	char reg_name[DSS_REG_NAME_LEN];
	u32 reg_size;
	u64 reg_base;
	void __iomem *reg_map_addr;
	unsigned char *reg_dump_addr;
};

enum hisi_dss_module_id {
	MODID_DSS_NOC_EXCEPTION = HISI_BB_MOD_DSS_START,
	MODID_DSS_DDRC_EXCEPTION,
	MODID_DSS_EXCEPTION_END = HISI_BB_MOD_DSS_END,
};

#endif
