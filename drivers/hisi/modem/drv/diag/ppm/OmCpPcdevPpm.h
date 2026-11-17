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


#ifndef __OM_CP_PCDEV_PPM_H__
#define __OM_CP_PCDEV_PPM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <mdrv.h>
#include <mdrv_pcdev.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define PPM_PCDEV_SOCP_CNF_BUFFER_SIZE          (8*1024)
#define PPM_PCDEV_SOCP_CNF_BUFFER_NUM           (8)

#define PPM_PCDEV_SOCP_IND_BUFFER_SIZE          (2*1024)
#define PPM_PCDEV_SOCP_IND_BUFFER_NUM           (2)
/*******************************************************************************
  3 枚举定义
*******************************************************************************/
typedef enum
{
    PPM_PCDEV_TYPE_IND,
    PPM_PCDEV_TYPE_CFG,
    PPM_PCDEV_TYPE_BUTT,
}PPM_PCDEV_TYPE_E;

/*****************************************************************************
  4 结构体定义
*****************************************************************************/
typedef struct PPM_PCDEV_MNTN_S
{
    u32 ulWriteNum1;
    u32 ulWriteNum2;
    u32 ulWriteMaxTime;
    u32 ulWriteErrNum;
    u32 ulWriteErrLen;
    u32 ulWriteErrRet;
    u32 ulWriteErrTime;
    u32 ulWriteInvalidChannNum;
    u32 ulSendLen;

    u32 ulWriteCbNum;

    u32 ulOpenNum;
    u32 ulOpenSlice;
    u32 ulOpenOkNum;
    u32 ulOpenOkSlice;
    u32 ulOpenOkEndNum;
    u32 ulOpenOkEndSlice;

    u32 ulOutNum;
    u32 ulOutSlice;

    u32 ulPcdevInNum;
    u32 ulPcdevInSlice;

    u32 ulStateErrNum;
    u32 ulStateErrSlice;

    u32 ulReadCbNum;            /* 收到AP发送数据的次数 */

    u32 ulHandleErrNum;
    u32 ulHandleGetRdBuffErrNum;

    u32 ulPcdevRcvPktNum;
    u32 ulPcdevRcvPktByte;
    u32 ulPcdevRcvErrNum;
    u32 ulPcdevRdBuffFreeErrNum;
}PPM_PCDEV_MNTN_T;


typedef struct PPM_PCEV_CTRL_S
{
    u32                 ready_state;
    PPM_PCDEV_MNTN_T    pcdev_mntn[PPM_PCDEV_TYPE_BUTT];
    UDI_HANDLE          port_udi_handle[PPM_PCDEV_TYPE_BUTT];
}PPM_PCDEV_CTRL_T;

/*****************************************************************************
  4 函数声明
*****************************************************************************/
u32 PPM_PcdevCfgSendData(u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen);
void PPM_PcdevCfgStatusCB(void * context);
void PPM_PcdevCfgWriteDataCB(u8* pucVirData, u8* pucPhyData, s32 lLen);
s32 PPM_PcdevCfgReadDataCB(void);
void PPM_PcdevCfgPortOpen(void);
void PPM_PcdevIndStatusCB(void *context);
void PPM_PcdevIndWriteDataCB(u8* pucVirData, u8* pucPhyData, s32 lLen);
void PPM_PcdevIndPortOpen(void);
u32 PPM_PcdevIndSendData(u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen);
u32 PPM_PcdevPortInit(void);
u32 PPM_PcdevPortSend(PPM_PCDEV_TYPE_E enHandle, u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen);
u32 PPM_PcdevShowDbgInfo(PPM_PCDEV_TYPE_E type);
void PPM_PcdevReady(void);
void PPM_PcdevEventRegister(void);
u32 PPM_PcdevSendIccMsg(u32 ulMsgId, u32 ulLen, void *context);

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 OTHERS定义
*****************************************************************************/




#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of OmCommonPpm.h*/

