/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: Contexthub softtimer driver.
 * Create: 2016-03-15
 */

#ifndef __SMART_CONTEXTHUB_SOFTTIMER_H__
#define __SMART_CONTEXTHUB_SOFTTIMER_H__

#include <linux/list.h>

#define TIMERLOAD(n)                        ((n) << 5)
#define TIMERVALUE(n)                       (((n) << 5) + 0x04)
#define TIMERCTRL(n)                        (((n) << 5) + 0x08)
#define TIMERINTCLR(n)                      (((n) << 5) + 0x0C)
#define TIMERRIS(n)                         (((n) << 5) + 0x10)
#define TIMERMIS(n)                         (((n) << 5) + 0x14)
#define TIMERBGLOAD(n)                      (((n) << 5) + 0x18)

#define TIMER_EN_ACK                        (1<<4)
#define TIMER_INT_MASK                      (1<<2)
#define TIMER_MODE_PERIOD                   (1<<1)
#define HARD_TIMER_ENABLE                   1
#define HARD_TIMER_DISABLE                  0
#define TIMER_ONCE_COUNT                    0
#define TIMER_PERIOD_COUNT                  1

typedef void (*softtimer_func)(unsigned long);

struct softtimer_list {
	softtimer_func func;
	unsigned long para;
	unsigned int timeout;
	unsigned int timer_type;
	struct list_head entry;
	unsigned int count_num;
	unsigned int is_base_timer;
	unsigned int init_flags;
};
/*
 * Add soft timer
 */
extern void smart_softtimer_add(struct softtimer_list *timer);
/*
 * Delete soft timer
 */
extern int smart_softtimer_delete(struct softtimer_list *timer);
/*
 * Create a soft timer
 */
extern int smart_softtimer_create(struct softtimer_list *timer, softtimer_func func,
		unsigned long para, unsigned int timeout);
/*
 * Modify soft timer
 */
extern int smart_softtimer_modify(struct softtimer_list *timer, unsigned int new_expire_time);
#endif
