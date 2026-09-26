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
 */

#include <securec.h>
#include "nv_msg.h"
#include "nv_comm.h"
#include "nv_debug.h"

#define THIS_MODU mod_nv

struct nv_global_msg_info_stru g_msg_ctrl;

nv_cmd_req *nv_get_free_req(void)
{
    nv_cmd_req *cmd_req_ptr = NULL;

    cmd_req_ptr = nv_malloc(sizeof(nv_cmd_req));
    if (cmd_req_ptr == NULL) {
        nv_printf("malloc fail\n");
        return NULL;
    }

    if (memset_s((void *)cmd_req_ptr, sizeof(nv_cmd_req), 0, sizeof(nv_cmd_req))) {
        nv_printf("memset err");
    }

    return cmd_req_ptr;
}

nv_cmd_req *nv_get_cmd_req(void)
{
    unsigned long flags;
    nv_cmd_req *cmd_req_ptr = NULL;
    msg_queue_t *msg_queue_ptr = NULL;

    cmd_req_ptr = NULL;
    msg_queue_ptr = &g_msg_ctrl.high_task_list;

    /* remove from current nv high/low cmd request list */
    spin_lock_irqsave(&msg_queue_ptr->lock, flags);
    if (!list_empty(&msg_queue_ptr->list)) {
        /*lint -save -e826*/ /* Info 826: (Info -- Suspicious pointer-to-pointer conversion (area too small)) */
        cmd_req_ptr = list_first_entry(&msg_queue_ptr->list, nv_cmd_req, stList);
        /*lint -restore*/
        list_del(&cmd_req_ptr->stList);
    }
    spin_unlock_irqrestore(&msg_queue_ptr->lock, flags);

    if (cmd_req_ptr != NULL)
        return cmd_req_ptr;

    msg_queue_ptr = &g_msg_ctrl.low_task_list;
    spin_lock_irqsave(&msg_queue_ptr->lock, flags);
    if (!list_empty(&msg_queue_ptr->list)) {
        /*lint -save -e826*/ /* Info 826: (Info -- Suspicious pointer-to-pointer conversion (area too small)) */
        cmd_req_ptr = list_first_entry(&msg_queue_ptr->list, nv_cmd_req, stList);
        /*lint -restore*/
        list_del(&cmd_req_ptr->stList);
    }
    spin_unlock_irqrestore(&msg_queue_ptr->lock, flags);

    return cmd_req_ptr;
}

int nv_put_cmd_req(nv_cmd_req *cmd_req_ptr)
{
    if (cmd_req_ptr == NULL) {
        nv_printf("illegal function parameter \n");
        return BSP_ERR_NV_FREE_BUF_FAIL;
    }

    nv_free(cmd_req_ptr);
    return 0;
}

u32 nv_send_rmsg(u32 msg_type, u32 req_sn)
{
    nv_cmd_req *cmd_req_ptr = NULL;
    msg_queue_t *msg_queue_ptr = NULL;
    unsigned long flags;
    u8 type, priority;
    nv_cb_func func = NULL;
    struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info();

    switch (msg_type) {
        case NV_ICC_REQ_FLUSH:
            type = NV_TASK_MSG_FLUSH;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_FLUSH_RWNV:
            type = NV_TASK_MSG_FLUSH_RWFILE;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_FLUSH_RDWR_ASYNC:
            type = NV_TASK_MSG_FLUSH;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = NULL;
            msg_queue_ptr = &g_msg_ctrl.low_task_list;
            break;

        case NV_ICC_REQ_LOAD_CARRIER_CUST:
            type = NV_TASK_MSG_LOAD_CARRIER;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_LOAD_CARRIER_RESUM:
            type = NV_TASK_MSG_LOAD_CARRIER_COMM;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_LOAD_BACKUP:
            type = NV_TASK_MSG_LOAD_BACKUP;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_UPDATE_DEFAULT:
            type = NV_TASK_MSG_UPDATE_DEFAULT;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_UPDATE_BACKUP:
            type = NV_TASK_MSG_UPDATE_BACKUP;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_FACTORY_RESET:
            type = NV_TASK_MSG_FACTORY_RESET;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        case NV_ICC_REQ_UPGRADE_FLUSH:
            type = NV_TASK_MSG_UPGRADE_FLUSH;
            priority = HIGH_PRIORITY_MSG_QUEUE;
            func = bsp_nvm_ccore_msg_cb;
            msg_queue_ptr = &g_msg_ctrl.high_task_list;
            break;

        default:
            nv_printf("invalid nv icc msg type %d \n", msg_type);
            return BSP_ERR_NV_ICC_CHAN_ERR;
    };

    cmd_req_ptr = nv_get_free_req();
    if (cmd_req_ptr == NULL) {
        nv_printf("malloc fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    cmd_req_ptr->msg_type = type;
    cmd_req_ptr->priority = priority;
    cmd_req_ptr->sn = req_sn;
    cmd_req_ptr->nv_msg_callback = func;

    spin_lock_irqsave(&msg_queue_ptr->lock, flags);
    list_add_tail(&cmd_req_ptr->stList, &msg_queue_ptr->list);
    spin_unlock_irqrestore(&msg_queue_ptr->lock, flags);

    osl_sem_up(&(nv_ctrl->task_sem));
    g_msg_ctrl.remote_wr_count++;
    return NV_OK;
}

u32 nv_msg_init(void)
{
    if (memset_s((void *)&g_msg_ctrl, sizeof(g_msg_ctrl), 0, sizeof(struct nv_global_msg_info_stru))) {
        nv_printf("memset err\n");
    }
    g_msg_ctrl.req_sn = 0;

    spin_lock_init(&g_msg_ctrl.high_task_list.lock);
    INIT_LIST_HEAD(&g_msg_ctrl.high_task_list.list);

    spin_lock_init(&g_msg_ctrl.low_task_list.lock);
    INIT_LIST_HEAD(&g_msg_ctrl.low_task_list.list);

    spin_lock_init(&g_msg_ctrl.req_list.lock);
    INIT_LIST_HEAD(&g_msg_ctrl.req_list.list);

    return NV_OK;
}

void nv_msg_dump(void)
{
    struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info();
    nv_printf("icc_cb_count %d \n", g_msg_ctrl.icc_cb_count);
    nv_printf("icc_cb_reply %d \n", g_msg_ctrl.icc_cb_reply);
    nv_printf("sync_wr_count %d \n", g_msg_ctrl.sync_wr_count);
    nv_printf("sync_wr_done_count %d \n", g_msg_ctrl.sync_wr_done_count);
    nv_printf("async_wr_count %d \n", g_msg_ctrl.async_wr_count);
    nv_printf("wr_timeout_count %d \n", g_msg_ctrl.wr_timeout_count);
    nv_printf("remote_wr_count %d \n", g_msg_ctrl.remote_wr_count);
    nv_printf("req_sn %d \n", g_msg_ctrl.req_sn);
    nv_printf("task_proc_count %d \n", nv_ctrl->task_proc_count);
    nv_printf("wake_lock_active status %d \n", nv_ctrl->wake_lock.active);
}

EXPORT_SYMBOL(nv_msg_dump);
