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

#ifndef __RFILE_SERVER_DUMP_H__
#define __RFILE_SERVER_DUMP_H__

#include "rfile_server.h"
#include <osl_types.h>

#define RFILE_MNTN_INNER_STATE_CNT 20
#define RFILE_MNTN_EVENT_STATE_CNT 20
#define RFILE_MNTN_FD_CNT 10
#define RFILE_MNTN_HANDLE_CNT 80
#define RFILE_MNTN_PATHLEN_MAX 64

struct rfile_state_trace {
    u32 slice;
    u32 state_type;
};
struct rfile_inner_state_dbg {
    struct rfile_state_trace trace[RFILE_MNTN_INNER_STATE_CNT];
    u32 trace_index;
};

struct rfile_event_trace {
    u32 slice;
    u32 event;
};

struct rfile_event_dbg {
    struct rfile_event_trace trace[RFILE_MNTN_EVENT_STATE_CNT];
    u32 trace_index;
};

enum rfile_acore_handle_state {
    RFILE_ACORE_ICC_RECV = 0,
    RFILE_ACORE_USER_SEND,
    RFILE_ACORE_USER_RECV,
    RFILE_ACORE_ICC_SEND
};

struct rfile_handle_trace {
    u32 slice;
    u32 op_type;
    u32 op_num;
    u32 cmd_cnt;
    u32 state_type;
    s32 ret;
};

struct rfile_acore_handle_dbg {
    struct rfile_handle_trace trace[RFILE_MNTN_HANDLE_CNT];
    u32 trace_index;
};


struct rfile_fd_dump {
    u32 fd;
    u32 offset;
    char path[RFILE_MNTN_PATHLEN_MAX];
};

struct rfile_fd_dbg {
    struct rfile_fd_dump debug[RFILE_MNTN_FD_CNT];
};

struct rfile_dbg_info {
    u64 dbg_start_byte;
    struct rfile_inner_state_dbg inner_state_trace;
    struct rfile_acore_handle_dbg acore_handle_trace;
    struct rfile_event_dbg event_trace;
    struct rfile_fd_dbg fd_dump;
};
void rfile_dump_init(void);
void rfile_dump_inner_state_dbg(enum rfile_acore_state state_type);
void rfile_dump_handle_dbg(u32 op_type, u32 op_num, u32 cmd_cnt, u32 state_type, s32 ret);
void rfile_dump_event_dbg(enum rfile_acore_event event);

#endif /* end of rfile_server_dump.h */
