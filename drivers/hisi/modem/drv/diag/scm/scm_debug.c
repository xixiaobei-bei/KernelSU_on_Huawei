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

#include "scm_debug.h"
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "soc_socp_adapter.h"
#include "diag_port_manager.h"
#include "ppm_common.h"
#include "diag_system_debug.h"


void scm_socp_send_data_to_udi_succ(socp_coder_dst_e chan_id, unsigned int phy_port,
    mdrv_ppm_port_debug_info_s *debug_info, u32 *send_len)
{
    ppm_pseudo_sync_s *usb_cfg_pseudo_sync = ppm_get_cfg_pseudo_sync_info();
    ppm_pseudo_sync_s *usb_ind_pseudo_sync = ppm_get_ind_pseudo_sync_info();
    if ((chan_id == SOCP_CODER_DST_OM_CNF) && (phy_port == CPM_CFG_PORT)) {
        if ((usb_cfg_pseudo_sync->length == 0) || (*send_len != usb_cfg_pseudo_sync->length)) {
            debug_info->usb_send_cb_abnormal_num++;
            debug_info->usb_send_cb_abnormal_len += *send_len;
        }

        *send_len = usb_cfg_pseudo_sync->length;
    } else if ((chan_id == SOCP_CODER_DST_OM_IND) && (phy_port == CPM_IND_PORT)) {
        if ((usb_ind_pseudo_sync->length == 0) || (*send_len != usb_ind_pseudo_sync->length)) {
            debug_info->usb_send_cb_abnormal_num++;
            debug_info->usb_send_cb_abnormal_len += *send_len;
        }

        *send_len = usb_ind_pseudo_sync->length;
    } else {
        ;
    }

    return;
}

