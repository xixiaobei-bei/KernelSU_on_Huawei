/*
 * power_event_ne.h
 *
 * notifier event for power module
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

#ifndef _POWER_EVENT_NE_H_
#define _POWER_EVENT_NE_H_

#include <linux/notifier.h>
#include <linux/kobject.h>

#define POWER_EVENT_NOTIFY_SIZE 1024
#define POWER_EVENT_NOTIFY_NUM  2

/* define blocking notifier type for power module */
enum power_event_blocking_notifier_type {
	POWER_BNT_BEGIN = 0,
	POWER_BNT_CONNECT = POWER_BNT_BEGIN,
	POWER_BNT_CHARGING,
	POWER_BNT_SOC_DECIMAL,
	POWER_BNT_WD,
	POWER_BNT_UVDM,
	POWER_BNT_DC,
	POWER_BNT_LIGHTSTRAP,
	POWER_BNT_OTG,
	POWER_BNT_WLC,
	POWER_BNT_WLTX_AUX,
	POWER_BNT_WLRX,
	POWER_BNT_WLTX,
	POWER_BNT_CHG,
	POWER_BNT_COUL,
	POWER_BNT_TYPEC,
	POWER_BNT_THIRDPLAT_PD,
	POWER_BNT_END,
};

/* define atomic notifier type for power module */
enum power_event_atomic_notifier_type {
	POWER_ANT_BEGIN = 0,
	POWER_ANT_CHARGE_FAULT = POWER_ANT_BEGIN,
	POWER_ANT_LVC_FAULT,
	POWER_ANT_SC_FAULT,
	POWER_ANT_UVDM_FAULT,
	POWER_ANT_END,
};

/* define notifier event for power module */
enum power_event_notifier_list {
	POWER_NE_BEGIN = 0,
	/* section: for connect */
	POWER_NE_USB_DISCONNECT = POWER_NE_BEGIN,
	POWER_NE_USB_CONNECT,
	POWER_NE_WIRELESS_DISCONNECT,
	POWER_NE_WIRELESS_CONNECT,
	POWER_NE_WIRELESS_TX_START,
	POWER_NE_WIRELESS_TX_STOP, /* num:5 */
	/* section: for charging */
	POWER_NE_START_CHARGING,
	POWER_NE_STOP_CHARGING,
	POWER_NE_SUSPEND_CHARGING,
	/* section: for soc decimal */
	POWER_NE_SOC_DECIMAL_DC,
	POWER_NE_SOC_DECIMAL_WL_DC, /* num:10 */
	/* section: for water detect */
	POWER_NE_WD_REPORT_DMD,
	POWER_NE_WD_REPORT_UEVENT,
	POWER_NE_WD_DETECT_BY_USB_DP_DN,
	POWER_NE_WD_DETECT_BY_USB_ID,
	POWER_NE_WD_DETECT_BY_USB_GPIO, /* num:15 */
	POWER_NE_WD_DETECT_BY_AUDIO_DP_DN,
	/* section: for uvdm */
	POWER_NE_UVDM_RECEIVE,
	/* section: for direct charger */
	POWER_NE_DC_CHECK_START,
	POWER_NE_DC_CHECK_SUCC,
	POWER_NE_DC_LVC_CHARGING, /* num:20 */
	POWER_NE_DC_SC_CHARGING,
	POWER_NE_DC_STOP_CHARGE,
	/* section: for lightstrap */
	POWER_NE_LIGHTSTRAP_ON,
	POWER_NE_LIGHTSTRAP_OFF,
	POWER_NE_LIGHTSTRAP_GET_PRODUCT_INFO, /* num:25 */
	POWER_NE_LIGHTSTRAP_EPT,
	/* section: for otg */
	POWER_NE_OTG_SC_CHECK_STOP,
	POWER_NE_OTG_SC_CHECK_START,
	POWER_NE_OTG_OCP_HANDLE,
	/* section: for wireless charge */
	POWER_NE_WLC_CHARGER_VBUS, /* num:30 */
	POWER_NE_WLC_ICON_TYPE,
	POWER_NE_WLC_TX_VSET,
	POWER_NE_WLC_READY,
	POWER_NE_WLC_HS_SUCC,
	POWER_NE_WLC_TX_CAP_SUCC, /* num:35 */
	POWER_NE_WLC_CERT_SUCC,
	POWER_NE_WLC_DC_START_CHARGING,
	POWER_NE_WLC_VBUS_CONNECT,
	POWER_NE_WLC_VBUS_DISCONNECT,
	POWER_NE_WLC_WIRED_VBUS_CONNECT, /* num:40 */
	POWER_NE_WLC_WIRED_VBUS_DISCONNECT,
	/* section: for wireless tx */
	POWER_NE_WLTX_GET_CFG,
	POWER_NE_WLTX_HANDSHAKE_SUCC,
	POWER_NE_WLTX_CHARGEDONE,
	POWER_NE_WLTX_CEP_TIMEOUT, /* num:45 */
	POWER_NE_WLTX_EPT_CMD,
	POWER_NE_WLTX_OVP,
	POWER_NE_WLTX_OCP,
	POWER_NE_WLTX_PING_RX,
	POWER_NE_WLTX_HALL_APPROACH, /* num:50 */
	POWER_NE_WLTX_AUX_PEN_HALL_APPROACH,
	POWER_NE_WLTX_AUX_KB_HALL_APPROACH,
	POWER_NE_WLTX_HALL_AWAY_FROM,
	POWER_NE_WLTX_AUX_PEN_HALL_AWAY_FROM,
	POWER_NE_WLTX_AUX_KB_HALL_AWAY_FROM, /* num:55 */
	POWER_NE_WLTX_ACC_DEV_CONNECTED,
	POWER_NE_WLTX_RCV_DPING,
	POWER_NE_WLTX_ASK_SET_VTX,
	POWER_NE_WLTX_GET_TX_CAP,
	POWER_NE_WLTX_TX_FOD, /* num:60 */
	POWER_NE_WLTX_RP_DM_TIMEOUT,
	POWER_NE_WLTX_TX_INIT,
	POWER_NE_WLTX_TX_AP_ON,
	POWER_NE_WLTX_IRQ_SET_VTX,
	POWER_NE_WLTX_GET_RX_PRODUCT_TYPE, /* num:65 */
	POWER_NE_WLTX_GET_RX_MAX_POWER,
	POWER_NE_WLTX_ASK_RX_EVT,
	/* section: for wireless rx */
	POWER_NE_WLRX_PWR_ON,
	POWER_NE_WLRX_READY,
	POWER_NE_WLRX_OCP, /* num:70 */
	POWER_NE_WLRX_OVP,
	POWER_NE_WLRX_OTP,
	POWER_NE_WLRX_LDO_OFF,
	POWER_NE_WLRX_TX_ALARM,
	POWER_NE_WLRX_TX_BST_ERR, /* num:75 */
	/* section: for charger */
	POWER_NE_CHG_START_CHARGING,
	POWER_NE_CHG_STOP_CHARGING,
	POWER_NE_CHG_CHARGING_DONE,
	POWER_NE_CHG_PRE_STOP_CHARGING,
	/* section: for coul */
	POWER_NE_COUL_LOW_VOL, /* num:80 */
	/* section: for charger fault */
	POWER_NE_CHG_FAULT_NON,
	POWER_NE_CHG_FAULT_BOOST_OCP,
	POWER_NE_CHG_FAULT_VBAT_OVP,
	POWER_NE_CHG_FAULT_SCHARGER,
	POWER_NE_CHG_FAULT_I2C_ERR, /* num:85 */
	POWER_NE_CHG_FAULT_WEAKSOURCE,
	POWER_NE_CHG_FAULT_CHARGE_DONE,
	/* section: for direct charger fault */
	POWER_NE_DC_FAULT_NON,
	POWER_NE_DC_FAULT_VBUS_OVP,
	POWER_NE_DC_FAULT_REVERSE_OCP, /* num:90 */
	POWER_NE_DC_FAULT_OTP,
	POWER_NE_DC_FAULT_TSBUS_OTP,
	POWER_NE_DC_FAULT_TSBAT_OTP,
	POWER_NE_DC_FAULT_TDIE_OTP,
	POWER_NE_DC_FAULT_INPUT_OCP, /* num:95 */
	POWER_NE_DC_FAULT_VDROP_OVP,
	POWER_NE_DC_FAULT_AC_OVP,
	POWER_NE_DC_FAULT_VBAT_OVP,
	POWER_NE_DC_FAULT_IBAT_OCP,
	POWER_NE_DC_FAULT_IBUS_OCP, /* num:100 */
	POWER_NE_DC_FAULT_CONV_OCP,
	POWER_NE_DC_FAULT_LTC7820,
	POWER_NE_DC_FAULT_INA231,
	POWER_NE_DC_FAULT_CC_SHORT,
	/* section: for uvdm charger fault */
	POWER_NE_UVDM_FAULT_OTG, /* num:105 */
	POWER_NE_UVDM_FAULT_COVER_ABNORMAL,
	/* section: for typec */
	POWER_NE_TYPEC_CURRENT_CHANGE,
	/* section: for third platform pd */
	POWER_NE_THIRDPLAT_PD_START,
	POWER_NE_THIRDPLAT_PD_STOP,
	POWER_NE_END,
};

struct power_event_notify_data {
	const char *event;
	int event_len;
};

int power_event_bnc_cond_register(unsigned int type, struct notifier_block *nb);
int power_event_bnc_register(unsigned int type, struct notifier_block *nb);
int power_event_bnc_unregister(unsigned int type, struct notifier_block *nb);
void power_event_bnc_notify(unsigned int type, unsigned long event, void *data);
int power_event_anc_register(unsigned int type, struct notifier_block *nb);
int power_event_anc_unregister(unsigned int type, struct notifier_block *nb);
void power_event_anc_notify(unsigned int type, unsigned long event, void *data);
void power_event_report_uevent(const struct power_event_notify_data *n_data);
void power_event_notify_sysfs(struct kobject *kobj, const char *dir, const char *attr);

#endif /* _POWER_EVENT_NE_H_ */
