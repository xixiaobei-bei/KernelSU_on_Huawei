/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#ifndef __DPTX_CORE_H__
#define __DPTX_CORE_H__

#include "../hisi_dp.h"

/**
 * Core interface functions
 */
void dptx_global_intr_en(struct dp_ctrl *dp);
void dptx_global_intr_dis(struct dp_ctrl *dp);
void dptx_global_intr_clear(struct dp_ctrl *dp);
int dptx_core_init(struct dp_ctrl *dptx);
bool dptx_check_dptx_id(struct dp_ctrl *dptx);
void dptx_core_init_phy(struct dp_ctrl *dptx);
int dptx_core_deinit(struct dp_ctrl *dptx);
void dptx_aux_disreset(struct dp_ctrl *dptx, bool enable);
void dptx_soft_reset(struct dp_ctrl *dptx, uint32_t bits);
void dptx_soft_reset_all(struct dp_ctrl *dptx);
void dptx_phy_soft_reset(struct dp_ctrl *dptx);
void dptx_typec_reset_ack(struct dp_ctrl *dptx);
/**
 * PHY IF Control
 */
void dptx_phy_set_lanes_status(struct dp_ctrl *dptx, bool bopen);
void dptx_phy_set_lanes(struct dp_ctrl*dptx, uint32_t num);
int dptx_phy_get_lanes(struct dp_ctrl *dptx);
void dptx_phy_set_rate(struct dp_ctrl *dptx, uint32_t rate);
int dwc_phy_get_rate(struct dp_ctrl *dptx);
int dptx_phy_wait_busy(struct dp_ctrl *dptx, uint32_t lanes);
void dptx_phy_set_pre_emphasis(struct dp_ctrl *dptx,
	uint32_t lane, uint32_t level);
void dptx_phy_set_vswing(struct dp_ctrl *dptx,
	uint32_t lane, uint32_t level);
void dptx_phy_enable_xmit(struct dp_ctrl *dptx,
	uint32_t lane, bool enable);

int dptx_phy_rate_to_bw(uint8_t rate);
int dptx_bw_to_phy_rate(uint8_t bw);
void dptx_phy_set_pattern(struct dp_ctrl *dptx, uint32_t pattern);
void dptx_enable_ssc(struct dp_ctrl *dptx);
bool dptx_sink_enabled_ssc(struct dp_ctrl *dptx);
#ifdef CONFIG_HISI_FB_V510
void dptx_init_hwparams(struct dp_ctrl *dptx);
#endif

#endif
