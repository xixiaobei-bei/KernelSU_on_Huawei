/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef PS_LOG_DRV_FILE_ID_DEFINE_H
#define PS_LOG_DRV_FILE_ID_DEFINE_H

#include "ps_log_file_id_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* DRVÔ´ÎÄ¼þIDºÅ */
typedef enum
{
                 DRV_FILE_ID_AMBADMTIMERDRV_C = DRV_TEAM_FILE_ID,
/* 2049  */      DRV_FILE_ID_AMBAINTRCTL_C,
/* 2050  */      DRV_FILE_ID_AMBASIO_C,
/* 2051  */      DRV_FILE_ID_AMBASYSCTRL_C,
/* 2052  */      DRV_FILE_ID_AMBATIMER_C,
/* 2053  */      DRV_FILE_ID_AMBAWDT_C,
/* 2054  */      DRV_FILE_ID_AMTS_C,
/* 2055  */      DRV_FILE_ID_BOOTCONFIG_C,
/* 2056  */      DRV_FILE_ID_BOOTINIT_C,
/* 2057  */      DRV_FILE_ID_CONTRLDRV_C,
/* 2058  */      DRV_FILE_ID_DBG_C,
/* 2059  */      DRV_FILE_ID_DEVICE_C,
/* 2060  */      DRV_FILE_ID_DIRLIB_C,
/* 2061  */      DRV_FILE_ID_DMA_C,
/* 2062  */      DRV_FILE_ID_DMAPI_C,
/* 2063  */      DRV_FILE_ID_DMCORE_C,
/* 2064  */      DRV_FILE_ID_DMDRV_C,
/* 2065  */      DRV_FILE_ID_DMERTCDRV_C,
/* 2066  */      DRV_FILE_ID_DMHKADCDRV_C,
/* 2067  */      DRV_FILE_ID_DMIRTCDRV_C,
/* 2068  */      DRV_FILE_ID_DMKEYPADDRV_C,
/* 2069  */      DRV_FILE_ID_DMLIB_C,
/* 2070  */      DRV_FILE_ID_DMLOGCTRL_C,
/* 2071  */      DRV_FILE_ID_DMSPY_C,
/* 2072  */      DRV_FILE_ID_DMSPY2_C,
/* 2073  */      DRV_FILE_ID_DOSFSFAT_C,
/* 2074  */      DRV_FILE_ID_DOSVDIRLIB_C,
/* 2075  */      DRV_FILE_ID_DRVFLASH_C,
/* 2076  */      DRV_FILE_ID_DRVLOG_C,
/* 2077  */      DRV_FILE_ID_DRVRTC_C,
/* 2078  */      DRV_FILE_ID_DWLD_L18_C,
/* 2079  */      DRV_FILE_ID_DWLD_M18_C,
/* 2080  */      DRV_FILE_ID_ECSUSBDEVICEDL_C,
/* 2081  */      DRV_FILE_ID_ECSUSBDEVICE_C,
/* 2082  */      DRV_FILE_ID_ECSUSBDMADL_C,
/* 2083  */      DRV_FILE_ID_ECSUSBDMA_C,
/* 2084  */      DRV_FILE_ID_ECSUSBINITDL_C,
/* 2085  */      DRV_FILE_ID_ECSUSBINIT_C,
/* 2086  */      DRV_FILE_ID_ECSUSBNET_C,
/* 2087  */      DRV_FILE_ID_ECSUSBTARGLIBDL_C,
/* 2088  */      DRV_FILE_ID_ECSUSBTARGLIB_C,
/* 2089  */      DRV_FILE_ID_ECSVERSION_C,
/* 2090  */      DRV_FILE_ID_EDMREG_C,
/* 2091  */      DRV_FILE_ID_EDRVNAMEPUBLIC_C,
/* 2092  */      DRV_FILE_ID_EDRVNAME_C,
/* 2093  */      DRV_FILE_ID_ENC28J60_C,
/* 2094  */      DRV_FILE_ID_ENC28J60END_C,
/* 2095  */      DRV_FILE_ID_FDIDRV_C,
/* 2096  */      DRV_FILE_ID_FDI_ACCTEST_C,
/* 2097  */      DRV_FILE_ID_FDI_FILE_TEST_C,
/* 2098  */      DRV_FILE_ID_FDI_INIT_MOUNT_C,
/* 2099  */      DRV_FILE_ID_FFS_HASHTBL_C,
/* 2100  */      DRV_FILE_ID_FFS_MALLOC_C,
/* 2101  */      DRV_FILE_ID_FI_RTOS_C,
/* 2102  */      DRV_FILE_ID_FLASHDAVAPI_C,
/* 2103  */      DRV_FILE_ID_GPIO_C,
/* 2104  */      DRV_FILE_ID_GPIOINT_C,
/* 2105  */      DRV_FILE_ID_HI6411SPI_C,
/* 2106  */      DRV_FILE_ID_HI_SD_MMC_BUS_C,
/* 2107  */      DRV_FILE_ID_HI_SD_MMC_HOST_C,
/* 2108  */      DRV_FILE_ID_HKADC_C,
/* 2109  */      DRV_FILE_ID_I28F256L18FLASH_C,
/* 2110  */      DRV_FILE_ID_I28F256M18FLASH_C,
/* 2111  */      DRV_FILE_ID_I28F256MTD_C,
/* 2112  */      DRV_FILE_ID_I2C_CORE_C,
/* 2113  */      DRV_FILE_ID_IFM_API_C,
/* 2114  */      DRV_FILE_ID_IFM_CMN_C,
/* 2115  */      DRV_FILE_ID_IFM_MFM_C,
/* 2116  */      DRV_FILE_ID_IFM_SYS_C,
/* 2117  */      DRV_FILE_ID_IFM_VM_C,
/* 2118  */      DRV_FILE_ID_INT_C,
/* 2119  */      DRV_FILE_ID_INTTST_C,
/* 2120  */      DRV_FILE_ID_IOLIB_C,
/* 2121  */      DRV_FILE_ID_IOSLIB_C,
/* 2122  */      DRV_FILE_ID_KEYPAD_C,
/* 2123  */      DRV_FILE_ID_KPDGPIO_C,
/* 2124  */      DRV_FILE_ID_LOGGER_C,
/* 2125  */      DRV_FILE_ID_MAIN_C,
/* 2126  */      DRV_FILE_ID_MFM_FM_API_C,
/* 2127  */      DRV_FILE_ID_MFM_INT_DLT_C,
/* 2128  */      DRV_FILE_ID_MFM_INT_FLT_C,
/* 2129  */      DRV_FILE_ID_MFM_INT_FORMAT_C,
/* 2130  */      DRV_FILE_ID_MFM_INT_OST_C,
/* 2131  */      DRV_FILE_ID_MFM_INT_PBT_C,
/* 2132  */      DRV_FILE_ID_MFM_INT_RBAPI_C,
/* 2133  */      DRV_FILE_ID_MFM_INT_RBFSH_C,
/* 2134  */      DRV_FILE_ID_MFM_INT_RBINT_C,
/* 2135  */      DRV_FILE_ID_MFM_INT_RECLAIM_C,
/* 2136  */      DRV_FILE_ID_MFM_INT_SCAN_C,
/* 2137  */      DRV_FILE_ID_MFM_INT_SYSTEM_C,
/* 2138  */      DRV_FILE_ID_MFM_LOW_FLASH_C,
/* 2139  */      DRV_FILE_ID_MFM_MUSB_API_C,
/* 2140  */      DRV_FILE_ID_MFM_MUSB_BOOT_C,
/* 2141  */      DRV_FILE_ID_MFM_MUSB_CACHE_C,
/* 2142  */      DRV_FILE_ID_MFM_MUSB_COMMON_C,
/* 2143  */      DRV_FILE_ID_MFM_MUSB_DCF_C,
/* 2144  */      DRV_FILE_ID_MFM_MUSB_DECF_C,
/* 2145  */      DRV_FILE_ID_MFM_MUSB_ENTRY_C,
/* 2146  */      DRV_FILE_ID_MFM_MUSB_EXT_C,
/* 2147  */      DRV_FILE_ID_MFM_MUSB_FILE_C,
/* 2148  */      DRV_FILE_ID_MFM_MUSB_FS_WRAP_C,
/* 2149  */      DRV_FILE_ID_MFM_MUSB_INFO_C,
/* 2150  */      DRV_FILE_ID_MFM_MUSB_LIST_C,
/* 2151  */      DRV_FILE_ID_MFM_MUSB_PLR_C,
/* 2152  */      DRV_FILE_ID_MFM_MUSB_REGROUP_C,
/* 2153  */      DRV_FILE_ID_MFM_MUSB_SCANDIR_C,
/* 2154  */      DRV_FILE_ID_MFM_MUSB_UCF_C,
/* 2155  */      DRV_FILE_ID_MFM_MUSB_VFAT_C,
/* 2156  */      DRV_FILE_ID_MNTNDRV_C,
/* 2157  */      DRV_FILE_ID_MTD_CARR_C,
/* 2158  */      DRV_FILE_ID_MTD_CFI_C,
/* 2159  */      DRV_FILE_ID_MTD_CFIR_C,
/* 2160  */      DRV_FILE_ID_MTD_GEN_C,
/* 2161  */      DRV_FILE_ID_MTD_GENR_C,
/* 2162  */      DRV_FILE_ID_MTD_INIT_C,
/* 2163  */      DRV_FILE_ID_MTD_LIB_C,
/* 2164  */      DRV_FILE_ID_MTD_PARR_C,
/* 2165  */      DRV_FILE_ID_MTD_SIB_C,
/* 2166  */      DRV_FILE_ID_MTD_SIBR_C,
/* 2167  */      DRV_FILE_ID_MTD_STARR_C,
/* 2168  */      DRV_FILE_ID_MUSB_DEMO_C,
/* 2169  */      DRV_FILE_ID_OS_C,
/* 2170  */      DRV_FILE_ID_OSSLIBDL_C,
/* 2171  */      DRV_FILE_ID_OSSLIB_C,
/* 2172  */      DRV_FILE_ID_OS_API_C,
/* 2173  */      DRV_FILE_ID_PL131_C,
/* 2174  */      DRV_FILE_ID_PMHI6402_C,
/* 2175  */      DRV_FILE_ID_PMUDMINTF_C,
/* 2176  */      DRV_FILE_ID_POS_API_C,
/* 2177  */      DRV_FILE_ID_PWRINTERFACE_C,
/* 2178  */      DRV_FILE_ID_RTC_C,
/* 2179  */      DRV_FILE_ID_RTCBOTH_C,
/* 2180  */      DRV_FILE_ID_RTCHI6402_C,
/* 2181  */      DRV_FILE_ID_SCI_C,
/* 2182  */      DRV_FILE_ID_SCIINTERFACE_C,
/* 2183  */      DRV_FILE_ID_SPIDRV_C,
/* 2184  */      DRV_FILE_ID_STANDLIB_C,
/* 2185  */      DRV_FILE_ID_SYSSERIAL_C,
/* 2186  */      DRV_FILE_ID_SYSTFFS_C,
/* 2187  */      DRV_FILE_ID_TEST_C,
/* 2188  */      DRV_FILE_ID_TEST_BOOT_DIS_C,
/* 2189  */      DRV_FILE_ID_TFFSCONFIG_C,
/* 2190  */      DRV_FILE_ID_TIMER_C,
/* 2191  */      DRV_FILE_ID_TIMERTST_C,
/* 2192  */      DRV_FILE_ID_TT_ANALYSECALLSTACK_C,
/* 2193  */      DRV_FILE_ID_USB4DOWNLOAD_C,
/* 2194  */      DRV_FILE_ID_USBCDROM_C,
/* 2195  */      DRV_FILE_ID_USBDESCRCOPYLIBDL_C,
/* 2196  */      DRV_FILE_ID_USBDESCRCOPYLIB_C,
/* 2197  */      DRV_FILE_ID_USBDISK_C,
/* 2198  */      DRV_FILE_ID_USBHANDLELIBDL_C,
/* 2199  */      DRV_FILE_ID_USBHANDLELIB_C,
/* 2200  */      DRV_FILE_ID_USBLISTLIBDL_C,
/* 2201  */      DRV_FILE_ID_USBLISTLIB_C,
/* 2202  */      DRV_FILE_ID_USBMODEMBULK_C,
/* 2203  */      DRV_FILE_ID_USBMODEMINT_C,
/* 2204  */      DRV_FILE_ID_USBMUX_C,
/* 2205  */      DRV_FILE_ID_USBSERIAL_C,
/* 2206  */      DRV_FILE_ID_USBTARGLIBDL_C,
/* 2207  */      DRV_FILE_ID_USBTARGLIB_C,
/* 2208  */      DRV_FILE_ID_USBTCDLIBDL_C,
/* 2209  */      DRV_FILE_ID_USBTCDLIB_C,
/* 2210  */      DRV_FILE_ID_USIMDMINTF_C,
/* 2211  */      DRV_FILE_ID_USIMSYSTEST_C,
/* 2212  */      DRV_FILE_ID_USRAPPINIT_C,
/* 2213  */      DRV_FILE_ID_USRCONFIG_C,
/* 2214  */      DRV_FILE_ID_USRENTRY_C,
/* 2215  */      DRV_FILE_ID_USRFSLIB_C,
/* 2216  */      DRV_FILE_ID_UTILITY_FS_C,
/* 2217  */      DRV_FILE_ID_VMBASELIB_C,
/* 2218  */      DRV_FILE_ID_WDTDRV_C,
/* 2219  */      DRV_FILE_ID_WDTTEST_C,

                DRV_FILE_ID_BUTT
}DRV_FILE_ID_DEFINE_ENUM;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif


