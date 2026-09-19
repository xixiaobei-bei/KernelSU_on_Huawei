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

#ifndef __BSP_PRINT_H__
#define __BSP_PRINT_H__

#ifdef __cplusplus
    extern "C" {
#endif
#include <product_config.h>
#include "drv_comm.h"
#include "bsp_trace.h"
/*****************************
* 宏定义
*****************************/
#undef MOD
#define MOD(x) #x

#undef BSP_MOD
#if (defined(BSP_CONFIG_PHONE_TYPE)) && (defined(__KERNEL__))
#define BSP_MOD(x) (MOD(x))
#else
#define BSP_MOD(x) ((MOD(x)) + 4)
#endif



/*****************************
* 枚举定义
* BSP_PRINT_OFF: 表示打印关闭
* BSP_P_FATAL的打印级别最高
*****************************/
typedef enum{
	BSP_PRINT_OFF = 0,
	BSP_P_FATAL = 1,
	BSP_P_ERR,
	BSP_P_WRN,
	BSP_P_INFO,
	BSP_P_DEBUG,

	BSP_LEVEL_SUM
}BSP_LOG_LEVEL;

/*module_tag:模块名，格式必须为 mod_xxx*/
typedef enum _module_tag
{
	/*mdrv_1 start*/
	mod_amon,
	mod_mid,
	mod_noc,
	mod_dmss,
	mod_rdr,
	mod_adump,
	mod_mdump,
	mod_watchpoint,
	mod_coresight,
	mod_print,
	mod_hds,
	mod_pdlock,
	mod_dlock,
	mod_sci,
	mod_dump,
	mod_diag,
	mod_diag_sys,
	mod_diag_dbg,
	mod_diag_port,
	mod_scm,
	mod_ppm,
	mod_soft_dec,
	mod_iqi,
	mod_socp,
    mod_diag_vcom,
	mod_version,
	mod_nv,
	mod_chr,
	mod_deflate,
	mod_dsp,
	mod_dspdfs,
	mod_dspdvs,
	mod_nrdsp,
	mod_mailbox,
	mod_xmailbox,
	mod_dsp_dvfs,
	mod_cpm,
	mod_nrcpm,
	mod_simhotplug,
	/*mdrv_1 end*/
	/*mdrv_2 start*/
	mod_emi,
	mod_security,
	mod_pmu_coul,
	mod_mtd,
	mod_fmc,
	mod_anten,
	mod_rffe,
	mod_abb,
	mod_memrepair,
	mod_mipi,
	mod_power,
	mod_bbp,
	mod_gpio,
	mod_pintrl,
	mod_mmc,
	mod_mloader,
	mod_pinctrl,
	mod_adc,
	mod_yaffs,
	mod_ubifs,
	mod_hkadc,
	mod_temperature,
	mod_easyrf,
	mod_loadm,
	mod_efuse,
	mod_rfic_load,
    mod_lcd,
	mod_leds,
	mod_rtc,
	mod_tsensor,
	mod_rfile,
	mod_hifi,
	mod_onoff,
	mod_sec,
	mod_zstd,
	mod_load_ps,
	mod_ios,
    mod_key,
	mod_i2c,
	mod_spi,
	mod_spi_slave,
	mod_load_fpga,
	/*mdrv_2 end*/
	/*mdrv_3 start*/
        mod_fiq,
        mod_thermal,
        mod_init,
        mod_l2cache,
        mod_osal,
        mod_osl,
	mod_cpufreq,
	mod_busfreq,
	mod_hardtimer,
	mod_systimer,
	mod_tickless,
	mod_cpuidle,
	mod_archpm,
	mod_pm_om,
	mod_avs,
	mod_sys_bus,
	mod_sysctrl,
	mod_wakelock,
	mod_wdt,
	mod_clk,
	mod_dpm,
	mod_of,
	mod_edma,
	mod_perf_stat,
	mod_mperf,
	mod_pm,
	mod_hotplug,
	mod_hwadp,
	mod_malloc_m,
	mod_mem,
	mod_pmu,
	mod_regulator_m,
	mod_reset_m,
	mod_rsracc,
	mod_softtimer,
	mod_sync,
	mod_s_mem,
	mod_vic,
	/*mdrv_3 end*/
	/*mdrv_4 start*/
	mod_ipf,
	mod_psam,
	mod_wan,
	mod_pss,
	mod_maa,
	mod_ipsec,
	mod_pcdev,
	mod_pcie_nic,
	mod_espe,
	mod_xlat,
	mod_spe_v7,
	mod_hisocket,
	mod_ecipher,
	mod_hcode,
	mod_cipher,
	mod_udi,
	mod_usb,
	mod_gmac,
	/*mdrv_4 end*/
	/*mdrv_5 start*/
	mod_ipc,
	mod_ipc_msg,
	mod_icc,
	mod_eicc,
	mod_dfc,
	mod_at_uart,
	mod_console,
	mod_cshell,
	mod_dynmem_rpt,
	mod_llt_tool,
	mod_llt_load,
	mod_ecdc,
	mod_board_trace,
	mod_sec_call,
	mod_reset,
	/*mdrv_5 end*/
	mod_all,/* 代表所有的模块 */
	MODU_MAX = 256,    /* 边界值 */


} module_tag;

/*****************************
* 结构体定义
*****************************/
typedef struct //各模块设置默认打印级别
{
	unsigned int con_level;
	unsigned int logbuf_level;
}bsp_syslevel_ctrl;

typedef struct
{
	unsigned int modlevel;
}bsp_print_tag;

/*****************************
* 函数声明
*****************************/
#ifdef ENABLE_BUILD_OM
void bsp_print(module_tag modid, BSP_LOG_LEVEL level, char *fmt,...);
/*
logl设置HIDS和控制台的系统默认打印级别
logm设置模块打印级别
logc查询打印级别
*/
int logs(unsigned int console, unsigned int logbuf);
int logm(unsigned int modid, unsigned int level);
void logc(unsigned int modid);
#else
void bsp_print(module_tag modid, BSP_LOG_LEVEL level, char *fmt,...){return ;}
int logs(unsigned int console, unsigned int logbuf){return 0;}
int logm(unsigned int modid, unsigned int level){return 0;}
void logc(unsigned int modid){return ;}
#endif

//如果认为是比较重要的流程，比如初始化流程，可以在err级别打印;默认打印info级别；如果是在低功耗或者是开机log，我们不往控制台中输入，只往socp中送
#define bsp_fatal(fmt, ...) (bsp_print(THIS_MODU, BSP_P_FATAL, "[%s]:"fmt, BSP_MOD(THIS_MODU), ##__VA_ARGS__))
#define bsp_err(fmt, ...)   (bsp_print(THIS_MODU, BSP_P_ERR,   "[%s]:"fmt, BSP_MOD(THIS_MODU),##__VA_ARGS__))
#define bsp_wrn(fmt, ...)   (bsp_print(THIS_MODU, BSP_P_WRN,   "[%s]:"fmt, BSP_MOD(THIS_MODU),##__VA_ARGS__))
#define bsp_info(fmt, ...)  (bsp_print(THIS_MODU, BSP_P_INFO,  "[%s]:"fmt, BSP_MOD(THIS_MODU),##__VA_ARGS__))
#define bsp_debug(fmt, ...) (bsp_print(THIS_MODU, BSP_P_DEBUG, "[%s]:"fmt, BSP_MOD(THIS_MODU),##__VA_ARGS__))
unsigned int bsp_get_print_status(void);

#ifdef __cplusplus
}
#endif

#endif
