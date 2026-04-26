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


#ifndef __PARAM_TO_SEC_OS_H__
#define __PARAM_TO_SEC_OS_H__

#include <osl_types.h>
#include <hi_efuse.h>
#include "product_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 这个枚举的修改需要同步修改一下几个地方
mbb tzdriver
vendor/hisi/system/kernel-4.14/drivers/hisi/tzdriver_hm/teek_client_id.h
mbb secos
vendor/hisi/system/secure_os/trustedcore_hm/prebuild/hm-teeos-release/headers/hm/TEE/tee_common.h(枚举已删除)
vendor/hisi/system/secure_os/trustedcore_hm/libs/hisi-platdrv/platform/balong/include/bsp_param_cfg.h
mbb、phone:ccore modem
vendor/hisi/modem/drv/acore/bootable/bootloader/legacy/modem/include/param_cfg_to_sec.h
vendor/hisi/modem/drv/acore/kernel/drivers/hisi/modem/drv/include/param_cfg_to_sec.h
vendor/hisi/modem/drv/ccore/include/fusion/param_cfg_to_sec.h
vendor/hisi/modem/drv/ccore/include/ccpu/param_cfg_to_sec.h
vendor/hisi/modem/drv/fastboot/include/param_cfg_to_sec.h

phone tzdriver
vendor/hisi/ap/kernel/drivers/tzdriver/teek_client_id.h
phone secos
vendor/thirdparty/secure_os/trustedcore_hm/prebuild/hm-teeos-release/headers/hm/TEE/tee_common.h(枚举已删除)
vendor/thirdparty/secure_os/trustedcore_hm/libs/hisi-platdrv/platform/kirin/secureboot/secboot.h
*/
#ifdef BSP_CONFIG_PHONE_TYPE
enum SVC_SECBOOT_IMG_TYPE {
    MODEM = 0,
    MODEM_LPMCU,
    XDSP,
    TAS,
    WAS,
    MODEM_COMM_IMG = 5,
    MODEM_DTB,
    NVM,
    NVM_S,
    MBN_R,
    MBN_A = 10,
    MODEM_COLD_PATCH,
    DSP_COLD_PATCH,
    MODEM_CERT = 13,
    MODEM_DTO = 13,
    MAX_SOC_MODEM,
    HIFI,
    ISP,
    IVP,
    SOC_MAX
};
#else
enum SVC_SECBOOT_IMG_TYPE {
    MODEM = 0,
    HIFI,
    DSP,
    XDSP,
    TAS,
    WAS = 5,
    CAS,
    MODEM_DTB,
    ISP,
    NVM,
    NVM_S = 10,
    MBN_R,
    MBN_A,
    MODEM_COMM_IMG,
    MODEM_CERT = 14,
    MODEM_DTO = 14,
    SOC_MAX,
    MAX_SOC_MODEM = SOC_MAX
};
#endif


#define PARAM_MAGIC_OFFSET (0x0) /* 魔数:判断参数有效性 */
#define PARAM_CFG_OFFSET (0x8)   /* 配置参数存放基地址 */

#ifdef DDR_SECURE_OS_ADDR
/* 系统启动过程中传递C核镜像执行地址、镜像大小等参数到安全OS，
参数的存放地址由之前的安全共享内存转移到安全OS 的内存中，
安全OS为DX分配1MB的内存空间，位于安全OS内存起始地址偏移0x500000处，
hiboot使用DX 这1M内存空间的最后16KB存放需要传递到安全OS的参数，
16KB中最后的2KB用于存放参数的地址信息，剩余的14KB用于存放需要传递的参数 */
#define BALONG_PARAM_BASE_ADDR (DDR_SECURE_OS_ADDR + 0x600000 - 0x800) /* 0xA07FF800--0xA07FF8FF */
#define BALONG_SEC_NV_BASE_ADDR (DDR_SECURE_OS_ADDR + 0x600000 - 0x1000)
#define SECBOOT_PARAM_CFG_BASE_ADDR (DDR_SECURE_OS_ADDR + 0x600000 - 0x4000)
#define SECBOOT_PARAM_CFG_SIZE 0x3000
#endif

/* sec boot start */
struct image_info {
    u64 ddr_addr;
    unsigned int ddr_size;
    unsigned int unreset_dependcore;
};

struct dynamic_load {
    u32 load_cmd;
    u32 verify_flag;
};

struct verify_param_info {
    u32 vrl_addr;
    u32 cmd_type;
    u32 image_addr;
    u32 image_size;
    u32 patch_addr;
    u32 patch_size;
    u32 splicing_addr;
    u32 splicing_size;
    u32 deflate_addr;
    u32 deflate_size;
    u32 verify_flag;
    u32 image_id;
};

struct secboot_verify_param {
    u32 core_idx;
    struct verify_param_info verify_param_info[0x2];
};

#define MODEM_NUM 3
struct sec_rnd_info {
    unsigned int image_offset[MODEM_NUM];
    unsigned int stack_guard[MODEM_NUM];
    unsigned int heap_offset[MODEM_NUM];
};

#ifdef FEATURE_KEEP_LEGICY_SEC_OS
struct sec_boot_modem_info {
    struct image_info image_info[SOC_MAX];
    struct dynamic_load dynamic_load;
    u32 aslr_flag;
    struct secboot_verify_param verify_param;
    struct sec_rnd_info sec_rnd_info;
};
#else
struct sec_boot_modem_info {
    struct dynamic_load dynamic_load;
    struct image_info image_info[MAX_SOC_MODEM];
    u32 aslr_flag;
    struct secboot_verify_param verify_param;
    struct sec_rnd_info sec_rnd_info;
};
#endif
/* sec boot end */
#define SHARE_TRNG_LENGTH (96)
typedef struct tag_PARAM_CFG {
    u32 magic;                /* 魔数，标识配置参数的状态 */
    u32 protect_barrier_size; /* 预留(4K)防止被踩，初始化为全F */
    u32 param_cfg_size;       /* 配置参数预留(16K)大小 */
    u64 icc_channel_base_addr;
    u32 icc_channel_max_size;
    struct sec_boot_modem_info sec_boot_modem_info;
    unsigned char trng_buf[SHARE_TRNG_LENGTH];
    u32 efuse_val[EFUSE_MAX_SIZE];
} g_param_cfg;

void set_param_cfg_to_sec(void);

void set_modem_info_to_sec(void);

#ifdef __cplusplus
}
#endif

#endif /* __PARAM_TO_SEC_OS_H__ */
