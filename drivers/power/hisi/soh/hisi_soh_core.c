/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: hisi_soh_core.c
 *
 * core for soh module
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

#include "hisi_soh_core.h"
#include <linux/delay.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <securec.h>
#include <linux/power/hisi/coul/coul_drv.h>
#include <linux/power/hisi/coul/coul_event.h>
#include <linux/power/hisi/bci_battery.h>
#ifdef CONFIG_BATT_EIS
#include <linux/power/hisi/hisi_eis.h>
#endif
#include <linux/mtd/hisi_nve_interface.h>
#include <huawei_platform/log/hwlog_kernel.h>
#include "../coul/coul_nv.h"

static struct soh_acr_device_ops     *g_soh_acr_core_ops;
static struct soh_dcr_device_ops     *g_soh_dcr_core_ops;
static struct soh_pd_leak_device_ops *g_soh_pd_leak_core_ops;
static struct soh_ovp_device_ops     *g_soh_ovp_core_ops;
static struct hisi_soh_device *g_soh_di;

/* default ovp threshold */
static struct soh_ovp_temp_vol_threshold
	default_ovh_thres[SOH_OVH_THRED_NUM] = {
	{ 4200, 60 }, { 4300, 55 }, { 4350, 50 }
}; /* temp & vol */
static struct soh_ovp_temp_vol_threshold
	default_ovl_thres[SOH_OVL_THRED_NUM] = {
	{ 4100, 60 }, { 4200, 55 }, {4300, 50}
}; /* temp & vol */
static struct soh_ovp_temp_vol_threshold
	default_ovl_safe_thres = { 4100, 50 }; /* temp | vol */

/* get nv info from boot */
static struct acr_nv_info *boot_acr_nv_info = NULL;
static struct dcr_nv_info *boot_dcr_nv_info = NULL;
static struct pd_leak_nv_info *boot_pdleak_nv_info = NULL;

struct hisi_soh_device *get_soh_core_device(void)
{
	return g_soh_di;
}

/* get array max value and min value. */
static void max_min_value(const int array[], u32 size, int *min, int *max)
{
	u32 i;
	int max_value;
	int min_value;

	if (!size || !max || !min)
		return;

	max_value = array[0];
	min_value = array[0];

	for (i = 1; i < size; i++) {
		hisi_soh_debug("[%s] temp[%u]=%d\n", __func__, i, array[i]);
		if (array[i] > max_value)
			max_value = array[i];

		if (array[i] < min_value)
			min_value = array[i];
	}

	*max = max_value;
	*min = min_value;
}

static void soh_wake_lock(struct hisi_soh_device *di)
{
	if (!di)
		return;
	if (!di->soh_wake_lock.active) {
		__pm_stay_awake(&di->soh_wake_lock);
		hisi_soh_info("soh core wake lock!\n");
	}
}

static void soh_wake_unlock(struct hisi_soh_device *di)
{
	if (!di)
		return;
	if (di->soh_wake_lock.active) {
		__pm_relax(&di->soh_wake_lock);
		hisi_soh_info("soh core wake unlock!\n");
	}
}

/* register the acr handler ops from  chipIC */
int soh_core_drv_ops_register(void *ops, enum soh_drv_ops_type ops_type)
{
	int ret = 0;

	if (!ops)
		hisi_soh_err(" %s ops register null!\n", __func__);

	switch (ops_type) {
	case ACR_DRV_OPS:
		g_soh_acr_core_ops = ops;
		break;
	case DCR_DRV_OPS:
		g_soh_dcr_core_ops = ops;
		break;
	case PD_LEAK_DRV_OPS:
		g_soh_pd_leak_core_ops = ops;
		break;
	case SOH_OVP_DRV_OPS:
		g_soh_ovp_core_ops = ops;
		break;
	default:
		hisi_soh_err("[%s] failed ops register!\n", __func__);
		ret = -1;
		break;
	}
	hisi_soh_info("[%s]ops_type = %d!\n", __func__, ops_type);
	return ret;
}

static int soh_drv_acr_ops_check(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	if (di->soh_acr_dev.acr_support) {
		if (!g_soh_acr_core_ops) {
			hisi_soh_err("[%s] acr_ops is not register!\n",
				__func__);
			return -1;
		}

		di->soh_acr_dev.acr_ops = g_soh_acr_core_ops;
		if (!di->soh_acr_dev.acr_ops->enable_acr ||
			!di->soh_acr_dev.acr_ops->calculate_acr ||
			!di->soh_acr_dev.acr_ops->clear_acr_flag ||
			!di->soh_acr_dev.acr_ops->clear_acr_ocp ||
			!di->soh_acr_dev.acr_ops->get_acr_chip_temp ||
			!di->soh_acr_dev.acr_ops->get_acr_flag ||
			!di->soh_acr_dev.acr_ops->get_acr_ocp ||
			!di->soh_acr_dev.acr_ops->get_acr_fault_status ||
			!di->soh_acr_dev.acr_ops->clear_acr_fault_status ||
			!di->soh_acr_dev.acr_ops->io_ctrl_acr_chip_en) {
			hisi_soh_err("[%s] soh acr device ops is NULL!\n",
				__func__);
			return -1;
		}
	}
	hisi_soh_info("[%s] suc!\n", __func__);
	return 0;
}

static int soh_drv_dcr_ops_check(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	if (di->soh_dcr_dev.dcr_support) {
		if (!g_soh_dcr_core_ops) {
			hisi_soh_err("[%s] dcr_ops is not register!\n", __func__);
			return -1;
		}

		di->soh_dcr_dev.dcr_ops = g_soh_dcr_core_ops;
		if (!di->soh_dcr_dev.dcr_ops->enable_dcr ||
			!di->soh_dcr_dev.dcr_ops->clear_dcr_flag ||
			!di->soh_dcr_dev.dcr_ops->get_dcr_flag ||
			!di->soh_dcr_dev.dcr_ops->get_dcr_info) {
			hisi_soh_err("[%s] soh dcr device ops func is NULL!\n", __func__);
			return -1;
		}
	}
	hisi_soh_info("[%s] suc!\n", __func__);
	return 0;
}

static int soh_drv_ovp_ops_check(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	if (di->soh_ovp_dev.ovp_support) {
		if (!g_soh_ovp_core_ops) {
			hisi_soh_err("%s ovp_ops is not register!\n", __func__);
			return -1;
		}

		di->soh_ovp_dev.soh_ovp_ops = g_soh_ovp_core_ops;
		if (!di->soh_ovp_dev.soh_ovp_ops->set_ovp_threshold ||
			!di->soh_ovp_dev.soh_ovp_ops->get_ovh_thred_cnt ||
			!di->soh_ovp_dev.soh_ovp_ops->enable_ovp ||
			!di->soh_ovp_dev.soh_ovp_ops->enable_dischg ||
			!di->soh_ovp_dev.soh_ovp_ops->get_stop_dischg_state) {
			hisi_soh_err("%s soh ovp device ops func is NULL!\n",
				__func__);
			return -1;
		}
	}
	hisi_soh_info("[%s] suc!\n", __func__);
	return 0;
}

static int soh_drv_pd_leak_ops_check(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	if (di->soh_pd_leak_dev.pd_leak_support) {
		if (!g_soh_pd_leak_core_ops) {
			hisi_soh_err("%s pd_leak_ops is not register!\n", __func__);
			return -1;
		}

		di->soh_pd_leak_dev.pd_leak_ops = g_soh_pd_leak_core_ops;
		if (!di->soh_pd_leak_dev.pd_leak_ops->enable_pd_leak ||
			!di->soh_pd_leak_dev.pd_leak_ops->get_pd_leak_fifo_depth ||
			!di->soh_pd_leak_dev.pd_leak_ops->get_pd_leak_info) {
			hisi_soh_err("%s soh dcr device ops func is NULL!\n",
				__func__);
			return -1;
		}
	}
	hisi_soh_info("[%s] suc!\n", __func__);
	return 0;
}

static int soh_drv_ops_check(struct hisi_soh_device *di,
	enum soh_drv_ops_type ops_type)
{
	int ret;

	if (!di)
		return -1;

	switch (ops_type) {
	case ACR_DRV_OPS:
		/* acr driver register check */
		ret = soh_drv_acr_ops_check(di);
		break;
	case DCR_DRV_OPS:
		/* dcr driver register check */
		ret = soh_drv_dcr_ops_check(di);
		break;
	case SOH_OVP_DRV_OPS:
		/* soh_ovp driver register check */
		ret = soh_drv_ovp_ops_check(di);
		break;
	case PD_LEAK_DRV_OPS:
		/* sohu pd driver register check */
		ret = soh_drv_pd_leak_ops_check(di);
		break;
	default:
		hisi_soh_err("[%s] failed ops register!\n",
			__func__);
		break;
	}
	return ret;
}

static int soh_acr_rw_nv(struct hisi_soh_device *di, enum nv_rw_type rw)
{
	struct acr_nv_info acrinfo;
	struct hisi_nve_info_user nve;
	int ret;
	errno_t err;

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
		SOH_ACR_NV_NAME, strlen(SOH_ACR_NV_NAME));
	if (err != EOK) {
		hisi_soh_err("[%s]strncpy_s fail, err=%d\n", __func__, err);
		return -1;
	}
	nve.nv_number = SOH_ACR_NV_NUM;
	nve.valid_size = sizeof(acrinfo);
	/* save battery info for acr */
	if (rw == NV_WRITE) {
		/* get nv data from read nv to Block overwrite */
		(void)memcpy_s(&acrinfo, sizeof(acrinfo),
			&di->soh_acr_dev.acr_nv, sizeof(di->soh_acr_dev.acr_nv));

		if (acrinfo.order_num < 0)
			acrinfo.order_num = -1;
		acrinfo.order_num = (acrinfo.order_num + 1) % SOH_ACR_NV_DATA_NUM;
		(void)memcpy_s(&acrinfo.soh_nv_acr_info[acrinfo.order_num],
			sizeof(acrinfo.soh_nv_acr_info[acrinfo.order_num]),
			&di->soh_acr_dev.soh_acr_info,
			sizeof(di->soh_acr_dev.soh_acr_info));

		nve.nv_operation = NV_WRITE;
		err = memcpy_s(nve.nv_data, sizeof(nve.nv_data),
			&acrinfo, sizeof(acrinfo));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
		ret = hisi_nve_direct_access(&nve);
		if (ret)
			hisi_soh_err("[%s]acr save nv fail, ret=%d\n",
				__func__, ret);
		else /* succ, writeback to acr_nv */
			(void)memcpy_s(&di->soh_acr_dev.acr_nv,
				sizeof(di->soh_acr_dev.acr_nv),
				&acrinfo, sizeof(acrinfo));
	} else { /* read acr nv data */
		nve.nv_operation = NV_READ;
		ret = hisi_nve_direct_access(&nve);
		if (ret) {
			hisi_soh_err("acr read nv partion fail, ret=%d\n", ret);
		} else {
			err = memcpy_s(&di->soh_acr_dev.acr_nv,
				sizeof(di->soh_acr_dev.acr_nv), nve.nv_data,
				sizeof(acrinfo));
			if (err != EOK) {
				hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
					__func__, err);
				return -1;
			}
		}
	}
	return ret;
}
static int soh_get_dcr_nv_data(struct hisi_soh_device *di, struct hisi_nve_info_user *nve)
{
	int ret;
	errno_t err;
	struct dcr_nv_info dcrinfo;

	nve->nv_operation = NV_READ;
	ret = hisi_nve_direct_access(nve);
	if (ret) {
		hisi_soh_err("dcr read nv partion fail, ret=%d\n", ret);
	} else {
		err = memcpy_s(&di->soh_dcr_dev.dcr_nv,
			sizeof(di->soh_dcr_dev.dcr_nv),
			nve->nv_data, sizeof(dcrinfo));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
	}
	return 0;
}
static int soh_dcr_rw_nv(struct hisi_soh_device *di, enum nv_rw_type rw)
{
	struct dcr_nv_info dcrinfo;
	struct hisi_nve_info_user nve;
	int ret;
	errno_t err;

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
		SOH_DCR_NV_NAME, strlen(SOH_DCR_NV_NAME));
	if (err != EOK) {
		hisi_soh_err("[%s]strncpy_s fail, err=%d\n", __func__, err);
		return -1;
	}
	nve.nv_number = SOH_DCR_NV_NUM;
	nve.valid_size = sizeof(dcrinfo);
	/* save battery info for dcr */
	if (rw == NV_WRITE) {
		/* get nv data from read nv to Block overwrite */
		(void)memcpy_s(&dcrinfo, sizeof(dcrinfo),
			&di->soh_dcr_dev.dcr_nv, sizeof(di->soh_dcr_dev.dcr_nv));

		if (dcrinfo.order_num < 0)
			dcrinfo.order_num = -1;
		dcrinfo.order_num = (dcrinfo.order_num + 1) % SOH_DCR_NV_DATA_NUM;
		(void)memcpy_s(&dcrinfo.soh_nv_dcr_info[dcrinfo.order_num],
			sizeof(dcrinfo.soh_nv_dcr_info[dcrinfo.order_num]),
			&di->soh_dcr_dev.soh_dcr_info,
			sizeof(di->soh_dcr_dev.soh_dcr_info));

		nve.nv_operation = NV_WRITE;
		err = memcpy_s(nve.nv_data, sizeof(nve.nv_data),
				&dcrinfo, sizeof(dcrinfo));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
		ret = hisi_nve_direct_access(&nve);
		if (ret)
			hisi_soh_err("[%s]dcr save nv fail, ret=%d\n",
				__func__, ret);
		else /* succ, writeback to dcr_nv */
			(void)memcpy_s(&di->soh_dcr_dev.dcr_nv,
				sizeof(di->soh_dcr_dev.dcr_nv),
				&dcrinfo, sizeof(dcrinfo));
	} else { /* read dcr nv data */
		ret = soh_get_dcr_nv_data(di, &nve);
		if (ret)
			return -1;
	}
	return ret;
}
static int soh_get_pd_leak_nv_data(struct hisi_soh_device *di, struct hisi_nve_info_user *nve)
{
	int ret;
	errno_t err;
	struct dcr_nv_info pdinfo;

	nve->nv_operation = NV_READ;
	ret = hisi_nve_direct_access(nve);
	if (ret) {
		hisi_soh_err("pd read nv partion fail, ret=%d\n", ret);
	} else {
		err = memcpy_s(&di->soh_pd_leak_dev.pd_leak_nv,
			sizeof(di->soh_pd_leak_dev.pd_leak_nv),
			nve->nv_data, sizeof(pdinfo));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
	}
	return 0;
}
static int soh_pd_leak_rw_nv(struct hisi_soh_device *di, enum nv_rw_type rw)
{
	struct pd_leak_nv_info pdinfo;
	struct hisi_nve_info_user nve;
	int ret;
	errno_t err;

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));

	err = strncpy_s(nve.nv_name, sizeof(nve.nv_name),
		SOH_PD_LEAK_NV_NAME, strlen(SOH_PD_LEAK_NV_NAME));
	if (err != EOK) {
		hisi_soh_err("[%s]strncpy_s fail, err=%d\n", __func__, err);
		return -1;
	}
	nve.nv_number  = SOH_PD_LEAK_NV_NUM;
	nve.valid_size = sizeof(pdinfo);
	/* save battery data for pd */
	if (rw == NV_WRITE) {
		/* get nv data from read nv to Block overwrite */
		(void)memcpy_s(&pdinfo, sizeof(pdinfo),
			&di->soh_pd_leak_dev.pd_leak_nv,
			sizeof(di->soh_pd_leak_dev.pd_leak_nv));

		if (pdinfo.order_num < 0)
			pdinfo.order_num = -1;
		pdinfo.order_num = (pdinfo.order_num + 1) % SOH_PD_NV_DATA_NUM;
		(void)memcpy_s(
			&pdinfo.soh_nv_pd_leak_current_info[pdinfo.order_num],
			sizeof(pdinfo.soh_nv_pd_leak_current_info[pdinfo.order_num]),
			&di->soh_pd_leak_dev.soh_pd_leak_current_info,
			sizeof(di->soh_pd_leak_dev.soh_pd_leak_current_info));

		nve.nv_operation = NV_WRITE;
		err = memcpy_s(nve.nv_data, sizeof(nve.nv_data),
			&pdinfo, sizeof(pdinfo));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
		ret = hisi_nve_direct_access(&nve);
		if (ret)
			hisi_soh_err("[%s]pd leak save nv fail, ret=%d\n",
				__func__, ret);
		else /* succ, writeback to pdinfo nv */
			(void)memcpy_s(&di->soh_pd_leak_dev.pd_leak_nv,
				sizeof(di->soh_pd_leak_dev.pd_leak_nv),
				&pdinfo, sizeof(pdinfo));
	} else { /* read pd leak nv data */
		ret = soh_get_pd_leak_nv_data(di, &nve);
		if (ret)
			return -1;
	}
	return ret;
}

int soh_rw_nv_info(struct hisi_soh_device *di,
	enum soh_type type, enum nv_rw_type rw)
{
	int ret;

	if (!di) {
		hisi_soh_err("[%s]NULL point!\n", __func__);
		return -1;
	}

	switch (type) {
	case SOH_ACR:
		ret = soh_acr_rw_nv(di, rw);
		break;

	case SOH_DCR:
		ret = soh_dcr_rw_nv(di, rw);
		break;

	case SOH_PD_LEAK:
		ret = soh_pd_leak_rw_nv(di, rw);
		break;

	default:
		hisi_soh_err("[%s] nv type err!!!\n", __func__);
		return -1;
	}
	if (!ret)
		hisi_soh_info("[%s] rw nv success!\n", __func__);
	return ret;
}

/* get nv info rom boot by reserved mem. */
static int soh_get_nv_info_from_boot(void)
{
	unsigned long nv_info_addr = get_nv_info_addr();
	if (!nv_info_addr) {
		hisi_soh_err("[%s] nv add from coul is null!\n", __func__);
		return -1;
	}

	boot_acr_nv_info = ioremap_wc(
		(nv_info_addr + RESERVED_MEM_FOR_PMU_COUL_ADDR_128),
		sizeof(*boot_acr_nv_info));
	if (!boot_acr_nv_info) {
		hisi_soh_err("[%s] boot_acr_nv_info is null!\n", __func__);
		goto acr_info_fail;
	}

	boot_dcr_nv_info = ioremap_wc(
		(nv_info_addr + RESERVED_MME_FOR_PMU_ACR_ADD_128),
		sizeof(*boot_dcr_nv_info));
	if (!boot_dcr_nv_info) {
		hisi_soh_err("[%s] boot_dcr_nv_info is null!\n", __func__);
		goto dcr_info_fail;
	 }

	boot_pdleak_nv_info = ioremap_wc(
		(nv_info_addr + RESERVED_MME_FOR_PMU_DCR_ADD_128),
		sizeof(*boot_pdleak_nv_info));
	if (!boot_pdleak_nv_info) {
		hisi_soh_err("[%s] boot_pdleak_nv_info is null!\n", __func__);
		goto pdleak_info_fail;
	}

	hisi_soh_info("[%s]acrinfo = 0x%pK, dcrinfo = 0x%pK, pdinfo = 0x%pK!\n",
		__func__, boot_acr_nv_info,
		boot_dcr_nv_info, boot_pdleak_nv_info);

	return 0;
pdleak_info_fail:
	iounmap(boot_dcr_nv_info);
	boot_dcr_nv_info = NULL;
dcr_info_fail:
	iounmap(boot_acr_nv_info);
	boot_acr_nv_info = NULL;
acr_info_fail:
	return -1;
}

static void soh_nv_info_uninit(void)
{
	  if (boot_acr_nv_info) {
		    iounmap(boot_acr_nv_info);
		    boot_acr_nv_info = NULL;
	  }
	  if (boot_dcr_nv_info) {
		    iounmap(boot_dcr_nv_info);
		    boot_dcr_nv_info = NULL;
	  }
	  if (boot_pdleak_nv_info) {
		    iounmap(boot_pdleak_nv_info);
		    boot_pdleak_nv_info = NULL;
	  }
}
static void parse_ovp_thd_dts(struct hisi_soh_device *di,
	const struct device_node *np)
{
	int ret;
	errno_t err;
	/* get ovp discharge  threshold */
	ret = of_property_read_u32_array(np, "soh_ovh_thd",
		(u32 *)&di->soh_ovp_dev.soh_ovh_thres[0],
		sizeof(di->soh_ovp_dev.soh_ovh_thres) / sizeof(int));
	if (ret) {
		err = memcpy_s(&di->soh_ovp_dev.soh_ovh_thres,
			sizeof(di->soh_ovp_dev.soh_ovh_thres),
			&default_ovh_thres[0],
			sizeof(default_ovh_thres));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return;
		}
		hisi_soh_err("[%s] get ovp dis thred  fail!\n", __func__);
	}
	hisi_soh_info("[%s]addr:soh_ovh_thres[0].temp[%d],[0].vol[%d],[1].temp[%d],[1].vol[%d],[2].temp[%d],[2].vol[%d]!\n",
		      __func__, di->soh_ovp_dev.soh_ovh_thres[0].temp,
		      di->soh_ovp_dev.soh_ovh_thres[0].bat_vol_mv,
		      di->soh_ovp_dev.soh_ovh_thres[1].temp,
		      di->soh_ovp_dev.soh_ovh_thres[1].bat_vol_mv,
		      di->soh_ovp_dev.soh_ovh_thres[2].temp,
		      di->soh_ovp_dev.soh_ovh_thres[2].bat_vol_mv);

	/* get ovp stop discharge threshold */
	ret = of_property_read_u32_array(np, "soh_ovl_thd",
		(u32 *)&di->soh_ovp_dev.soh_ovl_thres[0],
		sizeof(di->soh_ovp_dev.soh_ovl_thres) / sizeof(int));
	if (ret) {
		err = memcpy_s(&di->soh_ovp_dev.soh_ovl_thres,
			sizeof(di->soh_ovp_dev.soh_ovl_thres),
			&default_ovl_thres[0], sizeof(default_ovl_thres));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return;
		}
		hisi_soh_err("[%s] get soh_ovl_thd fail!\n", __func__);
	}
	hisi_soh_info("[%s]addr:soh_ovl_thd[0].temp[%d],[0].vol[%d],[1].temp[%d],[1].vol[%d],[2].temp[%d],[2].vol[%d]!\n",
		      __func__, di->soh_ovp_dev.soh_ovl_thres[0].temp,
		      di->soh_ovp_dev.soh_ovl_thres[0].bat_vol_mv,
		      di->soh_ovp_dev.soh_ovl_thres[1].temp,
		      di->soh_ovp_dev.soh_ovl_thres[1].bat_vol_mv,
		      di->soh_ovp_dev.soh_ovl_thres[2].temp,
		      di->soh_ovp_dev.soh_ovl_thres[2].bat_vol_mv);
}

static int parse_soh_ovp_dts(struct hisi_soh_device *di)
{
	struct device_node *np = NULL;
	int ret;
	errno_t err;

	if (!di)
		return -1;

	np = di->dev->of_node;
	if (!np) {
		hisi_soh_err("%s np is null!\n", __func__);
		return -1;
	}

	/* get support flag */
	if (of_property_read_u32(np, "soh_ovp_support",
		(u32 *)&di->soh_ovp_dev.ovp_support)) {
		di->soh_ovp_dev.ovp_support = 0;
		hisi_soh_err("[%s] get ovp support  fail!\n", __func__);
	}
	parse_ovp_thd_dts(di, np);
	/* get ovp stop discharge safe threshold */
	ret = of_property_read_u32_array(np, "soh_ovl_safe_thd",
		(u32 *)&di->soh_ovp_dev.soh_ovl_safe_thres,
		sizeof(struct soh_ovp_temp_vol_threshold) / sizeof(int));
	if (ret) {
		err = memcpy_s(&di->soh_ovp_dev.soh_ovl_safe_thres,
			sizeof(di->soh_ovp_dev.soh_ovl_safe_thres),
			&default_ovl_safe_thres,
			sizeof(default_ovl_safe_thres));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
		hisi_soh_err("[%s] get soh_ovl_safe_thres  fail!\n", __func__);
	}
	hisi_soh_info("[%s] addr:soh_ovl_safe_thres temp[%d],vol[%d] !\n",
		__func__, di->soh_ovp_dev.soh_ovl_safe_thres.temp,
		di->soh_ovp_dev.soh_ovl_safe_thres.bat_vol_mv);

	/* gte start discharge time with ovh interrupt received. */
	ret = of_property_read_u32_array(np,
		"soh_ovp_start_min_thd",
		(u32 *)&di->soh_ovp_dev.soh_ovp_start_time, 1);
	if (ret) {
		di->soh_ovp_dev.soh_ovp_start_time = OVP_START_MIN_MINUTES;
		hisi_soh_err("[%s] get ovp start time fail!\n", __func__);
	}
	hisi_soh_info("[%s]soh start time = [%d]min !\n",
		__func__, di->soh_ovp_dev.soh_ovp_start_time);

	return 0;
}

static int parse_soh_acr_dts(struct hisi_soh_device *di)
{
	struct device_node *np = NULL;

	if (!di) {
		hisi_soh_err("%s di is null!\n", __func__);
		return -1;
	}

	np = di->dev->of_node;
	if (!np) {
		hisi_soh_err("%s np is null!\n", __func__);
		return -1;
	}

	if (of_property_read_u32(np, "acr_support",
		(u32 *)&di->soh_acr_dev.acr_support)) {
		di->soh_acr_dev.acr_support = 0;
		hisi_soh_err("[%s] get acr support fail!\n", __func__);
	}
	hisi_soh_info("[%s] soh acr support = [%d]!\n",
		__func__, di->soh_acr_dev.acr_support);

	return 0;
}

static int parse_soh_dcr_dts(struct hisi_soh_device *di)
{
	struct device_node *np = NULL;

	if (!di) {
		hisi_soh_err("%s di is null!\n", __func__);
		return -1;
	}

	np = di->dev->of_node;
	if (!np) {
		hisi_soh_err("%s np is null!\n", __func__);
		return -1;
	}

	if (of_property_read_u32(np, "dcr_support",
		(u32 *)&di->soh_dcr_dev.dcr_support)) {
		di->soh_dcr_dev.dcr_support = 0;
		hisi_soh_err("[%s] get dcr support fail!\n", __func__);
	}
	hisi_soh_info("[%s] soh dcr support = [%d]!\n",
		__func__, di->soh_dcr_dev.dcr_support);

	return 0;
}

static int parse_soh_pd_leak_dts(struct hisi_soh_device *di)
{
	struct device_node *np = NULL;

	if (!di) {
		hisi_soh_err("%s di is null!\n", __func__);
		return -1;
	}

	np = di->dev->of_node;
	if (!np) {
		hisi_soh_err("%s np is null!\n", __func__);
		return -1;
	}

	if (of_property_read_u32(np, "pd_ocv_support",
		(u32 *)&di->soh_pd_leak_dev.pd_leak_support)) {
		di->soh_pd_leak_dev.pd_leak_support = 0;
		hisi_soh_err("[%s] get pd ocv support fail!\n", __func__);
	}
	if (of_property_read_u32(np, "soh_pd_leak_standby_leakage_current",
		(u32 *)&di->soh_pd_leak_dev.soh_pd_leak_current_info.sys_pd_leak_cc)) {
		di->soh_pd_leak_dev.soh_pd_leak_current_info.sys_pd_leak_cc = 0;
		hisi_soh_err("[%s] get pd cc fail!\n", __func__);
	}

	hisi_soh_info("[%s] soh pd leak support = [%d], pd_cc = %d!\n",
		__func__, di->soh_pd_leak_dev.pd_leak_support,
		di->soh_pd_leak_dev.soh_pd_leak_current_info.sys_pd_leak_cc);

	return 0;
}

/*
 * check acr start valid by temp current and bat cycle soc.
 * 1 battery temp        [0,55]
 * 2 battery current     [-20,20mA]
 * 3 battery cycle add   [1,]
 * 4 battery soc         //[95%,100%]
 * 5 chip temp           [0,70] in soh_acr_calculate_work
 */
int soh_acr_start_check(void)
{
	int t_bat, i_bat, real_soc;
	int cycle_bat_pre, cycle_bat_now;
	struct hisi_soh_device *di = g_soh_di;

	if (!di) {
		hisi_soh_err("%s di is NULL!\n", __func__);
		return -1;
	}
	hisi_soh_err("[%s-%d]is called!!\n", __func__, __LINE__);
	/* if dts not support,return success */
	if (!di->soh_acr_dev.acr_support) {
		hisi_soh_err("[%s] not support soh acr!\n", __func__);
		return -1;
	}

	/* acr nv close */
	if (di->soh_acr_dev.acr_nv.acr_control) {
		hisi_soh_err("[%s] nv close acr!\n", __func__);
		return -1;
	}

	t_bat = coul_drv_battery_temperature();
	i_bat = coul_drv_battery_current();
	cycle_bat_now = coul_drv_battery_cycle_count();

	if (di->soh_acr_dev.acr_nv.order_num < 0 ||
		di->soh_acr_dev.acr_nv.order_num >= SOH_ACR_NV_DATA_NUM)
		di->soh_acr_dev.acr_nv.order_num = -1;

	if (di->soh_acr_dev.acr_nv.order_num == -1)
		cycle_bat_pre = 0;
	else
		cycle_bat_pre =
		di->soh_acr_dev.acr_nv.soh_nv_acr_info[
		di->soh_acr_dev.acr_nv.order_num].batt_cycle;

	real_soc = coul_drv_battery_unfiltered_capacity();

	/* check battery temperature */
	if (t_bat > ACR_CAL_BAT_TEMP_MAX || t_bat <  ACR_CAL_BAT_TEMP_MIN) {
		hisi_soh_err("%s bat temp[%d]c  exceed the normal range!\n", __func__, t_bat);
		return -1;
	}

	/* check battery now current */
	if (abs(i_bat) > ACR_CAL_BAT_CUR_MAX) {
		hisi_soh_err("%s bat current[%d]mA, exceed the normal range!\n", __func__, i_bat);
		return -1;
	}

	/* check battery cycle */
	if (abs(cycle_bat_now - cycle_bat_pre) < di->soh_acr_dev.acr_cycle_inc) {
		hisi_soh_err("%s cycle inc not enough, pre = [%d], now = [%d]!\n",
			__func__, cycle_bat_pre, cycle_bat_now);
		return -1;
	}

	/* check battery real soc */
	if (real_soc  < ACR_CAL_BAT_SOC_MIN)
		hisi_soh_err("%s read soc low  ,real_soc = [%d]!\n", __func__, real_soc);

	hisi_soh_info("[%s]sucess, ibat = [%d], tbat = [%d], cycle_now = [%d], cycle_pre = [%d], soc =[%d]!!\n",
		__func__, i_bat, t_bat, cycle_bat_now, cycle_bat_pre, real_soc);
	hisi_soh_err("[%s-%d]is called!!\n", __func__, __LINE__);
	return 0;
}
/*
 * 1 avg_data - acr_data > avg_data*5%, acr_data invalid.
 * 2 two acr data is invlid, check fail.
 */
static int soh_acr_valid_check(struct soh_acr_device *di,
	int acr_data[], int data_cnt)
{
	int invaid_cnt = 0;
	int i;
	int avg_data;
	int sum_data = 0;
	int data_error;
	int max, min;

	if (!di || data_cnt == 0) {
		hisi_soh_err("%s input para is err,cnt[%d]!\n",
			__func__, data_cnt);
		return -1;
	}

	for (i = 0; i < data_cnt; i++) {
		hisi_soh_debug("[%s] acr_data[%d]=%d\n",
			__func__, i, acr_data[i]);
		sum_data += acr_data[i];
	}
	/*
	 * get max and min,
	 * average calculation removes maximum and minimum values
	 */
	max_min_value(acr_data, CAL_CNT, &min, &max);
	avg_data = (sum_data - max - min) / (data_cnt - 2);

	data_error = avg_data / ACR_AVG_PERCENT;
	/*
	 * if invalid cnt is more than 2 by
	 * compare avg acr and sample value , acr is invalid
	 */
	for (i = 0; i < data_cnt; i++) {
		if (abs(avg_data - acr_data[i]) > data_error) {
			hisi_soh_err("[%s] data invalid,avg[%d],acr[%d],err[%d],cnt[%d]!\n",
				__func__, avg_data, acr_data[i],
				data_error, invaid_cnt);
			if (invaid_cnt++ > ACR_DATA_INVALID_NUM)
				return -1;
		}
	}

	/* get final acr value */
	di->soh_acr_info.batt_acr = avg_data;

	hisi_soh_info("[%s] acr valid, acr = [%d]!\n", __func__, avg_data);
	return 0;
}

static enum acr_cal_result  soh_acr_get_data(
	struct soh_acr_device *di, int *acr_data, int *acr_temp, int *err_cnt)
{
	int i;
	u32 acr_flag;
	u32 acr_ocp;
	int retry = ACR_RETRY_NUM;
	int acr_data_tmp;

	/*
	 * get CAL_CYCLE*data for acr calcuate,
	 * acr can start again after acr_flag is cleared
	 */
	for (i = 0; i < CAL_CNT; i++) {
		do {
			/* if acr fault occurrs , this acr cal is ended */
			if (di->acr_ops->get_acr_fault_status()) {
				hisi_soh_err("[%s] fault err, exit!\n", __func__);
				return ACR_ERROR;
			}
			/*
			 * if not get acr_flag,
			 * stop acr check forever by writting nv control,
			 * reset pmu stop discharge
			 */
			if (!retry) {
				hisi_soh_err("%s acr flag not occur in 200s, fatel err!\n",
					__func__);
				return ACR_FATAL;
			}
			acr_flag = di->acr_ops->get_acr_flag();
			msleep(ACR_CAL_TIMER);
			hisi_soh_err("[%s] wait acr flag cnt[%d]!\n",
				__func__, ACR_RETRY_NUM - retry);
			retry--;
		} while (!acr_flag);
		/* if acr ocp happen,this calculation is abandoned */
		acr_ocp = di->acr_ops->get_acr_ocp();
		if (acr_ocp) {
			hisi_soh_err("[%s] acr ocp err, exit!\n", __func__);
			return ACR_ERROR;
		}
		/* if chip temperature exceeds ,this calculation is abandoned */
		*(acr_temp + i) = di->acr_ops->get_acr_chip_temp();
		if (*(acr_temp + i) > ACR_CAL_CHIP_TEMP_MAX ||
			*(acr_temp + i) < ACR_CAL_CHIP_TEMP_MIN) {
			hisi_soh_err("[%s] acr chip temp[%d]c exceed the normal range,exit!!\n",
				__func__, *(acr_temp + i));
			return ACR_ERROR;
		}
		acr_data_tmp = di->acr_ops->calculate_acr();
		/* if get acr err,shedule task and recalculate */
		if (acr_data_tmp < 0) {
			/* if err count exceeds max err, cancel this calculate */
			if (++(*err_cnt) >= ACR_DATA_ERR_RETRY_CNT)
				*err_cnt  = 0;
			hisi_soh_err("[%s] acr data[%d] err,cnt =%d, exit!!\n",
				__func__, acr_data_tmp, *err_cnt);
			return ACR_ERROR;
		} else {
			hisi_soh_info("[%s] acr data suc, acr_data_err_cnt =%d!!\n",
				__func__, *err_cnt);
			*(acr_data + i) = acr_data_tmp;
		}
		/*
		 * last check need disable acr before clear acr flag,
		 * otherwise discharging don't stop
		 */
		if (i == CAL_CNT - 1) {
			di->acr_ops->enable_acr(0);
			di->acr_ops->clear_acr_flag();
			hisi_soh_debug("[%s] cnt = %d, last check need disable arc and clear acr flag!\n",
				__func__, i);
		} else {
			/* clear acr flag, start next check */
			di->acr_ops->clear_acr_flag();
		}
		hisi_soh_info("[%s]cnt=[%d],acr=[%d],chip_temp=[%d]!\n",
			__func__, i, *(acr_data + i), *(acr_temp + i));
	}
	hisi_soh_info("[%s] normal!\n", __func__);
	return ACR_NORMAL;
}

/*
 * calculate acr ,check acr validity and save data into nv.
 * 1 acr flag indicates that one acr detection is complete.
 * 2 acr detection starts again after acr flag is cleared.
 * 3 acr cal work is stopped if
 *   acr data err is more than ACR_DATA_ERR_RETRY_CNT.
 * 4 acr cal work is stopped if chip temperature exceeds .
 * 5 save the highest and lowest chip temperature
 * 6 acr data is saved to NV if cal is success.
 * 7 acr check is forbidden by nv forever if fatal err occurs.
 * 8 If an acr exception(140_deb/ldo1 ocp) occurs,
 *	it needs to be pulled down 20 ms before
 *	the acr io are pulled up.
 */
static void soh_acr_calculate_work(struct work_struct *work)
{
	struct soh_acr_device *di = container_of(work,
		struct soh_acr_device, acr_work.work);
	int ret;
	int acr_data[CAL_CNT + 1] = {0};
	int acr_temp[CAL_CNT + 1] = {0};
	static int acr_data_err_cnt;

	/* acr cal don't allow  pmu to enter eco model */
	soh_wake_lock(g_soh_di);

	/* get acr battery info */
	di->soh_acr_info.batt_vol   = coul_drv_battery_voltage();
	di->soh_acr_info.batt_temp  = coul_drv_battery_temperature();
	di->soh_acr_info.batt_cycle = coul_drv_battery_cycle_count();

	/*
	 * io enable must be closed before
	 * enabling Because of the chip requirements
	 */
	di->acr_ops->io_ctrl_acr_chip_en(0);
	di->acr_ops->io_ctrl_acr_chip_en(1);
	di->acr_ops->enable_acr(1);

	/* get acr data */
	ret = soh_acr_get_data(di,
		&acr_data[0], &acr_temp[0], &acr_data_err_cnt);
	if (ret == ACR_NORMAL) {
		acr_data_err_cnt = 0;
	} else if (ret == ACR_ERROR) {
		(void)memset_s(&di->soh_acr_info, sizeof(di->soh_acr_info),
			0, sizeof(di->soh_acr_info));
		goto acr_exit;
	} else if (ret == ACR_FATAL) {
		(void)memset_s(&di->soh_acr_info, sizeof(di->soh_acr_info),
			0, sizeof(di->soh_acr_info));
		goto fatal_acr_not_stop;
	}

	/* get acr chip temp */
	max_min_value(acr_temp, CAL_CNT,
		&di->soh_acr_info.chip_temp[0], &di->soh_acr_info.chip_temp[1]);

	/* check acr data validity and get final acr value. */
	ret = soh_acr_valid_check(di, acr_data, CAL_CNT);
	/* save acr data into nv */
	if (!ret) {
		if (di->acr_prec_type == ACR_H_PRECISION)
			soh_rw_nv_info(g_soh_di, SOH_ACR, NV_WRITE);
		else
			di->acr_prec_type = ACR_H_PRECISION;
		hisi_soh_info("[%s] acr cal success!!\n", __func__);
		sysfs_notify(&g_soh_di->dev->kobj, NULL, "acr_raw");
		hisi_call_soh_blocking_notifiers(EVT_SOH_ACR_UPDATE,
			(void *)&di->soh_acr_info);
	} else {
		(void)memset_s(&di->soh_acr_info, sizeof(di->soh_acr_info),
			0, sizeof(di->soh_acr_info));
	}

acr_exit:
	hisi_soh_err("%s acr exit!!\n", __func__);
	di->acr_ops->enable_acr(0);
	di->acr_ops->clear_acr_flag();
	di->acr_ops->clear_acr_ocp();
	di->acr_ops->clear_acr_fault_status();
	di->acr_ops->io_ctrl_acr_chip_en(0);
	soh_wake_unlock(g_soh_di);
	/* if data err,schedule work to recalcualte */
	if (acr_data_err_cnt) {
		queue_delayed_work(system_power_efficient_wq, &di->acr_work,
			round_jiffies_relative(
				msecs_to_jiffies(ACR_RETRY_ACR_MS)));
	} else {
		/* set acr work end */
		mutex_lock(&g_soh_di->soh_mutex);
		di->acr_work_flag = 0;
		mutex_unlock(&g_soh_di->soh_mutex);
	}

	return;
fatal_acr_not_stop:
	hisi_soh_err("%s acr fatal err!!\n", __func__);
	di->acr_ops->enable_acr(0);
	di->acr_ops->io_ctrl_acr_chip_en(0);
	/* acr check is forbidden forever if fatel err occurs */
	di->acr_nv.acr_control = 1;
	soh_rw_nv_info(g_soh_di, SOH_ACR, NV_WRITE);
	/* set acr work end */
	mutex_lock(&g_soh_di->soh_mutex);
	di->acr_work_flag = 0;
	mutex_unlock(&g_soh_di->soh_mutex);
	soh_wake_unlock(g_soh_di);
#ifdef CONFIG_HISI_BB
	rdr_syserr_process_for_ap(MODID_AP_S_PMU, 0, 0);
#else
	machine_restart("AP_S_PMU");
#endif
}

/*  respond the acr events from coul core */
static int soh_acr_notifier_call(struct notifier_block *acr_nb,
	unsigned long event, void *data)
{
	struct soh_acr_device *di = container_of(acr_nb,
		struct soh_acr_device, soh_acr_notify);
	int allow_acr_work_flag;

	if ((event == BATT_SOH_ACR) && g_soh_di) {
		hisi_soh_err("[%s-%d]is called!!\n", __func__, __LINE__);
		if (soh_acr_start_check() == 0) {
			/* acr and dcr mutex */
			mutex_lock(&g_soh_di->soh_mutex);
			allow_acr_work_flag = g_soh_di->soh_dcr_dev.dcr_work_flag;
#ifdef CONFIG_BATT_EIS
			allow_acr_work_flag = allow_acr_work_flag || get_freq_work_flag();
#endif
			if (allow_acr_work_flag) {
				/*
				 * if dcr is working,
				 * acr will wait for next check
				 */
				mutex_unlock(&g_soh_di->soh_mutex);
				hisi_soh_err("[%s] dcr or eis_freq is working!!\n",
					__func__);
				return NOTIFY_OK;
			}
			/* set acr working flag */
			di->acr_work_flag = 1;
			mutex_unlock(&g_soh_di->soh_mutex);

			di->acr_prec_type = ACR_H_PRECISION;
			queue_delayed_work(system_power_efficient_wq,
				&di->acr_work, msecs_to_jiffies(0));
		}
	}
	return NOTIFY_OK;
}

static int soh_acr_init(struct hisi_soh_device *di)
{
	int ret;
	errno_t err;

	if (!di)
		return -1;

	/* get dts */
	if (parse_soh_acr_dts(di))
		return -1;

	/* if dts not support,return success */
	if (!di->soh_acr_dev.acr_support) {
		hisi_soh_err("%s not support soh acr!\n", __func__);
		return 0;
	}

	/* battery cycle increment */
	di->soh_acr_dev.acr_cycle_inc = ACR_CAL_BAT_CYCLE_INC;
	err = memcpy_s(&di->soh_acr_dev.acr_nv,
		sizeof(di->soh_acr_dev.acr_nv), boot_acr_nv_info,
		sizeof(*boot_acr_nv_info));
	if (err != EOK) {
		hisi_soh_err("[%s]memcpy_s fail, err=%d\n", __func__, err);
		return -1;
	}
	soh_nv_printf(di, SOH_ACR);
	/* acr nv close */
	if (di->soh_acr_dev.acr_nv.acr_control) {
		hisi_soh_err("[%s] nv close acr!\n", __func__);
		return 0;
	}
	/* acr first cal, order number init */
	if (!di->soh_acr_dev.acr_nv.soh_nv_acr_info[0].batt_acr) {
		di->soh_acr_dev.acr_nv.order_num = -1;
		hisi_soh_err("[%s] order num init!\n", __func__);
	}

	/* check drv ops register */
	if (soh_drv_ops_check(di, ACR_DRV_OPS))
		return -1;

	/* register notifier for coul core */
	di->soh_acr_dev.soh_acr_notify.notifier_call = soh_acr_notifier_call;
	ret = coul_register_blocking_notifier(
		&di->soh_acr_dev.soh_acr_notify);
	if (ret < 0) {
		hisi_soh_err("[%s]soh_acr_register_notifier failed!\n", __func__);
		return -1;
	}

	/* Init interrupt notifier work */
	INIT_DELAYED_WORK(&di->soh_acr_dev.acr_work, soh_acr_calculate_work);

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

static int soh_acr_uninit(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	/* if dts not support,return success */
	if (!di->soh_acr_dev.acr_support) {
		hisi_soh_err("%s not support soh acr!\n", __func__);
		return 0;
	}

	coul_unregister_blocking_notifier(&di->soh_acr_dev.soh_acr_notify);

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

/* respond the ovp events from soh driver */
static int soh_ovp_notifier_call(struct notifier_block *soh_ovp_nb,
	unsigned long event, void *data)
{
	struct soh_ovp_device *di = container_of(soh_ovp_nb,
		struct soh_ovp_device, soh_ovp_notify);

	di->ovp_type = event;

	/* if dts not support,return success */
	if (!di->ovp_support) {
		hisi_soh_err("[%s] not support soh ovp!\n", __func__);
		return NOTIFY_OK;
	}
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_ovp_work, msecs_to_jiffies(0));

	return NOTIFY_OK;
}

static int soh_ovp_exit_check(struct soh_ovp_device *di)
{
	int v_bat;
	int t_bat;
	int i;

	if (!di)
		return -1;

	t_bat = coul_drv_battery_temperature();
	v_bat = coul_drv_battery_voltage();

	for (i = 0; i < SOH_OVL_THRED_NUM; i++) {
		if (t_bat <= di->soh_ovl_thres[i].temp &&
			v_bat <= di->soh_ovl_thres[i].bat_vol_mv) {
			hisi_soh_err("[%s] ovp exit, temp = [%d], vol = [%d]!\n",
				__func__, t_bat, v_bat);
			return 1;
		}
	}

	hisi_soh_info("[%s] in ovp, temp = [%d], vol = [%d]!\n",
		__func__, t_bat, v_bat);
	return 0;
}

/*
 * 1 ovl check  is valid by chip after discharge starts .
 * 2 ovh check  is valid by chip after discharge stop.
 */
static void soh_ovp_check_delayed_work(struct work_struct *work)
{
	struct soh_ovp_device *di = container_of(work,
		struct soh_ovp_device, soh_ovp_work.work);
	static int ovh_start_time_s;
	static int ovp_dischg_flag;
	int ovh_now_time_s;
	int ovp_state;

	hisi_soh_info("[%s] ovp type = [%d]!\n", __func__, di->ovp_type);

	/* get ovh enter time */
	if (!ovh_start_time_s)
		ovh_start_time_s = hisi_getcurtime() / NSEC_PER_SEC;

	if (di->ovp_type == SOH_OVH) {
		/* ovh keep system awake for Power consumption to lower voltage */
		soh_wake_lock(g_soh_di);

		ovh_now_time_s = hisi_getcurtime() / NSEC_PER_SEC;
		/* ovh is more than 30min, start discharge */
		if ((ovh_now_time_s - ovh_start_time_s >
			di->soh_ovp_start_time * SEC_PER_MIN) &&
			!ovp_dischg_flag) {
			ovp_dischg_flag = 1;
			di->soh_ovp_ops->enable_dischg(1);
			msleep(OVP_DISCHARGE_MIN_MS);
			hisi_soh_err("[%s] ovh is more than 30 min, discharge!\n",
				__func__);
		}
		ovp_state = soh_ovp_exit_check(di);
		/*
		 * if ovh time is less than 30 min and
		 * vol and temp is back to normal, work id ended
		 */
		if (ovp_state && !ovp_dischg_flag) {
			hisi_soh_err("[%s] ovh not dischg, vol is back to Normal,work end!\n",
				__func__);
			/*
			 * ovp need to be enabled,
			 * if ovh generate but not discharge
			 */
			goto force_stop_dicharge;
		}
		/*
		 * if ovh time is more than 30 min and vol and
		 * temp is back to normal, discharge will be forced to stop
		 */
		if (ovp_state && ovp_dischg_flag) {
			hisi_soh_err("[%s] ovh dischg, vol is back to normal, force to stop discharge!\n",
				__func__);
			goto force_stop_dicharge;
		}
		goto continue_check;
	} else { /* SOH_OVL */
		/*
		 * soh work will be ended if
		 * soh ovl occurs and discharge is stopped automatically.
		 */
		if (di->soh_ovp_ops->get_stop_dischg_state()) {
			ovp_dischg_flag = 0;
			ovh_start_time_s = 0;
			soh_wake_unlock(g_soh_di);
			hisi_soh_err("[%s] soh ovl irq ,auto stop discharge,end work!\n",
				__func__);
			return;
		}
		/*
		 * discharge will be fored to stop
		 * if discharge isn't stopped automatically.
		 */
		hisi_soh_err("[%s] soh ovl irq ,auto stop dischg fail!\n",
			__func__);
		goto force_stop_dicharge;
	}
force_stop_dicharge:
	di->soh_ovp_ops->enable_dischg(0);
	/* enable ovp because enalbe_dischg will disable ovp after 30ms */
	msleep(30);
	di->soh_ovp_ops->enable_ovp(1);
	ovh_start_time_s = 0;
	ovp_dischg_flag = 0;
	soh_wake_unlock(g_soh_di);
	hisi_soh_err("[%s] force stop dischage!\n", __func__);
	return;
continue_check:
	hisi_soh_info("[%s] continue check!\n", __func__);
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_ovp_work, msecs_to_jiffies(OVP_WORK_DELAY_MS));
}

/* set vol and temp (ovh and ovl) threshold  */
void soh_ovp_set_protect_threshold(struct hisi_soh_device *di)
{
	int  ovh_fifo_cnt;
	int ret;

	if (!di)
		return;

	/* set ovp start threshold */
	ovh_fifo_cnt = di->soh_ovp_dev.soh_ovp_ops->get_ovh_thred_cnt();
	ovh_fifo_cnt = min(ovh_fifo_cnt, SOH_OVH_THRED_NUM);
	ret = di->soh_ovp_dev.soh_ovp_ops->set_ovp_threshold(
			SOH_OVH, &di->soh_ovp_dev.soh_ovh_thres[0],
			ovh_fifo_cnt);
	if (ret)
		hisi_soh_err("[%s] set ovp start thrd fail!\n", __func__);

	/* set ovp stop safe threshold */
	ret = di->soh_ovp_dev.soh_ovp_ops->set_ovp_threshold(
			SOH_OVL, &di->soh_ovp_dev.soh_ovl_safe_thres, 1);
	if (ret)
		hisi_soh_err("[%s] set ovp safe thrd fail!\n", __func__);
	else
		hisi_soh_info("[%s] suc!\n", __func__);
}

static int soh_ovp_init(struct hisi_soh_device *di)
{
	int ret;

	if (!di)
		return -1;

	/* get dts */
	if (parse_soh_ovp_dts(di))
		return -1;

	/* if dts not support,return success */
	if (!di->soh_ovp_dev.ovp_support) {
		hisi_soh_err("%s not support soh ovp!\n", __func__);
		return 0;
	}

	/* check drv ops register */
	if (soh_drv_ops_check(di, SOH_OVP_DRV_OPS))
		return -1;

	/* soh ovp register notifier for soh driver */
	di->soh_ovp_dev.soh_ovp_notify.notifier_call =
			soh_ovp_notifier_call;
	ret = hisi_soh_drv_register_atomic_notifier(
			&di->soh_ovp_dev.soh_ovp_notify);
	if (ret < 0) {
		hisi_soh_err("[%s]soh_ovp_register_notifier failed!\n", __func__);
		return -1;
	}

	/* Init interrupt notifier work */
	INIT_DELAYED_WORK(&di->soh_ovp_dev.soh_ovp_work,
			soh_ovp_check_delayed_work);

	/* set ovh and ovl gear */
	soh_ovp_set_protect_threshold(di);

	/* set ovp enable */
	di->soh_ovp_dev.soh_ovp_ops->enable_ovp(1);

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

static int soh_ovp_uninit(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	/* if dts not support,return success */
	if (!di->soh_ovp_dev.ovp_support) {
		hisi_soh_err("%s not support soh ovp!\n", __func__);
		return 0;
	}

	hisi_soh_drv_unregister_atomic_notifier(
				&di->soh_ovp_dev.soh_ovp_notify);

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

int soh_dcr_start_check(void)
{
	int t_bat;
	int i_bat;
	int cycle_bat_now;
	int cycle_bat_pre;
	int real_soc;
	struct hisi_soh_device *di = g_soh_di;

	if (!di) {
		hisi_soh_err("%s di is NULL!\n", __func__);
		return -1;
	}

	/* if dts not support,return success */
	if (!di->soh_dcr_dev.dcr_support) {
		hisi_soh_err("%s not support soh acr!\n", __func__);
		return -1;
	}
	/* dcr nv close */
	if (di->soh_dcr_dev.dcr_nv.dcr_control) {
		hisi_soh_err("[%s] nv close dcr!\n", __func__);
		return -1;
	}

	t_bat         = coul_drv_battery_temperature();
	i_bat         = coul_drv_battery_current();
	cycle_bat_now = coul_drv_battery_cycle_count();

	/* Prevent array Overflow */
	if (di->soh_dcr_dev.dcr_nv.order_num < 0 ||
		di->soh_dcr_dev.dcr_nv.order_num >= SOH_DCR_NV_DATA_NUM)
		di->soh_dcr_dev.dcr_nv.order_num = -1;

	if (di->soh_dcr_dev.dcr_nv.order_num == -1)
		cycle_bat_pre = 0;
	else
		cycle_bat_pre = di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[
			di->soh_dcr_dev.dcr_nv.order_num].batt_cycle;

	real_soc = coul_drv_battery_unfiltered_capacity();

	/* check battery temperature */
	if (t_bat > DCR_CAL_BAT_TEMP_MAX || t_bat <  DCR_CAL_BAT_TEMP_MIN) {
		hisi_soh_err("%s bat temp[%d]c  exceed the normal range!!\n",
			__func__, t_bat);
		return -1;
	}

	di->soh_dcr_dev.soh_dcr_info.batt_temp = t_bat;

	/* check battery now current, Consider current accuracy error */
	if (abs(i_bat) > DCR_CAL_BAT_CUR_INC_MAX) {
		hisi_soh_err("%s bat current[%d] mA exceed the normal range!!\n",
			__func__, i_bat);
		return -1;
	}

	/* check battery cycle */
	if (abs(cycle_bat_now - cycle_bat_pre) < di->soh_dcr_dev.dcr_cycle_inc) {
		hisi_soh_err("%s cycle inc not enough, pre = [%d], now = [%d]!!\n",
			__func__, cycle_bat_pre, cycle_bat_now);
		return -1;
	}
	di->soh_dcr_dev.soh_dcr_info.batt_cycle = cycle_bat_now;

	/* check battery real soc */
	if (real_soc  < DCR_CAL_BAT_SOC_MIN)
		hisi_soh_err("%s read soc low, real_soc = [%d]!!\n",
			__func__, real_soc);

	di->soh_dcr_dev.soh_dcr_info.batt_cycle = cycle_bat_now;

	/* check battery real soc */
	if (real_soc  < DCR_CAL_BAT_SOC_MIN)
		hisi_soh_err("%s read soc low  ,real_soc = [%d]!!\n",
			__func__, real_soc);

	hisi_soh_info("[%s]sucess, ibat = [%d], tbat = [%d], cycle_now = [%d], cycle_pre = [%d], soc =[%d]!!\n",
		__func__, i_bat, t_bat, cycle_bat_now,
		cycle_bat_pre, real_soc);
	return 0;
}

/*
 * 1 fifo current    [180, 220]mA
 * 2 fifo vol diff   [-10, 10]mV
 */
STATIC int soh_dcr_get_data2(int *i_ma, int *v_mv)
{
	struct hisi_soh_device *di = g_soh_di;
	unsigned int fifo_depth;
	int i;
	int vol_mv = 0;
	int cur_ma = 0;
	int last_vol_mv = 0;
	int ret;

	if (!di) {
		hisi_soh_err("[%s]di is NULL!\n", __func__);
		return -1;
	}
	fifo_depth = di->soh_dcr_dev.dcr_ops->get_dcr_fifo_depth();
	if (fifo_depth > DCR_FIFO_MAX)
		return -1;

	for (i = fifo_depth - 1; i >= 0; i--) {
		ret = di->soh_dcr_dev.dcr_ops->get_dcr_info(
			&cur_ma, &vol_mv, i);
		if (ret)
			return -1;
		/* check fifo current */
		if (abs(cur_ma - DCR_CAL_DISCHARGE_BASE_CUR) >
			DCR_CAL_BAT_CUR_INC_MAX) {
			hisi_soh_err("[%s] dcr fifo current[%d]mA ,exceed the normal range!!\n",
				__func__, cur_ma);
			return -1;
		}
		/* check fifo vol */
		if (last_vol_mv  && abs(last_vol_mv - vol_mv) >
			DCR_CAL_BAT_VOL_INC_MAX) {
			hisi_soh_err("[%s] dcr fifo vol[%d]mV ,exceed the normal range!!\n",
				__func__, vol_mv);
			return -1;
		}
		last_vol_mv = vol_mv;
	}

	*i_ma = cur_ma;
	*v_mv = vol_mv;
	hisi_soh_debug("[%s] current = %d ma, vol = %d mv!!\n",
		__func__, *i_ma, *v_mv);
	return 0;
}

/*
 * 1 fifo current    [180, 220]mA
 * 2 fifo vol diff   [-10, 10]mV
 */
STATIC int soh_dcr_get_data1(int *i_ma, int *v_mv)
{
	struct hisi_soh_device *di = g_soh_di;
	int vol_mv, cur_ma;
	int ret;

	if (!di) {
		hisi_soh_err("[%s]di is NULL!\n", __func__);
		return -1;
	}

	ret = di->soh_dcr_dev.dcr_ops->get_dcr_data1(&cur_ma, &vol_mv);
	/* get from coul */
	if (ret) {
		vol_mv = coul_drv_battery_voltage();
		cur_ma = coul_drv_battery_current();
	}
	/* check fifo current */
	if (abs(cur_ma - DCR_CAL_DISCHARGE_BASE_CUR) >
		DCR_CAL_BAT_CUR_INC_MAX) {
		hisi_soh_err("[%s] dcr fifo current[%d]mA ,exceed the normal range!!\n",
			__func__, cur_ma);
		return -1;
	}

	*i_ma = cur_ma;
	*v_mv = vol_mv;
	hisi_soh_debug("[%s] current = %d ma, vol = %d mv!!\n",
		__func__, *i_ma, *v_mv);

	return 0;
}

static int soh_dcr_wait_flag(struct soh_dcr_device *di)
{
	int retry = DCR_RETRY_NUM; /* 500s */
	u32 dcr_flag;
	int i_check_cycle = 0;
	int t_bat;
	int t_bat2;

	/* get dcr flag */
	do {
		if (!retry) {
			hisi_soh_err("[%s] dcr flag not occur in 500s, fatel err!\n",
				__func__);
			goto fatal_dcr_not_stop;
		}

		dcr_flag = di->dcr_ops->get_dcr_flag();

		t_bat = coul_drv_battery_temperature();
		/* check battery temperature */
		if (t_bat > DCR_CAL_BAT_TEMP_MAX ||
			t_bat < DCR_CAL_BAT_TEMP_MIN) {
			hisi_soh_err("[%s] bat temp[%d]c exceed the normal range, exit!!\n",
				__func__, t_bat);
			return DCR_ERROR;
		}

		/* check battery now current, Consider current accuracy error */
		t_bat2 = coul_drv_battery_current();
		if (abs(t_bat2 - DCR_CAL_DISCHARGE_BASE_CUR) >
			DCR_CAL_BAT_CUR_INC_MAX) {
			hisi_soh_err("[%s] bat current[%d]mA ,exceed the normal range!!\n",
				__func__, t_bat2);
			if (i_check_cycle++ > DCR_CURRENT_EXCD_MAX_CYCLE)
				return DCR_ERROR;
		}

		msleep(DCR_CHECK_FLAG_CYCLE_TIMER);
		hisi_soh_debug("[%s] dcr flag cnt[%d]!\n",
			__func__, DCR_RETRY_NUM - retry);
		retry--;
	} while (!dcr_flag);

	/* clear dcr flag for next check */
	di->dcr_ops->clear_dcr_flag();
	return DCR_SUCCES;
fatal_dcr_not_stop:
	hisi_soh_err("[%s] dcr fatal err!!\n", __func__);
	di->dcr_ops->enable_dcr(0);
	di->dcr_ops->clear_dcr_flag();
	/* set dcr work end */
	mutex_lock(&g_soh_di->soh_mutex);
	di->dcr_work_flag = 0;
	mutex_unlock(&g_soh_di->soh_mutex);
	/* acr check is forbidden forever if fatel err occurs */
	di->dcr_nv.dcr_control = 1;
	soh_rw_nv_info(g_soh_di, SOH_DCR, NV_WRITE);
	soh_wake_unlock(g_soh_di);
	return DCR_NOT_STOP;
}

/*
 * Function:        soh_dcr_calculate_work
 * Description:     calculate dcr ,check dcr validity and save data into nv.
 * Input:           struct work_struct *work
 * Output:          NULL
 * Return:          NULL
 * Remark:          1 the dcr detection is complete by dcr flag.
 *                  2 dcr data is saved to NV if cal is success.
 *                  dcr will be forbidden forever by
 *                   write nv if it doesn't stop in within
 *                   the specified time,
 *                  4 battery current        [180mA,220mA] in dcr discharge.
 *                  5 battery temp           [0,55] in dcr discharge.
 *                  6 battery temp change    [-5,5] in dcr discharge.
 *                  7 battery current by cc  [180mA,220mA]
 *                    in dcr discharge,it'll exit if err is
 *                    more than DCR_CURRENT_EXCD_MAX_CYCLE.
 */
static void soh_dcr_calculate_work(struct work_struct *work)
{
	struct soh_dcr_device *di = container_of(work,
		struct soh_dcr_device, dcr_work.work);

	int ret;
	int dcr_t;
	int cc_cur;
	int bat_val[BAT_NUM] = {0};

	if (!di) {
		hisi_soh_err("%s di is NULL!\n", __func__);
		return;
	}

	/* set dcr sample timer */
	di->dcr_ops->set_dcr_timer(DCR_TIMER_32);

	dcr_t = di->dcr_ops->get_dcr_timer();
	if (!dcr_t) {
		/* set dcr work end */
		mutex_lock(&g_soh_di->soh_mutex);
		di->dcr_work_flag = 0;
		mutex_unlock(&g_soh_di->soh_mutex);
		hisi_soh_err("[%s] dcr_t = 0\n", __func__);
		return;
	}
	/* acr cal don't allow  pmu to enter eco model */
	soh_wake_lock(g_soh_di);

	/* get bat info before dcr starts */
	bat_val[T_BAT0] = coul_drv_battery_temperature();
	bat_val[V_BAT0] = coul_drv_battery_voltage();
	bat_val[C_BAT0] = coul_drv_battery_cc_uah();

	/* enable dcr check */
	di->dcr_ops->enable_dcr(1);

	/* coul vol sample is 300ms */
	msleep(DCR_R0_WATI_MS);

	/* get r0 cal second vol */
	ret = soh_dcr_get_data1(&bat_val[I_BAT1], &bat_val[V_BAT1]);
	if (ret) {
		hisi_soh_err("[%s] get dcr data1 error %d !\n", __func__, ret);
		goto dcr_exit;
	}
	ret = soh_dcr_wait_flag(di);
	if (ret < 0)
		goto dcr_exit;
	else if (ret)
		return;
	/* get bat info after dcr ends */
	bat_val[C_BAT1] = coul_drv_battery_cc_uah();
	bat_val[T_BAT1] = coul_drv_battery_temperature();

	/* get dcr data2 */
	ret = soh_dcr_get_data2(&bat_val[I_BAT2], &bat_val[V_BAT2]);
	if (ret) {
		hisi_soh_err("[%s] get dcr data2 error %d, exit!\n",
			__func__, ret);
		goto dcr_exit;
	}

	hisi_soh_info("[%s] v_bat0[%d]mv, v_bat1[%d]mv, v_bat2[%d]mv, c_bat0[%d]mah, c_bat1[%d]mah, t_bat0[%d], t_bat1[%d]!\n",
		__func__, bat_val[V_BAT0], bat_val[V_BAT1], bat_val[V_BAT2],
		bat_val[C_BAT0], bat_val[C_BAT1],
		bat_val[T_BAT0], bat_val[T_BAT1]);

	/* check dcr info valid by cc */
	cc_cur = abs((bat_val[C_BAT1] - bat_val[C_BAT0]) *
		SEC_PER_H / (dcr_t * UAH_PER_MAH));
	if (abs(cc_cur - DCR_CAL_DISCHARGE_BASE_CUR) >
		DCR_CAL_BAT_CUR_INC_MAX_BY_CC) {
		hisi_soh_err("[%s] cc_cur = [%d]mA,cc1[%d] - cc0[%d] current by cc exceed the normal range, exit!\n",
			__func__, cc_cur, bat_val[C_BAT1], bat_val[C_BAT0]);
		goto dcr_exit;
	}

	hisi_soh_info("[%s]cc_cur = [%d]mA,cc0 = [%d]uah, cc1 = [%d]\n",
		__func__, cc_cur, bat_val[C_BAT0], bat_val[C_BAT1]);

	/* check dcr temp valid */
	if (abs(bat_val[T_BAT1] - bat_val[T_BAT0]) > DCR_CAL_BAT_TEMP_DIFF) {
		hisi_soh_err("[%s] t1[%d]-t0[%d] temp range is exceed the range,exit!\n",
			__func__, bat_val[T_BAT1], bat_val[T_BAT0]);
		goto dcr_exit;
	}

	/* cal dcr */
	if (cc_cur)
		di->soh_dcr_info.batt_dcr =
		(bat_val[V_BAT0] - bat_val[V_BAT2])*MOHM_PER_OHM / cc_cur;
	else
		di->soh_dcr_info.batt_dcr = 0;

	/* cal r0 */
	di->soh_dcr_info.batt_r0 =
		(bat_val[V_BAT0] - bat_val[V_BAT1]) *
		MOHM_PER_OHM / DCR_CAL_DISCHARGE_BASE_CUR;

	/* get dcr other info */
	di->soh_dcr_info.batt_cycle = coul_drv_battery_cycle_count();
	di->soh_dcr_info.batt_vol   = bat_val[V_BAT0];
	di->soh_dcr_info.batt_temp  = bat_val[T_BAT0];

	hisi_soh_info("[%s] r0 =[%d], dcr = [%d]\n",
		__func__, di->soh_dcr_info.batt_r0,
		di->soh_dcr_info.batt_dcr);
	/* save acr data into nv */
	soh_rw_nv_info(g_soh_di, SOH_DCR, NV_WRITE);
	sysfs_notify(&g_soh_di->dev->kobj, NULL, "dcr_raw");
	hisi_call_soh_blocking_notifiers(EVT_SOH_DCR_UPDATE,
		(void *)&di->soh_dcr_info);
	hisi_soh_info("[%s] dcr cal success!!\n", __func__);

dcr_exit:
	hisi_soh_info("[%s] dcr exit!!\n", __func__);
	di->dcr_ops->enable_dcr(0);
	di->dcr_ops->clear_dcr_flag();
	/* set dcr work end */
	mutex_lock(&g_soh_di->soh_mutex);
	di->dcr_work_flag = 0;
	mutex_unlock(&g_soh_di->soh_mutex);
	soh_wake_unlock(g_soh_di);
}

/* respond the dcr events from coul core */
static int soh_dcr_notifier_call(
	struct notifier_block *dcr_nb, unsigned long event, void *data)
{
	struct soh_dcr_device *di = container_of(dcr_nb,
		struct soh_dcr_device, soh_dcr_notify);
	int allow_dcr_work_flag;

	if ((event == BATT_SOH_DCR) && g_soh_di) {
		if (soh_dcr_start_check() == 0) {
			/* acr and dcr mutex */
			mutex_lock(&g_soh_di->soh_mutex);
			allow_dcr_work_flag = g_soh_di->soh_acr_dev.acr_work_flag;
#ifdef CONFIG_BATT_EIS
			allow_dcr_work_flag = allow_dcr_work_flag || get_freq_work_flag();
#endif
			if (allow_dcr_work_flag) {
				/*
				 * if acr is working,
				 * acr will wait for next check
				 */
				mutex_unlock(&g_soh_di->soh_mutex);
				hisi_soh_err("[%s] acr or eis_freq is working,wait for next!!\n",
					__func__);
				return NOTIFY_OK;
			}
			/* set dcr working flag */
			di->dcr_work_flag = 1;
			mutex_unlock(&g_soh_di->soh_mutex);
			queue_delayed_work(system_power_efficient_wq,
				&di->dcr_work, msecs_to_jiffies(0));
		}
	}
	return NOTIFY_OK;
}

static int soh_dcr_init(struct hisi_soh_device *di)
{
	int ret;
	errno_t err;

	if (!di)
		return -1;

	/* get dts */
	if (parse_soh_dcr_dts(di))
		return -1;

	/* if dts not support,return success */
	if (!di->soh_dcr_dev.dcr_support) {
		hisi_soh_err("%s not support soh dcr!\n", __func__);
		return 0;
	}

	/* battery cycle increment */
	di->soh_dcr_dev.dcr_cycle_inc = DCR_CAL_BAT_CYCLE_INC;
	err = memcpy_s(&di->soh_dcr_dev.dcr_nv,
		sizeof(di->soh_dcr_dev.dcr_nv), boot_dcr_nv_info,
		sizeof(*boot_dcr_nv_info));
	if (err != EOK) {
		hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
			__func__, err);
		return -1;
	}
	soh_nv_printf(di, SOH_DCR);

	/* dcr nv close */
	if (di->soh_dcr_dev.dcr_nv.dcr_control) {
		hisi_soh_err("[%s] nv close dcr!\n", __func__);
		return 0;
	}

	if (!di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[0].batt_dcr) {
		/* dcr first cal, order number init */
		di->soh_dcr_dev.dcr_nv.order_num = -1;
		hisi_soh_err("[%s] order num init!\n", __func__);
	}

	/* check drv ops register */
	if (soh_drv_ops_check(di, DCR_DRV_OPS))
		return -1;

	/* register notifier for coul core */
	di->soh_dcr_dev.soh_dcr_notify.notifier_call = soh_dcr_notifier_call;
	ret = coul_register_blocking_notifier(
		&di->soh_dcr_dev.soh_dcr_notify);
	if (ret < 0) {
		hisi_soh_err("[%s]soh_dcr_register_notifier failed!\n",
			__func__);
		return -1;
	}
	/* set dcr sample timer */
	di->soh_dcr_dev.dcr_ops->set_dcr_timer(DCR_TIMER_32);

	/* Init interrupt notifier work */
	INIT_DELAYED_WORK(&di->soh_dcr_dev.dcr_work, soh_dcr_calculate_work);
	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

static int soh_dcr_uninit(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	/* if dts not support,return success */
	if (!di->soh_dcr_dev.dcr_support) {
		hisi_soh_err("%s not support soh dcr!\n", __func__);
		return 0;
	}

	coul_unregister_blocking_notifier(&di->soh_dcr_dev.soh_dcr_notify);

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

static void soh_get_pd_data(struct hisi_soh_device *di,
	const struct pd_leak_chip_info *max_value,
	const struct pd_leak_chip_info *min_value)
{
	/* get pd leak cal data */
	if (!di || !max_value || !min_value)
		return;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_current[0] =
		min_value->ocv_cur_ua;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_current[1] =
		max_value->ocv_cur_ua;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_vol[0] =
		min_value->ocv_vol_uv;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_vol[1] =
		max_value->ocv_vol_uv;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.chip_temp[0] =
		min_value->ocv_chip_temp;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.chip_temp[1] =
		max_value->ocv_chip_temp;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.rtc_time[0] =
		min_value->ocv_rtc;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.rtc_time[1] =
		max_value->ocv_rtc;
	di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_cycle =
		coul_drv_battery_cycle_count();
}

static void soh_log_pd_info(const struct hisi_soh_device *di,
	const int leak_cc_ua)
{
	if (!di)
		return;
#ifdef CONFIG_HUAWEI_DUBAI
	HWDUBAI_LOGE("DUBAI_TAG_POWER_OFF_LEAK", "lcurr=%d cycle=%d temp0=%d temp1=%d bcurr0=%d bcurr1=%d "
		     "bvol0=%d bvol1=%d rtc0=%d rtc1=%d cc=%d",
		     leak_cc_ua,
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_cycle,
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.chip_temp[0],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.chip_temp[1],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_current[0],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_current[1],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_vol[0],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.batt_vol[1],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.rtc_time[0],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.rtc_time[1],
		     di->soh_pd_leak_dev.soh_pd_leak_current_info.sys_pd_leak_cc);
#endif
}
/*
 * 1 ocv [2500,4500]mv
 * 2 The greater the ocv, the shorter the corresponding rtc time
 */
static int soh_pd_get_valid_data_cal_pd_current(
	struct hisi_soh_device *di, struct pd_leak_chip_info *pdata, u32 size)
{
	u32 i;
	int ret;
	int ocv_val[MAX_DATA] = {0};
	struct pd_leak_chip_info *max_value = NULL;
	struct pd_leak_chip_info *min_value = NULL;

	if (!pdata ||
		!di || size < PD_OCV_CAL_MIN_NUM) {
		hisi_soh_err("[%s] not start pd ocv sample, size = %d",
			__func__, size);
		return -1;
	}

	max_value = pdata;
	min_value = pdata;

	for (i = 0; i < size; i++) {
		if ((pdata + i)->ocv_cur_ua > max_value->ocv_cur_ua) {
			max_value = pdata + i;
			ocv_val[MAX_NUM] = i;
		}

		if ((pdata + i)->ocv_cur_ua <= min_value->ocv_cur_ua) {
			min_value = pdata + i;
			ocv_val[MIN_NUM] = i;
		}
	}
	soh_get_pd_data(di, max_value, min_value);
	hisi_soh_info("[%s] [num:%d]max_current = %d, [num:%d]min_current = %d!\n",
		__func__, ocv_val[MAX_NUM], max_value->ocv_cur_ua,
		ocv_val[MIN_NUM], min_value->ocv_cur_ua);

	/* check ocv valid */
	if ((min_value->ocv_vol_uv / UV_PER_MV > PD_OCV_MAX ||
		min_value->ocv_vol_uv / UV_PER_MV < PD_OCV_MIN) ||
		(max_value->ocv_vol_uv / UV_PER_MV > PD_OCV_MAX ||
		max_value->ocv_vol_uv / UV_PER_MV < PD_OCV_MIN)) {
		hisi_soh_err("[%s] err:ocv is wrong, min = %d, max = %d uv!\n",
			__func__, min_value->ocv_vol_uv, max_value->ocv_vol_uv);
		return -1;
	}

	hisi_soh_info("[%s] [num:%d].ocv = %d, .temp = %d, [num:%d].ocv = %d, .temp = %d!\n",
		__func__, ocv_val[MAX_NUM], max_value->ocv_vol_uv,
		max_value->ocv_chip_temp, ocv_val[MIN_NUM],
		min_value->ocv_vol_uv, min_value->ocv_chip_temp);

	/* get uah by ocv */
	ret  =  coul_drv_cal_uah_by_ocv(
		max_value->ocv_vol_uv, &ocv_val[MAX_CUR_OCV_UAH]);
	ret  =  ret || coul_drv_cal_uah_by_ocv(
		min_value->ocv_vol_uv, &ocv_val[MIN_CUR_OCV_UAH]);
	if (ret) {
		hisi_soh_err("[%s]cal uah fail\n", __func__);
		return -1;
	}
	/* cal pd leak current ma */
	ocv_val[OCV_INT] = abs(max_value->ocv_rtc - min_value->ocv_rtc);
	ocv_val[SYS_LEAK_CC]  =
		di->soh_pd_leak_dev.soh_pd_leak_current_info.sys_pd_leak_cc;

	if (ocv_val[OCV_INT])
		ocv_val[LEAK_CC_UA] = (abs(
			ocv_val[MAX_CUR_OCV_UAH] -
			ocv_val[MIN_CUR_OCV_UAH]) - ocv_val[SYS_LEAK_CC]) *
			SEC_PER_H / ocv_val[OCV_INT];

	di->soh_pd_leak_dev.soh_pd_leak_current_info.leak_current_ma =
		ocv_val[LEAK_CC_UA] / UA_PER_MA;
	soh_log_pd_info(di, ocv_val[LEAK_CC_UA]);
	hisi_soh_info("[%s] suc, ocv_interval_s = %d, pd_current = [%d]mA!\n",
		__func__, ocv_val[OCV_INT], ocv_val[LEAK_CC_UA] / UA_PER_MA);
	return 0;
}

static void soh_pd_leak_cal(struct hisi_soh_device *di)
{
	u32 fifo_cnt;
	u32 i;
	int ret;
	struct pd_leak_chip_info *pd_chip_data = NULL;

	if (!di)
		return;

	/* get fifo count of pd current and vol */
	fifo_cnt =  di->soh_pd_leak_dev.pd_leak_ops->get_pd_leak_fifo_depth();
	if (fifo_cnt > PD_FIFO_MAX || !fifo_cnt) {
		hisi_soh_err("[%s] fifo err!\n", __func__);
		return;
	}
	pd_chip_data = kzalloc((sizeof(*pd_chip_data) * fifo_cnt),
						GFP_KERNEL);
	if (!pd_chip_data)
		return;

	/* get pd data from pd driver */
	for (i = 0; i < fifo_cnt; i++) {
		di->soh_pd_leak_dev.pd_leak_ops->get_pd_leak_info(
			pd_chip_data + i, i);
		if ((pd_chip_data + i)->ocv_vol_uv == 0) {
			hisi_soh_info("[%s] num:[%d]pd ocv = 0, exit!\n",
				__func__, i);
			break;
		}
	}
	/* cal pd leak current and save */
	ret = soh_pd_get_valid_data_cal_pd_current(di, pd_chip_data, i);
	if (!ret) {
		ret = soh_rw_nv_info(di, SOH_PD_LEAK, NV_WRITE);
		if (!ret)
			hisi_soh_info("[%s] suc!\n", __func__);
		else
			hisi_soh_err("[%s] nv w fail!\n", __func__);
	} else {
		hisi_soh_err("[%s] fail!\n", __func__);
	}
	kfree(pd_chip_data);
}

/*
 * alculate pd leak ,
 * check pd leak validity and save data into nv.
 */
static void soh_pd_leak_calculate_work(struct work_struct *work)
{
	struct hisi_soh_device *di = container_of(work,
		struct hisi_soh_device, soh_pd_leak_dev.pd_leak_work.work);
	/* get pd leak info */
	soh_pd_leak_cal(di);
}

static int soh_pd_leak_init(struct hisi_soh_device *di)
{
	errno_t err;

	if (!di)
		return -1;

	/* get dts */
	if (parse_soh_pd_leak_dts(di))
		return -1;

	/* if dts not support,return success */
	if (!di->soh_pd_leak_dev.pd_leak_support) {
		hisi_soh_err("%s not support soh pd leak!\n", __func__);
		return 0;
	}

	err = memcpy_s(&di->soh_pd_leak_dev.pd_leak_nv,
		sizeof(di->soh_pd_leak_dev.pd_leak_nv),
		boot_pdleak_nv_info,
		sizeof(*boot_pdleak_nv_info));
	if (err != EOK) {
		hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
		return -1;
	}
	soh_nv_printf(di, SOH_PD_LEAK);
	/* pd nv close */
	if (di->soh_pd_leak_dev.pd_leak_nv.pd_control) {
		hisi_soh_err("[%s] nv close pd!\n", __func__);
		return 0;
	}
	if (!di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[0].batt_vol[0]) {
		/* pd first cal, order number init */
		di->soh_pd_leak_dev.pd_leak_nv.order_num = -1;
		hisi_soh_err("[%s] order num init!\n", __func__);
	}

	/* check drv ops register */
	if (soh_drv_ops_check(di, PD_LEAK_DRV_OPS))
		return -1;

	/* enable pd ocv */
	di->soh_pd_leak_dev.pd_leak_ops->enable_pd_leak(SOH_EN);

	/* Init pd leak cal  work */
	INIT_DELAYED_WORK(&di->soh_pd_leak_dev.pd_leak_work,
		soh_pd_leak_calculate_work);
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_pd_leak_dev.pd_leak_work,
		msecs_to_jiffies(PD_WORK_DELAY_MS));

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

static int soh_pd_leak_uninit(struct hisi_soh_device *di)
{
	if (!di)
		return -1;

	/* if dts not support,return success */
	if (!di->soh_pd_leak_dev.pd_leak_support) {
		hisi_soh_err("%s not support soh pd leak!\n", __func__);
		return 0;
	}

	hisi_soh_info("[%s]suc!\n", __func__);
	return 0;
}

int soh_get_acr_resistance(struct acr_info *r, enum acr_type type)
{
	struct hisi_soh_device *di = g_soh_di;
	errno_t err;

	if (!r) {
		hisi_soh_err("%s pointer null!\n", __func__);
		return -1;
	}

	if (!di) {
		hisi_soh_err("%s g_soh_di null!\n", __func__);
		return -1;
	}

	switch (type) {
	case ACR_H_PRECISION:
		/* Prevent array Overflow */
		if (di->soh_acr_dev.acr_nv.order_num < 0 ||
			di->soh_acr_dev.acr_nv.order_num >= SOH_ACR_NV_DATA_NUM)
			di->soh_acr_dev.acr_nv.order_num = -1;

		if (di->soh_acr_dev.acr_nv.order_num == -1) {
			(void)memset_s(r, sizeof(*r), 0, sizeof(*r));
		} else {
			err = memcpy_s(r, sizeof(*r),
				&di->soh_acr_dev.acr_nv.soh_nv_acr_info[
				di->soh_acr_dev.acr_nv.order_num],
				sizeof(di->soh_acr_dev.acr_nv.soh_nv_acr_info[
				di->soh_acr_dev.acr_nv.order_num]));
			if (err != EOK) {
				hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
					__func__, err);
				return -1;
			}
		}
		break;

	case ACR_L_PRECISION:
		err = memcpy_s(r, sizeof(*r),
			&di->soh_acr_dev.soh_acr_info,
			sizeof(di->soh_acr_dev.soh_acr_info));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n", __func__, err);
			return -1;
		}
		break;
	default:
		break;
	}

	hisi_soh_info("[%s] num = %d, acr = %d, cycle = %d, vol = %d,"
		"batt_temp = %d, chip_temp[0] = %d, chip_temp[1] = %d!\n",
		__func__, di->soh_acr_dev.acr_nv.order_num, r->batt_acr,
		r->batt_cycle, r->batt_vol, r->batt_temp,
		r->chip_temp[0], r->chip_temp[1]);
	return 0;
}

int soh_get_dcr_resistance(struct dcr_info *r)
{
	struct hisi_soh_device *di = g_soh_di;
	errno_t err;

	if (!r) {
		hisi_soh_err("%s pointer null!\n", __func__);
		return -1;
	}

	if (!di) {
		hisi_soh_err("%s g_soh_di null!\n", __func__);
		return -1;
	}
	/* Prevent array Overflow */
	if (di->soh_dcr_dev.dcr_nv.order_num < 0 ||
		di->soh_dcr_dev.dcr_nv.order_num >= SOH_DCR_NV_DATA_NUM)
		di->soh_dcr_dev.dcr_nv.order_num = -1;

	if (di->soh_dcr_dev.dcr_nv.order_num == -1) {
		err = memset_s(r, sizeof(*r),
				0, sizeof(*r));
		if (err != EOK) {
			hisi_soh_err("[%s]memset_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
	} else {
		err = memcpy_s(r, sizeof(*r),
			&di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[
			di->soh_dcr_dev.dcr_nv.order_num],
			sizeof(di->soh_dcr_dev.dcr_nv.soh_nv_dcr_info[
			di->soh_dcr_dev.dcr_nv.order_num]));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
	}

	hisi_soh_info("[%s] num = %d, r0 = %d, dcr = %d, cycle = %d, vol = %d, batt_temp = %d!\n",
		__func__, di->soh_dcr_dev.dcr_nv.order_num, r->batt_r0,
		r->batt_dcr, r->batt_cycle, r->batt_vol, r->batt_temp);
	return 0;
}

int soh_get_poweroff_leakage(struct pd_leak_current_info *i)
{
	struct hisi_soh_device *di = g_soh_di;
	errno_t err;

	if (!i) {
		hisi_soh_err("%s pointer null!\n", __func__);
		return -1;
	}

	if (!di) {
		hisi_soh_err("%s g_soh_di null!\n", __func__);
		return -1;
	}
	/* Prevent array Overflow */
	if (di->soh_pd_leak_dev.pd_leak_nv.order_num < 0 ||
		di->soh_pd_leak_dev.pd_leak_nv.order_num >= SOH_PD_NV_DATA_NUM)
		di->soh_pd_leak_dev.pd_leak_nv.order_num = -1;

	if (di->soh_pd_leak_dev.pd_leak_nv.order_num == -1) {
		err = memset_s(i, sizeof(*i),
			0, sizeof(*i));
		if (err != EOK) {
			hisi_soh_err("[%s]memset_s fail, err=%d\n",
				__func__, err);
			return -1;
		}
	} else {
		err = memcpy_s(i, sizeof(*i),
			&di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[
			di->soh_pd_leak_dev.pd_leak_nv.order_num],
			sizeof(di->soh_pd_leak_dev.pd_leak_nv.soh_nv_pd_leak_current_info[
			di->soh_pd_leak_dev.pd_leak_nv.order_num]));
		if (err != EOK) {
			hisi_soh_err("[%s]memcpy_s fail, err=%d\n", __func__, err);
			return -1;
		}
	}

	hisi_soh_info("[%s] num = %d, leak_ma = %d, cycle = %d, sys_cc =%d, curr[0] =%d, curr[1] = %d,vol[0] = %d, vol[1]=%d, chip_temp[0]=%d, chip_temp[1]=%d, rtc[0]=%d, rtc[1]=%d!\n",
		      __func__, di->soh_pd_leak_dev.pd_leak_nv.order_num,
		      i->leak_current_ma, i->batt_cycle, i->sys_pd_leak_cc,
		      i->batt_current[0], i->batt_current[1],
		      i->batt_vol[0], i->batt_vol[1], i->chip_temp[0],
		      i->chip_temp[1], i->rtc_time[0], i->rtc_time[1]);
	return 0;
}


/* start acr calculate regardless of acr check conditions. */
void soh_acr_low_precision_cal_start(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	struct hisi_soh_device *di = g_soh_di;

	if (!di)
		return;

	di->soh_acr_dev.acr_prec_type = ACR_L_PRECISION;
	queue_delayed_work(system_power_efficient_wq,
		&di->soh_acr_dev.acr_work, msecs_to_jiffies(0));
#endif
}

static int soh_probe(struct platform_device *pdev)
{
	struct hisi_soh_device *di = NULL;
	int ret;

	ret = soh_get_nv_info_from_boot();
	if (ret)
		return -1;

	di = (struct hisi_soh_device *)devm_kzalloc(
		&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		ret = -1;
		hisi_soh_err("[%s]soh devm_kzalloc fail!\n", __func__);
		goto soh_kzalloc_fail;
	}

	di->dev = &pdev->dev;

	platform_set_drvdata(pdev, di);

	wakeup_source_init(&di->soh_wake_lock, "soh_wakelock");

	mutex_init(&di->soh_mutex);

	/* acr init, it doesn't affect other soh  modules if it fails. */
	ret = soh_acr_init(di);
	if (ret) {
		hisi_soh_err("[%s]soh acr init fail!\n", __func__);
		goto acr_init_fail;
	}

	/* dcr init */
	ret = soh_dcr_init(di);
	if (ret) {
		hisi_soh_err("[%s]soh dcr init fail!\n", __func__);
		goto dcr_init_fail;
	}

	/* ovp init */
	ret = soh_ovp_init(di);
	if (ret) {
		hisi_soh_err("[%s]soh ovp init fail!\n", __func__);
		goto ovp_init_fail;
	}

	/* pd leak init */
	ret = soh_pd_leak_init(di);
	if (ret) {
		hisi_soh_err("[%s]soh pd leak init fail!\n", __func__);
		goto pd_leak_init_fail;
	}

	ret = soh_create_sysfs_file(di);
	if (ret)
		goto create_sysfs_fail;

	g_soh_di = di;
	hisi_soh_info("[%s]ok!\n", __func__);
	return 0;

create_sysfs_fail:
	soh_pd_leak_uninit(di);
	soh_sysfs_remove_file(di);
pd_leak_init_fail:
	soh_ovp_uninit(di);
ovp_init_fail:
	soh_dcr_uninit(di);
dcr_init_fail:
	soh_acr_uninit(di);
acr_init_fail:
	wakeup_source_trash(&di->soh_wake_lock);
	mutex_destroy(&di->soh_mutex);
	platform_set_drvdata(pdev, NULL);
soh_kzalloc_fail:
	soh_nv_info_uninit();
	return ret;
}

static int soh_remove(struct platform_device *pdev)
{
	struct hisi_soh_device *di = platform_get_drvdata(pdev);

	if (!di) {
		hisi_soh_err("[%s]di is NULL!\n", __func__);
		return -ENODEV;
	}
	soh_sysfs_remove_file(di);
	soh_acr_uninit(di);
	soh_dcr_uninit(di);
	soh_ovp_uninit(di);
	soh_pd_leak_uninit(di);
	wakeup_source_trash(&di->soh_wake_lock);
	mutex_destroy(&di->soh_mutex);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_soh_di = NULL;
	soh_nv_info_uninit();
	return 0;
}

static const struct of_device_id hisi_soh_core_match_table[] = {
	{
		.compatible = "hisi,soh_core",
	},
	{
	},
};

static void soh_shutdown(struct platform_device *pdev)
{
	struct hisi_soh_device *di = platform_get_drvdata(pdev);

	if (!di)
		return;
	hisi_soh_err("[%s]ok!\n", __func__);
}

static struct platform_driver hisi_soh_core_driver = {
	.probe    = soh_probe,
	.remove   = soh_remove,
	.shutdown = soh_shutdown,
	.driver   = {
		.name           = "hisi_soh_core",
		.owner          = THIS_MODULE,
		.of_match_table = hisi_soh_core_match_table,
	},
};

int __init soh_core_init(void)
{
	return platform_driver_register(&hisi_soh_core_driver);
}

void __exit soh_core_exit(void)
{
	platform_driver_unregister(&hisi_soh_core_driver);
}

late_initcall(soh_core_init);
module_exit(soh_core_exit);


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("batt soh module");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
