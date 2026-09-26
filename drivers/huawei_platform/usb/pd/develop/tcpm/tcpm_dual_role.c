/*
 * tcpm_dual_role.c
 *
 * tcpm dual role
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

#include "inc/tcpm_dual_role.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include "inc/tcpm.h"
#include "inc/tcpc_core.h"

static enum dual_role_property tcpm_dual_role_props[] = {
	DUAL_ROLE_PROP_SUPPORTED_MODES,
	DUAL_ROLE_PROP_MODE,
	DUAL_ROLE_PROP_PR,
	DUAL_ROLE_PROP_DR,
	DUAL_ROLE_PROP_VCONN_SUPPLY,
};

static inline struct tcpm_dual_role *tcpm_dual_role_phy_to_dev(
	struct dual_role_phy_instance *dual_role)
{
	struct tcpc_device *tcpc = dev_get_drvdata(dual_role->dev.parent);

	return tcpc ? &tcpc->dr_dev : NULL;
}

static inline struct tcpm_dual_role *tcpm_dual_role_tcpc_to_dev(
	struct tcpc_device *tcpc)
{
	return tcpc ? &tcpc->dr_dev : NULL;
}

void tcpm_dual_role_set_mode(struct tcpc_device *tcpc, uint8_t mode)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dr_dev->mode = mode;
	dr_dev->dr = !mode;
	dual_role_instance_changed(dr_dev->phy);
}

void tcpm_dual_role_set_pr(struct tcpc_device *tcpc, uint8_t pr)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dr_dev->pr = pr;
	dual_role_instance_changed(dr_dev->phy);
}

void tcpm_dual_role_set_vconn(struct tcpc_device *tcpc, uint8_t vconn)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dr_dev->vconn_supply = vconn;
	dual_role_instance_changed(dr_dev->phy);
}

void tcpm_dual_role_set_sink(struct tcpc_device *tcpc)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dr_dev->pr = DUAL_ROLE_PROP_PR_SNK;
	dr_dev->dr = DUAL_ROLE_PROP_DR_DEVICE;
	dr_dev->mode = DUAL_ROLE_PROP_MODE_UFP;
	dr_dev->vconn_supply = DUAL_ROLE_PROP_VCONN_SUPPLY_NO;
	dual_role_instance_changed(dr_dev->phy);
}

void tcpm_dual_role_set_src(struct tcpc_device *tcpc)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dr_dev->pr = DUAL_ROLE_PROP_PR_SRC;
	dr_dev->dr = DUAL_ROLE_PROP_DR_HOST;
	dr_dev->mode = DUAL_ROLE_PROP_MODE_DFP;
	dr_dev->vconn_supply = DUAL_ROLE_PROP_VCONN_SUPPLY_YES;
	dual_role_instance_changed(dr_dev->phy);
}

void tcpm_dual_role_set_dft(struct tcpc_device *tcpc)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dr_dev->pr = DUAL_ROLE_PROP_PR_NONE;
	dr_dev->dr = DUAL_ROLE_PROP_DR_NONE;
	dr_dev->mode = DUAL_ROLE_PROP_MODE_NONE;
	dr_dev->vconn_supply = DUAL_ROLE_PROP_VCONN_SUPPLY_NO;
	dual_role_instance_changed(dr_dev->phy);
}

void tcpm_dual_role_changed(struct tcpc_device *tcpc)
{
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_tcpc_to_dev(tcpc);

	if (!dr_dev || !dr_dev->phy)
		return;

	dual_role_instance_changed(dr_dev->phy);
}

static int tcpm_dual_role_get_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, unsigned int *val)
{
	int ret = 0;
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_phy_to_dev(dual_role);

	if (!dr_dev)
		return -ENODEV;

	switch (prop) {
	case DUAL_ROLE_PROP_SUPPORTED_MODES:
		*val = dr_dev->supported_modes;
		break;
	case DUAL_ROLE_PROP_MODE:
		*val = dr_dev->mode;
		break;
	case DUAL_ROLE_PROP_PR:
		*val = dr_dev->pr;
		break;
	case DUAL_ROLE_PROP_DR:
		*val = dr_dev->dr;
		break;
	case DUAL_ROLE_PROP_VCONN_SUPPLY:
		*val = dr_dev->vconn_supply;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int tcpm_dual_role_prop_is_writeable(
	struct dual_role_phy_instance *dual_role, enum dual_role_property prop)
{
	int retval = -EINVAL;
	struct tcpm_dual_role *dr_dev = tcpm_dual_role_phy_to_dev(dual_role);

	if (!dr_dev)
		return -ENODEV;

	switch (prop) {
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	case DUAL_ROLE_PROP_PR:
	case DUAL_ROLE_PROP_DR:
	case DUAL_ROLE_PROP_VCONN_SUPPLY:
#else
	case DUAL_ROLE_PROP_MODE:
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
		if (dr_dev->supported_modes ==
			DUAL_ROLE_SUPPORTED_MODES_DFP_AND_UFP)
			retval = 1;
		break;
	default:
		break;
	}

	return retval;
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
static int tcpm_dual_role_set_prop_pr(struct tcpc_device *tcpc,
	unsigned int val)
{
	int ret;
	uint8_t role;

	switch (val) {
	case DUAL_ROLE_PROP_PR_SRC:
		role = PD_ROLE_SOURCE;
		break;
	case DUAL_ROLE_PROP_PR_SNK:
		role = PD_ROLE_SINK;
		break;
	default:
		return 0;
	}

	if (val == tcpc->dr_dev.pr) {
		pr_info("%s: same power role=%d\n", __func__, val);
		return 0;
	}

	ret = tcpm_dpm_pd_power_swap(tcpc, role, NULL);
	pr_info("%s: power role swap %d->%d,ret=%d\n", __func__,
		tcpc->dr_dev.pr, val, ret);
	if (ret == TCPM_ERROR_NO_PD_CONNECTED) {
		ret = tcpm_typec_role_swap(tcpc);
		pr_info("%s: typec role swap %d->%d, ret=%d\n",
			__func__, tcpc->dr_dev.pr, val, ret);
	}

	return ret;
}

static int tcpm_dual_role_set_prop_dr(struct tcpc_device *tcpc,
	unsigned int val)
{
	int ret;
	uint8_t role;

	switch (val) {
	case DUAL_ROLE_PROP_DR_HOST:
		role = PD_ROLE_DFP;
		break;
	case DUAL_ROLE_PROP_DR_DEVICE:
		role = PD_ROLE_UFP;
		break;
	default:
		return 0;
	}

	if (val == tcpc->dr_dev.dr) {
		pr_info("%s: same data role %d\n", __func__, val);
		return 0;
	}

	ret = tcpm_dpm_pd_data_swap(tcpc, role, NULL);
	pr_info("%s: data role swap %d->%d,ret=%d\n", __func__,
		tcpc->dr_dev.dr, val, ret);
	return ret;
}

static int tcpm_dual_role_set_prop_vconn(struct tcpc_device *tcpc,
	unsigned int val)
{
	int ret;
	uint8_t role;

	switch (val) {
	case DUAL_ROLE_PROP_VCONN_SUPPLY_NO:
		role = PD_ROLE_VCONN_OFF;
		break;
	case DUAL_ROLE_PROP_VCONN_SUPPLY_YES:
		role = PD_ROLE_VCONN_ON;
		break;
	default:
		return 0;
	}

	if (val == tcpc->dr_dev.vconn_supply) {
		pr_info("%s: same vconn role %d\n", __func__, val);
		return 0;
	}

	ret = tcpm_dpm_pd_vconn_swap(tcpc, role, NULL);
	pr_info("%s: vconn swap %d->%d,ret=%d\n", __func__,
		tcpc->dr_dev.vconn_supply, val, ret);
	return ret;
}
#else
static int tcpm_dual_role_set_prop_mode(struct tcpc_device *tcpc,
	unsigned int val)
{
	int ret;

	if (val == tcpc->dr_dev.mode) {
		pr_info("%s: same typec role %d\n", __func__, val);
		return 0;
	}

	ret = tcpm_typec_role_swap(tcpc);
	pr_info("%s: typec role swap %d->%d,ret=%d\n", __func__,
		tcpc->dr_dev.dual_role_mode, val, ret);
	return ret;
}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static int tcpm_dual_role_set_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, const unsigned int *val)
{
	struct tcpc_device *tcpc = dev_get_drvdata(dual_role->dev.parent);

	switch (prop) {
#ifdef CONFIG_HW_USB_POWER_DELIVERY
	case DUAL_ROLE_PROP_PR:
		tcpm_dual_role_set_prop_pr(tcpc, *val);
		break;
	case DUAL_ROLE_PROP_DR:
		tcpm_dual_role_set_prop_dr(tcpc, *val);
		break;
	case DUAL_ROLE_PROP_VCONN_SUPPLY:
		tcpm_dual_role_set_prop_vconn(tcpc, *val);
		break;
#else
	case DUAL_ROLE_PROP_MODE:
		tcpm_dual_role_set_prop_mode(tcpc, *val);
		break;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
	default:
		break;
	}

	return 0;
}

static void tcpm_dual_role_get_desc(struct tcpc_device *tcpc)
{
	u32 val;
	struct device_node *np = of_find_node_by_name(NULL, tcpc->desc.name);

	if (!np)
		np = tcpc->dev.parent->of_node;

	if (of_property_read_u32(np, "tcpc-dual,supported_modes", &val) >= 0) {
		if (val > DUAL_ROLE_PROP_SUPPORTED_MODES_TOTAL)
			tcpc->dr_dev.supported_modes =
					DUAL_ROLE_SUPPORTED_MODES_DFP_AND_UFP;
		else
			tcpc->dr_dev.supported_modes = val;
	}
}

static struct dual_role_phy_instance *tcpm_dual_role_create_phy(
	struct tcpc_device *tcpc)
{
	int len;
	char *str_name = NULL;
	struct dual_role_phy_desc *dual_desc = NULL;
	struct dual_role_phy_instance *phy = NULL;

	dual_desc = devm_kzalloc(&tcpc->dev, sizeof(*dual_desc), GFP_KERNEL);
	if (!dual_desc)
		return NULL;

	len = strlen(tcpc->desc.name) + TCPM_DUAL_ROLE_EXT_NAME_LEN;
	str_name = devm_kzalloc(&tcpc->dev, len, GFP_KERNEL);
	if (!str_name)
		goto desc_name_fail;

	snprintf(str_name, len, "dual-role-%s", tcpc->desc.name);
	dual_desc->name = str_name;

	dual_desc->properties = tcpm_dual_role_props;
	dual_desc->num_properties = ARRAY_SIZE(tcpm_dual_role_props);
	dual_desc->get_property = tcpm_dual_role_get_prop;
	dual_desc->set_property = tcpm_dual_role_set_prop;
	dual_desc->property_is_writeable = tcpm_dual_role_prop_is_writeable;

	phy = devm_dual_role_instance_register(&tcpc->dev, dual_desc);
	if (IS_ERR(phy)) {
		pr_err("tcpc fail to register dual role usb\n");
		goto dual_role_register_fail;
	}
	return phy;

dual_role_register_fail:
	devm_kfree(&tcpc->dev, str_name);
desc_name_fail:
	devm_kfree(&tcpc->dev, dual_desc);
	return NULL;
}


int tcpm_dual_role_init(struct tcpc_device *tcpc)
{
	struct tcpm_dual_role *dr_dev = NULL;

	if (!tcpc)
		return -EINVAL;

	dr_dev = &tcpc->dr_dev;
	dr_dev->phy = tcpm_dual_role_create_phy(tcpc);
	if (!dr_dev->phy)
		return -ENOMEM;

	tcpm_dual_role_get_desc(tcpc);
	dr_dev->pr = DUAL_ROLE_PROP_PR_NONE;
	dr_dev->dr = DUAL_ROLE_PROP_DR_NONE;
	dr_dev->mode = DUAL_ROLE_PROP_MODE_NONE;
	dr_dev->vconn_supply = DUAL_ROLE_PROP_VCONN_SUPPLY_NO;
	return 0;
}

void tcpm_dual_role_deinit(struct tcpc_device *tcpc)
{
	if (!tcpc || !tcpc->dr_dev.phy)
		return;

	devm_dual_role_instance_unregister(&tcpc->dev, tcpc->dr_dev.phy);
}