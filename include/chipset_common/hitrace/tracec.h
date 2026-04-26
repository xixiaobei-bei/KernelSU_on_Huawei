/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: hitrace interface
 * Author: lc.luchao@huawei.com
 * Create: 2019-06-08
 */

#ifndef HIVIEWDFX_HITRACE_C_H
#define HIVIEWDFX_HITRACE_C_H

#include <linux/string.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/random.h>
#include <asm/byteorder.h>
#include <linux/byteorder/generic.h>

enum hitrace_id_valid {
	HITRACE_ID_INVALID = 0,
	HITRACE_ID_VALID = 1,
};

enum hitrace_version {
	HITRACE_VER_1 = 0,
};

enum hitrace_flag {
	/* MIN: valid */
	HITRACE_FLAG_MIN = 0,
	/*DEFAULT: default value*/
	HITRACE_FLAG_DEFAULT = 0,
	/* trace sync and async call. default: trace sync call only */
	HITRACE_FLAG_INCLUDE_ASYNC = 1 << 0,
	/* donot create child span. default: create child span */
	HITRACE_FLAG_DONOT_CREATE_SPAN = 1 << 1,
	/* output tracepoint info in span. default: do not output */
	HITRACE_FLAG_TP_INFO = 1 << 2,
	/* donot output begin and end info. default: output */
	HITRACE_FLAG_NO_BE_INFO = 1 << 3,
	/* do not add id to log. default: add id to log */
	HITRACE_FLAG_DONOT_ENABLE_LOG = 1 << 4,
	/* the trace is triggered by fault */
	HITRACE_FLAG_FAULT_TRIGGER = 1 << 5,
	/* MAX: invalid */
	HITRACE_FLAG_MAX = 1 << 6,
};

/* HiTrace tracepoint type */
enum hitrace_tracepoint_type {
	HITRACE_TP_CS = 0, /* client send */
	HITRACE_TP_CR = 1, /* client receive */
	HITRACE_TP_SS = 2, /* server send */
	HITRACE_TP_SR = 3, /* server receive */
	HITRACE_TP_GENERAL = 4, /* general info */
	HITRACE_TP_MAX = 5, /* MAX: invalid */
};

enum hitrace_hash_data_index {
	DEVICE_ID = 0,
	PARENT_SPAN_ID = 1,
	SPAN_ID = 2,
	SECOND = 3,
	USECOND = 4,
};

struct hitrace_id_struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint64_t valid : 1;
	uint64_t ver : 3;
	uint64_t chain_id : 60;

	uint64_t flags : 12;
	uint64_t span_id : 26;
	uint64_t parent_span_id : 26;
#elif defined(__BIG_ENDIAN_BITFIELD)
	uint64_t chain_id : 60;
	uint64_t ver : 3;
	uint64_t valid : 1;

	uint64_t parent_span_id : 26;
	uint64_t span_id : 26;
	uint64_t flags : 12;
#else
#error "ERROR: No BIG_LITTLE_ENDIAN defines"
#endif
};

#define HITRACE_ID_LEN sizeof(struct hitrace_id_struct)

void hitrace_init_id(struct hitrace_id_struct *id_pointer);
struct hitrace_id_struct hitrace_begin(const char *name, int flags);
void hitrace_end(const struct hitrace_id_struct *id_pointer);
struct hitrace_id_struct hitrace_get_id(void);
void hitrace_set_id(const struct hitrace_id_struct *id_pointer);
void hitrace_clear_id(void);
struct hitrace_id_struct hitrace_create_span(void);
void hitrace_trace_point(enum hitrace_tracepoint_type type,
	const struct hitrace_id_struct *id_pointer,
	const char *fmt, ...) __printf(3, 4);
void hitrace_trace_point_with_args(enum hitrace_tracepoint_type type,
	const struct hitrace_id_struct *id_pointer,
	const char *fmt, va_list args);
void hitrace_set_chain_id(struct hitrace_id_struct *id_pointer,
	uint64_t chain_id);
struct hitrace_id_struct hitrace_bytes_to_id(const uint8_t *pid_array, int len);
int hitrace_id_to_bytes(const struct hitrace_id_struct *id_pointer,
	uint8_t *pid_array, int len);

static inline int hitrace_is_valid(const struct hitrace_id_struct *id_pointer)
{
	return (id_pointer) && (id_pointer->valid == HITRACE_ID_VALID);
}

static inline int hitrace_is_flag_enabled(
	const struct hitrace_id_struct *id_pointer, enum hitrace_flag flag)
{
	return (hitrace_is_valid(id_pointer) &&
		((id_pointer->flags & flag) != 0));
}

static inline void hitrace_enable_flag(struct hitrace_id_struct *id_pointer,
	enum hitrace_flag flag)
{
	if (hitrace_is_valid(id_pointer))
		id_pointer->flags |= flag;
}

static inline int hitrace_get_flags(struct hitrace_id_struct *id_pointer)
{
	if (!hitrace_is_valid(id_pointer))
		return 0;
	return id_pointer->flags;
}

static inline void hitrace_set_flags(
	struct hitrace_id_struct *id_pointer, int flags)
{
	if (hitrace_is_valid(id_pointer) && (flags >= HITRACE_FLAG_MIN) &&
		(flags < HITRACE_FLAG_MAX))
		id_pointer->flags = flags;
}

static inline uint64_t hitrace_get_chain_id(
	const struct hitrace_id_struct *id_pointer)
{
	if (!hitrace_is_valid(id_pointer))
		return 0;
	return id_pointer->chain_id;
}

static inline uint64_t hitrace_get_span_id(
	const struct hitrace_id_struct *id_pointer)
{
	if (!hitrace_is_valid(id_pointer))
		return 0;
	return id_pointer->span_id;
}

static inline void hitrace_set_span_id(
	struct hitrace_id_struct *id_pointer,
	uint64_t span_id)
{
	if (hitrace_is_valid(id_pointer))
		id_pointer->span_id = span_id;
}

static inline uint64_t hitrace_get_parent_span_id(
	const struct hitrace_id_struct *id_pointer)
{
	if (!hitrace_is_valid(id_pointer))
		return 0;
	return id_pointer->parent_span_id;
}

static inline void hitrace_set_parent_span_id(
	struct hitrace_id_struct *id_pointer, uint64_t parent_span_id)
{
	if (hitrace_is_valid(id_pointer))
		id_pointer->parent_span_id = parent_span_id;
}
#endif /* HIVIEWDFX_HITRACE_C_H */
