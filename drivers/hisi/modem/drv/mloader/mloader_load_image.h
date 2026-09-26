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

#ifndef _MLOADER_LOAD_IMAGE_H_
#define _MLOADER_LOAD_IMAGE_H_

#include <product_config.h>
#include <bsp_mloader.h>
#include <bsp_cold_patch.h>
#include <param_cfg_to_sec.h>
#include "mloader_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODEM_SYSBOOT_CCPU_START 1
#define MODEM_SYSBOOT_LR_CCPU_START 2

#define IMAGE_AVAILABLE 1
#define IMAGE_DISABLE 0

#define MLOADER_LOAD_UNINITED 1
#define MLOADER_LOAD_DOING 3
#define MLOADER_LOAD_IDLE 2
#define MLOADER_LOAD_MODEM_REQ_RESET 4
#define MLOADER_LOAD_MODEM_IN_RESET 5

#define MLOADER_LOAD_REQ_HALT 0
#define MLOADER_LOAD_IN_HALT 1
#define MLOADER_LOAD_LR 2


enum BOARDID_CCORE {
    BOARDID_ACORE_IDX = 0,
    BOARDID_CCORE_IDX,
    BOARDID_NRCORE_IDX,
    BOARDID_MCORE_IDX,
    BOARDID_L2CORE_IDX,
    BOARDID_CORE_MAX
};

extern dma_addr_t g_mloader_secboot_phy_addr[MLOADER_CORE_NUM];
extern int mloader_load_ccpu(void);
extern int modem_nv_init(void);
extern int VOS_ModuleInit(void);
extern int mloader_teek_init(void);
// patch
u32 mloader_patch_get_type(const char *file_name, unsigned int len);
// load dts
extern int mloader_load_and_verify_dtb_data(ccpu_img_info_s *image);
// load image
int bsp_mloader_load_image_unsec(const char *file_name, unsigned long image_addr, u32 *image_size, u32 ddr_size,
                                 u32 image_offset);
s32 load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size, u32 index);
// load lr
void mloader_start_load_lr(void);
void mloader_load_lr_img(void);
// load ccore imgs
int mloader_load_ccore_imgs(void);
// get global param
mloader_img_s *bsp_mloader_get_images_st(void);
mloader_addr_s *bsp_mloader_get_addr(void);
// sec call
int mloader_reset_modem(enum SVC_SECBOOT_IMG_TYPE image);
int mloader_trans_vrl_to_os(enum SVC_SECBOOT_IMG_TYPE image, void *buf, const unsigned int size);
int mloader_trans_data_to_os(enum SVC_SECBOOT_IMG_TYPE image, u32 run_addr, u64 buf, const unsigned int offset,
                             const unsigned int size);
int load_data_to_secos(const char *file_name, u32 offset, u32 size, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr,
                       u32 ddr_size, u32 index);
int mloader_verify_soc_image(enum SVC_SECBOOT_IMG_TYPE image, u32 run_addr);
int mloader_verify_modem_image(unsigned int core_id);

int mloader_icc_callback(u32 channel_id, u32 len, void *context);
int mloader_load_op_process(mloader_img_icc_info_s *mloader_msg, int *result, const char *file_name, unsigned int len);
u32 mloader_get_icc_cpu(u32 channel_id);
void mloader_msg_send_preprocess(mloader_img_icc_status_s *status_msg, mloader_img_icc_info_s *mloader_msg);
void bsp_mloader_load_image_init(void);


#ifdef __cplusplus
}
#endif

#endif
