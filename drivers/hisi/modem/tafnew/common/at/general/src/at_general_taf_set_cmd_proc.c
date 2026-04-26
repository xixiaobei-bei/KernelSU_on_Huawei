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

#include "at_general_taf_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "at_taf_agent_interface.h"
#include "at_mdrv_interface.h"
#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"
#include "at_input_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_TAF_SET_CMD_PROC_C

#define AT_VALID_CHAR_LEN 2
#define AT_CURC_PARA_MIN_NUM 1
#define AT_CURC_PARA_MAX_NUM 2
#define AT_CURC_MODE 0
#define AT_CURC_REPORT_CFG 1
#define AT_CURC_MODE_VAILD_VALUE 2
#define AT_SETPORT_PORT_TYPE 0
#define AT_SETPORT_PORT_TYPE_NUM 3
#define AT_SET_PORT_PORTTYPE_CHARCOUNT 2
#define AT_SET_PORT_PORTTYPE_CHAR1 2
#define AT_SET_PORT_PORTTYPE_CHAR2 1
#define AT_SETPORT_FIRST_PARA_MIN_NUM 1
#define AT_SETPORT_FIRST_PARA_MAX_NUM 2

static const AT_PortSecParaCheckEntity g_PortSecParaCheckTbl[] = {
    { 'A',  '1', 0 },
    { 'A',  '2', 0 },
    { '1',  '6', 0 },
    { '1',  '0', 0 },
    { '1',  '1', 0 },
    { '1',  '2', 0 },
    { '1',  '3', 0 },
    { '1',  '4', 0 },
    { '1',  '5', 0 },
};

TAF_UINT32 At_SetVPara(TAF_UINT8 indexNum)
{
    if (g_atParaList[0].paraLen != 0) {
        g_atVType = (AT_V_TYPE)g_atParaList[0].paraValue;
    } else {
        g_atVType = AT_V_ENTIRE_TYPE;
    }
    return AT_OK;
}

/*
 * Description: S3=[<value>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetS3Para(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        g_atS3 = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        g_atS3 = 13;  /* 命令行结束符，默认值为13 */
    }
    g_atCrLf[0] = g_atS3;
    return AT_OK;
}
/*
 * Description: S4=[<value>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetS4Para(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        g_atS4 = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        g_atS4 = 10;  /* 命令行换行符，默认值为10 */
    }
    g_atCrLf[1] = g_atS4;
    return AT_OK;
}
/*
 * Description: S5=[<value>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetS5Para(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        g_atS5 = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        g_atS5 = 8; /* 命令行退格符，默认值为8 */
    }
    return AT_OK;
}

/*
 * Description: S6=[<value>]
 * History:
 *  1.Date: 2009-05-27
 *    Modification: Created function
 */
TAF_UINT32 At_SetS6Para(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        g_atS6 = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        /*
         * refer to V.250:
         * Recommended default setting
         * 2 Wait two seconds before blind dialling
         */
        g_atS6 = 2;
    }
    return AT_OK;
}
/*
 * Description: S7=[<value>]
 * History:
 *  1.Date: 2009-05-27
 *    Modification: Created function
 */
TAF_UINT32 At_SetS7Para(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * refer to V.250:
     * S7:Number of seconds in which connection must be established or call will be disconnected
     */
    if (g_atParaList[0].paraLen != 0) {
        g_atS7 = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        /* 默认值填写为50 */
        g_atS7 = 50;
    }
    return AT_OK;
}

/*
 * Description: E[<value>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetEPara(TAF_UINT8 indexNum)
{
    if (g_atParaList[0].paraLen != 0) {
        g_atEType = (AT_CMD_ECHO_TYPE)g_atParaList[0].paraValue;
    } else {
        g_atEType = AT_E_ECHO_CMD;
    }
    return AT_OK;
}

/*
 * Description: T
 * History:
 *  1.Date: 2009-05-27
 *    Modification: Created function
 */
TAF_UINT32 At_SetTPara(TAF_UINT8 indexNum)
{
    /*This commands will have no effect in our code */
    return AT_OK;
}
/*
 * Description: P
 * History:
 *  1.Date: 2009-05-27
 *    Modification: Created function
 */
TAF_UINT32 At_SetPPara(TAF_UINT8 indexNum)
{
    /* This commands will have no effect in our code */
    return AT_OK;
}

TAF_UINT32 At_SetZPara(TAF_UINT8 indexNum)
{
    TAF_AT_Tz tz;
    VOS_CHAR  acTzStr[AT_SETZ_LEN + 1];

    VOS_UINT16 length;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
    errno_t    memResult;

    systemAppConfig = AT_GetSystemAppConfigAddr();

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    /*if ATZ was sent from CMUX port, just return OK*/
    if (AT_CheckCmuxUser(indexNum) == VOS_TRUE) {
        return AT_OK;
    }
#endif

    (VOS_VOID)memset_s(&tz, sizeof(tz), 0x00, sizeof(tz));

    if (*systemAppConfig == SYSTEM_APP_WEBUI) {
        if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_ATSETZ_RET_VALUE, &tz, AT_SETZ_LEN) == NV_OK) {
            memResult = memcpy_s(acTzStr, sizeof(acTzStr), tz.tz, AT_SETZ_LEN);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acTzStr), AT_SETZ_LEN);
            acTzStr[AT_SETZ_LEN] = 0;
            if (VOS_StrLen(acTzStr) > 0) {
                length = 0;
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", acTzStr);
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

                At_SendResultData((VOS_UINT8)indexNum, g_atSndCodeAddress, length);

                return AT_SUCCESS;
            }
        }
    }

    return AT_OK;
}
/*
 * Description: Q[value]
 * History:
 *  1.Date: 2009-08-06
 *    Modification: Created function
 */
TAF_UINT32 At_SetQPara(TAF_UINT8 indexNum)
{
    if (g_atParaList[0].paraLen != 0) {
        g_atQType = (AT_CMD_ECHO_TYPE)g_atParaList[0].paraValue;
    } else {
        g_atQType = 0;
    }
    return AT_OK;
}

VOS_UINT32 AT_SetLPara(VOS_UINT8 indexNum)
{
    return AT_OK;
}

VOS_UINT32 AT_SetMPara(VOS_UINT8 indexNum)
{
    return AT_OK;
}

/*
 * Description: +CSCS=[<chset>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCscsPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        g_atCscsType = (AT_CSCS_TYPE)g_atParaList[0].paraValue;
    } else {
        g_atCscsType = AT_CSCS_IRA_CODE;
    }
    return AT_OK;
}

/*
 * Description: +CMEE=[<n>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCmeePara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        g_atCmeeType = (AT_CMEE_TYPE)g_atParaList[0].paraValue;
    } else {
        g_atCmeeType = AT_CMEE_ONLY_ERROR;
    }
    return AT_OK;
}

TAF_UINT32 At_SetCgmiPara(TAF_UINT8 indexNum)
{
    VOS_UINT8 mfrId[TAF_MAX_MFR_ID_STR_LEN + 1];

    (VOS_VOID)memset_s(mfrId, TAF_MAX_MFR_ID_STR_LEN + 1, 0x00, TAF_MAX_MFR_ID_STR_LEN + 1);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_FMRID, mfrId, TAF_MAX_MFR_ID_STR_LEN) != NV_OK) {
        return AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s", mfrId);

        return AT_OK;
    }
}

TAF_UINT32 At_SetCrcPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        ssCtx->crcType = (AT_CRC_TYPE)g_atParaList[0].paraValue;
    } else {
        ssCtx->crcType = AT_CRC_DISABLE_TYPE;
    }

    return AT_OK;
}

VOS_UINT32 At_SetGcapPara(TAF_UINT8 indexNum)
{
    TAF_UINT16 length = 0;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %s",
        g_parseContext[indexNum].cmdElement->cmdName, "+CGSM,+DS,+ES");

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)
VOS_VOID AT_ClacCmdTrustListProc(VOS_VOID)
{
    const AT_CmdTrustListMapTbl *atCmdTrustListTbl = VOS_NULL_PTR;
    VOS_UINT32                   trustListCmdNum;
    VOS_UINT32                   i = 0;

    atCmdTrustListTbl = AT_GetUserCmdTrustListTbl();
    trustListCmdNum   = AT_GetUserCmdTrustListTblCmdNum();
    for (i = 0; i < trustListCmdNum; i++) {
        /* 判断是否需要显示 */
        if (atCmdTrustListTbl[i].chkFlag == TRUST_LIST_CMD_CLAC_IS_VISIBLE) {
            g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%c%c",
                atCmdTrustListTbl[i].atCmd, g_atS3, g_atS4);
        }
    }
}
#endif

VOS_VOID AT_ClacCmdNormalProc(VOS_VOID)
{
    VOS_UINT32              i                = 0;
    const AT_ParCmdElement *cmdBasicElement  = VOS_NULL_PTR;
    const AT_ParCmdElement *cmdExtendElement = VOS_NULL_PTR;
    VOS_UINT32        cmdNum;

    cmdBasicElement = At_GetBasicCmdTable();
    cmdNum     = At_GetBasicCmdNum();

    for (i = 0; i < cmdNum; i++) {
        /* 判断是否需要显示 */
        if ((cmdBasicElement[i].chkFlag & CMD_TBL_CLAC_IS_INVISIBLE) == 0) {
            g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%c%c",
                cmdBasicElement[i].cmdName, g_atS3, g_atS4);
        }
    }

    for (i = 0; i < g_atSmsCmdNum; i++) {
        /* 判断是否需要显示 */
        if ((g_atSmsCmdTab[i].chkFlag & CMD_TBL_CLAC_IS_INVISIBLE) == 0) {
            g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%c%c",
                g_atSmsCmdTab[i].cmdName, g_atS3, g_atS4);
        }
    }

    cmdExtendElement = At_GetExtendCmdTable();
    cmdNum     = At_GetExtendCmdNum();

    for (i = 0; i < cmdNum; i++) {
        /* 判断是否需要显示 */
        if ((cmdExtendElement[i].chkFlag & CMD_TBL_CLAC_IS_INVISIBLE) == 0) {
            g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%c%c",
                cmdExtendElement[i].cmdName, g_atS3, g_atS4);
        }
    }
}

VOS_UINT32 At_SetCLACPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 依次输出支持的可显示的AT命令 */
#if (FEATURE_AT_CMD_TRUST_LIST == FEATURE_ON)
    if (AT_IsNeedMatchTrustList() == VOS_TRUE) {
        AT_ClacCmdTrustListProc();
    } else {
        AT_ClacCmdNormalProc();
    }
#else
    AT_ClacCmdNormalProc();
#endif

    return AT_OK;
}

TAF_UINT32 At_SetCurcPara(TAF_UINT8 indexNum)
{
    AT_MTA_CurcSetNotify atCmd;
    VOS_UINT32           result;
    errno_t              memResult;

#if (FEATURE_LTE == FEATURE_ON)
    L4A_IND_Cfg   l4AIndCfgReq;
    ModemIdUint16 modemId;

    modemId = MODEM_ID_0;

    AT_PR_LOGI("Rcv Msg");
#endif

    /* 参数检查 */
    if ((g_atParaIndex > AT_CURC_PARA_MAX_NUM) || (g_atParaIndex < AT_CURC_PARA_MIN_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.curcRptType = (AT_MTA_RptGeneralCtrlTypeUint8 )g_atParaList[AT_CURC_MODE].paraValue;

    /* mode=0时，禁止所有的主动上报 */
    if (g_atParaList[AT_CURC_MODE].paraValue == 0) {
        (VOS_VOID)memset_s(g_atParaList[AT_CURC_REPORT_CFG].para, sizeof(g_atParaList[AT_CURC_REPORT_CFG].para), 0x00,
                 sizeof(g_atParaList[AT_CURC_REPORT_CFG].para));
    } else if (g_atParaList[AT_CURC_MODE].paraValue == 1) {
        /* mode=1时，打开所有的主动上报 */
        (VOS_VOID)memset_s(g_atParaList[AT_CURC_REPORT_CFG].para, sizeof(g_atParaList[AT_CURC_REPORT_CFG].para), 0xFF,
                 sizeof(g_atParaList[AT_CURC_REPORT_CFG].para));
    } else if (g_atParaList[AT_CURC_MODE].paraValue == AT_CURC_MODE_VAILD_VALUE) {
        /* mode=2时，按BIT位控制对应的主动上报,自定义配置主动上报命令需要<report_cfg>参数 */
        if ((g_atParaIndex != AT_CURC_PARA_MAX_NUM) ||
            (g_atParaList[AT_CURC_REPORT_CFG].paraLen != AT_CURC_RPT_CFG_PARA_LEN)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        result = At_AsciiNum2HexString(g_atParaList[AT_CURC_REPORT_CFG].para,
                                       &g_atParaList[AT_CURC_REPORT_CFG].paraLen);

        AT_PR_LOGI("Call interface success!");
        if (result != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memResult = memcpy_s(atCmd.rptCfg, sizeof(atCmd.rptCfg), g_atParaList[AT_CURC_REPORT_CFG].para,
                         (AT_CURC_RPT_CFG_MAX_SIZE));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmd.rptCfg), (AT_CURC_RPT_CFG_MAX_SIZE));

    /* 通知MTA此次curc的设置 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_CURC_SET_NOTIFY, (VOS_UINT8 *)&atCmd,
                                    sizeof(AT_MTA_CurcSetNotify), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

#if (FEATURE_LTE == FEATURE_ON)
    result = AT_GetModemIdFromClient(indexNum, &modemId);

    if (result != VOS_OK) {
        AT_ERR_LOG1("At_SetCurcPara:Get ModemID From ClientID fail,ClientID=%d", indexNum);
        return AT_ERROR;
    }

    if (AT_IsModemSupportRat(modemId, TAF_MMA_RAT_LTE) == VOS_TRUE) {
        /* 平台能力支持LTE */
        (VOS_VOID)memset_s(&l4AIndCfgReq, sizeof(l4AIndCfgReq), 0x00, sizeof(l4AIndCfgReq));
        l4AIndCfgReq.curcBitValid = 1;
        memResult = memcpy_s(l4AIndCfgReq.curcCfgReq, sizeof(l4AIndCfgReq.curcCfgReq), g_atParaList[1].para,
                             sizeof(l4AIndCfgReq.curcCfgReq));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(l4AIndCfgReq.curcCfgReq), sizeof(l4AIndCfgReq.curcCfgReq));

        /* 通知L此次curc的设置 */
        return AT_SetLIndCfgReq(indexNum, &l4AIndCfgReq);
    } else {
        return AT_OK;
    }
#else
    return AT_OK;
#endif
}

TAF_UINT32 At_SetU2DiagPara(TAF_UINT8 indexNum)
{
    AT_USB_EnumStatus usbEnumStatus;

    NV_PID_EnableType pidEnableType;

    (VOS_VOID)memset_s(&usbEnumStatus, sizeof(usbEnumStatus), 0x00, sizeof(usbEnumStatus));

    pidEnableType.pidEnabled = VOS_FALSE;

    /* 读取PID使能NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PID_ENABLE_TYPE, &pidEnableType, sizeof(NV_PID_EnableType)) != NV_OK) {
        return AT_ERROR;
    }

    /* NV读取成功，检查PID是否使能，若使能，返回ERROR */
    if (pidEnableType.pidEnabled != VOS_FALSE) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 参数有效性检查 */
    if (DRV_U2DIAG_VALUE_CHECK(g_atParaList[0].paraValue) != VOS_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充stUsbEnumStatus结构体 */
    (VOS_VOID)memset_s(&usbEnumStatus, sizeof(usbEnumStatus), 0x00, sizeof(AT_USB_EnumStatus));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_USB_ENUM_STATUS, &usbEnumStatus, sizeof(AT_USB_EnumStatus)) == NV_OK) {
        if ((g_atParaList[0].paraValue == usbEnumStatus.value) && (usbEnumStatus.status == 1)) {
            AT_INFO_LOG("At_SetU2DiagPara():The content to write is same as NV's");
            return AT_OK;
        }
    }

    usbEnumStatus.status = 1;
    usbEnumStatus.value  = g_atParaList[0].paraValue;

    /* 写入NVIM */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_USB_ENUM_STATUS, (VOS_UINT8 *)&usbEnumStatus,
                           sizeof(AT_USB_EnumStatus)) != NV_OK) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckSetPortParaValid(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_BOOL At_FistScanSetPortPara(VOS_UINT16 *semiIndex, VOS_UINT8 *inputString, VOS_UINT16 inputLen)
{
    VOS_UINT32          i;
    VOS_UINT32          j;
    VOS_UINT8           semiCount;
    const VOS_UINT8     aAvailableChar[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', 'A', 'B', 'D', 'E', 'F', ',', ';'
    };

    semiCount = 0;

    for (i = 0; i < inputLen; i++) {
        for (j = 0; j < (sizeof(aAvailableChar) / sizeof(VOS_UINT8)); j++) {
            if (*(inputString + i) == aAvailableChar[j]) {
                if (*(inputString + i) == ';') {
                    semiCount++;
                    *semiIndex = (VOS_UINT16)i;

                    /* 有且只能有一个分号 */
                    if (semiCount > 1) {
                        AT_ERR_LOG("At_FistScanSetPortPara:The num of semi can only be one!");
                        return VOS_FALSE;
                    }
                }
                break;
            }
        }

        /* 不是合法字符 */
        if ((sizeof(aAvailableChar) / sizeof(VOS_UINT8)) == j) {
            AT_ERR_LOG("At_FistScanSetPortPara:Invalid character!");
            return VOS_FALSE;
        }
    }

    /* 有且只能有一个分号并且不是第一个字符也不是最后一个字符 */
    if ((*semiIndex == 0) || ((inputLen - 1) == *semiIndex)) {
        AT_ERR_LOG("At_FistScanSetPortPara:The position of semi isn't suitable!");
        return VOS_FALSE;
    }

    /* 第一个和最后一个字符为逗号 */
    if ((*inputString == ',') || (inputString[inputLen - 1] == ',')) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_BOOL At_SecScanSetPortFirstPara(VOS_UINT16 semiIndex, VOS_UINT8 *firstParaCount, VOS_UINT8 *inputString,
                                    VOS_UINT16 inputLen)
{
    VOS_UINT32 i;
    VOS_UINT8  charCount = 0;

    for (i = 0; i <= semiIndex; i++) {
        /* 逗号或者第一个参数的结尾 */
        if ((*(inputString + i) == ',') || (*(inputString + i) == ';')) {
            /* 参数之间多个逗号的情况A1,,,,A2;1,2 返回ERROR */
            /* 必须是两个字符A1,A2,FF */
            if (charCount != AT_SET_PORT_PORTTYPE_CHARCOUNT) {
                AT_ERR_LOG("At_SecScanSetPortFirstPara:Invalid first parameter!");
                return VOS_FALSE;
            }

            if ((i >= AT_SET_PORT_PORTTYPE_CHARCOUNT) &&
                !(((inputString[i - AT_SET_PORT_PORTTYPE_CHAR1] == 'A') &&
                   (inputString[i - AT_SET_PORT_PORTTYPE_CHAR2] == '1')) ||
                  ((inputString[i - AT_SET_PORT_PORTTYPE_CHAR1] == 'A') &&
                   (inputString[i - AT_SET_PORT_PORTTYPE_CHAR2] == '2')) ||
                  ((inputString[i - AT_SET_PORT_PORTTYPE_CHAR1] == 'F') &&
                   (inputString[i - AT_SET_PORT_PORTTYPE_CHAR2] == 'F')))) {

                AT_ERR_LOG("At_SecScanSetPortFirstPara:Invalid first parameter!");
                return VOS_FALSE;
            }

            (*firstParaCount)++;

            charCount = 0;
        } else {
            charCount++;
        }

        if (charCount > AT_SET_PORT_PORTTYPE_CHARCOUNT) {
            AT_ERR_LOG("At_SecScanSetPortFirstPara:Invalid character!");
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_CheckPortSecParaCharValid(VOS_UINT32 loop, VOS_UINT8 *tmpInputString)
{
    VOS_UINT32      tblSize;
    VOS_UINT32      i;

    tblSize = sizeof(g_PortSecParaCheckTbl) / sizeof(AT_PortSecParaCheckEntity);

    for (i = 0; i < tblSize; i++) {
        if ((tmpInputString[loop - AT_SET_PORT_PORTTYPE_CHAR1] == g_PortSecParaCheckTbl[i].firstChar) &&
            (tmpInputString[loop - AT_SET_PORT_PORTTYPE_CHAR2] == g_PortSecParaCheckTbl[i].secondChar)) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_BOOL AT_SecParaTwoCharProc(VOS_UINT8 charCount, VOS_UINT32 loop, VOS_UINT8 *tmpInputString)
{
    VOS_BOOL bRet = VOS_TRUE;
    /* 如果是两个字符必须为A1,A2或16 */
    if (charCount == AT_SET_PORT_PORTTYPE_CHARCOUNT) {
        if ((loop >= AT_SET_PORT_PORTTYPE_CHARCOUNT) &&
            (AT_CheckPortSecParaCharValid(loop, tmpInputString) != VOS_TRUE)) {
            AT_ERR_LOG("At_SecScanSetPortSecPara:Invalid second parameter!");
            bRet = VOS_FALSE;
        }
    }

    return bRet;
}

VOS_BOOL At_SecScanSetPortSecPara(VOS_UINT16 semiIndex, VOS_UINT8 *secParaCount,
                                    VOS_UINT8 aucSecPara[AT_SETPORT_PARA_MAX_LEN][AT_SETPORT_PORT_TYPE_NUM],
                                    VOS_UINT8 *inputString,
                                    VOS_UINT16 inputLen)
{
    errno_t    memResult;
    VOS_UINT32 i;
    VOS_UINT8  charCount;
    VOS_UINT8  tmpInputString[AT_SETPORT_PARA_MAX_CHAR_LEN];
    VOS_UINT16 tmpInputStringLen;

    charCount = 0;

    if (inputLen > AT_SETPORT_PARA_MAX_CHAR_LEN) {
        return VOS_FALSE;
    }

    /* 第二个参数结尾加上逗号便于以下参数检查 */
    tmpInputStringLen = inputLen - semiIndex;
    if (tmpInputStringLen > 1) {
        memResult = memcpy_s(tmpInputString, sizeof(tmpInputString), inputString + semiIndex + 1,
                             (VOS_SIZE_T)(tmpInputStringLen - 1));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tmpInputString), (VOS_SIZE_T)(tmpInputStringLen - 1));
    }
    tmpInputString[tmpInputStringLen - 1] = ',';

    for (i = 0; i < tmpInputStringLen; i++) {
        if (*(tmpInputString + i) == ',') {
            /* 参数之间多个逗号的情况A1,A2;1,,,2返回eror */
            if (charCount == 0) {
                AT_ERR_LOG("At_SecScanSetPortSecPara:Invalid second parameter!");
                return VOS_FALSE;
            }

            if (AT_SecParaTwoCharProc(charCount, i, tmpInputString) == VOS_FALSE) {
                return VOS_FALSE;
            }

            /* 保存合法的参数 */
            if ((*secParaCount) >= AT_SETPORT_PARA_MAX_LEN) {
                return VOS_FALSE;
            }

            memResult = memcpy_s(aucSecPara[*secParaCount], charCount, (tmpInputString + i) - charCount, charCount);
            TAF_MEM_CHK_RTN_VAL(memResult, charCount, charCount);

            (*secParaCount)++;

            charCount = 0;
        } else {
            charCount++;
        }

        if (charCount > AT_VALID_CHAR_LEN) {
            AT_ERR_LOG("At_SecScanSetPortSecPara:Invalid second parameter!");
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

VOS_BOOL At_SaveSetPortFirstPara(VOS_UINT8 firstParaCount, VOS_UINT8 *firstPort, VOS_UINT8 *inputString)
{
    /* 参数个数不符合要求 */
    if ((firstParaCount != AT_SETPORT_FIRST_PARA_MIN_NUM) && (firstParaCount != AT_SETPORT_FIRST_PARA_MAX_NUM)) {
        return VOS_FALSE;
    }

    /* 只有一个参数: FF或者A1 */
    if (firstParaCount == 1) {
        if ((*inputString == 'F') && (*(inputString + 1) == 'F')) {
            *firstPort = 0xFF;
        } else if ((*inputString == 'A') && (*(inputString + 1) == '1')) {
            *firstPort = 0xA1;
        } else {
            AT_ERR_LOG("At_SaveSetPortFirstPara:Invalid first parameter!");
            return VOS_FALSE;
        }
    }
    /* 两个参数: A1,A2 */
    else {
        if ((*inputString == 'A') && (inputString[1] == '1') && (inputString[3] == 'A') && (inputString[4] == '2')) {
            *firstPort       = 0xA1;
            *(firstPort + 1) = 0xA2;
        } else {
            AT_ERR_LOG("At_SaveSetPortFirstPara:Invalid first parameter!");
            return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}

VOS_BOOL At_SaveSetPortSecPara(VOS_UINT8 secParaCount, VOS_UINT8 *secPort,
                               VOS_UINT8 aucSecPara[AT_SETPORT_PARA_MAX_LEN][AT_SETPORT_PORT_TYPE_NUM])
{
    VOS_UINT32 i;
    VOS_UINT32 j;

    /* 参数个数不符合要求 */
    if ((secParaCount == 0) || (secParaCount > AT_SETPORT_PARA_MAX_LEN)) {
        AT_ERR_LOG("At_SaveSetPortSecPara:Invalid second parameter!");
        return VOS_FALSE;
    }

    /* 参数不能重复 */
    for (i = 0; i < (VOS_UINT8)(secParaCount - 1); i++) {
        for (j = i + 1; j < secParaCount; j++) {
            if (!VOS_MemCmp(aucSecPara[i], aucSecPara[j], AT_SETPORT_PORT_TYPE_NUM)) {
                AT_ERR_LOG("At_SaveSetPortSecPara:Invalid second parameter!");
                return VOS_FALSE;
            }
        }
    }

    for (i = 0; i < secParaCount; i++) {
        for (j = 0; j < AT_SETPORT_DEV_LEN; j++) {
            if (!VOS_MemCmp(aucSecPara[i], g_setPortParaMap[j].atSetPara, AT_SETPORT_PORT_TYPE_NUM)) {
                secPort[i] = g_setPortParaMap[j].drvPara;
                break;
            }
        }

        /* 说明不是合法的设备形态，返回错误 */
        if (j >= AT_SETPORT_DEV_LEN) {
            AT_ERR_LOG("At_SaveSetPortSecPara:Invalid second parameter!");
            return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}

VOS_UINT32 AT_CheckPortTypePara(VOS_UINT8 firstParaCount, VOS_UINT8 *firstPort, VOS_UINT8 secParaCount,
                                VOS_UINT8 *secPort)
{
    VOS_UINT32 ret;

    /*
     * 切换前端口有效性检查:
     * 端口配置第一位必须为A1(CDROM),否则返回ERROR；
     * 第二位如果有，仅允许有A2(SD),否则返回ERROR；
     * 如果是直接上报多端口状态，分号前就直接是FF,否则返回ERROR。
     */
    if (firstParaCount == AT_SETPORT_FIRST_PARA_MAX_NUM) {
        if ((*(firstPort) != AT_DEV_CDROM) || (*(firstPort + 1) != AT_DEV_SD)) {
            return VOS_FALSE;
        }

    } else if (firstParaCount == AT_SETPORT_FIRST_PARA_MIN_NUM) {
        if ((*(firstPort) != AT_DEV_CDROM) && (*(firstPort) != AT_DEV_NONE)) {
            return VOS_FALSE;
        }

    } else {
        return VOS_FALSE;
    }

    /*
     * 切换后端口有效性检查:
     * 1.端口形态中必须有2(PCUI),否则返回ERROR；
     * 2.切换后端口必须是测试命令中输出的支持的端口，否则返回ERROR；
     * 3.不能设置有重复的端口，否则返回ERROR；
     * 4.MASS 不能设置到其他端口的前面，否则返回ERROR
     * 5.端口个数不能超过规定的个数,否则返回ERROR；
     */
    ret = (VOS_UINT32)DRV_USB_PORT_TYPE_VALID_CHECK(secPort, secParaCount);
    if (ret != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_BOOL At_IsAvailableSetPortPara(VOS_UINT8 *inputString, VOS_UINT16 inputLen, VOS_UINT8 *firstPort,
                                   VOS_UINT8 *secPort)
{
    /* 分号的位置 */
    VOS_UINT16 semiIndex = 0;
    VOS_UINT8  firstParaCount = 0;
    VOS_UINT8  secParaCount = 0;
    VOS_UINT8  secPara[AT_SETPORT_PARA_MAX_LEN][AT_SETPORT_PORT_TYPE_NUM];
    VOS_BOOL   bScanResult;
    VOS_UINT32 ret;

    (VOS_VOID)memset_s(secPara, sizeof(secPara), 0x00, sizeof(secPara));

    /* 第一轮扫描:是否有非法字符 */
    bScanResult = At_FistScanSetPortPara(&semiIndex, inputString, inputLen);

    if (bScanResult != VOS_TRUE) {
        AT_ERR_LOG("At_IsAvailableSetPortPara:First parameter is invalid!");
        return VOS_FALSE;
    }

    /* 第二轮扫描:每个参数必须是A1,A2,FF,A,B,D,E,1,2,3,4,5,6,7 */
    /* 第一个参数 */
    bScanResult = At_SecScanSetPortFirstPara(semiIndex, &firstParaCount, inputString, inputLen);

    if (bScanResult != VOS_TRUE) {
        AT_ERR_LOG("At_IsAvailableSetPortPara:First parameter is invalid!");
        return VOS_FALSE;
    }

    /* 第二个参数 */
    bScanResult = At_SecScanSetPortSecPara(semiIndex, &secParaCount, secPara, inputString, inputLen);

    if (bScanResult != VOS_TRUE) {
        AT_ERR_LOG("At_IsAvailableSetPortPara:Second parameter is invalid!");
        return VOS_FALSE;
    }

    /* 保存第一个参数 */
    bScanResult = At_SaveSetPortFirstPara(firstParaCount, firstPort, inputString);

    if (bScanResult != VOS_TRUE) {
        AT_ERR_LOG("At_IsAvailableSetPortPara:Save first parameter fail!");
        return VOS_FALSE;
    }

    /* 保存第二个参数 */
    bScanResult = At_SaveSetPortSecPara(secParaCount, secPort, secPara);

    if (bScanResult != VOS_TRUE) {
        AT_ERR_LOG("At_IsAvailableSetPortPara:Save second parameter fail!");
        return VOS_FALSE;
    }

    ret = AT_CheckPortTypePara(firstParaCount, firstPort, secParaCount, secPort);

    return ret;
}

VOS_UINT32 AT_ExistSpecificPortChange(VOS_UINT8 portType, VOS_UINT8 aucOldRewindPortStyle[],
                                      VOS_UINT8 aucNewRewindPortStyle[])
{
    VOS_UINT32 oldPortPos;
    VOS_UINT32 newPortPos;
    VOS_UINT32 portNum;

    oldPortPos = 0;
    newPortPos = 0;
    portNum    = 0;

    AT_GetSpecificPort(portType, aucOldRewindPortStyle, &oldPortPos, &portNum);
    AT_GetSpecificPort(portType, aucNewRewindPortStyle, &newPortPos, &portNum);

    /* 新增一个指定端口 */
    if ((oldPortPos == AT_DEV_NONE) && (newPortPos != AT_DEV_NONE)) {
        return VOS_TRUE;
    }

    /* 删除一个指定端口 */
    if ((oldPortPos != AT_DEV_NONE) && (newPortPos == AT_DEV_NONE)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_CheckSetPortRight(VOS_UINT8 aucOldRewindPortStyle[], VOS_UINT32 oldRewindPortStyleLen,
    VOS_UINT8 aucNewRewindPortStyle[], VOS_UINT32 newRewindPortStyleLen)
{
    VOS_UINT32 ret;

    ret = AT_ExistSpecificPortChange(AT_DEV_DIAG, aucOldRewindPortStyle, aucNewRewindPortStyle);

    if ((ret == VOS_TRUE) && (g_ate5RightFlag == AT_E5_RIGHT_FLAG_NO)) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_SetPort(VOS_UINT8 indexNum)
{
    errno_t           memResult;
    VOS_BOOL          bValidPara;
    VOS_UINT8         firstPort[AT_SETPORT_PARA_MAX_LEN] = { 0 };
    VOS_UINT8         secPort[AT_SETPORT_PARA_MAX_LEN] = { 0 };
    AT_DynamicPidType dynamicPidType = { 0 };
    NV_PID_EnableType pidEnableType;

    pidEnableType.pidEnabled = VOS_FALSE;

    if (AT_CheckSetPortParaValid() != VOS_TRUE) {
        return AT_ERROR;
    }

    /* 读取PID使能NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PID_ENABLE_TYPE, &pidEnableType, sizeof(NV_PID_EnableType)) != NV_OK) {
        AT_ERR_LOG("At_SetPort:Read NV failed!");
        return AT_ERROR;
    }

    /* NV读取成功，检查PID是否使能，若不使能，返回ERROR */
    if (pidEnableType.pidEnabled != VOS_TRUE) {
        AT_WARN_LOG("At_SetPort:PID is not enabled");
        return AT_ERROR;
    }

    /*
     * 1.参数为空
     * 2.参数超过最大长度
     * 参数最大长度: 两个参数的长度加一个分号
     */
    if ((g_atParaList[AT_SETPORT_PORT_TYPE].paraLen == 0) ||
        (g_atParaList[AT_SETPORT_PORT_TYPE].paraLen > ((AT_SETPORT_PARA_MAX_CHAR_LEN * AT_DOUBLE_LENGTH) + 1))) {
        return AT_ERROR;
    }

    /* 该AT命令支持用户输入大小写，先将小写转成大写，再进行字符串的解析 */
    At_UpString(g_atParaList[AT_SETPORT_PORT_TYPE].para, g_atParaList[AT_SETPORT_PORT_TYPE].paraLen);

    /* 参数解析 */
    bValidPara = At_IsAvailableSetPortPara(g_atParaList[AT_SETPORT_PORT_TYPE].para,
                                           g_atParaList[AT_SETPORT_PORT_TYPE].paraLen, firstPort, secPort);
    if (bValidPara == VOS_FALSE) {
        AT_WARN_LOG("At_SetPort:Parameter is invalid");
        return AT_ERROR;
    }

    /* 读NV失败，直接返回ERROR */
    if (NV_OK !=
        TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_HUAWEI_DYNAMIC_PID_TYPE, &dynamicPidType, sizeof(AT_DynamicPidType))) {
        AT_ERR_LOG("At_SetPort:Read NV fail!");
        return AT_ERROR;
    }

    /* 读NV成功，判断该NV的使能状态，若不使能，直接返回ERROR */
    if (dynamicPidType.nvStatus == VOS_TRUE) {
        /* 判断要写入内容是否与NV中内容相同，如果相同不再写操作，直接返回OK */
        if (!(VOS_MemCmp(dynamicPidType.firstPortStyle, firstPort, AT_SETPORT_PARA_MAX_LEN))) {
            if (!(VOS_MemCmp(dynamicPidType.rewindPortStyle, secPort, AT_SETPORT_PARA_MAX_LEN))) {
                AT_INFO_LOG("At_SetPort:Same, not need write");
                return AT_OK;
            }
        }
    } else {
        return AT_ERROR;
    }

    /* 此处增加DIAG口密码保护    */
    if (AT_CheckSetPortRight(dynamicPidType.rewindPortStyle, sizeof(dynamicPidType.rewindPortStyle),
            secPort, sizeof(secPort)) != AT_OK) {
        return AT_ERROR;
    }

    memResult = memcpy_s(dynamicPidType.firstPortStyle, sizeof(dynamicPidType.firstPortStyle), firstPort,
                         AT_SETPORT_PARA_MAX_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dynamicPidType.firstPortStyle), AT_SETPORT_PARA_MAX_LEN);
    memResult = memcpy_s(dynamicPidType.rewindPortStyle, sizeof(dynamicPidType.rewindPortStyle), secPort,
                         AT_SETPORT_PARA_MAX_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dynamicPidType.rewindPortStyle), AT_SETPORT_PARA_MAX_LEN);

    /* 写入NV */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_HUAWEI_DYNAMIC_PID_TYPE, (VOS_UINT8 *)&dynamicPidType,
                           sizeof(AT_DynamicPidType)) != NV_OK) {
        AT_ERR_LOG("At_SetPort:Write NV fail");
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}

TAF_UINT32 At_SetPcscInfo(TAF_UINT8 indexNum)
{
    AT_DynamicPidType dynamicPidType;
    NV_PID_EnableType pidEnableType;

    pidEnableType.pidEnabled = VOS_FALSE;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(&dynamicPidType, sizeof(dynamicPidType), 0x00, sizeof(AT_DynamicPidType));

    /* 读取PID使能NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PID_ENABLE_TYPE, &pidEnableType, sizeof(NV_PID_EnableType)) != NV_OK) {
        AT_ERR_LOG("At_SetPcscInfo:Read NV31 failed!");
        return AT_ERROR;
    }

    /* NV读取成功，检查PID是否使能，若不使能，返回ERROR */
    if (pidEnableType.pidEnabled != VOS_TRUE) {
        AT_WARN_LOG("At_SetPcscInfo:PID is not enabled!");
        return AT_ERROR;
    }

    /*  打开PCSC口 */
    if (g_atParaList[0].paraValue == VOS_TRUE) {
        return AT_OpenSpecificPort(AT_DEV_PCSC);
    }
    /* 关闭PCSC口 */
    else {
        return AT_CloseSpecificPort(AT_DEV_PCSC);
    }
}

TAF_UINT32 At_SetGetportmodePara(TAF_UINT8 indexNum)
{
    TAF_PH_Port  portInfo;
    TAF_PH_FmrId fmrID;
    TAF_UINT16   dataLen;
    TAF_UINT32   rslt;
    TAF_UINT32   portInfoLen;
    errno_t      memResult;

    /* 参数检查 */
    if ((g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) && (g_atParaList[0].paraLen == 0)) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(&portInfo, sizeof(portInfo), 0x00, sizeof(TAF_PH_Port));

    memResult = strncpy_s((VOS_CHAR *)portInfo.productName.productName, sizeof(portInfo.productName.productName),
                          PRODUCTION_CARD_TYPE, PRODUCTION_CARD_TYPE_MAX_LEN);
    TAF_STRCPY_CHK_RTN_VAL_CONTINUE(memResult, sizeof(portInfo.productName.productName), PRODUCTION_CARD_TYPE_MAX_LEN);

    (VOS_VOID)memset_s(&fmrID, sizeof(fmrID), 0x00, sizeof(TAF_PH_FmrId));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_FMRID, &fmrID, sizeof(TAF_PH_FmrId)) != NV_OK) {
        AT_WARN_LOG("At_SetMsIdInfo:WARNING:NVIM Read NV_ITEM_FMRID falied!");
        return AT_ERROR;
    } else {
        memResult = memcpy_s(portInfo.fmrId.mfrId, sizeof(portInfo.fmrId.mfrId), &fmrID, sizeof(TAF_PH_FmrId));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(portInfo.fmrId.mfrId), sizeof(TAF_PH_FmrId));
    }

    portInfoLen = TAF_MAX_PORT_INFO_LEN + 1;

    rslt = (VOS_UINT32)(DRV_GET_PORT_MODE((char *)portInfo.portId.portInfo, &portInfoLen));
    if (rslt != VOS_OK) {
        AT_WARN_LOG("At_SetGetportmodePara:DRV_GET_PORT_MODE Info fail.");
        return AT_ERROR;
    }

    dataLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s: %s", "TYPE", portInfo.productName.productName);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, ": %s,", portInfo.fmrId.mfrId);
    dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s", portInfo.portId.portInfo);

    g_atSendDataBuff.bufLen = dataLen;
    return AT_OK;
}

VOS_UINT32 AT_SetFclassPara(VOS_UINT8 indexNum)
{
    AT_WARN_LOG("AT_SetFclassPara: Not support this command!");
    (VOS_VOID)indexNum;
    return AT_ERROR;
}

VOS_UINT32 AT_SetGciPara(VOS_UINT8 indexNum)
{
    AT_WARN_LOG("AT_SetGciPara: Not support this command!");
    (VOS_VOID)indexNum;
    return AT_ERROR;
}

VOS_UINT32 AT_SetPcuiCtrlConcurrentFlagForTest(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    AT_SetPcuiCtrlConcurrentFlag((VOS_UINT8)(g_atParaList[0].paraValue));
    return AT_OK;
}

