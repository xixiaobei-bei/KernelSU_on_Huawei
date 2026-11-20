/*
 * tcpc_core.h
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

#ifndef _TCPC_CORE_H_
#define _TCPC_CORE_H_

#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include "tcpm.h"
#include "tcpci.h"
#include "tcpci_config.h"
#include "tcpm_dual_role.h"
#include "tcpm_notifier.h"
#include "debug.h"

#ifdef CONFIG_HW_USB_POWER_DELIVERY
#include "pd_core.h"
#endif

#define TCPC_MAX_NAME_LEN                32

struct typec_lpm;
struct typec_fsm;
struct timer_device;

struct tcpc_desc {
	uint8_t role_def;
	uint8_t rp_lvl;
	uint8_t vconn_supply;
	char name[TCPC_MAX_NAME_LEN];
};

struct tcpc_device {
	struct tcpc_ops *ops;
	void *drv_data;
	struct tcpc_desc desc;
	struct device dev;

	/* for tcpc timer & event */
	struct timer_device *timer;

	struct mutex access_lock;
	struct mutex typec_lock;

	atomic_t pending_event;
	wait_queue_head_t event_loop_wait_que;
	struct task_struct *event_task;
	bool event_loop_thead_stop;

	struct delayed_work init_work;
	struct delayed_work event_init_work;
	struct tcpm_notifier notifier;
	struct typec_lpm *lpm;

	/* For TCPC TypeC */
	struct typec_fsm *fsm;
	uint8_t typec_role;
	uint8_t typec_local_cc;
	uint8_t typec_local_rp_level;
	uint8_t typec_remote_cc[2];
	uint8_t typec_remote_rp_level;
	bool typec_polarity;
	bool typec_power_ctrl;

	uint8_t typec_legacy_cable;
	uint8_t typec_legacy_cable_suspect;
#ifdef CONFIG_TYPEC_CHECK_LEGACY_CABLE2
	uint8_t typec_legacy_retry_wk;
#endif /* CONFIG_TYPEC_CHECK_LEGACY_CABLE2 */

	bool typec_ext_discharge;

#ifdef CONFIG_TCPC_VCONN_SUPPLY_MODE
	uint8_t tcpc_vconn_supply;
#endif /* CONFIG_TCPC_VCONN_SUPPLY_MODE */

#ifdef CONFIG_TCPC_SOURCE_VCONN
	bool tcpc_source_vconn;
#endif /* CONFIG_TCPC_SOURCE_VCONN */

	uint32_t tcpc_flags;
	struct tcpm_dual_role dr_dev;

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	/* Event */
	uint8_t pd_event_count;
	uint8_t pd_event_head_index;
	uint8_t pd_msg_buffer_allocated;

	uint8_t pd_last_vdm_msg_id;
	bool pd_pending_vdm_event;
	bool pd_pending_vdm_reset;
	bool pd_pending_vdm_good_crc;
	bool pd_pending_vdm_discard;
	bool pd_pending_vdm_attention;
	bool pd_postpone_vdm_timeout;

	struct pd_msg pd_last_vdm_msg;
	struct pd_msg pd_attention_vdm_msg;
	struct pd_event pd_vdm_event;

	struct pd_msg pd_msg_buffer[PD_MSG_BUF_SIZE];
	struct pd_event pd_event_ring_buffer[PD_EVENT_BUF_SIZE];

	bool pd_pe_running;
	bool pd_hard_reset_event_pending;
	bool pd_wait_hard_reset_complete;
	bool pd_wait_pr_swap_complete;
	bool pd_ping_event_pending;
	uint8_t pd_bist_mode;
	uint8_t pd_transmit_state;
	uint8_t pd_wait_vbus_once;

#ifdef CONFIG_USB_PD_DIRECT_CHARGE
	bool pd_during_direct_charge;
	/* Huawei SCP flag */
	bool is_huawei_scp_chg;
#endif /* CONFIG_USB_PD_DIRECT_CHARGE */

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	bool pd_discard_pending;
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */

#ifdef CONFIG_HW_USB_PD_REV30
	uint8_t pd_retry_count;
#endif /* CONFIG_HW_USB_PD_REV30 */

	struct pd_port pd_port;
#ifdef CONFIG_HW_USB_PD_REV30
	struct notifier_block bat_nb;
	struct delayed_work bat_update_work;
	struct power_supply *bat_psy;
	uint8_t charging_status;
	int bat_soc;
#endif /* CONFIG_HW_USB_PD_REV30 */
#ifdef CONFIG_PD_WAIT_BC12
	uint8_t wait_bc12_cnt;
#endif /* CONFIG_PD_WAIT_BC12 */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */
	u8 vbus_level : 2;
	u8 irq_enabled : 1;
	u8 pd_inited_flag : 1;

	bool huawei_dock_svid_exist;
	bool vbus_cc1_short;
	bool vbus_cc2_short;
};

struct device_node *tcpc_get_dev_node(struct tcpc_device *tcpc, const char *name);
void typec_handle_timeout(struct tcpc_device *tcpc, uint32_t timer_id);
void typec_disable_wakeup_timer(struct tcpc_device *tcpc);
void tcpc_awake_attach_lock(struct tcpc_device *tcpc);
void tcpc_notfiy_pe_idle(struct tcpc_device *tcpc);
bool tcpc_is_wait_pe_idle(struct tcpc_device *tcpc);
void tcpc_set_sink_curr(struct tcpc_device *tcpc, int curr);
void tcpc_typec_error_recovery(struct tcpc_device *tcpc);
void tcpc_typec_disable(struct tcpc_device *tcpc);
void tcpc_typec_enable(struct tcpc_device *tcpc);
void tcpc_handle_pe_pr_swap(struct tcpc_device *tcpc);
int tcpc_swap_typec_role(struct tcpc_device *tcpc);
int tcpc_set_rp_level(struct tcpc_device *tcpc, uint8_t res);
uint8_t tcpc_get_attach_type(struct tcpc_device *tcpc);

#endif /* _TCPC_CORE_H_ */
