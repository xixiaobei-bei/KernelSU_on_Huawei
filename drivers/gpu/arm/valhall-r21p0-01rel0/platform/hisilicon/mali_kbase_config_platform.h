/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: This file describe HISI GPU platform related configuration
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

#ifndef MALI_KBASE_CONFIG_PLATFORM_H
#define MALI_KBASE_CONFIG_PLATFORM_H

/**
 * Maximum frequency GPU will be clocked at. Given in kHz.
 * This must be specified as there is no default value.
 *
 * Attached value: number in kHz
 * Default value: NA
 */
#define GPU_FREQ_KHZ_MAX 5000
/**
 * Minimum frequency GPU will be clocked at. Given in kHz.
 * This must be specified as there is no default value.
 *
 * Attached value: number in kHz
 * Default value: NA
 */
#define GPU_FREQ_KHZ_MIN 5000

#define POWER_MANAGEMENT_CALLBACKS (&pm_callbacks)

#define KBASE_PLATFORM_CALLBACKS ((uintptr_t)&platform_funcs)

#ifdef CONFIG_PM_DEVFREQ
#define POWER_MODEL_CALLBACKS ((uintptr_t)&hisi_model_ops)
#endif

#define GPU_SPEED_FUNC (NULL)

#define CPU_SPEED_FUNC (&kbase_cpuprops_get_default_clock_speed)

#define PLATFORM_FUNCS (KBASE_PLATFORM_CALLBACKS)

/**
 * @brief Tell whether a feature should be enabled
 */
#define kbase_has_hi_feature(kbdev, hi_feature)\
	test_bit(hi_feature, &((kbdev)->hisi_dev_data.hi_features_mask[0]))

/*
 * @brif Define external module base address
 */
#if defined (CONFIG_MALI_NORR_PHX)
#define SYS_REG_PMCTRL_BASE_ADDR           0xFFF01000
#define SYS_REG_PERICRG_BASE_ADDR          0xFFF05000
#define SYS_REG_PERICTRL_BASE_ADDR         0xFE02E000
#define SYS_REG_PERICTRL_BASE_ADDR_CS2     0xFFA2E000
#elif defined (CONFIG_MALI_TRYM) || defined (CONFIG_MALI_BORR) || defined (CONFIG_MALI_NATT)
#define SYS_REG_PMCTRL_BASE_ADDR           0xFFF01000
#define SYS_REG_PERICRG_BASE_ADDR          0xFFF05000
#define SYS_REG_PERICTRL_BASE_ADDR         0xFEC3E000
#define SYS_REG_SYSCTRL_BASE_ADDR          0xFA89B000
#ifdef CONFIG_HISI_GPU_DPM_PCR
#define SYS_REG_GPUPCR_BASE_ADDR           0xFEC30000
#endif
#else
#define SYS_REG_PMCTRL_BASE_ADDR           0xFFF31000
#define SYS_REG_PERICRG_BASE_ADDR          0xFFF35000
#define SYS_REG_PERICTRL_BASE_ADDR         0xE8A09000
#endif

/*
 * @brief Special register for fpga
 */
#ifdef CONFIG_MALI_BORR
#define MASK_BALTIMORE_FPGA_B010           0x36A05010 /* b010 logic*/
#define MASK_BALTIMORE_FPGA_B020           0x36A05020 /* b020 logic*/
#define MASK_BALTIMORE_FPGA_B021           0x36A05021 /* b021 logic*/
#define SYSCTRL_SOC_ID0_OFFSET             0xE00      /* fpga: soc id register offset */
#endif

/*
 * define some registers offset value
 */
#define SYS_REG_REMAP_SIZE                 0x1000
#define DPM_PCR_REMAP_SIZE                 0x4000

#define KBASE_PWR_KEY_VALUE                0x2968a819
#define KBASE_PWR_OVERRIDE_VALUE           0xc4b00960

/* mem low power feature configuration */
#if defined (CONFIG_MALI_BORR)
#define G3D_MEM_DSLP_ENABLE                0x01FFFFFF
#define G3D_MEM_DSLP_DISABLE               0xFE000000
#elif defined (CONFIG_MALI_TRYM)
/* bit[8:0]:Configuration 1 indicates that G3D enters DSLP mode */
#define G3D_MEM_DSLP_ENABLE                0x1FF
#define G3D_MEM_DSLP_DISABLE               0xFFFFFE00
#define G3D_MEM_AUTO_SD_ENABLE             0x2
#elif defined (CONFIG_MALI_NATT)
/* bit[4:0]:Configuration 1 indicates that G3D enters DSLP mode */
#define G3D_MEM_DSLP_ENABLE                0x1F
#define G3D_MEM_DSLP_DISABLE               0xFFFFFFE0
#define G3D_MEM_AUTO_SD_ENABLE             0x2
#endif

#define PERICTRL_19_OFFSET                 0x050
#define PERICTRL_21_OFFSET                 0x058
#define PERICTRL_92_OFFSET                 0x230
#define PERICTRL_93_OFFSET                 0x234

#define PERICTRL_93_MARSK                  0x70000

#if defined (CONFIG_MALI_GONDUL)
#define PERI_STAT_FPGA_GPU_EXIST           0xC4
#define PERI_STAT_FPGA_GPU_EXIST_MASK      0x4
#elif defined (CONFIG_MALI_NORR_PHX)
#define PERI_STAT_FPGA_GPU_EXIST           0xBC
#define PERI_STAT_FPGA_GPU_EXIST_MASK      0x40
#elif defined (CONFIG_MALI_BORR)
#define PERI_STAT_FPGA_GPU_EXIST           0xBC
#define PERI_STAT_FPGA_GPU_EXIST_MASK      0x1
#elif defined (CONFIG_MALI_TRYM) || defined (CONFIG_MALI_NATT)
#define PERI_STAT_FPGA_GPU_EXIST           0xBC
#define PERI_STAT_FPGA_GPU_EXIST_MASK      0x20
#else
#define PERI_STAT_FPGA_GPU_EXIST           0xBC
#define PERI_STAT_FPGA_GPU_EXIST_MASK      0x1
#endif

#if defined (CONFIG_MALI_MIMIR) || defined (CONFIG_MALI_SIGURD)
#define G3DAUTOCLKDIVBYPASS                0x268
#define VS_CTRL_2                          0x46c
#elif defined (CONFIG_MALI_HEIMDALL)
#define G3DAUTOCLKDIVBYPASS                0x268
#define VS_CTRL_2                          0x418
#elif defined (CONFIG_MALI_NORR)
#define G3DAUTOCLKDIVBYPASS                0x258
#define VS_CTRL_2                          0x448
#elif defined (CONFIG_MALI_NORR_PHX) || defined (CONFIG_MALI_GONDUL) || defined (CONFIG_MALI_TRYM) || defined (CONFIG_MALI_NATT)
#define G3DAUTOCLKDIVBYPASS                0x1D8
#define VS_CTRL_2                          0x448
#elif defined (CONFIG_MALI_BORR)
#define G3DAUTOCLKDIVBYPASS                0x258
#define VS_CTRL_2                          0x448
#endif
/* for cs */
#define G3DAUTOCLKDIVBYPASS_2              0x248
#define VS_CTRL_GPU                        0x448

#if defined(CONFIG_MALI_NORR_PHX)
/* set [1]bit to 1, Hardware auto shutdown */
#define MASK_AUTOSDBYHW                    0x2
#define MASK_ENABLEDSBYSF                  0x1FFFF
#define MASK_DISABLEDSBYSF                 0xFFFE0000
/* Hardware auto shutdown */
#define HW_AUTO_SHUTDOWN                   PERICTRL_93_OFFSET
/* deep sleep by software */
#define DEEP_SLEEP_BYSW                    PERICTRL_92_OFFSET
#elif defined(CONFIG_MALI_GONDUL)
/* set [1]bit to 1, Hardware auto shutdown */
#define MASK_AUTOSDBYHW                    0x2
/* gondul'PERI_CTRL92,6 regional ds respectively */
#define MASK_ENABLEDSBYSF                  0x7F
/* gondul'PERI_CTRL92,6 regional ds respectively */
#define MASK_DISABLEDSBYSF                 0xFFFFFF80
#define PERI_CTRL91                        0x22C
/* Hardware auto shutdown */
#define HW_AUTO_SHUTDOWN                   PERICTRL_91_OFFSET
/* deep sleep by software */
#define DEEP_SLEEP_BYSW                    PERICTRL_92_OFFSET
#else
#define MASK_AUTOSDBYHW                    0x80000000
#define MASK_ENABLEDSBYSF                  0x3FFFE
#define MASK_DISABLEDSBYSF                 0xFFFC0001
/* Hardware auto shutdown */
#define HW_AUTO_SHUTDOWN                   PERICTRL_93_OFFSET
/* deep sleep by software */
#define DEEP_SLEEP_BYSW                    PERICTRL_93_OFFSET
#endif

#define HASH_STRIPING_GRANULE              0xE00

#define MASK_ENABLESDBYSF                  0x1FFFF
#define MASK_DISABLESDBYSF                 0xFFFE0000

#define MASK_G3DHPMBYPASS                  0xFFF0FFF0
#define MASK_G3DAUTOCLKDIVBYPASS           0xFFFFFFFE

#define GPU_X2P_GATOR_BYPASS               0xFEFFFFFF

#define SYS_REG_CRG_CLOCK_EN               0x38
#define SYS_REG_CRG_CLCOK_STATUS           0x3C
#define SYS_REG_CRG_G3D                    0x84
#define SYS_REG_CRG_G3D_EN                 0x88
#define SYS_REG_CRG_RESET_STATUS           0x8C
#define SYS_REG_CRG_ISO_STATUS             0x14C

#define KBASE_PWR_RESET_VALUE              0x007C001C
#define KBASE_PWR_ACTIVE_BIT               0x2
#define KBASE_PWR_INACTIVE_MAX_LOOPS       100000

#define SYS_REG_CRG_W_CLOCK_EN             0x30
#define SYS_REG_CRG_W_CLOCK_CLOSE          0x34
#define SYS_REG_CRG_CLK_DIV_MASK_EN        0xF0

#define GPU_CRG_CLOCK_VALUE                0x00000038
#define GPU_CRG_CLOCK_POWER_OFF_MASK       0x00010000
#define GPU_CRG_CLOCK_POWER_ON_MASK        0x00010001

extern struct kbase_pm_callback_conf pm_callbacks;
extern struct kbase_platform_funcs_conf platform_funcs;

#endif /* MALI_KBASE_CONFIG_PLATFORM_H */
