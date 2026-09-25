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


#include "ppp_decode.h"
#include "ppp_comm_def.h"
#include "ppp_fcs.h"



/*协议栈打印打点方式下的.C文件宏定义*/
#define THIS_FILE_ID        PS_FILE_ID_PPP_DECODE_C

/* 协议域压缩1，FCS最小2 */
#define PPP_FRM_MIN_LEN     4

typedef VOS_VOID (*PPP_DEC_ProcDataState)(PppDecCtrl *ctrl, VOS_UINT8 cur);


/* 已经找到完整7E头和7E尾的帧, 但内容不合法，丢弃该帧，申请的内存重新使用 */
static VOS_VOID PPP_DEC_ReInitBuf(PppDecCtrl *ctrl)
{
    ctrl->used      = 0;
    ctrl->fcs       = PPP_INIT_FCS;
    ctrl->escMode   = VOS_FALSE;
}

/* 已经找到完整7E头和7E尾的帧，且内容合法，递交给上层处理，保存内存的指针清空 */
static VOS_VOID PPP_DEC_ClearBuf(PppDecCtrl *ctrl)
{
    ctrl->used      = 0;
    ctrl->fcs       = PPP_INIT_FCS;
    ctrl->escMode   = VOS_FALSE;

    ctrl->buf.raw   = VOS_NULL_PTR;
    ctrl->buf.data  = VOS_NULL_PTR;
    ctrl->buf.size  = 0;
}

/* 已经找到完整7E头和7E尾的帧，本层检查通过后，填充递交给上层携带的公共参数 */
static VOS_VOID PPP_DEC_FillNewFrmInfo(const PppDecCtrl *ctrl, PppDecFrame *frm)
{
    VOS_UINT16  acf;
    VOS_UINT16  proto;
    VOS_UINT8   hdrLen = 0;
    VOS_UINT8   hdrAcLen = 0;

    /* 地址域处理，压缩场景下该域长度为0，否则长度为2 */
    acf = (VOS_UINT16)((*(ctrl->buf.data) << 8) | *(ctrl->buf.data + 1));
    if (acf == PPP_ACF_S) {
        hdrLen += PPP_ACF_LEN;
        hdrAcLen += PPP_ACF_LEN;
    }

    /* 协议域处理，压缩场景下该域长度为1，否则长度为2 */
    if ((*(ctrl->buf.data + hdrLen) & PPP_PF_MASK) != 0) {
        hdrLen += PPP_CPF_LEN;
        proto = *(ctrl->buf.data + hdrAcLen);
    } else {
        hdrLen += PPP_PF_LEN;
        proto = (VOS_UINT16)((*(ctrl->buf.data + hdrAcLen) << 8) | *(ctrl->buf.data + hdrAcLen + 1));
    }

    frm->buf = ctrl->buf;
    frm->hdrLen = hdrLen;
    frm->hdrAcLen = hdrAcLen;
    frm->used = ctrl->used;
    frm->proto = proto;
    frm->pppId = ctrl->transData;
}


/* 已经找到完整7E头和7E尾的帧处理 */
static VOS_VOID PPP_DEC_ProcNewFrm(PppDecCtrl *ctrl)
{
    PppDecFrame   newFrm;

    /* FCS校验失败，所申请到的内存继续给下次使用 */
    if (ctrl->fcs != PPP_LAST_FCS) {
        PPP_DEC_ReInitBuf(ctrl);
        return;
    }

    /* 帧长度非法，所申请到的内存继续给下次 */
    if (ctrl->used < PPP_FRM_MIN_LEN) {
        PPP_DEC_ReInitBuf(ctrl);
        return;
    }

    /* 去除掉FCS字段后，递交给高层，内存由高层释放 */
    ctrl->used -= PPP_FCS_LEN;
    if (ctrl->ops.deliver != VOS_NULL_PTR) {
        PPP_DEC_FillNewFrmInfo(ctrl, &newFrm);
        PPP_DEC_ClearBuf(ctrl);
        ctrl->ops.deliver(&newFrm);
        return;
    }

    /* 理论上不应该走到该分支 */
    PPP_DEC_ReInitBuf(ctrl);
}

/* 尝试申请内存，并保存帧内容 */
/*lint -e{730}*/
static VOS_VOID PPP_DEC_SaveData2Buf(PppDecCtrl *ctrl, VOS_UINT8 data)
{
    if (ctrl->used < ctrl->buf.size) {
        /* 保存字符，并同时计算FCS */
        *(ctrl->buf.data + ctrl->used++) = data;
        ctrl->fcs = PPP_FCS_CalOne(ctrl->fcs, data);
    } else {
        /* 数据长度超过当前处理能力，设置FCS非法，在最终重组时丢弃该帧 */
        ctrl->fcs = PPP_INIT_FCS;
    }
}

/* 已经进入组帧状态处理 */
static VOS_VOID PPP_DEC_ProcInFrm(PppDecCtrl *ctrl, VOS_UINT8 cur)
{
    VOS_UINT8 data;

    if (cur == PPP_SYN_C) {
        PPP_DEC_ProcNewFrm(ctrl);
        ctrl->state = PPP_DEC_SYN;
        ctrl->escMode = VOS_FALSE;
    } else {
        /* 转义字符前缀，进入转义状态,下一个字符再处理 */
        if ((cur == PPP_ESC_C) && (ctrl->escMode != VOS_TRUE)){
            ctrl->escMode = VOS_TRUE;
            return;
        }

        /* 转义状态下字符处理 */
        if (ctrl->escMode == VOS_TRUE) {
            data = (cur ^ PPP_XOR_C);
            ctrl->escMode = VOS_FALSE;
        } else {
            data = cur;
        }

        /* 保存字符，并同时计算FCS */
        PPP_DEC_SaveData2Buf(ctrl, data);
    }
}

/* 空闲状态，只有0x7E会被处理 */
static VOS_VOID PPP_DEC_ProcInNull(PppDecCtrl *ctrl, VOS_UINT8 cur)
{
    if (cur == PPP_SYN_C) {
        ctrl->state = PPP_DEC_SYN;
    }
}

/* 收到0x7E状态，只有非0x7E会被处理 */
static VOS_UINT32 PPP_DEC_ProcInSyn(PppDecCtrl *ctrl, VOS_UINT8 cur)
{
    if (cur == PPP_SYN_C) {
        return VOS_OK;
    }

     /* 只在当前帧的第一个数据保存时尝试申请内存，并不能保证申请成功 */
    if (ctrl->buf.data == VOS_NULL_PTR) {
        if (ctrl->ops.alloc != VOS_NULL_PTR) {
            ctrl->ops.alloc(ctrl->ops.memMaxLen, &ctrl->buf);
        }
    }
    if (ctrl->buf.data == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    ctrl->state = PPP_DEC_FRM;
    PPP_DEC_ProcInFrm(ctrl, cur);

    return VOS_OK;
}

/* PPP解码处理，调用点保证参数的有效性 */
/*lint -e{730}*/
VOS_UINT32 PPP_DEC_Proc(PppDecCtrl *ctrl, const VOS_UINT8 *data, VOS_UINT16 dataLen, VOS_UINT32 pppId)
{
    VOS_UINT32  loop;

    ctrl->transData = pppId;
    for (loop = 0; loop < dataLen; ++loop) {
        if (ctrl->state == PPP_DEC_FRM) {
            PPP_DEC_ProcInFrm(ctrl, data[loop]);
        } else if (ctrl->state == PPP_DEC_SYN) {
            if (PPP_DEC_ProcInSyn(ctrl, data[loop]) != VOS_OK) {
                return VOS_ERR;
            }
        } else {
            PPP_DEC_ProcInNull(ctrl, data[loop]);
        }
    }

    return VOS_OK;
}

/* PPP解码实体初始化，调用点保证参数的有效性 */
static inline VOS_VOID PPP_DEC_CtrlInit(PppDecCtrl *ctrl, const PppDecCfg *ops, PPP_DEC_STATE_ENUM_UINT32 state)
{
    ctrl->state = state;
    ctrl->escMode = VOS_FALSE;
    ctrl->ops = *ops;
    ctrl->used = 0;
    ctrl->fcs = PPP_INIT_FCS;
    PPP_COMM_CLEAR_UPLAYER_MEM(&ctrl->buf);
}

/* PPP解码实体初始化，调用点保证参数的有效性 */
VOS_VOID PPP_DEC_CtrlCreate(PppDecCtrl *ctrl, const PppDecCfg *ops)
{
    PPP_DEC_CtrlInit(ctrl, ops, PPP_DEC_NULL);
}

/* PPP解码实体释放，调用点保证参数的有效性 */
VOS_VOID PPP_DEC_CtrlRelease(PppDecCtrl *ctrl)
{
    PppDecCfg   nullOp = {VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR, 0};

    /* 实体释放时，残留内存释放 */
    if ((ctrl->ops.free != VOS_NULL_PTR) && (ctrl->buf.raw != VOS_NULL_PTR)) {
        ctrl->ops.free(&(ctrl->buf));
    }

    PPP_DEC_CtrlInit(ctrl, &nullOp, PPP_DEC_BUTT);
}


