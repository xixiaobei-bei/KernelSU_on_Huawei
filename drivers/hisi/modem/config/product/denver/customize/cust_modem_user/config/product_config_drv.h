/* MD5: 76acf58116eabbfa5c513a185d178c6c*/
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

#if !defined(__PRODUCT_CONFIG_DRV_H__)
#define __PRODUCT_CONFIG_DRV_H__

#ifndef ENABLE_BUILD_VARS
#define ENABLE_BUILD_VARS 
#endif 

#ifndef FEATURE_UPGRADE_TL
#define FEATURE_UPGRADE_TL 
#endif 

#ifndef PRODUCT_NAME
#define PRODUCT_NAME "denver" 
#endif 

#ifndef PRODUCT_CFG_CHIP_SOLUTION_NAME
#define PRODUCT_CFG_CHIP_SOLUTION_NAME "Balong" 
#endif 

#ifndef CCPU_OS
#define CCPU_OS RTOSCK_SMP 
#endif 

#ifndef HCC_VERSION
#define HCC_VERSION 7.3 
#endif 

#ifndef PLATFORM
#define PLATFORM denver 
#endif 

#ifndef PLATFORM_HISI_BALONG
#define PLATFORM_HISI_BALONG denver 
#endif 

#ifndef CONFIG_VERSION_STUB
#endif 

#ifndef HW_VERSION_STUB
#define HW_VERSION_STUB 0x33F5FC01 
#endif 

#ifndef BSP_CONFIG_EDA
#endif 

#ifdef EMU_TYPE_ESL 
#ifndef CONFIG_UART_ESL
#define CONFIG_UART_ESL 
#endif 

#ifndef FEATURE_SVLSOCKET
#endif 

#ifndef BURN_ESL
#define BURN_ESL 
#endif 

#ifndef BSP_CONFIG_EMU_MCORE_DTB
#endif 

#ifndef CONFIG_NRBBP
#define CONFIG_NRBBP 
#endif 

#ifndef CONFIG_BALONG_L2CACHE
#endif 

#ifndef CONFIG_BALONG_MODEM_RESET_CTRL
#endif 

#else
#ifndef CONFIG_BALONG_L2CACHE
#define CONFIG_BALONG_L2CACHE 
#endif 

#ifndef EMU_EDMA_LOAD_HAC
#endif 

#ifndef MINI_XLOADER
#endif 

#ifndef MINI_FASTBOOT
#endif 

#ifndef BSP_CONFIG_EMU_NO_USB
#endif 

#ifndef CONFIG_BBP
#define CONFIG_BBP 
#endif 

#ifndef CONFIG_NRBBP
#define CONFIG_NRBBP 
#endif 

#ifndef BSP_CONFIG_EMU_MCORE_DTB
#endif 

#ifndef FEATURE_CPU_PRIVATE_SLICE
#define FEATURE_CPU_PRIVATE_SLICE 
#endif 

#ifndef FEATURE_CPU_PRIVATE_SLICE_HRT
#define FEATURE_CPU_PRIVATE_SLICE_HRT 
#endif 

#endif
#ifndef BSP_CONFIG_EMU
#endif 

#ifndef BSP_CONFIG_HI3650
#define BSP_CONFIG_HI3650 
#endif 

#ifndef BSP_CONFIG_PHONE_TYPE
#define BSP_CONFIG_PHONE_TYPE 
#endif 

#ifndef BSP_CONFIG_HI3660
#define BSP_CONFIG_HI3660 
#endif 

#ifndef BSP_CONFIG_BOARD_ASIC
#define BSP_CONFIG_BOARD_ASIC 
#endif 

#ifndef FULLSTACK_NR_LLRAM_ADDR
#define FULLSTACK_NR_LLRAM_ADDR DDR_FULLSTACK_MEM_ADDR 
#endif 

#ifndef FULLSTACK_HAC_LLRAM_ADDR
#define FULLSTACK_HAC_LLRAM_ADDR (FULLSTACK_NR_LLRAM_ADDR + NRCCPU_LLRAM_BASE_SIZE) 
#endif 

#ifndef FULLSTACK_WFI_ADDR
#define FULLSTACK_WFI_ADDR (FULLSTACK_HAC_LLRAM_ADDR + HAC_LLRAM_SIZE) 
#endif 

#ifndef FULLSTACK_WFI_SIZE
#define FULLSTACK_WFI_SIZE 0x10000 
#endif 

#ifndef BSP_CONFIG_BOARD_SFT
#endif 

#ifndef BSP_HAS_SEC_FEATURE
#endif 

#ifndef CONFIG_MALLOC_UNIFIED
#define CONFIG_MALLOC_UNIFIED 
#endif 

#ifndef CONFIG_SMART_SYSTEM_MODEM
#endif 

#ifndef CONFIG_FASTBOOT_DEBUG
#endif 

#ifndef CONFIG_OF
#define CONFIG_OF 
#endif 

#ifndef FEATURE_BALONG_MODEM_ATE
#endif 

#ifndef CONFIG_RINGBUFFER_STUB
#endif 

#ifndef CONFIG_CONSOLE_STUB
#endif 

#ifndef CCPU_CORE_NUM
#define CCPU_CORE_NUM 1 
#endif 

#ifndef CONFIG_CCPU_FIQ_SMP
#endif 

#ifndef CCPU_LLRAM_BASE_ADDR
#define CCPU_LLRAM_BASE_ADDR 0xE0800000 
#endif 

#ifndef CCPU_LLRAM_BASE_SIZE
#define CCPU_LLRAM_BASE_SIZE 0x18000 
#endif 

#ifndef CCPU_SRAM_SIZE
#define CCPU_SRAM_SIZE 0x2C00 
#endif 

#ifndef SOCP_LLRAM_ADDR
#define SOCP_LLRAM_ADDR (CCPU_LLRAM_BASE_ADDR + CCPU_SRAM_SIZE) 
#endif 

#ifndef SOCP_LLRAM_SIZE
#define SOCP_LLRAM_SIZE 0x8000 
#endif 

#ifndef LL2_LLRAM_BASE_ADDR
#define LL2_LLRAM_BASE_ADDR (SOCP_LLRAM_ADDR + SOCP_LLRAM_SIZE) 
#endif 

#ifndef LL2_LLRAM_SIZE
#define LL2_LLRAM_SIZE 0xC800 
#endif 

#ifndef CONFIG_CCPU_HAS_LLRAM
#define CONFIG_CCPU_HAS_LLRAM 
#endif 

#ifndef CCPU_LLRAM_ADDR
#define CCPU_LLRAM_ADDR (LL2_LLRAM_BASE_ADDR + LL2_LLRAM_SIZE) 
#endif 

#ifndef CCPU_LLRAM_SIZE
#define CCPU_LLRAM_SIZE (CCPU_LLRAM_BASE_SIZE - CCPU_SRAM_SIZE - SOCP_LLRAM_SIZE - LL2_LLRAM_SIZE) 
#endif 

#ifndef HI_SRAM_MEM_ADDR
#define HI_SRAM_MEM_ADDR CCPU_LLRAM_BASE_ADDR 
#endif 

#ifndef HI_SRAM_SIZE
#define HI_SRAM_SIZE CCPU_SRAM_SIZE 
#endif 

#ifndef DRV_SRAM_ADDR
#define DRV_SRAM_ADDR (HI_SRAM_MEM_ADDR) 
#endif 

#ifndef DRV_SRAM_SIZE
#define DRV_SRAM_SIZE 0x2000 
#endif 

#ifndef CPHY_SRAM_ADDR
#define CPHY_SRAM_ADDR ((DRV_SRAM_ADDR) + (DRV_SRAM_SIZE)) 
#endif 

#ifndef CPHY_SRAM_SIZE
#define CPHY_SRAM_SIZE 0x800 
#endif 

#ifndef CPHY_LPC_SRAM_ADDR
#define CPHY_LPC_SRAM_ADDR ( CPHY_SRAM_ADDR ) 
#endif 

#ifndef CPHY_LPC_SRAM_SIZE
#define CPHY_LPC_SRAM_SIZE 0x40 
#endif 

#ifndef CPHY_1X_DATA_MBX_SRAM_ADDR
#define CPHY_1X_DATA_MBX_SRAM_ADDR ( (CPHY_LPC_SRAM_ADDR) + (CPHY_LPC_SRAM_SIZE) ) 
#endif 

#ifndef CPHY_1X_DATA_MBX_SRAM_SIZE
#define CPHY_1X_DATA_MBX_SRAM_SIZE 0x40 
#endif 

#ifndef CPHY_HRPD_DATA_MBX_SRAM_ADDR
#define CPHY_HRPD_DATA_MBX_SRAM_ADDR ( (CPHY_1X_DATA_MBX_SRAM_ADDR) + (CPHY_1X_DATA_MBX_SRAM_SIZE) ) 
#endif 

#ifndef CPHY_HRPD_DATA_MBX_SRAM_SIZE
#define CPHY_HRPD_DATA_MBX_SRAM_SIZE 0x40 
#endif 

#ifndef GPHY_SRAM_ADDR
#define GPHY_SRAM_ADDR ((CPHY_SRAM_ADDR) + (CPHY_SRAM_SIZE)) 
#endif 

#ifndef GPHY_SRAM_SIZE
#define GPHY_SRAM_SIZE 0x40 
#endif 

#ifndef CONFIG_CCPU_HAS_TCM
#define CONFIG_CCPU_HAS_TCM 
#endif 

#ifndef CCPU_ITCM_ADDR
#define CCPU_ITCM_ADDR 0x0 
#endif 

#ifndef CCPU_ITCM_SIZE
#define CCPU_ITCM_SIZE 0x4000 
#endif 

#ifndef CCPU_ITCM_SIZE_CFG
#define CCPU_ITCM_SIZE_CFG (0x5u<<0x2) 
#endif 

#ifndef CCPU_DTCM_ADDR
#define CCPU_DTCM_ADDR (CCPU_ITCM_ADDR + CCPU_ITCM_SIZE) 
#endif 

#ifndef CCPU_DTCM_SIZE
#define CCPU_DTCM_SIZE 0x4000 
#endif 

#ifndef CCPU_DTCM_SIZE_CFG
#define CCPU_DTCM_SIZE_CFG (0x5u<<0x2) 
#endif 

#ifndef CONFIG_HAS_HAC
#define CONFIG_HAS_HAC 
#endif 

#ifndef HAC_OS
#define HAC_OS RTOSCK_SMP 
#endif 

#ifndef HAC_COMPILER
#define HAC_COMPILER HCC 
#endif 

#ifndef CONFIG_HAC_ARCH
#define CONFIG_HAC_ARCH "Cortex-R8" 
#endif 

#ifndef HAC_CORE_NUM
#define HAC_CORE_NUM 2 
#endif 

#ifndef HAC_TEXT_START_ADDR
#define HAC_TEXT_START_ADDR HAC_LLRAM_ADDR 
#endif 

#ifndef HAC_LLRAM_ADDR
#define HAC_LLRAM_ADDR 0xF4C00000 
#endif 

#ifndef HAC_LLRAM_SIZE
#define HAC_LLRAM_SIZE 0x180000 
#endif 

#ifndef HAC_LLRAM_UNCACHED_SIZE
#define HAC_LLRAM_UNCACHED_SIZE 0xB0000 
#endif 

#ifndef HAC_LLRAM_CACHED_SIZE
#define HAC_LLRAM_CACHED_SIZE (HAC_LLRAM_SIZE - HAC_LLRAM_UNCACHED_SIZE) 
#endif 

#ifndef CONFIG_HAC_HAS_TCM
#define CONFIG_HAC_HAS_TCM 
#endif 

#ifndef HAC_ITCM_SIZE
#define HAC_ITCM_SIZE 0x8000 
#endif 

#ifndef HAC_ITCM_SIZE_CFG
#define HAC_ITCM_SIZE_CFG (0x6u<<0x2) 
#endif 

#ifndef HAC_DTCM_SIZE
#define HAC_DTCM_SIZE 0x20000 
#endif 

#ifndef HAC_DTCM_SIZE_CFG
#define HAC_DTCM_SIZE_CFG (0x8u<<0x2) 
#endif 

#ifndef HAC_TCM_START_ADDR
#define HAC_TCM_START_ADDR 0xFE000000 
#endif 

#ifndef HAC_ITCM0_ADDR
#define HAC_ITCM0_ADDR HAC_TCM_START_ADDR 
#endif 

#ifndef HAC_ITCM1_ADDR
#define HAC_ITCM1_ADDR (HAC_ITCM0_ADDR + HAC_ITCM_SIZE) 
#endif 

#ifndef HAC_DTCM_ADDR
#define HAC_DTCM_ADDR 0xFE020000 
#endif 

#ifndef HAC_DTCM0_ADDR
#define HAC_DTCM0_ADDR HAC_DTCM_ADDR 
#endif 

#ifndef HAC_DTCM1_ADDR
#define HAC_DTCM1_ADDR HAC_DTCM_ADDR 
#endif 

#ifndef HAC_THUMB_OPTIMIZE
#define HAC_THUMB_OPTIMIZE 
#endif 

#ifndef NRCCPU_TEMP_CACHE_ADDR
#define NRCCPU_TEMP_CACHE_ADDR 0x1F000000 
#endif 

#ifndef NRCCPU_TEMP_CACHE_SIZE
#define NRCCPU_TEMP_CACHE_SIZE 0x00200000 
#endif 

#ifndef NRCCPU_TEMP_UNCACHE_ADDR
#define NRCCPU_TEMP_UNCACHE_ADDR (NRCCPU_TEMP_CACHE_ADDR+NRCCPU_TEMP_CACHE_SIZE) 
#endif 

#ifndef NRCCPU_TEMP_UNCACHE_SIZE
#define NRCCPU_TEMP_UNCACHE_SIZE 0x00200000 
#endif 

#ifndef CONFIG_HAS_NRCCPU
#define CONFIG_HAS_NRCCPU 
#endif 

#ifndef NRCCPU_OS
#define NRCCPU_OS RTOSCK_SMP 
#endif 

#ifndef NRCCPU_COMPILER
#define NRCCPU_COMPILER HCC 
#endif 

#ifndef CONFIG_NRCCPU_ARCH
#define CONFIG_NRCCPU_ARCH "Cortex-R8" 
#endif 

#ifndef NRCCPU_CORE_NUM
#define NRCCPU_CORE_NUM 2 
#endif 

#ifndef CONFIG_NRCCPU_HAS_LLRAM
#define CONFIG_NRCCPU_HAS_LLRAM 
#endif 

#ifndef NRCCPU_LLRAM_BASE_ADDR
#define NRCCPU_LLRAM_BASE_ADDR 0xF5400000 
#endif 

#ifndef NRCCPU_LLRAM_BASE_SIZE
#define NRCCPU_LLRAM_BASE_SIZE 0x000A0000 
#endif 

#ifndef NRCCPU_SRAM_SIZE
#define NRCCPU_SRAM_SIZE 0x2800 
#endif 

#ifndef NRCCPU_LLRAM_ADDR
#define NRCCPU_LLRAM_ADDR (NRCCPU_LLRAM_BASE_ADDR + NRCCPU_SRAM_SIZE) 
#endif 

#ifndef NRCCPU_LLRAM_SIZE
#define NRCCPU_LLRAM_SIZE (NRCCPU_LLRAM_BASE_SIZE - NRCCPU_SRAM_SIZE) 
#endif 

#ifndef CONFIG_NRCCPU_HAS_TCM
#define CONFIG_NRCCPU_HAS_TCM 
#endif 

#ifndef NRCCPU_ITCM_ADDR
#define NRCCPU_ITCM_ADDR 0x0 
#endif 

#ifndef NRCCPU_ITCM_SIZE
#define NRCCPU_ITCM_SIZE 0x4000 
#endif 

#ifndef NRCCPU_ITCM_SIZE_CFG
#define NRCCPU_ITCM_SIZE_CFG (0x5u<<0x2) 
#endif 

#ifndef NRCCPU_DTCM_ADDR
#define NRCCPU_DTCM_ADDR 0x10000 
#endif 

#ifndef NRCCPU_DTCM_SIZE
#define NRCCPU_DTCM_SIZE 0x10000 
#endif 

#ifndef NRCCPU_DTCM_SIZE_CFG
#define NRCCPU_DTCM_SIZE_CFG (0x7u<<0x2) 
#endif 

#ifndef MPU_ASLR_RESERVE
#define MPU_ASLR_RESERVE 0x100000 
#endif 

#ifndef MPU_ALGN_RESERVE
#define MPU_ALGN_RESERVE 0x100000 
#endif 

#ifndef DDR_MCORE_RELOC_SIZE
#define DDR_MCORE_RELOC_SIZE 0x10000 
#endif 

#ifndef CONFIG_NVIM
#define CONFIG_NVIM 
#endif 

#ifndef CONFIG_NR_NVIM
#define CONFIG_NR_NVIM 
#endif 

#ifndef FEATURE_NV_FLASH_ON
#endif 

#ifndef FEATURE_NV_EMMC_ON
#define FEATURE_NV_EMMC_ON 
#endif 

#ifndef FEATURE_NV_LFILE_ON
#endif 

#ifndef FEATURE_NV_RFILE_ON
#endif 

#ifndef FEATURE_NV_NO_MODEMNVM_SYS
#define FEATURE_NV_NO_MODEMNVM_SYS 
#endif 

#ifndef FEATURE_NV_SEC_ON
#define FEATURE_NV_SEC_ON 
#endif 

#ifndef FEATURE_DIS_HMAC_CHECK
#endif 

#ifndef EFUSE_HUK_BIT_START
#define EFUSE_HUK_BIT_START 1995 
#endif 

#ifndef FEATURE_TLPHY_MAILBOX
#define FEATURE_TLPHY_MAILBOX 
#endif 

#ifndef CONFIG_MAILBOX_TYPE
#define CONFIG_MAILBOX_TYPE 
#endif 

#ifndef CONFIG_HIFI_MAILBOX
#endif 

#ifndef FEATURE_CPHY_MAILBOX
#endif 

#ifndef ENABLE_BUILD_OM
#define ENABLE_BUILD_OM 
#endif 

#ifndef CONFIG_DUMP_LOG_ESCAPE_FIQ
#define CONFIG_DUMP_LOG_ESCAPE_FIQ 
#endif 

#ifndef FEATURE_OM_PHONE
#endif 

#ifndef ENABLE_BUILD_SYSVIEW
#endif 

#ifndef ENABLE_BUILD_CPUVIEW
#endif 

#ifndef ENABLE_BUILD_MEMVIEW
#endif 

#ifndef ENABLE_AMON_SOC
#endif 

#ifndef ENABLE_AMON_MDM
#define ENABLE_AMON_MDM 
#endif 

#ifndef ENABLE_BUILD_UTRACE
#endif 

#ifndef ENABLE_BUILD_HAC_DUMP
#define ENABLE_BUILD_HAC_DUMP 
#endif 

#ifndef ENABLE_BUILD_NRRDR
#define ENABLE_BUILD_NRRDR 
#endif 

#ifndef ENABLE_BUILD_DUMP_MDM_LPM3
#define ENABLE_BUILD_DUMP_MDM_LPM3 
#endif 

#ifndef ENABLE_BUILD_SOCP
#define ENABLE_BUILD_SOCP 
#endif 

#ifndef FEATURE_SOCP_VIC
#endif 

#ifndef SOCP_DECODE_ENABLE
#endif 

#ifndef CONFIG_DEFLATE
#endif 

#ifndef CONFIG_DIAG_NRM
#define CONFIG_DIAG_NRM 
#endif 

#ifndef CONFIG_FILEBROSWER
#endif 

#ifndef CONFIG_HIMS_NV_PROC
#define CONFIG_HIMS_NV_PROC 
#endif 

#ifndef FEATURE_SOCP_ADDR_64BITS
#define FEATURE_SOCP_ADDR_64BITS 
#endif 

#ifndef SOCP_V300
#define SOCP_V300 
#endif 

#ifndef DIAG_SYSTEM_5G
#define DIAG_SYSTEM_5G 
#endif 

#ifndef CONFIG_DIAG_SYSTEM
#define CONFIG_DIAG_SYSTEM 
#endif 

#ifndef CONFIG_DIAG_NETLINK
#endif 

#ifndef CONFIG_VER_3_TLBBP_DS_BOSTON_AND_LATER
#define CONFIG_VER_3_TLBBP_DS_BOSTON_AND_LATER 
#endif 

#ifndef DIAG_VERSION_ENG
#endif 

#ifndef DIAG_MODEM_SINGLE_CORE
#define DIAG_MODEM_SINGLE_CORE 
#endif 

#ifndef CONFIG_APPLOG
#endif 

#ifndef CONFIG_DIAG_FRAME
#define CONFIG_DIAG_FRAME 
#endif 

#ifndef FEATURE_SOCP_DECODE_INT_TIMEOUT
#define FEATURE_SOCP_DECODE_INT_TIMEOUT FEATURE_OFF 
#endif 

#ifndef FEATURE_SOCP_MEM_RESERVED			
#define FEATURE_SOCP_MEM_RESERVED			 FEATURE_ON 
#endif 

#ifndef FEATURE_HDS_PRINTLOG
#define FEATURE_HDS_PRINTLOG FEATURE_ON 
#endif 

#ifndef FEATURE_HDS_TRANSLOG
#define FEATURE_HDS_TRANSLOG FEATURE_ON 
#endif 

#ifndef FEATURE_SRE_PRINT_SLICE
#define FEATURE_SRE_PRINT_SLICE FEATURE_ON 
#endif 

#ifndef FEATURE_SRE_PRINT_RTC
#define FEATURE_SRE_PRINT_RTC FEATURE_OFF 
#endif 

#ifndef FEATURE_NOC_PARSE_ES
#endif 

#ifndef CONFIG_MID
#define CONFIG_MID 
#endif 

#ifndef CONFIG_NOC
#define CONFIG_NOC 
#endif 

#ifndef CONFIG_NOC_AP
#endif 

#ifndef CONFIG_MODEM_FULL_DUMP
#endif 

#ifndef CONFIG_MODEM_MINI_DUMP
#endif 

#ifndef CONFIG_MODEM_DUMP_BLACKLIST
#define CONFIG_MODEM_DUMP_BLACKLIST 
#endif 

#ifndef CONFIG_MODEM_SOCP_3_0
#define CONFIG_MODEM_SOCP_3_0 
#endif 

#ifndef CONFIG_BUS_ERR_NR
#define CONFIG_BUS_ERR_NR 
#endif 

#ifndef CONFIG_BUS_ERR_LR
#define CONFIG_BUS_ERR_LR 
#endif 

#ifndef CONFIG_BUS_ERR_AP
#endif 

#ifndef CONFIG_PDLOCK
#endif 

#ifndef CONFIG_PDLOCK_RENEW
#define CONFIG_PDLOCK_RENEW 
#endif 

#ifndef CONFIG_PDLOCK_AP
#endif 

#ifndef ENABLE_BUILD_PRINT
#define ENABLE_BUILD_PRINT 
#endif 

#ifndef CONFIG_BALONG_CORESIGHT
#endif 

#ifndef CONFIG_M3_APB_SPI
#endif 

#ifndef FEATURE_SAMPLE_LTE_CHAN 			
#define FEATURE_SAMPLE_LTE_CHAN 			 FEATURE_OFF 
#endif 

#ifndef FEATURE_SCI_PROTOL_T1
#define FEATURE_SCI_PROTOL_T1 FEATURE_OFF 
#endif 

#ifndef FEATURE_SCI_ESIM
#define FEATURE_SCI_ESIM FEATURE_OFF 
#endif 

#ifndef CONFIG_SC
#endif 

#ifndef CONFIG_DSP
#endif 

#ifndef CONFIG_NRDSP
#define CONFIG_NRDSP 
#endif 

#ifndef CONFIG_NRDSP_FULL_DUMP
#endif 

#ifndef CONFIG_NRDSP_MINI_DUMP
#define CONFIG_NRDSP_MINI_DUMP 
#endif 

#ifndef CONFIG_CBBE
#define CONFIG_CBBE 
#endif 

#ifndef MODEM_MEM_REPAIR
#define MODEM_MEM_REPAIR 
#endif 

#ifndef CONFIG_WATCHPOINT
#endif 

#ifndef ENABLE_BUILD_PRINT
#define ENABLE_BUILD_PRINT 
#endif 

#ifndef CONFIG_DSPDVS
#define CONFIG_DSPDVS 
#endif 

#ifndef FEATURE_DSP_DFS
#define FEATURE_DSP_DFS FEATURE_ON 
#endif 

#ifndef CONFIG_DSP_STRESS
#endif 

#ifndef FEATURE_GUBBP_HANDSHAKE
#define FEATURE_GUBBP_HANDSHAKE FEATURE_ON 
#endif 

#ifndef FEATURE_GUDRX_NEWVERSION
#define FEATURE_GUDRX_NEWVERSION FEATURE_ON 
#endif 

#ifndef FEATURE_BOSTON_ONLY_FEATURE
#define FEATURE_BOSTON_ONLY_FEATURE FEATURE_OFF 
#endif 

#ifndef FEATURE_DSP_PM_SEPARATE_MODE
#define FEATURE_DSP_PM_SEPARATE_MODE FEATURE_ON 
#endif 

#ifndef FEATURE_CSDR_COMBINE
#define FEATURE_CSDR_COMBINE FEATURE_ON 
#endif 

#ifndef FEATURE_GUC_TRANSPLANT
#define FEATURE_GUC_TRANSPLANT FEATURE_OFF 
#endif 

#ifndef CONFIG_DFS_DDR
#endif 

#ifndef CONFIG_DFS_NOC
#endif 

#ifndef SUPPORT_PARA_CFG
#endif 

#ifndef FEATURE_TCM_RETENTION
#define FEATURE_TCM_RETENTION FEATURE_ON 
#endif 

#ifndef FEATURE_TCM_PART_RETENTION
#define FEATURE_TCM_PART_RETENTION FEATURE_OFF 
#endif 

#ifndef CONFIG_NRCPM
#define CONFIG_NRCPM 
#endif 

#ifndef CONFIG_NEWVER_RFIC
#define CONFIG_NEWVER_RFIC 
#endif 

#ifndef CONFIG_NR_NEWMAILBOX
#define CONFIG_NR_NEWMAILBOX 
#endif 

#ifndef CONFIG_BBIC_LOWPOWER
#define CONFIG_BBIC_LOWPOWER 
#endif 

#ifndef CONFIG_NR_LTE_INTEROPER
#define CONFIG_NR_LTE_INTEROPER 
#endif 

#ifndef CONFIG_LIGHTSLEEP_POWERDOWN
#define CONFIG_LIGHTSLEEP_POWERDOWN 
#endif 

#ifndef FEATURE_MULTI_CHANNEL			
#define FEATURE_MULTI_CHANNEL			 FEATURE_OFF 
#endif 

#ifndef FEATURE_CHR_OM
#define FEATURE_CHR_OM FEATURE_OFF 
#endif 

#ifndef FEATURE_CHR_NR
#define FEATURE_CHR_NR 
#endif 

#ifndef CONFIG_GPIO_PL061
#define CONFIG_GPIO_PL061 
#endif 

#ifndef CONFIG_TRNG_SEED
#define CONFIG_TRNG_SEED 
#endif 

#ifndef FEATURE_ANTEN_DETECT
#define FEATURE_ANTEN_DETECT 
#endif 

#ifndef CONFIG_ANTEN
#define CONFIG_ANTEN 
#endif 

#ifndef CONFIG_ONOFF
#define CONFIG_ONOFF 
#endif 

#ifndef CONFIG_MLOADER_COLD_PATCH
#define CONFIG_MLOADER_COLD_PATCH 
#endif 

#ifndef CONFIG_LOAD_SEC_IMAGE
#define CONFIG_LOAD_SEC_IMAGE 
#endif 

#ifndef CONFIG_COMPRESS_CCORE_IMAGE
#define CONFIG_COMPRESS_CCORE_IMAGE 
#endif 

#ifndef CONFIG_COMPRESS_DTB_IMAGE
#endif 

#ifndef CONFIG_MODEM_DTB_LOAD_IN_KERNEL
#define CONFIG_MODEM_DTB_LOAD_IN_KERNEL 
#endif 

#ifndef CONFIG_ENABLE_DTO
#define CONFIG_ENABLE_DTO 
#endif 

#ifndef CONFIG_IS_DTB_VERIFY
#define CONFIG_IS_DTB_VERIFY 
#endif 

#ifndef CONFIG_MODEM_BALONG_ASLR
#define CONFIG_MODEM_BALONG_ASLR 
#endif 

#ifndef CONFIG_MODEM_BALONG_ASLR_CACHE_ON
#define CONFIG_MODEM_BALONG_ASLR_CACHE_ON 
#endif 

#ifndef CONFIG_MODEM_ASLR_DEBUG
#endif 

#ifndef CONFIG_ARM_BASLR_RELOC_SIZE
#define CONFIG_ARM_BASLR_RELOC_SIZE 0x1500000 
#endif 

#ifndef CONFIG_MLOADER
#define CONFIG_MLOADER 
#endif 

#ifndef FEATURE_DELAY_MODEM_INIT
#define FEATURE_DELAY_MODEM_INIT FEATURE_ON 
#endif 

#ifndef CONFIG_MODEM_PINTRL
#define CONFIG_MODEM_PINTRL 
#endif 

#ifndef CONFIG_ADC
#define CONFIG_ADC 
#endif 

#ifndef CONFIG_EFUSE
#define CONFIG_EFUSE 
#endif 

#ifndef CONFIG_EFUSE_NR
#define CONFIG_EFUSE_NR 
#endif 

#ifndef CONFIG_RTC_ON_SOC
#define CONFIG_RTC_ON_SOC 
#endif 

#ifndef CONFIG_RFILE_ON
#define CONFIG_RFILE_ON 
#endif 

#ifndef CONFIG_RFILE_USER
#define CONFIG_RFILE_USER 
#endif 

#ifndef FEATURE_THERMAL
#define FEATURE_THERMAL 
#endif 

#ifndef CONFIG_TEE_DECOUPLE
#define CONFIG_TEE_DECOUPLE 
#endif 

#ifdef MODEM_SANITIZER 
#ifndef STACK_CANARY_COMPILE
#endif 

#else
#ifndef STACK_CANARY_COMPILE
#define STACK_CANARY_COMPILE 
#endif 

#endif
#ifndef MODEM_FW_PARTITION_SIZE
#define MODEM_FW_PARTITION_SIZE 138936320 
#endif 

#ifndef CONFIG_CCPU_FIQ_SMP
#define CONFIG_CCPU_FIQ_SMP 
#endif 

#ifndef OS_K3V3_USE_LPM3_API
#endif 

#ifndef BSP_ICC_MCHANNEL_USE_LPM3TCM
#define BSP_ICC_MCHANNEL_USE_LPM3TCM 
#endif 

#ifndef CONFIG_FASTBOOT_UART_NUM
#define CONFIG_FASTBOOT_UART_NUM 0 
#endif 

#ifndef MCORE_MPU_ENABLE
#define MCORE_MPU_ENABLE 
#endif 

#ifndef CONFIG_MODULE_VIC
#endif 

#ifndef CONFIG_AT_UART
#endif 

#ifndef CONFIG_MEM_DEBUG
#endif 

#ifndef CONFIG_BALONG_MODEM_RESET
#define CONFIG_BALONG_MODEM_RESET 
#endif 

#ifndef DTS_STATIC_MEM_SIZE
#define DTS_STATIC_MEM_SIZE 0X5000 
#endif 

#ifndef CONFIG_AVS
#define CONFIG_AVS 
#endif 

#ifndef CONFIG_CPUFREQ
#define CONFIG_CPUFREQ 
#endif 

#ifndef CPUFREQ_DEBUG_INTERFACE
#endif 

#ifndef CONFIG_BUSFREQ
#define CONFIG_BUSFREQ 
#endif 

#ifndef CONFIG_IDLEFREQ
#define CONFIG_IDLEFREQ 
#endif 

#ifndef CONFIG_CCORE_REGULATOR
#define CONFIG_CCORE_REGULATOR 
#endif 

#ifndef CONFIG_CCORE_WDT
#define CONFIG_CCORE_WDT 
#endif 

#ifndef CONFIG_ACORE_WDT
#endif 

#ifndef CONFIG_WDT_HAC_BUILD
#define CONFIG_WDT_HAC_BUILD 
#endif 

#ifndef CONFIG_WDT_NR_BUILD
#define CONFIG_WDT_NR_BUILD 
#endif 

#ifndef CONFIG_PMIC_FPGA
#endif 

#ifndef CONFIG_NRCCPU_PMU
#define CONFIG_NRCCPU_PMU 
#endif 

#ifndef CONFIG_PMIC_SPMI
#define CONFIG_PMIC_SPMI 
#endif 

#ifndef CONFIG_PMU_NEW
#define CONFIG_PMU_NEW 
#endif 

#ifndef CONFIG_PMIC_SPMI_PROTOCOL
#define CONFIG_PMIC_SPMI_PROTOCOL 
#endif 

#ifndef CONFIG_PMU_XLOADER
#define CONFIG_PMU_XLOADER 
#endif 

#ifndef CONFIG_NR_DCXO_NV
#define CONFIG_NR_DCXO_NV 
#endif 

#ifdef EMU_TYPE_ESL 
#ifndef CONFIG_PMU
#endif 

#else
#ifndef CONFIG_PMU
#define CONFIG_PMU 
#endif 

#endif
#ifndef CONFIG_PMCTRL_MODEM
#define CONFIG_PMCTRL_MODEM 
#endif 

#ifndef CONFIG_CCORE_NOTIFIER
#define CONFIG_CCORE_NOTIFIER 
#endif 

#ifndef CONFIG_NRCCPU_PM
#define CONFIG_NRCCPU_PM 
#endif 

#ifndef CONFIG_NRCCPU_PM_DEBUG
#endif 

#ifndef CONFIG_LRCCPU_PM_DEBUG
#endif 

#ifndef CONFIG_BALONG_CCLK
#define CONFIG_BALONG_CCLK 
#endif 

#ifndef CONFIG_BALONG_CCLK_DEBUG
#endif 

#ifndef CONFIG_BALONG_CCLK_ATUOGATE
#define CONFIG_BALONG_CCLK_ATUOGATE 
#endif 

#ifndef CONFIG_PM_SCTRL
#define CONFIG_PM_SCTRL 
#endif 

#ifndef CONFIG_AVS_TEST
#define CONFIG_AVS_TEST 
#endif 

#ifndef CONFIG_MODULE_HAC_PM
#define CONFIG_MODULE_HAC_PM 
#endif 

#ifndef CONFIG_CCORE_CPU_IDLE
#define CONFIG_CCORE_CPU_IDLE 
#endif 

#ifndef CONFIG_HAC_CPU_IDLE
#define CONFIG_HAC_CPU_IDLE 
#endif 

#ifndef CONFIG_CCORE_PM
#define CONFIG_CCORE_PM 
#endif 

#ifndef CONFIG_MODULE_IPC
#define CONFIG_MODULE_IPC 
#endif 

#ifndef CONFIG_IPCM_USE_FPGA_VIC
#endif 

#ifndef CONFIG_MODULE_TIMER
#define CONFIG_MODULE_TIMER 
#endif 

#ifndef CONFIG_MODULE_HAC_SLICE
#define CONFIG_MODULE_HAC_SLICE 
#endif 

#ifndef CONFIG_CCORE_CPU
#define CONFIG_CCORE_CPU 
#endif 

#ifndef CONFIG_HAS_CCORE_WAKELOCK
#define CONFIG_HAS_CCORE_WAKELOCK 
#endif 

#ifndef CONFIG_HAS_HAC_WAKELOCK
#define CONFIG_HAS_HAC_WAKELOCK 
#endif 

#ifndef CONFIG_CCORE_BALONG_PM
#define CONFIG_CCORE_BALONG_PM 
#endif 

#ifndef CONFIG_BALONG_EDMA
#define CONFIG_BALONG_EDMA 
#endif 

#ifndef CONFIG_EDMA_DEBUG
#endif 

#ifndef CONFIG_PWC_MNTN_CCORE
#endif 

#ifndef CONFIG_HWADP
#define CONFIG_HWADP 
#endif 

#ifndef CONFIG_SYSCTRL
#define CONFIG_SYSCTRL 
#endif 

#ifndef CONFIG_SYSBUS
#endif 

#ifndef CONFIG_SYSBUS_NRCCPU
#endif 

#ifndef CONFIG_SYSBUS_HAC
#endif 

#ifndef CONFIG_BALONG_HPM_TEMP
#endif 

#ifndef CONFIG_MEM
#define CONFIG_MEM 
#endif 

#ifndef CONFIG_TCXO_BALONG
#endif 

#ifndef CONFIG_BALONG_DPM
#endif 

#ifndef CONFIG_USE_TIMER_STAMP
#endif 

#ifndef CONFIG_MODULE_BUSSTRESS
#endif 

#ifndef CONFIG_RSR_ACC
#define CONFIG_RSR_ACC 
#endif 

#ifndef CONFIG_RSRACC_DEBUG
#endif 

#ifndef CONFIG_CCPU_HOTPLUG
#endif 

#ifndef CONFIG_MPERF
#endif 

#ifndef CONFIG_AXIMEM_BALONG
#define CONFIG_AXIMEM_BALONG 
#endif 

#ifndef CONFIG_AXIMEM_CLK_RST
#define CONFIG_AXIMEM_CLK_RST 
#endif 

#ifndef CONFIG_NXDEP_MPU
#define CONFIG_NXDEP_MPU 
#endif 

#ifndef CONFIG_NXDEP_MPU_NEW
#define CONFIG_NXDEP_MPU_NEW 
#endif 

#ifndef CONFIG_PM_OM
#define CONFIG_PM_OM 
#endif 

#ifndef CONFIG_PM_OM_NR
#define CONFIG_PM_OM_NR 
#endif 

#ifndef CONFIG_PM_OM_DEBUG
#endif 

#ifndef CONFIG_IPF
#endif 

#ifndef CONFIG_PSAM
#endif 

#ifndef CONFIG_CIPHER
#endif 

#ifndef CONFIG_EIPF
#define CONFIG_EIPF 
#endif 

#ifndef CONFIG_WAN	
#define CONFIG_WAN	 
#endif 

#ifndef IPF_UL_DMA
#define IPF_UL_DMA 
#endif 

#ifndef CONFIG_NEW_PLATFORM
#define CONFIG_NEW_PLATFORM 
#endif 

#ifndef CONFIG_CIPHER_NEW
#define CONFIG_CIPHER_NEW 
#endif 

#ifndef CONFIG_CIPHER_EN_DC
#define CONFIG_CIPHER_EN_DC 
#endif 

#ifndef CONFIG_ECIPHER
#define CONFIG_ECIPHER 
#endif 

#ifndef CONFIG_MAA_BALONG
#define CONFIG_MAA_BALONG 
#endif 

#ifndef CONFIG_MAA_LR
#define CONFIG_MAA_LR 
#endif 

#ifndef CONFIG_MAA_NR
#define CONFIG_MAA_NR 
#endif 

#ifndef CONFIG_ESPE
#define CONFIG_ESPE 
#endif 

#ifndef CONFIG_BALONG_ESPE
#define CONFIG_BALONG_ESPE 
#endif 

#ifndef CONFIG_ESPE_PHONE_SOC
#define CONFIG_ESPE_PHONE_SOC 
#endif 

#ifndef CONFIG_MAA_LL1C
#define CONFIG_MAA_LL1C 
#endif 

#ifndef CONFIG_MAA_V2
#define CONFIG_MAA_V2 
#endif 

#ifndef MAA_ASIC
#define MAA_ASIC 
#endif 

#ifndef CONFIG_IPF_VESION
#define CONFIG_IPF_VESION 2 
#endif 

#ifndef CONFIG_IPF_ADQ_LEN
#define CONFIG_IPF_ADQ_LEN 5 
#endif 

#ifndef CONFIG_IPF_PROPERTY_MBB
#endif 

#ifndef CONFIG_USB_DWC3_VBUS_DISCONNECT
#endif 

#ifndef USB3_SYNOPSYS_PHY
#endif 

#ifndef CONFIG_USB_FORCE_HIGHSPEED
#endif 

#ifndef FEATURE_HISOCKET
#define FEATURE_HISOCKET FEATURE_OFF 
#endif 

#ifndef CONFIG_BALONG_TRANS_REPORT
#define CONFIG_BALONG_TRANS_REPORT 
#endif 

#ifndef CONFIG_HMI
#define CONFIG_HMI 
#endif 

#ifndef CONFIG_HMI_ICC
#define CONFIG_HMI_ICC 
#endif 

#ifndef CONFIG_HMI_DEBUG
#define CONFIG_HMI_DEBUG 
#endif 

#ifndef ENABLE_TEST_CODE
#endif 

#ifndef CONFIG_LLT_MDRV
#endif 

#ifndef CONFIG_ECDC
#endif 

#ifndef CONFIG_ICC
#define CONFIG_ICC 
#endif 

#ifndef CONFIG_NRICC
#define CONFIG_NRICC 
#endif 

#ifndef CONFIG_ICC_DEBUG
#endif 

#ifndef CONFIG_SEC_ICC
#define CONFIG_SEC_ICC 
#endif 

#ifndef CONFIG_EICC_V220
#define CONFIG_EICC_V220 
#endif 

#ifdef EMU_TYPE_EMU 
#ifndef CONFIG_EICC_PHY_EMU
#define CONFIG_EICC_PHY_EMU 
#endif 

#endif
#ifndef NPHY_EICC_ENABLE
#define NPHY_EICC_ENABLE 
#endif 

#ifndef CONFIG_CSHELL
#endif 

#ifndef CONFIG_NR_CSHELL
#endif 

#ifndef CONFIG_UART_SHELL
#endif 

#ifndef CONFIG_OS_INCLUDE_SHELL
#endif 

#ifndef CONFIG_SHELL_SYMBOL_REG
#endif 

#ifndef CONFIG_IPC_MSG
#define CONFIG_IPC_MSG 
#endif 

#ifndef CONFIG_IPC_MSG_AO_DISC
#define CONFIG_IPC_MSG_AO_DISC 
#endif 

#ifndef CONFIG_IPC_MSG_V230
#endif 

#ifndef CONFIG_BALONG_IPC_MSG_V230
#endif 

#ifndef CONFIG_HW_SPINLOCK
#define CONFIG_HW_SPINLOCK 
#endif 

#ifndef HI_NRSRAM_MEM_ADDR
#define HI_NRSRAM_MEM_ADDR NRCCPU_LLRAM_BASE_ADDR 
#endif 

#ifndef HI_NRSRAM_SIZE                		
#define HI_NRSRAM_SIZE                		 NRCCPU_SRAM_SIZE 
#endif 

#ifndef DRV_NRSRAM_ADDR               		
#define DRV_NRSRAM_ADDR               		 (HI_NRSRAM_MEM_ADDR) 
#endif 

#ifndef DRV_NRSRAM_SIZE						
#define DRV_NRSRAM_SIZE						 0x2000 
#endif 

#ifndef LPMCU_DRAM_WINDOW
#define LPMCU_DRAM_WINDOW 0x20000000 
#endif 

#ifndef DDR_MEM_ADDR       		
#define DDR_MEM_ADDR       		 0xA0000000 
#endif 

#ifndef DDR_MEM_SIZE       		
#define DDR_MEM_SIZE       		 0x11B00000 
#endif 

#ifndef DDR_APP_ACP_ADDR
#define DDR_APP_ACP_ADDR 0 
#endif 

#ifndef DDR_APP_ACP_SIZE
#define DDR_APP_ACP_SIZE 0 
#endif 

#ifndef DDR_MDM_ACP_ADDR
#define DDR_MDM_ACP_ADDR 0 
#endif 

#ifndef DDR_MDM_ACP_SIZE
#define DDR_MDM_ACP_SIZE 0 
#endif 

#ifndef DDR_TLPHY_IMAGE_SIZE
#define DDR_TLPHY_IMAGE_SIZE 0x800000 
#endif 

#ifndef DDR_NRPHY_IMAGE_SIZE
#define DDR_NRPHY_IMAGE_SIZE 0x800000 
#endif 

#ifndef DDR_MCORE_SIZE
#define DDR_MCORE_SIZE 0x6F00000 
#endif 

#ifndef DDR_MCORE_NR_SIZE
#define DDR_MCORE_NR_SIZE 0x2800000 
#endif 

#ifndef DDR_MCORE_DTS_SIZE
#define DDR_MCORE_DTS_SIZE 0x180000 
#endif 

#ifndef DDR_GU_SIZE
#define DDR_GU_SIZE 0x40000 
#endif 

#ifndef DDR_LPHY_SDR_SIZE
#define DDR_LPHY_SDR_SIZE 0x880000 
#endif 

#ifndef DDR_LCS_SIZE
#define DDR_LCS_SIZE 0x280000 
#endif 

#ifndef DDR_SDR_NR_SIZE
#define DDR_SDR_NR_SIZE 0x1400000 
#endif 

#ifndef DDR_CBBE_IMAGE_SIZE
#define DDR_CBBE_IMAGE_SIZE 0x0 
#endif 

#ifndef DDR_HARQ_UP_SIZE
#define DDR_HARQ_UP_SIZE 0x60000 
#endif 

#ifndef DDR_HARQ_NRUL_SIZE
#define DDR_HARQ_NRUL_SIZE 0x300000 
#endif 

#ifndef DDR_RFIC_SUB6G_IMAGE_SIZE
#define DDR_RFIC_SUB6G_IMAGE_SIZE 0x138000 
#endif 

#ifndef DDR_RFIC_HF_IMAGE_SIZE
#define DDR_RFIC_HF_IMAGE_SIZE 0x0c8000 
#endif 

#ifndef DDR_RFIC_DUMP_SIZE
#define DDR_RFIC_DUMP_SIZE 0x100000 
#endif 

#ifndef DDR_SEC_SHARED_SIZE
#define DDR_SEC_SHARED_SIZE 0x100000 
#endif 

#ifndef DDR_PDE_IMAGE_SIZE
#define DDR_PDE_IMAGE_SIZE 0x300000 
#endif 

#ifndef DDR_MAA_MDM_SIZE
#define DDR_MAA_MDM_SIZE 0xD00000 
#endif 

#ifndef DDR_FULLSTACK_MEM_SIZE
#define DDR_FULLSTACK_MEM_SIZE 0x500000 
#endif 

#ifndef DDR_LPMCU_IMAGE_SIZE
#define DDR_LPMCU_IMAGE_SIZE 0x40000 
#endif 

#ifndef DDR_EMU_HAC_LOAD_SIZE
#define DDR_EMU_HAC_LOAD_SIZE 0x180000 
#endif 

#ifndef DDR_MTD_MEM_SIZE
#define DDR_MTD_MEM_SIZE 0x0 
#endif 

#ifndef DDR_MCORE_UNCACHE_SIZE
#define DDR_MCORE_UNCACHE_SIZE 0x1F00000 
#endif 

#ifndef DDR_MCORE_NR_UNCACHE_SIZE
#define DDR_MCORE_NR_UNCACHE_SIZE 0x00700000 
#endif 

#ifndef DDR_TLPHY_IMAGE_ADDR
#define DDR_TLPHY_IMAGE_ADDR ((DDR_MEM_ADDR)) 
#endif 

#ifndef DDR_NRPHY_IMAGE_ADDR
#define DDR_NRPHY_IMAGE_ADDR ((DDR_TLPHY_IMAGE_ADDR)+(DDR_TLPHY_IMAGE_SIZE)) 
#endif 

#ifndef DDR_MCORE_ADDR
#define DDR_MCORE_ADDR ((DDR_NRPHY_IMAGE_ADDR)+(DDR_NRPHY_IMAGE_SIZE)) 
#endif 

#ifndef DDR_MCORE_NR_ADDR
#define DDR_MCORE_NR_ADDR ((DDR_MCORE_ADDR)+(DDR_MCORE_SIZE)) 
#endif 

#ifndef DDR_MCORE_DTS_ADDR
#define DDR_MCORE_DTS_ADDR ((DDR_MCORE_NR_ADDR)+(DDR_MCORE_NR_SIZE)) 
#endif 

#ifndef DDR_GU_ADDR
#define DDR_GU_ADDR ((DDR_MCORE_DTS_ADDR)+(DDR_MCORE_DTS_SIZE)) 
#endif 

#ifndef DDR_LPHY_SDR_ADDR
#define DDR_LPHY_SDR_ADDR ((DDR_GU_ADDR)+(DDR_GU_SIZE)) 
#endif 

#ifndef DDR_LCS_ADDR
#define DDR_LCS_ADDR ((DDR_LPHY_SDR_ADDR)+(DDR_LPHY_SDR_SIZE)) 
#endif 

#ifndef DDR_SDR_NR_ADDR
#define DDR_SDR_NR_ADDR ((DDR_LCS_ADDR)+(DDR_LCS_SIZE)) 
#endif 

#ifndef DDR_CBBE_IMAGE_ADDR
#define DDR_CBBE_IMAGE_ADDR ((DDR_SDR_NR_ADDR)+(DDR_SDR_NR_SIZE)) 
#endif 

#ifndef DDR_HARQ_UP_ADDR
#define DDR_HARQ_UP_ADDR ((DDR_CBBE_IMAGE_ADDR)+(DDR_CBBE_IMAGE_SIZE)) 
#endif 

#ifndef DDR_HARQ_NRUL_ADDR
#define DDR_HARQ_NRUL_ADDR ((DDR_HARQ_UP_ADDR)+(DDR_HARQ_UP_SIZE)) 
#endif 

#ifndef DDR_RFIC_SUB6G_IMAGE_ADDR
#define DDR_RFIC_SUB6G_IMAGE_ADDR ((DDR_HARQ_NRUL_ADDR)+(DDR_HARQ_NRUL_SIZE)) 
#endif 

#ifndef DDR_RFIC_HF_IMAGE_ADDR
#define DDR_RFIC_HF_IMAGE_ADDR ((DDR_RFIC_SUB6G_IMAGE_ADDR)+(DDR_RFIC_SUB6G_IMAGE_SIZE)) 
#endif 

#ifndef DDR_RFIC_DUMP_ADDR
#define DDR_RFIC_DUMP_ADDR ((DDR_RFIC_HF_IMAGE_ADDR)+(DDR_RFIC_HF_IMAGE_SIZE)) 
#endif 

#ifndef DDR_SEC_SHARED_ADDR
#define DDR_SEC_SHARED_ADDR ((DDR_RFIC_DUMP_ADDR)+(DDR_RFIC_DUMP_SIZE)) 
#endif 

#ifndef DDR_PDE_IMAGE_ADDR
#define DDR_PDE_IMAGE_ADDR ((DDR_SEC_SHARED_ADDR)+(DDR_SEC_SHARED_SIZE)) 
#endif 

#ifndef DDR_MAA_MDM_ADDR
#define DDR_MAA_MDM_ADDR ((DDR_PDE_IMAGE_ADDR)+(DDR_PDE_IMAGE_SIZE)) 
#endif 

#ifndef DDR_EMU_HAC_LOAD_ADDR
#define DDR_EMU_HAC_LOAD_ADDR ((DDR_MAA_MDM_ADDR)+(DDR_MAA_MDM_SIZE)) 
#endif 

#ifndef DDR_MTD_MEM_ADDR
#define DDR_MTD_MEM_ADDR ((DDR_EMU_HAC_LOAD_ADDR)+(DDR_EMU_HAC_LOAD_SIZE)) 
#endif 

#ifndef DDR_FULLSTACK_MEM_ADDR
#define DDR_FULLSTACK_MEM_ADDR ((DDR_MTD_MEM_ADDR)+(DDR_MTD_MEM_SIZE)) 
#endif 

#ifndef DDR_LPMCU_IMAGE_ADDR
#define DDR_LPMCU_IMAGE_ADDR ((DDR_FULLSTACK_MEM_ADDR)+(DDR_FULLSTACK_MEM_SIZE)) 
#endif 

#ifndef DDR_LRCCPU_DTS_SIZE
#define DDR_LRCCPU_DTS_SIZE 0x40000 
#endif 

#ifndef DDR_LPMCU_DTS_SIZE
#define DDR_LPMCU_DTS_SIZE 0x18000 
#endif 

#ifndef DDR_NRCCPU_DTS_SIZE
#define DDR_NRCCPU_DTS_SIZE 0x40000 
#endif 

#ifndef DDR_L2CPU_DTS_SIZE
#define DDR_L2CPU_DTS_SIZE 0x18000 
#endif 

#ifndef DDR_MDTS_TOTAL_SIZE
#define DDR_MDTS_TOTAL_SIZE ((DDR_MCORE_DTS_SIZE)) 
#endif 

#ifndef DDR_LRCCPU_DTS_ADDR
#define DDR_LRCCPU_DTS_ADDR ((DDR_MCORE_DTS_ADDR)) 
#endif 

#ifndef DDR_LPMCU_DTS_ADDR
#define DDR_LPMCU_DTS_ADDR ((DDR_LRCCPU_DTS_ADDR) + (DDR_LRCCPU_DTS_SIZE)) 
#endif 

#ifndef DDR_NRCCPU_DTS_ADDR
#define DDR_NRCCPU_DTS_ADDR ((DDR_LPMCU_DTS_ADDR) + (DDR_LPMCU_DTS_SIZE)) 
#endif 

#ifndef DDR_L2CPU_DTS_ADDR
#define DDR_L2CPU_DTS_ADDR ((DDR_NRCCPU_DTS_ADDR) + (DDR_NRCCPU_DTS_SIZE)) 
#endif 

#ifdef CONFIG_ENABLE_DTO 
#ifndef DDR_DTB_OVL_SIZE
#define DDR_DTB_OVL_SIZE 0x2000 
#endif 

#ifndef DDR_IMAGE_DTBO_SIZE
#define DDR_IMAGE_DTBO_SIZE 0xC8000 
#endif 

#ifndef DDR_LRCCPU_DTBO_SIZE
#define DDR_LRCCPU_DTBO_SIZE 0x2000 
#endif 

#ifndef DDR_LPMCU_DTBO_SIZE
#define DDR_LPMCU_DTBO_SIZE 0x2000 
#endif 

#ifndef DDR_NRCCPU_DTBO_SIZE
#define DDR_NRCCPU_DTBO_SIZE 0x2000 
#endif 

#ifndef DDR_L2CPU_DTBO_SIZE
#define DDR_L2CPU_DTBO_SIZE 0x2000 
#endif 

#ifndef DDR_MDTS_COMMON_SIZE
#define DDR_MDTS_COMMON_SIZE ((DDR_LRCCPU_DTS_SIZE) + (DDR_LPMCU_DTS_SIZE) + (DDR_NRCCPU_DTS_SIZE) + (DDR_L2CPU_DTS_SIZE)) 
#endif 

#ifndef DDR_MDTS_OVERLAY_SIZE
#define DDR_MDTS_OVERLAY_SIZE ((DDR_IMAGE_DTBO_SIZE) + (DDR_LRCCPU_DTBO_SIZE) + (DDR_LPMCU_DTBO_SIZE) + (DDR_NRCCPU_DTBO_SIZE) + (DDR_L2CPU_DTBO_SIZE)) 
#endif 

#ifndef DDR_IMAGE_DTBO_ADDR
#define DDR_IMAGE_DTBO_ADDR ((DDR_L2CPU_DTS_ADDR) + (DDR_L2CPU_DTS_SIZE)) 
#endif 

#ifndef DDR_LRCCPU_DTBO_ADDR
#define DDR_LRCCPU_DTBO_ADDR ((DDR_IMAGE_DTBO_ADDR) + (DDR_IMAGE_DTBO_SIZE)) 
#endif 

#ifndef DDR_LPMCU_DTBO_ADDR
#define DDR_LPMCU_DTBO_ADDR ((DDR_LRCCPU_DTBO_ADDR) + (DDR_LRCCPU_DTBO_SIZE)) 
#endif 

#ifndef DDR_NRCCPU_DTBO_ADDR
#define DDR_NRCCPU_DTBO_ADDR ((DDR_LPMCU_DTBO_ADDR) + (DDR_LPMCU_DTBO_SIZE)) 
#endif 

#ifndef DDR_L2CPU_DTBO_ADDR
#define DDR_L2CPU_DTBO_ADDR ((DDR_NRCCPU_DTBO_ADDR) + (DDR_NRCCPU_DTBO_SIZE)) 
#endif 

#endif
#ifndef MEM_ADJUST_INTERCEPT
#define MEM_ADJUST_INTERCEPT 
#endif 

#ifndef MCORE_TEXT_START_ADDR
#define MCORE_TEXT_START_ADDR ((DDR_MCORE_ADDR)) 
#endif 

#ifndef FASTBOOT_DDR_ENTRY
#define FASTBOOT_DDR_ENTRY ((DDR_FASTBOOT_ADDR)) 
#endif 

#ifndef PRODUCT_CFG_KERNEL_ENTRY
#define PRODUCT_CFG_KERNEL_ENTRY ((DDR_ACORE_ADDR)+0x80000-0x8000) 
#endif 

#ifndef PRODUCT_KERNEL_PARAMS_PHYS
#define PRODUCT_KERNEL_PARAMS_PHYS ((DDR_ACORE_ADDR)+0x100) 
#endif 

#ifndef ONCHIP_FASTBOOT_ADDR
#define ONCHIP_FASTBOOT_ADDR ((MCORE_TEXT_START_ADDR)+0x100000-0x1000) 
#endif 

#ifndef DDR_MCORE_UNCACHE_ADDR
#define DDR_MCORE_UNCACHE_ADDR ( (DDR_MCORE_ADDR   ) + (DDR_MCORE_SIZE) - (DDR_MCORE_UNCACHE_SIZE)) 
#endif 

#ifndef DDR_MCORE_NR_UNCACHE_ADDR
#define DDR_MCORE_NR_UNCACHE_ADDR ( (DDR_MCORE_NR_ADDR   ) + (DDR_MCORE_NR_SIZE) - (DDR_MCORE_NR_UNCACHE_SIZE)) 
#endif 

#ifndef MODEM_SANITIZER_ADDR_OFFSET
#define MODEM_SANITIZER_ADDR_OFFSET 0 
#endif 

#ifndef MODEM_SANITIZER_NR_ADDR_OFFSET
#define MODEM_SANITIZER_NR_ADDR_OFFSET 0 
#endif 

#ifndef MDM_SANTIZIER_MEM_SIZE
#define MDM_SANTIZIER_MEM_SIZE 0 
#endif 

#ifndef MDM_SANTIZIER_NR_MEM_SIZE
#define MDM_SANTIZIER_NR_MEM_SIZE 0 
#endif 

#ifndef MDM_SANTIZIER_MEM_ADDR
#define MDM_SANTIZIER_MEM_ADDR 0 
#endif 

#ifndef MDM_SANTIZIER_NR_MEM_ADDR
#define MDM_SANTIZIER_NR_MEM_ADDR 0 
#endif 

#ifndef DDR_UPA_ADDR
#define DDR_UPA_ADDR DDR_GU_ADDR 
#endif 

#ifndef DDR_UPA_SIZE
#define DDR_UPA_SIZE 0x00024000 
#endif 

#ifndef DDR_CQI_ADDR
#define DDR_CQI_ADDR ((DDR_UPA_ADDR) + (DDR_UPA_SIZE)) 
#endif 

#ifndef DDR_CQI_SIZE
#define DDR_CQI_SIZE 0x00003400 
#endif 

#ifndef DDR_ZSP_UP_ADDR
#define DDR_ZSP_UP_ADDR ((DDR_CQI_ADDR) + (DDR_CQI_SIZE)) 
#endif 

#ifndef DDR_ZSP_UP_SIZE
#define DDR_ZSP_UP_SIZE 0x00008000 
#endif 

#ifndef DDR_ZSP_UP_1_ADDR
#define DDR_ZSP_UP_1_ADDR ((DDR_ZSP_UP_ADDR) + (DDR_ZSP_UP_SIZE)) 
#endif 

#ifndef DDR_ZSP_UP_1_SIZE
#define DDR_ZSP_UP_1_SIZE 0x00008000 
#endif 

#ifndef SHM_SEC_SIZE          		
#define SHM_SEC_SIZE          		 (DDR_SEC_SHARED_SIZE) 
#endif 

#ifndef SHM_SIZE_PROTECT_BARRIER
#define SHM_SIZE_PROTECT_BARRIER (0x1000) 
#endif 

#ifndef SHM_SIZE_PARAM_CFG
#define SHM_SIZE_PARAM_CFG (0x4000) 
#endif 

#ifndef SHM_SIZE_SEC_ICC
#define SHM_SIZE_SEC_ICC (0x20000) 
#endif 

#ifndef SHM_SIZE_SEC_RESERVED
#define SHM_SIZE_SEC_RESERVED (0xb000) 
#endif 

#ifndef SHM_SIZE_SEC_MDMA9_PM_BOOT
#define SHM_SIZE_SEC_MDMA9_PM_BOOT (0x2000) 
#endif 

#ifndef SHM_SIZE_SEC_CERT			
#define SHM_SIZE_SEC_CERT			 (0) 
#endif 

#ifndef SHM_SIZE_SEC_DUMP			
#define SHM_SIZE_SEC_DUMP			 (0x24000) 
#endif 

#ifndef SHM_SIZE_SEC_MALLOC
#define SHM_SIZE_SEC_MALLOC (0x80000) 
#endif 

#ifndef MODEM_SHARED_DDR_BASE
#define MODEM_SHARED_DDR_BASE (0x18000000) 
#endif 

#ifndef MODEM_NSRO_SHARED_MEM_BASE
#define MODEM_NSRO_SHARED_MEM_BASE (MODEM_SHARED_DDR_BASE) 
#endif 

#ifndef DDR_NSRO_SHARED_MEM_SIZE
#define DDR_NSRO_SHARED_MEM_SIZE 0xC00000 
#endif 

#ifndef DDR_SHA_NV_SIZE
#define DDR_SHA_NV_SIZE 0xA00000 
#endif 

#ifndef DDR_SHA_SEC_MEM_SIZE
#define DDR_SHA_SEC_MEM_SIZE ((DDR_NSRO_SHARED_MEM_SIZE) - (DDR_SHA_NV_SIZE)) 
#endif 

#ifndef DDR_SHA_MEM_SIZE
#define DDR_SHA_MEM_SIZE 0x100000 
#endif 

#ifndef DDR_MCORE_NR_SHARED_MEM_SIZE
#define DDR_MCORE_NR_SHARED_MEM_SIZE 0x100000 
#endif 

#ifndef DDR_SHARED_MEM_SIZE
#define DDR_SHARED_MEM_SIZE ((DDR_SHA_MEM_SIZE)+(DDR_NSRO_SHARED_MEM_SIZE)) 
#endif 

#ifndef DDR_MNTN_SIZE
#define DDR_MNTN_SIZE 0x300000 
#endif 

#ifndef DDR_SHARED_MEM_ADDR
#define DDR_SHARED_MEM_ADDR ( MODEM_NSRO_SHARED_MEM_BASE ) 
#endif 

#ifndef DDR_SHAREDNR_MEM_ADDR
#define DDR_SHAREDNR_MEM_ADDR ((DDR_SHARED_MEM_ADDR      ) + (DDR_SHARED_MEM_SIZE      )) 
#endif 

#ifndef DDR_MNTN_ADDR
#define DDR_MNTN_ADDR ( (DDR_SHAREDNR_MEM_ADDR      ) + (DDR_MCORE_NR_SHARED_MEM_SIZE  ) ) 
#endif 

#ifndef MODEM_SOCP_DDR_BASE
#define MODEM_SOCP_DDR_BASE (0xAE180000) 
#endif 

#ifndef DDR_SOCP_SIZE
#define DDR_SOCP_SIZE 0x8000000 
#endif 

#ifndef DDR_HIFI_SIZE
#define DDR_HIFI_SIZE 0x0 
#endif 

#ifndef DDR_SOCP_ADDR
#define DDR_SOCP_ADDR ( MODEM_SOCP_DDR_BASE) 
#endif 

#ifndef DDR_HIFI_ADDR
#define DDR_HIFI_ADDR ( (DDR_SOCP_ADDR            ) + (DDR_SOCP_SIZE            ) ) 
#endif 

#ifndef NV_MBN_MAX_SIZE
#define NV_MBN_MAX_SIZE 0x20000 
#endif 

#ifndef NV_DDR_SIZE
#define NV_DDR_SIZE (DDR_SHA_NV_SIZE) 
#endif 

#ifndef NV_COMM_BIN_FILE_MAX_SIZE
#define NV_COMM_BIN_FILE_MAX_SIZE 0x9DFC00 
#endif 

#ifndef SHA_SEC_ICC_DDR_SIZE
#define SHA_SEC_ICC_DDR_SIZE (0x50000) 
#endif 

#ifndef SHM_SIZE_NR_PM
#define SHM_SIZE_NR_PM (NRCCPU_LLRAM_BASE_SIZE) 
#endif 

#ifndef SHM_SIZE_HAC_PM
#define SHM_SIZE_HAC_PM (0x80000) 
#endif 

#ifndef SHM_SIZE_PM_SCTRL
#define SHM_SIZE_PM_SCTRL (0x8000) 
#endif 

#ifndef SHM_SIZE_NSRO_RSV
#define SHM_SIZE_NSRO_RSV (0x88000) 
#endif 

#ifndef DDR_HIFI_MBX_ADDR
#define DDR_HIFI_MBX_ADDR ((DDR_SHARED_MEM_ADDR) + (DDR_NSRO_SHARED_MEM_SIZE)) 
#endif 

#ifndef DDR_HIFI_MBX_SIZE
#define DDR_HIFI_MBX_SIZE (0x9800) 
#endif 

#ifndef SHM_SIZE_HIFI_MBX
#define SHM_SIZE_HIFI_MBX (DDR_HIFI_MBX_SIZE) 
#endif 

#ifndef SHM_SIZE_HIFI
#define SHM_SIZE_HIFI (0) 
#endif 

#ifndef SHM_SIZE_TLPHY
#define SHM_SIZE_TLPHY (12*1024) 
#endif 

#ifndef SHM_SIZE_TEMPERATURE
#define SHM_SIZE_TEMPERATURE (1*1024) 
#endif 

#ifndef SHM_SIZE_DDM_LOAD
#define SHM_SIZE_DDM_LOAD (0) 
#endif 

#ifndef SHM_SIZE_MEM_APPA9_PM_BOOT
#define SHM_SIZE_MEM_APPA9_PM_BOOT (0) 
#endif 

#ifndef SHM_SIZE_MEM_MDMA9_PM_BOOT
#define SHM_SIZE_MEM_MDMA9_PM_BOOT (0) 
#endif 

#ifndef SHM_SIZE_TENCILICA_MULT_BAND
#define SHM_SIZE_TENCILICA_MULT_BAND (0) 
#endif 

#ifndef SHM_SIZE_ICC
#define SHM_SIZE_ICC (0x79800) 
#endif 

#ifndef SHM_SIZE_IPF
#define SHM_SIZE_IPF (0) 
#endif 

#ifndef SHM_SIZE_PSAM
#define SHM_SIZE_PSAM (0) 
#endif 

#ifndef SHM_SIZE_WAN
#define SHM_SIZE_WAN (0) 
#endif 

#ifndef SHM_SIZE_NV_UNSAFE
#define SHM_SIZE_NV_UNSAFE (0x400) 
#endif 

#ifndef SHM_SIZE_NV
#define SHM_SIZE_NV (NV_DDR_SIZE) 
#endif 

#ifndef SHM_SIZE_M3_MNTN
#define SHM_SIZE_M3_MNTN (0) 
#endif 

#ifndef SHM_SIZE_TIMESTAMP
#define SHM_SIZE_TIMESTAMP (1*1024) 
#endif 

#ifndef SHM_SIZE_IOS
#define SHM_SIZE_IOS (6*1024) 
#endif 

#ifndef SHM_SIZE_RESTORE_AXI
#define SHM_SIZE_RESTORE_AXI (CCPU_LLRAM_BASE_SIZE) 
#endif 

#ifndef SHM_SIZE_PMU
#define SHM_SIZE_PMU (3*1024) 
#endif 

#ifndef SHM_SIZE_PTABLE
#define SHM_SIZE_PTABLE (0) 
#endif 

#ifndef SHM_SIZE_CCORE_RESET
#define SHM_SIZE_CCORE_RESET (0x400) 
#endif 

#ifndef SHM_SIZE_PM_OM
#define SHM_SIZE_PM_OM (256*1024) 
#endif 

#ifndef SHM_SIZE_M3PM
#define SHM_SIZE_M3PM (0x1000) 
#endif 

#ifndef SHM_SIZE_SLICE_MEM
#define SHM_SIZE_SLICE_MEM (0x1000) 
#endif 

#ifndef SHM_SIZE_OSA_LOG
#define SHM_SIZE_OSA_LOG (0) 
#endif 

#ifndef SHM_SIZE_WAS_LOG
#define SHM_SIZE_WAS_LOG (0) 
#endif 

#ifndef SHM_SIZE_SRAM_BAK
#define SHM_SIZE_SRAM_BAK (0) 
#endif 

#ifndef SHM_SIZE_SRAM_TO_DDR
#define SHM_SIZE_SRAM_TO_DDR (0) 
#endif 

#ifndef SHM_SIZE_M3RSRACC_BD
#define SHM_SIZE_M3RSRACC_BD (0) 
#endif 

#ifndef SHM_SIZE_SIM_MEMORY
#define SHM_SIZE_SIM_MEMORY (0) 
#endif 

#ifndef SHM_SIZE_PRODUCT_MEM
#define SHM_SIZE_PRODUCT_MEM (0) 
#endif 

#ifndef SHM_SIZE_SHA_SEC_ICC
#define SHM_SIZE_SHA_SEC_ICC (SHA_SEC_ICC_DDR_SIZE) 
#endif 

#ifndef NRSHM_SIZE_ICC
#define NRSHM_SIZE_ICC (0xC0000) 
#endif 

#ifndef FEATURE_NV_PARTRION_MULTIPLEX
#define FEATURE_NV_PARTRION_MULTIPLEX FEATURE_ON 
#endif 

#ifndef AXIMEM_BASE_ADDR
#define AXIMEM_BASE_ADDR (0xfe380000) 
#endif 

#ifndef AXIMEM_MAX_LEN
#define AXIMEM_MAX_LEN (128*1024) 
#endif 

#ifndef AXIMEM_CACHE_MAX_LEN
#define AXIMEM_CACHE_MAX_LEN (4*1024) 
#endif 

#ifndef AXIMEM_UNCACHE_MAX_LEN
#define AXIMEM_UNCACHE_MAX_LEN (AXIMEM_MAX_LEN - AXIMEM_CACHE_MAX_LEN) 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
