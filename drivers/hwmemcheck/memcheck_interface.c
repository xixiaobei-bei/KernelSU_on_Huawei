/*
 * memcheck_interface.c
 *
 * unify kernel memory leak interface for adapt to different platform hisi or mtk
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "memcheck_interface.h"

size_t get_mem_total(int type)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_get_mem_total(type);
#else
	return mm_get_mem_total(type);
#endif
}

size_t get_mem_detail(int type, void *buf, size_t len)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_get_mem_detail(type, buf, len);
#else
	return mm_get_mem_detail(type, buf, len);
#endif
}

int page_trace_on(int type, char *name)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_page_trace_on(type, name);
#else
	return mm_page_trace_on(type, name);
#endif
}

int page_trace_off(int type, char *name)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_page_trace_off(type, name);
#else
	return mm_page_trace_off(type, name);
#endif
}

int page_trace_open(int type, int subtype)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_page_trace_open(type, subtype);
#else
	return mm_page_trace_open(type, subtype);
#endif
}

int page_trace_close(int type, int subtype)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_page_trace_close(type, subtype);
#else
	return mm_page_trace_close(type, subtype);
#endif
}

size_t page_trace_read(int type,
	struct mm_stack_info *info, size_t len, int subtype)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_page_trace_read(type, info, len, subtype);
#else
	return mm_page_trace_read(type, info, len, subtype);
#endif
}

size_t get_ion_by_pid(pid_t pid)
{
#ifdef CONFIG_HISI_PAGE_TRACE
	return hisi_get_ion_by_pid(pid);
#else
	return mm_get_ion_by_pid(pid);
#endif
}
