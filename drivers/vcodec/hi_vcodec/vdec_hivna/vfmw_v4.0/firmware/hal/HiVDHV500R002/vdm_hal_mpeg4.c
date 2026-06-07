/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for mp4.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#include "vdm_hal_mpeg4.h"

#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 mp4hal_cfg_dnmsg(mem_buffer_s *p_vdh_mem_map,
							   UINT32 *p_pic_height_mb,
							   UINT32 *p_pic_width_mb)
{
	UINT8 *p_msg_block = NULL;
	UINT32 *p_reg = NULL;
	UINT32 dat;
	SINT32 ret;

	vdmhal_assert_ret(
		p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr != NULL,
		"can not map down msg virtual address");

	/* step1: basic config < D0 ~ D7 > */
	p_msg_block = (UINT8 *)p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr
				+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE);

	/* step3: space arrangement < D12 ~ D23 > */
	/* D12: image_curr_recon_addr */
	p_reg = (UINT32 *)(p_msg_block + 4 * 8) + 4;

	rd_msgword(p_reg, dat);
	ret = check_frm_buf_addr(dat, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D12 is failed");

	/* D13: image_forward_ref_addr */
	p_reg++;
	rd_msgword(p_reg, dat);
	ret = check_frm_buf_addr(dat, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D13 is failed");

	/* D14: image_backward_ref_addr */
	p_reg++;
	rd_msgword(p_reg, dat);
	ret = check_frm_buf_addr(dat, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D14 is failed");

	/* D15: pmv_colmb_addr */
	p_reg++;
	rd_msgword(p_reg, dat);
	ret = check_pmv_buf_addr(dat, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D15 is failed");


	/* D16: pmv_backward_ref_addr */
	p_reg++;
	rd_msgword(p_reg, dat);
	ret = check_pmv_buf_addr(dat, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D16 is failed");

	/* D17: sed_top_addr */
	dat = (g_hw_mem[0].itrans_top_addr + 0xF) & 0xFFFFFFF0;
	p_reg++;
	wr_msgword(p_reg, dat);

	/* D18: pmv_top_addr */
	dat = (g_hw_mem[0].pmv_top_addr + 0xF) & 0xFFFFFFF0;
	p_reg++;
	wr_msgword(p_reg, dat);

	/* W' : D60 */
	p_reg = (UINT32 *)(p_msg_block + 4 * 60);
	dat = g_hw_mem[0].sed_top_addr & 0xFFFFFFF0;
	wr_msgword(p_reg, dat);

	/* W' : D63 */
	p_reg = (UINT32 *)(p_msg_block + 4 * 63);
	dat = g_hw_mem[0].msg_slot_addr[DN_MSG_SLOT_INDEX] + 4 * 64;
	wr_msgword(p_reg, dat);

	return VDMHAL_OK;
}
#endif
static SINT32 mp4hal_cfg_reg(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
{
	UINT32 d32;
	SINT32 slot_width;

	//basic_cfg0
	d32 = 0;
	d32 = (p_vdh_reg_cfg->vdh_basic_cfg0 & 0x000FFFFF) // [15:0] mbamt_to_dec
		  | (1 << 22)
		  | (0 << 23)
		  | (0 << 24)
		  | (1 << 25)
		  | (1 << 30)       // ld_qmatrix_flag
#ifdef ENV_SOS_KERNEL
		  | (1 << 31);      // Secure Mode
#else
		  | (0 << 31);      // Normal Mode
#endif
	wr_vreg(VREG_BASIC_CFG0, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg0 : 0x%x\n", d32);

	/*set uv order 0: v first; 1: u first */
	d32 = 0x2               // [3:0] video_standard
		  | (((p_vdh_reg_cfg->vdh_basic_cfg1 >> 13) &
			0x1) << 13)     // uv_order_en
		  | (1 << 14)       // [14] fst_slc_grp
		  | (1 << 15)       // [15] mv_output_en
		  | (1 << 16)       // [27:16] max_slcgrp_num
		  | (0 << 28)        // line_num_output_en
		  | (1 << 29)
		  | (0 << 30)     //compress_en
		  | (0 << 31);      // [31] ppfd_en   0==not ppfd dec
	wr_vreg(VREG_BASIC_CFG1, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg1 : 0x%x\n", d32);

	d32 = (p_vdh_reg_cfg->vdh_avm_addr) & 0xFFFFFFF0;  // mpeg4 down msg
	wr_vreg(VREG_AVM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "avm_addr : %pK\n", (void *)(uintptr_t)d32);

	d32 = (p_vdh_reg_cfg->vdh_vam_addr) & 0xFFFFFFF0;  // mpeg4 up msg
	wr_vreg(VREG_VAM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "vam_addr : %pK\n", (void *)(uintptr_t)d32);

	wr_vreg(VREG_STREAM_BASE_ADDR, p_vdh_reg_cfg->vdh_stream_base_addr, 0);
	dprint(PRN_VDMREG, "stream_basic_addr : %pK\n",
		(void *)(uintptr_t)(UINT32)p_vdh_reg_cfg->vdh_stream_base_addr);

	//EMAR_ADDR
	d32 = 0x101;

	slot_width = p_vdh_reg_cfg->vdh_ystride / 8;
	if (slot_width > 1920)
		d32 = d32 & (~(0x100));
	else
		d32 = d32 | (0x100);

	wr_scd_reg(REG_EMAR_ID, d32);

	d32 = 0x00300C03;
	wr_vreg(VREG_SED_TO, d32, 0);
	wr_vreg(VREG_ITRANS_TO, d32, 0);
	wr_vreg(VREG_PMV_TO, d32, 0);
	wr_vreg(VREG_PRC_TO, d32, 0);
	wr_vreg(VREG_RCN_TO, d32, 0);
	wr_vreg(VREG_DBLK_TO, d32, 0);
	wr_vreg(VREG_PPFD_TO, d32, 0);

	d32 = (p_vdh_reg_cfg->vdh_yst_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_YSTADDR_1D, d32, 0);
	dprint(PRN_VDMREG, "ystaddr_1d : %pK\n", (void *)(uintptr_t)d32);

	wr_vreg(VREG_YSTRIDE_1D, p_vdh_reg_cfg->vdh_ystride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", p_vdh_reg_cfg->vdh_ystride);

	wr_vreg(VREG_UVOFFSET_1D, p_vdh_reg_cfg->vdh_uv_offset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", p_vdh_reg_cfg->vdh_uv_offset);

	d32 = 0;
	wr_vreg(VREG_FF_APT_EN, d32, 0);
	dprint(PRN_VDMREG, "PRCMEM2_1D_CNT : 0x%x\n", d32);

	wr_vreg(VREG_UVSTRIDE_1D, p_vdh_reg_cfg->vdh_uvstride, 0);

	d32 = 0x03;
	wr_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
#else
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
#endif
{
	SINT32 ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;

	ret = mp4hal_cfg_dnmsg(p_vdh_mem_map, &pic_height_mb, &pic_width_mb);
	vdmhal_assert_ret(ret == VDMHAL_OK, "MP4 DnMsg config failed");
#endif
	ret = mp4hal_cfg_reg(p_vdh_reg_cfg);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP4 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
