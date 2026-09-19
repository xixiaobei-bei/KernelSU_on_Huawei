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


#ifndef __ACORE_NV_ID_GUCNAS_H__
#define __ACORE_NV_ID_GUCNAS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
  1 其他头文件包含
*****************************************************************************/
/*****************************************************************************
 枚举名    : GUCNAS_ACORE_NV_ID_ENUM
 枚举说明  : GUCNAS组件ACORE的NV项ID枚举
*****************************************************************************/
enum GUCNAS_ACORE_NV_ID_ENUM
{
/*    36 */     en_NV_Item_SW_VERSION_FLAG                  = 36,       /* OM_SW_VER_FLAG_STRU */
/*    90 */     en_NV_Item_BATTERY_ADC                      = 90,       /* VBAT_CALIBART_TYPE */

/*  2102 */     en_NV_Item_CDMAMODEMSWITCH_NOT_RESET_CFG    = 2102,     /* TAF_NVIM_CDMAMODEMSWITCH_NOT_RESET_CFG_STRU  用来配置CDMAMODEMSWITCH不重启特性是否开启，若不开启，设置后需RESET MODEM */

/*  2336 */     en_NV_Item_Ipv6_Address_Test_Mode_Cfg       = 2336,     /* TAF_NVIM_IPV6_ADDR_TEST_MODE_CFG_STRU        IPV6地址测试模式配置，0x55AA55AA为测试模式，其他值为正常模式 */
/*  2337 */     en_NV_Item_ADS_IPF_MODE_CFG                 = 2337,     /* TAF_NV_ADS_IPF_MODE_CFG_STRU                 IPF处理ADS下行数据的模式配置 */

/*  2354 */     en_NV_Item_ADS_WAKE_LOCK_CFG                = 2354,     /* TAF_NV_ADS_WAKE_LOCK_CFG_STRU ADS WAKELOCK 配置 */

/*  2361 */     en_NV_Item_Print_Modem_Log_Type             = 2361,     /* TAF_NV_PRINT_MODEM_LOG_TYPE_STRU             控制是否输出modem log的类型 */

/*  2369 */     en_NV_Item_ADS_MEM_POOL_CFG                 = 2369,     /* TAF_NV_ADS_MEM_POOL_CFG_STRU */

/*  2391 */     en_NV_Item_User_Reboot_Support_Flg          = 2391,     /* TAF_NVIM_USER_REBOOT_SUPPORT_STRU            用户是否可以发起整机复位 */

/*  2413 */     en_NV_Item_PDP_REDIAL_FOR_NO_CAUSE_CFG      = 2413,     /* TAF_NV_PDP_REDIAL_FOR_NO_CAUSE_CFG_STRU      PDP双栈激活，网侧无原因值重拨功能开关配置 */

/*  2421 */     en_NV_Item_Rnic_Napi_Cfg                    = 2421,     /* TAF_NV_RNIC_NAPI_CFG_STRU                    RNIC NAPI GRO配置 */

/*  2437 */     en_NV_Item_AT_CLIENT_CFG                    = 2437,     /* TAF_AT_NVIM_AT_CLIENT_CFG_STRU               双卡双通控制AT通道归属哪个Modem信息 */

/*  2461 */     en_NV_Item_ADS_PACKET_ERROR_FEEDBACK_CFG    = 2461,     /* TAF_NV_ADS_ERROR_FEEDBACK_CFG_STRU           下行错包反馈配置*/

/*  2470 */     en_NV_Item_Pdp_Redial_For_No_Pdp_Type_CAUSE_CFG = 2470, /* TAF_NV_PDP_REDIAL_FOR_NO_PDP_TYPE_CAUSE_CFG_STRU PDP双栈激活，网侧拒绝28号原因值重拨功能开关配置 */

/*  2478 */     en_NV_Item_IMS_SWITCH_RAT_RELATION_CFG      = 2478,     /* TAF_NV_IMS_SWITCH_RAT_RELATION_CFG_STRU      ^IMSSWITCH命令2/3G开关与4G开关关联配置 */

/*  2600 */     en_NV_Item_COMMDEGBUG_CFG                   = 2600,     /* TAF_AT_NVIM_COMMDEGBUG_CFG_STRU */
/*  2601 */     en_NV_Item_PID_Enable_Type                  = 2601,     /* nv_pid_enable_type */
/*  2602 */     en_NV_Item_AT_DISSD_PWD                     = 2602,     /* TAF_AT_NVIM_DISSD_PWD_STRU */
/*  2603 */     en_NV_Item_AT_DISLOG_PWD_NEW                = 2603,     /* TAF_AT_NVIM_DISLOG_PWD_NEW_STRU */
/*  2604 */     en_NV_Item_DEFAULT_LINK_OF_UART             = 2604,     /* TAF_AT_NVIM_DEFAULT_LINK_OF_UART_STRU */
/*  2605 */     en_NV_Item_AT_FACTORY_INFO                  = 2605,     /* TAF_AT_NVIM_FACTORY_INFO_STRU */
/*  2606 */     en_NV_Item_AT_MANUFACTURE_DATE              = 2606,     /* TAF_AT_NVIM_MANUFACTURE_DATE_STRU */
/*  2607 */     en_NV_Item_UART_CFG                         = 2607,     /* TAF_NV_UART_CFG_STRU */
/*  2608 */     en_NV_Item_PORT_BUFF_CFG                    = 2608,     /* TAF_NV_PORT_BUFF_CFG_STRU */

/*  2627 */     en_NV_Item_RXDIV_CONFIG                     = 2627,     /* TAF_AT_NVIM_RXDIV_CONFIG_STRU */

/*  2635 */     en_NV_Item_NDIS_DHCP_DEF_LEASE_TIME         = 2635,     /* NDIS_NV_DHCP_LEASE_HOUR_STRU */
/*  2636 */     en_NV_Item_CREG_CGREG_CI_Four_Byte_Rpt      = 2636,     /* NAS_NV_CREG_CGREG_CI_FOUR_BYTE_RPT_STRU */

/*  2640 */     en_NV_Item_ATSETZ_RET_VALUE                 = 2640,     /* TAF_AT_TZ_STRU */
/*  2641 */     en_NV_Item_NOT_SUPPORT_RET_VALUE            = 2641,     /* TAF_AT_NOT_SUPPORT_CMD_ERROR_TEXT_STRU */
/*  2642 */     en_NV_Item_AT_RIGHT_PASSWORD                = 2642,     /* TAF_AT_NVIM_RIGHT_OPEN_FLAG_STRU */

/*  2649 */     en_NV_Item_IPV6_ROUTER_MTU                  = 2649,     /* TAF_NDIS_NV_IPV6_ROUTER_MTU_STRU */
/*  2650 */     en_NV_Item_DIAL_CONNECT_DISPLAY_RATE        = 2650,     /* AT_NVIM_DIAL_CONNECT_DISPLAY_RATE_STRU */

/*  2655 */     en_NV_Item_ADS_Queue_Scheduler_Pri          = 2655,     /* ADS_UL_QUEUE_SCHEDULER_PRI_NV_STRU */
/*  2656 */     en_NV_Item_WIFI_INFO                        = 2656,     /* AT_WIFI_INFO_STRU， XML文件中的结构体名为nv_wifi_info */

/*  2675 */     en_NV_Item_Mux_Support_Flg                  = 2675,     /* TAF_AT_NVIM_MUX_SUPPORT_FLG_STRU */

/*  2677 */     en_NV_Item_MUX_REPORT_CFG                   = 2677,     /* TAF_AT_NVIM_MUX_REPORT_CFG_STRU */

/*  2679 */     en_NV_Item_SS_CUSTOMIZE_PARA                = 2679,     /* AT_SS_CUSTOMIZE_PARA_STRU */
/*  2680 */     en_NV_Item_SHARE_PDP_INFO                   = 2680,     /* TAF_NVIM_SHARE_PDP_INFO_STRU Share PDP特性控制NV */

/*  2696 */     en_NV_Item_ADS_DYNAMIC_THRESHOLD_CFG        = 2696,     /* ADS_NV_DYNAMIC_THRESHOLD_STRU */

/*  2713 */     en_NV_Item_AP_RPT_SRV_URL                   = 2713,     /* TAF_AT_NVIM_AP_RPT_SRV_URL_STRU */
/*  2714 */     en_NV_Item_AP_XML_INFO_TYPE                 = 2714,     /* TAF_AT_NVIM_AP_XML_INFO_TYPE_STRU */
/*  2715 */     en_NV_Item_AP_XML_RPT_FLAG                  = 2715,     /* TAF_AT_NVIM_AP_XML_RPT_FLG_STRU */

/*  8200 */     en_NV_Item_GmmInfo                          = 8200,     /* TAF_AT_NV_GMM_INFO_STRU */


/*  8340 */     en_NV_Item_REPORT_REG_ACT_FLG               = 8340,     /* TAF_AT_NVIM_REPORT_REG_ACT_FLG_STRU */

/*  8514 */     en_NV_Item_IPV6_CAPABILITY                  = 8514,     /* AT_NV_IPV6_CAPABILITY_STRU */

/*  9041 */     en_NV_Item_SubPlatFormInfo                  = 9041,     /* NAS_NVIM_PLATFORM_STRU */
/*  9070 */     en_NV_Item_Eqver                            = 9070,     /* TAF_AT_EQ_VER_STRU */
/*  9071 */     en_NV_Item_Csver                            = 9071,     /* TAF_NVIM_CS_VER_STRU */
/*  9110 */     en_NV_Item_MULTI_WIFI_KEY                   = 9110,     /* TAF_AT_MULTI_WIFI_SEC_STRU 支持多组SSID扩展新增WIKEY NV项 */
/*  9111 */     en_NV_Item_MULTI_WIFI_STATUS_SSID           = 9111,     /* TAF_AT_MULTI_WIFI_SSID_STRU 支持多组SSID扩展新增SSID NV项 */
/*  9113 */     en_NV_Item_AT_ABORT_CMD_PARA                = 9113,     /* AT_NVIM_ABORT_CMD_PARA_STRU */
/*  9128 */     en_NV_Item_CIMI_PORT_CFG                    = 9128,     /* TAF_AT_NVIM_CIMI_PORT_CFG_STRU */
/*  9269 */     en_NV_Item_DMS_DEBUG_CFG                    = 9269,     /* TAF_NV_PORT_DEBUG_CFG_STRU */

/* 32768 */     en_NV_Item_ManufactureInfo                  = 32768,    /* OM_MANUFACTUREINFO_STRU */

/* 50014 */     en_NV_Item_WIFI_MAC_ADDR                    = 50014,    /* OM_MAC_ADDR_STRU */
/* 50061 */     en_NV_Item_PPP_DIAL_ERR_CODE                = 50061,    /* NAS_NV_PPP_DIAL_ERR_CODE_STRU */
/* 50091 */     en_NV_Item_Huawei_Dynamic_PID_Type          = 50091,    /* AT_DYNAMIC_PID_TYPE_STRU */
/* 52001 */     en_NV_Item_WEB_ADMIN_PASSWORD               = 52001,    /* TAF_AT_NVIM_WEB_ADMIN_PASSWORD_STRU */

/*  BUTT */     en_NV_Item_GUCNAS_ACORE_NV_BUTT,
};


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __ACORE_NV_ID_GUCNAS_H__ */

