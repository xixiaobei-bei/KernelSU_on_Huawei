/*
 * HISI drm dmabuf ops implement
 *
 * Copyright (C) 2019 Hisilicon. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HISI_GEM_DMABUF_H__
#define __HISI_GEM_DMABUF_H__

struct dma_buf *hisi_gem_dmabuf_prime_export(struct drm_device *dev,
				     struct drm_gem_object *obj,
				     int flags);
struct drm_gem_object *hisi_gem_dmabuf_prime_import(struct drm_device *dev,
					    struct dma_buf *dma_buf);
struct drm_gem_object *hisi_drm_gem_prime_import_dev(struct drm_device *dev,
						struct dma_buf *dma_buf,
						struct device *attach_dev);
#ifdef CONFIG_DRM_DSS_IOMMU
struct hisi_drm_gem *hisi_gem_dmabuf_to_gem(struct dma_buf *dmabuf);
bool is_hisi_drm_dmabuf(struct dma_buf *dmabuf);
#else
static inline bool is_hisi_drm_dmabuf(struct dma_buf *dmabuf)
{
	return false;
}

static inline struct hisi_drm_gem *
hisi_gem_dmabuf_to_gem(struct dma_buf *dmabuf)
{
	return NULL;
}
#endif
#endif

