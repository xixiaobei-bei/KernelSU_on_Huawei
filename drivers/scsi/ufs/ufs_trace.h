/*
 * ufs_trace.h
 *
 * ufs device error debug
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#ifndef __UFS_TRACE_H
#define __UFS_TRACE_H

#include "ufs.h"
#include "ufshcd.h"

#define RDR_MODID_MMC_START HISI_BB_MOD_EMMC_START
#define RDR_MODID_UFSDEV_PANIC          0x81ffff03
#define RDR_MODID_MMC_END HISI_BB_MOD_EMMC_END

#if defined(UFS_TRACE_FLAG) && defined(CONFIG_HUAWEI_UFS_TRACE)
void ufs_trace_fd_init(void);
void ufs_rdr_hardware_err(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
#else
static inline void ufs_trace_fd_init(void){ return; };
static inline void ufs_rdr_hardware_err(struct ufs_hba *hba, struct ufshcd_lrb *lrbp){ return; };
#endif


#endif
