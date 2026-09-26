/*
 * npu_dfx_log.h
 *
 * about npu dfx log
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef _LOG_DRV_DEV_H_
#define _LOG_DRV_DEV_H_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/iommu.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <asm/barrier.h>
#include <linux/delay.h>
#include <linux/wait.h>

#include "linux/smp.h"
#include "linux/kernel.h"

#define LOG_DEVICE_MAX        64
#define LOG_CHANNEL_MAX       64
#define LOG_CHANNEL_MAX_TYPE  3
#define LOG_CHANNEL_MAX_ASYNC 2
#define LOG_BUF_MIN_SIZE      256
#define LOG_CHANNEL_MAX_NAME  32
#define LOG_GET_SEM_TIMES_MAX 3
#define LOG_BUF_MAX_SIZE      (1024 * 1024)
#define LOG_CACHE_LINE        64
#define LOG_POLL_DEPTH        512
#define LOG_WAIT_TS_NUM       50
#define LOG_SQ_SLOT_LEN       128
#define LOG_CQ_SLOT_LEN       32
#define LOG_MEMORY_ALIGN      4

#define LOG_BUF_SIZE      512

#define CHAR_DRIVER_NAME     "log_drv"
#define LOG_DRV_MODE_VERSION "NPU 1.10"

#define LOG_CHANNEL_DISABLE 0
#define LOG_CHANNEL_ENABLE  1

#define LOG_MODELE_EXIT  0
#define LOG_NPU_POWERDOWN  1

#define TIME_UNIT      (HZ)
#define TS2DRV_TIMEOUT ((HZ) / 100)       /* 0.01 second */
#define DEV_UNUSED     0
#define DEV_USED       1
#define LOG_PRINT_MAX 5

#define IDE_DRV_OK 0
#define IDE_DRV_ERROR (-1)
#define IDE_DRV_TIMEOUT (-2)

#define LOG_CHANNEL_TYPE_TS     0
#define LOG_CHANNEL_TYPE_LPM3   1
#define LOG_CHANNEL_TYPE_TEE    2

#define LOG_LPM3_BUF_BASE 0xa00000
#define LOG_LPM3_BUF_LEN  (128 * 1024)

#define LOG_LEVEL_DEFAULT 2     /* log level is warning */

typedef enum {
	LOG_TEST = 0,
	LOG_CREATE,
	LOG_DELETE,
	LOG_SET,
	LOG_SHOW,
	LOG_READ,
	LOG_POLL,
	LOG_DEVICE_INFO,
	LOG_DEVICE_ID
} log_op_type_t;

/* #pragma pack(1) */
enum log_channel_id_enum {
	LOG_CHANNEL_TS_ID = 0,
	LOG_CHANNEL_AICPU_ID = 10
};

enum log_channel_index_enum {
	LOG_CHANNEL_TS_IDX = 0,
	LOG_CHANNEL_AICPU_IDX = 1,
	LOG_CHANNEL_NUM // for TS log and AICPU log
};

typedef struct device_info_t {
	unsigned int dev_num;
	unsigned int core_num;
	unsigned int cpu_num;
} device_info_t;

typedef struct log_device_info_t {
	unsigned char  cmd_verify;
	unsigned char  device_state;
	unsigned short device_id;
	unsigned int   poll_head;
	unsigned int   poll_tail;
} log_device_info_t;

/* attribute: size:24 Byte */
typedef struct attribute_t {
	/* log level setting, by ide decide;
	 * ERROR   3
	 * WARNING 2
	 * INFO    1
	 * DEBUG   0
	 */
	unsigned int log_level;
	/* register polling attributes */
	unsigned int   interval;    /* loop interval */
	unsigned int   period;
	unsigned int   reg_num;     /* register polling num */
	unsigned char *reg_list;    /* register list with size num */
} attribute_t;

typedef struct log_ioctl_para_t {
	int device_id;
	int channel_type;
	int channel_id;
	int async;
	int number;  /* poll structure's number */
	int ret;
	unsigned int timeout;
	unsigned int buf_size;
	unsigned long long buf_phy;
	char *out_buf;
	char channel_name[LOG_CHANNEL_MAX_NAME];
	attribute_t attrib;
	device_info_t dev_info;
} log_ioctl_para_t;

typedef struct log_channel_info_t {
	int channel_state;
	int channel_type;
	unsigned int channel_id;
	int poll_flag;
	unsigned int buf_size;
	unsigned long phy_addr;
	unsigned char *vir_addr;
	log_ioctl_para_t ioctl_para;
	int ret_val;
	int print_num;
	/* for dlogd's multi thread; the cmds are create/delete/read/set */
	struct mutex cmd_mutex;
} log_channel_info_t;

/* sq info;   size: 48 Byte */
typedef struct log_sq_scheduler_t {
	unsigned int  cmd_verify;
	unsigned int  channel_id;
	unsigned int  channel_cmd;
	unsigned int  buf_len;  /* the actual buffer length for data */
	unsigned long phy_addr;
	attribute_t   attrib;
} log_sq_scheduler_t;

/* cq info;   size: 16 Byte */
typedef struct log_cq_scheduler_t {
	unsigned int cmd_verify;
	unsigned int channel_id;
	unsigned int channel_cmd;
	unsigned int ret_val;
} log_cq_scheduler_t;

typedef struct log_buf_ptr_t {
	volatile unsigned int buf_read;
	/* the malloc buffer length, and include head structure */
	volatile unsigned int buf_len;
	volatile unsigned int log_level;
	volatile unsigned int rev[13];
	volatile unsigned int buf_write;
	volatile unsigned int rev2[15];
} log_buf_ptr_t;

#define DATA_BUF_HEAD sizeof(log_buf_ptr_t)

typedef struct log_poll_info_t {
	unsigned int device_id;
	unsigned int channel_idx;
} log_poll_info_t;

enum drv_type {
	DRV_LOG,
	DRV_DEBUG,
	DRV_PROFILE,
	DRV_TYPE_MAX
};

enum channel_poll_type {
	POLL_INVALID,
	POLL_VALID
};

typedef struct log_lpm3_channel_info {
	int device_id;
	int channel_id;
	struct notifier_block nb;
	int flag;
} log_lpm3_channel_info_t;

struct char_device {
	struct class *dev_class;
	struct cdev cdev;
	dev_t devno;
};

typedef struct log_char_dev_global_info {
	struct char_device char_dev_log;
	struct log_device_info_t log_dev_info;
	struct log_channel_info_t log_channel[LOG_CHANNEL_NUM];
	struct log_poll_info_t *poll_box;
	struct device *log_dev;
	unsigned int op_cb_idx;    // callback index for open npu
	unsigned int rl_cb_idx;    // callback index for release npu
} log_char_dev_global_info_t;

void log_flush_cache(const unsigned char *base, unsigned int len);

int log_check_ioctl_para(const struct log_ioctl_para_t *log_para);

#endif /* _LOG_DRV_DEV_ */
