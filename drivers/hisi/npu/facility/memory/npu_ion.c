/*
 * npu_ion.c
 *
 * npu ion
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_ion.h"

#include <linux/err.h>
#include <linux/dma-buf.h>

#include "hisi_ion.h"
#include "npu_log.h"

void npu_ioctl_check_ionfd(npu_check_ion_t *check_ion)
{
	struct dma_buf *dmabuf = NULL;

	check_ion->result = NPU_INVALID_ION;
	dmabuf = dma_buf_get(check_ion->ionfd);
	if (IS_ERR_OR_NULL(dmabuf))
		return;

	if (is_ion_dma_buf(dmabuf))
		check_ion->result = NPU_VALID_ION;

	dma_buf_put(dmabuf);
}
