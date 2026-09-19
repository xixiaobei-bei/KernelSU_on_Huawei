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

#ifndef __RNIC_INTRA_MSG_H__
#define __RNIC_INTRA_MSG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "rnic_dev_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


enum RNIC_INTRA_MSG_ID_ENUM
{
    /* 0x0000 - 0x1000 留给内部消息   */
    ID_RNIC_CCPU_RESET_START_IND        = 0x1001,                               /* _H2ASN_MsgChoice RNIC_CCPU_RESET_IND_STRU */
    ID_RNIC_CCPU_RESET_END_IND          = 0x1002,                               /* _H2ASN_MsgChoice RNIC_CCPU_RESET_IND_STRU */

    ID_RNIC_IMS_DATA_PROC_IND           = 0x1003,                               /* _H2ASN_MsgChoice RNIC_CCPU_RESET_IND_STRU */

    ID_RNIC_NETDEV_READY_IND            = 0x1004,

    /* 可谓可测消息ID */
    ID_RNIC_MNTN_EVT_DIAL_CONNECT       = 0x6001,                               /* _H2ASN_MsgChoice RNIC_MNTN_DIAL_CONN_EVT_STRU */
    ID_RNIC_MNTN_EVT_DIAL_DISCONNECT    = 0x6002,                               /* _H2ASN_MsgChoice RNIC_MNTN_DIAL_DISCONN_EVT_STRU */

    /* 拨号模式信息 按需拨号*/
    ID_RNIC_MNTN_DIAL_MODE_INFO         = 0x6003,                               /* _H2ASN_MsgChoice RNIC_NOTIFY_MSG_STRU */

    /* 拨号时间信息 按需拨号*/
    ID_RNIC_MNTN_IDLE_TIMEOUT_INFO      = 0x6004,                               /* _H2ASN_MsgChoice RNIC_NOTIFY_MSG_STRU */

    /* 是否上报事件 按需拨号*/
    ID_RNIC_MNTN_EVENT_REPORT_INFO      = 0x6005,                               /* _H2ASN_MsgChoice RNIC_NOTIFY_MSG_STRU */

    ID_RNIC_RNIC_MSG_ID_BUTT
};
typedef VOS_UINT32 RNIC_INTRA_MSG_ID_ENUM_UINT32;


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

/*****************************************************************************
 结构名    : RNIC_IMS_DATA_PROC_IND_STRU
 结构说明  : 触发发数指示
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* 消息头 */    /* _H2ASN_Skip */
    RNIC_INTRA_MSG_ID_ENUM_UINT32       enMsgId;                                /* 消息ID */    /* _H2ASN_Skip */
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId;
    VOS_UINT8                           aucReserved[3];

} RNIC_IMS_DATA_PROC_IND_STRU;


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
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __RNIC_INTRA_MSG_H__ */
