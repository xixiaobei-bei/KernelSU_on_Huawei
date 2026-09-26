/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/**
 *  @brief   udi模块对外头文件
 *  @file    mdrv_udi.h
 *  @author  qinyu
 *  @version v1.0
 *  @date    2019.11.26
 *  @note    只适用非融合版本(版本号|修订日期|说明)
 *  <ul><li>v1.0|2019.9.15|创建文件</li></ul>
 *  @since
 */

#ifndef __MDRV_ACORE_UDI_H__
#define __MDRV_ACORE_UDI_H__

#include "mdrv_udi_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UDI_BUILD_DEV_ID(dev, type) ((dev << 8) | (type & 0x00ff))

enum udi_devices_main_id {
    UDI_DEV_USB_ACM = 0,
    UDI_DEV_USB_NCM,
    UDI_DEV_ICC,
    UDI_DEV_UART,
    UDI_DEV_HSUART,
    UDI_DEV_USB_ADC,
    UDI_DEV_CMUX,
    UDI_DEV_SOCKET,
    UDI_DEV_MAX
};

enum udi_acm_dev_type {
    UDI_USB_ACM_CTRL,
    UDI_USB_ACM_AT,
    UDI_USB_ACM_SHELL,
    UDI_USB_ACM_LTE_DIAG,
    UDI_USB_ACM_OM,
    UDI_USB_ACM_MODEM,
    UDI_USB_ACM_GPS,
    UDI_USB_ACM_3G_GPS,
    UDI_USB_ACM_3G_PCVOICE,
    UDI_USB_ACM_PCVOICE,
    UDI_USB_ACM_SKYTONE,
    UDI_USB_ACM_CDMA_LOG,
#ifdef CONFIG_PCDEV_BALONG_UDC
    UDI_PCDEV_AGENT_NV,
    UDI_PCDEV_AGENT_OM,
    UDI_PCDEV_AGENT_MSG,
    UDI_PCDEV_APPVCOM,
    UDI_PCDEV_MODEM,
    UDI_PCDEV_PMOM,
    UDI_PCDEV_RESERVE,
#else
    UDI_USB_HSIC_ACM0,
    UDI_USB_HSIC_ACM1,
    UDI_USB_HSIC_ACM2,
    UDI_USB_HSIC_ACM3,
    UDI_USB_HSIC_ACM4,
    UDI_USB_HSIC_ACM5,
    UDI_USB_HSIC_ACM6,
#endif
    UDI_USB_HSIC_ACM7,
    UDI_USB_HSIC_ACM8,
    UDI_USB_HSIC_ACM9,
    UDI_USB_HSIC_ACM10,
    UDI_USB_HSIC_ACM11,
    UDI_USB_HSIC_ACM12,
    UDI_USB_HSIC_ACM13,
    UDI_USB_HSIC_ACM14,
    UDI_USB_HSIC_MODEM0,
    UDI_USB_ACM_MAX
};

enum udi_socket_dev_type {
    UDI_SOCKET_LTE_DIAG = UDI_USB_ACM_LTE_DIAG,
    UDI_SOCKET_OM = UDI_USB_ACM_OM,
    UDI_SOCKET_GPS = UDI_USB_ACM_GPS,

    UDI_SOCKET_MAX
};

enum udi_ncm_dev_type {
    UDI_USB_NCM_NDIS,
    UDI_USB_NCM_CTRL,
    UDI_USB_NCM_NDIS1,
    UDI_USB_NCM_CTRL1,
    UDI_USB_NCM_NDIS2,
    UDI_USB_NCM_CTRL2,
    UDI_USB_NCM_NDIS3,
    UDI_USB_NCM_CTRL3,
    UDI_USB_NCM_BOTTOM, /* 用于MBB多网卡场景 */
    UDI_USB_HSIC_NCM0,
    UDI_USB_HSIC_NCM1,
    UDI_USB_HSIC_NCM2,

    UDI_USB_NCM_MAX /* 必须在最后, 用于边界值 */
};

enum udi_adc_dev_type {
    UDI_USB_ADC0,
    UDI_USB_ADC_MAX
};

enum udi_device_id {
    /* USB ACM */
    UDI_ACM_CTRL_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_CTRL),
    UDI_ACM_AT_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_AT),
    UDI_ACM_SHELL_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_SHELL),
    UDI_ACM_LTE_DIAG_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_LTE_DIAG),
    UDI_ACM_OM_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_OM),
    UDI_ACM_MODEM_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_MODEM),
    UDI_ACM_GPS_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_GPS),       /* HISO */
    UDI_ACM_3G_GPS_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_3G_GPS), /* ashell */
    UDI_ACM_3G_PCVOICE_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_3G_PCVOICE),
    UDI_ACM_PCVOICE_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_PCVOICE),
    UDI_ACM_CDMA_LOG_ID  = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_CDMA_LOG),
    UDI_ACM_SKYTONE_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_ACM_SKYTONE),
#ifdef CONFIG_PCDEV_BALONG_UDC
    UDI_PCDEV_AGENT_NV_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_AGENT_NV),
    UDI_PCDEV_AGENT_OM_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_AGENT_OM),
    UDI_PCDEV_AGENT_MSG_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_AGENT_MSG),
    UDI_PCDEV_APPVCOM_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_APPVCOM),
    UDI_PCDEV_MODEM_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_MODEM),
    UDI_PCDEV_PMOM_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_PMOM),
    UDI_PCDEV_RESERVE_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_PCDEV_RESERVE),
#else
    UDI_ACM_HSIC_ACM0_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM0),
    UDI_ACM_HSIC_ACM1_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM1),
    UDI_ACM_HSIC_ACM2_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM2),
    UDI_ACM_HSIC_ACM3_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM3),
    UDI_ACM_HSIC_ACM4_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM4),
    UDI_ACM_HSIC_ACM5_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM5),
    UDI_ACM_HSIC_ACM6_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM6),
#endif
    UDI_ACM_HSIC_ACM7_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM7),
    UDI_ACM_HSIC_ACM8_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM8),
    UDI_ACM_HSIC_ACM9_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM9),
    UDI_ACM_HSIC_ACM10_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM10),
    UDI_ACM_HSIC_ACM11_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM11),
    UDI_ACM_HSIC_ACM12_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM12),
    UDI_ACM_HSIC_ACM13_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM13),
    UDI_ACM_HSIC_ACM14_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_ACM14),
    UDI_ACM_HSIC_MODEM0_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ACM, UDI_USB_HSIC_MODEM0),
    /* USB NCM */
    UDI_NCM_NDIS_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_NDIS),
    UDI_NCM_CTRL_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_CTRL),
    UDI_NCM_NDIS1_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_NDIS1),
    UDI_NCM_CTRL1_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_CTRL1),
    UDI_NCM_NDIS2_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_NDIS2),
    UDI_NCM_CTRL2_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_CTRL2),
    UDI_NCM_NDIS3_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_NDIS3),
    UDI_NCM_CTRL3_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_NCM_CTRL3),

    UDI_NCM_HSIC_NCM0_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_HSIC_NCM0),
    UDI_NCM_HSIC_NCM1_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_HSIC_NCM1),
    UDI_NCM_HSIC_NCM2_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_NCM, UDI_USB_HSIC_NCM2),

    /* UART */
    UDI_UART_0_ID = UDI_BUILD_DEV_ID(UDI_DEV_UART, 0),
    UDI_UART_1_ID = UDI_BUILD_DEV_ID(UDI_DEV_UART, 1),

    /* HSUART */
    UDI_HSUART_0_ID = UDI_BUILD_DEV_ID(UDI_DEV_HSUART, 0),
    UDI_HSUART_1_ID = UDI_BUILD_DEV_ID(UDI_DEV_HSUART, 1),

    UDI_ADC_SND0_ID = UDI_BUILD_DEV_ID(UDI_DEV_USB_ADC, UDI_USB_ADC0),

    /* CMUX模拟的AT端口 */
    UDI_CMUX_AT_ID =  UDI_BUILD_DEV_ID(UDI_DEV_CMUX, 0),
    /* CMUX模拟的Modem端口 */
    UDI_CMUX_MDM_ID =  UDI_BUILD_DEV_ID(UDI_DEV_CMUX, 1),
    /* CMUX模拟的Extend端口 */
    UDI_CMUX_EXT_ID =  UDI_BUILD_DEV_ID(UDI_DEV_CMUX, 2),
    /* CMUX模拟的GPS端口 */
    UDI_CMUX_GPS_ID =  UDI_BUILD_DEV_ID(UDI_DEV_CMUX, 3),

    /* SOCKET */
    UDI_SOCKET_LTE_DIAG_ID = UDI_BUILD_DEV_ID(UDI_DEV_SOCKET, UDI_SOCKET_LTE_DIAG),
    UDI_SOCKET_OM_ID = UDI_BUILD_DEV_ID(UDI_DEV_SOCKET, UDI_SOCKET_OM),
    UDI_SOCKET_GPS_ID = UDI_BUILD_DEV_ID(UDI_DEV_SOCKET, UDI_SOCKET_GPS),
    /* MUST BE LAST */
    UDI_INVAL_DEV_ID = 0xFFFF
};

struct udi_open_param {
    enum udi_device_id devid; /* 设备ID */
    void *private;        /* 模块特有的数据 */
};

enum udi_ioctl_cmd_type {
    UDI_IOCTL_SET_WRITE_CB = 0xF001, /* 设置一个起始码值防止与系统定义冲突 */
    UDI_IOCTL_SET_READ_CB,

    UDI_IOCTL_INVAL_CMD = -1
};

/* ACM
 * IOCTL CMD 定义
 * UDI IOCTL 命令ID
 */
#define UDI_ACM_IOCTL_SET_READ_CB ACM_IOCTL_SET_READ_CB
#define UDI_ACM_IOCTL_GET_READ_BUFFER_CB ACM_IOCTL_GET_RD_BUFF
#define UDI_ACM_IOCTL_RETUR_BUFFER_CB ACM_IOCTL_RETURN_BUFF

#define UDI_ADC_IOCTL_SET_WRITE_CB 0X41000071
#define UDI_ADC_IOCTL_SET_READ_CB 0X41000072
#define UDI_ADC_IOCTL_GET_READ_BUFFER_CB 0X41000073
#define UDI_ADC_IOCTL_RETUR_BUFFER_CB 0X41000074

/* UART AT SIG DEFINE */
#define UDI_UART_IOCTL_SET_BAUD          0X7F001016

/* ACM MODEM SIG define */
#define SIGNALNOCH 0
#define SIGNALCH 1
#define RECV_ENABLE 1
#define RECV_DISABLE 0
#define SEND_ENABLE 1
#define SEND_DISABLE 0
#define HIGHLEVEL 1
#define LOWLEVEL 0

#define UDI_CMUX_IOCTL_RETURN_BUFF        0x7F001036
#define UDI_CMUX_IOCTL_WRITE_ASYNC        0x7F001037
#define UDI_CMUX_IOCTL_SET_READ_CB        0x7F001038
#define UDI_CMUX_IOCTL_GET_RD_BUFF        0x7F001039
#define UDI_CMUX_IOCTL_SET_FREE_CB        0x7F00103A
#define UDI_CMUX_IOCTL_SWITCH_MODE_CB     0x7F00103B
#define UDI_CMUX_IOCTL_CLOSE_PORT_CB      0x7F00103C
#define UDI_CMUX_IOCTL_SET_MSC_READ_CB    0x7F00103D
#define UDI_CMUX_IOCTL_MSC_MODE_READ_CB   0x7F00103E
#define UDI_CMUX_IOCTL_MSC_WRITE_CMD      0x7F00103F
#define UDI_UART_IOCTL_SLEEP_UNLOCK       0x7F001040
#define UDI_CMUX_IOCTL_SNC_CB             0x7F001041

/* ACM MODEM 管角信号结构体定义 */
struct modem_msc_stru {
    unsigned int op_dtr : 1;    /* DTR CHANGE FLAG */
    unsigned int op_dsr : 1;    /* DSR CHANGE FLAG */
    unsigned int op_cts : 1;    /* CTSCHANGE FLAG */
    unsigned int op_rts : 1;    /* RTS CHANGE FLAG */
    unsigned int op_ri : 1;     /* RI CHANGE FLAG */
    unsigned int op_dcd : 1;    /* DCD CHANGE FLAG */
    unsigned int op_fc : 1;     /* FC CHANGE FLAG */
    unsigned int op_brk : 1;    /* BRK CHANGE FLAG */
    unsigned int op_spare : 24; /* reserve */
    unsigned char uc_dtr;        /* DTR  VALUE */
    unsigned char uc_dsr;        /* DSR  VALUE */
    unsigned char uc_cts;        /* DTS VALUE */
    unsigned char uc_rts;        /* RTS  VALUE */
    unsigned char uc_ri;         /* RI VALUE */
    unsigned char uc_dcd;        /* DCD  VALUE */
    unsigned char uc_fc;         /* FC  VALUE */
    unsigned char uc_brk;        /* BRK  VALUE */
    unsigned char uc_brk_len;     /* BRKLEN VALUE */
};

typedef struct tagMODEM_MSC_STRU {
        u32 OP_Dtr:1;           /* DTR CHANGE FLAG */
        u32 OP_Dsr:1;           /* DSR CHANGE FLAG */
        u32 OP_Cts:1;           /* CTSCHANGE FLAG */
        u32 OP_Rts:1;           /* RTS CHANGE FLAG */
        u32 OP_Ri:1;            /* RI CHANGE FLAG */
        u32 OP_Dcd:1;           /* DCD CHANGE FLAG */
        u32 OP_Fc:1;            /* FC CHANGE FLAG */
        u32 OP_Brk:1;           /* BRK CHANGE FLAG */
        u32 OP_Spare:24;        /* reserve */
        u8   ucDtr;             /* DTR  VALUE */
        u8   ucDsr;             /* DSR  VALUE */
        u8   ucCts;             /* DTS VALUE */
        u8   ucRts;             /* RTS  VALUE */
        u8   ucRi;              /* RI VALUE */
        u8   ucDcd;             /* DCD  VALUE */
        u8   ucFc;              /* FC  VALUE */
        u8   ucBrk;             /* BRK  VALUE */
        u8   ucBrkLen;          /* BRKLEN VALUE */
} MODEM_MSC_STRU;

typedef void (*acm_modem_msc_read_cb)(MODEM_MSC_STRU *modem_msc);

/* NCM */
/* 上行线路收包函数指针 */
typedef void (*usb_uplink_rx_func)(int handle, void *pkt_node);

/* USB IOCTL枚举 */
enum ncm_ioctl_cmd_type {
    NCM_IOCTL_NETWORK_CONNECTION_NOTIF,      /* 0x0,NCM网络是否连接上 */
    NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF, /* 0x1,NCM设备协商的网卡速度 */
    NCM_IOCTL_IPV6_DNS_NOTIF,     /* 0x2 IPV6 DNS主动上报 */
    NCM_IOCTL_CLEAR_IPV6_DNS,     /* 0x3 清除IPV6 DNS在板端的缓存,param在此命令字没有意义，不填空指针即可 */
    NCM_IOCTL_FREE_BUFF,                     /* 0x4,释放底软buffer */
    NCM_IOCTL_FLOW_CTRL_NOTIF,               /* 0x5,流控控制开关 */
    NCM_IOCTL_AT_RESPONSE,                   /* 0x6,AT命令回应 */

    NCM_IOCTL_REG_UPLINK_RX_FUNC,            /* 0x7,注册上行收包回调函数 */
    NCM_IOCTL_REG_FREE_PKT_FUNC,             /* 0x8,注册释放包封装回调函数 */
    NCM_IOCTL_REG_AT_PROCESS_FUNC,           /* 0x9,注册AT命令处理回调函数 */
    NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC,     /* 0xa,注册网卡状态改变通知回调函数 */
    NCM_IOCTL_REG_NDIS_RESP_STATUS_FUNC, /* 0xb,NDIS通道AT命令状态处理回调函数 */
    
    NCM_IOCTL_GET_USED_MAX_BUFF_NUM,         /* 0xc,获取上层可以最多占用的ncm buffer个数 */
    NCM_IOCTL_GET_DEFAULT_TX_MIN_NUM,        /* 0xd,获取默认发包个数阈值，超过该阈值会启动一次NCM传输 */
    NCM_IOCTL_GET_DEFAULT_TX_TIMEOUT,        /* 0xe,获取默认发包超时时间，超过该时间会启动一次NCM传输 */
    NCM_IOCTL_GET_DEFAULT_TX_MAX_SIZE,       /* 0xf,获取默认发包字节阈值，超过该阈值会启动一次NCM传输 */
    NCM_IOCTL_GET_RX_BUF_SIZE,               /* 0x10,获取收包buffer大小 */
    NCM_IOCTL_GET_FLOWCTRL_STATUS,           /* 0x11,查询NCM流控状态 */
    NCM_IOCTL_GET_CUR_TX_MIN_NUM, /* 0x12 获取当前发包个数阈值 */
    NCM_IOCTL_GET_CUR_TX_TIMEOUT, /* 0x13 获取当前发包超时时间 */
    NCM_IOCTL_GET_NCM_STATUS,     /* 0x14 获取NCM网卡状态 enable:TRUE(1);disable:FALSE(0) */

    NCM_IOCTL_SET_PKT_ENCAP_INFO,            /* 0x15,设置包封装格式 */
    NCM_IOCTL_SET_TX_MIN_NUM,                /* 0x16,设置发包个数阈值，超过该阈值会启动一次NCM传输 */
    NCM_IOCTL_SET_TX_TIMEOUT,                /* 0x17,设置发包超时时间，超过该时间会启动一次NCM传输 */
    NCM_IOCTL_SET_TX_MAX_SIZE,               /* 0x18,该命令字不再使用。设置发包字节阈值，超过该阈值会启动一次NCM传输 */
    NCM_IOCTL_SET_PKT_STATICS,               /* 0x19,配置统计信息 */
    NCM_IOCTL_SET_IPV6_DNS,       /* 0x1a,配置IPV6 DNS */
    NCM_IOCTL_SET_ACCUMULATION_TIME,  /* 0x1b*/
    NCM_IOCTL_SET_RX_MIN_NUM, /*0x1c,配置收包个数阈值 */
    NCM_IOCTL_SET_RX_TIMEOUT, /*0x1d,配置收包超时时间 */
    NCM_IOCTL_SET_PDUSESSION  /*0x1f */
};

/* NCM连接状态枚举,NCM_IOCTL_NETWORK_CONNECTION_NOTIF命令字对应参数枚举 */
enum ncm_ioctl_connection_status {
    NCM_IOCTL_CONNECTION_LINKDOWN, /* NCM网络断开连接 */
    NCM_IOCTL_CONNECTION_LINKUP    /* NCM网络连接 */
};

/* NCM连接速度结构,NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF命令字对应参数结构体 */
struct ncm_ioctl_connection_speed {
    unsigned int down_bitrate;
    unsigned int up_bitrate;
};

/* AT命令回复数据指针及长度 NCM_IOCTL_AT_RESPONSE */
struct ncm_at_rsp {
    unsigned char *at_answer;
    unsigned int length;
};

/* AT命令接收函数指针，该函数为同步接口，对应NCM_IOCTL_REG_AT_PROCESS_FUNC命令字 */
typedef void (*usb_ndis_at_recv_func)(unsigned char *buf, unsigned int len);

/* NCM流控开关枚举,NCM_IOCTL_NETWORK_CONNECTION_NOTIF命令字对应参数枚举 */
enum ncm_ioctl_flow_ctrl {
    NCM_IOCTL_FLOW_CTRL_ENABLE, /* 打开流控 */
    NCM_IOCTL_FLOW_CTRL_DISABLE /* 关闭流控 */
};

/* NCM网卡状态改变通知枚举,NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC命令字对应参数枚举 */
enum ncm_ioctrl_connect_stus {
    NCM_IOCTL_STUS_CONNECT, /* 建链 */
    NCM_IOCTL_STUS_BREAK    /* 网卡断开,断链 */
};

/* 网卡状态切换通知函数，对应NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC命令字 */
typedef void (*usb_ndis_stus_chg_func)(enum ncm_ioctrl_connect_stus status, void *buff);

/* NDIS通道AT命令状态处理回调函数 */
typedef void (*usb_ndis_resp_status_cb)(void *buff, unsigned int status);

#define BSP_NCM_IPV6_DNS_LEN 32
struct ncm_ipv6_dns { /* 0x16 配置IPV6 DNS */
    unsigned char *ipv6_dns_info; /* 32字节，低16字节表示primaryDNS；高16字节表示SecondaryDNS。 */
    unsigned int length;
};

typedef int (*udi_adp_init_cb)(void);
typedef unsigned int (*udi_get_capability_cb)(enum udi_device_id dev_id);
typedef int (*udi_open_cb)(struct udi_open_param *param, int handle);
typedef int (*udi_close_cb)(void *private);
typedef int (*udi_write_cb)(void *private, void *mem_obj, unsigned int size);
typedef int (*udi_read_cb)(void *private, void *mem_obj, unsigned int size);
typedef int (*udi_ioctl_cb)(void *private, unsigned int cmd, void *param);

/* 驱动适配函数指针表 */
struct udi_drv_interface_table {
    /* capability */
    udi_adp_init_cb udi_get_capability_cb;

    /* data interfaces */
    udi_open_cb open_cb;
    udi_close_cb close_cb;
    udi_write_cb write_cb;
    udi_read_cb read_cb;
    udi_ioctl_cb ioctl_cb;
};

#ifdef CONFIG_UDI_SUPPORT
/*
 * 功能描述: 打开设备(数据通道)
 */
int mdrv_udi_open(struct udi_open_param *param);

/*
 * 功能描述: 关闭设备(数据通道)
 */
int mdrv_udi_close(int handle);

/*
 * 功能描述: 数据写
 *           pMemObj: buffer内存 或 内存链表对象
 *           u32Size: 数据写尺寸 或 内存链表对象可不设置
 */
int mdrv_udi_write(int handle, void *mem_obj, unsigned int size);

/*
 * 功能描述: 数据读
 *           pMemObj: buffer内存 或 内存链表对象
 *           u32Size: 数据读尺寸 或 内存链表对象可不设置
 */
int mdrv_udi_read(int handle, void *mem_obj, unsigned int size);

/*
 * 功能描述: 数据通道属性配置
 *           u32Cmd: IOCTL命令码
 *           pParam: 操作参数
 */
int mdrv_udi_ioctl(int handle, unsigned int cmd, void *param);
#else
static inline int mdrv_udi_open(struct udi_open_param *param)
{
    return 0;
}

static inline int mdrv_udi_close(int handle)
{
    return 0;
}

static inline int mdrv_udi_write(int handle, void *mem_obj, unsigned int size)
{
    return 0;
}

static inline int mdrv_udi_read(int handle, void *mem_obj, unsigned int size)
{
    return 0;
}

static inline int mdrv_udi_ioctl(int handle, unsigned int cmd, void *param)
{
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif

