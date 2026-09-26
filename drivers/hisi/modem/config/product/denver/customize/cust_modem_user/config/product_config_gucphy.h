/* MD5: 49db46fb9176aeab4c8b759ca5cd1d1d*/
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

#if !defined(__PRODUCT_CONFIG_GUCPHY_H__)
#define __PRODUCT_CONFIG_GUCPHY_H__

#ifndef BBIC_CHIP_ES
#define BBIC_CHIP_ES 
#endif 

#ifndef ZSP_DSP_CHIP_BB_TYPE		
#define ZSP_DSP_CHIP_BB_TYPE		 54 
#endif 

#ifndef NV_VERSION
#define NV_VERSION denver 
#endif 

#ifndef ZSP_DSP_PRODUCT_FORM		
#define ZSP_DSP_PRODUCT_FORM		 4 
#endif 

#ifndef BOARD
#define BOARD ASIC 
#endif 

#ifndef UPHY_BOARD_TYPE
#define UPHY_BOARD_TYPE 2 
#endif 

#ifndef FEATURE_TEMP_MULTI_MODE_LP
#define FEATURE_TEMP_MULTI_MODE_LP FEATURE_ON 
#endif 

#ifndef FEATURE_GSM_SDR
#define FEATURE_GSM_SDR FEATURE_ON 
#endif 

#ifndef FEATURE_GSM_SDR_DAIC
#define FEATURE_GSM_SDR_DAIC FEATURE_ON 
#endif 

#ifndef FEATURE_GSM_WHOLE_SDR		
#define FEATURE_GSM_WHOLE_SDR		 FEATURE_ON 
#endif 

#ifndef FEATURE_XBBE16_NEW_MAIL		
#define FEATURE_XBBE16_NEW_MAIL		 FEATURE_ON 
#endif 

#ifndef FEATURE_SRAM_400K
#define FEATURE_SRAM_400K FEATURE_OFF 
#endif 

#ifndef FEATURE_EXTERNAL_32K_CLK		
#define FEATURE_EXTERNAL_32K_CLK		 FEATURE_ON 
#endif 

#ifndef FEATURE_SOCP_ON_DEMAND			
#define FEATURE_SOCP_ON_DEMAND			 FEATURE_OFF 
#endif 

#ifndef FEATURE_TAS				
#define FEATURE_TAS				 FEATURE_ON 
#endif 

#ifdef EMU_TYPE_ESLPS 
#ifndef FEATURE_NPHY_ESLPS
#define FEATURE_NPHY_ESLPS 
#endif 

#endif
#ifdef EMU_TYPE_ESL 
#ifndef FEATURE_NPHY_ESL
#define FEATURE_NPHY_ESL 
#endif 

#endif
#ifndef FEATURE_VIRTUAL_BAND			
#define FEATURE_VIRTUAL_BAND			 FEATURE_ON 
#endif 

#ifndef FEATURE_HI6363                		
#define FEATURE_HI6363                		 FEATURE_ON 
#endif 

#ifndef FEATURE_RTT_TEST
#define FEATURE_RTT_TEST FEATURE_OFF 
#endif 

#ifndef FEATURE_RTT_RANDOM_TEST
#define FEATURE_RTT_RANDOM_TEST FEATURE_OFF 
#endif 

#ifndef FEATURE_GUTLC_ONE_DSP
#define FEATURE_GUTLC_ONE_DSP FEATURE_ON 
#endif 

#ifndef FEATURE_NX_CORE_OPEN
#define FEATURE_NX_CORE_OPEN FEATURE_ON 
#endif 

#ifndef FEATURE_CSDR
#define FEATURE_CSDR FEATURE_ON 
#endif 

#ifndef FEATURE_HITUNE
#define FEATURE_HITUNE FEATURE_OFF 
#endif 

#ifndef CPHY_PUB_DTCM_GLB_MINUS_LOCAL	
#define CPHY_PUB_DTCM_GLB_MINUS_LOCAL	 0x00800000 
#endif 

#ifndef GUC_RTT_TEST_ENABLED
#endif 

#ifndef PHY_CBT_ENABLED
#endif 

#ifndef MODEM_GUPHY_LTO
#define MODEM_GUPHY_LTO 
#endif 

#ifndef UPHY_NSCS_CTRL
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
