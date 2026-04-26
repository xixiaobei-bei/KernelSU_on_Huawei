/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/20
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
#include "ndis_entity.h"
#include "vos.h"
#include "mdrv.h"
#include "securec.h"
#include "nv_stru_gucnas.h"
#include "ps_tag.h"
#include "ps_common_def.h"
#include "LUPQueue.h"
#include "ndis_interface.h"
#include "ipv4_dhcp_server.h"
#include "ip_nd_server.h"
#include "ndis_om.h"
#include "ndis_ul_proc.h"
#include "ndis_interface.h"
#include "ndis_msg_proc.h"
#include "ndserver_entity.h"


#define THIS_MODU ps_ndis

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISENTITY_C
/*lint +e767*/

SPE_MAC_EtherHeader g_speMacHeader  = {{ 0x58, 0x02, 0x03, 0x04, 0x05, 0x06 },
                                       { 0x00, 0x11, 0x09, 0x64, 0x01, 0x01 },
                                       0x00000000 }; /* mac地址初始化为固定值 */

NDIS_Entity  g_ndisEntity[NAS_NDIS_MAX_ITEM] = {{0}};
NDIS_Entity *g_ndisEntityAddr = g_ndisEntity;
VOS_UINT32   g_nvMtu = 1500; /* IPV6 MTU默认取值 */
const VOS_UINT32 g_periodicArpCyc = 3000; /* 周期性ARP发送周期 */



/*
 * 功能描述: A核NDIS读取NV项的初始化函数
 * DHCP Lease Time, 设定范围为[1, 8784]小时
 * Vodafone    24小时
 * 其他        72小时
 * 时间经过DHCP Lease Time一半时，PC会主动发起续租，
 * 如果DHCP租约超期，则从PC通过NDIS通道发往单板的数据会出现目的不可达错误
 * 目前测试，当DHCP Lease Time小于等于4S时，对于数传影响较大，所以定义最小租约为1小时
 * 目前没有遇到超过8天的DHCP Lease Time，暂定上限为8784小时(366天)
 * 修改历史:
 *  1.日    期: 2012年4月24日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 Ndis_NvItemInit(VOS_VOID)
{
    VOS_UINT32                       rtn;
    VOS_UINT32                       dhcpLeaseHour;
    VOS_UINT32                       ipv6Mtu;
    NDIS_NV_DhcpLeaseHour            ndisDhcpLeaseHour = {0};
    TAF_NDIS_NvIpv6RouterMtu         ndisIPv6Mtu = {0};

    /* 从NV读取流控配置信息 */
    /*lint -e718*/
    /*lint -e732*/
    /*lint -e746*/
    rtn = Ndis_NvimItem_Read(NV_ITEM_NDIS_DHCP_DEF_LEASE_TIME, &ndisDhcpLeaseHour,
                             sizeof(NDIS_NV_DhcpLeaseHour));
    if (rtn != PS_SUCC) {
        PS_PRINTF_ERR("Ndis_NvItemInit, Fail to read NV DHCP_LEASE_TIME: %d\n", rtn);
        ndisDhcpLeaseHour.dhcpLeaseHour = TTF_NDIS_DHCP_DEFAULT_LEASE_HOUR;
    }

    /* NV值合法性判断 */
    dhcpLeaseHour = ndisDhcpLeaseHour.dhcpLeaseHour;
    if ((dhcpLeaseHour > 0) && (dhcpLeaseHour <= TTF_NDIS_DHCP_MAX_LEASE_HOUR)) {
        /* 3600:hour -> second */
        g_leaseTime = dhcpLeaseHour * 3600;
    }

    /* 从NV读取IPV6 MTU信息 */
    rtn = Ndis_NvimItem_Read(NV_ITEM_IPV6_ROUTER_MTU, &ndisIPv6Mtu, sizeof(TAF_NDIS_NvIpv6RouterMtu));
    /*lint +e746*/
    /*lint +e732*/
    /*lint +e718*/
    if (rtn != PS_SUCC) {
        PS_PRINTF_ERR("Ndis_NvItemInit, Fail to read NV IPV6_MTU: %d\n!", rtn);
        ndisIPv6Mtu.ipv6RouterMtu = TTF_NDIS_IPV6_MTU_DEFAULT;
    }

    /* NV值合法性判断 */
    ipv6Mtu = ndisIPv6Mtu.ipv6RouterMtu;
    if (ipv6Mtu == 0) {
        g_nvMtu = TTF_NDIS_IPV6_MTU_DEFAULT;
    } else {
        g_nvMtu = ipv6Mtu;
    }

    return PS_SUCC;
}

/*
 * 功能描述: 获取MAC地址
 * 修改历史:
 *  1.日    期: 2014年10月17日
 *    修改内容: 新生成函数
 */
VOS_UINT8 *Ndis_GetMacAddr(VOS_VOID)
{
    /* LTE协议栈MAC地址 */
    static VOS_UINT8 gUcMacAddressPstable[] = {
        0x4c, 0x54, 0x99, 0x45, 0xe5, 0xd5
    };

    return gUcMacAddressPstable;
}

/*
 * 功能描述: APP核NDIS功能的初始化函数
 * 修改历史:
 *  1.日    期: 2011年2月10日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 Ndis_Init(VOS_VOID)
{
    VOS_UINT32                     loop;
    VOS_UINT8                     *macAddr        = VOS_NULL_PTR;
    VOS_UINT16                     payLoad;
    VOS_INT32                      rlt;
    VOS_UINT32                     len            = 2; /* ip payload为2个字节 */
    NDIS_ArpPeriodTimer           *arpPeriodTimer = VOS_NULL_PTR;

    /*lint -e746*/
    macAddr = (VOS_UINT8 *)Ndis_GetMacAddr(); /* 获得单板MAC地址 */
    /*lint -e746*/
    if (macAddr == VOS_NULL_PTR) {
        PS_PRINTF_ERR("Ndis_Init, Ndis_GetMacAddr Fail!\n");
        return PS_FAIL;
    }

    payLoad = IP_PAYLOAD;
    for (loop = 0; loop < NAS_NDIS_MAX_ITEM; loop++) {
        g_ndisEntity[loop].rabType    = NDIS_RAB_NULL;
        g_ndisEntity[loop].handle     = NDIS_INVALID_HANDLE;
        g_ndisEntity[loop].rabId      = NDIS_INVALID_RABID;
        g_ndisEntity[loop].used       = PS_FALSE;
        g_ndisEntity[loop].spePort     = NDIS_INVALID_SPEPORT;
        g_ndisEntity[loop].speIpfFlag = PS_FALSE;
        rlt = memcpy_s(g_ndisEntity[loop].ipV4Info.macFrmHdr + ETH_MAC_ADDR_LEN, ETH_MAC_ADDR_LEN,
                       macAddr, ETH_MAC_ADDR_LEN);
        NDIS_CHK_SEC_RETURN_VAL((rlt != EOK), PS_FAIL);

        rlt = memcpy_s(g_ndisEntity[loop].ipV4Info.macFrmHdr + (2 * ETH_MAC_ADDR_LEN), len, /* 2:addr */
                       (VOS_UINT8 *)(&payLoad), len);
        NDIS_CHK_SEC_RETURN_VAL((rlt != EOK), PS_FAIL);

        /* 周期性ARP定时器初始化 */
        arpPeriodTimer               = &(g_ndisEntity[loop].ipV4Info.arpPeriodTimer);
        arpPeriodTimer->tm          = VOS_NULL_PTR;
        arpPeriodTimer->name       = NDIS_PERIOD_ARP_TMRNAME;
        arpPeriodTimer->timerValue = g_periodicArpCyc;
        /* 创建下行IP包缓存队列 */
        if (NDIS_GET_DL_PKTQUE(loop) != VOS_NULL_PTR) {
            continue;
        }
        rlt = LUP_CreateQue(NDIS_TASK_PID, &(NDIS_GET_DL_PKTQUE(loop)),
                            NDIS_IPV4_WAIT_ADDR_RSLT_Q_LEN);
        if (rlt != PS_SUCC) {
            PS_PRINTF_ERR("Ndis_Init, LUP_CreateQue DlPktQue fail.\n");
            continue;
        }
    }

    if (Ndis_NvItemInit() != PS_SUCC) { /* NV项初始化 */
        PS_PRINTF_ERR("Ndis_Init, Ndis_NvItemInit Fail!\n");
        return PS_FAIL;
    }

    return PS_SUCC;
}

/*
 * 功能描述: C核单独复位时NDIS的回调处理函数
 * 修改历史:
 * 1.日    期: 2019年10月10日
 *   修改内容: 新生成函数
 */
STATIC VOS_INT Ndis_ModemResetCb(drv_reset_cb_moment_e param, VOS_INT userData)
{
    VOS_UINT32  result;
    VOS_UINT32  resetCbBefore;         /* 单独复位BEFORE统计 */
    VOS_UINT32  resetCbAfter;          /* 单独复位AFTER统计 */
    if (param == MDRV_RESET_CB_BEFORE) {
        PS_PRINTF_ERR("NDIS before reset: enter.");

        /* Ndis 初始化 */
        result = Ndis_Init();
        if (result != PS_SUCC) {
            PS_PRINTF_ERR("APP_Ndis_Pid_InitFunc, Ndis_Init fail!\n");
            return VOS_ERR;
        }

        /* 复位统计之外的统计数据清0 */
        resetCbBefore = g_ndisStatStru.resetCbBefore;
        resetCbAfter  = g_ndisStatStru.resetCbAfter;
        (VOS_VOID)memset_s(&g_ndisStatStru, sizeof(NDIS_StatInfo), 0x00,
                           sizeof(NDIS_StatInfo));

        /* 增加复位统计 */
        g_ndisStatStru.resetCbBefore = resetCbBefore;
        g_ndisStatStru.resetCbAfter  = resetCbAfter;
        NDIS_STAT_RESET_CB_BEFORE(1);

        PS_PRINTF_ERR("NDIS before reset: succ.");
        return VOS_OK;
    } else if (param == MDRV_RESET_CB_AFTER) {
        PS_PRINTF_ERR("NDIS after reset enter.");

        /* 增加复位统计 */
        NDIS_STAT_RESET_CB_AFTER(1);
        PS_PRINTF_ERR("NDIS after reset: succ.");
        return VOS_OK;
    } else {
        return VOS_ERROR;
    }
}

/*
 * 功能描述: APP NDIS下行PID初始化函数
 * 修改历史:
 *  1.日    期: 2011年2月15日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 APP_Ndis_Pid_InitFunc(enum VOS_InitPhaseDefine phase)
{
    VOS_UINT32 result;

    switch (phase) {
        case VOS_IP_LOAD_CONFIG:
            result = Ndis_Init();
            if (result != PS_SUCC) {
                PS_PRINTF_ERR("APP_Ndis_Pid_InitFunc, Ndis_Init fail!\n");
                return VOS_ERR;
            }
            /* 给底软注册回调函数，用于C核单独复位的处理 */
            (VOS_VOID)mdrv_sysboot_register_reset_notify(PS_NDIS_FUNC_PROC_NAME, Ndis_ModemResetCb, 0,
                                                         RESET_PRIORITY_NDIS);
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

    return VOS_OK;
}

/*lint -e40*/
/*
 * 功能描述: NDIS的FID初始化函数
 * 修改历史:
 *  1.日    期: 2008年9月17日
 *    修改内容: 新生成函数
 */
VOS_UINT32 APP_NDIS_FidInit(enum VOS_InitPhaseDefine phase)
{
    VOS_UINT32 result;

    switch (phase) {
        case VOS_IP_LOAD_CONFIG:
            /* 注册NDIS PID */
            result = VOS_RegisterPIDInfo(NDIS_TASK_PID, (InitFunType)APP_Ndis_Pid_InitFunc,
                                         (MsgFunType)APP_Ndis_PidMsgProc);
            if (result != VOS_OK) {
                PS_PRINTF_ERR("APP_NDIS_FidInit, register NDIS PID fail!\n");
                return VOS_ERR;
            }

            /* 注册ND SERVER PID */
            result = VOS_RegisterPIDInfo(NDIS_NDSERVER_PID, (InitFunType)APP_NdServer_Pid_InitFunc,
                                         (MsgFunType)APP_NdServer_PidMsgProc);
            if (result != VOS_OK) {
                PS_PRINTF_ERR("APP_NDIS_FidInit, register NDSERVER PID fail!\n");
                return VOS_ERR;
            }

            result = VOS_RegisterMsgTaskPrio(NDIS_TASK_FID, VOS_PRIORITY_P4);
            if (result != VOS_OK) {
                PS_PRINTF_ERR("APP_NDIS_FidInit, register priority fail!\n");
                return VOS_ERR;
            }
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

    return PS_SUCC;
}

/*
 * 功能描述: 根据ExRabId查找NDIS实体的索引
 */
VOS_UINT32 NDIS_GetEntityIndex(VOS_UINT8 exRabId)
{
    VOS_UINT32 i = 0;

    /* 查询是否已存在相应Entity */
    do {
        if ((g_ndisEntity[i].used == PS_TRUE) && (exRabId == g_ndisEntity[i].rabId)) {
            /* 找到相应实体 */
            return i;
        }
    } while ((++i) < NAS_NDIS_MAX_ITEM);

    return NAS_NDIS_MAX_ITEM;
}

/*
 * 功能描述: 根据ExRabId查找NDIS实体
 * 修改历史:
 *  1.日    期: 2013年1月15日
 *    修改内容: 新生成函数
 */
NDIS_Entity *NDIS_GetEntityByRabId(VOS_UINT8 exRabId)
{
    VOS_UINT16 i = 0;

    /* 查询是否已存在相应Entity */
    do {
        if ((g_ndisEntity[i].used == PS_TRUE) && (exRabId == g_ndisEntity[i].rabId)) {
            /* 找到相应实体 */
            return &g_ndisEntity[i];
        }
    } while ((++i) < NAS_NDIS_MAX_ITEM);

    return VOS_NULL_PTR;
}

/*
 * 功能描述: 分配一个空闲的NDIS实体
 * 修改历史:
 *  1.日    期: 2013年1月15日
 *    修改内容: 新生成函数
 */
NDIS_Entity *NDIS_AllocEntity(VOS_VOID)
{
    VOS_UINT16 i = 0;

    /* 返回第一个空闲的实体 */
    do {
        if (g_ndisEntity[i].used == PS_FALSE) {
            /* 找到空闲实体 */
            return &g_ndisEntity[i];
        }
    } while ((++i) < NAS_NDIS_MAX_ITEM);

    return VOS_NULL_PTR;
}

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
/*
 * 功能描述: Ndis检查ExRabId取值是否在合法范围内
 * 修改历史:
 *  1.日    期: 2018年08月21日
 *    修改内容: 新生成函数
 */
VOS_UINT32 Ndis_ChkRabIdValid(VOS_UINT8 exRabId)
{
    /* B5000及以后的版本下，Ndis的索引为IFACE ID */
    if (exRabId >= PS_IFACE_ID_BUTT) {
        return PS_FAIL;
    }

    return PS_SUCC;
}
#else
/*
 * 功能描述: Ndis检查ExRabId取值是否在合法范围内
 * 修改历史:
 *  1.日    期: 2012年12月7日
 *    修改内容: 新生成函数
 *  2.日    期: 2013年1月15日
 *    修改内容: DSDA特性开发:对ModemID和RabId均做检查
 */
VOS_UINT32 Ndis_ChkRabIdValid(VOS_UINT8 exRabId)
{
    VOS_UINT16 modemId;
    VOS_UINT8  rabId;

    modemId = NDIS_GET_MODEMID_FROM_EXBID(exRabId);
    if (modemId >= MODEM_ID_BUTT) {
        return PS_FAIL;
    }

    rabId = NDIS_GET_BID_FROM_EXBID(exRabId);
    if ((rabId < MIN_VAL_EPSID) || (rabId > MAX_VAL_EPSID)) {
        return PS_FAIL;
    }

    return PS_SUCC;
}

#endif

VOS_UINT32 Ndis_UpdateMacAddr(VOS_UINT8 *ueMacAddr, VOS_UINT8 lenth, NDIS_Ipv4Info *arpV4Info)
{
    errno_t              rlt;
    if (lenth > ETH_MAC_ADDR_LEN) {
        return PS_FAIL;
    }
    rlt = memcpy_s(arpV4Info->ueMacAddr, ETH_MAC_ADDR_LEN, ueMacAddr, ETH_MAC_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_VAL((rlt != EOK), PS_FAIL);
    rlt = memcpy_s(arpV4Info->macFrmHdr, ETH_MAC_HEADER_LEN, ueMacAddr, ETH_MAC_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_VAL((rlt != EOK), PS_FAIL);

    arpV4Info->arpInitFlg = PS_TRUE;

    rlt = memcpy_s(g_speMacHeader.srcAddr, ETH_MAC_ADDR_LEN, (arpV4Info->macFrmHdr + ETH_MAC_ADDR_LEN),
                   ETH_MAC_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_VAL((rlt != EOK), PS_FAIL);

    rlt = memcpy_s(g_speMacHeader.dstAddr, ETH_MAC_ADDR_LEN, ueMacAddr, ETH_MAC_ADDR_LEN);
    NDIS_CHK_SEC_RETURN_VAL((rlt != EOK), PS_FAIL);

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#if (defined(CONFIG_BALONG_ESPE))
        mdrv_wan_set_ipfmac((VOS_VOID *)&g_speMacHeader);
#endif
#endif
    return PS_SUCC;
}



