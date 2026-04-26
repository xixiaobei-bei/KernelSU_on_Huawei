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


#ifndef __ACORE_NV_ID_GUCNAS_H__
#define __ACORE_NV_ID_GUCNAS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
  1 其他头文件包含
*****************************************************************************/
/*****************************************************************************
 枚举名    : GUCNAS_ACORE_NV_ID_ENUM
 枚举说明  : GUCNAS组件ACORE的NV项ID枚举
*****************************************************************************/
enum GUCNAS_ACORE_NV_ID_ENUM
{
/*  2337 */     NV_ITEM_ADS_IPF_MODE_CFG                 = 2337,     /* TAF_NV_AdsIpfModeCfg                 IPF处理ADS下行数据的模式配置 */
/*  2354 */     NV_ITEM_ADS_WAKE_LOCK_CFG                = 2354,     /* TAF_NV_AdsWakeLockCfg ADS WAKELOCK 配置 */
/*  2369 */     NV_ITEM_ADS_MEM_POOL_CFG                 = 2369,     /* TAF_NV_AdsMemPoolCfg */
/*  2461 */     NV_ITEM_ADS_PACKET_ERROR_FEEDBACK_CFG    = 2461,     /* TAF_NV_AdsErrorFeedbackCfg           下行错包反馈配置*/

/*  2655 */     NV_ITEM_ADS_QUEUE_SCHEDULER_PRI          = 2655,     /* ADS_UL_QueueSchedulerPriNv */

/*  2696 */     NV_ITEM_ADS_DYNAMIC_THRESHOLD_CFG        = 2696,     /* ADS_NV_DynamicThreshold */

/*  BUTT */     NV_ITEM_GUCNAS_ACORE_NV_BUTT,
};


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __ACORE_NV_ID_GUCNAS_H__ */

