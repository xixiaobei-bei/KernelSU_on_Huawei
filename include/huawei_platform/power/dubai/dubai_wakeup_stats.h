#ifndef DUBAI_WAKEUP_STATS_H
#define DUBAI_WAKEUP_STATS_H

#include <linux/types.h>

enum dubai_irq_wakeup_type {
	DUBAI_IRQ_WAKEUP_TYPE_AP,
	DUBAI_IRQ_WAKEUP_TYPE_LPMCU,
};

extern void dubai_log_wakeup_info(const char *tag, const char *fmt, ...);
extern void dubai_log_irq_wakeup(enum dubai_irq_wakeup_type type, const char *source, int gpio);

#endif // DUBAI_WAKEUP_STATS_H
