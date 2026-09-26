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
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include "dpu_utils.h"
#include "opr_mgr.h"
#include "res_mgr.h"

#define OPR_COUNT_MAX 100
#define DISP_LOG_TAG "RES_MGR"

static struct opr* find_opr(struct list_head *opr_list, uint64_t id)
{
	struct opr *opr = NULL;
	struct opr *_node_ = NULL;

	list_for_each_entry_safe(opr, _node_, opr_list, list_node) {
		if (!opr)
			continue;

		if (opr->id == id)
			return opr;
	}

	return NULL;
}

static void free_opr_list(struct list_head *opr_list)
{
	struct opr *opr = NULL;
	struct opr *_node_ = NULL;

	list_for_each_entry_safe(opr, _node_, opr_list, list_node) {
		if (!opr)
			continue;

		list_del(&opr->list_node);
		kfree(opr);
		opr = NULL;
	}
}

static int opr_mgr_init_opr(void *data, void __user *argp)
{
	struct dpu_opr_mgr *opr_mgr = (struct dpu_opr_mgr *)data;
	struct res_opr_type type = {0};
	opr_id opr_id[OPR_COUNT_MAX] = {0};
	uint32_t i;
	struct opr *opr = NULL;

	dpu_assert_if_cond(opr_mgr == NULL);

	if (copy_from_user(&type, argp, sizeof(type)) != 0) {
		dpu_pr_err("copy from user fail");
		return -1;
	}

	if (type.opr_count == 0 || type.opr_count > OPR_COUNT_MAX) {
		dpu_pr_err("opr_count %d is overflow", type.opr_count);
		return -1;
	}

	if ((uint64_t *)(uintptr_t)type.opr_id_ptr == NULL) {
		dpu_pr_err("opr_id_ptr is null");
		return -1;
	}

	if (copy_from_user(opr_id, (uint64_t *)(uintptr_t)(type.opr_id_ptr), sizeof(opr_id) * type.opr_count) != 0) {
		dpu_pr_err("copy from user opr id fail");
		return -1;
	}

	down(&opr_mgr->sem);
	for (i = 0; i < type.opr_count; ++i) {
		if (opr_id[i] == 0)
			continue;

		opr = kzalloc(sizeof(struct opr), GFP_KERNEL);
		if (!opr) {
			dpu_pr_err("alloc opr memory fail, i =%u", i);
			continue;
		}

		opr->id = opr_id[i];
		opr->type = type.opr_type;
		opr->scene_id = INVALID_SCENE_ID;
		opr->scene_type = INVALID_SCENE_TYPE;
		opr->state = OPR_STATE_IDLE;

		dpu_pr_info_opr(opr);

		list_add_tail(&opr->list_node, &opr_mgr->opr_list);
	}

	up(&opr_mgr->sem);
	return 0;
}

static int _request_opr(struct list_head *opr_list, struct res_opr_info *req_cmd)
{
	struct opr *opr = NULL;

	opr = find_opr(opr_list, req_cmd->opr_id);
	if (!opr) {
		dpu_pr_err("find opr fail");
		dpu_pr_req_cmd_info(req_cmd);
		return -1;
	}

	switch (opr->state) {
	case OPR_STATE_IDLE:  /* reqeust succ */
		opr->scene_id = req_cmd->scene_id;
		opr->scene_type = req_cmd->scene_type;
		opr->state = OPR_STATE_BUSY;

		req_cmd->result = 0;
		break;
	case OPR_STATE_BUSY: /* reqeust fail */
		req_cmd->result = -1;
		break;
	default:
		req_cmd->result = -1;
		dpu_pr_err("unsupport opr state");
		dpu_pr_info_opr(opr);
		return -1;
	}

	dpu_pr_info_opr(opr);
	return 0;
}

static long opr_mgr_request_opr(void *data, void __user *argp)
{
	struct dpu_opr_mgr *opr_mgr = (struct dpu_opr_mgr *)data;
	struct res_opr_info req_cmd;

	if (copy_from_user(&req_cmd, argp, sizeof(req_cmd)) != 0) {
		dpu_pr_err("copy from user fail");
		return -1;
	}

	if (req_cmd.opr_id < 0) {
		dpu_pr_err("invalid opr_id");
		return -1;
	}

	down(&opr_mgr->sem);
	if (_request_opr(&opr_mgr->opr_list, &req_cmd) != 0) {
		dpu_pr_err("reqeust opr fail");
		up(&opr_mgr->sem);
		return -1;
	}

	if (copy_to_user(argp, &req_cmd, sizeof(req_cmd)) != 0) {
		dpu_pr_err("copy to user fail");
		up(&opr_mgr->sem);
		return -EINVAL;
	}

	up(&opr_mgr->sem);
	return 0;
}

/* TODO */
static long opr_mgr_request_opr_sync(void *data, void __user *argp)
{
	return -1;
}

static long opr_mgr_release_opr(void *data, void __user *argp)
{
	struct dpu_opr_mgr *opr_mgr = (struct dpu_opr_mgr *)data;
	opr_id released_opr_id;
	struct opr *opr = NULL;

	if (copy_from_user(&released_opr_id, argp, sizeof(released_opr_id)) != 0) {
		dpu_pr_err("copy from user fail");
		return -1;
	}

	if (released_opr_id == 0) {
		dpu_pr_err("invalid opr_id");
		return -1;
	}

	opr = find_opr(&opr_mgr->opr_list, released_opr_id);
	if (!opr) {
		dpu_pr_err("find opr fail id=0x%x", released_opr_id);
		return -1;
	}

	opr->scene_id = INVALID_SCENE_ID;
	opr->scene_type = INVALID_SCENE_TYPE;
	opr->state = OPR_STATE_IDLE;
	return 0;
}

static long opr_mgr_ioctl(void *data, unsigned int cmd, void __user *argp)
{
	switch (cmd) {
	case RES_INIT_OPR:
		return opr_mgr_init_opr(data, argp);
	case RES_REQUEST_OPR:
		return opr_mgr_request_opr(data, argp);
	case RES_REQUEST_OPR_SYNC:
		return opr_mgr_request_opr_sync(data, argp);
	case RES_RELEASE_OPR:
		return opr_mgr_release_opr(data, argp);
	default:
		dpu_pr_debug("opr mgr unsupport cmd, need processed by other module");
		return -EINVAL;
	}

	return 0;
}
static void* opr_mgr_init(struct dpu_rm_data *rm_data)
{
	struct dpu_opr_mgr *opr_mgr = NULL;

	opr_mgr = kzalloc(sizeof(*opr_mgr), GFP_KERNEL);
	if (!opr_mgr)
		return NULL;

	sema_init(&opr_mgr->sem, 1);
	INIT_LIST_HEAD(&opr_mgr->opr_list);

	/* TODO: others */

	return NULL;
}

static void opr_mgr_deinit(void *data)
{
	struct dpu_opr_mgr *opr_mgr = (struct dpu_opr_mgr *)data;

	if (!opr_mgr)
		return;

	free_opr_list(&(opr_mgr->opr_list));

	kfree(opr_mgr);
	opr_mgr = NULL;

	data = NULL;
}

void dpu_rm_register_opr_mgr(struct list_head *resource_head)
{
	struct dpu_rm_resouce_node *opr_mgr_node = NULL;

	opr_mgr_node = kzalloc(sizeof(*opr_mgr_node), GFP_KERNEL);
	if (!opr_mgr_node)
		return;

	opr_mgr_node->init = opr_mgr_init;
	opr_mgr_node->deinit = opr_mgr_deinit;
	opr_mgr_node->ioctl = opr_mgr_ioctl;

	list_add_tail(&opr_mgr_node->list_node, resource_head);
}