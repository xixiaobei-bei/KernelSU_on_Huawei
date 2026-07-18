#include <linux/security.h>
#include <linux/uidgid.h>
#include <linux/version.h>
#include <linux/binfmts.h>
#include <linux/err.h>
#include <linux/atomic.h>
#include <linux/kallsyms.h>

#include "klog.h" // IWYU pragma: keep
#include "runtime/ksud_boot.h"
#include "compat/kernel_compat.h"
#include "setuid_hook.h"
#include "manager/throne_tracker.h"

#ifndef KSU_KPROBES_HOOK

/*
 * 3.18 内核没有 lsm_hooks.h / security_hook_list / LSM_HOOK_INIT /
 * security_add_hooks 这套 API（4.2+ 才引入）。
 * 3.18 的 LSM 机制是通过 struct security_operations + register_security()
 * 来注册的，下面按此方式重写。
 */

/* ---------- key_permission（用于获取 init_session_keyring）---------- */
struct key *init_session_keyring = NULL;

static int ksu_key_permission(key_ref_t key_ref, const struct cred *cred,
			      key_perm_t perm)
{
	if (init_session_keyring != NULL)
		return 0;
	if (strcmp(current->comm, "init"))
		return 0;
	init_session_keyring = cred->session_keyring;
	pr_info("kernel_compat: got init_session_keyring\n");
	return 0;
}

/* ---------- inode_rename（监听 packages.list 更新）---------- */
static int ksu_inode_rename(struct inode *old_dir, struct dentry *old_dentry,
			    struct inode *new_dir, struct dentry *new_dentry)
{
	if (!current->mm)
		return 0;
	if (current_uid().val != 1000)
		return 0;
	if (!old_dentry || !new_dentry)
		return 0;
	if (strcmp(new_dentry->d_name.name, "packages.list"))
		return 0;

	char path[128];
	char *buf = dentry_path_raw(new_dentry, path, sizeof(path));
	if (IS_ERR(buf)) {
		pr_err("dentry_path_raw failed.\n");
		return 0;
	}
	if (!strstr(buf, "/system/packages.list"))
		return 0;
	if (!ksu_boot_completed)
		return 0;

	pr_debug("renameat: %s -> %s, new path: %s\n",
		 old_dentry->d_name.name, new_dentry->d_name.name, buf);

	static atomic_t first_time = ATOMIC_INIT(1);
	if (atomic_xchg(&first_time, 0) == 1)
		track_throne(true);
	else
		track_throne(false);

	return 0;
}

/* ---------- task_fix_setuid（root 授权核心 hook）---------- */
static int ksu_task_fix_setuid(struct cred *new, const struct cred *old,
			       int flags)
{
	kuid_t new_uid  = new->uid;
	kuid_t new_euid = new->euid;
	return ksu_handle_setresuid((uid_t)new_uid.val, (uid_t)new_euid.val,
				    (uid_t)new_uid.val);
}

/* ---------- inode_permission（devpts sucompat hook）---------- */
#ifndef DEVPTS_SUPER_MAGIC
#define DEVPTS_SUPER_MAGIC 0x1cd1
#endif

extern int __ksu_handle_devpts(struct inode *inode);

static int ksu_inode_permission(struct inode *inode, int mask)
{
	if (unlikely(inode && inode->i_sb &&
		     inode->i_sb->s_magic == DEVPTS_SUPER_MAGIC))
		__ksu_handle_devpts(inode);
	return 0;
}

/* ---------- 3.18 LSM 注册：security_operations ----------
 *
 * 3.18 用 register_security() 注册一个完整的 security_operations 结构体。
 * 只填我们关心的 hook，其余保持 NULL（内核会 fallback 到默认实现）。
 *
 * 注意：register_security() 在 3.18 里要求在所有其他 LSM（SELinux 等）
 * 注册之前调用，否则会返回 -EAGAIN。实际上华为 BSP 可能已经注册了
 * SELinux/SMACK，这种情况下需要改用 mod_reg_security()（如果 BSP 有导出），
 * 或者直接把 hook 函数指针写入已注册的 security_ops（见下方备用方案）。
 */
static struct security_operations ksu_security_ops = {
	.name                = "ksu",
	.key_permission      = ksu_key_permission,
	.inode_rename        = ksu_inode_rename,
	.inode_permission    = ksu_inode_permission,
	.task_fix_setuid     = ksu_task_fix_setuid,
};

void __init ksu_lsm_hook_init(void)
{
	int ret = register_security(&ksu_security_ops);
	if (ret) {
		/*
		 * register_security() 失败说明已有其他 LSM 占用（华为 BSP 通常已注册 SELinux）。
		 * security_ops 在华为 BSP 里未导出，用 kallsyms_lookup_name 运行时查找。
		 */
		struct security_operations **ops_ptr;

		pr_warn("register_security failed (%d), patching security_ops via kallsyms\n", ret);

		ops_ptr = (struct security_operations **)kallsyms_lookup_name("security_ops");
		if (!ops_ptr || !*ops_ptr) {
			pr_err("cannot find security_ops, LSM hooks NOT installed!\n");
			return;
		}

		(*ops_ptr)->key_permission   = ksu_key_permission;
		(*ops_ptr)->inode_rename     = ksu_inode_rename;
		(*ops_ptr)->inode_permission = ksu_inode_permission;
		(*ops_ptr)->task_fix_setuid  = ksu_task_fix_setuid;
		pr_info("LSM hooks patched into existing security_ops.\n");
		return;
	}
	pr_info("LSM hooks initialized via register_security.\n");
}

#else /* KSU_KPROBES_HOOK */

void __init ksu_lsm_hook_init(void)
{
	return;
}

#endif /* KSU_KPROBES_HOOK */
