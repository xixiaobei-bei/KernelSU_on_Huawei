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

#ifndef __CHR_CFG_AP_H__
#define __CHR_CFG_AP_H__

#include "product_config.h"
#include "omerrorlog.h"
#include "blist.h"
#include "mdrv_sysboot.h"
#include "mdrv.h"

/* 优先级包数开始结束 */
#define PRIORITY_PACKET_START (0)
#define PRIORITY_PACKET_END (0x7F)

/* 周期配置包数开始结束 */
#define PERIOD_PACKET_START (0)
#define PERIOD_PACKET_END (0x7F)

#define PERIOD_CHK_FAIL (3)
#define PERIOD_CHK_COMPLETE (0)
#define PERIOD_CHK_CONTINUE (1)
#define PERIOD_CHK_RESTART (2)

#define CHR_SYSBOOT_PRIORITY_LEVEL (49)

/* a核CHR 任务收到的消息ID名 */
enum chr_id_req_msg_e {
    CHR_ID_RESET_CCORE = 0x1001,
    CHR_ID_ERR_LOG_REQ = 0x1002,
    CHR_ID_BLOCKLIST_REQ = 0x1003,
    CHR_ID_PRIORITY_REQ = 0x1004,
    CHR_ID_PERIOD_REQ = 0x1005,
    CHR_ID_REQ_BUTT
};

#define chr_print(fmt, ...) (mdrv_err("<%s>" fmt, __FUNCTION__, ##__VA_ARGS__))
#define chr_print_dbg(fmt, ...) (mdrv_debug("<%s>" fmt, __FUNCTION__, ##__VA_ARGS__))

/* 保存黑名单的结构体 */
typedef struct {
    unsigned int packet_len;
    chr_list_info_s *chr_block_list;
} chr_acpu_block_save_s;

/* OM a核发给c的黑名单结构体 */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msg_name;
    unsigned int packet_len;
    chr_list_info_s block_list[0];
} chr_acpu_block_list_s;

/* 保存优先级0的结构体 */
typedef struct {
    unsigned int priority_len;
    chr_priority_info_s priority_list[0];
} chr_acpu_priority_save_s;

/* OM a核发给c的优先级0的列表的结构体 */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msg_name;
    unsigned int packet_len;
    chr_priority_info_s priority_cfg[0];
} chr_acpu_priority_cfg_s;

/* 保存上报周期的结构体 */
typedef struct {
    unsigned int period; /* ap下发的period是8bit */
    unsigned int packet_len;
    chr_period_cfg_s period_cfg[0];
} chr_acpu_period_save_s;

/* 优先级链表结构体 */
typedef struct {
    LIST_S priority_list;
    unsigned int packet_len;
    chr_priority_info_s priority_cfg[0];
} chr_priority_node_s;

typedef struct {
    unsigned int sn;
    unsigned int packet_len;
    chr_priority_node_s priority_node;
} chr_priority_list_s;

/* 上报周期链表结构体 */
typedef struct {
    LIST_S period_list;
    unsigned int period;
    unsigned int packet_len;
    chr_period_cfg_s period_cfg[0];
} chr_period_node_s;

typedef struct {
    unsigned int sn;
    unsigned int packet_len;
    chr_period_node_s period_node;
} chr_period_list_s;

/* OM a核发给c的周期配置列表 */
typedef struct {
    VOS_MSG_HEADER
    unsigned int msg_name;
    unsigned int period;
    unsigned int packet_len;
    chr_period_cfg_s period_cfg[0];
} chr_acpu_period_cfg_s;

unsigned int chr_acpu_block_list_proc(unsigned char *data, unsigned int len);
unsigned int chr_acpu_priority_cfg_proc(unsigned char *data, unsigned int len);
unsigned int chr_acpu_period_cfg_proc(unsigned char *data, unsigned int len);
int chr_reset_ccore_cb(drv_reset_cb_moment_e param, int userdata);
unsigned int chr_acpu_reset_proc(void);

unsigned int chr_cfg_init(void);
void chr_acpu_send_app_cfg_result(unsigned int result);

#endif
