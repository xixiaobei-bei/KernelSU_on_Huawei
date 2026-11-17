/*
 * timer.c
 *
 * timer for pd/tcpc
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "inc/timer.h"
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/version.h>
#include <linux/sched/rt.h>
#include <uapi/linux/sched/types.h>
#include "inc/tcpci_event.h"
#include "inc/tcpc_core.h"

#define rt_mask64(i)            (((uint64_t)1) << (i))
#define timeout_val(val)        ((val) * 1000)
#define timeout_range(min, max) (((min) * 4000 + (max) * 1000) / 5)
#define timeout_val_us(val)     (val)
#define decl_to(ms)             timeout_val(ms)
#define decl_tou(us)            timeout_val_us(us)
#define decl_tor(min, max)      timeout_range(min, max)

typedef enum hrtimer_restart (*hrtimer_call)(struct hrtimer *timer);

struct timer_config {
	const char *name;
	hrtimer_call call;
	uint32_t timeouts;
};

struct timer_device {
	struct tcpc_device *tcpc;
	struct mutex lock;
	struct semaphore enable_mask_lock;
	spinlock_t tick_lock;
	uint64_t tick;
	uint64_t enable_mask;
	wait_queue_head_t  wait_que;
	struct task_struct *task;
	bool thread_stop;
	struct hrtimer list[PD_TIMER_NR];
};

static uint64_t timer_get_enable_mask(struct timer_device *tmr)
{
	uint64_t data;
	unsigned long flags;

	down(&tmr->enable_mask_lock);
	local_irq_save(flags);
	data = tmr->enable_mask;
	local_irq_restore(flags);
	up(&tmr->enable_mask_lock);

	return data;
}

static void timer_reset_enable_mask(struct timer_device *tmr)
{
	unsigned long flags;

	down(&tmr->enable_mask_lock);
	local_irq_save(flags);
	tmr->enable_mask = 0;
	local_irq_restore(flags);
	up(&tmr->enable_mask_lock);
}

static void timer_clear_enable_mask(struct timer_device *tmr, int nr)
{
	unsigned long flags;

	down(&tmr->enable_mask_lock);
	local_irq_save(flags);
	tmr->enable_mask &= ~rt_mask64(nr);

	spin_lock(&tmr->tick_lock);
	tmr->tick &= ~rt_mask64(nr);
	spin_unlock(&tmr->tick_lock);

	local_irq_restore(flags);
	up(&tmr->enable_mask_lock);
}

static void timer_set_enable_mask(struct timer_device *tmr, int nr)
{
	unsigned long flags;

	down(&tmr->enable_mask_lock);
	local_irq_save(flags);
	tmr->enable_mask |= rt_mask64(nr);
	local_irq_restore(flags);
	up(&tmr->enable_mask_lock);
}

static uint64_t timer_get_tick(struct timer_device *tmr)
{
	uint64_t data;
	unsigned long flags;

	spin_lock_irqsave(&tmr->tick_lock, flags);
	data = tmr->tick;
	spin_unlock_irqrestore(&tmr->tick_lock, flags);

	return data;
}

static void timer_clear_tick(struct timer_device *tmr, int nr)
{
	unsigned long flags;

	spin_lock_irqsave(&tmr->tick_lock, flags);
	tmr->tick &= ~rt_mask64(nr);
	spin_unlock_irqrestore(&tmr->tick_lock, flags);
}

static void timer_set_tick(struct timer_device *tmr, int nr)
{
	unsigned long flags;

	spin_lock_irqsave(&tmr->tick_lock, flags);
	tmr->tick |= rt_mask64(nr);
	spin_unlock_irqrestore(&tmr->tick_lock, flags);
}

#define timer_to_device(timer, index) \
	container_of(timer, struct timer_device, list[index])

#define timer_trigger(tmr, index) \
do { \
	timer_set_tick(tmr, index); \
	wake_up_interruptible(&(tmr)->wait_que); \
} while (0)

#define timer_func(tag, index) \
static enum hrtimer_restart timer_##tag(struct hrtimer *timer) \
{ \
	struct timer_device *tmr = timer_to_device(timer, index); \
	timer_trigger(tmr, index); \
	return HRTIMER_NORESTART; \
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
timer_func(bist_cont_mode, PD_TIMER_BIST_CONT_MODE)
timer_func(discover_id, PD_TIMER_DISCOVER_ID)
timer_func(hard_reset_complete, PD_TIMER_HARD_RESET_COMPLETE)
timer_func(no_response, PD_TIMER_NO_RESPONSE)
timer_func(ps_hard_reset, PD_TIMER_PS_HARD_RESET)
timer_func(ps_source_off, PD_TIMER_PS_SOURCE_OFF)
timer_func(ps_source_on, PD_TIMER_PS_SOURCE_ON)
timer_func(ps_transition, PD_TIMER_PS_TRANSITION)
timer_func(sender_response, PD_TIMER_SENDER_RESPONSE)
timer_func(sink_activity, PD_TIMER_SINK_ACTIVITY)
timer_func(sink_request, PD_TIMER_SINK_REQUEST)
timer_func(sink_wait_cap, PD_TIMER_SINK_WAIT_CAP)
timer_func(source_activity, PD_TIMER_SOURCE_ACTIVITY)
timer_func(source_capability, PD_TIMER_SOURCE_CAPABILITY)
timer_func(source_start, PD_TIMER_SOURCE_START)
timer_func(vconn_on, PD_TIMER_VCONN_ON)

#ifdef CONFIG_USB_PD_VCONN_STABLE_DELAY
timer_func(vconn_stable, PD_TIMER_VCONN_STABLE)
#endif /* CONFIG_USB_PD_VCONN_STABLE_DELAY */

timer_func(vdm_mode_entry, PD_TIMER_VDM_MODE_ENTRY)
timer_func(vdm_mode_exit, PD_TIMER_VDM_MODE_EXIT)
timer_func(vdm_response, PD_TIMER_VDM_RESPONSE)
timer_func(source_transition, PD_TIMER_SOURCE_TRANSITION)
timer_func(src_recover, PD_TIMER_SRC_RECOVER)
timer_func(vsafe0v_delay, PD_TIMER_VSAFE0V_DELAY)
timer_func(pd_discard, PD_TIMER_DISCARD)
timer_func(vbus_stable, PD_TIMER_VBUS_STABLE)
timer_func(vbus_present, PD_TIMER_VBUS_PRESENT)
timer_func(uvdm_response, PD_TIMER_UVDM_RESPONSE)
timer_func(dfp_flow_delay, PD_TIMER_DFP_FLOW_DELAY)
timer_func(ufp_flow_delay, PD_TIMER_UFP_FLOW_DELAY)
timer_func(vconn_ready, PD_TIMER_VCONN_READY)
timer_func(vdm_postpone, PD_PE_VDM_POSTPONE)

#ifdef CONFIG_HW_USB_PD_REV30
timer_func(ck_no_support, PD_TIMER_CK_NO_SUPPORT)

#ifdef CONFIG_USB_PD_REV30_PPS_SOURCE
timer_func(source_pps, PD_TIMER_SOURCE_PPS)
#endif /* CONFIG_USB_PD_REV30_PPS_SOURCE */

#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
timer_func(sink_tx, PD_TIMER_SINK_TX)
timer_func(deferred_evt, PD_TIMER_DEFERRED_EVT)
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */

#ifdef CONFIG_USB_PD_REV30_SNK_FLOW_DELAY_STARTUP
timer_func(snk_flow_delay, PD_TIMER_SNK_FLOW_DELAY)
#endif /* CONFIG_USB_PD_REV30_SNK_FLOW_DELAY_STARTUP */
#endif /* CONFIG_HW_USB_PD_REV30 */

timer_func(pe_idle_tout, PD_TIMER_PE_IDLE_TOUT)
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

timer_func(rt_vsafe0v_tout, TYPEC_RT_TIMER_SAFE0V_TOUT)
timer_func(rt_role_swap_start, TYPEC_RT_TIMER_ROLE_SWAP_START)
timer_func(rt_role_swap_stop, TYPEC_RT_TIMER_ROLE_SWAP_STOP)
timer_func(rt_legacy, TYPEC_RT_TIMER_STATE_CHANGE)
timer_func(rt_not_legacy, TYPEC_RT_TIMER_NOT_LEGACY)
timer_func(rt_legacy_stable, TYPEC_RT_TIMER_LEGACY_STABLE)
timer_func(rt_legacy_recycle, TYPEC_RT_TIMER_LEGACY_RECYCLE)
timer_func(rt_auto_discharge, TYPEC_RT_TIMER_AUTO_DISCHARGE)
timer_func(rt_low_power_mode, TYPEC_RT_TIMER_LOW_POWER_MODE)

#ifdef CONFIG_HW_USB_POWER_DELIVERY
timer_func(rt_pe_idle, TYPEC_RT_TIMER_PE_IDLE)

#ifdef CONFIG_PD_WAIT_BC12
timer_func(rt_sink_wait_bc12, TYPEC_RT_TIMER_SINK_WAIT_BC12)
#endif /* CONFIG_PD_WAIT_BC12 */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

timer_func(try_drp_try, TYPEC_TRY_TIMER_DRP_TRY)
timer_func(try_drp_trywait, TYPEC_TRY_TIMER_DRP_TRYWAIT)
timer_func(ccdebounce, TYPEC_TIMER_CCDEBOUNCE)
timer_func(pddebounce, TYPEC_TIMER_PDDEBOUNCE)

#ifdef CONFIG_COMPATIBLE_APPLE_TA
timer_func(apple_cc_open, TYPEC_TIMER_APPLE_CC_OPEN)
#endif /* CONFIG_COMPATIBLE_APPLE_TA */

timer_func(tryccdebounce, TYPEC_TIMER_TRYCCDEBOUNCE)
timer_func(srcdisconnect, TYPEC_TIMER_SRCDISCONNECT)
timer_func(error_recovery, TYPEC_TIMER_ERROR_RECOVERY)
timer_func(drp_src_toggle, TYPEC_TIMER_DRP_SRC_TOGGLE)
timer_func(norp_src, TYPEC_TIMER_NORP_SRC)

static const struct timer_config g_timers[PD_TIMER_NR] = {
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	{ "PD_TIMER_DISCOVER_ID", timer_discover_id, decl_tor(30, 60) },
	{ "PD_TIMER_BIST_CONT_MODE", timer_bist_cont_mode, decl_tor(40, 50) },
	{ "PD_TIMER_HARD_RESET_COMPLETE", timer_hard_reset_complete, decl_tor(4, 5) },
	{ "PD_TIMER_NO_RESPONSE", timer_no_response, decl_tor(4500, 5500) },
	{ "PD_TIMER_PS_HARD_RESET", timer_ps_hard_reset, decl_tor(25, 35) },
	{ "PD_TIMER_PS_SOURCE_OFF", timer_ps_source_off, decl_tor(750, 920) },
	{ "PD_TIMER_PS_SOURCE_ON", timer_ps_source_on, decl_tor(390, 480) },
	{ "PD_TIMER_PS_TRANSITION", timer_ps_transition, decl_tor(450, 550) },
	{ "PD_TIMER_SENDER_RESPONSE", timer_sender_response, decl_tor(24, 30) },
	{ "PD_TIMER_SINK_ACTIVITY", timer_sink_activity, decl_tor(120, 150) },
	{ "PD_TIMER_SINK_REQUEST", timer_sink_request, decl_tor(100, 100) },
	{ "PD_TIMER_SINK_WAIT_CAP", timer_sink_wait_cap, decl_tor(310, 620) },
	{ "PD_TIMER_SOURCE_ACTIVITY", timer_source_activity, decl_tor(40, 50) },
	{ "PD_TIMER_SOURCE_CAPABILITY", timer_source_capability, decl_tor(100, 200) },
	{ "PD_TIMER_SOURCE_START", timer_source_start, decl_to(20) },
	{ "PD_TIMER_VCONN_ON", timer_vconn_on, decl_to(100) },

#ifdef CONFIG_USB_PD_VCONN_STABLE_DELAY
	{ "PD_TIMER_VCONN_STABLE", timer_vconn_stable, decl_to(50) },
#endif /* CONFIG_USB_PD_VCONN_STABLE_DELAY */

	{ "PD_TIMER_VDM_MODE_ENTRY", timer_vdm_mode_entry, decl_tor(40, 50) },
	{ "PD_TIMER_VDM_MODE_EXIT", timer_vdm_mode_exit, decl_tor(40, 50) },
	{ "PD_TIMER_VDM_RESPONSE", timer_vdm_response, decl_tor(24, 30) },
	{ "PD_TIMER_SOURCE_TRANSITION", timer_source_transition, decl_tor(25, 35) },
	{ "PD_TIMER_SRC_RECOVER", timer_src_recover, decl_tor(660, 1000) },
	{ "PD_TIMER_VSAFE0V_DELAY", timer_vsafe0v_delay, decl_to(400) },
	{ "PD_TIMER_DISCARD", timer_pd_discard, decl_to(3) },
	{ "PD_TIMER_VBUS_STABLE", timer_vbus_stable, decl_to(125) },
	{ "PD_TIMER_VBUS_PRESENT", timer_vbus_present, decl_to(20), },
	{ "PD_TIMER_UVDM_RESPONSE", timer_uvdm_response, decl_to(500) },
	{ "PD_TIMER_DFP_FLOW_DELAY", timer_dfp_flow_delay, decl_to(30) },
	{ "PD_TIMER_UFP_FLOW_DELAY", timer_ufp_flow_delay, decl_to(300) },
	{ "PD_TIMER_VCONN_READY", timer_vconn_ready, decl_to(5) },
	{ "PD_PE_VDM_POSTPONE", timer_vdm_postpone, decl_tou(3000) },

#ifdef CONFIG_HW_USB_PD_REV30
	{ "PD_TIMER_CK_NO_SUPPORT", timer_ck_no_support, decl_tor(40, 50) },
#ifdef CONFIG_USB_PD_REV30_PPS_SOURCE
	{ "PD_TIMER_SOURCE_PPS", timer_source_pps, decl_to(14000) },
#endif /* CONFIG_USB_PD_REV30_PPS_SOURCE */
#ifdef CONFIG_USB_PD_REV30_COLLISION_AVOID
	{ "PD_TIMER_SINK_TX", timer_sink_tx, decl_to(25) },
	{ "PD_TIMER_DEFERRED_EVT", timer_deferred_evt, decl_to(5000) },
#endif /* CONFIG_USB_PD_REV30_COLLISION_AVOID */
#ifdef CONFIG_USB_PD_REV30_SNK_FLOW_DELAY_STARTUP
	{ "PD_TIMER_SNK_FLOW_DELAY", timer_snk_flow_delay, decl_to(300) },
#endif /* CONFIG_USB_PD_REV30_SNK_FLOW_DELAY_STARTUP */
#endif /* CONFIG_HW_USB_PD_REV30 */

	{ "PD_TIMER_PE_IDLE_TOUT", timer_pe_idle_tout, decl_to(10) },
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	{ "TYPEC_RT_TIMER_SAFE0V_TOUT", timer_rt_vsafe0v_tout, decl_to(650) },
	{ "TYPEC_RT_TIMER_ROLE_SWAP_START", timer_rt_role_swap_start, decl_to(20) },
	{ "TYPEC_RT_TIMER_ROLE_SWAP_STOP", timer_rt_role_swap_stop, decl_to(1200) },
	{ "TYPEC_RT_TIMER_STATE_CHANGE", timer_rt_legacy, decl_to(50) },
	{ "TYPEC_RT_TIMER_NOT_LEGACY", timer_rt_not_legacy, decl_to(5000) },
	{ "TYPEC_RT_TIMER_LEGACY_STABLE", timer_rt_legacy_stable, decl_to(30000) },
	{ "TYPEC_RT_TIMER_LEGACY_RECYCLE", timer_rt_legacy_recycle, decl_to(300000) },
	{ "TYPEC_RT_TIMER_AUTO_DISCHARGE", timer_rt_auto_discharge, decl_to(50) },
	{ "TYPEC_RT_TIMER_LOW_POWER_MODE", timer_rt_low_power_mode, decl_to(500) },

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	{ "TYPEC_RT_TIMER_PE_IDLE", timer_rt_pe_idle, decl_to(1) },
#ifdef CONFIG_PD_WAIT_BC12
	{ "TYPEC_RT_TIMER_SINK_WAIT_BC12", timer_rt_sink_wait_bc12, decl_to(50) },
#endif /* CONFIG_PD_WAIT_BC12 */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	{ "TYPEC_TIMER_ERROR_RECOVERY", timer_error_recovery, decl_tor(25, 25) },
	{ "TYPEC_TRY_TIMER_DRP_TRY", timer_try_drp_try, decl_tor(75, 150) },
	{ "TYPEC_TRY_TIMER_DRP_TRYWAIT", timer_try_drp_trywait, decl_tor(400, 800) },
	{ "TYPEC_TIMER_CCDEBOUNCE", timer_ccdebounce, decl_tor(100, 200) },
	{ "TYPEC_TIMER_PDDEBOUNCE", timer_pddebounce, decl_tor(10, 10) },
#ifdef CONFIG_COMPATIBLE_APPLE_TA
	{ "TYPEC_TIMER_APPLE_CC_OPEN", timer_apple_cc_open, decl_tor(200, 200) },
#endif /* CONFIG_COMPATIBLE_APPLE_TA */
	{ "TYPEC_TIMER_TRYCCDEBOUNCE", timer_tryccdebounce, decl_tor(10, 10) },
	{ "TYPEC_TIMER_SRCDISCONNECT", timer_srcdisconnect, decl_to(5) },
	{ "TYPEC_TIMER_DRP_SRC_TOGGLE", timer_drp_src_toggle, decl_to(60) },
	{ "TYPEC_TIMER_NORP_SRC", timer_norp_src, decl_to(300) },
};

static void timer_reset_range(struct timer_device *tmr, int start, int end)
{
	int i;
	uint64_t mask = timer_get_enable_mask(tmr);

	for (i = start; i < end; i++) {
		if (mask & rt_mask64(i)) {
			hrtimer_try_to_cancel(&tmr->list[i]);
			timer_clear_enable_mask(tmr, i);
		}
	}

	if (end == PD_TIMER_NR)
		typec_disable_wakeup_timer(tmr->tcpc);
}

void timer_restart(struct tcpc_device *tcpc, uint32_t timer_id)
{
	uint64_t mask;
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer || (timer_id >= PD_TIMER_NR))
		return;
	tmr = tcpc->timer;

	mask = timer_get_enable_mask(tmr);
	if (mask & rt_mask64(timer_id))
		timer_disable(tcpc, timer_id);

	timer_enable(tcpc, timer_id);
}

void timer_enable(struct tcpc_device *tcpc, uint32_t timer_id)
{
	uint32_t r, mod, tout;
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer || (timer_id >= PD_TIMER_NR))
		return;
	tmr = tcpc->timer;

	TIMER_DBG("Enable %s\n", g_timers[timer_id].name);
	mutex_lock(&tmr->lock);
	if (timer_id >= TYPEC_TIMER_START_ID)
		timer_reset_range(tmr, TYPEC_TIMER_START_ID, PD_TIMER_NR);

	timer_set_enable_mask(tmr, timer_id);
	tout = g_timers[timer_id].timeouts;

#ifdef CONFIG_USB_PD_RANDOM_FLOW_DELAY
	if ((timer_id == PD_TIMER_DFP_FLOW_DELAY) ||
		(timer_id == PD_TIMER_UFP_FLOW_DELAY))
		tout += timeout_val(jiffies & 0x07);
#endif /* CONFIG_USB_PD_RANDOM_FLOW_DELAY */
	mutex_unlock(&tmr->lock);

	/* 1000000us of 1s */
	r =  tout / 1000000;
	mod = tout % 1000000;
	/* 1000ns of 1us */
	hrtimer_start(&tmr->list[timer_id], ktime_set(r, mod * 1000),
		HRTIMER_MODE_REL);
}

void timer_disable(struct tcpc_device *tcpc, uint32_t timer_id)
{
	uint64_t mask;
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer || (timer_id >= PD_TIMER_NR))
		return;
	tmr = tcpc->timer;

	mask = timer_get_enable_mask(tmr);
	if (mask & rt_mask64(timer_id)) {
		hrtimer_try_to_cancel(&tmr->list[timer_id]);
		timer_clear_enable_mask(tmr, timer_id);
	}
}

void timer_reset(struct tcpc_device *tcpc)
{
	uint64_t mask;
	int i;
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer)
		return;
	tmr = tcpc->timer;

	mask = timer_get_enable_mask(tmr);
	for (i = 0; i < PD_TIMER_NR; i++) {
		if (mask & rt_mask64(i))
			hrtimer_try_to_cancel(&tmr->list[i]);
	}

	timer_reset_enable_mask(tmr);
}

void timer_reset_typec_debt(struct tcpc_device *tcpc)
{
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer)
		return;
	tmr = tcpc->timer;

	mutex_lock(&tmr->lock);
	timer_reset_range(tmr, TYPEC_TIMER_START_ID, PD_TIMER_NR);
	mutex_unlock(&tmr->lock);
}

void timer_reset_pe_tmr(struct tcpc_device *tcpc)
{
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer)
		return;
	tmr = tcpc->timer;

	mutex_lock(&tmr->lock);
	timer_reset_range(tmr, 0, PD_PE_TIMER_END_ID);
	mutex_unlock(&tmr->lock);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
}

static void timer_handle_triggered(struct timer_device *tmr)
{
	int i = 0;
	uint64_t enable_mask = timer_get_enable_mask(tmr);
	uint64_t triggered_timer = timer_get_tick(tmr);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	for (i = 0; i < PD_PE_TIMER_END_ID; i++) {
		if (triggered_timer & rt_mask64(i)) {
			TIMER_DBG("Trigger %s\n", g_timers[i].name);
			if (enable_mask & rt_mask64(i)) {
				timer_disable(tmr->tcpc, i);
				pd_handle_timer_out(tmr->tcpc, i);
			}
			timer_clear_tick(tmr, i);
		}
	}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	for (; i < PD_TIMER_NR; i++) {
		if (triggered_timer & rt_mask64(i)) {
			TIMER_DBG("Trigger %s\n", g_timers[i].name);
			if (enable_mask & rt_mask64(i)) {
				timer_disable(tmr->tcpc, i);
				typec_handle_timeout(tmr->tcpc, i);
			}
			timer_clear_tick(tmr, i);
		}
	}
}

static int timer_thread(void *param)
{
	struct timer_device *tmr = param;
	uint64_t *timer_tick = &tmr->tick;
	struct sched_param sch_param = { .sched_priority = MAX_RT_PRIO - 1 };

	sched_setscheduler(current, SCHED_FIFO, &sch_param);
	while (true) {
		wait_event_interruptible(tmr->wait_que, *timer_tick ||
			tmr->thread_stop);
		if (kthread_should_stop() || tmr->thread_stop)
			break;

		do {
			timer_handle_triggered(tmr);
		} while (*timer_tick);
	}

	return 0;
}

int timer_init(struct tcpc_device *tcpc)
{
	int i;
	struct timer_device *tmr = NULL;

	if (!tcpc)
		return -EINVAL;

	tmr = kzalloc(sizeof(*tmr), GFP_KERNEL);
	if (!tmr)
		return -ENOMEM;
	tcpc->timer = tmr;
	tmr->tcpc = tcpc;

	pr_info("pd timer number = %d\n", PD_TIMER_NR);
	mutex_init(&tmr->lock);
	sema_init(&tmr->enable_mask_lock, 1);
	spin_lock_init(&tmr->tick_lock);
	tmr->tick = 0;
	tmr->enable_mask = 0;

	tmr->task = kthread_create(timer_thread, tmr, "timer_%s.%p",
		dev_name(&tcpc->dev), tmr);
	init_waitqueue_head(&tmr->wait_que);
	wake_up_process(tmr->task);

	for (i = 0; i < PD_TIMER_NR; i++) {
		hrtimer_init(&tmr->list[i], CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		tmr->list[i].function = g_timers[i].call;
	}
	return 0;
}

void timer_deinit(struct tcpc_device *tcpc)
{
	uint64_t mask;
	int i;
	struct timer_device *tmr = NULL;

	if (!tcpc || !tcpc->timer)
		return;

	tmr = tcpc->timer;
	mask = timer_get_enable_mask(tmr);

	mutex_lock(&tmr->lock);
	tmr->thread_stop = true;
	wake_up_interruptible(&tmr->wait_que);
	kthread_stop(tmr->task);
	for (i = 0; i < PD_TIMER_NR; i++) {
		if (mask & rt_mask64(i))
			hrtimer_try_to_cancel(&tmr->list[i]);
	}

	pr_info("%s : de init ok\n", __func__);
	mutex_unlock(&tmr->lock);

	mutex_destroy(&tmr->lock);
	kfree(tmr);
	tcpc->timer = NULL;
}
