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


#include <mdrv.h>
#include <mdrv_diag_system.h>
#include "diag_msgbsp.h"
#include "diag_msgmsp.h"
#include "diag_debug.h"
#include "msp_diag_comm.h"
#include "diag_fs_proc.h"
#include "nv_stru_sys.h"
#include "nv_stru_gucnas.h"
#include "acore_nv_stru_gucnas.h"
#include "soc_socp_adapter.h"
#include "diag_message.h"
#include "diag_connect.h"

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGBSP_C

VOS_UINT32 g_DiagHdsCmdNumC = 0;
VOS_UINT32 g_DiagHdsCmdListC[DIAG_HDS_CMD_MAX] = {0};
VOS_UINT32 g_DiagHdsCmdNumNrm = 0;
VOS_UINT32 g_DiagHdsCmdListNrm[DIAG_HDS_CMD_MAX] = {0};
VOS_UINT32 g_DiagHdsCmdNumA = 0;
VOS_UINT32 g_DiagHdsCmdListA[DIAG_HDS_CMD_MAX] = {0};
DIAG_BSP_CTRL   g_DiagBspCtrl =
{
    1,
    SOCP_CODER_SRC_BSP_ACORE,
};

VOS_VOID diag_BspRecvCmdList(MsgBlock* pMsgBlock)
{
    DIAG_BSP_CMDLIST_STRU *pstMsg = (DIAG_BSP_CMDLIST_STRU *)pMsgBlock;

    if( pstMsg->ulLength  <  sizeof(pstMsg->ulMsgId) + sizeof(pstMsg->ulCmdNum) + pstMsg->ulCmdNum * sizeof(VOS_UINT32))
    {
        diag_error("datalen is too small,len:0x%x\n", pstMsg->ulLength);
        return;
    }

    if(MSP_PID_DIAG_AGENT == pMsgBlock->ulSenderPid)
    {
        g_DiagHdsCmdNumC = pstMsg->ulCmdNum;
        VOS_MemCpy_s(g_DiagHdsCmdListC, sizeof(g_DiagHdsCmdListC),  pstMsg->ulCmdList, g_DiagHdsCmdNumC*sizeof(VOS_UINT32));

        diag_crit("Rcv ccore cmd list, num: 0x%x\n",g_DiagHdsCmdNumC);
    }
    else if(MSP_PID_DIAG_NRM_AGENT == pMsgBlock->ulSenderPid)
    {
        g_DiagHdsCmdNumNrm = pstMsg->ulCmdNum;
        VOS_MemCpy_s(g_DiagHdsCmdListNrm, sizeof(g_DiagHdsCmdListNrm),  pstMsg->ulCmdList, g_DiagHdsCmdNumNrm*sizeof(VOS_UINT32));

        diag_crit("Rcv nrm cmd list, num: 0x%x\n", g_DiagHdsCmdNumNrm);
    }
    else
    {
        diag_error("ulSenderPid error 0x%x\n", pMsgBlock->ulSenderPid);
    }
    return;
}

VOS_VOID diag_BspCmdListInit(VOS_VOID)
{
    g_DiagHdsCmdNumA = DIAG_HDS_CMD_MAX;
    mdrv_hds_get_cmdlist(g_DiagHdsCmdListA, &g_DiagHdsCmdNumA);
}

VOS_BOOL diag_BspIsAcoreCmd(VOS_UINT32 ulCmdId)
{
    unsigned int i;

    if(!ulCmdId)
        return VOS_FALSE;

    for(i=0; i<g_DiagHdsCmdNumA; i++)
    {
        if(g_DiagHdsCmdListA[i] == ulCmdId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_BOOL diag_BspIsCcoreCmd(VOS_UINT32 ulCmdId)
{
    unsigned int i;

    if(!ulCmdId)
        return VOS_FALSE;

    for(i=0; i<g_DiagHdsCmdNumC; i++)
    {
        if(g_DiagHdsCmdListC[i] == ulCmdId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_BOOL diag_BspIsNrmCmd(VOS_UINT32 ulCmdId)
{
    unsigned int i;

    if(!ulCmdId)
        return VOS_FALSE;

    for(i=0; i<g_DiagHdsCmdNumNrm; i++)
    {
        if(g_DiagHdsCmdListNrm[i] == ulCmdId)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 diag_BspMsgProc(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_BSP_MSG_A_TRANS_C_STRU*    pstInfo;
    MSP_DIAG_CNF_INFO_STRU          stDiagInfo = {0};
    DIAG_BSP_COMM_CNF_STRU          stCnf = {0};
    DIAG_FRAME_INFO_STRU *          pstDiagHead = pData;
    VOS_UINT32 ulRet = ERR_MSP_INVALID_PARAMETER;
    VOS_UINT32 ulLen = 0;
    VOS_UINT32 acmd_flag = 0;
    VOS_UINT32 ccmd_flag = 0;

    mdrv_diag_PTR(EN_DIAG_PTR_BSP_MSG_PROC, 1, pData->ulCmdId, 0);

    if(DIAG_MSG_TYPE_BSP != pData->stID.pri4b)
    {
        diag_error("Rcv Error MsgId 0x%x\n",pData->ulCmdId);
        return ulRet;
    }

    acmd_flag = diag_BspIsAcoreCmd(pData->ulCmdId);
    ccmd_flag = diag_BspIsCcoreCmd(pData->ulCmdId);
    /*nrmcmd_flag = diag_BspIsNrmCmd(pData->ulCmdId);*/


    if  ((VOS_TRUE == acmd_flag) && (VOS_FALSE == ccmd_flag))
    {
        ulRet = mdrv_hds_msg_proc((VOS_VOID *)pData);
        if(ulRet != 0)
        {
            diag_error("acore msg_proc fail!\n",__FUNCTION__);
        }

        return ulRet;
    }
    else if ((VOS_FALSE == acmd_flag) && (VOS_TRUE == ccmd_flag))
    {
        /*通知ccore*/
        DIAG_MSG_BSP_ACORE_CFG_PROC(ulLen, pData, pstInfo, ulRet);
        return VOS_OK;
    }
    else if ((VOS_TRUE == acmd_flag) && (VOS_TRUE == ccmd_flag))
    {
        /*A核处理成功后通知ccore，A核处理不成功直接向工具回复*/
        ulRet = mdrv_hds_msg_proc((VOS_VOID*)pData);
        if(ulRet != 0)
        {
            diag_error("comm_acore msg_proc fail!\n",__FUNCTION__);
            return ulRet;
        }

        DIAG_MSG_BSP_ACORE_CFG_PROC(ulLen, pData, pstInfo, ulRet);
        return VOS_OK;
    }
    else
    {
        //todo...
    }

/*
    if(VOS_TRUE == acmd_flag)
    {
        ulRet = mdrv_hds_msg_proc((VOS_VOID *)pData);
        if(ulRet != 0)
        {
            diag_error("acore msg_proc fail!\n");
        }

        return ulRet;
    }

    if(VOS_TRUE == ccmd_flag)
    {
*/
        /*通知ccore*/
/*        DIAG_MSG_BSP_ACORE_CFG_PROC(ulLen, pData, pstInfo, ulRet);
        return VOS_OK;
    }

    if(VOS_TRUE == nrmcmd_flag)
    {
        DIAG_MSG_BSP_CFG_ACOE_TO_NRM(ulLen, pData, pstInfo, ulRet);
        return VOS_OK;
        //todo...
    }
*/
    return ulRet;

DIAG_ERROR:

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BSP;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnf, pstDiagHead);

    stCnf.ulRet = ERR_MSP_FAILURE;

    ulRet = DIAG_MsgReport(&stDiagInfo, &stCnf, sizeof(stCnf));

    return ulRet;

}

VOS_UINT32 diag_BspConnMgr(VOS_UINT8 *pData)
{
    DIAG_CONN_MSG_SEND_T *pstRevMsg;
    VOS_UINT32 ulRet;
    DIAG_CONNECT_RESULT stCnf;

    pstRevMsg = (DIAG_CONN_MSG_SEND_T *)pData;

    stCnf.ulChannelId = g_DiagBspCtrl.ulChannelId;
    if(ID_MSG_DIAG_CMD_CONNECT_REQ == pstRevMsg->ulMsgId)
    {
        ulRet = (VOS_UINT32)mdrv_hds_printlog_conn();
        if(ulRet)
        {
            diag_error("printlog conn error, ret:0x%x\n", ulRet);
            stCnf.ulResult = ulRet;
            return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
        }

        stCnf.ulResult = (VOS_UINT32)mdrv_hds_translog_conn();
        diag_crit("translog_conn ret:0x%x\n", stCnf.ulResult);
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
    }
    else
    {

        ulRet = (VOS_UINT32)mdrv_hds_printlog_disconn();
        if(ulRet)
        {
            diag_error("printlog disconn error, ret:0x%x\n", ulRet);
            stCnf.ulResult = ulRet;
            return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
        }

        stCnf.ulResult = (VOS_UINT32)mdrv_hds_translog_disconn();
        diag_crit("translog disconn ret:0x%x\n", stCnf.ulResult);
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_BSP, pstRevMsg->ulSn, g_DiagBspCtrl.ulChannelNum, &stCnf);
    }
}
/*****************************************************************************
 Function Name   : diag_BspMsgInit
 Description     : MSP dsp部分初始化
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_BspMsgInit(VOS_VOID)
{
    VOS_UINT32 ulRet;

    /*注册message消息回调*/
    DIAG_MsgProcReg(DIAG_MSG_TYPE_BSP, diag_BspMsgProc);
    mdrv_hds_cnf_register((hds_cnf_func)DIAG_MsgReport);
    diag_fsInit();
    diag_BspCmdListInit();

    ulRet = diag_ConnMgrSendFuncReg(DIAG_CONN_ID_ACPU_BSP, g_DiagBspCtrl.ulChannelNum, &g_DiagBspCtrl.ulChannelId, diag_BspConnMgr);
    if(ulRet)
    {
        diag_error("bsp reg ConnMsg fail(0x%x)\n", ulRet);
    }
}




