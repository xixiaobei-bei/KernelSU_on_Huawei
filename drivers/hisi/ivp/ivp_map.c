/*
 * This file define fd maps
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "ivp_map.h"
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ion.h>
#include <linux/dma-buf.h>
#include <linux/hisi-iommu.h>
#include <linux/hisi/hisi_ion.h>
#include "ivp_log.h"

#ifndef EOK
#define EOK 0
#endif

#define DMA_BUF_REF_MAX 20

enum DMA_BUF_STATUS {
	FREE = 0,
	WORK
};

struct ion_buffer {
	u64 magic;
	union {
		struct rb_node node;
		struct list_head list;
	};
	struct ion_device *dev;
	struct ion_heap *heap;
	unsigned long flags;
	unsigned long private_flags;
	size_t size;
	void *priv_virt;
	struct mutex lock;
	int kmap_cnt;
	void *vaddr;
	struct sg_table *sg_table;
	struct list_head attachments;
	char task_comm[TASK_COMM_LEN];
	pid_t pid;
	unsigned int id;
};

struct dma_buf_ref {
	int fd;
	int status;
	struct dma_buf *dmabuf;
};

struct dma_buf_ref g_dma_ref[DMA_BUF_REF_MAX];

int ivp_map_hidl_fd(struct device *dev, struct ivp_map_info *map_buf)
{
	struct dma_buf *buf = NULL;

	if (!dev || !map_buf) {
		ivp_err("invalid input");
		return -EINVAL;
	}

	if (map_buf->fd < 0) {
		ivp_err("fd is invalid");
		return -EINVAL;
	}

	buf = dma_buf_get(map_buf->fd);
	if (IS_ERR(buf)) {
		ivp_err("invalid file shared_fd = %d", map_buf->fd);
		return -EINVAL;
	}

	map_buf->iova = hisi_iommu_map_dmabuf(dev, buf,
		map_buf->fd_prot, &(map_buf->mapped_size));
	if (map_buf->iova == 0) {
		ivp_err("fail to map iommu iova");
		dma_buf_put(buf);
		return -ENOMEM;
	}

	dma_buf_put(buf);
	return EOK;
}

int ivp_unmap_hidl_fd(struct device *dev,
		struct ivp_map_info *map_buf)
{
	int ret = EOK;
	struct dma_buf *buf = NULL;

	if (!dev || !map_buf) {
		ivp_err("invalid input");
		return -EINVAL;
	}

	if (map_buf->fd < 0) {
		ivp_err("fd is invalid");
		return -EINVAL;
	}

	buf = dma_buf_get(map_buf->fd);
	if (IS_ERR(buf)) {
		ivp_err("fail to get dma dmabuf, fd = %d", map_buf->fd);
		return -EINVAL;
	}

	if (hisi_iommu_unmap_dmabuf(dev, buf, map_buf->iova) < 0) {
		ivp_err("fail to unmap fd = %d", map_buf->fd);
		ret = -ENOMEM;
	}

	map_buf->iova = 0;
	dma_buf_put(buf);
	return ret;
}


