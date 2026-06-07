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
#include  "CsdDebug.h"



#if( FEATURE_ON == FEATURE_CSD )

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
CSD_UL_STATUS_INFO_STRU                 g_stCsdStatusInfo = {0};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

VOS_VOID CSD_ShowULStatus(VOS_VOID)
{
    PS_PRINTF("CSD收到上行数据的个数                             %d\n",g_stCsdStatusInfo.ulULRecvPktNum);
    PS_PRINTF("CSD上行缓存中数据包的个数                         %d\n",g_stCsdStatusInfo.ulULSaveBuffPktNum);
    PS_PRINTF("CSD上行入队失败的次数                             %d\n",g_stCsdStatusInfo.ulULEnQueFail);
    PS_PRINTF("CSD发送上行缓存包数量                             %d\n",g_stCsdStatusInfo.ulULSendPktNum);
    PS_PRINTF("CSD发送上行数据时从队列中获取到空指针包数目       %d\n",g_stCsdStatusInfo.ulULQueNullNum);
    PS_PRINTF("CSD发送上行数据sk_buffer头转换到IMM头失败的包数目 %d\n",g_stCsdStatusInfo.ulULZcToImmFailNum);
    PS_PRINTF("CSD发送上行数据插入DICC通道失败的包数目           %d\n",g_stCsdStatusInfo.ulULInsertDiccFailNum);
    PS_PRINTF("\r\n");

    return;
}

VOS_VOID CSD_ShowDLStatus(VOS_VOID)
{
    PS_PRINTF("CSD收到下行数据的个数                             %d\n",g_stCsdStatusInfo.ulDLRecvPktNum);
    PS_PRINTF("CSD发送下行缓存包数量                             %d\n",g_stCsdStatusInfo.ulDLSendPktNum);
    PS_PRINTF("CSD下行发送失败包的数目                           %d\n",g_stCsdStatusInfo.ulDLSendFailNum);
    PS_PRINTF("\r\n");

    return;
}

VOS_VOID CSD_Help(VOS_VOID)
{
    PS_PRINTF("********************CSD软调信息************************\n");
    PS_PRINTF("CSD_ShowULStatus                         显示CSD上行统计信息\n");
    PS_PRINTF("CSD_ShowDLStatus                         显示CSD下行统计信息\n");

    return;
}









#endif

