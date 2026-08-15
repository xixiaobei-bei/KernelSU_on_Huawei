/*
 * hung_wp_screen.c
 *
 * Reporting the frozen screen alarm event when the screen is abnormal
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifdef CONFIG_HW_ZEROHUNG
#include <linux/version.h>
/* no sched/clock.h before kernel4.14 in which sched_show_task is declared */
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/clock.h>
#include <linux/sched/debug.h>
#endif
#include "chipset_common/hwzrhung/zrhung.h"
#include "chipset_common/hwzrhung/hung_wp_screen.h"
#include <log/log_usertype.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/input.h>
#include <linux/jiffies.h>

#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <mntn_public_interface.h>
#endif

#ifdef CONFIG_KEYBOARD_HISI_GPIO_KEY
#include <linux/hisi/hisi_powerkey_event.h>
#endif

#ifndef CONFIG_KEYBOARD_HISI_GPIO_KEY
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#define OF_GPIO_ACTIVE_LOW 0x1
#define PON_KPDPWR 0
#define PON_RESIN 1
#define PON_KEYS 2
#endif

#define TIME_CONVERT_UNIT 1000
#define DEFAULT_TIMEOUT 10
#define CONFIG_LEN_MAX 64
#define COMAND_LEN_MAX 256
#define CMD_BUF_FMT_BETA "T=WindowManager,T=PowerManagerService," \
			 "n=system_server,n=surfaceflinger," \
			 "n=android.hardware.graphics.composer@2.1-service," \
			 "n=android.hardware.graphics.composer@2.2-service," \
			 "d=%d,e=%llu"
#define CMD_BUF_FMT "T=WindowManager,T=PowerManagerService,n=system_server,d=%d,e=%llu"
#define BUFFER_TIME_END 1
#define BUFFER_TIME_START (BUFFER_TIME_END + 10)
#define NANOS_PER_MILLISECOND 1000000
#define NANOS_PER_SECOND (NANOS_PER_MILLISECOND * TIME_CONVERT_UNIT)

#define POWERKEYEVENT_MAX_COUNT 10
#define POWERKEYEVENT_DEFAULT_COUNT 3
#define POWERKEYEVENT_MAX_TIMEWINDOW 20
#define POWERKEYEVENT_DEFAULT_TIMEWINDOW 5
#define POWERKEYEVENT_MAX_LIMIT_MS 2000
#define POWERKEYEVENT_DEFAULT_LIMIT_MS 300
#define POWERKEYEVENT_MAX_REPORT_MIN 3600
#define POWERKEYEVENT_DEFAULT_REPORT_MIN 2
#define POWERKEYEVENT_TIME_LEN (POWERKEYEVENT_MAX_COUNT + 2)

struct hung_wp_screen_config {
	int enable;
	uint32_t timeout;
};
struct zrhung_powerkeyevent_config {
	int enable;
	unsigned int timewindow;
	unsigned int count;
	unsigned int ignore_limit;
	unsigned int report_limit;
};

struct hung_wp_screen_data {
	struct hung_wp_screen_config *config;
	struct timer_list timer;
#ifndef CONFIG_KEYBOARD_HISI_GPIO_KEY
	struct timer_list long_press_timer;
#endif
	struct workqueue_struct *workq;
	struct work_struct config_work;
	struct work_struct send_work;
	spinlock_t lock;
	int bl_level;
	int check_id;
	int tag_id;
};

static bool config_done;
static bool init_done;
static struct hung_wp_screen_config on_config;
static struct hung_wp_screen_config off_config;
static struct hung_wp_screen_data data;
static unsigned int vkeys_pressed;

static bool prkyevt_config_done;
static struct zrhung_powerkeyevent_config event_config = {0};

static unsigned int lastreport_time;
static unsigned int lastprkyevt_time;
static unsigned int powerkeyevent_time[POWERKEYEVENT_TIME_LEN] = {0};

static unsigned int newevt;
static unsigned int headevt;
struct work_struct powerkeyevent_config;
struct work_struct powerkeyevent_sendwork;

static struct notifier_block hung_wp_screen_powerkey_nb;

/* Get configuration from zerohung_config.xml */
static int zrhung_powerkeyevent_get_config(struct zrhung_powerkeyevent_config *cfg)
{
	int ret;
	char config_str[CONFIG_LEN_MAX] = {0};

	ret = zrhung_get_config(ZRHUNG_EVENT_POWERKEY, config_str, sizeof(config_str));
	if (ret != 0) {
		pr_err("%s: powerkeyevent read config fail: %d\n",
		       __func__, ret);
		cfg->enable = 0;
		cfg->timewindow = POWERKEYEVENT_DEFAULT_TIMEWINDOW;
		cfg->count = POWERKEYEVENT_DEFAULT_COUNT;
		cfg->ignore_limit = POWERKEYEVENT_DEFAULT_LIMIT_MS;
		cfg->report_limit = POWERKEYEVENT_DEFAULT_REPORT_MIN;
		return ret;
	}
	/* 5: the number of successful values returned */
	if (sscanf(config_str, "%d,%d,%d,%d,%d", &cfg->enable, &cfg->timewindow,
		   &cfg->count, &cfg->ignore_limit, &cfg->report_limit) != 5)
		pr_err("%s: get args from config_str not 5\n", __func__);

	pr_err("%s: ZRHUNG_EVENT_POWERKEY %d, %d, %d, %d, %d\n",
	       __func__, cfg->enable, cfg->timewindow, cfg->count,
	       cfg->ignore_limit, cfg->report_limit);

	if ((cfg->timewindow <= 0) || (cfg->timewindow > POWERKEYEVENT_MAX_TIMEWINDOW))
		cfg->timewindow = POWERKEYEVENT_DEFAULT_TIMEWINDOW;

	if ((cfg->count <= 0) || (cfg->count > POWERKEYEVENT_MAX_COUNT))
		cfg->count = POWERKEYEVENT_DEFAULT_COUNT;

	if ((cfg->ignore_limit <= 0) || (cfg->ignore_limit > POWERKEYEVENT_MAX_LIMIT_MS))
		cfg->ignore_limit = POWERKEYEVENT_DEFAULT_LIMIT_MS;

	if ((cfg->report_limit <= 0) || (cfg->report_limit > POWERKEYEVENT_MAX_REPORT_MIN))
		cfg->report_limit = POWERKEYEVENT_DEFAULT_REPORT_MIN;

	return ret;
}

/* Get configuration from zerohung_config.xml */
static void zrhung_powerkeyevent_config_work(struct work_struct *work)
{
	int ret;

	ret = zrhung_powerkeyevent_get_config(&event_config);
	if (ret == 0) {
		prkyevt_config_done = true;
		pr_err("%s: read config success\n", __func__);
	}
}

/* send event to zrhung when match out requirements model */
static void zrhung_powerkeyevent_send_work(struct work_struct *work)
{
	pr_err("%s: ZRHUNG_EVENT_POWERKEY send to zerohung\n", __func__);
	zrhung_send_event(ZRHUNG_EVENT_POWERKEY, NULL, NULL);
}

/* send event to zrhung when match out requirements model */
static void zrhung_powerkeyevent_report(unsigned int dur, unsigned int end)
{
	unsigned int send_interval;

	send_interval = end > lastreport_time ?
	    ((end - lastreport_time) / TIME_CONVERT_UNIT) : POWERKEYEVENT_DEFAULT_REPORT_MIN;
	if (unlikely(lastreport_time == 0)) {
		lastreport_time = end;
	} else if (send_interval < event_config.report_limit) {
		pr_err("%s: powerkeyevent too fast to report: %d\n", __func__, end);
		return;
	}
	lastreport_time = end;
	queue_work(data.workq, &powerkeyevent_sendwork);
}

/* just to build a loop in the array */
static unsigned int refresh_prkyevt_index(unsigned int event)
{
	unsigned int evt = event;

	if (evt < POWERKEYEVENT_MAX_COUNT)
		evt++;
	else
		evt = 0;
	return evt;
}

/* handle each powerkey event and check if match our model */
static void zrhung_new_powerkeyevent(unsigned int tmescs)
{
	unsigned int prkyevt_interval;
	unsigned int evt_index;
	int diff;

	powerkeyevent_time[newevt] = tmescs;
	evt_index = (newevt >= headevt) ?
	    (newevt - headevt) : (newevt + POWERKEYEVENT_MAX_COUNT + 1 - headevt);
	if (evt_index < (event_config.count - 1)) {
		pr_err("%s: powerkeyevent not enough-%d\n", __func__, event_config.count);
	} else {
		diff = powerkeyevent_time[newevt] - powerkeyevent_time[headevt];
		if (diff < 0) {
			pr_err("%s: powerkeyevent sth wrong in record time\n", __func__);
			return;
		}

		prkyevt_interval = (unsigned int)(diff / TIME_CONVERT_UNIT);
		if (prkyevt_interval <= event_config.timewindow) {
			pr_err
			    ("%s: powerkeyevent check send to zerohung %d\n",
			     __func__, tmescs);
			zrhung_powerkeyevent_report(prkyevt_interval, tmescs);
		}

		headevt = refresh_prkyevt_index(headevt);
	}
	newevt = refresh_prkyevt_index(newevt);
}

/* handle each powerkey event and check if match our model */
static void zrhung_powerkeyevent_handler(void)
{
	unsigned int curtime;
	unsigned long curjiff;

	if (!prkyevt_config_done) {
		pr_err
		    ("%s: powerkeyevent prkyevt_config_done=%d, read it\n",
		     __func__, prkyevt_config_done);
		queue_work(data.workq, &powerkeyevent_config);
		return;
	}
	if (event_config.enable == 0) {
		pr_err("%s: powerkeyevent check disable\n", __func__);
		return;
	}
	pr_err("%s: powerkeyevent check start\n", __func__);
	curjiff = jiffies;
	curtime = jiffies_to_msecs(curjiff);
	if (unlikely(lastprkyevt_time > curtime)) {
		pr_err("%s: powerkeyevent check but time overflow\n", __func__);
		lastprkyevt_time = curtime;
		return;
	} else if ((curtime - lastprkyevt_time) < event_config.ignore_limit) {
		pr_err
		    ("%s: powerkeyevent user press powerkey too fast-time:%d\n",
		     __func__, curtime);
		return;
	}
	lastprkyevt_time = curtime;
	zrhung_new_powerkeyevent(curtime);
}

/*
 * screen-off watchpoint should not work before vm finish starting.
 * -function get bootup infomation
 */
int hung_wp_screen_has_boot_success(void)
{
	static int boot_success;

	if (!boot_success)
#ifdef CONFIG_HISI_BB
		boot_success = (get_boot_keypoint() == STAGE_BOOTUP_END);
#else
		boot_success = 1;
#endif
	return boot_success;
}

/*
 * if there is a volumn key press  between powerkey press and release,
 * the event that screen being set to off would be stopped so screen-off
 * watchpoint should not report this normal situation.
 * -function call back from volume keys driver and get state to vkeys_pressed
 */
void hung_wp_screen_vkeys_cb(unsigned int knum, unsigned int value)
{
	if ((knum == WP_SCREEN_VUP_KEY) || (knum == WP_SCREEN_VDOWN_KEY))
		vkeys_pressed |= knum;
	else
		pr_err("%s: unkown volume keynum:%u\n", __func__, knum);
}

/* get a value which indicates if any vkeys has pressed */
static unsigned int hung_wp_screen_has_vkeys_pressed(void)
{
	return vkeys_pressed;
}

/* clear the pressed vkeys value */
static void hung_wp_screen_clear_vkeys_pressed(void)
{
	vkeys_pressed = 0;
}

/* Get configuration from zerohung_config.xml */
static int hung_wp_screen_get_config(int id, struct hung_wp_screen_config *cfg)
{
	int ret;
	char config[CONFIG_LEN_MAX] = {0};

	ret = zrhung_get_config(id, config, CONFIG_LEN_MAX);
	if (ret != 0) {
		pr_err("%s: read config:%d fail: %d\n", __func__, id, ret);
		cfg->enable = 0;
		cfg->timeout = DEFAULT_TIMEOUT;
		return ret;
	}

	/* 2: the number of successful values returned */
	if (sscanf(config, "%d,%d", &cfg->enable, &cfg->timeout) != 2)
		pr_err("%s: get enable or timeout failed\n", __func__);

	pr_err("%s: id=%d, enable=%d, timeout=%d\n",
	       __func__, id, cfg->enable, cfg->timeout);
	if (cfg->timeout <= 0)
		cfg->timeout = DEFAULT_TIMEOUT;

	return ret;
}

/* Get configuration from zerohung_config.xml */
static void hung_wp_screen_config_work(struct work_struct *work)
{
	int ret_on;
	int ret_off;

	ret_on = hung_wp_screen_get_config(ZRHUNG_WP_SCREENON, &on_config);
	ret_off = hung_wp_screen_get_config(ZRHUNG_WP_SCREENOFF, &off_config);
	if ((ret_on <= 0) && (ret_off <= 0)) {
		config_done = true;
		pr_err("%s: init done\n", __func__);
	}
}

/*
 * Called from lcd driver when setting backlight for mark:
 *   For hisi platform, it's "hisifb_set_backlight" in hisi_fb_bl.c
 */
void hung_wp_screen_setbl(int level)
{
#ifndef HUNG_WP_FACTORY_MODE
	unsigned long flags;

	if (!init_done)
		return;

	spin_lock_irqsave(&(data.lock), flags);
	data.bl_level = level;
	if (!config_done) {
		spin_unlock_irqrestore(&(data.lock), flags);
		return;
	}
	if (((data.check_id == ZRHUNG_WP_SCREENON) && (level != 0)) ||
	    ((data.check_id == ZRHUNG_WP_SCREENOFF) && (level == 0))) {
		pr_err("%s: check_id=%d, level=%d\n", __func__,
		       data.check_id, data.bl_level);
		del_timer(&data.timer);
		data.check_id = ZRHUNG_WP_NONE;
	}
	spin_unlock_irqrestore(&(data.lock), flags);
#endif
}

/* Send envent through zrhung_send_event */
void hung_wp_screen_send_work(struct work_struct *work)
{
	unsigned long flags = 0;
	char cmd_buf[COMAND_LEN_MAX] = {0};
	u64 cur_stamp;

#ifdef CONFIG_HISI_TIME
	cur_stamp = hisi_getcurtime() / NANOS_PER_SECOND;
#else
	cur_stamp = local_clock() / NANOS_PER_SECOND;
#endif
	pr_err("%s: cur_stamp=%llu\n", __func__, cur_stamp);
	if (get_logusertype_flag() == BETA_USER)
		snprintf(cmd_buf, COMAND_LEN_MAX, CMD_BUF_FMT_BETA,
			 data.config->timeout + BUFFER_TIME_START,
			 cur_stamp + BUFFER_TIME_END);
	else
		snprintf(cmd_buf, COMAND_LEN_MAX, CMD_BUF_FMT,
			 data.config->timeout + BUFFER_TIME_START,
			 cur_stamp + BUFFER_TIME_END);

	pr_err("%s: cmd_buf: %s\n", __func__, cmd_buf);
	zrhung_send_event(data.check_id, cmd_buf, "none");
	pr_err("%s: send event: %d\n", __func__, data.check_id);
	spin_lock_irqsave(&(data.lock), flags);
	data.check_id = ZRHUNG_WP_NONE;
	spin_unlock_irqrestore(&(data.lock), flags);
}

/* Send event when timeout */
static void hung_wp_screen_send(unsigned long pdata)
{
	del_timer(&data.timer);
	show_state_filter(TASK_UNINTERRUPTIBLE);
	pr_err("%s: hung_wp_screen_%d end\n", __func__, data.tag_id);
	queue_work(data.workq, &data.send_work);
}

/* Start up the timer work for screen on delay check */
void hung_wp_screen_start(int check_id)
{
	if (data.check_id != ZRHUNG_WP_NONE) {
		pr_err("%s: already in check_id: %d\n", __func__, data.check_id);
		return;
	}
	data.config = (check_id == ZRHUNG_WP_SCREENON) ? &on_config : &off_config;
	if (!data.config->enable) {
		pr_err("%s: id=%d, enable=%d\n", __func__,
		       check_id, data.config->enable);
		return;
	}

	/* check if bootup success */
	if (!hung_wp_screen_has_boot_success()) {
		pr_err("%s: bootup not completed yet\n", __func__);
		return;
	}

	data.check_id = check_id;
	if (timer_pending(&data.timer))
		del_timer(&data.timer);

	data.timer.expires = jiffies + msecs_to_jiffies(data.config->timeout * TIME_CONVERT_UNIT);
	add_timer(&data.timer);
	pr_err("%s: going to check ID=%d timeout=%d\n",
	       __func__, check_id, data.config->timeout);

	return;

}

#ifndef CONFIG_KEYBOARD_HISI_GPIO_KEY

/* Store the powerkey and volume down key state for key state check */
void *hung_wp_screen_qcom_pkey_press(int type, int state)
{
	static int keys[PON_KEYS] = {0};

	if ((type >= PON_KPDPWR) && (type < PON_KEYS))
		keys[type] = state;

	return (void *)keys;
}

static int hung_wp_screen_get_qcom_volup_gpio(void)
{
	static int volup_gpio;
	struct device_node *node = NULL;
	struct device_node *snode = NULL;
	/* -1 means already read fail before, so just quit */
	if (volup_gpio == -1)
		return -1;
	if (volup_gpio != 0)
		return volup_gpio;
	node = of_find_compatible_node(NULL, NULL, "gpio-keys");
	if (node == NULL) {
		volup_gpio = -1;
		pr_err("%s:gpio-keys not found\n", __func__);
		return -1;
	}
	snode = of_find_node_by_name(node, "vol_up");
	if (snode == NULL) {
		volup_gpio = -1;
		pr_err("%s:vol_up not found\n", __func__);
		return -1;
	}
	volup_gpio = of_get_named_gpio(snode, "gpios", 0);
	if (!gpio_is_valid(volup_gpio)) {
		volup_gpio = -1;
		pr_err("%s:invalid gpio: %d\n", __func__, volup_gpio);
		return -1;
	}
	pr_err("%s:volup gpio: %d\n", __func__, volup_gpio);
	return volup_gpio;
}

static int hung_wp_screen_qcom_vkey_get(void)
{
	int volup_gpio = hung_wp_screen_get_qcom_volup_gpio();
	int vol_up_state = (gpio_get_value(volup_gpio) ? 1 : 0) ^ OF_GPIO_ACTIVE_LOW;
	int *state = (int *)hung_wp_screen_qcom_pkey_press(PON_KEYS, 0);
	int vol_state = *(state + PON_RESIN) | vol_up_state;

	return vol_state;
}

static void hung_wp_screen_qcom_lpress(unsigned long pdata)
{
	unsigned long flags = 0;
	int *state = (int *)hung_wp_screen_qcom_pkey_press(PON_KEYS, 0);

	pr_err("%s: get powerkey state:%d\n", __func__, *state);

	spin_lock_irqsave(&(data.lock), flags);
	if (*state) {
		pr_err("%s: check_id=%d, level=%d\n", __func__,
		       data.check_id, data.bl_level);
		/* don't check screen off when powerkey long pressed */
		int *check_off = (int *)pdata;
		*check_off = 0;
	}
	spin_unlock_irqrestore(&(data.lock), flags);

	del_timer(&data.long_press_timer);
}
#endif

/* Call back from powerkey driver */
void hung_wp_screen_powerkey_ncb(unsigned long event)
{
#ifndef HUNG_WP_FACTORY_MODE
	static int check_off;
	int volkeys;
	unsigned int vkeys_pressed_tmp;
	unsigned long flags = 0;

	if (!init_done)
		return;

	if (!config_done) {
		pr_err("%s: config_done=%d, read config first\n",
		       __func__, config_done);
		queue_work(data.workq, &data.config_work);
		return;
	}
#ifdef CONFIG_KEYBOARD_HISI_GPIO_KEY
	volkeys = gpio_key_vol_updown_press_get();
#elif CONFIG_MTK_ZRHUNG_FEATURE
	volkeys = vkeys_pressed > 0 ? true : false;
#else
	volkeys = hung_wp_screen_qcom_vkey_get();
#endif

	spin_lock_irqsave(&(data.lock), flags);
	if (event == WP_SCREEN_PWK_PRESS) {
		pr_err("%s: hung_wp_screen_%d start! bllevel=%d, volkeys=%d\n",
		       __func__, ++data.tag_id, data.bl_level, volkeys);
		check_off = 0;
		if (data.bl_level == 0) {
			hung_wp_screen_start(ZRHUNG_WP_SCREENON);
		} else if (volkeys == 0) {
			hung_wp_screen_clear_vkeys_pressed();
			check_off = 1;
#ifndef CONFIG_KEYBOARD_HISI_GPIO_KEY
			pr_err("start longpress test timer\n");
			data.long_press_timer.data = (unsigned long)&check_off;
			data.long_press_timer.expires = jiffies + msecs_to_jiffies(TIME_CONVERT_UNIT);
			if (!timer_pending(&data.long_press_timer))
				add_timer(&data.long_press_timer);
#endif
		}
		zrhung_powerkeyevent_handler();
	} else if (check_off) {
		check_off = 0;
#ifndef CONFIG_KEYBOARD_HISI_GPIO_KEY
		del_timer(&data.long_press_timer);
#endif
		if ((event == WP_SCREEN_PWK_RELEASE) && (volkeys == 0)) {
			vkeys_pressed_tmp = hung_wp_screen_has_vkeys_pressed();
			if (vkeys_pressed_tmp)
				pr_err("%s: vkeys_pressed_tmp:0x%x\n",
				       __func__, vkeys_pressed_tmp);
			else
				hung_wp_screen_start(ZRHUNG_WP_SCREENOFF);
		}
	}
	spin_unlock_irqrestore(&(data.lock), flags);
#endif
}

static int hung_wp_screen_powerkey_reg_ncb(struct notifier_block *powerkey_nb,
					   unsigned long event, void *data)
{
#ifdef CONFIG_KEYBOARD_HISI_GPIO_KEY
	if (event == HISI_PRESS_KEY_DOWN)
		hung_wp_screen_powerkey_ncb(WP_SCREEN_PWK_PRESS);
	else if (event == HISI_PRESS_KEY_UP)
		hung_wp_screen_powerkey_ncb(WP_SCREEN_PWK_RELEASE);
	else if (event == HISI_PRESS_KEY_1S)
		hung_wp_screen_powerkey_ncb(WP_SCREEN_PWK_LONGPRESS);
#endif
	return 0;
}

/* return backlight to huawei_hung_task */
int hwhungtask_get_backlight(void)
{
	return data.bl_level;
}

/* Configuration init */
static int __init hung_wp_screen_init(void)
{
	init_done = false;
	config_done = false;
#ifdef HUNG_WP_FACTORY_MODE
	pr_err("%s: factory mode, not init\n", __func__);
#else
	pr_err("%s: +\n", __func__);
	data.bl_level = 1;
	data.tag_id = 0;
	data.check_id = ZRHUNG_WP_NONE;
	spin_lock_init(&(data.lock));
	init_timer(&data.timer);
	data.timer.function = hung_wp_screen_send;
	data.timer.data = 0;
#ifndef CONFIG_KEYBOARD_HISI_GPIO_KEY
	init_timer(&data.long_press_timer);
	data.long_press_timer.function = hung_wp_screen_qcom_lpress;
	data.long_press_timer.data = 0;
	hung_wp_screen_get_qcom_volup_gpio();
#endif
	data.workq = create_workqueue("hung_wp_screen_workq");
	if (data.workq == NULL) {
		pr_err("%s: create workq failed\n", __func__);
		return -1;
	}
	INIT_WORK(&data.config_work, hung_wp_screen_config_work);
	INIT_WORK(&data.send_work, hung_wp_screen_send_work);
	INIT_WORK(&powerkeyevent_config, zrhung_powerkeyevent_config_work);
	INIT_WORK(&powerkeyevent_sendwork, zrhung_powerkeyevent_send_work);
#ifdef CONFIG_KEYBOARD_HISI_GPIO_KEY
	hung_wp_screen_powerkey_nb.notifier_call = hung_wp_screen_powerkey_reg_ncb;
	hisi_powerkey_register_notifier(&hung_wp_screen_powerkey_nb);
#endif
	init_done = true;
	pr_err("%s: -\n", __func__);
#endif
	return 0;
}

module_init(hung_wp_screen_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Reporting the frozen screen alarm event");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

