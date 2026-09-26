/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * @brief   module_init模块在soc平台acore上底软内部头文件
 * @file    bsp_module_init_soc_modem.h
 * @version v1.0
 * @date    2020.11.20
 * <ul><li>v1.0|2020.11.20|创建文件</li></ul>
 * @since
 */
#ifndef __BSP_MODULE_INIT_SOC_MODEM_H__
#define __BSP_MODULE_INIT_SOC_MODEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* declare function for drv begin */
extern int bsp_load_image_init(void);
extern int sysctrl_init(void);
extern int bsp_shared_mem_init(void);
extern int bsp_slice_init(void);
extern int bsp_mem_init_mperf_info(void);
extern int fiq_init(void);
extern int hi_timer_init(void);
extern int bsp_softtimer_init(void);
extern int hi_ipc_init(void);
extern int bsp_ipc_init(void);
extern int bsp_icc_init(void);
extern int bsp_eicc_init(void);
extern int socp_logbuffer_mmap(void);
extern int bsp_dump_mem_init(void);
extern int bsp_udi_init(void);
extern int bsp_pm_om_dump_init(void);
extern int hwadp_init(void);
extern int bsp_mem_init_mperf_info(void);
extern int bsp_reset_init(void);
#ifdef ECDC_INIT_ENABLE
extern int ecdc_genetlink_init(void);
extern int llt_shell_verify_init(void);
#endif
extern int socp_init(void);
extern int deflate_init(void);
extern int bsp_dump_init(void);
extern int mmc_init(void);
extern int nv_init_dev(void);
extern int hi_socket_init(void);
extern int bsp_version_acore_init(void);
extern int soft_decode_init(void);
extern int ppm_init_phy_port(void);
extern int cpm_port_associate_init(void);
extern int scm_init(void);
extern int diag_vcom_init(void);
extern int bsp_hds_init(void);
extern int bsp_board_trace_init(void);
extern int bsp_trng_seed_init(void);
extern int bsp_dump_resave_modem_log(void);

#if (defined(CONFIG_MODEM_PINTRL) && defined(CONFIG_MODEM_PINTRL_KERNEL))
extern int bsp_modem_pinctrl_dts_parse(void);
#endif

#if defined(CONFIG_KERNEL_DTS_DECOUPLED)
extern int bsp_dt_init(void);
#endif

extern int print_early_log_init(void);

#ifdef CONFIG_AVS_TEST
extern int avs_test_init(void);
#endif

#if (defined(CONFIG_IPF) || defined(CONFIG_EIPF))
extern int ipf_pltfm_driver_init(void);
#endif
#ifdef CONFIG_PSAM
extern int psam_pltfm_driver_init(void);
#endif
#ifdef CONFIG_WAN
extern int wan_pltfm_driver_init(void);
#endif
extern int bsp_mem_init(void);
extern int bsp_modem_boot_init(void);
extern int bsp_efuse_agent_init(void);
extern int con_init(void);
extern int virtshl_init(void);
extern int cshell_port_init(void);
extern int nr_cshell_port_init(void);
extern int uart_dev_init(void);
extern int cshell_logger_init(void);
extern int bsp_pm_om_log_init(void);
extern int modem_log_init(void);
extern int pmic_modem_ocp_init(void);
extern int balong_bbp_acore_init(void);
extern int mdmcp_coresight_init(void);
extern int his_power_on_init(void);
extern int hisi_sim_hw_service_init(void);
extern int bastet_modem_driver_init(void);
extern int his_modem_init_driver(void);
extern int bsp_modem_cold_patch_init(void);
extern int pmic_dcxo_init(void);
extern int diag_init(void);

#ifdef BSP_CONFIG_PHONE_TYPE
extern int diag_mdmlog_nve_init(void);
#endif

#ifdef CONFIG_MAA_BALONG
extern int maa_init(void);
#endif
extern int aximem_init(void);
extern int fiq_5g_init(void);
#ifdef CONFIG_NMI
extern int nmi_init(void);
#endif
extern int bsp_malloc_init(void);
#ifdef CONFIG_NRDSP
extern s32 bsp_nrdsp_dump_tcm_init(void);
#endif
extern int mloader_driver_early_init(void);
extern int mloader_driver_init(void);
extern int lpmcu_modem_init(void);
extern int bsp_espe_module_init(void);
extern int spe_filter_init(void);
extern int trans_report_init(void);
#ifdef CONFIG_MLOADER_HOT_PATCH
extern int bsp_modem_hot_patch_b_init(void);
#endif
#ifdef DIGITAL_POWER_MONITOR
#ifndef CONFIG_HISI_MODEM_OHOS
extern int bsp_power_monitor_init(void);
#endif
#endif

extern int bsp_memory_init(void);
extern int bsp_memory_layout_init(void);
extern int nmi_init(void);
extern int bsp_msg_init(void);
extern int bbpds_init(void);
extern int ccpudebug_init(void);
extern int sysboot_load_modem_teeos(void);
#ifdef FEATURE_NVA_ON
extern int nva_init(void);
#endif

#ifdef CONFIG_HMI
extern int hmi_init(void);
#endif
/* declare function for drv end */
/*
 * @brief modem初始化函数指针类型
 */
typedef int (*modem_drv_module_init_func)(void);

/*
 * @brief modem初始化函数列表
 */
static modem_drv_module_init_func g_modem_drv_module_init_func[] = {
/* pure_initcall */
#ifdef BALONG_MODEM_TEEOS_MODULE
    sysboot_load_modem_teeos,
#endif
#if defined(CONFIG_KERNEL_DTS_DECOUPLED)
    bsp_dt_init,
#endif
#ifdef FEATURE_NVA_ON
    nva_init,
#endif
#ifdef CONFIG_SHARED_MEMORY
    bsp_memory_init,
#endif
#ifdef CONFIG_MEMORY_LAYOUT
    bsp_memory_layout_init,
#endif
    /* core_initcall */
    bsp_dump_resave_modem_log,
    sysctrl_init,
    bsp_shared_mem_init,
    bsp_slice_init,
#ifdef CONFIG_AXIMEM_BALONG
    aximem_init,
#endif
    bsp_dump_mem_init,
#ifdef CONFIG_MODULE_IPC
    hi_ipc_init,
#endif
#ifdef CONFIG_MODULE_IPC_FUSION
    bsp_ipc_init,
#endif
    mloader_driver_early_init,
    lpmcu_modem_init,
#ifdef CONFIG_MAA_BALONG
    maa_init,
#endif
/* arch_initcall */
#ifdef CONFIG_NMI
    nmi_init,
#else
    fiq_init,
#ifdef CONFIG_HAS_NRCCPU
    fiq_5g_init,
#endif
#endif
    hi_timer_init,
    bsp_softtimer_init,
#ifdef CONFIG_ICC
    bsp_icc_init,
#endif
#ifdef CONFIG_EICC_V200
    bsp_eicc_init,
#endif
    bsp_udi_init,

    /* arch_initcall_sync */
#ifdef CONFIG_BALONG_MSG
    bsp_msg_init,
#endif
#ifdef CONFIG_PM_OM_BALONG
    bsp_pm_om_dump_init,
#endif
    /* subsys_initcall */
    hwadp_init,
#ifdef CONFIG_AVS_TEST
    avs_test_init,
#endif
#ifdef CONFIG_MALLOC_UNIFIED
    bsp_malloc_init,
#endif
    /* fs_initcall */
    mmc_init,

    /* module_init */
    bsp_mem_init_mperf_info,
#ifdef CONFIG_BALONG_MODEM_RESET
    bsp_reset_init,
#endif
#ifdef ECDC_INIT_ENABLE
    ecdc_genetlink_init,
    llt_shell_verify_init,
#endif
    socp_init,

#ifdef CONFIG_DEFLATE
    deflate_init,
#endif
#ifdef DIAG_SYSTEM_FUSION
    bbpds_init,
#endif
#ifdef CONFIG_NRDSP
    bsp_nrdsp_dump_tcm_init,
#endif

    bsp_dump_init,
    bsp_modem_cold_patch_init,
#ifdef CONFIG_MLOADER_HOT_PATCH
    bsp_modem_hot_patch_b_init,
#endif
#if (defined(FEATURE_HISOCKET) && (FEATURE_HISOCKET == FEATURE_ON) || (defined(FEATURE_SVLSOCKET)))
    hi_socket_init,
#endif
    bsp_version_acore_init,

    soft_decode_init,
    diag_init,
    diag_mdmlog_nve_init,
#ifdef CONFIG_DIAG_NETLINK
    diag_vcom_init,
#endif

    bsp_hds_init,
#ifdef CONFIG_BALONG_ESPE
    bsp_espe_module_init,
    spe_filter_init,
#endif
#ifdef BOARD_TRACE
    bsp_board_trace_init,
#endif
#if (defined(CONFIG_IPF) || defined(CONFIG_EIPF))
    ipf_pltfm_driver_init,
#endif
#ifdef CONFIG_PSAM
    psam_pltfm_driver_init,
#endif
#ifdef CONFIG_WAN
    wan_pltfm_driver_init,
#endif
#ifdef CONFIG_MEM_BALONG
    bsp_mem_init,
#endif
    bsp_modem_boot_init,
#ifdef CONFIG_EFUSE
#ifdef CONFIG_EFUSE_BALONG_AGENT
    bsp_efuse_agent_init,
#endif
#endif
#ifdef CONFIG_CSHELL
    con_init,
    virtshl_init,
    cshell_port_init,
#ifdef CONFIG_NR_CSHELL
    nr_cshell_port_init,
#endif
    uart_dev_init,
    cshell_logger_init,
#endif
    bsp_pm_om_log_init,
#ifdef CONFIG_PM_OM_BALONG
    modem_log_init,
#endif
#ifdef PMIC_OCP
    pmic_modem_ocp_init,
#endif
#ifdef CONFIG_BALONG_TRANS_REPORT
    trans_report_init,
#endif
#ifdef CONFIG_BBP_ACORE
    balong_bbp_acore_init,
#endif
#if !defined(DRV_BUILD_SEPARATE)
#if (FEATURE_BASTET == FEATURE_ON)
    bastet_modem_driver_init,
#endif
#endif
#ifdef CONFIG_PHONE_DCXO_AP
    pmic_dcxo_init,
#endif
#ifdef CONFIG_TRNG_SEED
    bsp_trng_seed_init,
#endif
#ifdef DIGITAL_POWER_MONITOR
#ifndef CONFIG_HISI_MODEM_OHOS
    bsp_power_monitor_init,
#endif
#endif
};

static modem_drv_module_init_func g_modem_drv_late_init_func[] = {
    /* late_initcall */
#ifdef CONFIG_CORESIGHT
    mdmcp_coresight_init,
#endif
    his_power_on_init,
#ifdef CONFIG_CCPUDEBUG
    ccpudebug_init,
#endif
#ifdef CONFIG_HISI_SIM_HOTPLUG_SPMI
    hisi_sim_hw_service_init,
#endif

#if (defined(CONFIG_MODEM_PINTRL) && defined(CONFIG_MODEM_PINTRL_KERNEL))
    bsp_modem_pinctrl_dts_parse,
#endif

    mloader_driver_init,
    print_early_log_init,
#ifdef CONFIG_HMI
    hmi_init,
#endif
};


#ifdef __cplusplus
}
#endif
#endif
