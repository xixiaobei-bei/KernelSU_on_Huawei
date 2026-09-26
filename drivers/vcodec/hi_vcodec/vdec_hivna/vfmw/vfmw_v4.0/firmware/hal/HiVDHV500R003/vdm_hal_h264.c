/*
 * vdm_hal_h264.h
 *
 * This is vdec hal for h264.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
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

#include "vdm_hal_h264.h"

#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 h264hal_check_head_msg(mem_buffer_s *vdh_mem_map, UADDR pic_msg_slot_phy)
{
	UINT32 *slot = NULL;
	UINT32 tmp, i, offset;
	SINT32 ret;

	vdmhal_assert_ret(
		vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr != NULL,
		"can not map head msg virtual address");

	slot =
		(UINT32 *)(vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr +
		(DN_MSG_HEAD_SLOT_INDEX * H264_MSG_SLOT_RATIO * USE_MSG_SLOT_SIZE));

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

	// D3-D34
	for (i = 0; i < MAX_REF_FRAME_NUM; i++) {
		offset = 2 * i + 3;
		rd_msgword(slot + offset, tmp);
		tmp = hw_addr_lshift(tmp);
		ret = check_frm_buf_addr(tmp, vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"HeadYPhyAddr:Apc2Idc is failed");

		offset = 2 * i + 4;
		rd_msgword(slot + offset, tmp);
		tmp = hw_addr_lshift(tmp);
		ret = check_frm_buf_addr(tmp, vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"HeadCPhyAddr:Apc2Idc is failed");
	}

	return VDMHAL_OK;
}

static SINT32 h264hal_setpic_addrmsg(vdmhal_hwmem_s *hw_mem,
	UADDR fst_slc_msg_slot_phy, mem_buffer_s *vdh_mem_map,
	UINT32 *pic_height_mb, UINT32 *pic_width_mb)
{
	UINT32 *slot = NULL;
	UINT32 i, tmp;
	SINT32 ret;

	vdmhal_assert_ret(
		vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr != NULL,
		"can not map down msg virtual address\n");

	slot =
		(UINT32 *)(vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr +
		(DN_MSG_SLOT_INDEX * H264_MSG_SLOT_RATIO * USE_MSG_SLOT_SIZE));

	// D1
	rd_msgword(slot + 0x1, tmp);
	tmp = hw_addr_lshift(tmp);
	ret = check_frm_buf_addr(tmp, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D1 is failed");

	// D5
	tmp = align_up(hw_mem->sed_top_addr, MSG_SLOT_ALIGN_BYTE);
	tmp = hw_addr_rshift(tmp);
	wr_msgword(slot + 0x5, tmp);

	// D6
	tmp = align_up(hw_mem->pmv_top_addr, MSG_SLOT_ALIGN_BYTE);
	tmp = hw_addr_rshift(tmp);
	wr_msgword(slot + 0x6, tmp);

	rd_msgword(slot + 0x7, tmp);
	tmp = hw_addr_lshift(tmp);
	ret = check_pmv_buf_addr(tmp, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D7 is failed");

	// D8
	tmp = align_up(hw_mem->rcn_top_addr, MSG_SLOT_ALIGN_BYTE);
	tmp = hw_addr_rshift(tmp);
	wr_msgword(slot + 0x8, tmp);

	// D9
	tmp = align_up(fst_slc_msg_slot_phy, MSG_SLOT_ALIGN_BYTE);
	tmp = hw_addr_rshift(tmp);
	wr_msgword(slot + 0x9, tmp);

	// D26
	tmp = align_up(hw_mem->h264_mn_addr, MSG_SLOT_ALIGN_BYTE);
	tmp = hw_addr_rshift(tmp);
	wr_msgword(slot + 26, tmp);

	// D10 - D25
	for (i = 0; i < MAX_REF_FRAME_NUM; i++) {
		rd_msgword(slot + 10 + i, tmp);
		tmp = hw_addr_lshift(tmp);
		ret = check_frm_buf_addr(tmp, vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK, "HeadYPhyAddr is failed");
	}

	// D59
	tmp = align_up(hw_mem->dblk_top_addr, MSG_SLOT_ALIGN_BYTE);
	tmp = hw_addr_rshift(tmp);
	wr_msgword(slot + 59, tmp);

	return 0;
}
#endif

void handle_other_vreg(UINT32 d32, omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	((ystaddr_1d *)(&d32))->ystaddr_1d =
		align_down_hal(vdh_reg_cfg->vdh_yst_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_YSTADDR_1D, d32, 0);
	dprint(PRN_VDMREG, "ystaddr_1d : %pK\n", (void *)(uintptr_t)d32);

	write_vreg(VREG_YSTRIDE_1D, vdh_reg_cfg->vdh_ystride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", vdh_reg_cfg->vdh_ystride);

	write_vreg(VREG_UVOFFSET_1D, vdh_reg_cfg->vdh_uv_offset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", vdh_reg_cfg->vdh_uv_offset);

	d32 = 0;
	write_vreg(VREG_HEAD_INF_OFFSET, d32, 0);

	write_vreg(VREG_REF_PIC_TYPE, vdh_reg_cfg->vdh_ref_pic_type, 0);
	dprint(PRN_VDMREG, "ref_pic_type : 0x%x\n", vdh_reg_cfg->vdh_ref_pic_type);

	if (vdh_reg_cfg->vdh_ff_apt_en == 0x2)
		d32 = 0x2;
	else
		d32 = 0x0;
	write_vreg(VREG_FF_APT_EN, d32, 0);
	dprint(PRN_VDMREG, "ff_apt_en : 0x%x\n", d32);

	// UVSTRIDE_1D
	write_vreg(VREG_UVSTRIDE_1D, vdh_reg_cfg->vdh_uvstride, 0);
	dprint(PRN_VDMREG, "UVSTRIDE_1D = 0x%x\n", vdh_reg_cfg->vdh_uvstride);

	// CFGINFO_ADDR
	write_vreg(VREG_CFGINFO_ADDR, hw_addr_rshift(
		vdh_reg_cfg->vdh_cfg_info_addr), 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%pK\n",
		(void *)(uintptr_t)(UINT32)(hw_addr_rshift(
		vdh_reg_cfg->vdh_cfg_info_addr)));

	// DDR_INTERLEAVE_MODE
	d32 = 0x03;
	write_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);
}

void handle_vreg(UINT32 d32, omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	((avm_addr *)(&d32))->av_msg_addr =
		align_down_hal(vdh_reg_cfg->vdh_avm_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_AVM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "avm_addr : %pK\n", (void *)(uintptr_t)d32);

	d32 = 0;
	((vam_addr *)(&d32))->va_msg_addr =
		align_down_hal(vdh_reg_cfg->vdh_vam_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_VAM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "vam_addr : %pK\n", (void *)(uintptr_t)d32);

	d32 = 0;
	((stream_base_addr *)(&d32))->stream_base_addr =
		align_down_hal(vdh_reg_cfg->vdh_stream_base_addr,
		MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_STREAM_BASE_ADDR, d32, 0);
	dprint(PRN_VDMREG, "stream_base_addr : %pK\n", (void *)(uintptr_t)d32);

	d32 = rd_scd_reg(REG_EMAR_ID);
	if (vdh_reg_cfg->vdh_emar_id == 0)
		d32 = d32 & (~(0x100));
	else
		d32 = d32 | 0x100;
	wr_scd_reg(REG_EMAR_ID, d32);

	d32 = 0x00300C03;
	write_vreg(VREG_SED_TO, d32, 0);
	write_vreg(VREG_ITRANS_TO, d32, 0);
	write_vreg(VREG_PMV_TO, d32, 0);
	write_vreg(VREG_PRC_TO, d32, 0);
	write_vreg(VREG_RCN_TO, d32, 0);
	write_vreg(VREG_DBLK_TO, d32, 0);
	write_vreg(VREG_PPFD_TO, d32, 0);

	d32 = 0;
	handle_other_vreg(d32, vdh_reg_cfg);
}

void handle_basic_cfg1(UINT32 d32, omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	write_vreg(VREG_BASIC_CFG0, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg0 : 0x%x\n", d32);

	d32 = 0;
	((basic_cfg1 *)(&d32))->video_standard       = 0x0;
	((basic_cfg1 *)(&d32))->fst_slc_grp          =
	((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->fst_slc_grp;
	((basic_cfg1 *)(&d32))->mv_output_en         =
		((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->mv_output_en;
	((basic_cfg1 *)(&d32))->uv_order_en          =
		((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->uv_order_en;
	((basic_cfg1 *)(&d32))->vdh_2d_en            =
		((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->vdh_2d_en;
	((basic_cfg1 *)(&d32))->max_slcgrp_num       = 2;
	((basic_cfg1 *)(&d32))->compress_en          =
		((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->compress_en;
	((basic_cfg1 *)(&d32))->ppfd_en              = 0;
	((basic_cfg1 *)(&d32))->line_num_output_en   = 0;
	write_vreg(VREG_BASIC_CFG1, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg1 : 0x%x\n", d32);
	d32 = 0;
	handle_vreg(d32, vdh_reg_cfg);
}


#ifdef MSG_POOL_ADDR_CHECK
SINT32 h264hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map)
#else
SINT32 h264hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg)
#endif
{
	UINT32 d32;
#ifdef MSG_POOL_ADDR_CHECK
	SINT32 ret;
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;
	vdmhal_hwmem_s *hw_mem = &g_hw_mem[0];
#endif

	dprint(PRN_VDMREG,
	"\n***********************h264hal_startdec ***********************\n");
#ifdef MSG_POOL_ADDR_CHECK
	if (vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "vdh_mem_map is null\n");
		return VDMHAL_ERR;
	}

	if (((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->compress_en) {
		ret = h264hal_check_head_msg(vdh_mem_map,
			hw_mem->msg_slot_addr[DN_MSG_HEAD_SLOT_INDEX]);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"H264 head msg check failed");
	}

	ret = h264hal_setpic_addrmsg(hw_mem,
		hw_mem->msg_slot_addr[DN_MSG_SLOT_INDEX + 1],
		vdh_mem_map, &pic_height_mb, &pic_width_mb);
	vdmhal_assert_ret(ret == VDMHAL_OK, "H264 pic msg config failed");
#endif
	d32 = 0;
	((basic_cfg0 *)(&d32))->mbamt_to_dec         =
		((basic_cfg0 *)(&vdh_reg_cfg->vdh_basic_cfg0))->mbamt_to_dec;
	((basic_cfg0 *)(&d32))->load_qmatrix_flag    = 1;
	((basic_cfg0 *)(&d32))->marker_bit_detect_en = 0;
	((basic_cfg0 *)(&d32))->ac_last_detect_en    = 0;
	((basic_cfg0 *)(&d32))->coef_idx_detect_en   = 1;
	((basic_cfg0 *)(&d32))->vop_type_detect_en   = 0;
#ifdef ENV_SOS_KERNEL
	((basic_cfg0 *)(&d32))->sec_mode_en          = 1;
#else
	((basic_cfg0 *)(&d32))->sec_mode_en          = 0;
#endif
	handle_basic_cfg1(d32, vdh_reg_cfg);
	return VDMHAL_OK;
}

