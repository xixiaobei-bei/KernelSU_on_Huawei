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

#ifndef __BSP_VERSION_H__
#define __BSP_VERSION_H__

#include <osl_types.h>
#include <mdrv_version.h>
#include <bsp_print.h>
#include <bsp_shared_ddr.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VER_ERROR
#define VER_ERROR (-1)
#endif

#ifndef VER_OK
#define VER_OK 0
#endif

#define ver_print_error(fmt, ...)   (bsp_err("<%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

#define HW_VER_INVALID				(BSP_U32)0XFFFFFFFF
#define HW_VER_UDP_MASK				(BSP_U32)0XFF000000 	    /* MBB UDP单板掩码 */
#define HW_VER_UDP_UNMASK			(BSP_U32)(~HW_VER_UDP_MASK) /* MBB UDP掩码取反 */
#define MODEMID_VENDOR_MASK			(~(BSP_U32)0x3FF)           /* PHONE UDP单板掩码 */

#define HW_VER_HIONE_UDP_MAGIC      (BSP_U32)0X13245768

#define HW_VER_V765_UDP             (BSP_U32)0X76000000 /* V765 UDP */
#define HW_VER_V5000_UDP            (BSP_U32)0X77000000 /* V5000 UDP */
#define HW_VER_V5000_EMDM_UDP       (BSP_U32)0X37000000 /* V5000 extra modem UDP */
#define HW_VER_P533                 (BSP_U32)0XFE000000 /* P533 */
#define HW_VER_P535                 (BSP_U32)0XFD000000 /* P535 */

#define VERSION_MODULE_MAGIC        (0x2017)

typedef enum {
    CHIP_K970 = 0x3670,
    CHIP_K980 = 0x3680,
    CHIP_K990 = 0x3690,
    CHIP_BALTIMORE = 0x36A0,
    CHIP_K670 = 0x6260,
    CHIP_K680 = 0x6280,
    CHIP_DENVER = 0x6290,
    /* MBB */
    CHIP_P533  = 0x0533,
    CHIP_P535  = 0x0535,
    CHIP_V765  = 0x6965,
    CHIP_V5000 = 0x9500,
} version_chip_type_e;

typedef enum {
    PLAT_ASIC = 0x0,
    PLAT_ESL = 0x1,
    PLAT_HYBRID = 0x2,
    PLAT_EDA = 0x3,
    PLAT_FPGA = 0xa,
    PLAT_EMU = 0xe
} version_plat_type_e;

typedef enum {
    BSP_BOARD_TYPE_BBIT = 0,
    BSP_BOARD_TYPE_SFT,
    BSP_BOARD_TYPE_ASIC,
    BSP_BOARD_TYPE_SOC,
    BSP_BOARD_TYPE_PORTING,
    BSP_BOARD_TYPE_ESL,
    BSP_BOARD_TYPE_MAX
} version_board_type_e;

typedef enum {
    PRODUCT_MBB = 0x0,
    PRODUCT_PHONE = 0x1,
    PRODUCT_ERROR = 0x2
} version_product_type_e;
/* real values of reg for ESL and ESL_EMU are 0x000 and 0x010, respectively */
typedef enum {
    TYPE_ES = 0x1,
    TYPE_CS = 0x2,
    TYPE_V110 = 0x3,
    TYPE_ESL = 0x10,
    TYPE_ESL_EMU = 0x11,
    TYPE_ERR = 0xFF
} version_cses_type_e;

typedef enum {
    PLAT_INFO_ASIC = 0x0,
    PLAT_INFO_ESL_VDK = 0x1,
    PLAT_INFO_ESL_CANDENCE = 0x2,
    PLAT_INFO_ESL_SELF = 0x3,
    PLAT_INFO_EMU_ZEBU = 0x4,
    PLAT_INFO_EMU_Z1 = 0x5,
    PLAT_INFO_HYBRID_VDK_ZEBU = 0x6,
    PLAT_INFO_HYBRID_CANDENCE_Z1 = 0x7,
    PLAT_INFO_EDA = 0x8,
    PLAT_INFO_ERROR = 0xff
} version_plat_info_e;

typedef enum {
    TYPE_SINGLE_MDM = 0x001,
} version_single_mdm_e;

typedef enum {
    TYPE_NOT_ATE_SLT = 0x0,
    TYPE_SLT = 0x1,
    TYPE_ATE = 0x2,
    TYPE_ATE_UDP = 0x3, // for udp test
} version_ate_slt_type_e;

typedef struct {
    u32 product_id;                           /* 产品版本号，通过hkadc读取。NV和dts在用 */
    u32 product_id_udp_masked;    /* 屏蔽扣板信息的产品版本号。ioshare在用 */
    u32 chip_version;                       /* 芯片版本号 */
    u16 chip_type;                            /* 芯片类型，如CHIP_V5000=0x9500 */
    u8 plat_type;                              /* 平台类型，如asic/proting/emu */
    u8 plat_info;                              /* 细分平台类型 */
    u8 cses_type;                            /* 从boston起，芯片第一版逻辑为es(100)，第二版为cs(110) */
    u8 board_type;                          /* 平台类型，如BBIT SOC ASIC SFT */
    u8 base_board_id;                              /* 底板版本号 */
    u8 product_type;                       /* MBB or PHONE */
    u32 udp_flag;                           /* udp or phone,if udp = 0x13245768 */
    u16 version_magic;                  /* 0x2017 */
    u8 ate_slt_type;                    /* 用于识别ate/slt版本 */
    u8 reserve;
} bsp_version_info_s;

#define PRODUCT_ID_UDP_MASK 0x0000FC00
#define PRODUCT_ID_MASK 0xFFFFFFF0

enum {
    PRODUCTID_MSG_INIT = 0,
    PRODUCTID_MSG_REQUEST,
    PRODUCTID_MSG_UPDATE
};

struct productid_pcie_msg {
    u32 product_id;
    u32 msg_type;
};

/*
 * 以下提供给version_balong.c(a/c)
 */
#define VERSION_MAX_LEN 64

typedef struct {
    u32 product_id;                 /* 产品版本号NV和dts在用 */
    u32 boardid;                    /* 用于区分UDP和PHONE */
} version_sec_info_s;

/*
 * 以下提供给version.c(hiboot)
 */
typedef struct {
    u16 vol_low;
    u16 vol_high;
} voltage_range;

#define CHIP_TYPE_MASK 0xffff0000
#define PLAT_TYPE_MASK 0x0000f000
#define CSES_TYPE_MASK 0x00000fff
#define CSES_TYPE_MASK1 0x00000f00
#define PLAT_INFO_MASK 0x0000000f


/*
 * 以下提供给adp_version.c(a/c)
 */

#ifndef VER_MAX_LENGTH
#define VER_MAX_LENGTH  30
#endif

typedef struct {
    unsigned char comp_id;                            /* 组件号：参见COMP_TYPE */
    unsigned char comp_ver[VER_MAX_LENGTH + 1];         /* 最大版本长度 30 字符+ \0 */
} version_info_s;

/*
 * 以下提供给virtual boardid功能
 */
#define VIRTUAL_PRODUCTID_SET_OK      0x12345000
#define VIRTUAL_PRODUCTID_NO_SET      0x12345001
#define VIRTUAL_PRODUCTID_CMD_NULL    0x12345002
#define VIRTUAL_PRODUCTID_ERR_FORMAT  0x12345003
#define VIRTUAL_PRODUCTID_NV_NOBURN   0x12345004
#define VIRTUAL_PRODUCTID_SET_FLAG    0x12345005

#define MISC_VERSION_OFFSET 100

typedef struct  {
    unsigned err_code;
    unsigned virtual_productid;
    unsigned timestamp;
    unsigned set_ok_flag;
} misc_ptn_version_info;

typedef enum {
    VIRTUAL_PRODUCTID_MISC_OK = 0,
    VIRTUAL_PRODUCTID_MISC_ERROR,
} virtual_productid_misc_return_type;

/*
 * 以下为对外头文件声明
 */
char* bsp_version_get_hardware(void);
char* bsp_version_get_product_inner_name(void);
char* bsp_version_get_product_out_name(void);
char* bsp_version_get_build_date_time(void);
char* bsp_version_get_chip(void);
char* bsp_version_get_firmware(void);
char* bsp_version_get_release(void);
u32 bsp_version_hw_sub_id(void);
int bsp_version_acore_init1(void);
int bsp_version_acore_init(void);
int bsp_version_ccore_init(void);
int bsp_version_ddr_init(void);
void mdrv_ver_init(void);

const bsp_version_info_s* bsp_get_version_info(void);
const bsp_version_info_s* bsp_get_version_info_early(void);

int bsp_version_debug(void);

int bsp_version_update_productid(void);

#ifdef __cplusplus
}
#endif

#endif

