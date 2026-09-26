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

#ifndef __AT_OAM_INTERFACE_H__
#define __AT_OAM_INTERFACE_H__

#include "si_app_emat.h"
#include "v_msg.h"
#include "si_app_pih.h"
#include "si_app_pb.h"
#include "si_app_stk.h"
#include "mn_client.h"
#include "mn_call_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/*
 * 结构说明: TAF与OAM消息接口枚举
 */
enum AT_OAM_MsgType {
    /* STK -> AT */
    STK_AT_DATAPRINT_CNF, /* MN_APP_StkAtDataprint */
    STK_AT_EVENT_CNF,     /* _H2ASN_MsgChoice MN_APP_StkAtCnf */

    /* PIH -> AT */
    PIH_AT_EVENT_CNF, /* _H2ASN_MsgChoice MN_APP_PihAtCnf */

    /* PB -> AT */
    PB_AT_EVENT_CNF, /* _H2ASN_MsgChoice MN_APP_PbAtCnf */

    /* EMAT -> AT */
    EMAT_AT_EVENT_CNF, /* _H2ASN_MsgChoice MN_APP_EmatAtCnf */

    AT_OAM_MSG_TYPE_BUTT
};
typedef VOS_UINT32 AT_OAM_MsgTypeUint32;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: PIH给AT上报的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32       msgId;
    SI_PIH_EventInfo pihAtEvent;
} MN_APP_PihAtCnf;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: PIH给AT上报的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32      msgId;
    SI_PB_EventInfo pbAtEvent;
} MN_APP_PbAtCnf;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: STK给AT上报的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgId;
    SI_STK_PrintdataCnf stkAtPrint;
} MN_APP_StkAtDataprint;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: STK给AT上报的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32       msgId;
    SI_STK_EventInfo stkAtCnf;
} MN_APP_StkAtCnf;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明: EMAT给AT上报的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32        msgId;
    SI_EMAT_EventInfo ematAtCnf;
} MN_APP_EmatAtCnf;

extern VOS_VOID At_PbCallBackFunc(SI_PB_EventInfo *event);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_oam_interface.h */
