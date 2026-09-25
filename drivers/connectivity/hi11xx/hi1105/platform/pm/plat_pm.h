

#ifndef __PLAT_PM_H__
#define __PLAT_PM_H__

/* 其他头文件包含 */
#include <linux/mutex.h>
#include <linux/kernel.h>
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
#include <linux/pm_wakeup.h>
#endif
#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#endif
#include "plat_pm_wlan.h"
#include "hw_bfg_ps.h"
#include "board.h"
#ifdef CONFIG_HUAWEI_DSM
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#include <linux/huawei/dsm/dsm_pub.h>
#else
#include <dsm/dsm_pub.h>
#endif
#endif

/* 宏定义 */
#define BFG_LOCK_NAME               "bfg_wake_lock_etc"
#define BT_LOCK_NAME                "bt_wake_lock"
#define GNSS_LOCK_NAME              "gnss_wake_lock"

#define DEFAULT_WAKELOCK_TIMEOUT 2000 /* msec */

#define FIRMWARE_CFG_INIT_OK 0x01

#define SUCCESS 0
#define FAILURE 1

#define GNSS_AGREE_SLEEP     1
#define GNSS_NOT_AGREE_SLEEP 0

#define BFGX_SLEEP  0
#define BFGX_ACTIVE 1

#define HOST_DISALLOW_TO_SLEEP 0
#define HOST_ALLOW_TO_SLEEP    1

#define BFGX_PM_ENABLE  1
#define BFGX_PM_DISABLE 0

#define WAIT_DEVACK_MSEC              10
#define WAIT_DEVACK_CNT               10
#define WAIT_DEVACK_TIMEOUT_MSEC      200
#define WAIT_WKUP_DEVACK_TIMEOUT_MSEC 1000

#define DMD_EVENT_BUFF_SIZE 1024

/* 1103 bootloader DTCM 地址区间是0x20019000~0x2001a620，共0xa20(2592)字节 */
#define HI1103_BOOTLOAD_DTCM_BASE_ADDR 0x20019000
#define HI1103_BOOTLOAD_DTCM_SIZE      2592

/* iomcu power state:0,1->ST_POWERON,8->ST_SLEEP,9->ST_WAKEUP */
#define ST_POWERON       0
#define ST_POWERON_OTHER 1
#define ST_SLEEP         8
#define ST_WAKEUP        9

/* 超时时间要大于wkup dev work中的最长执行时间，否则超时以后进入DFR和work中会同时操作tty，导致冲突 */
#define WAIT_WKUPDEV_MSEC 10000

#define RAM_TEST_RUN_VOLTAGE_BIAS_HIGH 0x0 /* 拉偏高压 */
#define RAM_TEST_RUN_VOLTAGE_BIAS_LOW  0x1 /* 拉偏低压 */

#define RAM_TEST_RUN_VOLTAGE_REG_ADDR     0x50002010
#define RAM_TEST_RUN_PROCESS_SEL_REG_ADDR 0x50002014

/* device soc test专用宏 */
#define SOC_TEST_RUN_FINISH     0
#define SOC_TEST_RUN_BEGIN      1

#define SOC_TEST_STATUS_REG     0x50000014
#define SOC_TEST_MODE_REG       0x50000018
#define SOC_TEST_RST_FLAG       "0x5000001c,4"

/* device soc test mode */
#define TIMER_TEST_FLAG         BIT0
#define REG_MEM_CHECK_TEST_FLAG BIT1
#define INTERRUPT_TEST_FLAG     BIT2
#define MEM_MONITOR_TEST_FLAG   BIT3
#define DMA_TEST_FLAG           BIT4
#define CPU_TRACE_TEST_FLAG     BIT5
#define PATCH_TEST_FLAG         BIT6
#define SSI_MASTER_TEST_FLAG    BIT7
#define EFUSE_TEST_FLAG         BIT8
#define WDT_TEST_FLAG           BIT9
#define MPU_TEST_FLAG           BIT10
#define IPC_TEST_FLAG           BIT11
#define MBIST_TEST_FLAG         BIT12
#define SCHEDULE_FPU_TEST_FLAG  BIT13
#define I3C_SLAVE_TEST_FLAG     BIT14

#ifdef _PRE_HI_DRV_GPIO
#define BFGX_WKAEUP_HOST_GPIO (19 * 8 + 1)
#define BFGX_INT_ENABLE (1)
#define BFGX_INT_DISABLE (0)

/**type of GPIO interrupt*/
/** CNcomment:GPIO <D6>?<CF><C0><E0><D0><CD>*/
typedef enum hiBFGXUNF_GPIO_INTTYPE_E
{
    BFGX_HI_UNF_GPIO_INTTYPE_UP, /**<spring by the up edge*/                 /**< CNcomment:<C9><CF><C9><FD><D1>?<A5><B7><A2>*/
    BFGX_HI_UNF_GPIO_INTTYPE_DOWN, /**<spring by the down edge*/               /**< CNcomment:<CF>?<B5><D1>?<A5><B7><A2>*/
    BFGX_HI_UNF_GPIO_INTTYPE_UPDOWN, /**<spring by both the up and down edge*/   /**< CNcomment:?<D1>?<A5><B7><A2>*/
    BFGX_HI_UNF_GPIO_INTTYPE_HIGH, /**<spring by the high level*/              /**< CNcomment:<B8>?<E7>?<B4><A5><B7><A2>*/
    BFGX_HI_UNF_GPIO_INTTYPE_LOW, /**<spring by the low level*/               /**< CNcomment:<B5>?<E7>?<B4><A5><B7><A2>*/
    BFGX_HI_UNF_GPIO_INTTYPE_BUTT, /**<Invalid value*/                        /**< CNcomment:<B7>?<A8><B1>?<E7>?*/
} BFGX_HI_UNF_GPIO_INTTYPE_E;  //<B2>?<BC>HI_UNF_GPIO_INTTYPE_E?<BE><D9>

#ifdef _PRE_PRODUCT_HI3751_PLATO
#define HI_DRV_MODULE_GetFunction hi_drv_module_get_function
#define HI_DRV_GPIO_UnregisterServerFunc hi_drv_gpio_unregister_server_func
#endif
extern int HI_DRV_MODULE_GetFunction(unsigned int u32ModuleID, void** ppFunc);

typedef enum hiMOD_ID_E
{
    HI_ID_STB         = 0,

    /**< common. */
    HI_ID_SYS         = 1,
    HI_ID_MODULE      ,
    HI_ID_LOG         ,
    HI_ID_PROC        ,
    HI_ID_MEM         ,
    HI_ID_STAT        ,
    HI_ID_PDM         ,
    HI_ID_MEMDEV      ,
    HI_ID_CUSTOM      ,
    HI_ID_COMMON_BUTT ,

    HI_ID_DEMUX       = HI_ID_COMMON_BUTT + 1,
    HI_ID_DEMUX_BUTT  ,

    /**< audio. */
    HI_ID_ADEC        = HI_ID_DEMUX_BUTT + 1,
    HI_ID_AO          ,
    HI_ID_SIO_AI      ,
    HI_ID_SIO_AO      ,
    HI_ID_SIO         ,
    HI_ID_AI          ,
    HI_ID_AENC        ,
    HI_ID_SRC         ,
    HI_ID_AIAO        ,
    HI_ID_AFLT        ,
    HI_ID_ADSP        ,
    HI_ID_AMP         ,
    HI_ID_SIF         ,
    HI_ID_TTS         ,
    HI_ID_AUDIO_BUTT  ,

    /**< video. */
    HI_ID_VFMW        = HI_ID_AUDIO_BUTT + 1,
    HI_ID_SVDEC       ,
    HI_ID_DISP        ,
    HI_ID_HDMI        ,
    HI_ID_VO          ,
    HI_ID_FRC         ,
    HI_ID_VPSS        ,
    HI_ID_VDEC        ,
    HI_ID_VI          ,
    HI_ID_VENC        ,
    HI_ID_PQ          ,
    HI_ID_EDID        ,
    HI_ID_MEMC        ,
    HI_ID_VICAP       ,
    HI_ID_PANEL       ,
    HI_ID_FDMNG       ,
    HI_ID_RM          ,
    HI_ID_VIDEO_BUTT  ,

    /**< graphics. */
    HI_ID_TDE         = HI_ID_VIDEO_BUTT + 1,
    HI_ID_JPGDEC      ,
    HI_ID_JPGENC      ,
    HI_ID_FB          ,
    HI_ID_PNG         ,
    HI_ID_HIGO        ,
    HI_ID_GRAPHICS_BUTT,

    /**< player. */
    HI_ID_PVR         = HI_ID_GRAPHICS_BUTT + 1,
    HI_ID_AVPLAY      ,
    HI_ID_SYNC        ,
    HI_ID_VSYNC       ,
    HI_ID_ASYNC       ,
    HI_ID_FASTPLAY    ,
    HI_ID_PLAYER_BUTT ,

    /**< ecs. */
    HI_ID_FLASH       = HI_ID_PLAYER_BUTT + 1,
    HI_ID_IR          ,
    HI_ID_RTC         ,
    HI_ID_I2C         ,
    HI_ID_SCI         ,
    HI_ID_ETH         ,
    HI_ID_USB_PROTECT ,
    HI_ID_WDG         ,   /* watch dog used 'W' */
    HI_ID_GPIO        ,
    HI_ID_GPIO_I2C    ,
    HI_ID_DMAC        ,
    HI_ID_PMOC        ,
    HI_ID_TUNER       ,
    HI_ID_KEYLED      ,
    HI_ID_CIPHER      ,
    HI_ID_OTP         ,
    HI_ID_CA          ,
    HI_ID_PM          ,
    HI_ID_CI          ,
    HI_ID_CIMAXPLUS   ,
    HI_ID_TVP5150     ,
    HI_ID_SIL9293     ,
    HI_ID_PWM         ,
    HI_ID_LSADC       ,
    HI_ID_SPI         ,
    HI_ID_ECS_BUTT    ,

    /**< ATV */
    HI_ID_VFE         = HI_ID_ECS_BUTT + 1,
    HI_ID_TVD         ,
    HI_ID_HDDEC       ,
    HI_ID_HDMIRX      ,
    HI_ID_VBI         ,
    HI_ID_ATV_BUTT    ,

    /**< voip, bluetooth,alsa. */
    HI_ID_VOIP_HME    = HI_ID_ATV_BUTT + 1,
    HI_ID_NDPT        ,
    HI_ID_AUDADP      ,
    HI_ID_BT          ,
    HI_ID_ALSA        ,
    HI_ID_3G          ,
    HI_ID_KARAOKE     ,
    HI_ID_VOIP_BUTT   ,

    /**< vp. */
    HI_ID_VP          = HI_ID_VOIP_BUTT+1,
    HI_ID_HDCP        ,
    HI_ID_VP_BUTT     ,

    /**< subtitle. */
    HI_ID_SUBT        = HI_ID_VP_BUTT + 1,
    HI_ID_TTX         ,
    HI_ID_CC          ,
    HI_ID_SUBTITLE_BUTT,

    /**< loader. */
    HI_ID_LOADER      = HI_ID_SUBTITLE_BUTT + 1,
    HI_ID_LOADER_BUTT ,

    /**< user definition. */
    HI_ID_USR_START   = HI_ID_LOADER_BUTT + 1,
    HI_ID_USER              ,
    HI_ID_USR_END     = 0xFE,    /*Caution: to avoid overflow*/

    HI_ID_BUTT        = 0xFF
} HI_MOD_ID_E;

typedef struct hiGPIO_GET_GPIONUM_S
{
        unsigned char u8GpioGrpNum;     /* gpio group number*/
        unsigned char u8GpioMaxNum;     /* gpio  max number*/
} GPIO_GET_GPIONUM_S;

extern int HI_DRV_GPIO_UnregisterServerFunc(unsigned int u32GpioNo);

typedef int (*FN_GPIO_Get_Bit)(unsigned int, unsigned int*);
typedef int (*FN_GPIO_Set_Bit)(unsigned int, unsigned int);
typedef int (*FN_GPIO_Get_Num)(GPIO_GET_GPIONUM_S*);

typedef int (*FN_GPIO_Register_Server_Func)(unsigned int, void (*func)(unsigned int));
typedef int (*FN_GPIO_UnRegister_Server_Func)(unsigned int);
typedef int (*FN_GPIO_Set_Int_Type)(unsigned int, HI_UNF_GPIO_INTTYPE_E);
typedef int (*FN_GPIO_Set_Int_Enable)(unsigned int, bool);
typedef int (*FN_GPIO_Clear_GroupInt)(unsigned int);
typedef int (*FN_GPIO_Clear_BitInt)(unsigned int);
typedef int (*FN_GPIO_Get_UsrAddr)(unsigned int, void **);

typedef struct
{
        FN_GPIO_Get_Bit                pfnGpioDirGetBit;
        FN_GPIO_Set_Bit                pfnGpioDirSetBit;
        FN_GPIO_Get_Bit                pfnGpioReadBit;
        FN_GPIO_Set_Bit                pfnGpioWriteBit;
        FN_GPIO_Get_Num                pfnGpioGetNum;
        FN_GPIO_Register_Server_Func   pfnGpioRegisterServerFunc;
        FN_GPIO_UnRegister_Server_Func pfnGpioUnRegisterServerFunc;
        FN_GPIO_Set_Int_Type           pfnGpioSetIntType;
        FN_GPIO_Set_Int_Enable         pfnGpioSetIntEnable;
        FN_GPIO_Clear_GroupInt         pfnGpioClearGroupInt;
        FN_GPIO_Clear_BitInt           pfnGpioClearBitInt;
        FN_GPIO_Get_UsrAddr            pfnGPIOGetUsrAddr;
} GPIO_EXT_FUNC_S;

#endif

enum UART_STATE_ENUM {
    UART_NOT_READY = 0,
    UART_READY = 1,
    UART_BPS_CHG_SEND_ACK = 2,
    UART_BPS_CHG_IN_PROGRESS = 3,
    UART_BPS_CHG_SEND_COMPLETE = 4,
};

/* BFGX系统上电加载异常类型 */
enum BFGX_POWER_ON_EXCEPTION_ENUM {
    BFGX_POWER_FAILED = -1,
    BFGX_POWER_SUCCESS = 0,

    BFGX_POWER_PULL_POWER_GPIO_FAIL = 1,
    BFGX_POWER_TTY_OPEN_FAIL = 2,
    BFGX_POWER_TTY_FLOW_ENABLE_FAIL = 3,

    BFGX_POWER_WIFI_DERESET_BCPU_FAIL = 4,
    BFGX_POWER_WIFI_ON_BOOT_UP_FAIL = 5,

    BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL = 6,
    BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL = 7,

    BFGX_POWER_WAKEUP_FAIL = 8,
    BFGX_POWER_OPEN_CMD_FAIL = 9,

    BFGX_POWER_DOWNLOAD_FIRMWARE_INTERRUPT = 10,

    BFGX_POWER_ENUM_BUTT,
};

/* wifi系统上电加载异常类型 */
enum WIFI_POWER_ON_EXCEPTION_ENUM {
    WIFI_POWER_FAIL = -1,
    WIFI_POWER_SUCCESS = 0,
    WIFI_POWER_PULL_POWER_GPIO_FAIL = 1,

    WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL = 2,
    WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL = 3,

    WIFI_POWER_BFGX_ON_BOOT_UP_FAIL = 4,
    WIFI_POWER_BFGX_DERESET_WCPU_FAIL = 5,
    WIFI_POWER_BFGX_ON_FIRMWARE_DOWNLOAD_FAIL = 6,
    WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT = 7,

    WIFI_POWER_BFGX_OFF_PULL_WLEN_FAIL = 8,
    WIFI_POWER_BFGX_ON_PULL_WLEN_FAIL = 9,
    WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL = 10,

    WIFI_POWER_ENUM_BUTT,
};

#ifdef CONFIG_HI110X_GPS_SYNC
enum gnss_rat_mode_enum {
    gnss_rat_mode_no_service = 0,
    gnss_rat_mode_gsm = 1,
    gnss_rat_mode_cdma = 2,
    gnss_rat_mode_wcdma = 3,
    gnss_rat_mode_lte = 6,
    gnss_rat_mode_nr = 11,
    gnss_rat_mode_butt
};

enum gnss_sync_mode_enum {
    gnss_sync_mode_unknown = -1,
    gnss_sync_mode_lte = 0,
    gnss_sync_mode_lte2 = 1,
    gnss_sync_mode_cdma = 2,
    gnss_sync_mode_g1 = 3,
    gnss_sync_mode_g2 = 4,
    gnss_sync_mode_g3 = 5,
    gnss_sync_mode_pw = 6,
    gnss_sync_mode_sw = 7,
    gnss_sync_mode_nstu = 8,
    gnss_sync_mode_butt
};

enum gnss_sync_version_enum {
    gnss_sync_version_off = 0, // feature switch off
    gnss_sync_version_4g = 1, // support 2/3/4g
    gnss_sync_version_5g = 2, // based 1 and add 5g
    gnss_sync_version_butt
};
#endif

#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
struct hisi_bt_dev {
    struct hci_dev *hdev;
    struct task_struct *bt_recv_task;
};
#endif

/* private data for pm driver */
struct pm_drv_data {
    /* 3 in 1 interface pointer */
    struct ps_pm_s *ps_pm_interface;

    /* wlan interface pointer */
    struct wlan_pm_s *pst_wlan_pm_info;

    /* board customize info */
    BOARD_INFO *board;
    /* wake lock for bfg,be used to prevent host form suspend */
    oal_wakelock_stru bfg_wake_lock_etc;
    oal_wakelock_stru bt_wake_lock;
    oal_wakelock_stru gnss_wake_lock;

    /* mutex for sync */
    struct mutex host_mutex;

    /* wakelock protect spinlock while wkup isr VS allow sleep ack and devack_timer */
    spinlock_t wakelock_protect_spinlock;

    /* bfgx VS. bfg timer spinlock */
    spinlock_t uart_state_spinlock;

    /* uart could be used or not */
    uint8 uart_state;

    /* mark receiving data after set dev as sleep state but before get ack of device */
    uint8 rcvdata_bef_devack_flag;

    uint8 uc_dev_ack_wait_cnt;

    /* bfgx sleep state */
    uint8 bfgx_dev_state;

    /* hal layer ioctl set flag not really close bt */
    uint32 bt_fake_close_flag;

    /* flag for firmware cfg file init */
    uint64 firmware_cfg_init_flag;

    spinlock_t bfg_irq_spinlock;

    /* bfg irq num */
    uint32 bfg_irq;

    uint32 ul_irq_stat;

    /* bfg wakeup host count */
    uint32 bfg_wakeup_host;

    /* gnss lowpower state */
    atomic_t gnss_sleep_flag;

    atomic_t bfg_needwait_devboot_flag;

    /* flag to mark whether enable lowpower or not */
    uint32 bfgx_pm_ctrl_enable;
    uint8 bfgx_lowpower_enable;
    uint8 bfgx_bt_lowpower_enable;
    uint8 bfgx_gnss_lowpower_enable;
    uint8 bfgx_nfc_lowpower_enable;

    /* workqueue for wkup device */
    struct workqueue_struct *wkup_dev_workqueue;
    struct work_struct wkup_dev_work;
    struct work_struct send_disallow_msg_work;
    struct work_struct send_allow_sleep_work;
    struct work_struct baud_change_work;
    uint32 uart_baud_switch_to;

    /* wait device ack timer */
    struct timer_list bfg_timer;
    uint32 bfg_timer_mod_cnt;
    uint32 bfg_timer_mod_cnt_pre;
    unsigned long bfg_timer_check_time;
    uint64 rx_pkt_gnss_pre;
    uint32 gnss_votesleep_check_cnt;
    struct timer_list dev_ack_timer;
    struct timer_list baud_change_timer;

    /* the completion for waiting for host wake up device ok */
    struct completion host_wkup_dev_comp;
    /* the completion for waiting for dev ack(host allow sleep) ok */
    struct completion dev_ack_comp;
    /* the completion for waiting for dev boot ok */
    struct completion dev_bootok_ack_comp;
#ifdef _PRE_CONFIG_HISI_110X_BLUEZ
    struct hisi_bt_dev st_bt_dev;
#endif
};

#ifdef CONFIG_HI110X_GPS_SYNC
struct gnss_sync_data {
    void __iomem *addr_base_virt;
    uint32 addr_offset;
    uint32 version;
};
#endif

#define BUCK_MODE_OFFSET                        14
#define BUCK_MODE_MASK                          0xC000   // bit[15:14]
#define EXT_BUCK_OFF                             0        // 无外置buck,全内置
#define EXT_BUCK_I2C_CTL                         1        // I2C控制独立外置buck
#define EXT_BUCK_GPIO_CTL                        2        // GPIO控制独立外置buck
#define EXT_BUCK_HOST_CTL                        3        // host控制共享外置buck

/* EXTERN VARIABLE */
extern int ram_test_detail_tsensor_dump;
extern int ram_test_run_voltage_bias_sel;

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
typedef oal_uint32 (*work_cb)(oal_void);
void pm_host_walcb_register_etc(work_cb suspend_cb, work_cb resume_cb);
#endif

/* EXTERN FUNCTION */
#ifdef _PRE_CONFIG_S4_KIRIN_BUGFIX
extern irqreturn_t bfg_wake_host_isr_etc(int irq, void *dev_id);
#endif
extern void BuckPowerCtrl(uint8 enable, uint8 subsys);
extern int32 wait_gpio_level_etc(int32 gpio_num, int32 gpio_level, unsigned long timeout);
extern void bfgx_gpio_intr_enable(uint32 ul_en);
extern struct pm_drv_data *pm_get_drvdata_etc(void);
extern int32 host_wkup_dev_etc(void);
extern struct pm_drv_data *pm_get_drvdata_etc(void);
extern int32 bfgx_other_subsys_all_shutdown_etc(uint8 subsys);
extern void bfgx_print_subsys_state(void);
extern bool wlan_is_shutdown_etc(void);
extern bool bfgx_is_shutdown_etc(void);
extern int32 wlan_power_on_etc(void);
extern int32 wlan_power_off_etc(void);
extern int32 bfgx_power_on_etc(uint8 subsys);
extern int32 bfgx_power_off_etc(uint8 subsys);
extern int32 bfgx_pm_feature_set_etc(void);
extern int firmware_download_function_etc(uint32 which_cfg);
typedef int32 (*firmware_downlaod_privfunc)(void);
extern int firmware_download_function_priv(uint32 which_cfg, firmware_downlaod_privfunc);
extern oal_int32 hi110x_get_wifi_power_stat_etc(oal_void);
extern int32 device_mem_check_etc(unsigned long long *time);
extern int32 bfgx_uart_rcv_baud_change_req(uint8 uc_msg_type);
extern int32 bfgx_uart_rcv_baud_change_complete_ack(void);
extern int32 memcheck_is_working(void);
extern void bfg_wake_lock_etc(void);
extern void bfg_wake_unlock_etc(void);
extern void bfg_check_timer_work(void);
#ifdef CONFIG_HI110X_GPS_SYNC
extern struct gnss_sync_data *gnss_get_sync_data(void);
extern int gnss_sync_init(void);
extern void gnss_sync_exit(void);
#endif
#ifdef CONFIG_HUAWEI_DSM
extern void hw_1103_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...);
#endif
extern int32 pm_uart_send(uint8 *date, int32 len);
extern int low_power_init_etc(void);
extern void low_power_exit_etc(void);
extern int32 bfgx_dev_power_control_etc(uint8 subsys, uint8 flag);
#endif
