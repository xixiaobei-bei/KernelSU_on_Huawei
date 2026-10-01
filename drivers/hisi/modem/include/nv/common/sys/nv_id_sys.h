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


#ifndef __SYS_NV_ID_H__
#define __SYS_NV_ID_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 *  NV ID 的添加按从小到大排列
 */

typedef enum
{
    NV_ID_DRV_IMEI                        = 0,         //for syslist
    NV_ID_DRV_RESUME_FLAG                 = 4,         //for taf
    NV_ID_DRV_LED_CONTROL                 = 7,
    NV_ID_DRV_AT_SHELL_OPNE               = 33,
    NV_ID_DRV_NV_FACTORY_INFO_I           = 114,       //for syslist
    NV_ID_DRV_SCI_DSDA_SELECT             = 128,
    en_NV_Item_ErrLogCtrlInfo             = 134,    
    en_NV_Item_AlarmidRelationship        = 135,
    en_NV_Item_FTMDetail                  = 136,    
    NV_ID_DRV_SYS_ID_MAX                  = 1999
}NV_ID_SYS_ACORE_ENUM;

typedef enum
{
    en_NV_Item_IMEI                         = 0,
    en_NV_Resume_Flag                       = 4,
    en_NV_Item_USIM_TEMP_SENSOR_TABLE       = 13, 
    en_NV_Item_AT_SHELL_OPEN_FLAG           = 33,
    en_NV_Item_Factory_Info                 = 114,
    en_NV_Item_SCI_DSDA_CFG                 = 128
}NV_ID_SYS_OLD_ACORE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

