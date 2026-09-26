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
 *
 */
#ifndef __MSP_DIAG_COMM_H__
#define __MSP_DIAG_COMM_H__

#include <product_config.h>
#include <vos.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MDRV_DIAG_ID(module_id, log_type)   (unsigned int)(module_id | (log_type << 12))

#define MDRV_DIAG_AIR_MSG_LOG_ID(module_id, is_up_link)  MDRV_DIAG_ID(module_id, is_up_link)

#define MDRV_DIAG_GEN_LOG_MODULE(modemid, modetype, level)  \
    ((((VOS_UINT32)modemid & 0xff) << 24)  \
     | (((VOS_UINT32)modetype & 0xf) << 16)  \
     | (((VOS_UINT32)level & 0xf) << 12))

#define MDRV_DIAG_GEN_MODULE(modemid, modetype)  \
    ((((VOS_UINT32)modemid & 0xff) << 24)  \
     | (((VOS_UINT32)modetype & 0xf) << 16))

#define MDRV_DIAG_GEN_MODULE_EX(modemid, modetype, groupid)  \
    ((((VOS_UINT32)modemid & 0xff) << 24)  \
     | (((VOS_UINT32)modetype & 0xf) << 16) \
     | (((VOS_UINT32)groupid  & 0xf)  << 8))

#define MDRV_DIAG_GEN_LOG_ID(filenum, linenum)   \
    (((((VOS_UINT32)filenum & 0XFFFF)) << 16)   \
     | (((VOS_UINT32)linenum & 0XFFFF)))

#define OS_MSG_UL                (0x01)
#define OS_MSG_DL                (0x02)

#define DIAG_SIDE_UE             (0x1)
#define DIAG_SIDE_NET            (0x2)

#define DIAG_LOG_TYPE_INFO            0x00000008UL
#define DIAG_LOG_TYPE_AUDIT_FAILURE   0x00000004UL
#define DIAG_LOG_TYPE_AUDIT_SUCCESS   0x00000002UL
#define DIAG_LOG_TYPE_ERROR           0x00000001UL
#define DIAG_LOG_TYPE_WARNING         0x00000010UL

#define ID_MSG_DIAG_HSO_DISCONN_IND                 (0x00010004)

#define ID_MSG_DIAG_CMD_REPLAY_TO_PS    			(0x00010100)

/* diag 发送给各个子系统的建连请求 */
#define ID_MSG_DIAG_CMD_CONNECT_REQ    		        (0x00010200)
#define ID_MSG_DIAG_CMD_CONNECT_CNF    		        (ID_MSG_DIAG_CMD_CONNECT_REQ)
/* diag 发送给各个子系统的断连请求 */
#define ID_MSG_DIAG_CMD_DISCONNECT_REQ    		    (0x00010202)
#define ID_MSG_DIAG_CMD_DISCONNECT_CNF    		    (ID_MSG_DIAG_CMD_DISCONNECT_REQ)

/* diag 在EMU平台调用串口连接请求 */
#define ID_MSG_DIAG_DEBUG_EMU_CONNECT_REQ           (0x10010204)
#define ID_MSG_DIAG_DEBUG_EMU_CONNECT_CNF    		(ID_MSG_DIAG_DEBUG_EMU_CONNECT_REQ)

/* diag 发送给TL-PHY的消息连接请求 */
#define ID_MSG_DIAG_DSP_CONNECT_REQ    		        (0x30004903)
#define ID_MSG_DIAG_DSP_CONNECT_CNF    		        (ID_MSG_DIAG_DSP_CONNECT_REQ)
/* diag 发送给TL-PHY的消息断连请求 */
#define ID_MSG_DIAG_DSP_DISCONNECT_REQ    		    (0x30004904)
#define ID_MSG_DIAG_DSP_DISCONNECT_CNF    		    (ID_MSG_DIAG_DSP_DISCONNECT_REQ)


/* diag 发送给TL-PHY的维测开关 */
#define ID_MSG_DIAG_DSP_MNTN_SWITCH                 (0x30004907)
#define ID_MSG_DIAG_DSP_MNTN_SWITCH_CNF             (ID_MSG_DIAG_DSP_MNTN_SWITCH)

/* diag 发送给LTE-V的消息连接请求 */
#define ID_MSG_DIAG_LVDSP_CONNECT_REQ    		        (0x37004903)
#define ID_MSG_DIAG_LVDSP_CONNECT_CNF    		        (ID_MSG_DIAG_LVDSP_CONNECT_REQ)
/* diag 发送给LTE-V的消息断连请求 */
#define ID_MSG_DIAG_LVDSP_DISCONNECT_REQ    		    (0x37004904)
#define ID_MSG_DIAG_LVDSP_DISCONNECT_CNF    		    (ID_MSG_DIAG_LVDSP_DISCONNECT_REQ)


/* diag 发送给HL1C的消息连接请求 */
#define ID_MSG_DIAG_HL1C_CONNECT_REQ    		    (0x3f000007)
#define ID_MSG_DIAG_HL1C_CONNECT_CNF    		    (ID_MSG_DIAG_HL1C_CONNECT_REQ)
/* diag 发送给HL1C的消息断连请求 */
#define ID_MSG_DIAG_HL1C_DISCONNECT_REQ    		    (0x3f000006)
#define ID_MSG_DIAG_HL1C_DISCONNECT_CNF    		    (ID_MSG_DIAG_HL1C_DISCONNECT_REQ)

#define ID_MSG_DIAG_HL1C_MNTN_SWITCH                (0x3f00000a)
#define ID_MSG_DIAG_HL1C_MNTN_SWITCH_CNF            (ID_MSG_DIAG_HL1C_MNTN_SWITCH)

#define ID_MSG_DIAG_RFDSP_MNTN_SWITCH               (0x0000f805)

/* diag 发送给C-PHY的维测开关*/
#define ID_MSG_DIAG_CPHY_MNTN_SWITCH                (0x0000901d)

/* diag 发送给GU-PHY的维测开关*/
#define ID_MSG_DIAG_GUPHY_MNTN_SWITCH               (0x0000ff07)

#define     DIAG_HIFI_RELEASE_REQ                   (0x0000c001)

/* MSP_DIAG_STID_STRU:pri4b */
#ifdef DIAG_SYSTEM_5G
enum mdrv_diag_message_type {
    DIAG_MSG_TYPE_RSV       = 0x0,
    DIAG_MSG_TYPE_MSP       = 0x1,
    DIAG_MSG_TYPE_PS        = 0x2,
    DIAG_MSG_TYPE_PHY       = 0x3,
    DIAG_MSG_TYPE_BBP       = 0x4,
    DIAG_MSG_TYPE_HSO       = 0x5,
    DIAG_MSG_TYPE_BSP       = 0x9, /* MODEM BSP */
    DIAG_MSG_TYPE_EASYRF    = 0xa,
    DIAG_MSG_TYPE_AP_BSP    = 0xb, /* AP BSP */
    DIAG_MSG_TYPE_AUDIO     = 0xc,
    DIAG_MSG_TYPE_APP       = 0xe,
    DIAG_MSG_TYPE_BUTT
};
#else
enum mdrv_diag_message_type {
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

/* MSP_DIAG_STID_STRU:mode4b */
enum mdrv_diag_mode_type {
    DIAG_MODE_LTE  = 0x0,
    DIAG_MODE_TDS  = 0x1,
    DIAG_MODE_GSM  = 0x2,
    DIAG_MODE_UMTS = 0x3,
    DIAG_MODE_1X   = 0x4,
    DIAG_MODE_HRPD = 0x5,
    DIAG_MODE_NR   = 0x6,
    DIAG_MODE_LTEV = 0x7,
    DIAG_MODE_COMM = 0xf
};

enum mdrv_diag_modem_id {
    DIAG_MODEM_0 = 0x0,
    DIAG_MODEM_1 = 0x1,
    DIAG_MODEM_2 = 0x2,
    DIAG_MODEM_COMMON = 0x7
};

typedef enum {
    PS_LOG_LEVEL_OFF  = 0,
    PS_LOG_LEVEL_ERROR,
    PS_LOG_LEVEL_WARNING,
    PS_LOG_LEVEL_NORMAL,
    PS_LOG_LEVEL_INFO,
    PS_LOG_LEVEL_BUTT
} mdrv_diag_log_level_e;

typedef enum {
    DIAG_STATE_DISCONN  = 0,
    DIAG_STATE_CONN     = 1,
    DIAG_STATE_BUTT
} mdrv_diag_state_e;

typedef struct {
    VOS_UINT32        ulModule;		/* MDRV_DIAG_GEN_MODULE */
    VOS_UINT32        ulPid;
    VOS_UINT32        ulEventId;		/* 事件ID */
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
} mdrv_diag_event_ind_s;

typedef struct {
    VOS_UINT32        ulModule;       /* MDRV_DIAG_GEN_MODULE */
    VOS_UINT32        ulPid;
    VOS_UINT32        ulMsgId;
    VOS_UINT32        ulDirection;
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
} mdrv_diag_air_ind_s;

typedef struct {
    VOS_UINT32        ulModule;
    VOS_UINT32        ulPid;
    VOS_UINT32        ulMsgId;
    VOS_UINT32        ulReserve;
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
} mdrv_diag_trans_ind_s;

typedef struct {
    VOS_UINT32        ulModule;
    VOS_UINT32        ulPid;
    VOS_UINT32        ulMsgId;
    VOS_UINT32        ulReserve;
    VOS_UINT32        ulLength;
    VOS_VOID          *pData;
} mdrv_diag_dt_ind_s;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32              ulMsgId;
    VOS_UINT32              ulLen;
    VOS_UINT8               pContext[0];   /*lint !e43 */
} mdrv_diag_osa_msg_head_s;

typedef mdrv_diag_osa_msg_head_s mdrv_diag_cfg_msg_head_s;

typedef struct {
    VOS_UINT32		        ulMsgId;            /* 消息ID */
    VOS_UINT32		        ulLen;              /* 消息长度 */
    VOS_UINT8		        *pContext[0];   /*lint !e43 */
} mdrv_core_msg_head_s;

typedef struct {
    VOS_UINT32              ulMsgId;            /* 消息ID */
    VOS_UINT8	            *pContext[0];   /*lint !e43 */
} mdrv_diag_phy_msg_head_s;

typedef struct {
    VOS_UINT32			    ulChannelId;
    VOS_UINT32			    ulResult;
} mdrv_diag_connect_s;

typedef struct {
    VOS_UINT32				ulSn;
    mdrv_diag_connect_s     pstResult[0];   /*lint !e43 */
} mdrv_diag_conn_cnf_msg_s;

typedef struct {
    VOS_UINT32				ulSn;
} mdrv_diag_conn_req_msg_s;

/* 消息ID :ID_MSG_DIAG_CMD_REPLAY_TO_PS */
typedef struct {
    VOS_UINT32 ulReserved;
} mdrv_diag_cmd_reply_set_req_s;

/* 消息ID: DIAG_HIFI_RELEASE_REQ*/
typedef struct {
    VOS_UINT32                  ulRsv;
} mdrv_diag_msg_to_other_cpu_disconn_s;

typedef void (*mdrv_diag_conn_state_notify_fun)(mdrv_diag_state_e state);

typedef void* (*mdrv_diag_layer_msg_match_func)(void *pMsg);
/*****************************************************************************
 Function Name   : DIAG_TraceMatchFuncReg
 Description        : 层间消息过滤注册接口(此接口不支持重复注册，多次注册返还失败)
 Input                : 过滤处理函数
 Output              : None
 Return              : 返回值为注册结果: 0-注册成功；其他-注册失败
*****************************************************************************/
unsigned int mdrv_diag_layer_msg_match_func_reg(mdrv_diag_layer_msg_match_func pfunc);

typedef unsigned int (*mdrv_diag_layer_msg_notify_func)(unsigned int sendPid, void *pMsg);
/*****************************************************************************
 Function Name   : DIAG_TraceMatchFuncReg
 Description        : 层间消息过滤的notify接口注册(此接口不支持重复注册，多次注册返还失败)
 Input                : 过滤处理函数
 Output              : None
 Return              : 返回值为注册结果: 0-注册成功；其他-注册失败
*****************************************************************************/
unsigned int mdrv_diag_layer_msg_notify_func_reg(mdrv_diag_layer_msg_notify_func pfunc);

/*****************************************************************************
 函数 名      : mdrv_diag_trans_report
 功能描述   : 结构化数据上报接口(替换原来的DIAG_ReportCommand)
 输入参数   : mdrv_diag_trans_ind_s->ulModule( 31-24:modemid,19-16:modeid)
 输入参数    : mdrv_diag_trans_ind_s->ulPid(模块的PID)
                          mdrv_diag_trans_ind_s->ulMsgId(透传命令ID)
                          mdrv_diag_trans_ind_s->ulLength(透传信息的长度)
                          mdrv_diag_trans_ind_s->pData(透传信息)
*****************************************************************************/
unsigned int mdrv_diag_trans_report(mdrv_diag_trans_ind_s *pstData);

/*****************************************************************************
 函数 名       : mdrv_diag_trans_report_ex
 功能描述    : 结构化数据上报的扩展接口(相比DIAG_TransReport，新增了11-8bits 组件信息)
 输入参数    : mdrv_diag_trans_ind_s->ulModule( 31-24:modemid,19-16:modeid, 11-8:groupid )
 输入参数    : mdrv_diag_trans_ind_s->ulPid(模块的PID)
                            mdrv_diag_trans_ind_s->ulMsgId(透传命令ID)
                            mdrv_diag_trans_ind_s->ulLength(透传信息的长度)
                            mdrv_diag_trans_ind_s->pData(透传信息)
*****************************************************************************/
unsigned int mdrv_diag_trans_report_ex(mdrv_diag_trans_ind_s *pstData);

/*****************************************************************************
 函数 名      : mdrv_diag_log_report
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
unsigned int mdrv_diag_log_report(unsigned int ulModuleId, unsigned int ulPid, char *cFileName, unsigned int ulLineNum, char* pszFmt, ...);
/******************************************************************************
函数名称: mdrv_diag_logid_report
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
unsigned int mdrv_diag_logid_report(unsigned int ulModuleId, unsigned int ulPid, \
        unsigned int ulLogId, unsigned int amount, ...);

/*****************************************************************************
 函数 名     : mdrv_diag_event_report
 功能描述  : 事件上报接口，给PS使用(替换原来的DIAG_ReportEventLog)
 输入参数  : mdrv_diag_event_ind_s->ulModule( 31-24:modemid,19-16:modeid )
                          mdrv_diag_event_ind_s->ulEventId(event ID)
                          mdrv_diag_event_ind_s->ulLength(event的长度)
                          mdrv_diag_event_ind_s->pData(event信息)
*****************************************************************************/
unsigned int mdrv_diag_event_report(mdrv_diag_event_ind_s *pstEvent);

/*****************************************************************************
 函 数 名  : mdrv_diag_allow_air_msg_report
 功能描述  : 提供给协议栈,用于检测当前是否允许上报AirMsg
 输入参数  : VOS_VOID
 返回值    : ERR_MSP_SUCCESS = 0 当前允许上报空口消息
             ERR_MSP_CFG_LOG_NOT_ALLOW = 203 当前不允许上报空口消息

*****************************************************************************/
unsigned int mdrv_diag_allow_air_msg_report(void);

/*****************************************************************************
 函数 名       : mdrv_diag_air_report
 功能描述  : 空口消息上报接口，给PS使用(替换原来的DIAG_ReportAirMessageLog)
 输入参数  : mdrv_diag_air_ind_s->ulModule( 31-24:modemid,23-16:modeid)
                          mdrv_diag_air_ind_s->ulMsgId(空口消息ID)
                          mdrv_diag_air_ind_s->ulDirection(空口消息的方向)
                          mdrv_diag_air_ind_s->ulLength(空口消息的长度)
                          mdrv_diag_air_ind_s->pData(空口消息信息)
*****************************************************************************/
unsigned int mdrv_diag_air_report(mdrv_diag_air_ind_s *pstAir);

/*****************************************************************************
 函 数 名     : mdrv_diag_trace_report
 功能描述  : 层间消息上报接口，给PS使用(替换原来的DIAG_ReportLayerMessageLog)
 输入参数  : pMsg(标准的VOS消息体，源模块、目的模块信息从消息体中获取)
*****************************************************************************/
void mdrv_diag_trace_report(void *pMsg);

/*****************************************************************************
 Function Name   : mdrv_diag_get_conn_state
 Description        : 获取当前工具连接状态
 Return              : 1:connect; 0:disconnect
*****************************************************************************/
unsigned int mdrv_diag_get_conn_state(void);

/*****************************************************************************
 函 数 名     : Log_GetPrintLevel
 功能描述  : 得到模块Id、子模块Id在打印级别记录表中的索引号
 return               : mdrv_diag_log_level_e
*****************************************************************************/
unsigned int mdrv_diag_get_log_level(unsigned int pid);
/*****************************************************************************
 函 数 名  : mdrv_diag_rtt_report	1344
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
unsigned int mdrv_diag_rtt_report(void *pData, unsigned int ulDatalen);
/*****************************************************************************
 函 数 名     : mdrv_diag_dt_report
 功能描述  : 路测数据上报扩展接口
 输入参数  : mdrv_diag_dt_ind_s->ulModule( 31-24:modemid,23-16:modeid,11-8:groupid )
                          mdrv_diag_dt_ind_s->ulMsgId(透传命令ID)
                          mdrv_diag_dt_ind_s->ulLength(透传信息的长度)
                          mdrv_diag_dt_ind_s->pData(透传信息)
*****************************************************************************/
unsigned int mdrv_diag_dt_report(mdrv_diag_dt_ind_s *pstData);
/*****************************************************************************
 函 数 名     : mdrv_diag_dt_report
 功能描述  : 提供给Modem组件，用于通知组件当前工具连接断开状态变化；
 输入参数  : 1. DIAG_ConnStateNotifyFun函数会在DIAG和工具连接断开过程中调用，其中不能有任何阻塞性操作;
                          2. 不允许反复注册,单个组件只允许注册一次
*****************************************************************************/
unsigned int mdrv_diag_conn_state_notify_fun_reg(mdrv_diag_conn_state_notify_fun pfun);

#ifdef __cplusplus
    }
#endif
#endif
