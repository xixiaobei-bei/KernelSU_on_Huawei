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
#ifndef _ONOFF_MSG_H_
#define _ONOFF_MSG_H_

#include <linux/sched.h>
#include <linux/rtc.h>
#include <hi_sysboot.h>
#ifdef HI_BALONG_ONOFF_MSG
#include <mdrv_msg.h>
#endif

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

#ifdef HI_BALONG_ONOFF_MSG
typedef struct {
    MDRV_MSG_HEAD;
    tag_ctrl_msg_s msg_cont;
}onoff_msg_cont_s;
#endif
enum onoff_msg_type_e {
    ICC = 0,
    MSG = 1,
    TYPE_MAX = 2,
};

enum onoff_msg_send_e {
    NEED_SEND = 0,
    NO_NEED_SEND = 1,
    SEND_MAX = 2,
};

typedef struct {
    struct task_struct *msg_thr;
    int msg_run;
}onoff_msg_msg_info_s;

typedef struct {
    u32 reserved;
}onoff_msg_icc_info_s;

typedef struct {
    enum onoff_msg_type_e msg_type;
    onoff_msg_icc_info_s icc_info;
    onoff_msg_msg_info_s msg_info;
}onoff_msg_s;

int onoff_msg_init_process(void);
onoff_msg_s *onoff_get_msg_st(void);

int onoff_msg_init(void);
int onoff_icc_init_process(void);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif


