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

#ifndef __DIAG_SERVICE_H__
#define __DIAG_SERVICE_H__

#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <product_config.h>
#include <mdrv.h>
#include <osl_types.h>
#include "bsp_diag.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push) // 保存对齐状态
#pragma pack(4)    // 强制4字节对齐

#ifdef DIAG_SYSTEM_5G
#define SOCP_CODER_SRC_PS_IND SOCP_CODER_SRC_ACPU_IND
#define SOCP_CODER_SRC_CNF SOCP_CODER_SRC_ACPU_CNF
#else
#define SOCP_CODER_SRC_PS_IND SOCP_CODER_SRC_LOM_IND1
#define SOCP_CODER_SRC_CNF SOCP_CODER_SRC_LOM_CNF1
#endif

#define DIAG_SRV_SET_MODEM_ID(frame, modem_id) ((frame)->srv_head.mdmid_3b = (modem_id))
#define DIAG_SRV_SET_TRANS_ID(frame, trans_id) ((frame)->srv_head.msg_trans_id = (trans_id))
#define DIAG_SRV_SET_COMMAND_ID(frame, pri, mode, sec, cmd_id) \
    do {                                                       \
        (frame)->cmdid_stru.pri_4b = pri;                      \
        (frame)->cmdid_stru.mode_4b = mode;                    \
        (frame)->cmdid_stru.sec_5b = sec;                      \
        (frame)->cmdid_stru.cmdid_19b = cmd_id;                \
    } while (0)
#define DIAG_SRV_SET_COMMAND_ID_EX(frame, cmd_id) ((frame)->cmd_id = (cmd_id))
#define DIAG_SRV_SET_MSG_LEN(frame, len)                                \
    do {                                                                \
        (frame)->msg_len_stru.msg_len = (u16)((len)&0xffff);            \
        (frame)->msg_len_stru.resv = 0;                                 \
        (frame)->msg_len_stru.msg_ver = (u8)(((len)&0xff000000) >> 24); \
    } while (0)
#define DIAG_SRV_GET_REAL_LEN(len) ((len)&0xffff)

#ifdef DIAG_SYSTEM_5G
#define SERVICE_HEAD_SID(data) ((u32)(((diag_service_head_s *)(data))->sid_4b))
#else
#define SERVICE_HEAD_SID(data) ((u32)(((diag_service_head_s *)(data))->sid_8b))
#endif

#define SERVICE_MSG_PROC 0x1000

typedef struct {
    unsigned int ind_chan_id; /* 编码源通道ID，固定配置 */
    unsigned int cnf_chan_id; /* 编码源通道ID，固定配置 */
    struct wakeup_source stWakelock;
} diag_srv_ctrl_s;

void diag_srv_fill_head(diag_srv_head_s *srv_head);
void diag_srv_pkt_timeout_clear(void);
u32 diag_srv_create_pkt(diag_frame_head_s *frame);
diag_frame_head_s *diag_srv_save_pkt(diag_frame_head_s *frame, u32 len);
void diag_srv_destroy_pkt(diag_frame_head_s *frame);
u32 diag_srv_proc(void *data, u32 len);
void diag_srv_proc_func_reg(diag_srv_proc_cb srv_fn);
void diag_srv_init(void);
void diag_srv_write_data(socp_buffer_rw_s *rw_buffer, const void *payload, u32 len);
u32 diag_srv_ind_send(diag_msg_report_head_s *msg);
u32 diag_srv_pack_ind_data(diag_msg_report_head_s *msg);
u32 diag_srv_cnf_send(diag_msg_report_head_s *msg);
u32 diag_srv_pack_cnf_data(diag_msg_report_head_s *msg);
void diag_debug_dfr(void);
u32 diag_srv_pack_reset_data(diag_msg_report_head_s *msg);
u32 diag_srv_get_data(diag_frame_head_s *rcv_head, u32 len, diag_frame_head_s **new_head);
#ifdef DIAG_SYSTEM_FUSION
s32 diag_srv_msg_send(void *msg_data);
#endif

#pragma pack(pop) // 恢复对齐方式

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
