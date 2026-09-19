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


#ifndef __SYS_NV_DEF_H__
#define __SYS_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "nv_id_sys.h"
/*lint --e{761}*/
#ifndef __u8_defined
#define __u8_defined
typedef signed char          s8;
typedef unsigned char        u8;
typedef signed short         s16;
typedef unsigned short       u16;
typedef signed int           s32;
typedef unsigned int         u32;
typedef signed long long     s64;
typedef unsigned long long   u64;
#endif

#define NV_ITEM_IMEI_SIZE 16

typedef struct
{
    u8                           aucImei[NV_ITEM_IMEI_SIZE];
}IMEI_STRU;

/*****************************************************************************
 结构名    : RESUME_FLAG_STRU
 结构说明  : RESUME_FLAG结构
*****************************************************************************/
typedef struct
{
    u16   usResumeFlag; /*Range:[0, 1]*/
}RESUME_FLAG_STRU;

/*****************************************************************************
 结构名    : LED_CONTROL_NV_STRU
 结构说明  : LED_CONTROL_NV结构 ID=7
*****************************************************************************/
typedef struct
{
    u8   ucLedColor;      /*三色灯颜色，对应LED_COLOR的值*/
    u8   ucTimeLength;    /*该配置持续的时间长度，单位100ms*/
}LED_CONTROL_NV_STRU;

/*****************************************************************************
 结构名    : LED_CONTROL_STRU
 结构说明  : LED_CONTROL结构
*****************************************************************************/
typedef struct
{
    LED_CONTROL_NV_STRU   stLED[10];
}LED_CONTROL_STRU;

/*****************************************************************************
 结构名    : LED_CONTROL_STRU_ARRAY
 结构说明  : 三色灯状态序列单元结构
*****************************************************************************/
typedef struct
{
    LED_CONTROL_STRU    stLED_Control[32];
}LED_CONTROL_STRU_ARRAY;

typedef struct
{
    u16 major_ver;
    u16 minor_ver;
}NV_VERSION_NO_STRU;

/*****************************************************************************
 结构名    : USIM_TEMP_SENSOR_TABLE
 结构说明  : USIM_TEMP_SENSOR_TABLE结构
*****************************************************************************/
typedef struct
{
    s16   Temperature;
    u16   Voltage;
}USIM_TEMP_SENSOR_TABLE;

/*****************************************************************************
 结构名    : USIM_TEMP_SENSOR_TABLE_STRU
 结构说明  : USIM_TEMP_SENSOR_TABLE结构
*****************************************************************************/
typedef struct
{
    USIM_TEMP_SENSOR_TABLE UsimTempSensorTable[19];
}USIM_TEMP_SENSOR_TABLE_STRU;

/*****************************************************************************
 结构名    : NV_AT_SHELL_OPEN_FLAG_STRU
 结构说明  : NV_AT_SHELL_OPEN_FLAG结构 ID=33
*****************************************************************************/
typedef struct
{
    u32    NV_AT_SHELL_OPEN_FLAG;
}NV_AT_SHELL_OPEN_FLAG_STRU;

/*****************************************************************************
 结构名    : NV_NPNP_CONFIG_INFO
 结构说明  : NV_NPNP_CONFIG_INFO ID=67
*****************************************************************************/
typedef struct
{
    u32 npnp_open_flag;                           /* NPNP 特性一级NV开关, 0为未开启不可以使用, 1为开启可以使用 */
    u32 npnp_enable_flag;                         /* NPNP 特性二级NV项, 0为特性未使能, 1为特性使能             */
}NV_NPNP_CONFIG_INFO;

/*****************************************************************************
 结构名    : NV_FACTORY_INFO_I_STRU
 结构说明  : NV_FACTORY_INFO_I结构 ID=114
*****************************************************************************/
#define NV_FACTORY_INFO_I_SIZE    (78)
typedef struct
{
    u8 aucFactoryInfo[NV_FACTORY_INFO_I_SIZE];
}NV_FACTORY_INFO_I_STRU;

/*****************************************************************************
 结构名    : NV_SCI_CFG_STRU
 结构说明  : NV_SCI_CFG结构 ID=128
*****************************************************************************/
typedef struct
{
    u32 value;
} NV_SCI_CFG_STRU;


typedef struct
{
    u8 ucAlmStatus; /* 告警状态,默认0:close;1:open  */
    u8 ucAlmLevel;  /* 故障&告警级别*/
    u8 aucReportBitMap[2];
}NV_ID_ERR_LOG_CTRL_INFO_STRU;


typedef struct
{
    u32                          ulAlarmid;        /* 告警标识 */
    u32                          ulAlarmidDetail;  /* 告警相关性，32bit，每个bit代表一种相关性，0代表和该相关性无关 */
}OM_ALARM_ID_DETAIL_STRU;


typedef struct
{
    OM_ALARM_ID_DETAIL_STRU          astOmAlarmidRelationship[40]; /* 预设40组 */
}NV_ALARM_ID_RELATIONSHIP_STRU;


typedef struct
{
    u32                          ulFTMDetail; /* 工程模式相关性，32bit，每个bit代表一种相关性，0代表和该相关性无关 */
}NV_ID_FTM_DETAIL_STRU;

/*****************************************************************************
 结构名    : NV_THERMAL_TSENSOR_CONFIG_STRU
 结构说明  : NV_THERMAL_TSENSOR_CONFIG结构 ID=9213
*****************************************************************************/
typedef struct
{
    u32       enable;
    u32       lagvalue0;
    u32       lagvalue1;
    u32       thresvalue0;
    u32       thresvalue1;
    u32       rstthresvalue0;
    u32       rstthresvalue1;
    u32       alarmcount1;
    u32       alarmcount2;
    u32       resumecount;
    u32       acpumaxfreq;
    u32       gpumaxfreq;
    u32       ddrmaxfreq;
    u32       reserved[4];
}NV_THERMAL_TSENSOR_CONFIG_STRU;


/*****************************************************************************
 结构名    : NV_THERMAL_BAT_CONFIG_STRU
 结构说明  : NV_THERMAL_BAT_CONFIG结构 ID=9214
*****************************************************************************/
typedef struct
{
    u16       enable;
    u16       hkadcid;
    s16       highthres;
    u16       highcount;
    s16       lowthres;
    u16       lowcount;
    u32       reserved[2];

}NV_THERMAL_BAT_CONFIG_STRU;

/*****************************************************************************
 结构名    : NV_THERMAL_HKADC_CONFIG
 结构说明  : NV_THERMAL_HKADC_CONFIG结构 ID=9232
*****************************************************************************/
typedef struct
{
    u16 hkadc[32];
}NV_KADC_CHANNEL_CFG_STRU;

typedef struct
{
    u32   outconfig;
    u16   outperiod;
    u16   convertlistlen;
    u32   reserved[2];
}NV_THERMAL_HKADC_CONFIG;

/*****************************************************************************
 结构名    : NV_THERMAL_HKADC_CONFIG_STRU
 结构说明  : NV_THERMAL_HKADC_CONFIG结构 ID=9215
*****************************************************************************/
typedef struct
{
    NV_THERMAL_HKADC_CONFIG   hkadcCfg[14];
}NV_THERMAL_HKADC_CONFIG_STRU;

/*****************************************************************************
 结构名    : NV_KADC_PHYTOLOGICAL_CONFIGTCXO_CFG_STRU
 结构说明  : NV_KADC_PHYTOLOGICAL_CONFIGTCXO_CFG结构 ID=9216
*****************************************************************************/
typedef struct
{
    u16 hkadc[14];
}NV_KADC_PHYTOLOGICAL_CONFIGTCXO_CFG_STRU;

/*****************************************************************************
 结构名    : NV_TCXO_CFG_STRU
 结构说明  : NV_TCXO_CFG结构 ID=9217
*****************************************************************************/
typedef struct
{
    u32 tcxo_cfg;
}NV_TCXO_CFG_STRU;


typedef struct
{
    u8  enUartEnableCfg;
    u8  uartRsv[3];
}NV_UART_SWITCH_STRU;

/*****************************************************************************
 结构名    : NV_HUAWEI_PCCW_HS_HSPA_BLUE_STRU
 结构说明  : NV_HUAWEI_PCCW_HS_HSPA_BLUE结构 ID=50032
*****************************************************************************/
typedef struct
{
    u32  NVhspa_hs_blue; /*Range:[0,3]*/
}NV_HUAWEI_PCCW_HS_HSPA_BLUE_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

