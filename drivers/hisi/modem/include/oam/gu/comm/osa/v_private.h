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

/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: v_private.h                                                        */
/*                                                                           */
/* Author: Yang Xiangqian                                                    */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2006-10                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Author:                                                                */
/*    Modification: Create this file                                         */
/*                                                                           */
/* 2. Date: 2006-10                                                          */
/*    Author: Xu Cheng                                                       */
/*    Modification: Standardize code                                         */
/*                                                                           */
/*****************************************************************************/

#ifndef _V_PRIVATE_H
#define _V_PRIVATE_H


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif /* __cplusplus */
#endif /* __cplusplus */

#include "mdrv.h"

#define VOS_FLOW_REBOOT     0xffffffff

/* C 核 有 64K 空间, 将最后的16 K 空间拿出来给 任务定位用，每个任务最多 4K */
#if ( (OSA_CPU_CCPU == VOS_OSA_CPU) || (OSA_CPU_NRCPU == VOS_OSA_CPU) )
#define VOS_DUMP_MEM_TOTAL_SIZE         (48*1024)
#define VOS_TASK_DUMP_INFO_SIZE         (4*1024)
#else
#define VOS_DUMP_MEM_TOTAL_SIZE         (16*1024)
#define VOS_TASK_DUMP_INFO_SIZE         (4*1024)
#endif

#define VOS_DUMP_MEM_INFO_NUM           (4)
#define VOS_DUMP_MEM_ALL_SIZE           (VOS_DUMP_MEM_TOTAL_SIZE + (VOS_DUMP_MEM_INFO_NUM*VOS_TASK_DUMP_INFO_SIZE))

#define VOS_MEM_RECORD_BLOCK_SIZE       20

#define VOS_STR_MAX_LEN                 (64*1024)

enum
{
    RTC_CHECK_TIMER_ID = 0x10000000,
    RTC_CHECK_TIMER_RANG,                          /* 0x10000001 */
    RTC_CHECK_TIMER_NOT_EQUAL,                     /* 0x10000002 */
    START_32K_CALLBACK_RELTIMER_FAIL_TO_STOP,      /* 0x10000003 */
    START_32K_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE,  /* 0x10000004 */
    START_32K_RELTIMER_FAIL_TO_STOP,               /* 0x10000005 */
    START_32K_RELTIMER_FAIL_TO_ALLOCATE,           /* 0x10000006 */
    RESTART_32K_RELTIMER_NULL,                     /* 0x10000007 */
    RESTART_32K_RELTIMER_FAIL_TO_CHECK,            /* 0x10000008 */
    VOS_CHECK_TIMER_ID,                            /* 0x10000009 */
    VOS_CHECK_TIMER_RANG,                          /* 0x1000000A */
    VOS_CHECK_TIMER_NOT_EQUAL,                     /* 0x1000000B */
    START_RELTIMER_FLASE_MODE,                     /* 0x1000000C */
    START_RELTIMER_NULL,                           /* 0x1000000D */
    START_RELTIMER_TOO_BIG,                        /* 0x1000000E */
    START_26M_RELTIMER_FAIL_TO_STOP,               /* 0x1000000F */
    START_26M_RELTIMER_FAIL_TO_ALLOCATE,           /* 0x10000010 */
    RESTART_RELTIMER_NULL,                         /* 0x10000011 */
    RESTART_RELTIMER_NOT_RUNNING,                  /* 0x10000012 */
    RESTART_RELTIMER_FAIL_TO_CHECK,                /* 0x10000013 */
    START_CALLBACK_RELTIMER_FALSE_MODE,            /* 0x10000014 */
    START_CALLBACK_RELTIMER_NULL,                  /* 0x10000015 */
    START_CALLBACK_RELTIMER_TOO_BIG,               /* 0x10000016 */
    START_CALLBACK_RELTIMER_FAIL_TO_STOP,          /* 0x10000017 */
    START_CALLBACK_RELTIMER_FAIL_TO_ALLOCATE,      /* 0x10000018 */
    FAIL_TO_ALLOCATE_TIMER_MSG,                    /* 0x10000019 */
    FAIL_TO_ALLOCATE_MSG,                          /* 0x1000001A */
    FAIL_TO_ALLOCATE_MEM,                          /* 0x1000001B */
    DSP_REPORT_ERROR,                              /* 0x1000001C */
    INQUIRE_DSP_ERROR,                             /* 0x1000001D */
    WD_CHECK_ERROR,                                /* 0x1000001E */
    OSA_EXPIRE_ERROR,                              /* 0x1000001F */
    OSA_INIT_ERROR,                                /* 0x10000020 */
    OSA_SEND_MSG_NULL,                             /* 0x10000021 */
    OSA_SEND_MSG_PP_NULL,                          /* 0x10000022 */
    OSA_SEND_MSG_FAIL_TO_CHECK,                    /* 0x10000023 */
    OSA_SEND_MSG_PID_BIG,                          /* 0x10000024 */
    OSA_SEND_MSG_FAIL_TO_WRITE,                    /* 0x10000025 */
    OSA_SEND_URG_MSG_NULL,                         /* 0x10000026 */
    OSA_SEND_URG_MSG_PP_NULL,                      /* 0x10000027 */
    OSA_SEND_URG_MSG_FAIL_TO_CHECK,                /* 0x10000028 */
    OSA_SEND_URG_MSG_PID_BIG,                      /* 0x10000029 */
    OSA_SEND_URG_MSG_FAIL_TO_WRITE,                /* 0x1000002A */
    START_CALLBACK_RELTIMER_FUN_NULL,              /* 0x1000002B */
    START_RELTIMER_PRECISION_TOO_BIG,              /* 0x1000002C */
    START_CALLBACK_RELTIMER_PRECISION_TOO_BIG,     /* 0x1000002D */
    VOS_FAIL_TO_ALLOC_STATIC_MEM,                  /* 0x1000002E */
    HIFI_REPORT_ERROR,                             /* 0x1000002F */
    RTC_FLOAT_MUL_32_DOT_768,                      /* 0x10000030 */
    RTC_FLOAT_MUL_DOT_32768,                       /* 0x10000031 */
    RTC_FLOAT_DIV_32_DOT_768,                      /* 0x10000032 */
    OM_APP_ICC_INIT_ERROR,                         /* 0x10000033 */
    OSA_CHECK_MEM_ERROR,                           /* 0x10000034 */
    OSA_ALLOC_TASK_CONTROL_ERROR,                  /* 0x10000035 */
    OSA_CREATE_TASK_ERROR,                         /* 0x10000036 */
    OSA_FIND_TASK_ERROR,                           /* 0x10000037 */
    OSA_FIND_TASK_PARA_ERROR,                      /* 0x10000038 */
    OSA_SET_TASK_PRI_ERROR,                        /* 0x10000039 */
    RTC_TIMER_EXPIRED_TOO_SHORT,                   /* 0x1000003A */
    CBPCA_VIAMSG_INDEX_ERROR,                      /* 0x1000003B */
    VOS_GET_DRV_VER_INFO_ERROR,                    /* 0x1000003C */
    RTC_TIMER_EXPIRED_TOO_LONG,                    /* 0x1000003D */
    VOS_REBOOT_MEMCPY_MEM,                         /* 0x1000003E */
    VOS_REBOOT_MEMSET_MEM,                         /* 0x1000003F */
    VOS_REBOOT_MEMMOVE_MEM,                        /* 0x10000040 */
    VOS_MEMCTRL_ADDR_ERROR1,                       /* 0x10000041 */
    VOS_MEMCTRL_ADDR_ERROR2,                       /* 0x10000042 */
    VOS_MEMCTRL_ADDR_ERROR3,                       /* 0x10000043 */
    VOS_FREE_MSG_AGAIN,                            /* 0x10000044 */
    OM_APP_EICC_INIT_ERROR,                        /* 0x10000045 */
    START_RELTIMER_WRONG_CPUPID,                   /* 0x10000046 */
    VOS_GET_64BIT_SLICE_ERROR,                     /* 0x10000047 */

    OSA_REBOOT_MODULE_ID_BUTT = 0x1fffffff
};

enum
{
    OM_RNG_BUFFER_ALLOC = 0xE1000000,
    OM_TRANS_BUFFER_ALLOC,                          /* 0xE1000001 */
    USIMM_INIT_ALLOC,                               /* 0xE1000002 */
    USIMM_INIT_DL_LOG_FILE_ALLOC,                   /* 0xE1000003 */
    ACPU_USB_FRAME_INIT,                            /* 0xE1000004 */
    ACPU_PCV_OM_USB_ADDR,                           /* 0xE1000005 */
    ACPU_PCV_OM_PHY_ADDR,                           /* 0xE1000006 */
    ACPU_PCV_PHY_OM_ADDR,                           /* 0xE1000007 */
    PAM_ALLOC_COOKIE_ID_BUTT = 0xE1FFFFFF
};


enum VOS_GET_DRV_VER_INFO_ENUM
{
    VOS_GET_DRV_BOARD_PRODUCT_ID      = 0x00,
    VOS_GET_DRV_PROTOCOL_TYPE         = 0x01,
    VOS_GET_DRV_BOARD_TYPE            = 0x02,

    VOS_GET_DRV_VER_INFO_BUTT
};
typedef VOS_UINT32 VOS_GET_DRV_VER_INFO_ENUM_UINT32;


enum PM_LOG_COSA_PAM_ENUM
{
    PM_LOG_COSA_PAM_TIMER   = 0x00000001,
    PM_LOG_COSA_PAM_ICC     = 0x00000002,
    PM_LOG_COSA_PAM_BUTT,
};
typedef VOS_UINT32 PM_LOG_COSA_PAM_ENUM_UINT32;

VOS_INT VOS_GetDrvVerInfo(VOS_GET_DRV_VER_INFO_ENUM_UINT32 enVerInfo);

#define VOS_SIMPLE_FATAL_ERROR(ulModel)\
    mdrv_om_system_error(ulModel, (VOS_INT)ulFileID, usLineNo, VOS_NULL_PTR, 0)

#define VOS_ProtectionReboot(modId, arg1, arg2, arg3, arg3Length)\
    mdrv_om_system_error(modId, arg1, arg2, arg3, arg3Length)

#define VOS_GetBoardProductId()\
    VOS_GetDrvVerInfo(VOS_GET_DRV_BOARD_PRODUCT_ID)

#define VOS_GetBoardType()\
    VOS_GetDrvVerInfo(VOS_GET_DRV_BOARD_TYPE)

#define VOS_SAVE_STACK(modId) (modId | (0x1 << 24))

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#define DUMP_SAVE_MOD_OSA_MEM OM_AP_OSA
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#define DUMP_SAVE_MOD_OSA_MEM OM_CP_OSA
#endif

#if (OSA_CPU_NRCPU == VOS_OSA_CPU)
#define DUMP_SAVE_MOD_OSA_MEM OM_CP_OSA
#endif

#define VOS_EXCH_MEM_MALLOC\
    mdrv_om_get_field_addr(DUMP_SAVE_MOD_OSA_MEM)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _V_PRIVATE_H */

