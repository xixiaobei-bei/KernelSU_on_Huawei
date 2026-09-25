/*
 * battery_early_param.c
 *
 * battery early param.
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

#include "batt_early_param.h"
#include <huawei_platform/power/power_mesg_srv.h>

static struct batt_ic_para  g_ic_para = { 0 };
static struct batt_info_para  g_info_para = { 0 };
static struct batt_cert_para  g_cert_para = { 0 };
static char g_nv_sn[MAX_SN_LEN] = { 0 };

static int __init batt_parse_nv_sn_cmdline(char *p)
{
	int strp_len;

	if (!p)
		return -1;

	strp_len = strnlen(p, MAX_SN_LEN - 1);
	memcpy(g_nv_sn, p, strp_len);
	g_nv_sn[strp_len] = 0;
	return 0;
}
early_param("battery_nv_sn", batt_parse_nv_sn_cmdline);

static int batt_hexstr_to_array(char *p, char *hex, int hex_len)
{
	int hex_index = 0;
	int p_pos = 0;
	int p_len = strlen(p);
	char temp[BATT_HEX_STR_LEN] = { 0 };

	if (((p_len % (BATT_HEX_STR_LEN - 1)) != 0) ||
		((p_len / (BATT_HEX_STR_LEN - 1)) > hex_len))
		return -1;

	while (p_pos < p_len) {
		memcpy(temp, p + p_pos, BATT_HEX_STR_LEN - 1);
		temp[BATT_HEX_STR_LEN - 1] = 0;

		/* 16: decimal base */
		if (kstrtou8(temp, 16, &hex[hex_index]) != 0)
			return -1;

		p_pos += BATT_HEX_STR_LEN - 1;
		hex_index++;
	}

	return hex_index;
}

static int __init batt_parse_ic_info_cmdline(char *p)
{
	char *token = NULL;

	if (!p)
		return -1;

	memset(&g_ic_para, 0, sizeof(struct batt_ic_para));
	token = strsep(&p, ",");
	if (!token)
		goto parse_fail;

	/* 16: decimal base */
	if (kstrtoint(token, 16, &g_ic_para.ic_type) != 0)
		goto parse_fail;

	g_ic_para.uid_len = batt_hexstr_to_array(p,
		g_ic_para.uid, BATT_MAX_UID_LEN);
	if (g_ic_para.uid_len == 0)
		goto parse_fail;
	return 0;

parse_fail:
	g_ic_para.ic_type = LOCAL_IC_TYPE;
	return -1;
}
early_param("battery_ic", batt_parse_ic_info_cmdline);

static int __init batt_parse_info_cmdline(char *p)
{
	int value;
	char *token = NULL;

	if (!p)
		return -1;

	memset(&g_info_para, 0, sizeof(struct batt_info_para));
	token = strsep(&p, ",");
	if (!token)
		return -1;
	memcpy(g_info_para.type, token,
		strnlen(token, BATTERY_TYPE_BUFF_SIZE - 1));

	token = strsep(&p, ",");
	if (!token)
		return -1;
	/* 16: decimal base */
	if (kstrtoint(token, 16, &value) != 0)
		return -1;
	g_info_para.source = value;
	memcpy(g_info_para.sn, p, strnlen(p, MAX_SN_LEN - 1));
	return 0;
}
early_param("battery_info", batt_parse_info_cmdline);

static int __init batt_parse_cert_cmdline(char *p)
{
	int value;
	char *token = NULL;

	if (!p)
		return -1;

	memset(&g_cert_para, 0, sizeof(struct batt_cert_para));
	token = strsep(&p, ",");
	if (!token)
		return -1;

	/* 16: decimal base */
	if (kstrtoint(token, 16, &value) != 0)
		return -1;
	g_cert_para.key_result = value;

	g_cert_para.sign_len = batt_hexstr_to_array(p,
		g_cert_para.signature, BATT_MAX_SIGN_LEN);
	g_cert_para.valid_sign = 1;
	return 0;
}
early_param("battery_cert", batt_parse_cert_cmdline);

struct batt_ic_para *batt_get_ic_para(void)
{
	if (g_ic_para.ic_type == LOCAL_IC_TYPE)
		return NULL;

	return &g_ic_para;
}

struct batt_info_para *batt_get_info_para(void)
{
	if (g_ic_para.ic_type == LOCAL_IC_TYPE)
		return NULL;

	return &g_info_para;
}

struct batt_cert_para *batt_get_cert_para(void)
{
	if (!g_cert_para.valid_sign)
		return NULL;

	return &g_cert_para;
}

char *batt_get_nv_sn(void)
{
	if (strnlen(g_nv_sn, MAX_SN_LEN - 1) == 0)
		return NULL;

	return g_nv_sn;
}