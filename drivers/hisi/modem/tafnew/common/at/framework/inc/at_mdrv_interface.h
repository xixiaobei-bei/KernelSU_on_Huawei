/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef _AT_MDRV_INTERFACE_H_
#define _AT_MDRV_INTERFACE_H_

#include "vos.h"
#include "at_mntn.h"
#include "mdrv.h"
#if (!defined(MODEM_FUSION_VERSION))
#include "msp_diag.h"
#else
#include "mdrv_diag.h"
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#include "mdrv_socp.h"
#include "mdrv_misc.h"
#include "mdrv_sysboot.h"
#include "mdrv_om.h"          /* AT dump需要包含该文件 */
#include "mdrv_nvim.h"
#include "mdrv_efuse.h"
#endif
#endif

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#include <linux/pm_wakeup.h>
/* 针对hifireset_regcbfunc接口包含的头文件 */
#include "adrv.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/*
 * L4A用到的以下两个宏定义，原本是在mdrv_diag_system_common.h中的，
 * 但B5010上不再提供mdrv_diag_system_common.h，为了编译通过和最小量的修改代码，增加本地定义此宏
 */
#ifdef MODEM_FUSION_VERSION
#define ERR_MSP_UNKNOWN (501)    /* 字符串："未知错误" “unknown error” */
#define ERR_MSP_INVALID_OP (533) /* 字符串："非法操作（比如读取特殊NV）""Invalid Operation"	 */
#define  ERR_MSP_AT_CHANNEL_BUSY         (564) /* DIAG/MUX的AT通道BUSY*/
#endif

#define AT_SMS_WAKE_LOCK_TIMER_LEN 1000

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)

/*lint -esym(528,AT_SetDrvGpio)*/
static inline VOS_INT32 AT_SetDrvGpio(VOS_UINT8 group, VOS_UINT8 pin, VOS_UINT8 value)
{
    AT_PR_LOGI("enter");
    return DRV_GPIO_SET(group, pin, value);
}

/*lint -esym(528,AT_IsCryptoSecStarted)*/
static inline VOS_INT32 AT_IsCryptoSecStarted(VOS_UINT8 *data)
{
    AT_PR_LOGI("enter");
    return mdrv_crypto_sec_started(data);
}

/*lint -esym(528,AT_IsCryptoSecbootSupported)*/
static inline VOS_INT32 AT_IsCryptoSecbootSupported(VOS_UINT8 *data)
{
    AT_PR_LOGI("enter");
    return mdrv_crypto_secboot_supported(data);
}

/*lint -esym(528,AT_CryptoStartSecure)*/
static inline VOS_INT32 AT_CryptoStartSecure(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return mdrv_crypto_start_secure();
}

/*lint -esym(528,AT_GetDloadInfo)*/
static inline DLOAD_VER_INFO_S* AT_GetDloadInfo(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return mdrv_dload_get_info();
}

/*lint -esym(528,AT_SetDloadCurMode)*/
static inline VOS_VOID AT_SetDloadCurMode(DLOAD_MODE_E mode)
{
    AT_PR_LOGI("enter");
    mdrv_dload_set_curmode(mode);
}

/*lint -esym(528,AT_SetDloadSoftload)*/
static inline VOS_VOID AT_SetDloadSoftload(VOS_INT32 softLoad)
{
    AT_PR_LOGI("enter");
    mdrv_dload_set_softload(softLoad);
}

/*lint -esym(528,AT_EfuseIoctl)*/
static inline VOS_INT32 AT_EfuseIoctl(VOS_INT32 cmd, VOS_INT32 arg, VOS_UINT8 *buf, VOS_INT32 len)
{
    AT_PR_LOGI("enter");
    return mdrv_efuse_ioctl(cmd, arg, buf, len);
}

/*lint -esym(528,AT_GetMiscBatteryState)*/
static inline VOS_INT32 AT_GetMiscBatteryState(BATT_STATE_S *batteryState)
{
    AT_PR_LOGI("enter");
    return mdrv_misc_get_battery_state(batteryState);
}

/*lint -esym(528,AT_GetMiscCbcState)*/
static inline VOS_INT32 AT_GetMiscCbcState(VOS_UINT8 *bcs, VOS_UINT8 *bcl)
{
    AT_PR_LOGI("enter");
    return mdrv_misc_get_cbc_state(bcs, bcl);
}

/*lint -esym(528,AT_GetMiscChargeState)*/
static inline VOS_INT32 AT_GetMiscChargeState(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return mdrv_misc_get_charge_state();
}

/*lint -esym(528,AT_GetMiscChargingStatus)*/
static inline VOS_INT32 AT_GetMiscChargingStatus(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return mdrv_misc_get_charging_status();
}

/*lint -esym(528,AT_SetMiscChargeState)*/
static inline VOS_VOID AT_SetMiscChargeState(unsigned long state)
{
    AT_PR_LOGI("enter");
    mdrv_misc_set_charge_state(state);
}

/*lint -esym(528,AT_SplyMiscBattery)*/
static inline VOS_INT32 AT_SplyMiscBattery(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return mdrv_misc_sply_battery();
}

/*lint -esym(528,AT_SdProcess)*/
static inline VOS_UINT32 AT_SdProcess(VOS_UINT32 type, VOS_UINT32 addr, VOS_UINT32 data, VOS_UINT8 *buf,
                                      VOS_UINT32 *err)
{
    AT_PR_LOGI("enter");
    return mdrv_sd_at_process(type, addr, data, buf, err);
}

/*lint -esym(528,AT_GetSdOpptStatus)*/
static inline VOS_UINT32 AT_GetSdOpptStatus(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return mdrv_sd_get_opptstatus();
}

/*lint -esym(528,AT_SetModemState)*/
static inline VOS_INT32 AT_SetModemState(VOS_UINT16 modemId, VOS_UINT32 state)
{
    AT_PR_LOGI("enter");
    return mdrv_set_modem_state(state);
}

/*lint -esym(528,AT_GetSocpSdLogCfg)*/
static inline VOS_UINT32 AT_GetSocpSdLogCfg(socp_encdst_buf_log_cfg_s *cfg)
{
    AT_PR_LOGI("enter");
    return mdrv_socp_get_log_cfg(cfg);
}

/*lint -esym(528,AT_RegisterSysbootResetNotify)*/
static inline VOS_INT32 AT_RegisterSysbootResetNotify(const VOS_CHAR *name, pdrv_reset_cbfun cbFun, VOS_INT32 userData,
                                                      VOS_INT32 prioLevel)
{
    AT_PR_LOGI("enter");
    return mdrv_sysboot_register_reset_notify(name, cbFun, userData, prioLevel);
}

/*lint -esym(528,AT_SysbootRestart)*/
static inline VOS_VOID AT_SysbootRestart(VOS_VOID)
{
    AT_PR_LOGI("enter");
    mdrv_sysboot_restart();
}

/*lint -esym(528,AT_GetWifiPACurMode)*/
static inline VOS_INT32 AT_GetWifiPACurMode(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return WIFI_GET_PA_CUR_MODE();
}

/*lint -esym(528,AT_GetWifiPAMode)*/
static inline VOS_INT32 AT_GetWifiPAMode(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return WIFI_GET_PA_MODE();
}

/*lint -esym(528,AT_GetWifiRxPacketReport)*/
static inline VOS_VOID AT_GetWifiRxPacketReport(VOS_UINT32 *ucastPkts, VOS_UINT32 *mcastPkts)
{
    AT_PR_LOGI("enter");
    WIFI_GET_RX_PACKET_REPORT(ucastPkts, mcastPkts);
}

/*lint -esym(528,AT_GetWifiStatus)*/
static inline VOS_INT32 AT_GetWifiStatus(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return WIFI_GET_STATUS();
}

/*lint -esym(528,AT_GetWifiTcmdMode)*/
static inline VOS_INT32 AT_GetWifiTcmdMode(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return WIFI_GET_TCMD_MODE();
}

/*lint -esym(528,AT_WifiPowerShutdown)*/
static inline VOS_INT32 AT_WifiPowerShutdown(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return WIFI_POWER_SHUTDOWN();
}

/*lint -esym(528,AT_WifiPowerStart)*/
static inline VOS_INT32 AT_WifiPowerStart(VOS_VOID)
{
    AT_PR_LOGI("enter");
    return WIFI_POWER_START();
}

/*lint -esym(528,AT_SetWifiPaMode)*/
static inline VOS_INT32 AT_SetWifiPaMode(VOS_INT32 wifiPaMode)
{
    AT_PR_LOGI("enter");
    return WIFI_SET_PA_MODE(wifiPaMode);
}

#define AT_WIFI_TEST_CMD_CALL(cmd) do { \
    AT_TestWifiCmd(cmd, strlen(cmd)); \
} while (0)

/*lint -esym(528,AT_TestWifiCmd)*/
static inline VOS_VOID AT_TestWifiCmd(VOS_CHAR *cmdStr, VOS_UINT32 len)
{
    if (len != 0) {
        AT_PR_LOGI("enter");
    }
    WIFI_TEST_CMD(cmdStr);
}

/*lint -esym(528,AT_GetSdStatus)*/
static inline VOS_INT32 AT_GetSdStatus(VOS_VOID)
{
#if defined(__PC_UT__) && !defined(DMT)
    return AT_GetSdStatusStub();
#else
    return DRV_SDMMC_GET_STATUS();
#endif
}

static inline VOS_VOID AT_InitWakeLock(struct wakeup_source *ws, const char *name)
{
    AT_PR_LOGI("enter");
    wakeup_source_init(ws, name);
}

static inline VOS_VOID AT_WakeLock(struct wakeup_source *ws)
{
    AT_PR_LOGI("enter");
    __pm_stay_awake(ws);
}

static inline VOS_VOID AT_WakeUnLock(struct wakeup_source *ws)
{
    AT_PR_LOGI("enter");
    __pm_relax(ws);
}

static inline VOS_VOID AT_SmsWakeLock(struct wakeup_source *ws)
{
    AT_PR_LOGI("AT_SmsWakeLock enter");
    __pm_wakeup_event(ws, AT_SMS_WAKE_LOCK_TIMER_LEN);
}
#else

#if (VOS_OS_VER != VOS_WIN32)

/* 设备枚举最大端口个数 */
#define DYNAMIC_PID_MAX_PORT_NUM        17

/**@todo 桩接口，待删除*/
/*lint -esym(528,USB_otg_switch_get)*/
static inline int USB_otg_switch_get(unsigned char *pucValue)
{
    (void)pucValue;
    return 0;
}

/**
 * @todo 桩接口，待删除
 */
#define DRV_USB_PHY_SWITCH_GET(value) USB_otg_switch_get(value)
#define AT_USB_SWITCH_SET_VBUS_VALID            1
#define AT_USB_SWITCH_SET_VBUS_INVALID          2

#define USB_SWITCH_ON       1
#define USB_SWITCH_OFF      0

/* NV项50091结构，代表设备将要枚举的端口形态 */
typedef struct
{
    unsigned int ulStatus;
    unsigned char aucFirstPortStyle[DYNAMIC_PID_MAX_PORT_NUM];  /* 设备切换前端口形态 */
    unsigned char aucRewindPortStyle[DYNAMIC_PID_MAX_PORT_NUM]; /* 设备切换后端口形态 */
    unsigned char reserved[22];
}DRV_DYNAMIC_PID_TYPE_STRU;

/*lint -esym(528,DRV_GET_LINUXSYSTYPE)*/
static inline VOS_INT32 DRV_GET_LINUXSYSTYPE(VOS_VOID)
{
    return 0;
}

/*lint -esym(528,DRV_SET_PORT_QUIRY)*/
static inline unsigned int DRV_SET_PORT_QUIRY(DRV_DYNAMIC_PID_TYPE_STRU *pstDynamicPidType)
{
    return 0;
}

/**
 * @brief AT^PSTANDBY
 *
 * @par 描述:
 * AT^PSTANDBY
 *
 * @attention 无
 *
 * @todo 桩接口，待删除
 */
 /*lint -esym(528,DRV_PWRCTRL_STANDBYSTATEACPU)*/
static inline unsigned int DRV_PWRCTRL_STANDBYSTATEACPU(unsigned int ulStandbyTime, unsigned int ulSwitchtime)
{
    return 0;
}

/*lint -esym(528,DRV_OS_STATUS_SWITCH)*/
static inline int DRV_OS_STATUS_SWITCH(int enable)
{
    return 0;
}

/**
 * @brief 获取释放内存大小
 *
 * @par 描述:
 * 获取释放内存大小
 *
 * @attention 无
 *
 * @param 无
 *
 * @retval 释放内存大小
 *
 * @todo 桩接口，待删除
 */
 /*lint -esym(528,FREE_MEM_SIZE_GET)*/
static inline unsigned int FREE_MEM_SIZE_GET(void)
{
    return 0;
}

/*lint -esym(528,DRV_USB_GET_AVAILABLE_PORT_TYPE)*/
static inline unsigned int DRV_USB_GET_AVAILABLE_PORT_TYPE(unsigned char *pucPortType,
                            unsigned int *pulPortNum, unsigned int ulPortMax)
{
    return 0;
}

/*lint -esym(528,DRV_GET_DIAG_MODE_VALUE)*/
static inline int DRV_GET_DIAG_MODE_VALUE(unsigned char *pucDialmode,
                                          unsigned char *pucCdcSpec)
{
    return 0;
}

/*lint -esym(528,DRV_GET_U2DIAG_DEFVALUE)*/
static inline unsigned int DRV_GET_U2DIAG_DEFVALUE(void)
{
    return 0;
}

/**@todo 桩接口，待删除*/
/*lint -esym(528,DRV_AT_SETAPPDAILMODE)*/
static inline void DRV_AT_SETAPPDAILMODE(unsigned int ulStatus)
{
    (void)ulStatus;
    return;
}

/**@todo 桩接口，待删除*/
/*lint -esym(528,DRV_U2DIAG_VALUE_CHECK)*/
static inline int DRV_U2DIAG_VALUE_CHECK(unsigned int DiagValue)
{
    (void)DiagValue;
    return 0;
}

/**@todo 桩接口，待删除*/
/*lint -esym(528,DRV_USB_PHY_SWITCH_SET)*/
static inline int DRV_USB_PHY_SWITCH_SET(unsigned char ucValue)
{
    (void)ucValue;
    return 0;
}

/**@todo 桩接口，待删除*/
/*lint -esym(528,DRV_USB_PORT_TYPE_VALID_CHECK)*/
static inline int DRV_USB_PORT_TYPE_VALID_CHECK(unsigned char *pucPortType,
                                                unsigned int ulPortNum)
{
    (void)pucPortType;
    (void)ulPortNum;
    return 0;
}

/**@todo 桩接口，待删除*/
/*lint -esym(528,DRV_GET_PORT_MODE)*/
static inline unsigned char DRV_GET_PORT_MODE(char*PsBuffer, unsigned int*Length)
{
    (void)PsBuffer;
    (void)Length;
    return 0;
}

/**@todo 桩接口，待删除*/
/*lint -esym(528,DRV_SET_PID)*/
static inline int DRV_SET_PID(unsigned char u2diagValue)
{
    (void)u2diagValue;
    return 0;
}
#endif

/*lint -esym(528,AT_SetModemState)*/
static inline VOS_INT32 AT_SetModemState(VOS_UINT16 modemId, VOS_UINT32 state)
{
    return mdrv_sysboot_set_modem_state(modemId, state);
}

/*lint -esym(528,AT_GetSocpSdLogCfg)*/
static inline VOS_UINT32 AT_GetSocpSdLogCfg(socp_encdst_buf_log_cfg_s *cfg)
{
    AT_PR_LOGI("enter");
    return mdrv_diag_get_log_cfg(cfg);
}

#endif

/*lint -esym(528,AT_IsMiscSupport)*/
static inline bsp_module_support_e AT_IsMiscSupport(bsp_module_type_e moduleType)
{
    AT_PR_LOGI("enter");
    return mdrv_misc_support_check(moduleType);
}

/*lint -esym(528,AT_GetTimerAccuracyTimestamp)*/
static inline VOS_INT32 AT_GetTimerAccuracyTimestamp(VOS_UINT32 *high32BitValue, VOS_UINT32 *low32BitValue)
{
    AT_PR_LOGI("enter");
    return mdrv_timer_get_accuracy_timestamp(high32BitValue, low32BitValue);
}

#if (VOS_WIN32 == VOS_OS_VER || OSA_CPU_CCPU == VOS_OSA_CPU)
/*lint -esym(528,AT_SysbootShutdown)*/
static inline VOS_VOID AT_SysbootShutdown(sysboot_shutdown_reason_e reason)
{
    AT_PR_LOGI("enter");
    mdrv_sysboot_shutdown(reason);
}
#else
/*lint -esym(528,AT_SysbootShutdown)*/
static inline VOS_VOID AT_SysbootShutdown(VOS_VOID)
{
    AT_PR_LOGI("enter");
    mdrv_sysboot_shutdown();
}
#endif

/*lint -esym(528,AT_OmSystemError)*/
static inline VOS_VOID AT_OmSystemError(VOS_INT32 modId, VOS_INT32 arg1, VOS_INT32 arg2, VOS_CHAR *arg3,
                                        VOS_INT32 arg3Length)
{
    AT_PR_LOGI("enter");
    mdrv_om_system_error(modId, arg1, arg2, arg3, arg3Length);
}

/*lint -esym(528,AT_CheckFactoryNv)*/
static inline VOS_UINT32 AT_CheckFactoryNv(VOS_UINT32 mode)
{
    AT_PR_LOGI("enter");
    return mdrv_nv_check_factorynv(mode);
}

/*lint -esym(528,AT_GetEfuseDieid)*/
static inline VOS_INT32 AT_GetEfuseDieid(VOS_UINT8 *buf, VOS_INT32 length)
{
    AT_PR_LOGI("enter");
    return mdrv_efuse_get_dieid(buf, length);
}

/*lint -esym(528,AT_QueryPpmLogPort)*/
static inline VOS_UINT32 AT_QueryPpmLogPort(VOS_UINT32 *logPort)
{
    AT_PR_LOGI("enter");
    return mdrv_diag_get_log_port(logPort);
}

/*lint -esym(528,AT_SwitchDiagLogPort)*/
static inline VOS_UINT32 AT_SwitchDiagLogPort(VOS_UINT32 phyPort, VOS_UINT32 effect)
{
    AT_PR_LOGI("enter");
    return mdrv_diag_set_log_port(phyPort, effect);
}

/*lint -esym(528,AT_GetSocpCfgIndMode)*/
static inline VOS_INT32 AT_GetSocpCfgIndMode(VOS_UINT32 *mode)
{
    AT_PR_LOGI("enter");
    return mdrv_socp_get_cfg_ind_mode(mode);
}

/*lint -esym(528,AT_GetSocpCompressIndMode)*/
static inline VOS_INT32 AT_GetSocpCompressIndMode(VOS_UINT32 *mode)
{
    AT_PR_LOGI("enter");
    return mdrv_socp_get_cps_ind_mode(mode);
}

/*lint -esym(528,AT_SetSocpCfgIndMode)*/
static inline VOS_INT32 AT_SetSocpCfgIndMode(socp_ind_mode_e mode)
{
    AT_PR_LOGI("enter");
    return mdrv_socp_set_cfg_ind_mode(mode);
}

/*lint -esym(528,AT_SetSocpCpsIndMode)*/
static inline VOS_INT32 AT_SetSocpCpsIndMode(deflate_ind_compress_e mode)
{
    AT_PR_LOGI("enter");
    return mdrv_socp_set_cps_ind_mode(mode);
}

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _AT_MDRV_INTERFACE_H_ */
