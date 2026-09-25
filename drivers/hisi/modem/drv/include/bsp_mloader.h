/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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

#ifdef __cplusplus /* __cplusplus */
extern "C"
{
#endif /* __cplusplus */

#include <osl_common.h>
#include <bsp_icc.h>
#include <product_config.h>

/*****************************************************************************
     *                           Attention                           *
******************************************************************************
* Description : Driver for mloader
* Core        : Acore銆丆core
* Header File : the following head files need to be modified at the same time
*             : /acore/kernel/drivers/hisi/modem/drv/include/bsp_mloader.h
*             : /ccore/include/ccpu/bsp_mloader.h
*             : /ccore/include/fusion/bsp_mloader.h
******************************************************************************/

#define MLOADER_L2HAC_IMG     "modem_l2hac.bin"
#define MLOADER_LR_CCPU_IMG   "modem_lr_ccpu.bin"
#define MLOADER_NR_CCPU_IMG   "modem_nr_ccpu.bin"
#define MLOADER_LR_SDR_IMG    "modem_lr_sdr.bin"
#define MLOADER_NR_SDR_IMG    "modem_nr_sdr.bin"
#define MLOADER_NR_LL1C_IMG   "modem_nr_ll1c.bin"
#define MLOADER_NR_PHY_IMG    "modem_nr_phy.bin"
#define MLOADER_LR_PHY_IMG    "modem_lr_phy.bin"
#define MLOADER_DTS_IMG       "modem_dt.img"

#define MLOADER_OP_LOAD_IMAGE       (1)
#define MLOADER_OP_GET_IMAGE_SIZE   (2)
#define MLOADER_OP_VERIFY_IMAGE     (3)
#define MODEM_IMAGE_OFFSET_FOR_5G   (0x40)

#define MLOADER_SECBOOT_BUFLEN (0x100000)
#define MODEM_IMAGE_NAME_LEN_MAX_NO_PATH 32

typedef struct{
    char        name[MODEM_IMAGE_NAME_LEN_MAX_NO_PATH];
    u32         op;
    u32         channel_id;
    u32         img_idx;
    u32         ddr_addr;
    u32         ddr_size;
    u32         image_addr;
    u32         image_size;
    u32         image_offset;
    u32         request_id;
    u32         time_stamp;
    u32         core_id;
    u32         cmd_type;
} MLOADER_IMG_ICC_INFO_S;

typedef struct{
    u32         op;
    u32         img_idx;
    int         result;
    u32         image_addr;
    u32         image_size;
    u32         request_id;
    u32         time_stamp;
} MLOADER_IMG_ICC_STATUS_S;

typedef s32 (*mloader_cb_func)(void* run_addr, u32 ddr_size);

/*****************************************************************************
     *                        Attention   END                        *
*****************************************************************************/

int bsp_mloader_load_image(const char* file_name, MLOADER_IMG_ICC_INFO_S* mloader_msg, u32 index);

/* 加载所有modem镜像，初始化和modem单独复位时使用 */
int bsp_load_modem_images(void);
int bsp_mloader_load_reset(void);

int mloader_verify_modem_image(unsigned core_id);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */
#endif
