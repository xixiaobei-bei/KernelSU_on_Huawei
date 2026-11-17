/*
 * power_event.c
 *
 * event for power module
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

#include "power_event.h"
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_event
HWLOG_REGIST();

static struct power_event_dev *g_power_event_dev;
static struct blocking_notifier_head g_power_event_bnh[POWER_BNT_END];
static struct atomic_notifier_head g_power_event_anh[POWER_ANT_END];

static const char * const g_power_event_ne_table[POWER_NE_END] = {
	/* section: for connect */
	[POWER_NE_USB_DISCONNECT] = "usb_disconnect",
	[POWER_NE_USB_CONNECT] = "usb_connect",
	[POWER_NE_WIRELESS_DISCONNECT] = "wireless_disconnect",
	[POWER_NE_WIRELESS_CONNECT] = "wireless_connect",
	[POWER_NE_WIRELESS_TX_START] = "wireless_tx_start",
	[POWER_NE_WIRELESS_TX_STOP] = "wireless_tx_stop",
	/* section: for charging */
	[POWER_NE_START_CHARGING] = "start_charging",
	[POWER_NE_STOP_CHARGING] = "stop_charging",
	[POWER_NE_SUSPEND_CHARGING] = "suspend_charging",
	/* section: for soc decimal */
	[POWER_NE_SOC_DECIMAL_DC] = "soc_decimal_dc",
	[POWER_NE_SOC_DECIMAL_WL_DC] = "soc_decimal_wl_dc",
	/* section: for water detect */
	[POWER_NE_WD_REPORT_DMD] = "wd_report_dmd",
	[POWER_NE_WD_REPORT_UEVENT] = "wd_report_uevent",
	[POWER_NE_WD_DETECT_BY_USB_DP_DN] = "wd_detect_by_usb_dp_dn",
	[POWER_NE_WD_DETECT_BY_USB_ID] = "wd_detect_by_usb_id",
	[POWER_NE_WD_DETECT_BY_USB_GPIO] = "wd_detect_by_usb_gpio",
	[POWER_NE_WD_DETECT_BY_AUDIO_DP_DN] = "wd_detect_by_audio_dp_dn",
	/* section: for uvdm */
	[POWER_NE_UVDM_RECEIVE] = "uvdm_receive",
	/* section: for direct charger */
	[POWER_NE_DC_CHECK_START] = "dc_check_start",
	[POWER_NE_DC_CHECK_SUCC] = "dc_check_succ",
	[POWER_NE_DC_LVC_CHARGING] = "dc_lvc_charging",
	[POWER_NE_DC_SC_CHARGING] = "dc_sc_charging",
	[POWER_NE_DC_STOP_CHARGE] = "dc_stop_charge",
	/* section: for lightstrap */
	[POWER_NE_LIGHTSTRAP_ON] = "lightstrap_on",
	[POWER_NE_LIGHTSTRAP_OFF] = "lightstrap_off",
	[POWER_NE_LIGHTSTRAP_GET_PRODUCT_INFO] = "lightstrap_get_product_info",
	[POWER_NE_LIGHTSTRAP_EPT] = "lightstrap_ept",
	/* section: for otg */
	[POWER_NE_OTG_SC_CHECK_STOP] = "otg_sc_check_stop",
	[POWER_NE_OTG_SC_CHECK_START] = "otg_sc_check_start",
	[POWER_NE_OTG_OCP_HANDLE] = "otg_ocp_handle",
	/* section: for wireless charge */
	[POWER_NE_WLC_CHARGER_VBUS] = "wlc_charger_vbus",
	[POWER_NE_WLC_ICON_TYPE] = "wlc_icon_type",
	[POWER_NE_WLC_TX_VSET] = "wlc_tx_vset",
	[POWER_NE_WLC_READY] = "wlc_ready",
	[POWER_NE_WLC_HS_SUCC] = "wlc_hs_succ",
	[POWER_NE_WLC_TX_CAP_SUCC] = "wlc_tx_cap_succ",
	[POWER_NE_WLC_CERT_SUCC] = "wlc_cert_succ",
	[POWER_NE_WLC_DC_START_CHARGING] = "wlc_dc_start_charging",
	[POWER_NE_WLC_VBUS_CONNECT] = "wlc_vbus_connect",
	[POWER_NE_WLC_VBUS_DISCONNECT] = "wlc_vbus_disconnect",
	[POWER_NE_WLC_WIRED_VBUS_CONNECT] = "wlc_wired_vbus_connect",
	[POWER_NE_WLC_WIRED_VBUS_DISCONNECT] = "wlc_wired_vbus_disconnect",
	/* section: for wireless tx */
	[POWER_NE_WLTX_GET_CFG] = "wltx_get_cfg",
	[POWER_NE_WLTX_HANDSHAKE_SUCC] = "wltx_handshake_succ",
	[POWER_NE_WLTX_CHARGEDONE] = "wltx_chargedone",
	[POWER_NE_WLTX_CEP_TIMEOUT] = "wltx_cep_timeout",
	[POWER_NE_WLTX_EPT_CMD] = "wltx_ept_cmd",
	[POWER_NE_WLTX_OVP] = "wltx_ovp",
	[POWER_NE_WLTX_OCP] = "wltx_ocp",
	[POWER_NE_WLTX_PING_RX] = "wltx_ping_rx",
	[POWER_NE_WLTX_HALL_APPROACH] = "wltx_hall_approach",
	[POWER_NE_WLTX_HALL_AWAY_FROM] = "wltx_hall_away_from",
	[POWER_NE_WLTX_ACC_DEV_CONNECTED] = "wltx_acc_dev_connected",
	[POWER_NE_WLTX_RCV_DPING] = "wltx_rcv_dping",
	[POWER_NE_WLTX_ASK_SET_VTX] = "wltx_ask_set_vtx",
	[POWER_NE_WLTX_GET_TX_CAP] = "wltx_get_tx_cap",
	[POWER_NE_WLTX_TX_FOD] = "wltx_tx_fod",
	[POWER_NE_WLTX_RP_DM_TIMEOUT] = "wltx_rp_dm_timeout",
	[POWER_NE_WLTX_TX_INIT] = "wltx_tx_init",
	[POWER_NE_WLTX_TX_AP_ON] = "wltx_tx_ap_on",
	[POWER_NE_WLTX_IRQ_SET_VTX] = "wltx_irq_set_vtx",
	[POWER_NE_WLTX_GET_RX_PRODUCT_TYPE] = "wltx_get_rx_product_type",
	[POWER_NE_WLTX_GET_RX_MAX_POWER] = "wltx_get_rx_max_power",
	[POWER_NE_WLTX_ASK_RX_EVT] = "wltx_ask_rx_evt",
	/* section: for wireless rx */
	[POWER_NE_WLRX_PWR_ON] = "wlrx_pwr_on",
	[POWER_NE_WLRX_READY] = "wlrx_ready",
	[POWER_NE_WLRX_OCP] = "wlrx_ocp",
	[POWER_NE_WLRX_OVP] = "wlrx_ovp",
	[POWER_NE_WLRX_OTP] = "wlrx_otp",
	[POWER_NE_WLRX_LDO_OFF] = "wlrx_ldo_off",
	[POWER_NE_WLRX_TX_ALARM] = "wlrx_plim_tx_alarm",
	[POWER_NE_WLRX_TX_BST_ERR] = "wlrx_plim_tx_bst_err",
	/* section: for charger */
	[POWER_NE_CHG_START_CHARGING] = "chg_start_charging",
	[POWER_NE_CHG_STOP_CHARGING] = "chg_stop_charging",
	[POWER_NE_CHG_CHARGING_DONE] = "chg_charging_done",
	[POWER_NE_CHG_PRE_STOP_CHARGING] = "chg_pre_stop_charging",
	/* section: for coul */
	[POWER_NE_COUL_LOW_VOL] = "coul_low_vol",
	/* section: for charger fault */
	[POWER_NE_CHG_FAULT_NON] = "chg_fault_non",
	[POWER_NE_CHG_FAULT_BOOST_OCP] = "chg_fault_boost_ocp",
	[POWER_NE_CHG_FAULT_VBAT_OVP] = "chg_fault_vbat_ovp",
	[POWER_NE_CHG_FAULT_SCHARGER] = "chg_fault_scharger",
	[POWER_NE_CHG_FAULT_I2C_ERR] = "chg_fault_i2c_err",
	[POWER_NE_CHG_FAULT_WEAKSOURCE] = "chg_fault_weaksource",
	[POWER_NE_CHG_FAULT_CHARGE_DONE] = "chg_fault_charge_done",
	/* section: for direct charger fault */
	[POWER_NE_DC_FAULT_NON] = "dc_fault_non",
	[POWER_NE_DC_FAULT_VBUS_OVP] = "dc_fault_vbus_ovp",
	[POWER_NE_DC_FAULT_REVERSE_OCP] = "dc_fault_reverse_ocp",
	[POWER_NE_DC_FAULT_OTP] = "dc_fault_otp",
	[POWER_NE_DC_FAULT_TSBUS_OTP] = "dc_fault_tsbus_otp",
	[POWER_NE_DC_FAULT_TSBAT_OTP] = "dc_fault_tsbat_otp",
	[POWER_NE_DC_FAULT_TDIE_OTP] = "dc_fault_tdie_otp",
	[POWER_NE_DC_FAULT_INPUT_OCP] = "dc_fault_input_ocp",
	[POWER_NE_DC_FAULT_VDROP_OVP] = "dc_fault_vdrop_ovp",
	[POWER_NE_DC_FAULT_AC_OVP] = "dc_fault_ac_ovp",
	[POWER_NE_DC_FAULT_VBAT_OVP] = "dc_fault_vbat_ovp",
	[POWER_NE_DC_FAULT_IBAT_OCP] = "dc_fault_ibat_ocp",
	[POWER_NE_DC_FAULT_IBUS_OCP] = "dc_fault_ibus_ocp",
	[POWER_NE_DC_FAULT_CONV_OCP] = "dc_fault_conv_ocp",
	[POWER_NE_DC_FAULT_LTC7820] = "dc_fault_ltc7820",
	[POWER_NE_DC_FAULT_INA231] = "dc_fault_ina231",
	[POWER_NE_DC_FAULT_CC_SHORT] = "dc_fault_cc_short",
	/* section: for uvdm charger fault */
	[POWER_NE_UVDM_FAULT_OTG] = "uvdm_fault_otg",
	[POWER_NE_UVDM_FAULT_COVER_ABNORMAL] = "uvdm_fault_cover_abnormal",
	/* section: for typec */
	[POWER_NE_TYPEC_CURRENT_CHANGE] = "typec_current_change",
	/* section: for third platform pd */
	[POWER_NE_THIRDPLAT_PD_START] = "thirdplat_pd_start",
	[POWER_NE_THIRDPLAT_PD_STOP] = "thirdplat_pd_stop",
};

static const char *power_event_get_ne_name(unsigned int event)
{
	if ((event >= POWER_NE_BEGIN) && (event < POWER_NE_END))
		return g_power_event_ne_table[event];

	return "illegal ne";
}

static struct power_event_dev *power_event_get_dev(void)
{
	if (!g_power_event_dev) {
		hwlog_err("g_power_event_dev is null\n");
		return NULL;
	}

	return g_power_event_dev;
}

void power_event_report_uevent(const struct power_event_notify_data *n_data)
{
	char uevent_buf[POWER_EVENT_NOTIFY_SIZE] = { 0 };
	char *envp[POWER_EVENT_NOTIFY_NUM] = { uevent_buf, NULL };
	int i, ret;
	struct power_event_dev *l_dev = power_event_get_dev();

	if (!l_dev || !l_dev->sysfs_ne)
		return;

	if (!n_data || !n_data->event) {
		hwlog_err("n_data or event is null\n");
		return;
	}

	if (n_data->event_len >= POWER_EVENT_NOTIFY_SIZE) {
		hwlog_err("event_len is invalid\n");
		return;
	}

	for (i = 0; i < n_data->event_len; i++)
		uevent_buf[i] = n_data->event[i];
	hwlog_info("receive uevent_buf %d,%s\n", n_data->event_len, uevent_buf);

	ret = kobject_uevent_env(l_dev->sysfs_ne, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_err("notify uevent fail, ret=%d\n", ret);
}

void power_event_notify_sysfs(struct kobject *kobj, const char *dir, const char *attr)
{
	if (!kobj)
		return;

	if (!attr) {
		hwlog_err("attr is null\n");
		return;
	}

	if (dir)
		hwlog_info("receive sysfs event dir=%s attr=%s\n", dir, attr);
	else
		hwlog_info("receive sysfs event attr=%s\n", attr);

	sysfs_notify(kobj, dir, attr);
}

static void power_event_bnc_init(void)
{
	int i;

	for (i = 0; i < POWER_BNT_END; i++)
		BLOCKING_INIT_NOTIFIER_HEAD(&g_power_event_bnh[i]);
}

int power_event_bnc_cond_register(unsigned int type, struct notifier_block *nb)
{
	if (type >= POWER_BNT_END) {
		hwlog_err("invalid type=%u\n", type);
		return NOTIFY_OK;
	}

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_cond_register(&g_power_event_bnh[type], nb);
}

int power_event_bnc_register(unsigned int type, struct notifier_block *nb)
{
	if (type >= POWER_BNT_END) {
		hwlog_err("invalid type=%u\n", type);
		return NOTIFY_OK;
	}

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_register(&g_power_event_bnh[type], nb);
}

int power_event_bnc_unregister(unsigned int type, struct notifier_block *nb)
{
	if (type >= POWER_BNT_END) {
		hwlog_err("invalid type=%u\n", type);
		return NOTIFY_OK;
	}

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return blocking_notifier_chain_unregister(&g_power_event_bnh[type], nb);
}

void power_event_bnc_notify(unsigned int type, unsigned long event, void *data)
{
	if (type >= POWER_BNT_END) {
		hwlog_err("invalid type=%u\n", type);
		return;
	}

	hwlog_info("receive blocking event type=%u event=%lu,%s\n",
		type, event, power_event_get_ne_name(event));
	blocking_notifier_call_chain(&g_power_event_bnh[type], event, data);
}

static void power_event_anc_init(void)
{
	int i;

	for (i = 0; i < POWER_ANT_END; i++)
		ATOMIC_INIT_NOTIFIER_HEAD(&g_power_event_anh[i]);
}

int power_event_anc_register(unsigned int type, struct notifier_block *nb)
{
	if (type >= POWER_ANT_END) {
		hwlog_err("invalid type=%u\n", type);
		return NOTIFY_OK;
	}

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return atomic_notifier_chain_register(&g_power_event_anh[type], nb);
}

int power_event_anc_unregister(unsigned int type, struct notifier_block *nb)
{
	if (type >= POWER_ANT_END) {
		hwlog_err("invalid type=%u\n", type);
		return NOTIFY_OK;
	}

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	return atomic_notifier_chain_unregister(&g_power_event_anh[type], nb);
}

void power_event_anc_notify(unsigned int type, unsigned long event, void *data)
{
	if (type >= POWER_ANT_END) {
		hwlog_err("invalid type=%u\n", type);
		return;
	}

	hwlog_info("receive atomic event type=%u event=%lu,%s\n",
		type, event, power_event_get_ne_name(event));
	atomic_notifier_call_chain(&g_power_event_anh[type], event, data);
}

#if defined(CONFIG_SYSFS)
static ssize_t power_event_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info power_event_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(power_event, 0220, POWER_EVENT_SYSFS_TRIGGER, trigger),
};

#define POWER_EVENT_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_event_sysfs_field_tbl)

static struct attribute *power_event_sysfs_attrs[POWER_EVENT_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_event_sysfs_attr_group = {
	.attrs = power_event_sysfs_attrs,
};

static ssize_t power_event_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_event_dev *l_dev = power_event_get_dev();
	struct power_event_notify_data n_data;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_event_sysfs_field_tbl, POWER_EVENT_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case POWER_EVENT_SYSFS_TRIGGER:
		if (count >= POWER_EVENT_WR_BUF_SIZE) {
			hwlog_err("input too long\n");
			return -EINVAL;
		}

		n_data.event = buf;
		n_data.event_len = count;
		power_event_report_uevent(&n_data);
		break;
	default:
		break;
	}

	return count;
}

static struct device *power_event_sysfs_create_group(void)
{
	power_sysfs_init_attrs(power_event_sysfs_attrs,
		power_event_sysfs_field_tbl, POWER_EVENT_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "power_event",
		&power_event_sysfs_attr_group);
}

static void power_event_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_event_sysfs_attr_group);
}
#else
static inline struct device *power_event_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_event_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_event_init(void)
{
	struct power_event_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_power_event_dev = l_dev;
	power_event_bnc_init();
	power_event_anc_init();

	l_dev->dev = power_event_sysfs_create_group();
	if (l_dev->dev)
		l_dev->sysfs_ne = &l_dev->dev->kobj;

	return 0;
}

static void __exit power_event_exit(void)
{
	struct power_event_dev *l_dev = g_power_event_dev;

	if (!l_dev)
		return;

	power_event_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_power_event_dev = NULL;
}

subsys_initcall(power_event_init);
module_exit(power_event_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power event module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
