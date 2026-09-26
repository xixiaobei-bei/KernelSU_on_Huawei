/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef DKMD_DPU_H
#define DKMD_DPU_H

enum {
	DISP_IOCTL_CREATE_FENCE = 0x10,
	DISP_IOCTL_PRESENT,
	DISP_IOCTL_GET_SCENE_CONFIG,
};

#define DISP_IOCTL_MAGIC 'D'

#define DISP_CREATE_FENCE _IOWR(DISP_IOCTL_MAGIC, DISP_IOCTL_CREATE_FENCE, int)
#define DISP_PRESENT _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_PRESENT, struct disp_frame)
#define DISP_GET_SCENE_CONFIG _IOWR(DISP_IOCTL_MAGIC, DISP_IOCTL_GET_SCENE_CONFIG, struct scene_config)

struct disp_layer {
	int32_t shard_fd;
	int32_t acquired_fence;
};


#define DISP_LAYER_MAX_COUNT 32
struct disp_frame {
	uint32_t cmdlist_id;
	uint32_t layer_count;
	struct disp_layer layer[DISP_LAYER_MAX_COUNT];
};

#define SCENE_SPLIT_MAX 4
enum {
	SCENE_SPLIT_MODE_NONE = 0,
	SCENE_SPLIT_MODE_V,
	SCENE_SPLIT_MODE_H,
};

struct scene_config {
	uint8_t split_mode;
	uint8_t split_count;
	uint16_t split_ratio[SCENE_SPLIT_MAX];
};


#endif