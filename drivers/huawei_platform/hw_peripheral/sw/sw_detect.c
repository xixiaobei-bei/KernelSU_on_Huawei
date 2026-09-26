/*
 * sw_detect.c
 *
 * single wire device detect
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

#include "sw_detect.h"
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/device.h>
#include <linux/delay.h>
#include "sw_debug.h"

#define GPIO_VAL_HIGH           1
#define GPIO_VAL_LOW            0
#define DEFAULT_KB_ADC_CHANNEL  7
#define KB_WAKE_TIMEOUT         (400 * HZ)

static int sw_gpio_detect_notifier_register(
	struct sw_gpio_detector *detector, struct notifier_block *nb)
{
	int ret;

	if (!detector || !nb) {
		sw_print_err("notifier head not yet init\n");
		return -FAILURE;
	}

	ret = blocking_notifier_chain_register(
		&detector->detect_notifier, nb);

	wakeup_source_init(&detector->kb_wlock, "kb wakelock");
	return ret;
}

static int sw_gpio_detect_notifier_unregister(
	struct sw_gpio_detector *detector, struct notifier_block *nb)
{
	int ret;

	if (!detector || !nb) {
		sw_print_err("notifier head not yet init\n");
		return -FAILURE;
	}

	ret = blocking_notifier_chain_unregister(
		&detector->detect_notifier, nb);

	wakeup_source_trash(&detector->kb_wlock);
	return ret;
}

static void sw_gpio_detect_notify(struct sw_gpio_detector *detector)
{
	sw_print_info("enter\n");
	blocking_notifier_call_chain(&detector->detect_notifier,
		detector->dev_state, detector);
	sw_print_info("exit\n");
}

static void sw_detect_int_disable_irq(struct sw_gpio_detector *detector)
{
	unsigned long flags;

	spin_lock_irqsave(&detector->irq_enabled_lock, flags);
	if (detector->irq_enabled) {
		disable_irq_nosync(detector->detect_int_irq);
		detector->irq_enabled = false;
	}
	spin_unlock_irqrestore(&detector->irq_enabled_lock, flags);
}

static void sw_detect_int_enable_irq(struct sw_gpio_detector *detector)
{
	unsigned long flags;

	spin_lock_irqsave(&detector->irq_enabled_lock, flags);
	if (!detector->irq_enabled) {
		enable_irq(detector->detect_int_irq);
		detector->irq_enabled = true;
	}
	spin_unlock_irqrestore(&detector->irq_enabled_lock, flags);
}

static void sw_enable_detectintirq(struct sw_gpio_detector *detector,
	bool enable)
{
	if (enable)
		sw_detect_int_enable_irq(detector);
	else
		sw_detect_int_disable_irq(detector);
}

static irqreturn_t sw_detect_int_irq(int irq, void *dev_data)
{
	struct sw_gpio_detector *detector = dev_data;
	int gp;

	if (!detector) {
		sw_print_info("detector is null\n");
		return IRQ_HANDLED;
	}

	if (detector->start_detect == DETECT_OFF) {
		sw_print_info("detector->start_detect = 0\n");
		return IRQ_HANDLED;
	}
	gp = gpio_get_value(detector->detect_int_gpio);
	sw_print_info("sw_wakeup_irq gpio %x\n", gp);

	sw_detect_int_disable_irq(detector);
	schedule_work(&detector->irq_work);

	return IRQ_HANDLED;
}

/*
 * handler for detect irq
 * detect by charge detector if defined
 * detect by keyboard detector if defined
 */
static void sw_kb_detect_irq_work(struct work_struct *work)
{
	int cur_devstate;
	struct sw_gpio_detector *detector = container_of(work,
		struct sw_gpio_detector, irq_work);

	if (!detector->kb_wlock.active)
		__pm_wakeup_event(&detector->kb_wlock,
			jiffies_to_msecs(KB_WAKE_TIMEOUT));

	cur_devstate = detector->dev_state;

	if (detector->chg_detecor)
		detector->chg_detecor->detect_call(detector,
			detector->chg_detecor);

	/*
	 * 1.detect pogopin chg, so do not need detect kb
	 * 2.if pre is pogopin chg, future irq is pogopin chg disconnect,
	 * so do not need detect kb
	 */
	if ((detector->dev_state == DEVSTAT_CHGDEV_ONLINE) ||
		(cur_devstate == DEVSTAT_CHGDEV_ONLINE)) {
		sw_print_info("in pogopin not detect kb,cur = %d,now = %d\n",
			cur_devstate, detector->dev_state);
		if (cur_devstate != detector->dev_state)
			sw_gpio_detect_notify(detector);
		if (detector->kb_wlock.active)
			__pm_relax(&detector->kb_wlock);
		return;
	}

	if (detector->kb_detecor)
		detector->kb_detecor->detect_call(detector,
			detector->kb_detecor);

	if (cur_devstate != detector->dev_state)
		sw_gpio_detect_notify(detector);

	if (detector->kb_wlock.active)
		__pm_relax(&detector->kb_wlock);
}

static void sw_setup_kbint_config(struct sw_gpio_detector *pdev)
{
	int irq;
	int ret;

	if (!pdev)
		return;

	if (!gpio_is_valid(pdev->detect_int_gpio)) {
		sw_print_err("detect_int_gpio no valid\n");
		pdev->detect_int_gpio = INVALID_VALUE;
		return;
	}

	irq = gpio_to_irq(pdev->detect_int_gpio);
	if (irq < 0) {
		sw_print_err("detect_int_gpio gpio_to_irq fail %x\n", irq);
		goto out_free;
	}

	ret = request_irq(irq, sw_detect_int_irq,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		"sw_kb_int", pdev);
	if (ret) {
		sw_print_err("detect_int_gpio request_irq fail\n");
		goto out_free;
	}
	pdev->detect_int_irq = irq;

	/*
	 * default disable irq ,
	 * because sensorhub maybe not ready when sys start
	 */
	sw_detect_int_disable_irq(pdev);

	return;

out_free:
	gpio_free(pdev->detect_int_gpio);
	pdev->detect_int_gpio = INVALID_VALUE;
}

int sw_get_named_gpio(struct device_node *np,
	const char *propname, enum gpiod_flags flags)
{
	int gpio;
	int ret;

	if (!np || !propname)
		return -FAILURE;

	gpio = of_get_named_gpio(np, propname, 0);
	if (gpio == -EPROBE_DEFER)
		gpio = of_get_named_gpio(np, propname, 0);

	if (!gpio_is_valid(gpio)) {
		sw_print_err("get gpio [%s] fail\n", propname);
		return -FAILURE;
	}

	ret = gpio_request(gpio, propname);
	if (ret < 0) {
		sw_print_err("request gpio [%s] fail\n", propname);
		gpio_free(gpio);
		return -FAILURE;
	}

	if (flags == GPIOD_OUT_LOW)
		ret = gpio_direction_output(gpio, GPIO_VAL_LOW);
	else if (flags == GPIOD_OUT_HIGH)
		ret = gpio_direction_output(gpio, GPIO_VAL_HIGH);
	else if (flags == GPIOD_IN)
		ret = gpio_direction_input(gpio);

	if (ret < 0) {
		sw_print_err("set gpio [%s] flags fail\n", propname);
		gpio_free(gpio);
		return -FAILURE;
	}

	return gpio;
}

static int sw_parse_devtree(struct sw_gpio_detector *detector)
{
	struct device_node *np = NULL;
	u32 val;
	int ret;

	/* should be the same as dts node compatible property */
	np = of_find_compatible_node(NULL, NULL, "huawei,sw_kb");
	if (!np) {
		sw_print_err("unable to find %s\n", "huawei,sw_kb");
		return -ENOENT;
	}

	/* read adc channel */
	if (of_property_read_u32(np, "adc_kb_detect", &val)) {
		sw_print_err("dts:can not get adc_kb_detect\n");
		val = DEFAULT_KB_ADC_CHANNEL;
	}
	detector->detect_adc_no = val;

	/* read id int gpio */
	detector->detect_int_gpio = sw_get_named_gpio(np,
		"gpio_kb_int", GPIOD_IN);
	sw_setup_kbint_config(detector);
	if (detector->detect_int_gpio < 0) {
		sw_print_err("detect_int_gpio failed\n");
		ret = -EINVAL;
		goto err_free_gpio;
	}

	/* read kb detect */
	detector->kb_detecor = sw_load_kb_detect(np);
	if (!detector->kb_detecor) {
		ret = -EINVAL;
		goto err_free_irq;
	}

	/* read chg detect */
	if (of_property_read_u32(np, "enable_chg", &val)) {
		sw_print_err("dts:can not get enable_chg\n");
		detector->chg_detecor = NULL;
	} else {
		detector->chg_detecor = sw_load_chg_detect(np, val);
	}

	return SUCCESS;

err_free_irq:
	free_irq(detector->detect_int_irq, detector);
	detector->detect_int_irq = INVALID_VALUE;

err_free_gpio:
	if (detector->detect_int_gpio >= 0) {
		gpio_free(detector->detect_int_gpio);
		detector->detect_int_gpio = INVALID_VALUE;
	}

	return ret;
}

static void sw_gpio_notifyevent(struct sw_gpio_detector *sw_detector,
	unsigned long event, void *data)
{
	if (sw_detector->chg_detecor &&
		sw_detector->chg_detecor->event_call)
		sw_detector->chg_detecor->event_call(sw_detector,
			sw_detector->chg_detecor, event, data);

	if (sw_detector->kb_detecor &&
		sw_detector->kb_detecor->event_call)
		sw_detector->kb_detecor->event_call(sw_detector,
			sw_detector->kb_detecor, event, data);
}

static void sw_gpio_detect_exit(struct sw_gpio_detector *detector,
	void *data)
{
	struct sw_detector_ops *ops = (struct sw_detector_ops *)data;

	if (ops)
		sw_gpio_detect_notifier_unregister(detector, &ops->detect_nb);

	if (detector->detect_int_irq >= 0) {
		free_irq(detector->detect_int_irq, detector);
		detector->detect_int_irq = INVALID_VALUE;
	}

	if (detector->detect_int_gpio >= 0) {
		gpio_free(detector->detect_int_gpio);
		detector->detect_int_gpio = INVALID_VALUE;
	}

	if (detector->kb_detecor &&
		detector->kb_detecor->event_call) {
		detector->kb_detecor->event_call(detector,
			detector->kb_detecor, SW_NOTIFY_EVENT_DESTROY, NULL);
		detector->kb_detecor = NULL;
	}

	if (detector->chg_detecor &&
		detector->chg_detecor->event_call) {
		detector->chg_detecor->event_call(detector,
			detector->chg_detecor, SW_NOTIFY_EVENT_DESTROY, NULL);
		detector->chg_detecor = NULL;
	}
	kfree(detector);
}

static void sw_gpio_detect_do_events(void *detector,
	unsigned long event, void *data)
{
	struct sw_gpio_detector *sw_detector = detector;

	if (!sw_detector)
		return;

	switch (event) {
	case SW_NOTIFY_EVENT_DISCONNECT:
		sw_gpio_notifyevent(sw_detector, event, data);
		break;
	case SW_NOTIFY_EVENT_REDETECT:
		schedule_work(&sw_detector->irq_work);
		break;
	case SW_NOTIFY_EVENT_DESTROY:
		sw_gpio_detect_exit(sw_detector, data);
		break;
	default:
		break;
	}
}

int sw_gpio_detect_register(struct platform_device *pdev,
	struct sw_detector_ops *ops)
{
	struct sw_gpio_detector *detector = NULL;
	int ret;

	SW_PRINT_FUNCTION_NAME;
	if (!pdev || !ops) {
		sw_print_err("param failed\n");
		return -EINVAL;
	}

	detector = kzalloc(sizeof(*detector), GFP_KERNEL);
	if (!detector)
		return -ENOMEM;

	ops->detector = detector;
	ops->notify = sw_gpio_detect_do_events;

	/* init kb int irq control */
	spin_lock_init(&detector->irq_enabled_lock);
	detector->irq_enabled = true;
	detector->control_irq = sw_enable_detectintirq;

	ret = sw_parse_devtree(detector);
	if (ret < 0) {
		sw_print_err("dts parse failed\n");
		goto err_core_init;
	}

	INIT_WORK(&detector->irq_work, sw_kb_detect_irq_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&detector->detect_notifier);
	sw_gpio_detect_notifier_register(detector, &ops->detect_nb);

	return SUCCESS;

err_core_init:
	kfree(detector);
	return ret;
}
