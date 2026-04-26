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
/*
 * File Name       : mloader_load_image.c
 * Description     : load modem image(ccore image),run in ccore
 * History         :
 */
#include <product_config.h>
#include <securec.h>
#include <hi_mloader.h>
#include "mloader_comm.h"
#include "mloader_load_image.h"
#include "mloader_debug.h"
#include "mloader_msg.h"

#define THIS_MODU mod_mloader

u32 mloader_get_icc_cpu(u32 channel_id)
{
    u32 cpu_id = channel_id == ICC_CHANNEL_ID_MAKEUP(ICC_CHN_MLOADER, 0) ? ICC_CPU_MODEM : ICC_CPU_NRCCPU;
    return cpu_id;
}

int mloader_icc_callback(u32 channel_id, u32 len, void *context)
{
    int ret;
    mloader_img_s *mloader_images = NULL;

    mloader_images = bsp_mloader_get_images_st();

    ret = bsp_icc_read(channel_id, (u8 *)&(mloader_images->mloader_msg), sizeof(mloader_img_icc_info_s));
    if ((u32)ret != sizeof(mloader_img_icc_info_s)) {
        mloader_print_err("<%s> icc read error, ret = %d\n", __FUNCTION__, ret);
        return ret;
    }

    mloader_images->mloader_msg.channel_id = channel_id;
    mloader_print_err("modem icc callback wakeup load task\n");

    __pm_stay_awake(&mloader_images->wake_lock);
    osl_sem_up(&mloader_images->task_sem);
    return 0;
}

void mloader_send_msg_icc(mloader_img_icc_status_s *status_msg)
{
    u32 cpu_id;
    u32 ret;
    mloader_img_icc_info_s *mloader_msg = NULL;
    mloader_img_s *mloader_images = NULL;

    mloader_images = bsp_mloader_get_images_st();

    mloader_msg = &(mloader_images->mloader_msg);

    cpu_id = mloader_get_icc_cpu(mloader_msg->channel_id);
    mloader_print_err("send icc to ccore, for load (%d)%d.\n", cpu_id, status_msg->img_idx);
    ret = bsp_icc_send(cpu_id, mloader_msg->channel_id, (u8 *)(status_msg), sizeof(mloader_img_icc_status_s));
    if (sizeof(mloader_img_icc_status_s) != ret) {
        mloader_print_err("ret = 0x%x, msg_size = 0x%x\n", ret, (unsigned int)sizeof(mloader_img_icc_status_s));
    }
}

int mloader_msg_init(void)
{
    int ret;
    ret = bsp_icc_event_register(ICC_CHANNEL_ID_MAKEUP(ICC_CHN_MLOADER, 0), mloader_icc_callback, NULL, NULL, NULL);
    if (ret) {
        mloader_print_err("bsp_icc_event_register fail, ret = %d\n", ret);
        return ret;
    }
    ret = bsp_icc_event_register(ICC_CHANNEL_ID_MAKEUP(ICC_CHN_NRCCPU_APCPU_MLOADER, 0), mloader_icc_callback, NULL, NULL,
        NULL);
    if (ret) {
        mloader_print_err("bsp_icc_event_register fail, ret = %d\n", ret);
        return ret;
    }
    mloader_print_err("mloader msg init ok, msg type is icc.\n");
    return 0;
}

