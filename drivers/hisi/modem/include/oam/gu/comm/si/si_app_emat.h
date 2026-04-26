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

/*
 * 版权所有（c）华为技术有限公司 0018-2020
 * 功能描述: eSIM产线相关的AT命令对外接口
 * 生成日期: 2018年11月14日
 */

#ifndef __SI_APP_EMAT_H__
#define __SI_APP_EMAT_H__

#include "vos.h"
#include "usimm_ps_interface.h"

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
#include "mn_client.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "ccore_nv_stru_pam.h"
#include "sc_interface.h"
#endif
#include "nv_stru_pam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define SI_EMAT_ESIM_EID_MAX_LEN 16 /* EID 字节长度 */

#define SI_EMAT_ESIM_PKID_VALUE_LEN 22 /* PKID 字节长度 */

#define SI_EMAT_ESIM_PKID_GROUP_MAX_NUM 10 /* 最大PKID组数 */

enum SI_EMAT_Event {
    SI_EMAT_EVENT_ESIM_CLEAN_CNF = 0,   /* 清除profile信息 */
    SI_EMAT_EVENT_ESIM_CHECK_CNF = 1,   /* check profile是否为空 */
    SI_EMAT_EVENT_GET_ESIMEID_CNF = 2,  /* 获取ESIM的EID信息 */
    SI_EMAT_EVENT_GET_ESIMPKID_CNF = 3, /* 获取ESIM的PKID信息 */

    SI_EMAT_EVENT_BUTT,
};
typedef VOS_UINT32 SI_EMAT_EventUint32;

enum SI_EMAT_EsimCheckProfile {
    SI_EMAT_ESIMCHECK_NO_PROFILE = 0,  /* profile不存在 */
    SI_EMAT_ESIMCHECK_HAS_PROFILE = 1, /* profile存在  */

    SI_EMAT_ESIMCHECK_BUTT
};
typedef VOS_UINT32 SI_EMAT_EsimCheckProfileUint32;

/*
 * 功能说明: EID发送AT消息结构
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
typedef struct {
    VOS_UINT8 esimEID[SI_EMAT_ESIM_EID_MAX_LEN + 1];
    VOS_UINT8 rsv[3];
} SI_EMAT_EventEsimEidCnf;

/*
 * 功能说明: PKID发送AT消息结构
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
typedef struct {
    VOS_UINT8 esimPKID[SI_EMAT_ESIM_PKID_VALUE_LEN + 1];
    VOS_UINT8 rsv[3];
} SI_EMAT_EsimpkidValue;

/*
 * 功能说明: EMAT的请求消息结构
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
typedef struct {
    VOS_UINT32 pkIdNum;
    SI_EMAT_EsimpkidValue pkId[SI_EMAT_ESIM_PKID_GROUP_MAX_NUM];
    VOS_UINT8 rsv[4];
} SI_EMAT_EventEsimPkidCnf;

/*
 * 功能说明: EMAT的请求消息结构
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
typedef struct {
    SI_EMAT_EsimCheckProfileUint32 hasProfile;
} SI_EMAT_EventEsimCheckCnf;

/*
 * 功能说明: EMAT的请求消息结构
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
typedef struct {
    VOS_UINT32 rest;
} SI_EMAT_EventEsimCleanCnf;

/*
 * 功能说明: EMAT的请求消息结构
 * 1. 日    期: 2019年1月18日
 *    修改内容: eSIM适配项目新增
 */
typedef struct {
    VOS_UINT16 clientId;
    VOS_UINT8 opId;
    VOS_UINT8 reserve;
    SI_EMAT_EventUint32 eventType;
    VOS_UINT32 ematError;
    union {
        SI_EMAT_EventEsimCleanCnf esimCleanCnf;
        SI_EMAT_EventEsimCheckCnf esimCheckCnf;
        SI_EMAT_EventEsimEidCnf eidCnf;
        SI_EMAT_EventEsimPkidCnf pkIdCnf;
    } ematEvent;
} SI_EMAT_EventInfo;

VOS_UINT32 SI_EMAT_EsimCleanProfile(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

VOS_UINT32 SI_EMAT_EsimCheckProfile(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

VOS_UINT32 SI_EMAT_EsimEidQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

VOS_UINT32 SI_EMAT_EsimPKIDQuery(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
