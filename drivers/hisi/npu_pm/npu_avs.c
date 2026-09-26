/*
 * npu_avs.c
 *
 * npu avs and svfd
 *
 * Copyright (C) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/version.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/devfreq.h>
#include <linux/math64.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/pm_opp.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <npu_pm.h>
#include <linux/jiffies.h>
#include "npu_pm_private.h"
#include "npu_dvfs.h"
#ifdef CONFIG_THERMAL_TSENSOR
#include <linux/lpm_thermal.h>
#endif

#define HISI_AVS_NPU_TSENSOR_NAME  "npu"
#define SLICE_TO_US		32
#define AVS_INTERVAL		60000UL

static int hisi_npu_avs_tsensor;
static unsigned int g_avs_tzone_idx = 1;
static unsigned int g_npu_cur_svfd_threshold;

static unsigned int g_pa_val = 2;
static unsigned long g_last_pa_time;

enum {
	NPU_PM_REG_BASE,
	NPU_PM_REG_VS_CRG,
	NPU_PM_REG_VS_CTRL,
	MAX_NPU_PM_REG,
};

enum {
	BASE_VOLT,
	STEP_VOLT,
	MAX_VOLT_TYPE,
};

static unsigned int g_npu_avsvolt[MAX_VOLT_TYPE];

#ifdef CONFIG_NPU_PM_AVS_NEW_TZONE
static unsigned int get_avs_tzone_idx(int temp)
{
	unsigned int idx = 0;
	unsigned int i;

	for (i = 0; i < MAX_AVS_VOLT; i++) {
		if (temp <= (int)(g_dvfs_data.main_module->avs_temp[i])) {
			idx = i;
			break;
		}
	}
	if (i == MAX_AVS_VOLT)
		idx = MAX_AVS_VOLT - 1;

	return idx;
}
#else
static unsigned int get_avs_tzone_idx(int temp)
{
	if (temp <= (int)(g_dvfs_data.main_module->avs_temp[1]))
		return 0;
	else if (temp > (int)(g_dvfs_data.main_module->avs_temp[1]) &&
		 temp <= (int)(g_dvfs_data.main_module->avs_temp[2]))
		return 1;
	else if (temp > (int)(g_dvfs_data.main_module->avs_temp[2]) &&
		 temp <= (int)(g_dvfs_data.main_module->avs_temp[3]))
		return 2;
	else
		return 3;
}
#endif
unsigned int npu_volt_cfg2vol(unsigned int vol_cfg)
{
	return (g_npu_avsvolt[BASE_VOLT] +
		g_npu_avsvolt[STEP_VOLT] * vol_cfg) * K_RATIO; /* mV */
}

unsigned int update_avs_temperature_info(void)
{
#ifdef CONFIG_THERMAL_TSENSOR
	int temp = 0;
	int ret;

	ret = ipa_get_sensor_value(hisi_npu_avs_tsensor, &temp);
	/* get temp fail use last temp avs config */
	if (ret != 0)
		return g_avs_tzone_idx;
	temp = temp / K_RATIO;
	g_avs_tzone_idx = get_avs_tzone_idx(temp);
#endif

	return g_avs_tzone_idx;
}

static int check_avs_para_update(void)
{
	unsigned long now, tmp;
	int ret = 0;

	now = jiffies64_to_nsecs(get_jiffies_64());
	/* change us to ms, so we divide 1000 */
	now = div_u64(now, NSEC_PER_MSEC);
	tmp = (now - g_last_pa_time);
	if (tmp > AVS_INTERVAL) {
		g_last_pa_time = now;
		ret = 1;
	}

	return ret;
}

noinline int atfd_hisi_service_avs_smc(u64 function_id, u64 _arg0,
				       u64 _arg1, u64 _arg2)
{
	register u64 funcid asm("x0") = function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;

	(void)arg0;
	(void)arg1;
	(void)arg2;
	asm volatile (
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc	#0\n"
		: "+r" (funcid)
		: "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)funcid;
}

static int npu_pasensor_mode_cfg(void)
{
	int ret;

	ret = atfd_hisi_service_avs_smc(HISI_AVS_NPU_PA_MODE_CFG,
					0ULL, 0ULL, 0ULL);

	return ret;
}

static int npu_pasensor_mode_recovery(void)
{
	int ret;

	ret = atfd_hisi_service_avs_smc(HISI_AVS_NPU_PA_MODE_RECOVERY,
					0ULL, 0ULL, 0ULL);

	return ret;
}

static int npu_read_pasensor_code(struct hisi_npu_module_profile *module_profile)
{
	int ret;

	ret = atfd_hisi_service_avs_smc(HISI_AVS_NPU_GET_PA_VALUE,
					module_profile->pa_ctrl.pa_margin[PA_HCI],
					module_profile->pa_ctrl.pa_margin[PA_NBTI],
					0ULL);

	return ret;
}

#define NPU_PA_VOLT_UPLIMIT	30 /* mV */
unsigned int npu_cluster_avs(struct hisi_npu_module_profile *module_profile,
			     unsigned int open_switch)
{
	int ret;
	unsigned int pa_voltage;
	struct module_npu_pa_ctrl_info *pa_ctrl = NULL;

	/*
	 * this abnormal branch, return default code for safe,
	 * but in this branch we return 0
	 */
	if (module_profile == NULL)
		return 0;

	pa_ctrl = &module_profile->pa_ctrl;

	if (open_switch == 0)
		goto default_val;

	/* pasensor code update period is define in dts config */
	ret = check_avs_para_update();
	if (ret == 0)
		goto default_val;

	/* enable target pll */
	ret = clk_prepare_enable(pa_ctrl->pll_clk);
	if (ret != 0) {
		pr_err("[npu avs] %s %d clk enable fail%d\n",
		       __func__, __LINE__, ret);
		goto default_val;
	}

	ret = npu_pasensor_mode_cfg();
	if (ret != 0) {
		/* if read pasensor failed, return last pa data */
		pr_err("[npu avs] %s %d pasensor mode cfg fail%d\n",
		       __func__, __LINE__, ret);
		goto out;
	}
	g_pa_val = npu_read_pasensor_code(module_profile);
	if (g_pa_val > pa_ctrl->pa_max_code)
		g_pa_val = pa_ctrl->pa_max_code;

	ret = npu_pasensor_mode_recovery();
	if (ret != 0) {
		/* if read pasensor failed, return last pa data */
		pr_err("[npu avs] %s %d pasensor mode revocery fail%d\n",
		       __func__, __LINE__, ret);
		goto out;
	}
out:
	/* disable source pll */
	clk_disable_unprepare(pa_ctrl->pll_clk);
default_val:
	pr_debug("[npu avs] %s %d pasensor code is %d\n",
		 __func__, __LINE__, g_pa_val);

	pa_voltage = min_t(unsigned int, g_pa_val * pa_ctrl->pa_step_volt,
			   NPU_PA_VOLT_UPLIMIT);

	/* pa_ctrl.pa_mode_para[1] is pa code step volt */
	return (unsigned int)(pa_voltage * K_RATIO); /* uV */
}

void npu_update_svfd_code(struct hisi_npu_module_profile *module_profile,
			  int tar_pid)
{
	unsigned int temp_idx;
	struct hisi_npu_module_profile *main_module = g_dvfs_data.main_module;

	if (strcmp(module_profile->module_name, main_module->module_name) != 0)
		return;

	temp_idx = update_avs_temperature_info();

	g_npu_cur_svfd_threshold =
		main_module->profile_table[tar_pid].svfdcode[temp_idx];
}

static int hisi_avs_npu_thermal_init(struct device *dev)
{
#ifdef CONFIG_THERMAL_TSENSOR
	hisi_npu_avs_tsensor = ipa_get_tsensor_id(HISI_AVS_NPU_TSENSOR_NAME);
	if (hisi_npu_avs_tsensor < 0) {
		dev_err(dev, "get tsensor fail %d\n", g_dvfs_data.tsensor);
		return -ENOMEM;
	}
#endif

	return 0;
}

static int hisi_npu_avs_memory_cfg_init(struct device *dev,
					struct hisi_npu_module_profile *module_profile)
{
	int i;
	struct device_node *root = dev->of_node;
	void __iomem *base[MAX_NPU_PM_REG] = { NULL };

	for (i = 0; i < MAX_NPU_PM_REG; i++) {
		base[i] = of_iomap(root, i);
		if (base[i] == NULL) {
			dev_err(dev, "[%s] base[%d] iomap fail %ld\n",
				__func__, i, PTR_ERR(base[i]));

			while (i > 0) {
				iounmap(base[i - 1]);
				i--;
			}

			return -ENOMEM;
		}
	}

	module_profile->svfd_ctrl.vs_crg_base = base[NPU_PM_REG_VS_CRG];
	module_profile->svfd_ctrl.vs_ctrl_base = base[NPU_PM_REG_VS_CTRL];

	return 0;
}

static int hisi_npu_pasensor_clk_init(struct device *dev,
				      struct device_node *module_np,
				      struct hisi_npu_module_profile *module_profile)
{
	int ret;
	const char *clk_name = NULL;

	ret = of_property_read_string(module_np, "pa-clock-names", &clk_name);
	if (ret != 0) {
		dev_err(dev, "failed to find pa-clock-names\n");
		return ret;
	}
	module_profile->pa_ctrl.pll_clk = hisi_npu_find_clock(clk_name);

	return ret;
}

static void hisi_npu_avs_info_debug(struct device *dev,
				    struct hisi_npu_module_profile *module_profile)
{
	int i;

	for (i = 0; i < MAX_AVS_VOLT; i++)
		dev_dbg(dev, "npu avs temperature[%d] ->(%x)\n",
			i, module_profile->avs_temp[i]);

	for (i = 0; i < SVFD_REG_OFFSET_NUM; i++)
		dev_dbg(dev, "npu svfd offset %d ->(%x)\n",
			i, module_profile->svfd_ctrl.vdm_reg_offset[i]);

	for (i = 0; i < SVFD_CFG_BIT_NUM; i++)
		dev_dbg(dev, "npu svfd-cfg-bit %d ->(%x)\n",
			i, module_profile->svfd_ctrl.vdm_cfg_bit[i]);

	for (i = 0; i < PA_MARGIN_NUM; i++)
		dev_dbg(dev, "npu pa-code-margin %d ->(%x)\n",
			i, module_profile->pa_ctrl.pa_margin[i]);

	dev_dbg(dev, "avsvolt_compute base %u step %u\n",
		g_npu_avsvolt[BASE_VOLT], g_npu_avsvolt[STEP_VOLT]);
	dev_dbg(dev, "pa-safe-profile %u\n",
		module_profile->pa_ctrl.pa_safe_prof);
	dev_dbg(dev, "pa-step-volt %u\n",
		module_profile->pa_ctrl.pa_step_volt);
	dev_dbg(dev, "pa-max-code %u\n",
		module_profile->pa_ctrl.pa_max_code);
}

static bool avs_dt_func_para_check(struct device *dev,
				   struct device_node *module_np,
				   struct hisi_npu_module_profile *module_profile,
				   const char *main_module_name)
{
	if (dev == NULL ||
	    module_np == NULL ||
	    module_profile == NULL ||
	    main_module_name == NULL)
		return false;

	return true;
}

int hisi_npu_dt_avs_info_parse(struct device *dev,
			       struct device_node *module_np,
			       struct hisi_npu_module_profile *module_profile,
			       const char *main_module_name)
{
	int ret;
	int i, j;
	unsigned int *prof_switch = NULL;
	unsigned int *svfd_code = NULL;
	unsigned int *avs_volt = NULL;
	struct hisi_npu_profile *profile = module_profile->profile_table;

	if (!avs_dt_func_para_check(dev, module_np, module_profile,
				    main_module_name)) {
		pr_err("[npu avs] %s %d pasensor init fail\n",
		       __func__, __LINE__);
		return 0;
	}

	if (strncmp(main_module_name, module_profile->module_name,
		    strlen(main_module_name)) != 0)
		return 0;

	ret = hisi_avs_npu_thermal_init(dev);
	if (ret != 0) {
		dev_err(dev, "%s npu thermal init failed\n", __func__);
		return ret;
	}

	ret = hisi_npu_avs_memory_cfg_init(dev, module_profile);
	if (ret != 0) {
		dev_err(dev, "%s npu avs memcfg init failed\n", __func__);
		return ret;
	}

	ret = hisi_npu_pasensor_clk_init(dev, module_np, module_profile);
	if (ret != 0) {
		dev_err(dev, "%s npu avs pa_clk init failed\n", __func__);
		return ret;
	}

	prof_switch = devm_kzalloc(dev, sizeof(unsigned int) * module_profile->profile_count,
				   GFP_KERNEL);
	if (prof_switch == NULL) {
		dev_err(dev, "Failed to allocate npu prof switch buff\n");
		return -ENOMEM;
	}

	svfd_code = devm_kzalloc(dev, sizeof(unsigned int) *
				 module_profile->profile_count * MAX_AVS_VOLT, GFP_KERNEL);
	if (svfd_code == NULL) {
		dev_err(dev, "Failed to allocate npu svfd code buff\n");
		ret = -ENOMEM;
		goto svfd_alloc_fail;
	}

	avs_volt = devm_kzalloc(dev, sizeof(unsigned int) *
				module_profile->profile_count * MAX_AVS_VOLT, GFP_KERNEL);
	if (avs_volt == NULL) {
		dev_err(dev, "Failed to allocate npu avs volt buff\n");
		ret = -ENOMEM;
		goto avs_alloc_fail;
	}
	/* get avs config */
	ret = of_property_read_u32_array(module_np, "avs-temperature",
					 (u32 *)(module_profile->avs_temp),
					 MAX_AVS_VOLT);

	ret += of_property_read_u32_array(module_np, "svfd-reg-offset",
					  (u32 *)(module_profile->svfd_ctrl.vdm_reg_offset),
					  SVFD_REG_OFFSET_NUM);

	ret += of_property_read_u32_array(module_np, "svfd-cfg-bit",
					  (u32 *)(module_profile->svfd_ctrl.vdm_cfg_bit),
					  SVFD_CFG_BIT_NUM);

	ret += of_property_read_u32_array(module_np, "prof-sw",
					  (u32 *)(prof_switch),
					  module_profile->profile_count);

	ret += of_property_read_u32_array(module_np, "prof-svfdcode", (u32 *)(svfd_code),
					  module_profile->profile_count * MAX_AVS_VOLT);

	ret += of_property_read_u32_array(module_np, "prof-avsvolt", (u32 *)(avs_volt),
					  module_profile->profile_count * MAX_AVS_VOLT);

	ret += of_property_read_u32_array(module_np, "avsvolt_compute",
					  (u32 *)(g_npu_avsvolt),
					  MAX_VOLT_TYPE);

	ret += of_property_read_u32_array(module_np, "pa-safe-profile",
					  &module_profile->pa_ctrl.pa_safe_prof, 1);

	ret += of_property_read_u32_array(module_np, "pa-code-margin",
					  (u32 *)(module_profile->pa_ctrl.pa_margin),
					  PA_MARGIN_NUM);

	ret += of_property_read_u32_array(module_np, "pa-step-volt",
					  &module_profile->pa_ctrl.pa_step_volt, 1);

	ret += of_property_read_u32_array(module_np, "pa-max-code",
					  &module_profile->pa_ctrl.pa_max_code, 1);
	if (ret != 0) {
		pr_err("[npu avs] failed to get avs config from DT\n");
		goto parse_dt_fail;
	}

	hisi_npu_avs_info_debug(dev, module_profile);

	for (i = 0; i < module_profile->profile_count; i++) {
		profile[i].func_sw = prof_switch[i];
		dev_dbg(dev, "prof[%d],func_sw %x\n", i, profile[i].func_sw);

		for (j = 0; j < MAX_AVS_VOLT; j++) {
			profile[i].svfdcode[j] =
				svfd_code[i * MAX_AVS_VOLT + j];
			profile[i].avsvolt[j] = avs_volt[i * MAX_AVS_VOLT + j];
			dev_dbg(dev, "prof[%d],svfd[%d] %x, avs_volt[%d] %x\n",
				i, j, profile[i].svfdcode[j],
				j, profile[i].avsvolt[j]);
		}
	}
parse_dt_fail:
	devm_kfree(dev, avs_volt);
avs_alloc_fail:
	devm_kfree(dev, svfd_code);
svfd_alloc_fail:
	devm_kfree(dev, prof_switch);
	return ret;
}
