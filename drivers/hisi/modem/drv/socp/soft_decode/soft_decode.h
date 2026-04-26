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

#ifndef _SCM_SOFTDECODE_H_
#define _SCM_SOFTDECODE_H_

#include <osl_types.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <bsp_print.h>
#include "hdlc.h"
#include "ring_buffer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define RING_BUFFER_SIZE (36 * 1024) /* ring_buffer数据接收buffer大小 */
#define SEND_BUFFER_SIZE (36 * 1024) /* 软解码接收数据包大小 */
#define SOFT_DECODE_TASK_PRIORITY 76
#define SOFT_DECODE_TASK_STACK_SIZE 8192

#define soft_decode_crit(fmt, ...) bsp_err(KERN_ERR "[%s]:" fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__)
#define soft_decode_error(fmt, ...) \
    bsp_err(KERN_ERR "[%s]:<%s %d>" fmt, BSP_MOD(THIS_MODU), __FUNCTION__, __LINE__, ##__VA_ARGS__)

typedef struct {
    osl_sem_id sem_id;
    om_ring_s *ring_buffer;
    char *buffer;
} soft_decode_rcv_ctrl_s;

typedef struct {
    u32 data_len; /* 接收或发送数据长度 */
    u32 num;      /* 接收或发送数据次数 */
} scm_mantennace_info_s;

typedef struct {
    u32 sem_create_err;
    u32 sem_give_err;
    u32 ringbuff_create_err;
    u32 task_id_err;
    u32 buff_no_enough;
    u32 rbuff_flush;
    u32 rbuff_put_err;
} scm_soft_decode_rb_info_s;

typedef struct {
    scm_soft_decode_rb_info_s rb_info;
    scm_mantennace_info_s put_info;
    scm_mantennace_info_s get_info;
    scm_mantennace_info_s hdlc_decap_data;
    u32 frame_decap_data;
    u32 hdlc_init_err;
    u32 data_type_err;
    u32 cpm_reg_logic_rcv_suc;
} soft_decode_mntn_info_s;

u32 soft_decode_cfg_data_recv(u8 *buffer, u32 len);
u32 soft_decode_data_recv(const u8 *buffer, u32 len);
int soft_decode_init(void);
s32 soft_decode_recv_task(void *para);
u32 soft_decode_send_to_hdlc_dec(om_hdlc_s *hdlc_ctrl, const u8 *data, u32 len);
void soft_decode_data_dispatch(om_hdlc_s *hdlc_ctrl);
u32 soft_decode_hdlc_init(om_hdlc_s *hdlc_ctrl);
void soft_decode_info_show(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
