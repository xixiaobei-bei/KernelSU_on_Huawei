/*
 * hisi_kernel_gcov_helper.c
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * Author: nobody
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
 *
 */

void gcov_iter_new_gcov_get_panic_gcda(struct gcov_info *info)
{
	extern void *gcov_gcda_malloc_ptr_curr;
	extern unsigned int g_count_gcda;
	struct gcov_iterator *iter;

	iter = kzalloc(sizeof(struct gcov_iterator), GFP_KERNEL);
	if (!iter)
		goto err;
	iter->info = info;

	/* Dry-run to get the actual buffer size. */
	iter->size = convert_to_gcda(NULL, info);
	*(unsigned int*)(gcov_gcda_malloc_ptr_curr) = iter->size;
	gcov_gcda_malloc_ptr_curr += 4;

	*(unsigned int*)(gcov_gcda_malloc_ptr_curr) = strlen(info->filename);
	gcov_gcda_malloc_ptr_curr += 4;

	snprintf(gcov_gcda_malloc_ptr_curr, strlen(info->filename)+1, "%s", info->filename);
	gcov_gcda_malloc_ptr_curr += strlen(info->filename) + 1;

	iter->buffer = gcov_gcda_malloc_ptr_curr;
	convert_to_gcda(iter->buffer, info);
	gcov_gcda_malloc_ptr_curr += iter->size;

	g_count_gcda++;

	kfree(iter);
err:
	return;
}
