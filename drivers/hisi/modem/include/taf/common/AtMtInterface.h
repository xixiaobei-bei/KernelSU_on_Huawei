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

#ifndef __ATMTINTERFACE_H__
#define __ATMTINTERFACE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "product_config.h"
#if(FEATURE_ON == FEATURE_UE_MODE_NR)
#include "bbic_cal_shared.h"
#include "nascbtinterface.h"
#include "bbic_cal.h"
#endif
#include "TafDrvAgent.h"


#pragma pack(4)

#if(FEATURE_ON == FEATURE_UE_MODE_NR)
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 将BBIC CAL模块定义的消息转换成AT和BBIC模块间的消息 */
#define ID_AT_BBIC_CAL_RF_DEBUG_TX_REQ          ID_TOOL_BBIC_CAL_RF_DEBUG_TX_REQ            /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_TX_REQ_STRU           */
#define ID_BBIC_AT_CAL_MSG_CNF                  ID_BBIC_TOOL_CAL_MSG_CNF                    /* _H2ASN_MsgChoice BBIC_CAL_MSG_CNF_STRU                   */
#define ID_BBIC_AT_CAL_RF_DEBUG_TX_RESULT_IND   ID_BBIC_TOOL_CAL_RF_DEBUG_TX_RESULT_IND     /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_TX_RESULT_IND_STRU    */
#define ID_AT_BBIC_CAL_RF_DEBUG_GSM_TX_REQ      ID_TOOL_BBIC_CAL_RF_DEBUG_GSM_TX_REQ        /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU       */
#define ID_BBIC_AT_CAL_RF_DEBUG_GTX_MRX_IND     ID_BBIC_TOOL_CAL_RF_DEBUG_GTX_MRX_IND       /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU      */
#define ID_AT_BBIC_CAL_RF_DEBUG_RX_REQ          ID_TOOL_BBIC_CAL_RF_DEBUG_RX_REQ            /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_RX_REQ_STRU           */
#define ID_AT_BBIC_CAL_RF_DEBUG_RSSI_REQ        ID_TOOL_BBIC_CAL_RF_DEBUG_RSSI_REQ          /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_RSSI_REQ_STRU         */
#define ID_BBIC_AT_CAL_RF_DEBUG_RX_RSSI_IND     ID_BBIC_TOOL_CAL_RF_DEBUG_RX_RSSI_IND       /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_RX_RSSI_IND_STRU      */
#define ID_AT_BBIC_MIPI_READ_REQ                ID_TOOL_BBIC_MIPI_READ_REQ                  /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU    */
#define ID_BBIC_AT_MIPI_READ_CNF                ID_BBIC_TOOL_MIPI_READ_IND                  /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU    */
#define ID_AT_BBIC_MIPI_WRITE_REQ               ID_TOOL_BBIC_MIPI_WRITE_REQ                 /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU   */
#define ID_BBIC_AT_MIPI_WRITE_CNF               ID_BBIC_TOOL_MIPI_WRITE_IND                 /* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU   */
#define ID_AT_BBIC_PLL_QRY_REQ                  ID_TOOL_BBIC_PLL_QRY_REQ                    /* _H2ASN_MsgChoice BBIC_CAL_PLL_QRY_REQ_STRU               */
#define ID_BBIC_AT_PLL_QRY_CNF                  ID_BBIC_TOOL_PLL_QRY_IND                    /* _H2ASN_MsgChoice BBIC_CAL_PLL_QRY_IND_STRU               */
#define ID_AT_BBIC_TEMP_QRY_REQ                 ID_TOOL_BBIC_TEMP_QRY_REQ                   /* _H2ASN_MsgChoice BBIC_CAL_TEMP_QRY_REQ_STRU              */
#define ID_BBIC_AT_TEMP_QRY_CNF                 ID_BBIC_TOOL_TEMP_QRY_IND                   /* _H2ASN_MsgChoice BBIC_CAL_TEMP_QRY_IND_STRU              */
#define ID_AT_BBIC_DPDT_REQ                     ID_TOOL_BBIC_DPDT_REQ                       /* _H2ASN_MsgChoice BBIC_CAL_DPDT_REQ_STRU                  */
#define ID_BBIC_AT_DPDT_CNF                     ID_BBIC_TOOL_DPDT_IND                       /* _H2ASN_MsgChoice BBIC_CAL_DPDT_IND_STRU                  */
#define ID_AT_BBIC_DCXO_REQ                     ID_TOOL_BBIC_CAL_RF_DEBUG_DCXO_REQ          /* _H2ASN_MsgChoice BBIC_CAL_DCXO_REQ_STRU                  */  
#define ID_BBIC_AT_DCXO_CNF                     ID_BBIC_TOOL_CAL_RF_DEBUG_DCXO_IND         /* _H2ASN_MsgChoice BBIC_CAL_DCXO_CNF_STRU                  */


#define FREQ_UNIT_MHZ_TO_KHZ                    (1000)
#define LTE_CHANNEL_TO_FREQ_UNIT                (100)
#define W_CHANNEL_FREQ_TIMES                    (5)

#define G_CHANNEL_NO_124                        (124)
#define G_CHANNEL_NO_955                        (955)
#define G_CHANNEL_NO_1023                       (1023)

#define C_CHANNEL_NO_1                          (1)
#define C_CHANNEL_NO_799                        (799)
#define C_CHANNEL_NO_991                        (991)
#define C_CHANNEL_NO_1023                       (1023)
#define C_CHANNEL_NO_1024                       (1024)
#define C_CHANNEL_NO_1323                       (1323)

#define FWAVE_TYPE_CONTINUE                     (0xFF)

#define AT_MT_LOAD_DSP_FTM_MODE                 (5)

#define AT_MT_MIPI_READ_MAX_BYTE                (4)

#define MT_OK                                   (0)
#define MT_ERR                                  (1)


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16                          usMsgId;
    VOS_UINT16                          usRev;
}AT_MT_MSG_HEADER_STRU;

/* 填写消息头 */
#define AT_CFG_MT_MSG_HDR(pstMsg, ulRecvPid, usSndMsgId)\
            ((AT_MT_MSG_HEADER_STRU *)(pstMsg))->ulSenderCpuId   = VOS_LOCAL_CPUID;\
            ((AT_MT_MSG_HEADER_STRU *)(pstMsg))->ulSenderPid     = WUEPS_PID_AT;\
            ((AT_MT_MSG_HEADER_STRU *)(pstMsg))->ulReceiverCpuId = VOS_LOCAL_CPUID;\
            ((AT_MT_MSG_HEADER_STRU *)(pstMsg))->ulReceiverPid   = (ulRecvPid);\
            ((AT_MT_MSG_HEADER_STRU *)(pstMsg))->usMsgId         = (usSndMsgId);

/* 获取消息内容开始地址 */
#define AT_MT_GET_MSG_ENTITY(pstMsg)\
            ((VOS_VOID *)&(((AT_MT_MSG_HEADER_STRU *)(pstMsg))->usMsgId))

/* 获取消息长度 */
#define AT_MT_GET_MSG_LENGTH(pstMsg)\
            (((MSG_HEADER_STRU *)(pstMsg))->ulLength)


/* 封装消息初始化消息内容接口 */
#define AT_MT_CLR_MSG_ENTITY(pstMsg)\
            TAF_MEM_SET_S(AT_MT_GET_MSG_ENTITY(pstMsg), AT_MT_GET_MSG_LENGTH(pstMsg), 0x00, AT_MT_GET_MSG_LENGTH(pstMsg))


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum AT_PROTOCOL_BAND_ENUM
{
    AT_PROTOCOL_BAND_1                  =   1,
    AT_PROTOCOL_BAND_2                  =   2,
    AT_PROTOCOL_BAND_3                  =   3,
    AT_PROTOCOL_BAND_4                  =   4,
    AT_PROTOCOL_BAND_5                  =   5,
    AT_PROTOCOL_BAND_6                  =   6,
    AT_PROTOCOL_BAND_7                  =   7,
    AT_PROTOCOL_BAND_8                  =   8,
    AT_PROTOCOL_BAND_9                  =   9,
    AT_PROTOCOL_BAND_10                 =   10,
    AT_PROTOCOL_BAND_11                 =   11,
    AT_PROTOCOL_BAND_12                 =   12,
    AT_PROTOCOL_BAND_13                 =   13,
    AT_PROTOCOL_BAND_14                 =   14,
    AT_PROTOCOL_BAND_15                 =   15,
    AT_PROTOCOL_BAND_16                 =   16,
    AT_PROTOCOL_BAND_17                 =   17,
    AT_PROTOCOL_BAND_18                 =   18,
    AT_PROTOCOL_BAND_19                 =   19,
    AT_PROTOCOL_BAND_20                 =   20,
    AT_PROTOCOL_BAND_21                 =   21,
    AT_PROTOCOL_BAND_22                 =   22,
    AT_PROTOCOL_BAND_23                 =   23,
    AT_PROTOCOL_BAND_24                 =   24,
    AT_PROTOCOL_BAND_25                 =   25,
    AT_PROTOCOL_BAND_26                 =   26,
    AT_PROTOCOL_BAND_27                 =   27,
    AT_PROTOCOL_BAND_28                 =   28,
    AT_PROTOCOL_BAND_29                 =   29,
    AT_PROTOCOL_BAND_30                 =   30,
    AT_PROTOCOL_BAND_31                 =   31,
    AT_PROTOCOL_BAND_32                 =   32,
    AT_PROTOCOL_BAND_33                 =   33,
    AT_PROTOCOL_BAND_34                 =   34,
    AT_PROTOCOL_BAND_35                 =   35,
    AT_PROTOCOL_BAND_36                 =   36,
    AT_PROTOCOL_BAND_37                 =   37,
    AT_PROTOCOL_BAND_38                 =   38,
    AT_PROTOCOL_BAND_39                 =   39,
    AT_PROTOCOL_BAND_40                 =   40,
    AT_PROTOCOL_BAND_41                 =   41,
    AT_PROTOCOL_BAND_42                 =   42,
    AT_PROTOCOL_BAND_43                 =   43,
    AT_PROTOCOL_BAND_44                 =   44,
    AT_PROTOCOL_BAND_45                 =   45,
    AT_PROTOCOL_BAND_46                 =   46,
    AT_PROTOCOL_BAND_47                 =   47,
    AT_PROTOCOL_BAND_48                 =   48,
    AT_PROTOCOL_BAND_49                 =   49,
    AT_PROTOCOL_BAND_50                 =   50,
    AT_PROTOCOL_BAND_51                 =   51,
    AT_PROTOCOL_BAND_52                 =   52,
    AT_PROTOCOL_BAND_53                 =   53,
    AT_PROTOCOL_BAND_54                 =   54,
    AT_PROTOCOL_BAND_55                 =   55,
    AT_PROTOCOL_BAND_56                 =   56,
    AT_PROTOCOL_BAND_57                 =   57,
    AT_PROTOCOL_BAND_58                 =   58,
    AT_PROTOCOL_BAND_59                 =   59,
    AT_PROTOCOL_BAND_60                 =   60,
    AT_PROTOCOL_BAND_61                 =   61,
    AT_PROTOCOL_BAND_62                 =   62,
    AT_PROTOCOL_BAND_63                 =   63,
    AT_PROTOCOL_BAND_64                 =   64,
    AT_PROTOCOL_BAND_65                 =   65,
    AT_PROTOCOL_BAND_66                 =   66,

    AT_PROTOCOL_BAND_70                 =   70,
    AT_PROTOCOL_BAND_71                 =   71,

    AT_PROTOCOL_BAND_74                 =   74,
    AT_PROTOCOL_BAND_75                 =   75,
    AT_PROTOCOL_BAND_76                 =   76,
    AT_PROTOCOL_BAND_77                 =   77,
    AT_PROTOCOL_BAND_78                 =   78,
    AT_PROTOCOL_BAND_79                 =   79,
    AT_PROTOCOL_BAND_80                 =   80,
    AT_PROTOCOL_BAND_81                 =   81,
    AT_PROTOCOL_BAND_82                 =   82,
    AT_PROTOCOL_BAND_83                 =   83,
    AT_PROTOCOL_BAND_84                 =   84,

    AT_PROTOCOL_BAND_256                =   256,
    AT_PROTOCOL_BAND_257                =   257,
    AT_PROTOCOL_BAND_258                =   258,
    AT_PROTOCOL_BAND_259                =   259,
    AT_PROTOCOL_BAND_260                =   260,
    AT_PROTOCOL_BAND_261                =   261,

    AT_PROTOCOL_BAND_BUTT
};
typedef VOS_UINT16  AT_PROTOCOL_BAND_ENUM_UINT16;


enum AT_BAND_WIDTH_ENUM
{
    AT_BAND_WIDTH_200K                  = 0,
    AT_BAND_WIDTH_1M2288                = 1,
    AT_BAND_WIDTH_1M28                  = 2,
    AT_BAND_WIDTH_1M4                   = 3,
    AT_BAND_WIDTH_3M                    = 4,
    AT_BAND_WIDTH_5M                    = 5,
    AT_BAND_WIDTH_10M                   = 6,
    AT_BAND_WIDTH_15M                   = 7,
    AT_BAND_WIDTH_20M                   = 8,
    AT_BAND_WIDTH_25M                   = 9,
    AT_BAND_WIDTH_30M                   = 10,
    AT_BAND_WIDTH_40M                   = 11,
    AT_BAND_WIDTH_50M                   = 12,
    AT_BAND_WIDTH_60M                   = 13,
    AT_BAND_WIDTH_80M                   = 14,
    AT_BAND_WIDTH_90M                   = 15,
    AT_BAND_WIDTH_100M                  = 16,
    AT_BAND_WIDTH_200M                  = 17,
    AT_BAND_WIDTH_400M                  = 18,
    AT_BAND_WIDTH_800M                  = 19,
    AT_BAND_WIDTH_1G                    = 20,

    AT_BAND_WIDTH_BUTT
};
typedef VOS_UINT16   AT_BAND_WIDTH_ENUM_UINT16;


enum AT_SUB_CARRIER_SPACING_ENUM
{
    AT_SUB_CARRIER_SPACING_15K          = 0,
    AT_SUB_CARRIER_SPACING_30K          = 1,
    AT_SUB_CARRIER_SPACING_60K          = 2,
    AT_SUB_CARRIER_SPACING_120K         = 3,
    AT_SUB_CARRIER_SPACING_240K         = 4,

    AT_SUB_CARRIER_SPACING_BUTT
};
typedef VOS_UINT8  AT_SUB_CARRIER_SPACING_ENUM_UINT8;


enum AT_DUPLEX_MODE_ENUM
{
    AT_DUPLEX_MODE_FDD                  = 0,
    AT_DUPLEX_MODE_TDD                  = 1,
    AT_DUPLEX_MODE_SDL                  = 2,
    AT_DUPLEX_MODE_SUL                  = 3,

    AT_DUPLEX_MODE_BUTT
};
typedef VOS_UINT16  AT_DUPLEX_MODE_ENUM_UINT16;


enum AT_FWAVE_TYPE_ENUM
{
    AT_FWAVE_TYPE_BPSK                  = 0,
    AT_FWAVE_TYPE_PI2_BPSK              = 1,
    AT_FWAVE_TYPE_QPSK                  = 2,
    AT_FWAVE_TYPE_16QAM                 = 3,
    AT_FWAVE_TYPE_64QAM                 = 4,
    AT_FWAVE_TYPE_256QAM                = 5,
    AT_FWAVE_TYPE_GMSK                  = 6,
    AT_FWAVE_TYPE_8PSK                  = 7,
    AT_FWAVE_TYPE_CONTINUE              = 8,

    AT_FWAVE_TYPE_BUTT
};
typedef VOS_UINT8  AT_FWAVE_TYPE_ENUM_UINT8;


enum AT_BAND_WIDTH_VALUE_ENUM
{
    AT_BAND_WIDTH_VALUE_200K                  = 200,
    AT_BAND_WIDTH_VALUE_1M2288                = 1228,
    AT_BAND_WIDTH_VALUE_1M28                  = 1280,
    AT_BAND_WIDTH_VALUE_1M4                   = 1400,
    AT_BAND_WIDTH_VALUE_3M                    = 3000,
    AT_BAND_WIDTH_VALUE_5M                    = 5000,
    AT_BAND_WIDTH_VALUE_10M                   = 10000,
    AT_BAND_WIDTH_VALUE_15M                   = 15000,
    AT_BAND_WIDTH_VALUE_20M                   = 20000,
    AT_BAND_WIDTH_VALUE_25M                   = 25000,
    AT_BAND_WIDTH_VALUE_30M                   = 30000,
    AT_BAND_WIDTH_VALUE_40M                   = 40000,
    AT_BAND_WIDTH_VALUE_50M                   = 50000,
    AT_BAND_WIDTH_VALUE_60M                   = 60000,
    AT_BAND_WIDTH_VALUE_80M                   = 80000,
    AT_BAND_WIDTH_VALUE_90M                   = 90000,
    AT_BAND_WIDTH_VALUE_100M                  = 100000,
    AT_BAND_WIDTH_VALUE_200M                  = 200000,
    AT_BAND_WIDTH_VALUE_400M                  = 400000,
    AT_BAND_WIDTH_VALUE_800M                  = 800000,
    AT_BAND_WIDTH_VALUE_1G                    = 1000000,

    AT_BAND_WIDTH_VALUE_BUTT
};
typedef VOS_UINT32   AT_BAND_WIDTH_VALUE_ENUM_UINT32;



enum AT_CMD_RATMODE_ENUM
{
    AT_CMD_RATMODE_GSM                  = 0,
    AT_CMD_RATMODE_WCDMA                = 1,
    AT_CMD_RATMODE_LTE                  = 2,
    AT_CMD_RATMODE_TD                   = 3,
    AT_CMD_RATMODE_CDMA                 = 4,
    AT_CMD_RATMODE_NR                   = 5,
    AT_CMD_RATMODE_BUTT
};
typedef VOS_UINT8 AT_CMD_RATMODE_ENUM_UINT8;


enum AT_LOAD_DSP_RATMODE_ENUM
{
    AT_LOAD_DSP_RATMODE_GSM             = 0,
    AT_LOAD_DSP_RATMODE_WCDMA           = 1,
    AT_LOAD_DSP_RATMODE_CDMA            = 2,
    AT_LOAD_DSP_RATMODE_LTE             = 3,
    AT_LOAD_DSP_RATMODE_NR              = 4,
    AT_LOAD_DSP_RATMODE_BUTT
};
typedef VOS_UINT8 AT_LOAD_DSP_RATMODE_ENUM_UINT8;



enum AT_TEMP_CHANNEL_TYPE_ENUM
{
    AT_TEMP_CHANNEL_TYPE_LOGIC          = 0,
    AT_TEMP_CHANNEL_TYPE_PHY            = 1,

    AT_TEMP_CHANNEL_TYPE_BUTT
};
typedef UINT16 AT_TEMP_CHANNEL_TYPE_ENUM_UINT16;


enum AT_CMD_PALEVEL_ENUM
{
    AT_CMD_PALEVEL_HIGH                  = 0,
    AT_CMD_PALEVEL_MID                   = 1,
    AT_CMD_PALEVEL_LOW                   = 2,
    AT_CMD_PALEVEL_ULTRALOW              = 3,
    AT_CMD_PALEVEL_BUTT
};
typedef VOS_UINT8 AT_CMD_PALEVEL_ENUM_UINT8;


enum AT_ANT_TYPE_ENUM
{
    AT_ANT_TYPE_PRI                     = 1,                                    /* 主极 */
    AT_ANT_TYPE_DIV                     = 2,                                    /* 分极 */
    AT_ANT_TYPE_MIMO                    = 4,                                    /* MINO */

    AT_ANT_TYPE_BUTT
};
typedef VOS_UINT8   AT_ANT_TYPE_ENUM_UINT8;


enum AT_MIMO_TYPE_ENUM
{
    AT_MIMO_TYPE_2                      = 1,                                    /* 双天线 */
    AT_MIMO_TYPE_4                      = 2,                                    /* 四天线 */
    AT_MIMO_TYPE_8                      = 3,                                    /* 八天线 */

    AT_MIMO_TYPE_BUTT
};
typedef VOS_UINT8   AT_MIMO_TYPE_UINT8;


enum AT_MIMO_ANT_NUM_ENUM
{
    AT_MIMO_ANT_NUM_1                   = 1,                                    /* 第1根天线 */
    AT_MIMO_ANT_NUM_2                   = 2,                                    /* 第2根天线 */
    AT_MIMO_ANT_NUM_3                   = 3,                                    /* 第3根天线 */
    AT_MIMO_ANT_NUM_4                   = 4,                                    /* 第4根天线 */
    AT_MIMO_ANT_NUM_5                   = 5,                                    /* 第5根天线 */
    AT_MIMO_ANT_NUM_6                   = 6,                                    /* 第6根天线 */
    AT_MIMO_ANT_NUM_7                   = 7,                                    /* 第7根天线 */
    AT_MIMO_ANT_NUM_8                   = 8,                                    /* 第8根天线 */

    AT_MIMO_ANT_NUM_BUTT
};
typedef VOS_UINT8   AT_MIMO_ANT_NUM_ENUM_UINT8;
#endif


enum AT_TSELRF_PATH_ENUM
{
    AT_TSELRF_PATH_GSM                  = 1,
    AT_TSELRF_PATH_WCDMA_PRI            = 2,
    AT_TSELRF_PATH_WCDMA_DIV            = 3,
    AT_TSELRF_PATH_CDMA_PRI             = 4,
    AT_TSELRF_PATH_CDMA_DIV             = 5,
    AT_TSELRF_PATH_TD                   = 6,
    AT_TSELRF_PATH_WIFI                 = 7,
    AT_TSELRF_PATH_WIMAX                = 8,
    AT_TSELRF_PATH_FDD_LTE_PRI          = 9,
    AT_TSELRF_PATH_FDD_LTE_DIV          = 10,
    AT_TSELRF_PATH_FDD_LTE_MIMO         = 11,
    AT_TSELRF_PATH_TDD_LTE_PRI          = 12,
    AT_TSELRF_PATH_TDD_LTE_DIV          = 13,
    AT_TSELRF_PATH_TDD_LTE_MIMO         = 14,
    AT_TSELRF_PATH_NAVIGATION           = 15,
    AT_TSELRF_PATH_NR_PRI               = 16,
    AT_TSELRF_PATH_NR_DIV               = 17,
    AT_TSELRF_PATH_NR_MIMO              = 18,
    AT_TSELRF_PATH_BUTT
};
typedef VOS_UINT32  AT_TSELRF_PATH_ENUM_UINT32;

#if(FEATURE_ON == FEATURE_UE_MODE_NR)
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
//extern AT_MT_INFO_STRU                         g_stMtInfoCtx;


/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct
{
    AT_TSELRF_PATH_ENUM_UINT32          enTselPath;
    AT_ANT_TYPE_ENUM_UINT8              enAntType;
}AT_PATH_TO_ANT_TYPE_STRU;


typedef struct
{
    VOS_UINT32                          ulChannelMin;                               /* 信道最小值 */
    VOS_UINT32                          ulChannelMax;                               /* 信道最大值 */
}AT_CHANNEL_RANGE_STRU;


typedef struct
{
    VOS_UINT32                          ulFreqMin;                              /* 频率最小值,单位KHZ */
    VOS_UINT32                          ulFreqMax;                              /* 频率最大值,单位KHZ */
}AT_FREQ_RANGE_STRU;


typedef struct
{
    AT_FREQ_RANGE_STRU                  stFreqRange;
    VOS_UINT32                          ulFreqGlobal;                           /* 信道最大值,单位KHZ */
    VOS_UINT32                          ulFreqOffset;
    VOS_UINT32                          ulOffsetChannel;
    AT_CHANNEL_RANGE_STRU               stChannelRange;
}AT_NR_FREQ_OFFSET_TABLE_STRU;


typedef struct
{
    VOS_UINT16                          usBandNo;
    AT_DUPLEX_MODE_ENUM_UINT16          usMode;
    AT_CHANNEL_RANGE_STRU               stUlChannelRange;
    AT_CHANNEL_RANGE_STRU               stDlChannelRange;
    AT_FREQ_RANGE_STRU                  stUlFreqRange;
    AT_FREQ_RANGE_STRU                  stDlFreqRange;
}AT_NR_BAND_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usBandNo;
    VOS_UINT16                          usRev;
    VOS_UINT32                          ulDlLowFreq;
    VOS_UINT32                          ulDlChannelOffset;
    AT_CHANNEL_RANGE_STRU               stDlChannelRange;
    VOS_UINT32                          ulUlLowFreq;
    VOS_UINT32                          ulUlChannelOffset;
    AT_CHANNEL_RANGE_STRU               stUlChannelRange;
}AT_LTE_BAND_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usBandNo;
    VOS_UINT16                          usRev;
    VOS_UINT32                          ulUlFreqOffset;
    AT_CHANNEL_RANGE_STRU               stUlChannelRange;
    AT_FREQ_RANGE_STRU                  stUlFreqRange;
    VOS_UINT32                          ulDlFreqOffset;
    AT_CHANNEL_RANGE_STRU               stDlChannelRange;
    AT_FREQ_RANGE_STRU                  stDlFreqRange;
}AT_W_BAND_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usBandNo;
    VOS_UINT16                          usRev;
    VOS_UINT32                          ulFreqOffset;
    AT_CHANNEL_RANGE_STRU               stChannelRange;
}AT_G_BAND_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulUlChanNo;  /*上行的Channel No*/
    VOS_UINT32                          ulDlChanNo;  /*下行的Channel No*/
    VOS_UINT16                          usDspBand;  /*DSP格式的频段值*/
    VOS_UINT8                           aucReserved[2];
}AT_DSP_BAND_CHANNEL_STRU;




typedef struct
{
    VOS_UINT32                          ulUlFreq;                               /*上行频点:单位KHZ*/
    VOS_UINT32                          ulDlFreq;                               /*下行频点:单位KHZ*/
    VOS_UINT16                          usDspBand;                              /*DSP格式的频段值*/
    VOS_UINT8                           aucReserved[2];
}AT_DSP_BAND_FREQ_STRU;


typedef struct
{
    AT_BAND_WIDTH_ENUM_UINT16           enAtBandWidth;
    BANDWIDTH_ENUM_UINT16               enBbicBandWidth;
    AT_BAND_WIDTH_VALUE_ENUM_UINT32     enAtBandWidthValue;
}AT_BAND_WIDTH_INFO_STRU;


typedef struct
{
    VOS_BOOL                            bDspLoadFlag;
    VOS_BOOL                            bSetTxTselrfFlag;
    VOS_BOOL                            bSetRxTselrfFlag;
    VOS_UINT8                           ucIndex;
    AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8   enRatMode;
    AT_SUB_CARRIER_SPACING_ENUM_UINT8   enBbicScs;
    AT_FWAVE_TYPE_ENUM_UINT8            enFaveType;
    AT_DSP_RF_ON_OFF_ENUM_UINT8         enRxOnOff;       /* Rx on off值*/
    AT_DSP_RF_ON_OFF_ENUM_UINT8         enTxOnOff;       /* Tx On off值*/
    AT_DSP_RF_ON_OFF_ENUM_UINT8         enTempRxorTxOnOff; /* 临时记录Rx 或 Tx on off值*/
    VOS_UINT8                           ucAgcGainLevel;   /* 接收机AGCGAIN等级*/
    AT_CMD_PALEVEL_ENUM_UINT8           enPaLevel;        /* 发射机的PA等级*/
    AT_DCXOTEMPCOMP_ENABLE_ENUM_UINT8   enDcxoTempCompEnableFlg;
    VOS_UINT8                           aucRes[2];
    VOS_UINT16                          usFwavePower;
    AT_BAND_WIDTH_ENUM_UINT16           enBandWidth;  /* 存储AT下发的值，用于查询命令 */
    AT_DSP_BAND_CHANNEL_STRU            stBandArfcn;
    AT_TSELRF_PATH_ENUM_UINT32          enTseLrfTxPath;
    AT_TSELRF_PATH_ENUM_UINT32          enTseLrfRxPath;
    AT_MIMO_TYPE_UINT8                  enTxMimoType;
    AT_MIMO_TYPE_UINT8                  enRxMimoType;
    AT_MIMO_ANT_NUM_ENUM_UINT8          enTxMimoAntNum;
    AT_MIMO_ANT_NUM_ENUM_UINT8          enRxMimoAntNum;
}AT_MT_AT_INFO_STRU;


typedef struct
{
    NR_SCS_TYPE_COMM_ENUM_UINT8         enBbicScs;
    AT_ANT_TYPE_ENUM_UINT8              enTxAntType;
    AT_ANT_TYPE_ENUM_UINT8              enRxAntType;
    BBIC_CAL_PA_MODE_ENUM_UINT8         enPaLevel;
    MODU_TYPE_ENUM_UINT16               enFwaveType;
    VOS_INT16                           sFwavePower;
    RAT_MODE_ENUM_UINT16                enCurrtRatMode;
    RAT_MODE_ENUM_UINT16                enDpdtRatMode;
    AT_BAND_WIDTH_VALUE_ENUM_UINT32     enBandWidthValue; /* 真正的带宽频率 */
    AT_DSP_BAND_FREQ_STRU               stDspBandFreq;
    BBIC_TEMP_CHANNEL_TYPE_ENUM_UINT16  enCurrentChannelType;
    BANDWIDTH_ENUM_UINT16               enBandWidth;
    AT_LOAD_DSP_RATMODE_ENUM_UINT8      enLoadDspMode;
    VOS_UINT8                           ucRev;
    BBIC_CAL_DCXO_SET_ENUM_UINT16       enDcxoTempCompEnableFlg;
    AT_MIMO_TYPE_UINT8                  enTxMimoType;
    AT_MIMO_TYPE_UINT8                  enRxMimoType;
    AT_MIMO_ANT_NUM_ENUM_UINT8          enTxMimoAntNum;
    AT_MIMO_ANT_NUM_ENUM_UINT8          enRxMimoAntNum;
}AT_MT_BBIC_INFO_STRU;


typedef struct
{
    TAF_PH_TMODE_ENUM_UINT8             enCurrentTMode;
    VOS_UINT8                           aucRes[3];
    AT_MT_AT_INFO_STRU                  stAtInfo;
    AT_MT_BBIC_INFO_STRU                stBbicInfo;
}AT_MT_INFO_STRU;


/*AT与BBIC模块间消息处理函数指针*/
typedef VOS_UINT32 (*AT_BBIC_MSG_PROC_FUNC)(VOS_VOID *pMsg);

/*AT与CBT模块间消息处理函数指针*/
typedef VOS_UINT32 (*AT_CBT_MSG_PROC_FUNC)(VOS_VOID *pMsg);



typedef struct
{
    VOS_UINT32                          ulMsgType;
    AT_BBIC_MSG_PROC_FUNC               pProcMsgFunc;
}AT_PROC_BBIC_MSG_STRU;


typedef struct
{
    VOS_UINT32                          ulMsgType;
    AT_CBT_MSG_PROC_FUNC                pProcMsgFunc;
}AT_PROC_CBT_MSG_STRU;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
#endif


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AtBbicCalInterface.h */
