/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU related data structs
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */
#ifndef MALI_KBASE_HISILICON_H
#define MALI_KBASE_HISILICON_H

#include "mali_kbase_hisi_callback.h"
#include "linux/gmc.h"
#include "hisi_ipa/mali_kbase_ipa_ctx.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

struct gpu_error_num {
	u32 page_fault;
	u32 gpu_fault;
	u32 soft_reset;
	u32 hard_reset;
	u64 ts;
};

/* Always should keep same value with gpu_props.num_job_slots */
#define JOB_SLOT_NUM 3
/* Always should keep same definition with mali_kbase_jm_defs.h */
#ifndef SLOT_RB_SIZE
#define SLOT_RB_SIZE 2
#endif

#ifdef CONFIG_HISI_GPU_DPM_PCR
struct hisi_gpu_dpm {
	bool dpm_init_status;
	bool dpm_register_status;
	void __iomem *dpmreg;
	struct mutex gpu_power_lock;
};
#endif

/**
 * struct kbase_hisi_device_data - all hisi platform data in device level.
 *
 * @callbacks: The callbacks hisi implements.
 * @mgm_dev: The memory_group_manager_device used to alloc/free memory, etc.
 *           We can use this dev to alloc normal memory or last buffer memory.
 * @mgm_ops: The operation interfaces of @mgm_dev.
 * @lb_pools: The info related with device's last_buffer memory pools.
 * @cache_policy_info: The list of cache policy info object.
 * @nr_cache_policy: Number of entries in the list of cache_policy_info.
 */
struct kbase_hisi_device_data {
	struct kbase_hisi_callbacks *callbacks;

	/* Add other device data here */
	/* gpu virtual id */
	u32 gpu_vid;

	struct clk *clock;
#ifdef CONFIG_REGULATOR
	struct regulator *regulator;
#endif

	struct clk *clk;
	void __iomem *crgreg;
	void __iomem *pmctrlreg;
	void __iomem *pctrlreg;

#ifdef CONFIG_MALI_BORR
	void __iomem *sctrlreg;
#endif

#ifdef CONFIG_GPU_GMC_GENERIC
	struct gmc_device kbase_gmc_device;
	struct workqueue_struct *gmc_workqueue;
	int gmc_cancel;
#endif

#ifdef CONFIG_HISI_GPU_DPM_PCR
	struct hisi_gpu_dpm dpm_ctrl;
#endif

	/** List of hi features */
	unsigned long hi_features_mask[2];

	/* refcount for regulator,
	 * should be '1' after call regulator_enable()
	 * and be '0' after call regulator_disable().
	 */
	atomic_t regulator_refcount;

	/* flag to indicate that whether the platform has gpu function module */
	u32 gpu_fpga_exist;
	/* flag to indicate which chip */
	u32 gpu_chip_type;

#ifdef CONFIG_HISI_GPU_SHADER_PRESENT_CFG
	u32 shader_present_lo_cfg;
#endif

	/* record gpu error num:pagefault,gpufault,hardreset */
	struct gpu_error_num error_num;

	struct dsm_client *gpu_dsm_client;
	/* Interface runmode_is_factory() gives runmode is factory mode or not.
	 * factory_mode: false for non-factory mode, true for factory mode
	 * For non-factory mode,DMD will report according to threshold,
	 * factory mode will report every time.
	 */
	bool factory_mode;

	/* unhandled page fault count */
	u32 unhandled_page_fault_count;

	/* procfs entry for gpu_memory */
	struct proc_dir_entry *proc_gpu_memory_dentry;

	struct {
		u8 counter[JOB_SLOT_NUM];
		struct kbase_context *last_two_context_per_slot[JOB_SLOT_NUM][SLOT_RB_SIZE];
	} force_l2_flush;

#ifdef CONFIG_PM_DEVFREQ
	struct devfreq *devfreq;
#ifdef CONFIG_DEVFREQ_THERMAL
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	struct devfreq_cooling_device *devfreq_cooling;
#else
	struct thermal_cooling_device *devfreq_cooling;
#endif
#endif /* CONFIG_DEVFREQ_THERMAL */
#endif /* CONFIG_PM_DEVFREQ */

	/* GPU AI frequency schedule specific pid to collect fence info */
	pid_t game_pid;
	/* GPU AI frequency schedule fence trigger out counter */
	u64 signaled_seqno;

	/* Data about hisi dynamic IPA */
	struct kbase_ipa_context *ipa_ctx;
	struct work_struct bound_detect_work;
	unsigned long bound_detect_freq;
	unsigned long bound_detect_btime;
	bool as_stuck_hard_reset;
	/* GPU Throttle devfreq switch */
	atomic_t thro_enable;
};

/**
 * struct kbase_hisi_ctx_data - all hisi platform data in context level.
 *
 * @lb_pools: The info related with context's last_buffer memory pools.
 *            Contains the last buffer's memory pools with different policy id
 *            and corresponding lock.
 */
struct kbase_hisi_ctx_data {

/* Add other context data here */

	/* BASE_DEBUG_FENCE_TIMEOUT: timer and timer_started */
	struct hrtimer fence_wait_timer;
	int timer_started;

#ifdef CONFIG_GPU_GMC_GENERIC
	bool set_pt_flag;
#endif
};

#ifdef CONFIG_HUAWEI_DSM
#define GPU_JOB_FAIL_DSM_NO      922002000
#define GPU_PAGE_FAULT_DSM_NO    922002001
#define GPU_BIT_STUCK_DSM_NO     922002004
#define GPU_HARD_RESET_DSM_NO    922002005

#define dsm_report_enable(count, threshold, factory_mode) \
	(((count) % (threshold) == (threshold) - 1) || (factory_mode))

#define gpu_dsm_report(kbdev, dsm_no, fmt, args...) \
do { \
	if (kbdev->hisi_dev_data.gpu_dsm_client) { \
		if (!dsm_client_ocuppy(kbdev->hisi_dev_data.gpu_dsm_client)) { \
			dsm_client_record(kbdev->hisi_dev_data.gpu_dsm_client, \
				fmt, ##args); \
			dsm_client_notify(kbdev->hisi_dev_data.gpu_dsm_client, \
				dsm_no); \
			pr_info("[GPU_DSM]gpu dsm report err_no:%d\n", dsm_no); \
		} else \
			pr_err("[GPU_DSM]gpu dsm is busy! err_no:%d\n", dsm_no); \
	} else \
		pr_err("[GPU_DSM]gpu dsm client is NULL!\n"); \
} while (0)
#else
#define gpu_dsm_report(kbdev, dsm_no, fmt, args...)
#endif

#ifdef CONFIG_MALI_DEBUG
#define hisi_uint64_mulcheck(a, b) do { \
	if (check_uint64_multi_overflow((a), (b)) != 0) { \
		pr_err("Failed:Uint64 %lu and %lu multiply overflow!",\
			(a), (b));  \
	} \
} while (0)

static uint32_t check_uint64_multi_overflow(uint64_t a, uint64_t b)
{
	if (a == 0 || b == 0)
		return 0;

	if (a > (UINT64_MAX / b))
		return 1; /* overflow */

	return 0;
}

#define hisi_uint64_addcheck(a, b) do { \
	if (check_uint64_add_overflow((a), (b)) != 0) { \
		pr_err("Failed:Unt64 %lu and %lu addition overflow!",\
			(a), (b));  \
	} \
} while (0)

static uint32_t check_uint64_add_overflow(uint64_t a, uint64_t b)
{
	if (a == 0 || b == 0)
		return 0;

	if (a > (UINT64_MAX - b))
		return 1; /* overflow */

	return 0;
}
#else
#define hisi_uint64_mulcheck(a, b) CSTD_NOP()
#define hisi_uint64_addcheck(a, b) CSTD_NOP()
#endif /* CONFIG_MALI_DEBUG */

#endif /* MALI_KBASE_HISILICON_H */
