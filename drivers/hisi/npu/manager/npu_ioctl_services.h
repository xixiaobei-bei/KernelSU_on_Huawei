#ifndef __NPU_IOCTL_SERVICE_H
#define __NPU_IOCTL_SERVICE_H
#include <linux/cdev.h>

#include "npu_proc_ctx.h"

#define DEVDRV_ID_MAGIC    'D'
#define MAX_NODE_NUM   			4
#define MAX_NODE_NUM_OF_FRAME     8

#define DEVDRV_ALLOC_STREAM_ID	_IO(DEVDRV_ID_MAGIC, 1)
#define DEVDRV_FREE_STREAM_ID	_IO(DEVDRV_ID_MAGIC, 2)
#define DEVDRV_ALLOC_EVENT_ID	_IO(DEVDRV_ID_MAGIC, 3)
#define DEVDRV_FREE_EVENT_ID	_IO(DEVDRV_ID_MAGIC, 4)
#define DEVDRV_REPORT_WAIT	_IO(DEVDRV_ID_MAGIC, 5)
#define DEVDRV_GET_UIO_NUM	_IO(DEVDRV_ID_MAGIC, 6)
#define DEVDRV_MAILBOX_SEND	_IO(DEVDRV_ID_MAGIC, 7)
#define DEVDRV_MAILBOX_WAIT	_IO(DEVDRV_ID_MAGIC, 8)
#define DEVDRV_MAILBOX_GET	_IO(DEVDRV_ID_MAGIC, 9)
#define DEVDRV_MAILBOX_END	_IO(DEVDRV_ID_MAGIC, 10)
#define DEVDRV_ALLOC_MODEL_ID	_IO(DEVDRV_ID_MAGIC, 11)
#define DEVDRV_FREE_MODEL_ID	_IO(DEVDRV_ID_MAGIC, 12)
#define DEVDRV_ALLOC_NOTIFY_ID    	_IO(DEVDRV_ID_MAGIC, 13)
#define DEVDRV_FREE_NOTIFY_ID      	_IO(DEVDRV_ID_MAGIC, 14)
#define DEVDRV_GET_NOTIFY_ID_ADDR   _IO(DEVDRV_ID_MAGIC, 15)

#define DEVDRV_TSMSG_SEND_SYNC		_IO(DEVDRV_ID_MAGIC, 20)
#define DEVDRV_TSMSG_SEND_ASYNC	_IO(DEVDRV_ID_MAGIC, 21)
#define DEVDRV_TSMSG_ASYNC_REGISTER	_IO(DEVDRV_ID_MAGIC, 22)
#define DEVDRV_TSMSG_ASYNC_UNREGISTER	_IO(DEVDRV_ID_MAGIC, 23)
#define DEVDRV_TSMSG_RECEIVE_ASYNC	_IO(DEVDRV_ID_MAGIC, 24)
#define DEVDRV_GET_OCCUPY_STREAM_ID	_IO(DEVDRV_ID_MAGIC, 25)
#define DEVDRV_TSMSG_RECV_ASYNC   _IO(DEVDRV_ID_MAGIC, 26)
#define DEVDRV_ALLOC_TASK_ID        _IO(DEVDRV_ID_MAGIC, 27)
#define DEVDRV_FREE_TASK_ID         _IO(DEVDRV_ID_MAGIC, 28)
#define DEVDRV_GET_TS_TIMEOUT_ID		_IO(DEVDRV_ID_MAGIC, 30)

#define DEVDRV_CUSTOM_IOCTL				_IO(DEVDRV_ID_MAGIC, 64)

#define DEVDRV_MAX_CMD	65

int copy_from_user_safe(void *to, const void __user *from, unsigned long len);

int copy_to_user_safe(void __user *to, const void *from, unsigned long n);

long devdrv_npu_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

int devdrv_ioctl_alloc_stream(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_get_occupy_stream_id(struct devdrv_proc_ctx *proc_ctx,
													unsigned long arg);

int devdrv_ioctl_alloc_event(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_alloc_model(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_alloc_task(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_free_stream(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_free_event(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_free_model(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_free_task(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_mailbox_send(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_report_wait(struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_custom( struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

int devdrv_ioctl_get_ts_timeout( struct devdrv_proc_ctx *proc_ctx, unsigned long arg);

extern int devdrv_proc_npu_ioctl_call(struct devdrv_proc_ctx *proc_ctx,
	unsigned int cmd, unsigned long arg);

#endif /*__DEVDRV_MANAGER_H*/
