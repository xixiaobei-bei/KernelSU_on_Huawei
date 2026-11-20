/*
 * parse the cmdline
 *
 * Copyright (c) 2010-2020 Huawei Technologies Co., Ltd.
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
#include <linux/hisi/hisi_cmdline_parse.h>
#include <linux/of_platform.h>
#include <linux/memblock.h>
#include <linux/irqchip.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "securec.h"

/* Format of hex string: 0x12345678 */
#define RUNMODE_FLAG_NORMAL            0
#define RUNMODE_FLAG_FACTORY           1
#define SEPC_STRING_MAX                1
#define HEX_STRING_MAX                 10
#define TRANSFER_BASE                  16
#define PLATFORM_PRODUCT_ERRORID  0xFFFFFFFF
#define CAMERA_SPECS_MAX_LEN           128
#define VENDORID_BITS                  10
#define VENDORID_MASK                  ((1U << VENDORID_BITS) - 1)
#define MODEMID_MASK                   (~VENDORID_MASK)
#define CHECK_PRODUCT_OK               1
#define CHECK_PRODUCT_ERR              (-1)

static unsigned int platform_product_id = PLATFORM_PRODUCT_ERRORID;
static unsigned int runmode_factory = RUNMODE_FLAG_NORMAL;
static unsigned int sepc_info = 0;
static unsigned int uncheck_product_id = PLATFORM_PRODUCT_ERRORID;
static unsigned int cota_uncheck_product_id = PLATFORM_PRODUCT_ERRORID;

// g_camera_specs max length less than 128
static char g_camera_specs[CAMERA_SPECS_MAX_LEN] = {0};

static int __init early_parse_runmode_cmdline(char *ptr)
{
	if (ptr != NULL) {
		if (strncmp(ptr, "factory", strlen("factory")) == 0)
			runmode_factory = RUNMODE_FLAG_FACTORY;

		pr_info("runmode is %s, runmode_factory = %u\n", ptr,
			runmode_factory);
	}

	return 0;
}
early_param("androidboot.swtype", early_parse_runmode_cmdline);

/* the function interface to check factory/normal mode in kernel */
unsigned int runmode_is_factory(void)
{
	return runmode_factory;
}
EXPORT_SYMBOL(runmode_is_factory);

static int __init early_parse_camera_specs_from_cmdline(char *p)
{
	if (p == NULL) {
		pr_err("pointer of cmdline is NULL\n");
		return 0;
	}

	if (*p != '\0') {
		if (strncpy_s(g_camera_specs, sizeof(g_camera_specs),
			p, strlen(p) + 1) != 0)
			pr_err("copy camera_specs from cmdline fail\n");
		else
			pr_info("cmdline is %s, camera_specs is %s\n",
				p, g_camera_specs);
	}
	return 0;
}
early_param("camera_specs", early_parse_camera_specs_from_cmdline);

char *get_camera_specs(void)
{
	return g_camera_specs;
}
EXPORT_SYMBOL(get_camera_specs);

static int logctl_flag; /* default 0, switch logger off */
int get_logctl_value(void)
{
	return logctl_flag;
}
EXPORT_SYMBOL(get_logctl_value);

unsigned int enter_recovery_flag;
/*
 * parse boot_into_recovery cmdline which is passed from boot_recovery() of boot.c
 * Format : boot_into_recovery_flag=0 or 1
 */
static int __init early_parse_enterrecovery_cmdline(char *p)
{
	int ret;
	char enter_recovery[HEX_STRING_MAX + 1] = {0};

	if (p == NULL) {
		pr_err("%s:%d cmdline Err\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (memcpy_s(enter_recovery, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
		pr_err("%s:%d Err\n", __func__, __LINE__);
		return -EINVAL;
	}
	enter_recovery[HEX_STRING_MAX] = '\0';

	ret = kstrtouint(enter_recovery, TRANSFER_BASE, &enter_recovery_flag);
	if (ret)
		return ret;

	pr_info("enter recovery p:%s, enter_recovery_flag :%u\n", p,
			enter_recovery_flag);

	return 0;
}
early_param("enter_recovery", early_parse_enterrecovery_cmdline);

unsigned int get_boot_into_recovery_flag(void)
{
	return enter_recovery_flag;
}
EXPORT_SYMBOL(get_boot_into_recovery_flag);

unsigned int recovery_update_flag;
/*
 * Format : recovery_update=0 or 1
 */
static int __init early_parse_recovery_update_cmdline(char *p)
{
	int ret;
	char recovery_update[HEX_STRING_MAX + 1] = {0};

	if (p == NULL) {
		pr_err("%s:%d cmdline Err\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (memcpy_s(recovery_update, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
		pr_err("%s:%d Err\n", __func__, __LINE__);
		return -EINVAL;
	}
	recovery_update[HEX_STRING_MAX] = '\0';

	ret = kstrtouint(recovery_update, TRANSFER_BASE, &recovery_update_flag);
	if (ret)
		return ret;

	pr_info("recovery_update p:%s, recovery_update_flag :%u\n", p,
			recovery_update_flag);

	return 0;
}
early_param("recovery_update", early_parse_recovery_update_cmdline);

unsigned int get_recovery_update_flag(void)
{
	return recovery_update_flag;
}
EXPORT_SYMBOL(get_recovery_update_flag);

static int __init early_parse_logctl_cmdline(char *p)
{
	int ret;
	char logctl[HEX_STRING_MAX + 1] = {0};

	if (p == NULL) {
		logctl_flag = 0; /* error parameter, switch logger off */
		pr_err("%s: setup_logctl parameter error!\n", __func__);
		return 0;
	}

	if (memcpy_s(logctl, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
		pr_err("%s:%d Err\n", __func__, __LINE__);
		return -EINVAL;
	}

	ret = kstrtoint(logctl, TRANSFER_BASE, &logctl_flag);
	if (ret)
		return ret;

	pr_info("%s: p: %s, logctl: %d\n", __func__, p, logctl_flag);

	return 0;
}
early_param("setup_logctl", early_parse_logctl_cmdline);

/*
 * 0: battery is normal;
 * 1: battery is low;
 */
static int g_lowbattery;

int get_low_battery_flag(void)
{
	return g_lowbattery;
}

void set_low_battery_flag(int flag)
{
	g_lowbattery = flag;
}

static int __init early_parse_lowpower_cmdline(char *p)
{
	int ret;
	char clow[HEX_STRING_MAX + 1] = {0};

	if (p == NULL) {
		g_lowbattery = 0; /* error parameter, switch logger off */
		pr_err("%s input is NULL\n", __func__);
		return 0;
	}

	if (memcpy_s(clow, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
		pr_err("%s:%d Err\n", __func__, __LINE__);
		return -EINVAL;
	}

	ret = kstrtoint(clow, TRANSFER_BASE, &g_lowbattery);
	if (ret)
		return ret;

	pr_info("%s %d\n", __func__, g_lowbattery);
	return 0;
}
early_param("low_volt_flag", early_parse_lowpower_cmdline);

static int need_dsm_notify = 1;
int get_dsm_notify_flag(void)
{
	return need_dsm_notify;
}
EXPORT_SYMBOL(get_dsm_notify_flag);

static int __init early_parse_dsm_notify_cmdline(char *p)
{
	int ret;
	char clow[HEX_STRING_MAX + 1] = {0};

	if (p == NULL) {
		need_dsm_notify = 1; /* error parameter, dsm notify */
		pr_err("%s input is NULL\n", __func__);
		return 0;
	}

	if (memcpy_s(clow, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
		pr_err("%s clow copy fail\n", __func__);
		return -1;
	}

	ret = kstrtoint(clow, TRANSFER_BASE, &need_dsm_notify);
	if (ret)
		return ret;

	pr_info("%s %d\n", __func__, need_dsm_notify);
	return 0;
}
early_param("dsm_notify", early_parse_dsm_notify_cmdline);

/*
 * parse powerdown charge cmdline which is passed from fastoot.
 * Format : pd_charge=0 or 1
 */
static unsigned int pd_charge_flag;
static int __init early_parse_pdcharge_cmdline(char *p)
{
	if (p != NULL) {
		if (strncmp(p, "charger", strlen("charger")) == 0)
			pd_charge_flag = 1;
		else
			pd_charge_flag = 0;

		pr_info("power down charge p:%s, pd_charge_flag :%u\n", p,
				pd_charge_flag);
	}

	return 0;
}
early_param("androidboot.mode", early_parse_pdcharge_cmdline);

unsigned int get_pd_charge_flag(void)
{
	return pd_charge_flag;
}
EXPORT_SYMBOL(get_pd_charge_flag);

#if CONFIG_HISI_NVE_WHITELIST
static unsigned int userlock;
static int __init early_parse_userlock_cmdline(char *p)
{
	if (p != NULL) {
		if (strncmp(p, "locked", strlen("locked")) == 0)
			userlock = 1;
		else
			userlock = 0;

		pr_info("userlock p:%s, userlock :%u\n", p, userlock);
	}

	return 0;
}
early_param("userlock", early_parse_userlock_cmdline);

unsigned int get_userlock(void)
{
	return userlock;
}
EXPORT_SYMBOL(get_userlock);
#endif

unsigned int bsp_need_loadmodem(void)
{
	unsigned int rs = 0;

	rs |= get_recovery_update_flag();
	return !rs;
}
EXPORT_SYMBOL(bsp_need_loadmodem);

unsigned int is_load_modem(void)
{
	unsigned int rs = 0;

	rs |= get_recovery_update_flag();
	rs |= get_pd_charge_flag();
	return !rs;
}
EXPORT_SYMBOL(is_load_modem);


/*
 * Description: Save platform product id to kernel.
 *              The product id passed by cmd line from bootloader.
 * Input:
 *        char *p -- cmd line node
 * Return:
 *        always 0
 */
static int __init early_parse_product_id(char *p)
{
	int ret;
	char input_id[HEX_STRING_MAX + 1] = {0};

	if (p != NULL) {
		if (memcpy_s(input_id, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
			pr_err("%s:%d Err\n", __func__, __LINE__);
			return -EINVAL;
		}

		ret = kstrtouint(input_id, TRANSFER_BASE, &platform_product_id);
		if (ret)
			platform_product_id = PLATFORM_PRODUCT_ERRORID;

		pr_info("input product id:%s, parsed product id:0X%x\n", p, platform_product_id);
	}

	return 0;
}
early_param("productid", early_parse_product_id);

/*
 * Description: Get platform product id.
 *              The product id passed by cmd line from bootloader.
 * Input:
 *        void
 * Return:
 *        unsigned in product id
 *        error   -- return 0xFFFFFFFF
 *        success -- return not 0xFFFFFFFF
 */
unsigned int get_cmd_product_id(void)
{
	return platform_product_id;
}
EXPORT_SYMBOL(get_cmd_product_id);

/*
 * Description: Save sepc info to kernel.
 *              The sepc info passed by cmd line from bootloader.
 * Input:
 *        char *p -- cmd line node
 * Return:
 *        sepc info
 */
static int __init early_parse_sepc_info(char *p)
{
	int ret;
	char spec_info_update[SEPC_STRING_MAX + 1] = {0};

	if (p == NULL) {
		pr_err("%s:%d cmdline Err\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (memcpy_s(spec_info_update, SEPC_STRING_MAX + 1, p, SEPC_STRING_MAX) != EOK) {
		pr_err("%s:%d Err\n", __func__, __LINE__);
		return -EINVAL;
	}
	spec_info_update[SEPC_STRING_MAX] = '\0';

	ret = kstrtouint(spec_info_update, TRANSFER_BASE, &sepc_info);
	if (ret) {
		sepc_info = 0;
		return ret;
	}

	pr_info("spec_info_update p:%s, spec_info :%u\n", p,
			sepc_info);

	return 0;
}
early_param("sepc_info", early_parse_sepc_info);

/*
 * Description: Get sepc info.
 *              The seoc info passed by cmd line from bootloader.
 * Input:
 *        void
 * Return:
 *        unsigned int spec info
 *        error   -- return 0
 *        success -- return not 0
 */
unsigned int get_cmd_sepc_info(void)
{
	return sepc_info;
}

/*
 * Description: early_parse__uncheck_cota_product_id.
 *        The uncheck cota productid passed by cmd line from bootloader.
 * Input:
 *        char *p -- cmd line node
 * Return:
 *        unsigned int spec info
 *        error   -- return 0
 *        success -- return not 0
 */
static int __init early_parse__uncheck_cota_product_id(char *p)
{
	int ret;
	char input_id[HEX_STRING_MAX + 1] = {0};

	if (p != NULL) {
		if (memcpy_s(input_id, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
			pr_err("%s:%d Err\n", __func__, __LINE__);
			return -EINVAL;
		}

		ret = kstrtouint(input_id, TRANSFER_BASE, &cota_uncheck_product_id);
		if (ret)
			cota_uncheck_product_id = PLATFORM_PRODUCT_ERRORID;

		pr_info("input product id:%s, parsed cota product id:0X%x\n",
			p, cota_uncheck_product_id);
	}

	return 0;
}
early_param("cota_productid_unchecked", early_parse__uncheck_cota_product_id);

/*
 * Description: early_parse_uncheck_product_id.
 *        The uncheck  productid passed by cmd line from bootloader.
 * Input:
 *        char *p -- cmd line node
 * Return:
 *        unsigned int spec info
 *        error   -- return 0
 *        success -- return not 0
 */
static int __init early_parse_uncheck_product_id(char *p)
{
	int ret;
	char input_id[HEX_STRING_MAX + 1] = {0};

	if (p != NULL) {
		if (memcpy_s(input_id, HEX_STRING_MAX + 1, p, HEX_STRING_MAX) != EOK) {
			pr_err("%s:%d Err\n", __func__, __LINE__);
			return -EINVAL;
		}

		ret = kstrtouint(input_id, TRANSFER_BASE, &uncheck_product_id);
		if (ret)
			uncheck_product_id = PLATFORM_PRODUCT_ERRORID;

		pr_info("input product id:%s, parsed product id:0X%x\n",
			p, uncheck_product_id);
	}

	return 0;
}
early_param("productid_uncehcked", early_parse_uncheck_product_id);

/*
 * Description: check_productid
 *        check produt id is in deigned list or not
 * Input:
 *        int product_id -- productid need check
 *        int *prdt_list -- valid productid list
 *        int length ---- number of valid productid list
 * Return:
 *        error   -- return -1
 *        success -- return 1
 */
int check_productid(int product_id, const int *prdt_list, unsigned int length)
{
	unsigned int index = 0;

	while (index < length) {
		if (product_id == (unsigned int) prdt_list[index])
			return CHECK_PRODUCT_OK;
		index++;
	}
	return CHECK_PRODUCT_ERR;
}

/*
 * Description: get_fin_product_id
 *        get productid
 * Input:
 *        int *prdt_list -- valid productid list
 *        int length ---- number of valid productid list
 * Return:
 *        productid
 */
int get_fin_product_id(const int *prdt_list, unsigned int length)
{
	if (prdt_list == NULL || length == 0) {
		return PLATFORM_PRODUCT_ERRORID;
	}
	if (check_productid(cota_uncheck_product_id, prdt_list, length)
		== CHECK_PRODUCT_OK) {
		return cota_uncheck_product_id;
	} else if (check_productid(uncheck_product_id, prdt_list, length)
		== CHECK_PRODUCT_OK) {
		return uncheck_product_id;
	} else if (check_productid(uncheck_product_id & MODEMID_MASK,
		prdt_list, length) == CHECK_PRODUCT_OK) {
		return uncheck_product_id & MODEMID_MASK;
	} else {
		pr_err("%s:%d productid not found\n", __func__, __LINE__);
		return PLATFORM_PRODUCT_ERRORID;
	}
}
EXPORT_SYMBOL(get_fin_product_id);
