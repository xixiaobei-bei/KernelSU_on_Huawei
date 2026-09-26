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

#ifndef __SPE_PORT_H__
#define __SPE_PORT_H__

#include "espe.h"

#define PORT_TD_EVT_GAP_DEF 4
#define PORT_RD_EVT_GAP_DEF 0

#define PORT_UDP_LIMIT_TIME_DEF 0
#define PORT_UDP_LIMIT_CNT_DEF 0
#define PORT_RATE_LIMIT_TIME_DEF 0
#define PORT_RATE_LIMIT_BYTE_DEF 0

#define SPE_INTR_DEF_INTEVAL (8)  // 1/4ms
#define SPE_INTR_MIN_INTERVAL 0x1
#define SPE_CPU_PORT_DEF_MAX_RATE 30000  // unit mbps; After testing,this value corresponds to 1.6Gbps

#define SPE_PER_INTERVAL_CALC(x) ((x)*spe->irq_interval * 250 / 3 / 32768)

#define SPE_EVENT_DISABLE 0x8000FFFF
#define SPE_EVENT_ENABLE 0x0000FFFF

#define SPE_EVT_AD_EMPTY (32)
#define SPE_EVT_RD_FULL (5)

#define SPE_DEF_PORT_PRI 0xff
#define SPE_DEF_USB_ENC_PKT_CNT 50
#define SPE_DEF_USB_MAX_SIZE (63 * 1024)
#define SPE_DEF_USB_UNPACK_MAX_SIZE (16 * 1024)
#define SPE_DEF_PACK_ALIGN 0
#define SPE_DEF_PACK_DIV 896

enum spe_attach_brg_type {
    spe_attach_brg_brif,
    spe_attach_brg_normal,
    spe_attach_brg_mix,
    spe_attach_brg_br,
    spe_attach_brg_bottom
};
enum spe_port_td_copy_cfg {
    SPE_PORT_TD_COPY_DISABLE,
    SPE_PORT_TD_COPY_ENABLE
};

enum spe_port_stick_cfg {
    SPE_PORT_STICK_DISABLE,
    SPE_PORT_STICK_ENABLE
};

#ifdef CONFIG_BALONG_ESPE_FW
int espe_alloc_br_port(void);
int espe_free_br_port(unsigned portno);
void espe_port_attach_brg_modify(int portno, enum spe_attach_brg_type attach_brg);
#endif

void espe_intr_set_interval(unsigned irq_interval);
void espe_set_bypass(unsigned int port_num, unsigned int bypass_dir, unsigned int en);
int espe_enable_port(int portno);
int espe_disable_port(struct spe *spe, int portno);
void espe_en_port_stick_mode(struct spe *spe, unsigned int bypassport);
void espe_ackpkt_identify(struct spe *spe, unsigned int portno);
void espe_port_init(struct spe *spe);

#endif /* __SPE_PORT_H__ */
