/*
 * vfmw interface
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kern_levels.h>
#include <linux/sched.h>
#include <linux/kthread.h>

#include "basedef.h"
#include "public.h"
#include "vfmw_intf.h"

#ifdef HI_TVP_SUPPORT
#include "tvp_adapter.h"
#endif

#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#include "smmu_regs.h"
#endif
#include "vdm_hal_api.h"
#include "drv_omxvdec.h"

#ifndef IRQF_DISABLED
#define IRQF_DISABLED               (0x00000020)
#endif
#define VDM_TIMEOUT               (800)//ms
#define VDM_FPGA_TIMEOUT          (5000)//ms
#define SCD_TIMEOUT               (800)//ms
#define SCD_FPGA_TIMEOUT          (200000)//ms
#ifdef SECURE_VS_NOR_SECURE
#define SCEN_IDENT                (0x078)
#else
#define SCEN_IDENT                (0x828)
#endif
#define MAP_SIZE                  (256 * 1024)
#define MIN_VIDEO_MSG_POOL_SIZE   (10 * 1024 * 1024)

#define TIME_PERIOD(begin, end) ((end >= begin)? (end-begin):(0xffffffff - begin + end))

// cppcheck-suppress *
#define  VCTRL_ASSERT_RET(cond, else_print)                                      \
do {                                                                             \
	if (!(cond)) {                                                           \
		dprint(PRN_FATAL,"%s %d %s\n", __func__, __LINE__, else_print ); \
		return VCTRL_ERR;                                                \
	}                                                                        \
}while(0)

static DRV_MEM_S g_RegsBaseAddr;

#ifdef HI_TVP_SUPPORT
#define WAIT_TASK_EXIT_TIMEOUT (20) //ms
#define WAIT_TASK_EXIT_INER    (1)  //ms

typedef struct {
	struct task_struct *task;
	wait_queue_head_t waitq;
	bool suspend;
	bool resume;
} vdec_tvp_info;

static vdec_tvp_info vdec_tvp;
#endif

Vfmw_Osal_Func_Ptr g_vfmw_osal_fun_ptr;

#ifdef MSG_POOL_ADDR_CHECK
SINT32 CheckFrmBufAddr(UADDR SrcFrmAddr, MEM_BUFFER_S* pVdhMemMap)
{
	UINT32 index;
	HI_S32 is_mapped = 0;

	if (!SrcFrmAddr) {
		dprint(PRN_FATAL, "%s SrcFrmAddr is NULL", __func__);
		return VDMHAL_ERR;
	}

	for (index = VDH_SHAREFD_FRM_BUF; index < VDH_SHAREFD_MAX; index++) {
		if (pVdhMemMap[index].u8IsMapped == 0)
			break;

		if ((SrcFrmAddr >= pVdhMemMap[index].startPhyAddr)
			&& (SrcFrmAddr <= (pVdhMemMap[index].startPhyAddr
				+ pVdhMemMap[index].u32Size))) {
			is_mapped = 1;
			break;
		}
	}

	if (!is_mapped) {
		dprint(PRN_FATAL, "%s SrcFrmAddr is out of range (%d)\n", __func__, index);
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 CheckPmvBufAddr(UADDR SrcPmvAddr, MEM_BUFFER_S* pVdhMemMap)
{
	if ((SrcPmvAddr < pVdhMemMap[VDH_SHAREFD_PMV_BUF].startPhyAddr)
		|| (SrcPmvAddr > (pVdhMemMap[VDH_SHAREFD_PMV_BUF].startPhyAddr
			+ pVdhMemMap[VDH_SHAREFD_PMV_BUF].u32Size))) {
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 VCTRL_VDHUnmapMessagePool(MEM_BUFFER_S *pMemMap)
{
	HI_S32 ret;

	if (pMemMap->u8IsMapped == 1) {
		pMemMap->u8IsMapVirtual = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
		ret = VDEC_MEM_UnmapKernel(pMemMap);  //kernel4.14 do not need unmap
		if (ret)
			dprint(PRN_FATAL, "%s Unmap Kernel is error(MESSAGE_POOL)\n", __func__);
#endif
	}
	return VCTRL_OK;
}

SINT32 VCTRL_VDHMapMessagePool(MEM_BUFFER_S* pMemMap, HI_S32 share_fd, HI_BOOL isVdhAllBufRemap)
{
	HI_S32 ret;

	VDEC_SCENE scene = pMemMap->scene;
	if ((1 == pMemMap->u8IsMapped) && (HI_TRUE == isVdhAllBufRemap)) {
		pMemMap->u8IsMapVirtual = 1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
		VDEC_MEM_UnmapKernel(pMemMap);
#else
		//VDEC_MEM_PutMapInfo(pMemMap);
#endif
		memset(pMemMap, 0, sizeof(*pMemMap)); /* unsafe_function_ignore: memset */
		pMemMap->scene = scene;
	}

	if (pMemMap->u8IsMapped == 0) {
		pMemMap->u8IsMapVirtual = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
		ret = VDEC_MEM_MapKernel(share_fd, pMemMap);
#else
		ret = VDEC_MEM_GetMapInfo(share_fd, pMemMap);
#endif
		VCTRL_ASSERT_RET((ret == HI_SUCCESS), "share fd map failed");
		pMemMap->u8IsMapped = 1;
		pMemMap->u32ShareFd = share_fd;

		if (SCENE_VIDEO == pMemMap->scene) {
			ret = VDMHAL_IMP_OpenHAL(pMemMap);
		} else if (SCENE_HEIF == pMemMap->scene) {
			ret = VDMHAL_IMP_OpenHeifHAL(pMemMap);
		}

		if (ret) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
			VDEC_MEM_UnmapKernel(pMemMap);
#else
			VDEC_MEM_PutMapInfo(pMemMap);
#endif
			memset(pMemMap, 0, sizeof(*pMemMap)); /* unsafe_function_ignore: memset */
			dprint(PRN_FATAL, "%s VDMHAL_IMP_OpenHAL is failed\n", __func__);
			return VCTRL_ERR;
		}
	}

	return VCTRL_OK;
}
#endif

#ifdef HI_TVP_SUPPORT
#define NOTIFY_AND_WAIT_TVP_PROCESS(notify_flag) \
do { \
	notify_flag = true; \
	wake_up_interruptible(&vdec_tvp.waitq); \
	sleep_count = 0; \
	do { \
		VFMW_OSAL_mSleep(WAIT_TASK_EXIT_INER); \
		sleep_count++; \
	} while(notify_flag && (sleep_count < (WAIT_TASK_EXIT_TIMEOUT/WAIT_TASK_EXIT_INER))); \
	\
	if (notify_flag) { \
		dprint(PRN_ALWS, "task don't process\n"); \
	} \
	dprint(PRN_ALWS, "notify flag %d, sleep count %d\n", notify_flag, sleep_count); \
} while(0);
#endif

VOID VCTRL_Suspend(VOID)
{
	UINT8 isScdSleep = 0;
	UINT8 isVdmSleep = 0;
	UINT32 sleep_count = 0;
	UINT32 BeginTime, EntrTime, CurTime;

	EntrTime = VFMW_OSAL_GetTimeInMs();

	SCDDRV_PrepareSleep();

	VDMHAL_PrepareSleep();

	BeginTime = VFMW_OSAL_GetTimeInMs();
	do {
		if (SCDDRV_SLEEP_STAGE_SLEEP == SCDDRV_GetSleepStage())
			isScdSleep = 1;

		if (VDMHAL_GetSleepStage() == VDMDRV_SLEEP_STAGE_SLEEP)
			isVdmSleep = 1;

		if ((isScdSleep == 1) && (isVdmSleep == 1)) {
			break;
		} else {
			if (sleep_count > 30) {
				if (isScdSleep != 1) {
					dprint(PRN_FATAL, "Force scd sleep\n");
					SCDDRV_ForceSleep();
				}
				if (isVdmSleep != 1) {
					dprint(PRN_FATAL, "Force vdm sleep\n");
					VDMHAL_ForceSleep();
				}
				break;
			}

			VFMW_OSAL_mSleep(10);
			sleep_count++;
		}
	} while ((isScdSleep != 1) || (isVdmSleep != 1));

#ifdef HI_TVP_SUPPORT
	NOTIFY_AND_WAIT_TVP_PROCESS(vdec_tvp.suspend);
#endif

	CurTime = VFMW_OSAL_GetTimeInMs();
	dprint(PRN_ALWS, "Vfmw suspend totally take %d ms\n", TIME_PERIOD(EntrTime, CurTime));

	return;
}

VOID VCTRL_Resume(VOID)
{
	UINT32 EntrTime, CurTime;
#ifdef HI_TVP_SUPPORT
	UINT32 sleep_count = 0;
#endif

	EntrTime = VFMW_OSAL_GetTimeInMs();

	SMMU_InitGlobalReg();

	SCDDRV_ExitSleep();

	VDMHAL_ExitSleep();

#ifdef HI_TVP_SUPPORT
	NOTIFY_AND_WAIT_TVP_PROCESS(vdec_tvp.resume);
#endif

	CurTime = VFMW_OSAL_GetTimeInMs();
	dprint(PRN_ALWS, "Vfmw resume totally take %d ms\n", TIME_PERIOD(EntrTime, CurTime));

	return;
}

static SINT32 VCTRL_ISR(SINT32 irq, VOID *dev_id)
{
	UINT32 D32;
	D32 = RD_SCDREG(REG_SCD_INI_CLR)&0x1;
	if (D32 == 1)
		SCDDRV_ISR();

	RD_VREG(VREG_INT_STATE, D32, 0);
	if (D32 == 1)
		VDMHAL_ISR(0);

	return IRQ_HANDLED;
}

static SINT32 VCTRL_RequestIrq(UINT32 IrqNumNorm, UINT32 IrqNumProt, UINT32 IrqNumSafe)
{
#if !defined(VDM_BUSY_WAITTING)
	if (VFMW_OSAL_RequestIrq(IrqNumNorm, VCTRL_ISR, IRQF_DISABLED, "vdec_norm_irq", NULL) != 0) {    //for 2.6.24以后
		dprint(PRN_FATAL, "Request vdec norm irq %d failed\n", IrqNumNorm);
		return VCTRL_ERR;
	}
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	if (VFMW_OSAL_RequestIrq(IrqNumProt, VCTRL_ISR, IRQF_DISABLED, "vdec_prot_smmu_irq", NULL) != 0) {    //for 2.6.24以后
		dprint(PRN_FATAL, "Request vdec prot irq %d failed\n", IrqNumProt);
		return VCTRL_ERR;
	}
#endif
#endif

	return VCTRL_OK;
}

static VOID VCTRL_FreeIrq(UINT32 IrqNumNorm, UINT32 IrqNumProt, UINT32 IrqNumSafe)
{
#if !defined(VDM_BUSY_WAITTING)
	VFMW_OSAL_FreeIrq(IrqNumNorm, NULL);
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	VFMW_OSAL_FreeIrq(IrqNumProt, NULL);
#endif
#endif
}

static SINT32 VCTRL_HalInit(VOID)
{
#ifdef HIVDEC_SMMU_SUPPORT
	if (SMMU_Init() != SMMU_OK) {
		dprint(PRN_FATAL, "SMMU_Init failed\n");
		return VCTRL_ERR;
	}
#endif

	SCDDRV_init();
	VDMHAL_IMP_Init();
	SMMU_InitGlobalReg();

	return VCTRL_OK;
}

static VOID VCTRL_HalDeInit(VOID)
{
#ifdef HIVDEC_SMMU_SUPPORT
	SMMU_DeInit();
#endif
	VDMHAL_IMP_DeInit();
	SCDDRV_DeInit();
}

static SINT32 VCTRL_SCDGetAddrInfo(MEM_BUFFER_S* pMemMap, SCD_CONFIG_REG_S *ctrlReg)
{
	HI_S32 ret;
	HI_U32 index;

	VCTRL_ASSERT_RET((HI_NULL != pMemMap), "pMemMap parameter error");
	VCTRL_ASSERT_RET((HI_NULL != ctrlReg), "ctrlReg parameter error");
	VCTRL_ASSERT_RET((ctrlReg->ScdOutputBufNum <= SCD_OUTPUT_BUF_CNT),
                        "scd output buffer num is out of range");

	for (index = 0; index < (SCD_SHAREFD_OUTPUT_BUF + ctrlReg->ScdOutputBufNum); index++) {
		if ((pMemMap[index].u8IsMapped == 0)
				|| (ctrlReg->IsScdAllBufRemap)) {
			pMemMap[index].u8IsMapVirtual = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
			ret = VDEC_MEM_MapKernel(ctrlReg->scd_share_fd[index], &pMemMap[index]);
#else
			ret = VDEC_MEM_GetMapInfo(ctrlReg->scd_share_fd[index], &pMemMap[index]);
#endif
			VCTRL_ASSERT_RET((ret == HI_SUCCESS), "share fd map is failed");
			pMemMap[index].u8IsMapped = 1;
			pMemMap[index].u32ShareFd = ctrlReg->scd_share_fd[index];
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
			VDEC_MEM_UnmapKernel(&pMemMap[index]);
#else
			VDEC_MEM_PutMapInfo(&pMemMap[index]);
#endif
		}
	}
	return VCTRL_OK;
}

/* New Func added for judging current scenario is Video or HEIF */
static SINT32 VDEC_MEM_Check_Scene(MEM_BUFFER_S* pMemMap, HI_S32 share_fd)
{
	HI_S32 ret;
	VDEC_SCENE scene;

	pMemMap->u8IsMapVirtual = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	ret = VDEC_MEM_MapKernel(share_fd, pMemMap);
#else
	ret = VDEC_MEM_GetMapInfo(share_fd, pMemMap);
#endif
	VCTRL_ASSERT_RET((ret == VCTRL_OK), "msg sharefd map failed");

	if (pMemMap->u32Size < MIN_VIDEO_MSG_POOL_SIZE) {
		scene = SCENE_HEIF;
	} else {
		scene = SCENE_VIDEO;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	VDEC_MEM_UnmapKernel(pMemMap);
#else
	VDEC_MEM_PutMapInfo(pMemMap);
#endif
	memset(pMemMap, 0, sizeof(*pMemMap)); /* unsafe_function_ignore: memset */
	pMemMap->scene = scene;

	return VCTRL_OK;
}

static SINT32 VCTRL_GetMsgPoolAddr(MEM_BUFFER_S* pMemMap,
		MEM_BUFFER_S* pComMsgMap, HI_S32 share_fd, HI_BOOL isVdhAllBufRemap) {
	HI_S32 ret;

	if (pMemMap->u8IsMapped != 1) {
		ret = VDEC_MEM_Check_Scene(pMemMap, share_fd);
		if (ret) {
			dprint(PRN_FATAL, "%s %d %s\n", __func__, __LINE__, "msg sharefd map failed");
			return ret;
		}
	}

	if (SCENE_VIDEO == pMemMap->scene) {
		ret = VCTRL_VDHMapMessagePool(pComMsgMap, share_fd, isVdhAllBufRemap);
		VCTRL_ASSERT_RET((ret == VCTRL_OK), "video scenario, msg sharefd map failed");
		memcpy(pMemMap, pComMsgMap, sizeof(MEM_BUFFER_S)); /* unsafe_function_ignore: memcpy */
	} else {
		ret = VCTRL_VDHMapMessagePool(pMemMap, share_fd, isVdhAllBufRemap);
		VCTRL_ASSERT_RET((ret == VCTRL_OK), "heif scenario, msg sharefd map failed");
	}

	return VCTRL_OK;
}

static SINT32 VCTRL_VDHGetAddrInfo(MEM_BUFFER_S* pMemMap, MEM_BUFFER_S* pComMsgMap, OMXVDH_REG_CFG_S * pVdmRegCfg)
{
	HI_S32 ret;
	HI_U32 index;
	HI_S32 *share_fd = &(pVdmRegCfg->vdh_share_fd[0]);
	HI_BOOL isVdhFrmBufRemap = pVdmRegCfg->IsFrmBufRemap;
	HI_BOOL isVdhPmvBufRemap = pVdmRegCfg->IsPmvBufRemap;
	HI_BOOL isVdhAllBufRemap = pVdmRegCfg->IsAllBufRemap;

	VCTRL_ASSERT_RET((HI_NULL != pMemMap), "pMemMap parameter error");
	VCTRL_ASSERT_RET((pVdmRegCfg->vdhFrmBufNum <= MAX_FRAME_NUM), "vdhFrmBufNum error");
	VCTRL_ASSERT_RET((pVdmRegCfg->vdhStreamBufNum <= VDH_STREAM_BUF_CNT), "vdhStreamBufNum error");

	for (index = 0; index < (VDH_SHAREFD_FRM_BUF + pVdmRegCfg->vdhFrmBufNum); index++) {
		/* do not get addr info for addtional stream buffer share fd */
		if ((index >= (VDH_SHAREFD_STREAM_BUF + pVdmRegCfg->vdhStreamBufNum))
			&& (index < VDH_SHAREFD_PMV_BUF)) {
			continue;
		}
#ifdef MSG_POOL_ADDR_CHECK
		if (index == VDH_SHAREFD_MESSAGE_POOL) {
			ret = VCTRL_GetMsgPoolAddr(&pMemMap[index], pComMsgMap,
					share_fd[index], isVdhAllBufRemap);
			VCTRL_ASSERT_RET((ret == VCTRL_OK), "msg sharefd map failed");
			continue;
		}
#endif
		if ((pMemMap[index].u8IsMapped == 1) && (isVdhAllBufRemap)) {
			memset(&pMemMap[index], 0, sizeof(pMemMap[index]));  /* unsafe_function_ignore: memset */
		}
		/* pmv or fmv remap */
		if ((pMemMap[index].u8IsMapped == 1)
			&& (((index == VDH_SHAREFD_PMV_BUF) && (isVdhPmvBufRemap))
				|| ((index >= VDH_SHAREFD_FRM_BUF) && (isVdhFrmBufRemap)))) {
			memset(&pMemMap[index], 0, sizeof(pMemMap[index])); /* unsafe_function_ignore: memset */
		}

		if (pMemMap[index].u8IsMapped == 0) {
			pMemMap[index].u8IsMapVirtual = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
			ret = VDEC_MEM_MapKernel(share_fd[index], &pMemMap[index]);
#else
			ret = VDEC_MEM_GetMapInfo(share_fd[index], &pMemMap[index]);
#endif
			if (ret != HI_SUCCESS) {
				dprint(PRN_FATAL,"share fd map failed, index is %d", index);
				return VCTRL_ERR;
			}
			pMemMap[index].u8IsMapped = 1;
			pMemMap[index].u32ShareFd = share_fd[index];
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
			VDEC_MEM_UnmapKernel(&pMemMap[index]);
#else
			VDEC_MEM_PutMapInfo(&pMemMap[index]);
#endif
		}
	}
	return VCTRL_OK;
}

#ifdef HI_TVP_SUPPORT
static int vdec_tvp_task(void *data)
{
	int ret;

	dprint(PRN_ALWS, "enter vdec tvp task\n");

	if (TVP_VDEC_SecureInit() != VDEC_OK) {
		dprint(PRN_FATAL, "%s, tvp vdec init failed\n", __func__);
	}

	while (!kthread_should_stop()) {
		dprint(PRN_ALWS, "before wait event interruptible\n");
		ret = wait_event_interruptible(
					vdec_tvp.waitq,
					(vdec_tvp.resume ||
					 vdec_tvp.suspend ||
					 (kthread_should_stop())));/*lint !e666*/
		if (ret) {
			dprint(PRN_FATAL, "wait event interruptible failed\n");
			continue;
		}
		dprint(PRN_ALWS, "after wait event interruptible\n");

		if (vdec_tvp.resume) {
			dprint(PRN_ALWS, "tvp vdec resume\n");
			if (TVP_VDEC_Resume() != VDEC_OK) {
				dprint(PRN_FATAL, "tvp vdec resume failed\n");
			}
			vdec_tvp.resume = false;
		}

		if (vdec_tvp.suspend) {
			dprint(PRN_ALWS, "tvp vdec supend\n");
			if (TVP_VDEC_Suspend() != VDEC_OK) {
				dprint(PRN_FATAL, "tvp vdec suspend failed\n");
			}
			vdec_tvp.suspend = false;
		}
	}

	TVP_VDEC_SecureExit();

	dprint(PRN_ALWS, "exit vdec tvp task\n");

	return HI_SUCCESS;
}
#endif

SINT32 VCTRL_OpenDrivers(VOID)
{
	MEM_RECORD_S *pstMem;
	SINT32 ret   = VCTRL_ERR;

	pstMem = &g_RegsBaseAddr.stVdhReg;
	if (MEM_MapRegisterAddr(gVdhRegBaseAddr, MAP_SIZE, pstMem) == MEM_MAN_OK) {
		if (MEM_AddMemRecord(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length) != MEM_MAN_OK) {
			dprint(PRN_ERROR, "%s %d MEM_AddMemRecord failed\n", __func__, __LINE__);
			goto exit;
		}
	} else {
		dprint(PRN_FATAL, "Map vdh register failed! gVdhRegBaseAddr : %pK, gVdhRegRange : %d\n",
			(void *)(uintptr_t)gVdhRegBaseAddr, gVdhRegRange);
		goto exit;
	}

	ret = VCTRL_RequestIrq(gVdecIrqNumNorm, gVdecIrqNumProt, gVdecIrqNumSafe);
	if (ret != VCTRL_OK) {
		dprint(PRN_FATAL, "VCTRL_RequestIrq failed\n");
		goto exit;
	}

	if (VCTRL_HalInit() != VCTRL_OK) {
		dprint(PRN_FATAL, "VCTRL_HalInit failed\n");
		goto exit;
	}

	VFMW_OSAL_InitEvent(G_SCDHWDONEEVENT, 0);
	VFMW_OSAL_InitEvent(G_VDMHWDONEEVENT, 0);

	return VCTRL_OK;

exit:
	VCTRL_CloseVfmw();
	return VCTRL_ERR;
}

SINT32 VCTRL_OpenVfmw(VOID)
{
	memset(&g_RegsBaseAddr, 0, sizeof(g_RegsBaseAddr)); /* unsafe_function_ignore: memset */
	MEM_InitMemManager();
	if (VCTRL_OpenDrivers() != VCTRL_OK) {
		dprint(PRN_FATAL, "OpenDrivers fail\n");
		return VCTRL_ERR;
	}

#ifdef HI_TVP_SUPPORT
	if (!vdec_tvp.task) {
		memset(&vdec_tvp, 0, sizeof(vdec_tvp));/* unsafe_function_ignore: memset */
		init_waitqueue_head(&vdec_tvp.waitq);

		vdec_tvp.task = kthread_run(vdec_tvp_task, NULL, "vdec tvp task");
		if (IS_ERR(vdec_tvp.task)) {
			dprint(PRN_FATAL, "creat vdec tvp task failed\n"); // needn't return fail for normal video play
			memset(&vdec_tvp, 0, sizeof(vdec_tvp)); /* unsafe_function_ignore: memset */
		}
	}
#endif

	return VCTRL_OK;
}

SINT32 VCTRL_CloseVfmw(VOID)
{
	MEM_RECORD_S *pstMem;

	VCTRL_HalDeInit();

	pstMem = &g_RegsBaseAddr.stVdhReg;
	if (pstMem->Length != 0) {
		MEM_UnmapRegisterAddr(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length);
		MEM_DelMemRecord(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length);
		memset(&g_RegsBaseAddr.stVdhReg, 0, sizeof(g_RegsBaseAddr.stVdhReg)); /* unsafe_function_ignore: memset */
	}

	VCTRL_FreeIrq(gVdecIrqNumNorm, gVdecIrqNumProt, gVdecIrqNumSafe);

#ifdef HI_TVP_SUPPORT
	if (vdec_tvp.task) {
		kthread_stop(vdec_tvp.task);
		memset(&vdec_tvp, 0, sizeof(vdec_tvp)); /* unsafe_function_ignore: memset */
	}
#endif
	return VCTRL_OK;
}

SINT32 VCTRL_VDMHal_Process(OMXVDH_REG_CFG_S *pVdmRegCfg, VDMHAL_BACKUP_S *pVdmRegState,
				MEM_BUFFER_S* pVdhMemMap, MEM_BUFFER_S* pComMsgMap)
{
	HI_S32 ret = HI_SUCCESS;
	VDMDRV_SLEEP_STAGE_E sleepState;

	sleepState = VDMHAL_GetSleepStage();
	if (VDMDRV_SLEEP_STAGE_SLEEP == sleepState) {
		dprint(PRN_ALWS, "vdm sleep state\n");
		return HI_FAILURE;
	}

	if (pVdmRegCfg->vdh_reset_flag)
		VDMHAL_IMP_ResetVdm(0);

	ret = VCTRL_VDHGetAddrInfo(&pVdhMemMap[0], pComMsgMap, pVdmRegCfg);
	if (ret) {
		dprint(PRN_FATAL, "VCTRL_VDHGetAddrInfo is failed\n");
		return HI_FAILURE;
	}

	VFMW_OSAL_InitEvent(G_VDMHWDONEEVENT, 0);
	ret = VDMHAL_HwDecProc(pVdmRegCfg, pVdhMemMap);

	if (ret) {
		dprint(PRN_FATAL, "%s config error\n", __func__);
	} else {
		ret = VFMW_OSAL_WaitEvent(G_VDMHWDONEEVENT, ((0 == gIsFPGA) ? VDM_TIMEOUT : VDM_FPGA_TIMEOUT));
		if (ret == HI_SUCCESS) {
			VDMHAL_GetRegState(pVdmRegState);
		} else {
			dprint(PRN_FATAL, "VFMW_OSAL_WaitEvent wait time out\n");
			VDMHAL_IMP_ResetVdm(0);
		}
	}

	sleepState = VDMHAL_GetSleepStage();
	if (sleepState == VDMDRV_SLEEP_STAGE_PREPARE)
		VDMHAL_SetSleepStage(VDMDRV_SLEEP_STAGE_SLEEP);

	return ret;
}

SINT32 VCTRL_SCDHal_Process(OMXSCD_REG_CFG_S *pScdRegCfg,SCD_STATE_REG_S *pScdStateReg, MEM_BUFFER_S* pScdMemMap)
{
	HI_S32 ret = HI_SUCCESS;
	SCDDRV_SLEEP_STAGE_E sleepState;
	CONFIG_SCD_CMD cmd = pScdRegCfg->cmd;

	sleepState = SCDDRV_GetSleepStage();
	if (SCDDRV_SLEEP_STAGE_SLEEP == sleepState) {
		dprint(PRN_ALWS, "SCD sleep state\n");
		return HI_FAILURE;
	}

	if (pScdRegCfg->SResetFlag) {
		if (SCDDRV_ResetSCD() != HI_SUCCESS) {
			dprint(PRN_FATAL, "VDEC_IOCTL_SCD_WAIT_HW_DONE  Reset SCD failed\n");
			return HI_FAILURE;
		}
	}

	ret = VCTRL_SCDGetAddrInfo(&pScdMemMap[0], &(pScdRegCfg->SmCtrlReg));
	if (ret != VCTRL_OK) {
		dprint(PRN_FATAL, "memory map failure\n");
		return HI_FAILURE;
	}

	switch (cmd) {
	case CONFIG_SCD_REG_CMD:
		VFMW_OSAL_InitEvent(G_SCDHWDONEEVENT, 0);
		ret = SCDDRV_WriteReg(&pScdRegCfg->SmCtrlReg, pScdMemMap);
		if (ret != HI_SUCCESS) {
			dprint(PRN_FATAL, "SCD busy\n");
			return HI_FAILURE;
		}

		ret = VFMW_OSAL_WaitEvent(G_SCDHWDONEEVENT, ((0 == gIsFPGA) ? SCD_TIMEOUT : SCD_FPGA_TIMEOUT));
		if (ret == HI_SUCCESS) {
			SCDDRV_GetRegState(pScdStateReg);
		} else {
			dprint(PRN_ALWS, "VDEC_IOCTL_SCD_WAIT_HW_DONE  wait time out\n");
			SCDDRV_ResetSCD();
		}
		WR_SCDREG(REG_SCD_INT_MASK, 1);
		sleepState = SCDDRV_GetSleepStage();
		if (sleepState == SCDDRV_SLEEP_STAGE_PREPARE) {
			SCDDRV_SetSleepStage(SCDDRV_SLEEP_STAGE_SLEEP);
		}
		break;

	default:
		dprint(PRN_FATAL, " cmd type unknown:%d\n", cmd);
		return HI_FAILURE;
	}

	return ret;
}

SINT32 VCTRL_VDMHAL_IsRun(VOID)
{
	return VDMHAL_IsVdmRun(0);
}

HI_BOOL VCTRL_Scen_Ident(HI_U32 cmd)
{
	HI_U32 value;
#ifdef SECURE_VS_NOR_SECURE
	RD_VREG(SCEN_IDENT, value, 0);
#else
	value = RD_SCDREG(SCEN_IDENT);
#endif
#ifdef OMXVDEC_TVP_CONFLICT
	if ((value != 0) && (value != current->tgid) && (cmd != VDEC_IOCTL_SET_CLK_RATE)) {
#else
    if (value == current->tgid) {
#endif
		return HI_TRUE;
	}

	return HI_FALSE;
}

HI_S32 VFMW_DRV_ModInit(HI_VOID)
{
	OSAL_InitInterface();
	VFMW_OSAL_SemaInit(G_SCD_SEM);
	VFMW_OSAL_SemaInit(G_VDH_SEM);
	VFMW_OSAL_SemaInit(G_BPD_SEM);

	VFMW_OSAL_SpinLockInit(G_SPINLOCK_SCD);
	VFMW_OSAL_SpinLockInit(G_SPINLOCK_VDH);
	VFMW_OSAL_SpinLockInit(G_SPINLOCK_RECORD);
	VFMW_OSAL_InitEvent(G_SCDHWDONEEVENT, 0);
	VFMW_OSAL_InitEvent(G_VDMHWDONEEVENT, 0);

#ifdef MODULE
	dprint(PRN_ALWS, "%s : Load hi_vfmw.ko (%d) success\n", __func__, VFMW_VERSION_NUM);
#endif

#ifdef HI_TVP_SUPPORT
	memset(&vdec_tvp, 0, sizeof(vdec_tvp)); /* unsafe_function_ignore: memset */
#endif

	return 0;
}

HI_VOID VFMW_DRV_ModExit(HI_VOID)
{
#ifdef MODULE
	dprint(PRN_ALWS, "%s : Unload hi_vfmw.ko (%d) success\n",__func__, VFMW_VERSION_NUM);
#endif

	return;
}

module_init(VFMW_DRV_ModInit);
module_exit(VFMW_DRV_ModExit);

MODULE_AUTHOR("gaoyajun");
MODULE_LICENSE("GPL");
