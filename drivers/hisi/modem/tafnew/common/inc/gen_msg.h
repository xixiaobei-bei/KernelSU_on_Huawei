/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#ifndef __GEN_MSG_H__
#define __GEN_MSG_H__

#include "aps_l4a_interface.h"
#include "at_common.h"
#include "vos.h"
#include "mdrv.h"
#include "app_rrc_interface.h"
#include "app_rrc_lte_common_interface.h"
#include "app_l2_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma pack(push, 4)

#define MSP_UPGRADE_DLOAD_VER_LEN 30
#define MSP_UPGRADE_DLOAD_SOFTWARE_LEN 30
#define MSP_UPGRADE_DLOAD_ISO_LEN 100
#define MSP_UPGRADE_DLOAD_PRODUCTID_LEN 30

#define L4A_MAX_IPV6_PREFIX_NUM 6 /* 最大IPv6地址前缀数量 */

/*
 * MSP 内部的消息定义
 */

#if (VOS_OS_VER == VOS_WIN32)
#pragma warning(disable : 4200) /* disable nonstandard extension used : zero-sized array in struct/union. */
#pragma warning(disable : 4214) /* allows bitfield structure members to be of any integral type. */
#endif

/*
 * MUX相关接口
 * (0x00010000,0x00010050]
 */
#define ID_MSG_DIAG_CMD_REQ 0x00010001
#define ID_MSG_DIAG_CMD_CNF 0x00010002

/*
 * AT模块消息接口
 * [0x00010051,0x00010100)
 */

/*
 * DIAG 消息接口
 * [0x00010101,0x00010120)
 */
#define ID_MSG_DIAG_DRA_DSP_REQ 0x00010101
#define ID_MSG_DIAG_DRA_DSP_CNF 0x00010102 /* HAL_DRA --> DIAG */
#define ID_MSG_DIAG_TIMER_DSPTIMOUT_IND 0x00010103

#define ID_MSG_DIAG_SIMMTRANS_REQ 0x00010107
#define ID_MSG_DIAG_SIMMTRANS_CNF 0x00010108
#define ID_MSG_DIAG_SIMMTRANS_IND 0x00010109
#define ID_MSG_DIAG_AT_HSOADDR_IND 0x0001010A /* DIAG通知AT主机信息 */
#define ID_MSG_DIAG_AT_HSO_AT_REQ 0x0001010B  /* DIAG-->AT */
#define ID_MSG_AT_DIAG_HSO_AT_CNF 0x0001010C  /* AT  -->DIAG */
#define ID_MSG_AT_DIAG_HSO_AT_IND 0x0001010D  /* AT  -->DIAG */
#define ID_MSG_A_DIAG_HSO_CONN_IND 0x0001010E /* A DIAG  --> M DIAG */

/*
 * 针对POM模块的消息定义
 * [0x00010121,0x00010140)
 */
#define ID_MSG_SYM_SYSMODE_IND 0x00010121
#define ID_MSG_SYM_SET_TMODE_REQ 0x00010122
#define ID_MSG_SYM_SET_TMODE_CNF 0x00010123
#define ID_MSG_SUM_SOFTUPDATE_IND 0x00010128
#define ID_MSG_SUM_SOFTBACK_REQ 0x00010129
#define ID_MSG_SUM_SOFTBACK_CNF 0x0001012A
#define ID_MSG_SYM_GET_FTP_FILE_IND 0x0001012B
#define ID_MSG_SYM_SET_HPM_REQ 0x0001012C
#define ID_MSG_SYM_SET_HPM_CNF 0x0001012D
#define ID_MSG_SYM_RD_HPM_REQ 0x0001012E
#define ID_MSG_SYM_RD_HPM_CNF 0x0001012F
#define ID_MSG_SYM_ATF_REQ 0x00010130
#define ID_MSG_SYM_ATF_CNF 0x00010131
#define ID_MSG_SYM_QUERY_CARDLOCK_REQ 0x00010132
#define ID_MSG_SYM_QUERY_CARDLOCK_CNF 0x00010133

/*
 * 针对FTM模块的消息定义
 * [0x00010140,0x00010201)
 */
#define ID_MSG_FTM_SET_TXON_REQ 0x00010140
#define ID_MSG_FTM_SET_TXON_CNF 0x00010141
#define ID_MSG_FTM_RD_TXON_REQ 0x00010142
#define ID_MSG_FTM_RD_TXON_CNF 0x00010143
#define ID_MSG_FTM_SET_FCHAN_REQ 0x00010144
#define ID_MSG_FTM_SET_FCHAN_CNF 0x00010145
#define ID_MSG_FTM_RD_FCHAN_REQ 0x00010146
#define ID_MSG_FTM_RD_FCHAN_CNF 0x00010147
#define ID_MSG_FTM_SET_RXON_REQ 0x0001014e
#define ID_MSG_FTM_SET_RXON_CNF 0x0001014f
#define ID_MSG_FTM_RD_RXON_REQ 0x00010150
#define ID_MSG_FTM_RD_RXON_CNF 0x00010151
#define ID_MSG_FTM_SET_FWAVE_REQ 0x00010152
#define ID_MSG_FTM_SET_FWAVE_CNF 0x00010153
#define ID_MSG_FTM_SET_AAGC_REQ 0x00010156
#define ID_MSG_FTM_SET_AAGC_CNF 0x00010157
#define ID_MSG_FTM_RD_AAGC_REQ 0x00010158
#define ID_MSG_FTM_RD_AAGC_CNF 0x00010159
#define ID_MSG_FTM_FRSSI_REQ 0x0001015a
#define ID_MSG_FTM_FRSSI_CNF 0x0001015b
#define ID_MSG_FTM_OPENCHAN_REQ 0x0001015c
#define ID_MSG_FTM_OPENCHAN_CNF 0x0001015d

#define ID_MSG_FTM_TIME_OUT 0x0001015e

#define ID_MSG_FTM_PHY_POWER_REQ 0x0001015f
#define ID_MSG_FTM_PHY_POWER_CNF 0x00010160

#define ID_MSG_FTM_RD_FAGC_REQ 0x00010163
#define ID_MSG_FTM_RD_FAGC_CNF 0x00010164

/* 非信令综测start */
#define ID_MSG_FTM_SET_SRXPOW_REQ 0x000101d3
#define ID_MSG_FTM_SET_SRXPOW_CNF 0x000101d4
#define ID_MSG_FTM_RD_SRXPOW_REQ 0x000101d5
#define ID_MSG_FTM_RD_SRXPOW_CNF 0x000101d6

#define ID_MSG_FTM_SET_SRXBLER_REQ 0x000101db
#define ID_MSG_FTM_SET_SRXBLER_CNF 0x000101dc
#define ID_MSG_FTM_TX_CLT_INFO_IND 0x000101dd

/* 非信令综测end */

#define ID_MSG_FTM_SET_GODLOAD_REQ 0x000101f9
#define ID_MSG_FTM_SET_GODLOAD_CNF 0x000101fa

#define ID_MSG_FTM_SET_RESET_REQ 0x000101fb
#define ID_MSG_FTM_SET_RESET_CNF 0x000101fc

#define ID_MSG_L1A_CT_IND 0x00010450

/* 装备 */
#define ID_MSG_FTM_SET_TSELRF_REQ 0x00010950
#define ID_MSG_FTM_SET_TSELRF_CNF 0x00010951
/* 装备 */
#define ID_MSG_FTM_SET_TBAT_REQ 0x0001095c
#define ID_MSG_FTM_SET_TBAT_CNF 0x0001095d

#define ID_MSG_FTM_RD_TBAT_REQ 0x0001095e
#define ID_MSG_FTM_RD_TBAT_CNF 0x0001095f
#define ID_MSG_FTM_TMODE_TO_SECONDARY 0x00010964
#define ID_MSG_FTM_TMODE_TO_SECONDARY_CNF 0x00010965

#define ID_MSG_FTM_FCHAN_TO_DSP_MAIN 0x00010966
#define ID_MSG_FTM_FCHAN_TO_DSP_SECONDARY 0x00010968
#define ID_MSG_FTM_POLLTEMPR_REQ 0x00010969

/* notice: lte msg id range from 0x00011000 to 0x00011FFF */

#define ID_MSG_TDS_SET_TSELRF_REQ 0x0001097B
#define ID_MSG_TDS_SET_TSELRF_CNF 0x0001097C
#define ID_MSG_TDS_RD_TSELRF_REQ 0x0001097D
#define ID_MSG_TDS_RD_TSELRF_CNF 0x0001097E
#define ID_MSG_TDS_SET_FCHAN_REQ 0x0001097F
#define ID_MSG_TDS_SET_FCHAN_CNF 0x00010980
#define ID_MSG_TDS_RD_FCHAN_REQ 0x00010981
#define ID_MSG_TDS_RD_FCHAN_CNF 0x00010982
#define ID_MSG_TDS_SET_FWAVE_REQ 0x00010983
#define ID_MSG_TDS_SET_FWAVE_CNF 0x00010984

#define ID_MSG_TDS_SET_FTXON_REQ 0x00010985
#define ID_MSG_TDS_SET_FTXON_CNF 0x00010986
#define ID_MSG_TDS_RD_FTXON_REQ 0x00010987
#define ID_MSG_TDS_RD_FTXON_CNF 0x00010988
#define ID_MSG_TDS_SET_FRXON_REQ 0x00010989
#define ID_MSG_TDS_SET_FRXON_CNF 0x0001098A
#define ID_MSG_TDS_RD_FRXON_REQ 0x0001098B
#define ID_MSG_TDS_RD_FRXON_CNF 0x0001098C
#define ID_MSG_TDS_RD_FRSSI_REQ 0x0001098D
#define ID_MSG_TDS_RD_FRSSI_CNF 0x0001098E

#define ID_MSG_TDS_REQ 0x00010991
#define ID_MSG_TDS_CNF 0x00010992

#define ID_MSG_FTM_SET_LTCOMMCMD_REQ 0x000109BD
#define ID_MSG_FTM_SET_LTCOMMCMD_CNF 0x000109BE
#define ID_MSG_FTM_RD_LTCOMMCMD_REQ 0x000109BF
#define ID_MSG_FTM_RD_LTCOMMCMD_CNF 0x000109C0

#define ID_MSG_FTM_RD_FPLLSTATUS_REQ 0x000109c1
#define ID_MSG_FTM_RD_FPLLSTATUS_CNF 0x000109c2

#define ID_MSG_L4A_MODE_IND 0x00011001
#define ID_MSG_L4A_READ_CEREG_CNF 0x00011002
#define ID_MSG_L4A_CGANS_IND 0x00011003

#define ID_MSG_DIAG_PSTRANS_REQ 0x00011004
#define ID_MSG_DIAG_PSTRANS_CNF 0x00011005
#define ID_MSG_DIAG_PSTRANS_IND 0x00011006

#define ID_MSG_L4A_CSQ_INFO_REQ 0x00011007
#define ID_MSG_L4A_CSQ_INFO_CNF 0x00011008
#define ID_MSG_L4A_RSSI_IND 0x00011009

#define ID_MSG_L4A_ANLEVEL_IND 0x0001100A
#define ID_MSG_L4A_ANQUERY_INFO_REQ 0x0001100B
#define ID_MSG_L4A_ANQUERY_INFO_CNF 0x0001100C

#define ID_MSG_L4A_LWCLASHQRY_REQ 0x0001100D
#define ID_MSG_L4A_LWCLASHQRY_CNF 0x0001100E
#define ID_MSG_L4A_LWCLASH_IND 0x0001100F

#define ID_MSG_L4A_CERSSI_REQ 0x00011010
#define ID_MSG_L4A_CERSSI_CNF 0x00011011
#define ID_MSG_L4A_CERSSI_IND 0x00011012

#define ID_MSG_L4A_CELL_ID_REQ 0x00011015
#define ID_MSG_L4A_CELL_ID_CNF 0x00011016

#define ID_MSG_L4A_LTE_TO_IDLE_REQ 0x00011017
#define ID_MSG_L4A_LTE_TO_IDLE_CNF 0x00011018

#define ID_MSG_L4A_LW_THRESHOLD_REQ 0x00011019
#define ID_MSG_L4A_LW_THRESHOLD_CNF 0x0001101A

#define ID_MSG_L4A_FAST_DORM_REQ 0x0001101B
#define ID_MSG_L4A_FAST_DORM_CNF 0x0001101C

#define ID_MSG_L4A_CELL_INFO_REQ 0x00011020
#define ID_MSG_L4A_CELL_INFO_CNF 0x00011021
#define ID_MSG_L4A_CELL_INFO_IND 0x00011022

#define ID_MSG_L4A_LTE_CS_REQ 0x00011023
#define ID_MSG_L4A_LTE_CS_CNF 0x00011024

#define ID_MSG_L4A_IND_CFG_REQ 0x00011025

#define ID_MSG_L4A_CERSSI_INQ_REQ 0x00011026
#define ID_MSG_L4A_CERSSI_INQ_CNF 0x00011027

#define ID_MSG_L4A_ISMCOEXSET_REQ 0x00011028
#define ID_MSG_L4A_ISMCOEXSET_CNF 0x00011029

#define ID_MSG_L4A_RADVER_SET_REQ 0x00011030
#define ID_MSG_L4A_RADVER_SET_CNF 0x00011031

#define ID_MSG_L4A_LCACELLQRY_REQ 0x00011032
#define ID_MSG_L4A_LCACELLQRY_CNF 0x00011033
#define ID_MSG_L4A_LCACELL_IND 0x00011034

#define ID_MSG_L4A_LCASCELLINFO_QRY_REQ 0x00011035
#define ID_MSG_L4A_LCASCELLINFO_QRY_CNF 0x00011036

#define ID_MSG_TDS_DIAG_REQ (0x00020001)
#define ID_MSG_TDS_DIAG_CNF (0x00020002)

/*
 * 针对L1A消息接口的消息ID定义 [0x00010450,0x00010500)
 */

#define RSSI_POW_MAX_NUM 30
#define CT_F_FREQ_LIST_MAX_NUM 32

/*
 * MSP 内部消息参数定义
 */

/*
 * MUX相关接口
 * (0x00010000,0x00010050]
 */

/*
 * MSM模块的消息参数定义
 */

typedef struct {
    VOS_UINT16 index;
    VOS_UINT8  content[8];
} WIWEP_SUPPORT;

typedef struct {
    VOS_UINT32 bandValue;
    VOS_UINT8  bandStr[4];
} CLIP_SupportBands;

/* AUTHORITYVER */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_RdAuthorityverReq;

typedef struct {
    VOS_UINT32 errCode;
    /* VOS_UINT32 ulLength; 保留，也许以后有用 */
    VOS_CHAR szAuthorityVer[MSP_UPGRADE_DLOAD_VER_LEN + 1];
    VOS_CHAR padding;

} FTM_RdAuthorityverCnf;

/* AUTHORITYID */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_RdAuthorityidReq;

typedef struct {
    VOS_UINT32 errCode;
    /* VOS_UINT32 ulLength;保留，也许以后有用 */
    VOS_CHAR szAuthorityId[MSP_UPGRADE_DLOAD_VER_LEN + 1];
    VOS_CHAR padding0;
    VOS_CHAR szAuthorityType[2];
    VOS_CHAR padding[2];

} FTM_RdAuthorityidCnf;

/* BOOTROMVER */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_RdBootromverReq;

typedef struct {
    VOS_UINT32 errCode;
    /* VOS_UINT32 ulLength; 保留，也许以后有用 */
    VOS_CHAR szSendStr[MSP_UPGRADE_DLOAD_VER_LEN + 1];
    VOS_CHAR padding;

} FTM_RdBootromverCnf;

/* NVBACKUP */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_SetNvbackupReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT32 retValue;

} FTM_SetNvbackupCnf;

/* NVRESTORE */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_SetNvrestoreReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT32 retValue;

} FTM_SetNvrestoreCnf;

/* GODLOAD */
typedef struct {
    VOS_UINT32 nvBackupFlag;
} FTM_SetGodloadReq;

typedef struct {
    VOS_UINT32 errCode;

} FTM_SetGodloadCnf;

/* RESET */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_SetResetReq;

typedef struct {
    VOS_UINT32 errCode;

} FTM_SetResetCnf;

/* NVRSTSTTS */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_SetNvrststtsReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_CHAR   szNvRst[4];

} FTM_SetNvrststtsCnf;

/*
 * 描述: 旨在告诉MUX DIAG 已经丢弃该包数据 TO DO ...
 * ID: ID_MSG_SYM_SYSMODE_IND
 * REQ: NA
 * CNF: NA
 * IND: SYM_SysmodeInd
 * 说明:
 */
typedef enum _POM_SYSMODE {
    EN_POM_SYSMODE_NORMAL = 0,
    EN_POM_SYSMODE_FTM,
    EN_POM_SYSMODE_ETM,
    EN_POM_SYSMODE_UNKNOWN
} SYM_SYSMODE;

typedef struct {
    SYM_SYSMODE sysMode;
} SYM_SysmodeInd;

/*
 * 描述: 定义TMODE切换时相关接口 ^TMODE缠线校准和SYM交互
 *       SYM会根据参数设置或者不设置DSP校准
 * ID: ID_MSG_SYM_SET_TMODE_REQ,ID_MSG_SYM_SET_TMODE_CNF
 * REQ: SYM_SET_TmodeReq,SYM_RD_TmodeReq
 * CNF: SYM_SET_TmodeCnf,SYM_RD_TMODE_CNF_STRU
 * IND: NA
 * 说明:
 */
enum {
    EN_SYM_TMODE_SIGNAL = 0,
    EN_SYM_TMODE_NONESIGNAL,
    EN_SYM_TMODE_LOAD,
    EN_SYM_TMODE_RESET,
    EN_SYM_TMODE_SHUTDOWN,
    EN_SYM_TMODE_SIGNAL_NOCARD = 11,

    EN_SYM_TMODE_OFFLINE      = 12,
    EN_SYM_TMODE_F_NONESIGNAL = 13, /* TMODE=6改为13 */

    EN_SYM_TMODE_SYN_NONESIGNAL = 14, /* LTE 非信令综测 */
    EN_SYM_TMODE_SET_SECONDARY  = 15,
    EN_SYM_TMODE_GU_BT          = 16,
    EN_SYM_TMODE_TDS_FAST_CT    = 17,
    EN_SYM_TMODE_TDS_BT         = 18,
    EN_SYM_TMODE_COMM_CT        = 19,
    EN_SYM_TMODE_UNKNOWN        = 0xffffffff
};
typedef VOS_UINT32 SYM_TMODE_ENUM;

enum {
    EN_SYM_TMODE_STA_DOING = 0,
    EN_SYM_TMODE_STA_OK,
    EN_SYM_TMODE_STA_ERR,
    EN_SYM_TMODE_STA_UNKNOWN = 0xffffffff
};
typedef VOS_UINT32 SYM_TMODE_STA_ENUM;

/* 设置接口 */
typedef struct {
    VOS_UINT8 wcdmaIsSet; /* 1,IsSet; 0,notSet; */
    VOS_UINT8 cdmaIsSet;
    VOS_UINT8 tdscdmaIsSet;
    VOS_UINT8 gsmIsSet;
    VOS_UINT8 edgeIsSet;
    VOS_UINT8 awsIsSet;
    VOS_UINT8 fddlteIsSet;
    VOS_UINT8 tddlteIsSet;
    VOS_UINT8 wifiIsSet;
    VOS_UINT8 rsv1;
    VOS_UINT8 rsv2;
    VOS_UINT8 rsv3;
} AT_TmodeRatFlag;

/* AT^PLATFORM? */
typedef struct {
    VOS_UINT16 provider;
    VOS_UINT16 subPlatForm;
} AT_PLATFORM_Nv;

/* 设置接口 */
typedef struct {
    /* 是否添加控制信息待定 TODO: ... (陈文峰/00149739/2010-05-07) */
    SYM_TMODE_ENUM tmodeMode;
} SYM_SET_TmodeReq;

typedef struct {
    /* 是否添加控制信息待定 TODO: ... (陈文峰/00149739/2010-05-07) */
    SYM_TMODE_ENUM     tmodeMode;   /* 返回模式 */
    SYM_TMODE_STA_ENUM symTmodeSta; /* 返回状态 */

    /* ERR_SUCCESS(0):成功。 */
    /* 1:关闭发射机错误 */
    /* 2:单板模式错误 */
    /* 3:没有设置相关信道 */
    /* 4:打开发射机失败 */
    /* 5:其它错误 */
    VOS_UINT32 errCode; /* ERR_SUCCESS:成功。OTHER:失败 */
} SYM_SET_TmodeCnf;

/* 查询接口 */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} SYM_RD_TmodeReq;

typedef struct {
    VOS_UINT32 tmode;
    VOS_UINT32 cnfRet;
} AT_TmodeResult;

typedef SYM_SET_TmodeCnf SYM_RD_TMODE_CNF_STRU;

/*
 * 描述 : ^FAGC 读取RSRP接口
 * ID   : ID_MSG_FTM_RD_FAGC_REQ,ID_MSG_FTM_RD_FAGC_CNF
 * REQ  : FTM_RdFagcReq
 * CNF  : FTM_RdFagcCnf
 * IND  : NA
 * 说明 :
 */

/* 读取接口 */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_RdFagcReq;

typedef struct {
    VOS_INT16  rsrp; /* 功率值 */
    VOS_UINT16 rsd;

    /* ERR_MSP_SUCCESS 成功 */
    /* 超时 */
    /* 未打开 */
    VOS_UINT32 errCode;
} FTM_RdFagcCnf;

/* 电池校准参数数据结构 */
typedef struct {
    VOS_UINT16 minValue;
    VOS_UINT16 maxValue;
} MSP_VBAT_CALIBART_TYPE;

/*
 * 描述: ^F 相关接口
 * ID: ID_MSG_SYM_ATF_REQ,ID_MSG_SYM_ATF_CNF
 * REQ: SYM_AtfReq,
 * CNF: SYM_AtfCnf
 * IND: NA
 * 说明:
 */
typedef struct {
    VOS_UINT32 fpara; /* 保留字段 */
} SYM_AtfReq;

typedef struct {
    VOS_UINT32 errCode; /* ERR_SUCCESS:成功。OTHER:失败 */
} SYM_AtfCnf;

/*
 * 描述 : ^FTXON 非信令下模式下，设置打开发射机
 * ID   : ID_MSG_FTM_SET_TXON_REQ,ID_MSG_FTM_SET_TXON_CNF
 *        ID_MSG_FTM_RD_TXON_REQ,ID_MSG_FTM_RD_TXON_CNF
 * REQ  : FTM_SetTxonReq,FTM_RdTxonReq
 * CNF  : FTM_SetTxonCnf,FTM_RdTxonCnf
 * IND  : NA
 * 说明 :
 */

/* TXON开关类型 */
enum {
    /* 0:表示关闭发射机 */
    EN_FTM_TXON_CLOSE = 0,

    /* 1:表示打开发射机，包括基带调制和RF发射通道 */
    EN_FTM_TXON_UL_RFBBP_OPEN,

    /* 2:表示只打开RFIC Transmitter和PA ， */
    /* BB调制通道不打开，用于VCTCXO校准 */
    EN_FTM_TXON_RF_PA_OPEN,
    EN_FTM_TXON_UNKNOWN = 0xFFFFFFFF
};
typedef VOS_UINT32 FTM_TXON_SWT_ENUM;


enum FTM_CltEnable {
    FTM_CLT_DISABLE = 0,
    FTM_CLT_ENABLE,

    FTM_CLT_BUTT
};
typedef VOS_UINT8 FTM_CltEnableUint8;

typedef struct {
    VOS_UINT32 reserved;
} FTM_RdFpllstatusReq;

typedef struct {
    VOS_UINT16 txStatus;
    VOS_UINT16 rxStatus;
    VOS_UINT32 errCode;
} FTM_RdFpllstatusCnf;

/* SET */
typedef struct {
    FTM_TXON_SWT_ENUM  swtich;
    FTM_CltEnableUint8 cltEnableFlg;
    VOS_UINT8          reserved[3];
} FTM_SetTxonReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:关闭发射机错误 */
    /* 2:单板模式错误 */
    /* 3:没有设置相关信道 */
    /* 4:打开发射机失败 */
    /* 5:其它错误 */
    VOS_UINT32 errCode;
} FTM_SetTxonCnf;

/* READ */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_RdTxonReq;

typedef struct {
    FTM_TXON_SWT_ENUM swtich;

    /* ERR_SUCCESS(0):成功。 */
    /* 1:关闭发射机错误 */
    /* 2:单板模式错误 */
    /* 3:没有设置相关信道 */
    /* 4:打开发射机失败 */
    /* 5:其它错误 */
    VOS_UINT32 errCode;
} FTM_RdTxonCnf;

/*
 * 描述: ^FCHAN 设置非信令的信道
 * ID: ID_MSG_FTM_SET_FCHAN_REQ,ID_MSG_FTM_SET_FCHAN_CNF
 *     ID_MSG_FTM_RD_FCHAN_REQ,ID_MSG_FTM_RD_FCHAN_CNF
 * REQ: FTM_SetFchanReq,FTM_SetFchanCnf
 * CNF: FTM_RdFchanReq,FTM_RdFchanCnf
 * IND: NA
 * 说明:
 */

enum _FCHAN_MODE {
    EN_FCHAN_MODE_WCDMA = 0,
    EN_FCHAN_MODE_CDMA,
    EN_FCHAN_MODE_TD_SCDMA,
    EN_FCHAN_MODE_TD_GSM,
    EN_FCHAN_MODE_TD_EDGE,
    EN_FCHAN_MODE_TD_AWS,
    EN_FCHAN_MODE_FDD_LTE,
    EN_FCHAN_MODE_TDD_LTE,
    EN_FCHAN_MODE_TD_SELF_DIFINE,
#if (FEATURE_LTEV == FEATURE_ON)
    EN_FCHAN_MODE_LTEV = 10,
#endif
    EN_FCHAN_MODE_UNKONWN = 0xFF
};
typedef VOS_UINT32 FCHAN_MODE_ENUM;

/* 待确认 */
#define FTM_AT_FCHAN_BAND 8 /* 800M */
#define FTM_AT_FCHAN_BAND_MAX 14
#define FTM_DSP_FCHAN_BAND 20 /* 800M */

/* SET */
typedef struct {
    FCHAN_MODE_ENUM fchanMode;
    VOS_UINT32      channel; /* 频点 */
    VOS_UINT8       band;
    VOS_UINT8       padding;
    VOS_UINT16      listeningPathFlg;
} FTM_SetFchanReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:输入频段信息无法对应 */
    /* 3:设置信道信息失败 */
    /* 4:输入频段和信道信息无法组合 */
    /* 5:其它错误 */
    FCHAN_MODE_ENUM fchanMode;
    VOS_UINT32      errCode;
} FTM_SetFchanCnf;

/* READ */
typedef struct {
    VOS_UINT32 reserved; /* 保留字段 */
} FTM_RdFchanReq;

#define MSP_FTM_FREQ_VALID 1
#define MSP_FTM_FREQ_INVALID 0

typedef struct {
    FCHAN_MODE_ENUM fchanMode;
    VOS_UINT8       band;
    VOS_UINT8       padding[3]; /* PADDING */

    /* 装备AT命令新定义, 查询时上行和下行频道设置都需返回 */
    VOS_UINT32 ulChannel;
    VOS_UINT32 dlChannel;

    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:输入频段信息无法对应 */
    /* 3:设置信道信息失败 */
    /* 4:输入频段和信道信息无法组合 */
    /* 5:其它错误 */
    VOS_UINT32 errCode;
} FTM_RdFchanCnf;

/*
 * 描述 : ^FVCTCXO 设置VCTCXO的电压值
 * ID   : ID_MSG_FTM_SET_FVCTCXO_REQ,ID_MSG_FTM_SET_FVCTCXO_CNF
 *        ID_MSG_FTM_RD_FVCTCXO_REQ,ID_MSG_FTM_RD_FVCTCXO_CNF
 * REQ  : FTM_SET_FVCTCXO_REQ_STRU,FTM_RD_FVCTCXO_REQ_STRU
 * CNF  : FTM_SET_FVCTCXO_CNF_STRU,FTM_RD_FVCTCXO_CNF_STRU
 * IND  : NA
 * 说明 :
 */

#define FTM_CALPDDCS_EXECUTING 1 /* IP2校准执行中 */
#define FTM_CALPDDCS_EXECUTED 0  /* IP2校准执行完毕 */

typedef struct {
    VOS_UINT16 pdAutoFlg; /* 具体瞪闲行诺篮盼0～65535 */
    VOS_UINT16 rsv;       /* 保留字段 */
} FTM_SET_FFcalpddcsReq;
typedef struct {
    /* U32_T ulMsgID;	上报消息ID */
    VOS_UINT16 dacCfg; /* 自校准获得的DAC配置 */
    VOS_UINT16 vgaCfg; /* 自校准获得的VGA配置 */
    VOS_UINT16 status; /* VGA校准异常标志 */
    VOS_UINT16 rsv;
    /*
     * ERR_SUCCESS(0):成功。
     * 526    单板模式错误(比如信令模式)
     * 527    没有设置相关信道
     * 545    没有打开发射机
     * 566    没有打开接收机的快速校准
     * 501    未知错误
     */
    VOS_UINT32 errCode;
} FTM_RD_FFcalpddcsCnf;
// SET
typedef struct {
    VOS_UINT32 pdEnbFlg; /* 指示PD是否使能 */
    VOS_UINT16 dacCfg;
    VOS_UINT16 vgaCfg;
} FTM_SET_FFpdpowsReq;
typedef struct {
    VOS_UINT32 errCode;
} FTM_SET_FFpdpowsCnf;

typedef struct {
    VOS_UINT16 errCode; /* 状态 */
    VOS_UINT16 rsv;
    VOS_UINT16 pddcValue[CT_F_FREQ_LIST_MAX_NUM][RSSI_POW_MAX_NUM];
} FTM_RdFqpddcresCnf;

/*
 * 描述 : RXON 非信令下模式下，设置打开接收机
 * ID   : ID_MSG_FTM_SET_RXON_REQ,ID_MSG_FTM_SET_RXON_CNF
 *        ID_MSG_FTM_RD_RXON_REQ,ID_MSG_FTM_RD_RXON_CNF
 * REQ  : FTM_SetRxonReq,FTM_RdRxonReq
 * CNF  : FTM_SetRxonCnf,FTM_RdRxonCnf
 * IND  : NA
 * 说明 :
 */

/* RXON开关类型 */
enum {
    EN_FTM_RXON_CLOSE   = 0,         /* 接收机关闭     */
    EN_FTM_RXON_OPEN    = 1,         /* 接收机打开     */
    EN_FTM_RXON_UNKNOWN = 0xFFFFFFFF /* 接收机状态未知 */
};
typedef VOS_UINT32 FTM_RXON_SWT_ENUM;

/* SET */
typedef struct {
    /* 0: 表示关闭收双通道 */
    /* 1: 表示打开收双通道 */
    VOS_UINT32 rxSwt;
} FTM_SetRxonReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:打开接收机失败 */
    VOS_UINT32 errCode;
} FTM_SetRxonCnf;

/* READ */
typedef struct {
    VOS_UINT32 resved;
} FTM_RdRxonReq;

typedef struct {
    /* 0: 表示关闭收双通道 */
    /* 1: 表示打开收双通道 */
    VOS_UINT32 rxSwt;

    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:打开接收机失败 */
    VOS_UINT32 errCode;
} FTM_RdRxonCnf;

/*
 * 描述 : ^RFANT 设置RF通道选择
 *        配置RX通道。RTT收到该原语后选定需要进行RSSI折算和上报的接收通道
 * ID   : ID_MSG_FTM_SET_RXANT_REQ,ID_MSG_FTM_SET_RXANT_CNF
 *        ID_MSG_FTM_RD_RXANT_REQ,ID_MSG_FTM_RD_RXANT_CNF
 * REQ  : FTM_SetRxantReq,FTM_RdRxantReq
 * CNF  : FTM_SetRxantCnf,FTM_RdRxantCnf
 * IND  : NA
 * 说明 :
 */

#define FTM_RXANT_TYPE_TX 0
#define FTM_RXANT_TYPE_RX 1
#define FTM_RXANT_TYPE_UNKNOWN 0xff

#define FTM_RXANT_CLOSE_DCHAN 0
#define FTM_RXANT_OPEN_CHAN1 1
#define FTM_RXANT_OPEN_CHAN2 2
#define FTM_RXANT_OPEN_DCHAN 3
#define FTM_RXANT_UNKNOWN 0xff

/* for AT^TSELRF  选择射频通路指令 */
#define FTM_TSELRF_TD 6

#define FTM_TSELRF_WIFI 7
#define FTM_TSELRF_FDD_LTE_MAIN 9
#define FTM_TSELRF_FDD_LTE_SUB 10
#define FTM_TSELRF_FDD_LTE_MIMO 11
#define FTM_TSELRF_TDD_LTE_MAIN 12
#define FTM_TSELRF_TDD_LTE_SUB 13
#define FTM_TSELRF_TDD_LTE_MIMO 14

/* SET */
typedef struct {
    VOS_UINT8 chanelType;

    VOS_UINT8  chanelNo;
    VOS_UINT16 reserved;
} FTM_SetRxantReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:通道打开失败 */
    /* 2:通道关闭失败 */
    /* 3:不支持（如打开通道2的发送通道，DD800项目采用1T2R模式） */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_SetRxantCnf;

/* READ */
typedef struct {
    VOS_UINT32 reserved;
} FTM_RdRxantReq;

typedef struct {
    VOS_UINT8 chanelNo;
    VOS_UINT8 padding[3];

    /* ERR_SUCCESS(0):成功。 */
    /* 1:通道打开失败 */
    /* 2:通道关闭失败 */
    /* 3:不支持（如打开通道2的发送通道，DD800项目采用1T2R模式） */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_RdRxantCnf;

/* **************************************************************************** */
/* AT^TSELRF */
/* **************************************************************************** */
/* SET */
typedef struct {
    VOS_UINT8 path;

    VOS_UINT8 group;

    VOS_UINT16 reserved;
} FTM_SetTselrfReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:通道打开失败 */
    /* 2:通道关闭失败 */
    /* 3:不支持（如打开通道2的发送通道，DD800项目采用1T2R模式） */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_SetTselrfCnf;

/* READ */
typedef struct {
    VOS_UINT32 reserved;
} FTM_RdTselrfReq;

typedef struct {
    VOS_UINT8  path;
    VOS_UINT8  group;
    VOS_UINT16 rsv;

    /* ERR_SUCCESS(0):成功。 */
    /* 1:通道打开失败 */
    /* 2:通道关闭失败 */
    /* 3:不支持（如打开通道2的发送通道，DD800项目采用1T2R模式） */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_RdTselrfCnf;

/* SET */
typedef struct {
    /* 8-74 */
    VOS_UINT8 level;
    VOS_UINT8 padding[3];
} FTM_SetFpaReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:设置AAGC失败 */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_SetFpaCnf;

/* SET */
typedef struct {
    /* 8-74 */
    VOS_UINT16 txControlNum;
    VOS_UINT16 padding;
} FTM_SetFdacReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:设置AAGC失败 */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_SetFdacCnf;

/*
 * 描述: ^FWAVE 非信令下波形设置，单位 0.01db
 * ID: ID_MSG_FTM_SET_FWAVE_REQ ID_MSG_FTM_SET_FWAVE_CNF
 * REQ: FTM_SetFwaveReq,  FTM_RdFwaveReq
 * CNF: FTM_SetFwaveCnf,  FTM_RdFwaveCnf
 * IND: NA
 * 说明:
 */
enum _FWAVE_TYPE {
    EN_FWAVE_TYPE_CW = 0, /* 单音         */
    EN_FWAVE_TYPE_WCDMA,  /*  WCDMA调制波 */
    EN_FWAVE_TYPE_GSM,    /*  GSM调制波   */
    EN_FWAVE_TYPE_EDGE,   /*  EDGE调制波  */
    EN_FWAVE_TYPE_WIFI,   /*  WIFI调制波  */
    EN_FWAVE_TYPE_LTE,    /*  LTE调制波   */
    EN_FWAVE_TYPE_CDMA,   /*  CDMA调制波  */
    EN_FWAVE_TYPE_TDS,    /*  TDS调制波   */
    EN_FWAVE_TYPE_UNKONWN = 0xFFFF
};
typedef VOS_UINT32 FWAVE_TYPE_ENUM;

// SET
typedef struct {
    VOS_UINT16 type;  /* 波形类型 */
    VOS_UINT16 power; /* 波形功率  */
} FTM_SetFwaveReq;

typedef struct {
    VOS_UINT32 errCode;
} FTM_SetFwaveCnf;

// READ
typedef struct {
    VOS_UINT32 reserved;
} FTM_RdFwaveReq;

typedef struct {
    VOS_UINT16 type;  /* 波形类型 */
    VOS_UINT16 power; /* 波形功率  */
} FTM_RdFwaveCnf;

/*
 * 描述 : ^FLNA 配置RFIC的AAGC总增益，用于下行AGC表的校准，单位为dB。
 *        RTT收到该原语后将RFIC的总增益立即改为原语指示的增益值
 * ID   : ID_MSG_FTM_SET_AAGC_REQ,ID_MSG_FTM_SET_AAGC_CNF
 *        ID_MSG_FTM_RD_AAGC_REQ,ID_MSG_FTM_RD_AAGC_CNF
 * REQ  : FTM_SetAagcReq,FTM_RdAagcReq
 * CNF  : FTM_SetAagcCnf,FTM_RdAagcCnf
 * IND  : NA
 * 说明 :
 */

/* SET */
typedef struct {
    /* 8-74 */
    VOS_UINT8 aggcLvl;
    VOS_UINT8 padding[3];
} FTM_SetAagcReq;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:设置AAGC失败 */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_SetAagcCnf;

/* READ */
typedef struct {
    VOS_UINT32 reserved;
} FTM_RdAagcReq;

typedef struct {
    /* 8-74 */
    VOS_UINT8 aggcLvl;
    VOS_UINT8 padding[3];

    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:设置AAGC失败 */
    /* 4:其它错误 */
    VOS_UINT32 errCode;
} FTM_RdAagcCnf;

/*
 * AT^TBAT
 */
typedef struct {
    VOS_UINT8  type;
    VOS_UINT8  opr;
    VOS_UINT16 valueMin;
    VOS_UINT16 valueMax;
    VOS_UINT16 rsv;

} FTM_SetTbatReq;

typedef struct {
    VOS_UINT32 errCode;
} FTM_SetTbatCnf;

typedef struct {
    VOS_UINT8  type;
    VOS_UINT8  opr;
    VOS_UINT16 value;
} FTM_RdTbatReq;

typedef struct {
    VOS_UINT8  type;
    VOS_UINT8  rsv;
    VOS_UINT16 value;
    VOS_UINT32 errCode;
} FTM_RdTbatCnf;

typedef struct {
    VOS_UINT16 min;
    VOS_UINT16 max;
} FTM_VBAT_CALIBART_TYPE;

/* ******************************************************************************** */


typedef struct {
    VOS_UINT8  ulbcs;
    VOS_UINT8  ulbcl;
    VOS_UINT16 reserved;
    VOS_UINT32 result;
} DRV_GetCbcCnf;

/*
 * 描述: ^FRSSI 配置RFIC的AAGC总增益，用于下行AGC表的校准，单位为dB。
 *       RTT收到该原语后将RFIC的总增益立即改为原语指示的增益值
 * ID: ID_MSG_FTM_FRSSI_REQ,ID_MSG_FTM_FRSSI_CNF
 * REQ: FTM_FRSSI_Req
 * CNF: FTM_FRSSI_Cnf
 * IND: NA
 * 说明:
 */
typedef struct {
    /* 只使用整型数 */
    VOS_UINT32 reserved;
} FTM_FRSSI_Req;

typedef struct {
    /* 全部是整型值, 单位1/8dBm, 根据DSP接口, 为正值 */
    VOS_INT32 value1;
    VOS_INT32 value2;
    VOS_INT32 value3;
    VOS_INT32 value4;
    VOS_INT32 value5;
    VOS_INT32 value6;
    VOS_INT32 value7;
    VOS_INT32 value8;

    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:没有打开接收机 */
    /* 4:无信号输入或检测不到信号 */
    /* 5:其它错误 */
    VOS_UINT32 errCode;
} FTM_FRSSI_Cnf;


typedef struct {
    VOS_UINT32 msgId;              /* 消息ID */
    VOS_INT16  gammaReal;          /* 反射系数实部 */
    VOS_INT16  gammaImag;          /* 反射系数虚部 */
    VOS_UINT16 gammaAmpUc0;        /* 驻波检测场景0反射系数幅度 */
    VOS_UINT16 gammaAmpUc1;        /* 驻波检测场景1反射系数幅度 */
    VOS_UINT16 gammaAmpUc2;        /* 驻波检测场景2反射系数幅度 */
    VOS_UINT16 gammaAntCoarseTune; /* 粗调格点位置 */
    VOS_UINT32 ulwFomcoarseTune;   /* 粗调FOM值 */
    VOS_UINT16 cltAlgState;        /* 闭环算法收敛状态 */
    VOS_UINT16 cltDetectCount;     /* 闭环收敛总步数 */
    VOS_UINT16 dac0;               /* DAC0 */
    VOS_UINT16 dac1;               /* DAC1 */
    VOS_UINT16 dac2;               /* DAC2 */
    VOS_UINT16 dac3;               /* DAC3 */
} FTM_CltInfoInd;

/*
 * 描述 : SYM给FTM发起请求，要求打开非信令模式
 * ID   : ID_MSG_FTM_OPENCHAN_REQ,ID_MSG_FTM_OPENCHAN_CNF
 * REQ  : FTM_OPENCHAN_Req
 * CNF  : FTM_OPENCHAN_Cnf
 * IND  : NA
 * 说明 :
 */
/* 非信令下子模式 */

/* EN_SYM_TMODE_NONESIGNAL 普通校准 */
/* EN_SYM_TMODE_F_NONESIGNAL 快速校准 */

typedef struct {
    /* 只使用整型数 */
    /* EN_SYM_TMODE_NONESIGNAL */
    /* EN_SYM_TMODE_F_NONESIGNAL */
    VOS_UINT8 tmodeType;

    VOS_UINT8 padding[3];
} FTM_OPENCHAN_Req;

typedef struct {
    /* ERR_SUCCESS(0):成功。 */
    /* 1:单板模式错误 */
    /* 2:没有设置相关信道 */
    /* 3:没有打开接收机 */
    /* 4:无信号输入或检测不到信号 */
    /* 5:其它错误 */
    VOS_UINT32 errCode;
} FTM_OPENCHAN_Cnf;

/*
 * 描述: SYM给FTM发起请求，要求打开或者关闭工控
 * ID: ID_MSG_FTM_PHY_POWER_REQ,ID_MSG_FTM_PHY_POWER_CNF
 * REQ: FTM_PHY_PowerReq
 * CNF: FTM_PHY_PowerCnf
 * IND: NA
 * 说明:
 */
typedef struct {
    /* 只使用整型数 */
    VOS_UINT32 reserved;
} FTM_PHY_PowerReq;

typedef struct {
    VOS_UINT32 errCode;
} FTM_PHY_PowerCnf;

/*
 * FTM模块快速校准消息参数定义
 */

/*
 * 描述 : ^BANDSW 单板Band切换
 * ID   : ID_MSG_FTM_SET_BANDSW_REQ,ID_MSG_FTM_SET_BANDSW_CNF
 *        ID_MSG_FTM_RD_BANDSW_REQ,ID_MSG_FTM_RD_BANDSW_CNF
 * REQ  : FTM_SET_BANDSW_REQ_STRU,FTM_RD_BANDSW_REQ_STRU
 * CNF  : FTM_SET_BANDSW_CNF_STRU,FTM_RD_BANDSW_CNF_STRU
 * IND  : NA
 * 说明 :
 */

#define FTM_MAX_BAND_NUM 141
#define FTM_MAX_TDS_BAND_NUM 6

#define CT_F_TXPOW_SET_MAX_NUM 30

#define FTM_CT_F_PA_SET_MAX_NUM 30

#define FTM_CT_F_AAGC_SET_MAX_NUM 30

#define TXWAVE_TYPE_CW 0
#define TXWAVE_TYPE_LTE 1

#define START_TRIGGER_TYPE_TX 0
#define START_TRIGGER_TYPE_RX 1
#define START_TRIGGER_TYPE_TXRX 2
#define START_TRIGGER_TYPE_MAXTX 3

typedef struct {
    VOS_UINT32 rsv; /* 保留字段 */
} FTM_RdLtescinfoReq;

/* 非信令综测start */
/* ^SSYNC SET */
typedef struct {
    VOS_UINT32 status;
    VOS_UINT32 errCode;
} FTM_RdSsyncCnf;

typedef struct {
    VOS_UINT32 bandwide;
    VOS_UINT32 errCode;
} FTM_RdStxbwCnf;

typedef struct {
    VOS_UINT32 stxChannel;
    VOS_UINT32 errCode;
} FTM_RdStxchanCnf;

typedef struct {
    VOS_UINT16 subFrameAssign;
    VOS_UINT16 subFramePattern;
    VOS_UINT32 errCode;
} FTM_RdSsubframeCnf;

/* ^SPARA SET */
typedef struct {
    VOS_UINT16 type;
    VOS_UINT16 value;
} FTM_SetSparaReq;

typedef struct {
    VOS_UINT16 type;
    VOS_UINT16 value;
    VOS_UINT32 errCode;

} FTM_RdSparaCnf;

typedef struct {
    VOS_UINT32     typeNum;
    FTM_RdSparaCnf spara[32];
    VOS_UINT32     errCode;
} FTM_RdSparaCnfTotal;

typedef struct {
    VOS_UINT16 segNum;
    VOS_UINT16 reserved;
    VOS_UINT32 errCode;
} FTM_RdSsegnumCnf;

typedef struct {
    VOS_UINT16 usmodulutionNum;
    VOS_UINT16 rsv;
    VOS_UINT16 usmodulationList[256];
    VOS_UINT32 errCode;
} FTM_RdStxmodusCnf;

typedef struct {
    VOS_UINT16 usrbNum;
    VOS_UINT16 rsv;
    VOS_UINT16 usrbList[256];
    VOS_UINT32 errCode;
} FTM_RdStxrbnumsCnf;

typedef struct {
    VOS_UINT16 usrbposNum;
    VOS_UINT16 rsv;
    VOS_UINT16 usrbposList[256];
    VOS_UINT32 errCode;
} FTM_RdStxrbpossCnf;

typedef struct {
    VOS_UINT16 uspowerNum;
    VOS_UINT16 rsv;
    VOS_INT16  uspowerList[256];
    VOS_UINT32 errCode;
} FTM_RdStxpowsCnf;

typedef struct {
    VOS_UINT16 ustypeNum;
    VOS_UINT16 rsv;
    VOS_UINT16 ustypeList[256];
    VOS_UINT32 errCode;
} FTM_RdStxchantypesCnf;

typedef struct {
    VOS_UINT16 ussegmentLen;
    VOS_UINT16 reserved;
    VOS_UINT32 errCode;
} FTM_RdSseglenCnf;

typedef struct {
    VOS_UINT16 switchFlag; /* 非信令下行测试时，上行发射是否打开 0关闭，1打开 */
    VOS_INT16  power;      /* 非信令下行测试时，上行发射功率，范围-40~23 */
    VOS_UINT16 mod;        /* 非信令下行测试时，上行发射取值范围 0-2 ,0:QPSK,1:16QAM,2:64QAM */
    VOS_UINT16 rbNum;      /* 非信令下行测试时，上行发射RB数量，范围1-100 */
    VOS_UINT16 rbPos;      /* 非信令下行测试时，上行发射RB位置，范围0-99 */
    VOS_UINT16 reserved;
    VOS_UINT32 errCode;
} FTM_RdSrxsetCnf;

/* ^SRXPOW SET */
typedef struct {
    VOS_UINT16 switchFlag;
    VOS_INT16  usulPower;
    VOS_UINT16 moduType;
    VOS_UINT16 rbNum;
    VOS_UINT16 rbPos;
    VOS_UINT16 rsv;
} FTM_SetSrxpowReq;

typedef struct {
    VOS_UINT32 errCode;
} FTM_SetSrxpowCnf;

/* ^STXCHANTYPES RD */
typedef struct {
    VOS_UINT32 reserved;
} FTM_RdSrxpowReq;

typedef struct {
    VOS_UINT16 switchFlag;
    VOS_INT16  usulPower;
    VOS_UINT32 errCode;
} FTM_RdSrxpowCnf;

typedef struct {
    VOS_UINT16 ussubFrameNum;
    VOS_UINT16 reserved;
    VOS_UINT32 errCode;
} FTM_RdSrxsubfraCnf;

/* SRXBLER */
/* ^SRXBLER SET */
typedef struct {
    VOS_UINT16 usbler;
    VOS_UINT16 reserved;
} FTM_SetSrxblerReq;

typedef struct {
    VOS_UINT32 errCode;
} FTM_SetSrxblerCnf;

typedef struct {
    VOS_UINT16 usbler[2];
    VOS_INT16  snr[2];
    VOS_UINT32 errCode;
} FTM_RdSrxblerCnf;

// TDS BER RD
typedef struct {
    VOS_UINT32 ulbersum;
    VOS_UINT32 ulberErr;
    VOS_UINT32 errCode;
} FTM_RdStrxberCnf;

typedef struct {
    VOS_UINT16 type;
    VOS_UINT16 status;
    VOS_UINT32 errCode;
} FTM_RdSstartCnf;


typedef struct {
    VOS_UINT32 msgId;
    VOS_UINT32 rserved;
} MSP_PS_AtModeReq;

typedef struct {
    VOS_UINT32 msgId;
    VOS_UINT16 cellId;
    VOS_UINT16 freq;
} MSP_PS_SetSfnSyncReq;

typedef struct {
    VOS_UINT32 msgName;
    VOS_UINT32 ulresult;
} PS_MSP_SFN_SYNC_CNF;

typedef struct {
    VOS_UINT32 msgId;
    VOS_UINT16 type;
    VOS_UINT16 rsv;
} MSP_PS_SstopReq;

typedef struct {
    VOS_UINT32 msgId;
    VOS_UINT32 transType;
    VOS_UINT16 tpcStep;
    VOS_UINT16 rsv;
} MSP_PS_DpchReq;

typedef struct {
    VOS_UINT32 msgName;
    VOS_UINT32 status;
} PS_MSP_DPCH_CNF;

/* 非信令综测end */

/*
 * DIAG模块消息参数定义
 */

/*
 * 描述: DIAG为PS和SIMM提供的透明接口
 * ID: ID_MSG_DIAG_PSTRANS_REQ,ID_MSG_DIAG_PSTRANS_CNF,ID_MSG_DIAG_PSTRANS_IND
 * REQ: DIAG_PSTRANS_Req
 * CNF: DIAG_PSTRANS_CNF_STRU
 * IND: DIAG_PSTRANS_IND_STRU
 * 说明:
 */

typedef struct {
    VOS_UINT32 cmdId;                             /* 透传命令字 */
    VOS_UINT32 paramSize;                         /* 透传命令参数的长度 */
    VOS_UINT8  param[0]; /* 命令参数的数据,对应透传命令的参数 */  //lint !e43
} DIAG_PSTRANS_Req;

/* DIAG<-L4A,SIMM  CNF/IND 数据结构 */
typedef struct {
    VOS_UINT32 cmdId;                                /* 透传命令字(CNF) 或者 上报命令ID(IND ID) */
    VOS_UINT32 paramSize;                            /* 命令参数的长度 */
    VOS_UINT8  param[0]; /* 命令参数的数据 透传内部结构MSP不感知 */  //lint !e43
} DIAG_PSTRANS_CNF_STRU, DIAG_PSTRANS_IND_STRU;

/*
 * 描述 : DIAG为PS和SIMM提供的透明接口
 * ID   : ID_MSG_DIAG_SIMMTRANS_REQ,ID_MSG_DIAG_SIMMTRANS_CNF,ID_MSG_DIAG_SIMMTRANS_IND
 * REQ  : DIAG_SIMMTRANS_REQ_STRU
 * CNF  : DIAG_SIMMTRANS_CNF_STRU
 * IND  : DIAG_SIMMTRANS_IND_STRU
 * 说明 :
 */
typedef DIAG_PSTRANS_Req      DIAG_SIMMTRANS_REQ_STRU;
typedef DIAG_PSTRANS_CNF_STRU DIAG_SIMMTRANS_CNF_STRU;
typedef DIAG_PSTRANS_IND_STRU DIAG_SIMMTRANS_IND_STRU;

/*
 * 针对L1A消息接口的消息参数定义 [0x00010450,0x00010500)
 */

/*
 * 描述: L1A针对FTM的消息接口，存储RTT返回的结构体
 * ID: ID_MSG_L1A_CT_IND
 * REQ: NA
 * CNF: NA
 * IND: SIMM_STATUS_IND_STRU
 * 说明:
 */
typedef struct {
    VOS_UINT32 dspId;       /* 存储相关结构体 */
    VOS_UINT8  padding[0];  //lint !e43
} L1A_CT_Ind;

#define AT_CALIB_PARAM_APCOFF 6
#define AT_CALIB_PARAM_APCOFF_MAX 8
#define AT_CALIB_PARAM_APC 160
#define AT_CALIB_PARAM_APCFREQ 96
#define AT_CALIB_PARAM_AGC 32
#define AT_CALIB_PARAM_AGCFREQ 32
#define FTM_TDS_TX_LEVEL_NUM_MAX 3     /* TDS TX档位数量 */
#define FTM_TDS_RX_LEVEL_NUM_MAX 16    /* TDS RX档位数量 */
#define FTM_TDS_TX_CAL_POWERNUM_MAX 80 /* TDS TX校准功率个数 */
#define FTM_TDS_TX_CAL_FREQNUM_MAX 16  /* TDS TX校准频点个数 */
#define FTM_TDS_RX_CAL_FREQNUM_MAX 16  /* TDS RX校准频点个数 */
#define MAX_SCALIB_DATA_LEN 320

/*
 * 枚举说明: 校准控制操作类型
 */
typedef enum {
    AT_CALIB_OP_TYPE_CACHE, /* "CACHE", 校准数据未发送完，将已接收数据放入数据缓存 */
    AT_CALIB_OP_TYPE_USE,   /* "USE",  将已缓存的数据写入DHI 并通知DSP 立即加载启用 */
    AT_CALIB_OP_TYPE_SAVE,  /* "SAVE", 将已缓存的数据写入NANDFLASH 指定位置，释放接收缓存 */
    AT_CALIB_OP_TYPE_GET,   /* "GET",  查询当前缓存中的指定类型的数据。（读取缓存） */
    AT_CALIB_OP_TYPE_READ,  /* "READ", 查询NANDFLASH 中的指定类型的数据。（读取NANDFLASH) */
    AT_CALIB_OP_TYPE_INI,   /* "INI",  初始化缓存中的数据 */
    AT_CALIB_OP_TYPE_BEGIN, /* "BEGIN",校准开始 */
    AT_CALIB_OP_TYPE_END,   /* "END",  校准结束 */
    AT_CALIB_OP_TYPE_SET,   /* "SET",  设置DSP校准状态 */
    AT_CALIB_OP_TYPE_BUTT
} AT_TDS_OptType;
typedef VOS_UINT32 AT_TDS_OptTypeUint32;

/*
 * 枚举说明: 校准控制数据类型
 */
typedef enum {
    AT_CALIB_BAND_TYPE_A = 1, /* "BAND A", */
    AT_CALIB_BAND_TYPE_E = 5, /* "BAND E", */
    AT_CALIB_BAND_TYPE_F = 6, /* "BAND F", */
    AT_CALIB_BAND_TYPE_BUTT
} AT_TDS_BandType;
typedef VOS_UINT32 AT_TDS_BandTypeUint32;

/*
 * 枚举说明: 校准控制数据类型
 */
typedef enum {
    AT_CALIB_DATA_TYPE_APCOFFSETFLAG, /* "APCOFFSETFLAG", */
    AT_CALIB_DATA_TYPE_APCOFFSET,     /* "APCOFFSET", */
    AT_CALIB_DATA_TYPE_APC,           /* "APC", */
    AT_CALIB_DATA_TYPE_APCFREQ,       /* "APCFREQ", */
    AT_CALIB_DATA_TYPE_AGC,           /* "AGC", */
    AT_CALIB_DATA_TYPE_AGCFREQ,       /* "AGCFREQ", */
    AT_CALIB_DATA_TYPE_BUTT
} AT_TDS_DataType;
typedef VOS_UINT32 AT_TDS_DataTypeUint32;

/*
 * 结构说明: 校准控制相关数据结构
 */
typedef struct {
    VOS_UINT16 flag1; /* 启动结束校准 */
    VOS_UINT16 flag2; /* 档位标志 */
} FTM_ScalibApcoffsetSetting;

/*
 * 结构说明: 校准控制相关数据结构
 */
typedef struct {
    /* reserved for CQUPT begin */
    VOS_UINT32 boardId;
    VOS_UINT16 serial[12]; /* SN */
    VOS_UINT16 imei[5];    /* IMEI. */
    VOS_UINT16 imeiSv[5];  /* IMEISV. */
    VOS_UINT16 armFile[10];
    VOS_UINT16 zsp1File[10];
    VOS_UINT16 zsp2File[10];
    VOS_UINT16 tdRfTabFlag1; /* 射频表的标志1 */
    VOS_UINT16 reserved;
    VOS_UINT16 tdApcTab[80]; /* 射频自动功率控制表 */
    VOS_UINT16 tdAgcTab[6];  /* 射频自动增益表 */
    VOS_UINT16 tdApcOffset[6];
    VOS_UINT16 tdApcOffsetFlag;
    VOS_UINT16 tdAfcOffset; /* AFC校准步长 */
    VOS_UINT32 tdDcOffset;  /* 直流校准参数 */
    VOS_UINT16 afcAgcDcFlag;
    VOS_UINT16 reserved1[3];
    VOS_UINT16 tdRfTabFlag2; /* 射频标的标志2 */
    VOS_UINT16 reserved2[3];
    /* reserved for CQUPT end */
    /* added at 2012-6-11 begin */
    VOS_UINT16 apcOffFlag;
    VOS_UINT16 reserved3;
    VOS_UINT16 tdsTxPaPowerBandA[FTM_TDS_TX_LEVEL_NUM_MAX];
    VOS_UINT16 reserved4;
    VOS_UINT16 tdsTxPaPowerBandE[FTM_TDS_TX_LEVEL_NUM_MAX];
    VOS_UINT16 reserved5;
    VOS_UINT16 tdsTxPaPowerBandF[FTM_TDS_TX_LEVEL_NUM_MAX];
    VOS_UINT16 reserved6;
    VOS_UINT16 tdsTxApcCompBandA[FTM_TDS_TX_CAL_POWERNUM_MAX];
    VOS_UINT16 tdsTxApcCompBandE[FTM_TDS_TX_CAL_POWERNUM_MAX];
    VOS_UINT16 tdsTxApcCompBandF[FTM_TDS_TX_CAL_POWERNUM_MAX];
    VOS_UINT16 tdsTxApcFreqCompBandA[FTM_TDS_TX_LEVEL_NUM_MAX][FTM_TDS_TX_CAL_FREQNUM_MAX];
    VOS_UINT16 tdsTxApcFreqCompBandE[FTM_TDS_TX_LEVEL_NUM_MAX][FTM_TDS_TX_CAL_FREQNUM_MAX];
    VOS_UINT16 tdsTxApcFreqCompBandF[FTM_TDS_TX_LEVEL_NUM_MAX][FTM_TDS_TX_CAL_FREQNUM_MAX];
    VOS_UINT16 tdsRxAgcCompBandA[FTM_TDS_RX_LEVEL_NUM_MAX];
    VOS_UINT16 tdsRxAgcCompBandE[FTM_TDS_RX_LEVEL_NUM_MAX];
    VOS_UINT16 tdsRxAgcCompBandF[FTM_TDS_RX_LEVEL_NUM_MAX];
    VOS_UINT16 tdsRxAgcFreqCompBandA[FTM_TDS_RX_CAL_FREQNUM_MAX];
    VOS_UINT16 tdsRxAgcFreqCompBandE[FTM_TDS_RX_CAL_FREQNUM_MAX];
    VOS_UINT16 tdsRxAgcFreqCompBandF[FTM_TDS_RX_CAL_FREQNUM_MAX];
    /* added at 2012-6-11 end */
} FTM_ScalibSystemSetting;

/* MAXPOWER NV结构 */
typedef struct {
    VOS_INT16 asTxMaxPower[FTM_TDS_TX_LEVEL_NUM_MAX];
    VOS_INT16 asReserve;
} TDS_NV_PaPower;

typedef struct {
    VOS_UINT16 path;  /* 射频通路编号 */
    VOS_UINT16 group; /* path的补充 */
} TDS_SET_TselrfReq;

typedef struct {
    VOS_UINT16 mode; /* 模式 */
    VOS_UINT16 band; /* band */
    VOS_UINT16 freq; /* 频点 */
    VOS_UINT16 rsv;
} TDS_SET_FchanReq;

typedef struct {
    VOS_UINT16 type; /* 波形类型,0:单音 */
    VOS_UINT16 amp;  /* 发射的波形的功率 */
} TDS_SET_FwaveReq;

typedef struct {
    VOS_UINT16 swtich; /* 开关类型,0:close;1:open */
    VOS_UINT16 rsv;
} TDS_SET_FtxonReq;

typedef struct {
    VOS_UINT16 swtich; /* 开关类型,0:close;1:open */
    VOS_UINT16 rsv;
} TDS_SET_FrxonReq;

typedef struct {
    VOS_UINT32 rsv;
} TDS_SET_FrssiReq;

typedef struct {
    VOS_UINT32 errCode;
} TDS_SET_Cnf;

typedef struct {
    VOS_INT32  value;   /* RSSI值 */
    VOS_UINT32 errCode; /* 错误码 */
} TDS_SET_FrssiCnf;

/* adde by yangzhi for */
/*
 * 描述:   查询TDS的RF通路，用于^TSELRF
 * ID:     ID_MSG_TDS_FRSSI_REQ,ID_MSG_TDS_FRSSI_CNF
 * 结构:   ID_MSG_TDS_RD_TSELRF_REQ,ID_MSG_TDS_RD_TSELRF_CNF
 */
typedef struct {
    VOS_UINT16 chanelNo; /* DSP 不用管 */
    VOS_UINT16 rsv;
    VOS_UINT32 errCode; /* 错误码 */
} TDS_RdTselrfCnf;

/*
 * 描述:   查询TDS的通道属性，用于^FCHAN
 * ID:     ID_MSG_TDS_RD_FCHAN_REQ,ID_MSG_TDS_RD_FCHAN_CNF
 * 结构:
 */
typedef struct {
    FCHAN_MODE_ENUM fchanMode;
    VOS_UINT16      band;
    VOS_UINT16      ulFReqValid; /* DSP 不用管 */
    VOS_UINT16      dlFreqValid; /* DSP 不用管 */
    VOS_UINT16      ulChannel;
    VOS_UINT16      dlChannel;
    VOS_UINT16      rsv;
    VOS_UINT32      errCode; /* 错误码 */
} TDS_RdFchanCnf;

/*
 * 描述:   查询TDS的发射机通道属性，用于^FTXON
 * ID:     ID_MSG_TDS_RD_FTXON_REQ,ID_MSG_TDS_RD_FTXON_CNF
 * 结构:
 */
typedef struct {
    FTM_TXON_SWT_ENUM swtich;
    VOS_UINT32        errCode;
} TDS_RdFtxonCnf;

/*
 * 描述:   查询TDS的接收机通道属性，用于^FRXON
 * ID:     ID_MSG_TDS_RD_FRXON_REQ,ID_MSG_TDS_RD_FRXON_CNF
 * 结构:
 */
typedef struct {
    VOS_UINT32 rxSwt;
    VOS_UINT32 errCode;
} TDS_RdFrxonCnf;
#define FTM_STATUS_EXECUTING 0 /* IP2校准执行中 */
#define FTM_STATUS_EXECUTED 1  /* IP2校准执行完毕 */

/* IP2校准最大频点数 */
#define FTM_CALIIP2_MAX_CHAN_NUM 4

/* DCOC校准AGC档位个数 */
#define FTM_CALIDCOCS_AGC_NUM 16

/* GainState参数最大个数 */
#define FTM_GAINSTATE_MAX_NUM 60
#define FTM_GAINSTATE_MAX_NUM_HI6362 (16 * 2 * 3) /* k3v5 */

/* DBB衰减参数最大个数 */
#define FTM_DBBATT_MAX_NUM FTM_GAINSTATE_MAX_NUM

/* BB衰减参数最大个数 */
#define FTM_BBATT_MAX_NUM FTM_GAINSTATE_MAX_NUM

/* AT^FCALIIPS */
typedef struct {
    VOS_UINT32 rsv;
} FTM_RdNopraraReq;

typedef struct {
    VOS_UINT16 chanNum;
    VOS_UINT16 chan[FTM_CALIIP2_MAX_CHAN_NUM];
} FTM_SetFcaliip2SReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT16 status;
    VOS_UINT16 chanNum;
    VOS_UINT16 chan[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainIDivOff[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainQDivOff[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainIDivOn[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainQDivOn[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 divI[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 divQ[FTM_CALIIP2_MAX_CHAN_NUM];
} FTM_RdFcaliip2SCnf;

/* AT^FCALIDCOCS */
typedef struct {
    VOS_UINT16 channel;
    VOS_UINT16 rsv;
} FTM_SetFcalidcocsReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT16 status;
    VOS_UINT16 channel;
    VOS_UINT16 noblkAnT1I[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 noblkAnT1Q[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 blkAnT1I[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 blkAnT1Q[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 noblkAnT2I[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 noblkAnT2Q[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 blkAnT2I[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 blkAnT2Q[FTM_CALIDCOCS_AGC_NUM];
} FTM_RdFcalidcocsCnf;

/* AT^FCALIIP2SMRF */
#define FTM_CT_MAX_RFIC_PATH_NUM 2
#define FTM_CT_MAX_RX_ANT_NUM 2

typedef struct {
    VOS_UINT16 chanNum;
    VOS_UINT16 band;
    VOS_UINT16 rfid;
    VOS_UINT16 chan[FTM_CALIIP2_MAX_CHAN_NUM];
} FTM_SetFcaliip2SmrfReq;

typedef struct {
    VOS_UINT16 validFlg;
    VOS_UINT16 rsv;
    VOS_UINT16 mainIDivOff[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainQDivOff[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainIDivOn[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 mainQDivOn[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 divI[FTM_CALIIP2_MAX_CHAN_NUM];
    VOS_UINT16 divQ[FTM_CALIIP2_MAX_CHAN_NUM];
} FTM_FCALIIP2SMRF_CAL_ITEM;

typedef struct {
    VOS_UINT32                errCode;
    VOS_UINT16                status;
    VOS_UINT16                chanNum;
    VOS_UINT16                chan[FTM_CALIIP2_MAX_CHAN_NUM];
    FTM_FCALIIP2SMRF_CAL_ITEM ip2CalRst[FTM_CT_MAX_RFIC_PATH_NUM];
} FTM_RdFcaliip2SmrfCnf;

/* AT^FCALIDCOCSMRF */
typedef struct {
    VOS_UINT16 channel; /* 信道号 */
    VOS_UINT16 band;    /* band   */
    VOS_UINT16 rfid;    /* 通道号 */
    VOS_UINT16 rsv;
} FTM_SetFcalidcocsmrfReq;

typedef struct {
    VOS_UINT16 antCompDci[FTM_CALIDCOCS_AGC_NUM];
    VOS_UINT16 antCompDcq[FTM_CALIDCOCS_AGC_NUM];
} FTM_FcalidcocsmrfAntItem;

typedef struct {
    VOS_UINT16               validFlg;
    VOS_UINT16               validAntNum;
    FTM_FcalidcocsmrfAntItem antCal[FTM_CT_MAX_RX_ANT_NUM];
} FTM_FcalidcocsmrfDccompItem;

typedef struct {
    VOS_UINT32                  errCode;
    VOS_UINT16                  status;
    VOS_UINT16                  channel;
    FTM_FcalidcocsmrfDccompItem dcocCalRst[FTM_CT_MAX_RFIC_PATH_NUM];
} FTM_RdFcalidcocsmrfCnf;

/* AT^FGAINSTATES */
typedef struct {
    VOS_UINT16 gainStateNum;
    VOS_UINT16 rsv;
    VOS_UINT16 gainState[FTM_GAINSTATE_MAX_NUM_HI6362];
} FTM_SetFgainstatesReq;

/* AT^FDBBATTS */
typedef struct {
    VOS_UINT16 dbbCodeNum;
    VOS_UINT16 dbbCode;
} FTM_SetFdbbattsReq;

/* AT^FBBATTS */
typedef struct {
    VOS_UINT16 bbAttNum;
    VOS_UINT16 rsv;
    VOS_UINT16 bbAtt[FTM_BBATT_MAX_NUM];
} FTM_SetFbbattsReq;

/* AT^FCALITXIQS */
typedef struct {
    VOS_UINT16 channel;
    VOS_UINT16 band;
    VOS_UINT16 rfid;
    VOS_UINT16 rsv2;
} FTM_SetFcalitxiqsReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT16 status;
    VOS_UINT16 amplitude;
    VOS_UINT16 phase;
    VOS_UINT16 dci;
    VOS_UINT16 dcq;
    VOS_UINT16 reserved;
} FTM_RdFcalitxiqsCnf;

#define FTM_MAX_COMM_CMD_LEN 1152
typedef enum {
    EN_LTCOMMCMD_DEST_LDSP_CT = 0,
    EN_LTCOMMCMD_DEST_LDSP_BT = 1,
    EN_LTCOMMCMD_DEST_TDSP_CT = 2,
    EN_LTCOMMCMD_DEST_TDSP_BT = 3,
} FTM_LTCOMMCMD_DEST;

typedef struct {
    FTM_LTCOMMCMD_DEST cmdDest;
    VOS_UINT32         dataLen;
    VOS_CHAR           data[0];  //lint !e43
} FTM_SetLtcommcmdReq;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT32 dataLen;
    VOS_CHAR   data[0];  //lint !e43
} FTM_SetLtcommcmdCnf;

typedef struct {
    VOS_UINT32 errCode;
    VOS_UINT32 dataLen;
    /* 由于引用LMspLPhyInterface.h头文件后，会引起工具编译不过，这里直接用数据，大小需要与宏MAX_COMM_CMD_LEN保持一致辞 */
    VOS_CHAR data[1152];
} FTM_RdLtcommcmdCnf;

typedef struct {
    VOS_UINT32 msgId;                 /* 消息标识 */
    VOS_UINT32 dataLen;               /* 数据长度 */
    VOS_UINT8  data[0]; /* 数据     */  //lint !e43
} FTM_PHY_CommCmdSetReq;

typedef struct {
    VOS_UINT32 msgId;                 /* 消息标识 */
    VOS_UINT32 dataLen;               /* 数据长度 */
    VOS_UINT8  data[0]; /* 数据     */  //lint !e43
} PHY_FTM_CommCmdSetCnf;

typedef PHY_FTM_CommCmdSetCnf PHY_FTM_COMM_CMD_IND_STRU;

typedef struct {
    VOS_MSG_HEADER         /* _H2ASN_Skip */
    VOS_UINT32      msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl ctrl;
} L4A_COMM_Req;
typedef L4A_COMM_Req L4A_READ_LWCLASH_REQ_STRU;

typedef struct {
    VOS_UINT32 ulFreq;      /* 上行中心频点单位:100Khz */
    VOS_UINT32 dlFreq;      /* 下行中心频点 单位:100Khz */
    VOS_UINT16 ulBandwidth; /* 上行带宽 */
    VOS_UINT16 dlBandwidth; /* 上行带宽 */
    VOS_UINT8  camped;      /* 是否驻留 */
    VOS_UINT8  state;       /* 是否为冲突状态 */
    VOS_UINT8  band;        /* 频带指示 */
    APP_RRC_AntennaMaxLayersMimoUint8 dlMimo;
} L4A_LWCLASH_Info;

typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    VOS_UINT32         msgId;    /* _H2ASN_Skip */

    VOS_UINT16         clientId; /* 客户端ID */
    VOS_UINT8          opId;
    PS_BOOL_ENUM_UINT8 dl256QamFlag;
    VOS_UINT32         errorCode;

    L4A_LWCLASH_Info   lwclashInfo;
    VOS_UINT32         scellNum;
    RRC_APP_ScellInfo  scellInfo[LRRC_APP_LWCLASH_MAX_SCELL_NUM];
} L4A_READ_LwclashCnf;

typedef struct {
    VOS_MSG_HEADER            /* _H2ASN_Skip */
    VOS_UINT32         msgId; /* _H2ASN_Skip */

    VOS_UINT16         clientId; /* 客户端ID */
    VOS_UINT8          opId;     /* 操作码ID */
    PS_BOOL_ENUM_UINT8 dl256QamFlag;
    L4A_LWCLASH_Info   lwclashInfo;
    VOS_UINT32         scellNum;
    RRC_APP_ScellInfo  scellInfo[LRRC_APP_LWCLASH_MAX_SCELL_NUM];
} L4A_READ_LwclashInd;

typedef L4A_COMM_Req L4A_READ_LCACELL_REQ_STRU;

typedef struct {
    VOS_UINT8 ulConfigured; /* CA上行是否被配置，0:未配置，1:已配置 */
    VOS_UINT8 dlConfigured; /* CA下行是否被配置，0:未配置，1:已配置 */
    VOS_UINT8 actived;      /* SCell是否被激活，0:去激活，1:激活 */
    VOS_UINT8 rsv;
} L4A_CACELL_Info;

typedef struct {
    VOS_MSG_HEADER                /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId;    /* _H2ASN_Skip */

    VOS_UINT16          clientId; /* 客户端ID */
    VOS_UINT8           opId;
    VOS_UINT8           resv[1];
    VOS_UINT32          errorCode;

    L4A_CACELL_Info     lcacellInfo[CA_MAX_CELL_NUM];
} L4A_READ_LcacellCnf;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId; /* 客户端ID */
    VOS_UINT8           opId;     /* 操作码ID */
    VOS_UINT8           resv[1];  /* CID */
    L4A_CACELL_Info     lcacellInfo[CA_MAX_CELL_NUM];
} L4A_READ_LcacellInd;

/*
 * 描述:   ^ISMCOEX
 * ID:
 * 结构:
 */
#define L4A_ISMCOEX_BANDWIDTH_NUM 6

typedef struct {
    VOS_UINT32 flag;
    VOS_UINT32 txBegin;
    VOS_UINT32 txEnd;
    VOS_INT32  txPower;
    VOS_UINT32 rxBegin;
    VOS_UINT32 rxEnd;
} ISMCOEX_INFO;

typedef struct {
    VOS_MSG_HEADER
    APS_L4A_MsgIdUint32 msgId;

    APS_L4A_Appctrl ctrl;

    ISMCOEX_INFO coex[L4A_ISMCOEX_BANDWIDTH_NUM];
} L4A_ISMCOEX_Req;

typedef struct {
    VOS_MSG_HEADER
    APS_L4A_MsgIdUint32 msgId;

    VOS_UINT16 clientId;
    VOS_UINT8  opId;
    VOS_UINT8  resv[1];
    VOS_UINT32 errorCode;
} L4A_ISMCOEX_Cnf;

typedef L4A_COMM_Req L4A_READ_LTECS_REQ_STRU;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT8           n;
    VOS_UINT8           reserved[3];
} L4A_READ_CnmrReq;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId;  /* 客户端ID */
    VOS_UINT8           opId;      /* 操作码ID */
    VOS_UINT8           reserved[1];
    VOS_UINT32          errorCode; /* 错误码 */
    RRC_APP_LteCellInfo lteCelInfo;
} L4A_READ_CnmrCnf;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT8           ucnDb;
    VOS_UINT8           minRptTimerInterval;
    VOS_UINT8           reserved[2];
} L4A_CSQ_InfoReq;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId;  /* 客户端ID */
    VOS_UINT8           opId;      /* 操作码ID */
    VOS_UINT8           reserved[1];
    VOS_UINT32          errorCode; /* 错误码 */

    VOS_INT16           rssi; /* Rssi */
    VOS_INT16           rsd;  /* 填充字段 */
    VOS_INT16           rsrp; /* 范围：(-141,-44), 99为无效 */
    VOS_INT16           rsrq; /* 范围：(-40, -6) , 99为无效 */
    VOS_INT32           sinr; /* SINR RS_SNR */
    APP_RRC_CqiInfo     cqi;  /* CQI两个码字 */
} L4A_CSQ_InfoCnf;

typedef L4A_CSQ_InfoCnf L4A_CSQ_INFO_IND_STRU;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId;  /* 客户端ID */
    VOS_UINT8           opId;      /* 操作码ID */
    VOS_UINT8           reserved[1];
    VOS_UINT32          errorCode; /* 错误码 */

    VOS_INT16           rssi;  /* Rssi */
    VOS_INT16           level; /*  */
    VOS_INT16           rsrp;  /* 范围：(-141,-44), 99为无效 */
    VOS_INT16           rsrq;  /* 范围：(-40, -6) , 99为无效 */
} AT_AnlevelInfoCnf;

#define L4A_CURC_RPT_CFG_MAX_SIZE 8
typedef struct {
    VOS_UINT32 modeBitValid : 1;    /* save mode set command-- 1:valid 0:invalid */
    VOS_UINT32 modeBit : 1;         /* save mode set command-- 1:open 0:close */
    VOS_UINT32 lwurcBitValid : 1;   /* save lwclash set command- 1:valid 0:invalid */
    VOS_UINT32 lwurcBit : 1;        /* save lwclash set command- 1:open 0:close */
    VOS_UINT32 cerssiBitValid : 1;  /* save cerssi set command-- 1:valid 0:invalid */
    VOS_UINT32 cerssiBit : 1;       /* save cerssi set command-- 1:open 0:close */
    VOS_UINT32 rssiBitValid : 1;    /* save rssi set command--     1:valid 0:invalid */
    VOS_UINT32 rssiBit : 1;         /* save rssi set command--     1:open 0:close */
    VOS_UINT32 anlevelBitValid : 1; /* save anlevel set command--1:valid 0:invalid */
    VOS_UINT32 anlevelBit : 1;      /* save anlevel set command--1:open 0:close */
    VOS_UINT32 lcacellBitValid : 1; /* save lcacell set command--1:valid 0:invalid */
    VOS_UINT32 lcacellBit : 1;      /* save lcacell set command--1:open 0:close */
    VOS_UINT32 curcBitValid : 1;    /* save cruc=2                      --1:valid 0;invalid */
    VOS_UINT32 reserved : 21;
    VOS_UINT8  curcCfgReq[L4A_CURC_RPT_CFG_MAX_SIZE]; /* save curc等于2 */
} L4A_IND_Cfg;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          cellFlag;
} L4A_READ_CellInfoReq;

typedef struct {
    VOS_MSG_HEADER                /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32    msgId; /* _H2ASN_Skip */

    VOS_UINT16             clientId;  /* 客户端ID */
    VOS_UINT8              opId;      /* 操作码ID */
    VOS_UINT8              reserved[1];
    VOS_UINT32             errorCode; /* 错误码 */
    LRRC_APP_NcellListInfo ncellListInfo;
} L4A_READ_CellInfoCnf;

typedef L4A_COMM_Req L4A_READ_CELL_ID_REQ_STRU;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId; /* 客户端ID */
    VOS_UINT8           opId;     /* 操作码ID */
    VOS_UINT8           reserved[1];
    VOS_UINT32          errorCode; /* 错误码. */
    VOS_UINT16          mncNum;    /* ?? MNC ?? */
    VOS_UINT16          mcc;
    VOS_UINT16          mnc;
    VOS_UINT16          rev;
    VOS_UINT32          ci;  /* 0xFFFF???, */
    VOS_UINT16          pci; /* 0xFFFF???,??:(0,503) */
    VOS_UINT16          tac; /* TAC */
} L4A_READ_CellIdCnf;

typedef struct {
    VOS_UINT16 mcc; /* Mobile Country Code , 0x460 */
    VOS_UINT16 mnc; /* Mobile Network Code ,如果MNC只有两位,则高八位为0x0f */
} L4A_PLMN_Id;

typedef L4A_COMM_Req L4A_SET_LTE_TO_IDLE_REQ_STRU;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          flag;
} L4A_LW_ThreasholdCfgReq;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_INT32           flag;
    VOS_INT32           timerLen;
} L4A_SET_FastDormReq;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    L4A_IND_Cfg         indCfg;
} L4A_IND_CfgReq;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId; /* 客户端ID */
    VOS_UINT8           opId;     /* 操作码ID */
    VOS_UINT8           reserved[1];
    VOS_UINT32          errorCode; /* 错误码 */
} L4A_AT_CnfHeader;

typedef struct {
    VOS_MSG_HEADER             /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    APS_L4A_Appctrl     ctrl;
    VOS_UINT32          mod;
    VOS_UINT32          ver;
} L4A_SET_RadverReq;

typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    APS_L4A_MsgIdUint32 msgId; /* _H2ASN_Skip */

    VOS_UINT16          clientId; /* 客户端ID */
    VOS_UINT8           opId;     /* 操作码ID */
    VOS_UINT8           reserved;
    VOS_UINT32          errorCode; /* 错误码 */
} L4A_SET_RadverCnf;

typedef L4A_COMM_Req L4A_READ_ScellInfoReq;

typedef struct {
    VOS_MSG_HEADER
    APS_L4A_MsgIdUint32 msgId;
    VOS_UINT16          clientId;
    VOS_UINT8           opId;
    VOS_UINT8           reserved;
    VOS_UINT32          errorCode;
    VOS_UINT32          sCellCnt;
    RRC_SCELL_Info      sCellInfo[LRRC_SCELL_MAX_NUM];
} L4A_READ_ScellInfoCnf;

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __GEN_MSG_H__ */
