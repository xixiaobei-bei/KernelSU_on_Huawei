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

#ifndef __GUNASLOGFILTER_H__
#define __GUNASLOGFILTER_H__

#include "vos.h"
#include "taf_type_def.h"
#include "at_mn_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_PIH_FDN_ENABLE_REQ 1
#define AT_PIH_FDN_DISALBE_REQ 2
#define AT_LENGTH_OF_POINTER 4

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_VOID* GUNAS_FilterWrrToGmmMmMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterGmmMmToRrcMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterGasToMmMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterMmaToUsimMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterTafToTafMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterSsToMmMsg(PS_MsgHeader *msg);
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
VOS_VOID* GUNAS_FilterAtToAtMsg(PS_MsgHeader *msg);
#endif

VOS_VOID* GUNAS_FilterAtToMmaMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterAtToMtaMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterAtToPihMsg(PS_MsgHeader *msg);

VOS_VOID* GUNAS_FilterLayerMsg(struct MsgCB *msg);

VOS_VOID* GUNAS_OM_LayerMsgFilter(struct MsgCB *msg);

/* 下移C核后，原A核AT的脱敏函数不使用 */
#if (OSA_CPU_ACPU == VOS_OSA_CPU)
VOS_VOID GUNAS_OM_LayerMsgReplaceCBRegACore(VOS_VOID);
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_VOID I0_GUNAS_OM_LayerMsgReplaceCBRegCcore(VOS_VOID);
VOS_VOID I1_GUNAS_OM_LayerMsgReplaceCBRegCcore(VOS_VOID);
VOS_VOID I2_GUNAS_OM_LayerMsgReplaceCBRegCcore(VOS_VOID);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of gu_nas_log_filter.h */
