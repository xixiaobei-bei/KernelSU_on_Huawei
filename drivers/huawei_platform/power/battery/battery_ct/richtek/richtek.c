/*
 * richtek.c
 *
 * Authentication driver for richtek
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "richtek.h"
#include <linux/random.h>
#include <huawei_platform/power/power_mesg_srv.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/battery/battery_type_identify.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG hw_richtek_drv
HWLOG_REGIST();

static void rt_sched_affinity_to_current(void)
{
	long retval;
	int current_cpu;

	preempt_disable();
	current_cpu = smp_processor_id();
	preempt_enable();

	retval = sched_setaffinity(RT_CURRENT_TASK,
		cpumask_of(current_cpu));
	if (retval)
		hwlog_err("setting current cpu affinity failed %ld\n",
			retval);
	else
		hwlog_info("setting current cpu%d affinity\n",
			current_cpu);
}

static void rt_sched_affinity_to_all(void)
{
	long retval;
	cpumask_t dstp;

	cpumask_setall(&dstp);
	retval = sched_setaffinity(RT_CURRENT_TASK, &dstp);
	if (retval)
		hwlog_err("setting all cpus affinity failed %ld\n",
			retval);
	else
		hwlog_info("setting all cpus affinity\n");
}

static enum batt_ic_type rt_get_ic_type(void)
{
	return RICHTEK_A1_TYPE;
}

static int rt_get_device_id(struct rt_drv_data *drv_data)
{
	int ret;

	if (!drv_data->device_id)
		return RT_FAIL;

	if (*drv_data->device_id)
		return RT_SUCCESS;

	rt_sched_affinity_to_current();
	ret = drv_data->ic_des.com_ops.read(&drv_data->ic_des.obj,
		RT_READ_DEVICE_ID,
		drv_data->device_id + 1,
		drv_data->device_id_size);
	if (!ret)
		*drv_data->device_id = 1;

	rt_sched_affinity_to_all();
	return ret;
}

static int rt_get_rom_id(struct rt_drv_data *drv_data)
{
	int ret;

	if (!drv_data->rom_id)
		return RT_FAIL;

	if (*drv_data->rom_id)
		return RT_SUCCESS;

	rt_sched_affinity_to_current();
	ret = drv_data->ic_des.com_ops.read(&drv_data->ic_des.obj,
		RT_READ_ROM_ID, drv_data->rom_id + 1, drv_data->rom_id_size);
	if (!ret)
		*drv_data->rom_id = 1;

	rt_sched_affinity_to_all();
	return ret;
}

static int rt_get_status(struct rt_drv_data *drv_data, bool retry)
{
	int ret;

	if (!drv_data->status)
		return RT_FAIL;

	if (*drv_data->status)
		return RT_SUCCESS;

	if (retry)
		ret = drv_data->ic_des.com_ops.read(
			&drv_data->ic_des.obj, RT_READ_STATUS,
			drv_data->status + 1, drv_data->status_size);
	else
		ret = drv_data->ic_des.com_ops.read_one_shot(
			&drv_data->ic_des.obj, RT_READ_STATUS,
			drv_data->status + 1, drv_data->status_size);
	if (!ret)
		*drv_data->status = 1;

	return ret;
}

static int rt_get_batt_type(struct platform_device *pdev,
	const unsigned char **type, unsigned int *type_len)
{
	int ret;
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);
	char temp_batt_type[RT_BYTE_BITS] = { 0 };

	if (!drv_data || !drv_data->batt_type)
		return RT_FAIL;

	if (!type || !type_len) {
		hwlog_err("type & type_len should not be null\n");
		return RT_FAIL;
	}

	if (*drv_data->batt_type)
		goto return_batt_type;

	rt_sched_affinity_to_current();
	ret = drv_data->ic_des.com_ops.read(&drv_data->ic_des.obj,
		RT_READ_MFA_PAGE1, temp_batt_type, RT_BYTE_BITS);
	rt_sched_affinity_to_all();
	if (ret)
		return RT_FAIL;
	*drv_data->batt_type = 1;
	*(drv_data->batt_type + 1) = temp_batt_type[RT_BATT_TYPE_BIT1];
	*(drv_data->batt_type + 2) = temp_batt_type[RT_BATT_TYPE_BIT2];

return_batt_type:
	*type = drv_data->batt_type + 1;
	/* effective battery type length */
	*type_len = drv_data->batt_type_size;
	return RT_SUCCESS;
}

static void rt_parse_printable_sn(unsigned char *origin,
	unsigned int origin_length, unsigned char *sn, unsigned int sn_length)
{
	int index;
	char hex_print;
	int i;
	unsigned char *tmp = origin;

	/* 2: one char = two hex */
	if (origin_length < (RT_SN_CHAR_PRINT_SIZE + RT_SN_HEX_PRINT_SIZE / 2))
		return;

	if (sn_length < (RT_SN_CHAR_PRINT_SIZE + RT_SN_HEX_PRINT_SIZE))
		return;

	for (index = 0; index < RT_REAL_CHAR_NUM; index++)
		sn[index] = tmp[index];

	tmp = origin + RT_REAL_CHAR_NUM;
	for (i = 0; i < RT_YEAR_MONTH_NUM; i++) {
		if (is_odd(i))
			hex_print = (tmp[i / 2] & 0x0f);
		else
			hex_print = ((tmp[i / 2] & 0xf0) >> 4) & 0x0f;
		sprintf(sn + (i + index), "%X", hex_print);
	}
	index += RT_YEAR_MONTH_NUM;
	tmp++;

	hex_print = ((*tmp & 0x0f) << 4) | ((*(tmp + 1) & 0xf0) >> 4);
	*(sn + index) = hex_print;
	index++;
	tmp++;

	hex_print = (*(tmp + 1) & 0x0f);
	sprintf(sn + index, "%X", hex_print);
	index++;
	tmp++;

	for (i = 0; i < RT_SERIAL_CODE_NUM; i++) {
		if (is_odd(i))
			hex_print = (tmp[i / 2] & 0x0f);
		else
			hex_print = ((tmp[i / 2] & 0xf0) >> 4) & 0x0f;
		sprintf(sn + (i + index), "%X", hex_print);
	}
}

static int rt_get_sn(struct platform_device *pdev, struct power_genl_attr *res,
	const unsigned char **sn, unsigned int *sn_size)
{
	int ret;
	unsigned char tmp_sn[RT_MFA_PAGE1_SIZE] = { 0 };
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);

	if (!drv_data || !drv_data->sn || !sn || !sn_size)
		return RT_FAIL;
	if (drv_data->sn_size < RT_SN_MIN_SIZE) {
		hwlog_err("sn size %u is too small\n", drv_data->sn_size);
		return RT_FAIL;
	}

	if (*drv_data->sn)
		goto return_batt_sn;

	rt_sched_affinity_to_current();
	ret = drv_data->ic_des.com_ops.read(&drv_data->ic_des.obj,
		RT_READ_MFA_PAGE1, tmp_sn, RT_MFA_PAGE1_SIZE);
	rt_sched_affinity_to_all();
	if (ret)
		return RT_FAIL;

	rt_parse_printable_sn(tmp_sn, RT_MFA_PAGE1_SIZE,
		drv_data->sn + 1, RT_SN_MIN_SIZE);
	*drv_data->sn = 1;

return_batt_sn:
	*sn = drv_data->sn + 1;
	/* effective battery type length */
	*sn_size = drv_data->sn_size;
	return RT_SUCCESS;
}

static unsigned char rt_get_bit_status(struct rt_drv_data *drv_data,
	enum rt_status_bit bit)
{
	unsigned int bytes = bit / RT_BYTE_BITS;
	unsigned int byte_offset = bit % RT_BYTE_BITS;

	if (bytes >= drv_data->status_size)
		return RT_FAIL;
	bytes = drv_data->status_size - bytes - 1;

	return (drv_data->status[bytes + 1] >> byte_offset) & 0x1;
}

static int rt_check_device_id(struct rt_drv_data *drv_data)
{
	/* {0x94, 0x30, 0xAA, 0x00} : device id address */
	const char device_id[RT_DEVICE_ID_LEN] = { 0x94, 0x30, 0xAA, 0x00 };
	int i;

	if (drv_data->device_id_size != RT_DEVICE_ID_LEN)
		return RT_FAIL;

	for (i = 0; i < RT_DEVICE_ID_LEN; i++) {
		if (*(drv_data->device_id + i + 1) != device_id[i])
			return RT_FAIL;
	}
	return RT_SUCCESS;
}

static int rt_check_page_status(struct rt_drv_data *drv_data)
{
	unsigned char page_batt_type_status;
	unsigned char page_secret_status;
	char err = RT_SUCCESS;

	if (!drv_data->status || !(*drv_data->status))
		return RT_FAIL;

	/* battery type page */
	page_batt_type_status = rt_get_bit_status(drv_data, RT_MFA_PAGE1_LCK);
	if (!page_batt_type_status)
		hwlog_err("battery type page was not locked\n");
	else
		hwlog_info("info battery type page was locked\n");

	/* master secret page */
	page_secret_status = rt_get_bit_status(drv_data, RT_SECRET_LCK);
	if (!page_secret_status) {
		err = RT_FAIL;
		hwlog_err("secret page was not locked\n");
	} else {
		hwlog_info("info secret page was locked\n");
	}

	return err;
}

static int rt_check_ic_status(struct platform_device *pdev)
{
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);

	if (!drv_data)
		return RT_FAIL;

	/* rom id */
	if (rt_get_device_id(drv_data)) {
		hwlog_err("get rom id failed\n");
		return RT_FAIL;
	}

	if (rt_check_device_id(drv_data))
		return RT_FAIL;

	/* status */
	rt_sched_affinity_to_current();
	if (rt_get_status(drv_data, true)) {
		hwlog_err("get page status failed\n");
		rt_sched_affinity_to_all();
		return RT_FAIL;
	}
	rt_sched_affinity_to_all();

	return rt_check_page_status(drv_data);
}

static int rt_lock_sn_page(struct rt_drv_data *drv_data)
{
	int ret;
	int ret_status;
	unsigned char sn_secret_status;

	if (!drv_data->status)
		return RT_FAIL;

	rt_sched_affinity_to_current();
	*drv_data->status = 0;
	ret = drv_data->ic_des.com_ops.write(&drv_data->ic_des.obj,
		RT_LOCK_MFA_PAGE1, NULL, 0);
	/* refresh status after lock mfa page */
	ret_status = rt_get_status(drv_data, true);
	if (ret_status != 0)
		hwlog_err("get rt_status fail\n");

	rt_sched_affinity_to_all();

	/* make sure mfa page was locked */
	sn_secret_status = rt_get_bit_status(drv_data, RT_MFA_PAGE1_LCK);
	if (*drv_data->status && sn_secret_status)
		return RT_SUCCESS;

	hwlog_err("lock sn page failed %d %u %u\n", ret,
		*drv_data->status, sn_secret_status);
	return RT_FAIL;
}

static int rt_set_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);
	int ret = RT_FAIL;

	if (!drv_data || !drv_data->status)
		return RT_FAIL;

	mutex_lock(&drv_data->batt_safe_info_lock);
	/* 5 * HZ = 5s */
	__pm_wakeup_event(&(drv_data->write_lock), jiffies_to_msecs(5 * HZ));
	switch (type) {
	case BATT_MATCH_ABILITY:
		if (*(enum batt_match_type *)value == BATTERY_REMATCHABLE)
			break;

		if (*drv_data->status &&
			rt_get_bit_status(drv_data, RT_MFA_PAGE1_LCK)) {
			ret = RT_SUCCESS;
			hwlog_info("battery was already write protection\n");
			break;
		}

		hwlog_info("set battery to write protection\n");
		if (!rt_lock_sn_page(drv_data))
			ret = RT_SUCCESS;
		break;
	default:
		hwlog_err("unsupported battery safety type\n");
		break;
	}
	__pm_relax(&drv_data->write_lock);
	mutex_unlock(&drv_data->batt_safe_info_lock);

	return ret;
}

static int rt_get_batt_safe_info(struct platform_device *pdev,
	enum batt_safe_info_t type, void *value)
{
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);
	int ret = RT_FAIL;

	if (!drv_data || !drv_data->status)
		return RT_FAIL;

	mutex_lock(&drv_data->batt_safe_info_lock);
	switch (type) {
	case BATT_MATCH_ABILITY:
		if (rt_get_status(drv_data, true))
			break;

		if (rt_get_bit_status(drv_data, RT_MFA_PAGE1_LCK))
			*(enum batt_match_type *)value = BATTERY_UNREMATCHABLE;
		else
			*(enum batt_match_type *)value = BATTERY_REMATCHABLE;

		ret = RT_SUCCESS;
		break;
	default:
		hwlog_err("unsupported battery safety type\n");
		break;
	}
	mutex_unlock(&drv_data->batt_safe_info_lock);

	return ret;
}

static int rt_prepare(struct platform_device *pdev,
	enum res_type type, struct power_genl_attr *res)
{
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);
	unsigned char *mac_datum = NULL;
	const unsigned char work_challenge[RT_WORK_CHALLENGE_SIZE] = {
		0xfe, 0xce, 0xa3, 0xad, 0xe0, 0x6b, 0xa9, 0xac,
		0x8, 0x42, 0x7, 0xea, 0x72, 0xa4, 0x5a, 0xe1,
		0xf4, 0x7c, 0xe9, 0xc3, 0x13, 0x53, 0xec, 0xd0,
		0xaa, 0xc7, 0xf7, 0x12, 0x83, 0xcc, 0x6d, 0x5a
	};
	unsigned char work_secret[RT_WORK_SECRET_SIZE] = { 0 };

	if (!drv_data || !drv_data->mac_datum)
		return RT_FAIL;
	mac_datum = drv_data->mac_datum;

	if (!res) {
		hwlog_err("mac resource should not be null\n");
		return RT_FAIL;
	}

	switch (type) {
	case RES_CT:
		if (rt_get_rom_id(drv_data)) {
			hwlog_err("get rom id failed for prepare\n");
			return RT_FAIL;
		}

		memset(mac_datum, 0, drv_data->mac_datum_size);
		/* work challenge */
		memcpy(mac_datum, work_challenge,
			drv_data->work_challenge_size);
		mac_datum += drv_data->work_challenge_size;
		/* work secret */
		memcpy(mac_datum, work_secret,
			drv_data->work_secret_size);
		mac_datum += drv_data->work_secret_size;
		/* root challenge */
		get_random_bytes(mac_datum, drv_data->root_challenge_size);
		mac_datum += drv_data->root_challenge_size;
		/* rom id */
		memcpy(mac_datum, drv_data->rom_id + 1, drv_data->rom_id_size);

		res->data = drv_data->mac_datum;
		res->len = drv_data->mac_datum_size;
		return RT_SUCCESS;
	case RES_SN:
		res->data = NULL;
		res->len = 0;
		return RT_SUCCESS;
	default:
		hwlog_err("wrong mac resource type %ud\n", type);
		break;
	}

	return RT_FAIL;
}

static int rt_write_challenge(struct rt_drv_data *drv_data,
	unsigned char *challenge, unsigned int size)
{
	int ret;
	unsigned char ready;

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_WRITE_CHALLENGE, challenge, size);
	if (ret)
		return ret;

	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	ready = rt_get_bit_status(drv_data, RT_CHALLENGE_RDY);
	if (*drv_data->status && ready)
		return RT_SUCCESS;

	hwlog_err("challenge not ready %d %u %u\n", ret,
		*drv_data->status, ready);
	return RT_FAIL;
}

static int rt_gen_work_secret(struct rt_drv_data *drv_data)
{
	int ret;
	unsigned char *work_challenge = NULL;
	unsigned char ready;

	if (!drv_data->mac_datum)
		return RT_FAIL;
	work_challenge = drv_data->mac_datum;

	ret = rt_write_challenge(drv_data, work_challenge,
		drv_data->work_challenge_size);
	if (ret)
		return ret;

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_GEN_WORK_SECRET, NULL, 0);
	if (ret)
		return ret;

	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	ready = rt_get_bit_status(drv_data, RT_WK_SECRET_RDY);
	if (*drv_data->status && ready)
		return RT_SUCCESS;

	hwlog_err("work secret not ready %d %u %u\n", ret,
		*drv_data->status, ready);
	return RT_FAIL;
}

static int rt_gen_mac(struct rt_drv_data *drv_data)
{
	int ret;
	unsigned char *root_challenge = NULL;
	unsigned char ready;

	if (!drv_data->mac_datum)
		return RT_FAIL;
	root_challenge = drv_data->mac_datum + drv_data->work_secret_size +
		drv_data->work_challenge_size;

	ret = rt_write_challenge(drv_data, root_challenge,
		drv_data->root_challenge_size + drv_data->rom_id_size);
	if (ret)
		return ret;

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_AUTH_WI_ROM_ID, NULL, 0);
	if (ret)
		return ret;

	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	ready = rt_get_bit_status(drv_data, RT_MAC_RDY);
	if (*drv_data->status && ready)
		return RT_SUCCESS;

	hwlog_err("mac not ready %d %u %u\n", ret,
		*drv_data->status, ready);
	return RT_FAIL;
}

static int rt_certification(struct platform_device *pdev,
	struct power_genl_attr *res, enum key_cr *result)
{
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);
	unsigned char remote_mac[RT_MAX_MAC_SIZE] = { 0 };
	int i;
	const unsigned char *data = NULL;
	unsigned int data_len;

	if (!res || !result || !res->data || !drv_data) {
		hwlog_err("null point found in certification\n");
		return RT_FAIL;
	}
	data = res->data;
	data_len = res->len;

	if ((data_len != drv_data->mac_size) ||
		(data_len > RT_MAX_MAC_SIZE)) {
		hwlog_err("certification data length %u is not correct\n",
			data_len);
		return RT_FAIL;
	}

	rt_sched_affinity_to_current();
	for (i = 0; i < RT_AUTH_RETRY; i++) {
		if (rt_gen_work_secret(drv_data))
			continue;
		if (rt_gen_mac(drv_data))
			continue;
		if (drv_data->ic_des.com_ops.read_one_shot(
			&drv_data->ic_des.obj, RT_READ_MAC,
			remote_mac, data_len))
			continue;

		if (memcmp(remote_mac, data, data_len)) {
			*result = KEY_FAIL_UNMATCH;
			hwlog_err("mac is unmatch\n");
		} else {
			*result = KEY_PASS;
			hwlog_err("mac is match\n");
		}
		break;
	}

	if (i >= RT_AUTH_RETRY)
		*result = KEY_FAIL_TIMEOUT;

	rt_sched_affinity_to_all();
	return RT_SUCCESS;
}

#ifdef ONEWIRE_STABILITY_DEBUG
static int rt_snprintf_array(unsigned char *buf, int buf_len,
	const unsigned char *array, int array_len)
{
	int i;
	int ret = 0;

	for (i = 0; i < array_len; i++) {
		ret += snprintf(buf + ret, buf_len - ret, "%02X ", array[i]);
		/* size of "%02X " is 4 */
		if ((ret + 4) >= buf_len)
			break;
	}

	return ret;
}

static ssize_t rt_device_id_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int val;
	int ret;

	if (!drv_data)
		return 0;

	*drv_data->device_id = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_get_device_id(drv_data);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error");

	val = snprintf(buff, PAGE_SIZE, "ok ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val,
		drv_data->device_id + 1, drv_data->device_id_size);
	return val;
}

static ssize_t rt_rom_id_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int val;
	int ret;

	if (!drv_data)
		return 0;

	*drv_data->rom_id = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_get_rom_id(drv_data);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error");

	val = snprintf(buff, PAGE_SIZE, "ok ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val,
		drv_data->rom_id + 1, drv_data->rom_id_size);
	return val;
}

static ssize_t rt_mfa_page_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int val;
	int ret;
	unsigned char mfa_page1[RT_MFA_PAGE1_SIZE] = { 0 };
	unsigned char mfa_page2[RT_MFA_PAGE2_SIZE] = { 0 };

	if (!drv_data)
		return 0;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	ret = drv_data->ic_des.com_ops.read_one_shot(&drv_data->ic_des.obj,
		RT_READ_MFA_PAGE1, mfa_page1, RT_MFA_PAGE1_SIZE);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error read page1-%d", ret);

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	ret = drv_data->ic_des.com_ops.read_one_shot(&drv_data->ic_des.obj,
		RT_READ_MFA_PAGE2, mfa_page2, RT_MFA_PAGE2_SIZE);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error read page2-%d", ret);

	val = snprintf(buff, PAGE_SIZE, "ok page1: ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val, mfa_page1,
		RT_MFA_PAGE1_SIZE);
	val += snprintf(buff + val, PAGE_SIZE, " page2: ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val, mfa_page2,
		RT_MFA_PAGE2_SIZE);
	return val;
}

static ssize_t rt_status_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int val;

	if (!drv_data)
		return 0;

	*drv_data->status = 0;
	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_get_status(drv_data, false);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error");

	val = snprintf(buff, PAGE_SIZE, "ok ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val,
		drv_data->status + 1, drv_data->status_size);
	return val;
}

static ssize_t rt_check_ic_status_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;

	if (!drv_data || !pdev)
		return 0;

	*drv_data->status = 0;
	*drv_data->rom_id = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_check_ic_status(pdev);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error");

	return snprintf(buff, PAGE_SIZE, "ok");
}

static ssize_t rt_sn_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int val;
	int sn_len = 0;
	const unsigned char *sn = NULL;

	if (!drv_data || !pdev)
		return 0;

	*drv_data->sn = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_get_sn(pdev, 0, &sn, &sn_len);
	bat_type_release_mode(false);
	if (ret || !sn)
		return snprintf(buff, PAGE_SIZE, "error");

	val = snprintf(buff, PAGE_SIZE, "ok ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val, sn, sn_len);
	return val;
}

static ssize_t rt_batt_type_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int val;
	int type_len = 0;
	const unsigned char *type = NULL;

	if (!drv_data || !pdev)
		return 0;

	*drv_data->batt_type = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_get_batt_type(pdev, &type, &type_len);
	bat_type_release_mode(false);
	if (ret || !type)
		return snprintf(buff, PAGE_SIZE, "error");

	val = snprintf(buff, PAGE_SIZE, "ok ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val, type, type_len);
	return val;
}

static ssize_t rt_batt_safe_info_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	enum batt_match_type value;
	int ret;

	if (!drv_data || !pdev)
		return 0;

	*drv_data->status = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_get_batt_safe_info(pdev, BATT_MATCH_ABILITY, &value);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error");

	return snprintf(buff, PAGE_SIZE, "ok match type is %d", value);
}

static ssize_t rt_batt_safe_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	enum batt_match_type value = BATTERY_UNREMATCHABLE;
	int ret;

	if (!drv_data || !pdev)
		return -1;

	*drv_data->status = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_set_batt_safe_info(pdev, BATT_MATCH_ABILITY, &value);
	bat_type_release_mode(false);
	if (ret)
		return -1;

	return count;
}

static ssize_t rt_lock_mfa_page_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	unsigned char page_no;

	if (!drv_data)
		return -1;
	/* page no + '\0' */
	if (count != 2)
		return -1;
	/* 16: hexadecimal data */
	if (kstrtou8(buf, 16, &page_no))
		return -1;

	if (page_no >= RT_MFA_MAX_PAGE_NO)
		return -1;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	if (page_no == RT_MFA_PAGE1)
		ret = drv_data->ic_des.com_ops.write_one_shot(
			&drv_data->ic_des.obj, RT_LOCK_MFA_PAGE1, NULL, 0);
	else
		ret = drv_data->ic_des.com_ops.write_one_shot(
			&drv_data->ic_des.obj, RT_LOCK_MFA_PAGE2, NULL, 0);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return -1;

	return count;
}

static ssize_t rt_prepare_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	struct power_genl_attr res;
	int ret;
	int val;

	if (!drv_data || !pdev)
		return 0;

	*drv_data->rom_id = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_prepare(pdev, RES_CT, &res);
	bat_type_release_mode(false);
	if (ret || !res.data)
		return snprintf(buff, PAGE_SIZE, "error");

	val = snprintf(buff, PAGE_SIZE, "ok ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val,
		res.data, res.len);
	return val;
}

static ssize_t rt_certification_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	struct power_genl_attr res;
	enum key_cr result;
	int ret;
	unsigned char local_mac[RT_MAX_MAC_SIZE] = { 0 };

	if (!drv_data || !pdev)
		return 0;
	res.data = local_mac;
	res.len = drv_data->mac_size;

	*drv_data->rom_id = 0;
	bat_type_apply_mode(BAT_ID_SN);
	ret = rt_certification(pdev, &res, &result);
	bat_type_release_mode(false);
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error");

	return snprintf(buff, PAGE_SIZE, "ok key is %d", result);
}

static int rt_str_to_hex_array(const char *buf, unsigned int buf_size,
	char *hex, unsigned int hex_size)
{
	char str[RT_TEST_BUF_LEN] = { 0 };
	unsigned int len;
	unsigned int index = 0;
	char *sub = NULL;
	char *cur = NULL;

	len = (buf_size < sizeof(str) - 1) ? buf_size : (sizeof(str) - 1);
	memcpy(str, buf, len);

	cur = &str[0];
	sub = strsep(&cur, " ");
	while (sub) {
		if (*sub != '\0') {
			/* 16: data is hexadecimal */
			if (kstrtou8(sub, 16, &hex[index]))
				return -1;

			index++;
			if (index >= hex_size)
				return index;
		}
		sub = strsep(&cur, " ");
	}
	return index;
}

static ssize_t rt_mfa_page_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int hex_size;
	unsigned int cmd_no;
	unsigned char hex[RT_MAX_DATA_LEN];
	unsigned char page_no;
	enum rt_status_bit lock_bit;

	if (!drv_data)
		return -1;

	hex_size = rt_str_to_hex_array(buf, count, hex, RT_MAX_DATA_LEN);
	if (hex_size <= 1)
		return -1;
	page_no = hex[0];

	if (page_no >= RT_MFA_MAX_PAGE_NO)
		return -1;
	if (page_no == RT_MFA_PAGE1) {
		cmd_no = RT_WRITE_MFA_PAGE1;
		lock_bit = RT_MFA_PAGE1_LCK;
	} else {
		cmd_no = RT_WRITE_MFA_PAGE2;
		lock_bit = RT_MFA_PAGE2_LCK;
	}

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	if (!(*drv_data->status) || rt_get_bit_status(drv_data, lock_bit)) {
		bat_type_release_mode(false);
		rt_sched_affinity_to_all();
		return -1;
	}

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		cmd_no, hex + 1, hex_size - 1);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return -1;

	return count;
}

static ssize_t rt_sys_page_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int val;
	int ret;
	unsigned char sys_page[RT_SYS_PAGE_SIZE] = { 0 };

	if (!drv_data)
		return 0;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	if (!(*drv_data->status) || !rt_get_bit_status(drv_data, RT_SYS_MODE)) {
		bat_type_release_mode(false);
		rt_sched_affinity_to_all();
		return snprintf(buff, PAGE_SIZE, "not in sys mode %d", ret);
	}

	ret = drv_data->ic_des.com_ops.read_one_shot(&drv_data->ic_des.obj,
		RT_READ_SYS_PAGE, sys_page, RT_SYS_PAGE_SIZE);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error read sys page %d",
			ret);

	val = snprintf(buff, PAGE_SIZE, "ok sys page: ");
	val += rt_snprintf_array(buff + val, PAGE_SIZE - val, sys_page,
		RT_SYS_PAGE_SIZE);
	return val;
}

static ssize_t rt_sys_page_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int hex_size;
	unsigned char hex[RT_MAX_DATA_LEN] = { 0 };

	if (!drv_data)
		return -1;

	hex_size = rt_str_to_hex_array(buf, count, hex, RT_MAX_DATA_LEN);
	if (hex_size <= 0)
		return -1;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	if (!(*drv_data->status) || !rt_get_bit_status(drv_data, RT_SYS_MODE)) {
		bat_type_release_mode(false);
		rt_sched_affinity_to_all();
		return -1;
	}

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_WRITE_SYS_PAGE, hex, hex_size);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return -1;

	return count;
}

static int rt_unseal_sys(struct rt_drv_data *drv_data,
	unsigned char *psw, unsigned int size)
{
	int ret;
	unsigned char ready;

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_UNSEAL_SYS, psw, size);
	if (ret)
		return ret;

	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	ready = rt_get_bit_status(drv_data, RT_SYS_MODE);
	if (*drv_data->status && ready)
		return RT_SUCCESS;

	hwlog_err("unseal sys mode failed %d %u %u\n", ret,
		*drv_data->status, ready);
	return RT_FAIL;
}

static int rt_seal_nvm(struct rt_drv_data *drv_data)
{
	int ret;
	unsigned char ready;

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_SEAL_NVM, NULL, 0);
	if (ret)
		return ret;

	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	ready = rt_get_bit_status(drv_data, RT_SYS_MODE);
	if (*drv_data->status && !ready)
		return RT_SUCCESS;

	hwlog_err("seal sys mode failed %d %u %u\n", ret,
		*drv_data->status, ready);
	return RT_FAIL;
}

static ssize_t rt_sys_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int hex_size;
	unsigned char hex[RT_MAX_DATA_LEN] = { 0 };

	if (!drv_data)
		return -1;

	hex_size = rt_str_to_hex_array(buf, count, hex, RT_MAX_DATA_LEN);
	if (hex_size <= 0)
		return -1;
	if (((hex[0] == 0) && (hex_size != (RT_MODE_PSW_SIZE + 1)))
		|| ((hex[0] == 1) && (hex_size != 1)))
		return -1;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	if (hex[0] == 0)
		ret = rt_unseal_sys(drv_data, hex + 1, RT_MODE_PSW_SIZE);
	else
		ret = rt_seal_nvm(drv_data);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return -1;

	return count;
}

static ssize_t rt_sys_pwd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	int hex_size;
	unsigned char hex[RT_MAX_DATA_LEN] = { 0 };

	if (!drv_data)
		return -1;

	hex_size = rt_str_to_hex_array(buf, count, hex, RT_MAX_DATA_LEN);
	if (hex_size != RT_MODE_PSW_SIZE)
		return -1;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	*drv_data->status = 0;
	ret = rt_get_status(drv_data, false);
	if (!(*drv_data->status) || !rt_get_bit_status(drv_data, RT_SYS_MODE)) {
		bat_type_release_mode(false);
		rt_sched_affinity_to_all();
		return -1;
	}
	ret = drv_data->ic_des.com_ops.write(&drv_data->ic_des.obj,
		RT_WRITE_SYS_PWD, hex, RT_MODE_PSW_SIZE);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return -1;

	return count;
}

static ssize_t rt_cyc_cnt_show(struct device *dev,
	struct device_attribute *attr, char *buff)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);
	int ret;
	unsigned int cyc_cnt_before;
	unsigned int cyc_cnt_after;
	unsigned char buf[RT_CYC_CNT_SIZE] = { 0 };

	if (!drv_data)
		return 0;

	rt_sched_affinity_to_current();
	bat_type_apply_mode(BAT_ID_SN);
	ret = drv_data->ic_des.com_ops.read_one_shot(&drv_data->ic_des.obj,
		RT_READ_CYC_CN, buf, RT_CYC_CNT_SIZE);
	if (ret) {
		bat_type_release_mode(false);
		rt_sched_affinity_to_all();
		return snprintf(buff, PAGE_SIZE, "error read cyc cnt %d", ret);
	}
	cyc_cnt_before = buf[0] << RT_BYTE_BITS;
	cyc_cnt_before |= buf[1];

	ret = drv_data->ic_des.com_ops.write_one_shot(&drv_data->ic_des.obj,
		RT_INC_CYC_CNT, NULL, 0);
	if (ret) {
		bat_type_release_mode(false);
		rt_sched_affinity_to_all();
		return snprintf(buff, PAGE_SIZE, "error incr cys cnt %d", ret);
	}

	ret = drv_data->ic_des.com_ops.read_one_shot(&drv_data->ic_des.obj,
		RT_READ_CYC_CN, buf, RT_CYC_CNT_SIZE);
	bat_type_release_mode(false);
	rt_sched_affinity_to_all();
	if (ret)
		return snprintf(buff, PAGE_SIZE, "error read cyc cnt %d", ret);

	cyc_cnt_after = buf[0] << RT_BYTE_BITS;
	cyc_cnt_after |= buf[1];

	return snprintf(buff, PAGE_SIZE, "ok cyc cnt %u:%u",
		cyc_cnt_before, cyc_cnt_after);
}

static ssize_t rt_com_stat_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);

	if (!drv_data)
		return 0;

	return drv_data->ic_des.com_ops.get_stats(&drv_data->ic_des.obj,
		buf, PAGE_SIZE);
}

static ssize_t rt_com_stat_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt_drv_data *drv_data = dev_get_drvdata(dev);

	if (!drv_data)
		return -1;

	drv_data->ic_des.com_ops.reset_stats(&drv_data->ic_des.obj);
	return count;
}

static DEVICE_ATTR_RO(rt_device_id);
static DEVICE_ATTR_RO(rt_rom_id);
static DEVICE_ATTR_RW(rt_mfa_page);
static DEVICE_ATTR_RO(rt_status);
static DEVICE_ATTR_RO(rt_check_ic_status);
static DEVICE_ATTR_RO(rt_sn);
static DEVICE_ATTR_RO(rt_batt_type);
static DEVICE_ATTR_RW(rt_batt_safe_info);
static DEVICE_ATTR_WO(rt_lock_mfa_page);
static DEVICE_ATTR_RO(rt_prepare);
static DEVICE_ATTR_RO(rt_certification);
static DEVICE_ATTR_RW(rt_sys_page);
static DEVICE_ATTR_WO(rt_sys_mode);
static DEVICE_ATTR_WO(rt_sys_pwd);
static DEVICE_ATTR_RO(rt_cyc_cnt);
static DEVICE_ATTR_RW(rt_com_stat);

static const struct attribute *g_rt_attrs[] = {
	&dev_attr_rt_device_id.attr,
	&dev_attr_rt_rom_id.attr,
	&dev_attr_rt_mfa_page.attr,
	&dev_attr_rt_status.attr,
	&dev_attr_rt_check_ic_status.attr,
	&dev_attr_rt_sn.attr,
	&dev_attr_rt_batt_type.attr,
	&dev_attr_rt_batt_safe_info.attr,
	&dev_attr_rt_lock_mfa_page.attr,
	&dev_attr_rt_prepare.attr,
	&dev_attr_rt_certification.attr,
	&dev_attr_rt_sys_page.attr,
	&dev_attr_rt_sys_mode.attr,
	&dev_attr_rt_sys_pwd.attr,
	&dev_attr_rt_cyc_cnt.attr,
	&dev_attr_rt_com_stat.attr,
	NULL, /* sysfs_create_files need last one be NULL */
};
#endif /* ONEWIRE_STABILITY_DEBUG */

int rt_ct_ops_register(struct platform_device *pdev, struct batt_ct_ops *bco)
{
	int ret;
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);
	struct rt_comm_ops *ops = NULL;

	if (!drv_data) {
		hwlog_err("get driver data failed in rt register\n");
		return RT_FAIL;
	}
	ops = &drv_data->ic_des.com_ops;

	if (!ops->register_com_phy)
		return RT_FAIL;
	if (ops->register_com_phy(&drv_data->ic_des.obj,
		drv_data->phy_id)) {
		hwlog_err("register phy ops failed for rt\n");
		return RT_FAIL;
	}

	if (rt_get_device_id(drv_data)) {
		hwlog_err("get device id failed\n");
		return RT_FAIL;
	}
	ret = rt_get_rom_id(drv_data);
	if (ret != 0)
		hwlog_err("get rom_id fail\n");

	if (rt_check_ic_status(pdev)) {
		hwlog_err("%s ic status was not fine\n", pdev->name);
		return RT_FAIL;
	}

	bco->get_ic_type = rt_get_ic_type;
	bco->get_batt_type = rt_get_batt_type;
	bco->get_batt_sn = rt_get_sn;
	bco->set_batt_safe_info = rt_set_batt_safe_info;
	bco->get_batt_safe_info = rt_get_batt_safe_info;
	bco->prepare = rt_prepare;
	bco->certification = rt_certification;
	return RT_SUCCESS;
}

static int rt_parse_dts(struct device_node *node,
	struct rt_drv_data *drv_data)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"device-id-length", &drv_data->device_id_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"rom-id-length", &drv_data->rom_id_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"sn-length", &drv_data->sn_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"batt-type-length", &drv_data->batt_type_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"status-length", &drv_data->status_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"mac-length", &drv_data->mac_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"work-secret-length", &drv_data->work_secret_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"work-challenge-length",
			&drv_data->work_challenge_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"root-challenge-length",
			&drv_data->root_challenge_size, 0) ||
		power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
			"phy-ctrl", &drv_data->phy_id, 0))
		return RT_FAIL;

	drv_data->mac_datum_size = drv_data->work_challenge_size +
		drv_data->work_secret_size + drv_data->root_challenge_size +
		drv_data->rom_id_size;
	return RT_SUCCESS;
}

static int rt_memory_init(struct platform_device *pdev,
	struct rt_drv_data *drv_data)
{
	drv_data->device_id = devm_kzalloc(&pdev->dev,
		(drv_data->device_id_size + 1), GFP_KERNEL);
	if (!drv_data->device_id)
		return RT_FAIL;

	drv_data->rom_id = devm_kzalloc(&pdev->dev,
		(drv_data->rom_id_size + 1), GFP_KERNEL);
	if (!drv_data->rom_id)
		goto free_device_id_mem;

	/* sn data sign + eof */
	drv_data->sn = devm_kzalloc(&pdev->dev,
		(drv_data->sn_size + 2), GFP_KERNEL);
	if (!drv_data->sn)
		goto free_rom_id_mem;

	drv_data->batt_type = devm_kzalloc(&pdev->dev,
		(drv_data->batt_type_size + 1), GFP_KERNEL);
	if (!drv_data->batt_type)
		goto free_sn_mem;

	drv_data->status = devm_kzalloc(&pdev->dev,
		(drv_data->status_size + 1), GFP_KERNEL);
	if (!drv_data->status)
		goto free_batt_type_mem;

	drv_data->mac_datum = devm_kzalloc(&pdev->dev,
		drv_data->mac_datum_size, GFP_KERNEL);
	if (!drv_data->mac_datum)
		goto free_status_mem;

	wakeup_source_init(&drv_data->write_lock, pdev->name);
	mutex_init(&drv_data->batt_safe_info_lock);
	return RT_SUCCESS;

free_status_mem:
	devm_kfree(&pdev->dev, drv_data->mac_datum);
	drv_data->mac_datum = NULL;
free_batt_type_mem:
	devm_kfree(&pdev->dev, drv_data->batt_type);
	drv_data->batt_type = NULL;
free_sn_mem:
	devm_kfree(&pdev->dev, drv_data->sn);
	drv_data->sn = NULL;
free_rom_id_mem:
	devm_kfree(&pdev->dev, drv_data->rom_id);
	drv_data->rom_id = NULL;
free_device_id_mem:
	devm_kfree(&pdev->dev, drv_data->device_id);
	drv_data->device_id = NULL;
	return RT_FAIL;
}

static void  rt_drv_data_release(struct platform_device *pdev)
{
	struct rt_drv_data *drv_data = platform_get_drvdata(pdev);

	if (!drv_data)
		return;

	hwlog_info("rt: memory release\n");

	rt_destroy_com_obj(&drv_data->ic_des.obj, pdev);

	devm_kfree(&pdev->dev, drv_data->device_id);
	drv_data->device_id = NULL;
	devm_kfree(&pdev->dev, drv_data->rom_id);
	drv_data->rom_id = NULL;
	devm_kfree(&pdev->dev, drv_data->sn);
	drv_data->sn = NULL;
	devm_kfree(&pdev->dev, drv_data->batt_type);
	drv_data->batt_type = NULL;
	devm_kfree(&pdev->dev, drv_data->status);
	drv_data->status = NULL;
	devm_kfree(&pdev->dev, drv_data->mac_datum);
	drv_data->mac_datum = NULL;

	wakeup_source_trash(&drv_data->write_lock);
	mutex_destroy(&drv_data->batt_safe_info_lock);

	devm_kfree(&pdev->dev, drv_data);
	platform_set_drvdata(pdev, NULL);
}

static void  rt_memory_release(struct platform_device *pdev)
{
	rt_drv_data_release(pdev);
#ifdef ONEWIRE_STABILITY_DEBUG
	sysfs_remove_files(&pdev->dev.kobj, g_rt_attrs);
#endif
}

static int rt_driver_probe(struct platform_device *pdev)
{
	struct rt_drv_data *drv_data = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	hwlog_info("rt: probing\n");

	drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data), GFP_KERNEL);
	if (!drv_data)
		return RT_FAIL;

	if (rt_parse_dts(pdev->dev.of_node, drv_data))
		goto free_drv_data;
	if (rt_comm_register(&drv_data->ic_des, pdev, drv_data->phy_id))
		goto free_drv_data;
	if (rt_memory_init(pdev, drv_data))
		goto destroy_com_obj;

#ifdef ONEWIRE_STABILITY_DEBUG
	if (sysfs_create_files(&pdev->dev.kobj, g_rt_attrs)) {
		hwlog_err("create debug device node error\n");
		rt_drv_data_release(pdev);
		return RT_FAIL;
	}
#endif

	drv_data->ct_node.ic_memory_release = rt_memory_release;
	drv_data->ct_node.ct_ops_register = rt_ct_ops_register;
	drv_data->ct_node.ic_dev = pdev;
	INIT_LIST_HEAD(&drv_data->ct_node.node);
	add_to_aut_ic_list(&drv_data->ct_node);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = pdev->dev.driver->name;
		power_dev_info->dev_id = 0;
		power_dev_info->ver_id = 0;
	}

	platform_set_drvdata(pdev, drv_data);
	hwlog_info("rt: probing success\n");
	return RT_SUCCESS;

destroy_com_obj:
	rt_destroy_com_obj(&drv_data->ic_des.obj, pdev);
free_drv_data:
	devm_kfree(&pdev->dev, drv_data);
	return RT_FAIL;
}

static int rt_driver_remove(struct platform_device *pdev)
{
	rt_memory_release(pdev);
	return 0;
}

static const struct of_device_id rt_match_table[] = {
	{
		.compatible = "richtek,onewire-sha3",
	},
	{ /* end */ },
};

static struct platform_driver rt_driver = {
	.probe = rt_driver_probe,
	.remove = rt_driver_remove,
	.driver = {
		.name = "richtek-a1",
		.owner = THIS_MODULE,
		.of_match_table = rt_match_table,
	},
};

int __init rt_driver_init(void)
{
	return platform_driver_register(&rt_driver);
}

void __exit rt_driver_exit(void)
{
	platform_driver_unregister(&rt_driver);
}

subsys_initcall_sync(rt_driver_init);
module_exit(rt_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei rt9430wsc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
