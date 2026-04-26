/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
/*
 * 1 Include HeadFile
 */

#include "ndserver_entity.h"
#include "securec.h"
#include "ps_tag.h"
#include "ps_type_def.h"
#include "ip_ipm_global.h"
#include "ndis_entity.h"
#include "ndserver_debug.h"
#include "ip_nd_server.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERENTITY_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */

/* ND SERVER本地保存的配置参数 */
VOS_UINT8  g_ndServerMFlag;          /* M标识 */
VOS_UINT8  g_ndServerOFlag;          /* O标识 */
VOS_UINT16 g_routerLifetime; /* Router Lifetime */
VOS_UINT32 g_reachableTime;  /* Reachable Time */
VOS_UINT32 g_retransTimer;   /* Retrans Timer */

VOS_UINT32 g_nsTimerLen;         /* 非周期性NS定时器时长 */
VOS_UINT32 g_nsTimerMaxExpNum;   /* 非周期性NS最大超时次数 */
VOS_UINT32 g_periodicNsTimerLen; /* 周期性NS定时器时长 */
VOS_UINT32 g_periodicRaTimerLen; /* 周期性RA定时器时长，防止Router过期 */
VOS_UINT32 g_firstNsTimerLen;    /* 收到重复地址检测后等待的定时器时长 */
VOS_UINT32 g_raTimerLen;         /* 收到重复地址检测前周期发送RA定时器时长 */


/* 周期性发送RA时间计数器 */
VOS_UINT32 g_periodicRaTimeCnt[IP_NDSERVER_ADDRINFO_MAX_NUM];

/*
 * Description: 设置本地网络参数
 * History:
 *    1.      2011-04-01  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_SetLocalParam(VOS_VOID)
{
    /* M标识，0，主机不能通过DHCPv6获取IPv6地址 */
    g_ndServerMFlag = 0;

    /* O标识，1，主机可以通过无状态DHCPv6获取其他参数，例如DNS服务器，SIP服务器； */
    g_ndServerOFlag = 1;

    /* Router Lifetime(秒) */
    g_routerLifetime = 9000; /* 路由生存期9000s */

    /* 3600000:Reachable Time(毫秒) */
    g_reachableTime = 3600000;

    /* Retrans Timer(毫秒)，0，表示未知 */
    g_retransTimer = 0;

    /* 4000:非周期性NS定时器时长(毫秒) */
    g_nsTimerLen = 4000;

    /* 3:非周期性NS最大超时次数 */
    g_nsTimerMaxExpNum = 3;

    /* 60000:周期性NS定时器时长(毫秒) */
    g_periodicNsTimerLen = 60000;

    /* 3600000:周期性RA，防止Router过期(毫秒) */
    g_periodicRaTimerLen = 3600000;

    /* 2000:收到重复地址检测后等待的定时器时长(毫秒) */
    g_firstNsTimerLen = 2000;

    /* 15000:收到重复地址检测前周期发送RA的定时器时长(毫秒) */
    g_raTimerLen = 15000;

    return;
}

/*
 * Description: ND SERVER模块的初始化
 * History: VOS_VOID
 *    1.        2011-04-01  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_Init(VOS_VOID)
{
    VOS_UINT32 addrIndex;
    VOS_UINT16 payLoad = IP_IPV6_PAYLOAD;
    errno_t    rlt;

    IP_NDSERVER_SetLocalParam();
    IP_NDSERVER_DebugInit();

    for (addrIndex = 0; addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM; addrIndex++) {
        (VOS_VOID)memset_s(IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex), sizeof(IP_NdserverAddrInfo), IP_NULL,
                           sizeof(IP_NdserverAddrInfo));

        rlt = memcpy_s(IP_NDSERVER_ADDRINFO_GET_MACADDR(addrIndex), IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(),
                       IP_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        rlt = memcpy_s((IP_NDSERVER_ADDRINFO_GET_MACFRAME(addrIndex) + IP_MAC_ADDR_LEN),
                       (IP_ETH_MAC_HEADER_LEN - IP_MAC_ADDR_LEN), (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        /* payload长度为2个字节 */
        rlt = memcpy_s((IP_NDSERVER_ADDRINFO_GET_MACFRAME(addrIndex) + 2 * IP_MAC_ADDR_LEN),
                       (IP_ETH_MAC_HEADER_LEN - 2 * IP_MAC_ADDR_LEN), (VOS_UINT8 *)(&payLoad), 2); /* 2:byte */
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        g_periodicRaTimeCnt[addrIndex] = g_periodicRaTimerLen / g_periodicNsTimerLen;
        /* 创建下行IP包缓存队列 */
        rlt = LUP_CreateQue(NDIS_NDSERVER_PID, &(IP_NDSERVER_ADDRINFO_GET_DLPKTQUE(addrIndex)),
                            ND_IPV6_WAIT_ADDR_RSLT_Q_LEN);
        if (rlt != PS_SUCC) {
            PS_PRINTF_ERR("IP_NDSERVER_Init, LUP_CreateQue DlPktQue fail.\n");
            return;
        }
    }

    (VOS_VOID)memset_s(&g_ndServerTeDetectBuf, sizeof(IP_NdserverTeDetectBuf), IP_NULL,
                       sizeof(IP_NdserverTeDetectBuf));

    g_ndServerTeDetectBuf.head   = 0;
    g_ndServerTeDetectBuf.tail   = 0;
    g_ndServerTeDetectBuf.maxNum = 2 * IP_NDSERVER_ADDRINFO_MAX_NUM; /* 2:buf */

    return;
}
/*
 * 功能描述: ND SERVER PID初始化函数
 * 修改历史:
 *  1.日    期: 2011年12月7日
 *    修改内容: 新生成函数
 */
VOS_UINT32 APP_NdServer_Pid_InitFunc(enum VOS_InitPhaseDefine phase)
{
    switch (phase) {
        case VOS_IP_LOAD_CONFIG:
            IP_NDSERVER_Init();
            break;
        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
        case VOS_IP_BUTT:
            break;
        default:
            break;
    }
    return 0;
}


/*
 * 功能描述: 根据ExRabId查找NDSERVER实体的索引
 * 修改历史:
 *  1.日    期: 2013年1月15日
 *    修改内容: 新生成函数
 */
VOS_UINT32 NdSer_GetAddrInfoIdx(VOS_UINT8 exRabId)
{
    VOS_UINT32 i = 0;

    /* 查询是否已存在相应Entity */
    do {
        if ((PS_TRUE == g_ndServerAddrInfo[i].validFlag) && (exRabId == g_ndServerAddrInfo[i].epsbId)) {
            /* 找到相应实体 */
            return i;
        }
    } while ((++i) < IP_NDSERVER_ADDRINFO_MAX_NUM);

    return IP_NDSERVER_ADDRINFO_MAX_NUM;
}

/*
 * 功能描述: 分配一个空闲的NDSERVER实体
 * 修改历史:
 *  1.日    期: 2013年1月15日
 *    修改内容: 新生成函数
 */
IP_NdserverAddrInfo *NdSer_AllocAddrInfo(VOS_UINT32 *pUlIndex)
{
    VOS_UINT32 i = 0;

    if (pUlIndex == IP_NULL_PTR) {
        return IP_NULL_PTR;
    }

    *pUlIndex = IP_NDSERVER_ADDRINFO_MAX_NUM;

    do {
        if (PS_FALSE == g_ndServerAddrInfo[i].validFlag) {
            /* 找到空闲实体 */
            *pUlIndex = i;
            return &g_ndServerAddrInfo[i];
        }
    } while ((++i) < IP_NDSERVER_ADDRINFO_MAX_NUM);

    return IP_NULL_PTR;
}



