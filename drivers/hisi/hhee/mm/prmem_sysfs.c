/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Memory protection library - core functions
 * Date: 2020/04/15
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/hisi/prmem.h>
#include <linux/hisi/hkip.h>

/* ------------------------- sysfs common ----------------------------- */

extern struct kobject *kernel_kobj;

struct kobject *prmem_kobj;
EXPORT_SYMBOL(prmem_kobj);

static inline struct kobject *prmem_create_dir(const char *name,
					       struct kobject *parent)
{
	struct kobject *dir = NULL;

	dir = kobject_create_and_add(name, parent);
	if (!dir)
		return NULL;
	dir->sd->mode &= 077000;
	dir->sd->mode |= 000750;
	return dir;
}

#define SYSFS_SHOW_HELPER(obj, entry, format, value)			\
static ssize_t prmem_##obj##_show_##entry(struct kobject *dev,		\
					  struct kobj_attribute *attr,	\
					  char *buf)			\
{									\
	struct prmem_##obj##_sysfs_data *data = NULL;			\
									\
	data = container_of(attr, struct prmem_##obj##_sysfs_data,	\
			    attr_##entry);				\
	return snprintf(buf, PAGE_SIZE, format "\n", value);		\
}

#define SYSFS_SHOW_FIELD_HELPER(obj, entry, format, field)		\
	SYSFS_SHOW_HELPER(obj, entry, format, data->obj->field)

#define prmem_sysfs_attr_init(data, obj, attr_name)			\
do {									\
	sysfs_attr_init(&data->attr_##attr_name.attr);			\
	data->attr_##attr_name.attr.name = #attr_name;			\
	data->attr_##attr_name.attr.mode =				\
		VERIFY_OCTAL_PERMISSIONS(0640);				\
	data->attr_##attr_name.show = prmem_##obj##_show_##attr_name;	\
} while (0)

/* ---------------------- sysfs phys_pages_db  ------------------------ */

extern struct prmem_phys_db prmem_pmalloc_phys_db;
extern struct prmem_phys_db prmem_pgtable_pages_phys_db;
/**
 * struct prmem_sysfs_phys_db_data - structure for sysfs entries of phys db
 * @phys_db: pointer to the associated phys_db structure
 * @attr_db_pages_per_chunk: per chunk pages in the phys_db
 * @attr_total_free_pages_nr: the number of free pages in the phys_db
 * @attr_chunks_nr: the number of chunks currently allocated
 * @attr_max_chunks_nr: the number of max chunks that can be allocated
 *
 * NOTICE: the naming of both the structure and its associated reference
 * data are designed to match the pattern exploited in the macros
 * SYSFS_SHOW_HELPER() and SYSFS_SHOW_FIELD_HELPER()
 * Change the naming pattern and suffer the consequences of this action.
 */
struct prmem_phys_db_sysfs_data {
	struct prmem_phys_db *phys_db;
	struct kobj_attribute attr_max_pages_nr;
	struct kobj_attribute attr_db_pages_per_chunk;
	struct kobj_attribute attr_total_free_pages_nr;
	struct kobj_attribute attr_total_used_pages_nr;
	struct kobj_attribute attr_peak_total_used_pages_nr;
	struct kobj_attribute attr_chunks_nr;
	struct kobj_attribute attr_max_chunks_nr;
};

/*
 * NOTE: the db_pages_per_chunk uses a scaling factor together with the field.
 * For this to work, the field must be written first, and no braces must
 * be used.
 */
SYSFS_SHOW_FIELD_HELPER(phys_db, db_pages_per_chunk, "%08lu",
			db_pages_per_chunk);
SYSFS_SHOW_FIELD_HELPER(phys_db, total_free_pages_nr, "%08lu",
			total_free_pages_nr);
SYSFS_SHOW_FIELD_HELPER(phys_db, total_used_pages_nr, "%08lu",
			total_used_pages_nr);
SYSFS_SHOW_FIELD_HELPER(phys_db, peak_total_used_pages_nr, "%08lu",
			peak_total_used_pages_nr);
SYSFS_SHOW_FIELD_HELPER(phys_db, chunks_nr, "%08u", chunks_nr);

static __init
struct kobject *create_prmem_sysfs_phys_db(struct prmem_phys_db *db,
					   struct kobject *parent)
{
	struct kobject *prmem_phys_db_kobj = NULL;
	struct prmem_phys_db_sysfs_data *data = NULL;

	prmem_phys_db_kobj = prmem_create_dir(db->name, parent);
	if (!prmem_phys_db_kobj)
		return NULL;
	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;
	data->phys_db = db;
	prmem_sysfs_attr_init(data, phys_db, db_pages_per_chunk);
	prmem_sysfs_attr_init(data, phys_db, total_free_pages_nr);
	prmem_sysfs_attr_init(data, phys_db, total_used_pages_nr);
	prmem_sysfs_attr_init(data, phys_db, peak_total_used_pages_nr);
	prmem_sysfs_attr_init(data, phys_db, chunks_nr);
	{
		const struct attribute *attrs[] = {
			&data->attr_db_pages_per_chunk.attr,
			&data->attr_total_free_pages_nr.attr,
			&data->attr_total_used_pages_nr.attr,
			&data->attr_peak_total_used_pages_nr.attr,
			&data->attr_chunks_nr.attr,
			NULL,
		};
		sysfs_create_files(prmem_phys_db_kobj, attrs);
	}
	wr_assign(db->sysfs_data, data);
	return prmem_phys_db_kobj;
}

static __init void create_prmem_sysfs_phys_dbs(struct kobject *parent)
{
	struct kobject *phys_dbs_kobj = NULL;
	struct kobject *phys_db = NULL;


	phys_dbs_kobj = prmem_create_dir("phys_dbs", parent);
	if (phys_dbs_kobj == NULL)
		return;
	phys_db = create_prmem_sysfs_phys_db(&prmem_pmalloc_phys_db,
					     phys_dbs_kobj);
	if (phys_db == NULL)
		return;
	phys_db = create_prmem_sysfs_phys_db(&prmem_pgtable_pages_phys_db,
					     phys_dbs_kobj);
	if (phys_db == NULL)
		return;
}

/* -------------------------- sysfs pools  ---------------------------- */

/**
 * struct prmem_pool_sysfs_data - structure used for pool sysfs entries
 * @pool: pointer to the associated pool structure
 * @attr_protection_status: the pool protection
 * @attr_allocated: the amount of memory currently allocated for the pool
 * @attr_available: memory immediately available for allocation
 * @attr_cap: cap on the pool allocation
 * @attr_allocation_type: type of the pool
 *
 * NOTICE: the naming of both the structure and its associated reference
 * data are designed to match the pattern exploited in the macros
 * SYSFS_SHOW_HELPER() and SYSFS_SHOW_FIELD_HELPER()
 * Change the naming pattern and suffer the consequences of this action.
 */
struct prmem_pool_sysfs_data {
	struct prmem_pool *pool;
	struct kobj_attribute attr_protection_status;
	struct kobj_attribute attr_allocated;
	struct kobj_attribute attr_peak_allocated;
	struct kobj_attribute attr_available;
	struct kobj_attribute attr_cap;
	struct kobj_attribute attr_allocation_type;
};

static const char *prmem_pool_types_names[] = {
	[PRMEM_RO_NO_RECL] = "read only non reclaimable",
	[PRMEM_WR_NO_RECL] = "write rare non reclaimable",
	[PRMEM_START_WR_NO_RECL] =
		"pre-protected write rare non reclaimable",
	[PRMEM_START_WR_RECL] = "pre-protected write rare reclaimable",
	[PRMEM_WR_RECL] = "write rare reclaimable",
	[PRMEM_RO_RECL] = "read only reclaimable",
	[PRMEM_RW_RECL] = "read write reclaimable",
};

static const char *prmem_protection_names[] = {
	[PMALLOC_UNPROTECTED] = "unprotected",
	[PMALLOC_FAKE_NO_HHEE] = "fake_no_hhee",
	[PMALLOC_FAKE_BYPASSED] = "fake_bypassed",
	[PMALLOC_PROTECTED] = "protected",
};

static inline const char *get_protection_name(struct prmem_pool_sysfs_data
					      *data)
{
	enum prmem_protection_states index;

	index = data->pool->protection_status;
	return prmem_protection_names[index];
}

SYSFS_SHOW_HELPER(pool, protection_status, "%s",
		  get_protection_name(data));
SYSFS_SHOW_FIELD_HELPER(pool, allocated, "%016lu", allocated);
SYSFS_SHOW_FIELD_HELPER(pool, peak_allocated, "%016lu", peak_allocated);
SYSFS_SHOW_FIELD_HELPER(pool, available, "%016lu", offset);
SYSFS_SHOW_FIELD_HELPER(pool, cap, "%016lu", cap);

static ssize_t prmem_pool_show_allocation_type(struct kobject *dev,
					       struct kobj_attribute *attr,
					       char *buf)
{
	struct prmem_pool_sysfs_data *data = NULL;
	enum prmem_pool_type type;

	data = container_of(attr, struct prmem_pool_sysfs_data,
			    attr_allocation_type);
	type = (enum prmem_pool_type)pool_to_prmem_range_type(data->pool);
	if (type >= PRMEM_POOL_TYPE_NUM) {
		pr_err("prmem get pool type err\n");
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%s\n",
			prmem_pool_types_names[type]);
}

static struct kobject *create_prmem_sysfs_pool(struct prmem_pool *pool,
					       struct kobject *parent)
{
	struct kobject *root = NULL;
	struct prmem_pool_sysfs_data *data = NULL;

	root = prmem_create_dir(pool->name, parent);
	if (!root)
		return NULL;
	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;
	data->pool = pool;
	prmem_sysfs_attr_init(data, pool, protection_status);
	prmem_sysfs_attr_init(data, pool, allocated);
	prmem_sysfs_attr_init(data, pool, available);
	prmem_sysfs_attr_init(data, pool, peak_allocated);
	prmem_sysfs_attr_init(data, pool, cap);
	prmem_sysfs_attr_init(data, pool, allocation_type);
	{
		const struct attribute *attrs[] = {
			&data->attr_protection_status.attr,
			&data->attr_allocated.attr,
			&data->attr_peak_allocated.attr,
			&data->attr_available.attr,
			&data->attr_cap.attr,
			&data->attr_allocation_type.attr,
			NULL,
		};
		sysfs_create_files(root, attrs);
	}
	wr_assign(pool->sysfs_data, data);
	return root;
}

static __init void create_prmem_sysfs_pools(struct kobject *parent)
{
	unsigned long start;
	unsigned long end;
	unsigned long size;
	unsigned long p;
	struct kobject *pools_kobj = NULL;
	struct kobject *pool = NULL;

	pools_kobj = prmem_create_dir("pools", parent);
	if (!pools_kobj)
		return;

	start = (unsigned long)__start_data_prmem_pools;
	end = (unsigned long)__end_data_prmem_pools;
	size = roundup(sizeof(struct prmem_pool), sizeof(void *));
	for (p = start; p < end; p += size) {
		pool = create_prmem_sysfs_pool((struct prmem_pool *)(uintptr_t)p,
					       pools_kobj);
		if (!pool)
			return;
	}
}

/* -------------------------- sysfs caches ---------------------------- */
/*
 * struct prmem_cache_sysfs_data - structure used for cache sysfs entries
 * @cache: pointer to the associated cache structure
 * @attr_count: the number of objects in the cache
 * @attr_size: the size of the objects in the cache
 * @attr_pool: the associated pool, if any
 *
 * NOTICE: the naming of both the structure and its associated reference
 * data are designed to match the pattern exploited in the macros
 * SYSFS_SHOW_HELPER() and SYSFS_SHOW_FIELD_HELPER()
 * Change the naming pattern and suffer the consequences of this action.
 */
struct prmem_cache_sysfs_data {
	struct prmem_cache *cache;
	struct kobj_attribute attr_count;
	struct kobj_attribute attr_peak_count;
	struct kobj_attribute attr_size;
	struct kobj_attribute attr_pool;
};

SYSFS_SHOW_FIELD_HELPER(cache, count, "%016lu", count);
SYSFS_SHOW_FIELD_HELPER(cache, peak_count, "%016lu", peak_count);
SYSFS_SHOW_FIELD_HELPER(cache, size, "%016u", size);
SYSFS_SHOW_HELPER(cache, pool, "%s", (data->cache->pool) ?
		  (data->cache->pool->name) : "NONE");

static struct kobject *create_prmem_sysfs_cache(struct prmem_cache *cache,
						struct kobject *parent)
{
	struct kobject *root = NULL;
	struct prmem_cache_sysfs_data *data = NULL;

	root = prmem_create_dir(cache->name, parent);
	if (!root)
		return NULL;
	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return NULL;
	data->cache = cache;
	prmem_sysfs_attr_init(data, cache, count);
	prmem_sysfs_attr_init(data, cache, peak_count);
	prmem_sysfs_attr_init(data, cache, size);
	prmem_sysfs_attr_init(data, cache, pool);
	{
		const struct attribute *attrs[] = {
			&data->attr_count.attr,
			&data->attr_peak_count.attr,
			&data->attr_size.attr,
			&data->attr_pool.attr,
			NULL,
		};
		sysfs_create_files(root, attrs);
	}
	wr_assign(cache->sysfs_data, data);
	return root;
}

static void create_prmem_sysfs_caches(struct kobject *parent)
{
	unsigned long start;
	unsigned long end;
	unsigned long size;
	unsigned long p;
	struct kobject *caches_kobj = NULL;

	caches_kobj = prmem_create_dir("caches", parent);
	if (!caches_kobj)
		return;

	start = (unsigned long)__start_data_prmem_object_caches;
	end = (unsigned long)__end_data_prmem_object_caches;
	size = roundup(sizeof(struct prmem_cache), sizeof(void *));
	for (p = start; p < end; p += size) {
		struct kobject *cache = NULL;

		cache = create_prmem_sysfs_cache((struct prmem_cache *)(uintptr_t)p,
			caches_kobj);
		if (!cache)
			return;
	}
}

/* ------------------------- sysfs status ----------------------------- */

static ssize_t status_show(struct kobject *kobj,
			   struct kobj_attribute *attribute, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", get_prmem_status());
}

static struct kobj_attribute *create_status_sysfs(void)
{
	struct kobj_attribute *attr = NULL;

	attr = kmalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
	if (!attr)
		return NULL;
	sysfs_attr_init(&(attr->attr));

	attr->attr.name = "status";
	attr->attr.mode = 0440;
	attr->show = status_show;
	return attr;
}

/* ----------------------------- init --------------------------------- */

static __init int create_prmem_sysfs_entries(void)
{
	struct kobj_attribute *status = NULL;

	prmem_kobj = prmem_create_dir("prmem", kernel_kobj);
	if (!prmem_kobj)
		goto err1;
	status = create_status_sysfs();
	if (!status)
		goto err2;
	sysfs_create_file(prmem_kobj, (struct attribute *)status);
	create_prmem_sysfs_phys_dbs(prmem_kobj);
	create_prmem_sysfs_pools(prmem_kobj);
	create_prmem_sysfs_caches(prmem_kobj);
	return 0;
err2:
	kfree(prmem_kobj);
	prmem_kobj = NULL;
err1:
	pr_err("Error creating sysfs entries");
	return 0;
}
late_initcall(create_prmem_sysfs_entries);
