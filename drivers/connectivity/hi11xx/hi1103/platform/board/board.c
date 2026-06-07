

/* 头文件包含 */
#include "board.h"

#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
/*lint -e322*/ /*lint -e7*/
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif

#include <linux/fs.h>
/*lint +e322*/ /*lint +e7*/
#include "plat_debug.h"
#include "oal_ext_if.h"
#include "oal_sdio_host_if.h"
#include "plat_firmware.h"
#include "oal_hcc_bus.h"
#include "plat_pm.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "oneimage.h"
#include "oal_pcie_host.h"

/* GPIO_SSI Base Reg */
#define SSI_SM_CLEAR   0xC  /* 8006 */
#define SSI_AON_CLKSEL 0xE  /* 8007 */
#define SSI_SEL_CTRL   0x10 /* 8008 */
#define SSI_SSI_CTRL   0x12 /* 8009 */

#define SSI_AON_CLKSEL_TCXO 0x0
#define SSI_AON_CLKSEL_SSI  0x1

/* After 1105 */
#define SSI_SYS_CTL_ID 0x1C
#define SSI_GP_REG0    0x20
#define SSI_GP_REG1    0x24
#define SSI_GP_REG2    0x28
#define SSI_GP_REG3    0x2C

#define SSI_RPT_STS_L 0x38
#define SSI_RPT_STS_H 0x3C

#define SSI_SSI_RPT_STS_0 0x40
#define SSI_SSI_RPT_STS_1 0x44
#define SSI_SSI_RPT_STS_2 0x48
#define SSI_SSI_RPT_STS_3 0x4C
#define SSI_SSI_RPT_STS_4 0x50
#define SSI_SSI_RPT_STS_5 0x54
#define SSI_SSI_RPT_STS_6 0x58
#define SSI_SSI_RPT_STS_7 0x5C

#define GPIO_SSI_REG(offset) (0x8000 + ((offset) >> 1))

/* 全局变量定义 */
BOARD_INFO board_info_etc = { .ssi_gpio_clk = 0, .ssi_gpio_data = 0 };
EXPORT_SYMBOL(board_info_etc);

OAL_STATIC int32 board_probe_ret = 0;
OAL_STATIC struct completion board_driver_complete;

int hi11xx_kernel_crash = 0; /* linux kernel crash */
EXPORT_SYMBOL_GPL(hi11xx_kernel_crash);

char str_gpio_ssi_dump_path[100] = HISI_TOP_LOG_DIR "/wifi/memdump";
int ssi_is_logfile = 0;
int ssi_is_pilot = -1;
int ssi_dfr_bypass = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param_string(gpio_ssi_dump_path, str_gpio_ssi_dump_path,
                              sizeof(str_gpio_ssi_dump_path), S_IRUGO | S_IWUSR);
OAL_DEBUG_MODULE_PARM_DESC(gpio_ssi_dump_path, "gpio_ssi dump path");
oal_debug_module_param(ssi_is_logfile, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(ssi_is_pilot, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(ssi_dfr_bypass, int, S_IRUGO | S_IWUSR);

int hi11xx_os_build_variant = HI1XX_OS_BUILD_VARIANT_USER; /* default user mode */
oal_debug_module_param(hi11xx_os_build_variant, int, S_IRUGO | S_IWUSR);
#endif

OAL_DEFINE_SPINLOCK(g_ssi_lock);
oal_uint32 ssi_lock_state = 0x0;

DEVICE_BOARD_VERSION device_board_version_list_etc[BOARD_VERSION_BOTT] = {
    { .index = BOARD_VERSION_HI1102,  .name = BOARD_VERSION_NAME_HI1102 },
    { .index = BOARD_VERSION_HI1103,  .name = BOARD_VERSION_NAME_HI1103 },
    { .index = BOARD_VERSION_HI1102A, .name = BOARD_VERSION_NAME_HI1102A },
    { .index = BOARD_VERSION_HI1105,  .name = BOARD_VERSION_NAME_HI1105 },
};

DOWNLOAD_MODE device_download_mode_list_etc[MODE_DOWNLOAD_BUTT] = {
    { .index = MODE_SDIO, .name = DOWNlOAD_MODE_SDIO },
    { .index = MODE_PCIE, .name = DOWNlOAD_MODE_PCIE },
    { .index = MODE_UART, .name = DOWNlOAD_MODE_UART },
};

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
/* gnss_only uart_cfg */
ssi_file_st aSsiFile[] = {
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /* gnss only */
    { "/system/vendor/firmware/RAM_VECTOR.bin", 0x80100800 },
    { "/system/vendor/firmware/CPU_RAM_SCHED.bin", 0x80004000 },
#else
    /* mpw2 */
    { "/system/vendor/firmware/BCPU_ROM.bin",           0x80000000 },
    { "/system/vendor/firmware/VECTORS.bin",            0x80010000 },
    { "/system/vendor/firmware/RAM_VECTOR.bin",         0x80105c00 },
    { "/system/vendor/firmware/WCPU_ROM.bin",           0x4000 },
    { "/system/vendor/firmware/WL_ITCM.bin",            0x10000 },
    { "/system/vendor/firmware/PLAT_RAM_EXCEPTION.bin", 0x20002800 },
#endif
};
#endif

/* 函数定义 */
int ssi_check_wcpu_is_working(void);
int ssi_check_bcpu_is_working(void);
int ssi_read_reg_info_arry(ssi_reg_info **pst_reg_info, oal_uint32 reg_nums, oal_int32 is_logfile);
inline BOARD_INFO *get_hi110x_board_info_etc(void)
{
    return &board_info_etc;
}

int isAsic_etc(void)
{
    if (board_info_etc.is_asic == VERSION_ASIC) {
        return VERSION_ASIC;
    } else {
        return VERSION_FPGA;
    }
}
EXPORT_SYMBOL_GPL(isAsic_etc);

int is_wifi_support(void)
{
    if (board_info_etc.is_wifi_disable == 0) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(is_wifi_support);

int is_bfgx_support(void)
{
    if (board_info_etc.is_bfgx_disable == 0) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(is_bfgx_support);

int isPmu_clk_request_enable(void)
{
    if (board_info_etc.pmu_clk_share_enable == PMU_CLK_REQ_ENABLE) {
        return PMU_CLK_REQ_ENABLE;
    } else {
        return PMU_CLK_REQ_DISABLE;
    }
}

int32 get_hi110x_subchip_type(void)
{
    BOARD_INFO *bd_info = NULL;

    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        PS_PRINT_ERR("board info is err\n");
        return -EFAIL;
    }

    return bd_info->chip_nr;
}
EXPORT_SYMBOL_GPL(get_hi110x_subchip_type);

#ifdef _PRE_CONFIG_USE_DTS
int32 get_board_dts_node_etc(struct device_node **np, const char *node_prop)
{
    if (np == NULL || node_prop == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, node_prop=%p\n", np, node_prop);
        return BOARD_FAIL;
    }

    *np = of_find_compatible_node(NULL, NULL, node_prop);
    if (*np == NULL) {
        PS_PRINT_ERR("No compatible node %s found.\n", node_prop);
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32 get_board_dts_prop_etc(struct device_node *np, const char *dts_prop, const char **prop_val)
{
    int32 ret;

    if (np == NULL || dts_prop == NULL || prop_val == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_property_read_string(np, dts_prop, prop_val);
    if (ret) {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s\n", dts_prop);
        return ret;
    }

    PS_PRINT_SUC("have get dts_prop and prop_val: %s=%s\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

int32 get_board_dts_gpio_prop_etc(struct device_node *np, const char *dts_prop, int32 *prop_val)
{
    int32 ret;

    if (np == NULL || dts_prop == NULL || prop_val == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_get_named_gpio(np, dts_prop, 0);
    if (ret < 0) {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s, ret=%d\n", dts_prop, ret);
        return ret;
    }

    *prop_val = ret;
    PS_PRINT_SUC("have get dts_prop and prop_val: %s=%d\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

#endif

int32 get_board_gpio_etc(const char *gpio_node, const char *gpio_prop, int32 *physical_gpio)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node_etc(&np, gpio_node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = get_board_dts_gpio_prop_etc(np, gpio_prop, physical_gpio);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}

int32 get_board_custmize_etc(const char *cust_node, const char *cust_prop, const char **cust_prop_val)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    if (cust_node == NULL || cust_prop == NULL || cust_prop_val == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = get_board_dts_node_etc(&np, cust_node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = get_board_dts_prop_etc(np, cust_prop, cust_prop_val);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("get board customize info %s=%s\n", cust_prop, *cust_prop_val);

    return BOARD_SUCC;
#else
    return BOARD_FAIL;
#endif
}

int32 get_board_pmu_clk32k_etc(void)
{
    return board_info_etc.bd_ops.get_board_pmu_clk32k_etc();
}

int32 set_board_pmu_clk32k_etc(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    const char *clk_name = NULL;
    struct clk *clk = NULL;
    struct device *dev = NULL;

    dev = &pdev->dev;
    clk_name = board_info_etc.clk_32k_name;
    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1102) {
        clk = devm_clk_get(dev, "clk_pmu32kb");
    } else {
        clk = devm_clk_get(dev, clk_name);
    }

    if (clk == NULL) {
        PS_PRINT_ERR("Get 32k clk %s failed!!!\n", clk_name);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                             CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK);
        return BOARD_FAIL;
    }
    board_info_etc.clk_32k = clk;

    ret = clk_prepare_enable(clk);
    if (unlikely(ret < 0)) {
        devm_clk_put(dev, clk);
        PS_PRINT_ERR("enable 32K clk %s failed!!!", clk_name);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                             CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK);
        return BOARD_FAIL;
    }
#endif
    return BOARD_SUCC;
}

int32 get_board_uart_port_etc(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.get_board_uart_port_etc();
#else
    return BOARD_SUCC;
#endif
}

int32 check_evb_or_fpga_etc(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.check_evb_or_fpga_etc();
#else
    return BOARD_SUCC;
#endif
}

int32 check_hi110x_subsystem_support(void)
{
    if (board_info_etc.bd_ops.check_hi110x_subsystem_support == NULL) {
        return BOARD_SUCC;
    }

    return board_info_etc.bd_ops.check_hi110x_subsystem_support();
}

int32 check_pmu_clk_share_etc(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.check_pmu_clk_share_etc();
#else
    return BOARD_SUCC;
#endif
}

int32 board_get_power_pinctrl_etc(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    return board_info_etc.bd_ops.board_get_power_pinctrl_etc(pdev);
#else
    return BOARD_SUCC;
#endif
}

int32 board_power_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.get_board_power_gpio();
}
void free_board_power_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_power_gpio_etc();
}
#ifdef HAVE_HISI_IR
void free_board_ir_gpio(void)
{
    if (board_info_etc.bfgx_ir_ctrl_gpio > -1) {
        gpio_free(board_info_etc.bfgx_ir_ctrl_gpio);
    }
}
#endif
void free_board_wakeup_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_wakeup_gpio_etc();
}

void free_board_wifi_tas_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_wifi_tas_gpio_etc();
}

int32 board_wakeup_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.board_wakeup_gpio_init_etc();
}

int32 board_wifi_tas_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.board_wifi_tas_gpio_init_etc();
}
void free_board_flowctrl_gpio_etc(void)
{
    board_info_etc.bd_ops.free_board_flowctrl_gpio_etc();
}

int32 board_flowctrl_gpio_init_etc(void)
{
    return board_info_etc.bd_ops.board_flowctrl_gpio_init_etc();
}

void board_flowctrl_irq_init_etc(void)
{
    board_info_etc.flowctrl_irq = oal_gpio_to_irq(board_info_etc.flowctrl_gpio);
}

#ifdef HAVE_HISI_IR
int32 board_ir_ctrl_init(struct platform_device *pdev)
{
    return board_info_etc.bd_ops.board_ir_ctrl_init(pdev);
}
#endif

int32 board_gpio_init_etc(struct platform_device *pdev)
{
    int32 ret;

    /* power on gpio request */
    ret = board_power_gpio_init_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get power_on dts prop failed\n");
        goto err_get_power_on_gpio;
    }

    ret = board_wakeup_gpio_init_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get wakeup prop failed\n");
        goto oal_board_wakup_gpio_fail;
    }

    ret = board_wifi_tas_gpio_init_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get wifi tas prop failed\n");
        goto oal_board_wifi_tas_gpio_fail;
    }

#ifdef HAVE_HISI_IR
    ret = board_ir_ctrl_init(pdev);
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("get ir dts prop failed\n");
        goto err_get_ir_ctrl_gpio;
    }
#endif

    return BOARD_SUCC;

#ifdef HAVE_HISI_IR
err_get_ir_ctrl_gpio:
    free_board_wifi_tas_gpio_etc();
#endif
oal_board_wifi_tas_gpio_fail:
    free_board_wakeup_gpio_etc();
oal_board_wakup_gpio_fail:
    free_board_power_gpio_etc();
err_get_power_on_gpio:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                         CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO);
    return BOARD_FAIL;
}

int board_get_bwkup_gpio_val_etc(void)
{
    return gpio_get_value(board_info_etc.bfgn_wakeup_host);
}

int board_get_wlan_wkup_gpio_val_etc(void)
{
    return gpio_get_value(board_info_etc.wlan_wakeup_host);
}

int32 board_irq_init_etc(void)
{
    uint32 irq;
    int32 gpio;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    gpio = board_info_etc.wlan_wakeup_host;
    irq = gpio_to_irq(gpio);
    board_info_etc.wlan_irq = irq;

    PS_PRINT_INFO("wlan_irq is %d\n", board_info_etc.wlan_irq);
#endif

    gpio = board_info_etc.bfgn_wakeup_host;
    irq = gpio_to_irq(gpio);
    board_info_etc.bfgx_irq = irq;

    PS_PRINT_INFO("bfgx_irq is %d\n", board_info_etc.bfgx_irq);

    return BOARD_SUCC;
}

int32 board_clk_init_etc(struct platform_device *pdev)
{
    int32 ret;

    if (pdev == NULL) {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = board_info_etc.bd_ops.get_board_pmu_clk32k_etc();
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = set_board_pmu_clk32k_etc(pdev);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

void power_state_change_etc(int32 gpio, int32 flag)
{
    if (unlikely(gpio == 0)) {
        PS_PRINT_WARNING("gpio is 0, flag=%d\n", flag);
        return;
    }

    PS_PRINT_INFO("power_state_change_etc gpio %d to %s\n", gpio, (flag == BOARD_POWER_ON) ? "low2high" : "low");

    if (flag == BOARD_POWER_ON) {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
        mdelay(10);
        gpio_direction_output(gpio, GPIO_HIGHLEVEL);
        mdelay(20);
    } else if (flag == BOARD_POWER_OFF) {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
    }
}

int32 board_wlan_gpio_power_on(void *data)
{
    int32 gpio = (int32)(uintptr_t)(data);
    if (board_info_etc.host_wakeup_wlan) {
        /* host wakeup dev gpio pinmux to jtag when w boot,
          must gpio low when bootup */
        board_host_wakeup_dev_set(0);
    }
    power_state_change_etc(gpio, BOARD_POWER_ON);
    board_host_wakeup_dev_set(1);
    return 0;
}

int32 board_wlan_gpio_power_off(void *data)
{
    int32 gpio = (int32)(uintptr_t)(data);
    power_state_change_etc(gpio, BOARD_POWER_OFF);
    return 0;
}

int32 board_host_wakeup_dev_set(int value)
{
    if (board_info_etc.host_wakeup_wlan == 0) {
        PS_PRINT_INFO("host_wakeup_wlan gpio is 0\n");
        return 0;
    }
    PS_PRINT_DBG("host_wakeup_wlan set %s %pF\n", value ? "high" : "low", (void *)_RET_IP_);
    if (value) {
        return gpio_direction_output(board_info_etc.host_wakeup_wlan, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(board_info_etc.host_wakeup_wlan, GPIO_LOWLEVEL);
    }
}

int32 board_get_host_wakeup_dev_stat(void)
{
    return gpio_get_value(board_info_etc.host_wakeup_wlan);
}

int32 board_wifi_tas_set(int value)
{
    if (board_info_etc.wifi_tas_enable == WIFI_TAS_DISABLE) {
        return 0;
    }

    PS_PRINT_DBG("wifi tas gpio set %s %pF\n", value ? "high" : "low", (void *)_RET_IP_);

    if (value) {
        return gpio_direction_output(board_info_etc.rf_wifi_tas, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(board_info_etc.rf_wifi_tas, GPIO_LOWLEVEL);
    }
}

EXPORT_SYMBOL(board_wifi_tas_set);

int32 board_get_wifi_tas_gpio_state(void)
{
    return gpio_get_value(board_info_etc.rf_wifi_tas);
}

EXPORT_SYMBOL(board_get_wifi_tas_gpio_state);

int32 board_power_on_etc(uint32 ul_subsystem)
{
    return board_info_etc.bd_ops.board_power_on_etc(ul_subsystem);
}
int32 board_power_off_etc(uint32 ul_subsystem)
{
    return board_info_etc.bd_ops.board_power_off_etc(ul_subsystem);
}

int32 board_power_reset(uint32 ul_subsystem)
{
    return board_info_etc.bd_ops.board_power_reset(ul_subsystem);
}
EXPORT_SYMBOL(board_wlan_gpio_power_off);
EXPORT_SYMBOL(board_wlan_gpio_power_on);

/* just for hi1105 fpga test, delete later */
void hi1105_subchip_check_tmp(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node_etc(&np, DTS_NODE_HISI_HI110X);
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("DTS read node %s fail!!!\n", DTS_NODE_HISI_HI110X);
        return;
    }

    ret = of_property_read_bool(np, BOARD_VERSION_NAME_HI1105);
    if (ret) {
        PS_PRINT_INFO("hisi subchip type is hi1105\n");
        board_info_etc.chip_type = BOARD_VERSION_NAME_HI1105;
    }

    return;
#else
    return;
#endif
}

int32 find_device_board_version_etc(void)
{
    int32 ret;
    const char *device_version = NULL;

    ret = get_board_custmize_etc(DTS_NODE_HISI_HI110X, DTS_PROP_SUBCHIP_TYPE_VERSION, &device_version);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    board_info_etc.chip_type = device_version;
    return BOARD_SUCC;
}

int32 board_chiptype_init(void)
{
    int32 ret;

    ret = find_device_board_version_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("can not find device_board_version\n");
        return BOARD_FAIL;
    }

    /* just for hi1105 fpga test, delete later */
    hi1105_subchip_check_tmp();

    ret = check_device_board_name_etc();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("check device name fail\n");
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32 board_func_init(void)
{
    int32 ret;
    // board init
    memset_s(&board_info_etc, sizeof(board_info_etc), 0, sizeof(board_info_etc));

    board_info_etc.is_wifi_disable = 0;
    board_info_etc.is_bfgx_disable = 0;

    ret = board_chiptype_init();
    if (ret != BOARD_SUCC) {
        PS_PRINT_ERR("sub chiptype init fail\n");
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("hi11xx subchip is %s\n", board_info_etc.chip_type);

    switch (board_info_etc.chip_nr) {
        case BOARD_VERSION_HI1102:
            board_info_etc.bd_ops.get_board_power_gpio = hi1102_get_board_power_gpio;
            board_info_etc.bd_ops.free_board_power_gpio_etc = hi1102_free_board_power_gpio_etc;
            board_info_etc.bd_ops.board_wakeup_gpio_init_etc = hi1102_board_wakeup_gpio_init_etc;
            board_info_etc.bd_ops.free_board_wakeup_gpio_etc = hi1102_free_board_wakeup_gpio_etc;
            board_info_etc.bd_ops.bfgx_dev_power_on_etc = hi1102_bfgx_dev_power_on;
            board_info_etc.bd_ops.bfgx_dev_power_off_etc = hi1102_bfgx_dev_power_off;
            board_info_etc.bd_ops.wlan_power_off_etc = hi1102_wlan_power_off;
            board_info_etc.bd_ops.wlan_power_on_etc = hi1102_wlan_power_on;
            board_info_etc.bd_ops.board_power_on_etc = hi1102_board_power_on;
            board_info_etc.bd_ops.board_power_off_etc = hi1102_board_power_off;
            board_info_etc.bd_ops.board_power_reset = hi1102_board_power_reset;
            board_info_etc.bd_ops.get_board_pmu_clk32k_etc = hi1102_get_board_pmu_clk32k;
            board_info_etc.bd_ops.get_board_uart_port_etc = hi1102_get_board_uart_port;
            board_info_etc.bd_ops.board_ir_ctrl_init = hi1102_board_ir_ctrl_init;
            board_info_etc.bd_ops.check_evb_or_fpga_etc = hi1102_check_evb_or_fpga;
            board_info_etc.bd_ops.board_get_power_pinctrl_etc = hi1102_board_get_power_pinctrl;
            board_info_etc.bd_ops.get_ini_file_name_from_dts_etc = hi1102_get_ini_file_name_from_dts;
            break;
        case BOARD_VERSION_HI1103:
        case BOARD_VERSION_HI1105:
            board_info_etc.bd_ops.get_board_power_gpio = hi1103_get_board_power_gpio;
            board_info_etc.bd_ops.free_board_power_gpio_etc = hi1103_free_board_power_gpio_etc;
            board_info_etc.bd_ops.free_board_flowctrl_gpio_etc   =hi1103_free_board_flowctrl_gpio_etc;
            board_info_etc.bd_ops.board_wakeup_gpio_init_etc = hi1103_board_wakeup_gpio_init_etc;
            board_info_etc.bd_ops.free_board_wakeup_gpio_etc = hi1103_free_board_wakeup_gpio_etc;
            board_info_etc.bd_ops.board_flowctrl_gpio_init_etc   =hi1103_board_flowctrl_gpio_init_etc;
            board_info_etc.bd_ops.board_wifi_tas_gpio_init_etc = hi1103_board_wifi_tas_gpio_init_etc;
            board_info_etc.bd_ops.free_board_wifi_tas_gpio_etc = hi1103_free_board_wifi_tas_gpio_etc;
            board_info_etc.bd_ops.bfgx_dev_power_on_etc = hi1103_bfgx_dev_power_on;
            board_info_etc.bd_ops.bfgx_dev_power_off_etc = hi1103_bfgx_dev_power_off;
            board_info_etc.bd_ops.wlan_power_off_etc = hi1103_wlan_power_off;
            board_info_etc.bd_ops.wlan_power_on_etc = hi1103_wlan_power_on;
            board_info_etc.bd_ops.board_power_on_etc = hi1103_board_power_on;
            board_info_etc.bd_ops.board_power_off_etc = hi1103_board_power_off;
            board_info_etc.bd_ops.board_power_reset = hi1103_board_power_reset;
            board_info_etc.bd_ops.get_board_pmu_clk32k_etc = hi1103_get_board_pmu_clk32k;
            board_info_etc.bd_ops.get_board_uart_port_etc = hi1103_get_board_uart_port;
            board_info_etc.bd_ops.board_ir_ctrl_init = hi1103_board_ir_ctrl_init;
            board_info_etc.bd_ops.check_evb_or_fpga_etc = hi1103_check_evb_or_fpga;
            board_info_etc.bd_ops.check_pmu_clk_share_etc = hi1103_check_pmu_clk_share;
            board_info_etc.bd_ops.board_get_power_pinctrl_etc = hi1103_board_get_power_pinctrl;
            board_info_etc.bd_ops.get_ini_file_name_from_dts_etc = hi1103_get_ini_file_name_from_dts;
            board_info_etc.bd_ops.check_hi110x_subsystem_support = hi1103_check_hi110x_subsystem_support;
            break;
        default:
            PS_PRINT_ERR("board_info_etc.chip_nr=%d is illegal\n", board_info_etc.chip_nr);
            return BOARD_FAIL;
    }

    PS_PRINT_INFO("board_info_etc.chip_nr=%d, device_board_version is %s\n",
                  board_info_etc.chip_nr, board_info_etc.chip_type);
    return BOARD_SUCC;
}

int32 check_download_channel_name_etc(const char *wlan_buff, int32 *index)
{
    int32 i = 0;
    for (i = 0; i < MODE_DOWNLOAD_BUTT; i++) {
        if (strncmp(device_download_mode_list_etc[i].name, wlan_buff, strlen(device_download_mode_list_etc[i].name)) ==
            0) {
            *index = i;
            return BOARD_SUCC;
        }
    }
    return BOARD_FAIL;
}

int32 get_download_channel_etc(void)
{
    int32 ret;
    uint8 wlan_mode[DOWNLOAD_CHANNEL_LEN] = {0};
    uint8 bfgn_mode[DOWNLOAD_CHANNEL_LEN] = {0};

    /* wlan channel */
    ret = find_download_channel_etc(wlan_mode, sizeof(wlan_mode), INI_WLAN_DOWNLOAD_CHANNEL);
    if (ret != BOARD_SUCC) {
        /* 兼容1102,1102无此配置项 */
        board_info_etc.wlan_download_channel = MODE_SDIO;
        PS_PRINT_WARNING("can not find wlan_download_channel ,choose default:%s\n",
                         device_download_mode_list_etc[0].name);
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, NULL);
    } else {
        if (check_download_channel_name_etc(wlan_mode, &(board_info_etc.wlan_download_channel)) != BOARD_SUCC) {
            PS_PRINT_ERR("check wlan download channel:%s error\n", bfgn_mode);
            return BOARD_FAIL;
        }
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, wlan_mode);
    }

    /* bfgn channel */
    ret = find_download_channel_etc(bfgn_mode, sizeof(bfgn_mode), INI_BFGX_DOWNLOAD_CHANNEL);
    if (ret != BOARD_SUCC) {
        /* 如果不存在该项，则默认保持和wlan一致 */
        board_info_etc.bfgn_download_channel = board_info_etc.wlan_download_channel;
        PS_PRINT_WARNING("can not find bfgn_download_channel ,choose default:%s\n",
                         device_download_mode_list_etc[0].name);
        return BOARD_SUCC;
    }

    if (check_download_channel_name_etc(bfgn_mode, &(board_info_etc.bfgn_download_channel)) != BOARD_SUCC) {
        PS_PRINT_ERR("check bfgn download channel:%s error\n", bfgn_mode);
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("wlan_download_channel index:%d, bfgn_download_channel index:%d\n",
                  board_info_etc.wlan_download_channel, board_info_etc.bfgn_download_channel);

    return BOARD_SUCC;
}

uint32 ssi_dump_en = 0;
int32 get_ssi_dump_cfg(void)
{
    int32 l_cfg_value = 0;
    int32 l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32_etc(INI_MODU_PLAT, INI_SSI_DUMP_EN, &l_cfg_value);

    if (l_ret == INI_FAILED) {
        PS_PRINT_ERR("get_ssi_dump_cfg: fail to get ini, keep disable\n");
        return BOARD_SUCC;
    }

    ssi_dump_en = (uint32)l_cfg_value;

    PS_PRINT_INFO("get_ssi_dump_cfg: 0x%x\n", ssi_dump_en);

    return BOARD_SUCC;
}

int32 check_device_board_name_etc(void)
{
    int32 i = 0;
    for (i = 0; i < BOARD_VERSION_BOTT; i++) {
        if (strncmp(device_board_version_list_etc[i].name, board_info_etc.chip_type, HI11XX_SUBCHIP_NAME_LEN_MAX) ==
            0) {
            board_info_etc.chip_nr = i;
            return BOARD_SUCC;
        }
    }

    return BOARD_FAIL;
}

int32 get_uart_pclk_source_etc(void)
{
    return board_info_etc.uart_pclk;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
STATIC void board_cci_bypass_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret;
    struct device_node *np = NULL;

    if (pcie_memcopy_type == -1) {
        PS_PRINT_INFO("skip pcie mem burst control\n");
        return;
    }

    ret = get_board_dts_node_etc(&np, DTS_NODE_HISI_CCIBYPASS);
    if (ret != BOARD_SUCC) {
        /* cci enable */
        pcie_memcopy_type = 0;
        PS_PRINT_INFO("cci enable, pcie use mem burst 8 bytes\n");
    } else {
        /* cci bypass */
        pcie_memcopy_type = 1;
        PS_PRINT_INFO("cci bypass, pcie use mem burst 4 bytes\n");
    }
#endif
}
#endif

STATIC int32 hi110x_board_probe(struct platform_device *pdev)
{
    int ret;

    PS_PRINT_INFO("hi110x board init\n");
    ret = board_func_init();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = ini_cfg_init_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }
    ret = check_hi110x_subsystem_support();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = check_evb_or_fpga_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = check_pmu_clk_share_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = get_download_channel_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = get_ssi_dump_cfg();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_clk_init_etc(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    board_cci_bypass_init();
#endif

    ret = get_board_uart_port_etc();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_gpio_init_etc(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_irq_init_etc();
    if (ret != BOARD_SUCC) {
        goto err_gpio_source;
    }

    ret = board_get_power_pinctrl_etc(pdev);
    if (ret != BOARD_SUCC) {
        goto err_get_power_pinctrl;
    }

    PS_PRINT_INFO("board init ok\n");

    board_probe_ret = BOARD_SUCC;
    complete(&board_driver_complete);

    return BOARD_SUCC;

err_get_power_pinctrl:

err_gpio_source:
#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif
    free_board_wakeup_gpio_etc();
    free_board_power_gpio_etc();
    free_board_flowctrl_gpio_etc();

err_init:
    board_probe_ret = BOARD_FAIL;
    complete(&board_driver_complete);
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                         CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_POWER_GPIO);
    return BOARD_FAIL;
}

STATIC int32 hi110x_board_remove(struct platform_device *pdev)
{
    PS_PRINT_INFO("hi110x board exit\n");

#ifdef _PRE_CONFIG_USE_DTS
    if (board_info_etc.need_power_prepare == NEED_POWER_PREPARE) {
        devm_pinctrl_put(board_info_etc.pctrl);
    }
#endif

#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif

    free_board_wakeup_gpio_etc();
    free_board_power_gpio_etc();
    free_board_flowctrl_gpio_etc();

    return BOARD_SUCC;
}

int32 hi110x_board_suspend_etc(struct platform_device *pdev, pm_message_t state)
{
    return BOARD_SUCC;
}

int32 hi110x_board_resume_etc(struct platform_device *pdev)
{
    return BOARD_SUCC;
}

/* SSI调试代码start */
#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#define HI110X_SSI_CLK_GPIO_NAME  "hi110x ssi clk"
#define HI110X_SSI_DATA_GPIO_NAME "hi110x ssi data"
#define INTERVAL_TIME             10
#define SSI_DATA_LEN              16
#define SSI_CPU_ARM_REG_DUMP_CNT  2

/* 以下寄存器同1103 device定义 */
#define HI1103_W_CTL_BASE                   0x40000000
#define HI1103_W_CTL_WTOPCRG_SOFT_RESET_REG (HI1103_W_CTL_BASE + 0x30)

#define HI1103_WCPU_PATCH_BASE                  0x40004000
#define HI1103_WCPU_PATCH_WCPU_CFG_TRACE_EN_REG (HI1103_WCPU_PATCH_BASE + 0xC00)

#define HI1103_BCPU_PATCH_BASE                  0x48007000
#define HI1103_BCPU_PATCH_BCPU_CFG_TRACE_EN_REG (HI1103_BCPU_PATCH_BASE + 0xC00)

#define HI1103_GLB_CTL_BASE                    0x50000000
#define HI1103_GLB_CTL_SOFT_RST_BCPU_REG       (HI1103_GLB_CTL_BASE + 0x94)
#define HI1103_GLB_CTL_SYS_TICK_CFG_W_REG      (HI1103_GLB_CTL_BASE + 0xC0) /* 写1清零systick，写0无效 */
#define HI1103_GLB_CTL_SYS_TICK_VALUE_W_0_REG  (HI1103_GLB_CTL_BASE + 0xD0)
#define HI1103_GLB_CTL_PWR_ON_LABLE_REG        (HI1103_GLB_CTL_BASE + 0x200) /* 芯片上电标记寄存器 */
#define HI1103_GLB_CTL_WCPU_LOAD_REG           (HI1103_GLB_CTL_BASE + 0x400) /* WCPU_LOAD */
#define HI1103_GLB_CTL_WCPU_PC_L_REG           (HI1103_GLB_CTL_BASE + 0x404) /* WCPU_PC低16bit */
#define HI1103_GLB_CTL_WCPU_PC_H_REG           (HI1103_GLB_CTL_BASE + 0x408) /* WCPU_PC高16bit */
#define HI1103_GLB_CTL_WCPU_LR_L_REG           (HI1103_GLB_CTL_BASE + 0x40C) /* WCPU_LR低16bit */
#define HI1103_GLB_CTL_WCPU_LR_H_REG           (HI1103_GLB_CTL_BASE + 0x410) /* WCPU_LR高16bit */
#define HI1103_GLB_CTL_WCPU_SP_L_REG           (HI1103_GLB_CTL_BASE + 0x414) /* WCPU_SP低16bit */
#define HI1103_GLB_CTL_WCPU_SP_H_REG           (HI1103_GLB_CTL_BASE + 0x418) /* WCPU_SP高16bit */
#define HI1103_GLB_CTL_BCPU_LOAD_REG           (HI1103_GLB_CTL_BASE + 0x420) /* BCPU_LOAD */
#define HI1103_GLB_CTL_BCPU_PC_L_REG           (HI1103_GLB_CTL_BASE + 0x424) /* BCPU_PC低16bit */
#define HI1103_GLB_CTL_BCPU_PC_H_REG           (HI1103_GLB_CTL_BASE + 0x428) /* BCPU_PC高16bit */
#define HI1103_GLB_CTL_BCPU_LR_L_REG           (HI1103_GLB_CTL_BASE + 0x42C) /* BCPU_LR低16bit */
#define HI1103_GLB_CTL_BCPU_LR_H_REG           (HI1103_GLB_CTL_BASE + 0x430) /* BCPU_LR高16bit */
#define HI1103_GLB_CTL_BCPU_SP_L_REG           (HI1103_GLB_CTL_BASE + 0x434) /* BCPU_SP低16bit */
#define HI1103_GLB_CTL_BCPU_SP_H_REG           (HI1103_GLB_CTL_BASE + 0x438) /* BCPU_SP高16bit */
#define HI1103_GLB_CTL_TCXO_DET_CTL_REG        (HI1103_GLB_CTL_BASE + 0x700) /* TCXO时钟检测控制寄存器 */
#define HI1103_GLB_CTL_TCXO_32K_DET_CNT_REG    (HI1103_GLB_CTL_BASE + 0x704) /* TCXO时钟检测控制寄存器 */
#define HI1103_GLB_CTL_TCXO_32K_DET_RESULT_REG (HI1103_GLB_CTL_BASE + 0x708) /* TCXO时钟检测控制寄存器 */
#define HI1103_GLB_CTL_WCPU_WAIT_CTL_REG       (HI1103_GLB_CTL_BASE + 0xE00)
#define HI1103_GLB_CTL_BCPU_WAIT_CTL_REG       (HI1103_GLB_CTL_BASE + 0xE04)

#define HI1103_PMU_CMU_CTL_BASE                   0x50002000
#define HI1103_PMU_CMU_CTL_SYS_STATUS_0_REG       (HI1103_PMU_CMU_CTL_BASE + 0x200) /* 系统状态 */
#define HI1103_PMU_CMU_CTL_PMU_PROTECT_STATUS_REG (HI1103_PMU_CMU_CTL_BASE + 0x380) /* PMU状态查询 */

#define HI1103_PMU2_CMU_IR_BASE                   0x50003000
#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_2_REG (HI1103_PMU2_CMU_IR_BASE + 0x15C) /* PMU2_CMU_ABB 实际状态 */
#define HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_3_REG (HI1103_PMU2_CMU_IR_BASE + 0x17C) /* PMU2_CMU_ABB 实际状态 */
#define HI1103_PMU2_CMU_IR_CMU_RESERVE1_REG       (HI1103_PMU2_CMU_IR_BASE + 0x338) /* RESERVE 控制 */

/* WL_C0_ABB_RF电源控制 */
#define HI1103_PMU2_CMU_IR_SYSLDO_WL_C0_ABB_RF_PWR_EN_STS_REG (HI1103_PMU2_CMU_IR_BASE + 0xA88)

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#define SSI_CLK_GPIO  89
#define SSI_DATA_GPIO 91
#else
#define SSI_CLK_GPIO  75
#define SSI_DATA_GPIO 77
#endif

char *ssi_hi1103_mpw2_cpu_st_str[] = {
    "OFF",   /* 0x0 */
    "SLEEP", /* 0x1 */
    "IDLE",  /* 0x2 */
    "WORK"   /* 0x3 */
};
char *ssi_hi1103_pilot_cpu_st_str[] = {
    "OFF",              /* 0x0 */
    "BOOTING",          /* 0x1 */
    "SLEEPING",         /* 0x2 */
    "WORK",             /* 0x3 */
    "SAVING",           /* 0x4 */
    "PROTECT(ocp/scp)", /* 0x5 */
    "SLEEP",            /* 0x6 */
    "PROTECTING"        /* 0x7 */
};

static uint32 halt_det_cnt = 0; /* 检测soc异常次数 */
typedef struct _ssi_cpu_info_ {
    uint32 cpu_state;
    uint32 pc[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 lr[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 sp[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 reg_flag[SSI_CPU_ARM_REG_DUMP_CNT];
} ssi_cpu_info;

typedef struct _ssi_cpu_infos_ {
    ssi_cpu_info wcpu_info;
    ssi_cpu_info bcpu_info;
} ssi_cpu_infos;

static ssi_cpu_infos st_ssi_cpu_infos;

#define SSI_WRITE_DATA 0x5a5a
ssi_trans_test_st ssi_test_st = {0};

uint32 ssi_clk_etc = 0;              /* 模拟ssi时钟的GPIO管脚号 */
uint32 ssi_data_etc = 0;             /* 模拟ssi数据线的GPIO管脚号 */
uint16 ssi_base_etc = 0x8000;        /* ssi基址 */
uint32 interval_etc = INTERVAL_TIME; /* GPIO拉出来的波形保持时间，单位us */
uint32 delay_etc = 5;

/* ssi 工作时必须切换ssi clock, 此时aon会受到影响，BCPU/WCPU 有可能异常，慎用! */
int32 ssi_try_lock(void)
{
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    if (ssi_lock_state) {
        /* lock failed */
        oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
        return 1;
    }
    ssi_lock_state = 1;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}

int32 ssi_unlock(void)
{
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    ssi_lock_state = 0;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}
int32 wait_for_ssi_idle_timeout(int32 mstimeout)
{
    int32 can_sleep = 0;
    int32 timeout = mstimeout;
    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        can_sleep = 0;
    } else {
        can_sleep = 1;
    }
    /* 考虑效率，这里需要判断是否可以睡眠 */
    while (ssi_try_lock()) {
        if (can_sleep) {
            msleep(1);
        } else {
            mdelay(1);
        }
        if (!(--timeout)) {
            PS_PRINT_ERR("wait for ssi timeout:%dms\n", mstimeout);
            return 0;
        }
    }
    ssi_unlock();
    return timeout;
}
int32 ssi_show_setup_etc(void)
{
    PS_PRINT_INFO("clk=%d, data=%d, interval=%d us, ssi base=0x%x, r/w delay=%d cycle\n",
                  ssi_clk_etc, ssi_data_etc, interval_etc, ssi_base_etc, delay_etc);
    return BOARD_SUCC;
}

int32 ssi_setup_etc(uint32 interval, uint32 delay, uint16 ssi_base)
{
    interval_etc = interval;
    delay_etc = delay;
    ssi_base_etc = ssi_base;

    return BOARD_SUCC;
}

int32 ssi_request_gpio_etc(uint32 clk, uint32 data)
{
    int32 ret;

    PS_PRINT_DBG("request hi110x ssi GPIO\n");
#ifdef GPIOF_OUT_INIT_LOW
    ret = gpio_request_one(clk, GPIOF_OUT_INIT_LOW, HI110X_SSI_CLK_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    ssi_clk_etc = clk;

    ret = gpio_request_one(data, GPIOF_OUT_INIT_LOW, HI110X_SSI_DATA_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }
#else
    ret = gpio_request(clk, HI110X_SSI_CLK_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    gpio_direction_output(clk, 0);

    ret = gpio_request(data, HI110X_SSI_DATA_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }

    gpio_direction_output(data, 0);
#endif
    ssi_data_etc = data;

    return BOARD_SUCC;

err_get_ssi_data_gpio:
    gpio_free(clk);
    ssi_clk_etc = 0;
err_get_ssi_clk_gpio:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV,
                         CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO);

    return ret;
}

int32 ssi_free_gpio_etc(void)
{
    PS_PRINT_DBG("free hi110x ssi GPIO\n");

    if (ssi_clk_etc != 0) {
        gpio_free(ssi_clk_etc);
        ssi_clk_etc = 0;
    }

    if (ssi_data_etc != 0) {
        gpio_free(ssi_data_etc);
        ssi_data_etc = 0;
    }

    return BOARD_SUCC;
}

void ssi_clk_output_etc(void)
{
    gpio_direction_output(ssi_clk_etc, GPIO_LOWLEVEL);
    SSI_DELAY(interval_etc);
    gpio_direction_output(ssi_clk_etc, GPIO_HIGHLEVEL);
}

void ssi_data_output_etc(uint16 data)
{
    SSI_DELAY(5);
    if (data) {
        gpio_direction_output(ssi_data_etc, GPIO_HIGHLEVEL);
    } else {
        gpio_direction_output(ssi_data_etc, GPIO_LOWLEVEL);
    }

    SSI_DELAY(interval_etc);
}

int32 ssi_write_data_etc(uint16 addr, uint16 value)
{
    uint16 tx;
    uint32 i;

    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    /* 发送SYNC位 */
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 指示本次操作为写，高读低写 */
    PS_PRINT_DBG("tx r/w->w\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(0);

    /* 发送地址 */
    PS_PRINT_DBG("write addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 发送数据 */
    PS_PRINT_DBG("write value:0x%x\n", value);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (value >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx data bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 数据发送完成以后，保持delay个周期的0 */
    PS_PRINT_DBG("ssi write:finish, delay %d cycle\n", delay_etc);
    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    return BOARD_SUCC;
}

uint16 ssi_read_data_etc(uint16 addr)
{
#define SSI_READ_RETTY 1000
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    /* 发送SYNC位 */
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 指示本次操作为读，高读低写 */
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 发送地址 */
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 延迟一个clk，否则上一个数据只保持了半个时钟周期 */
    ssi_clk_output_etc();

    /* 设置data线GPIO为输入，准备读取数据 */
    gpio_direction_input(ssi_data_etc);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(ssi_data_etc));

    /* 读取SYNC同步位 */
    do {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        if (gpio_get_value(ssi_data_etc)) {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    } while (retry != SSI_READ_RETTY);

    if (retry == SSI_READ_RETTY) {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output_etc(0);
        return data;
    }

    for (i = 0; i < SSI_DATA_LEN; i++) {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        rx = gpio_get_value(ssi_data_etc);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /* 恢复data线GPIO为输出，并输出0 */
    ssi_data_output_etc(0);

    return data;
}

int32 ssi_write16_etc(uint16 addr, uint16 value)
{
#define write_retry 3
    uint32 retry = 0;
    uint16 read_v;

    do {
        ssi_write_data_etc(addr, value);
        read_v = ssi_read_data_etc(addr);
        if (value == read_v) {
            PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);
            return BOARD_SUCC;
        }
        retry++;
    } while (retry < write_retry);

    PS_PRINT_ERR("ssi write: 0x%x=0x%x ,read=0x%x fail\n", addr, value, read_v);

    return BOARD_FAIL;
}

uint16 ssi_read16_etc(uint16 addr)
{
    uint16 data;

    data = ssi_read_data_etc(addr);

    PS_PRINT_SUC("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_write32_etc(uint32 addr, uint16 value)
{
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;

    /* 往基地址写地址的高16位 */
    if (ssi_write16_etc(ssi_base_etc, addr_half_word_high) < 0) {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* 低地址写实际要写入的value */
    if (ssi_write16_etc(addr_half_word_low, value) < 0) {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

int32 ssi_read32_etc(uint32 addr)
{
    uint16 data;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;

    if (ssi_write16_etc(ssi_base_etc, addr_half_word_high) < 0) {
        PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
        return BOARD_FAIL;
    }

    data = ssi_read_data_etc(addr_half_word_low);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_read_data16(uint16 addr, uint16 *value)
{
#define SSI_READ_RETTY 1000
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    /* 发送SYNC位 */
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 指示本次操作为读，高读低写 */
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 发送地址 */
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 延迟一个clk，否则上一个数据只保持了半个时钟周期 */
    ssi_clk_output_etc();

    /* 设置data线GPIO为输入，准备读取数据 */
    gpio_direction_input(ssi_data_etc);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(ssi_data_etc));

    /* 读取SYNC同步位 */
    do {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        if (gpio_get_value(ssi_data_etc)) {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    } while (retry != SSI_READ_RETTY);

    if (retry == SSI_READ_RETTY) {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output_etc(0);
        return -OAL_EFAIL;
    }

    for (i = 0; i < SSI_DATA_LEN; i++) {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        rx = gpio_get_value(ssi_data_etc);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /* 恢复data线GPIO为输出，并输出0 */
    ssi_data_output_etc(0);

    *value = data;

    return OAL_SUCC;
}

/* 32bits address, 32bits value */
int32 ssi_read_value16(uint32 addr, uint16 *value, int16 last_high_addr)
{
    int32 ret;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;

    if (last_high_addr != addr_half_word_high) {
        if (ssi_write16_etc(ssi_base_etc, addr_half_word_high) < 0) {
            PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
            return BOARD_FAIL;
        }
    }

    ret = ssi_read_data16(addr_half_word_low, value);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, *value);

    return ret;
}

/*
 * 函 数 名  : ssi_read_value32
 * 功能描述  : gpio模拟SSI 读32BIT value
 *             1.配置SSI 为32BIT模式
 *             2.第一次读16BIT操作，SOC发起32BIT操作，返回低16BIT给HOST
 *             3.第二次读同一地址16BIT操作，SOC不发起总线操作，返回高16BIT给HOST
 *             4.如果跳过步骤3 读其他地址，SOC侧高16BIT 会被丢弃
 */
int32 ssi_read_value32(uint32 addr, uint32 *value, int16 last_high_addr)
{
    int32 ret;
    uint16 reg;

    ret = ssi_read_value16(addr, &reg, last_high_addr);
    if (ret) {
        PS_PRINT_ERR("read addr 0x%x low 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }
    *value = (uint32)reg;

    /* 读32位地址的高16位 */
    ret = ssi_read_value16(addr + 0x2, &reg, (addr >> 16));
    if (ret) {
        PS_PRINT_ERR("read addr 0x%x high 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }

    *value = ((reg << 16) | *value);

    return OAL_SUCC;
}

int32 ssi_read_value32_test(uint32 addr)
{
    int32 ret;
    uint32 value = 0xffffffff;

    ret = ssi_read_value32(addr, &value, (((addr >> 16) & 0xffff) + 1));
    if (ret) {
        PS_PRINT_ERR("ssi_read_value32 ret=%d\n", ret);
        return 0xffffffff;
    }

    return value;
}

int32 ssi_write_value32(uint32 addr, uint32 value)
{
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;
    uint16 addr_half_word_low_incr;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;
    addr_half_word_low_incr = ((addr + 2) & 0xffff) >> 1;

    /* 往基地址写地址的高16位 */
    if (ssi_write_data_etc(ssi_base_etc, addr_half_word_high) < 0) {
        PS_PRINT_ERR("ssi write high addr: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* 低地址写实际要写入的value */
    if (ssi_write_data_etc(addr_half_word_low, value & 0xffff) < 0) {
        PS_PRINT_ERR("ssi write low value: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    if (ssi_write_data_etc(addr_half_word_low_incr, (value >> 16) & 0xffff) < 0) {
        PS_PRINT_ERR("ssi write high value: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

/* 16bits/32bits switch mode */
int32 ssi_switch_ahb_mode(oal_int32 is_32bit_mode)
{
    return ssi_write16_etc(0x8001, !!is_32bit_mode);
}

int32 ssi_clear_ahb_highaddr(void)
{
    return ssi_write16_etc(ssi_base_etc, 0x0);
    ;
}

int32 do_ssi_file_test(ssi_file_st *file_st, ssi_trans_test_st *pst_ssi_test)
{
    uint32 ul_addr;
    OS_KERNEL_FILE_STRU *fp = NULL;
    mm_segment_t fs = {0};
    uint16 data_buf = 0;
    int32 rdlen = 0;
    int32 l_ret = BOARD_FAIL;
    const uint32 ul_count_everytime = 2; /* 表示每次循环读的字节数 */

    if ((pst_ssi_test == NULL) || (file_st == NULL)) {
        return BOARD_FAIL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open(file_st->file_name, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        set_fs(fs);
        fp = NULL;
        PS_PRINT_ERR("filp_open %s fail!!\n", file_st->file_name);
        return -EFAIL;
    }
    ul_addr = file_st->write_addr;
    PS_PRINT_INFO("begin file:%s", file_st->file_name);
    while (1) {
        data_buf = 0;
        rdlen = oal_file_read_ext(fp, fp->f_pos, (uint8 *)&data_buf, ul_count_everytime);

        if (rdlen > 0) {
            fp->f_pos += rdlen;
        } else if (rdlen == 0) {
            PS_PRINT_INFO("file read over:%s!!\n", file_st->file_name);
            break;
        } else {
            PS_PRINT_ERR("file read ERROR:%d!!\n", rdlen);
            goto test_fail;
        }
        l_ret = ssi_write32_etc(ul_addr, data_buf);
        if (l_ret != BOARD_SUCC) {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            goto test_fail;
        }
        pst_ssi_test->trans_len += ul_count_everytime;
        ul_addr += ul_count_everytime;
    }
    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;
    PS_PRINT_INFO("%s send finish\n", file_st->file_name);
    return BOARD_SUCC;
test_fail:
    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;
    return BOARD_FAIL;
}
typedef struct ht_test_s {
    int32 add;
    int32 data;
} ht_test_t;

ht_test_t ht_cnt[] = {
    { 0x50000314, 0x0D00 },
    { 0x50002724, 0x0022 },
    { 0x50002720, 0x0033 },
};
int32 test_hd_ssi_write(void)
{
    int32 i;
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }

    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    for (i = 0; i < sizeof(ht_cnt) / sizeof(ht_test_t); i++) {
        if (ssi_write32_etc(ht_cnt[i].add, ht_cnt[i].data) != 0) {
            PS_PRINT_ERR("error: ssi write fail s_addr:0x%x s_data:0x%x\n", ht_cnt[i].add, ht_cnt[i].data);
        } else {
        }
    }

    /* reset clk */
    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    if (ssi_free_gpio_etc() != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    PS_PRINT_ERR("ALL reg finish---------------------");
    return 0;
err_exit:
    PS_PRINT_ERR("test reg fail---------------------");
    ssi_free_gpio_etc();
    return BOARD_FAIL;
}
int32 ssi_single_write(int32 addr, int16 data)
{
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }

    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    /* set wcpu wait */
    if (ssi_write32_etc(addr, data) != BOARD_SUCC) {
        goto err_exit;
    }
    /* reset clk */
    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    if (ssi_free_gpio_etc() != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    return 0;
err_exit:
    ssi_free_gpio_etc();
    return BOARD_FAIL;
}
int32 ssi_single_read(int32 addr)
{
    int32 ret;
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    ret = ssi_read32_etc(addr);
    /* reset clk */
    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    if (ssi_free_gpio_etc() != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    return ret;
err_exit:
    ssi_free_gpio_etc();
    return BOARD_FAIL;
}
int32 ssi_file_test(ssi_trans_test_st *pst_ssi_test)
{
    int32 i = 0;
    if (pst_ssi_test == NULL) {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = 0;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hi1103_chip_power_on();
    hi1103_bfgx_enable();
    if (hi1103_wifi_enable()) {
        PS_PRINT_ERR("hi1103_wifi_enable failed!\n");
        return BOARD_FAIL;
    }
#endif

    // waring: fpga version should set 300801c0 1 to let host control ssi
    /* first set ssi clk ctl */
    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        return BOARD_FAIL;
    }
    // env init
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /* set bootloader deadbeaf */
    if (ssi_write32_etc(0x8010010c, 0xbeaf) != BOARD_SUCC) {
        PS_PRINT_ERR("set flag:beaf fail\n");
        return BOARD_FAIL;
    }
    if (ssi_write32_etc(0x8010010e, 0xdead) != BOARD_SUCC) {
        PS_PRINT_ERR("set flag:dead fail\n");
        return BOARD_FAIL;
    }
#else
    /* set wcpu wait */
    if (ssi_write32_etc(HI1103_GLB_CTL_WCPU_WAIT_CTL_REG, 0x1) != BOARD_SUCC) {
        PS_PRINT_ERR("set wcpu wait fail\n");
        return BOARD_FAIL;
    }

    /* reset wcpu */
    if (ssi_write32_etc(HI1103_W_CTL_WTOPCRG_SOFT_RESET_REG, 0xfe5e) != BOARD_SUCC) {
        // 脉冲复位
    }
    /* boot flag */
    if (ssi_write32_etc(HI1103_GLB_CTL_PWR_ON_LABLE_REG, 0xbeaf) != BOARD_SUCC) {
        PS_PRINT_ERR("set boot flag fail\n");
        return BOARD_FAIL;
    }
    /* dereset bcpu */
    if (ssi_write32_etc(HI1103_GLB_CTL_SOFT_RST_BCPU_REG, 1) != BOARD_SUCC) {
        PS_PRINT_ERR("dereset bcpu\n");
        return BOARD_FAIL;
    }
#endif
    /* file download */
    for (i = 0; i < sizeof(aSsiFile) / sizeof(ssi_file_st); i++) {
        if (do_ssi_file_test(&aSsiFile[i], pst_ssi_test) != BOARD_SUCC) {
            PS_PRINT_ERR("%s write %d error\n", aSsiFile[i].file_name, aSsiFile[i].write_addr);
            return BOARD_FAIL;
        }
    }
    /* let cpu go */
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /* reset bcpu */
    if (ssi_write32_etc(HI1103_GLB_CTL_SOFT_RST_BCPU_REG, 0) != BOARD_SUCC) {
        PS_PRINT_ERR("reset bcpu set 0 fail\n");
        return BOARD_FAIL;
    }
    if (ssi_write32_etc(HI1103_GLB_CTL_SOFT_RST_BCPU_REG, 1) != BOARD_SUCC) {
        PS_PRINT_ERR("reset bcpu set 1 fail\n");
        return BOARD_FAIL;
    }
#else
    /* clear b wait */
    if (ssi_write32_etc(HI1103_GLB_CTL_BCPU_WAIT_CTL_REG, 0x0) != BOARD_SUCC) {
        PS_PRINT_ERR("clear b wait\n");
        return BOARD_FAIL;
    }
    /* clear w wait */
    if (ssi_write32_etc(HI1103_GLB_CTL_WCPU_WAIT_CTL_REG, 0x0) != BOARD_SUCC) {
        PS_PRINT_ERR("clear w wait\n");
        return BOARD_FAIL;
    }
#endif
    /* reset clk */
    if (ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        return BOARD_FAIL;
    }
    return BOARD_SUCC;
}
int32 do_ssi_mem_test(ssi_trans_test_st *pst_ssi_test)
{
    uint32 i = 0;
    uint32 ul_write_base = 0x0;
    uint32 ul_addr;
    int32 l_ret = BOARD_FAIL;
    if (pst_ssi_test == NULL) {
        return BOARD_FAIL;
    }

    for (i = 0; i < pst_ssi_test->trans_len; i++) {
        ul_addr = ul_write_base + 2 * i;  // 按2字节读写
        l_ret = ssi_write32_etc(ul_addr, SSI_WRITE_DATA);
        if (l_ret != BOARD_SUCC) {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            return l_ret;
        }
        l_ret = ssi_read32_etc(ul_addr);
        if (l_ret != SSI_WRITE_DATA) {
            PS_PRINT_ERR("read write 0x%x error, expect:0x5a5a,actual:0x%x\n", ul_addr, l_ret);
            return l_ret;
        }
    }
    return BOARD_SUCC;
}
int32 ssi_download_test(ssi_trans_test_st *pst_ssi_test)
{
    int32 l_ret = BOARD_FAIL;
    const uint32 ul_test_trans_len = 1024;

    struct timeval stime, etime;

    if (pst_ssi_test == NULL) {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = ul_test_trans_len;
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        goto fail_process;
    }

    do_gettimeofday(&stime);
    switch (pst_ssi_test->test_type) {
        case SSI_MEM_TEST:
            l_ret = do_ssi_mem_test(pst_ssi_test);
            break;
        case SSI_FILE_TEST:
            l_ret = ssi_file_test(pst_ssi_test);
            break;
        default:
            PS_PRINT_ERR("error type=%d\n", pst_ssi_test->test_type);
            break;
    }
    do_gettimeofday(&etime);
    ssi_free_gpio_etc();
    if (l_ret != BOARD_SUCC) {
        goto fail_process;
    }
    pst_ssi_test->used_time = (etime.tv_sec - stime.tv_sec) * 1000 + (etime.tv_usec - stime.tv_usec) / 1000;
    pst_ssi_test->send_status = 0;
    return BOARD_SUCC;
fail_process:
    pst_ssi_test->used_time = 0;
    pst_ssi_test->send_status = -1;
    return BOARD_FAIL;
}

ssi_reg_info hi1103_glb_ctrl_full = { 0x50000000, 0x1000, SSI_RW_WORD_MOD };
ssi_reg_info hi1103_glb_ctrl_extend1 = { 0x50001400, 0x10,   SSI_RW_WORD_MOD };
ssi_reg_info hi1103_glb_ctrl_extend2 = { 0x50001540, 0xc,    SSI_RW_WORD_MOD };
ssi_reg_info hi1103_glb_ctrl_extend3 = { 0x50001600, 0x4,    SSI_RW_WORD_MOD };
ssi_reg_info hi1103_pmu_cmu_ctrl_full = { 0x50002000, 0xb00,  SSI_RW_WORD_MOD };
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_full = { 0x50003000, 0xa20,  SSI_RW_WORD_MOD };
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_tail = { 0x50003a80, 0xc,    SSI_RW_WORD_MOD };
ssi_reg_info hi1103_coex_ctl_part1 = { 0x5000a000, 0x354,  SSI_RW_WORD_MOD }; /* coex ctl part1 */
ssi_reg_info hi1103_coex_ctl_part2 = { 0x5000a500, 0x8,    SSI_RW_WORD_MOD };   /* coex ctl part2 */
ssi_reg_info hi1103_w_ctrl_full = { 0x40000000, 0x408,  SSI_RW_WORD_MOD };
ssi_reg_info hi1103_w_key_mem = { 0x2001e620, 0x80,   SSI_RW_DWORD_MOD };
ssi_reg_info hi1103_b_ctrl_full = { 0x48000000, 0x40c,  SSI_RW_WORD_MOD };
ssi_reg_info hi1103_pcie_ctrl_full = { 0x40007000, 0x4c8,  SSI_RW_DWORD_MOD };
ssi_reg_info hi1103_pcie_dbi_full = { 0x40102000, 0x900,  SSI_RW_DWORD_MOD };         /* 没建链之前不能读 */
ssi_reg_info hi1103_pcie_pilot_iatu_full = { 0x40104000, 0x2000, SSI_RW_DWORD_MOD }; /* 8KB */
ssi_reg_info hi1103_pcie_pilot_dma_full = { 0x40106000, 0x1000, SSI_RW_DWORD_MOD };  /* 4KB */
ssi_reg_info hi1103_pcie_dma_ctrl_full = { 0x40008000, 0x34,   SSI_RW_DWORD_MOD };
ssi_reg_info hi1103_pcie_sdio_ctrl_full = { 0x40101000, 0x180,  SSI_RW_DWORD_MOD };

ssi_reg_info hi1103_tcxo_detect_reg1 = { 0x50000040, 0x4,  SSI_RW_WORD_MOD };
ssi_reg_info hi1103_tcxo_detect_reg2 = { 0x500000c0, 0x14, SSI_RW_WORD_MOD };
ssi_reg_info hi1103_tcxo_detect_reg3 = { 0x50000700, 0xc,  SSI_RW_WORD_MOD };

ssi_reg_info *hi1103_aon_reg_full[] = {
    &hi1103_glb_ctrl_full,
    &hi1103_glb_ctrl_extend1,
    &hi1103_glb_ctrl_extend2,
    &hi1103_glb_ctrl_extend3,
    &hi1103_pmu_cmu_ctrl_full,
    &hi1103_pmu2_cmu_ir_ctrl_full,
    &hi1103_pmu2_cmu_ir_ctrl_tail
};

ssi_reg_info *hi1103_coex_ctl_full[] = {
    &hi1103_coex_ctl_part1,
    &hi1103_coex_ctl_part2,
};

ssi_reg_info *hi1103_tcxo_detect_regs[] = {
    &hi1103_tcxo_detect_reg1,
    &hi1103_tcxo_detect_reg2,
    &hi1103_tcxo_detect_reg3
};

// 0x5000_0000~0x500000FC
ssi_reg_info hi1103_glb_ctrl_cut1 = { 0x50000000, 0xfc, SSI_RW_WORD_MOD };
// 0x5000_0200~0x5000_020C
ssi_reg_info hi1103_glb_ctrl_cut2 = { 0x50000200, 0xc, SSI_RW_WORD_MOD };
// 0x5000_0400~0x5000_043C
ssi_reg_info hi1103_glb_ctrl_cut3 = { 0x50000400, 0x3c, SSI_RW_WORD_MOD };
// 0x5000_0500~0x5000_051C
ssi_reg_info hi1103_glb_ctrl_cut4 = { 0x50000500, 0x1c, SSI_RW_WORD_MOD };
// 0x5000_0700~0x5000_070C
ssi_reg_info hi1103_glb_ctrl_cut5 = { 0x50000700, 0xc, SSI_RW_WORD_MOD };
// 0x5000_0E00~0x5000_0E0C
ssi_reg_info hi1103_glb_ctrl_cut6 = { 0x50000E00, 0xc, SSI_RW_WORD_MOD };
// 0x5000_1400~0x5000_140C
ssi_reg_info hi1103_glb_ctrl_cut7 = { 0x50001400, 0x10, SSI_RW_WORD_MOD };
// 0x5000_1540~0x5000_1548
ssi_reg_info hi1103_glb_ctrl_cut8 = { 0x50001540, 0xc, SSI_RW_WORD_MOD };
// 0x5000_1600~0x5000_1604
ssi_reg_info hi1103_glb_ctrl_cut9 = { 0x50001600, 0x4, SSI_RW_WORD_MOD };

// PMU_CMU_CTRL
// 0x50002080~0x500021AC
ssi_reg_info hi1103_pmu_cmu_ctrl_cut1 = { 0x50002080, 0x12c, SSI_RW_WORD_MOD };
// 0x50002200~0x5000220C
ssi_reg_info hi1103_pmu_cmu_ctrl_cut2 = { 0x50002200, 0xc, SSI_RW_WORD_MOD };
// 0x50002380~0x5000239C
ssi_reg_info hi1103_pmu_cmu_ctrl_cut3 = { 0x50002380, 0x1c, SSI_RW_WORD_MOD };
// 0x50002800~0x5000283C
ssi_reg_info hi1103_pmu_cmu_ctrl_cut4 = { 0x50002800, 0x3c, SSI_RW_WORD_MOD };

// PMU2_CMU_IR_TS_EF_CTL
// 0x50003040~0x5000307C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut1 = { 0x50003040, 0x3c, SSI_RW_WORD_MOD };
// 0x5000311C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut2 = { 0x5000311C, 0x4, SSI_RW_WORD_MOD };
// 0x5000313C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut3 = { 0x5000313C, 0x4, SSI_RW_WORD_MOD };
// 0x5000315C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut4 = { 0x5000315C, 0x4, SSI_RW_WORD_MOD };
// 0x5000317C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut5 = { 0x5000317C, 0x4, SSI_RW_WORD_MOD };
// 0x5000319C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut6 = { 0x5000319C, 0x4, SSI_RW_WORD_MOD };
// 0x50003220~0x5000339C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut7 = { 0x50003220, 0x17c, SSI_RW_WORD_MOD };
// 0x50003420~0x5000343C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut8 = { 0x50003420, 0x1c, SSI_RW_WORD_MOD };
// 0x50003780~0x500037FC
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut9 = { 0x50003780, 0x7c, SSI_RW_WORD_MOD };
// 0x50003800~0x500038BF
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut10 = { 0x50003800, 0xc0, SSI_RW_WORD_MOD };
// 0x50003A80~0x50003A8C
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_cut11 = { 0x50003A80, 0xc, SSI_RW_WORD_MOD };

ssi_reg_info *hi1103_aon_reg_cut[] = {
    &hi1103_glb_ctrl_cut1,
    &hi1103_glb_ctrl_cut2,
    &hi1103_glb_ctrl_cut3,
    &hi1103_glb_ctrl_cut4,
    &hi1103_glb_ctrl_cut5,
    &hi1103_glb_ctrl_cut6,
    &hi1103_glb_ctrl_cut7,
    &hi1103_glb_ctrl_cut8,
    &hi1103_glb_ctrl_cut9,
    &hi1103_pmu_cmu_ctrl_cut1,
    &hi1103_pmu_cmu_ctrl_cut2,
    &hi1103_pmu_cmu_ctrl_cut3,
    &hi1103_pmu_cmu_ctrl_cut4,
    &hi1103_pmu2_cmu_ir_ctrl_cut1,
    &hi1103_pmu2_cmu_ir_ctrl_cut2,
    &hi1103_pmu2_cmu_ir_ctrl_cut3,
    &hi1103_pmu2_cmu_ir_ctrl_cut4,
    &hi1103_pmu2_cmu_ir_ctrl_cut5,
    &hi1103_pmu2_cmu_ir_ctrl_cut6,
    &hi1103_pmu2_cmu_ir_ctrl_cut7,
    &hi1103_pmu2_cmu_ir_ctrl_cut8,
    &hi1103_pmu2_cmu_ir_ctrl_cut9,
    &hi1103_pmu2_cmu_ir_ctrl_cut10,
    &hi1103_pmu2_cmu_ir_ctrl_cut11
};

ssi_reg_info hi1103_pcie_ctrl_cut1 = { 0x40007224, 0x4,  SSI_RW_DWORD_MOD };
ssi_reg_info hi1103_pcie_ctrl_cut2 = { 0x400072d0, 0x4,  SSI_RW_DWORD_MOD };
ssi_reg_info hi1103_pcie_ctrl_cut3 = { 0x40007430, 0x9c, SSI_RW_DWORD_MOD };

ssi_reg_info *hi1103_pcie_cfg_reg_cut[] = {
    &hi1103_pcie_ctrl_cut1,
    &hi1103_pcie_ctrl_cut2,
    &hi1103_pcie_ctrl_cut3
};

ssi_reg_info *hi1103_pcie_cfg_reg_full[] = {
    &hi1103_pcie_ctrl_full,
    &hi1103_pcie_dma_ctrl_full
};

ssi_reg_info *hi1103_pcie_dbi_mpw2_reg_full[] = {
    &hi1103_pcie_dbi_full,
};

ssi_reg_info *hi1103_pcie_dbi_pilot_reg_full[] = {
    &hi1103_pcie_dbi_full,
    &hi1103_pcie_pilot_dma_full,
};

int ssi_check_device_isalive(void)
{
    int i;
    uint32 reg;
    const uint32 check_times = 2;

    for (i = 0; i < check_times; i++) {
        reg = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BASE);
        if (reg == 0x101) {
            PS_PRINT_INFO("reg is 0x%x\n", reg);
            break;
        }
    }

    if (i == check_times) {
        PS_PRINT_INFO("ssi is fail, gpio-ssi did't support, reg=0x%x\n", reg);
        return -1;
    }
    return 0;
}

int ssi_check_is_pilot(void)
{
    int32 ret;
    uint16 value = 0;

    /* pilot pmuctrl 0x598 is reserved */
    if (ssi_is_pilot == -1) {
        ret = ssi_read_value16(0x50002598, &value, 0x0);
        if (ret) {
            PS_PRINT_ERR("read 0x50002598 failed\n");
            return ret;
        } else {
            PS_PRINT_INFO("value=0x%x [%s]\n", value, value ? "mpw2" : "pilot");
            if (value) {
                ssi_is_pilot = 0;
                return 0;
            } else {
                ssi_is_pilot = 1;
                return 1;
            }
        }
    } else {
        PS_PRINT_INFO("%s\n", ssi_is_pilot ? "pilot" : "mpw2");
        return ssi_is_pilot;
    }
}

int ssi_read_wpcu_pc_lr_sp(int trace_en)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /* read pc twice check whether wcpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32_etc(HI1103_GLB_CTL_WCPU_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_PC_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_LR_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_SP_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_WCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read wcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if (!pc && !lr && !sp) {
            PS_PRINT_INFO("wcpu pc lr sp all zero\n");
            if (trace_en) {
                if (ssi_check_is_pilot() == 1) {
                    if (ssi_check_wcpu_is_working()) {
                        PS_PRINT_INFO("wcpu try to enable trace en\n");
                        ssi_write32_etc(HI1103_WCPU_PATCH_WCPU_CFG_TRACE_EN_REG, 0x1);
                        oal_mdelay(1);
                    }
                    trace_en = 0;
                    i = -1;
                }
            }
        } else {
            if (st_ssi_cpu_infos.wcpu_info.reg_flag[i] == 0) {
                st_ssi_cpu_infos.wcpu_info.reg_flag[i] = 1;
                st_ssi_cpu_infos.wcpu_info.pc[i] = pc;
                st_ssi_cpu_infos.wcpu_info.lr[i] = lr;
                st_ssi_cpu_infos.wcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10);
    }

    return 0;
}

int ssi_read_bpcu_pc_lr_sp(int trace_en)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /* read pc twice check whether wcpu is runing */
    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        ssi_write32_etc(HI1103_GLB_CTL_BCPU_LOAD_REG, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_PC_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_PC_H_REG);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_LR_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_LR_H_REG);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_SP_L_REG);
        reg_high = (uint32)ssi_read32_etc(HI1103_GLB_CTL_BCPU_SP_H_REG);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read bcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if (!pc && !lr && !sp) {
            PS_PRINT_INFO("bcpu pc lr sp all zero\n");
            if (trace_en) {
                if (ssi_check_is_pilot() == 1) {
                    if (ssi_check_bcpu_is_working()) {
                        PS_PRINT_INFO("bcpu try to enable trace en\n");
                        ssi_write32_etc(HI1103_BCPU_PATCH_BCPU_CFG_TRACE_EN_REG, 0x1);
                        oal_mdelay(1);
                    }
                    trace_en = 0;
                    i = -1;
                }
            }
        } else {
            if (st_ssi_cpu_infos.bcpu_info.reg_flag[i] == 0) {
                st_ssi_cpu_infos.bcpu_info.reg_flag[i] = 1;
                st_ssi_cpu_infos.bcpu_info.pc[i] = pc;
                st_ssi_cpu_infos.bcpu_info.lr[i] = lr;
                st_ssi_cpu_infos.bcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10);
    }

    return 0;
}

void ssi_check_buck_scp_ocp_status(void)
{
    uint32 reg;

    /* buck ocp/acp */
    reg = (uint32)ssi_read32_etc(HI1103_PMU_CMU_CTL_PMU_PROTECT_STATUS_REG);
    if ((reg & (0xFFFFFFFC)) != 0) {
        /* bit 0,1 */
        PS_PRINT_INFO("buck protect status:0x%x invalid", reg);
        return;
    }

    PS_PRINT_INFO("buck protect status:0x%x %s %s \n", reg,
                  (reg & 0x1) ? "buck_scp_off" : "", (reg & 0x2) ? "buck_ocp_off" : "");
#ifdef CONFIG_HUAWEI_DSM
    if (reg & 0x3) {
        hw_1103_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_BUCK_PROTECTED, "%s: buck protect status:0x%x %s %s \n",
                                  __FUNCTION__, reg, (reg & 0x1) ? "buck_scp_off" : "",
                                  (reg & 0x2) ? "buck_ocp_off" : "");
        halt_det_cnt++;
    }
#endif
}

int ssi_check_wcpu_is_working(void)
{
    uint32 reg, mask;
    int32 ret = ssi_check_is_pilot();
    if (ret < 0) {
        return ret;
    }

    if (ret) {
        /* pilot */
        reg = (uint32)ssi_read32_etc(HI1103_PMU_CMU_CTL_SYS_STATUS_0_REG);
        mask = reg & 0x7;
        PS_PRINT_INFO("cpu state=0x%8x, wcpu is %s\n", reg, ssi_hi1103_pilot_cpu_st_str[mask]);
        st_ssi_cpu_infos.wcpu_info.cpu_state = mask;
        if (mask == 0x5) {
            ssi_check_buck_scp_ocp_status();
        }
        return (mask == 0x3);
    } else {
        /* mpw2 */
        reg = (uint32)ssi_read32_etc(0x50002240);
        mask = reg & 0x3;
        PS_PRINT_INFO("cpu state=0x%8x, wcpu is %s\n", reg, ssi_hi1103_mpw2_cpu_st_str[mask]);
        st_ssi_cpu_infos.wcpu_info.cpu_state = mask;
        return (mask == 0x3);
    }
}

int ssi_check_bcpu_is_working(void)
{
    uint32 reg, mask;
    int32 ret = ssi_check_is_pilot();
    if (ret < 0) {
        return ret;
    }

    if (ret) {
        /* pilot */
        reg = (uint32)ssi_read32_etc(HI1103_PMU_CMU_CTL_SYS_STATUS_0_REG);
        mask = (reg >> 3) & 0x7;
        PS_PRINT_INFO("cpu state=0x%8x, bcpu is %s\n", reg, ssi_hi1103_pilot_cpu_st_str[mask]);
        st_ssi_cpu_infos.bcpu_info.cpu_state = mask;
        if (mask == 0x5) {
            ssi_check_buck_scp_ocp_status();
        }
        return (mask == 0x3);
    } else {
        /* mpw2 */
        reg = (uint32)ssi_read32_etc(0x50002240);
        mask = (reg >> 2) & 0x3;
        PS_PRINT_INFO("cpu state=0x%8x, bcpu is %s\n", reg, ssi_hi1103_mpw2_cpu_st_str[mask]);
        st_ssi_cpu_infos.bcpu_info.cpu_state = mask;
        return (mask == 0x3);
    }
}

#define TCXO_32K_DET_VALUE 10
/* [+-x%] */
#define TCXO_LIMIT_THRESHOLD 5

#define TCXO_GATING_CLK 76800000 /* 默认时钟 */
#define TCXO_NOMAL_CKL  38400000
int ssi_detect_tcxo_is_normal(void)
{
    /*
     * tcxo detect 依赖tcxo时钟，
     * 如果在启动后tcxo 异常那么tcxo_32k_det_result 为旧值
     * 如果在启动后32k异常 那么sytem_tick为旧值
     */
    int ret;
    char *tcxo_str = "";
    int tcxo_is_abnormal = 0;
    uint32 reg;
    uint32 tcxo_enable;
    uint32 tcxo_det_value_src, tcxo_det_value_target;
    oal_uint32 clock_32k = 0;
    uint32 sys_tick_old, sys_tick_new, pmu2_cmu_abb_sts_3, pmu2_cmu_abb_sts_2;
    uint32 tcxo_det_res_old, tcxo_det_res_new, cmu_reserve1;
    oal_uint64 clock_tcxo = 0;
    oal_uint64 div_clock = 0;
    oal_uint64 tcxo_limit_low, tcxo_limit_high, tcxo_tmp;
    oal_uint64 base_tcxo_clock = TCXO_GATING_CLK;

    declare_time_cost_stru(cost);

    pmu2_cmu_abb_sts_3 = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_3_REG);
    pmu2_cmu_abb_sts_2 = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_PMU2_CMU_ABB_STS_2_REG);
    cmu_reserve1 = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_CMU_RESERVE1_REG);
    tcxo_det_value_src = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_32K_DET_CNT_REG);

    if (cmu_reserve1 & (1 << 7)) { /* 0x50003338的bit7表示时钟频率选择 */
        base_tcxo_clock = TCXO_GATING_CLK;
    } else {
        base_tcxo_clock = TCXO_NOMAL_CKL;
    }

    if ((!(pmu2_cmu_abb_sts_3 & (1 << 7)))
        && (pmu2_cmu_abb_sts_2 & (1 << 14))) {
        /* tcxo enable */
        tcxo_enable = 1;
    } else {
        /* system maybe sleep, tcxo disable */
        tcxo_enable = 0;
        PS_PRINT_ERR("tcxo gating normal\n");
    }

    tcxo_det_value_target = TCXO_32K_DET_VALUE;
    if (tcxo_det_value_src == tcxo_det_value_target) {
        /* 刚做过detect,改变det_value，观测值是否改变 */
        tcxo_det_value_target = TCXO_32K_DET_VALUE + 2;
    }

    /* 为了计算误差范围 */
    tcxo_tmp = div_u64(base_tcxo_clock, 100);
    tcxo_limit_low = (tcxo_tmp * (100 - TCXO_LIMIT_THRESHOLD));
    tcxo_limit_high = (tcxo_tmp * (100 + TCXO_LIMIT_THRESHOLD));

    sys_tick_old = (uint32)ssi_read32_etc(HI1103_GLB_CTL_SYS_TICK_VALUE_W_0_REG);
    tcxo_det_res_old = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_32K_DET_RESULT_REG);

    ssi_write32_etc(HI1103_GLB_CTL_SYS_TICK_CFG_W_REG, 0x2); /* 清零w systick */
    oal_get_time_cost_start(cost);

    if (tcxo_enable) {
        ssi_write32_etc(HI1103_GLB_CTL_TCXO_32K_DET_CNT_REG, tcxo_det_value_target); /* 设置计数周期 */
        ssi_write32_etc(HI1103_GLB_CTL_TCXO_DET_CTL_REG, 0x0);                       /* tcxo_det_en disable */

        /* to tcxo */
        ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO);

        oal_udelay(150);

        /* to ssi */
        ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI);
        ssi_write32_etc(HI1103_GLB_CTL_TCXO_DET_CTL_REG, 0x1); /* tcxo_det_en enable */
        /* to tcxo */
        ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO);
        oal_udelay(31 * tcxo_det_value_target * 2); /* wait detect done,根据设置的计数周期数等待 */

        /* to ssi */
        ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI);
    } else {
        oal_udelay(300);
    }

    ret = ssi_read_reg_info_arry(hi1103_tcxo_detect_regs, sizeof(hi1103_tcxo_detect_regs) / sizeof(ssi_reg_info *),
                                 ssi_is_logfile);
    if (ret) {
        return ret;
    }

    oal_udelay(1000); /* wait 32k count more */

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);

    sys_tick_new = (uint32)ssi_read32_etc(HI1103_GLB_CTL_SYS_TICK_VALUE_W_0_REG);

    reg = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_DET_CTL_REG);
    tcxo_det_res_new = (uint32)ssi_read32_etc(HI1103_GLB_CTL_TCXO_32K_DET_RESULT_REG);

    /* 32k detect */
    if (sys_tick_new == sys_tick_old) {
        PS_PRINT_ERR("32k sys_tick don't change after detect, 32k maybe abnormal, sys_tick=0x%x\n", sys_tick_new);
    } else {
        oal_uint64 us_to_s;
        us_to_s = time_cost_var_sub(cost);
        us_to_s += 1446; /* 经验值,误差1446us */
        clock_32k = (sys_tick_new * 1000) / (oal_uint32)us_to_s;
        PS_PRINT_ERR("32k runtime:%llu us , sys_tick:%u\n", us_to_s, sys_tick_new);
        PS_PRINT_ERR("32k realclock real= %u Khz[base=32768]\n", clock_32k);
    }

    /* tcxo enabled */
    if (tcxo_enable) {
        if (tcxo_det_res_new == tcxo_det_res_old) {
            /* tcxo 软件配置为打开此时应该有时钟 */
            PS_PRINT_ERR("tcxo don't change after detect, tcxo or 32k maybe abnormal, tcxo=0x%x\n", tcxo_det_res_new);
            if (tcxo_det_res_new == 0) {
                tcxo_is_abnormal = 1;
                tcxo_str = "non-tcxo";
            } else {
                /* 这里可能是无效的探测，要结合详细日志分析，此处DSM忽略改分支，不上报 */
                tcxo_is_abnormal = 0;
                tcxo_str = "tcxo-detect-invalid";
            }
        } else {
            /*
             * tcxo_det_res_new read from 16bit width register  <= 0xffff
             * (tcxo_det_res_new * 32768) = (检测到的计数周期数 * 32k时钟)
             */
            clock_tcxo = (oal_uint64)((tcxo_det_res_new * 32768) / (tcxo_det_value_target));
            div_clock = clock_tcxo;
            div_clock = div_u64(div_clock, 1000000); /* hz to Mhz */
            if ((clock_tcxo < tcxo_limit_low) || (clock_tcxo > tcxo_limit_high)) {
                /* 时钟误差超过阈值 */
                tcxo_is_abnormal = 2;
                tcxo_str = "tcxo clock-abnormal";
            } else {
                tcxo_is_abnormal = 0;
                tcxo_str = "tcxo normal";
            }
            PS_PRINT_ERR("%s real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n",
                         tcxo_str, clock_tcxo, div_clock, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high);
        }

        /* tcxo detect abnormal, dmd report */
        if (hi11xx_kernel_crash == 0) {
            /* kernel is normal */
            if (tcxo_is_abnormal) {
#ifdef CONFIG_HUAWEI_DSM
                hw_1103_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_TCXO_ERROR,
                    "%s: tcxo=%llu[%llu][limit:%llu~%llu] 32k_clock=%lu,det_tick=0x%x value=0x%x\n",
                    tcxo_str, clock_tcxo, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high, clock_32k,
                    tcxo_det_value_target, tcxo_det_res_new);
                halt_det_cnt++;
#endif
            }
        }
    }

    return ret;
}

int ssi_read_device_arm_register(int trace_en)
{
    int32 ret;
    int32 is_pilot;

    uint32 reg = (uint32)ssi_read32_etc(HI1103_PMU2_CMU_IR_SYSLDO_WL_C0_ABB_RF_PWR_EN_STS_REG);

    if (reg == 0x3) {
        PS_PRINT_ERR("0x50003a88 is 0x3, wifi chip maybe enter dft mode , please check!\n");
    }

    /* read PC */
    is_pilot = ssi_check_is_pilot();
    if (is_pilot < 0) {
        return is_pilot;
    }
    ret = ssi_check_wcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        ssi_read_wpcu_pc_lr_sp(trace_en);
    }
    bfgx_print_subsys_state();
    ret = ssi_check_bcpu_is_working();
    if (ret < 0) {
        return ret;
    }
    if (ret) {
        ssi_read_bpcu_pc_lr_sp(trace_en);
    }

    return 0;
}

int32 ssi_tcxo_mux(uint32 flag)
{
    int ret;
    int32 is_pilot;

    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_ERR("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        PS_PRINT_ERR("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                     board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data, ret);
        return ret;
    }

    PS_PRINT_INFO("SSI start set\n");

    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI);

    is_pilot = ssi_check_is_pilot();
    if (is_pilot != 1) {
        PS_PRINT_INFO("not pilot chip, return\n");
        ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO);
        ssi_free_gpio_etc();
        return 0;
    }

    if (flag == 1) {
        ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x0);
        ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x60);
        ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x60);
        ssi_write32_etc(HI1103_PMU2_CMU_IR_CMU_RESERVE1_REG, 0x100);
        PS_PRINT_INFO("SSI set 0x50003338 to 0x100\n");
    } else {
        ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x0);
    }

    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO);

    PS_PRINT_INFO("SSI set OK\n");

    ssi_free_gpio_etc();

    return 0;
}

int ssi_read_reg_prep(ssi_reg_info *pst_reg_info, oal_int32 *is_logfile, oal_int32 *is_atomic,
                      char *filename, oal_uint32 len)
{
    struct timeval tv;
    struct rtc_time tm = {0};
    int ret = OAL_SUCC;

    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        *is_logfile = 0;
        *is_atomic = 1;
    }

    if (*is_logfile) {
        do_gettimeofday(&tv);
        rtc_time_to_tm(tv.tv_sec, &tm);
        ret = snprintf_s(filename, len, len - 1, "%s/gpio_ssi_%08x_%08x_%04d%02d%02d%02d%02d%02d.bin",
                         str_gpio_ssi_dump_path,
                         pst_reg_info->base_addr,
                         pst_reg_info->base_addr + pst_reg_info->len - 1,
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                         tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
        if (ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            return -OAL_EFAIL;
        }
    }
    return ret;
}

int ssi_read_reg_info(ssi_reg_info *pst_reg_info,
                      void *buf, int32 buf_len,
                      oal_int32 is_logfile)
{
    int ret;
    int step = 4;
    oal_int32 is_atomic = 0;
    uint32 reg;
    uint16 reg16;
    uint32 ssi_address;
    uint32 realloc = 0;
    mm_segment_t fs;
    uint16 last_high_addr;
    int i, j, k, seg_size, seg_nums, left_size;
    const uint32 ul_max_ssi_read_retry_times = 3;

    OS_KERNEL_FILE_STRU *fp = NULL;
    const uint32 ul_filename_len = 200;
    char filename[ul_filename_len];

    memset_s(filename, sizeof(filename), 0, sizeof(filename));

    ret = ssi_read_reg_prep(pst_reg_info, &is_logfile, &is_atomic, filename, sizeof(filename));

    if (ret < 0) {
        return -OAL_EFAIL;
    }

    ret = ssi_check_device_isalive();
    if (ret) {
        PS_PRINT_INFO("gpio-ssi maybe dead before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
        return -OAL_EFAIL;
    }

    if (buf == NULL) {
        if (is_atomic) {
            buf = kmalloc(pst_reg_info->len, GFP_ATOMIC);
        } else {
            buf = OS_VMALLOC_GFP(pst_reg_info->len);
        }

        if (buf == NULL) {
            PS_PRINT_INFO("alloc mem failed before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
            return -OAL_ENOMEM;
        }
        buf_len = pst_reg_info->len;
        realloc = 1;
    }

    PS_PRINT_INFO("dump reg info 0x%x:%u, buf len:%u \n", pst_reg_info->base_addr, pst_reg_info->len, buf_len);

    fs = get_fs();
    set_fs(KERNEL_DS);

    if (is_logfile) {
        fp = filp_open(filename, O_RDWR | O_CREAT, 0644);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_INFO("open file %s failed ret=%ld\n", filename, PTR_ERR(fp));
            is_logfile = 0;
        } else {
            PS_PRINT_INFO("open file %s succ\n", filename);
            vfs_llseek(fp, 0, SEEK_SET);
        }
    }

    last_high_addr = 0x0;
    ssi_clear_ahb_highaddr();

    if (pst_reg_info->rw_mod == SSI_RW_DWORD_MOD) {
        /* switch 32bits mode */
        ssi_switch_ahb_mode(1);
    } else {
        ssi_switch_ahb_mode(0);
    }

retry:

    seg_nums = (pst_reg_info->len - 1 / buf_len) + 1;
    left_size = pst_reg_info->len;

    for (i = 0; i < seg_nums; i++) {
        seg_size = OAL_MIN(left_size, buf_len);
        for (j = 0; j < seg_size; j += step) {
            ssi_address = pst_reg_info->base_addr + i * buf_len + j;

            for (k = 0; k < ul_max_ssi_read_retry_times; k++) {
                reg = 0x0;
                reg16 = 0x0;
                if (pst_reg_info->rw_mod == SSI_RW_DWORD_MOD) {
                    ret = ssi_read_value32(ssi_address, &reg, last_high_addr);
                } else {
                    ret = ssi_read_value16(ssi_address, &reg16, last_high_addr);
                    reg = reg16;
                }

                if (ret == 0) {
                    break;
                }
            }
            if (k == ul_max_ssi_read_retry_times) {
                PS_PRINT_ERR("ssi read address 0x%x failed, retry %d times", ssi_address, k);
                goto fail_read;
            }
            last_high_addr = (ssi_address >> 16);
            oal_writel(reg, buf + j);
        }

        left_size -= seg_size;

        if (is_logfile) {
            ret = vfs_write(fp, buf, seg_size, &fp->f_pos);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
            vfs_fsync(fp, 0);
#else
            vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
            if (ret != seg_size) {
                PS_PRINT_ERR("ssi print file failed, request %d, write %d actual\n", seg_size, ret);
                is_logfile = 0;
                filp_close(fp, NULL);
                goto retry;
            }
        } else {
#ifdef CONFIG_PRINTK
            /* print to kenrel msg */
            print_hex_dump(KERN_INFO, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
                           buf, seg_size, false); /* 内核函数固定传参 */
#endif
        }
    }

    if (is_logfile) {
        filp_close(fp, NULL);
    }
    set_fs(fs);

    if (realloc) {
        if (is_atomic) {
            kfree(buf);
        } else {
            OS_MEM_VFREE(buf);
        }
    }

    if (pst_reg_info->rw_mod == SSI_RW_DWORD_MOD) {
        /* switch 16bits mode */
        ssi_switch_ahb_mode(0);
    }

    return 0;
fail_read:
    if (ssi_address != pst_reg_info->base_addr) {
        if (ssi_address > pst_reg_info->base_addr) {
#ifdef CONFIG_PRINTK
            /* print the read buf before errors */
            print_hex_dump(KERN_INFO, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
                           buf, OAL_MIN(buf_len, ssi_address - pst_reg_info->base_addr), false); /* 内核函数固定传参 */
#endif
        }
    }

    if (is_logfile) {
        filp_close(fp, NULL);
    }
    set_fs(fs);

    if (realloc) {
        if (is_atomic) {
            kfree(buf);
        } else {
            OS_MEM_VFREE(buf);
        }
    }

    if (pst_reg_info->rw_mod == SSI_RW_DWORD_MOD) {
        /* switch 16bits mode */
        ssi_switch_ahb_mode(0);
    }
    return ret;
}

int ssi_read_reg_info_test(uint32 base_addr, uint32 len, uint32 is_logfile, uint32 rw_mode)
{
    int ret;
    ssi_reg_info reg_info;

    struct st_exception_info *pst_exception_data = NULL;
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -OAL_EBUSY;
    }
    if ((!ssi_dfr_bypass) &&
        (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
         oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker) ||
         (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE))) {
        PS_PRINT_ERR("dfr is doing ,not do ssi read\n");
        return -OAL_EBUSY;
    }

    memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info));

    reg_info.base_addr = base_addr;
    reg_info.len = len;
    reg_info.rw_mod = rw_mode;

    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_INFO("gpio ssi don't support, check dts\n");
        return -1;
    }

    /* get ssi lock */
    if (ssi_try_lock()) {
        PS_PRINT_INFO("ssi is locked, request return\n");
        return -OAL_EFAIL;
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        ssi_unlock();
        return ret;
    }

    ssi_read16_etc(GPIO_SSI_REG(SSI_SSI_CTRL));
    ssi_read16_etc(GPIO_SSI_REG(SSI_SEL_CTRL));

    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI); /* switch to ssi clk, wcpu hold */
    PS_PRINT_INFO("switch ssi clk %s",
                  (ssi_read16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL)) == SSI_AON_CLKSEL_SSI) ? "ok" : "failed");

    ret = ssi_read_device_arm_register(1);
    if (ret) {
        goto ssi_fail;
    }

    PS_PRINT_INFO("ssi is ok, glb_ctrl is ready\n");

    ret = ssi_check_device_isalive();
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_read_reg_info(&reg_info, NULL, 0, is_logfile);
    if (ret) {
        goto ssi_fail;
    }

    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO); /* switch from ssi clk, wcpu continue */

    ssi_free_gpio_etc();
    ssi_unlock();

    return 0;
ssi_fail:
    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO); /* switch from ssi clk, wcpu continue */
    ssi_free_gpio_etc();
    ssi_unlock();
    return ret;
}

int ssi_read_reg_info_arry(ssi_reg_info **pst_reg_info, oal_uint32 reg_nums, oal_int32 is_logfile)
{
    int ret;
    int i;

    if (OAL_UNLIKELY(pst_reg_info == NULL)) {
        return -OAL_EFAIL;
    }

    for (i = 0; i < reg_nums; i++) {
        ret = ssi_read_reg_info(pst_reg_info[i], NULL, 0, is_logfile);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

static oal_void ssi_force_dereset_reg(oal_void)
{
    /* 解复位AON，注意寄存器配置顺序 */
    ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x60);
    ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x60);
}

static oal_void ssi_force_reset_reg(oal_void)
{
    /* 先复位再解复位AON，注意寄存器配置顺序 */
    ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x60);
    ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x60);
}

int ssi_force_reset_aon(void)
{
    int ret;
    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_INFO("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        PS_PRINT_INFO("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                      board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data, ret);
        return ret;
    }

    ssi_force_dereset_reg();

    PS_PRINT_INFO("ssi_force_reset_aon");

    ssi_free_gpio_etc();

    return 0;
}

int ssi_set_gpio_pins(int32 clk, int32 data)
{
    board_info_etc.ssi_gpio_clk = clk;
    board_info_etc.ssi_gpio_data = data;
    PS_PRINT_INFO("set ssi gpio clk:%d , gpio data:%d\n", clk, data);
    return 0;
}
EXPORT_SYMBOL_GPL(ssi_set_gpio_pins);

#define DSM_CPU_INFO_SIZE 256
void dsm_cpu_info_dump(void)
{
    int32 i;
    int32 ret;
    int32 count = 0;
    char buf[DSM_CPU_INFO_SIZE];
    /* dsm cpu信息上报 */
    if (halt_det_cnt || (hi11xx_kernel_crash)) {
        PS_PRINT_INFO("halt_det_cnt=%u hi11xx_kernel_crash=%d dsm_cpu_info_dump return\n",
                      halt_det_cnt, hi11xx_kernel_crash);
        return;
    }

    /* 没有检测到异常，上报记录的CPU信息 */
    memset_s((void *)buf, sizeof(buf), 0, sizeof(buf));
    ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1,
                     "wcpu_state=0x%x %s, bcpu_state=0x%x %s ",
                     st_ssi_cpu_infos.wcpu_info.cpu_state, ssi_is_pilot ?
                     (ssi_hi1103_pilot_cpu_st_str[st_ssi_cpu_infos.wcpu_info.cpu_state & 0x7]) :
                     (ssi_hi1103_mpw2_cpu_st_str[st_ssi_cpu_infos.wcpu_info.cpu_state & 0x3]),
                     st_ssi_cpu_infos.bcpu_info.cpu_state, ssi_is_pilot ?
                     (ssi_hi1103_pilot_cpu_st_str[st_ssi_cpu_infos.bcpu_info.cpu_state & 0x7]) :
                     (ssi_hi1103_mpw2_cpu_st_str[st_ssi_cpu_infos.bcpu_info.cpu_state & 0x3]));
    if (ret < 0) {
        goto done;
    }
    count += ret;

    for (i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++) {
        if (st_ssi_cpu_infos.wcpu_info.reg_flag[i] == 0) {
            continue;
        }
        ret = snprintf_s(buf + count, sizeof(buf) - count, sizeof(buf) - count - 1,
                         "wcpu[%d] pc:0x%x lr:0x%x sp:0x%x ", i, st_ssi_cpu_infos.wcpu_info.pc[i],
                         st_ssi_cpu_infos.wcpu_info.lr[i], st_ssi_cpu_infos.wcpu_info.sp[i]);
        if (ret < 0) {
            goto done;
        }
        count += ret;
    }

done:
#ifdef CONFIG_HUAWEI_DSM
    hw_1103_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_HALT, "%s\n", buf);
#else
    OAL_IO_PRINT("log str format err [non-dsm]%s\n", buf);
#endif
}

int ssi_dump_device_aon_regs(unsigned long long module_set)
{
    int ret = OAL_SUCC;
    if (module_set & SSI_MODULE_MASK_AON) {
        ret = ssi_read_reg_info_arry(hi1103_aon_reg_full, sizeof(hi1103_aon_reg_full) / sizeof(ssi_reg_info *),
                                     ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    if (module_set & SSI_MODULE_MASK_AON_CUT) {
        ret = ssi_read_reg_info_arry(hi1103_aon_reg_cut, sizeof(hi1103_aon_reg_cut) / sizeof(ssi_reg_info *),
                                     ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    if (module_set & SSI_MODULE_MASK_COEX_CTL) {
        ret = ssi_read_reg_info_arry(hi1103_coex_ctl_full, sizeof(hi1103_coex_ctl_full) / sizeof(ssi_reg_info *),
                                     ssi_is_logfile);
        if (ret) {
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

static int ssi_dump_device_arm_regs(unsigned long long module_set, int trace_en)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_ARM_REG) {
        ret = ssi_read_device_arm_register(trace_en);
        if (ret) {
            goto ssi_fail;
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_tcxo_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & (SSI_MODULE_MASK_AON | SSI_MODULE_MASK_AON_CUT)) {
        ret = ssi_detect_tcxo_is_normal();
        if (ret) {
            PS_PRINT_INFO("tcxo detect failed, continue dump\n");
        }
    }
    return 0;
}

static int ssi_dump_device_wcpu_key_mem(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_WCPU_KEY_DTCM) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_w_key_mem, NULL, 0, ssi_is_logfile);
            if (ret) {
                PS_PRINT_INFO("wcpu key mem read failed, continue try aon\n");
            }
        } else {
            PS_PRINT_INFO("wctrl can't dump, wcpu down\n");
        }
    }

    return 0;
}

static int ssi_dump_device_wctrl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_WCTRL) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_w_ctrl_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("wctrl can't dump, wcpu down\n");
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_pcie_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_PCIE_CFG) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info_arry(hi1103_pcie_cfg_reg_full,
                                         sizeof(hi1103_pcie_cfg_reg_full) / sizeof(ssi_reg_info *),
                                         ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("pcie cfg can't dump, wcpu down\n");
        }
    }

    if (module_set & SSI_MODULE_MASK_PCIE_CUT) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info_arry(hi1103_pcie_cfg_reg_cut,
                                         sizeof(hi1103_pcie_cfg_reg_cut) / sizeof(ssi_reg_info *),
                                         ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("pcie cfg cut can't dump, wcpu down\n");
        }
    }

    if (module_set & SSI_MODULE_MASK_PCIE_DBI) {
        if (ssi_check_wcpu_is_working()) {
            oal_uint32 reg_nums;
            ssi_reg_info **pst_pcie_dbi_reg;
            if (ssi_check_is_pilot() == 1) {
                reg_nums = sizeof(hi1103_pcie_dbi_pilot_reg_full) / sizeof(ssi_reg_info *);
                pst_pcie_dbi_reg = hi1103_pcie_dbi_pilot_reg_full;
            } else {
                reg_nums = sizeof(hi1103_pcie_dbi_mpw2_reg_full) / sizeof(ssi_reg_info *);
                pst_pcie_dbi_reg = hi1103_pcie_dbi_mpw2_reg_full;
            }

            ret = ssi_read_reg_info_arry(pst_pcie_dbi_reg, reg_nums, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("pcie dbi can't dump, wcpu down\n");
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_sdio_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_SDIO) {
        if (ssi_check_wcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_pcie_sdio_ctrl_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        } else {
            PS_PRINT_INFO("sdio can't dump, wcpu down\n");
        }
    }

    return 0;
ssi_fail:
    return ret;
}

static int ssi_dump_device_bctl_regs(unsigned long long module_set)
{
    int ret;

    if (module_set & SSI_MODULE_MASK_BCTRL) {
        if (ssi_check_bcpu_is_working()) {
            ret = ssi_read_reg_info(&hi1103_b_ctrl_full, NULL, 0, ssi_is_logfile);
            if (ret) {
                goto ssi_fail;
            }
        }
    }

    return 0;
ssi_fail:
    return ret;
}


OAL_STATIC int ssi_dump_device_regs_check_condition(unsigned long long module_set)
{
    struct st_exception_info *pst_exception_data = NULL;

    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_ERR("gpio ssi don't support, check dts\n");
        return OAL_FALSE;
    }

    /* 系统crash后强行dump,系统正常时user版本受控 */
    if ((hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_USER) && (hi11xx_kernel_crash == 0)) {
        /* user build, limit the ssi dump */
        if (!oal_print_rate_limit(30 * PRINT_RATE_SECOND)) { /* 30s打印一次 */
            /* print limit */
            module_set = 0;
            PS_PRINT_ERR("ssi dump print limit\n");
        }
    }

    if (module_set == 0) {
        PS_PRINT_ERR("ssi dump regs bypass\n");
        return OAL_FALSE;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return OAL_FALSE;
    }
    if ((!ssi_dfr_bypass) &&
        (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
         oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker) ||
         (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE))) {
        PS_PRINT_ERR("dfr is doing ,not do ssi read\n");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/* Try to dump all reg,
  ssi used to debug, we should */
int ssi_dump_device_regs(unsigned long long module_set)
{
    int ret;

    if (ssi_dump_device_regs_check_condition(module_set) != OAL_TRUE) {
        return -OAL_EBUSY;
    }

    /* get ssi lock */
    if (ssi_try_lock()) {
        PS_PRINT_INFO("ssi is locked, request return\n");
        return -OAL_EBUSY;
    }

    if (gpio_get_value(board_info_etc.power_on_enable) == 0) {
        PS_PRINT_INFO("110x power off,ssi return,power_on=%d\n", board_info_etc.power_on_enable);
        ssi_unlock();
        return -OAL_ENODEV;
    }

    DECLARE_DFT_TRACE_KEY_INFO("ssi_dump_device_regs", OAL_DFT_TRACE_FAIL);

    if (module_set & SSI_MODULE_MASK_AON_CUT) {
        module_set &= ~SSI_MODULE_MASK_AON;
    }

    if (module_set & SSI_MODULE_MASK_PCIE_CUT) {
        module_set &= ~(SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI);
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        ssi_unlock();
        return ret;
    }

    PS_PRINT_INFO("module_set=0x%llx\n", module_set);

    halt_det_cnt = 0;
    memset_s(&st_ssi_cpu_infos, sizeof(st_ssi_cpu_infos), 0, sizeof(st_ssi_cpu_infos));

    ssi_read16_etc(GPIO_SSI_REG(SSI_SSI_CTRL));
    ssi_read16_etc(GPIO_SSI_REG(SSI_SEL_CTRL));

    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI); /* switch to ssi clk, wcpu hold */

    PS_PRINT_INFO("switch ssi clk %s\n",
                  (ssi_read16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL)) == SSI_AON_CLKSEL_SSI) ? "ok" : "failed");

    ret = ssi_check_device_isalive();
    if (ret) {
        /* try to reset aon */
        ssi_force_reset_reg();
        PS_PRINT_INFO("ssi_ctrl:0x%x sel_ctrl:0x%x\n",
                      ssi_read16_etc(GPIO_SSI_REG(SSI_SSI_CTRL)),
                      ssi_read16_etc(GPIO_SSI_REG(SSI_SEL_CTRL)));
        ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI);
        if (ssi_check_device_isalive()) {
            PS_PRINT_INFO("after reset aon, ssi still can't work\n");
            goto ssi_fail;
        } else {
            PS_PRINT_INFO("after reset aon, ssi ok, dump acp/ocp reg\n");
            ssi_check_buck_scp_ocp_status();
            module_set = SSI_MODULE_MASK_COMM;
        }
    }

    ret = ssi_dump_device_arm_regs(module_set, 0);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_check_device_isalive();
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_aon_regs(module_set);
    if (ret != OAL_SUCC) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_arm_regs(module_set, 1);
    if (ret) {
        goto ssi_fail;
    }

    /* detect tcxo clock is normal, trigger */
    ssi_dump_device_tcxo_regs(module_set);

    ret = ssi_dump_device_wcpu_key_mem(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_wctrl_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_pcie_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_sdio_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_dump_device_bctl_regs(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO); /* switch from ssi clk, wcpu continue */

    ssi_free_gpio_etc();
    dsm_cpu_info_dump();
    ssi_unlock();

    return 0;
ssi_fail:
    ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO); /* switch from ssi clk, wcpu continue */

    ssi_free_gpio_etc();
    dsm_cpu_info_dump();
    ssi_unlock();
    return ret;
}
#endif

/* SSI调试代码end */
#ifdef _PRE_CONFIG_USE_DTS
static struct of_device_id hi110x_board_match_table[] = {
    {
        .compatible = DTS_COMP_HI110X_BOARD_NAME,
        .data = NULL,
    },
    {
        .compatible = DTS_COMP_HISI_HI110X_BOARD_NAME,
        .data = NULL,
    },
    {},
};
#endif

STATIC struct platform_driver hi110x_board_driver = {
    .probe = hi110x_board_probe,
    .remove = hi110x_board_remove,
    .suspend = hi110x_board_suspend_etc,
    .resume = hi110x_board_resume_etc,
    .driver = {
        .name = "hi110x_board",
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hi110x_board_match_table,
#endif
    },
};

int32 hi110x_board_init_etc(void)
{
    int32 ret;

    board_probe_ret = BOARD_FAIL;
    init_completion(&board_driver_complete);

#ifdef OS_HI1XX_BUILD_VERSION
    hi11xx_os_build_variant = OS_HI1XX_BUILD_VERSION;
    PS_PRINT_INFO("hi11xx_os_build_variant=%d\n", hi11xx_os_build_variant);
#endif

    ret = platform_driver_register(&hi110x_board_driver);
    if (ret) {
        PS_PRINT_ERR("Unable to register hisi connectivity board driver.\n");
        return ret;
    }

    if (wait_for_completion_timeout(&board_driver_complete, 10 * HZ)) {
        /* completed */
        if (board_probe_ret != BOARD_SUCC) {
            PS_PRINT_ERR("hi110x_board probe failed=%d\n", board_probe_ret);
            return board_probe_ret;
        }
    } else {
        /* timeout */
        PS_PRINT_ERR("hi110x_board probe timeout\n");
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("hi110x_board probe succ\n");

    return ret;
}

void hi110x_board_exit_etc(void)
{
    platform_driver_unregister(&hi110x_board_driver);
}

oal_uint8 bus_type = HCC_BUS_SDIO;
oal_int32 wifi_plat_dev_probe_state;

static int hisi_wifi_plat_dev_drv_probe(struct platform_device *pdev)
{
    int ret = 0;
    if (bus_type == HCC_BUS_SDIO) {
        ret = oal_wifi_platform_load_sdio();
        if (ret) {
            printk(KERN_ERR "[HW_CONN] oal_wifi_platform_load_sdio failed.\n");
            wifi_plat_dev_probe_state = -OAL_FAIL;
            return ret;
        }
    }

#ifdef CONFIG_HWCONNECTIVITY
    ret = create_hwconn_proc_file();
    if (ret) {
        printk(KERN_ERR "[HW_CONN] create proc file failed.\n");
        wifi_plat_dev_probe_state = -OAL_FAIL;
        return ret;
    }
#endif
    return ret;
}

static int hisi_wifi_plat_dev_drv_remove(struct platform_device *pdev)
{
    printk(KERN_ERR "[HW_CONN] hisi_wifi_plat_dev_drv_remove.\n");
    return OAL_SUCC;
}

#ifdef _PRE_CONFIG_USE_DTS
static const struct of_device_id hisi_wifi_match_table[] = {
    {
        .compatible = DTS_NODE_HI110X_WIFI,  // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};
#endif

static struct platform_driver hisi_wifi_platform_dev_driver = {
    .probe = hisi_wifi_plat_dev_drv_probe,
    .remove = hisi_wifi_plat_dev_drv_remove,
    .suspend = NULL,
    .shutdown = NULL,
    .driver = {
        .name = DTS_NODE_HI110X_WIFI,
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hisi_wifi_match_table,  // dts required code
#endif
    },
};

int32 hi11xx_get_os_build_variant(void)
{
    return hi11xx_os_build_variant;
}
EXPORT_SYMBOL(hi11xx_get_os_build_variant);

int32 hisi_wifi_platform_register_drv(void)
{
    int32 ret;
    PS_PRINT_FUNCTION_NAME;

    wifi_plat_dev_probe_state = OAL_SUCC;

    ret = platform_driver_register(&hisi_wifi_platform_dev_driver);
    if (ret) {
        PS_PRINT_ERR("Unable to register hisi wifi driver.\n");
    }
    /* platform_driver_register return always true */
    return wifi_plat_dev_probe_state;
}

void hisi_wifi_platform_unregister_drv(void)
{
    PS_PRINT_FUNCTION_NAME;

    platform_driver_unregister(&hisi_wifi_platform_dev_driver);

    return;
}
