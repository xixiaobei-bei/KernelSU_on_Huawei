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

#ifndef __AT_MT_INTERFACE_H__
#define __AT_MT_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "product_config.h"
#if (FEATURE_ON == FEATURE_UE_MODE_NR)
#include "nascbtinterface.h"
#include "bbic_mt_cal_ext.h"
#endif
#include "taf_drv_agent.h"

#pragma pack(push, 4)

#if (FEATURE_ON == FEATURE_UE_MODE_NR)
/* 将BBIC CAL模块定义的消息转换成AT和BBIC模块间的消息 */
#define ID_AT_BBIC_CAL_RF_DEBUG_TX_REQ ID_TOOL_BBIC_CAL_RF_DEBUG_TX_REQ
/* _H2ASN_MsgChoice BBIC_CAL_MSG_CNF_STRU                   */
#define ID_BBIC_AT_CAL_MSG_CNF ID_BBIC_TOOL_CAL_MSG_CNF
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU       */
#define ID_AT_BBIC_CAL_RF_DEBUG_GSM_TX_REQ ID_TOOL_BBIC_CAL_RF_DEBUG_GSM_TX_REQ
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU      */
#define ID_BBIC_AT_CAL_RF_DEBUG_GTX_MRX_IND ID_BBIC_TOOL_CAL_RF_DEBUG_GTX_MRX_IND
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU    */
#define ID_AT_BBIC_MIPI_READ_REQ ID_TOOL_BBIC_MIPI_READ_REQ
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU    */
#define ID_BBIC_AT_MIPI_READ_CNF ID_BBIC_TOOL_MIPI_READ_IND
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU   */
#define ID_AT_BBIC_MIPI_WRITE_REQ ID_TOOL_BBIC_MIPI_WRITE_REQ
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU   */
#define ID_BBIC_AT_MIPI_WRITE_CNF ID_BBIC_TOOL_MIPI_WRITE_IND
/* _H2ASN_MsgChoice BBIC_CAL_PLL_QRY_REQ_STRU               */
#define ID_AT_BBIC_PLL_QRY_REQ ID_TOOL_BBIC_PLL_QRY_REQ
/* _H2ASN_MsgChoice BBIC_CAL_PLL_QRY_IND_STRU               */
#define ID_BBIC_AT_PLL_QRY_CNF ID_BBIC_TOOL_PLL_QRY_IND
/* _H2ASN_MsgChoice BBIC_CAL_TEMP_QRY_REQ_STRU              */
#define ID_AT_BBIC_TEMP_QRY_REQ ID_TOOL_BBIC_TEMP_QRY_REQ
/* _H2ASN_MsgChoice BBIC_CAL_TEMP_QRY_IND_STRU              */
#define ID_BBIC_AT_TEMP_QRY_CNF ID_BBIC_TOOL_TEMP_QRY_IND
/* _H2ASN_MsgChoice BBIC_CAL_DPDT_REQ_STRU                  */
#define ID_AT_BBIC_DPDT_REQ ID_TOOL_BBIC_DPDT_REQ
/* _H2ASN_MsgChoice BBIC_CAL_DPDT_IND_STRU                  */
#define ID_BBIC_AT_DPDT_CNF ID_BBIC_TOOL_DPDT_IND
/* _H2ASN_MsgChoice BBIC_CAL_DCXO_REQ_STRU                  */
#define ID_AT_BBIC_DCXO_REQ ID_TOOL_BBIC_CAL_RF_DEBUG_DCXO_REQ
/* _H2ASN_MsgChoice BBIC_CAL_DCXO_CNF_STRU                  */
#define ID_BBIC_AT_DCXO_CNF ID_BBIC_TOOL_CAL_RF_DEBUG_DCXO_IND
/* _H2ASN_MsgChoice BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU       */
#define ID_BBIC_AT_GSM_TX_PD_CNF ID_BBIC_TOOL_CAL_RF_DEBUG_GTX_MRX_IND
/* _H2ASN_MsgChoice BBIC_CAL_SetTrxTasReqParaStru           */
#define ID_AT_BBIC_TRX_TAS_REQ ID_TOOL_BBIC_TRX_TAS_REQ
/* _H2ASN_MsgChoice BBIC_CAL_SetTrxTasCnfParaStru           */
#define ID_BBIC_AT_TRX_TAS_CNF ID_TOOL_BBIC_TRX_TAS_CNF

#define FREQ_UNIT_MHZ_TO_KHZ (1000)
#define LTE_CHANNEL_TO_FREQ_UNIT (100)
#define W_CHANNEL_FREQ_TIMES (5)

#define G_CHANNEL_NO_124 (124)
#define G_CHANNEL_NO_955 (955)
#define G_CHANNEL_NO_1023 (1023)

#define C_CHANNEL_NO_1 (1)
#define C_CHANNEL_NO_799 (799)
#define C_CHANNEL_NO_991 (991)
#define C_CHANNEL_NO_1023 (1023)
#define C_CHANNEL_NO_1024 (1024)
#define C_CHANNEL_NO_1323 (1323)

#define FWAVE_TYPE_CONTINUE (0xFF)

#define AT_MT_LOAD_DSP_FTM_MODE (5)

#define AT_MT_MIPI_READ_MAX_BYTE (4)

#define MT_OK (0)
#define MT_ERR (1)

#define AT_BBIC_CAL_MAX_GSM_SLOT 8


typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 usMsgId;
    VOS_UINT16 usRev;
} AT_MT_MsgHeader;


enum AT_PROTOCOL_Band {
    AT_PROTOCOL_BAND_1  = 1,
    AT_PROTOCOL_BAND_2  = 2,
    AT_PROTOCOL_BAND_3  = 3,
    AT_PROTOCOL_BAND_4  = 4,
    AT_PROTOCOL_BAND_5  = 5,
    AT_PROTOCOL_BAND_6  = 6,
    AT_PROTOCOL_BAND_7  = 7,
    AT_PROTOCOL_BAND_8  = 8,
    AT_PROTOCOL_BAND_9  = 9,
    AT_PROTOCOL_BAND_10 = 10,
    AT_PROTOCOL_BAND_11 = 11,
    AT_PROTOCOL_BAND_12 = 12,
    AT_PROTOCOL_BAND_13 = 13,
    AT_PROTOCOL_BAND_14 = 14,
    AT_PROTOCOL_BAND_15 = 15,
    AT_PROTOCOL_BAND_16 = 16,
    AT_PROTOCOL_BAND_17 = 17,
    AT_PROTOCOL_BAND_18 = 18,
    AT_PROTOCOL_BAND_19 = 19,
    AT_PROTOCOL_BAND_20 = 20,
    AT_PROTOCOL_BAND_21 = 21,
    AT_PROTOCOL_BAND_22 = 22,
    AT_PROTOCOL_BAND_23 = 23,
    AT_PROTOCOL_BAND_24 = 24,
    AT_PROTOCOL_BAND_25 = 25,
    AT_PROTOCOL_BAND_26 = 26,
    AT_PROTOCOL_BAND_27 = 27,
    AT_PROTOCOL_BAND_28 = 28,
    AT_PROTOCOL_BAND_29 = 29,
    AT_PROTOCOL_BAND_30 = 30,
    AT_PROTOCOL_BAND_31 = 31,
    AT_PROTOCOL_BAND_32 = 32,
    AT_PROTOCOL_BAND_33 = 33,
    AT_PROTOCOL_BAND_34 = 34,
    AT_PROTOCOL_BAND_35 = 35,
    AT_PROTOCOL_BAND_36 = 36,
    AT_PROTOCOL_BAND_37 = 37,
    AT_PROTOCOL_BAND_38 = 38,
    AT_PROTOCOL_BAND_39 = 39,
    AT_PROTOCOL_BAND_40 = 40,
    AT_PROTOCOL_BAND_41 = 41,
    AT_PROTOCOL_BAND_42 = 42,
    AT_PROTOCOL_BAND_43 = 43,
    AT_PROTOCOL_BAND_44 = 44,
    AT_PROTOCOL_BAND_45 = 45,
    AT_PROTOCOL_BAND_46 = 46,
    AT_PROTOCOL_BAND_47 = 47,
    AT_PROTOCOL_BAND_48 = 48,
    AT_PROTOCOL_BAND_49 = 49,
    AT_PROTOCOL_BAND_50 = 50,
    AT_PROTOCOL_BAND_51 = 51,
    AT_PROTOCOL_BAND_52 = 52,
    AT_PROTOCOL_BAND_53 = 53,
    AT_PROTOCOL_BAND_54 = 54,
    AT_PROTOCOL_BAND_55 = 55,
    AT_PROTOCOL_BAND_56 = 56,
    AT_PROTOCOL_BAND_57 = 57,
    AT_PROTOCOL_BAND_58 = 58,
    AT_PROTOCOL_BAND_59 = 59,
    AT_PROTOCOL_BAND_60 = 60,
    AT_PROTOCOL_BAND_61 = 61,
    AT_PROTOCOL_BAND_62 = 62,
    AT_PROTOCOL_BAND_63 = 63,
    AT_PROTOCOL_BAND_64 = 64,
    AT_PROTOCOL_BAND_65 = 65,
    AT_PROTOCOL_BAND_66 = 66,

    AT_PROTOCOL_BAND_70 = 70,
    AT_PROTOCOL_BAND_71 = 71,

    AT_PROTOCOL_BAND_74 = 74,
    AT_PROTOCOL_BAND_75 = 75,
    AT_PROTOCOL_BAND_76 = 76,
    AT_PROTOCOL_BAND_77 = 77,
    AT_PROTOCOL_BAND_78 = 78,
    AT_PROTOCOL_BAND_79 = 79,
    AT_PROTOCOL_BAND_80 = 80,
    AT_PROTOCOL_BAND_81 = 81,
    AT_PROTOCOL_BAND_82 = 82,
    AT_PROTOCOL_BAND_83 = 83,
    AT_PROTOCOL_BAND_84 = 84,

    AT_PROTOCOL_BAND_256 = 256,
    AT_PROTOCOL_BAND_257 = 257,
    AT_PROTOCOL_BAND_258 = 258,
    AT_PROTOCOL_BAND_259 = 259,
    AT_PROTOCOL_BAND_260 = 260,
    AT_PROTOCOL_BAND_261 = 261,

    AT_PROTOCOL_BAND_BUTT
};
typedef VOS_UINT16 AT_PROTOCOL_BandUint16;


enum AT_BAND_Width {
    AT_BAND_WIDTH_200K   = 0,
    AT_BAND_WIDTH_1M2288 = 1,
    AT_BAND_WIDTH_1M28   = 2,
    AT_BAND_WIDTH_1M4    = 3,
    AT_BAND_WIDTH_3M     = 4,
    AT_BAND_WIDTH_5M     = 5,
    AT_BAND_WIDTH_10M    = 6,
    AT_BAND_WIDTH_15M    = 7,
    AT_BAND_WIDTH_20M    = 8,
    AT_BAND_WIDTH_25M    = 9,
    AT_BAND_WIDTH_30M    = 10,
    AT_BAND_WIDTH_40M    = 11,
    AT_BAND_WIDTH_50M    = 12,
    AT_BAND_WIDTH_60M    = 13,
    AT_BAND_WIDTH_80M    = 14,
    AT_BAND_WIDTH_90M    = 15,
    AT_BAND_WIDTH_100M   = 16,
    AT_BAND_WIDTH_200M   = 17,
    AT_BAND_WIDTH_400M   = 18,
    AT_BAND_WIDTH_800M   = 19,
    AT_BAND_WIDTH_1G     = 20,

    AT_BAND_WIDTH_BUTT
};
typedef VOS_UINT16 AT_BAND_WidthUint16;


enum AT_SubCarrierSpacing {
    AT_SUB_CARRIER_SPACING_15K  = 0,
    AT_SUB_CARRIER_SPACING_30K  = 1,
    AT_SUB_CARRIER_SPACING_60K  = 2,
    AT_SUB_CARRIER_SPACING_120K = 3,
    AT_SUB_CARRIER_SPACING_240K = 4,

    AT_SUB_CARRIER_SPACING_BUTT
};
typedef VOS_UINT8 AT_SubCarrierSpacingUint8;


enum AT_DUPLEX_Mode {
    AT_DUPLEX_MODE_FDD = 0,
    AT_DUPLEX_MODE_TDD = 1,
    AT_DUPLEX_MODE_SDL = 2,
    AT_DUPLEX_MODE_SUL = 3,

    AT_DUPLEX_MODE_BUTT
};
typedef VOS_UINT16 AT_DUPLEX_ModeUint16;


enum AT_FWAVE_Type {
    AT_FWAVE_TYPE_BPSK     = 0,
    AT_FWAVE_TYPE_PI2_BPSK = 1,
    AT_FWAVE_TYPE_QPSK     = 2,
    AT_FWAVE_TYPE_16QAM    = 3,
    AT_FWAVE_TYPE_64QAM    = 4,
    AT_FWAVE_TYPE_256QAM   = 5,
    AT_FWAVE_TYPE_GMSK     = 6,
    AT_FWAVE_TYPE_8PSK     = 7,
    AT_FWAVE_TYPE_CONTINUE = 8,

    AT_FWAVE_TYPE_BUTT
};
typedef VOS_UINT8 AT_FWAVE_TypeUint8;


enum AT_BAND_WidthValue {
    AT_BAND_WIDTH_VALUE_200K   = 200,
    AT_BAND_WIDTH_VALUE_1M2288 = 1228,
    AT_BAND_WIDTH_VALUE_1M28   = 1280,
    AT_BAND_WIDTH_VALUE_1M4    = 1400,
    AT_BAND_WIDTH_VALUE_3M     = 3000,
    AT_BAND_WIDTH_VALUE_5M     = 5000,
    AT_BAND_WIDTH_VALUE_10M    = 10000,
    AT_BAND_WIDTH_VALUE_15M    = 15000,
    AT_BAND_WIDTH_VALUE_20M    = 20000,
    AT_BAND_WIDTH_VALUE_25M    = 25000,
    AT_BAND_WIDTH_VALUE_30M    = 30000,
    AT_BAND_WIDTH_VALUE_40M    = 40000,
    AT_BAND_WIDTH_VALUE_50M    = 50000,
    AT_BAND_WIDTH_VALUE_60M    = 60000,
    AT_BAND_WIDTH_VALUE_80M    = 80000,
    AT_BAND_WIDTH_VALUE_90M    = 90000,
    AT_BAND_WIDTH_VALUE_100M   = 100000,
    AT_BAND_WIDTH_VALUE_200M   = 200000,
    AT_BAND_WIDTH_VALUE_400M   = 400000,
    AT_BAND_WIDTH_VALUE_800M   = 800000,
    AT_BAND_WIDTH_VALUE_1G     = 1000000,

    AT_BAND_WIDTH_VALUE_BUTT
};
typedef VOS_UINT32 AT_BAND_WidthValueUint32;


enum AT_CmdRatmode {
    AT_CMD_RATMODE_GSM   = 0,
    AT_CMD_RATMODE_WCDMA = 1,
    AT_CMD_RATMODE_LTE   = 2,
    AT_CMD_RATMODE_TD    = 3,
    AT_CMD_RATMODE_CDMA  = 4,
    AT_CMD_RATMODE_NR    = 5,
    AT_CMD_RATMODE_BUTT
};
typedef VOS_UINT8 AT_CmdRatmodeUint8;


enum AT_LOAD_DspRatmode {
    AT_LOAD_DSP_RATMODE_GSM   = 0,
    AT_LOAD_DSP_RATMODE_WCDMA = 1,
    AT_LOAD_DSP_RATMODE_CDMA  = 2,
    AT_LOAD_DSP_RATMODE_LTE   = 3,
    AT_LOAD_DSP_RATMODE_NR    = 4,
    AT_LOAD_DSP_RATMODE_BUTT
};
typedef VOS_UINT8 AT_LOAD_DspRatmodeUint8;


enum AT_TEMP_ChannelType {
    AT_TEMP_CHANNEL_TYPE_LOGIC = 0,
    AT_TEMP_CHANNEL_TYPE_PHY   = 1,

    AT_TEMP_CHANNEL_TYPE_BUTT
};
typedef VOS_UINT16 AT_TEMP_ChannelTypeUint16;


enum AT_CmdPalevel {
    AT_CMD_PALEVEL_HIGH     = 0,
    AT_CMD_PALEVEL_MID      = 1,
    AT_CMD_PALEVEL_LOW      = 2,
    AT_CMD_PALEVEL_ULTRALOW = 3,
    AT_CMD_PALEVEL_BUTT
};
typedef VOS_UINT8 AT_CmdPalevelUint8;


enum AT_AntType {
    AT_ANT_TYPE_PRI  = 1, /* 主极 */
    AT_ANT_TYPE_DIV  = 2, /* 分极 */
    AT_ANT_TYPE_MIMO = 4, /* MINO */

    AT_ANT_TYPE_BUTT
};
typedef VOS_UINT8 AT_AntTypeUint8;


enum AT_GsmTxSlotType {
    AT_GSM_TX_8_SLOT = 0,
    AT_GSM_TX_1_SLOT = 1,
    AT_GSM_TX_2_SLOT = 2,
    AT_GSM_TX_3_SLOT = 3,
    AT_GSM_TX_4_SLOT = 4,

    AT_GSM_TX_SLOT_BUTT
};
typedef VOS_UINT8 AT_GsmTxSlotTypeUint8;


enum AT_MT_AntType {
    AT_MT_ANT_TYPE_TX = 0,
    AT_MT_ANT_TYPE_RX = 1,

    AT_MT_ANT_TYPE_BUTT
};
typedef VOS_UINT8 AT_MT_AntTypeUint8;


enum AT_MIMO_Type {
    AT_MIMO_TYPE_2 = 1, /* 双天线 */
    AT_MIMO_TYPE_4 = 2, /* 四天线 */
    AT_MIMO_TYPE_8 = 3, /* 八天线 */

    AT_MIMO_TYPE_BUTT
};
typedef VOS_UINT8 AT_MIMO_TYPE_UINT8;


enum AT_MIMO_AntNum {
    AT_MIMO_ANT_NUM_1 = 1, /* 第1根天线 */
    AT_MIMO_ANT_NUM_2 = 2, /* 第2根天线 */
    AT_MIMO_ANT_NUM_3 = 3, /* 第3根天线 */
    AT_MIMO_ANT_NUM_4 = 4, /* 第4根天线 */
    AT_MIMO_ANT_NUM_5 = 5, /* 第5根天线 */
    AT_MIMO_ANT_NUM_6 = 6, /* 第6根天线 */
    AT_MIMO_ANT_NUM_7 = 7, /* 第7根天线 */
    AT_MIMO_ANT_NUM_8 = 8, /* 第8根天线 */

    AT_MIMO_ANT_NUM_BUTT
};
typedef VOS_UINT8 AT_MIMO_AntNumUint8;


enum AT_RFIC_MemTest {
    AT_RFIC_MEM_TEST_PASS      = 0,          /* 底层返回成功 */
    AT_RFIC_MEM_TEST_RUNNING   = 0x7FFFFFFE, /* 底层还没返回结果 */
    AT_RFIC_MEM_TEST_NOT_START = 0x7FFFFFFF  /* 没有启动测试 */
};
typedef VOS_UINT32 AT_RFIC_MemTestUint32;


enum AT_SERDES_Test {
    AT_SERDES_TEST_RUNNING   = -2, /* 未测完 */
    AT_SERDES_TEST_NOT_START = -1, /* 没有启动测试 */
    AT_SERDES_TEST_PASS      = 0,  /* 返回成功 */
};
typedef VOS_INT32 AT_SERDES_TestInt32;

enum AT_SERDES_TEST_RST_ENUM {
    AT_SERDES_TEST_RST_SUCCESS   = 0,  /* 成功，1-0xff都表示失败 */
    AT_SERDES_TEST_RST_RUNNING   = -1, /* 正在测试 */
    AT_SERDES_TEST_RST_NOT_START = -2, /* 没有启动测试 */
    AT_SERDES_TEST_RST_NOT_MATCH = -3, /* 类型不匹配 */
};
typedef VOS_INT16 AT_SERDES_TEST_RST_ENUM_INT16;

#endif


enum AT_TSELRF_Path {
    AT_TSELRF_PATH_GSM          = 1,
    AT_TSELRF_PATH_WCDMA_PRI    = 2,
    AT_TSELRF_PATH_WCDMA_DIV    = 3,
    AT_TSELRF_PATH_CDMA_PRI     = 4,
    AT_TSELRF_PATH_CDMA_DIV     = 5,
    AT_TSELRF_PATH_TD           = 6,
    AT_TSELRF_PATH_WIFI         = 7,
    AT_TSELRF_PATH_WIMAX        = 8,
    AT_TSELRF_PATH_FDD_LTE_PRI  = 9,
    AT_TSELRF_PATH_FDD_LTE_DIV  = 10,
    AT_TSELRF_PATH_FDD_LTE_MIMO = 11,
    AT_TSELRF_PATH_TDD_LTE_PRI  = 12,
    AT_TSELRF_PATH_TDD_LTE_DIV  = 13,
    AT_TSELRF_PATH_TDD_LTE_MIMO = 14,
    AT_TSELRF_PATH_NAVIGATION   = 15,
    AT_TSELRF_PATH_NR_PRI       = 16,
    AT_TSELRF_PATH_NR_DIV       = 17,
    AT_TSELRF_PATH_NR_MIMO      = 18,
    AT_TSELRF_PATH_BUTT
};
typedef VOS_UINT32 AT_TSELRF_PathUint32;


enum AT_I2sTestState {
    AT_I2S_TEST_PASS      = 0,   /* 成功 */
    AT_I2S_TEST_FAILED    = 1,   /* 失败 */
    AT_I2S_TEST_TIMEOUT   = 2,   /* 定时器超时 */
    AT_I2S_TEST_RUNNING   = 254, /* 测试结果没返回 */
    AT_I2S_TEST_NOT_START = 255, /* 没有启动测试 */
};
typedef VOS_UINT8 AT_I2sTestStateUint8;


enum AT_UartTestState {
    AT_UART_TEST_PASS         = 0,   /* 成功 */
    AT_UART_TEST_DATA_ERROR   = 1,   /* 数据校验失败 */
    AT_UART_TEST_TIMEOUT      = 2,   /* 定时器超时 */
    AT_UART_TEST_ICC_NOT_OPEN = 3,   /* ICC没开启 */
    AT_UART_TEST_RUNNING      = 254, /* 测试结果没返回 */
    AT_UART_TEST_NOT_START    = 255, /* 没有启动测试 */
};
typedef VOS_UINT8 AT_UartTestStateUint8;

#if (FEATURE_ON == FEATURE_UE_MODE_NR)

typedef struct {
    AT_TSELRF_PathUint32 tselPath;
    AT_AntTypeUint8      antType;
    VOS_UINT8            rev[3];
} AT_PATH_ToAntType;


typedef struct {
    VOS_UINT32 channelMin; /* 信道最小值 */
    VOS_UINT32 channelMax; /* 信道最大值 */
} AT_CHANNEL_Range;


typedef struct {
    VOS_UINT32 freqMin; /* 频率最小值,单位KHZ */
    VOS_UINT32 freqMax; /* 频率最大值,单位KHZ */
} AT_FREQ_Range;


typedef struct {
    AT_FREQ_Range    freqRange;
    VOS_UINT32       freqGlobal; /* 信道最大值,单位KHZ */
    VOS_UINT32       freqOffset;
    VOS_UINT32       offsetChannel;
    AT_CHANNEL_Range channelRange;
} AT_NR_FreqOffsetTable;


typedef struct {
    VOS_UINT16           bandNo;
    AT_DUPLEX_ModeUint16 mode;
    AT_CHANNEL_Range     ulChannelRange;
    AT_CHANNEL_Range     dlChannelRange;
    AT_FREQ_Range        ulFreqRange;
    AT_FREQ_Range        dlFreqRange;
} AT_NR_BandInfo;


typedef struct {
    VOS_UINT16       bandNo;
    VOS_UINT16       rev;
    VOS_UINT32       dlLowFreq;
    VOS_UINT32       dlChannelOffset;
    AT_CHANNEL_Range dlChannelRange;
    VOS_UINT32       ulLowFreq;
    VOS_UINT32       ulChannelOffset;
    AT_CHANNEL_Range ulChannelRange;
} AT_LTE_BandInfo;


typedef struct {
    VOS_UINT16       bandNo;
    VOS_UINT16       rev;
    VOS_UINT32       ulFreqOffset;
    AT_CHANNEL_Range ulChannelRange;
    AT_FREQ_Range    ulFreqRange;
    VOS_UINT32       dlFreqOffset;
    AT_CHANNEL_Range dlChannelRange;
    AT_FREQ_Range    dlFreqRange;
} AT_W_BandInfo;


typedef struct {
    VOS_UINT16       bandNo;
    VOS_UINT16       ulPath; /* 上行path值 */
    VOS_UINT32       freqOffset;
    AT_CHANNEL_Range channelRange;
} AT_G_BandInfo;


typedef struct {
    VOS_UINT32 ulChanNo; /* 上行的Channel No */
    VOS_UINT32 dlChanNo; /* 下行的Channel No */
    VOS_UINT16 dspBand;  /* DSP格式的频段值 */
    VOS_UINT8  reserved[2];
} AT_DSP_BandChannel;



typedef struct {
    VOS_UINT32 ulFreq;  /* 上行频点:单位KHZ */
    VOS_UINT32 dlFreq;  /* 下行频点:单位KHZ */
    VOS_UINT16 dspBand; /* DSP格式的频段值 */
    VOS_UINT8  reserved[2];
} AT_DSP_BandFreq;


typedef struct {
    AT_BAND_WidthUint16      atBandWidth;
    BANDWIDTH_ENUM_UINT16    bbicBandWidth;
    AT_BAND_WidthValueUint32 atBandWidthValue;
} AT_BAND_WidthInfo;


typedef struct {
    VOS_BOOL                    dspLoadFlag;
    VOS_BOOL                    setTxTselrfFlag;
    VOS_BOOL                    setRxTselrfFlag;
    VOS_UINT8                   indexNum;
    AT_DEVICE_CmdRatModeUint8   ratMode;
    AT_SubCarrierSpacingUint8   bbicScs;
    AT_FWAVE_TypeUint8          faveType;
    AT_DSP_RfOnOffUint8         rxOnOff;         /* Rx on off值 */
    AT_DSP_RfOnOffUint8         txOnOff;         /* Tx On off值 */
    AT_DSP_RfOnOffUint8         tempRxorTxOnOff; /* 临时记录Rx 或 Tx on off值 */
    VOS_UINT8                   agcGainLevel;    /* 接收机AGCGAIN等级 */
    AT_CmdPalevelUint8          paLevel;         /* 发射机的PA等级 */
    AT_DCXOTEMPCOMP_EnableUint8 dcxoTempCompEnableFlg;
    AT_GsmTxSlotTypeUint8       gsmTxSlotType;
    AT_MT_AntTypeUint8          antType;
    VOS_UINT16                  fwavePower;
    AT_BAND_WidthUint16         bandWidth; /* 存储AT下发的值，用于查询命令 */
    AT_DSP_BandChannel          bandArfcn;
    AT_TSELRF_PathUint32        tseLrfTxPath;
    AT_TSELRF_PathUint32        tseLrfRxPath;
    AT_MIMO_TYPE_UINT8  txMimoType;
    AT_MIMO_TYPE_UINT8  rxMimoType;
    AT_MIMO_AntNumUint8 txMimoAntNum;
    AT_MIMO_AntNumUint8 rxMimoAntNum;
    VOS_UINT32 rficTestResult;
    VOS_UINT8  rbEnable;
    VOS_UINT8  reserved[3];
    VOS_UINT16 rbNum;
    VOS_UINT16 rbStartCfg;
} AT_MT_AtInfo;


typedef struct {
    NR_SCS_TYPE_COMM_ENUM_UINT8        bbicScs;
    AT_AntTypeUint8                    txAntType;
    AT_AntTypeUint8                    rxAntType;
    BBIC_CAL_PA_MODE_ENUM_UINT8        paLevel;
    MODU_TYPE_ENUM_UINT16              fwaveType;
    VOS_INT16                          fwavePower;
    RAT_MODE_ENUM_UINT16               currtRatMode;
    RAT_MODE_ENUM_UINT16               dpdtRatMode;
    AT_BAND_WidthValueUint32           bandWidthValue; /* 真正的带宽频率 */
    AT_DSP_BandFreq                    dspBandFreq;
    BBIC_TEMP_CHANNEL_TYPE_ENUM_UINT16 currentChannelType;
    BANDWIDTH_ENUM_UINT16              bandWidth;
    BBIC_CAL_DCXO_SET_ENUM_UINT16      dcxoTempCompEnableFlg;
    VOS_UINT16 txMimoType;
    VOS_UINT16 rxMimoType;
    VOS_UINT16 txMimoAntNum;
    VOS_UINT16 rxMimoAntNum;
    RAT_MODE_ENUM_UINT16 trxTasRatMode;
} AT_MT_BbicInfo;

typedef struct {
    AT_UartTestStateUint8 uartTestState;
    AT_I2sTestStateUint8  i2sTestState;
    VOS_UINT8             reserve[6];
} AT_MtCifTestInfo;


typedef struct {
    VOS_UINT32 rptFlg;
    VOS_INT16  result;
    VOS_UINT8  cmdType;
    VOS_UINT8  aucRes[1];
} AT_MT_SerdesTestAsyncInfo;


typedef struct {
    TAF_PH_TmodeUint8         currentTMode;
    VOS_UINT8                 res[3];
    VOS_INT32                 rserTestResult;
    AT_MT_SerdesTestAsyncInfo serdesTestAsyncInfo;
    AT_MtCifTestInfo          cifTestInfo; /* communication interface 通信接口测试信息 */
    AT_MT_AtInfo              atInfo;
    AT_MT_BbicInfo            bbicInfo;
} AT_MT_Info;

/* AT与BBIC模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_BBIC_MSG_PROC_FUNC)(struct MsgCB *pMsg);

/* AT与CBT模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_CBT_MSG_PROC_FUNC)(struct MsgCB *pMsg);

/* AT与PAM_AT模块间消息处理函数指针 */
typedef VOS_UINT32 (*AT_UECBT_MSG_PROC_FUNC)(struct MsgCB *pMsg);

typedef VOS_UINT32 (*AT_PHY_MSG_PROC_FUNC)(struct MsgCB *pMsg);


typedef struct {
    VOS_UINT32            msgType;
    AT_BBIC_MSG_PROC_FUNC procMsgFunc;
} AT_PROC_BbicMsg;


typedef struct {
    VOS_UINT32           msgType;
    AT_CBT_MSG_PROC_FUNC procMsgFunc;
} AT_PROC_CbtMsg;


typedef struct {
    VOS_UINT32             msgType;
    AT_UECBT_MSG_PROC_FUNC procMsgFunc;
} AT_PROC_UecbtMsg;


typedef struct {
    VOS_UINT32           msgType;
    AT_PHY_MSG_PROC_FUNC procMsgFunc;
} AT_PROC_DspIdleMsg;

#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of AtBbicCalInterface.h */
