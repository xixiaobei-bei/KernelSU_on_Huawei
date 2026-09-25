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
#ifndef __RNIC_CDS_INTERFACE_H__
#define __RNIC_CDS_INTERFACE_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

#ifdef WIN32
#pragma warning(disable:4200) /* zero-sized array in struct/union */
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define RNIC_CDS_MSG_ID_HEADER           (0x0000)
#define CDS_RNIC_MSG_ID_HEADER           (0x1000)

#define RNIC_CDS_MSG_ID_HEADER_V2        (0x2000)


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


enum RNIC_CDS_MSG_ID_ENUM
{
    /* RNIC <----> CDS */
    ID_RNIC_CDS_IMS_DATA_REQ            = RNIC_CDS_MSG_ID_HEADER + 0x01,        /* _H2ASN_MsgChoice RNIC_CDS_IMS_DATA_REQ_STRU */
    ID_CDS_RNIC_IMS_DATA_IND            = CDS_RNIC_MSG_ID_HEADER + 0x01,        /* _H2ASN_MsgChoice CDS_RNIC_IMS_DATA_IND_STRU */

    /* RNIC  ----> CDS */
    ID_RNIC_CDS_IMS_DATA_REQ_V2         = RNIC_CDS_MSG_ID_HEADER_V2 + 0x01,     /* _H2ASN_MsgChoice RNIC_CDS_IMS_DATA_REQ_V2_STRU */

    ID_RNIC_CDS_MSG_ID_ENUM_BUTT

};
typedef VOS_UINT32 RNIC_CDS_MSG_ID_ENUM_UINT32;


enum RNIC_CDS_WIFI_PDN_TYPE_ENUM
{
    RNIC_CDS_WIFI_PDN_TYPE_NORMAL       = 0x00,
    RNIC_CDS_WIFI_PDN_TYPE_EMC          = 0x01,

    RNIC_CDS_WIFI_PDN_TYPE_BUTT         = 0xFF
};
typedef VOS_UINT8 RNIC_CDS_WIFI_PDN_TYPE_ENUM_UINT8;


/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_MSG_HEADER                                                  /* 消息头 */    /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                    /* 消息类型 */  /* _H2ASN_Skip */

    VOS_UINT16                          usModemId;
    RNIC_CDS_WIFI_PDN_TYPE_ENUM_UINT8   enDataType;
    VOS_UINT8                           aucReserv[3];
    VOS_UINT16                          usDataLen;

    VOS_UINT8                           aucData[4];
} RNIC_CDS_IMS_DATA_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                  /* 消息头 */    /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                    /* 消息类型 */  /* _H2ASN_Skip */

    VOS_UINT16                          usModemId;
    RNIC_CDS_WIFI_PDN_TYPE_ENUM_UINT8   enDataType;
    VOS_UINT8                           aucReserv[3];
    VOS_UINT16                          usDataLen;

    VOS_UINT8                           aucData[4];
} CDS_RNIC_IMS_DATA_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT8                           ucIfaceId;
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usLen;
    VOS_UINT8                           aucData[0];/*lint !e43*/
} RNIC_CDS_IMS_DATA_REQ_V2_STRU;

/*****************************************************************************
  5 UNION定义
*****************************************************************************/

/*****************************************************************************
  6 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  H2ASN顶级消息结构定义
*****************************************************************************/
typedef struct
{
    RNIC_CDS_MSG_ID_ENUM_UINT32         enMsgId;            /* _H2ASN_MsgChoice_Export RNIC_CDS_MSG_ID_ENUM_UINT32 */
    VOS_UINT8                           aucMsg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          RNIC_CDS_MSG_ID_ENUM_UINT32
    ****************************************************************************/
} RNIC_CDS_INTERFACE_MSG_DATA;
/* _H2ASN_Length UINT32 */

typedef struct
{
    VOS_MSG_HEADER
    RNIC_CDS_INTERFACE_MSG_DATA          stMsgData;
} RnicCdsInterface_MSG;


#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_CDS_INTERFACE_H__ */

