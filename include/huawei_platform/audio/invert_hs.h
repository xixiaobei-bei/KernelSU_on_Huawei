/*
 * invert_hs.h
 *
 * invert headset driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef INVERT_HS_H
#define INVERT_HS_H

/* invert headset mic gnd connect status */
enum mic_gnd_connect_status {
	INVERT_HS_MIC_GND_DISCONNECT        = 0,
	INVERT_HS_MIC_GND_CONNECT           = 1,
};

#ifdef CONFIG_INVERT_HS
int invert_hs_control(int connect);
#else
static inline int invert_hs_control(int connect)
{
	return 0;
}
#endif

#endif // INVERT_HS_H
