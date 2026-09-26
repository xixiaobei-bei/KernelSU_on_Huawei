/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
 * Description: tca interface for usb, tca.h
 * Author: Hilisicon
 * Create: 2017-06-23
 */

#ifndef __LINUX_USB31_TCA_H__
#define __LINUX_USB31_TCA_H__

#include <linux/types.h>

typedef enum tca_irq_type_e {
	TCA_IRQ_HPD_OUT = 0,
	TCA_IRQ_HPD_IN = 1,
	TCA_IRQ_SHORT = 2,
	TCA_IRQ_MAX_NUM
}TCA_IRQ_TYPE_E;

typedef enum tca_device_type_e {
	TCA_CHARGER_CONNECT_EVENT = 0, /* usb device in */
	TCA_CHARGER_DISCONNECT_EVENT, /* usb device out */
	TCA_ID_FALL_EVENT, /* usb host in */
	TCA_ID_RISE_EVENT, /* usb host out */
	TCA_DP_OUT,
	TCA_DP_IN,
	TCA_DEV_MAX
}TCA_DEV_TYPE_E;

typedef enum tcpc_mux_ctrl_type {
	TCPC_NC = 0,
	TCPC_USB31_CONNECTED = 1,
	TCPC_DP = 2,
	TCPC_USB31_AND_DP_2LINE = 3,
	TCPC_MUX_MODE_MAX
}TCPC_MUX_CTRL_TYPE;

typedef enum typec_plug_orien_e {
	TYPEC_ORIEN_POSITIVE = 0,
	TYPEC_ORIEN_NEGATIVE = 1,
	TYPEC_ORIEN_MAX
}TYPEC_PLUG_ORIEN_E;

#ifdef CONFIG_CONTEXTHUB_PD
extern int  pd_event_notify(enum tca_irq_type_e irq_type, enum tcpc_mux_ctrl_type mode_type, enum tca_device_type_e dev_type, enum typec_plug_orien_e typec_orien);
#else
static inline int  pd_event_notify(enum tca_irq_type_e irq_type, enum tcpc_mux_ctrl_type mode_type,
		enum tca_device_type_e dev_type, enum typec_plug_orien_e typec_orien)
{
	return 0;
}
#endif

#endif /* __LINUX_USB31_TCA_H__ */
