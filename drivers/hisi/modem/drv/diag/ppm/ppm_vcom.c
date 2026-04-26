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
#include "ppm_vcom.h"
#include <product_config.h>
#include <mdrv_diag_system.h>
#include <securec.h>
#include <bsp_slice.h>
#include "diag_port_manager.h"
#include "ppm_port_switch.h"
#include "diag_system_debug.h"


/*
 * 2 全局变量定义
 */
/* 用于记录 VCOM 通道发送的统计信息 */
mdrv_ppm_vcom_debug_info_s g_ppm_vcom_debug_info[3] = {};

/*
 * 函 数 名: ppm_vcom_cfg_send_data
 * 功能描述: 从VCOM端口发送配置数据
 * 输入参数: pucVirAddr:   数据虚地址
 * pucPhyAddr:   数据实地址
 * data_len: 数据长度
 */
u32 ppm_vcom_cfg_send_data(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_num++;
    g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_len += len;

    if (PPM_VCOM_SEND_DATA(PPM_VCOM_CHAN_CTRL, virt_addr, len, PPM_VCOM_DATA_MODE_TRANSPARENT) != 0) {
        g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_err_num++;
        g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_err_len += len;

        diag_error("vcom cnf cmd failed, cnf sum len 0x%x, cnf err len 0x%x.\n",
            g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_len,
            g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_err_len);

        return CPM_SEND_ERR;
    }

    /* 与手机软件连接时，启动延时上报，且打印到缓存中，可以输出打印 */
    diag_crit("vcom cnf cmd success, cnf sum len 0x%x, cnf err len 0x%x.\n",
        g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_len, g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_err_len);

    return CPM_SEND_OK;
}

/*
 * 函 数 名: PPM_VComCfgEvtCB
 * 功能描述: 用于VCOM通道打开关闭回调
 * 输入参数: chan_id :通道号
 * ulEvent:   打开或者关闭
 */
void ppm_vcom_event_cb(u32 chan, u32 event)
{
    unsigned int logic_port;
    bool send_flag = false;

    diag_crit("ppm_vcom_event_cb Chan:%s Event:%s.\n", (chan == PPM_VCOM_CHAN_CTRL) ? "cnf" : "ind",
        (event == PPM_VCOM_EVT_CHAN_OPEN) ? "open" : "close");

    if (chan == PPM_VCOM_CHAN_CTRL) {
        logic_port = CPM_OM_CFG_COMM;
    } else if (chan == PPM_VCOM_CHAN_DATA) {
        logic_port = CPM_OM_IND_COMM;
    } else {
        diag_error("Error channel NO %d\n", chan);
        return;
    }

    /* 打开操作直接返回 */
    if (event == PPM_VCOM_EVT_CHAN_OPEN) {
        diag_crit("ppm_vcom_event_cb open, do nothing.\n");
        return;
    } else if (event == PPM_VCOM_EVT_CHAN_CLOSE) {
        send_flag = false;

        if ((cpm_query_phy_port(CPM_OM_CFG_COMM) == CPM_VCOM_CFG_PORT) &&
            (cpm_query_phy_port(CPM_OM_IND_COMM) == CPM_VCOM_IND_PORT)) {
            send_flag = true;
        }
        if (send_flag == true) {
            diag_crit("ppm_vcom_event_cb close, disconnect all ports.\n");
            ppm_disconnect_port(logic_port);
        }
    } else {
        diag_error("Error Event State %d\n", event);
    }

    return;
}

/*
 * 函 数 名: ppm_vcom_cfg_read_data
 * 功能描述: NAS把收到数据调用OM 接口发走
 * 输入参数:  ucDevIndex: 物理端口
 * data    : 收到数据
 * uslength : 数据长度
 */
u32 ppm_vcom_cfg_read_data(u32 index, u8 *data, u32 len)
{
    u32 ret;

    if (index != PPM_VCOM_CHAN_CTRL) {
        diag_error("PhyPort port is error: %d\n", index);

        return ERR_MSP_FAILURE;
    }

    g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_num++;
    g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_len += len;

    if ((data == NULL) || (len == 0)) {
        diag_error("Send data is NULL\n");

        return ERR_MSP_FAILURE;
    }

    /* 与手机软件连接时，下发命令有限，且打印到缓存中，可以输出打印 */
    diag_crit("vcom receive cmd, length : 0x%x, sum length : 0x%x.\n", len,
        g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_len);

    ret = cpm_com_receive(CPM_VCOM_CFG_PORT, data, len);
    if (ret != BSP_OK) {
        diag_error("cpm_com_receive error(0x%x)\n", ret);
        g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_err_num++;
        g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_err_len += len;
    }

    return BSP_OK;
}

/*
 * 函 数 名: ppm_vcom_ind_send_data
 * 功能描述: Vcom口承载的OM IND端口收到数据，调NAS接收
 * 输入参数: pucVirAddr:   数据虚地址
 * pucPhyAddr:   数据实地址
 * data_len:    数据长度
 */
u32 ppm_vcom_ind_send_data(u8 *virt_addr, u8 *phy_addr, u32 len)
{
    u32 in_slice;
    u32 out_slice;
    u32 write_slice;
    int ret;

    g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_num++;
    g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_len += len;

    in_slice = bsp_get_slice_value();
    ret = PPM_VCOM_SEND_DATA(PPM_VCOM_CHAN_DATA, virt_addr, len, PPM_VCOM_DATA_MODE_TRANSPARENT);

    diag_system_debug_vcom_len(len);
    diag_system_debug_send_data_end();
    out_slice = bsp_get_slice_value();

    write_slice = (in_slice > out_slice) ? (0xffffffff - in_slice + out_slice) : (out_slice - in_slice);

    if (write_slice > g_ppm_vcom_debug_info[CPM_OM_IND_COMM].max_time_len) {
        g_ppm_vcom_debug_info[CPM_OM_IND_COMM].max_time_len = write_slice;
    }

    if (ret != 0) {
        g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_err_num++;
        g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_err_len += len;

        diag_system_debug_vcom_fail_len(len);

        return CPM_SEND_ERR;
    }
    g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_sn++;
    diag_system_debug_vcom_sucess_len(len);

    return CPM_SEND_OK;
}

mdrv_ppm_vcom_debug_info_s *ppm_vcom_get_ind_info(void)
{
    return &g_ppm_vcom_debug_info[CPM_OM_IND_COMM];
}

/*
 * 函 数 名: ppm_vcom_cfg_port_init
 * 功能描述: 用于 Vcom 口OM CFG通道的初始化
 */
void ppm_vcom_cfg_port_init(void)
{
    struct diag_vcom_cb_ops ops;

    ops.event_cb = ppm_vcom_event_cb;
    ops.data_rx_cb = ppm_vcom_cfg_read_data;
    diag_vcom_register_ops(PPM_VCOM_CHAN_CTRL, &ops);

    cpm_phy_send_reg(CPM_VCOM_CFG_PORT, ppm_vcom_cfg_send_data);

    return;
}

/*
 * 函 数 名: ppm_vcom_ind_port_init
 * 功能描述: 用于 Vcom 口OM IND通道的初始化
 */
void ppm_vcom_ind_port_init(void)
{
    struct diag_vcom_cb_ops ops;

    ops.event_cb = ppm_vcom_event_cb;
    ops.data_rx_cb = NULL;
    diag_vcom_register_ops(PPM_VCOM_CHAN_DATA, &ops);

    cpm_phy_send_reg(CPM_VCOM_IND_PORT, ppm_vcom_ind_send_data);

    return;
}

/*
 * 函 数 名: ppm_vcom_port_init
 * 功能描述: 用于 Vcom 口各通道的初始化
 */
void ppm_vcom_port_init(void)
{
    (void)memset_s(g_ppm_vcom_debug_info, sizeof(g_ppm_vcom_debug_info), 0, sizeof(g_ppm_vcom_debug_info));

    /* Vcom 口OM IND通道的初始化 */
    ppm_vcom_ind_port_init();

    /* Vcom 口OM CNF通道的初始化 */
    ppm_vcom_cfg_port_init();

    return;
}

/*
 * 函 数 名: ppm_vcom_info_show
 * 功能描述: 用于打印 Vcom 口通道发送信息
 */
void ppm_vcom_info_show(void)
{
    diag_crit(" VCom28 Send num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_num);
    diag_crit(" VCom28 Send Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_len);

    diag_crit(" VCom28 Send Error num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_err_num);
    diag_crit(" VCom28 Send Error Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_send_err_len);

    diag_crit(" VCom28 receive num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_num);
    diag_crit(" VCom28 receive Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_len);

    diag_crit(" VCom28 receive Error num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_err_num);
    diag_crit(" VCom28 receive Error Len is %d.\n\n", g_ppm_vcom_debug_info[CPM_OM_CFG_COMM].vcom_rcv_err_len);

    diag_crit(" VCom31 Send num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_num);
    diag_crit(" VCom31 Send Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_len);
    diag_crit(" VCom31 Send sn is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_sn);

    diag_crit(" VCom31 Send Error num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_err_num);
    diag_crit(" VCom31 Send Error Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_send_err_len);

    diag_crit(" VCom31 receive num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_rcv_num);
    diag_crit(" VCom31 receive Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_rcv_len);

    diag_crit(" VCom31 receive Error num is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_rcv_err_num);
    diag_crit(" VCom31 receive Error Len is %d.\n", g_ppm_vcom_debug_info[CPM_OM_IND_COMM].vcom_rcv_err_len);

    return;
}
EXPORT_SYMBOL(ppm_vcom_info_show);

