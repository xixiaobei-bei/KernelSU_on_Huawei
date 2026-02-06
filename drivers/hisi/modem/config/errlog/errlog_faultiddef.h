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

 /******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : errlog_faultiddef.h
  版 本 号   : 初稿
  生成日期   : 2016年3月7日
  最近修改   :
  功能描述   : errlog_faultiddef.h头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2016年3月7日
    修改内容   : 创建文件

******************************************************************************/
#ifndef __ERRLOG_CFG_FAULT_ID_DEF_H__
#define __ERRLOG_CFG_FAULT_ID_DEF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 ???
*****************************************************************************/


/*****************************************************************************
  3 ????
*****************************************************************************/

/*****************************************************************************
 ???    : ERR_LOG_FAULT_ID_ENUM
 ????  : FAULT ID

  1.?    ?   : 2016?02?17?
    ????   : ERR LOG FAULT ID????????
*****************************************************************************/
enum ERR_LOG_FAULT_ID_ENUM
{
    /* TODO: python search flag satrt */
    FAULT_ID_GU_ERR_LOG_REPT                = 0X00, /* GU ErrLog ???? */
    /* Added for EMC_CHR, 2018-2-11, begin */
    FAULT_ID_EMC_DOMAIN_SEL_AND_RETRY       = 0x78,  /* ???????? */
    /* Added for EMC_CHR, 2018-2-11, end */

    FAULT_ID_1X_OOS                         = 0XA1, /* 1x?? */
    FAULT_ID_1X_MO_CALL_FAIL                = 0XA2, /* 1x???? */
    FAULT_ID_1X_MT_CALL_FAIL                = 0XA3, /* 1x???? */
    FAULT_ID_1X_CALL_EXCEPTION              = 0XA4, /* 1x?? */
    FAULT_ID_1X_REG_FAIL                    = 0XA5, /* ???? */
    FAULT_ID_1X_SHORT_MSG_SEND_FAIL         = 0XA6, /* ?????? */
    FAULT_ID_1X_SHORT_MSG_RCV_FAIL          = 0XA7, /* ?????? */
    FAULT_ID_1X_OOS_RECOVERY                = 0XA8, /* 1x???? */
    FAULT_ID_1X_QPCH_CHANGE                 = 0xA9, /* QPCH???? */
    FAULT_ID_1X_VOICE_QUALITY_BAD           = 0XB1, /* ????? */
    FAULT_ID_AP_GET_PHY_COUNT_DATA_REQ      = 0XB2, /* AP????????????? */
    FAULT_ID_1X_COUNT_DATA_REPORT           = 0XB3, /* CSDR 1X???????? */
    /* Added for ABNORMAL CHR, 2018-02-02, begin */
    FAULT_ID_1X_WAKESLEEP_REPORT            = 0XB4, /* MODEM功耗异常 */
    FAULT_ID_1X_ABNORMAL_REPORT             = 0XB5, /* 1x异常主动上报 */
    /* Added for ABNORMAL CHR, 2018-02-02, end */
    /* Modified for CHR_PHASE_II,2015-04-07,begin */
    FAULT_ID_HRPD_OR_LTE_OOS                = 0XC1, /* CL模式下HRPD或LTE丢网*/
    /* Modified for CHR_PHASE_II,2015-04-07,end */
    FAULT_ID_HRPD_PS_CALL_EXCEPTION         = 0XC2, /* PS呼叫失败 */
    FAULT_ID_HRPD_PS_SESSION_EXCEPTION      = 0XC3, /* PS会话异常 */
    FAULT_ID_HRPD_PS_DISC_EXCEPTION         = 0XC4, /* PS断链 */
    FAULT_ID_CL_OOS_SEARCH                  = 0XC5, /* CL多模搜网过程 */
    /* Modified for CHR_PHASE_II,2015-04-07,begin */
    FAULT_ID_HRPD_OR_LTE_OOS_RECOVERY       = 0XC6, /* CL模式下HRPD或LTE掉网恢复 */
    /* Modified for CHR_PHASE_II,2015-04-07,end */
    /* Modified for CHR NEW REQ,2015-04-07,begin */
    FAULT_ID_HRPD_EXCEPTION_COLLECTION      = 0XC7, /* 通用异常收集 */
    /* Modified for CHR NEW REQ,,2015-04-07,end */

    /* Added for CDMA No_Back_4G, 2016-06-01, begin */
    FAULT_ID_CL_LEAVE_LTE                   = 0xC8, /* CL模式下长时间不回4G */
    /* Added for CDMA No_Back_4G, 2016-06-01, end */

    /* Added for CDMA VoLTE, 2016-8-11, begin */
    FAULT_ID_VOLTE_IMS_1X_SWITCH            = 0xCB, /* 电信VOLTE下， ims<->1x乒乓切换主动上报 */
    /* Added for CDMA VoLTE, 2016-8-11, end */

    /* Added for DSDS Modem Control, 2017-2-13, begin */
    FAULT_ID_MODEM_RESTART_ABORT_SERVICE      = 0xCC,  /* modem重启导致业务释放主动上报，具体FAULT_ID需要与AP确认 */
    /* Added for DSDS Modem Control, 2017-2-13, end */

    /* Added for CHR需求变更, 2017-3-27, begin */
    FAULT_ID_CL_MULTI_RAT_MODE_SWITCH       = 0xCD,  /* CL下各个驻留模式异常切换 */
    FAULT_ID_GUL_CL_MODE_SWITCH             = 0xCE,  /* GUL/CL乒乓切换*/
    /* Added for CHR需求变更, 2017-3-27, end */

    FAULT_ID_CL_1X_RESEL                    = 0xCF,  /* 1X重选*/

    /* Added for CNAS REQ PACK, 2017-9-13, begin */
    FAULT_ID_ENC_VOICE_FAIL                = 0xD0,  /* 加密语音失败 */
    /* Added for CNAS REQ PACK, 2017-9-13, end */

    /* Added for LOCK optimization, 2018-3-22 begin */
    FAULT_ID_LOCK_COUNT                     = 0xD1, /* lock related */
    /* Added for LOCK optimization, 2018-3-22 end */

    /* Added for ClVolte_CHR 2018-5-11 begin */
    FAULT_ID_CL_IMS_INFO                    = 0XD2, /* ??CL???,ims?????? */
    /* Added for ClVolte_CHR 2018-5-11 end */

    /* Added for EHRPD_PDN_CHR, 2018-7-3, begin */
    FAULT_ID_EHRPD_HRPD_INFO                = 0XD3, /* EHRPD?PDN???? */
    /* Added for EHRPD_PDN_CHR, 2018-7-3, end */

    /* Added for 5G Iteration 25, 2018-10-29, begin */
    FAULT_ID_NRMM_REG_FAILURE_MAX_TIMES     = 0XD4,
    /* Added for 5G Iteration 25, 2018-10-29, end */

    /* TODO: python search flag end */
    FAULT_ID_ERR_LOG_ENUM_BUT
};


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/


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

#endif
