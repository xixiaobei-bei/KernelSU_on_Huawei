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

/*
 * 版权所有（c）华为技术有限公司 2012-2019
 * 功能描述: 电话本的的头文件
 * 生成日期: 2006年11月14日
 */

#ifndef __SIAPPPB_H__
#define __SIAPPPB_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "si_typedef.h"
#include "vos.h"
#include "usimm_ps_interface.h"
#include "nv_stru_pam.h"

#if (OSA_CPU_NRCPU != VOS_OSA_CPU)
#include "taf_oam_interface.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "ccore_nv_stru_pam.h"
#endif

#pragma pack(push, 4)

#if (FEATURE_PHONE_USIM == FEATURE_ON)

#define SI_PB_PHONENUM_MAX_LEN 40  /* 返回结构中号码的最大长度，是指字节数 */
#define SI_PB_ALPHATAG_MAX_LEN 242 /* 返回结构中名字的最大长度，是指字节数 */

#define SI_PB_EMAIL_MAX_LEN 64 /* 返回结构中邮件的最大长度，是指字节数 */

#define SI_PB_NUMBER_LEN 20 /* 协议栈下发的BCD号码长度 */

/* 哥伦比亚定制超过5个，暂时修改为支持16条 */
#define USIM_MAX_ECC_RECORDS 16
#define USIM_ECC_LEN 3

typedef SI_UINT32  SI_PB_AlphaTagTypeUint32;


/* 名字的编码类型，16-bit universal multiple-octet coded character set (ISO/IEC10646 [32]) */
#define SI_PB_ALPHATAG_TYPE_UCS2_80     0x80
#define SI_PB_ALPHATAG_TYPE_UCS2_81 0x81
#define SI_PB_ALPHATAG_TYPE_UCS2_82 0x82
#define SI_PB_ALPHATAG_TYPE_GSM 0x00 /* 名字的编码类型，GSM 7 bit default alphabet (3GPP TS 23.038) */
#define SI_PB_ALPHATAG_TYPE_UCS2 0x80

#define SI_PB_CONTENT_VALID 0x01
#define SI_PB_CONTENT_INVALID 0x00

#define SI_PB_LAST_TAG_TRUE 1

#define SI_PB_ALPHATAG_TRANSFER_TAG 0x1B

enum PB_FdnCheckRslt {
    PB_FDN_CHECK_SUCC = 0,
    PB_FDN_CHECK_NUM1_FAIL = 1,
    PB_FDN_CHECK_NUM2_FAIL = 2,
    PB_FDN_CHECK_BOTH_NUM_FAIL = 3,
    PB_FDN_CHECK_RSLT_BUTT
};
typedef VOS_UINT32 PB_FdnCheckRsltUint32;

/*
 * 功能说明: PB模块请求消息类型
 */
enum SI_PB_Req {
    SI_PB_READ_REQ = 0,       /* 号码本读取请求           */
    SI_PB_SET_REQ = 1,        /* 号码本设置请求           */
    SI_PB_MODIFY_REQ = 2,     /* 号码本修改请求           */
    SI_PB_DELETE_REQ = 3,     /* 号码本删除请求           */
    SI_PB_QUERY_REQ = 4,      /* 号码本查询请求           */
    SI_PB_ADD_REQ = 5,        /* 号码本添加请求           */
    SI_PB_SEARCH_REQ = 6,     /* 号码本查找请求           */
    SI_PB_SREAD_REQ = 7,      /* 复合号码本读取请求       */
    SI_PB_SMODIFY_REQ = 8,    /* 复合号码本修改请求       */
    SI_PB_SADD_REQ = 9,       /* 复合号码本添加请求       */
    SI_PB_UPDATE_AGOBAL = 10, /* 更新Acpu的全局变量内容   */
    SI_PB_UPDATE_CURPB = 11,  /* 更新当前电话本设置       */
    SI_PB_FDN_CHECK_REQ = 12, /* FDN号码匹配请求          */
    SI_PB_ECALL_QRY_REQ = 13, /* ECALL号码匹配请求        */

    SI_PB_REQ_BUTT
};
typedef SI_UINT32 SI_PB_ReqUint32;

/*
 * 功能说明: PB模块回复和上报消息类型
 */
enum SI_PB_Cnf {
    SI_PB_EVENT_INFO_IND = 0,        /* 号码本信息事件上报           */
    SI_PB_EVENT_READ_CNF = 1,        /* 号码本读取结果回复           */
    SI_PB_EVENT_QUERY_CNF = 2,       /* 号码本查询结果回复           */
    SI_PB_EVENT_SET_CNF = 3,         /* 号码本设置结果回复           */
    SI_PB_EVENT_ADD_CNF = 4,         /* 号码本添加结果回复           */
    SI_PB_EVENT_MODIFY_CNF = 5,      /* 号码本修改结果回复           */
    SI_PB_EVENT_DELETE_CNF = 6,      /* 号码本删除结果回复           */
    SI_PB_EVENT_SEARCH_CNF = 7,      /* 号码本搜索结果回复           */
    SI_PB_EVENT_SREAD_CNF = 8,       /* 复合号码本读取结果回复       */
    SI_PB_EVENT_SADD_CNF = 9,        /* 复合号码本添加结果回复       */
    SI_PB_EVENT_SMODIFY_CNF = 10,    /* 复合号码本修改结果回复       */
    SI_PB_EVENT_FDNCHECK_CNF = 11,   /* FDN号码匹配结果回复          */
    SI_PB_EVENT_ECALLQUERY_CNF = 12, /* ECALL号码查询结果回复        */
    SI_PB_EVENT_ECALLINIT_IND = 13,  /* ECALL号码初始化完成上报      */
    SI_PB_EVENT_BUTT
};

typedef VOS_UINT32 SI_PB_CnfUint32;

/*
 * 功能说明: PB模块存储器类型
 */
enum SI_PB_Storage {
    SI_PB_STORAGE_UNSPECIFIED = 0, /* 存储器类型不指定，根据设置确定介质类型 */
    SI_PB_STORAGE_SM = 1,          /* 存储器类型为ADN */
    SI_PB_STORAGE_ME = 2,          /* 存储器类型为NV */
    SI_PB_STORAGE_BOTH = 3,        /* 存储器类型为ADN和NV */
    SI_PB_STORAGE_ON = 4,          /* 存储器类型为MSISDN */
    SI_PB_STORAGE_FD = 5,          /* 存储器类型为FDN */
    SI_PB_STORAGE_EC = 6,          /* 存储器类型为ECC */
    SI_PB_STORAGE_BD = 7,          /* 存储器类型为BDN */
    SI_PB_STORAGE_SD = 8,          /* 存储器类型为SDN */
    SI_PB_STORAGE_BUTT
};

typedef VOS_UINT32 SI_PB_StorageUint32;

/* 电话本号码类型枚举 */
enum PB_NumberType {
    PB_NUMBER_TYPE_NORMAL = 129,        /* 普通号码类型 */
    PB_NUMBER_TYPE_INTERNATIONAL = 145, /* 国际号码类型 */
    PB_NUMBER_TYPE_BUTT
};

/* 仅数据卡产品支持电话本查询功能 */
typedef struct {
    SI_UINT16 inUsedNum;    /* 已使用记录数目 */
    SI_UINT16 totalNum;     /* 总记录数目 */
    SI_UINT16 textLen;      /* 支持的text的最大长度 */
    SI_UINT16 numLen;       /* 支持的number的最大长度 */
    SI_UINT16 emailTextLen; /* Emai长度 */
    SI_UINT16 anrNumberLen; /* ANR号码长度 */
} SI_PB_EventQueryCnf;

typedef struct {
    SI_UINT16 adnRecordNum;    /* ADN记录总数 */
    SI_UINT16 adnTextLen;      /* ADN姓名字段长度 */
    SI_UINT16 adnNumberLen;    /* ADN号码长度 */
    SI_UINT16 fdnRecordNum;    /* FDN记录总数 */
    SI_UINT16 fdnTextLen;      /* FDN姓名字段长度 */
    SI_UINT16 fdnNumberLen;    /* FDN号码长度 */
    SI_UINT16 bdnRecordNum;    /* BDN记录总数 */
    SI_UINT16 bdnTextLen;      /* BDN姓名字段长度 */
    SI_UINT16 bdnNumberLen;    /* BDN号码长度 */
    SI_UINT16 msisdnRecordNum; /* MSISDN记录总数 */
    SI_UINT16 msisdnTextLen;   /* MSISDN姓名字段长度 */
    SI_UINT16 msisdnNumberLen; /* MSISDN号码长度 */
    SI_UINT16 emailTextLen;    /* EMAIL长度 */
    SI_UINT16 anrNumberLen;    /* ANR号码长度 */
    SI_UINT16 fdnState;        /* FDN使能状态 */
    SI_UINT16 bdnState;        /* BDN使能状态 */
    SI_UINT16 cardType;        /* 卡类型：0 SIM卡　1 USIM卡 */
} SI_PB_EventInfoInd;

typedef struct {
    SI_UINT16 inUsedNum;    /* 已使用记录数目 */
    SI_UINT16 totalNum;     /* 总记录数目 */
    SI_UINT16 textLen;      /* 支持的text的最大长度 */
    SI_UINT16 numLen;       /* 支持的number的最大长度 */
    SI_UINT16 emailTextLen; /* Emai长度 */
    SI_UINT16 anrNumberLen; /* ANR号码长度 */
} SI_PB_EventSetCnf;

typedef struct {
    SI_UINT8 numberType;                         /* 号码类型，145或129 */
    SI_UINT8 numberLength;                       /* 号码长度，是不包括NumberType的长度 */
    SI_UINT8 number[SI_PB_PHONENUM_MAX_LEN + 2]; /* 号码，采用ASCII的编码方式 */
} SI_PB_AdditionNum;

typedef struct {
    SI_UINT32 emailLen;                  /* Email长度 */
    SI_UINT8 email[SI_PB_EMAIL_MAX_LEN]; /* Email，采用ASCII的编码方式 */
} SI_PB_Email;

typedef struct {
    SI_UINT16 index;                           /* 记录的index */
    SI_UINT8 alphaTag[SI_PB_ALPHATAG_MAX_LEN]; /* 记录的姓名字段包含编码类型 */
    SI_UINT8 alphaTagLength;                   /* 名字长度, 字节数，如果是UNICODE编码包含编码类型指示字节 */
    SI_UINT8 numberType;                       /* 号码类型，145或129 */
    SI_UINT8 numberLength;                     /* 号码长度，是不包括NumberType的长度 */
    SI_UINT8 validFlag;                        /* 记录有效标志 */
    SI_UINT8 number[SI_PB_PHONENUM_MAX_LEN];   /* 号码1，采用ASCII的编码方式 */
    SI_PB_AlphaTagTypeUint32 alphaTagType;     /* 编码类型 */
    SI_PB_AdditionNum additionNumber[3];
    SI_PB_Email email;
} SI_PB_Record;

typedef struct {
    SI_UINT16 recordNum; /* 记录的数目 */
    SI_UINT16 rsv;
    SI_PB_Record pbRecord; /* 第一个记录项 */
} SI_PB_EventReadCnf;

typedef struct {
    SI_UINT16 recordNum; /* 记录的数目 */
    SI_UINT16 rsv;
    SI_PB_Record pbRecord; /* 第一个记录项 */
} SI_PB_EventSearchCnf;

typedef struct {
    SI_UINT16 index; /* 返回添加的记录索引号 */
} SI_PB_EventAddCnf;

typedef struct {
    SI_UINT16 clientId;
    SI_UINT8 opId;
    SI_UINT8 reserve;
    SI_UINT32 pbEventType;
    SI_UINT32 pbError;
    SI_PB_StorateTypeUint32 storage;
    SI_UINT32 pbLastTag;
    union {
        SI_PB_EventQueryCnf pbQueryCnf;
        SI_PB_EventInfoInd pbInfoInd;
        SI_PB_EventSetCnf pbSetCnf;
        SI_PB_EventReadCnf pbReadCnf;
        SI_PB_EventAddCnf pbAddCnf;
        SI_PB_EventSearchCnf pbSearchCnf;
    } pbEvent;
} SI_PB_EventInfo;

typedef struct {
    SI_BOOL isESCExist;             /* ESC字节是否存在标志 */
    SI_UINT8 eccCode[USIM_ECC_LEN]; /* Emergency Call Code */
    SI_UINT8 esc;                   /* ESC字段值 */
    SI_UINT8 len;                   /* ESC字符长度 */
    SI_UINT8 alphaIdentifier[253];  /* ESC字符内容 */
    SI_UINT8 reserved[2];
} SI_PB_EccRecord;

typedef struct {
    SI_BOOL eccExists;   /* ECC文件是否存在 */
    SI_UINT32 reocrdNum; /* ECC中紧急呼叫号码的个数 */
    SI_PB_EccRecord eccRecord[USIM_MAX_ECC_RECORDS];
} SI_PB_EccData;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
} PBMsgBlock;

/*
 * 功能说明: FDN号码结构
 */
typedef struct {
    VOS_UINT32 num1Len;
    VOS_UINT8 num1[SI_PB_NUMBER_LEN];
    VOS_UINT32 num2Len;
    VOS_UINT8 num2[SI_PB_NUMBER_LEN];
} SI_PB_FdnNum;

/*
 * 功能说明: FDN号码匹配请求消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    USIMM_CardAppUint32 appType;
    VOS_UINT32 sendPara;
    VOS_UINT32 contextIndex;
    SI_PB_FdnNum fdnNum; /* FDN号码结构 */
} SI_PB_FdnCheckReq;

/*
 * 功能说明: FDN检测结果的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;           /* 消息名 */
    USIMM_CardAppUint32 appType;  /* APP类型 */
    VOS_UINT32 sendPara;          /* 透传参数 */
    VOS_UINT32 contextIndex;      /* index值 */
    PB_FdnCheckRsltUint32 result; /* 验证结果 */
} PB_FdnCheckCnf;

/*
 * 功能说明: Ecall读取FDN或SDN记录的请求
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;          /* 消息名 */
    USIMM_CardAppUint32 appType; /* APP类型 */
    SI_PB_StorageUint32 storage; /* 存储器类型，只能是SI_PB_STORAGE_FD或SI_PB_STORAGE_SD */
    VOS_UINT8 listLen;           /* 读取记录索引的长度 */
    VOS_UINT8 list[3];           /* 读取记录索引的内容 */
} SI_PB_EcallQryReq;

/*
 * 功能说明: ecall test/recfg number信息结构体
 * 1. 日    期: 2014年04月25日
 */
typedef struct {
    VOS_UINT8 ton;    /* 号码类型 */
    VOS_UINT8 length; /* BCD码的长度 */
    VOS_UINT8 index;  /* 从1开始 */
    VOS_UINT8 rsv[1];
    VOS_UINT8 callNumber[SI_PB_PHONENUM_MAX_LEN / 2];
} SI_PB_EcallNum;

/*
 * 功能说明: 查询ECALL的号码回复
 * 1. 日    期: 2014年04月25日
 */

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;          /* 消息名 */
    VOS_UINT32 rslt;             /* VOS_OK: 获取成功;VOS_ERR:获取失败(比如输入参数非法或读取失败) */
    USIMM_CardAppUint32 appType; /* APP类型 */
    SI_PB_StorageUint32 storage; /* 电话本类型,此时会指定SDN或FDN */
    VOS_UINT32 recordNum;        /* 读取电话本记录的总记录数 */
    SI_PB_EcallNum eCallNumber[2];
} SI_PB_EcallQryCnf;

/*
 * 功能说明: ECALL号码初始化指示消息
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;          /* 消息名 */
    USIMM_CardAppUint32 appType; /* APP类型 */
    VOS_UINT16 fdnRecordNum;     /* FDN总记录数 */
    VOS_UINT16 sdnRecordNum;     /* SDN总记录数 */
} SI_PB_EcallInitInd;

extern SI_UINT32 SI_PB_Read(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 index1,
    SI_UINT16 index2);

extern SI_UINT32 SI_PB_Set(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage);

extern SI_UINT32 SI_PB_Modify(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_SModify(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage,
    SI_PB_Record *record);

extern SI_UINT32 SI_PB_SRead(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 index1,
    SI_UINT16 index2);

extern SI_UINT32 SI_PB_Delete(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 pbIndex);

extern SI_UINT32 SI_PB_Add(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_SAdd(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_Search(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT8 length,
    SI_UINT8 *content);

extern SI_UINT32 SI_PB_Query(SI_UINT16 clientId, SI_UINT8 opId);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
extern VOS_UINT32 SI_PB_GetXeccNumber(SI_PIH_CardSlotUint32 slotId, SI_PB_EccData *eccData);
#endif

extern VOS_UINT32 WuepsPBPidInit(enum VOS_InitPhaseDefine initPhrase);

extern VOS_UINT32 I1_WuepsPBPidInit(enum VOS_InitPhaseDefine initPhrase);

extern VOS_UINT32 I2_WuepsPBPidInit(enum VOS_InitPhaseDefine initPhrase);

extern VOS_VOID SI_PB_InitContent(VOS_UINT8 len, VOS_UINT8 value, VOS_UINT8 *mem);

extern VOS_VOID I0_SI_PB_PidMsgProc(struct MsgCB *pbMsg);

extern VOS_VOID I1_SI_PB_PidMsgProc(struct MsgCB *pbMsg);

extern VOS_VOID I2_SI_PB_PidMsgProc(struct MsgCB *pbMsg);

extern VOS_UINT32 SI_PB_GetStorateType(VOS_VOID);

extern VOS_UINT32 SI_PB_GetSPBFlag(VOS_VOID);

#else /* (FEATURE_PHONE_USIM == FEATURE_ON) */

#define SI_PB_PHONENUM_MAX_LEN 40  /* 返回结构中号码的最大长度，是指字节数 */
#define SI_PB_ALPHATAG_MAX_LEN 242 /* 返回结构中名字的最大长度，是指字节数 */

#define SI_PB_EMAIL_MAX_LEN 64 /* 返回结构中邮件的最大长度，是指字节数 */

#define SI_PB_NUMBER_LEN (20) /* 协议栈下发的BCD号码长度 */

/* 哥伦比亚定制超过5个，暂时修改为支持16条 */
#define USIM_MAX_ECC_RECORDS 16
#define USIM_ECC_LEN 3

typedef SI_UINT32  SI_PB_AlphaTagTypeUint32;
#define SI_PB_ALPHATAG_TYPE_UCS2_80     0x80            /* 名字的编码类型，16-bit universal multiple-octet coded character set (ISO/IEC10646 [32]) */
#define SI_PB_ALPHATAG_TYPE_UCS2_81     0x81
#define SI_PB_ALPHATAG_TYPE_UCS2_82     0x82
#define SI_PB_ALPHATAG_TYPE_GSM         0x00            /* 名字的编码类型，GSM 7 bit default alphabet (3GPP TS 23.038) */
#define SI_PB_ALPHATAG_TYPE_UCS2        0x80

#define SI_PB_CONTENT_VALID 0x01
#define SI_PB_CONTENT_INVALID 0x00

#define SI_PB_LAST_TAG_TRUE 1

#define SI_PB_ALPHATAG_TRANSFER_TAG 0x1B

enum PB_FdnCheckRslt {
    PB_FDN_CHECK_SUCC          = 0,
    PB_FDN_CHECK_NUM1_FAIL     = 1,
    PB_FDN_CHECK_NUM2_FAIL     = 2,
    PB_FDN_CHECK_BOTH_NUM_FAIL = 3,
    PB_FDN_CHECK_RSLT_BUTT
};
typedef VOS_UINT32 PB_FdnCheckRsltUint32;

/*
 * 功能说明: PB模块请求消息类型
 */
enum SI_PB_Req {
    SI_PB_READ_REQ      = 0,  /* 号码本读取请求           */
    SI_PB_SET_REQ       = 1,  /* 号码本设置请求           */
    SI_PB_MODIFY_REQ    = 2,  /* 号码本修改请求           */
    SI_PB_DELETE_REQ    = 3,  /* 号码本删除请求           */
    SI_PB_QUERY_REQ     = 4,  /* 号码本查询请求           */
    SI_PB_ADD_REQ       = 5,  /* 号码本添加请求           */
    SI_PB_SEARCH_REQ    = 6,  /* 号码本查找请求           */
    SI_PB_SREAD_REQ     = 7,  /* 复合号码本读取请求       */
    SI_PB_SMODIFY_REQ   = 8,  /* 复合号码本修改请求       */
    SI_PB_SADD_REQ      = 9,  /* 复合号码本添加请求       */
    SI_PB_UPDATE_AGOBAL = 10, /* 更新Acpu的全局变量内容   */
    SI_PB_UPDATE_CURPB  = 11, /* 更新当前电话本设置       */
    SI_PB_FDN_CHECK_REQ = 12, /* FDN号码匹配请求          */
    SI_PB_ECALL_QRY_REQ = 13, /* ECALL号码匹配请求        */

    SI_PB_REQ_BUTT
};
typedef SI_UINT32 SI_PB_ReqUint32;

/*
 * 功能说明: PB模块回复和上报消息类型
 */
enum SI_PB_Cnf {
    SI_PB_EVENT_INFO_IND       = 0,  /* 号码本信息事件上报           */
    SI_PB_EVENT_READ_CNF       = 1,  /* 号码本读取结果回复           */
    SI_PB_EVENT_QUERY_CNF      = 2,  /* 号码本查询结果回复           */
    SI_PB_EVENT_SET_CNF        = 3,  /* 号码本设置结果回复           */
    SI_PB_EVENT_ADD_CNF        = 4,  /* 号码本添加结果回复           */
    SI_PB_EVENT_MODIFY_CNF     = 5,  /* 号码本修改结果回复           */
    SI_PB_EVENT_DELETE_CNF     = 6,  /* 号码本删除结果回复           */
    SI_PB_EVENT_SEARCH_CNF     = 7,  /* 号码本搜索结果回复           */
    SI_PB_EVENT_SREAD_CNF      = 8,  /* 复合号码本读取结果回复       */
    SI_PB_EVENT_SADD_CNF       = 9,  /* 复合号码本添加结果回复       */
    SI_PB_EVENT_SMODIFY_CNF    = 10, /* 复合号码本修改结果回复       */
    SI_PB_EVENT_FDNCHECK_CNF   = 11, /* FDN号码匹配结果回复          */
    SI_PB_EVENT_ECALLQUERY_CNF = 12, /* ECALL号码查询结果回复        */
    SI_PB_EVENT_ECALLINIT_IND  = 13, /* ECALL号码初始化完成上报      */
    SI_PB_EVENT_BUTT
};

typedef VOS_UINT32 SI_PB_CnfUint32;

/*
 * 功能说明: PB模块存储器类型
 */
enum SI_PB_Storage {
    SI_PB_STORAGE_UNSPECIFIED = 0, /* 存储器类型不指定，根据设置确定介质类型 */
    SI_PB_STORAGE_SM          = 1, /* 存储器类型为ADN */
    SI_PB_STORAGE_ME          = 2, /* 存储器类型为NV */
    SI_PB_STORAGE_BOTH        = 3, /* 存储器类型为ADN和NV */
    SI_PB_STORAGE_ON          = 4, /* 存储器类型为MSISDN */
    SI_PB_STORAGE_FD          = 5, /* 存储器类型为FDN */
    SI_PB_STORAGE_EC          = 6, /* 存储器类型为ECC */
    SI_PB_STORAGE_BD          = 7, /* 存储器类型为BDN */
    SI_PB_STORAGE_SD          = 8, /* 存储器类型为SDN */
    SI_PB_STORAGE_BUTT
};

typedef VOS_UINT32 SI_PB_StorageUint32;

/* 电话本号码类型枚举 */
enum PB_NumberType {
    PB_NUMBER_TYPE_NORMAL        = 129, /* 普通号码类型 */
    PB_NUMBER_TYPE_INTERNATIONAL = 145, /* 国际号码类型 */
    PB_NUMBER_TYPE_BUTT
};

/* 仅数据卡产品支持电话本查询功能 */
typedef struct {
    SI_UINT16 inUsedNum;    /* 已使用记录数目 */
    SI_UINT16 totalNum;     /* 总记录数目 */
    SI_UINT16 textLen;      /* 支持的text的最大长度 */
    SI_UINT16 numLen;       /* 支持的number的最大长度 */
    SI_UINT16 emailTextLen; /* Emai长度 */
    SI_UINT16 anrNumberLen; /* ANR号码长度 */
} SI_PB_EventQueryCnf;

typedef struct {
    SI_UINT16 adnRecordNum;    /* ADN记录总数 */
    SI_UINT16 adnTextLen;      /* ADN姓名字段长度 */
    SI_UINT16 adnNumberLen;    /* ADN号码长度 */
    SI_UINT16 fdnRecordNum;    /* FDN记录总数 */
    SI_UINT16 fdnTextLen;      /* FDN姓名字段长度 */
    SI_UINT16 fdnNumberLen;    /* FDN号码长度 */
    SI_UINT16 bdnRecordNum;    /* BDN记录总数 */
    SI_UINT16 bdnTextLen;      /* BDN姓名字段长度 */
    SI_UINT16 bdnNumberLen;    /* BDN号码长度 */
    SI_UINT16 msisdnRecordNum; /* MSISDN记录总数 */
    SI_UINT16 msisdnTextLen;   /* MSISDN姓名字段长度 */
    SI_UINT16 msisdnNumberLen; /* MSISDN号码长度 */
    SI_UINT16 emailTextLen;    /* EMAIL长度 */
    SI_UINT16 anrNumberLen;    /* ANR号码长度 */
    SI_UINT16 fdnState;        /* FDN使能状态 */
    SI_UINT16 bdnState;        /* BDN使能状态 */
    SI_UINT16 cardType;        /* 卡类型：0 SIM卡　1 USIM卡 */
} SI_PB_EventInfoInd;

typedef struct {
    SI_UINT16 inUsedNum;    /* 已使用记录数目 */
    SI_UINT16 totalNum;     /* 总记录数目 */
    SI_UINT16 textLen;      /* 支持的text的最大长度 */
    SI_UINT16 numLen;       /* 支持的number的最大长度 */
    SI_UINT16 emailTextLen; /* Emai长度 */
    SI_UINT16 anrNumberLen; /* ANR号码长度 */
} SI_PB_EventSetCnf;

typedef struct {
    SI_UINT8 numberType;                         /* 号码类型，145或129 */
    SI_UINT8 numberLength;                       /* 号码长度，是不包括NumberType的长度 */
    SI_UINT8 number[SI_PB_PHONENUM_MAX_LEN + 2]; /* 号码，采用ASCII的编码方式 */
} SI_PB_AdditionNum;

typedef struct {
    SI_UINT32 emailLen;                   /* Email长度 */
    SI_UINT8  email[SI_PB_EMAIL_MAX_LEN]; /* Email，采用ASCII的编码方式 */
} SI_PB_Email;

typedef struct {
    SI_UINT16           index;                            /* 记录的index */
    SI_UINT8            alphaTag[SI_PB_ALPHATAG_MAX_LEN]; /* 记录的姓名字段包含编码类型 */
    SI_UINT8            alphaTagLength;                   /* 名字长度, 字节数，如果是UNICODE编码包含编码类型指示字节 */
    SI_UINT8            numberType;                       /* 号码类型，145或129 */
    SI_UINT8            numberLength;                     /* 号码长度，是不包括NumberType的长度 */
    SI_UINT8            validFlag;                        /* 记录有效标志 */
    SI_UINT8            number[SI_PB_PHONENUM_MAX_LEN];   /* 号码1，采用ASCII的编码方式 */
    SI_PB_AlphaTagTypeUint32 alphaTagType;                     /* 编码类型 */
    SI_PB_AdditionNum   additionNumber[3];
    SI_PB_Email         email;
} SI_PB_Record;

typedef struct {
    SI_UINT16    recordNum; /* 记录的数目 */
    SI_UINT16    rsv;
    SI_PB_Record pbRecord; /* 第一个记录项 */
} SI_PB_EventReadCnf;

typedef struct {
    SI_UINT16    recordNum; /* 记录的数目 */
    SI_UINT16    rsv;
    SI_PB_Record pbRecord; /* 第一个记录项 */
} SI_PB_EventSearchCnf;

typedef struct {
    SI_UINT16 index; /* 返回添加的记录索引号 */
} SI_PB_EventAddCnf;

typedef struct {
    SI_UINT16          clientId;
    SI_UINT8           opId;
    SI_UINT8           reserve;
    SI_UINT32          pbEventType;
    SI_UINT32          pbError;
    SI_PB_StorateTypeUint32 storage;
    SI_UINT32          pbLastTag;
    union {
        SI_PB_EventQueryCnf  pbQueryCnf;
        SI_PB_EventInfoInd   pbInfoInd;
        SI_PB_EventSetCnf    pbSetCnf;
        SI_PB_EventReadCnf   pbReadCnf;
        SI_PB_EventAddCnf    pbAddCnf;
        SI_PB_EventSearchCnf pbSearchCnf;
    } pbEvent;
} SI_PB_EventInfo;

typedef struct {
    SI_BOOL  bESC;                  /* ESC字节是否存在标志 */
    SI_UINT8 eccCode[USIM_ECC_LEN]; /* Emergency Call Code */
    SI_UINT8 esc;                   /* ESC字段值 */
    SI_UINT8 len;                   /* ESC字符长度 */
    SI_UINT8 alphaIdentifier[253];  /* ESC字符内容 */
    SI_UINT8 reserved[2];
} SI_PB_EccRecord;

typedef struct {
    SI_BOOL         eccExists; /* ECC文件是否存在 */
    SI_UINT32       reocrdNum;  /* ECC中紧急呼叫号码的个数 */
    SI_PB_EccRecord eccRecord[USIM_MAX_ECC_RECORDS];
} SI_PB_EccData;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
} PBMsgBlock;

/*
 * 功能说明: FDN号码结构
 */
typedef struct {
    VOS_UINT32 num1Len;
    VOS_UINT8  num1[SI_PB_NUMBER_LEN];
    VOS_UINT32 num2Len;
    VOS_UINT8  num2[SI_PB_NUMBER_LEN];
} SI_PB_FdnNum;

/*
 * 功能说明: FDN号码匹配请求消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName; /* 消息名 */
    USIMM_CardAppUint32 appType;
    VOS_UINT32          sendPara;
    VOS_UINT32          contextIndex;
    SI_PB_FdnNum        fdnNum; /* FDN号码结构 */
} SI_PB_FdnCheckReq;

/*
 * 功能说明: FDN检测结果的消息结构
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32            msgName;      /* 消息名 */
    USIMM_CardAppUint32   appType;      /* APP类型 */
    VOS_UINT32            sendPara;     /* 透传参数 */
    VOS_UINT32            contextIndex; /* index值 */
    PB_FdnCheckRsltUint32 result;       /* 验证结果 */
} PB_FdnCheckCnf;

/*
 * 功能说明: Ecall读取FDN或SDN记录的请求
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName; /* 消息名 */
    USIMM_CardAppUint32 appType; /* APP类型 */
    SI_PB_StorageUint32 storage; /* 存储器类型，只能是SI_PB_STORAGE_FD或SI_PB_STORAGE_SD */
    VOS_UINT8           listLen; /* 读取记录索引的长度 */
    VOS_UINT8           list[3]; /* 读取记录索引的内容 */
} SI_PB_EcallQryReq;

/*
 * 功能说明: ecall test/recfg number信息结构体
 * 1. 日    期: 2014年04月25日
 */
typedef struct {
    VOS_UINT8 ton;    /* 号码类型 */
    VOS_UINT8 length; /* BCD码的长度 */
    VOS_UINT8 index;  /* 从1开始 */
    VOS_UINT8 rsv[1];
    VOS_UINT8 callNumber[SI_PB_PHONENUM_MAX_LEN / 2];
} SI_PB_EcallNum;

/*
 * 功能说明: 查询ECALL的号码回复
 * 1. 日    期: 2014年04月25日
 */

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName;   /* 消息名 */
    VOS_UINT32          rslt;      /* VOS_OK: 获取成功;VOS_ERR:获取失败(比如输入参数非法或读取失败) */
    USIMM_CardAppUint32 appType;   /* APP类型 */
    SI_PB_StorageUint32 storage;   /* 电话本类型,此时会指定SDN或FDN */
    VOS_UINT32          recordNum; /* 读取电话本记录的总记录数 */
    SI_PB_EcallNum      eCallNumber[2];
} SI_PB_EcallQryCnf;

/*
 * 功能说明: ECALL号码初始化指示消息
 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32          msgName;      /* 消息名 */
    USIMM_CardAppUint32 appType;      /* APP类型 */
    VOS_UINT16          fdnRecordNum; /* FDN总记录数 */
    VOS_UINT16          sdnRecordNum; /* SDN总记录数 */
} SI_PB_EcallInitInd;

extern SI_UINT32 SI_PB_Read(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 index1,
                            SI_UINT16 index2);

extern SI_UINT32 SI_PB_Set(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage);

extern SI_UINT32 SI_PB_Modify(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_SModify(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_SRead(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 index1,
                             SI_UINT16 index2);

extern SI_UINT32 SI_PB_Delete(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT16 pbIndex);

extern SI_UINT32 SI_PB_Add(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_SAdd(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_PB_Record *record);

extern SI_UINT32 SI_PB_Search(SI_UINT16 clientId, SI_UINT8 opId, SI_PB_StorateTypeUint32 storage, SI_UINT8 length,
                              SI_UINT8 *content);

extern SI_UINT32  SI_PB_Query(SI_UINT16 clientId, SI_UINT8 opId);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
extern VOS_UINT32 SI_PB_GetXeccNumber(SI_PB_EccData *eccData);
#endif

extern VOS_UINT32 WuepsPBPidInit(enum VOS_InitPhaseDefine initPhrase);

extern VOS_VOID SI_PB_InitContent(VOS_UINT8 len, VOS_UINT8 value, VOS_UINT8 *mem);

extern VOS_VOID I0_SI_PB_PidMsgProc(struct MsgCB *pbMsg);

extern VOS_UINT32 SI_PB_GetStorateType(VOS_VOID);

extern VOS_UINT32 SI_PB_GetSPBFlag(VOS_VOID);

#endif /* (FEATURE_PHONE_USIM == FEATURE_ON) */

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
