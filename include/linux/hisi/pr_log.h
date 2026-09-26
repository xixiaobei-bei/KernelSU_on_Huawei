/*
 *
 * file of AP macro definition.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __PR_LOG_H__
#define __PR_LOG_H__

#undef  pr_fmt
#define pr_fmt(fmt) "["PR_LOG_TAG"]:" fmt


#define MNTN_DUMP_TAG "mntn_dump"
#define PMIC_TAG "pmic"
#define SUB_PMIC_TAG "sub_pmic"
#define PMIC_MNTN_TAG "pmic_mntn"
#define PMIC_REGULATOR_TAG "pmic_regulator"
#define SUB_PMIC_REGULATOR_TAG "sub_pmic_regulator"
#define PMIC_REGULATOR_DEBUG_TAG "pmic_regulator_debug"
#define SPMI_TAG "spmi"
#define SPMI_DBGFS_TAG "spmi_dbgfs"
#define DMA_TAG "hisi_dma"
#define DMA64_TAG "hisi_dma64"
#define AMBA_PL011_TAG "hisi_amba_pl011"
#define LED_TAG "hisi_led"
#define POWERKEY_TAG "hisi_powerkey"
#define MNTN_BBOX_DIAGINFO  "bbox_diaginfo"
#define NOC_TRACE_TAG "noc_trace"
#define STM_TRACE_TAG "stm_trace"
#define AXI_TAG "dfx_axi"
#define MEMORY_DUMP_TAG "memory_dump"
#define BLACKBOX_TAG "blackbox"
#define BOOTTIME_TAG "boottime"
#define CODE_PROTECT_TAG "code_protect"
#define DUMP_TAG "dump"
#define FASTBOOTLOG_TAG "fastbootlog"
#define BOOTUP_KEYPOINT_TAG "dfx_bootup_keypoint"
#define FIQ_TAG "dfx_fiq"
#define POWEROFF_TAG "dfx_poweroff"
#define FILESYS_TAG "dfx_filesys"
#define MNTN_L3CACHE_ECC_TAG "mntn_l3cache_ecc"
#define UTIL_TAG "util"
#define VIRT_TO_PHYS_TAG "virt_to_phys"
#define NOC_TAG "dfx_noc"
#define QIC_TAG "dfx_qic"
#define SP805_WDT_TAG "dfx_sp805_wdt"
#define HI_V500_WDT_TAG "hi_v500_wdt"
#define MNTN_TEST_TAG "dfx_mntn_test"
#define EASY_SHELL_TAG "dfx_easy_shell"
#define MNTN_BC_PANIC_TAG "mntn_bc_panic"
#define MNTN_RECORD_SP_TAG "mntn_record_sp"
#define EAGLE_EYE_TAG "eagle_eye"
#define GPIO_TAG "gpio"
#define PINCTRL_TAG "pinctrl"
#define HWSPINLOCK_TAG "hwspinlock"
#define HWSPINLOCK_DEBUGFS_TAG "hwspinlock_debugfs"

#define AP_MAILBOX_TAG "ap_ipc"


#endif
