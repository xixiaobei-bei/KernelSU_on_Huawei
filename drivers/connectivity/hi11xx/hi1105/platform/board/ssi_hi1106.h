

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

#ifndef __SSI_HI1106_H__
#define __SSI_HI1106_H__

/* 以下寄存器是1106 device定义 */
#define HI1106_GLB_CTL_BASE                    0x40000000
#define HI1106_GLB_CTL_SOFT_RST_BCPU_REG       (HI1106_GLB_CTL_BASE + 0x98)
#define HI1106_GLB_CTL_SOFT_RST_GCPU_REG       (HI1106_GLB_CTL_BASE + 0x9C)
#define HI1106_GLB_CTL_SYS_TICK_CFG_W_REG      (HI1106_GLB_CTL_BASE + 0xC0) /* 写1清零systick，写0无效 */
#define HI1106_GLB_CTL_SYS_TICK_VALUE_W_0_REG  (HI1106_GLB_CTL_BASE + 0xD0)
#define HI1106_GLB_CTL_PWR_ON_LABLE_REG        (HI1106_GLB_CTL_BASE + 0x200) /* 芯片上电标记寄存器 */
#define HI1106_GLB_CTL_WCPU0_LOAD_REG          (HI1106_GLB_CTL_BASE + 0x1750) /* WCPU0_LOAD */
#define HI1106_GLB_CTL_WCPU0_PC_L_REG          (HI1106_GLB_CTL_BASE + 0x1754) /* WCPU0_PC低16bit */
#define HI1106_GLB_CTL_WCPU0_PC_H_REG          (HI1106_GLB_CTL_BASE + 0x1758) /* WCPU0_PC高16bit */
#define HI1106_GLB_CTL_WCPU1_LOAD_REG          (HI1106_GLB_CTL_BASE + 0x175C) /* WCPU1_LOAD */
#define HI1106_GLB_CTL_WCPU1_PC_L_REG          (HI1106_GLB_CTL_BASE + 0x1760) /* WCPU1_PC低16bit */
#define HI1106_GLB_CTL_WCPU1_PC_H_REG          (HI1106_GLB_CTL_BASE + 0x1764) /* WCPU1_PC高16bit */
#define HI1106_GLB_CTL_GCPU_LOAD_REG           (HI1106_GLB_CTL_BASE + 0x1B00) /* GCPU_LOAD */
#define HI1106_GLB_CTL_GCPU_PC_L_REG           (HI1106_GLB_CTL_BASE + 0x1B04) /* GCPU_PC低16bit */
#define HI1106_GLB_CTL_GCPU_PC_H_REG           (HI1106_GLB_CTL_BASE + 0x1B08) /* GCPU_PC高16bit */
#define HI1106_GLB_CTL_GCPU_LR_L_REG           (HI1106_GLB_CTL_BASE + 0x1B0C) /* GCPU_LR低16bit */
#define HI1106_GLB_CTL_GCPU_LR_H_REG           (HI1106_GLB_CTL_BASE + 0x1B10) /* GCPU_LR高16bit */
#define HI1106_GLB_CTL_GCPU_SP_L_REG           (HI1106_GLB_CTL_BASE + 0x1B14) /* GCPU_SP低16bit */
#define HI1106_GLB_CTL_GCPU_SP_H_REG           (HI1106_GLB_CTL_BASE + 0x1B18) /* GCPU_SP高16bit */
#define HI1106_GLB_CTL_BCPU_LOAD_REG           (HI1106_GLB_CTL_BASE + 0x1C00) /* BCPU_LOAD */
#define HI1106_GLB_CTL_BCPU_PC_L_REG           (HI1106_GLB_CTL_BASE + 0x1C04) /* BCPU_PC低16bit */
#define HI1106_GLB_CTL_BCPU_PC_H_REG           (HI1106_GLB_CTL_BASE + 0x1C08) /* BCPU_PC高16bit */
#define HI1106_GLB_CTL_BCPU_LR_L_REG           (HI1106_GLB_CTL_BASE + 0x1C0C) /* BCPU_LR低16bit */
#define HI1106_GLB_CTL_BCPU_LR_H_REG           (HI1106_GLB_CTL_BASE + 0x1C10) /* BCPU_LR高16bit */
#define HI1106_GLB_CTL_BCPU_SP_L_REG           (HI1106_GLB_CTL_BASE + 0x1C14) /* BCPU_SP低16bit */
#define HI1106_GLB_CTL_BCPU_SP_H_REG           (HI1106_GLB_CTL_BASE + 0x1C18) /* BCPU_SP高16bit */
#define HI1106_GLB_CTL_TCXO_DET_CTL_REG        (HI1106_GLB_CTL_BASE + 0x800) /* TCXO时钟检测控制寄存器 */
#define HI1106_GLB_CTL_TCXO_32K_DET_CNT_REG    (HI1106_GLB_CTL_BASE + 0x804) /* TCXO时钟检测控制寄存器 */
#define HI1106_GLB_CTL_TCXO_32K_DET_RESULT_REG (HI1106_GLB_CTL_BASE + 0x808) /* TCXO时钟检测控制寄存器 */
#define HI1106_GLB_CTL_WCPU_WAIT_CTL_REG       (HI1106_GLB_CTL_BASE + 0xF00)
#define HI1106_GLB_CTL_BCPU_WAIT_CTL_REG       (HI1106_GLB_CTL_BASE + 0xF04)

#define HI1106_PMU_CMU_CTL_BASE                   0x40002000
#define HI1106_PMU_CMU_CTL_SYS_STATUS_0_REG       (HI1106_PMU_CMU_CTL_BASE + 0x1E0) /* 系统状态 */

int hi1106_ssi_read_wcpu_pc_lr_sp(void);
int hi1106_ssi_read_bpcu_pc_lr_sp(void);
int hi1106_ssi_read_gpcu_pc_lr_sp(void);
int hi1106_ssi_read_device_arm_register(void);
int hi1106_ssi_device_regs_dump(unsigned long long module_set);

#endif /* #ifndef __SSI_HI1106_H__ */
#endif /* #ifdef __PRE_CONFIG_GPIO_TO_SSI_DEBUG */
