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

#ifndef __OM_USB_PPM_H__
#define __OM_USB_PPM_H__

#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <osl_types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#pragma pack(push)
#pragma pack(4)

/*
 * 4 结构体定义
 */
typedef struct {
    u32 usb_send_time;
    u32 usb_cb_time;
    u32 usb_max_send_time;
    u32 usb_max_cb_time;
    u32 usb_cb_cnt;
} ppm_usb_debug_info_s;

/*
 * 4 函数声明
 */
u32 ppm_usb_cfg_send_data(u8 *virt_addr, u8 *phy_addr, u32 len);
void ppm_usb_cfg_port_close(void);
void ppm_usb_cfg_status_cb(ACM_EVT_E state);
void ppm_usb_cfg_write_data_cb(char *virt_addr, char *phy_addr, int len);
void ppm_usb_cfg_read_data_cb(void);
void ppm_usb_cfg_port_open(void);
void ppm_usb_ind_status_cb(ACM_EVT_E state);
#ifdef BSP_CONFIG_PHONE_TYPE
void ppm_usb_ind_write_data_cb(char *virt_addr, char *phy_addr, int len);
#else
void ppm_usb_ind_write_data_cb(struct acm_write_info *acm_write_info);
#endif
void ppm_usb_ind_port_open(void);
void ppm_usb_ind_port_close(void);
u32 ppm_usb_ind_send_data(u8 *virt_addr, u8 *phy_addr, u32 len);
u32 ppm_usb_cfg_port_init(void);
u32 ppm_usb_ind_port_init(void);
u32 ppm_usb_port_init(void);
void ppm_query_usb_info(void *usb_info, u32 len);
void ppm_clear_usb_debug_info(void);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of OmCommonPpm.h */
