/*
 * volum key driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
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
#include <asm/irq.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/hisi-spmi.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_hisi_spmi.h>
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

#if (defined CONFIG_HISI_HI6421V900_PMU)
#define PMIC_GPIO_KEYUP_DATA 0x08
#define PMIC_GPIO_KEYDOWN_DATA 0x08
#define PMIC_KEYPAD_UP (1 << 1)
#define PMIC_KEYPAD_DOWN (1 << 2)
#define PMIC_KEYPAD_UP_DATA 0x2
#define PMIC_KEYPAD_DOWN_DATA 0x4
#define PMIC_GPIO_KEY_INT 0x2C1
#define PMIC_KEYUP_INT_CLEAR 0
#define PMIC_KEYPAD_UP_DATA_BIT 1
#define PMIC_KEYPAD_DOWN_DATA_BIT 2
#define PMIC_KEYPAD_UP_UP_INT 0
#define PMIC_KEYPAD_UP_DOWN_INT 1
#define PMIC_KEYPAD_DOWN_UP_INT 2
#define PMIC_KEYPAD_DOWN_DOWN_INT 3
#elif (defined CONFIG_HISI_HI6555V500_PMU)
#define PMIC_GPIO_KEYUP_DATA 0x09
#define PMIC_GPIO_KEYDOWN_DATA 0x09
#define PMIC_KEYPAD_UP (1 << 1)
#define PMIC_KEYPAD_DOWN (1 << 2)
#define PMIC_KEYPAD_UP_DATA 0x2
#define PMIC_KEYPAD_DOWN_DATA 0x4
#define PMIC_GPIO_KEY_INT 0x239
#define PMIC_KEYUP_INT_CLEAR 0
#define PMIC_KEYPAD_UP_DATA_BIT 1
#define PMIC_KEYPAD_DOWN_DATA_BIT 2
#define PMIC_KEYPAD_UP_UP_INT 0
#define PMIC_KEYPAD_UP_DOWN_INT 1
#define PMIC_KEYPAD_DOWN_UP_INT 2
#define PMIC_KEYPAD_DOWN_DOWN_INT 3
#elif (defined CONFIG_HISI_HI6555V300_PMU)
#define PMIC_GPIO_KEYUP_DATA 0x1ED
#define PMIC_GPIO_KEYUP_DIR 0x1EF
#define PMIC_GPIO_KEYUP_IS 0x1F1
#define PMIC_GPIO_KEYUP_IBE 0x1F2
#define PMIC_GPIO_KEYUP_PUSEL 0x1F5
#define PMIC_GPIO_KEYUP_PDSEL 0x1F6
#define PMIC_GPIO_KEYUP_DEBSEL 0x1F7
#define PMIC_GPIO_KEYDOWN_DATA 0x1F8
#define PMIC_GPIO_KEYDOWN_DIR 0x1EA
#define PMIC_GPIO_KEYDOWN_IS 0x1FC
#define PMIC_GPIO_KEYDOWN_IBE 0x1FD
#define PMIC_GPIO_KEYDOWN_PUSEL 0x200
#define PMIC_GPIO_KEYDOWN_PDSEL 0x201
#define PMIC_GPIO_KEYDOWN_DEBSEL 0x202
#else
#define PMIC_GPIO_KEYUP_DATA 0x264
#define PMIC_GPIO_KEYUP_DIR 0x266
#define PMIC_GPIO_KEYUP_IS 0x268
#define PMIC_GPIO_KEYUP_IBE 0x269
#define PMIC_GPIO_KEYUP_PUSEL 0x26C
#define PMIC_GPIO_KEYUP_PDSEL 0x26D
#define PMIC_GPIO_KEYUP_DEBSEL 0x26E
#define PMIC_GPIO_KEYDOWN_DATA 0x26F
#define PMIC_GPIO_KEYDOWN_DIR 0x271
#define PMIC_GPIO_KEYDOWN_IS 0x273
#define PMIC_GPIO_KEYDOWN_IBE 0x274
#define PMIC_GPIO_KEYDOWN_PUSEL 0x277
#define PMIC_GPIO_KEYDOWN_PDSEL 0x278
#define PMIC_GPIO_KEYDOWN_DEBSEL 0x279
#endif

#define KEY_VALUE_LOW 0x0
#define KEY_VALUE_HIGH 0x1

#define GPIO_HIGH_VOLTAGE 1
#define GPIO_LOW_VOLTAGE 0
#define TIMER_DEBOUNCE 15

#define KEY_PRESSED_SLEEP 5
#define KEY_IRQ_WAKEUP_INTERVAL 50
#define INT_STR_LEN 8
#define MAX_BUF_LEN 64
#define WRITE_BUF_LEN 8
#define PRESSED 1
#define UNPRESSED 0
#define DSM_KEY_BUF_SIZE 1024

static struct wakeup_source volume_up_key_lock;
static struct wakeup_source volume_down_key_lock;

#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
#define HISI_PMIC_GPIO_KEY_IRQ_CNT 4
static const char *hisi_pmic_gpio_key_irq_table[HISI_PMIC_GPIO_KEY_IRQ_CNT] = {
	"vol-up-up", "vol-up-down", "vol-down-up", "vol-down-down"
};
#else
#define HISI_PMIC_GPIO_KEY_IRQ_CNT 2
static const char *hisi_pmic_gpio_key_irq_table[HISI_PMIC_GPIO_KEY_IRQ_CNT] = {
	"vol-up", "vol-down",
};
#endif

#if defined(CONFIG_HUAWEI_DSM)
#define PRESS_KEY_INTERVAL 80   /* the minimum press interval */
#define STATISTIC_INTERVAL 60   /* the statistic interval for key event */
#define MAX_PRESS_KEY_COUNT 120 /* the default press count for a normal use */

static int volume_up_press_count;
static int volume_down_press_count;
static unsigned long volume_up_last_press_time;
static unsigned long volume_down_last_press_time;
/* used to reset the statistic variable */
static struct timer_list dsm_gpio_key_timer;

static struct dsm_dev dsm_key = {
	.name = "dsm_key",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = DSM_KEY_BUF_SIZE,
};

static struct dsm_client *key_dclient;
#endif

#ifdef CONFIG_VOLUME_KEY_MASK
#define VOLUME_KEY_ENABLE_MASK 0
#define VOLUME_KEY_DISABLE_MASK 1
static int key_mask_ctl = VOLUME_KEY_ENABLE_MASK;
static int key_mask_flag = VOLUME_KEY_ENABLE_MASK;
#endif

struct hisi_pmic_gpio_key {
	struct input_dev *input_dev;
	struct delayed_work gpio_keyup_work;
	struct delayed_work gpio_keydown_work;
	struct timer_list key_up_timer;
	struct timer_list key_down_timer;
	int gpiokey_irq[HISI_PMIC_GPIO_KEY_IRQ_CNT];
};

/* powerkey and keydown go to fastboot */
static unsigned char s_vol_down_hold;
static int vol_up_active_low = -1;
static int vol_down_active_low = -1;

#if defined(CONFIG_HUAWEI_DSM)
static void dsm_gpio_key_timer_func(unsigned long data)
{
	int key_press_error_found = -1;

	if (!dsm_client_ocuppy(key_dclient))
		key_press_error_found++;

	/* judge the press count */
	if ((!key_press_error_found) &&
		(volume_up_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_error_found++;
		dsm_client_record(key_dclient,
			"volume up key trigger on the abnormal style\n");
	} else if ((!key_press_error_found) &&
		(volume_down_press_count > MAX_PRESS_KEY_COUNT)) {
		key_press_error_found++;
		dsm_client_record(key_dclient,
			"volume down key trigger on the abnormal style\n");
	}

	if (key_press_error_found > 0)
		dsm_client_notify(key_dclient, DSM_KEY_ERROR_NO);
	else if (!key_press_error_found)
		dsm_client_unocuppy(key_dclient);

	/* reset the statistic variable */
	volume_up_press_count = 0;
	volume_down_press_count = 0;
	mod_timer(&dsm_gpio_key_timer, jiffies + STATISTIC_INTERVAL * HZ);
}
#endif

static unsigned int hisi_pmic_gpio_key_read(int gpio_address)
{
	return hisi_pmic_reg_read(gpio_address);
}

static void hisi_pmic_gpio_key_write(int gpiokey_set, int gpio_address)
{
	hisi_pmic_reg_write(gpio_address, gpiokey_set);
}

static int hisi_pmic_gpio_key_open(struct input_dev *dev)
{
	return 0;
}

static void hisi_pmic_gpio_key_close(struct input_dev *dev)
{
}

static void hisi_pmic_gpio_keyup_work(struct work_struct *work)
{
	struct hisi_pmic_gpio_key *gpio_key = container_of(
		work, struct hisi_pmic_gpio_key, gpio_keyup_work.work);
	unsigned int keyup_value;
	unsigned int rpt_action = GPIO_KEY_RELEASE;

	keyup_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
	keyup_value &= PMIC_KEYPAD_UP;
	keyup_value = keyup_value >> PMIC_KEYPAD_UP_DATA_BIT;
#endif
	/* judge key is pressed or released. */
	if (keyup_value == GPIO_LOW_VOLTAGE) {
		rpt_action = GPIO_KEY_PRESS;
#if defined(CONFIG_HUAWEI_DSM)
		volume_up_press_count++;
#endif
	} else if (keyup_value == GPIO_HIGH_VOLTAGE) {
		rpt_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[gpiokey][%s]invalid gpio key_value\n", __func__);
		return;
	}

#ifdef CONFIG_VOLUME_KEY_MASK
	/* make sure report in pair */
	if ((key_mask_ctl == VOLUME_KEY_DISABLE_MASK) &&
		(rpt_action == GPIO_KEY_PRESS))
		key_mask_flag = VOLUME_KEY_DISABLE_MASK;
	else if (key_mask_ctl == VOLUME_KEY_ENABLE_MASK)
		key_mask_flag = VOLUME_KEY_ENABLE_MASK;

	if (key_mask_flag == VOLUME_KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_vkeys_cb(WP_SCREEN_VUP_KEY, rpt_action);
#endif
		pr_info("[gpiokey] volumn key %u action %u\n",
			KEY_VOLUMEUP, rpt_action);
		input_report_key(gpio_key->input_dev, KEY_VOLUMEUP, rpt_action);
		input_sync(gpio_key->input_dev);
#ifdef CONFIG_VOLUME_KEY_MASK
	} else {
		pr_info("[gpiokey]volumn key is disabled %u\n", rpt_action);
	}
#endif

	if (keyup_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&volume_up_key_lock);
}

static void hisi_pmic_gpio_keydown_work(struct work_struct *work)
{
	struct hisi_pmic_gpio_key *gpio_key = container_of(
		work, struct hisi_pmic_gpio_key, gpio_keydown_work.work);
	unsigned int keydown_value;
	unsigned int rpt_action = GPIO_KEY_RELEASE;

	keydown_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
	keydown_value &= PMIC_KEYPAD_DOWN;
	keydown_value = keydown_value >> PMIC_KEYPAD_DOWN_DATA_BIT;
#endif
	/* judge key is pressed or released. */
	if (keydown_value == GPIO_LOW_VOLTAGE) {
		rpt_action = GPIO_KEY_PRESS;
#if defined(CONFIG_HUAWEI_DSM)
		volume_down_press_count++;
#endif
	} else if (keydown_value == GPIO_HIGH_VOLTAGE) {
		rpt_action = GPIO_KEY_RELEASE;
	} else {
		pr_err("[gpiokey][%s]invalid gpio key_value\n", __func__);
		return;
	}

#ifdef CONFIG_VOLUME_KEY_MASK
	/* make sure report in pair */
	if ((key_mask_ctl == VOLUME_KEY_DISABLE_MASK) &&
		(rpt_action == GPIO_KEY_PRESS))
		key_mask_flag = VOLUME_KEY_DISABLE_MASK;
	else if (key_mask_ctl == VOLUME_KEY_ENABLE_MASK)
		key_mask_flag = VOLUME_KEY_ENABLE_MASK;

	if (key_mask_flag == VOLUME_KEY_ENABLE_MASK) {
#endif
#ifdef CONFIG_HW_ZEROHUNG
		hung_wp_screen_vkeys_cb(WP_SCREEN_VDOWN_KEY, rpt_action);
#endif
		pr_info("[gpiokey]volumn key %u action %u\n",
			KEY_VOLUMEDOWN, rpt_action);
		input_report_key(
			gpio_key->input_dev, KEY_VOLUMEDOWN, rpt_action);
		input_sync(gpio_key->input_dev);
#ifdef CONFIG_VOLUME_KEY_MASK
	} else {
		pr_info("[gpiokey]volumn key is disabled %u\n", rpt_action);
	}
#endif

	if (keydown_value == GPIO_HIGH_VOLTAGE)
		__pm_relax(&volume_down_key_lock);
}

static void hisi_pmic_gpio_keyup_timer(unsigned long data)
{
	unsigned int keyup_value;
	struct hisi_pmic_gpio_key *gpio_key =
		(struct hisi_pmic_gpio_key *)(uintptr_t)data;
#if defined(CONFIG_HUAWEI_DSM)
	char *rcd_msg = "volume down key trigger on the abnormal style.\n";
#endif

	keyup_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
	keyup_value &= PMIC_KEYPAD_UP;
	keyup_value = keyup_value >> PMIC_KEYPAD_UP_DATA_BIT;
#endif
	/* judge key is pressed or released. */
	if (keyup_value == GPIO_LOW_VOLTAGE) {
		__pm_stay_awake(&volume_up_key_lock);
#if defined(CONFIG_HUAWEI_DSM)
		if ((jiffies - volume_up_last_press_time) <
			msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, rcd_msg);
				dsm_client_notify(key_dclient,
					DSM_KEY_ERROR_NO);
			}
		}
		volume_up_last_press_time = jiffies;
#endif
	}

	/* If @delay is 0 , Invoke work immediately */
	schedule_delayed_work(&(gpio_key->gpio_keyup_work), 0);
}

static void hisi_pmic_gpio_keydown_timer(unsigned long data)
{
	int keydown_value;
	struct hisi_pmic_gpio_key *gpio_key =
		(struct hisi_pmic_gpio_key *)(uintptr_t)data;
#if defined(CONFIG_HUAWEI_DSM)
	char *rcd_msg = "volume down key trigger on the abnormal style.\n";
#endif

	keydown_value = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
	keydown_value &= PMIC_KEYPAD_DOWN;
	keydown_value = keydown_value >> PMIC_KEYPAD_DOWN_DATA_BIT;
#endif
	/* judge key is pressed or released. */
	if (keydown_value == GPIO_LOW_VOLTAGE) {
		__pm_stay_awake(&volume_down_key_lock);

#if defined(CONFIG_HUAWEI_DSM)
		if ((jiffies - volume_down_last_press_time) <
			msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
			if (!dsm_client_ocuppy(key_dclient)) {
				dsm_client_record(key_dclient, rcd_msg);
				dsm_client_notify(key_dclient,
					DSM_KEY_ERROR_NO);
			}
		}
		volume_down_last_press_time = jiffies;
#endif
	}

	/* If @delay is 0 , Invoke work immediately */
	schedule_delayed_work(&(gpio_key->gpio_keydown_work), 0);
}

static void gpio_key_vol_updown_press_set_bit(u32 bit_number)
{
	s_vol_down_hold |= (1 << bit_number);
}

void pmic_gpio_key_vol_updown_press_set_zero(void)
{
	s_vol_down_hold = 0;
}

int pmic_gpio_key_vol_updown_press_get(void)
{
	return (int)s_vol_down_hold;
}

int is_pmic_gpio_key_vol_updown_pressed(void)
{
	u32 state1, state2;

	if ((vol_up_active_low < 0) || (vol_down_active_low < 0)) {
		pr_err("[%s]:vol_updown gpio or act_low is invalid!\n",
			__func__);
		return 0;
	}

	mdelay(KEY_PRESSED_SLEEP);
#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
	state1 = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA) & PMIC_KEYPAD_UP;
	state1 = ((state1 >> PMIC_KEYPAD_UP_DATA_BIT) ? PRESSED : UNPRESSED) ^
		(unsigned int)vol_up_active_low;
	state2 = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA) &
		PMIC_KEYPAD_DOWN;
	state2 = ((state2 >> PMIC_KEYPAD_DOWN_DATA_BIT) ? PRESSED : UNPRESSED) ^
		(unsigned int)vol_down_active_low;
#else
	state1 = (hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA) ?
		PRESSED : UNPRESSED) ^ (unsigned int)vol_up_active_low;
	state2 = (hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA) ?
		PRESSED : UNPRESSED) ^ (unsigned int)vol_down_active_low;
#endif
	if (state1 && state2)
		return PRESSED;

	return UNPRESSED;
}

#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
static void hisi_pmic_v900_clear_int(u32 num)
{
	/* num value is less than 31 */
	hisi_pmic_gpio_key_write((1 << num), PMIC_GPIO_KEY_INT);
}

static irqreturn_t hisi_pmic_gpio_key_irq_handler(int irq, void *dev_id)
{
	struct hisi_pmic_gpio_key *gpio_key =
		(struct hisi_pmic_gpio_key *)dev_id;
	u32 key_event;

	/* handle gpio key volume up & gpio key volume down event at here */
	/*
	 * 0:Volum up up key interrupt
	 * 1:Volum up down key interrupt
	 * 2:Volum down up key interrupt
	 * 3:Volum down down key interrupt
	 */
	if ((irq == gpio_key->gpiokey_irq[0]) ||
		(irq == gpio_key->gpiokey_irq[1])) {
		key_event = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
		if ((key_event & PMIC_KEYPAD_UP) == GPIO_LOW_VOLTAGE)
			gpio_key_vol_updown_press_set_bit(VOL_UP_BIT);
		else
			pmic_gpio_key_vol_updown_press_set_zero();

		mod_timer(&(gpio_key->key_up_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&volume_up_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
		if (irq == gpio_key->gpiokey_irq[0])
			hisi_pmic_v900_clear_int(PMIC_KEYPAD_UP_UP_INT);
		else
			hisi_pmic_v900_clear_int(PMIC_KEYPAD_UP_DOWN_INT);
	} else if ((irq == gpio_key->gpiokey_irq[2]) ||
			(irq == gpio_key->gpiokey_irq[3])) {
		key_event = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
		if ((key_event & PMIC_KEYPAD_DOWN) == GPIO_LOW_VOLTAGE)
			gpio_key_vol_updown_press_set_bit(VOL_DOWN_BIT);
		else
			pmic_gpio_key_vol_updown_press_set_zero();

		mod_timer(&(gpio_key->key_down_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&volume_down_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
		if (irq == gpio_key->gpiokey_irq[2])
			hisi_pmic_v900_clear_int(PMIC_KEYPAD_DOWN_UP_INT);
		else
			hisi_pmic_v900_clear_int(PMIC_KEYPAD_DOWN_DOWN_INT);
	} else {
		pr_err("[gpiokey] [%s]invalid irq %d!\n", __func__, irq);
	}
	return IRQ_HANDLED;
}
#else
static irqreturn_t hisi_pmic_gpio_key_irq_handler(int irq, void *dev_id)
{
	struct hisi_pmic_gpio_key *gpio_key =
		(struct hisi_pmic_gpio_key *)dev_id;
	u32 key_event;

	/* handle gpio key volume up & gpio key volume down event at here */
	if (irq == gpio_key->gpiokey_irq[0]) {
		key_event = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYUP_DATA);
		if (!key_event)
			gpio_key_vol_updown_press_set_bit(VOL_UP_BIT);
		else
			pmic_gpio_key_vol_updown_press_set_zero();

		mod_timer(&(gpio_key->key_up_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&volume_up_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
	} else if (irq == gpio_key->gpiokey_irq[1]) {
		key_event = hisi_pmic_gpio_key_read(PMIC_GPIO_KEYDOWN_DATA);
		if (!key_event)
			gpio_key_vol_updown_press_set_bit(VOL_DOWN_BIT);
		else
			pmic_gpio_key_vol_updown_press_set_zero();

		mod_timer(&(gpio_key->key_down_timer),
			jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		__pm_wakeup_event(&volume_down_key_lock,
			jiffies_to_msecs(KEY_IRQ_WAKEUP_INTERVAL));
	} else {
		pr_err("[gpiokey] [%s]invalid irq %d!\n", __func__, irq);
	}
	return IRQ_HANDLED;
}
#endif

static int hisi_pmic_gpio_key_irq_init(struct platform_device *pdev,
	struct hisi_pmic_gpio_key *gpio_key)
{
	int i, ret;

	for (i = 0; i < HISI_PMIC_GPIO_KEY_IRQ_CNT; i++) {
		gpio_key->gpiokey_irq[i] = platform_get_irq_byname(
			pdev, hisi_pmic_gpio_key_irq_table[i]);
		if (gpio_key->gpiokey_irq[i] < 0) {
			dev_err(&pdev->dev, "Failed to get %s irq number!\n",
				hisi_pmic_gpio_key_irq_table[i]);
			return -ENODEV;
		}

		ret = devm_request_irq(&pdev->dev, gpio_key->gpiokey_irq[i],
			hisi_pmic_gpio_key_irq_handler, IRQF_NO_SUSPEND,
			hisi_pmic_gpio_key_irq_table[i], gpio_key);
		if (ret) {
			dev_err(&pdev->dev, "Failed to request %s irq!\n",
				hisi_pmic_gpio_key_irq_table[i]);
			return -EINVAL;
		}
	}

	return 0;
}

#if defined(CONFIG_HISI_HI6421V900_PMU) || defined(CONFIG_HISI_HI6555V500_PMU)
static void hisi_pmic_gpio_key_reg_init(void)
{
	/* clear interrupt */
	hisi_pmic_gpio_key_write(PMIC_KEYUP_INT_CLEAR, PMIC_GPIO_KEY_INT);
}
#else
static void hisi_pmic_gpio_key_reg_init(void)
{
	/* 20ms filter */
	hisi_pmic_gpio_key_write(KEY_VALUE_HIGH, PMIC_GPIO_KEYUP_DEBSEL);
	hisi_pmic_gpio_key_write(KEY_VALUE_HIGH, PMIC_GPIO_KEYDOWN_DEBSEL);
	/* not PD */
	hisi_pmic_gpio_key_write(KEY_VALUE_LOW, PMIC_GPIO_KEYUP_PDSEL);
	hisi_pmic_gpio_key_write(KEY_VALUE_LOW, PMIC_GPIO_KEYDOWN_PDSEL);
	/* PU */
	hisi_pmic_gpio_key_write(KEY_VALUE_HIGH, PMIC_GPIO_KEYUP_PUSEL);
	hisi_pmic_gpio_key_write(KEY_VALUE_HIGH, PMIC_GPIO_KEYDOWN_PUSEL);
	/* input */
	hisi_pmic_gpio_key_write(KEY_VALUE_LOW, PMIC_GPIO_KEYUP_DIR);
	hisi_pmic_gpio_key_write(KEY_VALUE_LOW, PMIC_GPIO_KEYDOWN_DIR);
	/* edge trigger */
	hisi_pmic_gpio_key_write(KEY_VALUE_LOW, PMIC_GPIO_KEYUP_IS);
	hisi_pmic_gpio_key_write(KEY_VALUE_LOW, PMIC_GPIO_KEYDOWN_IS);
	/* double edge trigger */
	hisi_pmic_gpio_key_write(KEY_VALUE_HIGH, PMIC_GPIO_KEYUP_IBE);
	hisi_pmic_gpio_key_write(KEY_VALUE_HIGH, PMIC_GPIO_KEYDOWN_IBE);
}
#endif

#ifdef CONFIG_VOLUME_KEY_MASK
static ssize_t hisi_pmic_key_mask_read(struct file *file,
	char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[INT_STR_LEN] = {0};
	int ret;

	if (!user_buf) {
		pr_err("[gpiokey]user_buf is NULL!\n");
		return -EINVAL;
	}

	ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
		"%d", key_mask_flag);
	if (ret < 0) {
		pr_err("[%s]snprintf_s failed. ret=%d key_mask read:%d\n",
			__func__, ret, key_mask_flag);
		return ret;
	}

	pr_info("[%s]key_mask read:%d\n", __func__, key_mask_flag);

	return simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
}

static ssize_t hisi_pmic_key_mask_write(struct file *file,
	const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[MAX_BUF_LEN] = {0};
	size_t size;
	int mask = 0;

	size = min_t(size_t, count, WRITE_BUF_LEN);

	if (!user_buf) {
		pr_err("[gpiokey]user_buf is NULL!\n");
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, size))
		return 0;

	if (sscanf_s(buf, "%d", &mask) != 1) {
		pr_err("[gpiokey]sscanf_s fail!\n");
		return 0;
	}

	if ((mask == VOLUME_KEY_ENABLE_MASK) ||
		(mask == VOLUME_KEY_DISABLE_MASK))
		key_mask_ctl = mask;

	pr_info("[%s]key_mask write:%d\n", __func__, mask);

	return count;
}

static const struct file_operations key_mask_fops = {
	.read = hisi_pmic_key_mask_read,
	.write = hisi_pmic_key_mask_write,
};
#endif

static struct input_dev *hisi_pmic_gpio_key_new_input_dev(
	struct platform_device *pdev)
{
	struct input_dev *input_dev = NULL;

	input_dev = input_allocate_device();
	if (!input_dev) {
		pr_err("[gpiokey]allocate input device fail\n");
		return NULL;
	}

	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(KEY_VOLUMEUP, input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, input_dev->keybit);

	input_dev->open = hisi_pmic_gpio_key_open;
	input_dev->close = hisi_pmic_gpio_key_close;

	return input_dev;
}

static struct hisi_pmic_gpio_key *hisi_gpio_key_new_drvdata(
	struct platform_device *pdev)
{
	struct hisi_pmic_gpio_key *gpio_key = NULL;
	struct input_dev *input_dev = NULL;

	gpio_key = devm_kzalloc(
		&pdev->dev, sizeof(struct hisi_pmic_gpio_key), GFP_KERNEL);
	if (!gpio_key)
		return NULL;

	input_dev = hisi_pmic_gpio_key_new_input_dev(pdev);
	gpio_key->input_dev = input_dev;
	if (!gpio_key->input_dev) {
		pr_err("[gpiokey]new input dev fail\n");
		return NULL;
	}

	input_set_drvdata(input_dev, gpio_key);

	return gpio_key;
}

static void hisi_pmic_gpio_key_init_work(struct hisi_pmic_gpio_key *gpio_key)
{
	INIT_DELAYED_WORK(
		&(gpio_key->gpio_keyup_work), hisi_pmic_gpio_keyup_work);
	INIT_DELAYED_WORK((struct delayed_work *)(uintptr_t)
		(&(gpio_key->gpio_keydown_work)),
		hisi_pmic_gpio_keydown_work);
	wakeup_source_init(&volume_down_key_lock, "key_down_wake_lock");
	wakeup_source_init(&volume_up_key_lock, "key_up_wake_lock");

	vol_up_active_low = GPIO_KEY_PRESS;
	vol_down_active_low = GPIO_KEY_PRESS;

#if defined(CONFIG_HUAWEI_DSM)
	/* initialize the statistic variable */
	volume_up_press_count = 0;
	volume_down_press_count = 0;
	volume_up_last_press_time = 0;
	volume_down_last_press_time = 0;
#endif
}

static void hisi_pmic_gpio_key_setup_timer(struct hisi_pmic_gpio_key *gpio_key)
{
	setup_timer(&(gpio_key->key_up_timer), hisi_pmic_gpio_keyup_timer,
		(uintptr_t)gpio_key);
	setup_timer(&(gpio_key->key_down_timer), hisi_pmic_gpio_keydown_timer,
		(uintptr_t)gpio_key);

#if defined(CONFIG_HUAWEI_DSM)
	setup_timer(&dsm_gpio_key_timer, dsm_gpio_key_timer_func,
		(uintptr_t)gpio_key);
#endif
}

static int hisi_pmic_gpio_key_probe(struct platform_device *pdev)
{
	struct hisi_pmic_gpio_key *gpio_key = NULL;
	int err;

	dev_info(&pdev->dev, "hisi gpio key driver probes start!\n");

	gpio_key = hisi_gpio_key_new_drvdata(pdev);
	if (!gpio_key) {
		dev_err(&pdev->dev, "new driver data fail\n");
		return -ENOMEM;
	}

	/* initial work before we use it. */
	hisi_pmic_gpio_key_init_work(gpio_key);

	hisi_pmic_gpio_key_setup_timer(gpio_key);

	hisi_pmic_gpio_key_reg_init();

	err = input_register_device(gpio_key->input_dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to register input device!\n");
		goto err_register_dev;
	}

	device_init_wakeup(&pdev->dev, TRUE);
	platform_set_drvdata(pdev, gpio_key);

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

	err = hisi_pmic_gpio_key_irq_init(pdev, gpio_key);
	if (err) {
		dev_err(&pdev->dev, "Failed to request irq!\n");
		goto err_request_irq;
	}
	dev_info(&pdev->dev, "hisi gpio key driver probes successfully!\n");

	return 0;

err_request_irq:
	platform_set_drvdata(pdev, NULL);
	input_unregister_device(gpio_key->input_dev);

err_register_dev:
	wakeup_source_trash(&volume_down_key_lock);
	wakeup_source_trash(&volume_up_key_lock);

	if (gpio_key->input_dev)
		input_free_device(gpio_key->input_dev);

	pr_info("[gpiokey] gpio key probe failed! ret = %d\n", err);

	return err;
}

static int hisi_pmic_gpio_key_remove(struct platform_device *pdev)
{
	struct hisi_pmic_gpio_key *gpio_key = platform_get_drvdata(pdev);
	bool ret = true;

	if (!gpio_key) {
		pr_err("get invalid gpio_key pointer\n");
		return -EINVAL;
	}

	ret = cancel_delayed_work(&(gpio_key->gpio_keyup_work));
	if (!ret)
		pr_err("%s:cancel gpio key up work failed\n", __func__);

	ret = cancel_delayed_work(&(gpio_key->gpio_keydown_work));
	if (!ret)
		pr_err("%s:cancel gpio key down work failed\n", __func__);

	wakeup_source_trash(&volume_down_key_lock);
	wakeup_source_trash(&volume_up_key_lock);

	input_unregister_device(gpio_key->input_dev);
	platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_VOLUME_KEY_MASK
	remove_proc_entry("key_mask", 0);
#endif

	return 0;
}

#ifdef CONFIG_PM
static int hisi_pmic_gpio_key_suspend(
	struct platform_device *pdev, pm_message_t state)
{
	dev_info(&pdev->dev, "%s: suspend +\n", __func__);
	dev_info(&pdev->dev, "%s: suspend -\n", __func__);

	return 0;
}

static int hisi_pmic_gpio_key_resume(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s: resume +\n", __func__);
	dev_info(&pdev->dev, "%s: resume -\n", __func__);

	return 0;
}
#endif

static const struct of_device_id hisi_pmic_gpio_key_match[] = {
	{
		.compatible = "hisilicon,pmic-gpio-key",
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_pmic_gpio_key_match);

struct platform_driver hisi_pmic_gpio_key_driver = {
	.probe = hisi_pmic_gpio_key_probe,
	.remove = hisi_pmic_gpio_key_remove,
	.driver = {
		.name = "hisi_pmic_gpio_key",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_pmic_gpio_key_match),
	},
#ifdef CONFIG_PM
	.suspend = hisi_pmic_gpio_key_suspend,
	.resume = hisi_pmic_gpio_key_resume,
#endif
};

module_platform_driver(hisi_pmic_gpio_key_driver);

MODULE_DESCRIPTION("Hisilicon keypad platform driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
