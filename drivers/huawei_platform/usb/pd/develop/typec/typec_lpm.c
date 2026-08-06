/*
 * typec_lpm.c
 *
 * typec low power mode
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "typec_lpm.h"
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/pm_wakeup.h>
#include <linux/alarmtimer.h>
#include <linux/workqueue.h>
#include "inc/tcpci.h"
#include "inc/tcpm.h"
#include "inc/timer.h"
#include "inc/debug.h"

#define TCPC_LOW_POWER_MODE_RETRY         5

struct typec_lpm {
	struct tcpc_device *tcpc;
	bool typec_lpm;
	bool typec_cable_only;
	bool wake_lock_user;
	bool typec_watchdog;
#ifdef CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY
	bool typec_wakeup_once;
	bool typec_low_rp_duty_cntdown;
#endif /* CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY */
	uint8_t typec_lpm_pull;
	uint8_t typec_lpm_retry;
	uint8_t wake_lock_pd;
	struct mutex access_mutex;
	struct wakeup_source attach_wake_lock;
	struct wakeup_source dettach_wake_lock;

	struct alarm wake_up_timer;
	struct delayed_work wake_up_work;
	struct wakeup_source wakeup_wake_lock;
};

void typec_set_cable_only(struct typec_lpm *lpm, bool only)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	lpm->typec_cable_only = only;
	mutex_unlock(&lpm->access_mutex);
}

void typec_set_watchdog(struct typec_lpm *lpm, bool en)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	if (lpm->typec_watchdog == en) {
		mutex_unlock(&lpm->access_mutex);
		return;
	}

	lpm->typec_watchdog = en;
	tcpci_enable_watchdog(lpm->tcpc, en);
	if (!en || lpm->attach_wake_lock.active)
		tcpci_set_intrst(lpm->tcpc, en);
	mutex_unlock(&lpm->access_mutex);
}

void typec_attach_wake_lock(struct typec_lpm *lpm)
{
	if (!lpm)
		return;

	/* 6000ms */
	__pm_wakeup_event(&lpm->attach_wake_lock, 6000);
}

static int typec_set_wake_lock(struct typec_lpm *lpm, bool pd_lock,
	bool user_lock)
{
	bool ori_lock = false;
	bool new_lock = false;

	if (lpm->wake_lock_pd && lpm->wake_lock_user)
		ori_lock = true;
	else
		ori_lock = false;

	if (pd_lock && user_lock)
		new_lock = true;
	else
		new_lock = false;

	if (new_lock == ori_lock)
		return 0;

	if (new_lock) {
		TYPEC_DBG("wake_lock=1\n");
		typec_attach_wake_lock(lpm);
		if (lpm->typec_watchdog)
			tcpci_set_intrst(lpm->tcpc, true);
	} else {
		TYPEC_DBG("wake_lock=0\n");
		if (lpm->typec_watchdog)
			tcpci_set_intrst(lpm->tcpc, false);
		__pm_relax(&lpm->attach_wake_lock);
	}
	return 1;
}

void typec_set_wake_lock_pd(struct typec_lpm *lpm, bool pd_lock)
{
	uint8_t wake_lock_pd;

	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	wake_lock_pd = lpm->wake_lock_pd;

	if (pd_lock)
		wake_lock_pd++;
	else if (wake_lock_pd > 0)
		wake_lock_pd--;

	if (wake_lock_pd == 0)
		/* 5000ms */
		__pm_wakeup_event(&lpm->dettach_wake_lock, 5000);

	typec_set_wake_lock(lpm, wake_lock_pd, lpm->wake_lock_user);
	if (wake_lock_pd == 1)
		__pm_relax(&lpm->dettach_wake_lock);

	lpm->wake_lock_pd = wake_lock_pd;
	mutex_unlock(&lpm->access_mutex);
}

void typec_enable_lpm_timer(struct typec_lpm *lpm)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	if (lpm->typec_lpm && !lpm->typec_cable_only)
		timer_enable(lpm->tcpc, TYPEC_RT_TIMER_LOW_POWER_MODE);
	mutex_unlock(&lpm->access_mutex);
}

static inline void typec_disable_lpm_timer(struct typec_lpm *lpm)
{
	timer_disable(lpm->tcpc, TYPEC_RT_TIMER_LOW_POWER_MODE);
}

static void typec_try_to_lpm(struct typec_lpm *lpm)
{
	int ret;

	ret = tcpci_set_low_power_mode(lpm->tcpc, true, lpm->typec_lpm_pull);
	if (ret < 0)
		return;

	ret = tcpci_is_low_power_mode(lpm->tcpc);
	if ((ret < 0) || (ret == 1)) {
		TYPEC_INFO("%s: lpm=%d\n", __func__, ret);
		return;
	}

	if (lpm->typec_lpm_retry == 0) {
		TYPEC_INFO("%s: try lpm fail\n", __func__);
		return;
	}

	lpm->typec_lpm_retry--;
	TYPEC_DBG("%s: retry=%d\n", __func__, lpm->typec_lpm_retry);
	timer_enable(lpm->tcpc, TYPEC_RT_TIMER_LOW_POWER_MODE);
}

void typec_handle_lpm_timeout(struct typec_lpm *lpm)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	if (lpm->typec_lpm)
		typec_try_to_lpm(lpm);
	mutex_unlock(&lpm->access_mutex);
}

static void typec_enable_wakeup_timer_nolock(struct typec_lpm *lpm, bool en)
{
	int tout = 300; /* 300s */

	TYPEC_INFO("%s: en=%d,once=%d,cntdown=%d\n", __func__, en,
		lpm->typec_wakeup_once, lpm->typec_low_rp_duty_cntdown);

	if (en) {
#ifdef CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY
		if (!lpm->typec_wakeup_once) {
			if (lpm->typec_low_rp_duty_cntdown)
				tout = 5; /* 5s */
			else
				tout = 20; /* 20s */
		}
#endif  /* CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY */

		alarm_start_relative(&lpm->wake_up_timer, ktime_set(tout, 0));
	} else {
		alarm_cancel(&lpm->wake_up_timer);
	}
}

void typec_enable_lpm(struct typec_lpm *lpm, uint8_t pull)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	if (lpm->typec_cable_only) {
		TYPEC_DBG("%s: lpm ra only\n", __func__);
		if (tcpci_is_support(lpm->tcpc, TCPC_FLAGS_LPM_WAKEUP_WATCHDOG))
			typec_enable_wakeup_timer_nolock(lpm, true);

		mutex_unlock(&lpm->access_mutex);
		return;
	}

	if (!lpm->typec_lpm) {
		TYPEC_INFO("%s: enable lpm pull=%u\n", __func__, pull);
		lpm->typec_lpm = true;
		lpm->typec_lpm_retry = TCPC_LOW_POWER_MODE_RETRY;
		lpm->typec_lpm_pull = (uint8_t)pull;
		timer_enable(lpm->tcpc, TYPEC_RT_TIMER_LOW_POWER_MODE);
	}
	mutex_unlock(&lpm->access_mutex);
}

void typec_disable_lpm(struct typec_lpm *lpm)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	if (lpm->typec_lpm) {
		TYPEC_INFO("%s: disable lpm now\n", __func__);
		lpm->typec_lpm = false;
		typec_disable_lpm_timer(lpm);
		tcpci_set_low_rp_duty(lpm->tcpc, false);
		tcpci_set_low_power_mode(lpm->tcpc, false, TYPEC_CC_DRP);
	}

#ifdef CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY
	lpm->typec_wakeup_once = 0;
	lpm->typec_low_rp_duty_cntdown = 0;
#endif /* CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY */
	mutex_unlock(&lpm->access_mutex);
}

/*
 * Check the CC status
 * Rd (device) -> let cc_handler to handle it later
 * eMark Only -> Reschedule wakeup timer
 *
 * Open -> (true condition)
 * Ready to reenter low-power mode.
 * If we repeatedly enter this situation,
 * it will trigger low rp duty protection.
 */
static bool typec_check_reenter_lpm(struct typec_lpm *lpm)
{
	uint8_t cc1 = TYPEC_ROLE_UNKNOWN;
	uint8_t cc2 = TYPEC_ROLE_UNKNOWN;

	tcpci_read_cc(lpm->tcpc, &cc1, &cc2);
	TYPEC_DBG("%s++%d,%d\n", __func__, cc1, cc2);
	if ((cc1 == TYPEC_CC_VOLT_OPEN) && (cc2 == TYPEC_CC_VOLT_OPEN)) {
		lpm->typec_cable_only = false;
	} else if ((cc1 + cc2) == TYPEC_CC_VOLT_RA) {
		lpm->typec_cable_only = true;
		TYPEC_DBG("%s: typec cable only\n", __func__);
	} else {
		lpm->typec_cable_only = false;
		TYPEC_DBG("%s: cc not open\n",  __func__, cc1, cc2);
		return false;
	}

	return true;
}

/*
 * Check the Single Ra resistance (eMark) exists or not when
 *    1) Ra_detach INT triggered.
 *    2) Wakeup_Timer triggered.
 *
 * If reentering low-power mode and eMark still exists,
 * it may cause an infinite loop.
 * If the CC status is both open, return true; otherwise return false
 */
static bool typec_check_false_ra_detach(struct typec_lpm *lpm)
{
	uint8_t cc1 = TYPEC_ROLE_UNKNOWN;
	uint8_t cc2 = TYPEC_ROLE_UNKNOWN;
	bool drp = tcpci_get_cc_mode() >= TYPEC_ROLE_DRP;

	/*
	 * If the DUT is DRP and current CC status has stopped toggle,
	 * let cc_handler to handle it later. (after debounce)
	 * If CC is toggling, force CC to present Rp.
	 */
	if (drp) {
		tcpci_read_cc(lpm->tcpc, &cc1, &cc2);
		TYPEC_DBG("%s+%d,%d\n", __func__, cc1, cc2);
		if (cc1 != TYPEC_CC_DRP_TOGGLING)
			return true;

		tcpci_set_cc(lpm->tcpc, TYPEC_CC_RP);
		/* 1ms */
		usleep_range(1000, 2000);
	}

	if (!typec_check_reenter_lpm(lpm))
		return true;

	if (lpm->typec_cable_only && tcpci_is_support(lpm->tcpc,
		TCPC_FLAGS_LPM_WAKEUP_WATCHDOG))
		typec_enable_wakeup_timer_nolock(lpm, true);

#ifdef CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY
	TYPEC_INFO("%s: cable=%d,once=%d,cntdown=%d\n", __func__,
		lpm->typec_cable_only, lpm->typec_wakeup_once,
		lpm->typec_low_rp_duty_cntdown);
	if (!lpm->typec_cable_only) {
		if (lpm->typec_low_rp_duty_cntdown) {
			tcpci_set_low_rp_duty(lpm->tcpc, true);
		} else {
			lpm->typec_wakeup_once = false;
			lpm->typec_low_rp_duty_cntdown = true;
		}
	}
#endif /* CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY */

	/*
	 * If the DUT is DRP, force CC to toggle again.
	 */
	if (drp) {
		tcpci_set_cc(lpm->tcpc, TYPEC_CC_DRP);
		tcpci_alert_status_clear(lpm->tcpc,
			TCPC_REG_ALERT_EXT_RA_DETACH);
	}
	return lpm->typec_cable_only;
}

void typec_enter_lpm_again(struct typec_lpm *lpm)
{
	bool check_ra = false;

	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	check_ra = lpm->typec_lpm || lpm->typec_cable_only;
	if (check_ra && typec_check_false_ra_detach(lpm)) {
		mutex_unlock(&lpm->access_mutex);
		return;
	}

	TYPEC_INFO("%s+\n", __func__);

	lpm->typec_lpm = true;
	tcpci_set_low_power_mode(lpm->tcpc, true,
		(tcpci_get_cc_mode() == TYPEC_ROLE_SRC) ?
		TYPEC_CC_DRP : TYPEC_CC_RP);
	mutex_unlock(&lpm->access_mutex);
}

static void typec_wake_up_work_func(struct work_struct *work)
{
	struct typec_lpm *lpm = container_of(work, struct typec_lpm,
		wake_up_work.work);

	TYPEC_INFO("%s+\n", __func__);
#ifdef CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY
	mutex_lock(&lpm->access_mutex);
	lpm->typec_wakeup_once = true;
	mutex_unlock(&lpm->access_mutex);
#endif /* CONFIG_TYPEC_WAKEUP_ONCE_LOW_DUTY */

	typec_enter_lpm_again(lpm);

	__pm_relax(&lpm->wakeup_wake_lock);
}

static enum alarmtimer_restart typec_alarm_wakeup(struct alarm *alarm,
	ktime_t now)
{
	struct typec_lpm *lpm = container_of(alarm, struct typec_lpm,
		wake_up_timer);

	__pm_wakeup_event(&lpm->wakeup_wake_lock, 1000);
	schedule_delayed_work(&lpm->wake_up_work, 0);
	return ALARMTIMER_NORESTART;
}

void typec_enable_wakeup_timer(struct typec_lpm *lpm, bool en)
{
	if (!lpm)
		return;

	mutex_lock(&lpm->access_mutex);
	typec_enable_wakeup_timer_nolock(lpm, en);
	mutex_unlock(&lpm->access_mutex);
}

void typec_init_lpm(struct typec_lpm **lpm, struct tcpc_device *tcpc)
{
	struct typec_lpm *tlpm = NULL;

	if (!lpm || !tcpc)
		return;

	tlpm = kzalloc(sizeof(*tlpm), GFP_KERNEL);
	if (!tlpm)
		return;

	*lpm = tlpm;
	tlpm->tcpc = tcpc;
	mutex_init(&tlpm->access_mutex);
	wakeup_source_init(&tlpm->attach_wake_lock, "tcpc_attach_wakelock");
	wakeup_source_init(&tlpm->dettach_wake_lock, "tcpc_detach_wakelock");

	wakeup_source_init(&tlpm->wakeup_wake_lock, "wakeup_wake_lock");
	INIT_DELAYED_WORK(&tlpm->wake_up_work, typec_wake_up_work_func);
	alarm_init(&tlpm->wake_up_timer, ALARM_REALTIME, typec_alarm_wakeup);

	tlpm->wake_lock_pd = 0;
	tlpm->wake_lock_user = true;
}

void typec_deinit_lpm(struct typec_lpm **lpm)
{
	struct typec_lpm *tlpm = NULL;

	if (!lpm || !(*lpm))
		return;
	tlpm = *lpm;

	alarm_cancel(&tlpm->wake_up_timer);
	cancel_delayed_work(&tlpm->wake_up_work);
	wakeup_source_trash(&tlpm->dettach_wake_lock);
	wakeup_source_trash(&tlpm->attach_wake_lock);
	wakeup_source_trash(&tlpm->wakeup_wake_lock);
	mutex_destroy(&tlpm->access_mutex);

	kfree(*lpm);
	*lpm = NULL;
}
