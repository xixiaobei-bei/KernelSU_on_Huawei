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

/******************************************************************************
   1 头文件包含
******************************************************************************/
#include "RnicEntity.h"
#include "RnicDebug.h"
#include "RnicCtx.h"
#include "RnicDev.h"
#include <linux/version.h>
#include <net/sock.h>

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_RNIC_DEV_C

/******************************************************************************
   2 外部函数变量声明
******************************************************************************/

/******************************************************************************
   3 私有定义
******************************************************************************/

/******************************************************************************
   4 全局变量定义
*****************************************************************************/
/* 将网卡的操作进行静态映射 */
    static const struct net_device_ops rnic_ops = {
           .ndo_stop                = RNIC_StopNetCard,
           .ndo_open                = RNIC_OpenNetCard,
           .ndo_start_xmit          = RNIC_StartXmit,
           .ndo_set_mac_address     = RNIC_SetMacAddress,
           .ndo_change_mtu          = RNIC_ChangeMtu,
           .ndo_tx_timeout          = RNIC_ProcTxTimeout,
           .ndo_do_ioctl            = RNIC_Ioctrl,
           .ndo_get_stats           = RNIC_GetNetCardStats,
    };

#define BIND_VFILE_CRT_LEVEL            (0660)
#define RNIC_BIND_PROC_FILE_LEN         (6)                                     /* 5个字节pid字符串+1个字符结束符 */

VOS_UINT32                              g_ulBindPid = 0;

static const struct file_operations     g_stBindPidOps =
{
    .owner      = THIS_MODULE,
    .write      = RNIC_WriteBindPidFile,
    .read       = RNIC_ReadBindPidFile,
};

#define RNIC_DEV_NAME_PREFIX            "rmnet"
const RNIC_NETCARD_ELEMENT_TAB_STRU           g_astRnicManageTbl[RNIC_NET_ID_MAX_NUM] =
{
    {
      "0",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x06}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x01}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x06}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x01}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_0, {0, 0, 0, 0, 0}},

    { "1",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x07}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x02}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x07}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x02}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_1, {0, 0, 0, 0, 0}},

    { "2",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x08}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x03}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x08}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x03}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_2, {0, 0, 0, 0, 0}},

    { "3",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x09}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x04}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x09}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x04}, 0xDD86, {0, 0}},
      MODEM_ID_1, RNIC_RMNET_ID_3, {0, 0, 0, 0, 0}},

    { "4",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0a}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x05}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0a}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x05}, 0xDD86, {0, 0}},
      MODEM_ID_1, RNIC_RMNET_ID_4, {0, 0, 0, 0, 0}},

    { "5",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0b}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x06}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0b}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x06}, 0xDD86, {0, 0}},
      MODEM_ID_2, RNIC_RMNET_ID_5, {0, 0, 0, 0, 0}},

    { "6",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0c}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x07}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0c}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x07}, 0xDD86, {0, 0}},
      MODEM_ID_2, RNIC_RMNET_ID_6, {0, 0, 0, 0, 0}},


    { "_ims00",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0d}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x08}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0d}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x08}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_IMS00, {0, 0, 0, 0, 0}},

    { "_r_ims00",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0e}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x09}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0e}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x09}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_R_IMS00, {0, 0, 0, 0, 0}},

    { "_r_ims01",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0f}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0a}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0f}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0a}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_R_IMS01, {0, 0, 0, 0, 0}},

    { "_ims10",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x10}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0b}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x10}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0b}, 0xDD86, {0, 0}},
      MODEM_ID_1, RNIC_RMNET_ID_IMS10, {0, 0, 0, 0, 0}},

    { "_r_ims10",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x11}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0c}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x11}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0c}, 0xDD86, {0, 0}},
      MODEM_ID_1, RNIC_RMNET_ID_R_IMS10, {0, 0, 0, 0, 0}},

    { "_r_ims11",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x12}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0d}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x12}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0d}, 0xDD86, {0, 0}},
      MODEM_ID_1, RNIC_RMNET_ID_R_IMS11, {0, 0, 0, 0, 0}},

    { "_tun00",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x13}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0e}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x13}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0e}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN00, {0, 0, 0, 0, 0}},

    { "_tun01",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x14}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0f}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x14}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x0f}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN01, {0, 0, 0, 0, 0}},

    { "_tun02",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x15}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x10}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x15}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x10}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN02, {0, 0, 0, 0, 0}},

    { "_tun03",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x16}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x11}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x16}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x11}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN03, {0, 0, 0, 0, 0}},

    { "_tun04",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x17}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x12}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x17}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x12}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN04, {0, 0, 0, 0, 0}},

    { "_tun10",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x18}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x13}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x18}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x13}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN10, {0, 0, 0, 0, 0}},

    { "_tun11",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x19}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x14}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x19}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x14}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN11, {0, 0, 0, 0, 0}},

    { "_tun12",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1a}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x15}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1a}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x15}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN12, {0, 0, 0, 0, 0}},

    { "_tun13",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1b}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x16}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1b}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x16}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN13, {0, 0, 0, 0, 0}},

    { "_tun14",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1c}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x17}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1c}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x17}, 0xDD86, {0, 0}},
      MODEM_ID_BUTT, RNIC_RMNET_ID_TUN14, {0, 0, 0, 0, 0}},


    {
      "_emc0",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1d}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x18}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x1d}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x18}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_EMC0, {0, 0, 0, 0, 0}},
};

/******************************************************************************
   5 函数实现
******************************************************************************/

VOS_INT RNIC_StopNetCard(
    struct net_device                  *pstNetDev
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* 获取网卡私有数据 */
    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    napi_disable(&pstPriv->stNapi);

    /* 使用netif_stop_queue()停止网卡的数据收发 */
    netif_stop_queue(pstNetDev);

    /* 将私有数据中的网卡状态标志置为关闭 */
    pstPriv->enStatus = RNIC_NETCARD_STATUS_CLOSED;

    return 0;
}


VOS_INT RNIC_OpenNetCard(
    struct net_device                  *pstNetDev
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* 获取网卡私有数据 */
    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    napi_enable(&pstPriv->stNapi);

    /* 启动网卡接收数据 */
    netif_start_queue(pstNetDev);

    /* 将私有数据中的网卡状态标志置为打开 */
    pstPriv->enStatus = RNIC_NETCARD_STATUS_OPENED;

    return 0;
}


netdev_tx_t RNIC_StartXmit(
    struct sk_buff                     *pstSkb,
    struct net_device                  *pstNetDev
)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;

    pstPriv     = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    pstNetCntxt = RNIC_GetNetCntxtByRmNetId(pstPriv->enRmNetId);
    if (VOS_NULL_PTR == pstNetCntxt)
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_NET_TX_RMNETID_ERR_NUM(1);
        return NETDEV_TX_OK;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
    /* The value is a bit-shift of 1 second,
     * so 4 is ~64ms of queued data. Only affects local TCP
     * sockets.
     */
    sk_pacing_shift_update(pstSkb->sk, 4);
#endif

    RNIC_ProcessTxData(pstSkb, pstNetCntxt);

    return NETDEV_TX_OK;
}


VOS_INT RNIC_SetMacAddress(
    struct net_device                  *pstNetDev,
    VOS_VOID                           *pMacAddr
)
{
    struct sockaddr                    *pstAddr;
    VOS_INT                             lValid;

    /* 类型转换 */
    pstAddr = (struct sockaddr *)pMacAddr;

    /* MAC地址为无效值 */
    lValid = is_valid_ether_addr((VOS_UINT8 *)pstAddr->sa_data);
    if (VOS_FALSE == lValid)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_SetMacAddress: Ether addr is invalid!");
        return -EINVAL;
    }

    /* 将MAC地址赋值到网卡 */
    memcpy(pstNetDev->dev_addr, pstAddr->sa_data, pstNetDev->addr_len);

    return 0;
}


VOS_INT RNIC_ChangeMtu(
    struct net_device                  *pstNetDev,
    VOS_INT                             lNewMtu
)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;

    pstPriv     = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    pstNetCntxt = RNIC_GetNetCntxtByRmNetId(pstPriv->enRmNetId);

    if (VOS_NULL_PTR == pstNetCntxt)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_ChangeMtu: enRmNetId is invalid!");
        return -EINVAL;
    }

    if (RNIC_RMNET_R_IS_VALID(pstNetCntxt->enRmNetId))
    {
        if (lNewMtu > RNIC_R_IMS_MAX_PACKET)
        {
            RNIC_DEV_ERR_PRINTK("RNIC_ChangeMtu: R_IMS Mtu out of range!");
            return -EINVAL;
        }
    }
    else
    {
        /* new_mtu长度不在有效范围内 */
        if (lNewMtu > RNIC_MAX_PACKET)
        {
            RNIC_DEV_ERR_PRINTK("RNIC_ChangeMtu: Mtu out of range!");
            return -EINVAL;
        }
    }

    /* 网卡mtu完成赋值 */
    pstNetDev->mtu = (VOS_UINT)lNewMtu;

    return 0;
}


VOS_VOID RNIC_ProcTxTimeout(
    struct net_device                  *pstNetDev
)
{
    return;
}


VOS_INT RNIC_Ioctrl(
    struct net_device                  *pstNetDev,
    struct ifreq                       *pstIfr,
    VOS_INT                             lCmd
)
{
    return -EOPNOTSUPP;
}


struct net_device_stats *RNIC_GetNetCardStats(
    struct net_device                  *pstNetDev
)

{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* 获取私有数据 */
    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    /* 将设备的统计信息返回 */
    return &pstPriv->stStats;
}


VOS_VOID RNIC_DeinitNetCard(
    struct net_device                  *pstNetDev
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;

    /* 获取网卡私有数据 */
    pstPriv     = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(pstPriv->enRmNetId);

    /* 关闭载波 */
    netif_carrier_off(pstNetDev);

    /* 停止网卡接收数据 */
    netif_stop_queue(pstNetDev);

    /* 去注册网卡设备 */
    unregister_netdev(pstNetDev);

    /* 释放网卡设备 */
    free_netdev(pstNetDev);

    /* 清除网卡设备信息 */
    pstNetCntxt->pstPriv = VOS_NULL_PTR;

    return;
}


VOS_INT __init RNIC_InitNetCard(VOS_VOID)
{
    struct net_device                  *pstDev      = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    VOS_INT                             lRegRst;
    VOS_UINT8                           ucIndex;

    printk("RNIC_InitNetCard: entry, %u", VOS_GetSlice());

    for (ucIndex = 0 ; ucIndex < RNIC_NET_ID_MAX_NUM ; ucIndex++)
    {
        /* 初始化网卡私有信息 */
        pstNetCntxt = RNIC_GET_SPEC_NET_CTX(ucIndex);
        pstNetCntxt->pstPriv = VOS_NULL_PTR;

        /* 创建网卡设备netdev和私有数据 */
        /* alloc_etherdev这个接口分配了net_device和 RNIC_NETCARD_DEV_INFO_STRU两个接口的内存
           RNIC_NETCARD_DEV_INFO_STRU结构挂接在net_device的后面 */
        pstDev = alloc_etherdev(sizeof(RNIC_NETCARD_DEV_INFO_STRU));
        if (VOS_NULL_PTR == pstDev)
        {
            RNIC_DEV_ERR_PRINTK("RNIC_InitNetCard: net ptr is Null!");
            continue;
        }

        /* 给设备配置MAC地址,以后需要填固定值 */
        memcpy(pstDev->dev_addr, RNIC_GET_DST_MAC_ADDR(ucIndex), RNIC_MAC_ADDR_LEN);

        /* 设置默认的MTU值 */
        pstDev->mtu = RNIC_DEFAULT_MTU;

        snprintf(pstDev->name, sizeof(pstDev->name),
            "%s%s",
            RNIC_DEV_NAME_PREFIX, g_astRnicManageTbl[ucIndex].pucRnicNetCardName);

        /* 对申请到的net_device结构的设备指针的函数域进行赋值 */
        pstDev->netdev_ops = &rnic_ops;

        /* 网卡私有数据初始化 */
        pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstDev);
        pstPriv->pstDev    = pstDev;
        pstPriv->enRmNetId = ucIndex;

        netif_napi_add(pstDev, &pstPriv->stNapi, RNIC_Poll, RNIC_NAPI_POLL_DEFAULT_WEIGHT);

        /* 关闭载波 */
        netif_carrier_off(pstDev);

        /* 停止网卡接收数据 */
        netif_stop_queue(pstDev);

        /* 调用register_netdev函数对Netcard进行注册 */
        lRegRst = register_netdev(pstDev);
        if (VOS_OK != lRegRst)
        {
            /* 注册失败释放创建的RNIC设备 */
            RNIC_DEV_ERR_PRINTK("RNIC_InitNetCard:Register netdev failed!");
            free_netdev(pstDev);
            continue;
        }

        /* 保存网卡私有数据 */
        pstNetCntxt->pstPriv = pstPriv;

        /* 打开载波 */
        netif_carrier_on(pstDev);

        /* 启动网卡接收数据 */
        netif_start_queue(pstDev);
    }

    printk("RNIC_InitNetCard: eixt, %u", VOS_GetSlice());

    return 0;
}


ssize_t RNIC_ReadBindPidFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[RNIC_BIND_PROC_FILE_LEN];
    VOS_UINT32                          ulLength;

    if (*ppos > 0)
    {
        return 0;
    }

    TAF_MEM_SET_S(acModeTemp, sizeof(acModeTemp), 0x00, RNIC_BIND_PROC_FILE_LEN);

    VOS_sprintf_s((VOS_CHAR *)acModeTemp, sizeof(acModeTemp), "%d", g_ulBindPid);

    ulLength        = VOS_StrLen(acModeTemp);
    len             = TAF_MIN(len, ulLength);

    /*拷贝内核空间数据到用户空间上面*/
    if (0 == copy_to_user(buf,(VOS_VOID *)acModeTemp, (VOS_ULONG)len))
    {
        *ppos += (loff_t)len;

        return (ssize_t)len;
    }
    else
    {
        RNIC_DBG_BIND_PID_READ_CPY_ERR_NUM(1);
        return -EPERM;
    }

}


ssize_t RNIC_WriteBindPidFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[RNIC_BIND_PROC_FILE_LEN];
    VOS_UINT32                          ulStrLen;
    VOS_UINT32                          i;
    VOS_UINT32                          ulValue;

    ulValue = 0;
    TAF_MEM_SET_S(acModeTemp, sizeof(acModeTemp), 0x00, RNIC_BIND_PROC_FILE_LEN);

    if (NULL == buf)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_WriteBindPidFile: buf is NULL!");
        return -EFAULT;
    }

    if ((0 == len) || (len > RNIC_BIND_PROC_FILE_LEN))
    {
        RNIC_DBG_BIND_PID_WRITE_LEN_ERR_NUM(1);
        return -ENOSPC;
    }

    /*拷贝用户空间数据到内核空间上面*/
    if (copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, (VOS_ULONG)len) > 0)
    {
        RNIC_DBG_BIND_PID_WRITE_CPY_ERR_NUM(1);
        return -EFAULT;
    }

    acModeTemp[len-1] = '\0';

    ulStrLen = VOS_StrLen(acModeTemp);

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (acModeTemp[i] >= '0') && (acModeTemp[i] <= '9') )
        {
            ulValue = (ulValue * 10) + (acModeTemp[i] - '0');
        }
        else
        {
            RNIC_DBG_BIND_PID_WRITE_PID_ERR_NUM(1);
            return -EFAULT;
        }
    }

    g_ulBindPid  = ulValue;

     RNIC_SndNetManagerBindPidCfgInd(g_ulBindPid);

    return (ssize_t)len;
}


VOS_INT __init RNIC_InitBindProc(VOS_VOID)
{
    printk("RNIC_InitBindProc,entry,%u",VOS_GetSlice());

    if (VOS_NULL_PTR == proc_create("bind_pid", BIND_VFILE_CRT_LEVEL, VOS_NULL_PTR, &g_stBindPidOps))
    {
        RNIC_DEV_ERR_PRINTK("RNIC_InitBindProc: proc_create return ENOMEM.\n");
        return -ENOMEM;
    }

    printk("RNIC_InitBindProc,exit,%u",VOS_GetSlice());

    return 0;
}

module_init(RNIC_InitNetCard);
module_init(RNIC_InitBindProc);





