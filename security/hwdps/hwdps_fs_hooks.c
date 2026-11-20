/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Hwdps hooks for file system(f2fs).
 * Create: 2020-06-16
 */

#include "huawei_platform/hwdps/hwdps_fs_hooks.h"
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/rwsem.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/fscrypt_common.h>
#include <log/hiview_hievent.h>
#include <securec.h>
#include <uapi/linux/stat.h>
#include <huawei_platform/hwdps/hwdps_limits.h>

#ifdef CONFIG_HWDPS
static DECLARE_RWSEM(g_fs_callbacks_lock);

#define HWDPS_XATTR_NAME "hwdps"
#define HWDPS_KEY_DESC_STANDARD_FLAG 0x42

#define HWDPS_HIVIEW_ID 940016001
#define HWDPS_HIVIEW_CREATE 1000
#define HWDPS_HIVIEW_OPEN 1001
#define HWDPS_HIVIEW_UPDATE 1002
#define HWDPS_HIVIEW_ACCESS 1003
#define HWDPS_HIVIEW_ENCRYPT 1004

static hwdps_result_t default_create_fek(const u8 *desc, const u8 *fsname,
	const struct dentry *dentry, fek_efek_t *fek_efek, u32 parent_flags)
{
	(void)desc;
	(void)fsname;
	(void)dentry;
	(void)fek_efek;
	(void)parent_flags;
	return HWDPS_ERR_NO_FS_CALLBACKS;
}

static hwdps_result_t default_hwdps_has_access(const encrypt_id *id,
	const buffer_t *encoded_wfek, s32 flags)
{
	(void)id;
	(void)encoded_wfek;
	(void)flags;
	return HWDPS_ERR_NO_FS_CALLBACKS;
}

static hwdps_result_t default_get_fek(const u8 *desc, const encrypt_id *id,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek, u32 flags)
{
	(void)desc;
	(void)id;
	(void)encoded_wfek;
	(void)fek;
	(void)flags;
	return HWDPS_ERR_NO_FS_CALLBACKS;
}

static hwdps_result_t default_update_fek(const u8 *desc,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek, uid_t new_uid,
	uid_t old_uid)
{
	(void)desc;
	(void)encoded_wfek;
	(void)fek;
	(void)new_uid;
	(void)old_uid;
	return HWDPS_ERR_NO_FS_CALLBACKS;
}

static hwdps_result_t default_update_xattr_efek(const u8 *desc, uid_t uid,
	secondary_buffer_t *fek, struct inode *inode)
{
	(void)desc;
	(void)uid;
	(void)fek;
	(void)inode;
	return HWDPS_ERR_NO_FS_CALLBACKS;
}

static hwdps_result_t default_encrypt_fek(const u8 *desc,
	struct inode *inode,
	secondary_buffer_t *encoded_wfek)
{
	(void)desc;
	(void)inode;
	(void)encoded_wfek;
	return HWDPS_ERR_NO_FS_CALLBACKS;
}

static struct hwdps_fs_callbacks_t g_fs_callbacks = {
	.create_fek = default_create_fek,
	.hwdps_has_access = default_hwdps_has_access,
	.get_fek = default_get_fek,
	.update_fek = default_update_fek,
	.update_xattr_efek = default_update_xattr_efek,
	.encrypt_fek = default_encrypt_fek,
};

static hwdps_result_t hwdps_create_fek(const u8 *desc, struct inode *inode,
	const struct dentry *dentry, fek_efek_t *fek_efek, u32 parent_flags)
{
	hwdps_result_t res;
	const u8 *fsname = NULL;

	if (!inode || !inode->i_sb || !inode->i_sb->s_type)
		return HWDPS_ERR_INVALID_ARGS;

	fsname = inode->i_sb->s_type->name;
	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.create_fek(desc, fsname, dentry,
		fek_efek, parent_flags);
	up_read(&g_fs_callbacks_lock);

	return res;
}

static void hiview_for_hwdps(int type, hwdps_result_t result,
	const encrypt_id *id)
{
	struct hiview_hievent *event = hiview_hievent_create(HWDPS_HIVIEW_ID);

	if (!event) {
		pr_err("hwdps hiview event null");
		return;
	}

	/*lint -e644*/
	pr_info("%s result %d, %d, %u, %u\n", __func__,
		type, result, id->uid, id->task_uid);
	/*lint +e644*/
	hiview_hievent_put_integral(event, "type", type);
	hiview_hievent_put_integral(event, "result", result);
	hiview_hievent_put_integral(event, "task_uid",
		id->task_uid); //lint !e644
	hiview_hievent_put_integral(event, "uid", id->uid); //lint !e644

	hiview_hievent_report(event);
	hiview_hievent_destroy(event);
}

static void get_create_task_uid(encrypt_id *id)
{
	const struct cred *cred = get_current_cred(); //lint !e666

	if (!cred) {
		pr_err("%s cred error\n", __func__);
		id->task_uid = 0;
		id->uid = id->task_uid;
		return;
	}
	id->task_uid = cred->uid.val; /* task uid */
	put_cred(cred);
	id->uid = id->task_uid;
}

s32 f2fs_set_hwdps_enable_flags(struct inode *inode, void *fs_data)
{
	u32 flags;
	s32 res;

	if (!inode || !inode->i_sb || !inode->i_sb->s_cop ||
		!inode->i_sb->s_cop->get_hwdps_flags ||
		!inode->i_sb->s_cop->set_hwdps_flags) {
		pr_err("%s inode NULL\n", __func__);
		return -EINVAL;
	}

	res = inode->i_sb->s_cop->get_hwdps_flags(inode, fs_data, &flags);
	if (res != 0) {
		pr_err("%s get inode %lu hwdps flags res %d\n",
			__func__, inode->i_ino, res);
		return -EINVAL;
	}

	flags &= (~HWDPS_ENABLE_FLAG);
	flags |= HWDPS_XATTR_ENABLE_FLAG_NEW;
	res = inode->i_sb->s_cop->set_hwdps_flags(inode, fs_data, &flags);
	if (res != 0) {
		pr_err("%s set inode %lu hwdps flag res %d\n",
			__func__, inode->i_ino, res);
		return -EINVAL;
	}
	return res;
}

hwdps_result_t hwdps_has_access(struct inode *inode,
	const buffer_t *encoded_wfek, s32 flags)
{
	hwdps_result_t res;
	encrypt_id id;
	const struct cred *cred = NULL;

	if (!inode)
		return HWDPS_ERR_INVALID_ARGS;

	id.pid = task_tgid_nr(current);
	cred = get_current_cred(); //lint !e666
	if (!cred)
		return HWDPS_ERR_INVALID_ARGS;

	id.task_uid = cred->uid.val;
	put_cred(cred);
	id.uid = inode->i_uid.val;

	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.hwdps_has_access(&id, encoded_wfek, flags);
	up_read(&g_fs_callbacks_lock);

	if (res != 0)
		hiview_for_hwdps(HWDPS_HIVIEW_ACCESS, res, &id);
	return res;
}

hwdps_result_t hwdps_get_fek(const u8 *desc, struct inode *inode,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek, u32 flags)
{
	encrypt_id ids;
	hwdps_result_t res;
	const struct cred *cred = NULL;

	ids.pid = task_tgid_nr(current);
	if (!inode) {
		ids.task_uid = 0;
		ids.uid = 0;
		res = HWDPS_ERR_INTERNAL;
		goto out;
	}

	cred = get_current_cred(); //lint !e666
	if (!cred) {
		ids.task_uid = 0;
		ids.uid = 0;
		res = HWDPS_ERR_INTERNAL;
		goto out;
	}

	ids.task_uid = cred->uid.val; /* task uid */
	ids.uid = inode->i_uid.val; /* file uid */
	put_cred(cred);

	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.get_fek(desc, &ids, encoded_wfek, fek, flags);
	if (res != 0)
		goto out;
	if (flags == HWDPS_XATTR_ENABLE_FLAG) {
		res = g_fs_callbacks.update_xattr_efek(desc, ids.uid,
			fek, inode);
		if (res != 0)
			goto out;
		res = f2fs_set_hwdps_enable_flags(inode, NULL);
		if (res != 0)
			pr_err("hwdps ino %lu set hwdps enable flags err %d\n",
				inode->i_ino, res);
	}
out:
	up_read(&g_fs_callbacks_lock);
	if (res != 0)
		hiview_for_hwdps(HWDPS_HIVIEW_OPEN, res, &ids);
	return res;
}

s32 hwdps_check_support(struct inode *inode, uint32_t *flags)
{
	s32 err = 0;

	if (!flags)
		return -EINVAL;
	if (!inode || !inode->i_sb || !inode->i_sb->s_cop ||
		!inode->i_sb->s_cop->get_hwdps_flags)
		return -EOPNOTSUPP;

	if (!S_ISREG(inode->i_mode))
		return -EOPNOTSUPP;
	/*
	 * The inode->i_crypt_info->ci_hw_enc_flag keeps sync with the
	 * flags in xattr_header. And it can not be changed once the
	 * file is opened.
	 */
	if (!inode->i_crypt_info)
		err = inode->i_sb->s_cop->get_hwdps_flags(inode, NULL, flags);
	else
		*flags = (u32)(inode->i_crypt_info->ci_hw_enc_flag);

	if (err != 0)
		pr_err("hwdps ino %lu get flags err %d\n", inode->i_ino, err);
	else if ((*flags & HWDPS_ENABLE_FLAG) == 0)
		err = -EOPNOTSUPP;

	return err;
}

uint8_t *hwdps_do_get_attr(struct inode *inode, size_t size, u32 flags)
{
	s32 err;
	uint8_t *wfek = NULL;

	if (((flags == HWDPS_XATTR_ENABLE_FLAG_NEW) &&
		(size != HWDPS_ENCODED_WFEK_SIZE)) ||
		((flags == HWDPS_XATTR_ENABLE_FLAG) &&
		(size != HWDPS_ENCODE_WFEK_SIZE_OLD))) {
		pr_err("%s size err %lu\n", __func__, size);
		return NULL;
	}

	if (!inode || !inode->i_sb || !inode->i_sb->s_cop ||
		!inode->i_sb->s_cop->get_hwdps_attr)
		return NULL;

	wfek = kzalloc(size, GFP_NOFS);
	if (!wfek)
		return NULL;

	err = inode->i_sb->s_cop->get_hwdps_attr(inode, wfek, size, flags,
		NULL);
	if (err == -ENODATA) {
		pr_err("hwdps ino %lu hwdps xattr is null\n", inode->i_ino);
		goto free_out;
	} else if (err != size) {
		pr_err("hwdps ino %lu wrong encoded_wfek size %d\n",
			inode->i_ino, err);
		goto free_out;
	}
	return wfek;

free_out:
	kzfree(wfek);
	return NULL;
}

static s32 hwdps_dir_inherit_flags(struct inode *inode,
	void *fs_data, u32 parent_flags)
{
	s32 err;
	uint8_t encoded_wfek[HWDPS_ENCODED_WFEK_SIZE] = {0};

	if (!S_ISDIR(inode->i_mode))
		return 0;

	if ((parent_flags & HWDPS_XATTR_ENABLE_FLAG_NEW) != 0) {
		err = inode->i_sb->s_cop->set_hwdps_attr(inode, encoded_wfek,
			HWDPS_ENCODED_WFEK_SIZE, fs_data);
		if (err != 0) {
			pr_err("%s set_hwdps_attr err : %d\n", __func__, err);
			return err;
		}

		return f2fs_set_hwdps_enable_flags(inode, fs_data);
	}

	return 0;
}

static s32 hwdps_get_parent_dps_flag(struct inode *parent, u32 *flags,
	struct page *page)
{
	s32 err;
	uint8_t *wfek = NULL;

	if (!parent || !parent->i_sb || !parent->i_sb->s_cop ||
		!parent->i_sb->s_cop->get_hwdps_attr)
		return -ENODATA;

	wfek = kzalloc(HWDPS_ENCODED_WFEK_SIZE, GFP_NOFS);
	if (!wfek)
		return -ENOMEM;

	err = parent->i_sb->s_cop->get_hwdps_attr(parent, wfek,
		HWDPS_ENCODED_WFEK_SIZE, HWDPS_XATTR_ENABLE_FLAG_NEW, page);
	if (err == -ENODATA) {
		*flags = 0;
	} else if (err != HWDPS_ENCODED_WFEK_SIZE) {
		pr_err("ino %lu dir xattr size wrong %d\n",
			parent->i_ino, err);
		*flags = 0;
	} else {
		pr_info("hwdps ino %lu, parent has dps flag\n", parent->i_ino);
		*flags = HWDPS_XATTR_ENABLE_FLAG_NEW;
	}

	kfree(wfek);
	return 0;
}

static s32 check_params(const struct inode *parent,
	const struct inode *inode, const struct dentry *dentry,
	const void *fs_data)
{
	if (!dentry || !inode || !fs_data || !parent)
		return -EAGAIN;
	if (!parent->i_crypt_info) {
		pr_err("hwdps parent ci is null error\n");
		return -ENOKEY;
	}

	return HWDPS_SUCCESS;
}

static s32 set_inode_attr(struct inode *parent, struct inode *inode,
	const uint8_t *encoded_wfek, uint32_t fek_len, void *fs_data)
{
	if (parent->i_sb->s_cop->set_hwdps_attr)
		return parent->i_sb->s_cop->set_hwdps_attr(inode, encoded_wfek,
			fek_len, fs_data);
	pr_info("hwdps ino %lu no setxattr\n", inode->i_ino);
	return 0;
}

/*
 * Code is mainly copied from fscrypt_inherit_context
 *
 * funcs except hwdps_create_fek must not return EAGAIN
 *
 * Return:
 *  o 0: SUCC
 *  o other errno: the file is not supported by policy
 */
s32 hwdps_inherit_context(struct inode *parent, struct inode *inode,
	const struct dentry *dentry, void *fs_data, struct page *page)
{
	encrypt_id id;
	s32 err;
	uint8_t *encoded_wfek = NULL;
	uint8_t *fek = NULL;
	uint32_t encoded_len = 0;
	uint32_t fek_len = 0;
	fek_efek_t buffer_fek_efek = { &fek, &fek_len,
		&encoded_wfek, &encoded_len };
	u32 parent_flags = 0;

	/* no need to judge page because it can be null */
	err = check_params(parent, inode, dentry, fs_data);
	if (err != HWDPS_SUCCESS)
		return err;

	err = hwdps_get_parent_dps_flag(parent, &parent_flags, page);
	if (err != HWDPS_SUCCESS)
		return err;

	if (!S_ISREG(inode->i_mode))
		return hwdps_dir_inherit_flags(inode, fs_data, parent_flags);

	err = hwdps_create_fek(parent->i_crypt_info->ci_master_key, inode,
		dentry, &buffer_fek_efek, parent_flags);
	if (err == -HWDPS_ERR_NOT_SUPPORTED) {
		pr_info_once("hwdps ino %lu not protected\n", inode->i_ino);
		err = HWDPS_SUCCESS;
		goto free_hwdps;
	}
	if (err != HWDPS_SUCCESS) {
		pr_err("hwdps ino %lu create fek err %d\n", inode->i_ino, err);
		get_create_task_uid(&id);
		hiview_for_hwdps(HWDPS_HIVIEW_CREATE, err, &id);
		goto free_hwdps;
	}
	err = set_inode_attr(parent, inode, encoded_wfek, encoded_len, fs_data);
	if (err != 0) {
		pr_err("hwdps ino %lu setxattr err %d\n", inode->i_ino, err);
		goto free_hwdps;
	}
	err = f2fs_set_hwdps_enable_flags(inode, fs_data);
	if (err != 0)
		pr_err("hwdps ino %lu set hwdps enable flags err %d\n",
			inode->i_ino, err);
free_hwdps:
	kzfree(encoded_wfek);
	kzfree(fek);
	return err;
}

hwdps_result_t hwdps_update_fek(const u8 *desc, const buffer_t *encoded_wfek,
	secondary_buffer_t *fek, uid_t new_uid, uid_t old_uid)
{
	hwdps_result_t res;
	encrypt_id id;

	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.update_fek(desc, encoded_wfek, fek,
		new_uid, old_uid);
	up_read(&g_fs_callbacks_lock);

	id.uid = new_uid;
	id.task_uid = old_uid;
	if (res != 0) {
		pr_err("hwdps new_id %d update_fek err %d\n", new_uid, res);
		hiview_for_hwdps(HWDPS_HIVIEW_UPDATE, res, &id);
	}

	return res;
}

hwdps_result_t hwdps_get_fek_from_origin(const u8 *desc, struct inode *inode,
	secondary_buffer_t *encoded_wfek)
{
	hwdps_result_t res;
	encrypt_id id;

	down_read(&g_fs_callbacks_lock);
	res = g_fs_callbacks.encrypt_fek(desc, inode, encoded_wfek);
	up_read(&g_fs_callbacks_lock);
	if (res != 0) {
		pr_err("%s err %d\n", __func__, res);
		get_create_task_uid(&id);
		hiview_for_hwdps(HWDPS_HIVIEW_ENCRYPT, res, &id);
	}

	return res;
}

void hwdps_register_fs_callbacks(struct hwdps_fs_callbacks_t *callbacks)
{
	down_write(&g_fs_callbacks_lock);
	if (callbacks) {
		if (callbacks->create_fek)
			g_fs_callbacks.create_fek = callbacks->create_fek;
		if (callbacks->hwdps_has_access)
			g_fs_callbacks.hwdps_has_access =
				callbacks->hwdps_has_access;
		if (callbacks->get_fek)
			g_fs_callbacks.get_fek = callbacks->get_fek;
		if (callbacks->update_fek)
			g_fs_callbacks.update_fek = callbacks->update_fek;
		if (callbacks->update_xattr_efek)
			g_fs_callbacks.update_xattr_efek =
				callbacks->update_xattr_efek;
		if (callbacks->encrypt_fek)
			g_fs_callbacks.encrypt_fek = callbacks->encrypt_fek;
	}
	up_write(&g_fs_callbacks_lock);
}
EXPORT_SYMBOL(hwdps_register_fs_callbacks); //lint !e580

void hwdps_unregister_fs_callbacks(void)
{
	down_write(&g_fs_callbacks_lock);
	g_fs_callbacks.create_fek = default_create_fek;
	g_fs_callbacks.hwdps_has_access = default_hwdps_has_access;
	g_fs_callbacks.get_fek = default_get_fek;
	g_fs_callbacks.update_fek = default_update_fek;
	g_fs_callbacks.update_xattr_efek = default_update_xattr_efek;
	g_fs_callbacks.encrypt_fek = default_encrypt_fek;
	up_write(&g_fs_callbacks_lock);
}
EXPORT_SYMBOL(hwdps_unregister_fs_callbacks); //lint !e580

#endif
