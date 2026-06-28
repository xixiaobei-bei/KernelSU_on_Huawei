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

#ifndef __MDRV_DIAG_SYSTEM_H__
#define __MDRV_DIAG_SYSTEM_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include "mdrv_socp_common.h"
#include "mdrv_diag_system_common.h"

#ifdef DIAG_SYSTEM_5G
#define SCM_CODER_SRC_LOM_CNF                       (SOCP_CODER_SRC_ACPU_CNF)
#define SCM_CODER_SRC_LOM_IND                       (SOCP_CODER_SRC_ACPU_IND)
#else
#define SCM_CODER_SRC_LOM_CNF                       (SOCP_CODER_SRC_LOM_CNF1)
#define SCM_CODER_SRC_LOM_IND                       (SOCP_CODER_SRC_LOM_IND1)
#endif

#define DIAG_CODER_SRC_CNF_PADDR       (DIAG_CODER_SRC_ACORE_CNF_PADDR)
#define DIAG_CODER_SRC_CNF_LENGTH      (DIAG_CODER_SRC_ACORE_CNF_LENGTH)

#define DIAG_CODER_SRC_IND_PADDR       (DIAG_CODER_SRC_ACORE_IND_PADDR)
#define DIAG_CODER_SRC_IND_LENGTH      (DIAG_CODER_SRC_ACORE_IND_LENGTH)


enum
{
    CPM_AT_COMM,
    CPM_OM_IND_COMM,        /* OM可维可测端口 */
    CPM_OM_CFG_COMM,        /* OM配置端口 */
    CPM_DIAG_COMM,
    CPM_PCVOICE_COMM,
    CPM_COMM_BUTT
};
typedef unsigned int  CPM_LOGIC_PORT_ENUM_UINT32;


typedef struct
{
    unsigned int                      ulUSBWriteNum1;
    unsigned int                      ulUSBWriteNum2;
    unsigned int                      ulUSBWriteCBNum;
    unsigned int                      ulUSBWriteMaxTime;
    unsigned int                      ulUSBINNum;
    unsigned int                      ulUSBINTime;
    unsigned int                      ulUSBOutNum;
    unsigned int                      ulUSBOutTime;
    unsigned int                      ulUSBStateErrNum;
    unsigned int                      ulUSBStateErrTime;
    unsigned int                      ulUSBWriteErrTime;
    unsigned int                      ulUSBWriteErrNum;
    unsigned int                      ulUSBWriteErrValue;
    unsigned int                      ulUSBWriteErrLen;

    unsigned int                      ulUSBOpenNum;
    unsigned int                      ulUSBOpenSlice;
    unsigned int                      ulUSBOpenOkNum;
    unsigned int                      ulUSBOpenOkSlice;
    unsigned int                      ulUSBOpenOk2Num;
    unsigned int                      ulUSBOpenOk2Slice;
    unsigned int                      ulUSBCloseNum;
    unsigned int                      ulUSBCloseSlice;
    unsigned int                      ulUSBCloseOkNum;
    unsigned int                      ulUSBCloseOkSlice;

    unsigned int                      ulUSBIndPseudoSyncFailNum;
    unsigned int                      ulUSBIndPseudoSyncFailSlice;
    unsigned int                      ulUSBCnfPseudoSyncFailNum;
    unsigned int                      ulUSBCnfPseudoSyncFailSlice;
    unsigned int                      ulUSBUdiHandleErr;
    unsigned int                      ulUSBUdiHandleReadGetBufferErr;
    unsigned int                      ulUSBUdiCommRcvNullPtrErr;
    unsigned int                      ulUSBUdiHandleReadBufferFreeErr;
    unsigned int                      ulUSBRcvPktNum;
    unsigned int                      ulUSBRcvPktByte;

}OM_ACPU_PORT_INFO;

typedef struct
{
    unsigned int                          ulSocpCBParaErrNum;
    unsigned int                          ulUSBSendNum;
    unsigned int                          ulUSBSendLen;
    unsigned int                          ulUSBSendRealLen;
    unsigned int                          ulUSBSendErrNum;
    unsigned int                          ulUSBSendErrLen;
    unsigned int                          ulUSBSendCBAbnormalNum;
    unsigned int                          ulUSBSendCBAbnormalLen;

    unsigned int                          ulOmGetVirtErr;
    unsigned int                          ulOmGetVirtSendLen;
    unsigned int                          ulOmDiscardNum;
    unsigned int                          ulOmDiscardLen;
    unsigned int                          ulSocpReadDoneErrNum;
    unsigned int                          ulSocpReadDoneErrLen;
}OM_SOCP_CHANNEL_DEBUG_INFO;

#define OM_PORT_HANDLE_NUM      (5) /*OM_PORT_HANDLE_NUM = OM_PORT_HANDLE_BUTT*/
/*用于记录当前OM通道的信息*/
typedef struct
{
    OM_SOCP_CHANNEL_DEBUG_INFO          stIndDebugInfo;
    OM_SOCP_CHANNEL_DEBUG_INFO          stCnfDebugInfo;
    OM_ACPU_PORT_INFO                   astPortInfo[OM_PORT_HANDLE_NUM];
    unsigned int                          ulInvaldChannel;
}OM_ACPU_DEBUG_INFO;
/* 用于记录VCOM发送信息 */
typedef struct
{
    unsigned int                          ulVCOMSendSn;
    unsigned int                          ulVCOMSendNum;
    unsigned int                          ulVCOMSendLen;
    unsigned int                          ulVCOMSendErrNum;
    unsigned int                          ulVCOMSendErrLen;
    unsigned int                          ulMaxTimeLen;           /* 调用写接口花费的最大时长 */

    unsigned int                          ulVCOMRcvSn;
    unsigned int                          ulVCOMRcvNum;
    unsigned int                          ulVCOMRcvLen;
    unsigned int                          ulVCOMRcvErrNum;
    unsigned int                          ulVCOMRcvErrLen;
}OM_VCOM_DEBUG_INFO;

/* mntn info ***********************************************************************/
typedef enum
{
    EN_DIAG_DST_LOST_BRANCH,            /* 当前时间段分支失败次数 */

    EN_DIAG_DST_LOST_CPMWR,             /* 当前时间段CPM写失败次数 */

    EN_DIAG_DST_LOST_CPMCB,             /* 当前时间段CPM写回调失败次数 */
    
    EN_DIAG_DST_LOST_MAX
}DIAG_LOST_DST_ID_ENUM;

typedef struct
{
    unsigned int      ulChannelId;        /* 通道号 */
    char     chanName[16];       /*通道名*/
    unsigned int      ulLeftSize;         /*通道剩余buf大小*/
    unsigned int      ulDeltaTime;        /*和上次上报的时间间隔*/

    unsigned int      ulDeltaLostTimes;   /*上报时间段内的丢次数*/        
    unsigned int      ulDeltaLostLen;     /*上报时间段内丢弃数据长度*/        

    unsigned int      aulCurFailNum[EN_DIAG_DST_LOST_MAX];    /* 当前时间段内丢包的各类次数 */
    unsigned int      aulCurFailLen[EN_DIAG_DST_LOST_MAX];    /* 当前时间段内丢包的总数据长度 */

    unsigned int      ulDeltaOverFlowCnt;         /* 上报时间段内通道上溢次数 */
    unsigned int      ulDeltaPartOverFlowCnt;     /* 上报时间段内通道80%上溢次数 */

    unsigned int      ulDeltaSocpLen;             /* 上报时间间隔内从socp获取到的数据长度 */

    unsigned int      ulDeltaUSBLen;              /* 上报时间间隔内发送给USB的总字节数 */
    unsigned int      ulDeltaUSBOKLen;            /* 上报时间间隔内发送给USB返回成功的总字节数 */
    unsigned int      ulDeltaUSBFreeLen;          /* 上报时间间隔内USB回调释放的总字节数 */
    unsigned int      ulDeltaUSBFailLen;          /* 上报时间间写入USB失败的总字节数 */

    unsigned int      ulDeltaVcomLen;             /* 上报时间间隔内发送给Vcom的总字节数 */
    unsigned int      ulDeltaVcomFailLen;         /* 上报时间间隔内Vcom写入失败的总字节数 */
    unsigned int      ulDeltaVcomSucessLen;       /* 上报时间间隔内Vcom写入成功的总字节数 */

    unsigned int      ulDeltaSocketLen;           /* 上报时间间隔内发送给Socket的总字节数 */
    unsigned int      ulDeltaSocketSucessLen;     /* 上报时间间隔内Socket写入成功的总字节数 */
    unsigned int      ulDeltaSocketFailLen;       /* 上报时间间隔内Socket写入成功的总字节数 */

    unsigned int      ulDeltaSocpIntToPortTime;     /* 上报时间段内从socp上报中断开始到调用各端口发送的时间总和 */

    unsigned int      ulDeltaUsbSendTime;         /* 上报时间段内发送给USB占用的时间总和 */
    unsigned int      ulDeltaVcomSendTime;        /* 上报时间段内发送给Vcom占用的时间总和 */
    unsigned int      ulDeltaSockSendTime;        /* 上报时间段内发送给Socket占用的时间总和 */
    
    unsigned int      ulThrputPhy;               /* 物理通道吞吐率 */
    unsigned int      ulThrputCb;                /* 回调吞吐率 */

    unsigned int      aulReserve[12];              /* 预留 */

    unsigned int      aulToolreserve[12];         /* 给工具预留的64个字节，用于在工具上显示工具的维测信息 */
}DIAG_MNTN_DST_INFO_STRU;



typedef unsigned int (*PPM_DisconnectTLPortFuc)(void);
typedef unsigned int (*CPM_RCV_FUNC)(unsigned char *pucData, unsigned int ulLen);
/* 通道接收数据、数据长度、回卷数据、回卷数据长度 */
typedef void (*SCM_DECODERDESTFUCN)(SOCP_DECODER_DST_ENUM_U32 enChanID,unsigned char *pucData, unsigned int ulSize, unsigned char *pucRBData, unsigned int ulRBSize);

void mdrv_PPM_RegDisconnectCb(PPM_DisconnectTLPortFuc cb);
void mdrv_scm_reg_ind_coder_dst_send_fuc(void);
unsigned int mdrv_CPM_ComSend(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, unsigned char *pucVirData, unsigned char *pucPHYData, unsigned int ulLen);
void mdrv_CPM_LogicRcvReg(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, CPM_RCV_FUNC pRcvFunc);
unsigned int mdrv_PPM_LogPortSwitch(unsigned int  ulPhyPort, unsigned int ulEffect);
unsigned int mdrv_PPM_QueryLogPort(unsigned int  *pulLogPort);
void mdrv_scm_ind_dst_read_cb(void);
unsigned int mdrv_SCM_RegDecoderDestProc(SOCP_DECODER_DST_ENUM_U32 enChanlID, SCM_DECODERDESTFUCN func);
void mdrv_report_dst_mntn(void);
void mdrv_diag_get_dst_mntn_info(DIAG_MNTN_DST_INFO_STRU * dst_mntn);
void mdrv_diag_reset_dst_mntn_info(void);
void mdrv_scm_set_power_on_log(void);

#ifdef __cplusplus
}
#endif
#endif
