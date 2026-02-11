/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU related init
 * Author: Huawei Technologies Co., Ltd.
 * Create: 2014-2-24
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

#include <linux/ioport.h>
#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <mali_kbase_config.h>
#ifdef CONFIG_DEVFREQ_THERMAL
#include <linux/devfreq_cooling.h>
#endif

#include <trace/events/power.h>
#if (KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE)
#include <linux/pm_opp.h>
#else
#include <linux/opp.h>
#endif

#include "mali_kbase_hisi_callback.h"

#ifdef CONFIG_PM_DEVFREQ
#include <linux/hisi/hisi_devfreq.h>
#endif

#ifdef CONFIG_HISI_HW_VOTE_GPU_FREQ
#include <linux/hisi/hisi_hw_vote.h>
#endif

#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_REPORT_VSYNC
#include <linux/export.h>
#endif
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <backend/gpu/mali_kbase_pm_internal.h>
#include <backend/gpu/mali_kbase_device_internal.h>
#include "mali_kbase_config_platform.h"
#include "mali_kbase_config_hifeatures.h"
#ifdef CONFIG_HISI_IPA_THERMAL
#include <linux/thermal.h>
#endif
#ifdef CONFIG_HISI_DRG
#include <linux/hisi/hisi_drg.h>
#endif

#include <linux/hisi/hisi_gpufreq.h>
#include <linux/hisi/hisi_gpu_hook.h>

#define MALI_TRUE ((uint32_t)1)
#define MALI_FALSE ((uint32_t)0)

#define DEFAULT_POLLING_MS 20

#define RUNTIME_PM_DELAY_1MS 1
#define RUNTIME_PM_DELAY_30MS 30

#define HARD_RESET_AT_POWER_OFF 0

#define KHZ 1000 // KHz
typedef uint32_t mali_bool;

static struct kbase_device *kbase_dev;

#ifdef CONFIG_HISI_HW_VOTE_GPU_FREQ
static struct hvdev *gpu_hvdev;
#endif

typedef enum {
	MALI_ERROR_NONE = 0,
	MALI_ERROR_OUT_OF_GPU_MEMORY,
	MALI_ERROR_OUT_OF_MEMORY,
	MALI_ERROR_FUNCTION_FAILED,
} mali_error;

#ifndef CONFIG_OF
static struct kbase_io_resources io_resources = {
	.job_irq_number = 68, // irq number 68
	.mmu_irq_number = 69, // irq number 69
	.gpu_irq_number = 70, // irq number 70
	.io_memory_region = {
		.start = 0xFC010000,
		.end = 0xFC010000 + (4096 * 4) - 1 // end address
	}
};
#endif /* CONFIG_OF */

static int kbase_set_hi_features_mask(struct kbase_device *kbdev)
{
	const enum kbase_hi_feature *hi_features = NULL;
	u32 gpu_vid;
	u32 product_id;

	gpu_vid = kbdev->gpu_vid;
	product_id = gpu_vid & GPU_ID_VERSION_PRODUCT_ID;
	product_id >>= GPU_ID_VERSION_PRODUCT_ID_SHIFT;

	if (GPU_ID_IS_NEW_FORMAT(product_id)) {
		switch (gpu_vid) {
		/* arch_major, arch_minor, arch_rev, product_major,
		 * version_major, version_minor, version_status
		 */
		case GPU_ID2_MAKE(6, 0, 10, 0, 0, 0, 2):
			hi_features = kbase_hi_feature_tmix_r0p0;
			break;
		case GPU_ID2_MAKE(6, 2, 2, 1, 0, 0, 0):
			hi_features = kbase_hi_feature_thex_r0p0;
			break;
		case GPU_ID2_MAKE(6, 2, 2, 1, 0, 0, 1):
			hi_features = kbase_hi_feature_thex_r0p0;
			break;
		case GPU_ID2_MAKE(7, 2, 1, 1, 0, 0, 0):
			hi_features = kbase_hi_feature_tnox_r0p0;
			break;
		case GPU_ID2_MAKE(7, 4, 0, 2, 1, 0, 0):
			hi_features = kbase_hi_feature_tgox_r1p0;
			break;
		case GPU_ID2_MAKE(7, 0, 9, 0, 1, 1, 0):
			hi_features = kbase_hi_feature_tsix_r1p1;
			break;
		default:
			dev_err(kbdev->dev,
				"[hi-feature]Unknown GPU ID %x", gpu_vid);
			return -EINVAL;
		}
	} else {
		switch (gpu_vid) {
		case GPU_ID_MAKE(GPU_ID_PI_TFRX, 0, 2, 0):
			hi_features = kbase_hi_feature_t880_r0p2;
			break;
		case GPU_ID_MAKE(GPU_ID_PI_T83X, 1, 0, 0):
			hi_features = kbase_hi_feature_t830_r2p0;
			break;
		case GPU_ID_MAKE(GPU_ID_PI_TFRX, 2, 0, 0):
			hi_features = kbase_hi_feature_t880_r2p0;
			break;
		default:
			dev_err(kbdev->dev,
				"[hi-feature]Unknown GPU ID %x", gpu_vid);
			return -EINVAL;
		}
	}

	dev_info(kbdev->dev,
		"[hi-feature]GPU identified as 0x%04x r%dp%d status %d",
		((gpu_vid & GPU_ID_VERSION_PRODUCT_ID) >>
			GPU_ID_VERSION_PRODUCT_ID_SHIFT),
		(gpu_vid & GPU_ID_VERSION_MAJOR) >> GPU_ID_VERSION_MAJOR_SHIFT,
		(gpu_vid & GPU_ID_VERSION_MINOR) >> GPU_ID_VERSION_MINOR_SHIFT,
		((gpu_vid & GPU_ID_VERSION_STATUS) >>
			GPU_ID_VERSION_STATUS_SHIFT));

	for (; *hi_features != KBASE_HI_FEATURE_END; hi_features++)
		set_bit(*hi_features, &kbdev->hi_features_mask[0]);

	return 0;
}

#ifdef CONFIG_HISI_GPU_CORE_HOTPLUG
#define CORE_MASK_LEVEL_1	(4)
#define CORE_MASK_LEVEL_2	(8)
#define CORE_MASK_LEVEL_3	(12)
#define CORE_MASK_LEVEL_4	(16)

#define CORE_MASK_LEVEL_1_VALUE	(0x1111)
#define CORE_MASK_LEVEL_2_VALUE	(0x3333)
#define CORE_MASK_LEVEL_3_VALUE	(0x7777)
#define CORE_MASK_LEVEL_4_VALUE	(0xFFFF)

void hisi_gpu_thermal_cores_control(u64 cores)
{
	u64 updated_core_mask = CORE_MASK_LEVEL_4_VALUE;
	unsigned long flags;

	if (kbase_dev == NULL) {
		pr_err("[Mali]kbase_dev is null,seems platform uninitialized");
		return;
	}

	switch (cores) {
	case CORE_MASK_LEVEL_1:
		updated_core_mask = CORE_MASK_LEVEL_1_VALUE;
		kbase_dev->pm.thermal_controlling = true;
		break;
	case CORE_MASK_LEVEL_2:
		updated_core_mask = CORE_MASK_LEVEL_2_VALUE;
		kbase_dev->pm.thermal_controlling = true;
		break;
	case CORE_MASK_LEVEL_3:
		updated_core_mask = CORE_MASK_LEVEL_3_VALUE;
		kbase_dev->pm.thermal_controlling = true;
		break;
	case CORE_MASK_LEVEL_4:
		updated_core_mask = CORE_MASK_LEVEL_4_VALUE;
		kbase_dev->pm.thermal_controlling = false;
		break;
	default:
		dev_err(kbase_dev->dev,
			"Invalid cores set,support 0xF,0xFF,0xFFF,0xFFFF\n");
		break;
	}

	kbase_dev->pm.thermal_required_core_mask = updated_core_mask;
	spin_lock_irqsave(&kbase_dev->hwaccess_lock, flags);
	kbase_pm_set_debug_core_mask(kbase_dev, updated_core_mask, updated_core_mask, updated_core_mask);
	spin_unlock_irqrestore(&kbase_dev->hwaccess_lock, flags);
}
EXPORT_SYMBOL(hisi_gpu_thermal_cores_control);
#endif

static inline void kbase_platform_on(struct kbase_device *kbdev)
{
	if (kbdev->regulator) {
		int refcount;

		if (unlikely(regulator_enable(kbdev->regulator))) {
			dev_err(kbdev->dev, "Failed to enable regulator\n");
			BUG_ON(1);
		}

		refcount = atomic_inc_return(&kbdev->regulator_refcount);
		if (unlikely(refcount != 1))
			dev_err(kbdev->dev,
				"%s called not match, refcount:[%d]\n",
				__func__, refcount);

		if (kbdev->gpu_vid == 0) {
			kbdev->gpu_vid = kbase_reg_read(kbdev,
				GPU_CONTROL_REG(GPU_ID));
			if (unlikely(kbase_set_hi_features_mask(kbdev)))
				dev_err(kbdev->dev,
					"Failed to set hi features\n");
		}

		if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0004) ||
			(kbdev->gpu_chip_type == 2)) {
			kbase_reg_write(kbdev, GPU_CONTROL_REG(PWR_KEY),
				KBASE_PWR_KEY_VALUE);
			kbase_reg_write(kbdev, GPU_CONTROL_REG(PWR_OVERRIDE1),
				KBASE_PWR_OVERRIDE_VALUE);
		}

		if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0003)) {
			int value;

			value = readl(kbdev->pctrlreg + PERI_CTRL19) &
				GPU_X2P_GATOR_BYPASS;
			writel(value, kbdev->pctrlreg + PERI_CTRL19);
		}
	}
}

static inline void kbase_platform_off(struct kbase_device *kbdev)
{
	int refcount = atomic_read(&kbdev->regulator_refcount);

	if (kbdev->regulator && refcount) {
		if (unlikely(regulator_disable(kbdev->regulator)))
			dev_err(kbdev->dev, "Failed to disable regulator\n");

		refcount = atomic_dec_return(&kbdev->regulator_refcount);
		if (unlikely(refcount != 0))
			dev_err(kbdev->dev,
				"%s called not match, refcount:[%d]\n",
				__func__, refcount);
	}
}

#ifdef CONFIG_PM_DEVFREQ
static struct hisi_devfreq_data hisi_devfreq_priv_data = {
	.vsync_hit = 0,
	.cl_boost = 0,
};

static inline void gpu_devfreq_rcu_read_lock(void)
{
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	rcu_read_lock();
#endif
}

static inline void gpu_devfreq_rcu_read_unlock(void)
{
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	rcu_read_unlock();
#endif
}

static inline void gpu_devfreq_opp_put(struct dev_pm_opp *opp)
{
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	dev_pm_opp_put(opp);
#endif
}

static int mali_kbase_devfreq_target(struct device *dev, unsigned long *_freq,
	u32 flags)
{
	struct kbase_device *kbdev = NULL;
	unsigned long old_freq;
	struct dev_pm_opp *opp = NULL;
	unsigned long freq;
#ifdef CONFIG_HISI_IPA_THERMAL
	int gpu_id;
#endif

	KBASE_DEBUG_ASSERT(dev != NULL);
	kbdev = (struct kbase_device *)dev->platform_data;
	KBASE_DEBUG_ASSERT(kbdev != NULL);
	old_freq = kbdev->devfreq->previous_freq;
	gpu_devfreq_rcu_read_lock();
	opp = devfreq_recommended_opp(dev, _freq, flags);
	if (IS_ERR(opp)) {
		pr_err("[mali]Failed to get Operating Performance Point\n");
		gpu_devfreq_rcu_read_unlock();
		return PTR_ERR(opp);
	}
	freq = dev_pm_opp_get_freq(opp);
	gpu_devfreq_opp_put(opp);
	gpu_devfreq_rcu_read_unlock();

#ifdef CONFIG_HISI_IPA_THERMAL
	gpu_id = ipa_get_actor_id("gpu");
	if (gpu_id < 0) {
		pr_err("[mali]Failed to get ipa actor id for gpu.\n");
		return -ENODEV;
	}
	freq = ipa_freq_limit(gpu_id, freq);
#endif

	if (old_freq == freq)
		goto update_target;

	trace_clock_set_rate("clk-g3d", freq, raw_smp_processor_id());

#ifdef CONFIG_HISI_HW_VOTE_GPU_FREQ
	if (hisi_hv_set_freq(gpu_hvdev, freq / KHZ)) {
#else
	if (clk_set_rate((kbdev->clk), freq)) {
#endif
		pr_err("[mali]Failed to set gpu freqency, [%lu->%lu]\n",
			old_freq, freq);
		return -ENODEV;
	}

update_target:
	*_freq = freq;

	return 0;
}

#ifdef CONFIG_MALI_BOUND_REPORT
static bool mali_kbase_bound_report(struct kbase_device *kbdev,
	bool bound_event)
{
	bool out_data = false;
	bool bound_it = false;

	if (kbdev == NULL)
		return false;

	if (bound_event)
		kbdev->bound_report_info.bound_times_in_fifo++;
	if (kfifo_is_full(&kbdev->bound_report_info.bound_fifo)) {
		kfifo_out(&kbdev->bound_report_info.bound_fifo, &out_data, 1);
		if (out_data)
			kbdev->bound_report_info.bound_times_in_fifo--;
	}
	kfifo_in(&kbdev->bound_report_info.bound_fifo, &bound_event, 1);

	/* bound threshold 3 times */
	if (kbdev->bound_report_info.bound_times_in_fifo > 3)
		bound_it = true; // bound detected!!!

	--kbdev->bound_report_info.duration_times;
	/* new bound state is changed */
	if (bound_it != kbdev->bound_report_info.report_bound_flag) {
		/* check the period, make sure the previous flag state last at
		 * least for a while(BOUND_DURATION_TIMES*20 ms)
		 */
		if (kbdev->bound_report_info.duration_times <= 0) {
			kbdev->bound_report_info.report_bound_flag = bound_it;
			kbdev->bound_report_info.duration_times =
				BOUND_DURATION_TIMES;
		}
	}

	return kbdev->bound_report_info.report_bound_flag;
}
#endif

#ifdef CONFIG_DEVFREQ_THERMAL
void mali_kbase_devfreq_detect_bound_worker(struct work_struct *work)
{
	int err = 0;
	struct kbase_hisi_device_data *hisi_data = NULL;
	struct kbase_device *kbdev = NULL;
	bool bound_event = false;
	struct thermal_cooling_device *cdev = NULL;

	KBASE_DEBUG_ASSERT(work != NULL);
	hisi_data = container_of(work,
		struct kbase_hisi_device_data, bound_detect_work);
	KBASE_DEBUG_ASSERT(hisi_data != NULL);
	kbdev = container_of(hisi_data,
		struct kbase_device, hisi_dev_data);
	KBASE_DEBUG_ASSERT(kbdev != NULL);

	cdev = kbdev->devfreq_cooling;
#if defined(CONFIG_MALI_MIDGARD_DVFS)
	bound_event = kbase_ipa_dynamic_bound_detect(
		kbdev->hisi_dev_data.ipa_ctx, &err,
		kbdev->hisi_dev_data.bound_detect_freq,
		kbdev->hisi_dev_data.bound_detect_btime,
		cdev->ipa_enabled);
#endif

	cdev->ipa_enabled = false;

	cdev->bound_event = bound_event;

#ifdef CONFIG_MALI_BOUND_REPORT
	kbdev->bound_report_info.report_bound_flag =
		mali_kbase_bound_report(kbdev, bound_event);
#endif
}

static void mali_kbase_devfreq_detect_bound(struct kbase_device *kbdev,
	unsigned long cur_freq, unsigned long btime)
{
	kbdev->hisi_dev_data.bound_detect_freq = cur_freq;
	kbdev->hisi_dev_data.bound_detect_btime = btime;
	queue_work(system_unbound_wq,
		&kbdev->hisi_dev_data.bound_detect_work);
}
#endif

static int mali_kbase_get_dev_status(struct device *dev,
	struct devfreq_dev_status *stat)
{
	struct hisi_devfreq_data *priv_data = &hisi_devfreq_priv_data;
	struct kbase_device *kbdev = NULL;
#ifdef CONFIG_HISI_HW_VOTE_GPU_FREQ
	u32 freq = 0;
#endif

	KBASE_DEBUG_ASSERT(dev != NULL);
	KBASE_DEBUG_ASSERT(stat != NULL);
	kbdev = (struct kbase_device *)dev->platform_data;
	KBASE_DEBUG_ASSERT(kbdev != NULL);
	if (kbdev->pm.backend.metrics.kbdev != kbdev) {
		pr_err("%s pm backend metrics not initialized\n", __func__);
		return -EINVAL;
	}

	(void)kbase_pm_get_dvfs_action(kbdev);
	stat->busy_time = kbdev->pm.backend.metrics.utilisation;
	stat->total_time = 100; /* base time 100ns */
#ifdef CONFIG_HISI_HW_VOTE_GPU_FREQ
	hisi_hv_get_last(gpu_hvdev, &freq);
	stat->current_frequency = (unsigned long)freq * KHZ;
#else
	stat->current_frequency = clk_get_rate(kbdev->clk);
#endif
	priv_data->vsync_hit = kbdev->pm.backend.metrics.vsync_hit;
	priv_data->cl_boost = kbdev->pm.backend.metrics.cl_boost;
	stat->private_data = (void *)priv_data;

#ifdef CONFIG_DEVFREQ_THERMAL
	/* Avoid sending HWC dump cmd to GPU when GPU is power-off */
	if (kbdev->pm.backend.gpu_powered)
		(void)mali_kbase_devfreq_detect_bound(kbdev,
			stat->current_frequency, stat->busy_time);

#if KERNEL_VERSION(4, 1, 15) <= LINUX_VERSION_CODE
	memcpy(&kbdev->devfreq->last_status, stat, sizeof(*stat)); /* unsafe_function_ignore: memcpy */
#else
	memcpy(&kbdev->devfreq_cooling->last_status, stat, sizeof(*stat)); /* unsafe_function_ignore: memcpy */
#endif
#endif

	return 0;
}

static struct devfreq_dev_profile mali_kbase_devfreq_profile = {
	/* it would be abnormal to enable devfreq monitor
	 * during initialization.
	 */
	.polling_ms = DEFAULT_POLLING_MS, // STOP_POLLING,
	.target = mali_kbase_devfreq_target,
	.get_dev_status = mali_kbase_get_dev_status,
};
#endif

#ifdef CONFIG_REPORT_VSYNC
void mali_kbase_pm_report_vsync(int buffer_updated)
{
	unsigned long flags;

	if (kbase_dev) {
		spin_lock_irqsave(&kbase_dev->pm.backend.metrics.lock, flags);
		kbase_dev->pm.backend.metrics.vsync_hit = buffer_updated;
		spin_unlock_irqrestore(&kbase_dev->pm.backend.metrics.lock,
			flags);
	}
}
EXPORT_SYMBOL(mali_kbase_pm_report_vsync);
#endif

#ifdef CONFIG_HISI_GPU_AI_FENCE_INFO
int mali_kbase_report_fence_info(struct kbase_fence_info *fence)
{
	if (kbase_dev == NULL || fence == NULL)
		return -EINVAL;

	if (kbase_dev->game_pid != fence->game_pid)
		kbase_dev->signaled_seqno = 0;

	kbase_dev->game_pid = fence->game_pid;
	fence->signaled_seqno = kbase_dev->signaled_seqno;

	return 0;
}
#endif
EXPORT_SYMBOL(mali_kbase_report_fence_info);

#ifdef CONFIG_MALI_MIDGARD_DVFS
int kbase_platform_dvfs_event(struct kbase_device *kbdev, u32 utilisation,
	u32 util_gl_share, u32 util_cl_share[2])
{
	return 1;
}

int kbase_platform_dvfs_enable(struct kbase_device *kbdev, bool enable,
	int freq)
{
	unsigned long flags;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	if (enable != kbdev->pm.backend.metrics.timer_active) {
		if (enable) {
			spin_lock_irqsave(&kbdev->pm.backend.metrics.lock,
				flags);
			kbdev->pm.backend.metrics.timer_active = MALI_TRUE;
			spin_unlock_irqrestore(&kbdev->pm.backend.metrics.lock,
				flags);
			hrtimer_start(&kbdev->pm.backend.metrics.timer,
				HR_TIMER_DELAY_MSEC(kbdev->pm.dvfs_period),
				HRTIMER_MODE_REL);
		} else {
			spin_lock_irqsave(&kbdev->pm.backend.metrics.lock,
				flags);
			kbdev->pm.backend.metrics.timer_active = MALI_FALSE;
			spin_unlock_irqrestore(&kbdev->pm.backend.metrics.lock,
				flags);
			hrtimer_cancel(&kbdev->pm.backend.metrics.timer);
		}
	}

	return 1;
}
#endif

#ifdef CONFIG_DEVFREQ_THERMAL
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
static unsigned long
hisi_model_static_power(struct devfreq *devfreq __maybe_unused,
	unsigned long voltage)
#else
static unsigned long
hisi_model_static_power(unsigned long voltage)
#endif
{
	int temperature;
	const unsigned long voltage_cubed =
		(voltage * voltage * voltage) >> 10;
	unsigned long temp;
	unsigned long temp_squared;
	unsigned long temp_cubed;
	unsigned long temp_scaling_factor;
	struct device_node *dev_node = NULL;
	int ret;
	int i;
	const char *temperature_scale_capacitance[5];
	int capacitance[5] = {0};

	KBASE_DEBUG_ASSERT(devfreq != NULL);
	/* multiplication overflow check */
	hisi_uint64_mulcheck(voltage, voltage);
	hisi_uint64_mulcheck(voltage * voltage, voltage);

	dev_node = of_find_node_by_name(NULL, "capacitances");
	if (dev_node != NULL) {
		for (i = 0; i < 5; i++) {
			ret = of_property_read_string_index(dev_node,
				"hisilicon,gpu_temp_scale_capacitance", i,
				&temperature_scale_capacitance[i]);
			if (ret) {
				pr_err("%s temperature_scale_capacitance [%d] read err\n",
					__func__, i);
				continue;
			}

			ret = kstrtoint(temperature_scale_capacitance[i], 10,
				&capacitance[i]);
			if (ret)
				continue;
		}
	}

	temperature = get_soc_temp();
	/* convert to temp, temperature is 90000, so temp is 90 */
	temp = (unsigned long)((long)temperature) / 1000;
	/* multiplication overflow check */
	hisi_uint64_mulcheck(temp, temp);
	temp_squared = temp * temp;
	hisi_uint64_mulcheck(temp_squared, temp);
	temp_cubed = temp_squared * temp;
	/* multiplication overflow check */
	hisi_uint64_mulcheck((unsigned long)capacitance[3], temp_cubed);
	hisi_uint64_mulcheck((unsigned long)capacitance[2], temp_squared);
	hisi_uint64_mulcheck((unsigned long)capacitance[1], temp);
	hisi_uint64_addcheck((capacitance[3] * temp_cubed),
		(capacitance[2] * temp_squared));
	hisi_uint64_addcheck((capacitance[1] * temp),
		(capacitance[3] * temp_cubed + capacitance[2] * temp_squared));
	hisi_uint64_addcheck((unsigned long)capacitance[0],
		(capacitance[3] * temp_cubed + capacitance[2] * temp_squared +
		(capacitance[1] * temp)));
	temp_scaling_factor = capacitance[3] * temp_cubed +
		capacitance[2] * temp_squared +
		capacitance[1] * temp + capacitance[0];

	/* multiplication overflow check */
	hisi_uint64_mulcheck((unsigned long)capacitance[4], voltage_cubed);
	hisi_uint64_mulcheck(((capacitance[4] * voltage_cubed) >> 20),
		temp_scaling_factor);
	/* mW */
	return (((capacitance[4] * voltage_cubed) >> 20) *
		temp_scaling_factor) / 1000000; /* [false alarm]: no problem - fortify check */
}

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
static unsigned long
hisi_model_dynamic_power(struct devfreq *devfreq __maybe_unused,
	unsigned long freq, unsigned long voltage)
#else
static unsigned long
hisi_model_dynamic_power(unsigned long freq, unsigned long voltage)
#endif
{
	/* The inputs: freq (f) is in Hz, and voltage (v) in mV.
	 * The coefficient (c) is in mW/(MHz mV mV).
	 *
	 * This function calculates the dynamic power after this formula:
	 * Pdyn (mW) = c (mW/(MHz*mV*mV)) * v (mV) * v (mV) * f (MHz)
	 */
	const unsigned long v2 = (voltage * voltage) / 1000; /* m*(V*V) */
	const unsigned long f_mhz = freq / 1000000; /* MHz */
	unsigned long coefficient = 3600; /* mW/(MHz*mV*mV) */
	struct device_node *dev_node = NULL;
	u32 prop = 0;

	KBASE_DEBUG_ASSERT(devfreq != NULL);
	/* multiplication overflow check */
	hisi_uint64_mulcheck(voltage, voltage);

	dev_node = of_find_node_by_name(NULL, "capacitances");
	if (dev_node != NULL) {
		int ret = of_property_read_u32(dev_node,
			"hisilicon,gpu_dyn_capacitance", &prop);
		if (ret == 0)
			coefficient = prop;
	}

	/* multiplication overflow check */
	hisi_uint64_mulcheck(coefficient, v2);
	hisi_uint64_mulcheck(coefficient * v2, f_mhz);
	return (coefficient * v2 * f_mhz) / 1000000; /* mW */
}

#if (KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE)
static struct devfreq_cooling_power hisi_model_ops = {
#else
static struct devfreq_cooling_ops hisi_model_ops = {
#endif
	.get_static_power = hisi_model_static_power,
	.get_dynamic_power = hisi_model_dynamic_power,
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	.get_real_power = NULL,
#endif
};
#endif

#ifdef CONFIG_GPU_GMC_GENERIC
struct gmc_ops kbase_gmc_ops = {
	.compress_kctx = kbase_gmc_compress,
	.decompress_kctx = kbase_gmc_decompress,
	.meminfo_open = kbase_gmc_meminfo_open,
};
#endif

static int kbase_platform_backend_init(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_LAST_BUFFER
	hisi_lb_callbacks *lb_cbs = NULL;
#endif
	kbdev->hisi_dev_data.callbacks =
		(struct kbase_hisi_callbacks *)gpu_get_callbacks();

#ifdef CONFIG_MALI_LAST_BUFFER
	lb_cbs = kbase_hisi_get_lb_cbs(kbdev);
	if (lb_cbs->lb_init(kbdev))
		dev_err(kbdev->dev, "Init last buffer failed.\n");
#endif

#if defined(CONFIG_MALI_MIDGARD_DVFS) && defined(CONFIG_DEVFREQ_THERMAL)
	/* Add ipa GPU HW bound detection */
	kbdev->hisi_dev_data.ipa_ctx = kbase_dynipa_init(kbdev);
	if (!kbdev->hisi_dev_data.ipa_ctx) {
		dev_err(kbdev->dev,
			"GPU HW bound detection sub sys initialization failed\n");
	} else {
		INIT_WORK(&kbdev->hisi_dev_data.bound_detect_work,
			  mali_kbase_devfreq_detect_bound_worker);
	}
#endif

#ifdef CONFIG_GPU_GMC_GENERIC
	if (gmc_register_device(&kbase_gmc_ops, &kbdev->hisi_dev_data.kbase_gmc_device)) {
		/* Failed to initialize GMC. */
		dev_warn(kbdev->dev, "GMC initialization failed.\n");
	}
#endif

	return 0;
}

static void kbase_platform_backend_term(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_LAST_BUFFER
	hisi_lb_callbacks *lb_cbs = kbase_hisi_get_lb_cbs(kbdev);

	if (lb_cbs->lb_term(kbdev))
		dev_err(kbdev->dev, "Term last buffer failed.\n");
#endif

#if defined(CONFIG_MALI_MIDGARD_DVFS) && defined(CONFIG_DEVFREQ_THERMAL)
	if (kbdev->hisi_dev_data.ipa_ctx)
		kbase_dynipa_term(kbdev->hisi_dev_data.ipa_ctx);
#endif

	kbdev->hisi_dev_data.callbacks = NULL;
}

#ifdef CONFIG_DEVFREQ_THERMAL
void hisi_gpu_devfreq_cooling_init(struct kbase_device *kbdev)
{
	int err;
	struct devfreq_cooling_power *callbacks = NULL;

	callbacks = (struct devfreq_cooling_power *)POWER_MODEL_CALLBACKS;

	kbdev->devfreq_cooling = of_devfreq_cooling_register_power(
		kbdev->dev->of_node,
		kbdev->devfreq,
		callbacks);
	if (IS_ERR_OR_NULL(kbdev->devfreq_cooling)) {
		err = PTR_ERR(kbdev->devfreq_cooling);
		dev_err(kbdev->dev,
			"Failed to register cooling device (%d)\n",
			err);
	}
}
#else
static inline void hisi_gpu_devfreq_cooling_init(struct kbase_device *kbdev)
{
	(void)kbdev;
}
#endif

#ifdef CONFIG_PM_DEVFREQ
void hisi_gpu_devfreq_initial_freq(const struct kbase_device *kbdev)
{
#ifdef CONFIG_HISI_HW_VOTE_GPU_FREQ
	u32 freq = 0;
	unsigned long freq_hz;
	struct dev_pm_opp *opp = NULL;
	struct device *dev = kbdev->dev;

	gpu_hvdev = hisi_hvdev_register(dev, "gpu-freq", "vote-src-1");
	if (gpu_hvdev == NULL)
		pr_err("[%s] register hvdev fail!\n", __func__);

	hisi_hv_get_last(gpu_hvdev, &freq);
	freq_hz = (unsigned long)freq * KHZ;

	gpu_devfreq_rcu_read_lock();
	opp = dev_pm_opp_find_freq_ceil(dev, &freq_hz);
	if (IS_ERR(opp))
		freq_hz = mali_kbase_devfreq_profile.freq_table[0];

	gpu_devfreq_opp_put(opp);
	gpu_devfreq_rcu_read_unlock();

	/* update last freq in hv driver */
	hisi_hv_set_freq(gpu_hvdev, freq_hz / KHZ);
	mali_kbase_devfreq_profile.initial_freq = freq_hz;
#else
	mali_kbase_devfreq_profile.initial_freq = clk_get_rate(kbdev->clk);
#endif
}

void hisi_gpu_devfreq_init(struct kbase_device *kbdev)
{
	struct device *dev = kbdev->dev;
	int opp_count;

	opp_count = dev_pm_opp_get_opp_count(dev);
	if (opp_count <= 0)
		return;

	if (fhss_init(dev))
		dev_err(dev, "[gpufreq] Failed to init fhss\n");

	/* dev_pm_opp_of_add_table(dev) has been called by ddk */

	hisi_gpu_devfreq_initial_freq(kbdev);

	dev_set_name(dev, "gpufreq");
	kbdev->devfreq = devfreq_add_device(dev,
		&mali_kbase_devfreq_profile,
		GPU_DEFAULT_GOVERNOR,
		NULL);

	if (!IS_ERR_OR_NULL(kbdev->devfreq))
		hisi_gpu_devfreq_cooling_init(kbdev);

#ifdef CONFIG_HISI_DRG
	drg_devfreq_register(kbdev->devfreq);
#endif
}
#else
static inline void hisi_gpu_devfreq_init(struct kbase_device *kbdev)
{
	(void)kbdev;
}
#endif/*CONFIG_PM_DEVFREQ*/

static int kbase_platform_init(struct kbase_device *kbdev)
{
	struct device *dev = NULL;

	KBASE_DEBUG_ASSERT(kbdev != NULL);
	dev = kbdev->dev;
	dev->platform_data = kbdev;
	kbase_dev = kbdev;

	/* Init the hisilicon platform related data first. */
	if (kbase_platform_backend_init(kbdev)) {
		dev_err(kbdev->dev, "platform backend init failed.\n");
		return -EINVAL;
	}

	kbdev->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(kbdev->clk)) {
		dev_err(kbdev->dev, "Failed to get clk\n");
		return -EINVAL;
	}

	kbdev->regulator = devm_regulator_get(dev, "gpu");
	if (IS_ERR(kbdev->regulator)) {
		dev_err(kbdev->dev, "Failed to get regulator\n");
		return -EINVAL;
	}

	atomic_set(&kbdev->regulator_refcount, 0);

	hisi_gpu_devfreq_init(kbdev);

	/* dev name maybe modified by hisi_gpu_devfreq_init */
	dev_set_name(dev, "gpu");
	return 0;
}

static void kbase_platform_term(struct kbase_device *kbdev)
{
	KBASE_DEBUG_ASSERT(kbdev != NULL);
#ifdef CONFIG_PM_DEVFREQ
#ifdef CONFIG_HISI_DRG
	drg_devfreq_unregister(kbdev->devfreq);
#endif

	devfreq_remove_device(kbdev->devfreq);
#endif

	/* term the hisilicon platform related data at last. */
	kbase_platform_backend_term(kbdev);
}

kbase_platform_funcs_conf platform_funcs = {
	.platform_init_func = &kbase_platform_init,
	.platform_term_func = &kbase_platform_term,
};

static int pm_callback_power_on(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_MIDGARD_RT_PM
	int result;
	struct device *dev = kbdev->dev;

	/* norr es */
	if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0014)) {
		unsigned int reg;
		/* read PERI_CTRL21 and set it's [0~16]bit to 0,
		 * disable mem shutdown by software
		 */
		reg = readl(kbdev->pctrlreg + PERI_CTRL21) &
			MASK_DISABLESDBYSF;
		writel(reg, kbdev->pctrlreg + PERI_CTRL21);
		/* read PERI_CTRL93 and set it's [1~17]bit to 0,
		 * disable deep sleep by software
		 */
		reg = readl(kbdev->pctrlreg + PERI_CTRL93) &
			MASK_DISABLEDSBYSF;
		writel(reg, kbdev->pctrlreg + PERI_CTRL93);
	}
	/* norr cs, trym es, gondul */
	if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0015)) {
		unsigned int value;
		/**
		 * disable deep sleep by software
		 * norr cs read PERI_CTRL93 set [1~17]bit to 0,
		 * trym read PERI_CTRL92 set [0~16]bit to 0 and
		 * gondul read PERI_CTRL92 set [0~6]bit to 0
		 */
		value = readl(kbdev->pctrlreg + DEEP_SLEEP_BYSW) &
			MASK_DISABLEDSBYSF;
		writel(value, kbdev->pctrlreg + DEEP_SLEEP_BYSW);
	}

	if (unlikely(dev->power.disable_depth > 0)) {
		kbase_platform_on(kbdev);
	} else {
		result = pm_runtime_resume(dev);
		if (result < 0 && result == -EAGAIN)
			kbase_platform_on(kbdev);
		else if (result < 0)
			pr_err("[mali]pm_runtime_resume failed with result %d\n",
				result);
	}

	return 1;
#else
	kbase_platform_on(kbdev);

	return 1;
#endif
}

static void pm_callback_power_off(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_MIDGARD_RT_PM
	struct device *dev = kbdev->dev;
	int ret;
	int retry = 0;

	/* norr es */
	if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0014)) {
		unsigned int value;
		/* read PERI_CTRL21 and set it's [0~16]bit to 1,
		 * enable mem shutdown by software
		 */
		value = readl(kbdev->pctrlreg + PERI_CTRL21) |
			MASK_ENABLESDBYSF;
		writel(value, kbdev->pctrlreg + PERI_CTRL21);
		/* read PERI_CTRL93 and set it's [1~17]bit to 1,
		 * enable deep sleep by software
		 */
		value = readl(kbdev->pctrlreg + PERI_CTRL93) |
			MASK_ENABLEDSBYSF;
		writel(value, kbdev->pctrlreg + PERI_CTRL93);
	}
	/* norr cs, trym es, gondul */
	if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0015)) {
		unsigned int value;
		/**
		 * enable deep sleep by software
		 * norr cs read PERI_CTRL93 set [1~17]bit to 1,
		 * trym read PERI_CTRL92 set [0~16]bit to 1,
		 * gondul read PERI_CTRL92 set [0~6]bit to 1
		 */
		value = readl(kbdev->pctrlreg + DEEP_SLEEP_BYSW) |
			MASK_ENABLEDSBYSF;
		writel(value, kbdev->pctrlreg + DEEP_SLEEP_BYSW);
	}

#if HARD_RESET_AT_POWER_OFF
	/* Cause a GPU hard reset to test whether we have actually idled the GPU
	 * and that we properly reconfigure the GPU on power up.
	 * Usually this would be dangerous, but if the GPU is working correctly
	 * it should be completely safe as the GPU should not be active at this
	 * point. However this is disabled normally because it will most likely
	 * interfere with bus logging etc.
	 */
	KBASE_TRACE_ADD(kbdev, CORE_GPU_HARD_RESET, NULL, NULL, 0u, 0);
	kbase_reg_write(kbdev, GPU_CONTROL_REG(GPU_COMMAND),
		GPU_COMMAND_HARD_RESET);
#endif

	if (unlikely(dev->power.disable_depth > 0)) {
		kbase_platform_off(kbdev);
	} else {
		do {
			if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0007))
				ret = pm_schedule_suspend(dev,
					RUNTIME_PM_DELAY_1MS);
			else
				ret = pm_schedule_suspend(dev,
					RUNTIME_PM_DELAY_30MS);
			if (ret != -EAGAIN) {
				if (unlikely(ret < 0)) {
					pr_err("[mali] pm_schedule_suspend fail %d\n",
						ret);
					WARN_ON(1);
				}

				/* correct status */
				break;
			}

			/* -EAGAIN, repeated attempts for 1s totally */
			msleep(50); /* sleep 50 ms */
		} while (++retry < 20); /* loop 20 */
	}
#else
	kbase_platform_off(kbdev);
#endif
}

static int pm_callback_runtime_init(struct kbase_device *kbdev)
{
	KBASE_DEBUG_ASSERT(kbdev != NULL);
	pm_suspend_ignore_children(kbdev->dev, true);
	pm_runtime_enable(kbdev->dev);
	return 0;
}

static void pm_callback_runtime_term(struct kbase_device *kbdev)
{
	KBASE_DEBUG_ASSERT(kbdev != NULL);
	pm_runtime_disable(kbdev->dev);
}

static void pm_callback_runtime_off(struct kbase_device *kbdev)
{
#ifdef CONFIG_PM_DEVFREQ
	devfreq_suspend_device(kbdev->devfreq);
#elif defined(CONFIG_MALI_MIDGARD_DVFS)
	kbase_platform_dvfs_enable(kbdev, false, 0);
#endif

	kbase_platform_off(kbdev);
}

static int pm_callback_runtime_on(struct kbase_device *kbdev)
{
	/* norr es */
	if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0014)) {
		unsigned int value;
		/* read PERI_CTRL21 and set it's [0~16]bit to 0,
		 * disable mem shutdown by software
		 */
		value = readl(kbdev->pctrlreg + PERI_CTRL21) &
			MASK_DISABLESDBYSF;
		writel(value, kbdev->pctrlreg + PERI_CTRL21);
		/* read PERI_CTRL93 and set it's [1~17]bit to 0,
		 * disable deep sleep by software
		 */
		value = readl(kbdev->pctrlreg + PERI_CTRL93) &
			MASK_DISABLEDSBYSF;
		writel(value, kbdev->pctrlreg + PERI_CTRL93);
	}
	/* norr cs, trym es, gondul */
	if (kbase_has_hi_feature(kbdev, KBASE_FEATURE_HI0015)) {
		unsigned int value;
		/**
		 * disable deep sleep by software
		 * norr cs read PERI_CTRL93 set [1~17]bit to 0,
		 * trym read PERI_CTRL92 set [0~16]bit to 0,
		 * gondul read PERI_CTRL92 set [0~6]bit to 0
		 */
		value = readl(kbdev->pctrlreg + DEEP_SLEEP_BYSW) &
			MASK_DISABLEDSBYSF;
		writel(value, kbdev->pctrlreg + DEEP_SLEEP_BYSW);
	}
	kbase_platform_on(kbdev);

#ifdef CONFIG_PM_DEVFREQ
	devfreq_resume_device(kbdev->devfreq);
#elif defined(CONFIG_MALI_MIDGARD_DVFS)
	if (kbase_platform_dvfs_enable(kbdev, true, 0) != MALI_TRUE)
		return -EPERM;
#endif

	return 0;
}

static inline void pm_callback_suspend(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_MIDGARD_RT_PM
	if (!pm_runtime_status_suspended(kbdev->dev))
		pm_callback_runtime_off(kbdev);
	else
		pr_err("%s pm_runtime_status_suspended!\n", __func__);
#else
	pm_callback_power_off(kbdev);
#endif
}

static inline void pm_callback_resume(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_MIDGARD_RT_PM
	if (!pm_runtime_status_suspended(kbdev->dev))
		pm_callback_runtime_on(kbdev);
	else
		pm_callback_power_on(kbdev);
#else
	pm_callback_power_on(kbdev);
#endif
}

static inline int pm_callback_runtime_idle(struct kbase_device *kbdev)
{
	return 1;
}

struct kbase_pm_callback_conf pm_callbacks = {
	.power_on_callback = pm_callback_power_on,
	.power_off_callback = pm_callback_power_off,
	.power_suspend_callback = pm_callback_suspend,
	.power_resume_callback = pm_callback_resume,
#ifdef CONFIG_MALI_MIDGARD_RT_PM
	.power_runtime_init_callback = pm_callback_runtime_init,
	.power_runtime_term_callback = pm_callback_runtime_term,
	.power_runtime_off_callback = pm_callback_runtime_off,
	.power_runtime_on_callback = pm_callback_runtime_on,
	.power_runtime_idle_callback = pm_callback_runtime_idle
#else
	.power_runtime_init_callback = NULL,
	.power_runtime_term_callback = NULL,
	.power_runtime_off_callback = NULL,
	.power_runtime_on_callback = NULL,
	.power_runtime_idle_callback = NULL
#endif
};

static struct kbase_platform_config hi_platform_config = {
#ifndef CONFIG_OF
	.io_resources = &io_resources
#endif
};

struct kbase_platform_config *kbase_get_platform_config(void)
{
	return &hi_platform_config;
}

int kbase_platform_early_init(void)
{
	/* Nothing needed at this stage */
	return 0;
}
