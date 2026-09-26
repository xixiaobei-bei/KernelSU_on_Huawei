/*
 * vdm_hal_vp9.c
 *
 * This is VDMV300 hardware abstraction.
 *
 * Copyright (c) 2001-2020 Huawei Technologies CO., Ltd.
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

#ifndef __VDM_HAL_VP9_C__
#define __VDM_HAL_VP9_C__

#include "vdm_hal_vp9.h"
#include "vdm_hal.h"
#include "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 vp9hal_check_headmsg(mem_buffer_s *vdh_mem_map, UADDR pic_msg_slot_phy)
{
	UINT32 *slot = NULL;
	UINT32 *slot_base = NULL;
	UINT32 tmp, i, offset;
	SINT32 ret;

	vdmhal_assert_ret(
		vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr != NULL,
			  "can not map head msg virtual address");

	slot_base =
		(UINT32 *)(vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr +
		(DN_MSG_HEAD_SLOT_INDEX  * VP9_MSG_SLOT_RATIO * USE_MSG_SLOT_SIZE));
	slot = slot_base;

	// D0
	rd_msgword(slot, tmp);
	tmp = hw_addr_lshift(tmp);
	ret = check_frm_buf_addr(tmp, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "HeadYPhyAddr is failed");

	// D1
	rd_msgword(slot + 1, tmp);
	tmp = hw_addr_lshift(tmp);
	ret = check_frm_buf_addr(tmp, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "HeadCPhyAddr is failed");

	// D3,D4,D5,D6,D7,D8
	for (i = 0; i < 3; i++) {
		offset = 2 * i + 3;
		rd_msgword(slot + offset, tmp);
		tmp = hw_addr_lshift(tmp);
		ret = check_frm_buf_addr(tmp, vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"HeadYPhyAddr Apc2Idc is failed");
		offset = 2 * i + 4;
		rd_msgword(slot + offset, tmp);
		tmp = hw_addr_lshift(tmp);
		ret = check_frm_buf_addr(tmp, vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"HeadCPhyAddr Apc2Idc is failed");
	}

	return VDMHAL_OK;
}

/* vp9 picmsg */
static SINT32 vp9hal_cfgpic_msg(vdmhal_hwmem_s *hw_mem, mem_buffer_s *vdh_mem_map,
							    UINT32 *pic_height_mb, UINT32 *pic_width_mb)
{
	UINT32 d32 = 0;
	UINT32 *slot = NULL;
	SINT32 ret;

	vdmhal_assert_ret(
		vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr != NULL,
		"can not map down msg virtual address");

	slot =
		(UINT32 *)(vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr +
		(DN_MSG_SLOT_INDEX * VP9_MSG_SLOT_RATIO * USE_MSG_SLOT_SIZE));

	// D33
	rd_msgword(slot + 33, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_frm_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D33 is failed");

	// D34
	rd_msgword(slot + 34, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_frm_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D34 is failed");

	// D35
	rd_msgword(slot + 35, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_frm_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D35 is failed");

	// D36
	d32 = align_down_hal(hw_mem->sed_top_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 36, d32);

	// D37
	d32 = align_down_hal(hw_mem->pmv_top_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 37, d32);

	// D38
	d32 = align_down_hal(hw_mem->rcn_top_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 38, d32);

	// D39
	d32 = align_down_hal(hw_mem->vp9_prob_tab_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 39, d32);

	// D40
	d32 = align_down_hal(hw_mem->dblk_top_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 40, d32);

	// D41
	d32 = align_down_hal(hw_mem->vp9_prob_cnt_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 41, d32);

	// D42
	rd_msgword(slot + 42, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_pmv_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D42 is failed");

	// D43
	d32 = align_down_hal(hw_mem->dblk_left_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 43, d32);

	// D63
	d32 = align_down_hal(hw_mem->msg_slot_addr[DN_MSG_SLOT_INDEX + 1],
		MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(slot + 63, d32);

	return VDMHAL_OK;
}
#endif

void v9hal_basic_cfg(UINT32 d32, omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	((basic_cfg0 *)(&d32))->mbamt_to_dec      =
		((basic_cfg0 *)(&vdh_reg_cfg->vdh_basic_cfg0))->mbamt_to_dec;
	((basic_cfg0 *)(&d32))->load_qmatrix_flag = 0;

	write_vreg(VREG_BASIC_CFG0, d32, 0);
	dprint(PRN_VDMREG, "VREG_BASIC_CFG0 : 0x%x\n", d32);

	d32 = 0;
	((vp9_basic_cfg1 *)(&d32))->video_standard     = 0x0E;

	((vp9_basic_cfg1 *)(&d32))->uv_order_en        =
		((vp9_basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->uv_order_en;
	((vp9_basic_cfg1 *)(&d32))->fst_slc_grp        = 1;
	((vp9_basic_cfg1 *)(&d32))->mv_output_en       = 1;
	((vp9_basic_cfg1 *)(&d32))->max_slcgrp_num     = 3;
	((vp9_basic_cfg1 *)(&d32))->line_num_output_en = 0;

	((vp9_basic_cfg1 *)(&d32))->vdh_2d_en          =
		((vp9_basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->vdh_2d_en;
	// for tmp linear
	((vp9_basic_cfg1 *)(&d32))->frm_cmp_en         =
		((vp9_basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->frm_cmp_en;
	((vp9_basic_cfg1 *)(&d32))->ppfd_en            = 0;

	write_vreg(VREG_BASIC_CFG1, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg1 : 0x%x\n", d32);
}

static SINT32 vp9hal_cfgreg(omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	UINT32 d32 = 0;
	v9hal_basic_cfg(d32, vdh_reg_cfg);
	// avm_addr
	d32 = 0;
	((avm_addr *)(&d32))->av_msg_addr =
		align_down_hal(vdh_reg_cfg->vdh_avm_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_AVM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "avm_addr : %pK\n", (void *)(uintptr_t)d32);

	// vam_addr
	d32 = 0;
	((vam_addr *)(&d32))->va_msg_addr =
		align_down_hal(vdh_reg_cfg->vdh_vam_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_VAM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "vam_addr : %pK\n", (void *)(uintptr_t)d32);

	// stream_base_addr
	d32 = align_down_hal(vdh_reg_cfg->vdh_stream_base_addr,
	MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_STREAM_BASE_ADDR, d32, 0);
	dprint(PRN_VDMREG, "stream_base_addr : %pK\n", (void *)(uintptr_t)d32);

	// PRC_CACHE_TYPE
	d32 = 0x0;
	write_vreg(VREG_FF_APT_EN, d32, 0);
	dprint(PRN_VDMREG, "VREG_FF_APT_EN : 0x%x\n", d32);

	// EMAR_ADDR
	d32 = 0x101;
	wr_scd_reg(REG_EMAR_ID, d32);

	// TIME_OUT
	d32 = 0x00300C03;
	write_vreg(VREG_SED_TO,    d32, 0);
	write_vreg(VREG_ITRANS_TO, d32, 0);
	write_vreg(VREG_PMV_TO,    d32, 0);
	write_vreg(VREG_PRC_TO,    d32, 0);
	write_vreg(VREG_RCN_TO,    d32, 0);
	write_vreg(VREG_DBLK_TO,   d32, 0);
	write_vreg(VREG_PPFD_TO,   d32, 0);

	// DEC_OVER_INT_LEVEL
	d32 = 60;
	write_vreg(VREG_PART_DEC_OVER_INT_LEVEL, d32, 0);
	dprint(PRN_VDMREG, "VREG_PART_DEC_OVER_INT_LEVEL:0x%x\n", d32);

	// ystaddr_1d
	d32 = 0;
	((ystaddr_1d *)(&d32))->ystaddr_1d =
		align_down_hal(vdh_reg_cfg->vdh_yst_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_YSTADDR_1D, d32, 0);
	dprint(PRN_VDMREG, "ystaddr_1d : %pK\n", (void *)(uintptr_t)d32);

	// YSTRIDE_1D
	write_vreg(VREG_YSTRIDE_1D, vdh_reg_cfg->vdh_ystride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", vdh_reg_cfg->vdh_ystride);

	// UVOFFSET_1D
	write_vreg(VREG_UVOFFSET_1D, vdh_reg_cfg->vdh_uv_offset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", vdh_reg_cfg->vdh_uv_offset);

	// UVSTRIDE_1D
	write_vreg(VREG_UVSTRIDE_1D, vdh_reg_cfg->vdh_uvstride, 0);
	dprint(PRN_VDMREG, "UVSTRIDE_1D : 0x%x\n", vdh_reg_cfg->vdh_uvstride);

	// CFGINFO_ADDR
	d32 = vdh_reg_cfg->vdh_cfg_info_addr;
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_CFGINFO_ADDR, d32, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%pK\n",
		(void *)(uintptr_t)d32);

	// DDR_INTERLEAVE_MODE
	d32 = 0x3;
	write_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map)
#else
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg)
#endif
{
	SINT32 ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;
	vdmhal_hwmem_s *hw_mem = &g_hw_mem[0];

	if (vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "vdh_mem_map is null\n");
		return VDMHAL_ERR;
	}
	if (((vp9_basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->frm_cmp_en) {
		ret = vp9hal_check_headmsg(vdh_mem_map,
			hw_mem->msg_slot_addr[DN_MSG_HEAD_SLOT_INDEX]);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"VP9 head msg check failed");
	}

	ret = vp9hal_cfgpic_msg(hw_mem, vdh_mem_map, &pic_height_mb, &pic_width_mb);
	vdmhal_assert_ret(ret == VDMHAL_OK, "VP9 picture msg config failed");
#endif
	ret = vp9hal_cfgreg(vdh_reg_cfg);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "VP9 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
