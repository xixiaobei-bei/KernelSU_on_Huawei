/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: driver for iscd
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

#include "battery_iscd.h"
#include "../coul/coul_interface.h"
#include "battery_iscd_interface.h"

#define BASP_FATAL_VDEC_TH 100

static char g_dsm_buff[ISCD_DSM_LOG_SIZE_MAX] = {0};
static struct wakeup_source g_iscd_wakelock;

static void iscd_wake_lock(void)
{
	if (!g_iscd_wakelock.active) {
		__pm_stay_awake(&g_iscd_wakelock);
		iscd_core_info("iscd core wake lock\n");
	}
}

static void iscd_wake_unlock(void)
{
	if (g_iscd_wakelock.active) {
		__pm_relax(&g_iscd_wakelock);
		iscd_core_info("iscd core wake unlock\n");
	}
}

#ifdef CONFIG_HUAWEI_CHARGER
static void __fatal_isc_chg_prot(struct iscd_info *iscd, const int *capacity)
{
	int ret;

	if (*capacity >= (int)iscd->fatal_isc_soc_limit[UPLIMIT]) {
		ret = set_charger_disable_flags(CHARGER_SET_DISABLE_FLAGS,
			CHARGER_FATAL_ISC_TYPE);
		if (unlikely(ret))
			iscd_core_err("Set isc disable flags for charger failed\n");
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_WL_SC,
			POWER_IF_SYSFS_ENABLE_CHARGER, 0);
	} else if (*capacity <= (int)iscd->fatal_isc_soc_limit[RECHARGE]) {
		ret = set_charger_disable_flags(CHARGER_CLEAR_DISABLE_FLAGS,
			CHARGER_FATAL_ISC_TYPE);
		if (unlikely(ret))
			iscd_core_err("Clear isc disable flags for charger failed\n");
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_WL_SC,
			POWER_IF_SYSFS_ENABLE_CHARGER, 1);
	}
}

static int fatal_isc_chg_limit_soc(struct notifier_block *self,
	unsigned long event, void *data)
{
	int ret;
	struct iscd_info *iscd =
		container_of(self, struct iscd_info, fatal_isc_chg_limit_soc_nb);

	if (event == ISC_LIMIT_START_CHARGING_STAGE) {
		ret = set_charger_disable_flags(CHARGER_CLEAR_DISABLE_FLAGS,
			CHARGER_FATAL_ISC_TYPE);
		if (unlikely(ret))
			iscd_core_err("Clear charger ISC disable flag failed\n");
		power_if_kernel_sysfs_set(POWER_IF_OP_TYPE_WL_SC,
			POWER_IF_SYSFS_ENABLE_CHARGER, 0);
		__fatal_isc_chg_prot(iscd, (int *)data);
	} else if (event == ISC_LIMIT_UNDER_MONITOR_STAGE) {
		__fatal_isc_chg_prot(iscd, (int *)data);
	}
	return NOTIFY_OK;
}
#else
static int fatal_isc_chg_limit_soc(struct notifier_block *self,
	unsigned long event, void *data)
{
	return NOTIFY_OK;
}
#endif

#ifdef CONFIG_DIRECT_CHARGER
static void __fatal_isc_direct_chg_prot(struct iscd_info *iscd,
	const int *capacity)
{
	if (*capacity >= (int)iscd->fatal_isc_soc_limit[UPLIMIT])
		direct_charge_set_disable_flags(DC_SET_DISABLE_FLAGS,
			DC_DISABLE_FATAL_ISC_TYPE);
	else if (*capacity <= (int)iscd->fatal_isc_soc_limit[RECHARGE])
		direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
			DC_DISABLE_FATAL_ISC_TYPE);
}

static int fatal_isc_direct_chg_limit_soc(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct iscd_info *iscd = container_of(self, struct iscd_info,
		fatal_isc_direct_chg_limit_soc_nb);

	if (event == ISC_LIMIT_START_CHARGING_STAGE) {
		direct_charge_set_disable_flags(DC_CLEAR_DISABLE_FLAGS,
			DC_DISABLE_FATAL_ISC_TYPE);
		__fatal_isc_direct_chg_prot(iscd, (int *)data);
	} else if (event == ISC_LIMIT_UNDER_MONITOR_STAGE) {
		__fatal_isc_direct_chg_prot(iscd, (int *)data);
	}
	return NOTIFY_OK;
}
#else
static int fatal_isc_direct_chg_limit_soc(struct notifier_block *self,
	unsigned long event, void *data)
{
	return NOTIFY_OK;
}
#endif

static ssize_t coul_kernel_write(struct file *file, const char *buf,
	size_t count, loff_t pos)
{
	return kernel_write(file, buf, count, &pos);
}

/*
 * kernel should not op files in the filesystem that managered by user space
 * transplant this function into user space is prefered
 */
static void isc_splash2_file_sync(struct iscd_info *iscd)
{
	struct file *fd = NULL;
	ssize_t write_size;

	if (iscd->isc_splash2_ready) {
		fd = filp_open(ISC_DATA_FILE, O_WRONLY | O_TRUNC, 0);
		if (IS_ERR(fd)) {
			iscd_core_err("splash2 file system not ready for isc data record\n");
			return;
		}
		iscd->fatal_isc_hist.magic_num = iscd->iscd_file_magic_num;
		write_size = coul_kernel_write(fd,
			(const void *)&iscd->fatal_isc_hist,
			sizeof(iscd->fatal_isc_hist), 0);
		if (write_size != sizeof(iscd->fatal_isc_hist))
			iscd_core_err("Write %s failed(wtire:%zd expect:%zu) in %s\n",
				ISC_DATA_FILE, write_size,
				sizeof(iscd->fatal_isc_hist), __func__);
		filp_close(fd, NULL);
		iscd_core_info("sync fatal isc to splash2 success\n");
	} else {
		iscd_core_err("splash2 file system not ready for isc data record\n");
	}
}

static void isc_config_splash2_file_sync(struct iscd_info *iscd)
{
	struct file *fd = NULL;
	ssize_t write_size;

	fd = filp_open(ISC_CONFIG_DATA_FILE, O_WRONLY | O_TRUNC, 0);
	if (IS_ERR(fd)) {
		iscd_core_err("splash2 file isc_config. data not ready for isc config data record\n");
		return;
	}
	iscd->fatal_isc_config.write_flag = iscd->write_flag;
	iscd->fatal_isc_config.delay_cycles = iscd->isc_valid_delay_cycles;
	iscd->fatal_isc_config.has_reported = iscd->has_reported;
	iscd->fatal_isc_config.magic_num = iscd->iscd_file_magic_num;
	write_size = coul_kernel_write(fd, (const void *)&iscd->fatal_isc_config,
		sizeof(iscd->fatal_isc_config), 0);
	if (write_size != sizeof(iscd->fatal_isc_config))
		iscd_core_err("Write %s failed(wtire:%zd expect:%zu) in %s\n",
			ISC_CONFIG_DATA_FILE, write_size,
			sizeof(iscd->fatal_isc_config), __func__);

	filp_close(fd, NULL);
	iscd_core_info("sync fatal isc config to splash2 success\n");
}

void __fatal_isc_uevent(struct work_struct *work)
{
	struct iscd_info *iscd =
		container_of(work, struct iscd_info, fatal_isc_uevent_work);
	char *envp_ext[] = {"BATTERY_EVENT=FATAL_ISC", NULL};

	iscd->isc_prompt = 1;
	kobject_uevent_env(&iscd->dev->kobj, KOBJ_CHANGE, envp_ext);
}

static int fatal_isc_uevent_notify(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct iscd_info *iscd =
		container_of(self, struct iscd_info, fatal_isc_uevent_notify_nb);

	if (iscd->fatal_isc_config.has_reported != TRUE) {
		iscd->has_reported = TRUE;
		isc_config_splash2_file_sync(iscd);
	}

	if ((event != ISC_LIMIT_START_CHARGING_STAGE) &&
		(event != ISC_LIMIT_BOOT_STAGE))
		return NOTIFY_DONE;
	else if (iscd->isc_status)
		schedule_work(&iscd->fatal_isc_uevent_work);
	return NOTIFY_OK;
}

static void fatal_isc_dmd_wkfunc(struct work_struct *work)
{
	struct iscd_info *iscd = NULL;
	struct fatal_isc_dmd *reporter = NULL;

	reporter = container_of(work, struct fatal_isc_dmd, work.work);
	iscd = container_of(reporter, struct iscd_info, dmd_reporter);

	if (!dsm_client_ocuppy(power_dsm_get_dclient(POWER_DSM_BATTERY))) {
		dsm_client_record(power_dsm_get_dclient(POWER_DSM_BATTERY),
			"%s", reporter->buff);
		dsm_client_notify(power_dsm_get_dclient(POWER_DSM_BATTERY),
			reporter->err_no);
		iscd->fatal_isc_hist.dmd_report = 0;
		isc_splash2_file_sync(iscd);
		iscd_core_info("fatal isc dmd report:%d\n", reporter->err_no);
		kfree(reporter->buff);
		reporter->buff = NULL;
	} else if (reporter->retry < FATAL_ISC_DMD_RETRY) {
		reporter->retry++;
		schedule_delayed_work(&reporter->work,
			msecs_to_jiffies(FATAL_ISC_DMD_RETRY_INTERVAL));
	} else {
		kfree(reporter->buff);
		reporter->buff = NULL;
	}
}

static int fatal_isc_dsm_report(struct notifier_block *self,
	unsigned long event, void *data)
{
	int i, j;
	struct iscd_info *iscd =
		container_of(self, struct iscd_info, fatal_isc_dsm_report_nb);

	if (iscd->isc_status && iscd->fatal_isc_hist.dmd_report &&
		(event == ISC_LIMIT_BOOT_STAGE)) {
		iscd->dmd_reporter.retry = 0;
		/* check isc status is one of valid types */
		if (iscd->isc_status <= iscd->fatal_isc_trigger.valid_num) {
			iscd->dmd_reporter.err_no =
				iscd->fatal_isc_trigger.dmd_no[iscd->isc_status - 1];
		} else {
			iscd_core_err("DMD index %u was out of range %u\n",
				iscd->isc_status,
				iscd->fatal_isc_trigger.valid_num);
			return NOTIFY_OK;
		}
		j = (iscd->fatal_isc_hist.valid_num > MAX_FATAL_ISC_NUM) ?
		    MAX_FATAL_ISC_NUM : iscd->fatal_isc_hist.valid_num;
		if (iscd->dmd_reporter.buff == NULL) {
			iscd->dmd_reporter.buff = kzalloc((j + 1) *
				FATAL_ISC_DMD_LINE_SIZE + 1, GFP_KERNEL);
			if (iscd->dmd_reporter.buff == NULL)
				return NOTIFY_OK;
		} else {
			iscd_core_err("Fatal isc dmd buff not point to null before\n");
			return NOTIFY_OK;
		}
		snprintf(iscd->dmd_reporter.buff, FATAL_ISC_DMD_LINE_SIZE + 1,
			 "%7s%6s%6s%6s%6s%5s%5s\n",
			 "ISC(uA)", "FCC", "RM", "QMAX", "CYCLE", "YEAR", "YDAY");
		for (i = 0; i < j; i++)
			snprintf(iscd->dmd_reporter.buff + (i + 1) *
				FATAL_ISC_DMD_LINE_SIZE,
				FATAL_ISC_DMD_LINE_SIZE + 1, "%7d%6d%6d%6d%6d%5d%5d\n",
				iscd->fatal_isc_hist.isc[i],
				iscd->fatal_isc_hist.fcc[i] / PERMILLAGE,
				iscd->fatal_isc_hist.rm[i] / PERMILLAGE,
				iscd->fatal_isc_hist.qmax[i] / PERMILLAGE,
				iscd->fatal_isc_hist.charge_cycles[i] / PERCENT,
				iscd->fatal_isc_hist.year[i],
				iscd->fatal_isc_hist.yday[i]);
		schedule_delayed_work(&iscd->dmd_reporter.work,
			msecs_to_jiffies(0));
	}
	return NOTIFY_OK;
}

static int fatal_isc_ocv_update(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct iscd_info *iscd =
		container_of(self, struct iscd_info, fatal_isc_ocv_update_nb);
	int avg_current;
	static unsigned int ocv_update_cnt;

	if (check_coul_dev() == false) {
		iscd_core_err("global coul device pointer is null\n");
		return NOTIFY_OK;
	}

	if (event == ISC_LIMIT_START_CHARGING_STAGE) {
		ocv_update_cnt = 0;
	} else if (event == ISC_LIMIT_UNDER_MONITOR_STAGE) {
		if (iscd->charging_state == ISCD_CHARGE_NOT_CHARGE) {
			ocv_update_cnt++;
			ocv_update_cnt =
				(ocv_update_cnt > iscd->ocv_update_interval) ?
				iscd->ocv_update_interval : ocv_update_cnt;
		} else {
			ocv_update_cnt = 0;
		}
		if (ocv_update_cnt == iscd->ocv_update_interval) {
			avg_current = coul_drv_battery_current_avg();
			avg_current = (avg_current < 0) ?
				-avg_current : avg_current;
			if (avg_current > FATAL_ISC_OCV_UPDATE_THRESHOLD) {
				iscd_core_info("Current is too big %d mA to update ocv found in %s",
					avg_current, __func__);
				return NOTIFY_OK;
			}
			coul_drv_force_ocv();
			iscd_core_info("ocv update in %s, ocv_update_cnt = %u, average current = %d\n",
				__func__, ocv_update_cnt, avg_current);
			ocv_update_cnt = 0;
		}
	}
	return NOTIFY_OK;
}

static void isc_limit_monitor_work(struct work_struct *work)
{
	struct iscd_info *iscd =
		container_of(work, struct iscd_info, isc_limit_work.work);

	fatal_isc_protection(iscd, ISC_LIMIT_UNDER_MONITOR_STAGE);
}

static int isc_listen_to_charge_event(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct iscd_info *iscd = container_of(self, struct iscd_info,
		isc_listen_to_charge_event_nb);

	switch (event) {
	case POWER_NE_CHG_START_CHARGING:
		if (!iscd->need_monitor) {
			iscd->need_monitor = 1;
			if (iscd->isc_status)
				fatal_isc_protection(iscd,
					ISC_LIMIT_START_CHARGING_STAGE);
		}
		break;
	case POWER_NE_CHG_STOP_CHARGING:
		iscd->need_monitor = 0;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int iscd_check_ocv_variance(const int *ocv, int avg_ocv, int n)
{
	s64 var = 0;
	int detal_ocv;
	s64 detal_ocv_square;
	int i;

	if (n == 0) {
		iscd_core_err("ISCD %s input para is null\n", __func__);
		return FALSE;
	}

	for (i = 0; (i < n) && (ocv[i] > 0); i++) {
		detal_ocv = (int)(avg_ocv - ocv[i]);
		detal_ocv_square = (s64)((s64)detal_ocv * (s64)detal_ocv);
		var = (s64)(var + detal_ocv_square);
	}
	var /= n;
	iscd_core_info("ISCD ocv variance is %lld uV * uV\n", var);
	if (var >= ISCD_OCV_UV_VAR_THREHOLD)
		return FALSE;

	return TRUE;
}

static int iscd_sub_sample_batt_ocv(int *fifo_volt_uv, int fifo_depth, int *ocv_uv)
{
	int j, total_vol, total_cur, used, current_ua, voltage_uv;

	total_cur = 0;
	total_vol = 0;
	used = 0;
	for (j = 0; j < fifo_depth; j++) {
		fifo_volt_uv[j] = 0;
		current_ua = coul_drv_battery_fifo_curr(j);
		voltage_uv = coul_drv_battery_fifo_vol(j);
		if ((current_ua >= ISCD_OCV_CURRENT_LIMIT) ||
			(current_ua < ISCD_CURRENT_OFFSET)) {
			iscd_core_err("ISCD current invalid, value is %d uA\n",
				current_ua);
			continue;
		}
		if ((voltage_uv >= ISCD_OCV_UV_MAX) ||
			(voltage_uv <= ISCD_OCV_UV_MIN)) {
			iscd_core_err("ISCD invalid voltage = %d uV\n",
				voltage_uv);
			continue;
		}
		iscd_core_info("ISCD valid current = %duA, voltage = %duV\n",
			current_ua, voltage_uv);
		fifo_volt_uv[j] = voltage_uv;
		total_cur += current_ua;
		total_vol += voltage_uv;
		used++;
	}

	iscd_core_info("ISCD used = %d, total_vol = %d\n", used, total_vol);
	if (used >= ISCD_OCV_FIFO_VALID_CNT) {
		*ocv_uv = total_vol / used;
		iscd_core_info("ISCD avg_voltage_uv = %d\n", *ocv_uv);
		if (iscd_check_ocv_variance(fifo_volt_uv, *ocv_uv,
			fifo_depth) == TRUE) {
			current_ua = total_cur / used;
			*ocv_uv += (current_ua / MOHM_PER_OHM) *
				(coul_drv_battery_rpcb() /
				UOHM_PER_MOHM +
				DEFAULT_BATTERY_OHMIC_RESISTANCE);
			return SUCCESS;
		}
		iscd_core_err("ISCD variance sample ocv is out of range[0, %d]\n",
			ISCD_OCV_UV_VAR_THREHOLD);
	}
	return ERROR;
}

static int iscd_sample_battery_ocv_uv(int *ocv_uv)
{
	int i, ret, fifo_depth;
	int *fifo_volt_uv = NULL;

	if (check_coul_dev() == false)
		return ERROR;

	fifo_depth = coul_drv_battery_fifo_depth();
	fifo_volt_uv = kzalloc((size_t)(sizeof(int) * fifo_depth), GFP_KERNEL);
	if (fifo_volt_uv == NULL)
		return ERROR;

	for (i = 0; i < ISCD_SAMPLE_RETYR_CNT; i++) {
		ret = iscd_sub_sample_batt_ocv(fifo_volt_uv, fifo_depth, ocv_uv);
		if (ret == SUCCESS)
			break;
	}

	if (fifo_volt_uv != NULL)
		kfree(fifo_volt_uv);
	iscd_core_info("ISCD sampled ocv is %duV\n", *ocv_uv);
	return ret;
}

static int iscd_is_sample_allowed(struct iscd_info *iscd, int *tbatt)
{
	if (check_coul_dev() == false)
		return ERROR;

	if (iscd->charging_state != ISCD_CHARGE_DONE) {
		iscd->last_sample_cnt = 0;
		iscd_core_err("ISCD charge_state is %d, try to next loop\n", iscd->charging_state);
		return ERROR;
	}
	*tbatt = get_coul_dev_batt_temp();
	if ((*tbatt > iscd->tbatt_max) || (*tbatt < iscd->tbatt_min)) {
		iscd_core_err("ISCD battery temperature is %d, out of range [%d, %d]",
			*tbatt, iscd->tbatt_min, iscd->tbatt_max);
		return ERROR;
	}
	return SUCCESS;
}

static int iscd_sample_battery_info(struct iscd_info *iscd,
	struct iscd_sample_info *sample_info)
{
	int ocv_uv = 0;
	int tbatt;
	int ocv_soc_uah = 0;
	s64 cc_value, delta_cc;
	struct timespec sample_time;
	time_t delta_time;

	if (iscd_is_sample_allowed(iscd, &tbatt) == ERROR)
		return ERROR;

	sample_time = current_kernel_time();
	iscd->full_update_cc = get_iscd_full_update_cc();
	cc_value = iscd->full_update_cc + coul_drv_battery_cc_uah();
	if (iscd->size > 0) {
		delta_cc = iscd->sample_info[iscd->size - 1].cc_value - cc_value;
		delta_time = sample_time.tv_sec -
			iscd->sample_info[iscd->size - 1].sample_time.tv_sec;
		if ((delta_time < (time_t)ISCD_CALC_INTERVAL_900S) &&
			(delta_cc >= ISCD_RECHARGE_CC)) {
			iscd->last_sample_cnt = 0;
			iscd_core_err("ISCD delta_time(%ld) < %d, delta_cc(%lld) >= %d, try to next loop\n",
				delta_time, ISCD_CALC_INTERVAL_900S, delta_cc, ISCD_RECHARGE_CC);
			return ERROR;
		}
	}
	if (iscd_sample_battery_ocv_uv(&ocv_uv) != SUCCESS)
		return ERROR;

	if (coul_drv_cal_uah_by_ocv(ocv_uv, &ocv_soc_uah) != SUCCESS)
		return ERROR;

	iscd->last_sample_cnt++;
	sample_info->sample_cnt = iscd->last_sample_cnt;
	sample_info->sample_time = sample_time;
	sample_info->tbatt = tbatt;
	sample_info->ocv_volt_uv = ocv_uv;
	sample_info->cc_value = cc_value;
	sample_info->ocv_soc_uah = ocv_soc_uah;
	iscd->last_sample_time = sample_info->sample_time;
	iscd_core_info("ISCD sampled info: sample_cnt = %d, time_s = %ld, tbatt = %d, ocv_uV = %d, cc_uAh = %lld, ocv_soc_uAh = %lld\n",
		sample_info->sample_cnt, sample_info->sample_time.tv_sec,
		sample_info->tbatt / TENTH, sample_info->ocv_volt_uv,
		sample_info->cc_value, sample_info->ocv_soc_uah);

	return SUCCESS;
}

static void iscd_remove_sampled_info(struct iscd_info *iscd, int from, int to)
{
	int i, j;

	if ((from < 0) || (to < 0)) {
		iscd_core_err("ISCD %s input para error\n", __func__);
		return;
	}
	if (from > to)
		swap(from, to);

	iscd_core_info("iscd remove sampled info from index %d to index %d\n",
		from, to);
	for (i = to + 1, j = 0; (i < iscd->size) && (i >= 0) && (from + j >= 0);
		i++, j++) {
		iscd->sample_info[(int)(from + j)].sample_time =
			iscd->sample_info[i].sample_time;
		iscd->sample_info[(int)(from + j)].sample_cnt =
			iscd->sample_info[i].sample_cnt;
		iscd->sample_info[(int)(from + j)].tbatt =
			iscd->sample_info[i].tbatt;
		iscd->sample_info[(int)(from + j)].ocv_volt_uv =
			iscd->sample_info[i].ocv_volt_uv;
		iscd->sample_info[(int)(from + j)].cc_value =
			iscd->sample_info[i].cc_value;
		iscd->sample_info[(int)(from + j)].ocv_soc_uah =
			iscd->sample_info[i].ocv_soc_uah;
	}
	iscd->size -= ((to - from) + 1);
}

static void iscd_reset_isc_buffer(struct iscd_info *iscd)
{
	int i;

	iscd->isc_buff[0] = 0;
	for (i = 1; i < ISCD_ISC_MAX_SIZE; i++)
		iscd->isc_buff[i] = INVALID_ISC;
}

static void iscd_clear_sampled_info(struct iscd_info *iscd)
{
	iscd_core_info("ISCD clear sampled info, size = %d\n", iscd->size);
	if (!iscd->size) {
		iscd_core_err("ISCD sampled info is already empty\n");
		return;
	}
	iscd_remove_sampled_info(iscd, 0, iscd->size - 1);
}

static void iscd_append_sampled_info(struct iscd_info *iscd,
	const struct iscd_sample_info *sample_info)
{
	int i = iscd->size;

	if ((i < ISCD_SMAPLE_LEN_MAX) && (i >= 0)) {
		iscd->sample_info[i].sample_cnt = sample_info->sample_cnt;
		iscd->sample_info[i].sample_time = sample_info->sample_time;
		iscd->sample_info[i].tbatt = sample_info->tbatt;
		iscd->sample_info[i].ocv_volt_uv = sample_info->ocv_volt_uv;
		iscd->sample_info[i].cc_value = sample_info->cc_value;
		iscd->sample_info[i].ocv_soc_uah = sample_info->ocv_soc_uah;
		iscd->size += 1;
	}
}

static void iscd_insert_sampled_info(struct iscd_info *iscd,
	const struct iscd_sample_info *sample_info)
{
	if (iscd->size >= ISCD_SMAPLE_LEN_MAX) {
		iscd_core_info("ISCD sample size is %d, remove one from list\n",
			iscd->size);
		iscd_remove_sampled_info(iscd, 0, 0);
	}
	if (iscd->size &&
		(sample_info->sample_cnt > ISCD_INVALID_SAMPLE_CNT_FROM) &&
		(sample_info->sample_cnt <= ISCD_INVALID_SAMPLE_CNT_TO + 1)) {
		iscd_core_info("ISCD sample size is %d, remove one from list\n",
			iscd->size);
		iscd_remove_sampled_info(iscd, iscd->size - 1, iscd->size - 1);
	}
	iscd_append_sampled_info(iscd, sample_info);
}

static int iscd_check_ocv_abrupt_change(struct iscd_info *iscd)
{
	int size;
	time_t delta_time1, delta_time2;
	int delta_ocv1, delta_ocv2;
	s64 delta_cc1, delta_cc2;

	size = iscd->size;
	/* linear compare needs at least 3 sample */
	if (size >= LEAST_SAMPLE_NUM) {
		delta_time1 =
			iscd->sample_info[size - LAST_TWO].sample_time.tv_sec -
			iscd->sample_info[size - LAST_THREE].sample_time.tv_sec;
		delta_time2 =
			iscd->sample_info[size - LAST_ONE].sample_time.tv_sec -
			iscd->sample_info[size - LAST_TWO].sample_time.tv_sec;
		delta_cc1 = iscd->sample_info[size - LAST_TWO].cc_value -
			iscd->sample_info[size - LAST_THREE].cc_value;
		delta_cc2 = iscd->sample_info[size - LAST_ONE].cc_value -
			iscd->sample_info[size - LAST_TWO].cc_value;
		delta_ocv1 = iscd->sample_info[size - LAST_TWO].ocv_volt_uv -
			iscd->sample_info[size - LAST_THREE].ocv_volt_uv;
		delta_ocv2 = iscd->sample_info[size - LAST_ONE].ocv_volt_uv -
			iscd->sample_info[size - LAST_TWO].ocv_volt_uv;
		if ((abs(delta_ocv2) >= ISCD_OCV_DELTA_MAX / HALF) &&
			(abs(delta_ocv1) < ISCD_OCV_DELTA_MAX / HALF) &&
			(abs(delta_time2) < ISCD_CALC_INTERVAL_900S) &&
			(abs(delta_time1) < ISCD_CALC_INTERVAL_900S) &&
			(abs(delta_cc2) < ISCD_RECHARGE_CC) &&
			(abs(delta_cc1) < ISCD_RECHARGE_CC)) {
			iscd_core_err("ISCD %s the last OCV invalid: ocv %d->%d->%d uV\n",
				__func__,
				iscd->sample_info[size - LAST_THREE].ocv_volt_uv,
				iscd->sample_info[size - LAST_TWO].ocv_volt_uv,
				iscd->sample_info[size - LAST_ONE].ocv_volt_uv);
			return ISCD_INVALID;
		}
	}

	return ISCD_VALID;
}

static int iscd_check_cc_drift(struct iscd_info *iscd)
{
	int ret = FALSE;
	static int abn_cc_cnt;
	static int dmd_reported = FALSE;
	time_t delta_time;

	if (iscd->size < LEAST_SAMPLE_NUM_FOR_CHECK_CC)
		return FALSE;

	delta_time = iscd->sample_info[iscd->size - LAST_ONE].sample_time.tv_sec -
		iscd->sample_info[iscd->size - LAST_TWO].sample_time.tv_sec;
	/* recharging happening in 30 min after charging done is abnormal */
	if ((iscd->sample_info[iscd->size - LAST_TWO].sample_cnt == 1) &&
		(iscd->sample_info[iscd->size - LAST_ONE].sample_cnt == 1) &&
		(delta_time < (TWICE * ISCD_CALC_INTERVAL_900S))) {
		abn_cc_cnt++;
		ret = TRUE;
	} else {
		abn_cc_cnt = 0;
	}
	if (!dmd_reported && (abn_cc_cnt >= ISCD_DMD_REPORT_TIME_FOR_ABN_RECHARGE)) {
		iscd_core_err("Abnormal charging detected for more than 3 times, do dmd report\n");
		if (snprintf_s(g_dsm_buff, ISCD_DSM_LOG_SIZE_MAX,
			ISCD_DSM_LOG_SIZE_MAX - 1, "Abnormal cc drift detected for %d times\n",
			abn_cc_cnt) < 0)
			iscd_core_err("dmd content buffer overflow, report abandoned\n");
		else
			power_dsm_report_dmd(POWER_DSM_BATTERY,
				iscd->level_config[iscd->total_level - LAST_ONE].dsm_err_no,
				g_dsm_buff);
		memset_s(g_dsm_buff, sizeof(g_dsm_buff), 0x00, sizeof(g_dsm_buff));
		dmd_reported = TRUE;
	}

	return ret;
}

static int iscd_remove_invalid_samples(struct iscd_info *iscd)
{
	int ret = FALSE;
	time_t delta_time;
	int i, size;

	if (iscd->size <= 0) {
		iscd_core_err("ISCD %s input para is null\n", __func__);
		return FALSE;
	}

	size = iscd->size;
	/* clear all samples when the lasted OCV is below designed ocv_min,typically 4V */
	if (iscd->sample_info[size - 1].ocv_volt_uv <= iscd->ocv_min) {
		iscd_core_err("ISCD ocv %duV below %duV, clear sapmpled info\n",
			iscd->sample_info[size - 1].ocv_volt_uv, iscd->ocv_min);
		iscd_clear_sampled_info(iscd);
		iscd_reset_isc_buffer(iscd);
		iscd->rm_bcd = 0;
		iscd->fcc_bcd = 0;
		return TRUE;
	}

	/* clear all samples with recharging mis-detection happening */
	if (iscd_check_cc_drift(iscd)) {
		iscd_core_err("Abnormal recharge detected\n");
		iscd_clear_sampled_info(iscd);
		iscd_reset_isc_buffer(iscd);
		iscd->rm_bcd = 0;
		iscd->fcc_bcd = 0;
		return TRUE;
	}

	/* remove the samples whose delta time with the lasted sample is more than ISCD_SAMPLE_INTERVAL_MAX */
	for (i = size - 1; i >= 0; i--) {
		delta_time = iscd->sample_info[size - 1].sample_time.tv_sec -
			iscd->sample_info[i].sample_time.tv_sec;
		if (delta_time >= ISCD_SAMPLE_INTERVAL_MAX) {
			iscd_core_err("ISCD sample_time = %lds, sample[%d]_time = %lds, delta_time %ld >= %ds\n",
				iscd->sample_info[size - 1].sample_time.tv_sec,
				i, iscd->sample_info[i].sample_time.tv_sec,
				delta_time, ISCD_SAMPLE_INTERVAL_MAX);
			iscd_remove_sampled_info(iscd, 0, i);
			iscd_reset_isc_buffer(iscd);
			ret = TRUE;
			break;
		}
	}

	if (iscd_check_ocv_abrupt_change(iscd) == ISCD_INVALID) {
		iscd_core_err("ISCD the latest OCV is invalid, remove it from list\n");
		iscd_remove_sampled_info(iscd, size - 1, size - 1);
		iscd_reset_isc_buffer(iscd);
		ret = TRUE;
	}

	return ret;
}

static int get_delta_tbatt_abs(struct iscd_info *iscd, int index0, int index1)
{
	int delta_tbatt = 0;
	int delta_tbatt_abs;

	if (index1 >= 0 && index0 >= 0) {
		delta_tbatt = iscd->sample_info[index1].tbatt -
				iscd->sample_info[index0].tbatt;
		iscd_core_info("ISCD calc isc by sample s%d s%d, delta_tbatt(s2-s1) = %d / 10\n",
			       index0, index1, delta_tbatt);
	}
	delta_tbatt_abs = (delta_tbatt >= 0) ? delta_tbatt : -delta_tbatt;
	return delta_tbatt_abs;
}

static int get_delta_ocv_abs(struct iscd_info *iscd, int index0, int index1)
{
	int delta_ocv = 0;
	int delta_ocv_abs;

	if (index1 >= 0 && index0 >= 0) {
		delta_ocv = iscd->sample_info[index0].ocv_volt_uv -
			iscd->sample_info[index1].ocv_volt_uv;
		iscd_core_info("ISCD calc isc by sample s%d s%d, delta_ocv(s1-s2) = %duV\n",
			index0, index1, delta_ocv);
	}
	delta_ocv_abs = (delta_ocv >= 0) ? delta_ocv : -delta_ocv;
	return delta_ocv_abs;
}

static int iscd_calc_isc_by_two_samples(struct iscd_info *iscd,
	int index0, int index1)
{
	int delta_tbatt_abs, delta_ocv_abs, delta_time_int;
	s64 delta_cc, delta_cc_abs, delta_ocv_soc_uah;
	time_t delta_time, delta_time_abs;
	int isc = 0;

	if ((index0 < 0) || (index1 < 0)) {
		iscd_core_err("ISCD %s input para is null\n", __func__);
		return INVALID_ISC;
	}

	if (index0 > index1)
		swap(index0, index1);

	delta_time = iscd->sample_info[index1].sample_time.tv_sec -
		iscd->sample_info[index0].sample_time.tv_sec;
	delta_ocv_soc_uah = iscd->sample_info[index0].ocv_soc_uah -
		iscd->sample_info[index1].ocv_soc_uah;
	delta_cc = iscd->sample_info[index1].cc_value -
		iscd->sample_info[index0].cc_value;
	iscd_core_info("ISCD calc isc by sample s%d s%d, delta_time(s2-s1) = %lds, "
		"delta_ocv_soc_uAh(s1-s2) = %llduAh, delta_cc(s2-s1) = %llduAh\n",
		index0, index1, delta_time, delta_ocv_soc_uah, delta_cc);

	delta_time_abs = (delta_time >= 0) ? delta_time : -delta_time;
	delta_tbatt_abs = get_delta_tbatt_abs(iscd, index0, index1);
	delta_ocv_abs = get_delta_ocv_abs(iscd, index0, index1);
	delta_cc_abs = (delta_cc >= 0) ? delta_cc : -delta_cc;
	if ((delta_tbatt_abs <= iscd->tbatt_diff_max) &&
		((delta_time_abs >= (time_t)iscd->calc_time_interval_min) ||
		((delta_ocv_abs >= ISCD_OCV_DELTA_MAX) &&
		((int)delta_cc_abs <= ISCD_CC_DELTA_MAX) &&
		(delta_time_abs >= (time_t)ISCD_CALC_INTERVAL_900S)))) {
		delta_time_int = (int)delta_time;
		if (delta_time_int > 0) {
			isc = ((int)(delta_ocv_soc_uah - delta_cc)) *
				SEC_PER_HOUR / delta_time_int;
			iscd_core_info("ISCD isc calc by sample %d %d is %d\n",
				index0, index1, isc);
		}
		if (isc < ISCD_CURRENT_OFFSET) {
			iscd_core_err("ISCD isc calc by sample s%d s%d is invalid %d, discard it\n",
				index0, index1, isc);
			isc = INVALID_ISC;
		}
		return isc;
	}
	return INVALID_ISC;
}

static void iscd_push_isc_to_isc_buf(struct iscd_info *iscd, int isc_tmp)
{
	static int index = 1;

	iscd->isc_buff[index++] = isc_tmp;
	iscd->isc_buff[0] += 1;
	if (iscd->isc_buff[0] >= ISCD_ISC_MAX_SIZE)
		iscd->isc_buff[0] = ISCD_ISC_MAX_SIZE - 1;
	if (index >= ISCD_ISC_MAX_SIZE)
		index = 1;
}

static int avg_isc_by_threhold(struct iscd_info *iscd, int lower,
	int upper, int percent)
{
	s64 sum = 0;
	int cnt = 0;
	int isc_size;
	int avg = INVALID_ISC;
	int i;

	isc_size = iscd->isc_buff[0];
	for (i = 1; i < ISCD_ISC_MAX_SIZE;  i++) {
		if ((iscd->isc_buff[i] > lower) && (iscd->isc_buff[i] < upper)) {
			sum += iscd->isc_buff[i];
			cnt++;
		}
	}
	iscd_core_info("ISCD %s isc_size: %d, cnt: %d within threhold: [%d, %d]\n",
		__func__, isc_size, cnt, lower, upper);
	if ((cnt > 0) && (cnt > isc_size * percent / PERCENT))
		avg = (int)(sum / cnt);
	return avg;
}

static int iscd_standard_deviation_of_isc(struct iscd_info *iscd,
	int avg_isc)
{
	int i;
	s64 var = 0;
	int cnt = 0;
	int detal_isc;
	s64 detal_isc_square;

	for (i = 1; i < ISCD_ISC_MAX_SIZE; i++) {
		if (iscd->isc_buff[i] != INVALID_ISC) {
			detal_isc = (int)(avg_isc - iscd->isc_buff[i]);
			detal_isc_square = (s64)((s64)detal_isc * (s64)detal_isc);
			var += detal_isc_square;
			cnt++;
		}
	}
	if (cnt > 0) {
		var /= cnt;
		iscd_core_info("ISCD %s variance of isc: %lld, cnt: %d\n",
			__func__, var, cnt);
	}

	return (int)int_sqrt((unsigned long)var);
}

static int get_isc_current(struct iscd_info *iscd, int sample_size)
{
	int i, avg_isc, isc_buff_size, sigma_isc, avg_cnt;
	s64 sum_cnt = 0;

	isc_buff_size = iscd->isc_buff[0];
	avg_isc = avg_isc_by_threhold(iscd, -INVALID_ISC, INVALID_ISC, 0);
	iscd_core_info("ISCD %s isc_buff_size: %d, primary avg_isc: %duAh\n",
			__func__, isc_buff_size, avg_isc);
	if (avg_isc < ISCD_LARGE_ISC_THREHOLD) {
		for (i = 0; i < sample_size; i++)
			sum_cnt += iscd->sample_info[i].sample_cnt;
		avg_cnt = (int)(sum_cnt / sample_size);
		if ((isc_buff_size >= ISCD_SMALL_ISC_VALID_SIZE_1) ||
			((isc_buff_size >= ISCD_SMALL_ISC_VALID_SIZE_2) &&
			 (avg_cnt < ISCD_INVALID_SAMPLE_CNT_TO + 1))) {
			sigma_isc = iscd_standard_deviation_of_isc(iscd, avg_isc);
			avg_isc = avg_isc_by_threhold(iscd, avg_isc - sigma_isc,
				avg_isc + sigma_isc,
				ISCD_SMALL_ISC_VALID_PERCENT);
			iscd_core_info("ISCD %s standard deviation of isc: %d, final avg_isc: %duAh\n",
				__func__, sigma_isc, avg_isc);
			iscd->isc = avg_isc;
			return (avg_isc == INVALID_ISC) ? ISCD_INVALID : ISCD_VALID;
		}
	} else if ((avg_isc >= ISCD_LARGE_ISC_THREHOLD) &&
		(isc_buff_size >= ISCD_LARGE_ISC_VALID_SIZE)) {
		avg_isc = avg_isc_by_threhold(iscd, ISCD_LARGE_ISC_THREHOLD,
			INVALID_ISC, ISCD_LARGE_ISC_VALID_PERCENT);
		iscd_core_info("ISCD %s final avg_isc: %duAh\n",
			__func__, avg_isc);
		iscd->isc = avg_isc;
		return (avg_isc == INVALID_ISC) ? ISCD_INVALID : ISCD_VALID;
	}

	return ISCD_INVALID;
}

static int iscd_is_short_current_valid(struct iscd_info *iscd)
{
	int sample_size, chrg_cycle;
	time_t sample_time;

	if ((check_coul_dev() == false) || (iscd->size <= 0)) {
		iscd_core_err("ISCD %s input para is error\n", __func__);
		return ISCD_INVALID;
	}

	chrg_cycle = coul_drv_battery_cycle_count();
	if ((chrg_cycle < iscd->isc_valid_cycles) ||
		((iscd->isc_delay_cycles_enable == TRUE) &&
		(chrg_cycle <= iscd->isc_valid_delay_cycles))) {
		iscd_core_err("ISCD %s charge_cycle %d is less than %d or is less than %d, try to next loop\n",
			__func__, chrg_cycle, iscd->isc_valid_cycles,
			iscd->isc_valid_delay_cycles);
		return ISCD_INVALID;
	}

	iscd_core_info("ISCD %s chrg_cycle: %d, isc_delay_cycles: %d\n",
		__func__, chrg_cycle, iscd->isc_valid_delay_cycles);
	sample_size = iscd->size;
	sample_time = iscd->sample_info[sample_size - 1].sample_time.tv_sec -
		iscd->sample_info[0].sample_time.tv_sec;

	if (sample_time < ISCD_SMAPLE_TIME_MIN) {
		iscd_core_err("ISCD %s sample time(%lds) is less than %ds, try to next loop\n",
			__func__, sample_time, ISCD_SMAPLE_TIME_MIN);
		return ISCD_INVALID;
	}

	return get_isc_current(iscd, sample_size);
}

static void iscd_calc_isc_with_prev_samples(struct iscd_info *iscd, int index)
{
	int i, isc_tmp;

	if (index >= iscd->size) {
		iscd_core_err("ISCD %s para is error, index = %d\n",
			__func__, index);
		return;
	}
	if (index == 0) {
		iscd_core_err("ISCD %s samples is not enough, try to next loop\n",
			__func__);
		return;
	}

	for (i = 0; i < index; i++) {
		isc_tmp = iscd_calc_isc_by_two_samples(iscd, i, index);
		if (isc_tmp != INVALID_ISC)
			iscd_push_isc_to_isc_buf(iscd, isc_tmp);
	}
}

static void iscd_calc_isc_by_all_samples(struct iscd_info *iscd)
{
	int i;

	if (iscd->size <= 0) {
		iscd_core_err("ISCD %s input para is null\n", __func__);
		return;
	}

	for (i = 1; i < iscd->size; i++)
		iscd_calc_isc_with_prev_samples(iscd, i);
}

static void iscd_dump_dsm_info(struct iscd_info *iscd, char *buf)
{
	int i;
	unsigned int real_fcc_record[MAX_RECORDS_CNT] = {0};
	char tmp_buf[ISCD_ERR_NO_STR_SIZE] = {0};
	unsigned int nondc_volt_dec;

	iscd_core_info("ISCD %s ++\n", __func__);

	if ((check_coul_dev() == false) || (buf == NULL)) {
		iscd_core_err("ISCD %s input para is null\n", __func__);
		return;
	}

	nondc_volt_dec = get_nondc_vol_dec();
	snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE, "nondc_volt_dec is %u mv\n",
		nondc_volt_dec);
	strncat(buf, tmp_buf, strlen(tmp_buf));
	snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE, "battery is %s, charge_cycles is %d, "
		 "rm is %dmAh %dmAh, fcc is %dmAh %dmAh, Qmax is %dmAh\n",
		 coul_drv_battery_brand(), coul_drv_battery_cycle_count(),
		 iscd->rm_bcd / UA_PER_MA, coul_drv_battery_rm(),
		 iscd->fcc_bcd / UA_PER_MA, coul_drv_battery_fcc(),
		 update_coul_dev_qmax());
	strncat(buf, tmp_buf, strlen(tmp_buf));
	coul_drv_battery_get_recrod_fcc(MAX_RECORDS_CNT, real_fcc_record);
	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE,
			"BASP fcc[%d] is %u mAh\n", i, real_fcc_record[i]);
		strncat(buf, tmp_buf, strlen(tmp_buf));
	}
	snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE,
		"isc is %d uA\n", iscd->isc);
	strncat(buf, tmp_buf, strlen(tmp_buf));
	snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE, "sample_info:\n");
	strncat(buf, tmp_buf, strlen(tmp_buf));
	snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE,
		"id time  tbatt ocv     ocv_rm/uAh cc/uAh  cnt\n");
	strncat(buf, tmp_buf, strlen(tmp_buf));

	for (i = 0; i < iscd->size; i++) {
		snprintf(tmp_buf, (size_t)ISCD_ERR_NO_STR_SIZE, "%-2d %-5ld %-5d %-7d %-10lld %-7lld %-3d\n",
			i, iscd->sample_info[i].sample_time.tv_sec -
				iscd->sample_info[0].sample_time.tv_sec,
			iscd->sample_info[i].tbatt / TENTH,
			iscd->sample_info[i].ocv_volt_uv,
			iscd->sample_info[i].ocv_soc_uah,
			iscd->sample_info[i].cc_value,
			iscd->sample_info[i].sample_cnt);
		strncat(buf, tmp_buf, strlen(tmp_buf));
	}
	iscd_core_info("ISCD %s --\n", __func__);
}

static int iscd_dsm_report(struct iscd_info *iscd, int level)
{
	int ret = SUCCESS;
	struct timespec now = current_kernel_time();

	if ((level >= ISCD_MAX_LEVEL) || (level < 0)) {
		iscd_core_err("ISCD %s input para error\n", __func__);
		return ERROR;
	}

	if (iscd->level_config[level].dsm_report_cnt < ISCD_DSM_REPORT_CNT_MAX) {
		if (!iscd->level_config[level].dsm_report_time ||
			(now.tv_sec - iscd->level_config[level].dsm_report_time >=
				ISCD_DSM_REPORT_INTERVAL)) {
			iscd_dump_dsm_info(iscd, g_dsm_buff);
			ret = power_dsm_report_dmd(POWER_DSM_BATTERY,
				iscd->level_config[level].dsm_err_no, g_dsm_buff);
			if (ret == SUCCESS) {
				iscd->level_config[level].dsm_report_cnt++;
				iscd->level_config[level].dsm_report_time =
					now.tv_sec;
			}
			memset(g_dsm_buff, (unsigned)0, sizeof(g_dsm_buff));
		}
	}
	return ret;
}

static int __successive_time_limit_judgement(struct iscd_info *iscd,
	int time_limit, int last, int i)
{
	int delt_year, delt_day;

	if (time_limit) {
		delt_year = (int)iscd->fatal_isc_hist.year[last] -
			(int)iscd->fatal_isc_hist.year[i];
		if ((delt_year < 0) || (delt_year > 1))
			delt_day = -1;
		else
			delt_day = (delt_year ? ISC_DAYS_PER_YEAR : 0) +
				iscd->fatal_isc_hist.yday[last] -
				iscd->fatal_isc_hist.yday[i];
		if ((delt_day > (int)iscd->fatal_isc_trigger.deadline) ||
			(delt_day < 0))
			return 1;
	}
	return 0;
}

static void __successive_isc_judgement(struct iscd_info *iscd, int time_limit)
{
	int i, j, last;

	last = iscd->fatal_isc_hist.valid_num - 1;
	for (j = iscd->fatal_isc_trigger.valid_num; j > 0; j--) {
		for (i = last; i >= 0; i--) {
			if (iscd->fatal_isc_hist.isc[i] <
				iscd->fatal_isc_trigger.trigger_isc[j - 1])
				break;
			if (__successive_time_limit_judgement(iscd, time_limit,
				last, i))
				break;
		}
		if ((last - i) >= (int)(iscd->fatal_isc_trigger.trigger_num[j - 1])) {
			iscd->isc_status = j;
			iscd->fatal_isc_hist.isc_status = j;
			iscd->fatal_isc_hist.dmd_report = 1;
			break;
		}
	}
}

static inline void successive_isc_judgement_time(struct iscd_info *iscd)
{
	__successive_isc_judgement(iscd, ISC_TRIGGER_WITH_TIME_LIMIT);
}

static void fatal_isc_judgement(struct iscd_info *iscd, int type)
{
	switch (type) {
	case INVALID_ISC_JUDGEMENT:
		iscd->isc_status = 0;
		iscd->need_monitor = 0;
		break;
	case SUCCESSIVE_ISC_JUDGEMENT_TIME:
		iscd->fatal_isc_hist.trigger_type = SUCCESSIVE_ISC_JUDGEMENT_TIME;
		successive_isc_judgement_time(iscd);
		break;
	default:
		iscd_core_err("Unexpected type %d found in %s\n",
			type, __func__);
		break;
	}
	if (iscd->isc_status)
		iscd_core_info("fatal isc %u was found by using judgement type %d",
			iscd->isc_status, type);
}

int smallest_in_oneday(struct iscd_info *iscd, struct rtc_time *tm)
{
	int valid_num;

	if ((iscd == NULL) || (tm == NULL))
		return 0;

	if (iscd->isc == ISCD_FATAL_LEVEL_THREHOLD)
		return 0;

	valid_num = iscd->fatal_isc_hist.valid_num;
	if ((valid_num == 0) || (valid_num > MAX_FATAL_ISC_NUM))
		return 1;

	if (iscd->fatal_isc_hist.yday[valid_num - 1] == tm->tm_yday) {
		if (iscd->isc < iscd->fatal_isc_hist.isc[valid_num - 1]) {
			iscd->fatal_isc_hist.valid_num--;
			return 1;
		}
		return 0;
	}
	return 1;
}

static void refresh_isc_hist(struct iscd_info *iscd, const struct rtc_time *tm)
{
	int i, valid_num;

	valid_num = iscd->fatal_isc_hist.valid_num;
	if ((valid_num < MAX_FATAL_ISC_NUM) && (valid_num >= 0)) {
		iscd->fatal_isc_hist.isc[valid_num] = iscd->isc;
		iscd->fatal_isc_hist.rm[valid_num] = get_coul_dev_batt_rm();
		iscd->fatal_isc_hist.fcc[valid_num] = get_coul_dev_batt_fcc();
		iscd->fatal_isc_hist.qmax[valid_num] = get_coul_dev_qmax();
		iscd->fatal_isc_hist.charge_cycles[valid_num] =
			get_coul_dev_batt_chargecycles();
		iscd->fatal_isc_hist.year[valid_num] = tm->tm_year;
		iscd->fatal_isc_hist.yday[valid_num] = tm->tm_yday;
		iscd->fatal_isc_hist.valid_num++;
	} else if (valid_num == MAX_FATAL_ISC_NUM) {
		for (i = 0; i < MAX_FATAL_ISC_NUM - 1; i++) {
			iscd->fatal_isc_hist.isc[i] =
				iscd->fatal_isc_hist.isc[i + 1L];
			iscd->fatal_isc_hist.rm[i] =
				iscd->fatal_isc_hist.rm[i + 1L];
			iscd->fatal_isc_hist.fcc[i] =
				iscd->fatal_isc_hist.fcc[i + 1L];
			iscd->fatal_isc_hist.qmax[i] =
				iscd->fatal_isc_hist.qmax[i + 1L];
			iscd->fatal_isc_hist.charge_cycles[i] =
				iscd->fatal_isc_hist.charge_cycles[i + 1L];
			iscd->fatal_isc_hist.year[i] =
				iscd->fatal_isc_hist.year[i + 1L];
			iscd->fatal_isc_hist.yday[i] =
				iscd->fatal_isc_hist.yday[i + 1L];
		}
		iscd->fatal_isc_hist.isc[i] = iscd->isc;
		iscd->fatal_isc_hist.rm[i] = get_coul_dev_batt_rm();
		iscd->fatal_isc_hist.fcc[i] = get_coul_dev_batt_fcc();
		iscd->fatal_isc_hist.qmax[i] = get_coul_dev_qmax();
		iscd->fatal_isc_hist.charge_cycles[i] =
			get_coul_dev_batt_chargecycles();
		iscd->fatal_isc_hist.year[i] = tm->tm_year;
		iscd->fatal_isc_hist.yday[i] = tm->tm_yday;
	} else {
		iscd->fatal_isc_hist.valid_num = 1;
		iscd->fatal_isc_hist.isc[0] = iscd->isc;
		iscd->fatal_isc_hist.rm[0] = get_coul_dev_batt_rm();
		iscd->fatal_isc_hist.fcc[0] = get_coul_dev_batt_fcc();
		iscd->fatal_isc_hist.qmax[0] = get_coul_dev_qmax();
		iscd->fatal_isc_hist.charge_cycles[0] =
			get_coul_dev_batt_chargecycles();
		iscd->fatal_isc_hist.year[0] = tm->tm_year;
		iscd->fatal_isc_hist.yday[0] = tm->tm_yday;
	}
}

/*
 * parameter struct iscd_info *iscd is prefered.
 * However, datum is in struct smartstar_coul_device.
 * Interface is needed to define between coul & isc.
 */
void update_isc_hist(struct iscd_info *iscd,
	int (*valid)(struct iscd_info *iscd, struct rtc_time *tm))
{
	struct timespec ts;
	struct rtc_time tm;

	if ((iscd == NULL) || (valid == NULL)) {
		iscd_core_err("Null valid function found in %s\n", __func__);
		return;
	}

	ts = current_kernel_time();
	rtc_time64_to_tm(ts.tv_sec, &tm);
	tm.tm_year += TM_YEAR_OFFSET;

	if (valid(iscd, &tm)) {
		refresh_isc_hist(iscd, &tm);
		iscd_core_info("fatal isc %u:%d %d %d %d %d, %u is valid\n",
			iscd->fatal_isc_hist.isc_status,
			iscd->fatal_isc_hist.isc[0],
			iscd->fatal_isc_hist.isc[1],
			iscd->fatal_isc_hist.isc[2],
			iscd->fatal_isc_hist.isc[3],
			iscd->fatal_isc_hist.isc[4],
			iscd->fatal_isc_hist.valid_num);

		/* judge if fatal isc occured */
		fatal_isc_judgement(iscd, iscd->fatal_isc_trigger_type);

		/* sync isc history information to splash2 */
		isc_splash2_file_sync(iscd);
	}
}

static void iscd_process_short_current(struct iscd_info *iscd)
{
	int i;
	unsigned int nondc_volt_dec;

	/* report valid isc here */
	for (i = 0; i < iscd->total_level; i++) {
		if ((iscd->isc >= iscd->level_config[i].isc_min) &&
			(iscd->isc < iscd->level_config[i].isc_max)) {
			iscd_core_info("ISCD isc: %duA,  level: %d, threhold: [%d, %d)uA\n",
				iscd->isc, i, iscd->level_config[i].isc_min,
				iscd->level_config[i].isc_max);
			if (iscd_dsm_report(iscd, i))
				iscd_core_err("Reporting ISC level %d DMD failed in %s\n",
					i, __func__);
			break;
		}
	}

	/* update the isc history information */
	nondc_volt_dec = get_nondc_vol_dec();
	if (nondc_volt_dec <= BASP_FATAL_VDEC_TH)
		update_isc_hist(iscd, smallest_in_oneday);

	/* Going on isc detection */
	if (iscd->isc_status) {
		iscd->enable = DISABLED;
		fatal_isc_protection(iscd, ISC_LIMIT_BOOT_STAGE);
	}
}

static int iscd_calc_short_current(struct iscd_info *iscd, int rm_flag)
{
	int sample_size = iscd->size;
	int last_one = sample_size - LAST_ONE;
	int last_two = sample_size - LAST_TWO;

	/* isc must be calculated at least with 2 samples */
	if (sample_size < LEAST_SAMPLE_NUM_FOR_CALC_ISHROT) {
		iscd_core_err("ISCD %s sample size is %d\n, try to next calc\n",
			__func__, sample_size);
		return FALSE;
	}

	if (rm_flag == TRUE) {
		if (iscd->sample_info[sample_size - 1].sample_cnt <=
			ISCD_INVALID_SAMPLE_CNT_TO)
			return FALSE;
		iscd_calc_isc_by_all_samples(iscd);
		iscd_core_info("some invalid samples has been removed, calc short current with all samples\n");
		return TRUE;
	}

	if ((iscd->sample_info[last_one].sample_cnt > ISCD_INVALID_SAMPLE_CNT_TO) ||
		(iscd->sample_info[last_one].sample_cnt ==
			ISCD_STANDBY_SAMPLE_CNT)) {
		/* calc the last sample with its prev samples */
		iscd_calc_isc_with_prev_samples(iscd, last_one);
		return TRUE;
	} else if ((iscd->sample_info[last_one].sample_cnt >
			ISCD_INVALID_SAMPLE_CNT_FROM) &&
		(iscd->sample_info[last_one].sample_cnt <=
			ISCD_INVALID_SAMPLE_CNT_TO)) {
		iscd_core_info("ISCD %s cnt is %d, do nothing\n", __func__,
			iscd->sample_info[last_one].sample_cnt);
		return FALSE;
	} else if (iscd->sample_info[last_one].sample_cnt ==
			ISCD_INVALID_SAMPLE_CNT_FROM) {
		if ((iscd->sample_info[last_two].sample_cnt >
				ISCD_INVALID_SAMPLE_CNT_TO) ||
			(iscd->sample_info[last_two].sample_cnt ==
				ISCD_STANDBY_SAMPLE_CNT)) {
			iscd_core_info("ISCD %s this cnt is %d, but prev cnt is %d, do nothing\n",
				__func__,
				iscd->sample_info[last_one].sample_cnt,
				iscd->sample_info[last_two].sample_cnt);
			return FALSE;
		}
		/* calc the penultimate sample with its prev samples */
		iscd_calc_isc_with_prev_samples(iscd, last_two);
		return TRUE;
	}
	return FALSE;
}

static void iscd_timer_start(struct iscd_info *iscd, time_t delta_secs)
{
	ktime_t kt = ktime_set((s64)delta_secs, (unsigned long)0);

	hrtimer_start(&iscd->timer, kt, HRTIMER_MODE_REL);
}

static void check_batt_critical_electric_leakage(struct iscd_info *iscd)
{
	int batt_fcc, design_fcc, batt_ruc;

	if (check_coul_dev() == false)
		return;
	batt_fcc = coul_drv_battery_fcc();
	design_fcc = coul_drv_battery_fcc_design();
	batt_ruc = coul_drv_battery_rm();
	if (iscd->enable == ENABLED) {
		if ((batt_fcc >= design_fcc / HALF) &&
			(batt_ruc * PERCENT >= batt_fcc * FCC_MAX_PERCENT ||
			batt_fcc * PERCENT >= design_fcc * FCC_MAX_PERCENT)) {
			iscd->isc = ISCD_FATAL_LEVEL_THREHOLD;
			iscd_core_err("ISCD rm = %d, fcc = %d, set internal short current to %dmA\n",
				batt_ruc * PERMILLAGE, batt_fcc * PERMILLAGE,
				iscd->isc / UA_PER_MA);
			iscd_process_short_current(iscd);
		}
	}
}

static void iscd_leak_check_work(struct work_struct *work)
{
	struct iscd_info *iscd =
		container_of(work, struct iscd_info, leak_work.work);

	check_batt_critical_electric_leakage(iscd);
	queue_delayed_work(system_power_efficient_wq, &iscd->leak_work,
		round_jiffies_relative(msecs_to_jiffies(CALCULATE_LEAK_MS)));
}

static void iscd_work(struct work_struct *work)
{
	int ret;
	struct iscd_info *iscd =
		container_of(work, struct iscd_info, delayed_work.work);
	struct iscd_sample_info *sample_info = NULL;

	sample_info = kzalloc(sizeof(*sample_info), GFP_KERNEL);
	if (sample_info == NULL)
		goto FuncEnd;

	/* enter else than or when di->charging_state == ISCD_CHARGE_STOP make it error */
	if (iscd->charging_state == ISCD_CHARGE_STOP) {
		hrtimer_cancel(&iscd->timer);
	} else {
		if (iscd_sample_battery_info(iscd, sample_info) == SUCCESS) {
			iscd_insert_sampled_info(iscd, sample_info);
			ret = iscd_remove_invalid_samples(iscd);
			if ((iscd_calc_short_current(iscd, ret) == TRUE) &&
				(iscd_is_short_current_valid(iscd) == ISCD_VALID))
				iscd_process_short_current(iscd);
			if (iscd->enable == ENABLED)
				iscd_timer_start(iscd,
					(time_t)iscd->sample_time_interval);
		} else {
			if (iscd->enable == ENABLED)
				iscd_timer_start(iscd,
					(time_t)iscd->sample_time_interval /
						QUARTER);
		}
	}
	kfree(sample_info);
FuncEnd:
	iscd_wake_unlock();
}

static enum hrtimer_restart iscd_timer_func(struct hrtimer *timer)
{
	struct iscd_info *iscd = container_of(timer, struct iscd_info, timer);
	struct timespec now = current_kernel_time();
	time_t delta_time;

	delta_time = now.tv_sec - iscd->last_sample_time.tv_sec;
	iscd_core_info("ISCD delta time is %lds\n", delta_time);
	if (delta_time >=
		iscd->sample_time_interval - ISCD_SAMPLE_INTERVAL_DELTA) {
		iscd_core_info("ISCD %s ++\n", __func__);
		iscd_wake_lock();
		/* delay for battery stability */
		schedule_delayed_work(&iscd->delayed_work,
			msecs_to_jiffies((unsigned int)(
				iscd->sample_time_delay * MSEC_PER_SEC)));
		iscd_core_info("ISCD %s --\n", __func__);
	}

	return HRTIMER_NORESTART;
}

static int coul_kernel_read(struct file *file, loff_t offset, char *addr,
	unsigned long count)
{
	return (int)kernel_read(file, (void *)addr, count, &offset);
}

/* checking if splash2 had been mounted */
static int iscd_check_splash2_mounted(void)
{
	int ret_s, whence, read_size;
	char *buff = NULL;
	struct file *fd = NULL;
	char *find_str = NULL;

	buff = kzalloc(MOUNTS_INFO_FILE_MAX_SIZE + 1, GFP_KERNEL);
	if (buff == NULL)
		return -1;
	fd = filp_open(F2FS_MOUNTS_INFO, O_RDONLY, 0);
	if (IS_ERR(fd)) {
		iscd_core_err("Open %s failed in %s\n",
			F2FS_MOUNTS_INFO, __func__);
		kfree(buff);
		return -1;
	}
	whence = 0;
	while ((read_size = coul_kernel_read(fd, fd->f_pos, buff + whence,
			MOUNTS_INFO_FILE_MAX_SIZE - whence)) > 0) {
		find_str = strstr(buff, SPLASH2_MOUNT_INFO);
		if (find_str != NULL)
			break;
		fd->f_pos += read_size;
		whence += read_size;
		if (whence > (int)strlen(SPLASH2_MOUNT_INFO)) {
			ret_s = memmove_s(buff, MOUNTS_INFO_FILE_MAX_SIZE,
				buff + whence - strlen(SPLASH2_MOUNT_INFO),
				strlen(SPLASH2_MOUNT_INFO));
			if (ret_s)
				iscd_core_err("%s memmove_s failed\n", __func__);
			(void)memset_s(buff + strlen(SPLASH2_MOUNT_INFO),
				whence - strlen(SPLASH2_MOUNT_INFO), 0,
				whence - strlen(SPLASH2_MOUNT_INFO));
			whence = strlen(SPLASH2_MOUNT_INFO);
		}
	}
	if (find_str == NULL) {
		iscd_core_err("%s not mounted yet\n", SPLASH2_MOUNT_INFO);
		filp_close(fd, NULL);
		kfree(buff);
		return -1;
	}
	filp_close(fd, NULL);
	kfree(buff);
	iscd_core_info("splash2 had been mounted\n");
	return 0;
}

/* create directory /splash2/isc for data */
static int iscd_create_dir(void)
{
	int file_des;
	mm_segment_t old_fs;

	iscd_core_info("checking directory %s\n", ISC_DATA_DIRECTORY);
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	file_des = sys_access(ISC_DATA_DIRECTORY, 0);
	set_fs(old_fs);
	/* case for different errors need differenr process here */
	if (file_des < 0) {
		iscd_core_info("Access directory %s failed %d in %s\n",
			ISC_DATA_DIRECTORY, file_des, __func__);
		set_fs(KERNEL_DS);
		file_des = sys_mkdir(ISC_DATA_DIRECTORY, 0770); /* 0770: permission code */
		set_fs(old_fs);
		if (file_des < 0) {
			iscd_core_err("Create directory %s for recording fatal isc failed %d in %s\n",
				ISC_DATA_DIRECTORY, file_des, __func__);
			return -1;
		}
	}
	return 0;
}

static void fatal_isc_hist_process(struct iscd_info *iscd)
{
	if (iscd->fatal_isc_trigger_type == iscd->fatal_isc_hist.trigger_type) {
		iscd->isc_status = iscd->fatal_isc_hist.isc_status;
	} else {
		fatal_isc_judgement(iscd, iscd->fatal_isc_trigger_type);
		isc_splash2_file_sync(iscd);
	}
	if (iscd->isc_status) {
		iscd->enable = DISABLED;
		spin_lock(&iscd->boot_complete);
		iscd->isc_splash2_ready = 1;
		iscd->uevent_wait_for_send = 1;
		if (iscd->app_ready) {
			iscd->uevent_wait_for_send = 0;
			spin_unlock(&iscd->boot_complete);
			fatal_isc_protection(iscd, ISC_LIMIT_BOOT_STAGE);
		} else {
			spin_unlock(&iscd->boot_complete);
		}
	}
}

/* init isc history information from /splash2/isc/isc.data */
static int init_from_isc_data(struct iscd_info *iscd)
{
	int flags, read_size;
	char *buff = NULL;
	struct file *fd = NULL;
	mm_segment_t old_fs;

	buff = kzalloc(MOUNTS_INFO_FILE_MAX_SIZE + 1, GFP_KERNEL);
	if (buff == NULL)
		return -1;

	flags = O_RDWR | O_CREAT |
		(coul_drv_battery_removed_before_boot() ? O_TRUNC : 0);
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fd = filp_open(ISC_DATA_FILE, flags, 0660); /* 0660: permission code */
	set_fs(old_fs);
	if (IS_ERR(fd)) {
		kfree(buff);
		return -1;
	}

	read_size = coul_kernel_read(fd, 0, buff,
		sizeof(iscd->fatal_isc_hist) + 1);
	filp_close(fd, NULL);
	if (read_size == sizeof(iscd->fatal_isc_hist)) {
		iscd_core_info("fatal isc datum file size was correct\n");
		memcpy(&iscd->fatal_isc_hist, buff, read_size);
		if (iscd->fatal_isc_hist.magic_num == iscd->iscd_file_magic_num) {
			fatal_isc_hist_process(iscd);
		} else {
			iscd_core_info("fatal isc datum file was damaged\n");
			iscd->fatal_isc_hist.isc_status = 0;
			iscd->fatal_isc_hist.valid_num = 0;
			iscd->fatal_isc_hist.magic_num =
				iscd->iscd_file_magic_num;
		}
	} else {
		iscd_core_info("fatal isc datum file size was uncorrect\n");
	}
	spin_lock(&iscd->boot_complete);
	iscd->isc_splash2_ready = 1;
	spin_unlock(&iscd->boot_complete);
	iscd_core_info("%s was fined by %s\n", ISC_DATA_FILE, __func__);
	kfree(buff);
	return 0;
}

/* init isc history information from /splash2/isc/isc_config.data */
static void init_from_isc_config_data(struct iscd_info *iscd)
{
	int flags, read_size;
	struct file *fd = NULL;
	char *buff = NULL;

	buff = kzalloc(MOUNTS_INFO_FILE_MAX_SIZE + 1, GFP_KERNEL);
	if (buff == NULL)
		return;
	flags = O_RDWR | O_CREAT |
			(coul_drv_battery_removed_before_boot() ? O_TRUNC : 0);
	fd = filp_open(ISC_CONFIG_DATA_FILE, flags, 0660); /* 0660: permission code */
	if (IS_ERR(fd)) {
		iscd_core_err("Open and create %s failed in %s\n",
			ISC_CONFIG_DATA_FILE, __func__);
		kfree(buff);
		return;
	}
	read_size = coul_kernel_read(fd, 0, buff,
		sizeof(iscd->fatal_isc_config) + 1);
	filp_close(fd, NULL);
	if (read_size == (int)sizeof(iscd->fatal_isc_config)) {
		iscd_core_info("fatal isc config datum file size was correct\n");
		(void)memcpy_s(&iscd->fatal_isc_config,
			sizeof(iscd->fatal_isc_config), buff,
			sizeof(iscd->fatal_isc_config));

		if (iscd->fatal_isc_config.magic_num == APP_CLOSE_ISC_MAGIC_NUM) {
			iscd->enable = DISABLED;
			iscd_core_info("ISCD 1.0 is closed by SmartCharge\n");
			kfree(buff);
			return;
		}

		if (iscd->fatal_isc_config.magic_num == iscd->iscd_file_magic_num) {
			iscd->has_reported = iscd->fatal_isc_config.has_reported;
			if (iscd->fatal_isc_config.write_flag == TRUE) {
				iscd->isc_valid_delay_cycles =
					iscd->fatal_isc_config.delay_cycles;
				iscd->write_flag =
					iscd->fatal_isc_config.write_flag;
			}
		}
	} else {
		iscd_core_info("fatal isc config datum file size was uncorrect\n");
	}

	if ((iscd->isc_delay_cycles_enable == TRUE) && (iscd->write_flag != TRUE)) {
		iscd->isc_valid_delay_cycles =
			coul_drv_battery_cycle_count() + iscd->isc_chrg_delay_cycles;
		iscd->write_flag = TRUE;
		isc_config_splash2_file_sync(iscd);
	}
	kfree(buff);
}

/*
 * kernel should not op files in the filesystem that managered by user space
 * transplant this function into user space is prefered
 */
static void isc_hist_info_init(struct work_struct *work)
{
	struct iscd_info *iscd = container_of(work, struct iscd_info,
		isc_splash2_work.work);

	if (check_coul_dev() == false) {
		iscd_core_info("Input is null in %s\n", __func__);
		return;
	}
	if (iscd->isc_splash2_ready) {
		iscd_core_info("ISC splash2 has been initialized, so does not need to be reinitialized\n");
		return;
	}

	if (iscd_check_splash2_mounted())
		goto isc_init_retry;

	if (iscd_create_dir())
		goto isc_init_buff_free;

	if (init_from_isc_data(iscd))
		goto isc_init_buff_free;

	init_from_isc_config_data(iscd);

isc_init_buff_free:
	return;

isc_init_retry:
	if (iscd->isc_datum_init_retry < ISC_SPLASH2_INIT_RETRY) {
		iscd->isc_datum_init_retry++;
		schedule_delayed_work(&iscd->isc_splash2_work,
			msecs_to_jiffies(WAIT_FOR_SPLASH2_INTERVAL));
	}
}

static void get_isc_limit_dts(struct device_node *np, struct iscd_info *iscd)
{
	int ret, i;
	u32 temp[__MAX_FATAL_ISC_ACTION_TYPE] = {0};

	/* isc limitations setting up */
	iscd->fatal_isc_action = 0;
	iscd->fatal_isc_action_dts = 0;
	ret = of_property_read_u32_array(np, "fatal_isc_actions",
		temp, __MAX_FATAL_ISC_ACTION_TYPE);
	if (ret) {
		iscd_core_info("fatal_isc_actions not defined or right size in device tree\n");
	} else {
		if (temp[UPDATE_OCV_ACTION]) {
			ret = of_property_read_u32(np, "fatal_isc_ocv_update_interval",
				&iscd->ocv_update_interval);
			if (ret) {
				iscd_core_info("fatal_isc_ocv_update_interval not defined in device tree\n");
				temp[UPDATE_OCV_ACTION] = 0;
			} else if (ISC_LIMIT_CHECKING_INTERVAL == 0) {
				iscd_core_info("ISC_LIMIT_CHECKING_INTERVAL is 0 found in %s\n",
					__func__);
				temp[UPDATE_OCV_ACTION] = 0;
			} else {
				iscd->ocv_update_interval /=
					(ISC_LIMIT_CHECKING_INTERVAL /
						MSEC_PER_SEC);
			}
		}
		for (i = 0; i < __MAX_FATAL_ISC_ACTION_TYPE; i++) {
			if (temp[i])
				iscd->fatal_isc_action_dts |=
					BIT((unsigned int)i);
		}
	}
}

static void get_isc_trigger_inf(struct device_node *np, struct iscd_info *iscd)
{
	int ret, ret0, ret1, ret2;
	unsigned int i, j;

	ret = 0;
	for (i = 0, j = 0; i < iscd->fatal_isc_trigger.valid_num;
		i++, j += ISC_TRIGGER_STEP) {
		ret0 = of_property_read_u32_index(np, "fatal_isc_trigger_condition",
			j + FATAL_ISC_TRIGGER_NUM_OFFSET,
			&iscd->fatal_isc_trigger.trigger_num[i]);
		ret1 = of_property_read_u32_index(np, "fatal_isc_trigger_condition",
			j + FATAL_ISC_TRIGGER_ISC_OFFSET,
			(unsigned int *)(&iscd->fatal_isc_trigger.trigger_isc[i]));
		ret2 = of_property_read_u32_index(np, "fatal_isc_trigger_condition",
			j + FATAL_ISC_TRIGGER_DMD_OFFSET,
			(unsigned int *)(&iscd->fatal_isc_trigger.dmd_no[i]));
		ret = (ret || ret0 || ret1 || ret2);
	}
	if (ret) {
		iscd_core_err("Read fatal_isc_trigger_condition failed\n");
		iscd->fatal_isc_trigger_type = INVALID_ISC_JUDGEMENT;
	}
	ret = of_property_read_u32(np, "fatal_isc_deadline",
		&iscd->fatal_isc_trigger.deadline);
	if (ret) {
		iscd_core_info("fatal_isc_deadline not defined in device tree\n");
		iscd->fatal_isc_trigger.deadline = DEFAULT_FATAL_ISC_DEADLINE;
	}
}

static void get_isc_trigger_dts(struct device_node *np, struct iscd_info *iscd)
{
	int ret;

	ret = of_property_read_u32(np, "fatal_isc_trigger_type", &iscd->fatal_isc_trigger_type);
	if (ret) {
		iscd_core_info("fatal_isc_trigger_type not defined in device tree\n");
		iscd->fatal_isc_trigger_type = INVALID_ISC_JUDGEMENT;
	}

	/* set up fatal isc trigger function */
	switch (iscd->fatal_isc_trigger_type) {
	case INVALID_ISC_JUDGEMENT:
		break;
	case SUCCESSIVE_ISC_JUDGEMENT_TIME:
		ret = of_property_count_elems_of_size(np,
			"fatal_isc_trigger_condition", sizeof(int));
		if (ret <= 0 || (ret % ELEMS_PER_CONDITION) != 0 ||
			ret > (ELEMS_PER_CONDITION * MAX_FATAL_ISC_NUM)) {
			iscd_core_err("Uncorrect fatal_isc_trigger_condition size %d\n", ret);
			iscd->fatal_isc_trigger_type = INVALID_ISC_JUDGEMENT;
		} else {
			iscd->fatal_isc_trigger.valid_num = ret / ELEMS_PER_CONDITION;
			get_isc_trigger_inf(np, iscd);
		}
		break;
	default:
		iscd->fatal_isc_trigger_type = INVALID_ISC_JUDGEMENT;
		break;
	}
}

static void get_isc_fatal_dts(struct device_node *np, struct iscd_info *iscd)
{
	int ret;

	/* read out fatal isc device tree settings */
	ret = of_property_read_u32_index(np, "fatal_isc_soc_limit", 0,
		&iscd->fatal_isc_soc_limit[RECHARGE]);
	if (ret) {
		iscd_core_info("Can't read out fatal_isc_soc_limit first u32 from device tree\n");
		iscd->fatal_isc_soc_limit[RECHARGE] =
			DEFAULT_FATAL_ISC_RECHAGE_SOC;
	}
	iscd->fatal_isc_soc_limit[RECHARGE] =
		(iscd->fatal_isc_soc_limit[RECHARGE] < FATAL_ISC_SOC_LIMIT_UPPER) ?
		iscd->fatal_isc_soc_limit[RECHARGE] : MAX_DTS_FATAL_ISC_SOC_LIMIT;

	ret = of_property_read_u32_index(np, "fatal_isc_soc_limit", 1,
		&iscd->fatal_isc_soc_limit[UPLIMIT]);
	if (ret) {
		iscd_core_info("Can't read out fatal_isc_soc_limit second u32 from device tree\n");
		iscd->fatal_isc_soc_limit[UPLIMIT] =
			DEFAULT_FATAL_ISC_UPLIMIT_SOC;
	}
	iscd->fatal_isc_soc_limit[UPLIMIT] =
		(iscd->fatal_isc_soc_limit[UPLIMIT] <=
			iscd->fatal_isc_soc_limit[RECHARGE]) ?
		(iscd->fatal_isc_soc_limit[RECHARGE] + 1) :
			iscd->fatal_isc_soc_limit[UPLIMIT];
}

static void fatal_isc_init(struct device_node *np, struct iscd_info *iscd)
{
	spin_lock_init(&iscd->boot_complete);
	iscd_create_sysfs(iscd);
	get_isc_fatal_dts(np, iscd);
	get_isc_trigger_dts(np, iscd);
	get_isc_limit_dts(np, iscd);

	/* fatal isc works */
	INIT_DELAYED_WORK(&iscd->isc_splash2_work, isc_hist_info_init);
	INIT_DELAYED_WORK(&iscd->dmd_reporter.work, fatal_isc_dmd_wkfunc);
	INIT_DELAYED_WORK(&iscd->isc_limit_work, isc_limit_monitor_work);
	INIT_WORK(&iscd->fatal_isc_uevent_work, __fatal_isc_uevent);

	iscd->fatal_isc_action = FATAL_ISC_ACTION_DMD_ONLY;
	BLOCKING_INIT_NOTIFIER_HEAD(&iscd->isc_limit_func_head);
	iscd->fatal_isc_direct_chg_limit_soc_nb.notifier_call =
		fatal_isc_direct_chg_limit_soc;
	iscd->fatal_isc_chg_limit_soc_nb.notifier_call = fatal_isc_chg_limit_soc;
	iscd->fatal_isc_uevent_notify_nb.notifier_call = fatal_isc_uevent_notify;
	iscd->isc_listen_to_charge_event_nb.notifier_call =
		isc_listen_to_charge_event;
	iscd->fatal_isc_ocv_update_nb.notifier_call = fatal_isc_ocv_update;
	iscd->fatal_isc_dsm_report_nb.notifier_call = fatal_isc_dsm_report;
	set_fatal_isc_action(iscd);

	/* isc history datum initalization */
	if (iscd->fatal_isc_trigger_type != INVALID_ISC_JUDGEMENT) {
		/* set fatal_isc_hist.magic_num which is version of isc history datum */
		iscd->fatal_isc_hist.magic_num = iscd->iscd_file_magic_num;
		/* init fatal_isc_hist by splash2 file, here wair for splash2 mounted */
		schedule_delayed_work(&iscd->isc_splash2_work,
			msecs_to_jiffies(WAIT_FOR_SPLASH2_START));
	}
	iscd_core_info("fatal isc trigger type was %u\n",
			iscd->fatal_isc_trigger_type);
}

static void charge_done_process(struct iscd_info *iscd)
{
	if ((iscd->enable == ENABLED) &&
		(iscd->charging_state == ISCD_CHARGE_START)) {
		iscd->last_sample_cnt = 0;
		if (!iscd->rm_bcd || !iscd->fcc_bcd) {
			iscd->rm_bcd = get_coul_dev_batt_ruc();
			iscd->fcc_bcd = get_coul_dev_batt_fcc();
		}
		iscd->last_sample_time = current_kernel_time();
		hrtimer_start(&iscd->timer,
			ktime_set((s64)iscd->sample_time_interval,
			(unsigned long)0), HRTIMER_MODE_REL);
	}
}

static int iscd_chg_nb_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct iscd_info *iscd = container_of(nb, struct iscd_info, charger_nb);

	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		iscd->charging_state = ISCD_CHARGE_START;
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		iscd->charging_state = ISCD_CHARGE_STOP;
		if ((iscd->enable == ENABLED) && iscd->size) {
			iscd_wake_lock();
			schedule_delayed_work(&iscd->delayed_work,
				msecs_to_jiffies(0));
		}
		break;
	case VCHRG_CHARGE_DONE_EVENT:
		charge_done_process(iscd);
		iscd->charging_state = ISCD_CHARGE_DONE;
		break;
	case VCHRG_NOT_CHARGING_EVENT:
		iscd->charging_state = ISCD_CHARGE_NOT_CHARGE;
		iscd_core_err("charging is stop by fault\n");
		break;
	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		iscd->charging_state = ISCD_CHARGE_NOT_CHARGE;
		iscd_core_err("charging is stop by overvoltage\n");
		break;
	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		iscd->charging_state = ISCD_CHARGE_NOT_CHARGE;
		iscd_core_err("charging is stop by weaksource\n");
		break;
	default:
		iscd->charging_state = ISCD_CHARGE_NOT_CHARGE;
		iscd_core_err("unknow event %d\n", (int)event);
		break;
	}
	return NOTIFY_OK;
}

static int iscd_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct iscd_info *iscd = NULL;

	iscd = devm_kzalloc(&pdev->dev, sizeof(*iscd), GFP_KERNEL);
	if (iscd == NULL)
		return -ENOMEM;
	iscd->dev = &pdev->dev;
	iscd_reset_isc_buffer(iscd);
	INIT_DELAYED_WORK(&iscd->delayed_work, iscd_work);
	INIT_DELAYED_WORK(&iscd->leak_work, iscd_leak_check_work);
	wakeup_source_init(&g_iscd_wakelock, "iscd_wakelock");
	hrtimer_init(&iscd->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	iscd->timer.function = iscd_timer_func;
	iscd->charger_nb.notifier_call = iscd_chg_nb_call;
	bci_register_notifier(&iscd->charger_nb, 1);
	np = pdev->dev.of_node;
	if (np == NULL) {
		iscd_core_err("isc dts node can't find in %s", __func__);
	} else {
		coul_core_get_iscd_info(np, iscd);
		fatal_isc_init(np, iscd);
	}
	platform_set_drvdata(pdev, iscd);
	queue_delayed_work(system_power_efficient_wq, &iscd->leak_work,
		round_jiffies_relative(msecs_to_jiffies(CALCULATE_LEAK_MS)));
	return 0;
}

static int iscd_remove(struct platform_device *pdev)
{
	struct iscd_info *iscd = platform_get_drvdata(pdev);

	if (iscd != NULL) {
		devm_kfree(&pdev->dev, iscd);
		iscd = NULL;
	}
	wakeup_source_trash(&g_iscd_wakelock);
	return 0;
}

static int iscd_resume(struct platform_device *pdev)
{
	struct iscd_info *iscd = platform_get_drvdata(pdev);

	iscd_core_info("%s ++\n", __func__);
	if ((iscd->enable == ENABLED) && (iscd->charging_state == ISCD_CHARGE_DONE))
		hrtimer_start(&iscd->timer, ktime_set((unsigned long)0,
			(unsigned long)0), HRTIMER_MODE_REL);
	queue_delayed_work(system_power_efficient_wq, &iscd->leak_work,
		round_jiffies_relative(msecs_to_jiffies(CALCULATE_LEAK_MS)));
	iscd_core_info("%s --\n", __func__);
	return 0;
}

static int iscd_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct iscd_info *iscd = platform_get_drvdata(pdev);

	iscd_core_info("%s ++\n", __func__);
	cancel_delayed_work(&iscd->leak_work);
	iscd_core_info("%s --\n", __func__);
	return 0;
}

static const struct of_device_id battery_iscd_match[] = {
	{
		.compatible = "hisi,soft-isc",
		.data = NULL
	},
	{
		/* the end */
	}
};

MODULE_DEVICE_TABLE(of, battery_iscd_match);

static struct platform_driver battery_iscd_driver = {
	.probe = iscd_probe,
	.remove = iscd_remove,
#ifdef CONFIG_PM
	.suspend = iscd_suspend,
	.resume = iscd_resume,
#endif
	.driver = {
		.name = "battery iscd",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(battery_iscd_match),
	},
};

static int __init hisi_bat_iscd_init(void)
{
	platform_driver_register(&battery_iscd_driver);
	return 0;
}

device_initcall(hisi_bat_iscd_init);

static void __exit hisi_bat_iscd_exit(void)
{
	platform_driver_unregister(&battery_iscd_driver);
}

module_exit(hisi_bat_iscd_exit);

MODULE_DESCRIPTION("Battery ISCD driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");

