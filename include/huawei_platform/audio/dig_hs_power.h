/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset low power V300 driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#ifndef __DIG_HS_POWER_H__
#define __DIG_HS_POWER_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#define IOCTL_DIG_HS_POWER_BUCKBOOST_NO_HEADSET_CMD     _IO('Q', 0x01)
#define IOCTL_DIG_HS_POWER_SCHARGER_CMD                 _IO('Q', 0x02)

enum dig_hs_vboost_control_type {
	DIG_HS_VBOOST_CONTROL_PM = 0,
	DIG_HS_VBOOST_CONTROL_AUDIO,
};
enum dig_hs_voice_para {
	DIG_HS_POWER_GPIO_RESET = 0,
	DIG_HS_POWER_GPIO_SET,
};

#ifdef CONFIG_DIG_HS_POWER
int dig_hs_bst_ctrl_enable(bool enable, enum dig_hs_vboost_control_type type);
int dig_hs_low_power(void);
int dig_hs_normal_power(void);
int dig_hs_core_plug_in(void);
int dig_hs_core_plug_out(void);
#else
static inline int dig_hs_bst_ctrl_enable(bool enable,
	enum dig_hs_vboost_control_type type)
{
	return 0;
}

static inline int dig_hs_low_power(void)
{
	return 0;
}

static inline int dig_hs_normal_power(void)
{
	return 0;
}

static inline int dig_hs_core_plug_in(void)
{
	return 0;
}

static inline int dig_hs_core_plug_out(void)
{
	return 0;
}

#endif

#endif // __DIT_HS_POWER_H__
