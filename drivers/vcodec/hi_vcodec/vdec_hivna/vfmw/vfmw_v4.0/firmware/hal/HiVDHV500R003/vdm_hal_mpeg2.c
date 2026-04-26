/*
 * vdm_hal_mpeg2.c
 *
 * This is vdec hal for mp2.
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

#ifndef __VDM_HAL_MPEG2_C__
#define __VDM_HAL_MPEG2_C__

#include    "vdm_hal_mpeg2.h"

#include    "public.h"
#include    "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 mp2hal_cfg_dnmsg(vdmhal_hwmem_s *hw_mem,
							   mem_buffer_s *vdh_mem_map,
							   UINT32 *pic_height_mb,
							   UINT32 *pic_width_mb)
{
	UINT32 *msg_base = NULL;
	UINT32 d32;
	UADDR slc_dnmsg_phyaddr;
	SINT32 ret;

	vdmhal_assert_ret(
		vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr != NULL,
					  "can not map down msg virtual address");

	msg_base =
		(UINT32 *)(vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr +
		(DN_MSG_SLOT_INDEX * SLOT_MSG_RATIO * USE_MSG_SLOT_SIZE));

	rd_msgword(msg_base + 4, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_frm_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D4 is failed");

	rd_msgword(msg_base + 5, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_frm_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D5 is failed");

	rd_msgword(msg_base + 6, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_frm_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D6 is failed");

	rd_msgword(msg_base + 7, d32);
	d32 = hw_addr_lshift(d32);
	ret = check_pmv_buf_addr(d32, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D7 is failed");

	d32 = 0;
	((mp2dnmsg_d48 *)(&d32))->pmv_top_addr = align_up(hw_mem->pmv_top_addr,
		MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	wr_msgword(msg_base + 48, d32);

	slc_dnmsg_phyaddr = align_down_hal(hw_mem->msg_slot_addr[DN_MSG_SLOT_INDEX],
		MSG_SLOT_ALIGN_BYTE) + 64 * 4;

	d32 = 0;
	((mp2dnmsg_d63 *)(&d32))->first_slc_dnmsg_addr =
		hw_addr_rshift(slc_dnmsg_phyaddr);
	wr_msgword(msg_base + 63, d32);

	return VDMHAL_OK;
}
#endif

void handle_mp2hal_cfg_reg(SINT32 d32, omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	((avm_addr *)(&d32))->av_msg_addr =
		align_down_hal(vdh_reg_cfg->vdh_avm_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_AVM_ADDR, d32, 0);

	d32 = 0;
	((vam_addr *)(&d32))->va_msg_addr =
		align_down_hal(vdh_reg_cfg->vdh_vam_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_VAM_ADDR, d32, 0);

	write_vreg(VREG_STREAM_BASE_ADDR,
		hw_addr_rshift(vdh_reg_cfg->vdh_stream_base_addr), 0);

	// EMAR_ADDR
	d32 = 0x101;
	wr_scd_reg(REG_EMAR_ID, d32);

	// TIME_OUT
	d32 = 0x00300C03;
	write_vreg(VREG_SED_TO, d32, 0);
	write_vreg(VREG_ITRANS_TO, d32, 0);
	write_vreg(VREG_PMV_TO, d32, 0);
	write_vreg(VREG_PRC_TO, d32, 0);
	write_vreg(VREG_RCN_TO, d32, 0);
	write_vreg(VREG_DBLK_TO, d32, 0);
	write_vreg(VREG_PPFD_TO, d32, 0);

	d32 = 0;
	((ystaddr_1d *)(&d32))->ystaddr_1d =
		align_down_hal(vdh_reg_cfg->vdh_yst_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_YSTADDR_1D, d32, 0);
	write_vreg(VREG_YSTRIDE_1D, vdh_reg_cfg->vdh_ystride, 0);
	write_vreg(VREG_UVOFFSET_1D, vdh_reg_cfg->vdh_uv_offset, 0);

	d32 = 0;
	((ref_pic_type *)(&d32))->ref_pic_type_0 =
		((ref_pic_type *)(&vdh_reg_cfg->vdh_ref_pic_type))->ref_pic_type_0;
	((ref_pic_type *)(&d32))->ref_pic_type_1 =
		((ref_pic_type *)(&vdh_reg_cfg->vdh_ref_pic_type))->ref_pic_type_1;
	write_vreg(VREG_REF_PIC_TYPE, d32, 0);
	d32 = 0;
	((ff_apt_en *)(&d32))->ff_apt_en = 0;
	write_vreg(VREG_FF_APT_EN, d32, 0);

	// VREG_UVSTRIDE_1D
	write_vreg(VREG_UVSTRIDE_1D, vdh_reg_cfg->vdh_uvstride, 0);

	// DDR_INTERLEAVE_MODE
	d32 = 0x03;
	write_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);
}

static SINT32 mp2hal_cfg_reg(omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	UINT32 d32 = 0;
	((basic_cfg0 *)(&d32))->mbamt_to_dec         =
		((basic_cfg0 *)(&vdh_reg_cfg->vdh_basic_cfg0))->mbamt_to_dec;
	((basic_cfg0 *)(&d32))->load_qmatrix_flag    = 1;
	((basic_cfg0 *)(&d32))->marker_bit_detect_en = 1;
	((basic_cfg0 *)(&d32))->ac_last_detect_en    = 0;
	((basic_cfg0 *)(&d32))->coef_idx_detect_en   = 1;
	((basic_cfg0 *)(&d32))->vop_type_detect_en   = 0;
#ifdef ENV_SOS_KERNEL
	((basic_cfg0 *)(&d32))->sec_mode_en          = 1;
#else
	((basic_cfg0 *)(&d32))->sec_mode_en          = 0;
#endif
	write_vreg(VREG_BASIC_CFG0, d32, 0);

	d32 = 0;
	((basic_cfg1 *)(&d32))->video_standard       = 0x3;
	((basic_cfg1 *)(&d32))->fst_slc_grp          = 1;
	((basic_cfg1 *)(&d32))->mv_output_en         = 1;
	((basic_cfg1 *)(&d32))->uv_order_en          =
		((basic_cfg1 *)(&vdh_reg_cfg->vdh_basic_cfg1))->uv_order_en;
	((basic_cfg1 *)(&d32))->vdh_2d_en            = 1;
	((basic_cfg1 *)(&d32))->max_slcgrp_num       = 3;
	((basic_cfg1 *)(&d32))->ppfd_en              = 0;
	((basic_cfg1 *)(&d32))->line_num_output_en   = 0;
	((basic_cfg1 *)(&d32))->compress_en          = 0;
	/* set uv order 0: v first; 1: u first */
	write_vreg(VREG_BASIC_CFG1, d32, 0);

	d32 = 0;
	handle_mp2hal_cfg_reg(d32, vdh_reg_cfg);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map)
#else
SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg)
#endif
{
#ifdef MSG_POOL_ADDR_CHECK
	vdmhal_hwmem_s *hw_mem = &g_hw_mem[0];
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;

	if (vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "vdh_mem_map is null\n");
		return VDMHAL_ERR;
	}
	if (mp2hal_cfg_dnmsg(hw_mem, vdh_mem_map, &pic_height_mb, &pic_width_mb) != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP2 DnMsg config failed\n");
		return VDMHAL_ERR;
	}
#endif
	if (vdh_reg_cfg == NULL) {
		dprint(PRN_FATAL, "vdh_reg_cfg is null\n");
		return VDMHAL_ERR;
	}
	if (mp2hal_cfg_reg(vdh_reg_cfg) != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP2 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
