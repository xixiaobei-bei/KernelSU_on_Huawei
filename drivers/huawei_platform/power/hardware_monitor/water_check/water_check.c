/*
 * water_check.c
 *
 * water check monitor driver
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/pinctrl/consumer.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG water_check
HWLOG_REGIST();

#define WATER_IN                      1
#define WATER_NULL                    0
#define WATER_CHECK_PARA_LEVEL        8
#define SINGLE_POSITION               1
#define WATERCHECK_DMDLOG_SIZE        60
#define WATER_CHECK_DSM_IGNORE_NUM    99
#define DEBOUNCE_TIME                 3000
#define DSM_NEED_REPORT               1
#define DSM_REPORTED                  0
#define GPIO_NAME_SIZE                16
#define BATFET_DISABLED_ACTION        1
#define DECIMAL_BASE                  10

enum water_check_core_para_info {
	WATER_CHECK_TYPE = 0,
	WATER_CHECK_GPIO_NAME,
	WATER_CHECK_IRQ_NO,
	WATER_MULTIPLE_HANDLE,
	WATER_DMD_NO_OFFSET,
	WATER_IS_PROMPT,
	WATER_CHECK_ACTION,
	WATER_CHECK_PARA_TOTAL,
};

struct water_check_core_para {
	int check_type;
	char gpio_name[GPIO_NAME_SIZE];
	int irq_no;
	int dmd_no_offset;
	int gpio_no;
	u8 multiple_handle;
	u8 prompt;
	u8 action;
};

struct water_check_core_data {
	int total_type;
	struct water_check_core_para para[WATER_CHECK_PARA_LEVEL];
};

struct water_check_info {
	struct device *dev;
	u32 need_pinctrl_config;
	struct pinctrl *pinctrl;
	struct delayed_work water_detect_work;
	u8 last_check_status[WATER_CHECK_PARA_LEVEL];
	u8 dsm_report_status[WATER_CHECK_PARA_LEVEL];
	char dsm_buff[WATERCHECK_DMDLOG_SIZE];
	struct water_check_core_data data;
};

static struct water_check_info *g_info;
static struct wake_lock g_wc_wakelock;

int is_water_intrused(void)
{
	int i;
	int gpio_value;
	struct water_check_info *info = g_info;

	if (!info) {
		hwlog_info("info is null\n");
		return WATER_NULL;
	}

	for (i = 0; i < info->data.total_type; i++) {
		if (!strncmp(info->data.para[i].gpio_name,
			"usb", strlen("usb"))) {
			gpio_value = gpio_get_value(info->data.para[i].gpio_no);
			if (!gpio_value) {
				hwlog_info("water is detected in usb\n");
				return WATER_IN;
			}
		}
	}

	return WATER_NULL;
}

static void water_intruded_handle(int index, struct water_check_info *info)
{
	char s_buff[WATERCHECK_DMDLOG_SIZE] = { 0 };

	if (index < 0 || index > info->data.total_type) {
		hwlog_err("index is invalid\n");
		return;
	}

	/* dsm report */
	if (info->data.para[index].dmd_no_offset !=
		WATER_CHECK_DSM_IGNORE_NUM) {
		if (info->data.para[index].check_type == SINGLE_POSITION) {
			memset(s_buff, 0, WATERCHECK_DMDLOG_SIZE);
			snprintf(s_buff, WATERCHECK_DMDLOG_SIZE,
				"water check is triggered in: %s",
				info->data.para[index].gpio_name);
			hwlog_info("single_position dsm_buff:%s\n", s_buff);

			/* single position dsm report one time */
			power_dsm_dmd_report(POWER_DSM_BATTERY,
				ERROR_NO_WATER_CHECK_BASE +
				info->data.para[index].dmd_no_offset,
				s_buff);

			msleep(150); /* delay 150ms */
			info->dsm_report_status[index] = DSM_REPORTED;
		} else {
			hwlog_info("multiple_intruded dsm_buff:%s\n",
				info->dsm_buff);
			power_dsm_dmd_report(POWER_DSM_BATTERY,
				ERROR_NO_WATER_CHECK_BASE +
				info->data.para[index].dmd_no_offset,
				info->dsm_buff);
		}
	}

	/* other functions */
	switch (info->data.para[index].action) {
	case BATFET_DISABLED_ACTION:
		hwlog_info("charge set batfet to disable\n");
		msleep(50); /* delay 50ms */
		charge_set_batfet_disable(true);
		break;

	default:
		break;
	}
}

static void water_detect_work(struct work_struct *work)
{
	int i;
	u8 gpio_val;
	int water_intruded_num = 0;
	struct water_check_info *info = NULL;

	hwlog_info("detect_work start\n");

	if (!work) {
		hwlog_err("work is null\n");
		goto end_enable_irq;
	}

	info = container_of(work, struct water_check_info,
		water_detect_work.work);
	if (!info) {
		hwlog_err("info is null\n");
		goto end_enable_irq;
	}

	memset(&info->dsm_buff, 0, WATERCHECK_DMDLOG_SIZE);
	snprintf(info->dsm_buff, WATERCHECK_DMDLOG_SIZE,
		"water check is triggered in: ");

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].check_type != SINGLE_POSITION)
			continue;

		gpio_val = gpio_get_value_cansleep(info->data.para[i].gpio_no);
		if (!gpio_val) {
			if (info->data.para[i].multiple_handle) {
				water_intruded_num++;
				snprintf(info->dsm_buff +
					strlen(info->dsm_buff),
					WATERCHECK_DMDLOG_SIZE,
					"%s ",
					info->data.para[i].gpio_name);
			}

			if ((gpio_val ^ info->last_check_status[i]) ||
				info->dsm_report_status[i])
				water_intruded_handle(i, info);
		}

		if (gpio_val)
			info->dsm_report_status[i] = DSM_NEED_REPORT;

		info->last_check_status[i] = gpio_val;
	}

	/* multiple intruded */
	if (water_intruded_num > SINGLE_POSITION) {
		for (i = 0; i < info->data.total_type; i++) {
			if (water_intruded_num == info->data.para[i].check_type)
				water_intruded_handle(i, info);
		}
	}

end_enable_irq:
	hwlog_info("detect_work end\n");

	/* enable irq */
	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].check_type == SINGLE_POSITION)
			enable_irq(info->data.para[i].irq_no);
	}

	wake_unlock(&g_wc_wakelock);
}

static irqreturn_t water_check_irq_handler(int irq, void *p)
{
	int i;
	struct water_check_info *info = (struct water_check_info *)p;

	if (!info) {
		hwlog_err("info is null\n");
		return IRQ_NONE;
	}

	hwlog_info("irq_handler irq_no:%d\n", irq);

	wake_lock(&g_wc_wakelock);

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].check_type == SINGLE_POSITION)
			disable_irq_nosync(info->data.para[i].irq_no);
	}

	schedule_delayed_work(&info->water_detect_work,
		msecs_to_jiffies(DEBOUNCE_TIME));

	return IRQ_HANDLED;

}

static int water_check_gpio_config(struct water_check_info *info)
{
	int i;
	int ret;

	/* request gpio & register interrupt handler function */
	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].check_type != SINGLE_POSITION)
			continue;

		ret = devm_gpio_request_one(info->dev,
			(unsigned int)info->data.para[i].gpio_no,
			(unsigned long)GPIOF_DIR_IN,
			info->data.para[i].gpio_name);
		if (ret < 0) {
			hwlog_err("gpio request fail\n");
			goto fail_request_gpio;
		}

		info->data.para[i].irq_no =
			gpio_to_irq(info->data.para[i].gpio_no);
		if (info->data.para[i].irq_no < 0) {
			hwlog_err("gpio map to irq fail\n");
			ret = -EINVAL;
			goto fail_map_irq;
		}

		info->last_check_status[i] =
			gpio_get_value_cansleep(info->data.para[i].gpio_no);
		info->dsm_report_status[i] = DSM_NEED_REPORT;

		ret = devm_request_irq(info->dev,
			(unsigned int)info->data.para[i].irq_no,
			water_check_irq_handler,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
			IRQF_NO_SUSPEND | IRQF_ONESHOT,
			info->data.para[i].gpio_name, info);
		if (ret < 0) {
			hwlog_err("gpio irq request fail\n");
			goto fail_request_irq;
		}

		disable_irq_nosync(info->data.para[i].irq_no);
	}

	for (i = 0; i < info->data.total_type; i++) {
		if (info->data.para[i].check_type != SINGLE_POSITION)
			continue;

		hwlog_info("gpio_name:%s irq_no:%d gpio_sts:%d dsm_sts:%d\n",
		info->data.para[i].gpio_name,
		info->data.para[i].irq_no,
		info->last_check_status[i],
		info->dsm_report_status[i]);
	}

	return 0;

fail_request_irq:
fail_map_irq:
fail_request_gpio:
	return ret;
}

static int water_check_parse_extra_dts(struct device_node *np,
	struct water_check_info *info)
{
	int i;
	int j;
	int ret;
	int array_len;
	const char *tmp_string = NULL;
	char gpio_name[GPIO_NAME_SIZE] = { 0 };
	int gpio_no;

	/* parse water_check para */
	array_len = of_property_count_strings(np, "water_check_para");
	hwlog_info("array_len=%d\n", array_len);

	if ((array_len <= 0) ||
		(array_len % WATER_CHECK_PARA_TOTAL != 0) ||
		(array_len > WATER_CHECK_PARA_TOTAL * WATER_CHECK_PARA_LEVEL)) {
		info->data.total_type = 0;
		hwlog_err("water_check_para invalid\n");
		return -EINVAL;
	}

	info->data.total_type = array_len / WATER_CHECK_PARA_TOTAL;
	hwlog_info("total_type=%d\n", info->data.total_type);

	for (i = 0; i < info->data.total_type; i++) {
		for (j = 0; j < WATER_CHECK_PARA_TOTAL; j++) {
			ret = of_property_read_string_index(np,
				"water_check_para",
				i * WATER_CHECK_PARA_TOTAL + j, &tmp_string);
			if (ret) {
				hwlog_err("water_check_para dts read failed\n");
				return -EINVAL;
			}

			switch (j) {
			case WATER_CHECK_TYPE:
				ret = kstrtoint(tmp_string, DECIMAL_BASE,
					&info->data.para[i].check_type);
				if (ret) {
					hwlog_err("get kstrtoint failed\n");
					return -EINVAL;
				}
				break;

			case WATER_CHECK_GPIO_NAME:
				strncpy(info->data.para[i].gpio_name,
					tmp_string, GPIO_NAME_SIZE);

				if (info->data.para[i].check_type !=
					SINGLE_POSITION)
					break;

				memset(gpio_name, 0, GPIO_NAME_SIZE);
				snprintf(gpio_name, sizeof(gpio_name),
					"gpio_%s",
					info->data.para[i].gpio_name);

				gpio_no = of_get_named_gpio(np, gpio_name, 0);
				info->data.para[i].gpio_no = gpio_no;
				if (!gpio_is_valid(gpio_no)) {
					hwlog_err("gpio is not valid\n");
					return -EINVAL;
				}
				break;

			case WATER_CHECK_IRQ_NO:
				ret = kstrtoint(tmp_string, DECIMAL_BASE,
					&info->data.para[i].irq_no);
				if (ret) {
					hwlog_err("get kstrtoint failed\n");
					return -EINVAL;
				}
				break;

			case WATER_MULTIPLE_HANDLE:
				ret = kstrtou8(tmp_string, DECIMAL_BASE,
					&info->data.para[i].multiple_handle);
				if (ret) {
					hwlog_err("get kstrtou8 failed\n");
					return -EINVAL;
				}
				break;

			case WATER_DMD_NO_OFFSET:
				ret = kstrtoint(tmp_string, DECIMAL_BASE,
					&info->data.para[i].dmd_no_offset);
				if (ret) {
					hwlog_err("get kstrtoint failed\n");
					return -EINVAL;
				}
				break;

			case WATER_IS_PROMPT:
				ret = kstrtou8(tmp_string, DECIMAL_BASE,
					&info->data.para[i].prompt);
				if (ret) {
					hwlog_err("get kstrtou8 failed\n");
					return -EINVAL;
				}
				break;

			case WATER_CHECK_ACTION:
				ret = kstrtou8(tmp_string, DECIMAL_BASE,
					&info->data.para[i].action);
				if (ret) {
					hwlog_err("get kstrtou8 failed\n");
					return -EINVAL;
				}
				break;

			default:
				break;
			}
		}
	}

	for (i = 0; i < info->data.total_type; i++) {
		hwlog_info("para[%d] check_type:%d gpio:%d,%s %d %d %d %d %d\n",
			i,
			info->data.para[i].check_type,
			info->data.para[i].gpio_no,
			info->data.para[i].gpio_name,
			info->data.para[i].irq_no,
			info->data.para[i].multiple_handle,
			info->data.para[i].dmd_no_offset,
			info->data.para[i].prompt,
			info->data.para[i].action);
	}

	return 0;
}

static int water_check_probe(struct platform_device *pdev)
{
	int ret;
	struct water_check_info *info = NULL;
	struct device_node *np = NULL;
	struct pinctrl_state *pinctrl_def = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	g_info = info;
	info->dev = &pdev->dev;
	np = pdev->dev.of_node;

	wake_lock_init(&g_wc_wakelock, WAKE_LOCK_SUSPEND,
		"watercheck_wakelock");

	ret = of_property_read_u32(np, "need_pinctrl_config",
		&info->need_pinctrl_config);
	if (ret) {
		hwlog_err("need_pinctrl_config dts read failed\n");
		goto fail_free_mem;
	}
	hwlog_info("need_pinctrl_config=%d\n", info->need_pinctrl_config);

	if (info->need_pinctrl_config) {
		info->pinctrl = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR(info->pinctrl)) {
			hwlog_err("pinctrl get fail\n");
			goto fail_get_pinctrl;
		}

		pinctrl_def = pinctrl_lookup_state(info->pinctrl, "default");
		if (IS_ERR(pinctrl_def)) {
			hwlog_err("pinctrl lookup fail\n");
			goto fail_lookup_pinctrl;
		}

		/* configure the gpios as no-pull state */
		ret = pinctrl_select_state(info->pinctrl, pinctrl_def);
		if (ret) {
			hwlog_err("pinctrl config fail\n");
			goto fail_config_pinctrl;
		}
	}

	ret = water_check_parse_extra_dts(np, info);
	if (ret < 0)
		goto fail_parse_dts;

	ret = water_check_gpio_config(info);
	if (ret < 0)
		goto fail_check_gpio;

	INIT_DELAYED_WORK(&info->water_detect_work, water_detect_work);
	/* delay 8000ms to schedule work when power up first */
	schedule_delayed_work(&info->water_detect_work,
		msecs_to_jiffies(8000));

	dev_set_drvdata(&pdev->dev, info);

	hwlog_info("probe end\n");
	return 0;

fail_check_gpio:
fail_parse_dts:
fail_config_pinctrl:
fail_lookup_pinctrl:
	if (info->need_pinctrl_config)
		devm_pinctrl_put(info->pinctrl);
fail_get_pinctrl:
fail_free_mem:
	wake_lock_destroy(&g_wc_wakelock);
	devm_kfree(&pdev->dev, info);
	g_info = NULL;

	return ret;
}

static int water_check_remove(struct platform_device *pdev)
{
	struct water_check_info *info = dev_get_drvdata(&pdev->dev);

	hwlog_info("remove begin\n");

	if (!info)
		return -ENODEV;

	wake_lock_destroy(&g_wc_wakelock);
	cancel_delayed_work(&info->water_detect_work);

	if (info->need_pinctrl_config)
		devm_pinctrl_put(info->pinctrl);

	devm_kfree(&pdev->dev, info);
	dev_set_drvdata(&pdev->dev, NULL);
	g_info = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id water_check_of_match[] = {
	{
		.compatible = "hisilicon,water_check",
	},
	{},
};

static struct platform_driver water_check_drv = {
	.probe = water_check_probe,
	.remove = water_check_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "water_check",
		.of_match_table	= water_check_of_match,
	},
};

static int __init water_check_init(void)
{
	return platform_driver_register(&water_check_drv);
}

static void __exit water_check_exit(void)
{
	platform_driver_unregister(&water_check_drv);
}

late_initcall_sync(water_check_init);
module_exit(water_check_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("water check module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
