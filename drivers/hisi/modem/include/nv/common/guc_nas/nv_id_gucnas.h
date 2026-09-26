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



#ifndef __NV_ID_GUCNAS_H__
#define __NV_ID_GUCNAS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
  1 其他头文件包含
*****************************************************************************/
/*****************************************************************************
 枚举名    : GUCNAS_NV_ID_ENUM
 枚举说明  : GUCNAS组件公共的NV项ID枚举定义
*****************************************************************************/
enum GUCNAS_NV_ID_ENUM
{
/*     6 */     en_NV_Item_Serial_Num                       = 6,        /* TAF_NVIM_SERIAL_NUM_STRU */
/*    60 */     en_NV_Item_SVLTE_FLAG                       = 60,       /* SVLTE_SUPPORT_FLAG_STRU */
/*    70 */     en_NV_Item_LC_Ctrl_PARA                     = 70,       /* TAF_NV_LC_CTRL_PARA_STRU */
/*   121 */     en_NV_Item_System_APP_Config                = 121,      /* NAS_NVIM_SYSTEM_APP_CONFIG_STRU */

/*  2003 */     en_NV_Item_CFreqLock_CFG                    = 2003,     /* TAF_NVIM_CFREQ_LOCK_CFG_STRU */
/*  2095 */     en_NV_Item_LTE_SMS_CFG                      = 2095,     /* TAF_NVIM_LTE_SMS_CFG_STRU */
/*  2300 */     en_NV_Item_DSDS_Active_Modem_Mode           = 2300,     /* TAF_NV_DSDS_ACTIVE_MODEM_MODE_STRU */
/*  2316 */     en_NV_Item_ENHANCED_OPERATOR_NAME_SRV_CFG   = 2316,     /* TAF_NVIM_ENHANCED_OPER_NAME_SERVICE_CFG_STRU */
/*  2321 */     en_NV_Item_NVWR_SEC_CTRL                    = 2321,     /* TAF_NV_NVWR_SEC_CTRL_STRU    NVWR命令安全控制 */
/*  2331 */     en_NV_Item_Ccpu_Reset_Record                = 2331,     /* TAF_NVIM_CCPU_RESET_RECORD_STRU  C核单独复位记录 */
/*  2340 */     en_NV_Item_Ccwa_Ctrl_Mode                   = 2340,     /* TAF_CALL_NVIM_CCWA_CTRL_MODE_STRU */
/*  2352 */     en_NV_Item_Privacy_Log_Filter_Cfg           = 2352,     /* NAS_NV_PRIVACY_FILTER_CFG_STRU */
/*  2371 */     en_NV_Item_XCPOSRRPT_CFG                    = 2371,     /* TAF_NVIM_XCPOSRRPT_CFG_STRU 是否上报清除GPS缓存的辅助定位信息 */
/*  2387 */     en_NV_Item_USER_SYS_CFG_RAT_INFO            = 2387,     /* TAF_NVIM_MULTIMODE_RAT_CFG_STRU 用户SYS CFG RAT配置 */
/*  2396 */     en_NV_Item_Gps_Cust_CFG                     = 2396,     /* TAF_NVIM_GPS_CUST_CFG_STRU 查询GPS芯片类型 */

/*  2611 */     en_NV_Item_ProductName                      = 2611,     /* TAF_PH_PRODUCT_NAME_STRU */
/*  2615 */     en_NV_Item_MMA_AccessMode                   = 2615,     /* NAS_MMA_NVIM_ACCESS_MODE_STRU */
/*  2626 */     en_NV_Item_WINS_Config                      = 2626,     /* WINS_CONFIG_STRU */
/*  2630 */     en_NV_Item_SMS_ActiveMessage_Para           = 2630,     /* TAF_AT_NVIM_SMS_ACTIVE_MESSAGE_STRU */
/*  2644 */     en_NV_Item_PREVENT_TEST_IMSI_REG            = 2644,     /* NAS_PREVENT_TEST_IMSI_REG_STRU */

/*  2695 */     en_NV_Item_JAM_DETECT_CFG                   = 2695,     /* NV_NAS_JAM_DETECT_CFG_STRU */
/*  2698 */     en_NV_Item_FREQ_LOCK_CFG                    = 2698,     /* TAF_NVIM_FREQ_LOCK_CFG_STRU */
/*  2707 */     en_NV_Item_CUST_USSD_MODE                   = 2707,     /* TAF_USSD_NVIM_TRANS_MODE_STRU */

/*  6656 */     en_NV_Item_FollowOn_OpenSpeed_Flag          = 6656,     /* NAS_NVIM_FOLLOWON_OPENSPEED_FLAG_STRU */

/*  8203 */     en_NV_Item_FMRID                            = 8203,     /* TAF_PH_FMR_ID_STRU */
/*  8230 */     en_NV_Item_CCA_TelePara                     = 8230,     /* TAF_CCA_TELE_PARA_STRU */
/*  8233 */     en_NV_Item_MMA_MsClass                      = 8233,     /* NAS_NVIM_MS_CLASS_STRU */
/*  8237 */     en_NV_Item_SMS_SERVICE_Para                 = 8237,     /* TAF_NVIM_SMS_SERVICE_PARA_STRU */
/*  8266 */     en_NV_Item_Roam_Capa                        = 8266,     /* NAS_NVIM_ROAM_CFG_INFO_STRU */
/*  8267 */     en_NV_Item_CustomizeSimLockPlmnInfo         = 8267,     /* TAF_CUSTOM_SIM_LOCK_PLMN_INFO_STRU */
/*  8268 */     en_NV_Item_CardlockStatus                   = 8268,     /* TAF_CUSTOM_CARDLOCK_STATUS_STRU */
/*  8269 */     en_NV_Item_CustomizeSimLockMaxTimes         = 8269,     /* TAF_CUSTOM_SIM_LOCK_MAX_TIMES_STRU */
/*  8271 */     en_NV_Item_CustomizeService                 = 8271,     /* NAS_NVIM_CUSTOMIZE_SERVICE_STRU */
/*  8337 */     en_NV_Item_Imei_Svn                         = 8337,     /* TAF_SVN_DATA_STRU */
/*  8471 */     en_NV_Item_PCVOICE_Support_Flg              = 8471,     /* APP_VC_NVIM_PC_VOICE_SUPPORT_FLAG_STRU */
/*  8501 */     en_NV_Item_Sms_Me_Storage_Info              = 8501,     /* MN_MSG_ME_STORAGE_PARM_STRU */
/*  9007 */     en_NV_Item_RAT_PRIO_LIST                    = 9007,     /* TAF_PH_NVIM_RAT_ORDER_STRU */
/*  9052 */     en_NV_Item_Lte_Internation_Roam_Config      = 9052,     /* NAS_MMC_NVIM_LTE_INTERNATIONAL_ROAM_CFG_STRU */
/*  9118 */     en_NV_Item_SMS_MT_CUSTOMIZE_INFO            = 9118,     /* MN_MSG_MT_CUSTOMIZE_INFO_STRU */
/*  9123 */     en_NV_Item_CELL_SIGN_REPORT_CFG             = 9123,     /* NAS_NVIM_CELL_SIGN_REPORT_CFG_STRU */
/*  9130 */     en_NV_Item_IPV6_BACKPROC_EXT_CAUSE          = 9130,     /* TAF_NV_IPV6_FALLBACK_EXT_CAUSE_STRU */
/*  9136 */     en_NV_Item_SMS_Close_Path                   = 9136,     /* TAF_NVIM_SMS_CLOSE_PATH_CFG_STRU */
/*  9203 */     en_NV_Item_Platform_RAT_CAP                 = 9203,     /* PLATAFORM_RAT_CAPABILITY_STRU */
/*  9206 */     en_NV_Item_BODY_SAR_PARA                    = 9208,     /* MTA_BODY_SAR_PARA_STRU */
/*  9212 */     en_NV_Item_TEMP_PROTECT_CONFIG              = 9212,     /* TAF_TEMP_PROTECT_CONFIG_STRU */
/*  9237 */     en_NV_Item_SMS_DOMAIN                       = 9237,     /* TAF_NVIM_SMS_DOMAIN_STRU */
/* 50009 */     en_NV_Item_TRAFFIC_CLASS_Type               = 50009,    /* AT_TRAFFIC_CLASS_CUSTOMIZE_STRU */
/* 50056 */     en_NV_Item_SMS_CLASS0_TAILOR                = 50056,    /* MN_MSG_NVIM_CLASS0_TAILOR_STRU */

/*  BUTT */     en_NV_Item_GUCNAS_NV_BUTT,
};


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __NV_ID_GUCNAS_H__ */


