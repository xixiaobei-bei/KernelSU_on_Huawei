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

#ifndef __DIAG_PORT_MANAGER__
#define __DIAG_PORT_MANAGER__

#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>
#include "ppm_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push)
#pragma pack(4)

#define CPM_LOGIC_PHY_PORT(logic_port) (g_cpm_logic_port_cfg[logic_port].phy_port)
#define CPM_LOGIC_SEND_FUNC(logic_port) (g_cpm_logic_port_cfg[logic_port].send_func)
#define CPM_LOGIC_RCV_FUNC(logic_port) (g_cpm_logic_port_cfg[logic_port].rcv_func)

#define CPM_PHY_SEND_FUNC(phy_port) (g_cpm_phy_port_cfg[phy_port].send_func)
#define CPM_PHY_RCV_FUNC(phy_port) (g_cpm_phy_port_cfg[phy_port].rcv_func)

typedef enum {
    CPM_SEND_OK,
    CPM_SEND_FUNC_NULL,
    CPM_SEND_PARA_ERR,
    CPM_SEND_ERR,
    CPM_SEND_AYNC,
    CPM_SEND_BUTT
} cpm_send_result_e;

typedef u32 (*cpm_send_func)(u8 *virt_addr, u8 *phy_addr, u32 len);

typedef struct {
    cpm_send_func send_func;
    cpm_rcv_cb rcv_func;
} cpm_phy_port_cfg_s;

typedef struct {
    unsigned int phy_port;
    cpm_send_func send_func;
    cpm_rcv_cb rcv_func;
} cpm_logic_port_cfg_s;

typedef struct {
    u32 para_err;
    u32 null_err;
} cpm_com_port_err_info_s;

typedef struct {
    u32 port_err;
    cpm_com_port_err_info_s cpm_rcv_err_info[CPM_PORT_BUTT - CPM_IND_PORT];
} cpm_com_port_rcv_err_info_s;

typedef struct {
    u32 port_err;
    cpm_com_port_err_info_s cpm_send_err_info[CPM_COMM_BUTT];
} cpm_com_port_send_err_info_s;

void cpm_set_original_port(unsigned int port);
unsigned int cpm_get_original_port(void);
void cpm_phy_send_reg(unsigned int phy_port, cpm_send_func send_func);
void cpm_logic_rcv_reg(unsigned int logic_port, cpm_rcv_cb rcv_func);
void cpm_connect_ports(unsigned int phy_port, unsigned int logic_port);
void cpm_disconnect_ports(unsigned int phy_port, unsigned int logic_port);
int cpm_port_associate_init(void);
int cpm_hybrid_port_associate_init(void);
int cpm_asic_port_associate_init(void);
u32 cpm_com_send(unsigned int logic_port, u8 *virt_addr, u8 *phy_addr, u32 len);
u32 cpm_com_receive(unsigned int phy_port, u8 *addr, u32 len);
unsigned int cpm_query_phy_port(unsigned int logic_port);
DIAG_CHANNLE_PORT_CFG_STRU *cpm_get_port_cfg(void);
void cpm_show(void);
void cpm_com_err_show(void);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of cpm.h */
