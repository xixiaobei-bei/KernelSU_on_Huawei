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


#define THIS_FILE_ID PS_FILE_ID_PPP_CONVERT_C

MODULE_EXPORTED VOS_VOID PPP_ConvCreate(PPP_ConvCtrl* self, const PPP_ConvOps *ops)
{
    if ((self != VOS_NULL_PTR) && (ops != VOS_NULL_PTR)) {
        self->state = PPP_CONV_BUSY;
        PPP_DecCtrlCreate(&self->decCtrl, &ops->decCfg);
        self->encCfg = ops->encCfg;
    }
}

MODULE_EXPORTED VOS_VOID PPP_ConvGetCfg(const PPP_ConvCtrl* self, PPP_ConvOps *ops)
{
    if ((self != VOS_NULL_PTR) && (ops != VOS_NULL_PTR)) {
        ops->decCfg = self->decCtrl.ops;
        ops->encCfg = self->encCfg;
    }
}

MODULE_EXPORTED VOS_VOID PPP_ConvReCfg(PPP_ConvCtrl* self, const PPP_ConvOps *ops)
{
    if (self != VOS_NULL_PTR) {
        self->decCtrl.ops = ops->decCfg;
        self->encCfg      = ops->encCfg;
    }
}

MODULE_EXPORTED VOS_UINT32 PPP_ConvRelease(PPP_ConvCtrl* self)
{
    VOS_UINT32 ret = VOS_ERR;
    if (self != VOS_NULL_PTR) {
        if (self->state == PPP_CONV_NULL) {
            return ret;
        }

        self->state = PPP_CONV_NULL;
        PPP_DecCtrlRelease(&self->decCtrl);
        ret = VOS_OK;
    }

    return ret;
}

MODULE_EXPORTED VOS_UINT32 PPP_ConvDecode(PPP_ConvCtrl* self, const VOS_UINT8 *data, VOS_UINT16 dataLen, VOS_UINT32 transData)
{
    VOS_UINT32 ret = VOS_ERR;

    if ((self != VOS_NULL_PTR) && (data != VOS_NULL_PTR)) {
        if (self->state == PPP_CONV_NULL) {
            return VOS_ERR;
        }
        ret = PPP_DecProc(&self->decCtrl, data, dataLen, transData);
    }
    return ret;
}

MODULE_EXPORTED VOS_UINT32 PPP_ConvEncode(PPP_ConvCtrl* self, const PPP_EncInput *input, PPP_EncOutput *output)
{
    VOS_UINT32 ret = VOS_ERR;

    if ((self != VOS_NULL_PTR) && (input != VOS_NULL_PTR) && (output != VOS_NULL_PTR)) {
        if (self->state == PPP_CONV_NULL) {
            return VOS_ERR;
        }
        ret = PPP_EncProc(&self->encCfg, input, output);
    }
    return ret;
}

