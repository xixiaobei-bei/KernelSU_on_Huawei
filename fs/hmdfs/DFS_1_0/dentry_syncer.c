/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dentry_syncer.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: wangminmin4@huawei.com
 * Create: 2020-04-25
 *
 */

#include "dentry_syncer.h"

#include <linux/cdev.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/timer.h>
#include <linux/wait.h>

#include "hmdfs.h"

/*Iterate maximum number of requests at a time */
#define ITERATE_MAX_NUM		     10000
#define DENTRY_SYNCER_UPCALL_TIMEOUT 2
#define SYNCER_TASK_ID_START	     0
#define DIR_POS_MASK_BIT	     (POS_BIT_NUM - 1 - DEV_ID_BIT_NUM - 1)
#define ITERATE_POS_MASK	     GENMASK_ULL(DIR_POS_MASK_BIT, 0)

struct cdev_info {
	dev_t devno;
	struct cdev *cdev;
	struct class *class;
	struct device *device;
};

struct syncer_task {
	struct syncer_req *task_req;
	struct syncer_req *ret_req;
	struct completion completion;
	struct list_head list;
	struct timer_list task_timer;
	struct kref ref;
};

static DECLARE_WAIT_QUEUE_HEAD(syncer_wait);
static LIST_HEAD(syncer_ordinary_list);
static LIST_HEAD(syncer_prior_list);
static struct mutex read_mutex;
static struct mutex write_mutex;
static spinlock_t list_lock;
static struct cdev_info syncer_cdev;

static atomic_t global_ordinary_id = ATOMIC_INIT(SYNCER_TASK_ID_START);
static atomic_t global_prior_id = ATOMIC_INIT(SYNCER_TASK_ID_START);
static atomic_t pending_task_start_count = ATOMIC_INIT(SYNCER_TASK_ID_START);

static bool list_cleaning;

static unsigned long last_op_time;
#define WAKEUP_INTERVAL (60 * HZ)

static bool add_dentry_sync_task(struct syncer_task *syncer_task,
				 struct list_head *head)
{
	spin_lock_bh(&list_lock);
	if (syncer_task->task_req->status == START)
		atomic_inc(&pending_task_start_count);
	list_add_tail(&syncer_task->list, head);
	spin_unlock_bh(&list_lock);
	wake_up(&syncer_wait);
	return true;
}

static struct syncer_task *find_and_remove_task(struct list_head *head,
						__u32 id)
{
	struct syncer_task *syncer_task = NULL;
	struct syncer_task *tmp = NULL;

	spin_lock_bh(&list_lock);
	list_for_each_entry_safe(syncer_task, tmp, head, list) {
		if (syncer_task->task_req->reqid == id) {
			list_del(&syncer_task->list);
			kref_get(&syncer_task->ref);
			if (syncer_task->task_req->status == START)
				atomic_dec(&pending_task_start_count);
			spin_unlock_bh(&list_lock);
			return syncer_task;
		}
	}
	spin_unlock_bh(&list_lock);
	return NULL;
}

static struct syncer_task *lookup_and_trans_task(struct list_head *head,
						 int status)
{
	struct syncer_task *syncer_task = NULL;
	struct syncer_task *tmp = NULL;

	spin_lock_bh(&list_lock);
	list_for_each_entry_safe(syncer_task, tmp, head, list) {
		if (syncer_task->task_req->status == status) {
			kref_get(&syncer_task->ref);
			syncer_task->task_req->status = WORKING;
			atomic_dec(&pending_task_start_count);
			spin_unlock_bh(&list_lock);
			return syncer_task;
		}
	}
	spin_unlock_bh(&list_lock);
	return NULL;
}

static void release_task(struct kref *ref)
{
	struct syncer_task *tsk = container_of(ref, struct syncer_task, ref);

	kfree(tsk->task_req);
	kfree(tsk->ret_req);
	kfree(tsk);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void timer_call_back(unsigned long id)
{
	struct syncer_task *task = NULL;

	if (list_cleaning)
		return;

	hmdfs_debug("trigger task %lu", id);
	task = find_and_remove_task(&syncer_ordinary_list, (__u32)id);
	if (unlikely(!task)) {
		hmdfs_err("cannot find task %lu!", id);
		return;
	}

	/* skip removing task if it's already being read */
	if (task->task_req->status != START) {
		kref_put(&task->ref, release_task);
		return;
	}

	/* release initial refcount */
	kref_put(&task->ref, release_task);
	hmdfs_debug("delete task %lu success", id);
}
#else
static void timer_call_back(struct timer_list *timer)
{
	struct syncer_task *task =
		container_of(timer, struct syncer_task, task_timer);
	__u32 id = task->task_req->reqid;

	if (list_cleaning)
		return;

	/* skip removing task if it's already being read */
	if (task->task_req->status != START)
		return;

	hmdfs_debug("trigger task %u", id);
	task = find_and_remove_task(&syncer_ordinary_list, id);
	if (unlikely(!task)) {
		hmdfs_err("cannot find task %u!", id);
		return;
	}
	kref_put(&task->ref, release_task);
	/* release initial refcount */
	kref_put(&task->ref, release_task);
	hmdfs_debug("delete task %u success", id);
}
#endif

static void init_dentry_task_timer(struct syncer_task *task)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer(&task->task_timer);
	task->task_timer.expires = jiffies + 30 * HZ;
	task->task_timer.data = (unsigned long)task->task_req->reqid;
	task->task_timer.function = &timer_call_back;
	add_timer(&task->task_timer);
#else
	timer_setup(&task->task_timer, &timer_call_back, 0);
	task->task_timer.expires = jiffies + 30 * HZ;
	task->task_timer.function = &timer_call_back;
	add_timer(&task->task_timer);
#endif
}

static int handle_read_task(struct list_head *head, struct syncer_task *tmp,
			    char __user *ubuf, size_t size)
{
	int ret = 0;
	size_t len;

	hmdfs_debug("id %u type %d", tmp->task_req->reqid, tmp->task_req->type);

	len = sizeof(struct syncer_req) + tmp->task_req->len;
	if (size < len) {
		hmdfs_err("userspace buffer size less than task len");
		ret = -ENOMEM;
		goto release_task;
	}

	if (copy_to_user(ubuf, tmp->task_req, len)) {
		hmdfs_err("copy_to_user error, reqid %d", tmp->task_req->reqid);
		ret = -EFAULT;
		tmp->task_req->status = START;
		atomic_inc(&pending_task_start_count);
		return ret;
	}

	/* Only async tasks should be released here */
	if (tmp->task_req->type == DENTRY_ITERATE_REQUEST ||
	    tmp->task_req->type == DENTRY_LOOKUP_REQUEST)
		return ret;

release_task:
	/* Release initial refcount only if timer is deactivated successfully */
	if (del_timer_sync(&tmp->task_timer))
		kref_put(&tmp->ref, release_task);
	if (find_and_remove_task(head, tmp->task_req->reqid))
		kref_put(&tmp->ref, release_task);
	return ret;
}

static ssize_t read_dentry_syncer(struct file *file, char __user *ubuf,
				  size_t size, loff_t *off)
{
	int ret = 0;
	struct syncer_task *tmp = NULL;

	mutex_lock(&read_mutex);
	hmdfs_debug("for pending_task_start_count is %d",
		    atomic_read(&pending_task_start_count));

	if (!list_empty(&syncer_prior_list)) {
		tmp = lookup_and_trans_task(&syncer_prior_list, START);
		if (tmp) {
			hmdfs_debug("start read prior task");
			ret = handle_read_task(&syncer_prior_list, tmp, ubuf,
					       size);
			kref_put(&tmp->ref, release_task);
			goto out_read;
		}
	}
	if (!list_empty(&syncer_ordinary_list)) {
		tmp = lookup_and_trans_task(&syncer_ordinary_list, START);
		if (tmp) {
			hmdfs_debug("start read ordinary task");
			ret = handle_read_task(&syncer_ordinary_list, tmp, ubuf,
					       size);
			kref_put(&tmp->ref, release_task);
			goto out_read;
		}
	}

	hmdfs_warning("all list is null or no exit task");
	ret = -EPERM;
out_read:
	last_op_time = jiffies;
	mutex_unlock(&read_mutex);
	return ret;
}

static void handle_task_state(struct syncer_task *task, struct syncer_req *tmp)
{
	task->ret_req = tmp;

	switch (task->ret_req->status) {
	case SUCCESS:
		hmdfs_debug("meta_syncer task success, reqid %d type %d",
			    task->ret_req->reqid, task->ret_req->type);
		break;
	case FAIL:
		hmdfs_debug("meta_syncer task fail, reqid %d type %d",
			    task->ret_req->reqid, task->ret_req->type);
		break;
	default:
		hmdfs_err("unknown return status! reqid %d type %d",
			  task->ret_req->reqid, task->ret_req->type);
	}
}

static ssize_t write_dentry_syncer(struct file *file, const char __user *ubuf,
				   size_t size, loff_t *off)
{
	int ret = 0;
	struct syncer_task *task = NULL;
	struct syncer_req *tmp = NULL;

	mutex_lock(&write_mutex);
	if (list_empty(&syncer_prior_list) &&
	    list_empty(&syncer_ordinary_list)) {
		hmdfs_err("all list is null");
		ret = -EPERM;
		goto out_write;
	}
	/* 1 represent dentry name type */
	if (size > (sizeof(struct syncer_req) +
		    ITERATE_MAX_NUM * (NAME_MAX + 1)) ||
	    size < sizeof(struct syncer_req)) {
		hmdfs_err("size is not legal");
		ret = -EFAULT;
		goto out_write;
	}
	tmp = kzalloc(size, GFP_KERNEL);
	if (!tmp) {
		ret = -ENOMEM;
		goto out_write;
	}

	if (copy_from_user(tmp, ubuf, size)) {
		kfree(tmp);
		ret = -EFAULT;
		hmdfs_err("meta_write copy_from_user error");
		goto out_write;
	} else {
		if (size - sizeof(struct syncer_req) < tmp->len) {
			hmdfs_err("size is %zd, while syncer_req->len is %d",
				  size, tmp->len);
			ret = -EINVAL;
			kfree(tmp);
			goto out_write;
		}
		switch (tmp->type) {
		case DENTRY_LOOKUP_RESPONSE:
		case DENTRY_ITERATE_RESPONSE:
			task = find_and_remove_task(&syncer_prior_list,
						    tmp->reqid);
			if (!task) {
				hmdfs_err("prior_list no this task");
				kfree(tmp);
				goto out_write;
			}
			handle_task_state(task, tmp);
			complete(&task->completion);
			kref_put(&task->ref, release_task);
			break;
		default:
			hmdfs_err("meta_syncer return type is bad");
			kfree(tmp);
		}
	}

out_write:
	last_op_time = jiffies;
	mutex_unlock(&write_mutex);
	return ret;
}

static unsigned int poll_dentry_syncer(struct file *file,
				       struct poll_table_struct *poll_table)
{
	poll_wait(file, &syncer_wait, poll_table);
	if (atomic_read(&pending_task_start_count))
		return POLLIN | POLLRDNORM;
	return 0;
}

static const struct file_operations dentry_syncer_fops = {
	.read = read_dentry_syncer,
	.write = write_dentry_syncer,
	.poll = poll_dentry_syncer,
};

static int verify_and_copy_lookup_res(struct syncer_task *task,
				      struct metabase *result, int len)
{
	uint64_t total_len = 0;
	int buf_len = 0;
	struct metabase *info = NULL;

	buf_len = task->ret_req->len;
	if (buf_len <= sizeof(struct metabase)) {
		hmdfs_err("ret_req->len is too small %u, struct metabase is %u",
			  buf_len, (unsigned int)sizeof(struct metabase));
		return -EIO;
	}

	info = (struct metabase *)(task->ret_req->req_buf);
	total_len = sizeof(struct metabase) + (uint64_t)info->name_len +
		    (uint64_t)info->path_len;
	if (total_len >= INT_MAX) {
		hmdfs_err("total_len too long, struct metabase %u + name_len %u + path_len %u",
			  (unsigned int)sizeof(struct metabase), info->name_len,
			  info->path_len);
		return -EINVAL;
	}
	if (buf_len != total_len) {
		hmdfs_err("struct metabase %u + name_len %u + path_len %u do not match ret_req->len %u",
			  (unsigned int)sizeof(struct metabase), info->name_len,
			  info->path_len, buf_len);
		return -EIO;
	}

	if (len < total_len) {
		hmdfs_err("target too short %d, expected %d", len,
			  (int)total_len);
		return -EIO;
	}

	memcpy(result, info, total_len);
	return 0;
}

int dentry_syncer_lookup(struct lookup_request *lookup_req,
			 struct metabase *result, int len)
{
	int err = 0;
	int buf_len;
	__u32 id;
	struct syncer_task *tmp_task = NULL;
	struct syncer_req *tmp_req = NULL;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_KERNEL);
	if (!tmp_task) {
		err = -ENOMEM;
		return err;
	}

	buf_len = sizeof(struct lookup_request) + lookup_req->name_len +
		  lookup_req->path_len;
	tmp_req = kmalloc(sizeof(*tmp_req) + buf_len, GFP_KERNEL);
	if (!tmp_req) {
		hmdfs_err("alloc task_req fail");
		kfree(tmp_task);
		err = -ENOMEM;
		return err;
	}

	tmp_req->len = buf_len;
	memcpy(tmp_req->req_buf, lookup_req, buf_len);
	tmp_req->type = DENTRY_LOOKUP_REQUEST;
	tmp_req->status = START;
	id = (__u32)atomic_inc_return(&global_prior_id);
	tmp_req->reqid = id;
	tmp_task->task_req = tmp_req;
	kref_init(&tmp_task->ref);

	init_completion(&tmp_task->completion);
	if (!add_dentry_sync_task(tmp_task, &syncer_prior_list)) {
		kref_put(&tmp_task->ref, release_task);
		err = -ENOSPC;
		return err;
	}

	if (!wait_for_completion_timeout(&tmp_task->completion,
					 DENTRY_SYNCER_UPCALL_TIMEOUT * HZ)) {
		err = -EIO;
		goto out_err;
	}

	if (tmp_task->ret_req->status == SUCCESS) {
		err = verify_and_copy_lookup_res(tmp_task, result, len);
	} else {
		hmdfs_debug("failed to lookup the given path");
		err = -ERANGE;
	}

out_err:
	if (find_and_remove_task(&syncer_prior_list, id))
		kref_put(&tmp_task->ref, release_task);
	kref_put(&tmp_task->ref, release_task);
	return err;
}

static int do_single_iterate_req(__u32 index, __u64 dev_id, const char *path,
				 struct syncer_task *tmp_task)
{
	int err = 0;
	__u32 id;
	struct iterate_request *iterate_req =
		(struct iterate_request *)&tmp_task->task_req->req_buf;

	iterate_req->start = index;
	iterate_req->count = ITERATE_MAX_NUM;
	iterate_req->device_id = dev_id;
	iterate_req->path_len = strlen(path);
	memcpy(iterate_req->path, path, strlen(path));

	id = (__u32)atomic_inc_return(&global_prior_id);
	tmp_task->task_req->reqid = id;

	init_completion(&tmp_task->completion);
	if (!add_dentry_sync_task(tmp_task, &syncer_prior_list)) {
		err = -ENOSPC;
		return err;
	}

	if (!wait_for_completion_timeout(&tmp_task->completion,
					 DENTRY_SYNCER_UPCALL_TIMEOUT * HZ)) {
		err = -EIO;
		hmdfs_err("task timeout!");
	}

	if (find_and_remove_task(&syncer_prior_list, id))
		kref_put(&tmp_task->ref, release_task);

	return err;
}

static int loop_iterate_reqbuf(struct syncer_task *tmp_task, int *count,
			       struct dir_context *ctx, struct file *file)
{
	int err = 0;
	int pos;
	int index_first = -1;
	int index_end = -1;
	char *iterator_response = NULL;
	char *lookup_result = NULL;
	char *dentry_name = NULL;
	int len;
	unsigned int file_type;

	iterator_response = (char *)(tmp_task->ret_req->req_buf);
	for (pos = 0; pos < tmp_task->ret_req->len; pos++) {
		if (iterator_response[pos] != '\0' && index_first == -1)
			index_first = pos;
		if (iterator_response[pos] == '\0' && index_end == -1)
			index_end = pos;
		if (index_first < index_end) {
			lookup_result = iterator_response + index_first;
			dentry_name = lookup_result + 1;
			len = index_end - index_first - 1;
			file_type =
				(*lookup_result - '0' == 0 ? DT_REG : DT_DIR);
			if (!dir_emit(ctx, dentry_name, len, 0, file_type)) {
				err = 1;
				return err;
			}
			(*count)++;
			hmdfs_debug("%d", *count);
			ctx->pos += 1;
			index_first = -1;
			index_end = -1;
		}
	}
	return err;
}

static int handle_iterate_response(struct syncer_task *tmp_task, int *count,
				   int *index, bool *status,
				   struct dir_context *ctx, struct file *file)
{
	int err = 0;

	if (tmp_task->ret_req->status == SUCCESS &&
	    tmp_task->ret_req->len > 0) {
		err = loop_iterate_reqbuf(tmp_task, count, ctx, file);
		if (err) {
			hmdfs_info("dir_emit is full");
			*status = false;
			return err;
		}
	}

	if (tmp_task->ret_req->status == SUCCESS &&
	    tmp_task->ret_req->len == 0) {
		*status = false;
		hmdfs_debug("iterator result is empty");
	}

	if (tmp_task->ret_req->status == FAIL) {
		hmdfs_debug("iterator task return FAIL");
		*status = false;
		err = -ENOMSG;
	}

	if (*count == ITERATE_MAX_NUM) {
		*count = 0;
		*index = *index + ITERATE_MAX_NUM;
	} else {
		*status = false;
		hmdfs_debug("actual_count %d", *count);
	}
	return err;
}

int dentry_syncer_iterate(struct file *file, __u64 dev_id, const char *path,
			  struct dir_context *ctx)
{
	__u32 index = (unsigned long long)ctx->pos & ITERATE_POS_MASK;
	int actual_count = 0;
	int err = 0;
	struct syncer_task *tmp_task = NULL;
	struct syncer_req *tmp_req = NULL;
	bool status = true;
	int buf_len;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_KERNEL);
	if (!tmp_task) {
		err = -ENOMEM;
		return err;
	}
	kref_init(&tmp_task->ref);

	buf_len = sizeof(struct iterate_request) + strlen(path);
	tmp_req = kmalloc(sizeof(*tmp_req) + buf_len, GFP_KERNEL);
	if (!tmp_req) {
		hmdfs_err("alloc iterate_task_tmp fail");
		kfree(tmp_task);
		err = -ENOMEM;
		return err;
	}
	tmp_req->len = buf_len;
	tmp_req->type = DENTRY_ITERATE_REQUEST;
	tmp_req->status = START;
	tmp_task->task_req = tmp_req;

	while (status) {
		err = do_single_iterate_req(index, dev_id, path, tmp_task);
		if (err) {
			hmdfs_err("iterate task %d fail, err %d",
				  tmp_task->task_req->reqid, err);
			goto out_err;
		}
		err = handle_iterate_response(tmp_task, &actual_count, &index,
					      &status, ctx, file);
		kfree(tmp_task->ret_req);
		tmp_task->ret_req = NULL;
	}

out_err:
	kref_put(&tmp_task->ref, release_task);
	return err;
}

void dentry_syncer_create(struct metabase *create_req)
{
	int buf_len;
	struct syncer_task *tmp_task = NULL;
	struct syncer_req *tmp_req = NULL;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_KERNEL);
	if (!tmp_task)
		return;

	buf_len = sizeof(struct metabase) + create_req->path_len +
		  create_req->name_len + create_req->src_path_len;
	tmp_req = kmalloc(sizeof(*tmp_req) + buf_len, GFP_KERNEL);
	if (!tmp_req) {
		hmdfs_err("alloc create_task_req fail");
		kfree(tmp_task);
		return;
	}

	tmp_req->len = buf_len;
	memcpy(tmp_req->req_buf, create_req, buf_len);
	tmp_req->type = META_CREATE_REQUEST;
	tmp_req->status = START;
	tmp_req->reqid = (__u32)atomic_inc_return(&global_ordinary_id);
	tmp_task->task_req = tmp_req;
	kref_init(&tmp_task->ref);

	init_dentry_task_timer(tmp_task);
	if (!add_dentry_sync_task(tmp_task, &syncer_ordinary_list)) {
		del_timer_sync(&tmp_task->task_timer);
		kref_put(&tmp_task->ref, release_task);
	}
}

void dentry_syncer_update(struct metabase *update_req)
{
	int buf_len;
	struct syncer_task *tmp_task = NULL;
	struct syncer_req *tmp_req = NULL;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_KERNEL);
	if (!tmp_task)
		return;

	buf_len = sizeof(struct metabase) + update_req->path_len +
		  update_req->name_len + 1;
	tmp_req = kmalloc(sizeof(*tmp_req) + buf_len, GFP_KERNEL);
	if (!tmp_req) {
		hmdfs_err("alloc update_task_req fail");
		kfree(tmp_task);
		return;
	}

	tmp_req->len = buf_len;
	memcpy(tmp_req->req_buf, update_req, buf_len);
	tmp_req->type = META_UPDATE_REQUEST;
	tmp_req->status = START;
	tmp_req->reqid = (__u32)atomic_inc_return(&global_ordinary_id);
	tmp_task->task_req = tmp_req;
	kref_init(&tmp_task->ref);

	init_dentry_task_timer(tmp_task);
	if (!add_dentry_sync_task(tmp_task, &syncer_ordinary_list)) {
		del_timer_sync(&tmp_task->task_timer);
		kref_put(&tmp_task->ref, release_task);
	}
}

void dentry_syncer_remove(struct remove_request *remove_req)
{
	int buf_len;
	struct syncer_task *tmp_task = NULL;
	struct syncer_req *tmp_req = NULL;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_KERNEL);
	if (!tmp_task)
		return;

	buf_len = sizeof(struct remove_request) + remove_req->name_len +
		  remove_req->path_len;

	tmp_req = kmalloc(sizeof(*tmp_req) + buf_len, GFP_KERNEL);
	if (!tmp_req) {
		kfree(tmp_task);
		return;
	}

	tmp_req->len = buf_len;
	memcpy(tmp_req->req_buf, remove_req, buf_len);
	tmp_req->type = META_REMOVE_REQUEST;
	tmp_req->status = START;
	tmp_req->reqid = (__u32)atomic_inc_return(&global_ordinary_id);
	tmp_task->task_req = tmp_req;
	kref_init(&tmp_task->ref);

	init_dentry_task_timer(tmp_task);
	if (!add_dentry_sync_task(tmp_task, &syncer_ordinary_list)) {
		del_timer_sync(&tmp_task->task_timer);
		kref_put(&tmp_task->ref, release_task);
	}
}

void dentry_syncer_rename(struct _rename_request *rename_req)
{
	int buf_len;
	struct syncer_task *tmp_task = NULL;
	struct syncer_req *tmp_req = NULL;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_KERNEL);
	if (!tmp_task)
		return;

	buf_len = sizeof(struct _rename_request) + rename_req->oldparent_len +
		  rename_req->oldname_len + rename_req->newparent_len +
		  rename_req->newname_len;

	tmp_req = kmalloc(sizeof(*tmp_req) + buf_len, GFP_KERNEL);
	if (!tmp_req) {
		kfree(tmp_task);
		return;
	}

	tmp_req->len = buf_len;
	memcpy(tmp_req->req_buf, rename_req, buf_len);
	tmp_req->type = META_RENAME_REQUEST;
	tmp_req->status = START;
	tmp_req->reqid = (__u32)atomic_inc_return(&global_ordinary_id);
	tmp_task->task_req = tmp_req;
	kref_init(&tmp_task->ref);

	init_dentry_task_timer(tmp_task);
	if (!add_dentry_sync_task(tmp_task, &syncer_ordinary_list)) {
		del_timer_sync(&tmp_task->task_timer);
		kref_put(&tmp_task->ref, release_task);
	}
}

void dentry_syncer_wakeup(void)
{
	struct syncer_task *tmp_task;
	struct syncer_req *tmp_req;

	if (time_before(jiffies, last_op_time + WAKEUP_INTERVAL))
		return;

	tmp_task = kzalloc(sizeof(*tmp_task), GFP_NOWAIT);
	if (!tmp_task)
		return;

	tmp_req = kmalloc(sizeof(*tmp_req), GFP_NOWAIT);
	if (!tmp_req) {
		kfree(tmp_task);
		return;
	}

	tmp_req->len = 0;
	tmp_req->type = WAKEUP_REQUEST;
	tmp_req->status = START;
	tmp_req->reqid = (__u32)atomic_inc_return(&global_ordinary_id);
	tmp_task->task_req = tmp_req;
	kref_init(&tmp_task->ref);

	init_dentry_task_timer(tmp_task);
	if (!add_dentry_sync_task(tmp_task, &syncer_ordinary_list)) {
		del_timer_sync(&tmp_task->task_timer);
		kref_put(&tmp_task->ref, release_task);
	}

	last_op_time = jiffies;
	hmdfs_info("waking up userspace service");
}

static int dentry_syncer_chrdev_init(void)
{
	int err;

	err = alloc_chrdev_region(&syncer_cdev.devno, 0, 1,
				  DENTRY_SYNCER_CHRDEV_NAME);
	if (err) {
		hmdfs_err(
			"Failed to alloc dentry syncer device number! errno = %d",
			err);
		return err;
	}

	syncer_cdev.cdev = cdev_alloc();
	if (!syncer_cdev.cdev) {
		err = -ENOMEM;
		hmdfs_err("Failed to alloc dentry syncer cdev! err = %d", err);
		goto unregister_chrdev;
	}
	syncer_cdev.cdev->owner = THIS_MODULE;
	syncer_cdev.cdev->ops = &dentry_syncer_fops;

	err = cdev_add(syncer_cdev.cdev, syncer_cdev.devno, 1);
	if (err) {
		hmdfs_err("Failed to add dentry syncer cdev! errno = %d", err);
		goto del_cdev;
	}

	syncer_cdev.class =
		class_create(THIS_MODULE, DENTRY_SYNCER_CHRDEV_NAME);
	if (IS_ERR(syncer_cdev.class)) {
		err = PTR_ERR(syncer_cdev.class);
		hmdfs_err("Failed to create dentry syncer class! err = %d",
			  err);
		goto del_cdev;
	}

	syncer_cdev.device =
		device_create(syncer_cdev.class, NULL, syncer_cdev.devno, NULL,
			      DENTRY_SYNCER_CHRDEV_NAME);
	if (IS_ERR(syncer_cdev.device)) {
		err = PTR_ERR(syncer_cdev.device);
		hmdfs_err("Failed to create dentry syncer device! err = %d",
			  err);
		goto destroy_class;
	}

	return err;

destroy_class:
	class_destroy(syncer_cdev.class);
del_cdev:
	cdev_del(syncer_cdev.cdev);
unregister_chrdev:
	unregister_chrdev_region(syncer_cdev.devno, 1);

	hmdfs_err("Failed to init dentry syncer chrdev!");
	return err;
}

static void dentry_syncer_chrdev_destroy(void)
{
	device_destroy(syncer_cdev.class, syncer_cdev.devno);
	class_destroy(syncer_cdev.class);
	cdev_del(syncer_cdev.cdev);
	unregister_chrdev_region(syncer_cdev.devno, 1);
}

int dentry_syncer_init(void)
{
	int err = dentry_syncer_chrdev_init();

	if (err) {
		hmdfs_err("Failed to init dentry syncer cdev! errno = %d", err);
		return err;
	}
	mutex_init(&read_mutex);
	mutex_init(&write_mutex);
	spin_lock_init(&list_lock);
	return err;
}

void dentry_syncer_exit(void)
{
	struct syncer_task *task, *tmp_task;

	spin_lock_bh(&list_lock);
	list_cleaning = true;
	atomic_set(&pending_task_start_count, 0);
	list_for_each_entry_safe(task, tmp_task, &syncer_ordinary_list, list) {
		list_del(&task->list);
		del_timer_sync(&task->task_timer);
		kref_put(&task->ref, release_task);
	}
	list_for_each_entry_safe(task, tmp_task, &syncer_prior_list, list) {
		list_del(&task->list);
		del_timer_sync(&task->task_timer);
		kref_put(&task->ref, release_task);
	}
	spin_unlock_bh(&list_lock);
	dentry_syncer_chrdev_destroy();
}
