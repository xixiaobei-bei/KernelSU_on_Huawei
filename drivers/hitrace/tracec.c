/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: hitrace interface
 * Author: lc.luchao@huawei.com
 * Create: 2019-06-08
 */
#include "chipset_common/hitrace/tracec.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD002D03
#define LOG_TAG "HiTraceC"
#define HITRACE_TO_STRING(type) #type
#define HITRACE_GET_ID_FAIL (-1)
#define HITRACE_ALL_VALID 0
#define HITRACE_ALL_VALID_EXCEPT_SPAN 1
static const char * const hitrace_type_to_name[] = {
	HITRACE_TO_STRING(HITRACE_TP_CS),
	HITRACE_TO_STRING(HITRACE_TP_CR),
	HITRACE_TO_STRING(HITRACE_TP_SS),
	HITRACE_TO_STRING(HITRACE_TP_SR),
	HITRACE_TO_STRING(HITRACE_TP_GENERAL),
};

static const uint32_t hitrace_hash_data_num = 5;
static const int hitrace_tp_buffer_size = 1024;
static const uint32_t hitrace_seed = 131;

struct hitrace_chain_id_struct {
	union {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		struct {
			uint64_t reserved : 4;
			uint64_t usecond : 20;
			uint64_t second : 16;
			uint64_t cpu_id : 4;
			uint64_t device_id : 20;
		};
		struct {
			uint64_t padding : 4;
			uint64_t chain_id : 60;
		};
#elif defined(__BIG_ENDIAN_BITFIELD)
		struct {
			uint64_t device_id : 20;
			uint64_t cpu_id : 4;
			uint64_t second : 16;
			uint64_t usecond : 20;
			uint64_t reserved : 4;
		};
		struct {
			uint64_t chain_id : 60;
			uint64_t padding : 4;
		};
#else
#error "ERROR: No BIG_LITTLE_ENDIAN defines"
#endif
	};
};

struct hitrace_id_struct_extra {
	uint32_t set_tls : 1;
	uint32_t reserved : 31;
};

struct hitrace_id_struct_inner {
	struct hitrace_id_struct id;
	struct hitrace_id_struct_extra extra;
};

/* remove g_hitrace_id_inner when the interface in musl ready */
static struct hitrace_id_struct_inner g_hitrace_id_inner;

static inline struct hitrace_id_struct_inner get_thread_id_inner(void)
{
	/* get traceid from userspace depend on the interface in musl */
	return g_hitrace_id_inner;
}

static inline void set_thread_id_inner(const struct hitrace_id_struct *id)
{
	/* set traceid to userspace depend on the interface in musl */
	g_hitrace_id_inner.id = *id;
}

void hitrace_init_id(struct hitrace_id_struct *id_pointer)
{
	if (!id_pointer)
		return;
	id_pointer->valid = HITRACE_ID_INVALID;
	id_pointer->ver = 0;
	id_pointer->chain_id = 0;
	id_pointer->flags = 0;
	id_pointer->span_id = 0;
	id_pointer->parent_span_id = 0;
}
EXPORT_SYMBOL(hitrace_init_id);

struct hitrace_id_struct hitrace_get_id(void)
{
	struct hitrace_id_struct_inner thread_id = get_thread_id_inner();

	return thread_id.id;
}
EXPORT_SYMBOL(hitrace_get_id);

void hitrace_set_id(const struct hitrace_id_struct *id_pointer)
{
	struct hitrace_id_struct_inner thread_id;

	if (!hitrace_is_valid(id_pointer))
		return;
	thread_id = get_thread_id_inner();
	thread_id.id = *id_pointer;
	set_thread_id_inner(&(thread_id.id));
}
EXPORT_SYMBOL(hitrace_set_id);

void hitrace_clear_id(void)
{
	struct hitrace_id_struct_inner thread_id = get_thread_id_inner();

	hitrace_init_id(&(thread_id.id));
	set_thread_id_inner(&(thread_id.id));
}
EXPORT_SYMBOL(hitrace_clear_id);

static int hitrace_get_device_id(void)
{
    /* save device id and use it later */
	static int device_id;

	if (device_id == 0)
		get_random_bytes(&device_id, sizeof(device_id));
	return device_id;
}

static int hitrace_get_cpu_id(void)
{
	static unsigned int cpu_id;

	cpu_id++;
	return cpu_id;
}

static uint64_t hitrace_create_chain_id(void)
{
	/* get timestamp */
	struct timeval tv;
	struct hitrace_chain_id_struct chain_id = {
		.padding = 0,
		.chain_id = 0
	};

	do_gettimeofday(&tv);
	chain_id.device_id = hitrace_get_device_id();
	chain_id.cpu_id = hitrace_get_cpu_id();
	chain_id.second = tv.tv_sec;
	chain_id.usecond = tv.tv_usec;

	return chain_id.chain_id;
}

struct hitrace_id_struct hitrace_begin(const char *name, int flags)
{
	struct hitrace_id_struct id;
	struct hitrace_id_struct_inner thread_id = get_thread_id_inner();

	hitrace_init_id(&id);
	if (hitrace_is_valid(&(thread_id.id)) ||
		(flags < HITRACE_FLAG_MIN) || (flags >= HITRACE_FLAG_MAX))
		return id;
	thread_id.id.valid = HITRACE_ID_VALID;
	thread_id.id.ver = HITRACE_VER_1;
	thread_id.id.chain_id = hitrace_create_chain_id();
	thread_id.id.flags = flags;
	thread_id.id.span_id = 0;
	thread_id.id.parent_span_id = 0;

	set_thread_id_inner(&(thread_id.id));

	if (!hitrace_is_flag_enabled(&(thread_id.id),
		HITRACE_FLAG_NO_BE_INFO))
		/* use hilog interface instead. */
		pr_info("HiTraceBegin name:%s flag:%x\n",
			!name ? "" : name, flags);
	return thread_id.id;
}
EXPORT_SYMBOL(hitrace_begin);

void hitrace_end(const struct hitrace_id_struct *id_pointer)
{
	struct hitrace_id_struct_inner thread_id;

	if (!hitrace_is_valid(id_pointer)) {
		/* use hilog interface instead. */
		pr_err("HiTraceEnd error: invalid end id\n");
		return;
	}
	thread_id = get_thread_id_inner();
	if (!hitrace_is_valid(&(thread_id.id))) {
		/* use hilog interface instead. */
		pr_err("HiTraceEnd error: invalid thread id\n");
		return;
	}
	if (hitrace_get_chain_id(id_pointer) !=
		hitrace_get_chain_id(&(thread_id.id))) {
		/* use hilog interface instead. */
		pr_err(
			"HiTraceEnd error: end id(%lld) != thread id(%lld)\n",
			(uint64_t)id_pointer->chain_id,
			(uint64_t)(thread_id.id.chain_id));
		return;
	}
	if (!hitrace_is_flag_enabled(&(thread_id.id),
		HITRACE_FLAG_NO_BE_INFO))
		/* use hilog interface instead. */
		pr_info("HiTraceEnd\n");
	hitrace_init_id(&(thread_id.id));
	set_thread_id_inner(&(thread_id.id));
}
EXPORT_SYMBOL(hitrace_end);

/* BKDRHash */
uint32_t hash_func(const void *pdata, uint32_t data_len)
{
	uint32_t hash = 0;
	uint32_t len = data_len;
	const char *p = pdata;

	if ((!pdata) || data_len == 0)
		return 0;
	for (; len > 0; --len)
		hash = (hash * hitrace_seed) + (*p++);
	return hash;
}

struct hitrace_id_struct hitrace_create_span(void)
{
	struct timeval tv;
	struct hitrace_id_struct id = hitrace_get_id();
	uint32_t hash_data[hitrace_hash_data_num];
	uint32_t hash = 0;

	if (!hitrace_is_valid(&id))
		return id;
	if (hitrace_is_flag_enabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN))
		return id;
	/* create child span id */
	do_gettimeofday(&tv);
	hash_data[DEVICE_ID] = hitrace_get_device_id();
	hash_data[PARENT_SPAN_ID] = id.parent_span_id;
	hash_data[SPAN_ID] = id.span_id;
	hash_data[SECOND] = tv.tv_sec;
	hash_data[USECOND] = tv.tv_usec;
	hash = hash_func(hash_data, hitrace_hash_data_num * sizeof(uint32_t));
	id.parent_span_id = id.span_id;
	id.span_id = hash;
	return id;
}
EXPORT_SYMBOL(hitrace_create_span);

void hitrace_set_chain_id(struct hitrace_id_struct *id_pointer,
	uint64_t chain_id)
{
	if (id_pointer) {
		if (!hitrace_is_valid(id_pointer)) {
			id_pointer->valid = HITRACE_ID_VALID;
			id_pointer->ver = HITRACE_VER_1;
			id_pointer->flags = 0;
			id_pointer->span_id = 0;
			id_pointer->parent_span_id = 0;
		}
		id_pointer->chain_id = chain_id;
	}
}
EXPORT_SYMBOL(hitrace_set_chain_id);

struct hitrace_id_struct hitrace_bytes_to_id(const uint8_t *pid_array, int len)
{
	struct hitrace_id_struct id = { 0, 0, 0, 0, 0, 0 };
	__be64 test_valid = 0;

	if ((!pid_array) || (len != (int)HITRACE_ID_LEN))
		return id;
	test_valid = be64_to_cpu(*((__be64 *)pid_array));
	if (!hitrace_is_valid((struct hitrace_id_struct *)&test_valid))
		return id;
	*((__be64 *)(&id)) = be64_to_cpu(*((__be64 *)pid_array));
	*((__be64 *)(&id) + 1) = be64_to_cpu(*((__be64 *)pid_array + 1));
	return id;
}
EXPORT_SYMBOL(hitrace_bytes_to_id);

int hitrace_id_to_bytes(const struct hitrace_id_struct *id_pointer,
	uint8_t *pid_array, int len)
{
	if (!hitrace_is_valid(id_pointer) ||
		(len < (int)HITRACE_ID_LEN) || (!pid_array))
		return 0;
	*((__be64 *)pid_array) = cpu_to_be64(*((__be64 *)id_pointer));
	*((__be64 *)pid_array + 1) = cpu_to_be64(*((__be64 *)id_pointer + 1));
	return HITRACE_ID_LEN;
}
EXPORT_SYMBOL(hitrace_id_to_bytes);

void hitrace_trace_point_with_args(enum hitrace_tracepoint_type type,
	const struct hitrace_id_struct *id_pointer,
	const char *fmt, va_list args)
{
	char buff[hitrace_tp_buffer_size];
	int ret = 0;

	if (!hitrace_is_valid(id_pointer) ||
		type < 0 || type >= HITRACE_TP_MAX)
		return;
	if (!hitrace_is_flag_enabled(id_pointer, HITRACE_FLAG_TP_INFO))
		return;
	/* privacy parameter: vsnprintf => hilog_vsnprintf */
	ret = vsnprintf(buff, hitrace_tp_buffer_size, fmt, args);
	if (ret < 0)
		return;
	buff[hitrace_tp_buffer_size - 1] = 0;
	/* use hilog interface instead. */
	pr_info("%s %s\n", hitrace_type_to_name[type], buff);
}

void hitrace_trace_point(enum hitrace_tracepoint_type type,
	const struct hitrace_id_struct *id_pointer, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	hitrace_trace_point_with_args(type, id_pointer, fmt, args);
	va_end(args);
}
EXPORT_SYMBOL(hitrace_trace_point);

/* return: -1 -- fail; 0 -- all valid; 1 -- all valid except span */
int hitrace_get_id_for_hilog(uint64_t *pchain_id, uint32_t *pflags,
	uint64_t *pspan_id, uint64_t *pparent_span_id)
{
	struct hitrace_id_struct id = hitrace_get_id();

	if (!hitrace_is_valid(&id))
		return HITRACE_GET_ID_FAIL;
	if (!pchain_id || !pflags || !pspan_id || !pparent_span_id)
		return HITRACE_GET_ID_FAIL;
	if (hitrace_is_flag_enabled(&id, HITRACE_FLAG_DONOT_ENABLE_LOG))
		return HITRACE_GET_ID_FAIL;
	*pchain_id = hitrace_get_chain_id(&id);
	*pflags = hitrace_get_flags(&id);
	if (hitrace_is_flag_enabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN)) {
		*pspan_id = 0;
		*pparent_span_id = 0;
		return HITRACE_ALL_VALID_EXCEPT_SPAN;
	}
	*pspan_id = hitrace_get_span_id(&id);
	*pparent_span_id = hitrace_get_parent_span_id(&id);
	return HITRACE_ALL_VALID;
}

static int __init hitrace_init(void)
{
	pr_info("hitrace_init begin\n");
	/* Call HiLog Register Interface. */
	return 0;
}

static void __exit hitrace_exit(void)
{
	pr_info("hitrace_exit begin\n");
}

module_init(hitrace_init);
module_exit(hitrace_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("interface for hitrace");

