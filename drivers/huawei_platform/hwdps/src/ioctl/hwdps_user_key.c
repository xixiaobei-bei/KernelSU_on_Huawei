/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: This file contains the function required for operations
 *              about user key
 * Create: 2018-12-10
 * History: 2020-10-10
 */
#include "inc/ioctl/hwdps_user_key.h"
#include <uapi/linux/keyctl.h>
#include <keys/user-type.h>
#include <securec.h>
#include "inc/base/hwdps_utils.h"

static s32 get_user_key_from_keyring(struct key *keyring_key, u8 *user_key,
	u32 *user_key_len)
{
	s32 ret = 0;
	const struct user_key_payload *ukp;
	struct fscrypt_key *primary_key = NULL;

	ukp = user_key_payload_locked(keyring_key);
	if (!ukp) {
		hwdps_pr_err("Key was revoked before it could be acquired!");
		return -EKEYREVOKED;
	}

	primary_key = (struct fscrypt_key *)ukp->data;
	if ((ukp->datalen != sizeof(struct fscrypt_key)) ||
		(primary_key->size < 1) || /* the min num size */
		(primary_key->size > FS_MAX_KEY_SIZE)) {
		hwdps_pr_err("size of the fscrypt is incorrect! datalen = %d",
			ukp->datalen);
		return -EINVAL;
	}
	if (primary_key->size > *user_key_len) {
		hwdps_pr_err("user k %d is greater than %d  allocated buffer",
			primary_key->size, *user_key_len);
		return -EINVAL;
	} else {
		if (memcpy_s(user_key, *user_key_len, primary_key->raw,
			primary_key->size) != EOK) {
			return -EINVAL;
		}
		*user_key_len = primary_key->size;
	}
	return ret;
}

s32 get_user_key(u8 *user_key_desc_str, u32 user_key_desc_str_len,
	u8 *user_key, u32 *user_key_len)
{
	s32 ret;
	struct key *keyring_key = NULL;

	if (!user_key_desc_str ||
		user_key_desc_str_len != HWDPS_USER_KEY_DESC_STR_SIZE ||
		!user_key || !user_key_len)
		return -EINVAL;

	keyring_key = request_key(&key_type_logon, user_key_desc_str, NULL);
	if (IS_ERR(keyring_key)) {
		hwdps_pr_err("Failed to get keyring!");
		ret = PTR_ERR(keyring_key);
		goto do_return;
	}
	down_read(&keyring_key->sem);
	if (keyring_key->type != &key_type_logon) {
		hwdps_pr_err("The keyring key type is not logon!");
		ret = -ENOKEY;
		goto invalid;
	}
	ret = get_user_key_from_keyring(keyring_key, user_key, user_key_len);
	if (ret != 0)
		hwdps_pr_err("Failed to get user key from keyring,ret:%d", ret);

invalid:
	up_read(&keyring_key->sem);
	key_put(keyring_key);

do_return:
	return ret;
}
