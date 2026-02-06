/*
 * debug.h
 *
 * typec or pd debug info
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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "tcpci_config.h"

/* the switch of log message */
#define TYPEC_INFO_ENABLE     1
#define PE_EVENT_DBG_ENABLE   1
#define PE_STATE_INFO_ENABLE  1
#define TCPC_INFO_ENABLE      1
#define TCPC_TIMER_DBG_EN     0
#define PE_INFO_ENABLE        1
#define TCPC_DBG_ENABLE       0
#define TCPC_DBG2_ENABLE      0
#define DPM_INFO_ENABLE       1
#define DPM_INFO2_ENABLE      1
#define DPM_DBG_ENABLE        1
#define PD_ERR_ENABLE         1
#define PE_DBG_ENABLE         0
#define TYPEC_DBG_ENABLE      0

#define DP_INFO_ENABLE        1
#define DP_DBG_ENABLE         1

#define UVDM_INFO_ENABLE      1
#define TCPM_DBG_ENABLE       1

#ifdef CONFIG_USB_PD_ALT_MODE_RTDC
#define DC_INFO_ENABLE        1
#define DC_DBG_ENABLE         1
#endif /* CONFIG_USB_PD_ALT_MODE_RTDC */

#define PD_BUG_ON(x)    WARN_ON(x)
#define RT_DBG_INFO     pr_info
#define TCPC_DBG_PRESTR "TCPC-"

#if TCPC_TIMER_DBG_EN
#define TIMER_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TIMER:" format, ##args)
#else
#define TIMER_DBG(format, args...)
#endif /* TCPC_TIMER_DBG_EN */

#if TYPEC_DBG_ENABLE
#define TYPEC_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TYPEC:" format, ##args)
#else
#define TYPEC_DBG(format, args...)
#endif /* TYPEC_DBG_ENABLE */

#if TYPEC_INFO_ENABLE
#define TYPEC_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TYPEC:" format, ##args)
#else
#define TYPEC_INFO(format, args...)
#endif /* TYPEC_INFO_ENABLE */

#define TYPEC_ERR(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TYPEC-ERR:" format, ##args)

#if TCPC_INFO_ENABLE
#define TCPC_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TCPC:" format, ##args)
#else
#define TCPC_INFO(foramt, args...)
#endif /* TCPC_INFO_ENABLE */

#if TCPC_DBG_ENABLE
#define TCPC_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TCPC:" format, ##args)
#else
#define TCPC_DBG(format, args...)
#endif /* TCPC_DBG_ENABLE */

#if TCPC_DBG2_ENABLE
#define TCPC_DBG2(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TCPC:" format, ##args)
#else
#define TCPC_DBG2(format, args...)
#endif /* TCPC_DBG2_ENABLE */

#define TCPC_ERR(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TCPC-ERR:" format, ##args)

#define DP_ERR(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DP-ERR:" format, ##args)

#if DPM_INFO_ENABLE
#define DPM_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DPM:" format, ##args)
#else
#define DPM_INFO(format, args...)
#endif /* DPM_DBG_INFO */

#if DPM_INFO2_ENABLE
#define DPM_INFO2(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DPM:" format, ##args)
#else
#define DPM_INFO2(format, args...)
#endif /* DPM_DBG_INFO */

#if DPM_DBG_ENABLE
#define DPM_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DPM:" format, ##args)
#else
#define DPM_DBG(format, args...)
#endif /* DPM_DBG_ENABLE */

#if PD_ERR_ENABLE
#define PD_ERR(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "PD-ERR:" format, ##args)
#else
#define PD_ERR(format, args...)
#endif /* PD_ERR_ENABLE */

#if PE_INFO_ENABLE
#define PE_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "PE:" format, ##args)
#else
#define PE_INFO(format, args...)
#endif /* PE_INFO_ENABLE */

#if PE_EVENT_DBG_ENABLE
#define PE_EVT_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "PE-EVT:" format, ##args)
#else
#define PE_EVT_INFO(format, args...)
#endif /* PE_EVENT_DBG_ENABLE */

#if PE_DBG_ENABLE
#define PE_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "PE:" format, ##args)
#else
#define PE_DBG(format, args...)
#endif /* PE_DBG_ENABLE */

#if PE_STATE_INFO_ENABLE
#define PE_STATE_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "PE:" format, ##args)
#else
#define PE_STATE_INFO(format, args...)
#endif /* PE_STATE_IFNO_ENABLE */

#if DP_INFO_ENABLE
#define DP_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DP:" format, ##args)
#else
#define DP_INFO(format, args...)
#endif /* DP_INFO_ENABLE */

#if DP_DBG_ENABLE
#define DP_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DP:" format, ##args)
#else
#define DP_DBG(format, args...)
#endif /* DP_DBG_ENABLE */

#if UVDM_INFO_ENABLE
#define UVDM_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "UVDM:" format, ## args)
#else
#define UVDM_INFO(format, args...)
#endif /* UVDM_INFO_ENABLE */

#if TCPM_DBG_ENABLE
#define TCPM_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "TCPM:" format, ## args)
#else
#define TCPM_DBG(format, args...)
#endif /* TCPM_DBG_ENABLE */

#ifdef CONFIG_USB_PD_ALT_MODE_RTDC
#if DC_INFO_ENABLE
#define DC_INFO(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DC> " format, ## args)
#else
#define DC_INFO(format, args...)
#endif /* DC_INFO_ENABLE */

#if DC_DBG_ENABLE
#define DC_DBG(format, args...) \
	RT_DBG_INFO(TCPC_DBG_PRESTR "DC> " format, ## args)
#else
#define DC_DBG(format, args...)
#endif /* DC_DBG_ENABLE */
#endif /* CONFIG_USB_PD_ALT_MODE_RTDC */

#endif /* _DEBUG_H_ */
