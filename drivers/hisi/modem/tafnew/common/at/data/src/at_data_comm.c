/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "at_data_comm.h"
#include "at_data_proc.h"
#include "at_data_event_report.h"
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_COMM_C

const AT_ChdataRnicRmnetId g_atChdataRnicRmNetIdTab[] = {
    { AT_CH_DATA_CHANNEL_ID_0, RNIC_DEV_ID_BUTT, PS_IFACE_ID_NDIS0, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_1, RNIC_DEV_ID_RMNET0, PS_IFACE_ID_RMNET0, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_2, RNIC_DEV_ID_RMNET1, PS_IFACE_ID_RMNET1, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_3, RNIC_DEV_ID_RMNET2, PS_IFACE_ID_RMNET2, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_4, RNIC_DEV_ID_RMNET3, PS_IFACE_ID_RMNET3, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_5, RNIC_DEV_ID_RMNET4, PS_IFACE_ID_RMNET4, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_6, RNIC_DEV_ID_RMNET5, PS_IFACE_ID_RMNET5, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_7, RNIC_DEV_ID_RMNET6, PS_IFACE_ID_RMNET6, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_8, RNIC_DEV_ID_RMNET7, PS_IFACE_ID_RMNET7, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_9, RNIC_DEV_ID_RMNET8, PS_IFACE_ID_RMNET8, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_10, RNIC_DEV_ID_RMNET9, PS_IFACE_ID_RMNET9, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_11, RNIC_DEV_ID_RMNET10, PS_IFACE_ID_RMNET10, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_12, RNIC_DEV_ID_RMNET11, PS_IFACE_ID_RMNET11, { 0, 0 }},
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    { AT_CH_DATA_CHANNEL_ID_13, RNIC_DEV_ID_BUTT, PS_IFACE_ID_NDIS1, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_14, RNIC_DEV_ID_BUTT, PS_IFACE_ID_NDIS2, { 0, 0 }},
    { AT_CH_DATA_CHANNEL_ID_15, RNIC_DEV_ID_BUTT, PS_IFACE_ID_NDIS3, { 0, 0 }}
#endif
};

#if (FEATURE_IMS == FEATURE_ON)
VOS_VOID AT_ReportImsEmcStatResult(VOS_UINT8 indexNum, AT_PDP_StatusUint32 status)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^IMSEMCSTAT:%d%s", g_atCrLf, status, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}
#endif

VOS_UINT32 AT_PS_GetChDataValueFromRnicRmNetId(RNIC_DEV_ID_ENUM_UINT8   rnicRmNetId,
                                               AT_CH_DataChannelUint32 *dataChannelId)
{
    VOS_UINT32 i;
    CONST AT_ChdataRnicRmnetId *chdataRnicRmNetIdTab;

    chdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE(); i++) {
        if (rnicRmNetId == chdataRnicRmNetIdTab[i].rnicRmNetId) {
            *dataChannelId = chdataRnicRmNetIdTab[i].chdataValue;
            break;
        }
    }

    if (i >= AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE()) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_PS_GetChDataValueFromIfaceId(PS_IFACE_IdUint8 ifaceId, AT_CH_DataChannelUint32 *dataChannelId)
{
    VOS_UINT32 i;
    CONST AT_ChdataRnicRmnetId *chdataRnicRmNetIdTab;

    chdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE(); i++) {
        if (ifaceId == chdataRnicRmNetIdTab[i].ifaceId) {
            *dataChannelId = chdataRnicRmNetIdTab[i].chdataValue;
            break;
        }
    }

    if (i >= AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE()) {
        return VOS_ERR;
    }

    return VOS_OK;
}

CONST AT_ChdataRnicRmnetId* AT_PS_GetChDataCfgByChannelId(AT_CH_DataChannelUint32 dataChannelId)
{
    CONST AT_ChdataRnicRmnetId *chdataRnicRmNetIdTab = VOS_NULL_PTR;
    CONST AT_ChdataRnicRmnetId *chDataConfig         = VOS_NULL_PTR;
    VOS_UINT32                  i;

    chdataRnicRmNetIdTab = AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_PTR();

    /*  以上判断已能保证enDataChannelId的有效性，所以RM NET ID一定能在表中找到 */
    for (i = 0; i < AT_PS_GET_CHDATA_RNIC_RMNET_ID_TBL_SIZE(); i++) {
        if (dataChannelId == chdataRnicRmNetIdTab[i].chdataValue) {
            chDataConfig = &chdataRnicRmNetIdTab[i];
            break;
        }
    }

    return chDataConfig;
}

VOS_UINT32 At_TestCgeqnegPara(VOS_UINT8 indexNum)
{
    /*
     * CGEQNEG的测试命令和CGACT的查询命令的功能类似,都要获取当前各CID的激活
     * 情况,向TAF查询当前各CID的激活情况
     */
    if (TAF_PS_GetPdpContextState(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGEQNEG_TEST;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */

    /* 接下来到TAF事件上报的地方,获取当前哪些CID被激活,并打印当前哪些CID被激活. */
}

VOS_VOID AT_PS_DeleteIpAddrMap(VOS_UINT16 clientId, TAF_PS_CallPdpDeactivateCnf *event)
{
    AT_ModemPsCtx *psModemCtx = VOS_NULL_PTR;

    psModemCtx = AT_GetModemPsCtxAddrFromClientId(clientId);

#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
    if (event->ifaceId >= PS_IFACE_ID_BUTT) {
        return;
    }

    if ((event->primFlag == VOS_TRUE) && ((event->pdpType & TAF_PDP_IPV4) == TAF_PDP_IPV4)) {
        psModemCtx->ipAddrIfaceIdMap[event->ifaceId] = 0;
    }
#else
    if (!AT_PS_IS_RABID_VALID(event->rabId)) {
        return;
    }

    psModemCtx->ipAddrRabIdMap[event->rabId - AT_PS_RABID_OFFSET] = 0;
#endif
}

VOS_UINT32 AT_ChkTafPsEvtGetPrimPdpContextInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetPrimPdpContextInfoCnf *pdpContxt = VOS_NULL_PTR;
    const TAF_PS_Evt                      *event     = VOS_NULL_PTR;
    VOS_UINT32                             length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetPrimPdpContextInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPrimPdpContextInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event     = (const TAF_PS_Evt *)msg;
    pdpContxt = (const TAF_PS_GetPrimPdpContextInfoCnf *)(event->content);
    length   += (sizeof(TAF_PRI_PdpQueryInfo) * pdpContxt->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPrimPdpContextInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetSecPdpContextInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetSecPdpContextInfoCnf *pdpContxt = VOS_NULL_PTR;
    const TAF_PS_Evt                     *event     = VOS_NULL_PTR;
    VOS_UINT32                            length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetSecPdpContextInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetSecPdpContextInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event     = (const TAF_PS_Evt *)msg;
    pdpContxt = (const TAF_PS_GetSecPdpContextInfoCnf *)(event->content);
    length   += (sizeof(TAF_PDP_SecContext) * pdpContxt->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetSecPdpContextInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetTftInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetTftInfoCnf *pdpContxt = VOS_NULL_PTR;
    const TAF_PS_Evt           *event     = VOS_NULL_PTR;
    VOS_UINT32                  length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetTftInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetTftInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event     = (const TAF_PS_Evt *)msg;
    pdpContxt = (const TAF_PS_GetTftInfoCnf *)(event->content);
    length   += (sizeof(TAF_TFT_QureyInfo) * pdpContxt->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetTftInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetUmtsQosInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetUmtsQosInfoCnf *qosInfo = VOS_NULL_PTR;
    const TAF_PS_Evt               *event   = VOS_NULL_PTR;
    VOS_UINT32                      length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetUmtsQosInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetUmtsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    qosInfo = (const TAF_PS_GetUmtsQosInfoCnf *)(event->content);
    length += (sizeof(TAF_UMTS_QosQueryInfo) * qosInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetUmtsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetUmtsQosMinInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetUmtsQosMinInfoCnf *qosInfo = VOS_NULL_PTR;
    const TAF_PS_Evt                  *event   = VOS_NULL_PTR;
    VOS_UINT32                         length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetUmtsQosMinInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetUmtsQosMinInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    qosInfo = (const TAF_PS_GetUmtsQosMinInfoCnf *)(event->content);
    length += (sizeof(TAF_UMTS_QosQueryInfo) * qosInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetUmtsQosMinInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetDynamicUmtsQosInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetDynamicUmtsQosInfoCnf *qosInfo = VOS_NULL_PTR;
    const TAF_PS_Evt                      *event   = VOS_NULL_PTR;
    VOS_UINT32                             length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDynamicUmtsQosInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicUmtsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    qosInfo = (const TAF_PS_GetDynamicUmtsQosInfoCnf *)(event->content);
    length += (sizeof(TAF_UMTS_QosQueryInfo) * qosInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicUmtsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetPdpStateCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetPdpStateCnf *pdpState = VOS_NULL_PTR;
    const TAF_PS_Evt            *event    = VOS_NULL_PTR;
    VOS_UINT32                   length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetPdpStateCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPdpStateCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event    = (const TAF_PS_Evt *)msg;
    pdpState = (const TAF_PS_GetPdpStateCnf *)(event->content);
    length  += sizeof(TAF_CID_State) * pdpState->cidNum;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPdpStateCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetPdpIpAddrInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetPdpIpAddrInfoCnf *ipAddrInfo = VOS_NULL_PTR;
    const TAF_PS_Evt                 *event      = VOS_NULL_PTR;
    VOS_UINT32                        length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetPdpIpAddrInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPdpIpAddrInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event      = (const TAF_PS_Evt *)msg;
    ipAddrInfo = (const TAF_PS_GetPdpIpAddrInfoCnf *)(event->content);
    length    += (sizeof(TAF_PDP_AddrQueryInfo) * ipAddrInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPdpIpAddrInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetDynamicPrimPdpContextInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_Evt                             *event = VOS_NULL_PTR;
    const TAF_PS_GetDynamicPrimPdpContextInfoCnf *pdpContxt = VOS_NULL_PTR;
    VOS_UINT32                                    length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDynamicPrimPdpContextInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicPrimPdpContextInfoCnfMsgLen: msgLen, structLen.",
            VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event     = (const TAF_PS_Evt *)msg;
    pdpContxt = (const TAF_PS_GetDynamicPrimPdpContextInfoCnf *)(event->content);
    length   += (sizeof(TAF_PDP_DynamicPrimExt) * pdpContxt->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicPrimPdpContextInfoCnfMsgLen: msgLen, structLen.",
            VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetDynamicSecPdpContextInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetDynamicSecPdpContextInfoCnf *pdpContxt = VOS_NULL_PTR;
    const TAF_PS_Evt                            *event = VOS_NULL_PTR;
    VOS_UINT32                                   length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDynamicSecPdpContextInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicSecPdpContextInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg),
            length);
        return VOS_FALSE;
    }

    event     = (const TAF_PS_Evt *)msg;
    pdpContxt = (const TAF_PS_GetDynamicSecPdpContextInfoCnf *)(event->content);
    length   += (sizeof(TAF_PDP_DynamicSecExt) * pdpContxt->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicSecPdpContextInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg),
            length);
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetDynamicTftInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetDynamicTftInfoCnf *tftInfo = VOS_NULL_PTR;
    const TAF_PS_Evt                  *event = VOS_NULL_PTR;
    VOS_UINT32                         length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDynamicTftInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicTftInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    tftInfo = (const TAF_PS_GetDynamicTftInfoCnf *)(event->content);
    length += (sizeof(TAF_PF_Tft) * tftInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicTftInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetEpsQosInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetEpsQosInfoCnf *qosInfo = VOS_NULL_PTR;
    const TAF_PS_Evt              *event   = VOS_NULL_PTR;
    VOS_UINT32                     length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetEpsQosInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetEpsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    qosInfo = (const TAF_PS_GetEpsQosInfoCnf *)(event->content);
    length += (sizeof(TAF_EPS_QosExt) * qosInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetEpsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetDynamicEpsQosInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_Evt                     *event = VOS_NULL_PTR;
    const TAF_PS_GetDynamicEpsQosInfoCnf *epsQos = VOS_NULL_PTR;
    VOS_UINT32                            length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetDynamicEpsQosInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicEpsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event   = (const TAF_PS_Evt *)msg;
    epsQos  = (const TAF_PS_GetDynamicEpsQosInfoCnf *)(event->content);
    length += (sizeof(TAF_EPS_QosExt) * epsQos->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetDynamicEpsQosInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetPdpDnsInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetPdpDnsInfoCnf *pdpDnsInfo = VOS_NULL_PTR;
    const TAF_PS_Evt              *event      = VOS_NULL_PTR;
    VOS_UINT32                     length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetPdpDnsInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPdpDnsInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event      = (const TAF_PS_Evt *)msg;
    pdpDnsInfo = (const TAF_PS_GetPdpDnsInfoCnf *)(event->content);
    length    += (sizeof(TAF_DNS_QueryInfo) * pdpDnsInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetPdpDnsInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ChkTafPsEvtGetAuthDataInfoCnfMsgLen(const MSG_Header *msg)
{
    const TAF_PS_GetAuthdataInfoCnf *authInfo = VOS_NULL_PTR;
    const TAF_PS_Evt                *event    = VOS_NULL_PTR;
    VOS_UINT32                       length;

    length = sizeof(TAF_PS_Evt) + sizeof(TAF_PS_GetAuthdataInfoCnf) - TAF_DEFAULT_CONTENT_LEN;
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetAuthDataInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    event    = (const TAF_PS_Evt*)msg;
    authInfo = (const TAF_PS_GetAuthdataInfoCnf *)(event->content);
    length  += (sizeof(TAF_AUTHDATA_QueryInfo) * authInfo->cidNum);
    if ((VOS_GET_MSG_LEN(msg) + VOS_MSG_HEAD_LENGTH) < length) {
        AT_ERR_LOG2("AT_ChkTafPsEvtGetAuthDataInfoCnfMsgLen: msgLen, structLen.", VOS_GET_MSG_LEN(msg), length);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_Get3gppSmCauseByPsCause(TAF_PS_CauseUint32 cause)
{
    VOS_UINT32 ul3gppSmCause;

    if ((cause >= TAF_PS_CAUSE_SM_NW_SECTION_BEGIN) && (cause <= TAF_PS_CAUSE_SM_NW_SECTION_END)) {
        ul3gppSmCause = cause - TAF_PS_CAUSE_SM_NW_SECTION_BEGIN;
    }
    /* E5、闪电卡在PDP DEACTIVE时上报网侧的36号原因值 */
    else if (cause == TAF_PS_CAUSE_SUCCESS) {
#if (FEATURE_MBB_CUST == FEATURE_ON)
        ul3gppSmCause = AT_NDISSTAT_ERR_UNKNOWN;
#else
        ul3gppSmCause = TAF_PS_CAUSE_SM_NW_REGULAR_DEACTIVATION - TAF_PS_CAUSE_SM_NW_SECTION_BEGIN;
#endif
    } else {
        ul3gppSmCause = AT_NDISSTAT_ERR_UNKNOWN;
    }

    return ul3gppSmCause;
}

VOS_VOID AT_PS_SetPsCallErrCause(VOS_UINT16 clientId, TAF_PS_CauseUint32 psErrCause)
{
    AT_ModemPsCtx *psModemCtx = VOS_NULL_PTR;

    psModemCtx = AT_GetModemPsCtxAddrFromClientId(clientId);

    psModemCtx->psErrCause = psErrCause;
}

TAF_PS_CauseUint32 AT_PS_GetPsCallErrCause(VOS_UINT16 clientId)
{
    AT_ModemPsCtx *psModemCtx = VOS_NULL_PTR;

    psModemCtx = AT_GetModemPsCtxAddrFromClientId(clientId);

    return psModemCtx->psErrCause;
}

TAF_IFACE_UserTypeUint8 AT_PS_GetUserType(const VOS_UINT8 indexNum)
{
    TAF_IFACE_UserTypeUint8 userType = TAF_IFACE_USER_TYPE_BUTT;
    VOS_UINT8              *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    switch (g_atClientTab[indexNum].userType) {
        case AT_NDIS_USER:
            userType = TAF_IFACE_USER_TYPE_NDIS;
            break;

        case AT_APP_USER:
            userType = TAF_IFACE_USER_TYPE_APP;
            break;

        case AT_USBCOM_USER:
        case AT_UART_USER:
#if (FEATURE_IOT_CMUX == FEATURE_ON)
        case AT_CMUXAT_USER:
        case AT_CMUXMDM_USER:
        case AT_CMUXEXT_USER:
        case AT_CMUXGPS_USER:
#endif
            if ((AT_GetPcuiPsCallFlag() == VOS_TRUE) || (*systemAppConfig == SYSTEM_APP_WEBUI)) {
                userType = TAF_IFACE_USER_TYPE_APP;
                break;
            }

            userType = TAF_IFACE_USER_TYPE_NDIS;
            break;

        case AT_CTR_USER:
            if (AT_GetCtrlPsCallFlag() == VOS_TRUE) {
                userType = TAF_IFACE_USER_TYPE_APP;
                break;
            }

            if (*systemAppConfig != SYSTEM_APP_WEBUI) {
                userType = TAF_IFACE_USER_TYPE_NDIS;
                break;
            }
            break;

        case AT_PCUI2_USER:
            if (AT_GetPcui2PsCallFlag() == VOS_TRUE) {
                userType = TAF_IFACE_USER_TYPE_APP;
                break;
            }

            if (*systemAppConfig != SYSTEM_APP_WEBUI) {
                userType = TAF_IFACE_USER_TYPE_NDIS;
                break;
            }
            break;

        default:
            AT_WARN_LOG("AT_PS_GetUserType: UserType Is Invalid!");
            userType = TAF_IFACE_USER_TYPE_BUTT;
            break;
    }

    return userType;
}

#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
VOS_UINT32 AT_PS_CheckIfaceMatchWithClient(VOS_UINT16 clientId, VOS_UINT8 ifaceId, VOS_UINT32 usedFlg)
{
    TAF_IFACE_UserTypeUint8 userType;

    userType = AT_PS_GetUserType((VOS_UINT8)clientId);

    /* 映射关系未设置场景默认匹配 */
    if (usedFlg == VOS_FALSE) {
        return VOS_TRUE;
    }

    if (userType == TAF_IFACE_USER_TYPE_NDIS) {
        if (AT_IsNdisIface(ifaceId) == VOS_TRUE) {
            return VOS_TRUE;
        }
    } else if (userType == TAF_IFACE_USER_TYPE_APP) {
        if ((ifaceId >= PS_IFACE_ID_RMNET0) && (ifaceId <= PS_IFACE_ID_RMNET11)) {
            return VOS_TRUE;
        }
    } else {
        /* 其他端口 默认匹配 */
        return VOS_TRUE;
    }
    return VOS_FALSE;
}
#endif

AT_PS_DataChanlCfg* AT_PS_GetDataChanlCfg(VOS_UINT16 clientId, VOS_UINT8 cid)
{
    AT_CommPsCtx *psCtx = VOS_NULL_PTR;
    VOS_UINT32    rslt;
    VOS_UINT32    i;
    ModemIdUint16 modemId = MODEM_ID_0;

    psCtx   = AT_GetCommPsCtxAddr();
    rslt    = AT_GetModemIdFromClient(clientId, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_PS_GetDataChanlCfg: Get modem id fail.");
        return &(psCtx->channelCfg[AT_CH_DATA_CHANNEL_BUTT]);
    }

    for (i = 0; i < AT_CH_DATA_CHANNEL_BUTT; i++) {
        if ((psCtx->channelCfg[i].cid == cid) && (psCtx->channelCfg[i].modemId == modemId)) {
            /* 共享APN场景下存在单个cid 对应多个通道 需要进行通道类型判断 */
#if ((FEATURE_SHARE_APN == FEATURE_ON) && (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON))
            if (AT_PS_CheckIfaceMatchWithClient(clientId, (VOS_UINT8)(psCtx->channelCfg[i].ifaceId),
                    psCtx->channelCfg[i].used) == VOS_FALSE) {
                continue;
            }
#endif
            return &(psCtx->channelCfg[i]);
        }
    }

    return &(psCtx->channelCfg[AT_CH_DATA_CHANNEL_BUTT]);
}

