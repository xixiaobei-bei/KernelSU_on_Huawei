/* MD5: 281299afc9d14f76c86e1e3a2e286d52*/
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

#if !defined(__PRODUCT_CONFIG_PAM_H__)
#define __PRODUCT_CONFIG_PAM_H__

#ifndef BBP_MASTER_NONE
#define BBP_MASTER_NONE 0 
#endif 

#ifndef BBP_MASTER_VER1
#define BBP_MASTER_VER1 1 
#endif 

#ifndef BBP_MASTER_VER2
#define BBP_MASTER_VER2 2 
#endif 

#ifndef BBP_MASTER_VER3
#define BBP_MASTER_VER3 3 
#endif 

#ifndef BBP_MASTER_VER4
#define BBP_MASTER_VER4 4 
#endif 

#ifndef BBP_MASTER_VER5
#define BBP_MASTER_VER5 5 
#endif 

#ifndef BBP_MASTER_VER6
#define BBP_MASTER_VER6 6 
#endif 

#ifndef FEATURE_BBP_MASTER_VER
#define FEATURE_BBP_MASTER_VER (BBP_MASTER_VER5) 
#endif 

#ifndef FEATURE_POWER_TIMER
#define FEATURE_POWER_TIMER FEATURE_ON 
#endif 

#ifndef FEATURE_UE_UICC_MULTI_APP_SUPPORT
#define FEATURE_UE_UICC_MULTI_APP_SUPPORT FEATURE_ON 
#endif 

#ifndef FEATURE_VSIM
#define FEATURE_VSIM FEATURE_ON 
#endif 

#ifndef FEATURE_GUC_BBP_TRIG
#define FEATURE_GUC_BBP_TRIG FEATURE_ON 
#endif 

#ifndef FEATURE_CSIM_NONCHECK
#define FEATURE_CSIM_NONCHECK FEATURE_OFF 
#endif 

#ifndef FEATURE_GUC_BBP_TRIG_NEWVERSION
#define FEATURE_GUC_BBP_TRIG_NEWVERSION FEATURE_ON 
#endif 

#ifndef FEATURE_VOS_REDUCE_MEM_CFG
#define FEATURE_VOS_REDUCE_MEM_CFG FEATURE_OFF 
#endif 

#ifndef FEATURE_RTC_TIMER_DBG
#define FEATURE_RTC_TIMER_DBG FEATURE_ON 
#endif 

#ifndef FEATURE_PHONE_SC
#define FEATURE_PHONE_SC FEATURE_ON 
#endif 

#ifndef FEATURE_PHONE_USIM
#define FEATURE_PHONE_USIM FEATURE_ON 
#endif 

#ifndef FEATURE_SC_CUST
#define FEATURE_SC_CUST FEATURE_OFF 
#endif 

#ifndef FEATURE_SC_PUBLIC_DATA_KEY_UPDATE
#define FEATURE_SC_PUBLIC_DATA_KEY_UPDATE FEATURE_OFF 
#endif 

#ifndef FEATURE_PIN_CRYPTO_PROTECT
#define FEATURE_PIN_CRYPTO_PROTECT FEATURE_ON 
#endif 

#ifndef FEATURE_MBB_ACORE_64BIT_ENABLE
#define FEATURE_MBB_ACORE_64BIT_ENABLE FEATURE_OFF 
#endif 

#ifndef FEATURE_SC_NETWORK_UPDATE
#define FEATURE_SC_NETWORK_UPDATE FEATURE_OFF 
#endif 

#ifndef FEATURE_SC_RSA_PADDING_UPDATE
#define FEATURE_SC_RSA_PADDING_UPDATE FEATURE_OFF 
#endif 

#ifndef FEATURE_SC_PBKDF2_UPDATE
#define FEATURE_SC_PBKDF2_UPDATE FEATURE_ON 
#endif 

#ifndef FEATURE_VSIM_ICC_SEC_CHANNEL
#define FEATURE_VSIM_ICC_SEC_CHANNEL FEATURE_ON 
#endif 

#ifndef FEATURE_BOSTON_AFTER_FEATURE
#define FEATURE_BOSTON_AFTER_FEATURE FEATURE_ON 
#endif 

#ifndef FEATURE_XSIM
#define FEATURE_XSIM FEATURE_OFF 
#endif 

#ifndef FEATURE_SCI_SWITCH_OPTIMIZE
#define FEATURE_SCI_SWITCH_OPTIMIZE FEATURE_ON 
#endif 

#ifndef FEATURE_VOS_18H_TIMER
#define FEATURE_VOS_18H_TIMER FEATURE_ON 
#endif 

#ifndef FEATURE_SET_C1C2_VALUE
#define FEATURE_SET_C1C2_VALUE FEATURE_ON 
#endif 

#ifndef FEATURE_DEEP_SLEEP_ANTEN_DETECT
#define FEATURE_DEEP_SLEEP_ANTEN_DETECT FEATURE_OFF 
#endif 

#ifndef PAM_ENG_ENABLED
#endif 

#ifndef PAM_CT_ENABLED
#endif 

#ifndef PAM_BT_ENABLED
#endif 

#ifndef FEATURE_SINGLEMODEM_DUALSLOT
#define FEATURE_SINGLEMODEM_DUALSLOT FEATURE_OFF 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
