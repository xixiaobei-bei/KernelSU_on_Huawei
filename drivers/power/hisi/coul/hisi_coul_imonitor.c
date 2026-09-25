/*
 * hisi_coul_imonitor.c
 *
 * define the imonitor report interface for coul
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include "hisi_coul_imonitor.h"

/* imonitor table list */
static struct coul_imonitor_event_id imonitor_event_id[] = {
	{
		COUL_IMONITOR_BASP_BASIC_INFO,
		{.tv_sec = 0, .tv_nsec = 0},
		COUL_IMONITOR_REPORT_INTERVAL_WEEKLY
	},
};

static int coul_imonitor_basp_prepare_basic_info(
	struct imonitor_eventobj *obj, void *data)
{
	int ret = 0;
	struct coul_imonitor_basp_basic_info *basp_data = NULL;

	if (!obj || !data) {
		coul_imonitor_err("%s invalid argument\n", __func__);
		return -1;
	}

	basp_data = (struct coul_imonitor_basp_basic_info *)data;

	ret += power_bigdata_send_integer(obj, "ActDays",
		basp_data->act_days);
	ret += power_bigdata_send_integer(obj, "BatCap",
		basp_data->bat_cap);
	ret += power_bigdata_send_integer(obj, "BatTVSet",
		basp_data->bat_tv_set);
	ret += power_bigdata_send_integer(obj, "BatCyc",
		basp_data->bat_cyc);
	ret += power_bigdata_send_integer(obj, "BaspLv",
		basp_data->basp_lv);
	ret += power_bigdata_send_integer(obj, "Basp1Days",
		basp_data->basp_1_days);
	ret += power_bigdata_send_integer(obj, "Basp2Days",
		basp_data->basp_2_days);
	ret += power_bigdata_send_integer(obj, "Basp3Days",
		basp_data->basp_3_days);
	ret += power_bigdata_send_integer(obj, "Basp4Days",
		basp_data->basp_4_days);
	ret += power_bigdata_send_integer(obj, "Qmax",
		basp_data->q_max);
	ret += power_bigdata_send_integer(obj, "FGFcc1",
		basp_data->fg_fcc1);
	ret += power_bigdata_send_integer(obj, "FGFcc2",
		basp_data->fg_fcc2);
	ret += power_bigdata_send_integer(obj, "FGFcc3",
		basp_data->fg_fcc3);
	ret += power_bigdata_send_integer(obj, "ChaICTp",
		basp_data->cha_ic_tp);

	ret += power_bigdata_send_string(obj, "BatMan",
		basp_data->bat_man);
	ret += power_bigdata_send_string(obj, "BatMod",
		basp_data->bat_mode);
	ret += power_bigdata_send_string(obj, "BatParF",
		basp_data->bat_par_f);
	ret += power_bigdata_send_string(obj, "HTime1",
		basp_data->fcc_time1);
	ret += power_bigdata_send_string(obj, "HTime2",
		basp_data->fcc_time2);
	ret += power_bigdata_send_string(obj, "HTime3",
		basp_data->fcc_time3);

	coul_imonitor_info("%s success\n", __func__);
	return ret;
}

/*
 * Function:        coul_imonitor_report
 * Description:     report imonitor data
 * Input:           coul_imonitor_type_t type
 * Output:          NULL
 * Return:          NULL
 */
void coul_imonitor_report(enum coul_imonitor_type type, void *data)
{
	power_bigdata_report(POWER_BIGDATA_TYPE_COUL,
		coul_imonitor_basp_prepare_basic_info, data);
}

/*
 * Function:        coul_imonitor_weekly_report
 * Description:     report imonitor data once a week
 * Input:           coul_imonitor_type_t type
 * Output:          NULL
 * Return:          NULL
 */
void coul_imonitor_periodic_report(enum coul_imonitor_type type, void *data)
{
	struct timespec64 ts64_imoitor_now;
	struct timespec64 ts64_imoitor_sum;
	struct timespec64 ts64_imoitor_interval;

	if (!data)
		return;

	ts64_imoitor_interval.tv_sec = imonitor_event_id[type].interval;
	ts64_imoitor_interval.tv_nsec = 0;
	ts64_imoitor_now = current_kernel_time64();
	ts64_imoitor_sum = timespec64_add_safe(
				   imonitor_event_id[type].ts64_imoitor_last,
				   ts64_imoitor_interval);

	if (ts64_imoitor_sum.tv_sec == TIME_T_MAX) {
		coul_imonitor_err(
			"%s time overflow happend when add 24*7 hours\n",
			__func__);
	} else if (timespec64_compare(&ts64_imoitor_sum,
				      &ts64_imoitor_now) < 0) {
		coul_imonitor_report(type, data);
		imonitor_event_id[type].ts64_imoitor_last = ts64_imoitor_now;
	}
}
