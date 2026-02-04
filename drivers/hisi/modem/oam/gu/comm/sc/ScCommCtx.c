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
  文 件 名   : ScCommCtx.c
  版 本 号   : 初稿
  作    者   : d00212987
  生成日期   : 2015年08月10日
  最近修改   :
  功能描述   : SC CONTEXT文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年08月10日
    作    者   : d00212987
    修改内容   : 创建文件

******************************************************************************/

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "ScCommCtx.h"


#define THIS_FILE_ID PS_FILE_ID_SC_CONTEXT_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
 全局变量名    : g_stScCtx
 全局变量说明  : SC运行上下文
 1.日    期   : 2012年04月19日
   作    者   : w00184875
   修改内容   : 新建
*****************************************************************************/
static SC_CONTEXT_STRU                  g_stScCtx = {
    /* SC_COMM_GLOBAL_VAR */
    {
        {
            "/mnvm2:0/SC/Pers/CKFile.bin",
            "/mnvm2:0/SC/Pers/DKFile.bin",
            "/mnvm2:0/SC/Pers/AKFile.bin",
            "/mnvm2:0/SC/Pers/PIFile.bin",
            "/mnvm2:0/SC/Pers/ImeiFile_I0.bin",
            "/mnvm2:0/SC/Pers/ImeiFile_I1.bin",
            "/mnvm2:0/SC/Pers/ImeiFile_I2.bin",
        },
        {
            "/mnvm2:0/SC/Pers/CKSign.hash",
            "/mnvm2:0/SC/Pers/DKSign.hash",
            "/mnvm2:0/SC/Pers/AKSign.hash",
            "/mnvm2:0/SC/Pers/PISign.hash",
            "/mnvm2:0/SC/Pers/ImeiFile_I0.hash",
            "/mnvm2:0/SC/Pers/ImeiFile_I1.hash",
            "/mnvm2:0/SC/Pers/ImeiFile_I2.hash",
        }
    },
    /* SC_FAC_GLOBAL_VAR */
    {
        SC_AUTH_STATUS_UNDO,
        "",
        VOS_FALSE,
        ""
    },
    /* SC_APSEC_GLOBAL_VAR */
    {
        0,
        {0, 0},
        0,
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        SC_SECURITY_FILE_STATE_IDEL,
        {
            {0,VOS_NULL_PTR},{0,VOS_NULL_PTR},{0,VOS_NULL_PTR},{0,VOS_NULL_PTR},{0,VOS_NULL_PTR},
            {0,VOS_NULL_PTR},{0,VOS_NULL_PTR},{0,VOS_NULL_PTR},{0,VOS_NULL_PTR},{0,VOS_NULL_PTR}
        },
        "/mnvm2:0/SC/Apsec/SecureDataA.bin",
        "/mnvm2:0/SC/Apsec/SecureDataC.bin"
    }
};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

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
SC_CONTEXT_STRU* SC_CTX_GetScCtxAddr(VOS_VOID)
{
    return &(g_stScCtx);
}

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
SC_COMM_GLOBAL_STRU*  SC_CTX_GetCommGlobalVarAddr( VOS_VOID )
{
    return &g_stScCtx.stScCommGlobalVar;
}


