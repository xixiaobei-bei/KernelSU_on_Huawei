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

#ifndef __MSP_AT_H__
#define __MSP_AT_H__

#include <vos.h>
#include "TafTypeDef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

#define MSP_AT_SYSMODE_LTE


/*****************************************************************************
 消息定义
*****************************************************************************/
#define ID_MSG_AT_BASE  0x500  /* 注意，如果消息ID需要统一分配，则需要调整该基准值*/

/* Client 定义*/
enum
{
    EN_AT_FW_CLIENT_ID_AUTO     = 0x00,
    EN_AT_FW_CLIENT_ID_PCUI1    = 0x01,         /* AT的PC侧应用客户端，如mobile partner
 */
    EN_AT_FW_CLIENT_ID_PCUI2    = 0x02,         /* AT的PC侧应用客户端，如超级终端
 */
    EN_AT_FW_CLIENT_ID_NCMCTRL  = 0x03,         /* 虚拟网口
 */
    EN_AT_FW_CLIENT_ID_EQU      = 0x10,         /* AT的装备客户端
 */
    EN_AT_FW_CLIENT_ID_PPP      = 0x11,         /* PPP模块客户端
 */
    EN_AT_FW_CLIENT_ID_NDIS     = 0x12,         /* NDIS模块客户端
 */
    EN_AT_FW_CLIENT_ID_DIAG     = 0x20,         /* DIAG模块客户端
 */
    EN_AT_FW_CLIENT_ID_OM       = 0x21,         /* OM模块客户端
 */
    EN_AT_FW_CLIENT_ID_OM_AGENT = 0x22,
    EN_AT_FW_CLIENT_ID_ALL      = 0xFF
};

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                   ulMsgId;           /* 消息名 */
    VOS_UINT32                   ulClientId;        /* 端口号*/
    VOS_UINT32                   ulLen;             /* 数据长度 */
    VOS_UINT8                    pContext[0];       //lint !e43
}AT_FW_DATA_MSG_STRU;




#ifdef __cplusplus
}
#endif


#endif /*__MSP_AT_H__*/

