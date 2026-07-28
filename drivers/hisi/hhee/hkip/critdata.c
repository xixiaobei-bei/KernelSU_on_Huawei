/*
 * Copyright (c) 2017 Huawei Technologies.
 */

#include <asm/uaccess.h>
#include <linux/cred.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hisi/hisi_hkip.h>

#define DEFINE_HKIP_BITS(name, count) \
	u8 hkip_##name[ALIGN(DIV_ROUND_UP(count, 8), PAGE_SIZE)] \
		__aligned(PAGE_SIZE)

EXPORT_SYMBOL(hkip_hvc4);

static int __init hkip_register_bits(u8 *base, size_t size)
{
	unsigned long addr = (unsigned long)base;

	BUG_ON((addr | size) & ~PAGE_MASK);

	if (hkip_hvc3(HKIP_HVC_ROWM_REGISTER, addr, size))
		return -ENOTSUPP;
	return 0;
}

#define DEFINE_HKIP_TASK_BITS(name)	DEFINE_HKIP_BITS(name, PID_MAX_DEFAULT)

DEFINE_HKIP_TASK_BITS(addr_limit_bits);
EXPORT_SYMBOL(hkip_addr_limit_bits);

static DEFINE_HKIP_TASK_BITS(uid_root_bits);
static DEFINE_HKIP_TASK_BITS(gid_root_bits);

static bool hkip_compute_uid_root(const struct cred *creds)
{
	return uid_eq(creds->uid, GLOBAL_ROOT_UID) ||
		uid_eq(creds->euid, GLOBAL_ROOT_UID) ||
		uid_eq(creds->suid, GLOBAL_ROOT_UID) ||
	/* Note: FSUID can only change when EUID is zero. So a change of FSUID
	 * will not affect the overall root status bit: it will remain true. */
		!cap_isclear(creds->cap_inheritable) ||
		!cap_isclear(creds->cap_permitted);
}

int hkip_check_uid_root(void)
{
	const struct cred *creds;

	if (hkip_get_current_bit(hkip_uid_root_bits, true)) {
		return 0;
	}

	/* Note: In principles, FSUID cannot be zero if EGID is non-zero.
	 * But we check it separately anyway, in case of memory corruption. */
	creds = (struct cred *)current_cred();/*lint !e666*/

	if (unlikely(hkip_compute_uid_root(creds) ||
			uid_eq(creds->fsuid, GLOBAL_ROOT_UID))) {
		pr_alert("UID root escalation!\n");
		force_sig(SIGKILL, current);
		return -EPERM;
	}

	return 0;
}
EXPORT_SYMBOL(hkip_check_uid_root);

static void hkip_update_uid_root(const struct cred *creds)
{
	bool value = hkip_compute_uid_root(creds);
	hkip_set_current_bit(hkip_uid_root_bits, value);
}

static bool hkip_compute_gid_root(const struct cred *creds)
{
	return gid_eq(creds->gid, GLOBAL_ROOT_GID) ||
		gid_eq(creds->sgid, GLOBAL_ROOT_GID) ||
		in_egroup_p(GLOBAL_ROOT_GID) ;
}

int hkip_check_gid_root(void)
{
	const struct cred *creds;

	if (hkip_get_current_bit(hkip_gid_root_bits, true)) {
		return 0;
	}

	creds = (struct cred *)current_cred();/*lint !e666*/

	if (unlikely(hkip_compute_gid_root(creds) ||
			gid_eq(creds->fsgid, GLOBAL_ROOT_GID))) {
		pr_alert("GID root escalation!\n");
		force_sig(SIGKILL, current);
		return -EPERM;
	}

	return 0;
}
EXPORT_SYMBOL(hkip_check_gid_root);

static void hkip_update_gid_root(const struct cred *creds)
{
	bool value = hkip_compute_gid_root(creds);
	hkip_set_current_bit(hkip_gid_root_bits, value);
}

int hkip_check_xid_root(void)
{
	return hkip_check_uid_root() ?: hkip_check_gid_root();
}
EXPORT_SYMBOL(hkip_check_xid_root);

void hkip_update_xid_root(const struct cred *creds)
{
	hkip_update_uid_root(creds);
	hkip_update_gid_root(creds);
}
EXPORT_SYMBOL(hkip_update_xid_root);

void hkip_init_task(struct task_struct *task)
{
	/* NOTE: Taking RCU and incrementing the reference count is useless
	 * here, as the thread is not running yet, and credentials cannot
	 * change. But no clean ways to avoid it. */
	const struct cred *creds = get_task_cred(task);

	hkip_set_task_bit(hkip_uid_root_bits, task,
				hkip_compute_uid_root(creds));
	hkip_set_task_bit(hkip_gid_root_bits, task,
				hkip_compute_gid_root(creds));
	put_cred(creds);

	/* thread info is inherited(copied) from parent thread */
	hkip_set_task_bit(hkip_addr_limit_bits, task, hkip_is_kernel_fs());/*lint !e501*/
}

static int __init hkip_critdata_init(void)
{
	hkip_register_bits(hkip_addr_limit_bits, sizeof (hkip_addr_limit_bits));
	hkip_register_bits(hkip_uid_root_bits, sizeof (hkip_uid_root_bits));
	hkip_register_bits(hkip_gid_root_bits, sizeof (hkip_gid_root_bits));
	return 0;
}

module_init(hkip_critdata_init);
MODULE_DESCRIPTION("Huawei kernel critical data protection");
MODULE_AUTHOR("Remi Denis-Courmont <remi.denis.courmont@huawei.com>");
MODULE_LICENSE("GPL");
