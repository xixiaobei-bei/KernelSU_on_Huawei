#ifndef	__HISI_PMIC_DCXO_H
#define	__HISI_PMIC_DCXO_H

#define NVE_CALC1C2_NAME "CALC1C2"
#define PMIC_CLK_TOP_CTRL1_0_np_xo_trim_c2fix_MASK (0xF)
#define PMIC_DCXO_CFIX1  PMIC_CLK_TOP_CTRL1_1_ADDR(0)
#define PMIC_DCXO_CFIX2  PMIC_CLK_TOP_CTRL1_0_ADDR(0)
#define PMIC_DCXO_CFIX2_MASK    PMIC_CLK_TOP_CTRL1_0_np_xo_trim_c2fix_MASK

int pmu_dcxo_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix);
int pmu_dcxo_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix);
int pmu_dcxo_reg_set(uint16_t dcxo_ctrim, uint16_t dcxo_c2_fix);
int pmu_dcxo_reg_get(uint16_t *dcxo_ctrim, uint16_t *dcxo_c2_fix);




#endif