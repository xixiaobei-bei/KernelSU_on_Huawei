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

#include "ppp_public.h"
#include "securec.h"
#include "ttf_util.h"
#include "ps_trace_msg.h"



/*lint -e767  打点日志文件宏ID定义 */
#define THIS_FILE_ID PS_FILE_ID_PPP_PUBLIC_C
/*lint +e767   */

__STATIC_ PPP_DataQStat *g_dataStat = VOS_NULL_PTR;

#if (FEATURE_PPP == FEATURE_ON)

/*
 * 分配零拷贝内存
 * 该接口用在上行时需要保留MAC头长度，
 * 上行与ADS收发数为IP包，为与NDIS、E5保持数据结构统一，需要保留MAC头，
 * 零拷贝指针从C核返回的时候统一偏移固定字节，找到零拷贝头部。
 */
PPP_Zc* PPP_MemAlloc(VOS_UINT16 len, VOS_UINT16 reserveLen)
{
    /* 用在下行时保留长度填0，下行与USB收发数据为字节流形式的PPP帧，无MAC头 */
    PPP_Zc *mem = PPP_ZC_MEM_ALLOC(len + reserveLen);

    if (mem != VOS_NULL_PTR) {
        if (reserveLen > 0) {
            /* 空出保留长度，对PPP模块而言数据长度是usLen，这个函数必须在未赋值前调用 */
            PPP_ZC_RESERVE(mem, reserveLen);
        }

        g_dataStat->memAllocSuccCnt++;
    } else {
        g_dataStat->memAllocFailCnt++;
    }

    return mem;
}

/* 分配零拷贝内存块，并且填入已经填写完成的数据内容 */
PPP_Zc* PPP_MemClone(const VOS_UINT8 *src, VOS_UINT16 len, VOS_UINT16 reserveLen)
{
    PPP_Zc *mem = PPP_MemAlloc(len, reserveLen);

    if (mem != VOS_NULL_PTR) {
        /* 拷贝至内存数据部分 */
        PPPA_SF_CHK(memcpy_s(PPP_ZC_GET_DATA_PTR(mem), len, src, len));
        PPP_ZC_SET_DATA_LEN(mem, len);
    }

    return mem;
}

/* 从一TTF内存块获取一定长度的数据到指定目标地址 */
VOS_UINT32 PPP_MemGet(PPP_Zc *memSrc, VOS_UINT16 offset, VOS_UINT8 *dest, VOS_UINT16 len)
{
    VOS_UINT16 memSrcLen;

    /* 参数检查 */
    if ((memSrc == VOS_NULL_PTR) || (dest == VOS_NULL_PTR)) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "PPP_MemGet, Warning, Input Par pMemSrc Or pDest is Null!\r\n");

        return PS_FAIL;
    }

    if (len == 0) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "PPP_MemGet, Warning, Input Par usLen is 0!\r\n");

        return PS_FAIL;
    }

    /* 判断TTF内存块的长度是否符合要求 */
    memSrcLen = PPP_ZC_GET_DATA_LEN(memSrc);

    if (memSrcLen < (offset + len)) {
        PPP_MNTN_LOG2(PS_PRINT_WARNING, "PPP_MemGet, Warning, MemSrcLen <1> Less Than (Offset + Len) <1>!\r\n", memSrcLen,
            (offset + len));

        return PS_FAIL;
    }

    PPPA_SF_CHK(memcpy_s(dest, len, PPP_ZC_GET_DATA_PTR(memSrc) + offset, len));

    return PS_SUCC;
}

/* 释放零拷贝内存 */
VOS_VOID PPP_MemFree(PPP_Zc *mem)
{
    /* 释放零拷贝内存 */
    PPP_ZC_MEM_FREE(mem);

    g_dataStat->memTotalFreeCnt++;

    return;
}

VOS_VOID PPPA_SetDataStatAddr(PPP_DataQStat* dataStat)
{
    g_dataStat = dataStat;
}

VOS_VOID PPPA_SendPppsCommMsg(PPPS_PPPA_MsgTypeUint16 msgId)
{
    PPPS_PPPA_CommMsg *commMsg = VOS_NULL_PTR;

    commMsg = (PPPS_PPPA_CommMsg*)PS_ALLOC_MSG_WITH_HEADER_LEN(PS_PID_APP_PPP, sizeof(PPPS_PPPA_CommMsg));
    if (commMsg == VOS_NULL_PTR) {
        PPP_MNTN_LOG(PS_PRINT_WARNING, "alloc Msg Fail");
        return;
    }

    TTF_SET_MSG_RECEIVER_ID(commMsg, MSPS_PID_PPPS);
    commMsg->msgId = msgId;

    (VOS_VOID)PS_TRACE_AND_SND_MSG(PS_PID_APP_PPP, commMsg);
}

#else
VOS_VOID PPP_MemFree(PPP_Zc *mem)
{
    /* 释放零拷贝内存 */
    PPP_ZC_MEM_FREE(mem);

    return;
}

#endif

VOS_VOID PPPA_CheckSafeFuncRslt(VOS_BOOL result, VOS_UINT32 fileNo, VOS_UINT32 lineNo)
{
    if (result) {
        PPP_MNTN_LOG2(PS_PRINT_WARNING, "PPPA_CheckSafeFuncRslt Fail", fileNo, lineNo);
    }
}

