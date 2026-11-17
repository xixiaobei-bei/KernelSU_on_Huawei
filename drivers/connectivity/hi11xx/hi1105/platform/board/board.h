

#ifndef __BOARD_H__
#define __BOARD_H__

/* 其他头文件包含 */
#include <linux/regulator/machine.h>

#include "plat_type.h"
#include "hi_drv_gpio.h"
#include "board_hi1103.h"
#include "hisi_ini.h"
#include "ssi_common.h"

#ifdef CONFIG_HUAWEI_DSM
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#include <linux/huawei/dsm/dsm_pub.h>
#else
#include <dsm/dsm_pub.h>
#endif

#define DSM_1103_TCXO_ERROR        909030001
#define DSM_1103_DOWNLOAD_FIRMWARE 909030033
#define DSM_BUCK_PROTECTED         909030034
#define DSM_1103_HALT              909030035
#define DSM_WIFI_FEMERROR          909030036
#define DSM_PCIE_SWITCH_SDIO_FAIL  909030043
#define DSM_PCIE_SWITCH_SDIO_SUCC  909030044
#define DSM_BT_FEMERROR            913002007

extern void hw_110x_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...);
#endif

/* 宏定义 */
#define BOARD_SUCC 0
#define BOARD_FAIL (-1)

#define VERSION_FPGA 0
#define VERSION_ASIC 1

#define WIFI_TAS_DISABLE 0
#define WIFI_TAS_ENABLE  1

#define PMU_CLK_REQ_DISABLE 0
#define PMU_CLK_REQ_ENABLE  1

#define GPIO_LOWLEVEL  0
#define GPIO_HIGHLEVEL 1

#define NO_NEED_POWER_PREPARE 0
#define NEED_POWER_PREPARE    1

#define HI11XX_SUBCHIP_NAME_LEN_MAX 128

/* hi110x */
#define BOARD_VERSION_NAME_HI1102  "hi1102"
#define BOARD_VERSION_NAME_HI1102A "hi1102a"
#define BOARD_VERSION_NAME_HI1103  "hi1103"
#define BOARD_VERSION_NAME_HI1105  "hi1105"
#define BOARD_VERSION_NAME_HI1106  "hi1106"
#define BOARD_VERSION_NAME_HI113K  "hi1131k"

#define DTS_NODE_HISI_HI110X            "hisilicon,hi110x"
#define DTS_COMP_HISI_HI110X_BOARD_NAME DTS_NODE_HISI_HI110X

#define DTS_PROP_HI110X_VERSION "hi110x,asic_version"
#define DTS_PROP_HI110X_PMU_CLK "hi110x,pmu_clk_req"

#define DTS_PROP_HI110X_WIFI_DISABLE "hi110x,wifi_disable"
#define DTS_PROP_HI110X_BFGX_DISABLE "hi110x,bfgx_disable"

#define DTS_PROP_SUBCHIP_TYPE_VERSION "hi110x,subchip_type"

#define DTS_PROP_CLK_32K "huawei,pmu_clk32b"

#define DTS_PROP_HI110X_POWER_PREPARE "hi110x,power_prepare"
#define DTS_PROP_GPIO_TCXO_LEVEL      "hi110x,tcxo_gpio_level"

#define DTS_PROP_HI110X_GPIO_TCXO_FREQ_DETECT  "hi110x,gpio_tcxo_freq_detect"
#define PROC_NAME_HI110X_GPIO_TCXO_FREQ "hi110x_tcxo_freq_detect"

#define DTS_PROP_HI110X_BUCK_MODE   "hi110x,buck_mode"

/* power on */
#define DTS_PROP_GPIO_HI110X_POWEN_ON "hi110x,gpio_power_on"
#define PROC_NAME_GPIO_POWEN_ON       "power_on_enable"

/* gpio-ssi */
#define DTS_PROP_GPIO_HI110X_GPIO_SSI_CLK  "hi110x,gpio_ssi_clk"
#define DTS_PROP_GPIO_HI110X_GPIO_SSI_DATA "hi110x,gpio_ssi_data"

#define DTS_PROP_GPIO_HI110X_PCIE0_RST  "hi110x,gpio_pcie0_rst"
#define DTS_PROP_GPIO_HI110X_PCIE1_RST  "hi110x,gpio_pcie1_rst"

#define DTS_PROP_GPIO_WLAN_POWEN_ON_ENABLE "hi110x,gpio_wlan_power_on"
#define PROC_NAME_GPIO_WLAN_POWEN_ON       "wlan_power_on_enable"

#define DTS_PROP_GPIO_BFGX_POWEN_ON_ENABLE "hi110x,gpio_bfgx_power_on"
#define PROC_NAME_GPIO_BFGX_POWEN_ON       "bfgx_power_on_enable"

/* hisi_bfgx */
#define DTS_NODE_HI110X_BFGX            "hisilicon,hisi_bfgx"
#define PROC_NAME_GPIO_BFGX_WAKEUP_HOST "bfgx_wake_host"
#define DTS_PROP_GPIO_BFGX_IR_CTRL      "hi110x,gpio_bfgx_ir_ctrl"
#define PROC_NAME_GPIO_BFGX_IR_CTRL     "bfgx_ir_ctrl"
#define DTS_PROP_HI110X_IR_LDO_TYPE     "hi110x,irled_power_type"
#define DTS_PROP_HI110X_IRLED_LDO_POWER "hi110x,irled_power"
#define DTS_PROP_HI110X_IRLED_VOLTAGE   "hi110x,irled_voltage"

#define DTS_PROP_HI110X_GPIO_BFGX_WAKEUP_HOST "hi110x,gpio_bfgx_wakeup_host"
#define DTS_PROP_HI110X_GPIO_HOST_WAKEUP_BFGX "hi110x,gpio_host_wakeup_bfgx"
#define DTS_PROP_HI110X_UART_POART            "hi110x,uart_port"
#define DTS_PROP_HI110X_UART_PCLK             "hi110x,uart_pclk_normal"

/* hisi_me */
#define DTS_NODE_HI110X_ME            "hisilicon,hisi_me"
#define PROC_NAME_GPIO_GCPU_WAKEUP_HOST "gcpu_wake_host"

#define DTS_PROP_HI110X_GPIO_GCPU_WAKEUP_HOST "hi110x,gpio_gcpu_wakeup_host"
#define DTS_PROP_HI110X_GUART_POART            "hi110x,me_uart_port"

/* hisi_wifi */
#define DTS_NODE_HI110X_WIFI                  "hisilicon,hisi_wifi"
#define PROC_NAME_GPIO_WLAN_WAKEUP_HOST       "wlan_wake_host"
#define DTS_PROP_HI110X_GPIO_WLAN_WAKEUP_HOST "hi110x,gpio_wlan_wakeup_host"
#define DTS_PROP_GPIO_WLAN_FLOWCTRL           "hi110x,gpio_wlan_flow_ctrl"

#define DTS_PROP_HI110X_PCIE_RC_IDX      "hi110x,pcie_rc_idx"
#define DTS_PROP_HI110X_HOST_GPIO_SAMPLE "hi110x,gpio_sample_low"
#define DTS_PROP_HI110X_WIFI_TAS_STATE "hi110x,gpio_wifi_tas_state"

#define PROC_NAME_GPIO_HOST_WAKEUP_WLAN "host_wakeup_wlan"
#define DTS_PROP_GPIO_HOST_WAKEUP_WLAN  "hi110x,gpio_host_wakeup_wlan"

#define PROC_NAME_GPIO_HOST_WAKEUP_BFG "host_wakeup_bfg"

#define DTS_PROP_WIFI_TAS_EN    "hi110x,wifi_tas_enable"
#define PROC_NAME_GPIO_WIFI_TAS "wifi_tas"
#define DTS_PROP_GPIO_WIFI_TAS  "hi110x,gpio_wifi_tas"

/* hisi_cust_cfg */
#define COST_HI110X_COMP_NODE   "hi110x,customize"
#define PROC_NAME_INI_FILE_NAME "ini_file_name"

#define DTS_NODE_HISI_CCIBYPASS "hisi,ccibypass"

#ifdef _PRE_SUSPORT_OEMINFO
#define DTS_WIFI_CALIBRATE_IN_OEMINFO "hi110x,wifi_calibrate_in_oeminfo"
#endif

/* ini cfg */
#define INI_WLAN_DOWNLOAD_CHANNEL "board_info.wlan_download_channel"
#define INI_BFGX_DOWNLOAD_CHANNEL "board_info.bfgx_download_channel"

#define DOWNLOAD_MODE_SDIO "sdio"
#define DOWNLOAD_MODE_PCIE "pcie"
#define DOWNLOAD_MODE_UART "uart"

#define BOARD_VERSION_LEN    128
#define DOWNLOAD_CHANNEL_LEN 64

#define INI_SSI_DUMP_EN "ssi_dump_enable"
#define VOLATAGE_V_TO_MV 1000
/* STRUCT 定义 */
typedef struct bd_init_s {
    int32_t (*get_board_power_gpio)(struct platform_device *pdev);
    void (*free_board_power_gpio)(struct platform_device *pdev);
    void (*free_board_flowctrl_gpio)(void);
    int32_t (*board_wakeup_gpio_init)(struct platform_device *pdev);
    int32_t (*board_flowctrl_gpio_init)(void);
    void (*free_board_wakeup_gpio)(struct platform_device *pdev);
    int32_t (*board_wifi_tas_gpio_init)(void);
    void (*free_board_wifi_tas_gpio)(void);
    int32_t (*bfgx_dev_power_on)(void);
    int32_t (*bfgx_dev_power_off)(void);
    int32_t (*wlan_power_off)(void);
    int32_t (*wlan_power_on)(void);
    int32_t (*board_power_on)(uint32_t ul_subsystem);
    int32_t (*board_power_off)(uint32_t ul_subsystem);
    int32_t (*board_power_reset)(uint32_t ul_subsystem);
    int32_t (*get_board_pmu_clk32k)(void);
    int32_t (*get_board_uart_port)(void);
    int32_t (*board_ir_ctrl_init)(struct platform_device *pdev);
    int32_t (*check_evb_or_fpga)(void);
    int32_t (*check_hi110x_subsystem_support)(void);
    int32_t (*check_pmu_clk_share)(void);
    int32_t (*board_get_power_pinctrl)(void);
    int32_t (*get_ini_file_name_from_dts)(char *dts_prop, char *prop_value, uint32_t size);
#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
    void (*suspend_board_gpio_in_s4)(void);
    void (*resume_board_gpio_in_s4)(void);
#endif
#ifdef CONFIG_HWCONNECTIVITY_PC
    void (*wlan_wakeup_host_property_init)(void);
#endif
} bd_init_t;

typedef struct _gpio_ssi_ops_ {
    int32_t (*clk_switch)(int32_t ssi_clk);
    int32_t (*aon_reset)(void);
} gpio_ssi_ops;

/* private data for pm driver */
typedef struct {
    /* board init ops */
    struct bd_init_s bd_ops;

    gpio_ssi_ops ssi_ops;

    /* power */
    int32_t power_on_enable;
    int32_t bfgn_power_on_enable; /* 1103 product */
    int32_t wlan_power_on_enable; /* 1103 product */

    /* wakeup gpio */
    int32_t wlan_wakeup_host;
#ifdef CONFIG_HWCONNECTIVITY_PC
    int32_t wlan_wakeup_host_have_reverser;
#endif
    int32_t bfgn_wakeup_host;
    int32_t gcpu_wakeup_host; /* 1106 product */
    int32_t host_wakeup_wlan; /* 1103 product */
    int32_t host_wakeup_bfg;
    int32_t ep_pcie0_rst; /* device pcie0 rst pin */
    int32_t ep_pcie1_rst; /* device pcie1 rst pin */

    /* flowctrl gpio */
    int32_t flowctrl_gpio;

    int32_t rf_wifi_tas; /* 1103 product */

    int32_t ssi_gpio_clk;  /* gpio-ssi-clk */
    int32_t ssi_gpio_data; /* gpio-ssi-data */

    /* device hisi board verision */
    const char *chip_type;
    int32_t chip_nr;

    /* how to download firmware */
    int32_t wlan_download_channel;
    int32_t bfgn_download_channel;

#ifdef HAVE_HISI_IR
    bool have_ir;
    int32_t irled_power_type;
    int32_t bfgx_ir_ctrl_gpio;
    struct regulator *bfgn_ir_ctrl_ldo;
#endif

    /* hi110x irq info */
    uint32_t wlan_irq;
    uint32_t flowctrl_irq;
    uint32_t bfgx_irq;
    uint32_t gcpu_irq; /* 1106 product */

    /* hi110x uart info */
    const char *uart_port;
    const char *me_uart_port; /* 1106 product */
    int32_t uart_pclk;

    /* hi110x clk info */
    const char *clk_32k_name;
    struct clk *clk_32k;

    /* evb or fpga verison */
    int32_t is_asic;

    int32_t is_emu; // low freq emu platform

    int32_t is_wifi_disable;
    int32_t is_bfgx_disable;

    int32_t wifi_tas_enable;

    int32_t pmu_clk_share_enable;

    /* buck mode param ,get from ini */
    uint16_t buck_param;

    /* WIFI/BT/GNSS/FM/IR各子系统对buck的控制flag，bitmap控制 */
    uint16_t buck_control_flag;

    /* ini cfg */
    char *ini_file_name;

    /* prepare before board power on */
    int32_t need_power_prepare;
    int32_t tcxo_freq_detect;
    int32_t gpio_tcxo_detect_level;
} hi110x_board_info;

typedef struct _device_vesion_board {
    uint32_t index;
    const char name[BOARD_VERSION_LEN + 1];
} device_board_version;

typedef struct _download_mode {
    uint32_t index;
    uint8_t name[DOWNLOAD_CHANNEL_LEN + 1];
} download_mode;


enum hisi_device_board {
    /* HI110X 项目使用 0 ~ 999 */
    BOARD_VERSION_HI1102 = 0,
    BOARD_VERSION_HI1103 = 1,
    BOARD_VERSION_HI1102A = 2,
    BOARD_VERSION_HI1105 = 3,
    BOARD_VERSION_HI1106 = 4,
    BOARD_VERSION_BISHENG = 5,

    /* HI113X 项目使用 1000 ~ 2000 */
    BOARD_VERSION_HI1131K = 1000,
};

enum hisi_download_firmware_mode {
    MODE_SDIO = 0x0,
    MODE_PCIE = 0x1,
    MODE_UART = 0x2,
    MODE_DOWNLOAD_BUTT,
};
enum hisi_board_power {
    WLAN_POWER = 0x0,
    BFGX_POWER = 0x1,
    POWER_BUTT,
};
enum board_power_state {
    BOARD_POWER_OFF = 0x0,
    BOARD_POWER_ON = 0x1,
};

enum board_irled_power_type {
    IR_GPIO_CTRL = 0x0,
    IR_LDO_CTRL = 0x1,
};
#ifdef CONFIG_HUAWEI_DSM
typedef enum sub_system {
    SYSTEM_TYPE_WIFI = 0,
    SYSTEM_TYPE_BT = 1,
    SYSTEM_TYPE_GNSS = 2,
    SYSTEM_TYPE_FM = 3,
    SYSTEM_TYPE_PLATFORM = 4,
    SYSTEM_TYPE_BFG = 5,
    SYSTEM_TYPE_IR = 6,
    SYSTEM_TYPE_NFC = 7,
    SYSTEM_TYPE_BUT,
} sub_system_enum;
#endif

typedef enum _hi110x_release_vtype_ {
    HI110X_VTYPE_RELEASE = 0, /* release version */
    HI110X_VTYPE_RELEASE_DEBUG, /* beta user */
    HI110X_VTYPE_DEBUG, /* debug version */
    HI110X_VTYPE_BUT
} hi110x_release_vtype;

extern hi110x_board_info g_st_board_info;
extern uint32_t g_ssi_dump_en;

/* 函数声明 */
hi110x_board_info *get_hi110x_board_info(void);
int hi110x_is_asic(void);
int hi110x_is_emu(void);
void hi110x_emu_mdelay(int msec);
unsigned int hi110x_get_emu_timeout(unsigned int timeout);
int is_wifi_support(void);
int is_bfgx_support(void);
int pmu_clk_request_enable(void);
int is_hi110x_debug_type(void);
int32_t get_hi110x_subchip_type(void);
int32_t get_uart_pclk_source(void);
int32_t hi110x_board_init(void);
void hi110x_board_exit(void);
int32_t board_set_pcie_reset(int32_t is_master, int32_t is_poweron);

#ifndef HI1XX_OS_BUILD_VARIANT_USER
#define HI1XX_OS_BUILD_VARIANT_ROOT 1
#define HI1XX_OS_BUILD_VARIANT_USER 2
#endif
int32_t hi11xx_get_os_build_variant(void);
hi110x_release_vtype hi110x_get_release_type(void);
int32_t board_host_wakeup_dev_set(int value);
int32_t board_host_wakeup_bfg_set(int value);
int32_t board_get_host_wakeup_dev_stat(void);
int32_t board_wifi_tas_set(int value);
int32_t board_get_wifi_tas_gpio_state(void);
int32_t board_power_on(uint32_t subsystem);
int32_t board_power_off(uint32_t subsystem);
int32_t board_power_reset(uint32_t ul_subsystem);
int32_t board_wlan_gpio_power_on(void *data);
int32_t board_wlan_gpio_power_off(void *data);
int board_get_bwkup_gpio_val(void);
int board_get_gwkup_gpio_val(void);
int board_get_wlan_wkup_gpio_val(void);
const uint8_t* get_device_board_name(void);
int32_t board_chiptype_init(void);
int32_t get_board_gpio(const char *gpio_node, const char *gpio_prop, int32_t *physical_gpio);
int32_t board_flowctrl_gpio_init(void);
void board_flowctrl_irq_init(void);
void free_board_flowctrl_gpio(void);
void power_state_change(int32_t gpio, int32_t flag);
int32_t get_board_custmize(const char *cust_node, const char *cust_prop, const char **cust_prop_val);
int32_t get_board_dts_node(struct device_node **np, const char *node_prop);
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
int board_get_bfgx_enable_gpio_val(void);
int board_get_wlan_enable_gpio_val(void);
#ifdef _PRE_CONFIG_ARCH_KIRIN_S4_FEATURE
void suspend_board_gpio_in_s4(void);
void resume_board_gpio_in_s4(void);
#endif
#endif
#endif
