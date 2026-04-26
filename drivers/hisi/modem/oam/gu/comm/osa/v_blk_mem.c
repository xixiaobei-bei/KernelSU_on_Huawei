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
 * 功能描述: OSA内存管理功能实现
 * 生成日期: 2006年10月3日
 */

#include "vos_config.h"
#include "v_typdef.h"
#include "v_blk_mem.h"
#include "v_io.h"
#include "v_timer.h"
#include "vos_id.h"
#include "v_private.h"
#include "v_lib.h"
#include "product_config.h"
#include "v_mem_cfg.h"
#include "mdrv.h"
#include "pam_tag.h"


#if (VOS_OS_VER == VOS_LINUX)
#include <asm/dma-mapping.h>
#include <linux/version.h>
#ifdef CONFIG_ARM64
#include <linux/of_device.h>
#endif
#endif

#if (VOS_RTOSCK == VOS_OS_VER)
#include "sre_cache.h"
#endif

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_V_BLKMEM_C
#define THIS_MODU mod_pam_osa

/* which be used to calc real size of mem */
typedef struct {
    VOS_UINT_PTR address;
    VOS_UINT32   type;
    VOS_INT      cycle;
    VOS_UINT32   fileId;
    VOS_INT32    lineNo;
} VOS_LocationMemInfo;

/* the start address of mem pool */
VOS_UINT_PTR g_memPoolIntervalAddress[VOS_MEM_POOL_NUMBER + 1];

/* message control blocks */
VOS_MemCtrlBlock g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER];

/* memory control blocks */
VOS_MemCtrlBlock g_vosSimpleMemCtrlBlk[VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER];

#define VOS_MEM_HEAD_BLOCK_SIZE (sizeof(VOS_MemHeadBlock))

/* be used the head and tail of memory to protect overflow */
#define VOS_MEMORY_PROTECT_SIZE 1024

/* the number of words which on the head of the user's space */
#define VOS_MEMORY_RESERVED_WORD_HEAD 2

/* the number of words which on the tail of the user's space */
#define VOS_MEMORY_RESERVED_WORD_TAIL 1

/* 导出OSA内存控制信息的个数 */
#define VOS_DUMP_MEM_HEAD_NUM (3)

/* 导出OSA内存控制信息的个数加一是为了带出其它信息 */
#define VOS_DUMP_MEM_HEAD_TOTAL_NUM (VOS_DUMP_MEM_HEAD_NUM + 1)

/* the number of bytes which on the user's space */
#define VOS_MEMORY_RESERVED_BYTES \
    ((sizeof(VOS_UINT_PTR) * VOS_MEMORY_RESERVED_WORD_HEAD) + (sizeof(VOS_UINT_PTR) * VOS_MEMORY_RESERVED_WORD_TAIL))

/* 由于LINUX不支持浮点类型，由之前的浮点类型改为整型 0.1 -> 1 */
/* the ratio of count */
#define VOS_CALC_MEM_RATIO (10)
/* the begin address of user's space */
VOS_UINT_PTR g_vosSpaceStart;

/* the end address of user's space */
VOS_UINT_PTR g_vosSpaceEnd;

/* the begin address of user's space */
VOS_UINT_PTR g_vosSpaceAndProtectionStart;

/* the begin address of user's space */
VOS_UINT_PTR g_vosSpaceAndProtectionEnd;

/* the begin address of Mem Control space */
VOS_UINT_PTR g_vosMemCtrlSpaceStart;

/* the end address of Mem Control space */
VOS_UINT_PTR g_vosMemCtrlSpaceEnd;

/* the buf of VOS's mem */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#if (FEATURE_ON == FEATURE_VOS_REDUCE_MEM_CFG)
VOS_CHAR g_vosMemBuf[4559000];
#else
VOS_CHAR g_vosMemBuf[4881856];
#endif
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
/* the buf of VOS's mem */
VOS_CHAR g_vosMemBuf[1577424]; /* 由于A核支持64位，内存需要增大 */
#endif

#if (OSA_CPU_NRCPU == VOS_OSA_CPU)
VOS_CHAR g_vosMemBuf[4608600];
#endif

#if VOS_YES == VOS_MEMORY_CHECK

MODULE_EXPORTED VOS_UINT32 g_autoCheckMemoryThreshold;

VOS_VOID VOS_PrintUsedMsgInfo(VOS_VOID);

VOS_VOID VOS_PrintUsedTimerMsgInfo(VOS_VOID);
#endif

VOS_VOID VOS_MemDump(VOS_UINT32 info, VOS_UINT32 size, VOS_UINT32 fileId, VOS_INT32 lineNo,
                     const VOS_MemCtrlBlock *memCtrl, VOS_INT cycle);

VOS_VOID VOS_MsgDump(VOS_UINT32 info, VOS_UINT32 size, VOS_UINT32 fileId, VOS_INT32 lineNo);

#if (VOS_DEBUG == VOS_DOPRA_VER)

MEMORY_HOOK_FUNC g_funcAllocMemHook = VOS_NULL_PTR;
MEMORY_HOOK_FUNC g_funcFreeMemHook  = VOS_NULL_PTR;

#endif

extern VOS_UINT32 VOS_GetMsgName(VOS_UINT_PTR addrress);

/* 自旋锁，用来作Memory的临界资源保护 */
VOS_SPINLOCK g_vosMemSpinLock;

/* 自旋锁，用来作静态Memory的临界资源保护 */
VOS_SPINLOCK g_vosStaticMemSpinLock;

/* 自旋锁，用来作Dump Memory的临界资源保护 */
VOS_SPINLOCK g_vosDumpMemSpinLock;
VOS_UINT32   g_vosDumpMemFlag;

#if (VOS_LINUX == VOS_OS_VER)
#ifdef CONFIG_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))

struct platform_device *g_modemOsaPdev;

static int osa_driver_probe(struct platform_device *pdev)
{
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));

    g_modemOsaPdev = pdev;

    return VOS_OK;
}

static int osa_driver_remove(struct platform_device *pdev)
{
    return VOS_OK;
}

static const struct of_device_id g_osaDevOfMatch[] = {
    {
        .compatible = "hisilicon,hisi-osa",
        .data       = NULL,
    },
    {},
};

static struct platform_driver g_osaDriver = {
    .probe  = osa_driver_probe,
    .remove = osa_driver_remove,
    .driver = {
        .name = "hisi-osa",
        .of_match_table = g_osaDevOfMatch,
    },
};

#endif
#endif
#endif

/*
 * Description: Init one memory control block
 */
VOS_VOID VOS_MemCtrlBlkInit(VOS_MemCtrlBlock *vosMemCtrlBlock, VOS_UINT32 blockLength,
                            VOS_UINT32 totalBlockNumber, VOS_UINT_PTR *ctrlAddress, VOS_UINT_PTR *spaceAddress)
{
    VOS_UINT32          length;
    VOS_UINT32          i;
    VOS_MemHeadBlock *block = VOS_NULL_PTR;
    VOS_UINT_PTR        temp;
    VOS_UINT_PTR       *tempAddr = VOS_NULL_PTR;

    vosMemCtrlBlock->blockLength      = blockLength;
    vosMemCtrlBlock->totalBlockNumber = totalBlockNumber;
    vosMemCtrlBlock->idleBlockNumber  = totalBlockNumber;

    length = (VOS_MEMORY_RESERVED_BYTES + blockLength);

    vosMemCtrlBlock->buffer = *spaceAddress;
    /*lint -e647*/
    vosMemCtrlBlock->bufferEnd = *spaceAddress + length * totalBlockNumber;
    /*lint +e647*/
    vosMemCtrlBlock->blocks = (VOS_MemHeadBlock *)(*ctrlAddress);

    block = vosMemCtrlBlock->blocks;

    temp = vosMemCtrlBlock->buffer;

    for (i = 1; i < totalBlockNumber; i++) {
        block->memCtrlAddress = (VOS_UINT_PTR)vosMemCtrlBlock;
        block->memAddress     = temp;
        block->memUsedFlag    = VOS_NOT_USED;
        block->next          = block + 1;

        /* add protection on head */
        tempAddr  = (VOS_UINT_PTR *)temp;
        *tempAddr = (VOS_UINT_PTR)block;
        tempAddr++;
        *tempAddr = VOS_MEMORY_CRC;

        /* offset next */
        block++;
        temp += length;

        /* add protection on tail */
        tempAddr = (VOS_UINT_PTR *)temp;
        tempAddr--;
        *tempAddr = VOS_MEMORY_CRC;
    }
    block->memCtrlAddress = (VOS_UINT_PTR)vosMemCtrlBlock;
    block->memAddress     = temp;
    block->memUsedFlag    = VOS_NOT_USED;
    block->next          = VOS_NULL_PTR;
    /* add protection on head */
    tempAddr  = (VOS_UINT_PTR *)temp;
    *tempAddr = (VOS_UINT_PTR)block;
    tempAddr++;
    *tempAddr = VOS_MEMORY_CRC;

    /* offset next */
    block++;
    temp += length;

    /* add protection on tail */
    tempAddr = (VOS_UINT_PTR *)temp;
    tempAddr--;
    *tempAddr = VOS_MEMORY_CRC;

    *ctrlAddress  = (VOS_UINT_PTR)block;
    *spaceAddress = temp;

#if VOS_YES == VOS_MEMORY_CHECK
    vosMemCtrlBlock->busyBlocks = VOS_NULL_PTR;
#endif

#if VOS_YES == VOS_MEMORY_COUNT
    vosMemCtrlBlock->minIdleBlockNumber = totalBlockNumber;
    vosMemCtrlBlock->minSize            = 0x0fffffff;
    vosMemCtrlBlock->maxSize            = 0;
    vosMemCtrlBlock->realNumber        = 0;
    vosMemCtrlBlock->maxRealNumber     = 0;
#endif
}

/*
 * Description: Init all message control blocks
 */
VOS_UINT32 VOS_MemInit(VOS_VOID)
{
    VOS_INT      i;
    VOS_UINT32   totalSize     = 0;
    VOS_UINT32   totalCtrlSize = 0;
    VOS_VOID    *ctrlStart     = VOS_NULL_PTR;
    VOS_UINT_PTR spaceStart;
    VOS_UINT_PTR spaceEnd;
    VOS_UINT_PTR ctrlStartAddr;

#if (VOS_LINUX == VOS_OS_VER)
#ifdef CONFIG_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    VOS_INT32 regResult = 0;
#endif
#endif
#endif

    /* calculate msg+mem's size */
    for (i = 0; i < VOS_MEM_CTRL_BLOCK_NUMBER; i++) {
        if (0x00000003 & g_vosMsgBlockInfo[i].size) { /* 4 byte Aligned */
            mdrv_err("<VOS_MemInit> g_vosMsgBlockInfo=%d size=%d not aligned.\n", i, g_vosMsgBlockInfo[i].size);

            return VOS_ERR;
        }

        totalSize += (g_vosMsgBlockInfo[i].size + VOS_MEMORY_RESERVED_BYTES + VOS_MEM_HEAD_BLOCK_SIZE) *
                      g_vosMsgBlockInfo[i].number;
        totalCtrlSize += VOS_MEM_HEAD_BLOCK_SIZE * g_vosMsgBlockInfo[i].number;
    }

    for (i = 0; i < VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER; i++) {
        if (0x00000003 & g_vosMemBlockInfo[i].size) { /* 4 byte Aligned */
            mdrv_err("<VOS_MemInit> g_vosMemBlockInfo=%d size=%d not aligned.\n", i, g_vosMemBlockInfo[i].size);

            return VOS_ERR;
        }

        totalSize += (g_vosMemBlockInfo[i].size + VOS_MEMORY_RESERVED_BYTES + VOS_MEM_HEAD_BLOCK_SIZE) *
                      g_vosMemBlockInfo[i].number;
        totalCtrlSize += VOS_MEM_HEAD_BLOCK_SIZE * g_vosMemBlockInfo[i].number;
    }

    /* add protected space */
    totalSize += 2 * VOS_MEMORY_PROTECT_SIZE;
    totalCtrlSize += VOS_MEMORY_PROTECT_SIZE;

    if (totalSize > sizeof(g_vosMemBuf)) {
        mdrv_err("<VOS_MemInit> totalSize =%d totalCtrlSize =%d .\n", totalSize, totalCtrlSize);
        return VOS_ERR;
    }

    ctrlStart = (VOS_VOID *)g_vosMemBuf;

    ctrlStartAddr = (VOS_UINT_PTR)ctrlStart;

    g_vosMemCtrlSpaceStart = ctrlStartAddr;
    g_vosMemCtrlSpaceEnd   = ctrlStartAddr + sizeof(g_vosMemBuf);

    spaceStart     = ctrlStartAddr + totalCtrlSize;
    g_vosSpaceStart = spaceStart;

    spaceEnd     = ctrlStartAddr + totalSize - VOS_MEMORY_PROTECT_SIZE;
    g_vosSpaceEnd = spaceEnd;

    g_vosSpaceAndProtectionStart = g_vosSpaceStart - VOS_MEMORY_PROTECT_SIZE;

    g_vosSpaceAndProtectionEnd = g_vosSpaceEnd + VOS_MEMORY_PROTECT_SIZE;

    /* the start address of msg pool */
    g_memPoolIntervalAddress[0] = g_vosSpaceStart;

    for (i = 0; i < VOS_MEM_CTRL_BLOCK_NUMBER; i++) {
        VOS_MemCtrlBlkInit(&g_vosMemCtrlBlk[i], g_vosMsgBlockInfo[i].size, g_vosMsgBlockInfo[i].number, &ctrlStartAddr,
                           &spaceStart);
    }

    /* the start address of mem pool */
    g_memPoolIntervalAddress[1] = spaceStart;

    for (i = 0; i < VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER; i++) {
        VOS_MemCtrlBlkInit(&g_vosSimpleMemCtrlBlk[i], g_vosMemBlockInfo[i].size, g_vosMemBlockInfo[i].number, &ctrlStartAddr,
                           &spaceStart);
    }

    /* the start address of app mem pool */
    g_memPoolIntervalAddress[2] = spaceStart;

    VOS_SpinLockInit(&g_vosMemSpinLock);

    VOS_SpinLockInit(&g_vosStaticMemSpinLock);

    VOS_SpinLockInit(&g_vosDumpMemSpinLock);
    g_vosDumpMemFlag = VOS_TRUE;

#if (VOS_LINUX == VOS_OS_VER)
#ifdef CONFIG_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))

    regResult = platform_driver_register(&g_osaDriver);
    if (regResult) {
        mdrv_err("<VOS_MemInit> platform_driver_register fail, lRegResult=%d.\n", regResult);
    }

#endif
#endif
#endif

    return VOS_OK;
}

/*
 * Description: Check if other core is in mem dump process
 */
VOS_UINT32 VOS_MemDumpCheck(VOS_VOID)
{
    VOS_ULONG  lockLevel = 0;
    VOS_UINT32 dumpFlag;

    VOS_SpinLockIntLock(&g_vosDumpMemSpinLock, lockLevel);

    dumpFlag = g_vosDumpMemFlag;

    if (VOS_TRUE == g_vosDumpMemFlag) {
        g_vosDumpMemFlag = VOS_FALSE;
    }

    VOS_SpinUnlockIntUnlock(&g_vosDumpMemSpinLock, lockLevel);

    return dumpFlag;
}

VOS_VOID VOS_DumpVosMemFilterAddr(VOS_MemHeadBlock *memHead, VOS_UINT32 nums)
{
    VOS_UINT32 i;

    for (i = 0; i < nums; i++) {
        memHead[i].memCtrlAddress = VOS_NULL;
        memHead[i].memAddress = VOS_NULL;
        memHead[i].next = VOS_NULL_PTR;
#if VOS_MEMORY_CHECK == VOS_YES
        memHead[i].pre = VOS_NULL_PTR;
#endif
#if VOS_MEMORY_COUNT == VOS_YES
        memHead[i].realCtrlAddr = VOS_NULL;
#endif
    }

    return;
}

/*
 * Description: dump OSA Memory
 */
VOS_VOID VOS_DumpVosMem(VOS_MemHeadBlock *headBlock, VOS_UINT_PTR usrAddr, VOS_UINT32 errNo, VOS_UINT32 fileId,
    VOS_INT32 lineNo)
{
    VOS_MemHeadBlock *tmpMemHead = headBlock;
    VOS_MemHeadBlock memHead[VOS_DUMP_MEM_HEAD_TOTAL_NUM] = { { 0 } };

    /* 借用空间保存部分信息 */
    memHead[0].memCtrlAddress = (VOS_UINT_PTR)headBlock;
    memHead[0].memAddress = usrAddr;
    memHead[0].memUsedFlag = errNo;

    /* 保存出错的控制块前后个一个，总共3个控制块 */
    if (tmpMemHead != VOS_NULL_PTR) {
        tmpMemHead--;

        if ((VOS_UINT_PTR)tmpMemHead <= g_vosMemCtrlSpaceStart) {
            mdrv_wrn("<VOS_DumpVosMem> this block is first\n");
            tmpMemHead++;
        }

        if (memcpy_s((VOS_CHAR *)(&(memHead[1])), VOS_DUMP_MEM_HEAD_NUM * sizeof(VOS_MemHeadBlock),
            (VOS_CHAR *)tmpMemHead, VOS_DUMP_MEM_HEAD_NUM * sizeof(VOS_MemHeadBlock)) != EOK) {
            VOS_ProtectionReboot(VOS_REBOOT_MEMCPY_MEM, (VOS_INT)(THIS_FILE_ID), (VOS_INT)(__LINE__), 0, 0);
            return;
        }
    }

    VOS_DumpVosMemFilterAddr(memHead, VOS_DUMP_MEM_HEAD_TOTAL_NUM);
    VOS_ProtectionReboot(OSA_CHECK_MEM_ERROR, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)memHead, sizeof(memHead));

    return;
}

/*
 * Description: allocate a block
 */
VOS_VOID *VOS_MemCtrlBlkMalloc(VOS_MemCtrlBlock *vosMemCtrlBlock, VOS_UINT32 length, VOS_UINT32 fileId,
                               VOS_INT32 lineNo)
{
    VOS_ULONG           lockLevel;
    VOS_MemHeadBlock *block   = VOS_NULL_PTR;
    VOS_UINT_PTR       *temp = VOS_NULL_PTR;
    VOS_UINT_PTR        blockAddr;
    VOS_UINT32          crcTag;

    VOS_SpinLockIntLock(&g_vosMemSpinLock, lockLevel);

    if (0 == vosMemCtrlBlock->idleBlockNumber) {
        VOS_SpinUnlockIntUnlock(&g_vosMemSpinLock, lockLevel);

        return ((VOS_VOID *)VOS_NULL_PTR);
    } else {
        vosMemCtrlBlock->idleBlockNumber--;

        block = vosMemCtrlBlock->blocks;

        block->memUsedFlag = VOS_USED;

        vosMemCtrlBlock->blocks = block->next;
    }

#if VOS_YES == VOS_MEMORY_CHECK
    block->pre  = VOS_NULL_PTR;
    block->next = vosMemCtrlBlock->busyBlocks;
    if (VOS_NULL_PTR != vosMemCtrlBlock->busyBlocks) {
        vosMemCtrlBlock->busyBlocks->pre = block;
    }
    vosMemCtrlBlock->busyBlocks = block;
#endif

    VOS_SpinUnlockIntUnlock(&g_vosMemSpinLock, lockLevel);

#if VOS_YES == VOS_MEMORY_COUNT
    /* record the usage of control block */
    if (vosMemCtrlBlock->idleBlockNumber < vosMemCtrlBlock->minIdleBlockNumber) {
        vosMemCtrlBlock->minIdleBlockNumber = vosMemCtrlBlock->idleBlockNumber;
    }
#endif

    /* check memory */
    temp = (VOS_UINT_PTR *)(block->memAddress);

    blockAddr = *temp++;
    crcTag    = *temp;

    if (((VOS_UINT_PTR)block != blockAddr) || (VOS_MEMORY_CRC != crcTag)) {
        VOS_DumpVosMem(block, block->memAddress, VOS_ERRNO_MEMORY_ALLOC_CHECK, fileId, lineNo);

        return ((VOS_VOID *)VOS_NULL_PTR);
    }

    return (VOS_VOID *)block->memAddress;
}

/*
 * Description: allocate a timer message block
 */
VOS_VOID *VOS_TIMER_MemCtrlBlkMalloc(VOS_PID pid, VOS_UINT32 length)
{
    VOS_UINT_PTR       *space = VOS_NULL_PTR;
    VOS_MemHeadBlock *temp  = VOS_NULL_PTR;

    space = (VOS_UINT_PTR *)VOS_MemCtrlBlkMalloc(&g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1], length, 0, 0);
    if (VOS_NULL_PTR != space) {
#if VOS_YES == VOS_MEMORY_CHECK
        temp                 = (VOS_MemHeadBlock *)(*space);
        temp->ulcputickAlloc = VOS_GetSlice();
        temp->allocPid     = pid;
        temp->allocSize    = length;
#endif

#if VOS_YES == VOS_MEMORY_COUNT
        temp->realCtrlAddr = (VOS_UINT_PTR)(&g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1]);
        g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1].realNumber++;

        if (g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1].realNumber >
            g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1].maxRealNumber) {
            g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1].maxRealNumber =
                g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1].realNumber;
        }
#endif

        /* offset space which be reserved of OSA */
        space += VOS_MEMORY_RESERVED_WORD_HEAD;
        return (VOS_VOID *)space;
    }

    (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_MEMORY_FULL);

    mdrv_err("<VOS_TIMER_MemCtrlBlkMalloc> alloc timer msg fail.\n");

    VOS_ProtectionReboot(FAIL_TO_ALLOCATE_TIMER_MSG, 0, (VOS_INT)length, VOS_NULL_PTR, 0);

    return (VOS_NULL_PTR);
}

/*
 * Description: free a block
 */
VOS_UINT32 VOS_MemCtrlBlkFree(VOS_MemCtrlBlock *vosMemCtrlBlock, VOS_MemHeadBlock *block, VOS_UINT32 fileId,
                              VOS_PID pid)
{
    VOS_ULONG lockLevel;
#if VOS_YES == VOS_MEMORY_COUNT
    VOS_MemCtrlBlock *temp = VOS_NULL_PTR;
#endif

    VOS_SpinLockIntLock(&g_vosMemSpinLock, lockLevel);

    if (VOS_NOT_USED == block->memUsedFlag) {
        VOS_ProtectionReboot(VOS_FREE_MSG_AGAIN, (VOS_INT)fileId, (VOS_INT)pid, (VOS_CHAR *)block,
                             sizeof(VOS_MemHeadBlock));

        VOS_SpinUnlockIntUnlock(&g_vosMemSpinLock, lockLevel);

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_FREE_REPECTION);
        mdrv_err("<VOS_MemCtrlBlkFree> Free again.F=%d PID=%d.Tick=%d.\n", (VOS_INT)fileId, (VOS_INT)pid,
                 (VOS_INT)VOS_GetSlice());

        return VOS_ERR;
    }

#if VOS_YES == VOS_MEMORY_CHECK
    if (block == vosMemCtrlBlock->busyBlocks) {
        vosMemCtrlBlock->busyBlocks = block->next;
        if (VOS_NULL_PTR != vosMemCtrlBlock->busyBlocks) {
            vosMemCtrlBlock->busyBlocks->pre = VOS_NULL_PTR;
        }
    } else {
        block->pre->next = block->next;
        if (VOS_NULL_PTR != block->next) {
            (block->next)->pre = block->pre;
        }
    }

#if VOS_YES == VOS_ANALYZE_PID_MEM
    VOS_DecreasePidMemory(block->allocPid, block->allocSize, block->memRecord[3]);
#endif

    block->memRecord[6] = (fileId << 16) | ((VOS_UINT32)pid & 0x0000ffff);
    block->memRecord[7] = VOS_GetSlice();

#endif

    block->memUsedFlag = VOS_NOT_USED;

    block->next = vosMemCtrlBlock->blocks;

    vosMemCtrlBlock->blocks = block;

    vosMemCtrlBlock->idleBlockNumber++;

#if VOS_YES == VOS_MEMORY_COUNT
    /*lint -e613*/
    temp = (VOS_MemCtrlBlock *)(block->realCtrlAddr);
    /*lint +e613*/
    temp->realNumber--;
#endif

    VOS_SpinUnlockIntUnlock(&g_vosMemSpinLock, lockLevel);

    return VOS_OK;
}

/*
 * Description: allocate messagge memory
 */
VOS_VOID *VOS_MemBlkMalloc(VOS_PID pid, VOS_UINT32 length, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_INT             i;
    VOS_UINT_PTR       *space = VOS_NULL_PTR;
    VOS_MemHeadBlock *temp  = VOS_NULL_PTR;
#if VOS_YES == VOS_MEMORY_COUNT
    VOS_BOOL flag      = VOS_TRUE;
    VOS_INT  tempSuffix = 0;
#endif

#if (VOS_YES == VOS_CHECK_PARA)
    if (0 == length) {
        return (VOS_NULL_PTR);
    }
#endif

    for (i = 0; i < VOS_MEM_CTRL_BLOCK_NUMBER - 1; i++) {
        if (length <= g_vosMsgBlockInfo[i].size) {
#if VOS_YES == VOS_MEMORY_COUNT
            if (VOS_TRUE == flag) {
                flag      = VOS_FALSE;
                tempSuffix = i;
            }
#endif
            space = (VOS_UINT_PTR *)VOS_MemCtrlBlkMalloc(&g_vosMemCtrlBlk[i], length, fileId, lineNo);
            if (VOS_NULL_PTR != space) {
#if VOS_YES == VOS_MEMORY_CHECK
                temp                  = (VOS_MemHeadBlock *)(*space);
                temp->allocSize     = length;
                temp->memRecord[4] = (temp->memRecord[0] << 16) | (temp->allocPid & 0x0000ffff);
                temp->memRecord[5] = temp->ulcputickAlloc;
                temp->ulcputickAlloc  = VOS_GetSlice();
                temp->allocPid      = pid;
                temp->memRecord[0] = fileId;
                temp->memRecord[1] = (VOS_UINT32)lineNo;
#if VOS_YES == VOS_ANALYZE_PID_MEM
                temp->memRecord[3] = VOS_LOCATION_MSG;
                VOS_IncreasePidMemory(pid, length, VOS_LOCATION_MSG);
#endif

#endif

#if VOS_YES == VOS_MEMORY_COUNT
                temp->realCtrlAddr = (VOS_UINT_PTR)(&g_vosMemCtrlBlk[tempSuffix]);
                g_vosMemCtrlBlk[tempSuffix].realNumber++;

                if (g_vosMemCtrlBlk[tempSuffix].realNumber > g_vosMemCtrlBlk[tempSuffix].maxRealNumber) {
                    g_vosMemCtrlBlk[tempSuffix].maxRealNumber = g_vosMemCtrlBlk[tempSuffix].realNumber;
                }

                if (g_vosMemCtrlBlk[tempSuffix].maxSize < length) {
                    g_vosMemCtrlBlk[tempSuffix].maxSize = length;
                }

                if (g_vosMemCtrlBlk[tempSuffix].minSize > length) {
                    g_vosMemCtrlBlk[tempSuffix].minSize = length;
                }
#endif

                /* offset space which be reserved of OSA */
                space += VOS_MEMORY_RESERVED_WORD_HEAD;
                return (VOS_VOID *)space;
            }
        }
    }

    (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MSG_MEMORY_FULL);

    mdrv_err("<VOS_MemBlkMalloc> alloc msg fail size=%d. F=%d L=%d.\n", (VOS_INT)length, (VOS_INT)fileId, lineNo);

    VOS_MsgDump(pid, (VOS_UINT32)length, fileId, lineNo);

    return (VOS_NULL_PTR);
}

/*
 * Description: allocate memory
 */
MODULE_EXPORTED VOS_VOID *V_MemAlloc(VOS_UINT32 info, VOS_UINT8 ptNo, VOS_UINT32 size, VOS_UINT32 rebootFlag,
                                     VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    int                 i;
    VOS_UINT_PTR       *space   = VOS_NULL_PTR;
    VOS_MemHeadBlock *temp    = VOS_NULL_PTR;
    const VOS_MemBlockInfo *memInfo = VOS_NULL_PTR;
    VOS_MemCtrlBlock *memCtrl = VOS_NULL_PTR;
    VOS_INT             cycle;
#if VOS_YES == VOS_MEMORY_COUNT
    VOS_BOOL flag      = VOS_TRUE;
    VOS_INT  tempSuffix = 0;
#endif

#if (VOS_YES == VOS_CHECK_PARA)
    if (MEM_PT_BUTT <= ptNo) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_ALLOC_INPUTMSGISNULL);

        mdrv_err("<V_MemAlloc> partion invalid, F=%d L=%d P=%d.\n", fileId, lineNo, info);

        return VOS_NULL_PTR;
    }

    if (0 == size) {
        mdrv_err("<V_MemAlloc> size is 0,F=%d L=%d.\n", fileId, lineNo);

        return VOS_NULL_PTR;
    }

    /* 如果申请的空间大小超过0x7FFFFFFF个Byte，直接返回空指针 */
    if (0x7FFFFFFF < size) {
        mdrv_err("<V_MemAlloc> size over 0x7FFFFFFF, FileID=%d LineNo=%d.\n", fileId, lineNo);

        return VOS_NULL_PTR;
    }
#endif

    memInfo = g_vosMemBlockInfo;

    cycle = VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER;

    memCtrl = g_vosSimpleMemCtrlBlk;

    for (i = 0; i < cycle; i++) {
        if (size <= memInfo[i].size) {
#if VOS_YES == VOS_MEMORY_COUNT
            if (VOS_TRUE == flag) {
                flag      = VOS_FALSE;
                tempSuffix = i;
            }
#endif
            space = (VOS_UINT_PTR *)VOS_MemCtrlBlkMalloc(&(memCtrl[i]), size, fileId, lineNo);
            if (VOS_NULL_PTR != space) {
#if VOS_YES == VOS_MEMORY_CHECK
                temp                  = (VOS_MemHeadBlock *)(*space);
                temp->allocSize     = size;
                temp->memRecord[4] = (temp->memRecord[0] << 16) | (temp->allocPid & 0x0000ffff);
                temp->memRecord[5] = temp->ulcputickAlloc;
                temp->ulcputickAlloc  = VOS_GetSlice();
                temp->allocPid      = info;
                temp->memRecord[0] = fileId;
                temp->memRecord[1] = (VOS_UINT32)lineNo;
                /* record for check the leak of memory automatic */
                temp->memRecord[2] = (VOS_UINT32)ptNo;
#if VOS_YES == VOS_ANALYZE_PID_MEM
                temp->memRecord[3] = VOS_LOCATION_MEM;
                VOS_IncreasePidMemory(info, size, VOS_LOCATION_MEM);
#endif

#endif

#if VOS_YES == VOS_MEMORY_COUNT
                temp->realCtrlAddr = (VOS_UINT_PTR)(&(memCtrl[tempSuffix]));

                memCtrl[tempSuffix].realNumber++;

                if (memCtrl[tempSuffix].realNumber > memCtrl[tempSuffix].maxRealNumber) {
                    memCtrl[tempSuffix].maxRealNumber = memCtrl[tempSuffix].realNumber;
                }

                if (memCtrl[tempSuffix].maxSize < size) {
                    memCtrl[tempSuffix].maxSize = size;
                }

                if (memCtrl[tempSuffix].minSize > size) {
                    memCtrl[tempSuffix].minSize = size;
                }
#endif

                /* offset space which be reserved of OSA */
                space += VOS_MEMORY_RESERVED_WORD_HEAD;

#if (VOS_DEBUG == VOS_DOPRA_VER)

                if (VOS_NULL_PTR != g_funcAllocMemHook) {
                    g_funcAllocMemHook(size);
                }
#endif

                return (VOS_VOID *)space;
            }
        }
    }

    (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_FULL);

    mdrv_err("<V_MemAlloc> alloce memory fail size=%d. F=%d L=%d.\n", (VOS_INT)size, (VOS_INT)fileId, lineNo);

    if (VOS_FALSE == rebootFlag) {
        return VOS_NULL_PTR;
    }

    (VOS_VOID)VOS_MemDump(info, size, fileId, lineNo, memCtrl, cycle);

    return VOS_NULL_PTR;
}

/*
 * Description: calc mem's real size
 */
VOS_UINT32 VOS_CalcMem(const VOS_MemCtrlBlock *memCtrl, VOS_UINT32 *realSize, VOS_LocationMemInfo info)
{
    VOS_UINT_PTR  memAddress;
    VOS_INT       i;
    VOS_INT       suffix = 0x0fffffff;
    VOS_UINT32    quotient;
    VOS_UINT32    actualSize;
    VOS_UINT_PTR  startAddress;
    VOS_UINT_PTR  endAddress;
    VOS_UINT_PTR *pulstart = VOS_NULL_PTR;
    VOS_UINT_PTR *end   = VOS_NULL_PTR;

    memAddress = info.address;

    for (i = 0; i < info.cycle; i++) {
        if ((memAddress >= memCtrl[i].buffer) && (memAddress < memCtrl[i].bufferEnd)) {
            suffix = i;
            break;
        }
    }

    if (0x0fffffff == suffix) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_GLOBAL_COVER);

        mdrv_err("<VOS_CalcMem> MEM Global Cover: F=%d L=%d.\n", (VOS_INT)info.fileId, info.lineNo);

        return (VOS_ERRNO_MEMORY_GLOBAL_COVER);
    }

    actualSize = (VOS_UINT32)(VOS_MEMORY_RESERVED_BYTES + memCtrl[suffix].blockLength);

    quotient = (memAddress - memCtrl[suffix].buffer) / actualSize;
    /*lint -e647*/
    startAddress = memCtrl[suffix].buffer + (quotient * actualSize);
    /*lint +e647*/
    endAddress = startAddress + actualSize;

    pulstart = (VOS_UINT_PTR *)startAddress;
    end   = (VOS_UINT_PTR *)endAddress;

    pulstart++; /* offset Block */

    if (VOS_MEMORY_CRC != *pulstart) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_HEAD_COVER);

        mdrv_err("<VOS_CalcMem> MEM HEAD FAIL: F=%d L=%d.\n", (VOS_INT)info.fileId, info.lineNo);

        return (VOS_ERRNO_MEMORY_HEAD_COVER);
    }

    pulstart++; /* offset protection */

    end--; /* offset protection */

    if (VOS_MEMORY_CRC != *end) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_TAIL_COVER);

        mdrv_err("<VOS_CalcMem> MEM TAIL FAIL: F=%d L=%d.\n", (VOS_INT)info.fileId, info.lineNo);

        return (VOS_ERRNO_MEMORY_TAIL_COVER);
    }

    startAddress = (VOS_UINT_PTR)pulstart;

    if (VOS_LOCATION_MSG == info.type) {
        startAddress += VOS_MSG_BLK_HEAD_LEN;
    }

    endAddress = (VOS_UINT_PTR)end;

    if ((memAddress < startAddress) || (memAddress >= endAddress)) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_LOCATION_CANNOTDO);

        mdrv_err("<VOS_CalcMem> address out space: F=%d L=%d.\n", (VOS_INT)info.fileId, info.lineNo);

        return (VOS_ERRNO_MEMORY_LOCATION_CANNOTDO);
    }

    *realSize = (VOS_UINT32)(endAddress - memAddress);

    return VOS_OK;
}

/*
 * Description: location mem's head
 */
VOS_UINT32 VOS_LocationMem(const VOS_VOID *addr, VOS_UINT32 *realSize, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT_PTR               memAddress;
    VOS_LocationMemInfo info;

    memAddress = (VOS_UINT_PTR)addr;

    if ((memAddress < g_vosSpaceAndProtectionStart) || (memAddress > g_vosSpaceAndProtectionEnd)) {
        *realSize = 0xffffffff;
        return VOS_OK;
    }

    info.address = memAddress;
    info.fileId  = fileId;
    info.lineNo  = lineNo;

    /* MSG */
    if ((memAddress >= g_memPoolIntervalAddress[0]) && (memAddress < g_memPoolIntervalAddress[1])) {
        info.cycle = VOS_MEM_CTRL_BLOCK_NUMBER - 1;
        info.type = VOS_LOCATION_MSG;

        return VOS_CalcMem(g_vosMemCtrlBlk, realSize, info);
    }

    /* memory */
    if ((memAddress >= g_memPoolIntervalAddress[1]) && (memAddress < g_memPoolIntervalAddress[2])) {
        info.cycle = VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER;
        info.type = VOS_LOCATION_MEM;

        return VOS_CalcMem(g_vosSimpleMemCtrlBlk, realSize, info);
    }

    (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_LOCATION_INPUTINVALID);

    mdrv_err("<VOS_LocationMem> OSA MEM INPUTINVALID: F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

    return VOS_ERRNO_MEMORY_LOCATION_INPUTINVALID;
}

/*
 * Description: check memory is OSA's or not
 */
VOS_UINT32 VOS_MemCheck(const VOS_VOID *addr, VOS_UINT_PTR *block, VOS_UINT_PTR *ctrl, VOS_UINT32 fileId,
                        VOS_INT32 lineNo)
{
    VOS_UINT_PTR       *temp = VOS_NULL_PTR;
    VOS_UINT_PTR        headValue;
    VOS_UINT_PTR        tailValue;
    VOS_UINT_PTR        blockAddress;
    VOS_MemHeadBlock *headBlock = VOS_NULL_PTR;
    VOS_MemCtrlBlock *memCtrl   = VOS_NULL_PTR;
    VOS_UINT_PTR        tempAddress;
    VOS_UINT_PTR        userAddress;

    tempAddress = (VOS_UINT_PTR)addr;

#if (VOS_YES == VOS_CHECK_PARA)
    if (tempAddress < g_vosSpaceAndProtectionStart || tempAddress > g_vosSpaceAndProtectionEnd) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_NON_DOPRAMEM);

        mdrv_err("<VOS_MemCheck> NOT OSA MEM: F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        return (VOS_ERRNO_MEMORY_NON_DOPRAMEM);
    }

    if (tempAddress <= g_vosSpaceStart || tempAddress >= g_vosSpaceEnd) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_DOPRAMEM_OVERFLOW);

        mdrv_err("<VOS_MemCheck> OSA MEM OVERFLOW: F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        return (VOS_ERRNO_MEMORY_DOPRAMEM_OVERFLOW);
    }
#endif

    temp = (VOS_UINT_PTR *)addr;

    temp -= VOS_MEMORY_RESERVED_WORD_HEAD;

    userAddress = (VOS_UINT_PTR)temp;

    blockAddress = *temp;
    temp++;
    headValue = *temp;
    temp++;

    if (VOS_MEMORY_CRC != headValue) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_HEAD_COVER);

        VOS_DumpVosMem(VOS_NULL_PTR, userAddress, VOS_ERRNO_MEMORY_HEAD_COVER, fileId, lineNo);

        mdrv_err("<VOS_MemCheck> MEM HEAD FAIL: F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        return (VOS_ERRNO_MEMORY_HEAD_COVER);
    }

    headBlock = (VOS_MemHeadBlock *)blockAddress;
    if (headBlock->memAddress != userAddress) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_LINK_COVER);

        VOS_DumpVosMem(VOS_NULL_PTR, userAddress, VOS_ERRNO_MEMORY_LINK_COVER, fileId, lineNo);

        mdrv_err("<VOS_MemCheck> MEM LINK FAIL: F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        return (VOS_ERRNO_MEMORY_LINK_COVER);
    }

    *block = blockAddress;

    tempAddress = headBlock->memCtrlAddress;
    memCtrl    = (VOS_MemCtrlBlock *)tempAddress;

    *ctrl = tempAddress;

    temp = (VOS_UINT_PTR *)((VOS_UINT_PTR)temp + memCtrl->blockLength);

    tailValue = *temp;

    if (VOS_MEMORY_CRC != tailValue) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_TAIL_COVER);

        VOS_DumpVosMem(headBlock, userAddress, VOS_ERRNO_MEMORY_TAIL_COVER, fileId, lineNo);

        mdrv_err("<VOS_MemCheck> MEM TAIL FAIL: F=%d L=%d.\n", (VOS_INT)fileId, lineNo);

        return (VOS_ERRNO_MEMORY_TAIL_COVER);
    }

    return VOS_OK;
}

/*
 * Description: free memory
 */
MODULE_EXPORTED VOS_UINT32 V_MemFree(VOS_UINT32 info, VOS_VOID **ppAddr, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_UINT_PTR blockAdd;
    VOS_UINT_PTR ctrlkAdd;

#if (VOS_YES == VOS_CHECK_PARA)
    if (VOS_NULL_PTR == ppAddr) {
        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MEMORY_FREE_INPUTPIDINVALID);
    }

    if (VOS_NULL_PTR == *ppAddr) {
        mdrv_err("<V_MemFree> V_MemFree,free mem again.F=%d L=%d T=%d.\n", (VOS_INT)fileId, lineNo,
                 (VOS_INT)VOS_GetSlice());

        (VOS_VOID)VOS_SetErrorNo(VOS_ERRNO_MEMORY_FREE_INPUTPIDINVALID);
        return (VOS_ERRNO_MEMORY_FREE_INPUTPIDINVALID);
    }
#endif

    if (VOS_OK != VOS_MemCheck(*ppAddr, &blockAdd, &ctrlkAdd, fileId, lineNo)) {
        return VOS_ERR;
    }

    *ppAddr = VOS_NULL_PTR;

#if (VOS_DEBUG == VOS_DOPRA_VER)

    if (VOS_NULL_PTR != g_funcFreeMemHook) {
        g_funcFreeMemHook(0);
    }

#endif

    return VOS_MemCtrlBlkFree((VOS_MemCtrlBlock *)ctrlkAdd, (VOS_MemHeadBlock *)blockAdd, fileId,
                              (VOS_PID)info);
}

/*
 * Description: write memory info to flash
 */
VOS_VOID VOS_RefreshMemInfo(VOS_MemRecord *para, const VOS_MemCtrlBlock *memCtrl, VOS_INT cycle)
{
#if VOS_YES == VOS_MEMORY_COUNT
    VOS_INT i;

    for (i = 0; i < cycle; i++) {
        if (memCtrl[i].maxSize > para[i].maxSize) {
            para[i].maxSize = memCtrl[i].maxSize;
        }

        if (memCtrl[i].minSize < para[i].minSize) {
            para[i].minSize = memCtrl[i].minSize;
        }

        if (memCtrl[i].maxRealNumber > para[i].maxRealNumber) {
            para[i].maxRealNumber = memCtrl[i].maxRealNumber;
        }
    }
#endif

    return;
}

/*
 * Description: write memory info to flash
 */
VOS_VOID VOS_RecordmemInfo(VOS_MemRecord *para)
{
#if VOS_YES == VOS_MEMORY_COUNT
    VOS_MemRecord *temp = VOS_NULL_PTR;

    temp = para;

    VOS_RefreshMemInfo(temp, g_vosMemCtrlBlk, VOS_MEM_CTRL_BLOCK_NUMBER);

    temp = para + VOS_MEM_CTRL_BLOCK_NUMBER;

    VOS_RefreshMemInfo(temp, g_vosSimpleMemCtrlBlk, VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER);

#endif

    return;
}

#if VOS_YES == VOS_MEMORY_COUNT
/*
 * Description: print memory use info
 */
MODULE_EXPORTED VOS_VOID VOS_show_memory_info(VOS_VOID)
{
    int i;

    for (i = 0; i < VOS_MEM_CTRL_BLOCK_NUMBER; i++) {
        mdrv_debug("<VOS_show_memory_info> MSG %d Max is %d.\n", i, g_vosMemCtrlBlk[i].maxSize);
        mdrv_debug("<VOS_show_memory_info> MSG %d Min is %d.\n", i, g_vosMemCtrlBlk[i].minSize);
        mdrv_debug("<VOS_show_memory_info> MSG %d Real number is %d.\n", i, g_vosMemCtrlBlk[i].maxRealNumber);
        mdrv_debug("<VOS_show_memory_info> MSG %d Max use is %d.\r\n\n", i,
                   g_vosMemCtrlBlk[i].totalBlockNumber - g_vosMemCtrlBlk[i].minIdleBlockNumber);
    }

    for (i = 0; i < VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER; i++) {
        mdrv_debug("<VOS_show_memory_info> MEM %d Max is %d.\n", i, g_vosSimpleMemCtrlBlk[i].maxSize);
        mdrv_debug("<VOS_show_memory_info> MEM %d Min is %d.\n", i, g_vosSimpleMemCtrlBlk[i].minSize);
        mdrv_debug("<VOS_show_memory_info> MEM %d Real number is %d.\n", i, g_vosSimpleMemCtrlBlk[i].maxRealNumber);
        mdrv_debug("<VOS_show_memory_info> MEM %d Max use is %d.\r\n\n", i,
                   g_vosSimpleMemCtrlBlk[i].totalBlockNumber - g_vosSimpleMemCtrlBlk[i].minIdleBlockNumber);
    }
}
#endif

#if VOS_YES == VOS_MEMORY_CHECK

/*
 * Description: print used msg's useage info
 */
MODULE_EXPORTED VOS_VOID VOS_show_used_msg_info(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2, VOS_UINT32 para3)
{
    int                 i;
    VOS_MemHeadBlock *temp = VOS_NULL_PTR;

    for (i = VOS_MEM_CTRL_BLOCK_NUMBER - 1; i >= 0; i--) {
        mdrv_debug("<VOS_show_used_msg_info> messge Ctrl %d.\n", i);

        temp = g_vosMemCtrlBlk[i].busyBlocks;

        while (VOS_NULL_PTR != temp) {
            mdrv_debug("<VOS_show_used_msg_info>F=%d L=%d P=%d S=%d T=%u.\n", temp->memRecord[0],
                       temp->memRecord[1], temp->allocPid, temp->allocSize, temp->ulcputickAlloc);

            temp = temp->next;
        }
    }
}

/*
 * Description: print used timer msg's useage info
 */
MODULE_EXPORTED VOS_VOID VOS_show_used_timer_msg_info(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2,
                                                      VOS_UINT32 para3)
{
    VOS_MemHeadBlock *temp = VOS_NULL_PTR;

    mdrv_debug("<VOS_show_used_timer_msg_info> Timer messge used info.\n");

    temp = g_vosMemCtrlBlk[VOS_MEM_CTRL_BLOCK_NUMBER - 1].busyBlocks;

    while (VOS_NULL_PTR != temp) {
        mdrv_debug("<VOS_show_used_timer_msg_info> F=%d L=%d P=%d T=%u.\n", temp->memRecord[0],
                   temp->memRecord[1], temp->allocPid, temp->ulcputickAlloc);

        temp = temp->next;
    }
}

/*
 * Description: print used memory's useage info
 */
MODULE_EXPORTED VOS_VOID VOS_show_used_memory_info(VOS_UINT32 para0, VOS_UINT32 para1, VOS_UINT32 para2,
                                                   VOS_UINT32 para3)
{
    int                 i;
    VOS_MemHeadBlock *temp    = VOS_NULL_PTR;
    VOS_MemCtrlBlock *memCtrl = VOS_NULL_PTR;
    VOS_INT             cycle     = (VOS_INT)para1;
    VOS_UINT_PTR        tempValue  = (VOS_UINT_PTR)para0;

    memCtrl = (VOS_MemCtrlBlock *)tempValue;

    for (i = cycle - 1; i >= 0; i--) {
        mdrv_debug("<VOS_show_used_memory_info> memory Ctrl %d.\n", i);

        temp = memCtrl[i].busyBlocks;

        while (VOS_NULL_PTR != temp) {
            mdrv_debug("<VOS_show_used_memory_info> F=%d L=%d P=%d S=%d T=%u.\n", temp->memRecord[0],
                       temp->memRecord[1], temp->allocPid, temp->allocSize, temp->ulcputickAlloc);

            temp = temp->next;
        }
    }
}

/*
 * Description: print used MSG's useage info
 */
MODULE_EXPORTED VOS_VOID VOS_PrintUsedMsgInfo(VOS_VOID)
{
#if ((VOS_WIN32 == VOS_OS_VER) || (VOS_RTOSCK == VOS_OS_VER))
    VOS_UINT32 tempTaskID;
    VOS_UINT32 argument[4] = {0};

#endif

#if (VOS_RTOSCK == VOS_OS_VER)
    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_show_used_msg_info, 25, 32768, argument);

    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_SuspendAllTask, 26, 32768, argument);
#endif

#if (VOS_WIN32 == VOS_OS_VER)
    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_show_used_msg_info, 225, 32768, argument);

    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_SuspendAllTask, 224, 32768, argument);
#endif
}

/*
 * Description: print used MSG's useage info
 */
MODULE_EXPORTED VOS_VOID VOS_PrintUsedTimerMsgInfo(VOS_VOID)
{
#if ((VOS_WIN32 == VOS_OS_VER) || (VOS_RTOSCK == VOS_OS_VER))
    VOS_UINT32 tempTaskID;
    VOS_UINT32 argument[4] = {0};
#endif

#if (VOS_RTOSCK == VOS_OS_VER)
    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_show_used_timer_msg_info, 25, 32768, argument);

    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_SuspendAllTask, 26, 32768, argument);
#endif

#if (VOS_WIN32 == VOS_OS_VER)
    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_show_used_timer_msg_info, 225, 32768, argument);

    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_SuspendAllTask, 224, 32768, argument);
#endif
}

/*
 * Description: User suspend system
 */
VOS_VOID VOS_FatalError(VOS_VOID)
{
#if ((VOS_WIN32 == VOS_OS_VER) || (VOS_RTOSCK == VOS_OS_VER))
    VOS_UINT32 tempTaskID;
    VOS_UINT32 argument[4] = {0};
#endif

    mdrv_err("<VOS_FatalError> system halt.\n");

#if (VOS_RTOSCK == VOS_OS_VER)
    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_show_used_msg_info, 25, 32768, argument);

    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_SuspendAllTask, 26, 32768, argument);
#endif

#if (VOS_WIN32 == VOS_OS_VER)
    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_show_used_msg_info, 225, 32768, argument);

    (VOS_VOID)VOS_CreateTask(VOS_NULL_PTR, &tempTaskID, VOS_SuspendAllTask, 224, 32768, argument);
#endif
}

/*
 * Description: check the leak of memory
 */
VOS_VOID VOS_FindLeakMem(const VOS_MemCtrlBlock *memCtrl, VOS_INT cycle, VOS_UINT32 type)
{
    int                 i;
    VOS_MemHeadBlock *temp = VOS_NULL_PTR;

    for (i = cycle - 1; i >= 0; i--) {
        temp = memCtrl[i].busyBlocks;

        while (VOS_NULL_PTR != temp) {
            if (VOS_LOCATION_MEM == type) {
                if ((STATIC_DOPRA_MEM_PT == temp->memRecord[2]) || (STATIC_MEM_PT == temp->memRecord[2])) {
                    temp = temp->next;

                    continue;
                }
            }

            temp = temp->next;
        }
    }
}

/*
 * Description: check the leak of memory automatic
 */
MODULE_EXPORTED VOS_VOID VOS_AutoCheckMemory(VOS_VOID)
{
    VOS_ULONG  lockLevel;
    VOS_UINT32 tempTick;

    tempTick = VOS_GetSlice();

    mdrv_debug("<VOS_AutoCheckMemory> Auto Check Memory begin at T=%d.\n", tempTick);

    VOS_SpinLockIntLock(&g_vosMemSpinLock, lockLevel);

    VOS_FindLeakMem(g_vosMemCtrlBlk, VOS_MEM_CTRL_BLOCK_NUMBER, VOS_LOCATION_MSG);

    VOS_FindLeakMem(g_vosSimpleMemCtrlBlk, VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER, VOS_LOCATION_MEM);

    VOS_SpinUnlockIntUnlock(&g_vosMemSpinLock, lockLevel);

    mdrv_debug("<VOS_AutoCheckMemory> V_AutoCheckMemory end at T=%d.\n", VOS_GetSlice());

    return;
}

#endif

#if (VOS_DEBUG == VOS_DOPRA_VER)

/*
 * Description: register a hook to mem alloc
 */
VOS_UINT32 VOS_RegisterMemAllocHook(VOS_UINT32 mode, MEMORY_HOOK_FUNC pfnHook)
{
    if (VOS_ALLOC_MODE == mode) {
        g_funcAllocMemHook = pfnHook;

        return VOS_OK;
    } else if (VOS_FREE_MODE == mode) {
        g_funcFreeMemHook = pfnHook;

        return VOS_OK;
    } else {
        mdrv_err("<VOS_RegisterMemAllocHook> wrong mode .\n");

        return VOS_ERR;
    }
}

#endif

/*
 * Description: Record the info. of every memory block, before UE reboots.
 */
VOS_VOID VOS_MemDump(VOS_UINT32 info, VOS_UINT32 size, VOS_UINT32 fileId, VOS_INT32 lineNo,
                     const VOS_MemCtrlBlock *memCtrl, VOS_INT cycle)
{
    VOS_INT             i;
    VOS_UINT32          totalSize   = 0;
    VOS_UINT32         *dumpBuffer = VOS_NULL_PTR;
    VOS_MemHeadBlock *tmpMemHead = VOS_NULL_PTR;

    if (VOS_FALSE == VOS_MemDumpCheck()) {
        mdrv_debug("<VOS_MemDump> not need dump\n");

        return;
    }

    dumpBuffer = (VOS_UINT32 *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == dumpBuffer) {
        VOS_ProtectionReboot(FAIL_TO_ALLOCATE_MEM, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&size,
                             sizeof(size));

        return;
    }

    (VOS_VOID)VOS_TaskLock();

    if (memset_s(dumpBuffer, VOS_DUMP_MEM_TOTAL_SIZE, 0, VOS_DUMP_MEM_TOTAL_SIZE) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    *dumpBuffer++ = fileId;
    *dumpBuffer++ = (VOS_UINT32)lineNo;
    *dumpBuffer++ = info;
    *dumpBuffer++ = size;
    *dumpBuffer++ = VOS_GetSlice();

    totalSize += VOS_MEM_RECORD_BLOCK_SIZE;

    /* Get every memory block info. */
    for (i = cycle - 1; i >= 0; i--) {
        tmpMemHead = memCtrl[i].busyBlocks;

        while (VOS_NULL_PTR != tmpMemHead) {
            *dumpBuffer++ = tmpMemHead->memRecord[0];
            *dumpBuffer++ = tmpMemHead->memRecord[1];
            *dumpBuffer++ = tmpMemHead->allocPid;
            *dumpBuffer++ = tmpMemHead->allocSize;
            *dumpBuffer++ = tmpMemHead->ulcputickAlloc;

            totalSize += VOS_MEM_RECORD_BLOCK_SIZE;

            if ((totalSize + VOS_MEM_RECORD_BLOCK_SIZE) > VOS_DUMP_MEM_TOTAL_SIZE) {
                /* Jump from for loop */
                i = -1;
                break;
            }
            tmpMemHead = tmpMemHead->next;
        }
    }

    VOS_ProtectionReboot(FAIL_TO_ALLOCATE_MEM, (VOS_INT)fileId, (VOS_INT)lineNo, VOS_NULL_PTR, 0);

    (VOS_VOID)VOS_TaskUnlock();

    return;
}

/*
 * Description: Record the info. of every message block, before UE reboots.
 */
VOS_VOID VOS_MsgDump(VOS_UINT32 info, VOS_UINT32 size, VOS_UINT32 fileId, VOS_INT32 lineNo)
{
    VOS_INT             i;
    VOS_UINT32          totalSize   = 0;
    VOS_UINT32         *dumpBuffer = VOS_NULL_PTR;
    VOS_MemHeadBlock *tmpMemHead = VOS_NULL_PTR;
    VOS_UINT_PTR        address;

    if (VOS_FALSE == VOS_MemDumpCheck()) {
        mdrv_debug("<VOS_MsgDump> not need dump\n");

        return;
    }

    dumpBuffer = (VOS_UINT32 *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == dumpBuffer) {
        VOS_ProtectionReboot(FAIL_TO_ALLOCATE_MSG, (VOS_INT)fileId, (VOS_INT)lineNo, (VOS_CHAR *)&size,
                             sizeof(size));

        return;
    }

    (VOS_VOID)VOS_TaskLock();

    if (memset_s(dumpBuffer, VOS_DUMP_MEM_TOTAL_SIZE, 0, VOS_DUMP_MEM_TOTAL_SIZE) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    *(dumpBuffer++) = fileId;
    *(dumpBuffer++) = (VOS_UINT32)lineNo;
    *(dumpBuffer++) = info;
    *(dumpBuffer++) = size;
    *(dumpBuffer++) = VOS_GetSlice();

    totalSize += VOS_MEM_RECORD_BLOCK_SIZE;

    /* Get every message block info. */
    for (i = VOS_MEM_CTRL_BLOCK_NUMBER - 1; i >= 0; i--) {
        tmpMemHead = g_vosMemCtrlBlk[i].busyBlocks;

        while (VOS_NULL_PTR != tmpMemHead) {
            /* 省出一个阈来存放 cpu tick */
            *(dumpBuffer++) = (tmpMemHead->memRecord[0] << 16) | (tmpMemHead->memRecord[1] & 0x0000ffff);
            *(dumpBuffer++) = tmpMemHead->ulcputickAlloc;

            address = tmpMemHead->memAddress;
            address += VOS_MEMORY_RESERVED_WORD_HEAD * sizeof(VOS_UINT32);

            *(dumpBuffer++) = VOS_GetMsgName(address);
            *(dumpBuffer++) = tmpMemHead->allocSize;
            *(dumpBuffer++) = tmpMemHead->allocPid;

            totalSize += VOS_MEM_RECORD_BLOCK_SIZE;

            if ((totalSize + VOS_MEM_RECORD_BLOCK_SIZE) > VOS_DUMP_MEM_TOTAL_SIZE) {
                /* Jump from for loop */
                i = -1;
                break;
            }

            tmpMemHead = tmpMemHead->next;
        }
    }

    VOS_ProtectionReboot(FAIL_TO_ALLOCATE_MSG, (VOS_INT)fileId, (VOS_INT)lineNo, VOS_NULL_PTR, 0);

    (VOS_VOID)VOS_TaskUnlock();

    return;
}

/*
 * Description: get memory info
 */
MODULE_EXPORTED VOS_UINT32 VOS_GetFreeMemoryInfo(VOS_UINT32 size, VOS_UINT32 *freeBlockNum,
                                                 VOS_UINT32 *totalBlockNum)
{
    VOS_UINT32 i;
    VOS_UINT32 j;
    VOS_UINT32 tempFreeBlockNum  = 0;
    VOS_UINT32 temTotalBlockNum = 0;

    if (VOS_NULL_PTR == freeBlockNum) {
        return VOS_ERR;
    }

    if (VOS_NULL_PTR == totalBlockNum) {
        return VOS_ERR;
    }

    if (0 == size) {
        return VOS_ERR;
    }

    for (i = 0; i < VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER; i++) {
        if (size <= g_vosMemBlockInfo[i].size) {
            for (j = i; j < VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER; j++) {
                tempFreeBlockNum += g_vosSimpleMemCtrlBlk[j].idleBlockNumber;
                temTotalBlockNum += g_vosMemBlockInfo[j].number;
            }
            *freeBlockNum  = tempFreeBlockNum;
            *totalBlockNum = temTotalBlockNum;

            return VOS_OK;
        }
    }

    return VOS_ERR;
}

/*
 * Description: calc msg info
 */
VOS_BOOL VOS_CalcMsgInfo(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_UINT32 freeBlockNum  = 0;
    VOS_UINT32 totalBlockNum = 0;

    for (i = 0; i < VOS_MEM_CTRL_BLOCK_NUMBER; i++) {
        freeBlockNum += g_vosMemCtrlBlk[i].idleBlockNumber;
        totalBlockNum += g_vosMsgBlockInfo[i].number;
    }

    if (freeBlockNum < (totalBlockNum / VOS_CALC_MEM_RATIO)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: calc mem info
 */
VOS_BOOL VOS_CalcMemInfo(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_UINT32 freeBlockNum  = 0;
    VOS_UINT32 totalBlockNum = 0;

    for (i = 0; i < VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER; i++) {
        freeBlockNum  += g_vosSimpleMemCtrlBlk[i].idleBlockNumber;
        totalBlockNum += g_vosMemBlockInfo[i].number;
    }

    if (freeBlockNum < (totalBlockNum / VOS_CALC_MEM_RATIO)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

/*
 * Description: calc app memory info
 */
VOS_BOOL VOS_CalcAppMemInfo(VOS_VOID)
{
    return VOS_FALSE;
}

/*
 * Description: realloc memory
 */
VOS_UINT32 VOS_GetMemOccupyingInfo(VOS_MemOccupyingInfo *info, VOS_UINT32 len)
{
#if VOS_YES == VOS_MEMORY_CHECK
    VOS_INT             i;
    VOS_UINT32          suffix;
    VOS_MemHeadBlock *temp = VOS_NULL_PTR;

    if (memset_s(info, len, 0x00, len) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, (VOS_INT)len, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    (VOS_VOID)VOS_TaskLock();

    for (i = VOS_SIMPLE_MEM_CTRL_BLOCK_NUMBER - 1; i >= 0; i--) {
        temp = g_vosSimpleMemCtrlBlk[i].busyBlocks;

        while (VOS_NULL_PTR != temp) {
            if (VOS_PID_DOPRAEND > temp->allocPid) {
                (VOS_VOID)VOS_TaskUnlock();

                return VOS_ERR;
            }

            suffix = temp->allocPid - VOS_PID_DOPRAEND;
            info[suffix].totalNumber++;
            info[suffix].size += temp->allocSize;

            temp = temp->next;
        }
    }

    (VOS_VOID)VOS_TaskUnlock();

    return VOS_OK;
#else
    return VOS_ERR;
#endif
}

/*
 * Description: realloc memory
 */
VOS_UINT32 VOS_GetMsgOccupyingInfo(VOS_MemOccupyingInfo *info, VOS_UINT32 len)
{
#if VOS_YES == VOS_MEMORY_CHECK
    VOS_INT             i;
    VOS_UINT32          suffix;
    VOS_MemHeadBlock *temp = VOS_NULL_PTR;

    if (memset_s(info, len, 0x00, len) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, (VOS_INT)len, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }

    (VOS_VOID)VOS_TaskLock();

    for (i = VOS_MEM_CTRL_BLOCK_NUMBER - 1; i >= 0; i--) {
        temp = g_vosMemCtrlBlk[i].busyBlocks;

        while (VOS_NULL_PTR != temp) {
            if (VOS_PID_DOPRAEND > temp->allocPid) {
                (VOS_VOID)VOS_TaskUnlock();

                return VOS_ERR;
            }

            suffix = temp->allocPid - VOS_PID_DOPRAEND;
            info[suffix].totalNumber++;
            info[suffix].size += temp->allocSize;

            temp = temp->next;
        }
    }

    (VOS_VOID)VOS_TaskUnlock();

    return VOS_OK;
#else
    return VOS_ERR;
#endif
}

/*
 * Description: allocate static memory
 */
VOS_VOID *VOS_StaticMemAlloc(const VOS_CHAR *pcBuf, VOS_UINT32 bufSize, VOS_UINT32 allocSize, VOS_UINT32 *suffix)
{
    VOS_ULONG  lockLevel;
    VOS_UINT32 tempSize;
    VOS_UINT32 tempSuffix;

    if ((pcBuf == VOS_NULL_PTR) || (suffix == VOS_NULL_PTR)) {
        return VOS_NULL_PTR;
    }

    /* for ARM 4 byte aligned. Do it for peformence only */
    tempSize = (allocSize + VOS_ARM_ALIGNMENT) & (~VOS_ARM_ALIGNMENT);

    VOS_SpinLockIntLock(&g_vosStaticMemSpinLock, lockLevel);

    if ((*suffix + tempSize) <= bufSize) {
        tempSuffix = *suffix;
        *suffix += tempSize;

        VOS_SpinUnlockIntUnlock(&g_vosStaticMemSpinLock, lockLevel);

        return (VOS_VOID *)(&(pcBuf[tempSuffix]));
    }

    VOS_SpinUnlockIntUnlock(&g_vosStaticMemSpinLock, lockLevel);

    VOS_ProtectionReboot(VOS_FAIL_TO_ALLOC_STATIC_MEM, (VOS_INT)bufSize, 0, VOS_NULL_PTR, 0);

    return VOS_NULL_PTR;
}

/*
 * Description: modify the information which be used for debugging
 */
VOS_VOID VOS_ModifyMemBlkInfo(VOS_UINT_PTR addr, VOS_PID pid)
{
#if VOS_YES == VOS_MEMORY_CHECK
    VOS_MemHeadBlock *temp = VOS_NULL_PTR;

    temp = (VOS_MemHeadBlock *)(addr);

    temp->allocPid = pid;
#endif

    return;
}

/*
 * Description: allocate exc memory
 */
VOS_VOID *VOS_ExcDumpMemAlloc(VOS_UINT32 number)
{
    VOS_UINT8 *dumpBuffer = VOS_NULL_PTR;

    if (VOS_EXC_DUMP_MEM_NUM_BUTT <= number) {
        return VOS_NULL_PTR;
    }

    dumpBuffer = (VOS_UINT8 *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == dumpBuffer) {
        return VOS_NULL_PTR;
    }

    /* skip mem&msg dump */
    dumpBuffer += VOS_DUMP_MEM_TOTAL_SIZE;

    /*lint -e679*/
    dumpBuffer += (number * VOS_TASK_DUMP_INFO_SIZE);
    /*lint +e679*/

    return (VOS_VOID *)(dumpBuffer);
}

/*
 * Description: allocate cached memory.
 */
MODULE_EXPORTED VOS_VOID *VOS_CacheMemAllocDebug(VOS_UINT32 size, VOS_UINT32 cookie)
{
#if (VOS_RTOSCK == VOS_OS_VER)
    return SRE_MemCacheAlloc(size, MEM_ADDR_ALIGN_004);

#elif (VOS_LINUX == VOS_OS_VER)
    return kmalloc(size, GFP_KERNEL);
#else
    return malloc(size);
#endif
}

/*
 * Description: allocate unached memory.
 */
MODULE_EXPORTED VOS_UINT32 VOS_CacheMemFree(const VOS_VOID *addr)
{
#if (VOS_RTOSCK == VOS_OS_VER)
    return SRE_MemCacheFree((void *)addr);
#elif (VOS_LINUX == VOS_OS_VER)
    kfree(addr);
    return VOS_OK;
#else
    free(addr);
    return VOS_OK;
#endif
}

/*
 * Description: allocate uncached memory.
 */
MODULE_EXPORTED VOS_VOID *VOS_UnCacheMemAllocDebug(VOS_UINT32 size, VOS_UncacheMemAlign align,
                                                   VOS_UINT_PTR *realAddr, VOS_UINT32 cookie)
{
    VOS_VOID *virtAdd = VOS_NULL_PTR;

#if (VOS_LINUX == VOS_OS_VER)
    dma_addr_t address = 0;

#ifdef CONFIG_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#else
    struct device dev;
#endif
#endif

#endif

    if (0 == size) {
        return VOS_NULL_PTR;
    }

    if (VOS_NULL_PTR == realAddr) {
        return VOS_NULL_PTR;
    }

    /* keep lint happy */
    *realAddr = VOS_NULL;
    virtAdd     = VOS_NULL_PTR;

#if (VOS_RTOSCK == VOS_OS_VER)
    virtAdd = SRE_MemUncacheAlloc(size, (OS_MEM_ALIGN_E)align);

    *realAddr = (VOS_UINT_PTR)virtAdd;
#endif

#if (VOS_LINUX == VOS_OS_VER)

#ifdef CONFIG_ARM64

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    if (g_modemOsaPdev == VOS_NULL_PTR) {
        mdrv_err("<VOS_UnCacheMemAllocDebug> g_modemOsaPdev is NULL.");

        return VOS_NULL_PTR;
    }

    virtAdd = dma_alloc_coherent(&g_modemOsaPdev->dev, size, &address, GFP_KERNEL);
#else
    if (memset_s(&dev, sizeof(dev), 0, sizeof(dev)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    dma_set_mask_and_coherent(&dev, 0xffffffffffffffff);
    of_dma_configure(&dev, dev.of_node);
#endif
    virtAdd = dma_alloc_coherent(&dev, size, &address, GFP_KERNEL);
#endif

#else
    virtAdd = dma_alloc_coherent(VOS_NULL_PTR, size, &address, GFP_KERNEL);
#endif

    *realAddr = (VOS_UINT_PTR)address;
#endif

#if (VOS_WIN32 == VOS_OS_VER)
    virtAdd = (VOS_VOID *)malloc(size);

    *realAddr = (VOS_UINT_PTR)virtAdd;
#endif

    return virtAdd;
}

/*
 * Description: free uncached memory.
 */
MODULE_EXPORTED VOS_VOID VOS_UnCacheMemFree(VOS_VOID *virtAddr, VOS_VOID *phyAddr, VOS_UINT32 size)
{
#if (VOS_LINUX == VOS_OS_VER)
    dma_addr_t address;

#ifdef CONFIG_ARM64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#else
    struct device dev;
#endif
#endif

#endif

    if (0 == size) {
        return;
    }

    if ((VOS_NULL_PTR == virtAddr) || (VOS_NULL_PTR == phyAddr)) {
        return;
    }

#if (VOS_RTOSCK == VOS_OS_VER)
    (VOS_VOID)SRE_MemUncacheFree(virtAddr);
#endif

#if (VOS_LINUX == VOS_OS_VER)
    *(dma_addr_t *)(&address) = (dma_addr_t)phyAddr;

#ifdef CONFIG_ARM64

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    if (g_modemOsaPdev == VOS_NULL_PTR) {
        mdrv_err("<VOS_UnCacheMemFree> g_modemOsaPdev is NULL.");

        return;
    }

    dma_free_coherent(&g_modemOsaPdev->dev, size, virtAddr, address);
#else
    if (memset_s(&dev, sizeof(dev), 0, sizeof(dev)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    dma_set_mask_and_coherent(&dev, 0xffffffffffffffff);
    of_dma_configure(&dev, dev.of_node);
#endif
    dma_free_coherent(&dev, size, virtAddr, address);
#endif

#else
    dma_free_coherent(VOS_NULL_PTR, size, virtAddr, address);
#endif

#endif

#if (VOS_WIN32 == VOS_OS_VER)
    free(virtAddr);
#endif

    return;
}

/*
 * 功能描述: 根据输入的实地址，计算对应的虚地址
 */
VOS_UINT_PTR VOS_UncacheMemPhyToVirt(const VOS_UINT8 *curPhyAddr, const VOS_UINT8 *phyStart, const VOS_UINT8 *virtStart,
                                     VOS_UINT32 bufLen)
{
#if (VOS_LINUX == VOS_OS_VER)
    if ((curPhyAddr < phyStart) || (curPhyAddr >= (phyStart + bufLen))) {
        mdrv_err("<VOS_UncacheMemPhyToVirt> parameters not right.\n");

        return VOS_NULL;
    }

    return (VOS_UINT_PTR)((curPhyAddr - phyStart) + virtStart);
#else
    return (VOS_UINT_PTR)(curPhyAddr);
#endif
}

/*
 * 功能描述: 根据输入的虚地址，计算对应的实地址
 */
VOS_UINT_PTR VOS_UncacheMemVirtToPhy(const VOS_UINT8 *curVirtAddr, const VOS_UINT8 *phyStart, const VOS_UINT8 *virtStart,
                                     VOS_UINT32 bufLen)
{
#if (VOS_LINUX == VOS_OS_VER)
    if ((curVirtAddr < virtStart) || (curVirtAddr >= (virtStart + bufLen))) {
        mdrv_err("<VOS_UncacheMemVirtToPhy> parameters not right.\r\n");

        return VOS_NULL;
    }

    return (VOS_UINT_PTR)((curVirtAddr - virtStart) + phyStart);
#else
    return (VOS_UINT_PTR)(curVirtAddr);
#endif
}

/*
 * 功能描述: 刷CPU Write buffer
 */
/*lint -e522*/
MODULE_EXPORTED VOS_VOID VOS_FlushCpuWriteBuf(VOS_VOID)
{
#if (VOS_WIN32 != VOS_OS_VER)

#if (VOS_LINUX == VOS_OS_VER)
    __asm(" DSB sy ");
    __asm(" ISB sy ");
#else
    __asm__ __volatile__(" DSB ");
#if (VOS_RTOSCK == VOS_OS_VER)
    SRE_L2CacheWait();
#endif
    __asm__ __volatile__(" ISB ");
#endif

#endif
    return;
}
/*lint +e522*/

/*
 * 功能描述: 刷CPU Cache
 */
MODULE_EXPORTED VOS_VOID VOS_FlushCpuCache(VOS_VOID *address, VOS_UINT size)
{
#if (VOS_RTOSCK == VOS_OS_VER)
    (VOS_VOID)SRE_DCacheFlush(address, size, VOS_TRUE);
#elif (VOS_LINUX == VOS_OS_VER)
#elif (VOS_WIN32 == VOS_OS_VER)
    ;
#endif
}

/*
 * 功能描述: 获取内存临街资源保护的自旋锁
 */
VOS_SPINLOCK *VOS_GetStaticMemSpinLock(VOS_VOID)
{
    return &g_vosStaticMemSpinLock;
}

