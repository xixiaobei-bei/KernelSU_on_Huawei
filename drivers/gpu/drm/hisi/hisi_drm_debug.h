/*
 * Copyright (c) 2016 Linaro Limited.
 * Copyright (c) 2014-2020 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef HISI_DRM_DEBUGFS_H
#define HISI_DRM_DEBUGFS_H

#define HISI_DRM_NONE_FLAG (0x0)
#define HISI_DRM_COMPOSE_FLAG (1 << 0)
#define HISI_DRM_DUMP_REG_FLAG (1 << 1)
#define HISI_DRM_REG_PRINT_FLAG (1 << 2)
#define HISI_DRM_GEM_FLAG (1 << 3)
#define HISI_DRM_PDP_IRQ_FLAG (1 << 4)
#define HISI_DRM_DSI_IRQ_FLAG (1 << 5)
#define HISI_DRM_FRAME_FLAG (1 << 6)

struct drm_minor;
int hisi_drm_debugfs_init(struct drm_minor *minor);
void hisi_drm_printk(const char *level, unsigned int category,
	const char *func, const char *fmt, ...);

#define HISI_DRM_INFO_COMPOSE(fmt, ...) \
	hisi_drm_printk(KERN_INFO, HISI_DRM_COMPOSE_FLAG, \
		__func__, fmt, ##__VA_ARGS__)

#define HISI_DRM_INFO_GEM(fmt, ...) \
	hisi_drm_printk(KERN_INFO, HISI_DRM_GEM_FLAG, \
		__func__, fmt, ##__VA_ARGS__)

#define HISI_DRM_INFO_PDP_IRQ(fmt, ...) \
	hisi_drm_printk(KERN_INFO, HISI_DRM_PDP_IRQ_FLAG, \
		__func__, fmt, ##__VA_ARGS__)

#define HISI_DRM_INFO_DSI_IRQ(fmt, ...) \
	hisi_drm_printk(KERN_INFO, HISI_DRM_DSI_IRQ_FLAG, \
		__func__, fmt, ##__VA_ARGS__)

#define HISI_DRM_PRINT_REG(fmt, ...) \
	hisi_drm_printk(KERN_INFO, HISI_DRM_REG_PRINT_FLAG, \
		__func__, fmt, ##__VA_ARGS__)

#define HISI_DRM_DEBUG_FRAME(fmt, ...) \
	hisi_drm_printk(KERN_INFO, HISI_DRM_FRAME_FLAG, \
		__func__, fmt, ##__VA_ARGS__)

#endif
