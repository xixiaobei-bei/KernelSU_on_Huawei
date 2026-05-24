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
**************************************************************************** */
#include <product_config.h>
#include <mdrv_diag_system.h>
#include <soc_socp_adapter.h>
#include <bsp_socp.h>
#include "diag_port_manager.h"
#include "OmCommonPpm.h"
#include "diag_system_debug.h"
#include "scm_common.h"
#include "scm_ind_dst.h"
#include "scm_ind_src.h"
#include "scm_cnf_dst.h"
#include "scm_debug.h"


/* ****************************************************************************
  2 全局变量定义
**************************************************************************** */
SCM_CODER_DEST_CFG_STRU     g_astSCMCnfCoderDstCfg=
{
    SCM_CHANNEL_UNINIT,
    SOCP_CODER_DST_OM_CNF,
    SCM_CODER_DST_CNF_SIZE,
    SCM_CODER_DST_THRESHOLD,
    SOCP_TIMEOUT_TRF_SHORT,
    NULL,
    NULL,
    NULL
};

extern OM_ACPU_DEBUG_INFO g_stAcpuDebugInfo;
extern u32 g_DiagLogLevel;


u32 scm_malloc_cnf_dst_buff(void)
{
    unsigned long                        ulPHYAddr;

    /* 申请编码目的空间 */
    g_astSCMCnfCoderDstCfg.pucBuf = (u8*)scm_UnCacheMemAlloc(g_astSCMCnfCoderDstCfg.ulBufLen, &ulPHYAddr);

    /* 申请空间错误 */
    if (NULL == g_astSCMCnfCoderDstCfg.pucBuf)
    {
        /* 记录通道初始化标记为内存申请异常 */
        g_astSCMCnfCoderDstCfg.enInitState = SCM_CHANNEL_MEM_FAIL;

        return ERR_MSP_FAILURE;/* 返回错误 */
    }

    g_astSCMCnfCoderDstCfg.pucBufPHY = (u8*)ulPHYAddr;

    return BSP_OK;
}


u32 scm_cnf_dst_buff_init(void)
{
    unsigned long                        ulPHYAddr;
    u32                          ulBufLen;

    /* CNF通道不需要做延迟写入 */
    ulBufLen = g_astSCMCnfCoderDstCfg.ulBufLen;

    g_astSCMCnfCoderDstCfg.pucBuf = (u8*)scm_UnCacheMemAlloc(ulBufLen, &ulPHYAddr);

    /* 申请空间错误 */
    if (NULL == g_astSCMCnfCoderDstCfg.pucBuf)
    {
        /* 记录通道初始化标记为内存申请异常 */
        g_astSCMCnfCoderDstCfg.enInitState = SCM_CHANNEL_MEM_FAIL;

        return ERR_MSP_FAILURE;
    }

    g_astSCMCnfCoderDstCfg.pucBufPHY = (u8*)ulPHYAddr;

    return BSP_OK;
}



u32 scm_rls_cnf_dst_buff(u32 ulReadSize)
{
    u32                          ulDataLen;
    SOCP_BUFFER_RW_STRU                 stBuffer;
    SOCP_CODER_DST_ENUM_U32             ulChanlID;

    ulChanlID = g_astSCMCnfCoderDstCfg.enChannelID;

    if(0 == ulReadSize) /*释放通道所有数据*/
    {
        if (BSP_OK != bsp_socp_get_read_buff(ulChanlID, &stBuffer))
        {
            diag_error("Get Read Buffer is Error\n");
            return ERR_MSP_FAILURE;
        }

        ulDataLen = stBuffer.u32Size + stBuffer.u32RbSize;
    }
    else
    {
        ulDataLen = ulReadSize;
    }

    if (BSP_OK != bsp_socp_read_data_done(ulChanlID, ulDataLen))
    {
        diag_error("Read Data Done is Error\n");

        return ERR_MSP_FAILURE;
    }
    OM_ACPU_DEBUG_CHANNEL_TRACE(ulChanlID, NULL, ulDataLen, OM_ACPU_READ_DONE, OM_ACPU_DATA);
    return BSP_OK;
}


u32 scm_cnf_dst_channel_init(void)
{
    SOCP_CODER_DEST_CHAN_S              stChannel;

    /* 扩大编码目的通道1阈值门限 */
    stChannel.u32EncDstThrh = 2 * SCM_CODER_DST_GTHRESHOLD;

    stChannel.sCoderSetDstBuf.pucOutputStart    = g_astSCMCnfCoderDstCfg.pucBufPHY;

    stChannel.sCoderSetDstBuf.pucOutputEnd
        = (g_astSCMCnfCoderDstCfg.pucBufPHY + g_astSCMCnfCoderDstCfg.ulBufLen)-1;

    stChannel.sCoderSetDstBuf.u32Threshold      = g_astSCMCnfCoderDstCfg.ulThreshold;

    stChannel.u32EncDstTimeoutMode = g_astSCMCnfCoderDstCfg.ulTimeoutMode;

    if (BSP_OK != mdrv_socp_coder_set_dest_chan_attr(g_astSCMCnfCoderDstCfg.enChannelID, &stChannel))
    {
        g_astSCMCnfCoderDstCfg.enInitState = SCM_CHANNEL_CFG_FAIL;  /* 记录通道初始化配置错误 */

        return ERR_MSP_FAILURE;/* 返回失败 */
    }

    bsp_socp_encdst_dsm_init(g_astSCMCnfCoderDstCfg.enChannelID, SOCP_DEST_DSM_ENABLE);

    g_astSCMCnfCoderDstCfg.enInitState = SCM_CHANNEL_INIT_SUCC;     /* 记录通道初始化配置错误 */

    (void)bsp_socp_register_read_cb(g_astSCMCnfCoderDstCfg.enChannelID, (socp_read_cb)scm_cnf_dst_read_cb);

    return BSP_OK;
}
void scm_reg_cnf_coder_dst_send_fuc(void)
{
    diag_crit("SCM_RegCoderDestCnfChan.\n");

    g_astSCMCnfCoderDstCfg.pfunc = (SCM_CODERDESTFUCN)scm_send_cnf_data_to_udi;
}


void scm_send_cnf_data_to_udi(u8 *pucVirData, u8 *pucPHYData, u32 ulDataLen)
{
    u32                  ulResult;
    u32                  ulRet = ERR_MSP_FAILURE;
    CPM_PHY_PORT_ENUM_UINT32    enPhyport;
    u32                  ulSendDataLen;
    bool                    bUsbSendSucFlag = false;
    bool                    bUsbSendFlag = false;
    OM_SOCP_CHANNEL_DEBUG_INFO  *pstDebugInfo = NULL;
    CPM_LOGIC_PORT_ENUM_UINT32  enLogicPort;
    SOCP_CODER_DST_ENUM_U32     enChanID;

    enChanID = g_astSCMCnfCoderDstCfg.enChannelID;
    pstDebugInfo = &g_stAcpuDebugInfo.stCnfDebugInfo;
    enLogicPort  = CPM_OM_CFG_COMM;

    /*参数检查*/
    if ((0 == ulDataLen) ||(NULL == pucVirData))
    {
        pstDebugInfo->ulUSBSendCBAbnormalNum++;
        return;
    }


    PPM_GetSendDataLen(enChanID, ulDataLen, &ulSendDataLen, &enPhyport);

    OM_ACPU_DEBUG_CHANNEL_TRACE(enChanID, pucVirData, ulSendDataLen, OM_ACPU_SEND_USB, OM_ACPU_DATA);

    ulResult = CPM_ComSend(enLogicPort, pucVirData, pucPHYData, ulSendDataLen);

    if(CPM_SEND_ERR == ulResult)  /*当前通道已经发送失败，调用SOCP通道无数据搬运*/
    {
        pstDebugInfo->ulUSBSendErrNum++;
        pstDebugInfo->ulUSBSendErrLen += ulSendDataLen;
    }
    else if(CPM_SEND_FUNC_NULL == ulResult)   /*当前通道异常，扔掉所有数据*/
    {
        pstDebugInfo->ulOmDiscardNum++;
        pstDebugInfo->ulOmDiscardLen += ulDataLen;
    }
    else if(CPM_SEND_PARA_ERR == ulResult)   /* 发送数据获取实地址异常 */
    {
        pstDebugInfo->ulOmGetVirtErr++;
        pstDebugInfo->ulOmGetVirtSendLen += ulDataLen;
    }
    else if(CPM_SEND_AYNC == ulResult) //增加cpm错误码
    {
        bUsbSendSucFlag = true;
        bUsbSendFlag    = true;
        ulRet           = BSP_OK;
    }
    else if(CPM_SEND_OK == ulResult)
    {
        SCM_SocpSendDataToUDISucc(enChanID, enPhyport, pstDebugInfo, &ulSendDataLen);

        bUsbSendSucFlag = true;
    }
    else
    {
        diag_error("CPM_ComSend return Error(0x%x)\n", (s32)ulResult);
    }

    if(bUsbSendFlag != true)
    {
        ulRet = scm_rls_cnf_dst_buff(ulSendDataLen);

        if(BSP_OK != ulRet)
        {
            pstDebugInfo->ulSocpReadDoneErrNum++;
            pstDebugInfo->ulSocpReadDoneErrLen += ulSendDataLen;

            diag_error("SCM_RlsDestBuf return Error(0x%x)\n", (s32)ulRet);
        }
    }

    if ((BSP_OK == ulRet) && (true == bUsbSendSucFlag))
    {
        pstDebugInfo->ulUSBSendNum++;
        pstDebugInfo->ulUSBSendRealLen += ulSendDataLen;
    }

    return;
}


void scm_cnf_dst_read_cb(void)
{
    u32                          ulChType;
    SOCP_BUFFER_RW_STRU          stBuffer;
    u32                          ulTimerIn;
    u32                          ulTimerOut;
    unsigned long                ulVirtAddr;
    u32                          ulDstChID;

    ulDstChID = g_astSCMCnfCoderDstCfg.enChannelID;

    ulChType = SOCP_REAL_CHAN_TYPE(ulDstChID);

    if (SOCP_CODER_DEST_CHAN != ulChType)
    {
        diag_error("Channel Type(0x%x) is Error\n",ulChType);
        return;
    }

    if (BSP_OK != bsp_socp_get_read_buff(ulDstChID, &stBuffer))
    {
        diag_error("Get Read Buffer is Error\n");
        return;
    }

     /* 开机log功能，IND通道上报函数为空，使log缓存在本地 */
    if(NULL == g_astSCMCnfCoderDstCfg.pfunc)
    {
        diag_error("cnf dst channel is null\n");
        return;
    }

    if((0 == (stBuffer.u32Size + stBuffer.u32RbSize))||(NULL == stBuffer.pBuffer))
    {
        bsp_socp_read_data_done(ulDstChID, stBuffer.u32Size + stBuffer.u32RbSize);  /* 清空数据 */
        diag_error("Get RD error\n");/* 记录Log */
        return;
    }

    if(0 == stBuffer.u32Size)
    {
        return;
    }

    /* 发送数据 */
    ulVirtAddr = scm_UncacheMemPhyToVirt((u8 *)stBuffer.pBuffer,
                                g_astSCMCnfCoderDstCfg.pucBufPHY,
                                g_astSCMCnfCoderDstCfg.pucBuf,
                                g_astSCMCnfCoderDstCfg.ulBufLen);
    if((unsigned long)NULL == ulVirtAddr)
    {
        bsp_socp_read_data_done(ulDstChID, stBuffer.u32Size + stBuffer.u32RbSize);  /* 清空数据 */
        diag_error("stBuffer.pBuffer==NULL\n");
        return;
    }
    ulTimerIn = bsp_get_slice_value();
    g_astSCMCnfCoderDstCfg.pfunc((u8*)ulVirtAddr, (u8*)stBuffer.pBuffer,(u32)stBuffer.u32Size);
    ulTimerOut = bsp_get_slice_value();
    /* 记录回调函数的执行时间 */
    if(g_DiagLogLevel)
    {
        diag_crit("g_astSCMCnfCoderDstCfg.pfunc Proc time 0x%x\n", (ulTimerOut - ulTimerIn));        
    }    

    return;
}






