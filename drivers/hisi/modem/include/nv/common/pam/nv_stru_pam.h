/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef __NV_STRU_PAM_H__
#define __NV_STRU_PAM_H__

#include "vos.h"
#include "nv_id_pam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define ESN_NV_DATA_LEN (8)
#define MEID_NV_DATA_LEN (8)

/*
 * 功能说明: 物理端口号枚举值
 */
enum AT_PHY_Port {
    AT_UART_PORT = 0,
    AT_PCUI_PORT,
    AT_CTRL_PORT,
    AT_HSUART_PORT,
    AT_PCUI2_PORT,
    AT_PORT_BUTT
};
typedef VOS_UINT32 AT_PHY_PortUint32;

enum {
    CPM_IND_PORT = AT_PORT_BUTT, /* OM数据上报端口 */
    CPM_CFG_PORT,                /* OM配置端口 */
    CPM_SD_PORT,
    CPM_WIFI_OM_IND_PORT, /* WIFI下OM数据上报端口 */
    CPM_WIFI_OM_CFG_PORT, /* WIFI下OM配置下发端口 */
    CPM_WIFI_AT_PORT,     /* WIFI下AT端口 */
    CPM_HSIC_IND_PORT,
    CPM_HSIC_CFG_PORT,
    CPM_VCOM_IND_PORT, /* VCOM上OM数据上报接口 */
    CPM_VCOM_CFG_PORT, /* VCOM上OM配置接口 */
    CPM_FS_PORT,
    CPM_PORT_BUTT
};
typedef VOS_UINT32 CPM_PHY_PortUint32;


enum {
    CPM_CBT_PORT_USB,
    CPM_CBT_PORT_VCOM,
    CPM_CBT_PORT_SOCKET,
    CBP_CBT_PORT_BUTT
};
typedef VOS_UINT32 CPM_CBT_PortUint32;

enum {
    CPM_OM_PORT_TYPE_USB,
    CPM_OM_PORT_TYPE_VCOM,
    CPM_OM_PORT_TYPE_WIFI,
    CPM_OM_PORT_TYPE_SD,
    CPM_OM_PORT_TYPE_FS,
    CPM_OM_PORT_TYPE_HSIC,
    CBP_OM_PORT_TYPE_BUTT
};
typedef VOS_UINT32 CPM_OM_PortUint32;


enum ESN_MEID_Flag {
    ESN_MEID_DISABLE     = 0,
    ESN_ME_ENABLE_FLAG   = 1,
    MEID_ME_ENABLE_FLAG  = 2,
    ESN_MEID_ENABLE_BOTH = 3,
    ESN_MEID_ENABLE_FLAG_BUTT
};
typedef VOS_UINT8 ESN_MEID_FlagUint8;


typedef struct {
    /*
     * OM使用的通道。当前仅USB和VCOM生效。
     * 0：USB
     * 1：VCOM
     * 2：WIFI
     * 3：SD卡
     * 4：文件系统
     * 5：HSIC
     */
    CPM_OM_PortUint32   portNum;
    /*
     * 校准使用的通道。
     * 0：USB
     * 1：VCOM
     * 2：WIFI,wifi 特性因为安全原因默认不编译，需要打开特性宏重新编译版本
     */
    CPM_CBT_PortUint32  cbtPortNum;
} OM_ChannlePortCfg;


typedef struct {
    /*
     * ESN或MEID是否有效标记：
     * 0：ESN和MEID都无效；
     * 1：仅ESN有效；
     * 2：仅MEID有效；
     * 3：ESN和MEID都有效；
     */
    ESN_MEID_FlagUint8       esnMeIDFlag;
    VOS_UINT8                rsv;  /* 预留字节 */
    VOS_UINT8                esn[ESN_NV_DATA_LEN];   /* ESN内容，第一个字节表示长度信息； */
    VOS_UINT8                meId[MEID_NV_DATA_LEN]; /* MEID内容，第一个字节表示长度信息； */
    VOS_UINT8                rfu[6];  /* 预留字节 */
} NV_ESN_Meid;


typedef VOS_UINT32 SI_PB_StorateTypeUint32;
/* 结构说明  : USIM卡电话本控制信息。 */
typedef struct {
    /*
     * 0：未限定类型；
     * 1：SM；
     * 2：ME；
     * 3：SM和ME均可；
     * 4：ON；
     * 5：FD；
     * 6：EN；
     * 7：BD。
     * 此参数目前不使用。
     */
    SI_PB_StorateTypeUint32 curMaterial;
    VOS_UINT8               passwords[8];  /* 存储器密码，目前不使用。 */
    VOS_UINT16              used;  /* ME存储器已使用记录数，目前不使用。 */
    VOS_UINT16              total;  /* ME存储器最大支持记录数，目前不使用。 */
    VOS_UINT8               anrMaxNum; /* 扩展电话本支持的ANR文件的最大个数。最大支持3个，最小支持1个。 */
    /*
     * 扩展电话本是否支持EMAIL文件。
     * 0：不支持；
     * 1：支持。
     */
    VOS_UINT8               emailFlag;
    /*
     * 是否开启扩展电话本功能。
     * 0：不开启；
     * 1：开启。
     */
    VOS_UINT8               spbFlag;
    VOS_UINT8               rsv;  /* 保留。 */
} SI_PB_NVCtrlInfo;


typedef struct {
    VOS_UINT8 profileLen;  /* 长度指示。 */
    VOS_UINT8 profile[47];  /* profile内容。 */
} USIMM_UsimProfile;

typedef struct {
    VOS_UINT8 profileLen;  /* 长度指示。 */
    VOS_UINT8 profile[31];  /* profile内容。 */
} USIMM_SimProfile;

/* 结构说明  : 用于设置STK功能相关参数。 */
typedef struct {
    VOS_UINT8               funcEnable;  /* STK是否激活。 */
    VOS_UINT8               timer;  /* 默认的STK超时时间。 */
    VOS_UINT8               terminalType;  /* 终端类型，暂未使用。 */
    VOS_UINT8               rsv;  /* 预留字节，对齐用。 */
    USIMM_UsimProfile       usimProfile;  /* USIM Profile配置信息 */
    USIMM_SimProfile        simProfile;  /* SIM Profile配置信息 */
} USIMM_STK_CFG_STRU;

typedef USIMM_STK_CFG_STRU USIMM_StkCfg;


typedef struct {
    /*
     * bit[0-2] : 支持SCI的个数。
     * bit[3-7] : 保留。
     * bit[8-10] : modem 0使用的SCI接口，0：SCI0 1：SCI1；2：SCI2；其余值:多卡，暂不支持 。
     * bit[11-13] : modem 0使用的SCI接口，0：SCI0 1：SCI1；2：SCI2；其余值:多卡，暂不支持 。
     * bit[14-16] : modem 0使用的SCI接口，0：SCI0 1：SCI1；2：SCI2；其余值:多卡，暂不支持 。
     * bit[17-31] : 保留 。
     */
    VOS_UINT32 value;
} NV_SCI_Cfg;

enum {
    STK_CMD_NONEED_REPORT_APP = 0,
    STK_CMD_NEED_REPORT_APP = 1,
    STK_CMD_REPORT_APP_BUTT
};

/*
 * 功能说明: NV_STK_CmdReportAppCfg结构
 * 结构说明: 控制卡上报的主动命令是否需要通知给APP
 */
typedef struct {
    VOS_UINT32 value; /* 0:默认值，不需要上报APP；1:需要上报APP */
} NV_STK_CmdReportAppCfg;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
