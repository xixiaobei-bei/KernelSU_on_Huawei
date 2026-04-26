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
#include <product_config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <mdrv_sysboot.h>
#include <bsp_slice.h>

#include <securec.h>
#include "mloader_comm.h"
#include "mloader_load_image.h"
#include "mloader_debug.h"
#include "mloader_msg.h"
#include "mloader_error.h"

#define THIS_MODU mod_mloader

int mloader_verify_preprocess(mloader_img_icc_info_s *mloader_msg, const char *file_name, unsigned int len)
{
    int ret;
    int is_patch;
    enum modem_patch_type patch_type;
    struct cold_patch_info_s *mloader_cold_patch_info = NULL;

    mloader_cold_patch_info = mloader_get_cold_patch_info_st();
    if (mloader_cold_patch_info == NULL) {
        mloader_print_err("mloader_verify_preprocess fail to get mloader_cold_patch_info.\n");
        return -1;
    }

    is_patch = mloader_is_patch_image(file_name, len);
    if (is_patch) {
        patch_type = mloader_patch_get_type(file_name, len);
        if (patch_type == MAX_PATCH) {
            mloader_print_err("fail to get patch_type in mloader_verify_preprocess.\n");
            return MLOADER_GET_PATCH_TYPE_FAIL;
        }
        mloader_cold_patch_info->modem_patch_info[patch_type].patch_exist = 1;
        mloader_cold_patch_info->modem_patch_info[patch_type].patch_status = PUT_PATCH;
    } else {
        patch_type = MAX_PATCH;
    }
    ret = mloader_verify_modem_image(0);
#ifdef CONFIG_MLOADER_COLD_PATCH
    if (ret != 0) {
        mloader_record_cold_patch_splicing_ret_val(patch_type, ret);
    }
    if (mloader_msg->cmd_type == SPLICING_IMAGE) {
        mloader_update_modem_cold_patch_status(patch_type);
    }
#endif
    return ret;
}

int mloader_get_patch_status_preprocess(void)
{
    struct cold_patch_info_s *mloader_cold_patch_info = NULL;

    mloader_cold_patch_info = mloader_get_cold_patch_info_st();
    if (mloader_cold_patch_info == NULL) {
        mloader_print_err("mloader_get_patch_status_preprocess fail to get mloader_cold_patch_info.\n");
        return MLOADER_PATCH_LOAD_FAIL;
    }
    return mloader_cold_patch_info->modem_update_fail_count == 0x3 ? MLOADER_PATCH_LOAD_FAIL : MLOADER_PATCH_LOAD_SUCCESS;
}

int mloader_load_finished_preprocess(void)
{
    int ret = 0;
    struct cold_patch_info_s *mloader_cold_patch_info = NULL;

    mloader_cold_patch_info = mloader_get_cold_patch_info_st();
    if (mloader_cold_patch_info == NULL) {
        mloader_print_err("mloader_get_patch_status_preprocess fail to get mloader_cold_patch_info.\n");
        return MLOADER_PATCH_LOAD_FAIL;
    }
    if (bsp_modem_cold_patch_is_exist()) {
        ret = bsp_nvem_cold_patch_write(mloader_cold_patch_info);
        if (ret) {
            bsp_err("update cold patch nve failed!\n");
        }
    }
    mloader_update_end_time_debug_info();
    mloader_print_err("ccore imgs load completed.\n");
    return ret;
}

void mloader_msg_send_preprocess(mloader_img_icc_status_s *status_msg, mloader_img_icc_info_s *mloader_msg)
{
    mloader_addr_s *mloader_addr = NULL;
    mloader_addr = bsp_mloader_get_addr();
    if (mloader_addr == NULL) {
        mloader_print_err("mloader_msg_send_preprocess fail to get mloader_addr.\n");
        status_msg->image_addr = 0xffffffff;
    } else {
        status_msg->image_addr = (u32)(mloader_addr->mloader_secboot_phy_addr[0]);
    }

    status_msg->op = mloader_msg->op;
    status_msg->img_idx = mloader_msg->img_idx;
    status_msg->request_id = mloader_msg->request_id;
    status_msg->time_stamp = bsp_get_slice_value();
    return;
}

int mloader_load_op_process(mloader_img_icc_info_s *mloader_msg, int *result, const char *file_name, unsigned int len)
{
    int ret;

    mloader_print_err("load ccore imgs, op = %d\n", mloader_msg->op);

    switch (mloader_msg->op) {
        case MLOADER_OP_VERIFY_IMAGE:
            ret = mloader_verify_preprocess(mloader_msg, file_name, len);
            *result = ret == 0 ? 0 : ret;
            ret = NEED_SEND;
            break;
        case MLOADER_OP_GET_PATCH_STATUS:
            *result = mloader_get_patch_status_preprocess();
            ret = NEED_SEND;
            break;
        case MLOADER_OP_LOAD_FINISHED:
            ret = mloader_load_finished_preprocess();
            *result = ret == 0 ? 0 : ret;
            mloader_load_notifier_process(AFTER_ALL_IMAGE_LOAD);
            ret = NEED_SEND;
            break;
        default:
            mloader_print_err("msg op not right in mloader_load_ccore_imgs, op = 0x%x\n", mloader_msg->op);
            ret = NO_NEED_SEND;
            break;
    }
    return ret;
}

int mloader_load_ccore_imgs(void)
{
    int ret;
    mloader_img_icc_status_s status_msg;
    mloader_img_icc_info_s *mloader_msg = NULL;
    char file_name[MLOADER_FILE_NAME_LEN] = {0};
    mloader_img_s *mloader_images = NULL;

    mloader_images = bsp_mloader_get_images_st();

    mloader_msg = &(mloader_images->mloader_msg);

    mloader_print_err("start loading %s\n", mloader_msg->name);
    (void)memset_s(&status_msg, sizeof(status_msg), 0x0, sizeof(mloader_img_icc_status_s));

    mloader_update_imgs_op_start_time_debug_info();

    ret = mloader_get_file_name(file_name, mloader_msg->name, 0);
    if (ret) {
        status_msg.result = ret; /* image not found */
        goto LOAD_DONE;
    }
    mloader_update_imgs_op_img_idx_debug_info(mloader_msg->img_idx);
    mloader_update_imgs_op_op_debug_info(mloader_msg->op);

    ret = mloader_load_op_process(mloader_msg, &(status_msg.result), file_name, MLOADER_FILE_NAME_LEN);
    if (ret == NO_NEED_SEND) {
        goto DO_NOT_SEND;
    }
LOAD_DONE:
    mloader_msg_send_preprocess(&status_msg, mloader_msg);

    mloader_send_msg_icc(&status_msg);

    mloader_print_err("load image (%d) complete\n", status_msg.img_idx);

DO_NOT_SEND:
    mloader_update_imgs_op_end_time_debug_info();

    __pm_relax(&mloader_images->wake_lock);

    return ret;
}

