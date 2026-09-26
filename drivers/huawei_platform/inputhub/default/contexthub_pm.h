/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub pm header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __LINUX_SENSORHUB_PM_H__
#define __LINUX_SENSORHUB_PM_H__

#include "contexthub_debug.h"

#define RESUME_INIT 0
#define RESUME_MINI 1
#define RESUME_SKIP 2
#define SENSOR_VBUS "sensor-io"
#define SENSOR_VBUS_LDO34 "psensor-io"
#define SENSOR_VBUS_LDO12 "lsensor-io"

struct ipc_debug {
	int event_cnt[TAG_END];
	int pack_error_cnt;
};

typedef enum {
	SUB_POWER_ON,
	SUB_POWER_OFF
} sub_power_status;

struct ipc_debug *get_ipc_debug_info(void);
unsigned long get_sensor_jiffies(void);
uint32_t get_no_need_sensor_ldo24(void);
int get_key_state(void);
int sensorhub_io_driver_init(void);
void set_pm_notifier(void);
void enable_sensors_when_resume(void);
void disable_sensors_when_suspend(void);
int tell_ap_status_to_mcu(int ap_st);
void update_current_app_status(uint8_t tag, uint8_t cmd);
sys_status_t get_iom3_sr_status(void);
int get_iomcu_power_state(void);

extern struct config_on_ddr *g_config_on_ddr;
extern struct completion iom3_reboot;
extern struct regulator *sensorhub_vddio;

#endif /* __LINUX_SENSORHUB_PM_H__ */
