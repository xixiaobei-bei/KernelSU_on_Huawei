/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
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

#ifndef _BSP_MLOADER_H_
#define _BSP_MLOADER_H_

#include <product_config.h>
#include <osl_common.h>
#ifndef CONFIG_MODULE_IPC_FUSION
#include <bsp_icc.h>
#endif
#include <param_cfg_to_sec.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ****************************************************************************
 * Attention                           *
 * *****************************************************************************
 * Description : Driver for mloader
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/drivers/hisi/modem/drv/include/bsp_mloader.h
 * : /ccore/include/ccpu/bsp_mloader.h
 * : /ccore/include/fusion/bsp_mloader.h
 * **************************************************************************** */

#define MLOADER_L2HAC_IMG "modem_l2hac.bin"
#define MLOADER_NR_CCPU_IMG "modem_nr_ccpu.bin"
#define MLOADER_LR_SDR_IMG "modem_lr_sdr.bin"
#define MLOADER_NR_SDR_IMG "modem_nr_sdr.bin"
#define MLOADER_NR_LL1C_IMG "modem_nr_ll1c.bin"
#define MLOADER_NR_PHY_IMG "modem_nr_phy.bin"
#define MLOADER_LR_PHY_IMG "modem_lr_phy.bin"
#define MLOADER_DTS_IMG "modem_dt.img"

#define MLOADER_OP_VERIFY_IMAGE (3)
#define MLOADER_OP_GET_PATCH_STATUS (4)
#define MLOADER_OP_LOAD_FINISHED (5)
#define MLOADER_OP_HOT_PATCH_LOADED (6)
#define MLOADER_OP_GET_HOT_PATCH_INFO (7)

#define MODEM_IMAGE_OFFSET_FOR_5G (0x40)

#define MLOADER_PATCH_LOAD_FAIL 1
#define MLOADER_PATCH_LOAD_SUCCESS 0

#define MLOADER_SECBOOT_BUFLEN (0x100000UL)
#define MODEM_IMAGE_NAME_LEN_MAX_NO_PATH 32

typedef struct {
    char name[MODEM_IMAGE_NAME_LEN_MAX_NO_PATH];
    u32 op;
    u32 channel_id;
    u32 img_idx;
    u32 ddr_addr;
    u32 ddr_size;
    u32 image_addr;
    u32 image_size;
    u32 image_offset;
    u32 request_id;
    u32 time_stamp;
    u32 core_id;
    u32 cmd_type;
} mloader_img_icc_info_s;

typedef struct {
    u32 op;
    u32 img_idx;
    int result;
    u32 image_addr;
    u32 image_size;
    u32 request_id;
    u32 time_stamp;
} mloader_img_icc_status_s;

typedef enum {
    BEFORE_LR_LOAD = 0,
    AFTER_LR_LOAD = 1,
    BEFORE_NR_LOAD = 2,
    AFTER_NR_LOAD = 3,
    AFTER_ALL_IMAGE_LOAD = 4,
    LOAD_MAX,
} load_stage_e;


typedef enum {
    MLOADER_NORMAL_LOAD = 0,
    MLOADER_FS_LOAD = 1,
    MLOADER_BACK_LOAD = 2,
    MLOADER_BUTT
} mloader_load_e;

typedef void (*mloader_load_notifier_funcptr)(load_stage_e);

typedef struct {
    struct list_head list;
    mloader_load_notifier_funcptr func;
} mloader_load_notifier_s;

typedef s32 (*mloader_cb_func)(void *run_addr, u32 ddr_size);

/* ****************************************************************************
 * Attention   END                        *
 * *************************************************************************** */
int bsp_load_modem_images(void);
int bsp_mloader_load_reset(void);
/*
注意!!!!!!!!!!!!!
bsp_mloader_load_notifier_register 使用约束
1、分配好mloader_load_notifier_s 空间后，传入全局变量，否则功能会异常；
2、如果有的阶段不需要做处理，不能返回error；
3、不能阻塞流程往下走；
*/
int bsp_mloader_load_notifier_register(mloader_load_notifier_s *load_notifier);
int mloader_verify_modem_image(unsigned core_id);
/* 加载镜像（包括VRL头+镜像内容） */
int bsp_mloader_load_image(char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype, u32 run_addr, u32 ddr_size);
/* 只加载镜像实际内容 */
int bsp_mloader_load_single_image(const char *file_name, u32 offset, u32 size, enum SVC_SECBOOT_IMG_TYPE ecoretype,
    u32 run_addr, u32 ddr_size);
/* 只加载VRL头 */
int bsp_mloader_load_single_vrl(const char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype);
int bsp_mloader_verify_single_image(const char *file_name, enum SVC_SECBOOT_IMG_TYPE ecoretype);

#ifdef __cplusplus
}
#endif
#endif
