#ifndef __PLATFORM_H__
#define	__PLATFORM_H__

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#include "regulator.h"

#define RETURN_FAIL_IF_COND_IS_TRUE(cond, str)  \
do {                                       \
    if (cond)                              \
    {                                      \
        dprint(PRN_FATAL, "[%s : %d]- %s\n", __func__, __LINE__, str); \
        return HI_FAILURE;                 \
    }                                      \
}while(0)

#define VDEC_INIT_MUTEX(lock)              \
do {                                       \
	mutex_init(lock);                  \
} while(0)

#define VDEC_MUTEX_LOCK(lock)              \
do {                                       \
	mutex_lock(lock);                  \
} while(0)

#define VDEC_MUTEX_UNLOCK(lock)            \
do {                                       \
	mutex_unlock(lock);                \
} while(0)

#endif
