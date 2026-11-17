/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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




/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <securec.h>
#include <bsp_icc.h>
#include <osl_types.h>
#include <osl_sem.h>
#include <nv_stru_drv.h>
#include <bsp_slice.h>
#include "diag_port_manager.h"
#include "diag_system_debug.h"
#include "scm_common.h"
#include "osl_spinlock.h"
#include "OmPortSwitch.h"
/* ****************************************************************************
  2 全局变量定义
**************************************************************************** */
PPM_PORT_SWITCH_NV_INFO ppm_port_switch;
/* 端口切换信息的数据结构体 */
PPM_PORT_CFG_INFO_STRU                  g_stPpmPortSwitchInfo;

extern DIAG_CHANNLE_PORT_CFG_STRU                g_stPortCfg;
extern spinlock_t                            g_stPpmPortSwitchSpinLock;

/*****************************************************************************
 函 数 名      : ppm_ap_nv_icc_read_cb
 功能描述  : 用于注册ICC的回调函数
 输入参数  : id: icc通道号
                         len: 数据长度
                         context: 数据内容
 输出参数  : 无
 返 回 值     : BSP_OK/BSP_ERROR
*****************************************************************************/
static s32 ppm_ap_nv_icc_read_cb(u32 id , u32 len, void* context)
{
    PPM_PORT_SWITCH_NV_INFO ppm_port_switch_back = {};
    u32 msg_len = sizeof(ppm_port_switch_back);
    s32 ret = 0;

    if(len != msg_len)
    {
        diag_error("readcb_len err, len=%d, msg_len=%d\n", len, msg_len);
        return BSP_ERROR;
    }
    ret = bsp_icc_read(id, (u8 *)(&ppm_port_switch_back), len);
    if(msg_len != (u32)ret)
    {
        diag_error("icc_read err(0x%x), msg_len=%d\n", ret, len);
        return BSP_ERROR;
    }
    if(ppm_port_switch_back.msgid != PPM_MSGID_PORT_SWITCH_NV_C2A)
    {
        diag_error("msgid err, msgid=%d\n", ppm_port_switch_back.msgid);
    }
    ppm_port_switch.ret = ppm_port_switch_back.ret;
    if(ppm_port_switch_back.ret)
    {
        diag_error("NV_Write fail,sn=%d\n", ppm_port_switch_back.sn);
    }

    return BSP_OK;
}



u32 PPM_SetIndMode(void)
{
    u32                         ulCompressState;
    u32                         ret;

    /* 为了规避USB输出时开启了延时写入无法连接工具,切换到USB输出时需要重新设置SOCP的超时中断到默认值 */
    if (CPM_OM_PORT_TYPE_USB == g_stPortCfg.enPortNum)
    {
        ulCompressState = mdrv_socp_compress_status();
        if(COMPRESS_ENABLE_STATE == ulCompressState)
        {
            ret= mdrv_socp_compress_disable(SOCP_CODER_DST_OM_IND);
            if(BSP_OK != ret)
            {
                diag_error("deflate disable failed(0x%x)\n", ret);
                return (u32)ret;
            }
        }
        ret = bsp_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
        if(BSP_OK != ret)
        {
            diag_error("set socp ind mode failed(0x%x)\n", ret);
            return (u32)ret;

        }
    }
    return ERR_MSP_SUCCESS;
}

u32 PPM_PhyPortSwitch(u32 ulPhyPort)
{
    CPM_PHY_PORT_ENUM_UINT32            enPhyCfgPort;
    CPM_PHY_PORT_ENUM_UINT32            enPhyIndPort;
    unsigned long                       ulLockLevel;
    bool                                ulSndMsg;

    enPhyCfgPort = CPM_QueryPhyPort(CPM_OM_CFG_COMM);
    enPhyIndPort = CPM_QueryPhyPort(CPM_OM_IND_COMM);

    ulSndMsg = false;

    scm_SpinLockIntLock(&g_stPpmPortSwitchSpinLock, ulLockLevel);

    /* 切换到VCOM输出 */
    if (CPM_OM_PORT_TYPE_VCOM == ulPhyPort)
    {
        /* 当前是USB输出 */
        if ((CPM_CFG_PORT == enPhyCfgPort) && (CPM_IND_PORT == enPhyIndPort))
        {
            /* 需要断开连接 */
            ulSndMsg = true;

            CPM_DisconnectPorts(CPM_CFG_PORT, CPM_OM_CFG_COMM);
            CPM_DisconnectPorts(CPM_IND_PORT, CPM_OM_IND_COMM);
        }

        /* 当前OM走VCOM上报 */
        CPM_ConnectPorts(CPM_VCOM_CFG_PORT, CPM_OM_CFG_COMM);
        CPM_ConnectPorts(CPM_VCOM_IND_PORT, CPM_OM_IND_COMM);

        g_stPortCfg.enPortNum = CPM_OM_PORT_TYPE_VCOM;
    }
    /* 切换到USB输出 */
    else
    {
        /* 当前是VCOM输出 */
        if ((CPM_VCOM_CFG_PORT == enPhyCfgPort) && (CPM_VCOM_IND_PORT == enPhyIndPort))
        {
            /* 断开连接 */
            ulSndMsg = true;

            CPM_DisconnectPorts(CPM_VCOM_CFG_PORT, CPM_OM_CFG_COMM);
            CPM_DisconnectPorts(CPM_VCOM_IND_PORT, CPM_OM_IND_COMM);
        }

        /* OM走USB上报 */
        CPM_ConnectPorts(CPM_CFG_PORT, CPM_OM_CFG_COMM);
        CPM_ConnectPorts(CPM_IND_PORT, CPM_OM_IND_COMM);

        g_stPortCfg.enPortNum = CPM_OM_PORT_TYPE_USB;
    }

    scm_SpinUnlockIntUnlock(&g_stPpmPortSwitchSpinLock, ulLockLevel);

    return ulSndMsg;
}
u32 PPM_WritePortNv(void)
{
    u32 ret;

    ppm_port_switch.msgid = PPM_MSGID_PORT_SWITCH_NV_A2C;
    ppm_port_switch.sn ++;
    /* 默认错误，根据返回值查看是否写NV成功 */
    ppm_port_switch.ret = (u32)BSP_ERROR;
    ppm_port_switch.len = sizeof(ppm_port_switch.data);
    memcpy_s((void *)(&ppm_port_switch.data), sizeof(ppm_port_switch.data), (void *)(&g_stPortCfg), sizeof(g_stPortCfg));
    ret = (u32)bsp_icc_send(ICC_CPU_MODEM, (ICC_CHN_IFC<<16 | IFC_RECV_FUNC_PPM_NV), (u8*)(&ppm_port_switch), sizeof(ppm_port_switch));
    if(ret != sizeof(ppm_port_switch))
    {
        diag_error("bsp_icc_send fail(0x%x)\n", ret);
        return (u32)BSP_ERROR;
    }

    return ERR_MSP_SUCCESS;
}
/*****************************************************************************
 函 数 名  : PPM_LogPortSwitch
 功能描述  : 提供给NAS进行端口切换
 输入参数  : enPhyPort: 带切换物理端口枚举值
             ulEffect:是否立即生效
 输出参数  : 无
 返 回 值  : BSP_ERROR/BSP_OK

 修改历史  :
*****************************************************************************/
u32 PPM_LogPortSwitch(u32  ulPhyPort, bool ulEffect)
{
    u32                           ret;

    if ((CPM_OM_PORT_TYPE_USB != ulPhyPort) && (CPM_OM_PORT_TYPE_VCOM != ulPhyPort))
    {
        diag_error("enPhyPort error, port=%d\n", ulPhyPort);

        g_stPpmPortSwitchInfo.ulPortTypeErr++;

        return (u32)BSP_ERROR;
    }

    diag_crit("ulPhyPort:%s ulEffect:%s.\n",
                     (ulPhyPort == CPM_OM_PORT_TYPE_USB) ? "USB"  : "VCOM",
                     (ulEffect  == true) ? "TRUE" : "FALSE");

    /* 切换的端口与当前端口一致不切换 */
    if (ulPhyPort == g_stPortCfg.enPortNum)
    {
        /* 为了规避USB输出时开启了延时写入无法连接工具,切换到USB输出时需要重新设置SOCP的超时中断到默认值 */
        if (CPM_OM_PORT_TYPE_USB == g_stPortCfg.enPortNum)
        {
            ret = bsp_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
            if(BSP_OK != ret)
            {
                diag_error("set socp ind mode failed(0x%x)\n", ret);
                return (u32)ret;
            }
        }
        diag_crit("Set same port, don't need to do anything.\n");

        return BSP_OK;
    }

    g_stPpmPortSwitchInfo.ulStartSlice = bsp_get_slice_value();

    /* port switch */
    if (PPM_PhyPortSwitch(ulPhyPort))
    {
        PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_CNF);
    }

    /* 为了规避USB输出时开启了延时写入无法连接工具,切换到USB输出时需要重新设置SOCP的超时中断到默认值 */
    ret = PPM_SetIndMode();
    if(ret)
    {
        return ret;
    }

    g_stPpmPortSwitchInfo.ulSwitchSucc++;
    g_stPpmPortSwitchInfo.ulEndSlice = bsp_get_slice_value();

    /* write Nv */
    if (true == ulEffect)
    {
        ret = PPM_WritePortNv();
        if(ret)
        {
            return ret;
        }
    }
    diag_crit("PPM set port success!\n");

    return BSP_OK;
}

u32 PPM_QueryLogPort(u32  *pulLogPort)
{
    if (NULL == pulLogPort)
    {
        diag_error("para is NULL\n");
        return (u32)BSP_ERROR;
    }
    *pulLogPort = g_stPortCfg.enPortNum;
    if ((CPM_OM_PORT_TYPE_USB != *pulLogPort) && (CPM_OM_PORT_TYPE_VCOM != *pulLogPort))
    {
        return (u32)BSP_ERROR;
    }

    return BSP_OK;
}

int PPM_PortSwtichInit(void)
{
    u32 ret = 0;

    (void)memset_s(&g_stPpmPortSwitchInfo, sizeof(g_stPpmPortSwitchInfo), 0, sizeof(g_stPpmPortSwitchInfo));

    ret = bsp_icc_event_register((ICC_CHN_IFC<<16 | IFC_RECV_FUNC_PPM_NV), (read_cb_func)ppm_ap_nv_icc_read_cb,  (void *)NULL, (write_cb_func)NULL, (void *)NULL);
    if(ret)
    {
        diag_error("icc_event_register fail, channel id:0x%x ret:0x%x\n", (ICC_CHN_IFC<<16 | IFC_RECV_FUNC_PPM_NV), ret);
        return BSP_ERROR;
    }
    ppm_port_switch.sn = 0;


    return BSP_OK;
}


void PPM_PortSwitchInfoShow(void)
{
    diag_crit("Port Type Err num is %d\n", g_stPpmPortSwitchInfo.ulPortTypeErr);

    diag_crit("Port Switch Fail time is %d\n", g_stPpmPortSwitchInfo.ulSwitchFail);

    diag_crit("Port Switch Success time is %d\n", g_stPpmPortSwitchInfo.ulSwitchSucc);

    diag_crit("Port Switch Start slice is 0x%x\n", g_stPpmPortSwitchInfo.ulStartSlice);

    diag_crit("Port Switch End slice is 0x%x.\n", g_stPpmPortSwitchInfo.ulEndSlice);

    return;
}
EXPORT_SYMBOL(PPM_PortSwitchInfoShow);

EXPORT_SYMBOL(PPM_LogPortSwitch);

