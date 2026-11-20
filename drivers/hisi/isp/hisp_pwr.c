/*
 *  driver, hisp_pwr.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/hisi/hisi_load_image.h>
#include <linux/hisi/partition_ap_kernel.h>
#include <teek_client_id.h>
#include <partition_macro.h>
#include <linux/rproc_share.h>
#include <global_ddr_map.h>
#include "hisp_internel.h"
#include <isp_ddr_map.h>
#include <securec.h>
#include "../../clk/hisi_extreme/dvfs/peri_volt_poll.h"


#define ISP_CFG_SIZE    0x1000
#define ISP_LOAD_PARTITION    "isp_firmware"
#define HZ_TO_MHZ_DIV   1000000

#define ISP_SUB_CTRL_CANARY_ADDR       (0x6FC)

#define ISP_REG_LENGTH_ALIGN            (32)

struct hisp_pwr isp_pwr_dev;

static int need_powerup(unsigned int refs)
{
	if (refs == ISP_MAX_NUM_MAGIC)
		pr_err("[%s] exc, refs == 0xffffffff\n", __func__);

	return ((refs == 0) ? 1 : 0);
}

static int need_powerdn(unsigned int refs)
{
	if (refs == ISP_MAX_NUM_MAGIC)
		pr_err("[%s] exc, refs == 0xffffffff\n", __func__);

	return ((refs == 1) ? 1 : 0);
}

static int check_clock_valid(int clk)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	if (clk >= (int)dev->clock_num) {
		pr_err("[%s] Failed : clk %d >= %d\n",
				__func__, clk, dev->clock_num);
		return -EINVAL;
	}

	return 0;
}

int check_dvfs_valid(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	return dev->usedvfs;
}

static int invalid_dvfsmask(int clk)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret;

	ret = check_clock_valid(clk);
	if (ret < 0)
		return ret;

	return (0x01 & (dev->dvfsmask >> (unsigned int)clk));
}

static int hisp_clock_down(struct hisp_pwr *dev,
		int clk, unsigned int clkdown)
{
	int ret = 0;
	unsigned long ispclk = 0;
	unsigned int stat_machine = clkdown;
	unsigned int type;

	if (!check_dvfs_valid()) {
		pr_err("[%s] Failed : Do not Support DVFS\n", __func__);
		return -EINVAL;
	}

	if (clk >= (int)dev->clock_num) {
		pr_err("[%s] Failed : clk.%d >= %d\n",
			__func__, clk, dev->clock_num);
		return -EINVAL;
	}

	do {
		type = stat_machine;
		if (type >= HISP_CLKDOWN_MAX) {
			pr_err("[%s] Failed: type. %d > %d or < 0\n",
					__func__, type, HISP_CLKDOWN_MAX);
			return -EINVAL;
		}

		stat_machine++;
		ispclk = (unsigned long)dev->clkdn[type][clk];
		pr_info("[%s] Clock Down %lu.%lu MHz\n", __func__,
			ispclk / HZ_TO_MHZ_DIV, ispclk % HZ_TO_MHZ_DIV);
		if (ispclk == 0)
			continue;

		ret = clk_set_rate(dev->ispclk[clk], (unsigned long)ispclk);
		if (ret < 0) {
			pr_err("[%s] Failed: clk_set_rate.%d > %d\n",
				__func__, type, stat_machine);
			goto try_clock_down;
		}

		ret = clk_prepare_enable(dev->ispclk[clk]);
		if (ret < 0) {
			pr_err("[%s] Failed: clk_prepare_enable.%d, %d > %d\n",
				__func__, ret, type, stat_machine);
			goto try_clock_down;
		}
		rproc_set_shared_clk_value(clk, (unsigned int)ispclk);
try_clock_down:
		if (ret != 0 && stat_machine < HISP_CLKDOWN_MAX)
			pr_info("[%s]Clk Dwn %lu.%luMHz > %u.%uMHz\n", __func__,
				ispclk / HZ_TO_MHZ_DIV, ispclk % HZ_TO_MHZ_DIV,
				dev->clkdn[stat_machine][clk] / HZ_TO_MHZ_DIV,
				dev->clkdn[stat_machine][clk] % HZ_TO_MHZ_DIV);
	} while (ret != 0);

	return ret;
}

static unsigned long hisp_clock_get(struct hisp_pwr *dev, int clk)
{
	unsigned long ispclock = 0;

	ispclock = (unsigned long)dev->ispclk_value[clk];

	return ispclock;
}

static int hisp_clock_enable(struct hisp_pwr *dev, int clk)
{
	unsigned long ispclock = 0;
	int ret;

	if (check_clock_valid(clk))
		return -EINVAL;

	if (!strncmp(dev->clk_name[clk], "isp_sys", strlen("isp_sys"))) {
		ret = clk_prepare_enable(dev->ispclk[clk]);
		if (ret < 0) {
			pr_err("[%s] Failed: %d.%s.clk_prepare_enable.%d\n",
				__func__, clk, dev->clk_name[clk], ret);
			return -EINVAL;
		}
		pr_info("[%s] %d.%s.clk_prepare_enable\n",
				__func__, clk, dev->clk_name[clk]);
		return 0;
	}

	ispclock = hisp_clock_get(dev, clk);


	ret = clk_set_rate(dev->ispclk[clk], ispclock);
	if (ret < 0) {
		pr_err("[%s] Failed: %d.%d M, %d.%s.clk_set_rate.%d\n",
			__func__, (int)ispclock / HZ_TO_MHZ_DIV,
			(int)ispclock % HZ_TO_MHZ_DIV,
			clk, dev->clk_name[clk], ret);
		goto try_clock_down;
	}

	ret = clk_prepare_enable(dev->ispclk[clk]);
	if (ret < 0) {
		pr_err("[%s] Failed: %d.%d M, %d.%s.clk_prepare_enable.%d\n",
			__func__, (int)ispclock / HZ_TO_MHZ_DIV,
			(int)ispclock % HZ_TO_MHZ_DIV,
			clk, dev->clk_name[clk], ret);
		goto try_clock_down;
	}
	rproc_set_shared_clk_value(clk, (unsigned int)ispclock);
	pr_info("[%s] %d.%s.clk_set_rate.%d.%d M\n", __func__,
		clk, dev->clk_name[clk], (int)ispclock / HZ_TO_MHZ_DIV,
		(int)ispclock % HZ_TO_MHZ_DIV);

	return 0;

try_clock_down:
	return hisp_clock_down(dev, clk, HISP_CLK_SVS);
}

static void hisp_clock_disable(struct hisp_pwr *dev, int clk)
{
	unsigned long ispclock = 0;
	int ret = 0;

	if (check_clock_valid(clk))
		return;

	if (!strncmp(dev->clk_name[clk], "isp_sys", strlen("isp_sys"))) {
		pr_info("[%s] %d.%s.clk_disable_unprepare\n",
				__func__, clk, dev->clk_name[clk]);
		clk_disable_unprepare(dev->ispclk[clk]);
		return;
	}

	ispclock = (unsigned long)dev->clkdis_need_div[clk];
	if (ispclock != 0) {
		ret = clk_set_rate(dev->ispclk[clk], ispclock);
		if (ret < 0) {
			pr_err("[%s] Failed: need_div %d.%d M, %d.%s.rate.%d\n",
				__func__, (int)ispclock / HZ_TO_MHZ_DIV,
				(int)ispclock % HZ_TO_MHZ_DIV,
				clk, dev->clk_name[clk], ret);
			return;
		}
		pr_info("[%s] %d.%s.need_div clk_set_rate.%d.%d M\n", __func__,
			clk, dev->clk_name[clk], (int)ispclock / HZ_TO_MHZ_DIV,
			(int)ispclock % HZ_TO_MHZ_DIV);
	}

	ispclock = (unsigned long)dev->clkdis_dvfs[clk];
	ret = clk_set_rate(dev->ispclk[clk], ispclock);
	if (ret < 0) {
		pr_err("[%s] Failed: %d.%d M, %d.%s.clk_set_rate.%d\n",
			__func__, (int)ispclock / HZ_TO_MHZ_DIV,
			(int)ispclock % HZ_TO_MHZ_DIV, clk,
			dev->clk_name[clk], ret);
		return;
	}

	rproc_set_shared_clk_value(clk, (unsigned int)ispclock);
	pr_info("[%s] %d.%s.clk_set_rate.%d.%d M\n", __func__, clk,
			dev->clk_name[clk], (int)ispclock / HZ_TO_MHZ_DIV,
			(int)ispclock % HZ_TO_MHZ_DIV);

	clk_disable_unprepare(dev->ispclk[clk]);
}

static int isptop_power_up(void)
{
	int ret = 0;

	pr_info("[%s] +\n", __func__);

	ret = atfisp_isptop_power_up();
	if (ret != 0) {
		pr_err("[%s] atfisp_isptop_power_up.%d\n", __func__, ret);
		return ret;
	}
	pr_info("[%s] -\n", __func__);

	return 0;
}

static int isptop_power_down(void)
{
	int ret = 0;

	pr_info("[%s] +\n", __func__);

	ret = atfisp_isptop_power_down();
	if (ret != 0) {
		pr_err("[%s] atfisp_isptop_power_down.%d\n", __func__, ret);
		return ret;
	}
	pr_info("[%s] -\n", __func__);

	return 0;
}

static int hisp_disreset_ispcpu(u64 remap_addr, unsigned int canary)
{
	void __iomem *isp_subctrl_base;
	int ret = 0;

	pr_info("[%s] +\n", __func__);
	ret = get_ispsys_power_state(PWSTAT_MEDIA1);
	if (ret == 0) {
		pr_err("[%s] Failed : get_media1_subsys_power_state. %d\n",
				__func__, ret);
		return -1;
	}

	ret = get_ispsys_power_state(PWSTAT_ISPSYS);
	if (ret == 0) {
		pr_err("[%s] Failed : get_isptop_power_state. %d\n",
				__func__, ret);
		return -1;
	}

	isp_subctrl_base = get_regaddr_by_pa(SUBCTRL);
	if (isp_subctrl_base == NULL) {
		pr_err("[%s] Failed : isp_subctrl_base\n", __func__);
		return -1;
	}

	__raw_writel((unsigned int)(ISP_A7_REMAP_ENABLE | (remap_addr >> 16)),
		isp_subctrl_base + ISP_SUB_CTRL_ISP_A7_CTRL_0);/*lint !e648 */

	__raw_writel(canary, isp_subctrl_base + ISP_SUB_CTRL_CANARY_ADDR);

	ret = atfisp_disreset_ispcpu();
	if (ret < 0) {
		pr_err("[%s] atfisp_disreset_ispcpu. %d\n", __func__, ret);
		return ret;
	}

	pr_info("[%s] -\n", __func__);
	return 0;
}

static int hisp_pwr_setclkrate(unsigned int type, unsigned int rate)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret;

	if (invalid_dvfsmask(type)) {
		pr_err("[%s] Failed : DVFS type.0x%x, rate.%d, dvfsmask.0x%x\n",
				__func__, type, rate, dev->dvfsmask);
		return -EINVAL;
	}

	if ((rate > dev->ispclk_value[type]) || (rate == 0)) {
		pr_err("[%s] Fail: type.0x%x.%s, %d. %d.%08dM > %d. %d.%08dM\n",
			__func__, type, dev->clk_name[type],
			rate, rate / HZ_TO_MHZ_DIV,
			rate % HZ_TO_MHZ_DIV, dev->ispclk_value[type],
			dev->ispclk_value[type] / HZ_TO_MHZ_DIV,
			dev->ispclk_value[type] % HZ_TO_MHZ_DIV);
		return -EINVAL;
	}

	ret = clk_set_rate(dev->ispclk[type], (unsigned long)rate);
	if (ret < 0) {
		pr_err("[%s] Failed : DVFS Set.0x%x.%s Rate.%d. %d.%08d M\n",
				__func__, type, dev->clk_name[type], rate,
				rate / HZ_TO_MHZ_DIV, rate % HZ_TO_MHZ_DIV);
		return ret;
	}
	rproc_set_shared_clk_value((int)type, rate);
	pr_info("[%s] DVFS Set.0x%x.%s Rate.%d. %d.%08d M\n",
		__func__, type, dev->clk_name[type], rate,
		rate / HZ_TO_MHZ_DIV, rate % HZ_TO_MHZ_DIV);

	return 0;
}

static int hisp_subsys_common_powerup(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret, index, err_index;

	if (dev->use_buck13_flag)
		peri_buck13_mem_volt_set(DVFS_ISP_CHANEL, DVFS_BUCK13_VOLT1);

	ret = regulator_enable(dev->clockdep_supply);
	if (ret != 0) {
		pr_err("[%s] Failed: clockdep enable.%d\n", __func__, ret);
		return ret;
	}

	if (dev->use_smmuv3_flag) {
		ret = regulator_enable(dev->isptcu_supply);
		if (ret != 0) {
			pr_err("[%s] Failed:isptcu enable.%d\n", __func__, ret);
			goto err_tcu_poweron;
		}
	}

	for (index = 0; index < (int)dev->clock_num; index++) {
		ret = hisp_clock_enable(dev, index);
		if (ret < 0) {
			pr_err("[%s] Failed: hisp_clock_enable.%d, index.%d\n",
					__func__, ret, index);
			goto err_ispclk;
		}
	}

	ret = regulator_enable(dev->ispcore_supply);
	if (ret != 0) {
		pr_err("[%s] Failed: ispcore enable.%d\n", __func__, ret);
		goto err_ispclk;
	}

	if (dev->ispcpu_supply_flag) {
		ret = regulator_enable(dev->ispcpu_supply);
		if (ret != 0) {
			pr_err("[%s] Failed:ispcpu enable.%d\n", __func__, ret);
			goto err_ispcpu_supply;
		}
	}

	return 0;

err_ispcpu_supply:
	(void)regulator_disable(dev->ispcore_supply);

err_ispclk:
	for (err_index = 0; err_index < index; err_index++)
		hisp_clock_disable(dev, err_index);
	if (dev->use_smmuv3_flag)
		(void)regulator_disable(dev->isptcu_supply);

err_tcu_poweron:
	(void)regulator_disable(dev->clockdep_supply);

	return ret;
}

static int hisp_subsys_common_powerdn(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret, index;

	if (dev->ispcpu_supply_flag) {
		ret = regulator_disable(dev->ispcpu_supply);
		if (ret != 0)
			pr_err("[%s] Failed: ispcpu regulator_disable.%d\n",
					__func__, ret);
	}

	ret = regulator_disable(dev->ispcore_supply);
	if (ret != 0)
		pr_err("[%s] Failed: ispcore regulator_disable.%d\n",
				__func__, ret);

	for (index = 0; index < (int)dev->clock_num; index++)
		hisp_clock_disable(dev, index);

	if (dev->use_smmuv3_flag) {
		ret = regulator_disable(dev->isptcu_supply);
		if (ret != 0)
			pr_err("[%s] Failed: isp tcu regulator_disable.%d\n",
					__func__, ret);
	}
	ret = regulator_disable(dev->clockdep_supply);
	if (ret != 0)
		pr_err("[%s] Failed: isp clockdep regulator_disable.%d\n",
				__func__, ret);

	if (dev->use_buck13_flag)
		peri_buck13_mem_volt_set(DVFS_ISP_CHANEL, DVFS_BUCK13_VOLT0);

	return 0;
}


int get_ispsys_power_state(unsigned int type)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	void __iomem *cfg_base = NULL;
	unsigned int media_pw_stat = 0;
	unsigned int pwsta_num = 0;

	pwsta_num = dev->pwstat_num > PWSTAT_MAX ? PWSTAT_MAX : dev->pwstat_num;

	if (type >= pwsta_num) {
		pr_err("[%s] Failed: type.%d, pwstat_num.%d\n",
			__func__, type, pwsta_num);
		return 0;
	}

	if (dev->pwstat_offset[type] >= ISP_CFG_SIZE) {
		pr_err("[%s] Failed: pwstat_offset.0x%x, type.%d\n",
			__func__, dev->pwstat_offset[type], type);
		return 0;
	}

	cfg_base = get_regaddr_by_pa(dev->pwstat_type[type]);
	if (cfg_base == NULL) {
		pr_err("[%s] Failed: type.%d\n", __func__, type);
		return 0;
	}
	media_pw_stat = __raw_readl(cfg_base + dev->pwstat_offset[type]);
	media_pw_stat >>= dev->pwstat_bit[type];
	media_pw_stat &= 0x01;

	if (media_pw_stat == dev->pwstat_wanted[type])
		return 1;
	else
		return 0;
}

unsigned int hisp_get_smmuc3_flag(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	return dev->use_smmuv3_flag;
}

int hisp_subsys_powerup(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret, err;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_power.0x%x\n", __func__, dev->refs_isp_power);
	if (!need_powerup(dev->refs_isp_power)) {
		dev->refs_isp_power++;
		pr_info("[%s]: refs_isp.0x%x\n", __func__, dev->refs_isp_power);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	ret = hisp_subsys_common_powerup();
	if (ret < 0) {
		pr_err("[%s] Failed: hisp_subsys_common_powerup.%d\n",
			__func__, ret);
		goto err_common_powerup;
	}

	if (hisp_mntn_dumpregs() < 0)
		pr_err("Failed : get_ispcpu_cfg_info");

	if (use_nonsec_isp() || (!sec_process_use_ca_ta())) {
		ret = isptop_power_up();
		if (ret < 0) {
			pr_err("[%s]Failed: isptop_pu.%d\n", __func__, ret);
			goto err_isptop;
		}
	} else {
		ret = hisp_wait_rpmsg_completion();
		if (ret != 0)
			goto err_isptop;

		ret = teek_secisp_disreset();
		if (ret < 0) {
			pr_err("[%s] Failed : teek_secisp_disreset.%d.\n",
				__func__, ret);
			goto err_isptop;
		}
		hisp_pwr_sec_dst_vote();
	}

	dev->refs_isp_power++;
	pr_info("[%s] - refs_isp_power.0x%x\n", __func__, dev->refs_isp_power);
	mutex_unlock(&dev->pwrlock);

	return 0;

err_isptop:
	err = hisp_subsys_common_powerdn();
	if (err != 0)
		pr_err("[%s] Failed: hisp_subsys_common_powerdn.%d\n",
				__func__, err);

err_common_powerup:
	mutex_unlock(&dev->pwrlock);
	return ret;
}

int hisp_subsys_powerdn(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_power.0x%x\n", __func__, dev->refs_isp_power);
	if (!need_powerdn(dev->refs_isp_power)) {
		if (dev->refs_isp_power > 0)
			dev->refs_isp_power--;
		pr_info("[%s] + refs_isp_power.0x%x\n",
			__func__, dev->refs_isp_power);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}
	if (use_nonsec_isp() || (!sec_process_use_ca_ta())) {
		ret = isptop_power_down();
		if (ret != 0)
			pr_err("[%s] Failed: isptop_power_down.%d\n",
					__func__, ret);
	} else {
		ret = teek_secisp_reset();
		if (ret < 0)
			pr_err("[%s] Failed : teek_secisp_reset.%d.\n",
					__func__, ret);
		hisp_pwr_sec_rst_vote();
	}

	ret = hisp_subsys_common_powerdn();
	if (ret != 0)
		pr_err("[%s] Failed: hisp_subsys_common_powerdn.%d\n",
				__func__, ret);

	dev->refs_isp_power--;
	pr_info("[%s] + refs_isp_power.0x%x\n", __func__, dev->refs_isp_power);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_core_nsec_init(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret = 0;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_nsec_init.0x%x\n",
		__func__, dev->refs_isp_nsec_init);
	if (!need_powerup(dev->refs_isp_nsec_init)) {
		dev->refs_isp_nsec_init++;
		pr_info("[%s] + refs_isp_nsec_init.0x%x\n",
				__func__, dev->refs_isp_nsec_init);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	ret = ispmmu_init();
	if (ret < 0) {
		pr_err("[%s] Failed: ispmmu_init.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}
	dev->refs_isp_nsec_init++;
	pr_info("[%s] - refs_isp_nsec_init.0x%x\n",
		__func__, dev->refs_isp_nsec_init);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_core_nsec_exit(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_nsec_init.0x%x\n",
		__func__, dev->refs_isp_nsec_init);
	if (!need_powerdn(dev->refs_isp_nsec_init)) {
		if (dev->refs_isp_nsec_init > 0)
			dev->refs_isp_nsec_init--;
		pr_info("[%s] + refs_isp_nsec_init.0x%x\n",
			__func__, dev->refs_isp_nsec_init);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	ret = ispmmu_exit();
	if (ret != 0)
		pr_err("[%s] Failed : ispmmu_exit.%d\n", __func__, ret);

	dev->refs_isp_nsec_init--;
	pr_info("[%s] - refs_isp_nsec_init.0x%x\n",
		__func__, dev->refs_isp_nsec_init);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_core_sec_init(u64 phy_pgd_base)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_sec_init.0x%x\n",
		__func__, dev->refs_isp_sec_init);
	if (!need_powerup(dev->refs_isp_sec_init)) {
		dev->refs_isp_sec_init++;
		pr_info("[%s] + refs_isp_sec_init.0x%x\n",
				__func__, dev->refs_isp_sec_init);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	atfisp_isp_init(phy_pgd_base);
	dev->refs_isp_sec_init++;
	pr_info("[%s] + refs_isp_sec_init.0x%x\n",
		__func__, dev->refs_isp_sec_init);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_core_sec_exit(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_sec_init.0x%x\n",
		__func__, dev->refs_isp_sec_init);
	if (!need_powerdn(dev->refs_isp_sec_init)) {
		if (dev->refs_isp_sec_init > 0)
			dev->refs_isp_sec_init--;
		pr_info("[%s] + refs_isp_sec_init.0x%x\n",
			__func__, dev->refs_isp_sec_init);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	atfisp_isp_exit();
	dev->refs_isp_sec_init--;
	pr_info("[%s] + refs_isp_sec_init.0x%x\n",
		__func__, dev->refs_isp_sec_init);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_cpu_sec_init(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_ispcpu_init.0x%x\n",
		__func__, dev->refs_ispcpu_init);
	if (!need_powerup(dev->refs_ispcpu_init)) {
		dev->refs_ispcpu_init++;
		pr_info("[%s] + refs_ispcpu_init.0x%x\n",
				__func__, dev->refs_ispcpu_init);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	ret = atfisp_ispcpu_init();
	if (ret < 0) {
		pr_info("[%s] atfisp_ispcpu_init fail.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}

	dev->refs_ispcpu_init++;
	pr_info("[%s] + refs_ispcpu_init.0x%x\n",
		__func__, dev->refs_ispcpu_init);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_cpu_sec_exit(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_ispcpu_init.0x%x\n",
		__func__, dev->refs_ispcpu_init);
	if (!need_powerdn(dev->refs_ispcpu_init)) {
		if (dev->refs_ispcpu_init > 0)
			dev->refs_ispcpu_init--;
		pr_info("[%s] + refs_ispcpu_init.0x%x\n",
				__func__, dev->refs_ispcpu_init);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	atfisp_ispcpu_exit();
	dev->refs_ispcpu_init--;
	pr_info("[%s] + refs_ispcpu_init.0x%x\n",
		__func__, dev->refs_ispcpu_init);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_cpu_nsec_dst(u64 remap_addr, unsigned int canary)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_nsec_dst.0x%x\n",
		__func__, dev->refs_isp_nsec_dst);
	if (!need_powerup(dev->refs_isp_nsec_dst)) {
		dev->refs_isp_nsec_dst++;
		pr_info("[%s] + refs_isp_nsec_dst.0x%x\n",
		__func__, dev->refs_isp_nsec_dst);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	/* need config by secure core */
	ret = hisp_disreset_ispcpu(remap_addr, canary);
	if (ret < 0) {
		pr_err("[%s] Failed : disreset_ispcpu.%d\n", __func__, ret);
		mutex_unlock(&dev->pwrlock);
		return ret;
	}
	dev->refs_isp_nsec_dst++;
	pr_info("[%s] + refs_isp_nsec_dst.0x%x\n",
		__func__, dev->refs_isp_nsec_dst);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_cpu_nsec_rst(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_nsec_dst.0x%x\n",
		__func__, dev->refs_isp_nsec_dst);
	if (!need_powerdn(dev->refs_isp_nsec_dst)) {
		if (dev->refs_isp_nsec_dst > 0)
			dev->refs_isp_nsec_dst--;
		pr_info("[%s] + refs_isp_nsec_dst.0x%x\n",
			__func__, dev->refs_isp_nsec_dst);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}
	dev->refs_isp_nsec_dst--;
	pr_info("[%s] + refs_isp_nsec_dst.0x%x\n",
		__func__, dev->refs_isp_nsec_dst);
	mutex_unlock(&dev->pwrlock);

	return 0;
}

int hisp_pwr_cpu_sec_dst(int ispmem_reserved, unsigned long image_addr)
{
#ifdef CONFIG_LOAD_IMAGE
	struct hisp_pwr *dev = &isp_pwr_dev;
	struct load_image_info loadinfo;
	int ret;

	loadinfo.ecoretype      = ISP;
	loadinfo.image_addr     = image_addr;
	loadinfo.image_size     = MEM_ISPFW_SIZE;
	loadinfo.partion_name   = ISP_LOAD_PARTITION;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + %s.(0x%x), init.%x, ispmem_reserved.%x\n",
		__func__, loadinfo.partion_name, loadinfo.image_size,
		dev->refs_isp_sec_dst, ispmem_reserved);

	if (!need_powerup(dev->refs_isp_sec_dst)) {
		dev->refs_isp_sec_dst++;
		pr_info("[%s] + refs_isp_sec_dst.0x%x\n",
		__func__, dev->refs_isp_sec_dst);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	if (!ispmem_reserved) {
		ret = bsp_load_and_verify_image(&loadinfo);
		if (ret < 0) {
			pr_err("[%s]Failed : bsp_load_and_verify.%d, %s.0x%x\n",
				__func__, ret, loadinfo.partion_name,
				loadinfo.image_size);
			mutex_unlock(&dev->pwrlock);
			return ret;
		}
	} else {
		ret = bsp_load_sec_img(&loadinfo);
		if (ret < 0) {
			pr_err("[%s] Failed : bsp_load_sec_image.%d, %s.0x%x\n",
				__func__, ret, loadinfo.partion_name,
				loadinfo.image_size);
			mutex_unlock(&dev->pwrlock);
			return ret;
		}
	}

	dev->refs_isp_sec_dst++;
	pr_info("[%s]: bsp_load_image.%d, %s.0x%x, init.%x, ispmem_resved.%x\n",
		__func__, ret, loadinfo.partion_name, loadinfo.image_size,
		dev->refs_isp_sec_dst, ispmem_reserved);
	mutex_unlock(&dev->pwrlock);
#endif
	return 0;
}

int hisp_pwr_cpu_sec_rst(void)
{
#ifdef CONFIG_LOAD_IMAGE
	struct hisp_pwr *dev = &isp_pwr_dev;

	mutex_lock(&dev->pwrlock);
	pr_info("[%s] + refs_isp_sec_dst.0x%x\n",
		__func__, dev->refs_isp_sec_dst);
	if (!need_powerdn(dev->refs_isp_sec_dst)) {
		if (dev->refs_isp_sec_dst > 0)
			dev->refs_isp_sec_dst--;
		pr_info("[%s] + refs_isp_sec_dst.0x%x\n",
			__func__, dev->refs_isp_sec_dst);
		mutex_unlock(&dev->pwrlock);
		return 0;
	}

	dev->refs_isp_sec_dst--;
	pr_info("[%s] + refs_isp_sec_dst.0x%x\n",
		__func__, dev->refs_isp_sec_dst);
	mutex_unlock(&dev->pwrlock);
#endif
	return 0;
}

void hisp_pwr_sec_dst_vote(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	dev->refs_isp_sec_dst++;
	pr_info("[%s] + refs_isp_sec_dst.0x%x\n",
		__func__, dev->refs_isp_sec_dst);
}

void hisp_pwr_sec_rst_vote(void)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	dev->refs_isp_sec_dst--;
	pr_info("[%s] + refs_isp_sec_dst.0x%x\n",
		__func__, dev->refs_isp_sec_dst);
}

unsigned long hisp_pwr_getclkrate(unsigned int type)
{
	struct hisp_pwr *dev = &isp_pwr_dev;

	if (invalid_dvfsmask(type)) {
		pr_err("[%s] Failed : DVFS Invalid type.0x%x, dvfsmask.0x%x\n",
				__func__, type, dev->dvfsmask);
		return 0;
	}

	return (unsigned long)clk_get_rate(dev->ispclk[type]);
}

int hisp_pwr_nsec_setclkrate(unsigned int type, unsigned int rate)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret = 0;

	mutex_lock(&dev->pwrlock);
	if (dev->refs_isp_nsec_dst == 0) {
		pr_info("[%s]Failed: isp_nsec_dst.%d, check ISPCPU PowerDown\n",
				__func__, dev->refs_isp_nsec_dst);
		mutex_unlock(&dev->pwrlock);
		return -ENODEV;
	}

	ret = hisp_pwr_setclkrate(type, rate);
	if (ret < 0)
		pr_info("[%s]Failed : hisp_pwr_setclkrate.%d\n", __func__, ret);
	mutex_unlock(&dev->pwrlock);

	return ret;
}

int hisp_pwr_sec_setclkrate(unsigned int type, unsigned int rate)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret = 0;

	mutex_lock(&dev->pwrlock);
	if (dev->refs_isp_sec_dst == 0) {
		pr_info("[%s]Failed: isp_sec_dst.%d, check ISPCPU PowerDown\n",
				__func__, dev->refs_isp_sec_dst);
		mutex_unlock(&dev->pwrlock);
		return -ENODEV;
	}

	ret = hisp_pwr_setclkrate(type, rate);
	if (ret < 0)
		pr_info("[%s]Failed : hisp_pwr_setclkrate.%d\n", __func__, ret);
	mutex_unlock(&dev->pwrlock);

	return ret;
}

void hisp_rproc_ispclk_info_init(void)
{
	return;
}

int hisp_pwr_probe(struct platform_device *pdev)
{
	struct hisp_pwr *dev = &isp_pwr_dev;
	int ret = 0;

	pr_alert("[%s] +\n", __func__);
	dev->device = &pdev->dev;
	dev->ispcpu_supply_flag = 0;
	dev->refs_isp_power = 0;
	dev->refs_isp_nsec_init = 0;
	dev->refs_isp_sec_init = 0;
	dev->refs_ispcpu_init = 0;
	dev->refs_isp_nsec_dst = 0;
	dev->refs_isp_sec_dst = 0;

	ret = hisp_pwr_getdts(pdev, dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_nsec_getdts.%d.\n",
				__func__, ret);
		return ret;
	}

	dev->dvfsmask = (1 << ISPI2C_CLK);
	dev->dvfsmask |= (1 << ISPI3C_CLK);
	mutex_init(&dev->pwrlock);

	hisp_rproc_ispclk_info_init();

	pr_alert("[%s] -\n", __func__);

	return 0;
}

int hisp_pwr_remove(struct platform_device *pdev)
{
	struct hisp_pwr *dev = &isp_pwr_dev;


	mutex_destroy(&dev->pwrlock);
	return 0;
}


