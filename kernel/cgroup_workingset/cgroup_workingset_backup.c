/*
 * cgroup_workingset_backup.c
 *
 * The backup part of control group workingset subsystem
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "cgroup_workingset_internal.h"

#include <securec.h>

#include <crypto/hash.h>
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

static spinlock_t g_record_list_lock;
static LIST_HEAD(g_record_list);
static unsigned int g_record_cnt;
static unsigned int g_max_records_count = MAX_RECORD_COUNT_ON_1G;
static struct crypto_shash *g_tfm;

void workingset_record_list_init(void)
{
	if (totalram_pages > TOTAL_RAM_PAGES_3G)
		g_max_records_count = MAX_RECORD_COUNT_ON_4G;
	else if (totalram_pages > TOTAL_RAM_PAGES_2G)
		g_max_records_count = MAX_RECORD_COUNT_ON_3G;
	else if (totalram_pages > TOTAL_RAM_PAGES_1G)
		g_max_records_count = MAX_RECORD_COUNT_ON_2G;
	else
		g_max_records_count = MAX_RECORD_COUNT_ON_1G;

	spin_lock_init(&g_record_list_lock);
}

void workingset_insert_record_to_list_head(struct s_ws_record *record)
{
	spin_lock(&g_record_list_lock);
	g_record_cnt++;
	list_add(&record->list, &g_record_list);
	spin_unlock(&g_record_list_lock);
}

void workingset_insert_record_to_list_tail(struct s_ws_record *record)
{
	spin_lock(&g_record_list_lock);
	g_record_cnt++;
	list_add_tail(&record->list, &g_record_list);
	spin_unlock(&g_record_list_lock);
}

int workingset_crc32_init(void)
{
	if (g_tfm != NULL)
		return 0;

	g_tfm = crypto_alloc_shash("crc32c", 0, 0);
	return PTR_ERR_OR_ZERO(g_tfm);
}

void workingset_crc32_deinit(void)
{
	if (g_tfm != NULL) {
		crypto_free_shash(g_tfm);
		g_tfm = NULL;
	}
}

unsigned int workingset_crc32c(
	unsigned int crc, const void *address, unsigned int length)
{
	SHASH_DESC_ON_STACK(shash, g_tfm);
	unsigned int *ctx = (u32 *)shash_desc_ctx(shash);
	unsigned int retval;
	int err;

	shash->tfm = g_tfm;
	shash->flags = 0;
	*ctx = crc;

	err = crypto_shash_update(shash, address, length);
	if (err) {
		pr_err("%s, %d, err=%d\n", __func__, __LINE__, err);
		retval = crc;
	} else {
		retval = *ctx;
	}
	barrier_data(ctx);

	return retval;
}

static void workingset_recycle_record_rcrdlocked(struct s_ws_record *record)
{
	unsigned int idx;

	/*
	 * Free file_array only because use one vmalloc
	 * for file_array and cacheseq.
	 */
	if (record->data.file_array && (record->state &
	    E_RECORD_STATE_EXTERNAL_FILEPATH)) {
		for (idx = 0; idx < record->data.file_cnt; idx++) {
			kfree(record->data.file_array[idx].path);
			record->data.file_array[idx].path = NULL;
		}
	}
	record->data.file_cnt = 0;
	record->data.pageseq_cnt = 0;
	record->data.page_sum = 0;
	record->leader_blkio_cnt = 0;
	record->need_update = 0;
	record->is_af_cleared = 0;
	record->state = 0;
}

/*
 * workingset_get_record_header -
 * Get the header data of record from backup file.
 * @filp: the struct file pointer of opened file.
 * @offset: the start position of data compute checksum.
 * @header: the pointer of header of record.
 */
static int workingset_get_record_header(
	struct file *filp, size_t offset,
	struct s_ws_backup_record_header *header)
{
	int ret = 0;
	int length;
#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	loff_t pos = 0;

	length = kernel_read(filp, header, sizeof(*header), &pos);
#else
	length = kernel_read(filp, 0, header, sizeof(*header));
#endif
	if (length != sizeof(*header)) {
		pr_err("%s line %d: kernel_read failed, len = %d\n",
			__func__, __LINE__, length);
		ret = -EIO;
		goto out;
	}

	if ((header->magic != WORKINGSET_RECORD_MAGIC) ||
	    (header->record_version != CGROUP_WORKINGSET_VERSION) ||
	    (workingset_crc32c(0, &header->record_version,
	    sizeof(*header) - offset) != header->header_crc)) {
		pr_err("%s line %d: magic=%u, headercrc=%u\n",
			__func__, __LINE__, header->magic, header->header_crc);
		ret = -EIO;
	}

out:
	return ret;
}

static int workingset_record_writeback_data(
	struct file *filp, const void *data, unsigned int size,
	unsigned int *checksum, loff_t *pos)
{
	ssize_t writed_len;
	unsigned int input = *checksum;
	unsigned int output;

	output = workingset_crc32c(input, data, size);
	if (output == input) {
		pr_err("%s: crc failed! val=%u\n", __func__, output);
		return -EINVAL;
	}

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	writed_len = kernel_write(filp, data, size, *pos);
#else
	writed_len = kernel_write(filp, data, size, pos);
#endif
	if (size != writed_len) {
		pr_err("%s: write failed! err=%ld\n", __func__, writed_len);
		return -EIO;
	}

	*checksum = output;
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	*pos += writed_len;
#endif
	return 0;
}

/*
 * workingset_record_writeback_playload_rcrdlocked -
 * Writeback the playlod data of record.
 * @filp: the struct file pointer of opened file.
 * @record: the record need to writeback.
 * @pplayload_length: the pointer of length of playload data.
 * @pchecksum: the pointer of checksum of playload data.
 *
 * You need hold the lock of record before call this function.
 */
static int workingset_record_writeback_playload_rcrdlocked(
	struct file *filp, struct s_ws_record *record,
	unsigned int *pplayload_length, unsigned int *pchecksum)
{
	int ret;
	unsigned int length;
	unsigned int idx;
	loff_t pos = sizeof(struct s_ws_backup_record_header);

	*pchecksum = 0;
	ret = workingset_record_writeback_data(filp, record->data.file_array,
	    sizeof(struct s_path_node) * record->data.file_cnt,
	    pchecksum, &pos);
	if (ret)
		return ret;

	for (idx = 0; idx < record->data.file_cnt; idx++) {
		if (!record->data.file_array[idx].path)
			continue;

		length = (record->data.file_array[idx].pathlen ?
		    (record->data.file_array[idx].pathlen + 1) :
		    (strlen(record->data.file_array[idx].path) + 1));
		ret = workingset_record_writeback_data(filp,
		    record->data.file_array[idx].path,
		    length, pchecksum, &pos);
		if (ret)
			return ret;
	}

	ret = workingset_record_writeback_data(filp, record->data.cacheseq,
	    sizeof(unsigned int) * record->data.pageseq_cnt, pchecksum, &pos);
	if (ret)
		return ret;

	/* Truncate invalid data if it is existed. */
	if (vfs_truncate(&filp->f_path, pos))
		pr_warn("%s %s vfs_truncate failed!\n",
			__func__, record->owner.record_path);

	*pplayload_length = pos - sizeof(struct s_ws_backup_record_header);
	return 0;
}

static bool workingset_record_writeback_rcrdlocked(
	struct s_ws_record *record, struct file *filp)
{
	unsigned int crc_val;
	ssize_t writed_len;
	struct s_ws_backup_record_header header = {0};
	size_t offset =
	    offsetof(struct s_ws_backup_record_header, record_version);
#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	loff_t pos = 0;
#endif

	if (record->state & E_RECORD_STATE_UPDATE_HEADER_ONLY) {
		/* In the case, we update the header of record only. */
		if (workingset_get_record_header(filp, offset, &header))
			return false;
	} else {
		/* We write back the playload of record first. */
		if (workingset_record_writeback_playload_rcrdlocked(filp,
		    record, &header.playload_length,
		    &header.playload_checksum))
			return false;
		header.file_cnt = record->data.file_cnt;
		header.pageseq_cnt = record->data.pageseq_cnt;
		header.page_sum = record->data.page_sum;
		header.record_version = CGROUP_WORKINGSET_VERSION;
	}
	header.leader_blkio_cnt = record->leader_blkio_cnt;
	header.need_update = record->need_update;
	header.is_af_cleared = record->is_af_cleared;

	/* The last, we write back the playload of record. */
	crc_val = workingset_crc32c(0,
	    &header.record_version, sizeof(header) - offset);
	if (!crc_val) {
		pr_err("%s: checksum=0 crc_val=%u\n", __func__, crc_val);
		return false;
	}

	header.header_crc = crc_val;
	header.magic = WORKINGSET_RECORD_MAGIC;
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	writed_len = kernel_write(filp, &header, sizeof(header), 0);
#else
	writed_len = kernel_write(filp, &header, sizeof(header), &pos);
#endif
	if (writed_len != sizeof(header)) {
		pr_err("%s: write header err=%ld\n", __func__, writed_len);
		return false;
	}

	return true;
}

/*
 * workingset_backup_record_rcrdlocked - Writeback the data of record
 * @record: the record need to writeback.
 *
 * You need hold the lock of record before call this function.
 */
static bool workingset_backup_record_rcrdlocked(struct s_ws_record *record)
{
	bool ret = false;
	struct file *filp = NULL;

	if (!record->data.file_cnt || !record->data.pageseq_cnt ||
	    !record->owner.record_path)
		return ret;

	ws_dbg("%s: writeback %s record data to %s\n",
		__func__, record->owner.name, record->owner.record_path);
	filp = filp_open(record->owner.record_path,
	    O_LARGEFILE | O_RDWR, S_IRWUSR);
	if (IS_ERR_OR_NULL(filp)) {
		ws_dbg("%s: open %s, ret = %ld\n", __func__,
			record->owner.record_path, PTR_ERR(filp));
		return ret;
	}

	ret = workingset_record_writeback_rcrdlocked(record, filp);
	filp_close(filp, NULL);
	return ret;
}

/*
 * workingset_record_realloc_ownerbuffer_if_need -
 * Realloc memory for information of new owner .
 * @scanned: the old owner be replaced.
 * @owner: the new owner.
 *
 * If the size of @scanned owner information is larger than
 * @owner requests, reuse the memory of @scanned.
 */
static int workingset_record_realloc_ownerbuffer_if_need(
	struct s_ws_owner *scanned, const struct s_ws_owner *owner)
{
	unsigned int name_len;
	unsigned int path_len;
	char *new_name = NULL;
	char *new_path = NULL;

	if (!owner->name || !owner->record_path)
		return -EINVAL;

	name_len = strlen(owner->name);
	path_len = strlen(owner->record_path);
	if (!scanned->name || (strlen(scanned->name) < name_len)) {
		new_name = kzalloc(name_len + 1, GFP_NOFS);
		if (!new_name)
			return -ENOMEM;
	}
	if (!scanned->record_path ||
	    (strlen(scanned->record_path) < path_len)) {
		new_path = kzalloc(path_len + 1, GFP_NOFS);
		if (!new_path) {
			kfree(new_name); /*lint !e668*/
			return -ENOMEM;
		}
	}

	if (new_name) {
		kfree(scanned->name);
		scanned->name = new_name;
	}
	if (new_path) {
		kfree(scanned->record_path);
		scanned->record_path = new_path;
	}
	return 0;
}

/*
 * workingset_writeback_last_record_if_need -
 * Try to writeback the last record by lru .
 *
 * We consider the writeback is allways successful,
 * so clear the dirty flag of record.
 */
void workingset_writeback_last_record_if_need(void)
{
	struct s_ws_record *record = NULL;

	if (g_record_cnt >= g_max_records_count) {
		spin_lock(&g_record_list_lock);
		record = list_empty(&g_record_list) ? NULL :
		    list_last_entry(&g_record_list, struct s_ws_record, list);
		if (record) {
			list_del(&record->list);
			g_record_cnt--;
			spin_unlock(&g_record_list_lock);
			mutex_lock(&record->mutex);
			if ((record->state &
			    (E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY)) ==
			    (E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY)) {
				workingset_backup_record_rcrdlocked(record);
				record->state &= ~(E_RECORD_STATE_DIRTY |
				    E_RECORD_STATE_UPDATE_HEADER_ONLY);
			}
			mutex_unlock(&record->mutex);
			spin_lock(&g_record_list_lock);
			g_record_cnt++;
			list_add_tail(&record->list, &g_record_list);
		}
		spin_unlock(&g_record_list_lock);
	}
}

/*
 * workingset_writeback_all_records -
 * Writeback any cached records if they are dirty.
 */
void workingset_writeback_all_records(void)
{
	LIST_HEAD(temp_list);
	struct s_ws_record *record = NULL;
	struct list_head *pos = NULL;
	struct list_head *head = &g_record_list;
	int total_records = 0;
	int writeback_cnt = 0;

	spin_lock(&g_record_list_lock);
	while (!list_empty(head)) {
		pos = head->prev;
		list_del(pos);
		g_record_cnt--;
		spin_unlock(&g_record_list_lock);
		record = container_of(pos, struct s_ws_record, list);
		mutex_lock(&record->mutex);
		if ((record->state &
		    (E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY)) ==
		    (E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY)) {
			if (workingset_backup_record_rcrdlocked(record))
				writeback_cnt++;
			record->state &= ~(E_RECORD_STATE_DIRTY |
			    E_RECORD_STATE_UPDATE_HEADER_ONLY);
		}
		total_records++;
		list_add(pos, &temp_list);
		mutex_unlock(&record->mutex);
		spin_lock(&g_record_list_lock);
	}
	list_splice(&temp_list, &g_record_list);
	g_record_cnt += total_records;
	spin_unlock(&g_record_list_lock);
	ws_dbg("%s: total records=%u, writebacked=%d\n",
		__func__, total_records, writeback_cnt);
}

/*
 * workingset_get_playload_addr_rcrdlocked -
 * Get the playlod address of record and reset record.
 * @owner: the struct file pointer of opened file.
 * @record: the record need to reset.
 * @is_exist: indicate there is or not a record of the owner already.
 * @page_array: the pointer of page array of record.
 * @playload_pages: the count of pages.
 *
 * You need hold the lock of record before call this function.
 */
static unsigned int *workingset_get_playload_addr_rcrdlocked(
	const struct s_ws_owner *owner, struct s_ws_record *record,
	bool is_exist, struct page **page_array,
	unsigned int playload_pages)
{
	unsigned int *playload;

	/*lint -e446*/
	playload = vmap(page_array, playload_pages, VM_MAP, PAGE_KERNEL);
	/*lint +e446*/
	if (!playload) {
		pr_err("%s: out of space, vmap %u pages failed!\n",
			__func__, playload_pages);
		return NULL;
	}

	/* We don't need realloc memory when record of the owner is exist. */
	if (!is_exist && workingset_record_realloc_ownerbuffer_if_need(
	    &record->owner, owner)) {
		vunmap(playload);
		return NULL;
	}
	workingset_recycle_record_rcrdlocked(record);

	if (memset_s(playload, playload_pages * PAGE_SIZE,
	    0, playload_pages * PAGE_SIZE)) {
		vunmap(playload);
		return NULL;
	}
	return playload;
}

/*
 * workingset_get_available_record -
 * Prefer to select a clean record by lru.
 */
struct s_ws_record *workingset_get_available_record(void)
{
	struct s_ws_record *record = NULL;
	struct list_head *pos = NULL;

	/*
	 * If record is not existed, we replace oldest
	 * clean record in list.
	 */
	spin_lock(&g_record_list_lock);
	list_for_each_prev(pos, &g_record_list) {
		record = container_of(pos, struct s_ws_record, list);
		if (!(record->state & (E_RECORD_STATE_DIRTY |
		    E_RECORD_STATE_PREREADING)))
			break;
	}
	if (pos == &g_record_list)
		record = NULL;

	if (record) {
		list_del(&record->list);
		g_record_cnt--;
		spin_unlock(&g_record_list_lock);
	} else {
		spin_unlock(&g_record_list_lock);
		record = kzalloc(sizeof(struct s_ws_record), GFP_NOFS);
		if (record)
			mutex_init(&record->mutex);
	}

	return record;
}

static int workingset_get_record_header_wrapper(
	struct file *filp, struct s_ws_backup_record_header *header)
{
	int ret;
	size_t offset;

	offset = offsetof(struct s_ws_backup_record_header, record_version);
	ret = workingset_get_record_header(filp, offset, header);
	if (ret)
		goto out;

	if (header->playload_length >
	    (MAX_TOUCHED_PAGES_COUNT * sizeof(unsigned int)) +
	    (sizeof(struct s_path_node) + PATH_MAX_CHAR) *
	    MAX_TOUCHED_FILES_COUNT) {
		pr_err("%s line %d: playload(%u) is large than limit(%llu)\n",
			__func__, __LINE__, header->playload_length,
			(MAX_TOUCHED_PAGES_COUNT * sizeof(unsigned int)));
		ret = -EIO;
	}

out:
	return ret;
}

/*
 * workingset_prepare_record_pages -
 * Alloc pages for playload data.
 * @data: the collected data who prepare memory for.
 * @playload_pages: the count of pages to alloc.
 */
static struct page **workingset_prepare_record_pages(
	struct s_ws_data *data, unsigned int playload_pages, unsigned int *pidx)
{
	unsigned int idx;
	struct page *page = NULL;
	struct page **page_array = NULL;

	page_array = kcalloc(playload_pages, sizeof(struct page *), GFP_NOFS);
	if (!page_array)
		return NULL;

	if (data->array_page_cnt) {
		int ret;
		size_t cpy_size;

		cpy_size = sizeof(struct page *) * data->array_page_cnt;
		ret = memcpy_s(page_array, cpy_size, data->page_array,
		    cpy_size);
		if (ret) {
			pr_err("%s Line%d,ret=%d\n", __func__, __LINE__, ret);
			goto copy_fail;
		}
	}

	idx = data->array_page_cnt;
	while (idx < playload_pages) {
		page = alloc_page(GFP_NOFS);
		if (!page) {
			pr_err("%s: OOM, alloc %u pages failed!\n",
				__func__, playload_pages);
			goto alloc_fail;
		}
		page_array[idx] = page;
		idx++;
	}
	*pidx = idx;
	return page_array;

alloc_fail:
	while (idx-- > data->array_page_cnt)
		__free_page(page_array[idx]);
copy_fail:
	kfree(page_array);
	return NULL;
}

int workingset_prepare_record_space_wsrcrdlocked(
	const struct s_ws_owner *owner, struct s_ws_record *record,
	bool is_exist, unsigned int playload_size, unsigned int **pplayload)
{
	struct s_ws_data *data = &record->data;
	unsigned int *playload = NULL;
	unsigned int playload_pages;
	unsigned int idx = 0;

	if (!playload_size || (!data->page_array && data->array_page_cnt) ||
	    (data->page_array && !data->array_page_cnt))
		return -EINVAL;

	playload_pages = DIV_ROUND_UP(playload_size, PAGE_SIZE);
	/*
	 * In order to avoid more direct reclaim, so we reuse the memory
	 * of old record as far as possible when replace it.
	 */
	if (data->array_page_cnt < playload_pages) {
		struct page **page_array = workingset_prepare_record_pages(
		    data, playload_pages, &idx);

		if (!page_array)
			return -ENOMEM;

		playload = workingset_get_playload_addr_rcrdlocked(owner,
		    record, is_exist, page_array, playload_pages);
		if (!playload) {
			while (idx-- > data->array_page_cnt)
				__free_page(page_array[idx]);
			kfree(page_array);
			return -ENOSPC;
		}

		if (data->page_array) {
			/* Unmap old space */
			vunmap(data->file_array);
			kfree(data->page_array);
		}
		data->page_array = page_array;
	} else {
		playload = workingset_get_playload_addr_rcrdlocked(owner,
		    record, is_exist, data->page_array, playload_pages);
		if (!playload)
			return -ENOSPC;

		/* Unmap old space and free unnecessary memory. */
		vunmap(data->file_array);
		idx = data->array_page_cnt;
		while (idx-- > playload_pages) {
			__free_page(data->page_array[idx]);
			data->page_array[idx] = NULL;
		}
	}
	data->array_page_cnt = playload_pages;
	data->file_array = (struct s_path_node *)playload;
	*pplayload = playload;
	return 0;
}

static int workingset_get_record_playload(
	const struct s_ws_owner *owner,
	const struct s_ws_backup_record_header *header,
	struct file *filp, struct s_ws_record *record, unsigned int *playload)
{
	int len;
	int ret;
	loff_t pos = sizeof(*header);

#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	ret = kernel_read(filp, playload, header->playload_length, &pos);
#else
	ret = kernel_read(filp, pos, playload, header->playload_length);
#endif
	if (header->playload_length != ret) {
		pr_err("%s: kernel_read failed! ret=%d\n", __func__, ret);
		return -EINVAL;
	}

	if (header->playload_checksum != workingset_crc32c(0,
	    playload, header->playload_length)) {
		pr_err("%s: workingset_crc32c failed!\n", __func__);
		return -EINVAL;
	}

	len = strlen(owner->name) + 1;
	ret = memcpy_s(record->owner.name, len, owner->name, len);
	if (ret) {
		pr_err("%s Line%d,ret=%d\n", __func__, __LINE__, ret);
		return ret;
	}

	len = strlen(owner->record_path) + 1;
	ret = memcpy_s(record->owner.record_path, len,
	    owner->record_path, len);
	if (ret) {
		pr_err("%s Line%d,ret=%d\n", __func__, __LINE__, ret);
		return ret;
	}

	return 0;
}

static void init_record_with_data(
	const struct s_ws_owner *owner,
	const struct s_ws_backup_record_header *header,
	struct s_ws_record *record, char *playload)
{
	struct s_ws_data *data = &record->data;
	unsigned int idx;
	unsigned int pathnode_size;
	unsigned int len;

	record->state = E_RECORD_STATE_USED;
	record->owner.uid = owner->uid;
	data->file_cnt = header->file_cnt;
	data->pageseq_cnt = header->pageseq_cnt;
	data->page_sum = header->page_sum;
	record->leader_blkio_cnt = header->leader_blkio_cnt;
	record->need_update = header->need_update;
	record->is_af_cleared = header->is_af_cleared;
	pathnode_size = sizeof(struct s_path_node) * header->file_cnt;
	for (idx = 0, len = 0; idx < header->file_cnt; idx++) {
		if (!data->file_array[idx].path)
			continue;
		data->file_array[idx].path =
		    playload + pathnode_size + len;
		len += data->file_array[idx].pathlen + 1;
	}
	data->cacheseq = (unsigned int *)(playload + pathnode_size + len);
}

static struct s_ws_record *workingset_get_record_from_backup(
	const struct s_ws_owner *owner)
{
	int ret;
	struct file *filp = NULL;
	struct s_ws_record *record = NULL;
	struct s_ws_backup_record_header header = {0};
	unsigned int *playload = NULL;

	filp = filp_open(owner->record_path, O_LARGEFILE | O_RDONLY, 0);
	if (IS_ERR_OR_NULL(filp))
		return NULL;

	ws_dbg("%s: read record data from %s\n", __func__, owner->record_path);
	ret = workingset_get_record_header_wrapper(filp, &header);
	if (ret)
		goto out;

	record = workingset_get_available_record();
	if (!record)
		goto out;

	mutex_lock(&record->mutex);
	ret = workingset_prepare_record_space_wsrcrdlocked(owner,
	    record, false, header.playload_length, &playload);
	if (ret)
		goto alloc_space_fail;

	ret = workingset_get_record_playload(owner, &header, filp,
	    record, playload);
	if (ret)
		goto alloc_space_fail;

	init_record_with_data(owner, &header, record, (char *)playload);
	workingset_insert_record_to_list_head(record);
	mutex_unlock(&record->mutex);

	filp_close(filp, NULL);
	ws_dbg("%s: read %s completely!\n", __func__, owner->record_path);
	return record;

alloc_space_fail:
	workingset_insert_record_to_list_tail(record);
	record->state &= ~(E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY |
	    E_RECORD_STATE_UPDATE_HEADER_ONLY);
	mutex_unlock(&record->mutex);
out:
	filp_close(filp, NULL);
	return NULL;
}

/*
 * workingset_get_existed_record_wslocked -
 * Find the record of owner from cache or blockdev
 * @owner: the owner of record that we will be find.
 * @onlycache: don't get record from disk if it is true.
 *
 * We adjust the record to the head of list when we found it.
 */
struct s_ws_record *workingset_get_existed_record_wslocked(
	const struct s_ws_owner *owner, bool onlycache)
{
	struct s_ws_record *record = NULL;
	struct list_head *pos = NULL;
	struct list_head *head = &g_record_list;

	if (!owner->name)
		return NULL;

	spin_lock(&g_record_list_lock);
	list_for_each(pos, head) {
		record = container_of(pos, struct s_ws_record, list);
		if ((record->state & E_RECORD_STATE_USED) &&
		    (record->owner.uid == owner->uid) &&
		    !strcmp(record->owner.name, owner->name)) {
			break;
		}
	}

	if (pos != head) {
		list_move(pos, head);
		spin_unlock(&g_record_list_lock);
	} else if (!onlycache && owner->record_path) {
		spin_unlock(&g_record_list_lock);
		record = workingset_get_record_from_backup(owner);
	} else {
		spin_unlock(&g_record_list_lock);
		record = NULL;
	}

	return record;
}

/*
 * workingset_clear_record -
 * Clear the record of owner from the comming string.
 * @owner_string the comming string.
 *
 */
int workingset_clear_record(char *owner_string)
{
	int ret = 0;
	char *str = owner_string;
	char *token;
	unsigned int len;
	struct s_ws_record *record = NULL;
	struct list_head *pos = NULL;
	struct list_head *head = &g_record_list;

	token = strsep(&str, " ");
	if (token == NULL) {
		ret = -EINVAL;
		goto out;
	}
	len = strlen(token); /* lint !e668 */
	if (len <= 0 || len >= OWNER_MAX_CHAR) {
		ret = -EINVAL;
		goto out;
	}

	spin_lock(&g_record_list_lock);
	list_for_each(pos, head) {
		record = container_of(pos, struct s_ws_record, list);
		if ((record->state & E_RECORD_STATE_USED) &&
		    !strcmp(record->owner.name, token)) {
			record->state &= ~E_RECORD_STATE_USED;
			list_move_tail(pos, head);
			ret = E_RECORD_STATE_USED;
			break;
		}
	}
	spin_unlock(&g_record_list_lock);

	if (ret)
		ws_dbg("%s: invalidate the record of %s successfully!\n",
			__func__, token);
	return 0;

out:
	return ret;
}
