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

#include <osl_types.h>
#include <bsp_print.h>
#include <securec.h>
#include "hmi_chan_adp.h"
#ifdef CONFIG_HMI_ICC
#include "hmi_icc.h"
#endif
#include "hmi_common.h"


struct hmi_chan_adp g_hmi_chan_adp_ctx;

struct hmi_chan_adp *hmi_get_chan_adp_ctx(void)
{
    return &g_hmi_chan_adp_ctx;
}

int hmi_register_chan_adp_read_cb(hmi_chan_adp_read_cb_t cb)
{
    struct hmi_chan_adp *ctx = &g_hmi_chan_adp_ctx;

    if (cb == NULL) {
        ctx->stats.register_cb_null++;
        return -1;
    }

    ctx->read_cb = cb;

    return 0;
}

int hmi_chan_adp_send(unsigned char *data, int len)
{
    int ret;
    struct hmi_chan_adp *ctx = &g_hmi_chan_adp_ctx;

    ret = hmi_hal_send(data, len);
    if (ret != 0) {
        ctx->stats.hal_send_fail++;
        return -1;
    }

    ctx->stats.hal_send_succ++;

    return 0;
}

int hmi_chan_adp_read_cb(unsigned char *data, int len)
{
    int ret;
    struct hmi_chan_adp *ctx = &g_hmi_chan_adp_ctx;

    if (ctx->read_cb != NULL) {
        ret = ctx->read_cb(data, len);
        if (ret == 0) {
            ctx->stats.read_cb_called_succ++;
        } else {
            ctx->stats.read_cb_called_fail++;
        }
    } else {
        ctx->stats.read_cb_null++;
    }

    return 0;
}

int hmi_chan_adp_init(void)
{
    int ret;
    struct hmi_chan_adp *ctx = &g_hmi_chan_adp_ctx;

    ret = hmi_hal_init();
    if (ret != 0) {
        bsp_err("hmi hal init error\n");
        ctx->stats.hal_init_fail++;
        return -1;
    }

    ret = hmi_register_hal_read_cb(hmi_chan_adp_read_cb);
    if (ret != 0) {
        bsp_err("hmi_register_hal_read_cb error\n");
        ctx->stats.hal_register_cb_fail++;
        return -1;
    }

    return 0;
}

int hmi_get_chan_adp_om_length(void)
{
    int len;

    len = hmi_get_hal_om_len();
    if (len <= 0) {
        bsp_err("get hal om len err\n");
    }

    return len + sizeof(struct hmi_chan_adp_stat);
}

void hmi_report_chan_adp_info(unsigned char *data, int max_len)
{
    int ret;
    struct hmi_chan_adp *ctx = &g_hmi_chan_adp_ctx;
    int chan_adp_len = sizeof(struct hmi_chan_adp_stat);

    ret = memcpy_s(data, max_len, &ctx->stats, chan_adp_len);
    if (ret != EOK) {
        bsp_err("chan adp info cp err\n");
        return;
    }

    hmi_report_hal_info(data + chan_adp_len, max_len - chan_adp_len);
}