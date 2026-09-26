

/* 头文件包含 */
#define HI11XX_LOG_MODULE_NAME     "[HI1103_BOARD]"
#define HI11XX_LOG_MODULE_NAME_VAR hi1103_board_loglevel
#include "board_hi1103.h"

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
/*lint +e322*/ /*lint +e7*/
#include "plat_debug.h"
#include "bfgx_dev.h"
#include "plat_pm.h"
#include "oal_hcc_host_if.h"
#include "plat_uart.h"
#include "plat_firmware.h"
#include "pcie_linux.h"
#include "securec.h"
#include "chr_user.h"

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#include "bfgx_data_parse.h"
#include "wireless_patch.h"
#endif

#define SSI_GPIO_GROUP_ID                       480
#define SSI_GPIO_CLK_INDEX                      27
#define SSI_GPIO_DATA_INDEX                     26
#define POWER_ON_GPIO_ACPI_INDEX                0
#define BFGN_POWER_ON_GPIO_ACPI_INDEX           2
#define WLAN_POWER_ON_GPIO_ACPI_INDEX           1
#define BFGN_WAKEUP_HOST_GPIO_ACPI_INDEX        8
#define WLAN_WAKEUP_HOST_GPIO_ACPI_INDEX        7
#define HOST_WAKEUP_WLAN_GPIO_ACPI_INDEX        3

#ifdef PLATFORM_DEBUG_ENABLE
int32_t g_device_monitor_enable = 0;
#endif

STATIC int32_t g_ft_fail_powerdown_bypass = 0;

#define INI_WLAN_WAKEUP_HOST_REVERSE "wlan_wakeup_host_have_reverser"

STATIC int32_t request_ssi_gpio(void)
{
    int32_t ret;

    g_st_board_info.ssi_gpio_clk = (SSI_GPIO_GROUP_ID + SSI_GPIO_CLK_INDEX);
    g_st_board_info.ssi_gpio_data = (SSI_GPIO_GROUP_ID + SSI_GPIO_DATA_INDEX);

    ret = ssi_request_gpio(g_st_board_info.ssi_gpio_clk, g_st_board_info.ssi_gpio_data);
    if (ret != BOARD_SUCC) {
        ps_print_err("request ssi gpio fail\n");
        g_st_board_info.ssi_gpio_clk = 0;
        g_st_board_info.ssi_gpio_data = 0;
        return ret;
    }

    return BOARD_SUCC;
}

STATIC void free_ssi_gpio(void)
{
    ssi_free_gpio();
    g_st_board_info.ssi_gpio_clk = 0;
    g_st_board_info.ssi_gpio_data = 0;
}

int32_t hi1103_get_board_power_gpio(struct platform_device *pdev)
{
    struct gpio_desc *pdesc_power_on_enable = NULL;
    struct gpio_desc *pdesc_bfgn_power_on_enable = NULL;
    struct gpio_desc *pdesc_wlan_power_on_enable = NULL;

    if (pdev == NULL) {
        ps_print_err("%s has NULL input param!!!\n", __func__);
        return BOARD_FAIL;
    }

    request_ssi_gpio();

    pdesc_power_on_enable = devm_gpiod_get_index_optional(&pdev->dev, NULL, POWER_ON_GPIO_ACPI_INDEX, GPIOD_ASIS);
    if (pdesc_power_on_enable == NULL) {
        ps_print_err("%s pdesc_power_on_enable NULL!!!\n", __func__);
        free_ssi_gpio();
        return BOARD_FAIL;
    }
    g_st_board_info.power_on_enable = desc_to_gpio(pdesc_power_on_enable);
    ps_print_info("from gpiod CRS[0] power_on_enable:%d, gpio- ssi clk gpio:%d, ssi data gpio:%d\n",
                  g_st_board_info.power_on_enable, g_st_board_info.ssi_gpio_clk, g_st_board_info.ssi_gpio_data);

    pdesc_bfgn_power_on_enable = devm_gpiod_get_index_optional(&pdev->dev,
        NULL, BFGN_POWER_ON_GPIO_ACPI_INDEX, GPIOD_ASIS);
    if (pdesc_bfgn_power_on_enable == NULL) {
        ps_print_err("%s pdesc_bfgn_power_on_enable NULL!!!\n", __func__);
        goto err_get_bfgx_power_gpio;
    }
    g_st_board_info.bfgn_power_on_enable = desc_to_gpio(pdesc_bfgn_power_on_enable);
    ps_print_info("from gpiod CRS[2] bfgn_power_on_enable:%d\n", g_st_board_info.bfgn_power_on_enable);

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    pdesc_wlan_power_on_enable = devm_gpiod_get_index_optional(&pdev->dev,
        NULL, WLAN_POWER_ON_GPIO_ACPI_INDEX, GPIOD_ASIS);
    if (pdesc_wlan_power_on_enable == NULL) {
        ps_print_err("%s pdesc_wlan_power_on_enable NULL!!!\n", __func__);
        goto err_get_wlan_power_gpio;
    }
    g_st_board_info.wlan_power_on_enable = desc_to_gpio(pdesc_wlan_power_on_enable);
    ps_print_info("from gpiod CRS[1] wlan_power_on_enable:%d\n", g_st_board_info.wlan_power_on_enable);
#endif
    return BOARD_SUCC;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
err_get_wlan_power_gpio:
    devm_gpiod_put(&pdev->dev, pdesc_bfgn_power_on_enable);
#endif
err_get_bfgx_power_gpio:
    devm_gpiod_put(&pdev->dev, pdesc_power_on_enable);
    free_ssi_gpio();
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DEV_EVENT_CHIP,
                         CHR_PLAT_DRV_ERROR_POWER_GPIO);

    return BOARD_FAIL;
}

void hi1103_free_board_power_gpio(struct platform_device *pdev)
{
    devm_gpiod_put(&pdev->dev, gpio_to_desc(g_st_board_info.power_on_enable));
    devm_gpiod_put(&pdev->dev, gpio_to_desc(g_st_board_info.bfgn_power_on_enable));

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    devm_gpiod_put(&pdev->dev, gpio_to_desc(g_st_board_info.wlan_power_on_enable));
#endif
    free_ssi_gpio();
}

int32_t hi1103_board_wakeup_gpio_init(struct platform_device *pdev)
{
    struct gpio_desc *pdesc_host_wakeup_wlan = NULL;
    struct gpio_desc *pdesc_wlan_wakeup_host = NULL;
    struct gpio_desc *pdesc_bfgn_wakeup_host = NULL;

    /* bfgx wake host gpio request */
    pdesc_bfgn_wakeup_host = devm_gpiod_get_index_optional(&pdev->dev,
        NULL, BFGN_WAKEUP_HOST_GPIO_ACPI_INDEX, GPIOD_ASIS);
    if (pdesc_bfgn_wakeup_host == NULL) {
        ps_print_err("%s pdesc_bfgn_power_on_enable NULL!!!\n", __func__);
        goto err_get_bfgx_wkup_host_gpio;
    }
    g_st_board_info.bfgn_wakeup_host = desc_to_gpio(pdesc_bfgn_wakeup_host);
    ps_print_info("from gpiod CRS[8] bfgn_wakeup_host:%d\n", g_st_board_info.bfgn_wakeup_host);

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    /* wifi wake host gpio request */
    pdesc_wlan_wakeup_host = devm_gpiod_get_index_optional(&pdev->dev,
        NULL, WLAN_WAKEUP_HOST_GPIO_ACPI_INDEX, GPIOD_ASIS);
    if (pdesc_wlan_wakeup_host == NULL) {
        ps_print_err("%s pdesc_wlan_wakeup_host NULL!!!\n", __func__);
        goto err_get_wlan_wkup_host_gpio;
    }
    g_st_board_info.wlan_wakeup_host = desc_to_gpio(pdesc_wlan_wakeup_host);
    ps_print_info("from gpiod CRS[7] wlan_wakeup_host:%d\n", g_st_board_info.wlan_wakeup_host);

    /* host wake wlan gpio request */
    pdesc_host_wakeup_wlan = devm_gpiod_get_index_optional(&pdev->dev,
        NULL, HOST_WAKEUP_WLAN_GPIO_ACPI_INDEX, GPIOD_ASIS);
    if (pdesc_host_wakeup_wlan == NULL) {
        ps_print_err("%s pdesc_wlan_wakeup_host NULL!!!\n", __func__);
        goto err_get_host_wake_up_wlan_fail;
    }
    g_st_board_info.host_wakeup_wlan = desc_to_gpio(pdesc_host_wakeup_wlan);
    ps_print_info("from gpiod CRS[3] host_wakeup_wlan:%d\n", g_st_board_info.host_wakeup_wlan);
#endif

    return BOARD_SUCC;
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
err_get_host_wake_up_wlan_fail:
    devm_gpiod_put(&pdev->dev, pdesc_wlan_wakeup_host);

err_get_wlan_wkup_host_gpio:
    devm_gpiod_put(&pdev->dev, pdesc_bfgn_wakeup_host);
#endif
err_get_bfgx_wkup_host_gpio:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV, CHR_WIFI_DEV_EVENT_CHIP,
                         CHR_PLAT_DRV_ERROR_WAKEUP_GPIO);

    return BOARD_FAIL;
}

void hi1103_free_board_wakeup_gpio(struct platform_device *pdev)
{
    devm_gpiod_put(&pdev->dev, gpio_to_desc(g_st_board_info.bfgn_wakeup_host));

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    devm_gpiod_put(&pdev->dev, gpio_to_desc(g_st_board_info.wlan_wakeup_host));
    devm_gpiod_put(&pdev->dev, gpio_to_desc(g_st_board_info.host_wakeup_wlan));
#endif
}

int32_t hi1103_board_wifi_tas_gpio_init(void)
{
    return BOARD_SUCC;
}

void hi1103_free_board_wifi_tas_gpio(void)
{
    return;
}

void hi1103_wlan_wakeup_host_property_init(void)
{
    int32_t ret;
    int32_t cfg_val;

    ret = get_cust_conf_int32(INI_MODU_PLAT, INI_WLAN_WAKEUP_HOST_REVERSE, &cfg_val);
    if (ret == INI_FAILED) {
        ps_print_err("get %s failed\n", INI_WLAN_WAKEUP_HOST_REVERSE);
    }

    g_st_board_info.wlan_wakeup_host_have_reverser = cfg_val;
    ps_print_info("wlan_wakeup_host have reverser is or not: [%d]\n", cfg_val);
}
/*
 * 函 数 名  : hi1103_board_flowctrl_gpio_init
 * 功能描述  : 注册gpio流控中断
 */
int32_t hi1103_board_flowctrl_gpio_init(void)
{
    int32_t ret;
    int32_t physical_gpio = 0;

    /* 根据设备树DTS文件获取流控GPIO在host侧对应的管脚，保存在physical_gpio中 */
    ret = get_board_gpio(DTS_NODE_HI110X_WIFI, DTS_PROP_GPIO_WLAN_FLOWCTRL, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_err("get dts prop %s failed\n", DTS_PROP_GPIO_WLAN_FLOWCTRL);
        return BOARD_FAIL;
    }

    g_st_board_info.flowctrl_gpio = physical_gpio;

    /* 向内核申请使用该管脚 */
#ifdef GPIOF_IN
    ret = gpio_request_one(physical_gpio, GPIOF_IN, PROC_NAME_GPIO_WLAN_FLOWCTRL);
    if (ret) {
        ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_WLAN_FLOWCTRL);
        return BOARD_FAIL;
    }
#else
    ret = gpio_request(physical_gpio, PROC_NAME_GPIO_WLAN_FLOWCTRL);
    if (ret) {
        ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_WLAN_FLOWCTRL);
        return BOARD_FAIL;
    } else {
        ps_print_err("%s gpio_request succ\n", PROC_NAME_GPIO_WLAN_FLOWCTRL);
        gpio_direction_input(physical_gpio);
    }
#endif

    return BOARD_SUCC;
}

void hi1103_free_board_flowctrl_gpio(void)
{
    oal_gpio_free(g_st_board_info.flowctrl_gpio);
}

STATIC void hi110x_prepare_to_power_on(void)
{
    int32_t ret;
    hi110x_board_info *bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (bd_info->need_power_prepare == NO_NEED_POWER_PREPARE) {
        return;
    }

    ret = gpio_request_one(bd_info->tcxo_freq_detect, GPIOF_OUT_INIT_LOW, PROC_NAME_HI110X_GPIO_TCXO_FREQ);
    if (ret != BOARD_SUCC) {
        ps_print_err("%s gpio_request failed\n", PROC_NAME_HI110X_GPIO_TCXO_FREQ);
        return;
    }

    gpio_direction_output(bd_info->tcxo_freq_detect, bd_info->gpio_tcxo_detect_level);

    return;
}

STATIC void hi110x_post_to_power_on(void)
{
    hi110x_board_info *bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (bd_info->need_power_prepare == NO_NEED_POWER_PREPARE) {
        return;
    }

    gpio_free(bd_info->tcxo_freq_detect);

    return;
}

void hi1103_chip_power_on(void)
{
    int32_t ret;

    ps_print_info("chip_power on called by %pF\n", (void*)(uintptr_t)_RET_IP_);

    if (oal_gpio_get_value(g_st_board_info.bfgn_wakeup_host) == GPIO_HIGHLEVEL) {
        ps_print_err("bfg wakeup host gpio in wrong status\n");
    }

    ret = enable_board_pmu_clk32k();
    if (ret != BOARD_SUCC) {
        ps_print_err("hi1103 enable 32k clk fail.\n");
    }

    buck_power_ctrl(OAL_TRUE, PLAT_SUBSYS);

    board_host_wakeup_dev_set(0);
    hi110x_prepare_to_power_on();
    power_state_change(g_st_board_info.power_on_enable, BOARD_POWER_ON);
    hi110x_post_to_power_on();
}

void hi1103_chip_power_off(void)
{
    int32_t ret;

    ps_print_info("chip_power off called by %pF\n", (void*)(uintptr_t)_RET_IP_);

    power_state_change(g_st_board_info.power_on_enable, BOARD_POWER_OFF);

    buck_power_ctrl(OAL_FALSE, PLAT_SUBSYS);
    board_host_wakeup_dev_set(0);
    ret = disable_board_pmu_clk32k();
    if (ret != BOARD_SUCC) {
        ps_print_err("hi1103 disable 32k clk fail.\n");
    }
}

int32_t hi1103_wifi_enable(void)
{
    int32_t ret;
    uintptr_t gpio = g_st_board_info.wlan_power_on_enable;

    /* 第一次枚举时BUS 还未初始化 */
    ret = hcc_bus_power_ctrl_register(hcc_get_current_110x_bus(), HCC_BUS_CTRL_POWER_UP,
                                      board_wlan_gpio_power_on, (void *)gpio);
    if (ret) {
        ps_print_info("power ctrl down reg failed, ret=%d\n", ret);
    }
    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_UP);

    return ret;
}

int32_t hi1103_wifi_disable(void)
{
    int32_t ret;
    uintptr_t gpio = g_st_board_info.wlan_power_on_enable;

    ret = hcc_bus_power_ctrl_register(hcc_get_current_110x_bus(), HCC_BUS_CTRL_POWER_DOWN,
                                      board_wlan_gpio_power_off, (void *)gpio);
    if (ret) {
        ps_print_info("power ctrl down reg failed, ret=%d", ret);
    }
    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_DOWN);
    return ret;
}

void hi1103_bfgx_enable(void)
{
    power_state_change(g_st_board_info.bfgn_power_on_enable, BOARD_POWER_ON);
}

void hi1103_bfgx_disable(void)
{
    power_state_change(g_st_board_info.bfgn_power_on_enable, BOARD_POWER_OFF);
}

int32_t hi1103_board_power_on(uint32_t ul_subsystem)
{
    int32_t ret = SUCC;

    if (ul_subsystem == WLAN_POWER) {
        if (bfgx_is_shutdown()) {
            ps_print_info("wifi pull up power_on_enable gpio!\n");
            hi1103_chip_power_on();
            if (hi1103_wifi_enable()) {
                ret = WIFI_POWER_BFGX_OFF_PULL_WLEN_FAIL;
            }
            hi1103_bfgx_enable();
        } else {
            if (hi1103_wifi_enable()) {
                ret = WIFI_POWER_BFGX_ON_PULL_WLEN_FAIL;
            }
        }
    } else if (ul_subsystem == BFGX_POWER) {
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
        if (wlan_is_shutdown()) {
            ps_print_info("bfgx pull up power_on_enable gpio!\n");
            hi1103_chip_power_on();
            ret = hi1103_wifi_enable();
        }
#else
        hi1103_chip_power_on();
#endif
        hi1103_bfgx_enable();
    } else {
        ps_print_err("power input system:%d error\n", ul_subsystem);
    }

    return ret;
}

int32_t hi1103_board_power_off(uint32_t ul_subsystem)
{
    if (ul_subsystem == WLAN_POWER) {
        hi1103_wifi_disable();
        if (bfgx_is_shutdown()) {
            ps_print_info("wifi pull down power_on_enable!\n");
            hi1103_bfgx_disable();
            hi1103_chip_power_off();
        }
    } else if (ul_subsystem == BFGX_POWER) {
        hi1103_bfgx_disable();
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
        if (wlan_is_shutdown()) {
            ps_print_info("bfgx pull down power_on_enable!\n");
            hi1103_wifi_disable();
            hi1103_chip_power_off();
        }
#else
        hi1103_chip_power_off();
#endif
    } else {
        ps_print_err("power input system:%d error\n", ul_subsystem);
        return -EFAIL;
    }

    return SUCC;
}

int32_t hi1103_board_power_reset(uint32_t ul_subsystem)
{
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    int32_t ret;
#endif
    hi1103_bfgx_disable();
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hi1103_wifi_disable();
#endif
    hi1103_chip_power_off();
    hi1103_chip_power_on();
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    ret = hi1103_wifi_enable();
#endif
    hi1103_bfgx_enable();
    return ret;
}

void hi1103_bfgx_subsys_reset(void)
{
    // 维测, 判断之前的gpio状态
    ps_print_info("bfgx wkup host gpio val %d\n", board_get_bwkup_gpio_val());

    hi1103_bfgx_disable();
    mdelay(BFGX_SUBSYS_RST_DELAY);
    hi1103_bfgx_enable();
}

int32_t hi1103_wifi_subsys_reset(void)
{
    int32_t ret;
    hi1103_wifi_disable();
    mdelay(WIFI_SUBSYS_RST_DELAY);
    ret = hi1103_wifi_enable();
    return ret;
}

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
int32_t hi1103_bfgx_run_bcpu(void)
{
    int32_t ret;
    int32_t error = BFGX_POWER_SUCCESS;
    struct pm_top* pm_top_data = pm_get_top();

    if (wlan_is_shutdown()) {
        ret = firmware_download_function(BFGX_CFG);
        if (ret != BFGX_POWER_SUCCESS) {
            hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
            ps_print_err("bfgx download firmware fail!\n");
            error = (ret == -OAL_EINTR) ? BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT : BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL;
            return error;
        }
        hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);

        /* eng support monitor */
#ifdef PLATFORM_DEBUG_ENABLE
        if (!g_device_monitor_enable) {
#endif
            hi1103_wifi_disable();
#ifdef PLATFORM_DEBUG_ENABLE
        }
#endif
    } else {
        /* 此时BFGX 需要解复位BCPU */
        ps_print_info("wifi dereset bcpu\n");
        if (wlan_pm_open_bcpu() != BFGX_POWER_SUCCESS) {
            ps_print_err("wifi dereset bcpu fail!\n");
            error = BFGX_POWER_WIFI_DERESET_BCPU_FAIL;
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_BFGX_PWRON_BY_WIFI);

            return error;
        }
    }
    return error;
}
int32_t hi1103_bfgx_dev_power_on(void)
{
    int32_t ret;
    int32_t error = BFGX_POWER_SUCCESS;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_core_d = ps_get_core_reference(BUART);
    if ((ps_core_d == NULL) || (ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }

    pm_data = ps_core_d->pm_data;

    bfgx_gpio_intr_enable(pm_data, OAL_TRUE);

    ret = hi1103_board_power_on(BFGX_POWER);
    if (ret) {
        ps_print_err("hi1103_board_power_on bfg failed, ret=%d\n", ret);
        error = BFGX_POWER_PULL_POWER_GPIO_FAIL;
        goto bfgx_power_on_fail;
    }

    if (open_tty_drv(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("open tty fail!\n");
        error = BFGX_POWER_TTY_OPEN_FAIL;
        goto bfgx_power_on_fail;
    }

    error = hi1103_bfgx_run_bcpu();
    if (error != BFGX_POWER_SUCCESS) {
        goto bfgx_power_on_fail;
    }

    return BFGX_POWER_SUCCESS;

bfgx_power_on_fail:
#ifdef CONFIG_HUAWEI_DSM
    if (error != BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT) {
        hw_110x_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_DOWNLOAD_FIRMWARE,
                                  "bcpu download firmware failed,wifi %s,ret=%d,process:%s\n",
                                  wlan_is_shutdown() ? "off" : "on", error, current->comm);
    }
#endif
    hi1103_board_power_off(BFGX_POWER);
    return error;
}
#else
int32_t hi1103_bfgx_dev_power_on(void)
{
    int32_t ret;
    int32_t error = BFGX_POWER_SUCCESS;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_core_d = ps_get_core_reference(BUART);
    if ((ps_core_d == NULL) || (ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }

    pm_data = ps_core_d->pm_data;

    bfgx_gpio_intr_enable(pm_data, OAL_TRUE);

    ps_print_info("bfgx pull up power_on_enable gpio!\n");
    ret = hi1103_board_power_on(BFGX_POWER);
    if (ret) {
        ps_print_err("hi1103_board_power_on bfg failed ret=%d\n", ret);
        return BFGX_POWER_FAILED;
    }

    if (open_tty_drv(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("open tty fail!\n");
        error = BFGX_POWER_TTY_OPEN_FAIL;
        goto bfgx_power_on_fail;
    }

    st_tty_recv = ps_recv_patch;
    error = ps_core_d->pm_data->download_patch();
    if (error) { /* if download patch err,and close uart */
        error = release_tty_drv(ps_core_d);
        ps_print_err(" download_patch is failed!\n");
        goto bfgx_power_on_fail;
    }

    ps_print_suc(" download_patch is successfully!\n");

    return BFGX_POWER_SUCCESS;

bfgx_power_on_fail:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_POWER_UP_BFGX);

    hi1103_board_power_off(BFGX_POWER);
    return error;
}
#endif

int32_t hi1103_bfgx_dev_power_off(void)
{
    int32_t error = BFGX_POWER_SUCCESS;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_print_info("%s\n", __func__);

    ps_core_d = ps_get_core_reference(BUART);
    if ((ps_core_d == NULL) || (ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }

    if (uart_bfgx_close_cmd() != SUCCESS) {
        /* bfgx self close fail 了，后面也要通过wifi shutdown bcpu */
        ps_print_err("bfgx self close fail\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_BCPU);
    }

    pm_data = ps_core_d->pm_data;

    bfgx_gpio_intr_enable(pm_data, OAL_FALSE);

    if (release_tty_drv(ps_core_d) != SUCCESS) {
        /* 代码执行到此处，说明六合一所有业务都已经关闭，无论tty是否关闭成功，device都要下电 */
        ps_print_err("wifi off, close tty is err!");
    }

    if (!wlan_is_shutdown()) {
        ps_print_info("wifi shutdown bcpu\n");
        if (wlan_pm_shutdown_bcpu_cmd() != SUCCESS) {
            ps_print_err("wifi shutdown bcpu fail\n");
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_WIFI_CLOSE_BCPU);
            error = BFGX_POWER_FAILED;
        }
    }

    pm_data->bfgx_dev_state = BFGX_SLEEP;
    pm_data->uart_state = UART_NOT_READY;
    pm_data->uart_baud_switch_to = g_uart_default_baud_rate;
    ps_print_info("set BFGX_SLEEP,pm_data->uart_baud_switch_to baudrate %d\n", pm_data->uart_baud_switch_to);

    hi1103_board_power_off(BFGX_POWER);

    return error;
}

int32_t hi1103_wlan_power_off(void)
{
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    /* 先关闭SDIO TX通道 */
    hcc_bus_disable_state(hcc_get_current_110x_bus(), OAL_BUS_STATE_TX);

    /* wakeup dev,send poweroff cmd to wifi */
    if (wlan_pm_poweroff_cmd() != OAL_SUCC) {
        /* wifi self close 失败了也继续往下执行，uart关闭WCPU，异常恢复推迟到wifi下次open的时候执行 */
        declare_dft_trace_key_info("wlan_poweroff_cmd_fail", OAL_DFT_TRACE_FAIL);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_WCPU);
    }
    hcc_bus_disable_state(hcc_get_current_110x_bus(), OAL_BUS_STATE_ALL);

    hi1103_board_power_off(WLAN_POWER);

    if (wlan_pm_info != NULL) {
        wlan_pm_info->wlan_power_state = POWER_STATE_SHUTDOWN;
    }
    return SUCCESS;
}

static int32_t hi1103_firmware_download_syserr_to_selferr(int32_t error)
{
    int32_t ret = SUCCESS;

    if (error != WIFI_POWER_SUCCESS) {
        ps_print_err("firmware download fail\n");
        if (error == -OAL_EINTR) {
            ret = WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT;
        } else if (error == -OAL_ENOPERMI) {
            ret = WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL;
        } else {
            ret = WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL;
        }
    }
    return ret;
}
int32_t hi1103_wlan_power_on(void)
{
    int32_t ret;
    int32_t error = WIFI_POWER_SUCCESS;
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if (wlan_pm_info == NULL) {
        ps_print_err("wlan_pm_info is NULL!\n");
        return -FAILURE;
    }

    ret = hi1103_board_power_on(WLAN_POWER);
    if (ret) {
        ps_print_err("hi1103_board_power_on wlan failed ret=%d\n", ret);
        return -FAILURE;
    }

    hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    if (bfgx_is_shutdown()) {
        error = firmware_download_function(BFGX_AND_WIFI_CFG);
        hi1103_bfgx_disable();
    } else {
        error = firmware_download_function(WIFI_CFG);
    }

    if (error != WIFI_POWER_SUCCESS) {
        error = hi1103_firmware_download_syserr_to_selferr(error);
    } else {
        wlan_pm_info->wlan_power_state = POWER_STATE_OPEN;
    }

    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        declare_dft_trace_key_info("wlan_poweron HCC_BUS_POWER_PATCH_LAUCH by gpio_fail", OAL_DFT_TRACE_FAIL);
        ps_print_err("wlan_poweron HCC_BUS_POWER_PATCH_LAUCH by gpio fail ret=%d", ret);
        error = WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL;
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_WCPU_BOOTUP);
        goto wifi_power_fail;
    }

    return WIFI_POWER_SUCCESS;
wifi_power_fail:
#ifdef CONFIG_HUAWEI_DSM
    if (error != WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT && error != WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL) {
        hw_110x_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_1103_DOWNLOAD_FIRMWARE,
                                  "%s: failed to download firmware, bfgx %s, error=%d\n",
                                  __FUNCTION__, bfgx_is_shutdown() ? "off" : "on", error);
    }
#endif
    return error;
}

int32_t hi1103_get_board_pmu_clk32k(void)
{
    g_st_board_info.clk_32k_name = "clk_pmu32kb";
    ps_print_info("hi110x 32k clk name is %s\n", g_st_board_info.clk_32k_name);

    return BOARD_SUCC;
}

int32_t hi1103_get_board_uart_port(void)
{
    g_st_board_info.uart_port = "/dev/ttySC_SPI0";
    g_st_board_info.uart_pclk = UART_PCLK_NORMAL;
    ps_print_info("g_st_board_info.uart_port=%s\n", g_st_board_info.uart_port);
    return BOARD_SUCC;
}

#ifdef HAVE_HISI_IR
int32_t hi1103_board_ir_ctrl_gpio_init(void)
{
    int32_t ret;
    int32_t physical_gpio = 0;

    ret = get_board_gpio(DTS_NODE_HI110X_BFGX, DTS_PROP_GPIO_BFGX_IR_CTRL, &physical_gpio);
    if (ret != BOARD_SUCC) {
        ps_print_info("dts prop %s not exist\n", DTS_PROP_GPIO_BFGX_IR_CTRL);
        g_st_board_info.bfgx_ir_ctrl_gpio = -1;
    } else {
        g_st_board_info.bfgx_ir_ctrl_gpio = physical_gpio;

#ifdef GPIOF_OUT_INIT_LOW
        ret = gpio_request_one(physical_gpio, GPIOF_OUT_INIT_LOW, PROC_NAME_GPIO_BFGX_IR_CTRL);
        if (ret) {
            ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_BFGX_IR_CTRL);
        }
#else
        ret = gpio_request(physical_gpio, PROC_NAME_GPIO_BFGX_IR_CTRL);
        if (ret) {
            ps_print_err("%s gpio_request failed\n", PROC_NAME_GPIO_BFGX_IR_CTRL);
        } else {
            gpio_direction_output(physical_gpio, 0);
        }
#endif
    }

    return BOARD_SUCC;
}

int32_t hi1103_board_ir_ctrl_pmic_init(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret = BOARD_FAIL;
    struct device_node *np = NULL;
    int32_t irled_voltage = 0;
    if (pdev == NULL) {
        ps_print_err("board pmu pdev is NULL!\n");
        return ret;
    }

    ret = get_board_dts_node(&np, DTS_NODE_HI110X_BFGX);
    if (ret != BOARD_SUCC) {
        ps_print_err("DTS read node %s fail!!!\n", DTS_NODE_HI110X_BFGX);
        return ret;
    }

    g_st_board_info.bfgn_ir_ctrl_ldo = regulator_get(&pdev->dev, DTS_PROP_HI110X_IRLED_LDO_POWER);

    if (IS_ERR(g_st_board_info.bfgn_ir_ctrl_ldo)) {
        ps_print_err("board_ir_ctrl_pmic_init get ird ldo failed\n");
        return ret;
    }

    ret = of_property_read_u32(np, DTS_PROP_HI110X_IRLED_VOLTAGE, &irled_voltage);
    if (ret == BOARD_SUCC) {
        ps_print_info("set irled voltage %d mv\n", irled_voltage / VOLATAGE_V_TO_MV); /* V to mV */
        ret = regulator_set_voltage(g_st_board_info.bfgn_ir_ctrl_ldo, (int)irled_voltage, (int)irled_voltage);
        if (ret) {
            ps_print_err("board_ir_ctrl_pmic_init set voltage ldo failed\n");
            return ret;
        }
    } else {
        ps_print_err("get irled voltage failed ,use default\n");
    }

    ret = regulator_set_mode(g_st_board_info.bfgn_ir_ctrl_ldo, REGULATOR_MODE_NORMAL);
    if (ret) {
        ps_print_err("board_ir_ctrl_pmic_init set ldo mode failed\n");
        return ret;
    }
    ps_print_info("board_ir_ctrl_pmic_init success\n");
    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}
#endif

int32_t hi1103_board_ir_ctrl_init(struct platform_device *pdev)
{
    return BOARD_SUCC;
}

int32_t hi1103_check_evb_or_fpga(void)
{
    ps_print_info("HI1103 ASIC VERSION\n");
    g_st_board_info.is_asic = VERSION_ASIC;
    return BOARD_SUCC;
}

int32_t hi1103_check_hi110x_subsystem_support(void)
{
    g_st_board_info.is_wifi_disable = 0;
    g_st_board_info.is_bfgx_disable = 0;
    ps_print_info("wifi %s, bfgx %s\n",
                  (g_st_board_info.is_wifi_disable == 0) ? "enabled" : "disabled",
                  (g_st_board_info.is_bfgx_disable == 0) ? "enabled" : "disabled");
    return BOARD_SUCC;
}

int32_t hi1103_check_pmu_clk_share(void)
{
    g_st_board_info.pmu_clk_share_enable = PMU_CLK_REQ_DISABLE;
    ps_print_info("hi110x PMU clk request is %s\n", (g_st_board_info.pmu_clk_share_enable ? "enable" : "disable"));
    return BOARD_SUCC;
}

int32_t hi1103_board_get_power_pinctrl(void)
{
    ps_print_info("no need prepare before board power on\n");
    g_st_board_info.need_power_prepare = NO_NEED_POWER_PREPARE;
    return BOARD_SUCC;
}

int32_t hi1103_get_ini_file_name_from_dts(char *dts_prop, char *prop_value, uint32_t size)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32_t ret;
    struct device_node *np = NULL;
    int32_t len;
    char out_str[HISI_CUST_NVRAM_LEN] = { 0 };

    np = of_find_compatible_node(NULL, NULL, COST_HI110X_COMP_NODE);
    if (np == NULL) {
        ini_error("dts node %s not found", COST_HI110X_COMP_NODE);
        return INI_FAILED;
    }

    len = of_property_count_u8_elems(np, dts_prop);
    if (len < 0) {
        ini_error("can't get len of dts prop(%s)", dts_prop);
        return INI_FAILED;
    }

    len = ini_min(len, (int32_t)sizeof(out_str));
    ini_debug("read len of dts prop %s is:%d", dts_prop, len);
    ret = of_property_read_u8_array(np, dts_prop, out_str, len);
    if (ret < 0) {
        ini_error("read dts prop (%s) fail", dts_prop);
        return INI_FAILED;
    }

    len = ini_min(len, (int32_t)size);
    if (memcpy_s(prop_value, (size_t)size, out_str, (size_t)len) != EOK) {
        ini_error("memcpy_s error, destlen=%d, srclen=%d\n ", size, len);
        return INI_FAILED;
    }
    ini_debug("dts prop %s value is:%s", dts_prop, prop_value);
#endif
    return INI_SUCC;
}

/* factory test, wifi power on, do some test under bootloader mode */
void hi1103_dump_gpio_regs(void)
{
    uint16_t value;
    int32_t ret;
    value = 0;
    ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, &value);
    if (ret) {
        return;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "gpio reg 0x%x = 0x%x", GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, value);

    value = 0;
    ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, &value);
    if (ret) {
        return;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "gpio reg 0x%x = 0x%x", GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, value);
}

int32_t hi1103_check_device_ready(void)
{
    uint16_t value;
    int32_t ret;

    value = 0;
    ret = read_device_reg16(CHECK_DEVICE_RDY_ADDR, &value);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "read 0x%x reg failed, ret=%d", CHECK_DEVICE_RDY_ADDR, ret);
        return -OAL_EFAIL;
    }

    /* 读到0x101表示成功 */
    if (value != 0x101) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "device sysctrl reg error, value=0x%x", value);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

int32_t hi1103_check_wlan_wakeup_host_config(void)
{
    uint16_t value;
    int32_t ret;

    value = 0;
    ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, &value);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "read 0x%x reg failed, ret=%d",
                             (GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR), ret);
        return -1;
    }

    /* * 输出 */
    value |= (WLAN_DEV2HOST_GPIO);

    ret = write_device_reg16(GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, value);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d",
                             (GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR), value, ret);
        return -1;
    }
    return OAL_SUCC;
}

int32_t hi1103_check_wlan_wakeup_host(void)
{
    int32_t i;
    int32_t ret;
    const uint32_t test_times = 2;

    if (g_st_board_info.wlan_wakeup_host == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan_wakeup_host gpio is zero!");
        return -OAL_EIO;
    }

    ret = hi1103_check_wlan_wakeup_host_config();
    if (ret) {
        return ret;
    }

    for (i = 0; i < test_times; i++) {
        ret = write_device_reg16(GPIO_BASE_ADDR + GPIO_LEVEL_CONFIG_REGADDR, WLAN_DEV2HOST_GPIO);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d",
                                 (GPIO_BASE_ADDR + GPIO_LEVEL_CONFIG_REGADDR), WLAN_DEV2HOST_GPIO, ret);
            return -1;
        }

        oal_msleep(1);

        if (oal_gpio_get_value(g_st_board_info.wlan_wakeup_host) == 0) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pull gpio high failed!");
            hi1103_dump_gpio_regs();
            return -OAL_EFAIL;
        }

        ret = write_device_reg16(GPIO_BASE_ADDR + GPIO_LEVEL_CONFIG_REGADDR, 0x0);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d",
                                 (GPIO_BASE_ADDR + GPIO_LEVEL_CONFIG_REGADDR), WLAN_DEV2HOST_GPIO, ret);
            return -1;
        }

        oal_msleep(1);

        if (oal_gpio_get_value(g_st_board_info.wlan_wakeup_host) != 0) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pull gpio low failed!");
            hi1103_dump_gpio_regs();
            return -OAL_EFAIL;
        }

        oal_print_hi11xx_log(HI11XX_LOG_INFO, "check d2h wakeup io %d times ok", i + 1);
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "check d2h wakeup io done");
    return OAL_SUCC;
}

int32_t hi1103_check_host_wakeup_wlan_direction(void)
{
    uint16_t value;
    int32_t ret;

    value = 0;
    ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, &value);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "read 0x%x reg failed, ret=%d",
                             (GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR), ret);
        return -1;
    }

    /* * 输入 */
    value &= (~WLAN_HOST2DEV_GPIO);

    ret = write_device_reg16(GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR, value);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "write 0x%x reg failed,value=0x%x, ret=%d",
                             (GPIO_BASE_ADDR + GPIO_INOUT_CONFIG_REGADDR), value, ret);
        return -1;
    }
    return ret;
}

int32_t hi1103_check_host_wakeup_wlan_value(void)
{
    int32_t i, ret;
    uint16_t value;
    const uint32_t test_times = 2;

    for (i = 0; i < test_times; i++) {
        gpio_direction_output(g_st_board_info.host_wakeup_wlan, GPIO_HIGHLEVEL);
        oal_msleep(1);

        value = 0;
        ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, &value);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "read 0x%x reg failed, ret=%d",
                (GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR), ret);
            return -1;
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "read 0x%x reg=0x%x", (GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR), value);

        value &= WLAN_HOST2DEV_GPIO;

        if (value == 0) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pull gpio high failed!");
            hi1103_dump_gpio_regs();
            return -OAL_EFAIL;
        }

        gpio_direction_output(g_st_board_info.host_wakeup_wlan, GPIO_LOWLEVEL);
        oal_msleep(1);

        value = 0;
        ret = read_device_reg16(GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR, &value);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "read 0x%x reg failed, ret=%d",
                (GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR), ret);
            return -1;
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "read 0x%x reg=0x%x", (GPIO_BASE_ADDR + GPIO_LEVEL_GET_REGADDR), value);

        value &= WLAN_HOST2DEV_GPIO;

        if (value != 0) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pull gpio low failed!");
            hi1103_dump_gpio_regs();
            return -OAL_EFAIL;
        }

        oal_print_hi11xx_log(HI11XX_LOG_INFO, "check h2d wakeup io %d times ok", i + 1);
    }
    return 0;
}

int32_t hi1103_check_host_wakeup_wlan(void)
{
    int32_t ret;

    if (g_st_board_info.host_wakeup_wlan == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "host_wakeup_wlan gpio is zero!");
        return -OAL_EIO;
    }

    ret = hi1103_check_host_wakeup_wlan_direction();
    if (ret) {
        return ret;
    }

    ret = hi1103_check_host_wakeup_wlan_value();
    if (ret) {
        return ret;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "check h2d wakeup io done");
    return 0;
}

int32_t hi1103_check_wlan_gpio(void)
{
    int32_t ret;
    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();

    /* power on wifi, need't download firmware */
    ret = hi1103_board_power_on(WLAN_POWER);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "power on wlan failed=%d", ret);
        goto IO_CHECK_FAIL;
    }

    hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    ret = hcc_bus_reinit(wlan_pm_info->pst_bus);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "reinit bus %d failed, ret=%d", wlan_pm_info->pst_bus->bus_type, ret);
        goto IO_CHECK_FAIL;
    }

    wlan_pm_init_dev();
    ret = hi1103_check_device_ready();
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "check_device_ready failed, ret=%d", ret);
        goto IO_CHECK_FAIL;
    }

    /* check io */
    ret = hi1103_check_host_wakeup_wlan();
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "check_host_wakeup_wlan failed, ret=%d", ret);
        goto IO_CHECK_FAIL;
    }

    hi1103_board_power_off(WLAN_POWER);
    return OAL_SUCC;
IO_CHECK_FAIL:
    if (!g_ft_fail_powerdown_bypass) {
        hi1103_board_power_off(WLAN_POWER);
    }
    hcc_bus_and_wake_unlock(wlan_pm_info->pst_bus);
    return ret;
}

int32_t hi1103_dev_io_test(void)
{
    int32_t ret;
    declare_time_cost_stru(cost);

    struct wlan_pm_s *wlan_pm_info = wlan_pm_get_drv();
    if (wlan_pm_info == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan_pm_info is NULL!");
        return -FAILURE;
    }

    if (!bfgx_is_shutdown()) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bfgx is open, test abort!");
        bfgx_print_subsys_state();
        return -OAL_ENODEV;
    }

    if (!wlan_is_shutdown()) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan is open, test abort!");
        return -OAL_ENODEV;
    }

    oal_get_time_cost_start(cost);

    hcc_bus_and_wake_lock(wlan_pm_info->pst_bus);

    /* check io */
    ret = hi1103_check_wlan_gpio();
    if (ret) {
        return ret;
    }

    hcc_bus_and_wake_unlock(wlan_pm_info->pst_bus);

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hi1103 device io test cost %llu us", time_cost_var_sub(cost));
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
OAL_STATIC int32_t pcie_ip_test_pre(hcc_bus **old_bus)
{
    if (!bfgx_is_shutdown()) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bfgx is open, test abort!");
        bfgx_print_subsys_state();
        return -OAL_ENODEV;
    }

    if (!wlan_is_shutdown()) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan is open, test abort!");
        return -OAL_ENODEV;
    }

    *old_bus = hcc_get_current_110x_bus();
    if (*old_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi110x_bus is null, test abort!");
        return -OAL_ENODEV;
    }

    return OAL_SUCC;
}

int32_t hi1103_pcie_ip_test_core(int32_t test_count)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;

    pst_bus = hcc_get_current_110x_bus();

    hcc_bus_and_wake_lock(pst_bus);
    /* power on wifi, need't download firmware */
    ret = hi1103_board_power_on(WLAN_POWER);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "power on wlan failed=%d", ret);
        hcc_bus_and_wake_unlock(pst_bus);
        return ret;
    }

    hcc_bus_power_action(pst_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    ret = hcc_bus_reinit(pst_bus);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "reinit bus %s failed, ret=%d", pst_bus->name, ret);
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        goto PCIE_IP_TEST_FAIL;
    }

    wlan_pm_init_dev();

    ret = hi1103_check_device_ready();
    if (ret) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "check_device_ready failed, ret=%d", ret);
        goto PCIE_IP_TEST_FAIL;
    }

    ret = oal_pcie_ip_factory_test(pst_bus, test_count);
    if (ret) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie_ip_factory_test failed, ret=%d", ret);
        goto PCIE_IP_TEST_FAIL;
    }

    hi1103_board_power_off(WLAN_POWER);
    hcc_bus_and_wake_unlock(pst_bus);
    return OAL_SUCC;
PCIE_IP_TEST_FAIL:
    if (!g_ft_fail_powerdown_bypass) {
        hi1103_board_power_off(WLAN_POWER);
    }
    hcc_bus_and_wake_unlock(pst_bus);
    return ret;
}

int32_t hi1103_pcie_ip_test(int32_t test_count)
{
    int32_t ret;
    hcc_bus *old_bus = NULL;
    declare_time_cost_stru(cost);

    if (oal_pcie_110x_working_check() != OAL_TRUE) {
        /* 不支持PCIe,直接返回成功 */
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "do not support PCIe!");
        return OAL_SUCC;
    }

    ret = pcie_ip_test_pre(&old_bus);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    if (old_bus->bus_type != HCC_BUS_PCIE) {
        /* 尝试切换到PCIE */
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_PCIE);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "switch to PCIe ok.");
    } else {
        old_bus = NULL;
    }

    oal_get_time_cost_start(cost);

    ret = hi1103_pcie_ip_test_core(test_count);
    if (ret) {
        return ret;
    }

    if (old_bus != NULL) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, old_bus->bus_type);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "restore to bus %s failed, ret=%d", old_bus->name, ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "resotre to bus %s ok.", old_bus->name);
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hi1103 pcie ip test %llu us", time_cost_var_sub(cost));

    return OAL_SUCC;
}

EXPORT_SYMBOL(hi1103_pcie_ip_test);
EXPORT_SYMBOL(hi1103_dev_io_test);
OAL_STATIC uint32_t g_slt_pcie_status = 0;
OAL_STATIC hcc_bus *g_slt_old_bus = NULL;
/* for kirin slt test */
int32_t hi1103_pcie_chip_poweron_core(void)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;
    pst_bus = hcc_get_current_110x_bus();

    hcc_bus_and_wake_lock(pst_bus);
    /* power on wifi, need't download firmware */
    ret = hi1103_board_power_on(WLAN_POWER);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "power on wlan failed=%d", ret);
        hcc_bus_and_wake_unlock(pst_bus);
        return ret;
    }

    hcc_bus_power_action(pst_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE);
    ret = hcc_bus_reinit(pst_bus);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "reinit bus %d failed, ret=%d", pst_bus->bus_type, ret);
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        goto PCIE_CHIP_POWERON_CORE_FAIL;
    }

    wlan_pm_init_dev();

    ret = hi1103_check_device_ready();
    if (ret) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "check_device_ready failed, ret=%d", ret);
        goto PCIE_CHIP_POWERON_CORE_FAIL;
    }

    ret = oal_pcie_ip_init(pst_bus);
    if (ret) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG);
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_pcie_ip_init failed, ret=%d", ret);
        goto PCIE_CHIP_POWERON_CORE_FAIL;
    }

    ret = oal_pcie_ip_voltage_bias_init(pst_bus);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "voltage_bias_init failed, ret=%d", ret);
        goto PCIE_CHIP_POWERON_CORE_FAIL;
    }
    return OAL_SUCC;
PCIE_CHIP_POWERON_CORE_FAIL:
    if (!g_ft_fail_powerdown_bypass) {
        hi1103_board_power_off(WLAN_POWER);
    }
    hcc_bus_and_wake_unlock(pst_bus);
    return ret;
}

int32_t hi110x_pcie_chip_poweron(void *data)
{
    int32_t ret;
    hcc_bus *old_bus = NULL;

    if (oal_pcie_110x_working_check() != OAL_TRUE) {
        /* 不支持PCIe,直接返回成功 */
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "do not support PCIe!");
        return -OAL_ENODEV;
    }

    ret = pcie_ip_test_pre(&old_bus);
    if (ret != OAL_SUCC) {
        return -OAL_EFAIL;
    }

    if (old_bus->bus_type != HCC_BUS_PCIE) {
        /* 尝试切换到PCIE */
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, HCC_BUS_PCIE);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "switch to PCIe failed, ret=%d", ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "switch to PCIe ok.");
    } else {
        old_bus = NULL;
    }

    g_slt_old_bus = old_bus;

    ret = hi1103_pcie_chip_poweron_core();
    if (ret) {
        return ret;
    }

    g_slt_pcie_status = 1;

    return 0;
}

int32_t hi110x_pcie_chip_poweroff(void *data)
{
    int32_t ret;
    hcc_bus *pst_bus;

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can't find any wifi bus");
        return -OAL_ENODEV;
    }

    if (pst_bus->bus_type != HCC_BUS_PCIE) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "current bus is %s , not PCIe", pst_bus->name);
        return -OAL_ENODEV;
    }

    if (g_slt_pcie_status != 1) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pcie slt is not power on");
        return -OAL_EBUSY;
    }

    g_slt_pcie_status = 0;

    /* SLT下电之前打印链路信息 */
    hcc_bus_chip_info(pst_bus, OAL_FALSE, OAL_TRUE);

    hi1103_board_power_off(WLAN_POWER);

    hcc_bus_and_wake_unlock(pst_bus);

    if (g_slt_old_bus != NULL) {
        ret = hcc_switch_bus(HCC_CHIP_110X_DEV, g_slt_old_bus->bus_type);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "restore to bus %s failed, ret=%d", g_slt_old_bus->name, ret);
            return -OAL_ENODEV;
        }
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "resotre to bus %s ok.", g_slt_old_bus->name);
    }

    return 0;
}

int32_t hi110x_pcie_chip_transfer(void *ddr_address, uint32_t data_size, uint32_t direction)
{
    hcc_bus *pst_bus;

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can't find any wifi bus");
        return -OAL_ENODEV;
    }

    if (pst_bus->bus_type != HCC_BUS_PCIE) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "current bus is %s , not PCIe", pst_bus->name);
        return -OAL_ENODEV;
    }

    return oal_pcie_rc_slt_chip_transfer(pst_bus, ddr_address, data_size, (int32_t)direction);
}
EXPORT_SYMBOL(hi110x_pcie_chip_poweron);
EXPORT_SYMBOL(hi110x_pcie_chip_transfer);
EXPORT_SYMBOL(hi110x_pcie_chip_poweroff);
#endif

#if defined(CONFIG_PCIE_KIRIN_SLT_HI110X) && defined(CONFIG_HISI_DEBUG_FS)
int pcie_slt_hook_register(u32 rc_id, u32 device_type, int (*init)(void *), int (*on)(void *),
    int (*off)(void *), int (*setup)(void *), int (*data_transfer)(void *, u32, u32));
int32_t hi1103_pcie_chip_init(void *data)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "slt pcie init");
    return 0;
}

int32_t hi1103_pcie_chip_rc_slt_register(void)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    return pcie_slt_hook_register(g_kirin_rc_idx, 0x2,
                                  hi1103_pcie_chip_init, hi110x_pcie_chip_poweron,
                                  hi110x_pcie_chip_poweroff, hi1103_pcie_chip_init,
                                  hi110x_pcie_chip_transfer);
#else
    return 0;
#endif
}

int32_t hi1103_pcie_chip_rc_slt_unregister(void)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    return pcie_slt_hook_register(g_kirin_rc_idx, 0x2,
                                  NULL, NULL,
                                  NULL, NULL,
                                  NULL);
#else
    return 0;
#endif
}
EXPORT_SYMBOL(hi1103_pcie_chip_rc_slt_register);
EXPORT_SYMBOL(hi1103_pcie_chip_rc_slt_unregister);
#endif

EXPORT_SYMBOL(hi1103_chip_power_on);
EXPORT_SYMBOL(hi1103_chip_power_off);
EXPORT_SYMBOL(hi1103_bfgx_disable);
EXPORT_SYMBOL(hi1103_bfgx_enable);
