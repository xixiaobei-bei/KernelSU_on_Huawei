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

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <securec.h>
#include "../espe.h"
#include "../espe_core.h"
#include "../espe_dbg.h"
#include "../espe_ip_entry.h"
#include "../espe_mac_entry.h"
#include "../espe_port.h"
#include "../hal/espe_hal_reg.h"
#include "../hal/espe_hal_desc.h"

#include "espe_sysfs.h"

#define SPE_STR_LEN 20

struct spe_cmd_handler_s g_spe_cmd;

struct spe_filter_param g_mac_filter_array[] = {
    { "mac", filter_mac_addr },
    { "action", filter_action },
    { "reaction", filter_mac_reaction },
    { "redirect", filter_mac_redirect },
};

struct spe_filter_param g_mac_forward_array[] = {
    { "action", forward_action },
};

struct spe_filter_param g_ip_filter_array[] = {
    { "action", filter_action },
    { "filter_mask", filter_ip_mask },
    { "src_port_low", filter_src_port_low },
    { "src_port_high", filter_src_port_high },
    { "dst_port_low", filter_dest_port_low },
    { "dst_port_high", filter_dest_port_high },

    { "src_addr", filter_ip_src_addr },
    { "src_addr_mask", filter_ip_src_addr_mask },
    { "dst_addr", filter_ip_dest_addr },
    { "dst_addr_mask", filter_ip_dest_addr_mask },
    { "protocol", filter_ipv4_protocol },
    { "tos", filter_tos },
};

struct spe_filter_param g_ipv6_filter_array[] = {
    { "action", filter_action },
    { "filter_mask", filter_ip_mask },
    { "src_port_low", filter_src_port_low },
    { "src_port_high", filter_src_port_high },
    { "dst_port_low", filter_dest_port_low },
    { "dst_port_high", filter_dest_port_high },

    { "src_addr", filter_ipv6_src_addr },
    { "src_addr_mask", filter_ipv6_src_addr_mask },
    { "dst_addr", filter_ipv6_dest_addr },
    { "dst_addr_mask", filter_ipv6_dest_addr_mask },
    { "protocol", filter_ipv6_protocol },
    { "tc", filter_ipv6_tc },
    { "next_hdr", filter_ipv6_next_hdr },
    { "flow_label", filter_ipv6_flow_label },
};

struct spe_filter_param g_spe_black_white[] = {
    { "mode", spe_black_white_mode_set },
    { "black_white", spe_black_white_set },
    { "action", set_action },
};

struct spe_filter_param g_spe_set_eth_minlen_array[] = {
    { "value", spe_eth_min_len_value_set },
    { "action", set_action },
};

struct spe_filter_param g_udp_lmtnum_array[] = {
    { "portno", spe_udp_lim_portno_set },
    { "udp_rate", udp_rate_set },
    { "action", set_action },
};

struct spe_filter_param g_lmt_byte_array[] = {
    { "portno", spe_udp_lim_portno_set },
    { "byte_rate", port_rate_set },
    { "action", set_action },
};

struct spe_filter_param g_lmt_time_array[] = {
    { "time", spe_port_lim_time_set },
    { "action", set_action },
};

struct spe_filter_param g_spe_dynamic_switch[] = {
    { "switch", spe_switch },
    { "action", set_action },
};

struct spe_filter_param g_spe_l4_portnum_array[] = {
    { "protocol", spe_l4_protocol_get },
    { "portnum", spe_l4_portnum_get },
    { "action", filter_action },
};

#ifdef HISI_SPE_DEBUG
int g_last_filter_type = 0;
#endif

// function declaration
int espe_input(const char *buf, const char *fmt, ...)
{
    int ret;
    va_list arguments;

    va_start(arguments, fmt);
    ret = vsscanf_s(buf, fmt, arguments);
    va_end(arguments);

    if (ret < 0) {
        SPE_ERR("vsscanf_s err.\n");
    }

    return ret;
}

void get_black_white(unsigned int *mac_b_w, unsigned int *ip_b_w)
{
    struct spe *spe = &g_espe;
    spe_black_white_t black_white_reg;
    black_white_reg.u32 = spe_readl(spe->regs, SPE_BLACK_WHITE);
    *mac_b_w = black_white_reg.bits.spe_mac_black_white;
    *ip_b_w = black_white_reg.bits.spe_ip_black_white;
}

/* black_write = 0 : write ; black_write = 1 : black */
void set_black_white(enum spe_tab_list_type list_type, unsigned int black_white)
{
    struct spe *spe = &g_espe;
    spe_black_white_t black_white_reg;
    black_white_reg.u32 = spe_readl(spe->regs, SPE_BLACK_WHITE);

    switch (list_type) {
        case SPE_TAB_LIST_MAC_FILT:
            black_white_reg.bits.spe_mac_black_white = !!black_white;
            spe_writel(spe->regs, SPE_BLACK_WHITE, black_white_reg.u32);
            break;

        case SPE_TAB_LIST_IPV4_FILT:
        case SPE_TAB_LIST_IPV6_FILT:
            black_white_reg.bits.spe_ip_black_white = !!black_white;
            spe_writel(spe->regs, SPE_BLACK_WHITE, black_white_reg.u32);
            break;

        default:
            SPE_ERR("invalid list_type(%d)\n", list_type);
            break;
    }
}

void black_white_mac(unsigned int mac)
{
    struct spe *spe = &g_espe;
    spe_black_white_t list;

    list.u32 = spe_readl(spe->regs, SPE_BLACK_WHITE);
    list.bits.spe_mac_black_white = mac;
    spe_writel(spe->regs, SPE_BLACK_WHITE, list.u32);
}

void black_white_ip(unsigned int ip)
{
    spe_black_white_t list;
    struct spe *spe = &g_espe;

    list.u32 = spe_readl(spe->regs, SPE_BLACK_WHITE);
    list.bits.spe_ip_black_white = ip;
    spe_writel(spe->regs, SPE_BLACK_WHITE, list.u32);
}
int espe_mac_filt_act(void *mac_cmd_handler)
{
    struct spe_cmd_handler_s *cmd_handler = (struct spe_cmd_handler_s *)mac_cmd_handler;
    struct espe_mac_filt_entry mac_filter = {0};
    int ret;

    ret = memcpy_s(&mac_filter.mac_hi, sizeof(mac_filter.mac_hi), cmd_handler->u.mac_content.mac, sizeof(unsigned int));
    if (unlikely(ret)) {
        SPE_ERR("memcpy_s fail.\n");
    }
    ret = memcpy_s(&mac_filter.mac_lo, sizeof(mac_filter.mac_lo), cmd_handler->u.mac_content.mac + sizeof(unsigned int),
                   sizeof(unsigned short));
    if (unlikely(ret)) {
        SPE_ERR("memcpy_s fail.\n");
    }
    mac_filter.redir_act = cmd_handler->u.mac_content.action;
    mac_filter.redir_port = cmd_handler->u.mac_content.redirect;

    if (cmd_handler->action == SPE_ENTRY_DELETE) {
        ret = espe_mac_filt_del(&mac_filter);
        if (ret) {
            SPE_ERR("espe_mac_filt_del err.\n");
        }
    } else if (cmd_handler->action == SPE_ENTRY_ADD) {
        ret = espe_mac_filt_add(&mac_filter);
        if (ret) {
            SPE_ERR("espe_mac_filt_add err.\n");
        }
    }

    return ret;
}

int spe_ipv4_filt_act(void *ipv4_cmd_handler)
{
    struct spe_cmd_handler_s *cmd_handler = (struct spe_cmd_handler_s *)ipv4_cmd_handler;
    struct espe_ipv4_filt_entry ipv4_filter;
    int ret;

    if (cmd_handler->u.ip_content.dst_port_low > cmd_handler->u.ip_content.dst_port_high) {
        SPE_ERR("invaild ip port number. \n");
        return SPE_FAIL;
    }

    if (cmd_handler->u.ip_content.src_port_low > cmd_handler->u.ip_content.src_port_high) {
        SPE_ERR("invaild ip port number. \n");
        return SPE_FAIL;
    }

    ret = memset_s(&ipv4_filter, sizeof(ipv4_filter), 0, sizeof(struct espe_ipv4_filt_entry));
    if (ret) {
        SPE_ERR("memset_s err.\n");
    }

    ipv4_filter.sip = cmd_handler->u.ip_content.u.spe_ipv4.src_addr;
    ipv4_filter.sip = cpu_to_be32(ipv4_filter.sip);
    ipv4_filter.sip_mask = cmd_handler->u.ip_content.u.spe_ipv4.src_addr_mask;
    ipv4_filter.dip = cmd_handler->u.ip_content.u.spe_ipv4.dst_addr;
    ipv4_filter.dip = cpu_to_be32(ipv4_filter.dip);
    ipv4_filter.dip_mask = cmd_handler->u.ip_content.u.spe_ipv4.dst_addr_mask;
    ipv4_filter.tos = cmd_handler->u.ip_content.u.spe_ipv4.tos;
    ipv4_filter.protocol = cmd_handler->u.ip_content.u.spe_ipv4.protocol;

    ipv4_filter.filt_mask = cmd_handler->u.ip_content.filter_mask;
    ipv4_filter.dport_max = cpu_to_be16(cmd_handler->u.ip_content.dst_port_high);
    ipv4_filter.dport_min = cpu_to_be16(cmd_handler->u.ip_content.dst_port_low);
    ipv4_filter.sport_max = cpu_to_be16(cmd_handler->u.ip_content.src_port_high);
    ipv4_filter.sport_min = cpu_to_be16(cmd_handler->u.ip_content.src_port_low);

    if (cmd_handler->action == SPE_ENTRY_DELETE) {
        ret = espe_mac_entry_config(SPE_TAB_CTRL_DEL_IPV4_FILT, (unsigned int *)(&ipv4_filter),
                                    sizeof(struct espe_ipv4_filt_entry) / sizeof(unsigned int));
        if (ret) {
            SPE_ERR("del ipv4 filter enty fail,please check entry exit.");
        } else {
            SPE_ERR("del ipv4 filter enty sucess.");
        }
    } else if (cmd_handler->action == SPE_ENTRY_ADD) {
        ret = espe_mac_entry_config(SPE_TAB_CTRL_ADD_IPV4_FILT, (unsigned int *)(&ipv4_filter),
                                    sizeof(struct espe_ipv4_filt_entry) / sizeof(unsigned int));
        if (ret) {
            SPE_ERR("add ipv4 filter enty fail.");
        } else {
            SPE_ERR("add ipv4 filter enty sucess.");
        }
    }

    return ret;
}

int spe_ipv6_filt_act(void *ipv6_cmd_handler)
{
    struct spe_cmd_handler_s *cmd_handler = (struct spe_cmd_handler_s *)ipv6_cmd_handler;
    struct espe_ipv6_filt_entry ipv6_filter;
    int ret;

    if (cmd_handler->u.ip_content.dst_port_low > cmd_handler->u.ip_content.dst_port_high) {
        SPE_ERR("invaild ip port number. \n");
        return SPE_FAIL;
    }

    if (cmd_handler->u.ip_content.src_port_low > cmd_handler->u.ip_content.src_port_high) {
        SPE_ERR("invaild ip port number. \n");
        return SPE_FAIL;
    }

    ret = memset_s(&ipv6_filter, sizeof(ipv6_filter), 0, sizeof(struct espe_ipv6_filt_entry));
    if (ret) {
        SPE_ERR("memset_s err.\n");
    }
    ipv6_filter.sip[0] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.src_addr[0]);
    ipv6_filter.sip[1] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.src_addr[1]);
    ipv6_filter.sip[2] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.src_addr[2]);
    ipv6_filter.sip[3] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.src_addr[3]);
    ipv6_filter.sip_mask = cmd_handler->u.ip_content.u.spe_ipv6.src_addr_mask;

    ipv6_filter.dip[0] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.dst_addr[0]);
    ipv6_filter.dip[1] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.dst_addr[1]);
    ipv6_filter.dip[2] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.dst_addr[2]);
    ipv6_filter.dip[3] = cpu_to_be32(cmd_handler->u.ip_content.u.spe_ipv6.dst_addr[3]);
    ipv6_filter.dip_mask = cmd_handler->u.ip_content.u.spe_ipv6.dst_addr_mask;

    ipv6_filter.tc = cmd_handler->u.ip_content.u.spe_ipv6.tc;
    ipv6_filter.next_hdr = cmd_handler->u.ip_content.u.spe_ipv6.next_hdr;
    ipv6_filter.filt_mask = cmd_handler->u.ip_content.filter_mask;
    ipv6_filter.dport_max = cpu_to_be16(cmd_handler->u.ip_content.dst_port_high);
    ipv6_filter.dport_min = cpu_to_be16(cmd_handler->u.ip_content.dst_port_low);
    ipv6_filter.sport_max = cpu_to_be16(cmd_handler->u.ip_content.src_port_high);
    ipv6_filter.sport_min = cpu_to_be16(cmd_handler->u.ip_content.src_port_low);

    if (cmd_handler->action == SPE_ENTRY_DELETE) {
        ipv6_filter.flow_label = cmd_handler->u.ip_content.u.spe_ipv6.flow_label;
        ret = espe_mac_entry_config(SPE_TAB_CTRL_DEL_IPV6_FILT, (unsigned int *)(&ipv6_filter),
                                    sizeof(struct espe_ipv6_filt_entry) / sizeof(unsigned int));
    } else if (cmd_handler->action == SPE_ENTRY_ADD) {
        ret = espe_mac_entry_config(SPE_TAB_CTRL_ADD_IPV6_FILT, (unsigned int *)(&ipv6_filter),
                                    sizeof(struct espe_ipv6_filt_entry) / sizeof(unsigned int));
    }

    return ret;
}

void ipv4_filt_dump(unsigned int *spe_tab, int i)
{
    struct espe_ipv4_filter_entry_in_sram *ipv4_filter_entry = NULL;

    if (spe_tab[1] & IS_IP_FL_ENTRY) {
        ipv4_filter_entry = (struct espe_ipv4_filter_entry_in_sram *)spe_tab;
    } else {
        return;
    }
    SPE_ERR("[ipv4 entry (%d)] \nsip_mask:%d  dip_mask:%d  protocol:%d  tos:%d  filter_mask:0x%x  "
            " sport_min:%d  sport_max:%d  dport_min:%d  dport_max:%d\n",
            i, ipv4_filter_entry->sip_mask & 0xff, ipv4_filter_entry->dip_mask & 0xff,
            ipv4_filter_entry->protocol & 0xff, ipv4_filter_entry->tos & 0xff, ipv4_filter_entry->filter_mask & 0xff,
            be16_to_cpu(ipv4_filter_entry->sport_min & 0xffff), be16_to_cpu(ipv4_filter_entry->sport_max & 0xffff),
            be16_to_cpu(ipv4_filter_entry->dport_min & 0xffff), be16_to_cpu(ipv4_filter_entry->dport_max & 0xffff));
}

void ipv6_filt_dump(unsigned int *spe_tab, int i)
{
    struct espe_ipv6_filter_entry_in_sram *ipv6_filter_entry = NULL;

    if (spe_tab[1] & IS_IP_FL_ENTRY) {
        ipv6_filter_entry = (struct espe_ipv6_filter_entry_in_sram *)spe_tab;
    } else {
        return;
    }
    SPE_ERR("[ipv6 entry (%d)] \nsip_mask:%d  dip_mask:%d  next_hdr:0x%x  tc:%d  filter_mask:0x%x  "
            "flow_label:0x%x sport_min:%d  sport_max:%d  dport_min:%d  dport_max:%d\n",
            i, ipv6_filter_entry->sip_mask & 0xff, ipv6_filter_entry->dip_mask & 0xff,
            ipv6_filter_entry->next_hdr & 0xff, ipv6_filter_entry->tc & 0xff, ipv6_filter_entry->filter_mask & 0xff,
            ipv6_filter_entry->flow_label & 0xff, be16_to_cpu(ipv6_filter_entry->sport_min & 0xffff),
            be16_to_cpu(ipv6_filter_entry->sport_max & 0xffff), be16_to_cpu(ipv6_filter_entry->dport_min & 0xffff),
            be16_to_cpu(ipv6_filter_entry->dport_max & 0xffff));
}

static  inline void espe_l4_portnum_dump(struct spe *spe)
{
    unsigned int i;
    unsigned short portnum = 0;
    spe_l4_cfg_t l4_portnum_cfg;

    for (i = 0; i < MAX_L4_PORTNUM_CFG_NUM; i++) {
        l4_portnum_cfg.u32 = spe_readl(spe->regs, SPE_L4_PORTNUM_CFG(i));
        if (l4_portnum_cfg.u32) {
            if (l4_portnum_cfg.bits.spe_l4_portnum_cfg & BIT(16)) {
                portnum = be16_to_cpu((unsigned short)(l4_portnum_cfg.bits.spe_l4_portnum_cfg & 0xFFFF));
                SPE_ERR("TCP l4_portnum : %d \n", portnum);
            } else if (l4_portnum_cfg.bits.spe_l4_portnum_cfg & BIT(17)) {
                portnum = be16_to_cpu((unsigned short)(l4_portnum_cfg.bits.spe_l4_portnum_cfg & 0xFFFF));
                SPE_ERR("UDP l4_portnum : %d \n", portnum);
            } else {
                l4_portnum_cfg.bits.spe_l4_portnum_cfg = 0;
                spe_writel(spe->regs, SPE_L4_PORTNUM_CFG(i), l4_portnum_cfg.u32);
            }
        }
    }
    return;
}

static inline void espe_l4_portnum_del(struct spe *spe, int cfg)
{
    unsigned int i;
    spe_l4_cfg_t l4_portnum_cfg;

     for (i = 0; i < MAX_L4_PORTNUM_CFG_NUM; i++) {
        l4_portnum_cfg.u32 = spe_readl(spe->regs, SPE_L4_PORTNUM_CFG(i));
        if (l4_portnum_cfg.bits.spe_l4_portnum_cfg == cfg) {
            l4_portnum_cfg.bits.spe_l4_portnum_cfg = 0;
            spe_writel(spe->regs, SPE_L4_PORTNUM_CFG(i), l4_portnum_cfg.u32);
            break;
        }
     }
    return;
}

static inline void espe_l4_portnum_add(struct spe *spe, int cfg)
{
    unsigned int i;
    spe_l4_cfg_t l4_portnum_cfg;

    for (i = 0; i < MAX_L4_PORTNUM_CFG_NUM; i++) {
        l4_portnum_cfg.u32 = spe_readl(spe->regs, SPE_L4_PORTNUM_CFG(i));
        if (!l4_portnum_cfg.u32) {
            l4_portnum_cfg.bits.spe_l4_portnum_cfg = cfg;
            spe_writel(spe->regs, SPE_L4_PORTNUM_CFG(i), l4_portnum_cfg.u32);
            break;
        }
    }
    return;
}

int espe_l4_portnum_cfg(int cfg, unsigned int add_del_dump)
{
    struct spe *spe = &g_espe;

    switch (add_del_dump) {
        case ADD: /* add */
            espe_l4_portnum_add(spe, cfg);
            break;
        case DEL: /* del */
            espe_l4_portnum_del(spe, cfg);
            break;
        case DUMP: /* dump */
            espe_l4_portnum_dump(spe);
            break;
        default:
            return -ENOMEM;
    }

    return 0;
}

int spe_port_l4portno_to_cpu_del(unsigned short portnum, int l4_type)
{
    unsigned int cfg = 0;

    switch (l4_type) {
        case IPPROTO_UDP:
            cfg = cpu_to_be16(portnum) | BIT(17);
            break;
        case IPPROTO_TCP:
            cfg = cpu_to_be16(portnum) | BIT(16);
            break;
        default:
            SPE_ERR("L4 type %d is not supported! \n", l4_type);
            return -EPERM;
    }

    if (!espe_l4_portnum_cfg(cfg, DEL)) {
        return 0;
    }

    return -ENOMEM;
}

int spe_port_l4portno_to_cpu_add(unsigned short portnum, int l4_type)
{
    unsigned int cfg = 0;
    int ret;

    switch (l4_type) {
        case IPPROTO_UDP:
            cfg = cpu_to_be16(portnum) | BIT(17);
            break;
        case IPPROTO_TCP:
            cfg = cpu_to_be16(portnum) | BIT(16);
            break;
        default:
            SPE_ERR("L4 type %d is not supported! \n", l4_type);
            return -EPERM;
    }

    ret = spe_port_l4portno_to_cpu_del(portnum, l4_type);
    if (ret) {
        SPE_ERR("spe_port_l4portno_to_cpu_del fail. \n");
    }

    if (!espe_l4_portnum_cfg(cfg, ADD)) {
        return 0;
    }

    SPE_ERR("l4_portnum_cfg already full! \n");
    return -ENOMEM;
}

void spe_port_l4portno_to_cpu_dump(void)
{
    int ret;

    ret = espe_l4_portnum_cfg(0, DUMP);
    if (ret) {
        SPE_ERR("espe_l4_portnum_cfg fail. \n");
    }
    return;
}

/*
 * function name:  spe_mac_validate
 * description: do some basic mac check
 * mac format  01:02:03:04:05:06
 */
int mac_entry_set(struct mac_forward_entry *entry, int action)
{
    struct mac_forward_handler_s mac_forward_handler;
    int ret;
    ret = memset_s(&mac_forward_handler, sizeof(mac_forward_handler), 0, sizeof(struct mac_forward_handler_s));
    if (ret) {
        SPE_ERR("mac_entry_set memset fail \n");
        return -ENOMEM;
    }
    ret = memcpy_s(&mac_forward_handler.mac_forward, sizeof(mac_forward_handler.mac_forward), entry,
                   sizeof(struct mac_forward_entry));
    if (ret) {
        SPE_ERR("mac_entry_set memcpy fail \n");
        return -ENOMEM;
    }

    mac_forward_handler.action = action;
    mac_forward_handler.type = SPE_MAC_FORWARD_TABLE;

    return exe_spe_cmd();
}

/*
 * function name:  spe_mac_validate
 * description: do some basic mac check
 * mac format  01:02:03:04:05:06
 */
int spe_mac_validate(void)
{
    return SPE_OK;
}

int spe_sysfs_cmd_type(const char *buf, unsigned int buf_len)
{
    int ret;

    ret = memset_s(&g_spe_cmd, sizeof(g_spe_cmd), 0, sizeof(struct spe_cmd_handler_s));
    if (ret) {
        SPE_ERR("spe_sysfs_cmd_type fail \n");
    }

    if (0 == strcmp(buf, "ipv4")) {
        g_spe_cmd.type = SPE_IP_FILTER_TABLE;
        g_spe_cmd.filter_parser_ops = g_ip_filter_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_ip_filter_array);
        g_spe_cmd.u.ip_content.iptype = 0;
        SPE_ERR("ipv4 filter:\n");
    } else if (0 == strcmp(buf, "ipv6")) {
        g_spe_cmd.type = SPE_IPV6_FILTER_TABLE;
        g_spe_cmd.filter_parser_ops = g_ipv6_filter_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_ipv6_filter_array);
        g_spe_cmd.u.ip_content.iptype = 1;
        SPE_ERR("ipv6 filter:\n");
    } else if (0 == strcmp(buf, "mac_filter")) {
        g_spe_cmd.type = SPE_MAC_FILTER_TABLE;
        g_spe_cmd.filter_parser_ops = g_mac_filter_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_mac_filter_array);
        SPE_ERR("mac filter:\n");
    } else if (0 == strcmp(buf, "mac_forward")) {
        g_spe_cmd.type = SPE_MAC_FORWARD_TABLE;
        g_spe_cmd.filter_parser_ops = g_mac_forward_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_mac_forward_array);
        SPE_ERR("mac forward:\n");
    } else if (0 == strcmp(buf, "filter_type")) {
        g_spe_cmd.type = SPE_BLACK_WHITE_LIST;
        g_spe_cmd.filter_parser_ops = g_spe_black_white;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_spe_black_white);
        SPE_ERR("filter type:\n");
    } else if (0 == strcmp(buf, "udp_limit")) {
        g_spe_cmd.type = SPE_UDP_LMTNUM_TABLE;
        g_spe_cmd.filter_parser_ops = g_udp_lmtnum_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_udp_lmtnum_array);
        SPE_ERR("udp limit:\n");
    } else if (0 == strcmp(buf, "byte_limit")) {
        g_spe_cmd.type = SPE_LMTBYTE_TABLE;
        g_spe_cmd.filter_parser_ops = g_lmt_byte_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_lmt_byte_array);
        SPE_ERR("byte limit:\n");
    } else if (0 == strcmp(buf, "limit_time")) {
        g_spe_cmd.type = SPE_LMT_TIME_TABLE;
        g_spe_cmd.filter_parser_ops = g_lmt_time_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_lmt_time_array);
        SPE_ERR("limit time:\n");
    } else if (0 == strcmp(buf, "min_len")) {
        g_spe_cmd.type = SPE_ETH_MIN_LEN;
        g_spe_cmd.filter_parser_ops = g_spe_set_eth_minlen_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_spe_set_eth_minlen_array);
        SPE_ERR("min len:\n");
    } else if (0 == strcmp(buf, "switch")) {
        g_spe_cmd.type = SPE_DYNAMIC_SWITCH;
        g_spe_cmd.filter_parser_ops = g_spe_dynamic_switch;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_spe_dynamic_switch);
        SPE_ERR("dynamic switch mode:\n");
    } else if (0 == strcmp(buf, "l4_portnum")) {
        g_spe_cmd.type = SPE_L4_PORTNUM;
        g_spe_cmd.filter_parser_ops = g_spe_l4_portnum_array;
        g_spe_cmd.num_funcs = ARRAY_SIZE(g_spe_l4_portnum_array);
        SPE_ERR("l4 port number:\n");
    } else if (0 == strcmp(buf, "del_all_entrys")) {
        g_spe_cmd.type = SPE_DEL_ALL_ENTRYS;
        SPE_ERR("del_all_entrys:\n");
    } else {
        SPE_ERR("error parameter : %s\n", buf);
        return SPE_FAIL;
    }

    return SPE_OK;
}

int filter_mac_addr(const char *buf)
{
    int ret;
    int mac1[ETH_ALEN];
    char mac[ETH_ALEN];

    ret = espe_input(buf, "%2x:%2x:%2x:%2x:%2x:%2x", &mac1[0], &mac1[1], &mac1[2], &mac1[3], &mac1[4], &mac1[5]);
    if (ret < 0) {
        return ret;
    }

    mac[0] = mac1[0];
    mac[1] = mac1[1];
    mac[2] = mac1[2];
    mac[3] = mac1[3];
    mac[4] = mac1[4];
    mac[5] = mac1[5];

    if (spe_mac_validate() != SPE_OK) {
        SPE_ERR("100: mac validate fail\n");
        return SPE_FAIL;
    }

    ret = memcpy_s(g_spe_cmd.u.mac_content.mac, sizeof(g_spe_cmd.u.mac_content.mac), mac, sizeof(mac));
    if (ret) {
        SPE_ERR("100: memcpy fail\n");
    }

    return SPE_OK;
}

int filter_action(const char *buf)
{
    if (0 == strcmp(buf, "add")) {
        g_spe_cmd.action = SPE_ENTRY_ADD;
    } else if (0 == strcmp(buf, "delete")) {
        g_spe_cmd.action = SPE_ENTRY_DELETE;
    } else if (0 == strcmp(buf, "dump")) {
        g_spe_cmd.action = SPE_ENTRY_DUMP;
    } else {
        SPE_ERR("action %s is not supported !\n", buf);
        return SPE_FAIL;
    }

    SPE_ERR("action = %s\n", buf);
    return SPE_OK;
}

int forward_action(const char *buf)
{
    if (0 == strcmp(buf, "dump")) {
        g_spe_cmd.action = SPE_ENTRY_DUMP;
    } else {
        SPE_ERR("action %s is not supported !\n", buf);
        return SPE_FAIL;
    }

    SPE_ERR("action = %s\n", buf);
    return SPE_OK;
}

int filter_mac_reaction(const char *buf)
{
    int ret;
    unsigned redir_action;
    ret = espe_input(buf, "%4u", &redir_action);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.u.mac_content.action = redir_action;
    SPE_ERR("redir acriont = %d\n", redir_action);
    return SPE_OK;
}

int filter_mac_redirect(const char *buf)
{
    int ret;
    unsigned int port_index = 255;

    ret = espe_input(buf, "%3d", &port_index);
    if (ret < 0) {
        return ret;
    }
    if (port_index > SPE_PORT_NUM) {
        SPE_ERR("redir port can't be %d\n", port_index);
        return SPE_FAIL;
    }

    g_spe_cmd.u.mac_content.redirect = port_index;
    SPE_ERR("redir port = %d\n", port_index);
    return SPE_OK;
}

int filter_ip_mask(const char *buf)
{
    int ret;
    int mask;

    ret = espe_input(buf, "%4x", &mask);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.u.ip_content.filter_mask = (unsigned int)mask;
    SPE_ERR("mask = 0x%x\n", mask);

    return SPE_OK;
}

int filter_ip_addr(const char *buf, int type)
{
    int ret;
    unsigned int ip[4] = {0};

    ret = espe_input(buf, "%4d.%4d.%4d.%4d", &ip[0], &ip[1], &ip[2], &ip[3]);
    if (ret < 0) {
        return ret;
    }

    if (type == SPE_DEST_IP) {
        // dst_addr
        SPE_ERR("dip:*,*,%d,%d\n", ip[2], ip[3]);
        g_spe_cmd.u.ip_content.u.spe_ipv4.dst_addr = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
    } else if (type == SPE_SRC_IP) {
        // src_addr
        g_spe_cmd.u.ip_content.u.spe_ipv4.src_addr = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
        SPE_ERR("sip:*, *, %d, %d \n", ip[2], ip[3]);
    } else {
        SPE_ERR("type = %d", type);
        return SPE_FAIL;
    }

    return SPE_OK;
}

int filter_ip_src_addr(const char *buf)
{
    return filter_ip_addr(buf, SPE_SRC_IP);
}

int filter_ip_dest_addr(const char *buf)
{
    return filter_ip_addr(buf, SPE_DEST_IP);
}

int filter_ip_addr_mask(const char *buf, int type)
{
    int ret;
    unsigned int mask;

    ret = espe_input(buf, "%4d", &mask);
    if (ret < 0) {
        return ret;
    }

    if (mask > SPE_IPV4_MASK_LEN) {
        SPE_ERR("mask = %d", mask);
        return SPE_FAIL;
    }

    if (type == SPE_DEST_IP) {
        // dst_mask
        SPE_ERR("dmask:%d \n", mask);
        g_spe_cmd.u.ip_content.u.spe_ipv4.dst_addr_mask = mask;
    } else if (type == SPE_SRC_IP) {
        // src_mask
        SPE_ERR("smask:%d \n", mask);
        g_spe_cmd.u.ip_content.u.spe_ipv4.src_addr_mask = mask;
    } else {
        SPE_ERR("type = %d\n", type);
        return SPE_FAIL;
    }

    return SPE_OK;
}

int filter_ip_src_addr_mask(const char *buf)
{
    return filter_ip_addr_mask(buf, SPE_SRC_IP);
}

int filter_ip_dest_addr_mask(const char *buf)
{
    return filter_ip_addr_mask(buf, SPE_DEST_IP);
}

int filter_ipv4_protocol(const char *buf)
{
    if (0 == strcmp(buf, "tcp")) {
        g_spe_cmd.u.ip_content.u.spe_ipv4.protocol = IPPROTO_TCP;
    } else if (0 == strcmp(buf, "udp")) {
        g_spe_cmd.u.ip_content.u.spe_ipv4.protocol = IPPROTO_UDP;
    } else {
        SPE_ERR("protocol %s error ,and protocol should be tcp or udp\n", buf);
        return SPE_FAIL;
    }

    SPE_ERR("protocol = %s\n", buf);
    return SPE_OK;
}

int filter_tos(const char *buf)
{
    int ret;
    unsigned int tos = SPE_IPV4_TOS_DEF;
    ret = espe_input(buf, "%1d", &tos);
    if (ret < 0) {
        return ret;
    }

    if (tos > SPE_IPV4_TOS_MAX) {
        SPE_ERR("tos = %d ,out of rage!\n", tos);
        return SPE_FAIL;
    }

    g_spe_cmd.u.ip_content.u.spe_ipv4.tos = tos;
    SPE_ERR("tos = %d\n", tos);
    return SPE_OK;
}

int filter_src_port_low(const char *buf)
{
    int ret;
    unsigned int scr_port_low;
    ret = espe_input(buf, "%5d", &scr_port_low);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.u.ip_content.src_port_low = scr_port_low;
    SPE_ERR("scr port low is %d\n", scr_port_low);
    return SPE_OK;
}

int filter_src_port_high(const char *buf)
{
    int ret;
    unsigned int scr_port_high;
    ret = espe_input(buf, "%5d", &scr_port_high);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.u.ip_content.src_port_high = scr_port_high;
    SPE_ERR("scr port high is %d\n", scr_port_high);
    return SPE_OK;
}

int filter_dest_port_low(const char *buf)
{
    int ret;
    unsigned int dst_port_low;
    ret = espe_input(buf, "%5d", &dst_port_low);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.u.ip_content.dst_port_low = dst_port_low;
    SPE_ERR("dst port low is %d\n", dst_port_low);
    return SPE_OK;
}

int filter_dest_port_high(const char *buf)
{
    int ret;
    unsigned int dst_port_high;
    ret = espe_input(buf, "%5d", &dst_port_high);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.u.ip_content.dst_port_high = dst_port_high;
    SPE_ERR("dst port high is %d\n", dst_port_high);
    return SPE_OK;
}

int spe_black_white_mode_set(const char *buf)
{
    if (0 == strcmp(buf, "mac")) {
        g_spe_cmd.spe_blackwhite_mode = 0;
    } else if (0 == strcmp(buf, "ip")) {
        g_spe_cmd.spe_blackwhite_mode = 1;
    } else {
        SPE_ERR("paramate error! mode can't be %s\n", buf);
        return SPE_FAIL;
    }
    SPE_ERR("change %s black/white\n", buf);
    return SPE_OK;
}

int spe_black_white_set(const char *buf)
{
    if (0 == strcmp(buf, "black")) {
        g_spe_cmd.spe_blackwhite = 1;
    } else if (0 == strcmp(buf, "white")) {
        g_spe_cmd.spe_blackwhite = 0;
    } else {
        SPE_ERR("paramate error! please input black or white\n");
        return SPE_FAIL;
    }
    SPE_ERR("change %s black/white\n", buf);
    return SPE_OK;
}

int set_action(const char *buf)
{
    if (0 == strcmp(buf, "set")) {
        g_spe_cmd.spe_param_action = SPE_PARAM_SET;
    } else if (0 == strcmp(buf, "get")) {
        g_spe_cmd.spe_param_action = SPE_PARAM_GET;
    } else {
        SPE_ERR("paramate error! action should be set or get");
        return SPE_FAIL;
    }

    SPE_ERR("action = %s\n", buf);
    return SPE_OK;
}

int spe_eth_min_len_value_set(const char *buf)
{
    int ret;

    ret = espe_input(buf, "%4d", &g_spe_cmd.spe_eth_set_min_len);
    if (ret < 0) {
        return ret;
    }
    SPE_TRACE("input value is %d\n", g_spe_cmd.spe_eth_set_min_len);

    // spe_eth_minlen bit 0-10bits
    if (g_spe_cmd.spe_eth_set_min_len >= SPE_ETH_SET_MAX_LEN || g_spe_cmd.spe_eth_set_min_len <= SPE_ETH_SET_MIN_LEN) {
        SPE_ERR("out of range(17-2047):input value %d\n", g_spe_cmd.spe_eth_set_min_len);
        return SPE_FAIL;
    }

    SPE_ERR("set eth min len %d\n", g_spe_cmd.spe_eth_set_min_len);
    return SPE_OK;
}

int spe_udp_lim_portno_set(const char *buf)
{
    int ret;

    ret = espe_input(buf, "%4d", &g_spe_cmd.spe_udp_lim_portno);
    if (ret < 0) {
        return ret;
    }
    if (g_spe_cmd.spe_udp_lim_portno >= SPE_PORT_NUM) {
        SPE_ERR("out of range:port num can't be %d\n", g_spe_cmd.spe_udp_lim_portno);
        return SPE_FAIL;
    }

    SPE_ERR("set %d port udp limite\n", g_spe_cmd.spe_udp_lim_portno);
    return SPE_OK;
}

int udp_rate_set(const char *buf)
{
    int ret;

    ret = espe_input(buf, "0x%8x", &g_spe_cmd.spe_udp_lim_portno);
    if (ret < 0) {
        return ret;
    }
    if (g_spe_cmd.spe_udp_lim_portno > SPE_PORT_UDP_LIM_MAX) {
        SPE_ERR("out of range:udp limit can't be 0x%x\n", g_spe_cmd.spe_udp_lim_portno);
        return SPE_FAIL;
    }

    SPE_ERR("set udp limite:0x%x \n", g_spe_cmd.spe_udp_lim_portno);
    return SPE_OK;
}

int port_rate_set(const char *buf)
{
    int ret;

    ret = espe_input(buf, "0x%8x", &g_spe_cmd.spe_port_rate);
    if (ret < 0) {
        return ret;
    }
    if (g_spe_cmd.spe_port_rate > SPE_PORT_BYTE_LIM_MAX) {
        SPE_ERR("out of range:byte limit can't be 0x%x\n", g_spe_cmd.spe_port_rate);
        return SPE_FAIL;
    }

    SPE_ERR("set byte limite:0x%x\n", g_spe_cmd.spe_port_rate);
    return SPE_OK;
}

int spe_port_lim_time_set(const char *buf)
{
    int ret;

    ret = espe_input(buf, "0x%4x", &g_spe_cmd.spe_port_lim_time);
    if (ret < 0) {
        return ret;
    }
    if (g_spe_cmd.spe_port_lim_time > SPE_PORT_LIM_TIME_MAX) {
        SPE_ERR("out of range:byte limit can't be 0x%x\n", g_spe_cmd.spe_port_lim_time);
        return SPE_FAIL;
    }

    SPE_ERR("set limite time 0x%x\n", g_spe_cmd.spe_port_lim_time);
    return SPE_OK;
}

int spe_switch(const char *buf)
{
    if (0 == strcmp(buf, "on")) {
        g_spe_cmd.onoff = 0;
    } else if (0 == strcmp(buf, "off")) {
        g_spe_cmd.onoff = 1;
    } else {
        SPE_ERR("parameter error! switch should be on or off\n");
        return SPE_FAIL;
    }

    SPE_ERR("set switch %s\n", buf);
    return SPE_OK;
}

int spe_l4_protocol_get(const char *buf)
{
    if (0 == strcmp(buf, "tcp")) {
        g_spe_cmd.spe_l4_protocol = IPPROTO_TCP;
    } else if (0 == strcmp(buf, "udp")) {
        g_spe_cmd.spe_l4_protocol = IPPROTO_UDP;
    } else {
        SPE_ERR("protocol %s error ,and protocol should be tcp or udp\n", buf);
        return SPE_FAIL;
    }

    SPE_ERR("protocol = %s\n", buf);
    return SPE_OK;
}

int spe_l4_portnum_get(const char *buf)
{
    int ret;
    int tmp;
    ret = espe_input(buf, "%5d", &tmp);
    if (ret < 0) {
        return ret;
    }
    g_spe_cmd.spe_l4_port_num = (u16)tmp;
    SPE_ERR("set l4 portnum : %d\n", g_spe_cmd.spe_l4_port_num);
    return SPE_OK;
}

int char_to_num(char c)
{
    if ((c >= '0') && (c <= '9')) {
        return c - '0';
    } else if ((c >= 'a') && (c <= 'f')) {
        return c - 'a' + 10;
    } else if ((c >= 'A') && (c <= 'F')) {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

void ipv6_resolution(char *ipv6_l_input, unsigned int *ipv6_m, unsigned int ipv6_m_len, int *m, int *n)
{
    int *l = m;
    int i = 0;
    char *ipv6_l = ipv6_l_input;

    while ('\0' != *ipv6_l) {
        if (':' == *ipv6_l) {
            l = n;
            ipv6_l++;
            continue;
        }
        while ((':' != *ipv6_l) && ('\0' != *ipv6_l)) {
            ipv6_m[i] = ipv6_m[i] * 16 + char_to_num(*ipv6_l);
            ipv6_l++;
            continue;
        }
        if ('\0' == *ipv6_l) {
            (*l)++;
            return;
        }
        ipv6_l++;
        (*l)++;
        i++;
    }
    return;
}

int filter_ipv6_addr(const char *buf, int type)
{
    int ret;
    unsigned int ipv6[SPE_IPV6_ADDR_LEN] = {0};
    unsigned int ipv6_m[SPE_IPV6_ADDR_LEN] = {0};
    char ipv6_l[SPE_IPV6_ADDR_BUF] = {0};
    int m = 0;
    int n = 0;
    int i = 0;
    int d;

    ret = espe_input(buf, "%49s", ipv6_l, SPE_IPV6_ADDR_BUF);
    if (ret < 0) {
        return ret;
    }
    SPE_TRACE("%s\n", ipv6_l);
    ipv6_resolution(ipv6_l, ipv6_m, sizeof(ipv6_m), &m, &n);
    SPE_TRACE("ipv6_m  *:*:*:%x:%x:%x:%x:%x\n", ipv6_m[0], ipv6_m[1], ipv6_m[2], ipv6_m[3], ipv6_m[4], ipv6_m[5],
              ipv6_m[6], ipv6_m[7]);

    d = SPE_IPV6_ADDR_LEN - m - n;

    for (i = 0; i < m; i++) {
        ipv6[i] = ipv6_m[i];
    }

    for (i = 0; i < d; i++) {
        ipv6[m + i] = 0;
    }

    for (i = 0; i < n; i++) {
        ipv6[m + d + i] = ipv6_m[m + i];
    }

    if (type == SPE_DEST_IP) {
        // dst_addr
        g_spe_cmd.u.ip_content.u.spe_ipv6.dst_addr[0] = (ipv6[0] << 16) | ipv6[1];
        g_spe_cmd.u.ip_content.u.spe_ipv6.dst_addr[1] = (ipv6[2] << 16) | ipv6[3];
        g_spe_cmd.u.ip_content.u.spe_ipv6.dst_addr[2] = (ipv6[4] << 16) | ipv6[5];
        g_spe_cmd.u.ip_content.u.spe_ipv6.dst_addr[3] = (ipv6[6] << 16) | ipv6[7];
    } else if (type == SPE_SRC_IP) {
        g_spe_cmd.u.ip_content.u.spe_ipv6.src_addr[0] = (ipv6[0] << 16) | ipv6[1];
        g_spe_cmd.u.ip_content.u.spe_ipv6.src_addr[1] = (ipv6[2] << 16) | ipv6[3];
        g_spe_cmd.u.ip_content.u.spe_ipv6.src_addr[2] = (ipv6[4] << 16) | ipv6[5];
        g_spe_cmd.u.ip_content.u.spe_ipv6.src_addr[3] = (ipv6[6] << 16) | ipv6[7];
    } else {
        SPE_TRACE("type = %d", type);
        return SPE_FAIL;
    }

    return SPE_OK;
}

int filter_ipv6_src_addr(const char *buf)
{
    return filter_ipv6_addr(buf, SPE_SRC_IP);
}

int filter_ipv6_dest_addr(const char *buf)
{
    return filter_ipv6_addr(buf, SPE_DEST_IP);
}

int filter_ipv6_addr_mask(const char *buf, int type)
{
    int ret;
    unsigned int mask;

    ret = espe_input(buf, "%4d", &mask);
    if (ret < 0) {
        return ret;
    }

    if (mask > SPE_IPV6_MASK_LEN) {
        SPE_ERR("out of range, mask shoule less then 128");
        return SPE_FAIL;
    }

    if (type == SPE_DEST_IP) {
        // dst_addr
        SPE_ERR("dmask:%d \n", mask);
        g_spe_cmd.u.ip_content.u.spe_ipv6.dst_addr_mask = mask;
    } else if (type == SPE_SRC_IP) {
        SPE_ERR("smask:%d \n", mask);
        g_spe_cmd.u.ip_content.u.spe_ipv6.src_addr_mask = mask;
    } else {
        SPE_ERR("type = %d error \n", type);
    }

    return SPE_OK;
}
int filter_ipv6_src_addr_mask(const char *buf)
{
    return filter_ipv6_addr_mask(buf, SPE_SRC_IP);
}

int filter_ipv6_dest_addr_mask(const char *buf)
{
    return filter_ipv6_addr_mask(buf, SPE_DEST_IP);
}

int filter_ipv6_protocol(const char *buf)
{
    if (0 == strcmp(buf, "tcp")) {
        g_spe_cmd.u.ip_content.u.spe_ipv6.protocol = IPPROTO_TCP;
    } else if (0 == strcmp(buf, "udp")) {
        g_spe_cmd.u.ip_content.u.spe_ipv6.protocol = IPPROTO_UDP;
    } else {
        SPE_ERR("protocol %s error ,and protocol should be tcp or udp\n", buf);
        return SPE_FAIL;
    }

    return SPE_OK;
}

int filter_ipv6_tc(const char *buf)
{
    int ret;
    unsigned int tc = SPE_IPV6_TC_DEF;
    ret = espe_input(buf, "%1d", &tc);
    if (ret < 0) {
        return ret;
    }

    if (tc >= SPE_IPV6_TC_MAX) {
        SPE_ERR("tc = %s out of range\n", buf);
        return SPE_FAIL;
    }

    g_spe_cmd.u.ip_content.u.spe_ipv6.tc = tc;
    return SPE_OK;
}

int filter_ipv6_next_hdr(const char *buf)
{
    int ret;
    unsigned int hdr;
    ret = espe_input(buf, "%4d", &hdr);
    if (ret < 0) {
        return ret;
    }

    g_spe_cmd.u.ip_content.u.spe_ipv6.next_hdr = hdr;
    return SPE_OK;
}

int filter_ipv6_flow_label(const char *buf)
{
    int ret;
    unsigned int label;
    ret = espe_input(buf, "%4d", &label);
    if (ret < 0) {
        return ret;
    }

    g_spe_cmd.u.ip_content.u.spe_ipv6.flow_label = label;
    return SPE_OK;
}

static char spe_dash2underscore(char c)
{
    if (c == '-') {
        return '_';
    }
    return c;
}

bool spe_parameqn(const char *a, const char *b, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (spe_dash2underscore(a[i]) != spe_dash2underscore(b[i]))
            return false;
    }
    return true;
}

bool spe_parameq(const char *a, const char *b)
{
    return spe_parameqn(a, b, strlen(a) + 1);
}

/*
 * cmd line parse routine begins here
 */
static int spe_parse_one(char *param, const char *val)
{
    int i;
    int err;

    SPE_TRACE("param=%s,strlen(param)=%d,val=%s\n", param, strlen(param), val);

    if (val == NULL) {
        return -EINVAL;
    }

    /* Find parameter */
    SPE_TRACE("num:%d\n", g_spe_cmd.num_funcs);
    for (i = 0; i < g_spe_cmd.num_funcs; i++) {
        if (spe_parameq(param, g_spe_cmd.filter_parser_ops[i].name)) {
            /* No one handled NULL, so do it here. */
            SPE_TRACE("name:%s\n", g_spe_cmd.filter_parser_ops[i].name);
            err = g_spe_cmd.filter_parser_ops[i].setup_func(val);
            return err;
        }
    }

    SPE_ERR("unknown argument: %s\n", param);
    return SPE_FAIL;
}

static char *find_next_arg(char *args, char **param, char **val)
{
    unsigned int i;
    unsigned int equals = 0;
    char *next = NULL;

    for (i = 0; args[i]; i++) {
        if (isspace(args[i])) {
            break;
        }
        if (equals == 0) {
            if (args[i] == '=') {
                equals = i;
            }
        }
    }

    *param = args;
    if (!equals) {
        *val = NULL;
    } else {
        args[equals] = '\0';
        *val = args + equals + 1;
    }

    if (args[i]) {
        args[i] = '\0';
        next = args + i + 1;
    } else {
        next = args + i;
    }

    /* Chew up trailing spaces. */
    return skip_spaces(next);
}

int exe_spe_mac_filter_cmd(void)
{
    int ret = SPE_OK;

    if (g_spe_cmd.action == SPE_ENTRY_DUMP) {
        espe_macfl_entry_dump();
    } else {
        if (g_spe_cmd.type == SPE_MAC_FILTER_TABLE) {
            // table_ops ipv4_filt_act
            ret = espe_mac_filt_act(&g_spe_cmd);
            if (ret) {
                SPE_ERR("espe mac filt act err.\n");
            }
        }
    }

    if (ret) {
        SPE_ERR("exe spe mac filter cmd error!\n");
    }
    return ret;
}

int exe_spe_mac_forward_cmd(void)
{
    if (g_spe_cmd.action == SPE_ENTRY_DUMP) {
        espe_macfw_entry_dump();
    }

    return 0;
}

int exe_spe_black_white_cmd(enum spe_tab_list_type list_type, unsigned int black_white)
{
    spe_black_white_t black_white_reg;
    struct spe *spe = &g_espe;

    black_white_reg.u32 = spe_readl(spe->regs, SPE_BLACK_WHITE);
    if (g_spe_cmd.spe_param_action == SPE_PARAM_SET) {
        switch (list_type) {
            case SPE_TAB_LIST_MAC_FILT:
                black_white_reg.bits.spe_mac_black_white = !!black_white;
                spe_writel(spe->regs, SPE_BLACK_WHITE, black_white_reg.u32);
                break;

            case SPE_TAB_LIST_IPV4_FILT:
            case SPE_TAB_LIST_IPV6_FILT:
                black_white_reg.bits.spe_ip_black_white = !!black_white;
                spe_writel(spe->regs, SPE_BLACK_WHITE, black_white_reg.u32);
                break;

            default:
                SPE_ERR("invalid list_type(%d)\n", list_type);
                break;
        }
        SPE_TRACE("spe set black white \n");
    } else if (g_spe_cmd.spe_param_action == SPE_PARAM_GET) {
        SPE_ERR("mac:%s, ip:%s\n", black_white_reg.bits.spe_ip_black_white ? "black" : "white",
                black_white_reg.bits.spe_mac_black_white ? "black" : "white");
    } else {
        SPE_ERR("black/white set parameter error!\n");
    }
    return 0;
}

void espe_min_len_act(unsigned int min_len, unsigned int is_write)
{
    struct spe *spe = &g_espe;
    unsigned int minlen;

    minlen = spe_readl(spe->regs, SPE_ETH_MINLEN);

    if (is_write) {
        spe_writel(spe->regs, SPE_ETH_MINLEN, min_len);
    }
    SPE_ERR("eth min len:0x%x\n", minlen);
    return;
}

int exe_spe_eth_min_len_cmd(void)
{
    if (g_spe_cmd.type == SPE_ETH_MIN_LEN) {
        if (g_spe_cmd.spe_param_action == SPE_PARAM_SET) {
            espe_min_len_act(g_spe_cmd.spe_eth_set_min_len, 1);
        } else if (g_spe_cmd.spe_param_action == SPE_PARAM_GET) {
            espe_min_len_act(g_spe_cmd.spe_eth_set_min_len, 0);
        } else {
            SPE_ERR("eth_min_len set parameter error!\n");
        }
    }
    return 0;
}

int exe_spe_ip_filter_cmd(void)
{
    int ret = SPE_OK;

    if (g_spe_cmd.action == SPE_ENTRY_DUMP) {
        if (g_spe_cmd.u.ip_content.iptype == 0) {
            spe_ipv4_filter_dump();
        }

        if (g_spe_cmd.u.ip_content.iptype == 1) {
            spe_ipv6_filter_dump();
        }
    } else {
        if (g_spe_cmd.type == SPE_IP_FILTER_TABLE) {
            // table_ops ipv4_filt_act
            ret = spe_ipv4_filt_act(&g_spe_cmd);
            if (ret) {
                SPE_ERR("spe ipv4 filt act err.\n");
            }
        } else if (g_spe_cmd.type == SPE_IPV6_FILTER_TABLE) {
            // table_ops ipv6_filt_act
            ret = spe_ipv6_filt_act(&g_spe_cmd);
            if (ret) {
                SPE_ERR("spe ipv6 filt act err.\n");
            }
        }
    }
    return ret;
}

unsigned int espe_udp_limit_num_action(unsigned int portno, unsigned int udp_limi_max, unsigned int is_write)
{
    struct spe *spe = &g_espe;
    spe_port_lmttime_t en_status;

    if (is_write) {
        spe_writel(spe->regs, SPE_PORTX_UDP_LMTNUM(portno), udp_limi_max);
        en_status.u32 = spe_readl(spe->regs, SPE_PORT_LMTTIME);
        if (udp_limi_max != 0) {
            en_status.bits.spe_port_lmt_en |= 1 << portno;
        } else {
            en_status.bits.spe_port_lmt_en &= ~(1 << portno);
        }
        spe_writel(spe->regs, SPE_PORT_LMTTIME, en_status.u32);
        return 0;
    }

    return spe_readl(spe->regs, SPE_PORTX_UDP_LMTNUM(portno));
}

unsigned int espe_set_byte_limit_num(unsigned int portno, unsigned int byte_limi_max, unsigned int is_write)
{
    struct spe *spe = &g_espe;
    spe_port_lmttime_t en_status;

    if (is_write) {
        spe_writel(spe->regs, SPE_PORTX_LMTBYTE(portno), byte_limi_max);
        en_status.u32 = spe_readl(spe->regs, SPE_PORT_LMTTIME);
        if (byte_limi_max != 0) {
            en_status.bits.spe_port_lmt_en |= 1 << portno;
        } else {
            en_status.bits.spe_port_lmt_en &= ~(1 << portno);
        }
        spe_writel(spe->regs, SPE_PORT_LMTTIME, en_status.u32);

        return 0;
    }

    return spe_readl(spe->regs, SPE_PORTX_LMTBYTE(portno));
}

void espe_set_port_limit(unsigned int limit_time, unsigned int is_write)
{
    struct spe *spe = &g_espe;
    spe_port_lmttime_t lim_time;
    lim_time.u32 = spe_readl(spe->regs, SPE_PORT_LMTTIME);

    if (is_write) {
        lim_time.bits.spe_port_lmttime = limit_time;
        spe_writel(spe->regs, SPE_PORT_LMTTIME, lim_time.u32);
    }
    SPE_TRACE("limit time:0x%x\n", lim_time.u32);
}

int exe_spe_dynamic_swich_cmd(void)
{
    int ret = SPE_OK;
    if (g_spe_cmd.onoff) {
        espe_transfer_pause();
    } else {
        espe_transfer_restart();
    }
    SPE_ERR("dynamic swich is %s \n", g_spe_cmd.onoff ? "off" : "on");

    return ret;
}

int exe_spe_port_l4portno_cmd(void)
{
    int ret = SPE_OK;

    if (g_spe_cmd.action == SPE_ENTRY_ADD) {
        ret = spe_port_l4portno_to_cpu_add(g_spe_cmd.spe_l4_port_num, g_spe_cmd.spe_l4_protocol);
    } else if (g_spe_cmd.action == SPE_ENTRY_DELETE) {
        ret = spe_port_l4portno_to_cpu_del(g_spe_cmd.spe_l4_port_num, g_spe_cmd.spe_l4_protocol);
    } else if (g_spe_cmd.action == SPE_ENTRY_DUMP) {
        spe_port_l4portno_to_cpu_dump();
    } else {
        SPE_ERR("l4 portnum set parameter error!\n");
    }

    return ret;
}

int exe_spe_cmd(void)
{
    int ret = 0;

    // write spe reg
    switch (g_spe_cmd.type) {
        case SPE_IP_FILTER_TABLE:
            ret = exe_spe_ip_filter_cmd();
            break;
        case SPE_IPV6_FILTER_TABLE:
            ret = exe_spe_ip_filter_cmd();
            break;
        case SPE_MAC_FILTER_TABLE:
            ret = exe_spe_mac_filter_cmd();
            break;
        case SPE_MAC_FORWARD_TABLE:
            ret = exe_spe_mac_forward_cmd();
            break;
        case SPE_BLACK_WHITE_LIST:
            ret = exe_spe_black_white_cmd(g_spe_cmd.spe_blackwhite_mode, g_spe_cmd.spe_blackwhite);
            break;
        case SPE_ETH_MIN_LEN:
            ret = exe_spe_eth_min_len_cmd();
            break;
        case SPE_UDP_LMTNUM_TABLE:
            espe_udp_limit_num_action(g_spe_cmd.spe_udp_lim_portno, g_spe_cmd.spe_udp_rate, g_spe_cmd.spe_param_action);
            SPE_TRACE("udp num limit:0x%x\n", g_spe_cmd.spe_udp_rate);
            break;
        case SPE_LMTBYTE_TABLE:
            espe_set_byte_limit_num(g_spe_cmd.spe_udp_lim_portno, g_spe_cmd.spe_port_rate, g_spe_cmd.spe_param_action);
            SPE_TRACE("byte rate limit:0x%x\n", g_spe_cmd.spe_port_rate);
            break;
        case SPE_LMT_TIME_TABLE:
            espe_set_port_limit(g_spe_cmd.spe_port_lim_time, g_spe_cmd.spe_param_action);
            break;
        case SPE_DYNAMIC_SWITCH:
            ret = exe_spe_dynamic_swich_cmd();
            break;
        case SPE_L4_PORTNUM:
            ret = exe_spe_port_l4portno_cmd();
            break;
        case SPE_DEL_ALL_ENTRYS:
            SPE_ERR("not support for now. \n");
            break;
        default:
            SPE_ERR("g_spe_cmd.type = %d, error", g_spe_cmd.type);
            ret = SPE_FAIL;
            break;
    }

    return ret;
}

int filter_cmd_parser(char *args, unsigned int args_len)
{
    char *param = NULL;
    char *val = NULL;
    int ret;
    char type_str[SPE_STR_LEN] = {0};

    SPE_TRACE("Parsing ARGS: %s\n", args);

#ifdef HISI_SPE_DEBUG
    g_last_filter_type = g_spe_cmd.type;  // just for debug
#endif

    if (args == NULL) {
        SPE_ERR("args NULL.\n");
        return SPE_FAIL;
    }

    // clean all field
    ret = memset_s(&g_spe_cmd, sizeof(g_spe_cmd), 0, sizeof(struct spe_cmd_handler_s));
    if (ret) {
        SPE_ERR("spe_sysfs_cmd_type g_spe_cmd clean fail. \n");
    }

    /* clean leading spaces */
    args = skip_spaces(args);
    if (args == NULL) {
        SPE_ERR("args NULL.\n");
        return SPE_FAIL;
    }
    // cmdline must start with  "type=xx"  xx : ip, ipv6, mac
    // get type first
    if (0 != strncmp(args, "type=", strlen("type="))) {
        filter_cmd_help(args);
        return SPE_FAIL;
    }

    ret = memset_s(type_str, sizeof(type_str), 0, SPE_STR_LEN);
    if (ret) {
        SPE_ERR("spe_sysfs_cmd_type type_str clean fail \n");
    }
    // need to limit size of type_str
    ret = espe_input(args, "type=%15s", type_str, SPE_STR_LEN);
    if (ret < 0) {
        return ret;
    }
    ret = spe_sysfs_cmd_type(type_str, sizeof(type_str));
    if (ret == SPE_FAIL) {
        SPE_TRACE("cmd: %s\n", args);
        SPE_TRACE("cmd: %s, fail\n", type_str);
    }

    args = args + strlen(type_str) + strlen("type=");
    args = skip_spaces(args);

    SPE_TRACE("args=%s\n", args);
    while ((args != NULL) && *args) {
        int ret;

        args = find_next_arg(args, &param, &val);
        ret = spe_parse_one(param, val);

        switch (ret) {
            case SPE_FAIL:
                SPE_ERR("parameter error '%s'\n", val ? : "");
                return ret;

            case SPE_OK:
                break;

            default:
                SPE_ERR("'%s' invalid for parameter '%s'\n", val ? : "", param);
                return ret;
        }
    }

    return ret;
}

void filter_cmd_help(char *args)
{
    if (args != NULL) {
        SPE_TRACE("cmd: %s\n", args);
    }

    SPE_ERR("cmd should start with type=ip, type=ipv6 or type=mac\n");

    return;
}

void spe_ipv4_filter_dump(void)
{
    unsigned int spe_tab[SPE_IPV4_TAB_WORD_NO];
    int i, j;
    struct spe *spe = &g_espe;

    dbgen_en();

    for (i = 0; i < SPE_IPV4_TAB_NUMBER; i++) {
        for (j = 0; j < SPE_IPV4_TAB_WORD_NO; j++) {
            spe_tab[j] = spe_readl(spe->regs, SPE_IPV4_TAB(i * SPE_IPV4_TAB_WORD_NO + j));
        }
        ipv4_filt_dump(spe_tab, i);
    }
    dbgen_dis();
}

void spe_ipv6_filter_dump(void)
{
    struct spe *spe = &g_espe;
    unsigned int spe_tab[SPE_IPV6_TAB_WORD_NO];
    int i, j;

    dbgen_en();

    for (i = 0; i < SPE_IPV6_TAB_NUMBER; i++) {
        for (j = 0; j < SPE_IPV6_TAB_WORD_NO; j++) {
            spe_tab[j] = spe_readl(spe->regs, SPE_IPV6_TAB(i * SPE_IPV6_TAB_WORD_NO + j));
        }
        ipv6_filt_dump(spe_tab, i);
    }
    dbgen_dis();
}
