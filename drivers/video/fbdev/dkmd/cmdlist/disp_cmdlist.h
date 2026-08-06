/**
 * @file disp_cmdlist.h
 * @brief The ioctl the interface file for cmdlist node.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DISP_CMDLIST_H__
#define __DISP_CMDLIST_H__

#include <linux/types.h>

#define CMDLIST_IOCTL_MAGIC 'C'

#define CMDLIST_CREATE_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x1, struct cmdlist_node_client)
#define CMDLIST_SIGNAL_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x2, struct cmdlist_node_client)
#define CMDLIST_LINK_NEXT_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x3, struct cmdlist_node_client)
#define CMDLIST_APPEND_NEXT_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x4, struct cmdlist_node_client)
#define CMDLIST_DUMP_USER_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x5, struct cmdlist_node_client)
#define CMDLIST_DUMP_SCENE_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x6, struct cmdlist_node_client)
#define CMDLIST_DUMP_ALL_CLIENT _IOWR(CMDLIST_IOCTL_MAGIC, 0x7, struct cmdlist_node_client)
#define CMDLIST_INFO_GET _IOWR(CMDLIST_IOCTL_MAGIC, 0x8, unsigned)

enum {
	CONFIG_REGISTER_TYPE = 0,
	DATA_TRANSPORT_TYPE,
	SCENE_NOP_TYPE,
	MAX_TYPE_DEFINE,
};

struct cmdlist_node_client {
	uint32_t id;

	uint32_t type;
	uint32_t scene_id;
	uint32_t node_size;
	uint32_t dst_addr;

	uint32_t append_next_id;
	uint32_t link_next_id;

	int fence_fd;
	uint64_t viraddr;
	uint32_t phyaddr;
	int reserved;
};

struct cmdlist_info {
	uint64_t viraddr_base;
	uint32_t pool_size;
	int reserved;
};

#endif
