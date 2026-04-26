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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: OSA内存管理模块头文件
 * 生成日期: 2006年10月3日
 */

#ifndef _VOS_BLOCK_MEM_H
#define _VOS_BLOCK_MEM_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "v_typdef.h"

/* errno definiens */
#define VOS_ERRNO_MEMORY_ALLOC_CHECK 0x20030084
#define VOS_ERRNO_MEMORY_NON_DOPRAMEM 0x20030085
#define VOS_ERRNO_MEMORY_DOPRAMEM_OVERFLOW 0x20030086
#define VOS_ERRNO_MEMORY_HEAD_COVER 0x20030087
#define VOS_ERRNO_MEMORY_TAIL_COVER 0x20030088
#define VOS_ERRNO_MEMORY_GLOBAL_COVER 0x20030089
#define VOS_ERRNO_MEMORY_LINK_COVER 0x20030090

#define VOS_ERRNO_MSG_MEMORY_FULL 0x2003FFFF
#define VOS_ERRNO_MEMORY_FULL 0x2003FFFE
#define VOS_ERRNO_MEMORY_FREE_INPUTMSGISNULL 0x20030091
#define VOS_ERRNO_MEMORY_FREE_REPECTION 0x20030092
#define VOS_ERRNO_MEMORY_ALLOC_INPUTMSGISNULL 0x20030093
#define VOS_ERRNO_MEMORY_FREE_INPUTPIDINVALID 0x20030035
#define VOS_ERRNO_MEMORY_LOCATION_INPUTINVALID 0x2003EEEE
#define VOS_ERRNO_MEMORY_LOCATION_TIMERSPACE 0x2003DDDD
#define VOS_ERRNO_MEMORY_LOCATION_CANNOTDO 0x2003CCCC

#define VOS_ARM_ALIGNMENT (0x03U)

/* C 核 有 64K 空间, 将最后的16 K 空间拿出来给 任务定位用，每个任务最多 4K */
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
#define VOS_DUMP_MEM_TOTAL_SIZE (48 * 1024)
#define VOS_TASK_DUMP_INFO_SIZE (4 * 1024)
#else
#define VOS_DUMP_MEM_TOTAL_SIZE (16 * 1024)
#define VOS_TASK_DUMP_INFO_SIZE (4 * 1024)
#endif

#ifdef MODEM_SANITIZER
typedef struct {
    VOS_UINT32 pid;
    VOS_UINT32 depth;
    VOS_UINT32 stack[3];
} VOS_KasanMemTrack;

typedef struct {
    VOS_UINT_PTR      freeMagic;
    VOS_UINT_PTR      allocSize;
    VOS_KasanMemTrack allocTrack;
    VOS_KasanMemTrack freeTrack;
} OSA_KasanMemMeta;
#endif

typedef struct MEM_HEAD_BLOCK {
    VOS_UINT_PTR           memCtrlAddress; /* the address of  VOS_MemCtrlBlock */
    VOS_UINT_PTR           memAddress;     /* the address of User's */
    VOS_UINT32             memUsedFlag;    /* whether be used or not */
    struct MEM_HEAD_BLOCK *next;           /*  next block whether allocated or not */

#if VOS_YES == VOS_MEMORY_CHECK
    struct MEM_HEAD_BLOCK *pre;            /* the allocated block's previous */
    VOS_UINT32             allocSize;      /* the actual allocated size */
    VOS_UINT32             ulcputickAlloc; /* CPU tick of message allocation */
    VOS_UINT32             allocPid;       /* Pid who alloc the memory */
    VOS_UINT32             memRecord[8];   /* record something of user */
#endif

#if VOS_YES == VOS_MEMORY_COUNT
    VOS_UINT_PTR realCtrlAddr; /* which VOS_MemCtrlBlock should be allocated */
#endif
#ifdef MODEM_SANITIZER
    OSA_KasanMemMeta kasanInfo; /* kasan mem info */
#endif
} VOS_MemHeadBlock;

typedef struct {
    VOS_UINT32        blockLength;      /* block size */
    VOS_UINT32        totalBlockNumber; /* block number */
    VOS_UINT_PTR      buffer;           /* start address of block */
    VOS_UINT_PTR      bufferEnd;        /* end address of block */
    VOS_UINT32        idleBlockNumber;  /* the number of free block */
    VOS_UINT8         rsv[4];
    VOS_MemHeadBlock *blocks; /* list of free block */

#if VOS_YES == VOS_MEMORY_CHECK
    VOS_MemHeadBlock *busyBlocks; /* list of busy block */
#endif

#if VOS_YES == VOS_MEMORY_COUNT
    VOS_UINT32 minIdleBlockNumber; /* the MIN value of free block */
    VOS_UINT32 minSize;            /* the Min allocated size */
    VOS_UINT32 maxSize;            /* the Max allocated size */
    VOS_UINT32 realNumber;
    VOS_UINT32 maxRealNumber;
#endif
    VOS_UINT8 rsv1[4];
} VOS_MemCtrlBlock;

typedef struct {
#if VOS_YES == VOS_MEMORY_COUNT
    VOS_UINT32 minSize; /* the Min allocated size */
    VOS_UINT32 maxSize; /* the Max allocated size */
    VOS_UINT32 maxRealNumber;
    VOS_UINT8  rsv[4];
#endif
} VOS_MemRecord;

typedef struct {
    VOS_UINT32 totalNumber; /* the total number of occupying Msg\Mem */
    VOS_UINT32 size;        /* the total size of occupying Msg\Mem */
} VOS_MemOccupyingInfo;

/* the type of info which come from task */
enum VOS_ExcDumpMemNum {
    VOS_EXC_DUMP_MEM_NUM_1,
    VOS_EXC_DUMP_MEM_NUM_2,
    VOS_EXC_DUMP_MEM_NUM_3,
    VOS_EXC_DUMP_MEM_NUM_4,
    VOS_EXC_DUMP_MEM_NUM_BUTT
};
typedef VOS_UINT32 VOS_ExcDumpMemNumUint32;

/**
 * 内存对齐方式。
 * 需要和RTOS的OS_MEM_ALIGN_E一一对应
 */
typedef enum {
    VOS_UNCACHE_MEM_ADDR_ALIGN_004 = 2,  /**< 4字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_008 = 3,  /**< 8字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_016 = 4,  /**< 16字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_032 = 5,  /**< 32字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_064 = 6,  /**< 64字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_128 = 7,  /**< 128字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_256 = 8,  /**< 256字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_512 = 9,  /**< 512字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_1K  = 10, /**< 1K字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_2K  = 11, /**< 2K字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_4K  = 12, /**< 4K字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_8K  = 13, /**< 8K字节对齐*/
    VOS_UNCACHE_MEM_ADDR_ALIGN_16K = 14, /**< 16K字节对齐*/
    VOS_UNCACHE_MEM_ADDR_BUTT            /**< 字节对齐非法*/
} VOS_UncacheMemAlign;

VOS_UINT32 VOS_MemInit(VOS_VOID);

VOS_VOID *VOS_MemBlkMalloc(VOS_PID pid, VOS_UINT32 length, VOS_UINT32 fileId, VOS_INT32 lineNo);

VOS_VOID *VOS_TIMER_MemCtrlBlkMalloc(VOS_PID pid, VOS_UINT32 length);

/*lint -function(malloc(1),V_MemAlloc(3))*/
VOS_VOID *V_MemAlloc(VOS_UINT32 info, VOS_UINT8 ptNo, VOS_UINT32 size, VOS_UINT32 rebootFlag, VOS_UINT32 fileId,
                     VOS_INT32 lineNo);

#define VOS_MemAlloc(ulInfo, ucPtNo, ulSize) V_MemAlloc((ulInfo), (ucPtNo), (ulSize), VOS_TRUE, VOS_FILE_ID, __LINE__)

#define VOS_AssistantMemAlloc(ulInfo, ucPtNo, ulSize) \
    V_MemAlloc((ulInfo), (ucPtNo), (ulSize), VOS_FALSE, VOS_FILE_ID, __LINE__)

VOS_UINT32 V_MemFree(VOS_UINT32 info, VOS_VOID **ppAddr, VOS_UINT32 fileId, VOS_INT32 lineNo);

#define VOS_MemFree(ulInfo, pAddr) V_MemFree((ulInfo), (VOS_VOID **)(&(pAddr)), VOS_FILE_ID, __LINE__)

VOS_UINT32 VOS_MemCheck(const VOS_VOID *addr, VOS_UINT_PTR *block, VOS_UINT_PTR *ctrl, VOS_UINT32 fileId, VOS_INT32 lineNo);

VOS_UINT32 VOS_MemCtrlBlkFree(VOS_MemCtrlBlock *vosMemCtrlBlock, VOS_MemHeadBlock *block, VOS_UINT32 fileId,
                              VOS_PID pid);

VOS_UINT32 VOS_LocationMem(const VOS_VOID *addr, VOS_UINT32 *realSize, VOS_UINT32 fileId, VOS_INT32 lineNo);

VOS_VOID VOS_RecordmemInfo(VOS_MemRecord *para);

VOS_UINT32 VOS_GetFreeMemoryInfo(VOS_UINT32 size, VOS_UINT32 *freeBlockNum, VOS_UINT32 *totalBlockNum);

VOS_UINT32 VOS_GetMemOccupyingInfo(VOS_MemOccupyingInfo *info, VOS_UINT32 len);

VOS_UINT32 VOS_GetMsgOccupyingInfo(VOS_MemOccupyingInfo *info, VOS_UINT32 len);

VOS_VOID *VOS_StaticMemAlloc(const VOS_CHAR *pcBuf, VOS_UINT32 bufSize, VOS_UINT32 allocSize, VOS_UINT32 *suffix);

VOS_BOOL VOS_CalcMsgInfo(VOS_VOID);

VOS_BOOL VOS_CalcMemInfo(VOS_VOID);

VOS_BOOL VOS_CalcAppMemInfo(VOS_VOID);

#if VOS_YES == VOS_MEMORY_CHECK

VOS_VOID VOS_AutoCheckMemory(VOS_VOID);

extern VOS_UINT32 g_autoCheckMemoryThreshold;

#endif

#if (VOS_DEBUG == VOS_DOPRA_VER)

typedef VOS_VOID (*MEMORY_HOOK_FUNC)(VOS_UINT32 ulPara);

#define VOS_ALLOC_MODE 0
#define VOS_FREE_MODE 1

VOS_UINT32 VOS_RegisterMemAllocHook(VOS_UINT32 mode, MEMORY_HOOK_FUNC pfnHook);

#endif

VOS_VOID VOS_ModifyMemBlkInfo(VOS_UINT_PTR addr, VOS_PID pid);

VOS_VOID *VOS_ExcDumpMemAlloc(VOS_UINT32 number);

VOS_VOID *VOS_CacheMemAllocDebug(VOS_UINT32 size, VOS_UINT32 cookie);

VOS_UINT32 VOS_CacheMemFree(const VOS_VOID *addr);

VOS_VOID *VOS_UnCacheMemAllocDebug(VOS_UINT32 size, VOS_UncacheMemAlign align, VOS_UINT_PTR *realAddr,
                                   VOS_UINT32 cookie);

VOS_VOID VOS_UnCacheMemFree(VOS_VOID *virtAddr, VOS_VOID *phyAddr, VOS_UINT32 size);

VOS_UINT_PTR VOS_UncacheMemPhyToVirt(const VOS_UINT8 *curPhyAddr, const VOS_UINT8 *phyStart, const VOS_UINT8 *virtStart,
                                     VOS_UINT32 bufLen);

VOS_UINT_PTR VOS_UncacheMemVirtToPhy(const VOS_UINT8 *curVirtAddr, const VOS_UINT8 *phyStart, const VOS_UINT8 *virtStart,
                                     VOS_UINT32 bufLen);

VOS_VOID VOS_FlushCpuWriteBuf(VOS_VOID);

VOS_VOID VOS_FlushCpuCache(VOS_VOID *address, VOS_UINT size);

#if defined(COMM_ACPU_PC_LINT_HAPPY) || (VOS_WIN32 == VOS_OS_VER)

enum {
    GFP_KERNEL,
    GFP_DMA
};

VOS_VOID *kmalloc(VOS_UINT32 size, VOS_UINT32 flag);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _VOS_BLOCK_MEM_H */
