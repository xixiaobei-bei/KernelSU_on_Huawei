
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

#define F2FS_IOC_SET_SDP_ENCRYPTION_POLICY     _IOW(F2FS_IOCTL_MAGIC, 12,\
	struct fscrypt_sdp_policy)
#define F2FS_IOC_GET_SDP_ENCRYPTION_POLICY     _IOR(F2FS_IOCTL_MAGIC, 13, \
	struct fscrypt_sdp_policy)
#define F2FS_IOC_GET_ENCRYPTION_POLICY_TYPE    _IOR(F2FS_IOCTL_MAGIC, 14,\
	struct fscrypt_policy_type)

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
int f2fs_ioc_set_sdp_encryption_policy(struct file *filp, unsigned long arg);
int f2fs_ioc_get_sdp_encryption_policy(struct file *filp, unsigned long arg);
int f2fs_ioc_get_encryption_policy_type(struct file *filp, unsigned long arg);
int f2fs_is_file_sdp_encrypted(struct inode *inode);
int f2fs_sdp_crypt_inherit(struct inode *parent, struct inode *child,
			   void *dpage, void *fs_data);

/*
 * keyinfo.c
 */
int f2fs_get_crypt_keyinfo(struct inode *inode, void *fs_data, int *flag);

/*
 * sdp_context.c
 */
extern const struct f2fs_sdp_fscrypt_operations f2fs_sdp_cryptops;
#endif
#endif

