/* MD5: 3d4e2ace34f6403750a448cb31952a27*/
#if !defined(__PRODUCT_CONFIG_DRV_H__)
#define __PRODUCT_CONFIG_DRV_H__

#ifndef ENABLE_BUILD_VARS
#define ENABLE_BUILD_VARS 
#endif 

#ifndef FEATURE_UPGRADE_TL
#define FEATURE_UPGRADE_TL 
#endif 

#ifndef PRODUCT_NAME
#define PRODUCT_NAME "kirin710" 
#endif 

#ifndef PRODUCT_CFG_CHIP_SOLUTION_NAME
#define PRODUCT_CFG_CHIP_SOLUTION_NAME "Balong" 
#endif 

#ifndef CCPU_OS
#define CCPU_OS RTOSCK_SMP 
#endif 

#ifndef HCC_VERSION
#define HCC_VERSION 4.9 
#endif 

#ifndef PLATFORM
#define PLATFORM kirin710 
#endif 

#ifndef PLATFORM_HISI_BALONG
#define PLATFORM_HISI_BALONG kirin710 
#endif 

#ifndef CONFIG_VERSION_STUB
#endif 

#ifndef HW_VERSION_STUB
#define HW_VERSION_STUB 0x38F5FC04 
#endif 

#ifndef CONFIG_PLAT_PORTING				
#endif 

#ifndef BSP_CONFIG_EDA
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

#ifndef BSP_CONFIG_BOARD_SFT
#endif 

#ifndef BSP_HAS_SEC_FEATURE
#endif 

#ifndef CONFIG_SMART_SYSTEM_MODEM
#endif 

#ifndef CONFIG_FASTBOOT_DEBUG
#endif 

#ifndef CONFIG_OF
#define CONFIG_OF 
#endif 

#ifndef FEATURE_BALONG_MODEM_ATE
#define FEATURE_BALONG_MODEM_ATE 
#endif 

#ifndef CONFIG_RFILE_STUB
#endif 

#ifndef CONFIG_RINGBUFFER_STUB
#endif 

#ifndef CONFIG_CONSOLE_STUB
#endif 

#ifndef CCPU_CORE_NUM
#define CCPU_CORE_NUM 1 
#endif 

#ifndef CCPU_LLRAM_BASE_ADDR
#define CCPU_LLRAM_BASE_ADDR 0xE0800000 
#endif 

#ifndef CCPU_LLRAM_BASE_SIZE
#define CCPU_LLRAM_BASE_SIZE 0x20000 
#endif 

#ifndef CCPU_SRAM_SIZE
#define CCPU_SRAM_SIZE 0x3000 
#endif 

#ifndef DFC_LLRAM_BASE_ADDR
#define DFC_LLRAM_BASE_ADDR (CCPU_LLRAM_BASE_ADDR + CCPU_SRAM_SIZE) 
#endif 

#ifndef DFC_LLRAM_SIZE
#define DFC_LLRAM_SIZE 0x1c800 
#endif 

#ifndef MAILBOX_LLRAM_BASE_ADDR
#define MAILBOX_LLRAM_BASE_ADDR DFC_LLRAM_BASE_ADDR 
#endif 

#ifndef MAILBOX_LLRAM_SIZE
#define MAILBOX_LLRAM_SIZE 0x0000 
#endif 

#ifndef CPU_VIEW_LLRAM_ADDR
#define CPU_VIEW_LLRAM_ADDR ((MAILBOX_LLRAM_BASE_ADDR) + (MAILBOX_LLRAM_SIZE)) 
#endif 

#ifndef CPU_VIEW_LLRAM_SIZE
#define CPU_VIEW_LLRAM_SIZE 0x3000 
#endif 

#ifndef SOCP_LLRAM_ADDR
#define SOCP_LLRAM_ADDR ((CPU_VIEW_LLRAM_ADDR) + (CPU_VIEW_LLRAM_SIZE)) 
#endif 

#ifndef SOCP_LLRAM_SIZE
#define SOCP_LLRAM_SIZE 0x8000 
#endif 

#ifndef LL2_LLRAM_BASE_ADDR
#define LL2_LLRAM_BASE_ADDR ((SOCP_LLRAM_ADDR) + (SOCP_LLRAM_SIZE)) 
#endif 

#ifndef LL2_LLRAM_SIZE
#define LL2_LLRAM_SIZE 0x11800 
#endif 

#ifndef CONFIG_CCPU_HAS_LLRAM
#define CONFIG_CCPU_HAS_LLRAM 
#endif 

#ifndef CCPU_LLRAM_ADDR
#define CCPU_LLRAM_ADDR (DFC_LLRAM_BASE_ADDR + DFC_LLRAM_SIZE) 
#endif 

#ifndef CCPU_LLRAM_SIZE
#define CCPU_LLRAM_SIZE (CCPU_LLRAM_BASE_SIZE - CCPU_SRAM_SIZE - DFC_LLRAM_SIZE) 
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

#ifndef CONFIG_NVIM
#define CONFIG_NVIM 
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

#ifndef FEATURE_NV_CARRIER_CUST
#define FEATURE_NV_CARRIER_CUST 
#endif 

#ifndef FEATURE_NV_NO_MODEMNVM_SYS
#define FEATURE_NV_NO_MODEMNVM_SYS 
#endif 

#ifndef FEATURE_NV_SEC_ON
#endif 

#ifndef FEATURE_KEEP_LEGICY_SEC_OS
#define FEATURE_KEEP_LEGICY_SEC_OS 
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

#ifndef FEATURE_OM_PHONE
#endif 

#ifndef ENABLE_BUILD_SYSVIEW
#endif 

#ifndef ENABLE_BUILD_CPUVIEW
#endif 

#ifndef ENABLE_BUILD_MEMVIEW
#endif 

#ifndef ENABLE_BUILD_AMON_SOC
#endif 

#ifndef ENABLE_BUILD_AMON_MDM
#define ENABLE_BUILD_AMON_MDM 
#endif 

#ifndef ENABLE_BUILD_UTRACE
#endif 

#ifndef ENABLE_BUILD_SOCP
#define ENABLE_BUILD_SOCP 
#endif 

#ifndef CONFIG_DIAG_SYSTEM
#define CONFIG_DIAG_SYSTEM 
#endif 

#ifndef CONFIG_FILEBROSWER
#endif 

#ifndef FEATURE_SOCP_ADDR_64BITS
#define FEATURE_SOCP_ADDR_64BITS 
#endif 

#ifndef SOCP_DECODE_ENABLE
#define SOCP_DECODE_ENABLE 
#endif 

#ifndef CONFIG_DEFLATE
#endif 

#ifndef ENABLE_DIAG_FIX_ADDR
#endif 

#ifndef FEATURE_SOCP_DECODE_INT_TIMEOUT
#define FEATURE_SOCP_DECODE_INT_TIMEOUT FEATURE_OFF 
#endif 

#ifndef FEATURE_SOCP_ON_DEMAND			
#define FEATURE_SOCP_ON_DEMAND			 FEATURE_OFF 
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

#ifndef CONFIG_NOC
#define CONFIG_NOC 
#endif 

#ifndef CONFIG_NOC_AP
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

#ifndef CONFIG_CBBE
#define CONFIG_CBBE 
#endif 

#ifndef CONFIG_PDLOCK
#endif 

#ifndef CONFIG_CORESIGHT
#define CONFIG_CORESIGHT 
#endif 

#ifndef CONFIG_WATCHPOINT
#endif 

#ifndef CONFIG_DSPDVS
#define CONFIG_DSPDVS 
#endif 

#ifndef FEATURE_DSP_DFS
#define FEATURE_DSP_DFS FEATURE_ON 
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
#define CONFIG_DFS_DDR 
#endif 

#ifndef CONFIG_DFS_NOC
#endif 

#ifndef CONFIG_DFS_DSP
#define CONFIG_DFS_DSP 
#endif 

#ifndef MIAMI_ATE
#define MIAMI_ATE 
#endif 

#ifndef FEATURE_TCM_RETENTION
#define FEATURE_TCM_RETENTION FEATURE_OFF 
#endif 

#ifndef RFIC_CLK_STEADY_SLICE
#define RFIC_CLK_STEADY_SLICE 3 
#endif 

#ifndef FEATURE_MULTI_CHANNEL			
#define FEATURE_MULTI_CHANNEL			 FEATURE_OFF 
#endif 

#ifndef CONFIG_VER_3_TLBBP_DS_BOSTON_AND_LATER
#endif 

#ifndef CONFIG_LOAD_SEC_IMAGE
#define CONFIG_LOAD_SEC_IMAGE 
#endif 

#ifndef CONFIG_COMPRESS_CCORE_IMAGE
#define CONFIG_COMPRESS_CCORE_IMAGE 
#endif 

#ifndef CONFIG_COMPRESS_DTB_IMAGE
#define CONFIG_COMPRESS_DTB_IMAGE 
#endif 

#ifndef CONFIG_IS_DTB_VERIFY
#define CONFIG_IS_DTB_VERIFY 
#endif 

#ifndef CONFIG_MODEM_DTB_LOAD_IN_KERNEL
#define CONFIG_MODEM_DTB_LOAD_IN_KERNEL 
#endif 

#ifndef CONFIG_PHY_LOAD
#define CONFIG_PHY_LOAD 
#endif 

#ifndef FEATURE_DELAY_MODEM_INIT
#define FEATURE_DELAY_MODEM_INIT FEATURE_ON 
#endif 

#ifndef CONFIG_MODEM_PINTRL
#define CONFIG_MODEM_PINTRL 
#endif 

#ifndef CONFIG_GPIO_PL061
#define CONFIG_GPIO_PL061 
#endif 

#ifndef CONFIG_RFFE_MIPI
#define CONFIG_RFFE_MIPI 
#endif 

#ifndef CONFIG_RFFE_ANTEN
#define CONFIG_RFFE_ANTEN 
#endif 

#ifndef CONFIG_RFFE_POWER
#define CONFIG_RFFE_POWER 
#endif 

#ifndef CONFIG_ABB
#define CONFIG_ABB 
#endif 

#ifndef CONFIG_HIFI
#endif 

#ifndef CONFIG_BBP
#define CONFIG_BBP 
#endif 

#ifndef CONFIG_HW_SPINLOCK
#define CONFIG_HW_SPINLOCK 
#endif 

#ifndef MODEM_MEM_REPAIR
#endif 

#ifndef CONFIG_MIPI
#define CONFIG_MIPI 
#endif 

#ifndef CONFIG_LEDS_CCORE
#endif 

#ifndef CONFIG_ADC
#define CONFIG_ADC 
#endif 

#ifndef CONFIG_EFUSE
#define CONFIG_EFUSE 
#endif 

#ifndef CONFIG_TSENSOR
#endif 

#ifndef CONFIG_ONOFF
#define CONFIG_ONOFF 
#endif 

#ifndef RFIC_HI6363_TCVR
#define RFIC_HI6363_TCVR 
#endif 

#ifndef CONFIG_RTC_ON_SOC
#define CONFIG_RTC_ON_SOC 
#endif 

#ifndef STACK_CANARY_COMPILE
#define STACK_CANARY_COMPILE 
#endif 

#ifndef CONFIG_COLD_PATCH
#define CONFIG_COLD_PATCH 
#endif 

#ifndef CONFIG_SYSBUS
#endif 

#ifndef CONFIG_CCPU_FIQ_SMP
#define CONFIG_CCPU_FIQ_SMP 
#endif 

#ifndef OS_K3V3_USE_LPM3_API
#endif 

#ifndef BSP_ICC_MCHANNEL_USE_LPM3TCM
#define BSP_ICC_MCHANNEL_USE_LPM3TCM 
#endif 

#ifndef CONFIG_BALONG_L2CACHE
#define CONFIG_BALONG_L2CACHE 
#endif 

#ifndef CONFIG_FASTBOOT_UART_NUM
#define CONFIG_FASTBOOT_UART_NUM 0 
#endif 

#ifndef CONFIG_MODULE_VIC
#endif 

#ifndef CONFIG_AT_UART
#endif 

#ifndef CONFIG_ICC
#define CONFIG_ICC 
#endif 

#ifndef CONFIG_ARM_ENABLE_DTS
#endif 

#ifndef DTS_NAME
#define DTS_NAME v711 
#endif 

#ifndef DTS_STATIC_MEM_SIZE
#define DTS_STATIC_MEM_SIZE 0X5000 
#endif 

#ifndef CONFIG_CPUFREQ
#define CONFIG_CPUFREQ 
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

#ifndef CONFIG_PM_OM
#define CONFIG_PM_OM 
#endif 

#ifndef CONFIG_BALONG_CCLK
#define CONFIG_BALONG_CCLK 
#endif 

#ifndef CONFIG_CCORE_CPU_IDLE
#define CONFIG_CCORE_CPU_IDLE 
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

#ifndef FEATURE_CPU_PRIVATE_SLICE
#define FEATURE_CPU_PRIVATE_SLICE 
#endif 

#ifndef CONFIG_MODULE_SYNC
#define CONFIG_MODULE_SYNC 
#endif 

#ifndef CONFIG_CCORE_CPU
#define CONFIG_CCORE_CPU 
#endif 

#ifndef CONFIG_CCORE_NOTIFIER
#define CONFIG_CCORE_NOTIFIER 
#endif 

#ifndef CONFIG_IPC_MSG
#define CONFIG_IPC_MSG 
#endif 

#ifndef CONFIG_IPC_MSG_AO_DISC
#define CONFIG_IPC_MSG_AO_DISC 
#endif 

#ifndef CONFIG_DFC
#endif 

#ifndef CONFIG_HAS_CCORE_WAKELOCK
#define CONFIG_HAS_CCORE_WAKELOCK 
#endif 

#ifndef CONFIG_CCORE_BALONG_PM
#define CONFIG_CCORE_BALONG_PM 
#endif 

#ifndef CONFIG_BALONG_EDMA
#define CONFIG_BALONG_EDMA 
#endif 

#ifndef CONFIG_PWC_MNTN_CCORE
#endif 

#ifndef CONFIG_HWADP
#define CONFIG_HWADP 
#endif 

#ifndef CONFIG_SYSCTRL
#define CONFIG_SYSCTRL 
#endif 

#ifndef CONFIG_S_MEMORY
#endif 

#ifndef CONFIG_S_MEMORY_TEST
#endif 

#ifndef CONFIG_BALONG_HPM_TEMP
#endif 

#ifndef CONFIG_MEM
#define CONFIG_MEM 
#endif 

#ifndef CONFIG_TCXO_BALONG
#endif 

#ifndef CONFIG_BALONG_MODEM_RESET
#define CONFIG_BALONG_MODEM_RESET 
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

#ifndef CONFIG_MPERF
#endif 

#ifndef CONFIG_PMIC_FPGA
#endif 

#ifndef CONFIG_PMU_NEW
#define CONFIG_PMU_NEW 
#endif 

#ifndef CONFIG_PMU_RAMP_EN
#define CONFIG_PMU_RAMP_EN 
#endif 

#ifndef CONFIG_IPF
#define CONFIG_IPF 
#endif 

#ifndef CONFIG_PSAM
#define CONFIG_PSAM 
#endif 

#ifndef CONFIG_CIPHER
#endif 

#ifndef CONFIG_NEW_PLATFORM
#define CONFIG_NEW_PLATFORM 
#endif 

#ifndef CONFIG_CIPHER_NEW
#define CONFIG_CIPHER_NEW 
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

#ifndef CONFIG_SINGLE_CICOM
#define CONFIG_SINGLE_CICOM 
#endif 

#ifndef FEATURE_HISOCKET
#define FEATURE_HISOCKET FEATURE_OFF 
#endif 

#ifndef ENABLE_TEST_CODE
#endif 

#ifndef CONFIG_DYNAMIC_LOAD
#endif 

#ifndef MCORE_TEXT_START_ADDR
#define MCORE_TEXT_START_ADDR 0x20000000 
#endif 

#ifndef FASTBOOT_ENTRY
#define FASTBOOT_ENTRY 0x4fe00000 
#endif 

#ifndef FASTBOOT_DDR_ENTRY
#define FASTBOOT_DDR_ENTRY 0x5FD00000 
#endif 

#ifndef PRODUCT_CFG_KERNEL_ENTRY 		
#define PRODUCT_CFG_KERNEL_ENTRY 		 0x59008000 
#endif 

#ifndef PRODUCT_KERNEL_PARAMS_PHYS		
#define PRODUCT_KERNEL_PARAMS_PHYS		 0x59000100 
#endif 

#ifndef ONCHIP_FASTBOOT_ADDR
#define ONCHIP_FASTBOOT_ADDR 0x5F900000 
#endif 

#ifndef RTX_KERNEL_ENTRY
#define RTX_KERNEL_ENTRY 0x10000000 
#endif 

#ifndef DDR_MEM_ADDR
#define DDR_MEM_ADDR 0x20000000 
#endif 

#ifndef DDR_SECMEM_SIZE
#define DDR_SECMEM_SIZE 0x9500000 
#endif 

#ifndef DDR_MCORE_SIZE
#define DDR_MCORE_SIZE 0x8300000 
#endif 

#ifndef DDR_TLPHY_IMAGE_SIZE
#define DDR_TLPHY_IMAGE_SIZE 0x600000 
#endif 

#ifndef DDR_GU_SIZE
#define DDR_GU_SIZE 0x40000 
#endif 

#ifndef DDR_MCORE_DTS_SIZE
#define DDR_MCORE_DTS_SIZE 0x80000 
#endif 

#ifndef DDR_CBBE_IMAGE_SIZE
#define DDR_CBBE_IMAGE_SIZE 0x000000 
#endif 

#ifndef DDR_LPHY_SDR_SIZE
#define DDR_LPHY_SDR_SIZE 0x840000 
#endif 

#ifndef DDR_LCS_SIZE
#define DDR_LCS_SIZE 0x280000 
#endif 

#ifndef DDR_SEC_SHARED_SIZE
#define DDR_SEC_SHARED_SIZE 0x80000 
#endif 

#ifndef DDR_MCORE_ADDR
#define DDR_MCORE_ADDR ((DDR_MEM_ADDR)) 
#endif 

#ifndef DDR_TLPHY_IMAGE_ADDR
#define DDR_TLPHY_IMAGE_ADDR ((DDR_MCORE_ADDR)+(DDR_MCORE_SIZE)) 
#endif 

#ifndef DDR_GU_ADDR
#define DDR_GU_ADDR ((DDR_TLPHY_IMAGE_ADDR)+(DDR_TLPHY_IMAGE_SIZE)) 
#endif 

#ifndef DDR_MCORE_DTS_ADDR
#define DDR_MCORE_DTS_ADDR ( (DDR_GU_ADDR              ) + (DDR_GU_SIZE              ) ) 
#endif 

#ifndef DDR_CBBE_IMAGE_ADDR
#define DDR_CBBE_IMAGE_ADDR ( (DDR_MCORE_DTS_ADDR       ) + (DDR_MCORE_DTS_SIZE       ) ) 
#endif 

#ifndef DDR_LPHY_SDR_ADDR
#define DDR_LPHY_SDR_ADDR ( (DDR_CBBE_IMAGE_ADDR      ) + (DDR_CBBE_IMAGE_SIZE      ) ) 
#endif 

#ifndef DDR_LCS_ADDR
#define DDR_LCS_ADDR ( (DDR_LPHY_SDR_ADDR        ) + (DDR_LPHY_SDR_SIZE        ) ) 
#endif 

#ifndef DDR_SEC_SHARED_ADDR
#define DDR_SEC_SHARED_ADDR ( (DDR_LCS_ADDR             ) + (DDR_LCS_SIZE             ) ) 
#endif 

#ifndef DDR_MCORE_UNCACHE_SIZE
#define DDR_MCORE_UNCACHE_SIZE 0x2000000 
#endif 

#ifndef DDR_MCORE_UNCACHE_ADDR
#define DDR_MCORE_UNCACHE_ADDR (( DDR_MCORE_ADDR ) + (DDR_MCORE_SIZE) - (DDR_MCORE_UNCACHE_SIZE)) 
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
#define SHM_SIZE_SEC_DUMP			 (0) 
#endif 

#ifndef MODEM_SHARED_DDR_BASE
#define MODEM_SHARED_DDR_BASE (0x10000000) 
#endif 

#ifndef MODEM_SOCP_DDR_BASE
#define MODEM_SOCP_DDR_BASE (0x10E00000) 
#endif 

#ifndef DDR_SHA_NV_SIZE
#define DDR_SHA_NV_SIZE 0x700000 
#endif 

#ifndef DDR_SHA_MEM_SIZE
#define DDR_SHA_MEM_SIZE 0x180000 
#endif 

#ifndef DDR_SHARED_MEM_SIZE
#define DDR_SHARED_MEM_SIZE ((DDR_SHA_NV_SIZE)+(DDR_SHA_MEM_SIZE)) 
#endif 

#ifndef DDR_SOCP_SIZE
#define DDR_SOCP_SIZE 0x2000000 
#endif 

#ifndef DDR_MNTN_SIZE
#define DDR_MNTN_SIZE 0x100000 
#endif 

#ifndef DDR_HIFI_SIZE
#define DDR_HIFI_SIZE 0x0 
#endif 

#ifndef DDR_SHARED_MEM_ADDR
#define DDR_SHARED_MEM_ADDR ( (MODEM_SHARED_DDR_BASE                             ) ) 
#endif 

#ifndef DDR_SOCP_ADDR
#define DDR_SOCP_ADDR ( (MODEM_SOCP_DDR_BASE                               ) ) 
#endif 

#ifndef DDR_MNTN_ADDR
#define DDR_MNTN_ADDR ( (DDR_SOCP_ADDR        ) + (DDR_SOCP_SIZE           ) ) 
#endif 

#ifndef DDR_HIFI_ADDR
#define DDR_HIFI_ADDR ( (DDR_MNTN_ADDR        ) + (DDR_MNTN_SIZE           ) ) 
#endif 

#ifndef DDR_HIFI_MBX_ADDR
#define DDR_HIFI_MBX_ADDR ((DDR_SHARED_MEM_ADDR) + (DDR_SHA_NV_SIZE)) 
#endif 

#ifndef DDR_HIFI_MBX_SIZE
#define DDR_HIFI_MBX_SIZE (0X9800) 
#endif 

#ifndef NV_DDR_SIZE
#define NV_DDR_SIZE (DDR_SHA_NV_SIZE) 
#endif 

#ifndef NV_MBN_MAX_SIZE
#define NV_MBN_MAX_SIZE 0x20000 
#endif 

#ifndef NV_COMM_BIN_FILE_MAX_SIZE
#define NV_COMM_BIN_FILE_MAX_SIZE 0x6DFC00 
#endif 

#ifndef SHM_SIZE_HIFI_MBX
#define SHM_SIZE_HIFI_MBX (DDR_HIFI_MBX_SIZE) 
#endif 

#ifndef SHM_SIZE_HIFI
#define SHM_SIZE_HIFI (10*1024) 
#endif 

#ifndef SHM_SIZE_TLPHY
#define SHM_SIZE_TLPHY (0) 
#endif 

#ifndef SHM_SIZE_TEMPERATURE
#define SHM_SIZE_TEMPERATURE (3*1024) 
#endif 

#ifndef SHM_SIZE_DDM_LOAD
#define SHM_SIZE_DDM_LOAD (1*1024) 
#endif 

#ifndef SHM_SIZE_MEM_APPA9_PM_BOOT
#define SHM_SIZE_MEM_APPA9_PM_BOOT (0) 
#endif 

#ifndef SHM_SIZE_MEM_MDMA9_PM_BOOT
#define SHM_SIZE_MEM_MDMA9_PM_BOOT (0x2000) 
#endif 

#ifndef SHM_SIZE_TENCILICA_MULT_BAND
#define SHM_SIZE_TENCILICA_MULT_BAND (0) 
#endif 

#ifndef SHM_SIZE_ICC
#define SHM_SIZE_ICC (0x8B000) 
#endif 

#ifndef SHM_SIZE_IPF
#define SHM_SIZE_IPF (0x10000) 
#endif 

#ifndef SHM_SIZE_PSAM
#define SHM_SIZE_PSAM (0x1000) 
#endif 

#ifndef SHM_SIZE_WAN
#define SHM_SIZE_WAN (0) 
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
#define SHM_SIZE_PMU (0) 
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
#define SHM_SIZE_SIM_MEMORY (256*1024) 
#endif 

#ifndef SHM_SIZE_NV
#define SHM_SIZE_NV (NV_DDR_SIZE) 
#endif 

#ifndef FEATURE_NV_PARTRION_MULTIPLEX
#define FEATURE_NV_PARTRION_MULTIPLEX FEATURE_OFF 
#endif 

#ifndef RAT_GU
#define RAT_GU 0 
#endif 

#ifndef RAT_GUL
#define RAT_GUL 1 
#endif 

#ifndef RAT_MODE
#define RAT_MODE RAT_GUL 
#endif 

#endif /*__PRODUCT_CONFIG_H__*/ 
