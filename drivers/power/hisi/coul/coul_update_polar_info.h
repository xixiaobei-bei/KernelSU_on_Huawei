/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: polar operation interface for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_UPDATE_POLAR_INFO_H_
#define _COUL_UPDATE_POLAR_INFO_H_

#include "coul_core.h"
#include "coul_polar.h"

#define IPC_BAT_OCVINFO IPC_CMD(OBJ_LPM3, OBJ_AP, CMD_NOTIFY, TYPE_BAT)

#ifdef CONFIG_COUL_POLAR
int polar_ops_register(struct polar_device_ops *ops);
void update_polar_ishort_info_cc(int cc_temp);
void update_polar_ishort_info_time(unsigned int time_now);
void update_polar_info_chgdone(int status);
int get_coul_polar_avg(void);
int get_coul_polar_peak(void);
void get_polar_sysfs_info(struct polar_sysfs_info *info);
void clear_ishort_first_para(void);
void update_polar_ocv(struct info_to_update_polar_info *coul_info);
void polar_ipc_init(struct hisi_polar_device *di);
void resume_polar_calc(int temp, int ocv_soc_mv, int curr_now, int vol_now);
void update_polar_params(int ocv_soc_mv, int curr_now,
	int vol_now, bool update_flag);
bool could_sample_polar_ocv(int time_now, int charging_stop_time,
	int batt_temp, unsigned int polar_ocv_enable);
struct polar_device_ops *get_polar_dev_ops(void);
#else
static inline int polar_ops_register(struct polar_device_ops *ops)
{
	return 0;
}

static inline void update_polar_ishort_info_cc(int cc_temp)
{
}

static inline void update_polar_ishort_info_time(unsigned int time_now)
{
}

static inline void update_polar_info_chgdone(int status)
{
}

static inline void get_polar_sysfs_info(struct polar_sysfs_info *info)
{
}

static inline int get_coul_polar_avg(void)
{
	return 0;
}

static inline int get_coul_polar_peak(void)
{
	return 0;
}

static inline void update_polar_ocv(struct info_to_update_polar_info *coul_info)
{
}

static inline void resume_polar_calc(int temp, int ocv_soc_mv, int curr_now, int vol_now)
{
}

static inline void update_polar_params(int ocv_soc_mv,
	int curr_now, int vol_now, bool update_flag)
{
}

static inline void clear_ishort_first_para(void)
{
}

static inline bool could_sample_polar_ocv(int time_now, int charging_stop_time,
	int batt_temp, unsigned int polar_ocv_enable)
{
	return FALSE;
}

static inline void polar_ipc_init(struct hisi_polar_device *di)
{
}

#endif
#endif
