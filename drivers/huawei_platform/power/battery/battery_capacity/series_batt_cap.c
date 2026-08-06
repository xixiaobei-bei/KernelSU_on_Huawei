/*
 * series_batt_cap.c
 *
 * huawei dual series battery capacity
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/timekeeping.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_battery_capacity.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/battery_balance.h>
#include <huawei_platform/power/huawei_battery_temp.h>
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger.h>
#endif

#define HWLOG_TAG series_batt_cap
HWLOG_REGIST();

#define SERCAP_NV_NUM 418
#define SERCAP_DEF_TEMP 25
#define SERCAP_DEF_RATIO 1000
#define SERCAP_BAT_ID0 0
#define SERCAP_BAT_ID1 1
#define SERCAP_VOL_CNT 3
#define SERCAP_FULL_CAP 1000
#define SERCAP_SOC_FAST 100
#define SERCAP_LOW_VOL 3530
#define SERCAP_SHUTDOWN 2700
#define SERCAP_PRE_SHUTDOWN 300
#define SERCAP_TENTH 10
#define SERCAP_INTERVAL_NOR (15 * 1000)
#define SERCAP_INTERVAL_S1 (10 * 1000)
#define SERCAP_DEBOUNCE 300
#define SERCAP_PER_1 10
#define SERCAP_PER_2 20
#define SERCAP_PER_3 30
#define SERCAP_CURRENT_LIMIT 150
#define SERCAP_TRACK_DONE_CURRENT 200
#define SERCAP_OCV_TEMP_LIMIT 5
#define SERCAP_TEMP_LOW (-5)
#define SERCAP_TEMP_LOW_SHUTDOWN 2600
#define SERCAP_DEF_RBAT 120
#define SERCAP_DENSE_L0 3680
#define SERCAP_DENSE_H0 3880
#define SERCAP_VOL_DIFF_LIMIT 15
#define SERCAP_UPDATE_SOC_OCV 50
#define SERCAP_VOL_TEMP_LIMIT 15
#define SERCAP_VOL_CUR_LIMIT 80
#define SERCAP_UPDATE_SOC_VOL 300
#define SERCAP_TRACK_TIMEOUT (48 * 60 * 60) /* 72 Hour */
#define SERCAP_VOL_FULL_DELTA 40
#define SERCAP_DEF_FULL_VOL 4400
#define SERCAP_TRACK_SOC_LOW 30
#define SERCAP_TRACK_TEMP 10
#define SERCAP_TRACK_VOL_LOW 3600
#define SERCAP_RATIO_VALID 300
#define SERCAP_UV2MV 1000
#define SERCAP_RECHG_PROTECT 150

enum sercap_track_type {
	TRACK_TYPE_DIS,
	TRACK_TYPE_CHG,
};

enum sercap_track_state {
	SERCAP_TRACK_IDLE,
	SERCAP_TRACK_RUNNING,
};

struct sercap_track {
	time64_t time;
	int state;
	int type;
	int soc0_s; /* soc start */
	int soc1_s;
};

struct sercap_xy {
	int x;
	int y;
};

struct series_capacity {
	struct device *dev;
	struct power_supply *batt;
	struct notifier_block chg_nb;
	struct notifier_block bal_nb;
	struct delayed_work monitor;
	struct wake_lock wakelock;
	int fcc_ratio;
	int mixed_cap;
	int bat0_cap; /* cap with uuc */
	int bat1_cap;
	int pre_bat0_cap;
	int bat0_soc; /* soc real unfilter */
	int bat1_soc;
	int bat0_start;
	int bat1_start;
	int bat0_ocv;
	int avg_i;
	int cur;
	int bat0_vol;
	int bat1_vol;
	int bat0_temp;
	int bat1_temp;
	int psy_state;
	int chg_state;
	int bal_event;
	int ui_base_bat;
	int ui_bat_start;
	int ui_mixed_start;
	int ui_ratio;
	unsigned int interval;
	struct sercap_track track;
};

static unsigned long cmdline_ratio;
static unsigned long cmdline_vol;
static struct series_capacity *series_cap;
/* temperature - low voltage table */
static struct sercap_xy sercap_low_vol[] = {
	{ 10, SERCAP_LOW_VOL },
	{ SERCAP_TEMP_LOW, SERCAP_TEMP_LOW_SHUTDOWN },
};

static int sercap_interpolate(struct sercap_xy *tab, int size, int x)
{
	int i;
	int x1, x2, y1, y2;

	if (size <= 0 || !tab)
		return 0;

	if (x >= tab[0].x)
		return tab[0].y;
	if (x <= tab[size - 1].x)
		return tab[size - 1].y;

	for (i = 1; i < size; i++) {
		if (x >= tab[i].x)
			break;
	}

	x1 = tab[i - 1].x;
	y1 = tab[i - 1].y;
	x2 = tab[i].x;
	y2 = tab[i].y;
	/* tab[i - 1].x never equal to tab[i].x, i-1 is compared early */
	return (x - x2) * (y1 - y2) / (x1 - x2) + y2;
}

static int __init sercap_parse_cmdline(char *p)
{
	char *token = NULL;

	if (!p) {
		hwlog_err("point is null\n");
		return -1;
	}
	hwlog_info("point=%s\n", p);

	token = strsep(&p, ",");
	if (token) {
		if (kstrtoul(token, 0, &cmdline_ratio) != 0)
			cmdline_ratio = 0;
	}
	hwlog_info("cmdline_ratio=%ld\n", cmdline_ratio);
	token = strsep(&p, ",");
	if (token) {
		if (kstrtol(token, 0, &cmdline_vol) != 0)
			cmdline_vol = 0;
	}
	hwlog_info("cmdline_vol=%ld\n", cmdline_vol);

	return 0;
}
early_param("series_batt", sercap_parse_cmdline);

static int sercap_get_cmdline_vol(int *vol0, int *vol1)
{
	int v0, v1;

	/* low 16bit = voltage1, high 16bit = voltage0 */
	v1 = cmdline_vol & 0xFFFF;
	v0 = (cmdline_vol >> 16) & 0xFFFF;

	/* v0, v1 must be higher 2000mv and lower 5000mv */
	if (v0 < 2000 || v0 > 5000)
		return -EIO;
	if (v1 < 2000 || v1 > 5000)
		return -EIO;
	*vol0 = v0;
	*vol1 = v1;
	return 0;
}

static inline int sercap_vol2ocv(int vol, int cur)
{
	/* vol(mV) + i(mA)*r(mOhm) / 1000 = ocv(mV) */
	return vol + cur * SERCAP_DEF_RBAT / 1000;
}

static int sercap_vbat1_voltage(void)
{
	int bat1_vol;
	int i;
	int cnt = 0;
	int sum = 0;

	/* read 5 times for average voltage */
	for (i = 0; i < 5; i++) {
		bat1_vol = hw_battery_voltage(BAT_ID_1);
		if (bat1_vol > 0) {
			sum += bat1_vol;
			cnt++;
		}
	}

	if (cnt > 0)
		return bat1_vol = sum / cnt;
	else
		return -EIO;
}

static int sercap_get_batt_state(void)
{
	int ret;
	struct power_supply *psy = NULL;
	union power_supply_propval val;

	psy = power_supply_get_by_name("Battery");
	if (!psy)
		return POWER_SUPPLY_STATUS_UNKNOWN;

	ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_STATUS, &val);

	power_supply_put(psy);
	if (ret)
		return POWER_SUPPLY_STATUS_UNKNOWN;

	return val.intval;
}

static int sercap_adjust_cap(struct series_capacity *di, int state, int cap)
{
	int out = cap;

	if (out < 0)
		out = 0;

	if (out > SERCAP_FULL_CAP)
		out = SERCAP_FULL_CAP;

	switch (state) {
	/* fall-through */
	case POWER_SUPPLY_STATUS_DISCHARGING:
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		if (out > di->mixed_cap) {
			hwlog_info("discharge out(%d) > mixed_cap\n", out);
			out = di->mixed_cap;
		}
		if (di->bat1_cap == 0 || di->bat0_cap == 0) {
			hwlog_info("discharge bat0 or bat1 is 0\n");
			out = 0;
		}

		if (out < di->bat0_cap && out < di->bat1_cap) {
			hwlog_info("dischg out(%d) < all cap\n", out);
			out = di->bat0_cap < di->bat1_cap ?
				di->bat0_cap : di->bat1_cap;
		}
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		if (out < di->mixed_cap && di->avg_i > 0) {
			hwlog_info("charge out(%d) < mixed_cap\n", out);
			out = di->mixed_cap;
		}
		if (di->bat1_cap == SERCAP_FULL_CAP ||
			di->bat0_cap == SERCAP_FULL_CAP) {
			hwlog_info("charging bat0 or bat1 is 100\n");
			out = SERCAP_FULL_CAP;
		}

		if (out < di->bat0_cap && out < di->bat1_cap) {
			hwlog_info("chg out(%d) < all cap\n", out);
			out = di->bat0_cap < di->bat1_cap ?
				di->bat0_cap : di->bat1_cap;
		} else if (out > di->bat0_cap && out > di->bat1_cap) {
			hwlog_info("chg out(%d) > all cap\n", out);
			out = di->bat0_cap > di->bat1_cap ?
				di->bat0_cap : di->bat1_cap;
		}
		break;
	case POWER_SUPPLY_STATUS_FULL:
		if (hisi_battery_current_avg() >= 0) {
			/* if multi battery, get MAX voltage */
			if (hw_battery_voltage(BAT_ID_MAX) >=
				(hisi_battery_vbat_max() -
				SERCAP_RECHG_PROTECT)) {
				out = SERCAP_FULL_CAP;
				hwlog_info("force soc 100\n");
			}
		}
		break;
	default:
		break;
	}
	return out;
}

static void sercap_calc_first_empty(struct series_capacity *di,
	int *bat_id, int *bat_start, int *ui_ratio)
{
	int id, start, ratio;

	/* is_bat1_first_empty */
	if (di->bat1_cap * di->fcc_ratio / SERCAP_DEF_RATIO <=
	    di->bat0_cap) {
		id = SERCAP_BAT_ID1;
		start = di->bat1_cap;
	} else {
		id = SERCAP_BAT_ID0;
		start = di->bat0_cap;
	}

	if (start == 0)
		ratio = SERCAP_DEF_RATIO;
	else
		ratio = di->mixed_cap * SERCAP_DEF_RATIO / start;

	*bat_id = id;
	*bat_start = start;
	*ui_ratio = ratio;
}

static void sercap_calc_first_full(struct series_capacity *di,
	int *bat_id, int *bat_start, int *ui_ratio)
{
	int id, start, ratio;

	/* is_bat1_first_full */
	if ((SERCAP_FULL_CAP - di->bat1_cap) *
	    di->fcc_ratio / SERCAP_DEF_RATIO <=
	    (SERCAP_FULL_CAP - di->bat0_cap)) {
		id = SERCAP_BAT_ID1;
		start = di->bat1_cap;
	} else {
		id = SERCAP_BAT_ID0;
		start = di->bat0_cap;
	}

	if ((SERCAP_FULL_CAP - start) == 0)
		ratio = SERCAP_DEF_RATIO;
	else
		ratio = (SERCAP_FULL_CAP - di->mixed_cap) *
			SERCAP_DEF_RATIO / (SERCAP_FULL_CAP - start);

	*bat_id = id;
	*bat_start = start;
	*ui_ratio = ratio;
}

static void sercap_adjust_uiratio(struct series_capacity *di, int state)
{
	int id = di->ui_base_bat;
	int start = di->ui_bat_start;
	int ratio = di->ui_ratio;

	switch (state) {
	/* fall-through */
	case POWER_SUPPLY_STATUS_DISCHARGING:
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		sercap_calc_first_empty(di, &id, &start, &ratio);
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		if (di->pre_bat0_cap > di->bat0_cap || di->avg_i < 0) {
			hwlog_info("cap decrease when chg pre:%d,cap:%d\n",
				di->pre_bat0_cap, di->bat0_cap);
			sercap_calc_first_empty(di, &id, &start, &ratio);
		} else {
			sercap_calc_first_full(di, &id, &start, &ratio);
		}
		break;
	case POWER_SUPPLY_STATUS_FULL:
		if (di->bat1_cap > di->bat0_cap) {
			id = SERCAP_BAT_ID1;
			start = di->bat1_cap;
		} else {
			id = SERCAP_BAT_ID0;
			start = di->bat0_cap;
		}
		ratio = SERCAP_DEF_RATIO;
		break;
	default:
		id = SERCAP_BAT_ID0;
		start = di->bat0_cap;
		ratio = SERCAP_DEF_RATIO;
		break;
	}

	if (di->ui_base_bat != id || di->psy_state != state) {
		di->ui_base_bat = id;
		di->ui_mixed_start = di->mixed_cap;
		di->ui_bat_start = start;
		di->ui_ratio = ratio;
		if (di->ui_ratio == 0) {
			hwlog_info("ratio can not is 0\n");
			di->ui_ratio = SERCAP_DEF_RATIO;
			di->ui_mixed_start = di->ui_bat_start;
		}
		hwlog_info("ui ratio change:%d\n", ratio);
	}
}

static int sercap_bal_query_info(void)
{
	struct bal_info info;

	if (batt_bal_get_info(&info))
		return 0;

	return (int)info.sum_gauge;
}

static inline int sercap_get_bat0_cap(struct series_capacity *di)
{
	int cap = hisi_battery_capacity() * SERCAP_TENTH;

	if (cap < 0)
		cap = 0;

	if (cap > SERCAP_FULL_CAP)
		cap = SERCAP_FULL_CAP;

	return cap;
}

static inline int sercap_get_bat1_cap(struct series_capacity *di)
{
	int cap;

	cap = di->bat0_cap + di->bat1_soc - di->bat0_soc;

	if (cap < 0)
		cap = 0;

	if (cap > SERCAP_FULL_CAP)
		cap = SERCAP_FULL_CAP;

	return cap;
}

static inline bool sercap_is_dense_area(struct series_capacity *di)
{
	int vol = sercap_vol2ocv(di->bat1_vol, di->cur);

	if (vol > SERCAP_DENSE_L0 && vol < SERCAP_DENSE_H0)
		return true;
	return false;
}

static int sercap_try_adjust_by_ocv(struct series_capacity *di, int in_soc)
{
	int soc = in_soc;
	int v0, v1, ocv, soc_by_ocv;

	if (di->bat0_temp < SERCAP_OCV_TEMP_LIMIT ||
	    di->bat1_temp < SERCAP_OCV_TEMP_LIMIT)
		return soc;

	if (abs(di->avg_i) > SERCAP_CURRENT_LIMIT ||
	    abs(di->cur) > SERCAP_CURRENT_LIMIT)
		return soc;

	if (sercap_is_dense_area(di))
		return soc;

	/* wait a moment, check voltage again */
	msleep(SERCAP_DEBOUNCE);
	v1 = sercap_vbat1_voltage();
	v0 = hw_battery_voltage(BAT_ID_0);
	/* current is jumping */
	if (abs(v0 - di->bat0_vol) > SERCAP_VOL_DIFF_LIMIT ||
	    abs(v1 - di->bat1_vol) > SERCAP_VOL_DIFF_LIMIT)
		return soc;
	/* average voltage */
	v0 = (v0 + di->bat0_vol) / 2;
	v1 = (v1 + di->bat1_vol) / 2;
	ocv = di->bat0_ocv + v1 - v0;
	soc_by_ocv = hisi_coul_get_soc_by_ocv(di->bat1_temp, ocv);
	hwlog_info("soc_by_ocv:%d\n", soc_by_ocv);

	if (abs(soc_by_ocv - soc) > SERCAP_UPDATE_SOC_OCV) {
		hwlog_info("soc1 change by ocv\n");
		hwlog_info("v0:%d,v1:%d,ocv0:%d,ocv1:%d,soc:%d,soc_new:%d\n",
			   v0, v1, di->bat0_ocv, ocv, soc, soc_by_ocv);
		soc = soc_by_ocv;
	}
	return soc;
}

static int sercap_try_adjust_by_vol(struct series_capacity *di, int in_soc)
{
	int soc = in_soc;
	int v1, soc_by_ocv, cur;
	int ocv = sercap_vol2ocv(di->bat1_vol, di->cur);

	if (di->bat1_temp < SERCAP_VOL_TEMP_LIMIT)
		return soc;

	if (abs(di->avg_i) > SERCAP_VOL_CUR_LIMIT ||
	    abs(di->cur) > SERCAP_VOL_CUR_LIMIT)
		return soc;

	if (sercap_is_dense_area(di))
		return soc;

	soc_by_ocv = hisi_coul_get_soc_by_ocv(di->bat1_temp, ocv);

	if (abs(soc_by_ocv - soc) < SERCAP_UPDATE_SOC_VOL)
		return soc;
	/* wait a moment, check voltage again */
	msleep(SERCAP_DEBOUNCE);
	v1 = sercap_vbat1_voltage();
	cur = -hisi_battery_current();
	/* current is jumping */
	if (abs(v1 - di->bat1_vol) > SERCAP_VOL_DIFF_LIMIT ||
	    abs(di->cur) > SERCAP_VOL_CUR_LIMIT)
		return soc;
	/* average voltage and current */
	cur = (cur + di->cur) / 2;
	v1 = (v1 + di->bat0_vol) / 2;
	ocv = sercap_vol2ocv(v1, cur);
	soc_by_ocv = hisi_coul_get_soc_by_ocv(di->bat1_temp, ocv);
	hwlog_info("soc_by_vol:%d\n", soc_by_ocv);

	if (abs(soc_by_ocv - soc) > SERCAP_UPDATE_SOC_VOL) {
		hwlog_info("soc1 change by vol\n");
		hwlog_info("cur:%d,v1:%d,ocv0:%d,ocv1:%d,soc:%d,soc_new:%d\n",
			   cur, v1, di->bat0_ocv, ocv, soc, soc_by_ocv);
		/* average soc for new */
		soc = (soc_by_ocv + soc) / 2;
	}
	return soc;
}

static bool sercap_bat1_chg_is_full(struct series_capacity *di)
{
	AGING_SAFE_POLICY_TYPE basp = { 0 };
	int full_vol = hisi_battery_vbat_max();
	struct bal_info info = { 0 };
	enum bal_state bal_state;

	if (full_vol <= 0)
		full_vol = SERCAP_DEF_FULL_VOL;

	if (!hisi_battery_aging_safe_policy(&basp))
		full_vol -= basp.nondc_volt_dec;
	full_vol -= SERCAP_VOL_FULL_DELTA;

	if (batt_bal_get_info(&info))
		bal_state = BAL_STATE_IDLE;
	else
		bal_state = info.bal_state;
	if (di->chg_state == VCHRG_CHARGE_DONE_EVENT &&
	    di->bat1_vol > full_vol &&
	    abs(di->avg_i) < SERCAP_CURRENT_LIMIT &&
	    (bal_state == BAL_STATE_IDLE ||
	    (bal_state != BAL_STATE_IDLE && info.bat_id != BATT_ID_0)))
		return true;

	return false;
}

static bool sercap_bat0_chg_is_full(struct series_capacity *di)
{
	AGING_SAFE_POLICY_TYPE basp = { 0 };
	int full_vol = hisi_battery_vbat_max();
	struct bal_info info = { 0 };
	enum bal_state bal_state;

	if (full_vol <= 0)
		full_vol = SERCAP_DEF_FULL_VOL;

	if (!hisi_battery_aging_safe_policy(&basp))
		full_vol -= basp.nondc_volt_dec;
	full_vol -= SERCAP_VOL_FULL_DELTA;

	if (batt_bal_get_info(&info))
		bal_state = BAL_STATE_IDLE;
	else
		bal_state = info.bal_state;
	if (di->chg_state == VCHRG_CHARGE_DONE_EVENT &&
	    di->bat0_vol > full_vol &&
	    abs(di->avg_i) < SERCAP_CURRENT_LIMIT &&
	    (bal_state == BAL_STATE_IDLE ||
	    (bal_state != BAL_STATE_IDLE && info.bat_id != BATT_ID_1)))
		return true;

	return false;
}

static int sercap_try_adjust_by_chg(struct series_capacity *di, int in_soc)
{
	int soc = in_soc;

	if (sercap_bat1_chg_is_full(di) && soc < SERCAP_FULL_CAP) {
		/* battery is full */
		hwlog_info("bat1 charge full, soc is 100\n");
		soc = SERCAP_FULL_CAP;
	}
	return soc;
}

static int sercap_adjust_bat1_soc(struct series_capacity *di, int bat1_soc)
{
	int soc = bat1_soc;
	int vbat1 = di->bat1_vol;
	int low_vol;

	int ocv = hisi_coul_get_ocv() / SERCAP_UV2MV;

	if (ocv != di->bat0_ocv) {
		hwlog_info("bat0_ocv change:%d->%d\n", di->bat0_ocv, ocv);
		di->bat0_ocv = ocv;
		soc = sercap_try_adjust_by_ocv(di, soc);
	}

	soc = sercap_try_adjust_by_vol(di, soc);

	soc = sercap_try_adjust_by_chg(di, soc);

	if (di->psy_state == POWER_SUPPLY_STATUS_CHARGING ||
		di->psy_state == POWER_SUPPLY_STATUS_FULL)
		return soc;

	low_vol = sercap_interpolate(sercap_low_vol,
			sizeof(sercap_low_vol) / sizeof(struct sercap_xy),
			di->bat1_temp);

	if (low_vol <= 0)
		low_vol = SERCAP_LOW_VOL;

	hwlog_info("low_vol level:%d\n", low_vol);
	if (vbat1 > low_vol && soc < SERCAP_PER_3) {
		hwlog_info("soc is low,vol:%d, low_vol:%d\n", vbat1, low_vol);
		soc = SERCAP_PER_3;
	} else if ((vbat1 < SERCAP_PRE_SHUTDOWN + SERCAP_SHUTDOWN) &&
	    soc > SERCAP_PER_1) {
		int cnt = SERCAP_VOL_CNT;

		hwlog_info("pre shut down:%d\n", vbat1);
		/* confirm it is true low power */
		do {
			msleep(SERCAP_DEBOUNCE);
			vbat1 = sercap_vbat1_voltage();
		} while (vbat1 < (SERCAP_PRE_SHUTDOWN + SERCAP_SHUTDOWN) &&
			 --cnt > 0);
		if (cnt <= 0) {
			hwlog_info("pre shut down true:%d\n", vbat1);
			di->interval = SERCAP_INTERVAL_S1;
			soc = SERCAP_PER_1;
		}
	} else if (vbat1 < SERCAP_SHUTDOWN) {
		int cnt = SERCAP_VOL_CNT;
		/* confirm it is true low power */
		hwlog_info("shut down:%d\n", vbat1);
		do {
			msleep(SERCAP_DEBOUNCE);
			vbat1 = sercap_vbat1_voltage();
		} while (vbat1 < SERCAP_SHUTDOWN && --cnt > 0);
		if (cnt <= 0) {
			di->interval = SERCAP_INTERVAL_S1;
			soc = 0;
			hwlog_info("shut down true:%d\n", vbat1);
		}
	}

	return soc;
}

static inline int sercap_calc_bat1_soc(struct series_capacity *di)
{
	int soc;
	int cc = sercap_bal_query_info();
	int fcc = hisi_battery_fcc();
	int bal_soc = 0;

	if (di->fcc_ratio == 0)
		return di->bat0_soc;
	if (fcc != 0)
		bal_soc = cc / fcc * SERCAP_FULL_CAP;
	/* if charge balance is running, need compensate soc in here */
	soc = (di->bat0_soc - di->bat0_start) *
	    SERCAP_DEF_RATIO /
	    di->fcc_ratio + di->bat1_start +
	    (bal_soc * SERCAP_DEF_RATIO) / di->fcc_ratio;
	return soc;
}

static int sercap_cap_smooth(struct series_capacity *di, int cap)
{
	int delta = di->mixed_cap - cap;

	if (abs(delta) > SERCAP_PER_1)
		delta = delta > 0 ? SERCAP_PER_1 : -SERCAP_PER_1;
	di->mixed_cap -= delta;

	return di->mixed_cap;
}

static void sercap_select_work_interval(struct series_capacity *di, int cap)
{
	/* discharge */
	if (cap < di->mixed_cap && cap < SERCAP_SOC_FAST)
		di->interval = SERCAP_INTERVAL_S1;
	else
		di->interval = SERCAP_INTERVAL_NOR;
}

void sercap_get_base_info(struct series_capacity *di)
{
	int temp0 = SERCAP_DEF_TEMP;
	int temp1 = SERCAP_DEF_TEMP;

	di->avg_i = hisi_battery_current_avg();
	di->cur = -hisi_battery_current();
	di->bat1_vol = sercap_vbat1_voltage();
	di->bat0_vol = hw_battery_voltage(BAT_ID_0);
	huawei_battery_temp(BAT_TEMP_0, &temp0);
	huawei_battery_temp(BAT_TEMP_1, &temp1);
	di->bat0_temp = temp0;
	di->bat1_temp = temp1;
}

static int sercap_calc_cap(struct series_capacity *di)
{
	int state = sercap_get_batt_state();
	int cap, mixed_cap, adj_soc, soc;

	if (!is_hisi_battery_exist()) {
		di->mixed_cap = sercap_get_bat0_cap(di);
		return di->mixed_cap;
	}

	sercap_get_base_info(di);

	di->bat0_soc = hisi_coul_battery_ufcapacity_tenth();
	soc = sercap_calc_bat1_soc(di);

	adj_soc = sercap_adjust_bat1_soc(di, soc);
	di->bat1_soc = adj_soc;
	if (adj_soc != soc) {
		hwlog_info("adj_soc change:%d -> %d\n", soc, adj_soc);
		di->bat0_start = di->bat0_soc;
		di->bat1_start = di->bat1_soc;
	}

	di->pre_bat0_cap = di->bat0_cap;
	di->bat0_cap = sercap_get_bat0_cap(di);
	di->bat1_cap = sercap_get_bat1_cap(di);

	cap = (di->ui_base_bat == SERCAP_BAT_ID0) ? di->bat0_cap : di->bat1_cap;
	mixed_cap = (cap - di->ui_bat_start) * di->ui_ratio /
	    SERCAP_DEF_RATIO + di->ui_mixed_start;

	mixed_cap = sercap_adjust_cap(di, state, mixed_cap);

	sercap_select_work_interval(di, mixed_cap);

	di->mixed_cap = sercap_cap_smooth(di, mixed_cap);

	if (state != di->psy_state &&
	    state == POWER_SUPPLY_STATUS_DISCHARGING) {
		int cc = sercap_bal_query_info();
		int fcc = hisi_battery_fcc();

		hwlog_info("psy discharge reset bal info\n");
		if (fcc != 0)
			di->bat1_start += cc / fcc * SERCAP_FULL_CAP;
		/* reset balance info for next charge bal info */
		batt_bal_reset_info();
	}

	sercap_adjust_uiratio(di, state);

	di->psy_state = state;
	return di->mixed_cap;
}

static int sercap_chg_notifier(struct notifier_block *nb,
			       unsigned long event, void *data)
{
	struct series_capacity *di = container_of(nb,
						  struct series_capacity,
						  chg_nb);

	hwlog_info("notify chg_state:%ld\n", event);
	di->chg_state = event;
	return NOTIFY_OK;
}

static int sercap_bal_notifier(struct notifier_block *nb,
			       unsigned long event, void *data)
{
	struct series_capacity *di = container_of(nb,
						  struct series_capacity,
						  bal_nb);

	hwlog_info("notify bal_event:%ld\n", event);
	di->bal_event = event;
	return NOTIFY_OK;
}

static int sercap_nv_read(struct series_capacity *di, int *ratio)
{
	/* if cmdline_ratio < 30% or > 300% is invalid */
	if (cmdline_ratio < 300 || cmdline_ratio > 3000)
		return -EIO;

	*ratio = (int)cmdline_ratio;
	return 0;
}

static void sercap_nv_write(struct series_capacity *di)
{
	struct hisi_nve_info_user nvinfo;

	memset(&nvinfo, 0, sizeof(nvinfo));
	nvinfo.nv_operation = NV_WRITE;
	nvinfo.nv_number = SERCAP_NV_NUM;
	strncpy(nvinfo.nv_name, "BATCAP", sizeof(nvinfo.nv_name));
	nvinfo.valid_size = sizeof(di->fcc_ratio);

	if (nvinfo.valid_size > sizeof(nvinfo.nv_data))
		return;

	memcpy(&nvinfo.nv_data, &di->fcc_ratio, sizeof(di->fcc_ratio));
	if (hisi_nve_direct_access(&nvinfo))
		hwlog_err("save nv fail\n");
}

static void sercap_data_init(struct series_capacity *di)
{
	int ratio = SERCAP_DEF_RATIO;

	if (sercap_nv_read(di, &ratio))
		di->fcc_ratio = SERCAP_DEF_RATIO;
	else
		di->fcc_ratio = ratio;
}

static void sercap_init_ui_cap(struct series_capacity *di)
{
	int id = SERCAP_BAT_ID0;
	int start = di->bat0_cap;
	int ratio = SERCAP_DEF_RATIO;
	int last_soc = hisi_coul_get_last_powerdown_soc() * SERCAP_TENTH;

	/* if last powerdown soc is valid, use it to init ui cap */
	if (last_soc >= 0) {
		sercap_calc_first_empty(di, &id, &start, &ratio);
		di->mixed_cap = last_soc;
		di->ui_mixed_start = di->mixed_cap;
		di->ui_base_bat = id;
		di->ui_bat_start = start;
		di->ui_ratio = ratio;
		hwlog_info("use last_soc:%d init uicap\n", last_soc);
		return;
	}

	/* if battery capacity > 50%, use large capacity to init ui cap */
	if (di->bat0_cap > 500 && di->bat1_cap > 500) {
		if (di->bat0_cap > di->bat1_cap)
			id = SERCAP_BAT_ID0;
		else
			id = SERCAP_BAT_ID1;
	} else {
		if (di->bat0_cap < di->bat1_cap)
			id = SERCAP_BAT_ID0;
		else
			id = SERCAP_BAT_ID1;
	}
	if (id == SERCAP_BAT_ID0) {
		di->mixed_cap = di->bat0_cap;
		di->ui_base_bat = SERCAP_BAT_ID0;
		di->ui_bat_start = di->bat0_cap;
		hwlog_info("use bat0 init uicap\n");
	} else {
		di->mixed_cap = di->bat1_cap;
		di->ui_base_bat = SERCAP_BAT_ID1;
		di->ui_bat_start = di->bat1_cap;
		hwlog_info("use bat1 init uicap\n");
	}
	di->ui_mixed_start = di->mixed_cap;
	di->ui_ratio = SERCAP_DEF_RATIO;
}

static void sercap_info_init(struct series_capacity *di)
{
	int bat0_vol;
	int bat1_vol;
	int bat0_soc = hisi_coul_battery_ufcapacity_tenth();
	int delta_vol, ocv;

	if (bat0_soc > SERCAP_FULL_CAP)
		bat0_soc = SERCAP_FULL_CAP;
	if (sercap_get_cmdline_vol(&bat0_vol, &bat1_vol)) {
		bat0_vol = hw_battery_voltage(BAT_ID_0);
		bat1_vol = sercap_vbat1_voltage();
	}

	if (bat0_vol <= 0 || bat1_vol <= 0 || !is_hisi_battery_exist()) {
		di->bat1_soc = bat0_soc;
	} else {
		delta_vol = bat1_vol - bat0_vol;
		ocv = hisi_coul_get_ocv_by_soc(SERCAP_DEF_TEMP, bat0_soc);
		ocv += delta_vol; /* batter1 ocv */
		di->bat1_soc = hisi_coul_get_soc_by_ocv(SERCAP_DEF_TEMP, ocv);
	}
	di->bat0_soc = bat0_soc;
	di->bat0_start = di->bat0_soc;
	di->bat1_start = di->bat1_soc;

	di->bat0_cap = sercap_get_bat0_cap(di);
	di->pre_bat0_cap = di->bat0_cap;
	di->bat1_cap = sercap_get_bat1_cap(di);

	di->bat0_ocv = hisi_coul_get_ocv() / SERCAP_UV2MV;
	di->interval = SERCAP_INTERVAL_NOR;
	di->track.state = SERCAP_TRACK_IDLE;
	di->psy_state = POWER_SUPPLY_STATUS_UNKNOWN;

	sercap_init_ui_cap(di);
}

static void sercap_track_start(struct series_capacity *di,
			       enum sercap_track_type type)
{
	struct timespec64 ts = { 0 };

	get_monotonic_boottime64(&ts);
	di->track.time = ts.tv_sec;
	di->track.type = type;
	di->track.state = SERCAP_TRACK_RUNNING;
}

static bool sercap_track_check_chg_start(struct series_capacity *di)
{
	int ocv0, ocv1;
	int soc0, soc1;
	int v0, v1, cur;

	/* must start by discharge */
	if (di->psy_state != POWER_SUPPLY_STATUS_DISCHARGING)
		return false;

	if (di->bat0_temp < SERCAP_TRACK_TEMP ||
	    di->bat1_temp < SERCAP_TRACK_TEMP)
		return false;
	if (abs(di->cur) > SERCAP_CURRENT_LIMIT ||
	    abs(di->avg_i) > SERCAP_CURRENT_LIMIT)
		return false;

	ocv0 = sercap_vol2ocv(di->bat0_vol, di->cur);
	ocv1 = sercap_vol2ocv(di->bat1_vol, di->cur);

	if (ocv0 >= SERCAP_TRACK_VOL_LOW || ocv1 >= SERCAP_TRACK_VOL_LOW)
		return false;

	/* wait a moment, check voltage again */
	msleep(SERCAP_DEBOUNCE);
	v0 = hw_battery_voltage(BAT_ID_0);
	v1 = sercap_vbat1_voltage();
	cur = -hisi_battery_current();

	/* current is jumping */
	if (abs(v0 - di->bat0_vol) > SERCAP_VOL_DIFF_LIMIT ||
	    abs(v1 - di->bat1_vol) > SERCAP_VOL_DIFF_LIMIT ||
	    abs(di->cur) > SERCAP_CURRENT_LIMIT)
		return false;

	/* average voltage */
	v0 = (v0 + di->bat0_vol) / 2;
	v1 = (v1 + di->bat1_vol) / 2;
	cur = (cur + di->cur) / 2;

	ocv0 = sercap_vol2ocv(v0, cur);
	ocv1 = sercap_vol2ocv(v1, cur);

	if (ocv0 >= SERCAP_TRACK_VOL_LOW || ocv1 >= SERCAP_TRACK_VOL_LOW)
		return false;

	soc0 = hisi_coul_get_soc_by_ocv(di->bat0_temp, ocv0);
	soc1 = hisi_coul_get_soc_by_ocv(di->bat1_temp, ocv1);
	di->track.soc0_s = soc0;
	di->track.soc1_s = soc1;
	return true;
}

static void sercap_track_idle(struct series_capacity *di)
{
	if (sercap_bat1_chg_is_full(di) &&
	    sercap_bat0_chg_is_full(di) &&
	    di->bat1_soc >= SERCAP_FULL_CAP &&
	    di->bat0_soc >= SERCAP_FULL_CAP) {
		di->track.soc0_s = di->bat0_soc;
		di->track.soc1_s = di->bat1_soc;
		sercap_track_start(di, TRACK_TYPE_DIS);
		hwlog_info("discharge track start\n");
		return;
	}

	if (sercap_track_check_chg_start(di)) {
		sercap_track_start(di, TRACK_TYPE_CHG);
		hwlog_info("charge track start\n");
		return;
	}
}

static void sercap_track_dis_check(struct series_capacity *di)
{
	int delta0, delta1, ratio;
	int ocv0, ocv1;
	int soc0, soc1;
	int v0, v1, cur;

	if (di->bat0_temp < SERCAP_TRACK_TEMP ||
	    di->bat1_temp < SERCAP_TRACK_TEMP)
		return;
	if (abs(di->cur) > SERCAP_TRACK_DONE_CURRENT ||
	    abs(di->avg_i) > SERCAP_TRACK_DONE_CURRENT)
		return;

	ocv0 = sercap_vol2ocv(di->bat0_vol, di->cur);
	ocv1 = sercap_vol2ocv(di->bat1_vol, di->cur);

	if (ocv0 >= SERCAP_TRACK_VOL_LOW && ocv1 >= SERCAP_TRACK_VOL_LOW)
		return;

	/* wait a moment, check voltage again */
	msleep(SERCAP_DEBOUNCE);
	v0 = hw_battery_voltage(BAT_ID_0);
	v1 = sercap_vbat1_voltage();
	cur = -hisi_battery_current();

	/* current is jumping */
	if (abs(v0 - di->bat0_vol) > SERCAP_VOL_DIFF_LIMIT ||
	    abs(v1 - di->bat1_vol) > SERCAP_VOL_DIFF_LIMIT ||
	    abs(di->cur) > SERCAP_TRACK_DONE_CURRENT)
		return;

	/* average voltage and current */
	v0 = (v0 + di->bat0_vol) / 2;
	v1 = (v1 + di->bat1_vol) / 2;
	cur = (cur + di->cur) / 2;

	ocv0 = sercap_vol2ocv(v0, cur);
	ocv1 = sercap_vol2ocv(v1, cur);

	soc0 = hisi_coul_get_soc_by_ocv(di->bat0_temp, ocv0);
	soc1 = hisi_coul_get_soc_by_ocv(di->bat1_temp, ocv1);

	if (ocv0 >= SERCAP_TRACK_VOL_LOW && ocv1 >= SERCAP_TRACK_VOL_LOW) {
		return;
	} else if (ocv0 < SERCAP_TRACK_VOL_LOW && ocv1 < SERCAP_TRACK_VOL_LOW) {
		/* do nothing */
	} else if (ocv0 > ocv1) {
		int s0 = hisi_coul_get_soc_by_ocv(di->bat0_temp,
						  SERCAP_TRACK_VOL_LOW);
		if (di->bat0_soc < s0)
			soc0 = s0;
		else
			soc0 = di->bat0_soc;
	} else { /* ocv1 > ocv0 */
		soc1 = (soc1 - soc0) + di->bat0_soc;
		soc0 = di->bat0_soc;
	}
	hwlog_info("discharge track done\n");
	hwlog_info("v0:%d,v1%d,vsoc0:%d,vsoc1:%d,soc0:%d,soc1:%d\n",
		   di->bat0_vol, di->bat1_vol, soc0, soc1,
		   di->bat0_soc, di->bat1_soc);

	di->track.state = SERCAP_TRACK_IDLE;
	delta0 = di->track.soc0_s - soc0;
	delta1 = di->track.soc1_s - soc1;
	ratio = (delta0 * SERCAP_DEF_RATIO) / delta1;
	hwlog_info("discharge track done, ratio old:%d,new:%d\n",
		   di->fcc_ratio, ratio);
	if (abs(di->fcc_ratio - ratio) > SERCAP_RATIO_VALID) {
		hwlog_info("ratio invalid:%d\n", SERCAP_RATIO_VALID);
		return;
	}
	sercap_nv_write(di);
}

static void sercap_track_chg_check(struct series_capacity *di)
{
	int v0, v1;
	int delta0, delta1, ratio;
	int cc, fcc;
	int bal_soc = 0;

	if (!sercap_bat1_chg_is_full(di))
		return;
	/* check battery0 is also full */
	if (abs(di->bat0_vol - di->bat1_vol) > SERCAP_VOL_DIFF_LIMIT)
		return;

	/* wait a moment, check voltage again */
	msleep(SERCAP_DEBOUNCE);
	v0 = hw_battery_voltage(BAT_ID_0);
	v1 = sercap_vbat1_voltage();

	if (abs(v0 - v1) > SERCAP_VOL_DIFF_LIMIT)
		return;

	hwlog_info("charge track done\n");
	delta0 = SERCAP_FULL_CAP - di->track.soc0_s;
	delta1 = SERCAP_FULL_CAP - di->track.soc1_s;

	cc = sercap_bal_query_info();
	fcc = hisi_battery_fcc();
	if (fcc != 0)
		bal_soc = cc * SERCAP_FULL_CAP / fcc;

	if (delta1 == 0) {
		hwlog_info("charge track err delta1 == 0\n");
		return;
	}

	ratio = (delta0 * SERCAP_DEF_RATIO) / delta1 +
	    bal_soc * SERCAP_DEF_RATIO / delta1;

	hwlog_info("charge track done, ratio old:%d,new:%d,bal_cc:%d\n",
		   di->fcc_ratio, ratio, bal_soc);
	if (abs(di->fcc_ratio - ratio) > SERCAP_RATIO_VALID) {
		hwlog_info("ratio invalid:%d\n", SERCAP_RATIO_VALID);
		return;
	}
	sercap_nv_write(di);
}

static void sercap_track_running(struct series_capacity *di)
{
	struct timespec64 ts;
	static int pre_psy_state = POWER_SUPPLY_STATUS_DISCHARGING;

	get_monotonic_boottime64(&ts);
	if ((ts.tv_sec - di->track.time) > SERCAP_TRACK_TIMEOUT) {
		hwlog_info("track timeout\n");
		di->track.state = SERCAP_TRACK_IDLE;
		return;
	}
	if (di->track.type == TRACK_TYPE_DIS) {
		if (di->psy_state != POWER_SUPPLY_STATUS_DISCHARGING &&
		    di->bal_event != BAL_STOP) {
			hwlog_info("charging and blance cancel track\n");
			di->track.state = SERCAP_TRACK_IDLE;
		} else {
			sercap_track_dis_check(di);
		}
	} else if (di->track.type == TRACK_TYPE_CHG) {
		if (pre_psy_state != POWER_SUPPLY_STATUS_DISCHARGING &&
		    di->psy_state == POWER_SUPPLY_STATUS_DISCHARGING) {
			hwlog_info("charger plugout, cancel track\n");
			di->track.state = SERCAP_TRACK_IDLE;
		} else {
			sercap_track_chg_check(di);
		}
		pre_psy_state = di->psy_state;
	}
}

static void sercap_ratio_tracking(struct series_capacity *di)
{
	switch (di->track.state) {
	case SERCAP_TRACK_IDLE:
		sercap_track_idle(di);
		break;
	case SERCAP_TRACK_RUNNING:
		sercap_track_running(di);
		break;
	default:
		break;
	}
}

static void sercap_print_info(struct series_capacity *di)
{
	hwlog_info("s0:%d,s1:%d,ocv0:%d,avgi:%d,i:%d,v0:%d,v1:%d\n",
		di->bat0_start, di->bat1_start, di->bat0_ocv, di->avg_i,
		di->cur, di->bat0_vol, di->bat1_vol);
	hwlog_info("t0:%d,t1:%d,ps:%d,chgs:%d,bal:%d\n",
		di->bat0_temp, di->bat1_temp, di->psy_state, di->chg_state,
		di->bal_event);
	hwlog_info("ui_id:%d,ui_s:%d,mixed:%d,ui_ratio:%d,inval:%d,tr_st:%d\n",
		di->ui_base_bat, di->ui_bat_start, di->ui_mixed_start,
		di->ui_ratio, di->interval, di->track.state);
	if (di->track.state != SERCAP_TRACK_IDLE)
		hwlog_info("track:t_time:%ld,st:%d,ty:%d,soc0:%d,soc1:%d\n",
			(long)di->track.time, di->track.state, di->track.type,
			di->track.soc0_s, di->track.soc1_s);
}

int sercap_log_head(char *log_head, int size)
{
	if (!series_cap)
		return -ENOMEM;

	if (!log_head) {
		hwlog_err("chargelog_head is null\n");
		return -EINVAL;
	}

	snprintf(log_head, size,
		"ratio   uicap   cap0   cap1   soc0   soc1   uiratio   ");
	return 0;
}

int sercap_log(char *log, int size)
{

	struct series_capacity *di = series_cap;

	if (!di)
		return -ENOMEM;

	if (!log) {
		hwlog_err("chargelog is null\n");
		return -EINVAL;
	}

	snprintf(log, size,
		"%-5d   %-5d   %-4d   %-4d   %-4d   %-4d   %-7d   ",
		di->fcc_ratio, di->mixed_cap, di->bat0_cap, di->bat1_cap,
		di->bat0_soc, di->bat1_soc, di->ui_ratio);
	return 0;
}

static void sercap_monitor_work(struct work_struct *work)
{
	struct series_capacity *di = container_of(work, struct series_capacity,
						  monitor.work);
	wake_lock_timeout(&di->wakelock, HZ);
	sercap_calc_cap(di);
	sercap_ratio_tracking(di);
	if (wake_lock_active(&di->wakelock))
		wake_unlock(&di->wakelock);
	sercap_print_info(di);
	queue_delayed_work(system_power_efficient_wq,
			   &di->monitor, msecs_to_jiffies(di->interval));
}

static enum power_supply_property huawei_battery_props[] = {
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_CAPACITY,
};

static int huawei_battery_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct series_capacity *di = dev_get_drvdata(psy->dev.parent);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_HEALTH:
		if (strstr(saved_command_line, "androidboot.swtype=factory") ||
			!is_hisi_battery_exist()) {
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
			break;
		}

		if (hisi_battery_health() == POWER_SUPPLY_HEALTH_UNDERVOLTAGE)
			val->intval = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
		else if (di->mixed_cap == 0 && di->bat1_vol < SERCAP_SHUTDOWN)
			val->intval = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
		else
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		/* rounding, 0.1 as 1 */
		val->intval = (di->mixed_cap + SERCAP_TENTH - 1) /
				SERCAP_TENTH;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct power_supply_desc huawei_battery_desc = {
	.name = HUAWEI_BATTERY,
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = huawei_battery_props,
	.num_properties = ARRAY_SIZE(huawei_battery_props),
	.get_property = huawei_battery_get_property,
};

static ssize_t bat0_cap_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->bat0_cap);
}
static DEVICE_ATTR_RO(bat0_cap);

static ssize_t bat0_soc_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->bat0_soc);
}
static DEVICE_ATTR_RO(bat0_soc);

static ssize_t bat1_cap_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->bat1_cap);
}
static DEVICE_ATTR_RO(bat1_cap);

static ssize_t bat1_soc_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->bat1_soc);
}
static DEVICE_ATTR_RO(bat1_soc);

static ssize_t mixed_cap_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->mixed_cap);
}
static DEVICE_ATTR_RO(mixed_cap);

static ssize_t fcc_ratio_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->fcc_ratio);
}
static DEVICE_ATTR_RO(fcc_ratio);

static ssize_t ui_ratio_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct series_capacity *bat = dev_get_drvdata(dev);

	if (!bat)
		return -ENODEV;
	return sprintf(buf, "%d\n", bat->ui_ratio);
}
static DEVICE_ATTR_RO(ui_ratio);

static struct attribute *series_cap_attrs[] = {
	&dev_attr_bat0_cap.attr,
	&dev_attr_bat0_soc.attr,
	&dev_attr_bat1_cap.attr,
	&dev_attr_bat1_soc.attr,
	&dev_attr_mixed_cap.attr,
	&dev_attr_fcc_ratio.attr,
	&dev_attr_ui_ratio.attr,
	NULL
};

static const struct attribute_group series_cap_group = {
	.attrs = series_cap_attrs,
};

static int sercap_probe(struct platform_device *pdev)
{
	struct series_capacity *di = NULL;
	int ret;

	if (!pdev)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	di->dev = &pdev->dev;
	series_cap = di;
	platform_set_drvdata(pdev, di);

	di->batt = devm_power_supply_register(&pdev->dev,
				&huawei_battery_desc, NULL);
	if (IS_ERR(di->batt))
		return PTR_ERR(di->batt);

	ret = sysfs_create_group(&pdev->dev.kobj, &series_cap_group);
	if (ret) {
		hwlog_err("sercap sysfile fail\n");
		return ret;
	}

	wake_lock_init(&di->wakelock, WAKE_LOCK_SUSPEND, "series_batt_cap");
	sercap_data_init(di);
	sercap_info_init(di);
	INIT_DELAYED_WORK(&di->monitor, sercap_monitor_work);
	di->chg_nb.notifier_call = sercap_chg_notifier;
	hisi_register_notifier(&di->chg_nb, 0);
	di->bal_nb.notifier_call = sercap_bal_notifier;
	batt_bal_state_notifier_register(&di->bal_nb);
	queue_delayed_work(system_power_efficient_wq, &di->monitor, 0);
	return 0;
}

static int sercap_remove(struct platform_device *pdev)
{
	struct series_capacity *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	sysfs_remove_group(&pdev->dev.kobj, &series_cap_group);
	hisi_unregister_notifier(&di->chg_nb, 0);
	batt_bal_state_notifier_unregister(&di->bal_nb);
	wake_lock_destroy(&di->wakelock);
	return 0;
}

static int sercap_resume(struct platform_device *pdev)
{
	struct series_capacity *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	queue_delayed_work(system_power_efficient_wq, &di->monitor, 0);
	return 0;
}

static int sercap_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct series_capacity *di = platform_get_drvdata(pdev);

	if (!di)
		return 0;

	cancel_delayed_work(&di->monitor);
	return 0;
}

static const struct of_device_id series_cap_match_table[] = {
	{
		.compatible = "huawei,series-batt-cap",
		.data = NULL,
	},
	{},
};

static struct platform_driver series_cap_driver = {
	.probe = sercap_probe,
	.remove = sercap_remove,
	.resume = sercap_resume,
	.suspend = sercap_suspend,
	.driver = {
		.name = "huawei,series_batt_cap",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(series_cap_match_table),
	},
};

static int __init series_batt_cap_init(void)
{
	return platform_driver_register(&series_cap_driver);
}

static void __exit series_batt_cap_exit(void)
{
	platform_driver_unregister(&series_cap_driver);
}

fs_initcall_sync(series_batt_cap_init);
module_exit(series_batt_cap_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei series battery capacity driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
