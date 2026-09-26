
#ifndef __VFMW_LINUX_KERNEL_OSAL_HEADER__
#define  __VFMW_LINUX_KERNEL_OSAL_HEADER__

#include <asm/cacheflush.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/wait.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <linux/semaphore.h>

#include "vfmw_osal_ext.h"

/*======================================================================*/
/*                            struct define                             */
/*======================================================================*/
typedef struct hiKERN_EVENT_S {
	wait_queue_head_t queue_head;
	SINT32 flag;
} OSAL_EVENT;

typedef struct hiKERN_IRQ_LOCK_S {
	spinlock_t    irq_lock;
	unsigned long irq_lockflags;
	int           isInit;
} OSAL_IRQ_SPIN_LOCK;

/*======================================================================*/
/*                              define                                  */
/*======================================================================*/
typedef  struct task_struct*    OSAL_TASK;
typedef  struct file            OSAL_FILE;
typedef  struct semaphore       OSAL_SEMA;
typedef  OSAL_EVENT             OSAL_TASK_MUTEX;


/*======================================================================*/
/*                          function declare                            */
/*======================================================================*/

/************************************************************************/
/* time: get in ms/us */
/************************************************************************/
UINT32     OSAL_GetTimeInMs(VOID);
UINT32     OSAL_GetTimeInUs(VOID);

/************************************************************************/
/* file: open/close/read/write */
/************************************************************************/
SINT32     OSAL_FileWrite(char *buf, int len, struct file *filp);

/************************************************************************/
/* linux kernel osal function pointer initialize */
/************************************************************************/
VOID       OSAL_InitInterface(VOID);

#endif


