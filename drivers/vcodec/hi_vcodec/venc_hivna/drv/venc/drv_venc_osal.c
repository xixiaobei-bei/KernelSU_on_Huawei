/*
 * drv_venc_osal.c
 *
 * This is for venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "drv_venc_osal.h"
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/sched/clock.h>

/*lint -e747 -e712 -e732 -e715 -e774 -e845 -e438 -e838*/

int32_t venc_drv_osal_irq_init(uint32_t irq, irqreturn_t (*callback)(int32_t, void *))
{
	int32_t ret;

	if (irq == 0) {
		HI_FATAL_VENC("params is invaild");
		return HI_FAILURE;
	}

	ret = request_irq(irq, callback, 0, "DT_device", NULL);
	if (ret) {
		HI_FATAL_VENC("request irq failed");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

void venc_drv_osal_irq_free(uint32_t irq)
{
	free_irq(irq, NULL);
}

int32_t venc_drv_osal_lock_create(spinlock_t **phlock)
{
	spinlock_t *plock = NULL;

	plock = vmalloc(sizeof(spinlock_t));
	if (!plock) {
		HI_FATAL_VENC("vmalloc failed");
		return HI_FAILURE;
	}

	spin_lock_init(plock);
	*phlock = plock;

	return HI_SUCCESS;
}

void venc_drv_osal_lock_destroy(spinlock_t *hlock)
{
	if (hlock)
		vfree((void *)hlock);
}

int32_t venc_drv_osal_init_event(vedu_osal_event_t *event, int32_t initval)
{
	event->flag = initval;
	init_waitqueue_head(&(event->queue_head));
	return HI_SUCCESS;
}

int32_t venc_drv_osal_give_event(vedu_osal_event_t *event)
{
	event->flag = 1;
	wake_up_all(&(event->queue_head));
	return HI_SUCCESS;
}

uint64_t osal_get_sys_time_in_ms(void)
{
	uint64_t sys_time;

	sys_time = sched_clock();
	do_div(sys_time, 1000000);

	return sys_time;
}

uint64_t osal_get_sys_time_in_us(void)
{
	uint64_t sys_time;

	sys_time = sched_clock();
	do_div(sys_time, 1000);

	return sys_time;
}

uint64_t osal_get_during_time(uint64_t start_time)
{
	uint64_t end_time = osal_get_sys_time_in_us();

	return time_period(start_time, end_time);
}

void hi_sleep_ms(uint32_t millisec)
{
	msleep(millisec);
}

uint32_t *hi_mmap(uint32_t addr, uint32_t range)
{
	uint32_t *res_addr = NULL;

	res_addr = (uint32_t *)ioremap(addr, range);//lint !e446
	return res_addr;
}

void hi_munmap(uint32_t *mem_addr)
{
	if (!mem_addr) {
		HI_FATAL_VENC("params is invaild");
		return;
	}

	iounmap(mem_addr);
}

int32_t hi_strncmp(const char *str_name, const char *dst_name, int32_t size)
{
	if (str_name && dst_name) {
		return strncmp(str_name, dst_name, size);
	}

	return HI_FAILURE;
}

void *hi_mem_valloc(uint32_t mem_size)
{
	void *mem_addr = NULL;

	if (mem_size)
		mem_addr = vmalloc(mem_size);

	return mem_addr;
}

void hi_mem_vfree(const void *mem_addr)
{
	if (mem_addr)
		vfree(mem_addr);
}

void hi_venc_init_sem(void *sem)
{
	if (sem)
		sema_init((struct semaphore *)sem, 1);
}

int32_t hi_venc_down_interruptible(void *sem)
{
	int32_t ret = -1;

	if (sem)
		ret = down_interruptible((struct semaphore *)sem);

	if (ret)
		HI_FATAL_VENC("ret %d down interruptible fail", ret);

	return  ret;
}

void  hi_venc_up_interruptible(void *sem)
{
	if (sem)
		up((struct semaphore *)sem);
}

void osal_init_timer(struct timer_list *timer,
		void (*function)(unsigned long),
		unsigned long data)
{
	if (timer == NULL) {
		HI_FATAL_VENC("input timer is availed");
		return;
	}

	if (function == NULL) {
		HI_FATAL_VENC("input callback function is availed");
		return;
	}

	setup_timer(timer, function, data);
}

void osal_add_timer(struct timer_list *timer, uint64_t time_in_ms)
{
	if (timer == NULL) {
		HI_FATAL_VENC("input timer is availed");
		return;
	}

	mod_timer(timer, jiffies + msecs_to_jiffies(time_in_ms));
}

int32_t osal_del_timer(struct timer_list *timer, bool is_sync)
{
	if (timer == NULL) {
		HI_FATAL_VENC("input timer is availed");
		return HI_FAILURE;
	}

	if (!timer_pending(timer))
		return HI_FAILURE;

	if (is_sync)
		del_timer_sync(timer);
	else
		del_timer(timer);

	return HI_SUCCESS;
}
