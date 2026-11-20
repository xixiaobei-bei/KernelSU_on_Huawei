/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *              package list.
 * Create: 2020-06-16
 */

#include "inc/data/hwdps_packages.h"
#include <linux/hashtable.h>
#include <linux/slab.h>
#include "inc/base/hwdps_utils.h"
#include "inc/policy/hwdps_policy.h"

#define HWDPS_PKG_DEFAULT_HASHTABLESIZE 12

static DEFINE_HASHTABLE(g_packages_hashtable, HWDPS_PKG_DEFAULT_HASHTABLESIZE);

static s32 get_appid(uid_t uid)
{
	return (s32)(uid % HWDPS_PER_USER_RANGE);
}

static struct package_hashnode_t *get_package_hashnode(s32 appid)
{
	struct package_hashnode_t *package_hashnode = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_possible_safe(g_packages_hashtable, package_hashnode,
		tmp, hash_list, appid)
		if (package_hashnode->appid == appid)
			return package_hashnode;
	return NULL;
}

static void delete_pinfo_listnode(struct package_info_listnode_t *pinfo,
	bool free_self)
{
	if (!pinfo)
		return;
	hwdps_utils_free_package_info(pinfo->pinfo);
	kzfree(pinfo->pinfo);
	pinfo->pinfo = NULL;
	hwdps_utils_free_policy_ruleset(pinfo->ruleset_cache);
	kzfree(pinfo->ruleset_cache);
	pinfo->ruleset_cache = NULL;

	/*
	 * Make sure list add at the last time everytime so as not need to
	 * list_del pinfo_listnode.
	 */
	if (free_self)
		kzfree(pinfo);
}

static void delete_package_hashnode(
	struct package_hashnode_t *package_hashnode)
{
	struct package_info_listnode_t *package_info_listnode = NULL;
	struct package_info_listnode_t *tmp = NULL;

	if (!package_hashnode) {
		hwdps_pr_err("%s package_hashnode null\n", __func__);
		return;
	}
	if (!list_empty(&package_hashnode->pinfo_list)) {
		list_for_each_entry_safe(package_info_listnode, tmp,
			&package_hashnode->pinfo_list, list) {
			delete_pinfo_listnode(package_info_listnode, false);
			list_del(&package_info_listnode->list);
			kfree(package_info_listnode);
		}
		hwdps_pr_info("%s delete\n", __func__);
	}
	hash_del(&package_hashnode->hash_list);
	kzfree(package_hashnode);
}

static s32 add_pinfo_to_pkg_listnode(
	struct package_hashnode_t *package_hashnode,
	struct hwdps_package_info_t *pinfo)
{
	s32 ret;
	struct package_info_listnode_t *pinfo_listnode = NULL;

	pinfo_listnode = kzalloc(sizeof(struct package_info_listnode_t),
		GFP_KERNEL);
	if (!pinfo_listnode) {
		ret = -ENOMEM;
		goto cleanup;
	}
	pinfo_listnode->pinfo = kzalloc(sizeof(struct hwdps_package_info_t),
		GFP_KERNEL);
	if (!pinfo_listnode->pinfo) {
		ret = -ENOMEM;
		goto cleanup;
	}
	ret = hwdps_utils_copy_package_info(pinfo_listnode->pinfo,
		pinfo);
	if (ret != 0)
		goto cleanup;
	pinfo_listnode->ruleset_cache = kzalloc(
		sizeof(struct ruleset_cache_entry_t), GFP_KERNEL);
	if (!pinfo_listnode->ruleset_cache) {
		ret = -ENOMEM;
		goto cleanup;
	}
	ret = hwdps_analys_policy_ruleset(pinfo_listnode->ruleset_cache,
		pinfo);
	if (ret != 0)
		goto cleanup;
	list_add(&pinfo_listnode->list, &package_hashnode->pinfo_list);
	hwdps_pr_info("%s add %s in list\n", __func__, pinfo->app_policy);
	return ret;
cleanup:
	delete_pinfo_listnode(pinfo_listnode, true);
	return ret;
}

s32 hwdps_packages_insert(struct hwdps_package_info_t *pinfo)
{
	s32 ret;
	struct package_hashnode_t *package_hashnode = NULL;

	if (!pinfo)
		return -EINVAL;
	package_hashnode = get_package_hashnode(pinfo->appid);
	if (package_hashnode)
		return add_pinfo_to_pkg_listnode(package_hashnode, pinfo);
	package_hashnode = kzalloc(
		sizeof(struct package_hashnode_t), GFP_KERNEL);
	if (!package_hashnode) {
		ret = -ENOMEM;
		goto done;
	}
	package_hashnode->appid = pinfo->appid;
	INIT_LIST_HEAD(&package_hashnode->pinfo_list);
	hash_add(g_packages_hashtable, &package_hashnode->hash_list,
		package_hashnode->appid);
	ret = add_pinfo_to_pkg_listnode(package_hashnode, pinfo);
	if (ret != 0)
		goto cleanup;
	goto done;

cleanup:
	if (package_hashnode)
		delete_package_hashnode(package_hashnode);
done:
	return ret;
}

struct package_hashnode_t *get_hwdps_package_hashnode(uid_t uid)
{
	struct package_hashnode_t *package_hashnode = NULL;
	s32 appid = get_appid(uid);

	package_hashnode = get_package_hashnode(appid);
	if (!package_hashnode)
		return NULL;
	return package_hashnode;
}

static void delete_package_hashnode_by_policy(
	struct package_hashnode_t *package_hashnode, const u8 *policy,
	u64 policy_len)
{
	struct package_info_listnode_t *info = NULL;
	struct package_info_listnode_t *tmp = NULL;

	if (!package_hashnode)
		return;
	if (!list_empty(&package_hashnode->pinfo_list))
		list_for_each_entry_safe(info, tmp,
		&package_hashnode->pinfo_list, list) {
			if ((info->pinfo->app_policy_len == policy_len) &&
				memcmp(info->pinfo->app_policy,
				policy, policy_len) == 0) {
				delete_pinfo_listnode(
					info, false);
				list_del(&info->list);
				kfree(info);
			}
		}

	if (list_empty(&package_hashnode->pinfo_list)) {
		hwdps_pr_info("hwdps delete appid %d\n",
			package_hashnode->appid);
		hash_del(&package_hashnode->hash_list);
		kfree(package_hashnode);
	}
}

void hwdps_packages_delete(struct hwdps_package_info_t *pinfo)
{
	struct package_hashnode_t *package_node = NULL;
	struct hlist_node *tmp = NULL;
	unsigned long bkt = 0;

	if (!pinfo || !pinfo->app_policy) {
		hwdps_pr_err("unexpected bad param\n");
		return;
	}
	hwdps_pr_info("%s app_policy %s\n", __func__, pinfo->app_policy);
	if (hash_empty(g_packages_hashtable)) {
		hwdps_pr_err("pkg hash table empty\n");
		return;
	}

	hash_for_each_safe(g_packages_hashtable, bkt, tmp, package_node,
		hash_list)
		delete_package_hashnode_by_policy(package_node,
			pinfo->app_policy, pinfo->app_policy_len);
}

void hwdps_packages_delete_all(void)
{
	unsigned long bkt = 0;
	struct package_hashnode_t *package_node = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_safe(g_packages_hashtable, bkt, tmp,
		package_node, hash_list)
		delete_package_hashnode(package_node);
}
