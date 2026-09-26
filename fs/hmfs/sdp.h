/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Extend functions for per-f2fs sdp(sensitive data protection).
 * Create: 2017-11-10
 * History: 2020-10-10 add hwdps
 */

#ifndef __F2FS_SDP_H__
#define __F2FS_SDP_H__

#ifndef F2FS_FS_SDP_ENCRYPTION
#define F2FS_FS_SDP_ENCRYPTION 1
#endif

#if F2FS_FS_SDP_ENCRYPTION
/* sdp crypto policy for f2fs */
struct fscrypt_sdp_policy {
	__u8 version;
	__u8 sdpclass;
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

#define F2FS_IOC_SET_SDP_ENCRYPTION_POLICY     _IOW(F2FS_IOCTL_MAGIC, 80,\
	struct fscrypt_sdp_policy)
#define F2FS_IOC_GET_SDP_ENCRYPTION_POLICY     _IOR(F2FS_IOCTL_MAGIC, 81, \
	struct fscrypt_sdp_policy)
#define F2FS_IOC_GET_ENCRYPTION_POLICY_TYPE    _IOR(F2FS_IOCTL_MAGIC, 82,\
	struct fscrypt_policy_type)
#ifdef CONFIG_HWDPS
#define F2FS_IOC_SET_DPS_ENCRYPTION_POLICY     _IOW(F2FS_IOCTL_MAGIC, 83, \
	struct fscrypt_dps_policy)
#endif

struct f2fs_sdp_fscrypt_operations {
	int (*get_sdp_context)(struct inode *, void *, size_t, void *);
	int (*set_sdp_context)(struct inode *, const void *, size_t, void *);
	int (*update_sdp_context)(struct inode *, const void *, size_t, void *);
	int (*update_context)(struct inode *, const void *, size_t, void *);
	int (*get_sdp_encrypt_flags)(struct inode *, void *, u32 *);
	int (*set_sdp_encrypt_flags)(struct inode *, void *, u32 *);
};

/*
 * policy.c
 */
int hmfs_ioc_set_sdp_encryption_policy(struct file *filp, unsigned long arg);
int hmfs_ioc_get_sdp_encryption_policy(struct file *filp, unsigned long arg);
int hmfs_ioc_get_encryption_policy_type(struct file *filp, unsigned long arg);
#ifdef CONFIG_HWDPS
int hmfs_ioc_set_dps_encryption_policy(struct file *filp, unsigned long arg);
#endif
int hmfs_is_file_sdp_encrypted(struct inode *inode);
int hmfs_sdp_crypt_inherit(struct inode *parent, struct inode *child,
			   void *dpage, void *fs_data);

/*
 * keyinfo.c
 */
int hmfs_get_crypt_keyinfo(struct inode *inode, void *fs_data, int *flag);

/*
 * sdp_context.c
 */
extern const struct f2fs_sdp_fscrypt_operations hmfs_sdp_cryptops;
#endif
#endif

