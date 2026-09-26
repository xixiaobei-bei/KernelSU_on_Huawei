/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef IMM_IMPL_ACORE_H
#define IMM_IMPL_ACORE_H

#include "vos.h"
#include "product_config.h"

#if (defined(CONFIG_BALONG_SPE) && (VOS_OS_VER == VOS_LINUX))
#include "mdrv_spe_wport.h"
#endif

#if (VOS_OS_VER == VOS_LINUX)
#include <linux/skbuff.h>
#else
#include "skbuff.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/* 为了屏蔽不同版本的差异, IMM_ZcHeader/ImmZc放到头文件中 */
typedef struct sk_buff_head IMM_ZcHeader;

typedef struct sk_buff IMM_Zc;

#define IMM_ZcStaticAlloc(len) dev_alloc_skb(len)

/* 释放IMM_ZC_STRU内存 */
#if (defined(CONFIG_BALONG_SPE) && (VOS_OS_VER == VOS_LINUX))
#define IMM_ZcFree(immZc) mdrv_spe_wport_recycle(immZc)
#define IMM_ZcFreeAny(pstImmZc) mdrv_spe_wport_recycle((pstImmZc))
#else
#define IMM_ZcFree(immZc) kfree_skb(immZc)
#define IMM_ZcFreeAny(pstImmZc) dev_kfree_skb_any((pstImmZc))
#endif

/*
 * 添加MAC头. IMM_Zc - 指向IMM_ZC_STRU的指针, AddData - 添加的MAC头首地址, Len - MAC头的长度
 * 操作成功与否, VOS_OK - 成功, VOS_ERR - 失败
 * 此函数区分产品的形态,如MBB上此接口有效,智能机上返回ERR
 *
 */
extern unsigned int IMM_ZcAddMacHead(IMM_Zc *immZc, const unsigned char *addData);

/*
 * 从以太帧中剥去MAC头。
 * 操作成功与否, VOS_OK - 成功, VOS_ERR - 失败
 * 此函数区分产品的形态,如MBB上此接口有效,智能机上返回ERR
 *
 */
extern unsigned int IMM_ZcRemoveMacHead(IMM_Zc *immZc);

/* 
 * 数据添加到有效数据块的头部
 * 函数返回的数据块首地址，并且是添加数据之后的数据块地址。
 * 本接口只移动指针, 数据添加到有效数据块的头部之前,调用本接口
 */
#define IMM_ZcPush(immZc, len) skb_push(immZc, len)

/*
 * 从IMM_ZC指向的数据块的头部取出数据
 * 返回的数据块首地址，并且是取出数据之后的地址
 * 本接口只移动指针
 */
#define IMM_ZcPull(immZc, len) skb_pull(immZc, len)

/*
 * 添加数据在IMM_ZC指向的数据块的尾部
 * 返回的数据块尾部地址，并且是添加数据之前的数据块尾部地址。
 * 本接口只移动指针
 */
#define IMM_ZcPut(immZc, len) skb_put(immZc, len)

/*
 * 预留IMM_ZC指向的数据块头部空间。
 * 本接口只移动指针，为头部预留空间。
 * 只用于刚分配的IMM_ZC,IMM_ZC指向的数据块还没有使用；
 */
#define IMM_ZcReserve(immZc, len) skb_reserve(immZc, (int)len)

/* 获取预留空间长度 */
#define IMM_ZcGetReserveRoom(immZc) ((immZc)->end - (immZc)->tail)

/* 重置IMM_ZC尾部地址 */
#define IMM_ZcResetTailPointer(immZc) skb_reset_tail_pointer(immZc)

/* 得到数据头部预留空间字节数。 返回预留空间字节数 */
#define IMM_ZcHeadRoom(immZc) skb_headroom(immZc)

/* 得到数据尾部预留空间字节数。 返回预留空间字节数 */
#define IMM_ZcTailRoom(immZc) skb_tailroom(immZc)

/* 得到数据块首地址。 返回数据块首地址 */
#define IMM_ZcGetDataPtr(immZc) ((immZc)->data)

/* 得到数据块使用的字节数。 返回数据块使用的字节数 */
#define IMM_ZcGetUsedLen(immZc) ((immZc)->len)

/* 得到数据块使用的Protocol， 返回数据块使用Protocol */
#define IMM_ZcGetProtocol(immZc) ((immZc)->protocol)

/* 设置数据块使用的Protocol */
#define IMM_ZcSetProtocol(immZc, proto) ((immZc)->protocol = proto)

/* 获取数据块的优先级 */
#define IMM_ZcGetPriority(immZc) ((immZc)->priority)

/* 设置数据块的优先级 */
#define IMM_ZcSetPriority(immZc, pri) ((immZc)->priority = pri)

/* 设置内存跟踪信息 */
static inline VOS_VOID IMM_ZcTraceMem(IMM_Zc* immZc) {}

/*
 * 得到UserApp
 * 区分MBB和智能机形态,MBB上此接口有效,智能机上无效,直接返回0
 * 因此要求外部使用者代码隔离
 */
extern unsigned short IMM_ZcGetUserApp(IMM_Zc *immZc);

/*
 * 设置UserApp
 * 区分MBB和智能机形态,MBB上此接口有效,智能机上无效,为空函数
 * 因此要求外部使用者代码隔离
 */
extern void IMM_ZcSetUserApp(IMM_Zc *immZc, unsigned short app);

/* 队列初始化。 */
#define IMM_ZcQueueHeadInit(zcQueue) skb_queue_head_init(zcQueue)

/* 元素插入队列头部。 */
#define IMM_ZcQueueHead(zcQueue, newItem) skb_queue_head(zcQueue, newItem)

/* 元素插入队列尾部。 */
#define IMM_ZcQueueTail(zcQueue, newItem) skb_queue_tail(zcQueue, newItem)

/* 从队列头部取元素。 返回指向IMM_ZC_STRU的指针 */
#define IMM_ZcDequeueHead(zcQueue) skb_dequeue(zcQueue)

/* 从队列尾部取元素。 返回指向IMM_ZC_STRU的指针 */
#define IMM_ZcDequeueTail(zcQueue) skb_dequeue_tail(zcQueue)

/* 得到队列中的元素的数目。 */
#define IMM_ZcQueueLen(zcQueue) skb_queue_len(zcQueue)

/* 得到队列中的队首元素的指针 */
#define IMM_ZcQueuePeek(zcQueue) skb_peek(zcQueue)

/* 得到队列中的队尾元素的指针 */
#define IMM_ZcQueuePeekTail(zcQueue) skb_peek_tail(zcQueue)

#pragma pack()

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

