/*
 * hisi_charge_time.c  --  Calculate the remaining time of charging.
 *
 * Copyright (c) 2019 Huawei Technologies CO., Ltd.
 *
 * Author: yangshunlong
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/wakelock.h>

#include <hisi_partition.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

#include "hisi_charge_time.h"
#include "securec.h"
#include <huawei_platform/power/direct_charger.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>

struct hisi_chg_time_device *g_hisi_chg_time_info;

static u64 get_current_time(void)
{
#ifdef CONFIG_HISI_TIME
	u64 now = hisi_getcurtime() / NSEC_PRE_SECOND;
#else
	u64 now = local_clock() / NSEC_PRE_SECOND;
#endif
	return now;
}

/**
 * @brief	  : hisi_chg_time_partition_ready
 * @param[in]  : NA
 * @return	 : int -1:not ready,0:ready
 */
static int hisi_chg_time_partition_ready(void)
{
	char p_name[CHG_DUR_BUF_SHOW_LEN + 1] = {0};
	int ret;

	ret = flash_find_ptn_s(PART_BATT_TP_PARA, p_name, sizeof(p_name));
	if (ret)
		ct_err("%s()-line=%d\n", __func__, __LINE__);

	p_name[CHG_DUR_BUF_SHOW_LEN] = '\0';
	if (sys_access(p_name, 0)) {
		ct_err(
			"%s()-line=%d, name:%s\n", __func__, __LINE__, p_name);
		return -1;
	}
	return 0;
}

/**
 * @brief	  : hisi_chg_time_flash_open
 * @param[in]  : flags
 * @return	 : ::int
 */
static int hisi_chg_time_flash_open(int flags)
{
	char p_name[CHG_DUR_BUF_SHOW_LEN + 1] = {0};
	int ret, fd_dfx;

	ret = hisi_chg_time_partition_ready();
	if (ret) {
		ct_err("%s()-line=%d, ret %d\n", __func__, __LINE__, ret);
		return -1;
	}

	ret = flash_find_ptn_s(PART_BATT_TP_PARA, p_name, sizeof(p_name));
	if (ret)
		ct_err("%s()-line=%d\n", __func__, __LINE__);

	p_name[CHG_DUR_BUF_SHOW_LEN] = '\0';
	fd_dfx = sys_open(p_name, flags, CHG_DUR_FILE_LIMIT);
	if (fd_dfx < 0)
		ct_err("%s()-line=%d\n", __func__, __LINE__);

	return fd_dfx;
}

/**
 * @brief	  : hisi_chg_time_read_flash_data
 * @param[in]  : buf
 * @param[in]  : buf_size
 * @param[in]  : flash_offset
 * @return	:int
 * @note	:
 */
int hisi_chg_time_read_flash_data(void *buf,
					u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash = -1;
	int cnt = 0;
	mm_segment_t old_fs;

	if (!buf || buf_size == 0) {
		ct_err("%s()-line=%d\n", __func__, __LINE__);
		return 0;
	}

	old_fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	fd_flash = hisi_chg_time_flash_open(O_RDONLY);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			ct_err("%s()-line=%d, ret=%d, flash_offset=%x\n",
				__func__, __LINE__, ret, flash_offset);
			goto close;
		}
		cnt = sys_read(fd_flash, buf, buf_size);
		if (cnt != buf_size) {
			ct_err("%s()-line=%d, cnt=%d\n", __func__,
				__LINE__, cnt);
			goto close;
		}
	} else {
		ct_err("%s()-line=%d, fd_flash=%d\n", __func__, __LINE__,
			fd_flash);
		set_fs(old_fs);
		return fd_flash;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs); /*lint !e501*/
	return cnt;
}

/**
 * @brief	  : hisi_chg_time_add_flash_data
 * @param[in]  : p_buf
 * @param[in]  : buf_size
 * @param[in]  : flash_offset
 * @return	 : void
 * @note	   :
 */
void hisi_chg_time_write_flash_data(const void *p_buf,
					u32 buf_size, u32 flash_offset)
{
	int ret, fd_flash, cnt = 0;
	mm_segment_t old_fs;

	if (!p_buf) {
		ct_err("%s()-line=%d\n", __func__, __LINE__);
		return;
	}

	old_fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	fd_flash = hisi_chg_time_flash_open(O_RDWR | O_SYNC);
	if (fd_flash >= 0) {
		ret = sys_lseek(fd_flash, flash_offset, SEEK_SET);
		if (ret < 0) {
			ct_err("%s()-line=%d, ret=%d\n", __func__,
				__LINE__, ret);
			goto close;
		}
		cnt = sys_write(fd_flash, p_buf, buf_size);
		if (cnt != buf_size) {
			ct_err("%s()-line=%d, cnt=%d\n", __func__,
				__LINE__, cnt);
			goto close;
		}
	} else {
		ct_err("%s()-line=%d\n", __func__, __LINE__);
		set_fs(old_fs); /*lint !e501*/
		return;
	}
close:
	sys_close(fd_flash);
	set_fs(old_fs); /*lint !e501*/
}

static void hisi_chg_time_sub_param_printk(
	struct hisi_chg_time_param *type_param)
{
	int i, j;
	char buff[PARAM_SIZE] = {0};
	int offset = 0;
	int ret = 0;
	int str_len = PARAM_SIZE - 1;
	int soc;

	if (!type_param) {
		ct_err("%s()-line=%d error\n", __func__, __LINE__);
		return;
	}

	ct_info("%s start %d,TH %d,TL %d,fcc %d,cycles %d\n",
		__func__, type_param->start_soc, type_param->temp_high,
		type_param->temp_low, type_param->fcc, type_param->batt_cycles);

	for (i = 0; i < ROW; i++) {
		for (j = 0; j < COL; j++) {
			soc = i * ROW + j + 1;
			ret = snprintf_s(buff + offset, str_len - strlen(buff),
				30, "[%d(%d)] ", soc,
				type_param->step_time[soc]);
			if (ret < 0) {
				ct_err(
					" %s, ret %d, i %d, j %d error! %s\n",
					__func__, ret, i, j, buff);
			} else {
				offset += ret;
			}
		}
		ct_info(" %s,  %s\n", __func__, buff);
		if (memset_s(buff, PARAM_SIZE, 0, PARAM_SIZE) != EOK)
			ct_err("%s():%d:memset_s fail!\n",
						__func__, __LINE__);

		offset = 0;
	}
}

static void hisi_chg_time_param_printk(
	struct hisi_chg_time_info *param)
{
	ct_info("standard:\n");
	hisi_chg_time_sub_param_printk(&param->standard);

	ct_info("fcp:\n");
	hisi_chg_time_sub_param_printk(&param->fcp);

	ct_info("lvc:\n");
	hisi_chg_time_sub_param_printk(&param->lvc);

	ct_info("sc:\n");
	hisi_chg_time_sub_param_printk(&param->sc);
}

static int hisi_chg_time_read_param_from_flash(
					struct hisi_chg_time_device *di)
{
	int cnt = 0;
	size_t size = sizeof(struct hisi_chg_time_info);

	cnt = hisi_chg_time_read_flash_data(
		&di->flash_param, size, CHG_DUR_OFFSET);
	if (cnt <= 0) {
		ct_err("%s()-line=%d fail\n", __func__, __LINE__);
		return -1;
	}

	ct_info("%s\n", __func__);
	hisi_chg_time_param_printk(&di->flash_param);

	return 0;
}

static void hisi_chg_time_write_param_to_flash(
					struct hisi_chg_time_device *di)
{
	size_t size = sizeof(struct hisi_chg_time_info);

	hisi_chg_time_param_printk(&di->flash_param);

	hisi_chg_time_write_flash_data(&di->flash_param, size, CHG_DUR_OFFSET);
}

#ifdef CONFIG_HISI_DEBUG_FS
int test_val;
void hisi_chg_time_output_set(int valid, int sec)
{
	if (valid)
		test_val = (int)((unsigned int)sec | VALID_FLAG);
	else
		test_val = sec;
}

void hisi_chg_time_flash_test(int type, int soc, int time)
{
	struct hisi_chg_time_device *di = g_hisi_chg_time_info;
	struct hisi_chg_time_param *param = NULL;

	if (!di || soc > FULL_SOC || soc < 0)
		return;

	if (type == 0)
		param = &di->flash_param.standard;
	else if (type == 1)
		param = &di->flash_param.fcp;
	else if (type == 2)
		param = &di->flash_param.lvc;
	else if (type == 3)
		param = &di->flash_param.sc;
	else
		return;

	param->step_time[soc] = time;
	ct_info("%s type %d, i[%d] = %d\n", __func__, type, soc, time);

	if (soc == FULL_SOC) {
		param->batt_cycles = di->batt_info.batt_cycles;
		param->temp_high = NORMAL_TEMP;
		param->temp_low = NORMAL_TEMP;
		param->start_soc = 0;
		hisi_chg_time_write_param_to_flash(di);
	}
}
void hisi_chg_time_test_flag(int flag)
{
	struct hisi_chg_time_device *di = g_hisi_chg_time_info;

	if (!di)
		return;

	di->test_flag = flag;
}

/*
 * @brief	  : hisi_chg_time_flash_param_show
 * @param[in]  : dev
 * @param[in]  : attr
 * @param[in]  : buf
 * @return	 : ::ssize_t
 * @note	   :
 */
ssize_t hisi_chg_time_flash_param_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hisi_chg_time_device *di = g_hisi_chg_time_info;

	if (!di)
		return -EINVAL;

	ct_info("%s()-line=%d\n", __func__, __LINE__);
	hisi_chg_time_read_param_from_flash(di);
	return 0;
}

void hisi_chg_time_clear_flash_data(void)
{
	void *p_buf = NULL;

	p_buf = kzalloc(CHG_DUR_SIZE, GFP_KERNEL);
	if (!p_buf)
		return;

	hisi_chg_time_write_flash_data(p_buf, CHG_DUR_SIZE, CHG_DUR_OFFSET);
	kfree(p_buf);
}

ssize_t hisi_chg_time_flash_param_clear(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int status = count;

	if (!buf)
		return -EINVAL;

	ct_info("%s()-line=%d\n", __func__, __LINE__);
	hisi_chg_time_clear_flash_data();

	return status;
}

static DEVICE_ATTR(flash_param, 0644,
	hisi_chg_time_flash_param_show, hisi_chg_time_flash_param_clear);
#endif

static int hisi_chg_time_test_soc(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	static int soc;
	struct hisi_chg_time_device *di = g_hisi_chg_time_info;

	if (!di)
		return 0;

	if (soc > 100)
		soc = 0;
	return soc++;
#else
	return 0;
#endif
}

int hisi_chg_time_remaining(bool with_valid)
{
#ifdef CONFIG_HISI_DEBUG_FS
	if (test_val)
		return test_val;
#endif

	if (!g_hisi_chg_time_info)
		return -1;

	if (with_valid)
		return g_hisi_chg_time_info->remaining_duration_with_valid;
	else
		return g_hisi_chg_time_info->remaining_duration;
}

static int is_charge_full(struct hisi_chg_time_device *di)
{
	if (di->batt_info.soc == FULL_SOC)
		return 1;
	else
		return 0;
}
static int is_direct_charge(enum charge_type_enum charge_type)
{
	if (charge_type == CHG_SC || charge_type == CHG_LVC)
		return 1;
	else
		return 0;
}
static int hisi_chg_time_param_select(
	struct hisi_chg_time_device *di, int charger_type)
{
	ct_dbg("%s charger_type: %d\n", __func__, charger_type);

	switch (charger_type) {
	case CHG_5V2A:
		di->ref_curve = &di->flash_param.standard;
		ct_dbg("%s charger_type: CHG_5V2A\n", __func__);
		break;
	case CHG_FCP:
		di->ref_curve = &di->flash_param.fcp;
		ct_dbg("%s charger_type: CHG_FCP\n", __func__);
		break;
	case CHG_LVC:
		if (di->chg_info.cc_cable_detect_ok)
			di->ref_curve = &di->flash_param.lvc;
		else
			di->ref_curve = &di->flash_param.lvc_none_standard;
		ct_dbg("%s charger_type: CHG_LVC\n", __func__);
		break;
	case CHG_SC:
		if (di->chg_info.cc_cable_detect_ok)
			di->ref_curve = &di->flash_param.sc;
		else
			di->ref_curve = &di->flash_param.sc_none_standard;
		ct_dbg("%s charger_type: CHG_SC\n", __func__);
		break;
	default:
		di->ref_curve = NULL;
		ct_dbg(
			"%s charger_type: %d error\n", __func__, charger_type);
		return -1;
	}
	return 0;
}

static void hisi_chg_time_updata_flash(struct hisi_chg_time_device *di)
{
	int i;

	ct_info("%s charge_type %s[%d], start soc %d, soc %d\n", __func__,
		charge_type_str_enum[di->chg_info.charge_type],
		di->chg_info.charge_type, di->start_capacity,
		di->batt_info.soc);

	if (di->start_capacity < 0 || di->start_capacity > FULL_SOC) {
		ct_err("%s start_capacity %d error\n", __func__,
			di->start_capacity);
		return;
	}

	ct_info("%s ref curve :\n", __func__);
	hisi_chg_time_sub_param_printk(di->ref_curve);
	ct_info("%s cur curve :\n", __func__);
	hisi_chg_time_sub_param_printk(&di->cur_curve);

	ct_info("%s cur curve temp_high %d, temp_low %d\n", __func__,
		di->cur_curve.temp_high, di->cur_curve.temp_low);
	if (di->cur_curve.temp_high < TEMP_HIGH &&
		di->cur_curve.temp_low > TEMP_LOW) {
		if (di->ref_curve) {
			di->ref_curve->batt_cycles = di->batt_info.batt_cycles;
			di->ref_curve->start_soc = di->start_capacity;
			di->ref_curve->fcc = di->batt_info.fcc;
			di->ref_curve->temp_high = di->cur_curve.temp_high;
			di->ref_curve->temp_low = di->cur_curve.temp_low;
			for (i = FULL_SOC; i > di->start_capacity; i--) {
				di->ref_curve->step_time[i] =
					di->cur_curve.step_time[i];
				di->ref_curve->volt[i] = di->cur_curve.volt[i];
			}
		}
		hisi_chg_time_write_param_to_flash(di);
	}
}

static void hisi_chg_time_self_study(struct hisi_chg_time_device *di)
{
	u64 timestemp = get_current_time();

	ct_dbg("%s  +\n", __func__);

	if (di->charge_type_change) {
		di->cur_curve.start_soc = di->start_capacity;
		di->start_timestemp = timestemp;
		di->cur_curve.fcc = di->batt_info.fcc;
		di->cur_curve.batt_cycles = di->batt_info.batt_cycles;
		di->cur_curve.temp_low = di->cur_curve.temp_high =
			di->batt_info.batt_temp;
	}

	/*
	 *start soc is larger than the reference curve soc,
	 *and battery cycle is within 5 times, then not update the curve
	 */

	if (di->ref_curve->start_soc != 0 &&
		di->start_capacity > di->ref_curve->start_soc &&
		di->batt_info.batt_cycles < (di->ref_curve->batt_cycles
								+ CYCLE_TH)) {
		ct_info("%s start:%d ref st %d,cyc %d,ref cyc %d\n",
			__func__, di->start_capacity, di->ref_curve->start_soc,
			di->batt_info.batt_cycles, di->ref_curve->batt_cycles);
		return;
	}

	/*
	 * start soc after 40%, the first 5 minutes will not be updated to
	 *  avoid excessive current deviation due to internal resistance.
	 */
	if (is_direct_charge(di->chg_info.charge_type) &&
		(di->start_timestemp + MIN_START_TH) > timestemp &&
		di->start_capacity > SOC_START_TH) {
		ct_info("%s start:%ld now %ld,start_capacity %d\n",
			__func__, di->start_timestemp, timestemp,
			di->start_capacity);
		return;
	}

	if (di->batt_info.soc == (di->pre_capacity + 1)) {

		if (di->pre_capacity == di->start_capacity &&
			di->start_capacity > 2) {
			/* Discard the first value */
		} else {
			di->cur_curve.step_time[di->batt_info.soc] =
				timestemp - di->pre_timestemp;
			di->cur_curve.volt[di->batt_info.soc] =
				di->batt_info.volt_mv;

			ct_info("%s soc:%d now %ld,pre %ld, dur %d\n",
				__func__, di->batt_info.soc, timestemp,
				di->pre_timestemp,
				di->cur_curve.step_time[di->batt_info.soc]);
			/* for debug */
			hisi_chg_time_sub_param_printk(&di->cur_curve);
			if (is_charge_full(di)) {
				ct_info("%s soc is full\n", __func__);
				hisi_chg_time_updata_flash(di);
			}
			/* update maximum and minimum temperatures */
			if (di->cur_curve.temp_high < di->batt_info.batt_temp)
				di->cur_curve.temp_high =
					di->batt_info.batt_temp;
			if (di->cur_curve.temp_low > di->batt_info.batt_temp)
				di->cur_curve.temp_low =
					di->batt_info.batt_temp;
		}

		di->pre_timestemp = timestemp;
	}

	ct_dbg("%s  -\n", __func__);
}

static void hisi_chg_time_adjust_by_current(
					struct hisi_chg_time_device *di)
{
	int i;
	int ref_current, delt_mA;
	int compensation_time, durations;

	if (!di->param_dts.adjust_by_current)
		return;

	if (is_direct_charge(di->chg_info.charge_type)) {
		i = di->batt_info.soc;
		durations = di->remaining_duration;

		ref_current = di->batt_info.fcc * SEC_PRE_HOUR /
			di->ref_curve->step_time[i] / FULL_SOC;
		delt_mA = di->batt_info.curr_ma - ref_current;
		 /* pre 500mA add 1min */
		compensation_time = delt_mA / 500 * 60;
		durations -= compensation_time;

		ct_info("%s fcc %dmAh,time[%d] %d,cur %dmA\n", __func__,
			di->batt_info.fcc, i,
			di->ref_curve->step_time[i], ref_current);
		ct_info("%s cur %d,ref %d, delt %dmA, comp %d, dur %d\n",
			__func__, di->batt_info.curr_ma, ref_current, delt_mA,
			compensation_time, durations);
	}

}

static void hisi_chg_time_adjust_by_fcc(
			struct hisi_chg_time_device *di)
{
	int delt_time = 0;
	int remaining_duration = di->remaining_duration;
	int delt_fcc = di->batt_info.fcc - di->ref_curve->fcc;

	if (!di->param_dts.adjust_by_fcc)
		return;

	if (di->batt_info.soc > ADJUST_FCC_SOC)
		return;

	delt_time = delt_fcc * SEC_PRE_HOUR / ADJUST_FCC_CURRENT;

	remaining_duration += delt_time;
	ct_dbg("%s fcc %d,soc %d,ref %d,delt %d,delt T %d,dur %d,duration %d\n",
		__func__, di->batt_info.fcc, di->batt_info.soc,
		di->ref_curve->fcc, delt_fcc, delt_time,
		di->remaining_duration, remaining_duration);

	di->remaining_duration = remaining_duration;
}

static void hisi_chg_time_adjust_by_temperature(
			struct hisi_chg_time_device *di)
{
	if (!di->param_dts.adjust_by_temp)
		return;
}

static void hisi_chg_time_adjust_by_volt(
			struct hisi_chg_time_device *di)
{
	if (!di->param_dts.adjust_by_volt)
		return;
}

static void hisi_chg_time_calc_param(struct hisi_chg_time_device *di)
{
	int i = 0;
	int durations = 0;

	ct_dbg("%s  +\n", __func__);

	if (!di->ref_curve) {
		ct_info("%s no reference curve\n", __func__);
		di->remaining_duration = INVALID;
		di->remaining_duration_with_valid = INVALID;
		goto out;
	}

	for (i = FULL_SOC; i > di->batt_info.soc; i--) {
		if (di->ref_curve->step_time[i] <= 0) {
			di->remaining_duration = INVALID;
			ct_info("%s durations %d, i %d, time %d\n",
				__func__, di->remaining_duration, i,
				di->ref_curve->step_time[i]);
			goto out;
		} else {
			ct_info("%s durations %d, step_time[%d] = %d\n",
				__func__, durations, i,
				di->ref_curve->step_time[i]);
			durations += di->ref_curve->step_time[i];
		}
	}
	di->remaining_duration = durations;

	hisi_chg_time_adjust_by_volt(di);
	hisi_chg_time_adjust_by_current(di);
	hisi_chg_time_adjust_by_fcc(di);
	hisi_chg_time_adjust_by_temperature(di);

	/* Add valid flag for framework to check*/
	di->remaining_duration_with_valid =
		(int)((unsigned int)di->remaining_duration | VALID_FLAG);

out:
	ct_info("%s durations sec:%d, code:0xlx\n", __func__,
		di->remaining_duration, di->remaining_duration_with_valid);

	ct_dbg("%s  -\n", __func__);
}

static void hisi_chg_time_get_charger_type(
					struct hisi_chg_time_device *di)
{
	enum charge_type_enum charge_type = CHG_OTHERS;
	enum huawei_usb_charger_type type = charge_get_charger_type();

	if (direct_charge_get_super_charging_flag()) {
		/*
		 *when direct charger change to buck charger, flag still be true
		 */
		ct_dbg("%s supre charger succ\n", __func__);
		if (di->chg_info.direct_charge_type == SC_MODE) {
			charge_type = CHG_SC;
			di->chg_info.direct_charge_done = false;
		} else if (di->chg_info.direct_charge_type == LVC_MODE) {
			charge_type = CHG_LVC;
			di->chg_info.direct_charge_done = false;
		} else {
		/*
		 *when direct charger done and change to buck charger,
		 *charge type still be direct charger.
		 */
			ct_dbg(
				"%s direct charger done, charge type %d\n",
				__func__, di->chg_info.charge_type);
			di->chg_info.direct_charge_done = true;
			return;
		}
	} else {
		if (type == CHARGER_TYPE_STANDARD)
			charge_type = CHG_5V2A;
		else if (type == CHARGER_TYPE_USB)
			charge_type = CHG_USB;
		else if (type == CHARGER_TYPE_FCP)
			charge_type = CHG_FCP;
		else if (type == CHARGER_REMOVED)
			charge_type = CHG_NONE;
		else
			charge_type = CHG_OTHERS;
	}

	if (charge_type != di->chg_info.charge_type) {
		ct_info("%s type %d is %s,pre charge_type %s[%d]\n",
			__func__, type, charge_type_str_enum[charge_type],
			charge_type,
			charge_type_str_enum[di->chg_info.charge_type],
			di->chg_info.charge_type);

		di->charge_type_change = 1;
	}

	di->chg_info.charge_type = charge_type;
}

static void hisi_chg_time_get_charge_info(struct hisi_chg_time_device *di)
{
	int ret = 0;
	bool pre_direct_charger_done = di->chg_info.direct_charge_done;

	hisi_chg_time_get_charger_type(di);

	if (is_direct_charge(di->chg_info.charge_type)) {
		ret = direct_charge_get_info(
			CC_CABLE_DETECT_OK, &di->chg_info.cc_cable_detect_ok);
		if (ret) {
			ct_err("%s, get cc_cable_detect fail %d\n",
				__func__, ret);
			di->chg_info.cc_cable_detect_ok = 1;
		}

		ct_info("%s, get cc_cable_detect_ok is %d\n", __func__,
			di->chg_info.cc_cable_detect_ok);

		if (pre_direct_charger_done == false &&
			di->chg_info.direct_charge_done == true) {
			di->chg_info.direct_charge_done_soc = di->batt_info.soc;
			ct_dbg("%s, direct charge done at %d\n",
				__func__, di->batt_info.soc);
		}
	}
}

static void hisi_chg_time_get_batt_info(struct hisi_chg_time_device *di)
{
	di->batt_info.fcc = hisi_battery_get_limit_fcc();
	if (di->test_flag)
		di->batt_info.soc = hisi_chg_time_test_soc();
	else
		di->batt_info.soc = hisi_bci_show_capacity();
	di->batt_info.volt_mv = hisi_battery_voltage();
	di->batt_info.curr_ma = hisi_battery_current();
	di->batt_info.batt_temp = hisi_battery_temperature_for_charger();
	di->batt_info.batt_cycles = hisi_battery_cycle_count();

	ct_dbg("%s, fcc %d, soc %d, vol %d, cur %d, temp %d\n",
		__func__, di->batt_info.fcc, di->batt_info.soc,
		di->batt_info.volt_mv, di->batt_info.curr_ma,
		di->batt_info.batt_temp);
	/* soc check */
	if (di->batt_info.soc < 0 || di->batt_info.soc > FULL_SOC)
		di->batt_info.soc = 0;
}

static void hisi_charge_time_calc_work(struct work_struct *work)
{
	int ret = 0;
	struct hisi_chg_time_device *di = container_of(
		work, struct hisi_chg_time_device, charge_time_work.work);
	u64 timestemp = get_current_time();

	if (!di) {
		ct_dbg("%s di is null\n", __func__);
		return;
	}

	ct_dbg("%s +\n", __func__);
	di->work_running = 1;
	di->remaining_duration_with_valid = 0;

	hisi_chg_time_get_charge_info(di);
	hisi_chg_time_get_batt_info(di);
	ct_dbg("%s, pre soc :%d, soc %d\n", __func__,
		di->pre_capacity, di->batt_info.soc);

	if (di->charge_type_change) {
		di->start_capacity = di->batt_info.soc;
		di->pre_timestemp = timestemp;
		di->start_timestemp = timestemp;
		di->remaining_duration = 0;

		ret = hisi_chg_time_read_param_from_flash(di);
		if (ret < 0)
			goto next;

		/*reset current curve*/
		if (memset_s(&di->cur_curve, sizeof(struct hisi_chg_time_param),
			0, sizeof(struct hisi_chg_time_param)) != EOK)
			ct_err("%s():%d:memset_s fail!\n",
						__func__, __LINE__);

		ct_info("%s charge type change: soc %d, timestep %ld\n",
			__func__, di->batt_info.soc, di->start_timestemp);
	}

	if (di->chg_info.charge_type == CHG_NONE ||
		di->chg_info.charge_type == CHG_USB) {
		di->work_running = 0;
		ct_info(
			"%s, charge type is usb/none, exit work!\n", __func__);
		return;
	}

	ret = hisi_chg_time_param_select(di, di->chg_info.charge_type);
	if (ret) {
		di->work_running = 0;
		ct_err("%s param select error\n", __func__);
		return;
	}

	hisi_chg_time_self_study(di);
	hisi_chg_time_calc_param(di);
	di->pre_capacity = di->batt_info.soc;

	di->charge_type_change = 0;
	if (is_charge_full(di)) {
		di->work_running = 0;
		return;
	}
	ct_dbg("%s -\n", __func__);
next:
	queue_delayed_work(system_power_efficient_wq, &di->charge_time_work,
		msecs_to_jiffies(CHG_TYPE_DETECT_TIME));
}

static int hisi_charger_event_rcv(
	struct notifier_block *nb, unsigned long event, void *data)
{
	int ret = 0;
	struct hisi_chg_time_device *di =
		container_of(nb, struct hisi_chg_time_device, nb);

	ct_info("%s event: %ld\n", __func__, event);

	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
		if (di->work_running)
			return ret;
		di->work_running = 1;
		queue_delayed_work(system_power_efficient_wq,
			&di->charge_time_work, msecs_to_jiffies(1000));
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		di->work_running = 0;
		di->remaining_duration = 0;
		break;
	case VCHRG_START_CHARGING_EVENT:
		break;
	case VCHRG_NOT_CHARGING_EVENT:
		di->remaining_duration = 0;
		di->work_running = 0;
		break;
	case VCHRG_CHARGE_DONE_EVENT:
		break;
	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		break;
	case WIRELESS_TX_STATUS_CHANGED:
		break;
	default:
		ct_err("%s default run.\n", __func__);
		break;
	}

	return ret;
}
static int hisi_chg_time_dc_status_notifier_call(
	struct notifier_block *nb, unsigned long event, void *data)
{
	struct hisi_chg_time_device *di =
	container_of(nb, struct hisi_chg_time_device, direct_charger_nb);

	if (!di) {
		ct_err("%s di is null\n", __func__);
		return NOTIFY_OK;
	}
	switch (event) {
	case LVC_STATUS_CHARGING:
		di->chg_info.direct_charge_type = LVC_MODE;
		break;

	case SC_STATUS_CHARGING:
		di->chg_info.direct_charge_type = SC_MODE;
		break;
	case DC_STATUS_STOP_CHARGE:
	default:
		di->chg_info.direct_charge_type = UNDEFINED_MODE;
		break;
	}
	ct_err("%s direct charge type is %d\n", __func__,
		di->chg_info.direct_charge_type);

	return NOTIFY_OK;
}

static void parse_dts(struct device_node *np,
			struct hisi_chg_time_device *di)
{
	int ret;

	ret = of_property_read_u32(np, "adjust_by_volt",
				(u32 *)&(di->param_dts.adjust_by_volt));
	if (ret)
		ct_err("get adjust_by_volt fail\n");

	ret = of_property_read_u32(np, "adjust_by_current",
				(u32 *)&(di->param_dts.adjust_by_current));
	if (ret)
		ct_err("get adjust_by_current fail\n");

	ret = of_property_read_u32(np, "adjust_by_fcc",
				(u32 *)&(di->param_dts.adjust_by_fcc));
	if (ret)
		ct_err("get adjust_by_fcc fail\n");

	ret = of_property_read_u32(np, "adjust_by_temp",
				(u32 *)&(di->param_dts.adjust_by_temp));
	if (ret)
		ct_err("get adjust_by_temp fail\n");

}

static int hisi_chg_time_probe(struct platform_device *pdev)
{
	struct hisi_chg_time_device *di = NULL;
	struct device_node *np = NULL;
	int ret = 0;

	di = (struct hisi_chg_time_device *)devm_kzalloc(
		&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		ct_err("%s failed to alloc di struct\n", __func__);
		return -1;
	}
	di->dev = &pdev->dev;
	platform_set_drvdata(pdev, di);
	np = di->dev->of_node;

	parse_dts(np, di);

	di->nb.notifier_call = hisi_charger_event_rcv;
	ret = hisi_register_notifier(&di->nb, 1);
	if (ret)
		goto out0;

	di->direct_charger_nb.notifier_call =
		hisi_chg_time_dc_status_notifier_call;
	ret = direct_charge_notifier_chain_register(&di->direct_charger_nb);
	if (ret) {
		ct_err("%s register notifier failed\n", __func__);
		goto out1;
	}

	INIT_DELAYED_WORK(&di->charge_time_work, hisi_charge_time_calc_work);

#ifdef CONFIG_HISI_DEBUG_FS
	ret = device_create_file(di->dev, &dev_attr_flash_param);
	if (ret) {
		ct_err("failed to create file");
		goto out2;
	}
#endif

	g_hisi_chg_time_info = di;

	ct_err("%s succ\n", __func__);
	return 0;

#ifdef CONFIG_HISI_DEBUG_FS
out2:
	ret = direct_charge_notifier_chain_unregister(&di->direct_charger_nb);
	if (ret)
		ct_err("%s direct charge unregister fail\n", __func__);
#endif
out1:
	ret = hisi_unregister_notifier(&di->nb, 1);
	if (ret)
		ct_err("%s hisi_unregister_notifier fail\n", __func__);
out0:
	return -1;
}

static int hisi_chg_time_remove(struct platform_device *pdev)
{
	struct hisi_chg_time_device *di = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	kfree(di);
	g_hisi_chg_time_info = NULL;

	return 0;
}

static const struct of_device_id hisi_chg_time_of_match[] = {
	{.compatible = "hisilicon,charger_time", .data = NULL}, {},
};

MODULE_DEVICE_TABLE(of, hisi_chg_time_of_match);

static struct platform_driver hisi_chg_time_driver = {
	.probe = hisi_chg_time_probe,
	.remove = hisi_chg_time_remove,
	.driver = {
		.name = "charger-time",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_chg_time_of_match),
	},
};

static int __init hisi_chg_time_init(void)
{
	platform_driver_register(&hisi_chg_time_driver);

	return 0;
}

static void __exit hisi_chg_time_exit(void)
{
	platform_driver_unregister(&hisi_chg_time_driver);
}
late_initcall(hisi_chg_time_init);
module_exit(hisi_chg_time_exit);

MODULE_DESCRIPTION("charger time remaining driver");
MODULE_LICENSE("GPL v2");
