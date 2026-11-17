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

#ifndef DKMD_RES_MGR_H
#define DKMD_RES_MGR_H

#include <linux/types.h>

#define RES_IOCTL_MAGIC 'R'

enum {
	RES_IOCTL_INIT_OPRS = 0x10,
	RES_IOCTL_REQUEST_OPR,
	RES_IOCTL_REQUEST_OPR_SYNC,
	RES_IOCTL_RELEASE_OPR,
	RES_IOCTL_GET_CONFIG,
	RES_IOCTL_GET_IOVA,
	RES_IOCTL_CMD_MAX,
};


#define RES_INIT_OPR      _IOW(RES_IOCTL_MAGIC, RES_IOCTL_INIT_OPRS, struct res_opr_type)

/**
 * @brief
 *
 */
#define RES_REQUEST_OPR      _IOW(RES_IOCTL_MAGIC, RES_IOCTL_REQUEST_OPR, struct res_opr_info)

/**
 * @brief
 *
 */
#define RES_REQUEST_OPR_SYNC _IOW(RES_IOCTL_MAGIC, RES_IOCTL_REQUEST_OPR_SYNC, struct res_opr_info)

/**
 * @brief
 *
 */
#define RES_RELEASE_OPR      _IOW(RES_IOCTL_MAGIC, RES_IOCTL_RELEASE_OPR, uint64_t)

/**
 * @brief
 *
 */
#define DISP_GET_IOVA _IORW(RES_IOCTL_MAGIC, RES_IOCTL_GET_IOVA, struct res_dma_buf)

enum Scene_Type {
	SCENE_TYPE_ONLINE = 0,
	SCENE_TYPE_OFFLINE,
	SCENE_TYPE_MAX,
};

/**
 * @brief
 *
 */
struct res_opr_info {
	uint32_t scene_id;
	uint32_t scene_type;
	uint32_t opr_id;
	int32_t result;
};

/**
 * @brief
 *
 */
struct res_opr_type {
	uint32_t opr_type;
	uint32_t opr_count;
	uint64_t opr_id_ptr;
};

/*
 * shard_fd
 */
struct res_dma_buf {
	int shard_fd;
	int32_t iova;
};

#endif