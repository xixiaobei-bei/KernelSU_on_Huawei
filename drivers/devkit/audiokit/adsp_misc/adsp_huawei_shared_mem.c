/*
 * adsp_huawei_shared_mem driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "adsp_huawei_shared_mem.h"

#include <linux/module.h>
#include <linux/uaccess.h>
#include <adsp_helper.h>
#include <audio_controller_msg_id.h>
#include <audio_messenger_ipi.h>
#include <audio_task.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG adsp_huawei_shared_mem
HWLOG_REGIST();

static void send_shared_mem_info_to_adsp(uint32_t addr, uint32_t size)
{
	struct ipi_msg_t ipi_msg;

	adsp_register_feature(AUDIO_CONTROLLER_FEATURE_ID);
	audio_send_ipi_msg(
		&ipi_msg,
		TASK_SCENE_AUDIO_CONTROLLER_HIFI3_A,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_DIRECT_SEND,
		AUD_CTL_MSG_A2D_SHARED_MEM_INIT,
		addr,
		size,
		NULL);
	audio_send_ipi_msg(
		&ipi_msg,
		TASK_SCENE_AUDIO_CONTROLLER_HIFI3_B,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_DIRECT_SEND,
		AUD_CTL_MSG_A2D_SHARED_MEM_INIT,
		addr,
		size,
		NULL);
	adsp_deregister_feature(AUDIO_CONTROLLER_FEATURE_ID);
}

int update_adsp_shared_mem(const void __user *buf, size_t buf_size)
{
	void *mem_vaddr =
		adsp_get_reserve_mem_virt(ADSP_HUAWEI_SHARED_MEM_ID);
	phys_addr_t mem_paddr =
		adsp_get_reserve_mem_phys(ADSP_HUAWEI_SHARED_MEM_ID);
	size_t mem_size =
		adsp_get_reserve_mem_size(ADSP_HUAWEI_SHARED_MEM_ID);

	if ((mem_size < buf_size) || (mem_vaddr == NULL)) {
		hwlog_err("%s: shared mem error\n", __func__);
		return -ENOMEM;
	}
	if (copy_from_user(mem_vaddr, buf, buf_size)) {
		hwlog_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}
	send_shared_mem_info_to_adsp((uint32_t)mem_paddr, (uint32_t)buf_size);
	return 0;
}

int notify_adsp_shared_mem(size_t buf_size)
{
	void *mem_vaddr =
		adsp_get_reserve_mem_virt(ADSP_HUAWEI_SHARED_MEM_ID);
	phys_addr_t mem_paddr =
		adsp_get_reserve_mem_phys(ADSP_HUAWEI_SHARED_MEM_ID);
	size_t mem_size =
		adsp_get_reserve_mem_size(ADSP_HUAWEI_SHARED_MEM_ID);

	if ((mem_size < buf_size) || (mem_vaddr == NULL)) {
		hwlog_err("%s: shared mem error\n", __func__);
		return -ENOMEM;
	}
	send_shared_mem_info_to_adsp((uint32_t)mem_paddr, (uint32_t)buf_size);
	return 0;
}

MODULE_DESCRIPTION("adsp_huawei_shared_mem driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
