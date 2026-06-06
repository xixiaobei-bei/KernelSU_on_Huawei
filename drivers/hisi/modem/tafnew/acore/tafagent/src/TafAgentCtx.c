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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include  "TafAgentCtx.h"
#include  "TafAgentLog.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define    THIS_FILE_ID                 PS_FILE_ID_TAF_AGENT_CTX_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

TAFAGENT_CTX_STRU                       g_stTafAgentCtx = {0};
TAF_AGENT_STATS_INFO_STRU               g_stTafAgentStats = {0};

/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_UINT32 Taf_Agent_InitCtx(VOS_VOID)
{
    g_stTafAgentCtx.pucTafAcpuCnfMsg = VOS_NULL_PTR;

    /* 分配互斥信号量 */
    if (VOS_OK != VOS_SmMCreate("SYNC", VOS_SEMA4_FIFO, &g_stTafAgentCtx.hTafAcpuSyncSem))
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "Create aps acpu sycn sem failed!");
        TAF_AGENT_DBG_CREATE_MUTEX_SEM_FAIL_NUM(1);
        g_stTafAgentCtx.ulInitFlg = VOS_FALSE;
        return VOS_ERR;
    }
    else
    {
        TAF_AGENT_DBG_SAVE_MUTEX_SEM_ID(g_stTafAgentCtx.hTafAcpuSyncSem);
    }

    /* 分配二进制信号量 */
    if (VOS_OK != VOS_SmBCreate( "CNF", 0, VOS_SEMA4_FIFO, &g_stTafAgentCtx.hTafAcpuCnfSem))
    {
        TAFAGENT_ERROR_LOG(ACPU_PID_TAFAGENT, "Create aps acpu cnf sem failed!");
        TAF_AGENT_DBG_CREATE_BINARY_SEM_FAIL_NUM(1);
        g_stTafAgentCtx.ulInitFlg = VOS_FALSE;
        return VOS_ERR;
    }
    else
    {
        TAF_AGENT_DBG_SAVE_BINARY_SEM_ID(g_stTafAgentCtx.hTafAcpuCnfSem);
    }

    g_stTafAgentCtx.ulInitFlg = VOS_TRUE;

    TAF_AGENT_SET_ACPU_CNF_SEM_LOCK_FLG(VOS_FALSE);

    TAF_AGENT_ClearMsg();

    return VOS_OK;
}


VOS_SEM TAF_AGENT_GetTafAcpuSyncSem(VOS_VOID)
{
    return g_stTafAgentCtx.hTafAcpuSyncSem;
}


VOS_SEM TAF_AGENT_GetTafAcpuCnfSem(VOS_VOID)
{
    return g_stTafAgentCtx.hTafAcpuCnfSem;
}


VOS_UINT8* TAF_AGENT_GetTafAcpuCnfMsg(VOS_VOID)
{
    return g_stTafAgentCtx.pucTafAcpuCnfMsg;
}


VOS_VOID TAF_AGENT_SetTafAcpuCnfMsg(VOS_UINT8* pMsg)
{
    g_stTafAgentCtx.pucTafAcpuCnfMsg = pMsg;
}


VOS_VOID TAF_AGENT_ClearMsg(VOS_VOID)
{
    TAF_MEM_SET_S(g_stTafAgentCtx.aucMsg, sizeof(g_stTafAgentCtx.aucMsg), 0x00, sizeof(g_stTafAgentCtx.aucMsg));
}


VOS_UINT8* TAF_AGENT_SaveMsg(
    VOS_UINT8                          *pucMsg,
    VOS_UINT32                          ulLen
)
{
    if ((TAFAGENT_APS_MAX_MSG_LEN - VOS_MSG_HEAD_LENGTH) >= ulLen)
    {
        /* 备份回复消息 */
        TAF_MEM_CPY_S(g_stTafAgentCtx.aucMsg, sizeof(g_stTafAgentCtx.aucMsg), pucMsg, ulLen + VOS_MSG_HEAD_LENGTH);

        return &(g_stTafAgentCtx.aucMsg[0]);
    }
    else
    {
        PS_PRINTF("TAF_AGENT_SaveMsg-->msg len too big\n");
        return VOS_NULL_PTR;
    }
}


VOS_VOID TAF_AGENT_ShowStats(VOS_VOID)
{
    PS_PRINTF_INFO("TAF AGENT STATS START:                   \n");
    PS_PRINTF_INFO("ulInitFlg                              %d\n", g_stTafAgentCtx.ulInitFlg);
    PS_PRINTF_INFO("hTafAcpuSyncSem                        %u\n", g_stTafAgentCtx.hTafAcpuSyncSem); /*lint !e559 */
    PS_PRINTF_INFO("hTafAcpuCnfSem                         %u\n", g_stTafAgentCtx.hTafAcpuCnfSem); /*lint !e559 */
    PS_PRINTF_INFO("ulMutexSemId                           %u\n", g_stTafAgentStats.ulMutexSemId); /*lint !e559 */
    PS_PRINTF_INFO("ulBinarySemId                          %u\n", g_stTafAgentStats.ulBinarySemId); /*lint !e559 */
    PS_PRINTF_INFO("ulCreateMutexSemFailNum                %d\n", g_stTafAgentStats.ulCreateMutexSemFailNum);
    PS_PRINTF_INFO("ulCreateBinarySemFailNum               %d\n", g_stTafAgentStats.ulCreateBinarySemFailNum);
    PS_PRINTF_INFO("ulLockMutexSemFailNum                  %d\n", g_stTafAgentStats.ulLockMutexSemFailNum);
    PS_PRINTF_INFO("ulLockBinarySemFailNum                 %d\n", g_stTafAgentStats.ulLockBinarySemFailNum);
    PS_PRINTF_INFO("ulLastMutexSemErrRslt                  %x\n", g_stTafAgentStats.ulLastMutexSemErrRslt);
    PS_PRINTF_INFO("ulLastBinarySemErrRslt                 %x\n", g_stTafAgentStats.ulLastBinarySemErrRslt);
    PS_PRINTF_INFO("ulSyncMsgIsNullNum                     %d\n", g_stTafAgentStats.ulSyncMsgIsNullNum);
    PS_PRINTF_INFO("ulSyncMsgNotMatchNum                   %d\n", g_stTafAgentStats.ulSyncMsgNotMatchNum);

    PS_PRINTF_INFO("TAF AGENT STATS END.                     \n");

}



