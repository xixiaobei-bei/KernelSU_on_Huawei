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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "TafAppXsmsInterface.h"
#include "MnClient.h"




#define THIS_FILE_ID                    PS_FILE_ID_TAF_XSMS_API_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数定义
*****************************************************************************/
/*****************************************************************************
函 数 名  : TAF_XSMS_GetReceiverPid
功能描述  : 从Client ID到接收PID的转换
输入参数  : ClientId -- AT客户ID
输出参数  : pulReceiverPid -- 接收PID
返 回 值  : VOS_OK/VOS_ERR
调用函数  : 无
被调函数  :

修改历史      :
 1.日    期   : 2014年10月31日
   作    者   : h00300778
   修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 TAF_XSMS_GetReceiverPid(MN_CLIENT_ID_T  ClientId, VOS_UINT32 *pulReceiverPid)
{
    MODEM_ID_ENUM_UINT16    enModemID;

    /* 调用接口获取Modem ID */
    if (VOS_OK != AT_GetModemIdFromClient(ClientId, &enModemID))
    {
        return VOS_ERR;
    }

    if (MODEM_ID_1 == enModemID)
    {
        *pulReceiverPid = I1_UEPS_PID_XSMS;
    }
    else if (MODEM_ID_2 == enModemID)
    {
        *pulReceiverPid = I2_UEPS_PID_XSMS;
    }
    else
    {
        *pulReceiverPid = I0_UEPS_PID_XSMS;
    }

    return VOS_OK;
}


/*****************************************************************************
 函 数 名  : TAF_XSMS_SendSmsReq
 功能描述  : APP的短信发送请求
 输入参数  : usClientId:发起该请求的Client的ID
             ucOpId:标识本次操作
             enSndOption:发送选项
             pucData:要发送的短信内容
 输出参数  : 无
 返 回 值  : VOS_OK -- 消息发送成功
             VOS_ERR -- 消息发送成功
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年10月31日
    作    者   : h00300778
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 TAF_XSMS_SendSmsReq(
    MN_CLIENT_ID_T                      usClientId,
    MN_OPERATION_ID_T                   ucOpId,
    TAF_XSMS_SEND_OPTION_ENUM_UINT8     enSndOption,
    VOS_UINT8                          *pucData)
{
    return VOS_ERR;
}

/*****************************************************************************
 函 数 名  : TAF_XSMS_SetXsmsApMemFullReq
 功能描述  : 通知短信模块 AP侧短信存储空间是不是满了 0 满 1 可用
 输入参数  : usClientId:发起该请求的Client的ID
             ucOpId:标识本次操作
             ucApMemFullFlag:短信存储空间是不是满了 0 满 1 可用s
 输出参数  : 无
 返 回 值  : VOS_OK -- 消息发送成功
             VOS_ERR -- 消息发送成功
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年06月02日
    作    者   : c00299064
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 TAF_XSMS_SetXsmsApMemFullReq(
    MN_CLIENT_ID_T                      usClientId,
    MN_OPERATION_ID_T                   ucOpId,
    VOS_UINT8                           ucApMemFullFlag)
{
    return VOS_OK;
}


/*****************************************************************************
 函 数 名  : TAF_XSMS_WriteSmsReq
 功能描述  : APP的短信写入请求
 输入参数  : usClientId:发起该请求的Client的ID
             ucOpId:标识本次操作
             enStatus:短信状态
             pucData:要发送的短信内容
 输出参数  : 无
 返 回 值  : VOS_OK -- 消息发送成功
             VOS_ERR -- 消息发送成功
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年10月31日
    作    者   : h00300778
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 TAF_XSMS_WriteSmsReq(
    MN_CLIENT_ID_T                      usClientId,
    MN_OPERATION_ID_T                   ucOpId,
    TAF_XSMS_STATUS_ENUM_UINT8          enStatus,
    VOS_UINT8                          *pucData)
{
    return VOS_ERR;
}

/*****************************************************************************
 函 数 名  : TAF_XSMS_DeleteSmsReq
 功能描述  : APP的短信删除请求
 输入参数  : usClientId:发起该请求的Client的ID
             ucOpId:标识本次操作
             ucIndex:删除的索引号
 输出参数  : 无
 返 回 值  : VOS_OK -- 消息发送成功
             VOS_ERR -- 消息发送成功
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年10月31日
    作    者   : h00300778
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 TAF_XSMS_DeleteSmsReq(
    MN_CLIENT_ID_T                      usClientId,
    MN_OPERATION_ID_T                   ucOpId,
    VOS_UINT8                           ucIndex)
{
    return VOS_ERR;
}




