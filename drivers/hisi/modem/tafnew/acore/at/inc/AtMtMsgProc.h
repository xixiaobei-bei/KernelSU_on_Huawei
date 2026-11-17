/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef _AT_MT_MSG_PROC_H_
#define _AT_MT_MSG_PROC_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "AtMtInterface.h"


#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif


#pragma pack(4)


/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  7 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 全局变量声明
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
VOS_VOID  At_WTxCltIndProc(
    WPHY_AT_TX_CLT_IND_STRU            *pstMsg
);

VOS_UINT32 AT_RcvMtaRficSsiRdQryCnf(VOS_VOID *pMsg);

VOS_UINT32  At_MipiRdCnfProc( HPA_AT_MIPI_RD_CNF_STRU    *pstMsg );
VOS_UINT32  At_MipiWrCnfProc( HPA_AT_MIPI_WR_CNF_STRU    *pstMsg );
VOS_UINT32  At_SsiWrCnfProc( HPA_AT_SSI_WR_CNF_STRU  *pstMsg );
VOS_UINT32  At_SsiRdCnfProc(HPA_AT_SSI_RD_CNF_STRU   *pstMsg );

extern VOS_UINT32  At_PdmCtrlCnfProc( HPA_AT_PDM_CTRL_CNF_STRU  *pstMsg );

VOS_UINT32  At_SendContinuesWaveOnToHPA(
    VOS_UINT16                          usPower,
    VOS_UINT8                           ucIndex
);
VOS_VOID  At_RfCfgCnfReturnSuccProc(
    VOS_UINT8                           ucIndex
);

VOS_UINT32 AT_SetFchanRspErr(DRV_AGENT_FCHAN_SET_ERROR_ENUM_UINT32  enResult);

VOS_UINT32  At_RfCfgCnfReturnErrProc(
    VOS_UINT8                           ucIndex
);

VOS_UINT32 AT_RcvDrvAgentSetFchanRsp(VOS_VOID *pMsg);


#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
VOS_UINT32  At_SendContinuesWaveOnToCHPA(
    VOS_UINT8                           ucCtrlType,
    VOS_UINT16                          usPower
);
#endif

VOS_UINT32  At_SendTxOnOffToCHPA(VOS_UINT8 ucSwitch);

VOS_UINT32 At_SendRxOnOffToCHPA(
    VOS_UINT32                          ulRxSwitch
);

VOS_VOID  At_HpaRfCfgCnfProc(
    HPA_AT_RF_CFG_CNF_STRU              *pstMsg
);

VOS_VOID  At_RfRssiIndProc(
    HPA_AT_RF_RX_RSSI_IND_STRU          *pstMsg
);

VOS_VOID  At_CHpaRfCfgCnfProc(
    CHPA_AT_RF_CFG_CNF_STRU             *pstMsg
);
VOS_UINT32 AT_RcvMtaPowerDetQryCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvDrvAgentTseLrfSetRsp(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvDrvAgentSetFchanRsp(VOS_VOID *pMsg);

#else
VOS_UINT32 At_SndRxOnOffReq(VOS_VOID);
VOS_UINT32 At_SndTxOnOffReq(VOS_UINT32 ulPowerDetFlg);
VOS_UINT32  At_LoadPhy(VOS_VOID);
VOS_VOID AT_ProcCbtMsg(VOS_VOID *pstMsg);
VOS_VOID AT_ProcBbicMsg(VOS_VOID *pstMsg);
VOS_UINT32 AT_ProcSetWorkModeCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_ProcTxRxCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_ProcPowerDetCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_SndBbicCalMipiReadReq(
    VOS_UINT32 ulMipiPortSel,
    VOS_UINT32 ulSlaveId,
    VOS_UINT32 ulRegAddr,
    VOS_UINT32 ulByteCnt
);
VOS_UINT32 AT_SndBbicCalMipiWriteReq(
    VOS_UINT32 ulMipiPortSel,
    VOS_UINT32 ulSlaveId,
    VOS_UINT32 ulRegAddr,
    VOS_UINT32 ulByteCnt,
    VOS_UINT32 ulValue
);
VOS_UINT32 AT_RcvBbicCalMipiRedCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_RcvBbicCalMipiWriteCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_SndBbicPllStatusReq(VOS_VOID);
VOS_UINT32 At_RcvBbicPllStatusCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_SndBbicRssiReq(VOS_VOID);
VOS_UINT32 At_RcvBbicRssiInd(VOS_VOID *pstMsg);
VOS_UINT32 AT_SndBbicCalSetDpdtReq(
    BBIC_DPDT_OPERTYPE_ENUM_UINT16      enOperType,
    VOS_UINT32                          ulValue,
    VOS_UINT32                          ulWorkType
);
VOS_UINT32 AT_RcvBbicCalSetDpdtCnf(VOS_VOID *pstMsg);
VOS_UINT32 AT_SndBbicCalQryFtemprptReq(INT16 ulChannelNum);
VOS_UINT32 AT_RcvBbicCalQryFtemprptCnf(VOS_VOID *pstMsg);
VOS_UINT32 At_SndDcxoReq(VOS_VOID);
VOS_UINT32 At_RcvBbicCalDcxoCnf(VOS_VOID *pstMsg);
#endif

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif

#endif

