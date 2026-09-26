/* Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * FileName: vltmm_agent.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/list.h>
#include <linux/sizes.h>

#include "vltmm.h"
#include "vltmm_agent.h"
#include "agent.h"
#include "tc_ns_log.h"
#include "teek_client_constants.h"
#include "teek_ns_client.h"

static bool vltmm_check_data(struct vltmm_msg_t *msg)
{
	if (!msg)
		return false;

	if (msg->magic != TEE_VLTMM_AGENT_ID) {
		pr_err("vltmm check magic error, now is 0x%x\n",
			msg->magic);
		return false;
	}

	return true;
}

static int vltmm_agent_work(struct tee_agent_kernel_ops *agent_instance)
{
	struct vltmm_msg_t *msg = NULL;

	if (!agent_instance || !agent_instance->agent_buff) {
		pr_err("agent buff invalid\n");
		return -1;
	}

	msg = (struct vltmm_msg_t *)agent_instance->agent_buff;
	if (!vltmm_check_data(msg)) {
		msg->ret = TEEC_ERROR_BAD_FORMAT;
		return -1;
	}

	switch (msg->cmd) {
	case VLTMM_CMD_ALLOC:
		msg->ret = 0;
		if (__smem_allocator_msg_alloc(msg->cid, msg->data,
				VLTMM_BUFF_LEN - sizeof(struct vltmm_msg_t),
				msg->num))
			msg->ret = TEEC_ERROR_BAD_FORMAT;
		break;
	case VLTMM_CMD_FREE:
		__smem_allocator_msg_free(msg->cid, msg->data, msg->num);
		msg->ret = 0;
		break;
	default:
		msg->ret = TEEC_ERROR_BAD_FORMAT;
		break;
	}

	return 0;
}

static int vltmm_agent_init(struct tee_agent_kernel_ops *agent_instance)
{
	pr_err("vltmm agent is init is being invoked\n");

	return 0;
}

static int vltmm_agent_exit(struct tee_agent_kernel_ops *agent_instance)
{
	pr_err("vltmm agent is exit is being invoked\n");

	return 0;
}

static int vltmm_agent_crash_work(
	struct tee_agent_kernel_ops *agent_instance,
	struct tc_ns_client_context *context, unsigned int dev_file_id)
{
	pr_err("vltmm agent crash work, dev_id=%u\n", dev_file_id);
	return 0;
}

static struct tee_agent_kernel_ops vltmm_agent_ops = {
	.agent_name = "vltmm",
	.agent_id = TEE_VLTMM_AGENT_ID,
	.tee_agent_init = vltmm_agent_init,
	.tee_agent_work = vltmm_agent_work,
	.tee_agent_exit = vltmm_agent_exit,
	.tee_agent_crash_work = vltmm_agent_crash_work,
	.agent_buff_size = VLTMM_BUFF_LEN,
	.list = LIST_HEAD_INIT(vltmm_agent_ops.list)
};

int vltmm_agent_register(void)
{
	pr_err("vltmm register\n\n");
	tee_agent_kernel_register(&vltmm_agent_ops);

	return 0;
}

