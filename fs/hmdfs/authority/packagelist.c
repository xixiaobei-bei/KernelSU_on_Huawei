/*
 * copied from fs/sdcardfs/packagelist.c
 *
 * Copyright (c) 2013 Samsung Electronics Co. Ltd
 *   Authors: Daeho Jeong, Woojoong Lee, Seunghwan Hyun,
 *               Sunghwan Yun, Sungjong Seo
 *
 * This program has been developed as a stackable file system based on
 * the WrapFS which written by
 *
 * Copyright (c) 1998-2011 Erez Zadok
 * Copyright (c) 2009     Shrikar Archak
 * Copyright (c) 2003-2011 Stony Brook University
 * Copyright (c) 2003-2011 The Research Foundation of SUNY
 *
 * This file is dual licensed.  It may be redistributed and/or modified
 * under the terms of the Apache 2.0 License OR version 2 of the GNU
 * General Public License.
 */
#include <linux/configfs.h>
#include <linux/ctype.h>
#include <linux/dcache.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include "hmdfs.h"

DEFINE_MUTEX(hmdfs_super_list_lock);

static struct kmem_cache *hashtable_entry_cachep;

struct hashtable_entry {
	struct hlist_node hlist;
	struct hlist_node dlist;
	struct qstr key;
	atomic_t value;
};

static DEFINE_HASHTABLE(package_to_appid, 8);

static unsigned int full_name_case_hash(const void *salt,
					const unsigned char *name,
					unsigned int len)
{
	unsigned long hash = init_name_hash(salt);

	while (len--)
		hash = partial_name_hash(tolower(*name++), hash);
	return end_name_hash(hash);
}

struct package_details {
	struct config_item item;
	struct qstr name;
};

static void free_hashtable_entry(struct hashtable_entry *entry)
{
	kfree(entry->key.name);
	kmem_cache_free(hashtable_entry_cachep, entry);
}

static void remove_packagelist_entry_locked(const struct qstr *key)
{
	struct hashtable_entry *hash_cur;
	unsigned int hash = key->hash;
	struct hlist_node *h_t;
	HLIST_HEAD(free_list);

	hash_for_each_possible_rcu(package_to_appid, hash_cur, hlist, hash) {
		if (qstr_case_eq(key, &hash_cur->key)) {
			hash_del_rcu(&hash_cur->hlist);
			hlist_add_head(&hash_cur->dlist, &free_list);
			break;
		}
	}
	synchronize_rcu();
	hlist_for_each_entry_safe(hash_cur, h_t, &free_list, dlist)
		free_hashtable_entry(hash_cur);
}

static inline int qstr_copy(const struct qstr *src, struct qstr *dest)
{
	dest->name = kstrdup(src->name, GFP_KERNEL);
	dest->hash_len = src->hash_len;
	return !!dest->name;
}

static inline void qstr_init(struct qstr *q, const char *name)
{
	q->name = name;
	q->len = strlen(q->name);
	q->hash = full_name_case_hash(0, q->name, q->len);
}

static uid_t __get_appid(const struct qstr *key)
{
	struct hashtable_entry *hash_cur;
	unsigned int hash = key->hash;
	uid_t ret_id;

	rcu_read_lock();
	hash_for_each_possible_rcu(package_to_appid, hash_cur, hlist, hash) {
		if (qstr_case_eq(key, &hash_cur->key)) {
			ret_id = atomic_read(&hash_cur->value);
			rcu_read_unlock();
			return ret_id;
		}
	}
	rcu_read_unlock();
	return 0;
}

uid_t hmdfs_get_appid(const char *key)
{
	struct qstr q;

	qstr_init(&q, key);
	return __get_appid(&q);
}

static struct hashtable_entry *alloc_hashtable_entry(const struct qstr *key,
						     uid_t value)
{
	struct hashtable_entry *ret = kmem_cache_alloc(hashtable_entry_cachep,
							GFP_KERNEL);
	if (!ret)
		return NULL;
	INIT_HLIST_NODE(&ret->dlist);
	INIT_HLIST_NODE(&ret->hlist);

	if (!qstr_copy(key, &ret->key)) {
		kmem_cache_free(hashtable_entry_cachep, ret);
		return NULL;
	}

	atomic_set(&ret->value, value);
	return ret;
}

static int insert_packagelist_appid_entry_locked(const struct qstr *key,
						 uid_t value)
{
	struct hashtable_entry *hash_cur;
	struct hashtable_entry *new_entry;
	unsigned int hash = key->hash;

	hash_for_each_possible_rcu(package_to_appid, hash_cur, hlist, hash) {
		if (qstr_case_eq(key, &hash_cur->key)) {
			atomic_set(&hash_cur->value, value);
			return 0;
		}
	}
	new_entry = alloc_hashtable_entry(key, value);
	if (!new_entry)
		return -ENOMEM;
	hash_add_rcu(package_to_appid, &new_entry->hlist, hash);
	return 0;
}

static int insert_packagelist_entry(const struct qstr *key, uid_t value)
{
	int err;
	uid_t old_appid = hmdfs_get_appid(key->name);

	if (old_appid == value)
		return 0;
	mutex_lock(&hmdfs_super_list_lock);
	err = insert_packagelist_appid_entry_locked(key, value);
	mutex_unlock(&hmdfs_super_list_lock);

	return err;
}

static inline
struct package_details *to_package_details(struct config_item *item)
{
	return item ? container_of(item, struct package_details, item) : NULL;
}

static ssize_t package_details_appid_store(struct config_item *item,
				       const char *page, size_t count)
{
	unsigned int tmp;
	int ret;

	ret = kstrtouint(page, 10, &tmp);
	if (ret)
		return ret;

	ret = insert_packagelist_entry(&to_package_details(item)->name, tmp);

	if (ret)
		return ret;

	return count;
}

static void remove_packagelist_entry(const struct qstr *key)
{
	mutex_lock(&hmdfs_super_list_lock);
	remove_packagelist_entry_locked(key);
	mutex_unlock(&hmdfs_super_list_lock);
}

static void package_details_release(struct config_item *item)
{
	struct package_details *package_details = to_package_details(item);

	hmdfs_info("removing %s", package_details->name.name);
	remove_packagelist_entry(&package_details->name);
	kfree(package_details->name.name);
	kfree(package_details);
}

static ssize_t package_details_appid_show(struct config_item *item, char *page)
{
	return scnprintf(page, PAGE_SIZE, "%u\n",
			__get_appid(&to_package_details(item)->name));
}

static struct configfs_attribute package_details_attr_appid = {
	.ca_name	= "appid",
	.ca_mode	= S_IRUGO | S_IWUGO,
	.ca_owner	= THIS_MODULE,
	.show		= package_details_appid_show,
	.store		= package_details_appid_store,
};

// only config appid from userspace
static struct configfs_attribute *package_details_attrs[] = {
	&package_details_attr_appid,
	NULL,
};

static struct configfs_item_operations package_details_item_ops = {
	.release = package_details_release,
};

static struct config_item_type package_appid_type = {
	.ct_item_ops	= &package_details_item_ops,
	.ct_attrs	= package_details_attrs,
	.ct_owner	= THIS_MODULE,
};

// mkdir in hmdfs/
static struct config_item *packages_make_item(struct config_group *group,
					      const char *name)
{
	struct package_details *package_details;
	const char *tmp;

	package_details = kzalloc(sizeof(*package_details), GFP_KERNEL);
	if (!package_details)
		return ERR_PTR(-ENOMEM);
	tmp = kstrdup(name, GFP_KERNEL);
	if (!tmp) {
		kfree(package_details);
		return ERR_PTR(-ENOMEM);
	}
	qstr_init(&package_details->name, tmp);
	config_item_init_type_name(&package_details->item, name,
						&package_appid_type);

	return &package_details->item;
}

static struct configfs_group_operations packages_group_ops = {
	.make_item	= packages_make_item,
};

static struct config_item_type packages_type = {
	.ct_group_ops	= &packages_group_ops,
	.ct_owner	= THIS_MODULE,
};

static struct configfs_subsystem hmdfs_packages = {
	.su_group = {
		.cg_item = {
			.ci_namebuf = "hmdfs",
			.ci_type = &packages_type,
		},
	},
};

static int configfs_hmdfs_init(void)
{
	int ret;
	struct configfs_subsystem *subsys = &hmdfs_packages;

	config_group_init(&subsys->su_group);
	mutex_init(&subsys->su_mutex);
	ret = configfs_register_subsystem(subsys);
	if (ret)
		hmdfs_err("Error %d while registering subsystem", ret);

	return ret;
}

static void configfs_hmdfs_exit(void)
{
	configfs_unregister_subsystem(&hmdfs_packages);
}

int hmdfs_packagelist_init(void)
{
	hashtable_entry_cachep =
		kmem_cache_create("packagelist_hashtable_entry",
				sizeof(struct hashtable_entry), 0, 0, NULL);
	if (!hashtable_entry_cachep) {
		hmdfs_err("failed creating pkgl_hashtable entry slab cache");
		return -ENOMEM;
	}

	configfs_hmdfs_init();
	return 0;
}

static void hmdfs_packagelist_destroy(void)
{
	struct hashtable_entry *hash_cur;
	struct hlist_node *h_t;
	HLIST_HEAD(free_list);
	int i;

	mutex_lock(&hmdfs_super_list_lock);
	hash_for_each_rcu(package_to_appid, i, hash_cur, hlist) {
		hash_del_rcu(&hash_cur->hlist);
		hlist_add_head(&hash_cur->dlist, &free_list);
	}

	synchronize_rcu();
	hlist_for_each_entry_safe(hash_cur, h_t, &free_list, dlist)
		free_hashtable_entry(hash_cur);
	mutex_unlock(&hmdfs_super_list_lock);
	hmdfs_info("destroyed packagelist pkgld");
}

void hmdfs_packagelist_exit(void)
{
	configfs_hmdfs_exit();
	hmdfs_packagelist_destroy();
	kmem_cache_destroy(hashtable_entry_cachep);
}
