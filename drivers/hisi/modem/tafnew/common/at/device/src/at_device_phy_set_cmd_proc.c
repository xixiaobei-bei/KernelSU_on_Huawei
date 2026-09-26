/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include "at_device_phy_set_cmd_proc.h"
#include "securec.h"
#include "taf_phy_pid_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_app_mma.h"
#include "at_device_comm.h"
#include "at_lte_common.h"
#include "at_ctx.h"
#include "at_mdrv_interface.h"
#include "at_mt_msg_proc.h"
#include "at_input_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_PHY_SET_CMD_PROC_C

#define AT_LTCOMMCMD_MIN_PARA_NUM 5
#define AT_LTCOMMCMD_DEST 0
#define AT_LTCOMMCMD_BUFF_IN_LEN 1
#define AT_LTCOMMCMD_CRC_IN_LOWORD 2
#define AT_LTCOMMCMD_CRC_IN_HIWORD 3
#define AT_LTCOMMCMD_BUFFER_IN 4
#define AT_LTCOMMCMD_LEN_OUT 5
#define AT_LTCOMMCMD_CRC_OUT_LOWORD 6
#define AT_LTCOMMCMD_CRC_OUT_HIWORD 7
#define AT_HEC2_TO_VAL_MAX_NUM 2
#define AT_FWAVE_MAX_PARA_NUM 2
#define AT_FWAVE_CMD_ARRAY_LEN 200
#define AT_FWAVE_PARA_NUM 2
#define AT_FWAVE_WAVEFORM_TYPE 0
#define AT_FWAVE_WAVEFORM_POWER 1
#define AT_FWAVE_GSM_TX_SLOT_TYPE 2
#define AT_FWAVE_WAVEFORM_TYPE_SUPPORT_NUM 7
#define AT_FWAVE_WAVEFORM_CONTINUES_WAVE 8 /* 发射波形类型8为单音 */
#define AT_FWAVE_WAVEFORM_BPSK 0 /* 发射的波形类型为BPSK波形 */
#define AT_FCHAN_LISTENING_PATH_FLG 3
#define AT_FCHAN_MAX_PARA_NUM 3
#define AT_FCHAN_CHANNEL_NO 2
#define AT_FCHAN_PARA_MODE 0
#define AT_FCHAN_PARA_BAND 1
#define AT_FCHAN_PARA_CHANNEL_NO 2
#define AT_FCHAN_PARA_UL_CHANNEL 3
#define AT_FCHAN_PARA_DL_CHANNEL 4
#define AT_FCHAN_NR_PARA_NUM 4
#define AT_FCHAN_PARA_NUM 3
#define AT_TSELRF_PARA_MIN_NUM 1
#define AT_TSELRF_PARA_MAX_NUM 2
#define AT_TSELRF_PATH_ID 0
#define AT_TSELRF_GROUP 1
#define AT_TSELRF_CMD_MAX_LEN 200
#define AT_TSELRF_TX_OR_RX_PATH 0
#define AT_TSELRF_TX_OR_RX 1
#define AT_TSELRF_PARA_TYPE 2
#define AT_TSELRF_PARA_ANT_NUM 3
#define AT_TSELRF_PARA_NUM 2
#define AT_MIPIRD_PARA_NUM 4
#define AT_MIPIRD_SECONDARY_ADDR 2
#define AT_MIPIRD_REG_PARA 3
#define AT_DPDT_TO_BBIC_PARA_NUM 3
#define AT_DPDT_RAT_MODE 0
#define AT_DPDT_VALUE 1
#define AT_DPDT_WORK_TYPE 2
#define AT_DPDT_QRY_RAT_MODE 0
#define AT_DPDT_QRY_WORK_TYPE 1
#define AT_DPDT_PARA_NUM 2
#define AT_DPDTTEST_PARA_NUM 2
#define AT_DPDTQRY_PARA_NUM 2
#define AT_RADVER_PARA_NUM 2
#define AT_RADVER_MOD 0
#define AT_RADVER_VER 1
#define AT_FSERDESRT_TEST_NUM_INDEX 0
#define AT_FSERDESRT_SYNC_LEN_INDEX 1
#define AT_FSERDESRT_BURST_TIME_INDEX 2
#define AT_FSERDESRT_STALL_TIME_INDEX 3
#define AT_FSERDESRT_DL_SYNC_LEN_INDEX 4
#define AT_FSERDESRT_DL_BURST_TIME_INDEX 5
#define AT_FSERDESRT_DL_STALL_TIME_INDEX 6
#define AT_FSERDESRT_LS_TEST_INDEX 7
#define AT_FSERDESRT_FWD_CLK_AMP_INDEX 8
#define AT_PHY_SERDES_TEST_PARALIST_PARA1 2
#define AT_PHY_SERDES_TEST_PARALIST_PARA2 3
#define AT_PHY_SERDES_TEST_PARALIST_PARA3 4
#define AT_PHY_SERDES_TEST_PARALIST_PARA4 5
#define AT_PHY_SERDES_TEST_PARALIST_PARA5 6
#define AT_PHY_SERDES_TEST_PARALIST_PARA6 7
#define AT_PHY_SERDES_TEST_PARALIST_PARA7 8
#define AT_PHY_SERDES_TEST_PARALIST_PARA8 9
#define AT_PHY_SERDES_TEST_PARALIST_PARA9 10
#define AT_PHY_SERDES_TEST_PARALIST_PARA10 11
#define AT_PHY_SERDES_TEST_PARALIST_PARA11 12
#define AT_PHY_SERDES_TEST_PARALIST_PARA12 13
#define AT_PHY_SERDES_TEST_PARALIST_PARA13 14
#define AT_PHY_SERDES_TEST_PARALIST_PARA14 15
#define AT_PHY_SERDES_TEST_PARALIST_PARA15 16
#define AT_PHY_SERDES_TEST_PARALIST_PARA16 17
#define AT_PHY_SERDES_TEST_PARALIST_PARA17 18
#define AT_PHY_SERDES_TEST_PARALIST_PARA18 19
#define AT_PHY_SERDES_TEST_PARALIST_PARA19 20
#define AT_PHY_SERDES_TEST_PARALIST_PARA20 21
#define AT_SERDESTESTASYNC_PARA_MIN_NUM   2
#define AT_SERDESTESTASYNC_PARA_MAX_NUM   22
#define AT_SERDESTESTASYNC_PARA_MODE    0 /* SERDESTESTASYNC的第一个参数MODE */
#define AT_SERDESTESTASYNC_PARA_CMDTYPE 1 /* SERDESTESTASYNC的第二个参数CMDTYPE */
#define AT_SERDESTESTASYNC_PARA_RPTFLG  2 /* SERDESTESTASYNC的第三个参数RPTFLG */
#define AT_FSERDESRT_PARA_NUM 7
#define AT_DCXOTEMPCOMP_PARA_NUM 1
#define AT_MIPIOPERATE_PARA_NUM 5
#define AT_MIPIOPERATE_MIPI_PORT_SEL 1
#define AT_MIPIOPERATE_SECONDARY_ID 2
#define AT_MIPIOPERATE_REG_ADDR 3
#define AT_MIPIOPERATE_BYTE_CNT 4
#define AT_MIPIOPERATE_DATA_BITBYTE0_VALUE 5
#define AT_FRBINFO_PARA_NUM        2
#define AT_FRBINFO_PARA_RBNUM      0 /* FRBINFO的第一个参数RBNUM */
#define AT_FRBINFO_PARA_RBSTARTCFG 1 /* FRBINFO的第二个参数RBSTARTCFG */
#define AT_MIPIWR_PARA_NUM 5
#define AT_SSIWR_PARA_NUM 4
#define AT_MIPIREAD_PARA_NUM 7
#define AT_PHYMIPIWRITE_PARA_NUM 6
#define AT_CMD_MIPIWR_SECONDARY_ADDR 1
#define AT_CMD_MIPIWR_REG_ADDR 2
#define AT_CMD_MIPIWR_REG_DATA 3
#define AT_CMD_MIPIWR_MIPI_CHANNEL 4
#define AT_SSIWR_PARA_REG_ADDR 2
#define AT_SSIWR_PARA_DATA 3
#define AT_MIPI_READ_PARA_READ_TYPE 0
#define AT_MIPI_READ_PARA_MIPI_ID 1
#define AT_MIPI_READ_PARA_SECONDARY_ID 2
#define AT_MIPI_READ_PARA_REG_ADDR 3
#define AT_MIPI_READ_PARA_SPEED_TYPE 4
#define AT_MIPI_READ_PARA_READ_BIT_MASK 5
#define AT_MIPI_READ_PARA_REAERVED 6
#define AT_PHYMIPIWRITE_WR_TYPE 0
#define AT_PHYMIPIWRITE_MIPI_ID 1
#define AT_PHYMIPIWRITE_SECONDARY_ID 2
#define AT_PHYMIPIWRITE_REG_ADDR 3
#define AT_PHYMIPIWRITE_MIPI_DATA 4
#define AT_PHYMIPIWRITE_RESERVAED 5
#define AT_FPLL_STATUS_TX 0

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
static const AT_BAND_WidthInfo g_bandWidthTable[] = {
    { AT_BAND_WIDTH_200K, BANDWIDTH_200K, AT_BAND_WIDTH_VALUE_200K },
    { AT_BAND_WIDTH_1M2288, BANDWIDTH_1M2288, AT_BAND_WIDTH_VALUE_1M2288 },
    { AT_BAND_WIDTH_1M28, BANDWIDTH_1M28, AT_BAND_WIDTH_VALUE_1M28 },
    { AT_BAND_WIDTH_1M4, BANDWIDTH_1M4, AT_BAND_WIDTH_VALUE_1M4 },
    { AT_BAND_WIDTH_3M, BANDWIDTH_3M, AT_BAND_WIDTH_VALUE_3M },
    { AT_BAND_WIDTH_5M, BANDWIDTH_5M, AT_BAND_WIDTH_VALUE_5M },
    { AT_BAND_WIDTH_10M, BANDWIDTH_10M, AT_BAND_WIDTH_VALUE_10M },
    { AT_BAND_WIDTH_15M, BANDWIDTH_15M, AT_BAND_WIDTH_VALUE_15M },
    { AT_BAND_WIDTH_20M, BANDWIDTH_20M, AT_BAND_WIDTH_VALUE_20M },
    { AT_BAND_WIDTH_25M, BANDWIDTH_25M, AT_BAND_WIDTH_VALUE_25M },
    { AT_BAND_WIDTH_30M, BANDWIDTH_30M, AT_BAND_WIDTH_VALUE_30M },
    { AT_BAND_WIDTH_40M, BANDWIDTH_40M, AT_BAND_WIDTH_VALUE_40M },
    { AT_BAND_WIDTH_50M, BANDWIDTH_50M, AT_BAND_WIDTH_VALUE_50M },
    { AT_BAND_WIDTH_60M, BANDWIDTH_60M, AT_BAND_WIDTH_VALUE_60M },
    { AT_BAND_WIDTH_80M, BANDWIDTH_80M, AT_BAND_WIDTH_VALUE_80M },
    { AT_BAND_WIDTH_90M, BANDWIDTH_90M, AT_BAND_WIDTH_VALUE_90M },
    { AT_BAND_WIDTH_100M, BANDWIDTH_100M, AT_BAND_WIDTH_VALUE_100M },
    { AT_BAND_WIDTH_200M, BANDWIDTH_200M, AT_BAND_WIDTH_VALUE_200M },
    { AT_BAND_WIDTH_400M, BANDWIDTH_400M, AT_BAND_WIDTH_VALUE_400M },
    { AT_BAND_WIDTH_800M, BANDWIDTH_800M, AT_BAND_WIDTH_VALUE_800M },
    { AT_BAND_WIDTH_1G, BANDWIDTH_1G, AT_BAND_WIDTH_VALUE_1G },
};

/*
 * 频率的计算方法:FREF = FREF-Offs + ΔFGlobal (NREF – NREF-Offs)
 * Frequency     ΔFGlobal     FREF-Offs    NREF-Offs     Range of NREF
 * range [MHz]     [kHz]        [MHz]
 * 0 – 3000        5            0            0            0 – 599999
 * 3000 – 24250   15            3000        600000        600000 – 2016666
 * 24250 – 100000 60           24250.08      2016667      2016667 – 3279165
 */

static const AT_NR_FreqOffsetTable g_atNrFreqOffsetTable[] = {
    {{ 0, 3000 * FREQ_UNIT_MHZ_TO_KHZ }, 5, 0, 0, { 0, 599999 }},
    {{ 3000 * FREQ_UNIT_MHZ_TO_KHZ, 24250 * FREQ_UNIT_MHZ_TO_KHZ }, 15, 3000 * FREQ_UNIT_MHZ_TO_KHZ, 600000, { 600000, 2016666 }},
    {{ 24250 * FREQ_UNIT_MHZ_TO_KHZ, 100000 * FREQ_UNIT_MHZ_TO_KHZ }, 60, 24250080, 2016667, { 2016667, 3279165 }},
};

static const AT_NR_BandInfo g_atNrBandInfoTable[] = {
    { 1, AT_DUPLEX_MODE_FDD, { 384000, 396000 }, { 422000, 434000 }, { 1920000, 1980000 }, { 2110000, 2170000 }},
    { 2, AT_DUPLEX_MODE_FDD, { 370000, 382000 }, { 386000, 398000 }, { 1850000, 1910000 }, { 1930000, 1990000 }},
    { 3, AT_DUPLEX_MODE_FDD, { 342000, 357000 }, { 361000, 376000 }, { 1710000, 1785000 }, { 1805000, 1880000 }},
    { 5, AT_DUPLEX_MODE_FDD, { 164800, 169800 }, { 173800, 178800 }, { 824000, 849000 }, { 869000, 894000 }},
    { 7, AT_DUPLEX_MODE_FDD, { 500000, 514000 }, { 524000, 538000 }, { 2500000, 2570000 }, { 2620000, 2690000 }},
    { 8, AT_DUPLEX_MODE_FDD, { 176000, 183000 }, { 185000, 192000 }, { 880000, 915000 }, { 925000, 960000 }},
    { 12, AT_DUPLEX_MODE_FDD, { 139800, 143200 }, { 145800, 149200 }, { 699000, 716000 }, { 729000, 746000 }},
    { 20, AT_DUPLEX_MODE_FDD, { 166400, 172400 }, { 158200, 164200 }, { 832000, 862000 }, { 791000, 821000 }},
    { 25, AT_DUPLEX_MODE_FDD, { 370000, 383000 }, { 386000, 399000 }, { 1850000, 1915000 }, { 1930000, 1995000 }},
    { 28, AT_DUPLEX_MODE_FDD, { 140600, 149600 }, { 151600, 160600 }, { 703000, 748000 }, { 758000, 803000 }},
    { 34, AT_DUPLEX_MODE_TDD, { 402000, 405000 }, { 402000, 405000 }, { 2010000, 2025000 }, { 2010000, 2025000 }},
    { 38, AT_DUPLEX_MODE_TDD, { 514000, 524000 }, { 514000, 524000 }, { 2570000, 2620000 }, { 2570000, 2620000 }},
    { 39, AT_DUPLEX_MODE_TDD, { 376000, 384000 }, { 376000, 384000 }, { 1880000, 1920000 }, { 1880000, 1920000 }},
    { 40, AT_DUPLEX_MODE_TDD, { 460000, 480000 }, { 460000, 480000 }, { 2300000, 2400000 }, { 2300000, 2400000 }},
    { 41, AT_DUPLEX_MODE_TDD, { 499200, 537999 }, { 499200, 537999 }, { 2496000, 2690000 }, { 2496000, 2690000 }},
    { 51, AT_DUPLEX_MODE_TDD, { 285400, 286400 }, { 285400, 286400 }, { 1427000, 1432000 }, { 1427000, 1432000 }},
    /* 上下行频点不对称 */
    { 66, AT_DUPLEX_MODE_FDD, { 342000, 356000 }, { 422000, 440000 }, { 1710000, 1780000 }, { 2110000, 2200000 }},
    /* 上下行频点不对称 */
    { 70, AT_DUPLEX_MODE_FDD, { 339000, 342000 }, { 399000, 404000 }, { 1695000, 1710000 }, { 1995000, 2020000 }},
    { 71, AT_DUPLEX_MODE_FDD, { 132600, 139600 }, { 123400, 130400 }, { 663000, 698000 }, { 617000, 652000 }},
    /* 上下行频点不对称 */
    { 75, AT_DUPLEX_MODE_SDL, { 0, 0 }, { 286400, 303400 }, { 0, 0 }, { 1432000, 1517000 }},
    /* 上下行频点不对称 */
    { 76, AT_DUPLEX_MODE_SDL, { 0, 0 }, { 285400, 286400 }, { 0, 0 }, { 1427000, 1432000 }},
    { 77, AT_DUPLEX_MODE_TDD, { 620000, 680000 }, { 620000, 680000 }, { 3300000, 4200000 }, { 3300000, 4200000 }},
    { 78, AT_DUPLEX_MODE_TDD, { 620000, 653333 }, { 620000, 653333 }, { 3300000, 3800000 }, { 3300000, 3800000 }},
    { 79, AT_DUPLEX_MODE_TDD, { 693334, 733333 }, { 693334, 733333 }, { 4400000, 5000000 }, { 4400000, 5000000 }},
    { 80, AT_DUPLEX_MODE_SUL, { 342000, 357000 }, { 0, 0 }, { 1710000, 1785000 }, { 0, 0 }},
    { 81, AT_DUPLEX_MODE_SUL, { 176000, 183000 }, { 0, 0 }, { 880000, 915000 }, { 0, 0 }},
    { 82, AT_DUPLEX_MODE_SUL, { 166400, 172400 }, { 0, 0 }, { 832000, 862000 }, { 0, 0 }},
    { 83, AT_DUPLEX_MODE_SUL, { 140600, 149600 }, { 0, 0 }, { 703000, 748000 }, { 0, 0 }},
    { 84, AT_DUPLEX_MODE_SUL, { 384000, 396000 }, { 0, 0 }, { 1920000, 1980000 }, { 0, 0 }},
    { 86, AT_DUPLEX_MODE_SUL, { 342000, 356000 }, { 0, 0 }, { 1710000, 1780000 }, { 0, 0 }},
    { 257, AT_DUPLEX_MODE_TDD, { 2054166, 2104165 }, { 2054166, 2104165 }, { 26500000, 29500000 }, { 26500000, 29500000 }},
    { 258, AT_DUPLEX_MODE_TDD, { 2016667, 2070832 }, { 2016667, 2070832 }, { 24250000, 27500000 }, { 24250000, 27500000 }},
    { 260, AT_DUPLEX_MODE_TDD, { 2229166, 2279165 }, { 2229166, 2279165 }, { 37000000, 40000000 }, { 37000000, 40000000 }},
    { 261, AT_DUPLEX_MODE_TDD, { 2070833, 2087497 }, { 2070833, 2087497 }, { 27500000, 28500000 }, { 27500000, 28350000 }},
};

static const AT_LTE_BandInfo g_atLteBandInfoTable[] = {
    { 1, 0, 2110000, 0, { 0, 599 }, 1920000, 18000, { 18000, 18599 }},
    { 2, 0, 1930000, 600, { 600, 1199 }, 1850000, 18600, { 18600, 19199 }},
    { 3, 0, 1805000, 1200, { 1200, 1949 }, 1710000, 19200, { 19200, 19949 }},
    { 4, 0, 2110000, 1950, { 1950, 2399 }, 1710000, 19950, { 19950, 20399 }},
    { 5, 0, 869000, 2400, { 2400, 2649 }, 824000, 20400, { 20400, 20649 }},
    { 6, 0, 875000, 2650, { 2650, 2749 }, 830000, 20650, { 20650, 20749 }},
    { 7, 0, 2620000, 2750, { 2750, 3449 }, 2500000, 20750, { 20750, 21449 }},
    { 8, 0, 925000, 3450, { 3450, 3799 }, 880000, 21450, { 21450, 21799 }},
    { 9, 0, 1844900, 3800, { 3800, 4149 }, 1749900, 21800, { 21800, 22149 }},
    { 10, 0, 2110000, 4150, { 4150, 4749 }, 1710000, 22150, { 22150, 22749 }},
    { 11, 0, 1475900, 4750, { 4750, 4949 }, 1427900, 22750, { 22750, 22949 }},
    { 12, 0, 729000, 5010, { 5010, 5179 }, 699000, 23010, { 23010, 23179 }},
    { 13, 0, 746000, 5180, { 5180, 5279 }, 777000, 23180, { 23180, 23279 }},
    { 14, 0, 758000, 5280, { 5280, 5379 }, 788000, 23280, { 23280, 23379 }},
    { 17, 0, 734000, 5730, { 5730, 5849 }, 704000, 23730, { 23730, 23849 }},
    { 18, 0, 860000, 5850, { 5850, 5999 }, 815000, 23850, { 23850, 23999 }},
    { 19, 0, 875000, 6000, { 6000, 6149 }, 830000, 24000, { 24000, 24149 }},
    { 20, 0, 791000, 6150, { 6150, 6449 }, 832000, 24150, { 24150, 24449 }},
    { 21, 0, 1495900, 6450, { 6450, 6599 }, 1447900, 24450, { 24450, 24599 }},
    { 22, 0, 3510000, 6600, { 6600, 7399 }, 3410000, 24600, { 24600, 25399 }},
    { 23, 0, 2180000, 7500, { 7500, 7699 }, 2000000, 25500, { 25500, 25699 }},
    { 24, 0, 1525000, 7700, { 7700, 8039 }, 1626500, 25700, { 25700, 26039 }},
    { 25, 0, 1930000, 8040, { 8040, 8689 }, 1850000, 26040, { 26040, 26689 }},
    { 26, 0, 859000, 8690, { 8690, 9039 }, 814000, 26690, { 26690, 27039 }},
    { 27, 0, 852000, 9040, { 9040, 9209 }, 807000, 27040, { 27040, 27209 }},
    { 28, 0, 758000, 9210, { 9210, 9659 }, 703000, 27210, { 27210, 27659 }},
    { 29, 0, 717000, 9660, { 9660, 9769 }, 0, 0, { 0, 0 }},
    { 30, 0, 2350000, 9770, { 9770, 9869 }, 2305000, 27660, { 27660, 27759 }},
    { 31, 0, 462500, 9870, { 9870, 9919 }, 452500, 27760, { 27760, 27809 }},
    { 32, 0, 1452000, 9920, { 9920, 10359 }, 0, 0, { 0, 0, }},
    { 33, 0, 1900000, 36000, { 36000, 36199 }, 1900000, 36000, { 36000, 36199 }},
    { 34, 0, 2010000, 36200, { 36200, 36349 }, 2010000, 36200, { 36200, 36349 }},
    { 35, 0, 1850000, 36350, { 36350, 36949 }, 1850000, 36350, { 36350, 36949 }},
    { 36, 0, 1930000, 36950, { 36950, 37549 }, 1930000, 36950, { 36950, 37549 }},
    { 37, 0, 1910000, 37550, { 37550, 37749 }, 1910000, 37550, { 37550, 37749 }},
    { 38, 0, 2570000, 37750, { 37750, 38249 }, 2570000, 37750, { 37750, 38249 }},
    { 39, 0, 1880000, 38250, { 38250, 38649 }, 1880000, 38250, { 38250, 38649 }},
    { 40, 0, 2300000, 38650, { 38650, 39649 }, 2300000, 38650, { 38650, 39649 }},
    { 41, 0, 2496000, 39650, { 39650, 41589 }, 2496000, 39650, { 39650, 41589 }},
    { 42, 0, 3400000, 41590, { 41590, 43589 }, 3400000, 41590, { 41590, 43589 }},
    { 43, 0, 3600000, 43590, { 43590, 45589 }, 3600000, 43590, { 43590, 45589 }},
    { 44, 0, 703000, 45590, { 45590, 46589 }, 703000, 45590, { 45590, 46589 }},
    { 45, 0, 1447000, 46590, { 46590, 46789 }, 1447000, 46590, { 46590, 46789 }},
    { 46, 0, 5150000, 46790, { 46790, 54539 }, 5150000, 46790, { 46790, 54539 }},
    { 47, 0, 5855000, 54540, { 54540, 55239 }, 5855000, 54540, { 54540, 55239 }},
    { 48, 0, 3550000, 55240, { 55240, 56739 }, 3550000, 55240, { 55240, 56739 }},
    { 49, 0, 3550000, 56740, { 56740, 58239 }, 3550000, 56740, { 56740, 58239 }},
    { 50, 0, 1432000, 58240, { 58240, 59089 }, 1432000, 58240, { 58240, 59089 }},
    { 51, 0, 1427000, 59090, { 59090, 59139 }, 1427000, 59090, { 59090, 59139 }},
    { 52, 0, 3300000, 59140, { 59140, 60139 }, 3300000, 59140, { 59140, 60139 }},
    { 65, 0, 2110000, 65536, { 65536, 66435 }, 1920000, 131072, { 131072, 131971 }},
    /* 这个band的上下行信道是不对称的 */
    { 66, 0, 2110000, 66436, { 66436, 67335 }, 1710000, 131972, { 131972, 132671 }},
    { 67, 0, 738000, 67336, { 67336, 67535 }, 0, 0, { 0, 0 }},
    { 68, 0, 753000, 67536, { 67536, 67835 }, 698000, 132672, { 132672, 132971 }},
    { 69, 0, 2570000, 67836, { 67836, 68335 }, 0, 0, { 0, 0 }},
    /* 这个band的上下行信道是不对称的 */
    { 70, 0, 1995000, 68336, { 68336, 68585 }, 1695000, 132972, { 132972, 133121 }},
    { 71, 0, 617000, 68586, { 68586, 68935 }, 663000, 133122, { 133122, 133471 }},
    { 72, 0, 461000, 68936, { 68936, 68985 }, 451000, 133472, { 133472, 133521 }},
    { 73, 0, 460000, 68986, { 68986, 69035 }, 450000, 133522, { 133522, 133571 }},
    { 74, 0, 1475000, 69036, { 69036, 69465 }, 1427000, 133572, { 133572, 134001 }},
    { 75, 0, 1432000, 69466, { 69466, 70315 }, 0, 0, { 0, 0 }},
    { 76, 0, 1427000, 70316, { 70316, 70365 }, 0, 0, { 0, 0 }},
    { 85, 0, 728000, 70366, { 70366, 70545 }, 698000, 134002, { 134002, 134181 }},
    { 128, 0, 780500, 9435, { 9435, 9659 }, 725500, 27435, { 27435, 27659 }},
    { 140, 0, 2300000, 38650, { 38650, 39250 }, 2300000, 38650, { 38650, 39250 }},
};

static const AT_W_BandInfo g_atWBandInfoTable[] = {
    { 1, 0, 0, { 9612, 9888 }, { 1922400, 1977600 }, 0, { 10562, 10838 }, { 2112400, 2167600 }},
    { 2, 0, 0, { 9262, 9538 }, { 1852400, 1907600 }, 0, { 9662, 9938 }, { 1932400, 1987600 }},
    { 3, 0, 1525000, { 937, 1288 }, { 1712400, 1782600 }, 1575000, { 1162, 1513 }, { 1807400, 1877600 }},
    { 4, 0, 1450000, { 1312, 1513 }, { 1712400, 1752600 }, 1805000, { 1537, 1738 }, { 2112400, 2152600 }},
    { 5, 0, 0, { 4132, 4233 }, { 826400, 846600 }, 0, { 4357, 4458 }, { 871400, 891600 }},
    { 6, 0, 0, { 4162, 4188 }, { 832400, 837600 }, 0, { 4387, 4413 }, { 877400, 882600 }},
    { 7, 0, 2100000, { 2012, 2338 }, { 2502400, 2567600 }, 2175000, { 2237, 2563 }, { 2622400, 2687600 }},
    { 8, 0, 340000, { 2712, 2863 }, { 882400, 912600 }, 340000, { 2937, 3088 }, { 927400, 957600 }},
    { 9, 0, 0, { 8762, 8912 }, { 1752400, 1782400 }, 0, { 9237, 9387 }, { 1847400, 1877400 }},
    { 11, 0, 733000, { 3487, 3562 }, { 1430400, 1445400 }, 736000, { 3712, 3787 }, { 1478400, 1493400 }},
    { 19, 0, 770000, { 312, 363 }, { 832400, 842600 }, 735000, { 712, 763 }, { 877400, 887600 }},
};

static const AT_PATH_ToAntType g_path2AntTypeTable[] = {
    { AT_TSELRF_PATH_GSM, AT_ANT_TYPE_PRI },           { AT_TSELRF_PATH_WCDMA_PRI, AT_ANT_TYPE_PRI },
    { AT_TSELRF_PATH_WCDMA_DIV, AT_ANT_TYPE_DIV },     { AT_TSELRF_PATH_CDMA_PRI, AT_ANT_TYPE_PRI },
    { AT_TSELRF_PATH_CDMA_DIV, AT_ANT_TYPE_DIV },      { AT_TSELRF_PATH_FDD_LTE_PRI, AT_ANT_TYPE_PRI },
    { AT_TSELRF_PATH_FDD_LTE_DIV, AT_ANT_TYPE_DIV },   { AT_TSELRF_PATH_FDD_LTE_MIMO, AT_ANT_TYPE_MIMO },
    { AT_TSELRF_PATH_TDD_LTE_PRI, AT_ANT_TYPE_PRI },   { AT_TSELRF_PATH_TDD_LTE_DIV, AT_ANT_TYPE_DIV },
    { AT_TSELRF_PATH_TDD_LTE_MIMO, AT_ANT_TYPE_MIMO }, { AT_TSELRF_PATH_NR_PRI, AT_ANT_TYPE_PRI },
    { AT_TSELRF_PATH_NR_DIV, AT_ANT_TYPE_DIV },        { AT_TSELRF_PATH_NR_MIMO, AT_ANT_TYPE_MIMO },
};
#endif

/* 为了简化不考虑出错 */
VOS_UINT8 Hec2ToVal(VOS_UINT8 *str)
{
    VOS_UINT8 mid = 0;
    VOS_UINT8 val = 0;
    VOS_UINT8 i   = 0;

    for (i = 0; i < AT_HEC2_TO_VAL_MAX_NUM; i++) {
        if ((str[i] >= '0') && (str[i] <= '9')) {
            mid = str[i] - '0';
        } else if ((str[i] >= 'a') && (str[i] <= 'f')) {
            mid = str[i] - 'a' + 10; /* 将字符串转换成十进制数 */
        } else if ((str[i] >= 'A') && (str[i] <= 'F')) {
            mid = str[i] - 'A' + 10; /* 将字符串转换成十进制数 */
        } else {
            mid = 0;
        }

        val = (VOS_UINT8)(val << 4) + mid;
    }

    return val;
}

LOCAL VOS_UINT32 AT_CheckLtCommCmdPara(VOS_UINT32 *strInLen)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return VOS_FALSE;
    }

    if (g_atParaIndex < AT_LTCOMMCMD_MIN_PARA_NUM) {
        return VOS_FALSE;
    }

    if ((g_atParaList[AT_LTCOMMCMD_DEST].paraLen == 0) ||
        (g_atParaList[AT_LTCOMMCMD_BUFF_IN_LEN].paraLen == 0) ||
        (g_atParaList[AT_LTCOMMCMD_CRC_IN_LOWORD].paraLen == 0) ||
        (g_atParaList[AT_LTCOMMCMD_CRC_IN_HIWORD].paraLen == 0) ||
        (g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen == 0)) {
        return VOS_FALSE;
    }

    *strInLen = (VOS_UINT32)(g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen + g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen +
        g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen + g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen);

    if (*strInLen > FTM_MAX_COMM_CMD_LEN * AT_DOUBLE_LENGTH) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

LOCAL VOS_VOID AT_SaveLtCommCmdPara(VOS_UINT8 *pdataAucPara, VOS_UINT32 strInLen)
{
    errno_t memResult;

    memResult = memcpy_s(pdataAucPara, strInLen, g_atParaList[AT_LTCOMMCMD_BUFFER_IN].para,
        g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, strInLen, g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen);

    if (g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen > 0) {
        memResult = memcpy_s(pdataAucPara + g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen,
                             g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen +
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen +
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen,
                             g_atParaList[AT_LTCOMMCMD_LEN_OUT].para, g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen +
                            g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen +
                            g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen,
                            g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen);
    }
    if (g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen > 0) {
        memResult = memcpy_s(pdataAucPara + g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen +
                             g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen,
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen +
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen,
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].para,
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen +
                            g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen,
                            g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen);
    }
    if (g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen > 0) {
        memResult = memcpy_s(pdataAucPara + g_atParaList[AT_LTCOMMCMD_BUFFER_IN].paraLen +
                             g_atParaList[AT_LTCOMMCMD_LEN_OUT].paraLen +
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_LOWORD].paraLen,
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen,
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].para,
                             g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen,
                            g_atParaList[AT_LTCOMMCMD_CRC_OUT_HIWORD].paraLen);
    }
}

LOCAL VOS_UINT32 AT_VerifyLtCommParaCrc(VOS_UINT8 *pdataAucPara, VOS_UINT32 strInLen)
{
    VOS_UINT16           crcL;
    VOS_UINT16           crcH;
    VOS_UINT32           crc;
    VOS_UINT32           crcCal;

    crcL   = (VOS_UINT16)g_atParaList[AT_LTCOMMCMD_CRC_IN_LOWORD].paraValue;
    crcH   = (VOS_UINT16)g_atParaList[AT_LTCOMMCMD_CRC_IN_HIWORD].paraValue;
    crc    = crcH << 16;
    crc    = crc + crcL;
    crcCal = Calc_CRC32(pdataAucPara, strInLen);

    /* 对传入参数进行CRC校验  */
    if (crcCal != crc) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/*
 * 功能描述: 发送通用AT命令到TL C核
 */
VOS_UINT32 AT_SetLTCommCmdPara(VOS_UINT8 clientId)
{
    FTM_SetLtcommcmdReq *setReq = NULL;
    VOS_UINT32           rst;
    VOS_UINT32           i = 0;
    VOS_UINT32           len;
    VOS_UINT32 bufInLen;
    VOS_UINT32 strInLen;

    VOS_UINT8 *pdataAucPara = NULL;

    if (AT_CheckLtCommCmdPara(&strInLen) != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    pdataAucPara = (VOS_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, strInLen);
    if (pdataAucPara == NULL) {
        PS_PRINTF_WARNING("[%s]failed to malloc\n", __FUNCTION__);
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(pdataAucPara, strInLen, 0x00, strInLen);

    AT_SaveLtCommCmdPara(pdataAucPara, strInLen);

    /* 对传入参数进行CRC校验  */
    if (AT_VerifyLtCommParaCrc(pdataAucPara, strInLen) != VOS_TRUE) {
        VOS_MemFree(WUEPS_PID_AT, pdataAucPara);
        PS_PRINTF_WARNING("<%s> CRC ERROR\n", __FUNCTION__);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    bufInLen = (g_atParaList[AT_LTCOMMCMD_BUFF_IN_LEN].paraValue);
    /* 取小保护 */
    if (bufInLen > strInLen) {
        bufInLen = strInLen;
    }

    len = sizeof(FTM_SetLtcommcmdReq) + bufInLen / 2;
    setReq = (FTM_SetLtcommcmdReq *)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, len);

    if (setReq == VOS_NULL) {
        VOS_MemFree(WUEPS_PID_AT, pdataAucPara);
        PS_PRINTF_WARNING("<%s> failed to malloc\n", __FUNCTION__);
        return AT_ERROR;
    }
    memset_s(setReq, len, 0x00, len);

    setReq->cmdDest = (FTM_LTCOMMCMD_DEST)(g_atParaList[AT_LTCOMMCMD_DEST].paraValue);
    setReq->dataLen = bufInLen / 2;
    for (i = 0; i < setReq->dataLen; i++) {
        setReq->data[i] = (VOS_CHAR)Hec2ToVal(pdataAucPara + (VOS_UINT32)(2 * i));
    }

    VOS_MemFree(WUEPS_PID_AT, pdataAucPara);

    rst = atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_FTM_SET_LTCOMMCMD_REQ, clientId, (VOS_UINT8 *)(setReq), len);
    VOS_MemFree(WUEPS_PID_AT, setReq);
    if (rst == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_LTCOMMCMD_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

/* ***************************************************************************** */

/* 功能描述: */

/*  */

/* 参数说明: */

/*   ulIndex [in] ... */

/*                ... */

/*  */

/* 返 回 值: */

/*    TODO: ... */

/* ***************************************************************************** */
VOS_UINT32 atSetFWAVEPara(VOS_UINT8 clientId)
{
    VOS_UINT32      rst;
    FTM_SetFwaveReq fWaveSetReq = {0};

    /* 参数检查 */

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaIndex != AT_FWAVE_MAX_PARA_NUM) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[0].paraLen == 0) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[1].paraLen == 0) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    fWaveSetReq.type  = (VOS_UINT16)g_atParaList[0].paraValue;
    fWaveSetReq.power = (VOS_UINT16)g_atParaList[1].paraValue;

    rst = atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_FTM_SET_FWAVE_REQ, clientId, (VOS_UINT8 *)(&fWaveSetReq),
                           sizeof(fWaveSetReq));

    if (rst == AT_SUCCESS) {
        /* 设置当前操作类型 */

        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_FWAVE_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

/* ***************************************************************************** */

/* 功能描述: 设置非信令的信道 */

/*  */

/* 参数说明: */

/*   ulIndex [in] ... */

/*                ... */

/*  */

/* 返 回 值: */

/*    TODO: ... */

/* ***************************************************************************** */
VOS_UINT32 atSetFCHANPara(VOS_UINT8 clientId)
{
    FTM_SetFchanReq fCHANSetReq = {
        (FCHAN_MODE_ENUM)0,
    };
    VOS_UINT32 rst;
    /* 参数检查 */

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaIndex != AT_FCHAN_MAX_PARA_NUM) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[0].paraLen == 0) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[1].paraLen == 0) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[AT_FCHAN_CHANNEL_NO].paraLen == 0) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[AT_FCHAN_LISTENING_PATH_FLG].paraLen == 0) {
        g_atParaList[AT_FCHAN_LISTENING_PATH_FLG].paraValue = 0;
    }

    fCHANSetReq.fchanMode = (FCHAN_MODE_ENUM)(g_atParaList[0].paraValue);
    fCHANSetReq.band      = (VOS_UINT8)(g_atParaList[1].paraValue);
    fCHANSetReq.channel   = (g_atParaList[AT_FCHAN_CHANNEL_NO].paraValue);

    fCHANSetReq.listeningPathFlg = (VOS_UINT16)(g_atParaList[AT_FCHAN_LISTENING_PATH_FLG].paraValue);
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    (VOS_VOID)AT_SetGlobalFchan((VOS_UINT8)(g_atParaList[0].paraValue));
#endif

    rst = atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_FTM_SET_FCHAN_REQ, clientId, (VOS_UINT8 *)(&fCHANSetReq),
                           sizeof(fCHANSetReq));

    if (rst == AT_SUCCESS) {
        /* 设置当前操作类型 */

        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_FCHAN_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetWifiTselrfPara(VOS_UINT8 path)
{
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    VOS_INT32        bufLen;
    VOS_CHAR         acCmd[AT_TSELRF_CMD_MAX_LEN] = {0};
    VOS_UINT32       rst = ERR_MSP_SUCCESS;


    /*lint -e774*/
    if (path == FTM_TSELRF_WIFI && rst == ERR_MSP_SUCCESS) {
        /*lint +e774*/
        /* WIFI未Enable直接返回失败 */
        if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
            return AT_ERROR;
        }

        if ((g_atParaList[AT_TSELRF_GROUP].paraValue != 0) && (g_atParaList[AT_TSELRF_GROUP].paraValue != 1)) {
            return AT_ERROR;
        }

        AT_SetWifiRF(g_atParaList[AT_TSELRF_GROUP].paraValue);

        bufLen = snprintf_s(acCmd, (VOS_SIZE_T)sizeof(acCmd), (VOS_SIZE_T)(sizeof(acCmd) - 1), "wl txchain %d",
                            (g_atParaList[AT_TSELRF_GROUP].paraValue + 1));
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(acCmd), sizeof(acCmd) - 1);

        AT_WIFI_TEST_CMD_CALL(acCmd);

        bufLen = snprintf_s(acCmd, (VOS_SIZE_T)sizeof(acCmd), (VOS_SIZE_T)(sizeof(acCmd) - 1), "wl rxchain %d",
                            (g_atParaList[AT_TSELRF_GROUP].paraValue + 1));
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(acCmd), sizeof(acCmd) - 1);

        AT_WIFI_TEST_CMD_CALL(acCmd);

        bufLen = snprintf_s(acCmd, (VOS_SIZE_T)sizeof(acCmd), (VOS_SIZE_T)(sizeof(acCmd) - 1), "wl txant %d",
                            g_atParaList[AT_TSELRF_GROUP].paraValue);
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(acCmd), sizeof(acCmd) - 1);

        AT_WIFI_TEST_CMD_CALL(acCmd);

        bufLen = snprintf_s(acCmd, (VOS_SIZE_T)sizeof(acCmd), (VOS_SIZE_T)(sizeof(acCmd) - 1), "wl antdiv %d",
                            g_atParaList[AT_TSELRF_GROUP].paraValue);
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(acCmd), sizeof(acCmd) - 1);

        AT_WIFI_TEST_CMD_CALL(acCmd);

        return AT_OK;
    }
#endif

    return ERR_MSP_INVALID_PARAMETER;
}

VOS_UINT32 atSetTselrfPara(VOS_UINT8 clientId)
{
    FTM_SetTselrfReq tselrfSetReq = {0};
    VOS_UINT32       rst          = AT_SUCCESS;
    VOS_UINT8        path;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if ((g_atParaIndex != AT_TSELRF_PARA_MIN_NUM) && (g_atParaIndex != AT_TSELRF_PARA_MAX_NUM)) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (g_atParaList[AT_TSELRF_PATH_ID].paraLen == 0) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    path = (VOS_UINT8)(g_atParaList[AT_TSELRF_PATH_ID].paraValue);

    if ((path == FTM_TSELRF_FDD_LTE_MAIN) || (path == FTM_TSELRF_TDD_LTE_MAIN) || (path == FTM_TSELRF_FDD_LTE_SUB) ||
        (path == FTM_TSELRF_TDD_LTE_SUB) || (path == FTM_TSELRF_FDD_LTE_MIMO) || (path == FTM_TSELRF_TDD_LTE_MIMO) ||
        (path == FTM_TSELRF_TD)) {
        tselrfSetReq.path  = path;
        tselrfSetReq.group = (VOS_UINT8)g_atParaList[AT_TSELRF_GROUP].paraValue;
    } else {
        return AT_SetWifiTselrfPara(path);
    }

    rst = atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_FTM_SET_TSELRF_REQ, clientId, (VOS_UINT8 *)(&tselrfSetReq),
                           sizeof(tselrfSetReq));

    if (rst == AT_SUCCESS) {
        /* 设置当前操作类型 */

        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_TSELRF_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return rst;
}

VOS_UINT32 atSetFRXONPara(VOS_UINT8 clientId)
{
    FTM_SetRxonReq fRXONSetReq = {0};
    VOS_UINT32     rst;

    /* 参数检查 */

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    fRXONSetReq.rxSwt = g_atParaList[0].paraValue;

    rst = atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_FTM_SET_RXON_REQ, clientId, (VOS_UINT8 *)(&fRXONSetReq),
                           sizeof(fRXONSetReq));

    if (rst == AT_SUCCESS) {
        /* 设置当前操作类型 */

        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_FRXON_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
VOS_UINT32 AT_SetWifiFwavePara(VOS_VOID)
{
    VOS_CHAR acCmd[200] = {0};
    VOS_INT32 bufLen;

    /* WIFI未Enable直接返回失败 */
    if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 向WIFI发送单音波形信号请求,发射波形的功率大小,G以0.01dBm为单位,需要给功率大小除100 */
    bufLen = snprintf_s(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "athtestcmd -ieth0 --tx sine --txpwr %d",
        g_atParaList[1].paraValue / 100);
    TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(acCmd), sizeof(acCmd) - 1);

    AT_WIFI_TEST_CMD_CALL(acCmd);

    return AT_OK;
}

VOS_UINT32 AT_ProcTSelRfWifiPara(VOS_VOID)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    if (AT_IsMiscSupport(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) {
        /* WIFI未Enable直接返回失败 */
        if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
            return AT_ERROR;
        }

        atDevCmdCtrl->deviceRatMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 At_SendContinuesWaveOnToHPA(VOS_UINT16 power, VOS_UINT8 indexNum)
{
    AT_HPA_RfCfgReq   *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT16         mask;
    VOS_UINT8          ctrlType = WDSP_CTRL_TX_ONE_TONE;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 打开调制 */
    if (atDevCmdCtrl->cltEnableFlg == AT_DSP_CLT_ENABLE) {
        ctrlType = WDSP_CTRL_TX_THREE_TONE;
    }

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_HPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_HPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendContinuesWaveOnToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);

    /* 填写消息体 */
    msg->msgId = ID_AT_HPA_RF_CFG_REQ;

    /* Tx Cfg */
    mask                   = W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_POWER;
    msg->rfCfgPara.txPower = (VOS_INT16)power;

    /* 按位标识配置类型 */
    msg->rfCfgPara.mask      = mask;
    msg->rfCfgPara.txAfcInit = W_AFC_INIT_VALUE;

    /* 打开单音信号 */
    msg->rfCfgPara.txEnable = ctrlType;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendContinuesWaveOnToHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 At_SendContinuesWaveOnToCHPA(VOS_UINT8 ctrlType, VOS_UINT16 power)
{
    AT_CHPA_RfCfgReq *msg = VOS_NULL_PTR;
    VOS_UINT32        length;
    VOS_UINT16        mask;

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_CHPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_CHPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendContinuesWaveOnToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, UPHY_PID_CSDR_1X_CM, length);

    /* 填写消息体 */
    msg->msgId = ID_AT_CHPA_RF_CFG_REQ;

    /* Tx Cfg */
    mask                   = W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_POWER;
    msg->rfCfgPara.txPower = (VOS_INT16)power;

    /* 按位标识配置类型 */
    msg->rfCfgPara.mask      = mask;
    msg->rfCfgPara.txAfcInit = W_AFC_INIT_VALUE;

    /* 打开单音信号 */
    msg->rfCfgPara.txEnable = ctrlType;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendContinuesWaveOnToCHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}

VOS_UINT32 At_SendTxOnOffToCHPA(VOS_UINT8 switchStatus)
{
    AT_CHPA_RfCfgReq  *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT16         mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_CHPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_CHPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendTxOnOffToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, UPHY_PID_CSDR_1X_CM, length);

    /* 填写消息体 */
    msg->msgId = ID_AT_CHPA_RF_CFG_REQ;

    /* Tx Cfg */
    mask = W_RF_MASK_AFC | W_RF_MASK_TX_ARFCN | W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_PAMODE | W_RF_MASK_TX_POWDET;

    mask                   = mask | W_RF_MASK_TX_POWER;
    msg->rfCfgPara.txPower = (VOS_INT16)atDevCmdCtrl->power;

    /* Tx Cfg */
    msg->rfCfgPara.mask      = mask;             /* 按位标识配置类型 */
    msg->rfCfgPara.txAfcInit = W_AFC_INIT_VALUE; /* AFC */
    /* atDevCmdCtrl->stDspBandArfcn.usDspBand;     1,2,3...,协议中的band编号,注意不是BandId */
    msg->rfCfgPara.txBand    = 0;
    msg->rfCfgPara.txFreqNum = atDevCmdCtrl->dspBandArfcn.ulArfcn; /* Arfcn */
    msg->rfCfgPara.txPaMode  = atDevCmdCtrl->paLevel;

    if (switchStatus == AT_DSP_RF_SWITCH_ON) {
        msg->rfCfgPara.txEnable = WDSP_CTRL_TX_ON; /* 打开发射TX */
    } else {
        msg->rfCfgPara.txEnable = WDSP_CTRL_TX_OFF; /* 关闭发射TX */
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendTxOnOffToCHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}
#endif

VOS_UINT32 AT_SetFwavePara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

#if (FEATURE_LTE == FEATURE_ON)
    if (At_IsLteRatMode(atDevCmdCtrl->deviceRatMode) == VOS_TRUE) {
        return atSetFWAVEPara(indexNum);
    }
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_TDSCDMA) {
        return atSetFWAVEPara(indexNum);
    }
#endif

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数个数不正确，必须包括波形类型和波形功率 */
    if (g_atParaIndex != AT_FWAVE_PARA_NUM) {
        return AT_ERROR;
    }

    /* 目前波形类型只支持设置单音 */
    if (g_atParaList[AT_FWAVE_WAVEFORM_TYPE].paraValue > AT_FWAVE_WAVEFORM_TYPE_SUPPORT_NUM) {
        return AT_ERROR;
    }

    /* 该命令需在非信令模式下使用 */
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_ERROR;
    }

    /* WIFI */
    if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_WIFI) {
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
        return AT_SetWifiFwavePara();
#else
        return AT_ERROR;
#endif
    }

    /*
     * 把设置保存在本地变量
     * AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值
     * 功率值以0.01为单位，传给DSP的值会除10，所以AT还需要将该值再除10
     */
    atDevCmdCtrl->power     = (VOS_UINT16)(g_atParaList[AT_FWAVE_WAVEFORM_POWER].paraValue / 10);
    atDevCmdCtrl->powerFlag = VOS_TRUE;
    atDevCmdCtrl->fdacFlag  = VOS_FALSE;
    /* 记录下type信息，并转换为G物理层使用的TxMode，在向物理层发送ID_AT_GHPA_RF_TX_CFG_REQ时携带 */
    if (g_atParaList[AT_FWAVE_WAVEFORM_TYPE].paraValue == AT_FWAVE_WAVEFORM_BPSK) {
        atDevCmdCtrl->txMode = AT_FWAVE_WAVEFORM_CONTINUES_WAVE;
    } else {
        atDevCmdCtrl->txMode = (VOS_UINT16)g_atParaList[AT_FWAVE_WAVEFORM_TYPE].paraValue;
    }

    /* WCDMA */
    if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_WCDMA) {
        /* 向WDSP发送开关单音信号的原语请求 */
        if (At_SendContinuesWaveOnToHPA(atDevCmdCtrl->power, indexNum) == AT_FAILURE) {
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SET_FWAVE;
        atDevCmdCtrl->index                   = indexNum;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_CDMA) {
        /* 向X DSP发送开关单音信号的原语请求 */
        if (At_SendContinuesWaveOnToCHPA(WDSP_CTRL_TX_ONE_TONE, atDevCmdCtrl->power) == AT_FAILURE) {
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SET_FWAVE;
        atDevCmdCtrl->index                   = indexNum;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }
#endif

    return AT_OK;
}

VOS_UINT32 At_CheckWifiFChanPara(VOS_VOID)
{
    if (((g_atParaList[0].paraValue == AT_RAT_MODE_WIFI) && (g_atParaList[1].paraValue != AT_BAND_WIFI)) ||
        ((g_atParaList[1].paraValue == AT_BAND_WIFI) && (g_atParaList[0].paraValue != AT_RAT_MODE_WIFI))) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 At_CheckSupportFChanRat(VOS_VOID)
{
    if ((g_atParaList[0].paraValue != AT_RAT_MODE_GSM) && (g_atParaList[0].paraValue != AT_RAT_MODE_EDGE) &&
        (g_atParaList[0].paraValue != AT_RAT_MODE_WCDMA) && (g_atParaList[0].paraValue != AT_RAT_MODE_AWS)
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        && (g_atParaList[0].paraValue != AT_RAT_MODE_CDMA)
#endif
    ) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT8 At_GetDspLoadMode(VOS_UINT32 ratMode)
{
    if ((ratMode == AT_RAT_MODE_WCDMA) || (ratMode == AT_RAT_MODE_AWS)) {
        return VOS_RATMODE_WCDMA;
    } else if ((ratMode == AT_RAT_MODE_GSM) || (ratMode == AT_RAT_MODE_EDGE)) {
        return VOS_RATMODE_GSM;
    }
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    else if (ratMode == AT_RAT_MODE_CDMA) {
        return VOS_RATMODE_1X;
    }
#endif
    else {
        return VOS_RATMODE_BUTT;
    }
}

VOS_UINT32 At_SetFChanPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl    *atDevCmdCtrl = VOS_NULL_PTR;
    DRV_AGENT_FchanSetReq fchanSetReq;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 调用 LTE 模的接口分支 */
#if (FEATURE_LTE == FEATURE_ON)
    if (At_IsLteRatMode(g_atParaList[0].paraValue) == VOS_TRUE) {
        atDevCmdCtrl->deviceRatMode = (AT_DEVICE_CmdRatModeUint8)(g_atParaList[0].paraValue);
        return atSetFCHANPara(indexNum);
    }
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    if (g_atParaList[0].paraValue == AT_RAT_MODE_TDSCDMA) {
        atDevCmdCtrl->deviceRatMode = (AT_DEVICE_CmdRatModeUint8)(g_atParaList[0].paraValue);
        return atSetFCHANPara(indexNum);
    }
#endif

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_FCHAN_OTHER_ERR;
    }
    /* 参数不符合要求 */
    if (g_atParaIndex != AT_FCHAN_PARA_NUM) {
        return AT_FCHAN_OTHER_ERR;
    }

    /* 清除WIFI模式 */
    /* WIFI的第一个参数必须为8，第二个参数必须为15 */
    if (At_CheckWifiFChanPara() == VOS_FALSE) {
        return AT_CHANNEL_NOT_SET;
    }

    /* WIFI 分支 */
    if (g_atParaList[0].paraValue == AT_RAT_MODE_WIFI) {
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
        if (AT_IsMiscSupport(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) {
            /* WIFI未Enable直接返回失败 */
            if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
                return AT_FCHAN_OTHER_ERR;
            }

            atDevCmdCtrl->deviceRatMode = AT_RAT_MODE_WIFI;

            return AT_OK;
        }
#else
        return AT_ERROR;
#endif
    }

    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 检查FCHAN 的接入模式是否支持 */
    if (At_CheckSupportFChanRat() == VOS_FALSE) {
        return AT_DEVICE_MODE_ERROR;
    }

    if (g_atParaList[1].paraValue >= AT_BAND_BUTT) {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    (VOS_VOID)memset_s(&fchanSetReq, sizeof(fchanSetReq), 0x00, sizeof(DRV_AGENT_FchanSetReq));

    fchanSetReq.loadDspMode    = At_GetDspLoadMode(g_atParaList[0].paraValue);
    fchanSetReq.currentDspMode = At_GetDspLoadMode(atDevCmdCtrl->deviceRatMode); /* 当前接入模式 */
    fchanSetReq.dspLoadFlag    = atDevCmdCtrl->dspLoadFlag;
    fchanSetReq.deviceRatMode  = (VOS_UINT8)g_atParaList[0].paraValue;
    fchanSetReq.deviceAtBand   = (VOS_UINT8)g_atParaList[1].paraValue;
    fchanSetReq.channelNo      = (VOS_UINT16)g_atParaList[AT_FCHAN_CHANNEL_NO].paraValue;

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, DRV_AGENT_FCHAN_SET_REQ,
                               (VOS_UINT8 *)&fchanSetReq, sizeof(fchanSetReq), I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FCHAN_SET; /* 设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                              /* 等待异步事件返回 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SendRfCfgAntSelToHPA(VOS_UINT8 divOrPriOn, VOS_UINT8 indexNum)
{
    AT_HPA_RfCfgReq   *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT16         mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_HPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    msg = (AT_HPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendRfCfgAntSelToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);

    /* 填写消息体 */
    msg->msgId = ID_AT_HPA_RF_CFG_REQ;
    mask       = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL | W_RF_MASK_RX_RXONOFF;

    /* 按位标识配置类型 */
    msg->rfCfgPara.mask = mask;

    if (divOrPriOn == AT_RX_DIV_ON) {
        msg->rfCfgPara.rxAntSel = ANT_TWO;
        msg->rfCfgPara.rxEnable = DSP_CTRL_RX_ALL_ANT_ON;
    } else {
        msg->rfCfgPara.rxAntSel = ANT_ONE;
        msg->rfCfgPara.rxEnable = DSP_CTRL_RX_ANT1_ON;
    }

    msg->rfCfgPara.rxBand    = atDevCmdCtrl->dspBandArfcn.dspBand;
    msg->rfCfgPara.rxFreqNum = atDevCmdCtrl->dspBandArfcn.dlArfcn;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendRfCfgAntSelToHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ProcTSelRfWDivPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    if (atDevCmdCtrl->rxOnOff != DRV_AGENT_DSP_RF_SWITCH_ON) {
        atDevCmdCtrl->priOrDiv = AT_RX_DIV_ON;
        return AT_OK;
    }
    if (At_SendRfCfgAntSelToHPA(AT_RX_DIV_ON, indexNum) == AT_FAILURE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TSELRF_SET;
    atDevCmdCtrl->index                   = indexNum;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_ProcTSelRfWPriPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    if (atDevCmdCtrl->rxOnOff != DRV_AGENT_DSP_RF_SWITCH_ON) {
        atDevCmdCtrl->priOrDiv = AT_RX_PRI_ON;
        return AT_OK;
    }

    if (At_SendRfCfgAntSelToHPA(AT_RX_PRI_ON, indexNum) == AT_FAILURE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TSELRF_SET;
    atDevCmdCtrl->index                   = indexNum;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_BOOL AT_GetTseLrfLoadDspInfo(AT_TSELRF_PathUint32 path, DRV_AGENT_TselrfSetReq *tseLrf)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_BOOL           loadDsp;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* ^TSELRF命令设置的射频通路编号为GSM且当前已经LOAD了该通路，无需LOAD */
    if (path == AT_TSELRF_PATH_GSM) {
        if ((atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_GSM) && (atDevCmdCtrl->dspLoadFlag == VOS_TRUE)) {
            loadDsp = VOS_FALSE;
        } else {
            tseLrf->loadDspMode   = VOS_RATMODE_GSM;
            tseLrf->deviceRatMode = AT_RAT_MODE_GSM;
            loadDsp               = VOS_TRUE;
        }
        return loadDsp;
    }

    /* ^TSELRF命令设置的射频通路编号为WCDMA主集且当前已经LOAD了该通路，无需LOAD */
    if (path == AT_TSELRF_PATH_WCDMA_PRI) {
        if (((atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_WCDMA) || (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_AWS)) &&
            (atDevCmdCtrl->dspLoadFlag == VOS_TRUE)) {
            loadDsp = VOS_FALSE;
        } else {
            tseLrf->loadDspMode   = VOS_RATMODE_WCDMA;
            tseLrf->deviceRatMode = AT_RAT_MODE_WCDMA;
            loadDsp               = VOS_TRUE;
        }
        return loadDsp;
    }

    loadDsp = VOS_FALSE;

    AT_WARN_LOG("AT_GetTseLrfLoadDspInfo: enPath only support GSM or WCDMA primary.");

    return loadDsp;
}

VOS_UINT32 AT_SetTSelRfPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl     *atDevCmdCtrl = VOS_NULL_PTR;
    DRV_AGENT_TselrfSetReq tseLrf;
    VOS_BOOL               bLoadDsp;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数不符合要求 */
    if ((g_atParaIndex > AT_TSELRF_PARA_MAX_NUM) || (g_atParaList[AT_TSELRF_PATH_ID].paraLen == 0)) {
        return AT_ERROR;
    }

    if (g_atParaList[AT_TSELRF_PATH_ID].paraValue == AT_TSELRF_PATH_WIFI) {
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
        return AT_ProcTSelRfWifiPara();
#else
        return AT_ERROR;
#endif
    }

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    if (g_atParaList[AT_TSELRF_PATH_ID].paraValue == AT_TSELRF_PATH_TD) {
        return atSetTselrfPara(indexNum);
    }

#endif

#if (FEATURE_LTE == FEATURE_ON)
    if ((g_atParaList[AT_TSELRF_PATH_ID].paraValue != AT_TSELRF_PATH_WCDMA_PRI) &&
        (g_atParaList[AT_TSELRF_PATH_ID].paraValue != AT_TSELRF_PATH_WCDMA_DIV) &&
        (g_atParaList[AT_TSELRF_PATH_ID].paraValue != AT_TSELRF_PATH_GSM)) {
        return atSetTselrfPara(indexNum);
    }
#endif

    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_ERROR;
    }

    /* 打开分集必须在FRXON之后，参考RXDIV实现 */
    if (g_atParaList[AT_TSELRF_PATH_ID].paraValue == AT_TSELRF_PATH_WCDMA_DIV) {
        return AT_ProcTSelRfWDivPara(indexNum);
    }

    if (g_atParaList[AT_TSELRF_PATH_ID].paraValue == AT_TSELRF_PATH_WCDMA_PRI) {
        return AT_ProcTSelRfWPriPara(indexNum);
    }

    /* 此处判断是否需要重新加载DSP: 需要则发请求到C核加载DSP，否则，直接返回OK */
    bLoadDsp = AT_GetTseLrfLoadDspInfo(g_atParaList[AT_TSELRF_PATH_ID].paraValue, &tseLrf);
    if (bLoadDsp == VOS_TRUE) {
        if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                   DRV_AGENT_TSELRF_SET_REQ, (VOS_UINT8 *)&tseLrf, sizeof(tseLrf),
                                   I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TSELRF_SET; /* 设置当前操作模式 */
            return AT_WAIT_ASYNC_RETURN;                               /* 等待异步事件返回 */
        } else {
            return AT_ERROR;
        }
    }

    return AT_OK;
}

VOS_UINT32 At_SetCltPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不符合要求 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    atDevCmdCtrl->cltEnableFlg = (AT_DSP_CltEnableUint8)g_atParaList[0].paraValue;

    return AT_OK; /* 返回命令处理处理成功 */
}

VOS_UINT32 AT_SetMipiRdPara(VOS_UINT8 indexNum)
{
    AT_HPA_MipiRdReq  *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_MIPIRD_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    if (g_atParaList[0].paraValue == AT_RAT_MODE_FDD_LTE || g_atParaList[0].paraValue == AT_RAT_MODE_TDD_LTE) {
        return AT_ERROR;
    }

    length = sizeof(AT_HPA_MipiRdReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_HPA_MipiRdReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_SetMipiRdPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue == AT_RAT_MODE_GSM) {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_GPHY), length);
    } else {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);
    }

    msg->msgId         = ID_AT_HPA_MIPI_RD_REQ;
    msg->channel       = (VOS_UINT16)g_atParaList[1].paraValue;
    msg->secondaryAddr = (VOS_UINT16)g_atParaList[AT_MIPIRD_SECONDARY_ADDR].paraValue;
    msg->reg           = (VOS_UINT16)g_atParaList[AT_MIPIRD_REG_PARA].paraValue;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("AT_SetMipiRdPara: Send msg fail!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MIPI_RD; /* 设置当前操作模式 */
    atDevCmdCtrl->index                   = indexNum;

    return AT_WAIT_ASYNC_RETURN; /* 等待异步事件返回 */
}

/* ***************************************************************************** */

/* 功能描述: 打开上行信道 */

/*  */

/* 参数说明: */

/*   ulIndex [in] ... */

/*                ... */

/*  */

/* 返 回 值: */

/*    TODO: ... */

/* ***************************************************************************** */
VOS_UINT32 atSetFTXONPara(VOS_UINT8 clientId)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    FTM_SetTxonReq     fTXONSetReq  = {
        (FTM_TXON_SWT_ENUM)0,
    };
    VOS_UINT32 rst;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    // 参数检查

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    fTXONSetReq.swtich = (FTM_TXON_SWT_ENUM)(g_atParaList[0].paraValue);
    /* 只有在^FTXON=1 时才下发 */
    if (fTXONSetReq.swtich == EN_FTM_TXON_UL_RFBBP_OPEN) {
        fTXONSetReq.cltEnableFlg = (FTM_CltEnableUint8)atDevCmdCtrl->cltEnableFlg;
    }

    rst = atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_FTM_SET_TXON_REQ, clientId, (VOS_UINT8 *)(&fTXONSetReq),
                           sizeof(fTXONSetReq));

    if (rst == AT_SUCCESS) {
        /* 设置当前操作类型 */

        g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_FTXON_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 At_CheckFTxonPara(VOS_UINT8 switchStatus)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    if (switchStatus > AT_DSP_RF_SWITCH_ON) {
        return AT_FTXON_OTHER_ERR;
    }

    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    if (atDevCmdCtrl->dspLoadFlag == VOS_FALSE) {
        return AT_CHANNEL_NOT_SET;
    }

    return AT_OK;
}

TAF_UINT32 At_SendTxOnOffToHPA(TAF_UINT8 ucSwitch, TAF_UINT8 indexNum)
{
    AT_HPA_RfCfgReq   *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT16         mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_HPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_HPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);
    mask   = 0;

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendTxOnOffToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);

    /* 填写消息体 */
    msg->msgId = ID_AT_HPA_RF_CFG_REQ;

    /* Tx Cfg */
    mask = W_RF_MASK_AFC | W_RF_MASK_TX_TXONOFF | W_RF_MASK_TX_PAMODE | W_RF_MASK_TX_POWDET;

    /* AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值 */
    if (atDevCmdCtrl->fdacFlag == VOS_TRUE) {
        mask                 = mask | W_RF_MASK_TX_AGC;
        msg->rfCfgPara.txAgc = atDevCmdCtrl->fdac;
    } else {
        mask                   = mask | W_RF_MASK_TX_POWER;
        msg->rfCfgPara.txPower = (VOS_INT16)atDevCmdCtrl->power;
    }

    /* Tx Cfg */
    msg->rfCfgPara.mask      = mask;             /* 按位标识配置类型 */
    msg->rfCfgPara.txAfcInit = W_AFC_INIT_VALUE; /* AFC */
    /* 1,2,3...,协议中的band编号,注意不是BandId */
    msg->rfCfgPara.txBand    = atDevCmdCtrl->dspBandArfcn.dspBand;
    msg->rfCfgPara.txFreqNum = atDevCmdCtrl->dspBandArfcn.ulArfcn; /* Arfcn */
    msg->rfCfgPara.txPaMode  = atDevCmdCtrl->paLevel;

    if (ucSwitch == AT_DSP_RF_SWITCH_OFF) {
        msg->rfCfgPara.txEnable = WDSP_CTRL_TX_OFF; /* 关闭发射TX */
    } else {
        msg->rfCfgPara.txEnable = WDSP_CTRL_TX_ON; /* 打开发射TX */
        msg->rfCfgPara.mask |= W_RF_MASK_TX_ARFCN;

        /* 只有在^FTXON=1 时才下发 */
        if (atDevCmdCtrl->cltEnableFlg == AT_DSP_CLT_ENABLE) {
            msg->rfCfgPara.mask |= W_RF_MASK_TX_CLT;
            msg->rfCfgPara.mask &= ~(W_RF_MASK_TX_POWDET);
        }
        if (atDevCmdCtrl->dcxoTempCompEnableFlg == AT_DCXOTEMPCOMP_ENABLE) {
            msg->rfCfgPara.mask |= W_RF_MASK_TX_DCXOTEMPCOMP;
            msg->rfCfgPara.mask &= ~(W_RF_MASK_AFC);
        }
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendTxOnOffToHPA: Send msg fail!");
        return AT_FAILURE;
    }
    return AT_SUCCESS;
}

VOS_UINT32 At_SendTxOnOffToGHPA(VOS_UINT8 indexNum, VOS_UINT8 ucSwitch)
{
    AT_GHPA_RfTxCfgReq *txMsg        = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl  *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32          length;
    VOS_UINT16          mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_GHPA_RfTxCfgReq消息 */
    length = sizeof(AT_GHPA_RfTxCfgReq) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    txMsg = (AT_GHPA_RfTxCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);
    /*lint -restore */
    if (txMsg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendRfCfgReqToGHPA: alloc Tx msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)txMsg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_GPHY), length);

    /* Tx Cfg */

    /* 填写消息体 */
    txMsg->msgId = ID_AT_GHPA_RF_TX_CFG_REQ;
    mask         = G_RF_MASK_TX_AFC;

    /* AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值 */
    /*
     * 发射控制方式：
     * 0：GMSK电压控制,此方式下usTxVpa要配置；
     * 1：功率控制,此方式下usTxPower要配置；
     * 2：8PSK PaVbias电压&DBB Atten&RF Atten控制，
     * usPAVbias和usRfAtten,sDbbAtten三个参数都要配置；
     */
    if (atDevCmdCtrl->fdacFlag == VOS_TRUE) {
        txMsg->ctrlMode = AT_GHPA_RF_CTRLMODE_TYPE_GMSK;
        txMsg->txVpa    = atDevCmdCtrl->fdac; /* DAC设置 */
    } else {
        txMsg->ctrlMode = AT_GHPA_RF_CTRLMODE_TYPE_TXPOWER;
        txMsg->txPower  = atDevCmdCtrl->power;
    }

    /* Tx Cfg */
    txMsg->mask    = mask; /* 按位标识配置类型 */
    txMsg->afc     = 0;    /* AFC */
    txMsg->freqNum = (VOS_UINT16)((atDevCmdCtrl->dspBandArfcn.dspBand << 12) | atDevCmdCtrl->dspBandArfcn.ulArfcn);
    txMsg->txMode  = atDevCmdCtrl->txMode; /* 0:burst发送; 1:连续发送 */
    txMsg->modType = G_MOD_TYPE_GMSK;      /* 发射调制方式:0表示GMSK调制;1表示8PSK调制方式 */

    if (ucSwitch == AT_DSP_RF_SWITCH_ON) {
        txMsg->txEnable = GDSP_CTRL_TX_ON;
    } else {
        txMsg->txEnable = GDSP_CTRL_TX_OFF;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, txMsg) != VOS_OK) {
        AT_WARN_LOG("At_SendTxOnOffToGHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_SetFTxonPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    TAF_UINT8          switchStatus;
    VOS_UINT32         result;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 添加 LTE 模的接口分支 */
#if (FEATURE_LTE == FEATURE_ON)
    if ((atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_FDD_LTE) ||
#if (FEATURE_LTEV == FEATURE_ON)
        (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_LTEV) ||
#endif
        (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_TDD_LTE)) {
        return atSetFTXONPara(indexNum);
    }
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_TDSCDMA) {
        return atSetFTXONPara(indexNum);
    }

#endif
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_FTXON_OTHER_ERR;
    }
    /* 参数不符合要求 */
    if (g_atParaIndex != 1) {
        return AT_FTXON_OTHER_ERR;
    }

    switchStatus                  = (VOS_UINT8)g_atParaList[0].paraValue;
    atDevCmdCtrl->tempRxorTxOnOff = switchStatus;
    result                        = At_CheckFTxonPara(switchStatus);
    if (result != AT_OK) {
        return result;
    }

    if ((atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_WCDMA) || (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_AWS)) {
        if (At_SendTxOnOffToHPA(switchStatus, indexNum) == AT_FAILURE) {
            return AT_FTXON_SET_FAIL;
        }
    }
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    else if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_CDMA) {
        if (At_SendTxOnOffToCHPA(switchStatus) == AT_FAILURE) {
            return AT_FTXON_SET_FAIL;
        }
    }
#endif
    else {
        if (At_SendTxOnOffToGHPA(indexNum, switchStatus) == AT_FAILURE) {
            return AT_FTXON_SET_FAIL;
        }
    }
    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SET_FTXON;
    atDevCmdCtrl->index                   = indexNum;

    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 At_SetDpdtPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetDpdtValueReq atCmd;
    VOS_UINT32             rst;

    /* 参数检查 */
    if (g_atParaIndex != AT_DPDT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_SetDpdtValueReq));
    atCmd.ratMode   = (AT_MTA_CmdRatmodeUint8)g_atParaList[0].paraValue;
    atCmd.dpdtValue = g_atParaList[1].paraValue;

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_DPDT_VALUE_REQ, (VOS_UINT8 *)&atCmd,
                                 sizeof(AT_MTA_SetDpdtValueReq), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 indexNum)
{
    AT_MTA_QryDpdtValueReq atCmd;
    VOS_UINT32             rst;

    /* 参数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_QryDpdtValueReq));
    atCmd.ratMode = (AT_MTA_CmdRatmodeUint8)g_atParaList[0].paraValue;

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_QRY_DPDT_VALUE_REQ, (VOS_UINT8 *)&atCmd,
                                 sizeof(AT_MTA_QryDpdtValueReq), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DPDTQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

/*
 * 功能描述: ^DCXOTEMPCOMP设置命令处理函数
 */
VOS_UINT32 AT_SetDcxotempcompPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != AT_DCXOTEMPCOMP_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*  不是非信令模式下发返回错误 */
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    atDevCmdCtrl->dcxoTempCompEnableFlg = (AT_DCXOTEMPCOMP_EnableUint8)g_atParaList[0].paraValue;

    return AT_OK;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 At_SendRxOnOffToCHPA(VOS_UINT32 rxSwitch)
{
    AT_CHPA_RfCfgReq  *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT16         mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_CHPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_CHPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendRxOnOffToCHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, UPHY_PID_CSDR_1X_CM, length);

    /* 填写消息体 */
    msg->msgId = ID_AT_CHPA_RF_CFG_REQ;

    mask = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL | W_RF_MASK_RX_RXONOFF;

    /* 按位标识配置类型 */
    msg->rfCfgPara.mask = mask;

    /* 1,2,3...,协议中的band编号 */
    msg->rfCfgPara.rxBand    = 0; /* atDevCmdCtrl->stDspBandArfcn.usDspBand; */
    msg->rfCfgPara.rxFreqNum = atDevCmdCtrl->dspBandArfcn.ulArfcn;

    msg->rfCfgPara.rxAntSel = ANT_ONE;

    if (rxSwitch == AT_DSP_RF_SWITCH_ON) {
        /* CPU控制打开射频芯片,天线1 */
        msg->rfCfgPara.rxEnable = DSP_CTRL_RX_ANT1_ON;
    } else {
        /* 关闭接收RX */
        msg->rfCfgPara.rxEnable = DSP_CTRL_RX_OFF;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendRxOnOffToCHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}
#endif

VOS_UINT32 At_SendRxOnOffToGHPA(VOS_UINT8 indexNum, VOS_UINT32 rxSwitch)
{
    AT_GHPA_RfRxCfgReq *rxMsg        = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl  *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32          length;
    VOS_UINT16          mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_GHPA_RfRxCfgReq消息 */
    length = sizeof(AT_GHPA_RfRxCfgReq) - VOS_MSG_HEAD_LENGTH;
    rxMsg  = (AT_GHPA_RfRxCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (rxMsg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendRxOnOffToGHPA: alloc Rx msg fail!");
        return AT_FAILURE;
    }

    /* Rx Cfg */
    TAF_CfgMsgHdr((MsgBlock *)rxMsg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_GPHY), length);

    rxMsg->msgId = ID_AT_GHPA_RF_RX_CFG_REQ;
    rxMsg->rsv   = 0;
    rxMsg->rsv2  = 0;

    mask = G_RF_MASK_RX_ARFCN | G_RF_MASK_RX_AGCMODE | G_RF_MASK_RX_AGCGAIN | G_RF_MASK_RX_MODE;

    /* Reference MASK_CAL_RF_G_RX_* section */
    rxMsg->mask = mask;

    rxMsg->freqNum = (VOS_UINT16)((atDevCmdCtrl->dspBandArfcn.dspBand << 12) | atDevCmdCtrl->dspBandArfcn.ulArfcn);

    /* 0:停止; 1:连续接收; */
    if (rxSwitch == TRUE) {
        rxMsg->rxMode = AT_GDSP_RX_MODE_CONTINOUS_BURST;
    } else {
        rxMsg->rxMode = AT_GDSP_RX_MODE_STOP;
    }

    /* 1:Fast AGC,0:Slow AGC */
    rxMsg->agcMode = AT_GDSP_RX_SLOW_AGC_MODE;

    /* AGC档位，共四档,取值为0-3 */
    rxMsg->agcGain = atDevCmdCtrl->lnaLevel;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, rxMsg) != VOS_OK) {
        AT_WARN_LOG("At_SendRxOnOffToGHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_SendRxOnOffToHPA(VOS_UINT32 rxSwitch, VOS_UINT8 indexNum)
{
    AT_HPA_RfCfgReq   *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;
    VOS_UINT16         mask;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 申请AT_HPA_RfCfgReq消息 */
    length = sizeof(AT_HPA_RfCfgReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_HPA_RfCfgReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SendRxOnOffToHPA: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);

    /* 填写消息体 */
    msg->msgId = ID_AT_HPA_RF_CFG_REQ;

    mask = W_RF_MASK_RX_ARFCN | W_RF_MASK_RX_ANTSEL | W_RF_MASK_RX_RXONOFF;

    /* 按位标识配置类型 */
    msg->rfCfgPara.mask = mask;

    /* 1,2,3...,协议中的band编号 */
    msg->rfCfgPara.rxBand    = atDevCmdCtrl->dspBandArfcn.dspBand;
    msg->rfCfgPara.rxFreqNum = atDevCmdCtrl->dspBandArfcn.dlArfcn;

    /*
     * DSP频段Band1-Band9格式转换为
     *    W Rf support
     *         bit8       bit7      bit6     bit5    bit4     bit3      bit2     bit1
     * WCDMA   900(VIII)  2600(VII) 800(VI)  850(V)  1700(IV) 1800(III) 1900(II) 2100(I) oct1
     *         spare      spare     spare    spare   spare    spare     spare   J1700(IX)oct2
     */

    if (atDevCmdCtrl->priOrDiv == AT_RX_DIV_ON) {
        msg->rfCfgPara.rxAntSel = ANT_TWO;
    } else {
        msg->rfCfgPara.rxAntSel = ANT_ONE;
    }

    /* LNA模式控制 */
    msg->rfCfgPara.rxLnaGainMode = atDevCmdCtrl->lnaLevel;

    if (rxSwitch == AT_DSP_RF_SWITCH_ON) {
        if (msg->rfCfgPara.rxAntSel == ANT_TWO) {
            /* CPU控制打开射频芯片,天线1和2 */
            msg->rfCfgPara.rxEnable = DSP_CTRL_RX_ALL_ANT_ON;
        } else {
            /* CPU控制打开射频芯片,天线1 */
            msg->rfCfgPara.rxEnable = DSP_CTRL_RX_ANT1_ON;
        }
    } else {
        /* 关闭接收RX */
        msg->rfCfgPara.rxEnable = DSP_CTRL_RX_OFF;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("At_SendRxOnOffToHPA: Send msg fail!");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_SetFRxonPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         switchStatus;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 调用 LTE 模的接口分支 */
#if (FEATURE_LTE == FEATURE_ON)
    if (At_IsLteRatMode(atDevCmdCtrl->deviceRatMode) == VOS_TRUE) {
        return atSetFRXONPara(indexNum);
    }
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
    if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_TDSCDMA) {
        return atSetFRXONPara(indexNum);
    }
#endif

    /* 命令状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_FRXON_OTHER_ERR;
    }

    /* 参数不符合要求 */
    if (g_atParaIndex != 1) {
        return AT_FRXON_OTHER_ERR;
    }

    /* 该AT命令在AT^TMODE=1非信令模式下使用 */
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该AT命令需要在AT^FCHAN设置非信令信道命令执行成功后使用 */
    if (atDevCmdCtrl->dspLoadFlag == VOS_FALSE) {
        return AT_CHANNEL_NOT_SET;
    }

    switchStatus                  = g_atParaList[0].paraValue;
    atDevCmdCtrl->tempRxorTxOnOff = (AT_DSP_RfOnOffUint8)switchStatus;

    /* 把开关接收机请求发给W物理层 */
    if ((atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_WCDMA) || (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_AWS)) {
        if (At_SendRxOnOffToHPA(switchStatus, indexNum) == AT_FAILURE) {
            return AT_FRXON_SET_FAIL;
        }
    }
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    else if (atDevCmdCtrl->deviceRatMode == AT_RAT_MODE_CDMA) {
        if (At_SendRxOnOffToCHPA(switchStatus) == AT_FAILURE) {
            return AT_FRXON_SET_FAIL;
        }
    }
#endif
    else {
        /* 把开关接收机请求发给G物理层 */
        if (At_SendRxOnOffToGHPA(indexNum, switchStatus) == AT_FAILURE) {
            return AT_FRXON_SET_FAIL;
        }
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SET_FRXON;
    atDevCmdCtrl->index                   = indexNum;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetMipiWrPara(VOS_UINT8 indexNum)
{
    AT_HPA_MipiWrReq  *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_MIPIWR_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    length = sizeof(AT_HPA_MipiWrReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_HPA_MipiWrReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_SetMipiWrPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue == AT_RAT_MODE_GSM) {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_GPHY), length);
    } else {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);
    }

    msg->msgId         = ID_AT_HPA_MIPI_WR_REQ;
    msg->secondaryAddr = (VOS_UINT16)g_atParaList[AT_CMD_MIPIWR_SECONDARY_ADDR].paraValue;
    msg->regAddr       = (VOS_UINT16)g_atParaList[AT_CMD_MIPIWR_REG_ADDR].paraValue;
    msg->regData       = (VOS_UINT16)g_atParaList[AT_CMD_MIPIWR_REG_DATA].paraValue;
    msg->mipiChannel   = (VOS_UINT16)g_atParaList[AT_CMD_MIPIWR_MIPI_CHANNEL].paraValue;

    /* GSM and UMTS share the same PID */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("AT_SetMipiWrPara: Send msg fail!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MIPI_WR; /* 设置当前操作模式 */
    atDevCmdCtrl->index                   = indexNum;

    return AT_WAIT_ASYNC_RETURN; /* 等待异步事件返回 */
}

VOS_UINT32 AT_SetSSIWrPara(VOS_UINT8 indexNum)
{
    AT_HPA_SsiWrReq   *msg          = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         length;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_SSIWR_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    length = sizeof(AT_HPA_SsiWrReq) - VOS_MSG_HEAD_LENGTH;
    msg    = (AT_HPA_SsiWrReq *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_SetSSIWrPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue == AT_RAT_MODE_GSM) {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_GPHY), length);
    } else {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, AT_GetDestPid(indexNum, I0_DSP_PID_WPHY), length);
    }

    msg->msgId   = ID_AT_HPA_SSI_WR_REQ;
    msg->rficSsi = (VOS_UINT16)g_atParaList[1].paraValue;
    msg->regAddr = (VOS_UINT16)g_atParaList[AT_SSIWR_PARA_REG_ADDR].paraValue;
    msg->data    = (VOS_UINT16)g_atParaList[AT_SSIWR_PARA_DATA].paraValue;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_WARN_LOG("AT_SetSSIWrPara: Send msg fail!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SSI_WR; /* 设置当前操作模式 */
    atDevCmdCtrl->index                   = indexNum;

    return AT_WAIT_ASYNC_RETURN; /* 等待异步事件返回 */
}

VOS_UINT32 AT_SetMipiReadPara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    AT_MTA_MipiReadReq mipiReadReq;
    VOS_UINT32         result;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&mipiReadReq, sizeof(mipiReadReq), 0x00, sizeof(AT_MTA_MipiReadReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数目不正确 */
    if (g_atParaIndex != AT_MIPIREAD_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 不是非信令模式 */
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 参数长度检查 */
    if ((g_atParaList[AT_MIPI_READ_PARA_READ_TYPE].paraLen == 0) ||
        (g_atParaList[AT_MIPI_READ_PARA_MIPI_ID].paraLen == 0) ||
        (g_atParaList[AT_MIPI_READ_PARA_SECONDARY_ID].paraLen == 0) ||
        (g_atParaList[AT_MIPI_READ_PARA_REG_ADDR].paraLen == 0) ||
        (g_atParaList[AT_MIPI_READ_PARA_SPEED_TYPE].paraLen == 0) ||
        (g_atParaList[AT_MIPI_READ_PARA_READ_BIT_MASK].paraLen == 0) ||
        (g_atParaList[AT_MIPI_READ_PARA_REAERVED].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写消息参数 */
    mipiReadReq.readType    = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_READ_TYPE].paraValue;
    mipiReadReq.mipiId      = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_MIPI_ID].paraValue;
    mipiReadReq.secondaryId = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_SECONDARY_ID].paraValue;
    mipiReadReq.regAddr     = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_REG_ADDR].paraValue;
    mipiReadReq.speedType   = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_SPEED_TYPE].paraValue;
    mipiReadReq.readBitMask = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_READ_BIT_MASK].paraValue;
    mipiReadReq.reserved1   = (VOS_UINT16)g_atParaList[AT_MIPI_READ_PARA_REAERVED].paraValue;

    /* 发送消息给MTA */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_MIPIREAD_SET_REQ, (VOS_UINT8 *)&mipiReadReq,
                                    (VOS_SIZE_T)sizeof(mipiReadReq), I0_UEPS_PID_MTA);
    /* 发送失败 */
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetMipiReadPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MIPIREAD_SET;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetPhyMipiWritePara(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl     *atDevCmdCtrl = VOS_NULL_PTR;
    AT_MTA_PhyMipiWriteReq phyMipiWriteReq;
    VOS_UINT32             result;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&phyMipiWriteReq, sizeof(phyMipiWriteReq), 0x00, sizeof(AT_MTA_PhyMipiWriteReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数目不正确 */
    if (g_atParaIndex != AT_PHYMIPIWRITE_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 不是非信令模式 */
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 参数长度检查 */
    if ((g_atParaList[AT_PHYMIPIWRITE_WR_TYPE].paraLen == 0) ||
        (g_atParaList[AT_PHYMIPIWRITE_MIPI_ID].paraLen == 0) ||
        (g_atParaList[AT_PHYMIPIWRITE_SECONDARY_ID].paraLen == 0) ||
        (g_atParaList[AT_PHYMIPIWRITE_REG_ADDR].paraLen == 0) ||
        (g_atParaList[AT_PHYMIPIWRITE_MIPI_DATA].paraLen == 0) ||
        (g_atParaList[AT_PHYMIPIWRITE_RESERVAED].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写消息参数 */
    phyMipiWriteReq.writeType   = (VOS_UINT16)g_atParaList[AT_PHYMIPIWRITE_WR_TYPE].paraValue;
    phyMipiWriteReq.mipiId      = (VOS_UINT16)g_atParaList[AT_PHYMIPIWRITE_MIPI_ID].paraValue;
    phyMipiWriteReq.secondaryId = (VOS_UINT16)g_atParaList[AT_PHYMIPIWRITE_SECONDARY_ID].paraValue;
    phyMipiWriteReq.regAddr     = (VOS_UINT16)g_atParaList[AT_PHYMIPIWRITE_REG_ADDR].paraValue;
    phyMipiWriteReq.mipiData    = (VOS_UINT16)g_atParaList[AT_PHYMIPIWRITE_MIPI_DATA].paraValue;
    phyMipiWriteReq.reserved1   = (VOS_UINT16)g_atParaList[AT_PHYMIPIWRITE_RESERVAED].paraValue;

    /* 发送消息给MTA */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_PHYMIPIWRITE_SET_REQ, (VOS_UINT8 *)&phyMipiWriteReq,
                                    (VOS_SIZE_T)sizeof(phyMipiWriteReq), I0_UEPS_PID_MTA);
    /* 发送失败 */
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetPhyMipiWritePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PHYMIPIWRITE_SET;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetFPllStatusQryPara(VOS_UINT8 indexNum)
{
    return AT_CME_OPERATION_NOT_SUPPORTED;
}

#else

VOS_UINT32 AT_CheckNrFwaveTypePara(VOS_UINT32 para)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (para) {
        case AT_FWAVE_TYPE_BPSK:
        case AT_FWAVE_TYPE_PI2_BPSK:
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_16QAM:
        case AT_FWAVE_TYPE_64QAM:
        case AT_FWAVE_TYPE_256QAM:
        case AT_FWAVE_TYPE_CONTINUE:
            result = VOS_TRUE;
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_CheckLteFwaveTypePara(VOS_UINT32 para)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (para) {
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_16QAM:
        case AT_FWAVE_TYPE_64QAM:
        case AT_FWAVE_TYPE_256QAM:
        case AT_FWAVE_TYPE_CONTINUE:
            result = VOS_TRUE;
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_CheckCFwaveTypePara(VOS_UINT32 para)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (para) {
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_CONTINUE:
            result = VOS_TRUE;
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_CheckWFwaveTypePara(VOS_UINT32 para)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (para) {
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_CONTINUE:
            result = VOS_TRUE;
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_CheckGFwaveTypePara(VOS_UINT32 para)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (para) {
        case AT_FWAVE_TYPE_GMSK:
        case AT_FWAVE_TYPE_8PSK:
        case AT_FWAVE_TYPE_CONTINUE:
            result = VOS_TRUE;
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_CheckFwaveTypePara(VOS_UINT32 para)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;

    atMtInfoCtx = AT_GetMtInfoCtx();
    result      = VOS_TRUE;
    switch (atMtInfoCtx->bbicInfo.currtRatMode) {
        case RAT_MODE_GSM:
            result = AT_CheckGFwaveTypePara(para);
            break;
        case RAT_MODE_WCDMA:
            result = AT_CheckWFwaveTypePara(para);
            break;
        case RAT_MODE_CDMA:
            result = AT_CheckCFwaveTypePara(para);
            break;
        case RAT_MODE_LTE:
            result = AT_CheckLteFwaveTypePara(para);
            break;
        case RAT_MODE_NR:
            result = AT_CheckNrFwaveTypePara(para);
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_CovertAtFwaveTypeToBbicCal(AT_FWAVE_TypeUint8 typeIn, MODU_TYPE_ENUM_UINT16 *typeOut)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (typeIn) {
        case AT_FWAVE_TYPE_BPSK:
            *typeOut = MODU_TYPE_LTE_BPSK;
            break;
        case AT_FWAVE_TYPE_PI2_BPSK:
            *typeOut = MODU_TYPE_LTE_PI2_BPSK;
            break;
        case AT_FWAVE_TYPE_QPSK:
            *typeOut = MODU_TYPE_LTE_QPSK;
            break;
        case AT_FWAVE_TYPE_16QAM:
            *typeOut = MODU_TYPE_LTE_16QAM;
            break;
        case AT_FWAVE_TYPE_64QAM:
            *typeOut = MODU_TYPE_LTE_64QAM;
            break;
        case AT_FWAVE_TYPE_256QAM:
            *typeOut = MODU_TYPE_LTE_256QAM;
            break;
        case AT_FWAVE_TYPE_GMSK:
            *typeOut = MODU_TYPE_GMSK;
            break;
        case AT_FWAVE_TYPE_8PSK:
            *typeOut = MODU_TYPE_8PSK;
            break;
        /* BBIC没有单音定义，所以付值为BUTT */
        case AT_FWAVE_TYPE_CONTINUE:
            *typeOut = MODU_TYPE_BUTT;
            break;
        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_SetFwavePara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    VOS_CHAR    acCmd[AT_FWAVE_CMD_ARRAY_LEN]  = {0};
    VOS_INT32   bufLen;
#endif

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("AT_SetFwavePara Enter");

    /* 状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确，必须包括波形类型和波形功率 */
    if (g_atParaIndex < AT_FWAVE_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 该命令需在非信令模式下使用 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* WIFI */
    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_WIFI) {
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
        /* WIFI未Enable直接返回失败 */
        if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
            return AT_WIFI_NOT_ENABLE;
        }

        /* 向WIFI发送单音波形信号请求,发射波形的功率大小,G以0.01dBm为单位,需要给功率大小除100 */
        bufLen = snprintf_s(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "athtestcmd -ieth0 --tx sine --txpwr %d",
                            g_atParaList[AT_FWAVE_WAVEFORM_POWER].paraValue / 100);
        TAF_SPRINTF_CHK_RTN_VAL_CONTINUE(bufLen, sizeof(acCmd), sizeof(acCmd) - 1);

        AT_WIFI_TEST_CMD_CALL(acCmd);

        return AT_OK;
#else
        return AT_ERROR;
#endif
    }

    /* 是否加载了DSP */
    if (atMtInfoCtx->atInfo.dspLoadFlag != VOS_TRUE) {
        return AT_NOT_LOAD_DSP;
    }

    /* 根据接入技术判断波形类型是否支持 */
    if (AT_CheckFwaveTypePara(g_atParaList[AT_FWAVE_WAVEFORM_TYPE].paraValue) == VOS_FALSE) {
        return AT_FWAVE_TYPE_ERR;
    }

    atMtInfoCtx->atInfo.faveType = (AT_FWAVE_TypeUint8)g_atParaList[AT_FWAVE_WAVEFORM_TYPE].paraValue;

    /* 转换TYPE 参数 */
    AT_CovertAtFwaveTypeToBbicCal((AT_FWAVE_TypeUint8)g_atParaList[AT_FWAVE_WAVEFORM_TYPE].paraValue,
                                  &atMtInfoCtx->bbicInfo.fwaveType);

    /* MT工位工具下发的单位是0.01dB,  GUC物理层使用的单位是0.1db,LTE和NR使用的是0.125dB(X / 10 * (4 / 5)) */
    if ((atMtInfoCtx->bbicInfo.currtRatMode == RAT_MODE_LTE) ||
        (atMtInfoCtx->bbicInfo.currtRatMode == RAT_MODE_NR)) {
        atMtInfoCtx->bbicInfo.fwavePower    = (VOS_INT16)(g_atParaList[AT_FWAVE_WAVEFORM_POWER].paraValue * 2 / 25);
    } else {
        /* GUC物理层使用的单位是0.1db,需要给功率大小除10 */
        atMtInfoCtx->bbicInfo.fwavePower = (VOS_INT16)(g_atParaList[AT_FWAVE_WAVEFORM_POWER].paraValue / 10);
    }

    /* 若是测试GSM的调制信号，则需要带第三个参数 */
    if ((atMtInfoCtx->bbicInfo.currtRatMode == RAT_MODE_GSM) &&
        (atMtInfoCtx->atInfo.faveType != AT_FWAVE_TYPE_CONTINUE)) {
        if (g_atParaList[AT_FWAVE_GSM_TX_SLOT_TYPE].paraValue >= AT_GSM_TX_SLOT_BUTT) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            atMtInfoCtx->atInfo.gsmTxSlotType =
                (AT_GsmTxSlotTypeUint8)g_atParaList[AT_FWAVE_GSM_TX_SLOT_TYPE].paraValue;
        }
    }

    return AT_OK;
}

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
VOS_UINT32 At_SetFChanWifiProc(VOS_UINT32 band)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* WIFI的第一个参数必须为8，第二个参数必须为15 */
    if (band != AT_BAND_WIFI) {
        return AT_FCHAN_WIFI_BAND_ERR;
    }

    /* WIFI 分支 */
    if (AT_IsMiscSupport(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) {
        /* WIFI未Enable直接返回失败 */
        if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
            return AT_WIFI_NOT_ENABLE;
        }

        atMtInfoCtx->atInfo.ratMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    }

    return AT_NOT_SUPPORT_WIFI;
}

VOS_UINT32 AT_ProcTSelRfWifiPara(VOS_VOID)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();
    if (AT_IsMiscSupport(BSP_MODULE_TYPE_WIFI) == BSP_MODULE_SUPPORT) {
        /* WIFI未Enable直接返回失败 */
        if ((VOS_UINT32)AT_GetWifiStatus() == VOS_FALSE) {
            return AT_WIFI_NOT_ENABLE;
        }

        atMtInfoCtx->atInfo.ratMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    } else {
        return AT_NOT_SUPPORT_WIFI;
    }
}
#endif

VOS_UINT32 At_CovertAtModeToBbicCal(AT_DEVICE_CmdRatModeUint8 atMode, RAT_MODE_ENUM_UINT16 *bbicMode)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (atMode) {
        case AT_RAT_MODE_WCDMA:
        case AT_RAT_MODE_AWS:
            *bbicMode = RAT_MODE_WCDMA;
            break;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case AT_RAT_MODE_CDMA:
            *bbicMode = RAT_MODE_CDMA;
            break;
#endif

        case AT_RAT_MODE_GSM:
        case AT_RAT_MODE_EDGE:
            *bbicMode = RAT_MODE_GSM;
            break;

#if (FEATURE_LTE == FEATURE_ON)
        case AT_RAT_MODE_FDD_LTE:
        case AT_RAT_MODE_TDD_LTE:
#if (FEATURE_LTEV == FEATURE_ON)
        case AT_RAT_MODE_LTEV:
#endif
            *bbicMode = RAT_MODE_LTE;
            break;
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case AT_RAT_MODE_NR:
            *bbicMode = RAT_MODE_NR;
            break;
#endif

        case AT_RAT_MODE_TDSCDMA: /* 目前带NR的芯片不支持TD */
        case AT_RAT_MODE_WIFI:
        default:
            *bbicMode = RAT_MODE_BUTT;
            result    = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_GetNrFreqOffset(VOS_UINT32 channelNo, AT_NR_FreqOffsetTable *nrFreqOffset)
{
    VOS_UINT32 indexNum;
    VOS_UINT32 freqOffsetNo;
    errno_t    memResult;

    freqOffsetNo = sizeof(g_atNrFreqOffsetTable) / sizeof(AT_NR_FreqOffsetTable);

    for (indexNum = 0; indexNum < freqOffsetNo; indexNum++) {
        if ((channelNo >= g_atNrFreqOffsetTable[indexNum].channelRange.channelMin) &&
            (channelNo <= g_atNrFreqOffsetTable[indexNum].channelRange.channelMax)) {
            break;
        }
    }

    if (indexNum == freqOffsetNo) {
        return VOS_FALSE;
    }

    memResult = memcpy_s(nrFreqOffset, sizeof(AT_NR_FreqOffsetTable), &g_atNrFreqOffsetTable[indexNum],
                         sizeof(g_atNrFreqOffsetTable[indexNum]));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(AT_NR_FreqOffsetTable), sizeof(g_atNrFreqOffsetTable[indexNum]));

    return VOS_TRUE;
}

VOS_VOID AT_GetNrFreqFromUlChannelNo(VOS_UINT32 ulChannelNo, AT_NR_FreqOffsetTable *nrFreqOffset,
                                     AT_DSP_BandFreq *dspBandFreq, const AT_NR_BandInfo *bandInfo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_INT32   detlFreq;
    VOS_UINT32  ulChanNoRange;
    VOS_UINT32  dlChanNoRange;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 上行频点 */
    dspBandFreq->ulFreq = nrFreqOffset->freqOffset +
                          nrFreqOffset->freqGlobal * (ulChannelNo - nrFreqOffset->offsetChannel);

    atMtInfoCtx->atInfo.bandArfcn.ulChanNo = ulChannelNo;

    ulChanNoRange = bandInfo->ulChannelRange.channelMax - bandInfo->ulChannelRange.channelMin;

    dlChanNoRange = bandInfo->dlChannelRange.channelMax - bandInfo->dlChannelRange.channelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算下行频率 */
    if (ulChanNoRange != dlChanNoRange) {
        return;
    }

    /* 上下行的频点差 */
    detlFreq = (VOS_INT32)(bandInfo->dlFreqRange.freqMin - bandInfo->ulFreqRange.freqMin);

    /* 下行频点 */
    dspBandFreq->dlFreq = dspBandFreq->ulFreq + detlFreq;

    atMtInfoCtx->atInfo.bandArfcn.dlChanNo = bandInfo->dlChannelRange.channelMin + ulChannelNo -
                                             bandInfo->ulChannelRange.channelMin;
}

VOS_VOID AT_GetNrFreqFromDlChannelNo(VOS_UINT32 dlChannelNo, AT_NR_FreqOffsetTable *nrFreqOffset,
                                     AT_DSP_BandFreq *dspBandFreq, const AT_NR_BandInfo *bandInfo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_INT32   detlFreq;
    VOS_UINT32  ulChanNoRange;
    VOS_UINT32  dlChanNoRange;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 下行频点 */
    dspBandFreq->dlFreq = nrFreqOffset->freqOffset +
                          nrFreqOffset->freqGlobal * (dlChannelNo - nrFreqOffset->offsetChannel);
    atMtInfoCtx->atInfo.bandArfcn.dlChanNo = dlChannelNo;

    ulChanNoRange = bandInfo->ulChannelRange.channelMax - bandInfo->ulChannelRange.channelMin;

    dlChanNoRange = bandInfo->dlChannelRange.channelMax - bandInfo->dlChannelRange.channelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算上行频率 */
    if (ulChanNoRange != dlChanNoRange) {
        return;
    }

    /* 相同信道上的频点差 */
    detlFreq = (VOS_INT32)(bandInfo->dlFreqRange.freqMin - bandInfo->ulFreqRange.freqMin);

    /* 上行频点 */
    dspBandFreq->ulFreq = dspBandFreq->dlFreq - detlFreq;

    atMtInfoCtx->atInfo.bandArfcn.ulChanNo = bandInfo->ulChannelRange.channelMin + dlChannelNo -
                                             bandInfo->dlChannelRange.channelMin;
}

VOS_UINT32 AT_GetNrFreq(VOS_UINT32 channelNo)
{
    AT_DSP_BandFreq      *dspBandFreq = VOS_NULL_PTR;
    AT_MT_Info           *atMtInfoCtx = VOS_NULL_PTR;
    AT_NR_FreqOffsetTable nrFreqOffset;
    VOS_UINT32            indexNum;
    VOS_UINT32            supportBandNo;

    atMtInfoCtx   = AT_GetMtInfoCtx();
    dspBandFreq   = &atMtInfoCtx->bbicInfo.dspBandFreq;
    supportBandNo = sizeof(g_atNrBandInfoTable) / sizeof(AT_NR_BandInfo);

    for (indexNum = 0; indexNum < supportBandNo; indexNum++) {
        if (g_atNrBandInfoTable[indexNum].bandNo == atMtInfoCtx->atInfo.bandArfcn.dspBand) {
            break;
        }
    }

    /* Band不匹配 */
    if (indexNum == supportBandNo) {
        return VOS_FALSE;
    }

    dspBandFreq->dspBand = atMtInfoCtx->atInfo.bandArfcn.dspBand;

    /* 获取频率的偏移量,单位KHZ */
    if (AT_GetNrFreqOffset(channelNo, &nrFreqOffset) == VOS_FALSE) {
        return VOS_FALSE;
    }

    /* 判断工具下发的是否是上行信道号 */
    if ((channelNo >= g_atNrBandInfoTable[indexNum].ulChannelRange.channelMin) &&
        (channelNo <= g_atNrBandInfoTable[indexNum].ulChannelRange.channelMax)) {
        AT_GetNrFreqFromUlChannelNo(channelNo, &nrFreqOffset, dspBandFreq, &g_atNrBandInfoTable[indexNum]);
    }
    /* 判断工具下发的是否是下行信道号 */
    else if ((channelNo >= g_atNrBandInfoTable[indexNum].dlChannelRange.channelMin) &&
             (channelNo <= g_atNrBandInfoTable[indexNum].dlChannelRange.channelMax)) {
        AT_GetNrFreqFromDlChannelNo(channelNo, &nrFreqOffset, dspBandFreq, &g_atNrBandInfoTable[indexNum]);
    } else {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_VOID AT_GetLteFreqFromUlChannelNo(VOS_UINT32 ulChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                      const AT_LTE_BandInfo *bandInfo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  dlChannelNo;
    VOS_UINT32  ulChanNoRange;
    VOS_UINT32  dlChanNoRange;

    atMtInfoCtx = AT_GetMtInfoCtx();

    dspBandFreq->ulFreq = bandInfo->ulLowFreq + ulChannelNo * LTE_CHANNEL_TO_FREQ_UNIT -
                          bandInfo->ulChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;
    atMtInfoCtx->atInfo.bandArfcn.ulChanNo = ulChannelNo;

    ulChanNoRange = bandInfo->ulChannelRange.channelMax - bandInfo->ulChannelRange.channelMin;

    dlChanNoRange = bandInfo->dlChannelRange.channelMax - bandInfo->dlChannelRange.channelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算下行频率 */
    if (ulChanNoRange != dlChanNoRange) {
        return;
    }

    dlChannelNo = bandInfo->dlChannelRange.channelMin + ulChannelNo - bandInfo->ulChannelOffset;

    dspBandFreq->dlFreq = bandInfo->dlLowFreq + dlChannelNo * LTE_CHANNEL_TO_FREQ_UNIT -
                          bandInfo->dlChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;

    atMtInfoCtx->atInfo.bandArfcn.dlChanNo = dlChannelNo;
}

VOS_VOID AT_GetLteFreqFromDlChannelNo(VOS_UINT32 dlChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                      const AT_LTE_BandInfo *bandInfo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  ulChanNoRange;
    VOS_UINT32  dlChanNoRange;
    VOS_UINT32  ulChannelNo;

    atMtInfoCtx = AT_GetMtInfoCtx();

    dspBandFreq->dlFreq = bandInfo->dlLowFreq + dlChannelNo * LTE_CHANNEL_TO_FREQ_UNIT -
                          bandInfo->dlChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;
    atMtInfoCtx->atInfo.bandArfcn.dlChanNo = dlChannelNo;

    ulChanNoRange = bandInfo->ulChannelRange.channelMax - bandInfo->ulChannelRange.channelMin;

    dlChanNoRange = bandInfo->dlChannelRange.channelMax - bandInfo->dlChannelRange.channelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算上行频率 */
    if (ulChanNoRange != dlChanNoRange) {
        return;
    }

    ulChannelNo = bandInfo->ulChannelRange.channelMin + dlChannelNo - bandInfo->dlChannelOffset;

    dspBandFreq->ulFreq = bandInfo->ulLowFreq + ulChannelNo * LTE_CHANNEL_TO_FREQ_UNIT -
                          bandInfo->ulChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;

    atMtInfoCtx->atInfo.bandArfcn.ulChanNo = ulChannelNo;
}

VOS_UINT32 AT_GetLteFreq(VOS_UINT32 channelNo)
{
    AT_DSP_BandFreq *dspBandFreq = VOS_NULL_PTR;
    AT_MT_Info      *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32       indexNum;
    VOS_UINT32       supportBandNo;
    VOS_UINT32       result;

    atMtInfoCtx   = AT_GetMtInfoCtx();
    result        = VOS_TRUE;
    dspBandFreq   = &atMtInfoCtx->bbicInfo.dspBandFreq;
    supportBandNo = sizeof(g_atLteBandInfoTable) / sizeof(AT_LTE_BandInfo);

    /* Band号查询 */
    for (indexNum = 0; indexNum < supportBandNo; indexNum++) {
        if (g_atLteBandInfoTable[indexNum].bandNo == atMtInfoCtx->atInfo.bandArfcn.dspBand) {
            break;
        }
    }

    if (indexNum == supportBandNo) {
        return VOS_FALSE;
    }

    dspBandFreq->dspBand = atMtInfoCtx->atInfo.bandArfcn.dspBand;

    /* 判断工具下发的是否是上行信道号 */
    if ((channelNo >= g_atLteBandInfoTable[indexNum].ulChannelRange.channelMin) &&
        (channelNo <= g_atLteBandInfoTable[indexNum].ulChannelRange.channelMax)) {
        AT_GetLteFreqFromUlChannelNo(channelNo, dspBandFreq, &g_atLteBandInfoTable[indexNum]);
    }
    /* 判断工具下发的是否是下行信道号 */
    else if ((channelNo >= g_atLteBandInfoTable[indexNum].dlChannelRange.channelMin) &&
             (channelNo <= g_atLteBandInfoTable[indexNum].dlChannelRange.channelMax)) {
        AT_GetLteFreqFromDlChannelNo(channelNo, dspBandFreq, &g_atLteBandInfoTable[indexNum]);
    } else {
        result = VOS_FALSE;
    }

    return result;
}

VOS_UINT32 AT_GetWFreqFromUlChannelNo(VOS_UINT32 ulChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                      const AT_W_BandInfo *bandInfo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    dspBandFreq->ulFreq = ulChannelNo * FREQ_UNIT_MHZ_TO_KHZ / W_CHANNEL_FREQ_TIMES + bandInfo->ulFreqOffset;

    dspBandFreq->dlFreq = dspBandFreq->ulFreq + bandInfo->dlFreqRange.freqMin - bandInfo->ulFreqRange.freqMin;

    atMtInfoCtx->atInfo.bandArfcn.ulChanNo = ulChannelNo;
    atMtInfoCtx->atInfo.bandArfcn.dlChanNo = bandInfo->dlChannelRange.channelMin + ulChannelNo -
                                             bandInfo->ulChannelRange.channelMin;

    return VOS_TRUE;
}

VOS_UINT32 AT_GetWFreqFromDlChannelNo(VOS_UINT32 dlChannelNo, AT_DSP_BandFreq *dspBandFreq,
                                      const AT_W_BandInfo *bandInfo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    dspBandFreq->dlFreq = dlChannelNo * FREQ_UNIT_MHZ_TO_KHZ / W_CHANNEL_FREQ_TIMES + bandInfo->dlFreqOffset;

    dspBandFreq->ulFreq = dspBandFreq->dlFreq + bandInfo->ulFreqRange.freqMin - bandInfo->dlFreqRange.freqMin;

    atMtInfoCtx->atInfo.bandArfcn.dlChanNo = dlChannelNo;
    atMtInfoCtx->atInfo.bandArfcn.ulChanNo = bandInfo->ulChannelRange.channelMin + dlChannelNo -
                                             bandInfo->dlChannelRange.channelMin;

    return VOS_TRUE;
}

VOS_UINT32 AT_GetWFreq(VOS_UINT32 channelNo)
{
    AT_DSP_BandFreq *dspBandFreq = VOS_NULL_PTR;
    AT_MT_Info      *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32       indexNum;
    VOS_UINT32       supportBandNo;
    VOS_UINT32       result;

    atMtInfoCtx   = AT_GetMtInfoCtx();
    result        = VOS_TRUE;
    dspBandFreq   = &atMtInfoCtx->bbicInfo.dspBandFreq;
    supportBandNo = sizeof(g_atWBandInfoTable) / sizeof(AT_W_BandInfo);

    /*  判断当前band是否是支持的band */
    for (indexNum = 0; indexNum < supportBandNo; indexNum++) {
        if (atMtInfoCtx->atInfo.bandArfcn.dspBand == g_atWBandInfoTable[indexNum].bandNo) {
            break;
        }
    }

    if (indexNum == supportBandNo) {
        return VOS_FALSE;
    }

    dspBandFreq->dspBand = atMtInfoCtx->atInfo.bandArfcn.dspBand;

    /* 判断工具下发的是否是上行信道号 */
    if ((channelNo >= g_atWBandInfoTable[indexNum].ulChannelRange.channelMin) &&
        (channelNo <= g_atWBandInfoTable[indexNum].ulChannelRange.channelMax)) {
        AT_GetWFreqFromUlChannelNo(channelNo, dspBandFreq, &g_atWBandInfoTable[indexNum]);
    }
    /* 判断工具下发的是否是下行信道号 */
    else if ((channelNo >= g_atWBandInfoTable[indexNum].dlChannelRange.channelMin) &&
             (channelNo <= g_atWBandInfoTable[indexNum].dlChannelRange.channelMax)) {
        AT_GetWFreqFromDlChannelNo(channelNo, dspBandFreq, &g_atWBandInfoTable[indexNum]);
    } else {
        result = VOS_FALSE;
    }

    return result;
}

VOS_UINT32 AT_GetGFreq(VOS_UINT32 channelNo)
{
    AT_DSP_BandFreq *dspBandFreq = VOS_NULL_PTR;
    AT_MT_Info      *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32       indexNum;
    VOS_UINT32       supportBandNo;

    atMtInfoCtx = AT_GetMtInfoCtx();
    dspBandFreq = &atMtInfoCtx->bbicInfo.dspBandFreq;

    supportBandNo = AT_GET_GSM_BAND_INFO_TAB_LEN();

    /*  判断当前band是否是支持的band */
    for (indexNum = 0; indexNum < supportBandNo; indexNum++) {
        if (atMtInfoCtx->atInfo.bandArfcn.dspBand == g_atGBandInfoTable[indexNum].bandNo) {
            break;
        }
    }

    if (indexNum == supportBandNo) {
        return VOS_FALSE;
    }

    dspBandFreq->dspBand = atMtInfoCtx->atInfo.bandArfcn.dspBand;

    /*  判断信道是否在范围内 */
    if ((channelNo < g_atGBandInfoTable[indexNum].channelRange.channelMin) ||
        (channelNo > g_atGBandInfoTable[indexNum].channelRange.channelMax)) {
        return VOS_FALSE;
    }

    /* GSM 900的信道范围比较多 */
    if (dspBandFreq->dspBand == AT_PROTOCOL_BAND_8) {
        /* Fu等于890000 + 200*N */
        if (channelNo <= G_CHANNEL_NO_124) {
            dspBandFreq->ulFreq = 890000 + 200 * channelNo;
            dspBandFreq->dlFreq = dspBandFreq->ulFreq + g_atGBandInfoTable[indexNum].freqOffset;

            atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
            atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;

            return VOS_TRUE;
        }

        /* Fu等于890000 + 200*(N - 1024) */
        if ((channelNo >= G_CHANNEL_NO_955) && (channelNo <= G_CHANNEL_NO_1023)) {
            dspBandFreq->ulFreq = 890000 + 200 * channelNo - 200 * 1024;
            dspBandFreq->dlFreq = dspBandFreq->ulFreq + g_atGBandInfoTable[indexNum].freqOffset;

            atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
            atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;

            return VOS_TRUE;
        }
    }

    /* GSM 1800的信道范围比较多 */
    if (dspBandFreq->dspBand == AT_PROTOCOL_BAND_3) {
        /* Fu等于1710200 + 200*(N - 512) */
        dspBandFreq->ulFreq = 1710200 + 200 * channelNo - 200 * 512;
        dspBandFreq->dlFreq = dspBandFreq->ulFreq + g_atGBandInfoTable[indexNum].freqOffset;

        atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
        atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;
        return VOS_TRUE;
    }

    /* GSM 1900的信道范围比较多 */
    if (dspBandFreq->dspBand == AT_PROTOCOL_BAND_2) {
        /* Fu等于1850200 + 200*(N - 512) */
        dspBandFreq->ulFreq                    = 1850200 + 200 * channelNo - 200 * 512;
        dspBandFreq->dlFreq                    = dspBandFreq->ulFreq + g_atGBandInfoTable[indexNum].freqOffset;
        atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
        atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;
        return VOS_TRUE;
    }

    /* GSM 850的信道范围比较多 */
    if (dspBandFreq->dspBand == AT_PROTOCOL_BAND_5) {
        /* Fu等于824200 + 200*(N - 128) */
        dspBandFreq->ulFreq                    = 824200 + 200 * channelNo - 200 * 128;
        dspBandFreq->dlFreq                    = dspBandFreq->ulFreq + g_atGBandInfoTable[indexNum].freqOffset;
        atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
        atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_GetCFreq(VOS_UINT32 channelNo)
{
    AT_DSP_BandFreq *dspBandFreq = VOS_NULL_PTR;
    AT_MT_Info      *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();
    dspBandFreq = &atMtInfoCtx->bbicInfo.dspBandFreq;

    /* 当前只支持Band Class 0,Band5 */
    if (atMtInfoCtx->atInfo.bandArfcn.dspBand != AT_PROTOCOL_BAND_5) {
        return VOS_FALSE;
    }

    dspBandFreq->dspBand = atMtInfoCtx->atInfo.bandArfcn.dspBand;

    if ((channelNo >= C_CHANNEL_NO_1) && (channelNo <= C_CHANNEL_NO_799)) {
        /* 计算公式: Fu = 30N + 825000 */
        dspBandFreq->ulFreq = 30 * channelNo + 825000;

        /* 计算公式: Fd = 30N + 870000 */
        dspBandFreq->dlFreq = 30 * channelNo + 870000;

        atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
        atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;

        return VOS_TRUE;
    }

    if ((channelNo >= C_CHANNEL_NO_991) && (channelNo <= C_CHANNEL_NO_1023)) {
        /* 计算公式: Fu = 30(N - 1023) + 825000 */
        dspBandFreq->ulFreq = 825000 - 30 * (1023 - channelNo);

        /* 计算公式: Fd = 30(N -1023) + 870000 */
        dspBandFreq->dlFreq = 870000 - 30 * (1023 - channelNo);

        atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
        atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;

        return VOS_TRUE;
    }

    if ((channelNo >= C_CHANNEL_NO_1024) && (channelNo <= C_CHANNEL_NO_1323)) {
        /* 计算公式: Fu = 30(N - 1024) + 815040 */
        dspBandFreq->ulFreq = 30 * (channelNo - 1024) + 815040;

        /* 计算公式: Fd = 30(N -1024) + 860040 */
        dspBandFreq->dlFreq = 30 * (channelNo - 1024) + 860040;

        atMtInfoCtx->atInfo.bandArfcn.dlChanNo = channelNo;
        atMtInfoCtx->atInfo.bandArfcn.ulChanNo = channelNo;

        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_GetFreq(VOS_UINT32 channelNo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;

    atMtInfoCtx = AT_GetMtInfoCtx();
    result      = VOS_FALSE;

    switch (atMtInfoCtx->bbicInfo.currtRatMode) {
        case RAT_MODE_GSM:
            result = AT_GetGFreq(channelNo);
            break;

        case RAT_MODE_WCDMA:
            result = AT_GetWFreq(channelNo);
            break;
        case RAT_MODE_CDMA:
            result = AT_GetCFreq(channelNo);
            break;

        case RAT_MODE_LTE:
            result = AT_GetLteFreq(channelNo);
            break;

        case RAT_MODE_NR:
            result = AT_GetNrFreq(channelNo);
            break;

        default:
            result = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 At_CovertAtBandWidthToBbicCal(AT_BAND_WidthUint16 atBandWidth, BANDWIDTH_ENUM_UINT16 *bbicBandWidth)
{
    VOS_UINT32 indexNum;
    VOS_UINT32 bandWidthNo;

    bandWidthNo    = sizeof(g_bandWidthTable) / sizeof(g_bandWidthTable[0]);
    *bbicBandWidth = BANDWIDTH_BUTT;

    for (indexNum = 0; indexNum < bandWidthNo; indexNum++) {
        if (atBandWidth == g_bandWidthTable[indexNum].atBandWidth) {
            *bbicBandWidth = g_bandWidthTable[indexNum].bbicBandWidth;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 At_CovertAtBandWidthToValue(AT_BAND_WidthUint16 atBandWidth, AT_BAND_WidthValueUint32 *bandWidthValue)
{
    VOS_UINT32 indexNum;
    VOS_UINT32 bandWidthNo;

    bandWidthNo     = sizeof(g_bandWidthTable) / sizeof(g_bandWidthTable[0]);
    *bandWidthValue = AT_BAND_WIDTH_VALUE_BUTT;

    for (indexNum = 0; indexNum < bandWidthNo; indexNum++) {
        if (atBandWidth == g_bandWidthTable[indexNum].atBandWidth) {
            *bandWidthValue = g_bandWidthTable[indexNum].atBandWidthValue;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 At_CovertAtScsToBbicCal(AT_SubCarrierSpacingUint8 atScs, NR_SCS_TYPE_COMM_ENUM_UINT8 *bbicScs)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (atScs) {
        case AT_SUB_CARRIER_SPACING_15K:
            *bbicScs = NR_SCS_TYPE_COMM_15;
            break;
        case AT_SUB_CARRIER_SPACING_30K:
            *bbicScs = NR_SCS_TYPE_COMM_30;
            break;
        case AT_SUB_CARRIER_SPACING_60K:
            *bbicScs = NR_SCS_TYPE_COMM_60;
            break;
        case AT_SUB_CARRIER_SPACING_120K:
            *bbicScs = NR_SCS_TYPE_COMM_120;
            break;
        case AT_SUB_CARRIER_SPACING_240K:
            *bbicScs = NR_SCS_TYPE_COMM_240;
            break;
        default:
            *bbicScs = NR_SCS_TYPE_COMM_BUTT;
            result   = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 At_LoadPhy(VOS_VOID)
{
    AT_CCBT_LOAD_PHY_REQ_STRU *loadPhyReq  = VOS_NULL_PTR;
    AT_MT_Info                *atMtInfoCtx = VOS_NULL_PTR;

    /* 分配消息空间 */
    loadPhyReq = (AT_CCBT_LOAD_PHY_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                           sizeof(AT_CCBT_LOAD_PHY_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (loadPhyReq == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(loadPhyReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(loadPhyReq, CCPU_PID_CBT, ID_AT_CCBT_SET_WORK_MODE_REQ);

    /* 填写消息内容 */
#if (FEATURE_LTEV == FEATURE_ON)
    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_LTEV) {
        loadPhyReq->stComponentId.ucComponentType = AT_COMPONENTTYPE_RAT_LTEV;
    }
#endif
    loadPhyReq->stLoadphyPara.aenRatMode[0] = atMtInfoCtx->bbicInfo.currtRatMode;
    loadPhyReq->stLoadphyPara.enBusiness    = CBT_BUSINESS_TYPE_MT;

    loadPhyReq->stLoadphyPara.nosigType = CBT_NOSIG_TYPE_LF;
    if (AT_GetMtInfoCtx()->bbicInfo.dspBandFreq.dspBand >= AT_PROTOCOL_BAND_256) {
        loadPhyReq->stLoadphyPara.nosigType = CBT_NOSIG_TYPE_HF;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, loadPhyReq) != VOS_OK) {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_LoadPhy Exit");

    return VOS_TRUE;
}

VOS_UINT32 At_SetFChanPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetFChanPara Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 这条命令是每个band测试的第一条命令，将除当前运行模式外的信息清除 */
    (VOS_VOID)memset_s(&atMtInfoCtx->atInfo, sizeof(atMtInfoCtx->atInfo), 0, sizeof(AT_MT_AtInfo));
    (VOS_VOID)memset_s(&atMtInfoCtx->bbicInfo, sizeof(atMtInfoCtx->bbicInfo), 0, sizeof(AT_MT_BbicInfo));

    /* WIFI模式 */
    if (g_atParaList[AT_FCHAN_PARA_MODE].paraValue == AT_RAT_MODE_WIFI) {
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
        return At_SetFChanWifiProc(g_atParaList[AT_FCHAN_PARA_BAND].paraValue);
#else
        return AT_ERROR;
#endif
    }

    /* 当前模式判断 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /*
     * 参数不符合要求,支持NR的版本中，一定要配置Band width,
     * 目前参数包括必带参数模式，Band, 信道号和带宽和一个可选参数子载波间隔
     */
    if (g_atParaIndex < AT_FCHAN_NR_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_FCHAN_PARA_BAND].paraValue >= AT_PROTOCOL_BAND_BUTT) {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    /* 记录band信息, 要求工具下发就按协议band号下发 */
    atMtInfoCtx->atInfo.bandArfcn.dspBand = (AT_PROTOCOL_BandUint16)g_atParaList[AT_FCHAN_PARA_BAND].paraValue;

    /* 记录测试接入技术模式信息 */
    if (At_CovertAtModeToBbicCal((AT_DEVICE_CmdRatModeUint8)g_atParaList[AT_FCHAN_PARA_MODE].paraValue,
                                 &atMtInfoCtx->bbicInfo.currtRatMode) == VOS_FALSE) {
        return AT_FCHAN_RAT_ERR;
    }

    atMtInfoCtx->atInfo.ratMode = (AT_DEVICE_CmdRatModeUint8)g_atParaList[AT_FCHAN_PARA_MODE].paraValue;

    /* 获取频率信息 */
    if (AT_GetFreq(g_atParaList[AT_FCHAN_PARA_CHANNEL_NO].paraValue) == VOS_FALSE) {
        return AT_DEVICE_CHAN_BAND_CHAN_NOT_MAP;
    }

    /* 记录带宽信息 */
    if (At_CovertAtBandWidthToBbicCal((AT_BAND_WidthUint16)g_atParaList[AT_FCHAN_PARA_UL_CHANNEL].paraValue,
                                      &atMtInfoCtx->bbicInfo.bandWidth) == VOS_FALSE) {
        return AT_FCHAN_BAND_WIDTH_ERR;
    }

    atMtInfoCtx->atInfo.bandWidth = (AT_BAND_WidthUint16)g_atParaList[AT_FCHAN_PARA_UL_CHANNEL].paraValue;

    /* 将AT的带宽转换成真正的值记录下来 */
    At_CovertAtBandWidthToValue((AT_BAND_WidthUint16)g_atParaList[AT_FCHAN_PARA_UL_CHANNEL].paraValue,
                                &atMtInfoCtx->bbicInfo.bandWidthValue);

    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_NR) {
        if (g_atParaList[AT_FCHAN_PARA_DL_CHANNEL].paraLen == 0) {
            return AT_FCHAN_NO_SCS;
        }

        if (At_CovertAtScsToBbicCal((AT_SubCarrierSpacingUint8)g_atParaList[AT_FCHAN_PARA_DL_CHANNEL].paraValue,
                                    (VOS_UINT8 *)&atMtInfoCtx->bbicInfo.bbicScs) == VOS_FALSE) {
            return AT_FCHAN_SCS_ERR;
        }

        atMtInfoCtx->atInfo.bbicScs = (AT_SubCarrierSpacingUint8)g_atParaList[AT_FCHAN_PARA_DL_CHANNEL].paraValue;
    }

    /* 记录当前client */
    atMtInfoCtx->atInfo.indexNum = indexNum;

    /* 给CBT发消息，通知其加载PHY */
    if (At_LoadPhy() == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FCHAN_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_SND_MSG_FAIL;
}

VOS_UINT32 AT_CoverTselPathToPriOrDiv(AT_TSELRF_PathUint32 tselPath, AT_AntTypeUint8 *antType)
{
    VOS_UINT32 pathNo;
    VOS_UINT32 indexNum;

    pathNo = sizeof(g_path2AntTypeTable) / sizeof(AT_PATH_ToAntType);

    for (indexNum = 0; indexNum < pathNo; indexNum++) {
        if (tselPath == g_path2AntTypeTable[indexNum].tselPath) {
            *antType = g_path2AntTypeTable[indexNum].antType;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_CheckMimoPara(VOS_VOID)
{
    if (g_atParaList[AT_TSELRF_TX_OR_RX].paraValue == 0) {
        /* 双TX同时测试时，天线group参数传3 */
        if ((g_atParaList[AT_TSELRF_PARA_TYPE].paraValue != AT_MIMO_TYPE_2) ||
            (g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraValue > AT_MIMO_ANT_NUM_3)) {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO ERR");
            return AT_FAILURE;
        }

        return AT_SUCCESS;
    } else {
        if ((g_atParaList[AT_TSELRF_PARA_TYPE].paraValue == AT_MIMO_TYPE_2) &&
            (g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraValue > AT_MIMO_ANT_NUM_2)) {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO_2 ERR");
            return AT_FAILURE;
        }

        if ((g_atParaList[AT_TSELRF_PARA_TYPE].paraValue == AT_MIMO_TYPE_4) &&
            (g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraValue > AT_MIMO_ANT_NUM_4)) {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO_4 ERR");
            return AT_FAILURE;
        }

        if ((g_atParaList[AT_TSELRF_PARA_TYPE].paraValue == AT_MIMO_TYPE_8) &&
            (g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraValue > AT_MIMO_ANT_NUM_8)) {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO_8 ERR");
            return AT_FAILURE;
        }

        return AT_SUCCESS;
    }
}

VOS_UINT32 AT_ProcTSelRfPara(AT_AntTypeUint8 antType)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    if (g_atParaList[AT_TSELRF_TX_OR_RX].paraValue == 0) {
        /* 初始化 */
        atMtInfoCtx->atInfo.txMimoType     = AT_MIMO_TYPE_BUTT;
        atMtInfoCtx->atInfo.txMimoAntNum   = AT_MIMO_ANT_NUM_BUTT;
        atMtInfoCtx->bbicInfo.txMimoType   = 0xFF;
        atMtInfoCtx->bbicInfo.txMimoAntNum = 0xFF;

        if (antType == AT_ANT_TYPE_MIMO) {
            if (AT_CheckMimoPara() == AT_FAILURE) {
                return AT_MIMO_PARA_ERR;
            }

            atMtInfoCtx->atInfo.txMimoType   = (AT_MIMO_TYPE_UINT8)g_atParaList[AT_TSELRF_PARA_TYPE].paraValue;
            atMtInfoCtx->atInfo.txMimoAntNum = (AT_MIMO_AntNumUint8 )g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraValue;
            /* BBIC采用BIT表示 */
            atMtInfoCtx->bbicInfo.txMimoType   = (VOS_UINT16)AT_SET_BIT32(atMtInfoCtx->atInfo.txMimoType);
            atMtInfoCtx->bbicInfo.txMimoAntNum = (VOS_UINT16)AT_SET_BIT32(atMtInfoCtx->atInfo.txMimoAntNum - 1);

            /* 双TX测试 */
            if (atMtInfoCtx->atInfo.txMimoAntNum == AT_MIMO_ANT_NUM_3) {
                atMtInfoCtx->bbicInfo.txMimoAntNum = AT_MIMO_ANT_NUM_3;
            }
        }

        atMtInfoCtx->bbicInfo.txAntType     = antType;
        atMtInfoCtx->atInfo.tseLrfTxPath    = g_atParaList[AT_TSELRF_TX_OR_RX_PATH].paraValue;
        atMtInfoCtx->atInfo.setTxTselrfFlag = VOS_TRUE;
    } else if (g_atParaList[AT_TSELRF_TX_OR_RX].paraValue == 1) {
        /* 初始化 */
        atMtInfoCtx->atInfo.rxMimoType     = AT_MIMO_TYPE_BUTT;
        atMtInfoCtx->atInfo.rxMimoAntNum   = AT_MIMO_ANT_NUM_BUTT;
        atMtInfoCtx->bbicInfo.rxMimoType   = 0xFF;
        atMtInfoCtx->bbicInfo.rxMimoAntNum = 0xFF;

        if (antType == AT_ANT_TYPE_MIMO) {
            if (AT_CheckMimoPara() == AT_FAILURE) {
                return AT_MIMO_PARA_ERR;
            }

            atMtInfoCtx->atInfo.rxMimoType   = (AT_MIMO_TYPE_UINT8)g_atParaList[AT_TSELRF_PARA_TYPE].paraValue;
            atMtInfoCtx->atInfo.rxMimoAntNum = (AT_MIMO_AntNumUint8 )g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraValue;
            /* BBIC采用BIT表示 */
            atMtInfoCtx->bbicInfo.rxMimoType   = (VOS_UINT16)AT_SET_BIT32(atMtInfoCtx->atInfo.rxMimoType);
            atMtInfoCtx->bbicInfo.rxMimoAntNum = (VOS_UINT16)AT_SET_BIT32(atMtInfoCtx->atInfo.rxMimoAntNum - 1);
        }

        atMtInfoCtx->bbicInfo.rxAntType     = antType;
        atMtInfoCtx->atInfo.tseLrfRxPath    = g_atParaList[AT_TSELRF_TX_OR_RX_PATH].paraValue;
        atMtInfoCtx->atInfo.setRxTselrfFlag = VOS_TRUE;
    } else {
        return AT_ESELRF_TX_OR_RX_ERR;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetTSelRfPara(VOS_UINT8 indexNum)
{
    AT_MT_Info     *atMtInfoCtx = VOS_NULL_PTR;
    AT_AntTypeUint8 antType;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("AT_SetTSelRfPara Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_TSELRF_TX_OR_RX_PATH].paraValue >= AT_TSELRF_PATH_BUTT) {
        return AT_TSELRF_PATH_ERR;
    }

    if (g_atParaList[AT_TSELRF_TX_OR_RX_PATH].paraValue == AT_TSELRF_PATH_WIFI) {
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
        return AT_ProcTSelRfWifiPara();
#else
        return AT_ERROR;
#endif
    }

    /* 参数不符合要求,必须有两个参数 */
    if (g_atParaIndex < AT_TSELRF_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    if (AT_CoverTselPathToPriOrDiv(g_atParaList[AT_TSELRF_TX_OR_RX_PATH].paraValue, &antType) != VOS_TRUE) {
        return AT_TSELRF_PATH_ERR;
    }

    /* 检查MIMO的参数是否带下来 */
    if ((antType == AT_ANT_TYPE_MIMO) &&
        ((g_atParaList[AT_TSELRF_PARA_TYPE].paraLen == 0) || (g_atParaList[AT_TSELRF_PARA_ANT_NUM].paraLen == 0))) {
        return AT_MIMO_PARA_ERR;
    }

    return AT_ProcTSelRfPara(antType);
}

VOS_UINT32 At_SetFTxonPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetFTxonPara Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_FTXON_OTHER_ERR;
    }
    /* 参数不符合要求 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue >= AT_DSP_RF_SWITCH_BUTT) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 当前是否是FTM模式 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 检查是否加载了DSP */
    if (atMtInfoCtx->atInfo.dspLoadFlag == VOS_FALSE) {
        return AT_NOT_LOAD_DSP;
    }

    /* 检查是否设置了path */
    if (atMtInfoCtx->atInfo.setTxTselrfFlag == VOS_FALSE) {
        return AT_NOT_SET_PATH;
    }

    /* 记录全局变量 */
    atMtInfoCtx->atInfo.tempRxorTxOnOff = (AT_DSP_RfOnOffUint8)g_atParaList[0].paraValue;

    /* 给BBIC发消息 */
    if ((atMtInfoCtx->bbicInfo.currtRatMode == RAT_MODE_GSM) &&
        (atMtInfoCtx->atInfo.faveType != AT_FWAVE_TYPE_CONTINUE)) {
        if (At_SndGsmTxOnOffReq_ModulatedWave(VOS_FALSE) != VOS_TRUE) {
            return AT_SND_MSG_FAIL;
        }
    } else {
        if (At_SndTxOnOffReq(VOS_FALSE) != VOS_TRUE) {
            return AT_SND_MSG_FAIL;
        }
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SET_FTXON;
    atMtInfoCtx->atInfo.indexNum          = indexNum;

    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 At_SetDpdtPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  rst;

    atMtInfoCtx = AT_GetMtInfoCtx();
    rst         = AT_ERROR;

    AT_PR_LOGH("At_SetDpdtPara Enter");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 当前不为非信令模式 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != AT_DPDT_TO_BBIC_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 记录测试接入技术模式信息 */
    if (At_CovertRatModeToBbicCal((AT_CmdRatmodeUint8)g_atParaList[AT_DPDT_RAT_MODE].paraValue,
                                   &atMtInfoCtx->bbicInfo.dpdtRatMode) == VOS_FALSE) {
        return AT_DPDT_RAT_ERR;
    }

    /* AT发送DPDT设置消息至BBIC处理 */

    rst = AT_SndBbicCalSetDpdtReq(BBIC_DPDT_OPERTYPE_SET, g_atParaList[AT_DPDT_VALUE].paraValue,
                                  g_atParaList[AT_DPDT_WORK_TYPE].paraValue);

    if (rst == AT_SUCCESS) {
        atMtInfoCtx->atInfo.indexNum          = indexNum;
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  rst;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetQryDpdtPara Enter");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 当前不为非信令模式 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != AT_DPDTQRY_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 记录测试接入技术模式信息 */
    if (At_CovertRatModeToBbicCal((AT_CmdRatmodeUint8)g_atParaList[AT_DPDT_QRY_RAT_MODE].paraValue,
                                   &atMtInfoCtx->bbicInfo.dpdtRatMode) == VOS_FALSE) {
        return AT_DPDT_RAT_ERR;
    }

    /* AT发送DPDT查询消息至BBIC处理 */
    rst = AT_SndBbicCalSetDpdtReq(BBIC_DPDT_OPERTYPE_GET, 0, g_atParaList[AT_DPDT_QRY_WORK_TYPE].paraValue);

    if (rst == AT_SUCCESS) {
        atMtInfoCtx->atInfo.indexNum          = indexNum;
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DPDTQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SndUeCbtRfIcMemTestReq(VOS_VOID)
{
    AT_UECBT_RficMemTestReq *testReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    testReq = (AT_UECBT_RficMemTestReq *)PS_ALLOC_MSG(WUEPS_PID_AT,
        sizeof(AT_UECBT_RficMemTestReq) - VOS_MSG_HEAD_LENGTH);
    if (testReq == VOS_NULL_PTR) {
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(testReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(testReq, CCPU_PID_PAM_MFG, ID_AT_UECBT_RFIC_MEM_TEST_REQ);

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, testReq) != VOS_OK) {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndCbtRfIcMemTestNtf Exit");

    return AT_SUCCESS;
}

VOS_UINT32 At_SetRfIcMemTest(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetRfIcMemTest Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atMtInfoCtx->atInfo.rficTestResult == AT_RFIC_MEM_TEST_RUNNING) {
        return AT_ERROR;
    }

    if (atMtInfoCtx->atInfo.dspLoadFlag != VOS_TRUE) {
        return AT_NOT_LOAD_DSP;
    }

    /* 初始化全局变量 */
    atMtInfoCtx->atInfo.rficTestResult = AT_RFIC_MEM_TEST_NOT_START;

    /* 直接给BBIC发送消息 */
    result = At_SndUeCbtRfIcMemTestReq();
    if (result == AT_SUCCESS) {
        atMtInfoCtx->atInfo.rficTestResult = AT_RFIC_MEM_TEST_RUNNING;
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SndDspIdleSerdesRtReq(VOS_VOID)
{
    FESS_FLPM_NasSerdesAgingReq *testReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    testReq = (FESS_FLPM_NasSerdesAgingReq *)PS_ALLOC_MSG(WUEPS_PID_AT,
        sizeof(FESS_FLPM_NasSerdesAgingReq) - VOS_MSG_HEAD_LENGTH);
    if (testReq == VOS_NULL_PTR) {
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(testReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(testReq, I0_PHY_PID_IDLE, ID_FESS_NAS_FLPM_SERDES_AGING_REQ);

    testReq->testNum     = g_atParaList[AT_FSERDESRT_TEST_NUM_INDEX].paraValue;
    testReq->syncLen     = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_SYNC_LEN_INDEX].paraValue);
    testReq->burstTime   = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_BURST_TIME_INDEX].paraValue);
    testReq->stallTime   = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_STALL_TIME_INDEX].paraValue);
    testReq->dlSyncLen   = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_DL_SYNC_LEN_INDEX].paraValue);
    testReq->dlBurstTime = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_DL_BURST_TIME_INDEX].paraValue);
    testReq->dlStallTime = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_DL_STALL_TIME_INDEX].paraValue);

    if (g_atParaList[AT_FSERDESRT_LS_TEST_INDEX].paraLen != 0) {
        testReq->lsTest = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_LS_TEST_INDEX].paraValue);
    }

    if (g_atParaList[AT_FSERDESRT_FWD_CLK_AMP_INDEX].paraLen != 0) {
        testReq->fwdClkAmp = (VOS_UINT16)(g_atParaList[AT_FSERDESRT_FWD_CLK_AMP_INDEX].paraValue);
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, testReq) != VOS_OK) {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndPhySerDesrtTestReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 At_SetFSerdesRt(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetFSerdesRt Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atMtInfoCtx->rserTestResult == (VOS_INT32)AT_SERDES_TEST_RUNNING) {
        AT_PR_LOGE("At_SetFSerdesRt:The Running");
        return AT_ERROR;
    }

    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        AT_PR_LOGE("At_SetFSerdesRt:Model Error");
        return AT_DEVICE_MODE_ERROR;
    }

    if (g_atParaIndex < AT_FSERDESRT_PARA_NUM) {
        AT_PR_LOGE("At_SetFSerdesRt:Parameter Error");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    atMtInfoCtx->rserTestResult = (VOS_INT32)AT_SERDES_TEST_NOT_START;

    /* 直接给BBIC发送消息 */
    result = At_SndDspIdleSerdesRtReq();
    if (result == AT_SUCCESS) {
        atMtInfoCtx->rserTestResult = (VOS_INT32)AT_SERDES_TEST_RUNNING;
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SndDspIdleSerdesTestAsyncReq(VOS_VOID)
{
    FESS_FLPM_NasAtNobizReq *testReq     = VOS_NULL_PTR;
    AT_MT_Info              *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32               i;
    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 分配消息空间 */
    testReq = (FESS_FLPM_NasAtNobizReq *)PS_ALLOC_MSG(WUEPS_PID_AT,
        sizeof(FESS_FLPM_NasAtNobizReq) - VOS_MSG_HEAD_LENGTH);
    if (testReq == VOS_NULL_PTR) {
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(testReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(testReq, I0_PHY_PID_IDLE, ID_FESS_NAS_FLPM_AT_NOBIZ_REQ);

    testReq->type = atMtInfoCtx->serdesTestAsyncInfo.cmdType;
    for (i = 0; i < AT_SERDES_PARA_MAX_CNT; i++) {
        testReq->para[i] = g_atParaList[i + AT_PHY_SERDES_TEST_PARALIST_PARA1].paraValue;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, testReq) != VOS_OK) {
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndDspIdleSerdesTestAsyncReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 At_SetSerdesAsyncTest(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetSerdesAsyncTest Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex < AT_SERDESTESTASYNC_PARA_MIN_NUM || g_atParaIndex > AT_SERDESTESTASYNC_PARA_MAX_NUM) {
        AT_PR_LOGE("At_SetSerdesAsyncTest:Parameter Error");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_SERDESTESTASYNC_PARA_MODE].paraValue == 0) {
        /* 设置命令 */
        if (g_atParaList[AT_SERDESTESTASYNC_PARA_CMDTYPE].paraValue == 0) {
            /* 主动上报参数设置 */
            if (g_atParaList[AT_SERDESTESTASYNC_PARA_RPTFLG].paraLen == 0) {
                return AT_CME_INCORRECT_PARAMETERS;
            } else {
                atMtInfoCtx->serdesTestAsyncInfo.rptFlg = g_atParaList[AT_SERDESTESTASYNC_PARA_RPTFLG].paraValue;
                atMtInfoCtx->serdesTestAsyncInfo.result = (VOS_INT16)AT_SERDES_TEST_RST_NOT_START;
            }
        } else {
            /* 设置命令参数设置 */
            if (atMtInfoCtx->serdesTestAsyncInfo.result == (VOS_INT16)AT_SERDES_TEST_RST_RUNNING) {
                AT_PR_LOGE("At_SetSerdesAsyncTest:The Running");
                return AT_ERROR;
            }
            atMtInfoCtx->serdesTestAsyncInfo.result  = (VOS_INT16)AT_SERDES_TEST_RST_NOT_START;
            atMtInfoCtx->serdesTestAsyncInfo.cmdType =
                (VOS_UINT8)(g_atParaList[AT_SERDESTESTASYNC_PARA_CMDTYPE].paraValue);
        }

        result = At_SndDspIdleSerdesTestAsyncReq();
        if (result == AT_SUCCESS) {
            atMtInfoCtx->serdesTestAsyncInfo.result = (VOS_INT16)AT_SERDES_TEST_RST_RUNNING;
            return AT_OK;
        } else {
            return AT_ERROR;
        }
    } else {
        /* 查询命令 */
        if (atMtInfoCtx->serdesTestAsyncInfo.cmdType ==
            (VOS_INT8)g_atParaList[AT_SERDESTESTASYNC_PARA_CMDTYPE].paraValue) {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                atMtInfoCtx->serdesTestAsyncInfo.result);
        } else {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                AT_SERDES_TEST_RST_NOT_MATCH);
        }
    }
    return AT_OK;
}

VOS_UINT32 At_SndDcxoReq(VOS_VOID)
{
    BBIC_CAL_DCXO_REQ_STRU *dcxoReq     = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;

    /* 分配消息空间 */
    dcxoReq = (BBIC_CAL_DCXO_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                     sizeof(BBIC_CAL_DCXO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    if (dcxoReq == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(dcxoReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(dcxoReq, DSP_PID_BBA_CAL, ID_AT_BBIC_DCXO_REQ);

    dcxoReq->stPara.enSetType = atMtInfoCtx->bbicInfo.dcxoTempCompEnableFlg;
    dcxoReq->stPara.txArfcn   = atMtInfoCtx->atInfo.bandArfcn.ulChanNo;
    dcxoReq->stPara.usBand    = atMtInfoCtx->bbicInfo.dspBandFreq.dspBand;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, dcxoReq) != VOS_OK) {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndDcxoReq Exit");

    return VOS_TRUE;
}

VOS_UINT32 AT_SetDcxotempcompPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("AT_SetDcxotempcompPara Enter");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    if (g_atParaList[0].paraValue == 0) {
        atMtInfoCtx->bbicInfo.dcxoTempCompEnableFlg = BBIC_DCXO_SET_DISABLE;
    } else {
        atMtInfoCtx->bbicInfo.dcxoTempCompEnableFlg = BBIC_DCXO_SET_ENABLE;
    }

    if (At_SndDcxoReq() == VOS_FALSE) {
        return AT_SND_MSG_FAIL;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DCXOTEMPCOMP_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SndRxOnOffReq(VOS_VOID)
{
    AT_BBIC_CAL_MT_RX_REQ *rxReq       = VOS_NULL_PTR;
    AT_MT_Info            *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 分配消息空间 */
    rxReq = (AT_BBIC_CAL_MT_RX_REQ *)PS_ALLOC_MSG(WUEPS_PID_AT, sizeof(AT_BBIC_CAL_MT_RX_REQ) - VOS_MSG_HEAD_LENGTH);
    if (rxReq == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(rxReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(rxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_MT_RX_REQ);

    /* 填写消息内容 */
#if (FEATURE_LTEV == FEATURE_ON)
    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_LTEV) {
        rxReq->stComponentID.uwComponentType = AT_COMPONENTTYPE_RAT_LTEV;
    }
#endif
    rxReq->stMtRxPara.ucRxEnable      = atMtInfoCtx->atInfo.tempRxorTxOnOff;
    rxReq->stMtRxPara.uhwAgcGainIndex = atMtInfoCtx->atInfo.agcGainLevel;
    rxReq->stMtRxPara.enModemId       = MODEM_ID_0;
    rxReq->stMtRxPara.enRatMode       = atMtInfoCtx->bbicInfo.currtRatMode;
    rxReq->stMtRxPara.enBand          = atMtInfoCtx->bbicInfo.dspBandFreq.dspBand;
    rxReq->stMtRxPara.enBandWith      = atMtInfoCtx->bbicInfo.bandWidth;
    rxReq->stMtRxPara.dlFreqInfo      = atMtInfoCtx->bbicInfo.dspBandFreq.dlFreq;

    /* MIMO */
    if (atMtInfoCtx->bbicInfo.rxAntType == AT_ANT_TYPE_MIMO) {
        rxReq->stMtRxPara.usMimoType = atMtInfoCtx->bbicInfo.rxMimoType;
        rxReq->stMtRxPara.usAntMap   = atMtInfoCtx->bbicInfo.rxMimoAntNum;
    } else {
        rxReq->stMtRxPara.usMimoType = AT_SET_BIT32(1);
        rxReq->stMtRxPara.usAntMap   = atMtInfoCtx->bbicInfo.rxAntType;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, rxReq) != VOS_OK) {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndRxOnOffReq Exit");

    return VOS_TRUE;
}

VOS_UINT32 At_SetFRxonPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetFRxonPara Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不符合要求 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 该AT命令在AT^TMODE=1非信令模式下使用 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该AT命令需要在AT^FCHAN设置非信令信道命令执行成功后使用 */
    if (atMtInfoCtx->atInfo.dspLoadFlag == VOS_FALSE) {
        return AT_CHANNEL_NOT_SET;
    }

    /* 该AT命令需要在AT^TSELRF设置非信令信道命令执行成功后使用 */
    if (atMtInfoCtx->atInfo.setRxTselrfFlag == VOS_FALSE) {
        return AT_NOT_SET_PATH;
    }

    atMtInfoCtx->atInfo.tempRxorTxOnOff = (AT_DSP_RfOnOffUint8)g_atParaList[0].paraValue;

    /* 给BBIC CAL发消息 */
    if (At_SndRxOnOffReq() == VOS_FALSE) {
        return AT_SND_MSG_FAIL;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SET_FRXON;
    atMtInfoCtx->atInfo.indexNum          = indexNum;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SndBbicCalMipiReadReq(VOS_UINT32 mipiPortSel, VOS_UINT32 secondaryId, VOS_UINT32 regAddr,
                                    VOS_UINT32 byteCnt, VOS_UINT32 readSpeed)
{
    BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU *msg = VOS_NULL_PTR;
    VOS_UINT32                            length;

    /* 申请BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU消息 */
    length = sizeof(BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    msg    = (BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SndBbicCalMipiReadReq: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(msg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(msg, DSP_PID_BBA_CAL, ID_AT_BBIC_MIPI_READ_REQ);

    /* 与产品线确认，每次只读写一个MIPI */
    msg->stPara.uwMipiNum                = 1;
    msg->stPara.astCMD[0].bitMipiPortSel = mipiPortSel;
    msg->stPara.astCMD[0].bitSecondaryId = secondaryId;
    msg->stPara.astCMD[0].bitRegAddr     = regAddr;
    msg->stPara.astCMD[0].bitByteCnt     = byteCnt;
    msg->stPara.readSpeedType            = (VOS_UINT8)readSpeed;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_ERR_LOG("AT_SndBbicCalMipiReadReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalMipiReadReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 AT_SndBbicCalMipiWriteReq(VOS_UINT32 mipiPortSel, VOS_UINT32 secondaryId, VOS_UINT32 regAddr,
                                     VOS_UINT32 byteCnt, VOS_UINT32 value)
{
    BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU *msg = VOS_NULL_PTR;
    VOS_UINT32                             length;

    /* 申请BBIC_CAL_RF_DEBUG_READ_MIPI_REQ_STRU消息 */
    length = sizeof(BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    msg    = (BBIC_CAL_RF_DEBUG_WRITE_MIPI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SndBbicCalMipiWriteReqMsg: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(msg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(msg, DSP_PID_BBA_CAL, ID_AT_BBIC_MIPI_WRITE_REQ);

    msg->stPara.uwMipiNum                       = 1;
    msg->stPara.astData[0].stCmd.bitMipiPortSel = mipiPortSel;
    msg->stPara.astData[0].stCmd.bitSecondaryId = secondaryId;
    msg->stPara.astData[0].stCmd.bitRegAddr     = regAddr;
    msg->stPara.astData[0].stCmd.bitByteCnt     = byteCnt;
    msg->stPara.astData[0].stData.bitByte0      = value;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_ERR_LOG("AT_SndBbicCalMipiWriteReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalMipiWriteReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetMipiOpeRatePara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;
    VOS_UINT32  readSpeed;

    atMtInfoCtx = AT_GetMtInfoCtx();
    result      = AT_FAILURE;
    readSpeed   = 0;

    AT_PR_LOGH("AT_SetMipiOpeRatePara Enter");

    /* 状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不符合要求 */
    if (g_atParaIndex < AT_MIPIOPERATE_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue == 0) {
        if (g_atParaList[AT_MIPIOPERATE_DATA_BITBYTE0_VALUE].paraLen != 0) {
            readSpeed = g_atParaList[AT_MIPIOPERATE_DATA_BITBYTE0_VALUE].paraValue;
        }

        if (g_atParaList[AT_MIPIOPERATE_BYTE_CNT].paraValue <= AT_MT_MIPI_READ_MAX_BYTE) {
            /* 由于底层不支持设置读取速度，则最后一个参数忽略了 */
            result = AT_SndBbicCalMipiReadReq(g_atParaList[AT_MIPIOPERATE_MIPI_PORT_SEL].paraValue,
                                              g_atParaList[AT_MIPIOPERATE_SECONDARY_ID].paraValue,
                                              g_atParaList[AT_MIPIOPERATE_REG_ADDR].paraValue,
                                              g_atParaList[AT_MIPIOPERATE_BYTE_CNT].paraValue, readSpeed);
        }
    } else {
        if (g_atParaList[AT_MIPIOPERATE_DATA_BITBYTE0_VALUE].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* MIPI写的时候，默认只写一个值 */
        g_atParaList[AT_MIPIOPERATE_BYTE_CNT].paraValue = 1;
        result = AT_SndBbicCalMipiWriteReq(g_atParaList[AT_MIPIOPERATE_MIPI_PORT_SEL].paraValue,
                                           g_atParaList[AT_MIPIOPERATE_SECONDARY_ID].paraValue,
                                           g_atParaList[AT_MIPIOPERATE_REG_ADDR].paraValue,
                                           g_atParaList[AT_MIPIOPERATE_BYTE_CNT].paraValue,
                                           g_atParaList[AT_MIPIOPERATE_DATA_BITBYTE0_VALUE].paraValue);
    }

    /* 发送失败 */
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetMipiOpeRatePara: AT Snd ReqMsg fail.");
        return AT_SND_MSG_FAIL;
    }

    /* 发送成功，设置当前操作模式 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MIPIOPERATE_SET;
    atMtInfoCtx->atInfo.indexNum          = indexNum;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetFRbInfoPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("AT_SetFRbInfoPara Enter");

    /* 状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM) {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用 */
    if (atMtInfoCtx->atInfo.dspLoadFlag == VOS_FALSE) {
        return AT_NOT_LOAD_DSP;
    }

    /* 参数检查 */
    if (g_atParaIndex != AT_FRBINFO_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    atMtInfoCtx->atInfo.rbNum      = (VOS_UINT16)g_atParaList[AT_FRBINFO_PARA_RBNUM].paraValue;
    atMtInfoCtx->atInfo.rbStartCfg = (VOS_UINT16)g_atParaList[AT_FRBINFO_PARA_RBSTARTCFG].paraValue;
    atMtInfoCtx->atInfo.rbEnable   = VOS_TRUE;

    return AT_OK;
}

VOS_UINT32 At_SndDspIdleSltTestReq(VOS_VOID)
{
    FESS_FLPM_NasSerdesSltReq *testReq = VOS_NULL_PTR;

    /* 分配消息空间 */
    testReq = (FESS_FLPM_NasSerdesSltReq *)PS_ALLOC_MSG(WUEPS_PID_AT,
        sizeof(FESS_FLPM_NasSerdesSltReq) - VOS_MSG_HEAD_LENGTH);

    if (testReq == VOS_NULL_PTR) {
        AT_ERR_LOG("At_SndDspIdleSltTestReq: alloc msg fail !");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(testReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(testReq, I0_PHY_PID_IDLE, ID_FESS_NAS_FLPM_SERDES_SLT_REQ);

    if (g_atParaList[0].paraValue == AT_SET_SLT_SERDES_TEST) {
        testReq->testType = SLT_SERDES_TSET;
    }

    if (g_atParaList[0].paraValue == AT_SET_SLT_ALINK_TEST) {
        testReq->testType = SLT_ALINK_TEST;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, testReq) != VOS_OK) {
        AT_ERR_LOG("At_SndDspIdleSltTestReq: Send msg fail !");
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndDspIdleSltTestReq Exit");

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetFPllStatusQryPara(VOS_UINT8 indexNum)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  result;

    atMtInfoCtx = AT_GetMtInfoCtx();
    result      = AT_FAILURE;

    AT_PR_LOGH("AT_SetFPllStatusQryPara Enter");

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (atMtInfoCtx->atInfo.dspLoadFlag != VOS_TRUE) {
        return AT_NOT_LOAD_DSP;
    }

    result = AT_SndBbicPllStatusReq();

    /* 发送失败 */
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetFPllStatusQryPara: AT Snd ReqMsg fail.");
        return AT_ERROR;
    }

    atMtInfoCtx->atInfo.antType = (AT_MT_AntTypeUint8)g_atParaList[AT_FPLL_STATUS_TX].paraValue;

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FPLLSTATUS_SET;
    atMtInfoCtx->atInfo.indexNum          = indexNum;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 At_SetDpdtTestFlagPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetDpdttestFlagReq atCmd;
    VOS_UINT32                rst;

    /* 参数检查 */
    if (g_atParaIndex != AT_DPDTTEST_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.ratMode = (AT_MTA_CmdRatmodeUint8)g_atParaList[0].paraValue;
    atCmd.flag    = (VOS_UINT8)g_atParaList[1].paraValue;

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_DPDTTEST_FLAG_REQ,
                                 (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_SetDpdttestFlagReq), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DPDTTEST_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetRxTestModePara(VOS_UINT8 indexNum)
{
    AT_MTA_SetRxtestmodeReq rxTestModeCfg;
    VOS_UINT32              rst;

    (VOS_VOID)memset_s(&rxTestModeCfg, (VOS_SIZE_T)sizeof(rxTestModeCfg), 0x00, (VOS_SIZE_T)sizeof(rxTestModeCfg));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetRxTestModePara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetRxTestModePara : The number of input parameters is error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetRxTestModePara : The number of input parameters is zero.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rxTestModeCfg.cfg = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 发送跨核消息到C核, 设置侦听测试模式 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_RX_TEST_MODE_SET_REQ, (VOS_UINT8 *)&rxTestModeCfg,
                                 (VOS_SIZE_T)sizeof(rxTestModeCfg), I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetRxTestModePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RXTESTMODE_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetLcaCellRptCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_CaCellSetReq cACellType;
    VOS_UINT32          rst;

    (VOS_VOID)memset_s(&cACellType, sizeof(cACellType), 0x00, sizeof(cACellType));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数及长度检查 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    cACellType.blEnableType = g_atParaList[0].paraValue;

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_LTE_CA_CELL_RPT_CFG_SET_REQ, (VOS_UINT8 *)&cACellType, sizeof(cACellType),
                                 I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetLcaCellRptCfgPara: AT_SetLcaCellRptCfgPara fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LCACELLRPTCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 AT_SetRadverPara(VOS_UINT8 indexNum)
{
    VOS_UINT32        ret;
    L4A_SET_RadverReq req = {0};

    if ((g_atParaIndex != AT_RADVER_PARA_NUM) || (g_atParaList[AT_RADVER_MOD].paraLen == 0) ||
        (g_atParaList[AT_RADVER_VER].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    req.ctrl.clientId = g_atClientTab[indexNum].clientId;
    req.ctrl.opId     = 0;
    req.ctrl.pid      = WUEPS_PID_AT;

    req.mod = g_atParaList[AT_RADVER_MOD].paraValue;
    req.ver = g_atParaList[AT_RADVER_VER].paraValue;

    ret = atSendL4aDataMsg(g_atClientTab[indexNum].clientId, I0_MSP_L4_L4A_PID, ID_MSG_L4A_RADVER_SET_REQ,
                           (VOS_UINT8 *)(&req), sizeof(req));

    if (ret == ERR_MSP_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RADVER_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetForceSyncPara(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
#else
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
#endif
    AT_MTA_SetForcesyncReq atCmd;
    VOS_UINT32             ret;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));

    /* 该命令需在非信令模式下使用 */
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    atDevCmdCtrl = AT_GetDevCmdCtrl();
    if (atDevCmdCtrl->currentTMode != AT_TMODE_FTM)
#else
    atMtInfoCtx             = AT_GetMtInfoCtx();
    if (atMtInfoCtx->currentTMode != AT_TMODE_FTM)
#endif
    {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不等于1个，返回AT_CME_INCORRECT_PARAMETERS */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        AT_WARN_LOG("AT_SetForceSyncPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    atCmd.num = (VOS_UINT16)g_atParaList[0].paraValue;

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_FORCESYNC_SET_REQ, (VOS_UINT8 *)&atCmd,
                                 sizeof(AT_MTA_SetForcesyncReq), I0_UEPS_PID_MTA);

    if (ret != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetForceSyncPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FORCESYNC_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))

VOS_UINT32 AT_SetSltTestPara(VOS_UINT8 indexNum)
{
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    VOS_INT iResult = MDRV_OK;
#endif
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
#endif

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* SLT测试参数有效范围为1~6 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen != 1)) {
        AT_WARN_LOG("AT_SetSltTest: slt test para error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 根据命令参数执行相应的底软测试接口 */
    switch (g_atParaList[0].paraValue) {
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case AT_SET_SLT_SERDES_TEST:
        case AT_SET_SLT_ALINK_TEST:
            atMtInfoCtx = AT_GetMtInfoCtx();
            if (At_SndDspIdleSltTestReq() == AT_SUCCESS) {
                atMtInfoCtx->atInfo.indexNum          = indexNum;
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SLT_TEST_SET;

                /* 等待异步处理时间返回 */
                return AT_WAIT_ASYNC_RETURN;
            } else {
                return AT_ERROR;
            }
#endif
        default:
            AT_WARN_LOG("AT_SetSltTest: slt test fail!");
            return AT_ERROR;
    }
#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
    if (iResult != MDRV_OK) {
        AT_WARN_LOG("AT_SetSltTest: slt test fail!");
        return AT_ERROR;
    }

    return AT_OK;
#endif
}

#endif



