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

                  版权所有 (C), 2001-2012, 华为技术有限公司

 ******************************************************************************
  文 件 名   : ScCtx.h
  版 本 号   : 初稿
  作    者   : w00184875
  生成日期   : 2012年04月19日
  最近修改   :
  功能描述   : ScCtx.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2012年04月19日
    作    者   : w00184875
    修改内容   : 创建文件

******************************************************************************/

#ifndef __SCCOMMCTX_H__
#define __SCCOMMCTX_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "ScComm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define     SC_RAND_DATA_LEN            (4)         /* 随机数的字节数 */

#define     SC_SEC_AT_KEY_LEN           (16)                /* 安全AT key的长度 */

#define     SC_SECURITY_ITEM_NUM        (16)                /* 安全存储项个数 */


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
 枚举名    : SC_AUTH_STATUS_ENUM
 枚举说明  : 指定鉴权状态

  1.日    期   : 2012年04月07日
    作    者   : w00184875
    修改内容   : AP-Modem锁网锁卡项目新增枚举
*****************************************************************************/
enum SC_AUTH_STATUS_ENUM
{
    SC_AUTH_STATUS_UNDO          = 0x00,             /* 未进行鉴权 */
    SC_AUTH_STATUS_DONE          = 0x01,             /* 已经鉴权 */
    SC_AUTH_STATUS_BUTT
};
typedef VOS_UINT8 SC_AUTH_STATUS_ENUM_UINT8;

/*****************************************************************************
 枚举名    : SC_APSEC_FILE_STATE_INIT_ENUM
 枚举说明  : 安全存储文件初始化状态

  1.日    期   : 2012年8月27日
    作    者   : h59254
    修改内容   : V7R1C50 ISDB安全存储项目新增
*****************************************************************************/
enum SC_APSEC_FILE_STATE_INIT_ENUM
{
    SC_SECURITY_FILE_STATE_INIT         =   0x5A5A5A5A,     /* 安全存储文件初始化完成 */
    SC_SECURITY_FILE_STATE_IDEL         =   0x5A5A55AA,     /* 安全存储文件初始化未完成 */
    SC_SECURITY_FILE_STATE_BUTT,
};
typedef VOS_UINT32 SC_APSEC_FILE_STATE_INIT_ENUM_UINT32;

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
 结构名    : SC_COMM_GLOBAL_STRU
 结构说明  : SC COMM模块运行时全局变量
 1.日    期   : 2012年04月19日
   作    者   : w00184875
   修改内容   : 新建

*****************************************************************************/
typedef struct
{
    VOS_CHAR                           *acSecretFileName[SC_SECRET_FILE_TYPE_BUTT];
    VOS_CHAR                           *acSignFileName[SC_SECRET_FILE_TYPE_BUTT];
}SC_COMM_GLOBAL_STRU;

/*****************************************************************************
结构名    : SC_FAC_GLOBAL_STRU
结构说明  : 涉及到产线流程的全局变量

  1.日    期   : 2012年04月16日
    作    者   : w00184875
    修改内容   : 初始生成
*****************************************************************************/
typedef struct
{
    SC_AUTH_STATUS_ENUM_UINT8               enScFacAuthStatus;
    VOS_UINT8                               aucScFacRandData[SC_RAND_DATA_LEN];
    /* 发起产线鉴权标志 VOS_FALSE: 未发起产线鉴权; VOS_TRUE: 已发起产线鉴权 */
    VOS_UINT8                               ucIdentifyStartFlg;
    VOS_UINT8                               aucReserved[2];         /* 保留字节，用于四字节对齐 */
}SC_FAC_GLOBAL_STRU;

/*****************************************************************************
 结构名    : SC_APSEC_ITEM_STRU
 枚举说明  : 安全存储项在内存中保存的全局变量

  1.日    期   : 2012年8月27日
    作    者   : h59254
    修改内容   : V7R1C50 ISDB安全存储项目新增
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulItemLen;
    VOS_CHAR                           *pucContent;
    VOS_UINT8                           aucHash[SC_HASH_LEN];
}SC_APSEC_ITEM_STRU;

/* Added by h59254 for V7R1C50 ISDB Project, 2012/08/28, begin */
/*****************************************************************************
 结构名    : SC_APSEC_GLOBAL_STRU
 枚举说明  : 安全存储项在内存中保存的全局变量

  1.日    期   : 2012年8月27日
    作    者   : h59254
    修改内容   : V7R1C50 ISDB安全存储项目新增
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                               ucSeqID;
    VOS_UINT8                                               aucReserved[2];
    VOS_UINT8                                               ucSecATkeyFlg;
    VOS_UINT8                                               aucSecATkey[SC_SEC_AT_KEY_LEN];
    SC_APSEC_FILE_STATE_INIT_ENUM_UINT32                    ulInitState;
    SC_APSEC_ITEM_STRU                                      astApSecItem[SC_SECURITY_ITEM_NUM];
    VOS_CHAR                                               *pApSecFileA;
    VOS_CHAR                                               *pApSecFileC;
}SC_APSEC_GLOBAL_STRU;
/* Added by h59254 for V7R1C50 ISDB Project, 2012/08/28, end */

/*****************************************************************************
 结构名    : SC_CONTEXT_STRU
 结构说明  : SC 模块运行上下文
 1.日    期   : 2012年04月19日
   作    者   : w00184875
   修改内容   : 新建

*****************************************************************************/
typedef struct
{
    SC_COMM_GLOBAL_STRU                 stScCommGlobalVar;
    SC_FAC_GLOBAL_STRU                  stScFacGlobalVar;
    /* Added by h59254 for V7R1C50 ISDB Project, 2012/08/28, begin */
    SC_APSEC_GLOBAL_STRU                stScApSecGlobalVar;
    /* Added by h59254 for V7R1C50 ISDB Project, 2012/08/28, end */
}SC_CONTEXT_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

/*****************************************************************************
 函 数 名  : SC_CTX_GetCommGlobalVarAddr
 功能描述  : 获取SC公共操作的全局变量地址
 输入参数  : 无
 输出参数  : 无
 返 回 值  : SC公共操作的全局变量上下文指针
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年04月19日
    作    者   : w00184875
    修改内容   : 新生成函数
*****************************************************************************/
extern SC_COMM_GLOBAL_STRU*  SC_CTX_GetCommGlobalVarAddr( VOS_VOID );

/*****************************************************************************
 函 数 名  : SC_CTX_GetScCtxAddr
 功能描述  : 获取当前SC的CTX
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 返回当前SC的CTX地址
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2012年04月20日
   作    者   : w00184875
   修改内容   : 新生成函数

*****************************************************************************/
extern SC_CONTEXT_STRU* SC_CTX_GetScCtxAddr(VOS_VOID);

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
