/*
 * series_batt_charger.c
 *
 * series batt charger driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#ifdef CONFIG_COUL_DRV
#include <linux/power/hisi/coul/coul_drv.h>
#endif
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/series_batt_charger.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <huawei_platform/power/huawei_battery_capacity.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>

#define BATTERY_NUM                 2
#define IAVG_MAX                    3000
#define IRCOMP_R_DEFAULT            25
#define IRCOMP_I_MIN_DEFAULT        200
#define IRCOMP_I_MAX_DEFAULT        1000
#define UV_TO_MV                    1000
#define DIFF_VOL_10MV               10
#define DIFF_VOL_MAX                60
#define IAVG_0MA                    0
#define ADC_OFFSET                  16
#define RECHARGE_VOL_OFFSET         100
#define VTERM_MIN                   6800
#define NO_NEED_STABLE              0
#define WORK_DELAY_TIME             5000 /* 5s */

#define HWLOG_TAG series_batt_charger
HWLOG_REGIST();

struct series_batt_info {
	struct blocking_notifier_head chg_state_nh;
	struct charge_device_ops *g_ops;
	struct charge_device_ops local_ops;
	struct notifier_block nb;
	struct platform_device *pdev;
	struct device *dev;
	struct delayed_work set_vterm_work;
	int chg_cur;
	int ircomp_r;
	int ircomp_i_min;
	int ircomp_i_max;
	int last_chg_en;
	int get_chg_en;
	int set_chg_en;
	int force_term_curr;
	int term_curr;
	int term_vol;
	int need_stable;
	int vterm_comp;
	int last_cccv;
	int i_avg;
	int ichg;
	int vbat_max;
	int vbat_min;
};

static struct series_batt_info *g_series_batt_di;

int series_batt_chg_state_notifier_register(struct notifier_block *nb)
{
	if (!g_series_batt_di || !nb)
		return -EINVAL;

	return blocking_notifier_chain_register(
		&g_series_batt_di->chg_state_nh, nb);
}

int series_batt_chg_state_notifier_unregister(struct notifier_block *nb)
{
	if (!g_series_batt_di || !nb)
		return -EINVAL;

	return blocking_notifier_chain_unregister(
		&g_series_batt_di->chg_state_nh, nb);
}

static int series_batt_vterm_cal(struct series_batt_info *di,
	int term_vol, int vterm_signal)
{
	unsigned int chg_state = CHAGRE_STATE_NORMAL;
	int vol_diff_signal, vol_diff_series;
	int vol_sum, batt_num, delta;
	int cccv_pre;
	int ret;

	if (!di->g_ops->get_terminal_voltage) {
		cccv_pre = term_vol;
	} else {
		cccv_pre = di->g_ops->get_terminal_voltage();
		if (cccv_pre < 0)
			cccv_pre = term_vol;
	}

	if (di->g_ops->get_charge_state) {
		ret = di->g_ops->get_charge_state(&chg_state);
		if (ret)
			hwlog_info("get_charge_state fail\n");
	}

	hwlog_info("cccv_pre is %d\n", cccv_pre);
	batt_num = hw_battery_get_series_num();
	vol_sum = di->vbat_min + di->vbat_max;
	vol_diff_signal = vterm_signal - di->vbat_max;
	vol_diff_series = cccv_pre - vol_sum;

	if (vol_diff_signal < 0) {
		delta = abs(vol_diff_signal * batt_num);
		term_vol = cccv_pre - delta;
		hwlog_info("ir vterm drop to %dmv,delta:%d\n", term_vol, delta);
	} else if (vol_diff_signal > DIFF_VOL_10MV &&
		vol_diff_series > DIFF_VOL_10MV) {
		if ((chg_state & CHAGRE_STATE_CV_MODE) &&
			(di->i_avg < di->ircomp_i_min)) {
			term_vol = cccv_pre + vol_diff_series;
			hwlog_info("after vol comp,term_vol:%dmv\n", term_vol);
		}
	} else {
		if ((di->need_stable > 0) && (di->last_cccv > 0) &&
			(chg_state & CHAGRE_STATE_CV_MODE) &&
			(di->i_avg < di->ircomp_i_min))
			term_vol = cccv_pre;
	}

	return term_vol;
}

static int series_batt_get_ircomp(struct series_batt_info *di)
{
	int ir_comp_i_max;
	int vol_ir = 0;
	int i_avg = di->i_avg;

	if ((di->i_avg >= di->ircomp_i_min) && (di->i_avg < IAVG_MAX)) {
		ir_comp_i_max = di->ircomp_i_max < di->chg_cur ?
			di->ircomp_i_max : di->chg_cur;
		if (di->i_avg > ir_comp_i_max)
			i_avg = ir_comp_i_max;

		vol_ir = (i_avg * di->ircomp_r) / UV_TO_MV;
		hwlog_info("vol_ir:%d ircomp_r:%d\n", vol_ir, di->ircomp_r);
	}

	return vol_ir;
}

static int series_batt_set_vterm(struct series_batt_info *di)
{
	int term_vol, vol, vol_ir, term_vol_max;

	vol = di->term_vol - ADC_OFFSET;
	if ((vol < 0) || (vol > coul_drv_battery_vbat_max())) {
		hwlog_err("vol is out of range\n");
		return -1;
	}

	if (di->vbat_min < 0) {
		hwlog_err("vbat_min is out of range\n");
		return -1;
	}

	term_vol = di->vbat_min + vol;
	term_vol = term_vol > vol * BATTERY_NUM ? vol * BATTERY_NUM : term_vol;
	hwlog_info("term_vol:%d\n", term_vol);

	vol_ir = series_batt_get_ircomp(di);
	term_vol += vol_ir;
	term_vol = series_batt_vterm_cal(di, term_vol, vol);

	term_vol_max = vol + di->vbat_max + vol_ir + di->vterm_comp;
	if (term_vol > term_vol_max)
		term_vol = term_vol_max;

	term_vol = term_vol > VTERM_MIN ? term_vol : VTERM_MIN;
	di->last_cccv = term_vol;
	hwlog_info("terminal voltage:%d\n", term_vol);
	return di->g_ops->set_terminal_voltage(term_vol);
}

static void series_batt_set_vterm_work(struct work_struct *work)
{
	struct series_batt_info *di = NULL;
	int ret;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct series_batt_info, set_vterm_work.work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	if (charge_get_charger_type() == CHARGER_REMOVED ||
		direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE) {
		hwlog_info("charger remove or direct charger\n");
		cancel_delayed_work(&di->set_vterm_work);
		return;
	}

	di->i_avg = coul_drv_battery_current_avg();
	di->ichg = -coul_drv_battery_current();
	di->vbat_max = hw_battery_voltage(BAT_ID_MAX);
	di->vbat_min = hw_battery_voltage(BAT_ID_MIN);
	hwlog_info("ichg:%d, i_avg:%d, vbat_max:%d, vbat_min:%d\n",
		di->ichg, di->i_avg, di->vbat_max, di->vbat_min);

	ret = series_batt_set_vterm(di);
	if (ret < 0)
		hwlog_err("set termination voltage fail\n");

	schedule_delayed_work(&di->set_vterm_work,
		msecs_to_jiffies(WORK_DELAY_TIME));
}

static int series_batt_set_terminal_voltage(int vol)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->set_terminal_voltage) {
		hwlog_err("g_ops or set_terminal_voltage is null\n");
		return -1;
	}

	di->term_vol = vol;
	schedule_delayed_work(&di->set_vterm_work,
		msecs_to_jiffies(0));
	return 0;
}

static int series_batt_set_charge_current(int value)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->set_charge_current) {
		hwlog_err("g_ops or is set_charge_current null\n");
		return -1;
	}

	di->chg_cur = value;
	hwlog_info("charger current limit is [%d]ma\n", value);
	return di->g_ops->set_charge_current(value);
}

static int series_batt_set_charge_enable(int enable)
{
	struct series_batt_info *di = g_series_batt_di;
	int chg_en;

	if (!di || !di->g_ops || !di->g_ops->set_charge_enable) {
		hwlog_err("g_ops or set_charge_enable is null\n");
		return -1;
	}

	if (di->last_chg_en != enable) {
		hwlog_info("enable changed\n");
		if (enable)
			blocking_notifier_call_chain(&di->chg_state_nh,
				SERIES_BATT_CHG_EN, NULL);
		else
			blocking_notifier_call_chain(&di->chg_state_nh,
				SERIES_BATT_CHG_DISABLE, NULL);
	}

	if (enable == 0)
		di->last_cccv = 0;

	di->last_chg_en = enable;
	chg_en = enable && di->set_chg_en;
	di->get_chg_en = chg_en;
	hwlog_info("old_en:%d,chg_en=%d\n", enable, chg_en);
	return di->g_ops->set_charge_enable(chg_en);
}

static int series_batt_get_charge_state(unsigned int *state)
{
	struct series_batt_info *di = g_series_batt_di;
	int ret;
	unsigned int charger_type;

	if (!state || !di || !di->g_ops || !di->g_ops->get_charge_state) {
		hwlog_err("g_ops or get_charge_state is null\n");
		return -1;
	}

	ret = di->g_ops->get_charge_state(state);
	if (ret) {
		hwlog_err("get_charge_state fail\n");
		return ret;
	}

	if (*state & CHAGRE_STATE_CHRG_DONE)
		blocking_notifier_call_chain(&di->chg_state_nh,
			SERIES_BATT_CHG_DONE, NULL);

	charger_type = charge_get_charger_type();
	if (di->force_term_curr && (charger_type != CHARGER_REMOVED)) {
		if (hw_battery_voltage(BAT_ID_MAX) >=
			di->term_vol - RECHARGE_VOL_OFFSET)
			*state |= CHAGRE_STATE_CHRG_DONE;
	}

	return ret;
}

static int series_batt_set_terminal_current(int value)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->set_terminal_current) {
		hwlog_err("g_ops or set_terminal_current is null\n");
		return -1;
	}

	if (di->force_term_curr) {
		di->term_curr = di->force_term_curr;
		return di->g_ops->set_terminal_current(di->force_term_curr);
	}

	hwlog_info("terminal current is [%d]ma\n", value);
	di->term_curr = value;
	return di->g_ops->set_terminal_current(value);
}

static int series_batt_set_charger_hiz(int enable)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->set_charger_hiz) {
		hwlog_err("g_ops or set_charger_hiz is null\n");
		return -1;
	}

	if (enable)
		blocking_notifier_call_chain(&di->chg_state_nh,
			SERIES_BATT_HIZ_EN, NULL);
	else
		blocking_notifier_call_chain(&di->chg_state_nh,
			SERIES_BATT_HIZ_DISABLE, NULL);

	di->last_cccv = 0;
	hwlog_info("set_charger_hiz,enable:%d\n", enable);
	return di->g_ops->set_charger_hiz(enable);
}

static int series_batt_chip_init(struct chip_init_crit *init_crit)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->chip_init) {
		hwlog_err("di or g_ops or chip_init is null\n");
		return -1;
	}

	g_series_batt_di->last_cccv = 0;
	di->set_chg_en = CHARGE_ENABLE;
	return g_series_batt_di->g_ops->chip_init(init_crit);
}

int series_batt_set_charge_en(int enable)
{
	struct series_batt_info *di = g_series_batt_di;

	if (!di || !di->g_ops || !di->g_ops->set_charge_enable) {
		hwlog_err("di or g_ops or set_charge_enable is null\n");
		return -1;
	}

	hwlog_info("set_charge_en:en=%d\n", enable);
	g_series_batt_di->set_chg_en = enable;

	if (di->last_chg_en == 0)
		enable = 0;

	return g_series_batt_di->g_ops->set_charge_enable(enable);
}

int series_batt_set_term_curr(int value)
{
	if (!g_series_batt_di) {
		hwlog_err("g_series_batt_di is null\n");
		return -1;
	}

	hwlog_info("set_term_curr:curr=%d\n", value);
	g_series_batt_di->force_term_curr = value;
	return series_batt_set_terminal_current(value);
}

int series_batt_get_term_curr(void)
{
	if (!g_series_batt_di) {
		hwlog_err("g_series_batt_di is null\n");
		return -1;
	}

	return g_series_batt_di->term_curr;
}

int series_batt_get_vterm_single(void)
{
	if (!g_series_batt_di) {
		hwlog_err("g_series_batt_di is null\n");
		return -1;
	}

	return g_series_batt_di->term_vol;
}

int series_batt_ops_register(struct charge_device_ops *ops)
{
	struct series_batt_info *di = g_series_batt_di;
	int ret;

	if (!ops || !di) {
		hwlog_err("ops or di is null\n");
		return -1;
	}

	di->g_ops = ops;
	di->local_ops = *di->g_ops;
	di->local_ops.set_terminal_voltage = series_batt_set_terminal_voltage;
	di->local_ops.set_charge_current = series_batt_set_charge_current;
	di->local_ops.set_charge_enable = series_batt_set_charge_enable;
	di->local_ops.get_charge_state = series_batt_get_charge_state;
	di->local_ops.set_terminal_current = series_batt_set_terminal_current;
	di->local_ops.set_charger_hiz = series_batt_set_charger_hiz;
	di->local_ops.chip_init = series_batt_chip_init;

	ret = charge_ops_register(&di->local_ops);
	if (ret) {
		hwlog_err("register charge ops failed\n");
		return ret;
	}

	return ret;
}

static int get_series_batt_chargelog_head(char *chargelog, int size, void *dev_data)
{
	if (!chargelog) {
		hwlog_err("chargelog is null\n");
		return -1;
	}

	snprintf(chargelog, size, "%s",
		"bat0_temp   bat1_temp   mixed_temp   bat0_vol   bat1_vol   series_vol   Ibus   Vbus   ");

	return 0;
}

static int get_series_batt_chargelog(char *chargelog, int size, void *dev_data)
{
	int bat0_temp = 0;
	int bat1_temp = 0;
	int mixed_temp = 0;
	int bat0_vol;
	int bat1_vol;
	int series_vol;
	int ibus = 0;
	unsigned int vbus = 0;

	if (!chargelog) {
		hwlog_err("chargelog is null\n");
		return -1;
	}

	bat_temp_get_temperature(BAT_TEMP_1, &bat1_temp);
	bat_temp_get_temperature(BAT_TEMP_0, &bat0_temp);
	bat_temp_get_temperature(BAT_TEMP_MIXED, &mixed_temp);
	bat1_vol = hw_battery_voltage(BAT_ID_1);
	bat0_vol = hw_battery_voltage(BAT_ID_0);
	series_vol = hw_battery_voltage(BAT_ID_ALL);
	if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE) {
		ibus = direct_charge_get_ibus();
		vbus = direct_charge_get_vbus();
	} else {
		if (g_series_batt_di && g_series_batt_di->g_ops &&
			g_series_batt_di->g_ops->get_ibus &&
			g_series_batt_di->g_ops->get_vbus) {
			ibus = g_series_batt_di->g_ops->get_ibus();
			g_series_batt_di->g_ops->get_vbus(&vbus);
		}
	}

	snprintf(chargelog, size,
		"%-9d   %-9d   %-8d   %-8d   %-8d   %-10d%-4d   %-4d   ",
		bat0_temp, bat1_temp, mixed_temp,
		bat0_vol, bat1_vol, series_vol, ibus, vbus);
	return 0;
}

static int series_batt_recv_dc_status_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct series_batt_info *di = NULL;

	if (!nb)
		return NOTIFY_OK;

	di = container_of(nb, struct series_batt_info, nb);
	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_DC_LVC_CHARGING:
	case POWER_NE_DC_SC_CHARGING:
	case POWER_NE_DC_STOP_CHARGE:
		break;
	default:
		return NOTIFY_OK;
	}

	di->last_cccv = 0;
	return NOTIFY_OK;
}

static struct power_log_ops series_batt_log_ops = {
	.dev_name = "series_batt",
	.dump_log_head = get_series_batt_chargelog_head,
	.dump_log_content = get_series_batt_chargelog,
};

static void series_batt_parse_dts(struct device_node *np,
	struct series_batt_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ircomp_r", &di->ircomp_r, IRCOMP_R_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ircomp_i_min", &di->ircomp_i_min, IRCOMP_I_MIN_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ircomp_i_max", &di->ircomp_i_max, IRCOMP_I_MAX_DEFAULT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"need_stable", &di->need_stable, NO_NEED_STABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vterm_comp", &di->vterm_comp, DIFF_VOL_MAX);
}

static int series_batt_probe(struct platform_device *pdev)
{
	struct series_batt_info *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->g_ops = NULL;
	g_series_batt_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;

	series_batt_parse_dts(np, di);
	INIT_DELAYED_WORK(&di->set_vterm_work, series_batt_set_vterm_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&di->chg_state_nh);
	platform_set_drvdata(pdev, di);

	di->nb.notifier_call = series_batt_recv_dc_status_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_DC, &di->nb);
	if (ret)
		hwlog_err("register scp_charge_stage notifier failed\n");

	di->set_chg_en = CHARGE_ENABLE;
	di->force_term_curr = DEFAULT_VALUE;
	di->last_chg_en = DEFAULT_VALUE;
	di->term_curr = 0;
	di->term_vol = coul_drv_battery_vbat_max();
	series_batt_log_ops.dev_data = (void *)di;
	power_log_ops_register(&series_batt_log_ops);
	di->last_cccv = 0;
	return 0;
}

static int series_batt_remove(struct platform_device *pdev)
{
	struct series_batt_info *info = platform_get_drvdata(pdev);

	if (!info)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_series_batt_di = NULL;

	return 0;
}

static const struct of_device_id series_batt_match_table[] = {
	{
		.compatible = "huawei,series_batt_charger",
		.data = NULL,
	},
	{},
};

static struct platform_driver series_batt_driver = {
	.probe = series_batt_probe,
	.remove = series_batt_remove,
	.driver = {
		.name = "huawei,series_batt_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(series_batt_match_table),
	},
};

static int __init series_batt_init(void)
{
	return platform_driver_register(&series_batt_driver);
}

static void __exit series_batt_exit(void)
{
	platform_driver_unregister(&series_batt_driver);
}

rootfs_initcall(series_batt_init);
module_exit(series_batt_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("series batt charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
