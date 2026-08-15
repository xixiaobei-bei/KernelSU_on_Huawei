/*
 * vdec driver for scd master
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#include "public.h"
#include "scd_drv.h"
#include "vfmw_intf.h"
#include "vdm_hal_api.h"
#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#endif
// cppcheck-suppress *
#define SCD_CHECK_CFG_ADDR_RETURN(scdcfg, else_print, startPhy, endPhy) \
do {\
        if ((scdcfg == 0) || (scdcfg < startPhy) || (scdcfg > endPhy)) {\
                dprint(PRN_FATAL, "%s (%s) is out of range \n", __func__, else_print);\
                return SCDDRV_ERR;\
        }\
} while(0)
static SCDDRV_SLEEP_STAGE_E  s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_NONE;
static SCD_STATE_REG_S gScdStateReg;
static SCD_STATE_E s_SCDState = SCD_IDLE;

static VOID PrintScdVtrlReg(VOID);

SINT32 SCDDRV_ResetSCD(VOID)
{
	UINT32 tmp;
	UINT32 i;
	UINT32 reg_rst_ok;
	UINT32 reg;
	UINT32 *pScdResetReg   = NULL;
	UINT32 *pScdResetOkReg = NULL;

	pScdResetReg   = (UINT32 *) MEM_Phy2Vir(gSOFTRST_REQ_Addr);
	pScdResetOkReg = (UINT32 *) MEM_Phy2Vir(gSOFTRST_OK_ADDR);

	if (pScdResetReg == NULL || pScdResetOkReg == NULL) {
		dprint(PRN_FATAL, "scd reset register map fail\n");
		return VF_ERR_SYS;
	}

	tmp = RD_SCDREG(REG_SCD_INT_MASK);


	reg = *(volatile UINT32 *)pScdResetReg;
	*(volatile UINT32 *)pScdResetReg = reg | (UINT32) (1 << SCD_RESET_CTRL_BIT);

	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)pScdResetOkReg;
		if (reg_rst_ok & (1 << SCD_RESET_OK_BIT))
			break;
		VFMW_OSAL_uDelay(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "%s reset failed\n", __func__);
	else
		dprint(PRN_ALWS, "%s reset success\n", __func__);

	*(volatile UINT32 *)pScdResetReg = reg & (UINT32) (~(1 << SCD_RESET_CTRL_BIT));


	WR_SCDREG(REG_SCD_INT_MASK, tmp);

	s_SCDState = SCD_IDLE;
	return FMW_OK;
}

SINT32 SCDDRV_PrepareSleep(VOID)
{
	SINT32 ret = SCDDRV_OK;

	VFMW_OSAL_SemaDown(G_SCD_SEM);
	if (s_eScdDrvSleepStage == SCDDRV_SLEEP_STAGE_NONE) {
		if (SCD_IDLE == s_SCDState) {
			dprint(PRN_ALWS, "%s, idle state \n", __func__);
			s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_SLEEP;
		} else {
			dprint(PRN_ALWS, "%s, decoded state \n", __func__);
			s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_PREPARE;
		}

		ret = SCDDRV_OK;
	} else {
		ret = SCDDRV_ERR;
	}

	VFMW_OSAL_SemaUp(G_SCD_SEM);
	return ret;
}

SCDDRV_SLEEP_STAGE_E SCDDRV_GetSleepStage(VOID)
{
	return s_eScdDrvSleepStage;
}

VOID SCDDRV_SetSleepStage(SCDDRV_SLEEP_STAGE_E sleepState)
{
	VFMW_OSAL_SemaDown(G_SCD_SEM);
	s_eScdDrvSleepStage = sleepState;
	VFMW_OSAL_SemaUp(G_SCD_SEM);
}

VOID SCDDRV_ForceSleep(VOID)
{
	dprint(PRN_ALWS, "%s, force state \n", __func__);
	VFMW_OSAL_SemaDown(G_SCD_SEM);
	if (s_eScdDrvSleepStage != SCDDRV_SLEEP_STAGE_SLEEP) {
		SCDDRV_ResetSCD();
		s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_SLEEP;
	}
	VFMW_OSAL_SemaUp(G_SCD_SEM);
}

VOID SCDDRV_ExitSleep(VOID)
{
	VFMW_OSAL_SemaDown(G_SCD_SEM);
	s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_NONE;
	VFMW_OSAL_SemaUp(G_SCD_SEM);
}

static SINT32 SCDDRV_CheckAddress(UADDR scdcfg, HI_U32 startPhy, HI_U32 endPhy)
{
	if ((scdcfg == 0) || (scdcfg < startPhy) || (scdcfg > endPhy)) {
		return SCDDRV_ERR;
	}

	return SCDDRV_OK;
}

SINT32 SCDDRV_CheckCfgAddress(SCD_CONFIG_REG_S *pSmCtrlReg, MEM_BUFFER_S* pScdMemMap)
{
	UINT32 i;
	SINT32 ret = SCDDRV_OK;
	HI_U32 u32StartOutPutPhyAddr;
	HI_U32 u32EndOutPutPhyAddr;
	/* SINT32 DownMsgMaxOffset = (SM_MAX_DOWNMSG_SIZE + 15) & (~15);  */
	SINT32 UpMsgMaxOffset   = (SM_MAX_UPMSG_SIZE + 15) & (~15);
	UINT32 maxScdBufNum     = (pSmCtrlReg->ScdOutputBufNum + SCD_SHAREFD_OUTPUT_BUF) < SCD_SHAREFD_MAX ?
								(pSmCtrlReg->ScdOutputBufNum + SCD_SHAREFD_OUTPUT_BUF) : SCD_SHAREFD_MAX;

	for (i = SCD_SHAREFD_MESSAGE_POOL; i < maxScdBufNum; i++) {
		if ((INVALID_SHAREFD == pScdMemMap[i].u32ShareFd)
			|| (pScdMemMap[i].u8IsMapped == 0)) {
			dprint(PRN_FATAL, "%s parameter error: buffer no map. index: %d, isMapped: %d, shareFd: %d \n",
				__func__, i, pScdMemMap[i].u8IsMapped, pScdMemMap[i].u32ShareFd);
			return SCDDRV_ERR;
		}
	}

	if ((pSmCtrlReg->DownMsgPhyAddr == 0)
		|| (pSmCtrlReg->DownMsgPhyAddr < pScdMemMap[SCD_SHAREFD_MESSAGE_POOL].startPhyAddr)
		|| (pSmCtrlReg->DownMsgPhyAddr
			> (pScdMemMap[SCD_SHAREFD_MESSAGE_POOL].startPhyAddr
				+ pScdMemMap[SCD_SHAREFD_MESSAGE_POOL].u32Size
				/* - DownMsgMaxOffset */))) {
		dprint(PRN_FATAL, "%s DownMsgPhyAddr is out of range \n", __func__);
		return SCDDRV_ERR;
	}
	if ((pSmCtrlReg->UpMsgPhyAddr == 0)
		|| (pSmCtrlReg->UpMsgPhyAddr < pScdMemMap[SCD_SHAREFD_MESSAGE_POOL].startPhyAddr)
		|| (pSmCtrlReg->UpMsgPhyAddr
			> (pScdMemMap[SCD_SHAREFD_MESSAGE_POOL].startPhyAddr
				+ pScdMemMap[SCD_SHAREFD_MESSAGE_POOL].u32Size
				/* - UpMsgMaxOffset */))) {
		dprint(PRN_FATAL, "%s UpMsgPhyAddr  is out of range \n", __func__);
		return SCDDRV_ERR;
	}
	if ((pSmCtrlReg->UpLen < 0)
		|| (pSmCtrlReg->UpLen > UpMsgMaxOffset)) {
		dprint(PRN_FATAL, "%s UpLen is out of range \n", __func__);
		return SCDDRV_ERR;
	}


	for (i = SCD_SHAREFD_OUTPUT_BUF; i < maxScdBufNum; i++) {
		u32StartOutPutPhyAddr = pScdMemMap[i].startPhyAddr;
		u32EndOutPutPhyAddr   = pScdMemMap[i].startPhyAddr + pScdMemMap[i].u32Size;
		ret = SCDDRV_OK;
		ret += SCDDRV_CheckAddress(pSmCtrlReg->BufferFirst,
			u32StartOutPutPhyAddr, u32EndOutPutPhyAddr);
		ret += SCDDRV_CheckAddress(pSmCtrlReg->BufferLast,
			u32StartOutPutPhyAddr, u32EndOutPutPhyAddr);
		ret += SCDDRV_CheckAddress(pSmCtrlReg->BufferIni,
			pSmCtrlReg->BufferFirst, pSmCtrlReg->BufferLast);

		if (SCDDRV_OK == ret) {
			break;
		}
	}

	return ret;
}

SINT32 SCDDRV_WriteReg(SCD_CONFIG_REG_S *pSmCtrlReg, MEM_BUFFER_S* pScdMemMap)
{
	SINT32 ret;

	if (s_SCDState != SCD_IDLE)
		return SCDDRV_ERR;

	ret = SCDDRV_CheckCfgAddress(pSmCtrlReg, pScdMemMap);
	if (ret) {
		dprint(PRN_FATAL, "SCDDRV_CheckCfgAddress check failed\n");
		return SCDDRV_ERR;
	}

	s_SCDState = SCD_WORKING;
	WR_SCDREG(REG_SCD_INI_CLR, 1);

	// LIST_ADDRESS
	WR_SCDREG(REG_LIST_ADDRESS, (unsigned int)pSmCtrlReg->DownMsgPhyAddr);

	// UP_ADDRESS
	WR_SCDREG(REG_UP_ADDRESS, (UINT32) pSmCtrlReg->UpMsgPhyAddr);

	// UP_LEN
	WR_SCDREG(REG_UP_LEN, (UINT32) pSmCtrlReg->UpLen);

	// BUFFER_FIRST
	WR_SCDREG(REG_BUFFER_FIRST, (UINT32) pSmCtrlReg->BufferFirst);

	// BUFFER_LAST
	WR_SCDREG(REG_BUFFER_LAST, (UINT32) pSmCtrlReg->BufferLast);

	// BUFFER_INI
	WR_SCDREG(REG_BUFFER_INI, (UINT32) pSmCtrlReg->BufferIni);

	// SCD_PROTOCOL
#ifdef ENV_SOS_KERNEL
	WR_SCDREG(REG_SCD_PROTOCOL, (UINT32) ((pSmCtrlReg->ScdLowdlyEnable << 8)
		| (1 << 7)    /* sec mode */
		| ((pSmCtrlReg->SliceCheckFlag << 4) & 0x10)
		| (pSmCtrlReg->ScdProtocol & 0x0f)));
#ifdef HIVDEC_SMMU_SUPPORT
	SMMU_SetMasterReg(SCD, SECURE_ON, SMMU_OFF);
#endif

#else
	WR_SCDREG(REG_SCD_PROTOCOL, (UINT32) ((pSmCtrlReg->ScdLowdlyEnable << 8)
		| ((pSmCtrlReg->SliceCheckFlag << 4) & 0x10)
		| (pSmCtrlReg->ScdProtocol & 0x0f)));
#ifdef HIVDEC_SMMU_SUPPORT
	SMMU_SetMasterReg(SCD, SECURE_OFF, SMMU_ON);
#endif

#endif

#ifndef SCD_BUSY_WAITTING
	WR_SCDREG(REG_SCD_INT_MASK, 0);
#endif

	PrintScdVtrlReg();

	// SCD_START
	WR_SCDREG(REG_SCD_START, 0);
	WR_SCDREG(REG_SCD_START, (UINT32) (pSmCtrlReg->ScdStart & 0x01));

	return SCDDRV_OK;
}

VOID SCDDRV_SaveStateReg(VOID)
{
	gScdStateReg.ScdProtocol = RD_SCDREG(REG_SCD_PROTOCOL);
	gScdStateReg.Scdover     = RD_SCDREG(REG_SCD_OVER);
	gScdStateReg.ScdInt      = RD_SCDREG(REG_SCD_INT);
	gScdStateReg.ScdNum      = RD_SCDREG(REG_SCD_NUM);
	gScdStateReg.ScdRollAddr = RD_SCDREG(REG_ROLL_ADDR);
	gScdStateReg.SrcEaten    = RD_SCDREG(REG_SRC_EATEN);
	gScdStateReg.UpLen       = RD_SCDREG(REG_UP_LEN);
}

VOID SCDDRV_init(VOID)
{
	memset(&gScdStateReg, 0, sizeof(gScdStateReg)); /* unsafe_function_ignore: memset */
	s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_NONE;
	s_SCDState = SCD_IDLE;
}

VOID SCDDRV_DeInit(VOID)
{
	s_eScdDrvSleepStage = SCDDRV_SLEEP_STAGE_NONE;
	s_SCDState = SCD_IDLE;
}

VOID SCDDRV_ISR(VOID)
{
	SINT32 dat = 0;
	dat = RD_SCDREG(REG_SCD_OVER) & 0x01;

	if ((dat & 1) == 0) {
		dprint(PRN_FATAL, "End0: SM_SCDIntServeProc()\n");
		return;
	}

	SCDDRV_SaveStateReg();
	WR_SCDREG(REG_SCD_INI_CLR, 1);
	VFMW_OSAL_GiveEvent(G_SCDHWDONEEVENT);
}

VOID SCDDRV_GetRegState(SCD_STATE_REG_S *pScdStateReg)
{
	memcpy(pScdStateReg, &gScdStateReg, sizeof(*pScdStateReg)); /* unsafe_function_ignore: memcpy */
	s_SCDState = SCD_IDLE;
}

SINT32 WaitSCDFinish(VOID)
{
	SINT32 i;

	if (SCD_WORKING == s_SCDState) {
		for (i = 0; i < SCD_TIME_OUT_COUNT; i++) {
			if ((RD_SCDREG(REG_SCD_OVER) & 1))
				return SCDDRV_OK;
		}

		return SCDDRV_ERR;
	} else {
		return SCDDRV_OK;
	}
}

static VOID PrintScdVtrlReg(VOID)
{
	SCD_CONFIG_REG_S SmCtrlReg;
	memset(&SmCtrlReg, 0, sizeof(SmCtrlReg)); /* unsafe_function_ignore: memset */

	SmCtrlReg.DownMsgPhyAddr = RD_SCDREG(REG_LIST_ADDRESS);
	SmCtrlReg.UpMsgPhyAddr   = RD_SCDREG(REG_UP_ADDRESS);
	SmCtrlReg.UpLen          = RD_SCDREG(REG_UP_LEN);
	SmCtrlReg.BufferFirst    = RD_SCDREG(REG_BUFFER_FIRST);
	SmCtrlReg.BufferLast     = RD_SCDREG(REG_BUFFER_LAST);
	SmCtrlReg.BufferIni      = RD_SCDREG(REG_BUFFER_INI);
	SmCtrlReg.ScdProtocol    = RD_SCDREG(REG_SCD_PROTOCOL);
	SmCtrlReg.ScdStart       = RD_SCDREG(REG_SCD_START);

	dprint(PRN_SCD_REGMSG, "***Print Scd Vtrl Reg Now\n");
	dprint(PRN_SCD_REGMSG, "DownMsgPhyAddr : %pK\n", (void *)(uintptr_t)(SmCtrlReg.DownMsgPhyAddr));
	dprint(PRN_SCD_REGMSG, "UpMsgPhyAddr : %pK\n", (void *)(uintptr_t)(SmCtrlReg.UpMsgPhyAddr));
	dprint(PRN_SCD_REGMSG, "UpLen : %d\n", SmCtrlReg.UpLen);
	dprint(PRN_SCD_REGMSG, "BufferFirst : %pK\n", (void *)(uintptr_t)(SmCtrlReg.BufferFirst));
	dprint(PRN_SCD_REGMSG, "BufferLast : %pK\n", (void *)(uintptr_t)(SmCtrlReg.BufferLast));
	dprint(PRN_SCD_REGMSG, "BufferIni : %pK\n", (void *)(uintptr_t)(SmCtrlReg.BufferIni));
	dprint(PRN_SCD_REGMSG, "ScdProtocol : %d\n", SmCtrlReg.ScdProtocol);
	dprint(PRN_SCD_REGMSG, "ScdStart : %d\n", SmCtrlReg.ScdStart);
}

#ifdef ENV_ARMLINUX_KERNEL
SINT32 SCDDRV_IsScdIdle(VOID)
{
	SINT32 ret = SCDDRV_OK;
	if (SCD_IDLE == s_SCDState) {
		ret = SCDDRV_OK;
	} else if (SCD_WORKING == s_SCDState) {
		ret = SCDDRV_ERR;
	} else {
		ret = SCDDRV_ERR;
		dprint(PRN_ERROR, "%s : s_SCDState : %d is wrong\n", __func__, s_SCDState);
	}
	return ret;
}

#endif
