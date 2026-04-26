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

#ifndef __OM_SOCKET_PPM_H__
#define __OM_SOCKET_PPM_H__

#include <linux/net.h>
#include <linux/in.h>
#include <mdrv.h>
#include <osl_types.h>
#include <osl_sem.h>
#include "mdrv_socket.h"
#include "ppm_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

#define SOCKET_NUM_MAX 1
#define SOCK_WIFI_ADDR_LEN 16
#define SOCK_WIFI_DEFAULT_ADDR "127.0.0.1"
#define SOCK_OM_IND_SRC_PORT_NUM 20253
#define SOCK_OM_CFG_SRC_PORT_NUM 3000
#define SOCK_AT_SRC_PORT_NUM 20249
#define SOCK_OM_MSG_LEN 1000
#define SOCK_NULL (-1)
#define INVALID_SOCKET BSP_ERROR
#define SOCKET_ERROR BSP_ERROR

#define SOCK_FD_ISSET(d, set) (HI_FD_ISSET(d, set))
#define SOCK_FD_SET(d, set) (HI_FD_SET(d, set))
#define SOCK_FD_CLR(d, set) (HI_FD_CLR(d, set))
#define SOCK_FD_ZERO(set) (mdrv_sock_fd_zero(set))

typedef enum {
    SOCK_OK = 0,
    SOCK_NO_START,
    SOCK_START,
    SOCK_UDP_INIT_ERR,
    SOCK_TCP_INIT_ERR,
    SOCK_BIND_ERR,
    SOCK_LISTEN_ERR,
    SOCK_SELECT_ERR,
    SOCK_ACCEPT_ERR,
    SOCK_SEND_ERR,
    SOCK_BUTT
} sock_errno_type_e;

typedef enum {
    SOCKET_OM_IND,
    SOCKET_OM_CFG,
    SOCKET_AT,
    SOCKET_BUTT
} socket_chan_type_e;

typedef struct {
    int socket;
    int listener;
    osl_sem_id sem_id;
    unsigned int phy_port;
    u32 sock_port;
    s8 *sock_buff;
} socket_ctrl_info_s;

typedef struct {
    bool status;
    struct sockaddr_in *sock_addr;
    u32 total_len;
} socket_udp_info_s;

u32 ppm_socket_port_init(void);
u32 ppm_socket_udp_init(void);
bool ppm_socket_is_enable(void);
u32 ppm_socket_get_udp_info(void);
u32 ppm_socket_task_init(void);
u32 ppm_socket_at_recv(u8 *data, u32 len);
void ppm_socket_recv_cb_reg(unsigned int logic_port, cpm_rcv_cb recv_func);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of OmCommonPpm.h */
