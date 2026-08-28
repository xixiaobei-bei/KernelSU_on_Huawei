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


#include "PsCommonDef.h"
#include "TafPsApi.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_TAF_APS_API_C


/*****************************************************************************
   2 全局变量定义
*****************************************************************************/

/*****************************************************************************
   3 外部函数声明
*****************************************************************************/

/*****************************************************************************
   4 函数实现
*****************************************************************************/


VOS_UINT32 TAF_PS_SndMsg(
    VOS_UINT16                          usModemId,
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLength
)
{
    VOS_UINT32                          ulResult = 0;
    TAF_PS_MSG_STRU                    *pstMsg   = VOS_NULL_PTR;
    VOS_UINT32                          ulPid;

    switch (usModemId)
    {
        case MODEM_ID_0:
            ulPid = I0_WUEPS_PID_TAF;
            break;

        case MODEM_ID_1:
            ulPid = I1_WUEPS_PID_TAF;
            break;

        case MODEM_ID_2:
            ulPid = I2_WUEPS_PID_TAF;
            break;

        default:
            PS_PRINTF("TAF_PS_SndMsg: ModemId is Error!");
            return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAF_PS_MSG_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(
                                ulPid,
                                sizeof(MSG_HEADER_STRU) + ulLength);
    if (VOS_NULL_PTR == pstMsg)
    {
        return VOS_ERR;
    }

    pstMsg->stHeader.ulReceiverPid      = ulPid;
    pstMsg->stHeader.ulMsgName          = ulMsgId;

    /* 填写消息内容 */
    TAF_MEM_CPY_S(pstMsg->aucContent, ulLength, pData, ulLength);

    /* 发送消息 */
    ulResult = PS_SEND_MSG(ulPid, pstMsg);
    if (VOS_OK != ulResult)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 TAF_PS_GetDsFlowInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_DSFLOW_INFO_REQ_STRU     stGetDsFlowInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetDsFlowInfoReq, sizeof(stGetDsFlowInfoReq), 0x00, sizeof(TAF_PS_GET_DSFLOW_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DSFLOW_INFO_REQ消息 */
    stGetDsFlowInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDsFlowInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDsFlowInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_DSFLOW_INFO_REQ,
                             &stGetDsFlowInfoReq,
                             sizeof(TAF_PS_GET_DSFLOW_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ClearDsFlowInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_DSFLOW_CLEAR_CONFIG_STRU       *pstClearConfigInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CLEAR_DSFLOW_REQ_STRU        stClearDsFlowReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stClearDsFlowReq, sizeof(stClearDsFlowReq), 0x00, sizeof(TAF_PS_CLEAR_DSFLOW_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CLEAR_DSFLOW_REQ消息 */
    stClearDsFlowReq.stCtrl.ulModuleId = ulModuleId;
    stClearDsFlowReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stClearDsFlowReq.stCtrl.ucOpId     = ucOpId;
    stClearDsFlowReq.stClearConfigInfo = *pstClearConfigInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_CLEAR_DSFLOW_REQ,
                             &stClearDsFlowReq,
                             sizeof(TAF_PS_CLEAR_DSFLOW_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ConfigDsFlowRpt(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_DSFLOW_REPORT_CONFIG_STRU      *pstReportConfigInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CONFIG_DSFLOW_RPT_REQ_STRU   stConfigDsFlowRptReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stConfigDsFlowRptReq, sizeof(stConfigDsFlowRptReq), 0x00, sizeof(TAF_PS_CONFIG_DSFLOW_RPT_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CONFIG_DSFLOW_RPT_REQ消息 */
    stConfigDsFlowRptReq.stCtrl.ulModuleId = ulModuleId;
    stConfigDsFlowRptReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stConfigDsFlowRptReq.stCtrl.ucOpId     = ucOpId;
    stConfigDsFlowRptReq.stReportConfigInfo   = *pstReportConfigInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_CONFIG_DSFLOW_RPT_REQ,
                             &stConfigDsFlowRptReq,
                             sizeof(TAF_PS_CONFIG_DSFLOW_RPT_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ConfigVTFlowRpt(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_VTFLOW_REPORT_CONFIG_STRU      *pstReportConfigInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CONFIG_VTFLOW_RPT_REQ_STRU   stConfigVTFlowRptReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stConfigVTFlowRptReq, sizeof(stConfigVTFlowRptReq), 0x00, sizeof(TAF_PS_CONFIG_VTFLOW_RPT_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CONFIG_VTFLOW_RPT_REQ消息 */
    stConfigVTFlowRptReq.stCtrl.ulModuleId  = ulModuleId;
    stConfigVTFlowRptReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stConfigVTFlowRptReq.stCtrl.ucOpId      = ucOpId;
    stConfigVTFlowRptReq.stReportConfigInfo = *pstReportConfigInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_CONFIG_VTFLOW_RPT_REQ,
                             &stConfigVTFlowRptReq,
                             sizeof(TAF_PS_CONFIG_VTFLOW_RPT_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_TrigGprsData(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_GPRS_DATA_STRU                 *pstGprsDataInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_TRIG_GPRS_DATA_REQ_STRU      stTrigGprsDataReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stTrigGprsDataReq, sizeof(stTrigGprsDataReq), 0x00, sizeof(TAF_PS_TRIG_GPRS_DATA_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_TRIG_GPRS_DATA_REQ消息 */
    stTrigGprsDataReq.stCtrl.ulModuleId = ulModuleId;
    stTrigGprsDataReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stTrigGprsDataReq.stCtrl.ucOpId     = ucOpId;
    stTrigGprsDataReq.stGprsDataInfo    = *pstGprsDataInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_TRIG_GPRS_DATA_REQ,
                             &stTrigGprsDataReq,
                             sizeof(TAF_PS_TRIG_GPRS_DATA_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetApDsFlowRptCfg(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_APDSFLOW_RPT_CFG_STRU          *pstRptCfg
)
{
    TAF_PS_SET_APDSFLOW_RPT_CFG_REQ_STRU    stSetRptCfgReq;
    VOS_UINT32                              ulResult;

    TAF_MEM_SET_S(&stSetRptCfgReq, sizeof(stSetRptCfgReq), 0x00, sizeof(TAF_PS_SET_APDSFLOW_RPT_CFG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_APDSFLOW_RPT_CFG_REQ消息 */
    stSetRptCfgReq.stCtrl.ulModuleId = ulModuleId;
    stSetRptCfgReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetRptCfgReq.stCtrl.ucOpId     = ucOpId;

    TAF_MEM_CPY_S(&(stSetRptCfgReq.stRptCfg),
               sizeof(stSetRptCfgReq.stRptCfg),
               pstRptCfg,
               sizeof(TAF_APDSFLOW_RPT_CFG_STRU));

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_SET_APDSFLOW_RPT_CFG_REQ,
                             &stSetRptCfgReq,
                             sizeof(TAF_PS_SET_APDSFLOW_RPT_CFG_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetApDsFlowRptCfg(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_APDSFLOW_RPT_CFG_REQ_STRU    stGetRptCfgReq;
    VOS_UINT32                              ulResult;

    TAF_MEM_SET_S(&stGetRptCfgReq, sizeof(stGetRptCfgReq), 0x00, sizeof(TAF_PS_GET_APDSFLOW_RPT_CFG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_APDSFLOW_RPT_CFG_REQ消息 */
    stGetRptCfgReq.stCtrl.ulModuleId = ulModuleId;
    stGetRptCfgReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetRptCfgReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_APDSFLOW_RPT_CFG_REQ,
                             &stGetRptCfgReq,
                             sizeof(TAF_PS_GET_APDSFLOW_RPT_CFG_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetDsFlowNvWriteCfg(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_DSFLOW_NV_WRITE_CFG_STRU       *pstNvWriteCfg
)
{
    TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ_STRU stSetWriteNvCfgReq;
    VOS_UINT32                              ulResult;

    TAF_MEM_SET_S(&stSetWriteNvCfgReq, sizeof(stSetWriteNvCfgReq), 0x00, sizeof(TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ消息 */
    stSetWriteNvCfgReq.stCtrl.ulModuleId = ulModuleId;
    stSetWriteNvCfgReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetWriteNvCfgReq.stCtrl.ucOpId     = ucOpId;

    TAF_MEM_CPY_S(&(stSetWriteNvCfgReq.stNvWriteCfg),
               sizeof(stSetWriteNvCfgReq.stNvWriteCfg),
               pstNvWriteCfg,
               sizeof(TAF_DSFLOW_NV_WRITE_CFG_STRU));

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ,
                             &stSetWriteNvCfgReq,
                             sizeof(TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDsFlowNvWriteCfg(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_DSFLOW_NV_WRITE_CFG_REQ_STRU stGetNvWriteCfgReq;
    VOS_UINT32                              ulResult;

    TAF_MEM_SET_S(&stGetNvWriteCfgReq, sizeof(stGetNvWriteCfgReq), 0x00, sizeof(TAF_PS_GET_DSFLOW_NV_WRITE_CFG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DSFLOW_WRITE_NV_CFG_REQ消息 */
    stGetNvWriteCfgReq.stCtrl.ulModuleId = ulModuleId;
    stGetNvWriteCfgReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetNvWriteCfgReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_DSFLOW_NV_WRITE_CFG_REQ,
                             &stGetNvWriteCfgReq,
                             sizeof(TAF_PS_GET_DSFLOW_NV_WRITE_CFG_REQ_STRU));

    return ulResult;
}




VOS_UINT32 TAF_PS_SetCdataDialModeInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    TAF_PS_CDATA_DIAL_MODE_ENUM_UINT32  enDialMode
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CDATA_DIAL_MODE_REQ_STRU     stDialModeReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stDialModeReq, sizeof(stDialModeReq), 0x00, sizeof(TAF_PS_CDATA_DIAL_MODE_REQ_STRU));

    /* 构造TAF_PS_CDATA_DIAL_MODE_REQ_STRU消息 */
    stDialModeReq.stCtrl.ulModuleId     = ulModuleId;
    stDialModeReq.stCtrl.usClientId     = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stDialModeReq.stCtrl.ucOpId         = 0;
    stDialModeReq.enDialMode            = enDialMode;

    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_SET_CDMA_DIAL_MODE_REQ,
                             &stDialModeReq,
                             sizeof(TAF_PS_CDATA_DIAL_MODE_REQ_STRU));

    return ulResult;
}




TAF_PS_CDATA_BEAR_STATUS_ENUM_UINT8 TAF_PS_GetCdataBearStatus(
    VOS_UINT8                                               ucPdpId
)
{
    TAF_PS_CDATA_BEAR_STATUS_ENUM_UINT8 ucCdataBearStatus;

    ucCdataBearStatus = TAF_PS_CDATA_BEAR_STATUS_INACTIVE;


    return ucCdataBearStatus;
}


TAF_PS_CDATA_BEAR_STATUS_ENUM_UINT8  TAF_PS_GetPppStatus( VOS_VOID )
{
    VOS_UINT8                           ucPdpId;
    TAF_PS_CDATA_BEAR_STATUS_ENUM_UINT8 enCurrCdataStatus;
    TAF_PS_CDATA_BEAR_STATUS_ENUM_UINT8 enPreCdataStatus;

    enCurrCdataStatus = TAF_PS_CDATA_BEAR_STATUS_INACTIVE;
    enPreCdataStatus = TAF_PS_CDATA_BEAR_STATUS_INACTIVE;

    for (ucPdpId = 0; ucPdpId < TAF_PS_MAX_PDPID; ucPdpId++)
    {
        enCurrCdataStatus = TAF_PS_GetCdataBearStatus(ucPdpId);
        if ((enPreCdataStatus < enCurrCdataStatus)
        && (TAF_PS_CDATA_BEAR_STATUS_BUTT != enCurrCdataStatus))
        {
            enPreCdataStatus = enCurrCdataStatus;
        }

    }
    return enPreCdataStatus;
}





VOS_UINT32 TAF_PS_GetLteCsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId

)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_LTECS_REQ_STRU   stGetLteCsInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetLteCsInfoReq, sizeof(stGetLteCsInfoReq), 0x00, sizeof(TAF_PS_LTECS_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    stGetLteCsInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetLteCsInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetLteCsInfoReq.stCtrl.ucOpId     = ucOpId;


    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_LTE_CS_REQ,
                             &stGetLteCsInfoReq,
                             sizeof(TAF_PS_LTECS_REQ_STRU));


    return ulResult;
}


VOS_UINT32 TAF_PS_GetCemodeInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_CEMODE_REQ_STRU   stGetCemodeInfoReq;



    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetCemodeInfoReq, sizeof(stGetCemodeInfoReq), 0x00, sizeof(TAF_PS_CEMODE_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    stGetCemodeInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetCemodeInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetCemodeInfoReq.stCtrl.ucOpId     = ucOpId;


    /* 发送消息 */
    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_CEMODE_REQ,
                             &stGetCemodeInfoReq,
                             sizeof(TAF_PS_CEMODE_REQ_STRU));


    return ulResult;
}


VOS_UINT32 TAF_PS_Set_DataSwitchStatus(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucDataSwitch
)
{

    TAF_PS_SET_DATA_SWITCH_REQ_STRU      stDataSwitchReq;
    VOS_UINT32                           ulResult;

    TAF_MEM_SET_S(&stDataSwitchReq, sizeof(stDataSwitchReq), 0x00, sizeof(stDataSwitchReq));

    stDataSwitchReq.stCtrl.ulModuleId       = ulModuleId;
    stDataSwitchReq.stCtrl.usClientId       = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stDataSwitchReq.stCtrl.ucOpId           = ucOpId;
    stDataSwitchReq.ucDataSwitch            = ucDataSwitch;

    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_SET_DATA_SWITCH_REQ,
                             &stDataSwitchReq,
                             sizeof(stDataSwitchReq));


    return ulResult;
}


VOS_UINT32 TAF_PS_Set_DataRoamSwitchStatus(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucDataRoamSwitch
)
{

    TAF_PS_SET_DATA_ROAM_SWITCH_REQ_STRU      stDataRoamSwitchReq;
    VOS_UINT32                                ulResult;

    TAF_MEM_SET_S(&stDataRoamSwitchReq, sizeof(stDataRoamSwitchReq), 0x00, sizeof(stDataRoamSwitchReq));

    stDataRoamSwitchReq.stCtrl.ulModuleId       = ulModuleId;
    stDataRoamSwitchReq.stCtrl.usClientId       = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stDataRoamSwitchReq.stCtrl.ucOpId           = ucOpId;
    stDataRoamSwitchReq.ucDataRoamSwitch        = ucDataRoamSwitch;

    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_SET_DATA_ROAM_SWITCH_REQ,
                             &stDataRoamSwitchReq,
                             sizeof(stDataRoamSwitchReq));


    return ulResult;
}

VOS_UINT32 TAF_PS_Get_DataSwitchStatus(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{

    TAF_PS_GET_DATA_SWITCH_REQ_STRU          stGetDataSwitchReq;
    VOS_UINT32                               ulResult;

    TAF_MEM_SET_S(&stGetDataSwitchReq, sizeof(stGetDataSwitchReq), 0x00, sizeof(stGetDataSwitchReq));

    stGetDataSwitchReq.stCtrl.ulModuleId   = ulModuleId;
    stGetDataSwitchReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDataSwitchReq.stCtrl.ucOpId       = ucOpId;

    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_DATA_SWITCH_REQ,
                             &stGetDataSwitchReq,
                             sizeof(stGetDataSwitchReq));


    return ulResult;
}


VOS_UINT32 TAF_PS_Get_DataRoamSwitchStatus(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{

    TAF_PS_GET_DATA_ROAM_SWITCH_REQ_STRU          stGetDataRoamSwitchReq;
    VOS_UINT32                                    ulResult;

    TAF_MEM_SET_S(&stGetDataRoamSwitchReq, sizeof(stGetDataRoamSwitchReq), 0x00, sizeof(stGetDataRoamSwitchReq));

    stGetDataRoamSwitchReq.stCtrl.ulModuleId   = ulModuleId;
    stGetDataRoamSwitchReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDataRoamSwitchReq.stCtrl.ucOpId       = ucOpId;

    ulResult = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                             ID_MSG_TAF_PS_GET_DATA_ROAM_SWITCH_REQ,
                             &stGetDataRoamSwitchReq,
                             sizeof(stGetDataRoamSwitchReq));


    return ulResult;
}


