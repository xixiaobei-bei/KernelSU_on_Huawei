

#ifndef HWSDP_IOCTL_TEEC_H
#define HWSDP_IOCTL_TEEC_H
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include "teek_client_api.h"
#include "huawei_platform/hwsdp/hwsdp_ioctl.h"

#define HWSDP_TA_UUID { \
	0x596116b6, 0x1d7c, 0x46d8, \
	{ 0xb5, 0x40, 0xea, 0xaa, 0x52, 0xe7, 0x59, 0x79 } \
}

typedef struct {
	struct work_struct work;
	wait_queue_head_t notify_waitq;
	TEEC_Operation op;
	u32 cmd;
	TEEC_Result result;
	s32 resp;
} hwsdp_teec_work_t;

s32 hwsdp_init_tee(void);
s32 hwsdp_teec_ops(hwsdp_ioctl_data_t *kdata);
s32 init_hwsdp_work_queue(void);
void destory_hwsdp_work_resource(void);

#endif

