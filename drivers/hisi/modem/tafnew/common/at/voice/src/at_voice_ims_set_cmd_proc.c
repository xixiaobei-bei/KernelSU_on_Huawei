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
#include "at_voice_ims_set_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "at_voice_comm.h"
#include "at_data_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_IMS_SET_CMD_PROC_C

#define AT_IMSCTRLMSG_PARA_NUM 3
#define AT_IMSCTRLMSG_MSG_ID 0
#define AT_IMSCTRLMSG_MSG_LEN 1
#define AT_IMSCTRLMSG_MSG_LEN_INDEX 2
#define AT_NICKNAME_NICK_NAME 0
#define AT_RTTMODIFY_PARA_NUM 2
#define AT_IMSIPCAPCFG_PARA_NUM 2

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SndImsaImsCtrlMsg(VOS_UINT16 clientId, VOS_UINT8 opId, AT_IMS_CtrlMsg *atImsaMsgPara)
{
    AT_IMSA_ImsCtrlMsg *sndMsg = VOS_NULL_PTR;
    VOS_UINT32          msgLen;
    VOS_UINT32          ret;
    errno_t             memResult;
    ModemIdUint16       modemId = MODEM_ID_0;

    /* 获取client id对应的Modem Id */
    ret = AT_GetModemIdFromClient(clientId, &modemId);

    if (ret == VOS_ERR) {
        AT_ERR_LOG("AT_SndImsaImsCtrlMsg:AT_GetModemIdFromClient is error");
        return VOS_ERR;
    }

#if (FEATURE_MODEM1_SUPPORT_LTE == FEATURE_ON)
    if (modemId == MODEM_ID_2)
#else
    if ((modemId == MODEM_ID_1) || (modemId == MODEM_ID_2))
#endif
    {
        AT_ERR_LOG("AT_SndImsaImsCtrlMsg: enModemId is not support ims");
        return VOS_ERR;
    }

    msgLen = sizeof(AT_IMSA_ImsCtrlMsg) - AT_DATA_DEFAULT_LENGTH + atImsaMsgPara->msgLen;

    /* 申请内存  */
    sndMsg = (AT_IMSA_ImsCtrlMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, msgLen - VOS_MSG_HEAD_LENGTH);

    if (sndMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_SndImsaImsCtrlMsg:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    /* 填写相关参数 */
    TAF_CfgMsgHdr((MsgBlock *)sndMsg, WUEPS_PID_AT, AT_GetDestPid(clientId, PS_PID_IMSA), msgLen - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgId      = ID_AT_IMSA_IMS_CTRL_MSG;
    sndMsg->clientId   = clientId;
    sndMsg->opId       = opId;
    sndMsg->wifiMsgLen = atImsaMsgPara->msgLen;

    if (atImsaMsgPara->msgLen > 0) {
        memResult = memcpy_s(sndMsg->wifiMsg, atImsaMsgPara->msgLen, atImsaMsgPara->msgContext, atImsaMsgPara->msgLen);
        TAF_MEM_CHK_RTN_VAL(memResult, atImsaMsgPara->msgLen, atImsaMsgPara->msgLen);
    }

    /* 调用VOS发送原语 */
    return TAF_TraceAndSendMsg(WUEPS_PID_AT, sndMsg);
}
#endif

VOS_UINT32 AT_PS_ProcMapconMsg(const VOS_UINT16 clientId, const AT_MAPCON_CTRL_MSG_STRU *atMapConMsgPara)
{
    TAF_Ctrl          ctrl;
    TAF_PS_EpdgCtrl   epdgCtrl;
    VOS_UINT_PTR      msgContext;
    VOS_UINT32        msgType;
    errno_t           memResult;

    msgType = TAF_GET_HOST_UINT32(atMapConMsgPara->msgContext);
    msgContext = (VOS_UINT_PTR)atMapConMsgPara->msgContext;
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&epdgCtrl, sizeof(epdgCtrl), 0x00, sizeof(epdgCtrl));

    AT_NORM_LOG1("AT_PS_ProcMapconMsg: Msg Type is ", msgType);

    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, clientId, 0, &ctrl) == VOS_ERR) {
        return VOS_ERR;
    }

    switch (msgType) {
        case ID_WIFI_IMSA_IMS_PDN_ACTIVATE_CNF:

            if (atMapConMsgPara->msgLen != sizeof(TAF_PS_EpdgActCnfInfo)) {
                AT_ERR_LOG("AT_PS_ProcMapconMsg: activate cnf length is error!");
                return VOS_ERR;
            }

            epdgCtrl.opActCnf = VOS_TRUE;

            memResult = memcpy_s(&(epdgCtrl.actCnfInfo), sizeof(epdgCtrl.actCnfInfo), (VOS_UINT8*)msgContext,
                                 sizeof(TAF_PS_EpdgActCnfInfo));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(epdgCtrl.actCnfInfo), sizeof(TAF_PS_EpdgActCnfInfo));
            break;

        case ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_CNF:

            if (atMapConMsgPara->msgLen != sizeof(TAF_PS_EpdgDeactCnfInfo)) {
                AT_ERR_LOG("AT_PS_ProcMapconMsg: deactivate cnf length is error!");
                return VOS_ERR;
            }

            epdgCtrl.opDeActCnf = VOS_TRUE;

            memResult = memcpy_s(&(epdgCtrl.deActCnfInfo), sizeof(epdgCtrl.deActCnfInfo),
                                 (VOS_UINT8*)msgContext, sizeof(TAF_PS_EpdgDeactCnfInfo));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(epdgCtrl.deActCnfInfo), sizeof(TAF_PS_EpdgDeactCnfInfo));
            break;

        case ID_WIFI_IMSA_IMS_PDN_DEACTIVATE_IND:

            if (atMapConMsgPara->msgLen != sizeof(TAF_PS_EpdgDeactIndInfo)) {
                AT_ERR_LOG("AT_PS_ProcMapconMsg: deactivate ind length is error!");
                return VOS_ERR;
            }

            epdgCtrl.opDeActInd = VOS_TRUE;

            memResult = memcpy_s(&(epdgCtrl.deActIndInfo), sizeof(epdgCtrl.deActIndInfo),
                                 (VOS_UINT8*)msgContext, sizeof(TAF_PS_EpdgDeactIndInfo));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(epdgCtrl.deActIndInfo), sizeof(TAF_PS_EpdgDeactIndInfo));
            break;

        default:
            AT_ERR_LOG("AT_PS_ProcMapconMsg: Msg Type is error!");
            return VOS_ERR;
    }

    return TAF_PS_EpdgCtrlMsg(&ctrl, &epdgCtrl);
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetImsDomainCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                     rst;
    TAF_MMA_ImsDomainCfgTypeUint32 imsDomainCfg;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    imsDomainCfg = (TAF_MMA_ImsDomainCfgTypeUint32)g_atParaList[0].paraValue;

    /* 执行命令操作 */
    rst = TAF_MMA_SetImsDomainCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, imsDomainCfg);

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSDOMAINCFG_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

/*
 * 功能描述: 重置CLIENT CTRL TAB
 */
VOS_UINT32 AT_CheckImsctrlmsgPara(VOS_UINT8 indexNum, VOS_UINT16 *msgStrLen)
{
    VOS_UINT32      i;
    VOS_UINT32      indexValue;
    VOS_UINT32      msgSubStrCount;

    if (g_atParaIndex < AT_IMSCTRLMSG_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_IMSCTRLMSG_MSG_ID].paraLen == 0) || (g_atParaList[AT_IMSCTRLMSG_MSG_LEN].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    msgSubStrCount = (VOS_UINT32)g_atParaIndex - AT_DOUBLE_QUO_MARKS_LENGTH;

    for (i = 0; i < msgSubStrCount; i++) {
        indexValue = i + AT_IMSCTRLMSG_MSG_LEN_INDEX;

        if (g_atParaList[indexValue].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        *msgStrLen += g_atParaList[indexValue].paraLen;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetImsctrlmsgPara(VOS_UINT8 indexNum)
{
    AT_IMS_CtrlMsg *atImsaMsgPara = VOS_NULL_PTR;
    VOS_UINT8      *msg           = VOS_NULL_PTR;
    VOS_UINT32      atImsaMsgParaLen;
    VOS_UINT32      msgSubStrCount;
    VOS_UINT32      i;
    VOS_UINT32      indexValue;
    errno_t         memResult;
    VOS_UINT16      msgStrLen = 0;
    VOS_UINT16      length;
    VOS_UINT32      result;

    /* 参数检查 */
    if (AT_CheckImsctrlmsgPara(indexNum, &msgStrLen) != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    msgSubStrCount = (VOS_UINT32)g_atParaIndex - AT_DOUBLE_QUO_MARKS_LENGTH;

    /*lint -save -e516 */
    msg = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, msgStrLen + 1);
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SetImsctrlmsgPara: Memory malloc failed!");
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(msg, msgStrLen + 1, 0x00, msgStrLen + 1);

    length = 0;
    for (i = 0; i < msgSubStrCount; i++) {
        indexValue = i + AT_IMSCTRLMSG_MSG_LEN_INDEX;

        length += (VOS_UINT16)AT_FormatReportString(msgStrLen + 1, (VOS_CHAR *)msg, (VOS_CHAR *)msg + length, "%s",
            g_atParaList[indexValue].para);
    }

    /* 将字符串参数转换为码流 */
    result = At_AsciiNum2HexString(msg, &length);

    if ((result != AT_SUCCESS) || (g_atParaList[AT_IMSCTRLMSG_MSG_LEN].paraValue != length)) {
        PS_MEM_FREE(WUEPS_PID_AT, msg);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 申请内存空间，大小为Mapcon给IMSA发送的消息的长度 */
    atImsaMsgParaLen = sizeof(VOS_UINT32) + g_atParaList[AT_IMSCTRLMSG_MSG_LEN].paraValue;
    atImsaMsgPara    = (AT_IMS_CtrlMsg *)PS_MEM_ALLOC(WUEPS_PID_AT, atImsaMsgParaLen);

    if (atImsaMsgPara == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SetImsctrlmsgPara: Memory malloc failed!");
        PS_MEM_FREE(WUEPS_PID_AT, msg);

        return AT_ERROR;
    }

    (VOS_VOID)memset_s(atImsaMsgPara, atImsaMsgParaLen, 0x00, atImsaMsgParaLen);

    /* 填充AT_IMSA_ImsCtrlMsg */
    atImsaMsgPara->msgLen = g_atParaList[AT_IMSCTRLMSG_MSG_LEN].paraValue;
    if (atImsaMsgPara->msgLen > 0) {
        memResult = memcpy_s(atImsaMsgPara->msgContext, atImsaMsgPara->msgLen, msg, atImsaMsgPara->msgLen);
        TAF_MEM_CHK_RTN_VAL(memResult, atImsaMsgPara->msgLen, atImsaMsgPara->msgLen);
    }

    /* 转发MAPCON给IMSA的消息 */
    if (g_atParaList[AT_IMSCTRLMSG_MSG_ID].paraValue == 0) {
        result = AT_SndImsaImsCtrlMsg(g_atClientTab[indexNum].clientId, 0, atImsaMsgPara);
    } else {
        result = AT_PS_ProcMapconMsg(g_atClientTab[indexNum].clientId, (AT_MAPCON_CTRL_MSG_STRU *)atImsaMsgPara);
    }

    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, msg);
    PS_MEM_FREE(WUEPS_PID_AT, atImsaMsgPara);
    /*lint -restore */
    if (result != VOS_OK) {
        return AT_ERROR;
    }

    return AT_OK;
}

TAF_UINT32 AT_SetVolteimpuPara(TAF_UINT8 indexNum)
{
    AT_IMSA_VolteimpuQryReq impu;
    VOS_UINT32              result;

    (VOS_VOID)memset_s(&impu, sizeof(impu), 0x00, sizeof(AT_IMSA_VolteimpuQryReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送^VOLTEIMPU设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_VOLTEIMPU_QRY_REQ, impu.content,
                                    sizeof(impu.content), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMPU_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetNickNamePara(TAF_UINT8 indexNum)
{
    AT_IMSA_NicknameSetReq nickname;
    VOS_UINT32             result;
    errno_t                memResult;
    VOS_UINT16             maxNickNameLen;

    (VOS_VOID)memset_s(&nickname, sizeof(nickname), 0x00, sizeof(nickname));
    maxNickNameLen = 2 * (MN_CALL_DISPLAY_NAME_STRING_SZ - 1); /* UTF8编码格式，最大长度为256个字符 */

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNickNamePara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetNickNamePara: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_NICKNAME_NICK_NAME].paraLen == 0) ||
        (maxNickNameLen < g_atParaList[AT_NICKNAME_NICK_NAME].paraLen) ||
        /* 取值只能为偶数 */
        (g_atParaList[AT_NICKNAME_NICK_NAME].paraLen % 2 != 0)) {
        AT_WARN_LOG("AT_SetNickNamePara: para len is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_NICKNAME_NICK_NAME].para,
        &g_atParaList[AT_NICKNAME_NICK_NAME].paraLen) != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetNickNamePara: At_AsciiNum2HexString is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    nickname.nickName.nickNameLen = (VOS_UINT8)g_atParaList[AT_NICKNAME_NICK_NAME].paraLen;
    if (nickname.nickName.nickNameLen > 0) {
        memResult = memcpy_s(nickname.nickName.nickName, nickname.nickName.nickNameLen,
                             g_atParaList[AT_NICKNAME_NICK_NAME].para, g_atParaList[AT_NICKNAME_NICK_NAME].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, nickname.nickName.nickNameLen, g_atParaList[AT_NICKNAME_NICK_NAME].paraLen);
    }
    nickname.nickName.nickName[nickname.nickName.nickNameLen] = '\0';

    /* 给IMSA发送^NICKNAME设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_NICKNAME_SET_REQ,
                                    (VOS_UINT8 *)&(nickname.nickName), (VOS_UINT32)sizeof(IMSA_AT_NicknameInfo),
                                    PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetNickNamePara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NICKNAME_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetRttCfgPara(VOS_UINT8 indexNum)
{
    AT_IMSA_RttcfgSetReq atRttCfgSetReq;
    VOS_UINT32           result = 0;

    (VOS_VOID)memset_s(&atRttCfgSetReq, sizeof(atRttCfgSetReq), 0x00, sizeof(atRttCfgSetReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetRttCfgPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetRttCfgPara: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    atRttCfgSetReq.rttCfg = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 给IMSA发送^RTTCFG设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_RTTCFG_SET_REQ, &(atRttCfgSetReq.rttCfg),
                                    (VOS_UINT32)sizeof(atRttCfgSetReq.rttCfg), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetRttCfgPara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RTTCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetRttModifyPara(VOS_UINT8 indexNum)
{
    VOS_UINT32              result = 0;
    AT_IMSA_RttModifySetReq atRttModifySetReq;

    (VOS_VOID)memset_s(&atRttModifySetReq, sizeof(atRttModifySetReq), 0x00, sizeof(atRttModifySetReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetRttModifyPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != AT_RTTMODIFY_PARA_NUM) {
        AT_WARN_LOG("AT_SetRttModifyPara: para num is not equal 2!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    atRttModifySetReq.rttModify.callId    = (VOS_UINT8)g_atParaList[0].paraValue;
    atRttModifySetReq.rttModify.operation = (VOS_UINT8)g_atParaList[1].paraValue;

    /* 给IMSA发送^RTTMODIFY设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_RTT_MODIFY_SET_REQ, (VOS_UINT8 *)&(atRttModifySetReq.rttModify),
                                    (VOS_UINT32)sizeof(atRttModifySetReq.rttModify), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetRttModifyPara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RTTMODIFY_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 AT_SetImsIpCapPara(VOS_UINT8 indexNum)
{
    AT_IMSA_ImsIpCapSetInfo imsIpCapSetInfo;
    VOS_UINT32              result;

    (VOS_VOID)memset_s(&imsIpCapSetInfo, sizeof(imsIpCapSetInfo), 0x00, sizeof(imsIpCapSetInfo));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetImsIpCapPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != AT_IMSIPCAPCFG_PARA_NUM) {
        AT_WARN_LOG("AT_SetImsIpCapPara: para num is not equal 2!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        imsIpCapSetInfo.opIpsecFlag = VOS_TRUE;
        imsIpCapSetInfo.ipsecFlag   = g_atParaList[0].paraValue;
    }

    if (g_atParaList[1].paraLen != 0) {
        imsIpCapSetInfo.opKeepAliveFlag = VOS_TRUE;
        imsIpCapSetInfo.keepAliveFlag   = g_atParaList[1].paraValue;
    }

    /* 给IMSA发送^IMSIPCAPCFG设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_IMS_IP_CAP_SET_REQ, (VOS_UINT8 *)&(imsIpCapSetInfo),
                                    (VOS_SIZE_T)sizeof(AT_IMSA_ImsIpCapSetInfo), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetImsIpCapPara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSIPCAPCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}

