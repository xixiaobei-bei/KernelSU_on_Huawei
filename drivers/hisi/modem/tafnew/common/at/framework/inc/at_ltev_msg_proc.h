/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#ifndef _AT_LTEV_MSG_PROC_H_
#define _AT_LTEV_MSG_PROC_H_

#include "AtParse.h"
#include "at_mt_interface.h"

#if (FEATURE_LTEV == FEATURE_ON)
#include "taf_v2x_api.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
#if (FEATURE_LTEV == FEATURE_ON)
/* AT与VRRC模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_VRRC_MsgProcFunc)(MN_AT_IndEvt *msg);

/* AT与VMAC模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_VMAC_MsgProcFunc)(MN_AT_IndEvt *msg);

/* AT与VTC模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_VTC_MsgProcFunc)(VTC_AT_MsgCnf *msg);


/* AT与VSYNC模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_VSYNC_MsgProcFunc)(MN_AT_IndEvt *msg);

/*lint -e958 -e959 ;cause:64bit*/
typedef struct {
    AT_VRRC_MsgTypeUint16 msgName;
    VOS_UINT16            reserved;
    AT_VRRC_MsgProcFunc   procMsgFunc;
} AT_VRRC_ProcMsgTbl;

typedef struct {
    AT_VMAC_MsgTypeUint16 msgName;
    VOS_UINT16            reserved;
    AT_VMAC_MsgProcFunc   procMsgFunc;
} AT_VMAC_ProcMsgTbl;

typedef struct {
    AT_VRRC_MsgTypeUint16 msgName;
    VOS_UINT16            reserved;
    AT_VSYNC_MsgProcFunc  procMsgFunc;
} AT_VSYNC_ProcMsgTbl;


typedef struct {
    AT_VTC_MsgTypeUint32 msgName;
    AT_VTC_MsgProcFunc   procMsgFunc;
} AT_VTC_ProcMsgTbl;
/*lint +e958 +e959 ;cause:64bit*/

VOS_VOID   AT_ProcMsgFromVrrc(struct MsgCB *msg);
VOS_VOID   AT_ProcMsgFromVmac(struct MsgCB *msg);
VOS_VOID   AT_ProcMsgFromVtc(struct MsgCB *msg);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
