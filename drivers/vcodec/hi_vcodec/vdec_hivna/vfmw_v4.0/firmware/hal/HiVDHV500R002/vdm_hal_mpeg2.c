/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for mp2.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */
#ifndef __VDM_HAL_MPEG2_C__
#define __VDM_HAL_MPEG2_C__

#include    "vdm_hal_mpeg2.h"

#include    "public.h"
#include    "vdm_hal_api.h"
#include    "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 mp2hal_cfg_dnmsg(vdmhal_hwmem_s *p_hw_mem, mem_buffer_s *p_vdh_mem_map,
							   UINT32 *p_pic_height_mb, UINT32 *p_pic_width_mb)
{
	UINT32 *p_msg_base = NULL;
	UINT32 d32;
	UADDR slc_dnmsg_phyaddr;
	SINT32 ret;

	vdmhal_assert_ret(
		p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr != NULL,
		"can not map down msg virtual address");

	p_msg_base =
		(UINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr +
		(DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	d32 = 0;
	rd_msgword(p_msg_base + 4, d32);
	ret = check_frm_buf_addr(d32, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D4 is failed");

	d32 = 0;
	rd_msgword(p_msg_base + 5, d32);
	ret = check_frm_buf_addr(d32, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D5 is failed");

	d32 = 0;
	rd_msgword(p_msg_base + 6, d32);
	ret = check_frm_buf_addr(d32, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D6 is failed");

	d32 = 0;
	rd_msgword(p_msg_base + 7, d32);
	ret = check_pmv_buf_addr(d32, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D7 is failed");

	d32 = 0;
	((MP2DNMSG_D48 *)(&d32))->pmv_top_addr =
		(p_hw_mem->pmv_top_addr + 0xF) & 0xFFFFFFF0;
	wr_msgword(p_msg_base + 48, d32);

	slc_dnmsg_phyaddr =
		(p_hw_mem->msg_slot_addr[DN_MSG_SLOT_INDEX] & 0xFFFFFFF0) + 64 * 4;

	d32 = 0;
	((MP2DNMSG_D63 *)(&d32))->first_slc_dnmsg_addr = slc_dnmsg_phyaddr;
	wr_msgword(p_msg_base + 63, d32);

	return VDMHAL_OK;
}
#endif
static SINT32 mp2hal_cfgreg(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
{
	SINT32 d32;

	d32 = 0;
	((basic_cfg0 *)(&d32))->mbamt_to_dec         =
		((basic_cfg0 *)(&p_vdh_reg_cfg->vdh_basic_cfg0))->mbamt_to_dec;
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
	wr_vreg(VREG_BASIC_CFG0, d32, 0);

	d32 = 0;
	((basic_cfg1 *)(&d32))->video_standard       = 0x3;
	((basic_cfg1 *)(&d32))->fst_slc_grp          = 1;
	((basic_cfg1 *)(&d32))->mv_output_en         = 1;
	((basic_cfg1 *)(&d32))->uv_order_en          =
		((basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->uv_order_en;
	((basic_cfg1 *)(&d32))->vdh_2d_en            = 1;
	((basic_cfg1 *)(&d32))->max_slcgrp_num       = 3;
	((basic_cfg1 *)(&d32))->ppfd_en              = 0;
	((basic_cfg1 *)(&d32))->line_num_output_en   = 0;
	((basic_cfg1 *)(&d32))->compress_en          = 0;
	/*set uv order 0: v first; 1: u first */
	wr_vreg(VREG_BASIC_CFG1, d32, 0);

	d32 = 0;
	((avm_addr *)(&d32))->av_msg_addr =
		(p_vdh_reg_cfg->vdh_avm_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_AVM_ADDR, d32, 0);

	d32 = 0;
	((vam_addr *)(&d32))->va_msg_addr =
		(p_vdh_reg_cfg->vdh_vam_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_VAM_ADDR, d32, 0);

	wr_vreg(VREG_STREAM_BASE_ADDR, p_vdh_reg_cfg->vdh_stream_base_addr, 0);

	//EMAR_ADDR
	d32 = 0x101;
	wr_scd_reg(REG_EMAR_ID, d32);

	//TIME_OUT
	d32 = 0x00300C03;
	wr_vreg(VREG_SED_TO, d32, 0);
	wr_vreg(VREG_ITRANS_TO, d32, 0);
	wr_vreg(VREG_PMV_TO, d32, 0);
	wr_vreg(VREG_PRC_TO, d32, 0);
	wr_vreg(VREG_RCN_TO, d32, 0);
	wr_vreg(VREG_DBLK_TO, d32, 0);
	wr_vreg(VREG_PPFD_TO, d32, 0);

	d32 = 0;
	((ystaddr_1d *)(&d32))->ystaddr_1d =
		(p_vdh_reg_cfg->vdh_yst_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_YSTADDR_1D, d32, 0);
	wr_vreg(VREG_YSTRIDE_1D, p_vdh_reg_cfg->vdh_ystride, 0);
	wr_vreg(VREG_UVOFFSET_1D, p_vdh_reg_cfg->vdh_uv_offset, 0);

	d32 = 0;
	((ref_pic_type *)(&d32))->ref_pic_type_0 =
		((ref_pic_type *)(&p_vdh_reg_cfg->vdh_ref_pic_type))->ref_pic_type_0;
	((ref_pic_type *)(&d32))->ref_pic_type_1 =
		((ref_pic_type *)(&p_vdh_reg_cfg->vdh_ref_pic_type))->ref_pic_type_1;
	wr_vreg(VREG_REF_PIC_TYPE, d32, 0);
	d32 = 0;
	((ff_apt_en *)(&d32))->ff_apt_en = 0;//USE_FF_APT_EN;
	wr_vreg(VREG_FF_APT_EN, d32, 0);

	//VREG_UVSTRIDE_1D
	wr_vreg(VREG_UVSTRIDE_1D, p_vdh_reg_cfg->vdh_uvstride, 0);

	//DDR_INTERLEAVE_MODE
	d32 = 0x03;
	wr_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);

	return VDMHAL_OK;
}
#ifdef MSG_POOL_ADDR_CHECK
SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
#else
SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
#endif
{
#ifdef MSG_POOL_ADDR_CHECK
	vdmhal_hwmem_s *p_hw_mem = &g_hw_mem[0];
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;

	if (mp2hal_cfg_dnmsg(p_hw_mem, p_vdh_mem_map, &pic_height_mb,
		&pic_width_mb) != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP2 DnMsg config failed\n");
		return VDMHAL_ERR;
	}
#endif
	if (mp2hal_cfgreg(p_vdh_reg_cfg) != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP2 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
