/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_OVERLAY_UTILS_H
#define HISI_OVERLAY_UTILS_H

#if defined(CONFIG_DRMDRIVER)
#include <linux/hisi/hisi_drmdriver.h>
#endif
#include "../hisi_fb.h"
#include "../hisi_dss_smmu_base.h"
#if defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V360)
#include <soc_media1_crg_interface.h>
#include <soc_actrl_interface.h>
#include <soc_pmctrl_interface.h>
#include <soc_smmuv3_tbu_interface.h>
#endif

#include "hisi_dss_dump.h"
#include "hisi_dss_tool.h"
#include "hisi_dss_init.h"
#include "hisi_dss_handler.h"
#include "hisi_dss_dirty.h"
#include "hisi_dss_clip.h"
#include "hisi_dss_dfc.h"
#include "hisi_dss_mctl.h"
#include "hisi_dss_mif.h"
#include "hisi_dss_ovl.h"
#include "hisi_dss_rdma.h"
#include "hisi_dss_wdma.h"
#include "hisi_dss_scl.h"
#include "hisi_overlay_utils_platform.h"

#if defined(CONFIG_RCH_CLD)
#include "hisi_dss_cld.h"
#endif

#define P3_PROCESS_NEEDED_LAYER (1U << 31);

extern unsigned int g_dss_smmu_outstanding;
extern int g_enable_dirty_region_updt;
extern void *g_smmu_rwerraddr_virt;

int hisi_dss_ch_module_set_regs(struct dpu_fb_data_type *dpufd, int32_t mctl_idx,
	int chn_idx, uint32_t wb_type, bool enable_cmdlist);

int hisi_dss_ov_module_set_regs(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req, int ovl_idx,
	struct ov_module_set_regs_flag ov_module_flag);

int hisi_dss_prev_module_set_regs(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, uint32_t cmdlist_pre_idxs, bool enable_cmdlist, int *use_comm_mmbuf);
bool hisi_dss_check_reg_reload_status(struct dpu_fb_data_type *dpufd);
bool hisi_dss_check_crg_sctrl_status(struct dpu_fb_data_type *dpufd);
void hisi_ldi_underflow_handle_func(struct work_struct *work);
int hisi_dss_get_ov_data_from_user(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, const void __user *argp);
uint64_t hisi_dss_get_vsync_timediff(struct dpu_fb_data_type *dpufd);
int hisi_dss_check_vsync_timediff(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req);
bool csc_need_p3_process(uint32_t csc_mode);
bool hisi_dss_is_async_play(struct dpu_fb_data_type *dpufd);

#endif /* HISI_OVERLAY_UTILS_H */
