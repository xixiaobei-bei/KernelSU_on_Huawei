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


#include "hdlc_software.h"
#include "TTFComm.h"
#include "gucttf_tag.h"
#include "ppp_convert.h"
#include "hdlc_interface.h"
#include "ppp_input.h"
#include "ppp_public.h"


#define THIS_FILE_ID            PS_FILE_ID_HDLC_SOFTWARE_C
#define THIS_MODU               mod_ppp

typedef struct {
    VOS_UINT32                  ulUplinkIpDataProcCnt;             /* 上行IP数据包处理个数 */
    VOS_UINT32                  ulUplinkPppDataProcCnt;            /* 上行简单PPP帧处理个数 */

    VOS_UINT32                  ulDownlinkIpDataProcCnt;           /* 下行IP数据包处理个数 */
    VOS_UINT32                  ulDownlinkPppDataProcCnt;          /* 下行简单PPP帧处理个数 */

    VOS_UINT32                  ulMaxCntOnce;                      /* PPP一次最多处理的结点个数 */
} PPP_HDLC_SOFT_DATA_PROC_STAT_ST;

PPP_HDLC_SOFT_DATA_PROC_STAT_ST g_PppHdlcSoftStat = {0};
VOS_BOOL                        g_PppIsIpMode = VOS_TRUE;


VOS_UINT32 PPP_HDLC_Encap(PPP_ID pppId, PPP_ZC_STRU *pstData, VOS_BOOL isPppMode)
{
    VOS_UINT16      proto;
    VOS_UINT8      *head = PPP_ZC_GET_DATA_PTR(pstData);
    VOS_UINT8       headLen = 1;  /* 协议长度最小为1 */
    PppEncInput     input;
    PppEncOutput    output = {0};
    VOS_UINT32      loop;
    PPP_ZC_STRU    *data = VOS_NULL_PTR;

    if (isPppMode == VOS_TRUE) {
        if (PPP_ZC_GET_DATA_LEN(pstData) < sizeof(proto)) {
            PPP_MNTN_LOG(LOG_LEVEL_WARNING, "wrong data len!");
            return VOS_ERR;
        }
        /* 协议规定协议的bit0必须为1, bit8必须为0, 如果第一个字节的bit0不是1说明没有进行协议域压缩*/
        proto = head[0];
        if ((proto & 0x01) != 0x01) {
            proto = (proto << 8 | head[headLen]);
            headLen++;
        }
        PPP_ZC_REMOVE_HDR(pstData, headLen);
    } else {
        proto = PPPA_IP;
    }

    input.data      = PPP_ZC_GET_DATA_PTR(pstData);
    input.dataLen   = PPP_ZC_GET_DATA_LEN(pstData);
    input.proto     = proto;
    if (PPP_CONV_Encode(pppId, &input, &output) != VOS_OK) {
        PPP_MNTN_LOG(LOG_LEVEL_WARNING, "Encap PPP Fail!");
        return VOS_ERR;
    }

    for (loop = 0; loop < output.segCnt; loop++) {
        data = (PPP_ZC_STRU*)output.outSegs[loop].seg.raw;
        PPP_ZC_SET_DATA_LEN(data, output.outSegs[loop].used);
        PPPA_SendDlDataToAT(pppId, data);
    }

    return VOS_OK;
}

VOS_BOOL PPP_HDLC_SOFT_ProcData(PPP_ID usPppId, PPP_ZC_QUEUE_STRU *pstDataQ)
{
    PPP_ZC_STRU                        *pstMem = VOS_NULL_PTR;
    VOS_UINT32                          ulDealCnt = 0;
    PPP_DATA_TYPE_ENUM_UINT8            ucDataType;
    VOS_BOOL                            delayProc = VOS_TRUE;
    VOS_UINT32                          procRslt = VOS_OK;

    /* 一次最多处理 PPP_ONCE_DEAL_MAX_CNT 个节点 */
    for (ulDealCnt = 0; ulDealCnt < PPP_ONCE_DEAL_MAX_CNT; ulDealCnt++) {
        pstMem  = (PPP_ZC_STRU *)PPP_ZC_DEQUEUE_HEAD(pstDataQ);

        /* 队列为空不需要继续处理 */
        if ( pstMem == VOS_NULL_PTR) {
            delayProc = VOS_FALSE;
            break;
        }

        /*处理该结点(结点的释放动作已经在各处理函数内部完成，无需再释放结点)*/
        ucDataType = (PPP_ZC_GET_DATA_APP(pstMem) & 0x00FF);

        switch ( ucDataType )
        {
            case PPP_PULL_PACKET_TYPE:
                g_PppHdlcSoftStat.ulUplinkIpDataProcCnt++;
                procRslt = PPP_CONV_Decode(usPppId, PPP_ZC_GET_DATA_PTR(pstMem), PPP_ZC_GET_DATA_LEN(pstMem));

                break;
            case PPP_PUSH_PACKET_TYPE:
                g_PppHdlcSoftStat.ulDownlinkIpDataProcCnt++;
                procRslt = PPP_HDLC_Encap(usPppId, pstMem, VOS_FALSE);
                break;

            case PPP_PULL_RAW_DATA_TYPE:
                g_PppHdlcSoftStat.ulUplinkPppDataProcCnt++;
                procRslt = PPP_CONV_Decode(usPppId, PPP_ZC_GET_DATA_PTR(pstMem), PPP_ZC_GET_DATA_LEN(pstMem));
                break;
                
            case PPP_PUSH_RAW_DATA_TYPE:
                g_PppHdlcSoftStat.ulDownlinkPppDataProcCnt++;
                procRslt = PPP_HDLC_Encap(usPppId, pstMem, VOS_TRUE);
                break;

            default:
                PPP_MNTN_LOG1(LOG_LEVEL_WARNING,
                              "PPP_ProcDataNotify, WARNING, ucDataType %d is Abnormal!", ucDataType);
                break;
        }
        PPP_MemFree(pstMem);
        if (procRslt != VOS_OK) {
            break;
        }
    }

    if ( ulDealCnt > g_PppHdlcSoftStat.ulMaxCntOnce )
    {
        g_PppHdlcSoftStat.ulMaxCntOnce = ulDealCnt;
    }
    /* 一次处理节点数达到 PPP_ONCE_DEAL_MAX_CNT 需要延迟处理队列 */
    return delayProc;
}

VOS_UINT32 PPPA_DefMemAllocCB(VOS_UINT32 len, PppUplayerMem *buf)
{
    PPP_ZC_STRU *mem = PPP_MemAlloc(len, PPP_ZC_UL_RESERVE_LEN);
    if (mem == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    buf->data = PPP_ZC_GET_DATA_PTR(mem);
    buf->raw = mem;
    buf->size = len;
    return VOS_OK;
}

VOS_UINT32 PPPA_FrmMemAllocCB(VOS_UINT32 len, PppUplayerMem *buf)
{
    PPP_ZC_STRU *mem = PPP_MemAlloc(len, PPP_ZC_DL_RESERVE_LEN);
    if (mem == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    buf->data = PPP_ZC_GET_DATA_PTR(mem);
    buf->raw = mem;
    buf->size = len;
    return VOS_OK;
}

VOS_VOID PPPA_DefMemFreeCB(PppUplayerMem *buf)
{
    PPP_MemFree((PPP_ZC_STRU*)buf->raw);
}

VOS_VOID PPPA_DefRsltProcCB(PppDecFrame *frm)
{
    PPP_ZC_STRU     *mem = (PPP_ZC_STRU*)frm->buf.raw;

    PPP_ZC_SET_DATA_LEN(mem, frm->used);
    
    if (g_PppIsIpMode == VOS_TRUE) {
        PPP_ZC_REMOVE_HDR(mem, frm->hdrLen);
        PPP_HDLC_ProcIpModeUlData(frm->pppId, mem, frm->proto);
    } else {
        PPP_ZC_REMOVE_HDR(mem, frm->hdrAcLen);
        /* PPP模式下发送的是PPP报文，因此协议类型填0 */
        PPP_SendUlDataToAds(frm->pppId, mem, 0);
    }
}

VOS_VOID PPP_HDLC_SetUp(PPP_ID pppId, VOS_BOOL isIpMode)
{
    PppConvOps      convOps;

    convOps.decCfg.alloc        = PPPA_DefMemAllocCB;
    convOps.decCfg.free         = PPPA_DefMemFreeCB;
    convOps.decCfg.deliver      = PPPA_DefRsltProcCB;
    convOps.decCfg.memMaxLen    = IMM_MAX_ETH_FRAME_LEN - PPP_ZC_UL_RESERVE_LEN;
    convOps.encCfg.alloc        = PPPA_FrmMemAllocCB;
    convOps.encCfg.free         = PPPA_DefMemFreeCB;
    convOps.encCfg.accMap       = PPPA_DEFAULT_ACCM;
    convOps.encCfg.acf          = VOS_FALSE;
    convOps.encCfg.pcf          = VOS_FALSE;
    convOps.encCfg.memMaxLen    = IMM_MAX_ETH_FRAME_LEN;
    PPP_CONV_Create(pppId, &convOps);

    g_PppIsIpMode = isIpMode;
}

VOS_VOID PPP_HDLC_UpdateCfg(PPP_ID pppId, VOS_BOOL acf, VOS_BOOL pcf, VOS_UINT32 accm)
{
    PppConvOps  ops;
    
    PPP_CONV_GetCfg(pppId, &ops);
    ops.encCfg.acf     = acf;
    ops.encCfg.pcf     = pcf;
    ops.encCfg.accMap  = accm;

    PPP_CONV_ReCfg(pppId, &ops);
}




