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
#include "ppm_port_switch.h"
#include <securec.h>
#include <osl_spinlock.h>
#include <osl_types.h>
#include <osl_sem.h>
#include <bsp_icc.h>
#include <nv_stru_drv.h>
#include <bsp_slice.h>
#include "bsp_socp.h"
#include "diag_port_manager.h"
#include "scm_common.h"
#include "scm_rate_ctrl.h"
#include "diag_system_debug.h"


ppm_port_switch_nv_info_s g_ppm_port_switch_info;
ppm_port_cfg_info_s g_stPpmPortSwitchInfo; /* 端口切换信息的数据结构体 */

u32 ppm_set_ind_mode(void)
{
    u32 ret;
    DIAG_CHANNLE_PORT_CFG_STRU *diag_port_cfg = cpm_get_port_cfg();

    /* 为了规避USB输出时开启了延时写入无法连接工具,切换到USB输出时需要重新设置SOCP的超时中断到默认值 */
    if (diag_port_cfg->enPortNum == CPM_OM_PORT_TYPE_USB) {
        ret = bsp_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
        if (ret) {
            diag_error("set socp ind mode failed(0x%x)\n", ret);
            return (u32)ret;
        }
    }
    return ERR_MSP_SUCCESS;
}

u32 ppm_phy_port_switch(u32 phy_port)
{
    unsigned int cfg_port = cpm_query_phy_port(CPM_OM_CFG_COMM);
    unsigned int ind_port = cpm_query_phy_port(CPM_OM_IND_COMM);
    unsigned long lock;
    bool send_flag = false;
    unsigned int original_port;
    DIAG_CHANNLE_PORT_CFG_STRU *diag_port_cfg = cpm_get_port_cfg();
    spinlock_t *ppm_port_switch_spinlock = ppm_get_portswitch_lock();

    original_port = cpm_get_original_port();

    spin_lock_irqsave(ppm_port_switch_spinlock, lock);

    if (phy_port == CPM_OM_PORT_TYPE_VCOM) {
        /* 当前是USB输出 */
        if ((cfg_port == CPM_CFG_PORT) && (ind_port == CPM_IND_PORT)) {
            send_flag = true;

            if ((cfg_port == CPM_CFG_PORT) && (ind_port == CPM_IND_PORT)) {
                cpm_disconnect_ports(CPM_CFG_PORT, CPM_OM_CFG_COMM);
                cpm_disconnect_ports(CPM_IND_PORT, CPM_OM_IND_COMM);
            } else if ((cfg_port == CPM_WIFI_OM_CFG_PORT) && (ind_port == CPM_WIFI_OM_IND_PORT)) {
                cpm_disconnect_ports(CPM_WIFI_OM_CFG_PORT, CPM_OM_CFG_COMM);
                cpm_disconnect_ports(CPM_WIFI_OM_IND_PORT, CPM_OM_IND_COMM);
            } else {
                diag_error("exceptional cfg port = 0x%x, ind port=0x%x\n", cfg_port, ind_port);
            }
        }

        /* 当前OM走VCOM上报 */
        cpm_connect_ports(CPM_VCOM_CFG_PORT, CPM_OM_CFG_COMM);
        cpm_connect_ports(CPM_VCOM_IND_PORT, CPM_OM_IND_COMM);

        diag_port_cfg->enPortNum = CPM_OM_PORT_TYPE_VCOM;
        scm_set_rate_limit(SCM_RATE_PORT_VCOM);
    } else { /* 切换到USB||SOCKET输出 */
        /* 当前是VCOM输出 */
        if ((cfg_port == CPM_VCOM_CFG_PORT) && (ind_port == CPM_VCOM_IND_PORT)) {
            /* 断开连接 */
            send_flag = true;

            cpm_disconnect_ports(CPM_VCOM_CFG_PORT, CPM_OM_CFG_COMM);
            cpm_disconnect_ports(CPM_VCOM_IND_PORT, CPM_OM_IND_COMM);
        }

        if ((original_port == CPM_OM_PORT_TYPE_USB) || (original_port == CPM_OM_PORT_TYPE_VCOM)) {
            cpm_connect_ports(CPM_CFG_PORT, CPM_OM_CFG_COMM);
            cpm_connect_ports(CPM_IND_PORT, CPM_OM_IND_COMM);

            diag_port_cfg->enPortNum = CPM_OM_PORT_TYPE_USB;
            diag_crit("change port to 0x%x\n", diag_port_cfg->enPortNum);
        } else if (original_port == CPM_OM_PORT_TYPE_WIFI) {
            cpm_connect_ports(CPM_WIFI_OM_CFG_PORT, CPM_OM_CFG_COMM);
            cpm_connect_ports(CPM_WIFI_OM_IND_PORT, CPM_OM_IND_COMM);

            diag_port_cfg->enPortNum = CPM_OM_PORT_TYPE_WIFI;
            diag_crit("change port to 0x%x\n", diag_port_cfg->enPortNum);
        } else {
            diag_error("exceptional port = 0x%x\n", diag_port_cfg->enPortNum);
        }
        scm_set_rate_limit(SCM_RATE_PORT_USB);
    }

    spin_unlock_irqrestore(ppm_port_switch_spinlock, lock);

    return send_flag;
}
u32 ppm_write_port_nv(void)
{
    u32 ret;
    DIAG_CHANNLE_PORT_CFG_STRU *diag_port_cfg = cpm_get_port_cfg();

    g_ppm_port_switch_info.msg_id = PPM_MSGID_PORT_SWITCH_NV_A2C;
    g_ppm_port_switch_info.sn++;
    /* 默认错误，根据返回值查看是否写NV成功 */
    g_ppm_port_switch_info.ret = (u32)BSP_ERROR;
    g_ppm_port_switch_info.len = sizeof(g_ppm_port_switch_info.data);
    memcpy_s((void *)(&g_ppm_port_switch_info.data), sizeof(g_ppm_port_switch_info.data), (void *)diag_port_cfg,
        sizeof(DIAG_CHANNLE_PORT_CFG_STRU));
    ret = (u32)bsp_icc_send(ICC_CPU_MODEM, (ICC_CHN_IFC << 16 | IFC_RECV_FUNC_PPM_NV), (u8 *)(&g_ppm_port_switch_info),
        sizeof(g_ppm_port_switch_info));
    if (ret != sizeof(g_ppm_port_switch_info)) {
        diag_error("bsp_icc_send fail(0x%x)\n", ret);
        return (u32)ERR_MSP_INALID_LEN_ERROR;
    }

    return ERR_MSP_SUCCESS;
}
/*
 * 函 数 名: ppm_port_switch
 * 功能描述: 提供给NAS进行端口切换
 * 输入参数: phy_port: 带切换物理端口枚举值
 * ulEffect:是否立即生效
 */
u32 ppm_port_switch(u32 phy_port)
{
    u32 ret;
    DIAG_CHANNLE_PORT_CFG_STRU *diag_port_cfg = cpm_get_port_cfg();

    if ((phy_port != CPM_OM_PORT_TYPE_USB) && (phy_port != CPM_OM_PORT_TYPE_VCOM)) {
        diag_error("phy_port error, port=%d\n", phy_port);

        g_stPpmPortSwitchInfo.port_type_err++;
        return (u32)ERR_MSP_INVALID_PARAMETER;
    }

    diag_crit("phy_port:%s\n", (phy_port == CPM_OM_PORT_TYPE_USB) ? "USB" : "VCOM");

    /* 切换的端口与当前端口一致不切换 */
    if (phy_port == diag_port_cfg->enPortNum) {
        /* 为了规避USB输出时开启了延时写入无法连接工具,切换到USB输出时需要重新设置SOCP的超时中断到默认值 */
        if (diag_port_cfg->enPortNum == CPM_OM_PORT_TYPE_USB) {
            ret = bsp_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
            if (ret != BSP_OK) {
                diag_error("set socp ind mode failed(0x%x)\n", ret);
                return (u32)ret;
            }
        }
        diag_crit("Set same port, don't need to do anything.\n");

        return BSP_OK;
    }

    g_stPpmPortSwitchInfo.start_slice = bsp_get_slice_value();

    /* port switch */
    if (ppm_phy_port_switch(phy_port)) {
        ppm_disconnect_port(CPM_OM_CFG_COMM);
    }

    /* 为了规避USB输出时开启了延时写入无法连接工具,切换到USB输出时需要重新设置SOCP的超时中断到默认值 */
    ret = ppm_set_ind_mode();
    if (ret) {
        return ret;
    }

    g_stPpmPortSwitchInfo.switch_succ++;
    g_stPpmPortSwitchInfo.end_slice = bsp_get_slice_value();

    /* write Nv */
    ret = ppm_write_port_nv();
    if (ret) {
        return ret;
    }
    diag_crit("PPM set port success!\n");

    return BSP_OK;
}
/*
 * 函 数 名: ppm_query_port
 * 功能描述: 提供给NAS进行Log端口查询
 * 输入参数: 无
 * 输出参数: pulLogPort当前Log输出端口
 * 返 回 值: BSP_ERROR/BSP_OK
 */
u32 ppm_query_port(u32 *phy_port)
{
    DIAG_CHANNLE_PORT_CFG_STRU *diag_port_cfg = NULL;

    if (phy_port == NULL) {
        diag_error("para is NULL\n");
        return (u32)ERR_MSP_PARA_NULL;
    }

    diag_port_cfg = cpm_get_port_cfg();
    *phy_port = diag_port_cfg->enPortNum;

    if ((*phy_port != CPM_OM_PORT_TYPE_USB) && (*phy_port != CPM_OM_PORT_TYPE_VCOM)) {
        return (u32)ERR_MSP_INVALID_PARAMETER;
    }

    return BSP_OK;
}
u32 bsp_diag_set_log_port(u32 phy_port)
{
    return ppm_port_switch(phy_port);
}
u32 bsp_diag_get_log_port(u32 *phy_port)
{
    return ppm_query_port(phy_port);
}
int ppm_port_switch_init(void)
{
    (void)memset_s(&g_stPpmPortSwitchInfo, sizeof(g_stPpmPortSwitchInfo), 0, sizeof(g_stPpmPortSwitchInfo));

    g_ppm_port_switch_info.sn = 0;

    return BSP_OK;
}
ppm_port_cfg_info_s *ppm_get_switch_debug_info(void)
{
    return &g_stPpmPortSwitchInfo;
}

/*
 * 函 数 名: ppm_port_switch_info_show
 * 功能描述: 用于打印端口切换的统计信息
 */
void ppm_port_switch_info_show(void)
{
    diag_crit("Port Type Err num is %d\n", g_stPpmPortSwitchInfo.port_type_err);

    diag_crit("Port Switch Fail time is %d\n", g_stPpmPortSwitchInfo.switch_fail);

    diag_crit("Port Switch Success time is %d\n", g_stPpmPortSwitchInfo.switch_succ);

    diag_crit("Port Switch Start slice is 0x%x\n", g_stPpmPortSwitchInfo.start_slice);

    diag_crit("Port Switch End slice is 0x%x.\n", g_stPpmPortSwitchInfo.end_slice);

    return;
}
EXPORT_SYMBOL(ppm_port_switch_info_show);
EXPORT_SYMBOL(ppm_port_switch);

