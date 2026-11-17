/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Extend functions for per-f2fs sdp(sensitive data protection).
 * Create: 2020.08.22
 */

#ifndef _F2FS_SDP_H
#define _F2FS_SDP_H

#ifdef CONFIG_FS_SDP_ENCRYPTION
#define F2FS_FS_SDP_ENCRYPTION 1
#endif

#if defined(F2FS_FS_SDP_ENCRYPTION) || defined(CONFIG_HWDPS)
int f2fs_get_crypt_keyinfo(struct inode *inode, void *fs_data, int *flag);
#endif

#ifdef F2FS_FS_SDP_ENCRYPTION
/* sdp crypto policy for f2fs */
struct fscrypt_sdp_policy {
	__u8 version;
	__u8 sdp_class;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
} __packed;

/* crypto policy type for f2fs */
struct fscrypt_policy_type {
	__u8 version;
	__u8 encryption_type;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
} __packed;

#ifdef CONFIG_HWDPS

static const int RESERVED_DATA_LEN = 8;

/* dps crypto policy for f2fs */
struct fscrypt_dps_policy {
	__u8 version;
	__u8 reserved[RESERVED_DATA_LEN];
} __packed;
#endif

#define F2FS_IOC_SET_SDP_ENCRYPTION_POLICY	_IOW(F2FS_IOCTL_MAGIC, 80, \
	struct fscrypt_sdp_policy)
#define F2FS_IOC_GET_SDP_ENCRYPTION_POLICY	_IOR(F2FS_IOCTL_MAGIC, 81, \
	struct fscrypt_sdp_policy)
#define F2FS_IOC_GET_ENCRYPTION_POLICY_TYPE	_IOR(F2FS_IOCTL_MAGIC, 82, \
	struct fscrypt_policy_type)
#ifdef CONFIG_HWDPS
#define F2FS_IOC_SET_DPS_ENCRYPTION_POLICY	_IOW(F2FS_IOCTL_MAGIC, 83, \
	struct fscrypt_dps_policy)
#endif

/*
 * policy.c
 */
int f2fs_is_file_sdp_encrypted(struct inode *inode);
int sdp_crypt_inherit(struct inode *parent, struct inode *child,
	void *dpage, void *fs_data);

int f2fs_fscrypt_ioctl_set_sdp_policy(struct file *filp,
	const void __user *arg);
int f2fs_fscrypt_ioctl_get_sdp_policy(struct file *filp,
	void __user *arg);
int f2fs_fscrypt_ioctl_get_policy_type(struct file *filp,
	void __user *arg);
#ifdef CONFIG_HWDPS
int f2fs_fscrypt_ioctl_set_dps_policy(struct file *filp,
	const void __user *arg);
#endif

#endif
#endif
