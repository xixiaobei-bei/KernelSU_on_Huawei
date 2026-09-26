/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include "bsp_hds_ind.h"
#include <mdrv_diag.h>
#include <osl_types.h>
#include "bsp_trans_report.h"

struct wan_trans_s {
    int transreport_error;
    int get_dbg_info_error;
    struct timer_list trans_timer;
    struct list_head list;
    unsigned int init;
};

unsigned int g_a_hidslog_freq = 200; /* ms unit */
#define HDS_TRANS_RE_SUCC (0x00abcd00 + 0x5)

struct wan_trans_s g_wan_trans;
#define WAN_SET_TRANS_TIMEOUT() (jiffies + msecs_to_jiffies(g_a_hidslog_freq))

void wan_transreport_init_timer(void);

void bsp_set_netlog_freq(unsigned int timeout)
{
    g_a_hidslog_freq = timeout;
}

int bsp_trans_report_register(struct trans_report_info *info)
{
    wan_transreport_init_timer();
    if (info->data == NULL || info->size == 0 || info->func_cb == NULL) {
        return -EINVAL;
    }
    list_add_tail(&info->list, &g_wan_trans.list);
    return 0;
}

void trans_report_cycle(unsigned long data)
{
    struct wan_trans_s *wan_trans = (struct wan_trans_s *)((uintptr_t)data);
    struct trans_diag_info sTransInd = {0};
    struct trans_report_info *info = NULL;
    struct trans_report_info *tmp = NULL;
    int ret;

    sTransInd.ulModule = 0xf0900;
    sTransInd.ulPid = 0x8003;
    list_for_each_entry_safe(info, tmp, &wan_trans->list, list) {
        if (info->func_cb != NULL) {
            info->func_cb();

            sTransInd.ulMsgId = info->ul_msg_id;
            sTransInd.ulLength = info->size;
            sTransInd.pData = (info->data);
            ret = bsp_diag_trans_report(&sTransInd);
            if (ret != HDS_TRANS_RE_SUCC) {
                wan_trans->transreport_error++;
            } else {
                wan_trans->get_dbg_info_error++;
            }
        }
    }

    mod_timer(&wan_trans->trans_timer, WAN_SET_TRANS_TIMEOUT());
}

void wan_transreport_init_timer(void)
{
    /* init timer */
    struct wan_trans_s *wan_trans = &g_wan_trans;

    if (wan_trans->init != 1) {
        init_timer(&wan_trans->trans_timer);
        wan_trans->trans_timer.function = trans_report_cycle;
        wan_trans->trans_timer.data = (uintptr_t)wan_trans;
        wan_trans->trans_timer.expires = WAN_SET_TRANS_TIMEOUT();
        INIT_LIST_HEAD(&wan_trans->list);
        add_timer(&wan_trans->trans_timer);
        wan_trans->init = 1;
    }
}
