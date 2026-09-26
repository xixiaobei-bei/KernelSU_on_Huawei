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
#include "rfile_server.h"
#include "rfile_server_dump.h"
#include <linux/list.h>
#include <linux/slab.h>

#include <bsp_slice.h>
#include <bsp_dump.h>
#include <bsp_print.h>
#include <osl_sem.h>
#include <securec.h>


#define THIS_MODU mod_rfile

#define RFILE_OM_DUMP_SIZE 0xC00
#define RFILE_OM_DUMP_ID DUMP_MODEMAP_RFILE
struct rfile_dbg_info *g_rfile_dbg;

void rfile_dump_inner_state_dbg(enum rfile_acore_state state_type)
{
    struct rfile_inner_state_dbg *debug = NULL;
    struct rfile_state_trace *trace = NULL;
    u32 timestamp = bsp_get_slice_value();
    int i;

    if (g_rfile_dbg == NULL) {
        bsp_err("<%s> g_rfile_dbg is NULL!\n", __FUNCTION__);
        return;
    }
    debug = &(g_rfile_dbg->inner_state_trace);
    i = debug->trace_index % RFILE_MNTN_INNER_STATE_CNT;
    trace = &(debug->trace[i]);
    trace->slice = timestamp;
    trace->state_type = state_type;
    debug->trace_index++;
    return;
}

void rfile_dump_handle_dbg(u32 op_type, u32 op_num, u32 cmd_cnt, u32 state_type, s32 ret)
{
    struct rfile_acore_handle_dbg *debug = NULL;
    struct rfile_handle_trace *trace = NULL;
    u32 timestamp = bsp_get_slice_value();
    int i;

    if (g_rfile_dbg == NULL) {
        bsp_err("<%s> g_rfile_dbg is NULL!\n", __FUNCTION__);
        return;
    }
    debug = &(g_rfile_dbg->acore_handle_trace);
    i = debug->trace_index % RFILE_MNTN_HANDLE_CNT;
    trace = &(debug->trace[i]);
    trace->slice = timestamp;
    trace->op_type = op_type;
    trace->op_num = op_num;
    trace->cmd_cnt = cmd_cnt;
    trace->state_type = state_type;
    trace->ret = ret;
    debug->trace_index++;
    return;
}

void rfile_dump_event_dbg(enum rfile_acore_event event)
{
    struct rfile_event_dbg *debug = NULL;
    struct rfile_event_trace *trace = NULL;
    u32 timestamp = bsp_get_slice_value();
    int i;

    if (g_rfile_dbg == NULL) {
        bsp_err("<%s> g_rfile_dbg is NULL!\n", __FUNCTION__);
        return;
    }
    debug = &(g_rfile_dbg->event_trace);
    i = debug->trace_index % RFILE_MNTN_EVENT_STATE_CNT;
    trace = &(debug->trace[i]);
    trace->slice = timestamp;
    trace->event = event;
    debug->trace_index++;
    return;
}

void rfile_dump_fd_list(void)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct rfile_fd_list *elemt = NULL;
    int i = 0;
    int ret;
    struct rfile_main_stru_acore *rfile_main = rfile_get_main_info();

    if (g_rfile_dbg == NULL || rfile_main == NULL) {
        bsp_err("<%s> g_rfile_dbg or rfile_main is NULL!\n", __FUNCTION__);
        return;
    }
    osl_sem_down(&rfile_main->sem_fd_list);
    list_for_each_safe(p, n, &rfile_main->fd_list)
    {
        elemt = list_entry(p, struct rfile_fd_list, stlist);
        if (i >= RFILE_MNTN_FD_CNT) {
            bsp_err("<%s> elements in fd_list > 20,dump can only save 20\n", __FUNCTION__);
            break;
        }
        g_rfile_dbg->fd_dump.debug[i].fd = elemt->fd;
        g_rfile_dbg->fd_dump.debug[i].offset = elemt->offset;
        g_rfile_dbg->fd_dump.debug[i].path[0] = '\0';
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
        if (strlen(elemt->path) != 0) {
#else
        if (elemt->path != NULL) {
#endif
            ret = strncat_s(g_rfile_dbg->fd_dump.debug[i].path, RFILE_MNTN_PATHLEN_MAX, elemt->path,
                RFILE_MNTN_PATHLEN_MAX - 1);
            if (ret) {
                bsp_err("<%s> strncat_s err. ret = %d.\n", __FUNCTION__, ret);
            }
        }
        i++;
    }
    osl_sem_up(&rfile_main->sem_fd_list);
    return;
}

void rfile_dump_hook(void)
{
    rfile_dump_fd_list();
    return;
}

void rfile_dump_init(void)
{
    u32 rfile_dbg_size = sizeof(struct rfile_dbg_info);

    g_rfile_dbg =
        (struct rfile_dbg_info *)bsp_dump_register_field(RFILE_OM_DUMP_ID, "RFILE_ACORE", RFILE_OM_DUMP_SIZE, 0x2);
    if (g_rfile_dbg == NULL) {
        bsp_err("<%s> malloc from dump fail,malloc form ddr!", __FUNCTION__);
        g_rfile_dbg = (struct rfile_dbg_info *)kmalloc(rfile_dbg_size, GFP_KERNEL);
        if (g_rfile_dbg == NULL) {
            bsp_err("<%s> malloc from ddr fail,error!", __FUNCTION__);
            return;
        }
    }
    (void)memset_s(g_rfile_dbg, sizeof(struct rfile_dbg_info), 0, sizeof(struct rfile_dbg_info));

    (void)bsp_dump_register_hook("RFILE", rfile_dump_hook);

    g_rfile_dbg->dbg_start_byte = 0x23232323; // special start byte for rfile dump index

    rfile_dump_inner_state_dbg(RFILE_STATE_UNINITED);
    return;
}

s32 rfile_dump_get_fd_list(void)
{
    struct list_head *p = NULL;
    struct list_head *n = NULL;
    struct rfile_fd_list *elemt = NULL;
    int empty_flag = 1;
    struct rfile_main_stru_acore *rfile_main = rfile_get_main_info();

    if (rfile_main == NULL) {
        bsp_err("<%s> rfile_main is NULL!\n", __FUNCTION__);
        return -1;
    }

    list_for_each_safe(p, n, &rfile_main->fd_list)
    {
        empty_flag = 0;
        elemt = list_entry(p, struct rfile_fd_list, stlist);
        if (elemt->open_type == RFILE_OPEN_FILE) {
            if (elemt->fd_outdate_flag) {
                bsp_err("<%s> opened file %s, flag 0x%x, mode 0x%x, fd_org %d, fd_new %d, offset %d.\n",
                    __FUNCTION__, elemt->path, elemt->flags, elemt->mode, elemt->fd, elemt->fd_new, elemt->offset);
            } else {
                bsp_err("<%s> opened file %s, flag 0x%x, mode 0x%x, fd %d, offset %d.\n",
                    __FUNCTION__, elemt->path, elemt->flags, elemt->mode, elemt->fd, elemt->offset);
            }
        } else {
            if (elemt->fd_outdate_flag) {
                bsp_err("<%s> opened dir %s, fd_org %d, fd_new %d.\n", __FUNCTION__, elemt->path, elemt->fd,
                    elemt->fd_new);
            } else {
                bsp_err("<%s> opened dir %s, fd %d.\n", __FUNCTION__, elemt->path, elemt->fd);
            }
        }
    }
    if (empty_flag) {
        bsp_err("<%s> fd_list is empty!\n", __FUNCTION__);
    }
    return 0;
}

void rfile_dump_innerstate_show(void)
{
    u32 i, j, index_now;

    char *innerstate_info[] = {
        "IDLE", "DOING", "RESET", "UNINITED", "APP_RECOVERING"
    };
    index_now = g_rfile_dbg->inner_state_trace.trace_index % RFILE_MNTN_INNER_STATE_CNT;
    bsp_err("index now = %d\n", index_now);
    for (i = index_now, j = 0; i < RFILE_MNTN_INNER_STATE_CNT; i++, j++) {
        bsp_err("No.%d  timestamp:0x%x  state:%s\n", j, g_rfile_dbg->inner_state_trace.trace[i].slice,
            innerstate_info[g_rfile_dbg->inner_state_trace.trace[i].state_type]);
    }
    for (i = 0; i < index_now; i++, j++) {
        bsp_err("No.%d  timestamp:0x%x  state:%s\n", j, g_rfile_dbg->inner_state_trace.trace[i].slice,
            innerstate_info[g_rfile_dbg->inner_state_trace.trace[i].state_type]);
    }
    return;
}

void rfile_dump_ophandle_show(void)
{
    u32 i, j, index_now;

    char *handle_info[] = {
        "ICC_READ", "USER_SEND", "USER_RECV", "ICC_SEND"
    };
    char *type_info[] = {
        "open",   "close",  "write", "write_sync",   "read",           "seek",     "tell",
        "remove", "mkdir",  "rmdir", "opendir",      "readdir",        "closedir", "stat",
        "access", "rename", "check_load_mode", "reset", "open_recover"
    };

    index_now = g_rfile_dbg->acore_handle_trace.trace_index % RFILE_MNTN_HANDLE_CNT;
    bsp_err("index now = %d\n", index_now);
    for (i = index_now, j = 0; i < RFILE_MNTN_HANDLE_CNT; i++, j++) {
        bsp_err("No.%d  timestamp:0x%x  op:%s  op_num:%d  cmd_cnt:%d  state:%s  ret:%d\n", j,
            g_rfile_dbg->acore_handle_trace.trace[i].slice, type_info[g_rfile_dbg->acore_handle_trace.trace[i].op_type],
            g_rfile_dbg->acore_handle_trace.trace[i].op_num, g_rfile_dbg->acore_handle_trace.trace[i].cmd_cnt,
            handle_info[g_rfile_dbg->acore_handle_trace.trace[i].state_type],
            g_rfile_dbg->acore_handle_trace.trace[i].ret);
    }
    for (i = 0; i < index_now; i++, j++) {
        bsp_err("No.%d  timestamp:0x%x  op:%s  op_num:%d  cmd_cnt:%d  state:%s  ret:%d\n", j,
            g_rfile_dbg->acore_handle_trace.trace[i].slice, type_info[g_rfile_dbg->acore_handle_trace.trace[i].op_type],
            g_rfile_dbg->acore_handle_trace.trace[i].op_num, g_rfile_dbg->acore_handle_trace.trace[i].cmd_cnt,
            handle_info[g_rfile_dbg->acore_handle_trace.trace[i].state_type],
            g_rfile_dbg->acore_handle_trace.trace[i].ret);
    }
    return;
}

void rfile_dump_event_show(void)
{
    u32 i, j, index_now;

    char *event[] = {
        "ICC_CB", "RESET_BEFORE", "RESETTING", "SHUTDOWN", "APP_KILLED", "ICC_FULL"
    };

    index_now = g_rfile_dbg->event_trace.trace_index % RFILE_MNTN_EVENT_STATE_CNT;
    bsp_err("index now = %d\n", index_now);
    for (i = index_now, j = 0; i < RFILE_MNTN_EVENT_STATE_CNT; i++, j++) {
        bsp_err("No.%d  timestamp:0x%x  event:%s \n", j, g_rfile_dbg->event_trace.trace[i].slice,
            event[g_rfile_dbg->event_trace.trace[i].event]);
    }
    for (i = 0; i < index_now; i++, j++) {
        bsp_err("No.%d  timestamp:0x%x  event:%s \n", j, g_rfile_dbg->event_trace.trace[i].slice,
            event[g_rfile_dbg->event_trace.trace[i].event]);
    }
    return;
}

int rfile_dump_show(void)
{
    bsp_err("*************rfile fd dump*************\n");
    rfile_dump_get_fd_list();

    bsp_err("*************rfile innerstate dump*************\n");
    rfile_dump_innerstate_show();

    bsp_err("*************rfile ophandle dump*************\n");
    rfile_dump_ophandle_show();

    bsp_err("*************rfile event dump*************\n");
    rfile_dump_event_show();
    return 0;
}
