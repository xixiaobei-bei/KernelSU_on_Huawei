#ifndef MEMCHECK_INTERFACE_H
#define MEMCHECK_INTERFACE_H

#ifdef CONFIG_HISI_PAGE_TRACE
#include <linux/hisi/mem_trace.h>
#else
#include <linux/mem_track/mem_trace.h>
#endif

size_t get_mem_total(int type);
size_t get_mem_detail(int type, void *buf, size_t len);
int page_trace_on(int type, char *name);
int page_trace_off(int type, char *name);
int page_trace_open(int type, int subtype);
int page_trace_close(int type, int subtype);
size_t page_trace_read(int type,
	struct mm_stack_info *info, size_t len, int subtype);
size_t get_ion_by_pid(pid_t pid);

#endif /* MEMCHECK_INTERFACE_H */
