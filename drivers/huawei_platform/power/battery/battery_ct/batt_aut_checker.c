/*
 * batt_aut_checker.c
 *
 * battery authentication checker
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

#include "batt_aut_checker.h"
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_checker
HWLOG_REGIST();

/* this list contain all possible IC for all battery checkers */
LIST_HEAD(batt_aut_ic_head);
void add_to_aut_ic_list(struct batt_ct_ops_list *entry)
{
	list_add_tail(&entry->node, &batt_aut_ic_head);
}

static enum batt_ic_type get_ic_type_wrapper(struct batt_chk_data *drv_data)
{
	enum batt_ic_type type = LOCAL_IC_TYPE;

	if (!drv_data)
		return type;
	if (drv_data->ic_ops.get_ic_type) {
		bat_type_apply_mode(BAT_ID_SN);
		type = drv_data->ic_ops.get_ic_type();
		bat_type_release_mode(false);
	}

	return type;
}

static int get_ic_uuid_wrapper(struct batt_chk_data *drv_data,
	const unsigned char **uuid, unsigned int *uuid_len)
{
	int ret = -1;

	if (!drv_data)
		return ret;
	if (drv_data->ic_ops.get_ic_uuid) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_ic_uuid(drv_data->ic,
			uuid, uuid_len);
		bat_type_release_mode(false);
	}

	return ret;
}

static int get_batt_type_wrapper(struct batt_chk_data *drv_data,
	const unsigned char **type, unsigned int *type_len)
{
	int ret = -1;

	if (!drv_data || !type || !type_len)
		return ret;
	if (drv_data->ic_ops.get_batt_type) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_batt_type(drv_data->ic, type,
			type_len);
		bat_type_release_mode(false);
	}

	return ret;
}

static int get_batt_sn_wrapper(struct batt_chk_data *drv_data,
	struct power_genl_attr *res, const unsigned char **sn,
	unsigned int *sn_len_bits)
{
	int ret = -1;

	/*
	 * res is NULL allowed for some ic
	 * sn & sn_len_bits is NULL allowed for all ics
	 */
	if (!drv_data)
		return ret;
	if (drv_data->ic_ops.get_batt_sn) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_batt_sn(drv_data->ic, res, sn,
			sn_len_bits);
		bat_type_release_mode(false);
	}

	return ret;
}

static int prepare_wrapper(struct batt_chk_data *drv_data, enum res_type type,
	struct power_genl_attr *res)
{
	int ret = -1;

	if (!drv_data || !res)
		return -1;
	if (drv_data->ic_ops.prepare) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.prepare(drv_data->ic, type,
			res);
		bat_type_release_mode(false);
	}

	return ret;
}

static int certification_wrapper(struct batt_chk_data *drv_data,
	struct power_genl_attr *res, enum key_cr *result)
{
	int ret = -1;

	if (!drv_data || !result) /* res is NULL allowed for some ic */
		return ret;
	if (drv_data->ic_ops.certification) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.certification(drv_data->ic, res, result);
		bat_type_release_mode(false);
	}

	return ret;
}

static int set_act_signature_wrapper(struct batt_chk_data *drv_data,
	enum res_type type, const struct power_genl_attr *res)
{
	int ret = -1;

	if (!drv_data || !res)
		return -1;
	if (drv_data->ic_ops.set_act_signature) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.set_act_signature(drv_data->ic,
			type, res);
		bat_type_release_mode(false);
	}

	return ret;
}

static int set_batt_safe_info_wrapper(struct batt_chk_data *drv_data,
	enum batt_safe_info_t type, void *value)
{
	int ret = -1;

	if (!drv_data) /* value is NULL allowed */
		return ret;
	if (drv_data->ic_ops.set_batt_safe_info) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.set_batt_safe_info(drv_data->ic, type,
			value);
		bat_type_release_mode(false);
	}

	return ret;
}

static int get_batt_safe_info_wrapper(struct batt_chk_data *drv_data,
	enum batt_safe_info_t type, void *value)
{
	int ret = -1;

	if (!drv_data || !value)
		return ret;
	if (drv_data->ic_ops.get_batt_safe_info) {
		bat_type_apply_mode(BAT_ID_SN);
		ret = drv_data->ic_ops.get_batt_safe_info(drv_data->ic, type,
			value);
		bat_type_release_mode(false);
	}

	return ret;
}

static void power_down_wrapper(struct batt_chk_data *drv_data)
{
	if (!drv_data)
		return;
	if (drv_data->ic_ops.power_down) {
		bat_type_apply_mode(BAT_ID_SN);
		drv_data->ic_ops.power_down(drv_data->ic);
		bat_type_release_mode(false);
	}
}

static void check_key_nop(struct work_struct *work)
{
}

static void check_sn_nop(struct work_struct *work)
{
}

static const work_func_t check_key_funcs[] = {
	[UTIL_LOCAL_TYPE] = check_key_nop,
	[UTIL_BSC_TYPE] = bsc_key_func,
	[UTIL_BYC_TYPE] = byc_key_func,
};

static const work_func_t check_sn_funcs[] = {
	[UTIL_LOCAL_TYPE] = check_sn_nop,
	[UTIL_BSC_TYPE] = bsc_sn_func,
	[UTIL_BYC_TYPE] = byc_sn_func,
};

static const work_func_t act_signature_funcs[] = {
	[UTIL_LOCAL_TYPE] = check_key_nop,
	[UTIL_BSC_TYPE] = check_key_nop,
	[UTIL_BYC_TYPE] = byc_act_sign_func,
};

enum batt_aut_util_type checker_util_type(enum batt_ic_type ic_type)
{
	if (ic_type < IMP_VER20_IC_TYPE_START)
		return UTIL_BSC_TYPE;

	return UTIL_BYC_TYPE;
}

static const char * const ic_type2name[] = {
	[LOCAL_IC_TYPE] = "INVALID_DEFAULT_IC",
	[MAXIM_DS28EL15_TYPE] = "DS28EL15",
	[MAXIM_DS28EL16_TYPE] = "DS28EL16",
	[NXP_A1007_TYPE] = "A1007",
	[RICHTEK_A1_TYPE] = "RICHTEK_A1",
	[INFINEON_SLE95250_TYPE] = "INFINEON_SLE95250",
};

static ssize_t ic_type_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int type;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	type = drv_data->bco.get_ic_type(drv_data);
	if ((type < ARRAY_SIZE(ic_type2name)) && (type >= 0))
		return snprintf(buf, PAGE_SIZE, "%s", ic_type2name[type]);
	else
		return snprintf(buf, PAGE_SIZE, "ERROR_TYPE");
}

static ssize_t ic_uuid_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int ret;
	int i;
	int count = 0;
	unsigned int uuid_len;
	const unsigned char *uuid = NULL;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data || !drv_data->bco.get_ic_uuid)
		return snprintf(buf, PAGE_SIZE, "error data");

	ret = drv_data->bco.get_ic_uuid(drv_data, &uuid, &uuid_len);
	if (ret || !uuid || (uuid_len == 0))
		return snprintf(buf, PAGE_SIZE, "error:can't get uuid from ic");

	for (i = 0; i < uuid_len; i++) {
		count += snprintf(buf + count, PAGE_SIZE - count, "%02X",
			uuid[i]);
		/* 3 = sizeof("xx") */
		if ((count + 3) > PAGE_SIZE)
			return -1;
	}
	return uuid_len;
}

static ssize_t batt_type_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int i;
	int total_size = 0;
	const unsigned char *type = NULL;
	unsigned int type_len;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	if (drv_data->bco.get_batt_type(drv_data, &type, &type_len)) {
		hwlog_err("Get ic id failed in %s\n", __func__);
		return snprintf(buf, PAGE_SIZE, "ERROR:Can't get type from IC");
	}

	for (i = 0; i < type_len; i++)
		total_size += snprintf(&(buf[total_size]),
			BATT_ID_PRINT_SIZE_PER_CHAR, "%02x", type[i]);

	return total_size;
}

static ssize_t batt_id_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	const unsigned char *sn = NULL;
	unsigned int sn_len;
	int ret;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");
	ret = drv_data->bco.get_batt_sn(drv_data, &drv_data->sn_res, &sn,
		&sn_len);
	if (ret) {
		hwlog_err("Get battery SN failed in %s\n", __func__);
		return snprintf(buf, PAGE_SIZE, "ERROR:Can't get sn from IC");
	}
	memcpy(buf, sn, sn_len);
	buf[sn_len] = 0;

	return sn_len;
}

static ssize_t batt_source_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	enum batt_source temp;
	const char *str_type = NULL;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "error data");

	ret = drv_data->bco.get_batt_safe_info(drv_data,
		BATT_SPARE_PART, &temp);
	if (ret)
		return snprintf(buf, PAGE_SIZE,
			"error can't get source type from ic");

	if (temp == BATTERY_ORIGINAL)
		str_type = "original";
	else if (temp == BATTERY_SPARE_PART)
		str_type = "spare";
	else
		str_type = "unknown";

	return snprintf(buf, PAGE_SIZE, "%s", str_type);
}

static ssize_t batt_source_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	enum batt_source temp = BATTERY_ORIGINAL;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data  || !sysfs_streq(buf, "original"))
		return -EINVAL;
	if (drv_data->batt_src_current == BATTERY_ORIGINAL)
		return count;

	if (drv_data->bco.set_batt_safe_info(drv_data, BATT_SPARE_PART, &temp))
		return -EINVAL;

	drv_data->batt_src_current == BATTERY_SPARE_PART;
	return count;
}

static ssize_t ic_status_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.ic_status);
}

static ssize_t key_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.key_status);
}

static ssize_t sn_status_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.sn_status);
}

static ssize_t check_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->result.check_mode);
}

static int prepare_signature(struct batt_chk_data *drv_data,
	const char *key_id, char *buf)
{
	int i;
	int count;
	struct power_genl_attr res;

	if (drv_data->bco.prepare(drv_data, RES_CT, &res) ||
		(!res.data) || (res.len == 0)) {
		hwlog_err("prepare ecc signature failed\n");
		return -EINVAL;
	}

	hwlog_info("read act signature length is %d\n", res.len);
	count = snprintf(buf, PAGE_SIZE, "%s,", key_id);
	for (i = 0; i < res.len; i++) {
		count += snprintf(buf + count, PAGE_SIZE - count, "%02x",
			res.data[i]);
		/* length of "xx" is 3 */
		if ((count + 3) > PAGE_SIZE)
			return -EINVAL;
	}

	return count;
}

static ssize_t act_signature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data || !drv_data->bco.prepare)
		return snprintf(buf, PAGE_SIZE, "error data");

	return prepare_signature(drv_data, BATT_ACT_SIGN_ID, buf);
}

static int verrify_act_signature(struct batt_chk_data *drv_data,
	const char *buf, size_t count)
{
	int i;
	int bytes;
	struct power_genl_attr res;
	char data[BATT_MAX_SIGN_LEN] = { 0 };
	char temp[BATT_HEX_STR_BYTE] = { 0 };

	if (!count) {
		hwlog_err("count 0\n");
		return -EINVAL;
	}
	bytes = (buf[count - 1] == '\n') ? (count - 1) : count;
	/* 2 = even number */
	if ((bytes % 2) != 0)
		return -EINVAL;
	bytes = bytes / 2;
	if ((bytes + 1) >= BATT_MAX_SIGN_LEN)
		return -EINVAL;

	data[0] = bytes;
	for (i = 1; i < bytes + 1; i++) {
		temp[0] = buf[(i - 1) * 2];
		temp[1] = buf[(i - 1) * 2 + 1];
		temp[2] = '\0';
		/* 16 = hexadecimal */
		if (kstrtou8(temp, 16, data + i)) {
			hwlog_err("%s kstrtou8 failed\n", temp);
			return -EINVAL;
		}
	}

	res.data = data;
	res.len = bytes + 1;
	if (drv_data->bco.set_act_signature(drv_data, RES_ACT, &res)) {
		hwlog_err("set act signature to ic failed\n");
		return -EINVAL;
	}

	schedule_work(&drv_data->act_signature_work);
	flush_work(&drv_data->act_signature_work);
	return 0;
}

#ifdef BATTBD_FORCE_MATCH
static int set_cert_ready(struct batt_chk_data *drv_data)
{
	enum batt_cert_state temp = CERT_READY;

	if (drv_data->bco.set_batt_safe_info(drv_data, BATT_CERT_READY,
		&temp)) {
		hwlog_err("write cert ready state to ic failed\n");
		return -1;
	}
	return 0;
}
#endif

static ssize_t act_signature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data || !drv_data->bco.set_act_signature)
		return -EINVAL;

	hwlog_info("set act %d signature\n", count);
	if (verrify_act_signature(drv_data, buf, count) ||
		(drv_data->act_state != ACT_PASS)) {
		hwlog_err("verify act signature failed %d\n",
			drv_data->act_state);
		return -EINVAL;
	}

#ifdef BATTBD_FORCE_MATCH
	if (set_cert_ready(drv_data))
		return -EINVAL;
#endif
	return count;
}

static ssize_t retrofit_signature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data || !drv_data->bco.prepare)
		return snprintf(buf, PAGE_SIZE, "error data");

	return prepare_signature(drv_data, BATT_RETROFIT_SIGN_ID, buf);
}

static ssize_t retrofit_signature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data || !drv_data->bco.set_act_signature)
		return -EINVAL;

	hwlog_info("set retrofit %d signature\n", count);
	if (verrify_act_signature(drv_data, buf, count) ||
		(drv_data->act_state != RETROFIT_PASS)) {
		hwlog_err("verify retrofit signature fail %d\n",
			drv_data->act_state);
		return -EINVAL;
	}

#ifdef BATTBD_FORCE_MATCH
	if (set_cert_ready(drv_data))
		return -EINVAL;
#endif
	return count;
}

#ifdef BATTERY_LIMIT_DEBUG
static ssize_t rematchable_onboot_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d", drv_data->batt_rematch_onboot);
}

static ssize_t rematchable_current_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	enum batt_match_type temp;
	enum batt_aut_util_type type;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	type = checker_util_type(drv_data->ic_type);
	if (type == UTIL_BYC_TYPE) {
		temp = drv_data->batt_rematch_current;
	} else {
		ret = drv_data->bco.get_batt_safe_info(drv_data,
			BATT_MATCH_ABILITY, &temp);
		if (ret)
			return snprintf(buf, PAGE_SIZE, "%s", "Error");
	}

	return snprintf(buf, PAGE_SIZE, "%s",
		(temp == BATTERY_REMATCHABLE) ? "YES" : "NO");
}

static ssize_t batt_src_onboot_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	const char *str_type = NULL;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "error data");

	if (drv_data->batt_src_onboot == BATTERY_ORIGINAL)
		str_type = "original";
	else if (drv_data->batt_src_onboot == BATTERY_SPARE_PART)
		str_type = "spare";
	else
		str_type = "unknown";

	return snprintf(buf, PAGE_SIZE, "%s", str_type);
}

static ssize_t batt_cycles_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "error data");

	return snprintf(buf, PAGE_SIZE, "coul cycles:%d, ic cycles %d",
		drv_data->coul_cycles, drv_data->ic_cycles);
}

static ssize_t batt_act_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	const char *str_state = NULL;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "error data");

	if (drv_data->act_state == SIGN_FAIL)
		str_state = "fail";
	else if (drv_data->act_state == ACT_PASS)
		str_state = "act";
	else
		str_state = "retrofit";

	return snprintf(buf, PAGE_SIZE, "%s", str_state);
}

static ssize_t lock_battery_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	enum batt_match_type temp = BATTERY_UNREMATCHABLE;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	if (count < 1 || buf[0] != '1')
		return -1;
	ret = drv_data->bco.set_batt_safe_info(drv_data, BATT_MATCH_ABILITY,
		&temp);
	if (ret)
		return -1;

	return count;
}

static ssize_t free_cycles_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%u", drv_data->free_cycles);
}

static ssize_t free_cycles_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;

	if (!count) {
		hwlog_err("count 0\n");
		return -EINVAL;
	}

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	if ((buf[count - 1] == '\n') &&
		/* 10:buf size */
		!kstrtouint(buf, 10, &drv_data->free_cycles))
		return -1;

	return count;
}

static ssize_t check_result_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%d %d %d", drv_data->result.ic_status,
		drv_data->result.key_status, drv_data->result.sn_status);
}

#define CHECK_RESULT_STR_BUF_SIZE 64

static ssize_t check_result_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char *sub = NULL;
	char *cur = NULL;
	int temp0, temp1, temp2;
	char str[CHECK_RESULT_STR_BUF_SIZE] = { 0 };
	size_t len;
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	len = min_t(size_t, sizeof(str) - 1, count);
	memcpy(str, buf, len);
	cur = &str[0];
	sub = strsep(&cur, " ");
	if (!sub || kstrtoint(sub, 0, &temp0))
		return -1;
	sub = strsep(&cur, " ");
	if (!sub || kstrtoint(sub, 0, &temp1))
		return -1;
	if (!cur || kstrtoint(cur, 0, &temp2))
		return -1;
	drv_data->result.ic_status = temp0;
	drv_data->result.key_status = temp1;
	drv_data->result.sn_status = temp2;

	return count;
}

static ssize_t check_key_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%pf", drv_data->check_key_work.func);
}

static ssize_t check_key_type_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;
	int temp;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	/* 5:buf size   4:type length */
	if ((count >= 5) && !memcmp(buf, "type", 4)) {
		temp = buf[4] - '0';
		if ((temp >= 0) && (temp < ARRAY_SIZE(check_key_funcs)))
			drv_data->check_key_work.func = check_key_funcs[temp];
	}

	return count;
}

static ssize_t check_sn_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "Error data");

	return snprintf(buf, PAGE_SIZE, "%pf", drv_data->check_sn_work.func);
}

static ssize_t check_sn_type_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;
	int temp;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	/* 5:buf size   4:type length */
	if ((count >= 5) && !memcmp(buf, "type", 4)) {
		temp = buf[4] - '0';
		if ((temp >= 0) && (temp < ARRAY_SIZE(check_sn_funcs)))
			drv_data->check_sn_work.func = check_sn_funcs[temp];
	}

	return count;
}

static ssize_t check_key_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	/* 5:buf size */
	if ((count >= 5) && !memcmp(buf, "check", 5))
		schedule_work(&drv_data->check_key_work);

	return count;
}

static ssize_t check_sn_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct batt_chk_data *drv_data = NULL;

	dev_get_drv_data(drv_data, dev);
	if (!drv_data)
		return -1;
	/* 5:buf size */
	if ((count >= 5) && !memcmp(buf, "check", 5))
		schedule_work(&drv_data->check_sn_work);

	return count;
}
#endif /* BATTERY_LIMIT_DEBUG */

static const DEVICE_ATTR_RO(ic_type);
static const DEVICE_ATTR_RO(ic_uuid);
static const DEVICE_ATTR_RO(batt_type);
static const DEVICE_ATTR_RO(ic_status);
static const DEVICE_ATTR_RO(key_status);
static const DEVICE_ATTR_RO(sn_status);
static const DEVICE_ATTR_RO(batt_id);
static const DEVICE_ATTR_RW(batt_source);
static const DEVICE_ATTR_RO(check_mode);
static const DEVICE_ATTR_RW(act_signature);
static const DEVICE_ATTR_RW(retrofit_signature);
#ifdef BATTERY_LIMIT_DEBUG
static const DEVICE_ATTR_RO(rematchable_onboot);
static const DEVICE_ATTR_RO(rematchable_current);
static const DEVICE_ATTR_RO(batt_src_onboot);
static const DEVICE_ATTR_RO(batt_cycles);
static const DEVICE_ATTR_RO(batt_act_state);
static const DEVICE_ATTR_WO(lock_battery);
static const DEVICE_ATTR_RW(free_cycles);
static const DEVICE_ATTR_RW(check_result);
static const DEVICE_ATTR_RW(check_key_type);
static const DEVICE_ATTR_RW(check_sn_type);
static const DEVICE_ATTR_WO(check_key);
static const DEVICE_ATTR_WO(check_sn);
#endif /* BATTERY_LIMIT_DEBUG */

static const struct attribute *batt_checker_attrs[] = {
	&dev_attr_ic_type.attr,
	&dev_attr_ic_uuid.attr,
	&dev_attr_batt_type.attr,
	&dev_attr_ic_status.attr,
	&dev_attr_key_status.attr,
	&dev_attr_sn_status.attr,
	&dev_attr_batt_id.attr,
	&dev_attr_batt_source.attr,
	&dev_attr_check_mode.attr,
	&dev_attr_act_signature.attr,
	&dev_attr_retrofit_signature.attr,
#ifdef BATTERY_LIMIT_DEBUG
	&dev_attr_rematchable_onboot.attr,
	&dev_attr_rematchable_current.attr,
	&dev_attr_batt_src_onboot.attr,
	&dev_attr_batt_cycles.attr,
	&dev_attr_batt_act_state.attr,
	&dev_attr_lock_battery.attr,
	&dev_attr_free_cycles.attr,
	&dev_attr_check_result.attr,
	&dev_attr_check_key_type.attr,
	&dev_attr_check_sn_type.attr,
	&dev_attr_check_key.attr,
	&dev_attr_check_sn.attr,
#endif /* BATTERY_LIMIT_DEBUG */
	NULL, /* sysfs_create_files need last one be NULL */
};

/* create all node needed by driver */
static int batt_checker_node_create(struct platform_device *pdev)
{
	if (sysfs_create_files(&pdev->dev.kobj, batt_checker_attrs)) {
		hwlog_err("Can't create all expected nodes under %s in %s\n",
			pdev->dev.kobj.name, __func__);
		return BATTERY_DRIVER_FAIL;
	}

	return BATTERY_DRIVER_SUCCESS;
}

static int check_batt_ct_ops(struct batt_chk_data *drv_data)
{
	int ret = BATTERY_DRIVER_SUCCESS;

	if (!drv_data->ic_ops.get_batt_type) {
		ret = BATTERY_DRIVER_FAIL;
		hwlog_err("get_batt_type is not valid\n");
	}
	if (!drv_data->ic_ops.certification) {
		ret = BATTERY_DRIVER_FAIL;
		hwlog_err("certification is not valid\n");
	}
	if (!drv_data->ic_ops.get_batt_sn) {
		ret = BATTERY_DRIVER_FAIL;
		hwlog_err("get_batt_sn is not valid\n");
	}
	if (!drv_data->ic_ops.prepare) {
		ret = BATTERY_DRIVER_FAIL;
		hwlog_err("ct_prepare is not valid\n");
	}
	if (!drv_data->ic_ops.set_batt_safe_info) {
		ret = BATTERY_DRIVER_FAIL;
		hwlog_err("set_batt_safe_info is not valid\n");
	}
	if (!drv_data->ic_ops.get_batt_safe_info) {
		ret = BATTERY_DRIVER_FAIL;
		hwlog_err("get_batt_safe_info is not valid\n");
	}

	return ret;
}

static void init_wrapper_ops(struct batt_chk_data *drv_data)
{
	drv_data->bco.get_ic_type = get_ic_type_wrapper;
	drv_data->bco.get_ic_uuid = get_ic_uuid_wrapper;
	drv_data->bco.get_batt_type = get_batt_type_wrapper;
	drv_data->bco.get_batt_sn = get_batt_sn_wrapper;
	drv_data->bco.prepare = prepare_wrapper;
	drv_data->bco.certification = certification_wrapper;
	drv_data->bco.set_act_signature = set_act_signature_wrapper;
	drv_data->bco.set_batt_safe_info = set_batt_safe_info_wrapper;
	drv_data->bco.get_batt_safe_info = get_batt_safe_info_wrapper;
	drv_data->bco.power_down = power_down_wrapper;
}

static int get_ic_ops(struct device_node *node, struct batt_chk_data *drv_data)
{
	int i, j, ret;
	int ic_unready = -1;
	struct batt_ct_ops_list *pos = NULL;
	struct batt_ct_ops_list *pos_temp = NULL;
	phandle *dev_ic_phandles = NULL;
	phandle ic_phandle;
	int num;

	num = of_property_count_elems_of_size(node, "matchable",
		sizeof(phandle));
	if (num <= 0) {
		hwlog_err("No ic matchable for checker %s\n", node->name);
		return ic_unready;
	}
	dev_ic_phandles = kcalloc((size_t)num, sizeof(phandle), GFP_KERNEL);
	if (!dev_ic_phandles)
		return ic_unready;
	ret = of_property_read_u32_array(node, "matchable", dev_ic_phandles,
		num);
	if (ret) {
		hwlog_err("%s read dev_ic_phandles failed %d\n", node->name,
			ret);
		goto free_dev_ic_phandles;
	}

	/* 5: loop 5 to find ic first */
	for (i = 0; i < 5; i++) {
		list_for_each_entry(pos, &batt_aut_ic_head, node) {
			if (pos->ic_dev)
				ic_phandle = pos->ic_dev->dev.of_node->phandle;
			else
				continue;
			for (j = 0; j < num; j++) {
				if (ic_phandle == dev_ic_phandles[j])
					break;
			}
			/* if this node is not for this batt_info */
			if (j == num)
				continue;
			if (pos->ct_ops_register) {
				bat_type_apply_mode(BAT_ID_SN);
				ic_unready = pos->ct_ops_register(pos->ic_dev,
					&drv_data->ic_ops);
				bat_type_release_mode(false);
			}
			/* if node for this batt_info found */
			if (!ic_unready) {
				hwlog_info("%s find valid checker ic\n",
					node->name);
				drv_data->ic = pos->ic_dev;
				drv_data->ic_type =
					drv_data->bco.get_ic_type(drv_data);
				break;
			}
		}
		if (!ic_unready) {
			list_del_init(&pos->node);
			break;
		}
	}
	/* release useless memory allocated for ic */
	list_for_each_entry_safe(pos, pos_temp, &batt_aut_ic_head, node) {
		if (pos->ic_dev)
			ic_phandle = pos->ic_dev->dev.of_node->phandle;
		else
			continue;
		for (j = 0; j < num; j++) {
			if (ic_phandle == dev_ic_phandles[j] &&
				pos->ic_memory_release) {
				list_del_init(&pos->node);
				pos->ic_memory_release(pos->ic_dev);
				continue;
			}
		}
	}

	if (ic_unready) {
		drv_data->result.ic_status = IC_FAIL_UNKOWN;
		hwlog_info("%s battery can't find ic\n", node->name);
		goto free_dev_ic_phandles;
	}

	/* check battery battery information interface */
	if (check_batt_ct_ops(drv_data)) {
		drv_data->result.ic_status = IC_FAIL_UNKOWN;
		hwlog_err("%s did not have enough operations/n", node->name);
		goto free_dev_ic_phandles;
	}

	drv_data->result.ic_status = IC_PASS;

free_dev_ic_phandles:
	kfree(dev_ic_phandles);
	return ic_unready;
}

static int battery_cycles_limit_init(struct platform_device *pdev)
{
	struct batt_chk_data *drv_data = NULL;
	enum batt_aut_util_type type;

	drv_data = platform_get_drvdata(pdev);
	if (!drv_data) {
		hwlog_err("%s can't find valid platform device drvdata\n",
			pdev->dev.of_node->name);
		return BATTERY_DRIVER_FAIL;
	}

	/* battery charge cycles limitation */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG),
		pdev->dev.of_node, "free-cycles", &drv_data->free_cycles,
		BATT_FREE_LIMIT_CYC);

	/* battery charge cycles limitation for spare */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG),
		pdev->dev.of_node, "spare-cycles", &drv_data->spare_cycles,
		BATT_SPARE_LIMIT_CYC);

	type = checker_util_type(drv_data->ic_type);
	if (type == UTIL_BYC_TYPE)
		return byc_register_cycles_listener(drv_data);

	return bsc_register_cycles_listener(drv_data);
}

void init_battery_check_result(struct batt_chk_rslt *result)
{
	if (!result)
		return;
	/* initial battery check result */
	result->ic_status = IC_FAIL_UNKOWN;
	result->key_status = KEY_UNREADY;
	result->sn_status = SN_UNREADY;
#ifdef BATTBD_FORCE_MATCH
	result->check_mode = FACTORY_CHECK_MODE;
#else
	result->check_mode = COMMERCIAL_CHECK_MODE;
#endif /* BATTBD_FORCE_MATCH */
}

static int battery_work_init(enum batt_aut_util_type type,
	struct work_struct *work,
	const work_func_t *funcs,
	int num)
{
	if (type < num) {
		INIT_WORK(work, funcs[type]);
		return 0;
	}

	hwlog_err("ic type %d for battery work  %d error\n", type, num);
	return -1;
}

static int battery_all_work_init(struct batt_chk_data *drv_data)
{
	int ret = 0;
	enum batt_aut_util_type type = checker_util_type(drv_data->ic_type);

	ret += battery_work_init(type, &drv_data->check_key_work,
		check_key_funcs, ARRAY_SIZE(check_key_funcs));
	ret += battery_work_init(type, &drv_data->check_sn_work,
		check_sn_funcs, ARRAY_SIZE(check_sn_funcs));
	ret += battery_work_init(type, &drv_data->act_signature_work,
		act_signature_funcs, ARRAY_SIZE(act_signature_funcs));

	return ret;
}

void battery_release_chk_data(struct platform_device *pdev)
{
	struct batt_chk_data *drv_data = NULL;

	drv_data = platform_get_drvdata(pdev);
	if (!drv_data)
		return;

	kfree(drv_data->key_res.data);
	drv_data->key_res.data = NULL;
	drv_data->key_res.len = 0;
}

static struct batt_chk_data *batt_chk_data_init(struct platform_device *pdev)
{
	struct batt_chk_data *drv_data = NULL;
	int ret;

	/* set up device's driver data now */
	drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data)
		return NULL;
	platform_set_drvdata(pdev, drv_data);

	ret = of_property_read_u8(pdev->dev.of_node, "id-in-grp",
		&drv_data->id_in_grp);
	if (ret) {
		hwlog_err("%s read grp_no failed\n", pdev->dev.of_node->name);
		return NULL;
	}
	ret = of_property_read_u8(pdev->dev.of_node, "chks-in-grp",
		&drv_data->chks_in_grp);
	if (ret) {
		hwlog_err("%s read grp_total failed\n",
			pdev->dev.of_node->name);
		return NULL;
	}
	ret = of_property_read_u8(pdev->dev.of_node, "id-of-grp",
		&drv_data->id_of_grp);
	if (ret) {
		hwlog_err("%s read grp_id failed\n", pdev->dev.of_node->name);
		return NULL;
	}

	init_completion(&drv_data->key_prepare_ready);
	init_completion(&drv_data->sn_prepare_ready);
	init_completion(&drv_data->act_sign_complete);
	INIT_LIST_HEAD(&drv_data->entry.node);
	drv_data->entry.pdev = pdev;
	drv_data->release = battery_release_chk_data;
	init_battery_check_result(&drv_data->result);

	return drv_data;
}

static int battery_aut_checker_probe(struct platform_device *pdev)
{
	int local_ic;
	struct batt_chk_data *drv_data = NULL;

	if (!pdev)
		return BATTERY_DRIVER_FAIL;
	hwlog_info("%s is going to probing\n", pdev->name);
	drv_data = batt_chk_data_init(pdev);
	if (!drv_data) {
		hwlog_err("battery checker(%s) probe failed\n", pdev->name);
		return BATTERY_DRIVER_FAIL;
	}
	init_wrapper_ops(drv_data);
	/* get ic ops for battery type, new or old */
	local_ic = get_ic_ops(pdev->dev.of_node, drv_data);
	if (local_ic) {
		hwlog_err("%s get ic ops failed\n", pdev->name);
		return BATTERY_DRIVER_FAIL;
	}
	if (battery_all_work_init(drv_data))
		return BATTERY_DRIVER_FAIL;

	/* get battery cycles which distingush the old from the new */
	if (battery_cycles_limit_init(pdev)) {
		hwlog_err("%s set cycles limit failed\n", pdev->name);
		return BATTERY_DRIVER_FAIL;
	}
	/* battery node initialization */
	if (batt_checker_node_create(pdev)) {
		hwlog_err("%s create nodes failed\n", pdev->name);
		return BATTERY_DRIVER_FAIL;
	}
	add_to_batt_checkers_lists(&drv_data->entry);
	hwlog_info("%s probed successfully\n", pdev->name);

	return BATTERY_DRIVER_SUCCESS;
}

static int battery_aut_checker_remove(struct platform_device *pdev)
{
	return BATTERY_DRIVER_SUCCESS;
}

static const struct of_device_id battery_checkers_match_table[] = {
	{ .compatible = "huawei,battery-authentication", },
	{ /* end */ },
};

const struct of_device_id *get_battery_checkers_match_table(void)
{
	return battery_checkers_match_table;
}

static struct platform_driver battery_checkers_driver = {
	.probe = battery_aut_checker_probe,
	.remove = battery_aut_checker_remove,
	.driver = {
		.name = "Battery_Checker",
		.owner = THIS_MODULE,
		.of_match_table = battery_checkers_match_table,
	},
};

int __init battery_checkers_init(void)
{
	return platform_driver_register(&battery_checkers_driver);
}

void __exit battery_checkers_exit(void)
{
	platform_driver_unregister(&battery_checkers_driver);
}

subsys_initcall_sync(battery_checkers_init);
module_exit(battery_checkers_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("checker for battery authentication ic");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
