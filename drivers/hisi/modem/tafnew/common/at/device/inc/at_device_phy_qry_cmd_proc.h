/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#ifndef _AT_DEVICE_PHY_QRY_CMD_PROC_H_
#define _AT_DEVICE_PHY_QRY_CMD_PROC_H_

#include "vos.h"
#include "at_mt_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
VOS_UINT32 AT_QryProdTypePara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRficDieIDPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryRffeDieIDPara(VOS_UINT8 indexNum);
VOS_UINT32 atQryLTCommCmdPara(VOS_UINT8 clientId);
VOS_UINT32 AT_QryFpowdetTPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryFPllStatusPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryFrssiPara(VOS_UINT8 indexNum);
#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 atQryFCHANPara(VOS_UINT8 clientId);
VOS_UINT32 AT_QryLCaCellExPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryLcaCellRptCfgPara(VOS_UINT8 indexNum);
#endif
VOS_UINT32 At_QryFChanPara(VOS_UINT8 indexNum);
VOS_UINT32 atQryTselrfPara(VOS_UINT8 clientId);
VOS_UINT32 AT_QryTSelRfPara(VOS_UINT8 indexNum);
VOS_UINT32 atQryFTXONPara(VOS_UINT8 clientId);
VOS_UINT32 At_QryFTxonPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_QryDcxotempcompPara(VOS_UINT8 indexNum);
VOS_UINT32 At_QryFRxonPara(VOS_UINT8 indexNum);
VOS_UINT32 atQryFPllStatusPara(VOS_UINT8 clientId);
VOS_UINT32 atQryFRSSIPara(VOS_UINT8 clientId);
VOS_UINT32 atQryFRXONPara(VOS_UINT8 clientId);

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
extern VOS_UINT32 At_QryCltInfo(VOS_UINT8 indexNum);
#else
VOS_UINT32 At_QryRfIcMemTest(VOS_UINT8 indexNum);
VOS_UINT32 At_QryFSerdesRt(VOS_UINT8 indexNum);
VOS_UINT32 AT_SndBbicCalQryFtemprptReq(INT16 channelNum);
VOS_UINT32 At_CovertChannelTypeToBbicCal(AT_TEMP_ChannelTypeUint16           channelType,
                                         BBIC_TEMP_CHANNEL_TYPE_ENUM_UINT16 *bbicChannelType);
VOS_UINT32            AT_QryFtemprptPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_SndBbicRssiReq(VOS_VOID);
#endif

VOS_UINT32 AT_QryLcacellPara(VOS_UINT8 indexNum);
VOS_UINT32 AT_TestFPllStatusPara(VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
