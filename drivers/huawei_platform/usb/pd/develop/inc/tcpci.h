/*
 * typci.h
 *
 * tcpc interface
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

#ifndef _TCPCI_H_
#define _TCPCI_H_

#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/pm_wakeup.h>
#include <linux/err.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include "tcpci_config.h"

#define PE_STATE_FULL_NAME                 0
#define TCPC_LOW_RP_DUTY                   100
#define TCPC_NORMAL_RP_DUTY                330

/* TCPC Power Register Define */
#define TCPC_REG_POWER_STATUS_VBUS_PRES    (1 << 2)

/* TCPC Alert Register Define */
#define TCPC_REG_ALERT_EXT_RA_DETACH       (1 << 21)
#define TCPC_REG_ALERT_EXT_WATCHDOG        (1 << 18)
#define TCPC_REG_ALERT_EXT_VBUS_80         (1 << 17)
#define TCPC_REG_ALERT_EXT_WAKEUP          (1 << 16)

#define TCPC_REG_ALERT_VBUS_DISCNCT        (1 << 11)
#define TCPC_REG_ALERT_RX_BUF_OVF          (1 << 10)
#define TCPC_REG_ALERT_FAULT               (1 << 9)
#define TCPC_REG_ALERT_V_ALARM_LO          (1 << 8)
#define TCPC_REG_ALERT_V_ALARM_HI          (1 << 7)
#define TCPC_REG_ALERT_TX_SUCCESS          (1 << 6)
#define TCPC_REG_ALERT_TX_DISCARDED        (1 << 5)
#define TCPC_REG_ALERT_TX_FAILED           (1 << 4)
#define TCPC_REG_ALERT_RX_HARD_RST         (1 << 3)
#define TCPC_REG_ALERT_RX_STATUS           (1 << 2)
#define TCPC_REG_ALERT_POWER_STATUS        (1 << 1)
#define TCPC_REG_ALERT_CC_STATUS           (1 << 0)

#define TCPC_REG_ALERT_RX_MASK \
	(TCPC_REG_ALERT_RX_STATUS | TCPC_REG_ALERT_RX_BUF_OVF)
#define TCPC_REG_ALERT_HRESET_SUCCESS \
	(TCPC_REG_ALERT_TX_SUCCESS | TCPC_REG_ALERT_TX_FAILED)
#define TCPC_REG_ALERT_TX_MASK (TCPC_REG_ALERT_TX_SUCCESS | \
	TCPC_REG_ALERT_TX_FAILED | TCPC_REG_ALERT_TX_DISCARDED)
#define TCPC_REG_ALERT_TXRX_MASK \
	(TCPC_REG_ALERT_TX_MASK | TCPC_REG_ALERT_RX_MASK)

/* TCPC Behavior Flags */
#define TCPC_FLAGS_RETRY_CRC_DISCARD       (1 << 0)
#define TCPC_FLAGS_WAIT_HRESET_COMPLETE    (1 << 1)
#define TCPC_FLAGS_CHECK_CC_STABLE         (1 << 2)
#define TCPC_FLAGS_LPM_WAKEUP_WATCHDOG     (1 << 3)
#define TCPC_FLAGS_CHECK_RA_DETACHE        (1 << 4)
#define TCPC_FLAGS_PREFER_LEGACY2          (1 << 5)
#define TCPC_FLAGS_DISABLE_LEGACY          (1 << 6)
#define TCPC_FLAGS_PD_REV30                (1 << 7)
#define TCPC_FLAGS_WATCHDOG_EN             (1 << 8)
#define TCPC_FLAGS_WATER_DETECTION         (1 << 9)
#define TCPC_FLAGS_CABLE_TYPE_DETECTION    (1 << 10)
#define TCPC_FLAGS_VBUS_CC_SHORT_DETECTION (1 << 15)
#define TCPC_FLAGS_FOREIGN_OBJ_DETECTION   (1 << 11)
#define TCPC_FLAGS_TYPEC_OTP               (1 << 12)
#define TCPC_FLAGS_FLOATING_GROUND         (1 << 13)
#define TCPC_FLAGS_VSAFE0V_DETECT          (1 << 14)

enum tcpc_cc_pull {
	TYPEC_CC_RA = 0,
	TYPEC_CC_RP = 1,
	TYPEC_CC_RD = 2,
	TYPEC_CC_OPEN = 3,
	TYPEC_CC_DRP = 4, /* from Rd */

	TYPEC_CC_RP_DFT = 1,
	TYPEC_CC_RP_1_5 = 9,
	TYPEC_CC_RP_3_0 = 17,

	TYPEC_CC_DRP_DFT = 4,
	TYPEC_CC_DRP_1_5 = 12,
	TYPEC_CC_DRP_3_0 = 20,
	TYPEC_CC_INVALID = 100,
};

#define typec_cc_pull_get_res(pull)    ((pull) & 0x07)
#define typec_cc_pull_get_rp_lvl(pull) (((pull) & 0x18) >> 3)

enum tcpc_pd_rx_cap_type {
	TCPC_RX_CAP_SOP = 1 << 0,
	TCPC_RX_CAP_SOP_PRIME = 1 << 1,
	TCPC_RX_CAP_SOP_PRIME_PRIME = 1 << 2,
	TCPC_RX_CAP_SOP_DEBUG_PRIME = 1 << 3,
	TCPC_RX_CAP_SOP_DEBUG_PRIME_PRIME = 1 << 4,
	TCPC_RX_CAP_HARD_RESET = 1 << 5,
	TCPC_RX_CAP_CABLE_RESET = 1 << 6,
};

enum tcpc_pd_transmit_type {
	TCPC_TX_SOP = 0,
	TCPC_TX_SOP_PRIME = 1,
	TCPC_TX_SOP_PRIME_PRIME = 2,
	TCPC_TX_SOP_DEBUG_PRIME = 3,
	TCPC_TX_SOP_DEBUG_PRIME_PRIME = 4,
	TCPC_TX_HARD_RESET = 5,
	TCPC_TX_CABLE_RESET = 6,
	TCPC_TX_BIST_MODE_2 = 7
};

struct tcpc_device;

struct tcpc_ops {
	int (*init)(void *chip, bool sw_reset);
	int (*init_alert_mask)(void *chip);
	int (*alert_status_clear)(void *chip, uint32_t mask);
	int (*fault_status_clear)(void *chip, uint8_t status);
	int (*get_alert_mask)(void *chip, uint32_t *mask);
	int (*set_alert_mask)(void *chip, uint32_t mask);
	int (*get_alert_status)(void *chip, uint32_t *alert);
	int (*get_power_status)(void *chip, uint16_t *pwr_status);
	int (*get_fault_status)(void *chip, uint8_t *status);
	int (*get_cc)(void *chip, int *cc1, int *cc2);
	int (*set_cc)(void *chip, int pull);
	int (*set_polarity)(void *chip, int polarity, uint8_t remote_cc);
	int (*set_low_rp_duty)(void *chip, bool low_rp);
	int (*set_vconn)(void *chip, int enable);
	int (*deinit)(void *chip);
	int (*alert_vendor_defined_handler)(void *chip);
	bool (*is_vsafe0v)(void *chip);
	int (*set_vbus_cc_short_detection)(void *chip, bool cc1, bool cc2);
	int (*is_low_power_mode)(void *chip);
	int (*set_low_power_mode)(void *chip, bool en, int pull);

	int (*set_watchdog)(void *chip, bool en);
	int (*set_intrst)(void *chip, bool en);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	int (*set_msg_header)(void *chip, uint8_t power_role, uint8_t data_role);
	int (*set_rx_enable)(void *chip, uint8_t enable);
	int (*get_message)(void *chip, uint32_t *payload, uint16_t *head,
		enum tcpc_pd_transmit_type *type);
	int (*protocol_reset)(void *chip);
	int (*transmit)(void *chip, enum tcpc_pd_transmit_type type,
		uint16_t header, const uint32_t *data, uint8_t retrys);
	int (*set_bist_test_mode)(void *chip, bool en);
	int (*set_bist_carrier_mode)(void *chip, uint8_t pattern);

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	int (*retransmit)(void *chip, uint8_t retrys);
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	int (*get_cc_status)(void);
};

struct tcpc_device *tcpc_device_register(struct device *parent,
	const char *desc_key, struct tcpc_ops *ops, void *drv_data);
void tcpc_device_unregister(struct device *dev, struct tcpc_device *tcpc);
void tcpci_set_flags(struct tcpc_device *tcpc, uint32_t flags);
bool tcpci_is_support(struct tcpc_device *tcpc, uint32_t mask);
bool tcpci_is_pr_swaping(struct tcpc_device *tcpc);
void tcpci_handle_vbus_cc_short(struct tcpc_device *tcpc_dev, bool vbus_cc1_short,
	bool vbus_cc2_short);

int tcpci_alert(struct tcpc_device *tcpc);

int tcpci_alert_status_clear(struct tcpc_device *tcpc, uint32_t mask);
int tcpci_fault_status_clear(struct tcpc_device *tcpc, uint8_t status);
int tcpci_get_alert_mask(struct tcpc_device *tcpc, uint32_t *mask);
int tcpci_set_alert_mask(struct tcpc_device *tcpc, uint32_t mask);
int tcpci_get_alert_status(struct tcpc_device *tcpc, uint32_t *alert);
int tcpci_get_fault_status(struct tcpc_device *tcpc, uint8_t *fault);
int tcpci_get_power_status(struct tcpc_device *tcpc, uint16_t *pw_status);
int tcpci_init(struct tcpc_device *tcpc, bool sw_reset);
int tcpci_init_alert_mask(struct tcpc_device *tcpc);

void tcpci_set_cc_mode(int mode);
int tcpci_get_cc_mode(void);
int tcpci_get_cc_status(void);

void set_tcpc_dev(struct tcpc_device *tcpc_dev);
int tcpci_get_cc(struct tcpc_device *tcpc);
int tcpci_read_cc(struct tcpc_device *tcpc, uint8_t *cc1, uint8_t *cc2);
int tcpci_set_cc(struct tcpc_device *tcpc, int pull);
int tcpci_set_polarity(struct tcpc_device *tcpc, int polarity);
int tcpci_set_low_rp_duty(struct tcpc_device *tcpc, bool low_rp);
int tcpci_set_vconn(struct tcpc_device *tcpc, int enable);

int tcpci_is_low_power_mode(struct tcpc_device *tcpc);
int tcpci_set_low_power_mode(struct tcpc_device *tcpc, bool en, int pull);
int tcpci_set_watchdog(struct tcpc_device *tcpc, bool en);
int tcpci_alert_vendor_defined_handler(struct tcpc_device *tcpc);
bool tcpci_is_vsafe0v(struct tcpc_device *tcpc);
int tcpci_set_vbus_cc_short_detection(struct tcpc_device *tcpc, bool cc1,
	bool cc2);

#ifdef CONFIG_HW_USB_POWER_DELIVERY
int tcpci_set_msg_header(struct tcpc_device *tcpc, uint8_t power_role,
	uint8_t data_role);
int tcpci_set_rx_enable(struct tcpc_device *tcpc, uint8_t enable);
int tcpci_protocol_reset(struct tcpc_device *tcpc);
int tcpci_get_message(struct tcpc_device *tcpc, uint32_t *payload,
	uint16_t *head, enum tcpc_pd_transmit_type *type);
int tcpci_transmit(struct tcpc_device *tcpc, enum tcpc_pd_transmit_type type,
	uint16_t header, const uint32_t *data);
int tcpci_set_bist_test_mode(struct tcpc_device *tcpc, bool en);
int tcpci_set_bist_carrier_mode(struct tcpc_device *tcpc, uint8_t pattern);

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
int tcpci_retransmit(struct tcpc_device *tcpc);
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

int tcpci_set_intrst(struct tcpc_device *tcpc, bool en);
int tcpci_enable_watchdog(struct tcpc_device *tcpc, bool en);
int tcpci_enable_ext_discharge(struct tcpc_device *tcpc, bool en);

#endif /* _TCPCI_H_ */