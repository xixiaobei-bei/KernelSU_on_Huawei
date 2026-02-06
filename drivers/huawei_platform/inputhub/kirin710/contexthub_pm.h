/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Sensor Hub Channel Bridge
 */

#ifndef __LINUX_SENSORHUB_PM_H__
#define __LINUX_SENSORHUB_PM_H__

#define RESUME_INIT 0
#define RESUME_MINI 1
#define RESUME_SKIP 2
#define SENSOR_VBUS "sensor-io"

struct ipc_debug {
	int event_cnt[TAG_END];
	int pack_error_cnt;
};

typedef enum {
	SUB_POWER_ON,
	SUB_POWER_OFF
} sub_power_status;

int sensorhub_io_driver_init(void);
void set_pm_notifier(void);
void enable_sensors_when_resume(void);
void disable_sensors_when_suspend(void);
int tell_ap_status_to_mcu(int ap_st);
void update_current_app_status(uint8_t tag, uint8_t cmd);
sys_status_t get_iom3_sr_status(void);
int get_iomcu_power_state(void);

extern struct ipc_debug ipc_debug_info;
extern struct completion iom3_resume_mini;
extern struct completion iom3_resume_all;
extern uint32_t need_reset_io_power;
#endif /* __LINUX_SENSORHUB_PM_H__ */
