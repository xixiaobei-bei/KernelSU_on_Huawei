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

#include <linux/sched.h>
#include <uapi/linux/ip.h>
#include <uapi/linux/in.h>
#include "bsp_net_helper.h"

#define DEFAULT_PROTECT_TIME    1

void bsp_ip_limit_init(struct ip_limit_cfg* cfg, unsigned int limit)
{
    cfg->local_jiffies = jiffies;
    cfg->limit = limit;
    cfg->rx_cnt = 0;
    cfg->period = DEFAULT_PROTECT_TIME;
    cfg->drop_cnt = 0;
    cfg->in_cnt = 0;
    cfg->clear_cnt = 0;
    cfg->check_cnt = 0;
    cfg->helper_ip_cnt = 0;
    cfg->helper_udp_cnt = 0;
    cfg->helper_icmp_cnt = 0;
    cfg->helper_not_ip = 0;
    cfg->helper_not_care = 0;
    cfg->helper_not_ipv4 = 0;
}

volatile unsigned int g_hlp_pkt_print = 0;
void hlp_pr_set(unsigned int on)
{
    g_hlp_pkt_print = on;
}

int bsp_ip_limit(struct ip_limit_cfg* cfg, struct iphdr *ip)
{
    cfg->in_cnt++;

    if (g_hlp_pkt_print) {
        printk("ip->protocol:%x  %x", ip->protocol, IPPROTO_UDP);
        printk("\n");
        g_hlp_pkt_print = 0;
    }

    if (ip->version != IPVERSION) {
        cfg->helper_not_ipv4++;
        return 0;
    }

    if (ip->protocol == IPPROTO_UDP) {
        cfg->helper_udp_cnt++;
    } else if (ip->protocol == IPPROTO_ICMP) {
        cfg->helper_icmp_cnt++;
    } else {
        cfg->helper_not_care++;
    }

    /* calc time is over */
    if (time_after_eq(jiffies, cfg->local_jiffies + cfg->period)) {
        cfg->local_jiffies = jiffies;
        cfg->rx_cnt = 0;
        cfg->helper_udp_cnt = 0;
        cfg->helper_icmp_cnt = 0;
        cfg->clear_cnt++;
    }

    if (cfg->helper_udp_cnt > cfg->limit)
        return 1;
    if (cfg->helper_icmp_cnt > cfg->limit)
        return 1;

    return 0;
}

int bsp_eth_limit(struct ip_limit_cfg* cfg, unsigned char* data)
{
    struct iphdr *ip = NULL;
    struct ethhdr *ethhd = NULL;
    unsigned int i;

    cfg->in_cnt++;

    ip = (struct iphdr *)(data + ETH_HLEN);
    ethhd = (struct ethhdr *)data;

    if (g_hlp_pkt_print) {
        for (i = 0; i < sizeof(struct iphdr) + sizeof(struct ethhdr); i++) {
            printk("%x\t", data[i]);
        }
        printk("ethhd->proto:%x  %x", ethhd->h_proto, htons(ETH_P_IP));
        printk("ip->protocol:%x  %x", ip->protocol, IPPROTO_UDP);
        printk("\n");
        g_hlp_pkt_print = 0;
    }

    if (htons(ETH_P_IP) != ethhd->h_proto) {
        cfg->helper_not_ip++;
        return 0;
    }
    cfg->helper_ip_cnt++;

    return bsp_ip_limit(cfg, ip);
}

void bsp_ip_limit_show(struct ip_limit_cfg* cfg)
{
    printk("local_jiffies: %lx\n", cfg->local_jiffies);
    printk("limit: %x\n", cfg->limit);
    printk("rx_cnt: %x\n", cfg->rx_cnt);
    printk("period: %x\n", cfg->period);
    printk("drop_cnt: %x\n", cfg->drop_cnt);
    printk("in_cnt: %x\n", cfg->in_cnt);
    printk("clear_cnt: %x\n", cfg->clear_cnt);
    printk("check_cnt: %x\n", cfg->check_cnt);
    printk("helper_ip_cnt: %x\n", cfg->helper_ip_cnt);
    printk("helper_udp_cnt: %x\n", cfg->helper_udp_cnt);
    printk("helper_icmp_cnt: %x\n", cfg->helper_icmp_cnt);
    printk("helper_not_care:%x\n", cfg->helper_not_care);
    printk("helper_not_ipv4:%x\n", cfg->helper_not_ipv4);
}
