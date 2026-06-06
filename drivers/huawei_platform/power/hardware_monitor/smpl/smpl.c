/*
 * smpl.c
 *
 * smpl(sudden momentary power loss) error monitor driver
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/wakelock.h>
#include <linux/mfd/hisi_pmic_mntn.h>

#include <linux/hisi/usb/hisi_usb.h>
#ifdef CONFIG_HISI_BCI_BATTERY
#include <linux/power/hisi/hisi_bci_battery.h>
#endif
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG smpl
HWLOG_REGIST();

#define MAX_ERR_MSG_LEN            128

struct smpl_dev {
	struct device *dev;
	struct delayed_work monitor_work;
	/* vsys_pwroff_abs_pd disable/enable when insert or pull out charger */
	struct notifier_block usb_nb;
	struct work_struct abs_pd_work;
	atomic_t in_charging;
	struct wake_lock abs_pd_wakelock;
};

static struct smpl_dev *g_smpl_dev;
static unsigned int smpl_happened;

static int __init early_parse_normal_reset_type_cmdline(char *p)
{
	if (!p) {
		hwlog_err("cmdline is null\n");
		return 0;
	}

	hwlog_info("normal_reset_type=%s\n", p);

	/* AP_S_SMPL = 0x51  BR_POWERON_BY_SMPL = 0x26 */
	if (strstr(p, "SMPL"))
		smpl_happened = 1;

	hwlog_info("smpl happened=%d\n", smpl_happened);
	return 0;
}

early_param("normal_reset_type", early_parse_normal_reset_type_cmdline);

static void smpl_error_monitor_work(struct work_struct *work)
{
	char tmp_buf[MAX_ERR_MSG_LEN] = { 0 };
	int batt_vol;
	int batt_temp;
	int batt_ufsoc;
	int batt_afsoc;
	int batt_soc;

	hwlog_info("monitor_work begin\n");

	if (smpl_happened) {
		batt_vol = hisi_battery_voltage();
		batt_temp = hisi_battery_temperature();
		batt_ufsoc = hisi_battery_unfiltered_capacity();
		batt_afsoc = hisi_battery_capacity();
		batt_soc = hisi_bci_show_capacity();

		snprintf(tmp_buf, MAX_ERR_MSG_LEN - 1,
			"smpl vbatt=%d, temp=%d, ufsoc=%d, afsoc=%d, soc=%d\n",
			batt_vol, batt_temp, batt_ufsoc, batt_afsoc, batt_soc);

		hwlog_info("exception happened: %s\n", tmp_buf);

		power_dsm_dmd_report(POWER_DSM_SMPL, ERROR_NO_SMPL, tmp_buf);
	}

	hwlog_info("monitor_work end\n");
}

static void smpl_abs_pd_wake_lock(void)
{
	if (g_smpl_dev) {
		if (!wake_lock_active(&g_smpl_dev->abs_pd_wakelock)) {
			wake_lock(&g_smpl_dev->abs_pd_wakelock);
			hwlog_info("abs_pd wakelock\n");
		}
	}
}

static void smpl_abs_pd_wake_unlock(void)
{
	if (g_smpl_dev) {
		if (wake_lock_active(&g_smpl_dev->abs_pd_wakelock)) {
			wake_unlock(&g_smpl_dev->abs_pd_wakelock);
			hwlog_info("abs_pd wakeunlock\n");
		}
	}
}

static void smpl_abs_pd_work(struct work_struct *work)
{
	int ret;
	bool enable = false;
	struct smpl_dev *l_dev = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		smpl_abs_pd_wake_unlock();
		return;
	}

	l_dev = container_of(work, struct smpl_dev, abs_pd_work);
	if (!l_dev) {
		hwlog_info("l_dev is null\n");
		smpl_abs_pd_wake_unlock();
		return;
	}

	if (atomic_read(&l_dev->in_charging)) {
		/* in charge, disable pwroff_abs_pd */
		enable = false;
		hwlog_info("charger insert, disable pwroff_abs_pd\n");
	} else {
		/* not in charge, enable pwroff_abs_pd */
		enable = true;
		hwlog_info("charger pull out, enable pwroff_abs_pd\n");
	}

	hwlog_info("config enable:%d\n", enable);

	ret = hisi_pmic_mntn_config_vsys_pwroff_abs_pd(enable);
	if (ret < 0) {
		hwlog_info("vsys_pwroff_abs_pd config fail, ret:%d\n", ret);
	} else {
		ret = hisi_pmic_mntn_vsys_pwroff_abs_pd_state();
		hwlog_info("vsys_pwroff_abs_pd config ok, state:%d\n", ret);
	}

	smpl_abs_pd_wake_unlock();
}

static int smpl_charge_usb_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	bool valid_event = false;
	struct smpl_dev *l_dev = NULL;

	if (!nb) {
		hwlog_err("nb is null\n");
		return NOTIFY_OK;
	}

	l_dev = container_of(nb, struct smpl_dev, usb_nb);
	if (!l_dev) {
		hwlog_info("l_dev is null\n");
		return NOTIFY_OK;
	}

	switch ((enum hisi_charger_type)event) {
	/* fall through: sdp & cdp & dcp & unknown type */
	case CHARGER_TYPE_SDP:
	case CHARGER_TYPE_CDP:
	case CHARGER_TYPE_DCP:
	case CHARGER_TYPE_UNKNOWN:
		atomic_set(&l_dev->in_charging, 1);
		valid_event = true;
		break;

	case CHARGER_TYPE_NONE:
		atomic_set(&l_dev->in_charging, 0);
		valid_event = true;
		break;

	default:
		valid_event = false;
		break;
	}

	hwlog_info("event:%lu, in_charging:%d, valid_event:%d\n",
		event, atomic_read(&l_dev->in_charging), valid_event);

	if (valid_event) {
		hwlog_info("schedule work to config vsys_pwroff_abs_pd\n");

		/* 1. get wake lock */
		smpl_abs_pd_wake_lock();

		/* 2. queue work to check changer status and charging state */
		schedule_work(&l_dev->abs_pd_work);
	}

	return NOTIFY_OK;
}

static int smpl_probe(struct platform_device *pdev)
{
	struct smpl_dev *di = NULL;
	enum hisi_charger_type type;
	int ret;

	hwlog_info("probe begin\n");

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_smpl_dev = di;

	INIT_DELAYED_WORK(&di->monitor_work, smpl_error_monitor_work);
	schedule_delayed_work(&di->monitor_work, 100); /* 100 jiffies */

	INIT_WORK(&di->abs_pd_work, smpl_abs_pd_work);
	wake_lock_init(&di->abs_pd_wakelock, WAKE_LOCK_SUSPEND,
		"smpl_abs_pd_wakelock");

	di->usb_nb.notifier_call = smpl_charge_usb_notifier_call;
	ret = hisi_charger_type_notifier_register(&di->usb_nb);
	if (ret < 0) {
		wake_lock_destroy(&di->abs_pd_wakelock);
		kfree(di);
		g_smpl_dev = NULL;

		hwlog_err("charger_type_notifier register failed\n");
		return ret;
	}

	type = hisi_get_charger_type();
	hwlog_info("charger_type=%d\n", type);

	if (type != CHARGER_TYPE_NONE)
		smpl_charge_usb_notifier_call(&di->usb_nb, type, g_smpl_dev);

	hwlog_info("probe end\n");
	return 0;
}

static int smpl_remove(struct platform_device *pdev)
{
	hwlog_info("remove begin\n");

	if (!g_smpl_dev) {
		hwlog_info("g_smpl_dev is null\n");
		return -ENOMEM;
	}

	flush_work(&g_smpl_dev->abs_pd_work);
	wake_lock_destroy(&g_smpl_dev->abs_pd_wakelock);

	if (hisi_charger_type_notifier_unregister(&g_smpl_dev->usb_nb))
		hwlog_err("hisi_charger_type_notifier unregister fail\n");

	kfree(g_smpl_dev);
	g_smpl_dev = NULL;

	hwlog_info("remove ok\n");
	return 0;
}

static const struct of_device_id smpl_match_table[] = {
	{
		.compatible = "huawei,smpl",
		.data = NULL,
	},
	{},
};

static struct platform_driver smpl_driver = {
	.probe = smpl_probe,
	.remove = smpl_remove,
	.driver = {
		.name = "huawei,smpl",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(smpl_match_table),
	},
};

static int __init smpl_init(void)
{
	return platform_driver_register(&smpl_driver);
}


static void __exit smpl_exit(void)
{
	platform_driver_unregister(&smpl_driver);
}

module_init(smpl_init);
module_exit(smpl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smpl error monitor module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
