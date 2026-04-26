/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef OPR_MGR_H
#define OPR_MGR_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/semaphore.h>
#include "dkmd_res_mgr.h"

#define INVALID_SCENE_ID 0xffffffff
#define INVALID_SCENE_TYPE SCENE_TYPE_MAX
typedef uint32_t opr_id;

enum Opr_State {
	OPR_STATE_IDLE = 0,
	OPR_STATE_BUSY,
};

struct opr {
	struct list_head list_node;

	uint32_t type;
	opr_id id;

	uint32_t scene_id;
	uint32_t scene_type;
	uint32_t state;
};

struct dpu_opr_mgr {
	struct semaphore sem;

	struct list_head opr_list; // struct opr is node
};

void dpu_rm_register_opr_mgr(struct list_head *resource_head);

static inline void dpu_pr_req_cmd_info(const struct res_opr_info *info)
{
	dpu_pr_info("scene_id=%u, scene_type=%u, opr_id=%u", info->scene_id, info->scene_type, info->opr_id);
}

static inline void dpu_pr_info_opr(const struct opr *opr)
{
	dpu_pr_info("type=%u, id=0x%x scene_id=%u, scene_type=%u, state=%u", \
			 opr->type, opr->id, opr->scene_id, opr->scene_type, opr->state);
}

#endif