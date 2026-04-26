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
 * 功能描述: OSA启动功能实现
 * 生成日期: 2006年10月3日
 */

#include "v_typdef.h"
#include "v_root.h"
#include "vos_config.h"
#include "v_io.h"
#include "v_blk_mem.h"
#include "v_queue.h"
#include "v_sem.h"
#include "v_timer.h"
#include "vos_id.h"
#include "v_int.h"
#include "v_private.h"
#include "mdrv.h"
#include "pam_tag.h"
#include "mdrv_nvim.h"


#define THIS_FILE_ID PS_FILE_ID_VOS_MAIN_C
#define THIS_MODU mod_pam_osa

typedef struct {
    VOS_UINT32 startUpStage;
    VOS_UINT32 step;
    VOS_UINT16 fidInitStep;
    VOS_UINT16 fidInitId;
    VOS_UINT16 pidInitStep;
    VOS_UINT16 pidInitId;
} VOS_StartErrStep;

extern VOS_UINT32 g_omFidInit;
extern VOS_UINT32 g_omPidInit;
extern VOS_UINT16 g_fidInitStep;
extern VOS_UINT16 g_fidInitId;
extern VOS_UINT16 g_pidInitStep;
extern VOS_UINT16 g_pidInitId;

extern VOS_UINT32 VOS_MsgInit(VOS_VOID);

extern VOS_VOID OM_RecordMemInit(VOS_VOID);

VOS_UINT32 g_vosStartUpStage = 0x00010000;
VOS_UINT32 g_vosStartStep = 0;

HTIMER g_vosProtectInitTimer = VOS_NULL_PTR;

VOS_UINT32 *g_osaLogTmp = VOS_NULL_PTR;

/*
 * Description: Debug info Init
 */
VOS_VOID V_LogInit(VOS_VOID)
{
#if (VOS_RTOSCK == VOS_OS_VER)
    VOS_UINT_PTR recordAddr;

    /* 初始化定位信息 */
    recordAddr = (VOS_UINT_PTR)VOS_EXCH_MEM_MALLOC;

    if (VOS_NULL_PTR == recordAddr) {
        return;
    }

    /* COMM在PID初始化流程会用到部分内容，使用最后的16个UINT32作为记录 */
    g_osaLogTmp = (VOS_UINT32 *)(recordAddr + (VOS_DUMP_MEM_TOTAL_SIZE - 16 * sizeof(VOS_UINT32)));

    if (memset_s((VOS_VOID *)g_osaLogTmp, 16 * sizeof(VOS_UINT32), 0x5A, 16 * sizeof(VOS_UINT32)) != EOK) {
        mdrv_om_system_error(VOS_REBOOT_MEMSET_MEM, 0, (VOS_INT)((THIS_FILE_ID << 16) | __LINE__), 0, 0);
    }
#endif
    return;
}

/*
 * Description: record debug info
 */
VOS_VOID V_LogRecord(VOS_UINT32 vosIndex, VOS_UINT32 value)
{
#if (VOS_RTOSCK == VOS_OS_VER)
    if (VOS_NULL_PTR == g_osaLogTmp) {
        return;
    }

    if (vosIndex >= 16) {
        return;
    }

    g_osaLogTmp[vosIndex] = value;
#endif
    return;
}

/*
 * Description: main function
 */
MODULE_EXPORTED VOS_VOID root(VOS_VOID)
{
    mdrv_err("<root> VOS_Startup Begin !\n");

#if (VOS_WIN32 == VOS_OS_VER)
    VOS_SplInit();
#endif

    /* 2016.03.14:底软接口修改，先调用register函数申请内存，后面使用get field函数获取内存地址 */
    (VOS_VOID)mdrv_om_register_field(DUMP_SAVE_MOD_OSA_MEM, "OAM", VOS_DUMP_MEM_ALL_SIZE, 0);

    V_LogInit();

    if (VOS_OK != VOS_Startup(VOS_STARTUP_INIT_DOPRA_SOFEWARE_RESOURCE)) {
        mdrv_err("<root> VOS_Startup Phase 0: Error.\n");
    }

    if (VOS_OK != VOS_Startup(VOS_STARTUP_SET_TIME_INTERRUPT)) {
        mdrv_err("<root> VOS_Startup Phase 1: Error.\n");
    }

    if (VOS_OK != VOS_Startup(VOS_STARTUP_CREATE_TICK_TASK)) {
        mdrv_err("<root> VOS_Startup Phase2: Error.\n");
    }

    if (VOS_OK != VOS_Startup(VOS_STARTUP_CREATE_ROOT_TASK)) {
        mdrv_err("<root> VOS_Startup Phase 3: Error\n");
    }

    if (VOS_OK != VOS_Startup(VOS_STARTUP_SUSPEND_MAIN_TASK)) {
        mdrv_err("<root> VOS_Startup Phase 4: Error\n");
    }

    mdrv_err("<root> VOS_Startup End !\n");

    return;
}

/*
 * Description: reboot if OSA can't init
 */
VOS_VOID VOS_ProtectInit(VOS_UINT32 param1, VOS_UINT32 param2)
{
    VOS_StartErrStep step;

    step.startUpStage = g_vosStartUpStage;
    step.step         = g_vosStartStep;
    step.fidInitStep  = g_fidInitStep;
    step.fidInitId    = g_fidInitId;
    step.pidInitStep  = g_pidInitStep;
    step.pidInitId    = g_pidInitId;

    VOS_ProtectionReboot(OSA_EXPIRE_ERROR, 0, 0, (VOS_CHAR *)&step, sizeof(VOS_StartErrStep));
}

/*
 * Description: startup function
 */
VOS_UINT32 VOS_Startup(enum VOS_STARTUP_PHASE ph)
{
    VOS_UINT32 returnValue;
    VOS_UINT32 startUpFailStage = 0;

    switch (ph) {
        case VOS_STARTUP_INIT_DOPRA_SOFEWARE_RESOURCE:
            g_vosStartUpStage = 0x00010000;
            V_LogRecord(0, 0x00010000);

            if (VOS_OK != VOS_MemInit()) {
                startUpFailStage |= 0x0001;

                break;
            }

            VOS_SemCtrlBlkInit();

            VOS_QueueCtrlBlkInit();

            VOS_TaskCtrlBlkInit();

            if (VOS_OK != VOS_TimerCtrlBlkInit()) {
                startUpFailStage |= 0x0010;
            }

            OM_RecordMemInit();

            if (VOS_OK != RTC_TimerCtrlBlkInit()) {
                startUpFailStage |= 0x0100;
            }

            if (VOS_OK != VOS_PidCtrlBlkInit()) {
                startUpFailStage |= 0x0200;
            }

            if (VOS_OK != VOS_FidCtrlBlkInit()) {
                startUpFailStage |= 0x0400;
            }

            if (VOS_OK != CreateFidsQueque()) {
                startUpFailStage |= 0x0800;
            }
            break;

        case VOS_STARTUP_SET_TIME_INTERRUPT:
            g_vosStartUpStage = 0x00020000;
            V_LogRecord(0, 0x00020000);
            break;

        case VOS_STARTUP_CREATE_TICK_TASK:
            g_vosStartUpStage = 0x00040000;
            V_LogRecord(0, 0x00040000);

            /* create soft timer task */
            if (VOS_OK != VOS_TimerTaskCreat()) {
                startUpFailStage |= 0x0001;
            }

            if (VOS_OK != RTC_TimerTaskCreat()) {
                startUpFailStage |= 0x0002;
            }

            break;

        case VOS_STARTUP_CREATE_ROOT_TASK:
            g_vosStartUpStage = 0x00080000;
            V_LogRecord(0, 0x00080000);

            g_vosStartStep = 0x0000;
            V_LogRecord(1, 0x0000);

            returnValue = VOS_MsgInit();
            if (VOS_OK != returnValue) {
                startUpFailStage |= returnValue;
            }

            g_vosStartStep = 0x0008;
            V_LogRecord(1, 0x0008);

            if (VOS_OK != VOS_FidsInit()) {
                startUpFailStage |= 0x0008;
            }

            g_vosStartStep = 0x0020;
            V_LogRecord(1, 0x0020);

            /* create FID task & selftask task */
            if (VOS_OK != CreateFidsTask()) {
                startUpFailStage |= 0x0020;
            }

            g_vosStartStep = 0x0100;
            V_LogRecord(1, 0x0100);

            if (VOS_OK != VOS_PidsInit()) {
                startUpFailStage |= 0x0100;
            }
            break;

        case VOS_STARTUP_SUSPEND_MAIN_TASK:
            g_vosStartUpStage = 0x00100000;
            V_LogRecord(0, 0x00100000);

            g_vosStartStep = 0x0002;
            V_LogRecord(1, 0x0002);

            /* Resume FID task & selftask task */
            if (VOS_OK != VOS_ResumeFidsTask()) {
                startUpFailStage |= 0x0002;
            }

            g_vosStartStep = 0x0008;
            V_LogRecord(1, 0x0008);

            /* stop protect timer */

#if ((OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU))
            /* OSA初始化完成，需要调用DRV函数通知DRV OSA启动完成 */
            if (VOS_OK != mdrv_sysboot_ok()) {
                startUpFailStage |= 0x0008;
            }
#endif
            break;

        default:
            break;
    }

    /* calculate return value */
    if (0 != startUpFailStage) {
        returnValue = g_vosStartUpStage;
        returnValue |= startUpFailStage;
        mdrv_err("<VOS_Startupstartup> return value=%x.\n", returnValue);

        /* reboot */
        VOS_ProtectionReboot(OSA_INIT_ERROR, (VOS_INT)returnValue, (VOS_INT)g_omPidInit, (VOS_CHAR *)&g_omFidInit,
                             sizeof(VOS_UINT32));

        return (returnValue);
    } else {
        return (VOS_OK);
    }
}

#if (VOS_LINUX == VOS_OS_VER)

/*
 * Description:
 */
extern VOS_INT APP_VCOM_Init(VOS_VOID);

/*
 * Description: linux main function
 */
VOS_INT VOS_ModuleInit(VOS_VOID)
{
#if (FEATURE_OFF == FEATURE_DELAY_MODEM_INIT)
    APP_VCOM_Init();
#endif

    root();

    return 0;
}

#if (FEATURE_OFF == FEATURE_DELAY_MODEM_INIT)
/*
 * Description: linux main function
 */
static VOS_VOID VOS_ModuleExit(VOS_VOID)
{
    return;
}
#endif

/* when flash less on, VOS_ModuleInit should be called by bsp drv. */
#if (FEATURE_OFF == FEATURE_DELAY_MODEM_INIT)
module_init(VOS_ModuleInit);

module_exit(VOS_ModuleExit);

MODULE_AUTHOR("Balong OSA");

MODULE_DESCRIPTION("Hisilicon OSA");

MODULE_LICENSE("GPL");
#endif


#endif

