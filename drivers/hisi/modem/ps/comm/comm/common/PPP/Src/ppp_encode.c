/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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

#include "ppp_encode.h"
#include "ppp_fcs.h"



/*协议栈打印打点方式下的.C文件宏定义*/
#define THIS_FILE_ID        PS_FILE_ID_PPP_ENCODE_C

typedef struct {
    PppEncOutput           *output;
    VOS_UINT32              accMap;
    VOS_UINT16              fcs;
    VOS_UINT16              maxSegCnt;
    PppEncOutputSeg        *writeSeg;
} PppEncBufMgm;

/* 写入一个普通字符，不进行转义处理和FCS计算 */
/*lint -e{730}*/
static inline VOS_VOID PPP_ENC_WriteNonEscC2Buf(VOS_UINT8 data, PppEncBufMgm *buf)
{
    if (buf->writeSeg->used < buf->writeSeg->seg.size) {
        *(buf->writeSeg->seg.data + buf->writeSeg->used++) = data;
    } else {
        buf->writeSeg = &(buf->output->outSegs[buf->output->segCnt % buf->maxSegCnt]);
        buf->writeSeg->used = 0;
        buf->output->segCnt++;
        *(buf->writeSeg->seg.data + buf->writeSeg->used++) = data;
    }
}

/* 写入一个字符，对需要转义的字符进行转义处理 */
/*lint -e{730}*/
static inline VOS_VOID PPP_ENC_WriteC2Buf(VOS_UINT8 ch, PppEncBufMgm *buf)
{
    /* 转义字符处理 */
    if (((ch < PPP_XOR_C) && ((buf->accMap & (1u << ch)) != 0 )) || (ch == PPP_SYN_C) || (ch == PPP_ESC_C)) {
        PPP_ENC_WriteNonEscC2Buf(PPP_ESC_C, buf);
        PPP_ENC_WriteNonEscC2Buf((ch ^ PPP_XOR_C), buf);
    } else {
        PPP_ENC_WriteNonEscC2Buf(ch, buf);
    }
}

/* 逐一写入多个字符，每个字符进行FCS计算 */
static inline VOS_VOID PPP_ENC_WriteData2Buf(const VOS_UINT8 *data, VOS_UINT32 dataLen, PppEncBufMgm *buf)
{
    VOS_UINT32      loop;
    VOS_UINT8       cur;
    
    for (loop = 0; loop < dataLen; ++loop) {
        cur = *(data+loop);
        PPP_ENC_WriteC2Buf(cur, buf);
        buf->fcs = PPP_FCS_CalOne(buf->fcs, cur);
    }
}

/* 写入FCS字段 */
static inline VOS_VOID PPP_ENC_WriteFcs2Buf(PppEncBufMgm *buf)
{
    VOS_UINT16      fcs;

    fcs = ~(buf->fcs);

    /* 按照协议:fcs按照小端填写 */
    PPP_ENC_WriteC2Buf((VOS_UINT8)fcs, buf);
    PPP_ENC_WriteC2Buf((VOS_UINT8)(fcs >> 8), buf);
}

/* 写入地址域+控制域+协议域 */
static inline VOS_VOID PPP_ENC_WriteHdr2Buf(VOS_BOOL cAcf, VOS_BOOL cPf, VOS_UINT16 proto, PppEncBufMgm *buf)
{
    VOS_UINT32      tmpDataLen = 0;
    VOS_UINT8       tmpData[4];     /* 地址域+控制域+协议域最长4字节 */

    /* 准备地址域和控制域头部，LCP协议不能压缩 */
    if ((cAcf == VOS_FALSE) || (proto == PPP_LCP_P)) {
        tmpData[0] = PPP_ADDR_C;
        tmpData[1] = PPP_CTRL_C;
        tmpDataLen += PPP_ACF_LEN;
    }

    /* 准备协议域头部 */
    if ((cPf == VOS_TRUE) && ((proto >> 8) == 0)) {
        tmpData[tmpDataLen] = (VOS_UINT8)proto;
        tmpDataLen += PPP_CPF_LEN;
    } else {
        tmpData[tmpDataLen] = (VOS_UINT8)(proto >> 8);
        tmpData[tmpDataLen + 1] = (VOS_UINT8)proto;
        tmpDataLen += PPP_PF_LEN;
    }

    /* 填充协议头部：地址域+控制域+协议域 */
    PPP_ENC_WriteData2Buf(tmpData, tmpDataLen, buf);
}

/* 出错时，内存释放处理 */
static inline VOS_VOID PPP_ENC_ErrProc(PppEncCfg *cfg, PppEncBufMgm *buf)
{
    VOS_UINT32              loop;

    /* 此处申请内存失败不释放，外层统一调用相关接口释放 */
    for (loop = 0; loop < buf->maxSegCnt; ++loop) {
        if ((cfg->free != VOS_NULL_PTR) && (buf->output->outSegs[loop].seg.raw != VOS_NULL_PTR)) {
            cfg->free(&(buf->output->outSegs[loop].seg));
        }
    }
}

/* 处理完成时，未使用内存释放处理 */
static inline VOS_VOID PPP_ENC_ClearUnuseMem(PppEncCfg *cfg, PppEncBufMgm *buf)
{
    VOS_UINT32              loop;

    /* 此处申请内存失败不释放，外层统一调用相关接口释放 */
    for (loop = 0; loop < buf->maxSegCnt; ++loop) {
        if ((cfg->free != VOS_NULL_PTR) && (buf->output->outSegs[loop].used == 0)) {
            cfg->free(&(buf->output->outSegs[loop].seg));
        }
    }
}

/* 编码内存申请处理，最多申请两块内存 */
static inline VOS_UINT32 PPP_ENC_PrepareMem(PppEncCfg *cfg, const PppEncInput *input, PppEncBufMgm *buf)
{
    const VOS_UINT32        appendLen = 13;   /* 包括头尾0x7E+地址域+控制域+协议域+FCS */
    VOS_UINT32              encMaxLen;
    VOS_UINT32              segLen[PPP_ENC_MAX_SEGS];
    VOS_UINT32              loop;
    VOS_UINT32              loopMax;
    VOS_UINT32              ret; 

    /* 封装后可能的最大数据长度(2*原始数据长度+13B) */
    encMaxLen = (2 * input->dataLen + appendLen);
    if (encMaxLen > (cfg->memMaxLen * PPP_ENC_MAX_SEGS)) {
        return VOS_ERR;
    }

    if (encMaxLen > cfg->memMaxLen) {
        loopMax = PPP_ENC_MAX_SEGS;
        segLen[0] = cfg->memMaxLen;
        segLen[1] = (encMaxLen - cfg->memMaxLen);
    } else {
        loopMax = (PPP_ENC_MAX_SEGS - 1);
        segLen[0] = encMaxLen;
    }

    for (loop = 0; loop < loopMax; ++loop) {
        ret = VOS_ERR;
        if (cfg->alloc != VOS_NULL_PTR) {
            ret = cfg->alloc(segLen[loop], &(buf->output->outSegs[loop].seg));
        }

        /* 此处申请内存失败不释放，外层统一调用相关接口释放 */
        if (ret != VOS_OK) {
            return VOS_ERR;
        }
        buf->maxSegCnt++;
        /* 对内存申请结果进行校验 */
        if (buf->output->outSegs[loop].seg.size < segLen[loop]) {
            return VOS_ERR;
        }
    }
    
    buf->writeSeg = &(buf->output->outSegs[0]);
    buf->output->segCnt = 1;
    return VOS_OK;
}


/* 编码处理，当返回VOS_OK时，output中的结果才是有效的 */
VOS_UINT32 PPP_ENC_Proc(PppEncCfg *cfg, const PppEncInput *input, PppEncOutput *output)
{
    PppEncBufMgm    buf;
    VOS_UINT32      ret;
    VOS_UINT32      loop;

    /* 初始化编码上下文 */
    output->segCnt = 0;
    for (loop = 0; loop < PPP_ENC_MAX_SEGS; ++loop) {
        output->outSegs[loop].used = 0;
        PPP_COMM_CLEAR_UPLAYER_MEM(&(output->outSegs[loop].seg));
    }
    buf.output      = output;
    buf.accMap      = ((input->proto == PPP_LCP_P) ? PPP_DEFAULT_ACCMAP: cfg->accMap);
    buf.fcs         = PPP_INIT_FCS;
    buf.maxSegCnt   = 0;

    /* 准备内存,注意此处准备的内存块可能并没有使用完 */
    ret = PPP_ENC_PrepareMem(cfg, input, &buf);
    if (ret != VOS_OK) {
        PPP_ENC_ErrProc(cfg, &buf);
        return VOS_ERR;
    }

    /* 填写起始0x7E */
    PPP_ENC_WriteNonEscC2Buf(PPP_SYN_C, &buf);

    /* 填充协议头部：地址域+控制域+协议域 */
    PPP_ENC_WriteHdr2Buf(cfg->acf, cfg->pcf, input->proto, &buf);

    /* 填充报文内容 */
    PPP_ENC_WriteData2Buf(input->data, input->dataLen, &buf);
    
    /* 填写FCS */
    PPP_ENC_WriteFcs2Buf(&buf);
    
    /* 填写结尾0x7E */
    PPP_ENC_WriteNonEscC2Buf(PPP_SYN_C, &buf);
    
    /* 有可能预申请了两块，此处只用了一块，释放未使用的内存 */
    PPP_ENC_ClearUnuseMem(cfg, &buf);
    return VOS_OK;
}


