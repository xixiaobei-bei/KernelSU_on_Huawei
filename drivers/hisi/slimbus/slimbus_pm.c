/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include "slimbus_pm.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/pinctrl/consumer.h>
#include "linux/hisi/audio_log.h"
#include <rdr_audio_adapter.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include "slimbus.h"
#include "slimbus_types.h"
#include "slimbus_utils.h"
#include "slimbus_drv.h"
#include "slimbus_da_combine.h"

#define LOG_TAG "slimbus_pm"

#ifdef CONFIG_PM_SLEEP
static int32_t suspend_resource(struct device *device, struct slimbus_private_data *pd)
{
	int32_t ret = 0;
	uint32_t asppower;
	uint32_t aspclk;

	pd->portstate = slimbus_utils_port_state_get(pd->base_addr);

	AUDIO_LOGI("portstate: 0x%x usage_count: 0x%x status: %x, pmuclk: %d aspclk: %d, --",
		pd->portstate, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	if (pd->portstate)
		goto exit;

	/* make sure last msg has been processed finished */
	mdelay(1);
	slimbus_int_need_clear_set(true);
	/*
	 * while fm, da_combine pll is in high freq, slimbus framer is in codec side
	 * we need to switch to soc in this case, and switch to da_combine in resume
	 */
	if (pd->framerstate == SLIMBUS_FRAMER_CODEC) {
		AUDIO_LOGE("switch framer to soc");
		ret = slimbus_switch_framer(pd->device_type, SLIMBUS_FRAMER_SOC);
		if (ret) {
			AUDIO_LOGE("slimbus switch framer failed");
			goto exit;
		}
		pd->lastframer = SLIMBUS_FRAMER_CODEC;
	} else {
		pd->lastframer = SLIMBUS_FRAMER_SOC;
	}

	ret = slimbus_pause_clock(pd->device_type, SLIMBUS_RT_UNSPECIFIED_DELAY);
	if (ret)
		AUDIO_LOGE("slimbus pause clock failed, ret: %x", ret);

	/* make sure para has updated */
	mdelay(1);

	ret = slimbus_drv_stop();
	if (ret)
		AUDIO_LOGE("slimbus stop failed");

	/* set pin to  low power mode */
	ret = pinctrl_select_state(pd->pctrl, pd->pin_idle);
	if (ret) {
		AUDIO_LOGE("could not set pins to idle state");
		goto exit;
	}

	clk_disable_unprepare(pd->pmu_audio_clk);

	ret = regulator_disable(pd->regu_asp);
	if (ret) {
		AUDIO_LOGE("regulator disable failed");
		goto exit;
	}

	clk_disable_unprepare(pd->asp_subsys_clk);

exit:
	asppower = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_power_state_offset);
	aspclk = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_clk_state_offset);
	AUDIO_LOGI("portstate: 0x%x asppower: 0x%x aspclkstate: 0x%x usage_count: 0x%x status: %x, pmuclk: %d aspclk: %d, --",
		pd->portstate, asppower, aspclk, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	return ret;
}

static int32_t slimbus_suspend(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct slimbus_private_data *pd = platform_get_drvdata(pdev);
	int32_t ret;
	int32_t pm_ret;

	if (!pd) {
		AUDIO_LOGE("pd is null");
		return -EINVAL;
	}

	if (pd->pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(device);
		if (pm_ret < 0) {
			AUDIO_LOGE("pm resume error, pm_ret: %d", pm_ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return pm_ret;
		}
	}

	ret = suspend_resource(device, pd);

	return ret;
}

static void resume_exit(struct device *device,
	struct slimbus_private_data *pd, uint32_t asppower, uint32_t aspclk)
{
	if (pd->pm_runtime_support) {
		pm_runtime_mark_last_busy(device);
		pm_runtime_put_autosuspend(device);

		pm_runtime_disable(device);
		pm_runtime_set_active(device);
		pm_runtime_enable(device);
	}

	asppower = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_power_state_offset);
	aspclk = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_clk_state_offset);
	AUDIO_LOGI("portstate: 0x%x asppower: 0x%x aspclkstate: 0x%x usage_count: 0x%x status: %x, pmuclk: %d aspclk: %d, --",
		pd->portstate, asppower, aspclk, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));
}

static int32_t resume_resource(struct device *device,
	struct slimbus_private_data *pd, uint32_t asppower, uint32_t aspclk)
{
	struct slimbus_device_info *dev_info = NULL;
	struct slimbus_bus_config *bus_cfg = NULL;
	int32_t ret = 0;

	if (pd->portstate)
		goto exit;

	ret = clk_prepare_enable(pd->asp_subsys_clk);
	if (ret) {
		AUDIO_LOGE("asp_subsys_clk :clk prepare enable failed");
		goto exit;
	}

	ret = regulator_enable(pd->regu_asp);
	if (ret) {
		AUDIO_LOGE("couldn't enable regulators %d", ret);
		goto exit;
	}

	ret = clk_prepare_enable(pd->pmu_audio_clk);
	if (ret) {
		AUDIO_LOGE("pmu_audio_clk :clk prepare enable failed");
		goto exit;
	}
	/* make sure pmu clk has stable */
	mdelay(1);

	ret = pinctrl_select_state(pd->pctrl, pd->pin_default);
	if (ret) {
		AUDIO_LOGE("could not set pins to default state");
		goto exit;
	}

	slimbus_int_need_clear_set(false);

	dev_info = slimbus_get_devices_info(pd->device_type);

	if (dev_info == NULL) {
		AUDIO_LOGE("device is NULL");
		goto exit;
	}
	slimbus_utils_module_enable(dev_info, true);
	ret = slimbus_drv_resume_clock();
	if (ret)
		AUDIO_LOGE("slimbus resume clock failed, ret: %d", ret);

	ret = slimbus_dev_init(pd->type);
	if (ret) {
		AUDIO_LOGE("slimbus drv init failed");
		goto exit;
	}

	bus_cfg = slimbus_get_bus_config(SLIMBUS_BUS_CONFIG_NORMAL);

	if (bus_cfg == NULL) {
		AUDIO_LOGE("bus cfg is NULL");
		goto exit;
	}

	ret = slimbus_drv_bus_configure(bus_cfg);
	if (ret) {
		AUDIO_LOGE("slimbus bus configure failed");
		goto exit;
	}

	if (pd->lastframer == SLIMBUS_FRAMER_CODEC) {
		ret = slimbus_switch_framer(pd->device_type, SLIMBUS_FRAMER_CODEC);
		AUDIO_LOGI("switch_framer:%#x +", pd->lastframer);
	}

exit:
	resume_exit(device, pd, asppower, aspclk);

	return ret;
}

static int32_t slimbus_resume(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct slimbus_private_data *pd = platform_get_drvdata(pdev);
	int32_t ret;
	uint32_t asppower;
	uint32_t aspclk;

	if (!pd) {
		AUDIO_LOGE("pd is null");
		return -EINVAL;
	}
	asppower = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_power_state_offset);
	aspclk = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_clk_state_offset);

	AUDIO_LOGI("portstate: 0x%x asppower: 0x%x aspclkstate: 0x%x usage_count: 0x%x status: %x, pmuclk: %d aspclk: %d, --",
		pd->portstate, asppower, aspclk, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	ret = resume_resource(device, pd, asppower, aspclk);

	return ret;
}
#endif


#ifdef CONFIG_PM
static int32_t slimbus_runtime_suspend(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct slimbus_private_data *pd = platform_get_drvdata(pdev);
	int32_t ret = 0;
	uint32_t asppower;
	uint32_t aspclk;

	if (!pd) {
		AUDIO_LOGE("pd is null");
		return -EINVAL;
	}

	pd->portstate = slimbus_utils_port_state_get(pd->base_addr);

	AUDIO_LOGI("portstate: 0x%x trackstate: 0x%x usage_count: 0x%x status: %x pmuclk: %d aspclk: %d  ++",
		pd->portstate, slimbus_trackstate_get(), atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	if (pd->portstate != 0)
		AUDIO_LOGE("portstate is nozero: 0x%x", pd->portstate);

	/* make sure last msg has been processed finished */
	mdelay(1);
	slimbus_int_need_clear_set(true);

	ret = slimbus_pause_clock(pd->device_type, SLIMBUS_RT_UNSPECIFIED_DELAY);
	if (ret)
		AUDIO_LOGE("slimbus pause clock failed, ret: %#x", ret);

	/* make sure para has updated */
	mdelay(1);

	clk_disable_unprepare(pd->pmu_audio_clk);

	clk_disable_unprepare(pd->asp_subsys_clk);

	asppower = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_power_state_offset);
	aspclk = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_clk_state_offset);
	AUDIO_LOGI("portstate: 0x%x asppower: 0x%x aspclkstate: 0x%x usage_count: 0x%x status: %x, pmuclk: %d aspclk: %d, --",
		pd->portstate, asppower, aspclk, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	return ret;
}

static int32_t slimbus_runtime_resume(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct slimbus_private_data *pd = platform_get_drvdata(pdev);
	int32_t ret = 0;
	uint32_t asppower;
	uint32_t aspclk;

	if (!pd) {
		AUDIO_LOGE("pd is null");
		return -EINVAL;
	}

	asppower = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_power_state_offset);
	aspclk = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_clk_state_offset);

	AUDIO_LOGI("portstate: 0x%x asppower: 0x%x aspclkstate: 0x%x usage_count: 0x%x status: %x pmuclk: %d aspclk: %d ++",
		pd->portstate, asppower, aspclk, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	if (pd->portstate != 0)
		AUDIO_LOGE("portstate is nozero: 0x%x", pd->portstate);

	ret = clk_prepare_enable(pd->asp_subsys_clk);
	if (ret) {
		AUDIO_LOGE("asp_subsys_clk :clk prepare enable failed");
		goto exit;
	}

	ret = clk_prepare_enable(pd->pmu_audio_clk);
	if (ret) {
		AUDIO_LOGE("pmu_audio_clk :clk prepare enable failed");
		goto exit;
	}
	/* make sure pmu clk has stable */
	mdelay(1);

	slimbus_int_need_clear_set(false);

	ret = slimbus_drv_resume_clock();
	if (ret)
		AUDIO_LOGE("slimbus resume clock failed, ret: %d", ret);

exit:
	asppower = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_power_state_offset);
	aspclk = slimbus_utils_asp_state_get(pd->sctrl_base_addr, pd->asp_clk_state_offset);
	AUDIO_LOGI("portstate: 0x%x asppower: 0x%x aspclkstate: 0x%x usage_count: 0x%x status: %x, pmuclk: %d aspclk: %d, --",
		pd->portstate, asppower, aspclk, atomic_read(&(device->power.usage_count)), device->power.runtime_status,
		clk_get_enable_count(pd->pmu_audio_clk), clk_get_enable_count(pd->asp_subsys_clk));

	return ret;
}
#endif

const struct dev_pm_ops slimbus_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(slimbus_suspend, slimbus_resume)
	SET_RUNTIME_PM_OPS(slimbus_runtime_suspend, slimbus_runtime_resume, NULL)
};

const struct dev_pm_ops *slimbus_pm_get_ops(void)
{
	return &slimbus_pm_ops;
}

