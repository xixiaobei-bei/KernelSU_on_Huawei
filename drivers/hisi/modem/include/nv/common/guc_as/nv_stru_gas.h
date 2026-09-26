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

#ifndef __NV_STRU_GAS_H__
#define __NV_STRU_GAS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 2)

#include "vos.h"
#include "nv_id_guas.h"

#define NVIM_TDS_MAX_SUPPORT_BANDS_NUM 8

enum NVIM_TdsFreqBandList {
    ID_NVIM_TDS_FREQ_BAND_A = 0x01, /* 频点范围: 9504~9596  10054~10121 */
    ID_NVIM_TDS_FREQ_BAND_B = 0x02, /* 频点范围: 9254~9546  9654~9946 */
    ID_NVIM_TDS_FREQ_BAND_C = 0x04, /* 频点范围: 9554~9646 */
    ID_NVIM_TDS_FREQ_BAND_D = 0x08, /* 频点范围: 12854~13096 */
    ID_NVIM_TDS_FREQ_BAND_E = 0x10, /* 频点范围: 11504~11996 */
    ID_NVIM_TDS_FREQ_BAND_F = 0x20, /* 频点范围: 9404~9596 */
    ID_NVIM_TDS_FREQ_BAND_BUTT
};
typedef VOS_UINT8 NVIM_TdsFreqBandListUint8;


/*
 * 结构说明: 标识MS的EGPRS多时隙能力等级。参见协议24008下的10.1.5.7。
 */
typedef struct {
    /*
     * 0x0000：MS的EGPRS多时隙能力等级为0x000C；
     * 0x0001～0x000C：MS的EGPRS多时隙能力等级。
     * 范围： [0, 12]
     */
    VOS_UINT16 egprsMultiSlotClass;
    VOS_UINT8  rsv[2];
} NVIM_EgprsMultiSlotClass;

/*
 * 结构说明: MS是否支持EGPRS。该NV可以针对每个Modem单独配置。
 */
typedef struct {
    /*
     * 0x0000：不支持EGPRS；
     * 0x0001：支持EGPRS。
     */
    VOS_UINT16                          egprsFlag;
    VOS_UINT8                           rsv[2];  /* 预留 */
} NVIM_EgprsFlag;

/*
 * 结构说明: 表示GPRS Active Timer的长度。GPRS Active Timer运行过程中，不向GDSP发起背景搜索。
 */
typedef struct {
    /*
     * 0x0000_0000：启动GPRS Active Timer，GPRS Active Timer 的长度为100ms；
     * 0x0000_0001：GPRS Active Timer的长度为100ms；
     * 0x0000_0002：GPRS Active Timer的长度为200ms。
     * 以此类推。
     */
    VOS_UINT32                          gprsActiveTimerLength;
} NVIM_GprsActiveTimerLen;

/*
 * 结构说明: 标识MS的GPRS多时隙能力等级。
 */
typedef struct {
    /*
     * 0x0000：当取值为0x0000时MS的GPRS多时隙能力等级为0x000C。
     * 0x0001~0x000C：MS的GPRS多时隙能力等级。
     */
    VOS_UINT16                          gprsMultiSlotClass;
    VOS_UINT8                           rsv[2];  /* 预留 */
} NVIM_GprsMultiSlotClass;

/*
 * 结构说明: MS是否支持multislot classe 33。High Multislot Capability具体定义参见协议24008-10.5.1.7。
 */
typedef struct {
    /*
     * 0：High multislot class无效；
     * 1：High multislot class有效。
     */
    VOS_UINT16                              highMultislotClassFlg;
    /*
     * 0：High Multislot Class等级为0，目前仅支持设置为该值。
     * 该NV项需要和Gprs_MultiSlotClass或者Egprs_MultiSlotClass NV项联合使用。当High multislot class有效，High Multislot
     * Class等级为0并且Gprs_MultiSlotClass等级为12或者Egprs_MultiSlotClass为12时，表示MS支持多时隙能力等级33。
     */
    VOS_UINT16                              highMultislotClass;
} NVIM_GAS_HighMultiSlotClass;

/*
 * 结构说明: TDS支持的频段个数以及频段号。
 */
typedef struct {
    /* 支持的TDS频段个数，目前最大支持三个频段（A/E/F），不支持B/C/D这三个频段。取值范围0~6。 */
    VOS_UINT8                               bandCnt;
    VOS_UINT8                               rsv[3];  /* 预留 */
    /*
     * aucBandNo[x]表示支持的TDS频段号，譬如：
     * aucBandNo[0]=0x01, aucBandNo[1]=0x10,
     * aucBandNo[2]=0x20。
     */
    VOS_UINT8                               bandNo[NVIM_TDS_MAX_SUPPORT_BANDS_NUM];
} NVIM_UeTdsSupportFreqBandList;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

/* end of nv_stru_gas.h */
