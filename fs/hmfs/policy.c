
#include <keys/user-type.h>
#include <linux/printk.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/hmfs_fs.h>
#include <linux/fscrypt_common.h>
#include "hmfs.h"
#include "xattr.h"
#include "sdp_internal.h"

#ifdef CONFIG_HWDPS
#include <huawei_platform/hwdps/hwdps_fs_hooks.h>
#include <huawei_platform/hwdps/hwdps_limits.h>
#endif
#if F2FS_FS_SDP_ENCRYPTION
static inline bool f2fs_inode_is_config_encryption(struct inode *inode)
{
	if (!inode->i_sb->s_cop->get_context)
		return false;

	return (inode->i_sb->s_cop->get_context(inode, NULL, 0L) > 0);
}

int hmfs_inode_get_sdp_encrypt_flags(struct inode *inode, void *fs_data,
				     u32 *flag)
{
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);

	if (!sb->s_sdp_cop->get_sdp_encrypt_flags)
		return -EOPNOTSUPP;
	return sb->s_sdp_cop->get_sdp_encrypt_flags(inode, fs_data, flag);
}

int hmfs_inode_set_sdp_encryption_flags(struct inode *inode, void *fs_data,
					u32 sdp_enc_flag)
{
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	int res;
	u32 flags;

	if (sdp_enc_flag & (~0x0F))
		return -EINVAL;

	res = sb->s_sdp_cop->get_sdp_encrypt_flags(inode, fs_data, &flags);
	if (res)
		return res;

	flags |= sdp_enc_flag;

	return sb->s_sdp_cop->set_sdp_encrypt_flags(inode, fs_data, &flags);
}

static inline bool fscrypt_valid_enc_modes(u32 contents_mode,
					   u32 filenames_mode)
{
	if (contents_mode == FS_ENCRYPTION_MODE_AES_128_CBC &&
	    filenames_mode == FS_ENCRYPTION_MODE_AES_128_CTS)
		return true;

	if (contents_mode == FS_ENCRYPTION_MODE_AES_256_XTS &&
	    filenames_mode == FS_ENCRYPTION_MODE_AES_256_CTS)
		return true;

	return false;
}

static int hmfs_check_keyring_by_policy(unsigned char ver, struct inode *inode,
					const struct fscrypt_sdp_policy *policy)
{
	int res;
	u8 master_key_descriptor_tmp[FS_KEY_DESCRIPTOR_SIZE];

	if (ver > INLINE_CRYPTO_V2) {
		if (S_ISREG(inode->i_mode) && (inode->i_crypt_info) &&
			i_size_read(inode)) {
			pr_err("f2fs_sdp %s: inode(%lu) the file is not null in FBE3, can not set sdp.\n",
				__func__, inode->i_ino);
			return -EINVAL;
		}
	}
	memcpy(master_key_descriptor_tmp, policy->master_key_descriptor,
		FS_KEY_DESCRIPTOR_SIZE);
	res = hmfs_inode_check_sdp_keyring(master_key_descriptor_tmp, 0, ver);

	return res;
}

static int f2fs_create_sdp_encryption_context_from_policy(struct inode *inode,
					const struct fscrypt_sdp_policy *policy)
{
	int res = 0;
	struct f2fs_sdp_fscrypt_context sdp_ctx = { 0 };
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);

	if (!policy)
		return -EINVAL;

	if ((policy->sdpclass != FSCRYPT_SDP_ECE_CLASS)
			&& (policy->sdpclass != FSCRYPT_SDP_SECE_CLASS))
		return -EINVAL;

	if (!fscrypt_valid_enc_modes(policy->contents_encryption_mode,
				     policy->filenames_encryption_mode))
		return -EINVAL;

	if (policy->flags & ~FS_POLICY_FLAGS_VALID)
		return -EINVAL;

	res = hmfs_check_keyring_by_policy(sb->encryption_ver, inode, policy);
	if (res)
		return res;

	sdp_ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V2;
	memcpy(sdp_ctx.master_key_descriptor, policy->master_key_descriptor,
					FS_KEY_DESCRIPTOR_SIZE);
	sdp_ctx.contents_encryption_mode = policy->contents_encryption_mode;
	sdp_ctx.filenames_encryption_mode = policy->filenames_encryption_mode;
	sdp_ctx.flags = policy->flags;
	sdp_ctx.sdpclass = policy->sdpclass;
	sdp_ctx.version = policy->version;

	res = sb->s_sdp_cop->set_sdp_context(inode, &sdp_ctx, sizeof(sdp_ctx),
					     NULL);
	if (res) {
		pr_err("hmfs_sdp %s: inode(%lu) set sdp ctx failed res(%d)\n",
				__func__, inode->i_ino, res);
		return res;
	}

	if (policy->sdpclass == FSCRYPT_SDP_ECE_CLASS)
		res = hmfs_inode_set_sdp_encryption_flags(inode, NULL,
					HMFS_XATTR_SDP_ECE_CONFIG_FLAG);
	else
		res = hmfs_inode_set_sdp_encryption_flags(inode, NULL,
					HMFS_XATTR_SDP_SECE_CONFIG_FLAG);
	if (res)
		pr_err("hmfs_sdp %s: inode(%lu) set sdp config flags failed res(%d)\n",
				__func__, inode->i_ino, res);

	if (S_ISREG(inode->i_mode) && !res && (inode->i_crypt_info))
		res = hmfs_change_to_sdp_crypto(inode, NULL);

	return res;
}

static bool f2fs_is_sdp_context_consistent_with_policy(struct inode *inode,
					const struct fscrypt_sdp_policy *policy)
{
	int res = 0;
	struct f2fs_sdp_fscrypt_context ctx = { 0x0 };
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);

	if (!sb->s_sdp_cop->get_sdp_context)
		return false;

	res = sb->s_sdp_cop->get_sdp_context(inode, &ctx, sizeof(ctx), NULL);
	if (res != sizeof(ctx))
		return false;

	return (memcmp(ctx.master_key_descriptor, policy->master_key_descriptor,
			FS_KEY_DESCRIPTOR_SIZE) == 0 &&
			(ctx.sdpclass == policy->sdpclass) &&
			(ctx.flags == policy->flags) &&
			(ctx.contents_encryption_mode ==
			 policy->contents_encryption_mode) &&
			(ctx.filenames_encryption_mode ==
			 policy->filenames_encryption_mode));
}

static int f2fs_fscrypt_ioctl_set_sdp_policy(struct file *filp,
					     const void __user *arg)
{
	int ret = 0;
	struct fscrypt_sdp_policy policy = { 0x0 };
	struct inode *inode = file_inode(filp);
	u32 flag = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (copy_from_user(&policy, arg, sizeof(policy)))
		return -EFAULT;

	if (policy.version != 0)
		return -EINVAL;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);
	down_write(&F2FS_I(inode)->i_sdp_sem);

	ret = hmfs_inode_get_sdp_encrypt_flags(inode, NULL, &flag);
	if (ret)
		goto err;
	if (!f2fs_inode_is_config_encryption(inode)) {
		ret = -EINVAL;
	} else if (!HMFS_INODE_IS_CONFIG_SDP_ENCRYPTION(flag)) {
		ret = f2fs_create_sdp_encryption_context_from_policy(inode,
								&policy);
	} else if (!f2fs_is_sdp_context_consistent_with_policy(inode,
								&policy)) {
		pr_warn("f2fs_sdp %s: Policy inconsistent with sdp context\n",
				__func__);
		ret = -EINVAL;
	}

err:
	up_write(&F2FS_I(inode)->i_sdp_sem);
	inode_unlock(inode);

	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_fscrypt_ioctl_get_sdp_policy(struct file *filp,
					     void __user *arg)
{
	int res = 0;
	struct f2fs_sdp_fscrypt_context ctx = { 0x0 };
	struct fscrypt_sdp_policy policy = { 0x0 };
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);

	if (!test_hw_opt(sb, SDP_ENCRYPT))
		return -EOPNOTSUPP;

	if (!sb->s_sdp_cop->get_sdp_context)
		return -ENODATA;

	res = sb->s_sdp_cop->get_sdp_context(inode, &ctx, sizeof(ctx), NULL);
	if (res != sizeof(ctx))
		return -ENODATA;

	if (ctx.format != FS_ENCRYPTION_CONTEXT_FORMAT_V2)
		return -EINVAL;

	policy.version = ctx.version;
	policy.sdpclass = ctx.sdpclass;
	policy.contents_encryption_mode = ctx.contents_encryption_mode;
	policy.filenames_encryption_mode = ctx.filenames_encryption_mode;
	policy.flags = ctx.flags;
	memcpy(policy.master_key_descriptor, ctx.master_key_descriptor,
				FS_KEY_DESCRIPTOR_SIZE);

	if (copy_to_user(arg, &policy, sizeof(policy)))
		return -EFAULT;

	return 0;
}

#ifdef CONFIG_HWDPS
static int check_inode_flag(struct inode *inode)
{
	int res;
	u32 flags = 0;

	if (!inode || !inode->i_sb || !inode->i_sb->s_cop ||
		!inode->i_sb->s_cop->get_hwdps_flags) {
		pr_err("%s get_sdp_encrypt_flags NULL\n", __func__);
		return -EFAULT;
	}

	res = inode->i_sb->s_cop->get_hwdps_flags(inode, NULL, &flags);
	if (res != 0) {
		pr_err("%s get_sdp_encrypt_flags err = %d\n", __func__, res);
		return -EFAULT;
	}

	if ((flags & HWDPS_ENABLE_FLAG) != 0) {
		pr_err("%s has flag, no need to set again\n", __func__);
		return -EEXIST;
	}

	if (F2FS_INODE_IS_SDP_ENCRYPTED(flags) != 0) {
		pr_err("%s has sdp flags, failed to set dps flags\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int set_dps_attr_and_flag(struct inode *inode)
{
	int ret;
	uint8_t *encoded_wfek = NULL;
	uint32_t encoded_len = HWDPS_ENCODED_WFEK_SIZE;
	secondary_buffer_t buffer_wfek = { &encoded_wfek, &encoded_len };

	if (S_ISREG(inode->i_mode)) {
		ret = hwdps_get_fek_from_origin(
			inode->i_crypt_info->ci_master_key,
			inode, &buffer_wfek);
		if (ret != 0) {
			pr_err("hmfs_dps hwdps_get_fek_from_origin err %d\n",
				ret);
			goto err;
		}
	} else {
		encoded_wfek = kzalloc(HWDPS_ENCODED_WFEK_SIZE, GFP_NOFS);
		if (!encoded_wfek) {
			pr_err("hmfs_dps kzalloc err\n");
			ret = -ENOMEM;
			goto err;
		}
	}
	ret = inode->i_sb->s_cop->set_hwdps_attr(inode, encoded_wfek,
		encoded_len, NULL);
	if (ret != 0) {
		pr_err("hmfs_dps set_hwdps_attr err\n");
		goto err;
	}

	ret = f2fs_set_hwdps_enable_flags(inode, NULL);
	if (ret != 0) {
		pr_err("hmfs_dps f2fs_set_hwdps_enable_flags err %d\n", ret);
		goto err;
	}
err:
	kzfree(encoded_wfek);
	return ret;
}

static int set_dps_policy_inner(struct file *filp, struct inode *inode)
{
	int ret;

	ret = mnt_want_write_file(filp);
	if (ret != 0) {
		pr_err("hmfs_dps mnt_want_write_file erris %d\n", ret);
		return ret;
	}

	inode_lock(inode);
	ret = check_inode_flag(inode);
	if (ret == -EEXIST) {
		ret = 0;
		goto err;
	}
	if (ret != 0)
		goto err;

	ret = set_dps_attr_and_flag(inode);
	if (ret != 0)
		goto err;
	/* just set hw_enc_flag for regular file */
	if (S_ISREG(inode->i_mode) && inode->i_crypt_info)
		inode->i_crypt_info->ci_hw_enc_flag |=
			HWDPS_XATTR_ENABLE_FLAG_NEW;
err:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_fscrypt_ioctl_set_dps_policy(struct file *filp,
		const void __user *arg)
{
	struct fscrypt_dps_policy policy = {0};
	struct inode *inode = file_inode(filp);

	if (!inode_owner_or_capable(inode)) {
		pr_err("%s inode_owner_or_capable\n", __func__);
		return -EACCES;
	}

	if (copy_from_user(&policy, arg, sizeof(policy))) {
		pr_err("%s copy_from_user\n", __func__);
		return -EFAULT;
	}

	if (policy.version != 0) {
		pr_err("%s policy.version : %u\n", __func__, policy.version);
		return -EINVAL;
	}

	return set_dps_policy_inner(filp, inode);
}

int hmfs_ioc_set_dps_encryption_policy(struct file *filp, unsigned long arg)
{
	struct inode *inode = NULL;

	if (!filp)
		return -EINVAL;
	inode = file_inode(filp);

	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	return f2fs_fscrypt_ioctl_set_dps_policy(filp,
		(const void __user *)arg);
}
#endif

static int f2fs_fscrypt_ioctl_get_policy_type(struct file *filp,
					      void __user *arg)
{
	int res = 0;
	u32 flags = 0;
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sb = F2FS_I_SB(inode);
	struct fscrypt_info *ci = inode->i_crypt_info;
	struct fscrypt_policy_type policy = { 0 };

	if (!test_hw_opt(sb, SDP_ENCRYPT))
		return -EOPNOTSUPP;

	if (!ci)
		goto out;

	policy.contents_encryption_mode = ci->ci_data_mode;
	policy.filenames_encryption_mode = ci->ci_filename_mode;
	policy.version = 0;
	policy.encryption_type = FSCRYPT_CE_CLASS;
	memcpy(policy.master_key_descriptor, ci->ci_master_key,
		FS_KEY_DESCRIPTOR_SIZE);

	if (!sb->s_sdp_cop->get_sdp_encrypt_flags)
		goto out;

	res = sb->s_sdp_cop->get_sdp_encrypt_flags(inode, NULL, &flags);
	if (res)
		goto out;

	if (flags & F2FS_XATTR_SDP_ECE_ENABLE_FLAG)
		policy.encryption_type = FSCRYPT_SDP_ECE_CLASS;
	else if (flags & F2FS_XATTR_SDP_SECE_ENABLE_FLAG)
		policy.encryption_type = FSCRYPT_SDP_SECE_CLASS;
#ifdef CONFIG_HWDPS
	else if (flags & HWDPS_ENABLE_FLAG)
		policy.encryption_type = FSCRYPT_DPS_CLASS;
#endif
	else
		goto out;
out:
	if (copy_to_user(arg, &policy, sizeof(policy)))
		return -EFAULT;

	return 0;
}

int hmfs_ioc_set_sdp_encryption_policy(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (!test_hw_opt(sbi, SDP_ENCRYPT))
		return -EOPNOTSUPP;

	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);

	return f2fs_fscrypt_ioctl_set_sdp_policy(filp,
		(const void __user *)arg);
}

int hmfs_ioc_get_sdp_encryption_policy(struct file *filp, unsigned long arg)
{
	return f2fs_fscrypt_ioctl_get_sdp_policy(filp, (void __user *)arg);
}

int hmfs_ioc_get_encryption_policy_type(struct file *filp, unsigned long arg)
{
	return f2fs_fscrypt_ioctl_get_policy_type(filp, (void __user *)arg);
}

int hmfs_inode_check_sdp_keyring(u8 *descriptor, int enforce, unsigned char ver)
{
	int res = -ENOKEY;
	struct key *keyring_key = NULL;
	const struct user_key_payload *ukp = NULL;
	struct fscrypt_sdp_key *master_sdp_key = NULL;

	keyring_key = fscrypt_request_key(descriptor,
			FS_KEY_DESC_PREFIX, FS_KEY_DESC_PREFIX_SIZE);
	if (IS_ERR(keyring_key))
		return PTR_ERR(keyring_key);

	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon) {
		pr_err("hmfs_sdp %s: key type must be logon\n", __func__);
		goto out;
	}

	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		/* key was revoked before we acquired its semaphore */
		res = -EKEYREVOKED;
		goto out;
	}
	if (ukp->datalen != sizeof(struct fscrypt_sdp_key)) {
		pr_err("hmfs_sdp %s: sdp full key size incorrect: %d\n",
				__func__, ukp->datalen);
		goto out;
	}

	master_sdp_key = (struct fscrypt_sdp_key *)ukp->data;
	if (master_sdp_key->sdpclass != FSCRYPT_SDP_SECE_CLASS &&
		master_sdp_key->sdpclass != FSCRYPT_SDP_ECE_CLASS)
		goto out;

	if (ver > INLINE_CRYPTO_V2) {
		res = 0;
		goto out;
	}

	if (master_sdp_key->sdpclass == FSCRYPT_SDP_SECE_CLASS) {
		if (enforce == 1) {
			u8 sdp_pri_key[FS_MAX_KEY_SIZE] = { 0 };

			if (master_sdp_key->size == 0
					|| memcmp(master_sdp_key->raw, sdp_pri_key,
						master_sdp_key->size) == 0)
				goto out;
		}
	}

	res = 0;
out:
	up_read(&keyring_key->sem);
	key_put(keyring_key);
	return res;
}

int hmfs_sdp_crypt_inherit(struct inode *parent, struct inode *child,
			   void *dpage, void *fs_data)
{
	int res = 0;
	struct f2fs_sb_info *sb = F2FS_I_SB(parent);
	struct f2fs_sdp_fscrypt_context sdp_ctx;
	char *sdp_mkey_desc = NULL;

	if (!test_hw_opt(sb, SDP_ENCRYPT))
		return 0;

	res = sb->s_sdp_cop->get_sdp_context(parent, &sdp_ctx, sizeof(sdp_ctx),
					     dpage);
	if (res != sizeof(sdp_ctx))
		return 0;

	if (S_ISREG(child->i_mode)) {
		sdp_mkey_desc = sdp_ctx.master_key_descriptor;
		res = hmfs_inode_check_sdp_keyring(sdp_mkey_desc, 0,
							sb->encryption_ver);
		if (res)
			return res;
	}

	down_write(&F2FS_I(child)->i_sdp_sem);
	res = sb->s_sdp_cop->set_sdp_context(child, &sdp_ctx, sizeof(sdp_ctx),
					     fs_data);
	if (res)
		goto out;

	if (sdp_ctx.sdpclass == FSCRYPT_SDP_ECE_CLASS)
		res = hmfs_inode_set_sdp_encryption_flags(child, fs_data,
					HMFS_XATTR_SDP_ECE_CONFIG_FLAG);
	else if (sdp_ctx.sdpclass == FSCRYPT_SDP_SECE_CLASS)
		/* SECE can not be enable at the first time */
		res = hmfs_inode_set_sdp_encryption_flags(child, fs_data,
					HMFS_XATTR_SDP_SECE_CONFIG_FLAG);
	else
		res = -EOPNOTSUPP;
	if (res)
		res = -EINVAL;
out:
	up_write(&F2FS_I(child)->i_sdp_sem);
	return res;
}
#endif

