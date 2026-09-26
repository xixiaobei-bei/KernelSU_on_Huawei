/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hisi_pmic_eis.h
 *
 * eis pmic driver header
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
#ifndef _HISI_PMIC_EIS_H_
#define _HISI_PMIC_EIS_H_

#include <linux/mfd/hisi_pmic.h>
#include <linux/power/hisi/hisi_eis.h>
#include <pmic_interface.h>

#ifndef _STATIC
#define _STATIC
#endif

#ifdef CONFIG_BATT_EIS_VERBOSE_DEBUG
#define eis_debug(fmt, args...) \
	pr_debug("[dbg][%s-%d]:" fmt, __func__, __LINE__, ##args)
#else
#define eis_debug(fmt, args...) do {} while (0)
#endif

#define eis_inf(fmt, args...) \
	pr_info("[info][%s-%d]:" fmt, __func__, __LINE__, ##args)
#define eis_err(fmt, args...) \
	pr_err("[error][%s-%d]:" fmt, __func__, __LINE__, ##args)

/* reg read and write interface macro func */
#define eis_reg_read(reg_addr) pmic_read_reg(reg_addr)
#define eis_reg_write(reg_addr, reg_val) \
	pmic_write_reg((int)(reg_addr), (int)(reg_val))

/* n: lower bit, m: higher bit, n, and m itself is included */
#define eis_reg_getbits(x, n, m) (((x) & ~(~0U << ((m) - (n) + 1)) << (n)) >> (n))

/*
 * shift: the start bit(the lower end, start from 0), including itself;
 * mask: the length of bits(m-n+1)
 * when the set_val is out of the bit range of mask, the set val will be cut
 * from lower bit to higher end for the length of mask, then it will be
 * filled in the reg_val.
 */
#define eis_reg_setbits(reg_val, shift, mask, set_val) \
	(((reg_val)  = ((reg_val) & (~(((1U << (mask)) - 1) << (shift)))) \
	| ((unsigned int)((set_val) & ((1U << (mask)) - 1)) << (shift))))

#ifndef _SUCC
#define _SUCC                   0 /* for success flag */
#endif

#ifndef _ERROR
#define _ERROR                  (-1) /* for error debug */
#endif

#ifndef VALID
#define VALID                   1 /* for marking valid sampling data */
#endif

#ifndef INVALID
#define INVALID                 0 /* for marking invalid sampling data */
#endif

#define RANGE_ELEMENT_COUNT     2 /* min value and max value */

/* EIS BASIC MACRO */
#define V_BAT_FIFO_DEPTH        16
#define I_BAT_FIFO_DEPTH        4
#define V_BAT_H_SHIFT           8
#define I_BAT_H_SHIFT           8

#define EIS_IRQ_NULL_DEPLETED   0X3D
#define EIS_IRQ_FIFO_CLEAR      0X3C
#define EIS_FIFO_DATA_FILLED    1
#define LEN_I_BAT_TIMER_K       7
#define LEN_I_BAT_TIMER_J       4
#define INITIAL_FREQ_V_GAIN     10
#define INITIAL_FREQ_I_GAIN     512
#define LEN_V_BAT_GAIN          7
#define LEN_I_BAT_GAIN_2M       4
#define LEN_I_BAT_GAIN_5M       5
#define LEN_RES_EXTERNAL        2
#define LEN_CURT_PULL_AMPLTD    4
#define LEN_T_ANA2ADC           2
#define LEN_ADC_AVG_TIMES       4
#define LEN_T_CHNL2START        4
#define LEN_T_WAIT              4
#define EIS_F_CUR_PULL          200


/* sample current threshold and threshold error macro */
#define A_TO_MA                 1000
#define CUR_FREQ_THRES_H        200 /* unit: mA */
#define CUR_FREQ_THRES_L        0 /* unit: mA */
#define CUR_FREQ_THRES_H_ERR    40 /* unit: mA */
#define CUR_FREQ_THRES_L_ERR    40 /* unit: mA */
#define VOL_REF_MV              1800 /* unit: mV */
#define DIGIT2ANA_MAG           10000
#define CUR_TOTAL_STEP          4095
#define R_OHM_2M                2
#define R_OHM_5M                5
#define R_OHM_DEFAULT           (R_OHM_2M)
#define SCREEN_IS_ON            1
#define SCREEN_IS_OFF           0
#define HIGH_ERR_MAX            0XFF
#define LOW_ERR_MAX             0XFF
#define HIGH_MAX                0XFFF
#define LOW_MAX                 0XFFF
/* default value for 0xA012 */
#define T_WAIT                  0 /* unit mS */
#define T_CHNL2START            30 /* unit uS */
#define ADC_AVG_TIMES           4
#define T_ANA2ADC               50 /* unit US */
/* for vol and cur gain gear adjust */
#define V_GAIN_ADJUSTED         1
#define V_GAIN_UNADJUSTED       0
#define I_GAIN_ADJUSTED         1
#define I_GAIN_UNADJUSTED       0
#define V_I_GAIN_ADJUSTED       1
#define V_I_GAIN_UNADJUSTED     0
#define CALBRAT_A_DFT           1000000 /* unit: uA */

enum eis_chopper_enable {
	EIS_CHOPPER_DIS = 0,
	EIS_CHOPPER_EN = 1,
};

enum eis_v_channel_mask {
	VOL_NON_MASK = 0,
	VOL_MASK = 3,
};

enum eis_i_channel_mask {
	CUR_NON_MASK = 0,
	CUR_MASK = 3,
};

enum com_mode_op_amp_enable {
	OP_AMP_DIS = 0,
	OP_AMP_EN = 1,
};

enum eis_is_v_averaged {
	VOL_NON_AVGED = 0,
	VOL_AVGED = 1,
};

enum eis_is_i_averaged {
	CUR_NON_AVGED = 0,
	CUR_AVGED = 1,
};

enum eis_dischg_enable {
	EIS_DISCHG_DIS = 0,
	EIS_DISCHG_EN = 1,
};

enum eis_dischg_mode {
	EIS_HALF_EMPTY = 0,
	EIS_ANY_TIME = 1,
};

enum eis_irq_mask_ops {
	IRQ_NON_MASK = 0,
	IRQ_MASK = 1,
};

enum eis_irq_ops {
	_READ = 0,
	_CLEAR = 1,
};

enum eis_irq_status {
	NO_IRQ = 0,
	EXIST_IRQ = 1,
};

enum eis_curt_probe_state {
	DIS = 0,
	EN = 1,
};

/* start of ana register config */
#define EIS_ANA_CTRL0           PMIC_EIS_ANA_CTRL0_ADDR(0) /* 0xA000 */
#define EIS_ANA_CTRL1           PMIC_EIS_ANA_CTRL1_ADDR(0) /* 0xA001 */
#define EIS_ANA_CTRL2           PMIC_EIS_ANA_CTRL2_ADDR(0) /* 0xA002 */

/* start of eis test mode enable register */
#define EIS_TEST_CTRL           PMIC_EIS_TEST_CTRL_ADDR(0) /* 0x0A005 */
#define EIS_RESERVE0            PMIC_EIS_RESERVE0_ADDR(0) /* 0x0A006 */

/* eis enable register */
#define EIS_CTRL0               PMIC_EIS_CTRL0_ADDR(0) /* 0xA010 */

/* eis discharge enable register */
#define EIS_CTRL1               PMIC_EIS_CTRL1_ADDR(0) /* 0xA011 */

/* eis register time lag */
#define EIS_CTRL2               PMIC_EIS_CTRL2_ADDR(0) /* 0xA012 */

/* start of eis current offset register */
#define EISADC_INIT_I_OFFSET_L  PMIC_EISADC_INIT_I_OFFSET_DATA_L_ADDR(0) /* 0xA015 */
#define EISADC_INIT_I_OFFSET_H  PMIC_EISADC_INIT_I_OFFSET_DATA_H_ADDR(0) /* 0xA016 */

/* start of eis freq. single period register */
#define EIS_DET_TIMER           PMIC_EIS_DET_TIMER_ADDR(0) /* 0xA021 */

/* start of eis empty periods register */
#define EIS_DET_TIMER_N         PMIC_EIS_DET_TIMER_N_ADDR(0) /* 0xA022 */

/* start of eis sample periods register */
#define EIS_DET_TIMER_M_L       PMIC_EIS_DET_TIMER_M_L_ADDR(0) /* 0xA023 */
#define EIS_DET_TIMER_M_H       PMIC_EIS_DET_TIMER_M_H_ADDR(0) /* 0xA024 */

/* start of eis sample current periods register */
#define EIS_IBAT_DET_CTRL       PMIC_EIS_IBAT_DET_CTRL_ADDR(0) /* 0xA025 */

/* start of eis vol and cur averaged register */
#define EIS_ADC_AVERAGE         PMIC_EIS_ADC_AVERAGE_ADDR(0) /* 0xA026 */

/* start of eis current threshold register */
#define EIS_HTHRESHOLD_ERROR    PMIC_EIS_HTHRESHOLD_ERROR_ADDR(0) /* 0xA02A */
#define EIS_ICOMP_HTHRESHOLD_L  PMIC_EIS_ICOMP_HTHRESHOLD_L_ADDR(0) /* 0xA02B */
#define EIS_ICOMP_HTHRESHOLD_H  PMIC_EIS_ICOMP_HTHRESHOLD_H_ADDR(0) /* 0xA02C */
#define EIS_LTHRESHOLD_ERROR    PMIC_EIS_LTHRESHOLD_ERROR_ADDR(0) /* 0xA02D */
#define EIS_ICOMP_LTHRESHOLD_L  PMIC_EIS_ICOMP_LTHRESHOLD_L_ADDR(0) /* 0xA02E */
#define EIS_ICOMP_LTHRESHOLD_H  PMIC_EIS_ICOMP_LTHRESHOLD_H_ADDR(0) /* 0xA02F */
#define EIS_ICOMP_STATE_CLR     PMIC_EIS_ICOMP_STATE_CLR_ADDR(0) /* 0xA030 */
#define EIS_I_CURRENT_L         PMIC_EIS_I_CURRENT_L_ADDR(0) /* 0xA031 */
#define EIS_I_CURRENT_H         PMIC_EIS_I_CURRENT_L_ADDR(0) /* 0xA032 */

/* start of v_bat register */
#define EIS_VBAT_D1_L           PMIC_EIS_VBAT_D1_L_ADDR(0) /* 0xA033 */
#define EIS_VBAT_D1_H           PMIC_EIS_VBAT_D1_H_ADDR(0) /* 0xA034 */
#define EIS_VBAT_D2_L           PMIC_EIS_VBAT_D2_L_ADDR(0) /* 0xA035 */
#define EIS_VBAT_D2_H           PMIC_EIS_VBAT_D2_H_ADDR(0) /* 0xA036 */
#define EIS_VBAT_D3_L           PMIC_EIS_VBAT_D3_L_ADDR(0) /* 0xA037 */
#define EIS_VBAT_D3_H           PMIC_EIS_VBAT_D3_H_ADDR(0) /* 0xA038 */
#define EIS_VBAT_D4_L           PMIC_EIS_VBAT_D4_L_ADDR(0) /* 0xA039 */
#define EIS_VBAT_D4_H           PMIC_EIS_VBAT_D4_H_ADDR(0) /* 0xA03A */
#define EIS_VBAT_D5_L           PMIC_EIS_VBAT_D5_L_ADDR(0) /* 0xA03B */
#define EIS_VBAT_D5_H           PMIC_EIS_VBAT_D5_H_ADDR(0) /* 0xA03C */
#define EIS_VBAT_D6_L           PMIC_EIS_VBAT_D6_L_ADDR(0) /* 0xA03D */
#define EIS_VBAT_D6_H           PMIC_EIS_VBAT_D6_H_ADDR(0) /* 0xA03E */
#define EIS_VBAT_D7_L           PMIC_EIS_VBAT_D7_L_ADDR(0) /* 0xA03F */
#define EIS_VBAT_D7_H           PMIC_EIS_VBAT_D7_H_ADDR(0) /* 0xA040 */
#define EIS_VBAT_D8_L           PMIC_EIS_VBAT_D8_L_ADDR(0) /* 0xA041 */
#define EIS_VBAT_D8_H           PMIC_EIS_VBAT_D8_H_ADDR(0) /* 0xA042 */
#define EIS_VBAT_D9_L           PMIC_EIS_VBAT_D9_L_ADDR(0) /* 0xA043 */
#define EIS_VBAT_D9_H           PMIC_EIS_VBAT_D9_H_ADDR(0) /* 0xA044 */
#define EIS_VBAT_D10_L          PMIC_EIS_VBAT_D10_L_ADDR(0) /* 0xA045 */
#define EIS_VBAT_D10_H          PMIC_EIS_VBAT_D10_H_ADDR(0) /* 0xA046 */
#define EIS_VBAT_D11_L          PMIC_EIS_VBAT_D11_L_ADDR(0) /* 0xA047 */
#define EIS_VBAT_D11_H          PMIC_EIS_VBAT_D11_H_ADDR(0) /* 0xA048 */
#define EIS_VBAT_D12_L          PMIC_EIS_VBAT_D12_L_ADDR(0) /* 0xA049 */
#define EIS_VBAT_D12_H          PMIC_EIS_VBAT_D12_H_ADDR(0) /* 0xA04A */
#define EIS_VBAT_D13_L          PMIC_EIS_VBAT_D13_L_ADDR(0) /* 0xA04B */
#define EIS_VBAT_D13_H          PMIC_EIS_VBAT_D13_H_ADDR(0) /* 0xA04C */
#define EIS_VBAT_D14_L          PMIC_EIS_VBAT_D14_L_ADDR(0) /* 0xA04D */
#define EIS_VBAT_D14_H          PMIC_EIS_VBAT_D14_H_ADDR(0) /* 0xA04E */
#define EIS_VBAT_D15_L          PMIC_EIS_VBAT_D15_L_ADDR(0) /* 0xA04F */
#define EIS_VBAT_D15_H          PMIC_EIS_VBAT_D15_H_ADDR(0) /* 0xA050 */
#define EIS_VBAT_D16_L          PMIC_EIS_VBAT_D16_L_ADDR(0) /* 0xA051 */
#define EIS_VBAT_D16_H          PMIC_EIS_VBAT_D16_H_ADDR(0) /* 0xA052 */

/* start of i_bat register */
#define EIS_IBAT_P_D4_L         PMIC_EIS_IBAT_P_D4_L_ADDR(0) /* 0xA053 */
#define EIS_IBAT_P_D4_H         PMIC_EIS_IBAT_P_D4_H_ADDR(0) /* 0xA054 */
#define EIS_IBAT_P_D8_L         PMIC_EIS_IBAT_P_D8_L_ADDR(0) /* 0xA055 */
#define EIS_IBAT_P_D8_H         PMIC_EIS_IBAT_P_D8_H_ADDR(0) /* 0xA056 */
#define EIS_IBAT_P_D12_L        PMIC_EIS_IBAT_P_D12_L_ADDR(0) /* 0xA057 */
#define EIS_IBAT_P_D12_H        PMIC_EIS_IBAT_P_D12_L_ADDR(0) /* 0xA058 */
#define EIS_IBAT_P_D16_L        PMIC_EIS_IBAT_P_D16_L_ADDR(0) /* 0xA059 */
#define EIS_IBAT_P_D6_H         PMIC_EIS_IBAT_P_D16_H_ADDR(0) /* 0xA05A */

/* v_bat order number register */
#define EIS_VBAT_DET_NUM        PMIC_EIS_VBAT_DET_NUM_ADDR(0) /* 0xA05D */

/* start of periods recorder register */
#define EIS_DET_T_NUM_L         PMIC_EIS_DET_T_NUM_L_ADDR(0) /* 0xA05B */
#define EIS_DET_T_NUM_H         PMIC_EIS_DET_T_NUM_H_ADDR(0) /* 0xA05C */

/* start of eis process status */
#define EIS_DET_FLAG            PMIC_EIS_DET_FLAG_ADDR(0) /* 0xA05E */
#define EIS_INIT_END_FLAG       PMIC_EIS_INIT_END_FLAG_ADDR(0) /* 0xA05F */

/* start of eis irq and mask register */
#define EIS_IRQ                 PMIC_EIS_IRQ_ADDR(0) /* 0x02C4 */
#define EIS_IRQ_MASK            PMIC_IRQ_MASK_17_ADDR(0) /* 0x02A1 */
#endif /* _HISI_PMIC_EIS_H_ */
