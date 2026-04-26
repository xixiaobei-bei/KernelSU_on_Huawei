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

#ifndef _MLOADER_DEBUG_H_
#define _MLOADER_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

enum ccore_state_e {
    SETED = 1,
    LOADED = 2,
    VERIFIED = 3,
};

/* ONLY FOR DEBUG START */
struct mloader_ccore_debug_info {
    u32 start_time;
    u32 state;
    u32 hot_patch;
    u32 cold_patch;
    u32 end_time;
};

struct mloader_ccore_images_debug_info {
    u32 start_time;
    u32 core_id;
    u32 img_index;
    u32 op;
    u32 end_time;
};

struct mloader_hot_patch_debug_info {
    u32 fail_time;
    u32 fail_time_icc_flag;
    u32 fail_time_icc_time;
};
#define MAX_DEBUG_SIZE 32

#define MLOADER_MAGIC 0xabcd5a5a
typedef struct {
    u32 start_magic;
    u32 start_time;
    u32 op_state;
    struct mloader_hot_patch_debug_info hot_patch_debug_info;
    struct mloader_ccore_debug_info ccore_debug_info;
    struct mloader_ccore_images_debug_info imgs_debug_info[MAX_DEBUG_SIZE];
    u32 end_time;
    u32 end_magic;
} mloader_debug_info_s;
/* ONLY FOR DEBUG END */
void mloader_update_start_time_debug_info(void);
void mloader_update_end_time_debug_info(void);
void mloader_update_op_state(u32 op_state);
void mloader_update_ccore_start_time_debug_info(void);
void mloader_update_ccore_end_time_debug_info(void);
void mloader_update_ccore_state_debug_info(enum ccore_state_e state);
void mloader_update_ccore_cold_patch_debug_info(void);
void mloader_update_ccore_hot_patch_debug_info(void);
void mloader_update_hot_patch_debug_info(u32 fail_count);
void mloader_update_imgs_op_start_time_debug_info(void);
void mloader_update_imgs_op_end_time_debug_info(void);
void mloader_update_imgs_op_core_id_debug_info(u32 core_id);
void mloader_update_imgs_op_img_idx_debug_info(u32 img_idx);
void mloader_update_imgs_op_op_debug_info(u32 op);
void mloader_debug_init(void);

#ifdef __cplusplus
}
#endif

#endif
