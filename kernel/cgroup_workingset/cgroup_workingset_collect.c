/*
 * cgroup_workingset_collect.c
 *
 * The collect part of control group workingset subsystem
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

#include <linux/file.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

static const char * const g_excluded_dir[] = {
	"/dev/",
	"/bin/",
	"/etc/selinux/",
	NULL
};

static inline unsigned int create_handle(
	unsigned int file_num, unsigned int file_offset)
{
	return (file_num << FILE_OFFSET_BITS) |
	    (file_offset & MAX_TOUCHED_FILE_OFFSET);
}

static struct rb_node *rb_deepest_left_node(const struct rb_node *node)
{
	for (;;) {
		if (node->rb_left)
			node = node->rb_left;
		else
			return (struct rb_node *)node;
	}
}

static struct rb_node *rb_deepest_right_node(const struct rb_node *node)
{
	for (;;) {
		if (node->rb_right)
			node = node->rb_right;
		else
			return (struct rb_node *)node;
	}
}

static struct rb_node *rb_latest_left_ancestor(const struct rb_node *node)
{
	const struct rb_node *parent = NULL;
	const struct rb_node *temp_node = node;

	while (temp_node) {
		parent = rb_parent(temp_node);
		if (parent && temp_node == parent->rb_left)
			temp_node = parent;
		else
			return (struct rb_node *)parent;
	}

	return NULL;
}

static struct rb_node *rb_latest_right_ancestor(const struct rb_node *node)
{
	const struct rb_node *parent = NULL;
	const struct rb_node *temp_node = node;

	while (temp_node) {
		parent = rb_parent(temp_node);
		if (parent && temp_node == parent->rb_right)
			temp_node = parent;
		else
			return (struct rb_node *)parent;
	}

	return NULL;
}

struct rb_node *rb_prev_middleorder(const struct rb_node *node)
{
	if (!node)
		return NULL;

	if (node->rb_left)
		return rb_deepest_right_node(node->rb_left);
	else
		return rb_latest_left_ancestor(node);
}

struct rb_node *rb_next_middleorder(const struct rb_node *node)
{
	if (!node)
		return NULL;

	if (node->rb_right)
		return rb_deepest_left_node(node->rb_right);
	else
		return rb_latest_right_ancestor(node);
}

struct rb_node *rb_first_middleorder(const struct rb_root *root)
{
	if (!root->rb_node)
		return NULL;

	return rb_deepest_left_node(root->rb_node);
}

static void workingset_range_rb_erase(
	struct rb_root *root, struct s_pagecache_info *entry)
{
	if (!RB_EMPTY_NODE(&entry->rbnode)) {
		rb_erase(&entry->rbnode, root);
		RB_CLEAR_NODE(&entry->rbnode);
	}
}

static void workingset_range_rb_change_to_front(
	struct s_pagecache_info **old_entry,
	struct s_pagecache_info **new_entry)
{
	if (*old_entry < *new_entry) {
		struct s_pagecache_info *temp = *old_entry;

		*old_entry = *new_entry;
		*new_entry = temp;
	}
}

static void workingset_merge_ranges(
	struct rb_root *root, struct s_pagecache_info **pp_entry,
	struct s_pagecache_info **pp_merged_entry, bool probe, int *delta)
{
	if (probe && *pp_merged_entry) {
		workingset_range_rb_change_to_front(pp_entry, pp_merged_entry);
		workingset_range_rb_erase(root, *pp_entry);
		*delta -= ((*pp_entry)->offset_range.start &
		    PAGE_RANGE_HEAD_BIT_MASK) ? MULTI_PAGES_SEQ_WORDS :
		    SINGLE_PAGE_SEQ_WORDS;
		(*pp_entry)->offset_range.start = PAGECACHE_INVALID_OFFSET;
		(*pp_entry)->offset_range.end = PAGECACHE_INVALID_OFFSET;
	}
}

/*
 * Set 1 to the range header bit if the range has multipages,
 * or clear the range header bit.
 */
static int workingset_range_stick_flags(
	struct s_range *range, unsigned int start, unsigned int end,
	int major, int stage)
{
	int delta = 0;

	delta -= (range->start & PAGE_RANGE_HEAD_BIT_MASK) ?
	    MULTI_PAGES_SEQ_WORDS : SINGLE_PAGE_SEQ_WORDS;
	range->start = start |
	    ((major & PAGE_MAJOR_MASK) << PAGE_MAJOR_SHIFT) |
	    ((stage & PAGE_STAGE_NUM_MASK) << PAGE_STAGE_NUM_SHIFT);
	range->end = end  |
	    ((major & PAGE_MAJOR_MASK) << PAGE_MAJOR_SHIFT) |
	    ((stage & PAGE_STAGE_NUM_MASK) << PAGE_STAGE_NUM_SHIFT);
	if (end - start > 1) {
		delta += MULTI_PAGES_SEQ_WORDS;
		range->start |= PAGE_RANGE_HEAD_BIT_MASK;
		range->end |= PAGE_RANGE_HEAD_BIT_MASK;
	} else {
		delta += SINGLE_PAGE_SEQ_WORDS;
	}
	return delta;
}

static void workingset_rbtree_probe_left(
	struct s_probe_context *pc, unsigned int cur_start)
{
	if (!pc->merged_entry || pc->probe_left) {
		if (pc->merged_entry)
			pc->look_side = true;
		if (pc->start < cur_start) {
			/*
			 * Inserted entry or merged entry including
			 * current offset range.
			 */
			pc->parent = rb_prev_middleorder(
			    &pc->cur_entry->rbnode);
			workingset_merge_ranges(
				pc->root, &pc->cur_entry, &pc->merged_entry,
				pc->probe_left, &pc->delta);
			/*
			 * Probe left tree if there are smaller
			 * offset ranges.
			 */
			if (!pc->parent)
				pc->probe_left = false;
			else
				pc->probe_left = true;
		} else {
			/*
			 * Inserted entry or merged entry overlapped
			 * with current offset range.
			 */
			workingset_merge_ranges(
				pc->root, &pc->cur_entry, &pc->merged_entry,
				pc->probe_left, &pc->delta);
			pc->start = cur_start;
			pc->probe_left = false;
		}
	}
}

static void workingset_rbtree_probe_right(
	struct s_probe_context *pc, unsigned int cur_end)
{
	if (!pc->merged_entry || (pc->probe_right && !pc->probe_left)) {
		if (pc->look_side && pc->merged_entry) {
			/*
			 * There aren't any small offset range,
			 * so we look aside the bigger.
			 */
			pc->look_side = false;
			pc->parent = rb_next_middleorder(
			    &pc->merged_entry->rbnode);
			if (!pc->parent)
				pc->probe_right = false;
		} else if (pc->end > cur_end) {
			/*
			 * Inserted entry or merged entry including
			 * current offset range.
			 */
			if (!pc->probe_left)
				pc->parent = rb_next_middleorder(
				    &pc->cur_entry->rbnode);
			workingset_merge_ranges(
				pc->root, &pc->cur_entry, &pc->merged_entry,
				pc->probe_right, &pc->delta);
			/*
			 * Stop probing right tree if there are not
			 * any bigger offset ranges.
			 */
			if (!pc->parent)
				pc->probe_right = false;
			else
				pc->probe_right = true;
		} else {
			/*
			 * Inserted entry or merged entry overlapped
			 * with current offset range.
			 */
			workingset_merge_ranges(
				pc->root, &pc->cur_entry, &pc->merged_entry,
				pc->probe_right, &pc->delta);

			pc->end = cur_end;
			pc->probe_right = false;
		}
	}
}

static unsigned int workingset_get_repeate_pages_count(
	unsigned int start, unsigned int end,
	unsigned int cur_start, unsigned int cur_end)
{
	unsigned int nr_repeated;

	if ((start <= cur_start) && (end > cur_start)) {
		if (end >= cur_end)
			nr_repeated = cur_end - cur_start;
		else
			nr_repeated = end - cur_start;
	} else if ((start > cur_start) && (start < cur_end)) {
		if (end <= cur_end)
			nr_repeated = end - start;
		else
			nr_repeated = cur_end - start;
	} else {
		nr_repeated = 0;
	}
	return nr_repeated;
}

static void workingset_rb_insert_looper(struct s_probe_context *pc)
{
	struct s_range *range = NULL;
	unsigned int cur_start;
	unsigned int cur_end;
	unsigned int stage;

	/* Range[start, end) */
	while (*pc->link) {
		pc->parent = *pc->link;
		pc->cur_entry = rb_entry(
		    pc->parent, struct s_pagecache_info, rbnode);
		range = &pc->cur_entry->offset_range;
		cur_start = range->start & FILE_IDX_AND_OFFSET_MASK;
		cur_end = range->end & FILE_IDX_AND_OFFSET_MASK;

		if (cur_start > pc->end) {
			pc->link = &(*pc->link)->rb_left;
		} else if (cur_end < pc->start) {
			pc->link = &(*pc->link)->rb_right;
		} else {
			/*
			 * In the case, two ranges is overlapped or adjoined.
			 * Indicate major touched page offset range
			 * even if a page was touched by main thread
			 */
			pc->major |= (range->start >> PAGE_MAJOR_SHIFT) &
			    PAGE_MAJOR_MASK;
			stage = (range->start >> PAGE_STAGE_NUM_SHIFT) &
			    PAGE_STAGE_NUM_MASK;
			if (stage < pc->stage)
				pc->stage = stage;

			pc->repeat += workingset_get_repeate_pages_count(
			    pc->start, pc->end, cur_start, cur_end);
			/*
			 * We probe left child tree first, and merge overlapped
			 * range or adjoined range, then probe right child
			 * tree.
			 * Exchange the position between inserted range with
			 * adjoined range in order to preread these file pages
			 * as early as possible. and dicard the space of erased
			 * pagecache range in pagecache array.
			 */
			workingset_rbtree_probe_left(pc, cur_start);

			/*
			 * In the case, merge range first time
			 * or there are not any small offset range.
			 */
			workingset_rbtree_probe_right(pc, cur_end);

			if (!pc->merged_entry)
				pc->merged_entry = pc->cur_entry;

			if (!pc->probe_right && !pc->probe_left)
				break;
			pc->link = &pc->parent;
			continue;
		}

		if (!pc->merged_entry)
			continue;
		/*
		 * There are not any small offset range,
		 * so we look aside bigger offset range.
		 */
		if (pc->probe_left && pc->probe_right) {
			pc->probe_left = false;
			pc->parent = rb_next_middleorder(
			    &pc->merged_entry->rbnode);
			if (pc->parent) {
				pc->link = &pc->parent;
				continue;
			}
		}
		break;
	}
}

/*
 * workingset_range_rb_Insert -
 * Insert a page offset range into pageoffset range tree of a file.
 * @root: the root of page range tree
 * @entry: a entry of page offset range.
 * @repeat_cnt: output the count of overlaped page offset.
 * @seq_delta: output the count of added page offset range.
 *
 * Returns true when the inserted range has not overlapped
 * or adjoined with any ranges, or return false.
 */
static bool workingset_range_rb_insert(
	struct rb_root *root, struct s_pagecache_info *entry,
	struct s_cachepage_info *info, unsigned int *repeat_cnt,
	int *seq_delta)
{
	struct s_probe_context pc = {
		.root = root,
		.link = &root->rb_node,
		.parent = NULL,
		.merged_entry = NULL,
		.cur_entry = NULL,
		.repeat = 0,
		.probe_left = false,
		.probe_right = false,
		.look_side = false,
		.delta = 0,
		.start = entry->offset_range.start & FILE_IDX_AND_OFFSET_MASK,
		.end = entry->offset_range.end & FILE_IDX_AND_OFFSET_MASK,
		.stage = info->stage,
		.major = info->is_major};

	workingset_rb_insert_looper(&pc);

	if (pc.merged_entry) {
		pc.delta += workingset_range_stick_flags(
		    &pc.merged_entry->offset_range,
		    pc.start, pc.end, pc.major, pc.stage);
		*repeat_cnt = pc.repeat;
		*seq_delta = pc.delta;
		return true;
	}

	/* The inserted range has not overlapped or adjoined with any ranges */
	if (pc.major) {
		entry->offset_range.start |= (pc.major & PAGE_MAJOR_MASK) <<
		    PAGE_MAJOR_SHIFT;
		entry->offset_range.end |= (pc.major & PAGE_MAJOR_MASK) <<
		    PAGE_MAJOR_SHIFT;
	}
	if (pc.stage) {
		entry->offset_range.start |=
		    (pc.stage & PAGE_STAGE_NUM_MASK) << PAGE_STAGE_NUM_SHIFT;
		entry->offset_range.end |=
		    (pc.stage & PAGE_STAGE_NUM_MASK) << PAGE_STAGE_NUM_SHIFT;
	}
	rb_link_node(&entry->rbnode, pc.parent, pc.link);
	rb_insert_color(&entry->rbnode, root);

	return false;
}

/*lint -e454*/
/*lint -e456*/
void workingset_destroy_data(struct s_workingset *ws, bool is_locked)
{
	struct list_head *head = NULL;
	struct s_file_info *fileinfo = NULL;
	struct s_filp_list *curr = NULL;
	struct s_filp_list *next = NULL;

	if (!is_locked)
		mutex_lock(&ws->mutex);

	head = &ws->file_list;
	while (!list_empty(head)) {
		fileinfo = list_first_entry(head, struct s_file_info, list);
		list_del(&fileinfo->list);
		fileinfo->rbroot = RB_ROOT;
		kfree(fileinfo->path_node.path);

		if (!fileinfo->filp_list) {
			kfree(fileinfo);
			continue;
		}

		curr = fileinfo->filp_list;
		do {
			next = curr->next;
			if (curr->filp)
				fput(curr->filp);
			kfree(curr);
			curr = next;
		} while (curr);
		kfree(fileinfo);
	}
	kfree(ws->owner.name);
	kfree(ws->owner.record_path);

	ws->owner.uid = 0;
	ws->owner.pid = 0;
	ws->owner.name = NULL;
	ws->owner.record_path = NULL;
	ws->repeated_count = 0;
	ws->page_sum = 0;
	ws->stage_num = 0;
	ws->leader_blkio_cnt = 0;
	ws->leader_blkio_base = 0;
	ws->file_count = 0;
	ws->pageseq_count = 0;
	ws->alloc_index = 0;
	ws->clear_young = false;
	if (!is_locked)
		mutex_unlock(&ws->mutex);
}
/*lint +e456*/
/*lint +e454*/

/*
 * workingset_pagecache_info_cache_alloc_wslocked -
 * Alloc a page_cache space from the page_cache array of workingset.
 * @workingset: the owner of page_cache array
 *
 * Return the pointer of a free page_cache space or null.
 */
static struct s_pagecache_info *workingset_pagecache_info_cache_alloc_wslocked(
	struct s_workingset *ws)
{
	unsigned int page_idx = ws->alloc_index / PAGECACHEINFO_PER_PAGE;
	unsigned int off_in_page = ws->alloc_index % PAGECACHEINFO_PER_PAGE;

	/* The size of struct s_workingset space is PAGE_SIZE,
	 * including essentials and pages array.
	 */
	if (offsetof(struct s_workingset, cache_pages) +
	    (page_idx + 1) * sizeof(struct s_pagecache_info **) > PAGE_SIZE)
		goto out;

	if (!ws->cache_pages[page_idx]) {
		ws->cache_pages[page_idx] =
		    (struct s_pagecache_info *)get_zeroed_page(GFP_NOFS);
		if (!ws->cache_pages[page_idx])
			goto out;
	}

	return ws->cache_pages[page_idx] + off_in_page;

out:
	return NULL;
}

static bool path_is_under_dirs(const char *path, const char * const dir[])
{
	int i = 0;

	while (dir[i] != NULL) {
		if (strncmp(path, dir[i], strlen(dir[i])) == 0)
			return true;
		i++;
	}
	return false;
}

static unsigned int get_file_path_and_hashcode(
	struct file *file, char *buf, unsigned int buf_size,
	char **str_path, unsigned int *hashcode)
{
	unsigned int path_len;
	char *filepath;

	filepath = d_path(&file->f_path, buf, buf_size - 1);
	if (IS_ERR_OR_NULL(filepath))
		return 0;

	if (path_is_under_dirs(filepath, g_excluded_dir))
		return 0;

	path_len = strlen(filepath);
	*str_path = filepath;
	*hashcode = workingset_crc32c(0, filepath, path_len);
	return path_len;
}

static struct s_file_info *workingset_alloc_fileinfo(
	unsigned int path_len, gfp_t gfp_mask)
{
	struct s_file_info *fileinfo;

	fileinfo = kzalloc(sizeof(struct s_file_info), gfp_mask);
	if (!fileinfo)
		goto out;

	fileinfo->path_node.path = kzalloc(path_len + 1, gfp_mask);
	if (!fileinfo->path_node.path)
		goto fileinfo_free;

	fileinfo->filp_list = kzalloc(sizeof(struct s_filp_list), gfp_mask);
	if (!fileinfo->filp_list)
		goto filepath_free;
	return fileinfo;

filepath_free:
	kfree(fileinfo->path_node.path);
fileinfo_free:
	kfree(fileinfo);
out:
	return NULL;
}

static struct s_file_info *workingset_record_new_file_tree(
	struct s_workingset *ws, struct file *file, const char *filepath,
	const unsigned int hashcode, const unsigned int path_len)
{
	int ret;
	struct kstat stat;
	struct s_file_info *info = NULL;

#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	ret = vfs_getattr_nosec(&file->f_path, &stat, STATX_UID,
	    KSTAT_QUERY_FLAGS);
#else
	ret = vfs_getattr_nosec(&file->f_path, &stat);
#endif
	if (ret) {
		pr_err("%s, vfs_getattr failed! err=%d\n", __func__, ret);
		return NULL;
	}

	info = workingset_alloc_fileinfo(path_len, GFP_NOFS);
	if (!info) {
		pr_err("%s, oom, alloc info failed!\n", __func__);
		return NULL;
	}
	ret = strncpy_s(info->path_node.path, path_len + 1,
	    filepath, path_len);
	if (ret) {
		pr_err("%s Line%d,ret=%d\n", __func__, __LINE__, ret);
		kfree(info->filp_list);
		kfree(info->path_node.path);
		kfree(info);
		return NULL;
	}
	info->filp_list->filp = file;
	info->path_node.owner_uid = stat.uid.val;
	info->path_node.hashcode = hashcode;
	info->path_node.pathlen = path_len;
	info->pageseq_count = 0;
	info->rbroot = RB_ROOT;
	list_add_tail(&info->list, &ws->file_list);
	ws->file_count++;
#ifdef CONFIG_HW_CGROUP_WORKINGSET_DEBUG
	pr_info("%s, including %s\n", __func__, filepath);
#endif
	return info;
}

static int get_file_with_same_inode(
	const struct s_workingset *ws, struct file *file,
	struct s_file_info **file_info, bool *is_existed)
{
	int seq_num = 0;
	struct list_head *pos = NULL;

	list_for_each(pos, &ws->file_list) {
		struct s_file_info *info =
		    container_of(pos, struct s_file_info, list);
		struct s_filp_list *filp_list = info->filp_list;

		while (filp_list) {
			if (filp_list->filp->f_mapping->host ==
			    file->f_mapping->host) {
				*file_info = info;
				*is_existed = true;
				return seq_num;
			}
			filp_list = filp_list->next;
		}
		seq_num++;
	}

	return -ENOENT;
}

static int get_file_with_same_path(
	struct s_workingset *ws, const struct s_find_file_by_path *ffbp,
	struct s_file_info **file_info, bool *is_existed, int *existed_files)
{
	int seq_num = 0;
	struct list_head *pos = NULL;

	list_for_each(pos, &ws->file_list) {
		struct s_file_info *info =
		    container_of(pos, struct s_file_info, list);

		if ((info->path_node.hashcode == ffbp->hashcode) &&
		    !strcmp(info->path_node.path, ffbp->filepath)) {
			struct s_filp_list *filp_list = kzalloc(
			    sizeof(struct s_filp_list), GFP_NOFS);

			if (filp_list) {
				struct s_filp_list *temp = info->filp_list;

				while (temp->next)
					temp = temp->next;
				filp_list->filp = ffbp->file;
				temp->next = filp_list;
				*is_existed = false;
			} else {
				*is_existed = true;
			}

			*file_info = info;
			return seq_num;
		}
		seq_num++;
	}

	*existed_files = seq_num;
	return -ENOENT;
}

static int workingset_record_fileinfo_if_need_wslocked(
	struct s_workingset *ws, struct file *file,
	struct s_file_info **file_info, bool *is_existed)
{
	int ret;
	int seq_num = 0;
	unsigned int path_len;
	char *filepath = NULL;
	unsigned int hashcode;
	char buf[PATH_MAX_CHAR] = {'\0'};
	struct s_find_file_by_path ffbp;

	if (ws->pageseq_count >= MAX_TOUCHED_PAGES_COUNT)
		return -ENOSPC;

	/* First, match inode when search same file */
	ret = get_file_with_same_inode(ws, file, file_info, is_existed);
	if (ret >= 0)
		return ret;

	/* Get the path string of file and hashcode of path string. */
	path_len = get_file_path_and_hashcode(file, buf, PATH_MAX_CHAR,
	    &filepath, &hashcode);
	if (!path_len)
		return -EINVAL;

	/* Second, match hashcode and string when search same file */
	ffbp.file = file;
	ffbp.filepath = filepath;
	ffbp.hashcode = hashcode;
	ret = get_file_with_same_path(ws, &ffbp, file_info,
	    is_existed, &seq_num);
	if (ret >= 0)
		return ret;

	if (ws->file_count >= MAX_TOUCHED_FILES_COUNT)
		return -ENOSPC;

	*file_info = workingset_record_new_file_tree(ws, file,
	    filepath, hashcode, path_len);
	if (*file_info == NULL)
		return -EPERM;

	*is_existed = false;
	return seq_num;
}

static void workingset_rbtree_insert_node(
	struct s_workingset *ws, struct s_file_info *file_info,
	struct s_pagecache_info *page_cache, struct s_cachepage_info *info)
{
	unsigned int repeat_count = 0;
	int page_count_delta = 0;

	if (workingset_range_rb_insert(&file_info->rbroot, page_cache,
	    info, &repeat_count, &page_count_delta)) {
		ws->repeated_count += repeat_count;
		ws->page_sum += info->count - repeat_count;
		ws->pageseq_count += page_count_delta;
		file_info->pageseq_count += page_count_delta;
	} else {
		if (info->count > 1) {
			page_cache->offset_range.start |=
			    PAGE_RANGE_HEAD_BIT_MASK;
			page_cache->offset_range.end |=
			    PAGE_RANGE_HEAD_BIT_MASK;
			ws->pageseq_count += MULTI_PAGES_SEQ_WORDS;
			file_info->pageseq_count += MULTI_PAGES_SEQ_WORDS;
		} else {
			ws->pageseq_count += SINGLE_PAGE_SEQ_WORDS;
			file_info->pageseq_count += SINGLE_PAGE_SEQ_WORDS;
		}
		ws->page_sum += info->count;
		ws->alloc_index++;
	}
}

static int workingset_dealwith_pagecache_wslocked(
	struct s_cachepage_info *info, struct s_workingset *ws)
{
	int ret = 0;
	int file_idx;
	bool is_existed_file = false;
	struct s_pagecache_info *page_cache = NULL;
	struct s_file_info *file_info = NULL;

	/* Get position of current file in file list */
	file_idx = workingset_record_fileinfo_if_need_wslocked(ws, info->filp,
	    &file_info, &is_existed_file);
	if (file_idx < 0) {
		ret = file_idx;
		goto done;
	}

	page_cache = workingset_pagecache_info_cache_alloc_wslocked(ws);
	if (!page_cache) {
		ret = -ENOMEM;
		goto done;
	}

	page_cache->offset_range.start =
	    create_handle(file_idx, info->offset);
	page_cache->offset_range.end =
	    create_handle(file_idx, (info->offset + info->count));
	/* Insert page offset range to the range tree of file */
	workingset_rbtree_insert_node(ws, file_info, page_cache, info);

done:
	if (ret || is_existed_file)
		fput(info->filp);

	return ret;
}

static unsigned int workingset_collector_dequeue_buffer_locked(
	struct s_ws_collector *collector, char *buffer, size_t buf_size)
{
	unsigned int buffer_pos;
	unsigned int read_pos;
	unsigned int write_pos;
	unsigned int copy_size;

	read_pos = collector->read_pos;
	write_pos = collector->write_pos;

	if (read_pos > write_pos) {
		/* Write pointer has beed reversed. */
		if (COLLECTOR_CACHE_SIZE - read_pos > buf_size)
			copy_size = buf_size;
		else
			copy_size = COLLECTOR_CACHE_SIZE - read_pos;

		if (memcpy_s(buffer, copy_size,
		    collector->circle_buffer + read_pos, copy_size))
			goto out;

		read_pos += copy_size;
		buffer_pos = copy_size;

		/*
		 * Pick data from the head of circle buffer
		 * when local buffer is not full.
		 */
		if ((copy_size < buf_size) && write_pos) {
			if (write_pos > (buf_size - copy_size))
				copy_size = buf_size - copy_size;
			else
				copy_size = write_pos;

			if (memcpy_s(buffer + buffer_pos, copy_size,
			    collector->circle_buffer, copy_size))
				goto out;

			buffer_pos += copy_size;
			read_pos += copy_size;
		}
	} else {
		if (write_pos - read_pos > buf_size)
			copy_size = buf_size;
		else
			copy_size = write_pos - read_pos;

		if (memcpy_s(buffer, copy_size,
		    collector->circle_buffer + read_pos, copy_size))
			goto out;

		read_pos += copy_size;
		buffer_pos = copy_size;
	}

out:
	collector->read_pos = (read_pos >= COLLECTOR_CACHE_SIZE) ?
	    (read_pos - COLLECTOR_CACHE_SIZE) : read_pos;

	return buffer_pos;
}

/*lint -e454*/
static void workingset_collector_do_collect_locked(
	struct s_ws_collector *collector)
{
	char buffer[COLLECTOR_BATCH_COUNT * sizeof(struct s_cachepage_info)];
	unsigned int buffer_pos;
	unsigned int idx;

	while (collector->read_pos != collector->write_pos) {
		buffer_pos = workingset_collector_dequeue_buffer_locked(
		    collector, buffer, sizeof(buffer));

		for (idx = 0; idx < buffer_pos;
			idx += sizeof(struct s_cachepage_info)) {
			struct s_workingset *ws = collector->monitor;

			if (ws) {
				spin_unlock(&collector->lock); /*lint !e455*/
				mutex_lock(&ws->mutex);
				workingset_dealwith_pagecache_wslocked(
				(struct s_cachepage_info *)(buffer + idx), ws);
				mutex_unlock(&ws->mutex);
				spin_lock(&collector->lock);
			} else {
				return;
			}
		}
	}
}
/*lint +e454*/

static int workingset_collector_fill_record_cacheseq_wsrcrdlocked(
	struct s_ws_data *data, unsigned int *cacheseq_idx,
	struct s_pagecache_info **pagecache_array,
	unsigned int page_idx, unsigned int end_in_page)
{
	int ret = 0;
	unsigned int idx_in_page;
	unsigned int idx = *cacheseq_idx;
	struct s_pagecache_info *page_cache = NULL;

	/*
	 * In order to save memory, we save the range including
	 * single page in one word by cleaned range head bit.
	 */
	for (idx_in_page = 0; idx_in_page < end_in_page; idx_in_page++) {
		page_cache = pagecache_array[page_idx] + idx_in_page;
		if (unlikely(page_cache->offset_range.start ==
		    PAGECACHE_INVALID_OFFSET))
			continue;

		if (idx < data->pageseq_cnt) {
			data->cacheseq[idx++] = page_cache->offset_range.start;
		} else {
			pr_err("%s: idx=%u, cnt=%u never happend!\n",
				__func__, idx, data->pageseq_cnt);
			ret = -EPERM;
			break;
		}

		if ((page_cache->offset_range.start >> PAGE_RANGE_HEAD_SHIFT) &
		    PAGE_RANGE_HEAD_MASK) {
			if (idx < data->pageseq_cnt) {
				data->cacheseq[idx++] =
				    page_cache->offset_range.end;
			} else {
				pr_err("%s: idx=%u, cnt=%u never happend!\n",
					__func__, idx, data->pageseq_cnt);
				ret = -EPERM;
				break;
			}
		}
	}

	*cacheseq_idx = idx;
	return ret;
}

static int workingset_collector_fill_record_filenode_wsrcrdlocked(
	struct s_ws_data *data, struct list_head *head)
{
	int ret;
	unsigned int i;
	unsigned int idx = 0;
	struct list_head *pos = NULL;
	struct s_file_info *fileinfo = NULL;

	list_for_each(pos, head) {
		if (idx >= data->file_cnt)
			break;

		fileinfo = container_of(pos, struct s_file_info, list);
		if (!fileinfo->pageseq_count) {
			ret = memset_s(data->file_array + idx,
			    sizeof(fileinfo->path_node),
			    0, sizeof(fileinfo->path_node));
			if (ret) {
				pr_err("%s Line%d,ret=%d\n",
					__func__, __LINE__, ret);
				goto out;
			}
		} else {
			ret = memcpy_s(data->file_array + idx,
			    sizeof(fileinfo->path_node),
			    &fileinfo->path_node,
			    sizeof(fileinfo->path_node));
			if (ret) {
				pr_err("%s Line%d,ret=%d\n",
					__func__, __LINE__, ret);
				goto out;
			}
			/*
			 * The pointer of path is assigned to path
			 * of record, so don't free it in here.
			 */
			fileinfo->path_node.path = NULL;
		}
		idx++;
	}
	return 0;

out:
	for (i = 0; i < idx; i++) {
		kfree(data->file_array[i].path);
		data->file_array[i].path = NULL;
	}
	return ret;
}

static int workingset_collector_prepare_record_space_wsrcrdlocked(
	struct s_workingset *ws, struct s_ws_record *record, bool is_exist)
{
	int ret;
	struct s_ws_data *data = &record->data;
	unsigned int *playload = NULL;
	unsigned int pathnode_size;
	unsigned int playload_size;

	pathnode_size = sizeof(struct s_path_node) * ws->file_count;
	playload_size = pathnode_size +
	    sizeof(unsigned int) * ws->pageseq_count;
	ret = workingset_prepare_record_space_wsrcrdlocked(&ws->owner,
	    record, is_exist, playload_size, &playload);
	if (!ret)
		data->cacheseq = (unsigned int *)
		    ((char *)playload + pathnode_size);

	return ret;
}

static int workingset_collector_fill_record_owner_wsrcrdlocked(
	struct s_workingset *ws, struct s_ws_record *record)
{
	int ret;
	size_t cpy_size;

	record->owner.uid = ws->owner.uid;
	cpy_size = strlen(ws->owner.name) + 1;
	ret = memcpy_s(record->owner.name, cpy_size, ws->owner.name, cpy_size);
	if (ret) {
		pr_err("%s Line%d,ret=%d\n", __func__, __LINE__, ret);
		return ret;
	}

	cpy_size = strlen(ws->owner.record_path) + 1;
	ret = memcpy_s(record->owner.record_path, cpy_size,
	    ws->owner.record_path, cpy_size);
	if (ret) {
		pr_err("%s Line%d,ret=%d\n", __func__, __LINE__, ret);
		return ret;
	}

	return 0;
}

static int workingset_collector_read_data_wsrcrdlocked(
	struct s_workingset *ws, struct s_ws_record *record, bool is_exist)
{
	int ret;
	struct s_ws_data *data = &record->data;
	unsigned int idx;
	unsigned int page_idx;

	ret = workingset_collector_prepare_record_space_wsrcrdlocked(ws, record,
	    is_exist);
	if (ret)
		goto fill_data_fail;

	if (!is_exist) {
		ret = workingset_collector_fill_record_owner_wsrcrdlocked(
		    ws, record);
		if (ret)
			goto fill_data_fail;
	}
	data->file_cnt = ws->file_count;
	data->pageseq_cnt = ws->pageseq_count;
	data->page_sum = ws->page_sum;
	ret = workingset_collector_fill_record_filenode_wsrcrdlocked(data,
	    &ws->file_list);
	if (ret)
		goto fill_data_fail;

	record->state |= E_RECORD_STATE_EXTERNAL_FILEPATH;
	idx = 0;
	for (page_idx = 0;
		page_idx < (ws->alloc_index / PAGECACHEINFO_PER_PAGE);
		page_idx++) {
		ret = workingset_collector_fill_record_cacheseq_wsrcrdlocked(
		    data, &idx, ws->cache_pages, page_idx,
		    PAGECACHEINFO_PER_PAGE);
		if (ret)
			goto fill_data_fail;
	}
	ret = workingset_collector_fill_record_cacheseq_wsrcrdlocked(
	    data, &idx, ws->cache_pages, page_idx,
	    (ws->alloc_index % PAGECACHEINFO_PER_PAGE));
	if (ret)
		goto fill_data_fail;

	record->state |= E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY;
	return 0;

fill_data_fail:
	record->state &= ~(E_RECORD_STATE_USED | E_RECORD_STATE_DIRTY |
	    E_RECORD_STATE_UPDATE_HEADER_ONLY);
	return ret;
}

static struct s_ws_record *workingset_get_record_wslocked(
	struct s_workingset *ws, bool *is_exist)
{
	struct s_ws_record *record;

	record = workingset_get_existed_record_wslocked(&ws->owner, true);
	if (record) {
		*is_exist = true;
		/*
		 * Check the effect of prereading by comparing the blkio
		 * count on main thread.the empirical blkio used to
		 * deciding recollect page_cache info again.
		 */
		if (!ws->file_count || !ws->pageseq_count) {
			mutex_lock(&record->mutex);
			if (record->is_af_cleared == 0)
				workingset_preread_qos_wsrcrdlocked(ws, record);
			record->state &= ~E_RECORD_STATE_UPDATE_BASE_BLKIO;
			mutex_unlock(&record->mutex);
			return NULL;
		}
	} else {
		if (!ws->file_count || !ws->pageseq_count) {
			pr_warn("%s, busy!state=%s, path/name is null!\n",
				__func__, workingset_state_strs(ws->state));
			return NULL;
		}
		*is_exist = false;
		record = workingset_get_available_record();
	}

	return record;
}

static void workingset_collector_do_record_wslocked(
	struct s_workingset *ws, unsigned long discard_count)
{
	bool is_exist = false;
	struct s_ws_record *record = workingset_get_record_wslocked(
	    ws, &is_exist);

	if (!record)
		return;
	mutex_lock(&record->mutex);
	record->state &= ~E_RECORD_STATE_UPDATE_BASE_BLKIO;
	record->need_update = 0;

	/* Organize the collect data, and save in record */
	if (!workingset_collector_read_data_wsrcrdlocked(ws,
	    record, is_exist)) {
		if (ws->clear_young)
			record->is_af_cleared = 1;
		if (!is_exist) {
			/*
			 * We'll recollect info for the second times
			 * because there were some permit dialog
			 * in the first time.
			 */
			if (!ws->clear_young)
				record->need_update = 1;
			workingset_insert_record_to_list_head(record);
		}
		ws_dbg("%s: %ufls %usqs, pgs=%u,rpt=%lu,dscd=%lu\n", __func__,
			record->data.file_cnt, record->data.pageseq_cnt,
			ws->page_sum, ws->repeated_count, discard_count);
	} else if (!is_exist) {
		workingset_insert_record_to_list_tail(record);
	}
	mutex_unlock(&record->mutex);
}

static void workingset_collector_do_record_locked(
	struct s_workingset *ws, unsigned long discard_count)
{
	mutex_lock(&ws->mutex);

	ws_dbg("%s: uid=%u, name=%s, state=%s\n", __func__, ws->owner.uid,
		ws->owner.name, workingset_state_strs(ws->state));
	if (((ws->state & E_CGROUP_STATE_MONITOR_STOP) !=
	    E_CGROUP_STATE_MONITOR_STOP) ||
	    !ws->owner.name || !ws->owner.record_path) {
		pr_warn("%s, maybe busy!state=%s, path or name is null!\n",
			__func__, workingset_state_strs(ws->state));
		mutex_unlock(&ws->mutex);
		return;
	}

	workingset_collector_do_record_wslocked(ws, discard_count);
	workingset_destroy_data(ws, true);
	workingset_collector_reset(ws);
	ws->state = E_CGROUP_STATE_MONITOR_OUTOFWORK;
	mutex_unlock(&ws->mutex);
}

static void workingset_collector_do_work(struct s_ws_collector *collector)
{
	enum collector_wait_flags wait_flag;

	spin_lock(&collector->lock);
	wait_flag = collector->wait_flag;
	collector->wait_flag = F_NONE;
	if (wait_flag == F_COLLECT_PENDING) {
		workingset_collector_do_collect_locked(collector);
		spin_unlock(&collector->lock);
	} else if (wait_flag == F_RECORD_PENDING) {
		struct s_workingset *monitor = collector->monitor;
		unsigned long discard_count = collector->discard_count;

		collector->discard_count = 0;
		spin_unlock(&collector->lock);
		if (monitor)
			workingset_collector_do_record_locked(monitor,
				discard_count);
	} else {
		spin_unlock(&collector->lock);
	}
}

int workingset_collect_kworkthread(void *p)
{
	int ret;
	struct s_ws_collector *collector = p;

	if (!p) {
		pr_err("%s: p is NULL!\n", __func__);
		return 0;
	}

	while (!kthread_should_stop()) {
		/*lint -e578*/
		ret = wait_event_interruptible(collector->collect_wait,
		    ((collector->wait_flag == F_COLLECT_PENDING) ||
		    (collector->wait_flag == F_RECORD_PENDING)));
		/*lint +e578*/
		if (ret < 0)
			continue;

		workingset_collector_do_work(collector);
	}
	pr_err("%s: exit!\n", __func__);

	return 0;
}
