/*
 * scd_drv.h
 *
 * This is for vdec driver for scd master.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SCD_DRV_H__
#define __SCD_DRV_H__

#include "basedef.h"
#include "mem_manage.h"
#include "memory.h"
#include "vfmw.h"
#include "public.h"

#define SCDDRV_OK                       0
#define SCDDRV_ERR                     (-1)

#define SCD_TIME_OUT_COUNT              200

#define REG_SCD_START                   0x800
#define REG_LIST_ADDRESS                0x804
#define REG_UP_ADDRESS                  0x808
#define REG_UP_LEN                      0x80c
#define REG_BUFFER_FIRST                0x810
#define REG_BUFFER_LAST                 0x814
#define REG_BUFFER_INI_LSB              0x818
#define REG_SCD_PROTOCOL                0x820

#define REG_SCD_PREVIOUS_BYTE_LSB       0x83c
/* state registers */
#define REG_SCD_OVER                    0x840
#define REG_SCD_PREVIOUS_BYTE_MSB       0x844
#define REG_SCD_NUM                     0x84c
#define REG_ROLL_ADDR                   0x850
#define REG_SRC_EATEN                   0x854
#define REG_SCR_NXT_ADDR_LSB            0x85c
#define REG_BUFFER_INI_MSB              0x8A0
#define REG_SCR_NXT_ADDR_MSB            0x8A4
#define REG_ROLL_ADDR_MSB               0x8A8

#define REG_SCD_SAFE_INT_MASK           0x884
#define REG_SCD_SAFE_INI_CLR            0x888
#define REG_SCD_NORM_INT_MASK           0x81c
#define REG_SCD_NORM_INI_CLR            0x824

#ifdef ENV_SOS_KERNEL
#define REG_SCD_INT_MASK               REG_SCD_SAFE_INT_MASK
#define REG_SCD_INI_CLR                REG_SCD_SAFE_INI_CLR
#else
#define REG_SCD_INT_MASK               REG_SCD_NORM_INT_MASK
#define REG_SCD_INI_CLR                REG_SCD_NORM_INI_CLR
#endif

#define REG_AVS_FLAG                    0x0000
#define REG_EMAR_ID                     0x0004
#define REG_VDH_SELRST                  0x0008

/* up msg step */
#define SM_SCD_UP_INFO_NUM              3

#ifdef CFG_MAX_RAW_NUM
#define MAX_STREAM_RAW_NUM             (CFG_MAX_RAW_NUM)
#else
#define MAX_STREAM_RAW_NUM              1024
#endif
#ifdef CFG_MAX_SEG_NUM
#define MAX_STREAM_SEG_NUM             (CFG_MAX_SEG_NUM)
#else
#define MAX_STREAM_SEG_NUM             (1024 + 128)
#endif

/* when move raw stream once,use 8 word to hold 5 word scd down msg */
#define SM_DOWN_MSG_LEN                 8
#define SM_MAX_DOWNMSG_SIZE \
	(SM_DOWN_MSG_LEN * MAX_STREAM_RAW_NUM * sizeof(SINT32))
#define SM_MAX_UPMSG_SIZE \
	(SM_SCD_UP_INFO_NUM * MAX_STREAM_SEG_NUM * sizeof(SINT32))

#define SCD_OUTPUT_BUF_CNT              5

#define INVALID_SHAREFD                 0

#define SCD_LOWDLY_ENABLE_OFFSET        8
#define SCD_DRV_REG_OFFSET              7
#define SCD_PROTOCOL_OFFSET             0x0f
#define SLICE_CHECK_FLAG_OFFSET         4
#define SLICE_CHECK_FLAG_OPR            0x10
#define UP_MSG_MAX_OFFSET               15
#define RESET_SCD_COUNT                 100

typedef enum {
	FMW_OK          = 0,
	FMW_ERR_PARAM   = -1,
	FMW_ERR_NOMEM   = -2,
	FMW_ERR_NOTRDY  = -3,
	FMW_ERR_BUSY    = -4,
	FMW_ERR_RAWNULL = -5,
	FMW_ERR_SEGFULL = -6,
	FMW_ERR_SCD     = -7
} fmw_retval_e;

typedef enum {
	SCDDRV_SLEEP_STAGE_NONE = 0,
	SCDDRV_SLEEP_STAGE_PREPARE,
	SCDDRV_SLEEP_STAGE_SLEEP
} scd_drv_sleep_stage_e;

typedef enum {
	SCD_IDLE = 0,
	SCD_WORKING,
} scd_state_e;

/* register operator */
#define rd_scd_reg(reg)       mem_read_phy_word(g_scd_reg_base_addr + (reg))
#define wr_scd_reg(reg, dat)  mem_write_phy_word((g_scd_reg_base_addr + (reg)), dat)

#define fmw_assert_ret(cond, ret) \
	do { \
		if (!(cond)) \
			return ret; \
	} while (0)

typedef enum {
	SCD_SHAREFD_MESSAGE_POOL = 0,
	SCD_SHAREFD_OUTPUT_BUF   = 1,
	SCD_SHAREFD_MAX          = (SCD_SHAREFD_OUTPUT_BUF + SCD_OUTPUT_BUF_CNT)
} scd_sharefd;

typedef struct {
	SINT32 scd_over;
	SINT32 scd_int;
	SINT32 short_scd_num;
	SINT32 scd_num;
	UADDR scd_roll_addr;
	SINT32 src_eaten;
} sm_state_reg_s;

typedef struct {
	UINT8 slice_check_flag;
	UINT8 scd_start;
	UADDR down_msg_phy_addr;
	UADDR up_msg_phy_addr;
	SINT32 up_len;
	UADDR buffer_first;
	UADDR buffer_last;
	UADDR buffer_ini;
	UINT32 scd_protocol;
	UINT32 scd_lowdly_enable;
	SINT32 scd_share_fd[SCD_SHAREFD_MAX];
	UINT32 scd_output_buf_num;
	hi_bool is_scd_all_buf_remap;
} scd_config_reg_s;

typedef struct {
	hi_s32 scd_protocol;
	hi_s32 scd_over;
	hi_s32 scd_int;
	hi_s32 scd_num;
	UADDR  scd_roll_addr;
	hi_s32 src_eaten;
	hi_s32 up_len;
} scd_state_reg_s;

typedef enum hi_CONFIG_SCD_CMD {
	CONFIG_SCD_REG_CMD = 100,
} config_scd_cmd;

typedef struct {
	config_scd_cmd cmd;
	SINT32         e_vid_std;
	UINT32         s_reset_flag;
	UINT32         glb_reset_flag;
	scd_config_reg_s sm_ctrl_reg;
} omx_scd_reg_cfg_s;

SINT32 scd_drv_prepare_sleep(void);

scd_drv_sleep_stage_e scd_drv_get_sleep_stage(void);
void scd_drv_set_sleep_stage(scd_drv_sleep_stage_e sleep_state);

void scd_drv_force_sleep(void);

void scd_drv_exit_sleep(void);

SINT32 scd_drv_reset_scd(void);

SINT32 scd_drv_write_reg(scd_config_reg_s *sm_ctrl_reg, mem_buffer_s *scd_mem_map);

void scd_drv_get_reg_state(scd_state_reg_s *scd_state_reg);

void scd_drv_isr(void);

void scd_drv_init(void);

void scd_drv_deinit(void);

#ifdef ENV_ARMLINUX_KERNEL
SINT32 scd_drv_is_scd_idle(void);
#endif
#endif
