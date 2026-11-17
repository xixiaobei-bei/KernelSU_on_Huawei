/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: sysfs process func for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <securec.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include "coul_nv.h"
#include "coul_private_interface.h"
#ifdef CONFIG_CHARGE_TIME
#include "../charge_time/hisi_charge_time.h"
#endif
#include "coul_sysfs.h"
#ifdef CONFIG_COUL_POLAR
#include "coul_update_polar_info.h"
#endif

#define BATTERY_DEC_ENABLE      1
#define DEFAULT_INPUT_CAPACITY  50
#define STRTOL_DECIMAL_BASE     10
#define INPUT_CAPACITY_LOWER_LIMIT      0
#define INPUT_CAPACITY_UPPER_LIMIT      100
#define CALI_ADC_LOWER_LIMIT    0
#define CALI_ADC_UPPER_LIMIT    100

struct gaugelog_data {
	int temp;
	int voltage;
	int cur;
	int ufcapacity;
	int capacity;
	int afcapacity;
	int rm;
	int fcc;
	int uuc;
	int cc;
	int delta_rc;
	int ocv;
	int rbatt;
	unsigned int pd_charge;
	int high_pre_qmax;
	int charge_time;
};

static int g_pl_calibration_en;
static unsigned int g_hand_chg_capacity_flag;
static unsigned int g_input_capacity = DEFAULT_INPUT_CAPACITY;
static struct device *g_coul_dev;

int get_pl_calibration_en(void)
{
	return g_pl_calibration_en;
}

int get_hand_chg_capacity_flag(void)
{
	return g_hand_chg_capacity_flag;
}

int get_input_capacity(void)
{
	return g_input_capacity;
}

#ifdef CONFIG_SYSFS
static int g_do_save_offset_ret;
static int g_curr_cal_temp;

enum coul_sysfs_type {
	COUL_SYSFS_PL_CALIBRATION_EN = 0,
	COUL_SYSFS_PL_V_OFFSET_A,
	COUL_SYSFS_PL_V_OFFSET_B,
	COUL_SYSFS_PL_C_OFFSET_A,
	COUL_SYSFS_PL_C_OFFSET_B,
	COUL_SYSFS_ATE_V_OFFSET_A,
	COUL_SYSFS_ATE_V_OFFSET_B,
	COUL_SYSFS_DO_SAVE_OFFSET,
	COUL_SYSFS_GAUGELOG_HEAD,
	COUL_SYSFS_GAUGELOG,
	COUL_SYSFS_HAND_CHG_CAPACITY_FLAG,
	COUL_SYSFS_INPUT_CAPACITY,
	COUL_SYSFS_ABS_CC,
	COUL_SYSFS_BATTERY_ID_VOLTAGE,
	COUL_SYSFS_BATTERY_BRAND_NAME,
	COUL_SYSFS_RBATT,
	COUL_SYSFS_REAL_SOC,
	COUL_SYSFS_CALI_ADC,
	COUL_SYSFS_CURR_CAL_TEMP,
};

#define coul_sysfs_field(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, coul_sysfs_show, store), \
	.name = COUL_SYSFS_##n, \
}

#define coul_sysfs_field_rw(_name, n) \
	coul_sysfs_field(_name, n, S_IWUSR | S_IRUGO, coul_sysfs_store)

#define coul_sysfs_field_ro(_name, n) coul_sysfs_field(_name, n, S_IRUGO, NULL)

static ssize_t coul_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t coul_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

struct coul_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static struct coul_sysfs_field_info g_coul_sysfs_field_tbl[] = {
	coul_sysfs_field_rw(pl_calibration_en, PL_CALIBRATION_EN),
	coul_sysfs_field_rw(pl_v_offset_a, PL_V_OFFSET_A),
	coul_sysfs_field_rw(pl_v_offset_b, PL_V_OFFSET_B),
	coul_sysfs_field_rw(pl_c_offset_a, PL_C_OFFSET_A),
	coul_sysfs_field_rw(pl_c_offset_b, PL_C_OFFSET_B),
	coul_sysfs_field_ro(ate_v_offset_a, ATE_V_OFFSET_A),
	coul_sysfs_field_ro(ate_v_offset_b, ATE_V_OFFSET_B),
	coul_sysfs_field_rw(do_save_offset, DO_SAVE_OFFSET),
	coul_sysfs_field_ro(gaugelog, GAUGELOG),
	coul_sysfs_field_ro(gaugelog_head, GAUGELOG_HEAD),
	coul_sysfs_field_rw(hand_chg_capacity_flag, HAND_CHG_CAPACITY_FLAG),
	coul_sysfs_field_rw(input_capacity, INPUT_CAPACITY),
	coul_sysfs_field_ro(abs_cc, ABS_CC),
	coul_sysfs_field_ro(battery_id_voltage, BATTERY_ID_VOLTAGE),
	coul_sysfs_field_ro(battery_brand_name, BATTERY_BRAND_NAME),
	coul_sysfs_field_ro(rbatt, RBATT),
	coul_sysfs_field_ro(real_soc, REAL_SOC),
	coul_sysfs_field_rw(cali_adc, CALI_ADC),
	coul_sysfs_field_ro(curr_cal_temp, CURR_CAL_TEMP),
};

static struct attribute *g_coul_sysfs_attrs[ARRAY_SIZE(g_coul_sysfs_field_tbl) + 1];

static const struct attribute_group g_coul_sysfs_attr_group = {
	.attrs = g_coul_sysfs_attrs,
};

/* initialize coul_sysfs_attrs[] for coul attribute */
static void coul_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(g_coul_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		g_coul_sysfs_attrs[i] = &g_coul_sysfs_field_tbl[i].attr.attr;
	g_coul_sysfs_attrs[limit] = NULL;
}
/* get the current device_attribute from charge_sysfs_field_tbl by attr's name */
static struct coul_sysfs_field_info *coul_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit = ARRAY_SIZE(g_coul_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, g_coul_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}
	if (i >= limit)
		return NULL;

	return &g_coul_sysfs_field_tbl[i];
}

static ssize_t coul_sysfs_show_offset(struct smartstar_coul_device *di,
	u8 name, char *buf)
{
	int val;

	if ((di == NULL) || (buf == NULL))
		return -EINVAL;
	switch (name) {
	case COUL_SYSFS_PL_CALIBRATION_EN:
		return snprintf(buf, PAGE_SIZE, "%d\n", g_pl_calibration_en);
	case COUL_SYSFS_PL_V_OFFSET_A:
		val = DEFAULT_V_OFF_A;
		get_coul_cali_params(&val, NULL, NULL, NULL);
		return snprintf(buf, PAGE_SIZE, "%d\n", val);
	case COUL_SYSFS_PL_V_OFFSET_B:
		val = DEFAULT_V_OFF_B;
		get_coul_cali_params(NULL, &val, NULL, NULL);
		return snprintf(buf, PAGE_SIZE, "%d\n", val);
	case COUL_SYSFS_PL_C_OFFSET_A:
		val = DEFAULT_C_OFF_A;
		get_coul_cali_params(NULL, NULL, &val, NULL);
		return snprintf(buf, PAGE_SIZE, "%d\n", val);
	case COUL_SYSFS_PL_C_OFFSET_B:
		val = DEFAULT_C_OFF_B;
		get_coul_cali_params(NULL, NULL, NULL, &val);
		return snprintf(buf, PAGE_SIZE, "%d\n", val);
	case COUL_SYSFS_ATE_V_OFFSET_A:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->coul_dev_ops->get_ate_a());
	case COUL_SYSFS_ATE_V_OFFSET_B:
		return snprintf(buf, PAGE_SIZE, "%d\n",
			di->coul_dev_ops->get_ate_b());
	case COUL_SYSFS_DO_SAVE_OFFSET:
		return snprintf(buf, PAGE_SIZE, "%d\n", g_do_save_offset_ret);
	default:
		coul_core_err("(%s)NODE ERR!!HAVE NO THIS NODE: %u\n",
			__func__, name);
		break;
	}
	return 0;
}

static void get_log_data(struct gaugelog_data *log_data)
{
	log_data->temp = coul_get_battery_temperature();
	log_data->voltage = coul_get_battery_voltage_mv();
	log_data->cur = -(coul_get_battery_current_ma());
	log_data->ufcapacity = coul_battery_unfiltered_capacity();
	log_data->capacity = coul_get_battery_capacity();
	log_data->afcapacity = bci_show_capacity();
	log_data->rm = coul_get_battery_rm();
	log_data->fcc = coul_get_battery_fcc();
	log_data->uuc = coul_get_battery_uuc();
	log_data->cc = coul_get_battery_cc() / PERMILLAGE;
	log_data->delta_rc = coul_get_battery_delta_rc();
	log_data->ocv = coul_get_battery_ocv();
	log_data->rbatt = coul_get_battery_resistance();
	log_data->pd_charge = coul_get_pd_charge_flag();
	log_data->high_pre_qmax = coul_get_battery_qmax();
#ifdef CONFIG_CHARGE_TIME
	log_data->charge_time = hisi_chg_time_remaining(0);
#else
	log_data->charge_time = -1;
#endif
}

static ssize_t show_gaugelog(struct smartstar_coul_device *di, char *buf)
{
	int ret, len;
	struct gaugelog_data log_data = {0};
#ifdef CONFIG_COUL_POLAR
	struct polar_sysfs_info info = {0};

	get_polar_sysfs_info(&info);
#endif

	get_log_data(&log_data);

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
		"%-6d  %-6d  %-8d  %-6d  %-3d  %-5d  %-5d  %-6d  %-6d"
		"  %-7d  %-5d  %-6d  %-5d  %-4d  %-7d  %-5d  %-5d  "
		"%-9d   %-8d   ",
		log_data.voltage, (signed short)log_data.cur,
		log_data.ufcapacity, log_data.capacity, log_data.afcapacity,
		log_data.charge_time, log_data.rm, log_data.fcc,
		log_data.high_pre_qmax, log_data.uuc, log_data.cc,
		log_data.delta_rc, log_data.pd_charge, log_data.temp,
		log_data.ocv, log_data.rbatt, di->batt_limit_fcc / PERMILLAGE,
		di->last_ocv_level, di->batt_ocv_valid_to_refresh_fcc);
	if (ret < 0) {
		coul_core_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return 0;
	}

	len = strlen(buf);
#ifdef CONFIG_COUL_POLAR
	ret = snprintf_s(buf + len, PAGE_SIZE - len, PAGE_SIZE - len - 1,
		"%-7d    %-5d    %-5d    %-5d    %-9ld"
		"    %-7d    %-9d   %-8d  ",
		info.ocv_old, info.ori_vol, info.ori_cur,
		info.err_a, info.vol, info.curr_5s,
		info.curr_peak, info.curr_ua);
#else
	ret = snprintf_s(buf + len, PAGE_SIZE - len, PAGE_SIZE - len - 1,
		"%-7d    %-5d    %-5d    %-5d    %-9d"
		"    %-7d    %-9d   %-8d  ",
		0, 0, 0, 0, 0, 0, 0, 0);
#endif
	if (ret < 0) {
		coul_core_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return 0;
	}
	return strlen(buf);
}

static ssize_t sysfs_node_process(struct coul_sysfs_field_info *info,
	struct smartstar_coul_device *di, char *buf)
{
	int val, ufcapacity, rbatt;

	switch (info->name) {
	case COUL_SYSFS_GAUGELOG_HEAD:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s",
			"ss_VOL  ss_CUR  ss_ufSOC  ss_SOC  SOC  cTime  "
			"ss_RM  ss_FCC  Qmax     ss_UUC  ss_CC  ss_dRC"
			"  pdFlag Temp  ss_OCV   rbatt  fcc    ocv_level  "
			"fcc_flag   ocv_old    p_vol   p_cur    err_a"
			"    polar_vol   avg_cur    peak_cur"
			"    ishort   ");
	case COUL_SYSFS_GAUGELOG:
		return show_gaugelog(di, buf);
	case COUL_SYSFS_HAND_CHG_CAPACITY_FLAG:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			g_hand_chg_capacity_flag);
	case COUL_SYSFS_INPUT_CAPACITY:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			g_input_capacity);
	case COUL_SYSFS_ABS_CC:
		val = di->coul_dev_ops->get_abs_cc() +
			(di->coul_dev_ops->calculate_cc_uah() / PERMILLAGE);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", val);
	case COUL_SYSFS_BATTERY_ID_VOLTAGE:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			di->batt_id_vol);
	case COUL_SYSFS_BATTERY_BRAND_NAME:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n",
			di->batt_data->batt_brand);
	case COUL_SYSFS_RBATT:
		rbatt = coul_get_battery_resistance();
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", rbatt);
	case COUL_SYSFS_REAL_SOC:
		ufcapacity = coul_battery_unfiltered_capacity();
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			ufcapacity);
	case COUL_SYSFS_CALI_ADC:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", 0);
	case COUL_SYSFS_CURR_CAL_TEMP:
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			g_curr_cal_temp);
	default:
		coul_core_err("(%s)NODE ERR!!HAVE NO THIS NODE: %u\n",
			__func__, info->name);
		return 0;
	}
}

/* show the value for all coul device's node, return value:  0-sucess or others-fail */
static ssize_t coul_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct coul_sysfs_field_info *info = NULL;
	struct smartstar_coul_device *di = dev_get_drvdata(dev);

	info = coul_sysfs_field_lookup(attr->attr.name);
	if ((di == NULL) || (info == NULL))
		return -EINVAL;
	if (info->name <= COUL_SYSFS_DO_SAVE_OFFSET)
		return coul_sysfs_show_offset(di, info->name, buf);

	return sysfs_node_process(info, di, buf);
}

static ssize_t get_offset_value(const char *buf, size_t count, u8 name,
	int *variable)
{
	long val = 0;

	if (kstrtol(buf, STRTOL_DECIMAL_BASE, &val) < 0)
		return -EINVAL;
	*variable = val;
	if (name == COUL_SYSFS_PL_C_OFFSET_A)
		g_curr_cal_temp = coul_get_chip_temp();
	return count;
}

static ssize_t get_pl_calibation_en(struct smartstar_coul_device *di,
	const char *buf, size_t count, int *variable)
{
	long val = 0;

	if ((kstrtol(buf, STRTOL_DECIMAL_BASE, &val) < 0) ||
		(val < 0) || (val > 1))
		return -EINVAL;
	*variable = val;
	if (*variable == TRUE)
		di->coul_dev_ops->cali_auto_off();
	coul_cali_adc(di);
	return count;
}

static ssize_t get_capacity_flag(const char *buf, size_t count, int *variable)
{
	long val = 0;

	if ((kstrtol(buf, STRTOL_DECIMAL_BASE, &val) < 0) ||
		(val < 0) || (val > 1))
		return -EINVAL;
	*variable = val;
	coul_core_info("hand_chg_capacity_flag is set to %d\n", *variable);
	return count;
}

static ssize_t get_input_capacity_value(const char *buf, size_t count,
	int *variable)
{
	long val = 0;

	if ((kstrtol(buf, STRTOL_DECIMAL_BASE, &val) < 0) ||
		(val < INPUT_CAPACITY_LOWER_LIMIT) ||
		(val > INPUT_CAPACITY_UPPER_LIMIT))
		return -EINVAL;
	*variable = val;
	return count;
}

static ssize_t get_cali_adc(struct smartstar_coul_device *di,
	const char *buf, size_t count)
{
	long val = 0;

	if ((kstrtol(buf, STRTOL_DECIMAL_BASE, &val) < 0) ||
		(val < CALI_ADC_LOWER_LIMIT) || (val > CALI_ADC_UPPER_LIMIT))
		return -EINVAL;
	coul_core_info("cali_adc = %ld\n", val);
	if (val == 1)
		coul_cali_adc(di);
	return count;
}

/* set the value for coul_data's node which is can be written, return value:  0-sucess or others-fail */
static ssize_t coul_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct coul_sysfs_field_info *info = NULL;
	struct smartstar_coul_device *di = dev_get_drvdata(dev);
	int val, ret, v_offset_a, c_offset_a;

	info = coul_sysfs_field_lookup(attr->attr.name);
	if ((di == NULL) || (info == NULL))
		return -EINVAL;

	switch (info->name) {
	case COUL_SYSFS_PL_CALIBRATION_EN:
		return get_pl_calibation_en(di, buf, count, &g_pl_calibration_en);
	case COUL_SYSFS_PL_V_OFFSET_A:
		val = DEFAULT_V_OFF_A;
		ret = get_offset_value(buf, count, COUL_SYSFS_PL_V_OFFSET_A, &val);
		set_coul_cali_params(&val, NULL, NULL, NULL);
		return ret;
	case COUL_SYSFS_PL_V_OFFSET_B:
		val = DEFAULT_V_OFF_B;
		ret = get_offset_value(buf, count, COUL_SYSFS_PL_V_OFFSET_B, &val);
		set_coul_cali_params(NULL, &val, NULL, NULL);
		return ret;
	case COUL_SYSFS_PL_C_OFFSET_A:
		val = DEFAULT_C_OFF_A;
		ret = get_offset_value(buf, count, COUL_SYSFS_PL_C_OFFSET_A, &val);
		set_coul_cali_params(NULL, NULL, &val, NULL);
		return ret;
	case COUL_SYSFS_PL_C_OFFSET_B:
		val = DEFAULT_C_OFF_B;
		ret = get_offset_value(buf, count, COUL_SYSFS_PL_C_OFFSET_B, &val);
		set_coul_cali_params(NULL, NULL, NULL, &val);
		return ret;
	case COUL_SYSFS_DO_SAVE_OFFSET:
		v_offset_a = DEFAULT_V_OFF_A;
		c_offset_a = DEFAULT_C_OFF_A;
		get_coul_cali_params(&v_offset_a, NULL, &c_offset_a, NULL);
		g_do_save_offset_ret = save_cali_param(g_curr_cal_temp);
		coul_core_info("do_save_offset_ret:%d, v_offset_a:%d, c_offset_a:%d\n",
			g_do_save_offset_ret, v_offset_a, c_offset_a);
		break;
	case COUL_SYSFS_HAND_CHG_CAPACITY_FLAG:
		return get_capacity_flag(buf, count, &g_hand_chg_capacity_flag);
	case COUL_SYSFS_INPUT_CAPACITY:
		return get_input_capacity_value(buf, count, &g_input_capacity);
	case COUL_SYSFS_CALI_ADC:
		return get_cali_adc(di, buf, count);
	default:
		coul_core_err("(%s)NODE ERR! HAVE NO THIS NODE: %u\n", __func__, info->name);
		break;
	}
	return count;
}

/* create the coul device sysfs group, return value:   0-sucess or others-fail  */
static int coul_sysfs_create_group(struct smartstar_coul_device *di)
{
	coul_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &g_coul_sysfs_attr_group);
}

/* remove the charge device sysfs group */
static inline void coul_sysfs_remove_group(
	struct smartstar_coul_device *di)
{
	sysfs_remove_group(&di->dev->kobj, &g_coul_sysfs_attr_group);
}
#else
static int coul_sysfs_create_group(
	const struct smartstar_coul_device *di)
{
	return 0;
}

static inline void coul_sysfs_remove_group(
	const struct smartstar_coul_device *di)
{
	/* do nothing */
}
#endif

static ssize_t decress_batt_flag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct smartstar_coul_device *di = dev_get_drvdata(dev);

	if ((di == NULL) || (di->batt_data->pc_temp_ocv_lut0 == NULL))
		return 0;
	if (strstr(saved_command_line, "batt_decress_flag=true"))
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d--%d\n", 1,
			di->batt_data->pc_temp_ocv_lut0->ocv[0][0]);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d--%d\n", 0,
		di->batt_data->pc_temp_ocv_lut0->ocv[0][0]);
}

static int decress_vol_clear_battery_data(
	struct smartstar_coul_device *di)
{
	errno_t err;

	if (di == NULL)
		return 0;
	di->batt_chargecycles = 0;
	di->batt_changed_flag = 1;
	di->batt_limit_fcc = 0;
	di->adjusted_fcc_temp_lut = NULL; /* enable it when test ok */
	di->is_nv_need_save = 1;
	if (di->coul_dev_ops == NULL)
		return 0;
	di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	di->coul_dev_ops->clear_last_soc_flag();
	/* clear safe record fcc */
	di->nv_info.latest_record_index = 0;
	err = memset_s(di->nv_info.real_fcc_record,
		sizeof(di->nv_info.real_fcc_record),
		0, sizeof(di->nv_info.real_fcc_record));
	if (err != EOK) {
		coul_core_err("[%s]memset_s fail, err=%d\n", __func__, err);
		return 0;
	}
	coul_core_debug("battery changed, clean charge data\n");
	return 1;
}

static ssize_t do_clear_chg_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct smartstar_coul_device *di = dev_get_drvdata(dev);

	if (di == NULL)
		return 0;
	if (decress_vol_clear_battery_data(di))
		coul_core_debug("clear chg data done");
	return 0;
}

static DEVICE_ATTR(decress_flag, S_IRUGO, decress_batt_flag_show,
	do_clear_chg_data);

static struct attribute *g_flag_attr[] = {
	&dev_attr_decress_flag.attr,
	NULL,
};

static struct attribute_group g_flag_group = {
	.attrs = g_flag_attr,
};

static void create_coul_dev(struct smartstar_coul_device *di)
{
	int retval;

	if (g_coul_dev != NULL) {
		retval = sysfs_create_link(&g_coul_dev->kobj, &di->dev->kobj,
			"coul_data");
		if (retval != 0) {
			coul_core_err("%s failed to create sysfs link\n",
				__func__);
		} else if (di->dec_state == BATTERY_DEC_ENABLE) {
			retval = sysfs_create_group(&di->dev->kobj,
				&g_flag_group);
			if (retval != 0)
				coul_core_err("%s failed to create sysfs flag group\n",
					__func__);
		}
	} else {
		coul_core_err("%s failed to create new_dev\n", __func__);
	}
}

int coul_create_sysfs(void)
{
	int retval;
	struct class *power_class = NULL;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_err("%s input di is null", __func__);
		return -1;
	}

	retval = coul_sysfs_create_group(di);
	if (retval) {
		coul_core_err("%s failed to create sysfs group\n", __func__);
		return -1;
	}
	power_class = power_sysfs_get_class("hw_power");
	if (power_class) {
		if (g_coul_dev == NULL) {
			g_coul_dev = power_sysfs_get_device("hw_power", "coul");
			if (IS_ERR(g_coul_dev))
				g_coul_dev = NULL;
		}
		create_coul_dev(di);
	}
	return retval;
}

struct device *get_coul_dev_sysfs(void)
{
	return g_coul_dev;
}

