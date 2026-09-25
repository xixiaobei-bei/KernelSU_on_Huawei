

#ifndef __BOARD_H__
#define __BOARD_H__

/* 其他头文件包含 */
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include "plat_type.h"
#include "board_hi1102.h"
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

extern void hw_1103_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...);
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

#define PINMUX_SET_INIT 0
#define PINMUX_SET_SUCC 1

#define HI11XX_SUBCHIP_NAME_LEN_MAX 128

/* hi110x */
#define DTS_NODE_HISI_HI110X            "hisilicon,hi110x"
#define DTS_COMP_HISI_HI110X_BOARD_NAME DTS_NODE_HISI_HI110X

#define DTS_PROP_HI110X_VERSION "hi110x,asic_version"
#define DTS_PROP_HI110X_PMU_CLK "hi110x,pmu_clk_req"

#define DTS_PROP_HI110X_WIFI_DISABLE "hi110x,wifi_disable"
#define DTS_PROP_HI110X_BFGX_DISABLE "hi110x,bfgx_disable"

#define DTS_PROP_SUBCHIP_TYPE_VERSION "hi110x,subchip_type"

#define DTS_PROP_CLK_32K "huawei,pmu_clk32b"

#define DTS_PROP_HI110X_POWER_PREPARE "hi110x,power_prepare"
#define DTS_PROP_GPIO_XLDO_LEVEL      "xldo_gpio_level"

#define DTS_PROP_HI110X_GPIO_XLDO_PINMUX  "hi110x,gpio_xldo_pinmux"
#define PROC_NAME_HI110X_GPIO_XLDO_PINMUX "hi110x_xldo_pinmux"

#define DTS_PROP_HI110x_BUCK_MODE   "hi110x,buck_mode"

/* power on */
#define DTS_PROP_GPIO_HI110X_POWEN_ON "hi110x,gpio_power_on"
#define PROC_NAME_GPIO_POWEN_ON       "power_on_enable"

/* gpio-ssi */
#define DTS_PROP_GPIO_HI110X_GPIO_SSI_CLK  "hi110x,gpio_ssi_clk"
#define DTS_PROP_GPIO_HI110X_GPIO_SSI_DATA "hi110x,gpio_ssi_data"

#define DTS_PROP_GPIO_WLAN_POWEN_ON_ENABLE "hi110x,gpio_wlan_power_on"
#define PROC_NAME_GPIO_WLAN_POWEN_ON       "wlan_power_on_enable"

#define DTS_PROP_GPIO_BFGX_POWEN_ON_ENABLE "hi110x,gpio_bfgx_power_on"
#define PROC_NAME_GPIO_BFGX_POWEN_ON       "bfgx_power_on_enable"

/* hisi_bfgx */
#define DTS_NODE_HI110X_BFGX            "hisilicon,hisi_bfgx"
#define PROC_NAME_GPIO_BFGX_WAKEUP_HOST "bfgx_wake_host"
#define DTS_PROP_GPIO_BFGX_IR_CTRL      "hi110x,gpio_bfgx_ir_ctrl"
#define PROC_NAME_GPIO_BFGX_IR_CTRL     "bfgx_ir_ctrl"
#define DTS_PROP_HI110x_IR_LDO_TYPE     "hi110x,irled_power_type"
#define DTS_PROP_HI110x_IRLED_LDO_POWER "hi110x,irled_power"
#define DTS_PROP_HI110x_IRLED_VOLTAGE   "hi110x,irled_voltage"

#define DTS_PROP_HI110X_GPIO_BFGX_WAKEUP_HOST "hi110x,gpio_bfgx_wakeup_host"
#define DTS_PROP_HI110X_UART_POART            "hi110x,uart_port"
#define DTS_PROP_HI110X_UART_PCLK             "hi110x,uart_pclk_normal"

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

/* ini cfg */
#define INI_WLAN_DOWNLOAD_CHANNEL "board_info.wlan_download_channel"
#define INI_BFGX_DOWNLOAD_CHANNEL "board_info.bfgx_download_channel"

#define DOWNlOAD_MODE_SDIO "sdio"
#define DOWNlOAD_MODE_PCIE "pcie"
#define DOWNlOAD_MODE_UART "uart"

#define BOARD_VERSION_LEN    128
#define DOWNLOAD_CHANNEL_LEN 64

#define INI_SSI_DUMP_EN "ssi_dump_enable"


/* STRUCT 定义 */
typedef struct bd_init_s {
    int32 (*get_board_power_gpio)(struct platform_device *pdev);
    void (*free_board_power_gpio_etc)(struct platform_device *pdev);
    void (*free_board_flowctrl_gpio_etc)(void);
    int32 (*board_wakeup_gpio_init_etc)(struct platform_device *pdev);
    int32 (*board_flowctrl_gpio_init_etc)(void);
    void (*free_board_wakeup_gpio_etc)(struct platform_device *pdev);
    int32 (*board_wifi_tas_gpio_init_etc)(void);
    void (*free_board_wifi_tas_gpio_etc)(void);
    int32 (*bfgx_dev_power_on_etc)(void);
    int32 (*bfgx_dev_power_off_etc)(void);
    int32 (*wlan_power_off_etc)(void);
    int32 (*wlan_power_on_etc)(void);
    int32 (*board_power_on_etc)(uint32 ul_subsystem);
    int32 (*board_power_off_etc)(uint32 ul_subsystem);
    int32 (*board_power_reset)(uint32 ul_subsystem);
    int32 (*get_board_pmu_clk32k_etc)(void);
    int32 (*get_board_uart_port_etc)(void);
    int32 (*board_ir_ctrl_init)(struct platform_device *pdev);
    int32 (*check_evb_or_fpga_etc)(void);
    int32 (*check_hi110x_subsystem_support)(void);
    int32 (*check_pmu_clk_share_etc)(void);
    int32 (*board_get_power_pinctrl_etc)(struct platform_device *pdev);
    int32 (*get_ini_file_name_from_dts_etc)(int8 *dts_prop, int8 *prop_value, uint32 size);
#ifdef _PRE_CONFIG_S4_KIRIN_BUGFIX
    void (*suspend_board_gpio_etc_in_s4)(void);
    void (*resume_board_gpio_etc_in_s4)(void);
#endif
} bd_init_t;

typedef struct _gpio_ssi_ops_ {
    int32 (*clk_switch)(int32 ssi_clk);
    int32 (*aon_reset)(void);
} gpio_ssi_ops;

/* private data for pm driver */
typedef struct {
    /* board init ops */
    struct bd_init_s bd_ops;

    gpio_ssi_ops ssi_ops;

    /* power */
    int32 tcxo_1p95_enable; /* 1103 product */
    int32 power_on_enable;
    int32 bfgn_power_on_enable; /* 1103 product */
    int32 wlan_power_on_enable; /* 1103 product */

    /* wakeup gpio */
    int32 wlan_wakeup_host;
    int32 bfgn_wakeup_host;
    int32 host_wakeup_wlan; /* 1103 product */
    int32 host_wakeup_bfg;

    /* flowctrl gpio */
    int32 flowctrl_gpio;

    int32 rf_wifi_tas; /* 1103 product */

    int32 ssi_gpio_clk;  /* gpio-ssi-clk */
    int32 ssi_gpio_data; /* gpio-ssi-data */

    /* device hisi board verision */
    const char *chip_type;
    int32 chip_nr;

    /* how to download firmware */
    int32 wlan_download_channel;
    int32 bfgn_download_channel;

#ifdef HAVE_HISI_IR
    bool have_ir;
    int32 irled_power_type;
    int32 bfgx_ir_ctrl_gpio;
    struct regulator *bfgn_ir_ctrl_ldo;
#endif
    int32 xldo_pinmux;

    /* hi110x irq info */
    uint32 wlan_irq;
    uint32 flowctrl_irq;
    uint32 bfgx_irq;

    /* hi110x uart info */
    const char *uart_port;
    int32 uart_pclk;

    /* hi110x clk info */
    const char *clk_32k_name;
    struct clk *clk_32k;

    /* evb or fpga verison */
    int32 is_asic;

    int32 is_wifi_disable;
    int32 is_bfgx_disable;

    int32 wifi_tas_enable;

    int32 pmu_clk_share_enable;

    /* buck mode param ,get from ini */
    uint16 buck_param;

    /* WIFI/BT/GNSS/FM/IR各子系统对buck的控制flag，bitmap控制 */
    uint16 buck_control_flag;

    /* ini cfg */
    char *ini_file_name;

    /* prepare before board power on */
    int32 need_power_prepare;
    int32 pinmux_set_result;
    int32 gpio_xldo_level;
    struct pinctrl *pctrl;
    struct pinctrl_state *pins_normal;
    struct pinctrl_state *pins_idle;
} BOARD_INFO;

typedef struct _device_vesion_board {
    uint32 index;
    const char name[BOARD_VERSION_LEN + 1];
} DEVICE_BOARD_VERSION;

typedef struct _download_mode {
    uint32 index;
    uint8 name[DOWNLOAD_CHANNEL_LEN + 1];
} DOWNLOAD_MODE;


enum hisi_device_board {
    BOARD_VERSION_HI1102 = 0,
    BOARD_VERSION_HI1103 = 1,
    BOARD_VERSION_HI1102A = 2,
    BOARD_VERSION_HI1105 = 3,
    BOARD_VERSION_BOTT,
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
} SUB_SYSTEM;
#endif

typedef enum _hi110x_release_vtype_ {
    HI110X_VTYPE_RELEASE = 0, /* release version */
    HI110X_VTYPE_RELEASE_DEBUG, /* beta user */
    HI110X_VTYPE_DEBUG, /* debug version */
    HI110X_VTYPE_BUT
} hi110x_release_vtype;

extern DOWNLOAD_MODE device_download_mode_list_etc[MODE_DOWNLOAD_BUTT];
extern BOARD_INFO board_info_etc;

/* 函数声明 */
extern BOARD_INFO *get_hi110x_board_info_etc(void);
extern int isAsic_etc(void);
extern int is_wifi_support(void);
extern int is_bfgx_support(void);
extern int isPmu_clk_request_enable(void);
extern int is_hi110x_debug_type(void);
extern int32 get_hi110x_subchip_type(void);
extern int32 get_uart_pclk_source_etc(void);
extern int32 hi110x_board_init_etc(void);
extern void hi110x_board_exit_etc(void);


#ifndef HI1XX_OS_BUILD_VARIANT_USER
#define HI1XX_OS_BUILD_VARIANT_ROOT 1
#define HI1XX_OS_BUILD_VARIANT_USER 2
#endif
extern int32 hi11xx_get_os_build_variant(void);
extern hi110x_release_vtype hi110x_get_release_type(void);
extern int32 board_host_wakeup_dev_set(int value);
extern  int32 board_host_wakeup_bfg_set(int value);
extern int32 board_get_host_wakeup_dev_stat(void);
extern int32 board_wifi_tas_set(int value);
extern int32 board_get_wifi_tas_gpio_state(void);
extern int32 board_power_on_etc(uint32 subsystem);
extern int32 board_power_off_etc(uint32 subsystem);
extern int32 board_power_reset(uint32 ul_subsystem);
extern int32 board_wlan_gpio_power_on(void *data);
extern int32 board_wlan_gpio_power_off(void *data);
extern int board_get_bwkup_gpio_val_etc(void);
extern int board_get_wlan_wkup_gpio_val_etc(void);
extern int32 check_device_board_name_etc(void);
extern int32 board_chiptype_init(void);
extern int32 get_board_gpio_etc(const char *gpio_node, const char *gpio_prop, int32 *physical_gpio);
extern int32 get_board_gpio_etc(const char * gpio_node, const char * gpio_prop, int32 *physical_gpio);
extern int32 board_flowctrl_gpio_init_etc(void);
extern void board_flowctrl_irq_init_etc(void);
extern void free_board_flowctrl_gpio_etc(void);
extern void power_state_change_etc(int32 gpio, int32 flag);
extern int32 hisi_wifi_platform_register_drv(void);
extern void hisi_wifi_platform_unregister_drv(void);
extern int32 get_board_custmize_etc(const char *cust_node, const char *cust_prop, const char **cust_prop_val);
extern int32 get_board_dts_node_etc(struct device_node **np, const char *node_prop);
extern uint8 buck_mode_get(void);
extern int32 get_ssi_dump_cfg(void);
extern uint32 ssi_dump_en;
#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
extern int board_get_bfgx_enable_gpio_val_etc(void);
extern int board_get_wlan_enable_gpio_val_etc(void);
#ifdef _PRE_CONFIG_S4_KIRIN_BUGFIX
int32 enable_board_pmu_clk32k_etc(void);
int32 disable_board_pmu_clk32k_etc(void);
void suspend_board_gpio_etc_in_s4(void);
void resume_board_gpio_etc_in_s4(void);
#endif
#endif
#endif
