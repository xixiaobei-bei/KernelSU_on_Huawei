/*
 * tcpm_notifier.c
 *
 * tcpm notifier
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

#include "inc/tcpm_notifier.h"
#include <huawei_platform/usb/hw_pd_dev.h>
#include "inc/tcpc_core.h"

static int tcpm_notifier_func_stub(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct tcpm_notifier_block_wrapper *wpr =
		container_of(nb, struct tcpm_notifier_block_wrapper, stub_nb);
	struct notifier_block *action_nb = wpr->action_nb;

	return wpr->action_nb->notifier_call(action_nb, action, data);
}

static int tcpm_notifier_add_node(struct tcpm_notifier *notifier,
	struct tcpm_notifier_block_wrapper *wpr,
	struct notifier_block *key, int idx)
{
	struct tcpm_notifier_node *tail = NULL;
	struct tcpm_notifier_node *node  = NULL;

	node = devm_kzalloc(notifier->dev, sizeof(*node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;

	node->wpr = wpr;
	node->nb_key = key;
	node->idx = idx;

	mutex_lock(&notifier->nl_lock);
	tail = notifier->nl_head;
	if (tail) {
		while (tail->next)
			tail = tail->next;
		tail->next = node;
	} else {
		notifier->nl_head = node;
	}
	mutex_unlock(&notifier->nl_lock);

	return 0;
}

static void *tcpm_notifier_remove_node(struct tcpm_notifier *notifier,
	struct notifier_block *key, int idx)
{
	struct tcpm_notifier_block_wrapper *wpr = NULL;
	struct tcpm_notifier_node *prev = NULL;
	struct tcpm_notifier_node *node = notifier->nl_head;

	mutex_lock(&notifier->nl_lock);
	if (!node)
		goto normal_exit;

	while (node) {
		if ((node->nb_key == key) && (node->idx == idx)) {
			wpr = node->wpr;
			if (prev)
				prev->next = node->next;
			else
				notifier->nl_head = NULL;
			devm_kfree(notifier->dev, node);
			break;
		}
		prev = node;
		node = node->next;
	}

normal_exit:
	mutex_unlock(&notifier->nl_lock);
	return wpr;
}

static int tcpm_register_wrap_notifier(struct tcpm_notifier *notifier,
	struct notifier_block *nb, uint8_t idx)
{
	int ret;
	struct tcpm_notifier_block_wrapper *wpr = NULL;

	wpr = devm_kzalloc(notifier->dev, sizeof(*wpr), GFP_KERNEL);
	if (!wpr)
		return -ENOMEM;

	wpr->action_nb = nb;
	wpr->stub_nb.notifier_call = tcpm_notifier_func_stub;

	ret = tcpm_notifier_add_node(notifier, wpr, nb, idx);
	if (ret < 0) {
		pr_err("%s: add node fail ret=%d\n", __func__, ret);
		devm_kfree(notifier->dev, wpr);
		return ret;
	}

	ret = srcu_notifier_chain_register(notifier->evt_nh + idx,
		&wpr->stub_nb);
	if (ret < 0)
		devm_kfree(notifier->dev, wpr);
	return ret;
}

static bool tcpm_is_mulit_bits_set(uint32_t flags)
{
	if (flags) {
		flags &= (flags - 1);
		return flags ? true : false;
	}

	return false;
}

int tcpm_register_notifier(struct tcpm_notifier *notifier,
	struct notifier_block *nb, uint8_t flags)
{
	int i;
	int ret;

	if (!notifier || !notifier->dev || !nb)
		return -EINVAL;

	if (!tcpm_is_mulit_bits_set(flags)) {
		for (i = 0; i < TCP_NOTIFY_IDX_NR; i++) {
			if ((flags & (1 << i)) == 0)
				continue;
			return srcu_notifier_chain_register(
				&notifier->evt_nh[i], nb);
		}
		return -EFAULT;
	}

	for (i = 0; i < TCP_NOTIFY_IDX_NR; i++) {
		if ((flags & (1 << i)) == 0)
			continue;
		ret = tcpm_register_wrap_notifier(notifier, nb, i);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int tcpm_unregister_wrap_notifier(struct tcpm_notifier *notifier,
	struct notifier_block *nb, uint8_t idx)
{
	int ret;
	struct tcpm_notifier_block_wrapper *wpr = NULL;

	wpr = tcpm_notifier_remove_node(notifier, nb, idx);
	if (!wpr)
		return -ENOENT;

	ret = srcu_notifier_chain_unregister(notifier->evt_nh + idx,
		&wpr->stub_nb);
	devm_kfree(notifier->dev, wpr);
	return ret;
}

void tcpm_unregister_dev_notifier(struct tcpm_notifier *notifier,
	struct notifier_block *nb, uint8_t flags)
{
	int i;
	int ret;

	if (!notifier || !notifier->dev || !nb)
		return;

	for (i = 0; i < TCP_NOTIFY_IDX_NR; i++) {
		if ((flags & (1 << i)) == 0)
			continue;

		ret = tcpm_unregister_wrap_notifier(notifier, nb, i);
		if (ret == -ENOENT)
			ret = srcu_notifier_chain_unregister(
				notifier->evt_nh + i, nb);
		if (ret < 0)
			pr_err("%s: unregister fail idx=%d,ret=%d\n",
				__func__, i, ret);
	}
}

static void tcpm_notify(struct tcpm_notifier *notifier,
	struct tcp_notify *ny, uint8_t type, uint8_t state)
{
	int ret;

	if (!notifier || !notifier->dev)
		return;

	ret = srcu_notifier_call_chain(&notifier->evt_nh[type], state, ny);
	pr_info("%s: notify type=%d,state=%d,ret=%d\n", __func__,
		type, state, ret);
}

void tcpm_notify_vconn(struct tcpm_notifier *notifier, bool en)
{
	struct tcp_notify ny;

	ny.en_state.en = en;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_VBUS,
		TCP_NOTIFY_SOURCE_VCONN);
}

void tcpm_notify_wd_status(struct tcpm_notifier *notifier, bool detected)
{
	struct tcp_notify ny;

	ny.wd_status.water_detected = detected;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC, TCP_NOTIFY_WD_STATUS);
}

void tcpm_notify_cable_type(struct tcpm_notifier *notifier,
	enum tcpc_cable_type type)
{
	struct tcp_notify ny;

	ny.cable_type.type = type;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC, TCP_NOTIFY_CABLE_TYPE);
}

void tcpm_notify_vbus_cc_short(struct tcpm_notifier *notifier,
	bool cc1_short, bool cc2_short)
{
	struct tcp_notify ny;

	ny.vbus_cc_short.cc1 = cc1_short;
	ny.vbus_cc_short.cc2 = cc2_short;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC,
		TCP_NOTIFY_VBUS_CC_SHORT);
}

void tcpm_notify_typec_state(struct tcpm_notifier *notifier, uint8_t polarity,
	uint8_t old_state, uint8_t new_state, uint8_t rp)
{
	struct tcp_notify ny;
	int ret;

	ny.typec_state.polarity = polarity;
	ny.typec_state.old_state = old_state;
	ny.typec_state.new_state = new_state;
	ny.typec_state.rp_level = rp;

	ret = pd_dpm_handle_pe_event(PD_DPM_PE_EVT_TYPEC_STATE,
		&ny.typec_state);
	if (ret < 0)
		pr_err("%s: handle pe event fail\n", __func__);

	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_USB, TCP_NOTIFY_TYPEC_STATE);
}

void tcpm_notify_role_swap(struct tcpm_notifier *notifier, uint8_t event,
	uint8_t role)
{
	struct tcp_notify ny;
	struct pd_dpm_swap_state swap_state;

	swap_state.new_role = role;
	pd_dpm_handle_pe_event(event, &swap_state);

	ny.swap_state.new_role = role;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC, event);
}

void tcpm_notify_pd_state(struct tcpm_notifier *notifier, uint8_t connect)
{
	struct tcp_notify ny;
	struct pd_dpm_pd_state pd_state;

	pd_state.connected = connect;
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_PD_STATE, &pd_state);

	ny.pd_state.connected = connect;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_USB, TCP_NOTIFY_PD_STATE);
}

static void tcpm_notify_vbus_state(struct tcpm_notifier *notifier, bool sink,
	uint8_t type, int mv, int ma)
{
	struct tcp_notify ny;
	struct pd_dpm_vbus_state vbus_state;

	vbus_state.ma = ma;
	vbus_state.mv = mv;
	vbus_state.vbus_type = type;
	ny.vbus_state.ma = ma;
	ny.vbus_state.mv = mv;
	ny.vbus_state.type = type;

	if (sink) {
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SINK_VBUS, &vbus_state);
		tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_VBUS,
			TCP_NOTIFY_SINK_VBUS);
	} else {
		pd_dpm_handle_pe_event(PD_DPM_PE_EVT_SOURCE_VBUS, &vbus_state);
		tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_VBUS,
			TCP_NOTIFY_SOURCE_VBUS);
	}
}

void tcpm_notify_source_vbus_state(struct tcpm_notifier *notifier, uint8_t type,
	int mv, int ma)
{
	tcpm_notify_vbus_state(notifier, false, type, mv, ma);
}

void tcpm_notify_sink_vbus_state(struct tcpm_notifier *notifier, uint8_t type,
	int mv, int ma)
{
	tcpm_notify_vbus_state(notifier, true, type, mv, ma);
}

static void tcpm_notify_vbus_ctrl_state(struct tcpm_notifier *notifier,
	uint8_t state)
{
	struct tcp_notify ny;

	ny.vbus_state.ma = 0;
	ny.vbus_state.mv = 0;
	ny.vbus_state.type = 0;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_VBUS, state);
}

void tcpm_notify_diable_vbus_ctrl(struct tcpm_notifier *notifier)
{
	pd_dpm_handle_pe_event(PD_DPM_PE_EVT_DIS_VBUS_CTRL, NULL);
	tcpm_notify_vbus_ctrl_state(notifier, TCP_NOTIFY_DIS_VBUS_CTRL);
}

void tcpm_notify_source_vbus_wait(struct tcpm_notifier *notifier)
{
	tcpm_notify_vbus_ctrl_state(notifier, TCP_NOTIFY_ATTACHWAIT_SRC);
}

void tcpm_notify_sink_vbus_wait(struct tcpm_notifier *notifier)
{
	tcpm_notify_vbus_ctrl_state(notifier, TCP_NOTIFY_ATTACHWAIT_SNK);
}

void tcpm_notify_vbus_ext_discharge(struct tcpm_notifier *notifier, bool en)
{
	struct tcp_notify ny;

	ny.en_state.en = en;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_VBUS,
		TCP_NOTIFY_EXT_DISCHARGE);
}

void tcpm_notify_hard_reset_state(struct tcpm_notifier *notifier, uint8_t state)
{
	struct tcp_notify ny;

	ny.hreset_state.state = state;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC,
		TCP_NOTIFY_HARD_RESET_STATE);
}

void tcpm_notify_enter_mode(struct tcpm_notifier *notifier,
	uint16_t svid, uint8_t ops, uint32_t mode)
{
	struct tcp_notify ny;

	ny.mode_ctrl.svid = svid;
	ny.mode_ctrl.ops = ops;
	ny.mode_ctrl.mode = mode;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE, TCP_NOTIFY_ENTER_MODE);
}

void tcpm_notify_exit_mode(struct tcpm_notifier *notifier, uint16_t svid)
{
	struct tcp_notify ny;

	ny.mode_ctrl.svid = svid;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE, TCP_NOTIFY_EXIT_MODE);
}

void tcpm_notify_hpd_state(struct tcpm_notifier *notifier, uint8_t irq,
	uint8_t state)
{
	struct tcp_notify ny;

	ny.ama_dp_hpd_state.irq = irq;
	ny.ama_dp_hpd_state.state = state;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE,
		TCP_NOTIFY_AMA_DP_HPD_STATE);
}

void tcpm_notify_dp_state(struct tcpm_notifier *notifier, uint8_t polarity,
	uint32_t dp_state)
{
	struct tcp_notify ny;

	memset(&ny, 0, sizeof(ny));
	switch (dp_state & TCPM_DP_STATE_MODE_MASK) {
	case MODE_DP_USB:
		ny.ama_dp_state.sel_config = SW_USB;
		break;
	case MODE_DP_SNK:
		ny.ama_dp_state.sel_config = SW_DFP_D;
		ny.ama_dp_state.pin_assignment =
			(dp_state >> TCPM_DP_STATE_SNK_BIT) &
			TCPM_DP_STATE_PIN_MASK;
		break;
	case MODE_DP_SRC:
		ny.ama_dp_state.sel_config = SW_UFP_D;
		ny.ama_dp_state.pin_assignment =
			(dp_state >> TCPM_DP_STATE_SRC_BIT) &
			TCPM_DP_STATE_PIN_MASK;
		break;
	default:
		break;
	}

	if (ny.ama_dp_state.pin_assignment == 0)
		ny.ama_dp_state.pin_assignment =
			(dp_state >> TCPM_DP_STATE_SRC_BIT) &
			TCPM_DP_STATE_PIN_MASK;

	ny.ama_dp_state.signal = (dp_state >> TCPM_DP_STATE_SIGNAL_BIT) &
		TCPM_DP_STATE_SIGNAL_MASK;
	ny.ama_dp_state.polarity = polarity;
	ny.ama_dp_state.active = 1;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE,
		TCP_NOTIFY_AMA_DP_STATE);
}

void tcpm_notify_dp_attention(struct tcpm_notifier *notifier, uint8_t status)
{
	struct tcp_notify ny;

	ny.ama_dp_attention.state = status;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE,
		TCP_NOTIFY_AMA_DP_ATTENTION);
}

void tcpm_notify_dp_cfg_start(struct tcpm_notifier *notifier)
{
	struct tcp_notify ny;

	ny.ama_dp_state.sel_config = SW_USB;
	ny.ama_dp_state.active = 0;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE,
		TCP_NOTIFY_AMA_DP_STATE);
}

void tcpm_notify_uvdm(struct tcpm_notifier *notifier, bool ack, uint8_t cnt,
	uint16_t svid, uint32_t *data)
{
	struct tcp_notify ny;

	ny.uvdm_msg.ack = ack;
	if (ack) {
		ny.uvdm_msg.uvdm_cnt = cnt;
		ny.uvdm_msg.uvdm_svid = svid;
		ny.uvdm_msg.uvdm_data = data;
	} else {
		ny.uvdm_msg.uvdm_cnt = 0;
		ny.uvdm_msg.uvdm_svid = 0;
		ny.uvdm_msg.uvdm_data = NULL;
	}

	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE, TCP_NOTIFY_UVDM);
}

void tcpm_notify_dc_en_unlock(struct tcpm_notifier *notifier)
{
	struct tcp_notify ny;

	memset(&ny, 0, sizeof(ny));
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MODE,
		TCP_NOTIFY_DC_EN_UNLOCK);
}

void tcpm_notify_alert(struct tcpm_notifier *notifier, uint32_t ado)
{
	struct tcp_notify ny;

	ny.alert_msg.ado = ado;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC, TCP_NOTIFY_ALERT);
}

void tcpm_notify_status(struct tcpm_notifier *notifier, struct pd_status *sdb)
{
	struct tcp_notify ny;

	ny.status_msg.sdb = sdb;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC,
		TCP_NOTIFY_STATUS);
}

void tcpm_notify_request_bat_info(struct tcpm_notifier *notifier,
	enum pd_battery_reference ref)
{
	struct tcp_notify ny;

	ny.request_bat.ref = ref;
	tcpm_notify(notifier, &ny, TCP_NOTIFY_IDX_MISC,
		TCP_NOTIFY_REQUEST_BAT_INFO);
}

void tcpm_notifier_init(struct tcpc_device *tcpc)
{
	int i;
	struct tcpm_notifier *notifier = NULL;

	if (!tcpc)
		return;

	notifier = &tcpc->notifier;
	notifier->dev = &tcpc->dev;
	notifier->nl_head = NULL;
	mutex_init(&notifier->nl_lock);

	for (i = 0; i < TCP_NOTIFY_IDX_NR; i++)
		srcu_init_notifier_head(&notifier->evt_nh[i]);
}