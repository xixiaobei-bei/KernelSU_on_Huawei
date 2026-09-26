/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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

#ifndef __BSP_COLD_PATCH_H__
#define __BSP_COLD_PATCH_H__

#include "product_config.h"
#include <osl_types.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#ifdef CONFIG_MLOADER
#include <bsp_mloader.h>
#include <param_cfg_to_sec.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MODEM_UPDATE_FAIL_COUNT_MAX 3

enum modem_patch_status {
    PUT_PATCH_SUCESS,
    NOT_LOAD_PATCH,
    LOAD_PATCH_FAIL,
    PUT_PATCH,
    PUT_PATCH_FAIL,
};

#define NVME_COLD_PATCH_ID 260

enum modem_patch_type {
    CCORE_PATCH,
    NV_PATCH,
    DSP_PATCH,
    LR_PATCH,
    NR_PATCH = 4,
    LR_PHY_PATCH,
    RFDSP1_PATCH,
    RFDSP2_PATCH,
    L2HAC_PATCH = 8,
    NR_PHY_PATCH,
    PDE_PDF_PATCH,
    PDE_CSI_PATCH,
    RFDSP1_ES_PATCH,
    RFDSP2_ES_PATCH,
    RESERVED3,
    RESERVED4,
    MAX_PATCH = 16,
};

#pragma pack(push, 1)

struct modem_patch_info_s {
    char patch_exist;  // 补丁是否存在
    char patch_status; // 是否已经打补丁
    char patch_type;
    char reserved1;
};

struct patch_ret_value_s {
    unsigned int load_ret_val;
    unsigned int splice_ret_val;
};

struct cold_patch_info_s {
    char cold_patch_status; // 补丁是否已经打上，只要任意一个补丁镜像加载、拼接成功，该成员就会被置1;
    // modem在异常复位时，如果该成员被置为1，更新成员modem_update_fail_count
    char modem_patch_update_count; // 冷补丁更新次数
    char modem_update_fail_count; // modem升级失败次数，当该成员超过3次后，再次启动时就不会加载补丁镜像
    char nv_rollback; // 如果冷补丁存在，modem复位达到3次后，需要更新此标记，进行nv冷补丁回退
    struct modem_patch_info_s modem_patch_info[MAX_PATCH]; // 分别记录nv、dsp、ccore的补丁信息,剩下的预留
    u32 lr_patch_size;
};

#pragma pack(pop)

/* hot patch only start */
#define MODEM_HOT_PATCH_IMAGE_PATH "/mnt/modem/patch_hw/"

enum modem_hot_patch_type_e {
    XLOADER_PATCH = 0,
    HIBOOT_PATCH,
    MODEM_DT_PATCH,
    TRUSTED_CORE_PATCH,
    LPMCU_PATCH,
    HIFI_PATCH = 5,
    BOOT_PATCH,
    BL31_PATCH,
    LR_HOT_PATCH,
    NR_HOT_PATCH,
    LR_PHY_HOT_PATCH = 10,
    L2HAC_HOT_PATCH,
    NR_PHY_HOT_PATCH,
    PDE_PDF_HOT_PATCH,
    PDE_CSI_HOT_PATCH,
    RFDSP1_HOT_PATCH = 15,
    RFDSP2_HOT_PATCH,
    RFDSP1_ES_HOT_PATCH,
    RFDSP2_ES_HOT_PATCH,
    RESERVED1,
    MAX_HOT_PATCH = 20,
};

struct modem_hot_patch_name_info_s {
    enum modem_hot_patch_type_e patch_type;
    const char *name;
};

struct modem_hot_patch_info_s {
    char patch_exist;  // 补丁是否存在
    char patch_status; // 是否已经打补丁
    char result;
    char reserved1;
};

enum hot_patch_icc_op {
    GET_FAIL_COUNT = 0,
    PATCH_UPDATE_STATUS = 1,
};

struct mloader_hot_patch_icc_info_s {
    u32 op;
    u32 fail_count;
    u32 epatch_type;
    struct modem_hot_patch_info_s modem_hot_patch_info;
};

struct mloader_hot_patch_icc_info_to_b_s {
    char modem_update_fail_count;
    char reserved1;
    char reserved2;
    char reserved3;
};

struct hot_patch_info_b_s {
    volatile char icc_received_flag;
    char cold_patch_status; // 补丁是否已经打上，只要任意一个补丁镜像加载、拼接成功，该成员就会被置1;
    // modem在异常复位时，如果该成员被置为1，更新成员modem_update_fail_count
    char modem_update_fail_count; // modem升级失败次数，当该成员超过3次后，再次启动时就不会加载补丁镜像
    struct modem_hot_patch_info_s modem_hot_patch_info[MAX_HOT_PATCH];
};

enum patch_flag_e {
    PATCH_EXIST = 0,
    RESULT = 1,
};

struct hot_patch_info_s {
    char cold_patch_status; // 补丁是否已经打上，只要任意一个补丁镜像加载、拼接成功，该成员就会被置1;
    // modem在异常复位时，如果该成员被置为1，更新成员modem_update_fail_count
    char modem_patch_update_count; // 冷补丁更新次数
    char modem_update_fail_count; // modem升级失败次数，当该成员超过3次后，再次启动时就不会加载补丁镜像
    char rev;
    struct modem_patch_info_s modem_patch_info[MAX_HOT_PATCH]; // 分别记录nv、dsp、ccore的补丁信息,剩下的预留
    u32 lr_patch_size;
};

struct hot_patch_work_info_s {
    struct workqueue_struct *hot_patch_wq;
    struct work_struct update_nve_work;
};

void mloader_update_hot_patch_status(const char *file_name, enum patch_flag_e flag, u32 status);
void bsp_modem_update_hot_patch_nve(void);
int mloader_update_patch_info(const char *file_name, enum modem_patch_status status);
int modem_get_hot_patch_nve_info_before_load(void);
u32 modem_hot_patch_get_load_fail_time(void);
u32 mloader_hot_patch_get_load_fail_time(void);
void modem_hot_patch_debug_print(void);
bool mloader_hot_patch_is_exist(void);
/* hot patch only end */
struct cold_patch_info_s *mloader_get_cold_patch_info_st(void);
int bsp_nvem_cold_patch_read(struct cold_patch_info_s *p_cold_patch_info);
int bsp_nvem_cold_patch_write(struct cold_patch_info_s *p_cold_patch_info);
bool bsp_modem_cold_patch_is_exist(void);
void bsp_modem_cold_patch_update_modem_fail_count(void);
ssize_t modem_imag_patch_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t modem_imag_patch_status_show(struct device *dev, struct device_attribute *attr, char *buf);
int bsp_modem_cold_patch_init(void);
int bsp_modem_is_reboot_machine(void);
struct patch_ret_value_s *bsp_modem_get_patch_ret_s(void);
#ifdef CONFIG_MLOADER
int mloader_is_patch_image(const char *image_name, u32 buf_len);
int mloader_splicing_image(enum modem_patch_type epatch_type, int ret);
u32 mloader_patch_get_load_position_offset(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 ddr_size, int img_size);
u32 mloader_patch_get_load_fail_time(void);
void mloader_record_cold_patch_splicing_ret_val(enum modem_patch_type epatch_type, int value);
void mloader_update_modem_cold_patch_status(enum modem_patch_type epatch_type);
#endif
#ifdef __cplusplus
}
#endif

#endif
