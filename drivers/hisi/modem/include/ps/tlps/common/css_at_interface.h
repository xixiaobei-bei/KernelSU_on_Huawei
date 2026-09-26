/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, 
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

#ifndef CSS_AT_INTERFACE_H
#define CSS_AT_INTERFACE_H

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

#define MCC_INFO_VERSION_LEN (9)
#define AT_CSS_MAX_MCC_ID_NUM (17)
#define AT_CSS_BLOCK_CELL_LIST_VERSION_LEN (9)
/* add for CloudLine Feature 2017-6-28 begin */
#define AT_CSS_CLOUD_LINE_VERSION_LEN (9) /* 云端预置高铁线路版本号字符串长度 */
#define AT_CSS_PLMN_MAX_LINE_NUM (64)     /* 云端预置高铁线路一个PLMN下包含的最大线路个数 */
#define AT_CSS_TACLAC_MAX_LINE_NUM 6   /* 云端预置高铁线路一个TAC/LAC所对应的最大线路个数 */
/* add for CloudLine Feature 2017-6-28 end */

/* Added for EMC, 2018-1-9, begin */
#define AT_CSS_MRU_MAX_NUM (10) /* MRU可以存储的最大个数 */
/* Added for EMC, 2018-1-9, end */
/* plmn deployment, add begin, 2019-11-06 */
#define AT_CSS_PLMN_DEPLOYMENT_VERSION_LEN 9 /* 云预置布网策略版本号字符串长度 */
#define AT_CSS_MSG_DATALEN_MAX_SIZE 1500 /* AT->CSS消息携带的dataLen最大值 */
/* plmn deployment, add end, 2019-11-25 */
#define AT_CSS_MSG_DATALEN_REPORT_MAX_SIZE 500 /* CSS->AT消息携带的dataLen最大值 */
#define AT_CSS_CLOUD_LINE_INVALID_LINE_INDEX 0xFFFF /* AP配置索引表lineId为65535表示该plmn下没有高铁线路信息 */
#define AT_CSS_CLOUD_LINE_NSA_CELL_MAX_SCG_NUM 3 /* 高铁预置路径信息单个NSA小区最大SCG个数 */
#define AT_CSS_CLOUD_LINE_MAX_LINE_NUM 6 /* CSS最多存储6条线路的详细信息 */
#define AT_CSS_CLOUD_LINE_MAX_NODE_NUM 130 /* 每条线路中最多130个节点 */
#define AT_CSS_CLOUD_LINE_MAX_CELL_NUM 7 /* 每个节点中最多7个小区，包括LTE/SA/NSA三种类型的小区 */
#define AT_CSS_CLOUD_LINE_MAX_NSA_CELL_NUM 2 /* 每个节点的7个小区中最多2个NSA小区 */
#define CSS_AT_MSG_ID_HEADER 0x1000



enum CSS_AT_MsgType {
    /* AT->CSS */
    /* _MSGPARSE_Interception  AT_CSS_MccInfoSetReq */
    ID_AT_CSS_MCC_INFO_SET_REQ          = 0x0001,
    /* _MSGPARSE_Interception  AT_CSS_MccVersionInfoReq */
    ID_AT_CSS_MCC_VERSION_INFO_REQ      = 0x0002,
    /* _MSGPARSE_Interception  AT_CSS_BlockCellListSetReq */
    ID_AT_CSS_BLOCK_CELL_LIST_SET_REQ   = 0x0003,
    /* _MSGPARSE_Interception  AT_CSS_BlockCellListQueryReq */
    ID_AT_CSS_BLOCK_CELL_LIST_QUERY_REQ = 0x0004,
    /* Added for EMC, 2018-1-9, begin */
    /* Added for EMC, 2018-1-9, end */
    /* plmn deployment, add begin, 2019-11-06 */
    /* _MSGPARSE_Interception AT_CSS_PlmnDeploymentInfoSetReq  */
    ID_AT_CSS_PLMN_DEPLOYMENT_INFO_SET_REQ = 0x000b,
    /* plmn deployment, add end, 2019-11-25 */
    /* _MSGPARSE_Interception  AT_CSS_CloudDataSetReq */
    ID_AT_CSS_CLOUD_DATA_SET_REQ = 0x000c,
    /* _MSGPARSE_Interception  AT_CSS_ScreenStatusInd */
    ID_AT_CSS_SCREEN_STATUS_IND  = 0x000d,

    /* CSS->AT */
    ID_CSS_AT_MCC_INFO_SET_CNF          = CSS_AT_MSG_ID_HEADER + 0x01, /* _H2ASN_MsgChoice CSS_AT_MccInfoSetCnf -1_1 */
    ID_CSS_AT_MCC_VERSION_INFO_CNF      = CSS_AT_MSG_ID_HEADER + 0x02, /* _H2ASN_MsgChoice CSS_AT_MccVersionInfoCnf -1_1 */
    ID_CSS_AT_QUERY_MCC_INFO_NOTIFY     = CSS_AT_MSG_ID_HEADER + 0x03, /* _H2ASN_MsgChoice CSS_AT_QueryMccInfoNotify -1_1 */
    ID_CSS_AT_BLOCK_CELL_LIST_SET_CNF   = CSS_AT_MSG_ID_HEADER + 0x04, /* _H2ASN_MsgChoice CSS_AT_BlockCellListSetCnf -1_1 */
    ID_CSS_AT_BLOCK_CELL_LIST_QUERY_CNF = CSS_AT_MSG_ID_HEADER + 0x05, /* _H2ASN_MsgChoice CSS_AT_BlockCellListQueryCnf -1_1 */
    ID_CSS_AT_BLOCK_CELL_MCC_NOTIFY     = CSS_AT_MSG_ID_HEADER + 0x06, /* _H2ASN_MsgChoice CSS_AT_BlockCellMccNotify -1_1 */

    ID_CSS_AT_PLMN_DEPLOYMENT_INFO_SET_CNF = CSS_AT_MSG_ID_HEADER + 0x0f, /* _H2ASN_MsgChoice  CSS_AT_PlmnDeploymentInfoSetCnf -1_1 */
    ID_CSS_AT_CLOUD_DATA_SET_CNF = CSS_AT_MSG_ID_HEADER + 0x10, /* _H2ASN_MsgChoice CSS_AT_CloudDataSetCnf -1_1 */
    ID_CSS_AT_CLOUD_DATA_REPORT  = CSS_AT_MSG_ID_HEADER + 0x11, /* _H2ASN_MsgChoice CSS_AT_CloudDataReport -1_1 */

    ID_CSS_AT_MSG_BUTT
};
typedef VOS_UINT32 CSS_AT_MsgTypeUint32;

enum AT_CSS_Rat {
    AT_CSS_RAT_TYPE_GSM = 0, /* GSM接入技术 */
    AT_CSS_RAT_TYPE_WCDMA,   /* WCDMA接入技术 */
    AT_CSS_RAT_TYPE_LTE,     /* LTE接入技术 */

    AT_CSS_RAT_TYPE_BUTT
};
typedef VOS_UINT8 AT_CSS_RatUint8;

enum AT_CSS_SetMccOperateType {
    AT_CSS_SET_MCC_TYPE_ADD_MCC = 0,    /* 新增MCC */
    AT_CSS_SET_MCC_TYPE_DELETE_ALL_MCC, /* 删除所有MCC信息 */
    AT_CSS_SET_MCC_TYPE_DELETE_ONE_MCC, /* 删除一个MCC信息 */

    AT_CSS_SET_MCC_TYPE_BUTT
};
typedef VOS_UINT8 AT_CSS_SetMccOperateTypeUint8;


enum AT_CSS_BlockCellListOperateType {
    AT_CSS_BLOCK_CELL_LIST_ADD_ONE = 0, /* 新增一条BLOCK CELL信息 */
    AT_CSS_BLOCK_CELL_LIST_DELETE_ALL,  /* 删除所有BLOCK CELL信息 */

    AT_CSS_BLOCK_CELL_LIST_TYPE_BUTT
};
typedef VOS_UINT8 AT_CSS_BlockCellListOperateTypeUint8;

/* add for CloudLine Feature 2017-6-29 begin */
/* 结构说明: 高铁索引表、详细表操作类型 */
enum AT_CSS_CloudLineOperationType {
    AT_CSS_CLOUD_LINE_OPERATION_TYPE_ADD = 0,    /* 新增 */
    AT_CSS_CLOUD_LINE_OPERATION_TYPE_DELETE_ALL, /* 删除 */

    AT_CSS_CLOUD_LINE_OPERATION_TYPE_BUTT
};
typedef VOS_UINT8 AT_CSS_CloudLineOperationTypeUint8;

/* 结构说明: 高铁高铁详细信息表类型 */
enum AT_CSS_LineDetailInfoType {
    AT_CSS_LINE_DETAIL_INFO_TACLAC_PAIR = 0, /* TACLAC对 */
    AT_CSS_LINE_DETAIL_INFO_GSM_HO_PATH,     /* GSM HO或CSFB预置路径 */
    AT_CSS_LINE_DETAIL_INFO_GSM_HO_BAR,      /* GSM HO BAR路径 */
    /* add for Cloudline Lte Feature 2018-4-16 begin */
    AT_CSS_LINE_DETAIL_INFO_LTE_SA_NSA_HO_PATH, /* LTE/SA/NSA高铁预置路径 */
    /* add for Cloudline Lte Feature 2018-4-16 end */
    AT_CSS_LINE_DETAIL_INFO_NO_INFO = 0xFF /* 表示没有消息信息，停止解析 */
};
typedef VOS_UINT8 AT_CSS_LineDetailInfoTypeUint8;
/* add for CloudLine Feature 2017-6-29 end */

/* 结构说明: AT配置的云端数据特性类型 */
enum AT_CSS_CloudDataType {
    AT_CSS_CLOUD_DATA_TYPE_CLOUD_LINE = 0, /* 高铁预置路径 */
    AT_CSS_CLOUD_DATA_TYPE_BUTT
};
typedef VOS_UINT32 AT_CSS_CloudDataTypeUint32;

/* 结构说明: 云通信数据类型 */
enum AT_CSS_CloudLineDataType {
    AT_CSS_CLOUD_LINE_DATA_TYPE_LINE_INDEX  = 0, /* AP下发高铁预置路径索引表 */
    AT_CSS_CLOUD_LINE_DATA_TYPE_LINE_DETAIL = 1, /* AP下发高铁预置路径详细表 */
    AT_CSS_CLOUD_LINE_DATA_TYPE_REPORT_PLMN = 2, /* MODEM上报当前PLMN */
    AT_CSS_CLOUD_LINE_DATA_TYPE_REPORT_LINE = 3, /* MODEM上报当前线路信息 */
    AT_CSS_CLOUD_LINE_DATA_TYPE_REPORT_NODE = 4, /* MODEM上报当前NODE节点信息 */

    AT_CSS_CLOUD_LINE_DATA_TYPE_BUTT
};
typedef VOS_UINT32 AT_CSS_CloudLineDataTypeUint32;

/* 结构说明: 亮灭屏状态 */
enum AT_CSS_ScreenStatusType {
    AT_CSS_SCREEN_STATUS_OFF = 0, /* 灭屏 */
    AT_CSS_SCREEN_STATUS_ON  = 1, /* 亮屏 */
    AT_CSS_SCREEN_STATUS_BUFF
};
typedef VOS_UINT8 AT_CSS_ScreenStatusTypeUint8;

typedef struct {
    /*
     * MCC在aucMcc[2]中的存放格式,mcc为460:
     *              ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
     *     ---------------------------------------------------------------------------
     * aucMcc[0]    ||    MCC digit 2 = 6        |           MCC digit 1 = 4
     *     ---------------------------------------------------------------------------
     * aucMcc[1]    ||    无效                   |           MCC digit 3 = 0
     */
    VOS_UINT8 mcc[2]; /* MCC ID,共2字节 */
    VOS_UINT8 rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
} AT_CSS_MccId;


typedef struct {
    VOS_UINT32 mcc;
    VOS_UINT32 mnc;
} CSS_AT_PlmnId;

typedef struct {
    VOS_UINT32 freqHigh;
    VOS_UINT32 freqLow;
} AT_CSS_FreqRange;

typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT16     clientId;
    VOS_UINT8      rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
} AT_CSS_MccVersionInfoReq;

typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT16     clientId;
    VOS_UINT8      rsv[1]; /* 保留字段，长度为1,便于4字节对齐 */
    VOS_UINT8      versionId[MCC_INFO_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
} CSS_AT_MccVersionInfoCnf;


typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    /* add for fill right clientId 2016-02-23, begin  */
    MODEM_ID_ENUM_UINT16 modemId;
    /* add for fill right clientId 2016-02-23, end  */
    VOS_UINT16                    clientId;
    VOS_UINT8                     seq;                             /* 流水号 */
    AT_CSS_SetMccOperateTypeUint8 operateType;                     /* 操作类型 */
    VOS_UINT8                     versionId[MCC_INFO_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
    /* modified for fill right clientId 2016-02-23, begin  */
    VOS_UINT8 rsv[1]; /* 保留字段，长度为1,便于4字节对齐 */
    /* modified for fill right clientId 2016-02-23, end  */

    /*
     * 1)aucMccINfoBuff里存储的是MCC的信息，存储区的真实大小是ulMccInfoBuffLen里记录的字节数；
     * 2)aucMccINfoBuff里的格式为AT_CSS_MCC_INFO_STRU结构的格式，其中MNC个数,
     *   BAND个数，预置频段个数，预置频点的个数都是可变的。
     * typedef struct
     * {
     *     VOS_UINT8                           supportFlag;    // 1:表示支持GSM 2:表示支持WCDMA 4:表示支持LTE，三者可以自由组合
     *     AT_CSS_FreqRange              freqRange;
     * } AT_CSS_FreqRangeWithRat;
     * typedef struct
     * {
     *     VOS_UINT8                                   bandInd;
     *     VOS_UINT8                                   breqRangNum;
     *     //后面必须紧跟ucFreqRangNum个AT_CSS_FreqRangeWithRat结构
     *       的FreqRange信息，如果没有FreqRange，则需要将ucFreqRangNum填为0
     *     AT_CSS_FreqRangeWithRat             freqRangeArray[ucFreqRangNum];
     *     VOS_UINT8                                   preConfigFreqNum;
     *     //后面必须紧跟ucPreConfigFreqNum个AT_CSS_FreqRangeWithRat结构
     *       的PreConfigFreq信息，如果没有PreConfigFreq，则需要将ucPreConfigFreqNum填为0
     *     AT_CSS_FreqRangeWithRat             preConfigFreqArray[ucPreConfigFreqNum];      //预置频点列表
     * }AT_CSS_BandInfo;
     * typedef struct
     * {
     *     MNC在aucMnc[2]中的存放格式，mnc为01:
     *         ---------------------------------------------------------------------------
     *                  ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
     *         ---------------------------------------------------------------------------
     *     aucMnc[0]    ||    MNC digit 3 = f        |           无效
     *         ---------------------------------------------------------------------------
     *     aucMnc[1]    ||    MNC digit 2 = 1        |           MNC digit 1 = 0
     *         ---------------------------------------------------------------------------
     *     VOS_UINT8                           aucMnc[2];
     *     VOS_UINT8                           ucBandCount;// BAND的个数
     *     //后面必须紧跟ucBandCount个band的信息，如果没有BAND，则需要将ucBandCount填为0
     *     AT_CSS_BandInfo               bandArray[ucBandCount];
     * }AT_CSS_MncInfo;
     * typedef struct
     * {
     *     MCC在aucMcc[2]中的存放格式,mcc为460:
     *         ---------------------------------------------------------------------------
     *                  ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
     *         ---------------------------------------------------------------------------
     *     aucMcc[0]    ||    MCC digit 2 = 6        |           MCC digit 1 = 4
     *         ---------------------------------------------------------------------------
     *     aucMcc[1]    ||    无效                   |           MCC digit 3 = 0
     *         ---------------------------------------------------------------------------
     *     VOS_UINT8                           aucMcc[2];
     *     VOS_UINT8                           ucMncCount;// MNC的个数
     *     //后面必须紧跟ucMncCount个AT_CSS_MncInfo结构的mnc信息，如果没有mnc，则需要将ucMncCount填为0
     *     AT_CSS_MncInfo                mncAarry[ucMncCount];
     * }AT_CSS_MCC_INFO_STRU;
     * 3)aucMccINfoBuff中存储信息的格式AP与CSS直接对接，AT不会进行修改，AP先形成上述的格式，
     *   然后转成字符串格式发给AT，AT将字符串格式还原二进制码流，然后发给CSS；
     *   例如AP形成某一个字节为0x22，然后转化为字符串'22'，AT收到后再转成0x22;
     * 4)aucMccINfoBuff中的格式为小端；
     * 5)ulMccInfoBuffLen不能超过1.6K；
     */
    VOS_UINT32 mccInfoBuffLen;
    VOS_UINT8  mccInfoBuff[4]; /* bug前4字节 */
} AT_CSS_MccInfoSetReq;


typedef struct {
    VOS_MSG_HEADER                              /* _H2ASN_Skip */
        VOS_UINT32                       msgId; /* _H2ASN_Skip */
    VOS_UINT16                           clientId;
    VOS_UINT8                            seq;                                           /* 流水号 */
    AT_CSS_BlockCellListOperateTypeUint8 operateType;                                   /* 操作类型 */
    VOS_UINT8                            versionId[AT_CSS_BLOCK_CELL_LIST_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
    AT_CSS_RatUint8                      ratType;                                       /* 系统制式 */
    VOS_UINT8                            rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */

    /*
     * 1)aucBlockCellListBuff里存储的是BLOCK CELL的信息，存储区的真实大小是blockCellListBuffLen里记录的字节数；
     * 2)aucBlockCellListBuff里的格式为AT_CSS_GsmCloudBlockcell结构的格式
     * 码流参数BS1,BS2和BS3的编码格式相同，都按AT_CSS_GsmCloudBlockcell结构的格式编码：
     * 其中ucPairCount，astBlockListArray是可以变的
     * typedef struct
     * {
     *     CSS_PlmnId    plmn;
     *     VOS_UNIT8           pairCount;
     *     //后面必须紧跟ucPairCount个AT_CSS_GsmCloudBlockcellPair结构的pair信息，
     *       如果没有如果该mcc没有BLOCK CELL信息，AP不需要下发该MCC的信息，Modem认为该MCC没有BLOCK CELL信息，
     *       后续不再要求上报^REPORTBLOCKCELLMCC来获取BLOCK CELL
     *     AT_CSS_GsmCloudBlockcellPair blockListPairArray[ucPairCount];
     * }AT_CSS_GsmCloudBlockcell;
     * typedef struct
     * {
     *     VOS_UNIT32  ulMcc; //Modem内部plmn格式;比如460---0x00000604
     *     VOS_UNIT32  ulMnc; //Modem内部plmn格式;比如 01---0x000F0100
     * }CSS_PlmnId;
     * typedef struct
     * {
     *     VOS_UNIT16  usSourceCellLai;
     *     VOS_UNIT16  usSourceCellId;
     *     VOS_UNIT16  usBlockCellArfcn; //包含了GSM的频段信息，比如0x1029表示GSM900频段,41频点
     *     VOS_UNIT8   ucBlockCellBsic;  //包含ncc,bcc;比如0x14表示<2,4>
     *     VOS_UNIT8   ucBlockCellReduceVal;//针对本BLOCK CELL惩罚时降低能量db值
     * }AT_CSS_GsmCloudBlockcellPair;
     */

    VOS_UINT32 blockCellListBuffLen;
    VOS_UINT8  blockCellListBuff[4]; /* buf前4字节 */
} AT_CSS_BlockCellListSetReq;


typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT16     clientId;
    VOS_UINT8      rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
} AT_CSS_BlockCellListQueryReq;

typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT16     clientId;
    VOS_UINT8      seq; /* 流水号 */
    VOS_UINT8      rsv[1]; /* 保留字段，长度为1,便于4字节对齐 */

    VOS_UINT32 result; /* 0表示成功，1表示失败 */
} CSS_AT_MccInfoSetCnf;

typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT16     clientId;
    VOS_UINT8      rsv[1]; /* 保留字段，长度为1,便于4字节对齐 */
    VOS_UINT8      versionId[MCC_INFO_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
    VOS_UINT32     mccNum;                          /* MCC 个数 */
    AT_CSS_MccId   mccId[AT_CSS_MAX_MCC_ID_NUM];    /* MCC ID列表 */
} CSS_AT_QueryMccInfoNotify;


typedef struct {
    VOS_UINT32 mccNum;                     /* MCC 个数 */
    VOS_UINT32 mcc[AT_CSS_MAX_MCC_ID_NUM]; /* MCC 列表,Modem内部plmn格式;比如460---0x00000604 */
} CSS_AT_BlockCellMccInfo;


typedef struct {
    VOS_MSG_HEADER        /* _H2ASN_Skip */
    VOS_UINT32     msgId; /* _H2ASN_Skip */
    VOS_UINT16     clientId;
    VOS_UINT8      seq;   /* 流水号 */
    VOS_UINT8      reserved;
    VOS_UINT32     result; /* VOS_OK表示成功，VOS_ERR表示失败 */
} CSS_AT_BlockCellListSetCnf;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    VOS_UINT32              msgId; /* _H2ASN_Skip */
    VOS_UINT16              clientId;
    VOS_UINT8               reserved;
    VOS_UINT8               versionId[AT_CSS_BLOCK_CELL_LIST_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
    CSS_AT_BlockCellMccInfo mccInfo;                                       /* BLOCK CELLMCC信息 */
} CSS_AT_BlockCellListQueryCnf;


typedef struct {
    VOS_MSG_HEADER                 /* _H2ASN_Skip */
    VOS_UINT32              msgId; /* _H2ASN_Skip */
    VOS_UINT16              clientId;
    VOS_UINT8               reserved;
    VOS_UINT8               versionId[AT_CSS_BLOCK_CELL_LIST_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
    CSS_AT_BlockCellMccInfo mccInfo;                                       /* BLOCK CELLMCC信息 */
} CSS_AT_BlockCellMccNotify;


/* add for CloudLine Feature 2017-6-29 begin */



typedef struct {
    VOS_MSG_HEADER                                            /* _H2ASN_Skip */
    VOS_UINT32 msgId;                                         /* _H2ASN_Skip */
    VOS_UINT16     clientId;                                  /* _H2ASN_Skip */
    VOS_UINT16     lineNum;                                   /* 线路个数 */
    VOS_UINT16     lineIndexList[AT_CSS_TACLAC_MAX_LINE_NUM]; /* 线路列表 */
    CSS_AT_PlmnId  plmnId;                                    /* plmn id */
} CSS_AT_LineIndexNotify;

/* add for CloudLine Feature 2017-6-29 end */

/* plmn deployment, add begin, 2019-11-06 */

typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    CSS_AT_MsgTypeUint32 msgId; /* _H2ASN_Skip */
    VOS_UINT16           clientId;
    MODEM_ID_ENUM_UINT16 modemId;
    VOS_UINT8            seq; /* 流水号 */
    VOS_UINT8            versionId[AT_CSS_PLMN_DEPLOYMENT_VERSION_LEN]; /* 版本号，固定为xx.xx.xxx */
    VOS_UINT8            rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */

    /*
     * 1)data里存储的是云预置PLMN布网策略的信息，存储区的真实大小是dataLen里记录的字节数。
     * 2)data里的格式为CSS_PlmnDeploymentInfo结构的格式:
     *   当plmnNum为0时，代表下发的是默认策略。当重复下发默认策略时，后面的覆盖前面的。
     *   CSS收到相同策略的不同PLMN时，增加PLMN个数；当收到相同策略的相同PLMN时，剔除重复的。
     *   CSS受内存的约束尽力保存下发下来的配置，对于当前不识别的策略丢弃，保证向后兼容。
     * 3)data中存储信息的格式AP与CSS直接对接，AT不会进行修改，AP先形成CSS_PlmnDeploymentInfo的格式，
     *   然后转成字符串格式发给AT，AT对字符串内容不关心，只是将字符串格式还原二进制码流转发给CSS，
     *   例如AP形成某一个字节为0x22，然后转化为字符串'22'，AT收到后再转成0x22；
     * 4)data中的格式为小端；
     * 5)dataLen不能超过1.6K；
     *
     * enum CSS_PlmnDeploymentType {
     *     CSS_PLMN_DEPLOY_LTE_ONLY = 0,
     *     CSS_PLMN_DEPLOY_NSA_ONLY,
     *     CSS_PLMN_DEPLOY_SA_PREFER,
     *     CSS_PLMN_DEPLOY_NSA_PREFER,
     *
     *     CSS_PLMN_DEPLOY_BUTT
     * };
     * typedef VOS_UINT8 CSS_PlmnDeploymentTypeUint8;
     *
     * typedef struct {
     *     CSS_PlmnDeploymentTypeUint8 plmnDeployType;
     *     VOS_UINT32                  plmnNum;
     *     CSS_PlmnId                  plmn[plmnNum];
     * } CSS_NV_PlmnDeploymentInfo;
     */
    VOS_UINT32 dataLen;
    VOS_UINT8  data[4]; /* data前4字节 */
} AT_CSS_PlmnDeploymentInfoSetReq;


typedef struct {
    VOS_MSG_HEADER               /* _H2ASN_Skip */
    CSS_AT_MsgTypeUint32 msgId;  /* _H2ASN_Skip */
    VOS_UINT32           result; /* VOS_OK表示成功，VOS_ERR表示失败 */
    VOS_UINT16           clientId;
    VOS_UINT8            rsv[2]; /* 保留字段，长度为2,便于4字节对齐 */
} CSS_AT_PlmnDeploymentInfoSetCnf;
/* plmn deployment, add end, 2019-11-06 */

/*
 * 结构说明:AP通过此消息给CSS下发云通信数据
 */
typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    CSS_AT_MsgTypeUint32 msgId; /* _H2ASN_Skip */
    ModemIdUint16        modemId;
    VOS_UINT16           clientId;
    VOS_UINT32           dataLen;
    VOS_UINT8            data[4]; /* 消息前4字节 */
} AT_CSS_CloudDataSetReq;

/*
 * 结构说明:CSS通过此消息响应AP下发云通信数据的结果
 */
typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    CSS_AT_MsgTypeUint32 msgId; /* _H2ASN_Skip */
    VOS_UINT16           clientId;
    VOS_UINT8            reserve[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32           result;
} CSS_AT_CloudDataSetCnf;

/*
 * 结构说明:CSS通过该消息给AP发送云通信数据
 */
typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    CSS_AT_MsgTypeUint32 msgId; /* _H2ASN_Skip */
    VOS_UINT16           clientId;
    VOS_UINT8            reserve[2]; /* 保留字段，长度为2,便于4字节对齐 */
    VOS_UINT32           dataLen;
    VOS_UINT8            data[4]; /* 消息前4字节 */
} CSS_AT_CloudDataReport;

/*
 * 结构说明:AT通过该消息结构通知CSS亮灭屏状态
 */
typedef struct {
    VOS_MSG_HEADER              /* _H2ASN_Skip */
    CSS_AT_MsgTypeUint32 msgId; /* _H2ASN_Skip */
    VOS_UINT8            status;
    VOS_UINT8            reserve[3]; /* 保留字段，长度为3,便于4字节对齐 */
} AT_CSS_ScreenStatusInd;

/* ASN解析结构 */
typedef struct {
    VOS_UINT32 msgId; /* _H2ASN_MsgChoice_Export CSS_AT_MsgTypeUint32  */
    VOS_UINT8  msg[4]; /* 消息前4字节 */
    /* _H2ASN_MsgChoice_When_Comment          CSS_AT_MSG_TYPE_ENUM_UINT32 */
} CSS_AT_InterfaceMsgData;
/* _H2ASN_Length UINT32 */

typedef struct {
    VOS_MSG_HEADER
    CSS_AT_InterfaceMsgData msgData;
} CSS_AT_InterfaceMsg;

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
