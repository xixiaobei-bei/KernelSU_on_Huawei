/* MD5: 19ee21dce2fbdc25d891c3464aca4d1e*/
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

#if !defined(__PRODUCT_CONFIG_LR_PHY_H__)
#define __PRODUCT_CONFIG_LR_PHY_H__

#ifndef XTENSA_CORE_X_CACHE
#define XTENSA_CORE_X_CACHE Phoenix_NX 
#endif 

#ifndef XTENSA_CORE_X_SYSTEM
#define XTENSA_CORE_X_SYSTEM "RH-2018.5" 
#endif 

#ifndef XTENSA_CORE
#define XTENSA_CORE "$(CFG_XTENSA_CORE_X_CACHE)" 
#endif 

#ifndef XTENSA_SYSTEM
#define XTENSA_SYSTEM "$(ROOT_XTENSA_PATH_W)/XtDevTools/install/builds/$(CFG_XTENSA_CORE_X_SYSTEM)/$(CFG_XTENSA_CORE)/config" 
#endif 

#ifndef TENSILICA_BUILDS
#define TENSILICA_BUILDS "$(ROOT_XTENSA_PATH_W)/XtDevTools/install/builds/$(CFG_XTENSA_CORE_X_SYSTEM)" 
#endif 

#ifndef TENSILICA_TOOLS
#define TENSILICA_TOOLS "$(ROOT_XTENSA_PATH_W)/XtDevTools/install/tools/$(CFG_XTENSA_CORE_X_SYSTEM)" 
#endif 

#ifndef XTENSA_PREDICT_BUG
#define XTENSA_PREDICT_BUG 
#endif 

#ifndef XTENSA_INST_PREFETCH_BUG
#endif 

#ifndef XTENSA_DATA_PREFETCH_BUG
#endif 

#ifndef LPHY_DTCM_BASE
#define LPHY_DTCM_BASE 0xE3400000 
#endif 

#ifndef LPHY_ITCM_BASE
#define LPHY_ITCM_BASE 0xE3500000 
#endif 

#ifndef LPHY_DTCM_SIZE
#define LPHY_DTCM_SIZE 0xc0000 
#endif 

#ifndef LPHY_ITCM_SIZE
#define LPHY_ITCM_SIZE 0x60000 
#endif 

#ifndef LPHY_L2M_BASE
#define LPHY_L2M_BASE 0xE36C0000 
#endif 

#ifndef LPHY_L2M_SIZE
#define LPHY_L2M_SIZE 0xc0000 
#endif 

#ifndef LPHY_L2C_BASE
#define LPHY_L2C_BASE 0xE3780000 
#endif 

#ifndef LPHY_L2C_SIZE
#define LPHY_L2C_SIZE 0x80000 
#endif 

#ifndef LPHY_SRAM_BASE
#define LPHY_SRAM_BASE 0x20000 
#endif 

#ifndef LPHY_SRAM_SIZE
#define LPHY_SRAM_SIZE 0xe0000 
#endif 

#ifndef LPHY_DDR_BASE
#define LPHY_DDR_BASE ((DDR_TLPHY_IMAGE_ADDR)+0x10000) 
#endif 

#ifndef LPHY_DDR_SIZE
#define LPHY_DDR_SIZE 0x800000 
#endif 

#ifndef LPHY_DDR_IMAGE_BASE
#define LPHY_DDR_IMAGE_BASE ((LPHY_DDR_BASE)) 
#endif 

#ifndef LPHY_DDR_IMAGE_SIZE
#define LPHY_DDR_IMAGE_SIZE ((LPHY_DDR_SIZE)) 
#endif 

#ifndef LPHY_DDR_LINK_BASE
#define LPHY_DDR_LINK_BASE ((LPHY_DDR_IMAGE_BASE)+(LPHY_DDR_IMAGE_SIZE)) 
#endif 

#ifndef LPHY_DDR_LINK_SIZE
#define LPHY_DDR_LINK_SIZE (0x20000000-(LPHY_DDR_IMAGE_SIZE)) 
#endif 

#ifndef LPHY_TOTAL_IMG_SIZE
#define LPHY_TOTAL_IMG_SIZE ((LPHY_DTCM_SIZE)+(LPHY_ITCM_SIZE)+(LPHY_L2M_SIZE)+(LPHY_DDR_SIZE)) 
#endif 

#ifndef LPHY_SLT_DDR_BASE
#define LPHY_SLT_DDR_BASE 0x87180000 
#endif 

#ifndef LPHY_SLT_DDR_SIZE
#define LPHY_SLT_DDR_SIZE 0x180000 
#endif 

#ifndef CPHY_PUB_DTCM_BASE
#define CPHY_PUB_DTCM_BASE 0xe3479c00 
#endif 

#ifndef CPHY_PUB_ITCM_BASE
#define CPHY_PUB_ITCM_BASE 0xe3531000 
#endif 

#ifndef CPHY_PRV_DTCM_BASE
#define CPHY_PRV_DTCM_BASE 0xe3479c00 
#endif 

#ifndef CPHY_PRV_ITCM_BASE
#define CPHY_PRV_ITCM_BASE 0xe3531000 
#endif 

#ifndef CPHY_PUB_DTCM_SIZE
#define CPHY_PUB_DTCM_SIZE 0x46400 
#endif 

#ifndef CPHY_PUB_ITCM_SIZE
#define CPHY_PUB_ITCM_SIZE 0x2f000 
#endif 

#ifndef CPHY_PRV_DTCM_SIZE
#define CPHY_PRV_DTCM_SIZE 0x46400 
#endif 

#ifndef CPHY_PRV_ITCM_SIZE
#define CPHY_PRV_ITCM_SIZE 0x2f000 
#endif 

#ifndef LR_PHY_ARCH
#define LR_PHY_ARCH FEATURE_ON 
#endif 

#ifndef LPHY_EICC_ENABLE
#define LPHY_EICC_ENABLE 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
