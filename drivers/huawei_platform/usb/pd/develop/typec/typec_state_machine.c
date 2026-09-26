/*
 * typec_state_machine.c
 *
 * typec state machine
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

#include "typec_state_machine.h"
#include <linux/slab.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include "inc/tcpc_core.h"
#include "inc/typec_vbus.h"
#include "typec_state.h"
#include "typec_legacy_cable.h"
#include "typec_common.h"
#include "typec_lpm.h"

static void typecs_print_status(struct tcpc_device *tcpc, const char *tag)
{
	struct typec_state_object *sobj = &tcpc->fsm->objs[tcpc->fsm->cur_state];

	if (!tag)
		return;

	TYPEC_INFO("%s-%s: role=%d,local_cc=%d,cc=%d/%d,pe_run=%d,vbus=%d\n",
		sobj->name, tag, typec_get_role(), typec_get_local_cc(),
		typec_get_cc1(), typec_get_cc2(), pd_is_pe_running(tcpc),
		typec_inquire_vbus_level(tcpc, false));
}

static bool typecs_is_ignore_cc_change(struct tcpc_device *tcpc)
{
	if (typec_legacy_cc_change(tcpc))
		return true;

	if (tcpci_is_pr_swaping(tcpc)) {
		TYPEC_DBG("%s: PR.Swap Ignore CC_Alert\n", __func__);
		return true;
	}

	return false;
}

static int typecs_transfer_next_state(struct tcpc_device *tcpc)
{
	int ret;
	struct typec_state_object *sobj = NULL;

	if (tcpc->fsm->cur_state == tcpc->fsm->next_state)
		return 0;

	TYPEC_INFO("typec state %s->%s\n",
		tcpc->fsm->objs[tcpc->fsm->cur_state].name,
		tcpc->fsm->objs[tcpc->fsm->next_state].name);

	typecs_wait_ps_change(tcpc, TYPECS_WAIT_PS_DISABLE);

	tcpc->fsm->cur_state = tcpc->fsm->next_state;
	sobj = &tcpc->fsm->objs[tcpc->fsm->cur_state];
	if (!sobj->entry)
		return 0;

	ret = sobj->entry(tcpc);
	if (ret)
		TYPEC_ERR("%s: entry %s fail ret = %d\n", __func__,
			sobj->name, ret);

	return ret;
}

static int typecs_publish_cc_event(struct tcpc_device *tcpc)
{
	int ret;
	struct typec_state_object *sobj = NULL;

	sobj = &tcpc->fsm->objs[tcpc->fsm->cur_state];
	typecs_print_status(tcpc, "cc event");
	if (!sobj->cc_change)
		return 0;

	ret = sobj->cc_change(tcpc);
	if (ret) {
		TYPEC_ERR("%s: publish cc event fail ret=%d\n", __func__, ret);
		return ret;
	}

	return typecs_transfer_next_state(tcpc);
}

int typecs_handle_cc_change(struct tcpc_device *tcpc)
{
	int ret;

	if (!tcpc || !tcpc->fsm)
		return -EINVAL;

	ret = tcpci_get_cc(tcpc);
	if (ret < 0)
		return ret;

	if (typec_is_drp_toggling()) {
		TYPEC_DBG("%s: Warning DRP Toggling\n", __func__);
		typec_enable_lpm_timer(tcpc->lpm);
		return 0;
	}

	TYPEC_INFO("[CC_Alert] %d/%d\n", typec_get_cc1(), typec_get_cc2());
	pd_dpm_cc_dynamic_protect();
	typec_disable_lpm(tcpc->lpm);

	if (typecs_is_ignore_cc_change(tcpc))
		return 0;

	typec_set_cable_only(tcpc->lpm, false);
	return typecs_publish_cc_event(tcpc);
}

static int typecs_publish_ps_event(struct tcpc_device *tcpc)
{
	uint8_t last_state;
	int ret;
	struct typec_state_object *sobj = NULL;

	last_state = tcpc->fsm->cur_state;
	sobj = &tcpc->fsm->objs[tcpc->fsm->cur_state];
	typecs_print_status(tcpc, "ps event");
	if (!sobj->ps_change)
		return 0;

	ret = sobj->ps_change(tcpc);
	if (ret) {
		TYPEC_ERR("%s: publish ps event fail ret=%d\n", __func__, ret);
		return ret;
	}

	ret = typecs_transfer_next_state(tcpc);
	if (last_state != tcpc->fsm->cur_state)
		ret += typecs_handle_cc_change(tcpc);

	return ret;
}

int typecs_handle_ps_change(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return -EINVAL;

	if (typec_legacy_ps_change(tcpc))
		return 0;

	return typecs_publish_ps_event(tcpc);
}

static int typecs_publish_time_event(struct tcpc_device *tcpc, uint32_t timer_id)
{
	uint8_t last_state;
	int ret;
	struct typec_state_object *sobj = NULL;

	last_state = tcpc->fsm->cur_state;
	sobj = &tcpc->fsm->objs[tcpc->fsm->cur_state];
	typecs_print_status(tcpc, "time event");
	if (!sobj->timeout)
		return 0;

	ret = sobj->timeout(tcpc, timer_id);
	if (ret) {
		TYPEC_ERR("%s: publish time event fail ret=%d\n", __func__, ret);
		return ret;
	}

	ret = typecs_transfer_next_state(tcpc);
	if (last_state != tcpc->fsm->cur_state)
		ret += typecs_handle_cc_change(tcpc);

	return ret;
}

int typecs_handle_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	int ret;

	if (!tcpc || !tcpc->fsm)
		return -EINVAL;

	typec_lock(tcpc);
	if (tcpci_is_pr_swaping(tcpc)) {
		TYPEC_DBG("PR.Swap Ignore timer_evt\n");
		goto unlock_mutex;
	}

	TYPEC_DBG("%s: handle time event id=%u\n", __func__, timer_id);
	switch (timer_id) {
	case TYPEC_RT_TIMER_LOW_POWER_MODE:
		typec_handle_lpm_timeout(tcpc->lpm);
		break;
	case TYPEC_RT_TIMER_AUTO_DISCHARGE:
		typec_handle_auto_discharge_timeout(tcpc);
		break;
	case TYPEC_RT_TIMER_NOT_LEGACY:
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	case TYPEC_RT_TIMER_LEGACY_STABLE:
#ifdef CONFIG_TYPEC_LEGACY2_AUTO_RECYCLE
	case TYPEC_RT_TIMER_LEGACY_RECYCLE:
#endif /* CONFIG_TYPEC_LEGACY2_AUTO_RECYCLE */
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */
		typec_legacy_timeout(tcpc, timer_id);
		break;
#ifdef CONFIG_COMPATIBLE_APPLE_TA
	case TYPEC_TIMER_APPLE_CC_OPEN:
#endif /* CONFIG_COMPATIBLE_APPLE_TA */
	case TYPEC_TIMER_CCDEBOUNCE:
	case TYPEC_TIMER_PDDEBOUNCE:
	case TYPEC_TIMER_TRYCCDEBOUNCE:
	case TYPEC_TIMER_SRCDISCONNECT:
	case TYPEC_TIMER_NORP_SRC:
		pd_dpm_handle_pe_event(PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER, NULL);
	/* fall through */
	default:
		ret = typecs_publish_time_event(tcpc, timer_id);
		break;
	}

unlock_mutex:
	typec_unlock(tcpc);
	return ret;
}

void typecs_notfiy_pe_idle(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm || !tcpc->fsm->wait_pe_idle)
		return;

	TYPEC_INFO("%s: notify pe idle\n", __func__);
	timer_enable(tcpc, TYPEC_RT_TIMER_PE_IDLE);
}

bool typecs_is_wait_pe_idle(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return false;

	return tcpc->fsm->wait_pe_idle;
}

void typecs_set_sink_curr(struct tcpc_device *tcpc, int curr)
{
	if (!tcpc || !tcpc->fsm)
		return;

	if (tcpc->fsm->sink_curr != curr)
		TYPEC_INFO("%s: set sink curr %d->%d\n", __func__,
			tcpc->fsm->sink_curr, curr);

	tcpc->fsm->sink_curr = curr;
}

void typecs_typec_error_recovery(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return;

	pr_info("%s\n", __func__);
	typecs_next_state(tcpc, typec_errorrecovery);
	typecs_transfer_next_state(tcpc);
}

void typecs_typec_disable(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return;

	if (tcpc->fsm->cur_state == typec_disabled)
		return;

	TYPEC_INFO("%s\n", __func__);
	typecs_next_state(tcpc, typec_disabled);
	typecs_transfer_next_state(tcpc);
}

void typecs_typec_enable(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return;

	if (tcpc->fsm->cur_state != typec_disabled)
		return;

	TYPEC_INFO("%s\n", __func__);
	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
	typecs_transfer_next_state(tcpc);
}

void typecs_pe_disable(struct tcpc_device *tcpc)
{
	uint8_t attach_type;

	if (!tcpc || !tcpc->fsm)
		return;

	if (tcpc->fsm->disable_pe)
		return;

	attach_type = typecs_get_attach_type(tcpc);
	TYPEC_INFO("%s: attach_type=%d\n", __func__, attach_type);
	tcpc->fsm->disable_pe = true;
	if (attach_type != TYPEC_UNATTACHED)
		pd_notify_usb_port_detach(tcpc);
}

void typecs_pe_enable(struct tcpc_device *tcpc)
{
	uint8_t attach_type;

	if (!tcpc || !tcpc->fsm)
		return;

	if (!tcpc->fsm->disable_pe)
		return;

	attach_type = typecs_get_attach_type(tcpc);
	TYPEC_INFO("%s: attach_type=%d\n", __func__, attach_type);
	tcpc->fsm->disable_pe = false;
	if (attach_type != TYPEC_UNATTACHED)
		pd_notify_usb_port_attach(tcpc, attach_type);
}

void typecs_handle_pe_pr_swap(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return;

	switch (tcpc->fsm->cur_state) {
	case typec_attached_snk:
		typecs_next_state(tcpc, typec_attached_src);
		break;
	case typec_attached_src:
		typecs_next_state(tcpc, typec_attached_snk);
		break;
	default:
		TYPEC_ERR("%s: not attach state=%d\n", __func__,
			tcpc->fsm->cur_state);
		return;
	}

	TYPEC_INFO("%s\n", __func__);
	typecs_transfer_next_state(tcpc);
}

void typecs_change_typec_role(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return;

	TYPEC_INFO("%s: force unattach\n", __func__);

	typecs_next_state(tcpc, typecs_get_unattached_state(tcpc));
	typecs_transfer_next_state(tcpc);
	timer_enable(tcpc, TYPEC_TIMER_PDDEBOUNCE);
}

void typecs_swap_typec_role(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return;

	TYPEC_INFO("%s: swap role cur state=%d\n", __func__,
		tcpc->fsm->cur_state);

	if (tcpc->fsm->cur_state == typec_attached_snk)
		typecs_next_state(tcpc, typec_role_swap_snk);
	else if (tcpc->fsm->cur_state == typec_attached_src)
		typecs_next_state(tcpc, typec_role_swap_src);
	else
		return;

	typecs_transfer_next_state(tcpc);
}

uint8_t typecs_get_typec_state(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return typec_disabled;

	return tcpc->fsm->cur_state;
}

uint8_t typecs_get_attach_type(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->fsm)
		return false;

	return tcpc->fsm->attach_type;
}

int typecs_fsm_init(struct tcpc_device *tcpc)
{
	int ret;

	if (!tcpc)
		return -EINVAL;

	tcpc->fsm = kzalloc(sizeof(*(tcpc->fsm)), GFP_KERNEL);
	if (!tcpc->fsm)
		return -ENOMEM;

	ret = typecs_init(tcpc);
	if (ret) {
		TYPEC_ERR("%s: init typec state fail\n", __func__);
		kfree(tcpc->fsm);
		tcpc->fsm = NULL;
		return ret;
	}

	tcpc->typec_power_ctrl = true;
	ret = typecs_transfer_next_state(tcpc);
	TYPEC_INFO("%s: init typec state ret=%d\n", __func__, ret);
	return 0;
}

int typecs_fsm_deinit(struct tcpc_device *tcpc)
{
	int ret;

	if (!tcpc || !tcpc->fsm)
		return -EINVAL;

	ret = typecs_deinit(tcpc);
	TYPEC_INFO("%s: deinit typec state ret=%d\n", __func__, ret);
	kfree(tcpc->fsm);
	tcpc->fsm = NULL;

	return ret;
}
