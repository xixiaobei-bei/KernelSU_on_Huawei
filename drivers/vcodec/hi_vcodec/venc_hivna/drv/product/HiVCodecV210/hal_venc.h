#ifndef __HAL_VENC_H__
#define __HAL_VENC_H__

#include "hi_type.h"
#include "Vedu_RegAll.h"
#include "drv_venc_ioctl.h"

#define VENC_COMPATIBLE       "hisi,HiVCodecV210-venc"
#define VENC_CS_SUPPORT
#define VENC_FPGAFLAG_CS      "venc_fpga"
#define VENC_PCTRL_PERI_CS    0xE8A090BC
#define VENC_EXISTBIT_CS      0x20000

HI_VOID VENC_HAL_ClrAllInt(S_HEVC_AVC_REGS_TYPE * pVeduReg);
HI_VOID VENC_HAL_DisableAllInt(S_HEVC_AVC_REGS_TYPE * pVeduReg);
HI_S32 VENC_HAL_ResetReg(HI_VOID);
HI_VOID VENC_HAL_StartEncode(S_HEVC_AVC_REGS_TYPE * pVeduReg);
HI_VOID VENC_HAL_Get_CfgRegSimple(VENC_REG_INFO_S * pVeduReg);
HI_VOID VENC_HAL_Get_Reg_Venc(VENC_REG_INFO_S * pVeduReg);
HI_VOID VeduHal_CfgReg_IntraSet(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgReg_LambdaSet(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgReg_QpgmapSet(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgReg_AddrSet(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgReg_SlcHeadSet(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgReg_SMMUSet(HI_VOID);
HI_VOID VeduHal_CfgReg_PREMMUSet(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgRegSimple(VENC_REG_INFO_S * channelcfg);
HI_VOID VeduHal_CfgReg(VENC_REG_INFO_S * regcfginfo);
HI_VOID VENC_HAL_SetSmmuAddr(S_HEVC_AVC_REGS_TYPE * pVeduReg);


#endif				//__HAL_VENC_H__

