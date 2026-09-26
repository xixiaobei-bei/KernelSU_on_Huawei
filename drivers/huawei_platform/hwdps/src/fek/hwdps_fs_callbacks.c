/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations
 *              about map.
 * Create: 2020-06-16
 */

#include "inc/fek/hwdps_fs_callbacks.h"
#include <linux/cred.h>
#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/printk.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <securec.h>
#include <huawei_platform/hwdps/hwdps_ioctl.h>
#include <huawei_platform/hwdps/hwdps_limits.h>
#include "inc/base/hwdps_defines.h"
#include "inc/base/hwdps_utils.h"
#include "inc/data/hwdps_data.h"
#include "inc/data/hwdps_packages.h"
#include "inc/fek/hwdps_fs_callbacks.h"
#include "inc/policy/hwdps_policy.h"
#include "inc/tee/hwdps_alg.h"
#include "inc/tee/hwdps_keyinfo.h"

#define VERSION_LENGTH 1
#define PHASE2_INDEX_LENGTH 32
#define AES_IV_LENGTH 16
#define FEK_LENGTH 64
#define SHA256_TAG_LENGTH 32
#define PERMS_LENGTH 8
#define APP_UID_LENGTH 4
#define PATH_NO_EXT_SDCARD 2
#define PATH_OF_INSTALLD "/system/bin/installd"
#define PATH_LEN_OF_INSTALLD 20

static hwdps_result_t get_path(const struct dentry *dentry, u8 *path)
{
	u8 *path_tmp = NULL;
	hwdps_result_t ret = HWDPS_SUCCESS;
	u8 *buf = kzalloc(PATH_MAX, GFP_NOFS);

	if (!buf)
		return HWDPS_ERR_GET_PATH;
	path_tmp = dentry_path_raw((struct dentry *)dentry, buf, PATH_MAX);
	if (IS_ERR(path_tmp) || (strlen(path_tmp) >= PATH_MAX))
		goto err;
	if (memcpy_s(path, PATH_MAX, path_tmp, strlen(path_tmp)) != EOK)
		goto err;
	goto out;
err:
	ret = HWDPS_ERR_GET_PATH;
out:
	kzfree(buf);
	return ret;
}

static hwdps_result_t precheck_new_key_data(const struct dentry *dentry,
	uid_t uid, const s8 *fsname)
{
	hwdps_result_t res;
	struct package_hashnode_t *package_node = NULL;
	struct package_info_listnode_t *package_info_node = NULL;
	struct package_info_listnode_t *tmp = NULL;
	u8 *path = NULL;

	hwdps_data_read_lock();
	package_node = get_hwdps_package_hashnode(uid);
	if (!package_node) {
		res = -HWDPS_ERR_NOT_SUPPORTED;
		goto out;
	}
	path = kzalloc(PATH_MAX, GFP_NOFS);
	if (!path) {
		res = -HWDPS_ERR_NO_MEMORY;
		hwdps_pr_err("%s path alloc failed\n", __func__);
		goto out;
	}

	res = get_path(dentry, path);
	if (res != HWDPS_SUCCESS)
		goto out;
	list_for_each_entry_safe(package_info_node, tmp,
		&package_node->pinfo_list, list)
		if (hwdps_evaluate_policies(package_info_node, fsname,
			path)) {
			res = HWDPS_SUCCESS; /* success case */
			goto out;
		}
	hwdps_pr_err("hwdps %s failed\n", __func__);
	res = -HWDPS_ERR_NOT_SUPPORTED; /* policy never matched */
out:
	kzfree(path);
	hwdps_data_read_unlock();
	return res;
}

static hwdps_result_t precheck_uid_owner(const encrypt_id *id)
{
	if (id->task_uid == id->uid)
		return HWDPS_SUCCESS;
	hwdps_pr_err("%s uid and task uid error\n", __func__);
	return -HWDPS_ERR_NOT_OWNER;
}

static bool is_create_fek_param_invalid(const u8 *desc, const u8 *fsname,
	const struct dentry *dentry,
	const fek_efek_t *fek_efek)
{
	return (!desc || !fsname || !dentry || !fek_efek || !fek_efek->fek ||
		!fek_efek->fek_len || !fek_efek->efek ||
		!fek_efek->efek_len);
}

static hwdps_result_t create_fek_inner(const u8 *desc, uid_t uid,
	secondary_buffer_t *fek, secondary_buffer_t *encoded_wfek)
{
	buffer_t fek_temp = { NULL, 0 };
	hwdps_result_t res = HWDPS_SUCCESS;

	hwdps_pr_debug("%s enter\n", __func__);
	*encoded_wfek->len = sizeof(struct xattribs_v3_t);
	fek_temp.data = kzalloc(sizeof(s8) * HWDPS_FEK_SIZE, GFP_NOFS);
	if (!fek_temp.data) {
		res = -HWDPS_ERR_NO_MEMORY;
		goto err;
	}
	fek_temp.len = HWDPS_FEK_SIZE;
	*encoded_wfek->data = kzalloc(sizeof(u8) * (*encoded_wfek->len),
		GFP_NOFS);
	if (!*(encoded_wfek->data)) {
		res = -HWDPS_ERR_NO_MEMORY;
		goto err;
	}

	if (kernel_new_fek(desc, uid, encoded_wfek, &fek_temp) !=
		ERR_MSG_SUCCESS) {
		hwdps_pr_err("kernel_new_fek err\n");
		res = -HWDPS_ERR_INTERNAL;
		goto err;
	}
	*fek->data = fek_temp.data;
	fek_temp.data = NULL;
	*fek->len = HWDPS_FEK_SIZE;
	*encoded_wfek->len = sizeof(struct xattribs_v3_t);
	goto done;
err:
	kzfree(*(encoded_wfek->data));
	*(encoded_wfek->data) = NULL;
done:
	kzfree(fek_temp.data);
	return res;
}

/* This function is called when a new inode is created. */
static hwdps_result_t handle_create_fek(const u8 *desc, const u8 *fsname,
	const struct dentry *dentry, fek_efek_t *fek_efek, u32 parent_flags)
{
	hwdps_result_t res;
	uid_t uid;
	secondary_buffer_t fek;
	secondary_buffer_t encoded_wfek;
	const struct cred *cred = NULL;

	if (is_create_fek_param_invalid(desc, fsname, dentry, fek_efek))
		return HWDPS_ERR_INVALID_ARGS;
	cred = get_current_cred();
	if (!cred)
		return HWDPS_ERR_INVALID_ARGS;
	uid = cred->uid.val; /* task uid */
	put_cred(cred);

	res = precheck_new_key_data(dentry, uid, fsname);
	if ((res != HWDPS_SUCCESS) &&
		((parent_flags & HWDPS_ENABLE_FLAG) == 0))
		return res;

	fek.data = fek_efek->fek;
	fek.len = fek_efek->fek_len;
	encoded_wfek.data = fek_efek->efek;
	encoded_wfek.len = fek_efek->efek_len;
	res = create_fek_inner(desc, uid, &fek, &encoded_wfek);
	if (res != HWDPS_SUCCESS)
		hwdps_pr_err("create_fek_inner returned %d\n", res);
	else
		hwdps_pr_debug("create_fek_inner returned success\n");
	return (res > 0) ? -res : res;
}

#ifdef CONFIG_HWDPS_ENG_DEBUG
static bool is_pid_high_level(pid_t pid)
{
	size_t priv_task_len;
	s32 priv_num;
	s8 task_name[TASK_COMM_LEN] = {0};

	/*
	 * lists cmds have high level
	 * echo --> "sh"
	 * adb pull --> "sync svc 44", "sync svc 66"
	 */
	static const s8 * const priv_cmds[] = {
		"cat", "sh", "sync svc"
	};
	get_task_comm(task_name, current);
	priv_num = ARRAY_SIZE(priv_cmds);
	for (s32 i = 0; i < priv_num; ++i) {
		priv_task_len = strlen(priv_cmds[i]);
		if (!strncmp(priv_cmds[i], task_name, priv_task_len))
			return true;
	}
	hwdps_pr_info("task is not high level\n");
	return false;
}
#endif

static bool is_pid_installd(pid_t pid)
{
	return hwdps_utils_exe_check(pid, PATH_OF_INSTALLD,
		PATH_LEN_OF_INSTALLD);
}

static hwdps_result_t handle_hwdps_has_access(const encrypt_id *id,
	const buffer_t *encoded_wfek, s32 flags)
{
	hwdps_result_t res;

	hwdps_pr_debug("%s enter\n", __func__);

	if (!id || !encoded_wfek || !encoded_wfek->data ||
		((flags == HWDPS_XATTR_ENABLE_FLAG_NEW) &&
		(encoded_wfek->len != HWDPS_ENCODED_WFEK_SIZE)) ||
		((flags == HWDPS_XATTR_ENABLE_FLAG) &&
		(encoded_wfek->len != HWDPS_ENCODE_WFEK_SIZE_OLD))) {
		hwdps_pr_err("%s params error\n", __func__);
		return -HWDPS_ERR_INVALID_ARGS;
	}
	res = precheck_uid_owner(id);
	if (res != HWDPS_SUCCESS) {
		hwdps_pr_err("precheck_uid_owner error\n");
		goto priv;
	}
	goto out;
priv:
#ifdef CONFIG_HWDPS_ENG_DEBUG
	if (is_pid_high_level(id->pid) || is_pid_installd(id->pid))
#else
	if (is_pid_installd(id->pid))
#endif
		res = HWDPS_SUCCESS;
out:
	return (res > 0) ? -res : res;
}

static void check_ret_values(hwdps_result_t *res_inout,
	const secondary_buffer_t *fek)
{
	if (*res_inout != 0 || !fek || (*fek->len != HWDPS_FEK_SIZE))
		if (*res_inout > 0) {
			*res_inout = -*res_inout;
			hwdps_pr_err("res %d fek len %u\n", *res_inout,
				*fek->len);
		}
}

static void get_app_data_from_encoded_wfek(const buffer_t *encoded_wfek,
	u64 *profile_id)
{
	u32 app_data_offset = VERSION_LENGTH + PHASE2_INDEX_LENGTH +
		AES_IV_LENGTH + FEK_LENGTH;

	/* the *app_data should be changed before use profile_id */
	u8 *app_data = encoded_wfek->data + app_data_offset;

	if (profile_id)
		*profile_id = *((u64 *)(app_data + APP_UID_LENGTH));
}

static hwdps_result_t get_fek_old(const encrypt_id *id,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek)
{
	hwdps_result_t res;
	u64 profile_id_from_encoded = 0;

	get_app_data_from_encoded_wfek(encoded_wfek, &profile_id_from_encoded);
	res = precheck_uid_owner(id);
	if (res != HWDPS_SUCCESS) {
		hwdps_pr_err("precheck_uid_owner res %d.\n", -res);
		goto priv;
	}
	res = kernel_get_fek_v1(hwdps_utils_get_ausn(id->uid),
		encoded_wfek, fek);
	check_ret_values(&res, fek);
	return res;
priv:
#ifdef CONFIG_HWDPS_ENG_DEBUG
	if ((!*(fek->data) || (*fek->len == 0)) &&
		(is_pid_high_level(id->pid) || is_pid_installd(id->pid)))
#else
	if ((!*(fek->data) || (*fek->len == 0)) &&
		is_pid_installd(id->pid))
#endif
		return kernel_get_fek_v1(profile_id_from_encoded,
			encoded_wfek, fek);
	return (res > 0) ? -res : res;
}

static hwdps_result_t get_fek_new(const u8 *desc, const encrypt_id *id,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek)
{
	hwdps_result_t res;

	res = precheck_uid_owner(id);
	if (res != HWDPS_SUCCESS) {
		hwdps_pr_err("precheck_uid_owner res %d\n", -res);
			goto priv;
	}

	res = kernel_get_fek_v2(desc, id->task_uid, encoded_wfek, fek);
	check_ret_values(&res, fek);
	return res;

priv:
#ifdef CONFIG_HWDPS_ENG_DEBUG
	if ((!*(fek->data) || (*fek->len == 0)) &&
		(is_pid_high_level(id->pid) || is_pid_installd(id->pid)))
#else
	if ((!*(fek->data) || (*fek->len == 0)) &&
		is_pid_installd(id->pid))
#endif
		return kernel_get_fek_v2(desc, id->uid, encoded_wfek,
			fek);
	return (res > 0) ? -res : res;
}

static bool handle_get_fek_check_param(const u8 *desc, const encrypt_id *id,
	const buffer_t *encoded_wfek, const secondary_buffer_t *fek)
{
	if (!id || !desc || !encoded_wfek || !encoded_wfek->data || !fek ||
		!fek->data || !fek->len) {
		hwdps_pr_err("%s bad params\n", __func__);
		return false;
	}
	return true;
}

static hwdps_result_t handle_get_fek(const u8 *desc, const encrypt_id *id,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek, u32 flags)
{
	hwdps_result_t res;

	hwdps_pr_debug("%s enter\n", __func__);
	if (!handle_get_fek_check_param(desc, id, encoded_wfek, fek))
		return HWDPS_ERR_INVALID_ARGS;
	if (flags == HWDPS_XATTR_ENABLE_FLAG) {
		res = get_fek_old(id, encoded_wfek, fek);
	} else if (flags == HWDPS_XATTR_ENABLE_FLAG_NEW) {
		res = get_fek_new(desc, id, encoded_wfek, fek);
	} else {
		hwdps_pr_err("%s invalid flags:%d\n", __func__, flags);
		res = -HWDPS_ERR_INTERNAL;
	}
	if (res != 0) {
		hwdps_pr_err("%s failed res:%d, flags:%d", __func__, res,
			flags);
		kzfree(*(fek->data));
		*(fek->data) = NULL;
		*(fek->len) = 0;
	}
	return (res > 0) ? -res : res;
}

static hwdps_result_t handle_update_fek(const u8 *desc,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek,
	uid_t new_uid, uid_t old_uid)
{
	hwdps_result_t res;

	hwdps_pr_debug("%s new uid %lld old uid %lld\n", __func__,
		new_uid, old_uid);
	res = kernel_update_fek(desc, encoded_wfek, fek, new_uid,
		old_uid);
	check_ret_values(&res, fek);
	if (res != HWDPS_SUCCESS)
		goto err;
	goto out;
err:
	kzfree(*(fek->data));
	*(fek->data) = NULL;
	*fek->len = 0;
out:
	return (res > 0) ? -res : res;
}

static hwdps_result_t set_xattr_efek(struct inode *inode,
	buffer_t encoded_cipher)
{
	hwdps_result_t res;

	if (!inode || !inode->i_sb || !inode->i_sb->s_cop)
		return ERR_MSG_BAD_PARAM;

	if (inode->i_sb->s_cop->set_hwdps_attr) {
		res = inode->i_sb->s_cop->set_hwdps_attr(inode,
			encoded_cipher.data, encoded_cipher.len, NULL);
	} else {
		pr_info("%s: ino %lu no setxattr\n",
			__func__, inode->i_ino);
		res = HWDPS_SUCCESS;
	}
	return res;
}

static hwdps_result_t handle_update_xattr_efek(const u8 *desc, uid_t uid,
	secondary_buffer_t *fek, struct inode *inode)
{
	hwdps_result_t res;
	u8 aes_key[AES256_KEY_LEN] = {0};
	buffer_t aes_key_buffer = { aes_key, AES256_KEY_LEN };
	u8 encoded_cipher[PHASE3_CIPHERTEXT_LENGTH] = {0};
	buffer_t encoded_cipher_buffer = {
		encoded_cipher, PHASE3_CIPHERTEXT_LENGTH
	};
	buffer_t fek_buff = { *fek->data, *fek->len };

	res = hwdps_get_key(desc, &aes_key_buffer, uid);
	if (res != 0) {
		hwdps_pr_err("%s: hwdps_get_key in get key failed %d\n",
			__func__, res);
		res = HWDPS_ERR_SET_XATTR;
		goto cleanup;
	}

	res = hwdps_refresh_enc(aes_key_buffer, encoded_cipher_buffer,
		fek_buff);
	if (res != 0) {
		hwdps_pr_err("%s: hwdps enc failed %d\n", __func__, res);
		res = HWDPS_ERR_SET_XATTR;
		goto cleanup;
	}

	res = set_xattr_efek(inode, encoded_cipher_buffer);
	if (res != 0) {
		hwdps_pr_err("%s: set_xattr_efek failed %d\n", __func__, res);
		res = HWDPS_ERR_SET_XATTR_EFEK;
	}
cleanup:
	(void)memset_s(aes_key_buffer.data, aes_key_buffer.len,
		0, aes_key_buffer.len);
	return (res > 0) ? -res : res;
}

static bool is_encrypt_fek_param_invalid(const u8 *desc,
	const struct inode *inode, const secondary_buffer_t *encoded_wfek)
{
	return (!desc ||
		!inode || !inode->i_crypt_info ||
		!inode->i_crypt_info->ci_key ||
		!encoded_wfek || !encoded_wfek->data || !encoded_wfek->len);
}

static hwdps_result_t encrypt_fek_inner(const u8 *desc, uid_t uid,
	struct inode *inode, secondary_buffer_t *encoded_wfek)
{
	hwdps_result_t res;
	buffer_t fek_temp = { NULL, 0 };

	hwdps_pr_debug("%s enter\n", __func__);
	*encoded_wfek->len = sizeof(struct xattribs_v3_t);
	fek_temp.data = inode->i_crypt_info->ci_key;
	fek_temp.len = HWDPS_FEK_SIZE;
	*encoded_wfek->data = kzalloc(sizeof(u8) * (*encoded_wfek->len),
		GFP_NOFS);
	if (!*(encoded_wfek->data)) {
		res = -HWDPS_ERR_NO_MEMORY;
		goto err;
	}

	res = kernel_encrypt_key(desc, uid, encoded_wfek, &fek_temp);
	if (res != ERR_MSG_SUCCESS) {
		hwdps_pr_err("%s err res:%d\n", __func__, res);
		res = -HWDPS_ERR_INTERNAL;
		goto err;
	}
	*encoded_wfek->len = sizeof(struct xattribs_v3_t);
	goto done;
err:
	kzfree(*(encoded_wfek->data));
	*(encoded_wfek->data) = NULL;
done:
	return res;
}

static hwdps_result_t handle_encrypt_fek(const u8 *desc,
	struct inode *inode,
	secondary_buffer_t *encoded_wfek)
{
	hwdps_result_t res;
	uid_t uid;
	const struct cred *cred = NULL;

	if (is_encrypt_fek_param_invalid(desc, inode, encoded_wfek)) {
		hwdps_pr_err("%s check param err\n", __func__);
		return HWDPS_ERR_INVALID_ARGS;
	}
	cred = get_current_cred();
	if (!cred) {
		hwdps_pr_err("%s get cred failed\n", __func__);
		return HWDPS_ERR_INTERNAL;
	}
	uid = cred->uid.val; /* task uid */
	put_cred(cred);
	if (uid != inode->i_uid.val)
		return HWDPS_ERR_UID_NOT_TRUST;

	res = encrypt_fek_inner(desc, uid, inode, encoded_wfek);
	if (res != HWDPS_SUCCESS)
		hwdps_pr_err("%s: encrypt_fek_inner returned %d\n",
			__func__, res);
	else
		hwdps_pr_debug("encrypt_fek_inner returned success\n");
	return (res > 0) ? -res : res;
}

struct hwdps_fs_callbacks_t g_fs_callbacks = {
	.create_fek = handle_create_fek,
	.hwdps_has_access = handle_hwdps_has_access,
	.get_fek = handle_get_fek,
	.update_fek = handle_update_fek,
	.update_xattr_efek = handle_update_xattr_efek,
	.encrypt_fek = handle_encrypt_fek,
};

void hwdps_register_fs_callbacks_proxy(void)
{
	hwdps_register_fs_callbacks(&g_fs_callbacks);
}

void hwdps_unregister_fs_callbacks_proxy(void)
{
	hwdps_unregister_fs_callbacks();
}
