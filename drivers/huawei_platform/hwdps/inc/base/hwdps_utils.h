/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations about
 *              some tools.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_UTILS_H
#define _HWDPS_UTILS_H
#include <linux/kernel.h>
#include <huawei_platform/hwdps/hwdps_ioctl.h>

/* debug log tag */
#define HWDPS_DEBUG_TAG "hwdps"
#define HWDPS_PRINT_TAG HWDPS_DEBUG_TAG
#define hwdps_pr_err(fmt, args...) pr_err(" %s: " fmt "\n", \
	HWDPS_PRINT_TAG, ## args)
#define hwdps_pr_warn(fmt, args...) pr_warn(" %s: " fmt "\n", \
	HWDPS_PRINT_TAG, ## args)
#define hwdps_pr_warn_once(fmt, args...) pr_warn_once("%s: " fmt "\n", \
	HWDPS_PRINT_TAG, ## args)
#define hwdps_pr_info(fmt, args...) pr_info(" %s: " fmt "\n", \
	HWDPS_PRINT_TAG, ## args)
#define hwdps_pr_info_once(fmt, args...) pr_info_once(" %s: " fmt "\n", \
	HWDPS_PRINT_TAG, ## args)
#define hwdps_pr_debug(fmt, args...) pr_debug(" %s: " fmt "\n", \
	HWDPS_PRINT_TAG, ## args)
#define HWDPS_PER_USER_RANGE 100000
#define SYNC_PKG_CNT_MAX 200

void bytes_to_string(const u8 *bytes, u32 len, s8 *str, u32 str_len);

s64 hwdps_utils_get_ausn(uid_t uid);

bool hwdps_utils_exe_check(pid_t pid, const s8 *exe_path,
	s32 exe_path_len);

void hwdps_utils_free_package_info(struct hwdps_package_info_t *pinfo);

s32 hwdps_utils_copy_package_info(struct hwdps_package_info_t *dst,
	const struct hwdps_package_info_t *src);

s32 hwdps_utils_copy_package_info_from_user(
	struct hwdps_package_info_t *dst_info,
	const struct hwdps_package_info_t *src_info, bool copy_signer);

s32 hwdps_utils_copy_packages_from_user(struct hwdps_package_info_t *kern_pack,
	const struct hwdps_package_info_t *user_pack, u32 package_count);

/*
 * Description: This is called under very limited situations,
 *              key is always a hardcoded const string while str is gotton
 *              from outside, example {"path":"/data/user/0/com.pkg.xx"}.
 * Input: str: the origin string data
 * Input: key: the value of key
 * Output: out_len: the length of return value
 * Return: value: successfully get the key value
 *         -NULL: on failure
 */
s8 *hwdps_utils_get_json_str(const s8 *str, const s8 *key, u32 *out_len);
#endif
