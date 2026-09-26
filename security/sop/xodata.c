/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: sop kernel module
 * Create: 2019-08-03
 */

#include <linux/errno.h>
#include <linux/file.h>
#if defined(CONFIG_MTK_PLATFORM)
#include <uni/hkip/hkip.h>
#elif defined(CONFIG_ARCH_HISI)
#include <linux/hisi/hkip.h>
#endif
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/module.h>
#include <linux/sched/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "securec.h"

#define SOPLOCK_MAX_LEN 256
#define SOPLOCK_IOC 0xff
/* 1 2 3.. just distinguish different command */
#define SOPLOCK_SET_XOM _IO(SOPLOCK_IOC, 1)
#define SOPLOCK_SET_TAG  _IOWR(SOPLOCK_IOC, 2, struct sop_mem)
#define SOPLOCK_MMAP_KERNEL _IOW(SOPLOCK_IOC, 3, struct sop_mem)

static DEFINE_RWLOCK(sop_list_lock);
static DEFINE_MUTEX(sop_mmap_lock);

#define SOP_UNINITIALIZED 0
#define SOP_TAGGED 1
#define SOP_ALLOCATED 2

#ifdef CONFIG_HUAWEI_SOP_ENG
#define sop_debug(fmt, args...) pr_err(fmt, ##args)
#else
#define sop_debug(fmt, args...)
#endif

/* Type of the tag privilege */
enum tag_privilege_type {
	PRIVILEGED,
	UNPRIVILEGED,
};

struct sop_mem {
	uint64_t len;
	uint8_t tag[SOPLOCK_MAX_LEN];
	uint32_t tag_size;
	uint32_t state;
	uintptr_t map_base;
	uint64_t map_len;
};

struct sop_list {
	uint64_t len;
	uint64_t *base;
	uint8_t tag[SOPLOCK_MAX_LEN];
	uint32_t tag_size;
	int protected;
	struct list_head agent_tracked;
};

struct sop_user {
	struct sop_list *current_node;
	uint8_t state;
	uint64_t prot_mask;
};

LIST_HEAD(agent_tracked_list);

static int sop_open(struct inode *inode, struct file *file)
{
	struct sop_user *user = NULL;

	(void)inode;
	user = kzalloc(sizeof(*user), GFP_KERNEL);
	if (unlikely(!user))
		return -ENOMEM;

	user->state = SOP_UNINITIALIZED;
	user->prot_mask = PROT_READ | PROT_EXEC;
	file->private_data = user;
	return 0;
}

static int sop_release(struct inode *inode, struct file *file)
{
	(void)inode;
	kfree(file->private_data);
	return 0;
}

static inline uint64_t sop_get_nr_code_pages(uint64_t length)
{
	uint64_t n = length / PAGE_SIZE;

	return (length % PAGE_SIZE) ? (n + 1) : n;
}

/*
 * for each so file, sop_agent shall call in order SOPLOCK_SET_TAG, mmap,
 * SOPLOCK_SET_XOM to protect it. sop_lock introduces a state machine to
 * coordinate between these calls, and bails out if the state is wrong
 */
static int sop_set_xom(struct file *file)
{
	struct sop_list *node = NULL;
	struct sop_user *user = NULL;
	struct page *p = NULL;
	void *kaddr = NULL;
	int nr_pages;
	int count;

	user = file->private_data;
	if (user == NULL)
		return -EINVAL;

	if (user->state != SOP_ALLOCATED)
		return -EFAULT;

	node = user->current_node;
	if (node == NULL) {
		pr_err("%s: node error\n", __func__);
		return -EFAULT;
	}

	nr_pages = sop_get_nr_code_pages(node->len);

	/* increase page ref count explictly here from migration */
	/* SetPageReserved from being swap out */
	kaddr = node->base;
	count = nr_pages;
	do {
		p = vmalloc_to_page(kaddr);
		if (p == NULL) {
			pr_err("%s: vmalloc_to_page failed\n", __func__);
			return -EFAULT;
		}
		get_page(p);
		SetPageReserved(p);
		sop_debug("%s: page No %u : page: %pK, refcount: %d\n",
			__func__, count, p, page_count(p));
		kaddr += PAGE_SIZE;
	} while (--count > 0);

	/*
	 * set xom through hkip
	 * if error, just print the error message and go on
	 * 32bit or MTK platform share library need the compiler modification,
	 * else can not set xom
	 */
#ifndef CONFIG_ARCH_QCOM
	if (strstr(node->tag, "lib64")) {
		int ret = hkip_register_xo((uintptr_t)node->base, nr_pages * PAGE_SIZE);
		if (ret)
			pr_err("hkip_register_xo fails with %d\n", ret);
	}
#endif

	node->protected = 1;
	user->state = SOP_UNINITIALIZED;

	return 0;
}

static int remap_range_mem(struct vm_area_struct *vma, struct sop_list *node,
	uint64_t size)
{
	int ret;

	uint64_t *base = vmalloc_user(size);
	if (base == NULL) {
		pr_err("%s: malloc user memory failed\n", __func__);
		return -EFAULT;
	}

	sop_debug("%s: base = %pK\n", __func__, base);
	// remap the memory to user space
	ret = remap_vmalloc_range(vma, base, 0);
	if (ret) {
		pr_err("%s: remap error, ret %d\n", __func__, ret);
		vfree(base);
		return ret;
	}

	if (node->base != NULL) {
		pr_info("%s: note->base is not null\n", __func__);
		vfree(node->base);
	}

	node->base = base;
	node->len = size;

	return 0;
}

/*
 * After calling sop_mmap_privileged, sop_agent gets the allocated memory.
 * Eventually plain text of decrypted so code is copied to the memory, after
 * being decrypted by the agent. In this context, the mapped memory should
 * at least have WRITE permission, possibly other permissions as well. The
 * required permissions depend on the implementation of sop_agent, and we
 * can not make any assumptions here, hence we don't restrict the mapping
 * properties in the driver.
 */
static int sop_mmap_privileged(struct file *file, struct vm_area_struct *vma)
{
	struct sop_user *user = NULL;
	struct sop_list *node = NULL;
	int ret;
	uint64_t size;

	if ((file == NULL) || (file->private_data == NULL) || (vma == NULL)) {
		pr_err("%s: input parameter is null\n", __func__);
		return -EINVAL;
	}

	user = file->private_data;
	sop_debug("%s: user->state: %u\n", __func__, user->state);
	if (user->state != SOP_TAGGED)
		return -EFAULT;

	node = user->current_node;
	if (node == NULL)  {
		pr_err("%s: node error\n", __func__);
		return -EFAULT;
	}

	/* get vma size that kernel prepared */
	mutex_lock(&sop_mmap_lock);
	size = vma->vm_end - vma->vm_start;

	if ((node->base != NULL) && (node->len != 0)) {
		pr_err("%s: base or length error (%pK, %d)\n",
			__func__, node->base, node->len);
		mutex_unlock(&sop_mmap_lock);
		return -EINVAL;
	}

	ret = remap_range_mem(vma, node, size);
	if (ret != 0) {
		pr_err("%s: remap range mem failed, ret %d\n", __func__, ret);
		mutex_unlock(&sop_mmap_lock);
		return ret;
	}
	mutex_unlock(&sop_mmap_lock);

	user->state = SOP_ALLOCATED;
	return 0;
}

/* if current input so is in overlay fs */
static bool is_overlay_longer_path(uint8_t *cur, uint8_t *iter, uint32_t iter_len)
{
	char *str = NULL;

	if (strlen(cur) <= iter_len - 1)
		return false;
	str = strstr(cur, iter);
	if (str && (strlen(str) == (iter_len - 1)))
		return true;
	return false;
}

static int sop_set_tag_unprivileged(struct sop_user *user, struct sop_mem *mem)
{
	struct sop_list *iter = NULL;

	if ((user == NULL) || (mem == NULL))
		return -EFAULT;

	sop_debug("%s: user->state: %u\n", __func__, user->state);
	if (user->state != SOP_UNINITIALIZED)
		return -EFAULT;

	/* find tag in agent_tracked_list */
	read_lock(&sop_list_lock);
	list_for_each_entry(iter, &agent_tracked_list, agent_tracked) {
		sop_debug("%s: sop protected: %u\n", __func__, iter->protected);
		if (iter->protected == 1 &&
			(memcmp(iter->tag, mem->tag, iter->tag_size) == 0 ||
			is_overlay_longer_path(mem->tag, iter->tag, iter->tag_size))) {
			user->current_node = iter;
			user->state = SOP_TAGGED;
			mem->state = SOP_TAGGED;
			mem->len = iter->len;
			read_unlock(&sop_list_lock);
			return 0;
		}
	}
	read_unlock(&sop_list_lock);

	/* not found or protected */
	return -EINVAL;
}

static struct sop_list *sop_list_add_node(struct sop_mem *mem)
{
	struct sop_list *turbo = NULL;
	int ret;

	turbo = kzalloc(sizeof(*turbo), GFP_KERNEL);
	if (turbo == NULL)
		return NULL;

	ret = memcpy_s(turbo->tag, SOPLOCK_MAX_LEN, mem->tag, mem->tag_size);
	if (ret != EOK) {
		pr_err("%s: memcpy_s error!", __func__);
		kfree(turbo);
		return NULL;
	}

	turbo->tag_size = mem->tag_size;
	turbo->base = NULL;
	turbo->len = 0;
	turbo->protected = 0;
	write_lock(&sop_list_lock);
	list_add(&turbo->agent_tracked, &agent_tracked_list);
	write_unlock(&sop_list_lock);

	return turbo;
}

static int sop_set_tag_privileged(struct sop_user *user, struct sop_mem *mem)
{
	struct sop_list *iter = NULL;
	struct sop_list *turbo = NULL;

	if ((user == NULL) || (mem == NULL))
		return -EINVAL;

	sop_debug("%s: user->state: %u\n", __func__, user->state);
	if (user->state != SOP_UNINITIALIZED)
		return -EFAULT;

	/*
	 * check if tag is already protected;
	 * agent may restart for error handling;
	 */
	read_lock(&sop_list_lock);
	list_for_each_entry(iter, &agent_tracked_list, agent_tracked) {
		if (memcmp(iter->tag, mem->tag, iter->tag_size) == 0) {
			if (iter->protected == 0) {
				user->state = SOP_TAGGED;
				mem->state = SOP_TAGGED;
				user->current_node = iter;
			} else if (iter->protected == 1) {
				mem->state = SOP_UNINITIALIZED;
			}
			read_unlock(&sop_list_lock);
			return 0;
		}
	}
	read_unlock(&sop_list_lock);

	/* not found tag in current list, make new decrypt node */
	turbo = sop_list_add_node(mem);
	if (turbo == NULL) {
		pr_err("sop list add node error");
		return -EFAULT;
	}

	user->state = SOP_TAGGED;
	mem->state = SOP_TAGGED;
	user->current_node = turbo;

	sop_debug("%s: sop add track list success, tag: %s, tagsize: %u, base: %pK,"
		"len: %lx\n", __func__, turbo->tag, turbo->tag_size,
		turbo->base, (unsigned long)turbo->len);
	return 0;
}

static long sop_proc_set_tag(struct file *file,
	unsigned long arg, int privilege_status)
{
	struct sop_user *user = NULL;
	struct sop_mem mem = {0};
	int ret;

	user = file->private_data;
	if (user == NULL)
		return -EFAULT;

	if (unlikely(copy_from_user(&mem,
		(void __user *)(uintptr_t)arg, sizeof(mem))))
		return -EFAULT;

	sop_debug("%s: tag: %s, tagsize: %u\n", __func__, mem.tag, mem.tag_size);

	if (mem.tag_size > SOPLOCK_MAX_LEN) {
		pr_err("%s: invalid tag size, %u\n", __func__, mem.tag_size);
		return -EINVAL;
	}

	switch (privilege_status) {
	case PRIVILEGED:
		ret = sop_set_tag_privileged(user, &mem);
		if (ret != 0) {
			pr_err("%s: sop_set_tag_privileged returns %d", __func__, ret);
			return ret;
		}
		break;
	case UNPRIVILEGED:
		ret = sop_set_tag_unprivileged(user, &mem);
		if (ret != 0) {
			pr_err("%s: sop_set_tag_unprivileged returns %d", __func__, ret);
			return ret;
		}
		break;
	default:
		pr_err("%s: invalid operation status", __func__);
		return -EFAULT;
	}

	if (unlikely(copy_to_user((void __user *)(uintptr_t)arg, &mem, sizeof(mem))))
		return -EFAULT;

	return 0;
}

static long sop_ioctl_privileged(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case SOPLOCK_SET_TAG: {
		ret = sop_proc_set_tag(file, arg, PRIVILEGED);
		if (ret != 0) {
			pr_err("%s: sop_proc_set_tag error %d", __func__, ret);
			return ret;
		}
		break;
	}
	case SOPLOCK_SET_XOM: {
		ret = sop_set_xom(file);
		if (ret != 0) {
			pr_err("%s: sop_set_xom error %d", __func__, ret);
			return ret;
		}
		break;
	}
	default:
		pr_err("%s cmd error\n", __func__);
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_COMPAT
static long sop_compat_ioctl_privileged(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	return sop_ioctl_privileged(file, cmd, (uintptr_t)compat_ptr(arg));
}
#endif

static long sop_mmap_in_kernel(struct file *file, unsigned long arg)
{
	struct sop_user *user = NULL;
	struct sop_list *node = NULL;
	struct vm_area_struct *vma = NULL;
	struct mm_struct *mm = NULL;
	struct sop_mem mem = {0};
	int ret;

	user = file->private_data;
	if (user == NULL)
		return -EFAULT;

	if (unlikely(copy_from_user(&mem,
		(void __user *)(uintptr_t)arg, sizeof(mem))))
		return -EFAULT;

	node = user->current_node;
	if ((node == NULL) || (node->len != mem.map_len)) {
		pr_err("%s: node is null or map len error\n", __func__);
		return -EFAULT;
	}

	mm = get_task_mm(current);
	if (!mm) {
		pr_err("%s: could not get mm\n", __func__);
		return -EFAULT;
	}
	down_read(&mm->mmap_sem);
	vma = find_exact_vma(mm, mem.map_base, (mem.map_base + mem.map_len));
	if (!vma) {
		pr_err("%s: could not found the vma\n", __func__);
		up_read(&mm->mmap_sem);
		mmput(mm);
		return -EFAULT;
	}
	sop_debug("%s: found the vma\n", __func__);
	up_read(&mm->mmap_sem);

	down_write(&mm->mmap_sem);
	vma->vm_flags |= VM_MIXEDMAP | VM_SHARED | VM_MAYSHARE;
	up_write(&mm->mmap_sem);
	ret = remap_vmalloc_range(vma, node->base, 0);
	if (ret)
		pr_err("%s: remap error, ret %d\n", __func__, ret);
	sop_debug("%s: sop mmap in kernel success\n", __func__);

	mmput(mm);
	return ret;
}

static long sop_ioctl_unprivileged(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case SOPLOCK_SET_TAG: {
		ret = sop_proc_set_tag(file, arg, UNPRIVILEGED);
		if (ret != 0) {
			pr_err("%s: sop_proc_set_tag returns %d\n", __func__, ret);
			return ret;
		}
		break;
	}
	/* added for mmap in kernel */
	case SOPLOCK_MMAP_KERNEL: {
		ret = sop_mmap_in_kernel(file, arg);
		if (ret != 0) {
			pr_err("%s: sop_mmap_in_kernel failed, ret: %d\n", __func__, ret);
			return ret;
		}
		break;
	}
	default:
		pr_err("%s: cmd error: %d\n", __func__, cmd);
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_COMPAT
static long sop_compat_ioctl_unprivileged(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	return sop_ioctl_unprivileged(file, cmd, (uintptr_t)compat_ptr(arg));
}
#endif

static const struct file_operations g_fops_privileged = {
	.owner          = THIS_MODULE,
	.open           = sop_open,
	.release        = sop_release,
	.unlocked_ioctl = sop_ioctl_privileged,
	.mmap           = sop_mmap_privileged,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = sop_compat_ioctl_privileged,
#endif
};

static const struct file_operations g_fops_unprivileged = {
	.owner          = THIS_MODULE,
	.open           = sop_open,
	.release        = sop_release,
	.unlocked_ioctl = sop_ioctl_unprivileged,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = sop_compat_ioctl_unprivileged,
#endif
};

static struct miscdevice soplock_privileged_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sop_privileged",
	.fops  = &g_fops_privileged,
};

static struct miscdevice soplock_unprivileged_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sop_unprivileged",
	.fops  = &g_fops_unprivileged,
};

static int __init soplock_init(void)
{
	int ret;

	ret = misc_register(&soplock_privileged_misc);
	if (unlikely(ret)) {
		pr_err("register soplock_privileged_misc device fail!\n");
		return ret;
	}

	ret = misc_register(&soplock_unprivileged_misc);
	if (unlikely(ret)) {
		pr_err("register soplock_unprivileged_misc device fail!\n");
		misc_deregister(&soplock_privileged_misc);
		return ret;
	}

	pr_info("sop initialized\n");

	return ret;
}

device_initcall(soplock_init);
