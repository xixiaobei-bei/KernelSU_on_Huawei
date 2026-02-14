#ifndef __VFMW_INTF_H__
#define __VFMW_INTF_H__
#include "memory.h"
#include "vdm_drv.h"
#include "scd_drv.h"
#include "platform.h"

#define VCTRL_OK                0
#define VCTRL_ERR              -1
#define MSG_POOL_ADDR_CHECK

typedef struct hiDRV_MEM_S {
	MEM_RECORD_S stVdhReg;
} DRV_MEM_S;

typedef struct {
	MEM_BUFFER_S scd[SCD_SHAREFD_MAX];
	MEM_BUFFER_S vdh[VDH_SHAREFD_MAX];
	CLK_RATE_E   clk_rate;
	struct file  *file ;
}VDEC_MEM_INFO;

#ifdef MSG_POOL_ADDR_CHECK
SINT32 CheckFrmBufAddr(UINT32 SrcPmvAddr,MEM_BUFFER_S* pVdhMemMap);
SINT32 CheckPmvBufAddr(UINT32 SrcPmvAddr,MEM_BUFFER_S* pVdhMemMap);
SINT32 VCTRL_VDHUnmapMessagePool(MEM_BUFFER_S *pMemMap);
#endif

SINT32 VCTRL_OpenDrivers(VOID);

SINT32 VCTRL_OpenVfmw(VOID);

SINT32 VCTRL_CloseVfmw(VOID);

SINT32 VCTRL_VDMHal_Process(OMXVDH_REG_CFG_S *pVdmRegCfg, VDMHAL_BACKUP_S *pVdmRegStatei, MEM_BUFFER_S* pVdhMemMap, MEM_BUFFER_S *pComMsgMap);

SINT32 VCTRL_SCDHal_Process(OMXSCD_REG_CFG_S *pScdRegCfg, SCD_STATE_REG_S *pScdStateReg, MEM_BUFFER_S* pScdMemMap);

SINT32 VCTRL_VDMHAL_IsRun(VOID);

VOID VCTRL_Suspend(VOID);

VOID VCTRL_Resume(VOID);

HI_BOOL VCTRL_Scen_Ident(HI_U32 cmd);

#endif
