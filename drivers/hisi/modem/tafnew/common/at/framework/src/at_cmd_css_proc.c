/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "at_cmd_css_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "ps_common_def.h"
#include "at_event_report.h"
#include "at_custom_lnas_rslt_proc.h"
#include "at_custom_event_report.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CMD_CSS_PROC_C

/* AT与CSS模块间消息处理函数指针 */
static const AT_CSS_MsgProFunc g_atCssMsgTab[] = {
    /* 消息ID */ /* 消息处理函数 */
    { ID_CSS_AT_MCC_INFO_SET_CNF, AT_RcvCssMccInfoSetCnf },
    { ID_CSS_AT_MCC_VERSION_INFO_CNF, AT_RcvCssMccVersionQryCnf },
    { ID_CSS_AT_QUERY_MCC_INFO_NOTIFY, AT_RcvCssMccNotify },

    { ID_CSS_AT_BLOCK_CELL_LIST_SET_CNF, AT_RcvCssBlockCellListSetCnf },
    { ID_CSS_AT_BLOCK_CELL_LIST_QUERY_CNF, AT_RcvCssBlockCellListQryCnf },
    { ID_CSS_AT_BLOCK_CELL_MCC_NOTIFY, AT_RcvCssBlockCellMccNotify },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    { ID_CSS_AT_PLMN_DEPLOYMENT_INFO_SET_CNF, AT_RcvCssNwDeploymentSetCnf },
#endif
    { ID_CSS_AT_CLOUD_DATA_SET_CNF, AT_RcvCssCloudDataSetCnf },
    { ID_CSS_AT_CLOUD_DATA_REPORT, AT_RcvCssCloudDataReport },
};

VOS_VOID AT_ProcCssMsg(struct MsgCB *msg)
{
    CSS_AT_InterfaceMsg *msgTemp = VOS_NULL_PTR;
    VOS_UINT32           i;
    VOS_UINT32           msgCnt;
    VOS_UINT32           msgId;
    VOS_UINT32           rst;

    /* 从g_astAtProcMsgFromImsaTab中获取消息个数 */
    msgCnt  = sizeof(g_atCssMsgTab) / sizeof(AT_CSS_MsgProFunc);
    msgTemp = (CSS_AT_InterfaceMsg *)msg;

    /* 从消息包中获取MSG ID */
    msgId = msgTemp->msgData.msgId;

    /* g_astAtProcMsgFromCssTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atCssMsgTab[i].msgId == msgId) {
            rst = g_atCssMsgTab[i].procMsgFunc((struct MsgCB *)msgTemp);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcCssMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcCssMsg: Msg Id is invalid!");
    }
}

