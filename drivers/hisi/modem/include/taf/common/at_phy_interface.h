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

#ifndef __ATPHYINTERFACE_H__
#define __ATPHYINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "taf_dsp_phy.h"

#pragma pack(push, 4)

#define W_AFC_INIT_VALUE (0x959) /* W_AFC状态初值 */

#define WDSP_CTRL_TX_OFF 2 /* 表示DSP强制上行RF关闭 */
#define WDSP_CTRL_TX_ON 3  /* 表示DSP强制上行RF打开 */

#define WDSP_CTRL_TX_ONE_TONE 8 /* 表示打开同相单音信号 */
#define WDSP_CTRL_TX_TWO_TONE 9 /* 表示打开反相单音信号 */

#define WDSP_CTRL_TX_THREE_TONE 3 /* 表示打开调制 */

#define GDSP_CTRL_TX_OFF 0xAAAA /* 表示DSP强制上行RF关闭 */
#define GDSP_CTRL_TX_ON 0x5555  /* 表示DSP强制上行RF打开 */

#define WDSP_MAX_TX_AGC 2047
#define GDSP_MAX_TX_VPA 1023

#define BBP_PA_HIGH_MODE 1 /* PA高增益模式 */
#define BBP_PA_MID_MODE 2  /* PA中增益模式 */
#define BBP_PA_LOW_MODE 3  /* PA低增益模式 */
#define BBP_PA_AUTO_MODE 0 /* PA自动增益模式 */

#define DSP_CTRL_RX_OFF 2        /* CPU控制关闭射频芯片。 */
#define DSP_CTRL_RX_ANT1_ON 3    /* CPU控制打开射频芯片,天线1。 */
#define DSP_CTRL_RX_ALL_ANT_ON 7 /* CPU控制打开射频芯片,天线1和2 */

#define DSP_LNA_HIGH_GAIN_MODE (0x2)    /* DSP固定LNA为高增益模式 */
#define DSP_LNA_LOW_GAIN_MODE (0x1)     /* DSP固定LNA为低增益模式 */
#define DSP_LNA_NO_CTRL_GAIN_MODE (0x0) /* WBBP控制LNA模式,自动模式 */

#define AT_GDSP_AGC_GAIN1_75DB (0) /* GDSP AGC增益,0档 */
#define AT_GDSP_AGC_GAIN2_63DB (1) /* GDSP AGC增益,1档 */
#define AT_GDSP_AGC_GAIN3_43DB (2) /* GDSP AGC增益,2档 */
#define AT_GDSP_AGC_GAIN4_23DB (3) /* GDSP AGC增益,3档 */

#define AT_GDSP_RX_MODE_BURST (0)           /* 接收 */
#define AT_GDSP_RX_MODE_CONTINOUS_BURST (1) /* 连续接收 */
#define AT_GDSP_RX_MODE_STOP (2)            /* 停止 */

#define AT_GDSP_RX_SLOW_AGC_MODE (0) /* Slow AGC */
#define AT_GDSP_RX_FAST_AGC_MODE (1) /* fast AGC */

#define AT_DSP_RF_AGC_STATE_ERROR (-1) /* 取得测量值时的增益状态，-1表示出错 */

#define AT_DSP_RSSI_MEASURE_NUM (1)          /* RSSI的测量次数 */
#define AT_DSP_RSSI_MEASURE_INTERVAL (0)     /* RSSI的测量间隔时间，单位ms */
#define AT_DSP_RSSI_VALUE_UINT (0.125)       /* RSSI测量值，单位0.125dBm */
#define AT_DSP_RSSI_VALUE_MUL_THOUSAND (125) /* RSSI测量值,单位0.125dBm*1000 */

/* tyg */
#define AT_DPS_NOISERSSI_MAX_NUM (32)

#define AT_RX_PRI_ON (0) /* 测量主集 */
#define AT_RX_DIV_ON (1) /* 测量分集 */
#define AT_RXON_OPEN (0) /* 最近一次执行的是打开接收机RXON=1操作 */
#define AT_TXON_OPEN (1) /* 最近一次执行的是打开发射机TXON=1操作 */

#define AT_BIT0 0x00000001L
#define AT_BIT1 0x00000002L
#define AT_BIT2 0x00000004L
#define AT_BIT3 0x00000008L
#define AT_BIT4 0x00000010L
#define AT_BIT5 0x00000020L
#define AT_BIT6 0x00000040L
#define AT_BIT7 0x00000080L
#define AT_BIT8 0x00000100L
#define AT_BIT9 0x00000200L
#define AT_BIT10 0x00000400L
#define AT_BIT11 0x00000800L
#define AT_BIT12 0x00001000L
#define AT_BIT13 0x00002000L
#define AT_BIT14 0x00004000L
#define AT_BIT15 0x00008000L

/* W_RF_CFG_REQ命令的掩码 */
#define W_RF_MASK_AFC AT_BIT0
#define W_RF_MASK_TX_ARFCN AT_BIT1
#define W_RF_MASK_TX_TXONOFF AT_BIT2
#define W_RF_MASK_TX_PAMODE AT_BIT3
#define W_RF_MASK_TX_AGC AT_BIT4
#define W_RF_MASK_TX_POWER AT_BIT5
#define W_RF_MASK_RX_RXONOFF AT_BIT6
#define W_RF_MASK_RX_ARFCN AT_BIT7
#define W_RF_MASK_RX_ANTSEL AT_BIT8
#define W_RF_MASK_RX_LNAMODE AT_BIT9
#define W_RF_MASK_RX_PGC AT_BIT10
#define W_RF_MASK_TX_POWDET AT_BIT11
#define W_RF_MASK_TX_CLT AT_BIT12
#define W_RF_MASK_TX_DCXOTEMPCOMP AT_BIT14
/* G_RF_RX_CFG_REQ_STRU结构中uhwMask的比特位定义 */
#define G_RF_MASK_RX_ARFCN AT_BIT0   /* 下发频点 */
#define G_RF_MASK_RX_MODE AT_BIT1    /* 下发接收模式 */
#define G_RF_MASK_RX_AGCMODE AT_BIT2 /* 下发测量模式 */
#define G_RF_MASK_RX_AGCGAIN AT_BIT3 /* 下发AGC档位 */

/* G_RF_TX_CFG_REQ_STRU结构中uhwMask的比特位定义 */
/*
 * 注意: 1)在GMSK调制方式下，目标发射功率可通过G_RF_MASK_TX_GSMK_PA_VOLT
 *   或G_RF_MASK_TX_POWER下发,但不能同时下发;
 * 2)在8PSK调制方式下:目标功率值只能通过G_RF_MASK_TX_POWER下发
 * 3)各比特位可以同时下发，但不能相互冲突
 */
#define G_RF_MASK_TX_AFC AT_BIT0           /* 下发单板频率控制值 */
#define G_RF_MASK_TX_ARFCN AT_BIT1         /* 下发发射频点 */
#define G_RF_MASK_TX_TXEN AT_BIT2          /* 下发发射控制位 */
#define G_RF_MASK_TX_TXMODE AT_BIT3        /* 下发发送模式 */
#define G_RF_MASK_TX_MOD_TYPE AT_BIT4      /* 下发调制模式 */
#define G_RF_MASK_TX_GSMK_PA_VOLT AT_BIT5  /* 下发GMSK调制时 PA目标控制电压 */
#define G_RF_MASK_TX_POWER AT_BIT6         /* 下发目标发射控制值,GMSK和8PSK共用 */
#define G_RF_MASK_TX_8PSK_PA_VBIAS AT_BIT7 /* 下发8PSK调制时PA偏置电压 */
#define G_RF_MASK_TX_TXOLC AT_BIT8         /* 下发TXOLC寄存器值 */
#define G_RF_MASK_TX_DATA_PATTERN AT_BIT9

/* 调制方式 */
#define G_MOD_TYPE_GMSK 0 /* GMSK调制方式 */
#define G_MOD_TYPE_8PSK 1 /* 8PSK调制方式 */

/* noise tool 最大上报的结果个数 */
#define PHY_NOISE_MAX_RESULT_NUMB 32

/* 如果工具下发的上行功率为0xff则为不打开上行 */
#define PHY_NOIST_TOOL_CLOSE_TX 0xff

/* 打开天线的模式 */
enum AT_AntMode {
    ANT_ONE = 1, /* 只打开主集天线 */
    ANT_TWO = 2, /* 同时打开主集和分集天线 */
    ANT_BUTT
};
typedef VOS_UINT8 AT_AntModeUint8;

/* WDSP 格式频段定义 */
enum AT_WBand {
    W_FREQ_BAND1 = 1,
    W_FREQ_BAND2,
    W_FREQ_BAND3,
    W_FREQ_BAND4,
    W_FREQ_BAND5,
    W_FREQ_BAND6,
    W_FREQ_BAND7,
    W_FREQ_BAND8,
    W_FREQ_BAND9,
    W_FREQ_BAND10, /*  BAND10 不支持 */
    W_FREQ_BAND11,
    W_FREQ_BAND_BUTT
};
typedef VOS_UINT16 AT_WDspBandUint16;

/* GDSP 频段定义 */
enum AT_GDspBand {
    G_FREQ_BAND_GSM850 = 0,
    G_FREQ_BAND_GSM900,
    G_FREQ_BAND_DCS1800,
    G_FREQ_BAND_PCS1900,
    G_FREQ_BAND_BUTT
};
typedef VOS_UINT16 AT_GDspBandUint16;

/* AT HPA设置结果 0:success, 1:fail */
enum AT_HPA_CfgRlst {
    AT_HPA_RSLT_SUCC = 0,
    AT_HPA_RSLT_FAIL,
    AT_HPA_RSLT_BUTT
};
typedef VOS_UINT16 AT_HPA_CfgRlstUint16;

/* SLT TEST 枚举值设定 */
enum {
    SLT_SERDES_TSET = 8,
    SLT_ALINK_TEST  = 9,
    SLT_TEST_TYPE_BUTT
};
typedef VOS_UINT8 AT_PHY_SltTestType;

#define AT_SERDES_PARA_MAX_CNT 20

/* W慢速校准原语ID */
#define ID_AT_HPA_RF_CFG_REQ 0x2621
#define ID_HPA_AT_RF_CFG_CNF 0x62E0

#define ID_AT_HPA_RF_RX_RSSI_REQ 0x2622
#define ID_HPA_AT_RF_RX_RSSI_IND 0x62E1

/* W PLL锁定状态查询原语ID */
#define ID_AT_WPHY_RF_PLL_STATUS_REQ 0x2623
#define ID_AT_WPHY_RF_PLL_STATUS_CNF 0x62E2

/* G慢速校准原语ID */
#define ID_AT_GHPA_RF_RX_CFG_REQ 0x2415 /* RX慢速校准参数配置 */
#define ID_AT_GHPA_RF_TX_CFG_REQ 0x2418 /* TX慢速校准参数配置 */
#define ID_GHPA_AT_RF_MSG_CNF 0x4212    /* 通用回复消息原语 */

#define ID_AT_GHPA_RF_RX_RSSI_REQ 0x2416 /* RX慢速校准测量请求 */
#define ID_GHPA_AT_RF_RX_RSSI_IND 0x4210 /* RX慢速校准测量结果上报 */

/* G PLL锁定状态查询原语ID */
#define ID_AT_GPHY_RF_PLL_STATUS_REQ 0x2419
#define ID_AT_GPHY_RF_PLL_STATUS_CNF 0x4219

/* PD检测状态查询原语ID */
#define ID_AT_PHY_POWER_DET_REQ 0x2624
#define ID_AT_PHY_POWER_DET_CNF 0x62e3

/* noise tool 和wphy的接口 */
#define ID_AT_HPA_RF_NOISE_CFG_REQ 0x2625
#define ID_HPA_AT_RF_NOISE_RSSI_IND 0x62e4
/* 回复给NOISE TOOL的消息原语 原语ID */
#define ID_AT_GHPA_RF_NOISE_CFG_REQ 0x241A
#define ID_GHPA_AT_RF_NOISE_RSSI_IND 0x421A

#define ID_AT_HPA_MIPI_WR_REQ 0x2490
#define ID_HPA_AT_MIPI_WR_CNF 0x4290
#define ID_AT_HPA_MIPI_RD_REQ 0x2491
#define ID_HPA_AT_MIPI_RD_CNF 0x4291

#define ID_AT_HPA_SSI_WR_REQ 0x2492
#define ID_HPA_AT_SSI_WR_CNF 0x4292


#define ID_AT_HPA_PDM_CTRL_REQ 0x2494
#define ID_HPA_AT_PDM_CTRL_CNF 0x4294

#define ID_WPHY_AT_TX_CLT_IND 0x62e5

/* CDMA慢速校准原语ID */
#define ID_AT_CHPA_RF_CFG_REQ 0x6300
#define ID_CHPA_AT_RF_CFG_CNF 0x6301

#define ID_AT_CHPA_RF_RX_RSSI_REQ 0x6302
#define ID_CHPA_AT_RF_RX_RSSI_IND 0x6303

#ifndef MODEM_FUSION_VERSION
/* serdes老化测试请求和回复 */
#define ID_FESS_NAS_FLPM_SERDES_AGING_REQ 0x5202
#define ID_FESS_FLPM_NAS_SERDES_AGING_CNF 0x5282

/* serdes无业务测试请求和回复 */
#define ID_FESS_NAS_FLPM_AT_NOBIZ_REQ 0x5200
#define ID_FESS_FLPM_NAS_AT_NOBIZ_CNF 0x5280

/* serdes slt测试请求和回复 */
#define ID_FESS_NAS_FLPM_SERDES_SLT_REQ 0x5201
#define ID_FESS_FLPM_NAS_SERDES_SLT_CNF 0x5281
#endif

typedef struct {
    /* Tx Cfg */
    VOS_UINT16 mask;      /* 按位标识配置类型 */
    VOS_UINT16 txAfcInit; /* AFC */
    VOS_UINT16 txBand;    /* 1,2,3...,协议中的band编号,注意不是BandId */
    VOS_UINT16 txFreqNum; /* Arfcn */
    VOS_UINT16 txEnable;  /* TX通道使能控制 */
    VOS_UINT16 txPaMode;  /* PA模式控制 */
    VOS_INT16  txPower;   /* 0.1dBm */

    /* Rx Cfg */
    VOS_UINT16 rxEnable;  /* RX通道使能控制 */
    VOS_UINT16 rxBand;    /* 1,2,3...,协议中的band编号 */
    VOS_UINT16 rxFreqNum; /* Arfcn */
    VOS_UINT16 rxAntSel;  /* Ant1/Ant2 */
    VOS_UINT16 rsv;       /* 保留位 */

} C_RfCfgPara;

/* AT_HPA 参数配置原语接口 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16  msgId;     /* Msg ID */
    VOS_UINT16  rsv;       /* 保留位 */
    C_RfCfgPara rfCfgPara; /* RF配置参数结构 */
} AT_CHPA_RfCfgReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;    /* Msg ID */
    VOS_UINT16 rsv;      /* 保留位 */
    VOS_UINT16 measNum;  /* RSSI的测量次数 */
    VOS_UINT16 interval; /* RSSI的测量间隔时间，单位ms */
} AT_CHPA_RfRxRssiReq;

/* W的RF配置结构 */
/* RF配置参数结构 */
typedef struct {
    /* TX RX均使用 */
    VOS_INT16 rrcWidth; /* 该变量暂时不用,双载波模式时,该值为RRC带宽,范围[-5,5]MHz */

    /* Tx Cfg */
    VOS_UINT16 mask;      /* 按位标识配置类型 */
    VOS_UINT16 txAfcInit; /* AFC */
    VOS_UINT16 txBand;    /* 1,2,3...,协议中的band编号,注意不是BandId */
    VOS_UINT16 txFreqNum; /* Arfcn */
    VOS_UINT16 txEnable;  /* TX通道使能控制 */
    VOS_UINT16 txPaMode;  /* PA模式控制 */
    VOS_INT16  txPower;   /* 0.1dBm */
    VOS_INT16  dbbAtten;  /* DBB对信号衰减的控制字 映射到usTxAGC???，但含义不一致，找深圳确认 */
    VOS_UINT16 rfAtten;   /* RF对信号衰减的控制字 */
    VOS_UINT16 txAgc;     /* 发射AGC控制 */
    VOS_UINT16 paVbias;   /* W PA Vbias 的控制字 */

    /* Rx Cfg */
    VOS_UINT16 rxEnable;      /* RX通道使能控制 */
    VOS_UINT16 rxBand;        /* 1,2,3...,协议中的band编号 */
    VOS_UINT16 rxFreqNum;     /* Arfcn */
    VOS_UINT16 rxAntSel;      /* Ant1/Ant2 */
    VOS_UINT16 rxLnaGainMode; /* LNA模式控制 */

    VOS_UINT16 rxCarrMode; /* 0:单载波;1:双载波 */
} W_RfCfgPara;

/* AT_HPA 参数配置原语接口 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16  msgId;     /* Msg ID */
    VOS_UINT16  rsv;       /* 保留位 */
    W_RfCfgPara rfCfgPara; /* RF配置参数结构 */
} AT_HPA_RfCfgReq;

/* G的RF配置结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;   /* Msg ID */
    VOS_UINT16 rsv;     /* 保留位 */
    VOS_UINT16 mask;    /* Reference MASK_CAL_RF_G_RX_* section */
    VOS_UINT16 freqNum;
    VOS_UINT16 rxMode;  /* 0:burst接收; 1:连续接收;, */
    VOS_UINT16 agcMode; /* Fast AGC,Slow AGC */
    VOS_UINT16 agcGain; /* AGC档位，共四档,取值为0-3 */
    VOS_UINT16 rsv2;
} AT_GHPA_RfRxCfgReq;

enum AT_GHPA_RfCtrlmodeType {
    AT_GHPA_RF_CTRLMODE_TYPE_GMSK,
    AT_GHPA_RF_CTRLMODE_TYPE_TXPOWER,
    AT_GHPA_RF_CTRLMODE_TYPE_8PSK,
    AT_GHPA_RF_CTRLMODE_TYPE_BUTT
};
typedef VOS_UINT16 AT_GHPA_RfCtrlmodeTypeUint16;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16                   msgId;    /* Msg ID */
    VOS_UINT16                   rsv;      /* 保留位 */
    VOS_UINT16                   mask;     /* Reference MASK_CAL_RF_G_TX_* section */
    VOS_UINT16                   afc;      /* AFC */
    VOS_UINT16                   modType;  /* 发射调制方式:0表示GMSK调制;1表示8PSK调制方式 */
    VOS_UINT16                   freqNum;
    VOS_UINT16                   txEnable; /* 发送使能控制:0x5555-使能发送;0xAAAA-停止发送;TSC 0; TxData: 随机数 */
    VOS_UINT16                   txMode;   /* 0:burst发送; 1:连续发送 */
    AT_GHPA_RfCtrlmodeTypeUint16 ctrlMode; /* 发射控制方式：
                                            * 0：GMSK电压控制,此方式下usTxVpa要配置；
                                            * 1：功率控制,此方式下usTxPower要配置；
                                            * 2：8PSK PaVbias电压&DBB Atten&RF Atten控制，
                                            * usPAVbias和usRfAtten,sDbbAtten三个参数都要配置；
                                            */
    VOS_UINT16 reserved;
    VOS_UINT32 rfAtten;
    VOS_UINT16 txPower; /* 目标发射功率,单位为0.1dBm,GSM和EDGE共用 */
    VOS_UINT16 txVpa;   /* GSM PA发射功率控制电压,取值范围: */
    VOS_INT16  dbbAtten;
    VOS_UINT16 paVbias; /* EDGE PA Vbais 电压 */
} AT_GHPA_RfTxCfgReq;

/* Response Head Struct    W/G通用 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 transPrimId; /* 0x8001 */
    VOS_UINT16 rsv1;        /* 保留位 */
    VOS_UINT16 msgId;       /* Msg ID */
    VOS_UINT16 rsv;         /* 保留位 */
} HPA_AT_Header;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT16    cfgMsgId; /* 接收到的消息ID */
    VOS_UINT16    errFlg;   /* 0:success, 1:fail */
} HPA_AT_RfCfgCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;    /* Msg ID */
    VOS_UINT16 rsv;      /* 保留位 */
    VOS_UINT16 measNum;  /* RSSI的测量次数 */
    VOS_UINT16 interval; /* RSSI的测量间隔时间，单位ms */
} AT_HPA_RfRxRssiReq;

/* 接收到测量质量上报请求后，上报前一次接收的RSSI测量结果  G/W通用 */
typedef struct {
    HPA_AT_Header msgHeader;
    VOS_INT16     rssi;    /* RSSI测量值 [-2048,+2047]，单位0.125dBm */
    VOS_INT16     agcGain; /* 取得测量值时的增益状态，-1表示出错 */
} HPA_AT_RfRxRssiInd;

typedef struct {
    HPA_AT_Header msgHeader;          /* 消息头 */
    VOS_INT16     gammaReal;          /* 反射系数实部 */
    VOS_INT16     gammaImag;          /* 反射系数虚部 */
    VOS_UINT16    gammaAmpUc0;        /* 驻波检测场景0反射系数幅度 */
    VOS_UINT16    gammaAmpUc1;        /* 驻波检测场景1反射系数幅度 */
    VOS_UINT16    gammaAmpUc2;        /* 驻波检测场景2反射系数幅度 */
    VOS_UINT16    gammaAntCoarseTune; /* 粗调格点位置 */
    VOS_UINT32    fomcoarseTune;      /* 粗调FOM值 */
    VOS_UINT16    cltAlgState;        /* 闭环算法收敛状态 */
    VOS_UINT16    cltDetectCount;     /* 闭环收敛总步数 */
    VOS_UINT16    dac0;               /* DAC0 */
    VOS_UINT16    dac1;               /* DAC1 */
    VOS_UINT16    dac2;               /* DAC2 */
    VOS_UINT16    dac3;               /* DAC3 */
} WPHY_AT_TxCltInd;

typedef struct {
    VOS_UINT16 dlFreq;    /* PHY上报当前测量频点 */
    VOS_INT16  dlPriRssi; /* PHY上报对应主集RSSI */
    VOS_INT16  dlDivRssi; /* PHY上报对应分集RSSI */
    VOS_UINT16 rsv;
} PHY_NOISE_Result;

typedef struct {
    VOS_UINT16       dlRssiNum; /* PHY上报测量RSSI个数 */
    VOS_UINT16       meaStatus; /* PHY上报测量状态，0表示成功，1表示参数错误，2表示超时 */
    PHY_NOISE_Result dlRssiResult[AT_DPS_NOISERSSI_MAX_NUM];
} HPA_NOISE_RssiInd;

typedef struct {
    HPA_AT_Header     msgHeader; /* 标准头 */
    HPA_NOISE_RssiInd rssiInd;   /* 结果 */
} PHY_AT_RfNoiseRssiInd;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;   /* Msg ID */
    VOS_UINT16 rsv1;    /* 保留位 */
    VOS_UINT16 dspBand; /* DSP格式的频段值 */
    VOS_UINT16 rsv2;    /* 保留位 */
} AT_PHY_RfPllStatusReq;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT16    txStatus; /* 0:PLL失锁, 1:PLL锁定 */
    VOS_UINT16    rxStatus; /* 0:PLL失锁, 1:PLL锁定 */
} PHY_AT_RfPllStatusCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId; /* Msg ID */
    VOS_UINT16 rsv;   /* 保留位 */
} AT_PHY_PowerDetReq;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_INT16     powerDet; /* 功率检测结果，0.1dBm精度 */
    VOS_UINT16    rsv;      /* 保留位 */
} PHY_AT_PowerDetCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;       /* Msg ID */
    VOS_UINT16 mode;        /* UE 模式 */
    VOS_UINT16 band;        /* band信息 */
    VOS_UINT16 dlStartFreq; /* 下行开始频点 */
    VOS_UINT16 dlEndFreq;   /* 下行结束频点 */
    VOS_UINT16 freqStep;    /* 步长 */
    VOS_INT16  txPwr;       /* 发射功率 */
    VOS_UINT16 rsv;         /* 保留位 */
} AT_HPA_RfNoiseCfgReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;
    VOS_UINT16 secondaryAddr;
    VOS_UINT16 regAddr;
    VOS_UINT16 regData;
    VOS_UINT16 mipiChannel;
    VOS_UINT16 rsv;
} AT_HPA_MipiWrReq;


typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT32    errFlg; /* 0:success, 1:fail */
} HPA_AT_MipiWrCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;         /* Msg ID */
    VOS_UINT16 secondaryAddr; /* 接收端地址 */
    VOS_UINT16 reg;           /* 寄存器个数 */
    VOS_UINT16 channel;       /* 开始的寄存器 */
} AT_HPA_MipiRdReq;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT32    value;
    VOS_UINT32    result;
} HPA_AT_MipiRdCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId;
    VOS_UINT16 rficSsi; /* Mipi */
    VOS_UINT16 regAddr; /* 开始的寄存器 */
    VOS_UINT16 data;    /* 数据 */
} AT_HPA_SsiWrReq;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT32    errFlg; /* 0:success, 1:fail */
} HPA_AT_SsiWrCnf;

/* 接收到测量质量上报请求后，上报前一次接收的RSSI测量结果  G/W通用 */
typedef struct {
    HPA_AT_Header msgHeader;
    VOS_INT16     rssi;    /* RSSI测量值 [-2048,+2047]，单位0.125dBm */
    VOS_INT16     agcGain; /* 取得测量值时的增益状态，-1表示出错 */
} CHPA_AT_RfRxRssiInd;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT16    cfgMsgId; /* 接收到的消息ID */
    VOS_UINT16    errFlg;   /* 0:success, 1:fail */
} CHPA_AT_RfCfgCnf;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId; /* Msg ID */
    VOS_UINT16 rsv;   /* 保留位 */
    VOS_UINT16 pdmRegValue;
    VOS_UINT16 paVbias;
    VOS_UINT16 paVbias2;
    VOS_UINT16 paVbias3;
} AT_HPA_PdmCtrlReq;

typedef struct {
    HPA_AT_Header msgHeader;
    VOS_UINT32    result;
} HPA_AT_PdmCtrlCnf;

#ifndef MODEM_FUSION_VERSION
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId; /* Msg ID */
    VOS_UINT16 rsv;
    VOS_UINT32 testNum;     /* 测试次数，这只是AT命令配置的值，单位PHY与工具确认 */
    VOS_UINT16 syncLen;     /* 上行sync rept len */
    VOS_UINT16 burstTime;   /* 上行Burst时间，单位MC */
    VOS_UINT16 stallTime;   /* 上行Stall时间，单位MC */
    VOS_UINT16 dlSyncLen;   /* 下行sync rept len */
    VOS_UINT16 dlBurstTime; /* 下行Burst时间，单位MC */
    VOS_UINT16 dlStallTime; /* 下行Stall时间，单位MC */
    VOS_UINT16 lsTest;
    VOS_UINT16 fwdClkAmp;
} FESS_FLPM_NasSerdesAgingReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgID;
    VOS_UINT16 type;
    VOS_UINT32 para[AT_SERDES_PARA_MAX_CNT];
} FESS_FLPM_NasAtNobizReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16         msgId; /* Msg ID */
    VOS_UINT16         rsv;
    AT_PHY_SltTestType testType;
    VOS_UINT8          rsv1[3];
} FESS_FLPM_NasSerdesSltReq;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT16 msgId; /* Msg ID */
    VOS_UINT16 rsv;
    VOS_INT32  result; /* 0: succ；其他值:fail */
} FESS_FLPM_NasAtCnf;
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of at_phy_interface.h */
