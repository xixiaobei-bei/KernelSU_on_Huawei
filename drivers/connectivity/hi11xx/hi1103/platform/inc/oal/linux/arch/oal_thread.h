

#ifndef __OAL_LINUX_THREAD_H__
#define __OAL_LINUX_THREAD_H__

/* 其他头文件包含 */
#include <asm/atomic.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <asm/param.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/kthread.h>

/* STRUCT定义 */
typedef struct task_struct oal_task_stru;


/* 宏定义 */
#define oal_kthread_create      kthread_create
#define oal_kthread_bind        kthread_bind
#define oal_kthread_stop        kthread_stop
#define oal_kthread_run         kthread_run
#define oal_kthread_should_stop kthread_should_stop
#define oal_schedule            schedule
#define oal_wake_up_process     wake_up_process
#define oal_set_current_state   set_current_state
#define oal_cond_resched        cond_resched

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5610EVB) || \
     (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5610DMB) || \
     (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_VSPM310DMB) || \
     (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_WS835DMB))
#define oal_sched_setscheduler sched_setscheduler_export
#elif (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_1102COMMON)
#define oal_sched_setscheduler sched_setscheduler
#endif
#define oal_set_user_nice set_user_nice

#endif
