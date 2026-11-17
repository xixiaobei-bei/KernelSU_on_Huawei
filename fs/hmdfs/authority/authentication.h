/* SPDX-License-Identifier: GPL-2.0 */
/*
 * authentication.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: chenyi77@huawei.com
 * Create: 2020-06-22
 *
 */

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/sched/task.h>
#include <linux/xattr.h>
#include "hmdfs.h"

struct cache_fs_override {
	struct fs_struct *saved_fs;
	struct fs_struct *copied_fs;
	const struct cred *saved_cred;
};

#ifdef CONFIG_HMDFS_ANDROID

#define AID_ROOT             0
#define AID_SYSTEM        1000
#define AID_SDCARD_RW     1015
#define AID_MEDIA_RW      1023
#define AID_EVERYBODY     9997

/* copied from sdcardfs/multiuser.h */
#define AID_USER_OFFSET     100000 /* offset for uid ranges for each user */

#define HMDFS_PERM_XATTR "user.hmdfs.perm"

#define ROOT_UID KUIDT_INIT(AID_ROOT)
#define SYSTEM_UID KUIDT_INIT(AID_SYSTEM)
#define MEDIA_RW_UID KUIDT_INIT(AID_MEDIA_RW)

#define SYSTEM_GID KGIDT_INIT((gid_t) AID_SYSTEM)
#define MEDIA_RW_GID KGIDT_INIT(AID_MEDIA_RW)
#define SDCARD_RW_GID KGIDT_INIT(AID_SDCARD_RW)

#define PKG_ROOT_NAME "data"
#define SYSTEM_NAME "system"

/*
 * |           perm fix               | permmnt | permdfs   |  permpkg    | perm other
 * /mnt/mdfs/ accoundID / device view /  local  / DATA      / packageName /...
 *                                              / system    /...
 *                                              / documents /...
 *                                    /  devid  /.......
 *                      /    merge view         /
 *          / sdcard    /
 **/
#define HMDFS_PERM_MASK     0x000F
typedef enum {
	HMDFS_PERM_FIX = 0,
	HMDFS_PERM_MNT = 1,  // accoundid sdcard, by-devices, local, devid, merge
	HMDFS_PERM_DFS = 2,  // data, system, audios, documents
	HMDFS_PERM_PKG = 3,  // package name
	HMDFS_PERM_OTHER = 4,
	HMDFS_PERM_MAX = 0xF,
} hmdfs_perm_t;

static inline bool is_perm_fix(__u16 perm)
{
	return (perm & HMDFS_PERM_MASK) == HMDFS_PERM_FIX;
}

static inline bool is_perm_mnt(__u16 perm)
{
	return (perm & HMDFS_PERM_MASK) == HMDFS_PERM_MNT;
}

static inline bool is_perm_dfs(__u16 perm)
{
	return (perm & HMDFS_PERM_MASK) == HMDFS_PERM_DFS;
}

static inline bool is_perm_pkg(__u16 perm)
{
	return (perm & HMDFS_PERM_MASK) == HMDFS_PERM_PKG;
}

static inline bool is_perm_other(__u16 perm)
{
	return (perm & HMDFS_PERM_MASK) == HMDFS_PERM_OTHER;
}

static inline void hmdfs_check_cred(const struct cred *cred)
{
	if (cred->fsuid.val != AID_SYSTEM || cred->fsgid.val != AID_SYSTEM)
		hmdfs_warning("uid is %u, gid is %u", cred->fsuid.val,
			      cred->fsgid.val);
}

/* dir and file type mask for hmdfs */
#define HMDFS_DIR_TYPE_MASK 0x00F0

/* LEVEL 0  perm fix - permmnt , only root dir */
#define HMDFS_DIR_ROOT      0x0010

/* LEVEL 1 perm dfs */
#define HMDFS_DIR_PUBLIC    0x0020
#define HMDFS_DIR_DATA      0x0030
#define HMDFS_DIR_SYSTEM    0x0040

/* LEVEL 2 HMDFS_PERM_PKG */
#define HMDFS_DIR_PKG 0x0050

/* LEVEL 2~n HMDFS_PERM_OTHER */
#define PUBLIC_FILE 0x0060
#define PUBLIC_SUB_DIR 0x0070
#define SYSTEM_SUB_DIR 0x0080
#define SYSTEM_SUB_FILE 0x0090

#define HMDFS_DIR_PKG_SUB 0x00A0
#define HMDFS_FILE_PKG_SUB 0x00B0

/* access right is derived
 * PUBLIC_SUB_DIR SYSTEM_SUB_DIR HMDFS_DIR_PKG_SUB
 * PUBLIC_FILE SYSTEM_SUB_FILE HMDFS_FILE_PKG_SUB
 */
#define HMDFS_DIR_DEFAULT 0x00C0
#define HMDFS_FILE_DEFAULT 0x00D0
#define HMDFS_TYPE_DEFAULT 0x0000

static inline bool is_data_dir(__u16 perm)
{
	return (perm & HMDFS_DIR_TYPE_MASK) == HMDFS_DIR_DATA;
}

static inline bool is_pkg_dir(__u16 perm)
{
	return (perm & HMDFS_DIR_TYPE_MASK) == HMDFS_DIR_PKG;
}

static inline bool is_pkg_sub_dir(__u16 perm)
{
	return (perm & HMDFS_DIR_TYPE_MASK) == HMDFS_DIR_PKG_SUB;
}

static inline bool is_pkg_sub_file(__u16 perm)
{
	return (perm & HMDFS_DIR_TYPE_MASK) == HMDFS_FILE_PKG_SUB;
}

static inline bool is_default_dir(__u16 perm)
{
	return (perm & HMDFS_DIR_TYPE_MASK) == HMDFS_DIR_DEFAULT;
}

static inline bool is_default_file(__u16 perm)
{
	return (perm & HMDFS_DIR_TYPE_MASK) == HMDFS_FILE_DEFAULT;
}

#define AUTH_MASK 0x0F00
#define AUTH_PKG 0x0100
#define AUTH_SYSTEM 0x0200

static inline bool is_pkg_auth(__u16 perm)
{
	return (perm & AUTH_MASK) == AUTH_PKG;
}

static inline bool is_system_auth(__u16 perm)
{
	return (perm & AUTH_MASK) == AUTH_SYSTEM;
}

#define HMDFS_MOUNT_POINT_MASK 0xF000
#define HMDFS_MNT_COMMON 0x0000  // sdcard
#define HMDFS_MNT_SDCARD 0x1000  // sdcard
#define HMDFS_MNT_ACNTID 0x2000  // accound id

#define HMDFS_ALL_MASK (HMDFS_MOUNT_POINT_MASK | AUTH_MASK | HMDFS_DIR_TYPE_MASK | HMDFS_PERM_MASK)

static inline kuid_t get_appid_from_uid(kuid_t uid)
{
	kuid_t appid;

	appid.val = uid.val % AID_USER_OFFSET;
	return appid;
}

static inline void set_inode_gid(struct inode *inode, kgid_t gid)
{
	inode->i_gid = gid;
}

static inline kuid_t get_inode_uid(struct inode *inode)
{
	kuid_t uid = inode->i_uid;
	return uid;
}

static inline void set_inode_uid(struct inode *inode, kuid_t uid)
{
	inode->i_uid = uid;
}

static inline kuid_t hmdfs_override_inode_uid(struct inode *inode)
{
	kuid_t uid = get_inode_uid(inode);
	set_inode_uid(inode, current_fsuid());
	return uid;
}

static inline void hmdfs_revert_inode_uid(struct inode *inode, kuid_t uid)
{
	set_inode_uid(inode, uid);
}

static inline const struct cred *hmdfs_override_creds(const struct cred *new)
{
	if (!new)
		return NULL;

	return override_creds(new);
}

static inline void hmdfs_revert_creds(const struct cred *old)
{
	if (old)
		revert_creds(old);
}

static inline __u16 hmdfs_perm_get_next_level(__u16 perm)
{
	__u16 level = (perm & HMDFS_PERM_MASK) + 1;

	if (level <= HMDFS_PERM_OTHER)
		return level;
	else
		return HMDFS_PERM_OTHER;
}

struct fs_struct *hmdfs_override_fsstruct(struct fs_struct *saved_fs);
void hmdfs_revert_fsstruct(struct fs_struct *saved_fs,
			   struct fs_struct *copied_fs);
const struct cred *hmdfs_override_fsids(bool is_recv_thread);
const struct cred *hmdfs_override_dir_fsids(struct inode *dir,
					    struct dentry *dentry, __u16 *perm);
const struct cred *hmdfs_override_file_fsids(struct inode *dir, __u16 *perm);
void hmdfs_revert_fsids(const struct cred *old_cred);
int hmdfs_persist_perm(struct dentry *dentry, __u16 *perm);
__u16 hmdfs_read_perm(struct inode *inode);
void hmdfs_root_inode_perm_init(struct inode *root_inode);
void check_and_fixup_ownership(struct inode *parent_inode, struct inode *child,
			       struct dentry *lower_dentry, const char *name);
int hmdfs_override_dir_id_fs(struct cache_fs_override *or,
			struct inode *dir,
			struct dentry *dentry,
			__u16 *perm);
void hmdfs_revert_dir_id_fs(struct cache_fs_override *or);
void check_and_fixup_ownership_remote(struct inode *dir,
				      struct dentry *dentry);

/* for packagelist.c */
extern int hmdfs_packagelist_init(void);
extern void hmdfs_packagelist_exit(void);
extern uid_t hmdfs_get_appid(const char *key);

#else

static inline
void hmdfs_root_inode_perm_init(struct inode *root_inode)
{
}

static inline
void hmdfs_revert_fsids(const struct cred *old_cred)
{
}

static inline
int hmdfs_override_dir_id_fs(struct cache_fs_override *or,
			struct inode *dir,
			struct dentry *dentry,
			__u16 *perm)
{
	return 0;
}

static inline
void hmdfs_revert_dir_id_fs(struct cache_fs_override *or)
{
}

static inline
void check_and_fixup_ownership(struct inode *parent_inode, struct inode *child,
			       struct dentry *lower_dentry, const char *name)
{
}

static inline
const struct cred *hmdfs_override_fsids(bool is_recv_thread)
{
	return ERR_PTR(-ENOTTY);
}

static inline
const struct cred *hmdfs_override_creds(const struct cred *new)
{
	return ERR_PTR(-ENOTTY);
}

static inline
void hmdfs_revert_creds(const struct cred *old)
{

}

static inline
void check_and_fixup_ownership_remote(struct inode *dir,
				      struct dentry *dentry)
{
}

static inline
kuid_t hmdfs_override_inode_uid(struct inode *inode)
{
	return KUIDT_INIT((uid_t)0);
}

static inline
void hmdfs_revert_inode_uid(struct inode *inode, kuid_t uid)
{
}

static inline
void hmdfs_check_cred(const struct cred *cred)
{
}

#endif /* CONFIG_HMDFS_ANDROID */

#endif
