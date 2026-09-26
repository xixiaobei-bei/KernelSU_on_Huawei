/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: filesystem cryption extra policy
 * Author: 	laixinyi, hebiao
 * Create: 	2020-12-16
 */
#include <linux/fs.h>
#include <linux/fscrypt_common.h>
#include <linux/random.h>
#include <linux/blkdev.h>
#include <linux/syscalls.h>
#include <linux/hisi/fbe_ctrl.h>

#define FPUBKEY_LEN PUBKEY_LEN

int fscrypt_generate_metadata_nonce(u8 *nonce, struct inode *inode, size_t len)
{
	if (unlikely(!inode->i_sb->s_cop))
		return -EACCES;
	/*
	 * This is just for compatibility of F2FS.
	 * It should be deleted after F2FS optimise
	 */
	if (inode->i_sb->s_cop->get_generate_nonce) {
		inode->i_sb->s_cop->get_generate_nonce(nonce, inode, len);
		return 0;
	}
	/* Filesytem doesn't support metadata */
	if (!inode->i_sb->s_cop->get_metadata_context ||
	    !inode->i_sb->s_cop->set_ex_metadata_context ||
	    !inode->i_sb->s_cop->get_ex_metadata_context)
		return -EPERM;
	if (!S_ISREG(inode->i_mode) ||
	    !inode->i_sb->s_cop->is_inline_encrypted ||
	    !inode->i_sb->s_cop->is_inline_encrypted(inode))
		return -EPERM;
	get_random_bytes(nonce, CI_KEY_LEN_NEW);
	get_random_bytes(nonce + CI_KEY_LEN_NEW, METADATA_BYTE_IN_KDF);
	return 0;
}

static int fscrypt_ece_metadata_create(u8 *metadata)
{
	ktime_t start_time, end_time;
	uint64_t get_meta_delay = 0;
	int ret;

	if (unlikely(!metadata)) {
		pr_err("%s: invalid metadata\n", __func__);
		return -EINVAL;
	}

	start_time = ktime_get();
	ret = get_metadata(metadata, METADATA_BYTE_IN_KDF);
	end_time = ktime_get();
	if (unlikely(ret)) {
		if (end_time >= start_time)
			get_meta_delay = ktime_to_ms(end_time - start_time);
		pr_err("%s: runtime error %d, get metadata delay %lu \n", __func__, ret, get_meta_delay);
	}

	return ret;
}

static int fscrypt_sece_metadata_create(u8 *metadata, u8 *fpubkey, int keyindex,
					int flag)
{
	ktime_t start_time, end_time;
	uint64_t get_meta_delay = 0;
	int ret;
	struct fbex_sece_param param = { 0 };

	if (unlikely(!metadata || !fpubkey)) {
		pr_err("%s: invalid metadata\n", __func__);
		return -EINVAL;
	}
	param.cmd = flag;
	param.idx = keyindex;
	param.pubkey = fpubkey;
	param.key_len = FPUBKEY_LEN;
	param.metadata = metadata;
	param.iv_len = METADATA_BYTE_IN_KDF;

	start_time = ktime_get();
	ret = get_metadata_sece(&param);
	end_time = ktime_get();
	if (unlikely(ret)) {
		if (end_time >= start_time)
			get_meta_delay = ktime_to_ms(end_time - start_time);
		pr_err("%s: runtime error %d, get metadata delay %lu \n", __func__, ret, get_meta_delay);
	}

	return ret;
}

int fscrypt_get_metadata(struct inode *inode, struct fscrypt_info *ci_info)
{
	int res;
	const struct fscrypt_operations *cop = inode->i_sb->s_cop;

	/*
	 * This is just for compatibility of F2FS.
	 * It should be deleted after F2FS optimise
	 */
	if (cop->open_metadata)
		return cop->open_metadata(inode, ci_info);

	if (unlikely(!ci_info || !ci_info->ci_key)) {
		pr_err("%s: no ci_info for inline-encrypted file!\n", __func__);
		return -EINVAL;
	}

	/* Filesytem doesn't support metadata */
	if (!cop->get_metadata_context)
		return -EOPNOTSUPP;

	res = cop->get_metadata_context(inode, ci_info->ci_metadata,
					METADATA_BYTE_IN_KDF, NULL);
	/*
	* if getting metadata failed, this is a new file after hota
	* update. Therefore, we reuse the last 16 Bytes of ci_key as
	* metadata. if not, this is an old file, we keep the old method
	*/
	if (res == -ENODATA) {
		memcpy(ci_info->ci_metadata, ci_info->ci_key + CI_KEY_LEN_NEW,
		       METADATA_BYTE_IN_KDF);
		ci_info->ci_key_len = CI_KEY_LEN_NEW;
	} else if (res < 0) {
		pr_err("%s: inode %lu get metadata failed, res %d\n", __func__,
		       inode->i_ino, res);
		return res;
	} else if (res != METADATA_BYTE_IN_KDF) {
		pr_err("%s: inode %lu metadata invalid length: %d\n", __func__,
		       inode->i_ino, res);
		return -EINVAL;
	}

	ci_info->ci_key_index |= (CD << FILE_ENCRY_TYPE_BEGIN_BIT);
	return 0;
}

int fscrypt_new_ece_metadata(struct inode *inode, struct fscrypt_info *ci_info,
			     void *fs_data)
{
	int res;

	/* Filesytem doesn't support metadata */
	if (!inode->i_sb->s_cop->set_ex_metadata_context ||
	    !inode->i_sb->s_cop->get_ex_metadata_context)
		return 0;

	res = fscrypt_ece_metadata_create(ci_info->ci_metadata);
	if (unlikely(res)) {
		pr_err("%s: ece metadata create fail! \n", __func__);
		return res;
	}

	res = inode->i_sb->s_cop->set_ex_metadata_context(
		inode, ci_info->ci_metadata, METADATA_BYTE_IN_KDF, fs_data);
	if (unlikely(res)) {
		pr_err("%s: ece metadata set fail! \n", __func__);
		return res;
	}

	ci_info->ci_key_index &= FILE_ENCRY_TYPE_MASK;
	ci_info->ci_key_index |= (ECE << FILE_ENCRY_TYPE_BEGIN_BIT);
	return res;
}

int fscrypt_get_ece_metadata(struct inode *inode, struct fscrypt_info *ci_info,
			     void *fs_data, bool create)
{
	int res;

	/* Filesytem doesn't support metadata */
	if (!inode->i_sb->s_cop->set_ex_metadata_context ||
	    !inode->i_sb->s_cop->get_ex_metadata_context)
		return 0;

	if (create)
		return fscrypt_new_ece_metadata(inode, ci_info, fs_data);

	res = inode->i_sb->s_cop->get_ex_metadata_context(
		inode, ci_info->ci_metadata, METADATA_BYTE_IN_KDF, fs_data);
	if (unlikely(res != METADATA_BYTE_IN_KDF)) {
		pr_err("%s: inode(%lu) get ece metadata failed, res %d\n",
		       __func__, inode->i_ino, res);
		return -EINVAL;
	}
	ci_info->ci_key_index &= FILE_ENCRY_TYPE_MASK;
	ci_info->ci_key_index |= (ECE << FILE_ENCRY_TYPE_BEGIN_BIT);
	return 0;
}

int fscrypt_new_sece_metadata(struct inode *inode, struct fscrypt_info *ci_info,
			      void *fs_data)
{
	int res;
	u8 fpubkey[FPUBKEY_LEN];

	/* Filesytem doesn't support metadata */
	if (!inode->i_sb->s_cop->set_ex_metadata_context ||
	    !inode->i_sb->s_cop->get_ex_metadata_context)
		return 0;

	res = fscrypt_sece_metadata_create(ci_info->ci_metadata, fpubkey,
					   ci_info->ci_key_index,
					   SEC_FILE_ENCRY_CMD_ID_NEW_SECE);
	if (unlikely(res)) {
		pr_err("%s: sece metadata create failed, res:%d\n", __func__,
		       res);
		return res;
	}

	res = inode->i_sb->s_cop->set_ex_metadata_context(inode, fpubkey,
							  FPUBKEY_LEN, fs_data);
	if (unlikely(res)) {
		pr_err("%s: inode(%lu) set metadata failed, res %d\n", __func__,
		       inode->i_ino, res);
		return res;
	}

	ci_info->ci_key_index &= FILE_ENCRY_TYPE_MASK;
	ci_info->ci_key_index |= (SECE << FILE_ENCRY_TYPE_BEGIN_BIT);
	return 0;
}

int fscrypt_check_sece_metadata(struct inode *inode,
				struct fscrypt_info *ci_info, void *fs_data)
{
	u8 fpubkey[FPUBKEY_LEN];
	u8 ci_metadata[METADATA_BYTE_IN_KDF];
	int keyindex;
	int res;

	/* Filesytem doesn't support metadata */
	if (!inode->i_sb->s_cop->set_ex_metadata_context ||
	    !inode->i_sb->s_cop->get_ex_metadata_context)
		return 0;

	res = inode->i_sb->s_cop->get_ex_metadata_context(inode, fpubkey,
							  FPUBKEY_LEN, fs_data);
	if (unlikely(res != FPUBKEY_LEN)) {
		pr_err("%s: inode(%lu) get sece fpubkey failed, res %d\n",
		       __func__, inode->i_ino, res);
		return -EINVAL;
	}

	keyindex = ci_info->ci_key_index & FILE_ENCRY_TYPE_MASK;
	res = fscrypt_sece_metadata_create(ci_metadata, fpubkey, keyindex,
					   SEC_FILE_ENCRY_CMD_ID_GEN_METADATA);
	if (unlikely(res)) {
		pr_err("%s: screen locked, inode(%lu) get sece metadata failed, res:%d\n",
		       __func__, inode->i_ino, res);
		return -EKEYREVOKED;
	}

	if (unlikely(memcmp(ci_metadata, ci_info->ci_metadata,
			    METADATA_BYTE_IN_KDF))) {
		pr_err("%s: inode(%lu) metadata doesn't match %d\n", __func__,
		       inode->i_ino, res);
		return -EIO;
	}
	return 0;
}

int fscrypt_get_sece_metadata(struct inode *inode, struct fscrypt_info *ci_info,
			      void *fs_data, bool create)
{
	u8 fpubkey[FPUBKEY_LEN];
	int keyindex;
	int res;

	if (create)
		return fscrypt_new_sece_metadata(inode, ci_info, fs_data);

	/* Filesytem doesn't support metadata */
	if (!inode->i_sb->s_cop->set_ex_metadata_context ||
	    !inode->i_sb->s_cop->get_ex_metadata_context)
		return 0;

	res = inode->i_sb->s_cop->get_ex_metadata_context(inode, fpubkey,
							  FPUBKEY_LEN, fs_data);
	if (unlikely(res != FPUBKEY_LEN)) {
		pr_err("%s: inode(%lu) get sece fpubkey failed, res %d\n",
		       __func__, inode->i_ino, res);
		return -EINVAL;
	}

	keyindex = ci_info->ci_key_index & FILE_ENCRY_TYPE_MASK;
	res = fscrypt_sece_metadata_create(ci_info->ci_metadata, fpubkey,
					   keyindex,
					   SEC_FILE_ENCRY_CMD_ID_GEN_METADATA);
	if (unlikely(res)) {
		pr_err("%s: screen locked, inode(%lu) get sece metadata failed, res:%d\n",
		       __func__, inode->i_ino, res);
		return -EKEYREVOKED;
	}

	ci_info->ci_key_index &= FILE_ENCRY_TYPE_MASK;
	ci_info->ci_key_index |= (SECE << FILE_ENCRY_TYPE_BEGIN_BIT);
	return 0;
}

static bool fscrypt_encrypt_file_check(struct inode *inode)
{
	if (inode->i_sb->s_cop->encrypt_file_check)
		return inode->i_sb->s_cop->encrypt_file_check(inode);
	return false;
}

int fscrypt_vm_op_check(struct inode *inode)
{
	if (!inode)
		return 0;

	if (unlikely(fscrypt_encrypt_file_check(inode))) {
		pr_err("%s:fscrypt_encrypt_file_check intercept! \n", __func__);
		WARN_ON(1);
		return VM_FAULT_SIGSEGV;
	}
	return 0;
}

int fscrypt_lld_protect(const struct request *request)
{
	int err = 0;
	bool is_encrypt_file = request->ci_key ? true : false;

	if (is_encrypt_file) {
		int file_encrypt_type = (u32)request->ci_key_index >>
					FILE_ENCRY_TYPE_BEGIN_BIT;
		u32 slot_id = (u32)request->ci_key_index &
			      FILE_ENCRY_TYPE_MASK;

		if (file_encrypt_type == ECE) {
			if (fbex_slot_clean(slot_id)) {
				pr_err("[FBE3]%s: key slot is cleared, illegal to send ECE IO\n",
				       __func__);
				err = -EOPNOTSUPP;
			}
		}
	}

	return err;
}

static spinlock_t fe_rw_check_lock;
static int fe_key_file_rw_count;
static bool fe_lock_notify;
static struct work_struct fe_key_protect_work;

static void fe_do_key_protect(struct work_struct *work)
{
	bool clear_key = false;

	sys_sync();
	spin_lock(&fe_rw_check_lock);
	clear_key = fe_lock_notify && (!fe_key_file_rw_count);
	spin_unlock(&fe_rw_check_lock);
	if (clear_key) {
		pr_err("%s: rw count zero, clear keyslot!\n", __func__);
		fbe3_flushcache_done();
	}
}

static void fe_active_key_protect(void)
{
	schedule_work(&fe_key_protect_work);
}

void fbe3_lock_in(void)
{
	spin_lock(&fe_rw_check_lock);
	fe_lock_notify = true;
	pr_err("%s!\n", __func__);
	if (!fe_key_file_rw_count)
		fe_active_key_protect();
	spin_unlock(&fe_rw_check_lock);
}

void fbe3_unlock_in(void)
{
	spin_lock(&fe_rw_check_lock);
	fe_lock_notify = false;
	pr_err("%s!\n", __func__);
	spin_unlock(&fe_rw_check_lock);
}

int rw_begin(struct file *file)
{
	struct inode *inode = NULL;
	int ret = 0;

	if (file->f_mapping)
		inode = file->f_mapping->host;
	if (inode && inode->i_sb->s_cop &&
	    inode->i_sb->s_cop->get_encrypt_type &&
	    inode->i_sb->s_cop->get_encrypt_type(inode) == ECE) {
		int lock_check = fscrypt_encrypt_file_check(inode);
		spin_lock(&fe_rw_check_lock);
		if (lock_check) {
			pr_err("[FBE3]%s: rw for %lu is blocked when screen locked\n",
			       __func__, inode->i_ino);
			ret = -EKEYREVOKED;
		} else {
			fe_key_file_rw_count++;
		}
		spin_unlock(&fe_rw_check_lock);
	}

	return ret;
}

void rw_finish(int read_write, struct file *file)
{
	struct inode *inode = NULL;

	if (file->f_mapping)
		inode = file->f_mapping->host;
	if (inode && inode->i_sb->s_cop &&
	    inode->i_sb->s_cop->get_encrypt_type &&
	    inode->i_sb->s_cop->get_encrypt_type(inode) == ECE) {
		spin_lock(&fe_rw_check_lock);
		if (!fe_key_file_rw_count) {
			pr_err("[FBE3]%s: no match of rw count\n", __func__);
#ifdef CONFIG_HISI_DEBUG_FS
			BUG();
#endif
		}

		if (!(--fe_key_file_rw_count) && fe_lock_notify)
			fe_active_key_protect();
		spin_unlock(&fe_rw_check_lock);
	}
}

static int __init fscrypt_extra_ctrl_init(void)
{
	spin_lock_init(&fe_rw_check_lock);
	INIT_WORK(&fe_key_protect_work, fe_do_key_protect);
	return 0;
}
fs_initcall(fscrypt_extra_ctrl_init);
