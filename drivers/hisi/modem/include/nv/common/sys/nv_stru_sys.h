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

#ifndef __SYS_NV_DEF_H__
#define __SYS_NV_DEF_H__

#include "nv_id_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*lint --e{761}*/
#define NV_ITEM_IMEI_SIZE 16
/*
 * 结构名    : IMEI_STRU
 * 结构说明  : IMEI（International Mobile station Equipment Identity），前15bytes存放IMEI，
 * 前14位是TAC（Type Allocation Code）和SNR（Serial Number），第15位为校验位，第16位是保留位。
 * 每个设备的IMEI都是唯一的，详细信息参见协议23003 6.2。
 */
typedef struct {
    unsigned char aucImei[NV_ITEM_IMEI_SIZE]; /* IMEI */
} IMEI_STRU;

/*
 * 结构名    : RESUME_FLAG_STRU
 * 结构说明  : 不再使用。
 */
typedef struct {
    unsigned short usResumeFlag; /* Range:[0, 1] */
} RESUME_FLAG_STRU;

/*
 * 结构名    : LED_CONTROL_NV_STRU
 * 结构说明  : LED_CONTROL_NV结构 ID=7
 */
typedef struct {
    unsigned char ucLedColor;   /* 三色灯颜色，对应LED_COLOR的值 */
    unsigned char ucTimeLength; /* 该配置持续的时间长度，单位100ms */
} LED_CONTROL_NV_STRU;

/*
 * 结构名    : LED_CONTROL_STRU
 * 结构说明  : LED_CONTROL结构
 */
typedef struct {
    LED_CONTROL_NV_STRU stLED[0xa];
} LED_CONTROL_STRU;

/*
 * 结构名    : LED_CONTROL_STRU_ARRAY
 * 结构说明  : 三色灯状态序列单元结构
 */
typedef struct {
    LED_CONTROL_STRU stLED_Control[0x20];
} LED_CONTROL_STRU_ARRAY;

/* 结构说明  : 实现NV版本管理。 */
typedef struct {
    unsigned short major_ver; /* NV主版本，由HISI定义 */
    unsigned short minor_ver; /* NV次版本，由产品定制，不能超过主版本号 */
} NV_VERSION_NO_STRU;

/*
 * 结构名    : USIM_TEMP_SENSOR_TABLE
 * 结构说明  : USIM_TEMP_SENSOR_TABLE结构
 */
typedef struct {
    signed short Temperature;
    unsigned short Voltage;
} USIM_TEMP_SENSOR_TABLE;

/*
 * 结构名    : USIM_TEMP_SENSOR_TABLE_STRU
 * 结构说明  : USIM_TEMP_SENSOR_TABLE结构
 */
typedef struct {
    USIM_TEMP_SENSOR_TABLE UsimTempSensorTable[0x13];
} USIM_TEMP_SENSOR_TABLE_STRU;

/*
 * 结构名    : NV_AT_SHELL_OPEN_FLAG_STRU
 * 结构说明  : 此NV项保存SHELL口的开关状态。
 */
typedef struct {
    /*
     * 每次单板启动后SHELL的状态。
     * 0：SHELL口用于WIFI校准；
     * 1：SHELL口关闭；
     * 2：SHELL口打开，此时可通过SHELL口查看调试信息。
     */
    unsigned int NV_AT_SHELL_OPEN_FLAG;
} NV_AT_SHELL_OPEN_FLAG_STRU;

/*
 * 结构名    : NV_NPNP_CONFIG_INFO
 * 结构说明  : NV_NPNP_CONFIG_INFO ID=67
 */
typedef struct {
    unsigned int npnp_open_flag;   /* NPNP 特性一级NV开关, 0为未开启不可以使用, 1为开启可以使用 */
    unsigned int npnp_enable_flag; /* NPNP 特性二级NV项, 0为特性未使能, 1为特性使能             */
} NV_NPNP_CONFIG_INFO;

/*
 * 结构名    : NV_FACTORY_INFO_I_STRU
 * 结构说明  : NV_FACTORY_INFO_I结构 ID=114
 */
#define NV_FACTORY_INFO_I_SIZE (78)
/* 结构说明  : PCB版本和工位信息，试制时写入。 */
typedef struct {
    unsigned char aucFactoryInfo[NV_FACTORY_INFO_I_SIZE];
} NV_FACTORY_INFO_I_STRU;

/*
 * 结构名    : NV_ID_ERR_LOG_CTRL_INFO_STRU
 * 对应NVID: 133 NV_ID_ERR_LOG_CTRL_INFO
 * 结构说明  : 设置告警状态和级别。
    ucAlmLevel:Warning：0x04代表提示，Minor：0x03代表次要, Critical：0x01代表紧急（默认）
                        值为0x03， 0x03/0x02/0x01都上报
    aucReportBitMap:A0000000 00000DCB
                    A代表主动上报开关
                    B代表语音质量特性开关
                    C代表搜网特性故障主动上报开关
                    D代表RAT频繁切换主动上报开关
 */
typedef struct {
    /*
     * 告警状态。
     * 0：故障记录关闭；
     * 1：故障记录开启。
     */
    unsigned char ucAlmStatus;
    /*
     * 故障和告警级别。
     * 1：紧急；2：重要；3：次要；4：提示
     * 设置为某一级别，比该级别高的告警，都需要记录。
     * 例如：0x03时，0x03/0x02/0x01都上报。
     */
    unsigned char ucAlmLevel;
    unsigned char aucReportBitMap[0x2];
} NV_ID_ERR_LOG_CTRL_INFO_STRU;

/*
 * 结构名    : OM_ALARM_ID_DETAIL_STRU
 */
typedef struct {
    unsigned int ulAlarmid; /* 故障场景，取值范围0~39，最多40组。 */
    unsigned int
        ulAlarmidDetail; /* 故障告警相关性，32bit，每个bit代表一种相关性，取值范围0~0xFFFFFFFF，0代表和该相关性无关。
                          */
} OM_ALARM_ID_DETAIL_STRU;

#define NV_ALARM_ID_NUM 40
/*
 * 结构名    : NV_ALARM_ID_RELATIONSHIP_STRU
 * 对应NVID: 134 NV_ALARM_ID_RELATIONSHIP
 * 结构说明  : 故障告警相关性。
 * 例：故障上报 MODEM 0相关PID列表：
 * 假设故障0 场景和PID：I0_WUEPS_PID_MMC 和I0_DSP_PID_APM相关，ulAlarmidDetail对应bit 为设置为1，NV项值设置成：
 * 故障上报MODEM 1相关PID列表：
 * 相关配置同MODEM 0。

*/
typedef struct {
    OM_ALARM_ID_DETAIL_STRU astOmAlarmidRelationship[NV_ALARM_ID_NUM]; /* 预设40组 */
} NV_ALARM_ID_RELATIONSHIP_STRU;

/*
 * 结构名    : NV_ID_FTM_DETAIL_STRU
 * 对应NVID: 135 NV_ID_FTM_DETAIL
 * 结构说明  : 工程模式上报组件相关性。
 * 工程模式上报 MODEM
 * 0对应的PID列表：需要对应PID上报工程模式，把对应NV项值bit位改为1；例：需要PID：I0_WUEPS_PID_MM上报工程模式相关数据把NV项值设置成0x02。
 * 工程模式上报 MODEM 1对应的PID列表：配置同MODEM 0。
 */
typedef struct {
    unsigned int ulFTMDetail; /* 工程模式相关性，32bit，每个bit代表一种相关性，0代表和该相关性无关 */
} NV_ID_FTM_DETAIL_STRU;

/*
 * 结构名    : NV_THERMAL_TSENSOR_CONFIG_STRU
 * 结构说明  : NV_THERMAL_TSENSOR_CONFIG结构 ID=9213
 */
typedef struct {
    unsigned int enable;
    unsigned int lagvalue0;
    unsigned int lagvalue1;
    unsigned int thresvalue0;
    unsigned int thresvalue1;
    unsigned int rstthresvalue0;
    unsigned int rstthresvalue1;
    unsigned int alarmcount1;
    unsigned int alarmcount2;
    unsigned int resumecount;
    unsigned int acpumaxfreq;
    unsigned int gpumaxfreq;
    unsigned int ddrmaxfreq;
    unsigned int reserved[0x4];
} NV_THERMAL_TSENSOR_CONFIG_STRU;


/*
 * 结构名    : NV_THERMAL_BAT_CONFIG_STRU
 * 结构说明  : NV_THERMAL_BAT_CONFIG结构 ID=9214
 */
typedef struct {
    unsigned short enable;
    unsigned short hkadcid;
    signed short highthres;
    unsigned short highcount;
    signed short lowthres;
    unsigned short lowcount;
    unsigned int reserved[0x2];
} NV_THERMAL_BAT_CONFIG_STRU;

/*
 * 结构名    : NV_THERMAL_HKADC_CONFIG
 * 结构说明  : NV_THERMAL_HKADC_CONFIG结构 ID=9232
 */
typedef struct {
    unsigned short hkadc[0x20];
} NV_KADC_CHANNEL_CFG_STRU;

typedef struct {
    unsigned int outconfig;
    unsigned short outperiod;
    unsigned short convertlistlen;
    unsigned int reserved[0x2];
} NV_THERMAL_HKADC_CONFIG;

/*
 * 结构名    : NV_THERMAL_HKADC_CONFIG_STRU
 * 结构说明  : NV_THERMAL_HKADC_CONFIG结构 ID=9215
 */
typedef struct {
    NV_THERMAL_HKADC_CONFIG hkadcCfg[0xe];
} NV_THERMAL_HKADC_CONFIG_STRU;

/*
 * 结构名    : NV_KADC_PHYTOLOGICAL_CONFIGTCXO_CFG_STRU
 * 结构说明  : NV_KADC_PHYTOLOGICAL_CONFIGTCXO_CFG结构 ID=9216
 */
typedef struct {
    unsigned short hkadc[0xe];
} NV_KADC_PHYTOLOGICAL_CONFIGTCXO_CFG_STRU;

/*
 * 结构名    : NV_TCXO_CFG_STRU
 * 结构说明  : NV_TCXO_CFG结构 ID=9217
 */
typedef struct {
    unsigned int tcxo_cfg;
} NV_TCXO_CFG_STRU;

/*
 * 结构名    : NV_UART_SWITCH_STRU
 * 结构说明  : 用于记录UART功能是否处于使能状态 ID=9239
 */
typedef struct {
    unsigned char enUartEnableCfg;
    unsigned char uartRsv[0x3];
} NV_UART_SWITCH_STRU;

/*
 * 结构名    : NV_HUAWEI_PCCW_HS_HSPA_BLUE_STRU
 * 结构说明  : NV_HUAWEI_PCCW_HS_HSPA_BLUE结构 ID=50032
 */
typedef struct {
    unsigned int NVhspa_hs_blue; /* Range:[0,3] */
} NV_HUAWEI_PCCW_HS_HSPA_BLUE_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
