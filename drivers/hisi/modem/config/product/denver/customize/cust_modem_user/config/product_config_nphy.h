/* MD5: e47ac904fbd90539a8f8fd48f199aead*/
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

#if !defined(__PRODUCT_CONFIG_NPHY_H__)
#define __PRODUCT_CONFIG_NPHY_H__

#ifndef UPHY_CHIP_TYPE_PATH
#define UPHY_CHIP_TYPE_PATH cs 
#endif 

#ifdef OBB_TEN_LLT_MODE 
#ifndef XTENSA_CORE_NR
#define XTENSA_CORE_NR BB5000_NX 
#endif 

#else
#ifndef XTENSA_CORE_NR
#define XTENSA_CORE_NR BB5000v2_NX 
#endif 

#endif
#ifndef XTENSA_SYSTEM_NR
#define XTENSA_SYSTEM_NR "$(ROOT_XTENSA_PATH_W)/XtDevTools/install/builds/RH-2018.5-linux/$(CFG_XTENSA_CORE_NR)/config" 
#endif 

#ifndef TENSILICA_BUILDS_NR
#define TENSILICA_BUILDS_NR "$(ROOT_XTENSA_PATH_W)/XtDevTools/install/builds/RH-2018.5-linux" 
#endif 

#ifndef TENSILICA_TOOLS_NR
#define TENSILICA_TOOLS_NR "$(ROOT_XTENSA_PATH_W)/XtDevTools/install/tools/RH-2018.5-linux" 
#endif 

#ifndef XTENSA_PREDICT_BUG
#define XTENSA_PREDICT_BUG 
#endif 

#ifndef NPHY_DTCM_BASE
#define NPHY_DTCM_BASE 0xE3400000 
#endif 

#ifndef NPHY_ITCM_BASE
#define NPHY_ITCM_BASE 0xE3500000 
#endif 

#ifndef NPHY_DTCM_SIZE
#define NPHY_DTCM_SIZE 0x80000 
#endif 

#ifndef NPHY_ITCM_SIZE
#define NPHY_ITCM_SIZE 0x0 
#endif 

#ifndef NPHY_HL1C_DTCM_SIZE
#define NPHY_HL1C_DTCM_SIZE 0x80000 
#endif 

#ifndef NPHY_LL1D_DTCM_SIZE
#define NPHY_LL1D_DTCM_SIZE 0x80000 
#endif 

#ifndef NPHY_LL1U_DTCM_SIZE
#define NPHY_LL1U_DTCM_SIZE 0x80000 
#endif 

#ifndef NPHY_SDR_DTCM_SIZE
#define NPHY_SDR_DTCM_SIZE 0x80000 
#endif 

#ifndef NPHY_L2M_BASE
#define NPHY_L2M_BASE 0xE3600000 
#endif 

#ifndef NPHY_L2M_SIZE
#define NPHY_L2M_SIZE 0x0 
#endif 

#ifndef NPHY_L2C_BASE
#define NPHY_L2C_BASE 0xE3680000 
#endif 

#ifndef NPHY_L2C_SIZE
#define NPHY_L2C_SIZE 0x0 
#endif 

#ifndef NPHY_MMB_BASE
#define NPHY_MMB_BASE 0xF6800000 
#endif 

#ifndef NPHY_MMB_SIZE
#define NPHY_MMB_SIZE 0x200000 
#endif 

#ifndef NPHY_MMB_CODE_BASE
#define NPHY_MMB_CODE_BASE 0xF6800000 
#endif 

#ifndef NPHY_MMB_CODE_SIZE
#define NPHY_MMB_CODE_SIZE 0x199000 
#endif 

#ifndef NPHY_MMB_DATA_BASE
#define NPHY_MMB_DATA_BASE ((NPHY_MMB_CODE_BASE)+(NPHY_MMB_CODE_SIZE)) 
#endif 

#ifndef NPHY_MMB_DATA_SIZE
#define NPHY_MMB_DATA_SIZE ((NPHY_MMB_SIZE)-(NPHY_MMB_CODE_SIZE)) 
#endif 

#ifndef NPHY_MMB_COMM_SDATA_BASE
#define NPHY_MMB_COMM_SDATA_BASE ((NPHY_MMB_DATA_BASE)) 
#endif 

#ifndef NPHY_MMB_COMM_SDATA_SIZE
#define NPHY_MMB_COMM_SDATA_SIZE 0x4000 
#endif 

#ifndef NPHY_MMB_USER_SDATA_BASE
#define NPHY_MMB_USER_SDATA_BASE ((NPHY_MMB_COMM_SDATA_BASE)+(NPHY_MMB_COMM_SDATA_SIZE)) 
#endif 

#ifndef NPHY_MMB_USER_SDATA_SIZE
#define NPHY_MMB_USER_SDATA_SIZE 0x63000 
#endif 

#ifndef NPHY_MMB_USER_NC_SDATA_BASE
#define NPHY_MMB_USER_NC_SDATA_BASE ((NPHY_MMB_USER_SDATA_BASE)) 
#endif 

#ifndef NPHY_MMB_USER_NC_SDATA_SIZE
#define NPHY_MMB_USER_NC_SDATA_SIZE 0x9000 
#endif 

#ifndef NPHY_MMB_USER_EC_SDATA_BASE
#define NPHY_MMB_USER_EC_SDATA_BASE ((NPHY_MMB_USER_NC_SDATA_BASE)+(NPHY_MMB_USER_NC_SDATA_SIZE)) 
#endif 

#ifndef NPHY_MMB_USER_EC_SDATA_SIZE
#define NPHY_MMB_USER_EC_SDATA_SIZE 0x5A000 
#endif 

#ifndef NPHY_BOOT_REMAP_OFFSET
#define NPHY_BOOT_REMAP_OFFSET 0x2000 
#endif 

#ifdef EMU_TYPE_EMU 
#ifndef NPHY_SRAM_BASE
#define NPHY_SRAM_BASE 0x20000 
#endif 

#ifndef NPHY_SRAM_SIZE
#define NPHY_SRAM_SIZE 0xe0000 
#endif 

#ifndef NPHY_DDR_BASE
#define NPHY_DDR_BASE ((NPHY_SRAM_BASE)+(NPHY_SRAM_SIZE)) 
#endif 

#ifndef NPHY_DDR_SIZE
#define NPHY_DDR_SIZE 0x20000000 
#endif 

#ifndef NPHY_DDR_NX_IMAGE_BASE
#define NPHY_DDR_NX_IMAGE_BASE ((NPHY_DDR_BASE)) 
#endif 

#ifndef NPHY_DDR_NX_IMAGE_SIZE
#define NPHY_DDR_NX_IMAGE_SIZE 0x2000000 
#endif 

#ifndef NPHY_DDR_NX_BOOT_BASE
#define NPHY_DDR_NX_BOOT_BASE ((NPHY_DDR_NX_IMAGE_BASE)+(NPHY_BOOT_REMAP_OFFSET)) 
#endif 

#ifndef NPHY_DDR_PDE_IMAGE_BASE
#define NPHY_DDR_PDE_IMAGE_BASE ((NPHY_DDR_NX_IMAGE_BASE)+(NPHY_DDR_NX_IMAGE_SIZE)) 
#endif 

#ifndef NPHY_DDR_PDE_IMAGE_SIZE
#define NPHY_DDR_PDE_IMAGE_SIZE 0x1000000 
#endif 

#ifndef NPHY_DDR_CSI_IMAGE_BASE
#define NPHY_DDR_CSI_IMAGE_BASE ((NPHY_DDR_PDE_IMAGE_BASE)+(NPHY_DDR_PDE_IMAGE_SIZE)) 
#endif 

#ifndef NPHY_DDR_CSI_IMAGE_SIZE
#define NPHY_DDR_CSI_IMAGE_SIZE 0x800000 
#endif 

#ifndef NPHY_DDR_LINK_BASE
#define NPHY_DDR_LINK_BASE ((NPHY_DDR_CSI_IMAGE_BASE)+(NPHY_DDR_CSI_IMAGE_SIZE)) 
#endif 

#ifndef NPHY_DDR_SCRATCH_BASE
#define NPHY_DDR_SCRATCH_BASE ((NPHY_DDR_LINK_BASE)+(NPHY_DDR_LINK_SIZE)) 
#endif 

#ifndef NPHY_DDR_SCRATCH_SIZE
#define NPHY_DDR_SCRATCH_SIZE 0x10000000 
#endif 

#ifndef NPHY_DDR_SOCP_STUB_BUFF_BASE
#define NPHY_DDR_SOCP_STUB_BUFF_BASE ((NPHY_DDR_SCRATCH_BASE)+(NPHY_DDR_SCRATCH_SIZE)) 
#endif 

#ifndef NPHY_DDR_SOCP_STUB_BUFF_SIZE
#define NPHY_DDR_SOCP_STUB_BUFF_SIZE 0x800000 
#endif 

#else
#ifndef NPHY_SRAM_BASE
#define NPHY_SRAM_BASE 0 
#endif 

#ifndef NPHY_SRAM_SIZE
#define NPHY_SRAM_SIZE 0 
#endif 

#ifndef NPHY_DDR_BASE
#define NPHY_DDR_BASE ((DDR_NRPHY_IMAGE_ADDR)) 
#endif 

#ifndef NPHY_DDR_SIZE
#define NPHY_DDR_SIZE ((DDR_NRPHY_IMAGE_SIZE)) 
#endif 

#ifndef NPHY_DDR_NX_IMAGE_BASE
#define NPHY_DDR_NX_IMAGE_BASE ((NPHY_DDR_BASE)) 
#endif 

#ifndef NPHY_DDR_NX_IMAGE_SIZE
#define NPHY_DDR_NX_IMAGE_SIZE ((NPHY_DDR_SIZE)-(NPHY_DDR_LINK_SIZE)) 
#endif 

#ifndef NPHY_DDR_NX_BOOT_BASE
#define NPHY_DDR_NX_BOOT_BASE ((NPHY_DDR_NX_IMAGE_BASE)+(NPHY_BOOT_REMAP_OFFSET)) 
#endif 

#ifndef NPHY_DDR_LINK_BASE
#define NPHY_DDR_LINK_BASE ((NPHY_DDR_NX_IMAGE_BASE)+(NPHY_DDR_NX_IMAGE_SIZE)) 
#endif 

#ifndef NPHY_DDR_SCRATCH_BASE
#define NPHY_DDR_SCRATCH_BASE ((DDR_SDR_NR_ADDR)) 
#endif 

#ifndef NPHY_DDR_SCRATCH_SIZE
#define NPHY_DDR_SCRATCH_SIZE ((DDR_SDR_NR_SIZE)) 
#endif 

#endif
#ifndef NPHY_DDR_LINK_SIZE
#define NPHY_DDR_LINK_SIZE 0x200000 
#endif 

#ifndef NPHY_DDR_LINK_NC_SIZE
#define NPHY_DDR_LINK_NC_SIZE 0x100000 
#endif 

#ifndef NPHY_DDR_LINK_EC_SIZE
#define NPHY_DDR_LINK_EC_SIZE ((NPHY_DDR_LINK_SIZE)-(NPHY_DDR_LINK_NC_SIZE)) 
#endif 

#ifndef NPHY_DDR_USER_NC_SDATA_BASE
#define NPHY_DDR_USER_NC_SDATA_BASE ((NPHY_DDR_LINK_BASE)) 
#endif 

#ifndef NPHY_DDR_USER_NC_SDATA_SIZE
#define NPHY_DDR_USER_NC_SDATA_SIZE 0x100000 
#endif 

#ifndef NPHY_TOTAL_IMG_SIZE
#define NPHY_TOTAL_IMG_SIZE ((NPHY_DTCM_SIZE)+(NPHY_ITCM_SIZE)+(NPHY_L2M_SIZE)+(NPHY_DDR_SIZE)) 
#endif 

#ifndef NPHY_FEATURE_HF_SWITCH
#define NPHY_FEATURE_HF_SWITCH FEATURE_OFF 
#endif 

#ifndef UPHY_DESENSITIZATION
#endif 

#ifndef NPHY_OM_RPT_ENABLED
#endif 

#ifndef NPHY_DESENSITIZATION_OFF
#endif 

#ifndef TL_RTT_TEST_ENABLED
#endif 

#ifndef NPHY_ASSERT_ENABLED
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
