/*
 * bfm_mtk.h
 *
 * define the core's external public enum/macros/interface for
 * BFR(Boot Fail Recovery)
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef BFM_MTK_H
#define BFM_MTK_H
#include "chipset_common/bfmr/public/bfmr_public.h"

struct boot_context {
	u32 boot_magic;        /* must be initialized in slb1 */
	u32 boot_stage;        /* must be initialized in every stage */
	u32 last_boot_stage;
	u32 boot_error_no;     /* set this via set_boot_error */
	u32 last_boot_error_no;
	u32 hash_code;
};

#define BOOT_TIMER_INTERVAL  10000
#define BOOT_TOO_LONG_TIME   (1000*60*30)
#define BOOT_TOO_LONG_TIME_EXACT   (1000*60*3)
#define BFM_STAGE_CODE              0x00000001
#define BFM_ERROR_CODE              0x00000002
#define BFM_TIMER_EN_CODE           0x00000003
#define BFM_TIMER_SET_CODE          0x00000004
#define BFM_CAN_NOT_CALL_TRY_TO_RECOVERY  0xFFFFFFFF

/* IMEM ALLOCATION */
#define HWBOOT_LOG_INFO_SIZE 0x100

#define HWBOOT_MAGIC_NUMBER   (*((u32 *)("BOOT")))
#define HWBOOT_FAIL_INJECT_MAGIC  0x12345678

#define BOOT_LOG_CHECK_SUM_SIZE \
	((u32)(&(((struct boot_context *)0)->hash_code)))

/* export interface to bfm_core */
void set_boot_stage(enum bfmr_detail_stage stage);
u32 get_boot_stage(void);
int set_boot_fail_flag(enum bfm_errno_code bootfail_errno);

/* export interface to kernel */
void hwboot_fail_init_struct(void);
void hwboot_clear_magic(void);
int kmsg_print_to_ddr(char *buf, int size);

#endif
