/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include "at_mbb_cmd.h"
#include "securec.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_MBB_CMD_C

/* 初始化维护特性控制开关 */
VOS_VOID AT_InitMtTrustListCtrl(VOS_VOID)
{
    AT_MtTrustListCtrl        *mtTrustCtrl = VOS_NULL_PTR;
    TAF_NVIM_MtListAndEidCtrl  nvimMtCtrl;
    VOS_UINT32                 result;

    /* 获取维护特性控制开关上下文地址 */
    mtTrustCtrl = AT_GetCommMtTrustCtrlAddr();
    (VOS_VOID)memset_s(mtTrustCtrl, sizeof(AT_MtTrustListCtrl), 0x00, sizeof(AT_MtTrustListCtrl));

    /* 读取NV中获取MT语音短信Trust名单控制开关 */
    (VOS_VOID)memset_s(&nvimMtCtrl, sizeof(TAF_NVIM_MtListAndEidCtrl), 0, sizeof(TAF_NVIM_MtListAndEidCtrl));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MT_LIST_AND_EID_CTRL, &nvimMtCtrl, sizeof(TAF_NVIM_MtListAndEidCtrl));

    if (result != NV_OK) {
        AT_ERR_LOG("AT_InitMtTrustListCtrl: Read NV fail.");
    } else {
        /* NV读取成功，更新MT语音短信Trust名单控制开关到AT SDC中 */
        mtTrustCtrl->mtSmsTrustListEnable  = nvimMtCtrl.mtSmsTrustListEnable;
        mtTrustCtrl->mtCallTrustListEnable = nvimMtCtrl.mtCallTrustListEnable;
    }
}

/* 获取有效号码 */
const VOS_UINT8* AT_GetValidNum(const VOS_UINT8 *trustNum, VOS_UINT32 trustNumLen, VOS_UINT8 *trustNumPrefix)
{
    if (trustNum[0] == '+') {
        *trustNumPrefix = VOS_TRUE;
        return (trustNum + 1);
    }
    if (trustNumLen >= AT_TRUSTLIST_NUM_PREFIX_MAX_LEN) {
        if ((trustNum[0] == '0') && (trustNum[1] == '0')) {
            *trustNumPrefix = VOS_TRUE;
            return (trustNum + AT_TRUSTLIST_NUM_PREFIX_MAX_LEN);
        }
    }
    return trustNum;
}

/* 号码匹配 */
VOS_UINT32 AT_TrustNumCmp(const VOS_UINT8 *checkNum, VOS_UINT32 checkNumLen,
    const VOS_UINT8 *nvTrustNum, VOS_UINT32 nvTrustNumLen)
{
    const VOS_UINT8 *checkNumTmp      = VOS_NULL_PTR;
    const VOS_UINT8 *nvTrustNumTmp    = VOS_NULL_PTR;
    VOS_UINT8        checkNumPrefix   = VOS_FALSE;
    VOS_UINT8        nvTrustNumPrefix = VOS_FALSE;

    if ((nvTrustNumLen < AT_TRUSTLIST_NUM_MIN_LEN) || (nvTrustNumLen > (AT_TRUSTLIST_NUM_PARA_MAX_LEN + 1)) ||
        (checkNumLen < AT_TRUSTLIST_NUM_MIN_LEN) || (checkNumLen > (AT_TRUSTLIST_NUM_PARA_MAX_LEN + 1))) {
        AT_ERR_LOG("AT_TrustNumCmp: Length is invalid.");
        return VOS_FALSE;
    }
    nvTrustNumTmp = AT_GetValidNum(nvTrustNum, nvTrustNumLen, &nvTrustNumPrefix);
    checkNumTmp = AT_GetValidNum(checkNum, checkNumLen, &checkNumPrefix);
    if ((nvTrustNumTmp == VOS_NULL_PTR) || (checkNumTmp == VOS_NULL_PTR)) {
        AT_ERR_LOG("AT_TrustNumCmp: Get address is invalid.");
        return VOS_FALSE;
    }
    if ((VOS_StrCmp((VOS_CHAR *)nvTrustNumTmp, (VOS_CHAR *)checkNumTmp) == 0) && (checkNumPrefix == nvTrustNumPrefix)) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

/* 检查输入的号码是否已存在 */
VOS_UINT32 AT_CheckDeduplicationTrustNum(const TAF_NVIM_NumType *nvTrustList, VOS_UINT32 trustMaxNum)
{
    VOS_UINT32 num;
    VOS_UINT32 result;
    VOS_UINT32 nvTrustNumLen;

    for (num = 0; num < trustMaxNum; num++) {
        nvTrustNumLen = VOS_StrLen((VOS_CHAR *)nvTrustList[num].number);
        if (nvTrustNumLen != 0) {
            result = AT_TrustNumCmp(g_atParaList[1].para, g_atParaList[1].paraLen,
                                    nvTrustList[num].number, nvTrustNumLen);
            if (result == VOS_TRUE) {
                AT_ERR_LOG("AT_CheckDeduplicationTrustNum: Number already exists.");
                return VOS_TRUE;
            }
        }
    }
    return VOS_FALSE;
}

