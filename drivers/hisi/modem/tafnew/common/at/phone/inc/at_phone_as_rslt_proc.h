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
#ifndef __ATPHONEASRSLTPROC_H__
#define __ATPHONEASRSLTPROC_H__

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

#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)

enum AT_FreqlockModeType {
    AT_FREQLOCK_MODE_TYPE_GSM     = 1, /* GSM */
    AT_FREQLOCK_MODE_TYPE_WCDMA   = 2, /* WCDMA */
    AT_FREQLOCK_MODE_TYPE_TDSCDMA = 3, /* TD-SCDMA */
    AT_FREQLOCK_MODE_TYPE_LTE     = 4, /* LTE */
    AT_FREQLOCK_MODE_TYPE_BUTT
};
typedef VOS_UINT32 AT_FreqlockModeTypeUint32;


enum AT_FreqlockBandType {
    AT_FREQLOCK_BAND_TYPE_GSM_850  = 0, /* GSMÆµ¶Î850 */
    AT_FREQLOCK_BAND_TYPE_GSM_900  = 1, /* GSMÆµ¶Î900 */
    AT_FREQLOCK_BAND_TYPE_GSM_1800 = 2, /* GSMÆµ¶Î1800 */
    AT_FREQLOCK_BAND_TYPE_GSM_1900 = 3, /* GSMÆµ¶Î1900 */
    AT_FREQLOCK_BAND_TYPE_BUTT
};
typedef VOS_UINT32 AT_FreqlockBandTypeUint32;

typedef struct {
    AT_FreqlockModeTypeUint32 mode;
    VOS_UINT32                reserve;
    VOS_CHAR                 *strMode;
} AT_FreqlockModeTbl;


typedef struct {
    AT_FreqlockBandTypeUint32 band;
    VOS_UINT32                reserve;
    VOS_CHAR                 *strGsmBand;
} AT_FreqlockGsmBandTbl;
#endif

VOS_UINT32 AT_RcvMtaWrrCellinfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaCsnrQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaWrrFreqLockQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 AT_RcvMtaWrrCellSrhQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)
VOS_UINT32 AT_RcvMtaQryM2MFreqLockCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
#endif
VOS_UINT32 AT_RcvMtaGFreqLockQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);
VOS_UINT32 atQryCellIdCnfProc(struct MsgCB *msgBlock);
VOS_UINT32 AT_RcvMtaTransModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
