/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description:  headfile of wakeup_interface.c
 * Author: Shengwei Zhang
 * Create: 2019-03-31
 * History: NA
 */
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <sound/jack.h>
#include <linux/fs.h>
#include <linux/regmap.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <securec.h>
#include "wakeup.h"

#define HWLOG_TAG wakeup

struct wakeup_data {
    struct mutex notifier_lock;
};

static struct wakeup_data *g_wakeup_pdata;

static const struct of_device_id g_wakeup_of_match[] = {
    {
        .compatible = "huawei,wakeup",
    },
    { },
};

MODULE_DEVICE_TABLE(of, g_wakeup_of_match);

static int wakeup_notifier_call(unsigned long event, char *str);
char g_envp_hal[ENVP_LENTH + 1] = {0};

static long wakeup_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;
    char *wakeInfo = NULL;

    printk("%s: wakeup ioctl enter, data addr: %ld\n", __func__, arg);
    if (g_wakeup_pdata == NULL) {
        printk("%s: wakeup ioctl error: g_wakeup_pdata = null\n", __func__);
        return -EBUSY;
    }

    switch (cmd) {
        case WAKEUP_REPORT_EVENT:
            printk("%s: wakeup report event\n", __func__);
            ret = wakeup_notifier_call(1, "wakeup_report=true");
            break;
        case WAKEUP_INFO_EVENT:
            wakeInfo = (char *)(uintptr_t)arg;
            copy_from_user(g_envp_hal, wakeInfo, ENVP_LENTH);
            printk("%s: wakeup info event(%s)\n", __func__, g_envp_hal);
            ret = wakeup_notifier_call(1, g_envp_hal);
            break;
        default:
            printk("%s: unsupport cmd\n", __func__);
            ret = -EINVAL;
            break;
    }

    return (long)ret;
}

static const struct file_operations g_wakeup_fops = {
    .owner           = THIS_MODULE,
    .open            = simple_open,
    .unlocked_ioctl  = wakeup_ioctl,
    .compat_ioctl    = wakeup_ioctl,
};

static struct miscdevice g_wakeup_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "wakeup",
    .fops  = &g_wakeup_fops,
};

static int wakeup_notifier_call(unsigned long event, char *str)
{
    int ret;
    char envp_ext0[ENVP_LENTH];
    char *envp_ext[ENVP_EXT_MEMBER] = { envp_ext0, NULL };

    printk("%s: wakeup_notifier_call: wakeup is %lu, str is %s\n", __func__, event, str);
    mutex_lock(&g_wakeup_pdata->notifier_lock);
    ret = snprintf_s(envp_ext0, ENVP_LENTH, (ENVP_LENTH - 1), str);
    if (ret < 0) {
        printk("%s: snprintf_s failed, ret = %d\n", __func__, ret);
        mutex_unlock(&g_wakeup_pdata->notifier_lock);
        return ret;
    }

    kobject_uevent_env(&g_wakeup_miscdev.this_device->kobj, KOBJ_CHANGE, envp_ext);
    mutex_unlock(&g_wakeup_pdata->notifier_lock);

    return 0;
}

static int wakeup_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;

    printk("%s: wakeup probe enter\n", __func__);
    g_wakeup_pdata = devm_kzalloc(dev, sizeof(*g_wakeup_pdata), GFP_KERNEL);
    if (g_wakeup_pdata == NULL) {
        printk("%s: cannot allocate usb_audio_common data\n", __func__);
        return -ENOMEM;
    }
    mutex_init(&g_wakeup_pdata->notifier_lock);

    ret = misc_register(&g_wakeup_miscdev);
    if (ret != 0) {
        printk("%s: can't register wakeup miscdev, ret:%d\n", __func__, ret);
        goto err_out;
    }
    printk("%s: wakeup probe success\n", __func__);

    return 0;

err_out:
    misc_deregister(&g_wakeup_miscdev);
    devm_kfree(dev, g_wakeup_pdata);
    g_wakeup_pdata = NULL;

    return ret;
}

static int wakeup_remove(struct platform_device *pdev)
{
    if (g_wakeup_pdata != NULL) {
        printk("%s: wakeup free\n", __func__);
        devm_kfree(&pdev->dev, g_wakeup_pdata);
        g_wakeup_pdata = NULL;
    }

    misc_deregister(&g_wakeup_miscdev);

    printk("%s: exit\n", __func__);

    return 0;
}

static struct platform_driver g_wakeup_driver = {
    .driver = {
        .name           = "wakeup",
        .owner          = THIS_MODULE,
        .of_match_table = g_wakeup_of_match,
    },
    .probe  = wakeup_probe,
    .remove = wakeup_remove,
};

static int __init wakeup_init(void)
{
    int ret;
    ret = platform_driver_register(&g_wakeup_driver);
    if (ret > 0) {
        printk("%s: wakeup driver register failed\n", __func__);
        return ret;
    }
    printk("%s: wakeup driver register succeed\n", __func__);
    return ret;
}

static void __exit wakeup_exit(void)
{
    platform_driver_unregister(&g_wakeup_driver);
}

module_init(wakeup_init);
module_exit(wakeup_exit);

MODULE_DESCRIPTION("wakeup control driver");
MODULE_LICENSE("GPL v2");
