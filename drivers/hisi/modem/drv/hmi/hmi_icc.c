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

#include <bsp_icc.h>
#include <bsp_print.h>
#include <osl_malloc.h>
#include <securec.h>
#include "hmi_icc.h"

#define HMI_ICC_CHANNEL_ID ICC_CHANNEL_ID_MAKEUP(ICC_CHN_HMI, 0)
#define HMI_ICC_CHANNEL_SIZE 8192U

struct hmi_icc g_hmi_icc_ctx;

struct hmi_icc *hmi_get_icc_ctx(void)
{
    return &g_hmi_icc_ctx;
}

int hmi_register_hal_read_cb(hmi_hal_read_cb_t cb)
{
    struct hmi_icc *ctx = &g_hmi_icc_ctx;

    if (cb == NULL) {
        ctx->hmi_icc_stats.register_read_cb_null++;
        return -1;
    }

    ctx->read_cb = cb;

    return 0;
}

int hmi_hal_send(unsigned char *data, int len)
{
    struct hmi_icc *ctx = &g_hmi_icc_ctx;
    int ret;

    ret = bsp_icc_send(ICC_CPU_MODEM, HMI_ICC_CHANNEL_ID, data, len);
    if (ret < 0) {
        ctx->hmi_icc_stats.send_data_fail++;
        bsp_err("icc send err: %d", ret);
        return ret;   /* caller decide to free buf or send again */
    }

    ctx->hmi_icc_stats.send_data_succ++;
    return 0;
}

static int hmi_icc_readcb(unsigned int id, unsigned int size, void *context)
{
    struct hmi_icc *ctx = NULL;
    int read_size;
    int ret;

    if (context == NULL) {
        bsp_err("hmi_icc_readcb read context null\n");
        return -1;
    }

    ctx = (struct hmi_icc *)context;

    if (size > HMI_ICC_CHANNEL_SIZE || size < 0) {
        ctx->hmi_icc_stats.read_cb_invalid_para++;
        return -1;
    }

    read_size = bsp_icc_read(id, ctx->read_buf, size);
    if(size != read_size) {
        ctx->hmi_icc_stats.read_err++;
        return -1;
    }

    if (ctx->read_cb != NULL) {
        ret = ctx->read_cb(ctx->read_buf, size);
        if (ret != 0) {
            ctx->hmi_icc_stats.read_cb_called_fail++;
        }
    } else {
        ctx->hmi_icc_stats.read_cb_null++;
    }

    ctx->hmi_icc_stats.read_succ++;

    return 0;
}

int hmi_hal_init(void)
{
    struct hmi_icc *ctx = &g_hmi_icc_ctx;
    int ret;

    ctx->read_buf = (unsigned char *)osl_malloc(sizeof(unsigned char) * HMI_ICC_CHANNEL_SIZE);
    if (ctx->read_buf == NULL) {
        bsp_err("read_buf malloc err, size = %d\n", HMI_ICC_CHANNEL_SIZE);
        return -1;
    }

    ret = bsp_icc_event_register(HMI_ICC_CHANNEL_ID, hmi_icc_readcb, (void*)ctx, 0, 0);
    if (ret != 0) {
        bsp_err("icc event register fail\n");
        ctx->hmi_icc_stats.icc_event_register_fail++;
        return -1;
    }

    return 0;
}

int hmi_get_hal_om_len(void)
{
    struct hmi_icc *ctx = &g_hmi_icc_ctx;

    return sizeof(ctx->hmi_icc_stats);
}

void hmi_report_hal_info(unsigned char *data, int max_len)
{
    struct hmi_icc *ctx = &g_hmi_icc_ctx;
    int ret;

    ret = memcpy_s(data, max_len, &ctx->hmi_icc_stats, sizeof(ctx->hmi_icc_stats));
    if (ret != EOK) {
        bsp_err("icc event register fail\n");
    }
}
