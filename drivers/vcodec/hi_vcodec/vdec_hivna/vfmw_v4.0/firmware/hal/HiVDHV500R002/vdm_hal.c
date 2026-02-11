/*
 * vdm hal interface
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
#include "scd_drv.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#ifdef VFMW_MPEG2_SUPPORT
#include "vdm_hal_mpeg2.h"
#endif
#ifdef VFMW_H264_SUPPORT
#include "vdm_hal_h264.h"
#endif
#ifdef VFMW_HEVC_SUPPORT
#include "vdm_hal_hevc.h"
#endif
#ifdef VFMW_MPEG4_SUPPORT
#include "vdm_hal_mpeg4.h"
#endif
#ifdef VFMW_VP8_SUPPORT
#include "vdm_hal_vp8.h"
#endif
#ifdef VFMW_VP9_SUPPORT
#include "vdm_hal_vp9.h"
#endif
#include "vfmw_intf.h"
#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#endif

#define HW_HEIF_MEM_SIZE  (2 * 1024 * 1024)

// cppcheck-suppress *
#define VDH_CHECK_CFG_ADDR_EQ_RETURN(vdhcfg, else_print, cmpPhy) \
do {\
        if ((vdhcfg == 0) || (vdhcfg != cmpPhy)) {\
                dprint(PRN_FATAL, "%s (%s) is not equal\n", __func__, else_print);\
                return VDMHAL_ERR;\
        }\
} while(0)

// cppcheck-suppress *
#define VDH_CHECK_CFG_ADDR_RETURN(vdhcfg, else_print, startPhy, endPhy) \
do {\
        if ((vdhcfg == 0) || (vdhcfg < startPhy) || (vdhcfg > endPhy)) {\
                dprint(PRN_FATAL, "%s (%s) is out of range \n", __func__, else_print);\
                return VDMHAL_ERR;\
        }\
} while(0)

VDMHAL_HWMEM_S  g_HwMem[MAX_VDH_NUM];
VDMHAL_BACKUP_S g_VdmRegState;

static VDMDRV_SLEEP_STAGE_E s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
static VDMDRV_STATEMACHINE_E s_VdmState = VDM_IDLE_STATE;

VOID VDMHAL_EnableInt(SINT32 VdhId)
{
	UINT32 D32  = 0xFFFFFFFE;
	SINT32 *p32 = NULL;

	if (VdhId != 0) {
		dprint(PRN_FATAL, "VDH ID is wrong\n");
		return;
	}

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		p32 = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr);
		if (p32 == NULL) {
			dprint(PRN_FATAL, "vdm register virtual address not mapped, reset failed\n");
			return;
		}

		g_HwMem[VdhId].pVdmRegVirAddr = p32;
	}

	WR_VREG(VREG_INT_MASK, D32, VdhId);

	return;
}

static SINT32 VDMHAL_CheckRpCfgAddress(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
{
#ifdef MSG_POOL_ADDR_CHECK
	SINT32 *pMsgBase;
	UINT32 D32;
	SINT32 payloadBufNum;
	SINT32 headBufNum;
	SINT32 ret;
#endif
	VDMHAL_ASSERT_RET(pVdhMemMap != NULL, "pVdhMemMap is NULL");
	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].u8IsMapped != 0, "msg pool is not mapped");
	VDMHAL_ASSERT_RET(pVdhRegCfg->VdhAvmAddr != 0, "vdhAvmAddr is NULL");

	if (pVdhRegCfg->VdhAvmAddr != (pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].startPhyAddr
		+ (RP_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE))) {
		dprint(PRN_FATAL, "%s VdhAvmAddr is error", __func__);
		return VDMHAL_ERR;
	}
#ifdef MSG_POOL_ADDR_CHECK
	VDMHAL_ASSERT_RET(NULL != (SINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr),
			"repair msg virtual address is NULL");

	pMsgBase = (SINT32 *)(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].pStartVirAddr
		+ (RP_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	for (payloadBufNum = 0; payloadBufNum < 4; payloadBufNum++) {
		D32 = 0;
		RD_MSGWORD((pMsgBase + payloadBufNum), D32);
		ret = CheckFrmBufAddr(D32, pVdhMemMap);
		if (ret) {
			dprint(PRN_FATAL, "%s dst addr(%d) check failed \n", __func__, payloadBufNum);
			return VDMHAL_ERR;
		}
	}

	RD_MSGWORD((pMsgBase + 7), D32);
	if (!(D32 & 0x10000))
		return VDMHAL_OK;

	for (headBufNum = 8; headBufNum < 12; headBufNum++) {
		D32 = 0;
		RD_MSGWORD((pMsgBase + headBufNum), D32);
		ret = CheckFrmBufAddr(D32, pVdhMemMap);
		if (ret) {
			dprint(PRN_FATAL, "%s head addr(%d) check failed \n", __func__, headBufNum);
			return VDMHAL_ERR;
		}
	}
#endif
	return VDMHAL_OK;
}

SINT32 VDMHAL_CfgRpReg(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
{
	SINT32 D32 = 0;
	if (VDMHAL_CheckRpCfgAddress(pVdhRegCfg, pVdhMemMap) != VDMHAL_OK)
		return VDMHAL_ERR;
	WR_VREG(VREG_AVM_ADDR, pVdhRegCfg->VdhAvmAddr, 0);

	D32 = 0x2000C203;
	WR_VREG(VREG_BASIC_CFG1, D32, 0);

	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);

	return VDMHAL_OK;
}

#ifdef PLATFORM_HIVCODECV200
static VOID VDMHAL_CfgMemCtlReg(VOID)
{
	UINT32 D32  = 0x03400260;
	WR_VREG(VREG_VDH_ROM_RFT_EMA, D32, 0);
	SMMU_SetMemCtlReg();
}
#endif

VOID VDMHAL_IMP_Init(VOID)
{
	memset(g_HwMem, 0, sizeof(g_HwMem)); /* unsafe_function_ignore: memset */
	memset(&g_VdmRegState, 0, sizeof(g_VdmRegState)); /* unsafe_function_ignore: memset */

	g_HwMem[0].pVdmRegVirAddr  = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr);

	VDMHAL_IMP_GlbReset();
	s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
	s_VdmState = VDM_IDLE_STATE;
}

VOID VDMHAL_IMP_DeInit(VOID)
{
	s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
	s_VdmState = VDM_IDLE_STATE;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 VDMHAL_IMP_OpenHAL(MEM_BUFFER_S* pOpenParam)
{
	UINT32 i = 0;
	UADDR PhyAddr;
	UADDR PhyAddr_HEVC;
	UADDR MemBaseAddr;
	SINT32 Size, VdhId;

	VDMHAL_ASSERT_RET(NULL != pOpenParam, "pOpenParam is error ");
	VDMHAL_ASSERT_RET(0 != pOpenParam->u8IsMapped, "Message no map");

	MemBaseAddr = pOpenParam->startPhyAddr;
	Size = pOpenParam->u32Size;
	VdhId = 0;

	VDMHAL_ASSERT_RET(0 != MemBaseAddr, "MemBaseAddr error");
#ifdef VFMW_HEVC_SUPPORT
	VDMHAL_ASSERT_RET(Size >= HW_HEVC_MEM_SIZE, "VDMHAL_OpenHAL: Size error");
#else
	VDMHAL_ASSERT_RET(Size >= HW_MEM_SIZE, "VDMHAL_OpenHAL: Size error");
#endif

	/* overall info */
	g_HwMem[VdhId].HALMemBaseAddr = (MemBaseAddr + 1023) & (~1023);
	g_HwMem[VdhId].HALMemSize = Size - (g_HwMem[VdhId].HALMemBaseAddr - MemBaseAddr);
	g_HwMem[VdhId].VahbStride = VHB_STRIDE_BYTE;


	/* message pool */
	PhyAddr = g_HwMem[VdhId].HALMemBaseAddr;

#ifdef VFMW_HEVC_SUPPORT
	for (i = 0; i < HEVC_MAX_SLOT_NUM; i++) {
		g_HwMem[VdhId].MsgSlotAddr[i] = PhyAddr + i * 4 * HEVC_ONE_MSG_SLOT_LEN;
	}

	PhyAddr += HEVC_MAX_SLOT_NUM * 4 * HEVC_ONE_MSG_SLOT_LEN;
	g_HwMem[VdhId].ValidMsgSlotNum = HEVC_MAX_SLOT_NUM;
#else
	for (i = 0; i < MSG_SLOT_NUM; i++) {
		g_HwMem[VdhId].MsgSlotAddr[i] = PhyAddr + i * 4 * MSG_SLOT_SIZE;
	}

	PhyAddr += MSG_SLOT_NUM * 4 * MSG_SLOT_SIZE;
	g_HwMem[VdhId].ValidMsgSlotNum = MSG_SLOT_NUM;
#endif

	/*H264MnAddr mn_phy_addr */
	/*cabac mn tab */
	PhyAddr = (PhyAddr + 1023) & (~1023);
	g_HwMem[VdhId].H264MnAddr = PhyAddr;
	PhyAddr += (CA_MN_ADDR_LEN + 0xF) & (~0xF);

#ifdef VFMW_HEVC_SUPPORT
	PhyAddr = (PhyAddr + 1023) & (~1023);
	g_HwMem[VdhId].mn_phy_addr = PhyAddr;
	PhyAddr += (CA_HEVC_MN_ADDR_LEN + 0xF) & (~0xF);
#endif

	/* code table */
	PhyAddr_HEVC = PhyAddr = (PhyAddr + 0x8000) & (~0x8000);
	g_HwMem[VdhId].DblkTopAddr = PhyAddr;
	PhyAddr += DBLK_TOP_ADDR_LEN;
	g_HwMem[VdhId].ItransTopAddr = PhyAddr;
	PhyAddr += ITRANS_TOP_ADDR_LEN;
	/*sed_top + pmv_top + rcn_top*/
#ifndef VFMW_VP9_SUPPORT
	g_HwMem[VdhId].SedTopAddr = PhyAddr;
	PhyAddr += SED_TOP_ADDR_LEN;

	g_HwMem[VdhId].PmvTopAddr = PhyAddr;
	PhyAddr += PMV_TOP_ADDR_LEN;

	g_HwMem[VdhId].RcnTopAddr = PhyAddr;
	PhyAddr += 0;

#else
	g_HwMem[VdhId].SedTopAddr = PhyAddr;
	PhyAddr += VP9_SED_TOP_ADDR_LEN;

	g_HwMem[VdhId].PmvTopAddr = PhyAddr;
	PhyAddr += VP9_PMV_TOP_ADDR_LEN;

	g_HwMem[VdhId].RcnTopAddr = PhyAddr;
	PhyAddr += VP9_RCN_TOP_ADDR_LEN;

	g_HwMem[VdhId].DblkTopAddr = PhyAddr;
	PhyAddr += VP9_DBLK_TOP_ADDR_LEN;

	g_HwMem[VdhId].DblkLeftAddr = PhyAddr;
	PhyAddr += VP9_DBLK_LEFT_ADDR_LEN;
#endif
	g_HwMem[VdhId].PpfdBufAddr = PhyAddr;
	g_HwMem[VdhId].PpfdBufLen = (PPFD_BUF_LEN_DEFAULT / 128) - 1;
	PhyAddr += (PPFD_BUF_LEN_DEFAULT + 0xF) & (~0xF);

	g_HwMem[VdhId].IntensityConvTabAddr = PhyAddr;
	PhyAddr += INTENSITY_CONV_TAB_ADDR_LEN;

	g_HwMem[VdhId].Vp8TabAddr = PhyAddr;
	PhyAddr += VP8_TAB_ADDR_LEN;

	g_HwMem[VdhId].Vp6TabAddr = PhyAddr;
	PhyAddr += VP6_TAB_ADDR_LEN;
#ifdef VFMW_VP9_SUPPORT
	g_HwMem[VdhId].Vp9ProbTabAddr = PhyAddr;
	PhyAddr += VP9_PROB_TAB_ADDR_LEN;
	g_HwMem[VdhId].Vp9ProbCntAddr = PhyAddr;
#endif
#ifdef VFMW_HEVC_SUPPORT
	g_HwMem[VdhId].sed_top_phy_addr = PhyAddr_HEVC;

	if (HEVC_MAX_PIX_WIDTH > 4096)	{
		PhyAddr_HEVC += (HEVC_SED_TOP_ADDR_LEN + 0xF) & (~0xF);
	}

	g_HwMem[VdhId].pmv_top_phy_addr = PhyAddr_HEVC;

	if (HEVC_MAX_PIX_WIDTH > 4096)	{
		PhyAddr_HEVC += (HEVC_PMV_TOP_ADDR_LEN + 0xF) & (~0xF);
	}

	g_HwMem[VdhId].pmv_left_phy_addr = PhyAddr_HEVC;
	PhyAddr_HEVC += (HEVC_PMV_LEFT_ADDR_LEN + 0xF) & (~0xF);

	g_HwMem[VdhId].rcn_top_phy_addr = PhyAddr_HEVC;

	if (HEVC_MAX_PIX_WIDTH > 4096)	 {
		PhyAddr_HEVC += (HEVC_RCN_TOP_ADDR_LEN + 0xF) & (~0xF);
	}

	g_HwMem[VdhId].tile_segment_info_phy_addr = PhyAddr_HEVC;
	PhyAddr_HEVC += (HEVC_TILE_SEGMENT_INFO_LEN + 0xF) & (~0xF);

	g_HwMem[VdhId].dblk_left_phy_addr = PhyAddr_HEVC;
	PhyAddr_HEVC += (HEVC_DBLK_LEFT_ADDR_LEN + 0xF) & (~0xF);

	g_HwMem[VdhId].dblk_top_phy_addr = PhyAddr_HEVC;

	if (HEVC_MAX_PIX_WIDTH > 4096)	{
		PhyAddr_HEVC += (HEVC_DBLK_TOP_ADDR_LEN + 0xF) & (~0xF);
	}

	g_HwMem[VdhId].sao_left_phy_addr = PhyAddr_HEVC;
	PhyAddr_HEVC += (HEVC_SAO_LEFT_ADDR_LEN + 0xF) & (~0xF);

	g_HwMem[VdhId].sao_top_phy_addr = PhyAddr_HEVC;

	if (HEVC_MAX_PIX_WIDTH > 4096)	{
		PhyAddr_HEVC += (HEVC_SAO_TOP_ADDR_LEN + 0xF) & (~0xF);
	}

	g_HwMem[VdhId].ppfd_phy_addr = PhyAddr_HEVC;
	g_HwMem[VdhId].ppfd_buf_len = 0;
#endif
	return VDMHAL_OK;
}


#define HEIF_UPMSG_SLOT_INDEX           (0)
#define HEIF_DOWNMSG_HEAD_SLOT_INDEX    (4)
#define HEIF_DOWNMSG_SLOT_INDEX         (5)
#define HEIF_DOWNMSG_SLICE_SLOT_INDEX   (6)
#define HEIF_DOWNMSG_SLICE_NUM_MAX      (5)

#define HEIF_VDM_BUF_ALIGN_SIZE         (16) //by byte
#define HEIF_MSG_UNIT_SIZE              (4)  //by word
#define HEIF_UPMSG_STEP_SIZE            (4)
// down msg maybe add one slice for fault-tolerant, so up msg max num add 1
#define HEIF_UPMSG_SLICE_NUM_MAX        (HEIF_DOWNMSG_SLICE_NUM_MAX + 1)
#define HEIF_UP_MSG_SIZE                (HEIF_UPMSG_SLICE_NUM_MAX * HEIF_MSG_UNIT_SIZE)//by byte

#define HEIF_BURST_SIZE                 (64)//by word
#define HEIF_BURST_NUM_MAX              (5)
#define HEIF_VDM_MAX_MSG_NUM            (HEIF_DOWNMSG_SLICE_NUM_MAX + HEIF_DOWNMSG_SLICE_SLOT_INDEX)
#define HEIF_VDM_ONE_MSG_SIZE           (HEIF_BURST_SIZE * HEIF_BURST_NUM_MAX)//by word

#define HEIF_HEVC_CA_MN_BUF_SIZE        (1024)
#define HEIF_TILE_SEGMENT_INFO_LEN      (2048)

#define HEIF_PMV_LEFT_BUF_SIZE          (64 * 4 * 4096)//heif needn't it, support it for video stream

#define HEIF_VDM_MSG_BUF_SIZE           (HEIF_VDM_MAX_MSG_NUM * HEIF_MSG_UNIT_SIZE * HEIF_VDM_ONE_MSG_SIZE + \
                                        HEIF_HEVC_CA_MN_BUF_SIZE + \
                                        HEIF_TILE_SEGMENT_INFO_LEN + \
                                        HEIF_PMV_LEFT_BUF_SIZE + \
                                        HEIF_VDM_BUF_ALIGN_SIZE)

SINT32 VDMHAL_IMP_OpenHeifHAL(MEM_BUFFER_S* pOpenParam)
{
	UINT32 i = 0;
	UADDR  PhyAddr;
	UADDR  MemBaseAddr;
	SINT32 Size, VdhId;

	VDMHAL_ASSERT_RET(NULL != pOpenParam, "pOpenParam is error ");
	VDMHAL_ASSERT_RET(0 != pOpenParam->u8IsMapped, "Message no map");

	MemBaseAddr = pOpenParam->startPhyAddr;
	Size = pOpenParam->u32Size;
	VdhId = 1;

	VDMHAL_ASSERT_RET(0 != MemBaseAddr, "MemBaseAddr error");
	VDMHAL_ASSERT_RET(Size >= HEIF_VDM_MSG_BUF_SIZE, "VDMHAL_OpenHAL: Heif Size error");

	/* overall info */
	g_HwMem[VdhId].HALMemBaseAddr = MemBaseAddr ;
	g_HwMem[VdhId].HALMemSize = Size;


	/* message pool */
	PhyAddr = g_HwMem[VdhId].HALMemBaseAddr;
	PhyAddr = (PhyAddr + 0xF) & (~0xF);
	for (i = 0; i < HEIF_VDM_MAX_MSG_NUM; i++) {
		g_HwMem[VdhId].MsgSlotAddr[i] = PhyAddr + i * HEIF_MSG_UNIT_SIZE * HEIF_VDM_ONE_MSG_SIZE;
	}

	PhyAddr += HEIF_VDM_MAX_MSG_NUM * HEIF_MSG_UNIT_SIZE * HEIF_VDM_ONE_MSG_SIZE;
	g_HwMem[VdhId].ValidMsgSlotNum = HEIF_VDM_MAX_MSG_NUM;

	g_HwMem[VdhId].mn_phy_addr = PhyAddr;
	// TODO:determin phy addr len
	PhyAddr += HEIF_HEVC_CA_MN_BUF_SIZE;

	g_HwMem[VdhId].tile_segment_info_phy_addr = PhyAddr;
	PhyAddr += HEIF_TILE_SEGMENT_INFO_LEN;

	g_HwMem[VdhId].pmv_left_phy_addr = PhyAddr;
	PhyAddr += HEIF_PMV_LEFT_BUF_SIZE;

	return VDMHAL_OK;
}

#endif

VOID VDMHAL_IMP_ResetVdm(SINT32 VdhId)
{
	SINT32 i;
	SINT32 tmp = 0;
	UINT32 reg;
	UINT32 reg_rst_ok;
	UINT32 *pVdmResetVirAddr;
	UINT32 *pVdmResetOkVirAddr;

	pVdmResetVirAddr   = (UINT32 *) MEM_Phy2Vir(gSOFTRST_REQ_Addr);
	pVdmResetOkVirAddr = (UINT32 *) MEM_Phy2Vir(gSOFTRST_OK_ADDR);

	if ((pVdmResetVirAddr == NULL)
		|| (pVdmResetOkVirAddr == NULL)
		|| (g_HwMem[VdhId].pVdmRegVirAddr == NULL)) {
		dprint(PRN_FATAL, "map vdm register fail, vir(pVdmResetVirAddr) : (%pK), vir(pVdmResetOkVirAddr) : (%pK)\n", pVdmResetVirAddr, pVdmResetOkVirAddr);
		return;
	}

	RD_VREG(VREG_INT_MASK, tmp, VdhId);

	/* require mfde reset */
	reg = *(volatile UINT32 *)pVdmResetVirAddr;
	*(volatile UINT32 *)pVdmResetVirAddr = reg | (UINT32) (1 << MFDE_RESET_CTRL_BIT);

	/* wait for reset ok */
	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)pVdmResetOkVirAddr;
		if (reg_rst_ok & (1 << MFDE_RESET_OK_BIT))
			break;
		VFMW_OSAL_uDelay(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "%s reset failed\n", __func__);

	/* clear reset require */
	*(volatile UINT32 *)pVdmResetVirAddr = reg & (UINT32) (~(1 << MFDE_RESET_CTRL_BIT));


	WR_VREG(VREG_INT_MASK, tmp, VdhId);
	s_VdmState = VDM_IDLE_STATE;

	return;
}

VOID VDMHAL_IMP_GlbReset(VOID)
{
	SINT32 i;
	UINT32 reg, reg_rst_ok;
	UINT32 *pResetVirAddr   = NULL;
	UINT32 *pResetOKVirAddr = NULL;

	pResetVirAddr   = (UINT32 *) MEM_Phy2Vir(gSOFTRST_REQ_Addr);
	pResetOKVirAddr = (UINT32 *) MEM_Phy2Vir(gSOFTRST_OK_ADDR);

	if (pResetVirAddr == NULL || pResetOKVirAddr == NULL) {
		dprint(PRN_FATAL, "VDMHAL_GlbReset: map vdm register fail, vir(pResetVirAddr) : (%pK), vir(pResetOKVirAddr) : (%pK)\n", pResetVirAddr, pResetOKVirAddr);
		return;
	}


	/* require all reset, include mfde scd bpd */
	reg = *(volatile UINT32 *)pResetVirAddr;
	*(volatile UINT32 *)pResetVirAddr = reg | (UINT32) (1 << ALL_RESET_CTRL_BIT);

	/* wait for reset ok */
	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)pResetOKVirAddr;
		if (reg_rst_ok & (1 << ALL_RESET_OK_BIT))
			break;
		VFMW_OSAL_uDelay(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "Glb Reset Failed\n");

	/* clear reset require */
	*(volatile UINT32 *)pResetVirAddr = reg & (UINT32) (~(1 << ALL_RESET_CTRL_BIT));

#ifdef PLATFORM_HIVCODECV200
	VDMHAL_CfgMemCtlReg();
#endif
	return;
}

VOID VDMHAL_IMP_ClearIntState(SINT32 VdhId)
{
	SINT32 *p32;
	UINT32 D32 = 0xFFFFFFFF;

	if (VdhId > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: VdhId : %d is more than %d\n", __func__, VdhId, (MAX_VDH_NUM - 1));
		return;
	}

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		if ((p32 = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr)) != NULL) {
			g_HwMem[VdhId].pVdmRegVirAddr = p32;
		} else {
			dprint(PRN_FATAL, " %s %d vdm register virtual address not mapped, reset failed\n", __func__, __LINE__);
			return;
		}
	}

	WR_VREG(VREG_INT_STATE, D32, VdhId);

	return;
}

SINT32 VDMHAL_IMP_CheckReg(REG_ID_E reg_id, SINT32 VdhId)
{
	SINT32 *p32;
	SINT32 dat = 0;
	UINT32 reg_type;

	if (VdhId > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: Invalid VdhId is %d\n", __func__, VdhId);
		return VDMHAL_ERR;
	}

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		if ((p32 = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr)) != NULL) {
			g_HwMem[VdhId].pVdmRegVirAddr = p32;
		} else {
			dprint(PRN_FATAL, " %s %d vdm register virtual address not mapped, reset failed\n", __func__, __LINE__);
			return 0;
		}
	}

	switch (reg_id) {
	case VDH_STATE_REG:
		reg_type = VREG_VDH_STATE;
		break;

	case INT_STATE_REG:
		reg_type = VREG_INT_STATE;
		break;

	case INT_MASK_REG:
		reg_type = VREG_INT_MASK;
		break;

	case VCTRL_STATE_REG:
		reg_type = VREG_VCTRL_STATE;
		break;

	default:
		dprint(PRN_FATAL, "%s: unkown reg_id is %d\n", __func__, reg_id);
		return 0;
	}

	RD_VREG(reg_type, dat, 0);
	return dat;
}

SINT32 VDMHAL_IMP_CheckCfgAddress(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
{
	HI_U32 index;
	HI_S32 ret;
	HI_U32 vdhCfg;
	HI_U32 u32MapMsgAddr;
	HI_U32 u32StartStreamPhyAddr;
	HI_U32 u32EndStreamPhyAddr;
	HI_U32 vdhStreamBufNum;
#ifdef VFMW_HEVC_SUPPORT
	HI_S32 msg_slot_size = HEVC_ONE_MSG_SLOT_LEN;
#else
	HI_S32 msg_slot_size = MSG_SLOT_SIZE;
#endif
	HI_S32 is_mapped = 0;

	VDMHAL_ASSERT_RET(pVdhMemMap != NULL, "pVdhMemMap is NULL");
	VDMHAL_ASSERT_RET(pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].u8IsMapped != 0, "MESSAGE_POOL is not map");
	VDMHAL_ASSERT_RET(pVdhRegCfg->vdhStreamBufNum <= VDH_STREAM_BUF_CNT, "stream buffer num is out of range");

	vdhStreamBufNum = VDH_SHAREFD_STREAM_BUF + pVdhRegCfg->vdhStreamBufNum;
	for (index = VDH_SHAREFD_STREAM_BUF; index < vdhStreamBufNum; index++) {
		VDMHAL_ASSERT_RET(pVdhMemMap[index].u8IsMapped != 0, "STREAM_BUF is not map");
		VDMHAL_ASSERT_RET(pVdhMemMap[index].u32ShareFd != INVALID_SHAREFD, "pVdhMemMap fd is 0");
	}

	VDMHAL_ASSERT_RET(pVdhRegCfg->VdhYstAddr != 0, "pVdhRegCfg->VdhYstAddr is NULL");
	VDMHAL_ASSERT_RET((pVdhRegCfg->VdhYstride >= 0) && (pVdhRegCfg->VdhYstride <= MAX_STRIDE),
			"pVdhRegCfg->VdhYstride is error");
	VDMHAL_ASSERT_RET((pVdhRegCfg->VdhUvstride >= 0) && (pVdhRegCfg->VdhUvstride <= pVdhRegCfg->VdhYstride),
			"pVdhRegCfg->VdhUvstride is error");
	VDMHAL_ASSERT_RET((((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec) <= MAX_MB_NUM_IN_PIC,
			"pVdhRegCfg->mbamt_to_dec is error");

	/* down message check */
	u32MapMsgAddr = (pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].startPhyAddr
			+ (DN_MSG_SLOT_INDEX * 4 * msg_slot_size));
	VDH_CHECK_CFG_ADDR_EQ_RETURN(pVdhRegCfg->VdhAvmAddr, "down message", u32MapMsgAddr);

	/* up message check */
	u32MapMsgAddr = (pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].startPhyAddr
			+ (UP_MSG_SLOT_INDEX * 4 * msg_slot_size));
	VDH_CHECK_CFG_ADDR_EQ_RETURN(pVdhRegCfg->VdhVamAddr, "up message", u32MapMsgAddr);

	/* hfbc message check */
	u32MapMsgAddr = (pVdhMemMap[VDH_SHAREFD_MESSAGE_POOL].startPhyAddr
			 + (DN_MSG_HEAD_SLOT_INDEX  * 4 * msg_slot_size));
	VDH_CHECK_CFG_ADDR_EQ_RETURN(pVdhRegCfg->VdhCfgInfoAddr, "hfbc message", u32MapMsgAddr);

	/* stream  check */
	for (index = VDH_SHAREFD_STREAM_BUF; index < vdhStreamBufNum; index++) {
		u32StartStreamPhyAddr = pVdhMemMap[index].startPhyAddr;
		u32EndStreamPhyAddr = (pVdhMemMap[index].startPhyAddr
					+ pVdhMemMap[index].u32Size);
		vdhCfg = ((STREAM_BASE_ADDR *)(&(pVdhRegCfg->VdhStreamBaseAddr)))->stream_base_addr;


		 if ((vdhCfg != 0) && (vdhCfg >= u32StartStreamPhyAddr)
		 		&& (vdhCfg <= u32EndStreamPhyAddr)) {
			ret = VDMHAL_OK;
			break;
		 }

		 ret = VDMHAL_ERR;
	}

	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "%s VdhStreamBaseAddr is out of range \n", __func__);
		return ret;
	}

	/* output  check */
	for (index = VDH_SHAREFD_FRM_BUF; index < VDH_SHAREFD_MAX; index++) {
		if (pVdhMemMap[index].u8IsMapped == 0) {
			break;
		}
		if (((USIGN)(pVdhRegCfg->VdhYstAddr) >= pVdhMemMap[index].startPhyAddr)
			&& ((pVdhRegCfg->VdhUvoffset > 0)
				&& ((HI_U32)pVdhRegCfg->VdhUvoffset < pVdhMemMap[index].u32Size))
			&& ((USIGN)(pVdhRegCfg->VdhYstAddr)
				< (pVdhMemMap[index].startPhyAddr
					+ pVdhMemMap[index].u32Size
					- pVdhRegCfg->VdhUvoffset))) {
			is_mapped = 1;
			break;
		}
	}
	if (is_mapped != 1) {
		dprint(PRN_FATAL, "%s VdhYstAddr is out of range \n", __func__);
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
SINT32 VDMHAL_IMP_PrepareDec(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
{
	VDMHAL_HWMEM_S *pHwMem = &(g_HwMem[0]);
	SINT32 *p32;
	SINT32 ret;

	if (NULL == pHwMem->pVdmRegVirAddr)
	{
		if (NULL != (p32 = (SINT32 *)MEM_Phy2Vir(gVdhRegBaseAddr)) )
		{
			pHwMem->pVdmRegVirAddr = p32;
		}
		else
		{
			dprint(PRN_FATAL, "vdm register virtual address not mapped, VDMHAL_PrepareDecfailed\n");
			return VDMHAL_ERR;
		}
	}

	ret = VDMHAL_IMP_CheckCfgAddress(pVdhRegCfg, pVdhMemMap);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "VDMHAL_IMP_CheckCfgAddress is fail\n");
		return VDMHAL_ERR;
	}

	if (VFMW_AVS == pVdhRegCfg->VidStd)
		WR_SCDREG(REG_AVS_FLAG, 0x00000001);
	else
		WR_SCDREG(REG_AVS_FLAG, 0x00000000);

	WR_SCDREG(REG_VDH_SELRST, 0x00000001);

	switch (pVdhRegCfg->VidStd) {
#ifdef VFMW_H264_SUPPORT
	case VFMW_H264:
#ifdef MSG_POOL_ADDR_CHECK
		return H264HAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return H264HAL_StartDec(pVdhRegCfg);
#endif
#endif
#ifdef VFMW_HEVC_SUPPORT
	case VFMW_HEVC:
#ifdef MSG_POOL_ADDR_CHECK
		return HEVCHAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return HEVCHAL_StartDec(pVdhRegCfg);
#endif
#endif
#ifdef VFMW_MPEG2_SUPPORT
	case VFMW_MPEG2:
#ifdef MSG_POOL_ADDR_CHECK
		return MP2HAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return MP2HAL_StartDec(pVdhRegCfg);
#endif
#endif
#ifdef VFMW_MPEG4_SUPPORT
	case VFMW_MPEG4:
#ifdef MSG_POOL_ADDR_CHECK
		return MP4HAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return MP4HAL_StartDec(pVdhRegCfg);
#endif
#endif
#ifdef VFMW_VP8_SUPPORT
	case VFMW_VP8:
#ifdef MSG_POOL_ADDR_CHECK
		return VP8HAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return VP8HAL_StartDec(pVdhRegCfg);
#endif
#endif
#ifdef VFMW_VP9_SUPPORT
	case VFMW_VP9:
#ifdef MSG_POOL_ADDR_CHECK
		return VP9HAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return VP9HAL_StartDec(pVdhRegCfg);
#endif
#endif
#ifdef VFMW_MVC_SUPPORT
	case VFMW_MVC:
#ifdef MSG_POOL_ADDR_CHECK
		return H264HAL_StartDec(pVdhRegCfg,pVdhMemMap);
#else
		return H264HAL_StartDec(pVdhRegCfg);
#endif
#endif
	default:
		break;
	}

	return VDMHAL_ERR;
}

SINT32 VDMHAL_IsVdmRun(SINT32 VdhId)
{
	SINT32 Data32 = 0;

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		dprint(PRN_FATAL, "VDM register not mapped yet\n");
		return 0;
	}

	RD_VREG(VREG_VCTRL_STATE, Data32, VdhId);

	return (Data32 == 1 ? 0 : 1);
}

SINT32 VDMHAL_IMP_BackupInfo(VOID)
{
	SINT32 i = 0;
	SINT32 regTmp;
	g_VdmRegState.Int_State_Reg = VDMHAL_IMP_CheckReg(INT_STATE_REG, 0);

	RD_VREG(VREG_BASIC_CFG1, g_VdmRegState.BasicCfg1, 0);
	RD_VREG(VREG_VDH_STATE, g_VdmRegState.VdmState, 0);

	RD_VREG(VREG_MB0_QP_IN_CURR_PIC, g_VdmRegState.Mb0QpInCurrPic, 0);
	RD_VREG(VREG_SWITCH_ROUNDING, g_VdmRegState.SwitchRounding, 0);

	{
		RD_VREG(VREG_SED_STA, g_VdmRegState.SedSta, 0);
		RD_VREG(VREG_SED_END0, g_VdmRegState.SedEnd0, 0);
		RD_VREG(VREG_DEC_CYCLEPERPIC, g_VdmRegState.DecCyclePerPic, 0);
		RD_VREG(VREG_RD_BDWIDTH_PERPIC, g_VdmRegState.RdBdwidthPerPic, 0);
		RD_VREG(VREG_WR_BDWIDTH_PERPIC, g_VdmRegState.WrBdWidthPerPic, 0);
		RD_VREG(VREG_RD_REQ_PERPIC, g_VdmRegState.RdReqPerPic, 0);
		RD_VREG(VREG_WR_REQ_PERPIC, g_VdmRegState.WrReqPerPic, 0);
		RD_VREG(VREG_LUMA_SUM_LOW, g_VdmRegState.LumaSumLow, 0);
		RD_VREG(VREG_LUMA_SUM_HIGH, g_VdmRegState.LumaSumHigh, 0);
	}
	for (i = 0; i < 32; i++) {
		regTmp = VREG_LUMA_HISTORGRAM + i * 4;
		RD_VREG(regTmp, g_VdmRegState.LumaHistorgam[i], 0);
	}

	return VDMHAL_OK;
}

VOID VDMHAL_GetRegState(VDMHAL_BACKUP_S *pVdmRegState)
{
	memcpy(pVdmRegState, &g_VdmRegState, sizeof(*pVdmRegState)); /* unsafe_function_ignore: memcpy */
	s_VdmState = VDM_IDLE_STATE;
}

SINT32 VDMHAL_IMP_PrepareRepair(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
{
	VDMHAL_HWMEM_S *pHwMem = &(g_HwMem[0]);
	SINT32 *p32;
	SINT32 ret;

	if (NULL == pVdhRegCfg)
	{
		dprint(PRN_FATAL, "%s: parameter is NULL\n", __func__);
		return VDMHAL_ERR;
	}
	if ( NULL == pHwMem->pVdmRegVirAddr )
	{
		if ( NULL != (p32 = (SINT32 *)MEM_Phy2Vir(gVdhRegBaseAddr)) )
		{
			pHwMem->pVdmRegVirAddr = p32;
		}
		else
		{
			dprint(PRN_FATAL, "vdm register virtual address not mapped, VDMHAL_PrepareRepair failed\n");
			return VDMHAL_ERR;
		}
	}
	if (pVdhRegCfg->RepairTime == FIRST_REPAIR) {
		if (pVdhRegCfg->ValidGroupNum0 > 0)
			ret = VDMHAL_CfgRpReg(pVdhRegCfg, pVdhMemMap);
		else
			ret = VDMHAL_ERR;
	} else if (pVdhRegCfg->RepairTime == SECOND_REPAIR) {
		dprint(PRN_FATAL, "SECOND_REPAIR Parameter Error\n");
		ret = VDMHAL_ERR;
	} else {
		dprint(PRN_FATAL, " parameter error\n");
		ret = VDMHAL_ERR;
	}

	return ret;
}

VOID VDMHAL_IMP_StartHwRepair(SINT32 VdhId)
{
	SINT32 D32 = 0;

	RD_VREG(VREG_BASIC_CFG0, D32, VdhId);
#ifdef ENV_SOS_KERNEL
	D32 = 0x84000000;
#else
	D32 = 0x4000000;
#endif
	WR_VREG(VREG_BASIC_CFG0, D32, VdhId);

#ifdef HIVDEC_SMMU_SUPPORT
#ifdef ENV_SOS_KERNEL
	SMMU_SetMasterReg(MFDE, SECURE_ON, SMMU_OFF);
#else
	SMMU_SetMasterReg(MFDE, SECURE_OFF, SMMU_ON);
#endif
#endif

	VDMHAL_IMP_ClearIntState(VdhId);
	VDMHAL_EnableInt(VdhId);

	VFMW_OSAL_Mb();
	WR_VREG(VREG_VDH_START, 0, VdhId);
	WR_VREG(VREG_VDH_START, 1, VdhId);
	WR_VREG(VREG_VDH_START, 0, VdhId);

	return;
}

VOID VDMHAL_IMP_StartHwDecode(SINT32 VdhId)
{

#ifdef HIVDEC_SMMU_SUPPORT
#ifdef ENV_SOS_KERNEL
	SMMU_SetMasterReg(MFDE, SECURE_ON, SMMU_OFF);
#else
	SMMU_SetMasterReg(MFDE, SECURE_OFF, SMMU_ON);
#endif
#endif

	VDMHAL_IMP_ClearIntState(VdhId);
	VDMHAL_EnableInt(VdhId);

	VFMW_OSAL_Mb();
	WR_VREG(VREG_VDH_START, 0, 0);
	WR_VREG(VREG_VDH_START, 1, 0);
	WR_VREG(VREG_VDH_START, 0, 0);

	return;
}

VOID VDMHAL_ISR(SINT32 VdhId)
{
	VDMHAL_IMP_BackupInfo();
	VDMHAL_IMP_ClearIntState(VdhId);
	VFMW_OSAL_GiveEvent(G_VDMHWDONEEVENT);
}

SINT32 VDMHAL_HwDecProc(OMXVDH_REG_CFG_S *pVdhRegCfg, MEM_BUFFER_S* pVdhMemMap)
{
	SINT32 ret = VDMDRV_ERR;

	s_VdmState = VDM_DECODE_STATE;

	if (pVdhRegCfg->VdhStartHwDecFlag == 1) {
		ret = VDMHAL_IMP_PrepareDec(pVdhRegCfg, pVdhMemMap);
		if (ret == VDMDRV_OK) {
			VDMHAL_IMP_StartHwDecode(0);
		} else {
			ret = VDMDRV_ERR;
			dprint(PRN_FATAL, "%s prepare dec fail \n", __func__);
		}
	} else if (pVdhRegCfg->VdhStartRepairFlag == 1) {
		ret = VDMHAL_IMP_PrepareRepair(pVdhRegCfg, pVdhMemMap);
		if (ret == VDMDRV_OK) {
			VDMHAL_IMP_StartHwRepair(0);
		} else {
			ret = VDMDRV_ERR;
			dprint(PRN_FATAL, "%s prepare repair fail \n", __func__);
		}
	} else {
		ret = VDMDRV_ERR;
		dprint(PRN_FATAL, "%s process type error \n", __func__);
	}

	if (ret != VDMDRV_OK) {
		s_VdmState = VDM_IDLE_STATE;
	}
	return ret;
}

SINT32 VDMHAL_PrepareSleep(VOID)
{
	SINT32 ret = VDMDRV_OK;

	VFMW_OSAL_SemaDown(G_VDH_SEM);
	if (s_eVdmDrvSleepState == VDMDRV_SLEEP_STAGE_NONE) {
		if (VDM_IDLE_STATE == s_VdmState) {
			dprint(PRN_ALWS, "%s, idle state \n", __func__);
			s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_SLEEP;
		} else {
			dprint(PRN_ALWS, "%s, work state \n", __func__);
			s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_PREPARE;
		}

		ret = VDMDRV_OK;
	} else {
		ret = VDMDRV_ERR;
	}

	VFMW_OSAL_SemaUp(G_VDH_SEM);
	return ret;
}

VOID VDMHAL_ForceSleep(VOID)
{
	dprint(PRN_ALWS, "%s, force state \n", __func__);
	VFMW_OSAL_SemaDown(G_VDH_SEM);
	if (s_eVdmDrvSleepState != VDMDRV_SLEEP_STAGE_SLEEP) {
		VDMHAL_IMP_ResetVdm(0);
		s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_SLEEP;
	}

	VFMW_OSAL_SemaUp(G_VDH_SEM);
}

VOID VDMHAL_ExitSleep(VOID)
{
	VFMW_OSAL_SemaDown(G_VDH_SEM);
	s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
	VFMW_OSAL_SemaUp(G_VDH_SEM);
}

VDMDRV_SLEEP_STAGE_E VDMHAL_GetSleepStage(VOID)
{
	return s_eVdmDrvSleepState;
}

VOID VDMHAL_SetSleepStage(VDMDRV_SLEEP_STAGE_E sleepState)
{
	VFMW_OSAL_SemaDown(G_VDH_SEM);
	s_eVdmDrvSleepState = sleepState;
	VFMW_OSAL_SemaUp(G_VDH_SEM);
}
