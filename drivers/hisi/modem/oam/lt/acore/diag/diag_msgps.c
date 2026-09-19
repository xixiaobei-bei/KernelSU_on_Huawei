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




#include <product_config.h>
#include <vos.h>
#include <mdrv.h>
#include <msp.h>
#include "diag_msgps.h"
#include "diag_msgmsp.h"
#include "diag_debug.h"
#include "diag_acore_common.h"
#include "soc_socp_adapter.h"
#include "diag_connect.h"
#include "diag_api.h"

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGPS_C

DIAG_TRANS_HEADER_STRU g_stPSTransHead = {{VOS_NULL, VOS_NULL}, 0};
DIAG_MSGPS_CTRL g_DiagPsCtrl =
{
    .ulChannelNum   = 1,
    .ulChannelId    = SOCP_CODER_SRC_LOM_IND1,
};



VOS_UINT32 diagPsTransProcEntry(DIAG_FRAME_INFO_STRU* pstReq)
{
    return diag_TransReqProcEntry(pstReq, &g_stPSTransHead);
}

VOS_UINT32 diag_PsConnMgr(VOS_UINT8 * pData)
{
    DIAG_CONN_MSG_SEND_T *pstRevMsg;

    pstRevMsg = (DIAG_CONN_MSG_SEND_T *)pData;

    if(ID_MSG_DIAG_CMD_CONNECT_REQ == pstRevMsg->ulMsgId)
    {
        return diag_PsConnect(pData);
    }
    else
    {
        return diag_PsDisconnect(pData);
    }
}

VOS_UINT32 diag_PsConnect(VOS_UINT8 * pData)
{
    return ERR_MSP_SUCCESS;
}

VOS_VOID diag_ConnReset(VOS_VOID)
{
    /* 规避老的hids在建链时候下发disconnect命令，不能将开机log配置清除 */
    if(DIAG_IS_POLOG_ON)
    {
        g_ulDiagCfgInfo = DIAG_CFG_INIT | DIAG_CFG_POWERONLOG;
        diag_info("diag_ConnReset, keep init&poweronlog flag.\n");
    }
    else
    {
        g_ulDiagCfgInfo = DIAG_CFG_INIT;
    }

    return;
}

VOS_UINT32 diag_PsDisconnect(VOS_UINT8 * pData)
{
    return ERR_MSP_SUCCESS;
}
/*****************************************************************************
 Function Name   : diag_PsMsgInit
 Description     : MSP ps部分初始化
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_PsMsgInit(VOS_VOID)
{
    VOS_UINT32 ulRet;

    /* 创建节点保护信号量 Diag Trans Ps */
    ulRet = VOS_SmBCreate("DTP", 1, VOS_SEMA4_FIFO,&g_stPSTransHead.TransSem);
    if(VOS_OK != ulRet)
    {
        diag_error("VOS_SmBCreate failed.\n");
    }

    /* 初始化请求链表 */
    blist_head_init(&g_stPSTransHead.TransHead);

    /*注册message消息回调*/
    DIAG_MsgProcReg(DIAG_MSG_TYPE_PS,diagPsTransProcEntry);

    ulRet = diag_ConnMgrSendFuncReg(DIAG_CONN_ID_ACPU_PS, g_DiagPsCtrl.ulChannelNum, &g_DiagPsCtrl.ulChannelId, diag_PsConnMgr);
    if(ulRet)
    {
        diag_error("acpu ps reg ConnMsg fail(0x%x)\n", ulRet);
    }
}



VOS_VOID DIAG_ShowTransList(VOS_VOID)
{
    LIST_S* me = NULL;

    diag_crit("PS trans header 0x%pK, 0x%pK.\n", g_stPSTransHead.TransHead.next, g_stPSTransHead.TransHead.prev);

    /*添加信号量保护*/
    (VOS_VOID)VOS_SmP(g_stPSTransHead.TransSem,0);

    /* 在链表中查找每个子命令结点*/
    blist_for_each(me, &g_stPSTransHead.TransHead)
    {
        diag_crit("header 0x%pK, 0x%pK.\n", me->next, me->prev);
    }
    (VOS_VOID)VOS_SmV(g_stPSTransHead.TransSem);

    return ;
}

EXPORT_SYMBOL(DIAG_LogShowToFile);
EXPORT_SYMBOL(DIAG_ShowTransList);


