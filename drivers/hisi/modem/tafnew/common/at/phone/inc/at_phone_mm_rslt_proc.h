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
#ifndef __ATPHONEMMRSLTPROC_H__
#define __ATPHONEMMRSLTPROC_H__

#include "vos.h"
#include "taf_type_def.h"
#include "taf_app_mma.h"
#include "taf_ccm_api.h"
#include "at_ctx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)
/* SYSCFGEX命令支持制式组合定制 */
#define AT_SYSCFGEX_BAND_NUM_PER_SET 32
#define AT_SYSCFGEX_BAND_BIT_STRING_LEN 64
#define AT_SYSCFGEX_BAND_NAME_GROUP_LEN 400
#define AT_SYSCFGEX_RAT_GROUP_CTRL_MAX_ITEM 10 /* 支持AT命令^SYSCFGEX中acqorder最大组合数 */
#define AT_SYSCFGEX_RAT_GROUP_MAX_LEN 152 /* 制式组合字符串输出时RAT GROUP最大长度 */

#define AT_BAND_NAME_GROUP_MAX_VALUE 12

/* AT^SYSCFGEX=?返回结果中的LTE频段的bit位组合 */
typedef struct {
    VOS_CHAR bandBitString[AT_SYSCFGEX_BAND_BIT_STRING_LEN + 1];
} AtSysCfgExBandBitMask;

extern VOS_UINT32 AT_RcvMmaPhoneModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafMmaCsqQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaDetachCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaAttachCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaAttachStatusQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCopsQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 At_QryParaRspSysinfoProc(VOS_VOID *msg, VOS_UINT8 indexNum);

/* begin V7R1 PhaseI Modify */
VOS_UINT32 AT_QryParaRspSysinfoExProc(VOS_VOID *msg, VOS_UINT8 indexNum);
/* end V7R1 PhaseI Modify */
VOS_UINT32 AT_RcvTafMmaSyscfgTestCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 At_QryParaPlmnListProc(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaAccessModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern VOS_UINT32 AT_RcvMmaCFPlmnSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

extern VOS_UINT32 AT_RcvMmaCFPlmnQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
extern TAF_VOID At_QryParaRspCfplmnProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para);
extern VOS_UINT32 AT_RcvMmaPrefPlmnTypeSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID AT_ReportQryPrefPlmnCmdPara(TAF_MMA_PrefPlmnQueryCnf *cpolQryCnf, AT_ModemNetCtx *netCtx, VOS_UINT16 *length,
                                     VOS_UINT32 indexNum, VOS_UINT32 loop);

VOS_VOID AT_ReportQryPrefPlmnCmd(TAF_MMA_PrefPlmnQueryCnf *cpolQryCnf, VOS_UINT32 *validPlmnNum, AT_ModemNetCtx *netCtx,
                                 VOS_UINT16 *length, VOS_UINT32 indexNum);

VOS_UINT32 AT_RcvMmaPrefPlmnSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_UINT32 AT_RcvMmaPrefPlmnQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

VOS_UINT32 AT_RcvMmaPrefPlmnTestCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafMmaQuickStartSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvTafMmaQuickStartQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaAcInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaMMPlmnInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaCopnInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaRejinfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaNetScanCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaEmRssiCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_VOID   At_QryEonsUcs2RspProc(VOS_UINT8 indexNum, VOS_UINT8 opId,
                                 TAF_MMA_OperateName *eonsUcs2PlmnName, TAF_MMA_EonsUcs2HnbName *eonsUcs2HNBName);
VOS_UINT32 AT_RcvMmaEonsUcs2Cnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaVoiceDomainSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_VoiceDomainTransToOutputValue(TAF_MMA_VoiceDomainUint32 voiceDoman, VOS_UINT32 *value);
VOS_UINT32 AT_RcvMmaVoiceDomainQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif

VOS_UINT32 AT_RcvTafMmaCrpnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMmaUserSrvStateQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_CHAR* AT_ConvertRatModeForQryParaPlmnList(TAF_PH_RA_MODE raMode);
VOS_UINT32 atSetCsqCnfSameProc(struct MsgCB *msgBlock);
#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_VOID AT_FormatSysCfgExTestResult(AT_ModemNetCtx *netCtx, const TAF_MMA_SyscfgTestCnf *testCnf,
    VOS_CHAR *ratPara, VOS_UINT8 index);
#endif
#pragma pack(pop)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
