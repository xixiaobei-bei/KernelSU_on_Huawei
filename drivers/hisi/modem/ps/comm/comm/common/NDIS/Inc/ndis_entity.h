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

#ifndef __NDISENTITY_H__
#define __NDISENTITY_H__
/*
 * 1 其他头文件包含
 */
#include "vos.h"

#ifdef __EAGLEYE__
#include "eagleye.h"
#endif

#include "at_ndis_interface.h"
#include "ttf_comm.h"
#include  "PsTypeDef.h"
#include "LUPQueue.h"
#include "ip_comm.h"
#include "ndis_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/* DHCP选项中最大租约时间长度，单位小时，366天 */
#define TTF_NDIS_DHCP_MAX_LEASE_HOUR 8784

/* DHCP LEASE HOUR NV项默认取值 */
#define TTF_NDIS_DHCP_DEFAULT_LEASE_HOUR 144 /* 单位为小时 */

/* IPV6 MTU NV项默认取值 */
#define TTF_NDIS_IPV6_MTU_DEFAULT 1500 /* 单位为字节 */

/* 单独复位名称以及优先级 */
#define PS_NDIS_FUNC_PROC_NAME  "PS_NDIS"       /* NDIS注册给底软的名称 */
#define RESET_PRIORITY_NDIS     22              /* NDIS单独复位优先级 */

/* 通过ModemId和BearerID组合成ExBearerID，高2bit为ModemId,低6bit为BearerID */
#define NDIS_FORM_EXBID(modemId, bearerId) ((VOS_UINT8)(((modemId) << 6) | (bearerId)))

#if (VOS_OS_VER != VOS_WIN32)
/* 单板PID,FID，待定义 */
#define NDIS_TASK_PID PS_PID_APP_NDIS
#define NDIS_TASK_FID PS_FID_APP_NDIS_PPP_DIPC
#define APP_AT_PID WUEPS_PID_AT
#else
#define NDIS_TASK_PID PS_PID_NDIS
#define NDIS_TASK_FID LUEPS_FID_APP_NDIS
#define APP_AT_PID LUEPS_PID_AT
#endif

#define NAS_NDIS_MAX_ITEM 11
#define NDIS_ENTITY_IPV4 0x1 /* 该NDIS实体对应承载支持IPV4 */
#define NDIS_ENTITY_IPV6 0x2 /* 该NDIS实体对应承载支持IPV4 */
#define NDIS_ENTITY_ETH  0x4 /* 该NDIS实体对应承载支持ETHERNET */
#define NDIS_PERIOD_ARP_TMRNAME 1
#define Ndis_NvimItem_Read(ndisId, itemData, itemDataLen) \
        NDIS_NV_READX(MODEM_ID_0, ndisId, (VOS_VOID *)(itemData), itemDataLen)

#define NDIS_CHK_SEC_RETURN_VAL(x, y) \
        if (x) {                          \
            IPND_ERROR_LOG3(NDIS_NDSERVER_PID, "NDIS_CHK_SEC_RETURN_VAL!", (VOS_UINT32)x, THIS_FILE_ID, __LINE__); \
            return (y);                   \
        }
#define NDIS_GET_DL_PKTQUE(index) (g_ndisEntity[index].dlPktQue)

/* 从扩展ExBearerID中获得ModemId */
#define NDIS_GET_MODEMID_FROM_EXBID(exBearerId) (VOS_UINT16)(((exBearerId)&0xC0) >> 6)

/* 从扩展ExBearerID中获得BearerID */
#define NDIS_GET_BID_FROM_EXBID(exBearerId) (VOS_UINT8)((exBearerId)&0x3F)

/* 下行IP包缓存队列长度 */
#define NDIS_IPV4_WAIT_ADDR_RSLT_Q_LEN 10

#define NDIS_CHK_SEC_RETURN_NO_VAL(x) \
if (x) {                          \
    IPND_ERROR_LOG3(NDIS_NDSERVER_PID, "NDIS_CHK_SEC_RETURN_NO_VAL!", (VOS_UINT32)x, THIS_FILE_ID, __LINE__); \
    return;                       \
}

/* 鹰眼插桩 */
#ifdef __EAGLEYE__
#define COVERITY_TAINTED_SET(pkt) do { \
        Coverity_Tainted_Set(pkt); \
} while (0)

#else
#define COVERITY_TAINTED_SET(pkt)
#endif

#if (VOS_OS_VER != VOS_WIN32) /* 单字节对齐 */
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

/*
 * 结构名:  发给SPE的MAC头
 * 协议表格:
 * ASN.1描述:
 * 结构说明:
 */
typedef struct {
    VOS_UINT8  dstAddr[ETH_MAC_ADDR_LEN];
    VOS_UINT8  srcAddr[ETH_MAC_ADDR_LEN];
    VOS_UINT32 resv;
} SPE_MAC_EtherHeader;
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

/*
 * 3 枚举定义
 */
typedef enum {
    ARP_PAYLOAD  = 0x0608,
    IP_PAYLOAD   = 0x0008,
    IPV6_PAYLOAD = 0xdd86,
    GMAC_PAYLOAD_BUTT
} GMAC_PayloadType;
/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明:
 */
typedef union {
    VOS_UINT8  ipAddr8bit[IPV4_ADDR_LEN];
    VOS_UINT32 ipAddr32bit;
} IPV4_AddrItem;

/*
 * 协议表格:
 * ASN.1描述:
 * 结构说明:
 */
typedef union {
    VOS_UINT8  ipAddr8bit[IPV6_ADDR_LEN];
    VOS_UINT32 ipAddr32bit[4]; /* 4:addr */
} IPV6_AddrItem;


/*
 * 结构名: NDIS_RabType
 * 协议表格:
 * ASN.1描述:
 * 结构说明: NDIS承载类型枚举
 */
typedef enum {
    NDIS_RAB_NULL   = 0, /* 该承载未激活 */
    NDIS_RAB_IPV4   = 1, /* 该承载只支持IPV4 */
    NDIS_RAB_IPV6   = 2, /* 该承载只支持IPV6 */
    NDIS_RAB_IPV4V6 = 3, /* 该承载同时支持IPV4和IPV6 */
    NDIS_RAB_BUTT
} NDIS_RabType;
typedef VOS_UINT8 NDIS_RabTypeUint8;

/*
 * 结构名: NDIS_ArpPeriodTimer
 * 协议表格:
 * ASN.1描述:
 * 结构说明: ARP周期定时器结构体
 */
typedef struct {
    HTIMER     tm;          /* 定时器指针 */
    VOS_UINT32 name;       /* 定时器名字 */
    VOS_UINT32 timerValue; /* 定时器时长 */
} NDIS_ArpPeriodTimer;

/*
 * 结构名: NDIS_IPV4_INFO_STRU包结构
 * 协议表格:
 * ASN.1描述:
 * 结构说明:
 */
typedef struct {
    VOS_UINT32        arpInitFlg; /* 是否已获得UE的ARP映射 */
    VOS_UINT32        arpRepFlg;  /* ARP请求发送后受到Reply标志 */
    IPV4_AddrItem ueIpInfo;   /* UE IP,源于NAS配置 */
    IPV4_AddrItem gwIpInfo;   /* GW IP,源于NAS配置 */
    IPV4_AddrItem nmIpInfo;   /* NetMask,源于NAS配置 */
    VOS_UINT8         ueMacAddr[ETH_MAC_ADDR_LEN];
    VOS_UINT8         macFrmHdr[ETH_MAC_HEADER_LEN]; /* 完整以太帧头，便于组包 */
    NDIS_ArpPeriodTimer arpPeriodTimer;        /* 为了同64位操作系统兼容，保持在8字节对齐位置 */

    /* DHCP Server Info */
    VOS_UINT32        ipAssignStatus;
    IPV4_AddrItem primDnsAddr;
    IPV4_AddrItem secDnsAddr;
    IPV4_AddrItem primWinsAddr;
    IPV4_AddrItem secWinsAddr;
} NDIS_Ipv4Info;

/*
 * 结构名: NDIS_Entity
 * 协议表格:
 * ASN.1描述:
 * 结构说明:
 */
typedef struct {
    NDIS_RabTypeUint8   rabType;
    VOS_UINT8           rabId; /* 保存的是ExRabId的值 */
    PS_BOOL_ENUM_UINT8  used;   /* PS_TRUE:该实体被使用，PS_FALSE:该实体空闲 */
    VOS_UINT8           rev;    /* 预留字节 */
    VOS_INT32           handle;
    NDIS_Ipv4Info       ipV4Info;
    VOS_INT32           spePort;
    VOS_UINT32          speIpfFlag;
    LUP_QUEUE_STRU     *dlPktQue; /* 下行包缓存队列 */
} NDIS_Entity;

extern NDIS_Entity      g_ndisEntity[];
extern VOS_UINT32 g_nvMtu;
extern SPE_MAC_EtherHeader g_speMacHeader;
extern NDIS_Entity *g_ndisEntityAddr;


VOS_UINT32 NDIS_GetEntityIndex(VOS_UINT8 exRabId);
NDIS_Entity *NDIS_GetEntityByRabId(VOS_UINT8 exRabId);
NDIS_Entity *NDIS_AllocEntity(VOS_VOID);
VOS_UINT32 Ndis_ChkRabIdValid(VOS_UINT8 exRabId);
VOS_UINT32 Ndis_UpdateMacAddr(VOS_UINT8 *ueMacAddr, VOS_UINT8 lenth, NDIS_Ipv4Info *arpV4Info);
VOS_UINT32 AppNdis_SpeReadCb(VOS_INT32 lSpePort, VOS_VOID *pktNode);
VOS_UINT32 AppNdis_UsbReadCb(VOS_INT32 rbIdHandle, VOS_VOID *pktNode);
VOS_UINT8 *Ndis_GetMacAddr(VOS_VOID);


#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


