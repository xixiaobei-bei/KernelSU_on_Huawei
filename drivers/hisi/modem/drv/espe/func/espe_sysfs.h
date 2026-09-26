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

#ifndef ESPE_SYSFS_H
#define ESPE_SYSFS_H

#define SPE_OK 0
#define SPE_FAIL 1

#define SPE_DEST_IP 1
#define SPE_SRC_IP 2

#define SPE_IP_FILTER_TABLE 1
#define SPE_IPV6_FILTER_TABLE 2
#define SPE_MAC_FILTER_TABLE 3
#define SPE_MAC_FORWARD_TABLE 4
#define SPE_BLACK_WHITE_LIST 5
#define SPE_ETH_MIN_LEN 6
#define SPE_UDP_LMTNUM_TABLE 7
#define SPE_LMTBYTE_TABLE 8
#define SPE_LMT_TIME_TABLE 9
#define SPE_DYNAMIC_SWITCH 10
#define SPE_L4_PORTNUM 11
#define SPE_DEL_ALL_ENTRYS 12

#define SPE_PARAM_GET 0
#define SPE_PARAM_SET 1
#define SPE_ENTRY_ADD 2
#define SPE_ENTRY_DELETE 3
#define SPE_ENTRY_DUMP 4

#define HISI_SPE_DEBUG

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define SPE_IPV4_MASK_LEN 32

#define SPE_IPV4_TOS_DEF 255
#define SPE_IPV4_TOS_MAX 8

#define SPE_IPV6_MASK_LEN 128
#define SPE_IPV6_TC_DEF 255
#define SPE_IPV6_TC_MAX 8
#define SPE_IPV6_ADDR_LEN 8
#define SPE_IPV6_ADDR_BUF 50
#define SPE_ETH_SET_MIN_LEN 16
#define SPE_ETH_SET_MAX_LEN 2048

enum action {
    ADD = 0,
    DEL,
    DUMP,
    BOTTOM
};

struct spe_filter_param {
    const char *name;
    int (*setup_func)(const char *);
    int flag;  // just reserved now
};

struct mac_filter_entry {
    char mac[ETH_ALEN];
    char reserved;
    unsigned action : 4;
    unsigned redirect : 4;
};

struct mac_forward_entry {
    char mac[ETH_ALEN];
    u16 port_index;
};

struct ipv4_contex {
    unsigned int tos : 8;
    unsigned int protocol : 8;
    unsigned int src_addr;
    unsigned int src_addr_mask;
    unsigned int dst_addr;
    unsigned int dst_addr_mask;
};

struct ipv6_contex {
    unsigned int tc : 8;
    unsigned int next_hdr : 8;
    unsigned int flow_label;
    unsigned int src_addr[4];
    unsigned int src_addr_mask;
    unsigned int dst_addr[4];
    unsigned int dst_addr_mask;
    unsigned int protocol : 8;
};

struct ip_filter_entry {
    unsigned int filter_mask;
    unsigned int src_port_low : 16;
    unsigned int src_port_high : 16;
    unsigned int dst_port_low : 16;
    unsigned int dst_port_high : 16;
    unsigned int reserved : 15;
    unsigned int iptype : 1;
    union {
        struct ipv4_contex spe_ipv4;
        struct ipv6_contex spe_ipv6;
    } u;
};

struct spe_cmd_handler_s {
    int action;
    int type;
    int num_funcs;

    union {
        struct mac_filter_entry mac_content;
        struct ip_filter_entry ip_content;
    } u;

    struct spe_filter_param *filter_parser_ops;

    unsigned int spe_blackwhite_mode;
    unsigned int spe_blackwhite;
    unsigned int spe_eth_set_min_len;
    unsigned int spe_param_action;

    unsigned int spe_udp_lim_portno;
    unsigned int spe_udp_rate;
    unsigned int spe_port_lim_portno;
    unsigned int spe_port_rate;
    unsigned int spe_port_lim_time;

    unsigned int onoff;
    unsigned int spe_l4_protocol;
    unsigned short spe_l4_port_num;
};

struct mac_forward_handler_s {
    int action;
    int type;
    struct mac_forward_entry mac_forward;
};

int mac_filter_dump(void);
int ip_filter_dump(void);

int filter_cmd_parser(char *args, unsigned int args_len);

int exe_spe_cmd(void);

int filter_mac_addr(const char *buf);
int filter_action(const char *buf);
int filter_mac_reaction(const char *buf);
int filter_mac_redirect(const char *buf);

int forward_action(const char *buf);
int filter_ip_mask(const char *buf);
int filter_src_port_low(const char *buf);
int filter_src_port_high(const char *buf);
int filter_dest_port_low(const char *buf);

int filter_dest_port_high(const char *buf);

int filter_ip_src_addr(const char *buf);
int filter_ip_dest_addr(const char *buf);
int filter_ip_src_addr_mask(const char *buf);
int filter_ip_dest_addr_mask(const char *buf);
int filter_ipv4_protocol(const char *buf);
int filter_tos(const char *buf);
int filter_ipv6_src_addr(const char *buf);
int filter_ipv6_dest_addr(const char *buf);
int filter_ipv6_src_addr_mask(const char *buf);
int filter_ipv6_dest_addr_mask(const char *buf);
int filter_ipv6_protocol(const char *buf);
int filter_ipv6_tc(const char *buf);

int filter_ipv6_next_hdr(const char *buf);
int filter_ipv6_flow_label(const char *buf);
void filter_cmd_help(char *args);

int spe_black_white_mode_set(const char *buf);
int spe_black_white_set(const char *buf);
int spe_eth_min_len_value_set(const char *buf);
int spe_udp_lim_portno_set(const char *buf);
int udp_rate_set(const char *buf);
int spe_port_lim_portno_set(const char *buf);
int port_rate_set(const char *buf);
int spe_port_lim_time_set(const char *buf);

int spe_switch(const char *buf);
int set_action(const char *buf);

int spe_l4_protocol_get(const char *buf);
int spe_l4_portnum_get(const char *buf);

void spe_ipv4_filter_dump(void);
void spe_ipv6_filter_dump(void);

#endif
