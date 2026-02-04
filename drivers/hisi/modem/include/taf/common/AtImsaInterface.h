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

#ifndef  AT_IMSA_INTERFACE_H
#define  AT_IMSA_INTERFACE_H

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"

#include "TafApsApi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

#define AT_IMSA_IMPU_MAX_LENGTH     (128)

/* equals IMSA_MAX_CALL_NUMBER_LENGTH */
#define AT_IMSA_CALL_ASCII_NUM_MAX_LENGTH     (40)
/*****************************************************************************
  2 枚举定义
*****************************************************************************/
/*****************************************************************************
枚举名    : AT_IMSA_MSG_TYPE_ENUM
结构说明  : AT 和IMSA的之间的消息

  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 定义AT和IMSA之间的接口消息
*****************************************************************************/
enum AT_IMSA_MSG_TYPE_ENUM
{
    /* AT->IMSA */
    ID_AT_IMSA_CIREG_SET_REQ                = 0x0001,                           /* _H2ASN_MsgChoice AT_IMSA_CIREG_SET_REQ_STRU */
    ID_AT_IMSA_CIREG_QRY_REQ,                                                   /* _H2ASN_MsgChoice AT_IMSA_CIREG_QRY_REQ_STRU */
    ID_AT_IMSA_CIREP_SET_REQ,                                                   /* _H2ASN_MsgChoice AT_IMSA_CIREP_SET_REQ_STRU */
    ID_AT_IMSA_CIREP_QRY_REQ,                                                   /* _H2ASN_MsgChoice AT_IMSA_CIREP_QRY_REQ_STRU */
    ID_AT_IMSA_VOLTEIMPU_QRY_REQ,                                               /* _H2ASN_MsgChoice AT_IMSA_VOLTEIMPU_QRY_REQ_STRU */



    ID_AT_IMSA_IMS_REG_DOMAIN_QRY_REQ,                                          /* _H2ASN_MsgChoice AT_IMSA_IMS_REG_DOMAIN_QRY_REQ_STRU */
    ID_AT_IMSA_IMS_CTRL_MSG,                                                    /* _H2ASN_MsgChoice AT_IMSA_IMS_CTRL_MSG_STRU */

    ID_AT_IMSA_CALL_ENCRYPT_SET_REQ,                                            /* _H2ASN_MsgChoice AT_IMSA_CALL_ENCRYPT_SET_REQ_STRU */

    ID_AT_IMSA_ROAMING_IMS_QRY_REQ,                                             /* _H2ASN_MsgChoice AT_IMSA_ROAMING_IMS_QRY_REQ_STRU */


    /* IMSA->AT */
    ID_IMSA_AT_CIREG_SET_CNF                = 0x1001,                           /* _H2ASN_MsgChoice IMSA_AT_CIREG_SET_CNF_STRU */
    ID_IMSA_AT_CIREG_QRY_CNF,                                                   /* _H2ASN_MsgChoice IMSA_AT_CIREG_QRY_CNF_STRU */
    ID_IMSA_AT_CIREP_SET_CNF,                                                   /* _H2ASN_MsgChoice IMSA_AT_CIREP_SET_CNF_STRU */
    ID_IMSA_AT_CIREP_QRY_CNF,                                                   /* _H2ASN_MsgChoice IMSA_AT_CIREP_QRY_CNF_STRU */

    ID_IMSA_AT_VOLTEIMPU_QRY_CNF,                                               /* _H2ASN_MsgChoice IMSA_AT_VOLTEIMPU_QRY_CNF_STRU */




    ID_IMSA_AT_CIREGU_IND,                                                      /* _H2ASN_MsgChoice IMSA_AT_CIREGU_IND_STRU */
    ID_IMSA_AT_CIREPH_IND,                                                      /* _H2ASN_MsgChoice IMSA_AT_CIREPH_IND_STRU */
    ID_IMSA_AT_CIREPI_IND,                                                      /* _H2ASN_MsgChoice IMSA_AT_CIREPI_IND_STRU */

    ID_IMSA_AT_VT_PDP_ACTIVATE_IND,                                             /* _H2ASN_MsgChoice IMSA_AT_VT_PDP_ACTIVATE_IND_STRU */
    ID_IMSA_AT_VT_PDP_DEACTIVATE_IND,                                           /* _H2ASN_MsgChoice IMSA_AT_VT_PDP_DEACTIVATE_IND_STRU */

    ID_IMSA_AT_MT_STATES_IND,                                                   /* _H2ASN_MsgChoice IMSA_AT_MT_STATES_IND_STRU */

    ID_IMSA_AT_IMS_REG_DOMAIN_QRY_CNF,                                          /* _H2ASN_MsgChoice IMSA_AT_IMS_REG_DOMAIN_QRY_CNF_STRU */
    ID_IMSA_AT_IMS_CTRL_MSG,                                                    /* _H2ASN_MsgChoice IMSA_AT_IMS_CTRL_MSG_STRU */

    ID_IMSA_AT_CALL_ENCRYPT_SET_CNF,                                            /* _H2ASN_MsgChoice IMSA_AT_CALL_ENCRYPT_SET_CNF_STRU */

    ID_IMSA_AT_ROAMING_IMS_QRY_CNF,                                             /* _H2ASN_MsgChoice IMSA_AT_ROAMING_IMS_QRY_CNF_STRU */

    ID_IMSA_AT_IMS_RAT_HANDOVER_IND,                                            /* _H2ASN_MsgChoice IMSA_AT_IMS_RAT_HANDOVER_IND_STRU */
    ID_IMSA_AT_IMS_SRV_STATUS_UPDATE_IND,                                        /* _H2ASN_MsgChoice IMSA_AT_IMS_SRV_STATUS_UPDATE_IND_STRU */

    ID_AT_IMSA_MSG_BUTT
};
typedef  VOS_UINT32  AT_IMSA_MSG_TYPE_ENUM_UINT32;


/*****************************************************************************
枚举名    : AT_IMSA_IMS_REG_STATE_REPORT_ENUM
结构说明  : IMS注册状态上报控制

  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增枚举
*****************************************************************************/
enum AT_IMSA_IMS_REG_STATE_REPORT_ENUM
{
    AT_IMSA_IMS_REG_STATE_DISABLE_REPORT        = 0,
    AT_IMSA_IMS_REG_STATE_ENABLE_REPORT,
    AT_IMSA_IMS_REG_STATE_ENABLE_EXTENDED_REPROT,

    AT_IMSA_IMS_REG_STATE_REPROT_BUTT
};
typedef  VOS_UINT32  AT_IMSA_IMS_REG_STATE_REPORT_ENUM_UINT32;

/*****************************************************************************
枚举名    : AT_IMSA_CCWAI_MODE_ENUM
结构说明  : 网络是否支持IMSVOPS

  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增枚举
*****************************************************************************/
enum AT_IMSA_CCWAI_MODE_ENUM
{
    AT_IMSA_CCWAI_MODE_DISABLE               = 0,
    AT_IMSA_CCWAI_MODE_ENABLE,

    AT_IMSA_CCWAI_MODE_BUTT
};
typedef  VOS_UINT8 AT_IMSA_CCWAI_MODE_ENUM_UINT8;


/*****************************************************************************
枚举名    : AT_IMSA_IMSVOPS_CAPABILITY_ENUM
结构说明  : 网络是否支持IMSVOPS

  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增枚举
*****************************************************************************/
enum AT_IMSA_IMSVOPS_CAPABILITY_ENUM
{
    AT_IMSA_NW_NOT_SUPORT_IMSVOPS               = 0,
    AT_IMSA_NW_SUPORT_IMSVOPS,

    AT_IMSA_IMSVOPS_CAPABILITY_BUTT
};
typedef  VOS_UINT32  AT_IMSA_IMSVOPS_CAPABILITY_ENUM_UINT32;


/*****************************************************************************
枚举名    : AT_IMSA_CIREP_REPORT_ENUM
结构说明  : 是否允许CIREPH和CIREPI主动上报

  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增枚举
*****************************************************************************/
enum AT_IMSA_CIREP_REPORT_ENUM
{
    AT_IMSA_CIREP_REPORT_DISENABLE      = 0,
    AT_IMSA_CIREP_REPORT_ENABLE,

    AT_IMSA_CIREP_REPORT_BUTT
};
typedef  VOS_UINT32  AT_IMSA_CIREP_REPORT_ENUM_UINT32;


/*****************************************************************************
枚举名    : AT_IMSA_SRVCC_HANDVOER_ENUM
结构说明  : SRVCC过程状态

  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增枚举
*****************************************************************************/
enum AT_IMSA_SRVCC_HANDVOER_ENUM
{
    AT_IMSA_SRVCC_HANDOVER_STARTED                = 0,
    AT_IMSA_SRVCC_HANDOVER_SUCCESS,
    AT_IMSA_SRVCC_HANDOVER_CANCEL,
    AT_IMSA_SRVCC_HANDOVER_FAILURE,

    AT_IMSA_SRVCC_HANDOVER_BUTT
};
typedef  VOS_UINT32  AT_IMSA_SRVCC_HANDVOER_ENUM_UINT32;

/*****************************************************************************
 枚举名    : IMSA_AT_IMS_RAT_TYPE_ENUM
 枚举说明  : IMS接入类型
 1.日    期   : 2015年10月07日
   作    者   : f00179208
   修改内容   : 创建
*****************************************************************************/
enum IMSA_AT_IMS_RAT_TYPE_ENUM
{
    IMSA_AT_IMS_RAT_TYPE_LTE            = 0x00,
    IMSA_AT_IMS_RAT_TYPE_WIFI           = 0x01,
    IMSA_AT_IMS_RAT_TYPE_BUTT
};
typedef VOS_UINT8 IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8;
/*****************************************************************************
枚举名    : AT_IMSA_ROAMING_IMS_SUPPORT_ENUM
结构说明  : 漫游状态下是否支持IMS服务

  1.日    期   : 2016年1月15日
    作    者   : w00316404
    修改内容   : 新增枚举
*****************************************************************************/
enum AT_IMSA_ROAMING_IMS_SUPPORT_ENUM
{
    AT_IMSA_ROAMING_IMS_SUPPORT         = 1,
    AT_IMSA_ROAMING_IMS_NOT_SUPPORT     = 2,

    AT_IMSA_ROAMING_IMS_BUTT
};
typedef  VOS_UINT32  AT_IMSA_ROAMING_IMS_SUPPORT_ENUM_UINT32;

/*****************************************************************************
 枚举名    : IMSA_AT_IMS_REG_DOMAIN_TYPE_ENUM
 枚举说明  : IMS注册域类型
 1.日    期   : 2015年10月07日
   作    者   : f00179208
   修改内容   : 创建
*****************************************************************************/
enum IMSA_AT_IMS_REG_DOMAIN_TYPE_ENUM
{
    IMSA_AT_IMS_REG_DOMAIN_TYPE_LTE     = 0x00,
    IMSA_AT_IMS_REG_DOMAIN_TYPE_WIFI    = 0x01,
    IMSA_AT_IMS_REG_DOMAIN_TYPE_UNKNOWN = 0x02,
    IMSA_AT_IMS_REG_DOMAIN_TYPE_BUTT
};
typedef VOS_UINT8 IMSA_AT_IMS_REG_DOMAIN_TYPE_ENUM_UINT8;


/*****************************************************************************
 枚举名    : IMSA_AT_IMS_RAT_HO_STATUS_ENUM
 枚举说明  : IMS注册域切换状态
 1.日    期   : 2016年04月07日
   作    者   : f00179208
   修改内容   : 创建
*****************************************************************************/
enum IMSA_AT_IMS_RAT_HO_STATUS_ENUM
{
    IMSA_AT_IMS_RAT_HO_STATUS_SUCCESS               = 0x00,     /* RAT handover was successful */
    IMSA_AT_IMS_RAT_HO_STATUS_FAILURE               = 0x01,     /* RAT handover was failed */
    IMSA_AT_IMS_RAT_HO_STATUS_NOT_TRIGGERED         = 0x02,     /* RAT handover could nut be triggered */
    IMSA_AT_IMS_RAT_HO_STATUS_BUTT
};
typedef VOS_UINT8 IMSA_AT_IMS_RAT_HO_STATUS_ENUM_UINT8;

/*****************************************************************************
 枚举名    : IMSA_AT_IMS_SERVICE_STATUS_ENUM
 枚举说明  : IMS的服务状态
 1.日    期   : 2016年04月07日
   作    者   : f00179208
   修改内容   : 创建
*****************************************************************************/
enum IMSA_AT_IMS_SERVICE_STATUS_ENUM
{
    IMSA_AT_IMS_SERVICE_STATUS_NO_SERVICE           = 0x00,     /* IMS is no service */
    IMSA_AT_IMS_SERVICE_STATUS_LIMITED_SERVICE      = 0x01,     /* IMS is in limited service */
    IMSA_AT_IMS_SERVICE_STATUS_FULL_SERVICE         = 0x02,     /* IMS is in full service */
    IMSA_AT_IMS_SERVICE_STATUS_BUTT
};
typedef VOS_UINT8 IMSA_AT_IMS_SERVICE_STATUS_ENUM_UINT8;

/*****************************************************************************
 枚举名    : IMSA_AT_HO_CAUSE_ENUM
 枚举说明  : IMS的HO状态
 1.日    期   : 2016年04月07日
   作    者   :  z00353461
   修改内容   : 创建
*****************************************************************************/

enum IMSA_AT_HO_CAUSE_ENUM
{
    IMSA_AT_HO_CAUSE_SUCCESS                    = 0,       /**< HO命令执行成功 */
    IMSA_AT_HO_CAUSE_FAIL_PARA_ERR              = 1,       /**< HO命令执行失败，由于参数错误 */
    IMSA_AT_HO_CAUSE_FAIL_CN_REJ                = 2,       /**< HO命令执行失败，由于被网侧拒绝 */
    IMSA_AT_HO_CAUSE_FAIL_TIMER_EXP             = 3,       /**< HO命令执行失败，由于定时器超时 */
    IMSA_AT_HO_CAUSE_FAIL_CONN_RELEASING        = 4,       /**< HO命令执行失败，由于正在释放连接 */
    IMSA_AT_HO_CAUSE_FAIL_PDP_ACTIVATE_LIMIT    = 5,       /**< HO命令执行失败，由于激活的承载数限制 */
    IMSA_AT_HO_CAUSE_FAIL_SAME_APN_OPERATING    = 6,       /**< HO命令执行失败，由于APS正在执行操作 */
    IMSA_AT_HO_CAUSE_FAIL_TEMP_FORBIDDEN        = 7,       /**< HO命令执行失败，收到临时被拒原因值或者网侧不响应 */
    IMSA_AT_HO_CAUSE_FAIL_PERM_FORBIDDEN        = 8,       /**< HO命令执行失败，收到永久被拒原因值 */
    IMSA_AT_HO_CAUSE_FAIL_WIFI_READY_IND_TIMEOUT    = 9,   /**< HO命令执行失败，由于WIFI下发送READY IND超时 */

    IMSA_AT_HO_CAUSE_FAIL_OHTERS                    = 11,    /**< HO命令执行失败，由于其他原因 */
	IMSA_AT_HO_CAUSE_FAIL_NO_DSDS_RESOURCE			= 12,    /**< HO命令执行失败，由于DSDS下申请无线资源失败 */

    IMSA_AT_HO_CAUSE_BUTT
};
typedef VOS_UINT32 IMSA_AT_HO_CAUSE_ENUM_UINT32;





/*****************************************************************************
  3 类型定义
*****************************************************************************/
/*****************************************************************************
结构名    : AT_IMSA_MSG_STRU
结构说明  : AT模块与IMSA模块间的消息结构

1.日    期  : 2013年09月12日
  作    者  : Y00213812
  修改内容  : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    VOS_UINT8                           aucContent[4];
} AT_IMSA_MSG_STRU;

/*****************************************************************************
结构名    : AT_IMSA_CIREG_QRY_REQ_STRU
结构说明  : ID_AT_IMSA_CIREG_QRY_REQ的消息结构

1.日    期  : 2013年09月12日
  作    者  : Y00213812
  修改内容  : 新增结构
*****************************************************************************/
typedef AT_IMSA_MSG_STRU AT_IMSA_CIREG_QRY_REQ_STRU;


/*****************************************************************************
结构名    : AT_IMSA_CIREP_QRY_REQ_STRU
结构说明  : ID_AT_IMSA_CIREP_QRY_REQ的消息结构

1.日    期  : 2013年09月12日
  作    者  : Y00213812
  修改内容  : 新增结构
*****************************************************************************/
typedef AT_IMSA_MSG_STRU AT_IMSA_CIREP_QRY_REQ_STRU;

/*****************************************************************************
结构名    : AT_IMSA_VOLTEIMPU_QRY_REQ_STRU
结构说明  : ID_AT_IMSA_VOLTEIMPU_QRY_REQ的消息结构

1.日    期  : 2013年12月05日
  作    者  : Y00213812
  修改内容  : 新增结构
*****************************************************************************/
typedef AT_IMSA_MSG_STRU AT_IMSA_VOLTEIMPU_QRY_REQ_STRU;


/*****************************************************************************
 结构名  : AT_IMSA_CCWAI_SET_REQ_STRU
 结构说明: ^CCWAI IMS  呼叫等待
 修改历史      :
  1.日    期   : 2014年3月11日
    作    者   : w00209181
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    AT_IMSA_CCWAI_MODE_ENUM_UINT8       enMode;
    VOS_UINT32                          ulSrvClass;
} AT_IMSA_CCWAI_SET_REQ_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_CCWAI_SET_CNF_STRU
 结构说明: ^CCWAI IMS  呼叫等待
 修改历史      :
  1.日    期   : 2014年3月11日
    作    者   : w00209181
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           ucReserved;
    VOS_UINT32                          ulResult;
} IMSA_AT_CCWAI_SET_CNF_STRU;


/*****************************************************************************
 结构名  : AT_IMSA_CALL_ENCRYPT_SET_REQ_STRU
 结构说明: ^CALLENCRYPT: 设置语音加密请求
 修改历史      :
  1.日    期   : 2015年10月14日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved1[1];
    VOS_UINT8                           ucEncrypt;                              /* 0:不加密，1:加密 */
    VOS_UINT8                           aucReserved2[3];
} AT_IMSA_CALL_ENCRYPT_SET_REQ_STRU;

/*****************************************************************************
 结构名  : AT_IMSA_CALL_ENCRYPT_SET_CNF_STRU
 结构说明: ^CALLENCRYPT: 设置语音加密回复
 修改历史      :
  1.日    期   : 2015年10月14日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           ucReserved;
    VOS_UINT32                          ulResult;                               /* 成功返回VOS_OK，失败返回VOS_ERR */
} IMSA_AT_CALL_ENCRYPT_SET_CNF_STRU;

/*****************************************************************************
 结构名  : AT_IMSA_CIREG_SET_REQ_STRU
 结构说明: +CIREG IMS注册状态主动上报控制设置

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    AT_IMSA_IMS_REG_STATE_REPORT_ENUM_UINT32    enCireg;
} AT_IMSA_CIREG_SET_REQ_STRU;


/*****************************************************************************
 结构名  : IMSA_AT_CIREG_SET_CNF_STRU
 结构说明: +CIREG IMS注册状态主动上报控制

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    VOS_UINT32                          ulResult;                               /* 成功返回VOS_OK，失败返回VOS_ERR */
} IMSA_AT_CIREG_SET_CNF_STRU;


/*****************************************************************************
 结构名  : IMSA_AT_CIREG_QRY_CNF_STRU
 结构说明: +CIREG IMS注册状态查询结果

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];

    VOS_UINT32                          bitOpExtInfo    : 1 ;                   /* +CIREG=2时,标志位置1 */
    VOS_UINT32                          bitOpSpare      : 31;

    AT_IMSA_IMS_REG_STATE_REPORT_ENUM_UINT32    enCireg;
    VOS_UINT32                          ulRegInfo;
    VOS_UINT32                          ulExtInfo;                              /* +CIREG=2时 ，扩展信息暂时回复0 */
    VOS_UINT32                          ulResult;                               /* 成功返回VOS_OK，失败返回VOS_ERR */
} IMSA_AT_CIREG_QRY_CNF_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_CIREGU_IND_STRU
 结构说明: ID_IMSA_AT_CIREGU_IND IMS注册状态主动上报

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;                             /* 主动上报时填0X3FFF */
    VOS_UINT8                           ucOpId;                                 /* 填0 */
    VOS_UINT8                           aucReserved[1];

    VOS_UINT32                          bitOpExtInfo    : 1 ;
    VOS_UINT32                          bitOpSpare      : 31;

    VOS_UINT32                          ulRegInfo;
    VOS_UINT32                          ulExtInfo;
} IMSA_AT_CIREGU_IND_STRU;


/*****************************************************************************
 结构名  : AT_IMSA_CIREP_SET_REQ_STRU
 结构说明: +CIREP SRVCC切换状态主动上报控制

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    AT_IMSA_CIREP_REPORT_ENUM_UINT32    enReport;
} AT_IMSA_CIREP_SET_REQ_STRU;


/*****************************************************************************
 结构名  : IMSA_AT_CIREP_SET_CNF_STRU
 结构说明: +CIREP SRVCC切换状态主动上报控制

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    VOS_UINT32                          ulResult;                               /* 成功返回VOS_OK，失败返回VOS_ERR */
} IMSA_AT_CIREP_SET_CNF_STRU;


/*****************************************************************************
 结构名  : IMSA_AT_CIREP_QRY_CNF_STRU
 结构说明: +CIREP IMSVOPS能力查询

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    AT_IMSA_CIREP_REPORT_ENUM_UINT32            enReport;
    AT_IMSA_IMSVOPS_CAPABILITY_ENUM_UINT32      enImsvops;
    VOS_UINT32                          ulResult;                               /* 成功返回VOS_OK，失败返回VOS_ERR */
} IMSA_AT_CIREP_QRY_CNF_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_VOLTEIMPU_QRY_CNF_STRU
 结构说明: ^VOLTEIMPU IMPU信息查询结果

 修改历史      :
  1.日    期   : 2013年12月05日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                             /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;               /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    VOS_UINT32                          ulResult;              /* 成功返回VOS_OK，失败返回VOS_ERR */
    VOS_UINT32                          ulImpuLen;
    VOS_CHAR                            aucImpu[AT_IMSA_IMPU_MAX_LENGTH];
} IMSA_AT_VOLTEIMPU_QRY_CNF_STRU;


/*****************************************************************************
 结构名  : AT_IMSA_ROAMING_IMS_QRY_REQ_STRU
 结构说明: AT通知IMSA查询漫游是否支持的结构

 修改历史      :
  1.日    期   : 2016年1月15日
    作    者   : w00316404
    修改内容   : 新增结构
*****************************************************************************/
typedef AT_IMSA_MSG_STRU AT_IMSA_ROAMING_IMS_QRY_REQ_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_ROAMING_IMS_QRY_CNF_STRU
 结构说明: ^ROAMIMSSERVICE 漫游状态下IMS服务是否支持查询命令CNF消息

 修改历史      :
  1.日    期   : 2016年1月15日
    作    者   : w00316404
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                              ulMsgId;                            /* _H2ASN_Skip */
    VOS_UINT16                              usClientId;
    VOS_UINT8                               ucOpId;
    VOS_UINT8                               aucReserved[1];
    AT_IMSA_ROAMING_IMS_SUPPORT_ENUM_UINT32 enRoamingImsSupportFlag;
    VOS_UINT32                              ulResult;
} IMSA_AT_ROAMING_IMS_QRY_CNF_STRU;


/*****************************************************************************
 结构名  : IMSA_AT_CIREPH_IND_STRU
 结构说明: +CIREPH SRVCC切换状态主动上报

 修改历史      :
  1.日    期   : 2013年9月13日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;                             /* 主动上报时填0X3FFF */
    VOS_UINT8                           ucOpId;                                 /* 填0 */
    VOS_UINT8                           aucReserved[1];
    AT_IMSA_SRVCC_HANDVOER_ENUM_UINT32  enHandover;
} IMSA_AT_CIREPH_IND_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_CIREPI_IND_STRU
 结构说明: +CIREPI IMSVOPS主动上报

 修改历史      :
  1.日    期   : 2013年9月23日
    作    者   : Y00213812
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;                                /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;                             /* 主动上报时填0X3FFF */
    VOS_UINT8                           ucOpId;                                 /* 填0 */
    VOS_UINT8                           aucReserved[1];
    AT_IMSA_IMSVOPS_CAPABILITY_ENUM_UINT32      enImsvops;
} IMSA_AT_CIREPI_IND_STRU;


/*****************************************************************************
 结构名称: IMSA_AT_VT_PDP_ACTIVATE_IND_STRU
 结构说明: IMSA上报VT承载激活成功结果的结构

 修改历史      :
  1.日    期   : 2014年07月31日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                             /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;               /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8     enRatType;                  /* 注册域 */
    TAF_PDP_ADDR_STRU                   stPdpAddr;
    TAF_PDP_DNS_STRU                    stIpv4Dns;
    TAF_PDP_IPV6_DNS_STRU               stIpv6Dns;
} IMSA_AT_VT_PDP_ACTIVATE_IND_STRU;


/*****************************************************************************
 结构名称: IMSA_AT_VT_PDP_DEACTIVATE_IND_STRU
 结构说明: IMSA上报VT承载去激活成功结果的结构

 修改历史      :
  1.日    期   : 2014年07月31日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                             /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;               /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType;
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8     enRatType;             /* 注册域 */
    VOS_UINT8                           aucReserved[3];
} IMSA_AT_VT_PDP_DEACTIVATE_IND_STRU;

/*****************************************************************************
 结构名称： IMSA_AT_MT_STATES_IND_STRU
 结构说明:  IMSA上报AT收到被叫事件的结构

 修改历史:
 1.日期: 2015年03月18日
   作者: m00277840
   修改内容:新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                          /* _H2ASN_Skip */
    VOS_UINT32                          ulMsgId;            /* _H2ASN_Skip */
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    VOS_UINT32                          ulCauseCode;
    VOS_UINT8                           ucMtStatus;
    VOS_UINT8                           aucRsv[3];
    VOS_UINT8                           aucAsciiCallNum[AT_IMSA_CALL_ASCII_NUM_MAX_LENGTH];
} IMSA_AT_MT_STATES_IND_STRU;

/*****************************************************************************
 结构名  : AT_IMSA_IMS_REG_DOMAIN_QRY_REQ_STRU
 结构说明: 查询IMS注册域请求
 修改历史      :
  1.日    期   : 2015年10月07日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef AT_IMSA_MSG_STRU AT_IMSA_IMS_REG_DOMAIN_QRY_REQ_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_IMS_REG_DOMAIN_QRY_CNF_STRU
 结构说明: 查询IMS注册域回复
 修改历史      :
  1.日    期   : 2015年10月07日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                              ulMsgId;                            /* _H2ASN_Skip */
    VOS_UINT16                              usClientId;
    VOS_UINT8                               ucOpId;
    IMSA_AT_IMS_REG_DOMAIN_TYPE_ENUM_UINT8  enImsRegDomain;
} IMSA_AT_IMS_REG_DOMAIN_QRY_CNF_STRU;

/*****************************************************************************
 结构名  : AT_IMSA_IMS_CTRL_MSG_STRU
 结构说明: AT转发MAPCON->IMSA的内部消息
 修改历史      :
  1.日    期   : 2015年10月07日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                              ulMsgId;                            /* _H2ASN_Skip */
    VOS_UINT16                              usClientId;
    VOS_UINT8                               ucOpId;
    VOS_UINT8                               aucReserved[1];
    VOS_UINT32                              ulWifiMsgLen;
    VOS_UINT8                               aucWifiMsg[4];
} AT_IMSA_IMS_CTRL_MSG_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_IMS_CTRL_MSG_STRU
 结构说明: AT转发IMSA->MAPCON的内部消息
 修改历史      :
  1.日    期   : 2015年10月07日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                              ulMsgId;                            /* _H2ASN_Skip */
    VOS_UINT16                              usClientId;
    VOS_UINT8                               ucOpId;
    VOS_UINT8                               aucReserved[1];
    VOS_UINT32                              ulWifiMsgLen;
    VOS_UINT8                               aucWifiMsg[4];
} IMSA_AT_IMS_CTRL_MSG_STRU;


/*****************************************************************************
 结构名  : IMSA_AT_IMS_RAT_HANDOVER_IND_STRU
 结构说明: IMS注册域切换指示

 修改历史      :
  1.日    期   : 2016年04月07日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                              ulMsgId;                            /* _H2ASN_Skip */
    VOS_UINT16                              usClientId;                         /* 主动上报时填0X3FFF */
    VOS_UINT8                               ucOpId;                             /* 填0 */
    IMSA_AT_IMS_RAT_HO_STATUS_ENUM_UINT8    enHoStatus;                         /* 切换状态 */
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8         enSrcRat;                           /* 原有IMS注册域 */
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8         enDstRat;                           /* 目标IMS注册域 */
    VOS_UINT8                               aucReserved[2];
    IMSA_AT_HO_CAUSE_ENUM_UINT32            enCause;                            /* 切换失败原因值 */
} IMSA_AT_IMS_RAT_HANDOVER_IND_STRU;

/*****************************************************************************
 结构名  : IMSA_AT_IMS_SRV_STATUS_UPDATE_IND_STRU
 结构说明: IMS服务状态变化指示

 修改历史      :
  1.日    期   : 2016年04月07日
    作    者   : f00179208
    修改内容   : 新增结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    VOS_UINT32                              ulMsgId;                            /* _H2ASN_Skip */
    VOS_UINT16                              usClientId;                         /* 主动上报时填0X3FFF */
    VOS_UINT8                               ucOpId;                             /* 填0 */
    VOS_UINT8                               aucReserved[1];
    IMSA_AT_IMS_SERVICE_STATUS_ENUM_UINT8   enSmsSrvStatus;                     /* 短信的IMS服务状态 */
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8         enSmsSrvRat;                        /* 短信的IMS服务域 */
    IMSA_AT_IMS_SERVICE_STATUS_ENUM_UINT8   enVoIpSrvStatus;                    /* VoIP的IMS服务状态 */
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8         enVoIpSrvRat;                       /* VoIP的IMS服务域 */
    IMSA_AT_IMS_SERVICE_STATUS_ENUM_UINT8   enVtSrvStatus;                      /* VT的IMS服务状态 */
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8         enVtSrvRat;                         /* VT的IMS服务域 */
    IMSA_AT_IMS_SERVICE_STATUS_ENUM_UINT8   enVsSrvStatus;                      /* VS的IMS服务状态 */
    IMSA_AT_IMS_RAT_TYPE_ENUM_UINT8         enVsSrvRat;                         /* VS的IMS服务域 */
} IMSA_AT_IMS_SRV_STATUS_UPDATE_IND_STRU;

/*****************************************************************************
  4 宏定义
*****************************************************************************/


/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 接口函数声明
*****************************************************************************/

/*****************************************************************************
  7 OTHERS定义
*****************************************************************************/

/* ASN解析结构 */
typedef struct
{
    VOS_UINT32                          ulMsgId;                                /*_H2ASN_MsgChoice_Export AT_IMSA_MSG_TYPE_ENUM_UINT32 */
    VOS_UINT8                           aucMsg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          AT_IMSA_MSG_TYPE_ENUM_UINT32
    ****************************************************************************/
}AT_IMSA_INTERFACE_MSG_DATA;
/*_H2ASN_Length UINT32*/

typedef struct
{
    VOS_MSG_HEADER
    AT_IMSA_INTERFACE_MSG_DATA           stMsgData;
} AtImsaInterface_MSG;


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


