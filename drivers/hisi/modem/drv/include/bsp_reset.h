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

#ifndef __BSP_RESET_H__
#define __BSP_RESET_H__

#include <osl_common.h>
#include <bsp_memmap.h>
#include <mdrv_sysboot.h>
#include <bsp_shared_ddr.h>
#include <bsp_slice.h>
#include <bsp_sysctrl.h>

#define RESET_OK (0)
#define RESET_ERROR (-1)
#undef RESET_LOAD_MODEM_IMG_USE_SECOS

#define RESET_STAGE_GET(reset_info) ((reset_info)&0xff)
#define RESET_ACTION_GET(reset_info) (((reset_info) >> 16) & 0xff)
#define RESET_INFO_MAKEUP(action, stage) ((action << 16) | (stage))

enum { MDM_RESET_UNSUPPORT = 0, MDM_RESET_SUPPORT = 1 };

enum RESET_SYSFS_VALUE {
    BALONG_MODEM_OFF = 2,
    BALONG_MODEM_ON = 6,
    BALONG_MODEM_RESET = 10,
    BALONG_MODEM_RILD_SYS_ERR = 11,
    BALONG_MODEM_3RD_SYS_ERR = 12
};

enum MODEM_RESET_BOOT_MODE { CCORE_BOOT_NORMAL = 0, CCORE_IS_REBOOT = 0x87654321UL };

enum DRV_RESET_CB_PIOR {
    DRV_RESET_CB_PIOR_MIN = 0,
    DRV_RESET_CB_PIOR_ICC = 0,
    DRV_RESET_CB_PIOR_ESPE = 3,
    DRV_RESET_CB_PIOR_MAA = 4,
    DRV_RESET_CB_PIOR_IPF = 5,
    DRV_RESET_CB_PIOR_IQI = 6,
    // RESET_CALLCBFUN_PIOR_<组件名>
    // ...
    DRV_RESET_CB_PIOR_CSHELL = 16,
    DRV_RESET_CB_PIOR_RFILE,
    DRV_RESET_CB_PIOR_PCIE_PM,
    DRV_RESET_CB_PIOR_DIAG = 49,
    DRV_RESET_CB_PIOR_DUMPLPM3 = 50,
    DRV_RESET_CB_PIOR_ALL = 51,
    DRV_RESET_CB_PIOR_MAX
};

enum MODEM_ACTION { MODEM_POWER_OFF, MODEM_POWER_ON, MODEM_RESET, MODEM_NORMAL, INVALID_MODEM_ACTION };

/* M核和C核之间核间消息约定: 复位阶段信息复用DRV_RESET_CALLCBFUN_MOMENT枚举定义 */
enum RESET_ICC_MSG {
    RESET_MCORE_BEFORE_RESET_OK,
    RESET_MCORE_RESETING_OK,
    RESET_MCORE_AFTER_RESET_OK,
    RESET_MCORE_BEFORE_AP_TO_MDM_BUS_ERR,
    RESET_MCORE_BEFORE_MDM_TO_AP_BUS_ERR,
    RESET_MCORE_BEFORE_MODEM_ACCESS_DDRC_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEIN_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEINACK_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEOUT_ERR,
    RESET_MCORE_BEFORE_NOC_POWER_IDLEOUTACK_ERR,
    RESET_CCORE_REBOOT_OK,
    RESET_ACORE_HOST_IDLE_REQ,
    RESET_CCORE_HOST_IN_IDLE,
    RESET_INNER_CORE_INVALID_MSG,
};

/* 标识可否向对方核发送核间消息 */
enum RESET_MULTICORE_CHANNEL_STATUS {
    CCORE_STATUS = 0x00000001,
    MCORE_STATUS = 0x00000002,
    HIFI_STATUS = 0x00000004,
    INVALID_STATUS = 0x10000000
};

enum RESET_CBFUNC_PRIO { RESET_CBFUNC_PRIO_LEVEL_LOWT = 0, RESET_CBFUNC_PRIO_LEVEL_HIGH = 51 };

/**
 * 定义用户注册的host进idle子模块枚举类型。
 */
typedef enum bsp_host_idle {
    HWSPINLOCK_IDLE = 0x0,
    RSRACC_IDLE,
    BBE16_IDLE,
    CBBE16_IDLE,
    CIPHER_IDLE,
    EDMAC_IDLE,
    UPACC_IDLE,
    CICOM_IDLE,
    HDLC_IDLE,
    IPF_IDLE,
    BBP_RESET_IDLE,
    BBE16_SYSBUS,
    CBBE16_SYSBUS,
    SOCP_MODEM_CHAN_IDLE,
    NV_IDLE,
    GCOV_IDLE,
    SIM_IDLE,
    SEC_DUMP,
    MDM_CERT,
    HOST_IDLE_SUM
} drv_host_idle_e;

struct inter_multicore_msg {
    u32 reset_stage;
    u32 modem_action;
};

#define ccore_msg_switch_on(flag, status) \
    do {                                  \
        (flag) = (flag) | (status);       \
    } while (0)

#define ccore_msg_switch_off(flag, status) \
    do {                                   \
        (flag) = (flag) & (~(status));     \
    } while (0)

/**
 * 定义用户注册的host进idle钩子函数类型。
 */
typedef int (*host_idle_hook_func)(void);

#if defined(CONFIG_BALONG_MODEM_RESET) || defined(CONFIG_BALONG_MODEM_GLOBAL_RESET) || \
    defined(CONFIG_BALONG_MODEM_RESET_CTRL)

uintptr_t get_scbakdata13(void);
#define bsp_reset_bootflag_set(value)                                  \
    do {                                                               \
        writel((value), (volatile void*)(uintptr_t)get_scbakdata13()); \
    } while (0)

static inline u32 bsp_reset_ccore_is_reboot(void)
{
#ifdef __KERNEL__
    return (u32)((u32)CCORE_IS_REBOOT == (readl((const volatile void*)get_scbakdata13())) ? 1 : 0);
#else
    return (u32)((u32)CCORE_IS_REBOOT == (readl((void*)get_scbakdata13())) ? 1UL : 0UL);
#endif
}

/**
 *
 * @brief 向CP单独复位主流程的钩子注册函数
 *
 * @param  host_idle  [IN] 类型#host_idle_hook_func，注册的钩子函数。
 *
 * @retval  #BSP_OK  0，成功。#BSP_ERROR  0，失败。
 *
 * @par 依赖:bsp_reset.h：该接口声明所在的头文件。
 *
 */
int bsp_register_host_idle(drv_host_idle_e type, host_idle_hook_func host_idle);

u32 bsp_reset_is_successful(u32 timeout_ms);

#else /* !CONFIG_BALONG_MODEM_RESET */

#define bsp_reset_bootflag_set(value) ((value) = (value))
static inline u32 bsp_reset_ccore_is_reboot(void)
{
    return 0;
}

static inline int bsp_register_host_idle(drv_host_idle_e type, host_idle_hook_func host_idle)
{
    return 0;
}

static inline u32 bsp_reset_is_successful(u32 timeout_ms)
{
    return 1;
}
static inline u32 modem_reset_fail_id_get(void)
{
    return 0;
}

#endif /* end of CONFIG_BALONG_MODEM_RESET */

/* 共享内存中保存MODEM单独复位流程中的时间戳 */
/* 以下用于CCPU 复位流程时间戳 */
typedef enum {
    STAMP_RESET_BASE_ADDR = 0,
    STAMP_RESET_FIQ_COUNT,
    STAMP_RESET_IDLE_FAIL_COUNT,
    STAMP_RESET_HOST_ENTER_IDLE,
    STAMP_RESET_CIPHER_SOFT_RESET,
    STAMP_RESET_CIPHER_DISABLE_CHANNLE,
    STAMP_RESET_CIPHER_ENTER_IDLE,
    STAMP_RESET_EDMA_IN,
    STAMP_RESET_EDMA_OUT,
    STAMP_RESET_UPACC_ENTER_IDLE_1,
    STAMP_RESET_UPACC_ENTER_IDLE_2,
    STAMP_RESET_UPACC_ENTER_IDLE_3,
    STAMP_RESET_CICOM0_SOFT_RESET,
    STAMP_RESET_CICOM1_SOFT_RESET,
    STAMP_RESET_HDLC_SOFT_RESET,
    STAMP_RESET_IPF_SOFT_RESET,
    STAMP_RESET_IPF_ENTER_IDLE,
    STAMP_RESET_BBP_DMA_ENTER_IDLE,
    STAMP_RESET_WBBP_MSTER_STOP,
    STAMP_RESET_WBBP_SLAVE_STOP,
    STAMP_RESET_WBBP_ENTER_IDLE,
    STAMP_RESET_BBE16_ENTER_IDLE,
    STAMP_RESET_CBBE16_ENTER_IDLE,
    STAMP_RESET_HWSPINLOCK_IDLE,
    STAMP_RESET_BBP_DEBUG,
    STAMP_RESET_NV_IDLE,
    STAMP_RESET_BBP_HARQ_IDLE,
    STAMP_RESET_RSRACC_ENTER_IDLE,
    STAMP_RESET_RSRACC_STEP,
    STAMP_RESET_RSRACC_OUT_IDLE,
    STAMP_RESET_NXDSP_SYSBUS_RESET,
    STAMP_RESET_CBBE16_SYSBUS_RESET,
    STAMP_RESET_NXDSP_WAITI_STATE,
    STAMP_RESET_SOCP_MODEM_CHAN_STATE,
    STAMP_RESET_LREICC_IN,
    STAMP_RESET_LREICC_STEP,
    STAMP_RESET_LREICC_ERR,
    STAMP_RESET_LREICC_OUT,

    /* edma部分 细化 */
    STAMP_RESET_EDMA_DEV_NUM,
    STAMP_RESET_EDMA_CHECK_CLK,
    STAMP_RESET_EDMA_STOP_BUS,
    STAMP_RESET_EDMA_ENTER_IDLE,
    STAMP_RESET_HOST_IDLE_FAIL_LOG,
    /* 各 host 增加打点 在此之前 */
    STAMP_RESET_HOST_INOUT_IDLE,
    STAMP_RESET_HOST_IDLE_QUIT,
    STAMP_RESET_CORE0_INFIQ,
    STAMP_RESET_CORE1_INFIQ,

    STAMP_RESET_FIQ_OUT_COUNT,

    STAMP_RESET_NR_HOST_ENTER_IDLE,
    STAMP_RESET_NRFIQ_IN_COUNT,
    STAMP_RESET_NR_HOST_IN_IDLE,
    STAMP_RESET_NR_HOST_OUT_IDLE,
    STAMP_RESET_ECIPHER_SOFT_RESET,
    STAMP_RESET_ECIPHER_ENTER_IDLE,
    STAMP_RESET_NRDSP_ENTER_STATE,
    STAMP_RESET_NRDSP_WAITI_STATE,
    STAMP_RESET_LL1C_SYSBUS_RESET,
    STAMP_RESET_NRBBP_IDLE_STATE,
    STAMP_RESET_NREICC_IN,
    STAMP_RESET_NREICC_STEP,
    STAMP_RESET_NREICC_ERR,
    STAMP_RESET_NREICC_OUT,
    STAMP_RESET_HACEICC_IN,
    STAMP_RESET_HACEICC_STEP,
    STAMP_RESET_HACEICC_ERR,
    STAMP_RESET_HACEICC_OUT,
    STAMP_RESET_NR_HOST_IDLE_FAIL_LOG,
    STAMP_RESET_NR_HOST_END_IDLE,
    STAMP_RESET_NRCORE0_INFIQ,
    STAMP_RESET_NRCORE1_INFIQ,
    STAMP_RESET_NRCORE2_INFIQ,
    STAMP_RESET_NRCORE3_INFIQ,
    STAMP_RESET_NRFIQ_OUT_COUNT,

    /* 以下用于TSP融合框架单独复位流程时间戳 */
    STAMP_RESET_TSP_HOST_ENTER_IDLE,
    STAMP_RESET_TSP_HOST_IN_IDLE,
    STAMP_RESET_TSP_HOST_OUT_IDLE,
    STAMP_RESET_TSP_HOST_IDLE_FAIL_LOG,
    STAMP_RESET_TSP_MULTI_CB_MASK,
    STAMP_RESET_TSP_MULTI_CB_FAIL_LOG,
    STAMP_RESET_TSP_HOST_CORE_NO,
    STAMP_RESET_ULCIPHER_SOFT_RESET,
    STAMP_RESET_ULCIPHER_ENTER_IDLE,
    STAMP_RESET_L2DLE_RESET_STEP,
    STAMP_RESET_L2DLE_TS,
    STAMP_RESET_TSP_CLR_END,       // 复位后置HOST_ENTER_IDLE清零，后续保留
    STAMP_RESET_TSP_CORE0_INFIQ,
    STAMP_RESET_TSP_CORE1_INFIQ,
    STAMP_RESET_TSP_CORE2_INFIQ,
    STAMP_RESET_TSP_CORE3_INFIQ,
    STAMP_RESET_TSP_CORE4_INFIQ,
    STAMP_RESET_TSP_CORE5_INFIQ,
    STAMP_RESET_TSP_CORE6_INFIQ,
    STAMP_RESET_TSP_CORE7_INFIQ,
    STAMP_RESET_TSP_CORE8_INFIQ,
    STAMP_RESET_TSP_CORE9_INFIQ,
    STAMP_RESET_TSP_CORE10_INFIQ,
    STAMP_RESET_TSP_CORE11_INFIQ,
    STAMP_RESET_TSP_CORE12_INFIQ,
    STAMP_RESET_TSP_CORE13_INFIQ,
    STAMP_RESET_TSP_CORE14_INFIQ,
    STAMP_RESET_TSP_CORE15_INFIQ,
    STAMP_RESET_TSP_NMI_IN_COUNT,
    STAMP_RESET_TSP_NMI_OUT_COUNT,
    STAMP_RESET_TSP_IDLE_END,

    /* 以下用于M3单独复位流程时间戳 */
    STAMP_RESET_M3_BASE_ADDR,
    STAMP_RESET_M3_BUSERROR_STEP1,
    STAMP_RESET_M3_BUSERROR_STEP2,
    STAMP_RESET_M3_BUSERROR_STEP3,
    STAMP_RESET_M3_NOC_PROBE_DISABLE,
    STAMP_RESET_M3_NOC_ENTER_LPMODE_CONFIG,
    STAMP_RESET_M3_NOC_ENTER_LPMODE_STEP1,
    STAMP_RESET_M3_NOC_ENTER_LPMODE_STEP2,
    STAMP_RESET_M3_CLEAN_NMI,
    STAMP_RESET_M3_RESET_SUBSYS,

    /* 以下用于M3单独解复位流程时间戳 */
    STAMP_UNRESET_M3_BASE_ADDR,
    STAMP_UNRESET_M3_UNRESET_SUBSYS,
    STAMP_UNRESET_M3_A9_STAR_ADDR,
    STAMP_UNRESET_M3_UNRESET_A9,
} stamp_reset_e;


/* ****************************************************************************
 函 数 名  : bsp_reset_init
 功能描述  : reset模块初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : s32, RESET_OK: 初始化成功；其他值: 初始化失败
*****************************************************************************/
s32 bsp_reset_init(void);

int reset_of_node_init(void);
#if defined(__KERNEL__)
s32 bsp_reset_cb_func_register(const char *name, pdrv_reset_cbfun func, int user_data, int prior);

void bsp_modem_power_off(void);
void bsp_modem_power_on(void);
int bsp_cp_reset(void);
u32 bsp_reset_is_connect_ril(void);
u32 bsp_reset_is_feature_on(void);
u32 modem_reset_fail_id_get(void);
void bsp_reset_set_modem_wait_state(u32 state);
void bsp_reset_set_dump_wait_state(u32 state);
void ccore_ipc_enable(void);
void ccore_ipc_disable(void);
s32 bsp_dlock_reset_cb(drv_reset_cb_moment_e eparam, int usrdata);

#ifdef CONFIG_CSHELL
int cshell_mode_reset_cb(drv_reset_cb_moment_e eparam, int usrdata);
#endif
s32 bsp_rfile_reset_cb(drv_reset_cb_moment_e eparam, s32 usrdata);
s32 bsp_icc_channel_reset(drv_reset_cb_moment_e stage, int usrdata);
void ipc_modem_reset_cb(drv_reset_cb_moment_e eparam, int usrdata);
s32 bsp_mem_ccore_reset_cb(drv_reset_cb_moment_e enParam, int userdata);
int bsp_dump_sec_channel_free(drv_reset_cb_moment_e eparam, s32 usrdata);

#if (FEATURE_TDS_WCDMA_DYNAMIC_LOAD == FEATURE_ON)
extern s32 bsp_loadps_reset_cb(drv_reset_cb_moment_e eparam, s32 userdata);
#else
static inline s32 bsp_loadps_reset_cb(drv_reset_cb_moment_e eparam, s32 userdata)
{
    return 0;
}
#endif
#endif

#endif /*  __BSP_RESET_H__ */
