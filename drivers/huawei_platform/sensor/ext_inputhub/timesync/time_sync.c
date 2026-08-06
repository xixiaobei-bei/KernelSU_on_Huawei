/*
 * RTC subsystem, sync RTC time and timezone to MCU on startup
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/rtc.h>
#include <linux/time.h>
#include <securec.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/pvclock_gtod.h>
#include <linux/suspend.h>

#include "ext_sensorhub_api.h"
#include "time_sync.h"

#define SERVICE_ID 0x01
#define CMD_ID_SET_TIMER 0x96
#define CMD_ID_GET_TIMER 0x9D
#define SET_TIMER_TAG 0x03
#define SET_TIMER_TYPE 0x01
#define SET_TIMER_LEN 0x04
#define SET_TIMEZONE_TYPE 0x02
#define SET_TIMEZONE_LEN  0x02
#define TIMEZONE_DEFAULT 0xff
#define HOUR_TO_MINUTES 60
#define TLV_DEFAULT_SIZE 128
#define BYTE_LEN 8
#define RTC_STNC_TIME 3600
#define TIMEZONE_HOUR_INDEX 2
#define TIMEZONE_MINUTES_INDEX 3
#define RTC_YEAR_LINUX_INIT 1900
#define RTC_TIME_YEAR 2000
#define RTC_MONTH_INIT 1
#define TID 0x20
#define GET_TIMER_LEN 12
#define COMMAND_COUNT 1
#define TIME_GET_LEN 4
#define GET_TIME_LEN 1
#define RSUME_SEND_INTERVAL 5400
#define MCU_SEND_INTERVAL 5

struct time_tlv {
	unsigned char cmd_type;
	unsigned char time_type;
	unsigned char time_len;
	unsigned char time_val[4]; /* utc time */
	unsigned char timezone_type;
	unsigned char timezone_len;
	unsigned char timezone_val[2]; /* timeZone */
};

static struct timer_list rtc_timer;
static struct workqueue_struct *time_sync_wq;
static struct work_struct time_sync_work;
static int timezone_last;
static unsigned long last_resume_send;
static unsigned long last_mcu_send;
static int g_time_sync_init = 0;
static struct mutex sync_lock;

static void show_time(struct rtc_time *time)
{
	pr_err("[%s]: %d-%d-%d %d:%d:%d\n", __func__,
		time->tm_year + RTC_YEAR_LINUX_INIT,
		time->tm_mon + RTC_MONTH_INIT,
		time->tm_mday, time->tm_hour,
		time->tm_min, time->tm_sec);
}

static inline unsigned long get_uptime(void)
{
	struct timespec uptime;

	get_monotonic_boottime(&uptime);
	return uptime.tv_sec;
}

static int get_rtc_time(void)
{
	int ret;
	unsigned char buff[TIME_GET_LEN] = {0x01, 0x00, 0x02, 0x00};
	struct command cmd;

	cmd.service_id = SERVICE_ID;
	cmd.command_id = CMD_ID_GET_TIMER;
	cmd.send_buffer = &buff;
	cmd.send_buffer_len = sizeof(buff);

	ret = send_command(TIMESYNC_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		pr_err("[%s]getting time send failed\n", __func__);

	return ret;
}

static int parse_timezone(unsigned char *timezone_value, int timezone_len)
{
	int ret;
	int data = 0;
	int timezone;
	int hour;
	int minutes;
	int timezone_hl;
	unsigned char value_tmp[SET_TIMER_LEN];

	timezone = -sys_tz.tz_minuteswest;
	if (timezone == timezone_last) {
		pr_err("[%s]timezone %d not change, without sync", __func__, timezone);
		return -1;
	}

	timezone_last = timezone;
	minutes = abs(timezone % HOUR_TO_MINUTES);
	hour = timezone / HOUR_TO_MINUTES;
	if (hour < 0) /* west of Greenwich */
		hour = ((-hour) + TLV_DEFAULT_SIZE) << BYTE_LEN;
	else /* east of Greenwich */
		hour = hour << BYTE_LEN;

	timezone_hl = ntohl(hour + minutes);
	ret = memcpy_s(value_tmp, SET_TIMER_LEN, &timezone_hl, SET_TIMER_LEN);
	if (ret) {
		pr_err("[%s]:memcpy timezone failed!\n", __func__);
		return -1;
	}
	/* high 8 bit for hour */
	timezone_value[data++] = value_tmp[TIMEZONE_HOUR_INDEX];
	/* low 8 bit for minutes */
	timezone_value[data] = value_tmp[TIMEZONE_MINUTES_INDEX];

	return ret;
}

int send_time_to_mcu(unsigned long time)
{
	int ret;
	struct command cmd;
	unsigned int time_hl;

	struct time_tlv dm_set_time = {
		.cmd_type = SET_TIMER_TAG,
		.time_type = SET_TIMER_TYPE,
		.time_len = SET_TIMER_LEN,
		.timezone_type = SET_TIMEZONE_TYPE,
		.timezone_len = SET_TIMEZONE_LEN,
	};
	mutex_lock(&sync_lock);

	time_hl = ntohl(time);
	ret = memcpy_s(dm_set_time.time_val, SET_TIMER_LEN, &time_hl, SET_TIMER_LEN);
	if (ret) {
		pr_err("[%s]:cpy time failed!\n", __func__);
		mutex_unlock(&sync_lock);
		return -1;
	}

	ret = parse_timezone(dm_set_time.timezone_val, SET_TIMEZONE_LEN);
	if (ret == -1) {
		dm_set_time.timezone_val[0] = TIMEZONE_DEFAULT;
		dm_set_time.timezone_val[1] = TIMEZONE_DEFAULT;
	}
	pr_info("[%s]time zone is %02x, %02x\n", __func__,
		dm_set_time.timezone_val[0], dm_set_time.timezone_val[1]);

	cmd.service_id = SERVICE_ID;
	cmd.command_id = CMD_ID_SET_TIMER;
	cmd.send_buffer = (unsigned char *)&dm_set_time;
	cmd.send_buffer_len = sizeof(struct time_tlv);

	ret = send_command(TIMESYNC_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		pr_err("[%s]setting time send failed\n", __func__);

	last_mcu_send = get_uptime();
	mutex_unlock(&sync_lock);

	return ret;
}

static int rtc_hctosys_work(void)
{
	int err = -ENODEV;
	struct rtc_time tm;
	struct rtc_device *rtc = NULL;
	unsigned long time;

	rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	if (!rtc) {
		pr_err("[%s]open rtc device fail\n", __func__);
		goto err_open;
	}

	/* read Gregorian date from hardware clock */
	err = rtc_read_time(rtc, &tm);
	if (err) {
		pr_err("[%s]read the hardware clock fail\n", __func__);
		goto err_read;
	}
	show_time(&tm);

	/* Convert Gregorian date to seconds since 01-01-1970 00:00:00 */
	rtc_tm_to_time(&tm, &time);

	err = rtc_set_time(rtc, &tm);
	if (err)
		pr_err("[%s]set RTC time Fail\n", __func__);

err_read:
	rtc_class_close(rtc);

err_open:
	return err;
}

static void time_sync_work_callback(struct work_struct *work)
{
	int ret;
	unsigned long expires;

	ret = get_rtc_time(); /* time up, get time from MCU */
	if (ret < 0)
		pr_err("[%s]time up, unable to sync time\n", __func__);

	expires = jiffies + (RTC_STNC_TIME * HZ);
	mod_timer(&rtc_timer, expires);
}

static void rtc_sync_timer_func(unsigned long data)
{
	queue_work(time_sync_wq, &time_sync_work);
}

/* if the year less than 2000 and not the first sync
 * read the AP soc time and send it to mcu
 * otherwise, use mcu rtc time as default soc rtc
 */
static int sync_time(struct rtc_time *set_time)
{
	int err = -ENODEV;
	struct rtc_device *rtc = NULL;
	struct timespec64 tv64 = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};

	int year = set_time->tm_year + RTC_YEAR_LINUX_INIT;
	tv64.tv_sec = rtc_tm_to_time64(set_time);

	if (year <= RTC_TIME_YEAR && g_time_sync_init) {
		err = rtc_hctosys_work();
		if (err < 0)
			pr_err("sync the soc rtc fail in %s\n", __func__);

		g_time_sync_init = 1;
		return err;
	}

	mutex_lock(&sync_lock);
	unsigned long now_uptime = get_uptime();
	if ((now_uptime - last_mcu_send <= MCU_SEND_INTERVAL) && g_time_sync_init) {
		pr_err("[%s]:mcu send time interval less than 5!\n", __func__);
		mutex_unlock(&sync_lock);
		return 0;
	}
	mutex_unlock(&sync_lock);
	err = do_settimeofday64(&tv64);
	rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	if (rtc) {
		err = rtc_set_time(rtc, set_time);
		if (err)
			pr_err("[%s]: set RTC time Fail\n", __func__);
		rtc_class_close(rtc);
	}

	g_time_sync_init = 1;

	return err;
}

static int get_time_from_buffer(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len)
{
	int ret;
	struct rtc_time set_time = {0};
	unsigned long time_info = 0;

	if (!data) {
		pr_err("[%s] data is null\n", __func__);
		return -EINVAL;
	}

	if ((service_id != SERVICE_ID) || (command_id != CMD_ID_GET_TIMER)) {
		pr_err("[%s] service_id or command_id err\n", __func__);
		return -EINVAL;
	}

	if (data_len == GET_TIMER_LEN) {
		ret = memcpy_s(&time_info, TIME_GET_LEN, data + TIME_GET_LEN, TIME_GET_LEN);
		if (ret) {
			pr_err("[%s]:cpy time failed!\n", __func__);
			return -EINVAL;
		}
		rtc_time_to_tm(time_info, &set_time);
		show_time(&set_time);
		sync_time(&set_time);
	} else {
		pr_info("[%s] datalen: %u is error\n", __func__, data_len);
		return -EINVAL;
	}
	pr_info("service id is:[%d],command id is:[%d]\n", service_id, command_id);

	return 0;
}

static int get_time_from_channel(void)
{
	int ret;
	struct subscribe_cmds *sub_cmd = (struct subscribe_cmds *)
		kmalloc(sizeof(struct subscribe_cmds), GFP_KERNEL);

	if (!sub_cmd) {
		pr_err("[%s]sub cmd is null\n", __func__);
		return -EINVAL;
	}
	sub_cmd->cmd_cnt = COMMAND_COUNT;
	sub_cmd->cmds = kmalloc(sizeof(struct sid_cid), GFP_KERNEL);
	if (!sub_cmd->cmds) {
		kfree(sub_cmd);
		pr_err("[%s]sub_cmd->cmds is null\n", __func__);
		return -EINVAL;
	}
	sub_cmd->cmds->service_id = SERVICE_ID;
	sub_cmd->cmds->command_id = CMD_ID_GET_TIMER;

	ret = register_data_callback(TIMESYNC_CHANNEL, sub_cmd, get_time_from_buffer);
	if (ret < 0)
		pr_err("[%s]register data callback failed\n", __func__);

	kfree(sub_cmd->cmds);
	kfree(sub_cmd);

	return ret;
}

int handshake_callback(enum ext_sensorhub_event event, unsigned char tid,
	unsigned char *data, int data_len)
{
	int ret;

	if (event != COMMU_HANDSHAKE)
		return -EINVAL;

	/* register and monitor */
	ret = get_time_from_channel();
	if (ret < 0)
		pr_err("[%s]sync time failed\n", __func__);

	/* request data from the mcu */
	if (get_rtc_time() < 0) {
			pr_err("%s get init rtc from mcu error\n", __func__);
	} else {
			pr_err("%s get init rtc from mcu success\n", __func__);
	}
}

static int get_time_from_mcu(void)
{
	int ret;
	unsigned char buff[GET_TIME_LEN] = { 0x04 };
	struct command cmd;

	cmd.service_id = SERVICE_ID;
	cmd.command_id = CMD_ID_SET_TIMER;
	cmd.send_buffer = &buff;
	cmd.send_buffer_len = sizeof(buff);

	ret = send_command(TIMESYNC_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		pr_err("[%s]ap send message to mcu failed\n", __func__);

	return ret;
}

static void send_time_resume(void)
{
	unsigned long now_uptime = get_uptime();

	pr_info("[%s] try at resume send time to mcu", __func__);
	if (now_uptime - last_resume_send >= RSUME_SEND_INTERVAL) {
		pr_info("[%s] at resume send time to mcu, now up|last up:%ld|%ld\n",
			__func__, now_uptime, last_resume_send);
		last_resume_send = now_uptime;
		get_rtc_time();
		if (get_time_from_mcu() < 0)
			pr_err("%s ap send to mcu failed\n", __func__);
	}
}

static int time_sr_notify(struct notifier_block *notify_block,
			  unsigned long mode, void *unused)
{
	pr_info("%s get in", __func__);
	switch (mode) {
	case PM_POST_SUSPEND:
		pr_info("%s get in PM_POST_SUSPEND, resume", __func__);
		send_time_resume();
		break;
	default:
	    break;
	}
	return 0;
}

static struct notifier_block g_time_sr_notifier = {
	.notifier_call = time_sr_notify,
	.priority = INT_MIN,
};

static int __init time_sync_init(void)
{
	int ret;

	mutex_init(&sync_lock);
	time_sync_wq = create_singlethread_workqueue("time_sync");

	/* sync time and timezont on commu handshake */
	ret = register_event_callback(COMMU_HANDSHAKE, TID, handshake_callback);
	if (ret < 0) {
		pr_err("[%s]unable to sync time on startup\n", __func__);
	}

	register_pm_notifier(&g_time_sr_notifier);
	pr_info("%s get in", __func__);
	last_resume_send = 0;
}

static void __exit time_sync_exit(void)
{
	unregister_pm_notifier(&g_time_sr_notifier);
	unregister_event_callback(COMMU_HANDSHAKE, TID, handshake_callback);
	if (time_sync_wq != NULL) {
		destroy_workqueue(time_sync_wq);
		time_sync_wq = NULL;
	}
}

late_initcall(time_sync_init);
module_exit(time_sync_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei rtc-sync driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
