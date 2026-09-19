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

/******************************************************************************
   1 头文件包含
******************************************************************************/
#include "PsAcpuLogFilter.h"
#include "PsLogFilterComm.h"
#include "msp_diag_comm.h"


#define    THIS_FILE_ID        PS_FILE_ID_ACPU_LOG_FILTER_C

/******************************************************************************
   2 外部函数变量声明
******************************************************************************/

/******************************************************************************
   3 私有定义
******************************************************************************/


/******************************************************************************
   4 全局变量定义
******************************************************************************/


PS_OM_LAYER_MSG_REPLACE_CTRL_STRU            g_stAcpuLayerMsgReplaceCtrl =
{
    (VOS_UINT16)VOS_PID_BUTT,
    (VOS_UINT8)VOS_CPU_ID_ACPU,
    {0},
    {0, 0, VOS_NULL_PTR},
    {0, 0, VOS_NULL_PTR}
};

/******************************************************************************
   5 函数实现
******************************************************************************/


/* 防止PC工程编译过程中函数多重定义 */



VOS_VOID* PS_OM_ACpuLayerMsgReplaceCB
(
    VOS_VOID                           *pMsg
)
{
    return PS_OM_CpuLayerMsgCommReplace(pMsg, &g_stAcpuLayerMsgReplaceCtrl);
}


VOS_UINT32 PS_OM_ACpuLayerMsgReplaceMemFreeCB
(
    VOS_UINT32                          ulSendPid,
    VOS_VOID                           *pAddr
)
{
   return VOS_MemFree(ulSendPid, pAddr);
}



VOS_UINT32 PS_OM_LayerMsgReplaceCBReg
(
    VOS_UINT32                                    ulSendPid,
    PS_OM_LAYER_MSG_REPLACE_CB                    pFunc
)
{
    static VOS_UINT32       s_ulFirstRegFlag = 0;

    /* 初次注册，向底软注册回调函数 */
    if (0 == s_ulFirstRegFlag)
    {
        (VOS_VOID)DIAG_LayerMsgMatchFuncReg(PS_OM_ACpuLayerMsgReplaceCB);
        (VOS_VOID)DIAG_LayerMsgMatchNotifyFuncReg(PS_OM_ACpuLayerMsgReplaceMemFreeCB);
        s_ulFirstRegFlag = 0x5A5AA5A5;
    }

    return  PS_OM_LayerMsgReplaceCBCommReg(&g_stAcpuLayerMsgReplaceCtrl, ulSendPid, pFunc);
}


