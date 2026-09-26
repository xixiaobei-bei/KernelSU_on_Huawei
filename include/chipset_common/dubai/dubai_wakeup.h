#ifndef DUBAI_WAKEUP_H
#define DUBAI_WAKEUP_H

#include <linux/types.h>

#define DUBAI_WAKEUP_NUM_MAX					10
#define DUBAI_WAKEUP_TAG_LENGTH					48
#define DUBAI_WAKEUP_MSG_LENGTH					128

enum dubai_wakeup_type {
	DUBAI_WAKEUP_TYPE_INVALID = 0,
	DUBAI_WAKEUP_TYPE_RTC,
	DUBAI_WAKEUP_TYPE_GPIO,
	DUBAI_WAKEUP_TYPE_OTHERS,
};

struct dubai_wakeup_info {
	enum dubai_wakeup_type type;
	char tag[DUBAI_WAKEUP_TAG_LENGTH];
	char msg[DUBAI_WAKEUP_MSG_LENGTH];
};

typedef int (*dubai_suspend_notify_fn_t)(unsigned long mode);
typedef int (*dubai_wakeup_stats_fn_t)(struct dubai_wakeup_info *data, size_t num);

struct dubai_wakeup_stats_ops {
	dubai_suspend_notify_fn_t suspend_notify;
	dubai_wakeup_stats_fn_t get_stats;
};

extern void dubai_wakeup_notify(const char *source);

#endif // DUBAI_WAKEUP_H
