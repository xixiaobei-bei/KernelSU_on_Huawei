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


#include "product_config.h"
#include "omerrorlog.h"
#include "blist.h"
#include "mdrv_sysboot.h"
#include "mdrv.h"

/*优先级包数开始结束*/
#define   PRIORITY_PACKET_START            (0)
#define   PRIORITY_PACKET_END              (0x7F)



/*周期配置包数开始结束*/
#define   PERIOD_PACKET_START            (0)
#define   PERIOD_PACKET_END              (0x7F)

#define   PERIOD_CHK_FAIL            (3)
#define   PERIOD_CHK_COMPLETE        (0)
#define   PERIOD_CHK_CONTINUE        (1)
#define   PERIOD_CHK_RESTART         (2)

/*a核CHR 任务收到的消息ID名*/
enum CHR_ID_REQ_MSG_ENUM
{ 
    CHR_ID_RESET_CCORE        = 0x1001,
    CHR_ID_ERR_LOG_REQ        = 0x1002,
    CHR_ID_BLACKLIST_REQ      = 0x1003,
    CHR_ID_PRIORITY_REQ       = 0x1004,
    CHR_ID_PERIOD_REQ         = 0x1005,
    
    CHR_ID_REQ_BUTT 
   
};

//#define chr_print(fmt, ...)    (printk(KERN_ERR "[chr]:<%s> line = %d, "fmt, __FUNCTION__, __LINE__ ,##__VA_ARGS__))
#define chr_print(fmt, ...)    (mdrv_err("<%s>"fmt, __FUNCTION__, ##__VA_ARGS__))

/*保存黑名单的结构体*/
typedef struct
{
    VOS_UINT32          ulBlackListPacketLen;
    CHR_LIST_INFO_S    *pstChrBlackList;
    
}CHR_ACPU_BLACK_SAVE_STRU;


/*OM a核发给c的黑名单结构体*/
typedef struct
{   
    VOS_MSG_HEADER 
    VOS_UINT32                   ulMsgName;
    VOS_UINT32                   ulPacketLen;
    CHR_LIST_INFO_S              stOmAcpuBlackList[0];        
}OM_ACPU_BLACK_LIST_STRU;


/*保存优先级0的结构体*/
typedef struct
{
    VOS_UINT32                  ulPiorityLen;
    CHR_PRIORITY_INFO_S         PriorityCfg[0];
}CHR_ACPU_PRIORITY_SAVE_STRU;


/*OM a核发给c的优先级0的列表的结构体*/
typedef struct
{   
    VOS_MSG_HEADER 
    VOS_UINT32                     ulMsgName;
    VOS_UINT32                     ulPacketLen;
    CHR_PRIORITY_INFO_S            PriorityCfg[0];        
}OM_ACPU_PRIORITY_CFG_STRU;


/*保存上报周期的结构体*/
typedef struct
{
    VOS_UINT32                      ulperiod; /*ap下发的period是8bit*/
    VOS_UINT32                      ulPacketLen;
    CHR_PERIOD_CFG_STRU             PeriodCfg[0];
}CHR_ACPU_PERIOD_SAVE_STRU;



/*优先级链表结构体*/
typedef struct
{
    LIST_S                     PriorityList;
    VOS_UINT32                 ulPacketLen;
    CHR_PRIORITY_INFO_S        pstPriorityCfg[0];
}OM_PRIORITY_NODE_STRU;

typedef struct
{
    VOS_UINT32              ulSN;
    VOS_UINT32              ulPrioPacketLen;
    OM_PRIORITY_NODE_STRU   List;
}OM_PRIORITY_LIST_STRU;

/*上报周期链表结构体*/

typedef struct
{  
    LIST_S                  PeriodList;
    VOS_UINT32              ulperiod;
    VOS_UINT32              ulPacketLen;
    CHR_PERIOD_CFG_STRU     pstPeriodCfg[0];
}OM_PERIOD_NODE_STRU;

typedef struct
{
    VOS_UINT32              ulSN;
    VOS_UINT32              ulPeriodPacketLen;
    OM_PERIOD_NODE_STRU     List;
}OM_PERIOD_LIST_STRU;


/*OM a核发给c的周期配置列表*/
typedef struct
{   
    VOS_MSG_HEADER
    VOS_UINT32                    ulMsgName;
    VOS_UINT32                    ulperiod;
    VOS_UINT32                    ulPacketLen;      
    CHR_PERIOD_CFG_STRU           stOmAcpuPeriodCfg[0];  
    
}OM_ACPU_PERIOD_CFG_STRU;

VOS_UINT32 OM_AcpuBlackListProc(VOS_UINT8 *pucData, VOS_UINT32 ulLen);
VOS_UINT32 OM_AcpuPriorityCfgProc(VOS_UINT8 *pucData, VOS_UINT32 ulLen);
VOS_UINT32 OM_AcpuPeriodCfgProc(VOS_UINT8 *pucData, VOS_UINT32 ulLen);
VOS_INT    chr_ResetCcoreCB(DRV_RESET_CB_MOMENT_E enParam, int userdata);
VOS_UINT32 OM_AcpuResetProc(VOS_VOID);

VOS_UINT32 CHR_Cfg_Init(VOS_VOID);
VOS_VOID OM_AcpuSendAppcfgResult(VOS_UINT32 ulRest);

