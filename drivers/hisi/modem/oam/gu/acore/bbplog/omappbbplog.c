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

                  版权所有 (C), 2001-2013, 华为技术有限公司

 ******************************************************************************
  文 件 名      : ombbplogapp.c
  版 本 号      : 初稿
  作    者      : 蒋德彬 00174725
  生成日期      : 2012年11月16日
  最近修改      :
  功能描述      : BBP采数与可维可测相关函数实现
  函数列表      :
  修改历史      :
  1.日    期    : 2012年11月16日
    作    者    : 蒋德彬 00174725
    修改内容    : 创建文件

******************************************************************************/
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"
#include "omprivate.h"
#include "ombbplog.h"
#include "phyoaminterface.h"
#include "pamappom.h"



/*lint -e767 修改人：蒋德彬 */
#define    THIS_FILE_ID        PS_FILE_ID_OM_APP_BBP_LOG_C
/*lint +e767 修改人：蒋德彬 */

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 记录从底软获取到的DUMP内存地址和大小 */
BBP_DUMP_GET_FIX_DDR_ADDR               g_stBbpDumpFixDdr;

/*****************************************************************************
  3 外部函数声明
*****************************************************************************/


/*****************************************************************************
  4 消息处理函数映射表
*****************************************************************************/


/*****************************************************************************
  5 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : Om_AcpuQueryDumpMsgProc
 功能描述  : CCpu查询Dump信息响应函数
 输入参数  : pMsg

 输出参数  : NA

 返 回 值  : VOS_VOID

 修改历史      :
  1.日    期   : 2014年10月08日
    作    者   : d00212987
    修改内容   : BBP采数项目新增
*****************************************************************************/
VOS_VOID Om_AcpuQueryDumpMsgProc(MsgBlock* pMsg)
{
    BBP_DUMP_MSG_STRU                  *pstBbpDumpMsg;
    VOS_UINT16                          usPrimId;

    usPrimId = *(VOS_UINT16*)(pMsg->aucValue);
    if (BBP_QUERY_DUMP_INFO_REQ == usPrimId)
    {
        /* 申请消息发送给CCPU */
        pstBbpDumpMsg = (BBP_DUMP_MSG_STRU*)VOS_AllocMsg(ACPU_PID_PAM_OM,
                                sizeof(BBP_DUMP_MSG_STRU) - VOS_MSG_HEAD_LENGTH);

        /* 分配消息失败 */
        if (VOS_NULL_PTR == pstBbpDumpMsg)
        {
            return;
        }

        pstBbpDumpMsg->ulReceiverPid           = CCPU_PID_PAM_OM;
        pstBbpDumpMsg->usPrimId                = BBP_QUERY_DUMP_INFO_CNF;
        pstBbpDumpMsg->stDumpDdrInfo.ulPhyAddr = g_stBbpDumpFixDdr.ulPhyAddr;
        pstBbpDumpMsg->stDumpDdrInfo.ulPhySize = g_stBbpDumpFixDdr.ulPhySize;

        (VOS_VOID)VOS_SendMsg(ACPU_PID_PAM_OM, pstBbpDumpMsg);
    }

    return;
}

/*lint -save -e102 -e10 -e2 -e40 -e533 -e31 -e830 -e522 -e718 -e746 -e702 -e565  -e64 -e23 -e63 -e26 -e578 -e132 -e19 -e18 -e34 -e35 -e713*/
extern unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);

/*****************************************************************************
* 函 数 名  : Om_QueryDumpPhyAddr
*
* 功能描述  : 在代码编译阶段将CMD LINE中的DumpPhyAddr地址解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
VOS_INT __init Om_QueryDumpPhyAddr(VOS_CHAR *pucChar)
{
    g_stBbpDumpFixDdr.ulPhyAddr = (VOS_UINT32)simple_strtoul(pucChar, VOS_NULL, VOS_NULL);

    /* 物理地址是32位的实地址并且是8字节对齐的 */
    if ((VOS_NULL != (g_stBbpDumpFixDdr.ulPhyAddr % 4))
        || (VOS_NULL == g_stBbpDumpFixDdr.ulPhyAddr))
    {
        g_stBbpDumpFixDdr.ulPhyAddr = VOS_NULL;
    }

    return 0;
}

early_param("mdmreglogbase", Om_QueryDumpPhyAddr);

/*****************************************************************************
* 函 数 名  : SocpLogTimeoutParse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的TIMEOUT大小参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
VOS_INT __init Om_QueryDumpPhySize(VOS_CHAR *pucChar)
{
    /* Buffer的大小以Byte为单位 */
    g_stBbpDumpFixDdr.ulPhySize = (VOS_UINT32)simple_strtoul(pucChar, VOS_NULL, VOS_NULL);

    return 0;
}

early_param("mdmreglogsize", Om_QueryDumpPhySize);
/*lint -restore */

/*****************************************************************************
 函 数 名  : Om_AcpuPhyMsgProc
 功能描述  :
 输入参数  : MsgBlock* pMsg
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年1月22日
    作    者   : s00207770
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID  Om_AcpuPhyMsgProc( MsgBlock* pMsg )
{
    WPHY_OAM_QUERY_SAMPLE_REQ_STRU     *pstReqMsg;
    WPHY_OAM_QUERY_SAMPLE_CNF_STRU     *pstCnfMsg;

    pstReqMsg   = (WPHY_OAM_QUERY_SAMPLE_REQ_STRU*)pMsg;

    if (ID_OAM_WPHY_QUERY_SAMPLE_REQ == pstReqMsg->enMsgId)
    {
        pstCnfMsg   = (WPHY_OAM_QUERY_SAMPLE_CNF_STRU*)VOS_AllocMsg(ACPU_PID_PAM_OM, sizeof(WPHY_OAM_QUERY_SAMPLE_CNF_STRU) - VOS_MSG_HEAD_LENGTH);

        if (VOS_NULL_PTR == pstCnfMsg)
        {
            PS_LOG(ACPU_PID_PAM_OM, 0, PS_PRINT_ERROR,
                "Om_AcpuPhyMsgProc: Msg alloc Error.\n");
            return;
        }

        pstCnfMsg->ulReceiverPid    = pstReqMsg->ulSenderPid;
        pstCnfMsg->enMsgId          = ID_WPHY_OAM_QUERY_SAMPLE_CNF;

        if (VOS_NULL_PTR != g_stBbpDumpFixDdr.ulPhyAddr)
        {
            pstCnfMsg->esRslt       = VOS_TRUE;
            pstCnfMsg->ulPhyAddr    = g_stBbpDumpFixDdr.ulPhyAddr;
            pstCnfMsg->ulPhySize    = g_stBbpDumpFixDdr.ulPhySize;
        }
        else
        {
            pstCnfMsg->esRslt   = VOS_FALSE;
        }

        if (VOS_OK != VOS_SendMsg(ACPU_PID_PAM_OM, pstCnfMsg))
        {
            PS_LOG(ACPU_PID_PAM_OM, 0, PS_PRINT_ERROR,
                "Om_AcpuPhyMsgProc: Send Msg Error.\n");
        }

        return;
    }

    return;
}


