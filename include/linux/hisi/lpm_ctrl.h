 /*
 * lpm_ctrl.h
 *
 * head file for low power control
 *
 * Copyright (c) 2011-2021 Huawei Technologies Co., Ltd.
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

#ifndef __H_LPM_CTRL__
#define __H_LPM_CTRL__

#define SUSPEND_NOT_SWITCH_32K          0
#define SUSPEND_SWITCH_32K              1
#define NOT_SUPPORT_SWITCH             -1

#ifdef CONFIG_SR_SWITCH_32K
extern int lowpm_get_32k_switch(void);
#else
static int lowpm_get_32k_switch(void)
{
	return NOT_SUPPORT_SWITCH;
}
#endif

#endif /* __H_LPM_CTRL__ */

