#ifndef _MEMCHECK_COMMON_H
#define _MEMCHECK_COMMON_H

#include <linux/types.h>

#define TEST_BIT(v, i) ((v) & (1 << (i)))
#define	memcheck_err(format, ...)	\
	pr_err("MemLeak[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)
#define	memcheck_info(format, ...)	\
	pr_info("MemLeak[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)

struct memstat_all;
struct memstat_kernel;
struct track_cmd;
struct detail_info;
struct stack_info;
struct lmk_oom_read;
struct lmk_oom_write;
struct task_type_read;

unsigned short memcheck_get_memstat(struct memstat_all *p);
int memcheck_do_command(const struct track_cmd *cmd);
void memcheck_save_top_slub(const char *name);
int memcheck_detail_read(void *buf, struct detail_info *info);
int memcheck_stack_read(void *buf, struct stack_info *info);
int memcheck_stack_write(const void *buf, const struct stack_info *info);
int memcheck_stack_clear(void);
int memcheck_lmk_oom_read(void *buf, struct lmk_oom_read *rec);
int memcheck_lmk_oom_write(const struct lmk_oom_write *write);
int memcheck_get_task_type(void *buf, struct task_type_read *read);
int memcheck_wait_stack_ready(u16 type);

#endif /* _MEMCHECK_COMMON_H */
