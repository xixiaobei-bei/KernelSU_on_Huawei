/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: temp functions for coul module
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

#ifdef CONFIG_HUAWEI_BATTERY_TEMP_FITTING
#include <huawei_platform/power/battery_temp_fitting.h>
#endif
#include "coul_temp.h"

#define T_V_ARRAY_LENGTH        31 /* NTC Table length */
#define TEMP_IPC_GET_ABNORMAL   100
#define TEMPERATURE_CHANGE_LIMIT        50
#define T_V_ARRAY_LENGTH        31 /* NTC Table length */
#define ABNORMAL_BATT_TEMPERATURE_POWEROFF 670
#define DELTA_TEMP 150
#define LOW_BATT_TEMP_CHECK_THRESHOLD (-100)
#define V_PU    1800L /* mV */
#define R_PU    18000000L /* momh */
#define A_2_NA  1000000000L
#define MA_2_NA 1000000L
#define MAX_ADC_OFFSET          4096
#define TEMP_RETRY_TIMES        3
#define STABLY_TEMP_RETRY_TIMES 5
/* coul get temperature */
static int g_batt_temp_compensation_en;
static int g_batt_temp_compensation_r; /* momh */

/* NTC R-T Table */
static int g_t_r_table[][2] = {
	{ -40, 205200 }, { -39, 193800 }, { -38, 183100 }, { -37, 173100 },
	{ -36, 163600 }, { -35, 154800 }, { -34, 146500 }, { -33, 138700 },
	{ -32, 131300 }, { -31, 124400 }, { -30, 117900 }, { -29, 111800 },
	{ -28, 106000 }, { -27, 100600 }, { -26, 95510 }, { -25, 90690 },
	{ -24, 86150 }, { -23, 81860 }, { -22, 77810 }, { -21, 73990 },
	{ -20, 70370 }, { -19, 66960 }, { -18, 63740 }, { -17, 60690 },
	{ -16, 57800 }, { -15, 55070 }, { -14, 52490 }, { -13, 50040 },
	{ -12, 47720 }, { -11, 45520 }, { -10, 43440 }, { -9, 41460 },
	{ -8, 39590 }, { -7, 37810 }, { -6, 36130 }, { -5, 34530 },
	{ -4, 33000 }, { -3, 31560 }, { -2, 30190 }, { -1, 28880 },
	{ 0, 27640 }, { 1, 26460 }, { 2, 25330 }, { 3, 24260 }, { 4, 23240 },
	{ 5, 22270 }, { 6, 21350 }, { 7, 20470 }, { 8, 19630 }, { 9, 18830 },
	{ 10, 18060 }, { 11, 17340 }, { 12, 16640 }, { 13, 15980 },
	{ 14, 15350 }, { 15, 14740 }, { 16, 14170 }, { 17, 13620 },
	{ 18, 13090 }, { 19, 12590 }, { 20, 12110 }, { 21, 11650 },
	{ 22, 11210 }, { 23, 10790 }, { 24, 10380 }, { 25, 10000 },
	{ 26, 9632 }, { 27, 9279 }, { 28, 8942 }, { 29, 8619 }, { 30, 8309 },
	{ 31, 8012 }, { 32, 7727 }, { 33, 7454 }, { 34, 7192 }, { 35, 6941 },
	{ 36, 6700 }, { 37, 6468 }, { 38, 6246 }, { 39, 6033 }, { 40, 5828 },
	{ 41, 5631 }, { 42, 5441 }, { 43, 5259 }, { 44, 5084 }, { 45, 4916 },
	{ 46, 4754 }, { 47, 4598 }, { 48, 4448 }, { 49, 4304 }, { 50, 4165 },
	{ 51, 4031 }, { 52, 3902 }, { 53, 3778 }, { 54, 3658 }, { 55, 3543 },
	{ 56, 3432 }, { 57, 3325 }, { 58, 3222 }, { 59, 3123 }, { 60, 3027 },
	{ 61, 2934 }, { 62, 2845 }, { 63, 2759 }, { 64, 2676 }, { 65, 2595 },
	{ 66, 2518 }, { 67, 2443 }, { 68, 2371 }, { 69, 2301 }, { 70, 2233 },
	{ 71, 2168 }, { 72, 2105 }, { 73, 2045 }, { 74, 1986 }, { 75, 1929 },
	{ 76, 1874 }, { 77, 1821 }, { 78, 1770 }, { 79, 1720 }, { 80, 1672 },
	{ 81, 1625 }, { 82, 1580 }, { 83, 1536 }, { 84, 1493 }, { 85, 1451 },
	{ 86, 1411 }, { 87, 1372 }, { 88, 1334 }, { 89, 1297 }, { 90, 1261 },
};

/* NTC Table */
static int g_t_v_table[][2] = {
	{ -273, 4095 }, { -40, 3764 }, { -36, 3689 }, { -32, 3602 },
	{ -28, 3500 }, { -24, 3387 }, { -20, 3261 }, { -16, 3122 },
	{ -12, 2973 }, { -8, 2814 }, { -4, 2650 }, { 0, 2480 }, { 4, 2308 },
	{ 8, 2136 }, { 12, 1967 }, { 16, 1803 }, { 20, 1646 }, { 24, 1497 },
	{ 28, 1360 }, { 32, 1230 }, { 36, 1111 }, { 40, 1001 }, { 44, 903 },
	{ 48, 812 }, { 52, 729 }, { 56, 655 }, { 60, 590 }, { 64, 531 },
	{ 74, 406 }, { 84, 313 }, { 125, 110 }, { 0, 0 },
};

void contexthub_thermal_init(void)
{
#ifdef CONFIG_THERMAL_CONTEXTHUB
	int i;
	int v_t_table[T_V_ARRAY_LENGTH + 1][2];
	struct hw_chan_table *p_ddr_header = NULL;
	char *p_chub_ddr = NULL;
	void __iomem *share_addr = NULL;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return;
	share_addr = ioremap_wc(CONTEXTHUB_THERMAL_DDR_HEADER_ADDR,
		CONTEXTHUB_THERMAL_DDR_TOTAL_SIZE);
	if (share_addr == NULL) {
		coul_core_err("[%s]share_addr ioremap_wc failed\n", __func__);
		return;
	}
	memset((void *)share_addr, 0xFF, CONTEXTHUB_THERMAL_DDR_TOTAL_SIZE);
	p_chub_ddr = share_addr + CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE;
	p_ddr_header = (struct hw_chan_table *)share_addr;
	p_ddr_header++;
	p_ddr_header->usr_id = 0xFFFF;
	p_ddr_header->hw_channel = (unsigned short int)di->adc_batt_temp;
	p_ddr_header->table_id = (unsigned short int)HKADC_BAT_TABLEID;
	p_ddr_header->table_size = sizeof(v_t_table);
	if (p_ddr_header->table_size > CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE) {
		coul_core_err("[%s]tableSIZE[%u]MAX[%d]\n", __func__,
			p_ddr_header->table_size,
			CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE);
		iounmap(share_addr);
		return;
	}

	for (i = 0; i <= T_V_ARRAY_LENGTH; i++) {
		v_t_table[i][0] = adc_to_volt(g_t_v_table[T_V_ARRAY_LENGTH - i][1]);
		v_t_table[i][1] = g_t_v_table[T_V_ARRAY_LENGTH - i][0];
	}

	memcpy((void *)(p_chub_ddr + CONTEXTHUB_THERMAL_DDR_MEMBERS_SIZE *
		HKADC_BAT_TABLEID),
	       (void *)v_t_table, p_ddr_header->table_size);
	iounmap(share_addr);
#endif
}

static int get_ntc_table(const struct device_node *np)
{
	u32 ntc_table[T_V_ARRAY_LENGTH] = {0};
	int i, ret, len;

	len = of_property_count_u32_elems(np, "ntc_table");
	coul_core_debug("Load ntc length is %d\n", len);
	if (len != T_V_ARRAY_LENGTH + 1) {
		if (len == -1)
			coul_core_err("%s, ntc_table not exist, use default array\n",
				__func__);
		else
			coul_core_err("%s, ntc_table length is %d, use default array\n",
				__func__, len);
		return -1;
	}

	ret = of_property_read_u32_array(np, "ntc_table",
		ntc_table, T_V_ARRAY_LENGTH);
	if (ret) {
		coul_core_err("%s, ntc_table not exist, use default array\n",
			__func__);
		return -1;
	}

	for (i = 0; i < T_V_ARRAY_LENGTH; i++) {
		g_t_v_table[i][1] = ntc_table[i];
		coul_core_debug("T_V_TABLE[%d][1] = %d\t", i, g_t_v_table[i][1]);
	}
	return 0;
}

static void get_batt_temp_compensation_dts(
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "batt_temp_compensation_en",
		&g_batt_temp_compensation_en);
	if (ret) {
		coul_core_err("error:get batt_temp_compensation_en failed\n");
		g_batt_temp_compensation_en = 0;
	}
	ret = of_property_read_u32(np, "batt_temp_compensation_r",
		&g_batt_temp_compensation_r);
	if (ret) {
		coul_core_err("error:get batt_temp_compensation_r failed\n");
		g_batt_temp_compensation_r = 0;
	}
	coul_core_info("batt_temp_compensation_en:%d, compensation_r %d\n",
		g_batt_temp_compensation_en, g_batt_temp_compensation_r);
}

void get_temp_dts(struct device_node *np)
{
	if (np == NULL)
		return;

	if (get_ntc_table(np) == -1)
		coul_core_err("Use default ntc_table\n");
	else
		coul_core_err("Use ntc_table from dts\n");
	get_batt_temp_compensation_dts(np);
}

/*
 * convert battery temperature to adc sampling Code value
 * temp: battery temperature, return value: adc sampling Code value
 */
int coul_convert_temp_to_adc(int temp)
{
	int i;
	int adc;

	if (temp <= g_t_v_table[0][0])
		return g_t_v_table[0][1];
	if (temp >= g_t_v_table[T_V_ARRAY_LENGTH - 1][0])
		return g_t_v_table[T_V_ARRAY_LENGTH - 1][1];
	for (i = 0; i < T_V_ARRAY_LENGTH; i++) {
		if (temp == g_t_v_table[i][0])
			return g_t_v_table[i][1];
		if (temp < g_t_v_table[i][0])
			break;
	}
	if (i == 0)
		return g_t_v_table[0][1];
	adc = g_t_v_table[i - 1][1] + (temp - g_t_v_table[i - 1][0]) *
		(g_t_v_table[i][1] - g_t_v_table[i - 1][1]) /
		(g_t_v_table[i][0] - g_t_v_table[i - 1][0]);
	return adc;
}

static int adc_to_temp_permille(int temp_volt)
{
	int i;
	int temprature = 0;

	if (temp_volt >= g_t_v_table[0][1])
		return g_t_v_table[0][0] * PERMILLAGE;
	if (temp_volt <= g_t_v_table[T_V_ARRAY_LENGTH - 1][1])
		return g_t_v_table[T_V_ARRAY_LENGTH - 1][0] * PERMILLAGE;
	for (i = 0; i < T_V_ARRAY_LENGTH; i++) {
		if (temp_volt == g_t_v_table[i][1])
			return g_t_v_table[i][0] * PERMILLAGE;
		if (temp_volt > g_t_v_table[i][1])
			break;
	}
	if (i == 0)
		return g_t_v_table[0][0] * PERMILLAGE;
	if ((g_t_v_table[i][1] - g_t_v_table[i - 1][1]) != 0)
		temprature = g_t_v_table[i - 1][0] * PERMILLAGE +
			(long)(temp_volt - g_t_v_table[i - 1][1]) * PERMILLAGE *
			(g_t_v_table[i][0] - g_t_v_table[i - 1][0]) /
			(g_t_v_table[i][1] - g_t_v_table[i - 1][1]);
	return temprature;
}

/* return momh */
static long get_ntc_moh_from_temp(int temprature)
{
	unsigned int i;

	temprature = temprature / PERMILLAGE;
	for (i = 0; i < ARRAY_SIZE(g_t_r_table); i++) {
		if (temprature == g_t_r_table[i][0])
			return (long)g_t_r_table[i][1] * PERMILLAGE;
	}
	return -1;
}

static int check_ntc_moh_temp(int temprature)
{
	int len = ARRAY_SIZE(g_t_r_table);

	temprature = temprature / PERMILLAGE;
	if ((temprature < g_t_r_table[0][0]) ||
		(temprature > g_t_r_table[len - 1][0]))
		return -1;
	return 0;
}

static int _coul_battery_temp_compensation(int temp_orig_permille,
	long ibat)
{
	long r_ntc; /* momh */
	long i_ntc; /* nA */
	long r_compensate; /* momh */
	long r_ntc_new; /* momh */
	long v_adc; /* mV */
	int tbat_code;
	int temp_new_permille;

	/* 1.get ntc res from ADC temperature */
	r_ntc = get_ntc_moh_from_temp(temp_orig_permille);
	if (r_ntc < 0) {
		coul_core_err("get_ntc_res_from_temp err %d\n",
			      temp_orig_permille);
		return temp_orig_permille;
	}

	/* 2. calc I_ntc */
	i_ntc = (long long)V_PU * A_2_NA / (r_ntc + R_PU);
	if (i_ntc == 0)
		return temp_orig_permille;

	/* 3. R_compensate = I_(chg/disch)/I_NTC * (R_fpc+R_sense+R_BTB) */
	ibat = ibat * MA_2_NA;
	r_compensate = (long long)ibat * g_batt_temp_compensation_r / i_ntc;

	/* 4. calc I_ntcR_ntc_new */
	r_ntc_new = r_ntc - r_compensate;

	/* 5. Vadc =  Vpu * (R_ntc_new / (R_ntc_new + R_pu)) */
	if (r_ntc_new + R_PU == 0)
		return temp_orig_permille;
	v_adc = (long long)V_PU * r_ntc_new / (r_ntc_new + R_PU);

	/* 6. 4096 is MAX offset, ADC Volt is 1.8V */
	tbat_code = v_adc * MAX_ADC_OFFSET / V_PU;

	temp_new_permille = adc_to_temp_permille(tbat_code);

	coul_core_info("temp_orig %d, ibat %ld mA, temp_new %d\n",
		temp_orig_permille, ibat / PERMILLAGE / PERMILLAGE,
		temp_new_permille);

	return temp_new_permille;
}

static int coul_battery_temperature_compensation(
	int temp_orig_permille)
{
	long ibat; /* nA */
	int temp_new_permille;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return temp_orig_permille;

	if (g_batt_temp_compensation_en == 0)
		return temp_orig_permille;

	if (check_ntc_moh_temp(temp_orig_permille) < 0) {
		coul_core_err("check_ntc_res_temp err %d\n", temp_orig_permille);
		return temp_orig_permille;
	}

	ibat = (long)-(di->coul_dev_ops->get_battery_current_ua() / PERMILLAGE);
	if (ibat < DISCHARGE_CURRENT_OVERHIGH_TH)
		ibat = DISCHARGE_CURRENT_OVERHIGH_TH;
	if (ibat > bci_get_charge_current_overhigh_th())
		ibat = bci_get_charge_current_overhigh_th();

	temp_new_permille =
		_coul_battery_temp_compensation(temp_orig_permille, ibat);

	return temp_new_permille;
}

int coul_retry_temp_permille(enum battery_temp_user user)
{
	int cnt = 0;
	int t_adc, temperature;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		goto error;

	/* default is no battery in sft and udp, so here temp is fixed 25 */
	if (di->is_board_type != BAT_BOARD_ASIC)
		return DEFAULT_TEMP * PERMILLAGE;

	if (di->coul_dev_ops && di->coul_dev_ops->get_bat_temp) {
		while (cnt++ < TEMP_RETRY_TIMES) {
			t_adc = di->coul_dev_ops->get_bat_temp();
			if (t_adc < 0) {
				coul_core_err("Bat temp read fail, retry_cnt = %d\n",
					cnt);
			} else {
				temperature = adc_to_temp_permille(t_adc);
				temperature =
					coul_battery_temperature_compensation(
						temperature);
				/* check validity of temperature, do temp fitting if not */
#ifdef CONFIG_HUAWEI_BATTERY_TEMP_FITTING
				temperature = btf_try_fit(temperature);
#endif
				return temperature;
			}
		}
		goto error;
	}

	while (cnt++ < TEMP_RETRY_TIMES) {
		t_adc = hisi_adc_get_adc(di->adc_batt_temp);
		if (t_adc < 0) {
			coul_core_err("Bat temp read fail!, retry_cnt = %d\n",
				cnt);
		} else {
			temperature = adc_to_temp_permille(t_adc);
			temperature = coul_battery_temperature_compensation(
				temperature);
			/* check validity of temperature, do temp fitting if not */
#ifdef CONFIG_HUAWEI_BATTERY_TEMP_FITTING
			temperature = btf_try_fit(temperature);
#endif
			return temperature;
		}
	}

error:
	coul_core_err("Bat temp read retry 3 times, error\n");
	if (user == USER_CHARGER)
		return TEMP_IPC_GET_ABNORMAL * PERMILLAGE;
	return DEFAULT_TEMP * PERMILLAGE;
}

int coul_battery_temperature_tenth_degree(
	enum battery_temp_user user)
{
	return coul_retry_temp_permille(user) / PERCENT;
}

/* BATTERY_TEMP_USER:0-coul;1-charger, Return the battery temperature */
static int coul_ntc_temperature_compensation(
	struct smartstar_coul_device *di, enum battery_temp_user user)
{
	int i, temp_without_compensation, temp_with_compensation;
	int ichg = 0;
	bool batt_temp_fitted = false;

	/*
	 * modify the temperature obtained by sampling,
	 * according to the temperature compensation value corresponding to the different current
	 */
	temp_without_compensation = coul_battery_temperature_tenth_degree(user);
	temp_with_compensation = temp_without_compensation;
#ifdef CONFIG_HUAWEI_BATTERY_TEMP_FITTING
	batt_temp_fitted = btf_temp_fitted_eqr();
#endif
	if ((di->ntc_compensation_is == 1) && (!batt_temp_fitted) &&
			(temp_without_compensation >= COMPENSATION_THRESHOLD)) {
		ichg = di->coul_dev_ops->get_battery_current_ua() / PERMILLAGE;
		ichg = abs(ichg);
		for (i = 0; i < COMPENSATION_PARA_LEVEL; i++) {
			if (ichg >= di->ntc_temp_compensation_para[i].ntc_compensation_ichg) {
				temp_with_compensation = temp_without_compensation -
					di->ntc_temp_compensation_para[i].ntc_compensation_value;
				break;
			}
		}
	}

	coul_core_info("coul_ntc_temperature_compensation: current = %d, temp_without_compensation = %d, temp_with_compensation = %d\n",
		ichg, temp_without_compensation, temp_with_compensation);
	return temp_with_compensation;
}

/*
 * the fun for adc get some err, we can avoid
 * BATTERY_TEMP_USER:0-coul;1-charger, return value battery temperature
 */
int get_temperature_stably(struct smartstar_coul_device *di,
	enum battery_temp_user user)
{
	int cnt = 0;
	int temperature = 0;
	int delta;

	if (di == NULL) {
		coul_core_err("error, di is NULL, return default temp\n");
		return DEFAULT_TEMP * TENTH;
	}

	while (cnt++ < STABLY_TEMP_RETRY_TIMES) {
		temperature = coul_ntc_temperature_compensation(di, user);
		delta = abs(di->batt_temp - temperature);
		if ((delta > DELTA_TEMP) ||
			(temperature > ABNORMAL_BATT_TEMPERATURE_POWEROFF) ||
			(temperature <= LOW_BATT_TEMP_CHECK_THRESHOLD))
			continue;
		coul_core_info("stably temp!,old_temp =%d, cnt =%d, temp = %d\n",
			di->batt_temp, cnt, temperature);
		return temperature;
	}
	return temperature;
}

void update_battery_temperature(struct smartstar_coul_device *di,
	int status)
{
	int temp;

	if (di == NULL) {
		coul_core_err("%s, di is NULL\n", __func__);
		return;
	}
	temp = get_temperature_stably(di, USER_COUL);
	if (status == TEMPERATURE_INIT_STATUS) {
		coul_core_info("init temp = %d\n", temp);
		di->batt_temp = temp;
	} else {
		if (temp - di->batt_temp > TEMPERATURE_CHANGE_LIMIT) {
			coul_core_err("temperature change too fast, pre = %d, current = %d\n",
				di->batt_temp, temp);
			di->batt_temp = di->batt_temp + TEMPERATURE_CHANGE_LIMIT;
		} else if (di->batt_temp - temp > TEMPERATURE_CHANGE_LIMIT) {
			coul_core_err("temperature change too fast, pre = %d, current = %d\n",
				di->batt_temp, temp);
			di->batt_temp = di->batt_temp - TEMPERATURE_CHANGE_LIMIT;
		} else if (di->batt_temp != temp) {
			coul_core_info("temperature changed, pre = %d, current = %d\n",
				di->batt_temp, temp);
			di->batt_temp = temp;
		}
	}
}

