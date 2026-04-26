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

#define RST_LEN 2
#define SPE_INTR_MIN_INTERVAL 0x1
#define SPE_GENERAL_REG_NUM 65

struct espe_queue_stash {
    // spe queue
    unsigned int spe_tdqx_baddr_l[SPE_PORT_NUM];
    unsigned int spe_tdqx_baddr_h[SPE_PORT_NUM];
    unsigned int spe_tdqx_len[SPE_PORT_NUM];
    unsigned int spe_rdqx_baddr_l[SPE_PORT_NUM];
    unsigned int spe_rdqx_baddr_h[SPE_PORT_NUM];
    unsigned int spe_rdqx_len[SPE_PORT_NUM];
    unsigned int spe_tdqx_pri[SPE_PORT_NUM];

    // spe ports
    unsigned int spe_portx_udp_lmtnum[SPE_PORT_NUM];
    unsigned int spe_portx_lmtbyte[SPE_PORT_NUM];
    unsigned int spe_portx_property[SPE_PORT_NUM];
    unsigned int spe_portx_mac_addr_l[SPE_PORT_NUM];
    unsigned int spe_protx_mac_addr_h[SPE_PORT_NUM];
    unsigned int spe_portx_ipv6_addr_1s[SPE_PORT_NUM];
    unsigned int spe_portx_ipv6_addr_2n[SPE_PORT_NUM];
    unsigned int spe_portx_ipv6_addr_3t[SPE_PORT_NUM];
    unsigned int spe_portx_ipv6_addr_4t[SPE_PORT_NUM];
    unsigned int spe_portx_ipv6_mask[SPE_PORT_NUM];
    unsigned int spe_portx_ipv4_addr[SPE_PORT_NUM];
    unsigned int spe_portx_ipv4_mask[SPE_PORT_NUM];
};

enum espe_stash {
    STASH_REG_ADDR = 0,
    STASH_REG_VALUE = 1,
    STASH_REG_BUTTON = 2,
};

void espe_reset_modem_init(void);
