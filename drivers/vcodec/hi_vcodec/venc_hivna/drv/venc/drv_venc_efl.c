#include "drv_venc_efl.h"
#include "drv_venc_osal.h"
#include "hi_drv_mem.h"


/*lint -e774 -e697 -e838*/
/*lint -e685 -e568 -e687 -e701 -e713 -e574 -e702 -e737*/
HI_U32 gVencIsFPGA          = 0;
HI_U32 gVeduIrqNumNorm      = 0;
HI_U32 gVeduIrqNumPort      = 0;
HI_U32 gVeduIrqNumSafe      = 0;
HI_U32 gVencRegBaseAddr     = 0;
HI_U32 gVencRegRange        = 0;
HI_U64 gSmmuPageBaseAddr    = 0;
HI_U32 gVencQosMode         = 0;

U_FUNC_VCPI_RAWINT    g_hw_done_type ;
VEDU_OSAL_EVENT     g_hw_done_event;

/*******************************************************************/
VeduEfl_IpCtx_S VeduIpCtx;

HI_S32 VENC_SetDtsConfig(VeduEfl_DTS_CONFIG_S *pDtsConfig)
{
	if (!pDtsConfig){
		HI_FATAL_VENC("pDtsConfig is NULL\n");
		return HI_FAILURE;
	}

	if (pDtsConfig->VeduIrqNumNorm == 0 || pDtsConfig->VeduIrqNumProt == 0 || pDtsConfig->VeduIrqNumSafe == 0 || pDtsConfig->VencRegBaseAddr == 0 ||
		pDtsConfig->VencRegRange == 0 || pDtsConfig->SmmuPageBaseAddr == 0){
		HI_ERR_VENC("invalid param, VeduIrqNumNorm:%d, VeduIrqNumProt:%d, VeduIrqNumSafe:%d, VencRegBaseAddr:%pK, VencRegRange:%d, SmmuPageBaseAddr:%pK\n",
			pDtsConfig->VeduIrqNumNorm, pDtsConfig->VeduIrqNumProt, pDtsConfig->VeduIrqNumSafe, (void *)(uintptr_t)(pDtsConfig->VencRegBaseAddr), pDtsConfig->VencRegRange, (void *)(uintptr_t)(pDtsConfig->SmmuPageBaseAddr));
		return HI_FAILURE;
	}
	gVencIsFPGA       = pDtsConfig->IsFPGA;
	gVeduIrqNumNorm   = pDtsConfig->VeduIrqNumNorm;
	gVeduIrqNumPort   = pDtsConfig->VeduIrqNumProt;
	gVeduIrqNumSafe   = pDtsConfig->VeduIrqNumSafe;

	gVencRegBaseAddr  = pDtsConfig->VencRegBaseAddr;
	gVencRegRange     = pDtsConfig->VencRegRange;
	gSmmuPageBaseAddr = pDtsConfig->SmmuPageBaseAddr;

	gVencQosMode      = pDtsConfig->VencQosMode;

	return HI_SUCCESS;
}

static HI_VOID Venc_ISR( HI_VOID )
{
	S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

	HI_DBG_VENC("enter %s ()\n", __func__);

	if (!VeduIpCtx.pRegBase) {
		return ;
	}
	pAllReg  = (S_HEVC_AVC_REGS_TYPE *)VeduIpCtx.pRegBase;/*lint !e826 */

	g_hw_done_type.bits.vcpi_rint_vedu_timeout = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_vedu_timeout;
	g_hw_done_type.bits.vcpi_rint_vedu_slice_end = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_vedu_slice_end;
	g_hw_done_type.bits.vcpi_rint_ve_eop = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;

#ifdef VENC_SIMULATE
	pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop = 0;
#endif
	if (g_hw_done_type.bits.vcpi_rint_vedu_timeout) {
		pAllReg->VEDU_VCPI_INTCLR.u32 = 0x40;
		VENC_DRV_OsalGiveEvent(&g_hw_done_event);
	}

	if (g_hw_done_type.bits.vcpi_rint_vedu_slice_end)
		pAllReg->VEDU_VCPI_INTCLR.u32 = 0x2;

	if (g_hw_done_type.bits.vcpi_rint_ve_eop) {
		pAllReg->VEDU_VCPI_INTCLR.u32 = 0xFFFFFFFF;
		VENC_DRV_OsalGiveEvent(&g_hw_done_event);
	}

	HI_DBG_VENC("out %s ()\n", __func__);
}

/******************************************************************************
Function   :
Description: IP-VEDU & IP-JPGE Open & Close
Calls      :
Input      :
Output     :
Return     :
Others     :
******************************************************************************/
HI_S32 VENC_DRV_EflOpenVedu(HI_VOID)
{
	HI_DBG_VENC("enter %s()\n", __func__);

	HiMemSet((HI_VOID *)&VeduIpCtx, 0, sizeof(VeduIpCtx));

	if (VENC_DRV_OsalLockCreate( &VeduIpCtx.pChnLock ) == HI_FAILURE){
		HI_ERR_VENC("VENC_DRV_OsalLockCreate failed\n");
		return HI_FAILURE;
	}

	VeduIpCtx.pRegBase = (HI_U32 *)HiMmap(gVencRegBaseAddr, gVencRegRange);

	if (!VeduIpCtx.pRegBase){
		HI_ERR_VENC("ioremap failed\n");
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		return HI_FAILURE;
	}

	HI_DBG_VENC("HI_DDR_MEM_Init\n");
	if (HI_SUCCESS != DRV_MEM_INIT()) {
		HI_ERR_VENC("DRV_MEM_INIT failed\n");
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		HiMunmap(VeduIpCtx.pRegBase);
		return HI_FAILURE;
	}

	VeduIpCtx.IpFree = 1;
#ifndef HIVCODECV500
	VENC_HAL_SetSmmuAddr((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
#endif
	VENC_HAL_DisableAllInt((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
	VENC_HAL_ClrAllInt    ((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
#ifdef IRQ_EN
	if (VENC_DRV_OsalIrqInit(gVeduIrqNumNorm, Venc_ISR) == HI_FAILURE){
		HI_ERR_VENC("VENC_DRV_OsalIrqInit failed\n");
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		HiMunmap(VeduIpCtx.pRegBase);
		DRV_MEM_EXIT();
		return HI_FAILURE;
	}
#endif
	/* creat thread to manage channel */
	VeduIpCtx.StopTask    = 0;
	VeduIpCtx.TaskRunning = 0;

	VENC_DRV_OsalInitEvent(&g_hw_done_event, 0);

	HI_DBG_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_EflCloseVedu( HI_VOID )
{
	HI_U32 TimeOutCnt = 0;
#ifdef MD5_WC_EN
	int   i = 0;
	HI_U8 digesttmp[16] ;
	HI_U8 digesttmp2[100] ;
	HiMemSet(digesttmp, 0, 16);
	HiMemSet(digesttmp2, 0, 100);
#endif
	HI_DBG_VENC("enter %s()\n", __func__);
	VeduIpCtx.StopTask = 1;

	while ((VeduIpCtx.TaskRunning) && (TimeOutCnt < 100)) {
		HiSleepMs(1);
		TimeOutCnt ++;
	}

	VENC_HAL_DisableAllInt((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
	VENC_HAL_ClrAllInt((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */

#ifdef IRQ_EN
	VENC_DRV_OsalIrqFree(gVeduIrqNumNorm);
#endif
	HiMunmap(VeduIpCtx.pRegBase);
	DRV_MEM_EXIT();
	VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );

	HI_DBG_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_EflSuspendVedu(HI_VOID)
{
	HI_U32 TimeOutCnt = 0;
	HI_INFO_VENC("enter %s()\n", __func__);

	VeduIpCtx.StopTask = 1;

	while ((VeduIpCtx.TaskRunning) && (TimeOutCnt < 100)) {
		HiSleepMs(1);
		TimeOutCnt ++;
	}

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_EflResumeVedu(HI_VOID)
{
	HI_INFO_VENC("enter %s()\n", __func__);

	VeduIpCtx.StopTask    = 0;
	VeduIpCtx.TaskRunning = 0;

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

