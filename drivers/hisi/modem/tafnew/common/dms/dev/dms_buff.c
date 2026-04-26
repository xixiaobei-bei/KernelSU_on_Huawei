/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "dms_buff.h"

#include "dms_debug.h"
#if (VOS_OS_VER != VOS_LINUX)
#include "Linuxstub.h"
#endif

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_BUFF_C

struct DMS_StaticBuf g_dmsStaticBufInfo = {0};
VOS_UINT8 *g_dmsStaticBuf = VOS_NULL_PTR;

STATIC struct DMS_StaticBuf *DMS_BUF_GetStaticBufInfo(VOS_VOID)
{
    return &g_dmsStaticBufInfo;
}

STATIC VOS_UINT8 *DMS_BUF_GetStaticBufAddr(VOS_VOID)
{
    return g_dmsStaticBuf;
}

STATIC VOS_VOID DMS_BUF_SetStaticBufAddr(VOS_UINT8 *dmsStaticBuf, VOS_UINT32 bufSize)
{
    g_dmsStaticBuf = dmsStaticBuf;
}

VOS_BOOL DMS_BUF_IsStaticBuf(VOS_UINT8 *buf)
{
    VOS_UINT8 *dmsStaticBuf = VOS_NULL_PTR;

    dmsStaticBuf = DMS_BUF_GetStaticBufAddr();
    /* 地址在DMS_DEV_StaticBufInit申请的AT发送静态buf区间之内 */
    if ((buf >= dmsStaticBuf) && (buf < dmsStaticBuf + DMS_STATIC_ONE_BUF_SIZE *
        DMS_STATIC_BUF_NUM + DMS_STATIC_BUF_SIZE_EXT)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT8 *DMS_BUF_GetStaticBuf(VOS_UINT32 len)
{
    VOS_UINT8            *buf = VOS_NULL_PTR;
    struct DMS_StaticBuf *dmsStaticBufInfo = VOS_NULL_PTR;
    VOS_UINT32            i = 0;

    /* 数据长度超过静态buf最大size，返回失败 */
    if (len > DMS_STATIC_ONE_BUF_SIZE) {
        return VOS_NULL_PTR;
    }

    dmsStaticBufInfo = DMS_BUF_GetStaticBufInfo();
    for (i = 0; i < DMS_STATIC_BUF_NUM; i++) {
        if (dmsStaticBufInfo->bufSta[i].busy == DMS_BUF_STA_IDLE) {
            dmsStaticBufInfo->bufSta[i].busy = DMS_BUF_STA_BUSY;

            return dmsStaticBufInfo->bufSta[i].buff;
        }
    }

    /* 极限场景下 如果静态buf用完，申请动态内存使用 */
    buf = kmalloc(len, GFP_KERNEL | __GFP_DMA);
    if (buf == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }

    (VOS_VOID)memset_s(buf, len, 0x00, len);
    return buf;
}

VOS_VOID DMS_BUF_FreeStaticBuf(VOS_UINT8 *buf)
{
    struct DMS_StaticBuf *dmsStaticBufInfo = VOS_NULL_PTR;
    VOS_UINT32            i = 0;

    if (buf == VOS_NULL_PTR) {
        return;
    }

    dmsStaticBufInfo = DMS_BUF_GetStaticBufInfo();

    /* 静态buf释放 */
    for (i = 0; i < DMS_STATIC_BUF_NUM; i++) {
        if (dmsStaticBufInfo->bufSta[i].buff == buf) {
            dmsStaticBufInfo->bufSta[i].busy = DMS_BUF_STA_IDLE;
            return;
        }
    }

    /* 动态buf释放 */
    if (i == DMS_STATIC_BUF_NUM) {
        kfree(buf);
        buf = VOS_NULL_PTR;
    }
}

VOS_VOID DMS_BUF_StaticBufInit(VOS_VOID)
{
    struct DMS_StaticBuf *dmsStaticBufInfo = VOS_NULL_PTR;
    VOS_UINT8            *dmsStaticBuf = VOS_NULL_PTR;
    VOS_UINT8            *temp = VOS_NULL_PTR;
    VOS_UINT32            i = 0;
    VOS_UINT32            bufSize;

    bufSize = (DMS_STATIC_ONE_BUF_SIZE * DMS_STATIC_BUF_NUM + DMS_STATIC_BUF_SIZE_EXT);
    dmsStaticBuf = kmalloc(bufSize, GFP_KERNEL | __GFP_DMA);
    if (dmsStaticBuf == VOS_NULL_PTR) {
        DMS_LOGE("kmalloc fail.");
        return;
    }
    (VOS_VOID)memset_s(dmsStaticBuf, bufSize, 0x00, bufSize);
    DMS_BUF_SetStaticBufAddr(dmsStaticBuf, bufSize);

    /* 取32字节对齐的地址 */
    temp = dmsStaticBuf + (DMS_STATIC_BUF_SIZE_EXT - ((VOS_UINT_PTR)dmsStaticBuf % DMS_STATIC_BUF_SIZE_EXT));

    dmsStaticBufInfo = DMS_BUF_GetStaticBufInfo();
    dmsStaticBufInfo->bufType = DMS_BUF_TYP_DYMIC;

    /* 初始化缓冲信息 */
    for (i = 0; i < DMS_STATIC_BUF_NUM; i++) {
        dmsStaticBufInfo->bufSta[i].buff = (VOS_UINT8 *)((VOS_ULONG)i * DMS_STATIC_ONE_BUF_SIZE + temp);
        dmsStaticBufInfo->bufSta[i].busy = DMS_BUF_STA_IDLE;
    }
}

