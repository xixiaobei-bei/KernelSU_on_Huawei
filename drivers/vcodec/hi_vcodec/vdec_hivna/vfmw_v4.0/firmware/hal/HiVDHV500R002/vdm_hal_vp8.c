/*
 * vdec hal for vp8
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#ifndef __VDM_HAL_VP8_C__
#define __VDM_HAL_VP8_C__

#include "public.h"
#include "vdm_hal.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_vp8.h"

static SINT32 VP8HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	UINT32 D32;

	//BASIC_CFG0
	D32 = 0;
	((BASIC_CFG0 *)(&D32))->mbamt_to_dec      = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0 *)(&D32))->load_qmatrix_flag = 0;
#ifdef ENV_SOS_KERNEL
	((BASIC_CFG0 *)(&D32))->sec_mode_en       = 1;
#else
	((BASIC_CFG0 *)(&D32))->sec_mode_en       = 0;
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", pVdhRegCfg->VdhBasicCfg0);

	//BASIC_CFG1
	/*set uv order 0: v first; 1: u first */
	D32 = 0x20000000;
	((BASIC_CFG1 *)(&D32))->video_standard    = 0x0C;
	//((BASIC_CFG1 *)(&D32))->ddr_stride        = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1 *)(&D32))->fst_slc_grp       = 1;
	((BASIC_CFG1 *)(&D32))->mv_output_en      = 1;
	((BASIC_CFG1 *)(&D32))->uv_order_en       = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1 *)(&D32))->vdh_2d_en         = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((BASIC_CFG1 *)(&D32))->max_slcgrp_num    = 0;
	((BASIC_CFG1 *)(&D32))->compress_en       = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	((BASIC_CFG1 *)(&D32))->ppfd_en           = 0;
	WR_VREG( VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", pVdhRegCfg->VdhBasicCfg1);

	//AVM_ADDR
	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	//VAM_ADDR
	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	//STREAM_BASE_ADDR
	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : %pK\n", (void *)(uintptr_t)(UINT32)pVdhRegCfg->VdhStreamBaseAddr);

	//EMAR_ADDR
	D32 = 0x101;
	WR_SCDREG(REG_EMAR_ID, D32);

	//YSTADDR_1D
	D32 = 0;
	((YSTADDR_1D *)(&D32))->ystaddr_1d = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0; //caution
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : %pK\n", (void *)(uintptr_t)D32);

	//YSTRIDE_1D
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	//UVOFFSET_1D
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	D32 = 0;
	WR_VREG(VREG_HEAD_INF_OFFSET, D32, 0);

	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);

	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);
	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 VP8HAL_CheckHeadMsg(MEM_BUFFER_S* pVdhMemMap,UINT32 PicMsgSlotPhy)
{
	UINT32 *pSlot = NULL, *pSlot_base = NULL;
	UINT32 tmp;
	SINT32 ret;

	VDMHAL_ASSERT_RET( pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map head msg virtual address");

	pSlot_base = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_HEAD_SLOT_INDEX  * 4 * USE_MSG_SLOT_SIZE));
	pSlot = pSlot_base;

	//D0 curr frame head_ystaddr
	RD_MSGWORD(pSlot, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "CurPicHeadYPhyAddr is failed");

	//D1 curr frame head_cstaddr
	RD_MSGWORD(pSlot + 1, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "CurPicHeadCPhyAddr is failed");

	//D3
	RD_MSGWORD(pSlot + 3, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "LastRefPicHeadYPhyAddr is failed");

	//D4
	RD_MSGWORD(pSlot + 4, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "LastRefPicHeadCPhyAddr is failed");

	//D5
	RD_MSGWORD(pSlot + 5, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "GoldenRefPicHeadYPhyAddr is failed");

	//D6
	RD_MSGWORD(pSlot + 6, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "GoldenRefPicHeadCPhyAddr is failed");

	//D7
	RD_MSGWORD(pSlot + 7, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "AltRefPicHeadYPhyAddr is failed");

	//D8
	RD_MSGWORD(pSlot + 8, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "AltRefPicHeadCPhyAddr is failed");

	return VDMHAL_OK;
}

static SINT32 VP8HAL_CfgDnMsg(VDMHAL_HWMEM_S *pHwMem, MEM_BUFFER_S* pVdhMemMap,
	UINT32* pPicHeightMb, UINT32* pPicWidthMb)
{
	UINT32 *pMsgBase, *p32;
	UINT32 D32;
	SINT32 ret;
	SINT32 iCount;

	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map down msg virtual address");

	pMsgBase = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	p32 = pMsgBase;
	D32 = 0;

	/* D25 */
	RD_MSGWORD((p32 + 25), D32);
	if ((D32 < pVdhMemMap[VDH_SHAREFD_STREAM_BUF].u32StartPhyAddr)
		|| (D32 > (pVdhMemMap[VDH_SHAREFD_STREAM_BUF].u32StartPhyAddr
			+ pVdhMemMap[VDH_SHAREFD_STREAM_BUF].u32Size))) {
		dprint(PRN_FATAL, "%s check D25 failed \n", __func__);
		return VDMHAL_ERR;
	}

	p32 += 31;
	for (iCount = 0; iCount < 4; iCount++) {
		/* D32 ~ D35 */
		++p32;
		RD_MSGWORD(p32, D32);
		ret = CheckFrmBufAddr(D32,pVdhMemMap);
		if (ret == VDMHAL_ERR) {
			dprint(PRN_FATAL, "%s check D(%d) failed \n", __func__, (32 + iCount));
			return VDMHAL_ERR;
		}
	}

	/* D36 */
	++p32;
	D32 = 0;
	((VP8_PICDNMSG_D36 *) (&D32))->sed_top_addr = (pHwMem->SedTopAddr + 0xF) & 0xFFFFFFF0;  //caution!!
	WR_MSGWORD(p32, D32);

	// D37
	++p32;
	D32 = 0;
	((VP8_PICDNMSG_D37 *) (&D32))->pmv_top_addr = (pHwMem->PmvTopAddr + 0xF) & 0xFFFFFFF0;  //caution!
	WR_MSGWORD(p32, D32);

	// D38
	++p32;
	D32 = 0;
	((VP8_PICDNMSG_D38 *) (&D32))->rcn_top_addr = (pHwMem->RcnTopAddr + 0xF) & 0xFFFFFFF0;  //caution!
	WR_MSGWORD(p32, D32);

	/* D39 */
	++p32;
	D32 = 0;
	((VP8_PICDNMSG_D39 *) (&D32))->tab_addr = (pHwMem->Vp8TabAddr + 0xF) & 0xFFFFFFF0;//caution!
	WR_MSGWORD(p32, D32);

	/* D40 */
	++p32;
	D32 = 0;
	((VP8_PICDNMSG_D40 *) (&D32))->dblk_top_addr = (pHwMem->DblkTopAddr + 0xF) & 0xFFFFFFF0;    //caution!
	WR_MSGWORD(p32, D32);

	/* D63  next msg group -- caution!!1 */
	p32 += 23;
	D32 = pHwMem->MsgSlotAddr[DN_MSG_SLOT_INDEX] + 64 * 4;
	WR_MSGWORD(p32, D32);

	return VDMHAL_OK;
}
#endif

#ifdef MSG_POOL_ADDR_CHECK
SINT32 VP8HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
#else
SINT32 VP8HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
#endif
{
	SINT32 Ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 PicHeightMb;
	UINT32 PicWidthMb;
	VDMHAL_HWMEM_S *pHwMem = &g_HwMem[0];

	if (((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en) {
		Ret = VP8HAL_CheckHeadMsg(pVdhMemMap,pHwMem->MsgSlotAddr[DN_MSG_HEAD_SLOT_INDEX]);
		VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "VP8 head msg check failed");
	}

	Ret = VP8HAL_CfgDnMsg(pHwMem,pVdhMemMap,&PicHeightMb,&PicWidthMb);
	VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "VP8 DnMsg config failed");
#endif
	Ret = VP8HAL_CfgReg(pVdhRegCfg);
	if (Ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "VP8 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
