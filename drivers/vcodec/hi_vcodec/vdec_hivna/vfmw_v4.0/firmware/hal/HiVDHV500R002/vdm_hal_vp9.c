/*-----------------------------------------------------------------------*/
/*!!Warning: Huawei key information asset. No spread without permission. */
/*CODEMARK:EG4uRhTwMmgcVFBsBnYHCDadN5jJKSuVyxmmaCmKFU6eJEbB2fyHF9weu4/jer/hxLHb+S1e
E0zVg4C3NiZh4Rryzsvo1gOdvy7M+qFCBFQKTTAFAVC3Q4e533WXdeQrddo4r2cqTmRg3Xeb
SI3trXaSV012ETxvJrJ/pkfs27/lT6wemL9iW3PaGW8//pmW7hQ7qCDBgWp7sMvcMuyYAWRh
jMb6+4xlgVl55z+iUl5XDCi0pMRG2hXB2hXZd5i/HJastZrWJFR4dVOatPlImg==#*/
/*--!!Warning: Deleting or modifying the preceding information is prohibited.--*/





#ifndef __VDM_HAL_VP9_C__
#define __VDM_HAL_VP9_C__

#include "public.h"
#include "vdm_hal.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_vp9.h"


#ifdef MSG_POOL_ADDR_CHECK
static SINT32 VP9HAL_CheckHeadMsg(MEM_BUFFER_S* pVdhMemMap, SINT32 PicMsgSlotPhy)
{
	UINT32 *pSlot = NULL, *pSlot_base = NULL;
	UINT32 tmp, i, offset;
	SINT32 ret;

	VDMHAL_ASSERT_RET( pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map head msg virtual address");

	pSlot_base = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_HEAD_SLOT_INDEX  * 4 * USE_MSG_SLOT_SIZE));
	pSlot = pSlot_base;

	//D0 curr frame head_ystaddr
	RD_MSGWORD(pSlot, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr is failed");

	//D1 curr frame head_cstaddr
	RD_MSGWORD(pSlot + 1, tmp);
	ret = CheckFrmBufAddr(tmp,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadCPhyAddr is failed");

	for (i = 0; i < 3; i++) {
		//D(2*i + 3)
		offset = 2 * i + 3;
		RD_MSGWORD(pSlot + offset, tmp);
		ret = CheckFrmBufAddr(tmp,pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadYPhyAddr Apc2Idc is failed");
		//D(2*i + 4)
		offset = 2 * i + 4;
		RD_MSGWORD(pSlot + offset, tmp);
		ret = CheckFrmBufAddr(tmp,pVdhMemMap);
		VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "HeadCPhyAddr Apc2Idc is failed");
	}

	return VDMHAL_OK;
}

/************************************************************************/
/*  º¯ÊýÊµÏÖ                                                            */
/************************************************************************/
static SINT32 VP9HAL_CfgPicMsg( VDMHAL_HWMEM_S *pHwMem, MEM_BUFFER_S* pVdhMemMap,
	UINT32 *pPicHeightMb, UINT32 *pPicWidthMb)
{
	UINT32 D32;
	UINT32 *pSlot=NULL, *pSlot_base=NULL;
	SINT32 ret;

	VDMHAL_ASSERT_RET( pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr != NULL,
		"can not map down msg virtual address");

	pSlot_base = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));
	pSlot = pSlot_base;

	//D33
	RD_MSGWORD(pSlot + 33, D32);
	ret = CheckFrmBufAddr(D32,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D33 is failed");

	//D34
	RD_MSGWORD(pSlot + 34, D32);
	ret = CheckFrmBufAddr(D32,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D34 is failed");

	//D35
	RD_MSGWORD(pSlot + 35, D32);
	ret = CheckFrmBufAddr(D32,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D35 is failed");

	//D36-D41: vdh phy addr
	//D36
	D32 = pHwMem->SedTopAddr & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 36, D32);

	//D37
	D32 = pHwMem->PmvTopAddr & 0xFFFFFFF0;;
	WR_MSGWORD(pSlot + 37, D32);

	//D38

	D32 = pHwMem->RcnTopAddr & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 38, D32);

	//D39
	D32 = pHwMem->Vp9ProbTabAddr & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 39, D32);

	//D40
	D32 = pHwMem->DblkTopAddr & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 40, D32);

	//D41
	D32 = pHwMem->Vp9ProbCntAddr & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 41, D32);

	//D42

	RD_MSGWORD(pSlot + 42, D32);
	ret = CheckPmvBufAddr(D32, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D42 is failed");

	//D43
	D32 = pHwMem->DblkLeftAddr & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 43, D32);

	//D63
	D32 = pHwMem->MsgSlotAddr[DN_MSG_SLOT_INDEX + 1] & 0xFFFFFFF0;
	WR_MSGWORD(pSlot + 63, D32);

	return VDMHAL_OK;
}
#endif
static SINT32 VP9HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	UINT32 D32;

	//BASIC_CFG0
	D32 = 0;
	((BASIC_CFG0*)(&D32))->mbamt_to_dec      = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0*)(&D32))->load_qmatrix_flag = 0;


	WR_VREG(VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "VREG_BASIC_CFG0 : 0x%x\n", D32);

	//BASIC_CFG1
	D32 = 0;
	((VP9_BASIC_CFG1*)(&D32))->video_standard     = 0x0E;  //VFMW_VP9;

	((VP9_BASIC_CFG1*)(&D32))->uv_order_en        = ((VP9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((VP9_BASIC_CFG1*)(&D32))->fst_slc_grp        = 1;
	((VP9_BASIC_CFG1*)(&D32))->mv_output_en       = 1;
	((VP9_BASIC_CFG1*)(&D32))->max_slcgrp_num     = 3;
	((VP9_BASIC_CFG1*)(&D32))->line_num_output_en = 0;

	((VP9_BASIC_CFG1*)(&D32))->vdh_2d_en          = ((VP9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((VP9_BASIC_CFG1*)(&D32))->frm_cmp_en         = ((VP9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->frm_cmp_en; //for tmp linear
	((VP9_BASIC_CFG1*)(&D32))->ppfd_en            = 0;

	WR_VREG(VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", D32);

	//AVM_ADDR
	D32 = 0;
	((AVM_ADDR*)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	//VAM_ADDR
	D32 = 0;
	((VAM_ADDR*)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : %pK\n", (void *)(uintptr_t)D32);

	//STREAM_BASE_ADDR
	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : %pK\n", (void *)(uintptr_t)(UINT32)pVdhRegCfg->VdhStreamBaseAddr);

	//PRC_CACHE_TYPE
	D32 = 0x0;
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "VREG_FF_APT_EN : 0x%x\n", D32);

	//EMAR_ADDR
	D32 = 0x101;
	WR_SCDREG(REG_EMAR_ID, D32);

	//TIME_OUT
	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO,    D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO,    D32, 0);
	WR_VREG(VREG_PRC_TO,    D32, 0);
	WR_VREG(VREG_RCN_TO,    D32, 0);
	WR_VREG(VREG_DBLK_TO,   D32, 0);
	WR_VREG(VREG_PPFD_TO,   D32, 0);

	//DEC_OVER_INT_LEVEL
	D32 = 60;
	WR_VREG(VREG_PART_DEC_OVER_INT_LEVEL, D32, 0);
	dprint(PRN_VDMREG, "VREG_PART_DEC_OVER_INT_LEVEL:0x%x\n", D32);

	//YSTADDR_1D
	D32 = 0;
	((YSTADDR_1D *)(&D32))->ystaddr_1d = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : %pK\n", (void *)(uintptr_t)D32);

	//YSTRIDE_1D
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	//UVOFFSET_1D
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	//UVSTRIDE_1D
	WR_VREG(VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0);
	dprint(PRN_VDMREG, "UVSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhUvstride);

	//CFGINFO_ADDR
	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%pK\n", (void *)(uintptr_t)(UINT32)pVdhRegCfg->VdhCfgInfoAddr);

	//DDR_INTERLEAVE_MODE
	D32 = 0x3;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 VP9HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
#else
SINT32 VP9HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
#endif
{
	SINT32 Ret;
#ifdef MSG_POOL_ADDR_CHECK
	UINT32 PicHeightMb;
	UINT32 PicWidthMb;
	VDMHAL_HWMEM_S *pHwMem = &g_HwMem[0];

	if (((VP9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->frm_cmp_en) {
		Ret = VP9HAL_CheckHeadMsg(pVdhMemMap,pHwMem->MsgSlotAddr[DN_MSG_HEAD_SLOT_INDEX]);
		VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "VP9 head msg check failed");
	}

	Ret = VP9HAL_CfgPicMsg(pHwMem, pVdhMemMap,&PicHeightMb, &PicWidthMb);
	VDMHAL_ASSERT_RET(Ret == VDMHAL_OK, "VP9 picture msg config failed");
#endif
	Ret = VP9HAL_CfgReg(pVdhRegCfg);
	if (Ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "VP9 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
