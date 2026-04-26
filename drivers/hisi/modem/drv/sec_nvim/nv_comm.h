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

#ifndef _NV_COMM_H_
#define _NV_COMM_H_

#include <product_config.h>
#include <asm/io.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/dma-mapping.h>
#include <drv_comm.h>
#include <nv_stru_sys.h>
#include <nv_stru_drv.h>
#include <acore_nv_stru_drv.h>
#include <nv_id_msp.h>
#include <acore_nv_id_msp.h>
#include <param_cfg_to_sec.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <osl_spinlock.h>
#include <osl_bio.h>
#include <bsp_nvim.h>
#include <bsp_shared_ddr.h>
#include <bsp_pm_om.h>
#include <bsp_slice.h>
#include <bsp_ipc.h>
#include <bsp_icc.h>
#include "nv_debug.h"
#include "nv_partition_upgrade.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct nv_global_ctrl_info_stru *nv_get_ctrl_info(void);

#define nv_malloc(a) kmalloc(a, GFP_KERNEL)
#define nv_free(p) kfree(p)

#define nv_taskdelay(n) msleep(n)
#define nv_flush_cache(ptr, size) mb()

#define nv_debug_trace(pdata, len)  do {                               \
        u32 marcro_i;                                                  \
        struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info(); \
        if (nv_ctrl->debug_sw == true) {                               \
            for (marcro_i = 0; marcro_i < (len); marcro_i++) {         \
                nv_printf("%02x ", *((u8 *)(pdata) + marcro_i));       \
            }                                                          \
            nv_printf("\n");                                           \
        };                                                             \
    } while (0)

#define NV_ID_SYS_MAX_ID 0xd1ff
#define NV_ID_SYS_MIN_ID 0xd100

#define nv_spin_lock(nvflag, lock)                                     \
    do {                                                               \
        struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info(); \
        spin_lock_irqsave(&(nv_ctrl->spinlock), nvflag);               \
        bsp_ipc_spin_lock(lock);                                       \
    } while (0)

#define nv_spin_unlock(nvflag, lock)                                   \
    do {                                                               \
        struct nv_global_ctrl_info_stru *nv_ctrl = nv_get_ctrl_info(); \
        bsp_ipc_spin_unlock(lock);                                     \
        spin_unlock_irqrestore(&(nv_ctrl->spinlock), nvflag);          \
    } while (0)

#define IPC_SME_TIME_OUT 1000
#define NV_DELAY_TIME 1000

/* 睡眠唤醒状态 */
enum {
    NV_WAKEUP_STATE = 0,
    NV_SLEEP_STATE = 1
};

/* 操作进行与空闲状态 */
enum {
    NV_IDLE_STATE = 0,
    NV_OPS_STATE = 1
};

#define NV_MAX_WAIT_TICK 50000

#define NV_ICC_BUF_LEN 64
#define NV_ICC_SEND_COUNT 5
#define NV_ICC_TASK_PRIORITY 15
#define NV_ICC_TASK_STACK_SIZE 1024

#define NV_ROOT_PATH "/mnvm2:0"

enum section_type {
    NV_SECTION_HEAD = 0x0,
    NV_SECTION_RS,
    NV_SECTION_RO,
    NV_SECTION_RW,
    NV_SECTION_MAX,
};

enum {
    NV_PRODUCT_PHONE = 0,
    NV_PRODUCT_MBB = 1,
    NV_PRODUCT_BUTT
};

typedef struct nv_flush_list_node_stru {
    u16 itemid;  /* NV ID */
    u16 modemid; /* NV Length */
    struct list_head stList;
} nv_flush_node_s;

typedef struct _nv_wr_req {
    u32 itemid;
    u32 modemid;
    u32 offset;
    u8 *pdata;
    u32 size;
} nv_rdwr_req;

typedef struct _nv_revert_file_s {
    FILE *fp;
    u8 *global_info;
    u8 *ctrl_data;
} nv_revert_file_s;

/* global info type define */
struct nv_global_ctrl_info_stru {
    u8 nv_icc_buf[NV_ICC_BUF_LEN];
    osl_sem_id rw_sem; /* 与IPC锁同时使用保证本核访问的互斥 */
    osl_sem_id task_sem;
    osl_sem_id suspend_sem;
    u32 mid_prio; /* nv priority */
    NV_SELF_CTRL_STRU nv_self_ctrl;
    u32 debug_sw;
    u32 revert_count;
    u32 revert_search_err;
    u32 revert_len_err;
    spinlock_t spinlock;
    struct task_struct *task_id;
    nv_global_info_s *shared_addr;
    u32 icc_cb_count;
    u32 task_proc_count;
    struct list_head stList;
    bool statis_sw;
    struct wakeup_source wake_lock;
    u32 opState;
    u32 pmState;
    bool pmSw;
    u32 initStatus;
    u32 acore_flush_req;
    u32 acore_flush_count;

    osl_sem_id nv_list_sem;   /* 用于保护nv_list的信号量 */
    struct list_head nv_list; /* 存储待写入的list的buf */
};

#ifdef CONFIG_IMAGE_LOAD
extern s32 load_image(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size);
extern int bsp_load_modem_single_image(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size);
#endif
#ifdef CONFIG_MLOADER
extern int bsp_mloader_load_single_image(const char *file_name, u32 offset, u32 size,
    enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size);
extern int bsp_mloader_load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size);
#endif

extern int bsp_load_modem_single_image(enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size);

u32 nv_ipc_sem_take(u32 sem, u32 timeout);
void nv_ipc_sem_give(u32 sem);
u32 nv_read_from_mem(nv_rdwr_req *rreq, nv_item_info_s *item_info, nv_ctrl_info_s *ctrl_info);
u32 nv_get_file_len(FILE *fp);
void nv_create_flag_file(const s8 *path);
void nv_delete_flag_file(const s8 *path);
bool nv_flag_file_isExist(const s8 *path);
u32 nv_data_writeback(void);
bool nv_read_right(void);
void nv_modify_print_sw(u32 arg);
s32 nv_modify_pm_sw(s32 arg);
u32 bsp_nvm_reload(void);
u32 bsp_nvm_buf_init(void);
u32 bsp_nvm_recovery(void);
u32 nv_clear_upgrade_flag(void);
u32 nv_img_store_head(void);
const struct nv_path_info_stru *nv_get_path_info(void);

#ifdef CONFIG_SHARED_MEMORY
unsigned long nv_shared_ddr_get(void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _NV_COMM_H_ */
