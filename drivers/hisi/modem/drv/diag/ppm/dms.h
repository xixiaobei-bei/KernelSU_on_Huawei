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


#ifndef __DMS_H__
#define __DMS_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "OmCommonPpm.h"
#include <osl_types.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMS_VCOM_OM_CHAN_CTRL           (DMS_OM_CHAN_LTE_CTRL)
#define DMS_VCOM_OM_CHAN_DATA           (DMS_OM_CHAN_LTE_DATA)
#define DMS_VCOM_OM_CHAN_BUTT           (DMS_OM_CHAN_BUTT)


/*******************************************************************************
  3 枚举定义
*******************************************************************************/
enum DMS_OM_CHAN_ENUM
{
    DMS_OM_CHAN_LTE_CTRL                = 0,
    DMS_OM_CHAN_LTE_DATA,
    DMS_OM_CHAN_GU_DATA,

    DMS_OM_CHAN_BUTT
};
typedef u32 DMS_OM_CHAN_ENUM_UINT32;
enum DMS_OM_DATA_MODE_ENUM
{
    DMS_OM_DATA_MODE_TRANSPARENT        = 0,
    DMS_OM_DATA_MODE_COMPRESSED,

    DMS_OM_DATA_MODE_ENUM_BUTT
};
typedef u8 DMS_OM_DATA_MODE_ENUM_UINT8;

enum DMS_CHAN_EVT_ENUM
{
    DMS_CHAN_EVT_CLOSE                  = 0,
    DMS_CHAN_EVT_OPEN,

    DMS_CHAN_EVT_BUTT
};
typedef u32 DMS_CHAN_EVT_ENUM_UINT32;

typedef u32 (*DMS_OM_CHAN_DATA_READ_CB_FUNC)(DMS_OM_CHAN_ENUM_UINT32 enChan, u8 *pucData, u32 ulLength);
typedef void (*DMS_OM_CHAN_EVENT_CB_FUNC)(DMS_OM_CHAN_ENUM_UINT32 enChan, DMS_CHAN_EVT_ENUM_UINT32 enEvt);

/*****************************************************************************
  4 结构体定义
*****************************************************************************/


/*****************************************************************************
  4 函数声明
*****************************************************************************/
extern u32 DMS_WriteOmData(DMS_OM_CHAN_ENUM_UINT32 enChan, u8 *pucData, u32 ulLength, DMS_OM_DATA_MODE_ENUM_UINT8 enMode);
extern u32 DMS_RegOmChanDataReadCB(DMS_OM_CHAN_ENUM_UINT32 enChan, DMS_OM_CHAN_DATA_READ_CB_FUNC pFunc);
extern u32 DMS_RegOmChanEventCB(DMS_OM_CHAN_ENUM_UINT32 enChan, DMS_OM_CHAN_EVENT_CB_FUNC pFunc);

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 OTHERS定义
*****************************************************************************/




#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of OmCommonPpm.h*/

