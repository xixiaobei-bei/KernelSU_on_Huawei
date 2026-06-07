/*
 * drivers/inputhub/contexthub_pm.h
 *
 * sensors sysfs header
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
#ifndef __LINUX_SENSORHUB_PM_H__
#define __LINUX_SENSORHUB_PM_H__
#include "contexthub_debug.h"
#define RESUME_INIT 0
#define RESUME_MINI 1
#define RESUME_SKIP 2
#define SENSOR_VBUS "sensor-io"
#define SENSOR_VBUS_LDO12 "lsensor-io"

struct ipc_debug {
	int event_cnt[TAG_END];
	int pack_error_cnt;
};

typedef enum {
	SUB_POWER_ON,
	SUB_POWER_OFF
} sub_power_status;

extern int sensorhub_io_driver_init(void);
extern void set_pm_notifier(void);
extern void enable_sensors_when_resume(void);
extern void disable_sensors_when_suspend(void);
extern int tell_ap_status_to_mcu(int ap_st);
extern void update_current_app_status(uint8_t tag, uint8_t cmd);
extern unsigned long sensor_jiffies;
extern struct CONFIG_ON_DDR *pConfigOnDDr;
extern int resume_skip_flag;
extern struct completion iom3_reboot;
extern iomcu_power_status i_power_status;
extern char sensor_chip_info[SENSOR_MAX][MAX_CHIP_INFO_LEN];
extern int sensor_power_pmic_flag;
extern int sensor_power_init_finish;
extern int sensorhub_reboot_reason_flag;
extern int hifi_supported;
extern struct regulator *sensorhub_vddio;


#endif /* __LINUX_SENSORHUB_PM_H__ */
