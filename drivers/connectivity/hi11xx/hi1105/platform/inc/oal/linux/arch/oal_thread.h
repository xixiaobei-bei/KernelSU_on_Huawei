

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
#define oal_kthread_create(threadfn, data, namefmt, arg) kthread_create(threadfn, data, namefmt, arg)
#define oal_kthread_bind(task, cpu) kthread_bind(task, cpu)
#define oal_kthread_stop(task) kthread_stop(task)
#define oal_kthread_should_stop() kthread_should_stop()
#define oal_schedule() schedule()
#define oal_wake_up_process(task) wake_up_process(task)

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5610EVB) || \
     (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5610DMB) || \
     (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_VSPM310DMB) || \
     (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_WS835DMB))
#define oal_sched_setscheduler(task, policy, param) sched_setscheduler_export(task, policy, param)
#elif (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_1102COMMON)
#define oal_sched_setscheduler(task, policy, param) sched_setscheduler(task, policy, param)
#endif
#define oal_set_user_nice(task, nice) set_user_nice(task, nice)

#endif
