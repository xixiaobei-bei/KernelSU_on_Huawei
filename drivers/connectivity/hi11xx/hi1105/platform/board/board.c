

/* 头文件包含 */
#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#include <linux/acpi.h>
#include <linux/gpio/consumer.h>
#endif

/*lint -e322*/ /*lint -e7*/
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif

#include <linux/fs.h>
/*lint +e322*/ /*lint +e7*/

#if defined(CONFIG_LOG_EXCEPTION) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#include <log/log_usertype.h>
#endif
#include "board.h"
#include "plat_debug.h"
#include "oal_ext_if.h"
#include "oal_sdio_host_if.h"
#include "plat_firmware.h"
#include "oal_hcc_bus.h"
#include "plat_pm.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "pcie_host.h"
#include "oal_schedule.h"
#include "platform_common_clk.h"

/* 全局变量定义 */
hi110x_board_info g_st_board_info = { .ssi_gpio_clk = 0, .ssi_gpio_data = 0 };
EXPORT_SYMBOL(g_st_board_info);

OAL_STATIC int32_t g_board_probe_ret = 0;
OAL_STATIC struct completion g_board_driver_complete;
int g_emu_latency_times = 10; // 简单处理, 处理时间乘10
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_emu_latency_times, int, S_IRUGO | S_IWUSR);
#endif

OAL_STATIC device_board_version g_hi110x_board_version_list[] = {
    { .index = BOARD_VERSION_HI1102,  .name = BOARD_VERSION_NAME_HI1102 },
    { .index = BOARD_VERSION_HI1103,  .name = BOARD_VERSION_NAME_HI1103 },
    { .index = BOARD_VERSION_HI1102A, .name = BOARD_VERSION_NAME_HI1102A },
    { .index = BOARD_VERSION_HI1105,  .name = BOARD_VERSION_NAME_HI1105 },
    { .index = BOARD_VERSION_HI1106,  .name = BOARD_VERSION_NAME_HI1106 },
    { .index = BOARD_VERSION_HI1131K, .name = BOARD_VERSION_NAME_HI113K },
};

OAL_STATIC download_mode g_hi110x_download_mode_list[MODE_DOWNLOAD_BUTT] = {
    { .index = MODE_SDIO, .name = DOWNLOAD_MODE_SDIO },
    { .index = MODE_PCIE, .name = DOWNLOAD_MODE_PCIE },
    { .index = MODE_UART, .name = DOWNLOAD_MODE_UART },
};

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC int g_hi11xx_os_build_variant = HI1XX_OS_BUILD_VARIANT_USER; /* default user mode */
#endif

uint32_t g_ssi_dump_en = 0;

/* 函数定义 */
inline hi110x_board_info *get_hi110x_board_info(void)
{
    return &g_st_board_info;
}
EXPORT_SYMBOL_GPL(get_hi110x_board_info);

int hi110x_is_asic(void)
{
    if (g_st_board_info.is_asic == VERSION_ASIC) {
        return VERSION_ASIC;
    } else {
        return VERSION_FPGA;
    }
}
EXPORT_SYMBOL_GPL(hi110x_is_asic);

int hi110x_is_emu(void)
{
    return g_st_board_info.is_emu;
}
EXPORT_SYMBOL(hi110x_is_emu);

void hi110x_emu_mdelay(int msec)
{
    if (hi110x_is_emu() == OAL_TRUE) {
        ps_print_info("emu delay %d msec\n", msec);
        msleep(msec);
    }
}
EXPORT_SYMBOL(hi110x_emu_mdelay);

unsigned int hi110x_get_emu_timeout(unsigned int timeout)
{
    if (hi110x_is_emu() == OAL_TRUE) {
        unsigned int ret = timeout * (unsigned int)g_emu_latency_times;
        ps_print_info("emu timeout %u to %u\n", timeout, ret);
        return ret;
    }
    return timeout;
}
EXPORT_SYMBOL(hi110x_get_emu_timeout);

int is_wifi_support(void)
{
    if (g_st_board_info.is_wifi_disable == 0) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(is_wifi_support);

int is_bfgx_support(void)
{
    if (g_st_board_info.is_bfgx_disable == 0) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(is_bfgx_support);

int pmu_clk_request_enable(void)
{
    if (g_st_board_info.pmu_clk_share_enable == PMU_CLK_REQ_ENABLE) {
        return PMU_CLK_REQ_ENABLE;
    } else {
        return PMU_CLK_REQ_DISABLE;
    }
}

int is_hi110x_debug_type(void)
{
#if defined(CONFIG_LOG_EXCEPTION) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    if (get_logusertype_flag() == BETA_USER) {
        ps_print_info("current soft version is beta user\n");
        return OAL_TRUE;
    }
#endif

    if (hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_USER) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
EXPORT_SYMBOL_GPL(is_hi110x_debug_type);

int32_t get_hi110x_subchip_type(void)
{
    hi110x_board_info *bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -EFAIL;
    }

    return bd_info->chip_nr;
}
EXPORT_SYMBOL_GPL(get_hi110x_subchip_type);

#ifdef _PRE_CONFIG_USE_DTS
int32_t get_board_dts_node(struct device_node **np, const char *node_prop)
{
    if (np == NULL || node_prop == NULL) {
        ps_print_err("func has NULL input param!!!, np=%p, node_prop=%p\n", np, node_prop);
        return BOARD_FAIL;
    }

    *np = of_find_compatible_node(NULL, NULL, node_prop);
    if (*np == NULL) {
        ps_print_err("No compatible node %s found.\n", node_prop);
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32_t get_board_dts_prop(struct device_node *np, const char *dts_prop, const char **prop_val)
{
    int32_t ret;

    if (np == NULL || dts_prop == NULL || prop_val == NULL) {
        ps_print_err("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_property_read_string(np, dts_prop, prop_val);
    if (ret) {
        ps_print_err("can't get dts_prop value: dts_prop=%s\n", dts_prop);
        return ret;
    }

    return BOARD_SUCC;
}

int32_t get_board_dts_gpio_prop(struct device_node *np, const char *dts_prop, int32_t *prop_val)
{
    int32_t ret;

    if (np == NULL || dts_prop == NULL || prop_val == NULL) {
        ps_print_err("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_get_named_gpio(np, dts_prop, 0);
    if (ret < 0) {
        ps_print_err("can't get dts_prop value: dts_prop=%s, ret=%d\n", dts_prop, ret);
        return ret;
    }

    *prop_val = ret;
    ps_print_suc("have get dts_prop and prop_val: %s=%d\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

#endif

int32_t get_board_gpio(const char *gpio_node, const char *gpio_prop, int32_t *physical_gpio)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, gpio_node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = get_board_dts_gpio_prop(np, gpio_prop, physical_gpio);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}

int32_t get_board_custmize(const char *cust_node, const char *cust_prop, const char **cust_prop_val)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;

    if (cust_node == NULL || cust_prop == NULL || cust_prop_val == NULL) {
        ps_print_err("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = get_board_dts_node(&np, cust_node);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = get_board_dts_prop(np, cust_prop, cust_prop_val);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ps_print_info("get board customize info %s=%s\n", cust_prop, *cust_prop_val);

    return BOARD_SUCC;
#else
    return BOARD_FAIL;
#endif
}

#if (defined _PRE_PRODUCT_HI1620S_KUNPENG) || (defined _PRE_HI375X_NO_DTS) || (defined _PRE_WINDOWS_SUPPORT)
static int32_t set_board_pmu_clk32k(struct platform_device *pdev)
{
    return BOARD_SUCC;
}
int32_t enable_board_pmu_clk32k(void)
{
    return BOARD_SUCC;
}
int32_t disable_board_pmu_clk32k(void)
{
    return BOARD_SUCC;
}
#else
static int32_t set_board_pmu_clk32k(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    const char *clk_name = NULL;
    struct clk *clk = NULL;
    struct device *dev = NULL;

    dev = &pdev->dev;
    clk_name = g_st_board_info.clk_32k_name;
    clk = devm_clk_get(dev, clk_name);
    if (clk == NULL) {
        ps_print_err("Get 32k clk %s failed!!!\n", clk_name);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_DTS, CHR_PLAT_DRV_ERROR_32K_CLK_DTS);
        return BOARD_FAIL;
    }
    g_st_board_info.clk_32k = clk;
#endif
    return BOARD_SUCC;
}
int32_t enable_board_pmu_clk32k(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct clk *clk = NULL;

    clk = g_st_board_info.clk_32k;

    if (clk != NULL) {
        if (!__clk_is_enabled(clk)) {
            ret = clk_prepare_enable(clk);
            if (unlikely(ret < 0)) {
                ps_print_err("enable 32K clk failed!!!\n");
                chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                    CHR_PLT_DRV_EVENT_CLK, CHR_PLAT_DRV_ERROR_32K_CLK_EN);
                return BOARD_FAIL;
            }
        }
        ps_print_info("enable 32K clk success!\n");
        return BOARD_SUCC;
    } else {
        ps_print_err("clk32k not config!\n");
        return BOARD_FAIL;
    }
#endif
    return BOARD_SUCC;
}
int32_t disable_board_pmu_clk32k(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    struct clk *clk = NULL;

    clk = g_st_board_info.clk_32k;

    if (clk != NULL) {
        if (__clk_is_enabled(clk)) {
            clk_disable_unprepare(clk);
        }
        ps_print_info("disable 32K clk success!\n");
        return BOARD_SUCC;
    } else {
        ps_print_err("clk32k not config!\n");
        return BOARD_FAIL;
    }
#endif
    return BOARD_SUCC;
}
#endif

static int32_t get_board_uart_port(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_st_board_info.bd_ops.get_board_uart_port();
#else
    return BOARD_SUCC;
#endif
}

static int32_t check_evb_or_fpga(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_st_board_info.bd_ops.check_evb_or_fpga();
#else
    return BOARD_SUCC;
#endif
}

int32_t check_hi110x_subsystem_support(void)
{
    if (g_st_board_info.bd_ops.check_hi110x_subsystem_support == NULL) {
        return BOARD_SUCC;
    }

    return g_st_board_info.bd_ops.check_hi110x_subsystem_support();
}

int32_t check_pmu_clk_share(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_st_board_info.bd_ops.check_pmu_clk_share();
#else
    return BOARD_SUCC;
#endif
}

int32_t board_get_power_pinctrl(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_st_board_info.bd_ops.board_get_power_pinctrl();
#else
    return BOARD_SUCC;
#endif
}

int32_t board_power_gpio_init(struct platform_device *pdev)
{
    return g_st_board_info.bd_ops.get_board_power_gpio(pdev);
}
void free_board_power_gpio(struct platform_device *pdev)
{
    g_st_board_info.bd_ops.free_board_power_gpio(pdev);
}
#ifdef HAVE_HISI_IR
void free_board_ir_gpio(void)
{
    if (g_st_board_info.bfgx_ir_ctrl_gpio > -1) {
        gpio_free(g_st_board_info.bfgx_ir_ctrl_gpio);
    }
}
#endif
void free_board_wakeup_gpio(struct platform_device *pdev)
{
    g_st_board_info.bd_ops.free_board_wakeup_gpio(pdev);
}

void free_board_wifi_tas_gpio(void)
{
    g_st_board_info.bd_ops.free_board_wifi_tas_gpio();
}

int32_t board_wakeup_gpio_init(struct platform_device *pdev)
{
    return g_st_board_info.bd_ops.board_wakeup_gpio_init(pdev);
}

int32_t board_wifi_tas_gpio_init(void)
{
    return g_st_board_info.bd_ops.board_wifi_tas_gpio_init();
}
void free_board_flowctrl_gpio(void)
{
    g_st_board_info.bd_ops.free_board_flowctrl_gpio();
}

int32_t board_flowctrl_gpio_init(void)
{
    return g_st_board_info.bd_ops.board_flowctrl_gpio_init();
}

void board_flowctrl_irq_init(void)
{
    g_st_board_info.flowctrl_irq = oal_gpio_to_irq(g_st_board_info.flowctrl_gpio);
}

#ifdef HAVE_HISI_IR
int32_t board_ir_ctrl_init(struct platform_device *pdev)
{
    return g_st_board_info.bd_ops.board_ir_ctrl_init(pdev);
}
#endif

static int32_t board_gpio_init(struct platform_device *pdev)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return BOARD_SUCC;
#else
    int32_t ret;

    /* power on gpio request */
    ret = board_power_gpio_init(pdev);
    if (ret != BOARD_SUCC) {
        ps_print_err("get power_on dts prop failed\n");
        goto err_get_power_on_gpio;
    }

    ret = board_wakeup_gpio_init(pdev);
    if (ret != BOARD_SUCC) {
        ps_print_err("get wakeup prop failed\n");
        goto oal_board_wakup_gpio_fail;
    }

    ret = board_wifi_tas_gpio_init();
    if (ret != BOARD_SUCC) {
        ps_print_err("get wifi tas prop failed\n");
        goto oal_board_wifi_tas_gpio_fail;
    }

#ifdef HAVE_HISI_IR
    ret = board_ir_ctrl_init(pdev);
    if (ret != BOARD_SUCC) {
        ps_print_err("get ir dts prop failed\n");
        goto err_get_ir_ctrl_gpio;
    }
#endif

#ifdef CONFIG_HWCONNECTIVITY_PC
    g_st_board_info.bd_ops.wlan_wakeup_host_property_init();
#endif

    return BOARD_SUCC;

#ifdef HAVE_HISI_IR
err_get_ir_ctrl_gpio:
    free_board_wifi_tas_gpio();
#endif
oal_board_wifi_tas_gpio_fail:
    free_board_wakeup_gpio(pdev);
oal_board_wakup_gpio_fail:
    free_board_power_gpio(pdev);
err_get_power_on_gpio:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_BOARD_GPIO_INIT);

    return BOARD_FAIL;
#endif
}

int board_get_bwkup_gpio_val(void)
{
    return oal_gpio_get_value(g_st_board_info.bfgn_wakeup_host);
}
int board_get_gwkup_gpio_val(void)
{
    return oal_gpio_get_value(g_st_board_info.gcpu_wakeup_host);
}
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
int board_get_bfgx_enable_gpio_val(void)
{
    return gpio_get_value(g_st_board_info.bfgn_power_on_enable);
}

int board_get_wlan_enable_gpio_val(void)
{
    return gpio_get_value(g_st_board_info.wlan_power_on_enable);
}
#endif
#ifdef CONFIG_HWCONNECTIVITY_PC
/* wlan_wakeup_host_have_reverser  判断取反器是否存在 */
int board_get_wlan_wkup_gpio_val(void)
{
    bool ret = 0;
    ret = gpio_get_value(g_st_board_info.wlan_wakeup_host);

    ps_print_info("wlan_wakeup_host val: %d\n", ret);

    if (g_st_board_info.wlan_wakeup_host_have_reverser == 1) {
        return !ret;
    }

    return ret;
}
#else
int board_get_wlan_wkup_gpio_val(void)
{
    return oal_gpio_get_value(g_st_board_info.wlan_wakeup_host);
}
#endif

static int32_t board_irq_init(void)
{
    uint32_t irq;
    int32_t gpio;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    gpio = g_st_board_info.wlan_wakeup_host;
    irq = gpio_to_irq(gpio);
    g_st_board_info.wlan_irq = irq;

    ps_print_info("wlan_irq is %d\n", g_st_board_info.wlan_irq);
#endif

    gpio = g_st_board_info.bfgn_wakeup_host;
    irq = gpio_to_irq(gpio);
    g_st_board_info.bfgx_irq = irq;

    ps_print_info("bfgx_irq is %d\n", g_st_board_info.bfgx_irq);

    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1106) {
        gpio = g_st_board_info.gcpu_wakeup_host;
        irq = gpio_to_irq(gpio);
        g_st_board_info.gcpu_irq = irq;

        ps_print_info("gcpu_irq is %d\n", g_st_board_info.gcpu_irq);
    }

    return BOARD_SUCC;
}

static int32_t board_clk_init(struct platform_device *pdev)
{
    int32_t ret;

    if (pdev == NULL) {
        ps_print_err("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = g_st_board_info.bd_ops.get_board_pmu_clk32k();
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    ret = set_board_pmu_clk32k(pdev);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

void power_state_change(int32_t gpio, int32_t flag)
{
#ifdef _PRE_PRODUCT_HI3751V811
#else
    if (unlikely(gpio == 0)) {
        ps_print_warning("gpio is 0, flag=%d\n", flag);
        return;
    }
#endif
    ps_print_info("power_state_change gpio %d to %s\n", gpio, (flag == BOARD_POWER_ON) ? "low2high" : "low");

    if (flag == BOARD_POWER_ON) {
        oal_gpio_direction_output(gpio, GPIO_LOWLEVEL);
        mdelay(10); /* delay 10ms */
        oal_gpio_direction_output(gpio, GPIO_HIGHLEVEL);
        mdelay(20); /* delay 20ms */
    } else if (flag == BOARD_POWER_OFF) {
        oal_gpio_direction_output(gpio, GPIO_LOWLEVEL);
    }
}

int32_t board_wlan_gpio_power_on(void *data)
{
    int32_t gpio = (int32_t)(uintptr_t)(data);
    if (g_st_board_info.host_wakeup_wlan) {
        /* host wakeup dev gpio pinmux to jtag when w boot,
          must gpio low when bootup */
        board_host_wakeup_dev_set(0);
    }
    power_state_change(gpio, BOARD_POWER_ON);
    if (g_st_board_info.ep_pcie0_rst) {
        /* 拉高pcie_rst */
        oal_gpio_direction_output(g_st_board_info.ep_pcie0_rst, GPIO_LOWLEVEL);
        mdelay(1);
        oal_gpio_direction_output(g_st_board_info.ep_pcie0_rst, GPIO_HIGHLEVEL);
        mdelay(10); /* delay 10ms */
        ps_print_info("pcie0_rst %d pull up\n", g_st_board_info.ep_pcie0_rst);
    }
    hi110x_emu_mdelay(50000); // 50000 msec, 等待慢速侧PCIE建链, 时间取决于EMU平台的性能
    board_host_wakeup_dev_set(1);
    return 0;
}

int32_t board_wlan_gpio_power_off(void *data)
{
    int32_t gpio = (int32_t)(uintptr_t)(data);
    if (g_st_board_info.ep_pcie0_rst) {
        /* 拉低pcie_rst */
        oal_gpio_direction_output(g_st_board_info.ep_pcie0_rst, GPIO_LOWLEVEL);
        mdelay(1);
        ps_print_info("pcie0_rst %d pull down\n", g_st_board_info.ep_pcie0_rst);
    }
    power_state_change(gpio, BOARD_POWER_OFF);
    hi110x_emu_mdelay(10000); // 10000:5ms * 1000 * 2
    return 0;
}

int32_t board_host_wakeup_dev_set(int value)
{
    if (g_st_board_info.host_wakeup_wlan == 0) {
        ps_print_info("host_wakeup_wlan gpio is 0\n");
        return 0;
    }
    ps_print_info("host_wakeup_wlan set %s\n", value ? "high" : "low");
    if (value) {
        return oal_gpio_direction_output(g_st_board_info.host_wakeup_wlan, GPIO_HIGHLEVEL);
    } else {
        return oal_gpio_direction_output(g_st_board_info.host_wakeup_wlan, GPIO_LOWLEVEL);
    }
}

int32_t board_host_wakeup_bfg_set(int value)
{
    if (g_st_board_info.host_wakeup_bfg == 0) {
        ps_print_info("host_wakeup_bfg gpio is 0\n");
        return 0;
    }

    if (value) {
        return oal_gpio_direction_output(g_st_board_info.host_wakeup_bfg, GPIO_HIGHLEVEL);
    } else {
        return oal_gpio_direction_output(g_st_board_info.host_wakeup_bfg, GPIO_LOWLEVEL);
    }
}

int32_t board_set_pcie_reset(int32_t is_master, int32_t is_poweron)
{
    int32_t value = (is_poweron == OAL_TRUE) ? GPIO_HIGHLEVEL : GPIO_LOWLEVEL;
    int32_t gpio  = (is_master == OAL_TRUE) ? g_st_board_info.ep_pcie0_rst : g_st_board_info.ep_pcie1_rst;

    ps_print_info("set pcie reset gpio=%d value=%d\n", gpio, value);

    if (gpio == 0) {
        return -OAL_ENODEV;
    }

    oal_gpio_direction_output(gpio, value);
    return OAL_SUCC;
}

int32_t board_get_host_wakeup_dev_stat(void)
{
    return oal_gpio_get_value(g_st_board_info.host_wakeup_wlan);
}

int32_t board_wifi_tas_set(int value)
{
    if (g_st_board_info.wifi_tas_enable == WIFI_TAS_DISABLE) {
        return 0;
    }

    ps_print_dbg("wifi tas gpio set %s %pF\n", value ? "high" : "low", (void *)_RET_IP_);

    if (value) {
        return gpio_direction_output(g_st_board_info.rf_wifi_tas, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(g_st_board_info.rf_wifi_tas, GPIO_LOWLEVEL);
    }
}

EXPORT_SYMBOL(board_wifi_tas_set);

int32_t board_get_wifi_tas_gpio_state(void)
{
    if (g_st_board_info.wifi_tas_enable == WIFI_TAS_DISABLE) {
        return 0;
    }

    return gpio_get_value(g_st_board_info.rf_wifi_tas);
}

EXPORT_SYMBOL(board_get_wifi_tas_gpio_state);

int32_t board_power_on(uint32_t ul_subsystem)
{
    return g_st_board_info.bd_ops.board_power_on(ul_subsystem);
}
int32_t board_power_off(uint32_t ul_subsystem)
{
    return g_st_board_info.bd_ops.board_power_off(ul_subsystem);
}

int32_t board_power_reset(uint32_t ul_subsystem)
{
    return g_st_board_info.bd_ops.board_power_reset(ul_subsystem);
}
EXPORT_SYMBOL(board_wlan_gpio_power_off);
EXPORT_SYMBOL(board_wlan_gpio_power_on);

/* just for hi1106 fpga test, delete later */
static void hi110x_subchip_check_tmp(void)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return;
#else
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;

    ret = get_board_dts_node(&np, DTS_NODE_HISI_HI110X);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_HI110X);
        return;
    }

    ret = of_property_read_bool(np, BOARD_VERSION_NAME_HI1105);
    if (ret) {
        ps_print_info("hisi subchip type is 05\n");
        g_st_board_info.chip_type = BOARD_VERSION_NAME_HI1105;
    }

    ret = of_property_read_bool(np, BOARD_VERSION_NAME_HI1106);
    if (ret) {
        ps_print_info("hisi subchip type is 06 for fpga\n");
        g_st_board_info.chip_type = BOARD_VERSION_NAME_HI1106;
    }

    return;
#else
    return;
#endif
#endif
}

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
int32_t find_device_board_version(void)
{
#ifdef _PRE_WINDOWS_SUPPORT
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_HOST)
    g_st_board_info.chip_type = BOARD_VERSION_NAME_HI1105;
#else
    g_st_board_info.chip_type = BOARD_VERSION_NAME_HI1103;
#endif
#else
    g_st_board_info.chip_type = BOARD_VERSION_NAME_HI1103;
#endif
    return BOARD_SUCC;
}
#else
int32_t find_device_board_version(void)
{
    int32_t ret;
    const char *device_version = NULL;

    ret = get_board_custmize(DTS_NODE_HISI_HI110X, DTS_PROP_SUBCHIP_TYPE_VERSION, &device_version);
    if (ret != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    g_st_board_info.chip_type = device_version;
    return BOARD_SUCC;
}
#endif

static int32_t check_device_board_name(void)
{
    int32_t i = 0;
    for (i = 0; i < oal_array_size(g_hi110x_board_version_list); i++) {
        if (strncmp(g_hi110x_board_version_list[i].name, g_st_board_info.chip_type, HI11XX_SUBCHIP_NAME_LEN_MAX) ==
            0) {
            g_st_board_info.chip_nr = g_hi110x_board_version_list[i].index;
            return BOARD_SUCC;
        }
    }

    return BOARD_FAIL;
}

const uint8_t* get_device_board_name(void)
{
    if (oal_unlikely(g_st_board_info.chip_type == NULL)) {
        return NULL;
    }

    return g_st_board_info.chip_type;
}

int32_t board_chiptype_init(void)
{
    int32_t ret;

    ret = find_device_board_version();
    if (ret != BOARD_SUCC) {
        ps_print_err("can not find device_board_version\n");
        return BOARD_FAIL;
    }

    /* just for hi110x fpga test, delete later */
    hi110x_subchip_check_tmp();

    ret = check_device_board_name();
    if (ret != BOARD_SUCC) {
        ps_print_err("check device name fail\n");
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

#ifdef _PRE_HI_DRV_GPIO
int stub_init(void)
{
    return BOARD_SUCC;
}
void stub_init_void(struct platform_device *pdev)
{
    return;
}

void stub_init_ext_void(void)
{
    return;
}
#endif

STATIC void board_info_init_hi1103(void)
{
#ifdef _PRE_HI_DRV_GPIO
    g_st_board_info.bd_ops.get_board_power_gpio = hitv_get_board_power_gpio;
    g_st_board_info.bd_ops.free_board_power_gpio = stub_init_void;
    g_st_board_info.bd_ops.free_board_flowctrl_gpio = stub_init_ext_void;
    g_st_board_info.bd_ops.board_wakeup_gpio_init = hitv_board_wakeup_gpio_init;
    g_st_board_info.bd_ops.free_board_wakeup_gpio = stub_init_void;
    g_st_board_info.bd_ops.board_flowctrl_gpio_init = stub_init;
    g_st_board_info.bd_ops.board_wifi_tas_gpio_init = stub_init;
#else
    g_st_board_info.bd_ops.get_board_power_gpio = hi1103_get_board_power_gpio;
    g_st_board_info.bd_ops.free_board_power_gpio = hi1103_free_board_power_gpio;
    g_st_board_info.bd_ops.free_board_flowctrl_gpio = hi1103_free_board_flowctrl_gpio;
    g_st_board_info.bd_ops.board_wakeup_gpio_init = hi1103_board_wakeup_gpio_init;
    g_st_board_info.bd_ops.free_board_wakeup_gpio = hi1103_free_board_wakeup_gpio;
    g_st_board_info.bd_ops.board_flowctrl_gpio_init = hi1103_board_flowctrl_gpio_init;
    g_st_board_info.bd_ops.board_wifi_tas_gpio_init = hi1103_board_wifi_tas_gpio_init;
#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
    g_st_board_info.bd_ops.suspend_board_gpio_in_s4 = hi1103_suspend_gpio;
    g_st_board_info.bd_ops.resume_board_gpio_in_s4 = hi1103_resume_gpio;
#endif
#endif
    g_st_board_info.bd_ops.free_board_wifi_tas_gpio = hi1103_free_board_wifi_tas_gpio;
    g_st_board_info.bd_ops.bfgx_dev_power_on = hi1103_bfgx_dev_power_on;
    g_st_board_info.bd_ops.bfgx_dev_power_off = hi1103_bfgx_dev_power_off;
    g_st_board_info.bd_ops.wlan_power_off = hi1103_wlan_power_off;
    g_st_board_info.bd_ops.wlan_power_on = hi1103_wlan_power_on;
    g_st_board_info.bd_ops.board_power_on = hi1103_board_power_on;
    g_st_board_info.bd_ops.board_power_off = hi1103_board_power_off;
    g_st_board_info.bd_ops.board_power_reset = hi1103_board_power_reset;
    g_st_board_info.bd_ops.get_board_pmu_clk32k = hi1103_get_board_pmu_clk32k;
    g_st_board_info.bd_ops.get_board_uart_port = hi1103_get_board_uart_port;
    g_st_board_info.bd_ops.board_ir_ctrl_init = hi1103_board_ir_ctrl_init;
    g_st_board_info.bd_ops.check_evb_or_fpga = hi1103_check_evb_or_fpga;
    g_st_board_info.bd_ops.check_pmu_clk_share = hi1103_check_pmu_clk_share;
    g_st_board_info.bd_ops.board_get_power_pinctrl = hi1103_board_get_power_pinctrl;
    g_st_board_info.bd_ops.get_ini_file_name_from_dts = hi1103_get_ini_file_name_from_dts;
    g_st_board_info.bd_ops.check_hi110x_subsystem_support = hi1103_check_hi110x_subsystem_support;
#ifdef CONFIG_HWCONNECTIVITY_PC
    g_st_board_info.bd_ops.wlan_wakeup_host_property_init = hi1103_wlan_wakeup_host_property_init;
#endif
}

#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
void suspend_board_gpio_in_s4(void)
{
    ps_print_info("s4 suspend gpio..\n");
    if (g_st_board_info.bd_ops.suspend_board_gpio_in_s4 == NULL) {
        ps_print_err("suspend_board_gpio_in_s4 is null.\n");
        return;
    }
    g_st_board_info.bd_ops.suspend_board_gpio_in_s4();
    return;
}
void resume_board_gpio_in_s4(void)
{
    ps_print_info("s4 resume gpio..\n");
    if (g_st_board_info.bd_ops.resume_board_gpio_in_s4 == NULL) {
        ps_print_err("suspend_board_gpio_in_s4 is null.\n");
        return;
    }
    g_st_board_info.bd_ops.resume_board_gpio_in_s4();
    return;
}
#endif

static int32_t board_func_init(void)
{
    int32_t ret;

    memset_s(&g_st_board_info, sizeof(g_st_board_info), 0, sizeof(g_st_board_info));

    g_st_board_info.is_wifi_disable = 0;
    g_st_board_info.is_bfgx_disable = 0;

    ret = board_chiptype_init();
    if (ret != BOARD_SUCC) {
        ps_print_err("sub chiptype init fail\n");
        return BOARD_FAIL;
    }

    board_info_init_hi1103();
    ps_print_info("g_st_board_info.chip_nr=%d \n", g_st_board_info.chip_nr);
    return BOARD_SUCC;
}

static int32_t check_download_channel_name(const char *wlan_buff, int32_t *index)
{
    int32_t i = 0;
    for (i = 0; i < MODE_DOWNLOAD_BUTT; i++) {
        if (strncmp(g_hi110x_download_mode_list[i].name, wlan_buff, strlen(g_hi110x_download_mode_list[i].name)) ==
            0) {
            *index = i;
            return BOARD_SUCC;
        }
    }
    return BOARD_FAIL;
}

static int32_t get_download_channel(void)
{
    int32_t ret;
    uint8_t wlan_mode[DOWNLOAD_CHANNEL_LEN] = {0};
    uint8_t bfgn_mode[DOWNLOAD_CHANNEL_LEN] = {0};

    /* wlan channel */
    ret = get_cust_conf_string(INI_MODU_PLAT, INI_WLAN_DOWNLOAD_CHANNEL, wlan_mode, sizeof(wlan_mode));
    if (ret != INI_SUCC) {
        /* 兼容1102,1102无此配置项 */
#if defined(_PRE_WLAN_PCIE_ONLY) || defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
        g_st_board_info.wlan_download_channel = MODE_PCIE;
        ps_print_warning("can not find wlan_download_channel ,choose default:%s\n", "pcie");
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, "pcie");
#else
        g_st_board_info.wlan_download_channel = MODE_SDIO;
        ps_print_warning("can not find wlan_download_channel ,choose default:%s\n",
                         g_hi110x_download_mode_list[0].name);
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, NULL);
#endif
    } else {
        if (check_download_channel_name(wlan_mode, &(g_st_board_info.wlan_download_channel)) != BOARD_SUCC) {
            ps_print_err("check wlan download channel:%s error\n", bfgn_mode);
            return BOARD_FAIL;
        }
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, wlan_mode);
    }

    /* bfgn channel */
    ret = get_cust_conf_string(INI_MODU_PLAT, INI_BFGX_DOWNLOAD_CHANNEL, bfgn_mode, sizeof(bfgn_mode));
    if (ret != INI_SUCC) {
        /* 如果不存在该项，则默认保持和wlan一致 */
        g_st_board_info.bfgn_download_channel = g_st_board_info.wlan_download_channel;
        ps_print_warning("can not find bfgn_download_channel ,choose default:%s\n",
                         g_hi110x_download_mode_list[0].name);
        return BOARD_SUCC;
    }

    if (check_download_channel_name(bfgn_mode, &(g_st_board_info.bfgn_download_channel)) != BOARD_SUCC) {
        ps_print_err("check bfgn download channel:%s error\n", bfgn_mode);
        return BOARD_FAIL;
    }

    ps_print_info("wlan_download_channel index:%d, bfgn_download_channel index:%d\n",
                  g_st_board_info.wlan_download_channel, g_st_board_info.bfgn_download_channel);

    return BOARD_SUCC;
}

static int32_t get_ssi_dump_cfg(void)
{
    int32_t l_cfg_value = 0;
    int32_t l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_PLAT, INI_SSI_DUMP_EN, &l_cfg_value);
    if (l_ret == INI_FAILED) {
        ps_print_err("get_ssi_dump_cfg: fail to get ini, keep disable\n");
        return BOARD_SUCC;
    }

    g_ssi_dump_en = (uint32_t)l_cfg_value;

    ps_print_info("get_ssi_dump_cfg: 0x%x\n", g_ssi_dump_en);

    return BOARD_SUCC;
}

int32_t get_uart_pclk_source(void)
{
    return g_st_board_info.uart_pclk;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
STATIC void board_cci_bypass_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;

    if (g_pcie_memcopy_type == -1) {
        ps_print_info("skip pcie mem burst control\n");
        return;
    }

    ret = get_board_dts_node(&np, DTS_NODE_HISI_CCIBYPASS);
    if (ret != BOARD_SUCC) {
        /* cci enable */
        g_pcie_memcopy_type = 0;
        ps_print_info("cci enable, pcie use mem burst 8 bytes\n");
    } else {
        /* cci bypass */
        g_pcie_memcopy_type = 1;
        ps_print_info("cci bypass, pcie use mem burst 4 bytes\n");
    }
#endif
}
#endif

STATIC void buck_param_init_by_ini(void)
{
    int32_t l_cfg_value = 0;
    int32_t l_ret;

    /* 获取ini的配置值 */
    l_ret = get_cust_conf_int32(INI_MODU_PLAT, "buck_mode", &l_cfg_value);
    if (l_ret == INI_FAILED) {
        ps_print_err("get_ssi_dump_cfg: fail to get ini, keep disable\n");
        return;
    }

    g_st_board_info.buck_param = (uint16_t)l_cfg_value;

    ps_print_info("buck_param_init_by_ini: 0x%x\n",  g_st_board_info.buck_param);

    return;
}

/*
*  函 数 名  : buck_param_init
*  功能描述  : 从定制化ini文件中提取buck模式的配置参数。
*   110x支持内置buck和外置buck,firmware CFG文件中仅有关键字，实际值从ini文件中获取，方便定制化。
*   ini定制化格式[buck_mode=2,BUCK_CUSTOM_REG,value],符合CFG文件的WRITEM的语法,0x50001850为1105的BUCK_CUSTOM_REG
*   地址, value根据实际要求配置:
*   （1）   BIT[15,14]：表示是否采用外置buck
*           2'b00:  全内置buck
*           2'b01:  I2C控制独立外置buck
*           2'b10:  GPIO控制独立外置buck
*           2'b11:  host控制共享外置buck电压
*   （2）   BIT[8]：表示先调电压，才能buck_en.
*   （3）   BIT[7，0]: 代表不同的Buck器件型号
*/
STATIC void buck_param_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;
    int32_t buck_mode = 0;

    g_st_board_info.buck_param = 0;
    g_st_board_info.buck_control_flag = 0;

    ret = get_board_dts_node(&np, DTS_NODE_HISI_HI110X);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HISI_HI110X);
        return  buck_param_init_by_ini();
    }

    ret = of_property_read_u32(np, DTS_PROP_HI110X_BUCK_MODE, &buck_mode);
    if (ret == BOARD_SUCC) {
        ps_print_info("buck_param_init get dts config:0x%x\n", buck_mode);
        g_st_board_info.buck_param = (uint16_t)buck_mode;
    } else {
        ps_print_err("buck_param_init fail,get from ini\n");
        return  buck_param_init_by_ini();
    }

    ps_print_info("buck_param_init success,buck_param[0x%x],buck_control_flag[0x%x]\n",
                  g_st_board_info.buck_param, g_st_board_info.buck_control_flag);
    return ;
#else
    g_st_board_info.buck_param = 0;
    g_st_board_info.buck_control_flag = 0;

    return  buck_param_init_by_ini();
#endif
}

#ifdef _PRE_HI_DRV_GPIO
#define SSI_CLK_HISI_GPIO 7*8
#define SSI_DATA_HISI_GPIO (6 * 8 + 7)

STATIC void hitv_ssi_gpio_init(void)
{
    hitv_gpio_request(SSI_CLK_HISI_GPIO, 0);
    hitv_gpio_direction_output(SSI_CLK_HISI_GPIO, GPIO_HIGHLEVEL);
    hitv_gpio_request(SSI_DATA_HISI_GPIO, 0);
    hitv_gpio_direction_output(SSI_DATA_HISI_GPIO, GPIO_HIGHLEVEL);
}
#endif

STATIC int32_t hi110x_board_probe(struct platform_device *pdev)
{
    int ret;

    ret = board_func_init();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = ini_cfg_init();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }
#ifndef _PRE_PRODUCT_HI1620S_KUNPENG
    read_tcxo_dcxo_ini_file(); // 读取共时钟配置
#endif

    ret = check_hi110x_subsystem_support();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = check_evb_or_fpga();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = check_pmu_clk_share();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = get_download_channel();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = get_ssi_dump_cfg();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_clk_init(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    board_cci_bypass_init();
#endif

    ret = get_board_uart_port();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_gpio_init(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = board_irq_init();
    if (ret != BOARD_SUCC) {
        goto err_gpio_source;
    }

    ret = board_get_power_pinctrl();
    if (ret != BOARD_SUCC) {
        goto err_get_power_pinctrl;
    }

    buck_param_init();

    ps_print_info("board init ok\n");

    g_board_probe_ret = BOARD_SUCC;
    complete(&g_board_driver_complete);

    return BOARD_SUCC;

err_get_power_pinctrl:

err_gpio_source:
#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif
    free_board_wakeup_gpio(pdev);
    free_board_power_gpio(pdev);
    free_board_flowctrl_gpio();

err_init:
    g_board_probe_ret = BOARD_FAIL;
    complete(&g_board_driver_complete);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_BOARD_DRV_PROB);
    return BOARD_FAIL;
}

STATIC int32_t hi110x_board_remove(struct platform_device *pdev)
{
    ps_print_info("hi110x board exit\n");

#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif

    free_board_wakeup_gpio(pdev);
    free_board_power_gpio(pdev);
    free_board_flowctrl_gpio();
#ifdef _PRE_HI_DRV_GPIO
    gpio_free(g_st_board_info.wlan_wakeup_host);
#endif
    return BOARD_SUCC;
}

static int32_t hi110x_board_suspend(struct platform_device *pdev, oal_pm_message_t state)
{
    return BOARD_SUCC;
}

static int32_t hi110x_board_resume(struct platform_device *pdev)
{
    return BOARD_SUCC;
}

#ifdef _PRE_CONFIG_USE_DTS
STATIC struct of_device_id g_hi110x_board_match_table[] = {
    {
        .compatible = DTS_COMP_HISI_HI110X_BOARD_NAME,
        .data = NULL,
    },
#ifdef _PRE_WINDOWS_SUPPORT
    // {}, This format is not supported in windows
#else
    {},
#endif
};
#endif

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
static const struct acpi_device_id g_hi110x_board_acpi_match[] = {
    { "HISI1103", 0 },
    { "HISI110X", 0 },
#ifdef _PRE_WINDOWS_SUPPORT
    // {}, This format is not supported in windows
#else
    {},
#endif
};
#endif

STATIC struct platform_driver g_hi110x_board_driver = {
    .probe = hi110x_board_probe,
    .remove = hi110x_board_remove,
    .suspend = hi110x_board_suspend,
    .resume = hi110x_board_resume,
    .driver = {
        .name = "hi110x_board",
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = g_hi110x_board_match_table,
#endif
#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
        .acpi_match_table = ACPI_PTR(g_hi110x_board_acpi_match),
#endif
    },
};

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
MODULE_DEVICE_TABLE(acpi, g_hi110x_board_acpi_match);
#endif

int32_t hi110x_board_init(void)
{
    int32_t ret;

    g_board_probe_ret = BOARD_FAIL;
    init_completion(&g_board_driver_complete);

#ifdef _PRE_HI_DRV_GPIO
    hitv_ssi_gpio_init();
#endif
#ifdef OS_HI1XX_BUILD_VERSION
    g_hi11xx_os_build_variant = OS_HI1XX_BUILD_VERSION;
    ps_print_info("hi11xx_os_build_variant=%d\n", g_hi11xx_os_build_variant);
#endif

#ifdef _PRE_WINDOWS_SUPPORT
    oal_spin_lock_init(get_ssi_lock_glb_addr());
    ret = hi110x_board_probe(&g_hi110x_board_driver);
#else
    ret = platform_driver_register(&g_hi110x_board_driver);
    if (ret) {
        ps_print_err("Unable to register hisi connectivity board driver.\n");
        return ret;
    }

    if (wait_for_completion_timeout(&g_board_driver_complete, 10 * HZ)) { /* 10 */
        /* completed */
        if (g_board_probe_ret != BOARD_SUCC) {
            platform_driver_unregister(&g_hi110x_board_driver);
            ps_print_err("hi110x_board probe failed=%d\n", g_board_probe_ret);
            return g_board_probe_ret;
        }
    } else {
        /* timeout */
        platform_driver_unregister(&g_hi110x_board_driver);
        ps_print_err("hi110x_board probe timeout\n");
        return BOARD_FAIL;
    }
#endif
    ps_print_info("hi110x_board probe succ\n");

    return ret;
}

void hi110x_board_exit(void)
{
    platform_driver_unregister(&g_hi110x_board_driver);
}

int32_t hi11xx_get_os_build_variant(void)
{
    return g_hi11xx_os_build_variant;
}
EXPORT_SYMBOL(hi11xx_get_os_build_variant);

hi110x_release_vtype hi110x_get_release_type(void)
{
    hi110x_release_vtype vtype = HI110X_VTYPE_DEBUG;

    /* check the version release or debug */
    if (hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_ROOT) {
        ps_print_dbg("root user\n");
        return vtype;
    }

#if defined(CONFIG_LOG_EXCEPTION) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    if (get_logusertype_flag() == BETA_USER) {
        /* beta user */
        ps_print_dbg("beta user\n");
        return HI110X_VTYPE_RELEASE_DEBUG;
    } else {
        /* release(user & non beta user) */
        ps_print_dbg("release user\n");
        return HI110X_VTYPE_RELEASE;
    }
#else
    /* kernel module or no beta user function */
    ps_print_dbg("default user\n");
    vtype = HI110X_VTYPE_DEBUG;
#endif

    return vtype;
}
EXPORT_SYMBOL(hi110x_get_release_type);

