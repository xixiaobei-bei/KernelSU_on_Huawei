/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#include "vos.h"
#include "mdrv.h"
#include "mdrv_udi.h"
#include "Voice_agent_Main.h"
#include "PsTypeDef.h"
#include "Voice_agent_public.h"
#include <linux/kernel.h>
#include "Voice_agent_msg_proc.h"



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID                    PS_FILE_ID_DMS_CORE_C
/*****************************************************************************
 函 数 名  : HIFI_ACORE_MsgProc
 功能描述  : AT模块消息处理入口函数
 输入参数  : MsgBlock* pMsg
 输出参数  : 无
 返 回 值  : TAF_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.Date        : 2018/10/10
    Modification: Created function

*****************************************************************************/
static VOS_VOID VOICE_AGENT_MsgProc(MsgBlock* pMsg)
{
	return;
}
/*****************************************************************************
 函 数 名  : HIFI_ACORE_PidInit
 功能描述  : HIFI A AGENT PID初始化函数
 输入参数  : enum VOS_INIT_PHASE_DEFINE enPhase
 输出参数  : 无
 返 回 值  : VOS_UINT32
             VOS_OK
             VOS_ERR
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年09月10日
    修改内容   : 新生成函数
*****************************************************************************/
static VOS_UINT32  VOICE_AGENT_PidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
	pr_err("VOICE_AGENT_PidInit enter\n");

    switch ( enPhase )
    {
        case VOS_IP_INITIAL:
            break;

        default:
            break;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : HIFI_AcoreFidInit
 功能描述  : HIFI FID 初始化函数
 输入参数  :

 输出参数  :
 返 回 值  :
 调用函数  :
 被调函数  :
 修改历史  :
   1.日    期  : 2018年9月06日
     修改内容  : Creat Function
*****************************************************************************/
VOS_UINT32 VOICE_AGENT_FidInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32                          ulRelVal = 0;

    pr_err("VOICE_AGENT_FidInit enter\n");

    switch (ip)
    {
    case VOS_IP_LOAD_CONFIG:

        ulRelVal = VOS_RegisterPIDInfo(ACPU_PID_VOICE_AGENT, (Init_Fun_Type) VOICE_AGENT_PidInit, (Msg_Fun_Type) VOICE_AGENT_MsgProc);
        if (ulRelVal != VOS_OK)
        {
            VOICEAGENT_ERR_LOG("VOICE_AGENT_FidInit register pid error");
            return VOS_ERR;
        }

        ulRelVal = VOS_RegisterTaskPrio(ACPU_FID_VOICE_AGENT, VOS_PRIORITY_M5);
        if (ulRelVal != VOS_OK)
        {
            VOICEAGENT_ERR_LOG("VOICE_AGENT_FidInit register pid prio error");
            return VOS_ERR;
        }

        break;

    default:
        break;
    }

    return VOS_OK;
}




