/* SPDX-License-Identifier: GPL-2.0 */
/*
 * authentication.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: chenyi77@huawei.com
 * Create: 2020-06-22
 *
 */
#include "authentication.h"
#include <linux/fsnotify.h>
#include <linux/security.h>

#include "hmdfs.h"

struct fs_struct *hmdfs_override_fsstruct(struct fs_struct *saved_fs)
{
#if (defined CONFIG_HMDFS_ANDROID) && (defined CONFIG_SDCARD_FS)
	struct fs_struct *copied_fs = copy_fs_struct(saved_fs);

	if (!copied_fs)
		return NULL;
	copied_fs->umask = 0;
	task_lock(current);
	current->fs = copied_fs;
	task_unlock(current);
	return copied_fs;
#else
	return saved_fs;
#endif
}

void hmdfs_revert_fsstruct(struct fs_struct *saved_fs,
			   struct fs_struct *copied_fs)
{
#if (defined CONFIG_HMDFS_ANDROID) && (defined CONFIG_SDCARD_FS)
	task_lock(current);
	current->fs = saved_fs;
	task_unlock(current);
	free_fs_struct(copied_fs);
#endif
}

const struct cred *hmdfs_override_fsids(bool is_recv_thread)
{
	struct cred *cred = NULL;
	const struct cred *old_cred = NULL;

	cred = prepare_creds();
	if (!cred)
		return NULL;

	cred->fsuid = MEDIA_RW_UID;
	cred->fsgid = is_recv_thread ?
		      KGIDT_INIT((gid_t)AID_EVERYBODY) : MEDIA_RW_GID;

	old_cred = override_creds(cred);

	return old_cred;
}

const struct cred *hmdfs_override_dir_fsids(struct inode *dir,
					    struct dentry *dentry, __u16 *_perm)
{
	struct hmdfs_inode_info *hii = hmdfs_i(dir);
	struct cred *cred = NULL;
	const struct cred *old_cred = NULL;
	__u16 level = hmdfs_perm_get_next_level(hii->perm);
	__u16 perm = 0;

	cred = prepare_creds();
	if (!cred)
		return NULL;

	switch (level) {
	case HMDFS_PERM_MNT:
		/* system : media_rw */
		cred->fsuid = SYSTEM_UID;
		perm = (hii->perm & HMDFS_DIR_TYPE_MASK) | level;
		break;
	case HMDFS_PERM_DFS:
		/*
		 * data  : system : media_rw
		 * system: system : media_rw, need authority
		 * other : media_rw : media_rw
		 **/
		if (!strcmp(dentry->d_name.name, PKG_ROOT_NAME)) {
			cred->fsuid = SYSTEM_UID;
			perm = HMDFS_DIR_DATA | level;
		} else if (!strcmp(dentry->d_name.name, SYSTEM_NAME)) {
			cred->fsuid = SYSTEM_UID;
			perm = AUTH_SYSTEM | HMDFS_DIR_SYSTEM | level;
		} else {
			cred->fsuid = MEDIA_RW_UID;
			perm = HMDFS_DIR_PUBLIC | level;
		}
		break;
	case HMDFS_PERM_PKG:
		if (is_data_dir(hii->perm)) {
			/*
			 * Mkdir for app pkg.
			 * Get the appid by passing pkgname to configfs.
			 * Set ROOT + media_rw for remote install,
			 * local uninstall.
			 * Set appid + media_rw for local install.
			 */
			uid_t app_id = hmdfs_get_appid(dentry->d_name.name);

			if (app_id != 0)
				cred->fsuid = KUIDT_INIT(app_id);
			else
				cred->fsuid = ROOT_UID;
			perm = AUTH_PKG | HMDFS_DIR_PKG | level;
		} else {
			cred->fsuid = dir->i_uid;
			perm = (hii->perm & AUTH_MASK) | HMDFS_DIR_DEFAULT | level;
		}
		break;
	case HMDFS_PERM_OTHER:
		cred->fsuid = dir->i_uid;
		if (is_pkg_auth(hii->perm))
			perm = AUTH_PKG | HMDFS_DIR_PKG_SUB | level;
		else
			perm = (hii->perm & AUTH_MASK) | HMDFS_DIR_DEFAULT | level;
		break;
	default:
		/* ! it should not get to here */
		hmdfs_err("hmdfs perm incorrect got default case, level:%u", level);
		break;
	}

	cred->fsgid = MEDIA_RW_GID;
	*_perm = perm;
	old_cred = override_creds(cred);

	return old_cred;
}

int hmdfs_override_dir_id_fs(struct cache_fs_override *or,
			struct inode *dir,
			struct dentry *dentry,
			__u16 *perm)
{
	or->saved_cred = hmdfs_override_dir_fsids(dir, dentry, perm);
	if (!or->saved_cred)
		return -ENOMEM;

	or->saved_fs = current->fs;
	or->copied_fs = hmdfs_override_fsstruct(or->saved_fs);
	if (!or->copied_fs) {
		hmdfs_revert_fsids(or->saved_cred);
		return -ENOMEM;
	}

	return 0;
}

void hmdfs_revert_dir_id_fs(struct cache_fs_override *or)
{
	hmdfs_revert_fsstruct(or->saved_fs, or->copied_fs);
	hmdfs_revert_fsids(or->saved_cred);
}

const struct cred *hmdfs_override_file_fsids(struct inode *dir, __u16 *_perm)
{
	struct hmdfs_inode_info *hii = hmdfs_i(dir);
	struct cred *cred = NULL;
	const struct cred *old_cred = NULL;
	__u16 level = hmdfs_perm_get_next_level(hii->perm);
	uint16_t perm;

	perm = HMDFS_FILE_DEFAULT | level;

	cred = prepare_creds();
	if (!cred)
		return NULL;

	cred->fsuid = dir->i_uid;
	cred->fsgid = dir->i_gid;
	if (is_pkg_auth(hii->perm))
		perm = AUTH_PKG | HMDFS_FILE_PKG_SUB | level;
	else
		perm = (hii->perm & AUTH_MASK) | HMDFS_FILE_DEFAULT | level;

	*_perm = perm;
	old_cred = override_creds(cred);

	return old_cred;
}

void hmdfs_revert_fsids(const struct cred *old_cred)
{
	const struct cred *cur_cred;

	cur_cred = current->cred;
	revert_creds(old_cred);
	put_cred(cur_cred);
}

int hmdfs_persist_perm(struct dentry *dentry, __u16 *perm)
{
	int err;
	struct inode *minode = d_inode(dentry);

	if (!minode)
		return -EINVAL;

	inode_lock(minode);
	err = __vfs_setxattr(dentry, minode, HMDFS_PERM_XATTR, perm,
			     sizeof(*perm), XATTR_CREATE);
	if (!err)
		fsnotify_xattr(dentry);
	else if (err && err != -EEXIST)
		hmdfs_err("failed to setxattr, err=%d", err);
	inode_unlock(minode);
	return err;
}

__u16 hmdfs_read_perm(struct inode *inode)
{
	__u16 ret = 0;
	int size = 0;
	struct dentry *dentry = d_find_alias(inode);

	if (!dentry)
		return ret;

	size = __vfs_getxattr(dentry, inode, HMDFS_PERM_XATTR, &ret,
			     sizeof(ret));
	 /*
	  * some file may not set setxattr with perm
	  * eg. files created in sdcard dir by other user
	  * */
	if (size < 0 || size != sizeof(ret))
		ret = HMDFS_ALL_MASK;

	dput(dentry);
	return ret;
}

static __u16 __inherit_perm_dir(struct inode *parent, struct inode *inode)
{
	__u16 perm = 0;
	struct hmdfs_inode_info *info = hmdfs_i(parent);
	__u16 level = hmdfs_perm_get_next_level(info->perm);
	struct dentry *dentry = d_find_alias(inode);

	if (!dentry)
		return perm;

	switch (level) {
	case HMDFS_PERM_MNT:
		/* system : media_rw */
		perm = (info->perm & HMDFS_DIR_TYPE_MASK) | level;
		break;
	case HMDFS_PERM_DFS:
		/*
		 * data  : system : media_rw
		 * system: system : media_rw, need authority
		 * other : media_rw : media_rw
		 **/
		if (!strcmp(dentry->d_name.name, PKG_ROOT_NAME)) {
			// "data"
			perm = HMDFS_DIR_DATA | level;
		} else if (!strcmp(dentry->d_name.name, SYSTEM_NAME)) {
			 // "system"
			perm = AUTH_SYSTEM | HMDFS_DIR_SYSTEM | level;
		} else {
			perm = HMDFS_DIR_PUBLIC | level;
		}
		break;
	case HMDFS_PERM_PKG:
		if (is_data_dir(info->perm)) {
			/*
			 * Mkdir for app pkg.
			 * Get the appid by passing pkgname to configfs.
			 * Set ROOT + media_rw for remote install,
			 * local uninstall.
			 * Set appid + media_rw for local install.
			 */
			perm = AUTH_PKG | HMDFS_DIR_PKG | level;
		} else {
			perm = (info->perm & AUTH_MASK) | HMDFS_DIR_DEFAULT | level;
		}
		break;
	case HMDFS_PERM_OTHER:
		if (is_pkg_auth(info->perm))
			perm = AUTH_PKG | HMDFS_DIR_PKG_SUB | level;
		else
			perm = (info->perm & AUTH_MASK) | HMDFS_DIR_DEFAULT | level;
		break;
	default:
		/* ! it should not get to here */
		hmdfs_err("hmdfs perm incorrect got default case, level:%u", level);
		break;
	}
	dput(dentry);
	return perm;
}

static __u16 __inherit_perm_file(struct inode *parent)
{
	struct hmdfs_inode_info *hii = hmdfs_i(parent);
	__u16 level = hmdfs_perm_get_next_level(hii->perm);
	uint16_t perm;

	perm = HMDFS_FILE_DEFAULT | level;

	if (is_pkg_auth(hii->perm))
		perm = AUTH_PKG | HMDFS_FILE_PKG_SUB | level;
	else
		perm = (hii->perm & AUTH_MASK) | HMDFS_FILE_DEFAULT | level;

	return perm;
}

static void fixup_ownership(struct inode *child, struct dentry *lower_dentry,
		     uid_t uid)
{
	int err;
	struct iattr newattrs;

	newattrs.ia_valid = ATTR_UID | ATTR_FORCE;
	newattrs.ia_uid = KUIDT_INIT(uid);
	if (!S_ISDIR(d_inode(lower_dentry)->i_mode))
		newattrs.ia_valid |= ATTR_KILL_SUID | ATTR_KILL_PRIV;

	inode_lock(d_inode(lower_dentry));
	err = notify_change(lower_dentry, &newattrs, NULL);
	inode_unlock(d_inode(lower_dentry));

	if (!err)
		child->i_uid = KUIDT_INIT(uid);
	else
		hmdfs_err("update PKG uid failed, err = %d", err);
}

static void fixup_ownership_user_group(struct inode *child, struct dentry *lower_dentry,
		     uid_t uid, gid_t gid)
{
	int err;
	struct iattr newattrs;

	newattrs.ia_valid = ATTR_UID | ATTR_GID | ATTR_FORCE;
	newattrs.ia_uid = KUIDT_INIT(uid);
	newattrs.ia_gid = KGIDT_INIT(gid);
	if (!S_ISDIR(d_inode(lower_dentry)->i_mode))
		newattrs.ia_valid |= ATTR_KILL_SUID | ATTR_KILL_SGID | ATTR_KILL_PRIV;

	inode_lock(d_inode(lower_dentry));
	err = notify_change(lower_dentry, &newattrs, NULL);
	inode_unlock(d_inode(lower_dentry));

	if (!err) {
		child->i_uid = KUIDT_INIT(uid);
		child->i_gid = KGIDT_INIT(gid);
	} else {
		hmdfs_err("update PKG uid failed, err = %d", err);
	}
}

__u16 hmdfs_perm_inherit(struct inode *parent_inode, struct inode *child)
{
	__u16 perm;

	if (S_ISDIR(child->i_mode))
		perm = __inherit_perm_dir(parent_inode, child);
	else
		perm = __inherit_perm_file(parent_inode);
	return perm;
}

void check_and_fixup_ownership(struct inode *parent_inode, struct inode *child,
			       struct dentry *lower_dentry, const char *name)
{
	uid_t appid;
	struct hmdfs_inode_info *info = hmdfs_i(child);

	if (info->perm == HMDFS_ALL_MASK)
		info->perm = hmdfs_perm_inherit(parent_inode, child);

	switch (info->perm & HMDFS_DIR_TYPE_MASK) {
	case HMDFS_DIR_PKG:
		appid = hmdfs_get_appid(name);
		if (appid != child->i_uid.val)
			fixup_ownership(child, lower_dentry, appid);

		break;
	case HMDFS_DIR_DATA:
	case HMDFS_FILE_PKG_SUB:
	case HMDFS_DIR_DEFAULT:
	case HMDFS_FILE_DEFAULT:
		if (parent_inode->i_uid.val != child->i_uid.val ||
		    parent_inode->i_gid.val != child->i_gid.val)
			fixup_ownership_user_group(child, lower_dentry,
					parent_inode->i_uid.val,
					parent_inode->i_gid.val);
		break;
	case HMDFS_DIR_PUBLIC:
		fixup_ownership(child, lower_dentry, (uid_t)AID_MEDIA_RW);

		break;
	default:
		break;
	}
}

void check_and_fixup_ownership_remote(struct inode *dir,
				      struct dentry *dentry)
{
	struct hmdfs_inode_info *hii = hmdfs_i(dir);
	struct inode *dinode = d_inode(dentry);
	struct hmdfs_inode_info *dinfo = hmdfs_i(dinode);
	__u16 level = hmdfs_perm_get_next_level(hii->perm);
	__u16 perm = 0;

	hmdfs_debug("level:0x%X", level);
	switch (level) {
	case HMDFS_PERM_MNT:
		/* system : media_rw */
		dinode->i_uid = SYSTEM_UID;
		perm = (hii->perm & HMDFS_DIR_TYPE_MASK) | level;
		break;
	case HMDFS_PERM_DFS:
		/*
		 * data  : system : media_rw
		 * system: system : media_rw, need authority
		 * other : media_rw : media_rw
		 **/
		if (!strcmp(dentry->d_name.name, PKG_ROOT_NAME)) {
			// "data"
			dinode->i_uid = SYSTEM_UID;
			perm = HMDFS_DIR_DATA | level;
		} else if (!strcmp(dentry->d_name.name, SYSTEM_NAME)) {
			 // "system"
			dinode->i_uid = SYSTEM_UID;
			perm = AUTH_SYSTEM | HMDFS_DIR_SYSTEM | level;
		} else {
			dinode->i_uid = MEDIA_RW_UID;
			perm = HMDFS_DIR_PUBLIC | level;
		}
		break;
	case HMDFS_PERM_PKG:
		if (is_data_dir(hii->perm)) {
			/*
			 * Mkdir for app pkg.
			 * Get the appid by passing pkgname to configfs.
			 * Set ROOT + media_rw for remote install,
			 * local uninstall.
			 * Set appid + media_rw for local install.
			 */
			uid_t app_id = hmdfs_get_appid(dentry->d_name.name);

			if (app_id != 0)
				dinode->i_uid = KUIDT_INIT(app_id);
			else
				dinode->i_uid = ROOT_UID;
			perm = AUTH_PKG | HMDFS_DIR_PKG | level;
		} else {
			dinode->i_uid = dir->i_uid;
			perm = (hii->perm & AUTH_MASK) | HMDFS_DIR_DEFAULT | level;
		}
		break;
	case HMDFS_PERM_OTHER:
		dinode->i_uid = dir->i_uid;
		if (is_pkg_auth(hii->perm))
			perm = AUTH_PKG | HMDFS_DIR_PKG_SUB | level;
		else
			perm = (hii->perm & AUTH_MASK) | HMDFS_DIR_DEFAULT | level;
		break;
	default:
		/* ! it should not get to here */
		hmdfs_err("hmdfs perm incorrect got default case, level:%u", level);
		break;
	}

	dinode->i_gid = MEDIA_RW_GID;
	dinfo->perm = perm;
}

void hmdfs_root_inode_perm_init(struct inode *root_inode)
{
	struct hmdfs_inode_info *hii = hmdfs_i(root_inode);

	hii->perm = HMDFS_DIR_ROOT | HMDFS_PERM_MNT;
	set_inode_uid(root_inode, SYSTEM_UID);
	set_inode_gid(root_inode, MEDIA_RW_GID);
}
