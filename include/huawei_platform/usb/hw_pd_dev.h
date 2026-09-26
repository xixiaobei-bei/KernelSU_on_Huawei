/*
 * hw_pd_dev.h
 *
 * pd dpm header file.
 *
 * Copyright (c) 2016-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HW_PD_DEV_H_
#define _HW_PD_DEV_H_

#include <linux/device.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/completion.h>
#include <linux/hisi/usb/tca.h>
#include <linux/mfd/hisi_pmic_mntn.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pm_wakeup.h>
#include <linux/hisi/hisi_dss_dp.h>

/* indicate whether or not enable debug acc(double rp or double rd) */
#define CONFIG_DPM_USB_PD_CUSTOM_DBGACC

/* indicate whether or not enable debug acc sink(double rp) */
#define CONFIG_DPM_TYPEC_CAP_DBGACC_SNK

/* indicate whether or not enable custom src(double 56k) */
#define CONFIG_DPM_TYPEC_CAP_CUSTOM_SRC

/* used for combphy event process */
#define COMBPHY_MAX_PD_EVENT_COUNT              40
#define COMBPHY_PD_EVENT_INVALID_VAL            (-1)

/*
 * huawei's second-generation dock inserts digital headphones will repeatedly reset,
 * so we use svid to avoid this bug.
 */
#define PD_DPM_HW_DOCK_SVID                     0x12d1

/* used for cc abnormal change handler(cc interrupt storm), also used as water check */
#define PD_DPM_CC_CHANGE_COUNTER_THRESHOLD      50
#define PD_DPM_CC_CHANGE_INTERVAL               2000 /* ms */
#define PD_DPM_CC_CHANGE_MSEC                   1000
#define PD_DPM_CC_CHANGE_BUF_SIZE               10
#define PD_DPM_CC_DMD_COUNTER_THRESHOLD         1
#define PD_DPM_CC_DMD_INTERVAL                  (24 * 60 * 60) /* s */
#define PD_DPM_CC_DMD_BUF_SIZE                  1024
#define PD_DPM_INVALID_VAL                      (-1)

/* discover id ack:product vdo type offset, this is used for huawei wireless case(1/2) */
#define PD_DPM_PDT_OFFSET                       12
#define PD_DPM_PDT_MASK                         0x7
#define PD_DPM_PDT_VID_OFFSET                   16
#define PD_PID_COVER_ONE                        0x3000 /* use bcd value indicate huawei case1 */
#define PD_PID_COVER_TWO                        0x3001 /* use bcd value indicate huawei case2 */
#define PD_DPM_HW_VID                           0x12d1
#define PD_DPM_HW_CHARGER_PID                   0x3b30 /* use to indicate huawei charger pid */
#define PD_DPM_HW_PDO_MASK                      0xffff

/* cc status for big data collect */
#define PD_DPM_CC_OPEN                          0x00
#define PD_DPM_CC_DFT                           0x01
#define PD_DPM_CC_1_5                           0x02
#define PD_DPM_CC_3_0                           0x03
#define PD_DPM_CC2_STATUS_OFFSET                0x02
#define PD_DPM_CC_STATUS_MASK                   0x03
#define PD_DPM_BOTH_CC_STATUS_MASK              0x0f

/* used for cc orientation indicate cc none status */
#define CC_ORIENTATION_FACTORY_SET              1
#define CC_ORIENTATION_FACTORY_UNSET            0

/* used for pd reinit process */
#define LDO_NAME_SIZE                           16
#define PD_PDM_RE_ENABLE_DRP                    1

/* typec cc mode(UFP, DRP) */
enum pd_cc_mode {
	PD_DPM_CC_MODE_UFP,
	PD_DPM_CC_MODE_DRP,
};

/* used to indicate which event sent a completion message */
enum pd_wait_typec_complete {
	NOT_COMPLETE,
	COMPLETE_FROM_VBUS_DISCONNECT,
	COMPLETE_FROM_TYPEC_CHANGE,
};

enum pd_connect_result {
	PD_CONNECT_NONE,
	PD_CONNECT_TYPEC_ONLY,
	PD_CONNECT_TYPEC_ONLY_SNK_DFT,
	PD_CONNECT_TYPEC_ONLY_SNK,
	PD_CONNECT_TYPEC_ONLY_SRC,
	PD_CONNECT_PE_READY,
	PD_CONNECT_PE_READY_SNK,
	PD_CONNECT_PE_READY_SRC,

#ifdef CONFIG_DPM_USB_PD_CUSTOM_DBGACC
	PD_CONNECT_PE_READY_DBGACC_UFP,
	PD_CONNECT_PE_READY_DBGACC_DFP,
#endif /* CONFIG_USB_PD_CUSTOM_DBGACC */
};

/* used to policy engine report event */
enum pd_dpm_pe_event {
	PD_DPM_PE_EVT_DIS_VBUS_CTRL, /* disable vbus */
	PD_DPM_PE_EVT_SOURCE_VCONN, /* source vconn */
	PD_DPM_PE_EVT_SOURCE_VBUS, /* source vbus */
	PD_DPM_PE_EVT_SINK_VBUS, /* sink vbus */
	PD_DPM_PE_EVT_PR_SWAP, /* power role swap */
	PD_DPM_PE_EVT_DR_SWAP, /* data role swap */
	PD_DPM_PE_EVT_VCONN_SWAP, /* vconn swap */
	PD_DPM_PE_EVT_TYPEC_STATE, /* typec status */
	PD_DPM_PE_EVT_PD_STATE, /* pd status */
	PD_DPM_PE_ABNORMAL_CC_CHANGE_HANDLER, /* used to count cc change */
	PD_DPM_PE_CABLE_VDO, /* emark cable vdo info */
};

/* typec status */
enum pd_typec_attach_type {
	PD_DPM_TYPEC_UNATTACHED,
	PD_DPM_TYPEC_ATTACHED_SNK, /* single rp */
	PD_DPM_TYPEC_ATTACHED_SRC, /* single rd or ra + rd */
	PD_DPM_TYPEC_ATTACHED_AUDIO, /* double ra */
	PD_DPM_TYPEC_ATTACHED_DEBUG, /* double rd */

#ifdef CONFIG_DPM_TYPEC_CAP_DBGACC_SNK
	PD_DPM_TYPEC_ATTACHED_DBGACC_SNK, /* double rp */
#endif /* CONFIG_TYPEC_CAP_DBGACC_SNK */

#ifdef CONFIG_DPM_TYPEC_CAP_CUSTOM_SRC
	PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC, /* double 56k rp */
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC */

	PD_DPM_TYPEC_ATTACHED_VBUS_ONLY, /* cc open and vbus in */
	PD_DPM_TYPEC_UNATTACHED_VBUS_ONLY, /* cc open and vbus out */

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	PD_DPM_TYPEC_ATTACHED_CUSTOM_SRC2, /* double 22k or 10k rp */
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	PD_DPM_TYPEC_ATTACHED_NORP_SRC,
};

/* typec status */
enum pd_dpm_typec_status {
	PD_DPM_USB_TYPEC_NONE, /* typec status unknow */
	PD_DPM_USB_TYPEC_DETACHED, /* typec detached */
	PD_DPM_USB_TYPEC_DEVICE_ATTACHED, /* this device act as device */
	PD_DPM_USB_TYPEC_HOST_ATTACHED, /* this device act as host */
	PD_DPM_USB_TYPEC_AUDIO_ATTACHED, /* this device detect analog andio plug in */
	PD_DPM_USB_TYPEC_AUDIO_DETACHED, /* this device detect analog andio plug out */
};

/* uevent type */
enum pd_dpm_uevent_type {
	PD_DPM_UEVENT_START,
	PD_DPM_UEVENT_COMPLETE,
};

/* used to lock or unlock typec wakelock */
enum pd_dpm_wake_lock_type {
	PD_WAKE_LOCK = 100,
	PD_WAKE_UNLOCK,
};

/* used to decribe typec status reported from policy engine */
struct pd_dpm_typec_state {
	bool polarity; /* cc direction */
	int cc1_status; /* cc1 voltage status */
	int cc2_status; /* cc1 voltage status */
	int old_state; /* typec old status: described by pd_dpm_typec_status */
	int new_state; /* typec new status: described by pd_dpm_typec_status */
};

/* pd dpm interface */
struct pd_dpm_ops {
	void (*pd_dpm_detect_emark_cable)(void *client); /* start detect emark cable(c2c) */
	bool (*pd_dpm_get_hw_dock_svid_exist)(void *client); /* get if device has huawei svid */
	int (*pd_dpm_notify_direct_charge_status)(void *client, bool mode); /* get direct charge status(in or out) */
	void (*pd_dpm_set_cc_mode)(int mode); /* set cc mode(DFP, UFP) */
	int (*pd_dpm_get_cc_state)(void); /* get cc mode(DFP, UFP) */
	int (*pd_dpm_disable_pd)(void *client, bool disable); /* disable pd state machine */
	bool (*pd_dpm_check_cc_vbus_short)(void); /* check if cc vbus short */
	void (*pd_dpm_enable_drp)(int mode); /* enable typec drp */
	void (*pd_dpm_reinit_chip)(void *client); /* reinit typec chip config */
	int (*data_role_swap)(void *client); /* start data role swap */
};

struct pd_dpm_pd_state {
	uint8_t connected;
};

struct pd_dpm_swap_state {
	uint8_t new_role;
};

enum pd_dpm_vbus_type {
	PD_DPM_VBUS_TYPE_TYPEC = 20,
	PD_DPM_VBUS_TYPE_PD,
};

enum pd_dpm_cc_voltage_type {
	PD_DPM_CC_VOLT_OPEN = 0,
	PD_DPM_CC_VOLT_RA = 1,
	PD_DPM_CC_VOLT_RD = 2,

	PD_DPM_CC_VOLT_SNK_DFT = 5,
	PD_DPM_CC_VOLT_SNK_1_5 = 6,
	PD_DPM_CC_VOLT_SNK_3_0 = 7,

	PD_DPM_CC_DRP_TOGGLING = 15,
};

/* used to describe vbus status reported from policy engine */
struct pd_dpm_vbus_state {
	int mv; /* vbus value, unit mv */
	int ma; /* ibus value, unit ma */
	uint8_t vbus_type; /* vbus type: typec or pd */
	bool ext_power; /* remote device is self powered or external powered */
	int remote_rp_level; /* remote device cc rp level */
};

/* used to describe abnormal interrupt from pd(cc int or vbus int) */
enum abnomal_change_type {
	PD_DPM_ABNORMAL_CC_CHANGE,
	PD_DPM_UNATTACHED_VBUS_ONLY,
};

/* used to handle abnormal cc change and report dmd */
struct abnomal_change_info {
	enum abnomal_change_type event_id;
	bool first_enter; /* used to control if first cc change happend */
	int change_counter; /* used to count cc interrupt */
	int dmd_counter; /* used to count dmd report time */
	struct timespec64 ts64_last; /* last trigger time of cc interrupt  */
	struct timespec64 ts64_dmd_last; /* last trigger time of dmd report */
	int change_data[PD_DPM_CC_CHANGE_BUF_SIZE]; /* cc interrupt distributed data struct */
	int dmd_data[PD_DPM_CC_CHANGE_BUF_SIZE]; /* dmd data struct */
};

/* emark cable current capability */
enum cur_cap {
	PD_DPM_CURR_1P5A, /* 1.5A */
	PD_DPM_CURR_3A, /* 3A */
	PD_DPM_CURR_5A, /* 5A */
	PD_DPM_CURR_6A, /* 6A */
};

#define CABLE_CUR_CAP_SHIFT        5 /* cable current shift */
#define CABLE_EXT_CUR_CAP_SHIFT    13 /* cable current shift */
/* cable current MAST(bit5 and bit6 is defined in pd spec) */
#define CABLE_CUR_CAP_MASK         (BIT(5) | BIT(6))
/* addtitional curret MASK(bit13 bit14 and bit15 are defined in product vdo) */
#define CABLE_EXT_CUR_CAP_MASK     (BIT(13) | BIT(14) | BIT(15))

/* base on huawei uvdm protocol */
enum pd_product_type {
	PD_PDT_PD_ADAPTOR, /* adaptor */
	PD_PDT_PD_POWER_BANK, /* power bank */
	PD_PDT_WIRELESS_CHARGER, /* wireless charger */
	PD_PDT_WIRELESS_COVER, /* wireless case generation one */
	PD_PDT_WIRELESS_COVER_TWO, /* wireless case generation two */
	PD_PDT_TOTAL,
};

/* context of combphy, used to describe the event to control combphy, usb phy and dp phy */
struct pd_dpm_combphy_event {
	TCA_IRQ_TYPE_E irq_type;
	TCPC_MUX_CTRL_TYPE mode_type;
	TCA_DEV_TYPE_E dev_type;
	TYPEC_PLUG_ORIEN_E typec_orien;
};

struct pd_dpm_cable_info {
	unsigned int cable_vdo;
	unsigned int cable_vdo_ext;
};

struct pd_dpm_info {
	struct device *dev;
	struct mutex sink_vbus_lock;
	struct regulator *pd_vdd_ldo;
	struct notifier_block ocp_nb;
	struct dual_role_phy_instance *dual_role;
	struct dual_role_phy_desc *desc;
	enum hisi_charger_type charger_type;
	struct notifier_block usb_nb;
	struct notifier_block chrg_wake_unlock_nb;
	struct blocking_notifier_head pd_evt_nh;
	struct blocking_notifier_head pd_port_status_nh;
	struct atomic_notifier_head pd_wake_unlock_evt_nh;
	enum pd_dpm_uevent_type uevent_type;
	struct work_struct pd_work;
	const char *tcpc_name;

	/* usb state update */
	struct mutex usb_lock;
	int pending_usb_event;
	int last_usb_event;

	struct workqueue_struct *usb_wq;
	struct delayed_work usb_state_update_work;
	struct delayed_work cc_moisture_flag_restore_work;
	struct delayed_work reinit_pd_work;
	struct delayed_work otg_restore_work;

	struct pd_dpm_combphy_event last_combphy_notify_event;
	struct mutex pd_combphy_notify_lock;
	struct workqueue_struct *pd_combphy_wq;
	struct delayed_work pd_combphy_event_work;

	int vconn_en;
	int max_ocp_count;
	int ocp_delay_time;
	bool is_ocp;
	int pd_reinit_enable;
	bool bc12_finish_flag;
	bool pd_finish_flag;
	bool pd_source_vbus;
	unsigned long bc12_event;
	struct pd_dpm_vbus_state bc12_sink_vbus_state;
	int cur_usb_event;
	struct pd_dpm_cable_info cable_vdo;
	bool ctc_cable_flag;
	struct wakeup_source vdd_ocp_lock;
	const char *ldo_name;
	struct work_struct fb_work;
};

struct cc_check_ops {
	int (*is_cable_for_direct_charge)(void);
};

int cc_check_ops_register(struct cc_check_ops *ops);

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
struct cable_vdo_ops {
	int (*is_cust_src2_cable)(void);
};
int pd_dpm_cable_vdo_ops_register(struct cable_vdo_ops *ops);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */

/* for chip layer to get class created by core layer */
struct class *hw_pd_get_class(void);
struct tcpc_device *tcpc_dev_get_by_name(const char *name);
int register_pd_dpm_notifier(struct notifier_block *nb);
int unregister_pd_dpm_notifier(struct notifier_block *nb);
int register_pd_wake_unlock_notifier(struct notifier_block *nb);
int unregister_pd_wake_unlock_notifier(struct notifier_block *nb);
int register_pd_dpm_portstatus_notifier(struct notifier_block *nb);
int unregister_pd_dpm_portstatus_notifier(struct notifier_block *nb);
int pd_dpm_handle_pe_event(unsigned long event, void *data);
bool pd_dpm_get_pd_finish_flag(void);
bool pd_dpm_get_pd_source_vbus(void);
void pd_dpm_get_typec_state(int *typec_detach);
int pd_dpm_get_analog_hs_state(void);
void pd_dpm_get_charge_event(unsigned long *event, struct pd_dpm_vbus_state *state);
bool pd_dpm_get_high_power_charging_status(void);
bool pd_dpm_get_high_voltage_charging_status(void);
bool pd_dpm_get_optional_max_power_status(void);
void pd_dpm_set_max_power(int max_power);
bool pd_dpm_get_cc_orientation(void);
int pd_dpm_get_cc_state_type(unsigned int *cc1, unsigned int *cc2);

#ifdef CONFIG_CONTEXTHUB_PD
bool pd_dpm_get_last_hpd_status(void);
void pd_dpm_set_last_hpd_status(bool hpd_status);
void pd_dpm_set_combphy_status(TCPC_MUX_CTRL_TYPE mode);
TCPC_MUX_CTRL_TYPE pd_dpm_get_combphy_status(void);
int pd_dpm_handle_combphy_event(struct pd_dpm_combphy_event event);
int pd_dpm_get_pd_event_num(void);
#else
static inline bool pd_dpm_get_last_hpd_status(void)
{
	return false;
}

static inline void pd_dpm_set_last_hpd_status(bool hpd_status)
{
}

static inline void pd_dpm_set_combphy_status(TCPC_MUX_CTRL_TYPE mode)
{
}

static inline TCPC_MUX_CTRL_TYPE pd_dpm_get_combphy_status(void)
{
	return TCPC_NC;
}

static inline int pd_dpm_handle_combphy_event(struct pd_dpm_combphy_event event)
{
	return 0;
}

static inline int pd_dpm_get_pd_event_num(void)
{
	return 0;
}
#endif /* CONFIG_CONTEXTHUB_PD */

void pd_dpm_set_cc_voltage(int type);
enum pd_dpm_cc_voltage_type pd_dpm_get_cc_voltage(void);
int pd_dpm_ops_register(struct pd_dpm_ops *ops, void *client);
int pd_dpm_disable_pd(bool disable);
void pd_dpm_set_pd_finish_flag(bool flag);
bool pd_dpm_get_hw_dock_svid_exist(void);

#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
void pd_dpm_wakelock_ctrl(unsigned long event); /* PD_WAKE_LOCK,PD_WAKE_UNLOCK */
void pd_dpm_vbus_ctrl(unsigned long event); /* CHARGER_TYPE_NONE,PLEASE_PROVIDE_POWER */
void pd_dpm_set_audio_power_no_hs_state(bool flag);
#else
static inline void pd_dpm_wakelock_ctrl(unsigned long event) {};
static inline void pd_dpm_vbus_ctrl(unsigned long event) {};
static inline void pd_dpm_set_audio_power_no_hs_state(bool flag) {};
#endif /* CONFIG_TCPC_CLASS */

void pd_dpm_ignore_vbus_only_event(bool flag);
bool pd_dpm_ignore_vbus_event(void);
void pd_dpm_set_ignore_vbus_event(bool flag);
bool pd_dpm_ignore_bc12_event_when_vbusoff(void);
bool pd_dpm_ignore_bc12_event_when_vbuson(void);
void pd_dpm_set_ignore_vbus_event_when_vbusoff(bool _ignore_vbus_event);
void pd_dpm_set_ignore_bc12_event_when_vbuson(bool _ignore_bc12_event);
bool pd_dpm_ignore_vbuson_event(void);
bool pd_dpm_ignore_vbusoff_event(void);
void pd_dpm_set_ignore_vbuson_event(bool ignore);
void pd_dpm_set_ignore_vbusoff_event(bool ignore);
void pd_pdm_enable_drp(void);

void pd_dpm_start_data_role_swap(void);
int pd_dpm_get_cur_usb_event(void);
void pd_dpm_audioacc_sink_vbus(unsigned long event, void *data);
int pmic_vbus_irq_is_enabled(void);
void pogopin_set_pmic_vbus_irq_enable(int enable);
enum charger_event_type pd_dpm_get_source_sink_state(void);
int pd_dpm_notify_direct_charge_status(bool dc);
void dp_dfp_u_notify_dp_configuration_done(TCPC_MUX_CTRL_TYPE mode_type, int ack);
void pd_set_product_type(int type);
int pd_get_product_type(void);
void pd_set_product_id_info(unsigned int vid, unsigned int pid, unsigned int type);
bool pd_dpm_check_cc_vbus_short(void);
bool pd_dpm_get_cc_moisture_status(void);
int pd_dpm_get_otg_channel(void);
enum cur_cap pd_dpm_get_cvdo_cur_cap(void);
void pd_dpm_set_emark_detect_enable(int flag);
int pd_dpm_get_emark_detect_enable(void);
void pd_dpm_detect_emark_cable(void);
void pd_dpm_detect_emark_cable_finish(void);
bool pd_dpm_get_ctc_cable_flag(void);
void pd_dpm_cc_dynamic_protect(void);
void pd_dpm_set_source_sink_state(enum charger_event_type type);
bool pmic_vbus_is_connected(void);
void pmic_vbus_disconnect_process(void);
void pd_dpm_usb_host_on(void);
void pd_dpm_usb_host_off(void);
#if defined(CONFIG_TCPC_CLASS) || defined(CONFIG_HW_TCPC_CLASS)
bool pd_judge_is_cover(void);
#else
static inline bool pd_judge_is_cover(void)
{
	return false;
}
#endif /* CONFIG_TCPC_CLASS */

#endif /* _HW_PD_DEV_H_ */
