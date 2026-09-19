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

#ifndef _SCM_COMMON_H_
#define _SCM_COMMON_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <osl_types.h>
#include <osl_spinlock.h>
#include <bsp_dump.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/*****************************************************************************
 结构名    : 开机log功能是否打开

 结构说明  : cPowerOnlogC和cPowerOnlogA都为1，此功能才生效
*****************************************************************************/
typedef struct
{
    /* C核的开机log是否打开*/
    char                            cPowerOnlogC;

    /* AP的开机log是否打开,默认为0。在A核linux内核初始化时根据SOCP的50M是否存在修改此NV的值:存在，此值会被修改为1；否则，保持0不变 */
    char                            cPowerOnlogA;
    char                            cSpare1;
    char                            cSpare2;
}NV_POWER_ON_LOG_SWITCH_STRU;

/*open log使用共享内存的标志结构体 */
typedef struct
{
    char cPowerOnlogA;
    char cDsSocpBuffer;
    char reserved1;
    char reserved2;
}SHM_POWER_ON_LOG_FLAG_STRU;

/* 自旋锁初始化 */
#define scm_SpinLockInit( s )           spin_lock_init((s))
/* 先禁止本地中断然后获取自旋锁 */
#define scm_SpinLockIntLock( s, p )     spin_lock_irqsave(s, p)
/* 先使能本地中断然后释放自旋锁 */
#define scm_SpinUnlockIntUnlock( s, p ) spin_unlock_irqrestore(s, p)

void *scm_UnCacheMemAlloc(u32 ulSize, unsigned long *pulRealAddr);
void scm_FlushCpuWriteBuf(void);
unsigned long scm_UncacheMemPhyToVirt(u8 *pucCurPhyAddr, u8 *pucPhyStart, u8 *pucVirtStart, u32 ulBufLen);
u32 diag_shared_mem_write(u32 eType, u32 len, char *pData);
u32 diag_shared_mem_read(u32 eType);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


