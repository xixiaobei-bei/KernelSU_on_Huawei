/*
 * vdm_hal_mpeg4.c
 *
 * This is vdec hal for mp4.
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

#include "vdm_hal_mpeg4.h"

#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "vdm_hal_local.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 mp4hal_cfgdnmsg(mem_buffer_s *vdh_mem_map, UINT32 *pic_height_mb, UINT32 *pic_width_mb)
{
	UINT8 *msg_block = NULL;
	UINT32 *reg = NULL;
	UINT32 dat;
	SINT32 ret;

	vdmhal_assert_ret(
		vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr != NULL,
					  "can not map down msg virtual address");

	/* D0 ~ D7 */
	msg_block =
		(UINT8 *)vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_vir_addr +
		(DN_MSG_SLOT_INDEX * MPEG4_MSG_SLOT_RATIO * USE_MSG_SLOT_SIZE);

	/* D12 */
	reg = (UINT32 *)(msg_block + 4 * 8) + 4;

	rd_msgword(reg, dat);
	dat = hw_addr_lshift(dat);
	ret = check_frm_buf_addr(dat, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D12 is failed");

	/* D13 */
	reg++;
	rd_msgword(reg, dat);
	dat = hw_addr_lshift(dat);
	ret = check_frm_buf_addr(dat, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D13 is failed");

	/* D14 */
	reg++;
	rd_msgword(reg, dat);
	dat = hw_addr_lshift(dat);
	ret = check_frm_buf_addr(dat, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D14 is failed");

	/* D15 */
	reg++;
	rd_msgword(reg, dat);
	dat = hw_addr_lshift(dat);
	ret = check_pmv_buf_addr(dat, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D15 is failed");

	/* D16 */
	reg++;
	rd_msgword(reg, dat);
	dat = hw_addr_lshift(dat);
	ret = check_pmv_buf_addr(dat, vdh_mem_map);
	vdmhal_assert_ret(ret == VDMHAL_OK, "D16 is failed");

	/* D17 */
	dat = align_up(g_hw_mem[0].itrans_top_addr, MSG_SLOT_ALIGN_BYTE);
	dat = hw_addr_rshift(dat);
	reg++;
	wr_msgword(reg, dat);

	/* D18 */
	dat = align_up(g_hw_mem[0].pmv_top_addr, MSG_SLOT_ALIGN_BYTE);
	dat = hw_addr_rshift(dat);
	reg++;
	wr_msgword(reg, dat);

	/* D60 */
	reg = (UINT32 *)(msg_block + MPEG4_MSG_SLOT_RATIO * 60);
	dat = align_down_hal(g_hw_mem[0].sed_top_addr, MSG_SLOT_ALIGN_BYTE);
	dat = hw_addr_rshift(dat);
	wr_msgword(reg, dat);

	/* D63 */
	reg = (UINT32 *)(msg_block + MPEG4_MSG_SLOT_RATIO * 63);
	dat = g_hw_mem[0].msg_slot_addr[DN_MSG_SLOT_INDEX] + MPEG4_MSG_SLOT_RATIO * 64;
	dat = hw_addr_rshift(dat);
	wr_msgword(reg, dat);

	return VDMHAL_OK;
}
#endif

void handle_mp4hal_cfg_reg(UINT32 d32, omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	write_vreg(VREG_SED_TO, d32, 0);
	write_vreg(VREG_ITRANS_TO, d32, 0);
	write_vreg(VREG_PMV_TO, d32, 0);
	write_vreg(VREG_PRC_TO, d32, 0);
	write_vreg(VREG_RCN_TO, d32, 0);
	write_vreg(VREG_DBLK_TO, d32, 0);
	write_vreg(VREG_PPFD_TO, d32, 0);

	d32 = align_down_hal(vdh_reg_cfg->vdh_yst_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_YSTADDR_1D, d32, 0);
	dprint(PRN_VDMREG, "ystaddr_1d : %pK\n", (void *)(uintptr_t)d32);

	write_vreg(VREG_YSTRIDE_1D, vdh_reg_cfg->vdh_ystride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", vdh_reg_cfg->vdh_ystride);

	write_vreg(VREG_UVOFFSET_1D, vdh_reg_cfg->vdh_uv_offset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", vdh_reg_cfg->vdh_uv_offset);

	d32 = 0;
	write_vreg(VREG_FF_APT_EN, d32, 0);
	dprint(PRN_VDMREG, "PRCMEM2_1D_CNT : 0x%x\n", d32);

	write_vreg(VREG_UVSTRIDE_1D, vdh_reg_cfg->vdh_uvstride, 0);

	d32 = 0x03;
	write_vreg(VREG_DDR_INTERLEAVE_MODE, d32, 0);
}

static SINT32 mp4hal_cfgreg(omxvdh_reg_cfg_s *vdh_reg_cfg)
{
	UINT32 d32;
	SINT32 slot_width;

	d32 = (vdh_reg_cfg->vdh_basic_cfg0 & 0x000FFFFF) | // [15:0] mbamt_to_dec
		(1 << 22) |
		(0 << 23) |
		(0 << 24) |
		(1 << 25) |
		(1 << 30) |     // ld_qmatrix_flag
#ifdef ENV_SOS_KERNEL
		(1 << 31);      // Secure Mode
#else
		(0 << 31);      // Normal Mode
#endif
	write_vreg(VREG_BASIC_CFG0, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg0 : 0x%x\n", d32);

	/* set uv order 0: v first; 1: u first */
	d32 = 0x2     |     // [3:0] video_standard
		(((vdh_reg_cfg->vdh_basic_cfg1 >> 13) & 0x1) << 13) | // uv_order_en
		(1 << 14) |     // [14] fst_slc_grp
		(1 << 15) |     // [15] mv_output_en
		(1 << 16) |     // [27:16] max_slcgrp_num
		(0 << 28) |     // line_num_output_en
		(1 << 29) |     // vdh_2d_en
		(0 << 30) |     // compress_en
		(0 << 31);      // [31] ppfd_en   0==not ppfd dec
	write_vreg(VREG_BASIC_CFG1, d32, 0);
	dprint(PRN_VDMREG, "basic_cfg1 : 0x%x\n", d32);

	// mpeg4 down msg
	d32 = align_down_hal(vdh_reg_cfg->vdh_avm_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_AVM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "avm_addr : %pK\n", (void *)(uintptr_t)d32);

	// mpeg4 up msg
	d32 = align_down_hal(vdh_reg_cfg->vdh_vam_addr, MSG_SLOT_ALIGN_BYTE);
	d32 = hw_addr_rshift(d32);
	write_vreg(VREG_VAM_ADDR, d32, 0);
	dprint(PRN_VDMREG, "vam_addr : %pK\n", (void *)(uintptr_t)d32);

	write_vreg(VREG_STREAM_BASE_ADDR,
		hw_addr_rshift(vdh_reg_cfg->vdh_stream_base_addr), 0);
	dprint(PRN_VDMREG, "stream_base_addr : %pK\n",
		(void *)(uintptr_t)(UINT32)(hw_addr_rshift
		(vdh_reg_cfg->vdh_stream_base_addr)));

	// EMAR_ADDR
	d32 = 0x101;

	slot_width = vdh_reg_cfg->vdh_ystride / 8;
	if (slot_width > 1920)
		d32 = d32 & (~(0x100));
	else
		d32 = d32 | (0x100);

	wr_scd_reg(REG_EMAR_ID, d32);

	d32 = 0x00300C03;
	handle_mp4hal_cfg_reg(d32, vdh_reg_cfg);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map)
#else
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg)
#endif
{
	SINT32 ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 pic_height_mb;
	UINT32 pic_width_mb;

	if (vdh_mem_map == NULL) {
		dprint(PRN_FATAL, "vdh_mem_map is null\n");
		return VDMHAL_ERR;
	}
	ret = mp4hal_cfgdnmsg(vdh_mem_map, &pic_height_mb, &pic_width_mb);
	vdmhal_assert_ret(ret == VDMHAL_OK, "MP4 DnMsg config failed");
#endif
	ret = mp4hal_cfgreg(vdh_reg_cfg);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP4 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
