/*
 * rdr_common.c
 *
 * blackbox common functions moudle
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/export.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/hisi/hisi_log.h>
#include <securec.h>

#define HISI_LOG_TAG HISI_BLACKBOX_TAG

#include <linux/hisi/rdr_pub.h>
#include <mntn_subtype_exception.h>
#include "rdr_print.h"
#include "rdr_inner.h"
#include "rdr_field.h"
#include "rdr_print.h"
#include "rdr_debug.h"


#undef __REBOOT_REASON_MAP
#define __REBOOT_REASON_MAP(x, y) { #x, x, #y, y },
struct cmdword reboot_reason_map[] = {
	#include <mntn_reboot_reason_map.h>
};
#undef __REBOOT_REASON_MAP

#define HIMNTN_FIRST_NV_DATA 0
#define HIMNTN_SECOND_NV_DATA 1
#define HIMNTN_THIRD_NV_DATA 2
#define HIMNTN_FORTH_NV_DATA 3
#define HIMNTN_FIFTH_NV_DATA 4
#define HIMNTN_SIXTH_NV_DATA 5
#define RDR_DUMPCTRL_LENGTH 16

/*
 * reference to:
 *   http://3ms.huawei.com/hi/group/2527/wiki_4011227.html?for_statistic_from=creation_group_wiki
 */
/*
 * return:
 *     unsigned int: edition information
 *     0x01          EDITION_USER
 *     0x02          EDITION_INTERNAL_BETA
 *     0x03          EDITION_OVERSEA_BETA
 */
unsigned int bbox_check_edition(void)
{
	char tmp;
	unsigned int type;
	int fd = -1;
	long cnt;
	unsigned int dump_flag;

	fd = (int)sys_open(FILE_EDITION, O_RDONLY, FILE_LIMIT);
	if (fd < 0) {
		BB_PRINT_ERR("[%s]: open %s failed, return [%d]\n", __func__,
			FILE_EDITION, fd);
		return EDITION_USER;
	}

	cnt = sys_read((unsigned int)fd, &tmp, sizeof(tmp));
	if (cnt < 0) {
		BB_PRINT_ERR("[%s]: read %s failed, return [%ld]\n",
			__func__, FILE_EDITION, cnt);
		dump_flag = EDITION_USER;
		goto out;
	}

	if (tmp >= START_CHAR_0 && tmp <= END_CHAR_9) {
		type = (unsigned int)(unsigned char)(tmp - START_CHAR_0);

		if (type == OVERSEA_USER) {
			BB_PRINT_PN("%s: The edition is Oversea BETA, type is %#x\n", __func__, type);
			dump_flag = EDITION_OVERSEA_BETA;
		} else if (type == BETA_USER) {
			BB_PRINT_PN("%s: The edition is Internal BETA, type is %#x\n", __func__, type);
			dump_flag = EDITION_INTERNAL_BETA;
		} else if (type == COMMERCIAL_USER) {
			BB_PRINT_PN("%s: The edition is Commercial User, type is %#x\n", __func__, type);
			dump_flag = EDITION_USER;
		} else {
			BB_PRINT_PN("%s: The edition is default User, type is %#x\n", __func__, type);
			dump_flag = EDITION_USER;
		}
	} else {
		BB_PRINT_ERR("%s: The edition is default User, please check %s\n", __func__, FILE_EDITION);
		dump_flag = EDITION_USER;
	}

out:
	sys_close((unsigned int)fd);
	return dump_flag;
}
#define TIMELEN 8
#define DATELEN 11
/* Kernel native interface in version.c, but not declared in version.h */
extern int get_kernel_build_time(char *blddt, int dtlen, char *bldtm, int tmlen);

void rdr_get_builddatetime(u8 *out, u32 out_len)
{
	u8 *pout = out;
	u8 *p = NULL;
	u8 date[DATELEN + 1] = {0};
	u8 time[TIMELEN + 1] = {0};
	int cnt = RDR_BUILD_DATE_TIME_LEN;
	int ret;

	if (out == NULL) {
		BB_PRINT_ERR("[%s], out is null!\n", __func__);
		return;
	}

	if (out_len < RDR_BUILD_DATE_TIME_LEN) {
		BB_PRINT_ERR("[%s],out_len is too small!\n", __func__);
		return;
	}

	(void)memset_s((void *)out, out_len, 0, out_len);

	ret = get_kernel_build_time(date, DATELEN, time, TIMELEN);
	if (ret) {
		BB_PRINT_ERR("[%s], get kernel build time failed!\n", __func__);
		goto error;
	}
	date[DATELEN] = '\0';
	time[TIMELEN] = '\0';

	p = date;
	while (*p) {
		if (!cnt)
			goto error;
		if (*p != ' ') {
			*pout++ = *p++;
			cnt--;
		} else {
			p++;
		}
	}

	p = time;
	while (*p) {
		if (!cnt)
			goto error;
		if (*p != ':') {
			*pout++ = *p++;
			cnt--;
		} else {
			p++;
		}
	}

error:
	out[RDR_BUILD_DATE_TIME_LEN - 1] = '\0';
}

u64 rdr_get_tick(void)
{
	/* use only one int value to save time: */
	struct timespec uptime;
#if (KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE)
	do_posix_clock_monotonic_gettime(&uptime);
#else
	ktime_get_ts(&uptime);
#endif
	get_monotonic_boottime(&uptime);
	return (u64)uptime.tv_sec;
}

char *rdr_get_timestamp(void)
{
	struct rtc_time tm;
	struct timeval tv;
	static char databuf[DATA_MAXLEN + 1];

	BB_PRINT_START();

	(void)memset_s(databuf, DATA_MAXLEN + 1, 0, DATA_MAXLEN + 1);

	(void)memset_s(&tv, sizeof(tv), 0, sizeof(tv));

	(void)memset_s(&tm, sizeof(tm), 0, sizeof(tm));

	do_gettimeofday(&tv);
	tv.tv_sec -= (long)sys_tz.tz_minuteswest * 60;
	rtc_time_to_tm(tv.tv_sec, &tm);

	(void)snprintf(databuf, DATA_MAXLEN + 1, "%04d%02d%02d%02d%02d%02d",
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);

	BB_PRINT_DBG("rdr: %s [%s] !\n", __func__, databuf);
	BB_PRINT_END();
	return databuf;
}

struct blackbox_modid_list g_modid_list[] = {
	{ HISI_BB_MOD_MODEM_DRV_START, HISI_BB_MOD_MODEM_DRV_END, "MODEM DRV" },
	{ HISI_BB_MOD_MODEM_OSA_START, HISI_BB_MOD_MODEM_OSA_END, "MODEM OSA" },
	{ HISI_BB_MOD_MODEM_OM_START, HISI_BB_MOD_MODEM_OM_END, "MODEM OM" },
	{ HISI_BB_MOD_MODEM_GU_L2_START, HISI_BB_MOD_MODEM_GU_L2_END, "MODEM GU L2" },
	{ HISI_BB_MOD_MODEM_GU_WAS_START, HISI_BB_MOD_MODEM_GU_WAS_END, " MODEM GU WAS" },
	{ HISI_BB_MOD_MODEM_GU_GAS_START, HISI_BB_MOD_MODEM_GU_GAS_END, "MODEM GU GAS" },
	{ HISI_BB_MOD_MODEM_GU_NAS_START, HISI_BB_MOD_MODEM_GU_NAS_END, "MODEM GU NAS" },
	{ HISI_BB_MOD_MODEM_GU_DSP_START, HISI_BB_MOD_MODEM_GU_DSP_END, "MODEM GU DSP" },
	{ (unsigned int)HISI_BB_MOD_AP_START, (unsigned int)HISI_BB_MOD_AP_END, "ap" },
	{ (unsigned int)HISI_BB_MOD_FASTBOOT_START, (unsigned int)HISI_BB_MOD_FASTBOOT_END, "fastboot" },
	{ (unsigned int)HISI_BB_MOD_ISP_START, (unsigned int)HISI_BB_MOD_ISP_END, "isp" },
	{ (unsigned int)HISI_BB_MOD_EMMC_START, (unsigned int)HISI_BB_MOD_EMMC_END, "emmc" },
	{ (unsigned int)HISI_BB_MOD_CP_START, (unsigned int)HISI_BB_MOD_CP_END, "cp" },
	{ (unsigned int)HISI_BB_MOD_TEE_START, (unsigned int)HISI_BB_MOD_TEE_END, "tee" },
	{ (unsigned int)HISI_BB_MOD_HIFI_START, (unsigned int)HISI_BB_MOD_HIFI_END, "hifi" },
	{ (unsigned int)HISI_BB_MOD_LPM_START, (unsigned int)HISI_BB_MOD_LPM_END, "lpm3" },
	{ (unsigned int)HISI_BB_MOD_IOM_START, (unsigned int)HISI_BB_MOD_IOM_END, "iom3" },
	{ (u32)HISI_BB_MOD_HISEE_START, (u32)HISI_BB_MOD_HISEE_END, "hisee" },
	{ (unsigned int)HISI_BB_MOD_IVP_START, (unsigned int)HISI_BB_MOD_IVP_END, "ivp" },
	{ (unsigned int)HISI_BB_MOD_BFM_START, (unsigned int)HISI_BB_MOD_BFM_END, "bfm" },
	{ (unsigned int)HISI_BB_MOD_RESERVED_START, (unsigned int)HISI_BB_MOD_RESERVED_END,
	  "blackbox reserved" },
	{ (unsigned int)HISI_BB_MOD_MODEM_LPS_START, (unsigned int)HISI_BB_MOD_MODEM_LPS_END, "MODEM LPS" },
	{ (unsigned int)HISI_BB_MOD_MODEM_LMSP_START, (unsigned int)HISI_BB_MOD_MODEM_LMSP_END,
	  "MODEM LMSP" },
	{ (unsigned int)HISI_BB_MOD_NPU_START, (unsigned int)HISI_BB_MOD_NPU_END, "NPU" },
	{ (unsigned int)HISI_BB_MOD_CONN_START, (unsigned int)HISI_BB_MOD_CONN_END, "CONN" },
	/* delow end */
	{ (unsigned int)HISI_BB_MOD_RANDOM_ALLOCATED_START, (unsigned int)HISI_BB_MOD_RANDOM_ALLOCATED_END,
	  "blackbox random allocated" },
	{ (unsigned int)HISI_BB_MOD_DSS_START, (unsigned int)HISI_BB_MOD_DSS_END, "DSS" },
};

struct cmdword *get_reboot_reason_map(void)
{
	return reboot_reason_map;
}

u32 get_reboot_reason_map_size(void)
{
	return (u32)ARRAY_SIZE(reboot_reason_map);
}

/*
 * func description:
 *    get exception core str for core id
 * func args:
 *    u64 coreid
 * return value
 *    NULL  error
 *    !NULL core str
 */
char *rdr_get_exception_type(u64 e_exce_type)
{
	int i;

	for (i = 0; (unsigned int)i < get_reboot_reason_map_size(); i++) {
		if (reboot_reason_map[i].num == e_exce_type)
			return (char *)reboot_reason_map[i].name;
	}

	return "UNDEF";
}

enum EXCEPTION_CORE_LIST {
	EXCEPTION_CORE_AP,
	EXCEPTION_CORE_CP,
	EXCEPTION_CORE_TEEOS,
	EXCEPTION_CORE_HIFI,
	EXCEPTION_CORE_LPM3,
	EXCEPTION_CORE_IOM3,
	EXCEPTION_CORE_ISP,
	EXCEPTION_CORE_IVP,
	EXCEPTION_CORE_EMMC,
	EXCEPTION_CORE_MODEMAP,
	EXCEPTION_CORE_CLK,
	EXCEPTION_CORE_REGULATOR,
	EXCEPTION_CORE_BFM,
	EXCEPTION_CORE_HISEE,
	EXCEPTION_CORE_NPU,
	EXCEPTION_CORE_CONN,
	EXCEPTION_CORE_TRACE,
	EXCEPTION_CORE_DSS,
	EXCEPTION_CORE_UNDEF,
};

char *exception_core[RDR_CORE_MAX + 1] = {
	"AP",
	"CP",
	"TEEOS",
	"HIFI",
	"LPM3",
	"IOM3",
	"ISP",
	"IVP",
	"EMMC",
	"MODEMAP",
	"CLK",
	"REGULATOR",
	"BFM",
	"HISEE",
	"NPU",
	"CONN",
	"EXCEPTION_TRACE",
	"DSS",
	"UNDEF",
};

/*
 * Description:   get description of exception by core id
 * Return:        value:NULL error,!NULL core string
 */
char *rdr_get_exception_core(u64 coreid)
{
	char *core = NULL;

	switch (coreid) {
	case RDR_AP:
		core = exception_core[EXCEPTION_CORE_AP];
		break;
	case RDR_CP:
		core = exception_core[EXCEPTION_CORE_CP];
		break;
	case RDR_TEEOS:
		core = exception_core[EXCEPTION_CORE_TEEOS];
		break;
	case RDR_HIFI:
		core = exception_core[EXCEPTION_CORE_HIFI];
		break;
	case RDR_LPM3:
		core = exception_core[EXCEPTION_CORE_LPM3];
		break;
	case RDR_IOM3:
		core = exception_core[EXCEPTION_CORE_IOM3];
		break;
	case RDR_ISP:
		core = exception_core[EXCEPTION_CORE_ISP];
		break;
	case RDR_IVP:
		core = exception_core[EXCEPTION_CORE_IVP];
		break;
	case RDR_EMMC:
		core = exception_core[EXCEPTION_CORE_EMMC];
		break;
	case RDR_MODEMAP:
		core = exception_core[EXCEPTION_CORE_MODEMAP];
		break;
	case RDR_CLK:
		core = exception_core[EXCEPTION_CORE_CLK];
		break;
	case RDR_REGULATOR:
		core = exception_core[EXCEPTION_CORE_REGULATOR];
		break;
	case RDR_BFM:
		core = exception_core[EXCEPTION_CORE_BFM];
		break;
	case RDR_HISEE:
		core = exception_core[EXCEPTION_CORE_HISEE];
		break;
	case RDR_NPU:
		core = exception_core[EXCEPTION_CORE_NPU];
		break;
	case RDR_CONN:
		core = exception_core[EXCEPTION_CORE_CONN];
		break;
	case RDR_EXCEPTION_TRACE:
		core = exception_core[EXCEPTION_CORE_TRACE];
		break;
	case RDR_DSS:
		core = exception_core[EXCEPTION_CORE_DSS];
		break;
	default:
		core = exception_core[EXCEPTION_CORE_UNDEF];
		break;
	}
	return core;
}

atomic_t volatile bb_in_suspend = ATOMIC_INIT(0);
atomic_t volatile bb_in_reboot = ATOMIC_INIT(0);
atomic_t volatile bb_in_saving = ATOMIC_INIT(0);

int rdr_get_suspend_state(void)
{
	return atomic_read(&bb_in_suspend);
}

int rdr_get_reboot_state(void)
{
	return atomic_read(&bb_in_reboot);
}

int rdr_set_saving_state(int state)
{
	return atomic_set(&bb_in_saving, state);
}

#ifdef CONFIG_PM
static struct notifier_block bb_suspend_notifier;
static int bb_suspend_nb(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		BB_PRINT_DBG("%s: resume +\n", __func__);
		atomic_set(&bb_in_suspend, 0);
		BB_PRINT_DBG("%s: resume -\n", __func__);
		break;

	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		BB_PRINT_DBG("%s: suspend +\n", __func__);
		atomic_set(&bb_in_suspend, 1);
		while (1) {
			if (atomic_read(&bb_in_saving))
				msleep(1000);
			else
				break;
		}
		BB_PRINT_DBG("%s: suspend -\n", __func__);
		break;
	default:
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}
#endif

static int bb_reboot_nb(struct notifier_block *nb, unsigned long foo, void *bar)
{
	int i = 10;
	/* prevent access the emmc now: */
	BB_PRINT_PN("%s: shutdown +\n", __func__);
	atomic_set(&bb_in_reboot, 1);
	while (i--) {
		if (atomic_read(&bb_in_saving))
			msleep(1000);
		else
			break;
		BB_PRINT_DBG("rdr:is saving rdr, wait 1s\n");
	}
	rdr_field_reboot_done();
	BB_PRINT_PN("%s: shutdown -\n", __func__);

	return 0;
}

static u64 reserved_rdr_phymem_addr;
static u64 reserved_rdr_phymem_size;
static RDR_NVE g_nve;
static u64 g_max_logsize;
enum RDR_DTS_DATA_INDX {
	MAX_LOGNUM = 0,
	DUMPLOG_TIMEOUT,
	REBOOT_TIMES,
	DIAGINFO_SIZE,
	RDR_DTS_U32_NUMS,
};

struct rdr_dts_prop {
	int indx;
	const char *propname;
	u32 data;
} g_rdr_dts_data[RDR_DTS_U32_NUMS] = {
	{ MAX_LOGNUM,        "rdr-log-max-nums", },
	{ DUMPLOG_TIMEOUT,   "wait-dumplog-timeout", },
	{ REBOOT_TIMES,      "unexpected-max-reboot-times", },
	{ DIAGINFO_SIZE,     "rdr-diaginfo-size", },
};

u32 rdr_get_reboot_times(void)
{
	return g_rdr_dts_data[REBOOT_TIMES].data;
}

u64 rdr_reserved_phymem_addr(void)
{
	return reserved_rdr_phymem_addr;
}

u64 rdr_reserved_phymem_size(void)
{
	return reserved_rdr_phymem_size;
}

int rdr_get_dumplog_timeout(void)
{
	return g_rdr_dts_data[DUMPLOG_TIMEOUT].data;
}

u64 rdr_get_logsize(void)
{
	return g_max_logsize;
}

u32 rdr_get_diaginfo_size(void)
{
	return g_rdr_dts_data[DIAGINFO_SIZE].data;
}

RDR_NVE rdr_get_nve(void)
{
	return g_nve;
}


u32 rdr_get_lognum(void)
{
	return g_rdr_dts_data[MAX_LOGNUM].data;
}

char *blackbox_get_modid_str(u32 modid)
{
	u32 i;
	u32 modid_size = ARRAY_SIZE(g_modid_list);

	for (i = 0; i < modid_size; ++i) {
		if (modid >= g_modid_list[i].modid_span_little &&
		    modid <= g_modid_list[i].modid_span_big)
			return g_modid_list[i].modid_str;
	}

	return "error,modid not found";
}

static int rdr_get_property_data_u32(struct device_node *np)
{
	u32 value = 0;
	int i, ret;

	if (np == NULL) {
		BB_PRINT_ERR("[%s], parameter device_node np is NULL!\n", __func__);
		return -1;
	}

	for (i = 0; i < RDR_DTS_U32_NUMS; i++) {
		ret = of_property_read_u32(np, g_rdr_dts_data[i].propname, &value);
		if (ret) {
			BB_PRINT_ERR("[%s], cannot find g_rdr_dts_data[%d],[%s] in dts!\n",
			    __func__, i, g_rdr_dts_data[i].propname);
			return ret;
		}

		g_rdr_dts_data[i].data = value;
		BB_PRINT_DBG("[%s], get %s [0x%x] in dts!\n", __func__,
		    g_rdr_dts_data[i].propname, value);
	}

	return 0;
}

int rdr_common_early_init(void)
{
	int i, ret, len;
	struct device_node *np = NULL;
	struct resource res;
	const char *prdr_dumpctrl = NULL;
	struct rdr_area_data rdr_area_data;

	rdr_area_data.bbox_addr = NULL;
	rdr_area_data.value = 0;


	np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr");
	if (np == NULL) {
		BB_PRINT_ERR("[%s], find rdr_memory node fail!\n", __func__);
		return -ENODEV;
	}

	rdr_area_data.bbox_addr = of_parse_phandle(np, "bbox_addr", 0);
	if (rdr_area_data.bbox_addr == NULL) {
		BB_PRINT_ERR("[%s], no bbox_addr phandle\n", __func__);
		return -ENODEV;
	}

	ret = of_address_to_resource(rdr_area_data.bbox_addr, 0, &res);
	of_node_put(rdr_area_data.bbox_addr);
	if (ret) {
		BB_PRINT_ERR("failed to translate bbox_addr to resource: %d\n", ret);
		return ret;
	}

	reserved_rdr_phymem_addr = res.start;
	reserved_rdr_phymem_size = resource_size(&res);

	BB_PRINT_DBG("[%s], get phymem addr [%llx] size [%llx]in dts!\n",
	    __func__, reserved_rdr_phymem_addr, reserved_rdr_phymem_size);

	ret = of_property_read_string(np, "rdr-dumpctl", &prdr_dumpctrl);
	if (ret < 0 || prdr_dumpctrl == NULL || strlen(prdr_dumpctrl) > RDR_DUMPCTRL_LENGTH) {
		BB_PRINT_ERR("[%s], find rdr-dumpctl node fail! [%s]\n", __func__, prdr_dumpctrl);
		return ret;
	}
	BB_PRINT_DBG("[%s], get prdr_dumpctrl [%s] in dts!\n", __func__, prdr_dumpctrl);
	g_nve = 0;
	len = strlen(prdr_dumpctrl);
	for (i = --len; i >= 0; i--) {
		if (prdr_dumpctrl[i] == '1')
			g_nve |= (u64)1 << (unsigned int)(len - i);
	}
	BB_PRINT_DBG("[%s], get nve [0x%llx] in dts!\n", __func__, g_nve);
	ret = of_property_read_u32(np, "rdr-log-max-size", &rdr_area_data.value);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find rdr-log-max-size in dts!\n", __func__);
		return ret;
	}
	g_max_logsize = rdr_area_data.value;
	BB_PRINT_DBG("[%s], get rdr-log-max-size [0x%llx] in dts!\n", __func__, g_max_logsize);

	ret = rdr_get_property_data_u32(np);
	if (ret < 0)
		return ret;

	ret = of_property_read_u32(np, "rdr_area_num", &rdr_area_data.value);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find rdr_area_num in dts!\n", __func__);
		return ret;
	}
	BB_PRINT_DBG("[%s], get rdr_area_num [0x%x] in dts!\n", __func__, rdr_area_data.value);

	if (rdr_area_data.value > RDR_CORE_MAX) {
		BB_PRINT_ERR("[%s], invaild core num in dts!\n", __func__);
		return -1;
	}
	ret = of_property_read_u32_array(np, "rdr_area_sizes", &rdr_area_data.data[0], rdr_area_data.value);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find rdr_area_sizes in dts!\n", __func__);
		return ret;
	}
	rdr_set_area_info(0, rdr_area_data.value);
	for (i = 1; i < (int)rdr_area_data.value; i++) {
		rdr_set_area_info(i, rdr_area_data.data[i]);
		BB_PRINT_DBG("[%s], get rdr_area_num[%d]:[0x%x] in dts!\n", __func__, i, rdr_area_data.data[i]);
	}

	return ret;
}

static struct notifier_block bb_reboot_notifier;
int rdr_common_init(void)
{
#ifdef CONFIG_PM
	/* Register to get PM events */
	bb_suspend_notifier.notifier_call = bb_suspend_nb;
	bb_suspend_notifier.priority = -1;
	if (register_pm_notifier(&bb_suspend_notifier)) {
		BB_PRINT_ERR("%s: Failed to register for PM events\n",
		    __func__);
		return -1;
	}
#endif

	bb_reboot_notifier.notifier_call = bb_reboot_nb;
	bb_reboot_notifier.priority = -1;
	if (register_reboot_notifier(&bb_reboot_notifier)) {
		BB_PRINT_ERR("%s: Failed to register for Reboot events\n",
		    __func__);
		return -1;
	}
	return 0;
}

static char g_reboot_reason[RDR_REBOOT_REASON_LEN] = "undef";
static u32 g_reboot_type;
char *rdr_get_reboot_reason(void)
{
	return g_reboot_reason;
}

u32 rdr_get_reboot_type(void)
{
	return g_reboot_type;
}

static int __init early_parse_reboot_reason_cmdline(char *reboot_reason_cmdline)
{
	int i;

	if (reboot_reason_cmdline == NULL) {
		BB_PRINT_ERR("[%s:%d]: reboot_reason_cmdline is null\n", __func__, __LINE__);
		return -1;
	}

	(void)memset_s(g_reboot_reason, RDR_REBOOT_REASON_LEN, 0x0, RDR_REBOOT_REASON_LEN);

	if (memcpy_s(g_reboot_reason, RDR_REBOOT_REASON_LEN,
	    reboot_reason_cmdline, RDR_REBOOT_REASON_LEN - 1) != EOK) {
		BB_PRINT_ERR("[%s:%d]: memcpy_s err\n", __func__, __LINE__);
		return -1;
	}

	for (i = 0; (u32)i < get_reboot_reason_map_size(); i++) {
		if (!strncmp((char *)reboot_reason_map[i].name, g_reboot_reason, RDR_REBOOT_REASON_LEN)) {
			g_reboot_type = reboot_reason_map[i].num;
			break;
		}
	}
	BB_PRINT_PN("[%s][%s][%u]\n", __func__, g_reboot_reason, g_reboot_type);
	return 0;
}

early_param("reboot_reason", early_parse_reboot_reason_cmdline);

void *bbox_vmap(phys_addr_t paddr, size_t size)
{
	int i;
	void *vaddr = NULL;
	pgprot_t pgprot;
	unsigned long offset;
	int pages_count;
	struct page **pages = NULL;

	offset = paddr & ~PAGE_MASK;
	paddr &= PAGE_MASK;
	pages_count = PAGE_ALIGN(size + offset) / PAGE_SIZE;

	pages = kzalloc(sizeof(struct page *) * pages_count, GFP_KERNEL);
	if (pages == NULL)
		return NULL;

	pgprot = pgprot_writecombine(PAGE_KERNEL);

	for (i = 0; i < pages_count; i++)
		*(pages + i) = phys_to_page((uintptr_t)(paddr + PAGE_SIZE * i));

	vaddr = vmap(pages, pages_count, VM_MAP, pgprot);
	kfree(pages);
	if (vaddr == NULL)
		return NULL;

	return offset + (char *)vaddr;
}

void *hisi_bbox_map(phys_addr_t paddr, size_t size)
{
	void *vaddr = NULL;

	if (paddr < reserved_rdr_phymem_addr || !size || ((paddr + size) < paddr) ||
	   (paddr + size) > (reserved_rdr_phymem_addr + reserved_rdr_phymem_size)) {
		BB_PRINT_ERR("Error BBox memory\n");
		return NULL;
	}

	if (pfn_valid(reserved_rdr_phymem_addr >> PAGE_SHIFT))
		vaddr = bbox_vmap(paddr, size);
	else
		vaddr = ioremap_wc(paddr, size);

	return vaddr;
}
EXPORT_SYMBOL(hisi_bbox_map);
void hisi_bbox_unmap(const void *vaddr)
{
	if (vaddr == NULL)
		return;
	if (pfn_valid(reserved_rdr_phymem_addr >> PAGE_SHIFT))
		vunmap(vaddr);
	else
		iounmap((void __iomem *)vaddr);
}
EXPORT_SYMBOL(hisi_bbox_unmap);

/*
 * Description:  After the log directory corresponding to each exception is saved,
 *               this function needs to be called to indicate that the directory has been recorded
 *               and it can be packaged and uploaded by logserver.
 * Input:        logpath: the directory where the log is saved corresponding to the exception;
 *               step:the step which the exception log is saved in, and whether to continue using the flag;
 * Other:        used by rdr_core.c, rdr_hisi_ap_adapter.c
 */
void bbox_save_done(const char *logpath, u32 step)
{
	int fd = -1;
	int ret;
	char path[PATH_MAXLEN];
	u32 len;

	BB_PRINT_START();
	if (logpath == NULL ||
	   (strlen(logpath) + strlen(BBOX_SAVE_DONE_FILENAME) + 1) > PATH_MAXLEN) {
		BB_PRINT_ERR("logpath is invalid\n");
		return;
	}

	BB_PRINT_PN("logpath is [%s], step is [%u]\n", logpath, step);
	if (step == BBOX_SAVE_STEP_DONE) {
		/* combine the absolute path of the done file as a parameter of sys_mkdir */
		(void)memset_s(path, PATH_MAXLEN, 0, PATH_MAXLEN);
		len = strlen(logpath);
		ret = memcpy_s(path, PATH_MAXLEN - 1, logpath, len);
		if (ret != EOK) {
			BB_PRINT_ERR("[%s:%d]: memcpy_s err\n]", __func__, __LINE__);
			return;
		}

		strncat(path, BBOX_SAVE_DONE_FILENAME, ((PATH_MAXLEN - 1) - strlen(path)));

		/* create a done file under the timestamp directory */
		fd = sys_open(path, O_CREAT | O_WRONLY, FILE_LIMIT);
		if (fd < 0) {
			BB_PRINT_ERR("sys_mkdir [%s] error, fd is [%d]\n", path, fd);
			return;
		}
		sys_close(fd);

		/*
		 * according to the permission requirements,
		 * the hisi_logs directory and subdirectory group are adjusted to root-system
		 */
		ret = (int)bbox_chown((const char __user *)path, ROOT_UID, SYSTEM_GID, false);
		if (ret)
			BB_PRINT_ERR("chown %s uid [%d] gid [%d] failed err [%d]!\n",
			     PATH_ROOT, ROOT_UID, SYSTEM_GID, ret);
	}

	BB_PRINT_END();
}

/*
 * Description:  save reboot times to specified memory
 */
void rdr_record_reboot_times2mem(void)
{
	struct rdr_struct_s *pbb = NULL;

	BB_PRINT_START();
	pbb = rdr_get_pbb();
	pbb->top_head.reserve = RDR_UNEXPECTED_REBOOT_MARK_ADDR;
	BB_PRINT_END();
}

/*
 * Description:   reset the file saving reboot times
 */
void rdr_reset_reboot_times(void)
{
	struct file *fp = NULL;
	ssize_t length;
	char buf;

	BB_PRINT_START();
	fp = filp_open(RDR_REBOOT_TIMES_FILE, O_CREAT | O_RDWR, FILE_LIMIT);
	if (IS_ERR(fp)) {
		BB_PRINT_ERR("rdr:%s(),open %s fail\n", __func__,
			    RDR_REBOOT_TIMES_FILE);
		return;
	}
	buf = 0;
	vfs_llseek(fp, 0L, SEEK_SET);
	length = vfs_write(fp, &buf, sizeof(buf), &(fp->f_pos));
	if (length == sizeof(buf))
		vfs_fsync(fp, 0);

	filp_close(fp, NULL);
	BB_PRINT_END();
}

/*
 * Description:   write the enter reason of erecovery to /cache/recovery/last_erecovery_entry
 */
void rdr_record_erecovery_reason(void)
{
	struct file *fp = NULL;
	ssize_t length;
	const char *e_reason = "erecovery_enter_reason:=2015\n";

	BB_PRINT_START();

	fp = filp_open(RDR_ERECOVERY_REASON_FILE, O_CREAT | O_RDWR, FILE_LIMIT);
	if (IS_ERR(fp)) {
		BB_PRINT_ERR("rdr:%s(),open %s first fail,error No. %pK\n", __func__, RDR_ERECOVERY_REASON_FILE, fp);
		rdr_create_dir("/cache/recovery/");
		fp = filp_open(RDR_ERECOVERY_REASON_FILE, O_CREAT | O_RDWR | O_TRUNC, FILE_LIMIT);
		if (IS_ERR(fp)) {
			BB_PRINT_ERR("rdr:%s(),open %s second fail,error No. %pK\n",
			    __func__, RDR_ERECOVERY_REASON_FILE, fp);
			return;
		}
	}
	vfs_llseek(fp, 0L, SEEK_SET);
	length = vfs_write(fp, e_reason, strlen(e_reason) + 1, &(fp->f_pos));
	if (length == (strlen(e_reason) + 1))
		vfs_fsync(fp, 0);

	filp_close(fp, NULL);
	BB_PRINT_END();
}

/*
 * Description:   record the reboot times to file.
 * Return:        int: reboot times.
 */
int rdr_record_reboot_times2file(void)
{
	struct file *fp = NULL;
	ssize_t length;
	char buf = 0;

	BB_PRINT_START();
	fp = filp_open(RDR_REBOOT_TIMES_FILE, O_CREAT | O_RDWR, FILE_LIMIT);
	if (IS_ERR(fp)) {
		BB_PRINT_ERR("rdr:%s(),open %s fail\n", __func__,
			   RDR_REBOOT_TIMES_FILE);
		return 0;
	}

	vfs_llseek(fp, 0L, SEEK_SET);
	length = vfs_read(fp, &buf, sizeof(buf), &fp->f_pos);
	if (length == 0 || buf == 0)
		buf = 0;
	buf++;

	vfs_llseek(fp, 0L, SEEK_SET);
	length = vfs_write(fp, &buf, sizeof(buf), &(fp->f_pos));
	if (length == sizeof(buf))
		vfs_fsync(fp, 0);

	filp_close(fp, NULL);
	BB_PRINT_END();
	return buf;
}
