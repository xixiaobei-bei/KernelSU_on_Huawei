/*
 *
 * mailbox driver event manager.
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/spinlock.h>
#include <linux/slab.h>
#include <securec.h>
#include <linux/notifier.h>
#include "ipc_mailbox.h"
#include "ipc_mailbox_event.h"

#define PR_LOG_TAG AP_MAILBOX_TAG

#undef pr_fmt
#define pr_fmt(fmt) "[ap_ipc]:" fmt

#define event_pr_err(fmt, args...) pr_err(fmt "\n", ##args)
#define event_pr_info(fmt, args...) pr_info(fmt "\n", ##args)
#define event_pr_debug(fmt, args...) pr_debug(fmt "\n", ##args)

struct ipc_mbox_event_mgr {
	spinlock_t notify_group_lock;
	struct atomic_notifier_head *event_notify_group[MAX_IPC_EVENT_NUM];
};

struct ipc_mbox_event_obj {
	struct hisi_mbox_device *mbox;
	const char *fmt_msg;
	va_list args;
};

struct ipc_mbox_notify_block {
	unsigned int event;
	struct notifier_block nb;
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args);
	struct list_head node;
};

static struct ipc_mbox_event_mgr g_mbox_event_mgr;

spinlock_t g_block_list_lock;
static LIST_HEAD(g_notifier_block_list);

/*lint -e773 */
#define def_mbox_event(_event) \
	[_event] = #_event
/*lint +e773 */

static char *g_all_event_name[MAX_IPC_EVENT_NUM] = {
	def_mbox_event(EVENT_IPC_SYNC_SEND_BEGIN),
	def_mbox_event(EVENT_IPC_SYNC_SEND_END),
	def_mbox_event(EVENT_IPC_SYNC_SEND_TIMEOUT),

	def_mbox_event(EVENT_IPC_SYNC_TASKLET_JAM),
	def_mbox_event(EVENT_IPC_SYNC_ISR_JAM),
	def_mbox_event(EVENT_IPC_SYNC_ACK_LOST),

	def_mbox_event(EVENT_IPC_SYNC_RECV_ACK),
	def_mbox_event(EVENT_IPC_SYNC_PROC_RECORD),
	def_mbox_event(EVENT_IPC_BEFORE_IRQ_TO_MDEV),
	def_mbox_event(EVENT_IPC_AFTER_IRQ_TO_MDEV),
	def_mbox_event(EVENT_IPC_RECV_BH),
	def_mbox_event(EVENT_IPC_SEND_BH),
	def_mbox_event(EVENT_MBOX_COMPLETE_STATUS_CHANGE),
	def_mbox_event(EVENT_IPC_ASYNC_TASK_IN_QUEUE),
	def_mbox_event(EVENT_IPC_ASYNC_TASK_SEND),
	def_mbox_event(EVENT_IPC_ASYNC_IN_QUEUE_FAIL),
	def_mbox_event(EVENT_IPC_MBX_STATUS_CHANGE),
	def_mbox_event(EVENT_IPC_DEACTIVATE_BEGIN),
};

static struct atomic_notifier_head *ipc_get_event_notifier(unsigned int event)
{
	struct ipc_mbox_event_mgr *event_mgr = &g_mbox_event_mgr;
	struct atomic_notifier_head *notifier = NULL;

	if (event >= MAX_IPC_EVENT_NUM)
		return NULL;

	spin_lock(&event_mgr->notify_group_lock);

	/* Find event notifier link */
	notifier = event_mgr->event_notify_group[event];
	if (notifier) {
		/* If exist event notifier link, then return */
		spin_unlock(&event_mgr->notify_group_lock);
		return notifier;
	}

	/* If not exist, then create and map new event notifier link */
	notifier = kzalloc(sizeof(struct atomic_notifier_head), GFP_ATOMIC);
	if (notifier)
		ATOMIC_INIT_NOTIFIER_HEAD(notifier);

	event_mgr->event_notify_group[event] = notifier;

	spin_unlock(&event_mgr->notify_group_lock);

	return notifier;
}

static inline struct atomic_notifier_head *ipc_get_exist_event_notifier(
	unsigned int event)
{
	return (event >= MAX_IPC_EVENT_NUM) ?
		NULL : g_mbox_event_mgr.event_notify_group[event];
}

static int ipc_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct ipc_mbox_notify_block *mbox_nb = NULL;
	struct ipc_mbox_event_obj *event_obj = NULL;

	mbox_nb = container_of(nb, struct ipc_mbox_notify_block, nb);
	if (!mbox_nb) {
		event_pr_info("not find mbox notify block");
		return EVENT_SUCCESS;
	}

	event_obj = (struct ipc_mbox_event_obj *)data;
	mbox_nb->event_handle(event_obj->mbox, event,
		event_obj->fmt_msg, event_obj->args);

	return EVENT_SUCCESS;
}

static struct ipc_mbox_notify_block *ipc_get_one_event_nb(unsigned int event)
{
	struct ipc_mbox_notify_block *mbox_nb = NULL;
	struct list_head *block_list = &g_notifier_block_list;

	if (list_empty(block_list))
		return NULL;

	list_for_each_entry(mbox_nb, block_list, node) {
		if (mbox_nb && mbox_nb->event == event)
			return mbox_nb;
	}

	return NULL;
}

void ipc_clear_mailbox_event_notifier(unsigned int event)
{
	struct ipc_mbox_notify_block *mbox_nb = NULL;
	struct atomic_notifier_head *notifier = NULL;

	notifier = ipc_get_exist_event_notifier(event);
	if (!notifier) {
		event_pr_err("invalid event:%u in event unregist", event);
		return;
	}

	spin_lock(&g_block_list_lock);
	mbox_nb = ipc_get_one_event_nb(event);
	while (mbox_nb) {
		atomic_notifier_chain_unregister(notifier, &mbox_nb->nb);
		list_del(&mbox_nb->node);
		kfree(mbox_nb);
		mbox_nb = ipc_get_one_event_nb(event);
	}
	spin_unlock(&g_block_list_lock);

	kfree(notifier);
}

int ipc_unregist_mailbox_event(unsigned int event,
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args))
{
	struct ipc_mbox_notify_block *mbox_nb = NULL;
	struct atomic_notifier_head *notifier = NULL;
	struct list_head *block_list = &g_notifier_block_list;

	if (!event_handle) {
		event_pr_err("event handle is NULL");
		return ER_NULL_EVENT_HANDLE;
	}

	notifier = ipc_get_exist_event_notifier(event);
	if (!notifier) {
		event_pr_err("invalid event:%u in event unregist", event);
		return ER_INVALID_EVENT_NUM;
	}

	if (list_empty(block_list) || block_list->next == LIST_POISON1)
		return 0;

	spin_lock(&g_block_list_lock);
	list_for_each_entry(mbox_nb, block_list, node) {
		if (mbox_nb && mbox_nb->event == event &&
			mbox_nb->event_handle == event_handle)
			break;
	}

	if (!mbox_nb) {
		spin_unlock(&g_block_list_lock);
		return 0;
	}

	list_del(&mbox_nb->node);
	spin_unlock(&g_block_list_lock);

	atomic_notifier_chain_unregister(notifier, &mbox_nb->nb);
	kfree(mbox_nb);

	return 0;
}

static struct ipc_mbox_notify_block *ipc_new_mbox_event_notify_block(
	unsigned int event,
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args))
{
	struct ipc_mbox_notify_block *mbox_nb = NULL;

	mbox_nb = kzalloc(sizeof(struct ipc_mbox_notify_block), GFP_KERNEL);
	if (!mbox_nb)
		return NULL;

	mbox_nb->event = event;
	mbox_nb->event_handle = event_handle;
	mbox_nb->nb.next = NULL;
	mbox_nb->nb.notifier_call = ipc_event_notifier_call;

	/* Add to g_notifier_block_list for unregist event notifier */
	list_add_tail(&mbox_nb->node, &g_notifier_block_list);

	return mbox_nb;
}

int ipc_regist_mailbox_event(unsigned int event,
	void (*event_handle)(struct hisi_mbox_device *mbox,
		unsigned int event, const char *fmt_msg, va_list args))
{
	struct ipc_mbox_notify_block *mbox_nb = NULL;
	struct atomic_notifier_head *notifier = NULL;
	int ret;

	if (!event_handle) {
		event_pr_err("event handle is NULL");
		return ER_NULL_EVENT_HANDLE;
	}

	notifier = ipc_get_event_notifier(event);
	if (!notifier) {
		event_pr_err("invalid event:%u in event regist", event);
		return ER_INVALID_EVENT_NUM;
	}

	mbox_nb = ipc_new_mbox_event_notify_block(event, event_handle);
	if (!mbox_nb) {
		event_pr_err("alloc mbox notifier block failed");
		return ER_ALLOC_NOTIFIER_FAILED;
	}

	ret = atomic_notifier_chain_register(notifier, &mbox_nb->nb);
	if (ret) {
		event_pr_err("regist event notifier failed:%d", ret);
		return ER_NOTIFIER_REGIST_FAILED;
	}

	return EVENT_SUCCESS;
}

char *ipc_get_event_name(unsigned int event)
{
	if (event >= MAX_IPC_EVENT_NUM)
		return "UNKOWN_EVENT";

	return g_all_event_name[event] ?
		g_all_event_name[event] : "Not cfg event";
}

void ipc_notify_mailbox_event(unsigned int event,
	struct hisi_mbox_device *mbox, const char *fmt_msg, ...)
{
	struct atomic_notifier_head *notifier = NULL;
	struct ipc_mbox_event_obj event_obj;
	int ret;
	va_list args;

	if (!mbox) {
		event_pr_err("mbox or fmt_msg is null");
		return;
	}

	notifier = ipc_get_exist_event_notifier(event);
	if (!notifier) {
		event_pr_debug("unused event:%u:%s in event notify", event,
			ipc_get_event_name(event));
		return;
	}

	ret = memset_s(&event_obj, sizeof(struct ipc_mbox_event_obj),
		0, sizeof(struct ipc_mbox_event_obj));
	if (ret != EOK) {
		event_pr_err("memset event obj failed");
		return;
	}

	va_start(args, fmt_msg);
	event_obj.mbox = mbox;
	event_obj.fmt_msg = fmt_msg;
	event_obj.args = args;
	ret = atomic_notifier_call_chain(notifier, event, &event_obj);
	if (ret) {
		event_pr_err("notifier call failed:%d", ret);
		va_end(args);
		return;
	}
	va_end(args);
}

static int __init ipc_mbox_event_init(void)
{
	spin_lock_init(&g_mbox_event_mgr.notify_group_lock);
	spin_lock_init(&g_block_list_lock);
	return EVENT_SUCCESS;
}
module_init(ipc_mbox_event_init);

static void __exit ipc_mbox_event_exit(void)
{
}
module_exit(ipc_mbox_event_exit);
