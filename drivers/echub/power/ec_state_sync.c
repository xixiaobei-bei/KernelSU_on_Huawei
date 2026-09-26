/*
 * ec_state_sync.c
 *
 * ec_state_sync driver for suspend/resume
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/syscore_ops.h>
#include "../echub.h"

#define GPIO_OUT_HIGH           (1)
#define GPIO_OUT_LOW            (0)

#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
#define MAX_SLEEP_COUNT         (100)
#endif

struct ec_state_sync_info {
	struct device *dev;
	int    gpio_sync;
};

int g_gpio_num;
#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
int g_sleep_count;
#endif

/**********************************************************
*  Function:       echub_gpio_init
*  Discription:    module probe
*  Parameters:     gpio_num
*  return value:   0-sucess or others-fail
**********************************************************/
static void echub_gpio_init(int gpio_num)
{
	gpio_request(gpio_num, "echub_suspend");
	gpio_direction_output(gpio_num, GPIO_OUT_HIGH);
}

/**********************************************************
*  Function:       echub_syscore_suspend
*  Discription:    module suspend
*  Parameters:     void
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_syscore_suspend(void)
{
	echub_info("%s\n", __func__);
	gpio_set_value(g_gpio_num, GPIO_OUT_LOW);
#ifdef CONFIG_HUAWEI_ECHUB_LID_TEST
	g_sleep_count++;
	if (g_sleep_count > MAX_SLEEP_COUNT)
		g_sleep_count = 0;
#endif
	return 0;
}

/**********************************************************
*  Function:       echub_syscore_resume
*  Discription:    module resume
*  Parameters:     void
*  return value:   0-sucess or others-fail
**********************************************************/
static void echub_syscore_resume(void)
{
	echub_info("%s\n", __func__);
	gpio_set_value(g_gpio_num, GPIO_OUT_HIGH);
}

static struct syscore_ops echub_syscore_ops = {
	.suspend = echub_syscore_suspend,
	.resume  = echub_syscore_resume,
};

/**********************************************************
*  Function:       ec_state_parse_dts
*  Discription:    parse the module dts config value
*  Parameters:     np:device_node
*                  di:ec_state_sync_info
*  return value:   0-sucess or others-fail
**********************************************************/
static int ec_state_parse_dts(struct device_node *np, struct ec_state_sync_info *di)
{
	int ret = 0;

	ret = of_property_read_u32(np, "gpio_sync", (u32 *)&di->gpio_sync);
	if (ret) {
		echub_err("get gpio_sync failed\n");
		return -EINVAL;
	}
	g_gpio_num = di->gpio_sync;
	echub_info("gpio_sync = %d\n", di->gpio_sync);

	return 0;
}

/**********************************************************
*  Function:       ec_state_sync_probe
*  Discription:    module probe
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int ec_state_sync_probe(struct platform_device *pdev)
{
	struct ec_state_sync_info *di;
	int ret = 0;
	struct device_node *np = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (di == NULL) {
		echub_err("ec_state_sync_info is NULL!\n");
		return -ENOMEM;
	}

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	platform_set_drvdata(pdev, di);

	ret = ec_state_parse_dts(np, di);
	if (ret < 0) {
		echub_err("ec_state_sync_info get data from dts fail!\n");
		goto err_state;
	}
	echub_gpio_init(di->gpio_sync);
	register_syscore_ops(&echub_syscore_ops);
	echub_info("%s ok!\n", __func__);
	return 0;
err_state:
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	return ret;
}

/**********************************************************
*  Function:       ec_state_sync_remove
*  Discription:    module remove
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int ec_state_sync_remove(struct platform_device *pdev)
{
	struct ec_state_sync_info *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		echub_err("[%s]di is NULL!\n", __func__);
		return -ENODEV;
	}

	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;

	return 0;
}

static struct of_device_id ec_state_sync_match_table[] = {
	{
		.compatible = "huawei,ec_state_sync",
		.data = NULL,
	},
	{
	},
};
static struct platform_driver ec_state_sync_driver = {
	.probe        = ec_state_sync_probe,
	.remove       = ec_state_sync_remove,
	.driver       = {
		.name           = "huawei,ec_state_sync",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(ec_state_sync_match_table),
	},
};

/**********************************************************
*  Function:       ec_state_sync_init
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
int __init ec_state_sync_init(void)
{
	echub_info("%s\n", __func__);
	return platform_driver_register(&ec_state_sync_driver);
}

/**********************************************************
*  Function:       ec_state_sync_exit
*  Discription:    module exit
*  Parameters:     NULL
*  return value:   NULL
**********************************************************/
void __exit ec_state_sync_exit(void)
{
	platform_driver_unregister(&ec_state_sync_driver);
}

module_init(ec_state_sync_init);
module_exit(ec_state_sync_exit);

MODULE_AUTHOR("HUAWEI");
MODULE_DESCRIPTION("ec state sync module driver");
MODULE_LICENSE("GPL");
