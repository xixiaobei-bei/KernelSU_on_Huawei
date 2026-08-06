/*
 * hal_venc.h
 *
 * This is for venc register config.
 *
 * Copyright (c) 2010-2020 Huawei Technologies CO., Ltd.
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

#ifndef __HAL_VENC_H__
#define __HAL_VENC_H__

#include "hi_type.h"
#include "drv_venc_ioctl.h"

#define VENC_COMPATIBLE       "hisilicon,HiVCodecV200-venc"
#define VENC_ES_SUPPORT
#define VENC_CS_SUPPORT
#define VENC_FPGAFLAG_ES      "fpga_flag_es"
#define VENC_FPGAFLAG_CS      "fpga_flag_cs"
#define VENC_PCTRL_PERI_ES    0xE8A090A4
#define VENC_PCTRL_PERI_CS    0xE8A090BC
#define VENC_EXISTBIT_ES      0x4
#define VENC_EXISTBIT_CS      0x40000

void venc_hal_clr_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_disable_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_start_encode(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_get_reg_venc(struct stream_info *stream_info, uint32_t *reg_base);
void vedu_hal_cfg_reg_intraset(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_lambda_set(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_qpg_map_set(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_addr_set(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_slc_head_set(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_smmu_set(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_premmu_set(const struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_reg_simple(const struct encode_info *channel_cfg, int32_t core_id);
void vedu_hal_cfg_reg(const struct encode_info *regcfginfo, int32_t core_id);
void venc_hal_set_smmu_addr(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_get_slice_reg(struct stream_info *stream_info, uint32_t *reg_base);

#endif
