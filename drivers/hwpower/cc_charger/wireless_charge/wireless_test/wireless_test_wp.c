/*
 * wireless_test_wp.c
 *
 * wireless charge wp test
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>

#define HWLOG_TAG wireless_wp
HWLOG_REGIST();

#define WP_DTS_PARA_LEN             1
#define WP_DC_CHK_INTERVAL          100 /* ms */

struct wp_para {
	int cp_iout_th;
};

struct wp_dev_info {
	struct notifier_block wp_nb;
	struct delayed_work dc_chk_work;
	struct wp_para dts_para;
	int dc_type;
};

static void wp_send_charge_mode(struct wp_dev_info *di)
{
	int ret;

	hwlog_info("[send_charge_mode] mode=%d\n", di->dc_type);
	ret = wireless_send_charge_mode(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI, (u8)di->dc_type);
	if (ret)
		hwlog_err("send_charge_mode: failed\n");
}

static void wp_dc_chk_work(struct work_struct *work)
{
	int cp_avg_iout;
	struct wp_dev_info *di = container_of(work, struct wp_dev_info, dc_chk_work.work);

	if (!di)
		return;

	if (wldc_is_stop_charging_complete())
		return;

	cp_avg_iout = wldc_get_cp_avg_iout();
	if (cp_avg_iout > di->dts_para.cp_iout_th) {
		wp_send_charge_mode(di);
		return;
	}

	schedule_delayed_work(&di->dc_chk_work, msecs_to_jiffies(WP_DC_CHK_INTERVAL));
}

static void wp_start_dc_test(struct wp_dev_info *di)
{
	u8 tx_type = 0;

	wireless_charge_get_tx_adaptor_type(&tx_type);
	if (!wlrx_is_fac_tx(tx_type))
		return;

	hwlog_info("[wp_start_test] tx_type=%u\n", tx_type);
	schedule_delayed_work(&di->dc_chk_work, msecs_to_jiffies(0));
}

static int wp_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct wp_dev_info *di = container_of(nb, struct wp_dev_info, wp_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WLC_DC_START_CHARGING:
		if (data)
			di->dc_type = *((int *)data);
		wp_start_dc_test(di);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int wp_parse_dts(struct device_node *np, struct wp_dev_info *di)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"wlc_wp_para", (u32 *)&di->dts_para, WP_DTS_PARA_LEN))
		di->dts_para.cp_iout_th = 0;

	hwlog_info("[wp_para] cp_iout_th:%dma\n", di->dts_para.cp_iout_th);
	return 0;
}

static void wp_init(struct device *dev)
{
	int ret;
	struct wp_dev_info *di = NULL;

	if (!dev || !dev->of_node)
		return;

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	dev_set_drvdata(dev, di);
	ret = wp_parse_dts(dev->of_node, di);
	if (ret)
		goto parse_dts_fail;

	INIT_DELAYED_WORK(&di->dc_chk_work, wp_dc_chk_work);
	di->wp_nb.notifier_call = wp_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_WLC, &di->wp_nb);
	if (ret)
		goto nb_register_fail;

	hwlog_info("init succ\n");
	return;

nb_register_fail:
parse_dts_fail:
	devm_kfree(dev, di);
}

static void wp_exit(struct device *dev)
{
	struct wp_dev_info *di = NULL;

	if (!dev)
		return;

	di = dev_get_drvdata(dev);
	if (!di)
		return;

	cancel_delayed_work_sync(&di->dc_chk_work);
	(void)power_event_bnc_unregister(POWER_BNT_WLC, &di->wp_nb);
	devm_kfree(dev, di);
}

static struct power_test_ops g_wp_ops = {
	.name = "wlc_wp",
	.init = wp_init,
	.exit = wp_exit,
};

static int __init wp_module_init(void)
{
	return power_test_ops_register(&g_wp_ops);
}

module_init(wp_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless wp test module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
