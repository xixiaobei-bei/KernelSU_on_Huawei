
#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include "drv_venc_osal.h"
#include "drv_venc.h"
#include "venc_regulator.h"
#include "hal_venc.h"

#define MAX_OPEN_COUNT  3
//#define SCTRL_CHIPID (0xFFF0AE00)
/*lint -e750 -e838 -e715*/
#ifndef VM_RESERVED /*for kernel up to 3.7.0 version*/
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

#define D_VENC_CHECK_CFG_REG_ADDR_RET(MapAddr, CfgRegAddr, MapBufferSize)\
	if (((CfgRegAddr) < (MapAddr)) || ((CfgRegAddr) > ((MapAddr) + (MapBufferSize)))) {\
		return HI_FAILURE;\
	}
#define D_VENC_CHECK_CFG_REG_ENDADDR_RET(MapAddr, CfgRegAddr, MapBufferSize)\
	if (((CfgRegAddr) < ((MapAddr) -1)) || ((CfgRegAddr) > ((MapAddr) + (MapBufferSize)))) {\
		return HI_FAILURE;\
	}

#define D_VENC_CHECK_CFG_REG_STREAMADDR_RET(MapAddr, CfgRegAddr, MapBufferSize, SharedFd)\
	if ((((CfgRegAddr) < (MapAddr))  || ((CfgRegAddr) > ((MapAddr) + (MapBufferSize)))) && ((SharedFd != 0) && ((CfgRegAddr != 0) && (CfgRegAddr != 0xffffffff)))) {\
		return HI_FAILURE;\
	}
#define D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(MapAddr, CfgRegAddr, MapBufferSize, SharedFd)\
	if ((((CfgRegAddr) < ((MapAddr) - 1))  || ((CfgRegAddr) > ((MapAddr) + (MapBufferSize)))) && ((SharedFd != 0) && ((CfgRegAddr != 0) && (CfgRegAddr != 0xffffffff)))) {\
		return HI_FAILURE;\
	}

#define D_VENC_CHECK_SMRX_REG_ADDR_RET(MapAddr, CfgRegAddr, MapBufferSize)\
	if (((CfgRegAddr) < ((MapAddr) >> 15)) || ((CfgRegAddr) > (((MapAddr) + (MapBufferSize)) >> 15))) {\
		return HI_FAILURE;\
	}
#define D_VENC_CHECK_SMRX_REG_ENDADDR_RET(MapAddr, CfgRegAddr, MapBufferSize)\
	if (((CfgRegAddr) < (((MapAddr) - 1) >> 15)) || ((CfgRegAddr) > (((MapAddr) + (MapBufferSize)) >> 15))) {\
		return HI_FAILURE;\
	}

/*============Deviece===============*/
typedef struct {
	dev_t dev;
	struct device* venc_device;
	//struct device* venc_device_2;
	struct cdev cdev;
	struct class* venc_class;
}VENC_ENTRY;

typedef enum {
	UDP_DEFAULT,
	FPGA_ES,
	FPGA_CS,
}VENC_PLATFORM_E;

typedef struct {
	MEM_BUFFER_S internalbuffer;
	MEM_BUFFER_S imagebuffer;
	MEM_BUFFER_S streambuffer[MAX_STREAMBUF_NUM];
	MEM_BUFFER_S streamheadbuffer;
}VENC_MEM_INFO;

struct semaphore g_VencMutex;

HI_BOOL g_vencOpenFlag = HI_FALSE;
HI_BOOL g_vencDevDetected = HI_FALSE;

//VENC device open times
atomic_t g_VencCount = ATOMIC_INIT(0);

HI_S32 VENC_DRV_Resume(struct platform_device *pltdev);
HI_S32 VENC_DRV_Suspend(struct platform_device *pltdev, pm_message_t state);

static HI_S32  VENC_DRV_SetupCdev(VENC_ENTRY *venc, const struct file_operations *fops);
static HI_S32  VENC_DRV_CleanupCdev(VENC_ENTRY *venc);
static HI_S32  VENC_DRV_Probe(struct platform_device * pltdev);
static HI_S32  VENC_DRV_Remove(struct platform_device *pltdev);

extern VeduEfl_IpCtx_S   VeduIpCtx;
extern U_FUNC_VCPI_RAWINT  g_hw_done_type;
extern VEDU_OSAL_EVENT   g_hw_done_event;
extern HI_U32 gVencIsFPGA;

static HI_S32 venc_drv_waithwdone(U_FUNC_VCPI_RAWINT *hw_done_type)
{
	HI_S32 Ret = HI_FAILURE;

	Ret = VENC_DRV_OsalWaitEvent(&g_hw_done_event, ((1 == gVencIsFPGA) ? msecs_to_jiffies(1000000) : msecs_to_jiffies(500)));/*lint !e712 !e747 */

	if (Ret != 0) {
		hw_done_type->u32 = 0;
		HI_ERR_VENC("wait timeout, Ret value is %d\n", Ret);
		return Ret;
	}

	*hw_done_type = g_hw_done_type;
	return Ret;
}

static HI_S32 venc_drv_register_info(VENC_REG_INFO_S *regcfginfo)
{
	HI_S32 Ret = HI_SUCCESS;
	CMD_TYPE cmd  = regcfginfo->cmd;
	switch (cmd) {
	case VENC_SET_CFGREG:
		if (regcfginfo->bResetReg == HI_TRUE)
		{
			Ret = VENC_HAL_ResetReg();
			if (Ret != HI_SUCCESS)
			{
				HI_ERR_VENC("reset venc hal reset reg, Ret:%d\n", Ret);
				break;
			}
		}

		if (regcfginfo->bClkCfg == HI_TRUE)
		{
			Ret = Venc_SetClkRate(regcfginfo->clk_type);
			if (Ret != HI_SUCCESS) {
				HI_WARN_VENC("set venc clkrate failed, Ret:%d\n", Ret);
			}
		}

		VeduHal_CfgReg(regcfginfo);

		VENC_HAL_StartEncode((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */

		Ret = venc_drv_waithwdone(&regcfginfo->hw_done_type) ;

		if((Ret == HI_SUCCESS ) && (!regcfginfo->hw_done_type.bits.vcpi_rint_vedu_timeout))
		{
			VENC_HAL_Get_Reg_Venc(regcfginfo);
			HI_DBG_VENC("get venc hal reg info\n");
		}
		break;
	case VENC_SET_CFGREGSIMPLE:
		VeduHal_CfgRegSimple(regcfginfo);

		VENC_HAL_StartEncode((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */

		Ret = venc_drv_waithwdone(&regcfginfo->hw_done_type) ;

		if((Ret == HI_SUCCESS ) && (!regcfginfo->hw_done_type.bits.vcpi_rint_vedu_timeout))
		{
			VENC_HAL_Get_Reg_Venc(regcfginfo);
			HI_DBG_VENC("get venc hal reg info\n");
		}
		break;
	default:
		HI_ERR_VENC("cmd type unknown:0x%x in default case\n", cmd);
		Ret = HI_FAILURE;
		break;
	}
	return Ret;
}
static HI_S32 VENC_DRV_Open(struct inode *finode, struct file  *ffile)
{
	HI_S32 Ret = 0;

	Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret) {
		HI_FATAL_VENC("Open down interruptible failed\n");
		return HI_FAILURE;
	}

	if (atomic_read(&g_VencCount) == MAX_OPEN_COUNT) {
		HI_FATAL_VENC("open venc too much\n");
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return -EAGAIN;
	}

	if (atomic_inc_return(&g_VencCount) == 1) {
		Ret = VENC_DRV_BoardInit();
		if (Ret != HI_SUCCESS) {
			HI_FATAL_VENC("board init failed, ret value is %d\n", Ret);
			atomic_dec(&g_VencCount);
			HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
			return HI_FAILURE;
		}
		Ret = VENC_DRV_EflOpenVedu();
		if (Ret != HI_SUCCESS) {
			HI_FATAL_VENC("venc firmware layer open failed, ret value is %d\n", Ret);
			atomic_dec(&g_VencCount);
			VENC_DRV_BoardDeinit();
			HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
			return HI_FAILURE;
		}
	}

	g_vencOpenFlag = HI_TRUE;
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);

	HI_INFO_VENC("Open venc device successfully\n");
	return HI_SUCCESS;
}

static HI_S32 VENC_DRV_Close(struct inode *finode, struct file  *ffile)
{
	HI_S32 Ret = 0;

	Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret) {
		HI_FATAL_VENC("Close down interruptible failed\n");
		return HI_FAILURE;
	}

	if (atomic_dec_and_test(&g_VencCount)) {
		Ret = VENC_DRV_EflCloseVedu();
		if (Ret != HI_SUCCESS) {
			HI_FATAL_VENC("venc firmware layer close failed, ret value is %d\n", Ret);
			VENC_DRV_BoardDeinit();
			HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
			return HI_FAILURE;
		}

		VENC_DRV_BoardDeinit();
		g_vencOpenFlag = HI_FALSE;
	}

	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);

	HI_INFO_VENC("Close venc device successfully\n");
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_Suspend(struct platform_device *pltdev, pm_message_t state)
{
	HI_S32 Ret = 0;
	HI_INFO_VENC("+");

	Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret) {
		HI_ERR_VENC("Suspend down interruptible failed\n");
		return HI_FAILURE;
	}

	if (!g_vencOpenFlag) {
		goto exit;
	}

	Ret = VENC_DRV_EflSuspendVedu();
	if (Ret != HI_SUCCESS) {
		HI_FATAL_VENC("venc firmware layer suspend failed, ret value is %d\n", Ret);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return HI_FAILURE;
	}

	VENC_DRV_BoardDeinit();
	g_hw_done_event.flag = 0;
exit:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	HI_INFO_VENC("-");
	return HI_SUCCESS;
}/*lint !e715*/

HI_S32 VENC_DRV_Resume(struct platform_device *pltdev)
{
	HI_S32 Ret = 0;
	HI_INFO_VENC("+");

	Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret) {
		HI_FATAL_VENC("Resume down interruptible failed\n");
		return HI_FAILURE;
	}

	if (!g_vencOpenFlag) {
		goto exit;
	}
	Ret = VENC_DRV_BoardInit();
	if (Ret != HI_SUCCESS) {
		HI_FATAL_VENC("board init failed, ret value is %d\n", Ret);
		atomic_dec(&g_VencCount);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return HI_FAILURE;
	}
	Ret = VENC_DRV_EflResumeVedu();
	if (Ret != HI_SUCCESS) {
		HI_FATAL_VENC("venc firmware layer resume failed, ret value is %d\n", Ret);
		atomic_dec(&g_VencCount);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return HI_FAILURE;
	}
exit:
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	HI_INFO_VENC("-");
	return HI_SUCCESS;
}/*lint !e715*/

static HI_S32 VENC_MemMapInfo(HI_S32 ShareFd, MEM_BUFFER_S *pMemBuffer)
{
	HI_S32 Ret;

	if (ShareFd < 0) {
		HI_FATAL_VENC("invalid param, share fd is NULL\n");
		return HI_FAILURE;
	}

	Ret = DRV_MEM_MapKernel(ShareFd, pMemBuffer);
	if (Ret) {
		HI_FATAL_VENC("share fd map failed\n");
		return HI_FAILURE;
	}

	DRV_MEM_UnmapKernel(pMemBuffer);  // TODO: judge return value?

	return HI_SUCCESS;
}

static HI_S32 VENC_GetMemoryInfo(VENC_REG_INFO_S *pRegInfo, VENC_MEM_INFO *pMemMap)
{
	HI_S32 Ret;
	HI_S32 i;

	if (!pMemMap) {
		HI_FATAL_VENC("pMemMap parameter error\n");
		return HI_FAILURE;
	}

	Ret = VENC_MemMapInfo(pRegInfo->mem_info.InteralShareFd, &pMemMap->internalbuffer);
	if (Ret) {
		HI_FATAL_VENC("Interal buffer map failed\n");
		return HI_FAILURE;
	}

	Ret = VENC_MemMapInfo(pRegInfo->mem_info.ImageShareFd, &pMemMap->imagebuffer);
	if (Ret) {
		HI_FATAL_VENC("Image buffer map failed\n");
		return HI_FAILURE;
	}

	Ret = VENC_MemMapInfo(pRegInfo->mem_info.StreamHeadShareFd, &pMemMap->streamheadbuffer);
	if (Ret) {
		HI_FATAL_VENC("streamhead buffer map failed\n");
		return HI_FAILURE;
	}

	for (i = 0; i < MAX_STREAMBUF_NUM; i++) {
		if (pRegInfo->mem_info.StreamShareFd[i] == 0) {
			continue;
		}
		Ret = VENC_MemMapInfo(pRegInfo->mem_info.StreamShareFd[i], &pMemMap->streambuffer[i]);
		if (Ret) {
			HI_FATAL_VENC("Stream buffer map failed\n");
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

static HI_S32 VENCDRV_CheckInternalAddr(S_HEVC_AVC_REGS_TYPE_CFG *pAllReg,MEM_BUFFER_S *pInternalbuffer)
{
	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REC_YADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REC_CADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REC_YH_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REC_CH_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFY_L0_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFC_L0_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFYH_L0_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFCH_L0_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_PMELD_L0_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFY_L1_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFC_L1_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFYH_L1_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_REFCH_L1_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_PMELD_L1_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_PMEST_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_NBI_MVST_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_NBI_MVLD_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_PMEINFO_ST_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_PMEINFO_LD0_ADDR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_PMEINFO_LD1_ADDR,pInternalbuffer->u32Size);

#ifndef HIVCODEC_V210
	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_QPGLD_INF_ADDR,pInternalbuffer->u32Size);
#endif

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_NBI_MVST_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_NBI_MVST_ADDR_END ,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_NBI_MVLD_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_NBI_MVLD_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEST_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEST_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMELD_ADDR_STR ,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMELD_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEINFOST_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEINFOST_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEINFOLD0_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEINFOLD0_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEINFOLD1_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_PMEINFOLD1_ADDR_END,pInternalbuffer->u32Size);
#ifndef HIVCODEC_V210
	/* QPMAP register, not used in v210 */
	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_QPGLD_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_QPGLD_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_YH_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_YH_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_CH_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_CH_ADDR_END,pInternalbuffer->u32Size);
#endif
	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_YADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_YADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_CADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REC_CADDR_END,pInternalbuffer->u32Size);
#ifndef HIVCODEC_V210
	/* QPMAP register, not used in v210 */
	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_YH_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_YH_ADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_CH_ADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_CH_ADDR_END,pInternalbuffer->u32Size);
#endif
	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_YADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_YADDR_END,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_CADDR_STR,pInternalbuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->MMU_PRE_REF_CADDR_END,pInternalbuffer->u32Size);
#ifdef HIVCODEC_V210
	/* secure in v200, not check */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[12].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[12].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[13].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[13].bits.va_end,pInternalbuffer->u32Size);
	/* image addr when not in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[15].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[15].bits.va_end,pInternalbuffer->u32Size);
#endif
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[16].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[16].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[17].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[17].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[18].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[18].bits.va_end,pInternalbuffer->u32Size);
#ifndef HIVCODEC_V210
	/* image addr in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[19].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[19].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[20].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[20].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[21].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[21].bits.va_end,pInternalbuffer->u32Size);
	/* secure in v210, not check */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[22].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[22].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[23].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[23].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[24].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[24].bits.va_end,pInternalbuffer->u32Size);
#endif
#ifdef HIVCODEC_V210
	/* internal addr in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[29].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[29].bits.va_end,pInternalbuffer->u32Size);
	/* secure when not in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[30].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[30].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[31].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[31].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[32].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[32].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[33].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[33].bits.va_end,pInternalbuffer->u32Size);
#endif
#ifndef HIVCODEC_V210
	/* value not setted in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[39].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[39].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[40].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[40].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[41].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[41].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[42].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[42].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[43].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[43].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[44].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[44].bits.va_end,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[45].bits.va_str,pInternalbuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pInternalbuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[45].bits.va_end,pInternalbuffer->u32Size);
#endif
	return HI_SUCCESS;
}

static HI_S32 VENCDRV_CheckImageAddr(S_HEVC_AVC_REGS_TYPE_CFG *pAllReg,MEM_BUFFER_S *pImageBuffer)
{
	D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_TUNLCELL_ADDR,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_SRC_YADDR,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_SRC_CADDR,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_YADDR_STR,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_YADDR_END,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_CADDR_STR,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_CADDR_END,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_LOWDLY_ADDR_STR,pImageBuffer->u32Size);

	D_VENC_CHECK_CFG_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_LOWDLY_ADDR_END,pImageBuffer->u32Size);
#ifdef HIVCODEC_V210
	/* secure in v200 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[11].bits.va_str,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[11].bits.va_end,pImageBuffer->u32Size);
#endif
#ifndef HIVCODEC_V210
	/* internal addr in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[15].bits.va_str,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[15].bits.va_end,pImageBuffer->u32Size);
	/* secure in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[27].bits.va_str,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[27].bits.va_end,pImageBuffer->u32Size);
	/* value not setted in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[28].bits.va_str,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[28].bits.va_end,pImageBuffer->u32Size);
#endif
#ifdef HIVCODEC_V210
	/* internal addr in v200, image addr in v210 */
	D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[19].bits.va_str,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[19].bits.va_end,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[20].bits.va_str,pImageBuffer->u32Size);

	D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[20].bits.va_end,pImageBuffer->u32Size);
#endif
	if (pAllReg->VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == YUV420_PLANAR || pAllReg->VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == YUV422_PLANAR) {
		D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_SRC_VADDR,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_VADDR_STR,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_VADDR_END ,pImageBuffer->u32Size);
#ifndef HIVCODEC_V210
		/* internal addr in v210 */
		D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[29].bits.va_str,pImageBuffer->u32Size);

		D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[29].bits.va_end,pImageBuffer->u32Size);
#endif
#ifdef HIVCODEC_V210
		/* VEDU_SRC_VADDR, need in this condition */
		D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[21].bits.va_str,pImageBuffer->u32Size);

		D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[21].bits.va_end,pImageBuffer->u32Size);
#endif
	}

	if (pAllReg->VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == YUV420_SEMIPLANAR_CMP) {
		D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_YH_ADDR,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->VEDU_VCPI_CH_ADDR,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_YHADDR_STR,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_YHADDR_END,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_CHADDR_STR,pImageBuffer->u32Size);

		D_VENC_CHECK_CFG_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->MMU_PRE_SRC_CHADDR_END,pImageBuffer->u32Size);

		D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[25].bits.va_str,pImageBuffer->u32Size);

		D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[25].bits.va_end,pImageBuffer->u32Size);

		D_VENC_CHECK_SMRX_REG_ADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_1[26].bits.va_str,pImageBuffer->u32Size);

		D_VENC_CHECK_SMRX_REG_ENDADDR_RET(pImageBuffer->u64StartPhyAddr,pAllReg->SMMU_MSTR_SMRX_2[26].bits.va_end,pImageBuffer->u32Size);
	}

	return HI_SUCCESS;
}

static HI_S32 VENCDRV_CheckStreamAddr(S_HEVC_AVC_REGS_TYPE_CFG *pAllReg,VENC_MEM_INFO* pMemMap)
{
	if (pAllReg->VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == YUV420_SEMIPLANAR_CMP) {
		D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streamheadbuffer.u64StartPhyAddr,
			pAllReg->MMU_PRE_PPS_ADDR_STR,pMemMap->streamheadbuffer.u32Size,pMemMap->streamheadbuffer.u32ShareFd);
		D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streamheadbuffer.u64StartPhyAddr,
			pAllReg->MMU_PRE_PPS_ADDR_END,pMemMap->streamheadbuffer.u32Size,pMemMap->streamheadbuffer.u32ShareFd);
	}

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[0].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR0_STR,pMemMap->streambuffer[0].u32Size,pMemMap->streambuffer[0].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[0].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR0_END,pMemMap->streambuffer[0].u32Size,pMemMap->streambuffer[0].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[1].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR1_STR,pMemMap->streambuffer[1].u32Size,pMemMap->streambuffer[1].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[1].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR1_END,pMemMap->streambuffer[1].u32Size,pMemMap->streambuffer[1].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[2].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR2_STR,pMemMap->streambuffer[2].u32Size,pMemMap->streambuffer[2].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[2].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR2_END,pMemMap->streambuffer[2].u32Size,pMemMap->streambuffer[2].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[3].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR3_STR,pMemMap->streambuffer[3].u32Size,pMemMap->streambuffer[3].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[3].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR3_END,pMemMap->streambuffer[3].u32Size,pMemMap->streambuffer[3].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[4].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR4_STR,pMemMap->streambuffer[4].u32Size,pMemMap->streambuffer[4].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[4].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR4_END,pMemMap->streambuffer[4].u32Size,pMemMap->streambuffer[4].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[5].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR5_STR,pMemMap->streambuffer[5].u32Size,pMemMap->streambuffer[5].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[5].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR5_END,pMemMap->streambuffer[5].u32Size,pMemMap->streambuffer[5].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[6].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR6_STR,pMemMap->streambuffer[6].u32Size,pMemMap->streambuffer[6].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[6].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR6_END,pMemMap->streambuffer[6].u32Size,pMemMap->streambuffer[6].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[7].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR7_STR,pMemMap->streambuffer[7].u32Size,pMemMap->streambuffer[7].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[7].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR7_END,pMemMap->streambuffer[7].u32Size,pMemMap->streambuffer[7].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[8].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR8_STR,pMemMap->streambuffer[8].u32Size,pMemMap->streambuffer[8].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[8].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR8_END,pMemMap->streambuffer[8].u32Size,pMemMap->streambuffer[8].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[9].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR9_STR,pMemMap->streambuffer[9].u32Size,pMemMap->streambuffer[9].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[9].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR9_END,pMemMap->streambuffer[9].u32Size,pMemMap->streambuffer[9].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[10].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR10_STR,pMemMap->streambuffer[10].u32Size,pMemMap->streambuffer[10].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[10].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR10_END,pMemMap->streambuffer[10].u32Size,pMemMap->streambuffer[10].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[11].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR11_STR,pMemMap->streambuffer[11].u32Size,pMemMap->streambuffer[11].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[11].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR11_END,pMemMap->streambuffer[11].u32Size,pMemMap->streambuffer[11].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[12].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR12_STR,pMemMap->streambuffer[12].u32Size,pMemMap->streambuffer[12].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[12].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR12_END,pMemMap->streambuffer[12].u32Size,pMemMap->streambuffer[12].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[13].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR13_STR,pMemMap->streambuffer[13].u32Size,pMemMap->streambuffer[13].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[13].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR13_END,pMemMap->streambuffer[13].u32Size,pMemMap->streambuffer[13].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[14].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR14_STR,pMemMap->streambuffer[14].u32Size,pMemMap->streambuffer[14].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[14].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR14_END,pMemMap->streambuffer[14].u32Size,pMemMap->streambuffer[14].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[15].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR15_STR,pMemMap->streambuffer[15].u32Size,pMemMap->streambuffer[15].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMENDADDR_RET(pMemMap->streambuffer[15].u64StartPhyAddr,
		pAllReg->MMU_PRE_STRMADDR15_END,pMemMap->streambuffer[15].u32Size,pMemMap->streambuffer[15].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[0].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR0,pMemMap->streambuffer[0].u32Size,pMemMap->streambuffer[0].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[1].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR1,pMemMap->streambuffer[1].u32Size,pMemMap->streambuffer[1].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[2].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR2,pMemMap->streambuffer[2].u32Size,pMemMap->streambuffer[2].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[3].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR3,pMemMap->streambuffer[3].u32Size,pMemMap->streambuffer[3].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[4].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR4,pMemMap->streambuffer[4].u32Size,pMemMap->streambuffer[4].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[5].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR5,pMemMap->streambuffer[5].u32Size,pMemMap->streambuffer[5].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[6].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR6,pMemMap->streambuffer[6].u32Size,pMemMap->streambuffer[6].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[7].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR7,pMemMap->streambuffer[7].u32Size,pMemMap->streambuffer[7].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[8].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR8,pMemMap->streambuffer[8].u32Size,pMemMap->streambuffer[8].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[9].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR9,pMemMap->streambuffer[9].u32Size,pMemMap->streambuffer[9].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[10].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR10,pMemMap->streambuffer[10].u32Size,pMemMap->streambuffer[10].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[11].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR11,pMemMap->streambuffer[11].u32Size,pMemMap->streambuffer[11].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[12].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR12,pMemMap->streambuffer[12].u32Size,pMemMap->streambuffer[12].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[13].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR13,pMemMap->streambuffer[13].u32Size,pMemMap->streambuffer[13].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[14].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR14,pMemMap->streambuffer[14].u32Size,pMemMap->streambuffer[14].u32ShareFd);

	D_VENC_CHECK_CFG_REG_STREAMADDR_RET(pMemMap->streambuffer[15].u64StartPhyAddr,
		pAllReg->VEDU_VLCST_STRMADDR15,pMemMap->streambuffer[15].u32Size,pMemMap->streambuffer[15].u32ShareFd);

	return HI_SUCCESS;
}

static HI_S32 VENCDRV_CheckCfgAddress(S_HEVC_AVC_REGS_TYPE_CFG *pAllReg, VENC_MEM_INFO *pMemMap)
{
	HI_S32 Ret;

	Ret = VENCDRV_CheckInternalAddr(pAllReg, &pMemMap->internalbuffer);
	if (Ret) {
		HI_FATAL_VENC("check InternalAddr failed\n");
		return HI_FAILURE;
	}

	Ret = VENCDRV_CheckImageAddr(pAllReg, &pMemMap->imagebuffer);
	if (Ret) {
		HI_FATAL_VENC("check ImageAddr failed\n");
		return HI_FAILURE;
	}

	Ret = VENCDRV_CheckStreamAddr(pAllReg, pMemMap);
	if (Ret) {
		HI_FATAL_VENC("check StreamAddr failed\n");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

static long VENC_Ioctl(struct file *file, unsigned int ucmd, unsigned long uarg)
{
	HI_S32  Ret;
	HI_S32  cmd  = (HI_S32)ucmd;
	HI_VOID *arg = (HI_VOID *)uarg;
	VENC_REG_INFO_S *regcfginfo =  NULL;
	VENC_MEM_INFO VencMapInfo;

	if (!arg) {
		//HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		HI_FATAL_VENC("uarg is NULL\n");
		return HI_FAILURE;
	}

	switch (cmd) {
	case CMD_VENC_START_ENCODE:/*lint !e30 !e142*/
		VeduIpCtx.TaskRunning = 1;
		regcfginfo = (VENC_REG_INFO_S *)arg;
		HiMemSet((HI_VOID*)&VencMapInfo, 0, sizeof(VencMapInfo));

		Ret = VENC_GetMemoryInfo(regcfginfo, &VencMapInfo);
		if (Ret) {
			HI_FATAL_VENC("mem shared error\n");
			return HI_FAILURE;
		}
		Ret = VENCDRV_CheckCfgAddress(&regcfginfo->all_reg, &VencMapInfo);
		if (Ret) {
			HI_FATAL_VENC("check mem error\n");
			return HI_FAILURE;
		}

		VENC_DRV_OsalInitEvent(&g_hw_done_event, 0);

		Ret = venc_drv_register_info(regcfginfo);
		VeduIpCtx.TaskRunning = 0;
		HI_DBG_VENC("venc cfg reg info, Ret:%d\n", Ret);
		break;
	default:
		HI_ERR_VENC("venc cmd unknown:0x%x\n", ucmd);
		Ret = HI_FAILURE;
		break;
	}
	//HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	return Ret;
}
static long VENC_DRV_Ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long Ret;

	Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret != 0)
	{
		HI_FATAL_VENC("Ioctl, down interruptible failed\n");
		return Ret;
	}
	Ret = (long)HI_DRV_UserCopy(file, cmd, arg, VENC_Ioctl);
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	return Ret;
}

static struct file_operations VENC_FOPS =
{
	.owner          = THIS_MODULE,/*lint !e64 */
	.open           = VENC_DRV_Open,
	.unlocked_ioctl = VENC_DRV_Ioctl,
	.compat_ioctl   = VENC_DRV_Ioctl,  //用户态 ，内核态平台位宽不一致的时候，会调用
	.release        = VENC_DRV_Close,
	//.mmap         = VENC_DRV_MMap,   // reserve
};/*lint !e785 */

static const struct of_device_id venc_of_match[] = {
       { .compatible = VENC_COMPATIBLE, },/*lint !e785 */
       { }/*lint !e785 */
};


static struct platform_driver Venc_driver = {
	.probe   = VENC_DRV_Probe,
	.remove  = VENC_DRV_Remove,
	.suspend = VENC_DRV_Suspend,
	.resume  = VENC_DRV_Resume,
	.driver  = {
	.name    = "hi_venc",
	.owner   = THIS_MODULE,/*lint !e64 */
	.of_match_table = venc_of_match
	},/*lint !e785 */
};/*lint !e785 */

extern HI_U64 gSmmuPageBaseAddr;
static ssize_t VENC_DRV_show(struct device *dev, struct device_attribute *attr, char *buf)
{
#ifdef USER_DISABLE_VENC_PROC
	if (buf == HI_NULL) {
		HI_ERR_VENC("%s buf is null\n", __func__);
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "0x%pK\n", (void *)(uintptr_t)gSmmuPageBaseAddr); /* unsafe_function_ignore: snprintf */
#else
	return 0;
#endif
}

static DEVICE_ATTR(omxvenc_misc, 0444, VENC_DRV_show, NULL);

static HI_S32 VENC_DRV_SetupCdev(VENC_ENTRY *venc, const struct file_operations *fops)
{
	HI_S32 err = 0;

	HI_INFO_VENC("enter %s()\n", __func__);
	err = alloc_chrdev_region(&venc->dev, 0, 1, "hi_venc");
	if (err < 0) {
		return HI_FAILURE;
	}

	HiMemSet((HI_VOID*)&(venc->cdev), 0, sizeof(struct cdev));
	cdev_init(&(venc->cdev), &VENC_FOPS);

	venc->cdev.owner = THIS_MODULE;/*lint !e64 */
	venc->cdev.ops = &VENC_FOPS;
	err = cdev_add(&(venc->cdev), venc->dev, 1);

	/*在/sys/class/目录下创建设备类别目录hi_venc*/
	venc->venc_class = class_create(THIS_MODULE, "hi_venc");/*lint !e64 */
	if (IS_ERR(venc->venc_class)) {
		err = PTR_ERR(venc->venc_class);/*lint !e712 */
		HI_ERR_VENC("Fail to create hi_venc class\n");
		goto unregister_region;
		//return HI_FAILURE;/*lint !e438 */
	}

	/*在/dev/目录和/sys/class/hi_venc目录下分别创建设备文件hi_venc*/
	venc->venc_device = device_create(venc->venc_class, NULL, venc->dev, "%s", "hi_venc");
	if (IS_ERR(venc->venc_device)) {
		err = PTR_ERR(venc->venc_device);/*lint !e712 */
		HI_ERR_VENC("Fail to create hi_venc device\n");
		goto cls_destroy;
		//return HI_FAILURE;/*lint !e438 */
	}

	err = device_create_file(venc->venc_device, &dev_attr_omxvenc_misc);
	if (err) {
		HI_ERR_VENC("%s, failed to create device file\n", __func__);
		goto dev_destroy;
	}

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;

dev_destroy:
        device_destroy(venc->venc_class, venc->dev);
cls_destroy:
	class_destroy(venc->venc_class);
	venc->venc_class = HI_NULL;
unregister_region:
	unregister_chrdev_region(venc->dev, 1);
	return HI_FALSE;
}/*lint !e550 */

static HI_S32 VENC_DRV_CleanupCdev(VENC_ENTRY *venc)
{
	/*销毁设备类别和设备*/
	if (venc->venc_class) {
		device_remove_file(venc->venc_device, &dev_attr_omxvenc_misc);
		device_destroy(venc->venc_class,venc->dev);
		class_destroy(venc->venc_class);
	}

	cdev_del(&(venc->cdev));
	unregister_chrdev_region(venc->dev,1);

	return 0;
}

static HI_U32 VENC_DRV_Device_Idle(VENC_PLATFORM_E plt_frm)
{
	HI_S32 idle   = 0;
	HI_U32 *pctrl = HI_NULL;
	switch (plt_frm)
	{
#ifdef VENC_ES_SUPPORT
		case FPGA_ES :
		{
			pctrl  = (HI_U32 *)ioremap(VENC_PCTRL_PERI_ES, (unsigned long)4);/*lint !e747 */
			if(pctrl)
				idle = readl(pctrl) & VENC_EXISTBIT_ES;/*lint !e50 !e64 */
			break;
		}
#endif

#ifdef VENC_CS_SUPPORT
		case FPGA_CS :
		{
			pctrl  = (HI_U32 *)ioremap(VENC_PCTRL_PERI_CS, (unsigned long)4);/*lint !e747 */
			if(pctrl)
				idle = readl(pctrl) & VENC_EXISTBIT_CS;/*lint !e50 !e64 */ //b[18]
			break;
		}
#endif

		default :
		{
			printk(KERN_ERR "unkown platform %d\n", plt_frm);
			break;
		}
	};

	if (!pctrl) {
		printk(KERN_ERR "ioremap failed\n");
		return HI_FALSE;
	} else {
		iounmap(pctrl);
	}

	return (HI_U32)((idle != 0) ? HI_TRUE : HI_FALSE);
}

static HI_S32 VENC_DRV_Probe(struct platform_device * pltdev)
{
	HI_S32 ret = HI_FAILURE;
	VENC_ENTRY *venc = HI_NULL;
	HI_U32 fpga_cs_flag = 0;
	HI_U32 fpga_es_flag = 0;
	struct device *dev = &pltdev->dev;
	VENC_PLATFORM_E plat_form = UDP_DEFAULT;

	HI_INFO_VENC("omxvenc prepare to probe\n");
	HiVENC_INIT_MUTEX(&g_VencMutex);

#ifdef VENC_ES_SUPPORT
	ret = of_property_read_u32(dev->of_node, VENC_FPGAFLAG_ES, &fpga_es_flag);
	if (ret)
		HI_INFO_VENC("read failed, but fpga_es has defualt value\n");
	if (1 == fpga_es_flag)
		plat_form = FPGA_ES;
#endif

#ifdef VENC_CS_SUPPORT
	ret = of_property_read_u32(dev->of_node, VENC_FPGAFLAG_CS, &fpga_cs_flag);
	if (ret)
		HI_INFO_VENC("read failed, but fpga_cs has defualt value\n");
	if (1 == fpga_cs_flag)
		plat_form = FPGA_CS;
#endif

	if ((1 == fpga_cs_flag) || (1 == fpga_es_flag)) {
		printk(KERN_INFO "fpga platform\n");
		if (VENC_DRV_Device_Idle(plat_form) == HI_FALSE) {
			printk(KERN_ERR "venc is not exist\n");
			return HI_FAILURE;
		}
	} else {
		printk(KERN_INFO "not fpga platform\n");
	}

	if (g_vencDevDetected) {
		HI_INFO_VENC("venc device detected already\n");
		return HI_SUCCESS;
	}

	venc = HiMemVAlloc(sizeof(VENC_ENTRY));/*lint !e747 */
	if (!venc) {
		HI_FATAL_VENC("call vmalloc failed\n");
		return ret;
	}

	HiMemSet((HI_VOID *)venc, 0, sizeof(VENC_ENTRY));
	ret = VENC_DRV_SetupCdev(venc, &VENC_FOPS);
	if (ret < 0) {
		HI_ERR_VENC("setup char device failed\n");
		goto free;
	}

	//venc->venc_device_2 = &pltdev->dev;
	platform_set_drvdata(pltdev, venc);
	g_vencDevDetected = HI_TRUE;

	ret = Venc_Regulator_Init(pltdev);
	if (ret < 0) {
		HI_FATAL_VENC("init regulator failed\n");
		goto cleanup;
	}

	HI_INFO_VENC("omxvenc probe successfully\n");
	return ret;

cleanup:
	VENC_DRV_CleanupCdev(venc);
free:
	HiMemVFree(venc);
	return ret;
}

static HI_S32 VENC_DRV_Remove(struct platform_device *pltdev)
{
	VENC_ENTRY *venc = NULL;
	HI_INFO_VENC("omxvenc prepare to remove\n");

	venc = platform_get_drvdata(pltdev);
	if (venc) {
		VENC_DRV_CleanupCdev(venc);
		Venc_Regulator_Deinit(pltdev);
	}
	else {
		HI_ERR_VENC("get platform drvdata err\n");
	}

	platform_set_drvdata(pltdev,NULL);
	HiMemVFree(venc);
	g_vencDevDetected = HI_FALSE;

	HI_INFO_VENC("remove omxvenc successfully\n");
	return 0;
}

HI_S32 __init VENC_DRV_ModInit(HI_VOID)
{
	HI_S32 ret = 0;

	HI_INFO_VENC("enter %s()\n", __func__);

	ret = platform_driver_register(&Venc_driver);/*lint !e64 */
	if (ret) {
		HI_ERR_VENC("register platform driver failed\n");
		return ret;
	}
	HI_INFO_VENC("success\n");

#ifdef MODULE
	HI_INFO_VENC("Load hi_venc.ko success\t(%s)\n", VERSION_STRING);
#endif

	HI_INFO_VENC("exit %s()\n", __func__);

	return ret;
}

HI_VOID VENC_DRV_ModExit(HI_VOID)
{
	HI_INFO_VENC("enter %s()\n", __func__);

	platform_driver_unregister(&Venc_driver);

	HI_INFO_VENC("exit %s()\n", __func__);
	return;
}
/*lint -e528*/
module_init(VENC_DRV_ModInit); /*lint !e528*/
module_exit(VENC_DRV_ModExit); /*lint !e528*/
/*lint -e753*/
MODULE_LICENSE("Dual BSD/GPL"); /*lint !e753*/

