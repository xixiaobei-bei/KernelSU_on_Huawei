/*
 * drv_mailbox_platform.h
 *
 * platform definitions of mailbox driver
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

#ifndef __DRV_MAILBOX_PLATFORM_H__
#define __DRV_MAILBOX_PLATFORM_H__

#include <linux/semaphore.h>
#include <linux/spinlock.h>

#if defined(BSP_CORE_MODEM) /* C core system adaptation header file */
#include "drv_mailbox_port_vxworks.h"

#elif defined(BSP_CORE_APP) /* A core system adaptation header file */
#include "drv_mailbox_port_linux.h"

#elif defined(BSP_CORE_CM3) /* M core system adaptation header file */
#include "drv_mailbox_port_mcu.h"
#endif

#ifndef MAILBOX_TRUE
#define MAILBOX_TRUE 1
#endif

#ifndef MAILBOX_FALSE
#define MAILBOX_FALSE 0
#endif

#ifndef NULL
#define NULL (void *)(0)
#endif

/* Define the number and direction of the mailbox channel array */
#define MB_DIRECTION_INVALID 0
#define MB_DIRECTION_SEND 1
#define MB_DIRECTION_RECEIVE 2

#define MAILBOX_MAILCODE_INVALID 0xffffffffUL

enum MAILBOX_DELAY_SCENE_E {
	MAILBOX_DELAY_SCENE_MSG_FULL,
	MAILBOX_DELAY_SCENE_IFC_FULL
};

struct mb_mutex {
	uint32_t type;

	struct semaphore sem;

	spinlock_t splock;
	unsigned long flags;
};

/* Interface called between mailbox internal files */
void mailbox_assert(uint32_t err_no);

void mailbox_mutex_lock(struct mb_mutex *mutex);

void mailbox_mutex_unlock(struct mb_mutex *mutex);

int mailbox_int_context(void);

int mailbox_get_timestamp(void);

int mailbox_process_register(uint32_t channel_id,
	int (*cb)(uint32_t channel_id), void *priv);

int mailbox_channel_register(uint32_t channel_id, uint32_t int_src,
	uint32_t dst_id, uint32_t direct, struct mb_mutex *mutex);

void *mailbox_memcpy(void *dst, const void *src, uint32_t size);

void *mailbox_memset(void *m, int c, uint32_t size);

int mailbox_delivery(uint32_t channel_id);

void mailbox_local_work_free(void);

void mailbox_destory_platform(void);

int mailbox_init_platform(void);

int mailbox_scene_delay(uint32_t scene_id, int *try_times);

#if defined(BSP_CORE_CM3)
void mailbox_dpm_device_get(void);
void mailbox_dpm_device_put(void);
#else
#define mailbox_dpm_device_get()
#define mailbox_dpm_device_put()
#endif

#endif /* __DRV_MAILBOX_PLATFORM_H__ */
