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
#include "AtCmdCallProc.h"

#include "AtSndMsg.h"
#include "ATCmdProc.h"
#include "TafCcmApi.h"
#include "AtTafAgentInterface.h"
#include "AppVcApi.h"
#include "securec.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_CALL_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/
#if (FEATURE_ECALL == FEATURE_ON)

VOS_UINT32 AT_SetCecallPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ORIG_PARAM_STRU             stCallOrigPara;
    VOS_UINT32                          ulRst;
    MODEM_ID_ENUM_UINT16                enModemId;

    (VOS_VOID)memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    (VOS_VOID)memset_s(&stCallOrigPara, sizeof(stCallOrigPara), 0x00, sizeof(stCallOrigPara));

    /* 参数必须为1。
       参数个数为1时，参数长度不可能为0，
       即"AT+CECALL="情况下，gucAtParaIndex为0 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    switch (gastAtParaList[0].ulParaValue)
    {
        case 0:
            stCallOrigPara.enCallType = MN_CALL_TYPE_TEST;
            break;
        case 1:
            stCallOrigPara.enCallType = MN_CALL_TYPE_RECFGURATION;
            break;
        case 2:
            stCallOrigPara.enCallType = MN_CALL_TYPE_MIEC;
            break;
        case 3:
            stCallOrigPara.enCallType = MN_CALL_TYPE_AIEC;
            break;
        default:
            return AT_ERROR;
    }

    stCtrl.ulModuleId                    = WUEPS_PID_AT;
    stCtrl.usClientId                    = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                        = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stCallOrigPara,
                                  ID_TAF_CCM_CALL_ORIG_REQ,
                                  sizeof(stCallOrigPara),
                                  enModemId);

    if (ulRst == VOS_OK)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CECALL_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEclstartPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_ORIG_PARAM_STRU             stCallOrigPara;
    MN_CALL_CALLED_NUM_STRU             stDialNumber;
    VOS_UINT32                          ulRst;
    MODEM_ID_ENUM_UINT16                enModemId;
    APP_VC_SET_OPRTMODE_REQ_STRU        stEclOprtModepara;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stCallOrigPara, sizeof(stCallOrigPara), 0x00, sizeof(stCallOrigPara));

    /*  判断1: 必选参数的长度不能为0 */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        return AT_ERROR;
    }

    /* 判断2: 参数个数不对, 判断1已保证参数个数>=2 */
    if (gucAtParaIndex > 4)
    {
        return AT_ERROR;
    }

    /* 判断3:  AT^ECLSTART=1,1, 这种情况是错的 */
    if ((gucAtParaIndex == 3)
     && (gastAtParaList[2].usParaLen == 0))
    {
        return AT_ERROR;
    }

    /* 判断4:  AT^ECLSTART=1,1,, 这种情况是错的 */
    if ((gucAtParaIndex == 4)
        && (gastAtParaList[3].usParaLen == 0))
    {
        return AT_ERROR;
    }

    /* 判断5:  当前ecall通话中 */
    if (AT_HaveEcallActive(ucIndex, VOS_FALSE) == VOS_TRUE)
    {
        return AT_ERROR;
    }
    /* 初始化 */
    memset_s(&stEclOprtModepara, sizeof(stEclOprtModepara), 0x00, sizeof(stEclOprtModepara));

    /* oprt_mode 默认为PUSH模式 */
    if ((gastAtParaList[3].usParaLen == 0)
        || (gastAtParaList[3].ulParaValue == APP_VC_ECALL_OPRT_PUSH))
    {
        stEclOprtModepara.enEcallOpMode = APP_VC_ECALL_OPRT_PUSH;
    }
    else
    {
        stEclOprtModepara.enEcallOpMode = APP_VC_ECALL_OPRT_PULL;
    }

    /* 发送VC模块保存当前请求模式 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_SET_ECALL_OPRTMODE_REQ,
                                   &stEclOprtModepara,
                                   sizeof(stEclOprtModepara),
                                   I0_WUEPS_PID_VC);

    if (ulRst == TAF_FAILURE)
    {
        return AT_ERROR;
    }


    /* 初始化 */
    memset_s(&stDialNumber, sizeof(stDialNumber), 0x00, sizeof(stDialNumber));

    /* 带拨号号码 */
    if (gastAtParaList[2].usParaLen > 0)
    {

        /* 检查并转换电话号码 */
        if (AT_FillCalledNumPara(gastAtParaList[2].aucPara,
                                            gastAtParaList[2].usParaLen,
                                            &stDialNumber) != VOS_OK)
        {
            AT_UpdateCallErrInfo(ucIndex, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
            return AT_ERROR;
        }
    }

    /* ^ECLSTART=x,0  发起测试call */
    if (gastAtParaList[1].ulParaValue == 0)
    {
        stCallOrigPara.enCallType = MN_CALL_TYPE_TEST;

        /* 只有测试呼叫才下发电话号码，紧急呼叫不下发电话号码 */
        memcpy_s(&stCallOrigPara.stDialNumber, sizeof(stCallOrigPara.stDialNumber), &stDialNumber, sizeof(stDialNumber));
    }
    else if (gastAtParaList[1].ulParaValue == 1)
    {
        /* ^ECLSTART=0,1  用户发起紧急call */
        if (gastAtParaList[0].ulParaValue == 0)
        {
            stCallOrigPara.enCallType = MN_CALL_TYPE_MIEC;
        }
        /* ^ECLSTART=1,1  自动发起紧急call */
        else
        {
            stCallOrigPara.enCallType = MN_CALL_TYPE_AIEC;
        }
    }
	else
	{
        stCallOrigPara.enCallType = MN_CALL_TYPE_RECFGURATION;

        /* 只有测试呼叫才下发电话号码，紧急呼叫不下发电话号码 */
        memcpy_s(&stCallOrigPara.stDialNumber, sizeof(stCallOrigPara.stDialNumber), &stDialNumber, sizeof(stDialNumber));
	}

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stCallOrigPara,
                                  ID_TAF_CCM_CALL_ORIG_REQ,
                                  sizeof(stCallOrigPara),
                                  enModemId);

    if (ulRst == VOS_OK)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLSTART_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetEclstopPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_SUPS_PARAM_STRU             stCallSupsPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    /* 不带参数的设置命令 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_ERROR;
    }

    /* 初始化 */
    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stCallSupsPara, sizeof(stCallSupsPara), 0x00, sizeof(stCallSupsPara));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    stCallSupsPara.enCallSupsCmd = MN_CALL_SUPS_CMD_REL_ECALL;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stCallSupsPara,
                                  ID_TAF_CCM_CALL_SUPS_CMD_REQ,
                                  sizeof(stCallSupsPara),
                                  enModemId);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLSTOP_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetEclcfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    APP_VC_MSG_SET_ECALL_CFG_REQ_STRU   stEclcfgSetPara;

    /* 判断一: 必选参数的长度不能为0 */
    if (gastAtParaList[0].usParaLen == 0)
    {
        return AT_ERROR;
    }

    /* 判断二: 参数个数不对, 判断一已经保证参数个数>=1 */
    if (gucAtParaIndex > 3)
    {
        return AT_ERROR;
    }

    /* 判断3:  AT^ECLCFG=0, 或者 AT^ECLCFG=0,1, 这种情况是错的 */
    if (((gucAtParaIndex == 2)
         && (gastAtParaList[1].usParaLen == 0))
        || ((gucAtParaIndex == 3)
            && (gastAtParaList[2].usParaLen == 0)))
    {
        return AT_ERROR;
    }

    /* 初始化 */
    memset_s(&stEclcfgSetPara, sizeof(stEclcfgSetPara), 0x00, sizeof(stEclcfgSetPara));

    stEclcfgSetPara.enMode = (APP_VC_ECALL_MSD_MODE_ENUM_UINT16)gastAtParaList[0].ulParaValue;

    if (gastAtParaList[1].usParaLen == 0)
    {
        stEclcfgSetPara.enVocConfig = VOC_CONFIG_NO_CHANGE;
    }
    else
    {
        stEclcfgSetPara.enVocConfig = (APP_VC_ECALL_VOC_CONFIG_ENUM_UINT16)gastAtParaList[1].ulParaValue;
    }

    /* 取消ECALL重拨功能，保留第三个参数, 目前只能下发关闭命令 */
    if ((gastAtParaList[2].usParaLen != 0) && (gastAtParaList[2].ulParaValue == 1))
    {
        return AT_ERROR;
    }

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_SET_ECALL_CFG_REQ,
                                   &stEclcfgSetPara,
                                   sizeof(stEclcfgSetPara),
                                   I0_WUEPS_PID_VC);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetEclmsdPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    APP_VC_MSG_SET_MSD_REQ_STRU         stEclmsdPara;

    /* 参数必须为1。
       参数个数为1时，参数长度不可能为0，
       即"AT+CELMSD="情况下，gucAtParaIndex为0 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    /* 参数长度不对
       十六进制文本字符串，采用十六进制数据编码方式，字符串长度为280个字节，表示的是MSD协议要求的140个字节原始数据
    */
    if (((APP_VC_MSD_DATA_LEN * 2) != gastAtParaList[0].usParaLen))
    {
        return AT_ERROR;
    }

    if (At_AsciiString2HexSimple(stEclmsdPara.aucMsdData,
                                               gastAtParaList[0].aucPara,
                                               APP_VC_MSD_DATA_LEN * 2) == AT_FAILURE)
    {
        return AT_ERROR;
    }

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_SET_MSD_REQ,
                                   &stEclmsdPara,
                                   sizeof(stEclmsdPara),
                                   I0_WUEPS_PID_VC);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLMSD_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryCecallPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    TAF_CTRL_STRU                       stCtrl;
    MODEM_ID_ENUM_UINT16                enModemId;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* 发消息到C核获取当前所有通话信息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  VOS_NULL_PTR,
                                  ID_TAF_CCM_QRY_ECALL_INFO_REQ,
                                  0,
                                  enModemId);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CECALL_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryEclcfgPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_QRY_ECALL_CFG_REQ,
                                   VOS_NULL,
                                   0,
                                   I0_WUEPS_PID_VC);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLCFG_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_QryEclmsdPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_QRY_MSD_REQ,
                                   VOS_NULL,
                                   0,
                                   I0_WUEPS_PID_VC);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLMSD_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestEclstartPara(VOS_UINT8 ucIndex)
{

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: (0,1),(0,1,2),(0,1)",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    return AT_OK;
}


VOS_UINT32 AT_TestEclmsdPara(VOS_UINT8 ucIndex)
{
    /* ^eclmsd不支持测试命令，
        该函数可以在eclmsd测试命令时返回ERROR，而不是"(MSD)" */
    return AT_ERROR;
}

VOS_UINT8 AT_HaveEcallActive(VOS_UINT8 ucIndex, VOS_UINT8 ucCheckFlag)
{
    VOS_UINT8                           ucNumOfCalls = 0;
    VOS_UINT8                           i            = 0;
    VOS_UINT32                          ulRst;
    TAFAGENT_CALL_INFO_PARAM_STRU       astCallInfos[MN_CALL_MAX_NUM];

    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_ERR_LOG("AT_HaveEcallActive : ucIndex.");

        return VOS_FALSE;
    }
    memset_s(astCallInfos, sizeof(astCallInfos), 0x00, sizeof(astCallInfos));

    /* 从同步API获取通话信息 */
   ulRst = TAF_AGENT_GetCallInfoReq(gastAtClientTab[ucIndex].usClientId,
                                     &ucNumOfCalls,
                                     astCallInfos);

    if (ulRst != VOS_OK)
    {
        AT_ERR_LOG("AT_HaveEcallActive : TAF_AGENT_GetCallInfoReq.");

        return VOS_FALSE;
    }

    AT_NORM_LOG1("AT_HaveEcallActive : [ucCheckFlag]", ucCheckFlag);

    /* 当前有ECALL 通话返回TRUE */
    for (i = 0; i < ucNumOfCalls; i++)
    {
        if ((astCallInfos[i].enCallType == MN_CALL_TYPE_MIEC)
            || (astCallInfos[i].enCallType == MN_CALL_TYPE_AIEC)
            || (astCallInfos[i].enCallType == MN_CALL_TYPE_PSAP_ECALL)
            || (astCallInfos[i].enCallType == MN_CALL_TYPE_RECFGURATION)
            || (astCallInfos[i].enCallType == MN_CALL_TYPE_EMERGENCY)
            || (astCallInfos[i].enCallType == MN_CALL_TYPE_TEST))
        {
            if (ucCheckFlag == VOS_TRUE)
            {
                if (astCallInfos[i].enCallState == MN_CALL_S_ACTIVE)
                {
                    return VOS_TRUE;
                }
            }
            else
            {
                return VOS_TRUE;
            }
        }
    }
    return VOS_FALSE;
}


VOS_UINT32 AT_SetEclpushPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    APP_VC_MSG_SET_MSD_REQ_STRU         stEclmsdPara;

    /* 不带参数的设置命令 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_ERROR;
    }

    /* 当前非Ecall激活状态返回ERROR */
    if (AT_HaveEcallActive(ucIndex, VOS_TRUE) == VOS_FALSE)
    {
        return AT_ERROR;
    }

    memset_s(&stEclmsdPara, sizeof(APP_VC_MSG_SET_MSD_REQ_STRU), 0x00, sizeof(APP_VC_MSG_SET_MSD_REQ_STRU));
    /* 往VC发送APP_VC_MSG_ECALL_PUSH_REQ命令 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_SET_ECALL_PUSH_REQ,
                                   &stEclmsdPara,
                                   sizeof(stEclmsdPara),
                                   I0_WUEPS_PID_VC);

    if (ulRst == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ECLPUSH_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetEclAbortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulAbortReason = 0;

    /* 不带参数的设置命令 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)
    {
        return AT_ERROR;
    }

    /* 当前非Ecall激活状态返回ERROR */
    if (AT_HaveEcallActive(ucIndex, VOS_TRUE) == VOS_FALSE)
    {
        return AT_ERROR;
    }

    /* 往VC发送APP_VC_MSG_ECALL_ABORT_REQ命令 */
    ulRet = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   APP_VC_MSG_SET_ECALL_ABORT_REQ,
                                   &ulAbortReason,
                                   sizeof(ulAbortReason),
                                   I0_WUEPS_PID_VC);

    if (ulRet == TAF_SUCCESS)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ABORT_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEclModePara(VOS_UINT8 ucIndex)
{
    TAF_NVIM_CUSTOM_ECALL_CFG_STRU      stEcallCfg;
    VOS_UINT32                          ulResult;

    memset_s(&stEcallCfg, sizeof(stEcallCfg), 0x00, sizeof(stEcallCfg));

    /* 参数检查 */
    if ( g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD )
    {
        AT_ERR_LOG("AT_SetEclModePara: ucCmdOptType Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (gucAtParaIndex != 1)
    {
        AT_ERR_LOG("AT_SetEclModePara: num Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <mode>的值只支持设置为0-2 */
    if (gastAtParaList[0].ulParaValue > 2)
    {
        AT_ERR_LOG("AT_SetEclModePara: value Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*先读取NV的值*/
    ulResult = TAF_ACORE_NV_READ(MODEM_ID_0,
                                 en_NV_Item_Custom_eCall_Cfg,
                                 &stEcallCfg,
                                 sizeof(stEcallCfg));

    /*NV读取失败或NV未激活时，返回error*/
    if (ulResult != NV_OK)
    {
        AT_ERR_LOG("AT_SetEclModePara: NV read!");
        return AT_ERROR;
    }

    if (stEcallCfg.ucEcallForceMode != gastAtParaList[0].ulParaValue)
    {
        stEcallCfg.ucEcallForceMode = (VOS_UINT8) gastAtParaList[0].ulParaValue;
        /*写入NV的值*/
        ulResult = TAF_ACORE_NV_WRITE(MODEM_ID_0,
                                      en_NV_Item_Custom_eCall_Cfg,
                                      &stEcallCfg,
                                      sizeof(stEcallCfg));

        if (ulResult != NV_OK)
        {
            return AT_ERROR;
        }

        AT_SetEclModeValue(stEcallCfg.ucEcallForceMode);
    }

    return AT_OK;
}

VOS_UINT32 AT_QryEclModePara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    TAF_NVIM_CUSTOM_ECALL_CFG_STRU      stEcallCfg;
    VOS_UINT32                          ulResult;

    memset_s(&stEcallCfg, sizeof(stEcallCfg), 0x00, sizeof(stEcallCfg));
    /*先读取NV的值*/
    ulResult = TAF_ACORE_NV_READ(MODEM_ID_0,
                                 en_NV_Item_Custom_eCall_Cfg,
                                 &stEcallCfg,
                                 sizeof(stEcallCfg));

    /*NV读取失败或NV未激活时，不改变模式*/
    if (ulResult != NV_OK)
    {
        AT_ERR_LOG("AT_QryEclModePara: NV read error!");
    }
    else
    {
        AT_SetEclModeValue(stEcallCfg.ucEcallForceMode);
    }

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      "%s: %d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_GetEclModeValue());
    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}

VOS_UINT32 AT_TestEclModePara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                             (VOS_CHAR*)pgucAtSndCodeAddr,
                             (VOS_CHAR*)pgucAtSndCodeAddr,
                             "%s: %s",
                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                             g_stParseContext[ucIndex].pstCmdElement->pszParam);
    return AT_OK;
}


VOS_VOID AT_EcallAlackDisplay(
    AT_ECALL_ALACK_VALUE_STRU           stEcallAlackInfo,
    VOS_UINT16*                         usLength
)
{
    TAF_INT8                            cTimeZone   = 0;
    TIME_ZONE_TIME_STRU*                pstTimeInfo = VOS_NULL_PTR;

    pstTimeInfo = &stEcallAlackInfo.stEcallAlackTimeInfo.stUniversalTimeandLocalTimeZone;
    /* YYYY */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        "\"%4d/",
                                        pstTimeInfo->ucYear + 2000); /*year*/

    /* MM */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        "%d%d/",
                                        pstTimeInfo->ucMonth / 10,/* month high */
                                        pstTimeInfo->ucMonth % 10);/* month low */
    /* dd */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        "%d%d,",
                                        pstTimeInfo->ucDay / 10,/* day high */
                                        pstTimeInfo->ucDay % 10);/* day high */

    /* hh */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        "%d%d:",
                                        pstTimeInfo->ucHour / 10,/* hour high */
                                        pstTimeInfo->ucHour % 10);/* hour high */

    /* mm */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        "%d%d:",
                                        pstTimeInfo->ucMinute / 10,/* minutes high */
                                        pstTimeInfo->ucMinute % 10);/* minutes high */

    /* ss */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        "%d%d",
                                        pstTimeInfo->ucSecond / 10,/* sec high */
                                        pstTimeInfo->ucSecond % 10);/* sec high */

    /* 获得时区 */
    if ((stEcallAlackInfo.stEcallAlackTimeInfo.ucIeFlg & NAS_MM_INFO_IE_LTZ) == NAS_MM_INFO_IE_LTZ)
    {
        cTimeZone = stEcallAlackInfo.stEcallAlackTimeInfo.cLocalTimeZone;
    }
    else
    {
        cTimeZone = pstTimeInfo->cTimeZone;
    }

    if (cTimeZone == AT_INVALID_TZ_VALUE)
    {
        cTimeZone = 0;
    }

    if (cTimeZone != AT_INVALID_TZ_VALUE)
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                            "%s%02d\"",
                                            (cTimeZone < 0) ? "-" : "+",
                                            (cTimeZone < 0) ? ( - cTimeZone) : cTimeZone);
    }

    /* AlackValue */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + *usLength,
                                        ",%d",
                                        stEcallAlackInfo.ucEcallAlackValue);
    return;
}


VOS_VOID AT_EcallAlAckListDisplay(VOS_VOID)
{
    VOS_UINT32                          i = 0;
    VOS_UINT16                          usLength = 0;
    VOS_UINT32                          ulReadNum = 0;
    AT_ECALL_ALACK_INFO_STRU*           EcallAlackInfoAddr = VOS_NULL_PTR;

    EcallAlackInfoAddr = AT_EcallAlAckInfoAddr();

    /* 显示所有ALACK 值和接受时间 */
    for (i = 0; i <  (VOS_UINT32)AT_MIN(EcallAlackInfoAddr->ucEcallAlackNum, AT_ECALL_ALACK_NUM); i++)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR*)pgucAtSndCodeAddr,
                                           (TAF_CHAR*)pgucAtSndCodeAddr +
                                           usLength,
                                           "^ECLLIST: ");

        ulReadNum = (EcallAlackInfoAddr->ucEcallAlackBeginNum + i) % AT_ECALL_ALACK_NUM;
        AT_EcallAlackDisplay(EcallAlackInfoAddr->stEcallAlackInfo[ulReadNum], &usLength);

        if (i + 1 < EcallAlackInfoAddr->ucEcallAlackNum)
        {
            /* 回车换行 */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s", gaucAtCrLf);
        }
    }

    gstAtSendData.usBufLen = usLength;
    return;
}


VOS_UINT32 AT_QryEclListPara(VOS_UINT8 ucIndex)
{
    AT_EcallAlAckListDisplay();

    return AT_OK;
}


VOS_UINT32 AT_RcvVcMsgEcallPushCnfProc(MN_AT_IND_EVT_STRU* pstData)
{
    VOS_UINT8                           ucIndex = 0;
    VOS_UINT32                          ulRet   = VOS_ERR;
    APP_VC_SET_MSD_CNF_STRU*            pstRslt = VOS_NULL_PTR;

    if (pstData == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(pstData->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvVcMsgEcallPushCnfProc:WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvVcMsgEcallPushCnfProc : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ECLPUSH_SET)
    {
        AT_WARN_LOG("AT_RcvVcMsgEcallPushCnfProc:WARNING:AT ARE WAITING ANOTHER CMD!");

        return VOS_ERR;
    }

    pstRslt = (APP_VC_SET_MSD_CNF_STRU*)pstData->aucContent;

    if (pstRslt->ucRslt == VOS_OK)
    {
        ulRet = AT_OK;
    }
    else
    {
        ulRet = AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    At_FormatResultData(ucIndex, ulRet);

    return VOS_OK;
}


VOS_UINT32 AT_ProcVcEcallAbortCnf(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU*             pstVcEvtInfo
)
{
    VOS_UINT32                          ulRslt = AT_ERROR;

    /* 参数有效性判断 */
    if (pstVcEvtInfo == VOS_NULL)
    {
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_ProcVcEcallAbortCnf : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_ABORT_SET)
    {
        return VOS_ERR;
    }

    if (pstVcEvtInfo->bSuccess != VOS_TRUE)
    {
        ulRslt = AT_ERROR;
    }
    else
    {
        ulRslt = AT_OK;
    }

    AT_STOP_TIMER_CMD_READY(ucIndex);

    At_FormatResultData(ucIndex, ulRslt);

    return VOS_OK;
}


VOS_UINT32 AT_ProcVcReportEcallAlackEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU*             pstVcEvtInfo
)
{
    NAS_MM_INFO_IND_STRU                stLocalAtTimeInfo;
    MODEM_ID_ENUM_UINT16                enModemId             = MODEM_ID_0;
    AT_ECALL_ALACK_INFO_STRU*           pstEcallAlackInfoAddr = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;
    VOS_UINT32                          ulWriteNum            = 0;
    VOS_UINT16                          usLength              = 0;
    AT_MODEM_NET_CTX_STRU*              pstNetCtx             = VOS_NULL_PTR;
    NAS_MM_INFO_IND_STRU                stCCLKTimeInfo;
    VOS_UINT8                           ucEcallAlacknum       = 0;

    if (pstVcEvtInfo == VOS_NULL_PTR)
    {
        return VOS_ERR;
    }

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (ulRslt != VOS_OK)
    {
        AT_ERR_LOG("AT_ProcVcReportEcallAlackEvent: Get modem id fail.");

        return VOS_ERR;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);

    memset_s(&stCCLKTimeInfo,  sizeof(stCCLKTimeInfo), 0x00, sizeof(stCCLKTimeInfo));
    memset_s(&stLocalAtTimeInfo, sizeof(stCCLKTimeInfo), 0x00, sizeof(stLocalAtTimeInfo));

    if ((pstNetCtx->stTimeInfo.ucIeFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ)
    {
        /*首先根据网络或者CCLK的时间转换为localAT时间*/
        AT_GetLiveTime(&pstNetCtx->stTimeInfo, &stLocalAtTimeInfo, pstNetCtx->ulNwSecond);
    }
    else
    {
        memcpy_s(&stCCLKTimeInfo, sizeof(stCCLKTimeInfo), &pstNetCtx->stTimeInfo, sizeof(NAS_MM_INFO_IND_STRU));
        stCCLKTimeInfo.ucIeFlg = NAS_MM_INFO_IE_UTLTZ;
        AT_GetLiveTime(&stCCLKTimeInfo, &stLocalAtTimeInfo, 1);
    }

    /* 存储时间和ALACK 值 */
    pstEcallAlackInfoAddr = AT_EcallAlAckInfoAddr();

    if (pstEcallAlackInfoAddr->ucEcallAlackNum < AT_ECALL_ALACK_NUM)
    {
        pstEcallAlackInfoAddr->ucEcallAlackNum++;
        ulWriteNum = pstEcallAlackInfoAddr->ucEcallAlackNum - 1;
    }
    else
    {
        ucEcallAlacknum = ++pstEcallAlackInfoAddr->ucEcallAlackBeginNum;
        pstEcallAlackInfoAddr->ucEcallAlackBeginNum = ucEcallAlacknum % AT_ECALL_ALACK_NUM;
        ulWriteNum = pstEcallAlackInfoAddr->ucEcallAlackBeginNum - 1;
    }

    memcpy_s(&pstEcallAlackInfoAddr->stEcallAlackInfo[ulWriteNum].stEcallAlackTimeInfo,
             sizeof(NAS_MM_INFO_IND_STRU),
             &stLocalAtTimeInfo,
             sizeof(stLocalAtTimeInfo));

    pstEcallAlackInfoAddr->stEcallAlackInfo[ulWriteNum].ucEcallAlackValue = pstVcEvtInfo->ucEcallReportAlackValue;

    /* 显示时间和ALACK 值 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s^ECLREC: ",
                                       gaucAtCrLf);

    AT_EcallAlackDisplay(pstEcallAlackInfoAddr->stEcallAlackInfo[ulWriteNum], &usLength);

    /* 回车换行 */
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%s", gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    At_SendResultData(AT_CLIENT_TAB_CTRL_INDEX, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_VOID AT_RcvTafEcallStatusErrorInd(VOS_VOID)
{
    VOS_UINT16                          usLength;

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      (VOS_CHAR*)pgucAtSndCodeAddr,
                                      "%s^ECLSTAT: 2,3%s",
                                      gaucAtCrLf,
                                      gaucAtCrLf);

    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, pgucAtSndCodeAddr, usLength);

    return;
}
#endif


VOS_UINT32 At_RcvVcMsgDtmfDecoderIndProc(
    MN_AT_IND_EVT_STRU                 *pstData
)
{
    APP_VC_DTMF_DECODER_IND_STRU       *pstDtmfInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_CHAR                            aucOutput[2];

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(pstData->clientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("At_RcvVcMsgDtmfDecoderIndProc:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 初始化 */
    pstDtmfInd = (APP_VC_DTMF_DECODER_IND_STRU *)pstData->aucContent;
    aucOutput[0] = pstDtmfInd->ucDtmfCode;
    aucOutput[1] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DDTMF: %s%s",
                                                    gaucAtCrLf,
                                                    aucOutput,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 AT_CheckCfshNumber(
    VOS_UINT8                          *pucAtPara,
    VOS_UINT16                          usLen
)
{
    VOS_UINT16      ucLoop;

    /* 号码长度有效性判断:+号开头的国际号码，最大长度不能大于33；否则不能大于32 */
    if (pucAtPara[0] == '+')
    {
        if (usLen > (TAF_CALL_MAX_FLASH_DIGIT_LEN + 1))
        {
            return VOS_ERR;
        }

        pucAtPara++;
        usLen--;
    }
    else
    {
        if (usLen > TAF_CALL_MAX_FLASH_DIGIT_LEN)
        {
            return VOS_ERR;
        }
    }

    /* 号码字符有效性判断(不包含国际号码的首字符'+') */
    for (ucLoop = 0; ucLoop < usLen; ucLoop++)
    {
        if (  ((pucAtPara[ucLoop] >= '0') && (pucAtPara[ucLoop] <= '9'))
            || (pucAtPara[ucLoop] == '*')
            || (pucAtPara[ucLoop] == '#'))
        {
            continue;
        }
        else
        {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetCfshPara(VOS_UINT8 ucIndex)
{
    errno_t                             lMemResult;
    VOS_UINT32                          ulRst;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_FLASH_PARA_STRU            stFlashPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stFlashPara, sizeof(stFlashPara), 0x00, sizeof(stFlashPara));

    /* 参数过多 */
    if(gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 若携带了参数<number>，检查其有效性 */
    if (gucAtParaIndex == 1)
    {
        if (AT_CheckCfshNumber(gastAtParaList[0].aucPara,
                                         gastAtParaList[0].usParaLen) != VOS_OK)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        /* 这种AT命令AT^CFSH= 返回参数错误 */
        if(g_stATParseCmd.ucCmdOptType == AT_CMD_OPT_SET_PARA_CMD)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    memset_s(&stFlashPara, sizeof(stFlashPara), 0x00, sizeof(TAF_CALL_FLASH_PARA_STRU));

    stFlashPara.ucDigitNum = (VOS_UINT8)gastAtParaList[0].usParaLen;
    lMemResult = memcpy_s(stFlashPara.aucDigit, sizeof(stFlashPara.aucDigit), gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stFlashPara.aucDigit), gastAtParaList[0].usParaLen);

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_FLASH_REQ消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stFlashPara,
                                  ID_TAF_CCM_SEND_FLASH_REQ,
                                  sizeof(stFlashPara),
                                  enModemId);

    if (ulRst == VOS_OK)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CFSH_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_RcvTafCcmSndFlashRslt(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_SEND_FLASH_CNF_STRU        *pstSndFlashRslt = VOS_NULL_PTR;

    pstSndFlashRslt = (TAF_CCM_SEND_FLASH_CNF_STRU*)pMsg;

    /* 根据ClientID获取通道索引 */
    if(At_ClientIdToUserId(pstSndFlashRslt->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallSndFlashRslt: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT模块在等待^CFSH命令的操作结果事件上报 */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CFSH_SET)
    {
        AT_WARN_LOG("AT_RcvTafCallSndFlashRslt: Error Option!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 根据临时响应的错误码打印命令的结果 */
    if (pstSndFlashRslt->stResult.ucResult == VOS_OK)
    {
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return VOS_OK;
}


VOS_UINT32 At_TestCBurstDTMFPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16      usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CBURSTDTMF: (1,2),(0-9,*,#),(95,150,200,250,300,350),(60,100,150,200)");
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetCBurstDTMFPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    errno_t                             lMemResult;
    VOS_UINT16                          ucLoop;

    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_BURST_DTMF_PARA_STRU       stBurstDTMFPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stBurstDTMFPara, sizeof(stBurstDTMFPara), 0x00, sizeof(stBurstDTMFPara));

    /*参数有效性检查*/
    if(g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((gucAtParaIndex != 4)
        ||(gastAtParaList[0].usParaLen == 0)
        ||(gastAtParaList[1].usParaLen == 0)
        ||(gastAtParaList[2].usParaLen == 0)
        ||(gastAtParaList[3].usParaLen == 0))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key长度有效性判断 */
    if (gastAtParaList[1].usParaLen > TAF_CALL_MAX_BURST_DTMF_NUM)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key有效性判断 */
    for (ucLoop = 0; ucLoop < gastAtParaList[1].usParaLen; ucLoop++)
    {
        if (  ((gastAtParaList[1].aucPara[ucLoop] >= '0') && (gastAtParaList[1].aucPara[ucLoop] <= '9'))
            || (gastAtParaList[1].aucPara[ucLoop] == '*')
            || (gastAtParaList[1].aucPara[ucLoop] == '#'))
        {
            continue;
        }
        else
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }


    stBurstDTMFPara.ucCallId     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stBurstDTMFPara.ucDigitNum   = (VOS_UINT8)gastAtParaList[1].usParaLen;
    lMemResult = memcpy_s(stBurstDTMFPara.aucDigit, sizeof(stBurstDTMFPara.aucDigit), gastAtParaList[1].aucPara, stBurstDTMFPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(stBurstDTMFPara.aucDigit), stBurstDTMFPara.ucDigitNum);
    stBurstDTMFPara.ulOnLength   = gastAtParaList[2].ulParaValue;
    stBurstDTMFPara.ulOffLength  = gastAtParaList[3].ulParaValue;

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_BURST_DTMF_REQ消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stBurstDTMFPara,
                                  ID_TAF_CCM_SEND_BURST_DTMF_REQ,
                                  sizeof(stBurstDTMFPara),
                                  enModemId);

    if (ulRst == VOS_OK)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CBURSTDTMF_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_RcvTafCcmSndBurstDTMFCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_SEND_BURST_DTMF_CNF_STRU   *pstBurstDtmfCnf = VOS_NULL_PTR;

    pstBurstDtmfCnf = (TAF_CCM_SEND_BURST_DTMF_CNF_STRU*)pMsg;

    /* 根据ClientID获取通道索引 */
    if(At_ClientIdToUserId(pstBurstDtmfCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallSndBurstDTMFCnf: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT模块在等待^CBURSTDTMF命令命令的操作结果事件上报 */
    if ( gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CBURSTDTMF_SET )
    {
        AT_WARN_LOG("AT_RcvTafCallSndBurstDTMFCnf: Error Option!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 根据临时响应的错误码打印命令的结果 */
    if (pstBurstDtmfCnf->stBurstDtmfCnfPara.enResult != TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_SUCCESS)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSndBurstDTMFRslt(VOS_VOID *pMsg)
{
    return VOS_OK;
}



VOS_UINT32 AT_RcvTafCcmCalledNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALLED_NUM_INFO_IND_STRU   *pstCalledNum = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                             lMemResult;

    pstCalledNum = (TAF_CCM_CALLED_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstCalledNum->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallCalledNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }


    /* 初始化 */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstCalledNum->stCalledNumInfoPara.aucDigit, pstCalledNum->stCalledNumInfoPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstCalledNum->stCalledNumInfoPara.ucDigitNum);

    /* 在pstCalledNum->aucDigit的最后一位加'\0',防止因pstCalledNum->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstCalledNum->stCalledNumInfoPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCALLEDNUM].pucText,
                                                    pstCalledNum->stCalledNumInfoPara.enNumType,
                                                    pstCalledNum->stCalledNumInfoPara.enNumPlan,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCallingNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CALLING_NUM_INFO_IND_STRU                      *pstCallingNum = VOS_NULL_PTR;
    VOS_UINT8                                               ucIndex;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                                                 lMemResult;

    pstCallingNum = (TAF_CCM_CALLING_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstCallingNum->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallCallingNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));

    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstCallingNum->stCallIngNumInfoPara.aucDigit, pstCallingNum->stCallIngNumInfoPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstCallingNum->stCallIngNumInfoPara.ucDigitNum);

    /* 在pstCallingNum->aucDigit的最后一位加'\0',防止因pstCallingNum->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstCallingNum->stCallIngNumInfoPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCALLINGNUM].pucText,
                                                    pstCallingNum->stCallIngNumInfoPara.enNumType,
                                                    pstCallingNum->stCallIngNumInfoPara.enNumPlan,
                                                    pstCallingNum->stCallIngNumInfoPara.ucPi,
                                                    pstCallingNum->stCallIngNumInfoPara.ucSi,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmDispInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_DISPLAY_INFO_IND_STRU      *pstDisplayInfo = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM + 1];
    errno_t                             lMemResult;

    pstDisplayInfo = (TAF_CCM_DISPLAY_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstDisplayInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmDispInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstDisplayInfo->stDisPlayInfoIndPara.aucDigit, pstDisplayInfo->stDisPlayInfoIndPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstDisplayInfo->stDisPlayInfoIndPara.ucDigitNum);

    /* 在pstDisplayInfo->aucDigit的最后一位加'\0',防止因pstDisplayInfo->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstDisplayInfo->stDisPlayInfoIndPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%s,,,%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CDISP].pucText,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmExtDispInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_EXT_DISPLAY_INFO_IND_STRU                      *pstExtDispInfo = VOS_NULL_PTR;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM + 1];
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulDigitNum;
    errno_t                                                 lMemResult;
    VOS_UINT8                                               ucIndex;

    pstExtDispInfo = (TAF_CCM_EXT_DISPLAY_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstExtDispInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallExtDispInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    for (ulLoop = 0; ulLoop < pstExtDispInfo->stDisPlayInfoIndPara.ucInfoRecsDataNum; ulLoop++)
    {
        /* 在pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit的最后一位加'\0',
            防止因pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit无结束符，导致AT多上报 */
        ulDigitNum = pstExtDispInfo->stDisPlayInfoIndPara.aucInfoRecsData[ulLoop].ucDigitNum;
        memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
        lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstExtDispInfo->stDisPlayInfoIndPara.aucInfoRecsData[ulLoop].aucDigit, ulDigitNum);
        TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), ulDigitNum);
        aucDigit[ulDigitNum] = '\0';

        /* 输出查询结果 */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s%s%s,%d,%d,%d%s",
                                                        gaucAtCrLf,
                                                        gastAtStringTab[AT_STRING_CDISP].pucText,
                                                        aucDigit,
                                                        pstExtDispInfo->stDisPlayInfoIndPara.ucExtDispInd,
                                                        pstExtDispInfo->stDisPlayInfoIndPara.ucDisplayType,
                                                        pstExtDispInfo->stDisPlayInfoIndPara.aucInfoRecsData[ulLoop].ucDispalyTag,
                                                        gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }



    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmConnNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_CONN_NUM_INFO_IND_STRU     *pstConnNumInfo = VOS_NULL_PTR;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                             lMemResult;
    VOS_UINT8                           ucIndex;

    pstConnNumInfo = (TAF_CCM_CONN_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstConnNumInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmConnNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstConnNumInfo->stConnNumInfoIndPara.aucDigit, pstConnNumInfo->stConnNumInfoIndPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstConnNumInfo->stConnNumInfoIndPara.ucDigitNum);

    /* 在pstConnNumInfo->aucDigit的最后一位加'\0',防止因pstConnNumInfo->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstConnNumInfo->stConnNumInfoIndPara.ucDigitNum] = '\0';

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCONNNUM].pucText,
                                                    pstConnNumInfo->stConnNumInfoIndPara.enNumType,
                                                    pstConnNumInfo->stConnNumInfoIndPara.enNumPlan,
                                                    pstConnNumInfo->stConnNumInfoIndPara.ucPi,
                                                    pstConnNumInfo->stConnNumInfoIndPara.ucSi,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmRedirNumInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_REDIR_NUM_INFO_IND_STRU    *pstRedirNumInfo = VOS_NULL_PTR;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    usLength        = 0;
    pstRedirNumInfo = (TAF_CCM_REDIR_NUM_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstRedirNumInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmRedirNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstRedirNumInfo->stRedirNumInfoIndPara.aucDigitNum, pstRedirNumInfo->stRedirNumInfoIndPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstRedirNumInfo->stRedirNumInfoIndPara.ucDigitNum);

    /* 在pstRedirNumInfo->aucDigitNum的最后一位加'\0',防止因pstRedirNumInfo->aucDigitNum无结束符，导致AT多上报 */
    aucDigit[pstRedirNumInfo->stRedirNumInfoIndPara.ucDigitNum] = '\0';

    /* 输出查询结果，根据EXTENSIONBIT1，EXTENSIONBIT2输出可选项 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d,%d,%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_CREDIRNUM].pucText,
                                       pstRedirNumInfo->stRedirNumInfoIndPara.enNumType,
                                       pstRedirNumInfo->stRedirNumInfoIndPara.enNumPlan,
                                       aucDigit);

    if (pstRedirNumInfo->stRedirNumInfoIndPara.bitOpPi == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->stRedirNumInfoIndPara.ucPi);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (pstRedirNumInfo->stRedirNumInfoIndPara.bitOpSi == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->stRedirNumInfoIndPara.ucSi);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (pstRedirNumInfo->stRedirNumInfoIndPara.bitOpRedirReason == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->stRedirNumInfoIndPara.ucRedirReason);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSignalInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_SIGNAL_INFO_IND_STRU        *pstsignalInfo = VOS_NULL_PTR;
    VOS_UINT8                            ucIndex;

    pstsignalInfo = (TAF_CCM_SIGNAL_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstsignalInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmSignalInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CSIGTONE].pucText,
                                                    pstsignalInfo->stSignalInfoIndPara.ucSignalType,
                                                    pstsignalInfo->stSignalInfoIndPara.ucAlertPitch,
                                                    pstsignalInfo->stSignalInfoIndPara.ucSignal,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmLineCtrlInfoInd(VOS_VOID *pMsg)
{
    TAF_CCM_LINE_CTRL_INFO_IND_STRU                        *pstLineCtrlInfo = VOS_NULL_PTR;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               ucIndex;

    usLength = 0;
    pstLineCtrlInfo = (TAF_CCM_LINE_CTRL_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstLineCtrlInfo->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCcmLineCtrlInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 输出查询结果 */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                    (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%s%s%d",
                                    gaucAtCrLf,
                                    gastAtStringTab[AT_STRING_CLCTR].pucText,
                                    pstLineCtrlInfo->stLineCtrlInfoIndPara.ucPolarityIncluded);

    if (pstLineCtrlInfo->stLineCtrlInfoIndPara.ucToggleModePresent == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstLineCtrlInfo->stLineCtrlInfoIndPara.ucToggleMode);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (pstLineCtrlInfo->stLineCtrlInfoIndPara.ucReversePolarityPresent == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstLineCtrlInfo->stLineCtrlInfoIndPara.ucReversePolarity);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d%s",
                                       pstLineCtrlInfo->stLineCtrlInfoIndPara.ucPowerDenialTime,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmCCWACInd(VOS_VOID *pMsg)
{
    TAF_CCM_CCWAC_INFO_IND_STRU        *pstCCWAC = VOS_NULL_PTR;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                             lMemResult;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    usLength = 0;
    pstCCWAC = (TAF_CCM_CCWAC_INFO_IND_STRU*)pMsg;

    /* 根据clientId获取通道索引 */
    if(At_ClientIdToUserId(pstCCWAC->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallCCWACInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* 初始化 */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstCCWAC->stCcwacInfoPara.aucDigit, pstCCWAC->stCcwacInfoPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstCCWAC->stCcwacInfoPara.ucDigitNum);

    /* 在pstCCWAC->aucDigit的最后一位加'\0',防止因pstCCWAC->aucDigit无结束符，导致AT多上报 */
    aucDigit[pstCCWAC->stCcwacInfoPara.ucDigitNum] = '\0';


    /* 输出查询结果 */
    if (pstCCWAC->stCcwacInfoPara.ucSignalIsPresent == VOS_TRUE)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%s%s,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                            gaucAtCrLf,
                                            gastAtStringTab[AT_STRING_CCWAC].pucText,
                                            aucDigit,
                                            pstCCWAC->stCcwacInfoPara.ucPi,
                                            pstCCWAC->stCcwacInfoPara.ucSi,
                                            pstCCWAC->stCcwacInfoPara.enNumType,
                                            pstCCWAC->stCcwacInfoPara.enNumPlan,
                                            pstCCWAC->stCcwacInfoPara.ucSignalIsPresent,
                                            pstCCWAC->stCcwacInfoPara.ucSignalType,
                                            pstCCWAC->stCcwacInfoPara.ucAlertPitch,
                                            pstCCWAC->stCcwacInfoPara.ucSignal,
                                            gaucAtCrLf);

    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%s%s,%d,%d,%d,%d,%d,,,%s",
                                            gaucAtCrLf,
                                            gastAtStringTab[AT_STRING_CCWAC].pucText,
                                            aucDigit,
                                            pstCCWAC->stCcwacInfoPara.ucPi,
                                            pstCCWAC->stCcwacInfoPara.ucSi,
                                            pstCCWAC->stCcwacInfoPara.enNumType,
                                            pstCCWAC->stCcwacInfoPara.enNumPlan,
                                            pstCCWAC->stCcwacInfoPara.ucSignalIsPresent,
                                            gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 At_TestCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CCONTDTMF: (1,2),(0,1),(0-9,*,#)");
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_CONT_DTMF_PARA_STRU        stContDTMFPara;
    MODEM_ID_ENUM_UINT16                enModemId;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stContDTMFPara, sizeof(stContDTMFPara), 0x00, sizeof(stContDTMFPara));


    /* Check the validity of parameter */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Non set command!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  Check the validity of <Call_ID> and <Switch> */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Call_ID> or <Switch>!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start and the number of parameter isn't equal to 3.
       Or if the <Switch> is Stop and the number of parameter isn't equal to 2，both invalid */
    if (((gastAtParaList[1].ulParaValue == TAF_CALL_CONT_DTMF_STOP)
      && (gucAtParaIndex != AT_CCONTDTMF_PARA_NUM_MIN))
     || ((gastAtParaList[1].ulParaValue == TAF_CALL_CONT_DTMF_START)
      && (gucAtParaIndex != AT_CCONTDTMF_PARA_NUM_MAX)))
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: The number of parameters mismatch!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start,the <Dtmf_Key> should be setted and check its validity */
    if (gastAtParaList[1].ulParaValue == TAF_CALL_CONT_DTMF_START)
    {
        if (AT_CheckCContDtmfKeyPara() == VOS_ERR)
        {
            AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Dtmf_Key>!");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    stContDTMFPara.ucCallId     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stContDTMFPara.enSwitch     = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stContDTMFPara.ucDigit      = (VOS_UINT8)gastAtParaList[2].aucPara[0];

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_CONT_DTMF_REQ消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  &stContDTMFPara,
                                  ID_TAF_CCM_SEND_CONT_DTMF_REQ,
                                  sizeof(stContDTMFPara),
                                  enModemId);

    if (ulRst == VOS_OK)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCONTDTMF_SET;

        /* Return hang-up state */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_CheckCContDtmfKeyPara(VOS_VOID)
{
    if (gastAtParaList[2].usParaLen != 1)
    {
        return VOS_ERR;
    }

    if (((gastAtParaList[2].aucPara[0] >= '0') && (gastAtParaList[2].aucPara[0] <= '9'))
      || (gastAtParaList[2].aucPara[0] == '*')
      || (gastAtParaList[2].aucPara[0] == '#'))
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFCnf(VOS_VOID *pMsg)
{
    VOS_UINT8                           ucIndex;
    TAF_CCM_SEND_CONT_DTMF_CNF_STRU    *pstContDtmfCnf = VOS_NULL_PTR;

    pstContDtmfCnf = (TAF_CCM_SEND_CONT_DTMF_CNF_STRU*)pMsg;

    /* According to ClientID to get the index */
    if(At_ClientIdToUserId(pstContDtmfCnf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallSndContinuousDTMFCnf: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT module is waiting for report the result of ^CCONTDTMF command */
    if (gastAtClientTab[ucIndex].CmdCurrentOpt != AT_CMD_CCONTDTMF_SET)
    {
        AT_WARN_LOG("AT_RcvTafCallSndContinuousDTMFCnf: Error Option!");
        return VOS_ERR;
    }

    /* Use AT_STOP_TIMER_CMD_READY to recover the AT command state to READY state */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* According to the error code of temporary respond, printf the result of command */
    if (pstContDtmfCnf->stContDtmfCnfPara.enResult != TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_SUCCESS)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFRslt(VOS_VOID *pMsg)
{
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmRcvContinuousDtmfInd(VOS_VOID *pMsg)
{
    TAF_CCM_CONT_DTMF_IND_STRU         *pstRcvContDtmf = VOS_NULL_PTR;
    VOS_UINT8                           aucDigit[2];
    VOS_UINT8                           ucIndex;

    pstRcvContDtmf = (TAF_CCM_CONT_DTMF_IND_STRU*)pMsg;

    /* According to ClientID to get the index */
    if(At_ClientIdToUserId(pstRcvContDtmf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallRcvContinuousDtmfInd: Get Index Fail!");
        return VOS_ERR;
    }

    /*  Initialize aucDigit[0] with pstRcvContDtmf->ucDigit and  aucDigit[1] = '\0'
        Because At_sprintf does not allow to print pstRcvContDtmf->ucDigit with %c
        Hence, need to convert digit into string and print as string */
    aucDigit[0] = pstRcvContDtmf->stContDtmfIndPara.ucDigit;
    aucDigit[1] = '\0';

    /* Output the inquire result */
    if (pstRcvContDtmf->stContDtmfIndPara.enSwitch == TAF_CALL_CONT_DTMF_START)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s^CCONTDTMF: %d,%d,\"%s\"%s",
                                                        gaucAtCrLf,
                                                        pstRcvContDtmf->stCtrl.callId,
                                                        pstRcvContDtmf->stContDtmfIndPara.enSwitch,
                                                        aucDigit,
                                                        gaucAtCrLf);
    }
    else
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s^CCONTDTMF: %d,%d%s",
                                                        gaucAtCrLf,
                                                        pstRcvContDtmf->stCtrl.callId,
                                                        pstRcvContDtmf->stContDtmfIndPara.enSwitch,
                                                        gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCcmRcvBurstDtmfInd(VOS_VOID *pMsg)
{
    TAF_CCM_BURST_DTMF_IND_STRU        *pstRcvBurstDtmf = VOS_NULL_PTR;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM + 1];
    errno_t                             lMemResult;
    VOS_UINT8                           ucIndex;

    pstRcvBurstDtmf = (TAF_CCM_BURST_DTMF_IND_STRU*)pMsg;

    /* According to ClientID to get the index */
    if(At_ClientIdToUserId(pstRcvBurstDtmf->stCtrl.usClientId, &ucIndex) == AT_FAILURE)
    {
        AT_WARN_LOG("AT_RcvTafCallRcvBurstDtmfInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* initialization */
    memset_s(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    lMemResult = memcpy_s(aucDigit, sizeof(aucDigit), pstRcvBurstDtmf->stBurstDtmfIndPara.aucDigit, pstRcvBurstDtmf->stBurstDtmfIndPara.ucDigitNum);
    TAF_MEM_CHK_RTN_VAL(lMemResult, sizeof(aucDigit), pstRcvBurstDtmf->stBurstDtmfIndPara.ucDigitNum);

    /* Add the '\0' to the last byte of pstRcvBurstDtmf->aucDigit */
    aucDigit[pstRcvBurstDtmf->stBurstDtmfIndPara.ucDigitNum] = '\0';

    /* Output the inquire result */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^CBURSTDTMF: %d,\"%s\",%d,%d%s",
                                                    gaucAtCrLf,
                                                    pstRcvBurstDtmf->stCtrl.callId,
                                                    aucDigit,
                                                    pstRcvBurstDtmf->stBurstDtmfIndPara.ulOnLength,
                                                    pstRcvBurstDtmf->stBurstDtmfIndPara.ulOffLength,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

VOS_UINT32 AT_TestCclprPara( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-2)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetCclprPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;
    TAF_CTRL_STRU                       stCtrl;
    TAF_CALL_QRY_CLPR_PARA_STRU         stQryClprPara;
    MODEM_ID_ENUM_UINT16                enModemId;


    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stQryClprPara, sizeof(stQryClprPara), 0x00, sizeof(stQryClprPara));

    /* 参数检查 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    stQryClprPara.callId                = (MN_CALL_ID_T)gastAtParaList[0].ulParaValue;
    stQryClprPara.enQryClprModeType     = TAF_CALL_QRY_CLPR_MODE_C;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* 发送跨核消息ID_TAF_CCM_QRY_CLPR_REQ到C核 */
    ulResult = TAF_CCM_CallCommonReq(&stCtrl,
                                     (void *)&stQryClprPara,
                                     ID_TAF_CCM_QRY_CLPR_REQ,
                                     sizeof(stQryClprPara),
                                     enModemId);

    if (ulResult != VOS_OK)
    {
        AT_WARN_LOG("AT_SetCclprPara: TAF_XCALL_SendCclpr fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCLPR_SET;

    return AT_WAIT_ASYNC_RETURN;

}
#endif

VOS_UINT32 AT_SetRejCallPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_CC_CTX_STRU               *pstCcCtx = VOS_NULL_PTR;

    TAF_CTRL_STRU                       stCtrl;
    MN_CALL_SUPS_PARAM_STRU             stCallSupsPara;
    MODEM_ID_ENUM_UINT16                enModemId;

     /* 指令类型检查 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        AT_WARN_LOG("AT_SetRejCallPara : Current Option is not AT_CMD_REJCALL!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (gucAtParaIndex != 2)
    {
        AT_WARN_LOG("AT_SetRejCallPara : The number of input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((gastAtParaList[0].usParaLen == 0)
     || (gastAtParaList[1].usParaLen == 0))
    {
        AT_WARN_LOG("AT_SetRejCallPara : Input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));
    memset_s(&stCallSupsPara, sizeof(stCallSupsPara), 0x00, sizeof(stCallSupsPara));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    stCallSupsPara.enCallSupsCmd  = MN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITING;
    stCallSupsPara.callId         = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stCallSupsPara.enCallRejCause = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    if (TAF_CCM_CallCommonReq(&stCtrl,
                                      &stCallSupsPara,
                                      ID_TAF_CCM_CALL_SUPS_CMD_REQ,
                                      sizeof(stCallSupsPara),
                                      enModemId) != VOS_OK)
    {
        AT_WARN_LOG("AT_SetRejCallPara : Send Msg fail!");
        return AT_ERROR;
    }

    /* 停止自动接听 */
    pstCcCtx = AT_GetModemCcCtxAddrFromClientId(ucIndex);

    if (pstCcCtx->stS0TimeInfo.bTimerStart == VOS_TRUE)
    {
        AT_StopRelTimer(pstCcCtx->stS0TimeInfo.ulTimerName, &(pstCcCtx->stS0TimeInfo.s0Timer));
        pstCcCtx->stS0TimeInfo.bTimerStart = TAF_FALSE;
        pstCcCtx->stS0TimeInfo.ulTimerName = 0;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_REJCALL_SET;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
}


VOS_UINT32 AT_TestRejCallPara( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (0,1)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}

#if (FEATURE_IMS == FEATURE_ON)

VOS_UINT32 AT_QryCimsErrPara(VOS_UINT8 ucIndex)
{
    TAF_CALL_ERROR_INFO_TEXT_STRU      *pstCallErrInfo = VOS_NULL_PTR;

    pstCallErrInfo = AT_GetCallErrInfoText(ucIndex);

    /* 输出查询结果 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d,\"%s\"",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          AT_GetCsCallErrCause(ucIndex),
                                          pstCallErrInfo->acErrInfoText);

    return AT_OK;
}
#endif


VOS_UINT32 AT_QryCsChannelInfoPara( VOS_UINT8 ucIndex )
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulRst;
    MODEM_ID_ENUM_UINT16                enModemId;

    memset_s(&stCtrl, sizeof(stCtrl), 0x00, sizeof(stCtrl));

    stCtrl.ulModuleId                   = WUEPS_PID_AT;
    stCtrl.usClientId                   = gastAtClientTab[ucIndex].usClientId;
    stCtrl.ucOpId                       = gastAtClientTab[ucIndex].opId;

    if (AT_GetModemIdFromClient(gastAtClientTab[ucIndex].usClientId, &enModemId) != VOS_OK)
    {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    ulRst = TAF_CCM_CallCommonReq(&stCtrl,
                                  VOS_NULL_PTR,
                                  ID_TAF_CCM_QRY_CHANNEL_INFO_REQ,
                                  0,
                                  enModemId);


    if (ulRst != VOS_OK)
    {
        AT_WARN_LOG("AT_QryCsChannelInfoPara: Send Msg fail!");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CSCHANNELINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}



