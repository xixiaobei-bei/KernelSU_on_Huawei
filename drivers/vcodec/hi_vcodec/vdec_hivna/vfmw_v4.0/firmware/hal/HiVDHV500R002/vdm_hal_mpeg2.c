/*
 * vdec hal for mp2
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#ifndef __VDM_HAL_MPEG2_C__
#define __VDM_HAL_MPEG2_C__

#include    "public.h"
#include    "vdm_hal_api.h"
#include    "vdm_hal_local.h"
#include    "vdm_hal_mpeg2.h"

#ifdef MSG_POOL_ADDR_CHECK
static SINT32 MP2HAL_CfgDnMsg(VDMHAL_HWMEM_S *pHwMem, MEM_BUFFER_S* pVdhMemMap,
	UINT32* pPicHeightMb, UINT32* pPicWidthMb)
{
	UINT32 *pMsgBase;
	UINT32 D32;
	UADDR SlcDnMsgPhyAddr;
	SINT32 ret;

	VDMHAL_ASSERT_RET(NULL != pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr,
		"can not map down msg virtual address");

	pMsgBase = (UINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (DN_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	D32 = 0;
	RD_MSGWORD(pMsgBase + 4, D32);
	ret = CheckFrmBufAddr(D32,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D4 is failed");

	D32 = 0;
	RD_MSGWORD(pMsgBase + 5, D32);
	ret = CheckFrmBufAddr(D32,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D5 is failed");

	D32 = 0;
	RD_MSGWORD(pMsgBase + 6, D32);
	ret = CheckFrmBufAddr(D32,pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D6 is failed");

	D32 = 0;
	RD_MSGWORD(pMsgBase + 7, D32);
	ret = CheckPmvBufAddr(D32, pVdhMemMap);
	VDMHAL_ASSERT_RET(ret == VDMHAL_OK, "D7 is failed");

	D32 = 0;
	((MP2DNMSG_D48 *) (&D32))->pmv_top_addr = (pHwMem->PmvTopAddr + 0xF) & 0xFFFFFFF0;
	WR_MSGWORD(pMsgBase + 48, D32);

	SlcDnMsgPhyAddr = (pHwMem->MsgSlotAddr[DN_MSG_SLOT_INDEX] & 0xFFFFFFF0) + 64 * 4;

	D32 = 0;
	((MP2DNMSG_D63 *) (&D32))->first_slc_dnmsg_addr = SlcDnMsgPhyAddr;
	WR_MSGWORD(pMsgBase + 63, D32);

	return VDMHAL_OK;
}
#endif
static SINT32 MP2HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;
	D32 = 0;
	((BASIC_CFG0 *)(&D32))->mbamt_to_dec         = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0 *)(&D32))->load_qmatrix_flag    = 1;
	((BASIC_CFG0 *)(&D32))->marker_bit_detect_en = 1;
	((BASIC_CFG0 *)(&D32))->ac_last_detect_en    = 0;
	((BASIC_CFG0 *)(&D32))->coef_idx_detect_en   = 1;
	((BASIC_CFG0 *)(&D32))->vop_type_detect_en   = 0;
#ifdef ENV_SOS_KERNEL
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 1;
#else
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 0;
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0  );

	D32 = 0;
	((BASIC_CFG1 *)(&D32))->video_standard       = 0x3;
	//((BASIC_CFG1 *)(&D32))->ddr_stride           = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1 *)(&D32))->fst_slc_grp          = 1;
	((BASIC_CFG1 *)(&D32))->mv_output_en         = 1;
	((BASIC_CFG1 *)(&D32))->uv_order_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1 *)(&D32))->vdh_2d_en            = 1;
	((BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 3;
	((BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	((BASIC_CFG1 *)(&D32))->line_num_output_en   = 0;
	((BASIC_CFG1 *)(&D32))->compress_en          = 0;
	/*set uv order 0: v first; 1: u first */
	WR_VREG( VREG_BASIC_CFG1, D32, 0 );

	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);

	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);

	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);

	//EMAR_ADDR
	D32 = 0x101;
	WR_SCDREG(REG_EMAR_ID, D32);

	//TIME_OUT
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
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);

	D32 = 0;
	((REF_PIC_TYPE *)(&D32))->ref_pic_type_0 = ((REF_PIC_TYPE *)(&pVdhRegCfg->VdhRefPicType))->ref_pic_type_0;
	((REF_PIC_TYPE *)(&D32))->ref_pic_type_1 = ((REF_PIC_TYPE *)(&pVdhRegCfg->VdhRefPicType))->ref_pic_type_1;
	WR_VREG( VREG_REF_PIC_TYPE, D32, 0 );
	D32 = 0;
	((FF_APT_EN *)(&D32))->ff_apt_en = 0;//USE_FF_APT_EN;
	WR_VREG( VREG_FF_APT_EN, D32, 0 );

	//VREG_UVSTRIDE_1D
	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	//DDR_INTERLEAVE_MODE
	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}
#ifdef MSG_POOL_ADDR_CHECK
SINT32 MP2HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S *pVdhMemMap)
#else
SINT32 MP2HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
#endif
{
#ifdef MSG_POOL_ADDR_CHECK
	VDMHAL_HWMEM_S *pHwMem = &g_HwMem[0];
	UINT32 PicHeightMb;
	UINT32 PicWidthMb;

	if (MP2HAL_CfgDnMsg(pHwMem, pVdhMemMap,&PicHeightMb,&PicWidthMb) != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP2 DnMsg config failed\n");
		return VDMHAL_ERR;
	}
#endif
	if (MP2HAL_CfgReg(pVdhRegCfg) != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP2 register config failed\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

#endif
