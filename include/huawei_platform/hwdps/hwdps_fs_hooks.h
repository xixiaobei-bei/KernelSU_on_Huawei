/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file define function about fs hooks.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_FS_HOOKS_H
#define _HWDPS_FS_HOOKS_H

#include <linux/fs.h>
#include <linux/types.h>
#include <huawei_platform/hwdps/hwdps_defines.h>
#include <huawei_platform/hwdps/hwdps_error.h>
#include <huawei_platform/hwdps/hwdps_ioctl.h>

#ifdef CONFIG_HWDPS

/* filesystem callback functions */
struct hwdps_fs_callbacks_t {
	hwdps_result_t (*create_fek)(const u8 *desc, const u8 *fsname,
		const struct dentry *dentry, fek_efek_t *fek_efek,
		u32 flags);

	hwdps_result_t (*hwdps_has_access)(const encrypt_id *id,
		const buffer_t *encoded_wfek, s32 flags);

	hwdps_result_t (*get_fek)(const u8 *desc, const encrypt_id *id,
		const buffer_t *encoded_wfek, secondary_buffer_t *fek,
		u32 flags);

	hwdps_result_t (*update_fek)(const u8 *desc,
		const buffer_t *encoded_wfek, secondary_buffer_t *fek,
		uid_t new_uid, uid_t old_uid);

	hwdps_result_t (*update_xattr_efek)(const u8 *desc, uid_t uid,
		secondary_buffer_t *fek, struct inode *inode);

	hwdps_result_t (*encrypt_fek)(const u8 *desc, struct inode *inode,
		secondary_buffer_t *encoded_wfek);
};

/*
 * Description: Access a file encryption key. Note: memory for the output
 *              parameters encoded_wfek and inode will be check inside the
 *              function.
 * Input: inode: the inode requesting encoded_wfek and fek
 * Input: encoded_wfek: wrapped (i.e., encrypted) file encryption key with aad
 * Return: HWDPS_SUCCESS: successfully protected by hwdps
 *         -HWDPS_ERR_NOT_SUPPORTED:  not protected by hwdps
 *         -HWDPS_ERR_NO_FS_CALLBACKS: should wait
 *         other negative value: error
 */
hwdps_result_t hwdps_has_access(struct inode *inode,
	const buffer_t *encoded_wfek, s32 flags);

/*
 * Description: Decrypts a file encryption key. Note: memory for the output
 *              parameter fek will be allocated inside the function and it
 *              is the responsibility of the caller to release this memory.
 *              Furthermore, the contents of the memory pointed to
 *              by the output parameter fek_len will be overwritten.
 * Input: desc: the inode key description
 * Input: inode: the inode requesting fek
 * Input: encoded_wfek: the wrapped file encryption key with aad
 * Output: fek: raw file encryption key and len
 * Return: HWDPS_SUCCESS: successfully get raw fek
 *         -HWDPS_ERR_NOT_SUPPORTED:  not protected by hwdps
 *         -HWDPS_ERR_NO_FS_CALLBACKS: should wait
 *         other negative value: error
 */
hwdps_result_t hwdps_get_fek(const u8 *desc, struct inode *inode,
	const buffer_t *encoded_wfek, secondary_buffer_t *fek, u32 flags);

/*
 * Description: Loads filesystem callback functions.
 * Input: struct hwdps_fs_callbacks_t *callbacks: callback functions to be
 *        registered
 */
void hwdps_register_fs_callbacks(struct hwdps_fs_callbacks_t *callbacks);

/*
 * Description: Unloads filesystem callback functions.
 */
void hwdps_unregister_fs_callbacks(void);

/*
 * Description: Check the context of a file inode whether support the hwdps.
 * Input: inode: the inode requesting fek struct
 * Return: 0: successfully support hwdps
 *         -EOPNOTSUPP: not support hwdps
 */
s32 hwdps_check_support(struct inode *inode, uint32_t *flags);

/*
 * Description: Get the attribute of a file inode whether support the hwdps.
 * Input: inode: the inode requesting fek struct
 * Input: size: the size of the attribute len
 * Return: fek: the attribute key of file saved in inode
 *         -NULL: error
 */
uint8_t *hwdps_do_get_attr(struct inode *inode, size_t size, u32 flags);

#ifdef CONFIG_HWDPS
/*
 * Description: Inherit the context from parent inode context.
 * Input: parent: the parent inode struct
 * Input: inode: the inode requesting fek struct
 * Input: dentry: the dentry of this inode
 * Input: fs_data: the page data buffer
 * Return: 0: successfully inherit the context
 *         -ENOKEY: no file key
 */
s32 hwdps_inherit_context(struct inode *parent, struct inode *inode,
	const struct dentry *dentry, void *fs_data, struct page *dpage);
#endif

/*
 * Description: Update file encrypt key when fix uid.
 * Input: desc: primary key descriptor from context
 * Input: encoded_wfek: encoded wfek buffer
 * Input: fek: second buffer for fek
 * Input: new_uid: the new user id
 * Input: old_uid: the old user id from inode->i_ino
 * Return: HWDPS_SUCCESS: successfully update fek
 *         -HWDPS_ERR_NOT_SUPPORTED: not protected by hwdps
 *         -HWDPS_ERR_NO_FS_CALLBACKS: should wait
 *         other negative value: error
 */
hwdps_result_t hwdps_update_fek(const u8 *desc, const buffer_t *encoded_wfek,
	secondary_buffer_t *fek, uid_t new_uid, uid_t old_uid);

s32 f2fs_set_hwdps_enable_flags(struct inode *inode, void *fs_data);

hwdps_result_t hwdps_get_fek_from_origin(const u8 *desc, struct inode *inode,
	secondary_buffer_t *encoded_wfek);

#endif /* CONFIG_HWDPS */
#endif
