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

#include "at_safety_mm_set_cmd_proc.h"
#include "AtParse.h"
#include "securec.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "at_check_func.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAFETY_MM_SET_CMD_PROC_C

#define AT_CLCK_PASSWD_INDEX 2
#define AT_CLCK_CLASS 3
#define AT_ME_PS_DEACTIVE_TYPE 0
#define AT_ME_PS_ACTIVE_TYPE 1
#define AT_CLCK_FAC 0
#define AT_CLCK_MODE 1
#define AT_CPWD_FAC 0
#define AT_CPWD_OLDPWD 1
#define AT_CPWD_NEWPWD 2
#define AT_CLCK_PARA_MAX_NUM 4
#define AT_CPIN_PARA_MAX_NUM 2
#define AT_CPWD_PARA_MAX_NUM 3
#define AT_CPIN_PARA_NUM 2
#define AT_CMLCK_PARA_MAX_NUM 4
#define AT_CMLCK_PASSWD_INDEX 2
#define AT_CMLCK_DATA 3
#define AT_CMD_TYPE_PERSONALISATION_SET 0
#define AT_CMD_TYPE_PERSONALISATION_RETRIEVE 1

LOCAL VOS_UINT32 AT_ProcClckP2Type(TAF_UINT8 indexNum);
LOCAL VOS_UINT32 AT_ProcClckScType(TAF_UINT8 indexNum);
LOCAL VOS_UINT32 AT_ProcClckFdType(TAF_UINT8 indexNum);
LOCAL VOS_UINT32 AT_ProcClckPsType(TAF_UINT8 indexNum);
#if (FEATURE_PHONE_SC == FEATURE_ON)
LOCAL VOS_UINT32 AT_SetClckFacEqualPNPUPPPara(TAF_UINT8 indexNum);
#endif

static const AT_ClckFacTypeMatchEntity g_ClckFacTypeMatchTbl[] = {
    { AT_CLCK_P2_TYPE,  AT_ProcClckP2Type},
    { AT_CLCK_SC_TYPE,  AT_ProcClckScType},
    { AT_CLCK_FD_TYPE,  AT_ProcClckFdType},
    { AT_CLCK_PS_TYPE,  AT_ProcClckPsType},
#if (FEATURE_PHONE_SC == FEATURE_ON)
    { AT_CLCK_PN_TYPE,  AT_SetClckFacEqualPNPUPPPara},
    { AT_CLCK_PU_TYPE,  AT_SetClckFacEqualPNPUPPPara},
    { AT_CLCK_PP_TYPE,  AT_SetClckFacEqualPNPUPPPara},
#endif
};

#if (FEATURE_PHONE_SC == FEATURE_ON)
TAF_UINT32 AT_SetClckFacEqualPNPUPPPara(TAF_UINT8 indexNum)
{
    AT_MTA_SimlockunlockReq simlockUnlockSetReq;

    switch (g_atParaList[1].paraValue) {
        case AT_CLCK_MODE_UNLOCK: /* 设置<mode>为0，则进行锁网锁卡解锁操作 */
        {
            (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00, sizeof(AT_MTA_SimlockunlockReq));
            simlockUnlockSetReq.category = AT_GetSimlockUnlockCategoryFromClck();

            /* <password>参数检查 */
            if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen != AT_SIMLOCKUNLOCK_PWD_PARA_LEN) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            /* 密码的合法性检查，密码为16个“0”-“9”ASCII字符,密码由ASCII码转换为十进制数字 */
            if (At_AsciiNum2Num(simlockUnlockSetReq.password, g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                                g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen) != AT_SUCCESS) {
                (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00,
                    sizeof(AT_MTA_SimlockunlockReq));
                return AT_CME_INCORRECT_PARAMETERS;
            }

            /* 发送跨核消息到C核, 解锁锁网锁卡 */
            if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                       ID_AT_MTA_SIMLOCKUNLOCK_SET_REQ, (VOS_UINT8 *)&simlockUnlockSetReq,
                                       sizeof(AT_MTA_SimlockunlockReq), I0_UEPS_PID_MTA) != TAF_SUCCESS) {
                (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00,
                    sizeof(AT_MTA_SimlockunlockReq));
                return AT_ERROR;
            }

            /* 设置当前操作类型 */
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_SIMLOCKUNLOCK;
            (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00,
                sizeof(AT_MTA_SimlockunlockReq));
            return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
        }
        case AT_CLCK_MODE_LOCK: /* 设置<mode>为1，则直接返回错误信息 */
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        case AT_CLCK_MODE_QUERY_STATUS: /* 设置<mode>为2，则进行锁网锁卡功能查询操作 */
        {
            return AT_OK;
        }
        default: {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
}
#endif

LOCAL VOS_UINT32 AT_ProcClckP2Type(TAF_UINT8 indexNum)
{
    return AT_CME_INCORRECT_PARAMETERS;
}

VOS_UINT32 AT_CheckPinLength(VOS_UINT16 len, VOS_UINT8 *pin)
{
    if (pin == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_CheckPinLength():PIN is NULL PTR");
        return VOS_ERR;
    }

    if ((len > TAF_PH_PINCODELENMAX) || (len < TAF_PH_PINCODELENMIN)) {
        AT_ERR_LOG("AT_CheckPinLength():PIN LENGTH is Error");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_CheckPinValue(VOS_UINT16 len, VOS_UINT8 *pin)
{
    VOS_UINT8 i;

    if (pin == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_CheckPinValue(): PIN is NULL PTR");
        return VOS_ERR;
    }

    for (i = 0; i < len; i++) {
        if ((pin[i] > '9') || (pin[i] < '0')) {
            AT_ERR_LOG("AT_CheckPinValue(): PIN Value is Error");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_CheckPinAvail(VOS_UINT16 len, VOS_UINT8 *pin)
{
    if (AT_CheckPinLength(len, pin) != VOS_OK) {
        AT_ERR_LOG("AT_CheckPinAvail(): PIN Length is Error");
        return VOS_ERR;
    } else {
        if (AT_CheckPinValue(len, pin) != VOS_OK) {
            AT_ERR_LOG("AT_CheckPinAvail():PIN Value is Error");
            return VOS_ERR;
        }
    }
    return VOS_OK;
}

VOS_UINT32 AT_SetPinCmdType(TAF_PH_PinData *pinData)
{
    switch (g_atParaList[AT_CLCK_MODE].paraValue) {
        case 0: /* pin disable */
            pinData->cmdType = TAF_PIN_DISABLE;
            if (AT_CheckPinAvail(g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen,
                                 g_atParaList[AT_CLCK_PASSWD_INDEX].para) != VOS_OK) {
                (VOS_VOID)memset_s(pinData, sizeof(TAF_PH_PinData), 0xFF, sizeof(TAF_PH_PinData));
                (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                    0xff, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
                return AT_CME_INCORRECT_PARAMETERS;
            }
            break;
        /* pin enable */
        case 1:
            pinData->cmdType = TAF_PIN_ENABLE;
            if (AT_CheckPinAvail(g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen,
                                 g_atParaList[AT_CLCK_PASSWD_INDEX].para) != VOS_OK) {
                (VOS_VOID)memset_s(pinData, sizeof(TAF_PH_PinData), 0xFF, sizeof(TAF_PH_PinData));
                (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                     0xff, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
                return AT_CME_INCORRECT_PARAMETERS;
            }
            break;

        default:
            pinData->cmdType = TAF_PIN_QUERY;
            break;
    }
    return AT_OK;
}

LOCAL VOS_UINT32 AT_ProcClckScType(TAF_UINT8 indexNum)
{
    TAF_PH_PinData  pinData;
    errno_t         memResult;
    VOS_UINT32      result;

    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen > TAF_PH_PINCODELENMAX) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));

    /* 设置<passwd> */
    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen != 0) {
        memResult = memcpy_s((TAF_CHAR *)pinData.oldPin, sizeof(pinData.oldPin),
            (TAF_CHAR *)g_atParaList[AT_CLCK_PASSWD_INDEX].para, g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pinData.oldPin), g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
    }

    pinData.pinType = TAF_SIM_PIN;

    /* 设置CmdType */
    result = AT_SetPinCmdType(&pinData);
    if (result != AT_OK) {
        return result;
    }

    /* 执行命令操作 */
    if (Taf_PhonePinHandle(g_atClientTab[indexNum].clientId, 0, &pinData) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_PIN_HANDLE;
        (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0xff, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0xff, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_ERROR;
    }
}

LOCAL VOS_UINT32 AT_ProcClckFdType(TAF_UINT8 indexNum)
{
    VOS_UINT8  pin[TAF_PH_PINCODELENMAX];
    VOS_UINT32 cmdType;
    errno_t    memResult;
    VOS_UINT32 result;

    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen > TAF_PH_PINCODELENMAX) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(pin, sizeof(pin), 0xFF, sizeof(pin));

    /* 设置<passwd> */
    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen != 0) {
        memResult = memcpy_s((TAF_CHAR *)pin, sizeof(pin), (TAF_CHAR *)g_atParaList[AT_CLCK_PASSWD_INDEX].para,
            g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pin), g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
    }

    cmdType = g_atParaList[1].paraValue;

    /* 执行命令操作，当前只有FDN相关的操作，AT命令并不支持BDN功能 */
    if (cmdType == SI_PIH_FDN_BDN_ENABLE) {
        result = SI_PIH_FdnEnable(g_atClientTab[indexNum].clientId, 0, pin, TAF_PH_PINCODELENMAX);
    } else if (cmdType == SI_PIH_FDN_BDN_DISABLE) {
        result = SI_PIH_FdnDisable(g_atClientTab[indexNum].clientId, 0, pin, TAF_PH_PINCODELENMAX);
    } else {
        result = SI_PIH_FdnBdnQuery(g_atClientTab[indexNum].clientId, 0, SI_PIH_FDN_QUERY);
    }

    if (result == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_PIN_HANDLE;
        (VOS_VOID)memset_s(pin, sizeof(pin), 0xFF, sizeof(pin));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0xff, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(pin, sizeof(pin), 0xFF, sizeof(pin));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0xff, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_ERROR;
    }
}

VOS_VOID AT_SetPersonalisationCmdType(TAF_ME_PersonalisationData *mePersonalisationData)
{
    switch (g_atParaList[1].paraValue) {
        case AT_ME_PS_DEACTIVE_TYPE:
            mePersonalisationData->cmdType = TAF_ME_PERSONALISATION_DEACTIVE;
            break;

        case AT_ME_PS_ACTIVE_TYPE:
            mePersonalisationData->cmdType = TAF_ME_PERSONALISATION_ACTIVE;
            break;

        default:
            mePersonalisationData->cmdType = TAF_ME_PERSONALISATION_QUERY;
            break;
    }
}

VOS_UINT32 AT_ProcClckPsType(TAF_UINT8 indexNum)
{
    TAF_ME_PersonalisationData  mePersonalisationData;
    errno_t                     memResult;

    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen > TAF_PH_PINCODELENMAX) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData), 0x00, sizeof(TAF_ME_PersonalisationData));

    /* 设置<passwd> */
    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen != 0) {
        memResult = memcpy_s((VOS_CHAR *)mePersonalisationData.oldPwd, sizeof(mePersonalisationData.oldPwd),
            (VOS_CHAR *)g_atParaList[AT_CLCK_PASSWD_INDEX].para, g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mePersonalisationData.oldPwd),
                            g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
    }

    /* 设置PinType */
    mePersonalisationData.mePersonalType = TAF_SIM_PERSONALISATION;

    /* 设置CmdType */
    AT_SetPersonalisationCmdType(&mePersonalisationData);

    /* 执行命令操作 */
    if (Taf_MePersonalisationHandle(g_atClientTab[indexNum].clientId, 0, &mePersonalisationData) ==
        AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_PIN_HANDLE;
        (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData),
                 0x00, sizeof(TAF_ME_PersonalisationData));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0x00, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData),
                 0x00, sizeof(TAF_ME_PersonalisationData));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0x00, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_ERROR;
    }
}

VOS_UINT32 At_CheckClckPara(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParaList[AT_CLCK_FAC].paraLen == 0) || (g_atParaList[AT_CLCK_MODE].paraLen == 0)) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CLCK_PARA_MAX_NUM) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_VOID At_SsPrint2Code(TAF_SS_CODE *ssCode, TAF_UINT8 type)
{
    switch (type) {
        case AT_CLCK_AO_TYPE:
            *ssCode = TAF_BAOC_SS_CODE;
            break;

        case AT_CLCK_OI_TYPE:
            *ssCode = TAF_BOIC_SS_CODE;
            break;

        case AT_CLCK_OX_TYPE:
            *ssCode = TAF_BOICEXHC_SS_CODE;
            break;

        case AT_CLCK_AI_TYPE:
            *ssCode = TAF_BAIC_SS_CODE;
            break;

        case AT_CLCK_IR_TYPE:
            *ssCode = TAF_BICROAM_SS_CODE;
            break;

        case AT_CLCK_AB_TYPE:
            *ssCode = TAF_ALL_BARRING_SS_CODE;
            break;

        case AT_CLCK_AG_TYPE:
            *ssCode = TAF_BARRING_OF_OUTGOING_CALLS_SS_CODE;
            break;

        default:
            *ssCode = TAF_BARRING_OF_INCOMING_CALLS_SS_CODE;
            break;
    }
}

VOS_UINT32 AT_ProcClckModeUnlock(TAF_SS_ErasessReq *para, TAF_UINT8 indexNum)
{
    if (TAF_DeactivateSSReq(g_atClientTab[indexNum].clientId, 0, para) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_LOCK;
        (VOS_VOID)memset_s(para, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para), 0x00,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(para, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para), 0x00,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcClckModelock(TAF_SS_ErasessReq *para, TAF_UINT8 indexNum)
{
    if (TAF_ActivateSSReq(g_atClientTab[indexNum].clientId, 0, para) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_UNLOCK;
        (VOS_VOID)memset_s(para, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para), 0x00,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(para, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para), 0x00,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcClckModeQueryStatus(TAF_SS_ErasessReq *para, TAF_UINT8 indexNum)
{
    para->opBsService             = 0;
    g_atClientTab[indexNum].temp = g_atParaList[AT_CLCK_CLASS].paraValue;
    if (TAF_InterrogateSSReq(g_atClientTab[indexNum].clientId, 0, para) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_QUERY;
        (VOS_VOID)memset_s(para, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para), 0x00,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(para, sizeof(TAF_SS_ErasessReq), 0x00, sizeof(TAF_SS_ErasessReq));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para), 0x00,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcClckOtherType(TAF_UINT8 indexNum)
{
    TAF_SS_ErasessReq para;
    errno_t    memResult;

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));

    /* 设置<password> */
    if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen > 0) {
        if (g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen != TAF_SS_MAX_PASSWORD_LEN) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        para.opPassword = VOS_TRUE;
        memResult = memcpy_s((VOS_CHAR *)para.password, sizeof(para.password),
                             (VOS_CHAR *)g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                             g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(para.password), g_atParaList[AT_CLCK_PASSWD_INDEX].paraLen);
    }

    /* 设置SsCode */
    At_SsPrint2Code(&para.ssCode, (TAF_UINT8)g_atParaList[0].paraValue);

    /* 设置<class> */
    if (AT_FillSsBsService(&para, &g_atParaList[AT_CLCK_CLASS]) != VOS_OK) {
        (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));
        (VOS_VOID)memset_s(g_atParaList[AT_CLCK_PASSWD_INDEX].para,
                 sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para),
                 0x00, sizeof(g_atParaList[AT_CLCK_PASSWD_INDEX].para));
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    switch (g_atParaList[1].paraValue) {
        case 0: /* <mode>: unlock */
            return AT_ProcClckModeUnlock(&para, indexNum);

        case 1: /* <mode>: lock */
            return AT_ProcClckModelock(&para, indexNum);

        default: /* <mode>: query status */
            return AT_ProcClckModeQueryStatus(&para, indexNum);
    }
}

TAF_UINT32 At_SetClckPara(TAF_UINT8 indexNum)
{
    AT_ClckFacTypeMatchFunc     facTypeMatchFunc = VOS_NULL_PTR;
    VOS_UINT32                  loop;
    VOS_UINT32                  itemsNum;

    if (At_CheckClckPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    itemsNum = sizeof(g_ClckFacTypeMatchTbl) / sizeof(AT_ClckFacTypeMatchEntity);
    for (loop = 0; loop < itemsNum; loop++) {
        if (g_ClckFacTypeMatchTbl[loop].facType == g_atParaList[AT_CLCK_FAC].paraValue) {
            facTypeMatchFunc = g_ClckFacTypeMatchTbl[loop].facTypeMatchFunc;
            break;
        }
    }

    if (facTypeMatchFunc != VOS_NULL_PTR) {
        return facTypeMatchFunc(indexNum);
    } else {
        return AT_ProcClckOtherType(indexNum);
    }
}

VOS_UINT32 AT_CheckCpwdPara(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParaList[AT_CPWD_FAC].paraLen    == 0) || (g_atParaList[1].paraLen == 0) ||
        (g_atParaList[AT_CPWD_NEWPWD].paraLen == 0)) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CPWD_PARA_MAX_NUM) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_VOID AT_SetCpwdPinType(TAF_PH_PinData *pinData)
{
    /* 设置PinType */
    if (g_atParaList[0].paraValue == AT_CLCK_P2_TYPE) {
        pinData->pinType = TAF_SIM_PIN2;
    } else {
        pinData->pinType = TAF_SIM_PIN;
    }
}

VOS_UINT32 AT_SetCpwdPinValue(TAF_PH_PinData *pinData)
{
    errno_t         memResult;
    VOS_UINT32      i;

    /* 设置<pin> */
    if (g_atParaList[AT_CPWD_OLDPWD].paraLen != 0) {
        for (i = 0; i < g_atParaList[AT_CPWD_OLDPWD].paraLen; i++) {
            if ((g_atParaList[AT_CPWD_OLDPWD].para[i] > '9') || (g_atParaList[AT_CPWD_OLDPWD].para[i] < '0')) {
                (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para,
                         (VOS_SIZE_T)sizeof(g_atParaList[AT_CPWD_OLDPWD].para), 0xff,
                         (VOS_SIZE_T)sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }

        memResult = memcpy_s((TAF_VOID *)pinData->oldPin, sizeof(pinData->oldPin),
                             (TAF_VOID *)g_atParaList[AT_CPWD_OLDPWD].para,
                             g_atParaList[AT_CPWD_OLDPWD].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pinData->oldPin), g_atParaList[AT_CPWD_OLDPWD].paraLen);
    }

    /* 设置<newpin> */
    if (g_atParaList[AT_CPWD_NEWPWD].paraLen != 0) {
        for (i = 0; i < g_atParaList[AT_CPWD_NEWPWD].paraLen; i++) {
            if ((g_atParaList[AT_CPWD_NEWPWD].para[i] > '9') || (g_atParaList[AT_CPWD_NEWPWD].para[i] < '0')) {
                (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                         0xff, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }

        memResult = memcpy_s(pinData->newPin, sizeof(pinData->newPin),
                             g_atParaList[AT_CPWD_NEWPWD].para, g_atParaList[AT_CPWD_NEWPWD].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pinData->newPin), g_atParaList[AT_CPWD_NEWPWD].paraLen);
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckP2ScPara(VOS_VOID)
{
    if ((g_atParaList[AT_CPWD_OLDPWD].paraLen > TAF_PH_PINCODELENMAX) ||
        (g_atParaList[AT_CPWD_NEWPWD].paraLen > TAF_PH_PINCODELENMAX) ||
        (g_atParaList[AT_CPWD_OLDPWD].paraLen < TAF_PH_PINCODELENMIN) ||
        (g_atParaList[AT_CPWD_NEWPWD].paraLen < TAF_PH_PINCODELENMIN)) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_ProcCpwdFacP2ScType(TAF_UINT8 indexNum)
{
    TAF_PH_PinData  pinData;
    VOS_UINT32      result;

    if (AT_CheckP2ScPara() == AT_ERROR) {
        return AT_ERROR;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));

    result = AT_SetCpwdPinValue(&pinData);
    if (result != AT_OK) {
        return result;
    }

    AT_SetCpwdPinType(&pinData);

    /* 设置CmdType */
    pinData.cmdType = TAF_PIN_CHANGE;

    /* 执行命令操作 */
    if (Taf_PhonePinHandle(g_atClientTab[indexNum].clientId, 0, &pinData) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPWD_CHANGE_PIN;
        (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para, sizeof(g_atParaList[AT_CPWD_OLDPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para, sizeof(g_atParaList[AT_CPWD_OLDPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcCpwdFacPsType(TAF_UINT8 indexNum)
{
    TAF_ME_PersonalisationData  mePersonalisationData;
    errno_t                     memResult;

    if ((g_atParaList[AT_CPWD_OLDPWD].paraLen > TAF_PH_PINCODELENMAX) ||
        (g_atParaList[AT_CPWD_NEWPWD].paraLen > TAF_PH_PINCODELENMAX)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData), 0x00,
             sizeof(mePersonalisationData));

    /* 设置<oldpwd> */
    memResult = memcpy_s((VOS_UINT8 *)mePersonalisationData.oldPwd,
                         sizeof(mePersonalisationData.oldPwd), g_atParaList[AT_CPWD_OLDPWD].para,
                         g_atParaList[AT_CPWD_OLDPWD].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mePersonalisationData.oldPwd), g_atParaList[AT_CPWD_OLDPWD].paraLen);

    /* 设置<newpwd> */
    memResult = memcpy_s((VOS_CHAR *)mePersonalisationData.newPwd,
                         sizeof(mePersonalisationData.newPwd), (VOS_CHAR *)g_atParaList[AT_CPWD_NEWPWD].para,
                         g_atParaList[AT_CPWD_NEWPWD].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mePersonalisationData.newPwd), g_atParaList[AT_CPWD_NEWPWD].paraLen);

    /* 设置MePersonalType */
    mePersonalisationData.mePersonalType = TAF_SIM_PERSONALISATION;

    /* 设置CmdType */
    mePersonalisationData.cmdType = TAF_ME_PERSONALISATION_PWD_CHANGE;

    /* 执行命令操作 */
    if (Taf_MePersonalisationHandle(g_atClientTab[indexNum].clientId, 0, &mePersonalisationData) ==
        AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPWD_CHANGE_PIN;
        (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData),
                 0x00, sizeof(mePersonalisationData));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para, sizeof(g_atParaList[AT_CPWD_OLDPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData),
                 0x00,sizeof(mePersonalisationData));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para, sizeof(g_atParaList[AT_CPWD_OLDPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcCpwdFacOtherType(TAF_UINT8 indexNum)
{
    TAF_SS_RegpwdReq    para;
    errno_t             memResult;

    if ((g_atParaList[AT_CPWD_OLDPWD].paraLen > TAF_SS_MAX_PASSWORD_LEN) ||
        (g_atParaList[AT_CPWD_NEWPWD].paraLen > TAF_SS_MAX_PASSWORD_LEN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));

    /* 设置<oldpwd> */
    memResult = memcpy_s((TAF_CHAR *)para.oldPwdStr, sizeof(para.oldPwdStr),
                         (TAF_CHAR *)g_atParaList[AT_CPWD_OLDPWD].para, g_atParaList[AT_CPWD_OLDPWD].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(para.oldPwdStr), g_atParaList[AT_CPWD_OLDPWD].paraLen);

    /* 设置<newpwd> */
    memResult = memcpy_s((TAF_CHAR *)para.newPwdStr, sizeof(para.newPwdStr),
                         (TAF_CHAR *)g_atParaList[AT_CPWD_NEWPWD].para, g_atParaList[AT_CPWD_NEWPWD].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(para.newPwdStr), g_atParaList[AT_CPWD_NEWPWD].paraLen);

    /* 设置<newpwd> */
    memResult = memcpy_s((TAF_CHAR *)para.newPwdStrCnf, sizeof(para.newPwdStrCnf),
                         (TAF_CHAR *)g_atParaList[AT_CPWD_NEWPWD].para, g_atParaList[AT_CPWD_NEWPWD].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(para.newPwdStrCnf), g_atParaList[AT_CPWD_NEWPWD].paraLen);

    /* 设置SsCode */
    At_SsPrint2Code(&para.ssCode, (TAF_UINT8)g_atParaList[AT_CPWD_FAC].paraValue);

    /* 执行命令操作 */
    if (TAF_RegisterPasswordReq(g_atClientTab[indexNum].clientId, 0, &para) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPWD_SET;
        (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para, sizeof(g_atParaList[AT_CPWD_OLDPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(&para, sizeof(para), 0x00, sizeof(para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_OLDPWD].para, sizeof(g_atParaList[AT_CPWD_OLDPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_OLDPWD].para));
        (VOS_VOID)memset_s(g_atParaList[AT_CPWD_NEWPWD].para, sizeof(g_atParaList[AT_CPWD_NEWPWD].para),
                 0x00, sizeof(g_atParaList[AT_CPWD_NEWPWD].para));
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCpwdPara(TAF_UINT8 indexNum)
{
    if (AT_CheckCpwdPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 设置<fac> */
    switch (g_atParaList[AT_CPWD_FAC].paraValue) {
        case AT_CLCK_P2_TYPE:
        case AT_CLCK_SC_TYPE:
            return AT_ProcCpwdFacP2ScType(indexNum);
        case AT_CLCK_PS_TYPE:
            return AT_ProcCpwdFacPsType(indexNum);
        default:
            return AT_ProcCpwdFacOtherType(indexNum);
    }
}

TAF_UINT32 At_CheckCpinPara(AT_CmdIndex cmdIndex)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CPIN_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    /* 如果<pin>过长或过短，直接返回错误 */
    if ((g_atParaList[0].paraLen > TAF_PH_PINCODELENMAX) || (g_atParaList[0].paraLen < TAF_PH_PINCODELENMIN)) {
        if ((*systemAppConfig != SYSTEM_APP_ANDROID) || (cmdIndex != AT_CMD_CPIN)) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            /* AP-MODEM形态下，当参数个数为1时, 第一个参数允许长度为16 */
            if ((g_atParaList[0].paraLen != TAF_PH_UNLOCKCODELEN) || (g_atParaIndex != 1)) {
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }

    /* 如果两个参数时，<newpin>过长或过短，直接返回错误 */
    if (g_atParaIndex == AT_CPIN_PARA_NUM) {
        if ((g_atParaList[1].paraLen > TAF_PH_PINCODELENMAX) || (g_atParaList[1].paraLen < TAF_PH_PINCODELENMIN)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetCpinPara(TAF_UINT8 indexNum)
{
    errno_t          memResult;
    TAF_UINT32       result;
    MN_PH_SetCpinReq setCPinReq;

    /* 参数检查 */
    result = At_CheckCpinPara(AT_CMD_CPIN);
    if (result != AT_SUCCESS) {
        return result;
    }

    setCPinReq.para0Len = g_atParaList[0].paraLen;
    setCPinReq.para1Len = g_atParaList[1].paraLen;
    if (setCPinReq.para0Len > 0) {
        memResult = memcpy_s(setCPinReq.para0, sizeof(setCPinReq.para0), g_atParaList[0].para, setCPinReq.para0Len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(setCPinReq.para0), setCPinReq.para0Len);
    }
    if (setCPinReq.para1Len > 0) {
        memResult = memcpy_s(setCPinReq.para1, sizeof(setCPinReq.para1), g_atParaList[1].para, setCPinReq.para1Len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(setCPinReq.para1), setCPinReq.para1Len);
    }

    /* 通过TAF_MSG_MMA_SET_PIN 消息来设置CPIN */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, At_GetOpId(), TAF_MSG_MMA_SET_PIN,
                                    (VOS_UINT8 *)&setCPinReq, sizeof(setCPinReq), I0_WUEPS_PID_MMA);

    if (result == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MSG_MMA_SET_PIN;
        (VOS_VOID)memset_s(&setCPinReq, sizeof(setCPinReq), 0x0, sizeof(setCPinReq));
        (VOS_VOID)memset_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), 0x0, sizeof(g_atParaList[0].para));
        (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x0, sizeof(g_atParaList[1].para));
        return AT_WAIT_ASYNC_RETURN;
    }

    (VOS_VOID)memset_s(&setCPinReq, sizeof(setCPinReq), 0x0, sizeof(setCPinReq));
    (VOS_VOID)memset_s(g_atParaList[0].para, sizeof(g_atParaList[0].para), 0x0, sizeof(g_atParaList[0].para));
    (VOS_VOID)memset_s(g_atParaList[1].para, sizeof(g_atParaList[1].para), 0x0, sizeof(g_atParaList[1].para));
    return AT_ERROR;
}

/*
 * Description: ^CARDLOCK
 * History:
 *  1.Date: 2008-03-15
 *    Author: luojian
 *    Modification: Created function
 */
TAF_UINT32 At_SetCardlockPara(TAF_UINT8 indexNum)
{
    TAF_ME_PersonalisationData mePersonalisationData;
    errno_t                    memResult;

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->blockListFlg == (VOS_TRUE)) {
        return AT_ERROR;
    }
#endif

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <unlock_code>必须为长度为8的数字字符串 */
    if (g_atParaList[0].paraLen == TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX) {
        if (At_CheckNumString(g_atParaList[0].para, TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData), 0x00,
                sizeof(TAF_ME_PersonalisationData));
            /* 复制用户解锁码 */
            memResult = memcpy_s(mePersonalisationData.oldPwd, sizeof(mePersonalisationData.oldPwd),
                                 g_atParaList[0].para, TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mePersonalisationData.oldPwd), TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);
        }
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 安全命令类型为校验密码 */
    mePersonalisationData.cmdType = TAF_ME_PERSONALISATION_VERIFY;
    /* 锁卡操作为锁网络 */
    mePersonalisationData.mePersonalType = TAF_OPERATOR_PERSONALISATION;
    /* 执行命令操作 */
    if (Taf_MePersonalisationHandle(g_atClientTab[indexNum].clientId, 0, &mePersonalisationData) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARD_LOCK_SET;
        (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData), 0x00,
            sizeof(TAF_ME_PersonalisationData));
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData), 0x00,
            sizeof(TAF_ME_PersonalisationData));
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCpin2Para(TAF_UINT8 indexNum)
{
    errno_t        memResult;
    TAF_PH_PinData pinData;
    TAF_UINT32     result;

    /* 参数检查 */
    result = At_CheckCpinPara(AT_CMD_CPIN2);
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&pinData, sizeof(pinData), 0xFF, sizeof(pinData));

    /* 设置<pin> */
    if (g_atParaList[0].paraLen != 0) {
        memResult = memcpy_s((TAF_VOID *)pinData.oldPin, sizeof(pinData.oldPin), (TAF_VOID *)g_atParaList[0].para,
                             g_atParaList[0].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pinData.oldPin), g_atParaList[0].paraLen);
    }

    /* 设置<newpin> */
    if (g_atParaList[1].paraLen != 0) {
        memResult = memcpy_s((TAF_VOID *)pinData.newPin, sizeof(pinData.newPin), (TAF_VOID *)g_atParaList[1].para,
                             g_atParaList[1].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(pinData.newPin), g_atParaList[1].paraLen);
    }

    /* 设置CmdType */
    if (g_atParaList[1].paraLen == 0) {
        pinData.pinType = TAF_SIM_PIN2;
        pinData.cmdType = TAF_PIN_VERIFY;
    } else {
        pinData.pinType = TAF_SIM_PUK2;
        pinData.cmdType = TAF_PIN_UNBLOCK;
    }

    /* 执行命令操作 */
    if (Taf_PhonePinHandle(g_atClientTab[indexNum].clientId, 0, &pinData) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        if (g_atParaList[1].paraLen == 0) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPIN2_VERIFY_SET;
        } else {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPIN2_UNBLOCK_SET;
        }
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_CheckCmlckPara(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CMLCK_PARA_MAX_NUM) {
        return VOS_FALSE;
    }

    /* data长度大于15，直接返回参数错误 */
    if (g_atParaList[AT_CMLCK_DATA].paraLen > TAF_MAX_IMSI_LEN) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
/*
 * Description: +CLCK=<fac>,<action>[,<passwd>[,<data>]]
 * History:
 *  1.Date: 2007-10-06
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCmlckPara(TAF_UINT8 indexNum)
{
    errno_t    memResult;
    TAF_UINT32 ulresult;

    if (AT_CheckCmlckPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch (g_atParaList[0].paraValue) {
        case AT_CMLCK_PS_TYPE: {
            TAF_ME_PersonalisationData mePersonalisationData;

            if (g_atParaList[AT_CMLCK_PASSWD_INDEX].paraLen > TAF_PH_PINCODELENMAX) {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            /* 初始化 */
            (VOS_VOID)memset_s(&mePersonalisationData, sizeof(mePersonalisationData), 0x00, sizeof(TAF_ME_PersonalisationData));

            /* 设置<passwd> */
            if (g_atParaList[AT_CMLCK_PASSWD_INDEX].paraLen != 0) {
                memResult = memcpy_s((TAF_CHAR *)mePersonalisationData.oldPwd, sizeof(mePersonalisationData.oldPwd),
                                     (VOS_CHAR *)g_atParaList[AT_CMLCK_PASSWD_INDEX].para,
                                     g_atParaList[AT_CMLCK_PASSWD_INDEX].paraLen);
                TAF_MEM_CHK_RTN_VAL(memResult, sizeof(mePersonalisationData.oldPwd),
                                    g_atParaList[AT_CMLCK_PASSWD_INDEX].paraLen);
            }

            /* 设置PinType */
            mePersonalisationData.mePersonalType = TAF_SIM_PERSONALISATION;

            if (g_atParaList[AT_CMLCK_DATA].paraLen != 0) {
                ulresult = At_AsciiNum2Num(
                    mePersonalisationData.unMePersonalisationSet.SimPersionalisationStr.simPersonalisationStr,
                    g_atParaList[AT_CMLCK_DATA].para, g_atParaList[AT_CMLCK_DATA].paraLen);
                if (ulresult != AT_SUCCESS) {
                    return AT_CME_INCORRECT_PARAMETERS;
                }

                mePersonalisationData.unMePersonalisationSet.SimPersionalisationStr.dataLen =
                    (VOS_UINT8)g_atParaList[AT_CMLCK_DATA].paraLen;
            }

            /* 设置CmdType */
            switch (g_atParaList[1].paraValue) {
                case AT_CMD_TYPE_PERSONALISATION_SET:
                    if (g_atParaList[AT_CMLCK_DATA].paraLen == 0) {
                        return AT_CME_INCORRECT_PARAMETERS;
                    }
                    mePersonalisationData.cmdType = TAF_ME_PERSONALISATION_SET;

                    break;

                case AT_CMD_TYPE_PERSONALISATION_RETRIEVE:
                    mePersonalisationData.cmdType = TAF_ME_PERSONALISATION_RETRIEVE;

                    break;

                default:
                    return AT_CME_INCORRECT_PARAMETERS;
            }

            /* 执行命令操作 */
            if (Taf_MePersonalisationHandle(g_atClientTab[indexNum].clientId, 0, &mePersonalisationData) ==
                AT_SUCCESS) {
                /* 设置当前操作类型 */
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCK_PIN_HANDLE;
                return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
            } else {
                return AT_ERROR;
            }
        }

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }
}

VOS_UINT32 At_SetCpdwPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 通过给C CPU发ID_TAF_MMA_CPLS_SET_REQ消息通知MT下电 */
    rst = TAF_MMA_MtPowerDownReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MMA_MT_POWER_DOWN;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

#endif

