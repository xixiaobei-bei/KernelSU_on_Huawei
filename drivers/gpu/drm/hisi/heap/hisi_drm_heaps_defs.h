/*
 * hisi_drm_heaps_defs.h
 *
 * Heaps log manager, isolate from HISI DRM log manager.
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
#ifndef HISI_DRM_HEAPS_DEFS_H
#define HISI_DRM_HEAPS_DEFS_H

#include <linux/printk.h>

extern int hisi_drm_heaps_msg_level;
extern u32 hisi_drm_msg_level;

#define pr_heaps_err(msg, ...)                                                 \
	do { if (hisi_drm_heaps_msg_level > 3 || hisi_drm_msg_level > 3)    \
			pr_err("[HISI_DRM_HEAPS E]:%s: " msg, __func__, ##__VA_ARGS__); } while (0)

#define pr_heaps_warning(msg, ...)                                             \
	do { if (hisi_drm_heaps_msg_level > 4 || hisi_drm_msg_level > 4)    \
			pr_warning("[HISI_DRM_HEAPS W]:%s: " msg, __func__, ##__VA_ARGS__); } while (0)

#define pr_heaps_info(msg, ...)                                                \
	do { if (hisi_drm_heaps_msg_level > 6 || hisi_drm_msg_level > 6)    \
			pr_info("[HISI_DRM_HEAPS I]:%s: " msg, __func__, ##__VA_ARGS__); } while (0)

#define pr_heaps_debug(msg, ...)                                               \
	do { if (hisi_drm_heaps_msg_level > 7 || hisi_drm_msg_level > 7)    \
			pr_info("[HISI_DRM_HEAPS D]:%s: " msg, __func__, ##__VA_ARGS__); } while (0)

#endif
