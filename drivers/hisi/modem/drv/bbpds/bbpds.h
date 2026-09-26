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

#ifndef _BBPDS_H
#define _BBPDS_H

#include <linux/device.h>
#include "osl_types.h"
#include "mdrv_msg.h"
#include "bsp_print.h"

#ifdef __cplusplus
extern "C" {
#endif


#define bbpds_crit(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define bbpds_error(fmt, ...) bsp_err("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define bbpds_warning(fmt, ...) bsp_wrn("<%s %d>" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)


#define MSG_QNUM_BBPDS_APSS 0x1
#define MSG_QID_BBPDS_APSS 0x0


#define BBPDS_GET_DATA_MEM 0x4453 /* ASCII:DS */

typedef struct {
    u32 msg_id;
    u8 data[0];
} tsp_bbpds_info_s;

typedef struct {
    u32 mem_useable;
    u32 mem_addr;
    u32 mem_size;
} bbpds_data_mem_info_s;

typedef struct {
    MDRV_MSG_HEAD
    u8 ds_req[0];
} bbpds_msg_info_s;

s32 bbpds_init(void);

#ifdef DIAG_SYSTEM_FUSION
int bbpds_proc_task(void *data);
void bbpds_msg_proc(bbpds_msg_info_s *msg);
void bbpds_send_mem_info(void);
s32 bbpds_msg_send(const void *data, u32 len, u32 send_mid, u32 recv_mid);
#endif

s32 bbpds_buf_init(struct device_node *dev);

#ifdef __cplusplus
}
#endif

#endif
