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

#ifndef __OM_PORT_SWITCH_H__
#define __OM_PORT_SWITCH_H__

#include <mdrv_diag_system.h>
#include <osl_types.h>
#include <nv_stru_drv.h>
#include <ppm_common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

#define AP_CP_PORT_SWITCH_TIMEOUT_MS 80 /* 80ticks * 10ms equal 800ms */

typedef enum {
    PPM_MSGID_PORT_SWITCH_NV_A2C = 1, /* PPM把切端口NV从A核给C核写NV标志 */
    PPM_MSGID_PORT_SWITCH_NV_C2A = 2, /* PPM把切端口NV结果从ccore返回acore */
    PPM_MSGID_BUTT
} ppm_port_switch_msgid_e; // PPM_消息ID记录

typedef struct {
    u32 msg_id;
    u32 sn;
    u32 ret;
    u32 len;
    DIAG_CHANNLE_PORT_CFG_STRU data;
} ppm_port_switch_nv_info_s;

/* PPM端口统计信息 */
typedef struct {
    u32 port_type_err;
    u32 switch_fail;
    u32 switch_succ;
    u32 start_slice;
    u32 end_slice;
} ppm_port_cfg_info_s;

typedef struct {
    u32 msg_id;   /* 消息ID */
    u32 length;   /* 消息长度 */
    u32 port_num; /* 端口 */
} ap_cp_msg_port_s;

typedef struct {
    u32 msg_id; /* 消息ID */
    u32 length; /* 消息长度 */
    u32 ret;    /* 结果 */
} ap_cp_msg_ret_s;

void ppm_disconnect_port(unsigned int chan);
void ppm_disconnect_cb_reg(ppm_disconnect_port_cb cb);
u32 ppm_port_switch(u32 phy_port);
int ppm_port_switch_init(void);
void ppm_cp_port_switch_read_cb(void *context);
void ppm_ap_port_switch_read_cb(void *context);
u32 ppm_phy_port_switch(u32 phy_port);
u32 ppm_set_ind_mode(void);
u32 ppm_write_port_nv(void);
u32 ppm_query_port(u32 *pulLogPort);
u32 bsp_diag_set_port(u32 phy_port);
u32 bsp_diag_get_port(u32 *phy_port);
ppm_port_cfg_info_s *ppm_get_switch_debug_info(void);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of OmPortSwitch.h */
