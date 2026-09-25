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

#ifndef __HISI_ADP_PCDEV__
#define __HISI_ADP_PCDEV__

#define PCIE_CDEV_COUNT     16

enum pcdev_port_id_e{
    pcdev_ctrl = 0,
    pcdev_ttyGS0,
    pcdev_c_shell,
    pcdev_4g_diag,
    pcdev_3g_diag,
    pcdev_mdm,
    pcdev_gps,
    pcdev_a_shell,
    pcdev_err,
    pcdev_voice,
    pcdev_skytone,
    pcdev_agent_nv,
    pcdev_agent_om,
    pcdev_agent_msg,
    pcdev_appvcom,
    pcdev_modem,
    pcdev_pmom,
    pcdev_reserve,
    pcdev_port_bottom
};

struct pcdev_port_hids {
    unsigned long tx_packets;
    unsigned long tx_packets_finish;
    unsigned long rx_packets;
    unsigned long get_buf_call;
    unsigned long ret_buf_call;
    unsigned long read_cb_call;
};

struct pcdev_hids_report {
    struct pcdev_port_hids pcdev_port_hids[PCIE_CDEV_COUNT];
};


void *bsp_pcdev_open(u32 dev_id);
s32 bsp_pcdev_close(void* handle);
s32 bsp_pcdev_write(void* handle, void *buf, u32 size);
s32 bsp_pcdev_read(void* handle, void *buf, u32 size);
s32 bsp_pcdev_ioctl(void* handle, u32 cmd, void *para);

void pcie_cdev_notify_enum_done(void);
void pcie_cdev_notify_disable(void);
int pcdev_hids_cb(struct pcdev_hids_report *report);

#endif
