/*
 * volum key driver
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "hisi_gpio_key.h"

#include <asm/irq.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/proc_fs.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <securec.h>

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#ifdef CONFIG_HW_ZEROHUNG
#include <chipset_common/hwzrhung/hung_wp_screen.h>
#include <chipset_common/hwzrhung/zrhung.h>
#endif

#define TRUE 1

#define GPIO_KEY_PRESS 1
#define GPIO_KEY_RELEASE 0

#define GPIO_HIGH_VOLTAGE 1
#define GPIO_LOW_VOLTAGE 0
#define TIMER_DEBOUNCE 15

#ifdef CONFIG_HISI_PMIC_VIBRATOR
#define HISI_PMIC_VIBRATOR_SMARTKEY 3
int hisi_pmic_vibrator_haptics_set_type(int type);
#endif

#define INT_STR_LEN 8
#define MAX_BUF_LEN 64
#define WRITE_BUF_LEN 8
#define KEY_IRQ_WAKEUP_INTERVAL 50
#define KEY_PRESSED_SLEEP 5

#define PRESSED 1
#define UNPRESSED 0

static struct wakeup_source volume_up_key_lock;
static struct wakeup_source volume_down_key_lock;
static u32 support_smart_key;
static u32 smart_key_vibrate;

#if defined(CONFIG_HUAWEI_DSM)
#define PRESS_KEY_INTERVAL 80   /* the minimum press interval */
#define STATISTIC_INTERVAL 60   /* the statistic interval for key event */
#define MAX_PRESS_KEY_COUNT 120 /* the default press count for a normal use */
#define DSM_KEY_BUF_SIZE 1024

static int volume_up_press_count;
static int volume_down_press_count;
static unsigned long volume_down_last_press_time;
static unsigned long volume_up_last_press_time;
/* used to reset the statistic variable */
static struct timer_list dsm_gpio_key_timer;

static struct dsm_dev dsm_key = {
	.name = "dsm_key",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.buff_size = DSM_KEY_BUF_SIZE,
	.fops = NULL,
};

static struct dsm_client *key_dclient;
#endif

/* The following interface variable is used only for composite keys into
 * fastboot mode, complete dump function
 */
static unsigned char s_vol_down_hold;
static int vol_up_gpio = -1;
static int vol_up_active_low = -1;
static int vol_down_gpio = -1;
static int vol_down_active_low = -1;

#ifdef CONFIG_VOLUME_KEY_MASK
#define VOLUME_KEY_DISABLE_MASK 1
#define VOLUME_KEY_ENABLE_MASK 0
static int key_mask_ctl = VOLUME_KEY_ENABLE_MASK;
static int key_mask_flag = VOLUME_KEY_ENABLE_MASK;
#endif

struct hisi_gpio_key {
	struct input_dev *input_dev;
	struct delayed_work gpio_keyup_work;
	struct delayed_work gpio_keydown_work;
	struct timer_list key_up_timer;
	struct timer_list key_down_timer;
	int gpio_up;
	int gpio_down;
	int volume_up_irq;   /* volumn up key irq */
	int volume_down_irq; /* volumn down key irq */
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	struct delayed_work gpio_keyback_work;
	struct timer_list key_back_timer;
	int gpio_back;
	int key_back_irq;
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	struct delayed_work gpio_keysmart_work;
	struct timer_list key_smart_timer;
	int gpio_smart;
	int key_smart_irq;
#endif
	struct pinctrl *pctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_idle;
};

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
static struct wakeup_source back_key_lock;
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
static struct wakeup_source smart_key_lock;
#endif

#define MAX_KEY_IRQ_NUM 10

struct gpio_key_init_ctx {
	struct hisi_gpio_key *gpio_key;
	struct platform_device *pdev;
	u32 irq_arr[MAX_KEY_IRQ_NUM];
	int irq_cnt;
};

#if defined(CONFIG_HUAWEI_DSM)
static void dsm_gpio_key_timer_func(unsigned long data)
{
	int key_press_err_found = -1;

	if (!dsm_client_ocuppy(key_dclient))
		key_press_err_found++;

	/* judge the press count */
	if ((!key_press_err_found) &&
		(volume_up_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_err_found++;
		dsm_client_record(key_dclient,
			"volume up key trigger on the abnormal style\n");
	} else if (!key_press_err_found &&
		(volume_down_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_err_found++;
		dsm_client_record(key_dclient,
			"volume down key trigger on the abnormal style\n");
	}

	/* key_dclient preempt failed, we have nothing to do */
	if (key_press_err_found > 0)
		dsm_client_notify(key_dclient, DSM_KEY_ERROR_NO);
	else if (!key_press_err_found)
		dsm_client_unocuppy(key_dclient);

	/* reset the statistic variable */
	volume_up_press_count = 0;
	volume_down_press_count = 0;
	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);
}
#endif

static int of_get_key_gpio(struct device_node *np, const char *propname,
	int prop_index, int gpio_index, enum of_gpio_flags *flags)
{
	int ret;

#ifdef CONFIG_GPIO_LOGIC
	ret = of_get_gpio_by_prop(np, propname, prop_index, gpio_index, flags);
#else
	ret = of_get_named_gpio(np, propname, prop_index);
#endif

	return ret;
}

static int hisi_gpio_key_open(struct input_dev *dev)
{
	return 0;
}

static void hisi_gpio_key_close(struct input_dev *dev)
{
}

static void hisi_gpio_keyup_work(struct work_struct *work)
{
	struct hisi_gpio_key *gpio_key =
		container_of(work, struct hisi_gpio_key, gpio_keyup_work.work);
	unsigned int keyup_value;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keyup_value = gpio_get_value((unsigned int)gpio_key->gpio_up);
	/* judge key is pressed or released. */
	if (keyup_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
#if defined(CONFIG_HUAWEI_DSM)
		volume_up_press_count++;
#endif
	} else if (keyup_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[gpiokey][%s]invalid gpio key_value\n", __func__);
		return;
	}

#ifdef CONFIG_VOLUME_KEY_MASK
	/* make sure report in pair */
	if ((key_mask_ctl == VOLUME_KEY_DISABLE_MASK) &&
		(report_action == GPIO_KEY_PRESS))
		key_mask_flag = VOLUME_KEY_DISABLE_MASK;
	else if (key_mask_ctl == VOLUME_KEY_ENABLE_MASK)
		key_mask_flag = VOLUME_KEY_ENABLE_MASK;

	if (key_mask_flag == VOLUME_KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_vkeys_cb(WP_SCREEN_VUP_KEY, report_action);
#endif
		pr_info("[gpiokey] volumn key %d action %u\n",
			KEY_VOLUMEUP, report_action);
		input_report_key(
			gpio_key->input_dev, KEY_VOLUMEUP, report_action);
		input_sync(gpio_key->input_dev);
#ifdef CONFIG_VOLUME_KEY_MASK
	} else {
		pr_info("[gpiokey]volumn key is disabled %u\n", report_action);
	}
#endif

	if (keyup_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&volume_up_key_lock);
}

static void hisi_gpio_keydown_work(struct work_struct *work)
{
	struct hisi_gpio_key *gpio_key = container_of(
		work, struct hisi_gpio_key, gpio_keydown_work.work);
	unsigned int keydown_value;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keydown_value = gpio_get_value((unsigned int)gpio_key->gpio_down);
	/* judge key is pressed or released. */
	if (keydown_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
#if defined(CONFIG_HUAWEI_DSM)
		volume_down_press_count++;
#endif
	} else if (keydown_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[gpiokey][%s]invalid gpio key_value\n", __func__);
		return;
	}

#ifdef CONFIG_VOLUME_KEY_MASK
	/* make sure report in pair */
	if ((key_mask_ctl == VOLUME_KEY_DISABLE_MASK) &&
		(report_action == GPIO_KEY_PRESS))
		key_mask_flag = VOLUME_KEY_DISABLE_MASK;
	else if (key_mask_ctl == VOLUME_KEY_ENABLE_MASK)
		key_mask_flag = VOLUME_KEY_ENABLE_MASK;

	if (key_mask_flag == VOLUME_KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_vkeys_cb(WP_SCREEN_VDOWN_KEY, report_action);
#endif
		pr_info("[gpiokey]volumn key %d action %u\n",
			KEY_VOLUMEDOWN, report_action);
		input_report_key(
			gpio_key->input_dev, KEY_VOLUMEDOWN, report_action);
		input_sync(gpio_key->input_dev);
#ifdef CONFIG_VOLUME_KEY_MASK
	} else {
		pr_info("[gpiokey]volumn key is disabled %u\n", report_action);
	}
#endif

	if (keydown_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&volume_down_key_lock);
}

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
static void hisi_gpio_keyback_work(struct work_struct *work)
{
	struct hisi_gpio_key *gpio_key = container_of(
		work, struct hisi_gpio_key, gpio_keyback_work.work);
	unsigned int keyback_value;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keyback_value = gpio_get_value((unsigned int)gpio_key->gpio_back);
	/* judge key is pressed or released. */
	if (keyback_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
	} else if (keyback_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[gpiokey][%s]invalid gpio key_value\n", __func__);
		return;
	}

	pr_info("[gpiokey]back key %d action %u\n", KEY_BACK, report_action);
	input_report_key(gpio_key->input_dev, KEY_BACK, report_action);
	input_sync(gpio_key->input_dev);

	if (keyback_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&back_key_lock);
}
#endif

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
static void hisi_gpio_keysmart_work(struct work_struct *work)
{
	struct hisi_gpio_key *gpio_key = container_of(
		work, struct hisi_gpio_key, gpio_keysmart_work.work);
	unsigned int keysmart_value;
	unsigned int report_action = GPIO_KEY_RELEASE;

	keysmart_value = gpio_get_value((unsigned int)gpio_key->gpio_smart);
	/* judge key is pressed or released. */
	if (keysmart_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
#ifdef CONFIG_HISI_PMIC_VIBRATOR
		/* vibrator for vitrual btn */
		if (smart_key_vibrate)
			hisi_pmic_vibrator_haptics_set_type(
				HISI_PMIC_VIBRATOR_SMARTKEY);
#endif
	} else if (keysmart_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[gpiokey][%s]invalid gpio key_value\n", __func__);
		return;
	}

	pr_info("[gpiokey]smart key %d action %u\n", KEY_F24, report_action);
	input_report_key(gpio_key->input_dev, KEY_F24, report_action);
	input_sync(gpio_key->input_dev);

	if (keysmart_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&smart_key_lock);
}
#endif

static void gpio_keyup_timer(unsigned long data)
{
	int keyup_value;
	struct hisi_gpio_key *gpio_key =
		(struct hisi_gpio_key *)(uintptr_t)data;
#if defined(CONFIG_HUAWEI_DSM)
	char *record_msg = "volume up key trigger on the abnormal style\n";
#endif

	keyup_value = gpio_get_value((unsigned int)gpio_key->gpio_up);
	/* judge key is pressed or released. */
	if (keyup_value == GPIO_LOW_VOLTAGE) {
		__pm_stay_awake(&volume_up_key_lock);

#if defined(CONFIG_HUAWEI_DSM)
		if ((jiffies - volume_up_last_press_time) <
			msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, record_msg);
				dsm_client_notify(
					key_dclient, DSM_KEY_ERROR_NO);
			}
		}
		volume_up_last_press_time = jiffies;
#endif
	}

	/* If @delay is 0 , Invoke work immediately */
	schedule_delayed_work(&(gpio_key->gpio_keyup_work), 0);
}

static void gpio_keydown_timer(unsigned long data)
{
	int keydown_value;
	struct hisi_gpio_key *gpio_key =
		(struct hisi_gpio_key *)(uintptr_t)data;
#if defined(CONFIG_HUAWEI_DSM)
	char *record_msg = "volume down key trigger on the abnormal style\n";
#endif
	keydown_value = gpio_get_value((unsigned int)gpio_key->gpio_down);
	/* judge key is pressed or released. */
	if (keydown_value == GPIO_LOW_VOLTAGE) {
		__pm_stay_awake(&volume_down_key_lock);

#if defined(CONFIG_HUAWEI_DSM)
		if ((jiffies - volume_down_last_press_time) <
			msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, record_msg);
				dsm_client_notify(
					key_dclient, DSM_KEY_ERROR_NO);
			}
		}
		volume_down_last_press_time = jiffies;
#endif
	}

	/* If @delay is 0 , Invoke work immediately */
	schedule_delayed_work(&(gpio_key->gpio_keydown_work), 0);
}

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
static void gpio_keyback_timer(unsigned long data)
{
	int keyback_value;
	struct hisi_gpio_key *gpio_key = (struct hisi_gpio_key *)data;

	keyback_value = gpio_get_value((unsigned int)gpio_key->gpio_back);
	/* judge key is pressed or released. */
	if (keyback_value == GPIO_LOW_VOLTAGE)
		__pm_stay_awake(&back_key_lock);

	/* If @delay is 0 , Invoke work immediately */
	schedule_delayed_work(&(gpio_key->gpio_keyback_work), 0);
}
#endif

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
static void gpio_keysmart_timer(unsigned long data)
{
	int keysmart_value;
	struct hisi_gpio_key *gpio_key =
		(struct hisi_gpio_key *)(uintptr_t)data;

	keysmart_value = gpio_get_value((unsigned int)gpio_key->gpio_smart);
	/* judge key is pressed or released. */
	if (keysmart_value == GPIO_LOW_VOLTAGE)
		__pm_stay_awake(&smart_key_lock);

	/* If @delay is 0 , Invoke work immediately */
	schedule_delayed_work(&(gpio_key->gpio_keysmart_work), 0);
}
#endif

static void gpio_key_vol_updown_press_set_bit(u32 bit_number)
{
	s_vol_down_hold |= (1 << bit_number);
}

void gpio_key_vol_updown_press_set_zero(void)
{
	s_vol_down_hold = 0;
}

int gpio_key_vol_updown_press_get(void)
{
	return (int)s_vol_down_hold;
}

int is_gpio_key_vol_updown_pressed(void)
{
	u32 state1, state2;

	if ((vol_up_gpio < 0) || (vol_down_gpio < 0) ||
		(vol_up_active_low < 0) || (vol_down_active_low < 0)) {
		pr_err("[%s]:vol_updown gpio or active_low is invalid!\n",
			__func__);
		return UNPRESSED;
	}

	mdelay(KEY_PRESSED_SLEEP);
	state1 = (gpio_get_value_cansleep(vol_up_gpio) ? PRESSED : UNPRESSED)
		 ^ ((unsigned int)vol_up_active_low);
	state2 = (gpio_get_value_cansleep(vol_down_gpio) ? PRESSED : UNPRESSED)
		 ^ ((unsigned int)vol_down_active_low);
	if (!!state1 && !!state2)
		return PRESSED;

	return UNPRESSED;
}

static irqreturn_t hisi_gpio_key_irq_handler(int irq, void *dev_id)
{
	struct hisi_gpio_key *gpio_key = (struct hisi_gpio_key *)dev_id;
	int key_event = 0;

	/* handle gpio key volume up & gpio key volume down event at here */
	if (irq == gpio_key->volume_up_irq) {
		key_event = gpio_get_value_cansleep(
			(unsigned int)gpio_key->gpio_up);
		if (!key_event)
			gpio_key_vol_updown_press_set_bit(VOL_UP_BIT);
		else
			gpio_key_vol_updown_press_set_zero();

		mod_timer(&(gpio_key->key_up_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&volume_up_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
	} else if (irq == gpio_key->volume_down_irq) {
		key_event = gpio_get_value_cansleep(
			(unsigned int)gpio_key->gpio_down);
		if (!key_event)
			gpio_key_vol_updown_press_set_bit(VOL_DOWN_BIT);
		else
			gpio_key_vol_updown_press_set_zero();

		__pm_wakeup_event(&volume_down_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
		mod_timer(&(gpio_key->key_down_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	} else if (irq == gpio_key->key_back_irq) {
		mod_timer(&(gpio_key->key_back_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&back_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	} else if (support_smart_key && irq == gpio_key->key_smart_irq) {
		mod_timer(&(gpio_key->key_smart_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&smart_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
#endif
	} else {
		pr_err("[gpiokey] [%s]invalid irq %d!\n", __func__, irq);
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static const struct of_device_id hs_gpio_key_match[] = {
	{.compatible = "hisilicon,gpio-key"}, {},
};
MODULE_DEVICE_TABLE(of, hs_gpio_key_match);
#endif

#ifdef CONFIG_VOLUME_KEY_MASK
static ssize_t key_mask_read(
	struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[INT_STR_LEN] = {0};
	int ret;

	if (!user_buf) {
		pr_err("[gpiokey]user_buf is NULL!\n");
		return -EINVAL;
	}

	ret = snprintf_s(
		buf, sizeof(buf), sizeof(buf) - 1, "%d", key_mask_flag);
	if (ret < 0) {
		pr_err("[%s]snprintf_s failed. ret=%d key_mask read:%d\n",
			__func__, ret, key_mask_flag);
		return ret;
	}

	pr_info("[%s]key_mask read:%d\n", __func__, key_mask_flag);

	return simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
}

static ssize_t key_mask_write(struct file *file, const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	char buff[MAX_BUF_LEN] = {0};
	int mask = 0;
	size_t size;

	size = min_t(size_t, count, WRITE_BUF_LEN);

	if (!user_buf) {
		pr_err("[gpiokey]user_buf is NULL!\n");
		return -EINVAL;
	}

	if (copy_from_user(buff, user_buf, size)) {
		pr_err("[gpiokey]copy from user fail!\n");
		return 0;
	}

	if (sscanf_s(buff, "%d", &mask) != 1) {
		pr_err("[gpiokey]sscanf_s fail!\n");
		return 0;
	}

	if ((mask == VOLUME_KEY_ENABLE_MASK) ||
		(mask == VOLUME_KEY_DISABLE_MASK))
		key_mask_ctl = mask;

	pr_info("[%s]key_mask write:%d,%d\n", __func__, mask, key_mask_ctl);

	return count;
}

static const struct file_operations key_mask_fops = {
	.read = key_mask_read,
	.write = key_mask_write,
};
#endif

int hisi_exec_init_flow(const struct hisi_init_processor *init_flow,
	int flow_size, void *init_ctx)
{
	int i, max_flow_cnt, fail_idx;
	int ret = 0;

	if (!init_flow) {
		pr_err("init_flow is NULL\n");
		return -EINVAL;
	}

	if (!init_ctx) {
		pr_err("init_ctx is NULL\n");
		return -EINVAL;
	}

	max_flow_cnt = min_t(int, MAX_INIT_FLOW_CNT, flow_size);
	for (i = 0; i < max_flow_cnt; i++) {
		ret = init_flow[i].init(init_ctx);
		if (ret)
			break;
	}

	if (!ret) {
		pr_err("all init_flow has been execute success\n");
		return 0;
	}

	pr_err("%s:%d function invoke fail\n", __func__, i);

	fail_idx = i;
	for (; i >= 0; i--)
		if (init_flow[i].rollback)
			if (init_flow[i].rollback(init_ctx, i, fail_idx))
				pr_err("invoke %d rollback func fail\n", i);

	return ret;
}

static int hisi_gpio_key_get_of_node(void *init_ctx)
{
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;

	if (!of_match_node(hs_gpio_key_match, pdev->dev.of_node)) {
		dev_err(&pdev->dev, "dev node is not match. exiting\n");
		return -ENODEV;
	}

	return 0;
}

static int hisi_gpio_key_read_global_cfg(void *init_ctx)
{
	int err;
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;

	err = of_property_read_u32(pdev->dev.of_node, "support_smart_key",
		&support_smart_key);
	if (err) {
		support_smart_key = 0;
		pr_info("%s: Not support smart_key\n", __func__);
	} else {
		pr_info("%s: Support smart_key: %u\n", __func__,
			support_smart_key);
	}

	err = of_property_read_u32(pdev->dev.of_node, "smart_key_vibrate",
		&smart_key_vibrate);
	if (err) {
		smart_key_vibrate = 0;
		pr_info("%s: Not support smart_key vibrate\n", __func__);
	} else {
		pr_info("%s: Support smart_key vibrate: %u\n", __func__,
			smart_key_vibrate);
	}

	return 0;
}

static struct input_dev *hisi_gpio_key_new_input_dev(
	struct platform_device *pdev)
{
	struct input_dev *input_dev = NULL;

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&pdev->dev, "Failed to allocate struct input_dev!\n");
		return NULL;
	}

	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(KEY_VOLUMEUP, input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, input_dev->keybit);
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	set_bit(KEY_BACK, input_dev->keybit);
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key)
		set_bit(KEY_F24, input_dev->keybit);
#endif
	input_dev->open = hisi_gpio_key_open;
	input_dev->close = hisi_gpio_key_close;

	return input_dev;
}

static int hisi_gpio_key_init_work(void *init_ctx)
{
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;

	/* initial work before we use it. */
	INIT_DELAYED_WORK(&(gpio_key->gpio_keyup_work), hisi_gpio_keyup_work);
	INIT_DELAYED_WORK(&(gpio_key->gpio_keydown_work),
		hisi_gpio_keydown_work);

	wakeup_source_init(&volume_down_key_lock, "key_down_wake_lock");
	wakeup_source_init(&volume_up_key_lock, "key_up_wake_lock");
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	INIT_DELAYED_WORK(
		&(gpio_key->gpio_keyback_work), hisi_gpio_keyback_work);
	wakeup_source_init(&back_key_lock, "key_back_wake_lock");
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key) {
		INIT_DELAYED_WORK((struct delayed_work *)(uintptr_t)(
			&(gpio_key->gpio_keysmart_work)),
			hisi_gpio_keysmart_work);
		wakeup_source_init(&smart_key_lock, "key_smart_wake_lock");
	}
#endif
	return 0;
}

static int hisi_gpio_key_new_drvdata(void *init_ctx)
{
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;
	struct hisi_gpio_key *gpio_key = NULL;
	struct input_dev *input_dev = NULL;

	gpio_key = devm_kzalloc(&pdev->dev, sizeof(struct hisi_gpio_key),
		GFP_KERNEL);
	if (!gpio_key)
		return -ENOMEM;

	input_dev = hisi_gpio_key_new_input_dev(pdev);
	if (!input_dev) {
		dev_err(&pdev->dev, "new input dev fail\n");
		return -ENOMEM;
	}

	input_set_drvdata(input_dev, gpio_key);
	gpio_key->input_dev = input_dev;

	((struct gpio_key_init_ctx *)init_ctx)->gpio_key = gpio_key;

	return 0;
}

static int hisi_gpio_init_pin(void *init_ctx)
{
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;
	enum of_gpio_flags flags;

	gpio_key->gpio_up = of_get_key_gpio(
		pdev->dev.of_node, "gpio-keyup,gpio-irq", 0, 0, &flags);
	if (!gpio_is_valid(gpio_key->gpio_up))
		pr_info("%s:gpio of vol_up isn't valid, chk DTS\n", __func__);

	gpio_key->gpio_down = of_get_key_gpio(
		pdev->dev.of_node, "gpio-keydown,gpio-irq", 0, 0, &flags);
	if (!gpio_is_valid(gpio_key->gpio_down))
		pr_info("%s:gpio of vol_down isn't valid, chk DTS\n", __func__);

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	gpio_key->gpio_back = of_get_key_gpio(
		pdev->dev.of_node, "gpio-keyback,gpio-irq", 0, 0, &flags);
	if (!gpio_is_valid(gpio_key->gpio_back))
		pr_info("%s:gpio of back key isn't valid, chk DTS\n", __func__);
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key) {
		gpio_key->gpio_smart = of_get_key_gpio(pdev->dev.of_node,
			"gpio-keysmart,gpio-irq", 0, 0, &flags);
		if (!gpio_is_valid(gpio_key->gpio_smart))
			pr_info("%s:gpio of smart key isn't valid, chk DTS\n",
				__func__);
	}
#endif

	vol_up_gpio = gpio_key->gpio_up;
	vol_up_active_low = GPIO_KEY_PRESS;
	vol_down_gpio = gpio_key->gpio_down;
	vol_down_active_low = GPIO_KEY_PRESS;

	return 0;
}

static int hisi_gpio_key_gpio_to_irq(struct platform_device *pdev, int *irq,
	unsigned int gpio, char *label)
{
	int err;

	if (!gpio_is_valid(gpio)) {
		dev_info(&pdev->dev, "gpio:%u is invalid\n", gpio);
		return 0;
	}

	err = devm_gpio_request(&pdev->dev, gpio, label);
	if (err < 0) {
		dev_err(&pdev->dev, "Fail request gpio:%u\n", gpio);
		return err;
	}

	err = gpio_direction_input(gpio);
	if (err < 0) {
		dev_err(&pdev->dev, "Set %s direction failed!\n", label);
		return err;
	}

	*irq = gpio_to_irq(gpio);
	if (*irq < 0) {
		dev_err(&pdev->dev,
			"Get gpio key press irq fail, %s:%u!\n", label, gpio);
		return *irq;
	}

	return 0;
}

static int hisi_gpio_key_init_pctl(void *init_ctx)
{
	int err;
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;

	gpio_key->pctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(gpio_key->pctrl)) {
		dev_err(&pdev->dev, "failed to devm pinctrl get\n");
		return -EINVAL;
	}

	gpio_key->pins_default =
		pinctrl_lookup_state(gpio_key->pctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(gpio_key->pins_default)) {
		dev_err(&pdev->dev, "failed to pinctrl lookup state default\n");
		return -EINVAL;
	}

	gpio_key->pins_idle =
		pinctrl_lookup_state(gpio_key->pctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(gpio_key->pins_idle)) {
		dev_err(&pdev->dev, "failed to pinctrl lookup state idle\n");
		return -EINVAL;
	}

	err = pinctrl_select_state(gpio_key->pctrl, gpio_key->pins_default);
	if (err < 0) {
		dev_err(&pdev->dev, "set iomux normal error, %d\n", err);
		return -EINVAL;
	}

	return 0;
}

static int hisi_gpio_setup_timers(void *init_ctx)
{
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;

	setup_timer(&(gpio_key->key_up_timer), gpio_keyup_timer,
		(uintptr_t)gpio_key);
	setup_timer(&(gpio_key->key_down_timer), gpio_keydown_timer,
		(uintptr_t)gpio_key);
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	setup_timer(&(gpio_key->key_back_timer), gpio_keyback_timer,
		(uintptr_t)gpio_key);
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key)
		setup_timer(&(gpio_key->key_smart_timer), gpio_keysmart_timer,
			(uintptr_t)gpio_key);
#endif

#if defined(CONFIG_HUAWEI_DSM)
	setup_timer(&dsm_gpio_key_timer, dsm_gpio_key_timer_func,
		(uintptr_t)gpio_key);
#endif
	return 0;
}

static int hisi_gpio_req_irq(int key, int irq, struct hisi_gpio_key *gpio_key,
	struct gpio_key_init_ctx *ctx)
{
	int err;
	int *irq_arr = ctx->irq_arr;
	int irq_arr_size = ARRAY_SIZE(ctx->irq_arr);
	struct platform_device *pdev = ctx->pdev;

	if (!gpio_is_valid(key))
		return 0;

	if (ctx->irq_cnt >= irq_arr_size) {
		pr_err("irq_cnt is out of bounds, irq:%d key:%d\n", irq, key);
		return -ENOMEM;
	}

	err = request_irq(irq,
		hisi_gpio_key_irq_handler,
		IRQF_NO_SUSPEND |
		IRQF_TRIGGER_RISING |
		IRQF_TRIGGER_FALLING,
		pdev->name, gpio_key);
	if (err)
		return err;

	irq_arr[ctx->irq_cnt] = irq;
	ctx->irq_cnt++;

	return 0;
}

static void hisi_gpio_free_irqs(u32 *irq_arr, int irq_cnt,
	struct hisi_gpio_key *gpio_key)
{
	int i, cnt;

	cnt = min_t(int, MAX_KEY_IRQ_NUM, irq_cnt);
	for (i = 0; i < cnt; i++)
		free_irq(irq_arr[i], gpio_key);
}

static int hisi_gpio_key_gpio_to_irq_all(void *init_ctx)
{
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;
	int err;

	err = hisi_gpio_key_gpio_to_irq(pdev, &gpio_key->volume_up_irq,
		(unsigned int)gpio_key->gpio_up, "gpio_up");
	if (err)
		return err;

	err = hisi_gpio_key_gpio_to_irq(pdev, &gpio_key->volume_down_irq,
		(unsigned int)gpio_key->gpio_down, "gpio_down");
	if (err)
		return err;

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	err = hisi_gpio_key_gpio_to_irq(pdev, &gpio_key->key_back_irq,
		(unsigned int)gpio_key->gpio_back, "gpio_back");
	if (err)
		return err;
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key) {
		err = hisi_gpio_key_gpio_to_irq(pdev, &gpio_key->key_smart_irq,
			(unsigned int)gpio_key->gpio_smart, "gpio_smart");
		if (err)
			return err;
	}
#endif
	return 0;
}

static int hisi_gpio_key_req_irq_all(void *init_ctx)
{
	int err;
	struct gpio_key_init_ctx *ctx = (struct gpio_key_init_ctx *)init_ctx;
	struct hisi_gpio_key *gpio_key = ctx->gpio_key;
	struct platform_device *pdev = ctx->pdev;

	/*
	 * support failing irq that means volume-up-key is pressed,
	 * and rising irq which means volume-up-key is released.
	 */
	err = hisi_gpio_req_irq(gpio_key->gpio_up, gpio_key->volume_up_irq,
		gpio_key, ctx);
	if (err) {
		dev_err(&pdev->dev, "Request up interrupt fail!\n");
		return err;
	}

	/*
	 * support failing irq that means volume-down-key is pressed,
	 * and rising irq which means volume-down-key is released.
	 */
	err = hisi_gpio_req_irq(gpio_key->gpio_down, gpio_key->volume_down_irq,
		gpio_key, ctx);
	if (err) {
		dev_err(&pdev->dev, "Request down interrupt fail!\n");
		return err;
	}

#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	err = hisi_gpio_req_irq(gpio_key->gpio_back, gpio_key->key_back_irq,
		gpio_key, ctx);
	if (err) {
		dev_err(&pdev->dev, "Request back interrupt fail!\n");
		return err;
	}
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key) {
		err = hisi_gpio_req_irq(gpio_key->gpio_smart,
			gpio_key->key_smart_irq, gpio_key, ctx);
		if (err) {
			dev_err(&pdev->dev, "Request smart interrupt fail!\n");
			return err;
		}
	}
#endif
	return 0;
}

static int hisi_gpio_key_reg_indev(void *init_ctx)
{
	int err;
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;

	err = input_register_device(gpio_key->input_dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to register input device!\n");
		return err;
	}

	device_init_wakeup(&pdev->dev, TRUE);
	platform_set_drvdata(pdev, gpio_key);

	return 0;
}

static int hisi_gpio_key_init_last_step(void *init_ctx)
{
#if defined(CONFIG_HUAWEI_DSM)
	if (!key_dclient)
		key_dclient = dsm_register_client(&dsm_key);

	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);
#endif

#ifdef CONFIG_VOLUME_KEY_MASK
	/* 0660 is S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR */
	if (!proc_create("key_mask", 0660, NULL, &key_mask_fops))
		pr_err("[%s]:failed to create key_mask proc\n", __func__);
#endif

#if defined(CONFIG_HUAWEI_DSM)
	/* initialize the statistic variable */
	volume_up_press_count = 0;
	volume_down_press_count = 0;
	volume_up_last_press_time = 0;
	volume_down_last_press_time = 0;
#endif

	return 0;
}

static int hisi_gpio_key_init_pctl_rollback(void *init_ctx,
	int cur_idx, int fail_idx)
{
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;

	(void)cur_idx;
	(void)fail_idx;
	if (gpio_key->pctrl)
		devm_pinctrl_put(gpio_key->pctrl);

	return 0;
}

static int hisi_gpio_key_req_irq_all_rollback(void *init_ctx,
	int cur_idx, int fail_idx)
{
	struct gpio_key_init_ctx *ctx = (struct gpio_key_init_ctx *)init_ctx;
	struct hisi_gpio_key *gpio_key = ctx->gpio_key;

	(void)cur_idx;
	(void)fail_idx;
	hisi_gpio_free_irqs(ctx->irq_arr, ctx->irq_cnt, gpio_key);
	return 0;
}

static int hisi_gpio_key_new_drvdata_rollback(void *init_ctx,
	int cur_idx, int fail_idx)
{
	struct gpio_key_init_ctx *ctx = (struct gpio_key_init_ctx *)init_ctx;
	struct hisi_gpio_key *gpio_key = ctx->gpio_key;

	(void)cur_idx;
	(void)fail_idx;
	if (gpio_key && gpio_key->input_dev)
		input_free_device(gpio_key->input_dev);

	((struct gpio_key_init_ctx *)init_ctx)->gpio_key = NULL;
	return 0;
}

static int hisi_gpio_key_init_work_rollback(void *init_ctx,
	int cur_idx, int fail_idx)
{
	(void)cur_idx;
	(void)fail_idx;
	(void)init_ctx;
	wakeup_source_trash(&volume_down_key_lock);
	wakeup_source_trash(&volume_up_key_lock);
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	wakeup_source_trash(&back_key_lock);
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key)
		wakeup_source_trash(&smart_key_lock);
#endif
	return 0;
}

static int hisi_gpio_key_reg_indev_rollback(void *init_ctx,
	int cur_idx, int fail_idx)
{
	struct platform_device *pdev =
		((struct gpio_key_init_ctx *)init_ctx)->pdev;
	struct hisi_gpio_key *gpio_key =
		((struct gpio_key_init_ctx *)init_ctx)->gpio_key;

	if (cur_idx == fail_idx)
		return 0;

	input_unregister_device(gpio_key->input_dev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static int hisi_gpio_key_probe(struct platform_device *pdev)
{
	int err;
	struct gpio_key_init_ctx init_ctx;
	struct hisi_init_processor hisi_gpio_key_init_flow[] = {
#ifdef CONFIG_OF
		{hisi_gpio_key_get_of_node, NULL},
#endif
		{hisi_gpio_key_read_global_cfg, NULL},
		{hisi_gpio_key_new_drvdata, hisi_gpio_key_new_drvdata_rollback},
		{hisi_gpio_key_init_work, hisi_gpio_key_init_work_rollback},
		{hisi_gpio_init_pin, NULL}, /* initial key gpio pin. */
		{hisi_gpio_key_gpio_to_irq_all, NULL},
		{hisi_gpio_key_init_pctl, hisi_gpio_key_init_pctl_rollback},
		{hisi_gpio_setup_timers, NULL},
		{hisi_gpio_key_req_irq_all, hisi_gpio_key_req_irq_all_rollback},
		{hisi_gpio_key_reg_indev, hisi_gpio_key_reg_indev_rollback},
		{hisi_gpio_key_init_last_step, NULL}
	};

	/* Prepare init_ctx */
	if (memset_s(&init_ctx, sizeof(struct gpio_key_init_ctx), 0,
		sizeof(struct gpio_key_init_ctx))) {
		pr_err("%s:[gpiokey]memset_s failed\n", __func__);
		return -ENOMEM;
	}
	init_ctx.pdev = pdev;

	/* Execute init flow */
	err = hisi_exec_init_flow(hisi_gpio_key_init_flow,
		ARRAY_SIZE(hisi_gpio_key_init_flow), (void *)&init_ctx);
	if (err) {
		pr_info("[gpiokey] gpio key probe failed!ret = %d\n", err);
		return err;
	}
	dev_info(&pdev->dev, "hisi gpio key driver probes successfully!\n");

	return 0;
}

static int hisi_gpio_key_remove(struct platform_device *pdev)
{
	struct hisi_gpio_key *gpio_key = platform_get_drvdata(pdev);
	bool ret = true;

	if (!gpio_key) {
		pr_err("get invalid gpio_key pointer\n");
		return -EINVAL;
	}

	free_irq(gpio_key->volume_up_irq, gpio_key);
	free_irq(gpio_key->volume_down_irq, gpio_key);
	gpio_free((unsigned int)gpio_key->gpio_up);
	gpio_free((unsigned int)gpio_key->gpio_down);
	devm_pinctrl_put(gpio_key->pctrl);

	ret = cancel_delayed_work(&(gpio_key->gpio_keyup_work));
	if (!ret)
		pr_err("%s:cancel gpio key up work failed\n", __func__);

	ret = cancel_delayed_work(&(gpio_key->gpio_keydown_work));
	if (!ret)
		pr_err("%s:cancel gpio key down work failed\n", __func__);

	wakeup_source_trash(&volume_down_key_lock);
	wakeup_source_trash(&volume_up_key_lock);
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_HI6XXX
	free_irq(gpio_key->key_back_irq, gpio_key);
	gpio_free((unsigned int)gpio_key->gpio_back);
	cancel_delayed_work(&(gpio_key->gpio_keyback_work));
	wakeup_source_trash(&back_key_lock);
#endif
#ifdef CONFIG_HISI_GPIO_KEY_SUPPORT_SMART_KEY
	if (support_smart_key) {
		free_irq(gpio_key->key_smart_irq, gpio_key);
		gpio_free((unsigned int)gpio_key->gpio_smart);
		cancel_delayed_work(&(gpio_key->gpio_keysmart_work));
		wakeup_source_trash(&smart_key_lock);
	}
#endif

	input_unregister_device(gpio_key->input_dev);
	platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_VOLUME_KEY_MASK
	remove_proc_entry("key_mask", NULL);
#endif
	return 0;
}

#ifdef CONFIG_PM
static int hisi_gpio_key_suspend(
	struct platform_device *pdev, pm_message_t state)
{
	int err;
	struct hisi_gpio_key *gpio_key = platform_get_drvdata(pdev);

	if (!gpio_key) {
		pr_err("%s gpio_key is NULL\n", __func__);
		return -EINVAL;
	}

	dev_info(&pdev->dev, "%s: suspend +\n", __func__);

	err = pinctrl_select_state(gpio_key->pctrl, gpio_key->pins_idle);
	if (err < 0)
		dev_err(&pdev->dev, "set iomux normal error, %d\n", err);

	dev_info(&pdev->dev, "%s: suspend -\n", __func__);
	return 0;
}

static int hisi_gpio_key_resume(struct platform_device *pdev)
{
	int err;
	struct hisi_gpio_key *gpio_key = platform_get_drvdata(pdev);

	if (!gpio_key) {
		pr_err("%s gpio_key is NULL\n", __func__);
		return -EINVAL;
	}

	dev_info(&pdev->dev, "%s: resume +\n", __func__);

	err = pinctrl_select_state(gpio_key->pctrl, gpio_key->pins_default);
	if (err < 0)
		dev_err(&pdev->dev, "set iomux idle error, %d\n", err);

	dev_info(&pdev->dev, "%s: resume -\n", __func__);
	return 0;
}
#endif

struct platform_driver hisi_gpio_key_driver = {
	.probe = hisi_gpio_key_probe,
	.remove = hisi_gpio_key_remove,
	.driver = {
		.name = "hisi_gpio_key",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hs_gpio_key_match),
	},
#ifdef CONFIG_PM
	.suspend = hisi_gpio_key_suspend,
	.resume = hisi_gpio_key_resume,
#endif
};

module_platform_driver(hisi_gpio_key_driver);

MODULE_DESCRIPTION("Hisilicon keypad platform driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
