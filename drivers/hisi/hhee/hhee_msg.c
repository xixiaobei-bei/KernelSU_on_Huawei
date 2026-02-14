/*
 * Hisilicon HHEE exception driver .
 *
 * Copyright (c) 2012-2013 Linaro Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/errno.h>
#include <linux/io.h>
#include <linux/interrupt.h>

#include "hhee.h"
#include "hhee_msg.h"

static hhee_callback_t msg_callbacks[HHEE_MSG_ID_MAX];

static struct hhee_msg_state *msg_state;

static void handle_message(struct hhee_msg *msg)
{
	unsigned int id = msg->id;
	unsigned int len = msg->len;

	if (id >= HHEE_MSG_ID_MAX || len > HHEE_MSG_MAX_PAYLOAD) {
		pr_err("unexpected msg, seq=%llu, id=%u, len=%u\n",
			msg->seq, id, len);
		return;
	}

	if (!msg_callbacks[id]) {
		pr_err("unhandled msg, seq=%llu, id=%u, len=%u\n",
			msg->seq, id, len);
		return;
	}

	pr_debug("process msg, seq=%llu, id=%u, len=%u\n",
		msg->seq, id, len);

	msg_callbacks[id](len, (void *)msg->payload);
}

static void hhee_msg_handle(unsigned int start, unsigned int end)
{
	struct hhee_msg *msg = NULL;

	while (start != end) {
		msg = (struct hhee_msg *)((uintptr_t)msg_state +
			HHEE_MSG_SIZE * ((uintptr_t)start + 1));
		handle_message(msg);
		start++;
		if (start >= HHEE_MSG_MAX_SLOTS)
			start = 0;
	}
}

irqreturn_t hhee_irq_handle(int irq, void *data)
{
	unsigned int start, end;

	if (!msg_state)
		return -IRQ_HANDLED;

	start = msg_state->read_pos;
	end = msg_state->write_pos;

	if (start >= HHEE_MSG_MAX_SLOTS || end >= HHEE_MSG_MAX_SLOTS) {
		pr_err("unexpected error, pos=%u,%u\n", start, end);
		return IRQ_HANDLED;
	}

	hhee_msg_handle(start, end);
	msg_state->read_pos = end;

	return IRQ_HANDLED;
}

int hhee_msg_init(void)
{
	struct arm_smccc_res ret_res;
	uintptr_t buf_base;
	uint64_t buf_size;
	void *addr;

	int ret;

	ret_res = hhee_fn_hvc((unsigned long)HHEE_IRQ_INFOBUF, 0ul, 0ul, 0ul);
	buf_base = ret_res.a0;
	buf_size = ret_res.a1;

	if (!buf_base || buf_size != (HHEE_MSG_MAX_SLOTS + 1) * HHEE_MSG_SIZE) {
		ret = -EFAULT;
		goto err;
	}

	addr = ioremap_cache(buf_base, buf_size);
	if (!addr) {
		ret = -EFAULT;
		goto err;
	}

	msg_state = (struct hhee_msg_state *)addr;
	return 0;

err:
	pr_err("init error.\n");
	return ret;
}

int hhee_msg_register_handler(unsigned int id, hhee_callback_t fn)
{
	if (!fn || id >= HHEE_MSG_ID_MAX)
		return -EINVAL;

	if (!msg_state)
		return -EFAULT;

	msg_callbacks[id] = fn;
	msg_state->enabled[id] = 1;

	return 0;
}
