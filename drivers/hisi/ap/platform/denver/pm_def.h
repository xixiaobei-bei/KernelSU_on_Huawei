#ifndef __PM_DEF_H__
#define __PM_DEF_H__ 
#include <pmic_interface.h>
#include <m3_interrupts.h>
#include <soc_crgperiph_interface.h>
#include <m3_sram_map.h>
#define AP_WAKE_INT_MASK ( BIT(SOC_SCTRL_SCINT_MASK_gpio_22_int_START) | \
                                            BIT(SOC_SCTRL_SCINT_MASK_gpio_23_int_START) | \
                                            BIT(SOC_SCTRL_SCINT_MASK_gpio_24_int_START) | \
                                            BIT(SOC_SCTRL_SCINT_MASK_gpio_25_int_START) | \
                                            BIT(SOC_SCTRL_SCINT_MASK_gpio_26_int_START) | \
                                            BIT(SOC_SCTRL_SCINT_MASK_gpio_27_int_START) | \
                                    BIT(SOC_SCTRL_SCINT_MASK_rtc0_int_START) | \
                                    BIT(SOC_SCTRL_SCINT_MASK_rtc1_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer00_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer11_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer40_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer41_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer50_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer71_int_START) | \
                                      BIT(SOC_SCTRL_SCINT_MASK_timer61_int_START))
#define AP_WAKE_INT_MASK1 ( BIT(SOC_SCTRL_SCINT_MASK1_mad_int_START) | \
                         BIT(SOC_SCTRL_SCINT_MASK1_se_gpio1_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_gpio_28_int_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_gpio_21_int_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_gpio_20_int_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_int_gpio30_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_int_gpio29_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_int_gpio33_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_int_gpio32_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK1_int_gpio31_START))
#define AP_WAKE_INT_MASK2 ( BIT(SOC_SCTRL_SCINT_MASK2_gic_irq_int0_START) | \
                                                                                                    BIT(SOC_SCTRL_SCINT_MASK2_gic_irq_int1_START))
#define AP_WAKE_INT_MASK3 ( BIT(SOC_SCTRL_SCINT_MASK3_vad_int_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK3_intr_gpio_36_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK3_intr_gpio_35_START) | \
                       BIT(SOC_SCTRL_SCINT_MASK3_intr_gpio_34_START) | \
                             0x001FFFFF)
#define AO_IPC_WAKE_INT_MASK1 ( BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int0_START) | \
                                       BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int7_START) | \
                                    BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int8_START) | \
                                                                     BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int5_START) | \
                                  BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int1_START))
#define IOMCU_WAKE_INT_MASK ( BIT(SOC_SCTRL_SCINT_MASK_intr_iomcu_wdog_START) | \
                              BIT(SOC_SCTRL_SCINT_MASK_intr_wakeup_iomcu_START))
#define MODEM_INT_MASK (BIT(SOC_SCTRL_SCINT_MASK_timer01_int_START) | \
   BIT(SOC_SCTRL_SCINT_MASK_timer10_int_START) | \
   BIT(SOC_SCTRL_SCINT_MASK_timer20_int_START) | \
   BIT(SOC_SCTRL_SCINT_MASK_timer21_int_START) | \
   BIT(SOC_SCTRL_SCINT_MASK_timer30_int_START) | \
   BIT(SOC_SCTRL_SCINT_MASK_timer31_int_START))
#define MODEM_DRX_INT_MASK 0x0
#define MODEM_INT_MASK4 (BIT(SOC_SCTRL_SCINT_STAT4_lte_drx_arm_wakeup_int_START) | \
       BIT(SOC_SCTRL_SCINT_STAT4_tds_drx_arm_wakeup_int_START) | \
       BIT(SOC_SCTRL_SCINT_MASK4_g1_int_gbbp_to_cpu_on_START) | \
       BIT(SOC_SCTRL_SCINT_MASK4_g2_int_gbbp_to_cpu_on_START) | \
       BIT(SOC_SCTRL_SCINT_STAT4_w_arm_int02_START) | \
       BIT(SOC_SCTRL_SCINT_MASK4_cbbp_int01_2_START) | \
       BIT(SOC_SCTRL_SCINT_MASK4_g3_int_gbbp_to_cpu_on_START) | \
       BIT(SOC_SCTRL_SCINT_MASK4_w_arm_int02_2_START) | \
       BIT(SOC_SCTRL_SCINT_STAT4_lte2_drx_arm_wakeup_int_START) | \
       BIT(SOC_SCTRL_SCINT_MASK4_ltev_drx_arm_wakeup_int_START))
#define HIFI_WAKE_INT_MASK (BIT(SOC_SCTRL_SCINT_MASK_intr_asp_ipc_arm_START) | \
       BIT(SOC_SCTRL_SCINT_MASK_intr_asp_watchdog_START))
#define MODEM_NR_INT_MASK4 (BIT(SOC_SCTRL_SCDRX_INT_CFG_nr_drx_arm_wakeup_int_START))
#define MODEM_NR_INT_MASK5 (BIT(SOC_SCTRL_SCINT_STAT5_intr_timer140_START) | \
     BIT(SOC_SCTRL_SCINT_STAT5_intr_timer141_START) | \
     BIT(SOC_SCTRL_SCINT_STAT5_intr_timer150_START) | \
     BIT(SOC_SCTRL_SCINT_STAT5_intr_timer151_START))
#define AP_MASK (0x01)
#define MODEM_MASK (0x02)
#define HIFI_MASK (0x04)
#define IOMCU_MASK (0x08)
#define LPMCU_MASK (0x10)
#define GENERAL_SEE_MASK (0x20)
#define MODEM_NR_MASK (0x100)
#define HOTPLUG_MASK(cluster) (0x1 << ((cluster) + 6))
#define LPMCU_TICKMARK_FEATURE 
enum tickmark_point
{
    TICK_SYS_WAKEUP = 6,
    TICK_SYS_NORMAL = 8,
    TICK_DDR_AVAILABLE = 9,
    TICK_SRAM_AVAILABLE = 10,
    TICK_MODEM_SRAM_START = 11,
    TICK_SYS_MAIN_IN = 12,
    TICK_MODEM_SRAM_END = 13,
    TICK_MODEM_UP_START = 2,
    TICK_MODEM_UP_END = 3,
    TICK_MODEM_DOWN_START = 0,
    TICK_MODEM_DOWN_END = 1,
    TICK_SLEEPFLOW_ENTER = 14,
    TICK_MODEM_UNAVAILABLE = 15,
    TICK_PERI_UNAVAILABLE = 16,
    TICK_DDR_UNAVAILABLE = 17,
    TICK_SYS_UNAVAILABLE = 18,
    TICK_DEEPSLEEP_WFI_ENTER = 5,
    TICK_SYS_WAKEUP_END = 7,
    TICK_SR_DDR_OP = 19,
    TICK_REM_DDR_OP = 20,
    TICK_AP_WAKEUP_IRQ = 21,
    TICK_AP_WAKEUP_RESUME = 22,
    TICK_AP_WAKEUP_UP_BEFORE = 23,
    TICK_AP_WAKEUP_UP_AFTER = 24,
    TICK_USBPHY_BEFORE = 25,
    TICK_USBPHY_AFTER = 26,
 TICK_SYS_SUSPEND_ENTRY = 4,
 TICK_LATSTAT_STOP_ENTRY = 28,
 TICK_LATSTAT_STOP_EXIT = 29,
 TICK_MODEM_SUSPEND_ENTRY = 30,
 TICK_MODEM_SUSPEND_EXIT = 31,
 TICK_DDR_DPM_SUSPEND_ENTRY = 32,
 TICK_DDR_DPM_SUSPEND_EXIT = 33,
 TICK_DDR_SUSPEND_ENTRY = 34,
 TICK_DDR_SUSPEND_EXIT = 35,
 TICK_PERI_FIX_DOWN_ENTRY = 36,
 TICK_PERI_FIX_DOWN_EXIT = 37,
 TICK_AOBUS_SUSPEND_ENTRY = 38,
 TICK_AOBUS_SUSPEND_EXIT = 39,
 TICK_MODEM_RESUME_PRE_ENTRY = 40,
 TICK_MODEM_RESUME_PRE_EXIT = 41,
 TICK_PERI_FIX_UP_ENTRY = 42,
 TICK_PERI_FIX_UP_EXIT = 43,
 TICK_DDR_RESUME_ENTRY = 44,
 TICK_DDR_RESUME_EXIT = 45,
 TICK_MODEM_SRAM_BAK_START = 46,
 TICK_MODEM_SRAM_BAK_END = 47,
 TICK_MODEM_RESUME_AFTER_ENTRY = 48,
 TICK_MODEM_RESUME_AFTER_EXIT = 49,
 TICK_DDR_DPM_RESUME_ENTRY = 50,
 TICK_DDR_DPM_RESUME_EXIT = 51,
 TICK_AP_SUSPEND_ENTRY = 52,
 TICK_AP_SUSPEND_EXIT = 53,
 TICK_AP_WAKEUP_RESUME_END = 54,
 TICK_MARK_END_FLAG = 55,
 TICK_MAX = LPMCU_TELE_MNTN_DATA_TICKMARK_SIZE / 4,
};
enum sr_tick_pos {
 NO_SR = 0,
 KERNEL_SUSPEND_PREPARE,
 KERNEL_SUSPEND_IN,
 KERNEL_SUSPEND_SETFLAG,
 BL31_SUSPEND_IPC,
 LPMCU_AP_SUSPEND_IN,
 LPMCU_AP_SUSPEND_CPU,
 LPMCU_AP_SUSPEND_IOMCU,
 LPMCU_AP_SUSPEND_ASP,
 LPMCU_AP_SUSPEND_END,
 LPMCU_SYS_SUSPEND_IN,
 LPMCU_SYS_SUSPEND_DDRDFS,
 LPMCU_SYS_SUSPEND_IO,
 LPMCU_SYS_SUSPEND_CLK,
 LPMCU_SYS_SUSPEND_DDR,
 LPMCU_SYS_SUSPEND_UART,
 LPMCU_SYS_SUSPEND_AUTOFS,
 LPMCU_SYS_RESUME_IO,
 LPMCU_SYS_RESUME_AUTOFS,
 LPMCU_SYS_MAIN_IN,
 LPMCU_SYS_MAIN_END,
 LPMCU_AP_RESUME_IN,
 LPMCU_AP_RESUME_ASP,
 LPMCU_AP_RESUME_IOMCU,
 LPMCU_AP_RESUME_CPU,
 BL31_RESUME_IN,
 KERNEL_RESUME,
 KERNEL_RESUME_OUT,
 SR_ABNORMAL,
 SR_TICK_MAX,
};
#define PMUOFFSET_SR_TICK PMIC_HRST_REG11_ADDR(0)
#define COREPWRACK_MASK \
  (BIT(SOC_CRGPERIPH_PERPWRACK_fcm_little_core0_pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_little_core1_pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_little_core2_pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_little_core3_pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_middle_core0_pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_middle_core1_pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_middle_core2_pwrack_START ) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_fcm_big_core0_pwrack_START))
#define AO_IPC_PROCESSOR_MAX 3
#define AO_NSIPC_MAILBOX_MAX 3
#define AP_IPC_PROC_BIT 1
#define IRQ_COMB_GIC_IPC IRQ_IPC_GIC_INT_COMB
#ifdef CONFIG_SLT_SR
#define SLT_SR_FLAG (0x7D)
#define SLT_SR_SYS_FLAG (0xAE << 8)
#define SLT_SR_REQ_FLAG (0x5D << 16)
#if defined(__FASTBOOT__)
#define SLT_SR_BASE_ADDR SOC_SCTRL_SCBAKDATA23_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#elif defined(__CMSIS_RTOS)
#define SLT_SR_BASE_ADDR SOC_SCTRL_SCBAKDATA23_ADDR(SOC_LPMCU_SCTRL_BASE_ADDR)
#endif
#define SLT_TST_SR_FLG(x) ((readl(SLT_SR_BASE_ADDR) & 0xFF) == (x))
#define SLT_TST_SR_SYS_FLG(x) ((readl(SLT_SR_BASE_ADDR) & 0xFF00) == (x))
#define SLT_TST_SR_REQ_FLG(x) ((readl(SLT_SR_BASE_ADDR) & 0xFF0000) == (x))
#define SLT_SET_SUSPENDFLG(x) \
 do {writel((readl(SLT_SR_BASE_ADDR) | (x)), \
   SLT_SR_BASE_ADDR);} while (0)
#define SLT_CLR_SUSPENDFLG(x) \
 do {writel((readl(SLT_SR_BASE_ADDR) & (~(x))), \
   SLT_SR_BASE_ADDR);} while (0)
#endif
#endif
