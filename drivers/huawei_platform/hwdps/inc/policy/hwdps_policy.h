/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for policy.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_POLICY_H
#define _HWDPS_POLICY_H

#include <huawei_platform/hwdps/hwdps_ioctl.h>
#include "inc/data/hwdps_packages.h"

#define CERTINFO_HEX_LENGTH 64
#define PRIMARY_USER_FILE_PATH "/data/"
#define SUB_USER_FILE_PATH "/user/"
#define PRIMARY_PATH_NODE "/data/user/0/"
#define SUB_PATH_NODE "/data/user/"

bool hwdps_evaluate_policies(struct package_info_listnode_t *pkg_info_node,
	const u8 *fsname, const u8 *filename);

s32 hwdps_analys_policy_ruleset(struct ruleset_cache_entry_t *ruleset,
	struct hwdps_package_info_t *pinfo);

void hwdps_utils_free_policy_ruleset(struct ruleset_cache_entry_t *ruleset);

#endif
