/*
 * blpwm.h
 *
 * blpwm driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _BLPWM_H_
#define _BLPWM_H_

#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/mtd/hisi_nve_interface.h>

#include "protocol.h"

#define PINCTRL_STATE_IDLE             "idle"
#define PINCTRL_STATE_DEFAULT          "default"
#define DISABLE                        0
#define ENABLE                         1
/* AT */
#define IOCTL_BLPWM_PLAN1_CMD          _IO('Q', 0x01)
#define IOCTL_BLPWM_PLAN2_CMD          _IO('Q', 0x02)
#define IOCTL_BLPWM_PLAN3_CMD          _IO('Q', 0x03)
#define IOCTL_BLPWM_PLAN4_CMD          _IO('Q', 0x04)
#define IOCTL_BLPWM_RESET_CMD          _IO('Q', 0x05)
#define IOCTL_COVER_COLOR_CMD          _IO('Q', 0x0A)
#define IOCTL_COVER_LOOKUP_CMD         _IO('Q', 0x0E)
#define IOCTL_ADC_VOLT_CMD             _IO('Q', 0x0F)

#define AT_PLAN1_CMD                   1
#define AT_PLAN2_CMD                   2
#define AT_PLAN3_CMD                   3
#define AT_PLAN4_CMD                   4
#define AT_RESET_CMD                   5
/* B2 */
#define IOCTL_BLPWM_DUTY               _IO('Q', 0x06)
#define IOCTL_BLPWM_COLOR              _IO('Q', 0x07)
#define IOCTL_BLPWM_USEAGE             _IO('Q', 0x08)
#define IOCTL_BLPWM_USEAGE_STOP        _IO('Q', 0x09)
#define APP_PWM_CYCLE_MAX              100
#define APP_PWM_CYCLE_MIN              0
#define SENSOR_PWM_CYCLE_MAX           0
#define SENSOR_PWM_CYCLE_MIN           100
#define NV_NUMBER                      449
#define NV_VALID_SIZE                  15
#define DEBUG_BUFFER_SIZE              10
#define BLPWM_BOOT_DELAY               10000
#define DUTY_CHANNEL                   1
#define USAGE_CHANNEL                  2
#define USAGE_STOP_CHANNEL             3

/* COVER INFO */
#define SUPPORT_WITH_COLOR             3
#define ABNORMAL_WITH_COLOR            2
#define UNSUPPORT_WITH_NOCOLOR         1
#define SUPPORT_WITH_NOCOLOR           0
#define COVER_STATUS_DEBUG             2
#define COVER_INFO_ALREADY_READ        1
#define COVER_INFO_NOT_READ            0
/* IPC */
#define AP_CLOSE                       1
#define AP_OPEN                        2
#define BLPWM_SUPPORT_RSP              3

enum {
	COLOR_ID_MIN = 0,
	NOCOLOR = COLOR_ID_MIN,
	DARKBLUE,
	LIGHTBLUE,
	COLOR_ID_MAX = 5
};

struct blpwm_ops {
	int (*blpwm_cover_detect)(void);
	void (*blpwm_set_duty)(int duty);
	void (*blpwm_set_usage)(int usage);
	void (*blpwm_set_usage_stop)(int usage_stop);
	void (*blpwm_notiby_by_sns)(pkt_blpwm_sensor_req_t *blpwm_event);
};

struct blpwm_dev {
	struct blpwm_ops *p_ops;
	int color;
	int support;
	int duty_cycle;
	int usage;
	int usage_stop;
	int cover_status;
	struct delayed_work blpwm_boot_work;
};

#ifdef CONFIG_HUAWEI_BLPWM
int blpwm_register_ops(struct blpwm_ops *p_ops);
void blpwm_notify_by_sns(pkt_blpwm_sensor_req_t *blpwm_event);
void blpwm_open_sensorhub(void);
void blpwm_config_sensorhub(enum obj_sub_cmd sub_cmd);
void blpwm_close_sensorhub(void);
int blpwm_get_color(void);
int blpwm_get_support(void);
int blpwm_get_cover_status(void);
bool blpwm_is_support(void);
bool blpwm_is_valid_color(int color);
#else
static inline int blpwm_register_ops(struct blpwm_ops *p_ops)
{
	return -1;
}
static inline void blpwm_notify_by_sns(pkt_blpwm_sensor_req_t *blpwm_event)
{
	return;
}
static inline int blpwm_open_sensorhub(void)
{
	return -1;
}
static inline void blpwm_config_sensorhub(enum obj_sub_cmd sub_cmd)
{
	return;
}
static inline int blpwm_close_sensorhub(void)
{
	return -1;
}
static inline int blpwm_get_color(void)
{
	return -1;
}
static inline int blpwm_get_support(void)
{
	return -1;
}
static inline int blpwm_get_cover_status(void)
{
	return -1;
}
static inline bool blpwm_is_support(void)
{
	return false;
}
static inline bool blpwm_is_valid_color(int color)
{
	return false;
}
#endif /* HUAWEI_BLPWM */

#endif /* _BLPWM_H_ */

