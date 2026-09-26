/*
 * usbaudio_setinterface.h
 *
 * usbaudio setinterface.
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#ifndef __USBAUDIO_SETINTERFACE__
#define __USBAUDIO_SETINTERFACE__

int usbaudio_ctrl_set_pipeout_interface(unsigned int running, unsigned int rate);
int usbaudio_ctrl_set_pipein_interface(unsigned int running,
	unsigned int rate, unsigned int period);
void usbaudio_ctrl_nv_check(void);

#endif /* __USBAUDIO_SETINTERFACE__ */
