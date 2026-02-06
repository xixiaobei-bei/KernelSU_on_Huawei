/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *              package list.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_PACKAGES_H
#define _HWDPS_PACKAGES_H

#include <huawei_platform/hwdps/hwdps_ioctl.h>

struct ruleset_cache_entry_t {
	const s8 *path_node;
	u32 path_node_len;
};

struct package_info_listnode_t {
	struct hwdps_package_info_t *pinfo;
	struct ruleset_cache_entry_t *ruleset_cache;
	struct list_head list;
};

struct package_hashnode_t {
	s32 appid; /* this must be assigned */
	struct list_head pinfo_list;
	struct hlist_node hash_list;
};

s32 hwdps_packages_insert(struct hwdps_package_info_t *pinfo);

void hwdps_packages_delete(struct hwdps_package_info_t *pinfo);

void hwdps_packages_delete_all(void);

struct package_hashnode_t *get_hwdps_package_hashnode(uid_t uid);

#endif
