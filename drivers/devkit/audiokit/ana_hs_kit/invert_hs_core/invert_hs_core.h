/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: invert headset driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#ifndef INVERT_HS_CORE_H
#define INVERT_HS_CORE_H

#include "../ana_hs.h"

/* invert headset mic gnd connect status */
enum mic_gnd_connect_status {
	INVERT_HS_MIC_GND_DISCONNECT        = 0,
	INVERT_HS_MIC_GND_CONNECT           = 1,
};

#ifdef CONFIG_INVERT_HS_CORE
void invert_hs_core_control(int connect);
#else
static inline void invert_hs_core_control(int connect)
{
	return 0;
}
#endif

#endif // INVERT_HS_CORE_H
