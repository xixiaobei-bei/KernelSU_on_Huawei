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


#ifndef __MSP_DIAG_COMM_H__
#define __MSP_DIAG_COMM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>
#include <vos.h>
#include <mdrv.h>

/**************************************************************************
  宏定义
**************************************************************************/

/*最好PS将MODID 和DIAG_AIR_MSG_LOG_ID的地方都替换成DIAG_ID*/
#define DIAG_ID(module_id, log_type)   (VOS_UINT32)(module_id | (log_type << 12))

#define DIAG_AIR_MSG_LOG_ID(module_id, is_up_link)  DIAG_ID(module_id, is_up_link) /*module_id对应PID*/

#define DIAG_GEN_LOG_MODULE(modemid, modetype, level)  \
               (((VOS_UINT32)(modemid & 0xff) << 24)  \
              | ((VOS_UINT32)(modetype & 0xf) << 16)  \
              | ((VOS_UINT32)(level    & 0xf ) << 12))

#define DIAG_GEN_MODULE(modemid, modetype)  \
               (((VOS_UINT32)(modemid & 0xff) << 24)  \
              | ((VOS_UINT32)(modetype & 0xf) << 16))


#define DIAG_GEN_MODULE_EX(modemid, modetype, groupid)  \
                   (((VOS_UINT32)(modemid & 0xff) << 24)  \
                  | ((VOS_UINT32)(modetype & 0xf) << 16) \
                  | ((VOS_UINT32)(groupid  & 0xf)  << 8))

#define DIAG_GEN_LOG_ID(filenum, linenum)   \
                ((((VOS_UINT32)(filenum & 0XFFFF)) << 16)   \
                | ((VOS_UINT32)(linenum & 0XFFFF)))

/*is_up_link取值*/
#define OS_MSG_UL                (0x01)/* 表示上行消息*/
#define OS_MSG_DL                (0x02)/* 表示下行消息*/

#define DIAG_SIDE_UE             (0x1)  /* 表示UE接收的空口消息：NET-->UE*/
#define DIAG_SIDE_NET            (0x2)  /* 表示NET接收的空口消息：UE-->NET*/

/* 事件信息打印级别定义*/
#define DIAG_LOG_TYPE_INFO            0x00000008UL
#define DIAG_LOG_TYPE_AUDIT_FAILURE   0x00000004UL
#define DIAG_LOG_TYPE_AUDIT_SUCCESS   0x00000002UL
#define DIAG_LOG_TYPE_ERROR           0x00000001UL
#define DIAG_LOG_TYPE_WARNING         0x00000010UL

/* MSP内部使用*/
#define ID_MSG_DIAG_HSO_DISCONN_IND                 (0x00010004)

/*diag AGENT发给PS模块的HSO回放请求*/
#define ID_MSG_DIAG_CMD_REPLAY_TO_PS    			(0x00010100)

/* diag 发送给各个子系统的建连请求 */
#define ID_MSG_DIAG_CMD_CONNECT_REQ    		        (0x00010200)
#define ID_MSG_DIAG_CMD_CONNECT_CNF    		        (ID_MSG_DIAG_CMD_CONNECT_REQ)
/* diag 发送给各个子系统的断连请求 */
#define ID_MSG_DIAG_CMD_DISCONNECT_REQ    		    (0x00010202)
#define ID_MSG_DIAG_CMD_DISCONNECT_CNF    		    (ID_MSG_DIAG_CMD_DISCONNECT_REQ)

/* diag 发送给TL-PHY的消息连接请求 */
#define ID_MSG_DIAG_DSP_CONNECT_REQ    		        (0x30004903)
#define ID_MSG_DIAG_DSP_CONNECT_CNF    		        (ID_MSG_DIAG_DSP_CONNECT_REQ)
/* diag 发送给TL-PHY的消息断连请求 */
#define ID_MSG_DIAG_DSP_DISCONNECT_REQ    		    (0x30004904)
#define ID_MSG_DIAG_DSP_DISCONNECT_CNF    		    (ID_MSG_DIAG_DSP_DISCONNECT_REQ)

/* diag 发送给HL1C的消息连接请求 */
#define ID_MSG_DIAG_HL1C_CONNECT_REQ    		    (0x3f000007)
#define ID_MSG_DIAG_HL1C_CONNECT_CNF    		    (ID_MSG_DIAG_HL1C_CONNECT_REQ)
/* diag 发送给HL1C的消息断连请求 */
#define ID_MSG_DIAG_HL1C_DISCONNECT_REQ    		    (0x3f000006)
#define ID_MSG_DIAG_HL1C_DISCONNECT_CNF    		    (ID_MSG_DIAG_HL1C_DISCONNECT_REQ)
/* 端口信息 */
#define ID_MSG_DIAG_NRPHY_PORT_INFO                 (0x3f00000a)

/* 对外消息的范围与 DIAG_MESSAGE_TYPE_U32 拉通 */

/* 2000 - 2fff是与PS的消息范围 */

/* c000 - cfff是与HIFI的消息范围 */
#define     DIAG_HIFI_RELEASE_REQ                   (0x0000c001)

/*****************************************************************************
  4 Enum
*****************************************************************************/

/* ==============消息层头结构枚举值定义==================================== */

/* MSP_DIAG_STID_STRU:pri4b */
#ifdef DIAG_SYSTEM_5G
enum DIAG_MESSAGE_TYPE
{
    DIAG_MSG_TYPE_RSV       = 0x0,
    DIAG_MSG_TYPE_MSP       = 0x1,
    DIAG_MSG_TYPE_PS        = 0x2,
    DIAG_MSG_TYPE_PHY       = 0x3,
    DIAG_MSG_TYPE_BBP       = 0x4,
    DIAG_MSG_TYPE_HSO       = 0x5,
    DIAG_MSG_TYPE_BSP       = 0x9, /*MODEM BSP*/
    DIAG_MSG_TYPE_EASYRF    = 0xa,
    DIAG_MSG_TYPE_AP_BSP    = 0xb, /*AP BSP*/
    DIAG_MSG_TYPE_AUDIO     = 0xc,
    DIAG_MSG_TYPE_APP       = 0xe,
    DIAG_MSG_TYPE_BUTT
};
#else
enum DIAG_MESSAGE_TYPE
{
    DIAG_MSG_TYPE_RSV   = 0x0,
    DIAG_MSG_TYPE_MSP   = 0x1,
    DIAG_MSG_TYPE_PS    = 0x2,
    DIAG_MSG_TYPE_PHY   = 0x3,
    DIAG_MSG_TYPE_BBP   = 0x4,
    DIAG_MSG_TYPE_HSO   = 0x5,
    DIAG_MSG_TYPE_BSP   = 0x9,
    DIAG_MSG_TYPE_ISP   = 0xa,
    DIAG_MSG_TYPE_AUDIO = 0xc,
    DIAG_MSG_TYPE_APP   = 0xe,
    DIAG_MSG_TYPE_BUTT
};
#endif
typedef VOS_UINT32 DIAG_MESSAGE_TYPE_U32;

/* MSP_DIAG_STID_STRU:mode4b */
enum DIAG_MODE_TYPE
{
    DIAG_MODE_LTE  = 0x0,
    DIAG_MODE_TDS  = 0x1,
    DIAG_MODE_GSM  = 0x2,
    DIAG_MODE_UMTS = 0x3,
    DIAG_MODE_1X   = 0x4,
    DIAG_MODE_HRPD = 0x5,
    DIAG_MODE_NR   = 0x6,
    DIAG_MODE_COMM = 0xf
};
typedef VOS_UINT32 DIAG_MODE_TYPE_U32;


enum DIAG_MODEM_ID
{
    DIAG_MODEM_0 = 0x0,
    DIAG_MODEM_1 = 0x1,
    DIAG_MODEM_2 = 0x2
};
typedef unsigned int DIAG_MODEM_ID_U32;

typedef enum
{
    PS_LOG_LEVEL_OFF  = 0,
    PS_LOG_LEVEL_ERROR,
    PS_LOG_LEVEL_WARNING,
    PS_LOG_LEVEL_NORMAL,
    PS_LOG_LEVEL_INFO,
    PS_LOG_LEVEL_BUTT
}PS_LOG_LEVEL_EN;

/**************************************************************************
  5 结构定义
**************************************************************************/

/**************************************************************************
  函数声明
**************************************************************************/

/* ==============主动上报接口参数========================================== */

typedef struct
{
    VOS_UINT32        ulModule;		/* DIAG_GEN_MODULE*/
    VOS_UINT32        ulPid;
    VOS_UINT32        ulEventId;		/* 事件ID */
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
}DIAG_EVENT_IND_STRU;

typedef struct
{
    VOS_UINT32        ulModule;       /* DIAG_GEN_MODULE*/
    VOS_UINT32        ulPid;
    VOS_UINT32        ulMsgId;
    VOS_UINT32        ulDirection;
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
}DIAG_AIR_IND_STRU;

typedef struct
{
    VOS_UINT32        ulModule;
    VOS_UINT32        ulPid;
    VOS_UINT32        ulMsgId;
    VOS_UINT32        ulReserve;
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
} DIAG_TRANS_IND_STRU;

/*********************************连接管理相关********************************************/
/* OSA消息头 */
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32              ulMsgId;           /* 消息ID */
    VOS_UINT32              ulLen;             /* 消息长度 */
    VOS_UINT8               pContext[0];       /* 数据其实地址*/ /*lint !e43 */
}DIAG_OSA_MSG_HEAD;

typedef DIAG_OSA_MSG_HEAD DIAG_CFG_MSG_HEAD;

/* 核间消息头 */
typedef struct{
	VOS_UINT32		        ulMsgId;            /* 消息ID */
	VOS_UINT32		        ulLen;              /* 消息长度 */
	VOS_UINT8		        *pContext[0];       /* 数据其实地址*/ /*lint !e43 */
}DIAG_CORE_MSG_HEAD;

/* 和TLPHY消息头 */
typedef struct
{
	VOS_UINT32              ulMsgId;            /* 消息ID */
	VOS_UINT8	            *pContext[0];        /*lint !e43 */
}DIAG_PHY_MSG_HEAD;

typedef struct{
    VOS_UINT32			    ulChannelId;	    /* 通道ID */
    VOS_UINT32			    ulResult;		    /*  处理结果 0成功, 0x5C5C5C5C通道未分配, 0x5A5A5A5A通道未使用, 其他值失败*/
}DIAG_CONNECT_RESULT;

/* 连接消息回复结构体,  各组件-> DIAG */
typedef struct{
    VOS_UINT32				ulSn;
    DIAG_CONNECT_RESULT     pstResult[0];    /*lint !e43 */
}DIAG_CONN_CNF_MSG_STRU;

/* 连接消息分发结构体,  DIAG -> 各组件 */
typedef struct{
    VOS_UINT32				ulSn;
}DIAG_CONN_REQ_MSG_STRU;


/* 消息ID :ID_MSG_DIAG_CMD_REPLAY_TO_PS */
typedef struct
{
    VOS_UINT32 ulReserved;/*保留*/
} DIAG_CMD_REPLAY_SET_REQ_STRU;

/* 消息ID: DIAG_HIFI_RELEASE_REQ*/
typedef struct
{
    VOS_UINT32                  ulRsv;
} DIAG_MSG_TO_OTHERCPU_DISCONN_STRU;


/* 层间消息匹配接口，入参和返回值为标准的OSA消息格式 */
typedef VOS_VOID* (*DIAG_LayerMsgMatchFunc)(VOS_VOID *pMsg);
/*****************************************************************************
 Function Name   : DIAG_TraceMatchFuncReg
 Description        : 层间消息过滤注册接口(此接口不支持重复注册，多次注册返还失败)
 Input                : 过滤处理函数
 Output              : None
 Return              : 返回值为注册结果: 0-注册成功；其他-注册失败
*****************************************************************************/
VOS_UINT32 DIAG_LayerMsgMatchFuncReg(DIAG_LayerMsgMatchFunc pFun);

/* 层间消息匹配完成后的notify接口 */
typedef VOS_UINT32 (*DIAG_LayerMsgMatchNotifyFunc)(VOS_UINT32 sendPid, VOS_VOID *pMsg);
/*****************************************************************************
 Function Name   : DIAG_TraceMatchFuncReg
 Description        : 层间消息过滤的notify接口注册(此接口不支持重复注册，多次注册返还失败)
 Input                : 过滤处理函数
 Output              : None
 Return              : 返回值为注册结果: 0-注册成功；其他-注册失败
*****************************************************************************/
VOS_UINT32 DIAG_LayerMsgMatchNotifyFuncReg(DIAG_LayerMsgMatchNotifyFunc pFun);

/*****************************************************************************
 函数 名      : DIAG_TransReport
 功能描述   : 结构化数据上报接口(替换原来的DIAG_ReportCommand)
 输入参数   : DIAG_TRANS_IND_STRU->ulModule( 31-24:modemid,19-16:modeid)
 输入参数    : DIAG_TRANS_IND_STRU->ulPid(模块的PID)
                          DIAG_TRANS_IND_STRU->ulMsgId(透传命令ID)
                          DIAG_TRANS_IND_STRU->ulLength(透传信息的长度)
                          DIAG_TRANS_IND_STRU->pData(透传信息)
*****************************************************************************/
VOS_UINT32 DIAG_TransReport(DIAG_TRANS_IND_STRU *pstData);

/*****************************************************************************
 函数 名       : DIAG_TransReport_Ex
 功能描述    : 结构化数据上报的扩展接口(相比DIAG_TransReport，新增了11-8bits 组件信息)
 输入参数    : DIAG_TRANS_IND_STRU->ulModule( 31-24:modemid,19-16:modeid, 11-8:groupid )
 输入参数    : DIAG_TRANS_IND_STRU->ulPid(模块的PID)
                            DIAG_TRANS_IND_STRU->ulMsgId(透传命令ID)
                            DIAG_TRANS_IND_STRU->ulLength(透传信息的长度)
                            DIAG_TRANS_IND_STRU->pData(透传信息)
*****************************************************************************/
VOS_UINT32 DIAG_TransReport_Ex(DIAG_TRANS_IND_STRU *pstData);

/*****************************************************************************
 函数 名      : DIAG_LogReport
 功能描述  : 打印上报接口
 输入参数  : ulModuleId( 31-24:modemid,19-16:modeid,15-12:level )
                          ulPid( PID )
                          pcFileName(上报时会把文件路径删除，只保留文件名)
                          ulLineNum(行号)
                          pszFmt(可变参数)
注意事项  : 由于此接口会被协议栈频繁调用，为提高处理效率，本接口内部会使用1K的局部变量保存上报的字符串信息，
                         从而此接口对协议栈有两点限制，
                         一是调用此接口的任务栈中的内存要至少为此接口预留1K空间；
                         二是调用此接口输出的log不要超过1K（超出部分会自动丢弃）
*****************************************************************************/
VOS_UINT32 DIAG_LogReport(VOS_UINT32 ulModuleId, VOS_UINT32 ulPid, VOS_CHAR *cFileName, VOS_UINT32 ulLineNum, VOS_CHAR* pszFmt, ...);
/******************************************************************************
函数名称: DIAG_LogIdReport
功能描述: 打印点类型的打印接口函数
参数说明:ulModuleId[in]  : ( 31-24:modemid,23-16:modeid,15-12:level )
                      ulPid[in]           : PID
                      ulLogId[in]       : 由文件号和行号根据DIAG_LOG_ID生成
                      amount[in]       : 可变参数个数（不包括ulModuleId/ulLevel/ulLogId/amout）
                      ...                   : 可变参数
调用约束:
            1. 绝对禁止对此函数进行二次封装，只能转定义
            2. 支持可变的参数个数，但必须在调用时由参数amount指定参数个数
            3. 可变参数只支持int类型
            4. 目前版本中支持的最大参数个数是6个，超过的参数默认丢弃
******************************************************************************/
VOS_INT32 DIAG_LogIdReport(VOS_UINT32 ulModuleId, VOS_UINT32 ulPid,
                        VOS_UINT32 ulLogId, VOS_UINT32 amount, ...);

/*****************************************************************************
 函数 名     : DIAG_EventReport
 功能描述  : 事件上报接口，给PS使用(替换原来的DIAG_ReportEventLog)
 输入参数  : DIAG_EVENT_IND_STRU->ulModule( 31-24:modemid,19-16:modeid )
                          DIAG_EVENT_IND_STRU->ulEventId(event ID)
                          DIAG_EVENT_IND_STRU->ulLength(event的长度)
                          DIAG_EVENT_IND_STRU->pData(event信息)
*****************************************************************************/
VOS_UINT32 DIAG_EventReport(DIAG_EVENT_IND_STRU *pstEvent);

/*****************************************************************************
 函数 名       : DIAG_AirMsgReport
 功能描述  : 空口消息上报接口，给PS使用(替换原来的DIAG_ReportAirMessageLog)
 输入参数  : DIAG_AIR_IND_STRU->ulModule( 31-24:modemid,23-16:modeid)
                          DIAG_AIR_IND_STRU->ulMsgId(空口消息ID)
                          DIAG_AIR_IND_STRU->ulDirection(空口消息的方向)
                          DIAG_AIR_IND_STRU->ulLength(空口消息的长度)
                          DIAG_AIR_IND_STRU->pData(空口消息信息)
*****************************************************************************/
VOS_UINT32 DIAG_AirMsgReport(DIAG_AIR_IND_STRU *pstAir);

/*****************************************************************************
 函 数 名     : DIAG_TraceReport
 功能描述  : 层间消息上报接口，给PS使用(替换原来的DIAG_ReportLayerMessageLog)
 输入参数  : pMsg(标准的VOS消息体，源模块、目的模块信息从消息体中获取)
*****************************************************************************/
VOS_VOID DIAG_TraceReport(VOS_VOID *pMsg);

/*****************************************************************************
 Function Name   : DIAG_GetConnState
 Description        : 获取当前工具连接状态
 Return              : 1:connect; 0:disconnect
*****************************************************************************/
VOS_UINT32 DIAG_GetConnState(VOS_VOID);

/*****************************************************************************
 函 数 名     : Log_GetPrintLevel
 功能描述  : 得到模块Id、子模块Id在打印级别记录表中的索引号
 return               : PS_LOG_LEVEL_EN
*****************************************************************************/
VOS_UINT32 Diag_GetLogLevel(VOS_UINT32 ulPid);
/*****************************************************************************
 函 数 名  : DIAG_RttReport	1344
 功能描述  : 1. 上报TL PHY数据，NR/LR Modem提供,仅供TLPHY RTT使用
             2. 入参指针为TLPHY要上报的数据，包含TLPHY完整的数据格式，从service头开始的数据(从sid开始)
             3. Rtt Agent帧结构引用msp头文件msp_diag_comm.h(使用msp.h)
             4. 接口头文件声明放在msp_diag_comm.h(使用msp.h)
             5. pData不能为空 ulDataLen不能为0
             6. 必须要保证ulDataLen正确性，不能出现pData访问越界
             7. ulDataLen不能大于64K
 返回值   :  ERR_MSP_SUCCESS  成功
             ERR_MSP_DIAG_SRC_BUFF_ALLOC_FAIL 源端内存不足
             其他值          失败
*****************************************************************************/
VOS_UINT32 DIAG_RttReport(VOS_VOID *pData, VOS_UINT32 ulDatalen);
#ifdef __cplusplus
    }
#endif
#endif

