/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for hevc.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#include "vdm_hal_hevc.h"

#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 hevchal_check_headmsg(mem_buffer_s *p_vdh_mem_map,
								    UINT32 pic_msg_slot_phy)
{
	UINT32 *p_slot = NULL;
	UINT32 *p_slot_base = NULL;
	UINT32 tmp, i, offset;
	SINT32 ret;

	vdmhal_assert_ret(
		p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr != NULL,
		"can not map head msg virtual address");

	p_slot_base =
		(UINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr +
		(DN_MSG_HEAD_SLOT_INDEX  * 4 * USE_MSG_SLOT_SIZE));
	p_slot = p_slot_base;

	//D0 curr frame head_ystaddr
	rd_msgword(p_slot, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D0 HeadYPhyAddr is failed");

	//D1 curr frame head_cstaddr
	rd_msgword(p_slot + 1, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D1 HeadYPhyAddr is failed");

	for (i = 0; i < MAX_REF_FRAME_NUM; i++) {
		//D(2*i + 3)
		offset = 2 * i + 3;
		rd_msgword(p_slot + offset, tmp);
		ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK, "HeadYPhyAddr is failed");
		//D(2*i + 4)
		offset = 2 * i + 4;
		rd_msgword(p_slot + offset, tmp);
		ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK, "HeadYPhyAddr is failed");
	}
	return VDMHAL_OK;
}

static SINT32 hevchal_setpic_msg(vdmhal_hwmem_s *p_hw_mem, UADDR fst_slc_msg_slot_phy,
								 mem_buffer_s *p_vdh_mem_map, UINT32 *p_pic_height,
								 UINT32 *p_pic_width)
{
	UINT32 *p_slot = NULL;
	UINT32 *p_slot_base = NULL;
	UINT32 i;
	UINT32 tmp;
	SINT32 ret;

	vdmhal_assert_ret(
		p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr != NULL,
		"can not map down msg virtual address");

	p_slot_base =
		(UINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr +
		(DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	p_slot = p_slot_base;

	// D4
	tmp = p_hw_mem->sed_top_phy_addr;
	wr_msgword(p_slot + 4, tmp);

	// D5
	tmp = p_hw_mem->pmv_top_phy_addr;
	wr_msgword(p_slot + 5, tmp);

	// D6
	rd_msgword(p_slot + 6, tmp);
	ret = check_pmv_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D6 is failed");

	// D7
	tmp = p_hw_mem->rcn_top_phy_addr;
	wr_msgword(p_slot + 7, tmp);

	for (i = 0; i < 16; i++) {
		rd_msgword(p_slot + 8 + i, tmp);
		ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
		vdmhal_assert_ret(ret == VDMHAL_OK, "D8~D24 is failed");
	}

	// D25
	tmp = p_hw_mem->mn_phy_addr;
	wr_msgword(p_slot + 25, tmp);

	// D43
	tmp = p_hw_mem->pmv_left_phy_addr;
	wr_msgword(p_slot + 43, tmp);

	//D54
	tmp = 0;
	rd_msgword(p_slot + 54, tmp);
	*p_pic_width = tmp & 0x1fff;
	*p_pic_height = (tmp >> 16) & 0x1fff;

	// D55
	tmp = p_hw_mem->tile_segment_info_phy_addr;
	wr_msgword(p_slot + 55, tmp);

	// D56
	tmp = p_hw_mem->sao_left_phy_addr;
	wr_msgword(p_slot + 56, tmp);

	// D57
	tmp = p_hw_mem->dblk_left_phy_addr;
	wr_msgword(p_slot + 57, tmp);

	// D58
	tmp = p_hw_mem->sao_top_phy_addr;
	wr_msgword(p_slot + 58, tmp);

	// D59
	tmp = p_hw_mem->dblk_top_phy_addr;
	wr_msgword(p_slot + 59, tmp);

	// D63
	tmp = fst_slc_msg_slot_phy;
	wr_msgword(p_slot + 63, tmp);

	return VDMHAL_OK;
}
#endif

static SINT32 hevchal_cfg_vdm_reg(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
{
	UINT32 d32;

	// basic_cfg0
	d32 = 0;
	((hevc_basic_cfg0 *)(&d32))->marker_bit_detect_en = 0;
	((hevc_basic_cfg0 *)(&d32))->ac_last_detect_en    = 0;
	// (run_cnt>64) check enable switch
	((hevc_basic_cfg0 *)(&d32))->coef_idx_detect_en   = 1;
	((hevc_basic_cfg0 *)(&d32))->vop_type_detect_en   = 0;
	((hevc_basic_cfg0 *)(&d32))->load_qmatrix_flag    =
		((hevc_basic_cfg0 *)(&p_vdh_reg_cfg->vdh_basic_cfg0))
			->load_qmatrix_flag;
	// enable switch:conculate luma pixel
	((hevc_basic_cfg0 *)(&d32))->luma_sum_en          = 0;
	// enable switch:conculate luma histogram
	((hevc_basic_cfg0 *)(&d32))->luma_histogram_en    = 0;
	((hevc_basic_cfg0 *)(&d32))->mbamt_to_dec         =
		((hevc_basic_cfg0 *)(&p_vdh_reg_cfg->vdh_basic_cfg0))->mbamt_to_dec;
#ifdef ENV_SOS_KERNEL
	((hevc_basic_cfg0 *)(&d32))->vdh_safe_flag         = 1;
#else
	((hevc_basic_cfg0 *)(&d32))->vdh_safe_flag         = 0;
#endif
	wr_vreg(VREG_BASIC_CFG0, d32, 0);

	// basic_cfg1
	/*set uv order 0: v first; 1: u first */
	d32 = 0;
	((hevc_basic_cfg1 *)(&d32))->video_standard       = 0xD;
	((hevc_basic_cfg1 *)(&d32))->fst_slc_grp          =
		((hevc_basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->fst_slc_grp;
	((hevc_basic_cfg1 *)(&d32))->mv_output_en         = 1;
	((hevc_basic_cfg1 *)(&d32))->uv_order_en          =
		((hevc_basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->uv_order_en;
	((hevc_basic_cfg1 *)(&d32))->vdh_2d_en            =
		((hevc_basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->vdh_2d_en;
	((hevc_basic_cfg1 *)(&d32))->max_slcgrp_num       = 3;
	 // enable switch:output "decodered pixel line of current frame" to DDR
	((hevc_basic_cfg1 *)(&d32))->line_num_output_en   = 0;
	((hevc_basic_cfg1 *)(&d32))->frm_cmp_en           =
		((hevc_basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->frm_cmp_en;
	((hevc_basic_cfg1 *)(&d32))->ppfd_en              = 0;
	wr_vreg(VREG_BASIC_CFG1, d32, 0);

	// avm_addr
	d32 = 0;
	((avm_addr *)(&d32))->av_msg_addr =
		(p_vdh_reg_cfg->vdh_avm_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_AVM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "avm_addr : %pK\n", (void *)(uintptr_t)d32);

	// vam_addr
	d32 = 0;
	((vam_addr *)(&d32))->va_msg_addr =
		(p_vdh_reg_cfg->vdh_vam_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_VAM_ADDR, d32, 0);

	// stream_basic_addr
	d32 = 0;
	((stream_basic_addr *)(&d32))->stream_base_addr =
		(p_vdh_reg_cfg->vdh_stream_base_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_STREAM_BASE_ADDR, d32, 0);

	// TIME_OUT
	d32 = 0x00300C03;
	wr_vreg(VREG_SED_TO, d32, 0);
	wr_vreg(VREG_ITRANS_TO, d32, 0);
	wr_vreg(VREG_PMV_TO, d32, 0);
	wr_vreg(VREG_PRC_TO, d32, 0);
	wr_vreg(VREG_RCN_TO, d32, 0);
	wr_vreg(VREG_DBLK_TO, d32, 0);
	wr_vreg(VREG_PPFD_TO, d32, 0);

	wr_vreg(VREG_YSTADDR_1D, p_vdh_reg_cfg->vdh_yst_addr, 0);

	// YSTRIDE_1D
	wr_vreg(VREG_YSTRIDE_1D, p_vdh_reg_cfg->vdh_ystride, 0);

	// UVOFFSET_1D
	wr_vreg(VREG_UVOFFSET_1D, p_vdh_reg_cfg->vdh_uv_offset, 0);

	// HEAD_INF_OFFSET
	d32 = 0;
	// don't match with document l0021482
	wr_vreg(VREG_HEAD_INF_OFFSET, d32, 0);

	// UVSTRIDE_1D
	wr_vreg(VREG_UVSTRIDE_1D, p_vdh_reg_cfg->vdh_uvstride, 0);

	// CFGINFO_ADDR
	wr_vreg(VREG_CFGINFO_ADDR, p_vdh_reg_cfg->vdh_vfg_info_addr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%pK\n",
		(void *)(uintptr_t)(UINT32)p_vdh_reg_cfg->vdh_vfg_info_addr);

	// DDR_INTERLEAVE_MODE
	d32 = 0x03;
	wr_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);

	// ff_apt_en
	d32 = 0x2;
	wr_vreg(VREG_FF_APT_EN, d32, 0);
	dprint(PRN_VDMREG, "VREG_FF_APT_EN : 0x%x\n", d32);

	// EMAR_ADDR
	d32 = 0x101;
	wr_scd_reg(REG_EMAR_ID, d32);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 hevchal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
#else
SINT32 hevchal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
#endif
{
	SINT32 ret;

#ifdef MSG_POOL_ADDR_CHECK
	UINT32 pic_height;
	UINT32 pic_width;
	vdmhal_hwmem_s *p_hw_mem = NULL;

	if (p_vdh_mem_map->scene == SCENE_VIDEO) {
		p_hw_mem = &g_hw_mem[SCENE_VIDEO];
	} else if (p_vdh_mem_map->scene == SCENE_HEIF) {
		p_hw_mem = &g_hw_mem[SCENE_HEIF];
	} else {
		dprint(PRN_FATAL, "scene(%d) is not supported ",
			p_vdh_mem_map->scene);
		return VDMHAL_ERR;
	}

	if (((hevc_basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->frm_cmp_en) {
		ret = hevchal_check_headmsg(p_vdh_mem_map,
			p_hw_mem->msg_slot_addr[DN_MSG_HEAD_SLOT_INDEX]);
		vdmhal_assert_ret(ret == VDMHAL_OK,
			"HEVC head msg check failed");
	}

	ret = hevchal_setpic_msg(p_hw_mem,
		p_hw_mem->msg_slot_addr[DN_MSG_SLOT_INDEX + 1],
		p_vdh_mem_map, &pic_height, &pic_width);
	vdmhal_assert_ret(ret == VDMHAL_OK, "HEVC pic msg config failed");
#endif

	ret = hevchal_cfg_vdm_reg(p_vdh_reg_cfg);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "HEVC register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
