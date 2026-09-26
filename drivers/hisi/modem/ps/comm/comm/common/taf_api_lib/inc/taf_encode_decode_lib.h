/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#ifndef _TAF_ENCODE_DECODE_LIB_H_
#define _TAF_ENCODE_DECODE_LIB_H_

#include "PsLogdef.h"
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

enum TAF_COMM_EncodingType {
    TAF_COMM_ENCODING_7BIT,
    TAF_COMM_ENCODING_8BIT,
    TAF_COMM_ENCODING_UCS2,
    TAF_COMM_ENCODING_UTF8,
    TAF_COMM_ENCODING_UTF16,
    TAF_COMM_ENCODING_UTF32,
    TAF_COMM_ENCODING_BUTT
};
typedef VOS_UINT8 TAF_COMM_EncodingTypeUint8;

/*
 * 结构说明: SMS CBS USSD编码字符串结构
 *           enMsgCoding SMS CBS USSD业务支持编码
 *           ulLen       字符串长度
 *           pucStr      字符串指针
 */
typedef struct {
    TAF_COMM_EncodingTypeUint8 coding;
    VOS_UINT8                  reserved1[3];
    VOS_UINT32                 len;
    VOS_UINT8                 *str;
} TAF_COMM_StrWithEncodingType;

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
MODULE_EXPORTED VOS_UINT32 TAF_COMMAPI_ConvertStrEncodingType(
    TAF_COMM_StrWithEncodingType *srcStr,
    TAF_COMM_EncodingTypeUint8 dstCoding,
    VOS_UINT32 dstBuffLen,
    TAF_COMM_StrWithEncodingType *dstStr);
#endif

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
