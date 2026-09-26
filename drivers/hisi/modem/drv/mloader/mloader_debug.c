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
#include <linux/slab.h>
#include <securec.h>
#include <bsp_slice.h>
#include <bsp_dump.h>
#include "mloader_comm.h"
#include "mloader_debug.h"

#define THIS_MODU mod_mloader

#define MLOADER_DUMP_SIZE (1024)
mloader_debug_info_s *g_mloader_debug_info;

static u32 g_op_index = 0;

void mloader_update_start_time_debug_info(void)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_start_time_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->start_time = bsp_get_slice_value();
}

void mloader_update_end_time_debug_info(void)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_start_time_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->end_time = bsp_get_slice_value();
}

void mloader_update_op_state(u32 op_state)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_op_state get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->op_state = op_state;
}

void mloader_update_ccore_start_time_debug_info(void)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_ccore_start_time_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->ccore_debug_info.start_time = bsp_get_slice_value();
}

void mloader_update_ccore_end_time_debug_info(void)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_ccore_end_time_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->ccore_debug_info.end_time = bsp_get_slice_value();
}

void mloader_update_ccore_state_debug_info(enum ccore_state_e state)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_ccore_state_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->ccore_debug_info.state = state;
}

void mloader_update_ccore_cold_patch_debug_info(void)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_ccore_cold_patch_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->ccore_debug_info.cold_patch = 1;
}

void mloader_update_ccore_hot_patch_debug_info(void)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_ccore_hot_patch_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->ccore_debug_info.hot_patch = 1;
}

void mloader_update_hot_patch_debug_info(u32 fail_count)
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_hot_patch_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->hot_patch_debug_info.fail_time = fail_count;
    g_mloader_debug_info->hot_patch_debug_info.fail_time_icc_flag = 1;
    g_mloader_debug_info->hot_patch_debug_info.fail_time_icc_time = bsp_get_slice_value();
}


void mloader_update_imgs_op_start_time_debug_info()
{
    if (g_mloader_debug_info == NULL || g_op_index >= MAX_DEBUG_SIZE) {
        mloader_print_err("mloader_update_imgs_op_start_time_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->imgs_debug_info[g_op_index].start_time = bsp_get_slice_value();
}

void mloader_update_imgs_op_end_time_debug_info()
{
    if (g_mloader_debug_info == NULL) {
        mloader_print_err("mloader_update_imgs_op_end_time_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->imgs_debug_info[g_op_index].end_time = bsp_get_slice_value();
    g_op_index = (g_op_index + 1) % MAX_DEBUG_SIZE;
}

void mloader_update_imgs_op_core_id_debug_info(u32 core_id)
{
    if (g_mloader_debug_info == NULL || g_op_index >= MAX_DEBUG_SIZE) {
        mloader_print_err("mloader_update_imgs_op_core_id_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->imgs_debug_info[g_op_index].core_id = core_id;
}

void mloader_update_imgs_op_img_idx_debug_info(u32 img_idx)
{
    if (g_mloader_debug_info == NULL || g_op_index >= MAX_DEBUG_SIZE) {
        mloader_print_err("mloader_update_imgs_op_img_idx_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->imgs_debug_info[g_op_index].img_index = img_idx;
}

void mloader_update_imgs_op_op_debug_info(u32 op)
{
    if (g_mloader_debug_info == NULL || g_op_index >= MAX_DEBUG_SIZE) {
        mloader_print_err("mloader_update_imgs_op_op_debug_info get  g_mloader_debug_info failed.\n");
        return;
    }
    g_mloader_debug_info->imgs_debug_info[g_op_index].op = op;
}

void mloader_debug_init(void)
{
    g_mloader_debug_info =
        (mloader_debug_info_s *)bsp_dump_register_field(DUMP_MODEMAP_MLOADER, "mloader_acore", MLOADER_DUMP_SIZE, 0);

    if (g_mloader_debug_info == NULL) {
        g_mloader_debug_info = (mloader_debug_info_s *)kzalloc(sizeof(mloader_debug_info_s), GFP_KERNEL);
        if (g_mloader_debug_info == NULL) {
            mloader_print_err("g_mloader_debug_info malloc failed.\n");
            return;
        }
    }
    g_mloader_debug_info->start_magic = MLOADER_MAGIC;
    g_mloader_debug_info->end_magic = MLOADER_MAGIC;
}
