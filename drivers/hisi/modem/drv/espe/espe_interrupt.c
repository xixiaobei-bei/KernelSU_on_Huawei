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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include "bsp_dt.h"
#include <bsp_trans_report.h>

#include "espe.h"
#include "espe_dbg.h"
#include "espe_desc.h"
#include "espe_port.h"
#include "espe_interrupt.h"
#include <bsp_slice.h>

#define ESPE_SMP_BIT_SCHED 0
#define ESPE_SMP_BIT_SCHED_REQ 1

static void espe_intr_bh_workqueue(struct work_struct *work)
{
    struct spe *spe = &g_espe;
    struct spe_port_ctrl *ctrl = NULL;
    unsigned int port_num;

    spe->stat.wq_task_begin_time = bsp_get_slice_value();
    for (port_num = 0; port_num < SPE_PORT_NUM; port_num++) {
        ctrl = &spe->ports[port_num].ctrl;

        if (ctrl->port_flags.enable && (port_num != spe->ipfport.portno)) {
            espe_process_rd_desc(port_num);
        }
    }
    spe->stat.wq_task_end_time = bsp_get_slice_value();
    return;
}

void espe_process_desc_workqueue(void *spe_ctx, unsigned int evt_rd_done, unsigned int evt_td_done)
{
    struct spe *spe = (struct spe *)spe_ctx;
    unsigned int port_num;
    struct spe_port_ctrl *ctrl = NULL;

    for (port_num = 0; port_num < SPE_PORT_NUM; port_num++) {
        ctrl = &spe->ports[port_num].ctrl;

        if (ctrl->port_flags.enable) {
            if ((0x1 & (evt_td_done >> port_num)) && (port_num != spe->ipfport.portno)) {
                spe->stat.evt_td_complt[port_num]++;
                espe_process_td_desc(port_num);
            }

            if (0x1 & (evt_rd_done >> port_num)) {
                spe->stat.evt_rd_complt[port_num]++;
            }
        }
    }

    queue_work(spe->smp.workqueue, &spe->smp.work);
}

void espe_init_workqueue(struct spe *spe)
{
    INIT_WORK(&spe->smp.work, espe_intr_bh_workqueue);
    spe->smp.workqueue = alloc_ordered_workqueue("process_desc", 0);
    if (spe->smp.workqueue == NULL) {
        SPE_ERR("espe port workqueue alloc fail\n");
    }

    SPE_ERR("espe port workqueue allocd\n");
    return;
}

static irqreturn_t espe_interrupt(int irq, void *dev_id)
{
    struct spe *spe = (struct spe *)dev_id;
    unsigned int evt_rd_full;
    unsigned int evt_rd_done;
    unsigned int evt_td_done;
    unsigned int evt_ad_empty;
    unsigned int port_num;

    if (unlikely(spe->dbg_level & SPE_DBG_HIDS_UPLOAD)) {
        bsp_trans_report_set_time(); /* set trans report timestamp */
    }

    evt_rd_full = spe_readl_relaxed(spe->regs, SPE_RDQ_FULL_INTA);
    evt_rd_done = spe_readl_relaxed(spe->regs, SPE_RD_DONE_INTA);
    evt_td_done = spe_readl_relaxed(spe->regs, SPE_TD_DONE_INTA);
    evt_ad_empty = spe_readl_relaxed(spe->regs, SPE_ADQ_EMPTY_INTA);

    spe_writel_relaxed(spe->regs, SPE_RDQ_FULL_INTA_STATUS, evt_rd_full);
    spe_writel_relaxed(spe->regs, SPE_RD_DONE_INTA_STATUS, evt_rd_done);
    spe_writel_relaxed(spe->regs, SPE_TD_DONE_INTA_STATUS, evt_td_done);
    spe_writel_relaxed(spe->regs, SPE_ADQ_EMPTY_INTA_STATUS, evt_ad_empty);

    /*
     * spev200,if td fail, do not produce rd desc.need free mem at td desc.
     *         for ipf port, this operation is completed before pushing the pointer.
     * spev300, if ipf port td fail, produce rd desc.need free mem at cpu rd desc.
     *          if other port,td fail, do not produce rd desc.need free mem at other port desc.
     */
    if (spe->spe_version == ESPE_VER_V200 && spe->ipfport.alloced) {
        espe_process_td_desc(spe->ipfport.portno);
    }

    if (evt_rd_done || evt_td_done) {
        spe->stat.int_time = bsp_get_slice_value();
        spe->smp.process_desc((void *)spe, evt_rd_done, evt_td_done);
    }

    if (unlikely(evt_rd_full)) {
        for (port_num = 0; port_num < SPE_PORT_NUM; port_num++) {
            if (unlikely(0x1 & (evt_rd_full >> port_num))) {
                spe->stat.evt_rd_full[port_num]++;
            }
        }
    }

    if (unlikely(evt_ad_empty)) {
        if (0x1 & evt_ad_empty) {
            spe->stat.evt_ad_empty[SPE_ADQ0]++;
        }
        if (0x2 & evt_ad_empty) {
            spe->stat.evt_ad_empty[SPE_ADQ1]++;
        }
    }

#ifndef CONFIG_ESPE_PHONE_SOC
    if (spe->spe_version == ESPE_VER_V200 || spe->spe_version == ESPE_VER_V300) {
        espe_update_adq_wptr(spe);
    }
#endif

    return IRQ_HANDLED;
}

int espe_interrput_init(struct spe *spe)
{
    int ret;

    spe_writel_relaxed(spe->regs, SPE_RDQ_FULL_INTA_MASK, SPE_EVENT_ENABLE);
    spe_writel_relaxed(spe->regs, SPE_RD_DONE_INTA_MASK, SPE_EVENT_ENABLE);
    spe_writel_relaxed(spe->regs, SPE_TD_DONE_INTA_MASK, SPE_EVENT_ENABLE);
    spe_writel_relaxed(spe->regs, SPE_ADQ_EMPTY_INTA_MASK, SPE_EVENT_ENABLE);
    spe_writel_relaxed(spe->regs, SPE_BUS_ERR_INTA_MASK, SPE_EVENT_ENABLE);
    spe_writel_relaxed(spe->regs, SPE_INTA_INTERVAL, SPE_INTR_DEF_INTEVAL);

    // set to 1k as default
    spe_writel_relaxed(spe->regs, SPE_RDQ_FULL_LEVEL, SPE_EVT_RD_FULL);
    spe_writel(spe->regs, SPE_ADQ_EMPTY_LEVEL, SPE_EVT_AD_EMPTY);

    spe->smp.process_desc = espe_process_desc_workqueue;
    espe_init_workqueue(spe);

    ret = request_irq(spe->irq, espe_interrupt, IRQF_SHARED, "eSPE", spe);

    return ret;
}
