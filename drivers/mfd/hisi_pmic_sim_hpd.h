/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: pmic sim hpd irq process
 * Author: Hisilicon
 * Create: 2019-11-18
 */
#ifndef _HISI_PMIC_SIM_HPD_H_
#define _HISI_PMIC_SIM_HPD_H_

#include <linux/bitops.h>

#define LDO_DISABLE 0x00
#define LDO_ONOFF_MASK 0x01

#define SIM_ENABLE 0x04
#define SIM_ONOFF_MASK 0x05
/* main pmic sim hpd process interface */
#if defined(CONFIG_HISI_HI6421V800_PMU) || \
	defined(CONFIG_HISI_HI6421V900_PMU) || \
	defined(CONFIG_HISI_HI6555V500_PMU)

#include <pmic_interface.h>

#define HISI_PMIC_SIM_HPD_EN 1

#define SIM_IRQ_ADDR PMIC_IRQ1_ADDR(0)

#define SIM0_HPD_R_MASK BIT(PMIC_IRQ1_sim0_hpd_r_START)
#define SIM0_HPD_F_MASK BIT(PMIC_IRQ1_sim0_hpd_f_START)
#define SIM1_HPD_R_MASK BIT(PMIC_IRQ1_sim1_hpd_r_START)
#define SIM1_HPD_F_MASK BIT(PMIC_IRQ1_sim1_hpd_f_START)

#define SIM0_HPD_PENDING (SIM0_HPD_R_MASK | SIM0_HPD_F_MASK)
#define SPM1_HPD_PENDING (SIM1_HPD_R_MASK | SIM1_HPD_F_MASK)
#define SIM_HPD_PENDING (SIM0_HPD_PENDING | SPM1_HPD_PENDING)

/* sim hpd ldo power down en regs */
#define SIM_CTRL0_ADDR PMIC_SIM_CTRL0_ADDR(0)
#define SIM_CTRL1_ADDR PMIC_SIM_CTRL1_ADDR(0)

#define LDO11_ONOFF_ADDR PMIC_LDO11_ONOFF_ECO_ADDR(0)
#define LDO12_ONOFF_ADDR PMIC_LDO12_ONOFF_ECO_ADDR(0)
#define LDO16_ONOFF_ADDR PMIC_LDO16_ONOFF_ECO_ADDR(0)

#define SIM0_HPD_R_LDO11_PD BIT(PMIC_SIM_CTRL0_sim0_hpd_r_pd_en_START)
#define SIM0_HPD_F_LDO11_PD BIT(PMIC_SIM_CTRL0_sim0_hpd_f_pd_en_START)
#define SIM1_HPD_R_LDO12_PD BIT(PMIC_SIM_CTRL0_sim1_hpd_r_pd_en_START)
#define SIM1_HPD_F_LDO12_PD BIT(PMIC_SIM_CTRL0_sim1_hpd_f_pd_en_START)

#define SIM0_HPD_LDO12_PD BIT(PMIC_SIM_CTRL0_sim0_hpd_pd_ldo12_en_START)

#define SIM0_HPD_R_LDO16_PD BIT(PMIC_SIM_CTRL1_sim0_hpd_ldo16_en3_START)
#define SIM0_HPD_F_LDO16_PD BIT(PMIC_SIM_CTRL1_sim0_hpd_ldo16_en2_START)
#define SIM1_HPD_R_LDO16_PD BIT(PMIC_SIM_CTRL1_sim1_hpd_ldo16_en1_START)
#define SIM1_HPD_F_LDO16_PD BIT(PMIC_SIM_CTRL1_sim1_hpd_ldo16_en0_START)

void hisi_pmic_sim_hpd_proc(void);
#else
static inline void hisi_pmic_sim_hpd_proc(void)
{
}
#endif

/* sub pmic sim hpd process interface */
#if defined(CONFIG_HISI_HI6421V900_PMU) && \
	defined(CONFIG_HISI_PMIC_SUB_PMU_SPMI)

#include <pmic_sub_interface.h>

#define HISI_SUB_PMIC_SIM_HPD_EN 1

#define SUB_SIM_IRQ_ADDR SUB_PMIC_IRQ_SIM_GPIO_ADDR(0)
#define SUB_SIM_HPD_R_MASK BIT(SUB_PMIC_IRQ_SIM_GPIO_irq_sim_hpd_r_START)
#define SUB_SIM_HPD_F_MASK BIT(SUB_PMIC_IRQ_SIM_GPIO_irq_sim_hpd_f_START)
#define SUB_SIM_HPD_PENDING (SUB_SIM_HPD_R_MASK | SUB_SIM_HPD_F_MASK)

#define SUB_SIM_CTRL0_ADDR SUB_PMIC_SIM_CTRL0_ADDR(0)

#define SUB_SIM_HPD_PD_LDO53 BIT(SUB_PMIC_SIM_CTRL0_sim_hpd_pd_ldo53_en_START)
#define SUB_SIM_HPD_PD_LDO54 BIT(SUB_PMIC_SIM_CTRL0_sim_hpd_pd_ldo54_en_START)

#define SUB_SIM_HPD_F_PD_EN BIT(SUB_PMIC_SIM_CTRL0_sim_hpd_f_pd_en_START)
#define SUB_SIM_HPD_R_PD_EN BIT(SUB_PMIC_SIM_CTRL0_sim_hpd_r_pd_en_START)

#define LDO53_ONOFF_ADDR SUB_PMIC_LDO53_ONOFF_ECO_ADDR(0)
#define LDO54_ONOFF_ADDR SUB_PMIC_LDO54_ONOFF_ECO_ADDR(0)

#define SUB_SIM_CFG_0_ADDR SUB_PMIC_SIM_CFG_0_ADDR(0)
#define SUB_SIM_CFG_2_ADDR SUB_PMIC_SIM_CFG_2_ADDR(0)

void hisi_sub_pmic_sim_hpd_proc(void);

#else
static inline void hisi_sub_pmic_sim_hpd_proc(void)
{
}
#endif
#endif
