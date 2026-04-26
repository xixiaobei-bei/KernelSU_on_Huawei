/*
 * Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef HISI_MDFX_H
#define HISI_MDFX_H


#include <linux/kernel.h>
#include <linux/hisi/hisi_mdfx_uapi.h>

/*
 * Message printing priorities:
 * LEVEL 0 KERN_EMERG (highest priority)
 * LEVEL 1 KERN_ALERT
 * LEVEL 2 KERN_CRIT
 * LEVEL 3 KERN_ERR
 * LEVEL 4 KERN_WARNING
 * LEVEL 5 KERN_NOTICE
 * LEVEL 6 KERN_INFO
 * LEVEL 7 KERN_DEBUG (Lowest priority)
 */
enum mdfx_kmsg_level {
	DFX_KERN_EMERG = 0x0,
	DFX_KERN_ALERT,
	DFX_KERN_CRIT,
	DFX_KERN_ERR,
	DFX_KERN_WARNING,
	DFX_KERN_NOTICE,
	DFX_KERN_INFO,
	DFX_KERN_DEBUG,
};
typedef void (*dump_info_func)(int64_t id, uint64_t detail);

enum {
	DEF_EVENT_UNDER_FLOW = 0,
	DEF_EVENT_VACTIVE_TIMEOUT,
	DEF_EVENT_FENCE_TIMEOUT,
	DEF_EVENT_MAX,
};

struct mdfx_event_desc_t {
	uint64_t relevance_visitor_types;
	uint64_t actions;
	uint32_t detail_count;
	uint64_t *detail;
	char event_name[MDFX_EVENT_NAME_MAX];
};

#ifdef CONFIG_HISI_MDFX

extern bool mdfx_ready;
extern int mdfx_debug_level;

#define ENABLE_MDFX_LOG(level) (mdfx_ready && ((level) <= mdfx_debug_level))

// kernel log agent
#define MDFX_EMERG(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_EMERG)) \
			mdfx_logger_emit(id, "[E]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_ALERT(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_ALERT)) \
			mdfx_logger_emit(id, "[A]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_CRIT(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_CRIT))  \
			mdfx_logger_emit(id, "[C]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_ERR(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_ERR)) \
			mdfx_logger_emit(id, "[E]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_WARNING(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_WARNING)) \
			mdfx_logger_emit(id, "[W]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_NOTICE(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_NOTICE)) \
			mdfx_logger_emit(id, "[N]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_INFO(id, msg, ...)    \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_INFO)) \
			mdfx_logger_emit(id, "[I]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_DEBUG(id, msg, ...) \
	do { \
		if (ENABLE_MDFX_LOG(DFX_KERN_DEBUG)) \
			mdfx_logger_emit(id, "[D]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define MDFX_ASSERT_IF(cond) \
	do {\
		if (cond) { \
			pr_err("[MDFX]%s,line=%d assert\n", __func__, __LINE__); \
			BUG();\
		}\
	} while (0)


/*
 * Event Manager
 */
void mdfx_report_event(int64_t visitor_id, struct mdfx_event_desc_t *event_desc);
void mdfx_report_default_event(int64_t visitor_id, uint32_t default_event_type);

/**
 * visitor
 *
 * registor dump information callback,
 * such as dss need dump cmdlist, freq and voltage, those information just caller
 * know them.
 */
int64_t mdfx_create_visitor(uint64_t type, dump_info_func dumper_cb);
void mdfx_destroy_visitor(int64_t id);

/*
 * Tracing
 */
void mdfx_tracing_point(int64_t visitor_id, uint64_t tracing_type, char *msg);

/*
 * logger
 */
int mdfx_logger_emit(int64_t id, const char *fmt, ...);

/*
 * saving
 */
int mdfx_save_buffer_to_file(const char *tag, const char *modulename, const char *buf, uint32_t size);

#else  // CONFIG_HISI_MDFX

// kernel log agent
#define MDFX_EMERG(id, msg, ...)
#define MDFX_ALERT(id, msg, ...)
#define MDFX_CRIT(id, msg, ...)
#define MDFX_ERR(id, msg, ...)
#define MDFX_WARNING(id, msg, ...)
#define MDFX_NOTICE(id, msg, ...)
#define MDFX_INFO(id, msg, ...)
#define MDFX_DEBUG(id, msg, ...)
#define MDFX_ASSERT_IF(cond)

#define mdfx_create_visitor(type, dumper_cb) 0
#define mdfx_report_event(visitor_id, event_desc)
#define mdfx_report_default_event(visitor_id, default_event_type)
#define mdfx_destroy_visitor(id)
#define mdfx_tracing_point(visitor_id, tracing_type, msg)
#define mdfx_logger_emit(id, fmt, ...)
#define mdfx_save_buffer_to_file(tag, buf, size)

#endif

#endif /* MDFX_H */
