/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include <bsp_icc.h>
#include <bsp_sec_call.h>
#include <securec.h>
#include <bsp_print.h>
#define THIS_MODU mod_security

#define security_print(fmt, ...) (bsp_err("<%s> " fmt "\n", __FUNCTION__, ##__VA_ARGS__))
#define security_info(fmt, ...) (bsp_info("<%s> " fmt "\n", __FUNCTION__, ##__VA_ARGS__))

#define TRNG_POOL_DEPTH 2

int trng_request_icc_cb(unsigned int channel_id, unsigned int len, void *context)
{
    int ret;
    int length;
    unsigned int seed_group_number = 0;
    FUNC_CMD_ID cmd = FUNC_TRNG_SEED_REQUEST;

    security_info("acore trng_request icc cb!!");
    length = bsp_icc_read(channel_id, (u8 *)(&seed_group_number), sizeof(seed_group_number));
    if (length != (int)sizeof(seed_group_number)) {
        security_print("icc read len(%x) != expected len(%lx).\n", length, sizeof(seed_group_number));
        return -1;
    }

    if (seed_group_number != 1) {
        security_print("seed_group_number error, seed_group_number = (%x).", seed_group_number);
        return -1;
    }

    ret = bsp_sec_call(cmd, seed_group_number);
    if (ret != 0) {
        security_print("bsp_sec_call return error,ret is %d.\n", ret);
        return ret;
    }
    security_info("acore trng_request icc cb done !!");
    return 0;
}


int bsp_trng_seed_init(void)
{
    int ret;

    u32 chan_id = (ICC_CHN_IFC << 16U) | IFC_RECV_FUNC_TRNG_SEED;

    ret = bsp_icc_event_register(chan_id, (read_cb_func)trng_request_icc_cb, NULL, NULL, NULL);
    if (ret != ICC_OK) {
        security_print("[init]icc_event_register error.\n");
        return ret;
    }
    security_print("[init]ACORE bsp_trng_seed_init OK!");
    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_trng_seed_init);
#endif
