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
#include "GuNasLogFilter.h"
#include "MnMsgApi.h"

#include "Taf_Tafm_Remote.h"
#include "AtMtaInterface.h"
#include "MnMsgTs.h"
#include "PsLogFilterInterface.h"
#include "AtImsaInterface.h"


#include "AtInternalMsg.h"
#include "AtParse.h"
#include "AtCtx.h"



#define    THIS_FILE_ID        PS_FILE_ID_GU_NAS_LOG_FILTER_C

#define    AT_CMD_LEN_7             (7)
#define    AT_CMD_LEN_8             (8)
#define    AT_CMD_LEN_13            (13)
#define    AT_SMS_MODE              (1)

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
typedef VOS_VOID* (*pGuNasMsgFilterProcFunc)(
    PS_MSG_HEADER_STRU                 *pstMsg
);



typedef struct
{
    VOS_UINT32                                  ulSenderPid;
    VOS_UINT32                                  ulReceiverPid;
    pGuNasMsgFilterProcFunc                     pFuncFilterProc;
} GUNAS_MSG_FILTER_PROC_TBL_FUNC;

GUNAS_MSG_FILTER_PROC_TBL_FUNC              g_astGuNasMsgFilterProcFuncTbl[] =
{
    {WUEPS_PID_AT,          WUEPS_PID_AT,       GUNAS_FilterAtToAtMsg},

    {I0_WUEPS_PID_TAF,      WUEPS_PID_AT,       GUNAS_FilterTafToAtMsg},
    {WUEPS_PID_AT,          I0_WUEPS_PID_TAF,   GUNAS_FilterAtToTafMsg},
    {WUEPS_PID_AT,          I0_WUEPS_PID_MMA,   GUNAS_FilterAtToMmaMsg},
    {WUEPS_PID_AT,          I0_UEPS_PID_MTA,    GUNAS_FilterAtToMtaMsg},
    {WUEPS_PID_AT,          I0_MAPS_PIH_PID,    GUNAS_FilterAtToPihMsg},

    {I1_WUEPS_PID_TAF,      WUEPS_PID_AT,       GUNAS_FilterTafToAtMsg},
    {WUEPS_PID_AT,          I1_WUEPS_PID_TAF,   GUNAS_FilterAtToTafMsg},
    {WUEPS_PID_AT,          I1_WUEPS_PID_MMA,   GUNAS_FilterAtToMmaMsg},
    {WUEPS_PID_AT,          I1_UEPS_PID_MTA,    GUNAS_FilterAtToMtaMsg},
    {WUEPS_PID_AT,          I1_MAPS_PIH_PID,    GUNAS_FilterAtToPihMsg},

    {I2_WUEPS_PID_TAF,      WUEPS_PID_AT,       GUNAS_FilterTafToAtMsg},
    {WUEPS_PID_AT,          I2_WUEPS_PID_TAF,   GUNAS_FilterAtToTafMsg},
    {WUEPS_PID_AT,          I2_WUEPS_PID_MMA,   GUNAS_FilterAtToMmaMsg},
    {WUEPS_PID_AT,          I2_UEPS_PID_MTA,    GUNAS_FilterAtToMtaMsg},
    {WUEPS_PID_AT,          I2_MAPS_PIH_PID,    GUNAS_FilterAtToPihMsg},

};

VOS_CHAR*                                       g_apcATFileterTable[]=
{
        /* USIM相关 */
        "AT+CPIN"           ,
        "\r\n+CPIN:"        ,
        "AT+CLCK"           ,
        "\r\n+CLCK:"        ,
        "AT+CPWD"           ,
        "\r\n+CPWD:"        ,
        "AT^CPIN"           ,
        "\r\n^CPIN:"        ,
        "AT^CPIN2"          ,
        "\r\n^CPIN2:"       ,
        "AT+CPBW"           ,
        "\r\n+CPBW:"        ,
        "AT+CRSM"           ,
        "\r\n+CRSM:"        ,
        "AT+CSIM"           ,
        "\r\n+CSIM:"        ,
        "AT+CCHO"           ,
        "\r\n+CCHO:"        ,
        "AT+CCHC"           ,
        "AT+CNUM"           ,
        "\r\n+CNUM:"        ,
        "AT+CGLA"           ,
        "\r\n+CGLA:"        ,
        "AT+CRLA"           ,
        "\r\n+CRLA:"        ,
        //删除CIMI
        "AT^CSIN"           ,
        "\r\n^CSIN:"        ,
        "AT^CSTR"           ,
        "\r\n^CSTR:"        ,
        "AT^CSEN"           ,
        "\r\n^CSEN:"        ,
        "\r\n^CCIN:"        ,
        "AT^CISA"           ,
        "\r\n^CISA:"        ,
        "AT^CARDATR"        ,
        "\r\n^CARDATR:"     ,
        "AT^UICCAUTH"       ,
        "\r\n^UICCAUTH:"    ,
        "AT^CURSM"          ,
        "\r\n^CURSM:"       ,
        "AT^CGLA"           ,
        "\r\n^CGLA:"        ,
        "AT^CRLA"           ,
        "\r\n^CRLA:"        ,
        "\r\n^USIMICCID:"   ,

        /* 短信相关 */
        "AT+CMGS"           ,
        "AT+CMGW"           ,
        "AT+CMGC"           ,
        "\r\n+CMT:"         ,
        "\r\n+CMGR:"        ,
        "\r\n^RSTRIGGER:"   ,
        "\r\n+CMGS:"        ,
        "\r\n+CDS:"         ,

        /* SIM LOCK相关 */
        "AT^SIMLOCKUNLOCK"  ,
        "AT^CMLCK"          ,

        "AT^SILENTPININFO"  ,

};


/*****************************************************************************
  3 函数实现
*****************************************************************************/




VOS_UINT32 GUNAS_ATCmdFilter(
    VOS_UINT8                          *pucATData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT32                          i;
    VOS_UINT8                          *pucTempData = VOS_NULL_PTR;

    pucTempData = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, usLen);
    if (VOS_NULL_PTR == pucTempData)
    {
        return VOS_FALSE;
    }

    TAF_MEM_CPY_S(pucTempData, usLen, pucATData, usLen);

    (VOS_VOID)At_UpString(pucTempData, usLen);

    for (i = 0; i < (sizeof(g_apcATFileterTable)/sizeof(g_apcATFileterTable[0])); i++)
    {
        if (VOS_OK == PS_MEM_CMP((VOS_UINT8 *)g_apcATFileterTable[i], pucTempData, VOS_StrLen(g_apcATFileterTable[i])))
        {
            PS_MEM_FREE(WUEPS_PID_AT, pucTempData);
            return VOS_TRUE;
        }
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucTempData);
    return VOS_FALSE;
}


VOS_VOID* GUNAS_FilterAtToAtMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
)
{
    VOS_UINT8                          *pucTmpAdr = VOS_NULL_PTR;
    AT_MSG_STRU                        *pstAtMsg  = VOS_NULL_PTR;
    VOS_UINT16                          usLen;

    usLen  = 0;
    pstAtMsg = (AT_MSG_STRU *)pstMsg;

    switch (pstAtMsg->enMsgId)
    {
        case ID_AT_MNTN_INPUT_MSC:
        case ID_AT_MNTN_OUTPUT_MSC:
        case ID_AT_MNTN_START_FLOW_CTRL:
        case ID_AT_MNTN_STOP_FLOW_CTRL:
        case ID_AT_MNTN_REG_FC_POINT:
        case ID_AT_MNTN_DEREG_FC_POINT:
        case ID_AT_MNTN_PC_REPLAY_MSG:
        case ID_AT_MNTN_PC_REPLAY_CLIENT_TAB:
        case ID_AT_MNTN_RPT_PORT:
        case ID_AT_MNTN_PS_CALL_ENTITY_RPT:
        case ID_AT_COMM_CCPU_RESET_START:
        case ID_AT_COMM_CCPU_RESET_END:
        case ID_AT_COMM_HIFI_RESET_START:
        case ID_AT_COMM_HIFI_RESET_END:
        case ID_AT_NCM_CONN_STATUS_CMD:
        case ID_AT_WATER_LOW_CMD:
        case ID_AT_SWITCH_CMD_MODE:
            return pstMsg;

        default:
        {
            if (pstAtMsg->ucIndex < AT_MAX_CLIENT_NUM)
            {
                if (AT_SMS_MODE == g_stParseContext[pstAtMsg->ucIndex].ucMode)
                {
                    /* 短信模式直接进行过滤 */
                    MN_NORM_LOG1("GUNAS_FilterAtToAtMsg: TRUE,SMS MODE, ulMsgName ", pstAtMsg->enMsgId);
                    return VOS_NULL_PTR;
                }
            }

            pucTmpAdr                   = (VOS_UINT8 *)((VOS_UINT8 *)pstAtMsg
                                                        + sizeof(AT_MSG_STRU)
                                                        - sizeof(pstAtMsg->aucValue));

            usLen                       = pstAtMsg->usLen;

            if (VOS_TRUE == GUNAS_ATCmdFilter(pucTmpAdr, usLen))
            {
                MN_NORM_LOG1("GUNAS_FilterAtToAtMsg: TRUE ulMsgName ", pstAtMsg->enMsgId);
                return VOS_NULL_PTR;
            }

            return pstMsg;
        }
    }
}


VOS_VOID* GUNAS_FilterAtToMmaMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
)
{
    switch (pstMsg->ulMsgName)
    {
        /* 补充业务相关的信息 */
        case TAF_MSG_MMA_OP_PIN_REQ:
        case TAF_MSG_MMA_SET_PIN:
        case TAF_MSG_MMA_ME_PERSONAL_REQ:
            MN_NORM_LOG1("GUNAS_FilterAtToMmaMsg: TRUE ulMsgName ", pstMsg->ulMsgName);
            return VOS_NULL_PTR;

        default:
            return pstMsg;
    }
}


VOS_VOID* GUNAS_FilterAtToMtaMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
)
{
    switch (pstMsg->ulMsgName)
    {
        /* 补充业务相关的信息 */
        case ID_AT_MTA_SIMLOCKUNLOCK_SET_REQ:
            MN_NORM_LOG1("GUNAS_FilterAtToMtaMsg: TRUE ulMsgName ", pstMsg->ulMsgName);
            return VOS_NULL_PTR;

        default:
            return pstMsg;
    }
}


VOS_VOID* GUNAS_FilterAtToPihMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
)
{
    switch (pstMsg->ulMsgName)
    {
        /* 补充业务相关的信息 */
        /* SI_PIH_FDN_ENABLE_REQ */
        case AT_PIH_FDN_ENABLE_REQ:
        /* SI_PIH_FDN_DISALBE_REQ */
        case AT_PIH_FDN_DISALBE_REQ:
            MN_NORM_LOG1("GUNAS_FilterAtToPihMsg: TRUE ulMsgName ", pstMsg->ulMsgName);
            return VOS_NULL_PTR;

        default:
            return pstMsg;
    }
}


VOS_VOID* GUNAS_FilterAtToTafMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
)
{
    switch (pstMsg->ulMsgName)
    {
        /* 短信相关的信息 */
        case MN_MSG_MSGTYPE_SEND_RPDATA_DIRECT:
        case MN_MSG_MSGTYPE_SEND_RPDATA_FROMMEM:
        case MN_MSG_MSGTYPE_WRITE:
        case MN_MSG_MSGTYPE_READ:
        case MN_MSG_MSGTYPE_LIST:
            MN_NORM_LOG1("GUNAS_FilterAtToTafMsg: TRUE ulMsgName ", pstMsg->ulMsgName);
            return VOS_NULL_PTR;

        /* 补充业务相关的信息 */
        case TAF_MSG_ACTIVATESS_MSG:
        case TAF_MSG_DEACTIVATESS_MSG:
        case TAF_MSG_REGPWD_MSG:
            MN_NORM_LOG1("GUNAS_FilterAtToTafMsg: TRUE ulMsgName ", pstMsg->ulMsgName);
            return VOS_NULL_PTR;

        default:
            return pstMsg;
    }
}


VOS_VOID* GUNAS_FilterTafToAtMsg(
    PS_MSG_HEADER_STRU                 *pstMsg
)
{
    MN_AT_IND_EVT_STRU                 *pstAtEvent = VOS_NULL_PTR;
    VOS_UINT32                          ulEventType;

    ulEventType = 0;
    pstAtEvent  = (MN_AT_IND_EVT_STRU *)pstMsg;

    if (MN_CALLBACK_MSG   != pstMsg->ulMsgName)
    {
        return pstMsg;
    }

    TAF_MEM_CPY_S(&ulEventType, (VOS_UINT32)sizeof(ulEventType), pstAtEvent->aucContent, (VOS_UINT32)sizeof(VOS_UINT32));

    switch ((MN_MSG_EVENT_ENUM_U32)ulEventType)
    {
        case MN_MSG_EVT_SUBMIT_RPT:
        case MN_MSG_EVT_MSG_SENT:
        case MN_MSG_EVT_DELIVER:
        case MN_MSG_EVT_READ:
        case MN_MSG_EVT_LIST:
        case MN_MSG_EVT_WRITE:
            MN_NORM_LOG1("GUNAS_FilterTafToAtMsg: TRUE ulEventType ", ulEventType);
            return VOS_NULL_PTR;

        default:
            return pstMsg;
    }
}


VOS_VOID* GUNAS_FilterLayerMsg(
    struct MsgCB                       *pstMsg
)
{
    PS_MSG_HEADER_STRU                 *pstTempMsg = VOS_NULL_PTR;
    pGuNasMsgFilterProcFunc             pfunFilter;
    VOS_UINT32                          i;
    VOS_UINT32                          ulSenderPid;
    VOS_UINT32                          ulReceiverPid;

    pstTempMsg    = (PS_MSG_HEADER_STRU *)pstMsg;
    pfunFilter    = VOS_NULL_PTR;
    ulSenderPid   = pstMsg->ulSenderPid;
    ulReceiverPid = pstMsg->ulReceiverPid;

    if (VOS_FALSE == AT_GetPrivacyFilterEnableFlg())
    {
        return pstMsg;
    }


    for (i = 0; i < (sizeof(g_astGuNasMsgFilterProcFuncTbl)/sizeof(GUNAS_MSG_FILTER_PROC_TBL_FUNC)); i++)
    {
        if ((ulSenderPid   == g_astGuNasMsgFilterProcFuncTbl[i].ulSenderPid)
         && (ulReceiverPid == g_astGuNasMsgFilterProcFuncTbl[i].ulReceiverPid))
        {
            pfunFilter = g_astGuNasMsgFilterProcFuncTbl[i].pFuncFilterProc;
            break;
        }
    }

    if (VOS_NULL_PTR != pfunFilter)
    {
        return pfunFilter(pstTempMsg);
    }

    return pstMsg;
}


VOS_VOID* GUNAS_OM_LayerMsgFilter(
    struct MsgCB                       *pstMsg
)
{
    /* 注册函数接口调整，之前所有pid消息在一起处理过滤，先对所有pid使用当前函数注册过滤，
       在后续脱敏项目中对当前函数进行拆分，各个pid注册自己的过滤函数 */

    return GUNAS_FilterLayerMsg(pstMsg);
}


VOS_VOID  *NAS_OM_LogFilterImsaAtMtStatesIndMsgAcpu(VOS_VOID *pMsg)
{
    IMSA_AT_MT_STATES_IND_STRU          *pstSrcImsaAtMsg = VOS_NULL_PTR;
    IMSA_AT_MT_STATES_IND_STRU          *pstDstImsaAtMsg = VOS_NULL_PTR;

    pstSrcImsaAtMsg = (IMSA_AT_MT_STATES_IND_STRU *)pMsg;

    /* 申请新的内存 */
    pstDstImsaAtMsg = (VOS_VOID*)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, sizeof(IMSA_AT_MT_STATES_IND_STRU));
    if( VOS_NULL_PTR == pstDstImsaAtMsg )
    {
       return pMsg;
    }

    TAF_MEM_CPY_S(pstDstImsaAtMsg,
                   sizeof(IMSA_AT_MT_STATES_IND_STRU),
                   pstSrcImsaAtMsg,
                   sizeof(IMSA_AT_MT_STATES_IND_STRU));
    TAF_MEM_SET_S(pstDstImsaAtMsg->aucAsciiCallNum,
                   sizeof(pstDstImsaAtMsg->aucAsciiCallNum),
                   0xFF,
                   sizeof(pstDstImsaAtMsg->aucAsciiCallNum));

    return pstDstImsaAtMsg;

}


VOS_VOID  *NAS_OM_LogFilterImsaAtVolteImpuCnfMsgAcpu(VOS_VOID *pMsg)
{
    IMSA_AT_VOLTEIMPU_QRY_CNF_STRU          *pstSrcImsaAtMsg = VOS_NULL_PTR;
    IMSA_AT_VOLTEIMPU_QRY_CNF_STRU          *pstDstImsaAtMsg = VOS_NULL_PTR;

    pstSrcImsaAtMsg = (IMSA_AT_VOLTEIMPU_QRY_CNF_STRU *)pMsg;

    /* 申请新的内存 */
    pstDstImsaAtMsg = (VOS_VOID*)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, sizeof(IMSA_AT_VOLTEIMPU_QRY_CNF_STRU));
    if( VOS_NULL_PTR == pstDstImsaAtMsg )
    {
       return pMsg;
    }

    TAF_MEM_CPY_S(pstDstImsaAtMsg,
                   sizeof(IMSA_AT_VOLTEIMPU_QRY_CNF_STRU),
                   pstSrcImsaAtMsg,
                   sizeof(IMSA_AT_VOLTEIMPU_QRY_CNF_STRU));
    TAF_MEM_SET_S(pstDstImsaAtMsg->aucImpu,
                   sizeof(pstDstImsaAtMsg->aucImpu),
                   0XFF,
                   sizeof(pstDstImsaAtMsg->aucImpu));

    return pstDstImsaAtMsg;

}


STATIC VOS_VOID *NAS_OM_LogFilterImsaAtDmuserQryCnfMsgAcpu(VOS_VOID *pMsg)
{
    IMSA_AT_DMUSER_QRY_CNF_STRU          *pstSrcImsaAtMsg = VOS_NULL_PTR;
    IMSA_AT_DMUSER_QRY_CNF_STRU          *pstDstImsaAtMsg = VOS_NULL_PTR;

    pstSrcImsaAtMsg = (IMSA_AT_DMUSER_QRY_CNF_STRU *)pMsg;

    /* 申请新的内存 */
    pstDstImsaAtMsg = (VOS_VOID*)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT,sizeof(IMSA_AT_DMUSER_QRY_CNF_STRU));
    if( VOS_NULL_PTR == pstDstImsaAtMsg )
    {
       return pMsg;
    }

   TAF_MEM_SET_S(pstDstImsaAtMsg,
                sizeof(IMSA_AT_DMUSER_QRY_CNF_STRU),
                0,
                sizeof(IMSA_AT_DMUSER_QRY_CNF_STRU));

    TAF_MEM_CPY_S(pstDstImsaAtMsg,
                  sizeof(IMSA_AT_DMUSER_QRY_CNF_STRU),
                  pstSrcImsaAtMsg,
                  sizeof(IMSA_AT_DMUSER_QRY_CNF_STRU));
    /*过滤IMPI*/
    TAF_MEM_SET_S(pstDstImsaAtMsg->stDmUser.acImpi,
                  sizeof(pstSrcImsaAtMsg->stDmUser.acImpi),
                  0,
                  sizeof(pstSrcImsaAtMsg->stDmUser.acImpi));
     /*过滤IMPU*/
    TAF_MEM_SET_S(pstDstImsaAtMsg->stDmUser.acImpu,
                  sizeof(pstSrcImsaAtMsg->stDmUser.acImpu),
                  0,
                  sizeof(pstSrcImsaAtMsg->stDmUser.acImpu));

    return pstDstImsaAtMsg;
}


VOS_VOID *NAS_OM_LogFilterImsaAtMsgAcpu(
    struct MsgCB                       *pMsg
)
{
    PS_MSG_HEADER_STRU                  *pstTempMsg = VOS_NULL_PTR;
    VOS_VOID                            *pReturnTraceMsg = pMsg;

    pstTempMsg = (PS_MSG_HEADER_STRU *)pMsg;

    if (VOS_FALSE == AT_GetPrivacyFilterEnableFlg())
    {
        return pMsg;
    }

    if (WUEPS_PID_AT != pstTempMsg->ulReceiverPid)
    {
        return pMsg;
    }

    switch (pstTempMsg->ulMsgName)
    {
         case ID_IMSA_AT_MT_STATES_IND:
              pReturnTraceMsg = NAS_OM_LogFilterImsaAtMtStatesIndMsgAcpu(pMsg);
             break;

         case ID_IMSA_AT_VOLTEIMPI_QRY_CNF:
              pReturnTraceMsg = VOS_NULL_PTR;
             break;

         case ID_IMSA_AT_VOLTEIMPU_QRY_CNF:
             pReturnTraceMsg = NAS_OM_LogFilterImsaAtVolteImpuCnfMsgAcpu(pMsg);
            break;
         case ID_IMSA_AT_DMUSER_QRY_CNF:
             pReturnTraceMsg = NAS_OM_LogFilterImsaAtDmuserQryCnfMsgAcpu(pMsg);
            break;
        default:
             break;
    }

    return pReturnTraceMsg;
}


VOS_VOID GUNAS_OM_LayerMsgReplaceCBRegACore(VOS_VOID)
{
    PS_OM_LayerMsgReplaceCBReg(WUEPS_PID_AT, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I0_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I1_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I2_WUEPS_PID_TAF, GUNAS_OM_LayerMsgFilter);

    PS_OM_LayerMsgReplaceCBReg(I0_PS_PID_IMSA, NAS_OM_LogFilterImsaAtMsgAcpu);
    PS_OM_LayerMsgReplaceCBReg(I1_PS_PID_IMSA, NAS_OM_LogFilterImsaAtMsgAcpu);
}


