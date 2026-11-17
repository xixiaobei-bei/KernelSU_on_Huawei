/*
 * saudit.c
 *
 * security audit log functions
 *
 * Copyright (C) 2019-2019, Huawei Tech. Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <chipset_common/security/saudit.h>
#include <linux/time.h>
#include <linux/statfs.h>
#include <linux/stacktrace.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#define SAUDIT_STACK_DEPTH 16
#define SAUDIT_SELF_DEPTH 2

struct saudit_upload_work {
	struct work_struct worker;
	struct saudit_buffer sab;
};

static void saudit_log_vformat(struct saudit_buffer *sab, const char *fmt,
	va_list args)
{
	if (!sab)
		return;

	if (sab->offset >= SAUDIT_BUFSIZ - 1)
		return;
	sab->offset += vsnprintf(sab->info + sab->offset,
		SAUDIT_BUFSIZ - sab->offset - 1, fmt, args);
	if (sab->offset >= SAUDIT_BUFSIZ)
		sab->offset = SAUDIT_BUFSIZ - 1;
	sab->info[sab->offset] = '\0';
}

void saudit_log_format(struct saudit_buffer *sab, const char *fmt, ...)
{
	va_list args;

	if (!sab || !fmt)
		return;

	va_start(args, fmt);
	saudit_log_vformat(sab, fmt, args);
	va_end(args);
}

void saudit_log_stack(struct saudit_buffer *sab)
{
	unsigned long entries[SAUDIT_STACK_DEPTH] = {0};
	struct stack_trace trace = {
		.nr_entries = 0,
		.entries = entries,
		.max_entries = SAUDIT_STACK_DEPTH,
		.skip = SAUDIT_SELF_DEPTH,
	};

	if (!sab)
		return;

	if (sab->offset >= SAUDIT_BUFSIZ - 1)
		return;
	save_stack_trace(&trace);
	sab->offset += snprint_stack_trace(sab->info + sab->offset,
		SAUDIT_BUFSIZ - sab->offset - 1, &trace, 0);
	if (sab->offset >= SAUDIT_BUFSIZ)
		sab->offset = SAUDIT_BUFSIZ - 1;
	sab->info[sab->offset] = '\0';
}

struct saudit_buffer *saudit_log_start(unsigned int flags)
{
	struct saudit_upload_work *work_node = NULL;
	struct saudit_buffer *sab = NULL;

	if (flags & SAUDIT_ASYNC) {
		work_node = kzalloc(sizeof(*work_node), GFP_ATOMIC);
		if (work_node)
			sab = &work_node->sab;
	} else {
		sab = kzalloc(sizeof(*sab), GFP_ATOMIC);
	}
	return sab;
}

void saudit_log_end(struct saudit_buffer *sab)
{
	struct saudit_upload_work *work_node = NULL;

	if (!sab)
		return;

	if (kernel_stp_upload(sab->item, sab->info) < 0)
		pr_err("stp upload fail, %s\n", sab->item.name);
	if (sab->delay)
		mdelay(sab->delay);

	if (sab->flags & SAUDIT_ASYNC) {
		work_node = container_of(sab, struct saudit_upload_work, sab);
		kfree(work_node);
	} else {
		kfree(sab);
	}
}

static void saudit_log_end_work(struct work_struct *work)
{
	struct saudit_upload_work *work_node = NULL;

	work_node = container_of(work, struct saudit_upload_work, worker);
	saudit_log_end(&work_node->sab);
}

static void saudit_log_config(struct saudit_buffer *sab, int idx, int status,
	unsigned int flags)
{
	const struct stp_item_info *attr = NULL;

	if (!sab)
		return;

	sab->flags = flags;
	attr = get_item_info_by_idx(idx);
	if (!attr) {
		pr_err("get item attr %d fail\n", idx);
		return;
	}
	sab->item.id = attr->id;
	sab->item.status = status;
	sab->item.credible = (sab->flags & SAUDIT_REF) ?
		STP_REFERENCE : STP_CREDIBLE;
	(void)strncpy(sab->item.name, attr->name, STP_ITEM_NAME_LEN - 1);
	sab->item.name[STP_ITEM_NAME_LEN - 1] = '\0';

	switch (idx) {
	case KEY_FILES:
		sab->item.version = 1;
		break;
	case USERCOPY:
	case CFI:
		sab->flags |= SAUDIT_TRACE;
		sab->delay = 1000; /* 1s */
		break;
	case DOUBLE_FREE:
		sab->flags |= SAUDIT_TRACE;
		break;
	default:
		break;
	}
}

void saudit_log(int idx, int status, unsigned int flags, const char *fmt, ...)
{
	va_list args;
	struct timeval tv = {0};
	struct saudit_buffer *sab = NULL;
	struct saudit_upload_work *work_node = NULL;

	if (flags & SAUDIT_ATOMIC) {
		if (in_irq() || in_softirq() || irqs_disabled())
			return;
	}

	sab = saudit_log_start(flags);
	if (!sab) {
		pr_err("out of memory in saudit_log_start\n");
		return;
	}

	saudit_log_config(sab, idx, status, flags);

	do_gettimeofday(&tv);
	saudit_log_format(sab,
		"time=%ull,pid=%d,pcomm=%.20s,gid=%d,gcomm=%.20s,",
		(unsigned long long)tv.tv_sec, current->pid, current->comm,
		current->tgid, current->group_leader->comm);

	if (fmt) {
		va_start(args, fmt);
		saudit_log_vformat(sab, fmt, args);
		va_end(args);
	}

	if (sab->flags & SAUDIT_TRACE) {
		saudit_log_format(sab, ",stack=");
		saudit_log_stack(sab);
	}

	if (sab->flags & SAUDIT_ASYNC) {
		work_node = container_of(sab, struct saudit_upload_work, sab);
		INIT_WORK(&work_node->worker, saudit_log_end_work);
		queue_work(system_long_wq, &work_node->worker);
	} else {
		saudit_log_end(sab);
	}
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("provides saudit functions");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
