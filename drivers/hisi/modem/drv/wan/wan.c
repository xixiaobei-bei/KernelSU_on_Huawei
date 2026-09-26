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

#include <linux/platform_device.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/if_vlan.h>
#include <product_config.h>
#include <mdrv_wan.h>
#include <securec.h>
#include <mdrv_eipf.h>
#include <securec.h>
#include <bsp_maa.h>
#include <bsp_espe.h>
#include "wan.h"
#include "ipf.h"
#include "bsp_trans_report.h"
#include <bsp_slice.h>
#include "bsp_wan.h"
#include "bsp_dt.h"

#define SPE_WAN_TD_NUM (1024)
#ifdef CONFIG_ESPE_PHONE_SOC
#define SPE_WAN_RD_NUM (4096)
#else
#define SPE_WAN_RD_NUM (320)
#endif
#define SPE_WAN_RD_SKB_SIZE (1800)
#define TX_QUEUE_HIGH_WATERLEVEL (108)
#define TX_QUEUE_LOW_WATERLEVEL (64)
#define TX_MAP_FAIL_MAX (10000)
#define ERROR_VALUE (-1)
#define NAPI_WEIGHT (64)
#define UDP_PKT_LEN 1500
#define NUM_PER_SLICE (32768)
#define BIT_NUM_PER_BYTE (8)
#define WAN_SPEED_BTOMB (1024 * 1024)

struct wan_ctx_s g_wan_ctx = {0};

int wan_info_dump(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;

    bsp_err("spe_port   %u\n", wan->spe_port);
    bsp_err("water_level   %u\n", wan->water_level);

    bsp_err("rx_finish_rd_cnt   %u\n", wan->rx_finish_rd_cnt);
    bsp_err("rx_drop_cnt   %u\n", wan->rx_drop_cnt);
    bsp_err("rx_to_wan_cnt   %u\n", wan->rx_to_wan_cnt);
    bsp_err("rx_to_net_cnt   %u\n", wan->rx_to_net_cnt);
    bsp_err("rx_complete_intr_cnt   %u\n", wan->rx_complete_intr_cnt);
    bsp_err("in_rd   %u\n", wan->in_rd);
    bsp_err("tx_config_spe_fail   %u\n", wan->tx_config_spe_fail);

    bsp_err("tx_xmit_cnt   %u\n", wan->tx_xmit_cnt);
    bsp_err("tx_skb_null   %u\n", wan->tx_skb_null);
    bsp_err("tx_map_fail   %u\n", wan->tx_map_fail);
    bsp_err("tx_config_spe_succ   %u\n", wan->tx_config_spe_succ);
    bsp_err("tx_config_spe_fail   %u\n", wan->tx_config_spe_fail);

    bsp_err("rx_wan_nr_drop_stub   %u\n", wan->rx_wan_nr_drop_stub);
    bsp_err("tx_finish_td_cnt   %u\n", wan->tx_finish_td_cnt);
    bsp_err("tx_pkt_len_out_bound   %u\n", wan->tx_pkt_len_out_bound);

    return 0;
}

void wan_hds_transreport(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct wan_debug_info_s *debug_info = &wan->hds_info;

    debug_info->dbg_spe_port = wan->spe_port;
    debug_info->dbg_water_level = wan->water_level;
    debug_info->dbg_rx_finish_rd_cnt = wan->rx_finish_rd_cnt;
    debug_info->dbg_rx_drop_cnt = wan->rx_drop_cnt;
    debug_info->dbg_rx_to_wan_cnt = wan->rx_to_wan_cnt;
    debug_info->dbg_rx_to_net_cnt = wan->rx_to_net_cnt;
    debug_info->dbg_rx_complete_intr_cnt = wan->rx_complete_intr_cnt;
    debug_info->dbg_in_rd = wan->in_rd;
    debug_info->dbg_tx_config_spe_fail = wan->tx_config_spe_fail;
    debug_info->dbg_tx_xmit_cnt = wan->tx_xmit_cnt;
    debug_info->dbg_tx_skb_null = wan->tx_skb_null;
    debug_info->dbg_tx_map_fail = wan->tx_map_fail;
    debug_info->dbg_tx_config_spe_succ = wan->tx_config_spe_succ;
    debug_info->dbg_tx_config_spe_fail = wan->tx_config_spe_fail;
    debug_info->dbg_rx_wan_nr_drop_stub = wan->rx_wan_nr_drop_stub;
    debug_info->dbg_tx_finish_td_cnt = wan->tx_finish_td_cnt;
    debug_info->dbg_tx_pkt_len_out_bound = wan->tx_pkt_len_out_bound;

    return;
}

void wan_trans_report_register(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct trans_report_info *info = &wan->report_info;
    int ret;

    info->ul_msg_id = TRANS_MSGID_WAN;
    info->data = (void *)&wan->hds_info;
    info->size = sizeof(wan->hds_info);
    info->func_cb = wan_hds_transreport;
    INIT_LIST_HEAD(&info->list);

    ret = bsp_trans_report_register(info);
    if (ret) {
        bsp_err("trans report register fail\n");
    }
}

int wan_dev_dump(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct wan_dev_entry *ptr = NULL;

    list_for_each_entry(ptr, &wan->dev_list, list) {
        if (ptr != NULL) {
            bsp_err("ipv4 info  \n");
            bsp_err("dev_name   %s\n", ptr->info.dev->name);
            bsp_err("pdu_session_id   %u\n", ptr->info.v4.info.pdu_session_id);
            bsp_err("fc_head   %u\n", ptr->info.v4.info.fc_head);
            bsp_err("modem_id   %u\n", ptr->info.v4.info.modem_id);
            bsp_err("higi_pri_flag   %u\n", ptr->info.v4.info.higi_pri_flag);
            bsp_err("parse_en   %u\n", ptr->info.v4.info.parse_en);
            bsp_err("userfield0   %x\n", ptr->info.v4.userfield0);
            bsp_err("userfield1   %x\n", ptr->info.v4.userfield1);
            bsp_err("userfield2   %x\n", ptr->info.v4.userfield2);

            bsp_err("ipv6 info  \n");
            bsp_err("pdu_session_id   %u\n", ptr->info.v6.info.pdu_session_id);
            bsp_err("fc_head   %u\n", ptr->info.v6.info.fc_head);
            bsp_err("modem_id   %u\n", ptr->info.v6.info.modem_id);
            bsp_err("higi_pri_flag   %u\n", ptr->info.v6.info.higi_pri_flag);
            bsp_err("parse_en   %u\n", ptr->info.v6.info.parse_en);
            bsp_err("userfield0   %x\n", ptr->info.v6.userfield0);
            bsp_err("userfield1   %x\n", ptr->info.v6.userfield1);
            bsp_err("userfield2   %x\n", ptr->info.v6.userfield2);
        } else {
            bsp_err("wan dev NULL!\n");
        }
    }
    return 0;
}

int wan_rd_drop_en(int drop_stub_en)
{
    g_wan_ctx.rx_wan_nr_drop_stub = drop_stub_en;
    return 0;
}

void wan_speed(void)
{
    struct wan_ctx_s *wan_ctx = &g_wan_ctx;
    static unsigned int last_cpu_rd_cnt = 0;
    static unsigned int last_timestamp = 0;
    unsigned int cur_time;
    unsigned int cpu_rd_inc_cnt;
    unsigned int time_slice;

    cur_time = bsp_get_slice_value();

    if (wan_ctx->rx_to_wan_cnt >= last_cpu_rd_cnt) {
        cpu_rd_inc_cnt = wan_ctx->rx_to_wan_cnt - last_cpu_rd_cnt;
    } else {
        cpu_rd_inc_cnt = 0xFFFFFFFF + wan_ctx->rx_to_wan_cnt - last_cpu_rd_cnt;
    }

    if (cur_time > last_timestamp) {
        time_slice = cur_time - last_timestamp;
    } else {
        time_slice = 0xFFFFFFFF + cur_time - last_timestamp;
    }
    bsp_err("wan port speed during last sample:%lld Mb/s\n",
            ((long long)cpu_rd_inc_cnt * UDP_PKT_LEN * BIT_NUM_PER_BYTE * NUM_PER_SLICE) / time_slice / WAN_SPEED_BTOMB);

    last_cpu_rd_cnt = wan_ctx->rx_to_wan_cnt;

    last_timestamp = cur_time;
    return;
}


int wan_help(void)
{
    bsp_err("wan_info_dump  \n");
    bsp_err("wan_dev_dump  \n");
    bsp_err("wan_rd_drop_en  \n");
    return 0;
}

int mdrv_wan_callback_register(struct wan_callback_s *cb)
{
    if (!cb || !cb->wan_rx) {
        return -1;
    }

    if (memcpy_s(&g_wan_ctx.cb, sizeof(g_wan_ctx.cb), cb, sizeof(*cb))) {
        bsp_err("memcpy_s failed\n");
    }

    return 0;
}

int bsp_wan_stick_cb_register(wan_rx_stick_cb_t cb)
{
    if (cb == NULL) {
        return -1;
    }

    g_wan_ctx.stick_cb = cb;

    return 0;
}

int mdrv_wan_dev_info_register(struct wan_dev_info_s *info)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct wan_dev_entry *ptr = NULL;

    if (info == NULL || info->dev == NULL) {
        return -1;
    }


    list_for_each_entry(ptr, &wan->dev_list, list) {
        if (ptr->info.dev == info->dev) {
            if (memcpy_s(&ptr->info, sizeof(ptr->info), info, sizeof(*info))) {
                bsp_err("memcpy_s failed\n");
            }
            return bsp_espe_update_ipfw_entry(info);
        }
    }

    ptr = kzalloc(sizeof(struct wan_dev_entry), GFP_KERNEL);
    if (ptr == NULL) {
        return -1;
    }

    if (memcpy_s(&ptr->info, sizeof(ptr->info), info, sizeof(*info))) {
        bsp_err("memcpy_s failed\n");
    }
    list_add(&ptr->list, &wan->dev_list);

    return bsp_espe_update_ipfw_entry(info);
}

void mdrv_wan_del_direct_fw_dev(unsigned int devid)
{
    bsp_espe_del_direct_fw_dev(devid);
}

void mdrv_wan_set_ipfmac(struct ethhdr *mac_addr)
{
    if (mac_addr == NULL) {
        bsp_err("mac_addr is null\n");
        return;
    }
    bsp_espe_set_ipfmac(mac_addr);
    return;
}

struct wan_dev_info_s *find_wan_dev(struct net_device *dev)
{
    struct wan_dev_entry *entry = NULL;
    struct wan_ctx_s *wan = &g_wan_ctx;
    if (dev == NULL) {
        return NULL;
    }

    list_for_each_entry(entry, &wan->dev_list, list) {
        if (entry->info.dev == dev) {
            return &entry->info;
        }
    }
    return NULL;
}

static int wan_poll(struct napi_struct *napi, int budget)
{
    struct wan_private *priv = container_of(napi, struct wan_private, napi);

    spin_lock(&priv->poll_lock);

    napi_complete(napi);

    spin_unlock(&priv->poll_lock);

    return 0;
}

#if (defined(CONFIG_BALONG_ESPE))
int wan_spe_xmit_config_td(struct sk_buff *skb, unsigned long long maa_addr, unsigned int maa_flag)
{
    struct rx_cb_map_s *rx_cb = NULL;
    int ret;
    struct wan_ctx_s *wan = &g_wan_ctx;

    if (maa_flag & WAN_ESPE_L2ADDR_FROM_CB) {
        rx_cb = (struct rx_cb_map_s *)skb->cb;
        ret = bsp_espe_config_td(wan->spe_port, rx_cb->maa_l2, skb->len, skb, ESPE_TD_KICK_PKT);
    } else {
        ret = bsp_espe_config_td(wan->spe_port, maa_addr, skb->len, skb, ESPE_TD_KICK_PKT);
    }

    return ret;
}

static netdev_tx_t wan_spe_xmit(struct sk_buff *skb, unsigned int maa_flag)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct sk_buff* skb_unmap = NULL;
    unsigned long long maa_addr;
    unsigned int maa_addr_type;

    if (skb->len > ETH_PKT_LEN_MAX) {
        wan->tx_pkt_len_out_bound++;
    }

    if (maa_flag & WAN_MAA_OWN) {
        maa_addr = bsp_maa_get_maa(skb); // this maa addr is mac header;
        if (!maa_addr) {
            bsp_err("input skb %lx is not from maa!\n", (unsigned long)(uintptr_t)skb);
            wan->tx_map_fail++;
            if (wan->tx_map_fail > TX_MAP_FAIL_MAX) {
                BUG_ON(1);
            }
            return NETDEV_TX_OK;
        }

        maa_addr_type = bsp_maa_get_addr_type(maa_addr);
        if (unlikely(maa_addr_type != MAA_SKB_FROM_OWN)) {
            WARN_ON_ONCE(1);
        }
        bsp_trans_report_mark_pkt_info(maa_addr + skb->mac_len, trans_type_spe_ul, 0, 1);

        if (wan_spe_xmit_config_td(skb, maa_addr, maa_flag)) {
            wan->tx_config_spe_fail++;
            bsp_maa_free(maa_addr);
            return NETDEV_TX_OK;
        }
    } else {
        bsp_trans_report_mark_pkt_info((uintptr_t)skb->data + skb->mac_len, trans_type_spe_ul, 1, 0);
        maa_addr = bsp_maa_skb_map(skb, skb->mac_len);  // this maa addr is ip header;
        if (!maa_addr) {
            dev_kfree_skb_any(skb);
            wan->tx_map_fail++;
            return NETDEV_TX_OK;
        }

        if (bsp_espe_config_td(wan->spe_port, maa_addr - skb->mac_len, skb->len, skb, ESPE_TD_KICK_PKT)) {
            wan->tx_config_spe_fail++;
            skb_unmap = bsp_maa_skb_unmap(maa_addr);
            if (skb_unmap != NULL) {
                dev_kfree_skb_any(skb_unmap);
            }
            return NETDEV_TX_OK;
        }
    }

    wan->tx_config_spe_succ++;
    return NETDEV_TX_OK;
}
#endif

static netdev_tx_t wan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct wan_info_s *cb = (struct wan_info_s *)skb->cb;
    unsigned int maa_flag = 0;
    int ret;
    cb->info.fc_head = 0;
    cb->info.pdu_session_id = 1;
    cb->info.modem_id = 1;
    cb->info.higi_pri_flag = 1;
    cb->info.parse_en = 0;
    cb->userfield0 = 0;
    cb->userfield1 = 1;
    cb->userfield2 = 2;
    skb->mac_len = ETH_HLEN;
    ret = mdrv_wan_tx(skb, maa_flag);
    if (ret) {
        bsp_err("mdrv_wan_tx failed. \n");
        return NETDEV_TX_BUSY;
    }

    return NETDEV_TX_OK;
}

void wan_rx_complete(void *priv)
{
    if (g_wan_ctx.cb.rx_complete && g_wan_ctx.in_rd) {
        g_wan_ctx.cb.rx_complete();
        g_wan_ctx.rx_complete_intr_cnt++;
        g_wan_ctx.in_rd = 0;
    }
}

static const uint8_t g_wan_dst_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x04, 0x01
};
static const uint8_t g_wan_src_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x01, 0x01
};


static void set_mac_header(struct sk_buff *skb)
{
    int ret;
    struct ethhdr *eth = NULL;

    eth = (struct ethhdr *)skb->data;
    ret = memcpy_s(&eth->h_dest, ETH_ALEN, g_wan_dst_mac_addr, ETH_ALEN);
    if (ret) {
        bsp_err("memcpy_s failed\n");
    }
    ret = memcpy_s(&eth->h_source, ETH_ALEN, g_wan_src_mac_addr, ETH_ALEN);
    if (ret) {
        bsp_err("memcpy_s failed\n");
    }

    eth->h_proto = (__be16)ETH_P_PPP_MP;

    return;
}

void wan_set_cpu0_stub_en(unsigned int en)
{
    g_wan_ctx.cpu0_stub_en = en;
}

void wan_finish_rd(struct sk_buff *skb, unsigned int len, void *param, unsigned int flags)
{
    struct wan_ctx_s *wan_ctx = &g_wan_ctx;
    struct rx_cb_map_s *rx_cb = NULL;
    enum wan_rx_handle_rslt ret;

    wan_ctx->rx_finish_rd_cnt++;
    if (skb == NULL) {
        return;
    }

    if (unlikely(wan_ctx->rx_wan_nr_drop_stub)) {
        wan_ctx->rx_drop_cnt++;
        rx_cb = (struct rx_cb_map_s *)skb->cb;
        dev_kfree_skb_any(skb);
        return;
    }

    if (wan_ctx->cb.wan_rx && g_wan_ctx.cpu0_stub_en == 0) {
        skb_pull(skb, ETH_HLEN);
        ret = wan_ctx->cb.wan_rx(skb);
        if (ret == WAN_RX_PASS) {
            if (wan_ctx->stick_cb) {
                wan_ctx->stick_cb(skb);
            } else {
                dev_kfree_skb_any(skb);
            }
            wan_ctx->rx_stick_cnt++;
        }
        wan_ctx->rx_to_wan_cnt++;
        wan_ctx->in_rd = 1;
    } else {
        wan_ctx->rx_to_net_cnt++;
        set_mac_header(skb);
        skb->protocol = eth_type_trans(skb, wan_ctx->ndev);
        netif_rx(skb);
    }

    if (flags & ESPE_RD_INTR_DONE) {
        wan_ctx->rx_complete_intr_cnt++;

        wan_rx_complete(NULL);
    }

    return;
}

static const struct net_device_ops g_wan_ops = {
    .ndo_start_xmit = wan_start_xmit,
};

int mdrv_wan_tx(struct sk_buff *skb, unsigned int maa_flag)
{
    struct wan_ctx_s* wan = &g_wan_ctx;
    if (unlikely(skb == NULL)) {
        wan->tx_skb_null++;
        return NETDEV_TX_OK;
    }
    wan->tx_xmit_cnt++;
#if (defined(CONFIG_BALONG_ESPE))
    return wan_spe_xmit(skb, maa_flag);
#else
    consume_skb(skb);
    return 0;
#endif
}

static int wan_alloc_espe_port(struct wan_ctx_s *wan)
{
#if (defined(CONFIG_BALONG_ESPE))
    struct espe_port_comm_attr attr = {0};
    int ret;

    attr.rd_depth = SPE_WAN_RD_NUM;
    attr.td_depth = SPE_WAN_TD_NUM;
    attr.net = wan->ndev;
    attr.priv = (void *)wan->ndev;
    attr.enc_type = SPE_ENC_CPU;

    attr.ops.espe_finish_rd = wan_finish_rd;
    attr.ops.espe_complete_rd = wan_rx_complete;
    attr.ops.get_wan_info = find_wan_dev;

    wan->spe_port = bsp_espe_alloc_port(&attr);
    if (wan->spe_port < 0) {
        bsp_err("invalid wan->spe_port: %u\n", wan->spe_port);
        return ERROR_VALUE;
    }

    ret = bsp_espe_enable_port(wan->spe_port);
    if (ret != 0) {
        bsp_err("[probe]bsp_espe_enable_port fail\n");
        return ERROR_VALUE;
    }
#endif
    return 0;
}

static int wan_probe(struct platform_device *dev)
{
    int ret = 0;
    struct wan_ctx_s *wan = &g_wan_ctx;

    wan->ndev = alloc_etherdev(sizeof(struct wan_private));
    if (wan->ndev == NULL) {
        ret = ERROR_VALUE;
        bsp_err("alloc_etherdev faild!\n");
        goto WAN_PRB_ERR0;
    }

    dev_alloc_name(wan->ndev, "CPU%d");
    dev_hold(wan->ndev);
    wan->ndev->netdev_ops = &g_wan_ops;
    dev_put(wan->ndev);
    wan->priv = (struct wan_private *)netdev_priv(wan->ndev);
    netif_napi_add(wan->ndev, &wan->priv->napi, wan_poll, NAPI_WEIGHT);

    if (register_netdev(wan->ndev)) {
        ret = ERROR_VALUE;
        bsp_err("[probe]register_netdev faild!\n");
        goto WAN_PRB_ERR1;
    }

    ret = memcpy_s(wan->ndev->dev_addr, ETH_ALEN, g_wan_dst_mac_addr, ETH_ALEN);
    if (ret) {
        bsp_err("memcpy_s failed\n");
    }

    ret = wan_alloc_espe_port(wan);
    if (ret != 0) {
        goto WAN_PRB_ERR1;
    }
    INIT_LIST_HEAD(&wan->dev_list);

    ret = lan_init();
    if (ret != 0) {
        bsp_err("[probe]lan init fail\n");
    }

    wan->cpu0_stub_en = 0;
    wan_trans_report_register();
    bsp_err("[probe]wan_probe done\n");
    return 0;

WAN_PRB_ERR1:
    free_netdev(wan->ndev);
WAN_PRB_ERR0:
    return ret;
}
struct platform_device g_wan_device = {
    .name    = "wan",
    .id    = -1,
    .num_resources   = 0,
};

static const struct of_device_id g_wan_match[] = {
    { .compatible = "hisilicon,wan" },
    {},
};

static struct platform_driver g_wan_pltfm_driver = {
    .probe  = wan_probe,
    .driver = {
        .name   = "wan",
        .of_match_table = g_wan_match,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

EXPORT_SYMBOL(mdrv_wan_callback_register);
EXPORT_SYMBOL(wan_finish_rd);
EXPORT_SYMBOL(mdrv_wan_dev_info_register);
EXPORT_SYMBOL(g_wan_ctx);
EXPORT_SYMBOL(mdrv_wan_tx);

int wan_pltfm_driver_init(void)
{
    int ret;

    device_node_s *np = bsp_dt_find_compatible_node(NULL, NULL, "hisilicon,wan");
    if (np == NULL) {
        return -1;
    }

    if (bsp_dt_device_is_available(np) == false) {
        ret = platform_device_register(&g_wan_device);
        if (ret) {
            bsp_err("dev register fail.\n");
            return -1;
        }
    }

    ret = platform_driver_register(&g_wan_pltfm_driver);
    if (ret) {
        bsp_err("drv register fail.\n");
        return -1;
    }
    return 0;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(wan_pltfm_driver_init);
#endif
