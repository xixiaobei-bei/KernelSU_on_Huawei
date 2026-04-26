

#ifndef __BOARD_HI1103_H__
#define __BOARD_HI1103_H__

/* 其他头文件包含 */
#include "plat_type.h"
#include "hw_bfg_ps.h"

/* 宏定义 */
#define GPIO_BASE_ADDR        0X50004000
#define CHECK_DEVICE_RDY_ADDR 0X50000000
#define WLAN_HOST2DEV_GPIO ((unsigned int)(1 << 1))
#define WLAN_DEV2HOST_GPIO ((unsigned int)(1 << 0))

#define GPIO_DIRECTION_OUTPUT    0
#define GPIO_DIRECTION_INPUT     1

#define GPIO_LEVEL_CONFIG_REGADDR       0x0  /* GPIO管脚的电平值拉高或拉低寄存器 */
#define GPIO_INOUT_CONFIG_REGADDR       0x04 /* GPIO管脚的数据方向存器 */
#define GPIO_TYPE_CONFIG_REGADDR        0x30 /* GPIO管脚的模式寄存器:IO or INT */
#define GPIO_INT_POLARITY_REGADDR       0x3C /* GPIO中断极性寄存器 */
#define GPIO_INT_TYPE_REGADDR           0x38 /* GPIO中断触发类型寄存器:电平触发或边沿触发 */
#define GPIO_INT_CLEAR_REGADDR          0x4C /* GPIO清除中断寄存器，只对边沿触发的中断有效 */
#define GPIO_LEVEL_GET_REGADDR          0x50 /* GPIO管脚当前电平值寄存器 */
#define GPIO_INTERRUPT_DEBOUNCE_REGADDR 0x48 /* GPIO管脚是否使能去抖动 */

#define BFGX_SUBSYS_RST_DELAY   100
#define WIFI_SUBSYS_RST_DELAY   10

#define PROC_NAME_GPIO_WLAN_FLOWCTRL  "hi110x_wlan_flowctrl"

/* test ssi write bcpu code */
/* EXTERN VARIABLE */
#ifdef PLATFORM_DEBUG_ENABLE
extern int32_t g_device_monitor_enable;
#endif

/* 函数声明 */
int32_t hi1103_get_board_power_gpio(struct platform_device *pdev);
void hi1103_free_board_power_gpio(struct platform_device *pdev);
int32_t hi1103_board_wakeup_gpio_init(struct platform_device *pdev);
void hi1103_free_board_wakeup_gpio(struct platform_device *pdev);
int32_t hi1103_board_wifi_tas_gpio_init(void);
void hi1103_free_board_wifi_tas_gpio(void);
int32_t hi1103_check_pmu_clk_share(void);
int32_t hi1103_bfgx_dev_power_on(void);
int32_t hi1103_bfgx_dev_power_off(void);
int32_t hi1103_wlan_power_off(void);
int32_t hi1103_wlan_power_on(void);
int32_t hi1103_board_power_on(uint32_t ul_subsystem);
int32_t hi1103_board_power_off(uint32_t ul_subsystem);
int32_t hi1103_board_power_reset(uint32_t ul_subsystem);
int32_t hi1103_wifi_subsys_reset(void);
void hi1103_bfgx_subsys_reset(void);
int32_t hi1103_get_board_pmu_clk32k(void);
int32_t hi1103_get_board_uart_port(void);
int32_t hi1103_board_ir_ctrl_init(struct platform_device *pdev);
int32_t hi1103_check_evb_or_fpga(void);
int32_t hi1103_check_hi110x_subsystem_support(void);
int32_t hi1103_board_get_power_pinctrl(void);
int32_t hi1103_get_ini_file_name_from_dts(char *dts_prop, char *prop_value, uint32_t size);
void hi1103_chip_power_on(void);
void hi1103_bfgx_enable(void);
int32_t hi1103_wifi_enable(void);
void hi1103_chip_power_off(void);
void hi1103_bfgx_disable(void);
int32_t hi1103_wifi_disable(void);
#if defined(CONFIG_PCIE_KIRIN_SLT_HI110X) && defined(CONFIG_HISI_DEBUG_FS)
int32_t hi1103_pcie_chip_rc_slt_register(void);
int32_t hi1103_pcie_chip_rc_slt_unregister(void);
#endif
int32_t hi1103_board_flowctrl_gpio_init(void);
void hi1103_free_board_flowctrl_gpio(void);
#ifdef _PRE_HI_DRV_GPIO
int PdmGetHwVer(void); /* Get board type. */
int32_t hitv_get_board_power_gpio(struct platform_device *pdev);
int32_t hitv_board_wakeup_gpio_init(struct platform_device *pdev);
#endif
int32_t enable_board_pmu_clk32k(void);
int32_t disable_board_pmu_clk32k(void);
#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
void hi1103_suspend_gpio(void);
void hi1103_resume_gpio(void);
#endif
#ifdef CONFIG_HWCONNECTIVITY_PC
void hi1103_wlan_wakeup_host_property_init(void);
#endif
#ifdef _PRE_SUSPORT_OEMINFO
int32_t is_hitv_miniproduct(void);
#endif
#endif
