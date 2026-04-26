/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for init_user and
 *              package management.
 * Create: 2020-06-16
 */

#include "inc/ioctl/hwdps_ioctl_impl.h"
#include <securec.h>
#include <huawei_platform/hwdps/hwdps_ioctl.h>
#include "inc/base/hwdps_utils.h"
#include "inc/data/hwdps_data.h"
#include "inc/data/hwdps_packages.h"
#include "inc/policy/hwdps_policy.h"
#include "inc/tee/hwdps_adapter.h"
#include "inc/ioctl/hwdps_user_key.h"
#include "inc/base/hwdps_defines.h"

static hwdps_result_t get_result(s32 ret)
{
	switch (ret) {
	case 0:
		return HWDPS_SUCCESS;
	case -EINVAL:
		return HWDPS_ERR_INVALID_ARGS;
	case -ENOMEM:
		return HWDPS_ERR_NO_MEMORY;
	default:
		return HWDPS_ERR_INTERNAL;
	}
}

static void make_user_key_desc_str(u8 *user_key_desc_str,
	u32 user_key_desc_str_len, const u8 *user_key_desc_suffix,
	u32 user_key_desc_suffix_len)
{
	if (memcpy_s(user_key_desc_str, user_key_desc_str_len,
		HWDPS_USER_KEY_DESC_PREFIX_STR,
		HWDPS_USER_KEY_DESC_PREFIX_STR_SIZE) != EOK)
		return;

	bytes_to_string(user_key_desc_suffix, user_key_desc_suffix_len,
		user_key_desc_str + HWDPS_USER_KEY_DESC_PREFIX_STR_SIZE,
		HWDPS_USER_KEY_DESC_SUFFIX_STR_SIZE);
}

void hwdps_sync_installed_packages(
	struct hwdps_sync_packages_t *sync_installed_packages)
{
	s32 ret;
	u32 i;

	if (!sync_installed_packages)
		return;

	hwdps_data_write_lock();
	for (i = 0; i < sync_installed_packages->package_count; i++) {
		ret = hwdps_packages_insert(
			&sync_installed_packages->packages[i]);
		if (ret != 0)
			break;
	}

	hwdps_data_write_unlock();
	sync_installed_packages->ret = get_result(ret);
}

void hwdps_install_package(struct hwdps_install_package_t *install_package)
{
	s32 ret;

	if (!install_package)
		return;

	hwdps_data_write_lock();
	ret = hwdps_packages_insert(&install_package->pinfo);
	hwdps_data_write_unlock();
	install_package->ret = get_result(ret);
}

void hwdps_uninstall_package(
	struct hwdps_uninstall_package_t *uninstall_package)
{
	if (!uninstall_package)
		return;

	hwdps_data_write_lock();
	hwdps_packages_delete(&uninstall_package->pinfo);
	hwdps_data_write_unlock();
	uninstall_package->ret = HWDPS_SUCCESS;
}

void hwdps_init_user(struct hwdps_init_user_t *iusr)
{
	s32 ret;
	u8 user_key[HWDPS_USER_KEY_SIZE_MAX] = {0};
	u32 user_key_len = HWDPS_USER_KEY_SIZE_MAX;
	u8 user_key_desc_str[HWDPS_USER_KEY_DESC_STR_SIZE] = {0};

	make_user_key_desc_str(user_key_desc_str, sizeof(user_key_desc_str),
		iusr->key_desc, iusr->key_desc_len);
	ret = get_user_key(user_key_desc_str, sizeof(user_key_desc_str),
		user_key, &user_key_len);
	/* only use the the first half of user key bytes */
	user_key_len = (user_key_len == USER_KEY_LENGTH) ? (user_key_len / 2) :
		user_key_len;
	if (ret != HWDPS_SUCCESS) {
		hwdps_pr_err("Failed to get user key for ausn = %lld\n",
			iusr->ausn);
		iusr->ret = HWDPS_ERR_UNKNOWN_USER;
	} else {
		hwdps_pr_info("hwdps start check cred\n");
		ret = kernel_init_credential(iusr->ausn, user_key,
			user_key_len);
		hwdps_pr_info("end check cred\n");
		if (ret == ERR_MSG_SUCCESS) {
			iusr->ret = HWDPS_SUCCESS;
		} else {
			hwdps_pr_err(" ausn = %lld! ret = %d\n", iusr->ausn,
				ret);
			iusr->ret = HWDPS_ERR_INTERNAL;
		}
	}
	hwdps_pr_info("%s: ret %d, ausn:%lld\n", __func__, ret, iusr->ausn);

	if (memset_s(user_key, sizeof(user_key), 0, sizeof(user_key)) != EOK)
		hwdps_pr_err("%s memset_s fail\n", __func__);
}
