/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __MDRV_ACORE_MODULE_INIT_H__
#define __MDRV_ACORE_MODULE_INIT_H__

#ifdef __cplusplus
extern "C"
{
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
extern int bsp_icc_init(void);
extern int socp_logbuffer_mmap(void);
extern int bsp_dump_mem_init(void);
extern int bsp_udi_init(void);
extern int bsp_pm_om_dump_init(void);
extern int hwadp_init(void);
extern int bsp_mem_init_mperf_info(void);
extern int bsp_reset_init(void);
extern int ecdc_genetlink_init(void);
extern int llt_shell_verify_init(void);
extern int socp_init(void);
extern int deflate_init(void);
extern int deflate_dev_init(void);
extern int socp_debug_init(void);
extern int bsp_dump_init(void);
extern int nv_init_dev(void);
extern int hi_socket_init(void);
extern int bsp_version_acore_init(void);
extern int diag_ring_buffer_init(void);
extern int SCM_SoftDecodeCfgRcvTaskInit(void);
extern int PPM_InitPhyPort(void);
extern int CPM_PortAssociateInit(void);
extern int scm_init(void);
extern int diag_vcom_init(void);
extern int bsp_hds_init(void);
extern int bsp_board_trace_init(void);
extern int ipf_pltfm_driver_init(void);
extern int psam_pltfm_driver_init(void);
extern int thermal_up_init(void);
extern int bsp_mem_init(void);
extern int bsp_modem_boot_init(void);
extern int bsp_efuse_agent_init(void);
extern int con_init(void);
extern int virtshl_init(void);
extern int cshell_port_init(void);
extern int uart_dev_init(void);
extern int cshell_logger_init(void);
extern int bsp_pm_om_log_init(void);
extern int modem_log_init(void);
extern int pmic_modem_ocp_init(void);
extern int balong_bbp_acore_init(void);
extern int APP_VCOM_Init(void);
extern int nm_bind_pid_init(void);
extern int NM_CTRL_Init(void);
extern int rnic_init(void);
extern int DMS_NLK_Init(void);
extern int DMS_InitModemStatusFile(void);
extern int DMS_InitGetSliceFile(void);
extern int DMS_InitPorCfgFile(void);
extern int ADS_PlatDevInit(void);
extern int act_cdev_init(void);
extern int mdmcp_coresight_init(void);
extern int his_boot_init(void);
extern int hisi_sim_hotplug_init(void);
extern int bastet_modem_driver_init(void);
extern int his_modem_init_driver(void);
extern int bsp_modem_cold_patch_init(void);
extern int pmic_dcxo_init(void);
/* declare function for drv end */


typedef int (*modem_module_init_func)(void);

static modem_module_init_func g_modem_module_init_func[] = {
    /* DRV begin */

    /* core_initcall */
    sysctrl_init,
    bsp_shared_mem_init,
    bsp_slice_init,
    bsp_load_image_init,

    /* arch_initcall */
    fiq_init,
    hi_timer_init,
    bsp_softtimer_init,
    hi_ipc_init,
    bsp_icc_init,
    bsp_dump_mem_init,
    bsp_udi_init,

    /* arch_initcall_sync */
    bsp_pm_om_dump_init,

    /* subsys_initcall */
    hwadp_init,

    /* module_init */
    bsp_mem_init_mperf_info,
    bsp_reset_init,
#ifdef CONFIG_ECDC
    ecdc_genetlink_init,
    llt_shell_verify_init,
#endif
    socp_init,
    socp_debug_init,
    deflate_init,
    deflate_dev_init,
    bsp_dump_init,
    nv_init_dev,
    bsp_modem_cold_patch_init,
#if (defined(FEATURE_HISOCKET) && (FEATURE_HISOCKET == FEATURE_ON) || (defined(FEATURE_SVLSOCKET)))
    hi_socket_init,
#endif
    bsp_version_acore_init,
#if !defined(DRV_BUILD_SEPARATE)
    diag_ring_buffer_init,
    SCM_SoftDecodeCfgRcvTaskInit,
    PPM_InitPhyPort,
    CPM_PortAssociateInit,
    scm_init,
#ifdef CONFIG_DIAG_NETLINK
    diag_vcom_init,
#endif
#endif
    bsp_hds_init,
#ifdef BOARD_TRACE
    bsp_board_trace_init,
#endif
    ipf_pltfm_driver_init,
    psam_pltfm_driver_init,
    thermal_up_init,
    bsp_mem_init,
    bsp_modem_boot_init,
    bsp_efuse_agent_init,
#ifdef CONFIG_CSHELL
    con_init,
    virtshl_init,
    cshell_port_init,
    uart_dev_init,
    cshell_logger_init,
#endif
    bsp_pm_om_log_init,
    modem_log_init,
    pmic_modem_ocp_init,
    balong_bbp_acore_init,
    bastet_modem_driver_init,
    pmic_dcxo_init,
    /* DRV end */

#if !defined(DRV_BUILD_SEPARATE)
    /* OAM GU begin */

    /* OAM GU end */


    /* OAM TL begin */

    /* OAM TL end */


    /* PS GU begin */

    /* PS GU end */


    /* PS TL begin */

    /* PS TL end */


    /* TAF begin */
    APP_VCOM_Init,
    nm_bind_pid_init,
    NM_CTRL_Init,
    rnic_init,
    DMS_NLK_Init,
    DMS_InitModemStatusFile,
    DMS_InitGetSliceFile,
    DMS_InitPorCfgFile,
    ADS_PlatDevInit,
    /* TAF end */


    /* IMS begin */
    act_cdev_init,
    /* IMS end */
#endif

    /* late_initcall */
    mdmcp_coresight_init,
    his_boot_init,
    hisi_sim_hotplug_init,
    his_modem_init_driver,
};


#ifdef __cplusplus
}
#endif
#endif

