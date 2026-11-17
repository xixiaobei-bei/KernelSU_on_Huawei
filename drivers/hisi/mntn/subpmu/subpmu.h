/*
 * subpmu.h
 *
 * subpmu header for dieid
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef __SUBPMU_H__
#define __SUBPMU_H__

#ifdef CONFIG_HISI_DIEID
#define ID_OTP_CLK_CTRL	0x54
#define ID_OTP_CTRL0	0x51
#define ID_OTP_CTRL1	0x52
#define ID_OTP_0	0x4d
 /* 6423 dieid: 32bit, exactly 1/2 of total size */
#define HI6423_READ_TIMES	(HISI_PMU_IP_SIZE / 2)

/* define hi6423v100 & hi6422v300 commen para */
#define OPEN_CLK_VALUE	0x5a
#define RESET_VALUE	0x00
#define DELAY_TIME	30
#define AND_VALUE1	0xe3
#define AND_VALUE2	0xfb
#define AND_VALUE3	0xfc
#define AND_VALUE4	0xfd
#define AND_VALUE5	0x3f
#define AND_VALUE6	0xc0

#define HISI_PMU_TMP_BUF	50
#define HISI_PMU_IP_SIZE	8 /* 8 bytes */
#define HISI_PMU_BUF_SIZE	200

/* define hi6422v300 register */
#define OTP_119_112	0xf6
#define OTP_REG_BEGIN	0x0c
#define HI6422_OTP_REG_NUM	HISI_PMU_IP_SIZE
#define RST_REG_NUM	4

/* define hi6422v500 register */
#define OTP_START_V5	32
#define OTP_END_V5	39

#define PMU_IDX_START	2
#define OTP_INCTRL_SEL_BIT	1
#define OTP_REG_STEP	2
#define OTP_POR_INT_BIT	2

enum hi6422v300_reg {
	OTP_CLK_CTRL = 0xC0,
	OTP_CTRL0,
	OTP_CTRL1,
	OTP_CTRL2,
};

enum hi6422v500_reg {
	OTP0_V5 = 0xC0,
	OTP1_V5,
	OTP_CLK_CTRL_V5,
	OTP_CTRL0_V5,
	OTP_CTRL1_V5,
	OTP_CTRL2_V5,
};

struct pmu_para {
	const char *pmu_type;
	const char *bus_type;
	unsigned int slave_id;
};

struct hisi_subpmu_dieid {
	unsigned int dieid_num;
	struct pmu_para *pmu_info;
};

typedef int (*pmu_func_t)(const struct pmu_para *pmu_info,
			  unsigned char *die_id,
			  unsigned int dieid_size);
struct pmu_dieid_func_def {
	const char *pmu_type;
	pmu_func_t pmu_func;
};

int hisi_subpmu_get_dieid(char *dieid, unsigned int len);
#endif
#endif
