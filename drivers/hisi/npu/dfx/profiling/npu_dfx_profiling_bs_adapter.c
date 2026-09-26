/*
 * npu_dfx_profiling.c
 *
 * about npu dfx profiling
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
#include <linux/err.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/barrier.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#include "npu_log.h"
#include "npu_dfx.h"
#include "npu_cache.h"
#include "npu_manager_common.h"
#include "npu_mailbox_msg.h"
#include "npu_ioctl_services.h"
#ifdef PROFILING_USE_RESERVED_MEMORY
#include <linux/io.h>
#endif
#include <securec.h>
#include "npu_platform_resource.h"
#include "npu_dfx_profiling.h"

char *get_prof_channel_data_addr (struct prof_buff_desc *prof_buff, u32 channel)
{
	cond_return_error(prof_buff == NULL, NULL, "prof_buff is null\n");
	switch (channel) {
	case PROF_CHANNEL_TSCPU:
		return (char *)prof_buff->tscpu_data;
	case PROF_CHANNEL_AICPU:
		return (char *)prof_buff->aicpu_data;
	case PROF_CHANNEL_AICORE:
		return (char *)prof_buff->aicore_data;
	default:
		npu_drv_err("channel: %u error\n", channel);
	}
	return NULL;
}

u32 get_prof_channel_data_size(struct prof_buff_desc *prof_buff, u32 channel)
{
	cond_return_error(prof_buff == NULL, 0, "prof_buff is null\n");
	switch (channel) {
	case PROF_CHANNEL_TSCPU:
		return PROF_TSCPU_DATA_SIZE;
	case PROF_CHANNEL_AICPU:
		return PROF_AICPU_DATA_SIZE;
	case PROF_CHANNEL_AICORE:
		return PROF_AICORE_DATA_SIZE;
	default:
		npu_drv_err("channel: %u error\n", channel);
	}
	return 0;
}

int prof_buffer_init (struct prof_buff_desc *prof_buff)
{
	cond_return_error(prof_buff == NULL, PROF_ERROR, "prof buf nullptr\n");

	return PROF_OK;
}

int prof_buffer_release (struct prof_buff_desc *prof_buff)
{
	unused(prof_buff);
	return PROF_OK;
}
