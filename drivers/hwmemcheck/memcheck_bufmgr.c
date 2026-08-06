/*
 * memcheck_bufmgr.c
 *
 * save and read information from native or java process, send signal to them
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include "memcheck_common.h"
#include "memcheck_interface.h"
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/sort.h>
#include <linux/version.h>
#include <linux/thread_info.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif
#include <chipset_common/hwmemcheck/memcheck.h>

/* for detail information */
static int vm_type[] = { VM_IOREMAP, VM_ALLOC, VM_MAP, VM_USERMAP };

static const char *const vm_text[] = {
	"VM_IOREMAP",
	"VM_ALLOC",
	"VM_MAP",
	"VM_USERMAP",
};
static int slub_type[] = { SLUB_ALLOC, SLUB_FREE };

static const char *const slub_text[] = {
	"SLUB_ALLOC",
	"SLUB_FREE",
};

/* for stack information save and read */
#define STACK_WAIT_TIME_SEC	5
#define IDX_JAVA		0
#define IDX_NATIVE		1
#define STACK_NUM		2
static DEFINE_MUTEX(stack_mutex);
static void *stack_buf[STACK_NUM];
static u64 stack_len[STACK_NUM];
static wait_queue_head_t stack_ready;
static bool is_waiting[STACK_NUM];

/* for lmk and oom record save and read */
static DEFINE_SPINLOCK(list_lock);
static struct lmk_oom_rec *rec_list;
static u64 rec_num;

static int slub_cmp(const void *a, const void *b)
{
	const struct mm_slub_detail_info *info1 = a;
	const struct mm_slub_detail_info *info2 = b;

	if (info1->size < info2->size)
		return 1;
	else if (info1->size > info2->size)
		return -1;
	else
		return 0;
}

static int memcheck_slub_detail_read(size_t num, char *buf, size_t total)
{
	struct mm_slub_detail_info *list = NULL;
	size_t ret_num;
	size_t used = 0;
	int tmp;
	int i;

	list = vzalloc(num * sizeof(*list));
	if (!list)
		return 0;
	ret_num = get_mem_detail(SLUB_TRACK, list, num);
	if (ret_num == 0)
		goto err_list;

	/* sort the list */
	sort(list, ret_num, sizeof(*list), slub_cmp, NULL);

	/* save the top slub occupation name */
	memcheck_save_top_slub(list->name);

	/* convert to string */
	tmp = snprintf(buf, total,
		       "name	active_objs	num_objs	objsize	total_size\n");
	if (tmp < 0)
		goto err_list;
	used = min((size_t)tmp, total - 1);
	for (i = 0; i < ret_num; i++) {
		tmp =
		    snprintf(buf + used, total - used, "%s %lu %lu %lu %lu\n",
			     list[i].name, list[i].active_objs,
			     list[i].num_objs, list[i].objsize,
			     list[i].size);
		if (tmp < 0)
			goto err_list;
		used += min((size_t)tmp, total - used - 1);
		if (used >= (total - 1))
			break;
	}

err_list:
	vfree(list);

	return used;
}

static int ion_cmp(const void *a, const void *b)
{
	const struct mm_ion_detail_info *info1 = a;
	const struct mm_ion_detail_info *info2 = b;

	if (info1->size < info2->size)
		return 1;
	else if (info1->size > info2->size)
		return -1;
	else
		return 0;
}

static int memcheck_ion_detail_read(size_t num, char *buf, size_t total)
{
	struct mm_ion_detail_info *list = NULL;
	size_t ret_num;
	size_t used = 0;
	int tmp;
	int i;

	list = vzalloc(num * sizeof(*list));
	if (!list)
		return 0;
	ret_num = get_mem_detail(ION_TRACK, list, num);
	if (ret_num == 0)
		goto err_list;

	/* sort the list */
	sort(list, ret_num, sizeof(*list), ion_cmp, NULL);

	/* convert to string */
	tmp = snprintf(buf, total, "pid	size\n");
	if (tmp < 0)
		goto err_list;
	used = min((size_t)tmp, total - 1);
	for (i = 0; i < ret_num; i++) {
		tmp = snprintf(buf + used, total - used, "%d %lu\n",
			       list[i].pid, list[i].size);
		if (tmp < 0)
			goto err_list;
		used += min((size_t)tmp, total - used - 1);
		if (used >= (total - 1))
			break;
	}

err_list:
	vfree(list);

	return used;
}

static int vmalloc_cmp(const void *a, const void *b)
{
	const struct mm_vmalloc_detail_info *info1 = a;
	const struct mm_vmalloc_detail_info *info2 = b;

	if (info1->size < info2->size)
		return 1;
	else if (info1->size > info2->size)
		return -1;
	else
		return 0;
}

static const char *type_to_str(int type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(vm_type); i++) {
		if (vm_type[i] == type)
			return vm_text[i];
	}
	return "NA";
}

static int memcheck_vmalloc_detail_read(size_t num, char *buf, size_t total)
{
	struct mm_vmalloc_detail_info *list = NULL;
	size_t ret_num;
	size_t used = 0;
	int tmp;
	int i;

	list = vzalloc(num * sizeof(*list));
	if (!list)
		return 0;
	ret_num = get_mem_detail(VMALLOC_TRACK, list, num);
	if (ret_num == 0)
		goto err_list;

	/* sort the list */
	sort(list, ret_num, sizeof(*list), vmalloc_cmp, NULL);

	/* convert to string */
	tmp = snprintf(buf, total, "type	size\n");
	if (tmp < 0)
		goto err_list;
	used = min((size_t) tmp, total - 1);
	for (i = 0; i < ret_num; i++) {
		tmp = snprintf(buf + used, total - used, "%s %lu\n",
			       type_to_str(list[i].type), list[i].size);
		if (tmp < 0)
			goto err_list;
		used += min((size_t) tmp, total - used - 1);
		if (used >= (total - 1))
			break;
	}

err_list:
	vfree(list);

	return used;
}

int memcheck_detail_read(void *buf, struct detail_info *info)
{
	char *tmp = NULL;
	size_t num;
	size_t len;
	int ret = -EFAULT;

	if (info->type == MTYPE_KERN_SLUB)
		num = info->size / sizeof(struct mm_slub_detail_info);
	else if ((info->type == MTYPE_KERN_ION) ||
		 (info->type == MTYPE_USER_ION))
		num = info->size / sizeof(struct mm_ion_detail_info);
	else if (info->type == MTYPE_KERN_VMALLOC)
		num = info->size / sizeof(struct mm_vmalloc_detail_info);
	else
		return -EINVAL;
	if (!num) {
		info->size = 0;
		if (copy_to_user(buf, info, sizeof(*info))) {
			memcheck_err("copy_to_user failed\n");
			return -EFAULT;
		}
		memcheck_info("buf is too small to contain detail data\n");
		return 0;
	}

	tmp = vzalloc(info->size + 1);
	if (!tmp)
		return -ENOMEM;
	if (info->type == MTYPE_KERN_SLUB)
		len = memcheck_slub_detail_read(num, tmp, info->size);
	else if ((info->type == MTYPE_KERN_ION) ||
		 (info->type == MTYPE_USER_ION))
		len = memcheck_ion_detail_read(num, tmp, info->size);
	else if (info->type == MTYPE_KERN_VMALLOC)
		len = memcheck_vmalloc_detail_read(num, tmp, info->size);
	else
		goto err_buf;
	info->size = len;
	if (copy_to_user(buf, info, sizeof(*info))) {
		memcheck_err("copy_to_user failed\n");
		goto err_buf;
	}
	if (len && copy_to_user(buf + sizeof(*info), tmp, len)) {
		memcheck_err("copy_to_user failed\n");
		goto err_buf;
	}
	ret = 0;

err_buf:
	vfree(tmp);
	return ret;
}

static int stack_cmp(const void *a, const void *b)
{
	const struct mm_stack_info *info1 = a;
	const struct mm_stack_info *info2 = b;

	if (atomic_read(&info1->ref) < atomic_read(&info2->ref))
		return 1;
	else if (atomic_read(&info1->ref) > atomic_read(&info2->ref))
		return -1;
	else
		return 0;
}

static size_t memcheck_stack_to_str(struct mm_stack_info *list, size_t num,
				    char *buf, size_t total, size_t used)
{
	int i;
	int tmp;

	sort(list, num, sizeof(*list), stack_cmp, NULL);

	tmp = snprintf(buf + used, total - used, "PC	hits\n");
	if (tmp < 0)
		goto buf_done;
	used += min((size_t) tmp, total - used - 1);
	for (i = 0; i < num; i++) {
		tmp = snprintf(buf + used, total - used, "%pS %zu\n",
			       list[i].caller, list[i].ref);
		if (tmp < 0)
			goto buf_done;
		used += min((size_t)tmp, total - used - 1);
		if (used >= (total - 1))
			break;
	}
buf_done:
	return used;
}

static size_t memcheck_append_str(const char *str, char *buf, size_t total,
				  size_t used)
{
	int tmp;

	tmp = snprintf(buf + used, total - used, "%s\n", str);
	if (tmp < 0)
		return used;
	used += min((size_t) tmp, total - used - 1);
	return used;
}

static size_t memcheck_get_slub_stack(struct mm_stack_info *list, size_t num,
				      char *buf, size_t total)
{
	int i;
	int ret;
	size_t used = 0;
	size_t ret_num;

	for (i = 0; i < ARRAY_SIZE(slub_type); i++) {
		ret = page_trace_open(SLUB_TRACK, slub_type[i]);
		if (ret) {
			memcheck_info("open SLUB trace failed");
			return used;
		}
		ret_num = page_trace_read(SLUB_TRACK, list, num,
					       slub_type[i]);
		if (ret_num == 0) {
			memcheck_info("empty %s stack record", slub_text[i]);
			page_trace_close(SLUB_TRACK, slub_type[i]);
			continue;
		}
		ret = page_trace_close(SLUB_TRACK, slub_type[i]);
		if (ret) {
			memcheck_info("close SLUB trace failed");
			return used;
		}
		used = memcheck_append_str(slub_text[i], buf, total, used);
		if (used >= (total - 1))
			return used;
		used = memcheck_stack_to_str(list, ret_num, buf, total, used);
		if ((ret_num >= num) || (used >= (total - 1)))
			return used;
	}

	return used;
}

static size_t memcheck_get_vmalloc_stack(struct mm_stack_info *list,
					 size_t num, char *buf, size_t total)
{
	int i;
	int ret;
	size_t used = 0;
	size_t ret_num;

	for (i = 0; i < ARRAY_SIZE(vm_type); i++) {
		ret = page_trace_open(VMALLOC_TRACK, vm_type[i]);
		if (ret) {
			memcheck_info("open VMALLOC trace failed");
			return used;
		}
		ret_num = page_trace_read(VMALLOC_TRACK, list, num,
					       vm_type[i]);
		if (ret_num == 0) {
			memcheck_info("empty %s stack record", vm_text[i]);
			page_trace_close(VMALLOC_TRACK, vm_type[i]);
			continue;
		}
		ret = page_trace_close(VMALLOC_TRACK, vm_type[i]);
		if (ret) {
			memcheck_info("close VMALLOC trace failed");
			return used;
		}
		used = memcheck_append_str(vm_text[i], buf, total, used);
		if (used >= (total - 1))
			return used;
		used = memcheck_stack_to_str(list, ret_num, buf, total, used);
		if ((ret_num >= num) || (used >= (total - 1)))
			return used;
	}

	return used;
}

static size_t memcheck_get_buddy_stack(struct mm_stack_info *list, size_t num,
				       char *buf, size_t total)
{
	int ret;
	size_t used = 0;
	size_t ret_num;

	ret = page_trace_open(BUDDY_TRACK, BUDDY_TRACK);
	if (ret) {
		memcheck_info("open BUDDY trace failed");
		return used;
	}
	ret_num = page_trace_read(BUDDY_TRACK, list, num, BUDDY_TRACK);
	if (ret_num == 0) {
		memcheck_info("empty buddy stack record");
		page_trace_close(BUDDY_TRACK, BUDDY_TRACK);
		return used;
	}
	ret = page_trace_close(BUDDY_TRACK, BUDDY_TRACK);
	if (ret) {
		memcheck_info("close BUDDY trace failed");
		return used;
	}
	used = memcheck_stack_to_str(list, ret_num, buf, total, used);
	return used;
}

static size_t memcheck_get_lslub_stack(struct mm_stack_info *list, size_t num,
				       char *buf, size_t total)
{
	int ret;
	size_t used = 0;
	size_t ret_num;

	ret = page_trace_open(LSLUB_TRACK, LSLUB_TRACK);
	if (ret) {
		memcheck_info("open LSLUB trace failed");
		return used;
	}
	ret_num = page_trace_read(LSLUB_TRACK, list, num, LSLUB_TRACK);
	if (ret_num == 0) {
		memcheck_info("empty buddy stack record");
		page_trace_close(LSLUB_TRACK, LSLUB_TRACK);
		return used;
	}
	ret = page_trace_close(LSLUB_TRACK, LSLUB_TRACK);
	if (ret) {
		memcheck_info("close LSLUB trace failed");
		return used;
	}
	used = memcheck_stack_to_str(list, ret_num, buf, total, used);
	return used;
}

static int memcheck_get_stack_items(size_t num, char *buf, size_t total,
				    struct stack_info *info)
{
	struct mm_stack_info *list = NULL;
	size_t used = 0;

	list = vzalloc(num * sizeof(*list));
	if (!list)
		return 0;
	if (info->type == MTYPE_KERN_SLUB)
		used = memcheck_get_slub_stack(list, num, buf, total);
	else if (info->type == MTYPE_KERN_VMALLOC)
		used = memcheck_get_vmalloc_stack(list, num, buf, total);
	else if (info->type == MTYPE_KERN_BUDDY)
		used = memcheck_get_buddy_stack(list, num, buf, total);
	else if (info->type == MTYPE_KERN_LSLUB)
		used = memcheck_get_lslub_stack(list, num, buf, total);
	vfree(list);

	return used;
}

static int memcheck_kernel_stack_read(void *buf, struct stack_info *info)
{
	char *tmp = NULL;
	size_t num;
	size_t len;
	int ret;

	num = info->size / sizeof(struct mm_stack_info);
	if (!num) {
		info->size = 0;
		if (copy_to_user(buf, info, sizeof(*info))) {
			memcheck_err("copy_to_user failed\n");
			return -EFAULT;
		}
		memcheck_info("buf is too small to contain stack data\n");
		return 0;
	}

	tmp = vzalloc(info->size + 1);
	if (!tmp)
		return -ENOMEM;
	len = memcheck_get_stack_items(num, tmp, info->size, info);

	info->size = len;
	if (copy_to_user(buf, info, sizeof(*info))) {
		memcheck_err("copy_to_user failed\n");
		ret = -EFAULT;
		goto err_buf;
	}
	if (len && copy_to_user(buf + sizeof(*info), tmp, len)) {
		memcheck_err("copy_to_user failed\n");
		ret = -EFAULT;
		goto err_buf;
	}
	ret = 0;

err_buf:
	vfree(tmp);
	return ret;
}

int memcheck_stack_read(void *buf, struct stack_info *info)
{
	int ret = -EFAULT;
	size_t len;
	size_t java_len = 0;
	size_t total_len = 0;
	int idx;

	if (info->type & MTYPE_KERNEL)
		return memcheck_kernel_stack_read(buf, info);

	mutex_lock(&stack_mutex);
	for (idx = 0; idx < STACK_NUM; idx++) {
		if (!stack_buf[idx])
			continue;
		len = min(stack_len[idx], info->size - total_len);
		if (copy_to_user
		    (buf + sizeof(*info) + java_len, stack_buf[idx], len)) {
			memcheck_err("copy_to_user failed\n");
			goto unlock;
		}
		if (info->type & MTYPE_USER_PSS_JAVA)
			java_len = len;
		memcheck_info("read idx=%d,len=%llu\n", idx, len);
		total_len += len;
		if (total_len >= info->size)
			break;
	}
	if (total_len != info->size) {
		info->size = total_len;
		if (copy_to_user(buf, info, sizeof(*info))) {
			memcheck_err("copy_to_user failed\n");
			goto unlock;
		}
	}

	ret = 0;

unlock:
	mutex_unlock(&stack_mutex);

	return ret;
}

int memcheck_stack_clear(void)
{
	int idx;

	mutex_lock(&stack_mutex);
	for (idx = 0; idx < STACK_NUM; idx++) {
		if (stack_buf[idx]) {
			vfree(stack_buf[idx]);
			stack_buf[idx] = NULL;
			stack_len[idx] = 0;
		}
	}
	mutex_unlock(&stack_mutex);

	return 0;
}

int memcheck_stack_write(const void *buf, const struct stack_info *info)
{
	char *tmp = NULL;
	int idx;

	tmp = vzalloc(info->size + 1);
	if (!tmp)
		return -EFAULT;
	if (copy_from_user(tmp, buf + sizeof(*info), info->size)) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	tmp[info->size] = 0;

	idx = (info->type & MTYPE_USER_PSS_JAVA) ? IDX_JAVA : IDX_NATIVE;
	mutex_lock(&stack_mutex);
	if (stack_buf[idx])
		vfree(stack_buf[idx]);
	stack_buf[idx] = tmp;
	stack_len[idx] = info->size;
	mutex_unlock(&stack_mutex);

	if (is_waiting[idx])
		wake_up_interruptible(&stack_ready);

	return 0;
}

static int memcheck_check_wait_result(int left, bool is_java, bool is_native)
{
	if (!left) {
		if (is_java && (!is_native))
			memcheck_err("wait for java stack timeout\n");
		else if ((!is_java) && is_native)
			memcheck_err("wait for native stack timeout\n");
		else if (is_java && is_native)
			memcheck_err("wait for java and native timeout\n");
		return -ETIMEDOUT;
	} else if (left < 0) {
		if (is_java && (!is_native))
			memcheck_err("wait for java stack return %d\n", left);
		else if ((!is_java) && is_native)
			memcheck_err("wait for native stack return %d\n", left);
		else if (is_java && is_native)
			memcheck_err("wait for java and native return %d\n",
				     left);
		return -EFAULT;
	}

	return 0;
}

int memcheck_wait_stack_ready(u16 type)
{
	int left;
	bool is_java = (type & MTYPE_USER_PSS_JAVA) ? true : false;
	bool is_native = (type & MTYPE_USER_PSS_NATIVE) ? true : false;
	int index;
	int ret = 0;

	index = is_java ? IDX_JAVA : IDX_NATIVE;

	mutex_lock(&stack_mutex);
	is_waiting[index] = true;
	init_waitqueue_head(&stack_ready);
	mutex_unlock(&stack_mutex);
	left =
	    wait_event_interruptible_timeout(stack_ready,
					     stack_buf[index],
					     STACK_WAIT_TIME_SEC * HZ);
	mutex_lock(&stack_mutex);
	if (stack_buf[index])
		memcheck_info("get %s stack successfully\n",
			      is_java ? "java" : "native");
	else
		ret = memcheck_check_wait_result(left, is_java, is_native);
	is_waiting[index] = false;
	mutex_unlock(&stack_mutex);

	return ret;
}

int memcheck_lmk_oom_read(void *buf, struct lmk_oom_read *read)
{
	size_t num;
	int ret = -EFAULT;
	struct lmk_oom_rec list_temp[MEMCHECK_OOM_LMK_MAXNUM];
	unsigned long flags;

	spin_lock_irqsave(&list_lock, flags);

	/* num = 0 is valid return */
	memset(&list_temp[0], 0, sizeof(list_temp));
	num = min(rec_num, read->num);
	read->num = num;
	if (!num || !rec_list)
		goto copy_head;
	memcpy(&list_temp[0], &rec_list[0], num * sizeof(read->data[0]));
copy_head:
	spin_unlock_irqrestore(&list_lock, flags);

	if (copy_to_user(buf, read, sizeof(*read))) {
		memcheck_err("copy oom record num failed\n");
		return ret;
	}
	if (num && copy_to_user(buf + sizeof(*read), list_temp,
				num * sizeof(read->data[0]))) {
		memcheck_err("copy oom record failed\n");
		return ret;
	}

	ret = 0;
	return ret;
}

int memcheck_lmk_oom_write(const struct lmk_oom_write *write)
{
	size_t len;
	unsigned long flags;

	spin_lock_irqsave(&list_lock, flags);

	if (!rec_list) {
		rec_list = kcalloc(MEMCHECK_OOM_LMK_MAXNUM,
				   sizeof(write->data), GFP_ATOMIC);
		if (!rec_list) {
			spin_unlock_irqrestore(&list_lock, flags);
			return -ENOMEM;
		}
	}
	if (rec_num == MEMCHECK_OOM_LMK_MAXNUM) {
		len = (MEMCHECK_OOM_LMK_MAXNUM - 1) * sizeof(rec_list[0]);
		memmove(&rec_list[0], &rec_list[1], len);
		len = sizeof(write->data);
		memcpy(&rec_list[MEMCHECK_OOM_LMK_MAXNUM - 1],
		       &write->data, len);
	} else {
		len = sizeof(write->data);
		memcpy(&rec_list[rec_num], &write->data, len);
		rec_num++;
	}

	spin_unlock_irqrestore(&list_lock, flags);

	return 0;
}

int memcheck_get_task_type(void *buf, struct task_type_read *read)
{
	size_t num = 0;
	int ret = -EFAULT;
	struct task_type_rec *list = NULL;
	struct task_struct *p = NULL;

	list = vzalloc(read->num * sizeof(*list));
	if (!list)
		return 0;

	rcu_read_lock();
	for_each_process(p) {
		if (p->flags & PF_KTHREAD)
			continue;
		if (p->pid != p->tgid)
			continue;

		list[num].pid = p->pid;
		list[num].is_32bit =
		    test_ti_thread_flag(task_thread_info(p), TIF_32BIT);
		num++;
		if (num >= read->num)
			break;
	}
	rcu_read_unlock();

	/* num = 0 is valid return */
	read->num = num;
	if (copy_to_user(buf, read, sizeof(*read))) {
		memcheck_err("copy task type record num failed\n");
		goto err_buf;
	}
	if (!num) {
		ret = 0;
		goto err_buf;
	}
	if (copy_to_user(buf + sizeof(*read), list,
			 num * sizeof(read->data[0]))) {
		memcheck_err("copy task type record failed\n");
		goto err_buf;
	}
	ret = 0;

err_buf:
	vfree(list);

	return ret;
}

int memcheck_report_lmk_oom(pid_t pid, pid_t tgid, const char *name,
			    enum kill_type ktype, short adj, size_t pss)
{
	struct lmk_oom_write wr;
	size_t len;

	memset(&wr, 0, sizeof(wr));
	wr.magic = MEMCHECK_MAGIC;
	wr.data.pid = pid;
	wr.data.tgid = tgid;
	if (name) {
		len = strnlen(name, sizeof(wr.data.name) - 1);
		if (len)
			strncpy(wr.data.name, name, len);
	}
	wr.data.ktype = ktype;
	wr.data.adj = adj;
	wr.data.pss = pss;
	wr.data.timestamp = jiffies_to_msecs(jiffies);

	return memcheck_lmk_oom_write(&wr);
}
EXPORT_SYMBOL(memcheck_report_lmk_oom);
