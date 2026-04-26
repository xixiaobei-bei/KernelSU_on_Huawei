/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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


#ifndef _DMS_PORT_DEF_H_
#define _DMS_PORT_DEF_H_

#include <product_config.h>
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4200) /* zero-sized array in struct/union */
#endif

#pragma pack(push, 4)

/* 底软管脚信号DMS层定义 */
#define DMS_IO_CTRL_FC                      0x02
#define DMS_IO_CTRL_DSR                     0x20
#define DMS_IO_CTRL_DTR                     0x01
#define DMS_IO_CTRL_RFR                     0x04
#define DMS_IO_CTRL_CTS                     0x10
#define DMS_IO_CTRL_RI                      0x40
#define DMS_IO_CTRL_DCD                     0x80
#define DMS_NCM_IPV6_DNS_LEN                32

#define DMS_UART_A_SHELL                    0x5A5A5A5A
#define DMS_UART_C_SHELL                    0xA5A5A5A5

#define DMS_PORT_UART_DEFAULT_DCD_MODE      DMS_PORT_UART_DCD_MODE_CONNECT_ON
#define DMS_PORT_UART_DEFAULT_DTR_MODE      DMS_PORT_UART_DTR_MODE_HANGUP_CALL
#define DMS_PORT_UART_DEFAULT_DSR_MODE      DMS_PORT_UART_DSR_MODE_ALWAYS_ON

enum DMS_PortId {
    DMS_PORT_APP,
    DMS_PORT_APP1,
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    DMS_PORT_APP2,
    DMS_PORT_APP3,
    DMS_PORT_APP4,
    DMS_PORT_APP5,
    DMS_PORT_APP6,
    DMS_PORT_APP7,
    DMS_PORT_APP8,
    DMS_PORT_APP9,
    DMS_PORT_APP10,
    DMS_PORT_APP11,
    DMS_PORT_APP12,
    DMS_PORT_APP13,
    DMS_PORT_APP14,
    DMS_PORT_APP15,
    DMS_PORT_APP16,
    DMS_PORT_APP17,
    DMS_PORT_APP18,
    DMS_PORT_APP19,
    DMS_PORT_APP20,
    DMS_PORT_APP21,
    DMS_PORT_APP22,
    DMS_PORT_APP23,
    DMS_PORT_APP24,
    DMS_PORT_APP25,
    DMS_PORT_APP26,

    DMS_PORT_APP27,
    DMS_PORT_APP28,
    DMS_PORT_APP29,
    DMS_PORT_APP30,
    DMS_PORT_APP31,
    DMS_PORT_APP32,
    DMS_PORT_APP33,
    DMS_PORT_APP34,
    DMS_PORT_APP35,
    DMS_PORT_APP36,
    DMS_PORT_APP37,
    DMS_PORT_APP38,
    DMS_PORT_APP39,
    DMS_PORT_APP40,
    DMS_PORT_APP41,
    DMS_PORT_APP42,
    DMS_PORT_APP43,
    DMS_PORT_APP44,
    DMS_PORT_APP45,
    DMS_PORT_APP46,
    DMS_PORT_APP47,
    DMS_PORT_APP48,
    DMS_PORT_APP49,
    DMS_PORT_APP50,
    DMS_PORT_APP51,
    DMS_PORT_APP52,
#endif
    /* DMS_APP_PORT_SIZE
     * 仅做APP口索引检查使用，不能作为具体端口的索引，
     * 否则会和DMS_PORT_PCUI冲突
     */
    DMS_APP_PORT_SIZE,

    DMS_PORT_PCUI = DMS_APP_PORT_SIZE,
    DMS_PORT_CTRL,
    DMS_PORT_PCUI2,
    DMS_PORT_NCM_CTRL,
    DMS_PORT_UART,
    DMS_PORT_SOCK,

    DMS_PORT_MODEM,

#if (FEATURE_AT_HSUART == FEATURE_ON)
    DMS_PORT_HSUART,
#endif
    DMS_PORT_NCM_DATA,
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    DMS_PORT_NCM_DATA1,
    DMS_PORT_NCM_DATA2,
    DMS_PORT_NCM_DATA3,
#endif
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_PORT_CMUXAT,
    DMS_PORT_CMUXMDM,
    DMS_PORT_CMUXEXT,
    DMS_PORT_CMUXGPS,
#endif
#if (FEATURE_AT_PROXY == FEATURE_ON)
    DMS_PORT_ATP,
#endif

    DMS_PORT_BUTT
};
typedef VOS_UINT16 DMS_PortIdUint16;

enum DMS_ClientType {
    DMS_CLIENT_TYPE_COM,
    DMS_CLIENT_TYPE_MODEM,
#if (FEATURE_AT_HSUART == FEATURE_ON)
    DMS_CLIENT_TYPE_HSUART,
#endif
    DMS_CLIENT_TYPE_NDIS_DATA,
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    DMS_CLIENT_TYPE_CMUX,
#endif
    DMS_CLIENT_TYPE_BUTT,
};
typedef VOS_UINT8 DMS_ClientTypeUint8;

enum DMS_ConnectStat {
    DMS_CONNECT,
    DMS_DISCONNECT,
    DMS_CONNECT_BUTT,
};
typedef VOS_UINT8 DMS_ConnectStatUint8;

enum DMS_PortMode {
    DMS_PORT_MODE_CMD,
    DMS_PORT_MODE_ONLINE_DATA,
    DMS_PORT_MODE_ONLINE_CMD,
    DMS_PORT_MODE_BUTT,
};
typedef VOS_UINT8 DMS_PortModeUint8;

enum DMS_PortDataMode {
    DMS_PORT_DATA_PPP_RAW,
    DMS_PORT_DATA_PPP,
    DMS_PORT_DATA_OM,
    DMS_PORT_DATA_RAW_DATA,

    DMS_PORT_DATA_BUTT,
};
typedef VOS_UINT8 DMS_PortDataModeUint8;

enum DMS_PortDataState {
    DMS_PORT_DATA_STATE_STOP,
    DMS_PORT_DATA_STATE_START,

    DMS_PORT_DATA_STATE_BUTT,
};
typedef VOS_UINT8 DMS_PortDataStateUint8;

enum DMS_PortMsgId {
    /* event事件消息占用dms模块和其他模块交互消息id，
     * 当前dms和外部模块暂时没有消息交互，
     * 如果后续Dms模块和外部模块有消息交互，消息定义需要避开event占用的消息id，
     * 建议其他模块交互的消息ID从0x1000开始
     */
    /* DMS->AT */
    /* _MSGPARSE_Interception DMS_ELF_SUBSCRIP_EVENT_STRU */
    ID_DMS_PORT_SUBSCRIP_EVENT_NOTIFY      = 0x0000,

    ID_DMS_PORT_MSG_BUTT,
};
typedef VOS_UINT32 DMS_PortMsgIdUint32;

enum DMS_SubscripEventId {
    ID_DMS_EVENT_LOW_WATER_MARK,
    ID_DMS_EVENT_NCM_CONN_BREAK,
    ID_DMS_EVENT_USB_DISCONNECT,
    ID_DMS_EVENT_SWITCH_GW_MODE,
    ID_DMS_EVENT_ESCAPE_SEQUENCE,
    ID_DMS_EVENT_DTR_DEASSERT,
#if (FEATURE_IOT_CMUX == FEATURE_ON)
    ID_DMS_EVENT_UART_INIT,
#endif

    ID_DMS_EVENT_BUTT,
};
typedef VOS_UINT32 DMS_SubscripEventIdUint32;


enum DMS_PORT_UartDcdMode {
    DMS_PORT_UART_DCD_MODE_ALWAYS_ON  = 0x00,
    DMS_PORT_UART_DCD_MODE_CONNECT_ON = 0x01,

    DMS_PORT_UART_DCD_MODE_BUTT
};
typedef VOS_UINT8 DMS_PortUartDcdModeUint8;


enum DMS_PORT_UartDtrMode {
    DMS_PORT_UART_DTR_MODE_IGNORE          = 0x00,
    DMS_PORT_UART_DTR_MODE_SWITCH_CMD_MODE = 0x01,
    DMS_PORT_UART_DTR_MODE_HANGUP_CALL     = 0x02,

    DMS_PORT_UART_DTR_MODE_BUTT
};
typedef VOS_UINT8 DMS_PortUartDtrModeUint8;


enum DMS_PORT_UartDsrMode {
    DMS_PORT_UART_DSR_MODE_ALWAYS_ON  = 0x00,
    DMS_PORT_UART_DSR_MODE_CONNECT_ON = 0x01,

    DMS_PORT_UART_DSR_MODE_BUTT
};
typedef VOS_UINT8 DMS_PortUartDsrModeUint8;

enum DMS_WaterLevel {
    DMS_LEVEL_HIGH,
    DMS_LEVEL_LOW,

    DMS_LEVEL_BUTT
};
typedef VOS_UINT32 DMS_WaterLevelUint32;

/* NCM网卡状态改变通知枚举，参考底软ncm_ioctrl_connect_stus枚举 */
enum DMS_NcmStatus {
    DMS_NCM_IOCTL_STUS_CONNECT,
    DMS_NCM_IOCTL_STUS_BREAK,

    DMS_NCM_STATUS_BUTT
};
typedef VOS_UINT32 DMS_NcmStatusUint32;

/* NCM流控开关枚举，参考底软ncm_ioctl_flow_ctrl枚举 */
enum DMS_NcmFlowCtrlStus {
    DMS_NCM_FLOW_CTRL_ENABLE,
    DMS_NCM_FLOW_CTRL_DISABLE,

    DMS_NCM_FLOW_CTRL_BUTT
};
typedef VOS_UINT32 DMS_NcmFlowCtrlStusUint32;

/* NCM连接状态枚举,参考底软ncm_ioctl_connection_status枚举 */
enum DMS_NcmConnectionStatus {
    DMS_NCM_CONNECTION_LINKDOWN,
    DMS_NCM_CONNECTION_LINKUP,

    DMS_NCM_CONNECTION_BUTT
};
typedef VOS_UINT32 DMS_NcmConnectionStatusUint32;

/* UART数据位长*/
enum {
    DMS_WLEN_5_BITS,
    DMS_WLEN_6_BITS,
    DMS_WLEN_7_BITS,
    DMS_WLEN_8_BITS,
    DMS_WLEN_BUTT,
};
typedef VOS_UINT32 DMS_UART_WLEN_ENUM;

/* UART停止位长*/
enum {
    DMS_STP2_OFF,
    DMS_STP2_ON,
    DMS_STP2_BUTT,
};
typedef VOS_UINT32 DMS_UART_STP2_ENUM;

/* UART校验方式*/
enum {
    DMS_PARITY_NO_CHECK,
    DMS_PARITY_CHECK_ODD,
    DMS_PARITY_CHECK_EVEN,
    DMS_PARITY_CHECK_MARK,  /*校验位始终为1*/
    DMS_PARITY_CHECK_SPACE, /*校验位始终为0*/
    DMS_PARITY_CHECK_BUTT,
};
typedef VOS_UINT32 DMS_UART_PARITY_ENUM;

struct DMS_NcmConnBreakInfo {
    DMS_PortIdUint16 ncmDataPortId;
    VOS_UINT16 rsv;
};

struct DMS_PORT_SubscripEventNotify {
    VOS_MSG_HEADER
    DMS_PortMsgIdUint32         msgId;

    DMS_SubscripEventIdUint32   eventId;
    VOS_UINT16                  clientId;
    VOS_UINT16                  reserv;

    VOS_UINT32                  data[0];
};

struct DMS_MscInfo
{
    VOS_UINT32    dtrFlag   : 1;          /*DTR CHANGE FLAG*/
    VOS_UINT32    dsrFlag   : 1;          /*DSR CHANGE FLAG*/
    VOS_UINT32    ctsFlag   : 1;          /*CTSCHANGE FLAG*/
    VOS_UINT32    rtsFlag   : 1;          /*RTS CHANGE FLAG*/
    VOS_UINT32    riFlag    : 1;          /*RI CHANGE FLAG*/
    VOS_UINT32    dcdFlag   : 1;          /*DCD CHANGE FLAG*/
    VOS_UINT32    fcFlag    : 1;          /*FC CHANGE FLAG*/
    VOS_UINT32    brkFlag   : 1;          /*BRK CHANGE FLAG*/
    VOS_UINT32    spareFlag : 24;         /*reserve*/
    VOS_UINT8     dtr;                    /*DTR  VALUE*/
    VOS_UINT8     dsr;                    /*DSR  VALUE*/
    VOS_UINT8     cts;                    /*DTS VALUE*/
    VOS_UINT8     rts;                    /*RTS  VALUE*/
    VOS_UINT8     ri;                     /*RI VALUE*/
    VOS_UINT8     dcd;                    /*DCD  VALUE*/
    VOS_UINT8     fc;                     /*FC  VALUE*/
    VOS_UINT8     brk;                    /*BRK  VALUE*/
    VOS_UINT8     brklen;                 /*BRKLEN VALUE*/
    VOS_UINT8     reserv[3];
};

struct DMS_NcmIpv6Dns {
    VOS_UINT8 *ipv6DnsInfo;
    VOS_UINT32 length;
    VOS_UINT32 reserv;
};

struct DMS_ConnSpeed
{
    VOS_UINT32 downSpeed;
    VOS_UINT32 upSpeed;
};

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
