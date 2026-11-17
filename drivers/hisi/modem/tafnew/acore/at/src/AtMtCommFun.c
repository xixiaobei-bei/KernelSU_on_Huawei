/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtMtInterface.h"
#include "TafDrvAgent.h"
#include "AtParse.h"
#include "AtMtCommFun.h"
#include "AtMtMsgProc.h"
#include "ATCmdProc.h"
#include "AtInputProc.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_MT_COMM_FUN_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if(FEATURE_ON == FEATURE_UE_MODE_NR)
AT_MT_INFO_STRU                         g_stMtInfoCtx           = {0};
#else
AT_DEVICE_CMD_CTRL_STRU                 g_stAtDevCmdCtrl        = {0};
#endif


#if(FEATURE_ON == FEATURE_UE_MODE_NR)

AT_BAND_WIDTH_INFO_STRU g_astBandWidthTable[] =
{
    {AT_BAND_WIDTH_200K  , BANDWIDTH_200K  , AT_BAND_WIDTH_VALUE_200K  },
    {AT_BAND_WIDTH_1M2288, BANDWIDTH_1M2288, AT_BAND_WIDTH_VALUE_1M2288},
    {AT_BAND_WIDTH_1M28  , BANDWIDTH_1M28  , AT_BAND_WIDTH_VALUE_1M28  },
    {AT_BAND_WIDTH_1M4   , BANDWIDTH_1M4   , AT_BAND_WIDTH_VALUE_1M4   },
    {AT_BAND_WIDTH_3M    , BANDWIDTH_3M    , AT_BAND_WIDTH_VALUE_3M    },
    {AT_BAND_WIDTH_5M    , BANDWIDTH_5M    , AT_BAND_WIDTH_VALUE_5M    },
    {AT_BAND_WIDTH_10M   , BANDWIDTH_10M   , AT_BAND_WIDTH_VALUE_10M   },
    {AT_BAND_WIDTH_15M   , BANDWIDTH_15M   , AT_BAND_WIDTH_VALUE_15M   },
    {AT_BAND_WIDTH_20M   , BANDWIDTH_20M   , AT_BAND_WIDTH_VALUE_20M   },
    {AT_BAND_WIDTH_25M   , BANDWIDTH_25M   , AT_BAND_WIDTH_VALUE_25M   },
    {AT_BAND_WIDTH_30M   , BANDWIDTH_30M   , AT_BAND_WIDTH_VALUE_30M   },
    {AT_BAND_WIDTH_40M   , BANDWIDTH_40M   , AT_BAND_WIDTH_VALUE_40M   },
    {AT_BAND_WIDTH_50M   , BANDWIDTH_50M   , AT_BAND_WIDTH_VALUE_50M   },
    {AT_BAND_WIDTH_60M   , BANDWIDTH_60M   , AT_BAND_WIDTH_VALUE_60M   },
    {AT_BAND_WIDTH_80M   , BANDWIDTH_80M   , AT_BAND_WIDTH_VALUE_80M   },
    {AT_BAND_WIDTH_90M   , BANDWIDTH_90M   , AT_BAND_WIDTH_VALUE_90M   },
    {AT_BAND_WIDTH_100M  , BANDWIDTH_100M  , AT_BAND_WIDTH_VALUE_100M  },
    {AT_BAND_WIDTH_200M  , BANDWIDTH_200M  , AT_BAND_WIDTH_VALUE_200M  },
    {AT_BAND_WIDTH_400M  , BANDWIDTH_400M  , AT_BAND_WIDTH_VALUE_400M  },
    {AT_BAND_WIDTH_800M  , BANDWIDTH_800M  , AT_BAND_WIDTH_VALUE_800M  },
    {AT_BAND_WIDTH_1G    , BANDWIDTH_1G    , AT_BAND_WIDTH_VALUE_1G    },
};

AT_PATH_TO_ANT_TYPE_STRU g_astPath2AntTypeTable[]=
{
    {AT_TSELRF_PATH_GSM         , AT_ANT_TYPE_PRI },
    {AT_TSELRF_PATH_WCDMA_PRI   , AT_ANT_TYPE_PRI },
    {AT_TSELRF_PATH_WCDMA_DIV   , AT_ANT_TYPE_DIV },
    {AT_TSELRF_PATH_CDMA_PRI    , AT_ANT_TYPE_PRI },
    {AT_TSELRF_PATH_CDMA_DIV    , AT_ANT_TYPE_DIV },
    {AT_TSELRF_PATH_FDD_LTE_PRI , AT_ANT_TYPE_PRI },
    {AT_TSELRF_PATH_FDD_LTE_DIV , AT_ANT_TYPE_DIV },
    {AT_TSELRF_PATH_FDD_LTE_MIMO, AT_ANT_TYPE_MIMO},
    {AT_TSELRF_PATH_TDD_LTE_PRI , AT_ANT_TYPE_PRI },
    {AT_TSELRF_PATH_TDD_LTE_DIV , AT_ANT_TYPE_DIV },
    {AT_TSELRF_PATH_TDD_LTE_MIMO, AT_ANT_TYPE_MIMO},
    {AT_TSELRF_PATH_NR_PRI      , AT_ANT_TYPE_PRI },
    {AT_TSELRF_PATH_NR_DIV      , AT_ANT_TYPE_DIV },
    {AT_TSELRF_PATH_NR_MIMO     , AT_ANT_TYPE_MIMO},
};




/*
    频率的计算方法:FREF = FREF-Offs + ΔFGlobal (NREF – NREF-Offs)

    Frequency     ΔFGlobal     FREF-Offs    NREF-Offs     Range of NREF
    range [MHz]     [kHz]        [MHz]

    0 – 3000        5            0            0            0 – 599999
    3000 – 24250   15            3000        600000        600000 – 2016666
    24250 – 100000 60           24250.08      2016667      2016667 – 3279165
*/

const AT_NR_FREQ_OFFSET_TABLE_STRU g_astAtNrFreqOffsetTable[] =
{
    {{0                           , 3000 * FREQ_UNIT_MHZ_TO_KHZ  },  5, 0                          , 0      , {0      , 599999}},
    {{3000 * FREQ_UNIT_MHZ_TO_KHZ , 24250 * FREQ_UNIT_MHZ_TO_KHZ }, 15, 3000 * FREQ_UNIT_MHZ_TO_KHZ, 600000 , {600000 ,2016666}},
    {{24250 * FREQ_UNIT_MHZ_TO_KHZ, 100000 * FREQ_UNIT_MHZ_TO_KHZ}, 60, 24250080                   , 2016667, {2016667,3279165}},
};


const AT_NR_BAND_INFO_STRU g_astAtNrBandInfoTable[]=
{
    {1   ,   AT_DUPLEX_MODE_FDD, {384000  ,   396000  }, {422000  ,   434000  },{ 1920000    ,   1980000    }, {2110000    ,   2170000 }},
    {2   ,   AT_DUPLEX_MODE_FDD, {370000  ,   382000  }, {386000  ,   398000  },{ 1850000    ,   1910000    }, {1930000    ,   1990000 }},
    {3   ,   AT_DUPLEX_MODE_FDD, {342000  ,   357000  }, {361000  ,   376000  },{ 1710000    ,   1785000    }, {1805000    ,   1880000 }},
    {5   ,   AT_DUPLEX_MODE_FDD, {164800  ,   169800  }, {173800  ,   178800  },{ 824000     ,   849000     }, {869000     ,   894000  }},
    {7   ,   AT_DUPLEX_MODE_FDD, {500000  ,   514000  }, {524000  ,   538000  },{ 2500000    ,   2570000    }, {2620000    ,   2690000 }},
    {8   ,   AT_DUPLEX_MODE_FDD, {176000  ,   183000  }, {185000  ,   192000  },{ 880000     ,   915000     }, {925000     ,   960000  }},
    {12  ,   AT_DUPLEX_MODE_FDD, {139800  ,   143200  }, {145800  ,   149200  },{ 699000     ,   716000     }, {729000     ,   746000  }},
    {20  ,   AT_DUPLEX_MODE_FDD, {166400  ,   172400  }, {158200  ,   164200  },{ 832000     ,   862000     }, {791000     ,   821000  }},
    {25  ,   AT_DUPLEX_MODE_FDD, {370000  ,   383000  }, {386000  ,   399000  },{ 1850000    ,   1915000    }, {1930000    ,   1995000 }},
    {28  ,   AT_DUPLEX_MODE_FDD, {140600  ,   149600  }, {151600  ,   160600  },{ 703000     ,   748000     }, {758000     ,   803000  }},
    {34  ,   AT_DUPLEX_MODE_TDD, {402000  ,   405000  }, {402000  ,   405000  },{ 2010000    ,   2025000    }, {2010000    ,   2025000 }},
    {38  ,   AT_DUPLEX_MODE_TDD, {514000  ,   524000  }, {514000  ,   524000  },{ 2570000    ,   2620000    }, {2570000    ,   2620000 }},
    {39  ,   AT_DUPLEX_MODE_TDD, {376000  ,   384000  }, {376000  ,   384000  },{ 1880000    ,   1920000    }, {1880000    ,   1920000 }},
    {40  ,   AT_DUPLEX_MODE_TDD, {460000  ,   480000  }, {460000  ,   480000  },{ 2300000    ,   2400000    }, {2300000    ,   2400000 }},
    {41  ,   AT_DUPLEX_MODE_TDD, {499200  ,   537999  }, {499200  ,   537999  },{ 2496000    ,   2690000    }, {2496000    ,   2690000 }},
    {51  ,   AT_DUPLEX_MODE_TDD, {285400  ,   286400  }, {285400  ,   286400  },{ 1427000    ,   1432000    }, {1427000    ,   1432000 }},
    {66  ,   AT_DUPLEX_MODE_FDD, {342000  ,   356000  }, {422000  ,   440000  },{ 1710000    ,   1780000    }, {2110000    ,   2200000 }}, /* 上下行频点不对称 */
    {70  ,   AT_DUPLEX_MODE_FDD, {339000  ,   342000  }, {399000  ,   404000  },{ 1695000    ,   1710000    }, {1995000    ,   2020000 }}, /* 上下行频点不对称 */
    {71  ,   AT_DUPLEX_MODE_FDD, {132600  ,   139600  }, {123400  ,   130400  },{ 663000     ,   698000     }, {617000     ,   652000  }},
    {75  ,   AT_DUPLEX_MODE_SDL, {0       ,   0       }, {286400  ,   303400  },{ 0          ,   0          }, {1432000    ,   1517000 }}, /* 上下行频点不对称 */
    {76  ,   AT_DUPLEX_MODE_SDL, {0       ,   0       }, {285400  ,   286400  },{ 0          ,   0          }, {1427000    ,   1432000 }}, /* 上下行频点不对称 */
    {77  ,   AT_DUPLEX_MODE_TDD, {620000  ,   680000  }, {620000  ,   680000  },{ 3300000    ,   4200000    }, {3300000    ,   4200000 }},
    {78  ,   AT_DUPLEX_MODE_TDD, {620000  ,   653333  }, {620000  ,   653333  },{ 3300000    ,   3800000    }, {3300000    ,   3800000 }},
    {79  ,   AT_DUPLEX_MODE_TDD, {693334  ,   733333  }, {693334  ,   733333  },{ 4400000    ,   5000000    }, {4400000    ,   5000000 }},
    {80  ,   AT_DUPLEX_MODE_SUL, {342000  ,   357000  }, {0       ,   0       },{ 1710000    ,   1785000    }, {0          ,   0       }},
    {81  ,   AT_DUPLEX_MODE_SUL, {176000  ,   183000  }, {0       ,   0       },{ 880000     ,   915000     }, {0          ,   0       }},
    {82  ,   AT_DUPLEX_MODE_SUL, {166400  ,   172400  }, {0       ,   0       },{ 832000     ,   862000     }, {0          ,   0       }},
    {83  ,   AT_DUPLEX_MODE_SUL, {140600  ,   149600  }, {0       ,   0       },{ 703000     ,   748000     }, {0          ,   0       }},
    {84  ,   AT_DUPLEX_MODE_SUL, {384000  ,   396000  }, {0       ,   0       },{ 1920000    ,   1980000    }, {0          ,   0       }},
    {86  ,   AT_DUPLEX_MODE_SUL, {342000  ,   356000  }, {0       ,   0       },{ 1710000    ,   1780000    }, {0          ,   0       }},
    {257 ,   AT_DUPLEX_MODE_TDD, {2054166 ,   2104165 }, {2054166 ,   2104165 },{ 26500000   ,   29500000   }, {26500000   ,   29500000}},
    {258 ,   AT_DUPLEX_MODE_TDD, {2016667 ,   2070832 }, {2016667 ,   2070832 },{ 24250000   ,   27500000   }, {24250000   ,   27500000}},
    {260 ,   AT_DUPLEX_MODE_TDD, {2229166 ,   2279165 }, {2229166 ,   2279165 },{ 37000000   ,   40000000   }, {37000000   ,   40000000}},
    {261 ,   AT_DUPLEX_MODE_TDD, {2070833 ,   2087497 }, {2070833 ,   2087497 },{ 27500000   ,   28500000   }, {27500000   ,   28350000}},
};


const AT_LTE_BAND_INFO_STRU g_astAtLteBandInfoTable[] =
{
    {   1,   0, 2110000,  0,      {0,      599},    1920000,  18000,  {18000,  18599}  },
    {   2,   0, 1930000,  600,    {600,    1199},   1850000,  18600,  {18600,  19199}  },
    {   3,   0, 1805000,  1200,   {1200,   1949},   1710000,  19200,  {19200,  19949}  },
    {   4,   0, 2110000,  1950,   {1950,   2399},   1710000,  19950,  {19950,  20399}  },
    {   5,   0, 869000,   2400,   {2400,   2649},   824000,   20400,  {20400,  20649}  },
    {   6,   0, 875000,   2650,   {2650,   2749},   830000,   20650,  {20650,  20749}  },
    {   7,   0, 2620000,  2750,   {2750,   3449},   2500000,  20750,  {20750,  21449}  },
    {   8,   0, 925000,   3450,   {3450,   3799},   880000,   21450,  {21450,  21799}  },
    {   9,   0, 1844900,  3800,   {3800,   4149},   1749900,  21800,  {21800,  22149}  },
    {   10,  0, 2110000,  4150,   {4150,   4749},   1710000,  22150,  {22150,  22749}  },
    {   11,  0, 1475900,  4750,   {4750,   4949},   1427900,  22750,  {22750,  22949}  },
    {   12,  0, 729000,   5010,   {5010,   5179},   699000,   23010,  {23010,  23179}  },
    {   13,  0, 746000,   5180,   {5180,   5279},   777000,   23180,  {23180,  23279}  },
    {   14,  0, 758000,   5280,   {5280,   5379},   788000,   23280,  {23280,  23379}  },
    {   17,  0, 734000,   5730,   {5730,   5849},   704000,   23730,  {23730,  23849}  },
    {   18,  0, 860000,   5850,   {5850,   5999},   815000,   23850,  {23850,  23999}  },
    {   19,  0, 875000,   6000,   {6000,   6149},   830000,   24000,  {24000,  24149}  },
    {   20,  0, 791000,   6150,   {6150,   6449},   832000,   24150,  {24150,  24449}  },
    {   21,  0, 1495900,  6450,   {6450,   6599},   1447900,  24450,  {24450,  24599}  },
    {   22,  0, 3510000,  6600,   {6600,   7399},   3410000,  24600,  {24600,  25399}  },
    {   23,  0, 2180000,  7500,   {7500,   7699},   2000000,  25500,  {25500,  25699}  },
    {   24,  0, 1525000,  7700,   {7700,   8039},   1626500,  25700,  {25700,  26039}  },
    {   25,  0, 1930000,  8040,   {8040,   8689},   1850000,  26040,  {26040,  26689}  },
    {   26,  0, 859000,   8690,   {8690,   9039},   814000,   26690,  {26690,  27039}  },
    {   27,  0, 852000,   9040,   {9040,   9209},   807000,   27040,  {27040,  27209}  },
    {   28,  0, 758000,   9210,   {9210,   9659},   703000,   27210,  {27210,  27659}  },
    {   29,  0, 717000,   9660,   {9660,   9769},   0,        0,      {0,      0    }  },
    {   30,  0, 2350000,  9770,   {9770,   9869},   2305000,  27660,  {27660,  27759}  },
    {   31,  0, 462500,   9870,   {9870,   9919},   452500,   27760,  {27760,  27809}  },
    {   32,  0, 1452000,  9920,   {9920,   10359},  0,        0,      {0,      0,   }  },
    {   33,  0, 1900000,  36000,  {36000,  36199},  1900000,  36000,  {36000,  36199}  },
    {   34,  0, 2010000,  36200,  {36200,  36349},  2010000,  36200,  {36200,  36349}  },
    {   35,  0, 1850000,  36350,  {36350,  36949},  1850000,  36350,  {36350,  36949}  },
    {   36,  0, 1930000,  36950,  {36950,  37549},  1930000,  36950,  {36950,  37549}  },
    {   37,  0, 1910000,  37550,  {37550,  37749},  1910000,  37550,  {37550,  37749}  },
    {   38,  0, 2570000,  37750,  {37750,  38249},  2570000,  37750,  {37750,  38249}  },
    {   39,  0, 1880000,  38250,  {38250,  38649},  1880000,  38250,  {38250,  38649}  },
    {   40,  0, 2300000,  38650,  {38650,  39649},  2300000,  38650,  {38650,  39649}  },
    {   41,  0, 2496000,  39650,  {39650,  41589},  2496000,  39650,  {39650,  41589}  },
    {   42,  0, 3400000,  41590,  {41590,  43589},  3400000,  41590,  {41590,  43589}  },
    {   43,  0, 3600000,  43590,  {43590,  45589},  3600000,  43590,  {43590,  45589}  },
    {   44,  0, 703000,   45590,  {45590,  46589},  703000,   45590,  {45590,  46589}  },
    {   45,  0, 1447000,  46590,  {46590,  46789},  1447000,  46590,  {46590,  46789}  },
    {   46,  0, 5150000,  46790,  {46790,  54539},  5150000,  46790,  {46790,  54539}  },
    {   47,  0, 5855000,  54540,  {54540,  55239},  5855000,  54540,  {54540,  55239}  },
    {   48,  0, 3550000,  55240,  {55240,  56739},  3550000,  55240,  {55240,  56739}  },
    {   49,  0, 3550000,  56740,  {56740,  58239},  3550000,  56740,  {56740,  58239}  },
    {   50,  0, 1432000,  58240,  {58240,  59089},  1432000,  58240,  {58240,  59089}  },
    {   51,  0, 1427000,  59090,  {59090,  59139},  1427000,  59090,  {59090,  59139}  },
    {   52,  0, 3300000,  59140,  {59140,  60139},  3300000,  59140,  {59140,  60139}  },
    {   65,  0, 2110000,  65536,  {65536,  66435},  1920000,  131072, {131072, 131971} },
    {   66,  0, 2110000,  66436,  {66436,  67335},  1710000,  131972, {131972, 132671} }, /* 这个band的上下行信道是不对称的 */
    {   67,  0, 738000,   67336,  {67336,  67535},  0,        0,      {0,      0     } },
    {   68,  0, 753000,   67536,  {67536,  67835},  698000,   132672, {132672, 132971} },
    {   69,  0, 2570000,  67836,  {67836,  68335},  0,        0,      {0,      0     } },
    {   70,  0, 1995000,  68336,  {68336,  68585},  1695000,  132972, {132972, 133121} }, /* 这个band的上下行信道是不对称的 */
    {   71,  0, 617000,   68586,  {68586,  68935},  663000,   133122, {133122, 133471} },
    {   72,  0, 461000,   68936,  {68936,  68985},  451000,   133472, {133472, 133521} },
    {   73,  0, 460000,   68986,  {68986,  69035},  450000,   133522, {133522, 133571} },
    {   74,  0, 1475000,  69036,  {69036,  69465},  1427000,  133572, {133572, 134001} },
    {   75,  0, 1432000,  69466,  {69466,  70315},  0,        0,      {0,      0     } },
    {   76,  0, 1427000,  70316,  {70316,  70365},  0,        0,      {0,      0     } },
    {   85,  0, 728000,   70366,  {70366,  70545},  698000,   134002, {134002, 134181} },
    {   128, 0, 780500,   9435,   {9435,   9659},   725500,   27435,  {27435,  27659 } },
    {   140, 0, 2300000,  38650,  {38650,  39250},  2300000,  38650,  {38650,  39250 } },
};


const AT_W_BAND_INFO_STRU g_astAtWBandInfoTable[] =
{
    { 1,  0, 0,       {9612, 9888 },  {1922400,    1977600}, 0,       {10562,10838}, {2112400, 2167600 }},
    { 2,  0, 0,       {9262, 9538 },  {1852400,    1907600}, 0,       {9662, 9938 }, {1932400, 1987600 }},
    { 3,  0, 1525000, {937,  1288 },  {1712400,    1782600}, 1575000, {1162, 1513 }, {1807400, 1877600 }},
    { 4,  0, 1450000, {1312, 1513 },  {1712400,    1752600}, 1805000, {1537, 1738 }, {2112400, 2152600 }},
    { 5,  0, 0,       {4132, 4233 },  {826400,     846600 }, 0,       {4357, 4458 }, {871400,  891600  }},
    { 6,  0, 0,       {4162, 4188 },  {832400,     837600 }, 0,       {4387, 4413 }, {877400,  882600  }},
    { 7,  0, 2100000, {2012, 2338 },  {2502400,    2567600}, 2175000, {2237, 2563 }, {2622400, 2687600 }},
    { 8,  0, 340000,  {2712, 2863 },  {882400,     912600 }, 340000,  {2937, 3088 }, {927400,  957600  }},
    { 9,  0, 0,       {8762, 8912 },  {1752400,    1782400}, 0,       {9237, 9387 }, {1847400, 1877400 }},
    { 11, 0, 733000,  {3487, 3562 },  {1430400,    1445400}, 736000,  {3712, 3787 }, {1478400, 1493400 }},
    { 19, 0, 770000,  {312 , 363  },  {832400 ,    842600 }, 735000,  {712,  763  }, {877400 , 887600  }},
};


const AT_G_BAND_INFO_STRU g_astAtGBandInfoTable[] =
{
    {2, 0, 80000, {512, 810 }},                                                 /* GSM1900 */
    {3, 0, 95000, {512, 885 }},                                                 /* GSM1800 */
    {5, 0, 45000, {128, 251 }},                                                 /* GSM850 */
    {8, 0, 45000, {0  , 1023}},                                                 /* GSM900 */
};
#endif


#if(FEATURE_OFF == FEATURE_UE_MODE_NR)
/*****************************************************************************
  3 旧设置函数实现
*****************************************************************************/

VOS_UINT32  At_CheckSupportFChanRat(VOS_VOID)
{
    if ((AT_RAT_MODE_GSM != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_EDGE != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_WCDMA != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_AWS != gastAtParaList[0].ulParaValue)
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
     && (AT_RAT_MODE_CDMA != gastAtParaList[0].ulParaValue)
#endif
    )
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32  At_CheckWifiFChanPara(VOS_VOID)
{
    if (((AT_RAT_MODE_WIFI == gastAtParaList[0].ulParaValue) && (AT_BAND_WIFI != gastAtParaList[1].ulParaValue))
      ||((AT_BAND_WIFI == gastAtParaList[1].ulParaValue)&&(AT_RAT_MODE_WIFI != gastAtParaList[0].ulParaValue)))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT32  At_SetFChanPara(VOS_UINT8 ucIndex )
{
    DRV_AGENT_FCHAN_SET_REQ_STRU         stFchanSetReq;

    /* 调用 LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)

    if ( (AT_RAT_MODE_FDD_LTE == gastAtParaList[0].ulParaValue)
       ||(AT_RAT_MODE_TDD_LTE == gastAtParaList[0].ulParaValue))
    {

        g_stAtDevCmdCtrl.ucDeviceRatMode = (AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)(gastAtParaList[0].ulParaValue);

        return atSetFCHANPara(ucIndex);
    }

#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_RAT_MODE_TDSCDMA == gastAtParaList[0].ulParaValue)
    {
        g_stAtDevCmdCtrl.ucDeviceRatMode = (AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)(gastAtParaList[0].ulParaValue);
        return atSetFCHANPara(ucIndex);
    }
#endif


    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FCHAN_OTHER_ERR;
    }
        /* 参数不符合要求 */
    if (gucAtParaIndex != 3)
    {
        return AT_FCHAN_OTHER_ERR;
    }

    /* 清除WIFI模式 */
    /* WIFI的第一个参数必须为8，第二个参数必须为15*/
    if (VOS_FALSE == At_CheckWifiFChanPara())
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* WIFI 分支 */
    if (BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        if (AT_RAT_MODE_WIFI == gastAtParaList[0].ulParaValue)
        {
            /*WIFI未Enable直接返回失败*/
            if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
            {
                return AT_FCHAN_OTHER_ERR;
            }

            g_stAtDevCmdCtrl.ucDeviceRatMode = AT_RAT_MODE_WIFI;

            return AT_OK;
        }
    }
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 检查FCHAN 的接入模式是否支持*/
    if (VOS_FALSE == At_CheckSupportFChanRat())
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (AT_BAND_BUTT <= gastAtParaList[1].ulParaValue)
    {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    TAF_MEM_SET_S(&stFchanSetReq, sizeof(stFchanSetReq), 0x00, sizeof(DRV_AGENT_FCHAN_SET_REQ_STRU));

    stFchanSetReq.ucLoadDspMode       = At_GetDspLoadMode (gastAtParaList[0].ulParaValue);
    stFchanSetReq.ucCurrentDspMode    = At_GetDspLoadMode (g_stAtDevCmdCtrl.ucDeviceRatMode);  /*当前接入模式 */
    stFchanSetReq.bDspLoadFlag        = g_stAtDevCmdCtrl.bDspLoadFlag;
    stFchanSetReq.ucDeviceRatMode     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stFchanSetReq.ucDeviceAtBand      = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stFchanSetReq.usChannelNo         = (VOS_UINT16)gastAtParaList[2].ulParaValue;

    if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                              gastAtClientTab[ucIndex].opId,
                                              DRV_AGENT_FCHAN_SET_REQ,
                                              &stFchanSetReq,
                                              sizeof(stFchanSetReq),
                                              I0_WUEPS_PID_DRV_AGENT))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FCHAN_SET;             /*设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                           /* 等待异步事件返回 */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetWifiFwavePara(VOS_VOID)
{
    VOS_CHAR                        acCmd[200] = {0};
    
    /*WIFI未Enable直接返回失败*/
    if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
    {
        return AT_ERROR;
    }

    /*向WIFI发送单音波形信号请求*/
    VOS_sprintf_s(acCmd, sizeof(acCmd), "athtestcmd -ieth0 --tx sine --txpwr %d", gastAtParaList[1].ulParaValue/100);

    WIFI_TEST_CMD(acCmd);

    return AT_OK;
}


VOS_UINT32 AT_SetFwavePara(VOS_UINT8 ucIndex)
{
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
        || (AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFWAVEPara(ucIndex);
    }
#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if (AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFWAVEPara(ucIndex);
    }
#endif

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数不正确，必须包括波形类型和波形功率 */
    if (gucAtParaIndex != 2)
    {
        return AT_ERROR;
    }

    /* 目前波形类型只支持设置单音*/
    if (gastAtParaList[0].ulParaValue > 7)
    {
        return AT_ERROR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* WIFI*/
    if (AT_RAT_MODE_WIFI == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_SetWifiFwavePara();
    }


    /* 把设置保存在本地变量
       AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值
       功率值以0.01为单位，传给DSP的值会除10，所以AT还需要将该值再除10*/
    g_stAtDevCmdCtrl.usPower    = (VOS_UINT16)(gastAtParaList[1].ulParaValue/10);
    g_stAtDevCmdCtrl.bPowerFlag = VOS_TRUE;
    g_stAtDevCmdCtrl.bFdacFlag  = VOS_FALSE;
    /* 记录下type信息，并转换为G物理层使用的TxMode，在向物理层发送ID_AT_GHPA_RF_TX_CFG_REQ时携带 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        g_stAtDevCmdCtrl.usTxMode = 8;
    }
    else
    {
        g_stAtDevCmdCtrl.usTxMode = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    }


    /* WCDMA */
    if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* 向WDSP发送开关单音信号的原语请求 */
        if (AT_FAILURE == At_SendContinuesWaveOnToHPA(g_stAtDevCmdCtrl.usPower, ucIndex))
        {
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FWAVE;
        g_stAtDevCmdCtrl.ucIndex = ucIndex;

        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* 向X DSP发送开关单音信号的原语请求 */
        if (AT_FAILURE == At_SendContinuesWaveOnToCHPA(WDSP_CTRL_TX_ONE_TONE, g_stAtDevCmdCtrl.usPower))
        {
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FWAVE;
        g_stAtDevCmdCtrl.ucIndex = ucIndex;

        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }
#endif

    return AT_OK;

}


VOS_UINT32 At_CheckFTxonPara(VOS_UINT8 ucSwitch)
{
    if (ucSwitch > AT_DSP_RF_SWITCH_ON)
    {
        return AT_FTXON_OTHER_ERR;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    return AT_OK;
}



VOS_UINT32  At_SetFTxonPara(VOS_UINT8 ucIndex )
{
    TAF_UINT8                           ucSwitch;
    VOS_UINT32                          ulResult;

    /* 添加 LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFTXONPara(ucIndex);
    }
#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFTXONPara(ucIndex);
    }

#endif
    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FTXON_OTHER_ERR;
    }
    /* 参数不符合要求 */
    if (1 != gucAtParaIndex)
    {
        return AT_FTXON_OTHER_ERR;
    }

    ucSwitch = (VOS_UINT8) gastAtParaList[0].ulParaValue;
    g_stAtDevCmdCtrl.ucTempRxorTxOnOff = ucSwitch;
    ulResult = At_CheckFTxonPara(ucSwitch);
    if (AT_OK != ulResult)
    {
        return ulResult;
    }    

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (AT_FAILURE == At_SendTxOnOffToHPA(ucSwitch, ucIndex))
        {
            return AT_FTXON_SET_FAIL;
        }
    }
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    else if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        if (AT_FAILURE == At_SendTxOnOffToCHPA(ucSwitch))
        {
            return AT_FTXON_SET_FAIL;
        }
    }
#endif
    else
    {
        if (AT_FAILURE == At_SendTxOnOffToGHPA(ucIndex, ucSwitch))
        {
            return AT_FTXON_SET_FAIL;
        }
    }
    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FTXON;
    g_stAtDevCmdCtrl.ucIndex = ucIndex;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */

}


VOS_UINT32  At_SetFRxonPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                           ulSwitch;

     /* 调用 LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFRXONPara(ucIndex);
    }
#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFRXONPara(ucIndex);
    }
#endif


    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FRXON_OTHER_ERR;
    }

    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_FRXON_OTHER_ERR;
    }

    /* 该AT命令在AT^TMODE=1非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该AT命令需要在AT^FCHAN设置非信令信道命令执行成功后使用 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    ulSwitch = gastAtParaList[0].ulParaValue;
    g_stAtDevCmdCtrl.ucTempRxorTxOnOff = (AT_DSP_RF_ON_OFF_ENUM_UINT8)ulSwitch;

    /* 把开关接收机请求发给W物理层 */
    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS   == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (AT_FAILURE == At_SendRxOnOffToHPA(ulSwitch, ucIndex))
        {
            return AT_FRXON_SET_FAIL;
        }
    }
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    else if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        if (AT_FAILURE == At_SendRxOnOffToCHPA(ulSwitch))
        {
            return AT_FRXON_SET_FAIL;
        }
    }
#endif
    else
    {
        /* 把开关接收机请求发给G物理层 */
        if (AT_FAILURE == At_SendRxOnOffToGHPA(ucIndex, ulSwitch))
        {
            return AT_FRXON_SET_FAIL;
        }
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FRXON;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;

}


VOS_UINT32  AT_ProcTSelRfWifiPara(VOS_VOID)
{

    if ( BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        /*WIFI未Enable直接返回失败*/
        if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
        {
            return AT_ERROR;
        }

        g_stAtDevCmdCtrl.ucDeviceRatMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32  AT_ProcTSelRfWDivPara(VOS_UINT8 ucIndex)
{
    if (DRV_AGENT_DSP_RF_SWITCH_ON != g_stAtDevCmdCtrl.ucRxOnOff)
    {
        g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_DIV_ON;
        return AT_OK;
    }
    if (AT_FAILURE == At_SendRfCfgAntSelToHPA(AT_RX_DIV_ON, ucIndex))
    {
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32  AT_ProcTSelRfWPriPara(VOS_UINT8 ucIndex)
{
    if (DRV_AGENT_DSP_RF_SWITCH_ON != g_stAtDevCmdCtrl.ucRxOnOff)
    {
        g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_PRI_ON;
        return AT_OK;
    }

    if (AT_FAILURE == At_SendRfCfgAntSelToHPA(AT_RX_PRI_ON, ucIndex))
    {
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    return AT_WAIT_ASYNC_RETURN;
}



VOS_UINT32 AT_SetTSelRfPara(VOS_UINT8 ucIndex)
{
    DRV_AGENT_TSELRF_SET_REQ_STRU       stTseLrf;
    VOS_BOOL                            bLoadDsp;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数不符合要求 */
    if ((2 < gucAtParaIndex)
     || (0 == gastAtParaList[0].usParaLen))
    {
        return AT_ERROR;
    }

    if (AT_TSELRF_PATH_WIFI == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWifiPara();
    }

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_TSELRF_PATH_TD == gastAtParaList[0].ulParaValue)
    {
        return atSetTselrfPara(ucIndex);
    }
    
#endif

#if (FEATURE_ON == FEATURE_LTE)
    if ((AT_TSELRF_PATH_WCDMA_PRI!=gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_WCDMA_DIV!=gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_GSM !=gastAtParaList[0].ulParaValue))
    {
        return atSetTselrfPara(ucIndex);
    }
#endif

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* 打开分集必须在FRXON之后，参考RXDIV实现 */
    if (AT_TSELRF_PATH_WCDMA_DIV == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWDivPara(ucIndex);
    }

    if (AT_TSELRF_PATH_WCDMA_PRI == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWPriPara(ucIndex);
    }

    /* 此处判断是否需要重新加载DSP: 需要则发请求到C核加载DSP，否则，直接返回OK */
    AT_GetTseLrfLoadDspInfo(gastAtParaList[0].ulParaValue, &bLoadDsp, &stTseLrf);
    if (VOS_TRUE == bLoadDsp)
    {
        if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                                   gastAtClientTab[ucIndex].opId,
                                                   DRV_AGENT_TSELRF_SET_REQ,
                                                   &stTseLrf,
                                                   sizeof(stTseLrf),
                                                   I0_WUEPS_PID_DRV_AGENT))
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;             /*设置当前操作模式 */
            return AT_WAIT_ASYNC_RETURN;                                           /* 等待异步事件返回 */
        }
        else
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}


VOS_UINT32 At_SetFpaPara(VOS_UINT8 ucIndex)
{
    /* 调用LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return AT_CMD_NOT_SUPPORT;
    }
#endif
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FPA_OTHER_ERR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_FPA_OTHER_ERR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用,即^FCHAN成功执行后 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* 把设置保存在本地变量 */
    g_stAtDevCmdCtrl.ucPaLevel = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;

}


VOS_UINT32 At_SetFlnaPara(VOS_UINT8 ucIndex)
{

    /* 调用 LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFLNAPara(ucIndex);
    }
#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFLNAPara(ucIndex);
    }
#endif

    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FLNA_OTHER_ERR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_FLNA_OTHER_ERR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* 参数LNA等级取值范围检查 */
    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        /* WDSP LNA等级取值为0-2 */
        if (gastAtParaList[0].ulParaValue > DSP_LNA_HIGH_GAIN_MODE)
        {
            return AT_FLNA_OTHER_ERR;
        }
    }

    /* 把设置保存在本地变量 */
    g_stAtDevCmdCtrl.ucLnaLevel = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;
}



VOS_UINT32 At_SetDpdtPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_DPDT_VALUE_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_DPDT_VALUE_REQ_STRU));
    stAtCmd.enRatMode   = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ulDpdtValue = gastAtParaList[1].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_DPDT_VALUE_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_DPDT_VALUE_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}


VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 ucIndex)
{
    AT_MTA_QRY_DPDT_VALUE_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_QRY_DPDT_VALUE_REQ_STRU));
    stAtCmd.enRatMode = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_QRY_DPDT_VALUE_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_QRY_DPDT_VALUE_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDTQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

/*****************************************************************************
 函 数 名  : AT_SetDcxotempcompPara
 功能描述  : ^DCXOTEMPCOMP设置命令处理函数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_SetDcxotempcompPara(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    g_stAtDevCmdCtrl.enDcxoTempCompEnableFlg = (AT_DCXOTEMPCOMP_ENABLE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;
}


VOS_UINT32  AT_SetFDac(VOS_UINT8 ucIndex )
{

    TAF_UINT16                           usDAC;

    /*调用 LTE 模的接口分支*/
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return AT_CMD_NOT_SUPPORT;
    }
#endif
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FDAC_SET_FAIL;
    }
    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_FDAC_SET_FAIL;
    }
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    usDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (usDAC > WDSP_MAX_TX_AGC)
        {
            return AT_FDAC_SET_FAIL;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        }
    }
    else
    {
        if (usDAC > GDSP_MAX_TX_VPA)
        {
            return AT_FDAC_SET_FAIL;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        }
    }

    /*AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值*/
    g_stAtDevCmdCtrl.bFdacFlag  = VOS_TRUE;
    g_stAtDevCmdCtrl.bPowerFlag = VOS_FALSE;

    return AT_OK;    /* 返回命令处理挂起状态 */
}



VOS_UINT32 AT_SetMipiWrPara(VOS_UINT8 ucIndex)
{
    AT_HPA_MIPI_WR_REQ_STRU             *pstMsg;
    VOS_UINT32                          ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (5 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    ulLength = sizeof(AT_HPA_MIPI_WR_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_MIPI_WR_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetMipiWrPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue)
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                         = ID_AT_HPA_MIPI_WR_REQ;
    pstMsg->usSlaveAddr                     = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usRegAddr                       = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usRegData                       = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;
    pstMsg->usMipiChannel                   = ( VOS_UINT16 )gastAtParaList[4].ulParaValue;

    /*GSM and UMTS share the same PID*/
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetMipiWrPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_MIPI_WR;                   /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetSSIWrPara(VOS_UINT8 ucIndex)
{
    AT_HPA_SSI_WR_REQ_STRU                  *pstMsg;
    VOS_UINT32                              ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if ( 4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    ulLength = sizeof(AT_HPA_SSI_WR_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_SSI_WR_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if ( VOS_NULL_PTR == pstMsg )
    {
        AT_WARN_LOG("AT_SetSSIWrPara: alloc msg fail!");
        return AT_ERROR;
    }

    if ( AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue )
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                         = ID_AT_HPA_SSI_WR_REQ;
    pstMsg->usRficSsi                       = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usRegAddr                       = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usData                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if ( VOS_OK != PS_SEND_MSG( WUEPS_PID_AT, pstMsg ) )
    {
        AT_WARN_LOG("AT_SetSSIWrPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_SSI_WR;                    /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetSSIRdPara(VOS_UINT8 ucIndex)
{
    AT_HPA_SSI_RD_REQ_STRU                  *pstMsg;
    VOS_UINT32                              ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (3 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if ( AT_RAT_MODE_FDD_LTE == gastAtParaList[0].ulParaValue
      || AT_RAT_MODE_TDD_LTE == gastAtParaList[0].ulParaValue
      || AT_RAT_MODE_TDSCDMA == gastAtParaList[0].ulParaValue )
    {
        /* 调用TL函数处理 */
        return AT_SetTlRficSsiRdPara( ucIndex );
    }

    ulLength = sizeof(AT_HPA_SSI_RD_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_SSI_RD_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetMipiRdPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue)
    {
        pstMsg->ulReceiverPid                   = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid                   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                             = ID_AT_HPA_SSI_RD_REQ;
    pstMsg->usChannelNo                         = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->uwRficReg                           = gastAtParaList[2].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetMipiRdPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt      = AT_CMD_SSI_RD;                /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetMipiRdPara(VOS_UINT8 ucIndex)
{
    AT_HPA_MIPI_RD_REQ_STRU             *pstMsg;
    VOS_UINT32                          ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (AT_RAT_MODE_FDD_LTE == gastAtParaList[0].ulParaValue || AT_RAT_MODE_TDD_LTE == gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

    ulLength = sizeof(AT_HPA_MIPI_RD_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_MIPI_RD_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetMipiRdPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue)
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                         = ID_AT_HPA_MIPI_RD_REQ;
    pstMsg->uhwChannel                      = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->uhwSlaveAddr                    = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->uhwReg                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetMipiRdPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_MIPI_RD;                   /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetMipiReadPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MIPI_READ_REQ_STRU           stMipiReadReq;
    VOS_UINT32                          ulResult;

    /*局部变量初始化*/
    TAF_MEM_SET_S(&stMipiReadReq, (VOS_SIZE_T)sizeof(stMipiReadReq), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_MIPI_READ_REQ_STRU));

    /*参数检查*/
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*参数数目不正确*/
    if (7 != gucAtParaIndex )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*不是非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*参数长度检查*/
    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen)
      || (0 == gastAtParaList[4].usParaLen)
      || (0 == gastAtParaList[5].usParaLen)
      || (0 == gastAtParaList[6].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*填写消息参数*/
    stMipiReadReq.usReadType    = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stMipiReadReq.usMipiId      = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stMipiReadReq.usSlaveId     = (VOS_UINT16)gastAtParaList[2].ulParaValue;
    stMipiReadReq.usRegAddr     = (VOS_UINT16)gastAtParaList[3].ulParaValue;
    stMipiReadReq.usSpeedType   = (VOS_UINT16)gastAtParaList[4].ulParaValue;
    stMipiReadReq.usReadBitMask = (VOS_UINT16)gastAtParaList[5].ulParaValue;
    stMipiReadReq.usReserved1   = (VOS_UINT16)gastAtParaList[6].ulParaValue;


    /*发送消息给MTA*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MIPIREAD_SET_REQ,
                                      &stMipiReadReq,
                                      (VOS_SIZE_T)sizeof(stMipiReadReq),
                                      I0_UEPS_PID_MTA);
    /*发送失败*/
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetMipiReadPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /*发送成功，设置当前操作模式*/
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPIREAD_SET;

    /*等待异步处理时间返回*/
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetPhyMipiWritePara(VOS_UINT8 ucIndex)
{
    AT_MTA_PHY_MIPI_WRITE_REQ_STRU      stPhyMipiWriteReq;
    VOS_UINT32                          ulResult;

    /*局部变量初始化*/
    TAF_MEM_SET_S(&stPhyMipiWriteReq, (VOS_SIZE_T)sizeof(stPhyMipiWriteReq), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_PHY_MIPI_WRITE_REQ_STRU));

    /*参数检查*/
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*参数数目不正确*/
    if (6 != gucAtParaIndex )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*不是非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*参数长度检查*/
    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen)
      || (0 == gastAtParaList[4].usParaLen)
      || (0 == gastAtParaList[5].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*填写消息参数*/
    stPhyMipiWriteReq.usWriteType   = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stPhyMipiWriteReq.usMipiId      = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stPhyMipiWriteReq.usSlaveId     = (VOS_UINT16)gastAtParaList[2].ulParaValue;
    stPhyMipiWriteReq.usRegAddr     = (VOS_UINT16)gastAtParaList[3].ulParaValue;
    stPhyMipiWriteReq.usMipiData    = (VOS_UINT16)gastAtParaList[4].ulParaValue;
    stPhyMipiWriteReq.usReserved1   = (VOS_UINT16)gastAtParaList[5].ulParaValue;

    /*发送消息给MTA*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_PHYMIPIWRITE_SET_REQ,
                                      &stPhyMipiWriteReq,
                                      (VOS_SIZE_T)sizeof(stPhyMipiWriteReq),
                                      I0_UEPS_PID_MTA);
    /*发送失败*/
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetPhyMipiWritePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /*发送成功，设置当前操作模式*/
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PHYMIPIWRITE_SET;

    /*等待异步处理时间返回*/
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32  At_SetCltPara(VOS_UINT8 ucIndex)
{
    /* 状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不符合要求 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    g_stAtDevCmdCtrl.enCltEnableFlg = (AT_DSP_CLT_ENABLE_ENUM_UINT8) gastAtParaList[0].ulParaValue;

    return AT_OK;    /* 返回命令处理处理成功 */

}


VOS_UINT32 AT_SetPdmCtrlPara(VOS_UINT8 ucIndex)
{
    AT_HPA_PDM_CTRL_REQ_STRU                *pstMsg;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if ( 4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }
    /*lint -save -e516 */
    pstMsg   = (AT_HPA_PDM_CTRL_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR( sizeof(AT_HPA_PDM_CTRL_REQ_STRU) );
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetPdmCtrlPara: alloc msg fail!");
        return AT_ERROR;
    }

    /* 填写消息头 */
    AT_CFG_MSG_HDR( pstMsg, DSP_PID_WPHY, ID_AT_HPA_PDM_CTRL_REQ );

    pstMsg->usMsgID                             = ID_AT_HPA_PDM_CTRL_REQ;
    pstMsg->usRsv                               = 0;
    pstMsg->usPdmRegValue                       = ( VOS_UINT16 )gastAtParaList[0].ulParaValue;
    pstMsg->usPaVbias                           = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usPaVbias2                          = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usPaVbias3                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetPdmCtrlPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt      = AT_CMD_PDM_CTRL;                /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}

/*****************************************************************************
  4 查询函数实现
*****************************************************************************/


VOS_UINT32  At_QryFChanPara(VOS_UINT8 ucIndex )
{
    TAF_UINT16                 usLength;

#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFCHANPara(ucIndex);
    }
#endif

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前FCHAN的设置 */
    usLength =  (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%d,%d,%d,%d",
                                   g_stAtDevCmdCtrl.ucDeviceRatMode,
                                   g_stAtDevCmdCtrl.ucDeviceAtBand,
                                   g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn,
                                   g_stAtDevCmdCtrl.stDspBandArfcn.usDlArfcn);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFTxonPara(VOS_UINT8 ucIndex )
{
    TAF_UINT16                 usLength;

    /*添加 LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFTXONPara(ucIndex);
    }
#endif

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }
    /* 查询当前DAC的设置 */
    usLength =  (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%d",
                                   g_stAtDevCmdCtrl.ucTxOnOff);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFRxonPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    /* 添加LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFRXONPara(ucIndex);
    }
#endif

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前接收机开关状态 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stAtDevCmdCtrl.ucRxOnOff);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 AT_QryFpowdetTPara(VOS_UINT8 ucIndex)
{
    AT_PHY_POWER_DET_REQ_STRU          *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIsLteFlg;

    ucIsLteFlg = VOS_FALSE;

    /*判断当前接入模式，只支持W*/
    if ( (AT_RAT_MODE_WCDMA != g_stAtDevCmdCtrl.ucDeviceRatMode)
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
      && (AT_RAT_MODE_CDMA != g_stAtDevCmdCtrl.ucDeviceRatMode)
#endif
      && (AT_RAT_MODE_GSM != g_stAtDevCmdCtrl.ucDeviceRatMode)
      && (AT_RAT_MODE_FDD_LTE != g_stAtDevCmdCtrl.ucDeviceRatMode)
      && (AT_RAT_MODE_TDD_LTE != g_stAtDevCmdCtrl.ucDeviceRatMode) )
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 申请AT_PHY_POWER_DET_REQ_STRU消息 */
    ulLength = sizeof(AT_PHY_POWER_DET_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    pstMsg   = (AT_PHY_POWER_DET_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_QryFpowdetTPara: Alloc msg fail!");
        return AT_ERROR;
    }

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    /* CDMA的话，发送给UPHY_PID_CSDR_1X_CM */
    if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        pstMsg->ulReceiverPid = UPHY_PID_CSDR_1X_CM;
    }
    else
#endif
    if (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }
    else
    {
        ucIsLteFlg = VOS_TRUE;
    }

    if (VOS_FALSE == ucIsLteFlg)
    {
        pstMsg->usMsgID       = ID_AT_PHY_POWER_DET_REQ;
        pstMsg->usRsv         = 0;

        /* 向对应PHY发送消息 */
        ulResult = PS_SEND_MSG(WUEPS_PID_AT, pstMsg);
    }
    else
    {
        /*lint --e{516,830} */
        PS_FREE_MSG(WUEPS_PID_AT, pstMsg);
        ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                          gastAtClientTab[ucIndex].opId,
                                          ID_AT_MTA_POWER_DET_QRY_REQ,
                                          VOS_NULL_PTR,
                                          0,
                                          I0_UEPS_PID_MTA);
    }

    if (VOS_OK != ulResult)
    {
        AT_WARN_LOG("AT_QryFpowdetTPara: Send msg fail!");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPOWDET_QRY;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryFPllStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulReceiverPid;
    AT_PHY_RF_PLL_STATUS_REQ_STRU      *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMsgId;
#if (FEATURE_ON==FEATURE_LTE)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
            ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFPllStatusPara(ucIndex);
    }
#endif
    /*判断当前接入模式，只支持G/W*/
    if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
        usMsgId       = ID_AT_WPHY_RF_PLL_STATUS_REQ;
    }
    else if ( (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
            ||(AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode) )
    {
        ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
        usMsgId       = ID_AT_GPHY_RF_PLL_STATUS_REQ;
    }

    else
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 申请AT_PHY_RF_PLL_STATUS_REQ_STRU消息 */
    ulLength = sizeof(AT_PHY_RF_PLL_STATUS_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    pstMsg   = (AT_PHY_RF_PLL_STATUS_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_QryFPllStatusPara: Alloc msg fail!");
        return AT_ERROR;
    }

    /* 填充消息 */
    pstMsg->ulReceiverPid = ulReceiverPid;
    pstMsg->usMsgID       = usMsgId;
    pstMsg->usRsv1        = 0;
    pstMsg->usDspBand     = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->usRsv2        = 0;

    /* 向对应PHY发送消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_QryFPllStatusPara: Send msg fail!");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPLLSTATUS_QRY;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryFrssiPara(
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_RX_RSSI_REQ_STRU          *pstMsg;
    VOS_UINT32                          ulLength;

#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFRSSIPara(ucIndex);
    }
#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atQryFRSSIPara(ucIndex);
    }

#endif

    /*该命令需在非信令模式下使用*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*该命令需在设置非信令信道后使用*/
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /*该命令需要在打开接收机后使用*/
    if (AT_DSP_RF_SWITCH_OFF == g_stAtDevCmdCtrl.ucRxOnOff)
    {
        return AT_FRSSI_RX_NOT_OPEN;
    }

    /* GDSP LOAD的情况下不支持接收机和发射机同时打开，需要判断最近一次执行的是打开接收机操作
    还是打开发射机操作，如果是打开发射机操作，则直接返回出错无需和GDSP 交互 */
    if ((AT_TXON_OPEN == g_stAtDevCmdCtrl.ucRxonOrTxon)
     && ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
      || (AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode)))
    {
        return AT_FRSSI_OTHER_ERR;
    }

    /* 申请AT_HPA_RF_RX_RSSI_REQ_STRU消息 */
    ulLength = sizeof(AT_HPA_RF_RX_RSSI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    pstMsg   = (AT_HPA_RF_RX_RSSI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_QryFrssiPara: alloc msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    if ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
        pstMsg->usMsgID       = ID_AT_GHPA_RF_RX_RSSI_REQ;
    }
    else
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
        pstMsg->usMsgID       = ID_AT_HPA_RF_RX_RSSI_REQ;
    }

    pstMsg->usMeasNum  = AT_DSP_RSSI_MEASURE_NUM;
    pstMsg->usInterval = AT_DSP_RSSI_MEASURE_INTERVAL;
    pstMsg->usRsv      = 0;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_QryFrssiPara: Send msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_QUERY_RSSI;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */

}


VOS_UINT32 AT_QryTSelRfPara(VOS_UINT8 ucIndex)
{

#if(FEATURE_LTE == FEATURE_ON)
    return atQryTselrfPara(ucIndex);
#else
    VOS_UINT16                          usLength;

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TOTAL,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_GSM,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WCDMA_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WCDMA_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WIFI);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
#endif

}


VOS_UINT32  At_QryFpaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前发射机PA等级的设置 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stAtDevCmdCtrl.ucPaLevel);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFlnaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    /* 添加 LTE 模的接口分支 */
#if(FEATURE_LTE == FEATURE_ON)
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFLNAPara(ucIndex);
    }
#endif

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前发射机PA等级的设置 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stAtDevCmdCtrl.ucLnaLevel);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}

/*****************************************************************************
 函 数 名  : AT_QryDcxotempcompPara
 功能描述  : ^DCXOTEMPCOMP的查询命令处理函数
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32  AT_QryDcxotempcompPara(VOS_UINT8 ucIndex)
{

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    g_stAtDevCmdCtrl.enDcxoTempCompEnableFlg);

    return AT_OK;
}


VOS_UINT32  AT_QryFDac(VOS_UINT8 ucIndex )
{
    TAF_UINT16                 usLength;

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前DAC的设置 */
    usLength =  (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%d",
                                   g_stAtDevCmdCtrl.usFDAC);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32  At_QryCltInfo(VOS_UINT8 ucIndex)
{
    VOS_UINT16                 usLength;

    usLength = 0;

    /* 如果记录有效，则上报查询结果 */
    if (VOS_TRUE == g_stAtDevCmdCtrl.stCltInfo.ulInfoAvailableFlg)
    {

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                            "^CLTINFO: ",
                                            g_stAtDevCmdCtrl.stCltInfo.shwGammaReal,          /* 反射系数实部 */
                                            g_stAtDevCmdCtrl.stCltInfo.shwGammaImag,          /* 反射系数虚部 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc0,       /* 驻波检测场景0反射系数幅度 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc1,       /* 驻波检测场景1反射系数幅度 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc2,       /* 驻波检测场景2反射系数幅度 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAntCoarseTune,/* 粗调格点位置 */
                                            g_stAtDevCmdCtrl.stCltInfo.ulwFomcoarseTune,      /* 粗调FOM值 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwCltAlgState,       /* 闭环算法收敛状态 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwCltDetectCount,    /* 闭环收敛总步数 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac0,              /* DAC0 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac1,              /* DAC1 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac2,              /* DAC2 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac3);             /* DAC3 */

        /* 上报后本地记录就无效 */
        g_stAtDevCmdCtrl.stCltInfo.ulInfoAvailableFlg = VOS_FALSE;
    }

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 At_TestFdacPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength  = 0;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: (0-2047)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: (0-1023)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


/*****************************************************************************
  5 其它旧函数实现
*****************************************************************************/


VOS_UINT8 At_GetDspLoadMode(VOS_UINT32 ulRatMode)
{
    if ((AT_RAT_MODE_WCDMA == ulRatMode)
     || (AT_RAT_MODE_AWS == ulRatMode))
    {
        return VOS_RATMODE_WCDMA;
    }
    else if ((AT_RAT_MODE_GSM == ulRatMode)
          || (AT_RAT_MODE_EDGE == ulRatMode))
    {
        return VOS_RATMODE_GSM;
    }
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
    else if(AT_RAT_MODE_CDMA == ulRatMode)
    {
        return VOS_RATMODE_1X;
    }
#endif
    else
    {
        return VOS_RATMODE_BUTT;
    }

}


VOS_VOID AT_GetTseLrfLoadDspInfo(
    AT_TSELRF_PATH_ENUM_UINT32          enPath,
    VOS_BOOL                           *pbLoadDsp,
    DRV_AGENT_TSELRF_SET_REQ_STRU      *pstTseLrf
)
{
    /* ^TSELRF命令设置的射频通路编号为GSM且当前已经LOAD了该通路，无需LOAD */
    if (AT_TSELRF_PATH_GSM == enPath)
    {
        if ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
         && (VOS_TRUE == g_stAtDevCmdCtrl.bDspLoadFlag))
        {
            *pbLoadDsp = VOS_FALSE;
        }
        else
        {
            pstTseLrf->ucLoadDspMode     = VOS_RATMODE_GSM;
            pstTseLrf->ucDeviceRatMode   = AT_RAT_MODE_GSM;
            *pbLoadDsp                   = VOS_TRUE;
        }
        return;
    }

    /* ^TSELRF命令设置的射频通路编号为WCDMA主集且当前已经LOAD了该通路，无需LOAD */
    if (AT_TSELRF_PATH_WCDMA_PRI == enPath)
    {
        if (((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
          || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
         && (VOS_TRUE == g_stAtDevCmdCtrl.bDspLoadFlag))
        {
            *pbLoadDsp = VOS_FALSE;
        }
        else
        {
            pstTseLrf->ucLoadDspMode     = VOS_RATMODE_WCDMA;
            pstTseLrf->ucDeviceRatMode   = AT_RAT_MODE_WCDMA;
            *pbLoadDsp                   = VOS_TRUE;
        }
        return;
    }

    *pbLoadDsp = VOS_FALSE;

    AT_WARN_LOG("AT_GetTseLrfLoadDspInfo: enPath only support GSM or WCDMA primary.");

    return;
}


VOS_UINT32 AT_SetGlobalFchan(VOS_UINT8 ucRATMode)
{
    g_stAtDevCmdCtrl.ucDeviceRatMode = ucRATMode;

    return VOS_OK;
}


VOS_UINT32 AT_SetTlRficSsiRdPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;
    AT_MTA_RFICSSIRD_REQ_STRU           stRficSsiRdReq;

    /* 通道检查 */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    {
        return AT_ERROR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(&stRficSsiRdReq, sizeof(stRficSsiRdReq), 0x00, sizeof(stRficSsiRdReq));

    stRficSsiRdReq.usChannelNo            = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    stRficSsiRdReq.usRficReg              = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;

    /* 发送消息 */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_RFICSSIRD_QRY_REQ,
                                      &stRficSsiRdReq,
                                      sizeof(stRficSsiRdReq),
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetTlRficSsiRdPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_RFICSSIRD_SET;
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;
}

#else
/*****************************************************************************
  6 新SET函数实现
*****************************************************************************/


VOS_UINT32  At_SetFChanPara(VOS_UINT8 ucIndex )
{
    AT_PR_LOGH("At_SetFChanPara Enter");
    
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 这条命令是每个band测试的第一条命令，将除当前运行模式外的信息清除 */
    TAF_MEM_SET_S(&g_stMtInfoCtx.stAtInfo  , sizeof(g_stMtInfoCtx.stAtInfo)  , 0, sizeof(AT_MT_AT_INFO_STRU));
    TAF_MEM_SET_S(&g_stMtInfoCtx.stBbicInfo, sizeof(g_stMtInfoCtx.stBbicInfo), 0, sizeof(AT_MT_BBIC_INFO_STRU));

    /* WIFI模式 */
    if (AT_RAT_MODE_WIFI == gastAtParaList[0].ulParaValue)
    {
        return At_SetFChanWifiProc(gastAtParaList[1].ulParaValue);
    }

    /* 当前模式判断 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 参数不符合要求,支持NR的版本中，一定要配置Band width,
    目前参数包括必带参数模式，Band, 信道号和带宽和一个可选参数子载波间隔 */
    if (gucAtParaIndex < 4)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[1].ulParaValue >= AT_PROTOCOL_BAND_BUTT)
    {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    /* 记录band信息, 要求工具下发就按协议band号下发 */
    g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand = (AT_PROTOCOL_BAND_ENUM_UINT16)gastAtParaList[1].ulParaValue;

    /* 记录测试接入技术模式信息 */
    if (VOS_FALSE == At_CovertAtModeToBbicCal((AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)gastAtParaList[0].ulParaValue,
                                              &g_stMtInfoCtx.stBbicInfo.enCurrtRatMode))
    {
        return AT_FCHAN_RAT_ERR;
    }

    At_CovertAtModeToLoadDspMode((AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)gastAtParaList[0].ulParaValue,
                                  &g_stMtInfoCtx.stBbicInfo.enLoadDspMode);

    g_stMtInfoCtx.stAtInfo.enRatMode =(AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 获取频率信息 */
    if (VOS_FALSE == AT_GetFreq(gastAtParaList[2].ulParaValue))
    {
        return AT_DEVICE_CHAN_BAND_CHAN_NOT_MAP;
    }

    /* 记录带宽信息 */
    if (VOS_FALSE == At_CovertAtBandWidthToBbicCal((AT_BAND_WIDTH_ENUM_UINT16)gastAtParaList[3].ulParaValue,
                                                    &g_stMtInfoCtx.stBbicInfo.enBandWidth))
    {
        return AT_FCHAN_BAND_WIDTH_ERR;
    }

    g_stMtInfoCtx.stAtInfo.enBandWidth = (AT_BAND_WIDTH_ENUM_UINT16)gastAtParaList[3].ulParaValue;

    /* 将AT的带宽转换成真正的值记录下来 */
    At_CovertAtBandWidthToValue((AT_BAND_WIDTH_ENUM_UINT16)gastAtParaList[3].ulParaValue,
                                &g_stMtInfoCtx.stBbicInfo.enBandWidthValue);

    if (AT_RAT_MODE_NR == g_stMtInfoCtx.stAtInfo.enRatMode)
    {
        if (0 == gastAtParaList[4].usParaLen)
        {
            return AT_FCHAN_NO_SCS;
        }

        if (VOS_FALSE == At_CovertAtScsToBbicCal((AT_SUB_CARRIER_SPACING_ENUM_UINT8)gastAtParaList[4].ulParaValue,
                                                  (VOS_UINT8*)&g_stMtInfoCtx.stBbicInfo.enBbicScs))
        {
            return AT_FCHAN_SCS_ERR;
        }

        g_stMtInfoCtx.stAtInfo.enBbicScs = (AT_SUB_CARRIER_SPACING_ENUM_UINT8)gastAtParaList[4].ulParaValue;
    }

    /* 记录当前client */
    g_stMtInfoCtx.stAtInfo.ucIndex = ucIndex;

    /* 给CBT发消息，通知其加载PHY */
    if (VOS_TRUE == At_LoadPhy())
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FCHAN_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_SND_MSG_FAIL;
}


VOS_UINT32 AT_SetFwavePara(VOS_UINT8 ucIndex)
{
    VOS_CHAR                        acCmd[200] = {0};

    AT_PR_LOGH("AT_SetFwavePara Enter");

    /* 状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确，必须包括波形类型和波形功率 */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* WIFI*/
    if (AT_RAT_MODE_WIFI == g_stMtInfoCtx.stAtInfo.enRatMode)
    {
        /*WIFI未Enable直接返回失败*/
        if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
        {
            return AT_WIFI_NOT_ENABLE;
        }

        /*向WIFI发送单音波形信号请求*/
        VOS_sprintf_s(acCmd, sizeof(acCmd), "athtestcmd -ieth0 --tx sine --txpwr %d", gastAtParaList[1].ulParaValue/100);

        WIFI_TEST_CMD(acCmd);

        return AT_OK;
    }

    /* 是否加载了DSP */
    if (VOS_TRUE != g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_NOT_LOAD_DSP;
    }

    /* 根据接入技术判断波形类型是否支持 */
    if (VOS_FALSE == AT_CheckFwaveTypePara(gastAtParaList[0].ulParaValue))
    {
        return AT_FWAVE_TYPE_ERR;
    }

    /* 转换TYPE 参数 */
    AT_CovertAtFwaveTypeToBbicCal((AT_FWAVE_TYPE_ENUM_UINT8)gastAtParaList[0].ulParaValue, &g_stMtInfoCtx.stBbicInfo.enFwaveType);

    /* MT工位工具下发的单位是0.01dB,  GUC物理层使用的单位是0.1db,LTE和NR使用的是0.125dB(X / 10 * (4 / 5)) */
    if ((RAT_MODE_LTE == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
     || (RAT_MODE_NR == g_stMtInfoCtx.stBbicInfo.enCurrtRatMode))
    {
        g_stMtInfoCtx.stBbicInfo.sFwavePower = (VOS_INT16)(gastAtParaList[1].ulParaValue * 2 / 25 );
    }
    else
    {
        g_stMtInfoCtx.stBbicInfo.sFwavePower = (VOS_INT16)(gastAtParaList[1].ulParaValue / 10);
    }

    return AT_OK;
}


VOS_UINT32  At_SetFTxonPara(VOS_UINT8 ucIndex )
{
    AT_PR_LOGH("At_SetFTxonPara Enter");
    
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FTXON_OTHER_ERR;
    }
    /* 参数不符合要求 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[0].ulParaValue >= AT_DSP_RF_SWITCH_BUTT)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 当前是否是FTM模式 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 检查是否加载了DSP */
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_NOT_LOAD_DSP;
    }

    /* 检查是否设置了path */
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bSetTxTselrfFlag)
    {
        return AT_NOT_SET_PATH;
    }

    /* 记录全局变量 */
    g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff = (AT_DSP_RF_ON_OFF_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 给BBIC发消息 */
    if (VOS_TRUE != At_SndTxOnOffReq(VOS_FALSE))
    {
        return AT_SND_MSG_FAIL;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FTXON;
    g_stMtInfoCtx.stAtInfo.ucIndex         = ucIndex;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
}


VOS_UINT32  At_SetFRxonPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("At_SetFRxonPara Enter");
    
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 该AT命令在AT^TMODE=1非信令模式下使用 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该AT命令需要在AT^FCHAN设置非信令信道命令执行成功后使用 */
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* 该AT命令需要在AT^TSELRF设置非信令信道命令执行成功后使用 */
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bSetRxTselrfFlag)
    {
        return AT_NOT_SET_PATH;
    }

    g_stMtInfoCtx.stAtInfo.enTempRxorTxOnOff = (AT_DSP_RF_ON_OFF_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 给BBIC CAL发消息 */
    if (VOS_FALSE == At_SndRxOnOffReq())
    {
        return AT_SND_MSG_FAIL;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_SET_FRXON;
    g_stMtInfoCtx.stAtInfo.ucIndex          = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;

}


VOS_UINT32 AT_SetMipiOpeRatePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    ulResult = AT_FAILURE;

    AT_PR_LOGH("AT_SetMipiOpeRatePara Enter");

    /* 状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不符合要求 */
    if (5 > gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (0 == gastAtParaList[0].ulParaValue)
    {
        if (AT_MT_MIPI_READ_MAX_BYTE >= gastAtParaList[4].ulParaValue)
        {
            /* 由于底层不支持设置读取速度，则最后一个参数忽略了 */
            ulResult = AT_SndBbicCalMipiReadReq(gastAtParaList[1].ulParaValue,
                                                gastAtParaList[2].ulParaValue,
                                                gastAtParaList[3].ulParaValue,
                                                gastAtParaList[4].ulParaValue);
        }
    }
    else
    {
        if (0 == gastAtParaList[5].usParaLen)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* MIPI写的时候，默认只写一个值 */
        gastAtParaList[4].ulParaValue = 1;
        ulResult = AT_SndBbicCalMipiWriteReq(gastAtParaList[1].ulParaValue,
                                             gastAtParaList[2].ulParaValue,
                                             gastAtParaList[3].ulParaValue,
                                             gastAtParaList[4].ulParaValue,
                                             gastAtParaList[5].ulParaValue);
    }

    /* 发送失败 */
    if (AT_SUCCESS != ulResult)
    {
        AT_ERR_LOG("AT_SetMipiOpeRatePara: AT Snd ReqMsg fail.");
        return AT_SND_MSG_FAIL;
    }

    /* 发送成功，设置当前操作模式 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPIOPERATE_SET;
    g_stMtInfoCtx.stAtInfo.ucIndex         = ucIndex;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetFAgcgainPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("AT_SetFAgcgainPara Enter");
    
    /* 状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用 */
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_NOT_LOAD_DSP;
    }

    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_stMtInfoCtx.stAtInfo.ucAgcGainLevel = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;
}


VOS_UINT32  AT_ProcTSelRfWifiPara(VOS_VOID)
{
    if ( BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        /*WIFI未Enable直接返回失败*/
        if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
        {
            return AT_WIFI_NOT_ENABLE;
        }

        g_stMtInfoCtx.stAtInfo.enRatMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    }
    else
    {
        return AT_NOT_SUPPORT_WIFI;
    }
}


VOS_UINT32 AT_CheckMimoPara(VOS_VOID)
{
    if (0 == gastAtParaList[1].ulParaValue)
    {
        if ((AT_MIMO_TYPE_2 != gastAtParaList[2].ulParaValue)
         || (gastAtParaList[3].ulParaValue > AT_MIMO_ANT_NUM_2)) 
        {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO ERR");
            return AT_FAILURE;
        }

        return AT_SUCCESS;
    }
    else 
    {
        if ((AT_MIMO_TYPE_2 == gastAtParaList[2].ulParaValue)
         && (gastAtParaList[3].ulParaValue > AT_MIMO_ANT_NUM_2)) 
        {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO_2 ERR");
            return AT_FAILURE;
        }

        if ((AT_MIMO_TYPE_4 == gastAtParaList[2].ulParaValue)
         && (gastAtParaList[3].ulParaValue > AT_MIMO_ANT_NUM_4)) 
        {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO_4 ERR");
            return AT_FAILURE;
        }

        if ((AT_MIMO_TYPE_8 == gastAtParaList[2].ulParaValue)
         && (gastAtParaList[3].ulParaValue > AT_MIMO_ANT_NUM_8)) 
        {
            AT_PR_LOGH("AT_CheckMimoPara TX MIMO_8 ERR");
            return AT_FAILURE;
        }

        return AT_SUCCESS;         
    }
}



VOS_UINT32 AT_ProcTSelRfPara(AT_ANT_TYPE_ENUM_UINT8 enAntType)
{
    if (0 == gastAtParaList[1].ulParaValue)
    {
        /* 初始化 */
        g_stMtInfoCtx.stAtInfo.enTxMimoType     = AT_MIMO_TYPE_BUTT;
        g_stMtInfoCtx.stAtInfo.enTxMimoAntNum   = AT_MIMO_ANT_NUM_BUTT;
        g_stMtInfoCtx.stBbicInfo.enTxMimoType   = AT_MIMO_TYPE_BUTT;
        g_stMtInfoCtx.stBbicInfo.enTxMimoAntNum = AT_MIMO_ANT_NUM_BUTT;
        
        if (AT_ANT_TYPE_MIMO == enAntType)
        {
            if (AT_FAILURE == AT_CheckMimoPara())
            {
                return AT_MIMO_PARA_ERR;
            }

            g_stMtInfoCtx.stAtInfo.enTxMimoType     = (AT_MIMO_TYPE_UINT8)gastAtParaList[2].ulParaValue;
            g_stMtInfoCtx.stAtInfo.enTxMimoAntNum   = (AT_MIMO_ANT_NUM_ENUM_UINT8)gastAtParaList[3].ulParaValue;
            /* BBIC采用BIT表示 */
            g_stMtInfoCtx.stBbicInfo.enTxMimoType   = (VOS_UINT8)AT_SET_BIT32(g_stMtInfoCtx.stAtInfo.enTxMimoType);
            g_stMtInfoCtx.stBbicInfo.enTxMimoAntNum = (VOS_UINT8)AT_SET_BIT32(g_stMtInfoCtx.stAtInfo.enTxMimoAntNum - 1);
        }

        g_stMtInfoCtx.stBbicInfo.enTxAntType    = enAntType;
        g_stMtInfoCtx.stAtInfo.enTseLrfTxPath   = gastAtParaList[0].ulParaValue;
        g_stMtInfoCtx.stAtInfo.bSetTxTselrfFlag = VOS_TRUE;
    }
    else if (1 == gastAtParaList[1].ulParaValue)
    {
        /* 初始化 */
        g_stMtInfoCtx.stAtInfo.enRxMimoType     = AT_MIMO_TYPE_BUTT;
        g_stMtInfoCtx.stAtInfo.enRxMimoAntNum   = AT_MIMO_ANT_NUM_BUTT;
        g_stMtInfoCtx.stBbicInfo.enRxMimoType   = AT_MIMO_TYPE_BUTT;
        g_stMtInfoCtx.stBbicInfo.enRxMimoAntNum = AT_MIMO_ANT_NUM_BUTT;
        
        if (AT_ANT_TYPE_MIMO == enAntType)
        {
            if (AT_FAILURE == AT_CheckMimoPara())
            {
                return AT_MIMO_PARA_ERR;
            }

            g_stMtInfoCtx.stAtInfo.enRxMimoType     = (AT_MIMO_TYPE_UINT8)gastAtParaList[2].ulParaValue;
            g_stMtInfoCtx.stAtInfo.enRxMimoAntNum   = (AT_MIMO_ANT_NUM_ENUM_UINT8)gastAtParaList[3].ulParaValue;
            /* BBIC采用BIT表示 */
            g_stMtInfoCtx.stBbicInfo.enRxMimoType   = (VOS_UINT8)AT_SET_BIT32(g_stMtInfoCtx.stAtInfo.enRxMimoType);
            g_stMtInfoCtx.stBbicInfo.enRxMimoAntNum = (VOS_UINT8)AT_SET_BIT32(g_stMtInfoCtx.stAtInfo.enRxMimoAntNum - 1);
        }

        g_stMtInfoCtx.stBbicInfo.enRxAntType    = enAntType;
        g_stMtInfoCtx.stAtInfo.enTseLrfRxPath   = gastAtParaList[0].ulParaValue;
        g_stMtInfoCtx.stAtInfo.bSetRxTselrfFlag = VOS_TRUE;
    }
    else
    {
        return AT_ESELRF_TX_OR_RX_ERR;
    }

    return AT_OK;
}



VOS_UINT32 AT_SetTSelRfPara(VOS_UINT8 ucIndex)
{
    AT_ANT_TYPE_ENUM_UINT8              enAntType;

    AT_PR_LOGH("AT_SetTSelRfPara Enter");
    
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TSELRF_PATH_BUTT <= gastAtParaList[0].ulParaValue)
    {
        return AT_TSELRF_PATH_ERR;
    }

    if (AT_TSELRF_PATH_WIFI == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWifiPara();
    }

    /* 参数不符合要求,必须有两个参数 */
    if (2 > gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (VOS_TRUE != AT_CoverTselPathToPriOrDiv(gastAtParaList[0].ulParaValue, &enAntType))
    {
        return AT_TSELRF_PATH_ERR;
    }
    
    /* 检查MIMO的参数是否带下来 */
    if ((AT_ANT_TYPE_MIMO == enAntType)
     && ((0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen)))
    {
        return AT_MIMO_PARA_ERR;
    }

    return AT_ProcTSelRfPara(enAntType);
}


VOS_UINT32 At_SetFpaPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("At_SetFpaPara Enter");
    
    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FPA_OTHER_ERR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_FPA_OTHER_ERR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用,即^FCHAN成功执行后 */
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_NOT_LOAD_DSP;
    }

    if (VOS_FALSE == At_CovertAtPaLevelToBbicCal((AT_CMD_PALEVEL_ENUM_UINT8)gastAtParaList[0].ulParaValue, &g_stMtInfoCtx.stBbicInfo.enPaLevel))
    {
        return AT_FPA_LEVEL_ERR;
    }
    /* 把设置保存在本地变量 */
    g_stMtInfoCtx.stAtInfo.enPaLevel = (AT_CMD_PALEVEL_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;
}


VOS_UINT32 AT_SetDcxotempcompPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("AT_SetDcxotempcompPara Enter");
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (0 == gastAtParaList[0].ulParaValue)
    {
        g_stMtInfoCtx.stBbicInfo.enDcxoTempCompEnableFlg    = BBIC_DCXO_SET_DISABLE;
    }
    else
    {
        g_stMtInfoCtx.stBbicInfo.enDcxoTempCompEnableFlg    = BBIC_DCXO_SET_ENABLE;
    }

    if (VOS_FALSE == At_SndDcxoReq())
    {
        return AT_SND_MSG_FAIL;
    }
    
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DCXOTEMPCOMP_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_SetDpdtPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    ulRst = AT_ERROR;

    AT_PR_LOGH("At_SetDpdtPara Enter");

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD!= g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (3 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 记录测试接入技术模式信息 */
    if (VOS_FALSE == At_CovertRatModeToBbicCal((AT_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue, (VOS_UINT16*)&g_stMtInfoCtx.stBbicInfo.enDpdtRatMode))
    {
        return AT_DPDT_RAT_ERR;
    }

    /* AT发送DPDT设置消息至BBIC处理 */
    
    ulRst = AT_SndBbicCalSetDpdtReq(BBIC_DPDT_OPERTYPE_SET, gastAtParaList[1].ulParaValue, gastAtParaList[2].ulParaValue);

    if (AT_SUCCESS == ulRst)
    {
        g_stMtInfoCtx.stAtInfo.ucIndex = ucIndex;
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                  ulRst;

    AT_PR_LOGH("At_SetQryDpdtPara Enter");

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 记录测试接入技术模式信息 */
    if (VOS_FALSE == At_CovertRatModeToBbicCal((AT_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue, (VOS_UINT16*)&g_stMtInfoCtx.stBbicInfo.enDpdtRatMode))
    {
        return AT_DPDT_RAT_ERR;
    }

    /* AT发送DPDT查询消息至BBIC处理 */
    ulRst = AT_SndBbicCalSetDpdtReq(BBIC_DPDT_OPERTYPE_GET, 0, gastAtParaList[1].ulParaValue);

    if (AT_SUCCESS == ulRst)
    {
        g_stMtInfoCtx.stAtInfo.ucIndex = ucIndex;
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDTQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


/*****************************************************************************
  7 新查询函数实现
*****************************************************************************/

VOS_UINT32  At_QryFChanPara(VOS_UINT8 ucIndex )
{
    VOS_UINT16                          usLength;

    AT_PR_LOGH("At_QryFChanPara Enter");

    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前FCHAN的设置 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d,%d,%d,%d,%d",
                                       g_stMtInfoCtx.stAtInfo.enRatMode,
                                       g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand,
                                       g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo,
                                       g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo,
                                       g_stMtInfoCtx.stAtInfo.enBandWidth);

    if (AT_RAT_MODE_NR == g_stMtInfoCtx.stAtInfo.enRatMode)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       g_stMtInfoCtx.stAtInfo.enBbicScs);
    }
    
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFTxonPara(VOS_UINT8 ucIndex )
{
    VOS_UINT16                 usLength;
    
    AT_PR_LOGH("At_QryFTxonPara Enter");

    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stMtInfoCtx.stAtInfo.enTxOnOff);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFRxonPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    AT_PR_LOGH("At_QryFRxonPara Enter");

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前接收机开关状态 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stMtInfoCtx.stAtInfo.enRxOnOff);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 AT_QryFpowdetTPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("AT_QryFpowdetTPara Enter");
    
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 当前是否有有设置设置接入技术 */
    if (g_stMtInfoCtx.stBbicInfo.enCurrtRatMode >= RAT_MODE_BUTT)
    {
        return AT_ERROR;
    }

    /* 给BBIC发消息 */
    if (VOS_TRUE != At_SndTxOnOffReq(VOS_TRUE))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_FPOWDET_QRY;
    g_stMtInfoCtx.stAtInfo.ucIndex          = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryFAgcgainPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("AT_QryFAgcgainPara Enter");
    
     /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    g_stMtInfoCtx.stAtInfo.ucAgcGainLevel);

    return AT_OK;
}



VOS_UINT32 AT_QryFPllStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    ulResult = AT_FAILURE;

    AT_PR_LOGH("AT_QryFPllStatusPara Enter");

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VOS_TRUE != g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_ERROR;
    }

    ulResult = AT_SndBbicPllStatusReq();

    /* 发送失败 */
    if (AT_SUCCESS != ulResult)
    {
        AT_ERR_LOG("AT_QryFPllStatusPara: AT Snd ReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPLLSTATUS_QRY;
    g_stMtInfoCtx.stAtInfo.ucIndex         = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryFrssiPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    ulResult = AT_FAILURE;

    AT_PR_LOGH("At_QryFrssiPara Enter");

    /*该命令需在非信令模式下使用*/
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*该命令需在设置非信令信道后使用*/
    if (VOS_FALSE == g_stMtInfoCtx.stAtInfo.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /*该命令需要在打开接收机后使用*/
    if (AT_DSP_RF_SWITCH_ON != g_stMtInfoCtx.stAtInfo.enRxOnOff)
    {
        return AT_FRSSI_RX_NOT_OPEN;
    }

    ulResult = AT_SndBbicRssiReq();

    /* 发送失败 */
    if (AT_SUCCESS != ulResult)
    {
        AT_ERR_LOG("At_QryFrssiPara: AT Snd ReqMsg fail.");
        return AT_FRSSI_OTHER_ERR;
    }

    g_stMtInfoCtx.stAtInfo.ucIndex         = ucIndex;
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_QUERY_RSSI;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryTSelRfPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    AT_PR_LOGH("AT_QryTSelRfPara Enter");

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TOTAL_MT,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_GSM,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WCDMA_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WCDMA_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_CDMA_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_CDMA_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TD,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WIFI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_FDD_LTE_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_FDD_LTE_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_FDD_LTE_MIMO,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TDD_LTE_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TDD_LTE_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TDD_LTE_MIMO,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_NR_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_NR_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s: %d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_NR_MIMO);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}



VOS_UINT32  At_QryFpaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    AT_PR_LOGH("At_QryFpaPara Enter");

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前发射机PA等级的设置 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s: %d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stMtInfoCtx.stAtInfo.enPaLevel);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}



VOS_UINT32  AT_QryDcxotempcompPara(VOS_UINT8 ucIndex)
{
    AT_PR_LOGH("AT_QryDcxotempcompPara Enter");
    
    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stMtInfoCtx.enCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    g_stMtInfoCtx.stAtInfo.enDcxoTempCompEnableFlg);

    return AT_OK;
}


VOS_UINT32  AT_QryFtemprptPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    AT_PR_LOGH("AT_QryFtemprptPara Enter");

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 记录通道类型信息 */
    if (VOS_FALSE == At_CovertChannelTypeToBbicCal((AT_TEMP_CHANNEL_TYPE_ENUM_UINT16)gastAtParaList[0].ulParaValue,
                                                   &g_stMtInfoCtx.stBbicInfo.enCurrentChannelType))
    {
        return AT_ERROR;
    }

    /* AT发送Ftemprpt查询消息至BBIC处理 */
    ulRst = AT_SndBbicCalQryFtemprptReq((INT16)gastAtParaList[1].ulParaValue);

    if (AT_SUCCESS == ulRst)
    {
        g_stMtInfoCtx.stAtInfo.ucIndex = ucIndex;
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FTEMPRPT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}




/*****************************************************************************
  8 其它新函数实现
*****************************************************************************/


VOS_UINT32  At_CovertAtModeToBbicCal(
    AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8   enAtMode,
    RAT_MODE_ENUM_UINT16               *penBbicMode
)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;

    switch(enAtMode)
    {
        case AT_RAT_MODE_WCDMA:
        case AT_RAT_MODE_AWS:
            *penBbicMode = RAT_MODE_WCDMA;
            break;

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case AT_RAT_MODE_CDMA:
            *penBbicMode = RAT_MODE_CDMA;
            break;
#endif

        case AT_RAT_MODE_GSM:
        case AT_RAT_MODE_EDGE:
            *penBbicMode = RAT_MODE_GSM;
            break;

#if (FEATURE_LTE == FEATURE_ON)
        case AT_RAT_MODE_FDD_LTE:
        case AT_RAT_MODE_TDD_LTE:
            *penBbicMode = RAT_MODE_LTE;
            break;
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
        case AT_RAT_MODE_NR:
            *penBbicMode = RAT_MODE_NR;
            break;
#endif

        case AT_RAT_MODE_TDSCDMA: /* 目前带NR的芯片不支持TD */
        case AT_RAT_MODE_WIFI:
        default:
            *penBbicMode    = RAT_MODE_BUTT;
            ulResult        = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32  At_CovertAtModeToLoadDspMode(
    AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8   enAtMode,
    AT_LOAD_DSP_RATMODE_ENUM_UINT8     *penLoadMode
)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;

    switch(enAtMode)
    {
        case AT_RAT_MODE_WCDMA:
        case AT_RAT_MODE_AWS:
            *penLoadMode = AT_LOAD_DSP_RATMODE_WCDMA;
            break;

        case AT_RAT_MODE_CDMA:
            *penLoadMode = AT_LOAD_DSP_RATMODE_CDMA;
            break;

        case AT_RAT_MODE_GSM:
        case AT_RAT_MODE_EDGE:
            *penLoadMode = AT_LOAD_DSP_RATMODE_GSM;
            break;

        case AT_RAT_MODE_FDD_LTE:
        case AT_RAT_MODE_TDD_LTE:
            *penLoadMode = AT_LOAD_DSP_RATMODE_LTE;
            break;

        case AT_RAT_MODE_NR:
            *penLoadMode = AT_LOAD_DSP_RATMODE_NR;
            break;

        case AT_RAT_MODE_TDSCDMA: /* 目前带NR的芯片不支持TD */
        case AT_RAT_MODE_WIFI:
        default:
            *penLoadMode    = AT_LOAD_DSP_RATMODE_BUTT;
            ulResult        = VOS_FALSE;
            break;
    }

    return ulResult;
}



VOS_UINT32  At_CovertAtBandWidthToBbicCal(
    AT_BAND_WIDTH_ENUM_UINT16           enAtBandWidth,
    BANDWIDTH_ENUM_UINT16              *penBbicBandWidth
)
{
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulBandWidthNo;

    ulBandWidthNo       = sizeof(g_astBandWidthTable) / sizeof(g_astBandWidthTable[0]);
    *penBbicBandWidth   = BANDWIDTH_BUTT;

    for (ulIndex = 0; ulIndex < ulBandWidthNo; ulIndex++)
    {
        if (enAtBandWidth == g_astBandWidthTable[ulIndex].enAtBandWidth)
        {
            *penBbicBandWidth = g_astBandWidthTable[ulIndex].enBbicBandWidth;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_UINT32  At_CovertAtBandWidthToValue(
    AT_BAND_WIDTH_ENUM_UINT16           enAtBandWidth,
    AT_BAND_WIDTH_VALUE_ENUM_UINT32    *penBandWidthValue
)
{
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulBandWidthNo;

    ulBandWidthNo       = sizeof(g_astBandWidthTable) / sizeof(g_astBandWidthTable[0]);
    *penBandWidthValue  = AT_BAND_WIDTH_VALUE_BUTT;

    for (ulIndex = 0; ulIndex < ulBandWidthNo; ulIndex++)
    {
        if (enAtBandWidth == g_astBandWidthTable[ulIndex].enAtBandWidth)
        {
            *penBandWidthValue = g_astBandWidthTable[ulIndex].enAtBandWidthValue;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}



VOS_UINT32  At_CovertAtScsToBbicCal(
    AT_SUB_CARRIER_SPACING_ENUM_UINT8   enAtScs,
    NR_SCS_TYPE_COMM_ENUM_UINT8        *penBbicScs
)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;

    switch(enAtScs)
    {
        case AT_SUB_CARRIER_SPACING_15K:
            *penBbicScs = NR_SCS_TYPE_COMM_15;
            break;
        case AT_SUB_CARRIER_SPACING_30K:
            *penBbicScs = NR_SCS_TYPE_COMM_30;
            break;
        case AT_SUB_CARRIER_SPACING_60K:
            *penBbicScs = NR_SCS_TYPE_COMM_60;
            break;
        case AT_SUB_CARRIER_SPACING_120K:
            *penBbicScs = NR_SCS_TYPE_COMM_120;
            break;
        case AT_SUB_CARRIER_SPACING_240K:
            *penBbicScs = NR_SCS_TYPE_COMM_240;
            break;
        default:
            *penBbicScs = NR_SCS_TYPE_COMM_BUTT;
            ulResult    = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32  At_CovertRatModeToBbicCal(
    AT_CMD_RATMODE_ENUM_UINT8           enRatMode,
    RAT_MODE_ENUM_UINT16               *penBbicMode
)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;

    switch(enRatMode)
    {
        case AT_CMD_RATMODE_GSM:
            *penBbicMode = RAT_MODE_GSM;
            break;

        case AT_CMD_RATMODE_WCDMA:
            *penBbicMode = RAT_MODE_WCDMA;
            break;

        case AT_CMD_RATMODE_LTE:
            *penBbicMode = RAT_MODE_LTE;
            break;

        case AT_CMD_RATMODE_CDMA:
            *penBbicMode = RAT_MODE_CDMA;
            break;

        case AT_CMD_RATMODE_NR:
            *penBbicMode = RAT_MODE_NR;
            break;

        case AT_CMD_RATMODE_TD: /* 目前带NR的芯片不支持TD */
        default:
            *penBbicMode    = RAT_MODE_BUTT;
            ulResult        = VOS_FALSE;
            break;
    }

    return ulResult;
}



VOS_UINT32  At_CovertChannelTypeToBbicCal(
    AT_TEMP_CHANNEL_TYPE_ENUM_UINT16        enChannelType,
    BBIC_TEMP_CHANNEL_TYPE_ENUM_UINT16     *penBbicChannelType
)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_TRUE;

    switch(enChannelType)
    {
        case AT_TEMP_CHANNEL_TYPE_LOGIC:
            *penBbicChannelType = BBIC_TEMP_CHANNEL_TYPE_LOGIC;
            break;

        case AT_TEMP_CHANNEL_TYPE_PHY:
            *penBbicChannelType = BBIC_TEMP_CHANNEL_TYPE_PHY;
            break;

        default:
            *penBbicChannelType = BBIC_TEMP_CHANNEL_TYPE_BUTT;
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}



VOS_UINT32  At_CovertAtPaLevelToBbicCal(
    AT_CMD_PALEVEL_ENUM_UINT8           enAtPaLevel,
    BBIC_CAL_PA_MODE_ENUM_UINT8        *penBbicPaLevel
)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;

    switch(enAtPaLevel)
    {
        case AT_CMD_PALEVEL_HIGH:
            *penBbicPaLevel = BBIC_CAL_PA_GAIN_MODE_HIGH;
            break;

        case AT_CMD_PALEVEL_MID:
            *penBbicPaLevel = BBIC_CAL_PA_GAIN_MODE_MID;
            break;

        case AT_CMD_PALEVEL_LOW:
            *penBbicPaLevel = BBIC_CAL_PA_GAIN_MODE_LOW;
            break;

        case AT_CMD_PALEVEL_ULTRALOW:
            *penBbicPaLevel = BBIC_CAL_PA_GAIN_MODE_ULTRA_LOW;
            break;

        default:
            *penBbicPaLevel = BBIC_CAL_PA_GAIN_MODE_BUTT;
            ulResult        = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32 AT_GetNrFreqOffset(
    VOS_UINT32                          ulChannelNo,
    AT_NR_FREQ_OFFSET_TABLE_STRU       *pstNrFreqOffset
)
{
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulFreqOffsetNo;

    ulFreqOffsetNo = sizeof(g_astAtNrFreqOffsetTable) / sizeof(AT_NR_FREQ_OFFSET_TABLE_STRU);

    for (ulIndex = 0; ulIndex < ulFreqOffsetNo; ulIndex++)
    {
        if ((ulChannelNo >= g_astAtNrFreqOffsetTable[ulIndex].stChannelRange.ulChannelMin)
         && (ulChannelNo <= g_astAtNrFreqOffsetTable[ulIndex].stChannelRange.ulChannelMax))
        {
            break;
        }
    }

    if (ulIndex == ulFreqOffsetNo)
    {
        return VOS_FALSE;
    }

    TAF_MEM_CPY_S(pstNrFreqOffset, sizeof(AT_NR_FREQ_OFFSET_TABLE_STRU),
                  &g_astAtNrFreqOffsetTable[ulIndex], sizeof(g_astAtNrFreqOffsetTable[ulIndex]));

    return VOS_TRUE;
}


VOS_VOID AT_GetNrFreqFromUlChannelNo(
    VOS_UINT32                          ulUlChannelNo,
    AT_NR_FREQ_OFFSET_TABLE_STRU       *pstNrFreqOffset,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_NR_BAND_INFO_STRU         *pstBandInfo
)
{
    VOS_INT32                           lDetlFreq;
    VOS_UINT32                          ulUlChanNoRange;
    VOS_UINT32                          ulDlChanNoRange;

    /* 上行频点 */
    pstDspBandFreq->ulUlFreq = pstNrFreqOffset->ulFreqOffset
                             + pstNrFreqOffset->ulFreqGlobal
                             * (ulUlChannelNo - pstNrFreqOffset->ulOffsetChannel);

    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulUlChannelNo;

    ulUlChanNoRange = pstBandInfo->stUlChannelRange.ulChannelMax
                    - pstBandInfo->stUlChannelRange.ulChannelMin;

    ulDlChanNoRange = pstBandInfo->stDlChannelRange.ulChannelMax
                    - pstBandInfo->stDlChannelRange.ulChannelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算下行频率 */
    if (ulUlChanNoRange != ulDlChanNoRange)
    {
        return;
    }

    /* 上下行的频点差 */
    lDetlFreq = (VOS_INT32)(pstBandInfo->stDlFreqRange.ulFreqMin
              - pstBandInfo->stUlFreqRange.ulFreqMin);

    /* 下行频点 */
    pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq + lDetlFreq;


    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = pstBandInfo->stDlChannelRange.ulChannelMin
                                                   + ulUlChannelNo
                                                   - pstBandInfo->stUlChannelRange.ulChannelMin;
    return;
}


VOS_VOID AT_GetNrFreqFromDlChannelNo(
    VOS_UINT32                          ulDlChannelNo,
    AT_NR_FREQ_OFFSET_TABLE_STRU       *pstNrFreqOffset,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_NR_BAND_INFO_STRU         *pstBandInfo
)
{
    VOS_INT32                           lDetlFreq;
    VOS_UINT32                          ulUlChanNoRange;
    VOS_UINT32                          ulDlChanNoRange;

    /* 下行频点 */
    pstDspBandFreq->ulDlFreq =  pstNrFreqOffset->ulFreqOffset
                             + pstNrFreqOffset->ulFreqGlobal
                             * (ulDlChannelNo - pstNrFreqOffset->ulOffsetChannel);
    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulDlChannelNo;


    ulUlChanNoRange = pstBandInfo->stUlChannelRange.ulChannelMax
                        - pstBandInfo->stUlChannelRange.ulChannelMin;

    ulDlChanNoRange = pstBandInfo->stDlChannelRange.ulChannelMax
                    - pstBandInfo->stDlChannelRange.ulChannelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算上行频率 */
    if (ulUlChanNoRange != ulDlChanNoRange)
    {
        return;
    }


    /* 相同信道上的频点差 */
    lDetlFreq = (VOS_INT32)(pstBandInfo->stDlFreqRange.ulFreqMin
                             - pstBandInfo->stUlFreqRange.ulFreqMin);

    /* 上行频点 */
    pstDspBandFreq->ulUlFreq = pstDspBandFreq->ulDlFreq - lDetlFreq;

    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = pstBandInfo->stUlChannelRange.ulChannelMin
                                                   + ulDlChannelNo
                                                   - pstBandInfo->stDlChannelRange.ulChannelMin;
    return;
}



VOS_UINT32  AT_GetNrFreq(
    VOS_UINT32                          ulChannelNo
)
{
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq  = VOS_NULL_PTR;
    AT_NR_FREQ_OFFSET_TABLE_STRU        stNrFreqOffset;
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulSupportBandNo;

    pstDspBandFreq  = &g_stMtInfoCtx.stBbicInfo.stDspBandFreq;
    ulSupportBandNo = sizeof(g_astAtNrBandInfoTable) / sizeof(AT_NR_BAND_INFO_STRU);

    for (ulIndex = 0; ulIndex < ulSupportBandNo; ulIndex++)
    {
        if (g_astAtNrBandInfoTable[ulIndex].usBandNo == g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand)
        {
            break;
        }
    }

    /* Band不匹配 */
    if (ulIndex == ulSupportBandNo)
    {
        return VOS_FALSE;
    }

    pstDspBandFreq->usDspBand = g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand;

    /* 获取频率的偏移量,单位KHZ */
    if (VOS_FALSE == AT_GetNrFreqOffset(ulChannelNo, &stNrFreqOffset))
    {
        return VOS_FALSE;
    }


    /* 判断工具下发的是否是上行信道号 */
    if ((ulChannelNo >= g_astAtNrBandInfoTable[ulIndex].stUlChannelRange.ulChannelMin)
     && (ulChannelNo <= g_astAtNrBandInfoTable[ulIndex].stUlChannelRange.ulChannelMax))
    {
        AT_GetNrFreqFromUlChannelNo(ulChannelNo,
                                    &stNrFreqOffset,
                                    pstDspBandFreq,
                                    &g_astAtNrBandInfoTable[ulIndex]);
    }
    /* 判断工具下发的是否是下行信道号 */
    else if ((ulChannelNo >= g_astAtNrBandInfoTable[ulIndex].stDlChannelRange.ulChannelMin)
          && (ulChannelNo <= g_astAtNrBandInfoTable[ulIndex].stDlChannelRange.ulChannelMax))
    {
        AT_GetNrFreqFromDlChannelNo(ulChannelNo,
                                    &stNrFreqOffset,
                                    pstDspBandFreq,
                                    &g_astAtNrBandInfoTable[ulIndex]);
    }
    else
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;


}


VOS_VOID AT_GetLteFreqFromUlChannelNo(
    VOS_UINT32                          ulUlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_LTE_BAND_INFO_STRU        *pstBandInfo
)
{
    VOS_UINT32                          ulDlChannelNo;
    VOS_UINT32                          ulUlChanNoRange;
    VOS_UINT32                          ulDlChanNoRange;

    pstDspBandFreq->ulUlFreq = pstBandInfo->ulUlLowFreq
                               + ulUlChannelNo * LTE_CHANNEL_TO_FREQ_UNIT
                               - pstBandInfo->ulUlChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;
    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulUlChannelNo;

    ulUlChanNoRange = pstBandInfo->stUlChannelRange.ulChannelMax
                    - pstBandInfo->stUlChannelRange.ulChannelMin;

    ulDlChanNoRange = pstBandInfo->stDlChannelRange.ulChannelMax
                    - pstBandInfo->stDlChannelRange.ulChannelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算下行频率 */
    if (ulUlChanNoRange != ulDlChanNoRange)
    {
        return;
    }

    ulDlChannelNo = pstBandInfo->stDlChannelRange.ulChannelMin
                                + ulUlChannelNo
                                - pstBandInfo->ulUlChannelOffset;

    pstDspBandFreq->ulDlFreq = pstBandInfo->ulDlLowFreq
                               + ulDlChannelNo  * LTE_CHANNEL_TO_FREQ_UNIT
                               - pstBandInfo->ulDlChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;


    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulDlChannelNo;

    return;
}


VOS_VOID AT_GetLteFreqFromDlChannelNo(
    VOS_UINT32                          ulDlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_LTE_BAND_INFO_STRU        *pstBandInfo
)
{
    VOS_UINT32                          ulUlChanNoRange;
    VOS_UINT32                          ulDlChanNoRange;
    VOS_UINT32                          ulUlChannelNo;

    pstDspBandFreq->ulDlFreq = pstBandInfo->ulDlLowFreq
                               + ulDlChannelNo * LTE_CHANNEL_TO_FREQ_UNIT
                               - pstBandInfo->ulDlChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;
    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulDlChannelNo;

    ulUlChanNoRange = pstBandInfo->stUlChannelRange.ulChannelMax
                    - pstBandInfo->stUlChannelRange.ulChannelMin;

    ulDlChanNoRange = pstBandInfo->stDlChannelRange.ulChannelMax
                    - pstBandInfo->stDlChannelRange.ulChannelMin;

    /* 判断上下行信道是否对称，如果不对称不能计算上行频率 */
    if (ulUlChanNoRange != ulDlChanNoRange)
    {
        return;
    }

    ulUlChannelNo = pstBandInfo->stUlChannelRange.ulChannelMin
                                + ulDlChannelNo
                                - pstBandInfo->ulDlChannelOffset;

    pstDspBandFreq->ulUlFreq = pstBandInfo->ulUlLowFreq
                               + ulUlChannelNo * LTE_CHANNEL_TO_FREQ_UNIT
                               - pstBandInfo->ulUlChannelOffset * LTE_CHANNEL_TO_FREQ_UNIT;

    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulUlChannelNo;


    return;
}



VOS_UINT32  AT_GetLteFreq(
    VOS_UINT32                          ulChannelNo
)
{
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulSupportBandNo;
    VOS_UINT32                          ulResult;

    ulResult        = VOS_TRUE;
    pstDspBandFreq  = &g_stMtInfoCtx.stBbicInfo.stDspBandFreq;
    ulSupportBandNo = sizeof(g_astAtLteBandInfoTable) / sizeof(AT_LTE_BAND_INFO_STRU);

    /* Band号查询 */
    for (ulIndex = 0; ulIndex < ulSupportBandNo; ulIndex++)
    {
        if (g_astAtLteBandInfoTable[ulIndex].usBandNo == g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand)
        {
            break;
        }
    }

    if (ulIndex == ulSupportBandNo)
    {
        return VOS_FALSE;
    }

    pstDspBandFreq->usDspBand = g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand;

    /* 判断工具下发的是否是上行信道号 */
    if ((ulChannelNo >= g_astAtLteBandInfoTable[ulIndex].stUlChannelRange.ulChannelMin)
     && (ulChannelNo <= g_astAtLteBandInfoTable[ulIndex].stUlChannelRange.ulChannelMax))
    {
        AT_GetLteFreqFromUlChannelNo(ulChannelNo, pstDspBandFreq, &g_astAtLteBandInfoTable[ulIndex]);
    }
    /* 判断工具下发的是否是下行信道号 */
    else if ((ulChannelNo >= g_astAtLteBandInfoTable[ulIndex].stDlChannelRange.ulChannelMin)
          && (ulChannelNo <= g_astAtLteBandInfoTable[ulIndex].stDlChannelRange.ulChannelMax))
    {
        AT_GetLteFreqFromDlChannelNo(ulChannelNo, pstDspBandFreq, &g_astAtLteBandInfoTable[ulIndex]);
    }
    else
    {
        ulResult        = VOS_FALSE;
    }

    return ulResult;
}


VOS_UINT32 AT_GetWFreqFromUlChannelNo(
    VOS_UINT32                          ulUlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_W_BAND_INFO_STRU          *pstBandInfo
)
{
    pstDspBandFreq->ulUlFreq = ulUlChannelNo * FREQ_UNIT_MHZ_TO_KHZ / 5
                               +  pstBandInfo->ulUlFreqOffset;

    pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq
                               + pstBandInfo->stDlFreqRange.ulFreqMin
                               - pstBandInfo->stUlFreqRange.ulFreqMin;

    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulUlChannelNo;
    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = pstBandInfo->stDlChannelRange.ulChannelMin
                                                   + ulUlChannelNo
                                                   - pstBandInfo->stUlChannelRange.ulChannelMin;

    return VOS_TRUE;
}


VOS_UINT32 AT_GetWFreqFromDlChannelNo(
    VOS_UINT32                          ulDlChannelNo,
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq,
    const AT_W_BAND_INFO_STRU          *pstBandInfo
)
{
    pstDspBandFreq->ulDlFreq = ulDlChannelNo * FREQ_UNIT_MHZ_TO_KHZ / 5
                               +  pstBandInfo->ulDlFreqOffset;

    pstDspBandFreq->ulUlFreq = pstDspBandFreq->ulDlFreq
                               + pstBandInfo->stUlFreqRange.ulFreqMin
                               - pstBandInfo->stDlFreqRange.ulFreqMin;

    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulDlChannelNo;
    g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = pstBandInfo->stUlChannelRange.ulChannelMin
                                                   + ulDlChannelNo
                                                   - pstBandInfo->stDlChannelRange.ulChannelMin;

    return VOS_TRUE;
}


VOS_UINT32  AT_GetWFreq(
    VOS_UINT32                          ulChannelNo
)
{
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulSupportBandNo;
    VOS_UINT32                          ulResult;


    ulResult        = VOS_TRUE;
    pstDspBandFreq  = &g_stMtInfoCtx.stBbicInfo.stDspBandFreq;
    ulSupportBandNo = sizeof(g_astAtWBandInfoTable) / sizeof(AT_W_BAND_INFO_STRU);

    /*  判断当前band是否是支持的band */
    for (ulIndex = 0; ulIndex < ulSupportBandNo; ulIndex++)
    {
        if (g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand == g_astAtWBandInfoTable[ulIndex].usBandNo)
        {
            break;
        }
    }

    if (ulIndex == ulSupportBandNo)
    {
        return VOS_FALSE;
    }

    pstDspBandFreq->usDspBand = g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand;

    /* 判断工具下发的是否是上行信道号 */
    if ((ulChannelNo >= g_astAtWBandInfoTable[ulIndex].stUlChannelRange.ulChannelMin)
     && (ulChannelNo <= g_astAtWBandInfoTable[ulIndex].stUlChannelRange.ulChannelMax))
    {
        AT_GetWFreqFromUlChannelNo(ulChannelNo, pstDspBandFreq, &g_astAtWBandInfoTable[ulIndex]);
    }
    /* 判断工具下发的是否是下行信道号 */
    else if ((ulChannelNo >= g_astAtWBandInfoTable[ulIndex].stDlChannelRange.ulChannelMin)
          && (ulChannelNo <= g_astAtWBandInfoTable[ulIndex].stDlChannelRange.ulChannelMax))
    {
        AT_GetWFreqFromDlChannelNo(ulChannelNo, pstDspBandFreq, &g_astAtWBandInfoTable[ulIndex]);
    }
    else
    {
        ulResult        = VOS_FALSE;
    }

    return ulResult;
}


VOS_UINT32  AT_GetGFreq(
    VOS_UINT32                          ulChannelNo
)
{
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq = VOS_NULL_PTR;
    VOS_UINT32                          ulIndex;
    VOS_UINT32                          ulSupportBandNo;

    pstDspBandFreq = &g_stMtInfoCtx.stBbicInfo.stDspBandFreq;

    ulSupportBandNo = sizeof(g_astAtGBandInfoTable) / sizeof(AT_G_BAND_INFO_STRU);

    /*  判断当前band是否是支持的band */
    for (ulIndex = 0; ulIndex < ulSupportBandNo; ulIndex++)
    {
        if (g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand == g_astAtGBandInfoTable[ulIndex].usBandNo)
        {
            break;
        }
    }

    if (ulIndex == ulSupportBandNo)
    {
        return VOS_FALSE;
    }

    pstDspBandFreq->usDspBand = g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand;

    /*  判断信道是否在范围内 */
    if ((ulChannelNo < g_astAtGBandInfoTable[ulIndex].stChannelRange.ulChannelMin)
     || (ulChannelNo > g_astAtGBandInfoTable[ulIndex].stChannelRange.ulChannelMax))
    {
        return VOS_FALSE;
    }

    /* GSM 900的信道范围比较多 */
    if (AT_PROTOCOL_BAND_8 == pstDspBandFreq->usDspBand)
    {
        /* Fu=890000 + 200*N */
        if (ulChannelNo <= G_CHANNEL_NO_124)
        {
            pstDspBandFreq->ulUlFreq = 890000 + 200 * ulChannelNo;
            pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq + g_astAtGBandInfoTable[ulIndex].ulFreqOffset;

            g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
            g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;

            return VOS_TRUE;
        }

        /* Fu=890000 + 200*(N - 1024) */
        if ((ulChannelNo >= G_CHANNEL_NO_955)
         && (ulChannelNo <= G_CHANNEL_NO_1023))
        {
            pstDspBandFreq->ulUlFreq = 890000 + 200 * ulChannelNo - 200 * 1024;
            pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq + g_astAtGBandInfoTable[ulIndex].ulFreqOffset;

            g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
            g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;

            return VOS_TRUE;
        }
    }

    /* GSM 1800的信道范围比较多 */
    if (AT_PROTOCOL_BAND_3 == pstDspBandFreq->usDspBand)
    {
        /* Fu=1710200 + 200*(N - 512) */
        pstDspBandFreq->ulUlFreq = 1710200 + 200 * ulChannelNo - 200 * 512;
        pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq + g_astAtGBandInfoTable[ulIndex].ulFreqOffset;

        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;
        return VOS_TRUE;
    }

    /* GSM 1900的信道范围比较多 */
    if (AT_PROTOCOL_BAND_2 == pstDspBandFreq->usDspBand)
    {
        /* Fu=1850200 + 200*(N - 512) */
        pstDspBandFreq->ulUlFreq = 1850200 + 200 * ulChannelNo - 200 * 512;
        pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq + g_astAtGBandInfoTable[ulIndex].ulFreqOffset;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;
        return VOS_TRUE;
    }


    /* GSM 850的信道范围比较多 */
    if (AT_PROTOCOL_BAND_5 == pstDspBandFreq->usDspBand)
    {
        /* Fu=824200 + 200*(N - 128) */
        pstDspBandFreq->ulUlFreq = 824200 + 200 * ulChannelNo - 200 * 128;
        pstDspBandFreq->ulDlFreq = pstDspBandFreq->ulUlFreq + g_astAtGBandInfoTable[ulIndex].ulFreqOffset;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32  AT_GetCFreq(
    VOS_UINT32                          ulChannelNo
)
{
    AT_DSP_BAND_FREQ_STRU              *pstDspBandFreq = VOS_NULL_PTR;

    pstDspBandFreq = &g_stMtInfoCtx.stBbicInfo.stDspBandFreq;

    /* 当前只支持Band Class 0,Band5 */
    if (AT_PROTOCOL_BAND_5 != g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand)
    {
        return VOS_FALSE;
    }

    pstDspBandFreq->usDspBand = g_stMtInfoCtx.stAtInfo.stBandArfcn.usDspBand;

    if ((ulChannelNo >= C_CHANNEL_NO_1)
     && (ulChannelNo <= C_CHANNEL_NO_799))
    {
        /* 计算公式: Fu = 30N + 825000*/
        pstDspBandFreq->ulUlFreq = 30 * ulChannelNo + 825000;

        /* 计算公式: Fd = 30N + 870000*/
        pstDspBandFreq->ulDlFreq = 30 * ulChannelNo + 870000;

        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;

        return VOS_TRUE;
    }

    if ((ulChannelNo >= C_CHANNEL_NO_991)
     && (ulChannelNo <= C_CHANNEL_NO_1023))
    {
        /* 计算公式: Fu = 30(N - 1023) + 825000*/
        pstDspBandFreq->ulUlFreq = 825000 - 30 * (1023 - ulChannelNo);

        /* 计算公式: Fd = 30(N -1023) + 870000*/
        pstDspBandFreq->ulDlFreq = 870000 - 30 * (1023 - ulChannelNo);

        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;
        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;

        return VOS_TRUE;
    }

    if ((ulChannelNo >= C_CHANNEL_NO_1024)
     && (ulChannelNo <= C_CHANNEL_NO_1323))
    {
        /* 计算公式: Fu = 30(N - 1024) + 815040*/
        pstDspBandFreq->ulUlFreq = 30 * (ulChannelNo - 1024) + 815040;

        /* 计算公式: Fd = 30(N -1024) + 860040*/
        pstDspBandFreq->ulDlFreq = 30 * (ulChannelNo - 1024) + 860040;

        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulDlChanNo = ulChannelNo;

        g_stMtInfoCtx.stAtInfo.stBandArfcn.ulUlChanNo = ulChannelNo;

        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32  AT_GetFreq(
    VOS_UINT32                          ulChannelNo
)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_FALSE;

    switch(g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
    {
        case RAT_MODE_GSM:
            ulResult = AT_GetGFreq(ulChannelNo);
            break;

        case RAT_MODE_WCDMA:
            ulResult = AT_GetWFreq(ulChannelNo);
            break;
        case RAT_MODE_CDMA:
            ulResult = AT_GetCFreq(ulChannelNo);
            break;

        case RAT_MODE_LTE:
            ulResult = AT_GetLteFreq(ulChannelNo);
            break;

        case RAT_MODE_NR:
            ulResult = AT_GetNrFreq(ulChannelNo);
            break;

        default:
            ulResult = VOS_FALSE;
            break;

    }

    return ulResult;
}


VOS_UINT32  AT_CheckNrFwaveTypePara(VOS_UINT32 ulPara)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_TRUE;
    switch(ulPara)
    {
        case AT_FWAVE_TYPE_BPSK:
        case AT_FWAVE_TYPE_PI2_BPSK:
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_16QAM:
        case AT_FWAVE_TYPE_64QAM:
        case AT_FWAVE_TYPE_256QAM:
        case AT_FWAVE_TYPE_CONTINUE:
            ulResult = VOS_TRUE;
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32  AT_CheckLteFwaveTypePara(VOS_UINT32 ulPara)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_TRUE;
    switch(ulPara)
    {
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_16QAM:
        case AT_FWAVE_TYPE_64QAM:
        case AT_FWAVE_TYPE_256QAM:
        case AT_FWAVE_TYPE_CONTINUE:
            ulResult = VOS_TRUE;
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32  AT_CheckCFwaveTypePara(VOS_UINT32 ulPara)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_TRUE;
    switch(ulPara)
    {
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_CONTINUE:
            ulResult = VOS_TRUE;
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32  AT_CheckWFwaveTypePara(VOS_UINT32 ulPara)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_TRUE;
    switch(ulPara)
    {
        case AT_FWAVE_TYPE_QPSK:
        case AT_FWAVE_TYPE_CONTINUE:
            ulResult = VOS_TRUE;
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32  AT_CheckGFwaveTypePara(VOS_UINT32 ulPara)
{
    VOS_UINT32                          ulResult;

    ulResult = VOS_TRUE;
    switch(ulPara)
    {
        case AT_FWAVE_TYPE_GMSK:
        case AT_FWAVE_TYPE_8PSK:
        case AT_FWAVE_TYPE_CONTINUE:
            ulResult = VOS_TRUE;
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}



VOS_UINT32  AT_CheckFwaveTypePara(VOS_UINT32 ulPara)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;
    switch(g_stMtInfoCtx.stBbicInfo.enCurrtRatMode)
    {
        case RAT_MODE_GSM:
            ulResult = AT_CheckGFwaveTypePara(ulPara);
            break;
        case RAT_MODE_WCDMA:
            ulResult = AT_CheckWFwaveTypePara(ulPara);
            break;
        case RAT_MODE_CDMA:
            ulResult = AT_CheckCFwaveTypePara(ulPara);
            break;
        case RAT_MODE_LTE:
            ulResult = AT_CheckLteFwaveTypePara(ulPara);
            break;
        case RAT_MODE_NR:
            ulResult = AT_CheckNrFwaveTypePara(ulPara);
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}


VOS_UINT32 AT_CovertAtFwaveTypeToBbicCal(
    AT_FWAVE_TYPE_ENUM_UINT8            enType,
    MODU_TYPE_ENUM_UINT16              *penType
)
{
    VOS_UINT32                          ulResult;

    ulResult    = VOS_TRUE;
    switch(enType)
    {
        case AT_FWAVE_TYPE_BPSK:
            *penType = MODU_TYPE_LTE_BPSK;
            break;
        case AT_FWAVE_TYPE_PI2_BPSK:
            *penType = MODU_TYPE_LTE_PI2_BPSK;
            break;
        case AT_FWAVE_TYPE_QPSK:
            *penType = MODU_TYPE_LTE_QPSK;
            break;
        case AT_FWAVE_TYPE_16QAM:
            *penType = MODU_TYPE_LTE_16QAM;
            break;
        case AT_FWAVE_TYPE_64QAM:
            *penType = MODU_TYPE_LTE_64QAM;
            break;
        case AT_FWAVE_TYPE_256QAM:
            *penType = MODU_TYPE_LTE_256QAM;
            break;
        case AT_FWAVE_TYPE_GMSK:
            *penType = MODU_TYPE_GMSK;
            break;
        case AT_FWAVE_TYPE_8PSK:
            *penType = MODU_TYPE_8PSK;
            break;
        /* BBIC没有单音定义，所以付值为BUTT */
        case AT_FWAVE_TYPE_CONTINUE:
            *penType = MODU_TYPE_BUTT;
            break;
        default:
            ulResult = VOS_FALSE;
            break;
    }

    return ulResult;
}



VOS_UINT32 AT_CoverTselPathToPriOrDiv(
    AT_TSELRF_PATH_ENUM_UINT32          enTselPath,
    AT_ANT_TYPE_ENUM_UINT8             *penAntType
)
{
    VOS_UINT32                          ulPathNo;
    VOS_UINT32                          ulIndex;

    ulPathNo = sizeof(g_astPath2AntTypeTable) / sizeof(AT_PATH_TO_ANT_TYPE_STRU);

    for (ulIndex = 0; ulIndex < ulPathNo; ulIndex++ )
    {
        if (enTselPath == g_astPath2AntTypeTable[ulIndex].enTselPath)
        {
            *penAntType = g_astPath2AntTypeTable[ulIndex].enAntType;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_UINT32  At_SetFChanWifiProc(VOS_UINT32 ulBand)
{
    /* WIFI的第一个参数必须为8，第二个参数必须为15*/
    if (AT_BAND_WIFI != ulBand)
    {
        return AT_FCHAN_WIFI_BAND_ERR;
    }

    /* WIFI 分支 */
    if (BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        /*WIFI未Enable直接返回失败*/
        if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
        {
            return AT_WIFI_NOT_ENABLE;
        }

        g_stMtInfoCtx.stAtInfo.enRatMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    }

    return AT_NOT_SUPPORT_WIFI;
}


VOS_UINT32 At_GetBaseFreq(RAT_MODE_ENUM_UINT16 enRatMode)
{
    VOS_UINT32                          ulBaseFreq;
    switch(enRatMode)
    {
        case RAT_MODE_GSM:
            ulBaseFreq = 66700;
            break;
            
        case RAT_MODE_WCDMA:
            ulBaseFreq = 480000;
            break;
            
        case RAT_MODE_CDMA:
            ulBaseFreq = 153600;
            break;
            
        case RAT_MODE_LTE:
            ulBaseFreq = 480000;
            break;
            
        case RAT_MODE_NR: 
            ulBaseFreq = 480000;
            break;
            
        default: 
            ulBaseFreq = 480000;
            break;
    }

    return ulBaseFreq;
}


#endif



