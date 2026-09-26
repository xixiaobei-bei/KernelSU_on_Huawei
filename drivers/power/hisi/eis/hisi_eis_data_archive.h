/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_eis_data_archive.h
 *
 * eis core io interface header
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

#ifndef _BATT_EIS_CORE_UFS_H_
#define _BATT_EIS_CORE_UFS_H_

#include <linux/device.h>
#include <linux/kernel.h>
#include "hisi_eis_core_freq.h"
#include "pmic/hisi_pmic_eis.h"

#define EIS_BUF_PATH_LEN        128
#define EIS_FILE_LIMIT          0660

/* offset from 110K */
#define EIS_FREQ_OFFSET         0x1B800
/* 110K */
#define EIS_FREQ_SIZE           0x190F0
#define FREQ_INDEX_OFFSET       0x348FF
#define FREQ_INDEX_MAGIC        0x7FF7
#define EIS_FREQ_ROLLING        160
#define EIS_FREQ_ROW            16
#define EIS_FREQ_LINE_WIDTH     180
/* (7 * sizeof (int) + 2 * sizeof(long long)+ 8) * 16 + 1 */
#define EIS_FREQ_PARAM_SIZE     4200

void hisi_eis_freq_write_info_to_flash(struct eis_freq_info *freq_inf);
void hisi_eis_freq_info_printk(struct eis_freq_info *freq_info);
void hisi_eis_freq_read_single_info_from_flash(
	struct eis_freq_info *freq_inf, int index);
void hisi_eis_freq_read_all_info_from_flash(
	struct eis_freq_info (*freq_inf)[LEN_T_FREQ]);
int eis_get_g_full_freq_rolling(void);
int eis_get_g_freq_rolling_now(void);
void hisi_eis_freq_save_index_to_flash(void);
void hisi_eis_freq_read_index_from_flash(void);
int hisi_eis_read_flash_data(void *buf, u32 buf_size, u32 flash_offset);

#ifdef CONFIG_HISI_DEBUG_FS
void test_freq_task_schedule(void);
int test_eis_set_temp(int temp);
int get_test_bat_temp(void);
#endif /* CONFIG_HISI_DEBUG_FS */

#endif /* _BATT_EIS_CORE_UFS_H_ */

