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

/*
 * 1 头文件包含
 */
#include "ppm_socket.h"
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <bsp_nvim.h>
#include <osl_thread.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <nv_stru_drv.h>
#include <acore_nv_stru_drv.h>
#include <securec.h>
#include "mdrv_socket.h"
#include "diag_port_manager.h"
#include "scm_common.h"
#include "ppm_common.h"
#include "ppm_port_switch.h"
#include "bsp_version.h"
#include "diag_system_debug.h"


socket_ctrl_info_s g_ppm_socket_info[SOCKET_BUTT] = {
    { SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_OM_IND_PORT, SOCK_OM_IND_SRC_PORT_NUM, NULL },
    { SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_OM_CFG_PORT, SOCK_OM_CFG_SRC_PORT_NUM, NULL },
    { SOCK_NULL, SOCK_NULL, {}, CPM_WIFI_AT_PORT, SOCK_AT_SRC_PORT_NUM, NULL }
};

#if ((defined(FEATURE_HISOCKET)) && (FEATURE_HISOCKET == FEATURE_ON)) || (defined(FEATURE_SVLSOCKET))
/* 保存当前SOCKET的状态 */
u32 g_ppm_socket_state = SOCK_OK;

/* 自旋锁，用来作自处理任务的临界资源保护 */
spinlock_t g_ppm_socket_task_spinlock;

/* 保存当前SOCKETOM的初始化状态 */
u32 g_ppm_diag_init_state = false;

/* 保存当前SOCKETAT的初始化状态 */
u32 g_ppm_at_init_state = false;

u32 g_ppm_diag_conn_state = false;
u32 g_ppm_at_conn_state = false;

socket_udp_info_s g_ppm_udp_info = {
    false,
};
cpm_rcv_cb g_at_rcv_func;

bool ppm_socket_is_enable(void)
{
    DIAG_CHANNLE_PORT_CFG_STRU port_cfg = { 0 };
    s32 ret;
    /* 读取OM的物理输出通道 */
    ret = bsp_nvm_read(NV_ID_DRV_DIAG_PORT, (u8 *)&port_cfg, sizeof(DIAG_CHANNLE_PORT_CFG_STRU));
    if (ret) {
        diag_error("nvm read fail, ret = 0x%x\n", ret);
        return false;
    }

    /* 检测参数 */
    if (port_cfg.enPortNum != CPM_OM_PORT_TYPE_WIFI) {
        diag_error("exceptional port 0x%x\n", port_cfg.enPortNum);
        return false;
    }

    return true;
}

u32 ppm_socket_udp_init(void)
{
    socket_ctrl_info_s *socket_info = g_ppm_socket_info + SOCKET_OM_IND;
    socket_info->socket = mdrv_socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_info->socket < BSP_OK) {
        return BSP_ERROR;
    }
    /* 创建SOCKET保护信号量 */
    if (socket_info->phy_port == CPM_WIFI_OM_IND_PORT) {
        osl_sem_init(1, &socket_info->sem_id);
    } else {
        return BSP_ERROR;
    }

    return BSP_OK;
}

bool ppm_socket_tcp_init(socket_ctrl_info_s *socket_info)
{
    /* 创建SOCKET保护信号量 */
    if (socket_info->phy_port == CPM_WIFI_OM_CFG_PORT) {
        osl_sem_init(1, &socket_info->sem_id);
    } else if (socket_info->phy_port == CPM_WIFI_AT_PORT) {
        osl_sem_init(1, &socket_info->sem_id);
    } else {
        return false;
    }
    return true;
}

void ppm_socket_bind_listen(socket_ctrl_info_s *socket_info)
{
    struct sockaddr_in *sock_addr = NULL;
    s32 sock_len;
    errno_t err;

    if ((socket_info->listener != SOCK_NULL) || (socket_info->socket != SOCK_NULL)) {
        return;
    }

    socket_info->listener = mdrv_socket(AF_INET, SOCK_STREAM, 0);

    if (socket_info->listener < BSP_OK) {
        diag_error("mdrv_socket err!\n");
        socket_info->listener = SOCK_NULL;
        msleep(100); /* delay 100 ms */
        return;
    }

    sock_addr = osl_malloc(sizeof(struct sockaddr_in));
    if (sock_addr == NULL) {
        return;
    }

    err = memset_s(sock_addr, sizeof(*sock_addr), 0, sizeof(*sock_addr));
    if (err != EOK) {
        diag_error("memset err %x!\n", err);
    }
    sock_addr->sin_family = AF_INET;

    sock_addr->sin_addr.s_addr = 0;

    /* 监听的端口号 */
    sock_addr->sin_port = htons(socket_info->sock_port);

    sock_len = sizeof(struct sockaddr_in);

    /* 将监听Socket绑定到对应的端口上 */
    if (mdrv_bind(socket_info->listener, (struct sockaddr *)sock_addr, sock_len) == SOCKET_ERROR) {
        diag_error("socket shut down, listenr=0x%x\n", socket_info->listener);
        mdrv_shutdown(socket_info->listener, SHUT_RDWR);
        mdrv_close(socket_info->listener);
        socket_info->listener = SOCK_NULL;
        osl_free(sock_addr);
        diag_error("mdrv_bind err !\n");
        return;
    }

    /* 设置服务器端监听的最大客户端数 */
    if (mdrv_listen(socket_info->listener, SOCKET_NUM_MAX) == SOCKET_ERROR) {
        diag_error("socket shut down, listenr=0x%x\n", socket_info->listener);
        mdrv_shutdown(socket_info->listener, SHUT_RDWR);
        mdrv_close(socket_info->listener);
        socket_info->listener = SOCK_NULL;
        diag_error("mdrv_listen err !\n");
        osl_free(sock_addr);
        return;
    }

    osl_free(sock_addr);
    return;
}

void ppm_socket_accept_rcv(socket_ctrl_info_s *socket_info)
{
    hi_fd_set *listener = NULL;
    s32 ret;
    s32 *len = NULL;
    struct sockaddr_in *from = NULL;
    unsigned long lock;
    int socket;
    int max_sock;

    if ((socket_info->listener == SOCK_NULL) && (socket_info->socket == SOCK_NULL)) {
        diag_error("listener && socket err ! %d\n", socket_info->sock_port);
        return;
    }

    listener = osl_malloc(sizeof(hi_fd_set));
    if (listener == NULL) {
        diag_error("listener malloc fail\n");
        return;
    }
    from = osl_malloc(sizeof(struct sockaddr_in));
    if (from == NULL) {
        diag_error("from malloc fail\n");
        osl_free(listener);
        return;
    }

    mdrv_sock_fd_zero(listener);

    HI_FD_SET((u32)socket_info->listener, listener);

    if (socket_info->socket != SOCK_NULL) {
        HI_FD_SET((u32)socket_info->socket, listener);
    }

    max_sock = socket_info->socket > socket_info->listener ? socket_info->socket : socket_info->listener;

    ret = mdrv_select((int)max_sock + 1, listener, NULL, NULL, NULL);
    if (ret < 0) {
        diag_error("select err ! %d \n", socket_info->sock_port);
        osl_free(listener);
        osl_free(from);
        return;
    }

    if ((socket_info->listener != SOCK_NULL) && (HI_FD_ISSET((u32)socket_info->listener, listener))) {
        len = (s32 *)osl_malloc(sizeof(s32));
        if (len == NULL) {
            diag_error("len malloc fail\n");
            osl_free(listener);
            osl_free(from);
            return;
        }

        *len = (s32)sizeof(struct sockaddr_in);
        socket = mdrv_accept(socket_info->listener, (struct sockaddr *)from, len);
        if (socket < 0) {
            diag_error("socket shut down, listenr=0x%x\n", socket_info->listener);
            mdrv_shutdown(socket_info->listener, SHUT_RDWR);
            mdrv_close(socket_info->listener);
            socket_info->listener = SOCK_NULL;
            diag_error("mdrv_accept err, port = %d \n", socket_info->sock_port);
        } else {
            if (socket_info->socket != SOCK_NULL) {
                if (socket_info->phy_port == CPM_WIFI_OM_CFG_PORT) {
                    g_ppm_diag_conn_state = true;
                }
                if (socket_info->phy_port == CPM_WIFI_AT_PORT) {
                    g_ppm_at_conn_state = true;
                }
                (void)osl_sem_down(&(socket_info->sem_id));

                mdrv_shutdown(socket_info->socket, SHUT_RDWR);
                mdrv_close(socket_info->socket);

                socket_info->socket = socket;

                osl_free(len);
                osl_free(listener);
                osl_free(from);
                (void)osl_sem_up(&(socket_info->sem_id));

                return;
            }

            (void)osl_sem_down(&(socket_info->sem_id));
            socket_info->socket = socket;
            (void)osl_sem_up(&(socket_info->sem_id));

            if (socket_info->phy_port == CPM_WIFI_OM_CFG_PORT) {
                if (mdrv_getpeername(socket_info->socket, (struct sockaddr *)g_ppm_udp_info.sock_addr, len) < 0) {
                    diag_error("mdrv_getpeername failed.\n");
                } else {
                    /* 获取对端地址后，修改端口号，用于UDP传输 */
                    g_ppm_udp_info.sock_addr->sin_port = htons(g_ppm_socket_info[SOCKET_OM_IND].sock_port);
                    g_ppm_udp_info.status = true;
                }
            }
        }
        osl_free(len);
    }

    if (socket_info->socket == SOCK_NULL) {
        diag_error("socket=SOCK_NULL, Port=%d!\n", socket_info->sock_port);
        osl_free(listener);
        osl_free(from);
        return;
    }

    ret = mdrv_recv((int)socket_info->socket, socket_info->sock_buff, SOCK_OM_MSG_LEN, 0);
    if (ret <= 0) { /* 客户端断开之后服务端会持续受到长度为0的数据包 */
        (void)osl_sem_down(&(socket_info->sem_id));

        diag_error("socket shut down, listenr=0x%x\n", socket_info->listener);
        mdrv_shutdown(socket_info->socket, SHUT_RDWR);
        mdrv_close(socket_info->socket);
        socket_info->socket = SOCK_NULL;
        diag_error("socket_info->socket = 0x%x\n", socket_info->socket);

        (void)osl_sem_up(&(socket_info->sem_id));

        if (socket_info->phy_port == CPM_WIFI_OM_CFG_PORT) {
            ppm_disconnect_port(CPM_OM_CFG_COMM);

            g_ppm_udp_info.status = false;
        }
        diag_error("rcv err(0x%x), port=%d\n", ret, socket_info->sock_port);
        osl_free(listener);
        osl_free(from);
        return;
    }

    spin_lock_irqsave(&g_ppm_socket_task_spinlock, lock);
    /* 将接收到的数据提交给上层处理 */
    ret = (s32)cpm_com_receive(socket_info->phy_port, socket_info->sock_buff, ret);
    if (ret) {
        diag_error("cpm_com_receive error(0x%x)\n", ret);
    }
    spin_unlock_irqrestore(&g_ppm_socket_task_spinlock, lock);

    osl_free(listener);
    osl_free(from);
    return;
}

void ppm_socket_srv_proc(socket_ctrl_info_s *socket_info)
{
    for (;;) {
        ppm_socket_bind_listen(socket_info);
        ppm_socket_accept_rcv(socket_info);
    }
}

void ppm_socket_diag_srv_task(void)
{
    /* 初始化CFG口的SOCKET，采用TCP协议 */
    if (ppm_socket_tcp_init(g_ppm_socket_info + SOCKET_OM_CFG) == false) {
        return;
    }

    g_ppm_socket_state = SOCK_START;

    g_ppm_diag_init_state = true;
    /* CFG口自处理循环处理入口 */
    ppm_socket_srv_proc(g_ppm_socket_info + SOCKET_OM_CFG);

    return;
}

void ppm_socket_at_srv_task(void)
{
    /* 初始化SOCKET */
    if (ppm_socket_tcp_init(g_ppm_socket_info + SOCKET_AT) == false) {
        return;
    }

    g_ppm_at_init_state = true;

    ppm_socket_srv_proc(g_ppm_socket_info + SOCKET_AT);

    return;
}

/* UDP单包包长最大65535字节，减去UDP包头后最大65507个字节 */
#define UDP_MAX_LEN 65507

/* 获取UDP端口发送的信息 */
u32 ppm_socket_get_udp_info(void)
{
    return g_ppm_udp_info.total_len;
}

u32 ppm_socket_ind_send(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    s32 real_send_len;
    u32 send_len = 0;
    u8 *tmpaddr = virt_addr;
    struct hisk_info hiskinfo = { 0 };

    (void)osl_sem_down(&(g_ppm_socket_info[SOCKET_OM_IND].sem_id));

    hiskinfo.fd = g_ppm_socket_info[SOCKET_OM_IND].socket;

    if ((hiskinfo.fd == SOCK_NULL) || (g_ppm_udp_info.status == false)) {
        (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_OM_IND].sem_id));
        return BSP_ERROR;
    }

    while (len) {
        if (len > UDP_MAX_LEN) {
            send_len = UDP_MAX_LEN;
        } else {
            send_len = len;
        }

        diag_system_debug_socket_len(send_len);

        hiskinfo.buff = tmpaddr;
        hiskinfo.len = send_len;
        real_send_len = mdrv_sendto(&hiskinfo, 0, \
                (struct sockaddr *)g_ppm_udp_info.sock_addr, sizeof(struct sockaddr_in));
        if (real_send_len != send_len) {
            diag_system_debug_socket_fail_len(send_len);

            diag_error("uslength %d, nSndNum %d.\n", send_len, real_send_len);

            (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_OM_IND].sem_id));

            return BSP_ERROR;
        } else {
            diag_system_debug_socket_sucess_len(send_len);
            g_ppm_udp_info.total_len += real_send_len;
        }

        len -= send_len;
        tmpaddr += send_len;
    }

    diag_system_debug_send_data_end();

    (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_OM_IND].sem_id));

    return BSP_OK;
}

u32 ppm_socket_cfg_send(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    int socket;
    s32 real_send_len;

    if (g_ppm_diag_init_state == false) {
        return BSP_ERROR;
    }

    (void)osl_sem_down(&(g_ppm_socket_info[SOCKET_OM_CFG].sem_id));

    socket = g_ppm_socket_info[SOCKET_OM_CFG].socket;

    if (socket == SOCK_NULL) {
        (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_OM_CFG].sem_id));
        diag_error("socket err !\n");
        return BSP_ERROR;
    }
    /* 调用send将数据通过socket发送出去，走TCP */
    real_send_len = mdrv_send(socket, virt_addr, len, 0);

    (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_OM_CFG].sem_id));

    if (real_send_len != len) {
        diag_error("mdrv_send err %d,%d!\n", real_send_len, len);
        if (g_ppm_diag_conn_state == true) {
            diag_crit("Re Conn %d,%d!\n", real_send_len, len);
            return BSP_ERROR;
        }
        ppm_disconnect_port(CPM_OM_CFG_COMM);

        g_ppm_udp_info.status = false;

        (void)osl_sem_down(&(g_ppm_socket_info[SOCKET_OM_CFG].sem_id));

        diag_error("socket shut down, socket=0x%x\n", g_ppm_socket_info[SOCKET_OM_CFG].socket);
        mdrv_shutdown(g_ppm_socket_info[SOCKET_OM_CFG].socket, SHUT_RDWR);
        mdrv_close(g_ppm_socket_info[SOCKET_OM_CFG].socket);

        g_ppm_socket_info[SOCKET_OM_CFG].socket = SOCK_NULL;

        (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_OM_CFG].sem_id));

        return BSP_ERROR;
    }

    g_ppm_diag_conn_state = false;

    return BSP_OK;
}

u32 ppm_socket_at_send(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    int socket;
    s32 real_send_len;

    if (g_ppm_at_init_state == false) {
        return BSP_ERROR;
    }

    (void)osl_sem_down(&(g_ppm_socket_info[SOCKET_AT].sem_id));

    socket = g_ppm_socket_info[SOCKET_AT].socket;

    if (socket == SOCK_NULL) {
        (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_AT].sem_id));
        diag_error("socket err !\n");
        return BSP_ERROR;
    }

    /* 调用send将数据通过socket发送出去，走TCP */
    real_send_len = mdrv_send(socket, virt_addr, len, MSG_DONTWAIT);

    (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_AT].sem_id));
    if (real_send_len != len) {
        diag_error("mdrv_send err %d,%d!\n", real_send_len, len);
        if (g_ppm_at_conn_state == true) {
            diag_crit("Re Conn %d,%d!\n", real_send_len, len);
            return BSP_ERROR;
        }
        (void)osl_sem_down(&(g_ppm_socket_info[SOCKET_AT].sem_id));
        mdrv_shutdown(g_ppm_socket_info[SOCKET_AT].socket, SHUT_RDWR);
        mdrv_close(g_ppm_socket_info[SOCKET_AT].socket);
        g_ppm_socket_info[SOCKET_AT].socket = SOCK_NULL;
        (void)osl_sem_up(&(g_ppm_socket_info[SOCKET_AT].sem_id));
        return BSP_ERROR;
    }

    g_ppm_at_conn_state = false;

    return BSP_OK;
}

static u32 ppm_hybrid_socket_port_init(void)
{
    if (ppm_socket_task_init()) {
        return BSP_ERROR;
    }

    cpm_phy_send_reg(CPM_WIFI_OM_IND_PORT, (cpm_send_func)ppm_socket_ind_send);
    cpm_phy_send_reg(CPM_WIFI_OM_CFG_PORT, (cpm_send_func)ppm_socket_cfg_send);
    cpm_phy_send_reg(CPM_WIFI_AT_PORT, (cpm_send_func)ppm_socket_at_send);

    /* 由于初始化顺序问题，AT的通道接收函数需要在此处注册，后续需要将AT通道交接给NAS */
    cpm_logic_rcv_reg(CPM_AT_COMM, ppm_socket_at_recv);

    spin_lock_init(&g_ppm_socket_task_spinlock);

    diag_crit("ppm socket hybrid init ok\n");

    return BSP_OK;
}

static u32 ppm_asic_socket_port_init(void)
{
    if (ppm_socket_task_init()) {
        return BSP_ERROR;
    }

    if ((mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) && (ppm_socket_is_enable() == true)) {
        cpm_phy_send_reg(CPM_WIFI_OM_IND_PORT, (cpm_send_func)ppm_socket_ind_send);
        cpm_phy_send_reg(CPM_WIFI_OM_CFG_PORT, (cpm_send_func)ppm_socket_cfg_send);
        cpm_phy_send_reg(CPM_WIFI_AT_PORT, (cpm_send_func)ppm_socket_at_send);

        /* 由于初始化顺序问题，AT的通道接收函数需要在此处注册，后续需要将AT通道交接给NAS */
        cpm_logic_rcv_reg(CPM_AT_COMM, ppm_socket_at_recv);
    }

    spin_lock_init(&g_ppm_socket_task_spinlock);

    diag_crit("ppm socket asic init ok\n");

    return BSP_OK;
}

u32 ppm_socket_port_init(void)
{
    u32 ret;
    const bsp_version_info_s *version_info = bsp_get_version_info();

    if (version_info == NULL) {
        diag_error("get version info is null\n");
        return BSP_ERROR;
    }
    if ((version_info->plat_type == PLAT_ASIC) || (version_info->plat_type == PLAT_EMU) ||
        (version_info->plat_type == PLAT_FPGA)) {
        ret = ppm_asic_socket_port_init();
    } else {
        ret = ppm_hybrid_socket_port_init();
    }

    return ret;
}

static u32 ppm_hybrid_socket_task_init(void)
{
    u32 ret;
    OSL_TASK_ID task_id = 0;

    /* 接收SOCKET数据的自处理任务 */
    ret = (u32)osl_task_init("om_server", 76, 8096, (OSL_TASK_FUNC)ppm_socket_diag_srv_task, NULL,
        (OSL_TASK_ID *)&task_id);
    if (ret) {
        diag_error("creta sock om server task fail,ret = 0x%x\n", ret);
        return BSP_ERROR;
    }

    /* 接收SOCKET数据的自处理任务 */
    ret = (u32)osl_task_init("om_server", 76, 8096, (OSL_TASK_FUNC)ppm_socket_at_srv_task,
        NULL, (OSL_TASK_ID *)&task_id);
    if (ret) {
        diag_error("creta sock at server task fail, ret = 0x%x\n", ret);
        return BSP_ERROR;
    }

    ret = ppm_socket_udp_init();
    if (ret) {
        diag_error("udp socket init fail, ret = 0x%x\n", ret);
        return BSP_ERROR;
    }

    return BSP_OK;
}

static u32 ppm_asic_socket_task_init(void)
{
    u32 ret;
    OSL_TASK_ID task_id = 0;

    if ((mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) && (ppm_socket_is_enable() == true)) {
        /* 接收SOCKET数据的自处理任务 */
        ret = (u32)osl_task_init("om_server", 76, 8096, (OSL_TASK_FUNC)ppm_socket_diag_srv_task, NULL,
            (OSL_TASK_ID *)&task_id);
        if (ret) {
            diag_error("creta sock om server task fail,ret = 0x%x\n", ret);
            return BSP_ERROR;
        }

        /* 接收SOCKET数据的自处理任务 */
        ret = (u32)osl_task_init("om_server", 76, 8096, (OSL_TASK_FUNC)ppm_socket_at_srv_task, NULL,
            (OSL_TASK_ID *)&task_id);
        if (ret) {
            diag_error("creta sock at server task fail, ret = 0x%x\n", ret);
            return BSP_ERROR;
        }

        ppm_socket_udp_init();

        return BSP_OK;
    }

    return BSP_ERROR;
}

u32 ppm_socket_task_init(void)
{
    u32 ret;
    u32 i;
    const bsp_version_info_s *version_info = bsp_get_version_info();

    g_ppm_udp_info.sock_addr = (struct sockaddr_in *)osl_malloc(sizeof(struct sockaddr_in));
    if (g_ppm_udp_info.sock_addr == NULL) {
        return ERR_MSP_MALLOC_FAILUE;
    }

    for (i = SOCKET_OM_IND; i < SOCKET_BUTT; i++) {
        g_ppm_socket_info[i].sock_buff = (s8 *)osl_malloc(SOCK_OM_MSG_LEN);
        if (g_ppm_socket_info[i].sock_buff == NULL) {
            return ERR_MSP_MALLOC_FAILUE;
        }
    }

    if (version_info == NULL) {
        diag_error("get version info is null\n");
        return BSP_ERROR;
    }

    if ((version_info->plat_type == PLAT_ASIC) || (version_info->plat_type == PLAT_EMU) ||
        (version_info->plat_type == PLAT_FPGA)) {
        ret = ppm_asic_socket_task_init();
    } else {
        ret = ppm_hybrid_socket_task_init();
    }

    return ret;
}

void ppm_socket_recv_cb_reg(unsigned int logic_port, cpm_rcv_cb recv_func)
{
    if (logic_port != CPM_AT_COMM) {
        diag_error("logic num error, 0x%x\n", logic_port);
    } else {
        g_at_rcv_func = recv_func;
    }
    return;
}

u32 ppm_socket_at_recv(u8 *data, u32 len)
{
    if (g_at_rcv_func) {
        return g_at_rcv_func(data, len);
    } else {
        diag_error("at rev fuc is null\n");
        return ERR_MSP_DIAG_CB_NULL_ERR;
    }
}
#else
void ppm_socket_recv_cb_reg(unsigned int logic_port, cpm_rcv_cb rcv_func)
{
    return;
}
u32 ppm_socket_at_recv(u8 *data, u32 len)
{
    return 0;
}
#endif

