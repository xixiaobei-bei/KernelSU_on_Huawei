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

#ifndef __HDLC_INTERFACE_H__
#define __HDLC_INTERFACE_H__

#include "product_config.h"
#include "vos.h"
#include "ppp_public.h"
#include "imm_interface.h"

#if (FEATURE_HARDWARE_HDLC_FUNC == FEATURE_ON)
#include "hdlc_hardware_service.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define PPP_ONCE_DEAL_MAX_CNT 200

#define PPPA_DEFAULT_ACCM 0xFFFFFFFF

enum PPP_HDLC_ResultType {
    PPP_HDLC_RESULT_COMM_FINISH   = 0, /* 本次处理正常完成 */
    PPP_HDLC_RESULT_COMM_CONTINUE = 1, /* 本次处理正常，但还有数据在队列中待下次继续处理，用于限制单次最大处理个数 */
    PPP_HDLC_RESULT_COMM_ERROR    = 2, /* 本次处理出错 */

    PPP_HDLC_RESULT_BUTT
};
typedef VOS_UINT32 PPP_HDLC_ResultTypeUint32;

enum PPP_DataType {
    PPP_PULL_PACKET_TYPE = 1, /* IP类型上行数据 */
    PPP_PUSH_PACKET_TYPE,     /* IP类型下行数据 */
    PPP_PULL_RAW_DATA_TYPE,   /* PPP类型上行数据 */
    PPP_PUSH_RAW_DATA_TYPE    /* PPP类型下行数据 */
};
typedef VOS_UINT8 PPP_DataTypeUint8;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
