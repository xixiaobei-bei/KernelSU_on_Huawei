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


#ifndef _OM_PRIVATE_H_
#define _OM_PRIVATE_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define PAM_LOG_PATH                     MODEM_LOG_ROOT"/PAM"
#define PAM_FILE_EXIST                   0

#define PAM_STR_MAX_LEN                  256

#define PAM_VOS_QUEREY_PID_INFO_REQ      0x5aa5
#define PAM_VOS_QUEREY_PID_INFO_CNF      0xa55a
/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum
{
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
    DRX_SLAVE_WAKE_IPC_ERROR            = 0x20000016,
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

    OAM_PROTECTION_ID_BUTT              = 0x2fffffff
};

/* OM 的event定义 */
enum
{
    OAM_EVENT_TIMER             = 5,
    OAM_DEGUG_EVENT_ID1         = 6,
    OAM_DEGUG_EVENT_ID2         = 7,
    OAM_DEGUG_EVENT_ID3         = 8,
    OAM_DEGUG_EVENT_ID4         = 9,
    OAM_EVENT_CONNECT_INFO      = 10,

    /* 下次定义从 100 开始往后 之前的被使用 */
    OAM_DEGUG_EVENT_ID100       = 100,
    OAM_CBT_USB_EVENT_TIMER     = 101,

    OAM_EVENT_BUTT
};

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16           usPrimId;
    VOS_UINT16           usReserved;
}PAM_VOS_QUEREY_PID_INFO_REQ_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16           usPrimId;
    VOS_UINT16           usLen;
    VOS_UINT8            aucValue[4];
}PAM_VOS_QUEREY_PID_INFO_CNF_STRU;

typedef struct
{
   VOS_UINT16                       usSendPid;
   VOS_UINT16                       usRcvPid;
   VOS_UINT32                       ulMsgName;
   VOS_UINT32                       ulSliceStart;
   VOS_UINT32                       ulSliceEnd;
}OM_RECORD_INFO_STRU;

typedef struct
{
   VOS_UINT8                       *pucBuf;
   VOS_UINT32                       ulLen;
   VOS_UINT8                        aucRsv[4];
}OM_RECORD_BUF_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

#if (VOS_OS_VER == VOS_WIN32)
#define PAM_MEM_CPY_S(pDestBuffer, ulDestLen,  pSrcBuffer, ulCount) VOS_MemCpy_s( pDestBuffer, ulDestLen,  pSrcBuffer, ulCount)

#define PAM_MEM_SET_S(pDestBuffer, ulDestLen, ucData, ulCount) VOS_MemSet_s( pDestBuffer, ulDestLen, (VOS_CHAR)(ucData), ulCount )

#define PAM_MEM_MOVE_S(pDestBuffer, ulDestLen, pucSrcBuffer, ulCount) VOS_MemMove_s( pDestBuffer, ulDestLen, pucSrcBuffer, ulCount )
#else
#define PAM_MEM_CPY_S(pDestBuffer, ulDestLen,  pSrcBuffer, ulCount) { \
        if (VOS_NULL_PTR == VOS_MemCpy_s( pDestBuffer, ulDestLen,  pSrcBuffer, ulCount)) \
        {\
        }\
    }

#define PAM_MEM_SET_S(pDestBuffer, ulDestLen, ucData, ulCount) { \
        if (VOS_NULL_PTR == VOS_MemSet_s( pDestBuffer, ulDestLen, (VOS_CHAR)(ucData), ulCount )) \
        { \
        } \
    }

#define PAM_MEM_MOVE_S(pDestBuffer, ulDestLen, pucSrcBuffer, ulCount) { \
        if (VOS_NULL_PTR == VOS_MemMove_s( pDestBuffer, ulDestLen, pucSrcBuffer, ulCount )) \
        { \
        } \
    }
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

