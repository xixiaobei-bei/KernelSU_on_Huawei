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

#ifndef __SPE_MAC_ENTRY_H__
#define __SPE_MAC_ENTRY_H__

#include <linux/types.h>
#include <linux/slab.h>

#include "espe.h"

#define IS_MAC_FW_ENTRY (BIT(17))
#define IS_MAC_FL_ENTRY (BIT(16))
#define IS_QOS_MAC_ENTRY (BIT(26))

struct espe_mac_fw_entry {
    unsigned int mac_hi; /* mac in big endian */
    unsigned int mac_lo; /* mac in big endian */

    unsigned int vid : 12; /* vlan id in big endian */
    unsigned int reserve0 : 4;
    unsigned int port_br_id : 5; /* br id this entry belongs to */
    unsigned int reserve1 : 3;
    unsigned int port_no : 5; /* port no */
    unsigned int reserve2 : 3;

    unsigned int timestamp : 16; /* mac fw timestamp,software no care. */
    unsigned int is_static : 1;  /* is static entry */
    unsigned int reserve3 : 7;
    unsigned int userfiled : 8;
};

struct espe_mac_fw_entry_ext {
    struct list_head list;
    struct espe_mac_fw_entry ent;
};

struct espe_mac_filt_entry {
    unsigned int mac_hi;        /* mac in big endian */
    unsigned int mac_lo;        /* mac in big endian */
    unsigned int redir_act : 1; /* 0:no redir 1:redir */
    unsigned int reserve1 : 7;
    unsigned int redir_port : 5; /* redir port */
    unsigned int reserve2 : 19;
};

/* spe_mac_entry read from sram */
struct espe_mac_entry_in_sram {
    unsigned long long action : 1;
    unsigned long long rd_port : 5;
    unsigned long long bid : 5;
    unsigned long long vid : 12;
    unsigned long long port : 5;
    unsigned long long timestamp : 16;
    unsigned long long userfiled : 8;
    unsigned long long is_static : 1;
    unsigned long long reserve : 11;
    char mac[ETH_ALEN];
    char mac_fil_vld : 1;
    char mac_fw_vld : 1;
};

#ifdef SPEV300_SUPPORT

struct espe_qos_mac_entry {
    unsigned int mac_hi; /* mac in big endian */
    unsigned int mac_lo; /* mac in big endian */
    unsigned int limit_byte_lo;
    unsigned int limit_byte_hi : 8;
    unsigned int limit_en : 2;
    unsigned int reserver : 22;
};

/* spe_mac_qos_entry read from sram */
struct espe_qos_mac_entry_low_in_sram {
    unsigned long long lmt_en : 2;
    unsigned long long lmt_byte : 40;
    unsigned long long mac_low : 22;
};

struct espe_qos_mac_entry_high_in_sram {
    unsigned long long mac_high : 26;
    unsigned long long vid : 1;
    unsigned long long reserve0 : 37;
};

struct espe_qos_mac_entry_in_sram {
    struct espe_qos_mac_entry_low_in_sram ent_low;
    struct espe_qos_mac_entry_high_in_sram ent_high;
};

#endif

int espe_mac_entry_table_init(struct spe *spe);
void espe_mac_entry_table_exit(struct spe *spe);

#ifdef CONFIG_BALONG_ESPE_FW
void espe_mac_fw_add(const unsigned char *mac, unsigned short vid, unsigned int br_speno, unsigned int src_speno,
                     unsigned int is_static);
void espe_mac_fw_del(const unsigned char *mac, unsigned short vid, unsigned int br_speno, unsigned int src_speno);
int espe_mac_entry_config(enum spe_tab_ctrl_type type, const unsigned int *buf, unsigned int size);
int espe_mac_filt_add(struct espe_mac_filt_entry *ent);
int espe_mac_filt_del(struct espe_mac_filt_entry *ent);
void espe_macfw_entry_dump(void);
void espe_macfl_entry_dump(void);
#ifdef SPEV300_SUPPORT
void espe_qos_mac_en(void);
int espe_qos_mac_lmt_add(struct espe_qos_mac_entry *ent);
int espe_qos_mac_lmt_del(struct espe_qos_mac_entry *ent);
#endif
#else
static inline void espe_mac_fw_add(const unsigned char *mac, unsigned short vid, unsigned int br_speno,
                                   unsigned int src_speno, unsigned int is_static)
{
    return;
};
static inline void espe_mac_fw_del(const unsigned char *mac, unsigned short vid, unsigned int br_speno,
                                   unsigned int src_speno)
{
    return;
}
static inline int espe_mac_entry_config(enum spe_tab_ctrl_type type, const unsigned int *buf, unsigned int size)
{
    return 0;
}
static inline int espe_mac_filt_add(struct espe_mac_filt_entry *ent)
{
    return 0;
}
static inline int espe_mac_filt_del(struct espe_mac_filt_entry *ent)
{
    return 0;
}
static inline void espe_macfw_entry_dump(void)
{
    return;
}
static inline void espe_macfl_entry_dump(void)
{
    return;
}

#endif
#endif /* __SPE_MAC_ENTRY_H__ */
