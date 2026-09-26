/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef __ADSINTERFACE_H__
#define __ADSINTERFACE_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define ADS_INTERFACE_MSG_DATA_MSG_LEN 4


enum ADS_MSG_Id {
    /* PDP状态消息 */
    ID_APS_ADS_PDP_STATUS_IND, /* _H2ASN_MsgChoice ADS_PdpStatusInd */

    ID_ADS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32 ADS_MSG_IdUint32;


enum ADS_PdpStatus {
    ADS_PDP_STATUS_ACT,    /* PDP激活成功 */
    ADS_PDP_STATUS_MODIFY, /* PDP修改成功 */
    ADS_PDP_STATUS_DEACT,  /* PDP去激活成功 */
    ADS_PDP_STATUS_BUTT
};
typedef VOS_UINT8 ADS_PdpStatusUint8;


typedef enum {
    ADS_QCI_TYPE_QCI1_GBR    = 0,
    ADS_QCI_TYPE_QCI2_GBR    = 1,
    ADS_QCI_TYPE_QCI3_GBR    = 2,
    ADS_QCI_TYPE_QCI4_GBR    = 3,
    ADS_QCI_TYPE_QCI5_NONGBR = 4,
    ADS_QCI_TYPE_QCI6_NONGBR = 5,
    ADS_QCI_TYPE_QCI7_NONGBR = 6,
    ADS_QCI_TYPE_QCI8_NONGBR = 7,
    ADS_QCI_TYPE_QCI9_NONGBR = 8,

    ADS_QCI_TYPE_BUTT
} ADS_QciType;

typedef VOS_UINT8 ADS_QciTypeUint8;


typedef enum {
    ADS_PDP_IPV4   = 0x01, /* IPV4类型 */
    ADS_PDP_IPV6   = 0x02, /* IPV6类型 */
    ADS_PDP_IPV4V6 = 0x03, /* IPV4V6类型 */
    ADS_PDP_PPP    = 0x04, /* PPP类型 */

    ADS_PDP_TYPE_BUTT = 0xFF
} ADS_PdpType;

typedef VOS_UINT8 ADS_PdpTypeUint8;


typedef enum {
    ADS_CLEAN_RCV_CB_FUNC_TURE  = 0x00,
    ADS_CLEAN_RCV_CB_FUNC_FALSE = 0x01,

    ADS_CLEAN_RCV_CB_FUNC_BUTT = 0xFF
} ADS_CleanRcvCbFlag;

typedef VOS_UINT8 ADS_CleanRcvCbFlagUint8;


typedef struct {
    VOS_MSG_HEADER                     /* 消息头 */ /* _H2ASN_Skip */
    ADS_MSG_IdUint32        msgId;     /* 消息ID */ /* _H2ASN_Skip */
    ModemIdUint16           modemId;   /* Modem Id */
    VOS_UINT8               rabId;     /* Rab Id */
    ADS_PdpStatusUint8      pdpStatus; /* PDP状态 */
    ADS_QciTypeUint8        qciType;   /* QCI */
    ADS_PdpTypeUint8        pdpType;   /* PDP类型 */
    VOS_UINT8               xorHrpdUlIpfFlag;
    ADS_CleanRcvCbFlagUint8 cleanRcvCbFlag; /* 清除ADS下行回调标记 */
} ADS_PdpStatusInd;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
