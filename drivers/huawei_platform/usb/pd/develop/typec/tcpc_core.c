/*
 * tcpc_core.c
 *
 * tcpc core driver
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include "inc/tcpc_core.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include "inc/timer.h"
#include "inc/tcpci.h"
#include "inc/tcpm_dual_role.h"
#include "inc/typec_vbus.h"

#ifdef CONFIG_HW_USB_POWER_DELIVERY
#include "inc/pd_dpm_prv.h"
#include "inc/tcpm.h"
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
#include "typec_lpm.h"
#include "typec_common.h"
#include "typec_state_machine.h"
#include "typec_legacy_cable.h"

#define TCPC_CORE_VERSION        "2.0.10"
#define TCPC_PROPERTY_NAME_LEN   128

#define to_tcpc_device(obj) container_of(obj, struct tcpc_device, dev)

static struct class *tcpc_class;
static struct device_type tcpc_dev_type;

static ssize_t tcpc_show_property(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t tcpc_store_property(struct device *dev,
	struct device_attribute *attr,  const char *buf, size_t count);

#define tcpc_device_attr(_name, _mode) \
{ \
	.attr = { .name = #_name, .mode = _mode }, \
	.show = tcpc_show_property, \
	.store = tcpc_store_property, \
}

static struct device_attribute tcpc_device_attributes[] = {
	tcpc_device_attr(role_def, 0444),
	tcpc_device_attr(rp_lvl, 0444),
	tcpc_device_attr(pd_test, 0664),
	tcpc_device_attr(info, 0444),
	tcpc_device_attr(timer, 0664),
	tcpc_device_attr(caps_info, 0444),
	tcpc_device_attr(pe_ready, 0444),
	tcpc_device_attr(typec_state, 0444),
	tcpc_device_attr(pd_state, 0444),
};

enum {
	TCPC_DESC_ROLE_DEF = 0,
	TCPC_DESC_RP_LEVEL,
	TCPC_DESC_PD_TEST,
	TCPC_DESC_INFO,
	TCPC_DESC_TIMER,
	TCPC_DESC_CAP_INFO,
	TCPC_DESC_PE_READY,
	TCPC_DESC_TYPEC_STATE,
	TCPC_DESC_PD_STATE,
};

static struct attribute *tcpc_sysfs_attrs[ARRAY_SIZE(tcpc_device_attributes) + 1];

static struct attribute_group tcpc_attr_group = {
	.attrs = tcpc_sysfs_attrs,
};

static const struct attribute_group *tcpc_attr_groups[] = {
	&tcpc_attr_group,
	NULL,
};

static void tcpc_show_rp_lvl(struct tcpc_device *tcpc, char *buf, int len)
{
	if (tcpc->typec_local_rp_level == TYPEC_CC_RP_DFT)
		snprintf(buf, len, "%s\n", "Default");
	else if (tcpc->typec_local_rp_level == TYPEC_CC_RP_1_5)
		snprintf(buf, len, "%s\n", "1.5");
	else if (tcpc->typec_local_rp_level == TYPEC_CC_RP_3_0)
		snprintf(buf, len, "%s\n", "3.0");
}

static const char * const role_text[] = {
	"SNK Only",
	"SRC Only",
	"DRP",
	"Try.SRC",
	"Try.SNK",
};

static void tcpc_show_desc_info(struct tcpc_device *tcpc, char *buf, int len)
{
	snprintf(buf, len, "|^|==( %s info )==|^|\n", tcpc->desc.name);
	snprintf(buf + strlen(buf), len - strlen(buf), "role = %s\n",
		role_text[tcpc->desc.role_def]);
	if (tcpc->typec_local_rp_level == TYPEC_CC_RP_DFT)
		snprintf(buf + strlen(buf), len - strlen(buf), "rplvl = %s\n", "Default");
	else if (tcpc->typec_local_rp_level == TYPEC_CC_RP_1_5)
		snprintf(buf + strlen(buf), len - strlen(buf), "rplvl = %s\n", "1.5");
	else if (tcpc->typec_local_rp_level == TYPEC_CC_RP_3_0)
		snprintf(buf + strlen(buf), len - strlen(buf), "rplvl = %s\n", "3.0");
}

static void tcpc_show_typec_state(struct tcpc_device *tcpc, char *buf, int len)
{
	snprintf(buf, len, "typec_state=%d\ntypec_attach_type=%d\n"
		"typec_local_cc=%d\ntypec_polarity=%d\n"
		"typec_remote_cc[0]=%d\ntypec_remote_cc[1]=%d\n",
		typecs_get_typec_state(tcpc), typecs_get_attach_type(tcpc),
		tcpc->typec_local_cc, tcpc->typec_polarity,
		tcpc->typec_remote_cc[0], tcpc->typec_remote_cc[1]);
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
static void tcpc_show_pd_state(struct pd_port *port, char *buf, int len)
{
	snprintf(buf, len, "pd_port state==>\n"
		"state_machine=%d\npd_connect_state=%d\n"
		"pe_pd_state=%d\npe_vdm_state=%d\n"
		"pe_state_next=%d\npe_state_curr=%d\n"
		"pe state==>\n"
		"pd_connected=%d\npd_prev_connected=%d\n"
		"explicit_contract=%d\npe_ready=%d\n",
		port->state_machine, port->pd_connect_state,
		port->pe_pd_state, port->pe_vdm_state,
		port->pe_state_next, port->pe_state_curr,
		port->pe_data.pd_connected, port->pe_data.pd_prev_connected,
		port->pe_data.explicit_contract, port->pe_data.pe_ready);
}

static void tcpc_show_cap_info(struct pd_port *port, char *buf, int len)
{
	int i;
	struct tcpm_power_cap_val cap;
	struct pe_data *pe_data = &port->pe_data;

	snprintf(buf, len, "%s = %d\n%s = %d\n",
		"local_selected_cap", pe_data->local_selected_cap,
		"remote_selected_cap", pe_data->remote_selected_cap);

	snprintf(buf + strlen(buf), len - strlen(buf), "%s\n",
		"local_src_cap(type, vmin, vmax, oper)");
	for (i = 0; i < port->local_src_cap.nr; i++) {
		tcpm_extract_power_cap_val(port->local_src_cap.pdos[i], &cap);
		snprintf(buf + strlen(buf), len - strlen(buf), "%d %d %d %d\n",
			cap.type, cap.min_mv, cap.max_mv, cap.ma);
	}

	snprintf(buf + strlen(buf), len - strlen(buf), "%s\n",
		"local_snk_cap(type, vmin, vmax, ioper)");
	for (i = 0; i < port->local_snk_cap.nr; i++) {
		tcpm_extract_power_cap_val(port->local_snk_cap.pdos[i], &cap);
		snprintf(buf + strlen(buf), len - strlen(buf), "%d %d %d %d\n",
			cap.type, cap.min_mv, cap.max_mv, cap.ma);
	}

	snprintf(buf + strlen(buf), len - strlen(buf), "%s\n",
		"remote_src_cap(type, vmin, vmax, ioper)");
	for (i = 0; i < pe_data->remote_src_cap.nr; i++) {
		tcpm_extract_power_cap_val(pe_data->remote_src_cap.pdos[i], &cap);
		snprintf(buf + strlen(buf), len - strlen(buf), "%d %d %d %d\n",
			cap.type, cap.min_mv, cap.max_mv, cap.ma);
	}

	snprintf(buf + strlen(buf), len - strlen(buf), "%s\n",
		"remote_snk_cap(type, vmin, vmax, ioper)");
	for (i = 0; i < pe_data->remote_snk_cap.nr; i++) {
		tcpm_extract_power_cap_val(pe_data->remote_snk_cap.pdos[i], &cap);
		snprintf(buf + strlen(buf), len - strlen(buf), "%d %d %d %d\n",
			cap.type, cap.min_mv, cap.max_mv, cap.ma);
	}
}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static ssize_t tcpc_show_property(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tcpc_device *tcpc = to_tcpc_device(dev);
	const ptrdiff_t offset = attr - tcpc_device_attributes;

	if (!tcpc)
		return -ENODEV;

	switch (offset) {
	case TCPC_DESC_ROLE_DEF:
		snprintf(buf, PAGE_SIZE, "%s\n", role_text[tcpc->desc.role_def]);
		break;
	case TCPC_DESC_RP_LEVEL:
		tcpc_show_rp_lvl(tcpc, buf, PAGE_SIZE);
		break;
	case TCPC_DESC_PD_TEST:
		snprintf(buf, PAGE_SIZE, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
			"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
			"1: pr_swap", "2: dr_swap", "3: vconn_swap", "4: soft reset",
			"5: hard reset", "6: get_src_cap", "7: get_sink_cap",
			"8: discover_id", "9: discover_cable", "10: disable pe",
			"11: enable pe", "12: disable typec", "13: enable typec",
			"14: error recovery", "15: swap role");
		break;
	case TCPC_DESC_INFO:
		tcpc_show_desc_info(tcpc, buf, PAGE_SIZE);
		break;
	case TCPC_DESC_TYPEC_STATE:
		tcpc_show_typec_state(tcpc, buf, PAGE_SIZE);
		break;
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	case TCPC_DESC_CAP_INFO:
		tcpc_show_cap_info(&tcpc->pd_port, buf, PAGE_SIZE);
		break;
	case TCPC_DESC_PE_READY:
		if (tcpc->pd_port.pe_data.pe_ready)
			snprintf(buf, PAGE_SIZE, "%s\n", "yes");
		else
			snprintf(buf, PAGE_SIZE, "%s\n", "no");
		break;
	case TCPC_DESC_PD_STATE:
		tcpc_show_pd_state(&tcpc->pd_port, buf, PAGE_SIZE);
		break;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
	default:
		break;
	}
	return strlen(buf);
}

static int tcpc_get_parameters(char *buf, long int *param, int num)
{
	char *token = NULL;
	int base, cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num; cnt++) {
		if (token) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16; /* Hexadecimal */
			else
				base = 10; /* Decimal */

			if (kstrtoul(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

static int tcpc_store_swap_pr_role(struct tcpc_device *tcpc)
{
	uint8_t role;

	role = tcpm_inquire_pd_power_role(tcpc);
	if (role == PD_ROLE_SINK)
		role = PD_ROLE_SOURCE;
	else
		role = PD_ROLE_SINK;

	return tcpm_dpm_pd_power_swap(tcpc, role, NULL);
}

static int tcpc_store_swap_dr_role(struct tcpc_device *tcpc)
{
	uint8_t role;

	role = tcpm_inquire_pd_data_role(tcpc);
	if (role == PD_ROLE_UFP)
		role = PD_ROLE_DFP;
	else
		role = PD_ROLE_UFP;

	return tcpm_dpm_pd_data_swap(tcpc, role, NULL);
}

static int tcpc_store_swap_vconn_role(struct tcpc_device *tcpc)
{
	uint8_t role;

	role = tcpm_inquire_pd_vconn_role(tcpc);
	if (role == PD_ROLE_VCONN_OFF)
		role = PD_ROLE_VCONN_ON;
	else
		role = PD_ROLE_VCONN_OFF;

	return tcpm_dpm_pd_vconn_swap(tcpc, role, NULL);
}

static const char *const typec_role_name[] = {
	"UNKNOWN",
	"SNK",
	"SRC",
	"DRP",
	"TrySRC",
	"TrySNK",
};

static int tcpc_change_typec_role(struct tcpc_device *tcpc, uint8_t role)
{
	if ((role == TYPEC_ROLE_UNKNOWN) || (role >= TYPEC_ROLE_NR)) {
		TCPC_ERR("wrong typec role=%d\n", role);
		return -EINVAL;
	}

	mutex_lock(&tcpc->access_lock);
	TCPC_INFO("%s: typec role: %s->%s\n", __func__,
		typec_role_name[tcpc->typec_role],
		typec_role_name[role]);
	tcpc->typec_role = role;
	mutex_unlock(&tcpc->access_lock);

	typecs_change_typec_role(tcpc);
	return 0;
}

static int tcpc_store_des_role_def(struct tcpc_device *tcpc, const char *buf)
{
	int ret;
	long int val;

	ret = tcpc_get_parameters((char *)buf, &val, 1);
	if (ret < 0) {
		dev_err(&tcpc->dev, "get parameters fail\n");
		return -EINVAL;
	}

	return tcpc_change_typec_role(tcpc, val);
}

static int tcpc_store_des_timer(struct tcpc_device *tcpc, const char *buf)
{
	int ret;
	long int val;

	ret = tcpc_get_parameters((char *)buf, &val, 1);
	if (ret < 0) {
		dev_err(&tcpc->dev, "get parameters fail\n");
		return -EINVAL;
	}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	if ((val > 0) && (val <= PD_PE_TIMER_END_ID))
		pd_enable_timer(&tcpc->pd_port, val);
	else if ((val > PD_PE_TIMER_END_ID) && (val < PD_TIMER_NR))
		timer_enable(tcpc, val);
#else
	if ((val > 0) && (val < PD_TIMER_NR))
		timer_enable(tcpc, val);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
	return 0;
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
static int tcpc_store_pd_test(struct tcpc_device *tcpc, const char *buf)
{
	int ret = 0;
	long int val;

	ret = tcpc_get_parameters((char *)buf, &val, 1);
	if (ret < 0) {
		dev_err(&tcpc->dev, "get parameters fail\n");
		return -EINVAL;
	}

	switch (val) {
	case 1: /* Power Role Swap */
		ret = tcpc_store_swap_pr_role(tcpc);
		break;
	case 2: /* Data Role Swap */
		ret = tcpc_store_swap_dr_role(tcpc);
		break;
	case 3: /* Vconn Swap */
		ret = tcpc_store_swap_vconn_role(tcpc);
		break;
	case 4: /* Software Reset */
		ret = tcpm_dpm_pd_soft_reset(tcpc, NULL);
		break;
	case 5: /* Hardware Reset */
		ret = tcpm_dpm_pd_hard_reset(tcpc, NULL);
		break;
	case 6: /* Source Cap */
		ret = tcpm_dpm_pd_get_source_cap(tcpc, NULL);
		break;
	case 7: /* Sink Cap */
		ret = tcpm_dpm_pd_get_sink_cap(tcpc, NULL);
		break;
	case 8: /* Discover Id */
		ret = tcpm_dpm_vdm_discover_id(tcpc, NULL);
		break;
	case 9: /* Discover Cable */
		ret = tcpm_dpm_vdm_discover_cable(tcpc, NULL);
		break;
	case 10: /* disable pe */
		typecs_pe_disable(tcpc);
		break;
	case 11: /* enable pe */
		typecs_pe_enable(tcpc);
		break;
	case 12: /* disable typec */
		typecs_typec_disable(tcpc);
		break;
	case 13: /* enable typec */
		typecs_typec_enable(tcpc);
		break;
	case 14: /* error recovery */
		typecs_typec_error_recovery(tcpc);
		break;
	case 15: /* swap typec role */
		typecs_swap_typec_role(tcpc);
		break;
	default:
		break;
	}

	return ret;
}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static ssize_t tcpc_store_property(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tcpc_device *tcpc = to_tcpc_device(dev);
	const ptrdiff_t offset = attr - tcpc_device_attributes;
	int ret;

	if (!tcpc)
		return -ENODEV;

	dev_info(dev, "%s: offset=%d port name=%s\n", __func__, offset,
		tcpc->desc.name);
	switch (offset) {
	case TCPC_DESC_ROLE_DEF:
		ret = tcpc_store_des_role_def(tcpc, buf);
		break;
	case TCPC_DESC_TIMER:
		ret = tcpc_store_des_timer(tcpc, buf);
		break;
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	case TCPC_DESC_PD_TEST:
		ret = tcpc_store_pd_test(tcpc, buf);
		break;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
	default:
		break;
	}

	if (ret < 0)
		return ret;
	return count;
}

static int tcpc_match_device_by_name(struct device *dev, const void *data)
{
	const char *name = data;
	struct tcpc_device *tcpc = dev_get_drvdata(dev);

	return strcmp(tcpc->desc.name, name) == 0;
}

struct tcpc_device *tcpc_dev_get_by_name(const char *name)
{
	struct device *dev = NULL;

	if (!name)
		return NULL;

	dev = class_find_device(tcpc_class, NULL, (const void *)name,
		tcpc_match_device_by_name);
	return dev ? dev_get_drvdata(dev) : NULL;
}

uint8_t get_no_rpsrc_state(void)
{
	uint8_t attach_type;
	struct tcpc_device *tcpc = tcpc_dev_get_by_name("notify_tcp_dev_ready");

	if (!tcpc)
		return 0;

	attach_type = typecs_get_attach_type(tcpc);
	return attach_type == TYPEC_ATTACHED_CUSTOM_SRC;
}

void typec_handle_timeout(struct tcpc_device *tcpc, uint32_t timer_id)
{
	typecs_handle_timeout(tcpc, timer_id);
}

void typec_disable_wakeup_timer(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return;

	typec_enable_wakeup_timer(tcpc->lpm, false);
}

void tcpc_awake_attach_lock(struct tcpc_device *tcpc)
{
	if (!tcpc)
		return;

	typec_attach_wake_lock(tcpc->lpm);
}

void tcpc_notfiy_pe_idle(struct tcpc_device *tcpc)
{
	typecs_notfiy_pe_idle(tcpc);
}

bool tcpc_is_wait_pe_idle(struct tcpc_device *tcpc)
{
	return typecs_is_wait_pe_idle(tcpc);
}

void tcpc_set_sink_curr(struct tcpc_device *tcpc, int curr)
{
	typecs_set_sink_curr(tcpc, curr);
}

void tcpc_typec_error_recovery(struct tcpc_device *tcpc)
{
	typecs_typec_error_recovery(tcpc);
}

void tcpc_typec_disable(struct tcpc_device *tcpc)
{
	typecs_typec_disable(tcpc);
}

void tcpc_typec_enable(struct tcpc_device *tcpc)
{
	typecs_typec_enable(tcpc);
}

void tcpc_handle_pe_pr_swap(struct tcpc_device *tcpc)
{
	typec_lock(tcpc);
	typecs_handle_pe_pr_swap(tcpc);
	typec_unlock(tcpc);
}

int tcpc_swap_typec_role(struct tcpc_device *tcpc)
{
	uint8_t attach_type;

	if (!tcpc)
		return TCPM_ERROR_PARAMETER;

	if (tcpc->typec_role < TYPEC_ROLE_DRP)
		return TCPM_ERROR_NOT_DRP_ROLE;

	attach_type = tcpc_get_attach_type(tcpc);
	if ((attach_type != TYPEC_ATTACHED_SNK) &&
		(attach_type != TYPEC_ATTACHED_SRC))
		return TCPM_ERROR_UNATTACHED;

	typecs_swap_typec_role(tcpc);
	return TCPM_SUCCESS;
}

int tcpc_set_rp_level(struct tcpc_device *tcpc, uint8_t res)
{
	if (!tcpc)
		return -EINVAL;

	switch (res) {
	case TYPEC_CC_RP_DFT:
	case TYPEC_CC_RP_1_5:
	case TYPEC_CC_RP_3_0:
		TCPC_INFO("%s: TypeC-Rp: %d\n", __func__, res);
		tcpc->typec_local_rp_level = res;
		break;
	default:
		TCPC_INFO("%s: TypeC-Unknown-Rp=%d\n", __func__, res);
		return -EINVAL;
	}

	if (typecs_get_attach_type(tcpc) != TYPEC_UNATTACHED)
		return tcpci_set_cc(tcpc, res);

	return 0;
}

uint8_t tcpc_get_attach_type(struct tcpc_device *tcpc)
{
	return typecs_get_attach_type(tcpc);
}

static void tcpc_device_release(struct device *dev)
{
	struct tcpc_device *tcpc = to_tcpc_device(dev);

	if (!tcpc)
		return;

	pr_info("%s : %s device release\n", __func__, dev_name(dev));
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	tcpci_event_deinit(tcpc);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
	timer_deinit(tcpc);
}

struct device_node *tcpc_get_dev_node(struct tcpc_device *tcpc, const char *name)
{
	struct device_node *np = NULL;

	if (!tcpc || !name)
		return np;

	np = of_find_node_by_name(tcpc->dev.parent->of_node, name);
	if (!np)
		np = of_find_node_by_name(tcpc->dev.of_node, name);

	return np;
}

static int tcpc_property_full_name(const char *desc_key, const char *name,
	char *buf, int buf_len)
{
	int full_len;
	int key_len = 0;

	if (desc_key)
		key_len = strlen(desc_key);
	full_len = key_len + strlen(name);
	if (full_len >= buf_len)
		return -EFAULT;

	if (desc_key)
		snprintf(buf, buf_len, "%s,%s", desc_key, name);
	else
		snprintf(buf, buf_len, "%s", name);

	return 0;
}

static int tcpc_property_read_u32(struct device_node *np, const char *desc_key,
	const char *name, u32 *out_value)
{
	int ret;
	char full_name[TCPC_PROPERTY_NAME_LEN] = { 0 };

	ret = tcpc_property_full_name(desc_key, name, full_name,
		TCPC_PROPERTY_NAME_LEN);
	if (ret)
		return ret;

	return of_property_read_u32(np, full_name, out_value);
}

static int tcpc_property_read_string(struct device_node *np,
	const char *desc_key, const char *name, const char **out_string)
{
	int ret;
	char full_name[TCPC_PROPERTY_NAME_LEN] = { 0 };

	ret = tcpc_property_full_name(desc_key, name, full_name,
		TCPC_PROPERTY_NAME_LEN);
	if (ret)
		return ret;

	return of_property_read_string(np, full_name, out_string);
}

static int tcpc_get_role_def(struct device_node *np, const char *desc_key)
{
	int val;

	if (tcpc_property_read_u32(np, desc_key, "role_def", &val) >= 0) {
		if (val >= TYPEC_ROLE_NR)
			val = TYPEC_ROLE_DRP;
	} else {
		pr_info("%s: use default role drp\n", __func__);
		val = TYPEC_ROLE_DRP;
	}

	return val;
}

static int tcpc_get_rp_level(struct device_node *np, const char *desc_key)
{
	int val;

	if (tcpc_property_read_u32(np, desc_key, "rp_level", &val) < 0)
		return TYPEC_CC_RP_DFT;

	switch (val) {
	case 0: /* RP Default */
		return TYPEC_CC_RP_DFT;
	case 1: /* RP 1.5V */
		return TYPEC_CC_RP_1_5;
	case 2: /* RP 3.0V */
		return TYPEC_CC_RP_3_0;
	default:
		break;
	}

	return TYPEC_CC_RP_DFT;
}

static int tcpc_get_vconn_supply(struct device_node *np, const char *desc_key)
{
#ifdef CONFIG_TCPC_VCONN_SUPPLY_MODE
	int val;

	if (tcpc_property_read_u32(np, desc_key, "vconn_supply", &val) >= 0) {
		if (val >= TCPC_VCONN_SUPPLY_NR)
			val = TCPC_VCONN_SUPPLY_ALWAYS;
	} else {
		pr_info("%s: use default vconn_supply\n", __func__);
		val = TCPC_VCONN_SUPPLY_ALWAYS;
	}

	return val;
#else
	return TCPC_VCONN_SUPPLY_NEVER;
#endif /* CONFIG_TCPC_VCONN_SUPPLY_MODE */
}

static void tcpc_init_desc(struct tcpc_device *tcpc, const char *desc_key)
{
	int len;
	struct tcpc_desc *desc = &tcpc->desc;
	const char *name = "default";
	struct device_node *np = of_find_node_by_name(NULL, "type_c_port0");

	if (!np)
		np = tcpc->dev.parent->of_node;

	desc->role_def = tcpc_get_role_def(np, desc_key);
	desc->rp_lvl = tcpc_get_rp_level(np, desc_key);
	desc->vconn_supply = tcpc_get_vconn_supply(np, desc_key);

	tcpc_property_read_string(np, desc_key, "name", (char const **)&name);
	len = strlen(name);
	if (len >= TCPC_MAX_NAME_LEN)
		len = TCPC_MAX_NAME_LEN - 1;
	strlcpy(desc->name, name, len + 1);

	pr_info("%s: role=%d,rp=%d,vconn=%d,name=%s\n", __func__,
		desc->role_def, desc->rp_lvl, desc->vconn_supply, desc->name);
}

static void tcpc_init_work(struct work_struct *work);
static void tcpc_event_init_work(struct work_struct *work);

struct tcpc_device *tcpc_device_register(struct device *parent,
	const char *desc_key, struct tcpc_ops *ops, void *drv_data)
{
	struct tcpc_device *tcpc = NULL;
	int ret;

	pr_info("%s register tcpc device\n", __func__);
	tcpc = devm_kzalloc(parent, sizeof(*tcpc), GFP_KERNEL);
	if (!tcpc) {
		pr_err("%s : allocate tcpc memeory failed\n", __func__);
		return NULL;
	}

	tcpm_notifier_init(tcpc);
	mutex_init(&tcpc->access_lock);
	mutex_init(&tcpc->typec_lock);

	tcpc->dev.class = tcpc_class;
	tcpc->dev.type = &tcpc_dev_type;
	tcpc->dev.parent = parent;
	tcpc->dev.release = tcpc_device_release;
	dev_set_drvdata(&tcpc->dev, tcpc);
	tcpc->drv_data = drv_data;
	tcpc_init_desc(tcpc, desc_key);

	dev_set_name(&tcpc->dev, tcpc->desc.name);
	tcpc->ops = ops;
	tcpc->typec_local_rp_level = tcpc->desc.rp_lvl;

#ifdef CONFIG_TCPC_VCONN_SUPPLY_MODE
	tcpc->tcpc_vconn_supply = tcpc->desc.vconn_supply;
#endif /* CONFIG_TCPC_VCONN_SUPPLY_MODE */

	ret = device_register(&tcpc->dev);
	if (ret) {
		devm_kfree(parent, tcpc);
		return ERR_PTR(ret);
	}

	INIT_DELAYED_WORK(&tcpc->init_work, tcpc_init_work);
	INIT_DELAYED_WORK(&tcpc->event_init_work, tcpc_event_init_work);
	timer_init(tcpc);
	typec_init_lpm(&tcpc->lpm, tcpc);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	pd_core_init(tcpc);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	ret = tcpm_dual_role_init(tcpc);
	if (ret < 0)
		dev_err(&tcpc->dev, "dual role usb init fail\n");

	return tcpc;
}
EXPORT_SYMBOL(tcpc_device_register);

static int tcpc_pd_dpm_disable_pd(void *client, bool disable)
{
	TYPEC_INFO("%s: disable=%d\n", __func__, disable);

	if (!client)
		return -EINVAL;

	if (disable)
		/* use PD_HW_CC_DETACHED event to complete pd policy disable */
		pd_put_cc_detach_event(client);

	return 0;
}

static struct pd_dpm_ops tcpc_device_pd_dpm_ops = {
	.pd_dpm_get_hw_dock_svid_exist = NULL,
	.pd_dpm_notify_direct_charge_status = tcpm_typec_notify_direct_charge,
	.pd_dpm_set_cc_mode = tcpci_set_cc_mode,
	.pd_dpm_get_cc_state = tcpci_get_cc_status,
	.pd_dpm_disable_pd = tcpc_pd_dpm_disable_pd,
	.pd_dpm_detect_emark_cable = NULL,
	.pd_dpm_reinit_chip = NULL,
	.data_role_swap = NULL,
};

static int tcpc_typec_init(struct tcpc_device *tcpc, uint8_t role)
{
	if (role >= TYPEC_ROLE_NR) {
		TCPC_INFO("%s: wrong typeC-role: %d\n", role);
		return -EINVAL;
	}

	pd_dpm_ops_register(&tcpc_device_pd_dpm_ops, tcpc);

	TCPC_INFO("%s: type_role=%s\n", __func__, typec_role_name[role]);
	tcpc->typec_role = role;
	tcpc->typec_remote_cc[0] = TYPEC_CC_VOLT_OPEN;
	tcpc->typec_remote_cc[1] = TYPEC_CC_VOLT_OPEN;
	typec_legacy_init(tcpc);

	return typecs_fsm_init(tcpc);
}

static int tcpc_device_irq_enable(struct tcpc_device *tcpc)
{
	int ret;

	ret = tcpci_init(tcpc, false);
	if (ret < 0) {
		pr_err("%s tcpc init fail\n", __func__);
		return ret;
	}
	typec_refresh_power_status(tcpc);

	typec_lock(tcpc);
	ret = tcpc_typec_init(tcpc, tcpc->desc.role_def + 1);
	typec_unlock(tcpc);
	if (ret < 0) {
		pr_err("%s : tcpc typec init fail\n", __func__);
		return ret;
	}
	tcpci_init_alert_mask(tcpc);

	/* delay 5s */
	schedule_delayed_work(&tcpc->event_init_work, msecs_to_jiffies(5000));

	pr_info("%s : tcpc irq enable ok\n", __func__);
	return 0;
}

#ifdef CONFIG_HW_USB_PD_REV30
static void tcpc_bat_update_worker(struct work_struct *work)
{
	struct tcpc_device *tcpc = container_of(work, struct tcpc_device,
		bat_update_work.work);
	union power_supply_propval value;
	int ret;

	ret = power_supply_get_property(tcpc->bat_psy,
		POWER_SUPPLY_PROP_CAPACITY, &value);
	if (ret == 0) {
		TCPC_INFO("%s battery update soc = %d\n", __func__,
			value.intval);
		tcpc->bat_soc = value.intval;
	} else {
		TCPC_ERR("%s get battery capacity fail\n", __func__);
	}

	ret = power_supply_get_property(tcpc->bat_psy,
		POWER_SUPPLY_PROP_STATUS, &value);
	if (ret == 0) {
		if (value.intval == POWER_SUPPLY_STATUS_CHARGING) {
			TCPC_INFO("%s battery charging\n", __func__);
			tcpc->charging_status = BSDO_BAT_INFO_CHARGING;
		} else if (value.intval == POWER_SUPPLY_STATUS_DISCHARGING) {
			TCPC_INFO("%s battery discharging\n", __func__);
			tcpc->charging_status = BSDO_BAT_INFO_DISCHARGING;
		} else {
			TCPC_INFO("%s battery idle\n", __func__);
			tcpc->charging_status = BSDO_BAT_INFO_IDLE;
		}
	} else {
		TCPC_ERR("%s get battery charger now fail\n", __func__);
	}

	/* magnification is 10 */
	tcpm_update_bat_status_soc(tcpc, tcpc->charging_status, tcpc->bat_soc * 10);
}

static int tcpc_bat_nb_call(struct notifier_block *nb, unsigned long val, void *v)
{
	struct tcpc_device *tcpc = container_of(nb, struct tcpc_device, bat_nb);
	struct power_supply *psy = (struct power_supply *)v;

	if (!tcpc) {
		TCPC_ERR("%s tcpc is null\n", __func__);
		return NOTIFY_OK;
	}

	if ((val == PSY_EVENT_PROP_CHANGED) &&
		(strcmp(psy->desc->name, "battery") == 0))
		schedule_delayed_work(&tcpc->bat_update_work, 0);
	return NOTIFY_OK;
}
#endif /* CONFIG_HW_USB_PD_REV30 */

static void tcpc_event_init_work(struct work_struct *work)
{
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	struct tcpc_device *tcpc = container_of(work, struct tcpc_device,
		event_init_work.work);
#ifdef CONFIG_HW_USB_PD_REV30
	int retval;
#endif /* CONFIG_HW_USB_PD_REV30 */
	uint8_t attach_type;

	typec_lock(tcpc);
	tcpci_event_init(tcpc);
	tcpc->pd_inited_flag = 1; /* MTK Only */
	attach_type = typecs_get_attach_type(tcpc);
	pr_info("%s typec attach_type = %d\n", __func__, attach_type);
	if (attach_type != TYPEC_UNATTACHED)
		pd_put_cc_attach_event(tcpc, attach_type);
	typec_unlock(tcpc);

#ifdef CONFIG_HW_USB_PD_REV30
	INIT_DELAYED_WORK(&tcpc->bat_update_work, tcpc_bat_update_worker);
	tcpc->bat_psy = power_supply_get_by_name("battery");
	if (!tcpc->bat_psy) {
		TCPC_ERR("%s get battery psy fail\n", __func__);
		return;
	}
	tcpc->charging_status = BSDO_BAT_INFO_IDLE;
	tcpc->bat_soc = 0;
	tcpc->bat_nb.notifier_call = tcpc_bat_nb_call;
	tcpc->bat_nb.priority = 0;
	retval = power_supply_reg_notifier(&tcpc->bat_nb);
	if (retval < 0)
		pr_err("%s register power supply notifier fail\n", __func__);
#endif /* CONFIG_HW_USB_PD_REV30 */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
}

static void tcpc_init_work(struct work_struct *work)
{
	struct tcpc_device *tcpc = container_of(work, struct tcpc_device,
		init_work.work);

	pr_info("%s force start\n", __func__);

	tcpc_device_irq_enable(tcpc);
}

void tcpc_device_unregister(struct device *dev, struct tcpc_device *tcpc)
{
	if (!tcpc)
		return;

	typecs_fsm_deinit(tcpc);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	wakeup_source_trash(&tcpc->pd_port.pps_request_wake_lock);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	typec_deinit_lpm(&tcpc->lpm);
	tcpm_dual_role_deinit(tcpc);
	device_unregister(&tcpc->dev);
}
EXPORT_SYMBOL(tcpc_device_unregister);

static void tcpc_init_attrs(struct device_type *dev_type)
{
	int i;

	dev_type->groups = tcpc_attr_groups;
	for (i = 0; i < ARRAY_SIZE(tcpc_device_attributes); i++)
		tcpc_sysfs_attrs[i] = &tcpc_device_attributes[i].attr;
}

static int __init tcpc_class_init(void)
{
	pr_info("%s: %s\n", __func__, TCPC_CORE_VERSION);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	dpm_check_supported_modes();
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	tcpc_class = class_create(THIS_MODULE, "tcpc");
	if (IS_ERR(tcpc_class)) {
		pr_info("unable to create tcpc class; errno = %ld\n",
		       PTR_ERR(tcpc_class));
		return PTR_ERR(tcpc_class);
	}
	tcpc_init_attrs(&tcpc_dev_type);
	tcpc_class->suspend = NULL;
	tcpc_class->resume = NULL;

	pr_info("tcpc class init OK\n");
	return 0;
}

static void __exit tcpc_class_exit(void)
{
	class_destroy(tcpc_class);
	pr_info("TCPC class un-init OK\n");
}

subsys_initcall(tcpc_class_init);
module_exit(tcpc_class_exit);

static int tcpc_class_complete_work(struct device *dev, void *data)
{
	struct tcpc_device *tcpc = dev_get_drvdata(dev);

	if (tcpc) {
		pr_info("%s = %s\n", __func__, dev_name(dev));
		tcpc_device_irq_enable(tcpc);
	}
	return 0;
}

static int __init tcpc_class_complete_init(void)
{
	if (!IS_ERR(tcpc_class))
		class_for_each_device(tcpc_class, NULL, NULL,
			tcpc_class_complete_work);

	return 0;
}

late_initcall_sync(tcpc_class_complete_init);

MODULE_DESCRIPTION("Tcpc Port Control Core");
MODULE_VERSION(TCPC_CORE_VERSION);
MODULE_LICENSE("GPL");
