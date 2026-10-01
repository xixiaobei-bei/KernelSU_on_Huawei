/*
 * vdec hal for hevc
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_hevc.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 HEVCHAL_CheckHeadMsg(MEM_BUFFER_S* pVdhMemMap, UINT32 PicMsgSlotPhy)
{
	UINT32 *pSlot = NULL, *pSlot_base = NULL;
	UINT32 tmp, i, offset;
	SINT32 ret;

	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map head msg virtual address");

	pSlot_base = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_HEAD_SLOT_INDEX  * 4 * USE_MSG_SLOT_SIZE));
	pSlot = pSlot_base;

	//D0 curr frame head_ystaddr
	RD_MSGWORD(pSlot, tmp);
	ret = CheckFrmBufAddr(tmp, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D0 HeadYPhyAddr is failed");

	//D1 curr frame head_cstaddr
	RD_MSGWORD(pSlot + 1, tmp);
	ret = CheckFrmBufAddr(tmp, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D1 HeadYPhyAddr is failed");

	for (i = 0; i < MAX_REF_FRAME_NUM; i++) {
		//D(2*i + 3)
		offset = 2 * i + 3;
		RD_MSGWORD(pSlot + offset, tmp);
		ret = CheckFrmBufAddr(tmp, pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr is failed");
		//D(2*i + 4)
		offset = 2 * i + 4;
		RD_MSGWORD(pSlot + offset, tmp);
		ret = CheckFrmBufAddr(tmp, pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr is failed");
	}
	return VDMHAL_OK;
}

static SINT32 HEVCHAL_SetPicMsg(VDMHAL_HWMEM_S *pHwMem, UADDR FstSlcMsgSlotPhy,
	MEM_BUFFER_S* pVdhMemMap, UINT32* pPicHeight, UINT32* pPicWidth)
{
	UINT32 *pSlot, *pSlot_base;
	UINT32 i;
	UINT32 tmp;
	SINT32 ret;

	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map down msg virtual address");

	pSlot_base = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	pSlot = pSlot_base;

	//D4
	tmp = pHwMem->sed_top_phy_addr;
	WR_MSGWORD(pSlot + 4, tmp);

	//D5
	tmp = pHwMem->pmv_top_phy_addr;
	WR_MSGWORD(pSlot + 5, tmp);

	//D6
	RD_MSGWORD(pSlot + 6, tmp);
	ret = CheckPmvBufAddr(tmp, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D6 is failed");

	//D7
	tmp = pHwMem->rcn_top_phy_addr;
	WR_MSGWORD(pSlot + 7, tmp);

	for (i = 0; i < 16; i++) {
		RD_MSGWORD(pSlot + 8 + i,tmp);
		ret = CheckFrmBufAddr(tmp, pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D8~D24 is failed");
	}

	//D25
	tmp = pHwMem->mn_phy_addr;
	WR_MSGWORD(pSlot + 25, tmp);

	//D43
	tmp = pHwMem->pmv_left_phy_addr;
	WR_MSGWORD(pSlot + 43, tmp);

	//D54
	tmp = 0;
	RD_MSGWORD(pSlot + 54, tmp);
	*pPicWidth = tmp & 0x1fff;
	*pPicHeight = (tmp >> 16) & 0x1fff;

	//D55
	tmp = pHwMem->tile_segment_info_phy_addr;
	WR_MSGWORD(pSlot + 55, tmp);

	//D56
	tmp = pHwMem->sao_left_phy_addr;
	WR_MSGWORD(pSlot + 56, tmp);

	//D57
	tmp = pHwMem->dblk_left_phy_addr;
	WR_MSGWORD(pSlot + 57, tmp);

	//D58
	tmp = pHwMem->sao_top_phy_addr;
	WR_MSGWORD(pSlot + 58, tmp);

	//D59
	tmp = pHwMem->dblk_top_phy_addr;
	WR_MSGWORD(pSlot + 59, tmp);

	//D63
	tmp = FstSlcMsgSlotPhy;
	WR_MSGWORD(pSlot + 63, tmp);

	return VDMHAL_OK;
}
#endif

static SINT32 HEVCHAL_CfgVdmReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	UINT32 D32;

	//BASIC_CFG0
	D32 = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->marker_bit_detect_en = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->ac_last_detect_en    = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->coef_idx_detect_en   = 1; //(run_cnt>64) check enable switch
	((HEVC_BASIC_CFG0 *)(&D32))->vop_type_detect_en   = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->load_qmatrix_flag    = ((HEVC_BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->load_qmatrix_flag;
	((HEVC_BASIC_CFG0 *)(&D32))->luma_sum_en          = 0; //enable switch:conculate luma pixel
	((HEVC_BASIC_CFG0 *)(&D32))->luma_histogram_en    = 0; //enable switch:conculate luma histogram
	((HEVC_BASIC_CFG0 *)(&D32))->mbamt_to_dec         = ((HEVC_BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
#ifdef ENV_SOS_KERNEL
	((HEVC_BASIC_CFG0*)(&D32))->vdh_safe_flag         = 1;
#else
	((HEVC_BASIC_CFG0*)(&D32))->vdh_safe_flag         = 0;
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0 );

	//BASIC_CFG1
	/*set uv order 0: v first; 1: u first */
	D32 = 0;
	((HEVC_BASIC_CFG1 *)(&D32))->video_standard       = 0xD;
	((HEVC_BASIC_CFG1 *)(&D32))->fst_slc_grp          = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->fst_slc_grp;
	((HEVC_BASIC_CFG1 *)(&D32))->mv_output_en         = 1;
	((HEVC_BASIC_CFG1 *)(&D32))->uv_order_en          = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((HEVC_BASIC_CFG1 *)(&D32))->vdh_2d_en            = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((HEVC_BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 3;
	((HEVC_BASIC_CFG1 *)(&D32))->line_num_output_en   = 0; //enable switch:output "decodered pixel line of current frame" to DDR
	((HEVC_BASIC_CFG1 *)(&D32))->frm_cmp_en           = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->frm_cmp_en;
	((HEVC_BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	WR_VREG( VREG_BASIC_CFG1, D32, 0 );

	//AVM_ADDR
	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	//VAM_ADDR
	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);

	//STREAM_BASE_ADDR
	D32 = 0;
	((STREAM_BASE_ADDR *)(&D32))->stream_base_addr = (pVdhRegCfg->VdhStreamBaseAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_STREAM_BASE_ADDR, D32, 0);

	//TIME_OUT
	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);

	WR_VREG(VREG_YSTADDR_1D, pVdhRegCfg->VdhYstAddr, 0);

	//YSTRIDE_1D
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);

	//UVOFFSET_1D
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);

	//HEAD_INF_OFFSET
	D32 = 0;
	WR_VREG(VREG_HEAD_INF_OFFSET, D32, 0);

	//UVSTRIDE_1D
	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	//CFGINFO_ADDR
	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%pK\n", (void *)(uintptr_t)(UINT32)pVdhRegCfg->VdhCfgInfoAddr);

	//DDR_INTERLEAVE_MODE
	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	//FF_APT_EN
	D32 = 0x2;
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "VREG_FF_APT_EN : 0x%x\n", D32);

	//EMAR_ADDR
	D32 = 0x101;
	WR_SCDREG(REG_EMAR_ID, D32);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 HEVCHAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S *pVdhMemMap)
#else
SINT32 HEVCHAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
#endif
{
	SINT32 Ret;

#ifdef MSG_POOL_ADDR_CHECK
	UINT32 PicHeight;
	UINT32 PicWidth;
	VDMHAL_HWMEM_S *pHwMem = NULL;
	if (SCENE_VIDEO == pVdhMemMap->scene) {
		pHwMem = &g_HwMem[SCENE_VIDEO];
	} else if (SCENE_HEIF == pVdhMemMap->scene) {
		pHwMem = &g_HwMem[SCENE_HEIF];
	} else {
		dprint(PRN_FATAL, "scene(%d) is not supported ", pVdhMemMap->scene);
		return VDMHAL_ERR;
	}

	if (((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->frm_cmp_en) {
		Ret = HEVCHAL_CheckHeadMsg(pVdhMemMap,pHwMem->MsgSlotAddr[DN_MSG_HEAD_SLOT_INDEX]);
		VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "HEVC head msg check failed");
	}

	Ret = HEVCHAL_SetPicMsg(pHwMem, pHwMem->MsgSlotAddr[DN_MSG_SLOT_INDEX + 1],
		pVdhMemMap,&PicHeight,&PicWidth);
	VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "HEVC pic msg config failed");
#endif

	Ret = HEVCHAL_CfgVdmReg(pVdhRegCfg);
	if (Ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "HEVC register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
