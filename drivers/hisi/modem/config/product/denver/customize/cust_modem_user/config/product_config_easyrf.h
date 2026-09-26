/* MD5: 5b2789253b28fbeac36d777727d1a519*/
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

#if !defined(__PRODUCT_CONFIG_EASYRF_H__)
#define __PRODUCT_CONFIG_EASYRF_H__

#ifndef RFDSP_HARDMSG_M536
#define RFDSP_HARDMSG_M536 
#endif 

#ifndef RFIC_BOARD_TYPE_FPGA
#define RFIC_BOARD_TYPE_FPGA 0 
#endif 

#ifndef RFIC_BOARD_TYPE_SFT
#define RFIC_BOARD_TYPE_SFT 1 
#endif 

#ifndef RFIC_BOARD_TYPE_ASIC
#define RFIC_BOARD_TYPE_ASIC 2 
#endif 

#ifndef RFIC_BOARD_TYPE_SDAT
#define RFIC_BOARD_TYPE_SDAT 3 
#endif 

#ifndef RFIC_BOARD_TYPE_PORTING
#define RFIC_BOARD_TYPE_PORTING 4 
#endif 

#ifndef RFIC_CHIP_TYPE_6365
#define RFIC_CHIP_TYPE_6365 2 
#endif 

#ifndef RFIC_CHIP_TYPE_6370
#define RFIC_CHIP_TYPE_6370 3 
#endif 

#ifndef RFIC_CHIP_TYPE_6365CS
#define RFIC_CHIP_TYPE_6365CS 4 
#endif 

#ifndef RFIC_CHIP_TYPE_6370CS
#define RFIC_CHIP_TYPE_6370CS 5 
#endif 

#ifndef RFIC_SUB6G_XTENSA_CORE_SYSTEM
#define RFIC_SUB6G_XTENSA_CORE_SYSTEM "RG-2017.7" 
#endif 

#ifndef RFIC_SUB6G_XTENSA_CORE_CACHE
#define RFIC_SUB6G_XTENSA_CORE_CACHE Hi6365V100_Xten_fusion_P2 
#endif 

#ifndef RFIC_SUB6G_LSP_SCRIPT_PATH
#define RFIC_SUB6G_LSP_SCRIPT_PATH "6365cs" 
#endif 

#ifndef RFIC_SUB6G_CHIP_TYPE_PATH
#define RFIC_SUB6G_CHIP_TYPE_PATH "hi6365cs" 
#endif 

#ifndef RFIC_SUB6G_CHIP_TYPE
#define RFIC_SUB6G_CHIP_TYPE RFIC_CHIP_TYPE_6365CS 
#endif 

#ifndef RFIC_SUB6G_BOARD_TYPE
#define RFIC_SUB6G_BOARD_TYPE RFIC_BOARD_TYPE_ASIC 
#endif 

#ifndef RFIC_HF_XTENSA_CORE_SYSTEM
#define RFIC_HF_XTENSA_CORE_SYSTEM "RG-2017.7" 
#endif 

#ifndef RFIC_HF_XTENSA_CORE_CACHE
#define RFIC_HF_XTENSA_CORE_CACHE Hi6365V100_Xten_fusion_P2 
#endif 

#ifndef RFIC_HF_LSP_SCRIPT_PATH
#define RFIC_HF_LSP_SCRIPT_PATH "6370cs" 
#endif 

#ifndef RFIC_HF_CHIP_TYPE_PATH
#define RFIC_HF_CHIP_TYPE_PATH "hi6370cs" 
#endif 

#ifndef RFIC_HF_CHIP_TYPE
#define RFIC_HF_CHIP_TYPE RFIC_CHIP_TYPE_6370CS 
#endif 

#ifndef RFIC_HF_BOARD_TYPE
#define RFIC_HF_BOARD_TYPE RFIC_BOARD_TYPE_ASIC 
#endif 

#ifndef RFIC_BOARD_TYPE
#define RFIC_BOARD_TYPE RFIC_BOARD_TYPE_ASIC 
#endif 

#ifndef NXP_COMMON_STARTUP_DTCM_BASE
#define NXP_COMMON_STARTUP_DTCM_BASE 0xe3457700 
#endif 

#ifndef FEATURE_TWO_RFDSP_BIN
#define FEATURE_TWO_RFDSP_BIN FEATURE_OFF 
#endif 

#ifndef FEATURE_SUPPORT_RFIC1
#define FEATURE_SUPPORT_RFIC1 FEATURE_OFF 
#endif 

#ifndef FEATURE_EASYRF
#define FEATURE_EASYRF FEATURE_ON 
#endif 

#ifndef RFDSP_TRACE_MEM_SIZE
#define RFDSP_TRACE_MEM_SIZE 2048 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
