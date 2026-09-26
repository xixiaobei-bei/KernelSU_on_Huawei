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

#ifndef _MLOADER_COMM_H_
#define _MLOADER_COMM_H_

#include <product_config.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/sched.h>

#include <osl_common.h>
#include <osl_sem.h>

#include <bsp_msg.h>
#include <bsp_version.h>
#include <bsp_mloader.h>
#include <bsp_sec.h>
#include <bsp_print.h>

#ifdef __cplusplus
extern "C" {
#endif

#define mloader_print_err(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))
#define mloader_print_info(fmt, ...) (bsp_info(fmt, ##__VA_ARGS__))

#define MODEM_DTB_HEADER_LEN (1024 * 6)

#define MLOADER_FILE_NAME_LEN 128

#define MODEMID_MASK_BITS (0xff)
#define modemid_k_bits(n) (((n) >> 24) & MODEMID_MASK_BITS)
#define modemid_h_bits(n) (((n) >> 16) & MODEMID_MASK_BITS)
#define modemid_m_bits(n) (((n) >> 8) & MODEMID_MASK_BITS)
#define modemid_l_bits(n) ((n) & MODEMID_MASK_BITS)

#define DECOMPRESS_ZLIB 1
#define DECOMPRESS_ZSTD 2

#define MLOADER_RETURN_OK 0
#define MLOADER_PARAM_ERROR (-1)
#define MLOADER_NO_MEMORY (-2)
#define MLOADER_INTERFACE_ERROR (-3)

#define MLOADER_CORE_NUM 2

typedef struct modem_dt_table {
    u32 magic;
    u32 version;
    u32 num_entries;
} modem_dt_table_t;

typedef struct modem_dt_entry {
    u8 boardid[0x4];
    u8 reserved[0x4];
    u32 dtb_size;
    u32 vrl_size;
    u32 dtb_offset;
    u32 vrl_offset;
    u64 dtb_file;
    u64 vrl_file;
} modem_dt_entry_t;

typedef struct {
    char *name;
    char *dtb_ddr_name;
    u32 run_addr;
    u32 ddr_size;
    u32 core_id;
} ccpu_dts_info_s;

typedef struct {
    char *image_name;
    char *ddr_name;
    unsigned long ddr_addr;
    u32 ddr_size;
    u32 with_dts;
    ccpu_dts_info_s dtb;
} ccpu_img_info_s;

typedef struct {
    u32 image_num;
    u32 decompress_method;
    ccpu_img_info_s *img;
} mloader_info_s;

typedef struct {
    struct task_struct *taskid;
    mloader_img_icc_info_s mloader_msg;
    struct semaphore task_sem; /* 任务信号量 */
    struct wakeup_source wake_lock;
    volatile unsigned long load_flags;
    u32 op_state; /* 操作状态 */
    u32 outer_event;
    struct device *dev;
    struct list_head load_notifier_list;
} mloader_img_s;

typedef struct {
    u8 *mloader_secboot_buffer;
    dma_addr_t mloader_secboot_phy_addr[MLOADER_CORE_NUM];
    void *mloader_secboot_virt_addr_for_ccpu;
    unsigned long mloader_secboot_phy_addr_for_ccpu;
} mloader_addr_s;

enum msg_send_e {
    NEED_SEND = 0,
    NO_NEED_SEND = 1,
    SEND_MAX = 2,
};

enum cmd_type_e {
    VERIFY_IMAGE = 0,
    VERIFY_PATCH_IMAGE = 1,
    SPLICING_IMAGE = 2,
    INFLATE_IMAGE = 3,
};

#ifndef BSP_CONFIG_PHONE_TYPE
static inline int bsp_need_loadmodem(void)
{
    return 1;
}
#endif

ccpu_img_info_s *mloader_get_image_info(const char *image_name);
int mloader_get_file_size(const char *filename);
int mloader_get_file_name(char *file_name, const char *image_name, u32 is_sec);
int mloader_read_file(const char *file_name, unsigned int offset, unsigned int length, char *buffer);
int mloader_decompress(u32 ddr_size, void *vaddr_load, void *vaddr, u32 file_size);
int modem_dir_init(void);
void mloader_load_notifier_process(load_stage_e state);

#ifdef __cplusplus
}
#endif

#endif
