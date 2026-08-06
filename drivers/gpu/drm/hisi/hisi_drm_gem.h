/*
 * hisi_drm_gem.h
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#ifndef HISI_DRM_GEM_H
#define HISI_DRM_GEM_H

#include <drm/drm_gem.h>
#include <drm/drm_mode.h>
#include <linux/rbtree.h>

struct hisi_drm_gem {
	struct drm_gem_object base;

	unsigned int flags;
	unsigned long size;
	struct sg_table *sgt;
	unsigned long iova;
	unsigned int heap_id;

	struct mutex hisi_gem_lock;
	unsigned int kmap_cnt;
	void *vaddr;
#ifdef CONFIG_DRM_GEM_TRACE
	struct rb_node node;

	/* debug info, saved the task name which request memory from drm */
	char task_comm[TASK_COMM_LEN];

	/* debug info, saved the task's pid */
	pid_t pid;
#endif
};

#define to_hisi_gem(x) container_of(x, struct hisi_drm_gem, base)


struct hisi_drm_gem* hisi_drm_gem_create(
	struct drm_device *dev,
	unsigned int flags,
	unsigned long size);
void hisi_drm_gem_destroy(
	struct hisi_drm_gem *hisi_gem);

/* free gem object. */
void hisi_drm_gem_free_object(
	struct drm_gem_object *obj);
/* create memory region for drm framebuffer. */
int hisi_drm_gem_dumb_create(
	struct drm_file *file_priv,
	struct drm_device *dev,
	struct drm_mode_create_dumb *args);
int hisi_drm_gem_prime_handle_to_fd(
	struct drm_device *dev,
	struct drm_file *file_priv, uint32_t handle,
	uint32_t flags,
	int *prime_fd);

struct drm_gem_object* hisi_drm_gem_prime_import(
	struct drm_device *dev,
	struct dma_buf *dma_buf);
struct sg_table* hisi_drm_gem_prime_get_sg_table(
	struct drm_gem_object *obj);
struct drm_gem_object* hisi_drm_gem_prime_import_sg_table(
	struct drm_device *dev,
	struct dma_buf_attachment *attach,
	struct sg_table *sgt);

void* hisi_drm_gem_prime_vmap(
	struct drm_gem_object *obj);
void hisi_drm_gem_prime_vunmap(
	struct drm_gem_object *obj, void *vaddr);
int hisi_drm_gem_prime_mmap(
	struct drm_gem_object *obj,
	struct vm_area_struct *vma);

/* private ioctl */
int hisi_drm_gem_create_ioctl(
	struct drm_device *dev, void *data,
	struct drm_file *file_priv);
int hisi_drm_gem_mmap(struct file *file_priv, struct vm_area_struct *vma);

#endif /* HISI_DRM_GEM_H */
