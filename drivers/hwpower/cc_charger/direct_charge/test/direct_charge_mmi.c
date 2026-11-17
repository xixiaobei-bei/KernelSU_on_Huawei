/*
 * direct_charge_mmi.c
 *
 * mmi test for direct charge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <chipset_common/hwpower/hardware_monitor/btb_check.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_test.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_mmi
HWLOG_REGIST();

static struct dc_mmi_data *g_dc_mmi_di;

void dc_mmi_set_test_flag(bool flag)
{
	struct dc_mmi_data *di = g_dc_mmi_di;

	if (!di)
		return;

	di->dc_mmi_test_flag = flag;
	hwlog_info("set dc_mmi_test_flag=%d\n", di->dc_mmi_test_flag);
}

bool dc_mmi_get_test_flag(void)
{
	struct dc_mmi_data *di = g_dc_mmi_di;

	if (!di)
		return false;

	return di->dc_mmi_test_flag;
}

static void dc_mmi_timeout_work(struct work_struct *work)
{
	hwlog_info("dc mmi test timeout, restore flag\n");
	dc_mmi_set_test_flag(false);
}

static void dc_mmi_set_test_status(int value)
{
	struct dc_mmi_data *di = g_dc_mmi_di;
	unsigned long timeout;

	if (!di)
		return;

	if (value) {
		dc_mmi_set_test_flag(true);
		timeout = di->dts_para.timeout + DC_MMI_TO_BUFF;
		if (delayed_work_pending(&di->timeout_work))
			cancel_delayed_work_sync(&di->timeout_work);
		schedule_delayed_work(&di->timeout_work,
			msecs_to_jiffies(timeout * MSEC_PER_SEC));
	} else {
		dc_mmi_set_test_flag(false);
		cancel_delayed_work(&di->timeout_work);
	}
}

void dc_mmi_set_succ_flag(int mode, int value)
{
	struct dc_mmi_data *di = g_dc_mmi_di;

	if (!di)
		return;

	switch (mode) {
	case LVC_MODE:
		di->lvc_succ_flag = value;
		return;
	case SC_MODE:
		di->sc_succ_flag = value;
		return;
	default:
		return;
	}
}

static int dc_mmi_parse_dts(struct device_node *np, struct dc_mmi_data *di)
{
	u32 tmp_para[DC_MMI_PARA_MAX] = { 0 };

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"dc_mmi_para", tmp_para, DC_MMI_PARA_MAX)) {
		tmp_para[DC_MMI_PARA_TIMEOUT] = DC_MMI_DFLT_TIMEOUT;
		tmp_para[DC_MMI_PARA_EXPT_PORT] = DC_MMI_DFLT_EX_PROT;
	}
	di->dts_para.timeout = (int)tmp_para[DC_MMI_PARA_TIMEOUT];
	di->dts_para.expt_prot = tmp_para[DC_MMI_PARA_EXPT_PORT];
	di->dts_para.multi_sc_test = (int)tmp_para[DC_MMI_PARA_MULTI_SC_TEST];
	di->dts_para.ibat_th = (int)tmp_para[DC_MMI_PARA_IBAT_TH];
	di->dts_para.ibat_timeout = (int)tmp_para[DC_MMI_PARA_IBAT_TIMEOUT];
	hwlog_info("[dc_mmi_para] timeout:%ds, expt_prot:%d, multi_sc_test=%d, ibat_th=%d, ibat_timeout=%d\n",
		di->dts_para.timeout, di->dts_para.expt_prot,
		di->dts_para.multi_sc_test, di->dts_para.ibat_th,
		di->dts_para.ibat_timeout);

	return 0;
}

static int dc_mmi_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct dc_mmi_data *di = g_dc_mmi_di;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_DC_CHECK_START:
		di->prot_state = 0;
		di->ibat_first_match = false;
		hwlog_info("direct charge check start\n");
		break;
	case POWER_NE_DC_CHECK_SUCC:
		di->prot_state |= DC_MMI_PROT_DC_SUCC;
		hwlog_info("direct charge enter succ\n");
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int dc_mmi_multi_sc_test(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (l_di->cur_mode != CHARGE_MULTI_IC)
		return -1;

	return 0;
}

static int dc_mmi_ibat_test(struct dc_mmi_data *di)
{
	static unsigned long timeout;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	hwlog_info("mmi test get ibat=%d\n", l_di->ibat);
	if (!di->ibat_first_match && (l_di->ibat >= di->dts_para.ibat_th)) {
		di->ibat_first_match = true;
		timeout = jiffies + msecs_to_jiffies(
			di->dts_para.ibat_timeout * MSEC_PER_SEC);
	}
	if (l_di->ibat < di->dts_para.ibat_th) {
		di->ibat_first_match = false;
		return -1;
	}
	if (!time_after(jiffies, timeout))
		return -1;

	return 0;
}

static int dc_mmi_sc_result(struct dc_mmi_data *di)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || (l_di->working_mode != SC_MODE))
		return di->sc_succ_flag;

	if (btb_ck_get_result(BTB_VOLT_CHECK) || btb_ck_get_result(BTB_TEMP_CHECK))
		return DC_ERROR_BTB_CHECK;

	hwlog_info("prot = 0x%x\n", di->prot_state);
	if (direct_charge_get_stage_status() < DC_STAGE_CHARGING)
		return di->sc_succ_flag;
	dc_mmi_set_succ_flag(SC_MODE, DC_ERROR_PROT_CHECK);
	if ((di->prot_state & di->dts_para.expt_prot) != di->dts_para.expt_prot)
		return di->sc_succ_flag;
	if (di->dts_para.multi_sc_test) {
		dc_mmi_set_succ_flag(SC_MODE, DC_ERROR_OPEN_MULTI_SC);
		if (dc_mmi_multi_sc_test())
			return di->sc_succ_flag;
	}
	if (di->dts_para.ibat_th) {
		dc_mmi_set_succ_flag(SC_MODE, DC_ERROR_CUR_TEST);
		if (dc_mmi_ibat_test(di))
			return di->sc_succ_flag;
	}

	dc_mmi_set_succ_flag(SC_MODE, DC_SUCC);
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t dc_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t dc_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info dc_mmi_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_TIMEOUT, timeout),
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_LVC_RESULT, lvc_result),
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_SC_RESULT, sc_result),
	power_sysfs_attr_wo(dc_mmi, 0200, DC_MMI_SYSFS_TEST_STATUS, test_status),
};

#define DC_MMI_SYSFS_ATTRS_SIZE  ARRAY_SIZE(dc_mmi_sysfs_field_tbl)

static struct attribute *dc_mmi_sysfs_attrs[DC_MMI_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group dc_mmi_sysfs_attr_group = {
	.name = "dc_mmi",
	.attrs = dc_mmi_sysfs_attrs,
};

static ssize_t dc_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct dc_mmi_data *di = g_dc_mmi_di;
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name, dc_mmi_sysfs_field_tbl,
		DC_MMI_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_MMI_SYSFS_LVC_RESULT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", di->lvc_succ_flag);
	case DC_MMI_SYSFS_SC_RESULT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", dc_mmi_sc_result(di));
	case DC_MMI_SYSFS_TIMEOUT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", di->dts_para.timeout);
	default:
		break;
	}

	return 0;
}

static ssize_t dc_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name, dc_mmi_sysfs_field_tbl,
		DC_MMI_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case DC_MMI_SYSFS_TEST_STATUS:
		/* val=1 dc_mmi test start, val=0 dc_mmi test end */
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0 || (val < 0) ||
			(val > 1))
			return -EINVAL;
		dc_mmi_set_test_status((int)val);
		break;
	default:
		break;
	}

	return count;
}

static int dc_mmi_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(dc_mmi_sysfs_attrs, dc_mmi_sysfs_field_tbl,
		DC_MMI_SYSFS_ATTRS_SIZE);
	return sysfs_create_group(&dev->kobj, &dc_mmi_sysfs_attr_group);
}

static void dc_mmi_sysfs_remove_group(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &dc_mmi_sysfs_attr_group);
}
#else
static int dc_mmi_sysfs_create_group(struct device *dev)
{
	return 0;
}

static void dc_mmi_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void dc_mmi_init(struct device *dev)
{
	int ret;
	struct dc_mmi_data *di = NULL;

	if (!dev || !dev->of_node) {
		hwlog_err("init: para error\n");
		return;
	}

	di = devm_kzalloc(dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	g_dc_mmi_di = di;
	ret = dc_mmi_parse_dts(dev->of_node, di);
	if (ret)
		goto free_mem;

	di->lvc_succ_flag = DC_ERROR_ADAPTER_DETECT;
	di->sc_succ_flag = DC_ERROR_ADAPTER_DETECT;
	di->dc_mmi_test_flag = false;

	INIT_DELAYED_WORK(&di->timeout_work, dc_mmi_timeout_work);
	di->dc_mmi_nb.notifier_call = dc_mmi_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_DC, &di->dc_mmi_nb);
	if (ret)
		goto free_mem;

	ret = dc_mmi_sysfs_create_group(dev);
	if (ret)
		goto unregister_notifier;

	hwlog_info("init succ\n");
	return;

unregister_notifier:
	power_event_bnc_unregister(POWER_BNT_DC, &di->dc_mmi_nb);
free_mem:
	devm_kfree(dev, di);
	g_dc_mmi_di = NULL;
}

static void dc_mmi_exit(struct device *dev)
{
	if (!g_dc_mmi_di || !dev)
		return;

	dc_mmi_sysfs_remove_group(dev);
	cancel_delayed_work(&g_dc_mmi_di->timeout_work);
	power_event_bnc_unregister(POWER_BNT_DC, &g_dc_mmi_di->dc_mmi_nb);
	devm_kfree(dev, g_dc_mmi_di);
	g_dc_mmi_di = NULL;
}

static struct power_test_ops g_dc_mmi_ops = {
	.name = "dc_mmi",
	.init = dc_mmi_init,
	.exit = dc_mmi_exit,
};

static int __init dc_mmi_module_init(void)
{
	return power_test_ops_register(&g_dc_mmi_ops);
}

module_init(dc_mmi_module_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dc_mmi module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
