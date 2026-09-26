/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: nv operation for coul module
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
#include "coul_nv.h"

#define COUL_NV_NAME "HICOUL"
#define COUL_NV_NUM  316
#define HW_COUL_NV_NAME   "HWCOUL"
#define HW_COUL_NV_NUM    392
#define NV_OPERATE_SUCC   0
#define NV_NOT_DEFINED    (-22)
#define PMU_NV_ADDR_CMDLINE_MAX_LEN 30
#define HEX               16
#define DECIMAL           10
#define INVALID_NV_FCC_VAL 100
#define BAT_CALIBRATION_NV_NAME    "BATCALI"
#define BAT_CALIBRATION_NV_NUM     317

enum nv_operation_type {
	NV_WRITE_TYPE = 0,
	NV_READ_TYPE,
};

struct coul_cali_params {
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
};

struct coul_cali_nv_info {
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
	int c_chip_temp;
};

static unsigned long g_nv_info_addr;
/*
 * 0x34A10000~0x34A11000 is reserved for pmu coulomb,
 * we use these to transfer coul information from fastboot to kernel, we add charger info
 */
static char *g_p_charger;
static struct hw_coul_nv_info g_batt_backup_nv_info = {0};
static int g_batt_backup_nv_read;
static struct ss_coul_nv_info g_my_nv_info;

static struct coul_cali_params g_coul_cali_params = {
	DEFAULT_V_OFF_A, 0, DEFAULT_C_OFF_A, 0
};

void get_coul_cali_params(int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b)
{
	if (v_offset_a)
		*v_offset_a = g_coul_cali_params.v_offset_a;
	if (v_offset_b)
		*v_offset_b = g_coul_cali_params.v_offset_b;
	if (c_offset_a)
		*c_offset_a = g_coul_cali_params.c_offset_a;
	if (c_offset_b)
		*c_offset_b = g_coul_cali_params.c_offset_b;
}

void set_coul_cali_params(int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b)
{
	if (v_offset_a && (*v_offset_a != 0))
		g_coul_cali_params.v_offset_a = *v_offset_a;
	if (v_offset_b)
		g_coul_cali_params.v_offset_b = *v_offset_b;
	if (c_offset_a && (*c_offset_a != 0))
		g_coul_cali_params.c_offset_a = *c_offset_a;
	if (c_offset_b)
		g_coul_cali_params.c_offset_b = *c_offset_b;
}

unsigned long get_nv_info_addr(void)
{
	return g_nv_info_addr;
}

struct hw_coul_nv_info get_batt_backup_nv_info(void)
{
	return g_batt_backup_nv_info;
}

static int __init early_parse_pmu_nv_addr_cmdline(char *p)
{
	char buf[PMU_NV_ADDR_CMDLINE_MAX_LEN + 1] = {0};
	unsigned int len;

	if (p == NULL) {
		coul_core_err("NULL point in [%s]\n", __func__);
		return -1;
	}
	len = strlen(p);
	if (len > PMU_NV_ADDR_CMDLINE_MAX_LEN) {
		coul_core_err("pmu_nv_addr_cmdline length out of range\n");
		return -1;
	}
	memcpy(buf, p, len + 1);

	if (kstrtoul(buf, HEX, &g_nv_info_addr) < 0)
		coul_core_err("%s kstrtoul failed\n", __func__);
	return 0;
}

static int __init move_pmu_nv_info(void)
{
	struct ss_coul_nv_info *pmu_nv_addr = NULL;

	if (g_nv_info_addr == 0)
		return 0;

	pmu_nv_addr = ioremap_wc(g_nv_info_addr, sizeof(*pmu_nv_addr));

	coul_core_info("pmu_nv_addr=0x%pK\n", pmu_nv_addr);
	if (pmu_nv_addr == NULL) {
		coul_core_err("nv add err,pmu_nv_addr=0x%pK\n", pmu_nv_addr);
		return 0;
	}
	memcpy(&g_my_nv_info, pmu_nv_addr, sizeof(g_my_nv_info));

	g_p_charger = (void *)pmu_nv_addr;
	return 0;
}

char *get_charger_info_p(void)
{
	return g_p_charger;
}
EXPORT_SYMBOL(get_charger_info_p);

early_param("pmu_nv_addr", early_parse_pmu_nv_addr_cmdline);
arch_initcall(move_pmu_nv_info);

static int get_fcc_temp_info(struct smartstar_coul_device *di,
	const struct ss_coul_nv_info *pinfo, struct single_row_lut *preal_fcc_lut)
{
	int i;

	for (i = 0; i < MAX_TEMPS; i++) {
		if (pinfo->real_fcc[i] == 0)
			break;

		if (pinfo->real_fcc[i] < INVALID_NV_FCC_VAL) {
			coul_core_info("real fcc in nv is not currect\n");
			return 0;
		}

		preal_fcc_lut->x[i] = pinfo->temp[i];
		preal_fcc_lut->y[i] = pinfo->real_fcc[i];
	}

	if (i == 0) {
		coul_core_info("no real fcc data in nv\n");
		return 0;
	}

	preal_fcc_lut->cols = i;
	di->adjusted_fcc_temp_lut = preal_fcc_lut;
	coul_core_info("temp:real_fcc %d:%d %d:%d %d:%d %d:%d %d:%d %d:%d %d:%d\n",
		pinfo->temp[0], pinfo->real_fcc[0],
		pinfo->temp[1], pinfo->real_fcc[1],
		pinfo->temp[2], pinfo->real_fcc[2],
		pinfo->temp[3], pinfo->real_fcc[3],
		pinfo->temp[4], pinfo->real_fcc[4],
		pinfo->temp[5], pinfo->real_fcc[5],
		pinfo->temp[6], pinfo->real_fcc[6]);
	return 0;
}

/* get NV info from fastboot send */
int get_initial_params(struct smartstar_coul_device *di)
{
	struct ss_coul_nv_info *pinfo = NULL;
	struct single_row_lut *preal_fcc_lut = NULL;
	int v_offset_a = DEFAULT_V_OFF_A;
	int v_offset_b = DEFAULT_V_OFF_B;
	int c_offset_a = DEFAULT_C_OFF_A;
	int c_offset_b = DEFAULT_C_OFF_B;

	if (di == NULL) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return -1;
	}

	get_coul_cali_params(&v_offset_a, &v_offset_b, &c_offset_a, &c_offset_b);
	coul_core_info("dts:get v_a=%d,v_b=%d,c_a=%d,c_b=%d,dts_c=%d\n",
		v_offset_a, v_offset_b, c_offset_a, c_offset_b,
		di->dts_c_offset_a);

	pinfo = &di->nv_info;
	preal_fcc_lut = &di->adjusted_fcc_temp_lut_tbl1;

	memcpy(&di->nv_info, &g_my_nv_info, sizeof(g_my_nv_info));

	di->batt_chargecycles = pinfo->charge_cycles;
	di->high_pre_qmax = pinfo->qmax;
	di->batt_limit_fcc = pinfo->limit_fcc;
	di->batt_report_full_fcc_real = pinfo->report_full_fcc_real;
	v_offset_a = (pinfo->v_offset_a == 0) ?
		DEFAULT_V_OFF_A : pinfo->v_offset_a;
	v_offset_b = (pinfo->v_offset_b == 0) ?
		DEFAULT_V_OFF_B : pinfo->v_offset_b;
	c_offset_a = di->dts_c_offset_a;
	if (c_offset_a) {
		c_offset_a = (pinfo->c_offset_a == 0) ?
			c_offset_a : pinfo->c_offset_a;
		c_offset_b = (pinfo->c_offset_b == 0) ?
			c_offset_b : pinfo->c_offset_b;
	} else {
		c_offset_a = (pinfo->c_offset_a == 0) ?
			DEFAULT_C_OFF_A : pinfo->c_offset_a;
		c_offset_b = (pinfo->c_offset_b == 0) ?
			DEFAULT_C_OFF_B : pinfo->c_offset_b;
	}

	coul_core_info("pl_v_a=%d,pl_v_b=%d,pl_c_a=%d,pl_c_b=%d,cycles=%d,limit_fcc=%d\n"
		"report_full_fcc_real =%d,reg_c=%d, reg_v=%d,batt_id=%d\n",
		pinfo->v_offset_a, pinfo->v_offset_b, pinfo->c_offset_a,
		pinfo->c_offset_b, pinfo->charge_cycles, pinfo->limit_fcc,
		pinfo->report_full_fcc_real, pinfo->calc_ocv_reg_c,
		pinfo->calc_ocv_reg_v, pinfo->hkadc_batt_id_voltage);

	coul_core_info("real use a/b value, v_offset_a=%d,v_offset_b=%d,c_offset_a=%d,c_offset_b=%d\n",
		v_offset_a, v_offset_b, c_offset_a, c_offset_b);
	set_coul_cali_params(&v_offset_a, &v_offset_b, &c_offset_a, &c_offset_b);
	return get_fcc_temp_info(di, pinfo, preal_fcc_lut);
}

static int hw_coul_operate_nv(struct hw_coul_nv_info *info,
	enum nv_operation_type type)
{
	int ret = -1;
	struct hisi_nve_info_user nve;

	if (info == NULL) {
		coul_core_err("[%s]info is NULL\n", __func__);
		return ret;
	}
	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, HW_COUL_NV_NAME, sizeof(HW_COUL_NV_NAME));
	nve.nv_number = HW_COUL_NV_NUM;
	nve.valid_size = sizeof(*info);
	if (nve.valid_size > NVE_NV_DATA_SIZE) {
		coul_core_err("[%s]struct info is too big for nve\n",
			__func__);
		return ret;
	}

	if (type == NV_WRITE_TYPE) {
		nve.nv_operation = NV_WRITE;
		memcpy(nve.nv_data, info, sizeof(*info));
		ret = hisi_nve_direct_access(&nve);
		if (ret)
			coul_core_err("[%s]write nv failed, ret = %d\n",
				__func__, ret);
	} else {
		nve.nv_operation = NV_READ;
		ret = hisi_nve_direct_access(&nve);
		if (ret)
			coul_core_err("[%s]read nv failed, ret = %d\n",
				__func__, ret);
		else
			memcpy(info, nve.nv_data, sizeof(*info));
	}
	return ret;
}

void hw_coul_get_nv(struct smartstar_coul_device *di)
{
	int ret;

	if (di == NULL)
		return;
	if (di->batt_backup_nv_flag && (g_batt_backup_nv_read == 0)) {
		ret = hw_coul_operate_nv(&g_batt_backup_nv_info, NV_READ_TYPE);
		if (ret == NV_NOT_DEFINED) {
			coul_core_err("battery backup nv not defined, disable battery backup nv flag\n");
			di->batt_backup_nv_flag = 0;
		} else if (ret == NV_OPERATE_SUCC) {
			coul_core_info("read battery backup nv info succ\n");
			g_batt_backup_nv_read = 1;
		}
	}
}

static void hw_coul_update_fcc(struct smartstar_coul_device *di,
	const struct ss_coul_nv_info *pinfo)
{
	int i;

	/* divide by 100 to get full chargecycles */
	if ((di->batt_chargecycles / PERCENT != 0) &&
		(!di->need_restore_cycle_flag ||
			(di->nv_info.change_battery_learn_flag == 0))) {
		for (i = 0; i < MAX_TEMPS; i++) {
			g_batt_backup_nv_info.temp[i] = pinfo->temp[i];
			g_batt_backup_nv_info.real_fcc[i] = pinfo->real_fcc[i];
		}
	}
}

static void hw_coul_save_nv(struct smartstar_coul_device *di,
	struct ss_coul_nv_info *pinfo)
{
	if (di->batt_backup_nv_flag && (g_batt_backup_nv_read == 1)) {
		/* update battery backup nv fcc */
		hw_coul_update_fcc(di, pinfo);
		if (hw_coul_operate_nv(&g_batt_backup_nv_info, NV_WRITE_TYPE) ==
			NV_OPERATE_SUCC)
			coul_core_info("write battery backup nv info succ\n");
	}
}

static void save_backup_nv_info(struct smartstar_coul_device *di,
	struct ss_coul_nv_info *pinfo)
{
	int i;
	int refresh_fcc_success = 1;

	if ((di == NULL) || (pinfo == NULL))
		return;
	pinfo->qmax = (short)di->high_pre_qmax;
	pinfo->charge_cycles = di->batt_chargecycles;
	pinfo->limit_fcc = di->batt_limit_fcc;
	pinfo->report_full_fcc_real = di->batt_report_full_fcc_real;

	if (di->adjusted_fcc_temp_lut != NULL) {
		for (i = 0; i < di->adjusted_fcc_temp_lut->cols; i++) {
			if (di->adjusted_fcc_temp_lut->y[i] < INVALID_NV_FCC_VAL) {
				refresh_fcc_success = 0;
				break;
			}
		}
		if (refresh_fcc_success) {
			for (i = 0; i < di->adjusted_fcc_temp_lut->cols; i++) {
				pinfo->temp[i] = di->adjusted_fcc_temp_lut->x[i];
				pinfo->real_fcc[i] =
					di->adjusted_fcc_temp_lut->y[i];
			}
		}
	} else {
		for (i = 0; i < MAX_TEMPS; i++) {
			pinfo->temp[i] = 0;
			pinfo->real_fcc[i] = 0;
		}
	}
	/* save battery backup nv info */
	hw_coul_save_nv(di, pinfo);
}

int save_nv_info(struct smartstar_coul_device *di)
{
	int ret;
	struct hisi_nve_info_user nve;
	struct ss_coul_nv_info *pinfo = NULL;

	if (di == NULL) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return -1;
	}

	pinfo = &di->nv_info;
	if (!di->is_nv_read) {
		/* udp do not print err log */
		if (di->is_board_type == BAT_BOARD_ASIC)
			coul_core_err("save nv before read, error\n");
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
		return -1;
	}

	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, COUL_NV_NAME, sizeof(COUL_NV_NAME));
	nve.nv_number = COUL_NV_NUM;
	nve.valid_size = sizeof(*pinfo);
	nve.nv_operation = NV_WRITE;

	save_backup_nv_info(di, pinfo);
	memcpy(nve.nv_data, pinfo, sizeof(*pinfo));

	/* here save info in register */
	ret = hisi_nve_direct_access(&nve);
	if (ret) {
		coul_core_info("save nv partion failed, ret=%d\n", ret);
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_FAIL);
	} else {
		di->coul_dev_ops->set_nv_save_flag(NV_SAVE_SUCCESS);
	}

	return ret;
}

void hw_coul_update_chargecycles(struct smartstar_coul_device *di)
{
	if (di == NULL)
		return;
	if (di->batt_backup_nv_flag && (g_batt_backup_nv_read == 1)) {
		g_batt_backup_nv_info.charge_cycles += di->batt_soc_real / TENTH -
			di->charging_begin_soc / TENTH;
		coul_core_info("battery backup chargecycle=%d, added=%d\n",
			g_batt_backup_nv_info.charge_cycles,
			di->batt_soc_real / TENTH -
				di->charging_begin_soc / TENTH);
	}
}

int save_cali_param(int cur_cal_temp)
{
	int ret = 0;
	int c_offset_a = DEFAULT_C_OFF_A;
	struct hisi_nve_info_user nve;
	struct coul_cali_nv_info *pinfo =
		(struct coul_cali_nv_info *)(&(nve.nv_data[0]));
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return -1;
	memset(&nve, 0, sizeof(nve));
	strncpy(nve.nv_name, BAT_CALIBRATION_NV_NAME,
		sizeof(BAT_CALIBRATION_NV_NAME));
	nve.nv_number = BAT_CALIBRATION_NV_NUM;
	nve.valid_size = sizeof(*pinfo);
	nve.nv_operation = NV_WRITE;
	get_coul_cali_params(&(pinfo->v_offset_a), &(pinfo->v_offset_b),
		&c_offset_a, NULL);
	if (di->dts_c_offset_a != c_offset_a) {
		get_coul_cali_params(NULL, NULL, &(pinfo->c_offset_a),
			&(pinfo->c_offset_b));
		pinfo->c_chip_temp = cur_cal_temp;
	}
#ifdef CONFIG_HISI_DEBUG_FS
	ret = hisi_nve_direct_access(&nve);
#endif
	if (ret)
		coul_core_err("save cali param failed, ret=%d\n", ret);
	else
		coul_core_info("save cali param success\n");
	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
static int g_basp_nv;

static bool basp_check_input_valid(const char *buffer)
{
	const char *c = NULL;

	coul_core_info(BASP_TAG"buffer:%s\n", buffer);

	c = buffer;
	while ((*c != '\n') && (*c != '\0')) {
		if (!(((*c >= '0') && (*c <= '9')) || (*c == ' '))) {
			coul_core_err(BASP_TAG"[%s], input invalid\n", __func__);
			return FALSE;
		}
		c++;
	}
	return TRUE;
}

static bool basp_nv_buff_ops(struct smartstar_coul_device *di,
	const char *buffer, int *need_save)
{
#define MAX_TMP_BUF_LEN 10
	int indx = 0;
	long val = 0;
	const char *begin = NULL;
	const char *end = NULL;
	struct ss_coul_nv_info *pinfo = NULL;
	char num[MAX_TMP_BUF_LEN] = {0};

	pinfo = &di->nv_info;
	begin = buffer;
	while ((*begin != '\0') && (*begin != '\n')) {
		while (*begin == ' ')
			begin++;
		end = begin;
		while ((*end != ' ') && (*end != '\0') && (*end != '\n'))
			end++;
		if (end - begin >= MAX_TMP_BUF_LEN) {
			coul_core_err(BASP_TAG"[%s], input too big\n", __func__);
			return FALSE;
		}
		memcpy(num, begin, (end - begin));
		if (kstrtol(num, DECIMAL, &val) < 0) {
			coul_core_err(BASP_TAG"[%s], num:%s, convert fail\n",
				__func__, num);
			break;
		}

		*need_save = 1;
		/* 0 ~ MAX_RECORDS_CNT-1 are fcc value  MAX_RECORDS_CNT is chargecycles */
		switch (indx) {
		case MAX_RECORDS_CNT:
			di->batt_chargecycles = val;
			break;
		case MAX_RECORDS_CNT + 1:
			indx = 0;
			break;
		default:
			pinfo->real_fcc_record[indx++] = val;
			break;
		}

		(void)memset_s(num, sizeof(num), 0, sizeof(num));
		begin = end;
	}
	return TRUE;
#undef MAX_TMP_BUF_LEN
}

static int basp_nv_set(const char *buffer, const struct kernel_param *kp)
{
	int ret = 0;
	int need_save = 0;
	const char *begin = NULL;
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL)
		return -EINVAL;

	if (basp_check_input_valid(buffer) == FALSE)
		goto func_end;

	begin = buffer;
	if (*begin == '\0') {
		coul_core_err(BASP_TAG"[%s], input empty\n", __func__);
		return 0;
	}

	if (basp_nv_buff_ops(di, buffer, &need_save) == FALSE)
		goto func_end;

	if (need_save)
		ret = save_nv_info(di);
	if (ret)
		coul_core_err("[%s]save_nv_info fail, ret=%d\n", __func__, ret);

func_end:
	return strlen(buffer);
}

static int basp_nv_get(char *buffer, const struct kernel_param *kp)
{
	int i, ret;
	unsigned int len = strlen(buffer);
	struct smartstar_coul_device *di = get_coul_dev();

	if (di == NULL) {
		coul_core_err(BASP_TAG"[%s], input param NULL\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		ret = snprintf_s(buffer + len, PAGE_SIZE - len,
			PAGE_SIZE - len - 1, "learned_fcc[%d]:%d\n",
			i, di->nv_info.real_fcc_record[i]);
		if (ret > 0)
			len = strlen(buffer);
	}

	ret = snprintf_s(buffer + len, PAGE_SIZE - len,
		PAGE_SIZE - len - 1, "chargecycles:%d\n",
		di->batt_chargecycles);
	if (ret > 0)
		len = strlen(buffer);
	ret = snprintf_s(buffer + len, PAGE_SIZE - len,
		PAGE_SIZE - len - 1, "latest_record_index:%d\n",
		di->nv_info.latest_record_index);
	if (ret > 0)
		len = strlen(buffer);
	return len;
}

static struct kernel_param_ops g_basp_nv_ops = {
	.set = basp_nv_set,
	.get = basp_nv_get,
};

module_param_cb(basp_nv, &g_basp_nv_ops, &g_basp_nv, 0644);
#endif
