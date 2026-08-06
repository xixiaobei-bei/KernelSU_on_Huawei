/*
 * powerkey_spmi.c - powerkey driver
 *
 * Copyright (C) 2013 Hisilicon Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/version.h>

#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/util.h>
#include <linux/kthread.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>

#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>

#ifdef CONFIG_HISI_HI6XXX_PMIC
#include <soc_smart_interface.h>
#endif
#include <linux/hisi/powerkey_event.h>
#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_hisi_platform.h>
#endif
#ifdef CONFIG_HW_ZEROHUNG
#include <chipset_common/hwzrhung/zrhung.h>
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <pr_log.h>

#if defined(CONFIG_SUPPORT_SIM1_HPD_KEY_RESTART)
#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>

#define KEY_VALUE_LOW 0x0
#define SIM1_HPD_ENABLE 1

#define SIM1_HPD_PRESS 1
#define SIM1_HPD_RELEASE 0

#define POWERKEY_PRESS 0x80

#define PMIC_GPIO_SIM1_HPD_DATA   PMIC_GPIO1DATA_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_DIR    PMIC_GPIO1DIR_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_AFSEL  PMIC_GPIO1AFSEL_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_IS     PMIC_GPIO1IS_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_IBE    PMIC_GPIO1IBE_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_IEV    PMIC_GPIO1IEV_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_PUSEL  PMIC_GPIO1PUSEL_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_PDSEL  PMIC_GPIO1PDSEL_ADDR(0)
#define PMIC_GPIO_SIM1_HPD_DEBSEL PMIC_GPIO1DEBSEL_ADDR(0)
#define PMIC_IRQ_MASK_10 PMIC_IRQ_MASK_10_ADDR(0)
#define PMIC_STATUS0 PMIC_STATUS0_ADDR(0)


#define PMIC_GPIO_SIM1_HPD_20MS_FILTER 1
#define PMIC_GPIO_SIM1_HPD_NOT_PD 0
#define PMIC_GPIO_SIM1_HPD_PU 1
#define PMIC_GPIO_SIM1_HPD_INPUT 0
#define PMIC_GPIO_SIM1_HPD_GPIO_FUNC 0
#define PMIC_GPIO_SIM1_HPD_EDGE_TRIGGER 0
#define PMIC_GPIO_SIM1_HPD_DOUBLE_EDGE_TRIGGER 1
#define PMIC_GPIO1_HPD_IRQ 0

static irqreturn_t powerkey_sim1_hpd_hdl(int irq, void *data);
#endif

#define PR_LOG_TAG POWERKEY_TAG

#define POWER_KEY_RELEASE 0
#define POWER_KEY_PRESS 1

#define POWER_KEY_ARMPC 1
#define POWER_KEY_DEFAULT 0
#define POWER_KEY_CNT 0

static irqreturn_t powerkey_down_hdl(int irq, void *data);
static irqreturn_t powerkey_up_hdl(int irq, void *data);
static irqreturn_t powerkey_1s_hdl(int irq, void *data);
static irqreturn_t powerkey_6s_hdl(int irq, void *data);
static irqreturn_t powerkey_8s_hdl(int irq, void *data);
static irqreturn_t powerkey_10s_hdl(int irq, void *data);

#ifdef CONFIG_HUAWEI_DSM
#define PRESS_KEY_INTERVAL 80   /*the minimum press interval*/
#define STATISTIC_INTERVAL 60   /*the statistic interval for key event*/
#define MAX_PRESS_KEY_COUNT 120 /*the default press count for a normal use*/


static int powerkey_press_count;
static unsigned long powerkey_last_press_time;
static struct timer_list
	dsm_powerkey_timer; /*used to reset the statistic variable*/

static struct dsm_dev dsm_power_key = {
	.name = "dsm_power_key",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};
static struct dsm_client *power_key_dclient;
#endif

struct powerkey_irq_map {
	char *event_name;
	int irq_suspend_cfg;
	irqreturn_t (*irq_handler)(int irq, void *data);
};

struct powerkey_info {
	struct input_dev *idev;
	int type;
	struct wakeup_source pwr_wake_lock;
#if defined(CONFIG_SUPPORT_SIM1_HPD_KEY_RESTART)
	unsigned int sim1_hpd_flag;
	unsigned int sim1_hpd_press_status;
#endif
};

static struct semaphore long_presspowerkey_happen_sem;

static struct powerkey_irq_map g_event_name_cfg[] = {
	{ .event_name = "down", .irq_suspend_cfg = IRQF_NO_SUSPEND,
		.irq_handler = powerkey_down_hdl },
	{ .event_name = "up", .irq_suspend_cfg = IRQF_NO_SUSPEND,
		.irq_handler = powerkey_up_hdl },
	{ .event_name = "hold 1s", .irq_suspend_cfg = 0,
		.irq_handler = powerkey_1s_hdl },
	{ .event_name = "hold 6s", .irq_suspend_cfg = 0,
		.irq_handler = powerkey_6s_hdl },
	{ .event_name = "hold 8s", .irq_suspend_cfg = 0,
		.irq_handler = powerkey_8s_hdl },
	{ .event_name = "hold 10s", .irq_suspend_cfg = 0,
		.irq_handler = powerkey_10s_hdl }
};

int long_presspowerkey_happen(void *data)
{
	pr_err("powerkey long press hanppend\n");
	while (!kthread_should_stop()) {
		down(&long_presspowerkey_happen_sem);
#ifdef CONFIG_HISI_BB
		rdr_long_press_powerkey();
#endif
#ifdef CONFIG_HW_ZEROHUNG
		zrhung_save_lastword();
#endif
	}
	return 0;
}

#ifdef CONFIG_HUAWEI_DSM
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
static void powerkey_timer_func(struct timer_list *t)
#else
static void powerkey_timer_func(unsigned long data)
#endif
{
	if (powerkey_press_count > MAX_PRESS_KEY_COUNT) {
		if (!dsm_client_ocuppy(power_key_dclient)) {
			dsm_client_record(power_key_dclient,
				"powerkey trigger on the abnormal style.\n");
			dsm_client_notify(
				power_key_dclient, DSM_POWER_KEY_ERROR_NO);
		}
	}

	/* reset the statistic variable */
	powerkey_press_count = 0;
	mod_timer(&dsm_powerkey_timer, jiffies + STATISTIC_INTERVAL * HZ);
}

static void powerkey_dsm(void)
{
	powerkey_press_count++;
	if ((jiffies - powerkey_last_press_time) <
		msecs_to_jiffies(PRESS_KEY_INTERVAL)) {
		if (!dsm_client_ocuppy(power_key_dclient)) {
			dsm_client_record(power_key_dclient,
				"power key trigger on the abnormal style\n");
			dsm_client_notify(power_key_dclient,
				DSM_POWER_KEY_ERROR_NO);
		}
	}
	powerkey_last_press_time = jiffies;
}
#endif

#ifdef CONFIG_HISI_POWERKEY_DEBUG
static int test_powerkey_notifier_call(
	struct notifier_block *powerkey_nb, unsigned long event, void *data)
{
	int ret = 0;

	switch (event) {
	case PRESS_KEY_DOWN:
		pr_err("[%s] test power key press event!\n", __func__);
		break;
	case PRESS_KEY_UP:
		pr_err("[%s] test power key release event!\n", __func__);
		break;
	case PRESS_KEY_1S:
		pr_err("[%s] test response long press 1s event!\n", __func__);
		break;
	case PRESS_KEY_6S:
		pr_err("[%s] test response long press 6s event!\n", __func__);
		break;
	case PRESS_KEY_8S:
		pr_info("[%s] test response long press 8s event!\n", __func__);
		break;
	case PRESS_KEY_10S:
		pr_info("[%s] test response long press 10s event!\n", __func__);
		break;
	default:
		pr_err("[%s]invalid event %d!\n", __func__, (int)(event));
		ret = -1;
		break;
	}

	return ret;
}

static struct notifier_block test_powerkey_nb;
#endif

#if defined(CONFIG_SUPPORT_SIM1_HPD_KEY_RESTART)
static irqreturn_t powerkey_sim1_hpd_hdl(int irq, void *data)
{
	struct powerkey_info *info = (struct powerkey_info *)data;
	unsigned int keyup_value;

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	keyup_value = pmic_read_reg(PMIC_GPIO_SIM1_HPD_DATA);

	if (keyup_value == KEY_VALUE_LOW) {
		pr_err("[%s] power key press interrupt!\n", __func__);
		call_powerkey_notifiers(PRESS_KEY_DOWN, data);
#ifdef CONFIG_HUAWEI_DSM
		powerkey_dsm();
#endif
		input_report_key(info->idev, KEY_POWER, POWER_KEY_PRESS);
		input_sync(info->idev);

		info->sim1_hpd_press_status = SIM1_HPD_PRESS;
	} else {
		keyup_value = pmic_read_reg(PMIC_STATUS0);
		if (!(keyup_value & POWERKEY_PRESS)) {
			pr_err("[%s]power key release interrupt!\n", __func__);
			call_powerkey_notifiers(PRESS_KEY_UP, data);
			input_report_key(info->idev, KEY_POWER,
				POWER_KEY_RELEASE);
			input_sync(info->idev);
		}

		info->sim1_hpd_press_status = SIM1_HPD_RELEASE;
	}

	return IRQ_HANDLED;
}

static int pmic_sim1_hpd_irq_init(struct spmi_device *pdev,
	struct powerkey_info *info)
{
	int ret, irq;
	struct device *dev = &pdev->dev;

	irq = spmi_get_irq_byname(
		pdev, NULL, "sim1-hpd");
	if (irq < 0) {
		dev_err(dev, "failed to get %s irq\n", "sim1-hpd");
		return -ENOENT;
	}

	ret = devm_request_irq(
		dev, irq, powerkey_sim1_hpd_hdl,
		IRQF_NO_SUSPEND,
		"sim1-hpd", info);
	if (ret < 0) {
		dev_err(dev, "failed to request %s irq\n", "sim1-hpd");
		return -ENOENT;
	}

	pr_info("[%s] sim1_hpd_irq_init ok\n", __func__);
	return ret;
}

static void pmic_sim1_hpd_reg_init(void)
{
	/* 20ms filter */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_DEBSEL,
		PMIC_GPIO_SIM1_HPD_20MS_FILTER);
	/* not PD */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_PDSEL,
		PMIC_GPIO_SIM1_HPD_NOT_PD);
	/* PU */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_PUSEL,
		PMIC_GPIO_SIM1_HPD_PU);
	/* input */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_DIR,
		PMIC_GPIO_SIM1_HPD_INPUT);
	/* gpio func */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_AFSEL,
		PMIC_GPIO_SIM1_HPD_GPIO_FUNC);
	/* edge trigger */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_IS,
		PMIC_GPIO_SIM1_HPD_EDGE_TRIGGER);
	/* double edge trigger  */
	pmic_write_reg(PMIC_GPIO_SIM1_HPD_IBE,
		PMIC_GPIO_SIM1_HPD_DOUBLE_EDGE_TRIGGER);
	/* GPIO1 IRQ MASK*/
	pmic_write_reg(PMIC_IRQ_MASK_10,
		PMIC_GPIO1_HPD_IRQ);
}

static void pmic_sim1_hpd_dts_cfg(struct spmi_device *pdev,
	struct powerkey_info *info)
{
	int ret;
	int sim1_hpd_flag = 0;
	struct device *dev = &pdev->dev;

	ret = of_property_read_u32(pdev->res.of_node,
		"sim1-hpd", &sim1_hpd_flag);
	if (ret < 0) {
		info->sim1_hpd_flag = POWER_KEY_DEFAULT;
		dev_info(dev, "sim1_hpd_gpio not cpnfig\n");
	} else {
		info->sim1_hpd_flag = sim1_hpd_flag;
	}
}

#endif

static irqreturn_t powerkey_down_hdl(int irq, void *data)
{
	struct powerkey_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct powerkey_info *)data;

#if defined(CONFIG_SUPPORT_SIM1_HPD_KEY_RESTART)
	if ((info->sim1_hpd_flag) &&
		(info->sim1_hpd_press_status == SIM1_HPD_PRESS))
		return IRQ_HANDLED;

#endif

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	pr_err("[%s] power key press interrupt!\n", __func__);
	call_powerkey_notifiers(PRESS_KEY_DOWN, data);

#ifdef CONFIG_HUAWEI_DSM
	powerkey_dsm();
#endif

	input_report_key(info->idev, KEY_POWER, POWER_KEY_PRESS);
	input_sync(info->idev);

	/* If there is no power-key release irq, it's a ARM-PC device.
	 * report power button input when it was pressed
	 * (as opposed to when it was used to wake the system). */
	if (info->type == POWER_KEY_ARMPC) {
		input_report_key(info->idev, KEY_POWER, POWER_KEY_RELEASE);
		input_sync(info->idev);
	}

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_up_hdl(int irq, void *data)
{
	struct powerkey_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct powerkey_info *)data;

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	pr_err("[%s]power key release interrupt!\n", __func__);
	call_powerkey_notifiers(PRESS_KEY_UP, data);
	input_report_key(info->idev, KEY_POWER, POWER_KEY_RELEASE);
	input_sync(info->idev);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_1s_hdl(int irq, void *data)
{
	struct powerkey_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct powerkey_info *)data;

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	pr_err("[%s]response long press 1s interrupt!\n", __func__);
	call_powerkey_notifiers(PRESS_KEY_1S, data);
	input_report_key(info->idev, KEY_POWER, POWER_KEY_PRESS);
	input_sync(info->idev);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_6s_hdl(int irq, void *data)
{
	struct powerkey_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct powerkey_info *)data;

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	pr_err("[%s]response long press 6s interrupt!\n", __func__);
	call_powerkey_notifiers(PRESS_KEY_6S, data);
	up(&long_presspowerkey_happen_sem);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_8s_hdl(int irq, void *data)
{
	struct powerkey_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct powerkey_info *)data;

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	pr_info("[%s]response long press 8s interrupt!\n", __func__);
	call_powerkey_notifiers(PRESS_KEY_8S, data);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_10s_hdl(int irq, void *data)
{
	struct powerkey_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct powerkey_info *)data;

	__pm_wakeup_event(&info->pwr_wake_lock, jiffies_to_msecs(HZ));

	pr_info("[%s]response long press 10s interrupt!\n", __func__);
	call_powerkey_notifiers(PRESS_KEY_10S, data);

	return IRQ_HANDLED;
}

static int powerkey_irq_init(struct spmi_device *pdev,
	struct powerkey_info *info)
{
	int i, ret, irq;
	unsigned int type = POWER_KEY_DEFAULT;
	struct device *dev = &pdev->dev;

	ret = of_property_read_u32(pdev->res.of_node, "powerkey-type", &type);
	if (ret < 0) {
		info->type = POWER_KEY_DEFAULT;
		dev_info(dev, "use default powerkey type for mobile devices\n");
	} else {
		info->type = type;
	}

	for (i = 0; i < (int)ARRAY_SIZE(g_event_name_cfg); i++) {
		irq = spmi_get_irq_byname(
			pdev, NULL, g_event_name_cfg[i].event_name);
		if (irq < 0) {
			dev_err(dev, "failed to get %s irq id\n",
				g_event_name_cfg[i].event_name);
			continue;
		}

		ret = devm_request_irq(
			dev, irq, g_event_name_cfg[i].irq_handler,
			g_event_name_cfg[i].irq_suspend_cfg,
			g_event_name_cfg[i].event_name, info);
		if (ret < 0) {
			dev_err(dev, "failed to request %s irq\n",
				g_event_name_cfg[i].event_name);
			return -ENOENT;
		}
	}

#if defined(CONFIG_SUPPORT_SIM1_HPD_KEY_RESTART)
	if (info->sim1_hpd_flag == SIM1_HPD_ENABLE) {
		ret = pmic_sim1_hpd_irq_init(pdev, info);
		if (ret < 0) {
			dev_err(dev, "%s failed\n", __func__);
			return -ENOENT;
		}
	}
#endif

	return ret;
}

static int powerkey_probe(struct spmi_device *pdev)
{
	struct powerkey_info *info = NULL;
	struct device *dev = &pdev->dev;
	int ret;

	if (pdev == NULL) {
		dev_err(dev, "[Pwrkey]parameter error!\n");
		ret = -EINVAL;
		return ret;
	}

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;

	info->idev = input_allocate_device();
	if (info->idev == NULL) {
		dev_err(&pdev->dev, "Failed to allocate input device\n");
		ret = -ENOENT;
		devm_kfree(dev, info);
		return ret;
	}
	/*
	 * This initialization function is used to identify
	 * whether the device has only a power key.
	 */
	hisi_pmic_powerkey_only_status_get();
#ifdef CONFIG_HISI_MNTN_GT_WATCH_SUPPORT
	hisi_pm_get_gt_watch_support_state();
#endif
	info->idev->name = "ponkey_on";
	info->idev->phys = "ponkey_on/input0";
	info->idev->dev.parent = &pdev->dev;
	info->idev->evbit[0] = BIT_MASK(EV_KEY);
	__set_bit(KEY_POWER, info->idev->keybit);

	wakeup_source_init(&info->pwr_wake_lock, "android-pwr");

#if defined(CONFIG_HUAWEI_DSM)
	/* initialize the statistic variable */

	powerkey_press_count = 0;
	powerkey_last_press_time = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	timer_setup(&dsm_powerkey_timer, powerkey_timer_func, 0);
#else
	setup_timer(&dsm_powerkey_timer, powerkey_timer_func, (uintptr_t)info);
#endif
#endif

#ifdef CONFIG_HISI_POWERKEY_DEBUG
	test_powerkey_nb.notifier_call = test_powerkey_notifier_call;
	powerkey_register_notifier(&test_powerkey_nb);
#endif

	sema_init(&long_presspowerkey_happen_sem, POWER_KEY_CNT);

	if (!kthread_run(long_presspowerkey_happen, NULL, "long_powerkey"))
		pr_err("create thread long_presspowerkey_happen faild.\n");

#if defined(CONFIG_SUPPORT_SIM1_HPD_KEY_RESTART)
	pmic_sim1_hpd_dts_cfg(pdev, info);
	if (info->sim1_hpd_flag == SIM1_HPD_ENABLE) {
		info->sim1_hpd_press_status = SIM1_HPD_RELEASE;
		pmic_sim1_hpd_reg_init();
	}
#endif

	ret = powerkey_irq_init(pdev, info);
	if (ret < 0)
		goto unregister_err;

	ret = input_register_device(info->idev);
	if (ret) {
		dev_err(&pdev->dev, "Can't register input device: %d\n", ret);
		ret = -ENOENT;
		goto unregister_err;
	}

	dev_set_drvdata(&pdev->dev, info);

#ifdef CONFIG_HUAWEI_DSM
	if (power_key_dclient == NULL)
		power_key_dclient = dsm_register_client(&dsm_power_key);
	mod_timer(&dsm_powerkey_timer, jiffies + STATISTIC_INTERVAL * HZ);
#endif

	return ret;

unregister_err:
	wakeup_source_trash(&info->pwr_wake_lock);
	input_free_device(info->idev);
	devm_kfree(dev, info);

	return ret;
}

static int powerkey_remove(struct spmi_device *pdev)
{
	struct powerkey_info *info = dev_get_drvdata(&pdev->dev);
#ifdef CONFIG_HISI_POWERKEY_DEBUG
	powerkey_unregister_notifier(&test_powerkey_nb);
#endif
	if (info != NULL) {
		wakeup_source_trash(&info->pwr_wake_lock);
		input_free_device(info->idev);
		input_unregister_device(info->idev);
	}
	return 0;
}

#ifdef CONFIG_PM
static int powerkey_suspend(struct spmi_device *pdev, pm_message_t state)
{
	pr_info("[%s]suspend successfully\n", __func__);
	return 0;
}

static int powerkey_resume(struct spmi_device *pdev)
{
	pr_info("[%s]resume successfully\n", __func__);
	return 0;
}
#endif

static const struct of_device_id powerkey_of_match[] = {
	{
		.compatible = "hisilicon-hisi-powerkey",
	},
	{},
};

static struct spmi_device_id powerkey_spmi_id[] = {
	{"powerkey", 0}, {},
};

MODULE_DEVICE_TABLE(of, powerkey_of_match);

static struct spmi_driver powerkey_driver = {
	.probe = powerkey_probe,
	.remove = powerkey_remove,
	.driver = {
			.owner = THIS_MODULE,
			.name = "powerkey",
			.of_match_table = powerkey_of_match,
		},
#ifdef CONFIG_PM
	.suspend = powerkey_suspend,
	.resume = powerkey_resume,
#endif
	.id_table = powerkey_spmi_id,

};

static int __init powerkey_init(void)
{
	return spmi_driver_register(&powerkey_driver);
}

static void __exit powerkey_exit(void)
{
	spmi_driver_unregister(&powerkey_driver);
}

module_init(powerkey_init);
module_exit(powerkey_exit);

MODULE_DESCRIPTION("PMIC Power key driver");
MODULE_LICENSE("GPL v2");
