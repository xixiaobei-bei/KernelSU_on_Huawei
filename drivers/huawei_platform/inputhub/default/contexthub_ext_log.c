/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub ext log source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */
#include "contexthub_ext_log.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "linux/spinlock_types.h"
#include <linux/types.h>

#include <huawei_platform/log/hw_log.h>

#include "contexthub_route.h"

static struct inputhub_ext_log_notifier inputhub_ext_log_mag;
static bool inited;

int inputhub_ext_log_register_handler(int tag,
		int (*notify)(const struct pkt_header *head))
{
	struct inputhub_ext_notifier_node *pnode = NULL;
	struct inputhub_ext_notifier_node *n = NULL;
	int ret = 0;
	unsigned long flags = 0;

	if (!inited) {
		hwlog_err("%s not inited\n", __func__);
		return -EINVAL;
	}

	if ((!(tag >= TAG_BEGIN && tag < TAG_END)) || (!notify))
		return -EINVAL;

	spin_lock_irqsave(&inputhub_ext_log_mag.lock, flags);
	/* avoid regist more than once */
	list_for_each_entry_safe(pnode, n, &inputhub_ext_log_mag.head, entry) {
		if (tag == pnode->tag) {
			hwlog_warn("inputhub_ext_log tag = %d, notify = %pK has already registed in %s\n",
				tag, notify, __func__);
			goto out; /* return when already registed */
		}
	}

	/* make mcu_notifier_node */
	pnode = kzalloc(sizeof(struct inputhub_ext_notifier_node), GFP_ATOMIC);
	if (!pnode) {
		ret = -ENOMEM;
		goto out;
	}

	pnode->tag = tag;
	pnode->notify = notify;
	hwlog_info("%s tag %d registered\n", __func__, tag);
	/* add to list */
	list_add(&pnode->entry, &inputhub_ext_log_mag.head);
out:
	spin_unlock_irqrestore(&inputhub_ext_log_mag.lock, flags);

	return ret;
}

int is_inputhub_ext_log_notify(const struct pkt_header *head)
{
	return (head->tag == TAG_LOG_BUFF) &&
			(head->cmd == CMD_EXT_LOG_FLUSH);
}

static int inputhub_ext_log_process(const struct pkt_header *head)
{
	int found = 0;
	ext_logger_req_t *pkt_ext = (ext_logger_req_t *)head;
	/* search mcu_notifier, call all call_backs */
	struct inputhub_ext_notifier_node *pos = NULL;
	struct inputhub_ext_notifier_node *n = NULL;

	list_for_each_entry_safe(pos, n, &inputhub_ext_log_mag.head, entry) {
		if (pos->tag == pkt_ext->tag && pos->notify) {
			hwlog_info("%s tag %d handled\n", __func__, head->tag);
			pos->notify(head);
			found = 1;
		}
	}

	if (!found)
		hwlog_err("inputhub_ext_log_process tag %d not found\n", head->tag);

	return 0;
}

int inputhub_ext_log_init(void)
{
	INIT_LIST_HEAD(&inputhub_ext_log_mag.head);
	spin_lock_init(&inputhub_ext_log_mag.lock);
	register_mcu_event_notifier(TAG_LOG_BUFF, CMD_EXT_LOG_FLUSH,
		inputhub_ext_log_process);
	inited = true;
	hwlog_info("inputhub_ext_log_init success\n");
	return 0;
}

