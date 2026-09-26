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
#include "at_ss_taf_set_cmd_proc.h"
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
#include "mnmsgcbencdec.h"
#include "taf_std_lib.h"
#include "at_ss_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SS_TAF_SET_CMD_PROC_C

#define AT_CCUG_MAX_PARA_NUM 3
#define AT_CCUG_MODE_ENABLE 0
#define AT_CCUG_GROUP_INDEX 1
#define AT_CCUG_INFO 2
#define AT_CHLD_OP_CODE 0
#define AT_CHLD_CALL_TYPE 1
#define AT_CHLD_CALL_NUM 2
#define AT_CHLD_RTT_FLG 3
#define AT_CCUG_INDEX_VALUE_DENOTES_NO_INDEX 10
#define AT_CCUG_CUG_INDEX 10
#define AT_CSSN_MAX_PARA_NUM 2
#define AT_CUUS1_MESSAGE 2
#define AT_CUUS1_FIRST_AND_SECOND_PARA_COUNT 2 // <n>和<m>两个参数个数总数
#define AT_CCWA_PARA_MAX_NUM 3
#define AT_CCWA_CLASS 2
#define AT_CCFC_NUMBER 2
#define AT_CCFC_TYPE 3
#define AT_CCFC_CLASS 4
#define AT_CCFC_SUBADDR 5
#define AT_CCFC_SATTYPE 6
#define AT_CCFC_TIME 7
#define AT_CCFC_PARA_MAX_NUM 8
#define AT_SET_SS_CODE_CFU 0
#define AT_SET_SS_CODE_CFB 1
#define AT_SET_SS_CODE_CFNRY 2
#define AT_SET_SS_CODE_CFNRC 3
#define AT_SET_SS_CODE_AF 4
#define AT_CCFC_CLASS_DEDICATED_PAD_ACCESS 128 // dedicated PAD access参数值
#define AT_NOREPCONDTIME_DEFAULT 20
#define AT_CUSD_PARA_MAX_NUM 3
#define AT_CUSD_DCS 2
#define AT_CUSD_N 0
#define AT_CUSD_STR 1
#define AT_CUSD_N_EXIT_SESSION 2
#define AT_CUSD_N_CANCEL_SESSION_VALUE 2
#define AT_CTFR_NUMBER 0
#define AT_CTFR_SUBADDR 2
#define MN_CALL_MAX_ASCII_NUM_LEN (MN_CALL_MAX_BCD_NUM_LEN * 2)
#define AT_MN_CALL_REL_HELD_OR_UDUB 0
#define AT_MN_CALL_REL_ACT_ACPT_OTH 1
#define AT_MN_CALL_HOLD_ACT_ACPT_OTH 2
#define AT_MN_CALL_BUILD_MPTY 3
#define AT_MN_CALL_ECT 4
#define AT_MN_CALL_ACT_CCBS 5
#define AT_MN_CALL_ECONF_REL_USER 6
#define AT_MN_CALL_ECONF_MERGE_CALL 7
#define AT_MN_CALL_ACPT_WAITING_CALL 8
#define AT_MN_CALL_REL_CALL_X 1
#define AT_MN_CALL_HOLD_ALL_EXCPT_X 2
#define AT_CHLD_MAX_PARA_NUM 4
#define AT_CTFR_PARA_MAX_NUM 4
#define AT_USSD_STRING_CNT 1
#define AT_CMOLR_MAX_PARA_NUM 15
#define AT_CMOLR_HOR_ACC_SET 2
#define AT_CMOLR_HOR_ACC 3
#define AT_CMOLR_VER_REQ 4
#define AT_CMOLR_VER_ACC_SET 5
#define AT_CMOLR_VER_ACC 6
#define AT_CMOLR_VEL_REQ 7
#define AT_CMOLR_REP_MODE 8
#define AT_CMOLR_TIMEOUT 9
#define AT_CMOLR_INTERVAL 10
#define AT_CMOLR_SHAPE_REP 11
#define AT_CMOLR_PLANE 12
#define AT_CMOLR_NMEA_REP 13
#define AT_CMOLR_THIRD_PARTY_ADDRESS 14
#define AT_CMTLRA_PARA_NUM 2
#define AT_CMOLR_HOR_ACC_SET_INDEX 2
#define AT_CMOLR_HOR_ACC_INDEX 3
#define AT_CMOLR_VER_REQ_INDEX 4
#define AT_CMOLR_VER_ACC_SET_INDEX 5
#define AT_CMOLR_VER_ACC_INDEX 6
#define AT_CMOLR_VEL_REQ_INDEX 7
#define AT_CMOLR_REP_MODE_INDEX 8
#define AT_CMOLR_TIMEOUT_INDEX 9
#define AT_CMOLR_INTERVAL_INDEX 10
#define AT_CMOLR_SHAPE_REP_INDEX 11
#define AT_CMOLR_PLANE_INDEX 12
#define AT_CMOLR_NMEA_REP_INDEX 13
#define AT_CMOLR_THIRD_PARTY_ADDRESS_INDEX 14
#define AT_ALS_PARA_NUM 2
#define AT_CALLMODIFYINIT_PARA_NUM 3
#define AT_CALLMODIFYINIT_IDX 0
#define AT_CALLMODIFYINIT_CURR_CALL_TYPE 1
#define AT_CALLMODIFYANS_PARA_NUM 3
#define AT_CALLMODIFYANS_IDX 0
#define AT_CALLMODIFYANS_CURR_CALL_TYPE 1
#define AT_CALLMODIFYINIT_DEST_CALL_TYPE 2
#define AT_CALLMODIFYANS_DEST_CALL_TYPE 2
#define AT_VIDEO_CALL_CAP_SWITCH 0
#define AT_VIDEO_CALL_CAP_CCWA 1
#define AT_IMSVTCAPCFG_PARA_NUM 2

LOCAL VOS_UINT32 AT_CnmiOptTypeDProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeChldProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeCpinProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsRegisterProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsEraseProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsActivateProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsDeactivateProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsDeactiveCCBSProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsInterrogateProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeSsRegisterPswdProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeCusdProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId);
LOCAL VOS_UINT32 AT_CnmiOptTypeClipProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId);

static const AT_CnmiOptTypeMatchEntity g_CnmiOptTypeMatchTbl[] = {
    { AT_CMD_D_CS_VOICE_CALL_SET,  AT_CnmiOptTypeDProc},
    { AT_CMD_CHLD_SET,  AT_CnmiOptTypeChldProc},
    { AT_CMD_CPIN_UNBLOCK_SET,  AT_CnmiOptTypeCpinProc},
    { AT_CMD_CPIN2_UNBLOCK_SET,  AT_CnmiOptTypeCpinProc},
    { AT_CMD_CPIN_VERIFY_SET,  AT_CnmiOptTypeCpinProc},
    { AT_CMD_CPIN2_VERIFY_SET,  AT_CnmiOptTypeCpinProc},
    { AT_CMD_SS_REGISTER,  AT_CnmiOptTypeSsRegisterProc},
    { AT_CMD_SS_ERASE,  AT_CnmiOptTypeSsEraseProc},
    { AT_CMD_SS_ACTIVATE,  AT_CnmiOptTypeSsActivateProc},
    { AT_CMD_SS_DEACTIVATE,  AT_CnmiOptTypeSsDeactivateProc},
    { AT_CMD_SS_DEACTIVE_CCBS,  AT_CnmiOptTypeSsDeactiveCCBSProc},
    { AT_CMD_SS_INTERROGATE,  AT_CnmiOptTypeSsInterrogateProc},
    { AT_CMD_SS_INTERROGATE_CCBS,  AT_CnmiOptTypeSsInterrogateProc},
    { AT_CMD_SS_REGISTER_PSWD,  AT_CnmiOptTypeSsRegisterPswdProc},
    { AT_CMD_CUSD_REQ,  AT_CnmiOptTypeCusdProc},
    { AT_CMD_CLIP_READ,  AT_CnmiOptTypeClipProc},
};

TAF_UINT32 At_SetClipPara(TAF_UINT8 indexNum)
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
        ssCtx->clipType = (AT_CLIP_TYPE)g_atParaList[0].paraValue;
    } else {
        ssCtx->clipType = AT_CLIP_DISABLE_TYPE;
    }

    return AT_OK;
}

TAF_UINT32 At_SetClirPara(TAF_UINT8 indexNum)
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
        switch (g_atParaList[0].paraValue) {
            case AT_CLIR_AS_SUBSCRIPT:
                ssCtx->clirType = AT_CLIR_AS_SUBSCRIPT;
                break;

            case AT_CLIR_INVOKE:
                ssCtx->clirType = AT_CLIR_INVOKE;
                break;

            case AT_CLIR_SUPPRESS:
                ssCtx->clirType = AT_CLIR_SUPPRESS;
                break;

            default:
                return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        ssCtx->clirType = AT_CLIR_AS_SUBSCRIPT;
    }
    return AT_OK;
}

VOS_UINT32 At_SetColpPara(VOS_UINT8 indexNum)
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
        ssCtx->colpType = (AT_COLP_TYPE)g_atParaList[0].paraValue;
    } else {
        ssCtx->colpType = AT_COLP_DISABLE_TYPE;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCcugPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CCUG_MAX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<n> */
    if (g_atParaList[AT_CCUG_MODE_ENABLE].paraLen != 0) {
        ssCtx->ccugCfg.enable = g_atParaList[AT_CCUG_MODE_ENABLE].paraValue;
    } else {
        ssCtx->ccugCfg.enable = 0;
    }

    /* 设置<index> */
    if (g_atParaList[AT_CCUG_GROUP_INDEX].paraLen != 0) {
        if (g_atParaList[AT_CCUG_GROUP_INDEX].paraValue == AT_CCUG_INDEX_VALUE_DENOTES_NO_INDEX) {
            ssCtx->ccugCfg.indexPresent = 0;
            ssCtx->ccugCfg.index = AT_CCUG_CUG_INDEX;
        } else {
            ssCtx->ccugCfg.indexPresent = 1;
            ssCtx->ccugCfg.index        = (TAF_UINT8)g_atParaList[AT_CCUG_GROUP_INDEX].paraValue;
        }
    } else {
        ssCtx->ccugCfg.indexPresent = 0;
        ssCtx->ccugCfg.index = 0;
    }

    /* 设置<info> */
    if (g_atParaList[AT_CCUG_INFO].paraLen != 0) {
        ssCtx->ccugCfg.suppressPrefer = (g_atParaList[AT_CCUG_INFO].paraValue & 0x02) >> 1;
        ssCtx->ccugCfg.suppressOa     = g_atParaList[AT_CCUG_INFO].paraValue & 0x01;
    } else {
        ssCtx->ccugCfg.suppressPrefer = 0;
        ssCtx->ccugCfg.suppressOa     = 0;
    }
    return AT_OK;
}

TAF_UINT32 At_SetCssnPara(TAF_UINT8 indexNum)
{
    VOS_UINT32         rst;
    TAF_Ctrl           ctrl;
    MN_CALL_SetCssnReq cssnReq;
    ModemIdUint16      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&cssnReq, sizeof(cssnReq), 0x00, sizeof(cssnReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CSSN_MAX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    cssnReq.actNum     = MN_CALL_MAX_CSSN_MSG_NUM;
    cssnReq.setType[0] = MN_CALL_SET_CSSN_DEACT;
    cssnReq.setType[1] = MN_CALL_SET_CSSN_DEACT;

    if (g_atParaList[0].paraLen != 0) {
        cssnReq.setType[0] = (MN_CALL_SetCssnTypeUint32)g_atParaList[0].paraValue;
    }

    if (g_atParaList[1].paraLen != 0) {
        cssnReq.setType[1] = (MN_CALL_SetCssnTypeUint32)g_atParaList[1].paraValue;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, &cssnReq, ID_TAF_CCM_SET_CSSN_REQ, sizeof(cssnReq), modemId);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSSN_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

TAF_BOOL At_ParseSingleChldPara(TAF_UINT32 paraValue, MN_CALL_SupsParam *callSupsCmd)
{
    switch (paraValue) {
        case AT_MN_CALL_REL_HELD_OR_UDUB:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_REL_HELD_OR_UDUB;
            break;

        case AT_MN_CALL_REL_ACT_ACPT_OTH:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_REL_ACT_ACPT_OTH;
            break;

        case AT_MN_CALL_HOLD_ACT_ACPT_OTH:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH;
            break;

        case AT_MN_CALL_BUILD_MPTY:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_BUILD_MPTY;
            break;

        case AT_MN_CALL_ECT:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_ECT;
            break;

        case AT_MN_CALL_ACT_CCBS:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_ACT_CCBS;
            return AT_OK;

        case AT_MN_CALL_ECONF_REL_USER:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_ECONF_REL_USER;
            break;

        case AT_MN_CALL_ECONF_MERGE_CALL:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_ECONF_MERGE_CALL;
            break;

        case AT_MN_CALL_ACPT_WAITING_CALL:
            callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_ACPT_WAITING_CALL;
            break;

        default:
            return AT_ERROR;
    }
    return AT_OK;
}

TAF_BOOL At_ParseChldPara(TAF_UINT32 paraValue, MN_CALL_SupsParam *callSupsCmd)
{
    TAF_BOOL  bRetRst;
    TAF_UINT8 tmp;

    tmp = (TAF_UINT8)(paraValue / 10); /* 除10是为了获取参数值大于10时的操作码 */

    if (paraValue < 10) { /* 参数值小于10直接获取操作码 */
        bRetRst = At_ParseSingleChldPara(paraValue, callSupsCmd);
    } else {
        switch (tmp) {
            case AT_MN_CALL_REL_CALL_X:
                callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_REL_CALL_X;
                callSupsCmd->callId      = (MN_CALL_ID_T)(paraValue % AT_DECIMAL_BASE_NUM);
                bRetRst                  = AT_OK;
                break;

            case AT_MN_CALL_HOLD_ALL_EXCPT_X:
                callSupsCmd->callSupsCmd = MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X;
                callSupsCmd->callId      = (MN_CALL_ID_T)(paraValue % AT_DECIMAL_BASE_NUM);
                bRetRst                  = AT_OK;
                break;

            default:
                bRetRst = AT_ERROR;
                break;
        }
    }
    return bRetRst;
}

TAF_UINT32 At_SetChldPara(TAF_UINT8 indexNum)
{
    MN_CALL_SupsParam callChldParam;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    TAF_Ctrl      ctrl;
    VOS_UINT32    rst;
    ModemIdUint16 modemId;

    AT_PR_LOGH("At_SetChldPara Enter : CmdType = %d", g_atParaList[0].paraValue);

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

    (VOS_VOID)memset_s(&callChldParam, sizeof(callChldParam), 0x00, sizeof(callChldParam));

    rst = At_ParseChldPara(g_atParaList[0].paraValue, &callChldParam);
    if (rst == AT_ERROR) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送补充业务请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &callChldParam, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(callChldParam), modemId) ==
        VOS_OK) {
        /* 停止自动接听 */
        ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            AT_StopRelTimer(ccCtx->s0TimeInfo.timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        AT_PR_LOGH("At_SetChldPara success");

        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CHLD_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcUus1WithParm(VOS_UINT8 indexNum, MN_CALL_Uus1Param *uus1Group)
{
    VOS_UINT32 ret;
    VOS_UINT32 msgNum;
    VOS_UINT32 i;
    VOS_UINT32 rst;
    VOS_UINT32 strLength;
    errno_t    memResult;

    TAF_Ctrl          ctrl;
    MN_CALL_Uus1Param uus1Info;
    ModemIdUint16     modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&uus1Info, sizeof(uus1Info), 0x00, sizeof(uus1Info));

    /* 去处<n>和<m>后,实际的消息个数 */
    msgNum = g_atParaIndex;

    if (g_atParaList[AT_CUUS1_MESSAGE].paraLen == 0) {
        uus1Group->actNum = 0;
    } else {
        /* 设置message */
        for (i = AT_CUUS1_FIRST_AND_SECOND_PARA_COUNT; (i < msgNum && i < (AT_MAX_PARA_NUMBER - 1)); i++) {
            /*
             * 先需要转换,由于该命令可设置多次参数,AT格式中仅对前4个参数进行转换,
             * 后面的参数未进行转换,因此需要将数组中的值进行所转换
             */
            if (At_Auc2ul(g_atParaList[i].para, g_atParaList[i].paraLen, &g_atParaList[i].paraValue) != AT_SUCCESS) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            uus1Group->uus1Info[uus1Group->actNum].msgType = g_atParaList[i].paraValue;
            i++;

            /* <UUIE>不存在,表明是去激活 */
            if (g_atParaList[i].paraLen == 0) {
                uus1Group->setType[uus1Group->actNum] = MN_CALL_SET_UUS1_DEACT;
            } else {
                uus1Group->setType[uus1Group->actNum] = MN_CALL_SET_UUS1_ACT;
                /* 需将ASCII转换为HEX */
                if (At_AsciiNum2HexString(g_atParaList[i].para, &g_atParaList[i].paraLen) == AT_FAILURE) {
                    return AT_CME_INCORRECT_PARAMETERS;
                }

                strLength = sizeof(uus1Group->uus1Info[uus1Group->actNum].uuie);
                if (strLength < g_atParaList[i].paraLen) {
                    return AT_CME_INCORRECT_PARAMETERS;
                }
                memResult = memcpy_s(uus1Group->uus1Info[uus1Group->actNum].uuie,
                                     sizeof(uus1Group->uus1Info[uus1Group->actNum].uuie), g_atParaList[i].para,
                                     g_atParaList[i].paraLen);
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(uus1Group->uus1Info[uus1Group->actNum].uuie),
                                    g_atParaList[i].paraLen);
            }

            ret = MN_CALL_CheckUus1ParmValid(uus1Group->setType[uus1Group->actNum],
                                             &(uus1Group->uus1Info[uus1Group->actNum]));
            if (ret != MN_ERR_NO_ERROR) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            uus1Group->actNum++;

            /* 如果未携带UUIE,则默认表示结束不关心后面参数 */
            if (uus1Group->setType[uus1Group->actNum] == MN_CALL_SET_UUS1_DEACT) {
                break;
            }
        }
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    memResult = memcpy_s(&uus1Info, sizeof(uus1Info), uus1Group, sizeof(MN_CALL_Uus1Param));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(uus1Info), sizeof(MN_CALL_Uus1Param));

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /*
     * 1.对要设置的各个Uus1Info进行设定，然后通过MN_CALL_APP_SET_UUSINFO消息带参数结构
     * MN_CALL_Uus1Param通知CS进行参数设置
     */
    rst = TAF_CCM_CallCommonReq(&ctrl, &uus1Info, ID_TAF_CCM_SET_UUSINFO_REQ, sizeof(uus1Info), modemId);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APP_SET_UUSINFO_REQ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_SetCuus1Para(VOS_UINT8 indexNum)
{
    VOS_UINT32        ret;
    MN_CALL_Uus1Param uus1Info; /* 设置UUS1信息结构 */

    /*
     * 27007中该命令格式,该命令可以不携带任何参数,如果不携带参数直接返回OK
     * + CUUS1 =[<n>[,<m>[,<message>[,<UUIE> [,<message>[,<UUIE>[,...]]]]]]]
     */

    /* 不携带参数直接返回OK */
    if (g_atParaIndex == 0) {
        return AT_OK;
    }

    (VOS_VOID)memset_s(&uus1Info, sizeof(uus1Info), 0x00, sizeof(uus1Info));

    /* 设置<n> */
    if (g_atParaList[0].paraLen != 0) {
        uus1Info.cuus1IFlg = (MN_CALL_Cuus1FlgUint32)g_atParaList[0].paraValue;
    } else {
        /* <n>不存在同时参数个数不为0返回ERROR */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<m> */
    if (g_atParaList[1].paraLen != 0) {
        uus1Info.cuus1UFlg = (MN_CALL_Cuus1FlgUint32)g_atParaList[1].paraValue;
    }

    /* 设置message */
    /* 只带n和m参数时，也要给MN发消息 */
    ret = AT_ProcUus1WithParm(indexNum, &uus1Info);

    return ret;
}

TAF_UINT32 At_SetCcwaPara(TAF_UINT8 indexNum)
{
    TAF_SS_ErasessReq para;
    AT_ModemSsCtx    *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CCWA_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));

    /* 设置SsCode */
    para.ssCode = TAF_CW_SS_CODE;

    /* 设置<n> */
    if (g_atParaList[0].paraLen != 0) {
        ssCtx->ccwaType = (AT_CCWA_TYPE)g_atParaList[0].paraValue;
    } else {
        ssCtx->ccwaType = AT_CCWA_DISABLE_TYPE;
    }

    if (g_atParaList[1].paraLen == 0) {
        return AT_OK;
    }

    /* 设置<class> */
    if (AT_FillSsBsService(&para, &g_atParaList[AT_CCWA_CLASS]) != VOS_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    switch (g_atParaList[1].paraValue) {
        case 0: /* disable */
            if (TAF_DeactivateSSReq(g_atClientTab[indexNum].clientId, 0, &para) == AT_SUCCESS) {
                /* 设置当前操作类型 */
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCWA_DISABLE;
                return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
            } else {
                return AT_ERROR;
            }
            /* fall-through */

        case 1: /* enable */
            if (TAF_ActivateSSReq(g_atClientTab[indexNum].clientId, 0, &para) == AT_SUCCESS) {
                /* 设置当前操作类型 */
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCWA_ENABLE;
                return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
            } else {
                return AT_ERROR;
            }
            /* fall-through */

        default: /* query status */
            g_atClientTab[indexNum].temp = g_atParaList[AT_CCWA_CLASS].paraValue;
            para.opBsService             = 0;
            if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, &para) == AT_SUCCESS) {
                /* 设置当前操作类型 */
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCWA_QUERY;
                return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
            } else {
                return AT_ERROR;
            }
    }
}

VOS_UINT32 AT_CheckCcfcParaValid(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CCFC_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果<number>过长，直接返回错误 */
    if (g_atParaList[AT_CCFC_NUMBER].paraLen > TAF_SS_MAX_FORWARDED_TO_NUM_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果<subaddr>过长，直接返回错误 */
    if (g_atParaList[AT_CCFC_SUBADDR].paraLen > TAF_SS_MAX_FORWARDED_TO_SUBADDRESS_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查NUM号码输入不正确 */
    if (g_atParaList[AT_CCFC_NUMBER].paraLen != 0) {
        if (At_CheckDialString(g_atParaList[AT_CCFC_NUMBER].para,
                               g_atParaList[AT_CCFC_NUMBER].paraLen) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 检查SubAddress号码输入不正确 */
    if (g_atParaList[AT_CCFC_SUBADDR].paraLen != 0) {
        if (At_CheckDialString(g_atParaList[AT_CCFC_SUBADDR].para,
                               g_atParaList[AT_CCFC_SUBADDR].paraLen) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_OK;
}

TAF_SS_CODE AT_GetSsCodeFromCfccPara(VOS_VOID)
{
    TAF_SS_CODE code;

    /* 设置SsCode */
    switch (g_atParaList[0].paraValue) {
        case AT_SET_SS_CODE_CFU:
            code = TAF_CFU_SS_CODE;
            break;

        case AT_SET_SS_CODE_CFB:
            code = TAF_CFB_SS_CODE;
            break;

        case AT_SET_SS_CODE_CFNRY:
            code = TAF_CFNRY_SS_CODE;
            break;

        case AT_SET_SS_CODE_CFNRC:
            code = TAF_CFNRC_SS_CODE;
            break;

        case AT_SET_SS_CODE_AF:
            code = TAF_ALL_FORWARDING_SS_CODE;
            break;

        default:
            code = TAF_ALL_COND_FORWARDING_SS_CODE;
            break;
    }

    return code;
}

VOS_VOID AT_SetNumAttribute(TAF_SS_NUMTYPE *numType, TAF_UINT8 *fwdToNum)
{
    errno_t memResult;
    /* +表示国际号码，在号码类型中已经指示，不需要拷贝到消息中 */
    if (g_atParaList[AT_CCFC_NUMBER].para[0] == '+') {
        if ((g_atParaList[AT_CCFC_NUMBER].paraLen - 1) > 0) {
            memResult = memcpy_s((TAF_CHAR *)fwdToNum, TAF_SS_MAX_FORWARDED_TO_NUM_LEN + 1,
                                 (TAF_CHAR *)(g_atParaList[AT_CCFC_NUMBER].para + 1),
                                 (VOS_SIZE_T)(g_atParaList[AT_CCFC_NUMBER].paraLen - 1));
            TAF_MEM_CHK_RTN_VAL(memResult, TAF_SS_MAX_FORWARDED_TO_NUM_LEN + 1,
                                (VOS_SIZE_T)(g_atParaList[AT_CCFC_NUMBER].paraLen - 1));
        }
    } else {
        if (g_atParaList[AT_CCFC_NUMBER].paraLen > 0) {
            memResult = memcpy_s((TAF_CHAR *)fwdToNum, TAF_SS_MAX_FORWARDED_TO_NUM_LEN + 1,
                                 (TAF_CHAR *)g_atParaList[AT_CCFC_NUMBER].para,
                                 g_atParaList[AT_CCFC_NUMBER].paraLen);
            TAF_MEM_CHK_RTN_VAL(memResult, TAF_SS_MAX_FORWARDED_TO_NUM_LEN + 1,
                                g_atParaList[AT_CCFC_NUMBER].paraLen);
        }
    }

    if (g_atParaList[AT_CCFC_TYPE].paraLen) {
        *numType = (TAF_SS_NUMTYPE)g_atParaList[AT_CCFC_TYPE].paraValue;
    } else {
        *numType = At_GetCodeType(g_atParaList[AT_CCFC_NUMBER].para[0]);
    }
}

VOS_VOID AT_SetSubAddrAttribute(TAF_SS_SUBADDRTYPE *subAddrType, TAF_UINT8 *fwdToSubAddr)
{
    errno_t memResult;
    if (g_atParaList[AT_CCFC_SUBADDR].paraLen > 0) {
        memResult = memcpy_s((TAF_CHAR *)fwdToSubAddr, TAF_SS_MAX_FORWARDED_TO_SUBADDRESS_LEN + 1,
                             (TAF_CHAR *)g_atParaList[AT_CCFC_SUBADDR].para,
                             g_atParaList[AT_CCFC_SUBADDR].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, TAF_SS_MAX_FORWARDED_TO_SUBADDRESS_LEN + 1,
                            g_atParaList[AT_CCFC_SUBADDR].paraLen);
    }

    if (g_atParaList[AT_CCFC_SATTYPE].paraLen) {
        *subAddrType = (TAF_UINT8)g_atParaList[AT_CCFC_SATTYPE].paraValue;
    } else {
        *subAddrType = AT_CCFC_CLASS_DEDICATED_PAD_ACCESS;
    }
}

VOS_UINT32 AT_FillSsNoRepCondTime(VOS_UINT8 ssCode, AT_ParseParaType *atPara, TAF_SS_RegisterssReq *sSPara)
{
    if ((ssCode == TAF_ALL_FORWARDING_SS_CODE) || (ssCode == TAF_ALL_COND_FORWARDING_SS_CODE) ||
        (ssCode == TAF_CFNRY_SS_CODE)) {
        if (atPara->paraLen) {
            sSPara->opNoRepCondTime = 1;
            sSPara->noRepCondTime   = (TAF_SS_NOREPLYCONDITIONTIME)atPara->paraValue;
        } else {
            /* 默认值 */
            sSPara->opNoRepCondTime = 1;
            sSPara->noRepCondTime   = AT_NOREPCONDTIME_DEFAULT;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_DeactivateSSReq(TAF_UINT8 indexNum, TAF_SS_ErasessReq *eraseReq)
{
    if (TAF_DeactivateSSReq(g_atClientTab[indexNum].clientId, 0, eraseReq) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCFC_DISABLE;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_ActivateSSReq(TAF_UINT8 indexNum, TAF_SS_ErasessReq *activateReq)
{
    if (TAF_ActivateSSReq(g_atClientTab[indexNum].clientId, 0, activateReq) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCFC_ENABLE;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_InterrogateSSReq(TAF_UINT8 indexNum, TAF_SS_ErasessReq *interrogateReq)
{
    g_atClientTab[indexNum].temp = g_atParaList[AT_CCFC_CLASS].paraValue;

    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, interrogateReq) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCFC_QUERY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_RegisterSSReq(TAF_UINT8 indexNum, TAF_SS_RegisterssReq *registerReq)
{
    if (TAF_RegisterSSReq(g_atClientTab[indexNum].clientId, 0, registerReq) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCFC_REGISTRATION;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_EraseSSReq(TAF_UINT8 indexNum, TAF_SS_ErasessReq *eraseSSReq)
{
    if (TAF_EraseSSReq(g_atClientTab[indexNum].clientId, 0, eraseSSReq) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCFC_ERASURE;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

TAF_UINT32 At_SetCcfcPara(TAF_UINT8 indexNum)
{
    TAF_SS_RegisterssReq para;
    TAF_SS_ErasessReq    sndpara;
    VOS_UINT32           ret;

    ret = AT_CheckCcfcParaValid();
    if (ret != AT_OK) {
        return ret;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));
    (VOS_VOID)memset_s(&sndpara, sizeof(sndpara), 0x00, sizeof(sndpara));

    para.ssCode = AT_GetSsCodeFromCfccPara();

    /* 设置<number>,<type> */
    if (g_atParaList[AT_CCFC_NUMBER].paraLen != 0) {
        para.opFwdToNum = 1;
        para.opNumType  = 1;
        AT_SetNumAttribute(&para.numType, para.fwdToNum);
    } else {
        para.opFwdToNum = 0;
        para.opNumType  = 0;
    }

    /* 设置<subaddr>,<satype> */
    if (g_atParaList[AT_CCFC_SUBADDR].paraLen != 0) {
        para.opFwdToAddr   = 1;
        para.opSubAddrType = 1;
        AT_SetSubAddrAttribute(&(para.subAddrType), para.fwdToSubAddr);
    } else {
        para.opFwdToAddr   = 0;
        para.opSubAddrType = 0;
    }

    /* 设置<time> */
    AT_FillSsNoRepCondTime(para.ssCode, &g_atParaList[AT_CCFC_TIME], &para);

    /* 设置<class> */
    if (AT_FillSsBsService(&sndpara, &g_atParaList[AT_CCFC_CLASS]) != VOS_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    sndpara.ssCode = para.ssCode;

    if (sndpara.opBsService == VOS_TRUE) {
        para.opBsService             = sndpara.opBsService;
        para.bsService.bsType        = sndpara.bsService.bsType;
        para.bsService.bsServiceCode = sndpara.bsService.bsServiceCode;
    }

    /* 执行命令操作 */
    switch (g_atParaList[1].paraValue) {
        case 0: /* <mode>: disable */
            ret = AT_DeactivateSSReq(indexNum, &sndpara);
            break;

        case 1: /* <mode>: enable */
            ret = AT_ActivateSSReq(indexNum, &sndpara);
            break;

        case 2: /* <mode>: query status */
            ret = AT_InterrogateSSReq(indexNum, &sndpara);
            break;

        case 3: /* <mode>: registration */
            ret = AT_RegisterSSReq(indexNum, &para);
            break;

        default: /* <mode>: erasure */
            ret = AT_EraseSSReq(indexNum, &sndpara);
            break;
    }

    return ret;
}

TAF_UINT32 At_CheckUssdNumLen(VOS_UINT8 indexNum, TAF_SS_DATA_CODING_SCHEME dcs, TAF_UINT16 len)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (ssCtx->ussdTransMode == AT_USSD_NON_TRAN_MODE) {
        if (len > TAF_SS_MAX_USSDSTRING_LEN) {
            return AT_FAILURE;
        }
    } else {
        if (len > (TAF_SS_MAX_USS_CHAR * AT_DOUBLE_LENGTH)) { /* USSD字符串，最大长度为160个字符 */
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_CheckCusdPara(TAF_SS_DATA_CODING_SCHEME *datacodingScheme, VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CUSD_PARA_MAX_NUM) {
        return VOS_FALSE;
    }

    if (g_atParaList[AT_CUSD_DCS].paraLen == 0) {
        /* 默认为7bit编码 */
        *datacodingScheme = TAF_SS_7bit_LANGUAGE_UNSPECIFIED;
    } else {
        *datacodingScheme = (TAF_SS_DATA_CODING_SCHEME)g_atParaList[AT_CUSD_DCS].paraValue;

        /* 产品线要求支持uss2的ussd */
    }

    /* 如果<str>号码过长，直接返回错误 */
    if (At_CheckUssdNumLen(indexNum, *datacodingScheme, g_atParaList[AT_CUSD_STR].paraLen) == AT_FAILURE) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_ExcuteCusdReq(TAF_SS_ProcessUssReq *para, VOS_UINT8 indexNum)
{
    VOS_UINT32                ret;

    if (TAF_ProcessUnstructuredSSReq(g_atClientTab[indexNum].clientId, 0, para) == AT_SUCCESS) {
        /*  AT+CUSD=0时，也需要给C核发消息，但不用等C核回复 */
        if (g_atParaList[AT_CUSD_STR].paraLen == 0) {
            ret = AT_OK;
        } else {
            /* 设置当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CUSD_REQ;

            /* 返回命令处理挂起状态 */
            ret = AT_WAIT_ASYNC_RETURN;
        }
    } else {
        ret = AT_ERROR;
    }

    return ret;
}

TAF_UINT32 At_UnicodePrintToAscii(TAF_UINT8 *data, TAF_UINT16 *len)
{
    TAF_UINT8 *check      = data;
    TAF_UINT8 *write      = data;
    TAF_UINT8 *read       = data;
    TAF_UINT16 lenTemp    = 0;
    TAF_UINT16 chkLen     = 0;
    TAF_UINT8  firstByte  = 0;
    TAF_UINT8  secondByte = 0;
    TAF_UINT8  high       = 0;
    TAF_UINT8  low        = 0;

    /* 字符均为'0'-'9','a'-'f','A'-'F' */
    while (chkLen++ < *len) {
        if ((*check >= '0') && (*check <= '9')) {
            *check = *check - '0';
        } else if ((*check >= 'a') && (*check <= 'f')) {
            *check = (*check - 'a') + 10; /* 十六进制格式转换 */
        } else if ((*check >= 'A') && (*check <= 'F')) {
            *check = (*check - 'A') + 10; /* 十六进制格式转换 */
        } else {
            return AT_FAILURE;
        }
        check++;
    }

    while (lenTemp < *len) {
        /* 判断结尾 */
        firstByte = *read++; /* 前两位必须为0 */
        lenTemp++;
        secondByte = *read++;
        lenTemp++;

        if ((firstByte != 0) || (secondByte != 0)) {
            return AT_FAILURE;
        }

        high = *read++; /* 高位 */
        lenTemp++;
        low = *read++; /* 低位 */
        lenTemp++;

        *write++ = (TAF_UINT8)(high * 16) + low; /* 写入Unicode的一个字节 */
    }

    *len = lenTemp >> 2;
    return AT_SUCCESS;
}

VOS_UINT32 At_ProcAppReqUssdStr_Nontrans(TAF_SS_UssdString *ussdStr, TAF_SS_DATA_CODING_SCHEME datacodingScheme)
{
    TAF_UINT8        ussdStrInfo[TAF_SS_MAX_USSDSTRING_LEN * 2];
    MN_MSG_CbdcsCode dcsInfo;
    VOS_UINT32       asciiStrLen;
    VOS_UINT32       ret;

    (VOS_VOID)memset_s(ussdStrInfo, sizeof(ussdStrInfo), 0, sizeof(ussdStrInfo));
    (VOS_VOID)memset_s(&dcsInfo, sizeof(dcsInfo), 0, sizeof(dcsInfo));

    /* USSD与CBS的DCS的协议相同，调用CBS的DCS解析函数解码，详细情况参考23038 */
    ret = MN_MSG_DecodeCbsDcs(datacodingScheme, ussdStr->ussdStr, ussdStr->cnt, &dcsInfo);

    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("At_ProcAppReqUssdStr_Nontrans:WARNING: Decode Failure");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (dcsInfo.msgCoding == MN_MSG_MSG_CODING_UCS2) {
        /* 先进行DCS:UCS2的处理 */
        ret = At_UnicodePrint2Unicode(ussdStr->ussdStr, &(ussdStr->cnt));

        if (ret != AT_SUCCESS) {
            AT_WARN_LOG("At_ProcAppReqUssdStr_Nontrans: convert Unicode fail");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        /* 7Bit8Bit在非透传模式下先做CSCS的处理 */
        if (g_atCscsType == AT_CSCS_UCS2_CODE) {
            ret = At_UnicodePrintToAscii(ussdStr->ussdStr, &(ussdStr->cnt));

            if (ret != AT_SUCCESS) {
                AT_WARN_LOG("At_ProcAppReqUssdStr_Nontrans : CSCS Proc failed");
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }

        /* 再将7Bit做压缩编码处理 */
        if (dcsInfo.msgCoding == MN_MSG_MSG_CODING_7_BIT) {
            asciiStrLen = 0;
            /* TAF_SS_MAX_USSDSTRING_LEN * 2 为数组ussdStrInfo的最大长度 */
            ret         = TAF_STD_ConvertAsciiToDefAlpha(ussdStr->ussdStr, ussdStr->cnt, ussdStrInfo, &asciiStrLen,
                                                         TAF_SS_MAX_USSDSTRING_LEN * 2);
            ussdStr->cnt = (VOS_UINT16)asciiStrLen;
            if (ret != MN_ERR_NO_ERROR) {
                AT_WARN_LOG("At_ProcAppReqUssdStr_Nontrans : Conv7Bit Def Err");
                return AT_CME_INCORRECT_PARAMETERS;
            }

            /* 清除原码流残余信息 */
            /* TAF_SS_MAX_USSDSTRING_LEN * 2 为数组ussdStrInfo的最大长度 */
            (VOS_VOID)memset_s(ussdStr->ussdStr, TAF_SS_MAX_USSDSTRING_LEN * 2 * sizeof(TAF_UINT8), 0,
                               TAF_SS_MAX_USSDSTRING_LEN * 2 * sizeof(TAF_UINT8));

            ret = TAF_STD_Pack7Bit(ussdStrInfo, ussdStr->cnt, 0, ussdStr->ussdStr, &asciiStrLen);
            ussdStr->cnt = (VOS_UINT16)asciiStrLen;
            if (ret != MN_ERR_NO_ERROR) {
                AT_WARN_LOG("At_ProcAppReqUssdStr_Nontrans : Conv7Bit pack Err");
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_ConvUssdStrInCmd(TAF_SS_UssdString *ussdStr, TAF_SS_DATA_CODING_SCHEME datacodingScheme,
                               VOS_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;
    VOS_UINT32     ret;
    errno_t        memResult;

    if ((g_atParaList[AT_USSD_STRING_CNT].paraLen > 0) &&
        (g_atParaList[AT_USSD_STRING_CNT].paraLen <= sizeof(ussdStr->ussdStr))) {
        memResult = memcpy_s(ussdStr->ussdStr, sizeof(ussdStr->ussdStr), g_atParaList[AT_USSD_STRING_CNT].para,
                             g_atParaList[AT_USSD_STRING_CNT].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(ussdStr->ussdStr), g_atParaList[AT_USSD_STRING_CNT].paraLen);
    }
    ussdStr->cnt = g_atParaList[AT_USSD_STRING_CNT].paraLen;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (ssCtx->ussdTransMode == AT_USSD_NON_TRAN_MODE) {
        /* 非透传模式处理下发请求 */
        ret = At_ProcAppReqUssdStr_Nontrans(ussdStr, datacodingScheme);
    } else {
        /* 透传模式 */
        ret = At_AsciiNum2HexString(ussdStr->ussdStr, &(ussdStr->cnt));
    }

    if (ret != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 At_SetCusdPara(VOS_UINT8 indexNum)
{
    TAF_SS_ProcessUssReq     *para = VOS_NULL_PTR;
    VOS_UINT32                ret;
    TAF_SS_DATA_CODING_SCHEME datacodingScheme;
    VOS_UINT32                procRet;

    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    /* 获取SS上下文 */
    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    ret = AT_ERROR;

    if (AT_CheckCusdPara(&datacodingScheme, indexNum) != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*lint -save -e516 */
    para = (TAF_SS_ProcessUssReq *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_SS_ProcessUssReq));
    /*lint -restore */
    if (para == TAF_NULL_PTR) {
        return AT_ERROR;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(para, sizeof(TAF_SS_ProcessUssReq), 0x00, sizeof(TAF_SS_ProcessUssReq));

    /* 设置<n> */
    if (g_atParaList[AT_CUSD_N].paraLen != 0) {
        if (g_atParaList[AT_CUSD_N].paraValue != AT_CUSD_N_EXIT_SESSION) {
            para->cusdType = (TAF_SS_CusdTypeUint8)g_atParaList[AT_CUSD_N].paraValue;
        }
    } else {
        para->cusdType = TAF_SS_CUSD_TYPE_DISABLE;
    }

    /* <n>: cancel session */
    if (g_atParaList[AT_CUSD_N].paraValue == AT_CUSD_N_CANCEL_SESSION_VALUE) {
        if (TAF_SsReleaseComplete(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
            ret = AT_OK;

        } else {
            ret = AT_ERROR;
        }
    } else {
        /*  AT+CUSD=0时，也需要给C核发消息，但不用等C核回复 */
        if (g_atParaList[AT_CUSD_STR].paraLen == 0) {
            (VOS_VOID)memset_s(&(para->ussdStr), sizeof(para->ussdStr), 0x00, sizeof(para->ussdStr));
        } else {
            procRet = At_ConvUssdStrInCmd(&(para->ussdStr), datacodingScheme, indexNum);
            if (procRet != AT_OK) {
                /*lint -save -e516 */
                PS_MEM_FREE(WUEPS_PID_AT, para);
                /*lint -restore */
                return procRet;
            }
        }
        /* 设置<dcs> */
        para->datacodingScheme = datacodingScheme;

        para->ussdTransMode = ssCtx->ussdTransMode;

        /* 执行命令操作 */
        ret = AT_ExcuteCusdReq(para, indexNum);
    }
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, para);
    /*lint -restore */
    return ret;
}

TAF_UINT32 At_SetCtfrPara(TAF_UINT8 indexNum)
{
    MN_CALL_SupsParam callSupsParam;
    VOS_UINT32        rst;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    /* 参数检查 */
    if (g_atParaList[AT_CTFR_NUMBER].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CTFR_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果<number>过长，直接返回错误 */
    if (g_atParaList[AT_CTFR_NUMBER].paraLen > MN_CALL_MAX_ASCII_NUM_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果<subaddr>过长，直接返回错误 */
    if (g_atParaList[AT_CTFR_SUBADDR].paraLen > MN_CALL_MAX_ASCII_NUM_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&callSupsParam, sizeof(callSupsParam), 0x00, sizeof(callSupsParam));

    g_atParaList[AT_CTFR_NUMBER].para[g_atParaList[AT_CTFR_NUMBER].paraLen] = '\0';

    rst = AT_AsciiNumberToBcd((VOS_CHAR *)g_atParaList[AT_CTFR_NUMBER].para, callSupsParam.redirectNum.bcdNum,
                              &callSupsParam.redirectNum.numLen);
    if (rst != MN_ERR_NO_ERROR) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    callSupsParam.callId              = 0;
    callSupsParam.callSupsCmd         = MN_CALL_SUPS_CMD_DEFLECT_CALL;
    callSupsParam.redirectNum.numType = (TAF_UINT8)g_atParaList[1].paraValue;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送补充业务请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &callSupsParam, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(callSupsParam), modemId) ==
        VOS_OK) {
        /* 停止自动接听 */
        ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            AT_StopRelTimer(ccCtx->s0TimeInfo.timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CTFR_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetChldExPara(VOS_UINT8 indexNum)
{
    MN_CALL_SupsParam callChldParam;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    TAF_Ctrl      ctrl;
    VOS_UINT32    rst;
    ModemIdUint16 modemId;

    /* 参数过多 */
    if (g_atParaIndex > AT_CHLD_MAX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 只有增强型多方通话踢人的场景下，才有第3个参数，即chld=6,0,"135xxxxxxxx" */
    if ((g_atParaList[AT_CHLD_CALL_NUM].paraLen != 0) && (g_atParaList[AT_CHLD_OP_CODE].paraValue != 6)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((g_atParaList[AT_CHLD_OP_CODE].paraLen == 0) || (g_atParaList[AT_CHLD_CALL_TYPE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&callChldParam, sizeof(callChldParam), 0x00, sizeof(callChldParam));

    callChldParam.callType = (MN_CALL_TypeUint8)g_atParaList[AT_CHLD_CALL_TYPE].paraValue;

    rst = At_ParseChldPara(g_atParaList[AT_CHLD_OP_CODE].paraValue, &callChldParam);
    if (rst == AT_ERROR) {
        return AT_ERROR;
    }

    /* 如果是需要释放多方通话中成员，需要携带电话号码 */
    if (callChldParam.callSupsCmd == MN_CALL_SUPS_CMD_ECONF_REL_USER) {
        /* 没有携带号码 */
        if (g_atParaList[AT_CHLD_CALL_NUM].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 检查并转换电话号码 */
        if (AT_FillCalledNumPara((VOS_UINT8 *)g_atParaList[AT_CHLD_CALL_NUM].para,
                                 g_atParaList[AT_CHLD_CALL_NUM].paraLen,
                                 &callChldParam.removeNum) != VOS_OK) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    callChldParam.rttFlg = (VOS_UINT8)g_atParaList[AT_CHLD_RTT_FLG].paraValue;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送补充业务请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &callChldParam, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(callChldParam), modemId) ==
        VOS_OK) {
        /* 停止自动接听 */
        ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            AT_StopRelTimer(ccCtx->s0TimeInfo.timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CHLD_EX_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCnapPara(VOS_UINT8 indexNum)
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
        ssCtx->cnapType = (AT_CnapTypeUint8)g_atParaList[0].paraValue;
    } else {
        ssCtx->cnapType = AT_CNAP_DISABLE_TYPE;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetAlsPara(TAF_UINT8 indexNum)
{
    VOS_UINT32          ret;
    AT_ModemSsCtx      *ssCtx = VOS_NULL_PTR;
    TAF_Ctrl            ctrl;
    MN_CALL_SetAlsParam setAls;
    ModemIdUint16       modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&setAls, sizeof(setAls), 0x00, sizeof(setAls));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 参数过多 */
    if (g_atParaIndex > AT_ALS_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数不存在 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[1].paraLen != 0) {
        setAls.alsLine = (MN_CALL_AlsLineNoUint8)g_atParaList[1].paraValue;

        if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
            return AT_ERROR;
        }

        ret = TAF_CCM_CallCommonReq(&ctrl, &setAls, ID_TAF_CCM_SET_ALS_REQ, sizeof(setAls), modemId);
        if (ret != VOS_OK) {
            return AT_ERROR;
        }

        ssCtx->salsType = (TAF_UINT8)g_atParaList[0].paraValue;

        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APP_SET_ALS_REQ;

        return AT_WAIT_ASYNC_RETURN;
    }

    ssCtx->salsType = (TAF_UINT8)g_atParaList[0].paraValue;

    return AT_OK;
}

TAF_UINT32 At_SetUssdModePara(TAF_UINT8 indexNum)
{
    USSD_TRANS_Mode ussdTranMode;
    AT_ModemSsCtx  *ssCtx = VOS_NULL_PTR;

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

    if ((g_atParaList[0].paraValue != 1) && (g_atParaList[0].paraValue != 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 更新全局变量 */
    ssCtx                = AT_GetModemSsCtxAddrFromClientId(indexNum);
    ssCtx->ussdTransMode = (VOS_UINT16)g_atParaList[0].paraValue;

    ussdTranMode.status        = VOS_TRUE;
    ussdTranMode.ussdTransMode = (VOS_UINT8)ssCtx->ussdTransMode;

    /* 设置完拨号模式后，给C核发送消息 */
    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, TAF_MSG_SET_USSDMODE_MSG,
                               (VOS_UINT8 *)&ussdTranMode, sizeof(USSD_TRANS_Mode), I0_WUEPS_PID_TAF) != TAF_SUCCESS) {
        return AT_ERROR;
    }

    return AT_OK;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetCallModifyInitPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl               ctrl;
    TAF_CCM_CallModifyPara callMdfPara;
    VOS_UINT32             result;
    ModemIdUint16          modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(TAF_Ctrl));
    (VOS_VOID)memset_s(&callMdfPara, sizeof(callMdfPara), 0x00, sizeof(callMdfPara));

    /* 参数检查 */
    if (g_atParaIndex != AT_CALLMODIFYINIT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_CALLMODIFYINIT_IDX].paraLen == 0) ||
        (g_atParaList[AT_CALLMODIFYINIT_CURR_CALL_TYPE].paraLen == 0) ||
        (g_atParaList[AT_CALLMODIFYINIT_DEST_CALL_TYPE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    callMdfPara.callId         = (MN_CALL_ID_T)g_atParaList[AT_CALLMODIFYINIT_IDX].paraValue;
    callMdfPara.currCallType   = (MN_CALL_TypeUint8 )g_atParaList[AT_CALLMODIFYINIT_CURR_CALL_TYPE].paraValue;
    callMdfPara.expectCallType = (MN_CALL_TypeUint8 )g_atParaList[AT_CALLMODIFYINIT_DEST_CALL_TYPE].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    result = TAF_CCM_CallCommonReq(&ctrl, &callMdfPara, ID_TAF_CCM_CALL_MODIFY_REQ, sizeof(callMdfPara), modemId);

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CALL_MODIFY_INIT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetCallModifyAnsPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl                           ctrl;
    TAF_CCM_CallAnswerRemoteModifyPara ansRemoteMdfPara;
    VOS_UINT32                         result;
    ModemIdUint16                      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&ansRemoteMdfPara, sizeof(ansRemoteMdfPara), 0x00, sizeof(ansRemoteMdfPara));

    /* 参数检查 */
    if (g_atParaIndex != AT_CALLMODIFYANS_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_CALLMODIFYANS_IDX].paraLen == 0) ||
        (g_atParaList[AT_CALLMODIFYANS_CURR_CALL_TYPE].paraLen == 0) ||
        (g_atParaList[AT_CALLMODIFYANS_DEST_CALL_TYPE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    ansRemoteMdfPara.callId         = (MN_CALL_ID_T)g_atParaList[AT_CALLMODIFYANS_IDX].paraValue;
    ansRemoteMdfPara.currCallType   = (MN_CALL_TypeUint8 )g_atParaList[AT_CALLMODIFYANS_CURR_CALL_TYPE].paraValue;
    ansRemoteMdfPara.expectCallType = (MN_CALL_TypeUint8 )g_atParaList[AT_CALLMODIFYANS_DEST_CALL_TYPE].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    result = TAF_CCM_CallCommonReq(&ctrl, &ansRemoteMdfPara, ID_TAF_CCM_CALL_ANSWER_REMOTE_MODIFY_REQ,
                                   sizeof(ansRemoteMdfPara), modemId);

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CALL_MODIFY_ANS_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_ParseImsvtcapcfgPara(TAF_MMA_ImsVideoCallCap *imsVtCap)
{
    switch (g_atParaList[0].paraValue) {
        case AT_VIDEO_CALL_CAP_SWITCH:
            imsVtCap->videoCallCapType  = TAF_MMA_IMS_VIDEO_CALL_CAP_SWITCH;
            imsVtCap->videoCallCapValue = g_atParaList[1].paraValue;
            return VOS_TRUE;

        case AT_VIDEO_CALL_CAP_CCWA:
            imsVtCap->videoCallCapType  = TAF_MMA_IMS_VIDEO_CALL_CAP_CCWA;
            imsVtCap->videoCallCapValue = g_atParaList[1].paraValue;
            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}

VOS_UINT32 AT_SetImsVtCapCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32              rst;
    TAF_MMA_ImsVideoCallCap imsVtCap;

    /* 初始化结构体 */
    (VOS_VOID)memset_s(&imsVtCap, sizeof(imsVtCap), 0x00, sizeof(imsVtCap));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_IMSVTCAPCFG_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数值不对 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_ParseImsvtcapcfgPara(&imsVtCap) != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    rst = TAF_MMA_SetImsVtCapCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &imsVtCap);

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSVTCAPCFG_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetImsVideoCallCancelPara(TAF_UINT8 indexNum)
{
    TAF_Ctrl                  ctrl;
    VOS_UINT32                callId;
    VOS_UINT32                result;
    ModemIdUint16             modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetImsVideoCallCancelPara: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetImsVideoCallCancelPara: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    callId        = g_atParaList[0].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 给CCM发送^IMSVIDEOCALLCANCEL查询请求 */
    result = TAF_CCM_CallCommonReq(&ctrl, &callId, ID_TAF_CCM_CANCEL_ADD_VIDEO_REQ, sizeof(callId), modemId);

    if (result != VOS_OK) {
        AT_WARN_LOG("AT_SetImsVideoCallCancelPara: TAF_CCM_CallCommonReq is error!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSVIDEOCALLCANCEL_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_VOID AT_FillCmolrParaEnable(TAF_SSA_LcsMolrParaSet *molrPara)
{
    /* <enable> */
    if ((g_atParaIndex == 0) || (g_atParaList[0].paraLen == 0)) {
        molrPara->enable = TAF_SSA_LCS_MOLR_ENABLE_TYPE_DISABLE;
    } else {
        molrPara->enable = (VOS_UINT8)g_atParaList[0].paraValue;
    }
}

VOS_VOID AT_FillCmolrParaMethod(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= 1) || (g_atParaList[1].paraLen == 0)) {
        return;
    }

    /* <method> */
    molrPara->opMethod = VOS_TRUE;
    molrPara->method   = (VOS_UINT8)g_atParaList[1].paraValue;
}

VOS_UINT32 AT_FillCmolrParaHorAcc(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_HOR_ACC_SET_INDEX) || (g_atParaList[AT_CMOLR_HOR_ACC_SET].paraLen == 0)) {
        return VOS_OK;
    }

    /* <hor-acc-set> */
    molrPara->opHorAccSet = VOS_TRUE;
    molrPara->horAccSet   = (VOS_UINT8)g_atParaList[AT_CMOLR_HOR_ACC_SET].paraValue;

    if (molrPara->horAccSet == LCS_HOR_ACC_SET_PARAM) {
        /* <hor-acc> */
        /* 要求水平精度，但没有下发水平精度系数，返回失败 */
        if ((g_atParaIndex <= AT_CMOLR_HOR_ACC_INDEX) || (g_atParaList[AT_CMOLR_HOR_ACC].paraLen == 0)) {
            AT_WARN_LOG("AT_FillCmolrParaPartI: <hor-acc> is required!");
            return VOS_ERR;
        }

        molrPara->horAcc = (VOS_UINT8)g_atParaList[AT_CMOLR_HOR_ACC].paraValue;
    }

    return VOS_OK;
}

VOS_UINT32 AT_FillCmolrParaVerReq(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_VER_REQ_INDEX) || (g_atParaList[AT_CMOLR_VER_REQ].paraLen == 0)) {
        return VOS_OK;
    }

    /* <ver-req> */
    molrPara->opVerReq = VOS_TRUE;
    molrPara->verReq   = (VOS_UINT8)g_atParaList[AT_CMOLR_VER_REQ].paraValue;

    if ((molrPara->verReq != LCS_VER_REQUESTED) ||
        (g_atParaIndex <= AT_CMOLR_VER_ACC_SET_INDEX) || (g_atParaList[AT_CMOLR_VER_ACC_SET].paraLen == 0)) {
        return VOS_OK;
    }

    /* <ver-acc-set> */
    molrPara->opVerAccSet = VOS_TRUE;
    molrPara->verAccSet   = (VOS_UINT8)g_atParaList[AT_CMOLR_VER_ACC_SET].paraValue;

    if (molrPara->verAccSet == LCS_VER_ACC_SET_PARAM) {
        /* <ver-acc> */
        /* 要求垂直精度，但没有下发垂直精度系数，返回失败 */
        if ((g_atParaIndex <= AT_CMOLR_VER_ACC_INDEX) || (g_atParaList[AT_CMOLR_VER_ACC].paraLen == 0)) {
            AT_WARN_LOG("AT_FillCmolrParaVerReq: <ver-acc> is required!");
            return VOS_ERR;
        }

        molrPara->verAcc = (VOS_UINT8)g_atParaList[AT_CMOLR_VER_ACC].paraValue;
    }

    return VOS_OK;
}

VOS_VOID AT_FillCmolrParaVelReq(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_VEL_REQ_INDEX) || (g_atParaList[AT_CMOLR_VEL_REQ].paraLen == 0)) {
        return;
    }

    /* <vel-req> */
    molrPara->opVelReq = VOS_TRUE;
    molrPara->velReq   = (VOS_UINT8)g_atParaList[AT_CMOLR_VEL_REQ].paraValue;
}

VOS_UINT32 AT_FillCmolrParaRepMode(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_REP_MODE_INDEX) || (g_atParaList[AT_CMOLR_REP_MODE].paraLen == 0)) {
        return VOS_OK;
    }

    /* <rep-mode> */
    molrPara->opRepMode = VOS_TRUE;
    molrPara->repMode   = (VOS_UINT8)g_atParaList[AT_CMOLR_REP_MODE].paraValue;

    if (g_atParaIndex <= AT_CMOLR_TIMEOUT_INDEX) {
        return VOS_OK;
    }

    /* <timeout> */
    if (g_atParaList[AT_CMOLR_TIMEOUT].paraLen != 0) {
        molrPara->opTimeout = VOS_TRUE;
        molrPara->timeOut   = (VOS_UINT16)g_atParaList[AT_CMOLR_TIMEOUT].paraValue;
    }

    /* <interval> */
    if (molrPara->repMode == LCS_REP_MODE_PERIODIC_RPT) {
        if ((g_atParaIndex > AT_CMOLR_INTERVAL_INDEX) && (g_atParaList[AT_CMOLR_INTERVAL].paraLen != 0)) {
            molrPara->opInterval = VOS_TRUE;
            molrPara->interval   = (VOS_UINT16)g_atParaList[AT_CMOLR_INTERVAL].paraValue;
        }
    }

    if ((molrPara->opTimeout == VOS_TRUE) && (molrPara->opInterval == VOS_TRUE) &&
        (molrPara->interval < molrPara->timeOut)) {
        AT_WARN_LOG("AT_FillCmolrParaPartI: <interval> is too short!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID AT_FillCmolrParaShapeRep(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_SHAPE_REP_INDEX) || (g_atParaList[AT_CMOLR_SHAPE_REP].paraLen == 0)) {
        return;
    }

    /* <shape-rep> */
    molrPara->opShapeRep = VOS_TRUE;
    molrPara->shapeRep   = (VOS_UINT8)g_atParaList[AT_CMOLR_SHAPE_REP].paraValue;
}

VOS_VOID AT_FillCmolrParaPlane(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_PLANE_INDEX) || (g_atParaList[AT_CMOLR_PLANE].paraLen == 0)) {
        return;
    }

    /* <plane> */
    molrPara->opPlane = VOS_TRUE;
    molrPara->plane   = (VOS_UINT8)g_atParaList[AT_CMOLR_PLANE].paraValue;
}

VOS_UINT32 AT_Str2NmeaRep(VOS_UINT16 length, VOS_CHAR *pcStr, TAF_SSA_LcsNmeaRep *nmeaRep)
{
    errno_t    memResult;
    VOS_UINT32 num;
    VOS_UINT32 loop;
    VOS_CHAR  *pcTemp = VOS_NULL_PTR;
    VOS_CHAR   srcStr[AT_PARA_NMEA_MAX_LEN + 1];

    (VOS_VOID)memset_s(srcStr, sizeof(srcStr), 0x00, sizeof(srcStr));
    (VOS_VOID)memset_s(nmeaRep, sizeof(TAF_SSA_LcsNmeaRep), 0x00, sizeof(TAF_SSA_LcsNmeaRep));

    if (length > AT_PARA_NMEA_MAX_LEN) {
        AT_WARN_LOG("AT_Str2NmeaRep: NMEA format string is too long!");
        return VOS_ERR;
    }

    num = ((VOS_UINT32)length + 1) / (AT_PARA_NMEA_MIN_LEN + 1);

    if (num == 0) {
        AT_WARN_LOG("AT_Str2NmeaRep: NMEA format string is too short!");
        return VOS_ERR;
    }

    if (length > 0) {
        memResult = memcpy_s(srcStr, sizeof(srcStr), pcStr, length);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(srcStr), length);
    }
    pcTemp = srcStr;

    for (loop = 0; loop < num; loop++) {
        /* $GPGGA */
        if (VOS_MemCmp(pcTemp, AT_PARA_NMEA_GPGGA, AT_PARA_NMEA_MIN_LEN) == 0) {
            nmeaRep->gpgga = VOS_TRUE;
        }
        /* $GPRMC */
        else if (VOS_MemCmp(pcTemp, AT_PARA_NMEA_GPRMC, AT_PARA_NMEA_MIN_LEN) == 0) {
            nmeaRep->gprmc = VOS_TRUE;
        }
        /* $GPGLL */
        else if (VOS_MemCmp(pcTemp, AT_PARA_NMEA_GPGLL, AT_PARA_NMEA_MIN_LEN) == 0) {
            nmeaRep->gpgll = VOS_TRUE;
        }
        else {
            AT_WARN_LOG("AT_Str2NmeaRep: NMEA format string is incorrect!");
            return VOS_ERR;
        }

        pcTemp += (AT_PARA_NMEA_MIN_LEN + 1);
    }

    return VOS_OK;
}

VOS_UINT32 AT_FillCmolrParaNmeaRep(TAF_SSA_LcsMolrParaSet *molrPara)
{
    if ((g_atParaIndex <= AT_CMOLR_NMEA_REP_INDEX) || (g_atParaList[AT_CMOLR_NMEA_REP].paraLen == 0)) {
        return VOS_OK;
    }

    /* <NMEA-rep> */
    if ((molrPara->enable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA) ||
        (molrPara->enable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA_GAD)) {
        molrPara->opNmeaRep = VOS_TRUE;
        if (AT_Str2NmeaRep(g_atParaList[AT_CMOLR_NMEA_REP].paraLen, (VOS_CHAR *)g_atParaList[AT_CMOLR_NMEA_REP].para,
                           &(molrPara->nmeaRep)) != VOS_OK) {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_FillCmolrParaThdPtyAddr(TAF_SSA_LcsMolrParaSet *molrPara)
{
    errno_t memResult;
    if ((g_atParaIndex <= AT_CMOLR_THIRD_PARTY_ADDRESS_INDEX) ||
        (g_atParaList[AT_CMOLR_THIRD_PARTY_ADDRESS].paraLen == 0)) {
        return VOS_OK;
    }

    /* <third-party-address> */
    if (g_atParaList[AT_CMOLR_THIRD_PARTY_ADDRESS].paraLen > TAF_SSA_LCS_THIRD_PARTY_ADDR_MAX_LEN) {
        AT_WARN_LOG("AT_FillCmolrParaPartIII: <third-party-address> is too long!");
        return VOS_ERR;
    }

    molrPara->opThirdPartyAddr = VOS_TRUE;
    memResult                  = memcpy_s(molrPara->thirdPartyAddr, sizeof(molrPara->thirdPartyAddr),
                                          (VOS_CHAR *)g_atParaList[AT_CMOLR_THIRD_PARTY_ADDRESS].para,
                                          g_atParaList[AT_CMOLR_THIRD_PARTY_ADDRESS].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(molrPara->thirdPartyAddr),
                        g_atParaList[AT_CMOLR_THIRD_PARTY_ADDRESS].paraLen);

    return VOS_OK;
}

VOS_UINT32 AT_FillCmolrPara(TAF_SSA_LcsMolrParaSet *molrPara)
{
    (VOS_VOID)memset_s(molrPara, sizeof(TAF_SSA_LcsMolrParaSet), 0x00, sizeof(TAF_SSA_LcsMolrParaSet));

    /*
     * +CMOLR=[<enable>[,<method>[,<hor-acc-set>[,<hor-acc>[,<ver-req>
     * [,<ver-acc-set>[,<ver-acc>[,<vel-req> [,<rep-mode>[,<timeout>
     * [,<interval>[,<shape-rep>[,<plane>[,<NMEA-rep>
     * [,<third-party-address>]]]]]]]]]]]]]]]]
     */

    /* 参数过多，返回失败 */
    if (g_atParaIndex > AT_CMOLR_MAX_PARA_NUM) {
        AT_WARN_LOG("AT_FillCmolrPara: too many parameters!");
        return VOS_ERR;
    }

    AT_FillCmolrParaEnable(molrPara);

    AT_FillCmolrParaMethod(molrPara);

    if (AT_FillCmolrParaHorAcc(molrPara) != VOS_OK) {
        return VOS_ERR;
    }

    if (AT_FillCmolrParaVerReq(molrPara) != VOS_OK) {
        return VOS_ERR;
    }

    AT_FillCmolrParaVelReq(molrPara);

    if (AT_FillCmolrParaRepMode(molrPara) != VOS_OK) {
        return VOS_ERR;
    }

    AT_FillCmolrParaShapeRep(molrPara);

    AT_FillCmolrParaPlane(molrPara);

    if (AT_FillCmolrParaNmeaRep(molrPara) != VOS_OK) {
        return VOS_ERR;
    }

    if (AT_FillCmolrParaThdPtyAddr(molrPara) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetCmolrPara(VOS_UINT8 indexNum)
{
    TAF_SSA_LcsMolrParaSet molrPara;

    (VOS_VOID)memset_s(&molrPara, sizeof(molrPara), 0x00, sizeof(molrPara));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetCmolrPara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_FillCmolrPara(&molrPara) != VOS_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    TAF_SSA_SetCmolrInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &molrPara);

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMOLR_SET;

    /* 命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetCmolrePara(VOS_UINT8 indexNum)
{
    AT_ModemAgpsCtx *agpsCtx = VOS_NULL_PTR;

    agpsCtx = AT_GetModemAgpsCtxAddrFromClientId(g_atClientTab[indexNum].clientId);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetCmolrePara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多，返回失败 */
    if (g_atParaIndex > 1) {
        AT_WARN_LOG("AT_SetCmolrePara: too many parameters!");
        return AT_TOO_MANY_PARA;
    }

    /* 执行命令操作 */
    if (g_atParaList[0].paraLen == 0) {
        agpsCtx->cmolreType = AT_CMOLRE_NUMERIC;
    } else {
        agpsCtx->cmolreType = (VOS_UINT8)g_atParaList[0].paraValue;
    }

    /* 命令返回OK */
    return AT_OK;
}

VOS_UINT32 AT_SetCmtlrPara(VOS_UINT8 indexNum)
{
    TAF_SSA_LcsMtlrSubscribeUint8 subscribe;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetCmtlrPara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多，返回失败 */
    if (g_atParaIndex > 1) {
        AT_WARN_LOG("AT_SetCmtlrPara: too many parameters!");
        return AT_TOO_MANY_PARA;
    }

    /* 执行命令操作 */
    if (g_atParaList[0].paraLen == 0) {
        subscribe = TAF_SSA_LCS_MOLR_ENABLE_TYPE_DISABLE;
    } else {
        subscribe = (VOS_UINT8)g_atParaList[0].paraValue;
    }
    TAF_SSA_SetCmtlrInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, subscribe);

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMTLR_SET;

    /* 命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetCmtlraPara(VOS_UINT8 indexNum)
{
    TAF_SSA_LcsMtlraParaSet cmtlraPara;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetCmtlraPara: Option Type Incrrect!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数判断 */
    if (g_atParaIndex != AT_CMTLRA_PARA_NUM) {
        AT_WARN_LOG("AT_SetCmtlraPara: Incorrect Parameter Num!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    (VOS_VOID)memset_s(&cmtlraPara, sizeof(cmtlraPara), 0x00, sizeof(cmtlraPara));
    cmtlraPara.allow    = (VOS_UINT8)g_atParaList[0].paraValue;
    cmtlraPara.handleId = (VOS_UINT8)g_atParaList[1].paraValue;

    TAF_SSA_SetCmtlraInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &cmtlraPara);

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMTLRA_SET;

    /* 命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_CmmiProcUssd(VOS_UINT8 indexNum, TAF_SS_ProcessUssReq *ussdReq)
{
    TAF_UINT8  ussdStr[TAF_SS_MAX_USSDSTRING_LEN * 2];
    VOS_UINT32 procStrLen;
    VOS_UINT32 ret;

    (VOS_VOID)memset_s(ussdStr, sizeof(ussdStr), 0, sizeof(ussdStr));

    procStrLen = 0;

    /* 由于SSA的7Bit编解码上移至AT，CMMI下发前需将码流从Ascii转到Hex */
    /* TAF_SS_MAX_USSDSTRING_LEN * 2为数组ussdStr定义时的最大长度 */
    ret = TAF_STD_ConvertAsciiToDefAlpha(ussdReq->ussdStr.ussdStr, ussdReq->ussdStr.cnt, ussdStr, &procStrLen,
                                         TAF_SS_MAX_USSDSTRING_LEN * 2);

    ussdReq->ussdStr.cnt = (VOS_UINT16)procStrLen;

    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("AT_CmmiProcUssd : Conv7Bit Def Err");
        return TAF_FAILURE;
    }

    /* 清除原码流残余信息 */
    (VOS_VOID)memset_s(ussdReq->ussdStr.ussdStr, sizeof(ussdReq->ussdStr.ussdStr), 0, sizeof(ussdReq->ussdStr.ussdStr));

    ret = TAF_STD_Pack7Bit(ussdStr, ussdReq->ussdStr.cnt, 0, ussdReq->ussdStr.ussdStr, &procStrLen);

    ussdReq->ussdStr.cnt = (VOS_UINT16)procStrLen;

    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("AT_CmmiProcUssd : Conv7Bit pack Err");
        return TAF_FAILURE;
    }

    /* CMMI只带USSDString,默认打开主动上报 */
    ussdReq->cusdType = TAF_SS_CUSD_TYPE_ENABLE;

    /* 调用TAF接口发送 */
    if (TAF_ProcessUnstructuredSSReq(g_atClientTab[indexNum].clientId, 0, ussdReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CmmiProcUssd: TAF_ProcessUnstructuredSSReq failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeDProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId)
{
    TAF_Ctrl               ctrl;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    /* AT向CCM发送呼叫请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &(mmiOpParam->mnCallOrig), ID_TAF_CCM_CALL_ORIG_REQ,
                              sizeof(mmiOpParam->mnCallOrig), modemId) != VOS_OK) {
        AT_WARN_LOG("AT_CnmiOptTypeDProc: MN_CALL_Orig failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeChldProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId)
{
    TAF_Ctrl               ctrl;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    /* AT向CCM发送补充业务请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &(mmiOpParam->mnCallSupsReq), ID_TAF_CCM_CALL_SUPS_CMD_REQ,
                              sizeof(mmiOpParam->mnCallSupsReq), modemId) != VOS_OK) {
        AT_WARN_LOG("AT_CnmiOptTypeChldProc: MN_CALL_Sups failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeCpinProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId)
{
    if (Taf_PhonePinHandle(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->pinReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsRegisterProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsRegisterProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId)
{
    if (TAF_RegisterSSReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->registerSsReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsRegisterProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsEraseProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId)
{
    if (TAF_EraseSSReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->eraseSsReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsEraseProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsActivateProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId)
{
    if (TAF_ActivateSSReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->activateSsReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsActivateProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsDeactivateProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId)
{
    if (TAF_DeactivateSSReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->deactivateSsReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsDeactivateProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsDeactiveCCBSProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId)
{
    if (TAF_EraseCCEntryReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->ccbsEraseReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsDeactiveCCBSProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsInterrogateProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId)
{
    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->interrogateSsReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsInterrogateProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeSsRegisterPswdProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    ModemIdUint16 modemId)
{
    if (TAF_RegisterPasswordReq(g_atClientTab[indexNum].clientId, 0, &mmiOpParam->regPwdReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeSsRegisterPswdProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeCusdProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId)
{
    if (AT_CmmiProcUssd(indexNum, &mmiOpParam->processUssdReq) != TAF_SUCCESS) {
        AT_WARN_LOG("AT_CnmiOptTypeCusdProc failed.");
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeClipProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum, ModemIdUint16 modemId)
{
    AT_ModemSsCtx         *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    ssCtx->clipType = (AT_CLIP_TYPE)g_atParaList[0].paraValue;

    return VOS_FALSE;
}

LOCAL VOS_UINT32 AT_CnmiOptTypeCommProc(MN_MMI_OperationParam *mmiOpParam, VOS_UINT8 indexNum,
    AT_CmdCurOptUint32 cmdCurrentOpt, ModemIdUint16 modemId)
{
    AT_CnmiOptTypeMatchFunc     pktTypeMatchFunc = VOS_NULL_PTR;
    VOS_UINT32                  loop;
    VOS_UINT32                  itemsNum;
    VOS_UINT32                  ret = VOS_TRUE;

    itemsNum = sizeof(g_CnmiOptTypeMatchTbl) / sizeof(AT_CnmiOptTypeMatchEntity);

    for (loop = 0; loop < itemsNum; loop++) {
        if ((AT_CmdCurOptUint32)(g_CnmiOptTypeMatchTbl[loop].optType) == cmdCurrentOpt) {
            pktTypeMatchFunc = g_CnmiOptTypeMatchTbl[loop].pktTypeMatchFunc;
            break;
        }
    }

    if (pktTypeMatchFunc != VOS_NULL_PTR) {
        ret = pktTypeMatchFunc(mmiOpParam, indexNum, modemId);
    }

    return ret;
}

/*
 * 功能描述: 处理cimi
 */
VOS_UINT32 At_ProcCmmiStringParseCorrect(VOS_UINT8 indexNum, VOS_BOOL inCall, ModemIdUint16 modemId,
    MN_MMI_OperationParam *mmiOpParam, VOS_CHAR *pcOutRestMmiStr)
{
    AT_CmdCurOptUint32          cmdCurrentOpt;
    VOS_UINT32                  result = MN_ERR_NO_ERROR;

    switch (mmiOpParam->mmiOperationType) {
        case TAF_MMI_SUPPRESS_CLIR:
        case TAF_MMI_INVOKE_CLIR:
            g_clirOperate = ((mmiOpParam->mmiOperationType == TAF_MMI_SUPPRESS_CLIR) ?
                              MN_CALL_CLIR_SUPPRESS : MN_CALL_CLIR_INVOKE);

            if ((VOS_UINT16)(pcOutRestMmiStr - (VOS_CHAR *)(g_atParaList[0].para)) < g_atParaList[0].paraLen) {
                result = MN_MmiStringParse(pcOutRestMmiStr, inCall, mmiOpParam, &pcOutRestMmiStr);
            } else {
                return AT_CME_INCORRECT_PARAMETERS;
            }
            break;
        default:
            break;
    }

    if (result != MN_ERR_NO_ERROR) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 保存密码 */

    /* 调用TAF提供的接口，处理SS命令 */
    cmdCurrentOpt = At_GetMnOptType(mmiOpParam->mmiOperationType);
    if (cmdCurrentOpt == AT_CMD_INVALID) {
        return AT_ERROR;
    }

    if (AT_CnmiOptTypeCommProc(mmiOpParam, indexNum, cmdCurrentOpt, modemId) != VOS_TRUE) {
        return AT_OK;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = cmdCurrentOpt;
    (VOS_VOID)memset_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), 0x00, sizeof(g_atParaList[0].para));
    (VOS_VOID)memset_s(mmiOpParam, sizeof(MN_MMI_OperationParam), 0x00, sizeof(MN_MMI_OperationParam));
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 At_SetCmmiPara(VOS_UINT8 indexNum)
{
    VOS_BOOL                    inCall;
    MN_MMI_OperationParam      *mmiOpParam      = VOS_NULL_PTR;
    VOS_CHAR                   *pcOutRestMmiStr = VOS_NULL_PTR;
    AT_ModemCcCtx              *ccCtx = VOS_NULL_PTR;
    VOS_UINT32                  result;
    ModemIdUint16               modemId;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 没有呼叫存在的情况 */
    inCall = ((ccCtx->curIsExistCallFlag == VOS_FALSE) ? VOS_FALSE : VOS_TRUE);

    /*lint -save -e516 */
    mmiOpParam = (MN_MMI_OperationParam *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(MN_MMI_OperationParam));
    /*lint -restore */
    if (mmiOpParam == TAF_NULL_PTR) {
        AT_WARN_LOG("At_SetCmmiPara Mem Alloc FAILURE");
        return AT_ERROR;
    }

    g_clirOperate = MN_CALL_CLIR_AS_SUBS;

    pcOutRestMmiStr                          = (VOS_CHAR *)g_atParaList[0].para;
    pcOutRestMmiStr[g_atParaList[0].paraLen] = '\0';
    result = MN_MmiStringParse((VOS_CHAR *)(g_atParaList[0].para), inCall, mmiOpParam, &pcOutRestMmiStr);
    mmiOpParam->processUssdReq.datacodingScheme = TAF_SS_7bit_LANGUAGE_UNSPECIFIED;

    if (result != MN_ERR_NO_ERROR) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, mmiOpParam);
        /*lint -restore */
        return AT_CME_INCORRECT_PARAMETERS;
    }
    result = At_ProcCmmiStringParseCorrect(indexNum, inCall, modemId, mmiOpParam, pcOutRestMmiStr);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, mmiOpParam);
    /*lint -restore */
    return result;
}
