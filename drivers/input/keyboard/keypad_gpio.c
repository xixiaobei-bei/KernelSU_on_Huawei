/*
 * gpio keypad_gpio key driver
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

#include "keypad.h"

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#ifdef CONFIG_HW_ZEROHUNG
#include <chipset_common/hwzrhung/hung_wp_screen.h>
#include <chipset_common/hwzrhung/zrhung.h>
#endif

#define KEY_PRESSED_SLEEP 5

#define PRESSED 1
#define UNPRESSED 0

#define KEYPAD_GPIO_UP_KEY_NAME_STR "hisi_volume_up_key"
#define KEYPAD_GPIO_UP_KEY_NODE_NAME_STR "gpio-keyup,gpio-irq"
#define KEYPAD_GPIO_UP_KEY_WAKE_NAME_STR "key_up_wake_lock"
#define KEYPAD_GPIO_UP_KEY_GPIO_TO_IRQ_NAME_STR "gpio_up"

#define KEYPAD_GPIO_DOWN_KEY_NAME_STR "hisi_volume_down_key"
#define KEYPAD_GPIO_DOWN_KEY_NODE_NAME_STR "gpio-keydown,gpio-irq"
#define KEYPAD_GPIO_DOWN_KEY_WAKE_NAME_STR "key_down_wake_lock"
#define KEYPAD_GPIO_DOWN_KEY_GPIO_TO_IRQ_NAME_STR "gpio_down"

#if defined(CONFIG_HUAWEI_DSM)
#define PRESS_KEY_INTERVAL 80   /* the minimum press interval */
#define STATISTIC_INTERVAL 60   /* the statistic interval for key event */
#define MAX_PRESS_KEY_COUNT 120 /* the default press count for a normal use */
#define DSM_KEY_BUF_SIZE 1024

static int keypad_gpio_up_press_count;
static int keypad_gpio_down_press_count;
static unsigned long keypad_gpio_down_last_press_time;
static unsigned long keypad_gpio_up_last_press_time;
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

struct keypad_gpio_key_devices {
	struct keypad_key_devices *keypad_gpio_up_key;
	struct keypad_key_devices *keypad_gpio_down_key;
};

/* The following interface variable is used only for composite keys into
 * fastboot mode, complete dump function
 */
static unsigned char s_vol_down_hold;
static int vol_up_gpio = -1;
static int vol_up_active_low = -1;
static int vol_down_gpio = -1;
static int vol_down_active_low = -1;

#if defined(CONFIG_HUAWEI_DSM)
static void dsm_gpio_key_timer_func(unsigned long data)
{
	int key_press_err_found = -1;

	if (!dsm_client_ocuppy(key_dclient))
		key_press_err_found++;

	/* judge the press count */
	if ((!key_press_err_found) &&
		(keypad_gpio_up_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_err_found++;
		dsm_client_record(key_dclient,
			"keypad_gpio up key trigger on the abnormal style\n");
	} else if (!key_press_err_found &&
		(keypad_gpio_down_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_err_found++;
		dsm_client_record(key_dclient,
			"keypad_gpio down key trigger on the abnormal style\n");
	}

	/* key_dclient preempt failed, we have nothing to do */
	if (key_press_err_found > 0)
		dsm_client_notify(key_dclient, DSM_KEY_ERROR_NO);
	else if (!key_press_err_found)
		dsm_client_unocuppy(key_dclient);

	/* reset the statistic variable */
	keypad_gpio_up_press_count = 0;
	keypad_gpio_down_press_count = 0;
	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);
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

static const struct of_device_id keypad_gpio_key_match[] = {
	{.
		compatible = "hisilicon,gpio-key"
	},
	{},
};
MODULE_DEVICE_TABLE(of, keypad_gpio_key_match);

static void dms_point(struct keypad_key_devices *key_dev)
{
	if (!strcmp(key_dev->devices_names, KEYPAD_GPIO_UP_KEY_NAME_STR)) {
		char *record_up_msg =
			"keypad_gpio up key trigger on the abnormal style\n";

		if ((jiffies - keypad_gpio_up_last_press_time) <
			msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, record_up_msg);
				dsm_client_notify(
					key_dclient, DSM_KEY_ERROR_NO);
			}
		}
		keypad_gpio_up_last_press_time = jiffies;
	} else if (!strcmp(key_dev->devices_names,
		KEYPAD_GPIO_DOWN_KEY_NAME_STR)) {
		char *record_down_msg =
			"keypad_gpio dowm key trigger on the abnormal style\n";

		if ((jiffies - keypad_gpio_down_last_press_time) <
			msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, record_down_msg);
				dsm_client_notify(
					key_dclient, DSM_KEY_ERROR_NO);
			}
		}
		keypad_gpio_down_last_press_time = jiffies;
	}
}

static void report_key(struct keypad_key_devices *key_dev,
	unsigned int report_action)
{
	if (!strcmp(key_dev->devices_names,
		KEYPAD_GPIO_UP_KEY_NAME_STR)) {
		pr_info("[keypad_gpiokey] keypad_gpio up key %d action %u\n",
			KEY_VOLUMEUP, report_action);
		input_report_key(key_dev->input_dev,
			KEY_VOLUMEUP, report_action);
		input_sync(key_dev->input_dev);
	} else if (!strcmp(key_dev->devices_names,
		KEYPAD_GPIO_DOWN_KEY_NAME_STR)) {
		pr_info("[keypad_gpiokey]volumn down key %d action %u\n",
			KEY_VOLUMEDOWN, report_action);
		input_report_key(key_dev->input_dev,
			KEY_VOLUMEDOWN, report_action);
		input_sync(key_dev->input_dev);
	}
}

#ifdef CONFIG_HW_ZEROHUNG
static void hung_wp_screen(struct keypad_key_devices *key_dev,
	unsigned int report_action)
{
	if (!strcmp(key_dev->devices_names,
		KEYPAD_GPIO_UP_KEY_NAME_STR)) {
		hung_wp_screen_vkeys_cb(WP_SCREEN_VUP_KEY,
				report_action);
	} else if (!strcmp(key_dev->devices_names,
		KEYPAD_GPIO_DOWN_KEY_NAME_STR)) {
		hung_wp_screen_vkeys_cb(WP_SCREEN_VDOWN_KEY,
				report_action);
	}
}
#endif

static void keypad_gpio_key_work(struct work_struct *work)
{
	struct keypad_key_devices *key_dev =
		container_of(work, struct keypad_key_devices, gpio_key_work.work);
	struct keypad_common_data *g_key_common_data = NULL;
	unsigned int key_keypad_gpio_value;
	unsigned int report_action = GPIO_KEY_RELEASE;

#if defined(CONFIG_HUAWEI_DSM)
	dms_point(key_dev);
#endif

	key_keypad_gpio_value = gpio_get_value(
		(unsigned int)key_dev->keypad_num);
	/* judge key is pressed or released. */
	if (key_keypad_gpio_value == GPIO_LOW_VOLTAGE) {
		report_action = GPIO_KEY_PRESS;
#if defined(CONFIG_HUAWEI_DSM)
	if (!strcmp(key_dev->devices_names, KEYPAD_GPIO_UP_KEY_NAME_STR))
		keypad_gpio_up_press_count++;
	else if (!strcmp(key_dev->devices_names,
		KEYPAD_GPIO_DOWN_KEY_NAME_STR))
		keypad_gpio_down_press_count++;
#endif
	} else if (key_keypad_gpio_value == GPIO_HIGH_VOLTAGE) {
		report_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[keypad_gpiokey][%s]invalid gpio key_keypad_gpio\n", __func__);
		return;
	}

	g_key_common_data = keypad_get_common_data();
#ifdef CONFIG_KEY_MASK
	/* make sure report in pair */
	if ((g_key_common_data->key_mask_ctl == KEY_DISABLE_MASK) &&
		(report_action == GPIO_KEY_PRESS))
		g_key_common_data->key_mask_flag = KEY_DISABLE_MASK;
	else if (g_key_common_data->key_mask_ctl == KEY_ENABLE_MASK)
		g_key_common_data->key_mask_flag = KEY_ENABLE_MASK;

	if (g_key_common_data->key_mask_flag == KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen(key_dev, report_action);
#endif
		report_key(key_dev, report_action);
#ifdef CONFIG_KEY_MASK
	} else {
		pr_info("[keypad_gpiokey]%s key is disabled %u\n",
			key_dev->devices_names, report_action);
	}
#endif

	if (key_keypad_gpio_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&key_dev->gpio_key_lock);
}

static irqreturn_t gpio_key_keypad_gpio_irq_handler(int irq, void *dev_id)
{
	struct keypad_key_devices *key_dev =
		(struct keypad_key_devices *)dev_id;
	int key_event = 0;

	__pm_stay_awake(&key_dev->gpio_key_lock);

	/* handle gpio key keypad_gpio up & gpio key keypad_gpio down event at here */
		key_event = gpio_get_value_cansleep(
			(unsigned int)key_dev->keypad_num);
		if (!key_event) {
			if (!strcmp(key_dev->devices_names,
				KEYPAD_GPIO_UP_KEY_NAME_STR)) {
				gpio_key_vol_updown_press_set_bit(VOL_UP_BIT);
			} else if (!strcmp(key_dev->devices_names,
				KEYPAD_GPIO_DOWN_KEY_NAME_STR)) {
				gpio_key_vol_updown_press_set_bit(VOL_DOWN_BIT);
			}
		} else {
			gpio_key_vol_updown_press_set_zero();
		}

	/* If @delay is 0 , Invoke work immediately */
	queue_delayed_work(system_power_efficient_wq,
		&key_dev->gpio_key_work, TIMER_DEBOUNCE);

	return IRQ_HANDLED;
}

static void keypad_gpio_up_key_set_bit(struct keypad_key_devices *key_dev)
{
	set_bit(KEY_VOLUMEUP, key_dev->input_dev->keybit);
}

static void keypad_gpio_down_key_set_bit(struct keypad_key_devices *key_dev)
{
	set_bit(KEY_VOLUMEDOWN, key_dev->input_dev->keybit);
}

static int keypad_gpio_key_req_irq(struct keypad_key_devices *key_dev)
{
	int err;

	err = request_irq(key_dev->keypad_irq,
		gpio_key_keypad_gpio_irq_handler,
		IRQF_NO_SUSPEND |
		IRQF_TRIGGER_RISING |
		IRQF_TRIGGER_FALLING,
		key_dev->pdev->name, key_dev);
	if (err)
		return err;

	return 0;
}

static int dms_init(struct keypad_key_devices *key_dev)
{
	setup_timer(&dsm_gpio_key_timer, dsm_gpio_key_timer_func,
		(uintptr_t)key_dev);

	if (!key_dclient)
		key_dclient = dsm_register_client(&dsm_key);

	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);

	/* initialize the statistic variable */
	keypad_gpio_up_press_count = 0;
	keypad_gpio_down_press_count = 0;
	keypad_gpio_up_last_press_time = 0;
	keypad_gpio_down_last_press_time = 0;

	return 0;
}

static void keypad_gpio_init(struct keypad_key_devices *keypad_gpio_up_key,
	struct keypad_key_devices *keypad_gpio_down_key,
	struct keypad_gpio_key_devices *key_keypad_gpio_dev)
{
	keypad_gpio_up_key->devices_names = KEYPAD_GPIO_UP_KEY_NAME_STR;
	keypad_gpio_up_key->key_node_name = KEYPAD_GPIO_UP_KEY_NODE_NAME_STR;
	keypad_gpio_up_key->work_init_name = KEYPAD_GPIO_UP_KEY_WAKE_NAME_STR;
	keypad_gpio_up_key->gpio_to_irq_name =
		KEYPAD_GPIO_UP_KEY_GPIO_TO_IRQ_NAME_STR;

	keypad_gpio_up_key->key_set_bit = keypad_gpio_up_key_set_bit;
	keypad_gpio_up_key->key_work = keypad_gpio_key_work;
	keypad_gpio_up_key->keypad_req_irq = keypad_gpio_key_req_irq;

	keypad_gpio_down_key->devices_names = KEYPAD_GPIO_DOWN_KEY_NAME_STR;
	keypad_gpio_down_key->key_node_name =
		KEYPAD_GPIO_DOWN_KEY_NODE_NAME_STR;
	keypad_gpio_down_key->work_init_name =
		KEYPAD_GPIO_DOWN_KEY_WAKE_NAME_STR;
	keypad_gpio_down_key->gpio_to_irq_name =
		KEYPAD_GPIO_DOWN_KEY_GPIO_TO_IRQ_NAME_STR;

	keypad_gpio_down_key->key_set_bit = keypad_gpio_down_key_set_bit;
	keypad_gpio_down_key->key_work = keypad_gpio_key_work;
	keypad_gpio_down_key->keypad_req_irq = keypad_gpio_key_req_irq;

	key_keypad_gpio_dev->keypad_gpio_up_key = keypad_gpio_up_key;
	key_keypad_gpio_dev->keypad_gpio_down_key = keypad_gpio_down_key;

#if defined(CONFIG_HUAWEI_DSM)
	dms_init(keypad_gpio_up_key);
#endif

	vol_up_gpio = keypad_gpio_up_key->keypad_num;
	vol_up_active_low = GPIO_KEY_PRESS;
	vol_down_gpio = keypad_gpio_down_key->keypad_num;
	vol_down_active_low = GPIO_KEY_PRESS;
}

static int keypad_gpio_key_probe(struct platform_device *pdev)
{
	struct keypad_key_devices *keypad_gpio_up_key = NULL;
	struct keypad_key_devices *keypad_gpio_down_key = NULL;
	struct keypad_gpio_key_devices *key_keypad_gpio_dev = NULL;
	int ret;

	dev_info(&pdev->dev, "%s\n", __func__);

	ret = keypad_get_of_node(keypad_gpio_key_match, pdev);
	if (ret) {
		pr_err("keypad_get_of_node fail\n");
		return -EINVAL;
	}

	key_keypad_gpio_dev = devm_kzalloc(&pdev->dev,
		sizeof(*key_keypad_gpio_dev), GFP_KERNEL);
	if (!key_keypad_gpio_dev)
		return -ENOMEM;

	keypad_gpio_up_key = devm_kzalloc(&pdev->dev,
		sizeof(*keypad_gpio_up_key), GFP_KERNEL);
	if (!keypad_gpio_up_key)
		return -ENOMEM;

	keypad_gpio_down_key = devm_kzalloc(&pdev->dev,
		sizeof(*keypad_gpio_down_key), GFP_KERNEL);
	if (!keypad_gpio_down_key)
		return -ENOMEM;

	keypad_gpio_init(keypad_gpio_up_key, keypad_gpio_down_key,
		key_keypad_gpio_dev);

	keypad_gpio_up_key->pdev = pdev;
	keypad_gpio_down_key->pdev = pdev;

	ret = keypad_register_dev(keypad_gpio_up_key);
	if (ret) {
		pr_err("%s register fail\n",
			keypad_gpio_up_key->devices_names);
		return -EINVAL;
	}

	ret = keypad_register_dev(keypad_gpio_down_key);
	if (ret) {
		pr_err("%s register fail\n",
			keypad_gpio_down_key->devices_names);
		return -EINVAL;
	}

	platform_set_drvdata(pdev, key_keypad_gpio_dev);
	dev_info(&pdev->dev, "%s success\n", __func__);

	return 0;
}

static int keypad_gpio_key_remove(struct platform_device *pdev)
{
	struct keypad_gpio_key_devices *key_keypad_gpio_dev =
		platform_get_drvdata(pdev);

	if (!key_keypad_gpio_dev) {
		pr_err("%s:failed to get drvdata\n", __func__);
		return -EINVAL;
	}

	keypad_unregister_dev(key_keypad_gpio_dev->keypad_gpio_up_key);
	keypad_unregister_dev(key_keypad_gpio_dev->keypad_gpio_down_key);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int gpio_keypad_gpio_key_suspend(
	struct platform_device *pdev, pm_message_t state)
{
	dev_info(&pdev->dev, "%s: suspend +\n", __func__);
	dev_info(&pdev->dev, "%s: suspend -\n", __func__);

	return 0;
}

static int gpio_keypad_gpio_key_resume(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s: resume +\n", __func__);
	dev_info(&pdev->dev, "%s: resume -\n", __func__);

	return 0;
}
#endif

struct platform_driver keypad_gpio_key_driver = {
	.probe = keypad_gpio_key_probe,
	.remove = keypad_gpio_key_remove,
	.driver = {
		.name = "keypad_gpio_key",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(keypad_gpio_key_match),
	},
#ifdef CONFIG_PM
	.suspend = gpio_keypad_gpio_key_suspend,
	.resume = gpio_keypad_gpio_key_resume,
#endif
};

module_platform_driver(keypad_gpio_key_driver);

MODULE_DESCRIPTION("Gpio keypad_gpio key platform driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
