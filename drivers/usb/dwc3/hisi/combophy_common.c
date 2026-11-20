/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ComboPHY Common Module on platform
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/hisi/usb/tca.h>

const char *irq_type_string(enum tca_irq_type_e irq_type)
{
	static const char * const irq_type_strings[] = {
		[TCA_IRQ_HPD_OUT] = "TCA_IRQ_HPD_OUT",
		[TCA_IRQ_HPD_IN] = "TCA_IRQ_HPD_IN",
		[TCA_IRQ_SHORT] = "TCA_IRQ_SHORT",
	};

	if (irq_type > TCA_IRQ_SHORT)
		return "illegal irq_type";

	return irq_type_strings[irq_type];
}

const char *mode_type_string(enum tcpc_mux_ctrl_type mode_type)
{
	static const char * const mode_type_strings[] = {
		[TCPC_NC] = "TCPC_NC",
		[TCPC_USB31_CONNECTED] = "TCPC_USB31_CONNECTED",
		[TCPC_DP] = "TCPC_DP",
		[TCPC_USB31_AND_DP_2LINE] = "TCPC_USB31_AND_DP_2LINE",
	};

	if (mode_type > TCPC_USB31_AND_DP_2LINE)
		return "illegal mode_type";

	return mode_type_strings[mode_type];
}

const char *dev_type_string(enum tca_device_type_e dev_type)
{
	static const char * const dev_type_strings[] = {
		[TCA_CHARGER_CONNECT_EVENT] = "TCA_CHARGER_CONNECT_EVENT",
		[TCA_CHARGER_DISCONNECT_EVENT] = "TCA_CHARGER_DISCONNECT_EVENT",
		[TCA_ID_FALL_EVENT] = "TCA_ID_FALL_EVENT",
		[TCA_ID_RISE_EVENT] = "TCA_ID_RISE_EVENT",
		[TCA_DP_OUT] = "TCA_DP_OUT",
		[TCA_DP_IN] = "TCA_DP_IN",
	};

	if (dev_type > TCA_DP_IN)
		return "illegal dev_type";

	return dev_type_strings[dev_type];
}
