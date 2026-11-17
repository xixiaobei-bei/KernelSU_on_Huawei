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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#ifndef RF0_TYPE_ASIC_HI6365
#define RF0_TYPE_ASIC_HI6365
#endif 
#include "AtMtInterface.h"
#include "AtTypeDef.h"
#include "MnClient.h"
#include "ATCmdProc.h" 
#include "AtMtMsgProc.h"
#include "AtMtCommFun.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_MT_MSG_PROC_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if(FEATURE_ON == FEATURE_UE_MODE_NR)
const AT_PROC_BBIC_MSG_STRU g_astAtProcBbicMsgTab[]=
{
    {ID_BBIC_AT_CAL_MSG_CNF               , AT_ProcTxRxCnf},
    {ID_BBIC_AT_CAL_RF_DEBUG_TX_RESULT_IND, AT_ProcPowerDetCnf},
    {ID_BBIC_AT_MIPI_READ_CNF             , AT_RcvBbicCalMipiRedCnf},
    {ID_BBIC_AT_MIPI_WRITE_CNF            , AT_RcvBbicCalMipiWriteCnf},
    {ID_BBIC_AT_PLL_QRY_CNF               , At_RcvBbicPllStatusCnf},
    {ID_BBIC_AT_CAL_RF_DEBUG_RX_RSSI_IND  , At_RcvBbicRssiInd},
    {ID_BBIC_AT_DPDT_CNF                  , AT_RcvBbicCalSetDpdtCnf},
    {ID_BBIC_AT_TEMP_QRY_CNF              , AT_RcvBbicCalQryFtemprptCnf},
    {ID_BBIC_AT_DCXO_CNF                  , At_RcvBbicCalDcxoCnf},
};


const AT_PROC_CBT_MSG_STRU g_astAtProcCbtMsgTab[]=
{
    {ID_CCBT_AT_SET_WORK_MODE_CNF, AT_ProcSetWorkModeCnf},
};
#endif

#if(FEATURE_ON == FEATURE_UE_MODE_NR)
extern AT_MT_INFO_STRU                         g_stMtInfoCtx;
#else
extern AT_DEVICE_CMD_CTRL_STRU                 g_stAtDevCmdCtrl;
#endif

#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
/*****************************************************************************
  3 旧函数实现
*****************************************************************************/


VOS_UINT32  At_SendContinuesWaveOnToHPA(
    VOS_UINT16                          usPower,
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU              *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;
    VOS_UINT8                           ucCtrlType;

    ucCtrlType = WDSP_CTRL_TX_ONE_TONE;

    /* 打开调制 */
    if (AT_DSP_CLT_ENABLE == g_stAtDevCmdCtrl.enCltEnableFlg)
    {
        ucCtrlType = WDSP_CTRL_TX_THREE_TONE;
    }

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));

    /* 填写消息头 */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength        = ulLength;

    /* 填写消息体 */
    pstMsg->usMsgID = ID_AT_HPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask =  W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_POWER;
    pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)usPower;

    /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usMask      = usMask;
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;

    /* 打开单音信号 */
    pstMsg->stRfCfgPara.usTxEnable = ucCtrlType;

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_UINT32  At_SendContinuesWaveOnToCHPA(
    VOS_UINT8                           ucCtrlType,
    VOS_UINT16                          usPower
)
{
    AT_CHPA_RF_CFG_REQ_STRU            *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_CHPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg = (AT_CHPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_CHPA_RF_CFG_REQ_STRU), 0x00 ,sizeof(AT_CHPA_RF_CFG_REQ_STRU));

    /* 填写消息头 */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UPHY_PID_CSDR_1X_CM;
    pstMsg->ulLength        = ulLength;

    /* 填写消息体 */
    pstMsg->usMsgID = ID_AT_CHPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask =  W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_POWER;
    pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)usPower;

    /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usMask      = usMask;
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;

    /* 打开单音信号 */
    pstMsg->stRfCfgPara.usTxEnable = ucCtrlType;

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendContinuesWaveOnToCHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32  At_SendTxOnOffToCHPA(VOS_UINT8 ucSwitch)
{
    AT_CHPA_RF_CFG_REQ_STRU            *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_CHPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg = (AT_CHPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendTxOnOffToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_CHPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_CHPA_RF_CFG_REQ_STRU));

    /* 填写消息头 */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UPHY_PID_CSDR_1X_CM;
    pstMsg->ulLength        = ulLength;

    /* 填写消息体 */
    pstMsg->usMsgID = ID_AT_CHPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask =  W_RF_MASK_AFC | W_RF_MASK_TX_ARFCN \
             | W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_PAMODE |W_RF_MASK_TX_POWDET;

    usMask                       = usMask | W_RF_MASK_TX_POWER;
    pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)g_stAtDevCmdCtrl.usPower;

    /* Tx Cfg */
    pstMsg->stRfCfgPara.usMask      = usMask;                             /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;              /* AFC */
    pstMsg->stRfCfgPara.usTxBand    = 0;                             /* g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;     1,2,3...,协议中的band编号,注意不是BandId */
    pstMsg->stRfCfgPara.usTxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn; /* Arfcn */
    pstMsg->stRfCfgPara.usTxPAMode  = g_stAtDevCmdCtrl.ucPaLevel;

    if (AT_DSP_RF_SWITCH_ON == ucSwitch)
    {
        pstMsg->stRfCfgPara.usTxEnable = WDSP_CTRL_TX_ON;               /* 打开发射TX */
    }
    else
    {
        pstMsg->stRfCfgPara.usTxEnable = WDSP_CTRL_TX_OFF;               /* 关闭发射TX */
    }

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendTxOnOffToCHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32 At_SendRxOnOffToCHPA(
    VOS_UINT32                          ulRxSwitch
)
{
    AT_CHPA_RF_CFG_REQ_STRU            *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_CHPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_CHPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendRxOnOffToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_CHPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_CHPA_RF_CFG_REQ_STRU));

    /* 填写消息头 */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UPHY_PID_CSDR_1X_CM;
    pstMsg->ulLength        = ulLength;

    /* 填写消息体 */
    pstMsg->usMsgID = ID_AT_CHPA_RF_CFG_REQ;

    usMask = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL \
             | W_RF_MASK_RX_RXONOFF;

    /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usMask      = usMask;

    /* 1,2,3...,协议中的band编号 */
    pstMsg->stRfCfgPara.usRxBand    = 0;  /* g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;*/
    pstMsg->stRfCfgPara.usRxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn;

    pstMsg->stRfCfgPara.usRxAntSel = ANT_ONE;

    if (AT_DSP_RF_SWITCH_ON == ulRxSwitch)
    {      
        /* CPU控制打开射频芯片,天线1 */
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ANT1_ON;
    }
    else
    {
        /* 关闭接收RX */
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_OFF;
    }

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendRxOnOffToCHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

#endif


TAF_UINT32  At_SendTxOnOffToHPA(
    TAF_UINT8                           ucSwitch,
    TAF_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU             *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength    = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    usMask      = 0;

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendTxOnOffToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));

    /* 填写消息头 */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength        = ulLength;

    /* 填写消息体 */
    pstMsg->usMsgID = ID_AT_HPA_RF_CFG_REQ;

    /* Tx Cfg */
    usMask = W_RF_MASK_AFC | W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_PAMODE | W_RF_MASK_TX_POWDET;

    /*AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值*/
    if ( VOS_TRUE == g_stAtDevCmdCtrl.bFdacFlag )
    {
        usMask                      = usMask | W_RF_MASK_TX_AGC;
        pstMsg->stRfCfgPara.usTxAGC = g_stAtDevCmdCtrl.usFDAC;
    }
    else
    {
        usMask                       = usMask | W_RF_MASK_TX_POWER;
        pstMsg->stRfCfgPara.sTxPower = (VOS_INT16)g_stAtDevCmdCtrl.usPower;
    }

    /* Tx Cfg */
    pstMsg->stRfCfgPara.usMask      = usMask;                             /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usTxAFCInit = W_AFC_INIT_VALUE;              /* AFC */
    pstMsg->stRfCfgPara.usTxBand    = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;    /* 1,2,3...,协议中的band编号,注意不是BandId */
    pstMsg->stRfCfgPara.usTxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn; /* Arfcn */
    pstMsg->stRfCfgPara.usTxPAMode  = g_stAtDevCmdCtrl.ucPaLevel;

    if (AT_DSP_RF_SWITCH_OFF == ucSwitch)
    {
        pstMsg->stRfCfgPara.usTxEnable  = WDSP_CTRL_TX_OFF;                     /* 关闭发射TX */
    }
    else
    {
        pstMsg->stRfCfgPara.usTxEnable  = WDSP_CTRL_TX_ON;                      /* 打开发射TX */
        pstMsg->stRfCfgPara.usMask      |= W_RF_MASK_TX_ARFCN;

        /* 只有在^FTXON=1 时才下发 */
        if (AT_DSP_CLT_ENABLE == g_stAtDevCmdCtrl.enCltEnableFlg)
        {
            pstMsg->stRfCfgPara.usMask |= W_RF_MASK_TX_CLT;
            pstMsg->stRfCfgPara.usMask &= ~(W_RF_MASK_TX_POWDET);
        }
        if ( AT_DCXOTEMPCOMP_ENABLE == g_stAtDevCmdCtrl.enDcxoTempCompEnableFlg)
        {
            pstMsg->stRfCfgPara.usMask |= W_RF_MASK_TX_DCXOTEMPCOMP;
            pstMsg->stRfCfgPara.usMask &= ~(W_RF_MASK_AFC);
        }
    }

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendTxOnOffToHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}


VOS_UINT32  At_SendTxOnOffToGHPA(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucSwitch
)
{
    AT_GHPA_RF_TX_CFG_REQ_STRU          *pstTxMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_GHPA_RF_TX_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_GHPA_RF_TX_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    pstTxMsg = (AT_GHPA_RF_TX_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstTxMsg)
    {
        AT_WARN_LOG("At_SendRfCfgReqToGHPA: alloc Tx msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstTxMsg, sizeof(AT_GHPA_RF_TX_CFG_REQ_STRU), 0x00, sizeof(AT_GHPA_RF_TX_CFG_REQ_STRU));

    /* 填写消息头 */
    pstTxMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstTxMsg->ulSenderPid     = WUEPS_PID_AT;
    pstTxMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstTxMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    pstTxMsg->ulLength        = ulLength;

    /* Tx Cfg */

    /* 填写消息体 */
    pstTxMsg->usMsgID = ID_AT_GHPA_RF_TX_CFG_REQ;
    usMask            = G_RF_MASK_TX_AFC;

    /*AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值*/
    /* 发射控制方式：
        0：GMSK电压控制,此方式下usTxVpa要配置；
        1：功率控制,此方式下usTxPower要配置；
        2：8PSK PaVbias电压&DBB Atten&RF Atten控制，
        usPAVbias和usRfAtten,sDbbAtten三个参数都要配置；*/
    if ( VOS_TRUE == g_stAtDevCmdCtrl.bFdacFlag )
    {
        pstTxMsg->enCtrlMode = AT_GHPA_RF_CTRLMODE_TYPE_GMSK;
        pstTxMsg->usTxVpa = g_stAtDevCmdCtrl.usFDAC;  /* DAC设置 */
    }
    else
    {
        pstTxMsg->enCtrlMode = AT_GHPA_RF_CTRLMODE_TYPE_TXPOWER;
        pstTxMsg->usTxPower = g_stAtDevCmdCtrl.usPower;
    }

    /* Tx Cfg */
    pstTxMsg->usMask = usMask;              /* 按位标识配置类型 */
    pstTxMsg->usAFC =  0;                   /* AFC */
    pstTxMsg->usFreqNum =
                (VOS_UINT16)((g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand << 12) \
                            | g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn);
                                            /* (Band << 12) | Arfcn */
    pstTxMsg->usTxMode = g_stAtDevCmdCtrl.usTxMode;    /* 0:burst发送; 1:连续发送 */
    pstTxMsg->usModType = G_MOD_TYPE_GMSK;  /* 发射调制方式:0表示GMSK调制;1表示8PSK调制方式 */

    if (AT_DSP_RF_SWITCH_ON == ucSwitch)
    {
        pstTxMsg->usTxEnable = GDSP_CTRL_TX_ON;
    }
    else
    {
        pstTxMsg->usTxEnable = GDSP_CTRL_TX_OFF;
    }

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstTxMsg))
    {
        AT_WARN_LOG("At_SendTxOnOffToGHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32  At_SendRxOnOffToGHPA(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulRxSwitch
)
{
    AT_GHPA_RF_RX_CFG_REQ_STRU          *pstRxMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_GHPA_RF_RX_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_GHPA_RF_RX_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstRxMsg = (AT_GHPA_RF_RX_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstRxMsg)
    {
        AT_WARN_LOG("At_SendRxOnOffToGHPA: alloc Rx msg fail!");
        return AT_FAILURE;
    }

    /* Rx Cfg */
    pstRxMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    pstRxMsg->ulLength        = ulLength;
    pstRxMsg->usMsgID         = ID_AT_GHPA_RF_RX_CFG_REQ;
    pstRxMsg->usRsv           = 0;
    pstRxMsg->usRsv2          = 0;

    usMask = G_RF_MASK_RX_ARFCN | G_RF_MASK_RX_AGCMODE \
            | G_RF_MASK_RX_AGCGAIN | G_RF_MASK_RX_MODE;

    /* Reference MASK_CAL_RF_G_RX_* section */
    pstRxMsg->usMask = usMask;

    /* (Band << 12) | Arfcn */
    pstRxMsg->usFreqNum =
                    (VOS_UINT16)((g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand << 12) \
                                | g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn);

    /* 0:停止; 1:连续接收; */
    if ( TRUE == ulRxSwitch )
    {
        pstRxMsg->usRxMode  = AT_GDSP_RX_MODE_CONTINOUS_BURST;
    }
    else
    {
        pstRxMsg->usRxMode  = AT_GDSP_RX_MODE_STOP;
    }

    /* 1:Fast AGC,0:Slow AGC */
    pstRxMsg->usAGCMode = AT_GDSP_RX_SLOW_AGC_MODE;

    /* AGC档位，共四档,取值为0-3*/
    pstRxMsg->usAgcGain = g_stAtDevCmdCtrl.ucLnaLevel;

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstRxMsg))
    {
        AT_WARN_LOG("At_SendRxOnOffToGHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 At_SendRxOnOffToHPA(
    VOS_UINT32                          ulRxSwitch,
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU              *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMask;

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendRxOnOffToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));

    /* 填写消息头 */
    pstMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid     = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength        = ulLength;

    /* 填写消息体 */
    pstMsg->usMsgID = ID_AT_HPA_RF_CFG_REQ;

    usMask = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL \
             | W_RF_MASK_RX_RXONOFF;

    /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usMask      = usMask;

    /* 1,2,3...,协议中的band编号 */
    pstMsg->stRfCfgPara.usRxBand    = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->stRfCfgPara.usRxFreqNum = g_stAtDevCmdCtrl.stDspBandArfcn.usDlArfcn;

    /* DSP频段Band1-Band9格式转换为
       W Rf support
    -------------------------------------------------------------------------------
            bit8       bit7      bit6     bit5    bit4     bit3      bit2     bit1
    -------------------------------------------------------------------------------
    WCDMA   900(VIII)  2600(VII) 800(VI)  850(V)  1700(IV) 1800(III) 1900(II) 2100(I) oct1
            spare      spare     spare    spare   spare    spare     spare   J1700(IX)oct2
    */

    if (AT_RX_DIV_ON == g_stAtDevCmdCtrl.ucPriOrDiv)
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_TWO;
    }
    else
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_ONE;
    }

    /* LNA模式控制 */
    pstMsg->stRfCfgPara.usRxLNAGainMode = g_stAtDevCmdCtrl.ucLnaLevel;

    if (AT_DSP_RF_SWITCH_ON == ulRxSwitch)
    {
        if (ANT_TWO == pstMsg->stRfCfgPara.usRxAntSel)
        {
            /* CPU控制打开射频芯片,天线1和2 */
            pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ALL_ANT_ON;
        }
        else
        {
            /* CPU控制打开射频芯片,天线1 */
            pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ANT1_ON;
        }
    }
    else
    {
        /* 关闭接收RX */
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_OFF;
    }

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendRxOnOffToHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 At_SendRfCfgAntSelToHPA(
    VOS_UINT8                           ucDivOrPriOn,
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_CFG_REQ_STRU              *pstMsg;
    VOS_UINT32                           ulLength;
    VOS_UINT16                           usMask;

    /* 申请AT_HPA_RF_CFG_REQ_STRU消息 */
    ulLength = sizeof(AT_HPA_RF_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    pstMsg   = (AT_HPA_RF_CFG_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SendRfCfgAntSelToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    TAF_MEM_SET_S(pstMsg, sizeof(AT_HPA_RF_CFG_REQ_STRU), 0x00, sizeof(AT_HPA_RF_CFG_REQ_STRU));


    /* 填写消息头 */
    pstMsg->ulSenderCpuId      = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid        = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId    = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid      = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    pstMsg->ulLength           = ulLength;


    /* 填写消息体 */
    pstMsg->usMsgID            = ID_AT_HPA_RF_CFG_REQ;
    usMask                     = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL \
                                 | W_RF_MASK_RX_RXONOFF;

    /* 按位标识配置类型 */
    pstMsg->stRfCfgPara.usMask = usMask;

    if (AT_RX_DIV_ON == ucDivOrPriOn)
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_TWO;
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ALL_ANT_ON;
    }
    else
    {
        pstMsg->stRfCfgPara.usRxAntSel = ANT_ONE;
        pstMsg->stRfCfgPara.usRxEnable = DSP_CTRL_RX_ANT1_ON;
    }

    pstMsg->stRfCfgPara.usRxBand       = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->stRfCfgPara.usRxFreqNum    = g_stAtDevCmdCtrl.stDspBandArfcn.usDlArfcn;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SendRfCfgAntSelToHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}


VOS_UINT32 AT_RcvDrvAgentSetFchanRsp(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                 *pRcvMsg;
    DRV_AGENT_FCHAN_SET_CNF_STRU       *pstFchanSetCnf;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulError;

    /* 初始化 */
    pRcvMsg          = (DRV_AGENT_MSG_STRU *)pMsg;
    pstFchanSetCnf   = (DRV_AGENT_FCHAN_SET_CNF_STRU *)pRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstFchanSetCnf->stAtAppCtrl.usClientId,&ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSetFchanRsp:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSetFchanRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_FCHAN_SET */
    if (AT_CMD_FCHAN_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 设置有错误的处理 */
    ulError =  AT_SetFchanRspErr(pstFchanSetCnf->enResult);
    if(DRV_AGENT_FCHAN_SET_NO_ERROR != ulError)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, ulError);
        return VOS_OK;
    }

    /* 设置无错误的处理 */
    g_stAtDevCmdCtrl.bDspLoadFlag    = VOS_TRUE;
    g_stAtDevCmdCtrl.ucDeviceRatMode = (VOS_UINT8)pstFchanSetCnf->stFchanSetReq.ucDeviceRatMode;
    g_stAtDevCmdCtrl.ucDeviceAtBand  = (VOS_UINT8)pstFchanSetCnf->stFchanSetReq.ucDeviceAtBand;
    g_stAtDevCmdCtrl.stDspBandArfcn  = pstFchanSetCnf->stFchanSetReq.stDspBandArfcn;
    g_stAtDevCmdCtrl.usFDAC          = 0;                                       /* FDAC清零，防止G/W范围错误 */

    /* 调用AT_FormATResultDATa发送命令结果 */
    gstAtSendData.usBufLen = 0;
    At_FormatResultData(ucIndex,AT_OK);
    return VOS_OK;
}


VOS_VOID  At_HpaRfCfgCnfProc(
    HPA_AT_RF_CFG_CNF_STRU              *pstMsg
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (AT_HPA_RSLT_FAIL == pstMsg->usErrFlg)
    {
        AT_INFO_LOG("At_HpaRfCfgCnfProc: set rfcfg err");
        ulRslt = At_RfCfgCnfReturnErrProc(ucIndex);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    else
    {
        ulRslt = AT_OK;
        At_RfCfgCnfReturnSuccProc(ucIndex);

        /* ^FRSSI?在GDSP LOAD情况会收到ID_HPA_AT_RF_CFG_CNF消息,不用上报,等收到
           ID_HPA_AT_RF_RX_RSSI_IND消息时再上报 */
        if (AT_CMD_QUERY_RSSI != gastAtClientTab[ucIndex].CmdCurrentOpt)
        {
            AT_STOP_TIMER_CMD_READY(ucIndex);
            At_FormatResultData(ucIndex, ulRslt);
        }
    }
    return;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID  At_CHpaRfCfgCnfProc(
    CHPA_AT_RF_CFG_CNF_STRU             *pstMsg
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (AT_HPA_RSLT_FAIL == pstMsg->usErrFlg)
    {
        AT_INFO_LOG("At_CHpaRfCfgCnfProc: set rfcfg err");
        ulRslt = At_RfCfgCnfReturnErrProc(ucIndex);
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    else
    {
        ulRslt = AT_OK;
        At_RfCfgCnfReturnSuccProc(ucIndex);

        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, ulRslt);
    }
    return;
}
#endif


VOS_UINT32 AT_RcvDrvAgentTseLrfSetRsp(VOS_VOID *pMsg)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucIndex;
    DRV_AGENT_TSELRF_SET_CNF_STRU      *pstEvent;
    DRV_AGENT_MSG_STRU                 *pstRcvMsg;

    /* 初始化 */
    pstRcvMsg              = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent               = (DRV_AGENT_TSELRF_SET_CNF_STRU *)pstRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待TSELRF设置命令的结果事件上报 */
    if (AT_CMD_TSELRF_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 输出查询结果 */
    gstAtSendData.usBufLen = 0;
    if (DRV_AGENT_TSELRF_SET_NO_ERROR == pstEvent->enResult)
    {
        /* 设置错误码为AT_OK */
        ulRet                            = AT_OK;
        g_stAtDevCmdCtrl.bDspLoadFlag    = VOS_TRUE;
        g_stAtDevCmdCtrl.ucDeviceRatMode = pstEvent->ucDeviceRatMode;
        g_stAtDevCmdCtrl.usFDAC          = 0;

    }
    else
    {
        /* 查询失败返回ERROR字符串 */
        ulRet                            = AT_ERROR;
    }

    /* 4. 调用At_FormatResultData输出结果 */
    At_FormatResultData(ucIndex, ulRet);
    return VOS_OK;
}


VOS_VOID At_RfFpowdetTCnfProc(PHY_AT_POWER_DET_CNF_STRU *pstMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* 获取本地保存的用户索引 */
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (AT_CMD_FPOWDET_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("At_RfFPOWDETCnfProc: CmdCurrentOpt is not AT_CMD_FPOWDET_QRY!");
        return;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 应物理层要求，如果返回值为0x7FFF则为无效值，项查询者返回ERROR */
    if(0x7FFF == pstMsg->sPowerDet)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           "%s: %d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstMsg->sPowerDet);

        gstAtSendData.usBufLen = usLength;

        At_FormatResultData(ucIndex, AT_OK);
    }

    return;
}


VOS_UINT32 AT_RcvMtaPowerDetQryCnf(VOS_VOID *pMsg)
{
    /* 定义局部变量 */
    AT_MTA_MSG_STRU                    *pstMtaMsg;
    MTA_AT_POWER_DET_QRY_CNF_STRU      *pstPowerDetQryCnf;
    PHY_AT_POWER_DET_CNF_STRU           stPowerNetMsg;

    /* 初始化消息变量 */
    pstMtaMsg           = (AT_MTA_MSG_STRU *)pMsg;
    pstPowerDetQryCnf   = (MTA_AT_POWER_DET_QRY_CNF_STRU *)pstMtaMsg->aucContent;

    TAF_MEM_SET_S(&stPowerNetMsg, sizeof(stPowerNetMsg), 0x00, sizeof(PHY_AT_POWER_DET_CNF_STRU));

    if (MTA_AT_RESULT_NO_ERROR == pstPowerDetQryCnf->enResult)
    {
        stPowerNetMsg.sPowerDet = pstPowerDetQryCnf->sPowerDet;
    }
    else
    {
        stPowerNetMsg.sPowerDet = 0x7FFF;
    }

    At_RfFpowdetTCnfProc(&stPowerNetMsg);

    return VOS_OK;
}


VOS_VOID At_RfPllStatusCnfProc(PHY_AT_RF_PLL_STATUS_CNF_STRU *pstMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* 获取本地保存的用户索引 */
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (AT_CMD_FPLLSTATUS_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("At_RfPllStatusCnfProc: CmdCurrentOpt is not AT_CMD_FPLLSTATUS_QRY!");
        return;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: %d,%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstMsg->usTxStatus,
                                       pstMsg->usRxStatus);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return;
}


VOS_VOID  At_RfRssiIndProc(
    HPA_AT_RF_RX_RSSI_IND_STRU          *pstMsg
)
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    VOS_INT32                           lRssi;

    /*获取本地保存的用户索引*/
    ucIndex  = g_stAtDevCmdCtrl.ucIndex;

    if (AT_DSP_RF_AGC_STATE_ERROR == pstMsg->sAGCGain)  /* 错误 */
    {
        AT_WARN_LOG("AT_RfRssiIndProc err");
        ulRslt = AT_FRSSI_OTHER_ERR;
    }
    else
    {

        gstAtSendData.usBufLen = 0;

        /*由于RSSI测量值单位0.125dBm，为了消除浮点数*1000.*/
        lRssi = pstMsg->sRSSI * AT_DSP_RSSI_VALUE_MUL_THOUSAND;

        /*读取的RSSI值，采用正值上报，精确到0.1dBm定制值信息。如果当前的RSSI
          值为-85.1dBm，返回值为851. 由于之前乘1000，所以精确到0.1dBm这里要除100*/
        if (lRssi < 0 )
        {
            lRssi = (-1*lRssi)/100;
        }
        else
        {
            lRssi = lRssi/100;
        }

        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           lRssi);

        gstAtSendData.usBufLen = usLength;
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return;
}


VOS_UINT32  At_PdmCtrlCnfProc( HPA_AT_PDM_CTRL_CNF_STRU *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    ucIndex         = g_stAtDevCmdCtrl.ucIndex;

    if ( AT_CMD_PDM_CTRL != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (AT_HPA_RSLT_FAIL == pstMsg->ulResult)
    {
        AT_INFO_LOG("At_PdmCtrlCnfProc: read PdmCtrl err");
        ulRslt      = AT_ERROR;
    }
    else
    {
        ulRslt      = AT_OK;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_VOID  At_RfCfgCnfReturnSuccProc(
    VOS_UINT8                           ucIndex
)
{
    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        case AT_CMD_SET_FTXON:
            g_stAtDevCmdCtrl.ucTxOnOff = g_stAtDevCmdCtrl.ucTempRxorTxOnOff;

            /* 如果是打开发射机操作，需要记录最近一次执行的是打开发射机还是打开接收机操作 */
            if (AT_DSP_RF_SWITCH_ON == g_stAtDevCmdCtrl.ucTxOnOff)
            {
                g_stAtDevCmdCtrl.ucRxonOrTxon = AT_TXON_OPEN;
            }
            break;

        case AT_CMD_SET_FRXON:
            g_stAtDevCmdCtrl.ucRxOnOff = g_stAtDevCmdCtrl.ucTempRxorTxOnOff;

            /* 如果是打开接收机操作，需要记录最近一次执行的是打开发射机还是打开接收机操作 */
            if (AT_DSP_RF_SWITCH_ON == g_stAtDevCmdCtrl.ucRxOnOff)
            {
                g_stAtDevCmdCtrl.ucRxonOrTxon = AT_RXON_OPEN;
            }
            break;

        case AT_CMD_QUERY_RSSI:
            break;

        case AT_CMD_SET_RXDIV:
           if ((AT_OK == At_SaveRxDivPara(g_stAtDevCmdCtrl.usOrigBand, 1))
            && (AT_TMODE_FTM == g_stAtDevCmdCtrl.ucCurrentTMode))
           {
               g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_DIV_ON;
               g_stAtDevCmdCtrl.usRxDiv    = g_stAtDevCmdCtrl.usOrigBand;
           }
           break;

        case AT_CMD_SET_RXPRI:
            g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_PRI_ON;
            g_stAtDevCmdCtrl.usRxPri    = g_stAtDevCmdCtrl.usOrigBand;
            break;

        default:
            break;
    }

    return;

}


VOS_VOID  At_WTxCltIndProc(
    WPHY_AT_TX_CLT_IND_STRU            *pstMsg
)
{
    /* 初始化全局变量 */
    TAF_MEM_SET_S(&g_stAtDevCmdCtrl.stCltInfo, sizeof(g_stAtDevCmdCtrl.stCltInfo), 0x0, sizeof(AT_TX_CLT_INFO_STRU));

    /* 设置CLT信息有效标志 */
    g_stAtDevCmdCtrl.stCltInfo.ulInfoAvailableFlg   = VOS_TRUE;

    /* 将接入层上报的信息记录下全局变量中 */
    g_stAtDevCmdCtrl.stCltInfo.shwGammaReal             = pstMsg->shwGammaReal;                 /* 反射系数实部 */
    g_stAtDevCmdCtrl.stCltInfo.shwGammaImag             = pstMsg->shwGammaImag;                 /* 反射系数虚部 */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc0          = pstMsg->ushwGammaAmpUc0;              /* 驻波检测场景0反射系数幅度 */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc1          = pstMsg->ushwGammaAmpUc1;              /* 驻波检测场景1反射系数幅度 */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc2          = pstMsg->ushwGammaAmpUc2;              /* 驻波检测场景2反射系数幅度 */
    g_stAtDevCmdCtrl.stCltInfo.ushwGammaAntCoarseTune   = pstMsg->ushwGammaAntCoarseTune;       /* 粗调格点位置 */
    g_stAtDevCmdCtrl.stCltInfo.ulwFomcoarseTune         = pstMsg->ulwFomcoarseTune;             /* 粗调FOM值 */
    g_stAtDevCmdCtrl.stCltInfo.ushwCltAlgState          = pstMsg->ushwCltAlgState;              /* 闭环算法收敛状态 */
    g_stAtDevCmdCtrl.stCltInfo.ushwCltDetectCount       = pstMsg->ushwCltDetectCount;           /* 闭环收敛总步数 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac0                 = pstMsg->ushwDac0;                     /* DAC0 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac1                 = pstMsg->ushwDac1;                     /* DAC1 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac2                 = pstMsg->ushwDac2;                     /* DAC2 */
    g_stAtDevCmdCtrl.stCltInfo.ushwDac3                 = pstMsg->ushwDac3;                     /* DAC3 */

    return;
}


VOS_UINT32 AT_RcvMtaRficSsiRdQryCnf(VOS_VOID *pMsg)
{
    /* 定义局部变量 */
    AT_MTA_MSG_STRU                    *pstMtaMsg;
    MTA_AT_RFICSSIRD_CNF_STRU          *pstRficSsiRdCnf;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化消息变量 */
    ucIndex                 = 0;
    pstMtaMsg               = ( AT_MTA_MSG_STRU * )pMsg;
    pstRficSsiRdCnf         = ( MTA_AT_RFICSSIRD_CNF_STRU * )pstMtaMsg->aucContent;

    ucIndex                 = g_stAtDevCmdCtrl.ucIndex;

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaRficSsiRdQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_RFICSSIRD_SET */
    if ( AT_CMD_RFICSSIRD_SET != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvMtaRficSsiRdQryCnf: WARNING:Not AT_CMD_REFCLKFREQ_READ!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 输出寄存器的值 */
    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                                     "%s: %d",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     pstRficSsiRdCnf->ulRegValue );
    ulResult                = AT_OK;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32  At_MipiRdCnfProc( HPA_AT_MIPI_RD_CNF_STRU *pstMsg )
{
    VOS_UINT32                          usRslt;
    VOS_UINT32                          usData;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* 初始化本地变量 */
    usLength        = 0;

    /*获取本地保存的用户索引*/
    ucIndex         = g_stAtDevCmdCtrl.ucIndex;

    if (AT_CMD_MIPI_RD != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (AT_HPA_RSLT_FAIL == pstMsg->ulResult)
    {
        AT_INFO_LOG("At_MipiRdCnfProc: read mipi err");
        usRslt = AT_ERROR;
    }
    else
    {
        usRslt      = AT_OK;
        usData      = pstMsg->ulValue;

        usLength    = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              "%s:%d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              usData);
    }
    gstAtSendData.usBufLen  = usLength;
    At_FormatResultData(ucIndex, usRslt);
    return VOS_OK;
}


VOS_UINT32  At_MipiWrCnfProc( HPA_AT_MIPI_WR_CNF_STRU       *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

     if ( AT_CMD_MIPI_WR != gastAtClientTab[ucIndex].CmdCurrentOpt )
     {
          return VOS_ERR;
     }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (AT_HPA_RSLT_FAIL == pstMsg->usErrFlg)
    {
        AT_INFO_LOG("At_MipiWrCnfProc: set MipiCfg err");
        ulRslt  = AT_ERROR;
    }
    else
    {
        ulRslt  = AT_OK;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32  At_SsiWrCnfProc( HPA_AT_SSI_WR_CNF_STRU         *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    ucIndex     = g_stAtDevCmdCtrl.ucIndex;

    if (AT_CMD_SSI_WR != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);


    if (AT_HPA_RSLT_FAIL == pstMsg->usErrFlg)
    {
        AT_INFO_LOG("At_SsiWrCnfProc: set rfcfg err");
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt  = AT_OK;
    }

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32  At_SsiRdCnfProc( HPA_AT_SSI_RD_CNF_STRU          *pstMsg )
{
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulData;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* 初始化本地变量 */
    usLength        = 0;

    /*获取本地保存的用户索引*/
    ucIndex         = g_stAtDevCmdCtrl.ucIndex;

    if ( AT_CMD_SSI_RD != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
          return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (AT_HPA_RSLT_FAIL == pstMsg->ulResult)
    {
        AT_INFO_LOG("At_MipiRdCnfProc: read SSI err");
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt      = AT_OK;
        ulData      = pstMsg->ulValue;

        usLength    = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            "%s:%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            ulData );
    }

    gstAtSendData.usBufLen  = usLength;
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SetFchanRspErr(DRV_AGENT_FCHAN_SET_ERROR_ENUM_UINT32  enResult)
{
    if(DRV_AGENT_FCHAN_BAND_NOT_MATCH == enResult)
    {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    if(DRV_AGENT_FCHAN_BAND_CHANNEL_NOT_MATCH == enResult)
    {
        return AT_FCHAN_BAND_CHANNEL_NOT_MATCH;
    }

    if(DRV_AGENT_FCHAN_OTHER_ERR == enResult)
    {
        g_stAtDevCmdCtrl.bDspLoadFlag = VOS_FALSE;
        AT_WARN_LOG("AT_SetFChanPara: DSP Load fail!");
        return AT_FCHAN_OTHER_ERR;
    }

    return DRV_AGENT_FCHAN_SET_NO_ERROR;

}


VOS_UINT32  At_RfCfgCnfReturnErrProc(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRslt;

    switch (gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
       case AT_CMD_SET_FTXON:
           ulRslt = AT_FTXON_SET_FAIL;
           break;

       case AT_CMD_SET_FRXON:
           ulRslt = AT_FRXON_SET_FAIL;
           break;

       case AT_CMD_QUERY_RSSI:
           ulRslt = AT_FRSSI_OTHER_ERR;
           break;

       /* 设置主集和分集时收到DSP回复出错情况下返回的错误码相同 */
       case AT_CMD_SET_RXDIV:
       case AT_CMD_SET_RXPRI:
           ulRslt = AT_CME_RX_DIV_OTHER_ERR;
           break;

       default:
           ulRslt = AT_ERROR;
           break;
    }

   return ulRslt;
}


#else
/*****************************************************************************
  4 新函数实现
*****************************************************************************/


VOS_UINT32 At_SndRxOnOffReq(VOS_VOID)
{
    BBIC_CAL_RF_DEBUG_RX_REQ_STRU      *pstRxReq = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;

    /* 分配消息空间 */
    pstRxReq = (BBIC_CAL_RF_DEBUG_RX_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(BBIC_CAL_RF_DEBUG_RX_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == pstRxReq)
    {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstRxReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstRxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_RF_DEBUG_RX_REQ);

    /* 填写消息内容 */
    pstRxReq->stPara.uhwRxEnable                = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
    /* 使能RSSI上报 */
    pstRxReq->stPara.ucWbRssiReport             = VOS_TRUE;
    pstRxReq->stPara.enRxCfgMode                = BBIC_CAL_CONFIG_MODE_GAIN_INDEX;

    /* 公共部分 */
    /* MT工位测试一个modem就能覆盖所有的天线，且当前非信令下没有modem的概念 */
    pstRxReq->stPara.stCommonInfo.enModemId     = MODEM_ID_0;
    pstRxReq->stPara.stCommonInfo.enRatMode     = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;
    pstRxReq->stPara.stCommonInfo.enBand        = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    pstRxReq->stPara.stCommonInfo.enBandWith    = g_stMtInfoCtx.stBbicInfo.enBandWidth;
    
    if (AT_ANT_TYPE_MIMO == g_stMtInfoCtx.stBbicInfo.enRxAntType)
    {
        pstRxReq->stPara.stCommonInfo.uhwMimoType   = g_stMtInfoCtx.stBbicInfo.enRxMimoType;
        pstRxReq->stPara.ucAntCfgMap                = g_stMtInfoCtx.stBbicInfo.enRxMimoAntNum;
    }
    else
    {
        pstRxReq->stPara.stCommonInfo.uhwMimoType   = AT_SET_BIT32(1);
        pstRxReq->stPara.ucAntCfgMap                = g_stMtInfoCtx.stBbicInfo.enRxAntType;
    }
    
    if (MODU_TYPE_BUTT == g_stMtInfoCtx.stBbicInfo.enFwaveType)
    {
        pstRxReq->stPara.stCommonInfo.uhwSignalType = 0;                        /* 单音 */
        pstRxReq->stPara.stCommonInfo.enModType     = MODU_TYPE_LTE_QPSK;

        if (AT_RAT_MODE_GSM == g_stMtInfoCtx.stAtInfo.enRatMode)
        {
            pstRxReq->stPara.stCommonInfo.enModType         = MODU_TYPE_GMSK;
        }

        if (AT_RAT_MODE_EDGE == g_stMtInfoCtx.stAtInfo.enRatMode)
        {
            pstRxReq->stPara.stCommonInfo.enModType         = MODU_TYPE_8PSK;
        }
        
    }
    else
    {
        pstRxReq->stPara.stCommonInfo.uhwSignalType = 1;                        /* 调制 */
        pstRxReq->stPara.stCommonInfo.enModType     = g_stMtInfoCtx.stBbicInfo.enFwaveType;
    }

    if (RAT_MODE_NR == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
    {
        pstRxReq->stPara.stCommonInfo.enWaveType    = WAVEFORM_TYPE_DFT_S_OFDM; /* 这个参数没有命令设置，与产品线确认默认值为DFT_S_OFDM */
    }

    /* RX只需要填下行 */
    pstRxReq->stPara.stCommonInfo.uwRxFreqInfo      = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulDlFreq;

    /* 由于产品线每次只测一根天一，所以这两个数组暂都填成一样的 */
    for (ulIndex = 0; ulIndex < RFHAL_CAL_MAX_ANT_NUM; ulIndex++)
    {
        pstRxReq->stPara.aunAntInfo[ulIndex].stAgcGainIndex.uhwAgcTableIndex   = 0xFFFF;
        pstRxReq->stPara.aunAntInfo[ulIndex].stAgcGainIndex.uhwAgcGainIndex    = g_stMtInfoCtx.stAtInfo.ucAgcGainLevel;
        pstRxReq->stPara.aunAntInfo[ulIndex].stAgcGainIndex.enWorkType         = AGC_WORK_TYPE_NOBLOCK;
    }

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstRxReq))
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndRxOnOffReq Exit");

    return VOS_TRUE;
}


VOS_UINT32 At_SndTxOnOffReq(VOS_UINT32 ulPowerDetFlg)
{
    BBIC_CAL_RF_DEBUG_TX_REQ_STRU      *pstTxReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    pstTxReq = (BBIC_CAL_RF_DEBUG_TX_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(BBIC_CAL_RF_DEBUG_TX_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == pstTxReq)
    {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstTxReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstTxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_RF_DEBUG_TX_REQ);

    /* 开关 */
    pstTxReq->stPara.uhwTxEnable = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;

    /* 公共信息部分 */
    /* MT工位测试一个modem就能覆盖所有的天线，且当前非信令下没有modem的概念 */
    pstTxReq->stPara.stCommonInfo.enModemId     = MODEM_ID_0;
    pstTxReq->stPara.stCommonInfo.enRatMode     = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;
    pstTxReq->stPara.stCommonInfo.enBand        = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    pstTxReq->stPara.stCommonInfo.enBandWith    = g_stMtInfoCtx.stBbicInfo.enBandWidth;

    if (AT_ANT_TYPE_MIMO == g_stMtInfoCtx.stBbicInfo.enTxAntType)
    {
        pstTxReq->stPara.stCommonInfo.uhwMimoType   = g_stMtInfoCtx.stBbicInfo.enTxMimoType;
        pstTxReq->stPara.uhwAntMap                  = g_stMtInfoCtx.stBbicInfo.enTxMimoAntNum;
    }
    else
    {   
        /* 默认是1 TX */
        pstTxReq->stPara.stCommonInfo.uhwMimoType   = AT_SET_BIT32(0);
        pstTxReq->stPara.uhwAntMap                  = AT_MIMO_ANT_NUM_1;
    }

    if (MODU_TYPE_BUTT == g_stMtInfoCtx.stBbicInfo.enFwaveType)
    {
        pstTxReq->stPara.stCommonInfo.uhwSignalType     = 0;                    /* 单音 */
        pstTxReq->stPara.stCommonInfo.enModType         = MODU_TYPE_LTE_QPSK;   /* 单音默认写QPSK */

        if (AT_RAT_MODE_GSM == g_stMtInfoCtx.stAtInfo.enRatMode)
        {
            pstTxReq->stPara.stCommonInfo.enModType         = MODU_TYPE_GMSK;
        }

        if (AT_RAT_MODE_EDGE == g_stMtInfoCtx.stAtInfo.enRatMode)
        {
            pstTxReq->stPara.stCommonInfo.enModType         = MODU_TYPE_8PSK;
        }
        
        /* TX信号参数 */
        pstTxReq->stPara.stTxSignalPara.uhwSignalCnt    = 1;                    /* 单音写1 */
    }
    else
    {
        pstTxReq->stPara.stCommonInfo.uhwSignalType     = 2;                    /* 调制 */
        pstTxReq->stPara.stCommonInfo.enModType         = g_stMtInfoCtx.stBbicInfo.enFwaveType;
        /* TX信号参数 */
        pstTxReq->stPara.stTxSignalPara.uhwSignalCnt    = 0;                    /* 调制写0 */
    }

    pstTxReq->stPara.stTxSignalPara.aswOffset[0]   = (VOS_INT32)At_GetBaseFreq(g_stMtInfoCtx.stBbicInfo.enCurrtRatMode);                    
    pstTxReq->stPara.stTxSignalPara.aswOffset[1]   = (VOS_INT32)At_GetBaseFreq(g_stMtInfoCtx.stBbicInfo.enCurrtRatMode);

    if (RAT_MODE_NR == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
    {
        pstTxReq->stPara.stCommonInfo.enWaveType    = WAVEFORM_TYPE_DFT_S_OFDM; /* 这个参数没有命令设置，与产品线确认默认值为DFT_S_OFDM */

        /* 其它参数 */
        pstTxReq->stPara.unTxRatPara.stNrModuPara.enScsType = g_stMtInfoCtx.stBbicInfo.enBbicScs;
        pstTxReq->stPara.unTxRatPara.stNrModuPara.uhwRbNum  = (UINT16)g_stMtInfoCtx.stBbicInfo.enBandWidthValue / (15 * 12);
    }

    /* TX只需要填上行 */
    pstTxReq->stPara.stCommonInfo.uwTxFreqInfo      = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulUlFreq;

    /* TX功率参数 */
    pstTxReq->stPara.stTxPowerPara.enPowerCtrlMode          = POWER_CTRL_MODE_POWER;    /* MT默认写Power mode */
    pstTxReq->stPara.stTxPowerPara.stPaPara.ucTxPaMode      = g_stMtInfoCtx.stBbicInfo.enPaLevel;
    pstTxReq->stPara.stTxPowerPara.unPowerPara.shwTxPower   = g_stMtInfoCtx.stBbicInfo.sFwavePower;

    /* 是否查询Power Det */
    if (VOS_TRUE == ulPowerDetFlg)
    {
        pstTxReq->stPara.stMRxPara.uhwMrxEanble                 = VOS_TRUE;
        pstTxReq->stPara.stMRxPara.enMrxEstMode                 = MRX_ESTIMATE_POWER_MODE;
        pstTxReq->stPara.stMRxPara.enMrxCfgCode                 = BBIC_CAL_CONFIG_MODE_GAIN_INDEX;
        pstTxReq->stPara.stMRxPara.unMrxConfig.uhwMrxGainIndex  = 0;            /* 默认写0 */
    }

    /* 其它参数 */
    if (RAT_MODE_LTE == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
    {
        pstTxReq->stPara.unTxRatPara.stLteRbPara.ucRbStart = 0;            /* 默认写0 */
        /* 带宽/(15KHZ * 12) */
        pstTxReq->stPara.unTxRatPara.stLteRbPara.ucRbNum   = \
                                       (VOS_UINT8)g_stMtInfoCtx.stBbicInfo.enBandWidthValue / (15 * 12);
    }

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstTxReq))
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndTxOnOffReq Exit");

    return VOS_TRUE;

}


VOS_UINT32  At_LoadPhy(VOS_VOID)
{
    AT_CCBT_LOAD_PHY_REQ_STRU          *pstLoadPhyReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    pstLoadPhyReq = (AT_CCBT_LOAD_PHY_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(AT_CCBT_LOAD_PHY_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == pstLoadPhyReq)
    {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstLoadPhyReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstLoadPhyReq, CCPU_PID_CBT, ID_AT_CCBT_SET_WORK_MODE_REQ);

    /* 填写消息内容 */
    pstLoadPhyReq->usRat        = g_stMtInfoCtx.stBbicInfo.enLoadDspMode;
    pstLoadPhyReq->ucFtmMode    = AT_MT_LOAD_DSP_FTM_MODE;                                            /* CT类型,值为5 */
    pstLoadPhyReq->ucBusiness   = 1;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstLoadPhyReq))
    {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_LoadPhy Exit");

    return VOS_TRUE;
}


VOS_VOID AT_ProcCbtMsg(VOS_VOID *pstMsg)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usMsgId;


    ulMsgCnt = sizeof(g_astAtProcCbtMsgTab) / sizeof(AT_PROC_CBT_MSG_STRU);

    usMsgId  = ((AT_MT_MSG_HEADER_STRU *)pstMsg)->usMsgId;

    /* g_astAtProcCbtMsgTab查表，进行消息分发 */
    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtProcCbtMsgTab[i].ulMsgType == usMsgId)
        {
            ulRst = g_astAtProcCbtMsgTab[i].pProcMsgFunc(pstMsg);

            if (VOS_ERR == ulRst)
            {
                AT_ERR_LOG("AT_ProcCbtMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /*没有找到匹配的消息*/
    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcCbtMsg: Msg Id is invalid!");
    }

    return;
}


VOS_VOID AT_ProcBbicMsg(VOS_VOID *pstMsg)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulMsgCnt;
    VOS_UINT32                          ulRst;
    VOS_UINT16                          usMsgId;


    ulMsgCnt = sizeof(g_astAtProcBbicMsgTab) / sizeof(AT_PROC_BBIC_MSG_STRU);

    usMsgId  = ((AT_MT_MSG_HEADER_STRU *)pstMsg)->usMsgId;

    /* g_astAtProcBbicMsgTab查表，进行消息分发 */
    for (i = 0; i < ulMsgCnt; i++)
    {
        if (g_astAtProcBbicMsgTab[i].ulMsgType == usMsgId)
        {
            ulRst = g_astAtProcBbicMsgTab[i].pProcMsgFunc(pstMsg);

            if (VOS_ERR == ulRst)
            {
                AT_ERR_LOG("AT_ProcBbicMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /*没有找到匹配的消息*/
    if (ulMsgCnt == i)
    {
        AT_ERR_LOG("AT_ProcBbicMsg: Msg Id is invalid!");
    }

    return;
}


VOS_UINT32 AT_ProcSetWorkModeCnf(VOS_VOID *pstMsg)
{
    AT_CCBT_LOAD_PHY_CNF_STRU          *pstLoadCnf  = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pstLoadCnf  = (AT_CCBT_LOAD_PHY_CNF_STRU *)pstMsg;
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;

    AT_PR_LOGH("AT_ProcSetWorkModeCnf Enter");

    /* 下标保护 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_ProcSetWorkModeCnf: ulIndex err!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_FCHAN_SET */
    if (AT_CMD_FCHAN_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("AT_ProcSetWorkModeCnf: Not AT_CMD_FCHAN_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 非0表示错误 */
    if (MT_OK != pstLoadCnf->ulErrorCode)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_FCHAN_LOAD_DSP_ERR);
        return VOS_OK;
    }

    gstAtSendData.usBufLen = 0;
    g_stMtInfoCtx.stAtInfo.bDspLoadFlag = VOS_TRUE;
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_UINT32 AT_ProcTxRxCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_MSG_CNF_STRU              *pstCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstCnf          = (BBIC_CAL_MSG_CNF_STRU *)pstMsg;

    AT_PR_LOGH("AT_ProcTxRxCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_ProcTxRxCnf: ulIndex err!");
        return VOS_ERR;
    }

    if ((AT_CMD_SET_FTXON   != gastAtClientTab[ucIndex].CmdCurrentOpt)
     && (AT_CMD_SET_FRXON   != gastAtClientTab[ucIndex].CmdCurrentOpt)
     && (AT_CMD_FPOWDET_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt))
    {
        AT_ERR_LOG("AT_ProcTxRxCnf: Not Set FTXON,FRXON,FPOWDET QRY !");
        return VOS_ERR;
    }

    /* Power Det是借助TXON的消息发送的，如果是查询power det，除了回这条消息，还要回复 ID_BBIC_TOOL_CAL_RF_DEBUG_TX_RESULT_IND */
    if (AT_CMD_FPOWDET_QRY == gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        if (MT_OK != pstCnf->stPara.uwErrorCode)
        {
            /* 复位AT状态 */
            AT_STOP_TIMER_CMD_READY(ucIndex);
            gstAtSendData.usBufLen = 0;
            At_FormatResultData(ucIndex, AT_ERROR);
            return VOS_OK;
        }

        return VOS_OK;
    }

    if (AT_CMD_SET_FTXON == gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        /* 复位AT状态 */
        AT_STOP_TIMER_CMD_READY(ucIndex);

        if (MT_OK == pstCnf->stPara.uwErrorCode)
        {
            g_stMtInfoCtx.stAtInfo.enTxOnOff = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
            gstAtSendData.usBufLen = 0;
            At_FormatResultData(ucIndex, AT_OK);
            return VOS_OK;
        }
        else
        {
            gstAtSendData.usBufLen = 0;
            At_FormatResultData(ucIndex, AT_FTXON_SET_FAIL);
            return VOS_OK;
        }
    }

    /* 处理BBIC模块回复的FTXON消息 */
    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MT_OK == pstCnf->stPara.uwErrorCode)
    {
        g_stMtInfoCtx.stAtInfo.enRxOnOff    = g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff;
        gstAtSendData.usBufLen              = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_FRXON_SET_FAIL);
    }

    return VOS_OK;
}


VOS_UINT32 AT_ProcPowerDetCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_TX_RESULT_IND_STRU           *pstTxResultInd = VOS_NULL_PTR;
    VOS_UINT16                                      usLength;
    VOS_UINT8                                       ucIndex;
    VOS_INT32                                       lPowerValue;

    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    usLength        = 0;
    pstTxResultInd  = (BBIC_CAL_RF_DEBUG_TX_RESULT_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_ProcPowerDetCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
       AT_ERR_LOG("AT_ProcPowerDetCnf: ulIndex err !");
       return VOS_ERR;
    }

    if (AT_CMD_FPOWDET_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MT_OK != pstTxResultInd->stPara.uwErrorCode)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_OK;
    }

    if ((RAT_MODE_LTE == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
     || (RAT_MODE_NR == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode))
    {
        /* LTE和NR DSP上报精度为0.125 */
        lPowerValue = pstTxResultInd->stPara.shwAntPower * 10 / 8;             /* 上报精度0.1dB */
    }
    else
    {   
        /* GUC上报精度为0.1 */
        lPowerValue = pstTxResultInd->stPara.shwAntPower; 
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "^FPOWDET:%d",
                                        lPowerValue);

    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicCalMipiReadReq(
    VOS_UINT32                          ulMipiPortSel,
    VOS_UINT32                          ulSlaveId,
    VOS_UINT32                          ulRegAddr,
    VOS_UINT32                          ulByteCnt
)
{
    BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU       *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulLength;

    /* 申请BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU消息 */
    ulLength    = sizeof(BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("AT_SndBbicCalMipiReadReq: alloc msg fail!");
        return AT_FAILURE;
    }

     /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_MIPI_READ_REQ);

    /* 与产品线确认，每次只读写一个MIPI */
    pstMsg->stPara.uwMipiNum                = 1;
    pstMsg->stPara.astCMD[0].bitMipiPortSel = ulMipiPortSel;
    pstMsg->stPara.astCMD[0].bitSlaveId     = ulSlaveId;
    pstMsg->stPara.astCMD[0].bitRegAddr     = ulRegAddr;
    pstMsg->stPara.astCMD[0].bitByteCnt     = ulByteCnt;

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("AT_SndBbicCalMipiReadReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalMipiReadReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32 AT_SndBbicCalMipiWriteReq(
    VOS_UINT32                          ulMipiPortSel,
    VOS_UINT32                          ulSlaveId,
    VOS_UINT32                          ulRegAddr,
    VOS_UINT32                          ulByteCnt,
    VOS_UINT32                          ulValue
)
{
    BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU      *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                  ulLength;

    /* 申请BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU消息 */
    ulLength    = sizeof(BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("AT_SndBbicCalMipiWriteReqMsg: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_MIPI_WRITE_REQ);

    pstMsg->stPara.uwMipiNum                        = 1;
    pstMsg->stPara.astData[0].stCmd.bitMipiPortSel  = ulMipiPortSel;
    pstMsg->stPara.astData[0].stCmd.bitSlaveId      = ulSlaveId;
    pstMsg->stPara.astData[0].stCmd.bitRegAddr      = ulRegAddr;
    pstMsg->stPara.astData[0].stCmd.bitByteCnt      = ulByteCnt;
    pstMsg->stPara.astData[0].stData.bitByte0       = ulValue;

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("AT_SndBbicCalMipiWriteReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalMipiWriteReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32 AT_RcvBbicCalMipiRedCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU    *pstRedCnf = VOS_NULL_PTR;
    VOS_UINT32                               ulRslt;
    VOS_UINT32                               ulByteCnt;
    VOS_UINT16                               usLength;
    VOS_UINT8                                ucIndex;

    ulRslt      = AT_OK;
    usLength    = 0;

    /*获取本地保存的用户索引*/
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstRedCnf   = (BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalMipiRedCnf Enter");

    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiRedCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (AT_CMD_MIPIOPERATE_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiRedCnf: CmdCurrentOpt is not AT_CMD_MIPIOPERATE_SET!");
        return VOS_ERR;
    }

    if (MT_OK == pstRedCnf->uwResult)
    {
        ulRslt   = AT_OK;
        ulByteCnt = pstRedCnf->stPara.astData[0].stCmd.bitByteCnt;

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s: %d",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                   pstRedCnf->stPara.astData[0].stData.bitByte0);

        if (1 < ulByteCnt)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN - usLength,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d",
                                                   pstRedCnf->stPara.astData[0].stData.bitByte1);
        }

        if (2 < ulByteCnt)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN - usLength,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d",
                                                   pstRedCnf->stPara.astData[0].stData.bitByte2);
        }

        if (3 < ulByteCnt)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN - usLength,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d",
                                                   pstRedCnf->stPara.astData[0].stData.bitByte3);
        }

        gstAtSendData.usBufLen  = usLength;
    }
    else
    {
        AT_INFO_LOG("AT_RcvBbicCalMipiRedCnfMsg: read mipi err");
        ulRslt = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_RcvBbicCalMipiWriteCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU      *pstWriteCnf = VOS_NULL_PTR;
    VOS_UINT32                                  ulRslt;
    VOS_UINT8                                   ucIndex;


    /*获取本地保存的用户索引*/
    ulRslt      = AT_OK;
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstWriteCnf = (BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalMipiWriteCnf Enter");

    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiWriteCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (AT_CMD_MIPIOPERATE_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("AT_RcvBbicCalMipiWriteCnf: CmdCurrentOpt is not AT_CMD_MIPIOPERATE_SET!");
        return VOS_ERR;
    }

    if (MT_OK == pstWriteCnf->uwResult)
    {
        ulRslt = AT_OK;
    }
    else
    {
        ulRslt = AT_ERROR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicPllStatusReq(VOS_VOID)
{
    BBIC_CAL_PLL_QRY_REQ_STRU          *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 申请BBIC_CAL_PLL_QRY_REQ_STRU消息 */
    ulLength = sizeof(BBIC_CAL_PLL_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (BBIC_CAL_PLL_QRY_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("AT_SndBbicPllStatusReq: Alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_PLL_QRY_REQ);

    pstMsg->stPara.uhwBand   = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    pstMsg->stPara.enModType = MODU_TYPE_BUTT;                                  /* GSM  会用这个参数，但GSM不测调制模式 */
    pstMsg->stPara.enRatMode = g_stMtInfoCtx.stBbicInfo.enCurrtRatMode;

    /* 向对应PHY发送消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("AT_SndBbicPllStatusReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicPllStatusReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 At_RcvBbicPllStatusCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_PLL_QRY_IND_STRU          *pstQryCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;


    /*获取本地保存的用户索引*/
    ulRslt      = AT_OK;
    usLength    = 0;
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstQryCnf   = (BBIC_CAL_PLL_QRY_IND_STRU *)pstMsg;

    AT_PR_LOGH("At_RcvBbicPllStatusCnf Enter");

    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_ERR_LOG("At_RcvBbicPllStatusCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (AT_CMD_FPLLSTATUS_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("At_RcvBbicPllStatusCnf: CmdCurrentOpt is not AT_CMD_FPLLSTATUS_QRY!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MT_OK == pstQryCnf->stPara.uwErrorCode)
    {
        ulRslt   = AT_OK;
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: %d,%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstQryCnf->stPara.uwTxPllStatus,
                                       pstQryCnf->stPara.uwRxPllStatus);
    }
    else
    {
        ulRslt = AT_ERROR;
    }

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicRssiReq(VOS_VOID)
{
    BBIC_CAL_RF_DEBUG_RSSI_REQ_STRU          *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                ulLength;

    /* 申请BBIC_CAL_RF_DEBUG_RSSI_REQ_STRU消息 */
    ulLength = sizeof(BBIC_CAL_RF_DEBUG_RSSI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (BBIC_CAL_RF_DEBUG_RSSI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("AT_SndBbicRssiReq: alloc msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_RF_DEBUG_RSSI_REQ);

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("AT_SndBbicRssiReq: Send msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    AT_PR_LOGH("AT_SndBbicRssiReq Exit");

    return AT_SUCCESS;
}


VOS_UINT32  At_RcvBbicRssiInd(VOS_VOID *pstMsg)
{
    BBIC_CAL_RF_DEBUG_RX_RSSI_IND_STRU *pstRssiIndMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_INT32                           lRssi;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    lRssi           = 0;
    ulRslt          = AT_OK;
    usLength        = 0;
    ucIndex         = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstRssiIndMsg   = (BBIC_CAL_RF_DEBUG_RX_RSSI_IND_STRU *)pstMsg;

    AT_PR_LOGH("At_RcvBbicRssiInd Enter");

    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_ERR_LOG("At_RcvBbicRssiInd: ulIndex err !");
        return VOS_ERR;
    }

    if (AT_CMD_QUERY_RSSI != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("At_RcvBbicRssiInd: CmdCurrentOpt Not Query Rssi !");
        return VOS_ERR;
    }

    /* MIMO场景，1，3，5，7取ashwRssi[0]， 2，4，6，8取ashwRssi[1] */
    if ((AT_ANT_TYPE_PRI == g_stMtInfoCtx.stBbicInfo.enRxAntType)
     || ((1 == g_stMtInfoCtx.stAtInfo.enRxMimoAntNum % 2 )
      && (AT_ANT_TYPE_MIMO == g_stMtInfoCtx.stBbicInfo.enRxAntType)))
    {
        /*由于RSSI测量值单位0.125dBm，为了消除浮点数*1000.*/
        lRssi = pstRssiIndMsg->stPara.ashwRssi[0] * AT_DSP_RSSI_VALUE_MUL_THOUSAND;
    }
    else
    {
        /*由于RSSI测量值单位0.125dBm，为了消除浮点数*1000.*/
        lRssi = pstRssiIndMsg->stPara.ashwRssi[1] * AT_DSP_RSSI_VALUE_MUL_THOUSAND;
    }

    if (MT_OK != pstRssiIndMsg->stPara.uwErrorCode)
    {
        AT_ERR_LOG("At_RcvBbicRssiIndProc err");
        ulRslt = AT_ERROR;
    }
    else
    {
        /*读取的RSSI值，采用正值上报，精确到0.01dBm定制值信息。如果当前的RSSI
          值为-85.1dBm，返回值为8510. 由于之前乘1000，所以精确到0.01dBm这里要除10*/
        if (lRssi < 0 )
        {
            lRssi = (-1*lRssi)/100;
        }
        else
        {
            lRssi = lRssi/100;
        }

        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%s:%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           lRssi);

        gstAtSendData.usBufLen = usLength;
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);
    
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_SndBbicCalSetDpdtReq(
    BBIC_DPDT_OPERTYPE_ENUM_UINT16      enOperType,
    VOS_UINT32                          ulValue,
    VOS_UINT32                          ulWorkType
)
{
    BBIC_CAL_DPDT_REQ_STRU             *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 申请BBIC_CAL_DPDT_REQ_STRU消息 */
    ulLength = sizeof(BBIC_CAL_DPDT_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (BBIC_CAL_DPDT_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("At_SndBbicCalDpdtReq: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_DPDT_REQ);

    pstMsg->stPara.enRatMode                = g_stMtInfoCtx.stBbicInfo.enDpdtRatMode;
    pstMsg->stPara.enOperType               = enOperType;
    pstMsg->stPara.uwValue                  = ulValue;
    pstMsg->stPara.uhwModemWorkType         = (VOS_UINT16)ulWorkType;

    if ( VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("At_SndBbicCalDpdtReq: Send msg fail!");
        return AT_FAILURE;
    }

    
    AT_PR_LOGH("AT_SndBbicCalSetDpdtReq Exit");

    return AT_SUCCESS;
}



VOS_UINT32 AT_RcvBbicCalSetDpdtCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_DPDT_IND_STRU             *pstDpdtCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    ulRslt          = AT_ERROR;

    /* 获取本地保存的用户索引 */
    ucIndex = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstDpdtCnf = (BBIC_CAL_DPDT_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalSetDpdtCnf Enter");

    if (AT_MAX_CLIENT_NUM <= ucIndex)
    {
        AT_ERR_LOG("AT_RcvBbicCalSetDpdtCnf: ulIndex err !");
        return VOS_ERR;
    }

    if ((AT_CMD_DPDT_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
     && (AT_CMD_DPDTQRY_SET != gastAtClientTab[ucIndex].CmdCurrentOpt))
    {
        AT_ERR_LOG("AT_RcvBbicCalSetDpdtCnf: CmdCurrentOpt is not AT_CMD_DPDT_SET or AT_CMD_DPDTQRY_SET!");
        return VOS_ERR;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MT_OK != pstDpdtCnf->stPara.uwErrorCode)
    {
        AT_ERR_LOG1("AT_RcvBbicCalSetDpdtCnf: set dpdt error, ErrorCode is ", pstDpdtCnf->stPara.uwErrorCode);
        ulRslt = AT_ERROR;
    }
    else
    {
        /* 当操作类型状态为Get时，上报查询Dpdt结果 */
        if (BBIC_DPDT_OPERTYPE_GET == pstDpdtCnf->stPara.unOperType)
        {
            ulRslt = AT_OK;
            gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s: %d",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               pstDpdtCnf->stPara.uwValue);
        }
        /* 当操作类型状态为Set时，返回设置成功 */
        else
        {
            ulRslt = AT_OK;
            gstAtSendData.usBufLen = 0;
        }
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}




VOS_UINT32 AT_SndBbicCalQryFtemprptReq(INT16 lChannelNum)
{
    BBIC_CAL_TEMP_QRY_REQ_STRU         *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    /* 申请BBIC_CAL_TEMP_QRY_REQ_STRU消息 */
    ulLength    = sizeof(BBIC_CAL_TEMP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg      = (BBIC_CAL_TEMP_QRY_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("AT_SndBbicCalQryFtemprptReq: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstMsg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstMsg, DSP_PID_BBA_CAL, ID_AT_BBIC_TEMP_QRY_REQ);

    pstMsg->stPara.enChannelType  = g_stMtInfoCtx.stBbicInfo.enCurrentChannelType;
    pstMsg->stPara.hwChannelNum   = lChannelNum;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("AT_SndBbicCalQryFtemprptReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalQryFtemprptReq Exit");

    return AT_SUCCESS;
}



VOS_UINT32 AT_RcvBbicCalQryFtemprptCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_TEMP_QRY_IND_STRU         *pstFtemprptCnf = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    ulRslt   = AT_ERROR;

    /*获取本地保存的用户索引*/
    ucIndex = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstFtemprptCnf = (BBIC_CAL_TEMP_QRY_IND_STRU *)pstMsg;

    AT_PR_LOGH("AT_RcvBbicCalQryFtemprptCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_RcvBbicCalQryFtemprptCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (AT_CMD_FTEMPRPT_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("AT_RcvBbicCalQryFtemprptCnf: CmdCurrentOpt is not AT_CMD_OPT_READ_CMD!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MT_OK != pstFtemprptCnf->stPara.uwErrorCode)
    {
        ulRslt = AT_ERROR;
        AT_ERR_LOG1("AT_RcvBbicCalQryFtemprptCnf: qry Ftemprpt error, ErrorCode is ", pstFtemprptCnf->stPara.uwErrorCode);
        gstAtSendData.usBufLen = 0;
    }
    else
    {
        ulRslt = AT_OK;
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstFtemprptCnf->stPara.enChannelType,
                                      pstFtemprptCnf->stPara.hwChannelNum,
                                      pstFtemprptCnf->stPara.wTemperature);
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 At_SndDcxoReq(VOS_VOID)
{
    BBIC_CAL_DCXO_REQ_STRU             *pstDcxoReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    pstDcxoReq = (BBIC_CAL_DCXO_REQ_STRU*)PS_ALLOC_MSG(WUEPS_PID_AT,
                           sizeof(BBIC_CAL_DCXO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (VOS_NULL_PTR == pstDcxoReq)
    {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(pstDcxoReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(pstDcxoReq, DSP_PID_BBA_CAL, ID_AT_BBIC_DCXO_REQ);

    pstDcxoReq->stPara.enSetType    = g_stMtInfoCtx.stBbicInfo.enDcxoTempCompEnableFlg;
    pstDcxoReq->stPara.ulTxFreq     = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.ulUlFreq;
    pstDcxoReq->stPara.usBand       = g_stMtInfoCtx.stBbicInfo.stDspBandFreq.usDspBand;
    
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstDcxoReq))
    {
       return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndDcxoReq Exit");

    return VOS_TRUE;
    
}


VOS_UINT32 At_RcvBbicCalDcxoCnf(VOS_VOID *pstMsg)
{
    BBIC_CAL_DCXO_IND_STRU             *pstDcxoInd = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT8                           ucIndex;

    /*获取本地保存的用户索引*/
    ucIndex     = g_stMtInfoCtx.stAtInfo.ucIndex;
    pstDcxoInd  = (BBIC_CAL_DCXO_IND_STRU*)pstMsg;
    ulRslt      = AT_OK;

    AT_PR_LOGH("At_RcvBbicCalDcxoCnf Enter");

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("At_RcvBbicCalDcxoCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (AT_CMD_DCXOTEMPCOMP_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_ERR_LOG("At_RcvBbicCalDcxoCnf: CmdCurrentOpt is not AT_CMD_DCXOTEMPCOMP_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MT_OK != pstDcxoInd->ulErrorCode)
    {
        ulRslt = AT_ERROR;
        AT_ERR_LOG1("At_RcvBbicCalDcxoCnf: ErrorCode is ", pstDcxoInd->ulErrorCode);
        gstAtSendData.usBufLen = 0;
    }
    else
    {
        ulRslt = AT_OK;
        gstAtSendData.usBufLen = 0;
        g_stMtInfoCtx.stAtInfo.enDcxoTempCompEnableFlg = (AT_DCXOTEMPCOMP_ENABLE_ENUM_UINT8)g_stMtInfoCtx.stBbicInfo.enDcxoTempCompEnableFlg;
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


#endif


