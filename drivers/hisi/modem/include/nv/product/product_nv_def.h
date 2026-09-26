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


#ifndef __PRODUCT_NV_DEF_H__
#define __PRODUCT_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_SINGLE_GUC_BAND_CHECK_NUM 28
#define MAX_SINGLE_TL_BAND_CHECK_NUM 12
#define MAX_NV_GUC_CHECK_ITEM_NUM 32
#define MAX_NV_TL_CHECK_ITEM_NUM 64
#define NV_CHECK_HASH_LEN 32

/* 请参考结构体样例 */
typedef struct {
    int reserved; /* note */
} PRODUCT_MODULE_STRU;

typedef struct {
    unsigned int uwGucCrcResult;   /* GUc主卡NV的校验结果 */
    unsigned int uwTlCrcResult;    /* TL主卡NV的校验结果 */
    unsigned int uwGucM2CrcResult; /* GUC副卡NV的校验结果 */
} NV_CRC_CHECK_RESULT_STRU;

typedef struct {
    unsigned short uhwTransmitMode;                                /* 指示该组的发送模式 */
    unsigned short uhwBand;                                        /* 指示该组的BAND号 */
    unsigned short uhwEnable;                                      /* 指示是否有效 */
    unsigned short uhwValidCount;                                  /* 指示有效数据的个数 */
    unsigned short auhwNeedCheckID[MAX_SINGLE_GUC_BAND_CHECK_NUM]; /* 保存需要检查的NV_ID值 */
} SINGLE_GUC_BAND_NV_ID_STRU;

typedef struct {
    unsigned short uhwTransmitMode;                               /* 指示该组的发送模式 */
    unsigned short uhwBand;                                       /* 指示该组的BAND号 */
    unsigned short uhwEnable;                                     /* 指示是否有效 */
    unsigned short uhwValidCount;                                 /* 指示有效数据的个数 */
    unsigned short auhwNeedCheckID[MAX_SINGLE_TL_BAND_CHECK_NUM]; /* 保存需要检查的NV_ID值 */
} SINGLE_TL_BAND_NV_ID_STRU;

typedef struct {
    SINGLE_GUC_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_GUC_CHECK_ITEM_NUM]; /* 保存需要检查的NV_ID值 */
} NV_GUC_CHECK_ITEM_STRU;

typedef struct {
    SINGLE_TL_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_TL_CHECK_ITEM_NUM]; /* 保存需要检查的NV_ID值 */
} NV_TL_CHECK_ITEM_STRU;

typedef struct {
    unsigned short uhwWorkStationNum;
    unsigned short auhwNV50001InsertPoint[0x5];
    unsigned short auhwNV50002InsertPoint[0x5];
    unsigned short auhwNV50003InsertPoint[0x5];
    unsigned int rsv;
} NV_WORKSTATION_INFO_STRU;

typedef struct {
    NV_CRC_CHECK_RESULT_STRU astCRCResultTemp[0x6];
} NV_CRCRESULT_TEMP_STRU;

typedef struct {
    unsigned char astGucHashResult[NV_CHECK_HASH_LEN];   /* GUC主卡NV的校验结果 */
    unsigned char astTlHashResult[NV_CHECK_HASH_LEN];    /* TL主卡NV的校验结果 */
    unsigned char astGucM2HashResult[NV_CHECK_HASH_LEN]; /* GUC副卡NV的校验结果 */
} NV_HASH_CHECK_RESULT_STRU;

typedef struct {
    NV_HASH_CHECK_RESULT_STRU astHashResultTemp[0x6];
} NV_HASH_RESULT_TEMP_STRU;

/* ****************************************************************************
 结构名    : CHG_BATTERY_LOW_TEMP_PROTECT_NV
 结构说明  : CHG_BATTERY_LOW_TEMP_PROTECT_NV结构 ID=52005
**************************************************************************** */
typedef struct {
    unsigned int ulIsEnable;
    signed int lCloseAdcThreshold;
    unsigned int ulTempLowCount;
} CHG_BATTERY_LOW_TEMP_PROTECT_NV;


typedef struct {
    unsigned int ulIsEnable;
    signed int lCloseADCHold;
    unsigned int ulTempOverMax;
} OM_BATTREY_TEMP_CFG_STRU;

/* ****************************************************************************
 结构名    : nvi_cust_pid_type
 结构说明  : nvi_cust_pid_type结构
**************************************************************************** */
typedef struct {
    unsigned int nv_status;
    unsigned short cust_first_pid;
    unsigned short cust_rewind_pid;
} nvi_cust_pid_type;

/* ****************************************************************************
 结构名    : NV_WEBNAS_SD_WORKMODE_STRU
 结构说明  : NV_WEBNAS_SD_WORKMODE结构
**************************************************************************** */
typedef struct {
    unsigned int ulSDWorkMode;
} NV_WEBNAS_SD_WORKMODE_STRU;

/* ****************************************************************************
 结构名    : NV_OLED_TEMP_ADC_STRU
 结构说明  : NV_OLED_TEMP_ADC结构
**************************************************************************** */
typedef struct {
    signed short sTemp;
    signed short sADC;
} NV_OLED_TEMP_ADC_STRU;

/* ****************************************************************************
 结构名    : NV_OLED_TEMP_ADC_STRU_ARRAY
 结构说明  : NV_OLED_TEMP_ADC_STRU_ARRAY结构  ID=49
**************************************************************************** */
typedef struct {
    NV_OLED_TEMP_ADC_STRU stNV_OLED_TEMP_ADC[0x1c];
} NV_OLED_TEMP_ADC_STRU_ARRAY;

/* ****************************************************************************
 结构名    : TEMP_ADC_STRU
 结构说明  : TEMP_ADC结构
**************************************************************************** */
typedef struct {
    signed short sTemp;
    unsigned short usADC;
} TEMP_ADC_STRU;

/* ****************************************************************************
 结构名    : TEMP_ADC_STRU_ARRAY
 结构说明  : TEMP_ADC_STRU_ARRAY结构
**************************************************************************** */
typedef struct {
    TEMP_ADC_STRU stTempAdc[0x1c];
} TEMP_ADC_STRU_ARRAY;

/* ****************************************************************************
 结构名    : USB_SN_NV_INFO_STRU
 结构说明  : USB_SN_NV_INFO结构
**************************************************************************** */
typedef struct {
    unsigned int usbSnNvStatus;
    unsigned int usbSnNvSucFlag;
    unsigned int usbSnReserved1;
} USB_SN_NV_INFO_STRU;

/* ****************************************************************************
 结构名    : LIVE_TIME_STRU
 结构说明  : LIVE_TIME结构
**************************************************************************** */
typedef struct {
    unsigned int ulLiveTime;
} LIVE_TIME_STRU;

/* ****************************************************************************
 结构名    : LIVE_TIME_CONTROL_STRU
 结构说明  : LIVE_TIME_CONTROL结构
**************************************************************************** */
typedef struct {
    unsigned int ulEnable;
    unsigned int ulCycle;
} LIVE_TIME_CONTROL_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
