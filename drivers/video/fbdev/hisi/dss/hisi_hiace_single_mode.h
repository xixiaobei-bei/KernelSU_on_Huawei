/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_HIACE_SINGLE_MODE_H
#define HISI_HIACE_SINGLE_MODE_H

#include "hisi_fb.h"
#include "hisi_display_effect.h"

#define HIACE_TIMEOUT_PER_FRAME 20000  /* 20ms */

enum {
	EN_HIACE_SINGLE_MODE_IDLE = 0,
	EN_HIACE_SINGLE_MODE_WORKING = 0x10,
	EN_HIACE_SINGLE_MODE_WORKING_GLOBAL_HIST = 0x11,
	EN_HIACE_SINGLE_MODE_WORKING_LOCAL_HIST = 0x12,
	EN_HIACE_SINGLE_MODE_WORKING_FNA = 0x13,
	EN_HIACE_SINGLE_MODE_WORKING_ISR_FNA = 0x14,
	EN_HIACE_SINGLE_MODE_DONE = 0x20,
};

struct hiace_single_mode_info {
	/* for ioctl */
	struct dss_hiace_single_mode_ctrl_info ioctl_info;

	/* for internal ctrl */
	uint8_t single_mode_state;  /* idle, working or done */
	uint8_t ioctl_blocking_mode;  /* 0:blocking mode, 1:non-blocking mode */
	wait_queue_head_t wq_hist;
	struct semaphore wq_sem;
	struct mutex hist_lock;

	/* for information readbackgit */
	uint32_t block_once_num;
	uint32_t hist_block_h_ptr;
	uint32_t hist_block_v_ptr;

	/* readback info */
	uint32_t hist[HIACE_GHIST_RANK + HIACE_GHIST_RANK + YBLOCKNUM * XBLOCKNUM * HIACE_LHIST_RANK];
	uint32_t fna[YBLOCKNUM * XBLOCKNUM * HIACE_FNA_RANK];
};

void dpufb_hiace_single_mode_init(void);
void dpufb_hiace_single_mode_terminate(struct dpu_fb_data_type *dpufd, bool disable);
int dpufb_hiace_single_mode_trigger(struct fb_info *info, const void __user *argp);
int dpufb_hiace_single_mode_block_once_set(struct fb_info *info, const void __user *argp);
int dpufb_hiace_hist_get(struct fb_info *info, void __user *argp);
int dpufb_hiace_fna_get(struct fb_info *info, void __user *argp);
void hisi_hiace_single_mode_wq_handler(struct work_struct *work);
bool hisi_hiace_single_mode_handle_isr(struct dpu_fb_data_type *dpufd);

#endif
