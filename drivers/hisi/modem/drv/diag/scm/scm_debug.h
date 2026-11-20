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

#ifndef _SCM_DEBUG_H_
#define _SCM_DEBUG_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include "OmCommonPpm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/**************************************************************************
  5 Log函数定义
**************************************************************************/

#define scm_printf(fmt, ...) \
    bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_DIAG_SYSTEM,"[scm]:<%s %d> "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)

#define SCM_CODER_SRC_LOG(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d\n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_CODER_SRCRD_LOG(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d\n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_CODER_SRCRESET_LOG(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d \n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_CODER_SRC_ERR(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d \n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_CODER_DST_ERR(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d\n", __FUNCTION__,  pucStr, ulP1, ulP2)

#define SCM_DECODER_SRC_LOG(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d\n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_CODER_DST_LOG(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d\n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_DECODER_DST_LOG(pucStr, ulP1, ulP2)\
    bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_DIAG_SYSTEM,"[%s]:%s, %d, %d\n", __FUNCTION__, pucStr, ulP1, ulP2)

#define SCM_CODER_SRC_NUM           (2)       /* 编码源通道个数 */
#define SCM_CODER_DST_NUM           (2)       /* 编码目的通道个数 */
#define SCM_DECODER_SRC_NUM         (1)       /* 解码源通道个数 */
#define SCM_DECODER_DST_NUM         (1)       /* 解码目的通道个数 */


typedef struct{
    u32      ulFillFirstBuffNum;
    u32      ulFillFirstBuffMax;
    u32      ulFillNotEnough;
    u32      ulAfterFillNotEnough;

    u32      ulGetCoderBuffErr;
    u32      ulGetCoherentBuffErr;
    u32      ulGetWriteBufErr;

    u32      ulSendPacketNum;
    u32      ulSendDataLen;
    u32      ulSendFirstNotEnough;
    u32      ulSendAddrErr;
    u32      ulSendWriteDoneErr;
    u32      ulSendHeaderErr;

}SCM_CODER_SRC_DEBUG_STRU;

typedef struct
{
    INNER_LOG_DATA_STRU         stCoderSrcLog;                      /* 记录上层调用的发送接口log */
    INNER_LOG_DATA_STRU         stCoderSrcRDLog;                    /* 记录上层调用的释放RD接口log */
    INNER_LOG_DATA_STRU         stCoderSrcErr;                      /* 记录编码源通道回调错误Log */
    INNER_LOG_DATA_STRU         stCoderDstLog;                      /* 记录回调上层的编码目的通道函数Log */
    INNER_LOG_DATA_STRU         stCoderDstErr;                      /* 记录编码目的通道回调错误Log */
    u32                  ulCoderDstIndLen;                   /* 记录IND通道编码目的端送出的总字节数 */
}SCM_INFODATA_STRU;


void SCM_ChannelInfoShow(void);
void SCM_CoderSrcCHShow(u32 ulCfgNum);
void SCM_CoderDstCHShow(u32 ulCfgNum);
SCM_INFODATA_STRU* SCM_GetDebugLogInfo(void);
u32 SCM_GetDebugLogInfoLen(void);
void SCM_SocpSendDataToUDISucc(SOCP_CODER_DST_ENUM_U32 enChanID,CPM_PHY_PORT_ENUM_UINT32 enPhyport,OM_SOCP_CHANNEL_DEBUG_INFO *pstDebugInfo,u32 *pulSendDataLen);
void SCM_SoftDecodeInfoShow(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


