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



#ifndef __SI_APP_EMAT_H__
#define __SI_APP_EMAT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "UsimPsInterface.h"

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
#include "MnClient.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "ccore_nv_stru_pam.h"
#include "ScInterface.h"
#endif
#include "nv_stru_pam.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define SI_EMAT_ESIM_EID_MAX_LEN           (16)    /* EID 字节长度 */

#define SI_EMAT_ESIM_PKID_VALUE_LEN        (22)    /* PKID 字节长度 */

#define SI_EMAT_ESIM_PKID_GROUP_MAX_NUM    (10)    /* 最大PKID组数 */


/*******************************************************************************
  3 枚举定义
*******************************************************************************/
enum SI_EMAT_EVENT_ENUM
{
    SI_EMAT_EVENT_ESIM_CLEAN_CNF            = 0,      /* 清除profile信息 */
    SI_EMAT_EVENT_ESIM_CHECK_CNF            = 1,      /* check profile是否为空 */
    SI_EMAT_EVENT_GET_ESIMEID_CNF           = 2,      /* 获取ESIM的EID信息 */
    SI_EMAT_EVENT_GET_ESIMPKID_CNF          = 3,      /* 获取ESIM的PKID信息 */

    SI_EMAT_EVENT_BUTT,
};
typedef VOS_UINT32  SI_EMAT_EVENT_ENUM_UINT32;

enum SI_EMAT_ESIMCHECK_PROFILE_ENUM
{
    SI_EMAT_ESIMCHECK_NO_PROFILE            = 0,      /* profile不存在 */
    SI_EMAT_ESIMCHECK_HAS_PROFILE           = 1,      /* profile存在  */

    SI_EMAT_ESIMCHECK_BUTT
};
typedef VOS_UINT32  SI_EMAT_ESIMCHECK_PROFILE_ENUM_UINT32;

/*****************************************************************************
  4 数据结构定义
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           aucEsimEID[SI_EMAT_ESIM_EID_MAX_LEN + 1];
    VOS_UINT8                           aucRsv[3];
}SI_EMAT_EVENT_ESIMEID_CNF_STRU;


typedef struct
{
    VOS_UINT8                           aucEsimPKID[SI_EMAT_ESIM_PKID_VALUE_LEN + 1];
    VOS_UINT8                           aucRsv[3];
}SI_EMAT_ESIMPKID_VALUE_STRU;


typedef struct
{
    VOS_UINT32                          ulPKIDNum;
    SI_EMAT_ESIMPKID_VALUE_STRU         astPkid[SI_EMAT_ESIM_PKID_GROUP_MAX_NUM];
    VOS_UINT8                           aucRsv[4];
}SI_EMAT_EVENT_ESIMPKID_CNF_STRU;


typedef struct
{
    SI_EMAT_ESIMCHECK_PROFILE_ENUM_UINT32    enHasProfile;
}SI_EMAT_EVENT_ESIMCHECK_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulRest;
}SI_EMAT_EVENT_ESIMCLEAN_CNF_STRU;


typedef struct
{
    VOS_UINT16                              usClientId;
    VOS_UINT8                               ucOpId;
    VOS_UINT8                               ucReserve;
    SI_EMAT_EVENT_ENUM_UINT32               enEventType;
    VOS_UINT32                              ulEMATError;
    union
    {
        SI_EMAT_EVENT_ESIMCLEAN_CNF_STRU    stEsimCleanCnf;
        SI_EMAT_EVENT_ESIMCHECK_CNF_STRU    stEsimCheckCnf;
        SI_EMAT_EVENT_ESIMEID_CNF_STRU      stEidCnf;
        SI_EMAT_EVENT_ESIMPKID_CNF_STRU     stPKIDCnf;
    }EMATEvent;
}SI_EMAT_EVENT_INFO_STRU;

/*****************************************************************************
  5 回调函数数据结构定义
*****************************************************************************/


/*****************************************************************************
  6 函数声明
*****************************************************************************/
VOS_UINT32 SI_EMAT_GetReceiverPid(
    MN_CLIENT_ID_T                      ClientId,
    VOS_UINT32                         *pulReceiverPid);

VOS_UINT32 SI_EMAT_EsimCleanProfile(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

VOS_UINT32 SI_EMAT_EsimCheckProfile(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

VOS_UINT32 SI_EMAT_EsimEidQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

VOS_UINT32 SI_EMAT_EsimPKIDQuery(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId);

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of si_app_emat.h */
