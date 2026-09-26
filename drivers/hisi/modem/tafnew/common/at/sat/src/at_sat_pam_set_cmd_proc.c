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
#include "at_sat_pam_set_cmd_proc.h"
#include "securec.h"
#include "nv_stru_gucnas.h"
#include "at_ctx.h"
#include "at_type_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "at_input_proc.h"
#include "at_data_proc.h"
#include "at_check_func.h"
#include "at_mbb_cmd.h"
#include "at_msg_print.h"
#include "taf_std_lib.h"
#include "at_sim_comm.h"
#include "si_app_pb.h"
#include "si_app_stk.h"
#include "at_set_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAT_PAM_SET_CMD_PROC_C
#define AT_STSF_PARA_MAX_NUM 3
#define AT_STSF_CONFIG 1
#define AT_STSF_TIMEOUT 2
#define AT_STSF_CONFIG_LENGTH_MAX 32
#define AT_STGR_RESULT 2
#define AT_STGR_DATA 3
#define AT_STGR_PARA_MAX_NUM 4
#define AT_STGR_DATA_VALID_LENGTH 2
#define AT_STGR_MIN_VALID_NUM 3
#define AT_STGR_MAX_VALID_NUM 4
#define AT_DOUBLE_QUOTATION_MARKS_LENGTH 2
#define AT_SIMSLOT_MAX_PARA_NUM 3
#define AT_SIMSLOT_MODEM0 0
#define AT_SIMSLOT_MODEM1 1
#define AT_SIMSLOT_MODEM2 2
#define AT_SIMSLOT_MULTI_MODEM_CARD_NUM 3
#define AT_SIMSLOT_OTHER_MODEM_CARD_NUM 2
#define AT_ASCII_STR_MIN_LEN 2
#define AT_ASCII_STR_MAX_LEN 255
#define AT_STGR_CMD_TYPE 0
#define AT_STGR_CMD_NUM 1
#define AT_STGR_MENU_SELECTION 1
#define AT_STGR_NEED_HELP_INFORMATION 2

static const AT_AtCmdConvertSiStkTypeEntity g_AtCmdConvertSiStkTypeTbl[] = {
        { SI_AT_CMD_DISPLAY_TEXT,   SI_STK_DISPLAYTET },
        { SI_AT_CMD_GET_INKEY,      SI_STK_GETINKEY },
        { SI_AT_CMD_GET_INPUT,      SI_STK_GETINPUT },
        { SI_AT_CMD_SETUP_CALL,     SI_STK_SETUPCALL },
        { SI_AT_CMD_PLAY_TONE,      SI_STK_PLAYTONE },
        { SI_AT_CMD_SELECT_ITEM,    SI_STK_SELECTITEM },
        { SI_AT_CMD_REFRESH,        SI_STK_REFRESH },
        { SI_AT_CMD_SEND_SS,        SI_STK_SENDSS },
        { SI_AT_CMD_SEND_SMS,       SI_STK_SENDSMS },
        { SI_AT_CMD_SEND_USSD,      SI_STK_SENDUSSD },
        { SI_AT_CMD_LAUNCH_BROWSER, SI_STK_LAUNCHBROWSER },
        { SI_AT_CMD_SETUP_IDLE_MODE_TEXT, SI_STK_SETUPIDLETEXT },
        { SI_AT_CMD_LANGUAGENOTIFICATION, SI_STK_LANGUAGENOTIFICATION },
        { SI_AT_CMD_SETFRAMES,      SI_STK_SETFRAMES },
        { SI_AT_CMD_GETFRAMESSTATUS,SI_STK_GETFRAMESSTATUS },
    };

TAF_UINT32 At_SetStsfPara(TAF_UINT8 indexNum)
{
    USIMM_StkCfg  profileContent;
    TAF_UINT32    result;
    errno_t       memResult;
    TAF_UINT16    len = sizeof(USIMM_StkCfg);
    ModemIdUint16 modemId;

    if (g_atParaList[0].paraLen != 1) { /* check the para */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_STSF_PARA_MAX_NUM) { /* para too many */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_TERMINALPROFILE_SET, &profileContent, len);

    if (result != NV_OK) {
        return AT_ERROR;
    }

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* change the string to Ascii value */
    profileContent.funcEnable = (TAF_UINT8)g_atParaList[0].para[0] - 0x30;

    if (g_atParaList[1].paraLen != 0x00) { /* have the <config> para */
        /* change the string formate */
        if (At_AsciiNum2HexString(g_atParaList[1].para, &g_atParaList[1].paraLen) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* according the etsi102.223,the most len or Terminal Profile is 32 */
        if (g_atParaList[AT_STSF_CONFIG].paraLen > AT_STSF_CONFIG_LENGTH_MAX) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (AT_GetUsimInfoCtxFromModemId(modemId)->cardType == TAF_MMA_USIMM_CARD_USIM) {
            profileContent.usimProfile.profileLen = (TAF_UINT8)g_atParaList[1].paraLen;

            memResult = memcpy_s(profileContent.usimProfile.profile,
                                 (VOS_SIZE_T)sizeof(profileContent.usimProfile.profile), g_atParaList[1].para,
                                 profileContent.usimProfile.profileLen);
            TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(profileContent.usimProfile.profile),
                                profileContent.usimProfile.profileLen);
        } else {
            profileContent.simProfile.profileLen = (TAF_UINT8)g_atParaList[1].paraLen;

            memResult = memcpy_s(profileContent.simProfile.profile,
                                 (VOS_SIZE_T)sizeof(profileContent.simProfile.profile), g_atParaList[1].para,
                                 profileContent.simProfile.profileLen);
            TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(profileContent.simProfile.profile),
                                profileContent.simProfile.profileLen);
        }
    }

    if (g_atParaList[AT_STSF_TIMEOUT].paraLen != 0x00) { /* ave the <Timer>  para */
        result = At_Auc2ul(g_atParaList[AT_STSF_TIMEOUT].para, g_atParaList[AT_STSF_TIMEOUT].paraLen,
                           &g_atParaList[AT_STSF_TIMEOUT].paraValue);
        if (result != AT_SUCCESS) {
            AT_WARN_LOG("At_SetStsfPara():Warning: At_Auc2ul convert failed!");

            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ((g_atParaList[AT_STSF_TIMEOUT].paraValue > 0xFF) || (g_atParaList[AT_STSF_TIMEOUT].paraValue == 0)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        profileContent.timer = (TAF_UINT8)g_atParaList[AT_STSF_TIMEOUT].paraValue;
    }

    result = TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_TERMINALPROFILE_SET, (VOS_UINT8 *)&profileContent, len);

    if (result != NV_OK) {
        return AT_ERROR;
    }

    return AT_OK;
}

LOCAL VOS_UINT32 At_CheckAsciiStringLen(VOS_UINT16 srcLen)
{
    if ((srcLen < AT_ASCII_STR_MIN_LEN) || (srcLen > AT_ASCII_STR_MAX_LEN)) {
        return VOS_ERR;
    }

    return VOS_OK;
}

TAF_UINT32 At_AsciiString2HexText(SI_STK_TextString *textStr, TAF_UINT8 *src, TAF_UINT16 srcLen)
{
    TAF_UINT16 chkLen = 0;
    TAF_UINT8  tmp    = 0;
    TAF_UINT8  left   = 0;
    TAF_UINT8 *dst    = textStr->text;

    if (At_CheckAsciiStringLen(srcLen) != VOS_OK) {
        AT_ERR_LOG1("At_AsciiString2HexText: usSrcLen is wrong.", srcLen);
        return AT_FAILURE;
    }

    while (chkLen < srcLen) {
        if (AT_SetHighFourBitHexString(&left, src, chkLen) == AT_FAILURE) {
            return AT_FAILURE;
        }

        dst[tmp] = 0xf0 & (left << 4);

        chkLen += 1;

        if (AT_SetLowFourBitHexString(&dst[tmp], src, chkLen) == AT_FAILURE) {
            return AT_FAILURE;
        }

        chkLen += 1;

        tmp += 1;
    }

    textStr->len = tmp;

    return AT_SUCCESS;
}

/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_ChangeCmdResult(TAF_UINT32 resultNo, TAF_UINT8 *cmdResult)
{
    TAF_UINT32 result = AT_SUCCESS;

    switch (resultNo) {
        case SI_AT_RESULT_END_SESSION:
            *cmdResult = PROACTIVE_UICC_SESSION_TERMINATED_BY_THE_USER;
            break;
        case SI_AT_RESULT_PERFORM_SUCC:
            *cmdResult = COMMAND_PERFORMED_SUCCESSFULLY;
            break;
        case SI_AT_RESULT_HELP_REQUIRED:
            *cmdResult = HELP_INFORMATION_REQUIRED_BY_USER;
            break;
        case SI_AT_RESULT_BACKWARD_MOVE:
            *cmdResult = BACKWARD_MOVE_IN_THE_PROACTIVE_UICC_SESSION_REQUESTED_BY_USER;
            break;
        case SI_AT_RESULT_ME_NOT_SUPPORT:
            *cmdResult = COMMAND_BEYOND_TERMINALS_CAPABILITIES;
            break;
        case SI_AT_RESULT_ME_BROWSER_BUSY:
            *cmdResult = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
            break;
        case SI_AT_RESULT_ME_SS_BUSY:
            *cmdResult = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
            break;
        default:
            result = AT_ERROR;
    }

    return result;
}
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
TAF_UINT32 At_StgiGetKey(TAF_UINT8 indexNum, SI_STK_TerminalRsp *rsp)
{
    TAF_UINT8  cmdResult;
    TAF_UINT8 *inputKey;
    errno_t    memResult;
    TAF_UINT8  atStkCmdQualify = AT_GetStkCmdQualify();

    inputKey = (TAF_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, AT_PARA_MAX_LEN + 1);

    if (inputKey == NULL) {
        AT_NORM_LOG("At_StgiGetKey: Fail to malloc mem.");
        return AT_ERROR;
    }

    if (At_ChangeCmdResult(g_atParaList[AT_STGR_RESULT].paraValue, &cmdResult) == AT_ERROR) {
        VOS_MemFree(WUEPS_PID_AT, inputKey);
        return AT_ERROR;
    }

    rsp->result.result = cmdResult;

    if (cmdResult == COMMAND_PERFORMED_SUCCESSFULLY) {
        if ((g_atParaIndex != AT_STGR_PARA_MAX_NUM) ||
            ((g_atParaList[AT_STGR_DATA].paraLen >= (AT_PARA_MAX_LEN + 1)) ||
            (g_atParaList[AT_STGR_DATA].paraLen < AT_STGR_DATA_VALID_LENGTH))) {
            AT_WARN_LOG("At_StgiGetKey: 4th para error!");
            VOS_MemFree(WUEPS_PID_AT, inputKey);
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 专有参数 */
        rsp->uResp.getInkeyRsp.textStr.text = rsp->data;

        (VOS_VOID)memset_s(inputKey, AT_PARA_MAX_LEN + 1, 0x00, AT_PARA_MAX_LEN + 1);
        memResult = memcpy_s(inputKey, AT_PARA_MAX_LEN + 1, g_atParaList[AT_STGR_DATA].para,
                             g_atParaList[AT_STGR_DATA].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, g_atParaList[AT_STGR_DATA].paraLen);

        /*
         * 由于第四个参数是变参的，AT词法解析器无法解析，必须在代码中自行解析处理，
         * 解析时需要将引号去除，长度减2
         */
        if (At_AsciiString2HexText(&rsp->uResp.getInkeyRsp.textStr, inputKey + 1,
                                   g_atParaList[AT_STGR_DATA].paraLen - AT_DOUBLE_QUOTATION_MARKS_LENGTH) ==
            AT_FAILURE) {
            VOS_MemFree(WUEPS_PID_AT, inputKey);
            return AT_CME_INCORRECT_PARAMETERS;
        }

        rsp->uResp.getInkeyRsp.opTextStr = 1;

        if ((atStkCmdQualify & 0x02) == 0x00) {
            rsp->uResp.getInkeyRsp.textStr.dcs = 0x04;
        } else {
            rsp->uResp.getInkeyRsp.textStr.dcs = 0x08;
        }

        if ((atStkCmdQualify & 0x04) == 0x04) {
            *rsp->uResp.getInkeyRsp.textStr.text -= 0x30;
        }
    }

    VOS_MemFree(WUEPS_PID_AT, inputKey);
    return AT_SUCCESS;
}

TAF_UINT32 At_StgiGetInput(TAF_UINT8 indexNum, SI_STK_TerminalRsp *rsp)
{
    errno_t    memResult;
    TAF_UINT8  cmdResult;
    TAF_UINT8 *inputString;
    VOS_UINT32 length;
    TAF_UINT8  atStkCmdQualify = AT_GetStkCmdQualify();

    inputString = (TAF_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, AT_PARA_MAX_LEN + 1);

    if (inputString == NULL) {
        AT_NORM_LOG("At_StgiGetInput: Fail to malloc mem.");
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(inputString, AT_PARA_MAX_LEN + 1, 0x00, AT_PARA_MAX_LEN + 1);

    length = AT_PARA_MAX_LEN + 1;

    if (At_ChangeCmdResult(g_atParaList[AT_STGR_RESULT].paraValue, &cmdResult) == AT_ERROR) {
    VOS_MemFree(WUEPS_PID_AT, inputString);
        return AT_ERROR;
    }

    rsp->result.result = cmdResult;

    if (cmdResult == COMMAND_PERFORMED_SUCCESSFULLY) {
        /* 专有参数 */
        if (g_atParaList[AT_STGR_DATA].paraLen < 0x03) {
            rsp->uResp.getInkeyRsp.textStr.text = TAF_NULL_PTR;

            rsp->uResp.getInkeyRsp.textStr.len = 0;
        }
        else if (length > g_atParaList[AT_STGR_DATA].paraLen) {
            rsp->uResp.getInkeyRsp.textStr.text = rsp->data;

            (VOS_VOID)memset_s(inputString, AT_PARA_MAX_LEN + 1, 0x00, AT_PARA_MAX_LEN + 1);
            memResult = memcpy_s(inputString, AT_PARA_MAX_LEN + 1, g_atParaList[AT_STGR_DATA].para,
                                 g_atParaList[AT_STGR_DATA].paraLen);
            TAF_MEM_CHK_RTN_VAL(memResult, AT_PARA_MAX_LEN + 1, g_atParaList[AT_STGR_DATA].paraLen);

            /* 由于第四个参数是变参的，AT词法解析器无法解析，必须在代码中自行解析处理，解析时需要将引号去除，长度减2 */
            if (At_AsciiString2HexText(&rsp->uResp.getInputRsp.textStr, inputString + 1,
                                       g_atParaList[AT_STGR_DATA].paraLen - AT_DOUBLE_QUOTATION_MARKS_LENGTH) ==
                                       AT_FAILURE) {
                VOS_MemFree(WUEPS_PID_AT, inputString);
                return AT_CME_INCORRECT_PARAMETERS;
            }
        } else {
            AT_WARN_LOG1("At_StgiGetInput: 4th para error!", g_atParaList[AT_STGR_DATA].paraLen);
            VOS_MemFree(WUEPS_PID_AT, inputString);
            return AT_CME_INCORRECT_PARAMETERS;
        }

        rsp->uResp.getInputRsp.opTextStr = 1;

        if ((atStkCmdQualify & 0x02) == 0x00) {
            if ((atStkCmdQualify & 0x08) == 0x00) {
                rsp->uResp.getInkeyRsp.textStr.dcs = 0x04;
            } else {
                rsp->uResp.getInkeyRsp.textStr.dcs = 0x00;
            }
        } else {
            rsp->uResp.getInkeyRsp.textStr.dcs = 0x08;
        }
    }

    VOS_MemFree(WUEPS_PID_AT, inputString);
    return AT_SUCCESS;
}
#else
TAF_UINT32 At_StgiGetKey(TAF_UINT8 indexNum, SI_STK_TerminalRsp *rsp)
{
    TAF_UINT8 cmdResult;
    TAF_UINT8 inputKey[AT_PARA_MAX_LEN + 1];
    errno_t   memResult;
    TAF_UINT8 atStkCmdQualify = AT_GetStkCmdQualify();

    if (At_ChangeCmdResult(g_atParaList[AT_STGR_RESULT].paraValue, &cmdResult) == AT_ERROR) {
        return AT_ERROR;
    }

    rsp->result.result = cmdResult;

    if (cmdResult == COMMAND_PERFORMED_SUCCESSFULLY) {
        if ((g_atParaIndex != AT_STGR_PARA_MAX_NUM) ||
            (((VOS_SIZE_T)sizeof(inputKey) <= g_atParaList[AT_STGR_DATA].paraLen) ||
            (g_atParaList[AT_STGR_DATA].paraLen < AT_STGR_DATA_VALID_LENGTH))) {
            AT_WARN_LOG("At_StgiGetKey: 4th para error!");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 专有参数 */
        rsp->uResp.getInkeyRsp.textStr.text = rsp->data;

        (VOS_VOID)memset_s(inputKey, sizeof(inputKey), 0x00, sizeof(inputKey));
        memResult = memcpy_s(inputKey, sizeof(inputKey), g_atParaList[AT_STGR_DATA].para,
                             g_atParaList[AT_STGR_DATA].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(inputKey), g_atParaList[AT_STGR_DATA].paraLen);

        /*
         * 由于第四个参数是变参的，AT词法解析器无法解析，必须在代码中自行解析处理，
         * 解析时需要将引号去除，长度减2
         */
        if (At_AsciiString2HexText(&rsp->uResp.getInkeyRsp.textStr, inputKey + 1,
                                   g_atParaList[AT_STGR_DATA].paraLen - AT_DOUBLE_QUOTATION_MARKS_LENGTH) ==
            AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        rsp->uResp.getInkeyRsp.opTextStr = 1;

        if ((atStkCmdQualify & 0x02) == 0x00) {
            rsp->uResp.getInkeyRsp.textStr.dcs = 0x04;
        } else {
            rsp->uResp.getInkeyRsp.textStr.dcs = 0x08;
        }

        if ((atStkCmdQualify & 0x04) == 0x04) {
            *rsp->uResp.getInkeyRsp.textStr.text -= 0x30;
        }
    }

    return AT_SUCCESS;
}
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_StgiGetInput(TAF_UINT8 indexNum, SI_STK_TerminalRsp *rsp)
{
    errno_t   memResult;
    TAF_UINT8 cmdResult;
    TAF_UINT8 inputString[AT_PARA_MAX_LEN + 1];
    VOS_UINT32 length;
    TAF_UINT8  atStkCmdQualify = AT_GetStkCmdQualify();

    length = (VOS_SIZE_T)sizeof(inputString);

    if (At_ChangeCmdResult(g_atParaList[AT_STGR_RESULT].paraValue, &cmdResult) == AT_ERROR) {
        return AT_ERROR;
    }

    rsp->result.result = cmdResult;

    if (cmdResult == COMMAND_PERFORMED_SUCCESSFULLY) {
        /* 专有参数 */
        if (g_atParaList[AT_STGR_DATA].paraLen < 0x03) {
            rsp->uResp.getInkeyRsp.textStr.text = TAF_NULL_PTR;

            rsp->uResp.getInkeyRsp.textStr.len = 0;
        }
        else if (length > g_atParaList[AT_STGR_DATA].paraLen) {
            rsp->uResp.getInkeyRsp.textStr.text = rsp->data;

            (VOS_VOID)memset_s(inputString, sizeof(inputString), 0x00, sizeof(inputString));
            memResult = memcpy_s(inputString, sizeof(inputString), g_atParaList[AT_STGR_DATA].para,
                                 g_atParaList[AT_STGR_DATA].paraLen);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(inputString), g_atParaList[AT_STGR_DATA].paraLen);

            /* 由于第四个参数是变参的，AT词法解析器无法解析，必须在代码中自行解析处理，解析时需要将引号去除，长度减2 */
            if (At_AsciiString2HexText(&rsp->uResp.getInputRsp.textStr, inputString + 1,
                                       g_atParaList[AT_STGR_DATA].paraLen - AT_DOUBLE_QUOTATION_MARKS_LENGTH) ==
                                       AT_FAILURE) {
                return AT_CME_INCORRECT_PARAMETERS;
            }
        } else {
            AT_WARN_LOG1("At_StgiGetInput: 4th para error!", g_atParaList[AT_STGR_DATA].paraLen);
            return AT_CME_INCORRECT_PARAMETERS;
        }

        rsp->uResp.getInputRsp.opTextStr = 1;

        if ((atStkCmdQualify & 0x02) == 0x00) {
            if ((atStkCmdQualify & 0x08) == 0x00) {
                rsp->uResp.getInkeyRsp.textStr.dcs = 0x04;
            } else {
                rsp->uResp.getInkeyRsp.textStr.dcs = 0x00;
            }
        } else {
            rsp->uResp.getInkeyRsp.textStr.dcs = 0x08;
        }
    }

    return AT_SUCCESS;
}
#endif

TAF_UINT32 At_StgiSelectItem(TAF_UINT8 indexNum, SI_STK_TerminalRsp *rsp)
{
    TAF_UINT8  cmdResult;
    VOS_UINT32 rslt;

    if (At_ChangeCmdResult(g_atParaList[AT_STGR_RESULT].paraValue, &cmdResult) == AT_ERROR) {
        return AT_ERROR;
    }

    rsp->result.result = cmdResult;

    if (cmdResult == COMMAND_PERFORMED_SUCCESSFULLY) {
        rslt = At_Auc2ul(g_atParaList[AT_STGR_DATA].para, g_atParaList[AT_STGR_DATA].paraLen,
                         &g_atParaList[AT_STGR_DATA].paraValue);
        if (rslt != AT_SUCCESS) {
            AT_WARN_LOG("At_StgiSelectItem():Warning: At_Auc2ul convert failed!");
            return AT_ERROR;
        }

        /* 专有参数 */
        rsp->uResp.selectItemRsp.itemId.itemId = (TAF_UINT8)g_atParaList[AT_STGR_DATA].paraValue;
        rsp->uResp.selectItemRsp.opItemID      = 1;
    }

    return AT_SUCCESS;
}

/*
 * Description: chang the cmdNo to cmdType
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_ChangeSATCmdType(TAF_UINT32 cmdNo, TAF_UINT32 *cmdType)
{
    VOS_UINT32 tblSize;
    VOS_UINT32 i;

    tblSize = sizeof(g_AtCmdConvertSiStkTypeTbl) / sizeof(AT_AtCmdConvertSiStkTypeEntity);

    for (i = 0; i < tblSize; i++) {
        if (g_AtCmdConvertSiStkTypeTbl[i].atCmdType == cmdNo) {
            *cmdType = g_AtCmdConvertSiStkTypeTbl[i].siStkType;
            return AT_SUCCESS;
        }
    }

    return AT_ERROR;
}

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) || (defined(DMT)) || (defined(__PC_UT__)))
/*
 * Description: ^STGI
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetStgiPara(TAF_UINT8 indexNum)
{
    TAF_UINT32 result;
    TAF_UINT32 cmdType;

    if (g_stkFunctionFlag == TAF_FALSE) {
        return AT_ERROR;
    }

    if (g_atParaList[0].paraLen == 0) { /* check the para */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != 2) { /* the para number is must be 2 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue > 0x0C) { /* the <cmdType> is not more the 12 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue == 0x00) { /* get the main menu content */
        result = SI_STK_GetMainMenu(g_atClientTab[indexNum].clientId, 0);
    } else if (At_ChangeSATCmdType(g_atParaList[0].paraValue, &cmdType) == AT_ERROR) { /* get other SAT content */
        return AT_ERROR;
    } else {
        result = SI_STK_GetSTKCommand(g_atClientTab[indexNum].clientId, 0, cmdType);
    }

    if (result == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_STGI_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_CheckStgrPara(VOS_VOID)
{
    #if (VOS_WIN32 != VOS_OS_VER)
    if (g_stkFunctionFlag == TAF_FALSE) {
        return AT_ERROR;
    }
#endif

    if ((g_atParaIndex > AT_STGR_MAX_VALID_NUM) || (g_atParaIndex < AT_STGR_MIN_VALID_NUM)) { /* the para munber is too many or too few */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* the para1 and para2 is must be inputed */
    if ((g_atParaList[AT_STGR_CMD_TYPE].paraLen == 0) || (g_atParaList[AT_STGR_CMD_NUM].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_ProcStgrCmdType(TAF_UINT8 indexNum, SI_STK_TerminalRsp *rsponse)
{
    TAF_UINT32         result = 0;

    if (g_atParaList[AT_STGR_CMD_TYPE].paraValue == SI_AT_CMD_END_SESSION) {
        rsponse->result.result = PROACTIVE_UICC_SESSION_TERMINATED_BY_THE_USER;
    } else if (At_ChangeSATCmdType(g_atParaList[AT_STGR_CMD_TYPE].paraValue, &rsponse->cmdType) != AT_SUCCESS) {
        return AT_ERROR;
    } else {
    }

    switch (g_atParaList[0].paraValue) {
        case SI_AT_CMD_GET_INKEY:
            result = At_StgiGetKey(indexNum, rsponse); /* change the GetInkey response content */
            break;
        case SI_AT_CMD_GET_INPUT:
            result = At_StgiGetInput(indexNum, rsponse); /* change the GetInput response content */
            break;
        case SI_AT_CMD_SELECT_ITEM:
            result = At_StgiSelectItem(indexNum, rsponse); /* change the SelectItem response content */
            break;
        default:
            result = At_ChangeCmdResult(g_atParaList[AT_STGR_RESULT].paraValue, &rsponse->result.result);
            break;
    }

    return result;
}

TAF_UINT32 At_StgrSelMenu(TAF_UINT8 indexNum)
{
    SI_STK_MenuSelection *menu = VOS_NULL_PTR;
    SI_STK_Envelope       eNStru;
    VOS_UINT32            rslt;

    /* 参数过多 */
    if (g_atParaIndex > AT_STGR_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    eNStru.envelopeType  = SI_STK_ENVELOPE_MENUSEL;
    eNStru.deviceId.sdId = SI_DEVICE_KEYPAD;
    eNStru.deviceId.ddId = SI_DEVICE_UICC;

    menu = &eNStru.envelope.menuSelect;
    (VOS_VOID)memset_s(menu, sizeof(SI_STK_MenuSelection), 0x00, sizeof(SI_STK_MenuSelection));

    if (g_atParaList[AT_STGR_RESULT].paraValue == SI_AT_RESULT_HELP_REQUIRED) {
        menu->opHelp      = 1;
        menu->help.rsv[0] = 1;
    }

    /* select item */
    if (g_atParaList[AT_STGR_DATA].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    } else {
        rslt = At_Auc2ul(g_atParaList[AT_STGR_DATA].para, g_atParaList[AT_STGR_DATA].paraLen,
                         &g_atParaList[AT_STGR_DATA].paraValue);
        if (rslt != AT_SUCCESS) {
            AT_WARN_LOG("At_StgiSelectItem():Warning: At_Auc2ul convert failed!");
            return AT_ERROR;
        }

        menu->opItemIp      = 1;
        menu->itemId.itemId = (TAF_UINT8)g_atParaList[AT_STGR_DATA].paraValue;
    }

    /* 执行命令操作 */
    if (SI_STK_MenuSelectionFunc(g_atClientTab[indexNum].clientId, 0, &eNStru) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_STGR_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR; /* 返回命令处理挂起状态 */
    }
}

/*
 * Description: ^STGR=<cmdnum>,<cmdtype>,<result>,<data>
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetStgrPara(TAF_UINT8 indexNum)
{
    SI_STK_TerminalRsp rsponse;
    TAF_UINT32         result = 0;

    result = AT_CheckStgrPara();
    if (result != AT_OK) {
        return result;
    }

    rsponse.deviceId.sdId = SI_DEVICE_KEYPAD;
    rsponse.deviceId.ddId = SI_DEVICE_UICC;

    if (g_atParaList[AT_STGR_CMD_TYPE].paraValue == 0x00) {
        /* menu selection or need the help infomation */
        if ((g_atParaList[AT_STGR_RESULT].paraValue == AT_STGR_MENU_SELECTION) ||
            (g_atParaList[AT_STGR_RESULT].paraValue == AT_STGR_NEED_HELP_INFORMATION)) {
            return At_StgrSelMenu(indexNum);
        } else {
            return AT_OK;
        }
    }

    (VOS_VOID)memset_s(&rsponse, sizeof(rsponse), 0x00, sizeof(SI_STK_TerminalRsp));

    result = AT_ProcStgrCmdType(indexNum, &rsponse);

    if (result != AT_SUCCESS) {
        return result; /* return the error */
    }

    if (SI_STK_TerminalResponse(g_atClientTab[indexNum].clientId, 0, &rsponse) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_STGR_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 At_IsSimSlotAllowed(VOS_UINT32 modem0Slot, VOS_UINT32 modem1Slot, VOS_UINT32 modem2Slot)
{
#if (MULTI_MODEM_NUMBER == 3)
    /* 三Modem,配置单卡模式 */
    if ((modem1Slot == modem2Slot) && (modem1Slot == SI_PIH_CARD_SLOT_2) && (modem0Slot <= SI_PIH_CARD_SLOT_1)) {
        return VOS_TRUE;
    }
    /* 三Modem,任意两个Modem不能同时配置为同一卡槽 */
    if ((modem0Slot == modem1Slot) || (modem1Slot == modem2Slot) || (modem0Slot == modem2Slot)) {
        return VOS_FALSE;
    }
#else
    /* 双Modem,两个Modem不能同时配置为同一卡槽 */
    if (modem0Slot == modem1Slot) {
        return VOS_FALSE;
    }
#endif

    return VOS_TRUE;
}

VOS_UINT32 At_SetSIMSlotPara(VOS_UINT8 indexNum)
{
#if (MULTI_MODEM_NUMBER != 1)
    TAF_NvSciCfg sCICfg;
#endif
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_SIMSLOT_MAX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if ((g_atParaList[AT_SIMSLOT_MODEM0].paraLen == 0) || (g_atParaList[AT_SIMSLOT_MODEM1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 单modem不支持切换卡槽 */
#if (MULTI_MODEM_NUMBER == 1)
    return AT_CME_OPERATION_NOT_ALLOWED;
#else

    /* 三卡形态第3个参数不能为空，其余形态默认为卡槽2 */
#if (MULTI_MODEM_NUMBER == 3)
    if (g_atParaList[AT_SIMSLOT_MODEM2].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#else
    g_atParaList[AT_SIMSLOT_MODEM2].paraValue = SI_PIH_CARD_SLOT_2;
#endif

    /* 参数检查 */
    if (At_IsSimSlotAllowed(g_atParaList[AT_SIMSLOT_MODEM0].paraValue, g_atParaList[AT_SIMSLOT_MODEM1].paraValue,
                            g_atParaList[AT_SIMSLOT_MODEM2].paraValue) == VOS_FALSE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 从NV中读取当前SIM卡的SCI配置 */
    (VOS_VOID)memset_s(&sCICfg, sizeof(sCICfg), 0x00, sizeof(sCICfg));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_SCI_DSDA_CFG, &sCICfg, sizeof(sCICfg)) != NV_OK) {
        AT_ERR_LOG("At_SetSIMSlotPara: NV_ITEM_SCI_DSDA_CFG read fail!");
        return AT_ERROR;
    }

    /*
     * 根据用户设置的值修改card0位和card1位的值，在NV项中，这两项对应的bit位和取值含义如下:
     * card0: bit[8-10]：卡槽0使用的SCI接口
     *     0：使用SCI0（默认值）
     *     1：使用SCI1
     *     2：使用SCI2
     *     其余值：无效
     * card1:bit[11-13]：卡1槽使用的SCI接口
     *     0：使用SCI0
     *     1：使用SCI1（默认值）
     *     2：使用SCI2
     *     其余值：无效
     * card2:bit[14-16]：卡2槽使用的SCI接口
     *     0：使用SCI0
     *     1：使用SCI1
     *     2：使用SCI2（默认值）
     *     其余值：无效
     */
    sCICfg.bitCard0 = g_atParaList[AT_SIMSLOT_MODEM0].paraValue;
    sCICfg.bitCard1 = g_atParaList[AT_SIMSLOT_MODEM1].paraValue;

    /* 针对双卡形态增加保护，bitCard2使用NV默认值，与底软处理适配 */
#if (MULTI_MODEM_NUMBER == 3)
    sCICfg.bitCard2   = g_atParaList[AT_SIMSLOT_MODEM2].paraValue;
    sCICfg.bitCardNum = AT_SIMSLOT_MULTI_MODEM_CARD_NUM;
#else
    sCICfg.bitCardNum = AT_SIMSLOT_OTHER_MODEM_CARD_NUM;
#endif

    sCICfg.bitReserved0 = 0;
    sCICfg.bitReserved1 = 0;

    /* 将设置的SCI值保存到NV中 */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_SCI_DSDA_CFG, (VOS_UINT8 *)&sCICfg, sizeof(sCICfg)) != NV_OK) {
        AT_ERR_LOG("At_SetSIMSlotPara: NV_ITEM_SCI_DSDA_CFG write failed");
        return AT_ERROR;
    }

    return AT_OK;
#endif
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_SetSimInsertPara(VOS_UINT8 indexNum)
{
    TAF_SIM_InsertStateUint32 simInsertState;
    VOS_UINT32                result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    simInsertState = (TAF_SIM_InsertStateUint32)g_atParaList[0].paraValue;

    result = TAF_MMA_SimInsertReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  simInsertState);
    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIMINSERT_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}
#endif


