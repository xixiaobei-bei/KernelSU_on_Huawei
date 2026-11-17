/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: digital headset low power V600 driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#ifndef __DIG_HS_POWER_V600_H__
#define __DIG_HS_POWER_V600_H__

#include <linux/list.h>

#define IOCTL_DIG_HS_POWER_BUCKBOOST_NO_HEADSET_CMD     _IO('Q', 0x01)
#define IOCTL_DIG_HS_POWER_SCHARGER_CMD                 _IO('Q', 0x02)
#ifdef CONFIG_DIG_HS_POWER_V600
int dig_hs_v600_low_power(void);
int dig_hs_v600_normal_power(void);
#else
static inline int dig_hs_v600_low_power(void)
{
	return 0;
}
static inline int dig_hs_v600_normal_power(void)
{
	return 0;
}
#endif
#endif // __DIG_HS_POWER_V600_H__
