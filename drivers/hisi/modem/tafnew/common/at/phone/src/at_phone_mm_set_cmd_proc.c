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
#include "at_phone_mm_set_cmd_proc.h"
#include "securec.h"
#include "taf_app_mma.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_phone_comm.h"
#include "at_mdrv_interface.h"
#include "gunas_errno.h"
#include "at_check_func.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_MM_SET_CMD_PROC_C

#define AT_COPS_PARA_MAX_NUM 4
#define AT_CFUN_PARA_MAX_NUM 2
#define AT_CFUN_PARA_NUM 2
#define AT_CFUN_FUN 0
#define AT_CFUN_RST 1
#define AT_CFPLMN_OPER 1
#define AT_SYSCFGEX_ACQORDER 0
#define AT_SYSCFGEX_BAND 1
#define AT_SYSCFGEX_ROAM 2
#define AT_SYSCFGEX_SRVDOMAIN 3
#define AT_SYSCFGEX_LTEBAND 4
#define AT_NETSCAN_POWER 1
#define AT_NETSCAN_MODE 2
#define AT_NETSCAN_BAND 3
#define AT_SYSCFG_BAND 2
#define AT_SYSCFG_ROAM 3
#define AT_SYSCFG_SRVDOMAIN 4
#define AT_COPS_MODE 0
#define AT_COPS_OPER 2
#define AT_COPS_RAT 3
#define AT_CPOL_OPER 2
#define AT_CPOL_UTRAN 5
#define AT_CPOL_E_UTRAN 6
#define AT_CPOL_RAT 7
#define AT_CRPN_NAME_INDEX 0
#define AT_CRPN_NAME 1
#define TAF_MMA_RAT_ORDER_INDEX_0 0
#define TAF_MMA_RAT_ORDER_INDEX_1 1
#define TAF_MMA_RAT_ORDER_INDEX_2 2
#define AT_CFPLMN_OPER_MAX_VALID_LEN 6
#define AT_CFPLMN_OPER_MIN_VALID_LEN 5
#define AT_SYSCFG_VALID_NUM 5
#define AT_CRPN_NAME_MIN_LEN 5
#define AT_CRPN_NAME_MAX_LEN 6
#define AT_COPS_SHORT_OPER_LEN 5
#define AT_COPS_FULL_OPER_LEN 6
#define AT_SYSCFG_ROAM_INACTIVE_STATUS_MAX_VALUE 2
#define AT_SYSCFG_ROAM_ACTIVATE_STATUS_MAX_VALUE 3
#define AT_CPAM_PARA_MAX_NUM 2
#define AT_CGCATT_MODE_EQUALED_3_MAX_PARA_NUM 3
#define AT_CGCATT_MODE 1
#define AT_CGCATT_PS_STATE 2
#define AT_CGCATT_PARA_MAX_NUM 2
#define AT_CFPLMN_PARA_MAX_NUM 2
#define AT_UE_MODE_NR_AND_LTE_AT_CPOL_PARA_MAX_NUM 8
#define AT_UE_MODE_NR_OR_LTE_AT_CPOL_PARA_MAX_NUM 7
#define AT_CPOL_PARA_NUM 2
#define AT_CPOL_INDEX 0
#define AT_CPOL_FORMAT 1
#define AT_CPOL_GSM_ACTN 3
#define AT_CPOL_GSM_COMPACT_ACTN 4
#define AT_EMRSSICFG_PARA_NUM 2
#define AT_CRPN_PARA_VALID_NUM 2
#define AT_OTHER_MODE_AT_CPOL_PARA_MAX_NUM 6

#if (VOS_WIN32 == VOS_OS_VER)
VOS_VOID NAS_MSG_SndOutsideContextData_Part1(VOS_VOID);
VOS_VOID NAS_MSG_SndOutsideContextData_Part2(VOS_VOID);
VOS_VOID NAS_MSG_SndOutsideContextData_Part3(VOS_VOID);
#endif

VOS_UINT32 AT_CheckUserRebootSupportFlg(VOS_VOID)
{
    TAF_NVIM_UserRebootSupport userRebootSupport;
    VOS_UINT32                 result;
    VOS_UINT32                 len;

    len = sizeof(TAF_NVIM_UserRebootSupport);

    (VOS_VOID)memset_s(&userRebootSupport, len, 0x00, len);

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_USER_REBOOT_SUPPORT_FLG, &userRebootSupport, len);

    if (result != NV_OK) {
        AT_WARN_LOG("TAF_MMA_IsUserRebootSupport:Read Nvim Failed");
        return VOS_FALSE;
    }

    if (userRebootSupport.userRebootConfig == 0xEF) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
TAF_UINT32 At_SetCfunPara(TAF_UINT8 indexNum)
{
    TAF_MMA_PhoneModePara phoneModePara;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CFUN_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (((g_atParaList[AT_CFUN_FUN].paraLen == 0) || (g_atParaList[AT_CFUN_RST].paraLen == 0)) &&
        (g_atParaIndex == AT_CFUN_PARA_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<fun> */
    if (g_atParaList[AT_CFUN_FUN].paraValue >= TAF_PH_MODE_NUM_MAX) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

/* 输出TAF的回放数据 */
#if (VOS_WIN32 == VOS_OS_VER)
    NAS_MSG_SndOutsideContextData_Part1();
    NAS_MSG_SndOutsideContextData_Part2();
    NAS_MSG_SndOutsideContextData_Part3();
#endif
    /* 输出TAF的回放数据 */

    (VOS_VOID)memset_s(&phoneModePara, sizeof(phoneModePara), 0x00, sizeof(TAF_MMA_PhoneModePara));
    phoneModePara.phMode = (VOS_UINT8)g_atParaList[AT_CFUN_FUN].paraValue;

    if (g_atParaIndex == AT_CFUN_PARA_NUM) {
        phoneModePara.phReset = (VOS_UINT8)g_atParaList[AT_CFUN_RST].paraValue;

        /* NV项打开时AT+CFUN=1,1直接在A核触发整机复位 */
        if ((AT_CheckUserRebootSupportFlg() == VOS_TRUE) && (phoneModePara.phMode == TAF_PH_MODE_FULL) &&
            (phoneModePara.phReset == TAF_PH_OP_MODE_RESET)) {
            g_atSendDataBuff.bufLen = 0;
            At_FormatResultData(indexNum, AT_OK);
            VOS_TaskDelay(300); /* wait 300 tick */
            (VOS_VOID)TAF_ACORE_NV_FLUSH();
            AT_OmSystemError(NAS_REBOOT_MOD_ID_USER, 0, 0, VOS_NULL_PTR, 0);
            return AT_SUCCESS;
        }
    }

    if (TAF_MMA_PhoneModeSetReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &phoneModePara) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFUN_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCsqPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (TAF_MMA_QryCsqReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSQ_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_GetOperatorNameFromParam(VOS_UINT16 *operNameLen, VOS_CHAR *operName, VOS_UINT16 bufSize,
                                       VOS_UINT32 formatType)
{
    errno_t memResult;
    if ((formatType == AT_COPS_LONG_ALPH_TYPE) && (bufSize > g_atParaList[AT_COPS_OPER].paraLen) &&
        (g_atParaList[AT_COPS_OPER].paraLen != 0) && (g_atParaList[AT_COPS_OPER].paraLen <= TAF_PH_OPER_NAME_LONG)) {
        memResult = memcpy_s((VOS_CHAR *)operName, TAF_PH_OPER_NAME_LONG, (VOS_CHAR *)g_atParaList[AT_COPS_OPER].para,
                             g_atParaList[AT_COPS_OPER].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, TAF_PH_OPER_NAME_LONG, g_atParaList[AT_COPS_OPER].paraLen);
        operName[g_atParaList[AT_COPS_OPER].paraLen] = 0;
        *operNameLen                                 = g_atParaList[AT_COPS_OPER].paraLen;

        return AT_OK;
    } else if ((formatType == AT_COPS_SHORT_ALPH_TYPE) && (bufSize > g_atParaList[AT_COPS_OPER].paraLen) &&
               (g_atParaList[AT_COPS_OPER].paraLen != 0) &&
               (g_atParaList[AT_COPS_OPER].paraLen <= TAF_PH_OPER_NAME_LONG)) {
        memResult = memcpy_s((VOS_CHAR *)operName, TAF_PH_OPER_NAME_LONG, (VOS_CHAR *)g_atParaList[AT_COPS_OPER].para,
                             g_atParaList[AT_COPS_OPER].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, TAF_PH_OPER_NAME_LONG, g_atParaList[AT_COPS_OPER].paraLen);
        operName[g_atParaList[AT_COPS_OPER].paraLen] = 0;
        *operNameLen                                 = g_atParaList[AT_COPS_OPER].paraLen;

        return AT_OK;
    } else if ((bufSize > g_atParaList[AT_COPS_OPER].paraLen) &&
               ((g_atParaList[AT_COPS_OPER].paraLen == AT_COPS_SHORT_OPER_LEN) ||
                (g_atParaList[AT_COPS_OPER].paraLen == AT_COPS_FULL_OPER_LEN)) &&
                (g_atParaList[AT_COPS_OPER].paraLen <= TAF_PH_OPER_NAME_LONG)) {
        memResult = memcpy_s((VOS_CHAR *)operName, TAF_PH_OPER_NAME_LONG, (VOS_CHAR *)g_atParaList[AT_COPS_OPER].para,
                             g_atParaList[AT_COPS_OPER].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, TAF_PH_OPER_NAME_LONG, g_atParaList[AT_COPS_OPER].paraLen);
        operName[g_atParaList[AT_COPS_OPER].paraLen] = 0;
        *operNameLen                                 = g_atParaList[AT_COPS_OPER].paraLen;

        return AT_OK;
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }
}

LOCAL VOS_UINT32 AT_ProcCopsSetMode0(VOS_UINT8 indexNum)
{
    TAF_MMA_Ctrl     ctrl;
    TAF_PLMN_UserSel plmnUserSel;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(TAF_MMA_Ctrl));
    (VOS_VOID)memset_s(&plmnUserSel, sizeof(plmnUserSel), 0x00, sizeof(TAF_PLMN_UserSel));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = 0;

    plmnUserSel.bgSrchFlag = VOS_FALSE;
    plmnUserSel.srchType   = TAF_MMA_EXTEND_SEARCH_TYPE_FULLBAND;

    if (TAF_MMA_PlmnSearchReq(&ctrl, TAF_MMA_SEARCH_TYPE_AUTO, &plmnUserSel) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_SET_AUTOMATIC;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

LOCAL VOS_UINT32 AT_ProcCopsSetMode1(VOS_UINT8 indexNum)
{
    TAF_MMA_Ctrl         ctrl;
    TAF_PLMN_UserSel     plmnUserSel;
    VOS_UINT32           getOperNameRst;
    TAF_MMA_RatTypeUint8 phRat;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(TAF_MMA_Ctrl));
    (VOS_VOID)memset_s(&plmnUserSel, sizeof(plmnUserSel), 0x00, sizeof(TAF_PLMN_UserSel));

    /* 参数为空 */
    if (g_atParaList[AT_COPS_OPER].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写端口信息 */
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = 0;

    /* 设置<AcT> */
    At_SetCopsActPara(&phRat, g_atParaList[AT_COPS_RAT].paraLen, g_atParaList[AT_COPS_RAT].paraValue);

    plmnUserSel.formatType = (VOS_UINT8)g_atParaList[1].paraValue;
    plmnUserSel.accessMode = phRat;

    plmnUserSel.bgSrchFlag = VOS_FALSE;
    plmnUserSel.srchType   = TAF_MMA_EXTEND_SEARCH_TYPE_FULLBAND;

    /* 根据输入PLMN的类型来保存到TAF_PLMN_UserSel结构中，发送到MMA模块，有MMA进行转换并处理指定搜。 */
    getOperNameRst = AT_GetOperatorNameFromParam(&plmnUserSel.operNameLen, plmnUserSel.operName,
                                                 sizeof(plmnUserSel.operName), plmnUserSel.formatType);

    if (getOperNameRst != AT_OK) {
        return getOperNameRst;
    }

    if (TAF_MMA_PlmnSearchReq(&ctrl, TAF_MMA_SEARCH_TYPE_MANUAL, &plmnUserSel) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_SET_MANUAL;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

LOCAL VOS_UINT32 AT_ProcCopsSetMode2(VOS_UINT8 indexNum)
{
    TAF_MMA_DetachPara detachPara;

    (VOS_VOID)memset_s(&detachPara, sizeof(detachPara), 0x00, sizeof(TAF_MMA_DetachPara));

    detachPara.detachCause = TAF_MMA_DETACH_CAUSE_COPS;

    /* 转换用户的DETACH类型 */
    detachPara.detachDomain = TAF_MMA_SERVICE_DOMAIN_CS_PS;

    if (TAF_MMA_DetachReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &detachPara) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_SET_DEREGISTER;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

LOCAL VOS_UINT32 AT_ProcCopsSetMode3(VOS_UINT8 indexNum)
{
    TAF_MMA_CopsFormatTypeSetReq copsFormatSetReq;

    (VOS_VOID)memset_s(&copsFormatSetReq, sizeof(copsFormatSetReq), 0x00, sizeof(copsFormatSetReq));

    copsFormatSetReq.copsFormatType = (VOS_UINT8)g_atParaList[1].paraValue;

    Taf_SetCopsFormatTypeReq(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &copsFormatSetReq);

    return AT_OK;
}

LOCAL VOS_UINT32 AT_ProcCopsSetMode4(VOS_UINT8 indexNum)
{
    TAF_MMA_Ctrl         ctrl;
    TAF_PLMN_UserSel     plmnUserSel;
    VOS_UINT32           getOperNameRst;
    TAF_MMA_RatTypeUint8 phRat;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(TAF_MMA_Ctrl));
    (VOS_VOID)memset_s(&plmnUserSel, sizeof(plmnUserSel), 0x00, sizeof(TAF_PLMN_UserSel));

    /* 参数为空 */
    if (g_atParaList[AT_COPS_OPER].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写端口信息 */
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = 0;

    /* 设置<AcT> */
    At_SetCopsActPara(&phRat, g_atParaList[AT_COPS_RAT].paraLen, g_atParaList[AT_COPS_RAT].paraValue);

    plmnUserSel.formatType = (VOS_UINT8)g_atParaList[1].paraValue;
    plmnUserSel.accessMode = phRat;
    plmnUserSel.bgSrchFlag = VOS_FALSE;
    plmnUserSel.srchType   = TAF_MMA_EXTEND_SEARCH_TYPE_FULLBAND;

    /* 根据输入PLMN的类型来保存到TAF_PLMN_UserSel结构中，发送到MMA模块，有MMA进行转换并处理指定搜。 */
    getOperNameRst = AT_GetOperatorNameFromParam(&plmnUserSel.operNameLen, plmnUserSel.operName,
                                                 sizeof(plmnUserSel.operName), plmnUserSel.formatType);

    if (getOperNameRst != AT_OK) {
        return getOperNameRst;
    }

    if (TAF_MMA_PlmnSearchReq(&ctrl, TAF_MMA_SEARCH_TYPE_MANUAL_AUTO, &plmnUserSel) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPS_SET_MANUAL_AUTOMATIC_MANUAL;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCopsPara(TAF_UINT8 indexNum)
{
    VOS_UINT32                  i;
    const AT_UserReselProcTbl   userReselProcTbl[] = {
        /*   user option     reserved     proc func            */
        { 0, 0, AT_ProcCopsSetMode0 }, { 1, 0, AT_ProcCopsSetMode1 }, { 2, 0, AT_ProcCopsSetMode2 },
        { 3, 0, AT_ProcCopsSetMode3 }, { 4, 0, AT_ProcCopsSetMode4 },
    };

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_COPS_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[AT_COPS_MODE].paraLen == 0) {
        g_atParaList[AT_COPS_MODE].paraValue = 0;
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (At_CheckCurrRatModeIsCL((VOS_UINT8)(g_atClientTab[indexNum].clientId)) == VOS_TRUE) {
        if ((g_atParaList[AT_COPS_MODE].paraValue != 3) && (g_atParaList[AT_COPS_MODE].paraValue != 0)) {
            return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
        }
    }
#endif


    for (i = 0; i < sizeof(userReselProcTbl) / sizeof(userReselProcTbl[0]); i++) {
        if (g_atParaList[AT_COPS_MODE].paraValue == userReselProcTbl[i].userResel) {
            return userReselProcTbl[i].userReselProc(indexNum);
        }
    }

    return AT_ERROR;
}

TAF_UINT32 At_SetCgattPara(TAF_UINT8 indexNum)
{
    TAF_MMA_DetachPara detachPara;

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

    /* 执行命令操作 */
    if (g_atParaList[0].paraValue == 0) {
        /* 执行Detach操作 */
        (VOS_VOID)memset_s(&detachPara, sizeof(detachPara), 0x00, sizeof(TAF_MMA_DetachPara));
        detachPara.detachCause  = TAF_MMA_DETACH_CAUSE_USER_DETACH;
        detachPara.detachDomain = TAF_MMA_SERVICE_DOMAIN_PS;

        if (TAF_MMA_DetachReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &detachPara) == VOS_TRUE) {
            /* 设置当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGATT_DETAACH_SET;
            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        } else {
            return AT_ERROR;
        }
    } else {
        /* 执行Attach操作 */
        if (TAF_MMA_AttachReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, TAF_MMA_ATTACH_TYPE_GPRS) ==
            VOS_TRUE) {
            /* 设置当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGATT_ATTACH_SET;
            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        } else {
            return AT_ERROR;
        }
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 At_SetSystemInfo(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_MMA_QrySystemInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, VOS_FALSE) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSINFO_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_VOID AT_ReadRoamCfgInfoNvim(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx      *netCtx = VOS_NULL_PTR;
    NAS_NVIM_RoamCfgInfo roamFeature;
    VOS_UINT32           rst;
    ModemIdUint16        modemId = MODEM_ID_0;

    /* 获取ModemId */
    rst = AT_GetModemIdFromClient(indexNum, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("AT_ReadRoamCfgInfoNvim:Get ModemID From ClientID fail,ucIndex=%d", indexNum);
        return;
    }

    roamFeature.roamFeatureFlg = NV_ITEM_ACTIVE;
    roamFeature.roamCapability = 0;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    roamFeature.roamRule = NVIM_ROAM_RULE_CMP_PLMN;
#endif
    if (TAF_ACORE_NV_READ(modemId, NV_ITEM_ROAM_CAPA, &roamFeature, sizeof(NAS_NVIM_RoamCfgInfo)) != NV_OK) {
        AT_WARN_LOG("AT_ReadRoamCfgInfoNvim():WARNING: NV_ITEM_ROAM_CAPA Error");

        return;
    }

    if ((roamFeature.roamFeatureFlg != NV_ITEM_ACTIVE) && (roamFeature.roamFeatureFlg != NV_ITEM_DEACTIVE)) {
        roamFeature.roamFeatureFlg = NV_ITEM_DEACTIVE; /* 容错，nv配置无效值，按照特性关闭处理 */
    }

    if (roamFeature.roamCapability >= TAF_MMA_ROAM_BUTT) {
        roamFeature.roamCapability = TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF;
    }

    netCtx              = AT_GetModemNetCtxAddrFromModemId(modemId);
    netCtx->roamFeature = roamFeature.roamFeatureFlg;
    netCtx->roamCapa    = roamFeature.roamCapability;
 #if (FEATURE_MBB_CUST == FEATURE_ON)
    if (roamFeature.roamRule >= NVIM_ROAM_RULE_BUTT) {
        roamFeature.roamRule = NVIM_ROAM_RULE_CMP_PLMN;
    }
    netCtx->roamRule = roamFeature.roamRule;
 #endif
}

AT_SYSCFG_RatPrioUint8 AT_GetGuAccessPrioNvim(ModemIdUint16 modemId)
{
    NAS_MMA_NvimAccessMode accessMode;
    AT_SYSCFG_RatPrioUint8 acqorder;

    (VOS_VOID)memset_s(&accessMode, sizeof(accessMode), 0x00, sizeof(NAS_MMA_NvimAccessMode));

    if (TAF_ACORE_NV_READ(modemId, NV_ITEM_MMA_ACCESS_MODE, &accessMode, sizeof(NAS_MMA_NvimAccessMode)) != NV_OK) {
        return AT_SYSCFG_RAT_PRIO_AUTO;
    }

    acqorder = accessMode.accessMode[1];
    return acqorder;
}

VOS_VOID AT_ConvertSysCfgRatAutoOrder(AT_SYSCFG_RatPrioUint8 *acqorder, TAF_MMA_MultimodeRatCfg *sysCfgRatOrder,
    ModemIdUint16 modemId)
{
    /* 如果接入优先级不改变则默认从NVIM中获取 */
    if (*acqorder == AT_SYSCFG_RAT_PRIO_NO_CHANGE) {
        *acqorder = AT_GetGuAccessPrioNvim(modemId);
    }

    /* ucAcqorder为0自动或先WCDMA后GSM */

    if (*acqorder == AT_SYSCFG_RAT_PRIO_AUTO) {
        AT_SetDefaultRatPrioList(modemId, sysCfgRatOrder, VOS_FALSE, VOS_FALSE);
    } else if (*acqorder == AT_SYSCFG_RAT_PRIO_WCDMA) {
        sysCfgRatOrder->ratNum      = TAF_PH_MAX_GU_RAT_NUM;
        sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_WCDMA;
        sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_GSM;
        sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

    } else if (*acqorder == AT_SYSCFG_RAT_PRIO_GSM) {
        sysCfgRatOrder->ratNum      = TAF_PH_MAX_GU_RAT_NUM;
        sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_GSM;
        sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_WCDMA;
        sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

    } else {
    }
}

VOS_UINT32 AT_ConvertSysCfgRatOrderPara(VOS_UINT8 clientId, AT_SYSCFG_RatTypeUint8 ratMode,
                                        AT_SYSCFG_RatPrioUint8 acqorder, TAF_MMA_SysCfgPara *sysCfgPara)
{
    ModemIdUint16            modemId = MODEM_ID_0;
    VOS_UINT32               rst;
    TAF_MMA_MultimodeRatCfg *sysCfgRatOrder = VOS_NULL_PTR;

    sysCfgRatOrder = &(sysCfgPara->multiModeRatCfg);
    /* 获取ModemId */
    rst = AT_GetModemIdFromClient(clientId, &modemId);

    if (rst != VOS_OK) {
        AT_ERR_LOG1("AT_ConvertSysCfgRatOrderPara:Get ModemID From ClientID fail,ClientID:", clientId);
        return AT_ERROR;
    }
    switch (ratMode) {
        case AT_SYSCFG_RAT_AUTO:
            AT_ConvertSysCfgRatAutoOrder(&acqorder, sysCfgRatOrder, modemId);
            break;

        case AT_SYSCFG_RAT_GSM:
            /* G单模 */
            sysCfgRatOrder->ratNum      = 1;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_GSM;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_BUTT;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

            break;

        case AT_SYSCFG_RAT_WCDMA:
            /* W单模 */
            sysCfgRatOrder->ratNum      = 1;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_WCDMA;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_BUTT;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

            break;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case AT_SYSCFG_RAT_1X:
            /* 1X单模 */
            sysCfgRatOrder->ratNum      = 1;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_1X;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_BUTT;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

            break;

        case AT_SYSCFG_RAT_HRPD:
            /* HRPD单模 */
            sysCfgRatOrder->ratNum      = 1;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_HRPD;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_BUTT;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

            break;

        case AT_SYSCFG_RAT_1X_AND_HRPD:
            /* 1X&HRPD混合模式 */
            sysCfgRatOrder->ratNum      = 2;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_1X;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_HRPD;
            sysCfgRatOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;

            break;
#endif

        case AT_SYSCFG_RAT_NO_CHANGE:
            sysCfgRatOrder->ratNum         = 0;
            sysCfgPara->ratCfgNoChangeType = TAF_MMA_RAT_NO_CHANGE_CMD_SYSCFG;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 返回成功 */
    return AT_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
LOCAL VOS_VOID AT_SysCfgexSetRoamParaConvert(VOS_UINT8 clientId, VOS_UINT8 *outRoam)
{
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(clientId);

    if (netCtx->roamFeature == VOS_FALSE) {
        /* syscfg命令设置漫游能力不改变，使用原漫游能力 */
        if (*outRoam == AT_ROAM_FEATURE_OFF_NOCHANGE) {
            *outRoam = netCtx->roamCapa;
        } else {
            *outRoam = (*outRoam == VOS_FALSE) ? TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF :
                TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON;
            AT_WARN_LOG1("AT_SysCfgexSetRoamParaConvert():Convert RoamCapa, outRoam", *outRoam);
        }
    }
}

/*
 * 功能说明: 检查SYSCFGEX的acqorder是否在NV定义的组合列表中
 * 输入参数: ratOrder: 设置命令输入的RAT列表
 * 返回结果: VOS_TRUE: RAT列表在NV定义的组合列表中
 *           VOS_FALSE: RAT列表不在NV定义的组合列表中
 */
LOCAL VOS_UINT32 AT_CheckRatOrder(const AT_SyscfgexRatOrder *ratOrder)
{
    VOS_UINT8 i;
    VOS_UINT8 j;
    VOS_UINT8 *tmpRatOrder = VOS_NULL_PTR;
    AT_ModemMbbSysCfgExRatGroupCtrl *mbbSysCfgExCtrl = AT_GetModemMbbSysCfgExRatGroupCtrl(MODEM_ID_0);

    for (i = 0; i < mbbSysCfgExCtrl->itemNum; i++) {
        /* 逐个接入技术进行匹配 */
        tmpRatOrder = mbbSysCfgExCtrl->ratGroupList[i];

        for (j = 0; j < ratOrder->ratOrderNum; j++) {
            if ((ratOrder->ratOrder[j] == tmpRatOrder[j]) ||
                ((ratOrder->ratOrder[j] == AT_SYSCFGEX_RAT_NO_CHANGE) &&
                (tmpRatOrder[j] == AT_SYSCFGEX_ACT_NOT_CHANGE))) {
                continue;
            }
            break;
        }
        /* 匹配成功 */
        if ((j == ratOrder->ratOrderNum) && (tmpRatOrder[j] == AT_SYSCFGEX_ACT_INVALID)) {
            return VOS_TRUE;
        }
    }
    return VOS_FALSE;
}

/*
 * 功能说明: 检查设置的制式是否在NV50198列表中
 * 输入参数: ratOrder: 设置命令输入的RAT列表
 * 返回结果: VOS_TRUE: 输入合法
 *           VOS_FALSE: 输入非法
 */
VOS_UINT32 AT_CheckSysCfgExRatRestrict(const AT_SyscfgexRatOrder *ratOrder)
{
    AT_ModemMbbSysCfgExRatGroupCtrl *mbbSysCfgExCtrl = AT_GetModemMbbSysCfgExRatGroupCtrl(MODEM_ID_0);
    /* 需要限制设置的接入技术 */
    if (mbbSysCfgExCtrl->restrictFlg == VOS_TRUE) {
        /* 仅列表中设置的组合可以设置 */
        return AT_CheckRatOrder(ratOrder);
    }
    return VOS_TRUE;
}

/*
 * 功能说明: 将SYSCFGEX的00参数转换为对应的制式组合
 * 输出参数: 转换后的制式组合存储结构指针
 */
VOS_UINT32 AT_ConvertAutoModeToRatList(TAF_MMA_MultimodeRatCfg *ratOrder)
{
    VOS_UINT8 ratIdx;
    VOS_UINT8 ratNum = 0;

    const AT_ModemMbbAutoModeRatInfo *autoModeRatInfo = AT_GetModemMbbModeRatInfo(MODEM_ID_0);
    if (autoModeRatInfo->ratNum == 0) {
        return VOS_FALSE;
    }

    for (ratIdx = 0; ratIdx < autoModeRatInfo->ratNum && ratNum < TAF_MMA_RAT_BUTT; ratIdx++) {
        switch (autoModeRatInfo->ratList[ratIdx]) {
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            case TAF_MMA_RAT_NR: {
                ratOrder->ratOrder[ratNum++] = TAF_MMA_RAT_NR;
                break;
            }
#endif
#if (FEATURE_LTE == FEATURE_ON)
            case TAF_MMA_RAT_LTE: {
                ratOrder->ratOrder[ratNum++] = TAF_MMA_RAT_LTE;
                break;
            }
#endif
            case TAF_MMA_RAT_WCDMA: {
                ratOrder->ratOrder[ratNum++] = TAF_MMA_RAT_WCDMA;
                break;
            }
            case TAF_MMA_RAT_GSM: {
                ratOrder->ratOrder[ratNum++] = TAF_MMA_RAT_GSM;
                break;
            }
            default: {
                AT_WARN_LOG1("AT_ConvertAutoModeToRatList():Convert rat invalid, rat", autoModeRatInfo->ratList[ratIdx]);
                break;
            }
        }
    }
    ratOrder->ratNum = ratNum;
    return VOS_TRUE;
}
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_SetSysCfgPara(VOS_UINT8 indexNum)
{
    TAF_MMA_SysCfgPara sysCfgPara;

    VOS_UINT32      rst;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    AT_PR_LOGI("Rcv Msg");

    (VOS_VOID)memset_s(&sysCfgPara, sizeof(sysCfgPara), 0x00, sizeof(TAF_MMA_SysCfgPara));

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    if (g_atParaIndex != AT_SYSCFG_VALID_NUM) { /* 参数过多 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0) ||
        (g_atParaList[AT_SYSCFG_BAND].paraLen == 0) || (g_atParaList[AT_SYSCFG_ROAM].paraLen == 0) ||
        (g_atParaList[AT_SYSCFG_SRVDOMAIN].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = AT_ConvertSysCfgRatOrderPara(indexNum, (VOS_UINT8)g_atParaList[0].paraValue,
                                       (VOS_UINT8)g_atParaList[1].paraValue, &sysCfgPara);

    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = AT_ConvertSysCfgExGuBandPara(g_atParaList[AT_SYSCFG_BAND].para, g_atParaList[AT_SYSCFG_BAND].paraLen,
                                       &sysCfgPara.guBand);
    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 需要将L的频段设置为不改变 */
    sysCfgPara.userPrio = (VOS_UINT8)g_atParaList[1].paraValue;
    (VOS_VOID)memset_s(&(sysCfgPara.lBand), sizeof(sysCfgPara.lBand), 0x00, sizeof(TAF_USER_SetLtePrefBandInfo));
    sysCfgPara.lBand.bandInfo[0] = TAF_PH_BAND_NO_CHANGE;

    AT_ReadRoamCfgInfoNvim(indexNum);

    if (((netCtx->roamFeature == AT_ROAM_FEATURE_OFF) &&
         (g_atParaList[AT_SYSCFG_ROAM].paraValue > AT_SYSCFG_ROAM_INACTIVE_STATUS_MAX_VALUE)) ||
        ((netCtx->roamFeature == AT_ROAM_FEATURE_ON) &&
         (g_atParaList[AT_SYSCFG_ROAM].paraValue > AT_SYSCFG_ROAM_ACTIVATE_STATUS_MAX_VALUE))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    sysCfgPara.roam      = (VOS_UINT8)g_atParaList[AT_SYSCFG_ROAM].paraValue;
    sysCfgPara.srvDomain = (VOS_UINT8)g_atParaList[AT_SYSCFG_SRVDOMAIN].paraValue;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    AT_SysCfgexSetRoamParaConvert(indexNum, &(sysCfgPara.roam));
#endif

    /* 执行命令操作 */
    if (TAF_MMA_SetSysCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &sysCfgPara) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSCFG_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}
#endif

LOCAL VOS_VOID AT_ConvertSysRatToTafRat(AT_SyscfgexRatOrder *sysCfgExRatOrder, TAF_MMA_MultimodeRatCfg *ratOrder)
{
    VOS_UINT32  i;
    /* 非组合原因值的接入技术转换 */
    ratOrder->ratNum = sysCfgExRatOrder->ratOrderNum;

    for (i = 0; i < sysCfgExRatOrder->ratOrderNum; i++) {
        switch (sysCfgExRatOrder->ratOrder[i]) {
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            case AT_SYSCFGEX_RAT_NR:
                ratOrder->ratOrder[i] = TAF_MMA_RAT_NR;
                break;
#endif

#if (FEATURE_LTE == FEATURE_ON)
            case AT_SYSCFGEX_RAT_LTE:
                ratOrder->ratOrder[i] = TAF_MMA_RAT_LTE;
                break;
#endif
            case AT_SYSCFGEX_RAT_WCDMA:
                ratOrder->ratOrder[i] = TAF_MMA_RAT_WCDMA;
                break;

            case AT_SYSCFGEX_RAT_GSM:
                ratOrder->ratOrder[i] = TAF_MMA_RAT_GSM;
                break;
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
            case AT_SYSCFGEX_RAT_1X:
                ratOrder->ratOrder[i] = TAF_MMA_RAT_1X;
                break;

            case AT_SYSCFGEX_RAT_DO:
                ratOrder->ratOrder[i] = TAF_MMA_RAT_HRPD;
                break;
#endif
            default:
                AT_ERR_LOG1("AT_ConvertSysRatToTafRat, invalid rat ", sysCfgExRatOrder->ratOrder[i]);
                break;
        }
    }
}

VOS_VOID At_ConvertSysCfgExRat(VOS_UINT8 clientId, AT_SyscfgexRatOrder *sysCfgExRatOrder,
                               TAF_MMA_SysCfgPara *sysCfgExSetPara)
{
    ModemIdUint16            modemId = MODEM_ID_0;
    VOS_UINT32               rst;
    TAF_MMA_MultimodeRatCfg *ratOrder = VOS_NULL_PTR;

    ratOrder = &(sysCfgExSetPara->multiModeRatCfg);

    /* 获取ModemID */
    rst = AT_GetModemIdFromClient(clientId, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("At_ConvertSysCfgExRat:Get ModemID From ClientID fail,ClientID=%d", clientId);
        return;
    }

    /* 组合原因值00的接入技术转换 */
    if ((sysCfgExRatOrder->ratOrderNum == 1) && (sysCfgExRatOrder->ratOrder[0] == AT_SYSCFGEX_RAT_AUTO)) {
#if (FEATURE_MBB_CUST == FEATURE_ON)
        if (AT_ConvertAutoModeToRatList(ratOrder) == VOS_FALSE) {
            AT_SetDefaultRatPrioList(modemId, ratOrder, VOS_TRUE, VOS_TRUE);
        }
#else
        AT_SetDefaultRatPrioList(modemId, ratOrder, VOS_TRUE, VOS_TRUE);
#endif
        return;
    }

    /* 组合原因值99的接入技术转换 */
    if ((sysCfgExRatOrder->ratOrderNum == 1) && (sysCfgExRatOrder->ratOrder[0] == AT_SYSCFGEX_RAT_NO_CHANGE)) {
        ratOrder->ratNum                    = 0;
        sysCfgExSetPara->ratCfgNoChangeType = TAF_MMA_RAT_NO_CHANGE_CMD_SYSCFGEX;
        return;
    }
    AT_ConvertSysRatToTafRat(sysCfgExRatOrder, ratOrder);
}

LOCAL VOS_UINT32 AT_CheckSyscfgExRatPara(VOS_UINT8 clientId, AT_SyscfgexRatOrder *syscfgExRatOrder,
    TAF_MMA_SysCfgPara *sysCfgExSetPara)
{

    syscfgExRatOrder->ratOrderNum = AT_MIN(syscfgExRatOrder->ratOrderNum, TAF_PH_MAX_SUPPORT_RAT_NUM);
    /* acqorder字符串模式设置有重复 */
    if (At_CheckRepeatedRatOrder(syscfgExRatOrder) == VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 如果NV50198要求限制输入接入技术组合 */
    if (AT_CheckSysCfgExRatRestrict(syscfgExRatOrder) == VOS_FALSE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif
    /* 将SYSCFGEX的接入技术形式转换为TAF_MMA_RatOrder结构格式 */
    At_ConvertSysCfgExRat(clientId, syscfgExRatOrder, sysCfgExSetPara);

    return AT_OK;
}

VOS_UINT32 AT_ConvertSysCfgExRatOrderPara(VOS_UINT8 clientId, TAF_MMA_SysCfgPara *sysCfgExSetPara)
{
    VOS_UINT32          rst;
    VOS_UINT32          i;
    VOS_UINT32          ratOrder;
    VOS_UINT8           indexNum = 0;
    VOS_UINT32          ratAutoNoChangeExistFlag = VOS_FALSE;
    AT_SyscfgexRatOrder syscfgExRatOrder;

    /* acqorder字符串长度为奇数 */
    if ((g_atParaList[AT_SYSCFGEX_ACQORDER].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* acqorder字符串含非数字 */
    rst = At_CheckNumString(g_atParaList[AT_SYSCFGEX_ACQORDER].para, g_atParaList[AT_SYSCFGEX_ACQORDER].paraLen);
    if (rst != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* acqorder字符串长度是偶数但大于12也非法 */
    if ((g_atParaList[AT_SYSCFGEX_ACQORDER].paraLen / 2) > TAF_PH_MAX_SUPPORT_RAT_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for (i = 0; i < g_atParaList[AT_SYSCFGEX_ACQORDER].paraLen; i += AT_SYSCFGEX_RAT_MODE_STR_LEN) {
        rst = At_String2Hex(&g_atParaList[AT_SYSCFGEX_ACQORDER].para[i], AT_SYSCFGEX_RAT_MODE_STR_LEN, (VOS_UINT32 *)&ratOrder);
        if (rst != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 如果ulRatOrder是0x99十六进制数，需要转换为十进制的99 */
        if (ratOrder == AT_SYSCFGEX_RAT_NO_CHANGE_HEX_VALUE) {
            ratOrder = AT_SYSCFGEX_RAT_NO_CHANGE;
        }

        /* acqorder字符串含非00/01/02/03/04/07/08/99的组合 */
        if (At_CheckSysCfgExRatTypeValid((AT_SyscfgexRatTypeUint8)ratOrder) == VOS_FALSE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (((AT_SyscfgexRatTypeUint8)ratOrder == AT_SYSCFGEX_RAT_AUTO) ||
            ((AT_SyscfgexRatTypeUint8)ratOrder == AT_SYSCFGEX_RAT_NO_CHANGE)) {
            ratAutoNoChangeExistFlag = VOS_TRUE;
        }

        syscfgExRatOrder.ratOrder[indexNum] = (AT_SyscfgexRatTypeUint8)ratOrder;
        indexNum++;
    }

    syscfgExRatOrder.ratOrderNum = indexNum;

    /* acqorder字符串是00，99和其他值的组合 */
    if ((ratAutoNoChangeExistFlag == VOS_TRUE) && (syscfgExRatOrder.ratOrderNum > 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    return AT_CheckSyscfgExRatPara(clientId, &syscfgExRatOrder, sysCfgExSetPara);
}

VOS_UINT32 At_CheckRepeatedRatOrder(AT_SyscfgexRatOrder *syscfgExRatOrder)
{
    VOS_UINT32 i;
    VOS_UINT32 j;

    for (i = 0; i < syscfgExRatOrder->ratOrderNum; i++) {
        for (j = i + 1; j < syscfgExRatOrder->ratOrderNum; j++) {
            if (syscfgExRatOrder->ratOrder[i] == syscfgExRatOrder->ratOrder[j]) {
                return VOS_TRUE;
            }
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 At_CheckSysCfgExRatTypeValid(VOS_UINT8 ratOrder)
{
    switch (ratOrder) {
        case AT_SYSCFGEX_RAT_AUTO:
        case AT_SYSCFGEX_RAT_NO_CHANGE:
        case AT_SYSCFGEX_RAT_LTE:
        case AT_SYSCFGEX_RAT_WCDMA:
        case AT_SYSCFGEX_RAT_GSM:
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case AT_SYSCFGEX_RAT_1X:
        case AT_SYSCFGEX_RAT_DO:
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case AT_SYSCFGEX_RAT_NR:
#endif
            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}

VOS_UINT32 AT_ConvertSysCfgExGuBandPara(VOS_UINT8 *para, VOS_UINT16 len, TAF_UserSetPrefBand64 *prefBandPara)
{
    VOS_UINT32 rslt;

    prefBandPara->bandHigh = 0;
    prefBandPara->bandLow  = 0;

    /* 如果频段为空，返回失败 */
    if (len == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 频段值小于32位时 */
    if (len <= AT_HEX_LEN_OF_ULONG_TYPE) {
        rslt = At_String2Hex(para, len, &(prefBandPara->bandLow));
    } else {
        /* 64位频段的高32位字符串转换为16进制数，转换失败返回 */
        rslt = At_String2Hex(para, (len - AT_HEX_LEN_OF_ULONG_TYPE), &(prefBandPara->bandHigh));
        if (rslt == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 64位频段的低32位字符串转换为16进制数，转换失败返回  */
        rslt = At_String2Hex(para + (len - AT_HEX_LEN_OF_ULONG_TYPE), AT_HEX_LEN_OF_ULONG_TYPE,
                             &(prefBandPara->bandLow));
    }

    if (rslt == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_ConvertSysCfgExLteBandPara(VOS_UINT8 *para, VOS_UINT16 len, TAF_USER_SetLtePrefBandInfo *prefBandPara)
{
    TAF_USER_SetLtePrefBandInfo allBand;
    VOS_UINT32                  rslt = AT_SUCCESS;
    VOS_UINT32                  loop;
    VOS_UINT16                  convertCnt;

    convertCnt = len / AT_HEX_LEN_OF_ULONG_TYPE;
    (VOS_VOID)memset_s(&allBand, sizeof(allBand), 0xFF, sizeof(TAF_USER_SetLtePrefBandInfo));
    (VOS_VOID)memset_s(prefBandPara, sizeof(TAF_USER_SetLtePrefBandInfo), 0x00, sizeof(TAF_USER_SetLtePrefBandInfo));

    /* 参数过长，返回错误 */
    if ((len > TAF_MMA_LTE_BAND_MAX_LENGTH * AT_HEX_LEN_OF_ULONG_TYPE)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AP侧新需求，如果频段为空，认为频段不变 */
    if (len == 0) {
        prefBandPara->bandInfo[0] = TAF_PH_BAND_NO_CHANGE;

        return AT_OK;
    }

    for (loop = 0; loop < convertCnt; loop++) {
        rslt |= At_String2Hex(para + (VOS_UINT16)((len - (loop + 1) * AT_HEX_LEN_OF_ULONG_TYPE)),
                              AT_HEX_LEN_OF_ULONG_TYPE, &(prefBandPara->bandInfo[loop]));
    }

    if ((len % AT_HEX_LEN_OF_ULONG_TYPE) != 0) {
        rslt |= At_String2Hex(para, len - (VOS_UINT16)(convertCnt * AT_HEX_LEN_OF_ULONG_TYPE),
                              &(prefBandPara->bandInfo[convertCnt]));
    }

    if (rslt != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AP侧需求，全部设置为F时，认为是全频段 */
    if (VOS_MemCmp(prefBandPara, &allBand, sizeof(TAF_USER_SetLtePrefBandInfo)) == 0) {
        (VOS_VOID)memset_s(prefBandPara, sizeof(TAF_USER_SetLtePrefBandInfo), 0x00, sizeof(TAF_USER_SetLtePrefBandInfo));
        prefBandPara->bandInfo[0] = MN_MMA_LTE_LOW_BAND_ANY;
        prefBandPara->bandInfo[1] = MN_MMA_LTE_HIGH_BAND_ANY;
    }

    return AT_OK;
}

LOCAL VOS_UINT32 AT_IsModeConflictingDomain(TAF_MMA_SysCfgPara *sysCfgExSetPara)
{
    /* 如果设置的模式里有L 或者 NR,且服务域是cs only，返回VOS_TRUE */
    if ((g_atParaList[AT_SYSCFGEX_SRVDOMAIN].paraValue == TAF_MMA_SERVICE_DOMAIN_CS) ||
        (g_atParaList[AT_SYSCFGEX_SRVDOMAIN].paraValue == TAF_MMA_SERVICE_DOMAIN_ANY)) {
        if (AT_IsSpecRatSupported(TAF_MMA_RAT_LTE, &sysCfgExSetPara->multiModeRatCfg) == VOS_TRUE) {
            return VOS_TRUE;
        }
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        if (AT_IsSpecRatSupported(TAF_MMA_RAT_NR, &sysCfgExSetPara->multiModeRatCfg) == VOS_TRUE) {
            return VOS_TRUE;
        }
#endif
    }
    return VOS_FALSE;
}

VOS_UINT32 AT_CheckSysCfgExPara(VOS_UINT8 clientId, TAF_MMA_SysCfgPara *sysCfgExSetPara)
{
    VOS_UINT32      rst;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(clientId);

    /* 检查参数个数,如果不为7个，返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != AT_SYSCFGEX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查acqorder参数合法性,如果合法,进行转换并记录,如果不合法直接返回失败 */
    rst = AT_ConvertSysCfgExRatOrderPara(clientId, sysCfgExSetPara);

    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (sysCfgExSetPara->ratCfgNoChangeType != TAF_MMA_RAT_NO_CHANGE_CMD_SYSCFGEX) {
        sysCfgExSetPara->userPrio = AT_GetSysCfgPrioRat(sysCfgExSetPara);
    }

    /* 如果设置的模式里有L,且服务域是cs only，返回失败 */

    if (AT_IsModeConflictingDomain(sysCfgExSetPara) == VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查band参数合法性,如果合法,进行转换并记录,如果不合法直接返回失败 */
    rst = AT_ConvertSysCfgExGuBandPara(g_atParaList[AT_SYSCFGEX_BAND].para, g_atParaList[AT_SYSCFGEX_BAND].paraLen,
                                       &sysCfgExSetPara->guBand);

    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 漫游特性未激活,漫游能力取值范围0-2; 漫游特性激活,漫游能力取值范围0-3 */
    AT_ReadRoamCfgInfoNvim(clientId);
    if (((netCtx->roamFeature == AT_ROAM_FEATURE_OFF) &&
         (g_atParaList[AT_SYSCFGEX_ROAM].paraValue > AT_ROAM_FEATURE_OFF_NOCHANGE)) ||
        ((netCtx->roamFeature == AT_ROAM_FEATURE_ON) &&
         (g_atParaList[AT_SYSCFGEX_ROAM].paraValue > TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    sysCfgExSetPara->roam = (VOS_UINT8)g_atParaList[AT_SYSCFGEX_ROAM].paraValue;

    /* 检查lteband参数合法性,如果合法,进行转换并记录,如果不合法直接返回失败 */
    rst = AT_ConvertSysCfgExLteBandPara(g_atParaList[AT_SYSCFGEX_LTEBAND].para,
                                        g_atParaList[AT_SYSCFGEX_LTEBAND].paraLen, &sysCfgExSetPara->lBand);
    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    sysCfgExSetPara->srvDomain = (VOS_UINT8)g_atParaList[AT_SYSCFGEX_SRVDOMAIN].paraValue;

    return rst;
}

VOS_UINT32 AT_SetSysCfgExPara(VOS_UINT8 indexNum)
{
    TAF_MMA_SysCfgPara sysCfgExSetPara;

    VOS_UINT32 rst;

    AT_PR_LOGI("Rcv Msg");

    (VOS_VOID)memset_s(&sysCfgExSetPara, sizeof(sysCfgExSetPara), 0x00, sizeof(sysCfgExSetPara));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    rst = AT_CheckSysCfgExPara(indexNum, &sysCfgExSetPara);

    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#if (FEATURE_MBB_CUST == FEATURE_ON)
    AT_SysCfgexSetRoamParaConvert(indexNum, &(sysCfgExSetPara.roam));
#endif
    /* 与MMA进行交互不成功返回失败，交互成功返回命令处理挂起状态 */
    rst = TAF_MMA_SetSysCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &sysCfgExSetPara);
    AT_PR_LOGI("Call interface success!");

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSCFG_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

/* begin V7R1 PhaseI Modify */

VOS_UINT32 At_SetSystemInfoEx(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_MMA_QrySystemInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, VOS_TRUE) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SYSINFOEX_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
/* end V7R1 PhaseI Modify */

VOS_UINT32 AT_ConvertCpamRatOrderPara(VOS_UINT8 clientId, MN_MMA_CpamRatTypeUint8 mode,
                                      MN_MMA_CpamRatPrioUint8 acqorder, TAF_MMA_SysCfgPara *sysCfgSetPara,
                                      AT_SYSCFG_RatPrioUint8 *syscfgRatPrio)
{
    TAF_MMA_MultimodeRatCfg *ratOrder = VOS_NULL_PTR;
    AT_SYSCFG_RatPrioUint8   nvimRatPrio;
    ModemIdUint16            modemId = MODEM_ID_0;
    VOS_UINT32               rst;

    ratOrder = &(sysCfgSetPara->multiModeRatCfg);

    rst = AT_GetModemIdFromClient(clientId, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("AT_ConvertCpamRatOrderPara:Get ModemID From ClientID fail,ClientID=%d", clientId);
        return AT_ERROR;
    }

    /* 如果接入优先级不改变则默认从NVIM中获取 */
    /* 由于NVIM中的接入优先级和CPAM的设置的不一样需要转化一下 */
    if (acqorder == MN_MMA_CPAM_RAT_PRIO_NO_CHANGE) {
        nvimRatPrio = AT_GetGuAccessPrioNvim(modemId);
        *syscfgRatPrio = nvimRatPrio;
    } else if (acqorder == MN_MMA_CPAM_RAT_PRIO_GSM) {
        *syscfgRatPrio = AT_SYSCFG_RAT_PRIO_GSM;
    } else {
        *syscfgRatPrio = AT_SYSCFG_RAT_PRIO_WCDMA;
    }

    switch (mode) {
        case MN_MMA_CPAM_RAT_TYPE_WCDMA_GSM:
            if (*syscfgRatPrio == AT_SYSCFG_RAT_PRIO_GSM) {
                /* GSM优先 */
                ratOrder->ratNum      = TAF_PH_MAX_GU_RAT_NUM;
                ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_GSM;
                ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_WCDMA;
                ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;
            } else {
                /* WCDMA优先 */
                ratOrder->ratNum      = TAF_PH_MAX_GU_RAT_NUM;
                ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_WCDMA;
                ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_GSM;
                ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;
            }

            break;

        case MN_MMA_CPAM_RAT_TYPE_GSM:
            /* G单模 */
            ratOrder->ratNum      = 1;
            ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_GSM;
            ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_BUTT;
            ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;
            break;

        case MN_MMA_CPAM_RAT_TYPE_WCDMA:
            /* W单模 */
            ratOrder->ratNum      = 1;
            ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_0] = TAF_MMA_RAT_WCDMA;
            ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_1] = TAF_MMA_RAT_BUTT;
            ratOrder->ratOrder[TAF_MMA_RAT_ORDER_INDEX_2] = TAF_MMA_RAT_BUTT;
            break;

        case MN_MMA_CPAM_RAT_TYPE_NOCHANGE:
            ratOrder->ratNum                  = 0;
            sysCfgSetPara->ratCfgNoChangeType = TAF_MMA_RAT_NO_CHANGE_CMD_CPAM;

            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 At_SetCpamPara(VOS_UINT8 indexNum)
{
    TAF_MMA_SysCfgPara sysCfgSetPara;

    VOS_UINT32      rst;
    AT_ModemNetCtx *netCtx = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&sysCfgSetPara, sizeof(sysCfgSetPara), 0x00, sizeof(TAF_MMA_SysCfgPara));

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    rst = AT_OK;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CPAM_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        g_atParaList[0].paraValue = MN_MMA_CPAM_RAT_TYPE_NOCHANGE;
    }

    if (g_atParaList[1].paraLen == 0) {
        g_atParaList[1].paraValue = MN_MMA_CPAM_RAT_PRIO_NO_CHANGE;
    }

    rst = AT_ConvertCpamRatOrderPara(indexNum, (VOS_UINT8)g_atParaList[0].paraValue,
                                     (VOS_UINT8)g_atParaList[1].paraValue, &sysCfgSetPara, &sysCfgSetPara.userPrio);

    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* stSysCfgSetPara其他参数赋值 */
    if (netCtx->roamFeature == AT_ROAM_FEATURE_ON) {
        sysCfgSetPara.roam = TAF_MMA_ROAM_UNCHANGE;
    } else {
        sysCfgSetPara.roam = AT_ROAM_FEATURE_OFF_NOCHANGE;
    }
    sysCfgSetPara.srvDomain       = TAF_MMA_SERVICE_DOMAIN_NOCHANGE;
    sysCfgSetPara.guBand.bandLow  = TAF_PH_BAND_NO_CHANGE;
    sysCfgSetPara.guBand.bandHigh = 0;
    (VOS_VOID)memset_s(&(sysCfgSetPara.lBand), sizeof(sysCfgSetPara.lBand), 0x00, sizeof(TAF_USER_SetLtePrefBandInfo));
    sysCfgSetPara.lBand.bandInfo[0] = TAF_PH_BAND_NO_CHANGE;

    /* 执行命令操作 */
    if (TAF_MMA_SetSysCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &sysCfgSetPara) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPAM_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

TAF_MMA_ServiceDomainUint8 At_ConvertDetachTypeToServiceDomain(VOS_UINT32 cgcattMode)
{
    switch (cgcattMode) {
        case AT_CGCATT_MODE_PS:
            return TAF_MMA_SERVICE_DOMAIN_PS;

        case AT_CGCATT_MODE_CS:
            return TAF_MMA_SERVICE_DOMAIN_CS;

        case AT_CGCATT_MODE_CS_PS:
            return TAF_MMA_SERVICE_DOMAIN_CS_PS;

        default:
            AT_ERR_LOG("At_ConvertDetachTypeToServiceDomain Detach Type ERROR");
            return TAF_MMA_SERVICE_DOMAIN_CS_PS;
    }
}

TAF_UINT32 At_SetCgcattParaDetach(TAF_UINT8 ucIndex)
{
    TAF_MMA_DetachPara stDetachPara;

    /* 执行Detach操作 */
    (VOS_VOID)memset_s(&stDetachPara, sizeof(stDetachPara), 0x00, sizeof(TAF_MMA_DetachPara));
    stDetachPara.detachCause = TAF_MMA_DETACH_CAUSE_USER_DETACH;

    /* 只有detach mode=3时 可支持三个参数，其他情况都是两个参数 */
    if (g_atParaList[AT_CGCATT_MODE].paraValue == TAF_MMA_ATTACH_TYPE_GPRS_IMSI) {
        if (g_atParaIndex > AT_CGCATT_MODE_EQUALED_3_MAX_PARA_NUM) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (g_atParaList[AT_CGCATT_PS_STATE].paraValue > 1) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (g_atParaList[AT_CGCATT_PS_STATE].paraValue == 1) {
            stDetachPara.detachCause = TAF_MMA_DETACH_CAUSE_SMS_ANTI_ATTACK;
        }
    } else {
        if (g_atParaIndex > AT_CGCATT_PARA_MAX_NUM) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 转换用户的DETACH类型 */
    stDetachPara.detachDomain = At_ConvertDetachTypeToServiceDomain(g_atParaList[1].paraValue);

    if (TAF_MMA_DetachReq(WUEPS_PID_AT, g_atClientTab[ucIndex].clientId, 0, &stDetachPara) == VOS_TRUE) {
        /* 设置当前操作类型 */
        switch (g_atParaList[1].paraValue) {
            case AT_CGCATT_MODE_PS:
                g_atClientTab[ucIndex].cmdCurrentOpt = AT_CMD_CGCATT_PS_DETAACH_SET;
                break;

            case AT_CGCATT_MODE_CS:
                g_atClientTab[ucIndex].cmdCurrentOpt = AT_CMD_CGCATT_CS_DETAACH_SET;
                break;

            default:
                g_atClientTab[ucIndex].cmdCurrentOpt = AT_CMD_CGCATT_PS_CS_DETAACH_SET;
                break;
        }
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_MMA_AttachTypeUint8 At_ConvertCgcattModeToAttachType(VOS_UINT32 cgcattMode)
{
    switch (cgcattMode) {
        case AT_CGCATT_MODE_PS:
            return TAF_MMA_ATTACH_TYPE_GPRS;

        case AT_CGCATT_MODE_CS:
            return TAF_MMA_ATTACH_TYPE_IMSI;

        case AT_CGCATT_MODE_CS_PS:
            return TAF_MMA_ATTACH_TYPE_GPRS_IMSI;

        default:
            AT_ERR_LOG("At_ConvertCgcattModeToAttachType Attach Oper ERROR");
            return TAF_MMA_ATTACH_TYPE_GPRS_IMSI;
    }
}

TAF_UINT32 At_SetCgcattPara(TAF_UINT8 indexNum)
{
    TAF_MMA_AttachTypeUint8 attachType = TAF_MMA_ATTACH_TYPE_BUTT;

    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* detach 处理封装成子函数 */
    if (g_atParaList[0].paraValue == 0) {
        return At_SetCgcattParaDetach(indexNum);
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CGCATT_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 转换用户的ATTACH类型 */
    attachType = At_ConvertCgcattModeToAttachType(g_atParaList[1].paraValue);

    /* 执行Attach操作 */
    if (TAF_MMA_AttachReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, attachType) == VOS_TRUE) {
        /* 设置当前操作类型 */
        switch (g_atParaList[1].paraValue) {
            case AT_CGCATT_MODE_PS:
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGCATT_PS_ATTACH_SET;
                break;

            case AT_CGCATT_MODE_CS:
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGCATT_CS_ATTACH_SET;
                break;

            default:
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGCATT_PS_CS_ATTACH_SET;
                break;
        }
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetSrvstPara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptSetReq atCmd;
    VOS_UINT32                  result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.reqType       = AT_MTA_SET_SRVST_RPT_TYPE;
    atCmd.u.srvstRptFlg = 0;

    if (g_atParaList[0].paraLen != 0) {
        atCmd.u.srvstRptFlg = (VOS_UINT8)g_atParaList[0].paraValue;
    }

    /* 给MTA发送^srvst设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

LOCAL VOS_UINT32 AT_CheckSetFplmnPara(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CFPLMN_PARA_MAX_NUM) {
        return VOS_FALSE;
    }

    if (g_atParaList[0].paraLen != 1) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

LOCAL VOS_UINT32 AT_CheckSetFplmnParaOper(TAF_PH_FplmnOperate *fplmnOperate)
{
    if ((g_atParaList[AT_CFPLMN_OPER].paraLen > AT_CFPLMN_OPER_MAX_VALID_LEN) ||
        (g_atParaList[AT_CFPLMN_OPER].paraLen < AT_CFPLMN_OPER_MIN_VALID_LEN)) {
        return VOS_FALSE;
    }
    /* Mcc */
    if (At_String2Hex(g_atParaList[AT_CFPLMN_OPER].para, AT_MCC_LENGTH, &(fplmnOperate->plmn.mcc)) ==AT_FAILURE) {
        return VOS_FALSE;
    }
    /* Mnc */
    if (At_String2Hex(&g_atParaList[AT_CFPLMN_OPER].para[3], g_atParaList[AT_CFPLMN_OPER].paraLen - AT_MNC_LENGTH,
                      &(fplmnOperate->plmn.mnc)) == AT_FAILURE) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

TAF_UINT32 At_SetFplmnPara(TAF_UINT8 indexNum)
{
    TAF_PH_FplmnOperate fplmnOperate;

    if (AT_CheckSetFplmnPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex == 1) {
        if (g_atParaList[0].paraValue != 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            fplmnOperate.plmn.mcc = 0;
            fplmnOperate.plmn.mnc = 0;
            fplmnOperate.cmdType  = TAF_PH_FPLMN_DEL_ALL;
        }
    } else if (g_atParaIndex == AT_CFPLMN_PARA_MAX_NUM) {
        if (AT_CheckSetFplmnParaOper(&fplmnOperate) != VOS_TRUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        fplmnOperate.plmn.mcc &= 0x0FFF;

        if (g_atParaList[AT_CFPLMN_OPER].paraLen == AT_CFPLMN_OPER_MIN_VALID_LEN) {
            fplmnOperate.plmn.mnc |= 0x0F00;
        }

        fplmnOperate.plmn.mnc &= 0x0FFF;

        if (g_atParaList[0].paraValue == 0) {
            fplmnOperate.cmdType = TAF_PH_FPLMN_DEL_ONE;
        } else if (g_atParaList[0].paraValue == 1) {
            fplmnOperate.cmdType = TAF_PH_FPLMN_ADD_ONE;
        } else {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (TAF_MMA_SetFPlmnInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &fplmnOperate) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFPLMN_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetModePara(TAF_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptSetReq atCmd;
    VOS_UINT32                  result;
#if (FEATURE_LTE == FEATURE_ON)
    L4A_IND_Cfg   l4AIndCfgReq;
    ModemIdUint16 modemId;

    modemId = MODEM_ID_0;
#endif

    /* AT命令参数合法性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.reqType      = AT_MTA_SET_MODE_RPT_TYPE;
    atCmd.u.modeRptFlg = AT_MODE_RESULT_CODE_NOT_REPORT_TYPE;

    if (g_atParaList[0].paraLen != 0) {
        atCmd.u.modeRptFlg = (VOS_UINT8)g_atParaList[0].paraValue;
    }

    /* 给MTA发送^mode设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

#if (FEATURE_LTE == FEATURE_ON)
    result = AT_GetModemIdFromClient(indexNum, &modemId);

    if (result != VOS_OK) {
        AT_ERR_LOG1("At_SetModePara:Get ModemID From ClientID fail,ClientID=%d", indexNum);
        return AT_ERROR;
    }

    if (AT_IsModemSupportRat(modemId, TAF_MMA_RAT_LTE) == VOS_TRUE) {
        (VOS_VOID)memset_s(&l4AIndCfgReq, sizeof(l4AIndCfgReq), 0x00, sizeof(l4AIndCfgReq));
        l4AIndCfgReq.modeBitValid = 1;
        l4AIndCfgReq.modeBit      = g_atParaList[0].paraValue;

        /* 通知L此次mode的设置 */
        AT_SetLIndCfgReq(indexNum, &l4AIndCfgReq);
    }
#endif

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SetCplsPara(VOS_UINT8 indexNum)
{
    VOS_UINT32              rst;
    MN_PH_PrefPlmnTypeUint8 prefPlmnType;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        prefPlmnType = (MN_PH_PrefPlmnTypeUint8)g_atParaList[0].paraValue;
    } else {
        prefPlmnType = MN_PH_PREF_PLMN_UPLMN;
    }

    if (prefPlmnType > MN_PH_PREF_PLMN_HPLMN) {
        return AT_CME_OPERATION_NOT_SUPPORTED;
    }

    /* 发送消息 ID_TAF_MMA_PREF_PLMN_TYPE_SET_REQ 给 MMA 处理 */
    rst = TAF_MMA_SetPrefPlmnTypeReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                     &prefPlmnType); /* 要求设置的优先网络类型 */

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MMA_SET_PREF_PLMN_TYPE;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_SetCpolPara(VOS_UINT8 indexNum)
{
    TAF_PH_SetPrefplmn prefPlmn;
    VOS_UINT32         rst;
    AT_ModemNetCtx    *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&prefPlmn, sizeof(prefPlmn), 0x00, sizeof(prefPlmn));

    /* 参数过多 */
#if (FEATURE_UE_MODE_NR == FEATURE_ON) && (FEATURE_LTE == FEATURE_ON)
    if (g_atParaIndex > AT_UE_MODE_NR_AND_LTE_AT_CPOL_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#elif (FEATURE_LTE == FEATURE_ON) || (FEATURE_UE_MODE_NR == FEATURE_ON)
    if (g_atParaIndex > AT_UE_MODE_NR_OR_LTE_AT_CPOL_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#else
    if (g_atParaIndex > AT_OTHER_MODE_AT_CPOL_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    if ((g_atParaIndex == AT_CPOL_PARA_NUM) && (g_atParaList[AT_CPOL_INDEX].paraLen == 0) &&
        (g_atParaList[AT_CPOL_FORMAT].paraLen != 0)) {
        /*
         * 当前只存在<format>，如果当前对应的<oper>能够获取，则读取命令中<oper>
         * 的format需要改变
         */
        netCtx->cpolFormatType = (AT_COPS_FORMAT_TYPE)g_atParaList[AT_CPOL_FORMAT].paraValue;
        return AT_OK;
    }

    if (netCtx->prefPlmnType > MN_PH_PREF_PLMN_HPLMN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 保存参数到结构中，发送给MMA进行处理 */
    prefPlmn.prefPlmnType     = netCtx->prefPlmnType;
    prefPlmn.atParaIndex      = g_atParaIndex;
    prefPlmn.indexLen         = g_atParaList[AT_CPOL_INDEX].paraLen;
    prefPlmn.index            = g_atParaList[AT_CPOL_INDEX].paraValue;
    prefPlmn.formatLen        = g_atParaList[AT_CPOL_FORMAT].paraLen;
    prefPlmn.format           = g_atParaList[AT_CPOL_FORMAT].paraValue;
    prefPlmn.gsmRatLen        = g_atParaList[AT_CPOL_GSM_ACTN].paraLen;
    prefPlmn.gsmRat           = g_atParaList[AT_CPOL_GSM_ACTN].paraValue;
    prefPlmn.gsmCompactRatLen = g_atParaList[AT_CPOL_GSM_COMPACT_ACTN].paraLen;
    prefPlmn.gsmCompactRat    = g_atParaList[AT_CPOL_GSM_COMPACT_ACTN].paraValue;
    prefPlmn.utrnLen          = g_atParaList[AT_CPOL_UTRAN].paraLen;
    prefPlmn.utrn             = g_atParaList[AT_CPOL_UTRAN].paraValue;

#if (FEATURE_UE_MODE_NR == FEATURE_ON) && (FEATURE_LTE == FEATURE_ON)
    prefPlmn.eutrnLen = g_atParaList[AT_CPOL_E_UTRAN].paraLen;
    prefPlmn.eutrn    = g_atParaList[AT_CPOL_E_UTRAN].paraValue;
    prefPlmn.nrRatLen = g_atParaList[AT_CPOL_RAT].paraLen;
    prefPlmn.nrRat    = g_atParaList[AT_CPOL_RAT].paraValue;
#elif (FEATURE_LTE == FEATURE_ON)
    /* LTE的接入技术以及长度 */
    prefPlmn.eutrnLen = g_atParaList[AT_CPOL_E_UTRAN].paraLen;
    prefPlmn.eutrn    = g_atParaList[AT_CPOL_E_UTRAN].paraValue;
#elif (FEATURE_UE_MODE_NR == FEATURE_ON)
    prefPlmn.usNrRatLen = g_atParaList[AT_CPOL_E_UTRAN].paraLen;
    prefPlmn.ulNrRat    = g_atParaList[AT_CPOL_E_UTRAN].paraValue;
#else
#endif

    if (prefPlmn.formatLen == 0) {
        prefPlmn.formatLen = 1;
        prefPlmn.format    = AT_COPS_NUMERIC_TYPE;
    }

    netCtx->cpolFormatType = (AT_COPS_FORMAT_TYPE)prefPlmn.format;

    if (g_atParaList[AT_CPOL_OPER].paraLen != 0) {
        rst = AT_GetOperatorNameFromParam(&prefPlmn.operNameLen, prefPlmn.operName, sizeof(prefPlmn.operName),
                                          prefPlmn.format);
        if (rst != AT_OK) {
            return rst;
        }
    }

    if (TAF_MMA_SetCpolReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &prefPlmn) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPOL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetQuickStart(TAF_UINT8 indexNum)
{
    AT_PR_LOGI("Rcv Msg");
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue > 1) {
        return AT_ERROR;
    }

    /* 执行命令操作 */
    if (TAF_MMA_SetQuickStartReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, g_atParaList[0].paraValue) ==
        VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CQST_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCopnPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    VOS_UINT16 fromIndex = 0;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_ERROR;
    }
    ret = TAF_MMA_QryCopnInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, fromIndex, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COPN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_GetValidNetScanPowerParam(VOS_INT16 *psCellPower)
{
    VOS_UINT32 cellPower = 0;

    /* power长度大于4或小于3表示无效参数 */
    if ((g_atParaList[AT_NETSCAN_POWER].paraLen > AT_NETSCAN_POWER_PARA_MAX_LEN) ||
        (g_atParaList[AT_NETSCAN_POWER].paraLen < AT_NETSCAN_POWER_PARA_MIN_LEN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串第一个参数不为'-'，即为无效参数 */
    if (g_atParaList[AT_NETSCAN_POWER].para[0] != '-') {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* POWER首字符为'-',获取后面的值 */
    if (atAuc2ul(&g_atParaList[AT_NETSCAN_POWER].para[1],
                 g_atParaList[AT_NETSCAN_POWER].paraLen - 1, &cellPower) != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 由于POWER值为负，所以转换下 */
    *psCellPower = -(VOS_INT16)cellPower;

    /* GU Power取值判断[-110,-47]；LTE/NR Power取值判断[-125,-47] */
    if (*psCellPower > AT_NETSCAN_POWER_MAX_VALUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_NETSCAN_MODE].paraValue == TAF_MMA_NET_SCAN_RAT_MODE_LTE
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        || g_atParaList[AT_NETSCAN_MODE].paraValue == TAF_MMA_NET_SCAN_RAT_MODE_LTE_NRDC ||
        g_atParaList[AT_NETSCAN_MODE].paraValue == TAF_MMA_NET_SCAN_RAT_MODE_NR
#endif
    ) {
        if (*psCellPower < AT_NETSCAN_LTE_POWER_MIN_VALUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        if (*psCellPower < AT_NETSCAN_POWER_MIN_VALUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_OK;
}

VOS_UINT32 AT_GetValidNetScanBandPara(TAF_MMA_BandSet *bandSet)
{
    VOS_UINT8 *para = VOS_NULL_PTR;
    VOS_UINT16 len;
    VOS_UINT16 tmpLen;
    VOS_UINT8  indexNum;
    VOS_UINT8  isNullBand;

    para       = g_atParaList[AT_NETSCAN_BAND].para;
    len        = g_atParaList[AT_NETSCAN_BAND].paraLen;
    isNullBand = VOS_TRUE;
    (VOS_VOID)memset_s(bandSet, sizeof(TAF_MMA_BandSet), 0, sizeof(TAF_MMA_BandSet));

    /* 如果频段为空，则认为是全频段搜索 */
    if (len == 0) {
        if ((g_atParaList[AT_NETSCAN_MODE].paraValue == TAF_MMA_NET_SCAN_RAT_MODE_GSM) ||
            (g_atParaList[AT_NETSCAN_MODE].paraValue == TAF_MMA_NET_SCAN_RAT_MODE_UMTS)) {
            bandSet->band[0] = AT_ALL_GU_BAND;
        } else if (g_atParaList[AT_NETSCAN_MODE].paraValue == TAF_MMA_NET_SCAN_RAT_MODE_LTE) {
            bandSet->band[0] = MN_MMA_LTE_LOW_BAND_ANY;
            bandSet->band[1] = MN_MMA_LTE_HIGH_BAND_ANY;
        } else {
            AT_NORM_LOG1("AT_GetValidNetScanBandPara: <mode>", g_atParaList[AT_NETSCAN_MODE].paraValue);
        }
        return AT_OK;
    }

    /* 转换频段 */
    for (indexNum = 0; indexNum < TAF_MMA_BAND_MAX_LEN; indexNum++) {
        tmpLen = (len < AT_HEX_LEN_OF_ULONG_TYPE) ? len : AT_HEX_LEN_OF_ULONG_TYPE;

        if (At_String2Hex(para + g_atParaList[AT_NETSCAN_BAND].paraLen - (indexNum * AT_HEX_LEN_OF_ULONG_TYPE) - tmpLen,
                          tmpLen, &(bandSet->band[indexNum])) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (bandSet->band[indexNum] != 0) {
            isNullBand = VOS_FALSE;
        }

        if (len <= AT_HEX_LEN_OF_ULONG_TYPE) {
            break;
        }

        len -= AT_HEX_LEN_OF_ULONG_TYPE;
    }

    /* 不允许扫描空BAND */
    if (isNullBand == VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_GetValidNetScanPara(TAF_MMA_NetScanSetPara *netScanSetPara)
{
    /* 检查参数个数,如果超过4个，返回AT_CME_INCORRECT_PARAMETERS */
    if ((g_atParaIndex > AT_NETSCAN_PARA_MAX_NUM) || (g_atParaIndex < AT_NETSCAN_PARA_MIN_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数校验 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置CellNum */
    netScanSetPara->cellNum = (VOS_UINT16)g_atParaList[0].paraValue;

    /* 设置CellPow */
    if (AT_GetValidNetScanPowerParam(&(netScanSetPara->cellPow)) != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 缺省<mode>参数，但不缺省<band>参数，则认为参数错误 */
    if ((g_atParaIndex == AT_NETSCAN_PARA_MAX_NUM) && (g_atParaList[AT_NETSCAN_MODE].paraLen == 0) &&
        (g_atParaList[AT_NETSCAN_BAND].paraLen != 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果用户没输入接入技术，该g_atParaList[2].ulParaValue就是0 */
    if (g_atParaList[AT_NETSCAN_MODE].paraLen == 0) {
        netScanSetPara->rat = TAF_MMA_NET_SCAN_RAT_MODE_GSM;
    } else {
        netScanSetPara->rat = (TAF_MMA_NetScanRatModeUint8)g_atParaList[AT_NETSCAN_MODE].paraValue;
    }

    /* 检查band参数合法性,如果合法,进行转换并记录,如果不合法直接返回失败 */
    return AT_GetValidNetScanBandPara(&netScanSetPara->band);
}

VOS_UINT32 AT_SetNetScanPara(VOS_UINT8 indexNum)
{
    VOS_UINT32             rst;
    TAF_MMA_NetScanSetPara netScanSetPara;

    (VOS_VOID)memset_s(&netScanSetPara, sizeof(netScanSetPara), 0x00, sizeof(netScanSetPara));

    rst = AT_GetValidNetScanPara(&netScanSetPara);
    if (rst != AT_OK) {
        TAF_LOG(WUEPS_PID_AT, VOS_NULL, PS_LOG_LEVEL_ERROR,
                "AT_SetNetScanPara:ERROR: AT_GetValidNetScanPara returns FAILURE");
        return rst;
    }

    rst = TAF_MMA_NetScanReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &netScanSetPara);
    if (rst == VOS_TRUE) {
        /* 指示当前用户的命令操作类型为设置命令 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NETSCAN_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEmRssiCfgPara(VOS_UINT8 indexNum)
{
    TAF_MMA_EmrssicfgReq emRssiCfgPara;

    (VOS_VOID)memset_s(&emRssiCfgPara, sizeof(emRssiCfgPara), 0x00, sizeof(emRssiCfgPara));

    /* 检查是否设置命令 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("At_SetEmRssiCfgPara: Not Set Command!");
        return AT_ERROR;
    }

    /* 检查参数个数 */
    if (g_atParaIndex != AT_EMRSSICFG_PARA_NUM) {
        AT_WARN_LOG("At_SetEmRssiCfgPara: Input parameters go wrong!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将入参封装到结构体中再发起请求 */
    emRssiCfgPara.emRssiCfgRat       = (VOS_UINT8)g_atParaList[0].paraValue;
    emRssiCfgPara.emRssiCfgThreshold = (VOS_UINT8)g_atParaList[1].paraValue;

    /* 发送消息给C核处理 */
    if (TAF_MMA_SetEmRssiCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                &emRssiCfgPara) == VOS_TRUE) {
        /* 返回命令处理挂起状态 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EMRSSICFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("At_SetEmRssiCfgPara: AT send ERROR!");
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCesqPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = TAF_MMA_QryCerssiReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    /* 执行命令操作 */
    if (result == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CESQ_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_InputValueTransToVoiceDomain(VOS_UINT32 value, TAF_MMA_VoiceDomainUint32 *voiceDomain)
{
    VOS_UINT32 rst = VOS_TRUE;

    switch (value) {
        case AT_VOICE_DOMAIN_TYPE_CS_ONLY:
            *voiceDomain = TAF_MMA_VOICE_DOMAIN_CS_ONLY;
            break;

        case AT_VOICE_DOMAIN_TYPE_CS_PREFERRED:
            *voiceDomain = TAF_MMA_VOICE_DOMAIN_CS_PREFERRED;
            break;

        case AT_VOICE_DOMAIN_TYPE_IMS_PS_PREFERRED:
            *voiceDomain = TAF_MMA_VOICE_DOMAIN_IMS_PS_PREFERRED;
            break;

        case AT_VOICE_DOMAIN_TYPE_IMS_PS_ONLY:
            *voiceDomain = TAF_MMA_VOICE_DOMAIN_IMS_PS_ONLY;
            break;

        default:
            *voiceDomain = TAF_MMA_VOICE_DOMAIN_BUTT;
            rst          = VOS_FALSE;
            break;
    }

    return rst;
}

VOS_UINT32 AT_SetCevdpPara(VOS_UINT8 indexNum)
{
    TAF_MMA_VoiceDomainUint32 voiceDomain = TAF_MMA_VOICE_DOMAIN_BUTT;
    VOS_UINT32                rst = VOS_FALSE;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空，协议没有明确规定默认值，此处默认设置为CS ONLY */
    if (g_atParaList[0].paraLen == 0) {
        voiceDomain = TAF_MMA_VOICE_DOMAIN_CS_ONLY;
    } else {
        if (AT_InputValueTransToVoiceDomain(g_atParaList[0].paraValue, &voiceDomain) != VOS_TRUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    rst = TAF_MMA_SetVoiceDomainReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, voiceDomain);
    if (rst != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VOICE_DOMAIN_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 AT_SetCindPara(VOS_UINT8 indexNum)
{
    AT_ModemNetCtx   *netCtx = VOS_NULL_PTR;
    TAF_START_InfoInd startInfoInd;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    (VOS_VOID)memset_s(&startInfoInd, sizeof(startInfoInd), 0x00, sizeof(startInfoInd));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex == 0) {
        return AT_OK;
    }

    /* +CIND=[<ind>] 目前只支持signal的设置 */
    if (g_atParaList[0].paraLen != 0) {
        netCtx->cerssiReportType = (AT_CerssiReportTypeUint8)g_atParaList[0].paraValue;

        startInfoInd.actionType          = TAF_START_EVENT_INFO_FOREVER;
        startInfoInd.signThreshold       = (VOS_UINT8)g_atParaList[0].paraValue;
        startInfoInd.rrcMsgType          = TAF_EVENT_INFO_CELL_SIGN;
        startInfoInd.minRptTimerInterval = netCtx->cerssiMinTimerInterval;

        if (TAF_MMA_SetCindReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &startInfoInd) == VOS_TRUE) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CIND_SET;
            return AT_WAIT_ASYNC_RETURN;
        } else {
            return AT_ERROR;
        }
    }

    return AT_OK;
}

TAF_UINT32 At_SetCrpnPara(TAF_UINT8 indexNum)
{
    TAF_MMA_CrpnQryPara mnMmaCrpnQry;
    errno_t             memResult;

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_OK;
    }

    if (g_atParaIndex != AT_CRPN_PARA_VALID_NUM) {
        AT_WARN_LOG("At_SetCrpnPara: g_atParaIndex != 2.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 若<name_index>或<name>参数为空，则返回失败 */
    if ((g_atParaList[AT_CRPN_NAME_INDEX].paraLen == 0) || (g_atParaList[AT_CRPN_NAME].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查<name>参数长度有效性 */
    if (g_atParaList[AT_CRPN_NAME_INDEX].paraValue == 0) {
        if (g_atParaList[AT_CRPN_NAME].paraLen >= TAF_PH_OPER_NAME_LONG) {
            return AT_CME_TEXT_STRING_TOO_LONG;
        }
    } else if (g_atParaList[AT_CRPN_NAME_INDEX].paraValue == 1) {
        if (g_atParaList[AT_CRPN_NAME].paraLen >= TAF_PH_OPER_NAME_SHORT) {
            return AT_CME_TEXT_STRING_TOO_LONG;
        }
    } else {
        if ((g_atParaList[AT_CRPN_NAME].paraLen < AT_CRPN_NAME_MIN_LEN) ||
            (g_atParaList[AT_CRPN_NAME].paraLen > AT_CRPN_NAME_MAX_LEN)) {
            return AT_CME_TEXT_STRING_TOO_LONG;
        }
    }

    (VOS_VOID)memset_s(&mnMmaCrpnQry, sizeof(mnMmaCrpnQry), 0x00, sizeof(TAF_MMA_CrpnQryPara));

    /* 发送消息结构赋值 */
    mnMmaCrpnQry.currIndex   = 0;
    mnMmaCrpnQry.qryNum      = TAF_PH_CRPN_PLMN_MAX_NUM;
    mnMmaCrpnQry.plmnType    = (VOS_UINT8)g_atParaList[AT_CRPN_NAME_INDEX].paraValue;
    mnMmaCrpnQry.plmnNameLen = g_atParaList[AT_CRPN_NAME].paraLen;
    memResult = memcpy_s(mnMmaCrpnQry.plmnName, sizeof(mnMmaCrpnQry.plmnName), g_atParaList[AT_CRPN_NAME].para,
                         g_atParaList[AT_CRPN_NAME].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mnMmaCrpnQry.plmnName), g_atParaList[AT_CRPN_NAME].paraLen);

    if (TAF_MMA_QryCrpnReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &mnMmaCrpnQry) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CRPN_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

