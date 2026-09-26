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
#include "at_test_phy_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_PHY_RSLT_PROC_C
VOS_UINT16 AT_TasTestOut(MTA_AT_TasTestQryCnf *mtaAtTasTestReqCnf)
{
    VOS_UINT32 i; /* 循环用 */
    VOS_UINT16 length = 0;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "level index: %d (%d, %d, %d, %d)%s", mtaAtTasTestReqCnf->currLevel,
        mtaAtTasTestReqCnf->levelInfo.uhwSrcAntTimeLength, mtaAtTasTestReqCnf->levelInfo.uhwDestAntTimeLength,
        mtaAtTasTestReqCnf->levelInfo.srcAntTxPowerGain, mtaAtTasTestReqCnf->levelInfo.destAntTxPowerGain, g_atCrLf);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "level table:%s", g_atCrLf);

    if (mtaAtTasTestReqCnf->usedTable.uhwLevelNum > MTA_AT_MAX_STATEII_LEVEL_ITEM) {
        mtaAtTasTestReqCnf->usedTable.uhwLevelNum = MTA_AT_MAX_STATEII_LEVEL_ITEM;
    }

    /* 打印表格 */
    for (i = 0; i < mtaAtTasTestReqCnf->usedTable.uhwLevelNum; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "            (%d, %d, %d, %d)%s",
            mtaAtTasTestReqCnf->usedTable.levelItem[i].uhwSrcAntTimeLength,
            mtaAtTasTestReqCnf->usedTable.levelItem[i].uhwDestAntTimeLength,
            mtaAtTasTestReqCnf->usedTable.levelItem[i].srcAntTxPowerGain,
            mtaAtTasTestReqCnf->usedTable.levelItem[i].destAntTxPowerGain, g_atCrLf);
    }
    return length;
}

VOS_UINT32 AT_RcvMtaTasTestQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *rcvMsg             = VOS_NULL_PTR;
    MTA_AT_TasTestQryCnf *mtaAtTasTestReqCnf = VOS_NULL_PTR;
    VOS_UINT16            length;

    /* 初始化局部变量 */
    rcvMsg             = (AT_MTA_Msg *)msg;
    mtaAtTasTestReqCnf = (MTA_AT_TasTestQryCnf *)rcvMsg->content;
    length             = 0;

    /* 格式化上报命令 */
    if (mtaAtTasTestReqCnf->result != MTA_AT_RESULT_NO_ERROR) {
        /* 命令结果 *AT_ERROR */
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    } else {
        /* 命令结果 *AT_OK */
        length                  = AT_TasTestOut(mtaAtTasTestReqCnf);
        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }
}

