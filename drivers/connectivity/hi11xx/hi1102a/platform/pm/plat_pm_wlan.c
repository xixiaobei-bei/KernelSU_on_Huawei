

/*****************************************************************************
  1 Header File Including
*****************************************************************************/
#define HI11XX_LOG_MODULE_NAME     "[WLAN_PM]"
#define HI11XX_LOG_MODULE_NAME_VAR wlan_pm_loglevel
#include <linux/module.h> /* kernel module definitions */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>

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
#include <linux/pm_wakeup.h>

#include "oal_sdio.h"
#include "oal_sdio_comm.h"
#include "oal_sdio_host_if.h"
#include "oal_hcc_bus.h"

#include "plat_type.h"
#include "plat_debug.h"
#include "board.h"
#include "plat_pm_wlan.h"
#include "plat_pm.h"

#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"
#include "bfgx_exception_rst.h"
#include "securec.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif  // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_PM_WLAN_C

/*****************************************************************************
  2 Global Variable Definition
*****************************************************************************/

struct wlan_pm_s *gpst_wlan_pm_info = OAL_PTR_NULL;

pm_callback_stru gst_wlan_pm_callback = {
    .pm_wakeup_dev = wlan_pm_wakeup_dev,
    .pm_state_get = wlan_pm_state_get,
    .pm_wakeup_host = wlan_pm_wakeup_host,
    .pm_feed_wdg = wlan_pm_feed_wdg,
    .pm_wakeup_dev_ack = wlan_pm_wakeup_dev_ack,
    .pm_disable = wlan_pm_disable_check_wakeup,

};

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static RAW_NOTIFIER_HEAD(wifi_pm_chain);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_uint8 custom_cali_done = OAL_FALSE;
#endif
oal_bool_enum wlan_pm_switch = OAL_FALSE;
oal_uint8 wlan_device_pm_switch = OAL_TRUE;  // device 低功耗开关
oal_uint8 wlan_ps_mode = 1;
oal_uint8 wlan_fast_ps_mode_dyn_ctl = 0;  // app layer dynamic ctrl enable
oal_uint8 wlan_fast_check_cnt = 10;       // device每20ms检查一次如果检查g_wlan_fast_check_cnt依旧无数据收发则进入低功耗模式

#ifdef _PRE_WLAN_RF_AUTOCALI
oal_uint8 autocali_switch = OAL_FALSE;
EXPORT_SYMBOL_GPL(autocali_switch);
#endif

#ifdef _PRE_WLAN_DOWNLOAD_PM
oal_uint16 download_rate_limit_pps = 0;
EXPORT_SYMBOL_GPL(download_rate_limit_pps);
#endif

EXPORT_SYMBOL_GPL(wlan_pm_switch);
EXPORT_SYMBOL_GPL(wlan_device_pm_switch);
EXPORT_SYMBOL_GPL(wlan_ps_mode);
EXPORT_SYMBOL_GPL(wlan_fast_check_cnt);
EXPORT_SYMBOL_GPL(wlan_fast_ps_mode_dyn_ctl);

/* 30000ms/100ms = 300 cnt */
oal_uint32 wlan_sleep_request_forbid_limit = (30000) / (WLAN_SLEEP_TIMER_PERIOD * WLAN_SLEEP_DEFAULT_CHECK_CNT);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL_GPL(custom_cali_done);
#endif

void wlan_pm_wakeup_work(oal_work_stru *pst_worker);
void wlan_pm_sleep_work(oal_work_stru *pst_worker);

void wlan_pm_wdg_timeout(unsigned long data);

int32 wlan_pm_wakeup_done_callback(void *data);
int32 wlan_pm_close_done_callback(void *data);
int32 wlan_pm_open_bcpu_done_callback(void *data);
int32 wlan_pm_close_bcpu_done_callback(void *data);
int32 wlan_pm_halt_bcpu_done_callback(void *data);
void wlan_pm_deepsleep_delay_timeout(struct wlan_pm_s *pm_data);
int32 wlan_pm_stop_deepsleep_delay_timer(struct wlan_pm_s *pm_data);

int32 wlan_pm_stop_wdg(struct wlan_pm_s *pst_wlan_pm_info);
oal_int wlan_pm_work_submit(struct wlan_pm_s *pst_wlan_pm, oal_work_stru *pst_worker);
void wlan_pm_info_clean(void);


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wlan_pm_register_notifier(struct notifier_block *nb)
{
    return raw_notifier_chain_register(&wifi_pm_chain, nb);
}

oal_void wlan_pm_unregister_notifier(struct notifier_block *nb)
{
    raw_notifier_chain_unregister(&wifi_pm_chain, nb);
}
#endif
#ifdef CONFIG_HUAWEI_DSM

static struct dsm_dev dsm_wifi = {
    .name = "dsm_wifi",
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
    .fops = NULL,
    .buff_size = DSM_DEV_BUFF_SIZE,
};

struct dsm_client *hw_1102a_dsm_client = NULL;

void hw_1102a_register_wifi_dsm_client(void)
{
    if (hw_1102a_dsm_client == NULL) {
        hw_1102a_dsm_client = dsm_register_client(&dsm_wifi);
    }
}
void hw_1102a_unregister_wifi_dsm_client(void)
{
    if (hw_1102a_dsm_client != NULL) {
        dsm_unregister_client(hw_1102a_dsm_client, &dsm_wifi);
        hw_1102a_dsm_client = NULL;
    }
}
#define LOG_BUF_SIZE 512
void hw_1102a_dsm_client_notify(int dsm_id, const char *fmt, ...)
{
    char buf[LOG_BUF_SIZE] = {0};
    va_list ap;
    int size;

    DECLARE_DFT_TRACE_KEY_INFO("hw_1102a_dsm_client_notify", OAL_DFT_TRACE_FAIL);
    if (hw_1102a_dsm_client) {
        if (fmt != NULL) {
            va_start(ap, fmt);
            size = vsnprintf_s(buf, LOG_BUF_SIZE, LOG_BUF_SIZE - 1, fmt, ap);
            va_end(ap);
            if (size < 0) {
                OAM_WARNING_LOG0(0, OAM_SF_PWR, "buf copy failed");
                return;
            }
        }

        if (!dsm_client_ocuppy(hw_1102a_dsm_client)) {
            dsm_client_record(hw_1102a_dsm_client, buf);
            dsm_client_notify(hw_1102a_dsm_client, dsm_id);
            OAL_IO_PRINT("[I]wifi dsm client notify success, dsm_id=%d[%s]\n", dsm_id, buf);
        } else {
            dsm_client_unocuppy(hw_1102a_dsm_client);
            if (!dsm_client_ocuppy(hw_1102a_dsm_client)) {
                dsm_client_record(hw_1102a_dsm_client, buf);
                dsm_client_notify(hw_1102a_dsm_client, dsm_id);
                OAL_IO_PRINT("[I]wifi dsm notify success, dsm_id=%d[%s]\n", dsm_id, buf);
            } else {
                OAL_IO_PRINT("[E]wifi dsm client ocuppy, dsm notify failed, dsm_id=%d\n", dsm_id);
            }
        }
    } else {
        OAL_IO_PRINT("[E]wifi dsm client is null, dsm notify failed, dsm_id=%d\n", dsm_id);
    }
}
EXPORT_SYMBOL(hw_1102a_dsm_client_notify);
#endif
/*
 * 函 数 名  : wlan_pm_get_drv
 * 功能描述  : 获取全局wlan结构
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
struct wlan_pm_s *wlan_pm_get_drv(oal_void)
{
    return gpst_wlan_pm_info;
}

EXPORT_SYMBOL_GPL(wlan_pm_get_drv);

/*
 * 函 数 名  : wlan_pm_sleep_request
 * 功能描述  : 发送sleep 请求给device
 * 返 回 值  : SUCC/FAIL
 */
oal_int32 wlan_pm_sleep_request(struct wlan_pm_s *pst_wlan_pm)
{
    return hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_SLEEP_REQ);
}

/*
 * 函 数 名  : wlan_pm_allow_sleep_callback
 * 功能描述  : device应答allow_sleep消息处理
 */
oal_int32 wlan_pm_allow_sleep_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_allow_sleep_callback");

    pst_wlan_pm->ul_sleep_stage = SLEEP_ALLOW_RCV;
    OAL_COMPLETE(&pst_wlan_pm->st_sleep_request_ack);

    return SUCCESS;
}

/*
 * 函 数 名  : wlan_pm_disallow_sleep_callback
 * 功能描述  : device应答allow_sleep消息处理
 */
oal_int32 wlan_pm_disallow_sleep_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    if (oal_print_rate_limit(PRINT_RATE_SECOND)) { /* 1s打印一次 */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "sleep request dev disalow, device busy");
    }

    pst_wlan_pm->ul_sleep_stage = SLEEP_DISALLOW_RCV;
    OAL_COMPLETE(&pst_wlan_pm->st_sleep_request_ack);

    return SUCCESS;
}

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
OAL_STATIC hcc_switch_action plat_pm_switch_action;
OAL_STATIC oal_int32 wlan_switch_action_callback(oal_uint32 dev_id, hcc_bus *old_bus, hcc_bus *new_bus, oal_void *data)
{
    struct wlan_pm_s *pst_wlan_pm;

    if (data == NULL) {
        return -OAL_EINVAL;
    }

    if (HCC_CHIP_110X_DEV != dev_id) {
        /* ignore other wlan dev */
        return OAL_SUCC;
    }

    pst_wlan_pm = (struct wlan_pm_s *)data;

    /* Update new bus */
    pst_wlan_pm->pst_bus = new_bus;
    pst_wlan_pm->pst_bus->pst_pm_callback = &gst_wlan_pm_callback;

    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : wlan_pm_init
 * 功能描述  : WLAN PM初始化接口
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
struct wlan_pm_s *wlan_pm_init(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm;
    BOARD_INFO *pst_board = get_hi110x_board_info();

    if (OAL_PTR_NULL == pst_board) {
        return OAL_PTR_NULL;
    }

    pst_wlan_pm = kzalloc(sizeof(struct wlan_pm_s), GFP_KERNEL);
    if (pst_wlan_pm == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "no mem to allocate wlan_pm_data");
        return OAL_PTR_NULL;
    }

    memset_s(pst_wlan_pm, sizeof(struct wlan_pm_s), 0, sizeof(struct wlan_pm_s));
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    memset_s((oal_void *)&plat_pm_switch_action, OAL_SIZEOF(plat_pm_switch_action),
             0, OAL_SIZEOF(plat_pm_switch_action));
    plat_pm_switch_action.name = "plat_pm_wlan";
    plat_pm_switch_action.switch_notify = wlan_switch_action_callback;
    hcc_switch_action_register(&plat_pm_switch_action, (oal_void *)pst_wlan_pm);

    pst_wlan_pm->pst_bus = hcc_get_current_110x_bus();
    if (pst_wlan_pm->pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc bus is NULL, failed to create wlan_pm_wq!");
        kfree(pst_wlan_pm);
        return OAL_PTR_NULL;
    }
    pst_wlan_pm->pst_bus->pst_pm_callback = &gst_wlan_pm_callback;
#endif
    pst_wlan_pm->ul_wlan_pm_enable = OAL_FALSE;
    pst_wlan_pm->ul_apmode_allow_pm_flag = OAL_TRUE; /* 默认允许下电 */

    /* work queue初始化 */
    pst_wlan_pm->pst_pm_wq = oal_create_singlethread_workqueue("wlan_pm_wq");
    if (!pst_wlan_pm->pst_pm_wq) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "Failed to create wlan_pm_wq!");
        kfree(pst_wlan_pm);
        return OAL_PTR_NULL;
    }
    /* register wakeup and sleep work */
    OAL_INIT_WORK(&pst_wlan_pm->st_wakeup_work, wlan_pm_wakeup_work);
    OAL_INIT_WORK(&pst_wlan_pm->st_sleep_work, wlan_pm_sleep_work);

    /* 初始化芯片自检work */
    OAL_INIT_WORK(&pst_wlan_pm->st_ram_reg_test_work, wlan_device_mem_check_work);

    /* sleep timer初始化 */
    init_timer(&pst_wlan_pm->st_watchdog_timer);
    pst_wlan_pm->st_watchdog_timer.data = (uintptr_t)pst_wlan_pm;
    pst_wlan_pm->st_watchdog_timer.function = (void *)wlan_pm_wdg_timeout;
    pst_wlan_pm->ul_wdg_timeout_cnt = WLAN_SLEEP_DEFAULT_CHECK_CNT;
    pst_wlan_pm->ul_wdg_timeout_curr_cnt = 0;
    pst_wlan_pm->ul_packet_cnt = 0;
    pst_wlan_pm->ul_packet_total_cnt = 0;
    pst_wlan_pm->ul_packet_check_time = 0;
    pst_wlan_pm->ul_sleep_forbid_check_time = 0;

    /* sleep timer初始化 */
    init_timer(&pst_wlan_pm->st_deepsleep_delay_timer);
    pst_wlan_pm->st_deepsleep_delay_timer.data = (uintptr_t)pst_wlan_pm;
    pst_wlan_pm->st_deepsleep_delay_timer.function = (void *)wlan_pm_deepsleep_delay_timeout;

    oal_wake_lock_init(&pst_wlan_pm->st_deepsleep_wakelock, "wifi_deeepsleep_delay_wakelock");

    pst_wlan_pm->ul_wlan_power_state = POWER_STATE_SHUTDOWN;
    pst_wlan_pm->ul_wlan_dev_state = HOST_ALLOW_TO_SLEEP;
    pst_wlan_pm->ul_sleep_stage = SLEEP_STAGE_INIT;

    pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_get_pm_pause_func = OAL_PTR_NULL;
    pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify = OAL_PTR_NULL;
    pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_pm_state_notify = OAL_PTR_NULL;

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    pst_wlan_pm->ul_wkup_src_print_en = OAL_FALSE;
#endif
    gpst_wlan_pm_info = pst_wlan_pm;

    OAL_INIT_COMPLETION(&pst_wlan_pm->st_open_bcpu_done);
    OAL_INIT_COMPLETION(&pst_wlan_pm->st_close_bcpu_done);
    OAL_INIT_COMPLETION(&pst_wlan_pm->st_close_done);
    OAL_INIT_COMPLETION(&pst_wlan_pm->st_wakeup_done);
    OAL_INIT_COMPLETION(&pst_wlan_pm->st_wifi_powerup_done);
    OAL_INIT_COMPLETION(&pst_wlan_pm->st_sleep_request_ack);
    OAL_INIT_COMPLETION(&pst_wlan_pm->st_halt_bcpu_done);
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_WAKEUP_SUCC, wlan_pm_wakeup_done_callback, pst_wlan_pm);
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_ALLOW_SLEEP, wlan_pm_allow_sleep_callback, pst_wlan_pm);
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_DISALLOW_SLEEP, wlan_pm_disallow_sleep_callback, pst_wlan_pm);
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_POWEROFF_ACK, wlan_pm_close_done_callback, pst_wlan_pm);
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_OPEN_BCPU_ACK, wlan_pm_open_bcpu_done_callback, pst_wlan_pm);
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_CLOSE_BCPU_ACK, wlan_pm_close_bcpu_done_callback, pst_wlan_pm);
    hcc_message_register(hcc_get_110x_handler(), D2H_MSG_HALT_BCPU, wlan_pm_halt_bcpu_done_callback, pst_wlan_pm);

    pst_wlan_pm->pst_bus->data_int_count = 0;
    pst_wlan_pm->pst_bus->wakeup_int_count = 0;
#endif
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_init ok!");
    return pst_wlan_pm;
}

/*
 * 函 数 名  : wlan_pm_exit
 * 功能描述  : WLAN pm退出接口
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint wlan_pm_exit(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    wlan_pm_stop_wdg(pst_wlan_pm);

    wlan_pm_stop_deepsleep_delay_timer(pst_wlan_pm);

    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_WAKEUP_SUCC);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_WLAN_READY);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_ALLOW_SLEEP);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_DISALLOW_SLEEP);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_POWEROFF_ACK);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_OPEN_BCPU_ACK);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_CLOSE_BCPU_ACK);
    hcc_bus_message_unregister(pst_wlan_pm->pst_bus, D2H_MSG_HALT_BCPU);
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hcc_switch_action_unregister(&plat_pm_switch_action);
#endif

    kfree(pst_wlan_pm);

    gpst_wlan_pm_info = OAL_PTR_NULL;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_exit ok!");

    return OAL_SUCC;
}

/*
 * 函 数 名  : wlan_pm_is_poweron
 * 功能描述  : wifi是否上电
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint32 wlan_pm_is_poweron(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    if (pst_wlan_pm->ul_wlan_power_state == POWER_STATE_OPEN) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}
EXPORT_SYMBOL_GPL(wlan_pm_is_poweron);

/*
 * 函 数 名  : wlan_pm_get_wifi_srv_handler
 * 功能描述  : 获取回调handler指针
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
struct wifi_srv_callback_handler *wlan_pm_get_wifi_srv_handler(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return NULL;
    }

    return &pst_wlan_pm->st_wifi_srv_handler;
}
EXPORT_SYMBOL_GPL(wlan_pm_get_wifi_srv_handler);
/*
 * 函 数 名  : wlan_pm_idle_sleep_vote
 * 功能描述  : wlan投票是否允许kirin进入32k idle模式
 * 输入参数  : TRUE:允许，FALSE:不允许
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_void wlan_pm_idle_sleep_vote(oal_uint8 uc_allow)
{
#ifdef CONFIG_HISI_IDLE_SLEEP
    if (uc_allow == ALLOW_IDLESLEEP) {
        hisi_idle_sleep_vote(ID_WIFI, 0);
    } else {
        hisi_idle_sleep_vote(ID_WIFI, 1);
    }
#endif
}

oal_int32 hitalk_pm_open(oal_void)
{
    oal_int32 ret;
    hcc_bus *pst_bus;
    struct wlan_pm_s *pst_hitalk_pm = wlan_pm_get_drv();

    struct pm_drv_data *pm_data = pm_get_drvdata();

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "hitalk_pm_open enter");

    if ((pm_data == NULL) || (pst_hitalk_pm == OAL_PTR_NULL)) {
        OAM_ERROR_LOG2(0, OAM_SF_PWR, "hitalk_pm_open::pm_data[%lX] or pst_hitalk_pm[%lX] is NULL!",
                       (uintptr_t)pm_data, (uintptr_t)pst_hitalk_pm);
        return OAL_FAIL;
    }

    mutex_lock(&pm_data->host_mutex);

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "hitalk_pm_open::get 110x bus failed!");
        mutex_unlock(&pm_data->host_mutex);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NON_BUS);
        return OAL_FAIL;
    }

    if (pst_hitalk_pm->ul_wlan_power_state == POWER_STATE_OPEN) {
        mutex_unlock(&pm_data->host_mutex);
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "hitalk_pm_open::aleady opened");
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    if (!pst_hitalk_pm->pst_bus->st_bus_wakelock.lock_count) {
        /* make sure open only lock once */
        hcc_bus_wake_lock(pst_hitalk_pm->pst_bus);
        wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "hitalk_pm_idle_sleep_vote DISALLOW::hisi_idle_sleep_vote ID_WIFI 1!");
    }

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "hitalk_pm_open::get wakelock %lu!",
                     pst_hitalk_pm->pst_bus->st_bus_wakelock.lock_count);

    pst_hitalk_pm->ul_open_cnt++;

    if (OAL_SUCC != hitalk_power_on()) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "hitalk_pm_open::hitalk_power_on fail!");
        pst_hitalk_pm->ul_wlan_power_state = POWER_STATE_SHUTDOWN;
        hcc_bus_wake_unlock(pst_hitalk_pm->pst_bus);
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);
        mutex_unlock(&pm_data->host_mutex);
        DECLARE_DFT_TRACE_KEY_INFO("hitalk_power_on_fail", OAL_DFT_TRACE_FAIL);
        return OAL_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    // 初始化配置定制化参数
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    if (OAL_SUCC != hwifi_hcc_customize_h2d_data_cfg()) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "hitalk_pm_open::hwifi_hcc_customize_h2d_data_cfg fail");
    }
#else
    hwifi_custom_host_read_cfg_init();
#endif

    OAL_INIT_COMPLETION(&pst_bus->st_device_ready);

    if (OAL_PTR_NULL == custom_process_func.p_custom_cali_func) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "hitalk_pm_open::NO custom_process_func registered");
        hcc_bus_wake_unlock(pst_hitalk_pm->pst_bus);
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NO_CUSTOM_CALL);
        mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
    if (OAL_PTR_NULL == custom_process_func.p_priv_ini_config_func) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "hitalk_pm_open::NO p_priv_ini_config_func registered");
        hcc_bus_wake_unlock(pst_hitalk_pm->pst_bus);
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NO_CUSTOM_CALL);
        mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

    /* 私有定制化下发 */
    if (custom_process_func.p_priv_ini_config_func() != OAL_SUCC) {
        OAM_ERROR_LOG0(0, OAM_SF_CUSTOM, "hitalk_pm_open::p_priv_ini_config_func fail");
    }
#endif

    /* 如果校准下发成功则等待device ready；否则继续打开wifi */
    if (OAL_SUCC == custom_process_func.p_custom_cali_func()) {
#ifdef _PRE_WLAN_RF_AUTOCALI
        /* 开机不执行自动化校准 */
        if ((autocali_switch == OAL_FALSE) || (custom_cali_done == OAL_FALSE))
#endif
        {
            if (0 == oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
                                                     (oal_uint32)OAL_MSECS_TO_JIFFIES(HOST_WAIT_BOTTOM_INIT_TIMEOUT))) {
                DECLARE_DFT_TRACE_KEY_INFO("hitalk_wait_custom_cali_fail", OAL_DFT_TRACE_FAIL);

                OAM_ERROR_LOG1(0, OAM_SF_PWR, "hitalk_pm_open::hitalk_pm_wait_custom_cali timeout %d !!!!!!",
                               HOST_WAIT_BOTTOM_INIT_TIMEOUT);

                CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                     CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_CALL_TIMEOUT);

                mutex_unlock(&pm_data->host_mutex);
                if (OAL_TRUE == oal_trigger_bus_exception(pm_data->pst_wlan_pm_info->pst_bus, OAL_TRUE)) {
                    oal_print_hi11xx_log(HI11XX_LOG_WARN, "dump device mem when cali custom failed!");
                }
                return OAL_FAIL;
            }
        }
    }
#endif

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "hitalk_pm_open::wlan_pm_open SUCC!!");
    DECLARE_DFT_TRACE_KEY_INFO("hitalk_open_succ", OAL_DFT_TRACE_SUCC);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_POWERUP_EVENT, (oal_void *)pst_hitalk_pm); /* powerup chain */
#endif

    wlan_pm_enable();

    /* WIFI开机成功后,通知业务侧 */
    if (pst_hitalk_pm->st_wifi_srv_handler.p_wifi_srv_open_notify != OAL_PTR_NULL) {
        pst_hitalk_pm->st_wifi_srv_handler.p_wifi_srv_open_notify(OAL_TRUE);
    }

    /* 将timeout值恢复为默认值，并启动定时器 */
    wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);

    ret = hcc_dev_switch_enable(HCC_CHIP_110X_DEV);
    if (ret != OAL_SUCC) {
        DECLARE_DFT_TRACE_KEY_INFO("hcc_dev_switch_enable failed", OAL_DFT_TRACE_FAIL);
    }

    mutex_unlock(&pm_data->host_mutex);

    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(hitalk_pm_open);

oal_uint32 hitalk_pm_close(oal_void)
{
    struct wlan_pm_s *pst_hitalk_pm = wlan_pm_get_drv();

    struct pm_drv_data *pm_data = pm_get_drvdata();

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "hitalk_pm_close enter");

    if (pm_data == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "hitalk_pm_close,pm_data is NULL!");
        return OAL_FAIL;
    }

    if (pst_hitalk_pm == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "pst_hitalk_pm is null");
        return OAL_FAIL;
    }

    if (!pst_hitalk_pm->ul_apmode_allow_pm_flag) {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "hitalk_pm_close,AP mode,do not shutdown power.");
        return OAL_ERR_CODE_FOBID_CLOSE_DEVICE;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hitalk_pm_close start!!");

    mutex_lock(&pm_data->host_mutex);

    hcc_dev_switch_disable(HCC_CHIP_110X_DEV);

    pst_hitalk_pm->ul_close_cnt++;

    if (pst_hitalk_pm->ul_wlan_power_state == POWER_STATE_SHUTDOWN) {
        mutex_unlock(&pm_data->host_mutex);
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    /* WIFI关闭前,通知业务侧 */
    if (pst_hitalk_pm->st_wifi_srv_handler.p_wifi_srv_open_notify != OAL_PTR_NULL) {
        pst_hitalk_pm->st_wifi_srv_handler.p_wifi_srv_open_notify(OAL_FALSE);
    }

    wlan_pm_disable();

    wlan_pm_stop_wdg(pst_hitalk_pm);

    wlan_pm_stop_deepsleep_delay_timer(pst_hitalk_pm);

    wlan_pm_info_clean();

    /* mask rx ip data interrupt */
    hcc_bus_rx_int_mask(hcc_get_current_110x_bus());

    if (OAL_SUCC != hitalk_power_off()) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "hitalk_power_off FAIL!\n");
        mutex_unlock(&pm_data->host_mutex);
        DECLARE_DFT_TRACE_KEY_INFO("hitalk_power_off_fail", OAL_DFT_TRACE_FAIL);
        return OAL_FAIL;
    }

    pst_hitalk_pm->ul_wlan_power_state = POWER_STATE_SHUTDOWN;

    /* unmask rx ip data interrupt */
    hcc_bus_rx_int_unmask(hcc_get_current_110x_bus());

    hcc_bus_wake_unlock(pst_hitalk_pm->pst_bus);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_POWERDOWN_EVENT, (oal_void *)pst_hitalk_pm); /* powerdown chain */
#endif

    wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "hitalk_pm_close release wakelock %lu!\n",
                     pst_hitalk_pm->pst_bus->st_bus_wakelock.lock_count);

    hcc_bus_wakelocks_release_detect(pst_hitalk_pm->pst_bus);

    mutex_unlock(&pm_data->host_mutex);

    hcc_dev_flowctrl_on(hcc_get_110x_handler(), 0);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "hitalk_pm_close succ!\n");
    DECLARE_DFT_TRACE_KEY_INFO("hitalk_close_succ", OAL_DFT_TRACE_SUCC);

    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(hitalk_pm_close);

/*
 * 函 数 名  : wlan_pm_open
 * 功能描述  : open wifi,如果bfgx没有开启,上电,否则，下命令开WCPU
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_int32 wlan_pm_open(oal_void)
{
    oal_int32 ret;
    hcc_bus *pst_bus;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    struct pm_drv_data *pm_data = pm_get_drvdata();

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_open enter");

    if ((pm_data == NULL) || (pst_wlan_pm == OAL_PTR_NULL)) {
        OAM_ERROR_LOG2(0, OAM_SF_PWR, "wlan_pm_open::pm_data[%lX] or pst_wlan_pm[%lX] is NULL!",
                       (uintptr_t)pm_data, (uintptr_t)pst_wlan_pm);
        return OAL_FAIL;
    }

    mutex_lock(&pm_data->host_mutex);

    pst_bus = hcc_get_current_110x_bus();
    if (pst_bus == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_open::get 110x bus failed!");
        mutex_unlock(&pm_data->host_mutex);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NON_BUS);
        return OAL_FAIL;
    }

    if (pst_wlan_pm->ul_wlan_power_state == POWER_STATE_OPEN) {
        mutex_unlock(&pm_data->host_mutex);
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_open::aleady opened");
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    if (!pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count) {
        /* make sure open only lock once */
        hcc_bus_wake_lock(pst_wlan_pm->pst_bus);
        wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_idle_sleep_vote DISALLOW::hisi_idle_sleep_vote ID_WIFI 1!");
    }

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_open::get wakelock %lu!",
                     pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);

    pst_wlan_pm->ul_open_cnt++;

    if (OAL_SUCC != wlan_power_on()) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_open::wlan_power_on fail!");
        pst_wlan_pm->ul_wlan_power_state = POWER_STATE_SHUTDOWN;
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);
        mutex_unlock(&pm_data->host_mutex);
        DECLARE_DFT_TRACE_KEY_INFO("wlan_power_on_fail", OAL_DFT_TRACE_FAIL);
        return OAL_FAIL;
    }

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    // 初始化配置定制化参数
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    if (OAL_SUCC != hwifi_hcc_customize_h2d_data_cfg()) {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "wlan_pm_open::hwifi_hcc_customize_h2d_data_cfg fail");
    }
#else
    hwifi_custom_host_read_cfg_init();
#endif

    OAL_INIT_COMPLETION(&pst_bus->st_device_ready);

    if (OAL_PTR_NULL == custom_process_func.p_custom_cali_func) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_open::NO custom_process_func registered");
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NO_CUSTOM_CALL);
        mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
    if (OAL_PTR_NULL == custom_process_func.p_priv_ini_config_func) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_open::NO p_priv_ini_config_func registered");
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NO_CUSTOM_CALL);
        mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

    /* 私有定制化下发 */
    if (custom_process_func.p_priv_ini_config_func() != OAL_SUCC) {
        OAM_ERROR_LOG0(0, OAM_SF_CUSTOM, "wlan_pm_open::p_priv_ini_config_func fail");
    }
#endif

    /* 如果校准下发成功则等待device ready；否则继续打开wifi */
    if (OAL_SUCC == custom_process_func.p_custom_cali_func()) {
#ifdef _PRE_WLAN_RF_AUTOCALI
        /* 开机不执行自动化校准 */
        if ((autocali_switch == OAL_FALSE) || (custom_cali_done == OAL_FALSE))
#endif
        {
            if (0 == oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
                                                     (oal_uint32)OAL_MSECS_TO_JIFFIES(HOST_WAIT_BOTTOM_INIT_TIMEOUT))) {
                DECLARE_DFT_TRACE_KEY_INFO("wlan_wait_custom_cali_fail", OAL_DFT_TRACE_FAIL);

                OAM_ERROR_LOG1(0, OAM_SF_PWR, "wlan_pm_open::wlan_pm_wait_custom_cali timeout %d !!!!!!",
                               HOST_WAIT_BOTTOM_INIT_TIMEOUT);

                CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                     CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_CALL_TIMEOUT);

                mutex_unlock(&pm_data->host_mutex);
                if (OAL_TRUE == oal_trigger_bus_exception(pm_data->pst_wlan_pm_info->pst_bus, OAL_TRUE)) {
                    oal_print_hi11xx_log(HI11XX_LOG_WARN, "dump device mem when cali custom failed!");
                }
                return OAL_FAIL;
            }
        }
    }
#endif

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_open::wlan_pm_open SUCC!!");
    DECLARE_DFT_TRACE_KEY_INFO("wlan_open_succ", OAL_DFT_TRACE_SUCC);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_POWERUP_EVENT, (oal_void *)pst_wlan_pm); /* powerup chain */
#endif

    wlan_pm_enable();

    /* WIFI开机成功后,通知业务侧 */
    if (pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify != OAL_PTR_NULL) {
        pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify(OAL_TRUE);
    }

    /* 将timeout值恢复为默认值，并启动定时器 */
    wlan_pm_set_timeout(WLAN_SLEEP_DEFAULT_CHECK_CNT);

    ret = hcc_dev_switch_enable(HCC_CHIP_110X_DEV);
    if (ret != OAL_SUCC) {
        DECLARE_DFT_TRACE_KEY_INFO("hcc_dev_switch_enable failed", OAL_DFT_TRACE_FAIL);
    }

    mutex_unlock(&pm_data->host_mutex);

    return OAL_SUCC;
}
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
oal_int32 ram_reg_test_result = OAL_SUCC;
unsigned long long ram_reg_test_time = 0;
oal_int32 wlan_mem_check_mdelay = 1000;
oal_int32 bfgx_mem_check_mdelay = 1500;

wlan_memdump_t st_wlan_memdump_cfg = { 0x60000000, 0x1000 };

oal_uint32 set_wlan_mem_check_mdelay(int32 mdelay)
{
    wlan_mem_check_mdelay = mdelay;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "set_wlan_mem_check_mdelay::set delay:%dms!!", wlan_mem_check_mdelay);
    return 0;
}
oal_uint32 set_bfgx_mem_check_mdelay(int32 mdelay)
{
    bfgx_mem_check_mdelay = mdelay;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "bfgx_mem_check_mdelay::set delay:%dms!!", bfgx_mem_check_mdelay);
    return 0;
}

EXPORT_SYMBOL_GPL(set_wlan_mem_check_mdelay);

wlan_memdump_t *get_wlan_memdump_cfg(void)
{
    return &st_wlan_memdump_cfg;
}

oal_uint32 set_wlan_mem_check_memdump(int32 addr, int32 len)
{
    st_wlan_memdump_cfg.addr = addr;
    st_wlan_memdump_cfg.len = len;
    st_wlan_memdump_cfg.en = 1;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "set_wlan_mem_check_memdump set ok: addr:0x%x,len:%d", addr, len);
    return 0;
}

EXPORT_SYMBOL_GPL(set_wlan_mem_check_memdump);

oal_int32 wlan_device_mem_check(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (OAL_WARN_ON(!pst_wlan_pm)) {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "%s fail!:pst_wlan_pm is null \n", __FUNCTION__);
        return -OAL_FAIL;
    }

    ram_reg_test_result = OAL_SUCC;
    ram_reg_test_time = 0;
    hcc_bus_wake_lock(pst_wlan_pm->pst_bus);

    if (0 != wlan_pm_work_submit(pst_wlan_pm, &pst_wlan_pm->st_ram_reg_test_work)) {
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "st_ram_reg_test_work submit work fail !\n");
    }

    return OAL_SUCC;
}
oal_int32 wlan_device_mem_check_result(unsigned long long *time)
{
    *time = ram_reg_test_time;
    return ram_reg_test_result;
}
oal_void wlan_device_mem_check_work(oal_work_stru *pst_worker)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    mutex_lock(&pm_data->host_mutex);

    hcc_bus_disable_state(pm_data->pst_wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
    ram_reg_test_result = device_mem_check(&ram_reg_test_time);
    hcc_bus_enable_state(pm_data->pst_wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);

    hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);

    mutex_unlock(&pm_data->host_mutex);
}
EXPORT_SYMBOL_GPL(wlan_device_mem_check);
EXPORT_SYMBOL_GPL(wlan_device_mem_check_result);

#endif

EXPORT_SYMBOL_GPL(wlan_pm_open);

/*
 * 函 数 名  : wlan_pm_close
 * 功能描述  : close wifi,如果bfgx没有开,下电，否则下命令关WCPU
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint32 wlan_pm_close(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    struct pm_drv_data *pm_data = pm_get_drvdata();

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_close enter");

    if (pm_data == NULL) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_close,pm_data is NULL!");
        return OAL_FAIL;
    }

    if (pst_wlan_pm == OAL_PTR_NULL) {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "pst_wlan_pm is null");
        return OAL_FAIL;
    }

    if (!pst_wlan_pm->ul_apmode_allow_pm_flag) {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_close,AP mode,do not shutdown power.");
        return OAL_ERR_CODE_FOBID_CLOSE_DEVICE;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_close start!!");

    mutex_lock(&pm_data->host_mutex);

    hcc_dev_switch_disable(HCC_CHIP_110X_DEV);

    pst_wlan_pm->ul_close_cnt++;

    if (pst_wlan_pm->ul_wlan_power_state == POWER_STATE_SHUTDOWN) {
        mutex_unlock(&pm_data->host_mutex);
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    /* WIFI关闭前,通知业务侧 */
    if (pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify != OAL_PTR_NULL) {
        pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_open_notify(OAL_FALSE);
    }

    wlan_pm_disable();

    wlan_pm_stop_wdg(pst_wlan_pm);

    wlan_pm_stop_deepsleep_delay_timer(pst_wlan_pm);

    wlan_pm_info_clean();

    /* mask rx ip data interrupt */
    hcc_bus_rx_int_mask(hcc_get_current_110x_bus());

    if (OAL_SUCC != wlan_power_off()) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_power_off FAIL!\n");
        mutex_unlock(&pm_data->host_mutex);
        DECLARE_DFT_TRACE_KEY_INFO("wlan_power_off_fail", OAL_DFT_TRACE_FAIL);
        return OAL_FAIL;
    }

    pst_wlan_pm->ul_wlan_power_state = POWER_STATE_SHUTDOWN;

    /* unmask rx ip data interrupt */
    hcc_bus_rx_int_unmask(hcc_get_current_110x_bus());

    hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_POWERDOWN_EVENT, (oal_void *)pst_wlan_pm); /* powerdown chain */
#endif

    wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_close release wakelock %lu!\n",
                     pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);

    hcc_bus_wakelocks_release_detect(pst_wlan_pm->pst_bus);

    mutex_unlock(&pm_data->host_mutex);

    hcc_dev_flowctrl_on(hcc_get_110x_handler(), 0);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_close succ!\n");
    DECLARE_DFT_TRACE_KEY_INFO("wlan_close_succ", OAL_DFT_TRACE_SUCC);
    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(wlan_pm_close);

/*
 * 函 数 名  : wlan_pm_enable
 * 功能描述  : 使能wlan平台低功耗
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint32 wlan_pm_enable(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (!wlan_pm_switch) {
        return OAL_SUCC;
    }

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    if (pst_wlan_pm->ul_wlan_pm_enable == OAL_TRUE) {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_enable already enabled!");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_SUCC;
    }

    pst_wlan_pm->ul_wlan_pm_enable = OAL_TRUE;

    wlan_pm_feed_wdg();

    hcc_tx_transfer_unlock(hcc_get_110x_handler());

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_enable SUCC!");

    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(wlan_pm_enable);

/*
 * 函 数 名  : wlan_pm_disable_check_wakeup
 * 功能描述  : 去使能wlan平台检查唤醒
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint32 wlan_pm_disable_check_wakeup(oal_int32 flag)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    if (pst_wlan_pm->ul_wlan_pm_enable == OAL_FALSE) {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_disable already disabled!");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_SUCC;
    }

    if (flag == OAL_TRUE) {
        if (OAL_SUCC != wlan_pm_wakeup_dev()) {
            OAM_WARNING_LOG0(0, OAM_SF_PWR, "pm wake up dev fail!");
        }
    }

    pst_wlan_pm->ul_wlan_pm_enable = OAL_FALSE;

    hcc_tx_transfer_unlock(hcc_get_110x_handler());

    wlan_pm_stop_wdg(pst_wlan_pm);

    oal_cancel_work_sync(&pst_wlan_pm->st_wakeup_work);
    oal_cancel_work_sync(&pst_wlan_pm->st_sleep_work);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_disable SUCC!");

    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(wlan_pm_disable_check_wakeup);

oal_uint32 wlan_pm_disable(oal_void)
{
    return wlan_pm_disable_check_wakeup(OAL_TRUE);
}
EXPORT_SYMBOL_GPL(wlan_pm_disable);

oal_uint32 wlan_pm_statesave(oal_void)
{
    if (wlan_pm_switch) {
        return wlan_pm_disable();
    } else {
        return OAL_SUCC;
    }
}

EXPORT_SYMBOL_GPL(wlan_pm_statesave);

oal_uint32 wlan_pm_staterestore(oal_void)
{
    if (wlan_pm_switch) {
        return wlan_pm_enable();
    } else {
        return OAL_SUCC;
    }
}
EXPORT_SYMBOL_GPL(wlan_pm_staterestore);

/*
 * 函 数 名  : wlan_pm_init_dev
 * 功能描述  : 初始化device的状态
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint wlan_pm_init_dev(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_int ret;
    hcc_bus *pst_bus;

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    pst_bus = hcc_get_current_110x_bus();
    if (OAL_WARN_ON(pst_bus == NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_init get non bus!");

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_NON_BUS);
        return OAL_FAIL;
    }

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_init_dev!\n");

    pst_wlan_pm->ul_wlan_dev_state = HOST_DISALLOW_TO_SLEEP;

    /* wait for bus wakeup */
    ret = down_timeout(&pst_bus->sr_wake_sema, 6 * HZ);
    if (ret == -ETIME) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "host bus controller is not ready!");
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        DECLARE_DFT_TRACE_KEY_INFO("wifi_controller_wait_init_fail", OAL_DFT_TRACE_FAIL);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_WIFI_DRV_ERROR_POWER_ON_SDIO_NO_READY);
        return OAL_FAIL;
    }
    up(&pst_bus->sr_wake_sema);

    return (oal_uint)hcc_bus_wakeup_request(pst_wlan_pm->pst_bus);
}

static void ssi_dump_for_wlan_pm_wakeup_dev_func_only(oal_void)
{
    if (HI1XX_ANDROID_BUILD_VARIANT_USER == hi11xx_get_android_build_variant()) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "dump when dfr");
    } else {
        if (oal_print_rate_limit(30 * PRINT_RATE_SECOND)) {  /* 30s打印一次 */
            ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL | SSI_MODULE_MASK_WCTRL);
        }
    }
}

/*
 * 函 数 名  : wlan_pm_wakeup_dev
 * 功能描述  : 唤醒device
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint wlan_pm_wakeup_dev(oal_void)
{
    oal_uint32 ul_ret;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_int32 ret;
    oal_uint8 uc_retry;
    oal_uint8 uc_wakeup_retry = 0;
    static oal_uint8 uc_wakeup_err_count = 0;
    hcc_bus *pst_bus;
    ktime_t time_start, time_stop;
    oal_uint64 trans_us;

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    if (pst_wlan_pm->ul_wlan_pm_enable == OAL_FALSE) {
        if (pst_wlan_pm->ul_wlan_dev_state == HOST_ALLOW_TO_SLEEP) {
            /* 唤醒流程没走完不允许发送数据 */
            return OAL_EFAIL;
        } else {
            return OAL_SUCC;
        }
    }

    if (pst_wlan_pm->ul_wlan_dev_state == HOST_DISALLOW_TO_SLEEP) {
        return OAL_SUCC;
    }

    pst_bus = hcc_get_current_110x_bus();
    if (OAL_WARN_ON(pst_bus == NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_wakeup_dev get non bus!\n");

        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WAKEUP_DEV);
        return OAL_FAIL;
    }
    wlan_pm_stop_deepsleep_delay_timer(pst_wlan_pm);

wakeup_again:
    time_start = ktime_get();
    hcc_bus_wake_lock(pst_wlan_pm->pst_bus);
    wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);

    OAL_INIT_COMPLETION(&pst_wlan_pm->st_wakeup_done);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wait bus wakeup");

    /* wait for bus wakeup */
    ret = down_timeout(&pst_bus->sr_wake_sema, 6 * HZ);
    if (ret == -ETIME) {
        pst_wlan_pm->ul_wakeup_fail_wait_sdio++;
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wifi controller is not ready!");
        DECLARE_DFT_TRACE_KEY_INFO("wifi_controller_wait_fail", OAL_DFT_TRACE_FAIL);
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        goto wakeup_fail;
    }
    up(&pst_bus->sr_wake_sema);

    if (pst_wlan_pm->pst_bus->bus_type == HCC_BUS_PCIE) {
        /* 依赖回来的GPIO 做唤醒，此时回来的消息PCIE 还不确定是否已经唤醒，
           PCIE通道不可用 */
        oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_FALSE);
        oal_atomic_set(&wakeup_dev_wait_ack, 1);
        oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_TRUE);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "wifi wakeup cmd send,wakelock cnt %lu",
                             pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);
        ret = hcc_bus_wakeup_request(pst_wlan_pm->pst_bus);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "wakeup request failed ret=%d", ret);
            DECLARE_DFT_TRACE_KEY_INFO("wifi wakeup cmd send fail", OAL_DFT_TRACE_FAIL);
            ssi_dump_for_wlan_pm_wakeup_dev_func_only();
            hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
            goto wakeup_fail;
        }

        pst_wlan_pm->ul_wlan_dev_state = HOST_DISALLOW_TO_SLEEP;
    } else if (pst_wlan_pm->pst_bus->bus_type == HCC_BUS_SDIO) {
#ifdef _PRE_PLAT_FEATURE_HI110X_SDIO_GPIO_WAKE
        /* use gpio to wakeup sdio device
          1.触发上升沿
          2.sdio wakeup 寄存器写0,写0会取消sdio mem pg功能 */
        for (uc_retry = 0; uc_retry < WLAN_SDIO_MSG_RETRY_NUM; uc_retry++) {
            OAL_INIT_COMPLETION(&pst_wlan_pm->st_wakeup_done);
            oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_FALSE);
            oal_atomic_set(&wakeup_dev_wait_ack, 1);
            oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_TRUE);
            board_host_wakeup_dev_set(0); /* wakeup dev */
            oal_udelay(100);
            board_host_wakeup_dev_set(1); /* wakeup dev */
            ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_wakeup_done,
                                                     (oal_uint32)OAL_MSECS_TO_JIFFIES(WLAN_WAKUP_MSG_WAIT_TIMEOUT));
            if (ul_ret != 0) {
                /* sdio gpio wakeup dev sucess */
                DECLARE_DFT_TRACE_KEY_INFO("gpio_wakeup_sdio_succ", OAL_DFT_TRACE_SUCC);
                break;
            }
        }

        if (uc_retry == WLAN_SDIO_MSG_RETRY_NUM) {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "oal_sdio_gpio_wakeup_dev retry %d failed", uc_retry);
            DECLARE_DFT_TRACE_KEY_INFO("oal_sdio_gpio_wakeup_dev final fail", OAL_DFT_TRACE_EXCEP);
            ssi_dump_for_wlan_pm_wakeup_dev_func_only();
            hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
            goto wakeup_fail;
        }

        hcc_bus_enable_state(pst_wlan_pm->pst_bus, OAL_BUS_STATE_ALL);

        OAL_INIT_COMPLETION(&pst_wlan_pm->st_wakeup_done);
#endif
        /* set sdio register */
        oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_FALSE);

        oal_print_hi11xx_log(HI11XX_LOG_INFO, "wifi wakeup cmd send,wakelock cnt %lu",
                             pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);
        ret = hcc_bus_wakeup_request(pst_wlan_pm->pst_bus);
        if (ret != OAL_SUCC) {
            DECLARE_DFT_TRACE_KEY_INFO("wifi wakeup cmd send fail", OAL_DFT_TRACE_FAIL);
            for (uc_retry = 0; uc_retry < WLAN_SDIO_MSG_RETRY_NUM; uc_retry++) {
                msleep(10);
                ret = hcc_bus_wakeup_request(pst_wlan_pm->pst_bus);
                if (ret == OAL_SUCC) {
                    break;
                }

                ssi_dump_for_wlan_pm_wakeup_dev_func_only();
                OAM_ERROR_LOG2(0, OAM_SF_PWR, "oal_wifi_wakeup_dev retry %d ret = %d", uc_retry, ret);
                DECLARE_DFT_TRACE_KEY_INFO("wifi wakeup cmd send retry fail", OAL_DFT_TRACE_FAIL);
            }

            /* after max retry still fail,log error */
            if (ret != OAL_SUCC) {
                pst_wlan_pm->ul_wakeup_fail_set_reg++;
                OAM_ERROR_LOG1(0, OAM_SF_PWR, "oal_wifi_wakeup_dev Fail ret = %d", ret);
                DECLARE_DFT_TRACE_KEY_INFO("oal_wifi_wakeup_dev final fail", OAL_DFT_TRACE_EXCEP);
                oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_TRUE);
                hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
                goto wakeup_fail;
            }
        }

        oal_atomic_set(&wakeup_dev_wait_ack, 1);

        pst_wlan_pm->ul_wlan_dev_state = HOST_DISALLOW_TO_SLEEP;
        oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_TRUE);
        up(&pst_wlan_pm->pst_bus->rx_sema);
    } else {
        DECLARE_DFT_TRACE_KEY_INFO("oal_wifi_wakeup_dev final fail", OAL_DFT_TRACE_EXCEP);
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        goto wakeup_fail;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (!is_dfr_test_en(WIFI_WKUP_FAULT)) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "[dfr test] trigger wkup fail!\n");
        uc_wakeup_err_count = WLAN_WAKEUP_FAIL_MAX_TIMES;
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
        goto wakeup_fail;
    }
#endif

    ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_wakeup_done,
                                             (oal_uint32)OAL_MSECS_TO_JIFFIES(WLAN_WAKUP_MSG_WAIT_TIMEOUT));
    if (ul_ret == 0) {
        oal_int32 sleep_state = hcc_bus_get_sleep_state(pst_wlan_pm->pst_bus);
        if ((DISALLOW_TO_SLEEP_VALUE == sleep_state) || (sleep_state < 0)) {
            if (OAL_UNLIKELY(sleep_state < 0)) {
                OAM_ERROR_LOG1(0, OAM_SF_PWR, "get state failed, sleep_state=%d", sleep_state);
            }

            pst_wlan_pm->ul_wakeup_fail_timeout++;
            OAM_WARNING_LOG0(0, OAM_SF_PWR, "oal_wifi_wakeup_dev SUCC to set 0xf0 = 0");
            hcc_bus_sleep_request(pst_wlan_pm->pst_bus);
            pst_wlan_pm->ul_wlan_dev_state = HOST_ALLOW_TO_SLEEP;
            hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
            if (0 == uc_wakeup_retry) {
                OAM_WARNING_LOG1(0, OAM_SF_PWR,
                                 "wlan_pm_wakeup_dev wait device complete fail,wait time %d ms!,try again",
                                 WLAN_WAKUP_MSG_WAIT_TIMEOUT);
                uc_wakeup_retry++;
                goto wakeup_again;
            } else {
                OAM_ERROR_LOG2(0, OAM_SF_PWR, "wlan_pm_wakeup_dev [%d]wait device complete fail,wait time %d ms!",
                               uc_wakeup_err_count, WLAN_WAKUP_MSG_WAIT_TIMEOUT);
                oal_print_hi11xx_log(HI11XX_LOG_INFO,
                                     KERN_ERR "wlan_pm_wakeup_dev [%d]wait device complete fail,wait time %d ms!",
                                     uc_wakeup_err_count, WLAN_WAKUP_MSG_WAIT_TIMEOUT);
                goto wakeup_fail;
            }
        } else {
            pst_wlan_pm->ul_wakeup_fail_set_reg++;
            OAM_ERROR_LOG0(0, OAM_SF_PWR, "wakeup_dev Fail to set 0xf0 = 0");
            oal_print_hi11xx_log(HI11XX_LOG_INFO, KERN_ERR "wakeup_dev Fail to set 0xf0 = 0");
            pst_wlan_pm->ul_wlan_dev_state = HOST_ALLOW_TO_SLEEP;
            hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
            goto wakeup_fail;
        }
    }

    pst_wlan_pm->ul_wakeup_succ++;
    DECLARE_DFT_TRACE_KEY_INFO("wlan_wakeup_succ", OAL_DFT_TRACE_SUCC);

    pst_wlan_pm->ul_wdg_timeout_curr_cnt = 0;
    pst_wlan_pm->ul_packet_cnt = 0;
    pst_wlan_pm->ul_packet_check_time = jiffies + msecs_to_jiffies(WLAN_PACKET_CHECK_TIME);
    pst_wlan_pm->ul_packet_total_cnt = 0;

    /* HOST WIFI退出低功耗,通知业务侧开启定时器 */
    if (pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_pm_state_notify != OAL_PTR_NULL) {
        pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_pm_state_notify(OAL_TRUE);
    }

    wlan_pm_feed_wdg();

    uc_wakeup_err_count = 0;

    oal_usleep_range(500, 510);

    hcc_bus_wakeup_complete(pst_wlan_pm->pst_bus);

    pst_wlan_pm->ul_sleep_fail_forbid_cnt = 0;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_WAKEUP_EVENT, (oal_void *)pst_wlan_pm); /* wakeup chain */
#endif

    time_stop = ktime_get();
    trans_us = (oal_uint64)ktime_to_us(ktime_sub(time_stop, time_start));
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wakeup dev succ, cost %llu us", trans_us);
    return OAL_SUCC;
wakeup_fail:
    DECLARE_DFT_TRACE_KEY_INFO("wlan_wakeup_fail", OAL_DFT_TRACE_FAIL);
    uc_wakeup_err_count++;

    /* pm唤醒失败超出门限，启动dfr流程 */
    if (uc_wakeup_err_count > WLAN_WAKEUP_FAIL_MAX_TIMES) {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "Now ready to enter DFR process after [%d]times wlan_wakeup_fail!",
                       uc_wakeup_err_count);
        uc_wakeup_err_count = 0;
        hcc_bus_exception_submit(pst_wlan_pm->pst_bus, WIFI_WAKEUP_FAIL);
    }
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WAKEUP_DEV);

    return OAL_FAIL;
}

oal_void wlan_pm_wakeup_dev_ack(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm;

    if (oal_atomic_read(&wakeup_dev_wait_ack)) {
        pst_wlan_pm = wlan_pm_get_drv();

        if (pst_wlan_pm == OAL_PTR_NULL) {
            return;
        }

        pst_wlan_pm->ul_wakeup_dev_ack++;

        OAL_COMPLETE(&pst_wlan_pm->st_wakeup_done);

        oal_atomic_set(&wakeup_dev_wait_ack, 0);

    }

    return;
}

/*
 * 函 数 名  : wlan_pm_open_bcpu
 * 功能描述  : 唤醒BCPU
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint wlan_pm_open_bcpu(oal_void)
{
#define RETRY_TIMES 3
    oal_uint32 i;
    oal_int32 ret = OAL_FAIL;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_int32 ul_ret;

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    /* 通过sdio配置命令，解复位BCPU */
    OAM_WARNING_LOG0(0, OAM_SF_PWR, "open BCPU");

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    for (i = 0; i < RETRY_TIMES; i++) {
        ret = wlan_pm_wakeup_dev();
        if (ret == OAL_SUCC) {
            break;
        }
    }

    if (ret != OAL_SUCC) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_wakeup_dev fail!");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_FAIL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_wakeup_dev succ, retry times [%d]", i);

    OAL_INIT_COMPLETION(&pst_wlan_pm->st_open_bcpu_done);

    ret = hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_RESET_BCPU);
    if (ret == OAL_SUCC) {
        /* 等待device执行命令 */
        up(&pst_wlan_pm->pst_bus->rx_sema);
        ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_open_bcpu_done,
                                                 (oal_uint32)OAL_MSECS_TO_JIFFIES(WLAN_OPEN_BCPU_WAIT_TIMEOUT));
        if (ul_ret == 0) {
            OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_open_bcpu wait device ACK timeout !");
            hcc_tx_transfer_unlock(hcc_get_110x_handler());
            return OAL_FAIL;
        }

        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_SUCC;
    } else {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "fail to send H2D_MSG_RESET_BCPU, ret=%d", ret);
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_FAIL;
    }
}

/*
 * 函 数 名  : wlan_pm_wakeup_host
 * 功能描述  : device唤醒host
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_uint wlan_pm_wakeup_host(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (OAL_WARN_ON(!pst_wlan_pm)) {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "%s fail!:pst_wlan_pm is null \n", __FUNCTION__);
        return -OAL_FAIL;
    }

    hcc_bus_wake_lock(pst_wlan_pm->pst_bus);
    wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);
    OAM_INFO_LOG1(0, OAM_SF_PWR, "wlan_pm_wakeup_host get wakelock %lu!\n",
                  pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);

    if (0 != wlan_pm_work_submit(pst_wlan_pm, &pst_wlan_pm->st_wakeup_work)) {
        pst_wlan_pm->ul_wakeup_fail_submit_work++;
        hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
    } else {
        pst_wlan_pm->ul_wakeup_succ_work_submit++;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wlan_pm_work_submit
 * 功能描述  : 提交一个kernel work
 * 返 回 值  : 初始化返回值，成功或失败原因
 */

oal_int wlan_pm_work_submit(struct wlan_pm_s *pst_wlan_pm, oal_work_stru *pst_worker)
{
    oal_int i_ret = 0;

    if (oal_work_is_busy(pst_worker)) {
        /* If comm worker is processing,
          we need't submit again */
        i_ret = -OAL_EBUSY;
        goto done;
    } else {
        OAM_INFO_LOG1(0, OAM_SF_PWR, "WiFi %lX Worker Submit\n", (uintptr_t)pst_worker->func);
        if (false == queue_work(pst_wlan_pm->pst_pm_wq, pst_worker)) {
            i_ret = -OAL_EFAIL;
        }
    }
done:
    return i_ret;
}

/*
 * 函 数 名  : wlan_pm_wakeup_work
 * 功能描述  : device唤醒host work
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
void wlan_pm_wakeup_work(oal_work_stru *pst_worker)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_uint l_ret;

    OAM_INFO_LOG0(0, OAM_SF_PWR, "wlan_pm_wakeup_work start!\n");

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    l_ret = wlan_pm_wakeup_dev();
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        DECLARE_DFT_TRACE_KEY_INFO("wlan_wakeup_fail", OAL_DFT_TRACE_FAIL);
    }

    hcc_tx_transfer_unlock(hcc_get_110x_handler());

    /* match for the work submit */
    hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
    DECLARE_DFT_TRACE_KEY_INFO("wlan_d2h_wakeup_succ", OAL_DFT_TRACE_SUCC);
    OAM_INFO_LOG1(0, OAM_SF_PWR, "wlan_pm_wakeup_work release wakelock %lu!\n",
                  pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);

    return;
}

/*
 * 函 数 名  : wlan_pm_wakeup_done_callback
 * 功能描述  : device应答wakeup succ消息处理
 */

int32 wlan_pm_wakeup_done_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    OAM_INFO_LOG0(0, OAM_SF_PWR, "wlan_pm_wakeup_done_callback !");

    pst_wlan_pm->ul_wakeup_done_callback++;

    wlan_pm_wakeup_dev_ack();

    return SUCCESS;
}

/*
 * 函 数 名  : wlan_pm_close_done_callback
 * 功能描述  : device应答poweroff ack消息处理
 */

int32 wlan_pm_close_done_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_close_done_callback !");

    /* 关闭RX通道，防止SDIO RX thread继续访问SDIO */
    hcc_bus_disable_state(hcc_get_current_110x_bus(), OAL_BUS_STATE_RX);

    pst_wlan_pm->ul_close_done_callback++;
    OAL_COMPLETE(&pst_wlan_pm->st_close_done);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "complete H2D_MSG_PM_WLAN_OFF done!");

    return SUCCESS;
}

/*
 * 函 数 名  : wlan_pm_open_bcpu_done_callback
 * 功能描述  : device应答open bcpu ack消息处理
 */

int32 wlan_pm_open_bcpu_done_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_open_bcpu_done_callback !");

    pst_wlan_pm->ul_open_bcpu_done_callback++;
    OAL_COMPLETE(&pst_wlan_pm->st_open_bcpu_done);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "complete H2D_MSG_RESET_BCPU done!");

    return SUCCESS;
}

/*
 * 函 数 名  : wlan_pm_close_bcpu_done_callback
 * 功能描述  : device应答open bcpu ack消息处理
 */

int32 wlan_pm_close_bcpu_done_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_close_bcpu_done_callback !");

    pst_wlan_pm->ul_close_bcpu_done_callback++;
    OAL_COMPLETE(&pst_wlan_pm->st_close_bcpu_done);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "complete H2D_MSG_PM_BCPU_OFF done!");

    return SUCCESS;
}

/*
 * 函 数 名  : wlan_pm_halt_bcpu_done_callback
 * 功能描述  : device应答open bcpu ack消息处理
 */

int32 wlan_pm_halt_bcpu_done_callback(void *data)
{
    struct wlan_pm_s *pst_wlan_pm = (struct wlan_pm_s *)data;

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_halt_bcpu_done_callback !");

    OAL_COMPLETE(&pst_wlan_pm->st_halt_bcpu_done);

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "complete wlan_pm_halt_bcpu_done_callback done!");

    return SUCCESS;
}

OAL_STATIC oal_void sleep_request_host_forbid_print(struct wlan_pm_s *pst_wlan_pm,
                                                    const oal_uint32 ul_host_forbid_sleep_limit)
{
    if (pst_wlan_pm->ul_sleep_request_host_forbid >= ul_host_forbid_sleep_limit) {
        /* 防止频繁打印 */
        if (oal_print_rate_limit(10 * PRINT_RATE_SECOND)) { /* 10s打印一次 */
            oal_int32 allow_print;
            OAM_WARNING_LOG2(0, OAM_SF_PWR, "wlan_pm_sleep_work_etc host forbid sleep %ld, forbid_cnt:%u",
                             pst_wlan_pm->ul_sleep_stage, pst_wlan_pm->ul_sleep_request_host_forbid);
            allow_print = oal_print_rate_limit(10 * PRINT_RATE_MINUTE); /* 10分钟打印一次 */
            hcc_bus_print_trans_info(pst_wlan_pm->pst_bus,
                                     allow_print ?
                                     (HCC_PRINT_TRANS_FLAG_DEVICE_STAT | HCC_PRINT_TRANS_FLAG_DEVICE_REGS) : 0x0);
        }
    } else {
        /* 防止频繁打印 */
        if (oal_print_rate_limit(10 * PRINT_RATE_SECOND)) { /* 10s打印一次 */
            OAM_WARNING_LOG2(0, OAM_SF_PWR, "wlan_pm_sleep_work_etc host forbid sleep %ld, forbid_cnt:%u",
                             pst_wlan_pm->ul_sleep_stage, pst_wlan_pm->ul_sleep_request_host_forbid);
        }
    }
}
oal_uint64 old_tx, old_rx;
oal_uint64 new_tx, new_rx;
/*
 * 函 数 名  : wlan_pm_sleep_work
 * 功能描述  : pm睡眠
 * 返 回 值  : 成功或失败原因
 */
void wlan_pm_sleep_work(oal_work_stru *pst_worker)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_int32 l_ret;
    oal_uint32 ul_ret;
    oal_uint8 uc_retry;
    oal_bool_enum_uint8 en_wifi_pause_pm = OAL_FALSE;
    static oal_uint8 uc_fail_sleep_count = 0;
    const oal_uint32 ul_host_forbid_sleep_limit = 10;

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    if (pst_wlan_pm->ul_wlan_pm_enable == OAL_FALSE) {
        wlan_pm_feed_wdg();
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return;
    }
    /* 协议栈回调获取是否pause低功耗 */
    if (pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_get_pm_pause_func) {
        en_wifi_pause_pm = pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_get_pm_pause_func();
    }

    if (en_wifi_pause_pm == OAL_TRUE) {
        wlan_pm_feed_wdg();
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return;
    }
    if (pst_wlan_pm->ul_wlan_dev_state == HOST_ALLOW_TO_SLEEP) {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "wakeuped,ne ed not do again");
        wlan_pm_feed_wdg();
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return;
    }

    pst_wlan_pm->ul_sleep_stage = SLEEP_REQ_SND;

    OAL_INIT_COMPLETION(&pst_wlan_pm->st_sleep_request_ack);

    if (oal_print_rate_limit(PRINT_RATE_MINUTE)) { /* 1分钟打印一次 */
        hcc_bus_chip_info(pst_wlan_pm->pst_bus, OAL_FALSE, OAL_FALSE);
    }
    l_ret = wlan_pm_sleep_request(pst_wlan_pm);
    if (l_ret != OAL_SUCC) {
        pst_wlan_pm->ul_sleep_fail_request++;
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_sleep_request fail !\n");
        goto fail_sleep;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "sleep request send!");
    up(&pst_wlan_pm->pst_bus->rx_sema);

    ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_sleep_request_ack,
                                             (oal_uint32)OAL_MSECS_TO_JIFFIES(WLAN_SLEEP_MSG_WAIT_TIMEOUT));
    if (ul_ret == 0) {
        pst_wlan_pm->ul_sleep_fail_wait_timeout++;
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_sleep_work wait completion fail !\n");
        goto fail_sleep;
    }

    if (pst_wlan_pm->ul_sleep_stage == SLEEP_ALLOW_RCV) {
        /* check host */
        l_ret = hcc_bus_sleep_request_host(pst_wlan_pm->pst_bus);
        if (l_ret != OAL_SUCC) {
            pst_wlan_pm->ul_sleep_request_host_forbid++;
            DECLARE_DFT_TRACE_KEY_INFO("wlan_forbid_sleep_host", OAL_DFT_TRACE_SUCC);
            sleep_request_host_forbid_print(pst_wlan_pm, ul_host_forbid_sleep_limit);
            goto sleep_forbid;
        } else {
            pst_wlan_pm->ul_sleep_request_host_forbid = 0;
        }
        oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_FALSE);

        pst_wlan_pm->ul_wlan_dev_state = HOST_ALLOW_TO_SLEEP;

        l_ret = hcc_bus_sleep_request(pst_wlan_pm->pst_bus);
        if (oal_print_rate_limit(PRINT_RATE_SECOND)) { /* 1s打印一次 */
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "wifi sleep cmd send ,wakelock cnt %lu",
                                 pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);
        }
        if (l_ret != OAL_SUCC) {
            for (uc_retry = 0; uc_retry < WLAN_SDIO_MSG_RETRY_NUM; uc_retry++) {
                msleep(10);
                l_ret = hcc_bus_sleep_request(pst_wlan_pm->pst_bus);
                ;
                if (l_ret == OAL_SUCC) {
                    break;
                }
                OAM_ERROR_LOG2(0, OAM_SF_PWR, "sleep_dev retry %d ret = %d", uc_retry, l_ret);
            }

            /* after max retry still fail,log error */
            if (l_ret != OAL_SUCC) {
                pst_wlan_pm->ul_sleep_fail_set_reg++;
                DECLARE_DFT_TRACE_KEY_INFO("wlan_sleep_cmd_fail", OAL_DFT_TRACE_FAIL);
                OAM_ERROR_LOG1(0, OAM_SF_PWR, "sleep_dev Fail ret = %d\r\n", l_ret);
                pst_wlan_pm->ul_wlan_dev_state = HOST_DISALLOW_TO_SLEEP;
                oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_TRUE);
                goto fail_sleep;
            }
        }

        oal_wlan_gpio_intr_enable(HBUS_TO_DEV(pst_wlan_pm->pst_bus), OAL_TRUE);

        pst_wlan_pm->ul_sleep_fail_forbid_cnt = 0;
        pst_wlan_pm->ul_sleep_fail_forbid = 0;
    } else {
        pst_wlan_pm->ul_sleep_fail_forbid++;
        if (pst_wlan_pm->ul_sleep_fail_forbid == 1) {
            pst_wlan_pm->ul_sleep_forbid_check_time = jiffies + msecs_to_jiffies(WLAN_SLEEP_FORBID_CHECK_TIME);
        } else if ((pst_wlan_pm->ul_sleep_fail_forbid != 0) &&
                   (time_after(jiffies, pst_wlan_pm->ul_sleep_forbid_check_time))) {
            /* 暂时连续2分钟被forbid sleep，上报一次CHR，看大数据再决定做不做DFR */
            CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                                 CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_SLEEP_FORBID);
            pst_wlan_pm->ul_sleep_fail_forbid = 0;
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "device_forbid_sleep for %d second",
                                 WLAN_SLEEP_FORBID_CHECK_TIME / 1000);
            if (OAL_TRUE == oal_trigger_bus_exception(pst_wlan_pm->pst_bus, OAL_TRUE)) {
                oal_print_hi11xx_log(HI11XX_LOG_WARN, "tigger dump device mem for device_forbid_sleep %d second",
                                     WLAN_SLEEP_FORBID_CHECK_TIME / 1000);
            }
        }
        pst_wlan_pm->ul_sleep_fail_forbid_cnt++;
        if (pst_wlan_pm->ul_sleep_fail_forbid_cnt <= 1) {
            /* get hcc trans count */
            hcc_bus_get_trans_count(pst_wlan_pm->pst_bus, &old_tx, &old_rx);
        } else {
            /* ul_sleep_fail_forbid_cnt > 1 */
            hcc_bus_get_trans_count(pst_wlan_pm->pst_bus, &new_tx, &new_rx);
            /* trans pending */
            if (pst_wlan_pm->ul_sleep_fail_forbid_cnt >= wlan_sleep_request_forbid_limit) {
                /* maybe device memleak */
                DECLARE_DFT_TRACE_KEY_INFO("wlan_forbid_sleep_print_info", OAL_DFT_TRACE_SUCC);
                OAM_WARNING_LOG2(0, OAM_SF_PWR,
                                 "wlan_pm_sleep_work device forbid sleep %ld, forbid_cnt:%u try dump device mem info",
                                 pst_wlan_pm->ul_sleep_stage, pst_wlan_pm->ul_sleep_fail_forbid_cnt);
                OAM_WARNING_LOG4(0, OAM_SF_PWR, "old[tx:%u rx:%u] new[tx:%u rx:%u]", old_tx, old_rx, new_tx, new_rx);
                pst_wlan_pm->ul_sleep_fail_forbid_cnt = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                hcc_print_current_trans_info();
#endif
                hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_DEVICE_MEM_DUMP);
            } else if ((pst_wlan_pm->ul_sleep_fail_forbid_cnt % (wlan_sleep_request_forbid_limit / 10)) == 0) {
                oal_print_hi11xx_log(HI11XX_LOG_INFO,
                                     "sleep request too many forbid %ld, device busy, forbid_cnt:%u, old[tx:%u rx:%u] new[tx:%u rx:%u]",
                                     pst_wlan_pm->ul_sleep_stage, pst_wlan_pm->ul_sleep_fail_forbid_cnt,
                                     (oal_uint32)old_tx, (oal_uint32)old_rx, (oal_uint32)new_tx, (oal_uint32)new_rx);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_DBG,
                                     "sleep request forbid %ld, device busy, forbid_cnt:%u, old[tx:%u rx:%u] new[tx:%u rx:%u]",
                                     pst_wlan_pm->ul_sleep_stage, pst_wlan_pm->ul_sleep_fail_forbid_cnt,
                                     (oal_uint32)old_tx, (oal_uint32)old_rx, (oal_uint32)new_tx, (oal_uint32)new_rx);
            }
        }

        DECLARE_DFT_TRACE_KEY_INFO("wlan_forbid_sleep", OAL_DFT_TRACE_SUCC);
        goto sleep_forbid;
    }

    pst_wlan_pm->ul_sleep_stage = SLEEP_CMD_SND;

    /* 继续持锁500ms, 防止系统频繁进入退出PM */
    oal_wake_lock(&pst_wlan_pm->st_deepsleep_wakelock);

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_sleep_work hold deepsleep_wakelock....%lu",
                         pst_wlan_pm->st_deepsleep_wakelock.lock_count);
    mod_timer(&pst_wlan_pm->st_deepsleep_delay_timer, jiffies + msecs_to_jiffies(WLAN_WAKELOCK_HOLD_TIME));

    hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
    hcc_tx_transfer_unlock(hcc_get_110x_handler());
    wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);

    /* HOST WIFI进入低功耗,通知业务侧关闭定时器 */
    if (pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_pm_state_notify != OAL_PTR_NULL) {
        pst_wlan_pm->st_wifi_srv_handler.p_wifi_srv_pm_state_notify(OAL_FALSE);
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_SLEEP_EVENT, (oal_void *)pst_wlan_pm); /* sleep chain */
#endif

    DECLARE_DFT_TRACE_KEY_INFO("wlan_sleep_ok", OAL_DFT_TRACE_SUCC);
    pst_wlan_pm->ul_sleep_succ++;

    if (pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count != 0) {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_sleep_work release wakelock %lu!\n",
                         pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);
    }

    uc_fail_sleep_count = 0;

    return;

fail_sleep:

    uc_fail_sleep_count++;
    wlan_pm_feed_wdg();
    hcc_tx_transfer_unlock(hcc_get_110x_handler());

    /* 失败超出门限，启动dfr流程 */
    if (uc_fail_sleep_count > WLAN_WAKEUP_FAIL_MAX_TIMES) {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "Now ready to enter DFR process after [%d]times wlan_sleep_fail!",
                       uc_fail_sleep_count);
        uc_fail_sleep_count = 0;
        wlan_pm_stop_wdg(pst_wlan_pm);
        hcc_bus_exception_submit(pst_wlan_pm->pst_bus, WIFI_WAKEUP_FAIL);
    }
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                         CHR_WIFI_DRV_EVENT_PLAT, CHR_PLAT_DRV_ERROR_WAKEUP_DEV);
    return;

sleep_forbid:

    uc_fail_sleep_count = 0;
    wlan_pm_feed_wdg();
    hcc_tx_transfer_unlock(hcc_get_110x_handler());
    return;
}

/*
 * 函 数 名  : wlan_pm_state_get
 * 功能描述  : 获取pm的sleep状态
 * 返 回 值  : 1:allow to sleep; 0:disallow to sleep
 */

oal_uint wlan_pm_state_get(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    return pst_wlan_pm->ul_wlan_dev_state;
}

/*
 * 函 数 名  : wlan_pm_state_set
 * 功能描述  : 获取pm的sleep状态
 * 输入参数  :
 * 输出参数  :
 * 返 回 值  : 1:allow to sleep; 0:disallow to sleep
 */

oal_void wlan_pm_state_set(struct wlan_pm_s *pst_wlan_pm, oal_uint ul_state)
{
    pst_wlan_pm->ul_wlan_dev_state = ul_state;
}
/*
 * 函 数 名  : wlan_pm_set_timeout
 * 功能描述  : 睡眠定时器超时时间设置
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_void wlan_pm_set_timeout(oal_uint32 ul_timeout)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return;
    }

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_set_timeout[%d]", ul_timeout);

    pst_wlan_pm->ul_wdg_timeout_cnt = ul_timeout;

    pst_wlan_pm->ul_wdg_timeout_curr_cnt = 0;

    pst_wlan_pm->ul_packet_cnt = 0;

    wlan_pm_feed_wdg();
}
EXPORT_SYMBOL_GPL(wlan_pm_set_timeout);
/*
 * 函 数 名  : wlan_pm_feed_wdg
 * 功能描述  : 启动50ms睡眠定时器
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_void wlan_pm_feed_wdg(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    pst_wlan_pm->ul_sleep_feed_wdg_cnt++;

#ifdef _PRE_WLAN_DOWNLOAD_PM
    if (download_rate_limit_pps != 0) {
        mod_timer(&pst_wlan_pm->st_watchdog_timer, jiffies + msecs_to_jiffies(10));
    } else {
        mod_timer(&pst_wlan_pm->st_watchdog_timer, jiffies + msecs_to_jiffies(WLAN_SLEEP_TIMER_PERIOD));
    }
#else
    mod_timer(&pst_wlan_pm->st_watchdog_timer, jiffies + msecs_to_jiffies(WLAN_SLEEP_TIMER_PERIOD));
#endif
}

/*
 * 函 数 名  : wlan_pm_stop_wdg
 * 功能描述  : 停止50ms睡眠定时器
 * 返 回 值  : 初始化返回值，成功或失败原因
 */

int32 wlan_pm_stop_wdg(struct wlan_pm_s *pst_wlan_pm_info)
{
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_stop_wdg");

    pst_wlan_pm_info->ul_wdg_timeout_curr_cnt = 0;
    pst_wlan_pm_info->ul_packet_cnt = 0;

    if (in_interrupt()) {
        return del_timer(&pst_wlan_pm_info->st_watchdog_timer);
    } else {
        return del_timer_sync(&pst_wlan_pm_info->st_watchdog_timer);
    }
}

static int wlan_pm_submit_sleep_work(struct wlan_pm_s *pm_data)
{
    if (pm_data->ul_packet_cnt == 0) {
        pm_data->ul_wdg_timeout_curr_cnt++;
        if ((pm_data->ul_wdg_timeout_curr_cnt >= pm_data->ul_wdg_timeout_cnt)) {
            if (wlan_pm_work_submit(pm_data, &pm_data->st_sleep_work) == 0) {
                /* 提交了sleep work后，定时器不重启，避免重复提交sleep work */
                pm_data->ul_sleep_work_submit++;
                pm_data->ul_wdg_timeout_curr_cnt = 0;
                return OAL_SUCC;
            }
            OAM_WARNING_LOG0(0, OAM_SF_PWR, "wlan_pm_sleep_work_etc submit fail,work is running !\n");
        }
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "plat:wlan_pm_wdg_timeout_etc %d have packet %d....",
                             pm_data->ul_wdg_timeout_curr_cnt, pm_data->ul_packet_cnt);
        pm_data->ul_wdg_timeout_curr_cnt = 0;
        pm_data->ul_packet_cnt = 0;

        /* 有报文收发,连续forbid sleep次数清零 */
        pm_data->ul_sleep_fail_forbid = 0;
    }
    return -OAL_EFAIL;
}

/*
 * 函 数 名  : wlan_pm_wdg_timeout
 * 功能描述  : 50ms睡眠定时器超时处理，提交一个sleep work
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
void wlan_pm_wdg_timeout(unsigned long data)
{
    struct wlan_pm_s *pm_data = (struct wlan_pm_s *)(uintptr_t)data;
    if (pm_data == NULL) {
        return;
    }
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_wdg_timeout_etc....%d", pm_data->ul_wdg_timeout_curr_cnt);

    /* hcc bus switch process */
    hcc_bus_performance_core_schedule(HCC_CHIP_110X_DEV);

    pm_data->ul_packet_cnt += pm_wifi_rxtx_count;  // 和hmac中统计收发包分离

    pm_data->ul_packet_total_cnt += pm_wifi_rxtx_count;
    if (time_after(jiffies, pm_data->ul_packet_check_time)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pkt_num:WIFI[%d]", pm_data->ul_packet_total_cnt);
        pm_data->ul_packet_check_time = jiffies + msecs_to_jiffies(WLAN_PACKET_CHECK_TIME);
    }

    pm_wifi_rxtx_count = 0;

    /* 低功耗关闭时timer不会停 */
    if (pm_data->ul_wlan_pm_enable) {
        if (wlan_pm_submit_sleep_work(pm_data) == OAL_SUCC) {
            return;
        }

    } else {
        pm_data->ul_packet_cnt = 0;
    }

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_feed_wdg");
    wlan_pm_feed_wdg();

    return;
}

void wlan_pm_deepsleep_delay_timeout(struct wlan_pm_s *pm_data)
{
    oal_wake_unlock(&pm_data->st_deepsleep_wakelock);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_deepsleep_delay_timeout release wakelock....%lu",
                         pm_data->st_deepsleep_wakelock.lock_count);

    if (pm_data->st_deepsleep_wakelock.lock_count != 0) {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_deepsleep_delay_timeout release wakelock %lu!\n",
                         pm_data->st_deepsleep_wakelock.lock_count);
    }
}

int32 wlan_pm_stop_deepsleep_delay_timer(struct wlan_pm_s *pm_data)
{
    oal_wake_unlock(&pm_data->st_deepsleep_wakelock);

    oal_print_hi11xx_log(HI11XX_LOG_DBG, "wlan_pm_stop_deepsleep_delay_timer release wakelock....%lu",
                         pm_data->st_deepsleep_wakelock.lock_count);

    if (pm_data->st_deepsleep_wakelock.lock_count != 0) {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_stop_deepsleep_delay_timer release wakelock %lu!\n",
                         pm_data->st_deepsleep_wakelock.lock_count);
    }
    if (in_interrupt()) {
        return del_timer(&pm_data->st_deepsleep_delay_timer);
    } else {
        return del_timer_sync(&pm_data->st_deepsleep_delay_timer);
    }
}

/*
 * 函 数 名  : wlan_pm_poweroff_cmd
 * 功能描述  : 发消息到device，wifi device关闭wifi系统资源，等待bcpu给它下电
 * 返 回 值  :
 */
oal_int32 wlan_pm_poweroff_cmd(oal_void)
{
    oal_int32 ret;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "Send H2D_MSG_PM_WLAN_OFF cmd");

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    if (OAL_SUCC != wlan_pm_wakeup_dev()) {
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_FAIL;
    }

    ret = hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_PM_WLAN_OFF);
    if (ret == OAL_SUCC) {
        /* 等待device执行命令 */
        msleep(20);
        if (0 == board_get_wlan_wkup_gpio_val()) {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "wlan_pm_poweroff_cmd  wait device ACK timeout && GPIO_LEVEL[%d] !",
                           board_get_wlan_wkup_gpio_val());

#ifdef PLATFORM_DEBUG_ENABLE
            debug_uart_read_wifi_mem(OAL_FALSE);
#endif
            hcc_tx_transfer_unlock(hcc_get_110x_handler());

            return OAL_FAIL;
        }

    } else {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "fail to send H2D_MSG_PM_WLAN_OFF");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_FAIL;
    }

    hcc_tx_transfer_unlock(hcc_get_110x_handler());

    return OAL_SUCC;
}

void wlan_pm_wkup_src_debug_set(oal_uint32 ul_en)
{
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return;
    }
    pst_wlan_pm->ul_wkup_src_print_en = ul_en;

#endif
}

EXPORT_SYMBOL_GPL(wlan_pm_wkup_src_debug_set);

oal_uint32 wlan_pm_wkup_src_debug_get(void)
{
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FALSE;
    }
    return pst_wlan_pm->ul_wkup_src_print_en;
#else
    return OAL_FALSE;
#endif
}
EXPORT_SYMBOL_GPL(wlan_pm_wkup_src_debug_get);

/*
 * 函 数 名  : wlan_pm_shutdown_bcpu_cmd
 * 功能描述  : 发消息到device，wifi device关闭BCPU
 */
oal_int32 wlan_pm_shutdown_bcpu_cmd(oal_void)
{
#define RETRY_TIMES 3
    oal_uint32 i;
    oal_int32 ret = OAL_FAIL;
    oal_uint32 ul_ret;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    OAM_WARNING_LOG0(0, OAM_SF_PWR, "Send H2D_MSG_PM_BCPU_OFF cmd");

    hcc_tx_transfer_lock(hcc_get_110x_handler());

    for (i = 0; i < RETRY_TIMES; i++) {
        ret = wlan_pm_wakeup_dev();
        if (ret == OAL_SUCC) {
            break;
        }
    }

    if (ret != OAL_SUCC) {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_wakeup_dev fail!");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_FAIL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "wlan_pm_wakeup_dev succ, retry times [%d]", i);

    OAL_INIT_COMPLETION(&pst_wlan_pm->st_close_bcpu_done);

    ret = hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_PM_BCPU_OFF);
    if (ret == OAL_SUCC) {
        /* 等待device执行命令 */
        ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_close_bcpu_done,
                                                 (oal_uint32)OAL_MSECS_TO_JIFFIES(WLAN_POWEROFF_ACK_WAIT_TIMEOUT));
        if (ul_ret == 0) {
            OAM_ERROR_LOG0(0, OAM_SF_PWR, "wlan_pm_shutdown_bcpu_cmd wait device ACK timeout !");
            hcc_tx_transfer_unlock(hcc_get_110x_handler());
            return OAL_FAIL;
        }
    } else {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "fail to send H2D_MSG_PM_BCPU_OFF");
        hcc_tx_transfer_unlock(hcc_get_110x_handler());
        return OAL_FAIL;
    }

    hcc_tx_transfer_unlock(hcc_get_110x_handler());

    return OAL_SUCC;
}

/*
 * 函 数 名  : wlan_pm_dump_info
 * 功能描述  : debug, 发消息到device，串口输出维测信息
 */

void wlan_pm_dump_host_info(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    struct oal_sdio *pst_sdio = oal_get_sdio_default_handler();

    if (pst_wlan_pm == NULL) {
        return;
    }

    OAL_IO_PRINT("----------wlan_pm_dump_host_info begin-----------\n");
    OAL_IO_PRINT("power on:%ld, enable:%ld,wlan_pm_switch:%d\n",
                 pst_wlan_pm->ul_wlan_power_state, pst_wlan_pm->ul_wlan_pm_enable, wlan_pm_switch);
    OAL_IO_PRINT("dev state:%ld, sleep stage:%ld\n", pst_wlan_pm->ul_wlan_dev_state, pst_wlan_pm->ul_sleep_stage);
    OAL_IO_PRINT("open:%d,close:%d\n", pst_wlan_pm->ul_open_cnt, pst_wlan_pm->ul_close_cnt);
    if (pst_sdio != NULL) {
        OAL_IO_PRINT("sdio suspend:%d,sdio resume:%d\n", pst_sdio->ul_sdio_suspend, pst_sdio->ul_sdio_resume);
    }
    OAL_IO_PRINT("gpio_intr[no.%d]:%llu\n",
                 pst_wlan_pm->pst_bus->bus_dev->ul_wlan_irq, pst_wlan_pm->pst_bus->gpio_int_count);
    OAL_IO_PRINT("data_intr:%llu\n", pst_wlan_pm->pst_bus->data_int_count);
    OAL_IO_PRINT("wakeup_intr:%llu\n", pst_wlan_pm->pst_bus->wakeup_int_count);
    OAL_IO_PRINT("D2H_MSG_WAKEUP_SUCC:%d\n", pst_wlan_pm->pst_bus->msg[D2H_MSG_WAKEUP_SUCC].count);
    OAL_IO_PRINT("D2H_MSG_ALLOW_SLEEP:%d\n", pst_wlan_pm->pst_bus->msg[D2H_MSG_ALLOW_SLEEP].count);
    OAL_IO_PRINT("D2H_MSG_DISALLOW_SLEEP:%d\n", pst_wlan_pm->pst_bus->msg[D2H_MSG_DISALLOW_SLEEP].count);

    OAL_IO_PRINT("wakeup_dev_wait_ack:%d\n", oal_atomic_read(&wakeup_dev_wait_ack));
    OAL_IO_PRINT("wakeup_succ:%d\n", pst_wlan_pm->ul_wakeup_succ);
    OAL_IO_PRINT("wakeup_dev_ack:%d\n", pst_wlan_pm->ul_wakeup_dev_ack);
    OAL_IO_PRINT("wakeup_done_callback:%d\n", pst_wlan_pm->ul_wakeup_done_callback);
    OAL_IO_PRINT("wakeup_succ_work_submit:%d\n", pst_wlan_pm->ul_wakeup_succ_work_submit);
    OAL_IO_PRINT("wakeup_fail_wait_sdio:%d\n", pst_wlan_pm->ul_wakeup_fail_wait_sdio);
    OAL_IO_PRINT("wakeup_fail_timeout:%d\n", pst_wlan_pm->ul_wakeup_fail_timeout);
    OAL_IO_PRINT("wakeup_fail_set_reg:%d\n", pst_wlan_pm->ul_wakeup_fail_set_reg);
    OAL_IO_PRINT("wakeup_fail_submit_work:%d\n", pst_wlan_pm->ul_wakeup_fail_submit_work);
    OAL_IO_PRINT("sleep_succ:%d\n", pst_wlan_pm->ul_sleep_succ);
    OAL_IO_PRINT("sleep feed wdg:%d\n", pst_wlan_pm->ul_sleep_feed_wdg_cnt);
    OAL_IO_PRINT("sleep_fail_request:%d\n", pst_wlan_pm->ul_sleep_fail_request);
    OAL_IO_PRINT("sleep_fail_set_reg:%d\n", pst_wlan_pm->ul_sleep_fail_set_reg);
    OAL_IO_PRINT("sleep_fail_wait_timeout:%d\n", pst_wlan_pm->ul_sleep_fail_wait_timeout);
    OAL_IO_PRINT("sleep_fail_forbid:%d\n", pst_wlan_pm->ul_sleep_fail_forbid);
    OAL_IO_PRINT("sleep_work_submit:%d\n", pst_wlan_pm->ul_sleep_work_submit);
    OAL_IO_PRINT("wklock_cnt:%lu\n \n", pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);
    OAL_IO_PRINT("----------wlan_pm_dump_host_info end-----------\n");
}

static oal_int32 wlan_pm_host_info_print_part_one_etc(struct wlan_pm_s *pst_wlan_pm, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 count = 0;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_dev_wait_ack:%d\n",
                     oal_atomic_read(&wakeup_dev_wait_ack));
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_succ:%d\n",
                     pst_wlan_pm->ul_wakeup_succ);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_dev_ack:%d\n",
                     pst_wlan_pm->ul_wakeup_dev_ack);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_done_callback:%d\n",
                     pst_wlan_pm->ul_wakeup_done_callback);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_succ_work_submit:%d\n",
                     pst_wlan_pm->ul_wakeup_succ_work_submit);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_fail_wait_sdio:%d\n",
                     pst_wlan_pm->ul_wakeup_fail_wait_sdio);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_fail_timeout:%d\n",
                     pst_wlan_pm->ul_wakeup_fail_timeout);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_fail_set_reg:%d\n",
                     pst_wlan_pm->ul_wakeup_fail_set_reg);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_fail_submit_work:%d\n",
                     pst_wlan_pm->ul_wakeup_fail_submit_work);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    return count;
}

static oal_int32 wlan_pm_host_info_print_part_two_etc(struct wlan_pm_s *pst_wlan_pm, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 count = 0;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep_succ:%d\n", pst_wlan_pm->ul_sleep_succ);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep feed wdg:%d\n",
                     pst_wlan_pm->ul_sleep_feed_wdg_cnt);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep_fail_request:%d\n",
                     pst_wlan_pm->ul_sleep_fail_request);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep_fail_set_reg:%d\n",
                     pst_wlan_pm->ul_sleep_fail_set_reg);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep_fail_wait_timeout:%d\n",
                     pst_wlan_pm->ul_sleep_fail_wait_timeout);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep_fail_forbid:%d\n",
                     pst_wlan_pm->ul_sleep_fail_forbid);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sleep_work_submit:%d\n",
                     pst_wlan_pm->ul_sleep_work_submit);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wklock_cnt:%lu\n \n",
                     pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "----------wlan_pm_host_info_print_etc end-----------\n");
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    return count;
}

/*
 * 函 数 名  : wlan_pm_host_info_print
 * 功能描述  : cat维测节点输出host低功耗统计
 */

oal_int32 wlan_pm_host_info_print(struct wlan_pm_s *pst_wlan_pm, char *buf, oal_int32 buf_len)
{
    oal_int32 ret = 0;
    oal_int32 count = 0;
    struct oal_sdio *pst_sdio = oal_get_sdio_default_handler();

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "----------wlan_pm_host_info_print_etc begin-----------\n");
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "power on:%ld, enable:%ld,g_wlan_pm_switch_etc:%d\n",
                     pst_wlan_pm->ul_wlan_power_state, pst_wlan_pm->ul_wlan_pm_enable, wlan_pm_switch);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "dev state:%ld, sleep stage:%ld\n",
                     pst_wlan_pm->ul_wlan_dev_state, pst_wlan_pm->ul_sleep_stage);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "open:%d,close:%d\n",
                     pst_wlan_pm->ul_open_cnt, pst_wlan_pm->ul_close_cnt);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    if (pst_sdio != NULL) {
        ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "sdio suspend:%d,sdio resume:%d\n",
                         pst_sdio->ul_sdio_suspend, pst_sdio->ul_sdio_resume);
        if (ret < 0) {
            OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
            return count;
        }
        count += ret;
    }

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "gpio_intr[no.%d]:%llu\n",
                     pst_wlan_pm->pst_bus->bus_dev->ul_wlan_irq, pst_wlan_pm->pst_bus->gpio_int_count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "data_intr:%llu\n",
                     pst_wlan_pm->pst_bus->data_int_count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "wakeup_intr:%llu\n",
                     pst_wlan_pm->pst_bus->wakeup_int_count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "D2H_MSG_WAKEUP_SUCC:%d\n",
                     pst_wlan_pm->pst_bus->msg[D2H_MSG_WAKEUP_SUCC].count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "D2H_MSG_ALLOW_SLEEP:%d\n",
                     pst_wlan_pm->pst_bus->msg[D2H_MSG_ALLOW_SLEEP].count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "D2H_MSG_DISALLOW_SLEEP:%d\n",
                     pst_wlan_pm->pst_bus->msg[D2H_MSG_DISALLOW_SLEEP].count);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = wlan_pm_host_info_print_part_one_etc(pst_wlan_pm, buf, buf_len);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    ret = wlan_pm_host_info_print_part_two_etc(pst_wlan_pm, buf, buf_len);
    if (ret < 0) {
        OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;

    return count;
}

void wlan_pm_dump_device_info(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    hcc_bus_send_message(pst_wlan_pm->pst_bus, H2D_MSG_PM_DEBUG);
}

void wlan_pm_info_clean(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    struct oal_sdio *pst_sdio = oal_get_sdio_default_handler();

    pst_wlan_pm->pst_bus->data_int_count = 0;
    pst_wlan_pm->pst_bus->wakeup_int_count = 0;

    pst_wlan_pm->pst_bus->msg[D2H_MSG_WAKEUP_SUCC].count = 0;
    pst_wlan_pm->pst_bus->msg[D2H_MSG_ALLOW_SLEEP].count = 0;
    pst_wlan_pm->pst_bus->msg[D2H_MSG_DISALLOW_SLEEP].count = 0;

    if (pst_sdio != NULL) {
        pst_sdio->ul_sdio_suspend = 0;
        pst_sdio->ul_sdio_resume = 0;
    }

    pst_wlan_pm->ul_wakeup_succ = 0;
    pst_wlan_pm->ul_wakeup_dev_ack = 0;
    pst_wlan_pm->ul_wakeup_done_callback = 0;
    pst_wlan_pm->ul_wakeup_succ_work_submit = 0;
    pst_wlan_pm->ul_wakeup_fail_wait_sdio = 0;
    pst_wlan_pm->ul_wakeup_fail_timeout = 0;
    pst_wlan_pm->ul_wakeup_fail_set_reg = 0;
    pst_wlan_pm->ul_wakeup_fail_submit_work = 0;

    pst_wlan_pm->ul_sleep_succ = 0;
    pst_wlan_pm->ul_sleep_feed_wdg_cnt = 0;
    pst_wlan_pm->ul_wakeup_done_callback = 0;
    pst_wlan_pm->ul_sleep_fail_set_reg = 0;
    pst_wlan_pm->ul_sleep_fail_wait_timeout = 0;
    pst_wlan_pm->ul_sleep_fail_forbid = 0;
    pst_wlan_pm->ul_sleep_work_submit = 0;

    return;
}

oal_void wlan_pm_debug_sleep(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm && pst_wlan_pm->pst_bus) {
        hcc_bus_sleep_request(pst_wlan_pm->pst_bus);

        pst_wlan_pm->ul_wlan_dev_state = HOST_ALLOW_TO_SLEEP;
    }

    return;
}

oal_void wlan_pm_debug_wakeup(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    if (pst_wlan_pm && pst_wlan_pm->pst_bus) {
        hcc_bus_wakeup_request(pst_wlan_pm->pst_bus);

        pst_wlan_pm->ul_wlan_dev_state = HOST_DISALLOW_TO_SLEEP;
    }

    return;
}

oal_void wlan_pm_debug_wake_lock(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    hcc_bus_wake_lock(pst_wlan_pm->pst_bus);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_debug_wake_lock:wklock_cnt = %lu",
                         pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);

    return;
}

oal_void wlan_pm_debug_wake_unlock(void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    hcc_bus_wake_unlock(pst_wlan_pm->pst_bus);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan_pm_debug_wake_unlock:wklock_cnt = %lu",
                         pst_wlan_pm->pst_bus->st_bus_wakelock.lock_count);

    return;
}



