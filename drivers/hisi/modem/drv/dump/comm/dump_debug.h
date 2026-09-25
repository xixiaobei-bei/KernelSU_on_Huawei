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
#ifndef __DUMP_DEBUG_H__
#define __DUMP_DEBUG_H__

#define DUMP_MAMP_DEGBU_SIZE (0x4000)

void dump_debug_init(void);
void dump_debug_record(u32 step,u32 data);



typedef enum
{
    DUMP_DEBUG_RECV_LR_IPC                 = 0x10,
    DUMP_DEBUG_STOP_LR_WDT                 = 0x11,
    DUMP_DEBUG_STOP_AP_ETB                 = 0x12,
    DUMP_DEBUG_FIND_LR_TASK                = 0x13,
    DUMP_DEBUG_FILL_EXCINFO                = 0x14,
    DUMP_DEBUG_REGISTER_EXCINFO            = 0x15,
    DUMP_DEBUG_EXIT_REGISTER_EXCINFO       = 0x16,
    DUMP_DEBUG_ENTER_DUMP_CALLBACK         = 0x17,
    DUMP_DEBUG_ENTER_SAVE_CALLBACK_INFO    = 0x18,
    DUMP_DEBUG_ENTER_NOC_CALLBACK          = 0x19,
    DUMP_DEBUG_ENTER_DMSS_CALLBACK         = 0x1a,
    DUMP_DEBUG_ENTER_MDMCP_CALLBACK        = 0x1b,
    DUMP_DEBUG_ENTER_MDMAP_CALLBACK        = 0x1c,
    DUMP_DEBUG_ENTER_NR_CALLBACK           = 0x1d,
    DUMP_DEBUG_NOTIFY_LR                   = 0x1e,
    DUMP_DEBUG_NOTIFY_NR                   = 0x1f,
    DUMP_DEBUG_MDMAP_CALLBACK_HOOK_BEFORE  = 0x20,
    DUMP_DEBUG_MDMAP_CALLBACK_HOOK         = 0x21,
    DUMP_DEBUG_MDMAP_CALLBACK_HOOK_AFTER   = 0x22,
    DUMP_DEBUG_TRIGER_SAVE_TASK            = 0x23,
    DUMP_DEBUG_UPDATE_FAIL_COUNT           = 0x24,
    DUMP_DEBUG_NOTIFY_RDR                  = 0x25,
    DUMP_DEBUG_SAVE_BASEINFO               = 0x26,
    DUMP_DEBUG_SAVE_MSG_FULL_INFO          = 0x27,
    DUMP_DEBUG_SAVE_BALONG_RDR_HEAD        = 0x28,
    DUMP_DEBUG_SAVE_MDM_MANDATORY_LOG      = 0x29,
    DUMP_DEBUG_SAVE_LR_MANDATORY_LOG       = 0x2a,
    DUMP_DEBUG_SAVE_NR_MANDATORY_LOG       = 0x2b,
    DUMP_DEBUG_SAVE_OPTION_LOG             = 0x2c,
    DUMP_DEBUG_SAVE_APR_LOG                = 0x2d,
    DUMP_DEBUG_EXCEPTION_HANDLE_DONE       = 0x2f,
    DUMP_DEBUG_MDM_RESET_START             = 0x30,
    DUMP_DEBUG_MDM_RESET_FAIL              = 0x31,
    DUMP_DEBUG_MDM_RESET_SUCCESS           = 0x32,
    DUMP_DEBUG_MDM_RESET_NOT_SUPPORT       = 0x33,
    DUMP_DEBUG_MDM_RESET_CHECK_FAIL        = 0x34,
    DUMP_DEBUG_CLEAR_CBOOT_AREA            = 0x35,
    DUMP_DEBUG_SAVE_CBOOT_AREA             = 0x36,
    DUMP_DEBUG_GET_CP_FIELD_FAIL           = 0x37,
    DUMP_DEBUG_CP_DONE_SUCCESS             = 0x38,
    DUMP_DEBUG_CP_DONE_TIMEOUT             = 0x39,
    DUMP_DEBUG_CP_WDT_HANDLER              = 0x3a,
    DUMP_DEBUG_STOP_APSS                   = 0x3b,
    DUMP_DEBUG_WAIT_CP_BOTH_FAIL           = 0x3c,
    DUMP_DEBUG_WAIT_CP_DONE                = 0x3d,
    DUMP_DEBUG_EXCWPTION_HANDLE_DONE       = 0x3e,
    DUMP_DEBUG_NOT_HANDLE_NEW_EXC          = 0x3f,
    DUMP_DEBUG_CALL_RDR_SYSTEM_ERROR       = 0x40,
    DUMP_DEBUG_SAVE_MNTN_BIN               = 0x41,
    DUMP_DEBUG_SAVE_USER_DATA              = 0x42,
    DUMP_DEBUG_MDMAP_ENTER_SYSTEM_ERROR    = 0x43,
    DUMP_DEBUG_SAVE_REG                    = 0x44,
    DUMP_DEBUG_SAVE_BOOT_LOG               = 0x45,
    DUMP_DEBUG_ENANLE_LR_SEC               = 0x46,
    DUMP_DEBUG_ENANLE_NR_SEC               = 0x47,
    DUMP_DEBUG_ENANLE_DEST_SEC             = 0x48,
    DUMP_DEBUG_TRIGER_SEC_OS               = 0x49,
    DUMP_DEBUG_DISABLE_LR_SEC              = 0x4a,
    DUMP_DEBUG_DISABLE_NR_SEC              = 0x4b,
    DUMP_DEBUG_SEC_FILE_SAVE               = 0x4c,
    DUMP_DEBUG_SEC_FILE_SAVE_FAIL          = 0x4d,
    DUMP_DEBUG_SEC_SIGLE_FILE_SAVE         = 0x4e,
    DUMP_DEBUG_ENTER_NR_IPC                = 0x4f,
    DUMP_DEBUG_STOP_NR_WDT                 = 0x50,
    DUMP_DEBUG_GET_NR_FIELD_FAIL           = 0x51,
    DUMP_DEBUG_GET_NR_CPU_NAME             = 0x52,
    DUMP_DEBUG_SAVE_NR_BASEINFO            = 0x53,
    DUMP_DEBUG_FILL_NR_EXCINFO             = 0x54,
    DUMP_DEBUG_ENTER_NR_WDT                = 0x55,
    DUMP_DEBUG_GET_NR_DONE                 = 0x56,
    DUMP_DEBUG_GET_NR_DONE_FAIL            = 0x57,
    DUMP_DEBUG_SEND_NR_FIQ                 = 0x58,
    DUMP_DEBUG_NR_BOTH_FAIL                = 0x59,
    DUMP_DEBUG_SAVE_NR_CCPU_DDR            = 0x5a,
    DUMP_DEBUG_SAVE_NR_CCPU_LLRAM          = 0x5b,
    DUMP_DEBUG_SAVE_NR_L2HAC_LLRAM         = 0x5c,
    DUMP_DEBUG_SAVE_NR_PDE                 = 0x5d,
    DUMP_DEBUG_SAVE_NR_SHARE               = 0x5e,
    DUMP_DEBUG_SAVE_LR_DDR_BIN             = 0x5f,
    DUMP_DEBUG_SAVE_LR_DTS                 = 0x60,
    DUMP_DEBUG_SAVE_MDM_SEC_SHARE          = 0x61,
    DUMP_DEBUG_SAVE_MDM_SHARE              = 0x62,
    DUMP_DEBUG_SAVE_MDM_LLRAM              = 0x63,
    DUMP_DEBUG_SAVE_EASYRF                 = 0x64,
    DUMP_DEBUG_SAVE_LPHY                   = 0x65,
    DUMP_DEBUG_ENTER_DMSS_NOC_AGAIN        = 0x66,
    DUMP_DEBUG_POWER_MODEM_OFF             = 0x67,
    DUMP_DEBUG_SEND_CP_FIQ                 = 0x68,
    DUMP_DEBUG_SAVE_LR_SRAM                = 0x69,
    DUMP_DEBUG_CLEAR_LAST_EXCINFO          = 0x70,
    DUMP_DEBUG_ENABLE_LR_WDT               = 0x71,
    DUMP_DEBUG_ENABLE_NR_WDT               = 0x72,
    DUMP_DEBUG_TRIGER_PRESS                = 0x73,
    DUMP_DEBUG_START_PRESS                 = 0x74,

}DUMP_DEBUT_STEP;


#endif

