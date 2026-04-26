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
#include "smmu.h"
#include "hal_common.h"

#ifdef HIVCODECV520
#define VENC_COMPATIBLE "hisilicon,HiVCodecV520-venc"
#else
#define VENC_COMPATIBLE "hisilicon,HiVCodecV500-venc"  // reused for hivcodecv510 v500
#endif

#define VENC_CS_SUPPORT
#define VENC_FPGAFLAG_CS      "venc_fpga"
#define VENC_PCTRL_PERI_CS    0xFEC3E0BC
#define VENC_EXISTBIT_CS      0x08

void venc_hal_clr_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_disable_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_start_encode(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_get_reg_venc(struct stream_info *stream_info, uint32_t *reg_base);
void venc_hal_get_reg_stream_len(struct stream_info *stream_info, uint32_t *reg_base);
void venc_hal_cfg_curld_osd01(struct encode_info *channel_cfg, uint32_t *reg_base);
void vedu_hal_cfg_smmu(struct encode_info *channel_cfg, uint32_t core_id);
void vedu_hal_set_int(uint32_t *reg_base);
void vedu_hal_cfg(volatile uint32_t *base, uint32_t offset, uint32_t value);
void venc_hal_get_slice_reg(struct stream_info *stream_info, uint32_t *reg_base);

#endif
