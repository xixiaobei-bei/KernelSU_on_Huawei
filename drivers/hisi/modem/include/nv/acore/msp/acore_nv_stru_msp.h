/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
 * Description :file format:UTF-8_format
 */


#ifndef __MSP_NV_DEF_ACORE_H__
#define __MSP_NV_DEF_ACORE_H__

#include "vos.h"
#include "acore_nv_id_msp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define CHR_FAULTID_GUTL_MIN (0x0)
#define CHR_FAULTID_GUTL_MAX (0xA0)
#define CHR_FAULTID_CDMA_MIN (0xA1)
#define CHR_FAULTID_CDMA_MAX (0xFF)

#define CHR_FAULTID_NUM_MAX (33)


/*
 * 结构名    : NV_ID_CHR_CONFIG_CTRL_INFO_STRU
 * 对应NVID: 1001 NV_ID_CHR_CONFIG_CTRL_INFO
 * 结构说明  :内部使用。总体控制NV,1个
 */
typedef struct _FAULTID_MAP {
    VOS_UINT16 usFaultId;
    VOS_UINT8 aucReserved0[0x2];
    VOS_UINT32 ulNvId;
    VOS_UINT8 aucReserved1[0x4]; // 保留位
} FAULTID_MAP;

/* 结构说明  : CHR GUTL相关故障faultid控制nv。 */
typedef struct _CHR_CONFIG_CTRL {
    /* CHR GUTL的故障faultid的总个数 */
    VOS_UINT32 ulFaultIdNum;
    /*
     * 最大支持17个faultid
     * usFaultId：2 byte， 故障faultid号
     * aucReserved0[2]：2 byte，保留位
     * ulNvId：4 byte，故障对应的nvid号
     * aucReserved1[4]：4 byte，保留位
     */
    FAULTID_MAP aucFaulIdMap[CHR_FAULTID_NUM_MAX];
} NV_ID_CHR_CONFIG_CTRL_INFO_STRU;

/*
 * 结构名    : NV_ID_CHR_C_CONFIG_CTRL_INFO_STRU
 * 对应NVID: 1001 NV_ID_CHR_CONFIG_CTRL_INFO
 * 结构说明  : 内部使用。CHR CDMA相关故障faultid控制nv
 */
typedef struct _CHR_C_CONFIG_CTRL {
    VOS_UINT32 ulFaultIdNum; /* CHR CDMA的故障faultid的总个数 */
    /*
     * 最大支持22个faultid
     * usFaultId：2 byte， 故障faultid号
     * aucReserved0[2]：2 byte，保留位
     * ulNvId：4 byte，故障对应的nvid号
     * aucReserved1[4]：4 byte，保留位
     */
    FAULTID_MAP aucFaulIdMap[CHR_FAULTID_NUM_MAX];
} NV_ID_CHR_C_CONFIG_CTRL_INFO_STRU;

/*
 * 结构名    : NV_ID_CHR_CONFIG_CTRL_INFO_STRU
 * 对应NVID  : NV_ID_CHR_CONFIG_CTRL_INFO
 * 结构说明  : GUTLX使用的NV,255个
 */
typedef struct _CHR_ALARM {
    VOS_UINT32 ulPid;
    VOS_UINT32 ulAlarmId;
} CHR_ALARM;

#define NV_CHR_PID_MAX_NUM 96
/* 结构说明  : CHR CDMA 故障FAULT_ID_HRPD_OR_LTE_OOS（CL模式下HRPD或LTE丢网）相关的pid和alarmid */
typedef struct _CHR_FAULTID_CONFIG {
    VOS_UINT32 ulFaultId;  /* CHR GUTL的故障faultid:FAULT_ID_HRPD_OR_LTE_OOS（CL模式下HRPD或LTE丢网） */
    VOS_UINT32 ulAlarmNum; /* Faultid对应的alrmmid个数 */
    /*
     * 最大支持96个PID（alarmid）
     * ulPid：4 byte， 故障faultid对应的pid
     * ulAlarmId：4 byte，故障faultid对应
     */
    CHR_ALARM aucAlarmMap[NV_CHR_PID_MAX_NUM];
} NV_ID_CHR_FAULTID_CONFIG_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
