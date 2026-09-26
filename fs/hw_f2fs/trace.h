// SPDX-License-Identifier: GPL-2.0
/*
 * f2fs IO tracer
 *
 * Copyright (c) 2014 Motorola Mobility
 * Copyright (c) 2014 Jaegeuk Kim <jaegeuk@kernel.org>
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM f2fs

#if !defined(__F2FS_HW_TRACE_H__) || defined(TRACE_HEADER_MULTI_READ)
#define __F2FS_HW_TRACE_H__

#include <linux/tracepoint.h>

TRACE_EVENT(f2fs_skip_log_writeback,

        TP_PROTO(unsigned int ino_num),

        TP_ARGS(ino_num),

        TP_STRUCT__entry(
                __field(unsigned int, ino_num)
        ),

        TP_fast_assign(
                __entry->ino_num = ino_num;
        ),

        TP_printk("f2fs skip log writeback :ino %u", __entry->ino_num)
);

TRACE_EVENT(f2fs_cold_file_should_IPU,

        TP_PROTO(unsigned int ino_num),

        TP_ARGS(ino_num),

        TP_STRUCT__entry(
                __field(unsigned int, ino_num)
        ),
        TP_fast_assign(
                __entry->ino_num = ino_num;
        ),

        TP_printk("f2fs cold file need ipu :ino %u", __entry->ino_num)
);

#endif /* __TRACE_TRACE_H__ */

#ifdef CREATE_TRACE_POINTS
#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE trace
#include <trace/define_trace.h>

#undef TRACE_INCLUDE_FILE
#undef TRACE_INCLUDE_PATH
#endif

#ifndef __F2FS_TRACE_H__
#define __F2FS_TRACE_H__

#ifdef CONFIG_F2FS_IO_TRACE

enum file_type {
	__NORMAL_FILE,
	__DIR_FILE,
	__NODE_FILE,
	__META_FILE,
	__ATOMIC_FILE,
	__VOLATILE_FILE,
	__MISC_FILE,
};

struct last_io_info {
	int major, minor;
	pid_t pid;
	enum file_type type;
	struct f2fs_io_info fio;
	block_t len;
};

extern void f2fs_trace_pid(struct page *);
extern void f2fs_trace_ios(struct f2fs_io_info *, int);
extern void f2fs_build_trace_ios(void);
extern void f2fs_destroy_trace_ios(void);
#else
#define f2fs_trace_pid(p)
#define f2fs_trace_ios(i, n)
#define f2fs_build_trace_ios()
#define f2fs_destroy_trace_ios()

#endif
#endif /* __F2FS_TRACE_H__ */
