
#ifndef TC_NS_LOG_H_
#define TC_NS_LOG_H_

#include <linux/printk.h>
enum {
	TZ_DEBUG_VERBOSE = 0,
	TZ_DEBUG_DEBUG,
	TZ_DEBUG_INFO,
	TZ_DEBUG_WARN,
	TZ_DEBUG_ERROR,
};

#ifdef DEF_ENG
#define TEE_ENG_LOG_MASK 2
#define TEE_LOG_MASK TEE_ENG_LOG_MASK
#else
#define TEE_USR_LOG_MASK 3
#define TEE_LOG_MASK TEE_USR_LOG_MASK
#endif

#define tlogv(fmt, args...) \
do { \
	if (TZ_DEBUG_VERBOSE >= TEE_LOG_MASK) \
		pr_info("(%i, %s)%s: " fmt, current->pid, current->comm, __func__, ## args); \
} while (0)


#define tlogd(fmt, args...) \
do { \
	if (TZ_DEBUG_DEBUG >= TEE_LOG_MASK) \
		pr_info("(%i, %s)%s: " fmt, current->pid, current->comm, __func__, ## args); \
} while (0)


#define tlogi(fmt, args...) \
do { \
	if (TZ_DEBUG_INFO >= TEE_LOG_MASK) \
		pr_info("(%i, %s)%s: " fmt, current->pid, current->comm, __func__, ## args); \
} while (0)


#define tlogw(fmt, args...) \
do { \
	if (TZ_DEBUG_WARN >= TEE_LOG_MASK) \
		pr_warn("(%i, %s)%s: " fmt, current->pid, current->comm, __func__, ## args);\
} while (0)


#define tloge(fmt, args...) \
		pr_err("(%i, %s)%s: " fmt, current->pid, current->comm, __func__, ## args)

#endif
