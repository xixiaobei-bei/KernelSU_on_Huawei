/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for vp8.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#ifndef __VDM_HAL_VP8_C__
#define __VDM_HAL_VP8_C__

#include "public.h"
#include "vdm_hal_vp8.h"
#include "vdm_hal.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"

static SINT32 vp8hal_cfgreg(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
{
	UINT32 d32;

	//basic_cfg0
	d32 = 0;
	((basic_cfg0 *)(&d32))->mbamt_to_dec      =
		((basic_cfg0 *)(&p_vdh_reg_cfg->vdh_basic_cfg0))->mbamt_to_dec;
	((basic_cfg0 *)(&d32))->load_qmatrix_flag = 0;
#ifdef ENV_SOS_KERNEL
	((basic_cfg0 *)(&d32))->sec_mode_en       = 1;
#else
	((basic_cfg0 *)(&d32))->sec_mode_en       = 0;
#endif
	wr_vreg(VREG_BASIC_CFG0, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg0 : 0x%x\n", p_vdh_reg_cfg->vdh_basic_cfg0);

	//basic_cfg1
	/*set uv order 0: v first; 1: u first */
	d32 = 0x20000000;
	((basic_cfg1 *)(&d32))->video_standard    = 0x0C;
	((basic_cfg1 *)(&d32))->fst_slc_grp       = 1;
	((basic_cfg1 *)(&d32))->mv_output_en      = 1;
	((basic_cfg1 *)(&d32))->uv_order_en       = ((basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->uv_order_en;
	((basic_cfg1 *)(&d32))->vdh_2d_en         = ((basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->vdh_2d_en;
	((basic_cfg1 *)(&d32))->max_slcgrp_num    = 0;
	((basic_cfg1 *)(&d32))->compress_en       = ((basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->compress_en;
	((basic_cfg1 *)(&d32))->ppfd_en           = 0;
	wr_vreg(VREG_BASIC_CFG1, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg1 : 0x%x\n", p_vdh_reg_cfg->vdh_basic_cfg1);

	//avm_addr
	d32 = 0;
	((avm_addr *)(&d32))->av_msg_addr =
		(p_vdh_reg_cfg->vdh_avm_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_AVM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "avm_addr : %pK\n", (void *)(uintptr_t)d32);

	//vam_addr
	d32 = 0;
	((vam_addr *)(&d32))->va_msg_addr =
		(p_vdh_reg_cfg->vdh_vam_addr) & 0xFFFFFFF0;
	wr_vreg(VREG_VAM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "vam_addr : %pK\n", (void *)(uintptr_t)d32);

	//stream_basic_addr
	wr_vreg(VREG_STREAM_BASE_ADDR, p_vdh_reg_cfg->vdh_stream_base_addr, 0);
	dprint(PRN_VDMREG, "stream_basic_addr : %pK\n",
		(void *)(uintptr_t)(UINT32)p_vdh_reg_cfg->vdh_stream_base_addr);

	//EMAR_ADDR
	d32 = 0x101;
	wr_scd_reg(REG_EMAR_ID, d32);

	//ystaddr_1d
	d32 = 0;
	((ystaddr_1d *)(&d32))->ystaddr_1d =
		(p_vdh_reg_cfg->vdh_yst_addr) & 0xFFFFFFF0; // caution
	wr_vreg(VREG_YSTADDR_1D, d32, 0);
	dprint(PRN_VDMREG, "ystaddr_1d : %pK\n", (void *)(uintptr_t)d32);

	//YSTRIDE_1D
	wr_vreg(VREG_YSTRIDE_1D, p_vdh_reg_cfg->vdh_ystride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", p_vdh_reg_cfg->vdh_ystride);

	//UVOFFSET_1D
	wr_vreg(VREG_UVOFFSET_1D, p_vdh_reg_cfg->vdh_uv_offset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", p_vdh_reg_cfg->vdh_uv_offset);

	d32 = 0;
	wr_vreg(VREG_HEAD_INF_OFFSET, d32, 0);

	wr_vreg(VREG_UVSTRIDE_1D, p_vdh_reg_cfg->vdh_uvstride, 0);

	wr_vreg(VREG_CFGINFO_ADDR, p_vdh_reg_cfg->vdh_vfg_info_addr, 0);

	d32 = 0x03;
	wr_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);
	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 vp8hal_check_headmsg(mem_buffer_s *p_vdh_mem_map,
	UINT32 pic_msg_slot_phy)
{
	UINT32 *p_slot = NULL;
	UINT32 *p_slot_base = NULL;
	UINT32 tmp;
	SINT32 ret;

	vdmhal_assert_ret(
		p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr != NULL,
		"can not map head msg virtual address");

	p_slot_base =
		(UINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr +
		(DN_MSG_HEAD_SLOT_INDEX  * 4 * USE_MSG_SLOT_SIZE));
	p_slot = p_slot_base;

	// D0 curr frame head_ystaddr
	rd_msgword(p_slot, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "CurPicHeadYPhyAddr is failed");

	// D1 curr frame head_cstaddr
	rd_msgword(p_slot + 1, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "CurPicHeadCPhyAddr is failed");

	// D3
	rd_msgword(p_slot + 3, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "LastRefPicHeadYPhyAddr is failed");

	// D4
	rd_msgword(p_slot + 4, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "LastRefPicHeadCPhyAddr is failed");

	// D5
	rd_msgword(p_slot + 5, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK,
					  "GoldenRefPicHeadYPhyAddr is failed");

	// D6
	rd_msgword(p_slot + 6, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK,
					  "GoldenRefPicHeadCPhyAddr is failed");

	// D7
	rd_msgword(p_slot + 7, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "AltRefPicHeadYPhyAddr is failed");

	// D8
	rd_msgword(p_slot + 8, tmp);
	ret = check_frm_buf_addr(tmp, p_vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "AltRefPicHeadCPhyAddr is failed");

	return VDMHAL_OK;
}

static SINT32 vp8hal_cfg_dnmsg(vdmhal_hwmem_s *p_hw_mem,
							   mem_buffer_s *p_vdh_mem_map,
							   UINT32 *p_pic_height_mb,
							   UINT32 *p_pic_width_mb)
{
	UINT32 *p_msg_base = NULL;
	UINT32 *p32 = NULL;
	UINT32 d32;
	SINT32 ret;
	SINT32 icount;

	vdmhal_assert_ret(
		p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr != NULL,
		"can not map down msg virtual address");

	p_msg_base =
		(UINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr +
				   (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	p32 = p_msg_base;
	d32 = 0;

	/* D25 */
	rd_msgword((p32 + 25), d32);
	if ((p_vdh_mem_map[VDH_SHAREFD_STREAM_BUF].start_phy_addr > d32) ||
		(d32 > (p_vdh_mem_map[VDH_SHAREFD_STREAM_BUF].start_phy_addr +
				p_vdh_mem_map[VDH_SHAREFD_STREAM_BUF].size))) {
		dprint(PRN_FATAL, "%s check D25 failed\n", __func__);
		return VDMHAL_ERR;
	}

	p32 += 31;
	for (icount = 0; icount < 4; icount++) {
		/* d32 ~ D35 */
		++p32;
		rd_msgword(p32, d32);
		ret = check_frm_buf_addr(d32, p_vdh_mem_map);
		if (ret == VDMHAL_ERR) {
			dprint(PRN_FATAL, "%s check D(%d) failed\n",
				   __func__, (32 + icount));
			return VDMHAL_ERR;
		}
	}

	/* D36 */
	++p32;
	d32 = 0;
	((vp8_picdnmsg_d36 *)(&d32))->sed_top_addr =
		(p_hw_mem->sed_top_addr + 0xF) & 0xFFFFFFF0;   // caution!!
	wr_msgword(p32, d32);

	// D37
	++p32;
	d32 = 0;
	((vp8_picdnmsg_d37 *)(&d32))->pmv_top_addr =
		(p_hw_mem->pmv_top_addr + 0xF) & 0xFFFFFFF0;   // caution!
	wr_msgword(p32, d32);

	// D38
	++p32;
	d32 = 0;
	((vp8_picdnmsg_d38 *)(&d32))->rcn_top_addr =
		(p_hw_mem->rcn_top_addr + 0xF) & 0xFFFFFFF0;   // caution!
	wr_msgword(p32, d32);

	/* D39 */
	++p32;
	d32 = 0;
	((vp8_picdnmsg_d39 *)(&d32))->tab_addr =
		(p_hw_mem->vp8_tab_addr + 0xF) & 0xFFFFFFF0; // caution!
	wr_msgword(p32, d32);

	/* D40 */
	++p32;
	d32 = 0;
	((vp8_picdnmsg_d40 *)(&d32))->dblk_top_addr =
		(p_hw_mem->dblk_top_addr + 0xF) & 0xFFFFFFF0; // caution!
	wr_msgword(p32, d32);

	/* D63  next msg group -- caution!!1 */
	p32 += 23;
	d32 = p_hw_mem->msg_slot_addr[DN_MSG_SLOT_INDEX] + 64 * 4;
	wr_msgword(p32, d32);

	return VDMHAL_OK;
}
#endif

#ifdef MSG_POOL_ADDR_CHECK
SINT32 vp8hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
#else
SINT32 vp8hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg)
#endif
{
	SINT32 ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;
	vdmhal_hwmem_s *p_hw_mem = &g_hw_mem[0];

	if (((basic_cfg1 *)(&p_vdh_reg_cfg->vdh_basic_cfg1))->compress_en) {
		ret = vp8hal_check_headmsg(p_vdh_mem_map,
			p_hw_mem->msg_slot_addr[DN_MSG_HEAD_SLOT_INDEX]);
			vdmhal_assert_ret(ret == VDMHAL_OK,
			"VP8 head msg check failed");
	}

	ret = vp8hal_cfg_dnmsg(p_hw_mem, p_vdh_mem_map, &pic_height_mb, &pic_width_mb);
	vdmhal_assert_ret(ret == VDMHAL_OK, "VP8 DnMsg config failed");
#endif
	ret = vp8hal_cfgreg(p_vdh_reg_cfg);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "VP8 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
