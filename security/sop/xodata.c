

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/sched/mm.h>
#include <linux/hisi/hisi_hkip.h>
#include "securec.h"

#define SOPLOCK_MAX_LEN 256
#define SOPLOCK_IOC 0xff
/* 1 2 3.. just distinguish different command */
#define SOPLOCK_SET_XOM _IO(SOPLOCK_IOC, 1)
#define SOPLOCK_SET_TAG  _IOWR(SOPLOCK_IOC, 2, struct sop_mem)
#define SOPLOCK_MMAP_KERNEL _IOW(SOPLOCK_IOC, 3, struct sop_mem)

#define SOP_UNINITIALIZED 0
#define SOP_TAGGED 1
#define SOP_ALLOCATED 2
#define PROT_MASK (PROT_EXEC | PROT_READ | PROT_WRITE)

#ifdef HUAWEI_SOP_ENG
#define sop_debug(fmt, args...) pr_err(fmt, ##args)
#else
#define sop_debug(fmt, args...)
#endif

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
	int ret;

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
	 * 32bit share library need the compiler modification, else can not set xom
	 */
	if (strstr(node->tag, "lib64")) {
		ret = hkip_register_xo((uintptr_t)node->base, nr_pages * PAGE_SIZE);
		if (ret)
			pr_err("hkip_register_xo fails with %d\n", ret);
	}
	node->protected = 1;
	user->state = SOP_UNINITIALIZED;

	return 0;
}

/*
 * After calling sop_mmap_unprivileged, caller gets the mapped memory that
 * is execution-only, protected by HKIP in stage 2 address translation.
 * In this context, we know the mapped memory should have EXEC permission
 * only, hence we check the prot argument passed when caller calls mmap,
 * and restrict the permission to be a subset of PROT_EXEC | PROT_READ.
 * Reading the protected memory region traps to HKIP, which returns 0.
 */
static int sop_mmap_unprivileged(struct file *file, struct vm_area_struct *vma)
{
	struct sop_user *user = NULL;
	struct sop_list *node = NULL;
	int ret;

	if ((file == NULL) || (vma == NULL)) {
		pr_err("%s: input parameter is null\n", __func__);
		return -EINVAL;
	}

	user = file->private_data;
	if (user == NULL) {
		pr_err("%s: file private data is null\n", __func__);
		return -EFAULT;
	}

	sop_debug("%s: user->state: %u\n", __func__, user->state);
	if (user->state != SOP_TAGGED) {
		pr_err("%s: state is not SOP_TAGGED\n", __func__);
		return -EFAULT;
	}

	/* requested permission should be a subset of user->prot_mask */
	if (unlikely((vma->vm_flags & ~calc_vm_prot_bits(user->prot_mask, 0)) &
			calc_vm_prot_bits(PROT_MASK, 0))) {
		pr_err("%s: permission request error\n", __func__);
		return -EPERM;
	}

	node = user->current_node;
	if (node == NULL) {
		pr_err("%s: node error\n", __func__);
		return -EFAULT;
	}

	/* remap memory for caller */
	ret = remap_vmalloc_range(vma, node->base, 0);
	if (ret) {
		pr_err("%s: remap error, ret %d", __func__, ret);
		return ret;
	}

	user->state = SOP_UNINITIALIZED;
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
	size = vma->vm_end - vma->vm_start;

	if ((node->base == NULL) || (node->len == 0)) {
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
			pr_info("error: note->base is not null\n");
			vfree(node->base);
		}
		node->base = base;
		node->len = size;
	} else if ((node->base != NULL) && (node->len == size)) {
		ret = remap_vmalloc_range(vma, node->base, 0);
		if (ret != 0) {
			pr_err("%s: remap only error, ret %d\n", __func__, ret);
			return ret;
		}
	} else {
		pr_err("%s: base or length error (%pK, %d)\n",
			__func__, node->base, node->len);
		return -EINVAL;
	}
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
	list_for_each_entry(iter, &agent_tracked_list, agent_tracked) {
		sop_debug("sop protected: %u\n", iter->protected);
		if (iter->protected == 1 &&
			(memcmp(iter->tag, mem->tag, iter->tag_size) == 0 ||
			is_overlay_longer_path(mem->tag, iter->tag, iter->tag_size))) {
			user->current_node = iter;
			user->state = SOP_TAGGED;
			mem->state = SOP_TAGGED;
			mem->len = iter->len;
			return 0;
		}
	}

	/* not found or protected */
	return -EINVAL;
}

static int sop_set_tag_privileged(struct sop_user *user, struct sop_mem *mem)
{
	struct sop_list *iter = NULL;
	struct sop_list *turbo = NULL;
	int ret;

	if ((user == NULL) || (mem == NULL))
		return -EINVAL;

	sop_debug("%s: user->state: %u\n", __func__, user->state);
	if (user->state != SOP_UNINITIALIZED)
		return -EFAULT;

	/*
	 * check if tag is already protected;
	 * agent may restart for error handling;
	 */
	list_for_each_entry(iter, &agent_tracked_list, agent_tracked) {
		if (memcmp(iter->tag, mem->tag, iter->tag_size) == 0) {
			if (iter->protected == 0) {
				user->state = SOP_TAGGED;
				mem->state = SOP_TAGGED;
				user->current_node = iter;
			} else if (iter->protected == 1) {
				mem->state = SOP_UNINITIALIZED;
			}
			return 0;
		}
	}

	/* not found, make new tag */
	turbo = kzalloc(sizeof(*turbo), GFP_KERNEL);
	if (turbo == NULL)
		return -ENOMEM;
	ret = memcpy_s(turbo->tag, SOPLOCK_MAX_LEN, mem->tag, mem->tag_size);
	if (ret != EOK) {
		pr_err("memcpy_s error!");
		kfree(turbo);
		return -EFAULT;
	}
	turbo->tag_size = mem->tag_size;
	turbo->base = NULL;
	turbo->len = 0;
	turbo->protected = 0;
	list_add(&turbo->agent_tracked, &agent_tracked_list);
	user->state = SOP_TAGGED;
	mem->state = SOP_TAGGED;
	user->current_node = turbo;

	sop_debug("%s: add track list, tag: %s, tagsize: %u, base: %pK, len: %lx\n",
		__func__, turbo->tag, turbo->tag_size,
		turbo->base, (unsigned long)turbo->len);
	return 0;
}

static long sop_proc_set_tag_privileged(struct file *file,
	unsigned long arg)
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

	ret = sop_set_tag_privileged(user, &mem);
	if (ret != 0) {
		pr_err("sop_set_tag_privileged returns %d", ret);
		return ret;
	}

	if (unlikely(copy_to_user((void __user *)(uintptr_t)arg,
		&mem, sizeof(mem))))
		return -EFAULT;

	return 0;
}

static long sop_ioctl_privileged(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case SOPLOCK_SET_TAG: {
		ret = sop_proc_set_tag_privileged(file, arg);
		if (ret != 0) {
			pr_err("sop_proc_set_tag error %d", ret);
			return ret;
		}
		break;
	}
	case SOPLOCK_SET_XOM: {
		ret = sop_set_xom(file);
		if (ret != 0) {
			pr_err("sop_set_xom error %d", ret);
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

static long sop_proc_set_tag_unprivileged(struct file *file,
	unsigned long arg)
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

	ret = sop_set_tag_unprivileged(user, &mem);
	if (ret != 0) {
		pr_err("sop_set_tag_privileged returns %d", ret);
		return ret;
	}

	if (unlikely(copy_to_user((void __user *)(uintptr_t)arg,
		&mem, sizeof(mem))))
		return -EFAULT;

	return 0;
}
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
	mmput(mm);

	node = user->current_node;
	if (node == NULL) {
		pr_err("%s: node error\n", __func__);
		return -EFAULT;
	}

	down_write(&mm->mmap_sem);
	vma->vm_flags |= VM_MIXEDMAP | VM_SHARED | VM_MAYSHARE;
	up_write(&mm->mmap_sem);
	ret = remap_vmalloc_range(vma, node->base, 0);
	if (ret)
		pr_err("%s: remap error, ret %d\n", __func__, ret);
	sop_debug("%s: sop mmap in kernel success\n", __func__);
	return ret;
}
static long sop_ioctl_unprivileged(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case SOPLOCK_SET_TAG: {
		ret = sop_proc_set_tag_unprivileged(file, arg);
		if (ret != 0) {
			pr_err("sop_proc_set_tag returns %d\n", ret);
			return ret;
		}
		break;
	}
	/* added for mmap in kernel */
	case SOPLOCK_MMAP_KERNEL: {
		ret = sop_mmap_in_kernel(file, arg);
		if (ret != 0) {
			pr_err("sop_mmap_in_kernel failed, ret: %d\n", ret);
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

static const struct file_operations fops_privileged = {
	.owner          = THIS_MODULE,
	.open           = sop_open,
	.release        = sop_release,
	.unlocked_ioctl = sop_ioctl_privileged,
	.mmap           = sop_mmap_privileged,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = sop_compat_ioctl_privileged,
#endif
};

static const struct file_operations fops_unprivileged = {
	.owner          = THIS_MODULE,
	.open           = sop_open,
	.release        = sop_release,
	.unlocked_ioctl = sop_ioctl_unprivileged,
	.mmap           = sop_mmap_unprivileged,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = sop_compat_ioctl_unprivileged,
#endif
};

static struct miscdevice soplock_privileged_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sop_privileged",
	.fops  = &fops_privileged,
};

static struct miscdevice soplock_unprivileged_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sop_unprivileged",
	.fops  = &fops_unprivileged,
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
