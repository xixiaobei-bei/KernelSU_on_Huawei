/*
 * cgroup_pids.c
 *
 * Process number limiting controller for cgroups.
 *
 * Used to allow a cgroup hierarchy to stop any new processes
 * from fork()ing after a certain limit is reached.
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/atomic.h>
#include <linux/cgroup.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/eventfd.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/version.h>

#define PIDS_MAX (PID_MAX_LIMIT + 1ULL)

struct pids_cgroup {
	struct cgroup_subsys_state css;

	/*
	 * Use 64-bit types so that we can safely represent "max" as
	 * (PID_MAX_LIMIT + 1).
	 */
	atomic64_t counter;
	int64_t limit;

	/* each group_pids is default limit */
	int64_t group_soft_limit;
	int64_t group_limit;
	/* The list of pid_event structs. */
	struct list_head event_list;
	/* Have to grab the lock on events traversal or modifications. */
	spinlock_t event_list_lock;
	struct list_head group_pids_list;

	int64_t token;
};

static struct pids_cgroup *css_pids(struct cgroup_subsys_state *css)
{
	return css ? container_of(css, struct pids_cgroup, css) : NULL;
}

static struct pids_cgroup *task_pids(struct task_struct *task)
{
	return css_pids(task_css(task, pids_cgrp_id));
}

static struct pids_cgroup *parent_pids(struct pids_cgroup *pids)
{
	return css_pids(pids->css.parent);
}

struct group_pids {
	/* All the tasks in the same QoS group are in this list */
	struct list_head head;
	/* Linked to the global group_pids_list */
	struct list_head node;
	int64_t soft_limit;
	int64_t limit;

	atomic64_t counter;
	int64_t token;
	/* true: has signal to user space, at default, it is false, no */
	bool signal;
};

static void pids_event(struct pids_cgroup *pids, struct group_pids *gp);
static DEFINE_SPINLOCK(group_pids_lock);

static int group_pids_try_charge(struct group_pids *gp, int num)
{
	int64_t new;
	int ret;

	if (!gp)
		return 0;

	new = atomic64_add_return(num, &gp->counter);
	if (new > gp->limit) {
		ret = atomic64_add_negative(-num, &gp->counter);
		return -ENOMEM;
	}
	if (new > gp->soft_limit) {
		/* send a pid over soft limit event to user space */
		return -EDQUOT;
	}
	return 0;
}

static void group_pids_uncharge(struct group_pids *gp, int num)
{
	if (!gp)
		return;
	WARN_ON_ONCE(atomic64_add_negative(-num, &gp->counter));
}

static void group_pids_migrate(struct task_struct *tsk, struct group_pids *gp)
{
	struct group_pids *old_gp = tsk->group_pids;

	if (old_gp) {
		list_del(&tsk->group_pids_list);
		atomic64_dec(&old_gp->counter);

		if (list_empty(&old_gp->head)) {
			list_del(&old_gp->node);
			kfree(old_gp);
		}
	}

	list_add_tail(&tsk->group_pids_list, &gp->head);
	atomic64_inc(&gp->counter);
	tsk->group_pids = gp;
}

static int group_pids_max_show(struct seq_file *m, void *v)
{
	struct pids_cgroup *pids = css_pids(seq_css(m));
	struct group_pids *gp = NULL;
	struct task_struct *task = NULL;

	spin_lock(&group_pids_lock);

	list_for_each_entry(gp, &pids->group_pids_list, node) {
		WARN_ON(list_empty(&gp->head));

		task = list_first_entry(&gp->head, struct task_struct,
					group_pids_list);
		seq_printf(m, "%llu %lu %llu %llu %llu\n",
			   (long long)gp->token,
			   (long)task_tgid_vnr(task),
			   (long long)gp->soft_limit,
			   (long long)gp->limit,
			   (long long)atomic64_read(&gp->counter));
	}

	spin_unlock(&group_pids_lock);
	return 0;
}

static int group_pids_limit_show(struct seq_file *m, void *v)
{
	struct pids_cgroup *pids = css_pids(seq_css(m));

	seq_printf(m, "%llu,%llu\n",
		   (long long)pids->group_soft_limit,
		   (long long)pids->group_limit);
	return 0;
}

static ssize_t group_pids_limit_write(struct kernfs_open_file *of,
				      char *buf, size_t nbytes, loff_t off)
{
	struct pids_cgroup *pids = css_pids(of_css(of));
	struct group_pids *gp = NULL;
	int64_t soft_limit = 0;
	int64_t max = 0;

	/* 2: the number of successful values returned */
	if (sscanf(buf, "%llu,%llu", &soft_limit, &max) != 2)
		return -EINVAL;

	if (soft_limit > max)
		return -EINVAL;

	pids->group_soft_limit = soft_limit;
	pids->group_limit = max;

	spin_lock(&group_pids_lock);

	list_for_each_entry(gp, &pids->group_pids_list, node) {
		gp->soft_limit = soft_limit;
		gp->limit = max;
	}

	spin_unlock(&group_pids_lock);
	return nbytes;
}

static struct cgroup_subsys_state *pids_css_alloc(struct cgroup_subsys_state *parent_css)
{
	struct pids_cgroup *pids = NULL;

	pids = kzalloc(sizeof(*pids), GFP_KERNEL);
	if (!pids)
		return ERR_PTR(-ENOMEM);

	pids->limit = PIDS_MAX;
	pids->group_soft_limit = PIDS_MAX;
	pids->group_limit = PIDS_MAX;
	atomic64_set(&pids->counter, 0);
	INIT_LIST_HEAD(&pids->group_pids_list);
	INIT_LIST_HEAD(&pids->event_list);
	spin_lock_init(&pids->event_list_lock);
	return &pids->css;
}

static void pids_css_free(struct cgroup_subsys_state *css)
{
	kfree(css_pids(css));
}

static void pids_cancel(struct pids_cgroup *pids, int num)
{
	WARN_ON_ONCE(atomic64_add_negative(-num, &pids->counter));
}

static void pids_uncharge(struct pids_cgroup *pids, int num)
{
	struct pids_cgroup *p = NULL;

	for (p = pids; p; p = parent_pids(p))
		pids_cancel(p, num);
}

static void pids_charge(struct pids_cgroup *pids, int num)
{
	struct pids_cgroup *p = NULL;

	for (p = pids; p; p = parent_pids(p))
		atomic64_add(num, &p->counter);
}

static int pids_try_charge(struct pids_cgroup *pids, int num)
{
	struct pids_cgroup *p = NULL;
	struct pids_cgroup *q = NULL;

	for (p = pids; p; p = parent_pids(p)) {
		int64_t new = atomic64_add_return(num, &p->counter);

		if (new > p->limit)
			goto revert;
	}

	return 0;

revert:
	for (q = pids; q != p; q = parent_pids(q))
		pids_cancel(q, num);
	pids_cancel(p, num);

	return -EAGAIN;
}

/* This is protected by cgroup lock */
static struct group_pids *tmp_gp;
static struct pids_cgroup *pids_attach_old_cs;

static int pids_can_attach(struct cgroup_taskset *tset)
{
	struct cgroup_subsys_state *css = NULL;
	struct pids_cgroup *pids = NULL;

	/* used later by pids_attach() */
	pids_attach_old_cs = task_pids(cgroup_taskset_first(tset, &css));
	pids = css_pids(css);

	WARN_ON(tmp_gp);
	tmp_gp = kzalloc(sizeof(*tmp_gp), GFP_KERNEL);
	if (!tmp_gp)
		return -ENOMEM;

	tmp_gp->token = pids->token++;
	tmp_gp->limit = PIDS_MAX;
	atomic64_set(&tmp_gp->counter, 0);
	INIT_LIST_HEAD(&tmp_gp->head);
	tmp_gp->soft_limit = pids->group_soft_limit;
	tmp_gp->limit = pids->group_limit;

	return 0;
}

static void pids_cancel_attach(struct cgroup_taskset *tset)
{
	kfree(tmp_gp);
	tmp_gp = NULL;
}

static void pids_attach(struct cgroup_taskset *tset)
{
	struct cgroup_subsys_state *css = NULL;
	struct pids_cgroup *pids = NULL;
	struct task_struct *task = NULL;
	int64_t num = 0;

	cgroup_taskset_first(tset, &css);
	pids = css_pids(css);

	spin_lock(&group_pids_lock);

	cgroup_taskset_for_each(task, css, tset) {
		num++;
		pids_uncharge(pids_attach_old_cs, 1);

		group_pids_migrate(task, tmp_gp);
	}
	list_add(&tmp_gp->node, &pids->group_pids_list);

	spin_unlock(&group_pids_lock);

	pids_charge(pids, num);

	tmp_gp = NULL;
}

int cgroup_pids_can_fork(void)
{
	struct pids_cgroup *pids = NULL;
	int ret;

	rcu_read_lock();
	pids = task_pids(current);
	rcu_read_unlock();
	ret = pids_try_charge(pids, 1);
	if (ret)
		return ret;

	spin_lock(&group_pids_lock);
	ret = group_pids_try_charge(current->group_pids, 1);
	if (ret == -ENOMEM) {
		pr_warn("Pid %d(%s) over pids cgroup hard_limit\n",
			task_tgid_vnr(current), current->comm);
		pids_uncharge(pids, 1);
	} else if (ret == -EDQUOT) {
		pids_event(pids, current->group_pids);
		ret = 0;
	}
	spin_unlock(&group_pids_lock);
	return ret;
}

void cgroup_pids_cancel_fork(void)
{
	struct pids_cgroup *pids = NULL;

	rcu_read_lock();
	pids = task_pids(current);
	rcu_read_unlock();
	pids_uncharge(pids, 1);

	spin_lock(&group_pids_lock);
	group_pids_uncharge(current->group_pids, 1);
	spin_unlock(&group_pids_lock);
}

static void pids_fork(struct task_struct *tsk)
{
	spin_lock(&group_pids_lock);
	if (current->group_pids) {
		tsk->group_pids = current->group_pids;
		list_add_tail(&tsk->group_pids_list,
			      &current->group_pids->head);
	}
	spin_unlock(&group_pids_lock);
}

static void pids_exit(struct task_struct *task)
{
	struct css_set *cset = NULL;
	struct cgroup_subsys_state *old_css = NULL;
	struct pids_cgroup *pids = NULL;

	cset = task_css_set(task);
	if (!cset)
		return;

	old_css = cset->subsys[pids_cgrp_id];
	pids = css_pids(old_css);
	pids_uncharge(pids, 1);

	spin_lock(&group_pids_lock);

	if (task->group_pids) {
		atomic64_dec(&task->group_pids->counter);
		list_del(&task->group_pids_list);

		if (list_empty(&task->group_pids->head)) {
			list_del(&task->group_pids->node);
			kfree(task->group_pids);
		}
	}

	spin_unlock(&group_pids_lock);
}

static int pids_max_write(struct cgroup_subsys_state *css,
			  struct cftype *cft, s64 max)
{
	struct pids_cgroup *pids = css_pids(css);

	if ((max < 0) || (max > INT_MAX))
		return -EINVAL;

	pids->limit = max;
	return 0;
}

static s64 pids_max_show(struct cgroup_subsys_state *css, struct cftype *cft)
{
	struct pids_cgroup *pids = css_pids(css);

	return pids->limit;
}

static s64 pids_current_read(struct cgroup_subsys_state *css,
			     struct cftype *cft)
{
	struct pids_cgroup *pids = css_pids(css);

	return atomic64_read(&pids->counter);
}

struct pids_event {
	struct eventfd_ctx *efd;
	struct list_head node;
};

static void pids_event(struct pids_cgroup *pids, struct group_pids *gp)
{
	struct pids_event *ev = NULL;

	/* is gp has signal to user space, protect by group_pids_lock */
	if (gp->signal)
		return;

	spin_lock(&pids->event_list_lock);
	list_for_each_entry(ev, &pids->event_list, node) {
		eventfd_signal(ev->efd, 1);
		gp->signal = true;
	}
	spin_unlock(&pids->event_list_lock);
}

static int group_pids_events_show(struct seq_file *m, void *v)
{
	struct pids_cgroup *pids = css_pids(seq_css(m));
	struct group_pids *gp = NULL;
	struct task_struct *task = NULL;
	int64_t count;

	spin_lock(&group_pids_lock);
	list_for_each_entry(gp, &pids->group_pids_list, node) {
		WARN_ON(list_empty(&gp->head));

		task = list_first_entry(&gp->head, struct task_struct,
					group_pids_list);

		count = atomic64_read(&gp->counter);
		if (count < gp->soft_limit)
			continue;

		seq_printf(m, "%llu %lu %lu %llu\n",
			   (long long)gp->token,
			   (long)task_tgid_vnr(task),
			   (long)from_kuid_munged(current_user_ns(), task_uid(task)),
			   (long long)count);
	}
	spin_unlock(&group_pids_lock);
	return 0;
}

struct pids_cgroup_event {
	/* memcg which the event belongs to. */
	struct pids_cgroup *pids;
	/* eventfd to signal userspace about the event. */
	struct eventfd_ctx *eventfd;
	/* Each of these stored in a list by the cgroup. */
	struct list_head list;
	int (*register_event)(struct pids_cgroup *pids,
			      struct eventfd_ctx *eventfd, const char *args);
	void (*unregister_event)(struct pids_cgroup *pids,
				  struct eventfd_ctx *eventfd);
	poll_table pt;
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	struct wait_queue_head *wqh;
	struct wait_queue_entry wait;
#else
	wait_queue_head_t *wqh;
	wait_queue_t wait;
#endif
	struct work_struct remove;
};

static int pids_cgroup_usage_register_event(struct pids_cgroup *pids,
					    struct eventfd_ctx *eventfd,
					    const char *args)
{
	struct pids_event *ev = NULL;

	ev = kzalloc(sizeof(*ev), GFP_KERNEL);
	if (!ev)
		return -ENOMEM;
	ev->efd = eventfd;

	spin_lock(&pids->event_list_lock);
	list_add(&ev->node, &pids->event_list);
	spin_unlock(&pids->event_list_lock);

	return 0;
}

static void pids_cgroup_usage_unregister_event(struct pids_cgroup *pids,
					       struct eventfd_ctx *eventfd)
{
	struct pids_event *ev = NULL;

	spin_lock(&pids->event_list_lock);
	list_for_each_entry(ev, &pids->event_list, node) {
		if (ev->efd != eventfd)
			continue;
		list_del(&ev->node);
		kfree(ev);
		break;
	}
	spin_unlock(&pids->event_list_lock);
}

static void pids_event_remove(struct work_struct *work)
{
	struct pids_cgroup_event *event = container_of(work, struct pids_cgroup_event, remove);
	struct pids_cgroup *pids = event->pids;

	remove_wait_queue(event->wqh, &event->wait);

	event->unregister_event(pids, event->eventfd);

	/* Notify userspace the event is going away. */
	eventfd_signal(event->eventfd, 1);

	eventfd_ctx_put(event->eventfd);
	kfree(event);
	css_put(&pids->css);
}

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
static int pids_event_wake(struct wait_queue_entry *wait, unsigned int mode,
#else
static int pids_event_wake(wait_queue_t *wait, unsigned int mode,
#endif
			   int sync, void *key)
{
	struct pids_cgroup_event *event = container_of(wait, struct pids_cgroup_event, wait);
	struct pids_cgroup *pids = event->pids;
	uintptr_t flags = (uintptr_t)key;

	if (flags & POLLHUP) {
		spin_lock(&pids->event_list_lock);
		if (!list_empty(&event->list)) {
			list_del_init(&event->list);
			schedule_work(&event->remove);
		}
		spin_unlock(&pids->event_list_lock);
	}

	return 0;
}

static void pids_event_ptable_queue_proc(struct file *file,
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
					 struct wait_queue_head *wqh,
#else
					 wait_queue_head_t *wqh,
#endif
					 poll_table *pt)
{
	struct pids_cgroup_event *event = container_of(pt, struct pids_cgroup_event, pt);

	event->wqh = wqh;
	add_wait_queue(wqh, &event->wait);
}

static ssize_t pids_write_event_control(struct kernfs_open_file *of,
					char *buf, size_t nbytes, loff_t off)
{
	struct cgroup_subsys_state *css = of_css(of);
	struct pids_cgroup *pids = css_pids(css);
	struct pids_cgroup_event *event = NULL;
	struct cgroup_subsys_state *cfile_css = NULL;
	unsigned int efd;
	unsigned int cfd;
	struct fd efile;
	struct fd cfile;
	const char *name = NULL;
	char *endp = NULL;
	int ret;

	buf = strstrip(buf);

	efd = simple_strtoul(buf, &endp, 10);
	if (*endp != ' ')
		return -EINVAL;
	buf = endp + 1;

	cfd = simple_strtoul(buf, &endp, 10);
	if ((*endp != ' ') && (*endp != '\0'))
		return -EINVAL;
	buf = endp + 1;

	event = kzalloc(sizeof(*event), GFP_KERNEL);
	if (!event)
		return -ENOMEM;

	event->pids = pids;
	INIT_LIST_HEAD(&event->list);
	init_poll_funcptr(&event->pt, pids_event_ptable_queue_proc);
	init_waitqueue_func_entry(&event->wait, pids_event_wake);
	INIT_WORK(&event->remove, pids_event_remove);

	efile = fdget(efd);
	if (!efile.file) {
		ret = -EBADF;
		goto out_kfree;
	}

	event->eventfd = eventfd_ctx_fileget(efile.file);
	if (IS_ERR(event->eventfd)) {
		ret = PTR_ERR(event->eventfd);
		goto out_put_efile;
	}

	cfile = fdget(cfd);
	if (!cfile.file) {
		ret = -EBADF;
		goto out_put_eventfd;
	}

	/* the process need read permission on control file */
	/* AV: shouldn't we check that it's been opened for read instead? */
	ret = inode_permission(file_inode(cfile.file), MAY_READ);
	if (ret < 0)
		goto out_put_cfile;

	name = cfile.file->f_path.dentry->d_name.name;

	if (!strcmp(name, "pids.group_event")) {
		event->register_event = pids_cgroup_usage_register_event;
		event->unregister_event = pids_cgroup_usage_unregister_event;
	} else {
		ret = -EINVAL;
		goto out_put_cfile;
	}

	cfile_css = css_tryget_online_from_dir(cfile.file->f_path.dentry->d_parent,
					       &pids_cgrp_subsys);
	ret = -EINVAL;
	if (IS_ERR(cfile_css))
		goto out_put_cfile;
	if (cfile_css != css) {
		css_put(cfile_css);
		goto out_put_cfile;
	}

	ret = event->register_event(pids, event->eventfd, buf);
	if (ret)
		goto out_put_css;

	efile.file->f_op->poll(efile.file, &event->pt);

	spin_lock(&pids->event_list_lock);
	list_add(&event->list, &pids->event_list);
	spin_unlock(&pids->event_list_lock);

	fdput(cfile);
	fdput(efile);

	return nbytes;

out_put_css:
	css_put(css);
out_put_cfile:
	fdput(cfile);
out_put_eventfd:
	eventfd_ctx_put(event->eventfd);
out_put_efile:
	fdput(efile);
out_kfree:
	kfree(event);

	return ret;
}

static void pids_css_offline(struct cgroup_subsys_state *css)
{
	struct pids_cgroup *pids = css_pids(css);
	struct pids_cgroup_event *event = NULL;
	struct pids_cgroup_event *tmp = NULL;

	spin_lock(&pids->event_list_lock);
	list_for_each_entry_safe(event, tmp, &pids->event_list, list) {
		list_del_init(&event->list);
		schedule_work(&event->remove);
	}
	spin_unlock(&pids->event_list_lock);
}

static struct cftype files[] = {
	{
	 .name = "max",
	 .write_s64 = pids_max_write,
	 .read_s64 = pids_max_show,
	 .flags = CFTYPE_NOT_ON_ROOT,
	},
	{
	 .name = "current",
	 .read_s64 = pids_current_read,
	},
	{
	 .name = "group_limit",
	 .seq_show = group_pids_limit_show,
	 .write = group_pids_limit_write,
	 .flags = CFTYPE_NOT_ON_ROOT,
	},
	{
	 .name = "group_tasks",
	 .seq_show = group_pids_max_show,
	 .flags = CFTYPE_NOT_ON_ROOT,
	},
	{
	 .name = "cgroup.event_control", /* XXX: for compat */
	 .write = pids_write_event_control,
	 .flags = CFTYPE_NO_PREFIX | CFTYPE_WORLD_WRITABLE,
	},
	{
	 .name = "group_event",
	 .seq_show = group_pids_events_show,
	 .flags = CFTYPE_NOT_ON_ROOT,
	},
	{} /* terminate */
};

struct cgroup_subsys pids_cgrp_subsys = {
	.css_alloc = pids_css_alloc,
	.css_offline = pids_css_offline,
	.css_free = pids_css_free,
	.can_attach = pids_can_attach,
	.cancel_attach = pids_cancel_attach,
	.attach = pids_attach,
	.fork = pids_fork,
	.exit = pids_exit,
	.legacy_cftypes = files,
	.early_init = 0,
};
