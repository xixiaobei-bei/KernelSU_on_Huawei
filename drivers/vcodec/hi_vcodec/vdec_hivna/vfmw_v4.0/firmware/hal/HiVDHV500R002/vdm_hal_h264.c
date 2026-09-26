/*
 * vdec hal for h264
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
#include "vdm_hal_h264.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 H264HAL_CheckHeadMsg(MEM_BUFFER_S* pVdhMemMap,UADDR PicMsgSlotPhy)
{
	UINT32 *pSlot = NULL, *pSlot_base = NULL;
	UINT32 tmp, i, offset;
	SINT32 ret;

	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map head msg virtual address");

	pSlot_base = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_HEAD_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	pSlot = pSlot_base;

	//D0 curr frame head_ystaddr
	RD_MSGWORD(pSlot, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr is failed");

	//D1 curr frame head_cstaddr
	RD_MSGWORD(pSlot + 1, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadCPhyAddr is failed");

	for (i = 0; i < MAX_REF_FRAME_NUM; i++) {
		//D(2*i + 3)
		offset = 2 * i + 3;
		RD_MSGWORD(pSlot + offset, tmp);
		ret = CheckFrmBufAddr(tmp,pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr:Apc2Idc is failed");

		//D(2*i + 4)
		offset = 2 * i + 4;
		RD_MSGWORD(pSlot + offset, tmp);
		ret = CheckFrmBufAddr(tmp,pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadCPhyAddr:Apc2Idc is failed");
	}

	return VDMHAL_OK;
}

static SINT32 H264HAL_SetPicAddrMsg(VDMHAL_HWMEM_S *pHwMem, UADDR FstSlcMsgSlotPhy,
	MEM_BUFFER_S* pVdhMemMap, UINT32* pPicHeightMb, UINT32* pPicWidthMb)
{
	UINT32 *pSLOT, *pSLOT_BASE;
	UINT32 i, tmp;
	SINT32 ret;

	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map down msg virtual address\n");

	pSLOT_BASE = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));
	pSLOT = pSLOT_BASE;

	//D1
	tmp = 0;
	RD_MSGWORD(pSLOT + 0x1,tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D1 is failed");
	//D5
	tmp = (pHwMem->SedTopAddr + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pSLOT + 0x5, tmp);

	//D6
	tmp = (pHwMem->PmvTopAddr + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pSLOT + 0x6, tmp);

	RD_MSGWORD(pSLOT + 0x7, tmp);
	ret = CheckPmvBufAddr(tmp, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D7 is failed");

	//D8
	tmp = (pHwMem->RcnTopAddr + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pSLOT + 0x8, tmp);

	//D9
	tmp = (FstSlcMsgSlotPhy + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pSLOT + 0x9, tmp);

	//D26
	tmp = (pHwMem->H264MnAddr + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pSLOT + 26, tmp);

	for (i = 0; i < MAX_REF_FRAME_NUM; i++) {
		RD_MSGWORD(pSLOT + 10 + i,tmp);
		ret = CheckFrmBufAddr(tmp, pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr is failed");
	}

	//D59
	tmp = (pHwMem->DblkTopAddr + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pSLOT + 59, tmp);

	return 0;
}
#endif

#ifdef MSG_POOL_ADDR_CHECK
SINT32 H264HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg,MEM_BUFFER_S* pVdhMemMap)
#else
SINT32 H264HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
#endif
{
	UINT32 D32;
#ifdef MSG_POOL_ADDR_CHECK
	SINT32 Ret;
	UINT32 PicHeightMb;
	UINT32 PicWidthMb;
	VDMHAL_HWMEM_S *pHwMem = &g_HwMem[0];
#endif

	dprint(PRN_VDMREG, "\n***********************H264HAL_StartDec ***********************\n");
#ifdef MSG_POOL_ADDR_CHECK
	if (((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en) {
		Ret = H264HAL_CheckHeadMsg(pVdhMemMap,pHwMem->MsgSlotAddr[DN_MSG_HEAD_SLOT_INDEX]);
		VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "H264 head msg check failed");
	}

	Ret = H264HAL_SetPicAddrMsg( pHwMem, pHwMem->MsgSlotAddr[DN_MSG_SLOT_INDEX + 1],
		pVdhMemMap,&PicHeightMb,&PicWidthMb);
	VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "H264 pic msg config failed");
#endif
	D32 = 0;
	((BASIC_CFG0 *)(&D32))->mbamt_to_dec         = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0 *)(&D32))->load_qmatrix_flag    = 1;
	((BASIC_CFG0 *)(&D32))->marker_bit_detect_en = 0;
	((BASIC_CFG0 *)(&D32))->ac_last_detect_en    = 0;
	((BASIC_CFG0 *)(&D32))->coef_idx_detect_en   = 1;
	((BASIC_CFG0 *)(&D32))->vop_type_detect_en   = 0;
#ifdef ENV_SOS_KERNEL
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 1;
#else
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 0;
#endif
	WR_VREG(VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", D32);

	D32 = 0;
	((BASIC_CFG1 *)(&D32))->video_standard       = 0x0;
	//((BASIC_CFG1 *)(&D32))->ddr_stride           = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1 *)(&D32))->fst_slc_grp          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->fst_slc_grp;
	((BASIC_CFG1 *)(&D32))->mv_output_en         = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->mv_output_en;
	((BASIC_CFG1 *)(&D32))->uv_order_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1 *)(&D32))->vdh_2d_en            = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 2;
	((BASIC_CFG1 *)(&D32))->compress_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	((BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	((BASIC_CFG1 *)(&D32))->line_num_output_en   = 0;
	WR_VREG(VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", D32);

	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	D32 = 0;
	((STREAM_BASE_ADDR *)(&D32))->stream_base_addr = (pVdhRegCfg->VdhStreamBaseAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_STREAM_BASE_ADDR, D32, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : %pK\n", (void *)(uintptr_t)D32);

	D32 = RD_SCDREG(REG_EMAR_ID);
	if (pVdhRegCfg->VdhEmarId == 0) {
		D32 = D32 & (~(0x100));
	} else {
		D32 = D32 | 0x100;
	}
	WR_SCDREG(REG_EMAR_ID, D32);

	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);

	D32 = 0;
	((YSTADDR_1D *)(&D32))->ystaddr_1d = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : %pK\n", (void *)(uintptr_t)D32);

	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	D32 = 0;
	WR_VREG(VREG_HEAD_INF_OFFSET, D32, 0);

	WR_VREG(VREG_REF_PIC_TYPE, pVdhRegCfg->VdhRefPicType, 0);
	dprint(PRN_VDMREG, "REF_PIC_TYPE : 0x%x\n", pVdhRegCfg->VdhRefPicType);

	if (pVdhRegCfg->VdhFfAptEn == 0x2) {
		D32 = 0x2;
	} else {
		D32 = 0x0;
	}
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "FF_APT_EN : 0x%x\n", D32);

	//UVSTRIDE_1D
	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );
	dprint(PRN_VDMREG, "UVSTRIDE_1D = 0x%x\n", pVdhRegCfg->VdhUvstride);

	//CFGINFO_ADDR
	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%pK\n", (void *)(uintptr_t)(UINT32)pVdhRegCfg->VdhCfgInfoAddr);

	//DDR_INTERLEAVE_MODE
	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}
