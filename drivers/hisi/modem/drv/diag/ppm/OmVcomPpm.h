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


#ifndef __OM_VCOM_PPM_H__
#define __OM_VCOM_PPM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <product_config.h>
#include "OmCommonPpm.h"
#include <osl_types.h>
#include "dms.h"
#include "diag_vcom.h"


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
#if (FEATURE_ON == FEATURE_LOGCAT_SINGLE_CHANNEL)
#define PPM_VCOM_CHAN_CTRL              (DMS_OM_CHAN_LTE_CTRL)
#define PPM_VCOM_CHAN_DATA              (DMS_OM_CHAN_LTE_DATA)
    
#define PPM_VCOM_EVT_CHAN_CLOSE         (DMS_CHAN_EVT_CLOSE)
#define PPM_VCOM_EVT_CHAN_OPEN          (DMS_CHAN_EVT_OPEN)
    
#define PPM_VCOM_DATA_MODE_COMPRESSED   (DMS_OM_DATA_MODE_COMPRESSED)
#define PPM_VCOM_DATA_MODE_TRANSPARENT  (DMS_OM_DATA_MODE_TRANSPARENT)
    
#define PPM_VOM_SEND_DATA(chan,data,len,mode)\
                DMS_WriteOmData(chan,data,len,mode)
#else
#define PPM_VCOM_CHAN_CTRL              (DIAG_VCOM_CHAN_CTRL)
#define PPM_VCOM_CHAN_DATA              (DIAG_VCOM_CHAN_DATA)
    
#define PPM_VCOM_EVT_CHAN_CLOSE         (DIAG_VCOM_EVT_CHAN_CLOSE)
#define PPM_VCOM_EVT_CHAN_OPEN          (DIAG_VCOM_EVT_CHAN_OPEN)
    
#define PPM_VCOM_DATA_MODE_COMPRESSED   (DIAG_VCOM_OM_DATA_MODE_COMPRESSED)
#define PPM_VCOM_DATA_MODE_TRANSPARENT  (DIAG_VCOM_OM_DATA_MODE_TRANSPARENT)
    
#define PPM_VOM_SEND_DATA(chan,data,len,mode)\
                diag_vcom_report_om_data(chan,data,len,mode)
#endif


/*******************************************************************************
  3 枚举定义
*******************************************************************************/


/*****************************************************************************
  4 结构体定义
*****************************************************************************/


/*****************************************************************************
  4 函数声明
*****************************************************************************/
u32 PPM_VComCfgSendData(u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen);

u32 PPM_VComCfgReadData(u32 ulDevIndex, u8 *pData, u32 uslength);

u32 PPM_VComIndSendData(u8 *pucVirAddr, u8 *pucPhyAddr, u32 ulDataLen);

OM_VCOM_DEBUG_INFO *PPM_VComGetIndInfo(void);

void   PPM_VComCfgPortInit(void);

void   PPM_VComIndPortInit(void);

void   PPM_VComPortInit(void);

void   GU_OamErrLogVComPortInit(void);

void   PPM_VComInfoShow(void);

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

