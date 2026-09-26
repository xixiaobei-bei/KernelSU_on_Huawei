/* MD5: 2fc4d8ead4beecb53f47daba0731a1d9*/
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

#if !defined(__PRODUCT_CONFIG_GUCAS_H__)
#define __PRODUCT_CONFIG_GUCAS_H__

#ifndef FEATURE_ON
#define FEATURE_ON 1 
#endif 

#ifndef FEATURE_OFF
#define FEATURE_OFF 0 
#endif 

#ifndef FEATURE_BBPMST_FREQUENCY_REDUCE
#define FEATURE_BBPMST_FREQUENCY_REDUCE FEATURE_OFF 
#endif 

#ifndef FEATURE_CSD
#define FEATURE_CSD FEATURE_OFF 
#endif 

#ifndef FEATURE_CST_ASYN_OR_NOT_TRANS
#define FEATURE_CST_ASYN_OR_NOT_TRANS FEATURE_OFF 
#endif 

#ifndef FEATURE_HARDWARE_HDLC_FUNC
#define FEATURE_HARDWARE_HDLC_FUNC FEATURE_OFF 
#endif 

#ifndef FEATURE_MMU_MEDIUM
#define FEATURE_MMU_MEDIUM FEATURE_ON 
#endif 

#ifndef FEATURE_NFEXT
#define FEATURE_NFEXT FEATURE_ON 
#endif 

#ifndef FEATURE_PPP
#define FEATURE_PPP FEATURE_ON 
#endif 

#ifndef FEATURE_SMALL_MAILBOX
#define FEATURE_SMALL_MAILBOX FEATURE_OFF 
#endif 

#ifndef FEATURE_TTF_MEM_DEBUG
#define FEATURE_TTF_MEM_DEBUG FEATURE_ON 
#endif 

#ifndef FEATURE_VISP_VPP
#define FEATURE_VISP_VPP FEATURE_ON 
#endif 

#ifndef VISP_IMS_NIC_CNT
#define VISP_IMS_NIC_CNT 8 
#endif 

#ifndef TTF_CF_FEATURE
#define TTF_CF_FEATURE FEATURE_OFF 
#endif 

#ifndef FEATURE_DSDS
#define FEATURE_DSDS FEATURE_ON 
#endif 

#ifndef FEATURE_FIX_DSDS
#define FEATURE_FIX_DSDS 
#endif 

#ifndef GAS_PTL_VER_PRE_R99
#define GAS_PTL_VER_PRE_R99 (PS_PTL_VER_PRE_R99) 
#endif 

#ifndef GAS_PTL_VER_R99
#define GAS_PTL_VER_R99 (PS_PTL_VER_R99) 
#endif 

#ifndef GAS_PTL_VER_R3
#define GAS_PTL_VER_R3 (PS_PTL_VER_R3) 
#endif 

#ifndef GAS_PTL_VER_R4
#define GAS_PTL_VER_R4 (PS_PTL_VER_R4) 
#endif 

#ifndef GAS_PTL_VER_R5
#define GAS_PTL_VER_R5 (PS_PTL_VER_R5) 
#endif 

#ifndef GAS_PTL_VER_R6
#define GAS_PTL_VER_R6 (PS_PTL_VER_R6) 
#endif 

#ifndef GAS_PTL_VER_R7
#define GAS_PTL_VER_R7 (PS_PTL_VER_R7) 
#endif 

#ifndef GAS_PTL_VER_R8
#define GAS_PTL_VER_R8 (PS_PTL_VER_R8) 
#endif 

#ifndef GAS_PTL_VER_R9
#define GAS_PTL_VER_R9 (PS_PTL_VER_R9) 
#endif 

#ifndef GAS_UE_REL_VER
#define GAS_UE_REL_VER (GAS_PTL_VER_R9) 
#endif 

#ifndef GSM_ENCODIX_VER_083
#define GSM_ENCODIX_VER_083 0 
#endif 

#ifndef GSM_ENCODIX_VER_186
#define GSM_ENCODIX_VER_186 1 
#endif 

#ifndef GSM_ENCODIX_VER
#define GSM_ENCODIX_VER GSM_ENCODIX_VER_186 
#endif 

#ifndef FEATURE_DC_MIMO
#define FEATURE_DC_MIMO FEATURE_OFF 
#endif 

#ifndef FEATURE_DC_UPA
#define FEATURE_DC_UPA FEATURE_OFF 
#endif 

#ifndef FEATURE_DL_E_CELL_FACH
#define FEATURE_DL_E_CELL_FACH FEATURE_ON 
#endif 

#ifndef FEATURE_UL_E_CELL_FACH
#define FEATURE_UL_E_CELL_FACH FEATURE_ON 
#endif 

#ifndef FEATURE_MODEM1_SUPPORT_WCDMA
#define FEATURE_MODEM1_SUPPORT_WCDMA FEATURE_ON 
#endif 

#ifndef FEATURE_HARDWARE_HDLC_ON_CCPU
#define FEATURE_HARDWARE_HDLC_ON_CCPU FEATURE_ON 
#endif 

#ifndef FEATURE_UE_MODE_CDMA	
#define FEATURE_UE_MODE_CDMA	 FEATURE_ON 
#endif 

#ifndef FEATURE_CPROC_INT_OPTIMIZE
#define FEATURE_CPROC_INT_OPTIMIZE FEATURE_OFF 
#endif 

#ifndef FEATURE_BASTET
#define FEATURE_BASTET FEATURE_ON 
#endif 

#ifndef GU_PRODUCT_VERSION
#define GU_PRODUCT_VERSION VERSION_V7R1 
#endif 

#ifndef FEATURE_GUAS_REL_IND_RECONSTRUCT
#define FEATURE_GUAS_REL_IND_RECONSTRUCT FEATURE_ON 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
