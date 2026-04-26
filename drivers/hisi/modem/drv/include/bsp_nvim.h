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

#ifndef __BSP_NVIM_H__
#define __BSP_NVIM_H__

#include <product_config.h>
#include <drv_comm.h>
#include <mdrv_rfile.h>
#include <bsp_nvim_mem.h>
#include <osl_common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define BSP_ERR_NV_BASE BSP_DEF_ERR(15, 0)

#define NV_GLOBAL_CTRL_INFO_SIZE (sizeof(nv_ctrl_info_s))

#define NV_GLOBAL_FILE_LIST_ADDR (NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE)
#define NV_GLOBAL_FILE_ELEMENT_SIZE (sizeof(nv_file_info_s))

#define NV_REF_LIST_ITEM_SIZE (sizeof(nv_item_info_s))

#define NV_MBN_NV_SIZE ((unsigned long)1024 * 128)
#define NV_MBN_NV_ADDR ((uintptr_t)SHM_MEM_NV_ADDR + (SHM_MEM_NV_SIZE - NV_MBN_NV_SIZE))

#define NV_MAX_FILE_SIZE (SHM_MEM_NV_SIZE - (NV_GLOBAL_INFO_SIZE + NV_MBN_NV_SIZE))

#ifndef NV_OK
#define NV_OK (0)
#endif

#ifndef NV_ERROR
#define NV_ERROR 0xffffffff
#endif
/*lint -esym(652,*)*/
#ifndef FILE
#define FILE void
#endif
/*lint +esym(652,*)*/
#ifndef NULL
#define NULL (void *)0
#endif


/* 外部组件读取双卡双待数据sim卡数据使用一下宏定义 */
#ifndef MODEM_ID_0
#define MODEM_ID_0 0 /* 卡1参数 */
#endif

#ifndef MODEM_ID_1
#define MODEM_ID_1 1 /* 卡2参数 */
#endif

#ifndef MODEM_ID_2
#define MODEM_ID_2 2 /* 卡2参数 */
#endif

#ifndef MODEM_ID_BUTT
#define MODEM_ID_BUTT 3
#endif

enum {
    NV_USIMM_CARD_1 = 1,
    NV_USIMM_CARD_2 = 2,
    NV_USIMM_CARD_3 = 3
};

/* init state */
enum {
    NV_BOOT_INIT_FAIL = 0,
    NV_INIT_FAIL,
    NV_KERNEL_INIT_DOING,
    NV_BOOT_INIT_OK,
    NV_INIT_OK
};


/* file ops */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif


#define NV_FILE_USING 0x766e /* nv */
#define NV_FILE_UPDATA 0x34BA77

/* 内存数据类型 */
#define NV_MEM_DLOAD 0x12345678
#define NV_MEM_DEFAULT 0xabcde123
#define NV_MEM_BACKUP 0xdce582e1
#define NV_MEM_COLDPATCH 0x1234abcd
#define NV_MEM_UPGRADE_SUCCESS 0xa1b2c3d4

#define NV_CTRL_FILE_MAGIC_NUM 0x224e4944     /* 文件头标志位 */
#define NV_BOOT_UPGRADE_SUCC_STATE 0x5687dcfe /* hiboot 阶段升级是否成功标志 */
#define NV_FILE_TAIL_MAGIC_NUM 0xabcd8765     /* 文件尾标志 */

#define NV_IMEI_LEN 15

#define NV_FILE_READ "rb"
#define NV_FILE_RW "rb+"
#define NV_FILE_WRITE "wb+"

#define B_READ (0)
/*lint --e{607}*/
/*lint -restore*/
#define NV_FILE_MAX_NUM 20
#define NV_MAX_UNIT_SIZE 2048

#define NV_FILE_SYS_NV_PATH "/system/sys_nv.bin"
#define NV_BACK_PATH "/system/nvbak.bin"
#define NV_DLOAD_PATH "/system/nvdload.bin"
#define NV_DLOAD_CUST_PATH "/system/nvdload_cust.bin"
#define NV_XNV_CARD1_PATH "/system/xnv_card1.xml"
#define NV_XNV_CARD1_MAP_PATH "/system/xnv_card1.map"
#define NV_XNV_CARD2_PATH "/system/xnv_card2.xml"
#define NV_XNV_CARD2_MAP_PATH "/system/xnv_card2.map"
#define NV_XNV_CARD3_PATH "/system/xnv_card3.xml"
#define NV_XNV_CARD3_MAP_PATH "/system/xnv_card3.map"
#define NV_CUST_CARD1_PATH "/system/cust_card1.xml"
#define NV_CUST_CARD2_PATH "/system/cust_card2.xml"
#define NV_CUST_CARD3_PATH "/system/cust_card2.xml"
#define NV_DEFAULT_PATH "/system/manufactrue_ver.bin"
#define NV_DATA_ROOT_PATH "/mnt/modem/mnvm2:0"

#define NV_COLD_PATCH_PATH "/system/nvcoldpatch.bin"
#define NV_COLD_RPATCH_PATH "/system/nvcoldrestore.bin"

#ifdef BSP_CONFIG_PHONE_TYPE
#define NV_IMG_PATH "/modem_nv/nv.bin"
#define NV_IMG_FLAG_PATH (MODEM_LOG_ROOT "/drv/nv/img_flag.bin")
#define NV_BACK_FLAG_PATH (MODEM_LOG_ROOT "/drv/nv/back_flag.bin")
#define NV_SYS_FLAG_PATH (MODEM_LOG_ROOT "/drv/nv/sys_flag.bin")


#define NV_LOG_MAX_SIZE 0x80000 /* 512k */
#else
#define NV_IMG_PATH "/nv.bin"
#define NV_IMG_FLAG_PATH (MODEM_LOG_ROOT "/drv/nv/img_flag.bin")
#define NV_BACK_FLAG_PATH (MODEM_LOG_ROOT "/drv/nv/back_flag.bin")
#define NV_SYS_FLAG_PATH (MODEM_LOG_ROOT "/drv/nv/sys_flag.bin")

#define NV_LOG_MAX_SIZE 0x4000 /* 16k */
#endif

#define NV_LOG_PATH (MODEM_LOG_ROOT "/drv/nv/NvLog.txt")

/* nv image file */
#define NVM_IMG_BIN 0x5a5a5a5a
#define NVM_IMG_BAK 0x7a7a7a7a
#ifdef BSP_CONFIG_PHONE_TYPE
#define NV_MBN_PATH "/mbn_nv/carrier.bin"
#else
#ifdef CONFIG_NV_AB_FEATURE
#define NV_MBN_PATH "/mbn_nv/carrier.bin"
#else
#define NV_MBN_PATH "/mbn_nv/sec_carrier.bin"
#endif
#endif
#ifdef FEATURE_NV_CARRIER_CUST
#define NV_MBN_COMM_PATH "/mbn_nv/comm.mbn"
#else
#define NV_MBN_COMM_PATH "/mbn_nv/comm.bin"
#endif

#define NV_IMG_HEAD_PATH "/modem_nv/nv_ctrl.bin"
#define NV_IMG_RESUM_PATH "/modem_nv/nv_resum.bin"
#define NV_IMG_RDONLY_PATH "/modem_nv/nv_rdonly.bin"
#define NV_IMG_RDWR_PATH "/modem_nv/nv_rdwr.bin"
#define NV_IMG_RDWR_PATH1 "/modem_nv/nv_rdwr.bk"
#define NV_IMG_FLAG_SAVE_PATH "/modem_nv/nv_flag.txt"

enum {
    NV_IMG = 0,    /* nv.bin                         */
    NV_MBN,        /* carrier.bin or sec_carrier.bin */
    NV_MBN_COMM,   /* comm.bin or comm.mbn */
    NV_IMG_HEAD,   /* nv_ctrl.bin */
    NV_IMG_RESUM,  /* nv_resume.bin */
    NV_IMG_RDONLY, /* nv_rdonly.bin */
    NV_IMG_RDWR,   /* nv_rdwr.bin */
    NV_IMG_RDWR1,  /* nv_rdwr.bk */
    NV_IMG_FLAG,   /* nv_flag.txt */
    NV_MAX,
};

#define NV_FILE_NAME_MAXLEN 128
struct nv_img_info {
    char *root_dir;
    s8 file[NV_MAX][NV_FILE_NAME_MAXLEN];
};

/* error code */
#define BSP_ERR_NV_MEM_INIT_FAIL (BSP_ERR_NV_BASE + 0x1)
#define BSP_ERR_NV_INVALID_PARAM (BSP_ERR_NV_BASE + 0x2)
#define BSP_ERR_NV_MALLOC_FAIL (BSP_ERR_NV_BASE + 0x3)
#define BSP_ERR_NV_ERASER_FAIL (BSP_ERR_NV_BASE + 0x4)
#define BSP_ERR_NV_FREE_BUF_FAIL (BSP_ERR_NV_BASE + 0x5)
#define BSP_ERR_NV_NO_FILE (BSP_ERR_NV_BASE + 0x6)
#define BSP_ERR_NV_READ_FILE_FAIL (BSP_ERR_NV_BASE + 0x7)
#define BSP_ERR_NV_WRITE_FILE_FAIL (BSP_ERR_NV_BASE + 0x8)
#define BSP_ERR_NV_FTELL_FILE_FAIL (BSP_ERR_NV_BASE + 0x9)
#define BSP_ERR_NV_SEEK_FILE_FAIL (BSP_ERR_NV_BASE + 0xa)
#define BSP_ERR_NV_REMOVE_FILE_FAIL (BSP_ERR_NV_BASE + 0xb)
#define BSP_ERR_NV_CLOSE_FILE_FAIL (BSP_ERR_NV_BASE + 0xc)
#define BSP_ERR_NV_READ_DATA_FAIL (BSP_ERR_NV_BASE + 0xd)
#define BSP_ERR_NV_BACKUP_FILE_FAIL (BSP_ERR_NV_BASE + 0xe)
#define BSP_ERR_NV_BACKUP_DATA_FAIL (BSP_ERR_NV_BASE + 0xf)
#define BSP_ERR_NV_UPDATE_FILE_FAIL (BSP_ERR_NV_BASE + 0x10)
#define BSP_ERR_NV_UPDATE_DATA_FAIL (BSP_ERR_NV_BASE + 0x11)
#define BSP_ERR_NV_SECTION_NOT_SUPPORT_UPDATE (BSP_ERR_NV_BASE + 0x12)
#define BSP_ERR_NV_NO_WRITE_RIGHT (BSP_ERR_NV_BASE + 0x13)
#define BSP_ERR_NV_WRITE_DATA_FAIL (BSP_ERR_NV_BASE + 0x14)
#define BSP_ERR_NV_FILE_ERROR (BSP_ERR_NV_BASE + 0x15)
#define BSP_ERR_NV_NO_THIS_ID (BSP_ERR_NV_BASE + 0x16)
#define BSP_ERR_NV_ICC_CHAN_ERR (BSP_ERR_NV_BASE + 0x17)
#define BSP_ERR_NV_ITEM_LEN_ERR (BSP_ERR_NV_BASE + 0x18)
#define BSP_ERR_NV_RECOVER_BAK_ERR (BSP_ERR_NV_BASE + 0x19)
#define BSP_ERR_NV_AUTH_FAIL (BSP_ERR_NV_BASE + 0x20)
#define BSP_ERR_NV_DDR_OPS_ERR (BSP_ERR_NV_BASE + 0x21)
#define BSP_ERR_NV_OVERTIME_WAIT (BSP_ERR_NV_BASE + 0x22)
#define BSP_ERR_NV_NAND_ALL_BAD (BSP_ERR_NV_BASE + 0x23)
#define BSP_ERR_NV_OPEN_FILE_FAIL (BSP_ERR_NV_BASE + 0x24)
#define BSP_ERR_NV_HEAD_MAGIC_ERR (BSP_ERR_NV_BASE + 0x25)
#define BSP_ERR_NV_DDR_NOT_ENOUGH_ERR (BSP_ERR_NV_BASE + 0x26)
#define BSP_ERR_NV_CRC_CODE_ERR (BSP_ERR_NV_BASE + 0x27)
#define BSP_ERR_NV_GET_NV_LEN_ERR (BSP_ERR_NV_BASE + 0x28)
#define BSP_ERR_NV_DELOAD_CHECK_ERR (BSP_ERR_NV_BASE + 0x29)
#define BSP_ERR_NV_MEMCPY_ERR (BSP_ERR_NV_BASE + 0x30)
#define BSP_ERR_NV_MEMSET_ERR (BSP_ERR_NV_BASE + 0x31)
#define BSP_ERR_NV_OVER_MEM_ERR (BSP_ERR_NV_BASE + 0x32)      /* 大小越界 */
#define BSP_ERR_NV_MEM_SIZE_ERR (BSP_ERR_NV_BASE + 0x33)      /* 大小不正确 */
#define BSP_ERR_NV_XML_CFG_ERR (BSP_ERR_NV_BASE + 0x34)       /* NV的xml配置不正确 */
#define BSP_ERR_NV_TIME_OUT_ERR (BSP_ERR_NV_BASE + 0x35)      /* 超时错误 */
#define BSP_ERR_NV_GIVE_IPC_ERR (BSP_ERR_NV_BASE + 0x36)      /* give ipc错误 */
#define BSP_ERR_NV_NOT_SUPPORT_ERR (BSP_ERR_NV_BASE + 0x37)   /* 不支持错误 */
#define BSP_ERR_NV_REG_DUMP_ERR (BSP_ERR_NV_BASE + 0x38)      /* 注册可维可测异常 */
#define BSP_ERR_NV_INVALID_MDMID_ERR (BSP_ERR_NV_BASE + 0x39) /* MODEMID参数错误 */
#define BSP_ERR_NV_BACUP_INVALID_ERR (BSP_ERR_NV_BASE + 0x40) /* 备份分区无效参数错误 */
#define BSP_ERR_NV_CRC_CTRL_ERR (BSP_ERR_NV_BASE + 0x41)
#define BSP_ERR_NV_CRC_RESUME_SUCC (BSP_ERR_NV_BASE + 0x42)
#define BSP_ERR_NV_SEM_CREATE_ERR (BSP_ERR_NV_BASE + 0x43)  /* 信号量创建错误 */
#define BSP_ERR_NV_RELEASE_SEM_ERR (BSP_ERR_NV_BASE + 0x44) /* 信号量释放错误 */
#define BSP_ERR_NV_CUST_PINFO_ERR (BSP_ERR_NV_BASE + 0x45)
#define BSP_ERR_NV_CUST_PMAP_ERR (BSP_ERR_NV_BASE + 0x46)
#define BSP_ERR_NV_CUST_SECINFO_ERR (BSP_ERR_NV_BASE + 0x47)
#define BSP_ERR_NV_CUST_DECPRS_ERR (BSP_ERR_NV_BASE + 0x48)
#define BSP_ERR_NV_CUST_SECMAGIC_ERR (BSP_ERR_NV_BASE + 0x49)
#define BSP_ERR_NV_CUST_MDMNUM_ERR (BSP_ERR_NV_BASE + 0x4A)
#define BSP_ERR_NV_CUST_NOXNV_ERR (BSP_ERR_NV_BASE + 0x4B)
#define BSP_ERR_NV_FILE_OVER_MEM_ERR (BSP_ERR_NV_BASE + 0x4C)
#define BSP_ERR_NV_NULL_EFUSE_ERR (BSP_ERR_NV_BASE + 0x4D)
#define BSP_ERR_NV_NO_COLDPATCH (BSP_ERR_NV_BASE + 0x4E)
#define BSP_ERR_NV_DATA_NULL (BSP_ERR_NV_BASE + 0x4F)
#define BSP_ERR_NV_INVALID_NVE_FIELD (BSP_ERR_NV_BASE + 0x50)
#define BSP_ERR_NV_WRITE_NVE_FAIL (BSP_ERR_NV_BASE + 0x51)
#define BSP_ERR_NV_READ_NVE_FAIL (BSP_ERR_NV_BASE + 0x52)
#define BSP_ERR_NV_OPEN_EICC_FAIL (BSP_ERR_NV_BASE + 0x53)
#define BSP_ERR_NV_READ_EICC_FAIL (BSP_ERR_NV_BASE + 0x54)
#define BSP_ERR_NV_INVALID_EICC_MSG_TYPE (BSP_ERR_NV_BASE + 0x55)
#define BSP_ERR_NV_INVALID_MEM_BUF (BSP_ERR_NV_BASE + 0x56)
#define BSP_ERR_NV_HAMC_CHECK_ERR (BSP_ERR_NV_BASE + 0x57)
#define BSP_ERR_NV_UPDATE_VERIFY_ERR (BSP_ERR_NV_BASE + 0x58)
#define BSP_ERR_NV_CUST_VERIFY_ERR (BSP_ERR_NV_BASE + 0x59)
#define BSP_ERR_NV_DATA_MAGICNUM_ERR (BSP_ERR_NV_BASE + 0x5A)
#define BSP_ERR_NV_ALL_UPDATE_ERR (BSP_ERR_NV_BASE + 0x5B)
#define BSP_ERR_NV_NO_UPDATE_ERR (BSP_ERR_NV_BASE + 0x5C)
#define BSP_ERR_NV_SEC_FILE_ERR (BSP_ERR_NV_BASE + 0x5D)
#define BSP_ERR_NV_GET_VERIFY_BUF (BSP_ERR_NV_BASE + 0x5E)
#define BSP_ERR_NV_PART_VERIFICATION (BSP_ERR_NV_BASE + 0x5F)
#define BSP_ERR_NV_PARTITION_INFO_ERR (BSP_ERR_NV_BASE + 0x60)
#define BSP_ERR_NV_GET_CUST_PRDTID_ERR (BSP_ERR_NV_BASE + 0x61)
#define BSP_ERR_NV_REVERT_RW_ERR (BSP_ERR_NV_BASE + 0x62)
#define BSP_ERR_NV_WRITE_BACK_ERR (BSP_ERR_NV_BASE + 0x63)
#define BSP_ERR_NV_CLEAR_UPGRADE_ERR (BSP_ERR_NV_BASE + 0x64)
#define BSP_ERR_NV_MEM_CHECK_ERR (BSP_ERR_NV_BASE + 0x65)
#define BSP_ERR_NV_GET_FILE_LEN_ERR (BSP_ERR_NV_BASE + 0x66)
#define BSP_ERR_NV_SEND_CPMSG_ERR (BSP_ERR_NV_BASE + 0x67)

#define NV_MID_PRI_LEVEL_NUM 7

#define NV_CTRL_DATA_CRC (1 << 0)  /* data段CRC校验标志         bit0 */
#define NV_CTRL_CTRL_CRC (1 << 1)  /* ctrl段CRC校验标志         bit1 */
#define NV_CTRL_ITEM_CRC (1 << 2)  /* 单nvid做CRC校验标志       bit2 */
#define NV_CTRL_MODEM_CRC (1 << 3) /* 单nv modem做CRC校验标志   bit3 */
#define NV_CTRL_COMPRESS (1 << 4)  /* NV定制数据是否进行压缩    bit4 */

enum {
    NV_MODE_FACTORY = 1,
    NV_MODE_USER = 2,
    NV_MODE_BUTT
};

/* priority type */
enum {
    NV_HIGH_PRIORITY = 0,
    NV_MID_PRIORITY1,
    NV_MID_PRIORITY2,
    NV_MID_PRIORITY3,
    NV_MID_PRIORITY4,
    NV_MID_PRIORITY5,
    NV_MID_PRIORITY6,
    NV_LOW_PRIORITY = 7,
    NV_BUTT_PRIORITY
};

enum {
    NV_FLAG_NO_CRC = 0,   /* 不进行CRC校验 */
    NV_FLAG_NEED_CRC = 1, /* 需要进行CRC校验 */
    NV_MARK_CRC_BUTTON,
};

#define NV_FILE_VER_LEN 2
#define NV_TIME_TAG_LEN 4
#define NV_PRDT_VER_LEN 8
/* NV Ctrl File Header Struct */
typedef struct nv_ctrl_file_info_stru {
    u32 magicnum;
    u32 ctrl_size;               /* ctrl file size */
    u8 version[NV_FILE_VER_LEN]; /* file version */
    u8 modem_num;                /* modem num */
    u8 crc_mark;
    u32 file_offset; /* Offset of the File list start address */
    u32 file_num;    /* File num at file list */
    u32 file_size;   /* File list size */
    u32 ref_offset;  /* Offset of the NV reference data start address */
    u32 ref_count;   /* NV ID num */
    u32 ref_size;    /* NV reference data size */
    u8 reserve2[0xC];
    u32 timetag[NV_TIME_TAG_LEN];         /* time stamp */
    u32 product_version[NV_PRDT_VER_LEN]; /* product version */
} nv_ctrl_info_s;

#define NV_SECTION_NAME_LEN 20
typedef struct nv_file_info_stru {
    u8 file_id;   /* NV File ID */
    u8 file_type; /* NV File type */
    u8 file_reserve[0x2];
    u8 file_name[NV_SECTION_NAME_LEN]; /* NV File Name */
    u32 file_nvnum;                    /* NV File nv all num */
    u32 file_offset;                   /* NV File offset */
    u32 file_size;                     /* NV File size */
} nv_file_info_s;

#define NV_MODEM_NUM_MAX 3
/* NV reference Data Struct */
typedef struct nv_item_info_stru {
    u32 itemid;                   /* NV ID */
    u32 nv_off[NV_MODEM_NUM_MAX]; /* NV OFFSET,modem0~2 */
    u16 nv_len;                   /* NV Length */
    u8 modem_num;                 /* NV modem num */
    u8 priority;                  /* NV priority */
    u8 resume;                    /* NV resume */
    u8 reserve[0x2];                /* reserve  */
    u8 file_id;                   /* NV file id */
} nv_item_info_s;

typedef struct nv_global_file_handle_stru {
    u32 file_id;
    u32 offset; /* compare with the ctrl file start addr */
    u32 size;
} nv_global_file_handle_s;

typedef struct nv_hiboot_debug_stru {
    u32 line;
    u32 ret;
    u32 reseverd1;
    u32 reseverd2;
} nv_hiboot_debug_s;

typedef struct nv_flush_info_stru {
    u16 itemid;  /* NV ID */
    u16 modemid; /* modem id */
} nv_flush_item_s;

typedef struct nv_flush_list_stru {
    u32 count;
    nv_flush_item_s list[0]; /*lint !e43*/
} nv_flush_list;

typedef union debug_ctrl_union {
    u32 value;
    struct ctrl_bits_stru {
        u32 write_print_ctrl : 1; /* 写NV过程是否打印,1 打印 0 不打印 */
        u32 reserved : 31;
    } ctrl_bits;
} debug_ctrl_union_t;

#pragma pack(push)
#pragma pack(4)
/* nv global ddr info, size < 1k */
/* 此结构体的为A核C核公用的结构体
请不要轻易添加或者删除结构体成员 */
enum _file_type {
    NV_FILE_HEAH = 0x0, // 不属于文件属性，只表示文件头
    NV_FILE_ATTRIBUTE_RESUM,
    NV_FILE_ATTRIBUTE_RDONLY,
    NV_FILE_ATTRIBUTE_RDWR,
    NV_FILE_ATTRIBUTE_MAX,
};

#define RSNV_FILE_BIT (1 << NV_FILE_ATTRIBUTE_RESUM)
#define RONV_FILE_BIT (1 << NV_FILE_ATTRIBUTE_RDONLY)
#define RWNV_FILE_BIT (1 << NV_FILE_ATTRIBUTE_RDWR)
#define ALL_FILE_MASK (RSNV_FILE_BIT | RONV_FILE_BIT | RWNV_FILE_BIT)

#define RSNV_FILE_MASK (~(1 << NV_FILE_ATTRIBUTE_RESUM))
#define RONV_FILE_MASK (~(1 << NV_FILE_ATTRIBUTE_RDONLY))
#define RWNV_FILE_MASK (~(1 << NV_FILE_ATTRIBUTE_RDWR))

struct nv_mem_buf_info {
    u32 is_need_flush_backup;
    u32 buf_size;
    unsigned long mem_buf_addr;
};
typedef struct nv_global_ddr_info_stru {
    u32 ddr_read;           /* whether ddr can to write #need delete */
    u32 nvdload_boot_state; /* nv upgrade state */
    u32 acore_init_state;   /* acore init state,after kernel init OK ,then start next step */
    u32 ccore_init_state;   /* ccore init state,only after acore kernel init ok or init all ok */
    u32 mcore_init_state;
    u32 mem_file_type;
    u32 priority; /* reg the priority sum */
    u32 flush_time;
    u32 flush_threshold;
    u32 flush_backnvm;
    u32 file_num;                                       /* file list file num */
    u32 file_len;                                       /* file toatl len,include ctrl file */
    u32 file_sign;                                      /* file sign status */
    debug_ctrl_union_t debug_ctrl;                      /* debug控制 */
    nv_global_file_handle_s file_info[NV_FILE_MAX_NUM]; /* reg every file size&offset */
    nv_hiboot_debug_s fb_debug;
    struct nv_mem_buf_info nminfo;
} nv_global_info_s;
#pragma pack(pop)

/* icc msg type */
enum {
    NV_ICC_REQ_SYS = 64,
    NV_ICC_REQ_INSTANT_FLUSH = 65,     /* 立即写入请求,需要等待回复 */
    NV_ICC_REQ_CCORE_DELAY_FLUSH = 66, /* ccore 延迟写入请求，不需要等待回复 */
    NV_REQ_ACORE_DELAY_FLUSH = 67,     /* acore 延迟写入请求，不需要等待回复 */
    NV_ICC_RESUME_ITEM = 68,
    NV_ICC_RESUME = 0xFF /* 请求恢复DDR中的nv数据 */
};

/* eicc message type between NR and LR */
enum {
    NV_EICC_REQ_SYNC_PROPERTY_ZEOR = 0,
    NV_EICC_REQ_ASYNC_PROPERTY_ONE = 1,
};

/* DDR中NV文件的长度 */
#define NV_FILE_LEN (((nv_global_info_s *)NV_GLOBAL_INFO_ADDR)->file_len)
#define NV_FILE_CTRL_SIZE (((nv_ctrl_info_s *)NV_GLOBAL_CTRL_INFO_ADDR)->ctrl_size)
#define NV_FILE_CTRL_CRC_MARK ((((nv_ctrl_info_s *)NV_GLOBAL_CTRL_INFO_ADDR)->crc_mark) & NV_CTRL_CTRL_CRC)
#define NV_FILE_CRC_MARK ((((nv_ctrl_info_s *)NV_GLOBAL_CTRL_INFO_ADDR))->crc_mark)

u32 nv_readEx(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);
u32 nv_writeEx(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);

u32 nv_readEx_partition(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen, const s8 *path);
u32 nv_readEx_img(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);
u32 nv_readEx_factory(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);

u32 bsp_nvm_dcread(u32 modem_id, u32 itemid, u8 *pdata, u32 datalen);
u32 bsp_nvm_dcwrite(u32 modem_id, u32 itemid, u8 *pdata, u32 datalen);

u32 bsp_nvm_dcreadpart(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);
u32 bsp_nvm_dcwritepart(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);

u32 bsp_nvm_dcread_direct(u32 modem_id, u32 itemid, u8 *pdata, u32 datalen);
u32 bsp_nvm_dcwrite_direct(u32 modem_id, u32 itemid, u8 *pdata, u32 datalen);

u32 bsp_nvm_auth_dcread(u32 modem_id, u32 itemid, u8 *pdata, u32 datalen);
u32 bsp_nvm_auth_dcwrite(u32 modem_id, u32 itemid, u8 *pdata, u32 datalen);


#define bsp_nvm_read(itemid, pdata, datalen) bsp_nvm_dcread(NV_USIMM_CARD_1, itemid, pdata, datalen)
#define bsp_nvm_write(itemid, pdata, datalen) bsp_nvm_dcwrite(NV_USIMM_CARD_1, itemid, pdata, datalen)

#define bsp_nvm_readpart(itemid, offset, pdata, datalen) \
    bsp_nvm_dcreadpart(NV_USIMM_CARD_1, itemid, offset, pdata, datalen)
#define bsp_nvm_writepart(itemid, offset, pdata, datalen) \
    bsp_nvm_dcwritepart(NV_USIMM_CARD_1, itemid, offset, pdata, datalen)

#define bsp_nvm_read_direct(itemid, pdata, datalen) bsp_nvm_dcread_direct(NV_USIMM_CARD_1, itemid, pdata, datalen)
#define bsp_nvm_write_direct(itemid, pdata, datalen) bsp_nvm_dcwrite_direct(NV_USIMM_CARD_1, itemid, pdata, datalen)

#define bsp_nvm_authread(itemid, pdata, datalen) bsp_nvm_auth_dcread(NV_USIMM_CARD_1, itemid, pdata, datalen)
#define bsp_nvm_authwrite(itemid, pdata, datalen) bsp_nvm_auth_dcwrite(NV_USIMM_CARD_1, itemid, pdata, datalen)

u32 bsp_nvm_om_write(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);

u32 bsp_nvm_backup(u32 crc_flag);

u32 bsp_nvm_write_resum(u32 modem_id, u32 itemid, u8 *pdata, u32 offset, u32 datalen);
u32 bsp_nvm_flush_resum(void);

u32 bsp_nvm_write_debug(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen);
u32 bsp_nvm_flush_debug(void);

u32 bsp_nvm_mreset_load(void);

/* 将所有数据刷到文件系统中 */
u32 bsp_nvm_flush(void);
u32 bsp_nvm_flush_rwbuf(void);
void bsp_nvm_wakeup_task(void);


/* 将内存中的数据刷到sys nv中 */
u32 bsp_nvm_flushSys(void);

u32 bsp_nvm_get_len(u32 itemid, u32 *length);
u32 bsp_nvm_authgetlen(u32 itemid, u32 *len);

u32 bsp_nvm_revert_default(void);

u32 bsp_nvm_update_default(void);

u32 bsp_nvm_update_backup(void);

u32 bsp_nvm_xml_decode(void);

u32 nvm_read_rand(u32 nvid);
u32 nvm_read_randex(u32 nvid, u32 modem_id);
u32 nvm_write_randex(u32 nvid, u32 modem_id);

u32 bsp_nvm_check_factory_status(u32 mode);

/* acore */
u32 bsp_nvm_get_modem_num(void);
int modem_nv_init(void);

/* ccore */
void *bsp_nvm_get_addr(u32 modemid, u32 itemid);

s32 bsp_nvm_kernel_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __BSP_NVIM_H__ */
