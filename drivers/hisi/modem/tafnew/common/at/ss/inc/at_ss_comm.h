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
#ifndef __ATSSCOMM_H__
#define __ATSSCOMM_H__

#include "at_ctx.h"
#include "AtParse.h"
#include "at_mn_interface.h"
#include "taf_app_ssa.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define AT_UNKNOWN_CLCK_CLASS 0
#define AT_PARA_NMEA_MAX_LEN 41
#define AT_PARA_NMEA_MIN_LEN 6
#define AT_PARA_NMEA_GPGSA "$GPGSA"
#define AT_PARA_NMEA_GPGGA "$GPGGA"
#define AT_PARA_NMEA_GPGSV "$GPGSV"
#define AT_PARA_NMEA_GPRMC "$GPRMC"
#define AT_PARA_NMEA_GPVTG "$GPVTG"
#define AT_PARA_NMEA_GPGLL "$GPGLL"
#define AT_PARA_CNAP_MAX_NAME_LEN 183

VOS_VOID AT_CnapConvertNameStr(TAF_CALL_Cnap *nameIndicator, VOS_UINT8 *nameStr, VOS_UINT8 nameStrMaxLen);
extern VOS_VOID   AT_ReportCnapInfo(VOS_UINT8 indexNum, TAF_CALL_Cnap *nameIndicator);
extern TAF_UINT8 At_GetClckClassFromBsCode(TAF_SS_BasicService *bs);
VOS_UINT32 AT_ConvertTafSsaErrorCode(VOS_UINT8 ucIndex, TAF_ERROR_CodeUint32 enErrorCode);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
