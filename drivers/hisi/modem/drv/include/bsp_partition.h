/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __BSP_PARTITION_H__
#define __BSP_PARTITION_H__
#include <product_config.h>
#include <osl_types.h>
#include <linux/mtd/mtd.h>

#define PTABLE_END_STR                  "end"
#define PTABLE_NAND_OFFSET              FLASH_PTABLE_OFFSET

#define PART_MAGIC                      12345678
#define PART_NAMELEN                    16
#define PRODUCT_NAMELEN                 8
#define TOTAL_PART_NUM                  128
#define NAND_PTABLE_INFO_LEN            16
#define NAND_PTABLE_HEAD_LEN            sizeof(struct ptable_head)
#define NAND_PTABLE_PART_LEN            sizeof(struct partition)
#define NAND_PTABLE_SECOND_PART_LEN     sizeof(struct second_partition)
#define NAND_PTABLE_PART_OFFSET         (NAND_PTABLE_INFO_LEN + NAND_PTABLE_HEAD_LEN)

#define PTABLE_PART_FLAG_FS             1
#define PTABLE_PART_FLAG_PROTECT        2
#define PTABLE_PART_FLAG_SECOND         8

#ifndef PTABLE_VERSION_ID
#define PTABLE_VERSION_ID 1
#endif

struct ptable_head {
    char product_name[PRODUCT_NAMELEN]; 
    unsigned int part_num;
    unsigned int second_part_num;
    unsigned int chip_size;
    unsigned int property;
    unsigned int reserved[0x2];
};

struct partition {
    char name[PART_NAMELEN];
    unsigned int start;
    unsigned int length;
    unsigned int flags;
    unsigned int reserved;
};

struct second_partition {
    char name[PART_NAMELEN];
    char father[PART_NAMELEN];
    unsigned int length;
    unsigned int reserved;
};

struct partition_head {
    unsigned int magic_num;
    unsigned int version_id;
    unsigned int reserved[0x2];
    struct ptable_head head;
    struct partition *part;
    struct second_partition *second_part;
};

/* partition name define, must be less than 16 characters */
#define PTABLE_GPT_NM                   "ptable"
#define PTABLE_M3BOOT_NM                "m3boot"
#define PTABLE_HIBOOT_NM              "hiboot"
#define PTABLE_NVDEFAULT_NM             "nvdefault"
#define PTABLE_NVBACK_NM                "nvbacklte"
#define PTABLE_NVIMG_NM                 "nvimg"
#define PTABLE_NVDLOAD_NM               "nvdload"
#define PTABLE_NVCUST_NM                "nvcust"
#define PTABLE_NVA_NM                   "nva"
#define PTABLE_NANDMONITOR_NM           "nandmonitor"
#define PTABLE_UCE_NM                   "uce"
#define PTABLE_HACLOAD_NM               "hacload"
#define PTABLE_SEC_STORAGE_NM           "sec_storage"
#define PTABLE_OEMINFO_NM               "oeminfo"
#define PTABLE_USERDATA_NM              "userdata"
#define PTABLE_ONLINE_NM                "online"
#define PTABLE_KERNEL_NM                "kernel"
#define PTABLE_KERNELBK_NM              "kernelbk"
#define PTABLE_LOGO_NM                  "logo"
#define PTABLE_M3IMG_NM                 "m3image"
#define PTABLE_HIFI_NM                  "hifi"
#define PTABLE_MISC_NM                  "misc"
#define PTABLE_SYSTEM_NM                "system"
#define PTABLE_DTS_NM                   "dts"
#define PTABLE_KERNEL_DT_NM              "dt"
#define PTABLE_KERNEL_DTCUST_NM          "dtcust"
#define PTABLE_MODEM_DT_NM               "modem_dt"
#define PTABLE_MODEM_DTCUST_NM           "modem_dtcust"
#define PTABLE_MODEM_SECURE_NM           "modem_secure"
#define PTABLE_CDROMISO_NM              "cdromiso"
#define PTABLE_CACHE_NM                 "cache"
#define PTABLE_RECOVERYA_NM             "recovery-a"
#define PTABLE_RECOVERYB_NM             "recovery-b"
#define PTABLE_TEEOS_NM                 "teeos"
#define PTABLE_MODEM_FW_NM              "modem_fw"
#define PTABLE_TRUSTED_FW_NM            "trusted_fw"
#define PTABLE_UBIUSE_NM                "ubi_use"
#define PTABLE_DEBUG_CERT               "debug_cert"
#define PTABLE_SLT_TSP_NM               "slt_tsp"
#define PTABLE_SLT_TVP_NM               "slt_tvp"
#define PTABLE_SLT_HIFI_NM              "slt_hifi"
#define PTABLE_SLT_PDE_NM               "slt_pde"

int ptable_shared_memory_init(void);
struct partition_head *bsp_partition_get_partition_head(void);
struct partition *bsp_partition_get_partition_by_name(const char *name);
struct partition *bsp_partition_get_partition_by_addr(unsigned int addr);
int bsp_partition_get_partition_flags(const char *name);
int balong_add_mtd_partitions(struct mtd_info *mtd);

#endif
