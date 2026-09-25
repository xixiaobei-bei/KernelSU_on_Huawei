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

#include "ppp_convert.h"


/*协议栈打印打点方式下的.C文件宏定义*/
#define THIS_FILE_ID            PS_FILE_ID_PPP_CONVERT_C
#define PPP_CTRL_ENT(pppId)     (&g_pppConvEnt[(pppId)])

enum PppConvState {
    PPP_CONV_NULL,
    PPP_CONV_BUSY,
    PPP_CONV_BUTT
};
typedef VOS_UINT32 PPP_CONV_STATE_ENUM_UINT32;

/* 解码控制实体 */
typedef struct {
    PPP_CONV_STATE_ENUM_UINT32      state;
    PppDecCtrl                      decCtrl;
    PppEncCfg                       encCfg;
} PppConvCtrl;

PppConvCtrl                         g_pppConvEnt[PPP_CONV_MAX_CNT] = {{0}};

VOS_VOID PPP_CONV_Create(VOS_UINT16 pppId, const PppConvOps *ops)
{
    if (pppId < PPP_CONV_MAX_CNT) {
        PPP_CTRL_ENT(pppId)->state = PPP_CONV_BUSY;
        PPP_DEC_CtrlCreate(&(PPP_CTRL_ENT(pppId)->decCtrl), &ops->decCfg);
        PPP_CTRL_ENT(pppId)->encCfg = ops->encCfg;
    }
}

VOS_VOID PPP_CONV_GetCfg(VOS_UINT16 pppId, PppConvOps *ops)
{
    if (pppId < PPP_CONV_MAX_CNT) {
        ops->decCfg = PPP_CTRL_ENT(pppId)->decCtrl.ops;
        ops->encCfg = PPP_CTRL_ENT(pppId)->encCfg;
    }
}

VOS_VOID PPP_CONV_ReCfg(VOS_UINT16 pppId, const PppConvOps *ops)
{
    if (pppId < PPP_CONV_MAX_CNT) {
        PPP_CTRL_ENT(pppId)->decCtrl.ops    = ops->decCfg;
        PPP_CTRL_ENT(pppId)->encCfg         = ops->encCfg;
    }
}

VOS_UINT32 PPP_CONV_Release(VOS_UINT16 pppId)
{
    VOS_UINT32  ret = VOS_ERR;
    if (pppId < PPP_CONV_MAX_CNT) {
        if (PPP_CTRL_ENT(pppId)->state == PPP_CONV_NULL) {
            return ret;
        }

        PPP_CTRL_ENT(pppId)->state = PPP_CONV_NULL;
        PPP_DEC_CtrlRelease(&(PPP_CTRL_ENT(pppId)->decCtrl));
        ret = VOS_OK;
    }

    return ret;
}

VOS_UINT32 PPP_CONV_Decode(VOS_UINT16 pppId, const VOS_UINT8 *data, VOS_UINT16 dataLen)
{
    VOS_UINT32  ret = VOS_ERR;

    if (pppId < PPP_CONV_MAX_CNT) {
        if (PPP_CTRL_ENT(pppId)->state == PPP_CONV_NULL) {
            return VOS_ERR;
        }
        ret = PPP_DEC_Proc(&(PPP_CTRL_ENT(pppId)->decCtrl), data, dataLen, pppId);
    }
    return ret;
}

VOS_UINT32 PPP_CONV_Encode(VOS_UINT16 pppId, const PppEncInput *input, PppEncOutput *output)
{
    VOS_UINT32  ret = VOS_ERR;

    if (pppId < PPP_CONV_MAX_CNT) {
        if (PPP_CTRL_ENT(pppId)->state == PPP_CONV_NULL) {
            return VOS_ERR;
        }
        ret = PPP_ENC_Proc(&(PPP_CTRL_ENT(pppId)->encCfg), input, output);
    }
    return ret;
}


