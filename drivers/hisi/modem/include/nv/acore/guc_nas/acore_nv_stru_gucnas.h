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

#ifndef __ACORE_NV_STRU_GUCNAS_H__
#define __ACORE_NV_STRU_GUCNAS_H__

#include "vos.h"
#include "acore_nv_id_gucnas.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define ADS_UL_QUEUE_SCHEDULER_PRI_MAX 9 /* 优先级级别最大数 */


enum ADS_IpfMode {
    ADS_RD_INT   = 0x00, /* 中断上下文 */
    ADS_RD_THRD  = 0x01, /* 线程上下文 */

    ADS_IPFMODE_BUTT
};
typedef VOS_UINT8 ADS_IpfModeUint8;


typedef struct {
    /*
     * IPF处理ADS下行数据的模式。
     * 0：中断上下文；
     * 1：线程上下文。
     */
    ADS_IpfModeUint8 ipfMode;
    VOS_UINT8        reserved0; /* 保留位 */
    VOS_UINT8        reserved1; /* 保留位 */
    VOS_UINT8        reserved2; /* 保留位 */
} TAF_NV_AdsIpfModeCfg;


typedef struct {
    /*
     * 持锁功能是否使能。
     * 1：使能
     * 0：禁止
     */
    VOS_UINT32 enable;
    VOS_UINT32 txWakeTimeout; /* 发送持锁时间，单位：毫秒。 */
    VOS_UINT32 rxWakeTimeout; /* 接收持锁时间，单位：毫秒。 */
    VOS_UINT32 reserved;      /* 保留字节。 */
} TAF_NV_AdsWakeLockCfg;


typedef struct {
    /*
     * 内存块数量。
     * 注：BlkSize为448 bytes的内存块数最小为512；BlkSize为1540 bytes的内存块数最小768块。
     */
    VOS_UINT16 blkNum;
    VOS_UINT16 reserved0; /* 保留字节。 */
    VOS_UINT16 reserved1; /* 保留字节。 */
    VOS_UINT16 reserved2; /* 保留字节。 */

} TAF_NV_AdsMemCfg;


typedef struct {
    /*
     * 免CAHCE功能是否使能。
     * 1：使能；
     * 0：禁止。
     */
    VOS_UINT32 enable;
    /*
     * memCfg[0]（BlkSize为448 bytes的内存配置）
     * memCfg[1]（BlkSize为1540 bytes的内存配置）
     */
    TAF_NV_AdsMemCfg memCfg[2];
} TAF_NV_AdsMemPoolCfg;


typedef struct {
    /*
     * 反馈功能是否使能。
     * 1：使能
     * 0：禁止
     */
    VOS_UINT32 enabled;
    /* 错包率阈值，取值范围[50,100]，单位：百分比。 */
    VOS_UINT32 errorRateThreshold;  /* 错包率阈值，取值范围[50,100]，单位：百分比。 */
    VOS_UINT32 detectDuration;      /* 错包检测持续时间，取值范围[1000,10000]，单位：毫秒。*/
    VOS_UINT32 reserved;            /* 保留字节 */

} TAF_NV_AdsErrorFeedbackCfg;


typedef struct {
    /*
     * 是否使能ADS_Queue_Scheduler_Pri特性。
     * 0：未使能，表示不采用优先级,先激活先处理；
     * 1：采用优先级算法。
     */
    VOS_UINT32 status;
    VOS_UINT16 priWeightedNum[ADS_UL_QUEUE_SCHEDULER_PRI_MAX]; /* ADS上行队列优先级的加权值。 */
    VOS_UINT8  rsv[2];                                         /* 保留位 */
} ADS_UL_QueueSchedulerPriNv;


typedef struct {
    VOS_UINT32 waterLevel1; /* 水线界别1 */
    VOS_UINT32 waterLevel2; /* 水线界别2 */
    VOS_UINT32 waterLevel3; /* 水线界别3 */
    VOS_UINT32 waterLevel4; /* 水线界别4,预留 */
} ADS_UL_WaterMarkLevel;


typedef struct {
    VOS_UINT32 threshold1; /* 赞包门限1 */
    VOS_UINT32 threshold2; /* 赞包门限2 */
    VOS_UINT32 threshold3; /* 赞包门限3 */
    VOS_UINT32 threshold4; /* 赞包门限4 */
} ADS_UL_ThresholdLevel;


typedef struct {
    VOS_UINT32                   activeFlag;       /* 使能标识。0：去使能，1：使能 */
    VOS_UINT32                   protectTmrExpCnt; /* 保护定时器超时计数时长，单位：秒。 */
    ADS_UL_WaterMarkLevel waterMarkLevel;   /* 数据包水线界别。 */
    ADS_UL_ThresholdLevel  thresholdLevel;   /* 动态组包门限。 */
    VOS_UINT32                   reserved[6];     /* 保留字段 */
} ADS_NV_DynamicThreshold;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __ACORE_NV_STRU_GUCNAS_H__ */
