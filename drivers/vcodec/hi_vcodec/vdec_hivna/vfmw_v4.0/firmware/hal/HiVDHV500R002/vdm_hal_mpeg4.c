/*
 * vdec hal for mp4
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
#include "vdm_hal_mpeg4.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 MP4HAL_CfgDnMsg(MEM_BUFFER_S* pVdhMemMap, UINT32* pPicHeightMb, UINT32* pPicWidthMb)
{
	UINT8 *pMsgBlock;
	UINT32 *pReg;
	UINT32 Dat;
	SINT32 ret;

	VDMHAL_ASSERT_RET(NULL != pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr,
		"can not map down msg virtual address");

	/* step1: basic config < D0 ~ D7 > */
	pMsgBlock = (UINT8*)pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE);

	/* step3: space arrangement < D12 ~ D23 > */
	/* D12: image_curr_recon_addr */
	pReg = (UINT32 *) (pMsgBlock + 4 * 8) + 4;

	RD_MSGWORD(pReg, Dat);
	ret = CheckFrmBufAddr(Dat, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D12 is failed");

	/* D13: image_forward_ref_addr */
	pReg++;
	RD_MSGWORD(pReg, Dat);
	ret = CheckFrmBufAddr(Dat, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D13 is failed");

	/* D14: image_backward_ref_addr */
	pReg++;
	RD_MSGWORD(pReg, Dat);
	ret = CheckFrmBufAddr(Dat, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D14 is failed");

	/* D15: pmv_colmb_addr */
	pReg++;
	RD_MSGWORD(pReg, Dat);
	ret = CheckPmvBufAddr(Dat, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D15 is failed");


	/* D16: pmv_backward_ref_addr */
	pReg++;
	RD_MSGWORD(pReg, Dat);
	ret = CheckPmvBufAddr(Dat, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D16 is failed");

	/* D17: sed_top_addr */
	Dat = (g_HwMem[0].ItransTopAddr + 0xF) & 0xFFFFFFF0;
	pReg++;
	WR_MSGWORD(pReg, Dat);

	/* D18: pmv_top_addr */
	Dat = (g_HwMem[0].PmvTopAddr + 0xF) & 0xFFFFFFF0;
	pReg++;
	WR_MSGWORD(pReg, Dat);

	/* W' : D60 */
	pReg = (UINT32 *) (pMsgBlock + 4 * 60);
	Dat = g_HwMem[0].SedTopAddr & 0xFFFFFFF0;
	WR_MSGWORD(pReg, Dat);

	/* W' : D63 */
	pReg = (UINT32 *) (pMsgBlock + 4 * 63);
	Dat = g_HwMem[0].MsgSlotAddr[DN_MSG_SLOT_INDEX] + 4 * 64;
	WR_MSGWORD(pReg, Dat);

	return VDMHAL_OK;
}
#endif
static SINT32 MP4HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	UINT32 D32;
	SINT32 SlotWidth;

	//BASIC_CFG0
	D32 = 0;
	D32 = (pVdhRegCfg->VdhBasicCfg0 & 0x000FFFFF) // [15:0] mbamt_to_dec
		  | ( 1 << 22 )
		  | ( 0 << 23 )
		  | ( 0 << 24 )
		  | ( 1 << 25 )
		  | ( 1 << 30 )     // ld_qmatrix_flag
#ifdef ENV_SOS_KERNEL
		  | ( 1 << 31 );    // Secure Mode
#else
		  | ( 0 << 31 );    // Normal Mode
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", D32);

	/*set uv order 0: v first; 1: u first */
	D32 = 0x2               // [3:0] video_standard
		  | (((pVdhRegCfg->VdhBasicCfg1 >> 13) & 0x1) << 13 )     // uv_order_en
		  | ( 1 << 14 )     // [14] fst_slc_grp
		  | ( 1 << 15 )     // [15] mv_output_en
		  | ( 1 << 16 )     // [27:16] max_slcgrp_num
		  | ( 0 << 28 )      // line_num_output_en
		  | ( 1 << 29 )
		  | ( 0 << 30 )   //compress_en
		  | ( 0 << 31 );    // [31] ppfd_en   0==not ppfd dec
	WR_VREG(VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", D32);

	D32 = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;  // mpeg4 down msg
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	D32 = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;  // mpeg4 up msg
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : %pK\n", (void *)(uintptr_t)(UINT32)pVdhRegCfg->VdhStreamBaseAddr);

	//EMAR_ADDR
	D32 = 0x101;

	SlotWidth = pVdhRegCfg->VdhYstride / 8;
	if (SlotWidth > 1920)
	{
		D32 = D32 & (~(0x100));
	}
	else
	{
		D32 = D32 | (0x100);
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

	D32 = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : %pK\n", (void *)(uintptr_t)D32);

	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	D32 = 0;
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "PRCMEM2_1D_CNT : 0x%x\n", D32);

	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 MP4HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
#else
SINT32 MP4HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
#endif
{
	SINT32 ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 PicHeightMb;
	UINT32 PicWidthMb;

	ret = MP4HAL_CfgDnMsg(pVdhMemMap,&PicHeightMb,&PicWidthMb);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "MP4 DnMsg config failed");
#endif
	ret = MP4HAL_CfgReg(pVdhRegCfg);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP4 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
