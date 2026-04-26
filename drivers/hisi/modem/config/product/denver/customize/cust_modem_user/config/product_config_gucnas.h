/* MD5: 14777cbd76e9f7f625332e7f35542f63*/
/*
 * copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#if !defined(__PRODUCT_CONFIG_GUCNAS_H__)
#define __PRODUCT_CONFIG_GUCNAS_H__

#ifndef NAS_NEW_ARCH
#define NAS_NEW_ARCH 
#endif 

#ifndef FEATURE_DSDS_ADAP_FOR_CDMA
#define FEATURE_DSDS_ADAP_FOR_CDMA 
#endif 

#ifndef FEATURE_DATA_SERVICE_NEW_PLATFORM
#define FEATURE_DATA_SERVICE_NEW_PLATFORM FEATURE_ON 
#endif 

#ifndef FEATURE_UE_MODE_G
#define FEATURE_UE_MODE_G FEATURE_ON 
#endif 

#ifndef FEATURE_UE_MODE_W
#define FEATURE_UE_MODE_W FEATURE_ON 
#endif 

#ifndef FEATURE_LTE
#define FEATURE_LTE FEATURE_ON 
#endif 

#ifndef FEATURE_ELTE
#define FEATURE_ELTE FEATURE_OFF 
#endif 

#ifndef FEATURE_UE_MODE_TDS
#define FEATURE_UE_MODE_TDS FEATURE_OFF 
#endif 

#ifndef FEATURE_UE_MODE_NR
#define FEATURE_UE_MODE_NR FEATURE_ON 
#endif 

#ifndef FEATURE_PC_VOICE
#define FEATURE_PC_VOICE FEATURE_OFF 
#endif 

#ifndef FEATURE_GCBS
#define FEATURE_GCBS FEATURE_ON 
#endif 

#ifndef FEATURE_WCBS
#define FEATURE_WCBS FEATURE_ON 
#endif 

#ifndef FEATURE_ETWS
#define FEATURE_ETWS FEATURE_ON 
#endif 

#ifndef FEATURE_AGPS
#define FEATURE_AGPS FEATURE_ON 
#endif 

#ifndef FEATURE_XPDS
#define FEATURE_XPDS FEATURE_OFF 
#endif 

#ifndef FEATURE_TC
#define FEATURE_TC FEATURE_ON 
#endif 

#ifndef FEATURE_AGPS_GPIO
#define FEATURE_AGPS_GPIO FEATURE_OFF 
#endif 

#ifndef FEATRUE_XML_PARSER
#define FEATRUE_XML_PARSER FEATURE_ON 
#endif 

#ifndef NAS_FEATURE_SMS_FLASH_SMSEXIST
#define NAS_FEATURE_SMS_FLASH_SMSEXIST FEATURE_OFF 
#endif 

#ifndef FEATURE_IPV6
#define FEATURE_IPV6 FEATURE_ON 
#endif 

#ifndef FEATURE_RMNET_CUSTOM
#define FEATURE_RMNET_CUSTOM FEATURE_OFF 
#endif 

#ifndef FEATURE_VCOM_EXT
#define FEATURE_VCOM_EXT FEATURE_ON 
#endif 

#ifndef FEATURE_AT_HSIC
#define FEATURE_AT_HSIC FEATURE_OFF 
#endif 

#ifndef FEATURE_AT_HSUART
#define FEATURE_AT_HSUART FEATURE_OFF 
#endif 

#ifndef FEATURE_CL_INTERWORK
#define FEATURE_CL_INTERWORK FEATURE_OFF 
#endif 

#ifndef FEATURE_MULTI_MODEM
#define FEATURE_MULTI_MODEM FEATURE_ON 
#endif 

#ifndef MULTI_MODEM_NUMBER
#define MULTI_MODEM_NUMBER 2 
#endif 

#ifndef FEATURE_ECALL
#define FEATURE_ECALL FEATURE_OFF 
#endif 

#ifndef FEATURE_E5
#define FEATURE_E5 FEATURE_OFF 
#endif 

#ifndef FEATURE_POWER_ON_OFF
#define FEATURE_POWER_ON_OFF FEATURE_OFF 
#endif 

#ifndef FEATURE_SECURITY_SHELL          			
#define FEATURE_SECURITY_SHELL          			 FEATURE_ON 
#endif 

#ifndef FEATURE_WIFI
#define FEATURE_WIFI FEATURE_OFF 
#endif 

#ifndef FEATURE_HUAWEI_VP
#define FEATURE_HUAWEI_VP FEATURE_OFF 
#endif 

#ifndef PS_PTL_VER_PRE_R99
#define PS_PTL_VER_PRE_R99 (-2) 
#endif 

#ifndef PS_PTL_VER_R99
#define PS_PTL_VER_R99 (-1) 
#endif 

#ifndef PS_PTL_VER_R3
#define PS_PTL_VER_R3 0 
#endif 

#ifndef PS_PTL_VER_R4
#define PS_PTL_VER_R4 1 
#endif 

#ifndef PS_PTL_VER_R5
#define PS_PTL_VER_R5 2 
#endif 

#ifndef PS_PTL_VER_R6
#define PS_PTL_VER_R6 3 
#endif 

#ifndef PS_PTL_VER_R7
#define PS_PTL_VER_R7 4 
#endif 

#ifndef PS_PTL_VER_R8
#define PS_PTL_VER_R8 5 
#endif 

#ifndef PS_PTL_VER_R9
#define PS_PTL_VER_R9 6 
#endif 

#ifndef PS_UE_REL_VER
#define PS_UE_REL_VER PS_PTL_VER_R9 
#endif 

#ifndef FEATURE_PROBE_FREQLOCK
#define FEATURE_PROBE_FREQLOCK FEATURE_ON 
#endif 

#ifndef FEATURE_CHINA_TELECOM_VOICE_ENCRYPT
#define FEATURE_CHINA_TELECOM_VOICE_ENCRYPT FEATURE_OFF 
#endif 

#ifndef FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE
#define FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE FEATURE_OFF 
#endif 

#ifndef FEATURE_DX_SECBOOT
#define FEATURE_DX_SECBOOT FEATURE_OFF 
#endif 

#ifndef FEATURE_EDA_SUPPORT
#define FEATURE_EDA_SUPPORT FEATURE_ON 
#endif 

#ifndef FEATURE_MBB_HSRCELLINFO
#define FEATURE_MBB_HSRCELLINFO FEATURE_OFF 
#endif 

#ifndef FEATURE_RNIC_NAPI_GRO
#define FEATURE_RNIC_NAPI_GRO FEATURE_ON 
#endif 

#ifndef FEATURE_LOGCAT_SINGLE_CHANNEL
#define FEATURE_LOGCAT_SINGLE_CHANNEL FEATURE_OFF 
#endif 

#ifndef FEATURE_AT_NV_WRITE_SUPPORT
#endif 

#ifndef FEATURE_READ_LOG_FILTER_CFG
#endif 

#ifndef FEATURE_MEMCPY_REBOOT_CFG
#define FEATURE_MEMCPY_REBOOT_CFG FEATURE_OFF 
#endif 

#ifndef FEATURE_DEBUG_APP_PORT
#define FEATURE_DEBUG_APP_PORT FEATURE_OFF 
#endif 

#ifndef FEATURE_PHONE_ENG_AT_CMD
#define FEATURE_PHONE_ENG_AT_CMD FEATURE_OFF 
#endif 

#ifndef FEATURE_DCXO_HI1102_SAMPLE_SHARE
#define FEATURE_DCXO_HI1102_SAMPLE_SHARE FEATURE_OFF 
#endif 

#ifndef FEATURE_NRNAS_SHARE_MEM
#define FEATURE_NRNAS_SHARE_MEM FEATURE_ON 
#endif 

#ifdef EMU_TYPE_ESL 
#ifndef FEATURE_NRNAS_ESL
#define FEATURE_NRNAS_ESL FEATURE_ON 
#endif 

#else
#ifndef FEATURE_NRNAS_ESL
#define FEATURE_NRNAS_ESL FEATURE_OFF 
#endif 

#endif
#ifndef FEATURE_MM_TASK_COMBINE
#define FEATURE_MM_TASK_COMBINE FEATURE_OFF 
#endif 

#ifndef FEATURE_SND_NRMM_MSG_TYPE
#define FEATURE_SND_NRMM_MSG_TYPE FEATURE_ON 
#endif 

#ifndef FEATURE_LTEV
#define FEATURE_LTEV FEATURE_OFF 
#endif 

#ifndef FEATURE_LTEV_WL
#define FEATURE_LTEV_WL FEATURE_OFF 
#endif 

#ifndef FEATURE_PC5_DATA_CHANNEL
#define FEATURE_PC5_DATA_CHANNEL FEATURE_OFF 
#endif 

#ifndef FEATURE_CW_AT_AUTH_PROTECT_CFG
#define FEATURE_CW_AT_AUTH_PROTECT_CFG FEATURE_OFF 
#endif 

#ifndef MODEM_LTO
#define MODEM_LTO 
#endif 

#ifndef FEATURE_DEBUG_PRINT_ADDRESS
#define FEATURE_DEBUG_PRINT_ADDRESS FEATURE_OFF 
#endif 

#ifndef FEATURE_ACPU_PHONE_MODE
#define FEATURE_ACPU_PHONE_MODE FEATURE_ON 
#endif 

#ifndef FEATURE_CCORE_RESET_RPT_BY_FILE_NODE
#define FEATURE_CCORE_RESET_RPT_BY_FILE_NODE FEATURE_OFF 
#endif 

#ifndef FEATURE_ACORE_MODULE_TO_CCORE
#define FEATURE_ACORE_MODULE_TO_CCORE FEATURE_OFF 
#endif 

#ifndef FEATURE_AT_PROXY
#define FEATURE_AT_PROXY FEATURE_OFF 
#endif 

#ifndef FEATURE_SHARE_APN
#define FEATURE_SHARE_APN FEATURE_OFF 
#endif 

#ifndef FEATURE_SHARE_PDP
#define FEATURE_SHARE_PDP FEATURE_OFF 
#endif 

#ifndef FEATURE_MULTI_NCM
#define FEATURE_MULTI_NCM FEATURE_OFF 
#endif 

#ifndef FEATURE_IOT_CMUX
#define FEATURE_IOT_CMUX FEATURE_OFF 
#endif 

#ifndef FEATURE_IOT_ATNLPROXY
#define FEATURE_IOT_ATNLPROXY FEATURE_OFF 
#endif 

#ifndef FEATURE_RNIC_PCIE_EP
#endif 

#ifndef FEATURE_MBB_CUST
#define FEATURE_MBB_CUST FEATURE_OFF 
#endif 

#ifndef FEATURE_MBB_HOTINOUT
#define FEATURE_MBB_HOTINOUT FEATURE_OFF 
#endif 

#ifndef FEATURE_AT_CMD_TRUST_LIST
#define FEATURE_AT_CMD_TRUST_LIST FEATURE_OFF 
#endif 

#ifndef FEATURE_IOT_UART_CUST
#define FEATURE_IOT_UART_CUST FEATURE_OFF 
#endif 

#ifndef FEATURE_MT_CALL_SMS_WAKELOCK
#define FEATURE_MT_CALL_SMS_WAKELOCK FEATURE_OFF 
#endif 

#ifndef FEATURE_IOT_RAW_DATA
#define FEATURE_IOT_RAW_DATA FEATURE_OFF 
#endif 

#ifndef FEATURE_UART_BAUDRATE_AUTO_ADAPTION
#define FEATURE_UART_BAUDRATE_AUTO_ADAPTION FEATURE_OFF 
#endif 

#ifndef FEATURE_ASSOCIATED_CARD_LOCK
#define FEATURE_ASSOCIATED_CARD_LOCK FEATURE_OFF 
#endif 

#ifndef FEATURE_SPM_DOMAIN_SELECT_NEW_PLATFORM
#define FEATURE_SPM_DOMAIN_SELECT_NEW_PLATFORM FEATURE_OFF 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
