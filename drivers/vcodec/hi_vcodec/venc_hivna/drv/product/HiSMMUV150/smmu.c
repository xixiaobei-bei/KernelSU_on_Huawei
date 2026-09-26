/*
 * smmu.c
 *
 * This is for vdec smmu.
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

#include "smmu.h"
#include "smmu_regs.h"
#include "drv_mem.h"
#include "drv_common.h"
#include "tvp_adapter.h"
#include "venc_regulator.h"

#define SMRX_ID_SIZE                 37
#define HIVENC_SMMU_COMMON_OFFSET    0x20000
#define HIVENC_SMMU_MASTER_OFFSET    0x1A000

#define MSTR_RIGHT_SHIFT  15
#define BIT_WEIGHT        21
#define START_BIT         0

typedef struct {
	int32_t *smmu_common_base_viraddr;
	int32_t *smmu_master_base_viraddr;
} smmu_rge_vir_s;

smmu_rge_vir_s g_vencsmmuregvir;

int32_t g_vencsmmulnitflag = 0;
extern venc_smmu_err_add_t g_smmu_err_mem;

#define rd_smmu_common_vreg(reg, dat) \
	dat = *((volatile int32_t *)((int8_t *)g_vencsmmuregvir.smmu_common_base_viraddr + (reg)));

#define wr_smmu_common_vreg(reg, dat) \
	*((volatile int32_t *)((int8_t *)g_vencsmmuregvir.smmu_common_base_viraddr + (reg))) = dat;

#define rd_smmu_master_vreg(reg, dat) \
	dat = *((volatile int32_t *)((int8_t *)g_vencsmmuregvir.smmu_master_base_viraddr + (reg)));

#define wr_smmu_master_vreg(reg, dat) \
	*((volatile int32_t *)((int8_t *)g_vencsmmuregvir.smmu_master_base_viraddr + (reg))) = dat;

static void set_common_reg(int32_t addr, int32_t val, uint32_t bw, uint32_t bs)
{
	int32_t mask = (1UL << bw) - 1UL;
	int32_t tmp = 0;

	rd_smmu_common_vreg(addr, tmp);
	tmp &= ~(mask << bs);/*lint !e502*/
	wr_smmu_common_vreg(addr, tmp | ((val & mask) << bs));/*lint !e665*/
}

static void set_master_reg(int32_t addr, int32_t val, uint32_t bw, uint32_t bs)
{
	int32_t mask = (1UL << bw) - 1UL;
	int32_t tmp = 0;

	rd_smmu_master_vreg(addr, tmp);
	tmp &= ~(mask << bs);/*lint !e502*/
	wr_smmu_master_vreg(addr, tmp | ((val & mask) << bs));/*lint !e665*/
}

int32_t venc_smmu_init(bool is_protected, int32_t core_id)
{
	uint32_t i;
	uint64_t smmu_page_base_addr = venc_get_smmu_ttb();
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	uint32_t *regbase = venc->ctx[core_id].reg_base;

	(void)memset_s(&g_vencsmmuregvir, sizeof(smmu_rge_vir_s), 0, sizeof(smmu_rge_vir_s));
	g_vencsmmuregvir.smmu_common_base_viraddr   = (uint32_t *)((uint64_t)(uintptr_t)regbase + HIVENC_SMMU_COMMON_OFFSET);
	if (g_vencsmmuregvir.smmu_common_base_viraddr == NULL) {
		HI_FATAL_VENC("smmu_common_base_viraddr is NULL");
		return SMMU_ERR;
	}

	g_vencsmmuregvir.smmu_master_base_viraddr   = (uint32_t *)((uint64_t)(uintptr_t)regbase + HIVENC_SMMU_MASTER_OFFSET);
	if (g_vencsmmuregvir.smmu_master_base_viraddr == NULL) {
		HI_FATAL_VENC("smmu_master_base_viraddr is NULL ");
		return SMMU_ERR;
	}

	/* smmu master */
	set_master_reg(SMMU_MSTR_GLB_BYPASS, 0x0, 32, 0); // 32: bit width

	for (i = 0; i < SMRX_ID_SIZE; i++) {
		set_master_reg(SMMU_MSTR_SMRX_0 + i * 0x4, 0x1000, 32, 0); // 32: bit width
		set_common_reg(SMMU_SMRX_NS + i * 0x4, 0xbc,  32, 0); // 32: bit width
	}

	/* mstr refld_luma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 0 * 0x4, 0x41, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 0 * 0x4, 0x14, 8, 12); // 8: bit width, 12: bit start

	/* mstr  refld_chroma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 1 * 0x4, 0x17, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 1 * 0x4, 0xA, 8, 12); // 8: bit width, 12: bit start

	/* mst curld_y */
	set_master_reg(SMMU_MSTR_SMRX_0 + 13 * 0x4, 0, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 13 * 0x4, 0x40, 8, 12); // 8: bit width, 12: bit start

	/* mst curld_u */
	set_master_reg(SMMU_MSTR_SMRX_0 + 14 * 0x4, 0, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 14 * 0x4, 0x10, 8, 12); // 8: bit width, 12: bit start

	/* mst curld_v */
	set_master_reg(SMMU_MSTR_SMRX_0 + 15 * 0x4, 0, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 15 * 0x4, 0x4, 8, 12); // 8: bit width, 12: bit start

	/* mst_pme_ld */
	set_master_reg(SMMU_MSTR_SMRX_0 + 21 * 0x4, 0x5, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 21 * 0x4, 0x1, 8, 12); // 8: bit width, 12: bit start

	/* mst rec_st_luma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 22 * 0x4, 0, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 22 * 0x4, 0x17, 8, 12); // 8: bit width, 12: bit start

	/* mst rec_st_chroma */
	set_master_reg(SMMU_MSTR_SMRX_0 + 23 * 0x4, 0, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 23 * 0x4, 0xD, 8, 12); // 8: bit width, 12: bit start

	/* mst rec_st_luma_h */
	set_master_reg(SMMU_MSTR_SMRX_0 + 35 * 0x4, 0x1, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 35 * 0x4, 0x1, 8, 12); // 8: bit width, 12: bit start

	/* mst rec_st_chroma_h */
	set_master_reg(SMMU_MSTR_SMRX_0 + 36 * 0x4, 0x1, 8, 4); // 8: bit width, 4: bit start
	set_master_reg(SMMU_MSTR_SMRX_0 + 36 * 0x4, 0x1, 8, 12); // 8: bit width, 12: bit start

	set_common_reg(SMMU_CB_TTBR0, smmu_page_base_addr & 0xFFFFFFFF, 32, 0); // 32: bit width
	set_common_reg(SMMU_CB_TTBR_MSB, ((smmu_page_base_addr >> 32) & 0xFFFF), 16, 0); // 16: bit width

#ifdef SUPPORT_SECURE_VENC
	if (is_protected == true) {
		HI_DBG_VENC("config_master smmu reg to secure core_id %d", core_id);
		config_master(SEC_VENC_ON, core_id);
	} else {
		HI_DBG_VENC("reset_master smmu reg to non-secure");
		config_master(SEC_VENC_OFF, core_id);
#endif
		/* common scr */
		set_common_reg(SMMU_SCR, 0x0, 1, 0);
		set_common_reg(SMMU_INTCLR_NS, 0x3f, 32, 0); // 32: bit width

		/* smmu context config */
		set_common_reg(SMMU_CB_TTBCR, 0x1, 1, 0);

		set_common_reg(SMMU_ERR_RDADDR_NS, g_smmu_err_mem.read_addr & 0xFFFFFFFF, 32, 0); // 32: bit width
		set_common_reg(SMMU_ERR_WRADDR_NS, g_smmu_err_mem.write_addr & 0xFFFFFFFF, 32, 0); // 32: bit width
		// 16: bit width
		set_common_reg(SMMU_ERR_ADDR_MSB_NS, ((g_smmu_err_mem.read_addr >> 32) & 0xFFFF), 16, 0);
		// 16: bit width, 16: bit start
		set_common_reg(SMMU_ERR_ADDR_MSB_NS, ((g_smmu_err_mem.write_addr >> 32) & 0xFFFF), 16, 16);
#ifdef SUPPORT_SECURE_VENC
	}
#endif

	return SMMU_OK;
}

int32_t venc_smmu_cfg(struct encode_info *channelcfg, uint32_t *reg_base)
{
	(void)memset_s(&g_vencsmmuregvir, sizeof(smmu_rge_vir_s), 0, sizeof(smmu_rge_vir_s));

	g_vencsmmuregvir.smmu_master_base_viraddr = (uint32_t *)((uint64_t)(uintptr_t)reg_base + HIVENC_SMMU_MASTER_OFFSET);
	if (g_vencsmmuregvir.smmu_master_base_viraddr == NULL) {
		HI_FATAL_VENC("smmu_master_base_viraddr is NULL ");
		return SMMU_ERR;
	}

	/* ref ld luma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, START_BIT);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref ld chroma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref ld luma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref ld chroma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REFLD_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REFLD_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* qpg ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + QPG_LD_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + QPG_LD_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* nbi ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + NBI_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + NBI_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme info ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + PMEINFO_LD_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PMEINFO_LD_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme ld */
	set_master_reg(SMMU_MSTR_SMRX_1 + PME_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PME_LD * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st luma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st chroma */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* qpg st */
	set_master_reg(SMMU_MSTR_SMRX_1 + QPG_ST_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + QPG_ST_QP * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme info st */
	set_master_reg(SMMU_MSTR_SMRX_1 + PMEINFO_ST_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PMEINFO_ST_FLAG * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* nbi st */
	set_master_reg(SMMU_MSTR_SMRX_1 + NBI_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + NBI_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* pme st */
	set_master_reg(SMMU_MSTR_SMRX_1 + PME_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + PME_ST * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st luma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_H_LUMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	/* ref st chroma header */
	set_master_reg(SMMU_MSTR_SMRX_1 + REC_ST_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_start >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);
	set_master_reg(SMMU_MSTR_SMRX_2 + REC_ST_H_CHROMA * 0x4, channelcfg->venc_inter_buffer.master_stream_end >> MSTR_RIGHT_SHIFT, BIT_WEIGHT, 0);

	return SMMU_OK;
}

#ifdef VENC_DEBUG_ENABLE
void venc_smmu_debug(uint32_t *reg_base, bool first_cfg_flag)
{
	uint32_t tmp = 0;

	if (first_cfg_flag)
		return;

	(void)memset_s(&g_vencsmmuregvir, sizeof(smmu_rge_vir_s), 0, sizeof(smmu_rge_vir_s));
	g_vencsmmuregvir.smmu_master_base_viraddr = (uint32_t *)((uint64_t)(uintptr_t)reg_base + HIVENC_SMMU_MASTER_OFFSET);
	if (g_vencsmmuregvir.smmu_master_base_viraddr == NULL) {
		HI_FATAL_VENC("smmu master base addr is NULL");
		return;
	}

	rd_smmu_master_vreg(0x0044, tmp);
	if (tmp & 0x1) {
		HI_ERR_VENC("enter read interrupt");
		rd_smmu_master_vreg(0x0050, tmp);
		HI_ERR_VENC("read sid 0x%x", tmp);
		rd_smmu_master_vreg(0x0054, tmp);
		HI_ERR_VENC("read addr 0x%x", tmp);
	}

	if (tmp & 0x4) {
		HI_ERR_VENC("enter write interrupt");
		rd_smmu_master_vreg(0x0058, tmp);
		HI_ERR_VENC("write sid 0x%x", tmp);
		rd_smmu_master_vreg(0x005C, tmp);
		HI_ERR_VENC("write addr 0x%x", tmp);
	}

	wr_smmu_master_vreg(0x004C, 0x1f);
}
#endif
