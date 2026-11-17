/*
 * da_combine_mbhc_rear_jack.c
 *
 * da_combine mbhc rear jack driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#define LOG_TAG "da_combine_mbhc_rear_jack"

#include <linux/hisi/da_combine/da_combine_mbhc_rear_jack.h>

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <sound/jack.h>
#include <linux/hisi/audio_log.h>

#define REAR_JACK_GPIOS_MAXNUM 3

struct rear_jack_detect {
	int jack_num;
	struct gpio_desc *gpios[REAR_JACK_GPIOS_MAXNUM];

	bool is_rear_jack_init;
	int jack_pins_mask;
	struct snd_soc_jack_pin jack_pins[REAR_JACK_GPIOS_MAXNUM];
	struct snd_soc_jack jack;

	struct gpio_desc *headphone_switch_gpio;
	struct gpio_desc *head_mic_switch_gpio;
};

static struct rear_jack_detect jack_detect;

static void rear_jack_report(struct rear_jack_detect *rear_jd)
{
	int i;
	int value;
	int report_value = 0;

	if (!rear_jd->is_rear_jack_init) {
		AUDIO_LOGW("the rear jack is not initialized");
		return;
	}

	for (i = 0; i < rear_jd->jack_num; ++i) {
		value = gpiod_get_value(rear_jd->gpios[i]);
		AUDIO_LOGI("gpio%u get value is %d", desc_to_gpio(rear_jd->gpios[i]), value);

		if (value == 0)
			report_value |= rear_jd->jack_pins[i].mask;
	}

	snd_soc_jack_report(&rear_jd->jack, report_value, rear_jd->jack_pins_mask);
}

static irqreturn_t rear_jack_irq_handler(int irq, void *data)
{
	msleep(300);

	rear_jack_report(data);

	return IRQ_HANDLED;
}

static struct gpio_desc *rear_jack_irq_init_one(struct device *dev, const char *prop_name, const char *label,
	struct rear_jack_detect *rear_jd)
{
	int irq;
	int ret;
	struct gpio_desc *desc = devm_gpiod_get_optional(dev, prop_name, GPIOD_IN);

	if (IS_ERR_OR_NULL(desc)) {
		AUDIO_LOGW("get gpiod prop %s failed with value %ld", prop_name, PTR_ERR(desc));
		return NULL;
	}

	irq = gpiod_to_irq(desc);
	if (irq < 0) {
		AUDIO_LOGE("gpio%d to irq failed with error %d", desc_to_gpio(desc), irq);
		return NULL;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL, rear_jack_irq_handler,
		IRQF_ONESHOT | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, label, rear_jd);
	if (ret != 0) {
		AUDIO_LOGE("request gpio%d irq %d failed with error %d", desc_to_gpio(desc), irq, ret);
		return NULL;
	}

	return desc;
}

struct rear_jack_prop_t {
	const char *prop_name;
	struct snd_soc_jack_pin pin;
};

static struct rear_jack_prop_t rear_jack_prop[REAR_JACK_GPIOS_MAXNUM] = {
	{
		.prop_name = "line_in_rear_jd",
		.pin = {
			.pin = "LineIn",
			.mask = SND_JACK_LINEIN,
		}
	}, {
		.prop_name = "hp_out_rear_jd",
		.pin = {
			.pin = "Rear HeadPhone",
			.mask = SND_JACK_HEADPHONE,
		}
	}, {
		.prop_name = "mic_in_rear_jd",
		.pin = {
			.pin = "Rear Mic",
			.mask = SND_JACK_MICROPHONE,
		}
	}
};

static void rear_jack_irq_init(struct device *dev,
	struct rear_jack_detect *rear_jd)
{
	int i;
	struct gpio_desc *desc = NULL;
	struct rear_jack_prop_t *jack = NULL;

	rear_jd->jack_num = 0;
	rear_jd->jack_pins_mask = 0;

	for (i = 0; i < REAR_JACK_GPIOS_MAXNUM; ++i) {
		jack = &rear_jack_prop[i];

		desc = rear_jack_irq_init_one(dev, jack->prop_name, jack->pin.pin, rear_jd);
		if (desc == NULL)
			continue;

		rear_jd->gpios[rear_jd->jack_num] = desc;
		rear_jd->jack_pins[rear_jd->jack_num].pin = jack->pin.pin;
		rear_jd->jack_pins[rear_jd->jack_num].mask = jack->pin.mask;
		rear_jd->jack_pins_mask |= jack->pin.mask;

		rear_jd->jack_num++;
	}
}

static void headset_switch_gpio_init(struct device *dev, struct gpio_desc **gpiod, const char *prop_name)
{
	struct gpio_desc *desc = devm_gpiod_get_optional(dev, prop_name, GPIOD_OUT_LOW);

	if (IS_ERR_OR_NULL(desc)) {
		AUDIO_LOGW("gpiod get prop %s failed with value %ld", prop_name, PTR_ERR(desc));
	} else {
		*gpiod = desc;
		AUDIO_LOGI("init gpio%u value to 0", desc_to_gpio(desc));
	}
}

static void set_gpiod_switch_value(struct gpio_desc *desc, int value)
{
	if (desc == NULL)
		return;

	gpiod_set_value(desc, value);
	AUDIO_LOGI("set gpio%u value to %d", desc_to_gpio(desc), value);
}

void da_combine_set_headphone_switch_gpio_value(int value)
{
	set_gpiod_switch_value(jack_detect.headphone_switch_gpio, value);
}

void da_combine_set_head_mic_switch_gpio_value(int value)
{
	set_gpiod_switch_value(jack_detect.head_mic_switch_gpio, value);
}

void rear_jack_init(struct snd_soc_component *codec)
{
	int ret;
	struct rear_jack_detect *rear_jd = &jack_detect;

	if (codec == NULL)
		return;
	if (!of_property_read_bool(codec->dev->of_node, "single_kcontrol_route_rear_jack"))
		return;
	if (!devres_open_group(codec->dev, NULL, GFP_KERNEL))
		return;

	rear_jack_irq_init(codec->dev, rear_jd);
	if (rear_jd->jack_num == 0) {
		devres_remove_group(codec->dev, NULL);
		return;
	}

	ret = snd_soc_card_jack_new(codec->card, "Rear Jack", rear_jd->jack_pins_mask, &rear_jd->jack,
		rear_jd->jack_pins, rear_jd->jack_num);
	if (ret != 0) {
		devres_release_group(codec->dev, NULL);
		AUDIO_LOGE("jack new failed with error %d", ret);
		return;
	}
	devres_remove_group(codec->dev, NULL);
	headset_switch_gpio_init(codec->dev, &rear_jd->headphone_switch_gpio, "headphone_switch");
	headset_switch_gpio_init(codec->dev, &rear_jd->head_mic_switch_gpio, "head_mic_switch");

	rear_jd->is_rear_jack_init = true;
	rear_jack_report(rear_jd);

	AUDIO_LOGI("rear jack init done");
}

void da_combine_rear_jack_plug_in_detect_wrapper(void)
{
	rear_jack_report(&jack_detect);
}
