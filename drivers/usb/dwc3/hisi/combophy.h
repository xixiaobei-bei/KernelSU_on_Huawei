/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: combophy.h for combophy driver
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

#ifndef __COMBOPHY_H__
#define __COMBOPHY_H__


/* API form PD */
void dp_dfp_u_notify_dp_configuration_done(enum tcpc_mux_ctrl_type mode_type, int ack);

/* inner API form chip usb */
int chip_usb_otg_event_sync(enum tcpc_mux_ctrl_type mode_type,
	enum otg_dev_event_type event, enum typec_plug_orien_e typec_orien);
int dwc3_set_combophy_clk(void);
void dwc3_close_combophy_clk(void);

/* API provide */
#if defined(CONFIG_CONTEXTHUB_PD) && !defined(CONFIG_CHIP_COMBOPHY)
int combophy_poweroff(void);
/* only for usb hifi switch to usb3 */
int combophy_sw_sysc(enum tcpc_mux_ctrl_type new_mode,
	enum typec_plug_orien_e typec_orien, bool lock_flag);
int combophy_is_highspeed_only(void);
int combophy_is_usb_only(void);
void combophy_suspend_process(void);
void combophy_resume_process(void);
#else
static inline int combophy_poweroff(void)
{
	return 0;
}
static inline int combophy_sw_sysc(enum tcpc_mux_ctrl_type new_mode,
	enum typec_plug_orien_e typec_orien, bool lock_flag)
{
	return 0;
}
static inline int combophy_is_highspeed_only(void)
{
	return 0;
}

static inline int combophy_is_usb_only(void)
{
	return 0;
}

static inline void combophy_suspend_process(void)
{
}
static inline void combophy_resume_process(void)
{
}
#endif

#endif /* __COMBOPHY_H__ */
