/*
 * power_thermalzone.c
 *
 * thermal for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/thermal.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_thermalzone.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_tz
HWLOG_REGIST();

static struct power_tz_info *g_power_tz_info;

#define power_tz_ntc_table_size(a)  (sizeof(a) / sizeof((a)[0][0]) / 2)

/* ntc code is Thermitor 07050125 (resistor to temperature) */
static int ntc_pullup_100k_r_t[][POWER_TZ_GROUP_SIZE] = {
	{ 2561, 125 },     { 2634, 124 },     { 2709, 123 },
	{ 2787, 122 },     { 2868, 121 },     { 2952, 120 },
	{ 3038, 119 },     { 3128, 118 },     { 3220, 117 },
	{ 3316, 116 },     { 3415, 115 },     { 3518, 114 },
	{ 3624, 113 },     { 3734, 112 },     { 3848, 111 },
	{ 3966, 110 },     { 3966, 109 },     { 4215, 108 },
	{ 4346, 107 },     { 4482, 106 },     { 4623, 105 },
	{ 4769, 104 },     { 4921, 103 },     { 5078, 102 },
	{ 5241, 101 },     { 5410, 100 },     { 5586, 99 },
	{ 5768, 98 },      { 5957, 97 },      { 6153, 96 },
	{ 6357, 95 },      { 6568, 94 },      { 6788, 93 },
	{ 7016, 92 },      { 7254, 91 },      { 7500, 90 },
	{ 7756, 89 },      { 8022, 88 },      { 8299, 87 },
	{ 8587, 86 },      { 8887, 85 },      { 9198, 84 },
	{ 9522, 83 },      { 9859, 82 },      { 10210, 81 },
	{ 10580, 80 },     { 10960, 79 },     { 11350, 78 },
	{ 11760, 77 },     { 12190, 76 },     { 12640, 75 },
	{ 13110, 74 },     { 13600, 73 },     { 14100, 72 },
	{ 14630, 71 },     { 15180, 70 },     { 15760, 69 },
	{ 16360, 68 },     { 16990, 67 },     { 17640, 66 },
	{ 18320, 65 },     { 19040, 64 },     { 19780, 63 },
	{ 20560, 62 },     { 21370, 61 },     { 22220, 60 },
	{ 23110, 59 },     { 24040, 58 },     { 25010, 57 },
	{ 26030, 56 },     { 27090, 55 },     { 28200, 54 },
	{ 29360, 53 },     { 30580, 52 },     { 31860, 51 },
	{ 33190, 50 },     { 34600, 49 },     { 36060, 48 },
	{ 37600, 47 },     { 39210, 46 },     { 40900, 45 },
	{ 42670, 44 },     { 44530, 43 },     { 46490, 42 },
	{ 48530, 41 },     { 50680, 40 },     { 52940, 39 },
	{ 55310, 38 },     { 57810, 37 },     { 60420, 36 },
	{ 63180, 35 },     { 66070, 34 },     { 69120, 33 },
	{ 72320, 32 },     { 75690, 31 },     { 79230, 30 },
	{ 82970, 29 },     { 86900, 28 },     { 91040, 27 },
	{ 95400, 26 },     { 100000, 25 },    { 104800, 24 },
	{ 110000, 23 },    { 115400, 22 },    { 121000, 21 },
	{ 127000, 20 },    { 133400, 19 },    { 140100, 18 },
	{ 147200, 17 },    { 154600, 16 },    { 162500, 15 },
	{ 170900, 14 },    { 179700, 13 },    { 189000, 12 },
	{ 198900, 11 },    { 209400, 10 },    { 220500, 9 },
	{ 232200, 8 },     { 244700, 7 },     { 257800, 6 },
	{ 271800, 5 },     { 286700, 4 },     { 302400, 3 },
	{ 319100, 2 },     { 336800, 1 },     { 355600, 0 },
	{ 375600, -1 },    { 396900, -2 },    { 419500, -3 },
	{ 443500, -4 },    { 469100, -5 },    { 496300, -6 },
	{ 525300, -7 },    { 556200, -8 },    { 589000, -9 },
	{ 624100, -10 },   { 661500, -11 },   { 701300, -12 },
	{ 743900, -13 },   { 789300, -14 },   { 837800, -15 },
	{ 889600, -16 },   { 945000, -17 },   { 1004000, -18 },
	{ 1068000, -19 },  { 1135000, -20 },  { 1208000, -21 },
	{ 1286000, -22 },  { 1369000, -23 },  { 1458000, -24 },
	{ 1554000, -25 },  { 1656000, -26 },  { 1767000, -27 },
	{ 1885000, -28 },  { 2012000, -29 },  { 2149000, -30 },
	{ 2296000, -31 },  { 2454000, -32 },  { 2624000, -33 },
	{ 2807000, -34 },  { 3005000, -35 },  { 3218000, -36 },
	{ 3447000, -37 },  { 3695000, -38 },  { 3962000, -39 },
	{ 4251000, -40 },
};

/* ntc code is Thermitor 100K (voltage to temperature) */
static int ntc_pullup_100k_v_t[][POWER_TZ_GROUP_SIZE] = {
	{ 46, 125 }, { 47, 124 }, { 49, 123 },
	{ 50, 122 }, { 52, 121 }, { 53, 120 },
	{ 55, 119 }, { 56, 118 }, { 58, 117 },
	{ 60, 116 }, { 61, 115 }, { 63, 114 },
	{ 65, 113 }, { 67, 112 }, { 69, 111 },
	{ 71, 110 }, { 73, 109 }, { 76, 108 },
	{ 78, 107 }, { 80, 106 }, { 83, 105 },
	{ 85, 104 }, { 88, 103 }, { 91, 102 },
	{ 93, 101 }, { 96, 100 }, { 99, 99 },
	{ 102, 98 }, { 105, 97 }, { 109, 96 },
	{ 112, 95 }, { 116, 94 }, { 119, 93 },
	{ 123, 92 }, { 127, 91 }, { 131, 90 },
	{ 135, 89 }, { 139, 88 }, { 143, 87 },
	{ 148, 86 }, { 153, 85 }, { 158, 84 },
	{ 163, 83 }, { 168, 82 }, { 174, 81 },
	{ 179, 80 }, { 185, 79 }, { 191, 78 },
	{ 197, 77 }, { 204, 76 }, { 210, 75 },
	{ 217, 74 }, { 225, 73 }, { 232, 72 },
	{ 239, 71 }, { 247, 70 }, { 255, 69 },
	{ 264, 68 }, { 272, 67 }, { 281, 66 },
	{ 290, 65 }, { 300, 64 }, { 310, 63 },
	{ 320, 62 }, { 330, 61 }, { 341, 60 },
	{ 352, 59 }, { 363, 58 }, { 375, 57 },
	{ 387, 56 }, { 400, 55 }, { 412, 54 },
	{ 426, 53 }, { 439, 52 }, { 453, 51 },
	{ 467, 50 }, { 482, 49 }, { 497, 48 },
	{ 512, 47 }, { 528, 46 }, { 544, 45 },
	{ 561, 44 }, { 578, 43 }, { 595, 42 },
	{ 613, 41 }, { 631, 40 }, { 649, 39 },
	{ 668, 38 }, { 687, 37 }, { 706, 36 },
	{ 726, 35 }, { 746, 34 }, { 766, 33 },
	{ 787, 32 }, { 808, 31 }, { 829, 30 },
	{ 850, 29 }, { 872, 28 }, { 894, 27 },
	{ 915, 26 }, { 938, 25 }, { 960, 24 },
	{ 982, 23 }, { 1005, 22 }, { 1027, 21 },
	{ 1049, 20 }, { 1072, 19 }, { 1094, 18 },
	{ 1117, 17 }, { 1139, 16 }, { 1161, 15 },
	{ 1183, 14 }, { 1205, 13 }, { 1226, 12 },
	{ 1248, 11 }, { 1269, 10 }, { 1290, 9 },
	{ 1311, 8 }, { 1331, 7 }, { 1351, 6 },
	{ 1371, 5 }, { 1390, 4 }, { 1409, 3 },
	{ 1428, 2 }, { 1446, 1 }, { 1464, 0 },
	{ 1481, -1 }, { 1498, -2 }, { 1514, -3 },
	{ 1530, -4 }, { 1546, -5 }, { 1561, -6 },
	{ 1575, -7 }, { 1589, -8 }, { 1603, -9 },
	{ 1616, -10 }, { 1629, -11 }, { 1641, -12 },
	{ 1655, -13 }, { 1666, -14 }, { 1675, -15 },
	{ 1686, -16 }, { 1696, -17 }, { 1705, -18 },
	{ 1715, -19 }, { 1723, -20 }, { 1732, -21 },
	{ 1740, -22 }, { 1747, -23 }, { 1755, -24 },
	{ 1762, -25 }, { 1768, -26 }, { 1775, -27 },
	{ 1781, -28 }, { 1786, -29 }, { 1792, -30 },
	{ 1797, -31 }, { 1802, -32 }, { 1806, -33 },
	{ 1811, -34 }, { 1815, -35 }, { 1819, -36 },
	{ 1822, -37 }, { 1826, -38 }, { 1829, -39 },
	{ 1832, -40 },
};

/* ntc code is Thermitor 07050124 10K (voltage to temperature) */
static int ntc_pullup_10k_v_t[][POWER_TZ_GROUP_SIZE] = {
	{ 208, 125 },   { 218, 124 },   { 217, 123 },
	{ 222, 122 },   { 227, 121 },   { 232, 120 },
	{ 237, 119 },   { 242, 118 },   { 247, 117 },
	{ 253, 116 },   { 259, 115 },   { 264, 114 },
	{ 270, 113 },   { 277, 112 },   { 283, 111 },
	{ 289, 110 },   { 296, 109 },   { 303, 108 },
	{ 310, 107 },   { 317, 106 },   { 324, 105 },
	{ 332, 104 },   { 340, 103 },   { 347, 102 },
	{ 356, 101 },   { 364, 100 },   { 373, 99 },
	{ 381, 98 },    { 391, 97 },    { 400, 96 },
	{ 409, 95 },    { 419, 94 },    { 429, 93 },
	{ 439, 92 },    { 450, 91 },    { 461, 90 },
	{ 472, 89 },    { 483, 88 },    { 495, 87 },
	{ 507, 86 },    { 519, 85 },    { 532, 84 },
	{ 545, 83 },    { 558, 82 },    { 572, 81 },
	{ 586, 80 },    { 600, 79 },    { 614, 78 },
	{ 630, 77 },    { 645, 76 },    { 661, 75 },
	{ 677, 74 },    { 694, 73 },    { 711, 72 },
	{ 728, 71 },    { 746, 70 },    { 765, 69 },
	{ 783, 68 },    { 802, 67 },    { 822, 66 },
	{ 842, 65 },    { 863, 64 },    { 884, 63 },
	{ 905, 62 },    { 927, 61 },    { 950, 60 },
	{ 973, 59 },    { 996, 58 },    { 1020, 57 },
	{ 1045, 56 },   { 1070, 55 },   { 1095, 54 },
	{ 1122, 53 },   { 1148, 52 },   { 1175, 51 },
	{ 1203, 50 },   { 1231, 49 },   { 1260, 48 },
	{ 1289, 47 },   { 1319, 46 },   { 1349, 45 },
	{ 1380, 44 },   { 1411, 43 },   { 1443, 42 },
	{ 1475, 41 },   { 1508, 40 },   { 1541, 39 },
	{ 1575, 38 },   { 1609, 37 },   { 1643, 36 },
	{ 1678, 35 },   { 1714, 34 },   { 1750, 33 },
	{ 1786, 32 },   { 1822, 31 },   { 1859, 30 },
	{ 1896, 29 },   { 1934, 28 },   { 1972, 27 },
	{ 2010, 26 },   { 2048, 25 },   { 2086, 24 },
	{ 2125, 23 },   { 2164, 22 },   { 2203, 21 },
	{ 2242, 20 },   { 2281, 19 },   { 2321, 18 },
	{ 2360, 17 },   { 2399, 16 },   { 2438, 15 },
	{ 2476, 14 },   { 2515, 13 },   { 2554, 12 },
	{ 2593, 11 },   { 2631, 10 },   { 2669, 9 },
	{ 2707, 8 },    { 2745, 7 },    { 2782, 6 },
	{ 2819, 5 },    { 2856, 4 },    { 2892, 3 },
	{ 2927, 2 },    { 2963, 1 },    { 2997, 0 },
	{ 3032, -1 },   { 3066, -2 },   { 3099, -3 },
	{ 3131, -4 },   { 3164, -5 },   { 3195, -6 },
	{ 3226, -7 },   { 3256, -8 },   { 3286, -9 },
	{ 3315, -10 },  { 3343, -11 },  { 3371, -12 },
	{ 3398, -13 },  { 3425, -14 },  { 3451, -15 },
	{ 3476, -16 },  { 3500, -17 },  { 3524, -18 },
	{ 3547, -19 },  { 3569, -20 },  { 3591, -21 },
	{ 3612, -22 },  { 3633, -23 },  { 3653, -24 },
	{ 3672, -25 },  { 3690, -26 },  { 3708, -27 },
	{ 3726, -28 },  { 3742, -29 },  { 3758, -30 },
	{ 3774, -31 },  { 3789, -32 },  { 3803, -33 },
	{ 3817, -34 },  { 3831, -35 },  { 3843, -36 },
	{ 3856, -37 },  { 3867, -38 },  { 3879, -39 },
	{ 3890, -40 },
};

/* ntc code is Thermitor 07050124 12K (voltage to temperature) */
static int ntc_pullup_12k_v_t[][POWER_TZ_GROUP_SIZE] = {
	{ 175, 125 },   { 178, 124 },   { 182, 123 },
	{ 186, 122 },   { 191, 121 },   { 195, 120 },
	{ 199, 119 },   { 204, 118 },   { 208, 117 },
	{ 213, 116 },   { 218, 115 },   { 223, 114 },
	{ 228, 113 },   { 233, 112 },   { 238, 111 },
	{ 244, 110 },   { 250, 109 },   { 255, 108 },
	{ 261, 107 },   { 268, 106 },   { 274, 105 },
	{ 280, 104 },   { 287, 103 },   { 294, 102 },
	{ 301, 101 },   { 308, 100 },   { 315, 99 },
	{ 323, 98 },    { 331, 97 },    { 338, 96 },
	{ 347, 95 },    { 355, 94 },    { 364, 93 },
	{ 373, 92 },    { 382, 91 },    { 391, 90 },
	{ 401, 89 },    { 411, 88 },    { 421, 87 },
	{ 431, 86 },    { 442, 85 },    { 453, 84 },
	{ 464, 83 },    { 486, 82 },    { 488, 81 },
	{ 500, 80 },    { 512, 79 },    { 525, 78 },
	{ 539, 77 },    { 552, 76 },    { 566, 75 },
	{ 580, 74 },    { 595, 73 },    { 610, 72 },
	{ 626, 71 },    { 641, 70 },    { 658, 69 },
	{ 674, 68 },    { 691, 67 },    { 709, 66 },
	{ 727, 65 },    { 745, 64 },    { 764, 63 },
	{ 783, 62 },    { 803, 61 },    { 823, 60 },
	{ 844, 59 },    { 866, 58 },    { 887, 57 },
	{ 909, 56 },    { 932, 55 },    { 955, 54 },
	{ 979, 53 },    { 1004, 52 },   { 1029, 51 },
	{ 1054, 50 },   { 1080, 49 },   { 1107, 48 },
	{ 1134, 47 },   { 1161, 46 },   { 1189, 45 },
	{ 1218, 44 },   { 1247, 43 },   { 1277, 42 },
	{ 1308, 41 },   { 1339, 40 },   { 1370, 39 },
	{ 1402, 38 },   { 1435, 37 },   { 1468, 36 },
	{ 1501, 35 },   { 1535, 34 },   { 1570, 33 },
	{ 1605, 32 },   { 1640, 31 },   { 1676, 30 },
	{ 1713, 29 },   { 1749, 28 },   { 1786, 27 },
	{ 1824, 26 },   { 1862, 25 },   { 1900, 24 },
	{ 1938, 23 },   { 1977, 22 },   { 2017, 21 },
	{ 2056, 20 },   { 2096, 19 },   { 2135, 18 },
	{ 2175, 17 },   { 2215, 16 },   { 2255, 15 },
	{ 2295, 14 },   { 2335, 13 },   { 2376, 12 },
	{ 2416, 11 },   { 2455, 10 },   { 2495, 9 },
	{ 2535, 8 },    { 2575, 7 },    { 2614, 6 },
	{ 2653, 5 },    { 2692, 4 },    { 2731, 3 },
	{ 2769, 2 },    { 2807, 1 },    { 2845, 0 },
	{ 2882, -1 },   { 2919, -2 },   { 2955, -3 },
	{ 2990, -4 },   { 3026, -5 },   { 3060, -6 },
	{ 3095, -7 },   { 3128, -8 },   { 3161, -9 },
	{ 3193, -10 },  { 3225, -11 },  { 3256, -12 },
	{ 3286, -13 },  { 3316, -14 },  { 3345, -15 },
	{ 3373, -16 },  { 3401, -17 },  { 3428, -18 },
	{ 3454, -19 },  { 3480, -20 },  { 3505, -21 },
	{ 3529, -22 },  { 3552, -23 },  { 3575, -24 },
	{ 3597, -25 },  { 3619, -26 },  { 3639, -27 },
	{ 3659, -28 },  { 3679, -29 },  { 3697, -30 },
	{ 3716, -31 },  { 3733, -32 },  { 3750, -33 },
	{ 3766, -34 },  { 3782, -35 },  { 3796, -36 },
	{ 3811, -37 },  { 3825, -38 },  { 3838, -39 },
	{ 3851, -40 },
};

/* ntc code is Thermitor 07050124 18K (voltage to temperature) */
static int ntc_pullup_18k_v_t[][POWER_TZ_GROUP_SIZE] = {
	{ 0, 0 },       { 110, 125 },   { 313, 84 },
	{ 406, 74 },    { 531, 64 },    { 590, 60 },
	{ 655, 56 },    { 729, 52 },    { 812, 48 },
	{ 903, 44 },    { 1001, 40 },   { 1111, 36 },
	{ 1230, 32 },   { 1360, 28 },   { 1497, 24 },
	{ 1646, 20 },   { 1803, 16 },   { 1967, 12 },
	{ 2136, 8 },    { 2308, 4 },    { 2480, 0 },
	{ 2650, -4 },   { 2814, -8 },   { 2973, -12 },
	{ 3122, -16 },  { 3261, -20 },  { 3387, -24 },
	{ 3500, -28 },  { 3602, -32 },  { 3689, -36 },
	{ 3764, -40 },  { 4095, -273 },
};

/* ntc code is Thermitor ERTJZEG103FA (resistor to temperature) */
static int ntc_pullup_18k_r_t[][POWER_TZ_GROUP_SIZE] = {
	{ 4986, 125 },     { 5111, 124 },     { 5240, 123 },
	{ 5372, 122 },     { 5509, 121 },     { 5650, 120 },
	{ 5795, 119 },     { 5945, 118 },     { 6099, 117 },
	{ 6258, 116 },     { 6421, 115 },     { 6590, 114 },
	{ 6764, 113 },     { 6943, 112 },     { 7127, 111 },
	{ 7317, 110 },     { 7513, 109 },     { 7715, 108 },
	{ 7922, 107 },     { 8136, 106 },     { 8357, 105 },
	{ 8584, 104 },     { 8818, 103 },     { 9059, 102 },
	{ 9307, 101 },     { 9563, 100 },     { 9826, 99 },
	{ 10100, 98 },     { 10380, 97 },     { 10670, 96 },
	{ 10970, 95 },     { 11270, 94 },     { 11590, 93 },
	{ 11920, 92 },     { 12260, 91 },     { 12610, 90 },
	{ 12970, 89 },     { 13340, 88 },     { 13720, 87 },
	{ 14110, 86 },     { 14510, 85 },     { 14930, 84 },
	{ 15360, 83 },     { 15800, 82 },     { 16250, 81 },
	{ 16720, 80 },     { 17200, 79 },     { 17700, 78 },
	{ 18210, 77 },     { 18740, 76 },     { 19290, 75 },
	{ 19860, 74 },     { 20450, 73 },     { 21050, 72 },
	{ 21680, 71 },     { 22330, 70 },     { 23010, 69 },
	{ 23710, 68 },     { 24430, 67 },     { 25180, 66 },
	{ 25950, 65 },     { 26760, 64 },     { 27590, 63 },
	{ 28450, 62 },     { 29340, 61 },     { 30270, 60 },
	{ 31230, 59 },     { 32220, 58 },     { 33250, 57 },
	{ 34320, 56 },     { 35430, 55 },     { 36580, 54 },
	{ 37780, 53 },     { 39020, 52 },     { 40310, 51 },
	{ 41650, 50 },     { 43040, 49 },     { 44480, 48 },
	{ 45980, 47 },     { 47540, 46 },     { 49160, 45 },
	{ 50840, 44 },     { 52590, 43 },     { 54410, 42 },
	{ 56310, 41 },     { 58280, 40 },     { 60330, 39 },
	{ 62460, 38 },     { 64680, 37 },     { 67000, 36 },
	{ 69410, 35 },     { 71920, 34 },     { 74540, 33 },
	{ 77270, 32 },     { 80120, 31 },     { 83090, 30 },
	{ 86190, 29 },     { 89420, 28 },     { 92790, 27 },
	{ 96320, 26 },     { 100000, 25 },    { 103800, 24 },
	{ 107900, 23 },    { 112100, 22 },    { 116500, 21 },
	{ 121100, 20 },    { 125900, 19 },    { 130900, 18 },
	{ 136200, 17 },    { 141700, 16 },    { 147400, 15 },
	{ 153500, 14 },    { 159800, 13 },    { 166400, 12 },
	{ 173400, 11 },    { 180600, 10 },    { 188300, 9 },
	{ 196300, 8 },     { 204700, 7 },     { 213500, 6 },
	{ 222700, 5 },     { 232400, 4 },     { 242600, 3 },
	{ 253300, 2 },     { 264600, 1 },     { 276400, 0 },
	{ 288800, -1 },    { 301900, -2 },    { 315600, -3 },
	{ 330000, -4 },    { 345300, -5 },    { 361300, -6 },
	{ 378100, -7 },    { 395900, -8 },    { 414600, -9 },
	{ 434400, -10 },   { 455200, -11 },   { 477200, -12 },
	{ 500400, -13 },   { 524900, -14 },   { 550700, -15 },
	{ 578000, -16 },   { 606900, -17 },   { 637400, -18 },
	{ 669600, -19 },   { 703700, -20 },   { 739900, -21 },
	{ 778100, -22 },   { 818600, -23 },   { 861500, -24 },
	{ 906900, -25 },   { 955100, -26 },   { 1006000, -27 },
	{ 1060000, -28 },  { 1118000, -29 },  { 1179000, -30 },
	{ 1244000, -31 },  { 1313000, -32 },  { 1387000, -33 },
	{ 1465000, -34 },  { 1548000, -35 },  { 1636000, -36 },
	{ 1731000, -37 },  { 1831000, -38 },  { 1938000, -39 },
	{ 2052000, -40 },
};

/* ntc valut is 100K (voltage to temperature) */
static int ntc_pullup_22k_v_t[][POWER_TZ_GROUP_SIZE] = {
	{ 107, 124 },  { 117, 120 },  { 127, 116 },
	{ 139, 112 },  { 152, 108 },  { 166, 104 },
	{ 182, 100 },  { 200, 96 },   { 220, 92 },
	{ 242, 88 },   { 267, 84 },   { 296, 80 },
	{ 327, 76 },   { 362, 72 },   { 402, 68 },
	{ 447, 64 },   { 497, 60 },   { 554, 56 },
	{ 617, 52 },   { 688, 48 },   { 767, 44 },
	{ 856, 40 },   { 954, 36 },   { 1063, 32 },
	{ 1183, 28 },  { 1314, 24 },  { 1456, 20 },
	{ 1609, 16 },  { 1772, 12 },  { 1943, 8 },
	{ 2120, 4 },   { 2302, 0 },   { 2485, -4 },
	{ 2666, -8 },  { 2842, -12 }, { 3010, -16 },
	{ 3167, -20 }, { 3312, -24 }, { 3443, -28 },
	{ 3559, -32 }, { 3660, -36 }, { 3746, -40 },
};

/* ntc code is Thermitor 10K (resistor to temperature) */
static int ntc_pullup_10k_r_t[][POWER_TZ_GROUP_SIZE] = {
	{ 593, 124 }, { 646, 120 }, { 706, 116 },
	{ 772, 112 }, { 847, 108 }, { 930, 104 },
	{ 1024, 100 }, { 1130, 96 }, { 1249, 92 },
	{ 1384, 88 }, { 1537, 84 }, { 1711, 80 },
	{ 1909, 76 }, { 2136, 72 }, { 2396, 68 },
	{ 2695, 64 }, { 3039, 60 }, { 3438, 56 },
	{ 3901, 52 }, { 4440, 48 }, { 5070, 44 },
	{ 5810, 40 }, { 6681, 36 }, { 7710, 32 },
	{ 8932, 28 }, { 10389, 24 }, { 12133, 20 },
	{ 14231, 16 }, { 16767, 12 }, { 19847, 8 },
	{ 23608, 4 }, { 28224, 0 }, { 33922, -4 },
	{ 40997, -8 }, { 49837, -12 }, { 60952, -16 },
	{ 75022, -20 }, { 92957, -24 }, { 115988, -28 },
	{ 145792, -32 }, { 184674, -36 }, { 235831, -40 },
};

static int data_to_temp_linear(long data, int index_min, int index_max,
	const int *trans_list)
{
	int prorate;
	int resistance_min;
	int resistance_max;
	int temper_bottom;
	int temper_top;
	int itemper;

	resistance_min = *(trans_list + 2 * index_min + 0);
	resistance_max = *(trans_list + 2 * index_max + 0);
	temper_bottom = *(trans_list + 2 * index_min + 1);
	temper_top = *(trans_list + 2 * index_max + 1);

	if ((resistance_max - resistance_min) == 0)
		return POWER_TZ_DEFAULT_TEMP;

	prorate = (((resistance_max - data) * PRORATE_OF_INIT) /
		(resistance_max - resistance_min));
	itemper = (((temper_bottom - temper_top) * prorate) +
		(temper_top * PRORATE_OF_INIT));

	return itemper;
}


static int get_data_range_using_dichotomy(int *temp, long data, int len,
	const int *list)
{
	int low = 0;
	int mid;

	if (!temp || !list)
		return -1;

	if (data < *(list + 2 * 0 + 0)) {
		*temp = (*(list + 2 * 0 + 1)) * PRORATE_OF_INIT;
		return 0;
	} else if (data > *(list + 2 * (len - 1) + 0)) {
		*temp = (*(list + 2 * (len - 1) + 1)) * PRORATE_OF_INIT;
		return 0;
	}

	while (low <= len) {
		mid = (low + len) / 2;
		if (*(list + 2 * mid + 0) > data) {
			if (*(list + 2 * (mid - 1) + 0) < data) {
				*temp = data_to_temp_linear(data, (mid - 1),
					mid, list);
				return 0;
			}
			len = mid - 1;
		} else if (*(list + 2 * mid + 0) < data) {
			if (*(list + 2 * (mid + 1) + 0) > data) {
				*temp = data_to_temp_linear(data, mid,
					(mid + 1), list);
				return 0;
			}
			low = mid + 1;
		} else {
			*temp = (*(list + 2 * mid + 1)) * PRORATE_OF_INIT;
			break;
		}
	}

	return 0;
}

static int power_tz_get_temp(struct thermal_zone_device *thermal, int *temp)
{
	struct power_tz_sensor *sensor = NULL;
	int ret;
	long data = 0;
	int *list = NULL;
	int len;

	if (!thermal || !thermal->devdata || !temp) {
		hwlog_err("thermal or temp is null\n");
		return -EINVAL;
	}
	sensor = thermal->devdata;

	if (!sensor->get_raw_data) {
		hwlog_err("get_raw_data is null[sensor:%s ops_name:%s]\n",
			sensor->sensor_name, sensor->ops_name);
		goto fail_get_temp;
	}

	ret = sensor->get_raw_data(sensor->adc_channel, &data, sensor->dev_data);
	if (ret) {
		hwlog_err("temp get fail[sensor:%s ops_name:%s]\n",
			sensor->sensor_name, sensor->ops_name);
		goto fail_get_temp;
	}

	switch (sensor->ntc_index) {
	case NTC_PULLUP_100K_RT:
		len = power_tz_ntc_table_size(ntc_pullup_100k_r_t);
		list = (int *)ntc_pullup_100k_r_t;
		break;
	case NTC_PULLUP_10K_VT:
		len = power_tz_ntc_table_size(ntc_pullup_10k_v_t);
		list = (int *)ntc_pullup_10k_v_t;
		break;
	case NTC_PULLUP_12K_VT:
		len = power_tz_ntc_table_size(ntc_pullup_12k_v_t);
		list = (int *)ntc_pullup_12k_v_t;
		break;
	case NTC_PULLUP_18K_VT:
		len = power_tz_ntc_table_size(ntc_pullup_18k_v_t);
		list = (int *)ntc_pullup_18k_v_t;
		break;
	case NTC_PULLUP_18K_RT:
		len = power_tz_ntc_table_size(ntc_pullup_18k_r_t);
		list = (int *)ntc_pullup_18k_r_t;
		break;
	case NTC_PULLUP_22K_VT:
		len = power_tz_ntc_table_size(ntc_pullup_22k_v_t);
		list = (int *)ntc_pullup_22k_v_t;
		break;
	case NTC_PULLUP_10K_RT:
		len = power_tz_ntc_table_size(ntc_pullup_10k_r_t);
		list = (int *)ntc_pullup_10k_r_t;
		break;
	case NTC_PULLUP_100K_VT:
		len = power_tz_ntc_table_size(ntc_pullup_100k_v_t);
		list = (int *)ntc_pullup_100k_v_t;
		break;
	case NTC_NO_CONVERSION:
		*temp = (int)(data * THOUSAND_UNIT);
		goto judging_temp;
	default:
		len = 0;
		list = NULL;
		hwlog_err("unknown ntc:%d\n", sensor->ntc_index);
		break;
	}

	ret = get_data_range_using_dichotomy(temp, data, len, list);
	if (ret) {
		hwlog_err("temp get fail\n");
		goto fail_get_temp;
	}

judging_temp:
	if ((*temp < POWER_TZ_MIN_TEMP) || (*temp > POWER_TZ_MAX_TEMP)) {
		hwlog_err("invalid temp [%d]\n", *temp);
		goto fail_get_temp;
	}

	return 0;

fail_get_temp:
	*temp = POWER_TZ_DEFAULT_TEMP;
	return -1;
}

static struct thermal_zone_device_ops power_tz_dev_ops = {
	.get_temp = power_tz_get_temp,
};

static int power_tz_get_adc_sample(int adc_channel, long *data,
	void *dev_data)
{
	int i;
	int adc_sample;

	for (i = 0; i < POWER_TZ_ADC_RETRY_TIMES; i++) {
		adc_sample = power_platform_get_adc_sample(adc_channel);
		if (adc_sample < 0)
			hwlog_err("adc read fail\n");
		else
			break;
	}
	*data = adc_sample;

	return 0;
}

int power_tz_ops_register(struct power_tz_ops *ops, char *ops_name)
{
	int i;
	struct power_tz_info *info = g_power_tz_info;

	if (!info || !ops) {
		hwlog_err("info or ops is null\n");
		return -EPERM;
	}

	for (i = 0; i < info->total_sensor; i++) {
		if (!strncmp(ops_name, info->sensor[i].ops_name,
			strlen(info->sensor[i].ops_name))) {
			info->sensor[i].get_raw_data = ops->get_raw_data;
			info->sensor[i].dev_data = ops->dev_data;
			break;
		}
	}

	if (i >= info->total_sensor) {
		hwlog_err("%s ops register fail\n", ops_name);
		return -EPERM;
	}

	hwlog_info("%s ops register ok\n", ops_name);
	return 0;
}

static int power_tz_parse_dts(struct device_node *dev_node,
	struct power_tz_info *info)
{
	int i = 0;
	struct device_node *child_node = NULL;
	const char *sensor_name = NULL;
	const char *ops_name = NULL;

	info->total_sensor = of_get_child_count(dev_node);
	if (info->total_sensor <= 0) {
		hwlog_err("total_sensor dts read failed\n");
		return -EINVAL;
	}

	for_each_child_of_node(dev_node, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG), child_node,
			"sensor_name", &sensor_name))
			return -EINVAL;

		if (power_dts_read_string(power_dts_tag(HWLOG_TAG), child_node,
			"ops_name", &ops_name))
			return -EINVAL;

		if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
			"adc_channel", &info->sensor[i].adc_channel, 0))
			return -EINVAL;

		if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
			"ntc_index", &info->sensor[i].ntc_index, 0))
			return -EINVAL;

		if (!strncmp(ops_name, POWER_TZ_DEFAULT_OPS,
			strlen(POWER_TZ_DEFAULT_OPS)))
			info->sensor[i].get_raw_data = power_tz_get_adc_sample;

		strncpy(info->sensor[i].sensor_name, sensor_name,
			(POWER_TZ_STR_MAX_LEN - 1));
		strncpy(info->sensor[i].ops_name, ops_name,
			(POWER_TZ_STR_MAX_LEN - 1));

		i++;
	}

	for (i = 0; i < info->total_sensor; i++)
		hwlog_info("para[%d]:sensor:%s,ops:%s,adc_chann:%d,ntc_index:%d\n",
			i,
			info->sensor[i].sensor_name,
			info->sensor[i].ops_name,
			info->sensor[i].adc_channel,
			info->sensor[i].ntc_index);

	return 0;
}

static int power_tz_probe(struct platform_device *pdev)
{
	struct power_tz_info *info = NULL;
	struct device_node *np = NULL;
	struct device_node *dev_node = NULL;
	int ret = -1;
	int i;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	g_power_tz_info = info;
	info->pdev = pdev;
	np = pdev->dev.of_node;

	dev_node = of_find_node_by_name(np, "sensors");
	if (!dev_node) {
		hwlog_err("sensors node dts read failed\n");
		goto fail_free_mem;
	}

	ret = power_tz_parse_dts(dev_node, info);
	if (ret < 0)
		goto fail_parse_dts;

	for (i = 0; i < info->total_sensor; i++) {
		info->sensor[i].tz_dev = thermal_zone_device_register(
			info->sensor[i].sensor_name,
			0, 0, &info->sensor[i], &power_tz_dev_ops, NULL, 0, 0);
		if (IS_ERR(info->sensor[i].tz_dev)) {
			hwlog_err("thermal zone register fail\n");
			ret = -ENODEV;
			goto fail_register_tz;
		}
	}

	platform_set_drvdata(pdev, info);
	return 0;

fail_register_tz:
	for (i = 0; i < info->total_sensor; i++)
		thermal_zone_device_unregister(info->sensor[i].tz_dev);
fail_parse_dts:
	of_node_put(dev_node);
fail_free_mem:
	devm_kfree(&pdev->dev, info);
	g_power_tz_info = NULL;
	return ret;
}

static int power_tz_remove(struct platform_device *pdev)
{
	int i;
	struct power_tz_info *info = platform_get_drvdata(pdev);

	if (!info)
		return -ENODEV;

	for (i = 0; i < info->total_sensor; i++)
		thermal_zone_device_unregister(info->sensor[i].tz_dev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	g_power_tz_info = NULL;

	return 0;
}

static const struct of_device_id power_tz_match_table[] = {
	{
		.compatible = "huawei,power_thermalzone",
		.data = NULL,
	},
	{},
};

static struct platform_driver power_tz_driver = {
	.probe = power_tz_probe,
	.remove = power_tz_remove,
	.driver = {
		.name = "huawei,power_thermalzone",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(power_tz_match_table),
	},
};

static int __init power_tz_init(void)
{
	return platform_driver_register(&power_tz_driver);
}

static void __exit power_tz_exit(void)
{
	platform_driver_unregister(&power_tz_driver);
}

fs_initcall_sync(power_tz_init);
module_exit(power_tz_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power thermal_zone module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
