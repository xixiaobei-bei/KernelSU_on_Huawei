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

#ifndef __BSP_SEC_H
#define __BSP_SEC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ****************************************************************************
 * Attention                           *
 * *****************************************************************************
 * Description : Driver for secboot
 * Core        : Acore,Mcore,Fastboot
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/drivers/hisi/modem/drv/include/bsp_sec.h
 * : /mcore/xloader/include/bsp_sec.h
 * : /fastboot/include/bsp_sec.h
 * **************************************************************************** */
#define IMAGE_NAME_LEN 16
#define IMAGE_VER_LEN 32
#define IMAGE_RESERVE_LEN 44

struct image_head {
    char image_name[IMAGE_NAME_LEN];
    char image_version[IMAGE_VER_LEN];

    unsigned int image_index;
    unsigned int image_length;
    unsigned int load_addr;
    unsigned int crc;
    unsigned int is_sec;
    unsigned int is_compressed;

    char reserved[IMAGE_RESERVE_LEN];
    unsigned int xloader_image_version;
    unsigned int xloader_dbg_cert_size;
    unsigned int xloader_image_size;
};

#define SECBOOT_VRL_TABLE_SIZE (4 * 1024)
#define BALONG_IMAGE_HEAG_VRL_OFFSET (SECBOOT_VRL_TABLE_SIZE - 128)

#ifdef __cplusplus
}
#endif

#endif
