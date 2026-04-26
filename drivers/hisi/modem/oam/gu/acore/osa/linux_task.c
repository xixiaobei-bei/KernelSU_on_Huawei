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
 * 功能描述: OSA linux操作系统上任务功能实现
 * 生成日期: 2006年10月3日
 */

#include "vos_config.h"
#include "v_typdef.h"
#include "v_task.h"
#include "v_io.h"
#include "v_int.h"
#include "vos_id.h"
#include "v_sem.h"
#include "v_private.h"
#include "mdrv.h"
#include "pam_tag.h"

#if (VOS_OS_VER != VOS_WIN32)
#include <securec.h>
#endif

#if (VOS_LINUX == VOS_OS_VER)
#include <linux/version.h>
#include <linux/kthread.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/types.h>
#include <linux/sched/debug.h>
#include <linux/sched/signal.h>
#endif
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "ut_mem.h"
#endif


#if (VOS_LINUX == VOS_OS_VER) || defined(LLT_OS_LINUX)

#define THIS_FILE_ID PS_FILE_ID_V_TASK_C
#define THIS_MODU mod_pam_osa

#define VOS_TASK_DELAY_MAX_LEN 16777216 /* unit is ms 2^24 */

typedef struct {
    int                 flag;
    int                 tid;      /* task ID */
    VOS_TaskEntryType function; /* the entry of task */
    VOS_UINT32          priority;
    VOS_UINT32          stackSize;
    VOS_UINT32          args[VOS_TARG_NUM]; /* the argument of the entry */
    VOS_CHAR            name[VOS_MAX_LENGTH_TASK_NAME];
    VOS_UINT32          eventIsCreate; /* event create or not */
    VOS_UINT32          events;        /* current event */
    VOS_UINT32          expects;       /* expected event */
    VOS_UINT32          receives;      /* have been received event */
    VOS_UINT32          flags;         /* event mode */
    VOS_UINT32          fid;           /* belong to a FID or not */
    pid_t               linuxThreadId; /* maped Linux thread id */
    VOS_UINT8           rsv[4];
} VOS_TaskControlBlock;

/* the number of task's control block */
VOS_UINT32 g_vosTaskCtrlBlkNumber;

/* the start address of task's control block */
VOS_TaskControlBlock *g_vosTaskCtrlBlk = VOS_NULL_PTR;

#define VOS_TASK_CTRL_BUF_SIZE (sizeof(VOS_TaskControlBlock) * VOS_MAX_TASK_NUMBER)

VOS_UINT32 g_vosTaskCtrlBuf[VOS_TASK_CTRL_BUF_SIZE / 4];

typedef VOS_INT (*LINUX_START_ROUTINE)(VOS_VOID *);

/* 自旋锁，用来作task的临界资源保护 */
VOS_SPINLOCK g_vosTaskSpinLock;

VOS_INT VOS_LinuxTaskEntry(VOS_VOID *arg);

/*
 * Description: record debug info
 */
VOS_VOID VOS_TaskPrintCtrlBlkInfo(VOS_VOID)
{
    VOS_CHAR *pcBuff = VOS_NULL_PTR;

    pcBuff = (VOS_CHAR *)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == pcBuff) {
        return;
    }

    if (memcpy_s(pcBuff, VOS_DUMP_MEM_ALL_SIZE, (VOS_CHAR *)g_vosTaskCtrlBuf, VOS_TASK_CTRL_BUF_SIZE) != EOK) {
        mdrv_err("<VOS_TaskPrintCtrlBlkInfo> memcpy_s error.\n");
    }

    return;
}

/*
 * Description: Init task's control block
 */
VOS_VOID VOS_TaskCtrlBlkInit(VOS_VOID)
{
    int i;

    g_vosTaskCtrlBlkNumber = VOS_MAX_TASK_NUMBER;

    g_vosTaskCtrlBlk = (VOS_TaskControlBlock *)g_vosTaskCtrlBuf;

    for (i = 0; i < (int)g_vosTaskCtrlBlkNumber; i++) {
        g_vosTaskCtrlBlk[i].flag            = VOS_TASK_CTRL_BLK_IDLE;
        g_vosTaskCtrlBlk[i].tid             = i;
        g_vosTaskCtrlBlk[i].linuxThreadId = 0;
        g_vosTaskCtrlBlk[i].eventIsCreate = VOS_FALSE;
        g_vosTaskCtrlBlk[i].events        = 0;
        g_vosTaskCtrlBlk[i].expects       = 0;
        g_vosTaskCtrlBlk[i].receives      = 0;
        g_vosTaskCtrlBlk[i].flags         = VOS_EVENT_NOWAIT;
        g_vosTaskCtrlBlk[i].fid           = 0xffffffff;
    }

    VOS_SpinLockInit(&g_vosTaskSpinLock);

    return;
}

/*
 * Description: allocate a block
 */
VOS_UINT32 VOS_TaskCtrlBlkGet(VOS_VOID)
{
    VOS_UINT32 i;
    VOS_ULONG  lockLevel;

    VOS_SpinLockIntLock(&g_vosTaskSpinLock, lockLevel);

    for (i = 0; i < g_vosTaskCtrlBlkNumber; i++) {
        if (g_vosTaskCtrlBlk[i].flag == VOS_TASK_CTRL_BLK_IDLE) {
            g_vosTaskCtrlBlk[i].flag = VOS_TASK_CTRL_BLK_BUSY;

            break;
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosTaskSpinLock, lockLevel);

    if (i < g_vosTaskCtrlBlkNumber) {
        return i;
    } else {
        mdrv_err("<VOS_TaskCtrlBlkGet> allocate task control block fail.\n");

        return (VOS_TASK_CTRL_BLK_NULL);
    }
}

/*
 * Description: free a block
 */
VOS_UINT32 VOS_TaskCtrlBlkFree(VOS_UINT32 tid)
{
    VOS_ULONG lockLevel;

    if (tid < g_vosTaskCtrlBlkNumber) {
        if (g_vosTaskCtrlBlk[tid].flag == VOS_TASK_CTRL_BLK_IDLE) {
            mdrv_err("<VOS_TaskCtrlBlkFree> free Idle Task.\n");

            return VOS_ERR;
        } else {
            VOS_SpinLockIntLock(&g_vosTaskSpinLock, lockLevel);

            g_vosTaskCtrlBlk[tid].flag = VOS_TASK_CTRL_BLK_IDLE;

            VOS_SpinUnlockIntUnlock(&g_vosTaskSpinLock, lockLevel);

            return VOS_OK;
        }
    } else {
        mdrv_err("<VOS_TaskCtrlBlkFree> Tid exceed TaskCtrlBlkNumber.\n");

        return VOS_ERR;
    }
}

/*
 * Description: linux task's entry
 */
VOS_INT VOS_LinuxTaskEntry(VOS_VOID *arg)
{
    VOS_TaskControlBlock *temp = VOS_NULL_PTR;
    VOS_UINT32              para1;
    VOS_UINT32              para2;
    VOS_UINT32              para3;
    VOS_UINT32              para4;

    temp = (VOS_TaskControlBlock *)arg;

    para1 = temp->args[0];
    para2 = temp->args[1];
    para3 = temp->args[2];
    para4 = temp->args[3];

    temp->function(para1, para2, para3, para4);

    return 0;
}

/*
 * Description: create task with default task mode: not be added in running list
 */
VOS_UINT32 VOS_CreateTaskOnly(const VOS_CHAR *puchName, VOS_UINT32 *taskID, VOS_TaskEntryType pfnFunc,
                              VOS_UINT32 priority, VOS_UINT32 stackSize, const VOS_UINT32 aulArgs[VOS_TARG_NUM])
{
    int                 i;
    VOS_UINT32          iTid;
    VOS_CHAR            defaultName[8] = {0};
    VOS_UCHAR           value;
    VOS_INT_PTR         funcAddr;
    struct task_struct *tsk = VOS_NULL_PTR;
    struct sched_param  param;

    if (taskID == VOS_NULL_PTR) {
        VOS_SetErrorNo(VOS_ERRNO_TASK_CREATE_INPUTTIDISNULL);
        return( VOS_ERRNO_TASK_CREATE_INPUTTIDISNULL );
    }

    if (pfnFunc == VOS_NULL_PTR) {
        VOS_SetErrorNo(VOS_ERRNO_TASK_CREATE_INPUTENTRYISNULL);
        return( VOS_ERRNO_TASK_CREATE_INPUTENTRYISNULL );
    }

    iTid = VOS_TaskCtrlBlkGet();
    if (iTid == (VOS_UINT32)VOS_TASK_CTRL_BLK_NULL) {
        VOS_TaskPrintCtrlBlkInfo();

        funcAddr = (VOS_INT_PTR)pfnFunc;

        VOS_ProtectionReboot(OSA_ALLOC_TASK_CONTROL_ERROR, (VOS_INT)g_vosTaskCtrlBlkNumber, (VOS_INT)funcAddr,
                             VOS_NULL_PTR, 0);

        VOS_SetErrorNo(VOS_ERRNO_TASK_CREATE_NOFREETCB);
        return (VOS_ERRNO_TASK_CREATE_NOFREETCB);
    }

    /* caller not use name asign a default name */
    if (VOS_NULL_PTR == puchName) {
        defaultName[0] = 't';
        defaultName[1] = 'i';
        defaultName[2] = 'd';
        defaultName[3] = (VOS_CHAR)(48 + iTid / 100);
        value           = (VOS_UCHAR)(iTid % 100);
        defaultName[4] = (VOS_CHAR)(48 + value / 10);
        defaultName[5] = (VOS_CHAR)(48 + value % 10);
        defaultName[6] = '\0';
    }

    *taskID = iTid;

    if (VOS_NULL_PTR != puchName) {
        if (memcpy_s(g_vosTaskCtrlBlk[iTid].name, VOS_MAX_LENGTH_TASK_NAME, puchName,
                     VOS_StrNLen(puchName, VOS_MAX_LENGTH_TASK_NAME)) != EOK) {
            mdrv_om_system_error(VOS_REBOOT_MEMCPY_MEM, (VOS_INT)VOS_StrNLen(puchName, VOS_MAX_LENGTH_TASK_NAME),
                                 (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
        }

        g_vosTaskCtrlBlk[iTid].name[VOS_MAX_LENGTH_TASK_NAME - 1] = '\0';
    } else {
        g_vosTaskCtrlBlk[iTid].name[0] = defaultName[0];
        g_vosTaskCtrlBlk[iTid].name[1] = defaultName[1];
        g_vosTaskCtrlBlk[iTid].name[2] = defaultName[2];
        g_vosTaskCtrlBlk[iTid].name[3] = defaultName[3];
        g_vosTaskCtrlBlk[iTid].name[4] = defaultName[4];
        g_vosTaskCtrlBlk[iTid].name[5] = defaultName[5];
        g_vosTaskCtrlBlk[iTid].name[6] = defaultName[6];
    }

    g_vosTaskCtrlBlk[iTid].function  = pfnFunc;
    g_vosTaskCtrlBlk[iTid].priority  = priority;
    g_vosTaskCtrlBlk[iTid].stackSize = stackSize;

    for (i = 0; i < VOS_TARG_NUM; i++) {
        g_vosTaskCtrlBlk[iTid].args[i] = aulArgs[i];
    }

    tsk = kthread_create((LINUX_START_ROUTINE)VOS_LinuxTaskEntry, (VOS_VOID *)&g_vosTaskCtrlBlk[iTid],
                         g_vosTaskCtrlBlk[iTid].name);
    if (tsk == VOS_NULL) {
        VOS_TaskPrintCtrlBlkInfo();

        VOS_ProtectionReboot(OSA_CREATE_TASK_ERROR, (VOS_INT)(__LINE__), (VOS_INT)iTid, VOS_NULL_PTR, 0);

        VOS_TaskCtrlBlkFree(iTid);
        VOS_SetErrorNo(VOS_ERRNO_TASK_CREATE_OSALCREATEFAIL);
        return (VOS_ERRNO_TASK_CREATE_OSALCREATEFAIL);
    }

    g_vosTaskCtrlBlk[iTid].linuxThreadId = tsk->pid;

    param.sched_priority = (VOS_INT)priority;

    if (VOS_NULL != sched_setscheduler(tsk, SCHED_FIFO, &param)) {
        mdrv_err("<VOS_CreateTaskOnly> Creat sched_setscheduler fail, Task=%s, ID=%d \n", puchName, iTid);

        VOS_TaskPrintCtrlBlkInfo();

        VOS_ProtectionReboot(OSA_SET_TASK_PRI_ERROR, (VOS_INT)priority, (VOS_INT)iTid, (VOS_CHAR *)tsk,
                             (VOS_INT)sizeof(struct task_struct));

        return VOS_ERR;
    }

    return VOS_OK;
}

/*
 * Description: create task with default task mode:
 */
VOS_UINT32 VOS_CreateTask(const VOS_CHAR *puchName, VOS_UINT32 *taskID, VOS_TaskEntryType pfnFunc,
    VOS_UINT32 priority, VOS_UINT32 stackSize, const VOS_UINT32 aulArgs[VOS_TARG_NUM])
{
    struct task_struct *tsk = VOS_NULL_PTR;
    pid_t               linuxThreadId;
    VOS_UINT32          result;

    if (taskID == VOS_NULL_PTR) {
        VOS_SetErrorNo(VOS_ERRNO_TASK_CREATE_INPUTTIDISNULL);
        return( VOS_ERRNO_TASK_CREATE_INPUTTIDISNULL );
    }

    if (pfnFunc == VOS_NULL_PTR) {
        VOS_SetErrorNo(VOS_ERRNO_TASK_CREATE_INPUTENTRYISNULL);
        return( VOS_ERRNO_TASK_CREATE_INPUTENTRYISNULL );
    }

    result = VOS_CreateTaskOnly(puchName, taskID, pfnFunc, priority, stackSize, aulArgs);
    if (VOS_OK != result) {
        mdrv_err("<VOS_CreateTask> createTaskOnly fail\n");
        return VOS_ERR;
    }

    linuxThreadId = g_vosTaskCtrlBlk[*taskID].linuxThreadId;

    tsk = pid_task(find_vpid(linuxThreadId), PIDTYPE_PID);
    if (VOS_NULL_PTR == tsk) {
        mdrv_err("<VOS_CreateTask> tsk NULL\n");
        return VOS_ERR;
    }

    if (tsk->pid != linuxThreadId) {
        mdrv_err("<VOS_CreateTask> tsk pid not equal\n");
        return VOS_ERR;
    }

    (VOS_VOID)wake_up_process(tsk);

    return VOS_OK;
}

/*
 * Description: Resume a task
 */
VOS_UINT32 VOS_ResumeTask(VOS_UINT32 taskID)
{
    pid_t               temp;
    struct task_struct *tsk = VOS_NULL_PTR;

    if (taskID >= g_vosTaskCtrlBlkNumber) {
        return (VOS_ERR);
    }

    temp = g_vosTaskCtrlBlk[taskID].linuxThreadId;

    tsk = pid_task(find_vpid(temp), PIDTYPE_PID);
    if (VOS_NULL_PTR == tsk) {
        return (VOS_ERR);
    }

    if (tsk->pid != temp) {
        printk("resume find task to set pri fail.\r\n");
        return VOS_ERR;
    }

    wake_up_process(tsk);

    return VOS_OK;
}

/*
 * Description: Let the task sleep specified millseconds, only tick precision
 */
VOS_UINT32 VOS_TaskDelay(VOS_UINT32 millSecs)
{
    if (millSecs > VOS_TASK_DELAY_MAX_LEN) {
        return VOS_ERR;
    }

    /*lint -e446 -e666*/
    set_current_state(TASK_UNINTERRUPTIBLE);
    /*lint +e446 +e666*/

    /*lint -e647*/
    (VOS_VOID)schedule_timeout((((millSecs)*HZ) / 1000) + 1);
    /*lint +e647*/
    return VOS_OK;
}

/*
 * Description: forbid task attemper.
 */
VOS_UINT32 VOS_TaskLock(VOS_VOID)
{
    preempt_disable();

    return VOS_OK;
}

/*
 * Description: permit task attemper.
 */
VOS_UINT32 VOS_TaskUnlock(VOS_VOID)
{
    preempt_enable();

    return VOS_OK;
}

/*
 * Description: Delete a task, on some platform this operation is prohibited.
 */
VOS_UINT32 VOS_DeleteTask(VOS_UINT32 taskID)
{
    mdrv_debug("<VOS_DeleteTask>: Delete TaskId=%x", taskID);

    return (VOS_OK);
}

/*
 * Description: Get current task's ID
 */
VOS_UINT32 VOS_GetCurrentTaskID(VOS_VOID)
{
    pid_t     osid;
    int       i;
    VOS_ULONG lockLevel;

    osid = current->pid;

    VOS_SpinLockIntLock(&g_vosTaskSpinLock, lockLevel);

    for (i = 0; i < (VOS_INT)g_vosTaskCtrlBlkNumber; i++) {
        if (VOS_TASK_CTRL_BLK_BUSY == g_vosTaskCtrlBlk[i].flag) {
            if (osid == g_vosTaskCtrlBlk[i].linuxThreadId) {
                VOS_SpinUnlockIntUnlock(&g_vosTaskSpinLock, lockLevel);

                return (VOS_UINT32)i;
            }
        }
    }

    VOS_SpinUnlockIntUnlock(&g_vosTaskSpinLock, lockLevel);

    mdrv_err("<VOS_GetCurrentTaskID> Current Task ID not found.\n");

    return (VOS_NULL_LONG);
}

/*
 * Description:Create resource for the Event.
 */
VOS_UINT32 VOS_CreateEvent(VOS_UINT32 taskID)
{
    if (taskID >= g_vosTaskCtrlBlkNumber) {
        return VOS_ERR;
    }

    if (VOS_TASK_CTRL_BLK_IDLE == g_vosTaskCtrlBlk[taskID].flag) {
        return VOS_ERR;
    }

    g_vosTaskCtrlBlk[taskID].eventIsCreate = VOS_TRUE;
    g_vosTaskCtrlBlk[taskID].events        = 0;
    g_vosTaskCtrlBlk[taskID].expects       = 0;
    g_vosTaskCtrlBlk[taskID].receives      = 0;
    g_vosTaskCtrlBlk[taskID].flags         = VOS_EVENT_NOWAIT;

    return VOS_OK;
}

/*
 * Description: checke a event exists or not
 */
VOS_UINT32 VOS_CheckEvent(VOS_UINT32 taskID)
{
#if (VOS_YES == VOS_CHECK_PARA)
    if (taskID >= g_vosTaskCtrlBlkNumber) {
        return VOS_ERR;
    }

    if (VOS_TASK_CTRL_BLK_IDLE == g_vosTaskCtrlBlk[taskID].flag) {
        return VOS_ERR;
    }

    if (VOS_FALSE == g_vosTaskCtrlBlk[taskID].eventIsCreate) {
        return VOS_ERR;
    }

    if (0xffffffff == g_vosTaskCtrlBlk[taskID].fid) {
        return VOS_ERR;
    }
#endif

    return VOS_OK;
}

/*
 * Description: record corresponding of FID&TID
 */
VOS_UINT32 VOS_RecordFIDTIDInfo(VOS_UINT32 fid, VOS_UINT32 tid)
{
    g_vosTaskCtrlBlk[tid].fid = fid;

    return VOS_OK;
}

/*
 * Description:Send Event to the TaskID.
 */
VOS_UINT32 VOS_EventWrite(VOS_UINT32 taskID, VOS_UINT32 events)
{
    /* the functin should be full later */
    VOS_ULONG  lockLevel;
    VOS_UINT32 tempQueue;
    VOS_SEM    tempSem;

    if (VOS_OK != VOS_CheckEvent(taskID)) {
        mdrv_err("<VOS_EventWrite> EVENT not exist.\n");
        return VOS_ERR;
    }

    VOS_SpinLockIntLock(&g_vosTaskSpinLock, lockLevel);

    g_vosTaskCtrlBlk[taskID].events |= events;

    VOS_SpinUnlockIntUnlock(&g_vosTaskSpinLock, lockLevel);

    tempQueue = VOS_GetQueueIDFromFid(g_vosTaskCtrlBlk[taskID].fid);
#if (VOS_YES == VOS_CHECK_PARA)
    if (0xffffffff == tempQueue) {
        return VOS_ERR;
    }
#endif

    tempSem = VOS_GetSemIDFromQueue(tempQueue);
#if (VOS_YES == VOS_CHECK_PARA)
    if (0xffffffff == tempSem) {
        return VOS_ERR;
    }
#endif

    if (VOS_OK != VOS_SmV(tempSem)) {
        mdrv_err("<VOS_EventWrite> Vos_SmV Fail.\n");

        return VOS_ERR;
    } else {
        return VOS_OK;
    }
}

/*
 * Description:Read event
 */
VOS_UINT32 VOS_EventRead(VOS_UINT32 events, VOS_UINT32 flags, VOS_UINT32 timeOutInMillSec,
                         VOS_UINT32 *retEvents)
{
    /* the functin should be full later */
    VOS_UINT32 taskSelf; /* Self task ID */
    VOS_UINT32 tempQueue;
    VOS_SEM    tempSem;
    VOS_ULONG  lockLevel;
    VOS_UINT32 tempEvent;

    if (retEvents == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    taskSelf = VOS_GetCurrentTaskID();
#if (VOS_YES == VOS_CHECK_PARA)
    if (0xffffffff == taskSelf) {
        return VOS_ERR;
    }

    if (VOS_EVENT_NOWAIT & flags) {
        mdrv_err("<VOS_EventRead> OSA don't support VOS_EVENT_NOWAIT.\n");

        return VOS_ERR;
    }
#endif

    if (!(VOS_EVENT_ANY & flags)) {
        mdrv_err("<VOS_EventRead> OSA don't support VOS_EVENT_ALL.\n");

        return VOS_ERR;
    }

    if (0 == events) {
        mdrv_err("<VOS_EventRead> OSA don't support event = 0.\n");

        return VOS_ERR;
    }

    if (VOS_OK != VOS_CheckEvent(taskSelf)) {
        mdrv_err("<VOS_EventRead> VOS_EventRead EVENT not exist.\n");

        return VOS_ERR;
    }

    tempQueue = VOS_GetQueueIDFromFid(g_vosTaskCtrlBlk[taskSelf].fid);
#if (VOS_YES == VOS_CHECK_PARA)
    if (0xffffffff == tempQueue) {
        return VOS_ERR;
    }
#endif

    tempSem = VOS_GetSemIDFromQueue(tempQueue);
#if (VOS_YES == VOS_CHECK_PARA)
    if (0xffffffff == tempSem) {
        return VOS_ERR;
    }
#endif

    if (VOS_OK != VOS_SmP(tempSem, timeOutInMillSec)) {
        mdrv_err("<VOS_EventRead> VOS_SmP Fail.\n");
        return VOS_ERR;
    }

    VOS_SpinLockIntLock(&g_vosTaskSpinLock, lockLevel);

    tempEvent = g_vosTaskCtrlBlk[taskSelf].events & events;

    if (tempEvent) {
        g_vosTaskCtrlBlk[taskSelf].events &= ~tempEvent;
        *retEvents = tempEvent;
    } else {
        *retEvents = VOS_MSG_SYNC_EVENT;
    }

    VOS_SpinUnlockIntUnlock(&g_vosTaskSpinLock, lockLevel);

    return VOS_OK;
}

/*
 * Description: get the queue ID of a TId
 */
VOS_UINT32 VOS_GetQueueIDFromTID(VOS_UINT32 tid)
{
#if (VOS_YES == VOS_CHECK_PARA)
    if (tid >= g_vosTaskCtrlBlkNumber) {
        mdrv_err("<VOS_GetQueueIDFromTID> TID invaild.\n");

        return 0xffffffff;
    }

    if (0xffffffff == g_vosTaskCtrlBlk[tid].fid) {
        mdrv_err("<VOS_GetQueueIDFromTID> FID invaild.\n");

        return 0xffffffff;
    }
#endif

    return VOS_GetQueueIDFromFid(g_vosTaskCtrlBlk[tid].fid);
}

/*
 * Description: get linux thread id from OSA Tid.
 */
VOS_UINT32 VOS_GetRealTID(VOS_UINT32 tid)
{
    if (tid >= g_vosTaskCtrlBlkNumber) {
        mdrv_err("<VOS_GetRealTID> TID invaild.\n");

        return 0xffffffff;
    }

    return (VOS_UINT32)g_vosTaskCtrlBlk[tid].linuxThreadId;
}

#endif

