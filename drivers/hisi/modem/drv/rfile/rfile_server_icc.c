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
#include "rfile_server_icc.h"
#include <bsp_print.h>
#include <bsp_icc.h>

#define THIS_MODU mod_rfile

#define RFILE_CCORE_ICC_CHAN (ICC_CHN_RFILE << 16 | RFILE_RECV_FUNC_ID)

s32 rfile_icc_cb(u32 channel_id, u32 len, void *context)
{
    bsp_debug("<%s> entry.\n", __FUNCTION__);

    if (channel_id != RFILE_CCORE_ICC_CHAN) {
        bsp_err("<%s> channel_id %d error.\n", __FUNCTION__, channel_id);
        return BSP_ERROR;
    }

    rfile_icc_common_cb();

    return BSP_OK;
}

int rfile_icc_common_recv(void* databuf, u32 datalen)
{
    return bsp_icc_read(RFILE_CCORE_ICC_CHAN, (u8 *)databuf, datalen);
}

int rfile_icc_common_send(void* databuf, u32 datalen)
{
    return bsp_icc_send(ICC_CPU_MODEM, RFILE_CCORE_ICC_CHAN, (u8 *)databuf, datalen);
}

int rfile_icc_common_init(void)
{
    int ret;
    ret = bsp_icc_event_register(RFILE_CCORE_ICC_CHAN, rfile_icc_cb, NULL, NULL, NULL);
    if (ret) {
        bsp_err("[init] <%s> bsp_icc_event_register failed.\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

