

/* 头文件包含 */
#include <linux/module.h> /* kernel module definitions */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>
#include <linux/mutex.h>
#include <linux/kernel.h>

#ifdef CONFIG_HISI_IDLE_SLEEP
#include <linux/hisi/hisi_idle_sleep.h>
#endif
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>

#include "board.h"
#include "ssi_common.h"
#include "hw_bfg_ps.h"
#include "plat_type.h"
#include "plat_debug.h"
#include "plat_sdio.h"
#include "plat_uart.h"
#include "plat_firmware.h"
#include "plat_pm.h"
#include "plat_exception_rst.h"
#include "securec.h"

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#include "wireless_patch.h"
#endif

#include "oal_sdio.h"
#include "oal_sdio_comm.h"
#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oal_schedule.h"
#include "plat_firmware.h"
#include "bfgx_exception_rst.h"

#ifdef _PRE_SHARE_BUCK_SURPORT
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_PM_C

int ram_test_ssi_error_dump = 0;
oal_debug_module_param(ram_test_ssi_error_dump, int, S_IRUGO | S_IWUSR);
int ram_test_ssi_pass_dump = 0;
oal_debug_module_param(ram_test_ssi_pass_dump, int, S_IRUGO | S_IWUSR);
int ram_test_detail_result_dump = 1;
oal_debug_module_param(ram_test_detail_result_dump, int, S_IRUGO | S_IWUSR);
int ram_test_detail_tsensor_dump = 1;
oal_debug_module_param(ram_test_detail_tsensor_dump, int, S_IRUGO | S_IWUSR);
int ram_test_mem_pass_dump = 0;
oal_debug_module_param(ram_test_mem_pass_dump, int, S_IRUGO | S_IWUSR);

/*
 * 0 表示 用例全部跑完,
 * 1表示 case1跑完返回，
 * 2表示 case2跑完返回 类推
 */
int ram_test_run_process_sel = 0x0;
oal_debug_module_param(ram_test_run_process_sel, int, S_IRUGO | S_IWUSR);

int ram_test_run_voltage_bias_sel = RAM_TEST_RUN_VOLTAGE_BIAS_HIGH;
oal_debug_module_param(ram_test_run_voltage_bias_sel, int, S_IRUGO | S_IWUSR);

int ram_test_wifi_hold_time = 0; /* after done the test, we hold the process to test signal(ms) */
oal_debug_module_param(ram_test_wifi_hold_time, int, S_IRUGO | S_IWUSR);

int ram_test_bfgx_hold_time = 0; /* after done the test, we hold the process to test signal(ms) */
oal_debug_module_param(ram_test_bfgx_hold_time, int, S_IRUGO | S_IWUSR);

#ifdef _PRE_HI_DRV_GPIO
GPIO_EXT_FUNC_S *pstHisiGpioFunc = NULL;
#endif

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
struct s_pm_wal_host_handler {
    work_cb pf_wal_host_resume_work_func;
    work_cb pf_wal_host_suspend_work_func;
};

struct s_pm_wal_host_handler g_pm_wal_host_handler = {NULL, NULL};

typedef enum {
    PM_S3S4_CHR_WIFI_RESUME_HANDLE_NULL = 1,
    PM_S3S4_CHR_WIFI_SUSPEND_HANDLE_NULL = 2,
    PM_S3S4_CHR_BT_RESUME_HANDLE_NULL = 3,
    PM_S3S4_CHR_BT_SUSPEND_HANDLE_NULL = 4,
    PM_S3S4_CHR_WIFI_RESUME_FAIL = 5,
    PM_S3S4_CHR_WIFI_SUSPEND_FAIL = 6,
    PM_S3S4_CHR_BT_RESUME_FAIL = 7,
    PM_S3S4_CHR_BT_SUSPEND_FAIL = 8,
    PM_S3S4_CHR_BUTT
} pm_s3s4_status_enum;

typedef struct tag_pm_s3s4_chr_info {
    oal_uint16 us_s3s4_status;
    oal_uint8 resv[2];
} pm_s3s4_chr_info_stru;
#ifdef _PRE_CONFIG_S3_HCI_DEV_OPT
extern int extend_hci_dev_do_open(struct hci_dev *hdev);
#endif

#define POWER_OFF 0
#define POWER_ON 1
#define BT_IOCTL_HCISETPROTO 101
#define BT_IOCTL_HCIUNSETPROTO 102
int g_wifi_is_enable = 0;
int g_bfgx_is_enable = 0;

void pm_host_walcb_register_etc(work_cb suspend_cb, work_cb resume_cb)
{
    g_pm_wal_host_handler.pf_wal_host_suspend_work_func = suspend_cb;
    g_pm_wal_host_handler.pf_wal_host_resume_work_func = resume_cb;
}

EXPORT_SYMBOL(pm_host_walcb_register_etc);
void resume_hi110x_wifi(void)
{
    int ret = FAILURE;
    pm_s3s4_chr_info_stru pm_s3s4_chr_info = { 0 };

    if (OAL_LIKELY(g_pm_wal_host_handler.pf_wal_host_resume_work_func)) {
        PS_PRINT_INFO("resume_hi110x_wifi::wal_host_resume_work_etc\n");
        ret = g_pm_wal_host_handler.pf_wal_host_resume_work_func();
        if (ret != SUCCESS) {
            pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_WIFI_RESUME_FAIL;
        }
    } else {
        PS_PRINT_ERR("resume_hi110x_wifi::wal_host_resume_work_etc NULL\n");
        pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_WIFI_RESUME_HANDLE_NULL;
    }

    if (ret != SUCCESS) {
        CHR_EXCEPTION_P(CHR_PLATFORM_S3S4_EVENTID,
                        (oal_uint8 *)(&pm_s3s4_chr_info), OAL_SIZEOF(pm_s3s4_chr_info_stru));
    }
}

int resume_hi110x_bfgx(void)
{
#ifdef _PRE_CONFIG_S3_HCI_DEV_OPT
    int ret;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    pm_s3s4_chr_info_stru pm_s3s4_chr_info = { 0 };

    if (pm_data == NULL) {
        PS_PRINT_ERR("resume_hi110x_bfgx::pm_get_drvdata_etc return null\n");
        pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_BT_RESUME_HANDLE_NULL;
        CHR_EXCEPTION_P(CHR_PLATFORM_S3S4_EVENTID,
                        (oal_uint8 *)(&pm_s3s4_chr_info), OAL_SIZEOF(pm_s3s4_chr_info_stru));
        return -FAILURE;
    }

    ret = extend_hci_dev_do_open(pm_data->st_bt_dev.hdev);
    PS_PRINT_INFO("extend_hci_dev_do_open in resume_hi110x_bfgx\n");
    if (ret != SUCCESS) {
        PS_PRINT_ERR("extend_hci_dev_do_open fail in resume_hi110x\n");
        pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_BT_RESUME_FAIL;
        CHR_EXCEPTION_P(CHR_PLATFORM_S3S4_EVENTID,
                        (oal_uint8 *)(&pm_s3s4_chr_info), OAL_SIZEOF(pm_s3s4_chr_info_stru));
    }
    return ret;
#else
    PS_PRINT_INFO("resume_hi110x_bfgx::hw_bt_ioctl register\n");
    hw_bt_ioctl(NULL, BT_IOCTL_HCISETPROTO, 0);
    return SUCCESS;
#endif
}

#ifdef _PRE_CONFIG_S4_KIRIN_BUGFIX
int set_board_s4_etc(unsigned long mode)
{
    int ret = -1;
    if(mode == PM_HIBERNATION_PREPARE) {
       /* free interrupt(GPIO/PCIE) */
       oal_free_irq_in_s4();
       /* close 32khz clk in S4 suspend */
       ret = disable_board_pmu_clk32k_etc();
       if(ret != BOARD_SUCC) {
           PS_PRINT_INFO("close 32khz Fail in S4 suspend_hi110x\n");
       } else {
           PS_PRINT_INFO("close 32khz Succ in S4 suspend_hi110x\n");
       }
       /* gpio deinit */
       suspend_board_gpio_etc_in_s4();
    } else if(mode == PM_POST_HIBERNATION) {
       /* gpio init */
       resume_board_gpio_etc_in_s4();
        /* open 32khz clk in S4 resume */
       ret = enable_board_pmu_clk32k_etc();
       if(ret != BOARD_SUCC) {
           PS_PRINT_INFO("open 32khz Fail in S4 resume_hi110x\n");
       } else {
           PS_PRINT_INFO("open 32khz Succ in S4 resume_hi110x\n");
       }
       /* interrupt request (GPIO/PCIE) and disable request */
       oal_request_irq_in_s4();
    } else {
        PS_PRINT_INFO("not set 32khz clk in other mode\n");
    }
    return ret;
}
#endif

void resume_hi110x(void)
{
    int ret;
    PS_PRINT_INFO("resume_hi110x\n");

    if (g_wifi_is_enable) {
        resume_hi110x_wifi();
    } else {
        PS_PRINT_INFO("wifi is disable,so no need call wal_host_resume_work_etc in resume_hi110x\n");
    }

    if (g_bfgx_is_enable) {
        ret = resume_hi110x_bfgx();
        if (ret != SUCCESS) {
            return;
        }
    } else {
        PS_PRINT_INFO("bfgx is disable,so no need call hw_bt_ioctl register in resume_hi110x\n");
    }
}

int suspend_hi110x_bfgx(void)
{
#ifdef _PRE_CONFIG_S3_HCI_DEV_OPT
    int ret;
    pm_s3s4_chr_info_stru pm_s3s4_chr_info = { 0 };
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("suspend_hi110x_bfgx::pm_get_drvdata_etc return null\n");
        pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_BT_SUSPEND_HANDLE_NULL;
        CHR_EXCEPTION_P(CHR_PLATFORM_S3S4_EVENTID,
                        (oal_uint8 *)(&pm_s3s4_chr_info), OAL_SIZEOF(pm_s3s4_chr_info_stru));
        return -FAILURE;
    }

    ret = hci_dev_do_close(pm_data->st_bt_dev.hdev);
    PS_PRINT_INFO("hci_dev_do_close in suspend_hi110x_bfgx\n");
    if (ret != SUCCESS) {
        PS_PRINT_ERR("hci_dev_do_close fail in suspend_hi110x_bfgx\n");
        pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_BT_SUSPEND_FAIL;
        CHR_EXCEPTION_P(CHR_PLATFORM_S3S4_EVENTID,
                        (oal_uint8 *)(&pm_s3s4_chr_info), OAL_SIZEOF(pm_s3s4_chr_info_stru));
    } else {
        g_bfgx_is_enable = POWER_ON;
    }
    return ret;
#else
    PS_PRINT_INFO("suspend_hi110x_bfgx::hw_bt_ioctl unregister\n");
    hw_bt_ioctl(NULL, BT_IOCTL_HCIUNSETPROTO, 0);
    g_bfgx_is_enable = POWER_ON;
    return SUCCESS;
#endif
}

void suspend_hi110x_wifi(void)
{
    int ret;
    pm_s3s4_chr_info_stru pm_s3s4_chr_info = { 0 };

    if (OAL_LIKELY(g_pm_wal_host_handler.pf_wal_host_suspend_work_func)) {
        PS_PRINT_INFO("suspend_hi110x_wifi::wal_host_suspend_work_etc\n");
        ret = g_pm_wal_host_handler.pf_wal_host_suspend_work_func();
        if (ret != SUCCESS) {
            pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_WIFI_SUSPEND_FAIL;
        }
    } else {
        PS_PRINT_ERR("suspend_hi110x_wifi::wal_host_suspend_work_etc NULL\n");
        pm_s3s4_chr_info.us_s3s4_status = PM_S3S4_CHR_WIFI_SUSPEND_HANDLE_NULL;
        ret = -FAILURE;
    }

    if (ret != SUCCESS) {
        CHR_EXCEPTION_P(CHR_PLATFORM_S3S4_EVENTID,
                        (oal_uint8 *)(&pm_s3s4_chr_info), OAL_SIZEOF(pm_s3s4_chr_info_stru));
    } else {
        g_wifi_is_enable = POWER_ON;
    }
}

void suspend_hi110x(void)
{
    int ret;
    PS_PRINT_INFO("suspend_hi110x\n");

    if (board_get_bfgx_enable_gpio_val_etc()) {
        ret = suspend_hi110x_bfgx();
        if (ret != SUCCESS) {
            return;
        }
    } else {
        PS_PRINT_INFO("bfgx is disable,so no need call hw_bt_ioctl unregister in suspend_hi110x\n");
        g_bfgx_is_enable = POWER_OFF;
    }

    if (board_get_wlan_enable_gpio_val_etc()) {
        suspend_hi110x_wifi();
    } else {
        PS_PRINT_INFO("wifi is disable,so no need call wal_host_suspend_work_etc in suspend_hi110x\n");
        g_wifi_is_enable = POWER_OFF;
    }
}
#endif

#ifdef _PRE_HOST_SUSPEND_UART_POWERDOWN
#define WAIT_TTY_CLOSE_TIME 200

static int send_disallow_msg_via_tty(struct pm_drv_data *pm_data, struct ps_core_s *ps_core_d)
{
    int ret = 0;
    PS_PRINT_INFO("host resume start!\n");

    if (bfgx_is_shutdown_etc() == false) {
        open_tty_drv_etc(ps_core_d->pm_data);

        ret = prepare_to_visit_node_etc(ps_core_d);
        if ( ret < 0 ) {
            PS_PRINT_ERR("wake up device fail, bring to reset work\n");
            plat_exception_handler_etc(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
            return ret;
        }

        queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);

        post_to_visit_node_etc(ps_core_d);
    }
    return 0;
}

static void release_tty_after_send_msg(struct ps_core_s *ps_core_d)
{
    release_tty_drv_etc(ps_core_d->pm_data);
    msleep(WAIT_TTY_CLOSE_TIME);
}
#endif

#ifdef _PRE_CONFIG_HISI_S3S4_POWER_STATE
static int pf_suspend_notify(struct notifier_block *notify_block,
                             unsigned long mode, void *unused)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return IRQ_NONE;
    }

    switch (mode) {
        case PM_POST_SUSPEND:
        case PM_POST_HIBERNATION:
            PS_PRINT_INFO("S3S4 resume now!\n");
#ifdef _PRE_CONFIG_S4_KIRIN_BUGFIX
            set_board_s4_etc(mode);
#endif
            resume_hi110x();
            break;

        case PM_SUSPEND_PREPARE:
        case PM_HIBERNATION_PREPARE:
            PS_PRINT_INFO("S3S4 suspend now!\n");
            suspend_hi110x();
#ifdef _PRE_CONFIG_S4_KIRIN_BUGFIX
            set_board_s4_etc(mode);
#endif
            break;
        default:
            break;
    }
    return 0;
}
#else

/*
 * Function: suspend_notify
 * Description: suspend notify call back
 * Ruturn: 0 -- success
 */
static int pf_suspend_notify(struct notifier_block *notify_block,
                             unsigned long mode, void *unused)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
#ifdef _PRE_HOST_SUSPEND_UART_POWERDOWN
    int32 ret = 0;
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }
#endif
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return IRQ_NONE;
    }

    switch (mode) {
        case PM_POST_SUSPEND:
#ifdef _PRE_HOST_SUSPEND_UART_POWERDOWN
            ret = send_disallow_msg_via_tty(pm_data, ps_core_d);
            if (ret != 0) {
                return ret;
            }
#endif
            PS_PRINT_INFO("host resume OK!\n");
            break;
        case PM_SUSPEND_PREPARE:
#ifdef _PRE_HOST_SUSPEND_UART_POWERDOWN
            release_tty_after_send_msg(ps_core_d);
#endif
            PS_PRINT_INFO("host suspend now!\n");
            break;
        default:
            break;
    }
    return 0;
}
#endif

static struct notifier_block pf_suspend_notifier = {
    .notifier_call = pf_suspend_notify,
    .priority = INT_MIN,
};

struct pm_drv_data *pm_drv_data_t_etc = NULL;

struct pm_drv_data *pm_get_drvdata_etc(void)
{
    return pm_drv_data_t_etc;
}

static void pm_set_drvdata(struct pm_drv_data *data)
{
    pm_drv_data_t_etc = data;
}

#ifdef CONFIG_HUAWEI_DSM
OAL_DEFINE_SPINLOCK(g_dsm_lock);
/*
 * 函 数 名  : hw_1103_dsm_client_notify
 * 功能描述  : DMD事件上报
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
static struct dsm_dev dsm_wifi = {
    .name = "dsm_wifi",
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
    .fops = NULL,
    .buff_size = DMD_EVENT_BUFF_SIZE,
};

static struct dsm_dev dsm_bt = {
    .name = "dsm_bt",
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
    .fops = NULL,
    .buff_size = DMD_EVENT_BUFF_SIZE,
};

struct dsm_client *hw_1103_dsm_wifi_client = NULL;
struct dsm_client *hw_1103_dsm_bt_client = NULL;

void hw_1103_register_dsm_client(void)
{
    if (hw_1103_dsm_wifi_client == NULL) {
        hw_1103_dsm_wifi_client = dsm_register_client(&dsm_wifi);
    }

    if (hw_1103_dsm_bt_client == NULL) {
        hw_1103_dsm_bt_client = dsm_register_client(&dsm_bt);
    }
}

void hw_1103_unregister_dsm_client(void)
{
    if (hw_1103_dsm_wifi_client != NULL) {
        dsm_unregister_client(hw_1103_dsm_wifi_client, &dsm_wifi);
        hw_1103_dsm_wifi_client = NULL;
    }
    if (hw_1103_dsm_bt_client != NULL) {
        dsm_unregister_client(hw_1103_dsm_bt_client, &dsm_bt);
        hw_1103_dsm_bt_client = NULL;
    }
}
#define LOG_BUF_SIZE 512
int last_dsm_id = 0;

void hw_1103_dsm_client_notify(int sub_sys, int dsm_id, const char *fmt, ...)
{
    oal_ulong flags;
    char buf[LOG_BUF_SIZE] = {0};
    struct dsm_client *dsm_client_buf = NULL;
    va_list ap;
    int32 ret = 0;

    switch (sub_sys) {
        case SYSTEM_TYPE_WIFI:
        case SYSTEM_TYPE_PLATFORM:
            dsm_client_buf = hw_1103_dsm_wifi_client;
            break;
        case SYSTEM_TYPE_BT:
            dsm_client_buf = hw_1103_dsm_bt_client;
            break;

        default:
            OAM_ERROR_LOG1(0, 0, "hw_1103_dsm_client_notify::unsupport dsm sub_type[%d]", sub_sys);
            break;
    }

    if (dsm_client_buf != NULL) {
        if (fmt != NULL) {
            va_start(ap, fmt);
            ret = vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, fmt, ap);
            va_end(ap);
            if (ret < 0) {
                OAM_ERROR_LOG1(0, 0, "vsnprintf_s fail, line[%d]\n", __LINE__);
                return;
            }
        } else {
            OAM_ERROR_LOG1(0, 0, "dsm_client_buf is null, line[%d]\n", __LINE__);
            return;
        }
    }

    oal_spin_lock_irq_save(&g_dsm_lock, &flags);
    if (!dsm_client_ocuppy(dsm_client_buf)) {
        dsm_client_record(dsm_client_buf, buf);
        dsm_client_notify(dsm_client_buf, dsm_id);
        last_dsm_id = dsm_id;
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "wifi dsm_client_notify success,dsm_id=%d", dsm_id);
        OAL_IO_PRINT("[I]wifi dsm_client_notify success,dsm_id=%d[%s]\n", dsm_id, buf);
    } else {
        OAM_ERROR_LOG2(0, OAM_SF_PWR, "wifi dsm_client_notify failed,last_dsm_id=%d dsm_id=%d", last_dsm_id, dsm_id);
        OAL_IO_PRINT("[E]wifi dsm_client_notify failed,last_dsm_id=%d dsm_id=%d\n", last_dsm_id, dsm_id);

        //retry dmd record
        dsm_client_unocuppy(dsm_client_buf);
        if (!dsm_client_ocuppy(dsm_client_buf)) {
            dsm_client_record(dsm_client_buf, buf);
            dsm_client_notify(dsm_client_buf, dsm_id);
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "wifi dsm_client_notify success,dsm_id=%d", dsm_id);
            OAL_IO_PRINT("[I]wifi dsm notify success, dsm_id=%d[%s]\n", dsm_id, buf);
        } else {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "wifi dsm client ocuppy, dsm notify failed, dsm_id=%d", dsm_id);
            OAL_IO_PRINT("[E]wifi dsm client ocuppy, dsm notify failed, dsm_id=%d\n", dsm_id);
        }
    }
    oal_spin_unlock_irq_restore(&g_dsm_lock, &flags);
}
EXPORT_SYMBOL(hw_1103_dsm_client_notify);

#endif

/*
 * Prototype    : check_bfg_state_etc
 * Description  : check whether bfg is sleep or not
 * Return       : 0  -  sleep
 *                1  -  active
 *                -1 -  pm_data is null
 */
int32 check_bfg_state_etc(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    return pm_data->bfgx_dev_state;
}

STATIC void host_allow_devslp_in_node(struct ps_core_s *ps_core_d)
{
    /* make "host_allow_bfg_sleep_etc()" happy */
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is null\n");
        return;
    }

    atomic_dec(&ps_core_d->node_visit_flag);
    if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
        PS_PRINT_INFO("queue_work send_allow_sleep_work not return true\n");
    }
    /* recovery the original value */
    atomic_inc(&ps_core_d->node_visit_flag);
}

void bfgx_state_set_etc(uint8 on)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }
    PS_PRINT_WARNING("bfgx_state_set_etc:%d --> %d\n", pm_data->bfgx_dev_state, on);
    pm_data->bfgx_dev_state = on;
}

int32 bfgx_state_get_etc(void)
{
    return check_bfg_state_etc();
}

STATIC void bfgx_uart_state_set(uint8 uart_state)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

    PS_PRINT_WARNING("bfgx_uart_state_set:%d-->%d", pm_data->uart_state, uart_state);
    pm_data->uart_state = uart_state;
}

int8 bfgx_uart_state_get(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -1;
    }

    return pm_data->uart_state;
}

int32 bfgx_uart_get_baud_rate(void)
{
    struct ps_plat_s *ps_plat_d = NULL;

    ps_get_plat_reference_etc(&ps_plat_d);
    if (unlikely(ps_plat_d == NULL)) {
        PS_PRINT_ERR("ps_plat_d is NULL\n");
        return -EINVAL;
    }

    return ps_plat_d->baud_rate;
}

void bfgx_uart_baud_change_work(struct work_struct *work)
{
    int ret;
    uint32 wait_cnt = 0;
    const uint32 ul_max_wait_cnt = 10000;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    struct ps_core_s *ps_core_d = NULL;
    uint64 flags;

    PS_PRINT_INFO("%s\n", __func__);

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core has not inited\n");
        return;
    }

    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    bfgx_uart_state_set(UART_BPS_CHG_SEND_ACK);
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    /* 切波特率时，此消息是host发送的最后一个UART数据 */
    ps_tx_urgent_cmd(ps_core_d, SYS_MSG, PL_BAUT_CHG_REQ_ACK);

    while (pm_data->uart_state != UART_BPS_CHG_IN_PROGRESS) {
        oal_udelay(200);
        wait_cnt++;
        if (wait_cnt >= ul_max_wait_cnt) {
            PS_PRINT_ERR("wait device start baud change timeout\n");
            spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
            bfgx_uart_state_set(UART_READY);
            spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

            return;
        }
    }

    ret = ps_change_uart_baud_rate_etc(pm_data->uart_baud_switch_to, FLOW_CTRL_ENABLE);
    if (ret != 0) {
        PS_PRINT_ERR("It is bad!!!, change uart rate fail\n");
    }

    /* 等待device完成切换，pull down GPIO */
    while (board_get_bwkup_gpio_val_etc() == 1) {
        oal_udelay(200);
        wait_cnt++;
        if (wait_cnt >= ul_max_wait_cnt) {
            PS_PRINT_ERR("wait device bps change complete && pull down gpio fail\n");
            return;
        }
    }

    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    bfgx_uart_state_set(UART_BPS_CHG_SEND_COMPLETE);
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    /* 切波特率完成后的第一个握手消息 */
    ps_tx_urgent_cmd(ps_core_d, SYS_MSG, PL_BAUT_CHG_COMPLETE);

    mod_timer(&pm_data->baud_change_timer, jiffies + msecs_to_jiffies(100));

    return;
}

int32 bfgx_uart_rcv_baud_change_req(uint8 uc_msg_type)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    int32 cur_rate;

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    PS_PRINT_INFO("%s\n", __func__);

    cur_rate = bfgx_uart_get_baud_rate();
    if (((uc_msg_type == SYS_INF_BAUD_CHG_2M_REQ) && (cur_rate == HIGH_FREQ_BAUD_RATE)) ||
        ((uc_msg_type == SYS_INF_BAUD_CHG_6M_REQ) && (cur_rate == LOW_FREQ_BAUD_RATE))) {
        if (uc_msg_type == SYS_INF_BAUD_CHG_6M_REQ) {
            pm_data->uart_baud_switch_to = HIGH_FREQ_BAUD_RATE;
        } else {
            pm_data->uart_baud_switch_to = LOW_FREQ_BAUD_RATE;
        }
        queue_work(pm_data->wkup_dev_workqueue, &pm_data->baud_change_work);
    } else {
        PS_PRINT_ERR("It is bad!!!, req = 0x%x,HIGH_FREQ_BAUD_RATE=%d,current = 0x%x\n",
                     HIGH_FREQ_BAUD_RATE, uc_msg_type, cur_rate);
    }
    PS_PRINT_INFO("uart_baud_switch_to baudrate %d\n", pm_data->uart_baud_switch_to);
    return 0;
}

int32 bfgx_uart_rcv_baud_change_complete_ack(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    struct ps_core_s *ps_core_d = NULL;

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core has not inited\n");
        return -EINVAL;
    }

    PS_PRINT_INFO("%s\n", __func__);
    del_timer_sync(&pm_data->baud_change_timer);

    /* restart the tx work */
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
void bfgx_uart_baud_change_expire(uint64 data)
#else
void bfgx_uart_baud_change_expire(struct timer_list *t)
#endif
{
    uint64 flags;
    struct ps_core_s *ps_core_d = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    struct pm_drv_data *pm_data = (struct pm_drv_data *)(uintptr_t)data;
#else
    struct pm_drv_data *pm_data = from_timer(pm_data, t, baud_change_timer);
#endif

    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("devack timer para is null\n");
        return;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core has not inited\n");
        return;
    }

    PS_PRINT_INFO("%s\n", __func__);
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    if (pm_data->uart_state == UART_BPS_CHG_SEND_COMPLETE) {
        ps_tx_urgent_cmd(ps_core_d, SYS_MSG, PL_BAUT_CHG_COMPLETE);
        mod_timer(&pm_data->baud_change_timer, jiffies + msecs_to_jiffies(100));
    }
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
}

int32 bfgx_pm_feature_set_etc(void)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -FAILURE;
    }

    if (pm_data->bfgx_pm_ctrl_enable == BFGX_PM_DISABLE) {
        PS_PRINT_INFO("bfgx platform pm ctrl disable\n");
        msleep(50);
        return SUCCESS;
    }

    if (pm_data->bfgx_lowpower_enable == BFGX_PM_ENABLE) {
        PS_PRINT_INFO("bfgx platform pm enable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_PL_ENABLE_PM);
    } else {
        PS_PRINT_INFO("bfgx platform pm disable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_PL_DISABLE_PM);
    }

    if (pm_data->bfgx_bt_lowpower_enable == BFGX_PM_ENABLE) {
        PS_PRINT_INFO("bfgx bt pm enable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_BT_ENABLE_PM);
    } else {
        PS_PRINT_INFO("bfgx bt pm disable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_BT_DISABLE_PM);
    }

    if (pm_data->bfgx_gnss_lowpower_enable == BFGX_PM_ENABLE) {
        PS_PRINT_INFO("bfgx gnss pm enable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_GNSS_ENABLE_PM);
    } else {
        PS_PRINT_INFO("bfgx gnss pm disable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_GNSS_DISABLE_PM);
    }

    if (pm_data->bfgx_nfc_lowpower_enable == BFGX_PM_ENABLE) {
        PS_PRINT_INFO("bfgx nfc pm enable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_NFC_ENABLE_PM);
    } else {
        PS_PRINT_INFO("bfgx nfc pm disable\n");
        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_NFC_DISABLE_PM);
    }

    msleep(20);

    return SUCCESS;
}

/*
 * Prototype    : bfg_wake_lock_etc
 * Description  : control bfg wake lock
 * Input        : uint8 on: 0 means wake unlock ,1 means wake lock.
 */
void bfg_wake_lock_etc(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    oal_wakelock_stru *pst_bfg_wake_lock;
    oal_ulong ul_flags;

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    pst_bfg_wake_lock = &pm_data->bfg_wake_lock_etc;

    oal_spin_lock_irq_save(&pst_bfg_wake_lock->lock, &ul_flags);
    if (oal_wakelock_active(pst_bfg_wake_lock) == 0) {
        __pm_stay_awake(&pst_bfg_wake_lock->st_wakelock);
        pst_bfg_wake_lock->locked_addr = (uintptr_t)_RET_IP_;
        pst_bfg_wake_lock->lock_count++;
        if (OAL_UNLIKELY(pst_bfg_wake_lock->debug)) {
            printk(KERN_INFO "wakelock[%s] lockcnt:%lu <==%pf\n",
                   pst_bfg_wake_lock->st_wakelock.name, pst_bfg_wake_lock->lock_count, (oal_void *)_RET_IP_);
        }
#ifdef CONFIG_HISI_IDLE_SLEEP
        hisi_idle_sleep_vote(ID_GPS, 1);
        PS_PRINT_INFO("hisi_idle_sleep_vote 1!\n");
#endif
        PS_PRINT_INFO("bfg_wakelock active[%d],cnt %lu\n",
                      oal_wakelock_active(pst_bfg_wake_lock), pst_bfg_wake_lock->lock_count);
    }
    oal_spin_unlock_irq_restore(&pst_bfg_wake_lock->lock, &ul_flags);
#endif

    return;
}

void bfg_wake_unlock_etc(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    oal_wakelock_stru *pst_bfg_wake_lock;
    oal_ulong ul_flags;

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    pst_bfg_wake_lock = &pm_data->bfg_wake_lock_etc;

    oal_spin_lock_irq_save(&pst_bfg_wake_lock->lock, &ul_flags);

    if (oal_wakelock_active(pst_bfg_wake_lock)) {
        pst_bfg_wake_lock->lock_count--;
        __pm_relax(&pst_bfg_wake_lock->st_wakelock);
        pst_bfg_wake_lock->locked_addr = (oal_ulong)0x0;

        if (OAL_UNLIKELY(pst_bfg_wake_lock->debug)) {
            printk(KERN_INFO "wakeunlock[%s] lockcnt:%lu <==%pf\n", pst_bfg_wake_lock->st_wakelock.name,
                   pst_bfg_wake_lock->lock_count, (oal_void *)_RET_IP_);
        }
#ifdef CONFIG_HISI_IDLE_SLEEP
        hisi_idle_sleep_vote(ID_GPS, 0);
        PS_PRINT_INFO("hisi_idle_sleep_vote 0!\n");
#endif
        PS_PRINT_INFO("bfg_wakelock active[%d], cnt %lu\n",
                      oal_wakelock_active(pst_bfg_wake_lock), pst_bfg_wake_lock->lock_count);
    } else {
        PS_PRINT_INFO("bfg_wakelock not active,cnt %lu\n", pst_bfg_wake_lock->lock_count);
    }
    oal_spin_unlock_irq_restore(&pst_bfg_wake_lock->lock, &ul_flags);
#endif

    return;
}

static inline int32 ps_change_baud_rate_retry(int64 baud_rate, uint8 flowctl_status)
{
    const uint32 set_baud_retry = 3;
    int ret = OAL_TRUE;
    uint32 j = set_baud_retry;
    while (ps_change_uart_baud_rate_etc(baud_rate, flowctl_status) != 0) {
        PS_PRINT_WARNING("change uart rate fail,left retry cnt:%d,do retry\n", j);
        DECLARE_DFT_TRACE_KEY_INFO("change uart rate fail", OAL_DFT_TRACE_FAIL);
        if (--j) {
            msleep(100);
        } else {
            PS_PRINT_ERR("change uart rate %ld fail,retried %u but not succ\n", baud_rate, set_baud_retry);
            ret = OAL_FALSE;
            break;
        }
    }

    return ret;
}

static int32 process_host_wkup_dev_fail(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data)
{
    uint64 flags;
    int bwkup_gpio_val;

    if (!OAL_IS_ERR_OR_NULL(ps_core_d->tty) && tty_chars_in_buffer(ps_core_d->tty)) {
        PS_PRINT_INFO("tty tx buf is not empty\n");
    }

    bwkup_gpio_val = board_get_bwkup_gpio_val_etc();
    PS_PRINT_INFO("bfg still NOT wkup, gpio level:%d\n", bwkup_gpio_val);

    if (bwkup_gpio_val == 0) {
        DECLARE_DFT_TRACE_KEY_INFO("bfg wakeup fail", OAL_DFT_TRACE_EXCEP);
        if (is_hi110x_debug_type() != OAL_TRUE) {
            PS_PRINT_INFO("user mode or maybe beta user,ssi dump bypass\n");
        } else {
            (void)ssi_dump_err_regs(SSI_ERR_BFG_WAKE_UP_FAIL);
        }
        return OAL_FALSE;
    } else {
        PS_PRINT_INFO("bfg wakeup ack lost, complete it\n");
        spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
        bfg_wake_lock_etc();
        bfgx_state_set_etc(BFGX_ACTIVE);
        complete(&pm_data->dev_ack_comp);
        spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);
#ifndef _PRE_H2D_GPIO_WKUP
        // set default baudrate. should not try again if failed, return succ
        (void)ps_change_baud_rate_retry(default_baud_rate, FLOW_CTRL_ENABLE);
#endif
        queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

#ifdef _PRE_H2D_GPIO_WKUP
static oal_int32 host_wkup_dev_via_gpio(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data)
{
    const uint32 wkup_retry = 3;
    int32 ret;
    uint64 timeleft;
    int bwkup_gpio_val = 0;
    uint32 i;

    for (i = 0; i < wkup_retry; i++) {
        bwkup_gpio_val = board_get_bwkup_gpio_val_etc();
        PS_PRINT_INFO("bfg wakeup dev,try %u,cur gpio level:%u\n", i + 1, bwkup_gpio_val);

        board_host_wakeup_bfg_set(GPIO_HIGHLEVEL);
        mdelay(1);
        board_host_wakeup_bfg_set(GPIO_LOWLEVEL);

        timeleft = wait_for_completion_timeout(&pm_data->dev_ack_comp,
                                               msecs_to_jiffies(WAIT_WKUP_DEVACK_TIMEOUT_MSEC));
        if (timeleft || (bfgx_state_get_etc() == BFGX_ACTIVE)) {
            bwkup_gpio_val = board_get_bwkup_gpio_val_etc();
            PS_PRINT_INFO("bfg wkup OK, gpio level:%d\n", bwkup_gpio_val);

            return OAL_TRUE;
        } else {
            ret = process_host_wkup_dev_fail(ps_core_d, pm_data);
            if (ret == OAL_TRUE) {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}
#else
static oal_int32 host_wkup_dev_via_uart(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data)
{
    const uint32 wkup_retry = 3;
    int32 ret;
    uint64 timeleft;
    uint8 zero_num = 0;
    int bwkup_gpio_val = 0;
    uint32 i;

    // prepare baudrate
    ret = ps_change_baud_rate_retry(WKUP_DEV_BAUD_RATE, FLOW_CTRL_DISABLE);
    if (ret != OAL_TRUE) {
        return OAL_FALSE;
    }

    for (i = 0; i < wkup_retry; i++) {
        bwkup_gpio_val = board_get_bwkup_gpio_val_etc();
        PS_PRINT_INFO("bfg wakeup dev,try %u,cur gpio level:%u\n", i + 1, bwkup_gpio_val);
        /* uart write long zero to wake up device */
        ps_write_tty_etc(ps_core_d, &zero_num, sizeof(uint8));

        timeleft = wait_for_completion_timeout(&pm_data->dev_ack_comp,
                                               msecs_to_jiffies(WAIT_WKUP_DEVACK_TIMEOUT_MSEC));
        if (timeleft || (bfgx_state_get_etc() == BFGX_ACTIVE)) {
            bwkup_gpio_val = board_get_bwkup_gpio_val_etc();
            PS_PRINT_INFO("bfg wkup OK, gpio level:%d\n", bwkup_gpio_val);

            // set default baudrate. should not try again if failed, return succ
            (void)ps_change_baud_rate_retry(default_baud_rate, FLOW_CTRL_ENABLE);
            return OAL_TRUE;
        } else {
            ret = process_host_wkup_dev_fail(ps_core_d, pm_data);
            if (ret == OAL_TRUE) {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}
#endif

static void host_wakeup_dev(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data)
{
    int ret;

#ifdef _PRE_H2D_GPIO_WKUP
    ret = host_wkup_dev_via_gpio(ps_core_d, pm_data);
    if (ret != OAL_TRUE) {
        PS_PRINT_ERR("host gpio wkup bfg fail\n");
    }
#else
    /* begin to wake up device via uart rxd */
    ret = host_wkup_dev_via_uart(ps_core_d, pm_data);
    if (ret != OAL_TRUE) {
        ps_change_uart_baud_rate_etc(default_baud_rate, FLOW_CTRL_ENABLE);
        PS_PRINT_INFO("host wkup bfg fail\n");
    }
#endif
}

/*
 * Prototype    : bfg_sleep_wakeup
 * Description  : function for bfg device wake up
 */
void host_wkup_dev_work_etc(struct work_struct *work)
{
    uint64 timeleft;
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;

    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

    PS_PRINT_INFO("%s,dev:%d,uart:%d\n", __func__, bfgx_state_get_etc(), bfgx_uart_get_baud_rate());

    ret = ps_get_core_reference_etc(&ps_core_d);
    if ((ret != SUCCESS) || (ps_core_d == NULL)) {
        PS_PRINT_ERR("%s,ps_core_d is null!\n", __func__);
        return;
    }

    if (is_tty_open(ps_core_d->pm_data)) {
        PS_PRINT_ERR("%s,tty is closed skip!\n", __func__);
        return;
    }
    /* if B send work item of wkup_dev before A's work item finished, then
     * B should not do actual wkup operation.
     */
    if (bfgx_state_get_etc() == BFGX_ACTIVE) {
        if (waitqueue_active(&pm_data->host_wkup_dev_comp.wait)) {
            PS_PRINT_INFO("it seems like dev ack with NoSleep\n");
        } else { /* 目前用了一把host_mutex大锁，这种case不应存在，但低功耗模块不应依赖外部 */
            PS_PRINT_INFO("B do wkup_dev work item after A do it but not finished\n");
        }
        complete_all(&pm_data->host_wkup_dev_comp);
        return;
    }

    /* prepare to wake up device */
    ps_uart_state_pre_etc(ps_core_d->tty);
    timeleft = wait_for_completion_timeout(&pm_data->dev_ack_comp, msecs_to_jiffies(WAIT_DEVACK_TIMEOUT_MSEC));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait dev allow slp ack timeout\n");
        DECLARE_DFT_TRACE_KEY_INFO("wait dev allow slp ack timeout", OAL_DFT_TRACE_FAIL);
        return;
    }

    INIT_COMPLETION(pm_data->dev_ack_comp);

    /* device doesn't agree to slp */
    if (bfgx_state_get_etc() == BFGX_ACTIVE) {
        complete_all(&pm_data->host_wkup_dev_comp);
        PS_PRINT_DBG("we know dev ack with NoSleep\n");
        return;
    }

    host_wakeup_dev(ps_core_d, pm_data);
}

static int32 bfg_wait_tty_resume(struct ps_core_s *ps_core_d)
{
#define MAX_TTYRESUME_LOOPCNT 300
#define MAX_SENSORHUB_LOOPCNT 30
#ifdef ASYNCB_SUSPENDED
    uint32 loop_tty_resume_cnt = 0;
#endif
#ifdef CONFIG_INPUTHUB
    uint32 loop_sensorhub_resume_cnt = 0;
#endif

    if ((ps_core_d->tty) && (ps_core_d->tty->port)) {
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
        while (tty_port_suspended(ps_core_d->tty->port)) {
            if (loop_tty_resume_cnt++ >= MAX_TTYRESUME_LOOPCNT) {
                PS_PRINT_ERR("tty is not ready, state:%d!\n", tty_port_suspended(ps_core_d->tty->port));
                return OAL_FALSE;
            }
            msleep(10);
        }
        PS_PRINT_INFO("tty state: 0x%x ,loop_tty_resume_cnt:%d\n",
                      tty_port_suspended(ps_core_d->tty->port), loop_tty_resume_cnt);
#else
        PS_PRINT_INFO("tty port flag 0x%x\n", (unsigned int)ps_core_d->tty->port->flags);
#ifdef ASYNCB_SUSPENDED
        while (test_bit(ASYNCB_SUSPENDED, (volatile unsigned long *)&(ps_core_d->tty->port->flags))) {
            if (loop_tty_resume_cnt++ >= MAX_TTYRESUME_LOOPCNT) {
                PS_PRINT_ERR("tty is not ready, flag is 0x%x!\n", (unsigned int)ps_core_d->tty->port->flags);
                return OAL_FALSE;
            }
            msleep(10);
        }
#endif
#endif

#ifdef CONFIG_INPUTHUB
        if (get_uart_pclk_source_etc() == UART_PCLK_FROM_SENSORHUB) {
            /* 查询sensorhub状态，如果不是wkup状态，uart的时钟可能会不对 */
            if (getSensorMcuMode() == 1) {
                PS_PRINT_INFO("sensorbub state is %d\n", get_iomcu_power_state());
                /* 0,1->ST_POWERON,8->ST_SLEEP,9->ST_WAKEUP */
                while ((get_iomcu_power_state() != ST_WAKEUP) && (get_iomcu_power_state() != ST_POWERON_OTHER) &&
                       (get_iomcu_power_state() != ST_POWERON)) {
                    if (loop_sensorhub_resume_cnt++ >= MAX_SENSORHUB_LOOPCNT) {
                        PS_PRINT_ERR("sensorhub not wakeup yet, state is %d\n", get_iomcu_power_state());
                        return OAL_FALSE;
                    }
                    msleep(10);
                }
            }
        }
#endif
    } else {
        PS_PRINT_ERR("tty has not inited\n");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

#ifdef CONFIG_INPUTHUB
/* 麒麟内核函数，先用内核版本宏隔开 */
/* sensorbub模块的函数，睡眠唤醒时用来查询sensorhub的状态 */
extern int getSensorMcuMode(void);
extern int get_iomcu_power_state(void);
#endif

void host_send_disallow_msg_etc(struct work_struct *work)
{

    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    uint64 flags;

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

    PS_PRINT_INFO("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core has not inited\n");
        return;
    }

    /*
     * 防止host睡眠情况下被dev唤醒进入gpio中断后直接在这里下发消息，
     * 此时uart可能还没有ready,所以这里等待tty resume之后才下发消息
     */
    if (bfg_wait_tty_resume(ps_core_d) == OAL_FALSE) {
        return;
    }

    /* 发生过如下case: 在提交disallow sleep work前work queque中残留了一个allow sleep work未处理，
    导致disallow消息发送的时候，由于allow sleep消息先发下去了，芯片已经睡眠，disallow消息未发送成功 */
    if (bfgx_state_get_etc() != BFGX_ACTIVE) {
        PS_PRINT_ERR("device not in active state, no msg could send,commit a wakeup work.\n");
        queue_work(pm_data->wkup_dev_workqueue, &pm_data->wkup_dev_work);
        return;
    }

    /* 设置uart可用,下发disallow sleep消息,唤醒完成 */
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    bfgx_uart_state_set(UART_READY);
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    if (oal_atomic_read(&ir_only_mode) == 0) {
        if (ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_DISALLOW_SLP) != 0) {
            PS_PRINT_INFO("SYS_CFG_DISALLOW_SLP MSG send fail, retry\n");
            msleep(10);
            queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);
            return;
        }
    }

    /*
     * 这里设置完成量对于dev wkup host没有意义, 只是保证和host wkup dev的操作一致
     * 注意这就要求host wkup dev前需要INIT完成量计数
     */
    complete_all(&pm_data->host_wkup_dev_comp);

    if (oal_atomic_read(&ir_only_mode) == 0) {
        /* if any of BFNI is open, we should mod timer. */
        if ((!bfgx_other_subsys_all_shutdown_etc(BFGX_GNSS)) ||
            (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP)) {
            mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
            pm_data->bfg_timer_mod_cnt++;
            PS_PRINT_INFO("mod_timer:host_send_disallow_msg_etc\n");
        }
        ps_core_d->ps_pm->operate_beat_timer(BEAT_TIMER_RESET);
    }

    if (atomic_read(&pm_data->bfg_needwait_devboot_flag) == NEED_SET_FLAG) {
        complete(&pm_data->dev_bootok_ack_comp);
    }
}

static int32 agree_allow_bfg_sleep(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data, uint64 flags)
{
    /* if someone is visiting the dev_node */
    if (atomic_read(&ps_core_d->node_visit_flag) > 0) {
        PS_PRINT_INFO("someone visit node, not send allow sleep msg\n");
        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
        /* gnss write do NOT mod timer */
        mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
        pm_data->bfg_timer_mod_cnt++;
        return OAL_FALSE;
    }

    if ((atomic_read(&pm_data->gnss_sleep_flag) != GNSS_AGREE_SLEEP) ||
        (ps_chk_tx_queue_empty(ps_core_d) == false)) {
        PS_PRINT_INFO("tx queue not empty, not send allow sleep msg\n");
        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
        mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
        pm_data->bfg_timer_mod_cnt++;
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

void host_allow_bfg_sleep_etc(struct work_struct *work)
{
    uint64 flags;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

    PS_PRINT_INFO("%s\n", __func__);

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core has not inited\n");
        return;
    }

    if (ps_core_d->tty_have_open == false) {
        PS_PRINT_INFO("tty has closed, not send msg to dev\n");
        return;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return;
    }
    if (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE) {
        PS_PRINT_ERR("plat is doing dfr not allow sleep\n");
        mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
        pm_data->bfg_timer_mod_cnt++;
        return;
    }

    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);

    if (agree_allow_bfg_sleep(ps_core_d, pm_data, flags) != OAL_TRUE) {
        return;
    }

    /* 设置device状态为睡眠态，在host唤醒dev完成之前(或dev唤醒host前)uart不可用 */
    PS_PRINT_INFO("%s,set UART_NOT_READY,BFGX_SLEEP\n", __func__);
    ps_core_d->ps_pm->bfgx_uart_state_set(UART_NOT_READY);
    ps_core_d->ps_pm->bfgx_dev_state_set(BFGX_SLEEP);
    /* clear mod cnt */
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    mod_timer(&pm_data->dev_ack_timer, jiffies + (WAIT_DEVACK_MSEC * HZ / 1000));

    /*
     * we need reinit completion cnt as 0, to prevent such case:
     * 1)host allow dev sleep, dev ack with OK, cnt=1,
     * 2)device wkup host,
     * 3)host allow dev sleep,
     * 4)host wkup dev, it will wait dev_ack succ immediately since cnt==1,
     * 5)dev ack with ok, cnt=2,
     * this case will cause host wait dev_ack invalid.
     */
    INIT_COMPLETION(pm_data->dev_ack_comp);

    if (ps_tx_urgent_cmd(ps_core_d, SYS_MSG, SYS_CFG_ALLOWDEV_SLP) != 0) {
        PS_PRINT_INFO("SYS_CFG_ALLOWDEV_SLP MSG send fail\n");
    }
#ifdef _PRE_HI_DRV_GPIO
    else {
        hitv_gpio_direction_output(board_info_etc.host_wakeup_bfg, GPIO_LOWLEVEL);
    }
#endif
}

/*
 * Prototype    : bfg_check_timer_work
 * Description  : check bfg timer is work fine
 * input        : ps_core_d
 */
void bfg_check_timer_work(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return;
    }

    /* 10s后没有人启动bfg timer 补救:直接提交allow to sleep work */
    if ((pm_data->bfg_timer_mod_cnt_pre == pm_data->bfg_timer_mod_cnt) && (pm_data->bfg_timer_mod_cnt != 0)
        && (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP)) {
        if (time_after(jiffies, pm_data->bfg_timer_check_time)) {
            DECLARE_DFT_TRACE_KEY_INFO("bfg_timer not work in 10s", OAL_DFT_TRACE_FAIL);
            if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
                PS_PRINT_INFO("queue_work send_allow_sleep_work not return true\n");
            } else {
                PS_PRINT_INFO("timer_state(%d),queue_work send_allow_sleep_work succ\n",
                              timer_pending(&pm_data->bfg_timer));
            }
        }
    } else {
        pm_data->bfg_timer_mod_cnt_pre = pm_data->bfg_timer_mod_cnt;
        pm_data->bfg_timer_check_time = jiffies + msecs_to_jiffies(PL_CHECK_TIMER_WORK);
    }
}

static int32 bfg_timer_expire_param_check(struct pm_drv_data *pm_data)
{
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is null\n");
        return OAL_FALSE;
    }

    if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
        PS_PRINT_DBG("lowpower function disabled\n");
        return OAL_FALSE;
    }
    if (pm_data->ps_pm_interface->bfgx_dev_state_get() == BFGX_SLEEP) {
        PS_PRINT_DBG("dev has been sleep\n");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*
 * Prototype    : bfg_timer_expire_etc
 * Description  : bfg timer expired function
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
void bfg_timer_expire_etc(uint64 data)
#else
void bfg_timer_expire_etc(struct timer_list *t)
#endif
{
    struct ps_core_s *ps_core_d = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    struct pm_drv_data *pm_data = (struct pm_drv_data *)(uintptr_t)data;
#else
    struct pm_drv_data *pm_data = from_timer(pm_data, t, bfg_timer);
#endif

    if (bfg_timer_expire_param_check(pm_data) == OAL_FALSE) {
        return;
    }

    ps_core_d = pm_data->ps_pm_interface->ps_core_data;
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return;
    }

    if (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP &&
        (pm_data->uart_state < UART_BPS_CHG_SEND_ACK)) {
        if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
            PS_PRINT_INFO("queue_work send_allow_sleep_work not return true\n");
        }
        pm_data->gnss_votesleep_check_cnt = 0;
        pm_data->rx_pkt_gnss_pre = 0;
    } else if (pm_data->uart_state < UART_BPS_CHG_SEND_ACK) {
        /* GNSS NOT AGREE SLEEP ,Check it */
        if (pm_data->rx_pkt_gnss_pre != ps_core_d->rx_pkt_num[BFGX_GNSS]) {
            pm_data->rx_pkt_gnss_pre = ps_core_d->rx_pkt_num[BFGX_GNSS];
            pm_data->gnss_votesleep_check_cnt = 0;

            mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
            pm_data->bfg_timer_mod_cnt++;
        } else {
            pm_data->gnss_votesleep_check_cnt++;
            if (pm_data->gnss_votesleep_check_cnt >= PL_CHECK_GNSS_VOTE_CNT) {
                PS_PRINT_ERR("gnss_votesleep_check_cnt %d,set GNSS_AGREE_SLEEP\n", pm_data->gnss_votesleep_check_cnt);
                atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
                queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work);

                pm_data->gnss_votesleep_check_cnt = 0;
                pm_data->rx_pkt_gnss_pre = 0;
            } else {
                mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
                pm_data->bfg_timer_mod_cnt++;
            }
        }
    } else {
        PS_PRINT_INFO("uart_state %d\n", pm_data->uart_state);
        mod_timer(&pm_data->bfg_timer, jiffies + (PLATFORM_SLEEP_TIME * HZ / 1000));
        pm_data->bfg_timer_mod_cnt++;
    }
}

int32 host_wkup_dev_etc(void)
{
    uint64 timeleft;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    struct ps_core_s *ps_core_d = NULL;
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }
    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d ot tty is NULL\n");
        return -EINVAL;
    }
    if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
        return 0;
    }
    PS_PRINT_DBG("wkup start\n");

    INIT_COMPLETION(pm_data->host_wkup_dev_comp);
    queue_work(pm_data->wkup_dev_workqueue, &pm_data->wkup_dev_work);
    ps_uart_state_pre_etc(ps_core_d->tty);
    timeleft = wait_for_completion_timeout(&pm_data->host_wkup_dev_comp, msecs_to_jiffies(WAIT_WKUPDEV_MSEC));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        PS_PRINT_ERR("wait wake up dev timeout\n");
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PM, CHR_PLAT_DRV_ERROR_BFG_WKUP_DEV);

        return -ETIMEDOUT;
    }
    PS_PRINT_DBG("wkup over\n");

    return 0;
}

/*
 * Prototype    : bfgx_other_subsys_all_shutdown_etc
 * Description  : check other subsystem is shutdown or not
 * Input        : uint8 type: means one system to check
 * Return       : 0 - other subsystem are all shutdown
 *                1 - other subsystem are not all shutdown
 */
int32 bfgx_other_subsys_all_shutdown_etc(uint8 subsys)
{
    int32 i = 0;
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    for (i = 0; i < BFGX_BUTT; i++) {
        if (i == subsys) {
            continue;
        }

        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) == POWER_STATE_OPEN) {
            return false;
        }
    }

    return true;
}

/*
 * Prototype    : bfgx_print_subsys_state
 * Description  : check all sub system state
 */
void bfgx_print_subsys_state(void)
{
    int32 i = 0;
    int32 total;
    int32 count = 0;
    const uint32 ul_print_str_len = 200;
    char print_str[ul_print_str_len];
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return;
    }

    total = 0;

    for (i = 0; i < BFGX_BUTT; i++) {
        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) == POWER_STATE_OPEN) {
            total = snprintf_s(print_str + count, sizeof(print_str) - count, sizeof(print_str) - count - 1,
                               "%s:%s ", ps_core_d->bfgx_info[i].name, "on ");
            if (total < 0) {
                OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
                return;
            }
            count += total;
        } else {
            total = snprintf_s(print_str + count, sizeof(print_str) - count, sizeof(print_str) - count - 1,
                               "%s:%s ", ps_core_d->bfgx_info[i].name, "off");
            if (total < 0) {
                OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
                return;
            }
            count += total;
        }
    }

    PS_PRINT_ERR("%s\n", print_str);
}

void bfgx_gpio_intr_enable(uint32 ul_en)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    uint64 flags;
    spin_lock_irqsave(&pm_data->bfg_irq_spinlock, flags);
    if (ul_en) {
        /* 不再支持中断开关嵌套 */
        if (pm_data->ul_irq_stat) {
            enable_irq(pm_data->bfg_irq);
            pm_data->ul_irq_stat = 0;
        }
    } else {
        if (!pm_data->ul_irq_stat) {
            disable_irq_nosync(pm_data->bfg_irq);
            pm_data->ul_irq_stat = 1;
        }
    }
    spin_unlock_irqrestore(&pm_data->bfg_irq_spinlock, flags);
}

int32 bfgx_dev_power_on_etc(void)
{
    uint64 timeleft;
    int32 error;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    BOARD_INFO *bd_info = NULL;

    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        PS_PRINT_ERR("board info is err\n");
        return BFGX_POWER_FAILED;
    }
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return BFGX_POWER_FAILED;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }
    /* 防止Host睡眠 */
    oal_wake_lock(&pm_data->bfg_wake_lock_etc);

    INIT_COMPLETION(pm_data->dev_bootok_ack_comp);
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);
    error = bd_info->bd_ops.bfgx_dev_power_on_etc();
    if (error != BFGX_POWER_SUCCESS) {
        goto bfgx_power_on_fail;
    }

    ps_uart_state_pre_etc(ps_core_d->tty);
    /* WAIT_BFGX_BOOTOK_TIME:这个时间目前为1s，有1s不够的情况，需要关注 */
    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        ps_uart_state_dump_etc(ps_core_d->tty);
        if (wlan_is_shutdown_etc()) {
            PS_PRINT_ERR("wifi off, wait bfgx boot up ok timeout\n");
            error = BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL;
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP_WITH_WIFI_OFF);

            goto bfgx_power_on_fail;
        } else {
            PS_PRINT_ERR("wifi on, wait bfgx boot up ok timeout\n");
            error = BFGX_POWER_WIFI_ON_BOOT_UP_FAIL;
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP_WITH_WIFI_ON);

            goto bfgx_power_on_fail;
        }
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);

    if (oal_atomic_read(&ir_only_mode) == 0) {
        if (get_hi110x_subchip_type() != BOARD_VERSION_HI1102) {
            if (wlan_is_shutdown_etc()) {
                ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_NOTIFY_WIFI_CLOSE);
            } else {
                ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, SYS_CFG_NOTIFY_WIFI_OPEN);
            }
        }

        bfgx_pm_feature_set_etc();
    }

    return BFGX_POWER_SUCCESS;

bfgx_power_on_fail:
    oal_wake_unlock(&pm_data->bfg_wake_lock_etc);
    return error;
}

/*
 * Prototype    : bfgx_dev_power_off_etc
 * Description  : bfg device power off function
 * Return       : 0 means succeed,-1 means failed
 */
int32 bfgx_dev_power_off_etc(void)
{
    int32 error = SUCCESS;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    BOARD_INFO *bd_info = NULL;
    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        PS_PRINT_ERR("board info is err\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -FAILURE;
    }

    /* 单红外没有心跳 */
    if (oal_atomic_read(&ir_only_mode) == 0) {
        PS_PRINT_INFO("delete sleep timer and work!\n");
        pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
        cancel_work_sync(&pm_data->send_allow_sleep_work);
        del_timer_sync(&pm_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;
    }

    /* 下电即将完成，需要在此时设置下次上电要等待device上电成功的flag */
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);
    bd_info->bd_ops.bfgx_dev_power_off_etc();

    ps_core_d->rx_pkt_sys = 0;
    ps_core_d->rx_pkt_oml = 0;

    bfg_wake_unlock_etc();

    return error;
}

/*
 * Prototype    : bfgx_dev_power_control_etc
 * Description  : bfg power control function
 * Input        : int32 flag: 1 means on, 0 means off
 *                uint8 type: means one of bfg
 * Return       : 0 means succeed,-1 means failed
 */
int32 bfgx_dev_power_control_etc(uint8 subsys, uint8 flag)
{
    int32 ret = 0;

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    if ((subsys != BFGX_GNSS)) {
        PS_PRINT_ERR("gnss only only not support subs:%d\n", subsys);
        return -FAILURE;
    }
#endif

    if (flag == BFG_POWER_GPIO_UP) {
        ret = bfgx_power_on_etc(subsys);
        if (ret) {
            PS_PRINT_ERR("bfgx power on is error!\n");
        }
    } else if (flag == BFG_POWER_GPIO_DOWN) {
        ret = bfgx_power_off_etc(subsys);
        if (ret) {
            PS_PRINT_ERR("bfgx power off is error!\n");
        }
    } else {
        PS_PRINT_ERR("invalid input data!\n");
        ret = -FAILURE;
    }

    return ret;
}

static void firmware_download_fail_proc(struct pm_drv_data *pm_data, int which_cfg)
{
    PS_PRINT_ERR("firmware download fail!\n");
    DECLARE_DFT_TRACE_KEY_INFO("patch_download_fail", OAL_DFT_TRACE_FAIL);
    if (which_cfg == BFGX_CFG) {
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_BFG_FW_DOWN);
    } else {
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_WIFI_FW_DOWN);
    }

    if (pm_data->pst_wlan_pm_info->pst_bus->bus_type == HCC_BUS_SDIO) {
        (void)ssi_dump_err_regs(SSI_ERR_FIRMWARE_DOWN_SDIO_FAIL);
    } else {
        (void)ssi_dump_err_regs(SSI_ERR_FIRMWARE_DOWN_FAIL);
    }

    if (hi11xx_get_os_build_variant() != HI1XX_OS_BUILD_VARIANT_USER) {
        /* dump bootloader rw dtcm */
        ssi_read_reg_info_test(HI1103_BOOTLOAD_DTCM_BASE_ADDR, HI1103_BOOTLOAD_DTCM_SIZE, 1, SSI_RW_DWORD_MOD);
    }
}

/*
 * Prototype    : firmware_download_function_etc
 * Description  : download wlan patch
 * Return       : 0 means succeed,-1 means failed
 */
int firmware_download_function_priv(uint32 which_cfg, firmware_downlaod_privfunc priv_func)
{
    int32 ret;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_time = 0;
    static unsigned long long count = 0;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();

    PS_PRINT_INFO("enter firmware_download_function_etc\n");

    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    if (which_cfg >= CFG_FILE_TOTAL) {
        PS_PRINT_ERR("cfg file index [%d] outof range\n", which_cfg);
        return -FAILURE;
    }

    if (pm_data->pst_wlan_pm_info->pst_bus == NULL) {
        PS_PRINT_ERR("pst_bus is null\n");
        return -FAILURE;
    }

    start_time = ktime_get();

    hcc_bus_wake_lock(pm_data->pst_wlan_pm_info->pst_bus);
    hcc_bus_lock(pm_data->pst_wlan_pm_info->pst_bus);

    ret = hcc_bus_reinit(pm_data->pst_wlan_pm_info->pst_bus);
    if (ret != OAL_SUCC) {
        hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        PS_PRINT_ERR("sdio reinit failed, ret:%d!\n", ret);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_FW_SDIO_INIT);
        return -FAILURE;
    }

    wlan_pm_init_dev_etc();

    /* firmware_cfg_init_etc(sdio) function should just be called once */
    if (!test_bit(FIRMWARE_CFG_INIT_OK, &pm_data->firmware_cfg_init_flag)) {
        PS_PRINT_INFO("firmware_cfg_init_etc begin\n");
        ret = firmware_cfg_init_etc();
        if (ret) {
            PS_PRINT_ERR("firmware_cfg_init_etc failed, ret:%d!\n", ret);
            hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
            hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);

            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_CFG_FAIL_FIRMWARE_DOWN);
            return ret;
        }

        PS_PRINT_INFO("firmware_cfg_init_etc OK\n");
        set_bit(FIRMWARE_CFG_INIT_OK, &pm_data->firmware_cfg_init_flag);
    }

    PS_PRINT_INFO("firmware_download_etc begin\n");

    /* do some private command before load cfg */
    if (priv_func != NULL) {
        ret = priv_func();
        if (ret) {
            PS_PRINT_ERR("priv_func=%pf failed, ret:%d!\n", (oal_void *)priv_func, ret);
            hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
            hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);
            return ret;
        }
    }

    ret = firmware_download_etc(which_cfg);

    if (ret < 0) {
        hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        if (ret != -OAL_EINTR) {
            firmware_download_fail_proc(pm_data, which_cfg);
        } else {
            /* download firmware interrupt */
            PS_PRINT_INFO("firmware download interrupt!\n");
            DECLARE_DFT_TRACE_KEY_INFO("patch_download_interrupt", OAL_DFT_TRACE_FAIL);
        }
        return ret;
    }
    DECLARE_DFT_TRACE_KEY_INFO("patch_download_ok", OAL_DFT_TRACE_SUCC);

    hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
    hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);

    PS_PRINT_INFO("firmware_download_etc success\n");

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);

    if (total_time > max_time) {
        max_time = total_time;
    }

    count++;
    PS_PRINT_WARNING("download firmware, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     count, total_time, max_time);

    return SUCCESS;
}

int firmware_download_function_etc(uint32 which_cfg)
{
    return firmware_download_function_priv(which_cfg, NULL);
}

bool wlan_is_shutdown_etc(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return true;
    }

    return ((pm_data->pst_wlan_pm_info->ul_wlan_power_state == POWER_STATE_SHUTDOWN) ? true : false);
}

bool bfgx_is_shutdown_etc(void)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return true;
    }

    return ps_chk_bfg_active_etc(ps_core_d) ? false : true;
}
EXPORT_SYMBOL(bfgx_is_shutdown_etc);

int32 wifi_power_fail_process_etc(int32 error)
{
    int32 ret = WIFI_POWER_FAIL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return ret;
    }

    if (error >= WIFI_POWER_ENUM_BUTT) {
        PS_PRINT_ERR("error is undefined, error=[%d]\n", error);
        return ret;
    }

    PS_PRINT_INFO("wifi power fail, error=[%d]\n", error);

    switch (error) {
        case WIFI_POWER_SUCCESS:
        case WIFI_POWER_PULL_POWER_GPIO_FAIL:
            break;

        /* BFGX off，wifi firmware download fail和wait boot up fail，直接返回失败，上层重试，不走DFR */
        case WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL:
            if (oal_trigger_bus_exception(pm_data->pst_wlan_pm_info->pst_bus, OAL_TRUE) == OAL_TRUE) {
                /* exception is processing, can't power off */
                PS_PRINT_INFO("bfgx off,sdio exception is working\n");
                break;
            }
            PS_PRINT_INFO("bfgx off,set wlan_power_state to shutdown\n");
            oal_wlan_gpio_intr_enable_etc(HBUS_TO_DEV(pm_data->pst_wlan_pm_info->pst_bus), OAL_FALSE);
            pm_data->pst_wlan_pm_info->ul_wlan_power_state = POWER_STATE_SHUTDOWN;

        case WIFI_POWER_ON_FIRMWARE_DOWNLOAD_INTERRUPT:
            ret = -OAL_EINTR;
            /* 被系统中断的异常, 修改返回值类型 */
        case WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL:
        case WIFI_POWER_BFGX_OFF_PULL_WLEN_FAIL:
        case WIFI_POWER_ON_FIRMWARE_FILE_OPEN_FAIL:
            PS_PRINT_INFO("wifi power fail: pull down power on gpio\n");
            board_power_off_etc(WLAN_POWER);
            break;

        /* BFGX on，wifi上电失败，进行全系统复位，wifi本次返回失败，上层重试 */
        case WIFI_POWER_BFGX_ON_BOOT_UP_FAIL:
            if (oal_trigger_bus_exception(pm_data->pst_wlan_pm_info->pst_bus, OAL_TRUE) == OAL_TRUE) {
                /* exception is processing, can't power off */
                PS_PRINT_INFO("bfgx on,sdio exception is working\n");
                break;
            }
            PS_PRINT_INFO("bfgx on,set wlan_power_state to shutdown\n");
            oal_wlan_gpio_intr_enable_etc(HBUS_TO_DEV(pm_data->pst_wlan_pm_info->pst_bus), OAL_FALSE);
            pm_data->pst_wlan_pm_info->ul_wlan_power_state = POWER_STATE_SHUTDOWN;
        case WIFI_POWER_BFGX_DERESET_WCPU_FAIL:
        case WIFI_POWER_BFGX_ON_FIRMWARE_DOWNLOAD_FAIL:
        case WIFI_POWER_BFGX_ON_PULL_WLEN_FAIL:
            if (plat_power_fail_exception_info_set_etc(SUBSYS_WIFI, THREAD_WIFI, WIFI_POWERON_FAIL) ==
                WIFI_POWER_SUCCESS) {
                bfgx_system_reset_etc();
                plat_power_fail_process_done_etc();
            } else {
                PS_PRINT_ERR("wifi power fail, set exception info fail\n");
            }
            break;

        default:
            PS_PRINT_ERR("error is undefined, error=[%d]\n", error);
            break;
    }

    return ret;
}

int32 wifi_notify_bfgx_status(uint8 ucStatus)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -EFAULT;
        ;
    }

    if (!bfgx_is_shutdown_etc()) {
        ret = prepare_to_visit_node_etc(ps_core_d);
        if (ret < 0) {
            PS_PRINT_ERR("prepare work fail, bring to reset work\n");
            plat_exception_handler_etc(SUBSYS_BFGX, THREAD_IDLE, BFGX_WAKEUP_FAIL);
            return ret;
        }

        ps_tx_sys_cmd_etc(ps_core_d, SYS_MSG, ucStatus);

        post_to_visit_node_etc(ps_core_d);
    }

    return 0;
}

/* 共享外置buck上下电接口 */
void BuckPowerCtrl(uint8 enable, uint8 subsys)
{
#ifdef _PRE_SHARE_BUCK_SURPORT
#define BUCK_1_P15_V    1150000
#define BUCK_1_P05_V    1050000
#ifndef WBG_PMIC_REQ
#define WBG_PMIC_REQ 8
#endif

    if (subsys > PLAT_SUBSYS) {
        return ;
    }

#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
    if (buck_mode_get() == EXT_BUCK_HOST_CTL) {
        struct hw_comm_pmic_cfg_t fpPmicLdoSet;
        uint8 high_vset = OAL_TRUE;

        if (enable) {
            board_info_etc.buck_control_flag |= (1 << subsys);
        } else {
            board_info_etc.buck_control_flag &= ~(1 << subsys);
        }

        /* 仅平台默认上电，或者WIFI/BT打开，1.15v */
        if ((board_info_etc.buck_control_flag == HI110X_PLAT_SUB_MASK) ||
            (board_info_etc.buck_control_flag & HI110X_BT_SUB_MASK) ||
            (board_info_etc.buck_control_flag & HI110X_WIFI_SUB_MASK)) {
            high_vset = OAL_TRUE;
        } else {
            high_vset = OAL_FALSE;
        }

        fpPmicLdoSet.pmic_num = 0;
        fpPmicLdoSet.pmic_power_type = VOUT_BUCK_1;
        if (high_vset) {
            fpPmicLdoSet.pmic_power_voltage = BUCK_1_P15_V;
        } else {
            fpPmicLdoSet.pmic_power_voltage = BUCK_1_P05_V;
        }
        /* 打开或关闭 0/1 */
        if (board_info_etc.buck_control_flag) {
            fpPmicLdoSet.pmic_power_state = 1;
        } else {
            fpPmicLdoSet.pmic_power_state = 0;
        }

        PS_PRINT_INFO("share buck mode, buck_flag[0x%x],hw_pmic_power_cfg:power_state[%d],vset[%d]\n",
                      board_info_etc.buck_control_flag, fpPmicLdoSet.pmic_power_state, fpPmicLdoSet.pmic_power_voltage);

        hw_pmic_power_cfg(WBG_PMIC_REQ, &fpPmicLdoSet);
    } else {
        PS_PRINT_INFO("NOT share buck mode[0x%x], do nothing\n", board_info_etc.buck_param);
    }
#endif
    return ;
#endif
}

int32 wlan_power_on_etc(void)
{
    int32 error;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_download_time = 0;
    static unsigned long long num = 0;
    BOARD_INFO *bd_info = NULL;

    PS_PRINT_INFO("wlan power on!\n");

    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        return -FAILURE;
    }

    /* wifi上电时如果单红外打开，则需要关闭单红外，下载全patch */
    if (oal_atomic_read(&ir_only_mode) != 0) {
        if (hw_ir_only_open_other_subsys() != BFGX_POWER_SUCCESS) {
            PS_PRINT_ERR("ir only mode,but close ir only mode fail!\n");
            return -FAILURE;
        }
    }

    start_time = ktime_get();

    if (hcc_bus_exception_is_busy(hcc_get_current_110x_bus()) == OAL_TRUE) {
        DECLARE_DFT_TRACE_KEY_INFO("open_fail_exception_is_busy", OAL_DFT_TRACE_FAIL);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_POWER_ON_EXCP);
        return -FAILURE;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (is_dfr_test_en(WIFI_POWER_ON_FAULT)) {
        error = WIFI_POWER_BFGX_DERESET_WCPU_FAIL;
        PS_PRINT_WARNING("dfr test WIFI_POWER_ON_FAULT enable\n");
        goto wifi_power_fail;
    }
#endif

    BuckPowerCtrl(OAL_TRUE, WIFI_SUBSYS);

    error = bd_info->bd_ops.wlan_power_on_etc();
    if (error != WIFI_POWER_SUCCESS) {
        goto wifi_power_fail;
    }

    if (get_hi110x_subchip_type() != BOARD_VERSION_HI1102) {
        wifi_notify_bfgx_status(SYS_CFG_NOTIFY_WIFI_OPEN);
    }

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);

    if (total_time > max_download_time) {
        max_download_time = total_time;
    }

    num++;
    PS_PRINT_WARNING("power on, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     num, total_time, max_download_time);

    return WIFI_POWER_SUCCESS;

wifi_power_fail:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_POWER_UP_WIFI);
    return wifi_power_fail_process_etc(error);
}

int32 wlan_power_off_etc(void)
{
    int32 error;
    BOARD_INFO *bd_info = NULL;

    PS_PRINT_INFO("wlan power off!\n");

    bd_info = get_hi110x_board_info_etc();
    if (unlikely(bd_info == NULL)) {
        PS_PRINT_ERR("board info is err\n");
        return -FAILURE;
    }

    if (get_hi110x_subchip_type() != BOARD_VERSION_HI1102) {
        wifi_notify_bfgx_status(SYS_CFG_NOTIFY_WIFI_CLOSE);
    }

    error = bd_info->bd_ops.wlan_power_off_etc();

    BuckPowerCtrl(OAL_FALSE, WIFI_SUBSYS);

    return error;
}

int32 bfgx_power_on_etc(uint8 subsys)
{
    int32 ret = BFGX_POWER_SUCCESS;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_download_time = 0;
    static unsigned long long num = 0;

    start_time = ktime_get();

    BuckPowerCtrl(OAL_TRUE, subsys);

    if (bfgx_other_subsys_all_shutdown_etc(subsys)) {
        ret = bfgx_dev_power_on_etc();
        if (ret != BFGX_POWER_SUCCESS) {
            return ret;
        }
    }

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_download_time) {
        max_download_time = total_time;
    }

    num++;
    PS_PRINT_WARNING("power on, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     num, total_time, max_download_time);

    return BFGX_POWER_SUCCESS;
}

int32 bfgx_power_off_etc(uint8 subsys)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret = SUCCESS;
    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return -FAILURE;
    }

    if (ps_chk_only_gnss_and_cldslp_etc(ps_core_d)) {
        PS_PRINT_SUC("%s power off request sleep!\n", bfgx_subsys_name_etc[subsys]);
        host_allow_devslp_in_node(ps_core_d);
    }

    PS_PRINT_INFO("%s power off!\n", bfgx_subsys_name_etc[subsys]);

    if (bfgx_other_subsys_all_shutdown_etc(subsys)) {
        ret = bfgx_dev_power_off_etc();
    }

    BuckPowerCtrl(OAL_FALSE, subsys);

    return ret;
}

#ifdef PLATFORM_DEBUG_ENABLE
uint32 wlan_soc_check_mdelay = 12000;
uint32 bfgx_soc_check_mdelay = 25000;
uint16 wlan_soc_test_mode = 0xf7ff; // 默认不测IPC。IPC有单独的用例测
uint16 bfgx_soc_test_mode = 0xb7ff; // 默认不测IPC,I3C

void set_wlan_soc_test_mdelay(uint32 mdelay)
{
    wlan_soc_check_mdelay = mdelay;
    PS_PRINT_INFO("===================set wlan_soc_check_mdelay = [%u]ms \n", wlan_soc_check_mdelay);
}

void set_bfgx_soc_test_mdelay(uint32 mdelay)
{
    bfgx_soc_check_mdelay = mdelay;
    PS_PRINT_INFO("===================set bfgx_soc_check_mdelay = [%u]ms \n", bfgx_soc_check_mdelay);
}

void set_wlan_soc_test_mode(uint16 test_mode)
{
    wlan_soc_test_mode = test_mode;
    PS_PRINT_INFO("===================set soc_test_mode = [0x%x] \n", wlan_soc_test_mode);
}

void set_bfgx_soc_test_mode(uint16 test_mode)
{
    bfgx_soc_test_mode = test_mode;
    PS_PRINT_INFO("===================set soc_test_mode = [0x%x] \n", bfgx_soc_test_mode);
}

void soc_test_help(void)
{
    PS_PRINT_INFO("****TIMER_TEST_FLAG          BIT0**** \n");
    PS_PRINT_INFO("****REG_MEM_CHECK_TEST_FLAG  BIT1**** \n");
    PS_PRINT_INFO("****INTERRUPT_TEST_FLAG      BIT2**** \n");
    PS_PRINT_INFO("****MEM_MONITOR_TEST_FLAG    BIT3**** \n");
    PS_PRINT_INFO("****DMA_TEST_FLAG            BIT4**** \n");
    PS_PRINT_INFO("****CPU_TRACE_TEST_FLAG      BIT5**** \n");
    PS_PRINT_INFO("****PATCH_TEST_FLAG          BIT6**** \n");
    PS_PRINT_INFO("****SSI_MASTER_TEST_FLAG     BIT7**** \n");
    PS_PRINT_INFO("****EFUSE_TEST_FLAG(null)    BIT8**** \n");
    PS_PRINT_INFO("****WDT_TEST_FLAG            BIT9**** \n");
    PS_PRINT_INFO("****MPU_TEST_FLAG            BIT10*** \n");
    PS_PRINT_INFO("****IPC_TEST_FLAG            BIT11*** \n");
    PS_PRINT_INFO("****MBIST_TEST_FLAG(null)    BIT12*** \n");
    PS_PRINT_INFO("****SCHEDULE_FPU_TEST_FLAG(b)BIT13*** \n");
    PS_PRINT_INFO("****I3C_SLAVE_TEST_FLAG(b)   BIT14*** \n");
    PS_PRINT_INFO("======= ecall set_(wlan or bfgx)_soc_test_mode_hi110x with a mode can set test mode\n");
    PS_PRINT_INFO("======= ecall set_(wlan or bfgx)_soc_test_mdelay_hi110x with a mode can set test mdelay\n");
}

static int32 get_device_soc_test_result(void)
{
    int32 ret;
    int32 test_flag = 0;

    ret = number_type_cmd_send_etc(RMEM_CMD_KEYWORD, SOC_TEST_RST_FLAG);
    if (ret < 0) {
        PS_PRINT_WARNING("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, SOC_TEST_RST_FLAG, ret);
        return -FAILURE;
    }

    ret = read_msg_etc((uint8 *)&test_flag, sizeof(test_flag));
    if (ret < 0) {
        PS_PRINT_WARNING("read device test flag fail, read_len = %d, return = %d\n", (int32)sizeof(test_flag), ret);
        return -FAILURE;
    }
    PS_PRINT_WARNING("get device test flag: ****[0x%x]**** \n", test_flag);
    return test_flag;
}

static int32 wlan_soc_test_init(void)
{
    int32 ret;

    /* 对应device逻辑，保证每次用例都得到正确的初始化 */
    ret = write_device_reg16(SOC_TEST_STATUS_REG, SOC_TEST_RUN_FINISH);
    if (ret) {
        PS_PRINT_ERR("SOC-TEST status set failed ret=%d\n", ret);
        board_power_off_etc(WLAN_POWER);
        return -FAILURE;
    }

    ret = write_device_reg16(SOC_TEST_MODE_REG, wlan_soc_test_mode);
    if (ret) {
        PS_PRINT_ERR("SOC-TEST mode set failed ret=%d\n", ret);
        board_power_off_etc(WLAN_POWER);
        return -FAILURE;
    }
    PS_PRINT_SUC("SOC-TEST mode ****[0x%x]**** set succ \n", wlan_soc_test_mode);
    return 0;
}

static int32 soc_ipc_test_init(void)
{
    int32 ret;

    /* 对应device逻辑，保证每次用例都得到正确的初始化 */
    ret = write_device_reg16(SOC_TEST_STATUS_REG, SOC_TEST_RUN_FINISH);
    if (ret) {
        PS_PRINT_ERR("SOC-TEST status set failed ret=%d\n", ret);
        board_power_off_etc(WLAN_POWER);
        return -FAILURE;
    }

    ret = write_device_reg16(SOC_TEST_MODE_REG, IPC_TEST_FLAG);
    if (ret) {
        PS_PRINT_ERR("SOC-TEST mode set failed ret=%d\n", ret);
        board_power_off_etc(WLAN_POWER);
        return -FAILURE;
    }
    PS_PRINT_SUC("SOC-TEST mode ****[0x%x]**** set succ \n", IPC_TEST_FLAG);
    return 0;
}

static int32 bfgx_soc_test_init(void)
{
    int32 ret;

    /* 对应device逻辑，保证每次用例都得到正确的初始化 */
    ret = write_device_reg16(SOC_TEST_STATUS_REG, SOC_TEST_RUN_FINISH);
    if (ret) {
        PS_PRINT_ERR("SOC-TEST status set failed ret=%d\n", ret);
        board_power_off_etc(WLAN_POWER);
        return -FAILURE;
    }

    ret = write_device_reg16(SOC_TEST_MODE_REG, bfgx_soc_test_mode);
    if (ret) {
        PS_PRINT_ERR("SOC-TEST mode set failed ret=%d\n", ret);
        board_power_off_etc(WLAN_POWER);
        return -FAILURE;
    }
    PS_PRINT_SUC("SOC-TEST mode ****[0x%x]**** set succ \n", bfgx_soc_test_mode);
    return 0;
}

static void print_soc_err_msg(uint16 us_err_rst)
{
    printk("[plat]********* / ");
    if (us_err_rst & TIMER_TEST_FLAG) {
        printk("timer / ");
    }
    if (us_err_rst & REG_MEM_CHECK_TEST_FLAG) {
        printk("reg mem check / ");
    }
    if (us_err_rst & INTERRUPT_TEST_FLAG) {
        printk("interrupt / ");
    }
    if (us_err_rst & MEM_MONITOR_TEST_FLAG) {
        printk("mem monitor / ");
    }
    if (us_err_rst & DMA_TEST_FLAG) {
        printk("dma / ");
    }
    if (us_err_rst & CPU_TRACE_TEST_FLAG) {
        printk("cpu trace / ");
    }
    if (us_err_rst & PATCH_TEST_FLAG) {
        printk("patch / ");
    }
    if (us_err_rst & SSI_MASTER_TEST_FLAG) {
        printk("ssi master / ");
    }
    if (us_err_rst & WDT_TEST_FLAG) {
        printk("wdt / ");
    }
    if (us_err_rst & MPU_TEST_FLAG) {
        printk("mpu / ");
    }
    if (us_err_rst & IPC_TEST_FLAG) {
        printk("ipc / ");
    }
    if (us_err_rst & SCHEDULE_FPU_TEST_FLAG) {
        printk("fpu / ");
    }
    if (us_err_rst & I3C_SLAVE_TEST_FLAG) {
        printk("i3c / ");
    }
    printk("test failed *********\r\n");

}

static void check_soc_test_result(uint16 us_expect_val, uint16 us_reg_val, bool is_wifi)
{
    uint16 us_err_rst;

    if (us_reg_val == us_expect_val) {
        PS_PRINT_SUC("=============device %s SOC-TEST success!\n", is_wifi ? "WCPU" : "BCPU");
    } else {
        PS_PRINT_ERR("=============device %s SOC-TEST failed!\n", is_wifi ? "WCPU" : "BCPU");
        if (us_expect_val > us_reg_val) {
            us_err_rst = (uint16)(us_expect_val - us_reg_val);
        } else {
            PS_PRINT_ERR("*****ut result not accord with expect, please check code*****\n");
            PS_PRINT_ERR("*****expect result = [0x%x], real result = [0x%x]*****\n", us_expect_val, us_reg_val);
            return;
        }
        print_soc_err_msg(us_err_rst);
    }
}

static void device_soc_ipc_test(void)
{
    int32 ret;
    const int32 ipc_test_mdelay = 2000;

    PS_PRINT_INFO("===================start ipc SOC-TEST!\n");
    ret = board_power_on_etc(WLAN_POWER);
    if (ret) {
        PS_PRINT_ERR("WLAN_POWER on failed ret=%d\n", ret);
        return;
    }

    ret = firmware_download_function_priv(BFGX_AND_WIFI_CFG, soc_ipc_test_init);
    if (ret == SUCCESS) {
        /* 等待device信息处理，ipc用例2s足够 */
        mdelay(ipc_test_mdelay);
        ret = get_device_soc_test_result();
    }
    board_power_off_etc(WLAN_POWER);

    if (ret == IPC_TEST_FLAG) {
        PS_PRINT_INFO("=============device ipc SOC-TEST success!\n");
    } else {
        PS_PRINT_INFO("=============device ipc SOC-TEST failed!\n");
    }
    PS_PRINT_INFO("=======================================================================!\n");
}

static int32 device_bfgx_soc_test(void)
{
    int ret;

    PS_PRINT_INFO("===================start bcpu SOC-TEST!\n");
    ret = board_power_on_etc(WLAN_POWER);
    if (ret) {
        PS_PRINT_ERR("WLAN_POWER on failed ret=%d\n", ret);
        return -FAILURE;
    }

    ret = firmware_download_function_priv(BFGX_CFG, bfgx_soc_test_init);
    if (ret == SUCCESS) {
        /* 等待device信息处理 */
        mdelay(bfgx_soc_check_mdelay);
        ret = get_device_soc_test_result();
    }
    board_power_off_etc(WLAN_POWER);

    return ret;
}

/*
 * 因rom same版本soc test用例bin文件过大，在单device测试时下bin文件时间过长，而采取host的方式下bin。
 * 编译时1105请用build_test.sh asic/fpga (rom same版本)方式取bin；
 *       1103请用build_test_gcc.sh pilot asic/fpga (rom same版本)方式取bin。
 * 测试时，替换bin文件之后，直接调用到device_soc_test() 函数即可(ecall)
 * 默认情况下除i3c之外其他的用例都会测到。
 * 注:1105 build_soc_test.sh asic/fpga 编出的是不保证rom same的版本，此版本bin较小，可以用单device方式测试；
 *    1103 build_test_gcc.sh mpw2 编出的是不保证rom same的版本，但是patch和ssi master用例测不了。
 */
void device_soc_test(void)
{
    int32 w_ret;
    int32 b_ret;

    if (!bfgx_is_shutdown_etc()) {
        PS_PRINT_ERR("SOC-TEST need bfgx shut down!\n");
        bfgx_print_subsys_state();
        return;
    }
    if (!wlan_is_shutdown_etc()) {
        PS_PRINT_ERR("SOC-TEST need wlan shut down!\n");
        return;
    }

    PS_PRINT_INFO("===================start wcpu SOC-TEST!\n");
    w_ret = board_power_on_etc(WLAN_POWER);
    if (w_ret) {
        PS_PRINT_ERR("WLAN_POWER on failed ret=%d\n", w_ret);
        return;
    }

    w_ret = firmware_download_function_priv(WIFI_CFG, wlan_soc_test_init);
    if (w_ret == SUCCESS) {
        /* 等待device信息处理 */
        mdelay(wlan_soc_check_mdelay);
        w_ret = get_device_soc_test_result();
    }
    board_power_off_etc(WLAN_POWER);

    b_ret = device_bfgx_soc_test();

    device_soc_ipc_test();

    if (w_ret == -FAILURE) {
        PS_PRINT_INFO("=============device wcpu SOC-TEST failed! read device result reg fail!\n");
    } else {
        check_soc_test_result(wlan_soc_test_mode, (uint16)w_ret, true);
    }
    PS_PRINT_INFO("=======================================================================!\n");
    if (b_ret == -FAILURE) {
        PS_PRINT_INFO("=============device bcpu SOC-TEST failed! read device result reg fail!\n");
    } else {
        check_soc_test_result(bfgx_soc_test_mode, (uint16)b_ret, false);
    }
    PS_PRINT_INFO("======= ecall soc_test_help_hi110x can get some help\n");
}
#endif // endif of PLATFORM_DEBUG_ENABLE

int32 pro_memcheck_en = 0;
struct completion pro_memcheck_finish;
int32 memcheck_is_working(void)
{
    if (pro_memcheck_en) {
        complete(&pro_memcheck_finish);
        PS_PRINT_INFO("is in product mem check test !bfg_wakeup_host=%d\n",
                      oal_gpio_get_value(board_info_etc.bfgn_wakeup_host));
        return 0;
    }
    return -1;
}
void memcheck_bfgx_init(void)
{
    bfgx_gpio_intr_enable(OAL_TRUE);
    PS_PRINT_INFO("memcheck_bfgx_init\n");
    pro_memcheck_en = 1;
    init_completion(&pro_memcheck_finish);
}
void memcheck_bfgx_exit(void)
{
    pro_memcheck_en = 0;
    bfgx_gpio_intr_enable(OAL_FALSE);
}

int32 wait_gpio_level_etc(int32 gpio_num, int32 gpio_level, unsigned long timeout)
{
    int32 gpio_value;
    if ((gpio_num < 0) || (timeout == 0)) {
        OAL_WARN_ON(1);
        return -FAILURE;
    }
    gpio_level = (gpio_level == GPIO_LOWLEVEL) ? GPIO_LOWLEVEL : GPIO_HIGHLEVEL;
    for (;;) {
        gpio_value = oal_gpio_get_value(gpio_num);
        gpio_value = (gpio_value == GPIO_LOWLEVEL) ? GPIO_LOWLEVEL : GPIO_HIGHLEVEL;
        if (gpio_value == gpio_level) {
            return SUCCESS;
        }

        if (time_after(jiffies, timeout)) {
            return -FAILURE;
        } else {
            oal_msleep(10);
        }
    }
}

int32 memcheck_bfgx_is_succ(void)
{
    unsigned long timeout;
    unsigned long timeout_hold;
    oal_uint32 hold_time = 100; /* 拉高维持100ms */
    declare_time_cost_stru(cost);

    /* 中断改成电平判断，WLAN POWERON拉高瞬间存在毛刺会误报中断 */
    timeout = jiffies + msecs_to_jiffies(bfgx_mem_check_mdelay);
    PS_PRINT_INFO("bfgx memcheck gpio level check start,timeout=%d ms\n", bfgx_mem_check_mdelay);
    oal_get_time_cost_start(cost);

    while (1) {
        // wait gpio high
        if (wait_gpio_level_etc(board_info_etc.bfgn_wakeup_host, GPIO_HIGHLEVEL, timeout) != SUCCESS) {
            PS_PRINT_ERR("[E]wait wakeup gpio to high timeout [%u] ms[%lu:%lu]\n",
                         bfgx_mem_check_mdelay, jiffies, timeout);
            return -FAILURE;
        }
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        PS_PRINT_INFO("bfgx wake up host gpio to high %llu us", time_cost_var_sub(cost));

        // high level hold time
        timeout_hold = jiffies + msecs_to_jiffies(hold_time);
        oal_msleep(10);

        // wait high level hold time
        if (wait_gpio_level_etc(board_info_etc.bfgn_wakeup_host, GPIO_LOWLEVEL, timeout_hold) == SUCCESS) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "[E]gpio pull down again, retry");
            oal_msleep(10);
            continue;
        }

        // gpio high and hold enough time.
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        PS_PRINT_INFO("bfgx hold high level %u ms,check succ, test cost %llu us",
                      hold_time, time_cost_var_sub(cost));
        break;
    }

    return SUCCESS;
}

static int32 device_mem_check_priv_init(void)
{
    int32 ret;
    ret = write_device_reg16(RAM_TEST_RUN_VOLTAGE_REG_ADDR,
                             (ram_test_run_voltage_bias_sel == RAM_TEST_RUN_VOLTAGE_BIAS_HIGH) ?
                              RAM_TEST_RUN_VOLTAGE_BIAS_HIGH : RAM_TEST_RUN_VOLTAGE_BIAS_LOW);
    if (ret) {
        PS_PRINT_ERR("write reg=0x%x value=0x%x failed, high bias\n",
                     RAM_TEST_RUN_VOLTAGE_REG_ADDR, ram_test_run_voltage_bias_sel);
        return ret;
    }

    PS_PRINT_INFO("ram_test_run_voltage_bias_sel=%d, [%s]\n",
                  ram_test_run_voltage_bias_sel,
                  (ram_test_run_voltage_bias_sel == RAM_TEST_RUN_VOLTAGE_BIAS_HIGH) ?
                   "high voltage bias" : "low voltage bias");

    if (ram_test_run_process_sel) {
        ret = write_device_reg16(RAM_TEST_RUN_PROCESS_SEL_REG_ADDR,
                                 ram_test_run_process_sel);
        if (ret) {
            PS_PRINT_ERR("write reg=0x%x value=0x%x failed, process sel\n",
                         RAM_TEST_RUN_PROCESS_SEL_REG_ADDR, ram_test_run_process_sel);
            return ret; /* just test, if failed, we don't return */
        }
        PS_PRINT_INFO("ram_test_run_process_sel=%d\n", ram_test_run_process_sel);
    }
    return ret;
}

static void factory_cfg_init(bool is_wifi)
{
    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1103) {
        pilot_cfg_patch_in_vendor[RAM_REG_TEST_CFG] = is_wifi ?
            RAM_CHECK_CFG_HI1103_PILOT_PATH : RAM_BCPU_CHECK_CFG_HI1103_PILOT_PATH;
    } else if (get_hi110x_subchip_type() == BOARD_VERSION_HI1105) {
        hi1105_asic_cfg_patch_in_vendor[RAM_REG_TEST_CFG] = is_wifi ?
            RAM_CHECK_CFG_HI1105_ASIC_PATH : RAM_BCPU_CHECK_CFG_HI1105_ASIC_PATH;
    } else {
        PS_PRINT_ERR("board subchip type err, only support 1103&1105, now is [%d] ", get_hi110x_subchip_type());
    }
}

int32 device_mem_check_etc(unsigned long long *time)
{
    int32 ret;
    uint32 wcost = 0;
    uint32 bcost = 0;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    if (time == NULL) {
        PS_PRINT_ERR("param time is  NULL!\n");
        return -FAILURE;
    }
    start_time = ktime_get();

    PS_PRINT_INFO("device ram reg test!\n");

    if (!bfgx_is_shutdown_etc()) {
        PS_PRINT_SUC("factory ram reg test need bfgx shut down!\n");
        bfgx_print_subsys_state();
        return -FAILURE;
    }
    if (!wlan_is_shutdown_etc()) {
        PS_PRINT_SUC("factory ram reg test need wlan shut down!\n");
        return -FAILURE;
    }

    ret = board_power_on_etc(WLAN_POWER);
    if (ret) {
        PS_PRINT_ERR("WLAN_POWER on failed ret=%d\n", ret);
        return -FAILURE;
    }

    PS_PRINT_INFO("===================start wcpu ram reg test!\n");
    ret = firmware_download_function_priv(RAM_REG_TEST_CFG, device_mem_check_priv_init);
    if (ret == SUCCESS) {
        /* 等待device信息处理 */
        mdelay(wlan_mem_check_mdelay);
        ret = is_device_mem_test_succ();
        if (ram_test_detail_result_dump) {
            get_device_ram_test_result(true, &wcost);
        }

        if (ram_test_wifi_hold_time) {
            oal_msleep(ram_test_wifi_hold_time);
        }

        if (!ret) {
            PS_PRINT_INFO("==device wcpu ram reg test success!\n");
            if (ram_test_ssi_pass_dump)
                ssi_dump_device_regs(SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG |
                                     SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
            if (ram_test_mem_pass_dump) {
                get_device_test_mem(true);
            }
        } else {
            PS_PRINT_INFO("==device wcpu ram reg test failed!\n");
            if (ram_test_ssi_error_dump) {
                ssi_dump_device_regs(SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG |
                                     SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
            } else {
                ssi_dump_device_regs(SSI_MODULE_MASK_ARM_REG);
            }
            get_device_test_mem(true);
            goto exit_error;
        }
    }
    PS_PRINT_INFO("===================start bcpu ram reg test!\n");
    board_power_off_etc(WLAN_POWER);

    board_power_on_etc(WLAN_POWER);

    factory_cfg_init(false);

    ret = firmware_get_cfg_etc(cfg_path_etc[RAM_REG_TEST_CFG], RAM_REG_TEST_CFG);
    if (ret) {
        PS_PRINT_INFO("ini analysis fail!\n");
        goto exit_error;
    }

    memcheck_bfgx_init();

    ret = firmware_download_function_priv(RAM_REG_TEST_CFG, device_mem_check_priv_init);

    factory_cfg_init(true);

    if (ret == SUCCESS) {
        /* 等待device信息处理 */
        ret = memcheck_bfgx_is_succ();
        if (ram_test_detail_result_dump) {
            get_device_ram_test_result(false, &bcost);
        }

        if (ram_test_bfgx_hold_time) {
            oal_msleep(ram_test_bfgx_hold_time);
        }

        if (!ret) {
            PS_PRINT_INFO("==device bcpu ram reg test success!\n");
            if (ram_test_ssi_pass_dump)
                ssi_dump_device_regs(SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG |
                                     SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
            if (ram_test_mem_pass_dump) {
                get_device_test_mem(false);
            }

            PS_PRINT_INFO("[memcheck]bfg_wakeup_host=%d\n", oal_gpio_get_value(board_info_etc.bfgn_wakeup_host));
        } else {
            PS_PRINT_INFO("==device bcpu ram reg test failed!\n");
            firmware_get_cfg_etc(cfg_path_etc[RAM_REG_TEST_CFG], RAM_REG_TEST_CFG);
            if (ram_test_ssi_error_dump) {
                ssi_dump_device_regs(SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG |
                                     SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
            } else {
                ssi_dump_device_regs(SSI_MODULE_MASK_ARM_REG);
            }
            get_device_test_mem(false);
            goto exit_error;
        }
    }
    firmware_get_cfg_etc(cfg_path_etc[RAM_REG_TEST_CFG], RAM_REG_TEST_CFG);

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);

    *time = total_time;

    if (wcost + bcost) {
        PS_PRINT_SUC("device mem reg test time [%llu]us, actual cost=%u us\n", total_time, wcost + bcost);
    } else {
        PS_PRINT_SUC("device mem reg test time [%llu]us\n", total_time);
    }
exit_error:
    memcheck_bfgx_exit();
    board_power_off_etc(WLAN_POWER);
    return ret;
}

EXPORT_SYMBOL(device_mem_check_etc);
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
/*
 * Prototype    : pm_uart_send
 * Description  : uart patch transmit function
 * Input        : uint8 *date: address of data
 *                int32 len: length of data
 * Return       : length which has been sent
 */
int32 pm_uart_send(uint8 *data, int32 len)
{
    uint16 count = 0;

    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -1;
    }

    if (pm_data->ps_pm_interface->write_patch == NULL) {
        PS_PRINT_ERR("bfg_write_patch is NULL!\n");
        return -1;
    }

    while (1) {
        /* this function return the length has been sent */
        count = pm_data->ps_pm_interface->write_patch(data, len);
        /* data has been sent over and return */
        if (count == len) {
            return len;
        }

        /* data has not been sent over, we will send again */
        data = data + count;
        len = len - count;
        msleep(1);
    }
}

/*
 * Prototype    : bfg_patch_download_function
 * Description  : download bfg patch
 * Return       : 0 means succeed,-1 means failed
 */
int bfg_patch_download_function(void)
{
    int32 ret;
    int32 counter = 0;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_time = 0;
    static unsigned long long count = 0;
    unsigned long long total_time = 0;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -1;
    }

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is err\n");
        return -1;
    }

    PS_PRINT_DBG("enter\n");
    start_time = ktime_get();

    /* patch_init(uart) function should just be called once */
    ret = patch_init(ENUM_INFO_UART);
    if (ret) {
        PS_PRINT_ERR("patch modem init failed, ret:%d!\n", ret);
        return ret;
    }

    /* bfg patch download, three times for fail */
    for (counter = 0; counter < 3; counter++) {
        ps_uart_state_pre_etc(ps_core_d->tty);
        ret = patch_download_patch(ENUM_INFO_UART);
        if (ret) {
            PS_PRINT_ERR("bfg patch download fail, and reset power!\n");
            ps_uart_state_dump_etc(ps_core_d->tty);
        } else {
            end_time = ktime_get();
            trans_time = ktime_sub(end_time, start_time);
            total_time = (unsigned long long)ktime_to_us(trans_time);
            if (total_time > max_time) {
                max_time = total_time;
            }

            PS_PRINT_WARNING("download bfg patch succ,count [%llu], current time [%llu]us, max time [%llu]us\n",
                             count, total_time, max_time);
            /* download patch successfully */
            return ret;
        }
    }

    /* going to exception */
    PS_PRINT_ERR("bfg patch download has failed finally!\n");
    return ret;
}
#endif
/*
 * Prototype    : ps_pm_register_etc
 * Description  : register interface for 3 in 1
 * Input        : struct ps_pm_s *new_pm: interface want to register
 * Return       : 0 means succeed,-1 means failed
 */
int32 ps_pm_register_etc(struct ps_pm_s *new_pm)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL\n");
        return -FAILURE;
    }

    if (new_pm == NULL) {
        PS_PRINT_ERR("new_pm is null!\n");
        return -FAILURE;
    }

    pm_data->ps_pm_interface = new_pm;
    pm_data->ps_pm_interface->pm_priv_data = pm_data;
    pm_data->ps_pm_interface->bfg_wake_lock_etc = bfg_wake_lock_etc;
    pm_data->ps_pm_interface->bfg_wake_unlock_etc = bfg_wake_unlock_etc;
    pm_data->ps_pm_interface->bfgx_dev_state_get = bfgx_state_get_etc;
    pm_data->ps_pm_interface->bfgx_dev_state_set = bfgx_state_set_etc;
    pm_data->ps_pm_interface->bfg_power_set = bfgx_dev_power_control_etc;
    pm_data->ps_pm_interface->bfgx_uart_state_get = bfgx_uart_state_get;
    pm_data->ps_pm_interface->bfgx_uart_state_set = bfgx_uart_state_set;
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    pm_data->ps_pm_interface->download_patch = bfg_patch_download_function;
    pm_data->ps_pm_interface->recv_patch = bfg_patch_recv;
    pm_data->ps_pm_interface->write_patch = ps_patch_write;
#endif
    PS_PRINT_SUC("pm registered over!");

    return SUCCESS;
}
EXPORT_SYMBOL_GPL(ps_pm_register_etc);

/*
 * Prototype    : ps_pm_unregister_etc
 * Description  : unregister interface for 3 in 1
 * Input        : struct ps_pm_s *new_pm: interface want to unregister
 * Return       : 0 means succeed,-1 means failed
 */
int32 ps_pm_unregister_etc(struct ps_pm_s *new_pm)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL\n");
        return -FAILURE;
    }

    PS_PRINT_DBG("enter\n");

    if (new_pm == NULL) {
        PS_PRINT_ERR("new_pm is null!\n");
        return -FAILURE;
    }

    new_pm->bfg_wake_lock_etc = NULL;
    new_pm->bfg_wake_unlock_etc = NULL;
    new_pm->bfgx_dev_state_get = NULL;
    new_pm->bfgx_dev_state_set = NULL;
    new_pm->bfg_power_set = NULL;
    new_pm->bfgx_uart_state_set = NULL;
    new_pm->bfgx_uart_state_get = NULL;
    pm_data->ps_pm_interface = NULL;

    PS_PRINT_SUC("pm unregistered over!");

    return SUCCESS;
}
EXPORT_SYMBOL_GPL(ps_pm_unregister_etc);

/*
 * Prototype    : bfg_wake_host_isr_etc
 * Description  : functions called when bt wake host via GPIO
 */
irqreturn_t bfg_wake_host_isr_etc(int irq, void *dev_id)
{
    struct ps_core_s *ps_core_d = NULL;
    uint64 flags;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (!memcheck_is_working()) {
        return IRQ_NONE;
    }
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return IRQ_NONE;
    }

    PS_PRINT_INFO("%s\n", __func__);
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    if (pm_data->uart_state >= UART_BPS_CHG_SEND_ACK) {
        if (pm_data->uart_state == UART_BPS_CHG_SEND_ACK) {
            bfgx_uart_state_set(UART_BPS_CHG_IN_PROGRESS);
        } else if (pm_data->uart_state == UART_BPS_CHG_SEND_COMPLETE) {
            bfgx_uart_state_set(UART_READY);
        }

        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
        return IRQ_HANDLED;
    }
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    pm_data->bfg_wakeup_host++;

    uc_wakeup_src_debug = 1;

    ps_get_core_reference_etc(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return IRQ_NONE;
    }

    spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
    bfg_wake_lock_etc();
    bfgx_state_set_etc(BFGX_ACTIVE);
    complete(&pm_data->dev_ack_comp);
    spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);

    queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);

    return IRQ_HANDLED;
}

#ifdef _PRE_HI_DRV_GPIO
void bfgx_wake_host_etc(uint32 para1)
{
    pstHisiGpioFunc->pfnGpioClearBitInt(BFGX_WKAEUP_HOST_GPIO);
    PS_PRINT_INFO("bgfx_wake_host_etc entering.-------------add GpioClearBitInt shangshengyan\n");
    (void)bfg_wake_host_isr_etc(0, NULL);
}
#endif

/* return 1 for wifi power on,0 for off. */
oal_int32 hi110x_get_wifi_power_stat_etc(oal_void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return 0;
    }
    return (pm_data->pst_wlan_pm_info->ul_wlan_power_state != POWER_STATE_SHUTDOWN);
}
EXPORT_SYMBOL(hi110x_get_wifi_power_stat_etc);

STATIC int low_power_remove(void)
{
    int ret = 0;
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -FAILURE;
    }

    wlan_pm_exit_etc();

    free_irq(pm_data->bfg_irq, NULL);

    /* delete timer */
    del_timer_sync(&pm_data->bfg_timer);
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    del_timer_sync(&pm_data->dev_ack_timer);
    /* destory wake lock */
    oal_wake_lock_exit(&pm_data->bfg_wake_lock_etc);
    oal_wake_lock_exit(&pm_data->bt_wake_lock);
    oal_wake_lock_exit(&pm_data->gnss_wake_lock);
    /* free platform driver data struct */
    kfree(pm_data);

    pm_data = NULL;

    pm_set_drvdata(NULL);

    return ret;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
STATIC void devack_timer_expire(uint64 data)
#else
STATIC void devack_timer_expire(struct timer_list *t)
#endif
{
    uint64 flags;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    struct pm_drv_data *pm_data = (struct pm_drv_data *)(uintptr_t)data;
#else
    struct pm_drv_data *pm_data = from_timer(pm_data, t, dev_ack_timer);
#endif
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("devack timer para is null\n");
        return;
    }

    PS_PRINT_INFO("%s\n", __func__);

    if (board_get_bwkup_gpio_val_etc() == 1) { /* 读出对应gpio管脚的值 */
        pm_data->uc_dev_ack_wait_cnt++;
        if (pm_data->uc_dev_ack_wait_cnt < WAIT_DEVACK_CNT) {
            mod_timer(&pm_data->dev_ack_timer, jiffies + (WAIT_DEVACK_MSEC * HZ / 1000));
            return;
        }
        /* device doesn't agree to sleep */
        PS_PRINT_INFO("device does not agree to sleep\n");
        if (unlikely(pm_data->rcvdata_bef_devack_flag == 1)) {
            PS_PRINT_INFO("device send data to host before dev rcv allow slp msg\n");
            pm_data->rcvdata_bef_devack_flag = 0;
        }

        bfgx_state_set_etc(BFGX_ACTIVE);
        bfgx_uart_state_set(UART_READY);
        /*
         * we mod timer at any time, since we could get another chance to sleep
         * in exception case like:dev agree to slp after this ack timer expired
         */
        if ((!bfgx_other_subsys_all_shutdown_etc(BFGX_GNSS)) ||
            (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP)) {
            mod_timer(&pm_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
            pm_data->bfg_timer_mod_cnt++;
        }

        complete(&pm_data->dev_ack_comp);
    } else {
        spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
        if (pm_data->bfgx_dev_state == BFGX_ACTIVE) {
            PS_PRINT_INFO("wkup isr occur during wait for dev allow ack\n");
        } else {
            pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
            bfg_wake_unlock_etc();
        }
        spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);

        complete(&pm_data->dev_ack_comp);

        pm_data->uc_dev_ack_wait_cnt = 0;
    }
}

static void low_power_probe_timer_init(struct pm_drv_data *pm_data)
{
    /* init timer */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    init_timer(&pm_data->dev_ack_timer);
    pm_data->dev_ack_timer.function = devack_timer_expire;
    pm_data->dev_ack_timer.data = (uintptr_t)pm_data;
#else
    timer_setup(&pm_data->dev_ack_timer, devack_timer_expire, 0);
#endif
    pm_data->uc_dev_ack_wait_cnt = 0;

    /* init bfg data timer */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    init_timer(&pm_data->bfg_timer);
    pm_data->bfg_timer.function = bfg_timer_expire_etc;
    pm_data->bfg_timer.data = (uintptr_t)pm_data;
#else
    timer_setup(&pm_data->bfg_timer, bfg_timer_expire_etc, 0);
#endif
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;
    pm_data->bfg_timer_check_time = 0;
    pm_data->rx_pkt_gnss_pre = 0;
    pm_data->gnss_votesleep_check_cnt = 0;

    PS_PRINT_INFO("uart baud change support version\n");
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    init_timer(&pm_data->baud_change_timer);
    pm_data->baud_change_timer.function = bfgx_uart_baud_change_expire;
    pm_data->baud_change_timer.data = (uintptr_t)pm_data;
#else
    timer_setup(&pm_data->baud_change_timer, bfgx_uart_baud_change_expire, 0);
#endif
}

#ifdef _PRE_HI_DRV_GPIO
int hitv_bfgx_wakeup_int_init(void)
{
    int ret;

    ret = pstHisiGpioFunc->pfnGpioDirSetBit(BFGX_WKAEUP_HOST_GPIO, 1);        /*input=1,ouput=0, HI_DRV_GPIO_SetDirBit*/
    ret |= pstHisiGpioFunc->pfnGpioSetIntType(BFGX_WKAEUP_HOST_GPIO, BFGX_HI_UNF_GPIO_INTTYPE_UP);
    PS_PRINT_ERR("HI_DRV_MODULE_GetFunction BT_GPIO TYPE_UP %08x \n", ret);
    ret |= pstHisiGpioFunc->pfnGpioRegisterServerFunc(BFGX_WKAEUP_HOST_GPIO, bfgx_wake_host_etc);       /*register intertupt callback function*/
    ret |= pstHisiGpioFunc->pfnGpioClearBitInt(BFGX_WKAEUP_HOST_GPIO);
    ret |= pstHisiGpioFunc->pfnGpioSetIntEnable(BFGX_WKAEUP_HOST_GPIO, BFGX_INT_ENABLE);                 /*debugging phase,keep enable*/
    PS_PRINT_INFO("HI_DRV_MODULE_GetFunction BT_GPIO pfnGpio %08x \n", ret);

    return ret;
}
#endif

STATIC int low_power_probe(void)
{
    int ret;
    struct pm_drv_data *pm_data = NULL;
    struct workqueue_struct *host_wkup_dev_workq = NULL;

    pm_data = kzalloc(sizeof(struct pm_drv_data), GFP_KERNEL);
    if (pm_data == NULL) {
        PS_PRINT_ERR("no mem to allocate pm_data\n");
        goto PMDATA_MALLOC_FAIL;
    }

    pm_data->pst_wlan_pm_info = wlan_pm_init_etc();
    if (pm_data->pst_wlan_pm_info == 0) {
        PS_PRINT_ERR("no mem to allocate wlan_pm_info\n");
        goto WLAN_INIT_FAIL;
    }

    pm_data->firmware_cfg_init_flag = 0;
    pm_data->rcvdata_bef_devack_flag = 0;
    pm_data->bfgx_dev_state = BFGX_SLEEP;
    pm_data->bt_fake_close_flag = OAL_FALSE;
    pm_data->bfgx_pm_ctrl_enable = BFGX_PM_DISABLE;
    pm_data->bfgx_lowpower_enable = BFGX_PM_ENABLE; /* enable host low_power function defaultly */

    pm_data->bfgx_bt_lowpower_enable = BFGX_PM_ENABLE;
    pm_data->bfgx_gnss_lowpower_enable = BFGX_PM_DISABLE;
    pm_data->bfgx_nfc_lowpower_enable = BFGX_PM_DISABLE;

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

    spin_lock_init(&pm_data->bfg_irq_spinlock);
    pm_data->board = get_hi110x_board_info_etc();
    pm_data->bfg_irq = (pm_data->board != NULL ? pm_data->board->bfgx_irq : 0);

#ifndef _PRE_HI_DRV_GPIO
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    ret = request_irq(pm_data->bfg_irq, bfg_wake_host_isr_etc, IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
                      "bfgx_wake_host", NULL);
#else
    ret = request_irq(pm_data->bfg_irq, bfg_wake_host_isr_etc, IRQF_DISABLED | IRQF_TRIGGER_RISING,
                      "bfgx_wake_host", NULL);
#endif
    if (ret < 0) {
        PS_PRINT_ERR("couldn't acquire %s IRQ\n", PROC_NAME_GPIO_BFGX_WAKEUP_HOST);
        goto REQ_IRQ_FAIL;
    }

    disable_irq_nosync(pm_data->bfg_irq);
#else
    /*doesn't support linux kernel interrupt register,using hisi interfaces*/
    ret = HI_DRV_MODULE_GetFunction(HI_ID_GPIO, (void**)(&pstHisiGpioFunc));
    if (ret)
    {
        PS_PRINT_ERR("HI_DRV_MODULE_GetFunction BT_GPIO Failed %08x \n", ret);
        goto WLAN_INIT_FAIL;
    }

    ret = hitv_bfgx_wakeup_int_init();
#endif

    pm_data->ul_irq_stat = 1; /* irq diabled default. */

    /* create an ordered workqueue with @max_active = 1 & WQ_UNBOUND flag to wake up device */
    host_wkup_dev_workq = create_singlethread_workqueue("wkup_dev_workqueue");
    if (host_wkup_dev_workq == NULL) {
        PS_PRINT_ERR("create wkup workqueue failed\n");
        goto CREATE_WORKQ_FAIL;
    }
    pm_data->wkup_dev_workqueue = host_wkup_dev_workq;
    INIT_WORK(&pm_data->wkup_dev_work, host_wkup_dev_work_etc);
    INIT_WORK(&pm_data->send_disallow_msg_work, host_send_disallow_msg_etc);
    INIT_WORK(&pm_data->send_allow_sleep_work, host_allow_bfg_sleep_etc);
    INIT_WORK(&pm_data->baud_change_work, bfgx_uart_baud_change_work);

    /* init bfg wake lock */
    oal_wake_lock_init(&pm_data->bfg_wake_lock_etc, BFG_LOCK_NAME);
    oal_wake_lock_init(&pm_data->bt_wake_lock, BT_LOCK_NAME);
    oal_wake_lock_init(&pm_data->gnss_wake_lock, GNSS_LOCK_NAME);

    /* init mutex */
    mutex_init(&pm_data->host_mutex);

    /* init spinlock */
    spin_lock_init(&pm_data->uart_state_spinlock);
    spin_lock_init(&pm_data->wakelock_protect_spinlock);

    pm_data->uart_state = UART_NOT_READY;
    pm_data->uart_baud_switch_to = default_baud_rate;

    low_power_probe_timer_init(pm_data);

    /* init completion */
    init_completion(&pm_data->host_wkup_dev_comp);
    init_completion(&pm_data->dev_ack_comp);
    init_completion(&pm_data->dev_bootok_ack_comp);

    /* set driver data */
    pm_set_drvdata(pm_data);

    /* register host pm */
    ret = register_pm_notifier(&pf_suspend_notifier);
    if (ret < 0) {
        PS_PRINT_ERR("%s : register_pm_notifier failed!\n", __func__);
    }

    return OAL_SUCC;

CREATE_WORKQ_FAIL:
    free_irq(pm_data->bfg_irq, NULL);
#ifndef _PRE_HI_DRV_GPIO
REQ_IRQ_FAIL:
#endif

WLAN_INIT_FAIL:
    kfree(pm_data);
PMDATA_MALLOC_FAIL:
    return -ENOMEM;
}

int low_power_init_etc(void)
{
    int ret;

    ret = low_power_probe();
    if (ret != SUCCESS) {
        PS_PRINT_ERR("low_power_init_etc: low_power_probe fail\n");
    }

    PS_PRINT_INFO("low_power_init_etc: success\n");
    return ret;
}

void low_power_exit_etc(void)
{
    low_power_remove();
    firmware_cfg_clear_etc();
}

#ifdef CONFIG_HI110X_GPS_SYNC
#define GNSS_SYNC_IOREMAP_SIZE 0x1000
struct gnss_sync_data *gnss_sync_data_t = NULL;

struct gnss_sync_data *gnss_get_sync_data(void)
{
    return gnss_sync_data_t;
}

static void gnss_set_sync_data(struct gnss_sync_data *data)
{
    gnss_sync_data_t = data;
}

static int gnss_sync_probe(struct platform_device *pdev)
{
    struct gnss_sync_data *sync_info = NULL;
    struct device_node *np = pdev->dev.of_node;
    uint32 addr_base;
    int32 ret;
    uint32 version = 0;

    PS_PRINT_INFO("[GPS] gnss sync probe start\n");

    ret = of_property_read_u32(np, "version", &version);
    if (ret != SUCCESS) {
        PS_PRINT_ERR("[GPS] get gnss sync version failed!\n");
        return -FAILURE;
    }

    PS_PRINT_INFO("[GPS] gnss sync version %d\n", version);
    if (version == 0) {
        return SUCCESS;
    }

    sync_info = kzalloc(sizeof(struct gnss_sync_data), GFP_KERNEL);
    if (sync_info == NULL) {
        PS_PRINT_ERR("[GPS] alloc memory failed\n");
        return -ENOMEM;
    }

    sync_info->version = version;

    ret = of_property_read_u32(np, "addr_base", &addr_base);
    if (ret != SUCCESS) {
        PS_PRINT_ERR("[GPS] get gnss sync reg base failed!\n");
        ret = -FAILURE;
        goto sync_get_resource_fail;
    }

    sync_info->addr_base_virt = ioremap(addr_base, GNSS_SYNC_IOREMAP_SIZE);
    if (sync_info->addr_base_virt == NULL) {
        PS_PRINT_ERR("[GPS] gnss sync reg ioremap failed!\n");
        ret = -ENOMEM;
        goto sync_get_resource_fail;
    }

    ret = of_property_read_u32(np, "addr_offset", &sync_info->addr_offset);
    if (ret != SUCCESS) {
        PS_PRINT_ERR("[GPS] get gnss sync reg offset failed!\n");
        ret = -FAILURE;
        goto sync_get_resource_fail;
    }

    PS_PRINT_INFO("[GPS] gnss sync probe is finished!\n");

    gnss_set_sync_data(sync_info);

    return SUCCESS;

sync_get_resource_fail:
    gnss_set_sync_data(NULL);
    kfree(sync_info);
    return ret;
}

static void gnss_sync_shutdown(struct platform_device *pdev)
{
    struct gnss_sync_data *sync_info = gnss_get_sync_data();
    PS_PRINT_INFO("[GPS] gnss sync shutdown!\n");

    if (sync_info == NULL) {
        PS_PRINT_ERR("[GPS] gnss sync info is NULL.\n");
        return;
    }

    gnss_set_sync_data(NULL);
    kfree(sync_info);
    return;
}

#define DTS_COMP_GNSS_SYNC_NAME "hisilicon,hisi_gps_sync"

static const struct of_device_id gnss_sync_match_table[] = {
    {
        .compatible = DTS_COMP_GNSS_SYNC_NAME,  // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};

static struct platform_driver gnss_sync_driver = {
    .probe = gnss_sync_probe,
    .suspend = NULL,
    .remove = NULL,
    .shutdown = gnss_sync_shutdown,
    .driver = {
        .name = "hisi_gps_sync",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(gnss_sync_match_table),  // dts required code
    },
};

int gnss_sync_init(void)
{
    int ret;
    ret = platform_driver_register(&gnss_sync_driver);
    if (ret) {
        PS_PRINT_ERR("[GPS] unable to register gnss sync driver!\n");
    } else {
        PS_PRINT_INFO("[GPS] gnss sync init ok!\n");
    }
    return ret;
}

void gnss_sync_exit(void)
{
    platform_driver_unregister(&gnss_sync_driver);
}
#endif
