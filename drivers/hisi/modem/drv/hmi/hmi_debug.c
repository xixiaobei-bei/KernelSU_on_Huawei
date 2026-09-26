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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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
#include <bsp_print.h>
#include "hmi_core.h"
#include "hmi_chan_adp.h"
#include "hmi_icc.h"
#include "hmi_common.h"

void hmi_display_core_stat(void)
{
    struct hmi_core *ctx = hmi_get_core_ctx();

    bsp_err("recv_skb_null: %u\n", ctx->stat.recv_skb_null);
    bsp_err("recv_nlhdr_null: %u\n", ctx->stat.recv_nlhdr_null);
    bsp_err("invalid_msg_len: %u\n", ctx->stat.invalid_msg_len);
    bsp_err("reply_invalid_para: %u\n", ctx->stat.reply_invalid_para);
    bsp_err("recv_msg_succ: %u\n", ctx->stat.recv_msg_succ);
    bsp_err("msg_alloc_failed: %u\n", ctx->stat.msg_alloc_failed);
    bsp_err("msg_put_failed: %u\n", ctx->stat.msg_put_failed);
    bsp_err("payload_copy_failed: %u\n", ctx->stat.payload_copy_failed);
    bsp_err("reply_succ: %u\n", ctx->stat.reply_succ);
    bsp_err("data_parse_invalid_para: %u\n", ctx->stat.data_parse_invalid_para);
    bsp_err("data_parse_reply_msg_failed: %u\n", ctx->stat.data_parse_reply_msg_failed);
    bsp_err("data_parse_pkt_succ: %u\n", ctx->stat.data_parse_pkt_succ);
}

void hmi_display_chan_adp_stat(void)
{
    struct hmi_chan_adp *ctx = hmi_get_chan_adp_ctx();

    bsp_err("register_cb_null: %u\n", ctx->stats.register_cb_null);
    bsp_err("hal_send_fail: %u\n", ctx->stats.hal_send_fail);
    bsp_err("hal_send_succ: %u\n", ctx->stats.hal_send_succ);
    bsp_err("read_cb_called_fail: %u\n", ctx->stats.read_cb_called_fail);
    bsp_err("read_cb_called_succ: %u\n", ctx->stats.read_cb_called_succ);
    bsp_err("read_cb_null: %u\n", ctx->stats.read_cb_null);
    bsp_err("hal_init_fail: %u\n", ctx->stats.hal_init_fail);
    bsp_err("hal_register_cb_fail: %u\n", ctx->stats.hal_register_cb_fail);
}

void hmi_display_icc_stat(void)
{
    struct hmi_icc *ctx = hmi_get_icc_ctx();

    bsp_err("register_read_cb_null: %u\n", ctx->hmi_icc_stats.register_read_cb_null);
    bsp_err("send_data_fail: %u\n", ctx->hmi_icc_stats.send_data_fail);
    bsp_err("send_data_succ: %u\n", ctx->hmi_icc_stats.send_data_succ);
    bsp_err("read_cb_invalid_para: %u\n", ctx->hmi_icc_stats.read_cb_invalid_para);
    bsp_err("read_err: %u\n", ctx->hmi_icc_stats.read_err);
    bsp_err("read_cb_null: %u\n", ctx->hmi_icc_stats.read_cb_null);
    bsp_err("read_cb_called_fail: %u\n", ctx->hmi_icc_stats.read_cb_called_fail);
    bsp_err("read_succ: %u\n", ctx->hmi_icc_stats.read_succ);
    bsp_err("icc_event_register_fail: %u\n", ctx->hmi_icc_stats.icc_event_register_fail);
}

