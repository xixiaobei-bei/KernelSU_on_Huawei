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

#include "imm_mem_zc.h"
#include "v_id.h"
#include "product_config.h"
#include "PsTypeDef.h"
#include "ttf_util.h"
#include "securec.h"

#define THIS_FILE_ID PS_FILE_ID_IMM_ZC_C

#define IMM_PRIV_CB(skb) ((IMM_PrivCb*)((skb)->cb))

#pragma pack(push, 4)
typedef struct {
    unsigned short app;
    unsigned char  rsv[2];
} IMM_PrivCb;
#pragma pack(pop)

/*
 * 给IMM_Zc零拷贝结构添加MAC头
 * ndis acore实现使用该接口, ndis部署在ccore时，不需要该接口
 */
unsigned int IMM_ZcAddMacHead(IMM_Zc *immZc, const unsigned char *addData)
{
    unsigned char* destAddr = VOS_NULL_PTR;

    if ((immZc == VOS_NULL_PTR) || (addData == VOS_NULL_PTR)) {
        return VOS_ERR;
    }

    if ((immZc->data - immZc->head) < IMM_MAC_HEADER_RES_LEN) {
        return VOS_ERR;
    }

    destAddr = IMM_ZcPush(immZc, IMM_MAC_HEADER_RES_LEN);
    TTF_ACORE_SF_CHK(memcpy_s(destAddr, IMM_MAC_HEADER_RES_LEN, addData, IMM_MAC_HEADER_RES_LEN));
    return VOS_OK;
}

/*
 * 移除零拷贝结构MAC头
 * ndis acore实现使用该接口, ndis部署在ccore时，不需要该接口
 */
unsigned int IMM_ZcRemoveMacHead(IMM_Zc *immZc)
{
    if ((immZc == VOS_NULL_PTR) || (immZc->len < IMM_MAC_HEADER_RES_LEN)) {
        return VOS_ERR;
    }

    IMM_ZcPull(immZc, IMM_MAC_HEADER_RES_LEN);
    return VOS_OK;
}

/* 得到UserApp。 */
unsigned short IMM_ZcGetUserApp(IMM_Zc *immZc)
{
    if (immZc == VOS_NULL_PTR) {
        return 0;
    }

    return IMM_PRIV_CB(immZc)->app;
}

/* 设置UserApp。 */
void IMM_ZcSetUserApp(IMM_Zc *immZc, unsigned short app)
{
    if (immZc != VOS_NULL_PTR) {
        IMM_PRIV_CB(immZc)->app = app;
    }
}
