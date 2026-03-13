#ifndef __PM_DEF_H__
#define __PM_DEF_H__ 
#include <pmic_interface.h>
#include <soc_crgperiph_interface.h>
#ifndef LPMCU_USER_VERSION
#define MODEM_TEST BIT(SOC_SCTRL_SCINT_MASK_timer61_int_START)
#else
#define MODEM_TEST 0
#endif
#define AP_WAKE_INT_MASK (BIT(SOC_SCTRL_SCINT_MASK_gpio_22_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_gpio_23_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_gpio_24_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_gpio_25_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_gpio_26_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_gpio_27_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_rtc_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_rtc1_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_timer00_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_timer11_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_timer40_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_timer41_int_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_timer50_int_START) | \
                         BIT(SOC_SCTRL_SCINT_MASK_timer71_int_START) | \
       MODEM_TEST)
#define AP_WAKE_INT_MASK1 (BIT(SOC_SCTRL_SCINT_MASK1_se_gpio1_START) | \
       BIT(SOC_SCTRL_SCINT_MASK1_gpio_28_int_START) | \
       BIT(SOC_SCTRL_SCINT_MASK1_gpio_18_int_START) | \
       BIT(SOC_SCTRL_SCINT_MASK1_gpio_19_int_START))
#define AO_IPC_WAKE_INT_MASK1 (BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int0_START) | \
        BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int19_START) | \
        BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int18_START) | \
        BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int17_START) | \
        BIT(SOC_SCTRL_SCINT_MASK1_ao_ipc_int16_START))
#define IOMCU_WAKE_INT_MASK (BIT(SOC_SCTRL_SCINT_MASK_intr_iomcu_wdog_START) | \
        BIT(SOC_SCTRL_SCINT_MASK_intr_wakeup_iomcu_START))
#define MODEM_INT_MASK (0x0000f610)
#define MODEM_DRX_INT_MASK (0x000080bf)
#define HIFI_WAKE_INT_MASK (BIT(SOC_SCTRL_SCINT_MASK_intr_asp_ipc_arm_START) | \
                      BIT(SOC_SCTRL_SCINT_MASK_intr_asp_watchdog_START))
#define AP_MASK (0x01)
#define MODEM_MASK (0x02)
#define HIFI_MASK (0x04)
#define IOMCU_MASK (0x08)
#define LPMCU_MASK (0x10)
#define HISEE_MASK (0x20)
#define HOTPLUG_MASK(cluster) (0x1 << ((cluster) + 6))
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
  (BIT(SOC_CRGPERIPH_PERPWRACK_a53_0_core0pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_0_core1pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_0_core2pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_0_core3pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_1_core0pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_1_core1pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_1_core2pwrack_START) \
  | BIT(SOC_CRGPERIPH_PERPWRACK_a53_1_core3pwrack_START))
#define AO_IPC_PROCESSOR_MAX 3
#define AO_NSIPC_MAILBOX_MAX 3
#define AP_IPC_PROC_BIT 1
#ifdef CONFIG_SLT_SR
#define SLT_SR_FLAG (0x7D)
#define SLT_SR_SYS_FLAG (0xAE << 8)
#define SLT_SR_REQ_FLAG (0x5D << 16)
#if defined(__FASTBOOT__)
#define SLT_SR_BASE_ADDR SOC_SCTRL_SCBAKDATA11_ADDR(SOC_ACPU_SCTRL_BASE_ADDR)
#elif defined(__CMSIS_RTOS)
#define SLT_SR_BASE_ADDR SOC_SCTRL_SCBAKDATA11_ADDR(SOC_LPMCU_SCTRL_BASE_ADDR)
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
