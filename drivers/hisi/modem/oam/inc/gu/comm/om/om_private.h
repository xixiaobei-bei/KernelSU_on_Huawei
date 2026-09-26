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
 * 功能描述: 该头文件为OM的头文件，主要提供给OM模块内部使用
 * 生成日期: 2008年5月3日
 */
#ifndef _OM_PRIVATE_H_
#define _OM_PRIVATE_H_

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#define PAM_LOG_PATH MODEM_LOG_ROOT "/PAM"
#define PAM_FILE_EXIST 0

#define PAM_STR_MAX_LEN 256
#define PAM_KMSG_PRINT_STR_LEN  99

#define PAM_VOS_QUEREY_PID_INFO_REQ 0x5aa5
#define PAM_VOS_QUEREY_PID_INFO_CNF 0xa55a
#define PAM_DBG_PRINT_IND 0x5a5a

enum {
    OAM_PROTECTION_LOAD_DSP             = 0x20000000,
    OAM_PROTECTION_DSP_Init             = 0x20000001,
    OAM_UL_AHB_PARTITION_ERR            = 0x20000002,
    OAM_UL_AHB_LENGTH_ERR               = 0x20000003,
    OAM_DL_R99_AHB_HEAD_ERR             = 0x20000004,
    OAM_UL_R99_AHB_HEAD_ERR             = 0x20000005,
    OAM_DL_PP_AHB_HEAD_ERR              = 0x20000006,
    OAM_UL_PP_AHB_HEAD_ERR              = 0x20000007,
    OAM_DL_NPP_AHB_HEAD_ERR             = 0x20000008,
    OAM_UL_NPP_AHB_HEAD_ERR             = 0x20000009,
    OAM_PARA_CHECK_ERR                  = 0x2000000a,
    DRV_CACHEMALLOC_ERR                 = 0x2000000b,
    OAM_PROTECTION_DSP_SHARE            = 0x2000000c,
    OAM_USB_SEND_ERROR                  = 0x2000000d,
    OAM_LOAD_DSP_BUFFER_ERR             = 0x2000000e,

    DRX_REPORT_ERROR                    = 0x2000000f,
    DRX_REPORT_BBP_POWERDOWN_ERROR      = 0x20000010,
    DRX_REPORT_BBP_READ_ERROR           = 0x20000011,
    DRX_ACTIVATEHW_ERROR                = 0x20000012,
    DRX_REPORT_HANDSHAKE_ABNORMAL       = 0x20000013,
    DRX_PHY_PROTECT_ERROR               = 0x20000014,
    DRX_FULLNET_SET_ERROR               = 0x20000015,

    DRX_CLEAR_BBP_INT_ERROR             = 0x20000017,
    DRX_BBP_WAKE_TIMEOUT                = 0x20000018,
    DRX_BBP_CLK_SW_TIMEOUT              = 0x20000019,

    SPY_FLOWCTRL_POWEROFF               = 0x20002000,
    CBT_CSDR_DDR_BASE_ADDR_INVALID      = 0x20002001,
    CBT_CSDR_DTCM_BASE_ADDR_INVALID     = 0x20002002,

    DRV_SOCP_ERROR_START                = 0x20000050,
    DRV_SOCP_ERROR_END                  = 0x20000060,
    OAM_REG_REPORT_ERROR_END            = 0x20000070,
    OAM_PC_LENGTH_TOO_BIG               = 0x20000080,

    HPA_2GFRAME_ISR_NOT_INTERRUPT_CALL  = 0x20000090,
    HPA_3GFRAME_ISR_NOT_INTERRUPT_CALL  = 0x20000092,

    PAM_USIMM_SLOT_MODEM_ERR            = 0x200000A0,
    PAM_USIMM_SLOT_MODEM_ERR_1          = 0x200000A1,
    PAM_USIMM_SLOT_MODEM_ERR_2          = 0x200000A2,
    PAM_USIMM_SLOT_MODEM_ERR_3          = 0x200000A3,
    PAM_USIMM_SLOT_MODEM_ERR_4          = 0x200000A4,

    PAM_PIH_SCICHG_TIMEOUT              = 0x200000B0,

    PAM_REBOOT_MEMCPY_MEM               = 0x21000000,
    PAM_REBOOT_MEMSET_MEM               = 0x22000000,
    PAM_REBOOT_MEMMOVE_MEM              = 0x23000000,

    PAM_CBT_REBOOT_MEMCPY_MEM           = 0x24000000,
    PAM_CBT_REBOOT_MEMSET_MEM           = 0x25000000,
    PAM_CBT_REBOOT_MEMMOVE_MEM          = 0x26000000,

    PAM_SC_ALLOC_CUST_IMG_MEM_ERR       = 0x27000000,
    PAM_SC_LOADY_CUST_IMG_MEM_ERR       = 0x27000001,
    PAM_SC_CUST_IMG_DATA_CHECK_ERR      = 0x27000002,

    OAM_PROTECTION_ID_BUTT              = 0x2fffffff
};

enum {
    OAM_EVENT_TIMER        = 5,
    OAM_DEGUG_EVENT_ID1    = 6,
    OAM_DEGUG_EVENT_ID2    = 7,
    OAM_DEGUG_EVENT_ID3    = 8,
    OAM_DEGUG_EVENT_ID4    = 9,
    OAM_EVENT_CONNECT_INFO = 10,

    /* 下次定义从 100 开始往后 之前的被使用 */
    OAM_DEGUG_EVENT_ID100   = 100,
    OAM_CBT_USB_EVENT_TIMER = 101,

    OAM_EVENT_BUTT
};

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 primId;
    VOS_UINT16 reserved;
} PAM_VOS_QuereyPidInfoReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 primId;
    VOS_UINT16 len;
    VOS_UINT8  value[4];
} PAM_VOS_QuereyPidInfoCnf;

typedef struct {
    VOS_UINT16 sendPid;
    VOS_UINT16 rcvPid;
    VOS_UINT32 msgName;
    VOS_UINT32 sliceStart;
    VOS_UINT32 sliceEnd;
} OM_RecordInfo;

typedef struct {
    VOS_UINT8 *buf;
    VOS_UINT32 len;
    VOS_UINT8  rsv[4];
} OM_RecordBuf;

typedef struct {
    VOS_CHAR str[PAM_KMSG_PRINT_STR_LEN + 1];
    VOS_UINT32 data1;
    VOS_UINT32 data2;
    VOS_UINT32 data3;
    VOS_UINT32 data4;
}PAM_KmsgData;
/*
 * 结构说明: 输出到kmsg信息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgName;
    VOS_UINT16 rsv;
    PAM_KmsgData kmsgData;
} PAM_KmsgInd;

#if (VOS_OS_VER == VOS_WIN32)
#define PAM_MEM_CPY_S(destBuffer, destLen, srcBuffer, count) \
    VOS_MemCpy_s(destBuffer, destLen, srcBuffer, count)

#define PAM_MEM_SET_S(destBuffer, destLen, data, count) \
    VOS_MemSet_s(destBuffer, destLen, (VOS_CHAR)(data), count)

#define PAM_MEM_MOVE_S(destBuffer, destLen, srcBuffer, count) \
    VOS_MemMove_s(destBuffer, destLen, srcBuffer, count)
#else
#define PAM_MEM_CPY_S(destBuffer, destLen, srcBuffer, count)                       \
    {                                                                                    \
        if (VOS_NULL_PTR == VOS_MemCpy_s(destBuffer, destLen, srcBuffer, count)) { \
        }                                                                                \
    }

#define PAM_MEM_SET_S(destBuffer, destLen, data, count)                                   \
    {                                                                                            \
        if (VOS_NULL_PTR == VOS_MemSet_s(destBuffer, ulDestLen, (VOS_CHAR)(data), count)) { \
        }                                                                                        \
    }

#define PAM_MEM_MOVE_S(destBuffer, destLen, srcBuffer, count)                       \
    {                                                                                       \
        if (VOS_NULL_PTR == VOS_MemMove_s(destBuffer, destLen, srcBuffer, count)) { \
        }                                                                                   \
    }
#endif

static inline VOS_UINT32 PAM_GetMin(VOS_UINT32 num1, VOS_UINT32 num2)
{
    return (((num1) > (num2)) ? (num2) : (num1));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
