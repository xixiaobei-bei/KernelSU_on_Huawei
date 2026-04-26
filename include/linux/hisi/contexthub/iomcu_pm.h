/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_pm.h.
 * Author: Huawei
 * Create: 2019/11/05
 */
#ifndef __IOMCU_PM_H
#define __IOMCU_PM_H
#include <linux/hisi/contexthub/protocol_as.h>

struct ipc_debug {
	int event_cnt[TAG_END];
	int pack_error_cnt;
};

enum sub_power_status {
	SUB_POWER_ON,
	SUB_POWER_OFF
};

struct iomcu_power_status {
	uint8_t power_status;
	uint8_t app_status[TAG_END];
	uint32_t idle_time;
	uint64_t active_app_during_suspend;
};

extern int g_iom3_power_state;

extern struct iomcu_power_status g_i_power_status;

extern struct mutex g_mutex_pstatus;

extern int resume_skip_flag;

#define RESUME_INIT 0
#define RESUME_MINI 1
#define RESUME_SKIP 2

#define SENSOR_VBUS "sensor-io"
#define SENSOR_VBUS_LDO12 "lsensor-io"

#define SENSOR_POWER_DO_RESET                           (0)
#define SENSOR_POWER_NO_RESET                           (1)

#define SENSOR_REBOOT_REASON_MAX_LEN 32

/*
 * Function    : sensorhub_io_driver_init
 * Description : driver init for pm
 * Input       : none
 * Output      : none
 * Return      : none
 */
int sensorhub_io_driver_init(void);

/*
 * Function    : sensor_power_check
 * Description : when boot, call this to check sensor power
 * Input       : none
 * Output      : none
 * Return      : 0 is ok, otherwise error, always return succ currently
 */
int sensor_power_check(void);

/*
 * Function    : update_current_app_status
 * Description : ipc interface will call it, update app power status
 * Input       : [tag] app tag
 *             : [cmd] different cmd will record different power status
 * Output      : none
 * Return      : none
 */
void update_current_app_status(u8 tag, u8 cmd);

/*
 * Function    : tell_ap_status_to_mcu
 * Description : when suspend or resume, tell ap status to mcu
 * Input       : [ap_st] ap status
 * Output      : none
 * Return      : none
 */
int tell_ap_status_to_mcu(int ap_st);

/*
* Description : get current iom3 sr status
* Output      : none
* Return      : none
*/
sys_status_t get_iom3_sr_status(void);

/*
* Description : enter contexthub s4 status
* Output      : none
* Return      : 0 is ok, otherwise error
*/
int sensorhub_pm_s4_entry(void);

void inputhub_pm_callback(struct pkt_header *head);

extern struct ipc_debug g_ipc_debug_info;
extern struct completion g_iom3_reboot;
extern struct completion g_iom3_resume_mini;
extern struct completion g_iom3_resume_all;

#endif /* __IOMCU_PM_H */
