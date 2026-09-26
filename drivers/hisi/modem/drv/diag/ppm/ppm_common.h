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

#ifndef __OM_COMMON_PPM_H__
#define __OM_COMMON_PPM_H__

#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include <bsp_print.h>
#include <nv_stru_drv.h>
#include "ppm_usb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

#define USB_MAX_DATA_LEN (1024 * 1024) /* USB发送的最大数据长度 */
#define BIT_N(num) (0x01 << (num))

#define OM_ACPU_RECV_USB BIT_N(0)
#define OM_ACPU_DISPATCH_MSG BIT_N(1)
#define OM_ACPU_SEND_SOCP BIT_N(2)
#define OM_ACPU_SEND_USB BIT_N(3)
#define OM_ACPU_USB_CB BIT_N(4)
#define OM_ACPU_SEND_USB_IND BIT_N(5)
#define OM_ACPU_ERRLOG_SEND BIT_N(6)
#define OM_ACPU_ERRLOG_RCV BIT_N(7)
#define OM_ACPU_ERRLOG_PRINT BIT_N(8)
#define OM_ACPU_RECV_SOCKET BIT_N(9)
#define OM_ACPU_SEND_SOCKET BIT_N(10)
#define OM_ACPU_DATA BIT_N(11)
#define OM_ACPU_READ_DONE BIT_N(12)

#define OM_SOCP_CNF_BUFFER_SIZE (8 * 1024)
#define OM_SOCP_CNF_BUFFER_NUM 8

#define OM_SOCP_IND_BUFFER_SIZE (2 * 1024)
#define OM_SOCP_IND_BUFFER_NUM 2

/* 物理通道类型枚举 */
typedef enum {
    CPM_OM_PORT_TYPE_USB = 0,
    CPM_OM_PORT_TYPE_VCOM = 1,
    CPM_OM_PORT_TYPE_WIFI = 2,
    CPM_OM_PORT_TYPE_SD = 3,
    CPM_OM_PORT_TYPE_FS = 4,
    CPM_OM_PORT_TYPE_PCDEV = 6,
    CPM_OM_PORT_TYPE_BUTT
} cpm_om_port_type_e;

/* UDI设备句柄 */
typedef enum {
    OM_USB_IND_PORT_HANDLE = 0,
    OM_USB_CFG_PORT_HANDLE = 1,
    OM_USB_CBT_PORT_HANDLE = 2,
    OM_HSIC_IND_PORT_HANDLE = 3,
    OM_HSIC_CFG_PORT_HANDLE = 4,
    OM_PCDEV_IND_PORT_HANDLE = 5,
    OM_PCDEV_CFG_PORT_HANDLE = 6,
    OM_PORT_HANDLE_BUTT /* OM_PORT_HANDLE_NUM = OM_PORT_HANDLE_BUTT，如果更改此枚举注意更改OM_PORT_HANDLE_NUM */
} ppm_port_handle_e;

typedef struct {
    u8 *virt_addr;
    u8 *phy_addr;
    s32 length;
} ppm_buff_info_s;

typedef struct {
    u8 *pucAsyncCBData; /* DRV_UDI_IOCTL接口异步返回后传入的数据指针 */
    u32 length;         /* DRV_UDI_IOCTL接口异步返回后返回的实际处理数据长度 */
    u32 ulRsv;          /* Reserve */
    ppm_buff_info_s last_ind_send;
} ppm_pseudo_sync_s;

typedef enum {
    AT_UART_PORT = 0,
    AT_PCUI_PORT,
    AT_CTRL_PORT,
    AT_HSUART_PORT,
    AT_PCUI2_PORT,
    AT_PORT_BUTT
} at_phy_port_e;

typedef enum {
    CPM_IND_PORT = AT_PORT_BUTT, /* OM数据上报端口 */
    CPM_CFG_PORT,                /* OM配置端口 */
    CPM_SD_PORT,
    CPM_WIFI_OM_IND_PORT, /* WIFI下OM数据上报端口 */
    CPM_WIFI_OM_CFG_PORT, /* WIFI下OM配置下发端口 */
    CPM_WIFI_AT_PORT,     /* WIFI下AT端口 */
    CPM_HSIC_IND_PORT,
    CPM_HSIC_CFG_PORT,
    CPM_VCOM_IND_PORT, /* VCOM上OM数据上报接口 */
    CPM_VCOM_CFG_PORT, /* VCOM上OM配置接口 */
    CPM_FS_PORT,
    CPM_PCDEV_IND_PORT,
    CPM_PCDEV_CFG_PORT,
    CPM_PORT_BUTT
} cpm_phy_port_e;

typedef struct {
    u32 msg_id;
    u32 timestamp;
    u32 data;
} diag_port_command_s;

typedef struct {
    u32 msg_id;
    s32 result;
    u32 timestamp;
    u32 data[0];
} diag_port_command_resp_s;

u32 ppm_read_port_data(unsigned int phy_port, void *udi_handle, unsigned int ppm_handle);
void ppm_port_write_asy_cb(unsigned int ppm_handle, ppm_buff_info_s *ret_addr);
void ppm_read_port_data_init(unsigned int phy_port, unsigned int ppm_handle, void *read_cb, void *write_cb,
    void *state_cb);
void ppm_port_close_proc(unsigned int handle, unsigned int phy_port);
u32 ppm_port_send(unsigned int handle, u8 *virt_addr, u8 *phy_addr, u32 data_len);
void ppm_port_status(unsigned int handle, unsigned int phy_port, ACM_EVT_E port_state);
void ppm_get_send_data_len(socp_coder_dst_e chan, u32 data_len, u32 *send_len, unsigned int *phy_port);
u32 ppm_port_init(void);
int ppm_init_phy_port(void);
void ppm_port_info_show(unsigned int handle);
void ppm_debug_port_info_show(void);
void ppm_port_switch_info_show(void);
ppm_pseudo_sync_s *ppm_get_cfg_pseudo_sync_info(void);
ppm_pseudo_sync_s *ppm_get_ind_pseudo_sync_info(void);
mdrv_ppm_chan_debug_info_s *ppm_get_chan_debug_info(void);
void *ppm_get_udi_handle(unsigned int port_handle);
spinlock_t *ppm_get_portswitch_lock(void);
#ifdef DIAG_SYSTEM_FUSION
u32 ppm_port_commad_proc(const u8 *port_command_msg);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of OmCommonPpm.h */
