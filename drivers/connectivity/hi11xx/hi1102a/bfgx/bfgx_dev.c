

/* Include Head file */
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/version.h>
#if ((defined CONFIG_LOG_EXCEPTION) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) && \
    (!defined PLATFORM_DEBUG_ENABLE))
#include <log/log_usertype.h>
#endif


#include "board.h"
#include "hw_bfg_ps.h"
#include "bfgx_ioctrl.h"
#include "plat_type.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "plat_pm.h"
#include "bfgx_user_ctrl.h"
#include "bfgx_exception_rst.h"
#include "plat_firmware.h"
#include "plat_cali.h"
#include "platform_common_clk.h"
#include "securec.h"
#include "oal_ext_if.h"
#include "gps_refclk_src_3.h"

/* Define global variable */
/*
 * This references is the per-PS platform device in the arch/arm/
 * board-xx.c file.
 */
struct platform_device *g_hw_ps_device = NULL;
OAL_STATIC atomic_t g_debug_cnt = ATOMIC_INIT(0);
dump_cmd_queue g_dump_cmd_queue;

OAL_STATIC uint32 g_bfgx_open_cmd[BFGX_BUTT] = {
    SYS_CFG_OPEN_BT,
    SYS_CFG_OPEN_FM,
    SYS_CFG_OPEN_GNSS,
    SYS_CFG_OPEN_IR,
    SYS_CFG_OPEN_NFC,
};

OAL_STATIC uint32 g_bfgx_close_cmd[BFGX_BUTT] = {
    SYS_CFG_CLOSE_BT,
    SYS_CFG_CLOSE_FM,
    SYS_CFG_CLOSE_GNSS,
    SYS_CFG_CLOSE_IR,
    SYS_CFG_CLOSE_NFC,
};

OAL_STATIC uint32 g_bfgx_open_cmd_timeout[BFGX_BUTT] = {
    WAIT_BT_OPEN_TIME,
    WAIT_FM_OPEN_TIME,
    WAIT_GNSS_OPEN_TIME,
    WAIT_IR_OPEN_TIME,
    WAIT_NFC_OPEN_TIME,
};

OAL_STATIC uint32 g_bfgx_close_cmd_timeout[BFGX_BUTT] = {
    WAIT_BT_CLOSE_TIME,
    WAIT_FM_CLOSE_TIME,
    WAIT_GNSS_CLOSE_TIME,
    WAIT_IR_CLOSE_TIME,
    WAIT_NFC_CLOSE_TIME,
};

const uint8 *g_bfgx_subsys_name[BFGX_BUTT] = {
    "BT",
    "FM",
    "GNSS",
    "IR",
    "NFC",
};

OAL_STATIC struct bt_data_combination g_bt_data_combination = {0};

OAL_STATIC oal_int32 g_bfgx_open_ssi_dump = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_bfgx_open_ssi_dump, int, S_IRUGO | S_IWUSR);
#endif

uint32 g_gnss_me_thread_status = DEV_THREAD_EXIT;
uint32 g_gnss_lppe_thread_status = DEV_THREAD_EXIT;
atomic_t g_ir_only_mode = ATOMIC_INIT(0);

OAL_STATIC bool g_device_log_status = false;
void ps_set_device_log_status(bool status)
{
    g_device_log_status = status;
}
bool ps_is_device_log_enable(void)
{
#if ((defined CONFIG_LOG_EXCEPTION) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) && \
    (!defined PLATFORM_DEBUG_ENABLE))
    unsigned int log_usertype = get_log_usertype();
    bool status = ((log_usertype > COMMERCIAL_USER) && (log_usertype < OVERSEA_COMMERCIAL_USER));
    status = (status || g_device_log_status);
    return status;
#else
    return true;
#endif
}

/*
 * Prototype    : ps_get_plat_reference
 * Description  : reference the plat's dat,This references the per-PS
 *                  platform device in the arch/arm/board-xx.c file.
 * input        : use *g_hw_ps_device
 * output       : the have registered platform device struct
 */
int32 ps_get_plat_reference(struct ps_plat_s **plat_data)
{
    struct platform_device *pdev = NULL;
    struct ps_plat_s *ps_plat_d = NULL;

    pdev = g_hw_ps_device;
    if (pdev == NULL) {
        *plat_data = NULL;
        ps_print_err("%s pdev is NULL\n", __func__);
        return FAILURE;
    }

    ps_plat_d = dev_get_drvdata(&pdev->dev);
    *plat_data = ps_plat_d;

    return SUCCESS;
}

/*
 * Prototype    : ps_get_core_reference
 * Description  : reference the core's data,This references the per-PS
 *                  platform device in the arch/xx/board-xx.c file..
 */
int32 ps_get_core_reference(struct ps_core_s **core_data)
{
    struct platform_device *pdev = NULL;
    struct ps_plat_s *ps_plat_d = NULL;

    pdev = g_hw_ps_device;
    if (pdev == NULL) {
        *core_data = NULL;
        ps_print_err("%s pdev is NULL\n", __func__);
        return FAILURE;
    }

    ps_plat_d = dev_get_drvdata(&pdev->dev);
    if (ps_plat_d == NULL) {
        ps_print_err("ps_plat_d is NULL\n");
        return FAILURE;
    }

    *core_data = ps_plat_d->core_data;

    return SUCCESS;
}

/*
 * Prototype    : ps_chk_bfg_active
 * Description  : to chk wether or not bfg active
 * input        : ps_plat_d
 */
bool ps_chk_bfg_active(struct ps_core_s *ps_core_d)
{
    int32 i = 0;
    for (i = 0; i < BFGX_BUTT; i++) {
        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) != POWER_STATE_SHUTDOWN) {
            return true;
        }
    }

    return false;
}

/*
 * Prototype    : ps_bfg_subsys_active
 * Description  : check bfgx subsys active or no
 */
bool ps_bfg_subsys_active(uint32 subsys)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return false;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys id[%d] exceed limit\n", subsys);
        return false;
    }

    if (atomic_read(&ps_core_d->bfgx_info[subsys].subsys_state) == POWER_STATE_SHUTDOWN) {
        ps_print_info("subsys %s is shutdown\n", g_bfgx_subsys_name[subsys]);
        return  false;
    }

    return true;
}

/* only gnss is open and it agree to sleep */
bool ps_chk_only_gnss_and_cldslp(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if ((atomic_read(&ps_core_d->bfgx_info[BFGX_BT].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_FM].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_IR].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_NFC].subsys_state) == POWER_STATE_SHUTDOWN) &&
        (atomic_read(&ps_core_d->bfgx_info[BFGX_GNSS].subsys_state) == POWER_STATE_OPEN) &&
        (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP) &&
        (pm_data->bfgx_dev_state == BFGX_ACTIVE)) {
        return true;
    }
    return false;
}

bool ps_chk_tx_queue_empty(struct ps_core_s *ps_core_d)
{
    PS_PRINT_FUNCTION_NAME;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL");
        return true;
    }

    if (skb_queue_empty(&ps_core_d->tx_urgent_seq) &&
        skb_queue_empty(&ps_core_d->tx_high_seq) &&
        skb_queue_empty(&ps_core_d->tx_low_seq)) {
        return true;
    }
    return false;
}

/*
 * Prototype    : ps_alloc_skb
 * Description  : allocate mem for new skb
 */
struct sk_buff *ps_alloc_skb(uint16 len)
{
    struct sk_buff *skb = NULL;

    PS_PRINT_FUNCTION_NAME;

    skb = alloc_skb(len, GFP_KERNEL);
    if (skb == NULL) {
        ps_print_warning("can't allocate mem for new skb, len=%d\n", len);
        return NULL;
    }

    skb_put(skb, len);

    return skb;
}

/*
 * Prototype    : ps_kfree_skb
 * Description  : when close a function, kfree skb
 */
void ps_kfree_skb(struct ps_core_s *ps_core_d, uint8 type)
{
    struct sk_buff *skb = NULL;

    PS_PRINT_FUNCTION_NAME;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL");
        return;
    }

    while ((skb = ps_skb_dequeue(ps_core_d, type))) {
        kfree_skb(skb);
    }

    switch (type) {
        case TX_URGENT_QUEUE:
            skb_queue_purge(&ps_core_d->tx_urgent_seq);
            break;
        case TX_HIGH_QUEUE:
            skb_queue_purge(&ps_core_d->tx_high_seq);
            break;
        case TX_LOW_QUEUE:
            skb_queue_purge(&ps_core_d->tx_low_seq);
            break;
        case RX_GNSS_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue);
            break;
        case RX_FM_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_FM].rx_queue);
            break;
        case RX_BT_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_BT].rx_queue);
            break;
        case RX_DBG_QUEUE:
            skb_queue_purge(&ps_core_d->rx_dbg_seq);
            break;
        case RX_NFC_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue);
            break;
        case RX_IR_QUEUE:
            skb_queue_purge(&ps_core_d->bfgx_info[BFGX_IR].rx_queue);
            break;
        default:
            ps_print_err("queue type is error, type=%d\n", type);
            break;
    }
    return;
}

/*
 * Prototype    : ps_restore_skbqueue
 * Description  : when err and restore skb to seq function.
 */
int32 ps_restore_skbqueue(struct ps_core_s *ps_core_d, struct sk_buff *skb, uint8 type)
{
    PS_PRINT_FUNCTION_NAME;

    if (unlikely((skb == NULL) || (ps_core_d == NULL))) {
        ps_print_err(" skb or ps_core_d is NULL\n");
        return -EINVAL;
    }

    switch (type) {
        case RX_GNSS_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue, skb);
            break;
        case RX_FM_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_FM].rx_queue, skb);
            break;
        case RX_BT_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_BT].rx_queue, skb);
            break;
        case RX_IR_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_IR].rx_queue, skb);
            break;
        case RX_NFC_QUEUE:
            skb_queue_head(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue, skb);
            break;
        case RX_DBG_QUEUE:
            skb_queue_head(&ps_core_d->rx_dbg_seq, skb);
            break;

        default:
            ps_print_err("queue type is error, type=%d\n", type);
            break;
    }

    return 0;
}

/* prepare to visit dev_node */
int32 prepare_to_visit_node(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = NULL;
    uint8 uart_ready;
    uint64 flags;

    pm_data = pm_get_drvdata();
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -EFAULT;
    }

    /* lock wake_lock */
    bfg_wake_lock();

    /* try to wake up device */
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    atomic_inc(&ps_core_d->node_visit_flag); /* mark someone is visiting dev node */
    uart_ready = ps_core_d->ps_pm->bfgx_uart_state_get();
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    if (uart_ready == UART_NOT_READY) {
        if (host_wkup_dev() != 0) {
            ps_print_err("wkup device FAILED!\n");
            atomic_dec(&ps_core_d->node_visit_flag);
            return -EIO;
        }
    }
    return 0;
}

/* we should do something before exit from visiting dev_node */
int32 post_to_visit_node(struct ps_core_s *ps_core_d)
{
    atomic_dec(&ps_core_d->node_visit_flag);

    return 0;
}

int32 alloc_seperted_rx_buf(uint8 subsys, uint32 len, uint8 alloctype)
{
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint8 *p_rx_buf = NULL;

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys out of range! subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_BT) {
        ps_print_dbg("%s no sepreted buf\n", g_bfgx_subsys_name[subsys]);
        return 0;
    }

    if (alloctype >= ALLOC_BUFF) {
        ps_print_err("alloc type out of range! subsys=%d,alloctype=%d\n", subsys, alloctype);
        return -EINVAL;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }
    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    if (alloctype == KZALLOC) {
        p_rx_buf = kzalloc(len, GFP_KERNEL);
    } else if (alloctype == VMALLOC) {
        p_rx_buf = vmalloc(len);
    }

    if (p_rx_buf == NULL) {
        ps_print_err("alloc failed! subsys=%d, len=%d\n", subsys, len);
        return -ENOMEM;
    }

    spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    pst_sepreted_data->rx_prev_seq = RX_SEQ_NULL;
    pst_sepreted_data->rx_buf_all_len = 0;
    pst_sepreted_data->rx_buf_ptr = p_rx_buf;
    pst_sepreted_data->rx_buf_org_ptr = p_rx_buf;
    spin_unlock(&pst_sepreted_data->sepreted_rx_lock);

    return 0;
}

int32 free_seperted_rx_buf(uint8 subsys, uint8 alloctype)
{
    struct ps_core_s *ps_core_d = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint8 *buf_ptr = NULL;

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys out of range! subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_BT) {
        ps_print_dbg("%s no sepreted buf\n", g_bfgx_subsys_name[subsys]);
        return 0;
    }

    if (alloctype >= ALLOC_BUFF) {
        ps_print_err("alloc type out of range! subsys=%d,alloctype=%d\n", subsys, alloctype);
        return -EINVAL;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }
    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
    spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    pst_sepreted_data->rx_prev_seq = RX_SEQ_NULL;
    pst_sepreted_data->rx_buf_all_len = 0;
    pst_sepreted_data->rx_buf_ptr = NULL;
    pst_sepreted_data->rx_buf_org_ptr = NULL;
    spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
    if (buf_ptr != NULL) {
        if (alloctype == KZALLOC) {
            kfree(buf_ptr);
        } else if (alloctype == VMALLOC) {
            vfree(buf_ptr);
        }
    }

    return 0;
}

int32 bfgx_open_fail_process(uint8 subsys, int32 error)
{
    struct ps_core_s *ps_core_d = NULL;

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is error, subsys=[%d]\n", subsys);
        return BFGX_POWER_FAILED;
    }

    if (error >= BFGX_POWER_ENUM_BUTT) {
        ps_print_err("error is error, error=[%d]\n", error);
        return BFGX_POWER_FAILED;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return BFGX_POWER_FAILED;
    }

    ps_print_info("bfgx open fail, type=[%d]\n", error);

    if (g_bfgx_open_ssi_dump) {
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL);
    }

    switch (error) {
        case BFGX_POWER_PULL_POWER_GPIO_FAIL:
        case BFGX_POWER_TTY_OPEN_FAIL:
        case BFGX_POWER_TTY_FLOW_ENABLE_FAIL:
            break;

        case BFGX_POWER_WIFI_DERESET_BCPU_FAIL:
        case BFGX_POWER_WIFI_ON_BOOT_UP_FAIL:
            if (plat_power_fail_exception_info_set(SUBSYS_BFGX, subsys, BFGX_POWERON_FAIL) == BFGX_POWER_SUCCESS) {
                bfgx_system_reset();
                plat_power_fail_process_done();
            } else {
                ps_print_err("bfgx power fail, set exception info fail\n");
            }

            ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
            break;

        case BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL:
        case BFGX_POWER_DOWNLOAD_FIRMWARE_FAIL:
            ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
            break;

        case BFGX_POWER_WAKEUP_FAIL:
        case BFGX_POWER_OPEN_CMD_FAIL:
            if (plat_power_fail_exception_info_set(SUBSYS_BFGX, subsys, BFGX_POWERON_FAIL) == BFGX_POWER_SUCCESS) {
                if (bfgx_subsystem_reset() != EXCEPTION_SUCCESS) {
                    ps_print_err("bfgx_subsystem_reset failed \n");
                }
                plat_power_fail_process_done();
            } else {
                ps_print_err("bfgx power fail, set exception info fail\n");
            }

            ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
            break;

        default:
            ps_print_err("error is undefined, error=[%d]\n", error);
            break;
    }

    return BFGX_POWER_SUCCESS;
}

/*
 * Prototype    : uart_wifi_open
 * Description  : functions called by wifi pm to open wifi throuhg bfgx system
 * output       : return !0--> open is false
 *                return 0 --> open is ok
 */
int32 uart_wifi_open(void)
{
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft;
    int32 ret;

    ps_print_info("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }

    /* 如果BFGIN睡眠，则唤醒之 */
    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return ret;
    }

    ps_print_info("uart open WCPU\n");
    oal_reinit_completion(ps_core_d->wait_wifi_opened);
    /* tx sys bt open info */
    ps_uart_state_pre(ps_core_d->tty);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_OPEN_WIFI);

    timeleft = wait_for_completion_timeout(&ps_core_d->wait_wifi_opened, msecs_to_jiffies(WAIT_WIFI_OPEN_TIME));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait wifi open ack timeout\n");
        post_to_visit_node(ps_core_d);
        return -ETIMEDOUT;
    }

    post_to_visit_node(ps_core_d);

    msleep(20);

    return SUCCESS;
}

/*
 * Prototype    : uart_wifi_close
 * Description  : functions called by wifi pm to close wifi throuhg bfgx system
 * output       : return !0--> open is false
 *                return 0 --> open is ok
 */
int32 uart_wifi_close(void)
{
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft;
    int32 ret;

    ps_print_info("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }

    /* 如果BFGIN睡眠，则唤醒之 */
    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return ret;
    }

    ps_print_info("uart close WCPU\n");

    oal_reinit_completion(ps_core_d->wait_wifi_closed);
    ps_uart_state_pre(ps_core_d->tty);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_CLOSE_WIFI);

    timeleft = wait_for_completion_timeout(&ps_core_d->wait_wifi_closed, msecs_to_jiffies(WAIT_WIFI_CLOSE_TIME));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait wifi close ack timeout\n");
        post_to_visit_node(ps_core_d);
        return -ETIMEDOUT;
    }

    ps_print_warning("uart close WCPU done,gpio level[%d]\n", board_get_wlan_wkup_gpio_val());

    post_to_visit_node(ps_core_d);

    return SUCCESS;
}

/*
 * Prototype    : uart_bfgx_close_cmd
 * Description  : functions called by bfgn pm to close bcpu throuhg bfgx system
 * output       : return !0--> open is false
 *                return 0 --> open is ok
 */
int32 uart_bfgx_close_cmd(void)
{
#define WAIT_CLOSE_TIMES  100
    struct ps_core_s *ps_core_d = NULL;
    int bwkup_gpio_val = 1;
    int32 ret;
    int i;

    ps_print_info("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }

    /* 单红外dev不处理系统消息 */
    if (oal_atomic_read(&g_ir_only_mode) != 0) {
        return SUCCESS;
    }

    /* 如果BFGIN睡眠，则唤醒之 */
    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return ret;
    }

    /* 下发BFGIN shutdown命令 */
    ps_print_info("uart shutdown BCPU\n");

    ps_uart_state_pre(ps_core_d->tty);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_SHUTDOWN_SLP);

    ret = FAILURE;
    for (i = 0; i < WAIT_CLOSE_TIMES; i++) {
        bwkup_gpio_val = board_get_bwkup_gpio_val();
        if (bwkup_gpio_val == 0) {
            ret = SUCCESS;
            break;
        }
        msleep(10);
    }
    ps_print_info("bfg gpio level:%d, i=%d\n", bwkup_gpio_val, i);

    if (ret == FAILURE) {
        ps_uart_state_dump(ps_core_d->tty);
    }

    post_to_visit_node(ps_core_d);

    return ret;
}

int32 bfgx_open_cmd_send(uint32 subsys)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EINVAL;
    }

    /* 单红外dev不处理系统消息 */
    if (oal_atomic_read(&g_ir_only_mode) != 0) {
        return SUCCESS;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_IR) {
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, g_bfgx_open_cmd[subsys]);
        msleep(20);
        return 0;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (!is_dfr_test_en(BFGX_POWEON_FAULT)) {
        ps_print_warning("[dfr test]:trigger powon fail\n");
        return -EINVAL;
    }
#endif

    if (subsys == BFGX_GNSS) {
        g_gnss_me_thread_status = DEV_THREAD_EXIT;
        g_gnss_lppe_thread_status = DEV_THREAD_EXIT;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    oal_reinit_completion(pst_bfgx_data->wait_opened);
    ps_uart_state_pre(ps_core_d->tty);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, g_bfgx_open_cmd[subsys]);
    timeleft = wait_for_completion_timeout(&pst_bfgx_data->wait_opened,
                                           msecs_to_jiffies(g_bfgx_open_cmd_timeout[subsys]));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait %s open ack timeout\n", g_bfgx_subsys_name[subsys]);
        if (subsys == BFGX_GNSS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_OPEN_GNSS);
        } else if (subsys == BFGX_BT) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_OPEN_BT);
        }

        return -ETIMEDOUT;
    }

    return 0;
}

int32 bfgx_close_cmd_send(uint32 subsys)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    /* 单红外dev不处理系统消息 */
    if (oal_atomic_read(&g_ir_only_mode) != 0) {
        return SUCCESS;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    if (!is_dfr_test_en(BFGX_POWEOFF_FAULT)) {
        ps_print_warning("[dfr test]:trigger power off fail\n");
        return -EINVAL;
    }
#endif

    if (subsys == BFGX_IR) {
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, g_bfgx_close_cmd[subsys]);
        msleep(20);
        return 0;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    oal_reinit_completion(pst_bfgx_data->wait_closed);
    ps_uart_state_pre(ps_core_d->tty);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, g_bfgx_close_cmd[subsys]);
    timeleft = wait_for_completion_timeout(&pst_bfgx_data->wait_closed,
                                           msecs_to_jiffies(g_bfgx_close_cmd_timeout[subsys]));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait %s close ack timeout\n", g_bfgx_subsys_name[subsys]);
        if (g_ssi_dump_en) {
            ssi_dump_device_regs(SSI_MODULE_MASK_COMM);
        }
        if (subsys == BFGX_GNSS) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_GNSS);
        } else if (subsys == BFGX_BT) {
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_CLOSE, CHR_PLAT_DRV_ERROR_CLOSE_BT);
        }

        return -ETIMEDOUT;
    }

    return 0;
}

/* 单红外模式打开其他子系统时调用,关闭红外 */
int32 hw_ir_only_open_other_subsys(void)
{
    int32 ret;

    ret = hw_bfgx_close(BFGX_IR);
    oal_atomic_set(&g_ir_only_mode, 0);
    return ret;
}

int32 hw_bfgx_open(uint32 subsys)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    int32 error;

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    ps_print_info("open %s\n", g_bfgx_subsys_name[subsys]);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL) ||
                 (ps_core_d->ps_pm->bfg_power_set == NULL) ||
                 (ps_core_d->ps_pm->pm_priv_data == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        ps_print_warning("%s has opened! It's Not necessary to send msg to device\n", g_bfgx_subsys_name[subsys]);
        return BFGX_POWER_SUCCESS;
    }

    if (alloc_seperted_rx_buf(subsys, g_bfgx_rx_max_frame[subsys], VMALLOC) != BFGX_POWER_SUCCESS) {
        ps_print_err("no mem allocate to read!\n");
        return -ENOMEM;
    }

    /* 当单红外模式打开其他子系统时，需要关闭单红外才能其他子系统正常上电 */
    if ((oal_atomic_read(&g_ir_only_mode) != 0) && subsys != BFGX_IR) {
        if (hw_ir_only_open_other_subsys() != BFGX_POWER_SUCCESS) {
            ps_print_err("ir only mode,but close ir only mode fail!\n");
            free_seperted_rx_buf(subsys, VMALLOC);
            return -ENOMEM;
        }
    }

    error = ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_UP);
    if (error != BFGX_POWER_SUCCESS) {
        ps_print_err("set %s power on err! error = %d\n", g_bfgx_subsys_name[subsys], error);
        goto bfgx_power_on_fail;
    }

    if (prepare_to_visit_node(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("prepare work FAIL\n");
        error = BFGX_POWER_WAKEUP_FAIL;
        goto bfgx_wakeup_fail;
    }

    if (bfgx_open_cmd_send(subsys) != BFGX_POWER_SUCCESS) {
        ps_print_err("bfgx open cmd fail\n");
        error = BFGX_POWER_OPEN_CMD_FAIL;
        goto bfgx_open_cmd_fail;
    }

    /* 单红外没有低功耗 */
    if (oal_atomic_read(&g_ir_only_mode) == 0) {
        mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + oal_msecs_to_jiffies(BT_SLEEP_TIME));
        ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;
    }

    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_OPEN);
    post_to_visit_node(ps_core_d);

    return BFGX_POWER_SUCCESS;

bfgx_open_cmd_fail:
    post_to_visit_node(ps_core_d);
bfgx_wakeup_fail:
bfgx_power_on_fail:
    free_seperted_rx_buf(subsys, VMALLOC);
    bfgx_open_fail_process(subsys, error);
    return BFGX_POWER_FAILED;
}

int32 hw_bfgx_close(uint32 subsys)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys is err, subsys is [%d]\n", subsys);
        return -EINVAL;
    }

    ps_print_info("close %s\n", g_bfgx_subsys_name[subsys]);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    if (POWER_STATE_SHUTDOWN == atomic_read(&pst_bfgx_data->subsys_state)) {
        ps_print_warning("%s has closed! It's Not necessary to send msg to device\n", g_bfgx_subsys_name[subsys]);
        return BFGX_POWER_SUCCESS;
    }
    wake_up_interruptible(&pst_bfgx_data->rx_wait);

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        /* 唤醒失败，bfgx close时的唤醒失败不进行DFR恢复 */
        ps_print_err("prepare work FAIL\n");
    }

    ret = bfgx_close_cmd_send(subsys);
    if (ret < 0) {
        /* 发送close命令失败，不进行DFR，继续进行下电流程，DFR恢复延迟到下次open时或者其他业务运行时进行 */
        ps_print_err("bfgx close cmd fail\n");
    }

    atomic_set(&pst_bfgx_data->subsys_state, POWER_STATE_SHUTDOWN);
    free_seperted_rx_buf(subsys, VMALLOC);
    ps_kfree_skb(ps_core_d, g_bfgx_rx_queue[subsys]);

    ps_core_d->rx_pkt_num[subsys] = 0;
    ps_core_d->tx_pkt_num[subsys] = 0;
    if (bfgx_other_subsys_all_shutdown(BFGX_GNSS)) {
        del_timer_sync(&pm_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;
    }

    ret = ps_core_d->ps_pm->bfg_power_set(subsys, BFG_POWER_GPIO_DOWN);
    if (ret) {
        /* 下电失败，不进行DFR，DFR恢复延迟到下次open时或者其他业务运行时进行 */
        ps_print_err("set %s power off err!ret = %d", g_bfgx_subsys_name[subsys], ret);
    }

    post_to_visit_node(ps_core_d);

    return 0;
}

// 子系统关闭则不再写数据
STATIC int32 hw_bfgx_write_check(struct file *filp, const int8 __user *buf,
                                 struct ps_core_s *ps_core_d, uint32 subsys)
{
    struct st_bfgx_data *pst_bfgx_data = NULL;
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) || (ps_core_d->ps_pm == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
        ps_print_warning("%s has closed! It's Not necessary to send msg to device\n", g_bfgx_subsys_name[subsys]);
        return -EINVAL;
    }
    return 0;
}

/*
 * Prototype    : hw_bt_open
 * Description  : functions called from above bt hal,when open bt file
 * input        : "/dev/hwbt"
 * output       : return !0--> open is false
 *                return 0 --> open is ok
 */
STATIC int32 hw_bt_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_open(BFGX_BT);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_bt_read
 * Description  : functions called from above bt hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
STATIC ssize_t hw_bt_read(struct file *filp, int8 __user *buf,
                          size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_BT_QUEUE)) == NULL) {
        ps_print_warning("bt read skb queue is null!\n");
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_BT_QUEUE);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_BT_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_bt_write
 * Description  : functions called from above bt hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
STATIC ssize_t hw_bt_write(struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 total_len;
    int32 ret;
    uint8 __user *puser = (uint8 __user *)buf;
    uint8 type = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    ret = hw_bfgx_write_check(filp, buf, ps_core_d, BFGX_BT);
    if (ret < 0) {
        g_bt_data_combination.len = 0;
        return ret;
    }

    if (count > BT_TX_MAX_FRAME) {
        ps_print_err("bt skb len is too large!\n");
        g_bt_data_combination.len = 0;
        return -EINVAL;
    }

    /* 适配O，BT数据分两次下发，先发数据类型，长度固定为1Byte，然后发数据，需要在驱动中组合起来发给device */
    if (count == BT_TYPE_DATA_LEN) {
        get_user(type, puser);
        g_bt_data_combination.type = type;
        g_bt_data_combination.len = count;

        return count;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        ps_print_err("bt tx high seqlen large than MAXNUM\n");
        g_bt_data_combination.len = 0;
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        g_bt_data_combination.len = 0;
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->ps_pm->pm_priv_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    total_len = count + g_bt_data_combination.len + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);

    skb = ps_alloc_skb(total_len);
    if (skb == NULL) {
        ps_print_err("ps alloc skb mem fail\n");
        post_to_visit_node(ps_core_d);
        g_bt_data_combination.len = 0;
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head) + g_bt_data_combination.len], buf, count)) {
        ps_print_err("copy_from_user from bt is err\n");
        kfree_skb(skb);
        post_to_visit_node(ps_core_d);
        g_bt_data_combination.len = 0;
        return -EFAULT;
    }

    if (g_bt_data_combination.len == BT_TYPE_DATA_LEN) {
        skb->data[sizeof(struct ps_packet_head)] = g_bt_data_combination.type;
    }

    g_bt_data_combination.len = 0;

    ps_add_packet_head(skb->data, BT_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    ps_core_d->tx_pkt_num[BFGX_BT]++;
    post_to_visit_node(ps_core_d);

    return count;
}

/*
 * Prototype    : hw_bt_poll
 * Description  : called by bt func from hal;
 *                  check whether or not allow read and write
 * input        : file handle
 */
STATIC uint32 hw_bt_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32 mask = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_BT].rx_wait, wait);

    if (ps_core_d->bfgx_info[BFGX_BT].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_bt_ioctl
 * Description  : called by bt func from hal; default not use
 * input        : file handle, cmd, arg
 */
STATIC int64 hw_bt_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    PS_PRINT_FUNCTION_NAME;

    return 0;
}

/*
 * Prototype    : hw_bt_release
 * Description  : called by bt func from hal when close bt inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_bt_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_BT);

    oal_wake_unlock_force(&pm_data->bt_wake_lock);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

STATIC int32 hw_nfc_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_open(BFGX_NFC);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

STATIC ssize_t hw_nfc_read(struct file *filp, int8 __user *buf,
                           size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_NFC_QUEUE)) == NULL) {
        ps_print_warning("nfc read skb queue is null!");
        return 0;
    }

    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_NFC_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);
    if (skb->len == 0) {
        kfree_skb(skb);
    } else {
        ps_restore_skbqueue(ps_core_d, skb, RX_NFC_QUEUE);
    }

    return count1;
}

STATIC ssize_t hw_nfc_write(struct file *filp, const int8 __user *buf,
                            size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);

    ret = hw_bfgx_write_check(filp, buf, ps_core_d, BFGX_NFC);
    if (ret < 0) {
        return ret;
    }

    if (count > NFC_TX_MAX_FRAME) {
        ps_print_err("bt skb len is too large!\n");
        return -EINVAL;
    }

    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_NFC, BFGX_WAKEUP_FAIL);
        return ret;
    }
    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    /* to divide up packet function and tx to tty work */
    if (ps_tx_nfcbuf(ps_core_d, buf, count) < 0) {
        ps_print_err("hw_nfc_write is err\n");
        post_to_visit_node(ps_core_d);
        return -EFAULT;
    }

    ps_core_d->tx_pkt_num[BFGX_NFC]++;
    post_to_visit_node(ps_core_d);

    ps_print_dbg("NFC data write end\n");

    return count;
}

/*
 * Prototype    : hw_nfc_poll
 * Description  : called by nfc func from hal;
 *                check whether or not allow read and write
 * input        : file handle
 */
STATIC uint32 hw_nfc_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32 mask = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_NFC].rx_wait, wait);

    if (ps_core_d->bfgx_info[BFGX_NFC].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

STATIC int32 hw_nfc_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_NFC);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_ir_open
 * Description  : open ir device
 * input        : struct inode *inode, struct file *filp
 * output       : return 0
 */
STATIC int32 hw_ir_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    /* judge ir only mode */
    if ((wlan_is_shutdown() == true) && (bfgx_is_shutdown() == true)
        && (get_hi1103_asic_type() == HI1103_ASIC_PILOT)) {
        oal_atomic_set(&g_ir_only_mode, 1);
    }

    ret = hw_bfgx_open(BFGX_IR);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_ir_read
 * Description  : read ir node data
 * input        : struct file *filp, int8 __user *buf, size_t count,loff_t *f_pos
 * output       : return read len
 */
STATIC ssize_t hw_ir_read(struct file *filp, int8 __user *buf,
                          size_t count, loff_t *f_pos)
{
    uint16 ret_count;
    struct sk_buff *skb = NULL;
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_IR_QUEUE)) == NULL) {
        ps_print_dbg("ir read skb queue is null!\n");
        return 0;
    }

    ret_count = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, ret_count)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_IR_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, ret_count);

    if (skb->len == 0) {
        kfree_skb(skb);
    } else {
        ps_restore_skbqueue(ps_core_d, skb, RX_IR_QUEUE);
    }

    return ret_count;
}

/*
 * Prototype    : hw_ir_write
 * Description  : write data to ir node
 * input        : struct file *filp, const int8 __user *buf, size_t count, loff_t *f_pos
 * output       : return write len
 */
STATIC ssize_t hw_ir_write(struct file *filp, const int8 __user *buf,
                           size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);

    ret = hw_bfgx_write_check(filp, buf, ps_core_d, BFGX_IR);
    if (ret < 0) {
        return ret;
    }

    if (count > IR_TX_MAX_FRAME) {
        ps_print_err("IR skb len is too large!\n");
        return -EINVAL;
    }

    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_IR, BFGX_WAKEUP_FAIL);
        return ret;
    }

    if (oal_atomic_read(&g_ir_only_mode) == 0) {
        /* modify expire time of uart idle timer */
        mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
        ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;
    }

    /* to divide up packet function and tx to tty work */
    if (ps_tx_irbuf(ps_core_d, buf, count) < 0) {
        ps_print_err("hw_ir_write is err\n");
        post_to_visit_node(ps_core_d);
        return -EFAULT;
    }

    ps_core_d->tx_pkt_num[BFGX_IR]++;
    post_to_visit_node(ps_core_d);

    ps_print_dbg("IR data write end\n");

    return count;
}

/*
 * Prototype    : hw_ir_release
 * Description  : release
 * input        : struct inode *inode, struct file *filp
 * output       : return 0
 */
STATIC int32 hw_ir_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_IR);
    oal_atomic_set(&g_ir_only_mode, 0);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_fm_open
 * Description  : functions called from above fm hal,when open fm file
 * input        : struct inode *inode, struct file *filp
 * output       : return !0--> open is false
                  return 0 --> open is ok
 */
STATIC int32 hw_fm_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_open(BFGX_FM);
    if ((board_fm_lan_support() == OAL_TRUE) && (ret == BFGX_POWER_SUCCESS)) {
        fm_lan_notifier_call_chain(FM_OPEN);
    }

    ps_core_d->fm_read_delay = FM_READ_DEFAULT_TIME;

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_fm_read
 * Description  : functions called from above fm hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
STATIC ssize_t hw_fm_read(struct file *filp, int8 __user *buf,
                          size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1;
    int64 timeout;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (ps_core_d->bfgx_info[BFGX_FM].rx_queue.qlen == 0) { /* if don,t data, and wait timeout function */
        if (filp->f_flags & O_NONBLOCK) {                   /* if O_NONBLOCK read and return */
            return -EAGAIN;
        }
        /* timeout function;when have data,can interrupt */
        timeout = wait_event_interruptible_timeout(ps_core_d->bfgx_info[BFGX_FM].rx_wait,
                                                   (ps_core_d->bfgx_info[BFGX_FM].rx_queue.qlen > 0),
                                                   msecs_to_jiffies(ps_core_d->fm_read_delay));
        if (!timeout) {
            ps_print_dbg("fm read time out!\n");
            return -ETIMEDOUT;
        }
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_FM_QUEUE)) == NULL) {
        ps_print_warning("fm read no data!\n");
        return -ETIMEDOUT;
    }

    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_FM_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);

    /* if skb->len == 0: curr skb read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_FM_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_fm_write
 * Description  : functions called from above fm hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
STATIC ssize_t hw_fm_write(struct file *filp, const int8 __user *buf,
                           size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);

    ret = hw_bfgx_write_check(filp, buf, ps_core_d, BFGX_FM);
    if (ret < 0) {
        return ret;
    }

    /* if count is too large;and don,t tx */
    if (count > (FM_TX_MAX_FRAME - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end))) {
        ps_print_err("err:fm packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_FM, BFGX_WAKEUP_FAIL);
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    /* to divide up packet function and tx to tty work */
    if (ps_tx_fmbuf(ps_core_d, buf, count) < 0) {
        ps_print_err("hw_fm_write is err\n");
        post_to_visit_node(ps_core_d);
        return -EFAULT;
    }
    ps_core_d->tx_pkt_num[BFGX_FM]++;

    post_to_visit_node(ps_core_d);

    ps_print_dbg("FM data write end\n");

    return count;
}

/*
 * Prototype    : hw_fm_ioctl
 * Description  : called by hw func from hal when open power gpio or close power gpio
 * input        : file handle, cmd, arg
 */
STATIC int64 hw_fm_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (file == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (cmd == FM_SET_READ_TIME) {
        if (arg < FM_MAX_READ_TIME) { /* set timeout for fm read function */
            ps_core_d->fm_read_delay = arg;
        } else {
            ps_print_err("arg is too large!\n");
            return -EINVAL;
        }
    }

    return 0;
}

/*
 * Prototype    : hw_fm_release
 * Description  : called by fm func from hal when close fm inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_fm_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_FM);
    if ((board_fm_lan_support() == OAL_TRUE) && (ret == BFGX_POWER_SUCCESS)) {
        fm_lan_notifier_call_chain(FM_CLOSE);
    }

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/* device bfgx pm debug switch on/off */
void plat_pm_debug_switch(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is null\n");
        return;
    }

    ps_print_info("%s", __func__);

    ps_get_core_reference(&ps_core_d);

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return;
    }

    ps_tx_sys_cmd(ps_core_d, SYS_MSG, PL_PM_DEBUG);

    post_to_visit_node(ps_core_d);

    return;
}

/*
 * Prototype    : hw_gnss_open
 * Description  : functions called from above gnss hal,when open gnss file
 * input        : "/dev/hwgnss"
 * output       : return 0 --> open is ok
 *                return !0--> open is false
 */
STATIC int32 hw_gnss_open(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);

    ret = hw_bfgx_open(BFGX_GNSS);

    ps_core_d->gnss_read_delay = GNSS_READ_DEFAULT_TIME;

    if (ret != BFGX_POWER_SUCCESS) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    }

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

/*
 * Prototype    : hw_gnss_poll
 * Description  : called by gnss func from hal;
 *                check whether or not allow read and write
 * input        : file handle
 */
STATIC uint32 hw_gnss_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32 mask = 0;

    ps_print_dbg("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_GNSS].rx_wait, wait);

    ps_print_dbg("%s, recive gnss data\n", __func__);

    if (ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_gnss_read
 * Description  : functions called from above gnss hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
STATIC ssize_t hw_gnss_read(struct file *filp, int8 __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    struct sk_buff_head read_queue;
    int32 count1;
    uint8 seperate_tag = GNSS_SEPER_TAG_INIT;
    int32 copy_cnt = 0;
    uint32 ret;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb_queue_head_init(&read_queue);

    spin_lock(&ps_core_d->gnss_rx_lock);
    do {
        if ((skb = ps_skb_dequeue(ps_core_d, RX_GNSS_QUEUE)) == NULL) {
            spin_unlock(&ps_core_d->gnss_rx_lock);
            if (read_queue.qlen != 0) {
                // 没有找到last包，skb queue就空了
                ps_print_err("skb dequeue error, qlen=%x!\n", read_queue.qlen);
                goto skb_dequeue_error;
            } else {
                ps_print_info("gnss read no data!\n");
                return 0;
            }
        }

        seperate_tag = skb->data[skb->len - 1];
        switch (seperate_tag) {
            case GNSS_SEPER_TAG_INIT:
            case GNSS_SEPER_TAG_LAST:
                break;
            default:
                ps_print_err("seperate_tag=%x not support\n", seperate_tag);
                seperate_tag = GNSS_SEPER_TAG_LAST;
                break;
        }

        skb_queue_tail(&read_queue, skb);
    } while (GNSS_SEPER_TAG_INIT == seperate_tag);
    spin_unlock(&ps_core_d->gnss_rx_lock);

    copy_cnt = 0;
    do {
        skb = skb_dequeue(&read_queue);
        if (skb == NULL) {
            ps_print_err("copy dequeue error, copy_cnt=%x\n", copy_cnt);
            goto skb_dequeue_error;
        }

        if (skb->len <= 1) {
            ps_print_err("skb len error,skb->len=%x,copy_cnt=%x,count=%x\n", skb->len, copy_cnt, (uint32)count);
            goto copy_error;
        }

        count1 = skb->len - 1;
        if (count1 + copy_cnt > count) {
            ps_print_err("copy total len error,skb->len=%x,tag=%x,copy_cnt=%x,read_cnt=%x\n",
                         skb->len, skb->data[skb->len - 1], copy_cnt, (uint32)count);
            goto copy_error;
        }

        ret = copy_to_user(buf + copy_cnt, skb->data, count1);
        if (ret != 0) {
            ps_print_err("copy_to_user err,ret=%x,dest=%p,src=%p,tag:%x,count1=%x,copy_cnt=%x,read_cnt=%x\n",
                         ret, buf + copy_cnt, skb->data, skb->data[skb->len - 1], count1, copy_cnt, (uint32)count);
            goto copy_error;
        }

        copy_cnt += count1;
        kfree_skb(skb);
    } while (read_queue.qlen != 0);

    return copy_cnt;

copy_error:
    kfree_skb(skb);
skb_dequeue_error:
    while ((skb = skb_dequeue(&read_queue)) != NULL) {
        ps_print_err("free skb: len=%x, tag=%x\n", skb->len, skb->data[skb->len - 1]);
        kfree_skb(skb);
    }

    return -EFAULT;
}

/*
 * Prototype    : hw_gnss_write
 * Description  : functions called from above gnss hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
STATIC ssize_t hw_gnss_write(struct file *filp, const int8 __user *buf,
                             size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(pm_data == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    ret = hw_bfgx_write_check(filp, buf, ps_core_d, BFGX_GNSS);
    if (ret < 0) {
        return ret;
    }

    if (count > GNSS_TX_MAX_FRAME) {
        ps_print_err("err:gnss packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        ps_print_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, BFGX_WAKEUP_FAIL);
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->ps_pm->pm_priv_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* to divide up packet function and tx to tty work */
    if (ps_tx_gnssbuf(ps_core_d, buf, count) < 0) {
        ps_print_err("hw_gnss_write is err\n");
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
        count = -EFAULT;
    }

    ps_core_d->tx_pkt_num[BFGX_GNSS]++;
    post_to_visit_node(ps_core_d);

    return count;
}

STATIC int64 gnss_abb_clk_para_cmd(oal_int32 gnss_abb_clk_state)
{
    if (gnss_abb_clk_state == GNSS_ABB_CLK_ENABLE) {
        plat_gnss_clk_enable();
        return 0;
    } else if (gnss_abb_clk_state == GNSS_ABB_CLK_DISABLE) {
        plat_gnss_clk_disable();
        return 0;
    } else {
        ps_print_err("get gnss abb clk para error %d\n", gnss_abb_clk_state);
        return -EINVAL;
    }
}

/*
 * Prototype    : hw_gnss_ioctl
 * Description  : called by gnss func from hal when open power gpio or close power gpio
 * input        : file handle, cmd, arg
 */
STATIC int64 hw_gnss_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 __user *puser;
    oal_int32 coeff_para[COEFF_NUM];
    oal_int32 gnss_abb_clk_state;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (file == NULL))) {
        return -EINVAL;
    }

    switch (cmd) {
        case GNSS_SET_READ_TIME:
            if (arg < GNSS_MAX_READ_TIME) { /* set timeout for gnss read function */
                ps_core_d->gnss_read_delay = arg;
            } else {
                return -EINVAL;
            }
            break;
        case PLAT_GNSS_DCXO_SET_PARA_CMD:
            puser = (int32 __user *)arg;
            if (copy_from_user(coeff_para, puser, COEFF_NUM * sizeof(oal_int32))) {
                ps_print_err("[dcxo] get gnss update para error\n");
                return -EINVAL;
            }
            update_dcxo_coeff(coeff_para, COEFF_NUM * sizeof(oal_int32));
            break;
        case PLAT_GNSS_ABB_CLK_PARA_CMD:
            puser = (int32 __user *)arg;
            if (copy_from_user(&gnss_abb_clk_state, puser, sizeof(oal_int32))) {
                ps_print_err("get gnss abb clk state error\n");
                return -EINVAL;
            }
            return gnss_abb_clk_para_cmd(gnss_abb_clk_state);
        case PLAT_GNSS_SLEEP_VOTE_CMD:
            gnss_lowpower_vote(arg);
            break;
        default:
            ps_print_warning("hw_gnss_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}

/*
 * Prototype    : hw_gnss_release
 * Description  : called by gnss func from hal when close gnss inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_gnss_release(struct inode *inode, struct file *filp)
{
    int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    if (unlikely((inode == NULL) || (filp == NULL))) {
        ps_print_err("%s param is error", __func__);
        return -EINVAL;
    }

    mutex_lock(&pm_data->host_mutex);

    ret = hw_bfgx_close(BFGX_GNSS);

    oal_wake_unlock_force(&pm_data->gnss_wake_lock);

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);

    mutex_unlock(&pm_data->host_mutex);

    return ret;
}

#ifndef HI110X_HAL_MEMDUMP_ENABLE
/*
 * Prototype    : plat_exception_dump_file_rotate_init
 * Description  : driver init rotate resource
 */
void plat_exception_dump_file_rotate_init(void)
{
    init_waitqueue_head(&g_dump_cmd_queue.dump_type_wait);
    skb_queue_head_init(&g_dump_cmd_queue.dump_type_queue);
    atomic_set(&g_dump_cmd_queue.rotate_finish_state, ROTATE_FINISH);
    ps_print_info("plat exception dump file rotate init success\n");
}

/*
 * Prototype    : plat_rotate_finish_set
 * Description  : app set rotate state when rotate finish
 */
void plat_rotate_finish_set(void)
{
    atomic_set(&g_dump_cmd_queue.rotate_finish_state, ROTATE_FINISH);
}

/*
 * Prototype    : plat_wait_last_rotate_finish
 * Description  : wait former app rotate finish
 */
void plat_wait_last_rotate_finish(void)
{
    uint8 retry = 0;

#define RETRY_TIME 3

    while (atomic_read(&g_dump_cmd_queue.rotate_finish_state) != ROTATE_FINISH) {
        /* maximum app rotate time is 0.1s */
        oal_udelay(100);
        if ((++retry) >= RETRY_TIME) {
            ps_print_warning("retry wait last rotate fail:retry =%d", retry);
            break;
        }
    }

    atomic_set(&g_dump_cmd_queue.rotate_finish_state, ROTATE_NOT_FINISH);
}

/*
 * Prototype    : plat_send_rotate_cmd_2_app
 * Description  : driver send rotate cmd to app for rotate file
 */
int32 plat_send_rotate_cmd_2_app(uint32 which_dump)
{
    struct sk_buff *skb = NULL;

    if (which_dump >= CMD_DUMP_BUFF) {
        ps_print_warning("which dump:%d error\n", which_dump);
        return -EINVAL;
    }
    if (skb_queue_len(&g_dump_cmd_queue.dump_type_queue) > MEMDUMP_ROTATE_QUEUE_MAX_LEN) {
        ps_print_warning("too many dump type in queue,dispose type:%d", which_dump);
        return -EINVAL;
    }

    skb = alloc_skb(sizeof(which_dump), GFP_KERNEL);
    if (skb == NULL) {
        ps_print_err("alloc errno skbuff failed! len=%d, errno=%x\n", (int32)sizeof(which_dump), which_dump);
        return -EINVAL;
    }
    skb_put(skb, sizeof(which_dump));
    *(uint32 *)skb->data = which_dump;
    skb_queue_tail(&g_dump_cmd_queue.dump_type_queue, skb);
    ps_print_info("save rotate cmd [%d] in queue\n", which_dump);

    wake_up_interruptible(&g_dump_cmd_queue.dump_type_wait);

    return 0;
}

/*
 * Prototype    : plat_dump_rotate_cmd_read
 * Description  : app read rotate cmd
 */
int32 plat_dump_rotate_cmd_read(uint64 arg)
{
    uint32 __user *puser = (uint32 __user *)(uintptr_t)arg;

    struct sk_buff *skb = NULL;

    if (!access_ok(VERIFY_WRITE, (uintptr_t)puser, (int32)sizeof(uint32))) {
        ps_print_err("address can not write\n");
        return -EINVAL;
    }

    if (wait_event_interruptible(g_dump_cmd_queue.dump_type_wait,
                                 (skb_queue_len(&g_dump_cmd_queue.dump_type_queue)) > 0)) {
        return -EINVAL;
    }

    skb = skb_dequeue(&g_dump_cmd_queue.dump_type_queue);
    if (skb == NULL) {
        ps_print_warning("skb is NULL\n");
        return -EINVAL;
    }

    if (copy_to_user(puser, skb->data, sizeof(uint32))) {
        ps_print_warning("copy_to_user err!restore it, len=%d\n", (int32)sizeof(uint32));
        skb_queue_head(&g_dump_cmd_queue.dump_type_queue, skb);
        return -EINVAL;
    }

    ps_print_info("read rotate cmd [%d] from queue\n", *(uint32 *)skb->data);

    skb_pull(skb, skb->len);
    kfree_skb(skb);

    return 0;
}

/*
 * Prototype    : hw_debug_ioctl
 * Description  : called by ini_plat_dfr_set
 */
STATIC int64 hw_debug_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    if (file == NULL) {
        ps_print_err("file is null\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PLAT_DFR_CFG_CMD:
            plat_dfr_cfg_set(arg);
            break;
        case PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD:
            plat_beattimer_timeout_reset_cfg_set(arg);
            break;
        case PLAT_DUMP_FILE_READ_CMD:
            plat_dump_rotate_cmd_read(arg);
            break;
        case PLAT_DUMP_ROTATE_FINISH_CMD:
            plat_rotate_finish_set();
            break;
        default:
            ps_print_warning("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}

#else
int32 plat_excp_dump_rotate_cmd_read(uint64 arg, memdump_info_t *memdump_info)
{
    uint32 __user *puser = (uint32 __user *)arg;

    struct sk_buff *skb = NULL;

    if (!access_ok(VERIFY_WRITE, puser, (int32)sizeof(uint32))) {
        ps_print_err("address can not write\n");
        return -EINVAL;
    }

    if (wait_event_interruptible(memdump_info->dump_type_wait, (skb_queue_len(&memdump_info->dump_type_queue)) > 0)) {
        return -EINVAL;
    }

    skb = skb_dequeue(&memdump_info->dump_type_queue);
    if (skb == NULL) {
        ps_print_warning("skb is NULL\n");
        return -EINVAL;
    }

    if (copy_to_user(puser, skb->data, sizeof(uint32))) {
        ps_print_warning("copy_to_user err!restore it, len=%d,arg=%ld\n", (int32)sizeof(uint32), arg);
        skb_queue_head(&memdump_info->dump_type_queue, skb);
        return -EINVAL;
    }

    ps_print_info("read rotate cmd [%d] from queue\n", *(uint32 *)skb->data);

    skb_pull(skb, skb->len);
    kfree_skb(skb);

    return 0;
}

int32 plat_bfgx_dump_rotate_cmd_read(uint64 arg)
{
    return plat_excp_dump_rotate_cmd_read(arg, &g_bcpu_memdump_cfg);
}

int32 plat_wifi_dump_rotate_cmd_read(uint64 arg)
{
    return plat_excp_dump_rotate_cmd_read(arg, &g_wcpu_memdump_cfg);
}

/*
 * Prototype    : hw_debug_ioctl
 * Description  : called by ini_plat_dfr_set
 */
STATIC int64 hw_debug_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    if (file == NULL) {
        ps_print_err("file is null\n");
        return -EINVAL;
    }

    switch (cmd) {
        case PLAT_DFR_CFG_CMD:
            plat_dfr_cfg_set(arg);
            break;
        case PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD:
            plat_beattimer_timeout_reset_cfg_set(arg);
            break;
        default:
            ps_print_warning("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}
#endif

int32 arm_timeout_submit(enum BFGX_THREAD_ENUM subs)
{
#define DFR_SUBMIT_LIMIT_TIME  300  /* second */
    static unsigned long long dfr_submit_last_time = 0;
    unsigned long long dfr_submit_current_time;
    unsigned long long dfr_submit_interval_time;
    struct timespec dfr_submit_time;

    if (subs >= BFGX_THREAD_BOTTOM) {
        return -EINVAL;
    }

    ps_print_info("[subs id:%d]arm timeout trigger", subs);

    dfr_submit_time = current_kernel_time();
    dfr_submit_current_time = dfr_submit_time.tv_sec;
    dfr_submit_interval_time = dfr_submit_current_time - dfr_submit_last_time;

    /* 5分钟内最多触发一次 */
    if ((dfr_submit_interval_time > DFR_SUBMIT_LIMIT_TIME) || (dfr_submit_last_time == 0)) {
        dfr_submit_last_time = dfr_submit_current_time;
        plat_exception_handler(SUBSYS_BFGX, subs, BFGX_ARP_TIMEOUT);
        return 0;
    } else {
        ps_print_err("[subs id:%d]arm timeout cnt max than limit", subs);
        return -EAGAIN;
    }
}

#ifdef HI110X_HAL_MEMDUMP_ENABLE
STATIC int32 hw_excp_read(struct file *filp, int8 __user *buf,
                          size_t count, loff_t *f_pos, memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;
    uint16 count1;

    if (unlikely((buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }
    if ((skb = skb_dequeue(&memdump_t->quenue)) == NULL) {
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        skb_queue_head(&memdump_t->quenue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&memdump_t->quenue, skb);
    }
    return count1;
}

STATIC ssize_t hw_bfgexcp_read(struct file *filp, int8 __user *buf,
                               size_t count, loff_t *f_pos)
{
    return hw_excp_read(filp, buf, count, f_pos, &g_bcpu_memdump_cfg);
}

STATIC int64 hw_bfgexcp_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    int32 ret = 0;
    if (file == NULL) {
        ps_print_err("file is null\n");
        return -EINVAL;
    }
    switch (cmd) {
        case PLAT_BFGX_DUMP_FILE_READ_CMD:
            ret = plat_bfgx_dump_rotate_cmd_read(arg);
            break;
        case DFR_HAL_GNSS_CFG_CMD:
            return arm_timeout_submit(THREAD_GNSS);
        case DFR_HAL_BT_CFG_CMD:
            return arm_timeout_submit(THREAD_BT);
        case DFR_HAL_FM_CFG_CMD:
            return arm_timeout_submit(THREAD_FM);
        default:
            ps_print_warning("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return ret;
}

STATIC int64 hw_wifiexcp_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    int32 ret = 0;

    if (file == NULL) {
        ps_print_err("file is null\n");
        return -EINVAL;
    }
    switch (cmd) {
        case PLAT_WIFI_DUMP_FILE_READ_CMD:
            ret = plat_wifi_dump_rotate_cmd_read(arg);
            break;
        default:
            ps_print_warning("hw_debug_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return ret;
}

STATIC ssize_t hw_wifiexcp_read(struct file *filp, int8 __user *buf,
                                size_t count, loff_t *f_pos)
{
    return hw_excp_read(filp, buf, count, f_pos, &g_wcpu_memdump_cfg);
}
#endif
/*
 * Prototype    : hw_debug_open
 * Description  : functions called from above oam hal,when open debug file
 * input        : "/dev/hwbfgdbg"
 * output       : return !0--> open is false
 *                return 0 --> open is ok
 */
STATIC int32 hw_debug_open(struct inode *inode, struct file *filp)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    oal_atomic_inc(&g_debug_cnt);
    ps_print_info("%s g_debug_cnt=%d\n", __func__, oal_atomic_read(&g_debug_cnt));

    atomic_set(&ps_core_d->dbg_recv_dev_log, 1);

    oal_atomic_set(&ps_core_d->dbg_read_delay, DBG_READ_DEFAULT_TIME);

    return 0;
}

/*
 * Prototype    : hw_debug_read
 * Description  : functions called from above oam hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
STATIC ssize_t hw_debug_read(struct file *filp, int8 __user *buf,
                             size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 count1 = 0;
    int64 timeout;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (ps_core_d->rx_dbg_seq.qlen == 0) { /* if no data, and wait timeout function */
        if (filp->f_flags & O_NONBLOCK) {  /* if O_NONBLOCK read and return */
            return -EAGAIN;
        }

        /* timeout function;when have data,can interrupt */
        timeout = wait_event_interruptible_timeout(ps_core_d->rx_dbg_wait,
                                                   (ps_core_d->rx_dbg_seq.qlen > 0),
                                                   msecs_to_jiffies(oal_atomic_read(&ps_core_d->dbg_read_delay)));
        if (!timeout) {
            ps_print_dbg("debug read time out!\n");
            return -ETIMEDOUT;
        }
    }

    /* pull skb data from skb queue */
    if ((skb = ps_skb_dequeue(ps_core_d, RX_DBG_QUEUE)) == NULL) {
        ps_print_dbg("dbg read no data!\n");
        return -ETIMEDOUT;
    }
    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("debug copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_DBG_QUEUE);
        return -EFAULT;
    }

    skb_pull(skb, count1);

    /* if skb->len == 0: curr skb read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_DBG_QUEUE);
    }

    return count1;
}

/*
 * Prototype    : hw_debug_write
 * Description  : functions called from above oam hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
#ifdef PLATFORM_DEBUG_ENABLE
STATIC int32 hw_debug_write_check(struct ps_core_s *ps_core_d)
{
    if (ps_core_d->tty_have_open == false) {
        ps_print_err("err: uart not opened!\n");
        return -EFAULT;
    }

    if (is_bfgx_exception() == PLAT_EXCEPTION_RESET_BUSY) {
        ps_print_err("dfr is processing, skip.\n");
        return -EBUSY;
    }

    return OAL_SUCC;
}

STATIC ssize_t hw_debug_write(struct file *filp, const int8 __user *buf,
                              size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 total_len;
    int32 ret = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (hw_debug_write_check(ps_core_d) < 0) {
        ps_print_err("check failed\n");
        return -EINVAL;
    }

    if (count > (DBG_TX_MAX_FRAME - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end))) {
        ps_print_err("err: dbg packet is too large!\n");
        return -EINVAL;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        return 0;
    }

    if (ps_chk_bfg_active(ps_core_d) == false) {
        ps_print_err("bfg is closed, /dev/hwdebug cant't write!!!\n");
        return -EINVAL;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return ret;
    }
    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    total_len = count + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);

    skb = ps_alloc_skb(total_len);
    if (skb == NULL) {
        ps_print_err("ps alloc skb mem fail\n");
        post_to_visit_node(ps_core_d);
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, count)) {
        ps_print_err("copy_from_user from dbg is err\n");
        kfree_skb(skb);
        post_to_visit_node(ps_core_d);
        return -EFAULT;
    }

    ps_add_packet_head(skb->data, OML_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    post_to_visit_node(ps_core_d);

    return count;
}
#endif

/*
 * Prototype    : hw_debug_release
 * Description  : called by oam func from hal when close debug inode
 * input        : have opened file handle
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
STATIC int32 hw_debug_release(struct inode *inode, struct file *filp)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (inode == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    oal_atomic_dec(&g_debug_cnt);
    ps_print_info("%s g_debug_cnt=%d", __func__, oal_atomic_read(&g_debug_cnt));
    if (oal_atomic_read(&g_debug_cnt) == 0) {
        /* wake up bt dbg wait queue */
        wake_up_interruptible(&ps_core_d->rx_dbg_wait);
        atomic_set(&ps_core_d->dbg_recv_dev_log, 0);

        /* kfree have rx dbg skb */
        ps_kfree_skb(ps_core_d, RX_DBG_QUEUE);
    }

    return 0;
}

#ifndef HI110X_HAL_MEMDUMP_ENABLE
STATIC int32 hw_excp_open(struct inode *inode, struct file *filp)
{
    ps_print_info("%s", __func__);
    return 0;
}

STATIC int32 hw_excp_release(struct inode *inode, struct file *filp)
{
    ps_print_info("%s", __func__);
    return 0;
}

STATIC int64 hw_excp_ioctl(struct file *file, uint32 cmd, uint64 arg)
{
    struct st_exception_info *pst_exception_data = NULL;
    if (file == NULL) {
        ps_print_err("file is null\n");
        return -EINVAL;
    }
    ps_print_info("%s", __func__);

    get_exception_info_reference(&pst_exception_data);
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EINVAL;
    }

    switch (cmd) {
        case DFR_HAL_GNSS_CFG_CMD:
            return arm_timeout_submit(THREAD_GNSS);
            break;
        case DFR_HAL_BT_CFG_CMD:
            return arm_timeout_submit(THREAD_BT);
            break;
        case DFR_HAL_FM_CFG_CMD:
            return arm_timeout_submit(THREAD_FM);
            break;
        default:
            ps_print_warning("hw_excp_ioctl cmd = %d not find\n", cmd);
            return -EINVAL;
    }

    return 0;
}
#endif

uart_loop_cfg g_uart_loop_test_cfg = { 256, 60000, 0, 0, 0 };
uart_loop_test_struct *g_uart_loop_test_info = NULL;

void uart_loop_test_tx_buf_init(uint8 *puc_data, uint16 us_data_len)
{
    get_random_bytes(puc_data, us_data_len);
}

int32 uart_loop_set_pkt_count(uint32 count)
{
    ps_print_info("uart loop test, set pkt count to [%d]\n", count);
    g_uart_loop_test_cfg.loop_count = count;

    return 0;
}

int32 uart_loop_set_pkt_len(uint32 pkt_len)
{
    ps_print_info("uart loop test, set pkt len to [%d]\n", pkt_len);
    g_uart_loop_test_cfg.pkt_len = pkt_len;

    return 0;
}

int32 alloc_uart_loop_test(void)
{
    uint8 *uart_loop_tx_buf = NULL;
    uint8 *uart_loop_rx_buf = NULL;
    uint16 pkt_len = 0;

    if (g_uart_loop_test_info == NULL) {
        g_uart_loop_test_info = (uart_loop_test_struct *)kzalloc(sizeof(uart_loop_test_struct), GFP_KERNEL);
        if (g_uart_loop_test_info == NULL) {
            ps_print_err("malloc g_uart_loop_test_info fail\n");
            goto malloc_test_info_fail;
        }

        pkt_len = g_uart_loop_test_cfg.pkt_len;
        if (pkt_len == 0 || pkt_len > UART_LOOP_MAX_PKT_LEN) {
            pkt_len = UART_LOOP_MAX_PKT_LEN;
            g_uart_loop_test_cfg.pkt_len = UART_LOOP_MAX_PKT_LEN;
        }

        uart_loop_tx_buf = (uint8 *)kzalloc(pkt_len, GFP_KERNEL);
        if (uart_loop_tx_buf == NULL) {
            ps_print_err("malloc uart_loop_tx_buf fail\n");
            goto malloc_tx_buf_fail;
        }

        memset_s(uart_loop_tx_buf, pkt_len, 0xa5, pkt_len);

        uart_loop_rx_buf = (uint8 *)kzalloc(pkt_len, GFP_KERNEL);
        if (uart_loop_rx_buf == NULL) {
            ps_print_err("malloc uart_loop_rx_buf fail\n");
            goto malloc_rx_buf_fail;
        }

        g_uart_loop_test_cfg.uart_loop_enable = 1;
        g_uart_loop_test_cfg.uart_loop_tx_random_enable = 1;

        init_completion(&g_uart_loop_test_info->set_done);
        init_completion(&g_uart_loop_test_info->loop_test_done);

        g_uart_loop_test_info->test_cfg = &g_uart_loop_test_cfg;
        g_uart_loop_test_info->tx_buf = uart_loop_tx_buf;
        g_uart_loop_test_info->rx_buf = uart_loop_rx_buf;
        g_uart_loop_test_info->rx_pkt_len = 0;

        ps_print_info("uart loop test, pkt len is [%d]\n", pkt_len);
        ps_print_info("uart loop test, loop count is [%d]\n", g_uart_loop_test_cfg.loop_count);
    }

    return 0;

malloc_rx_buf_fail:
    kfree(uart_loop_tx_buf);
malloc_tx_buf_fail:
    kfree(g_uart_loop_test_info);
    g_uart_loop_test_info = NULL;
malloc_test_info_fail:
    return -ENOMEM;
}

void free_uart_loop_test(void)
{
    if (g_uart_loop_test_info == NULL) {
        return;
    }
    ps_print_err("free uart loop test buf\n");
    g_uart_loop_test_cfg.uart_loop_enable = 0;
    kfree(g_uart_loop_test_info->rx_buf);
    kfree(g_uart_loop_test_info->tx_buf);
    kfree(g_uart_loop_test_info);
    g_uart_loop_test_info = NULL;

    return;
}

int32 uart_loop_test_open(void)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 error;

    ps_print_info("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL) ||
                 (ps_core_d->ps_pm->bfg_power_set == NULL))) {
        ps_print_err("ps_core_d is err\n");
        return -EINVAL;
    }

    if (ps_chk_bfg_active(ps_core_d)) {
        ps_print_err("bfgx subsys must all close\n");
        return -EINVAL;
    }

    if (alloc_uart_loop_test() != BFGX_POWER_SUCCESS) {
        ps_print_err("alloc mem for uart loop test fail!\n");
        goto alloc_mem_fail;
    }

    error = ps_core_d->ps_pm->bfg_power_set(BFGX_GNSS, BFG_POWER_GPIO_UP);
    if (error != BFGX_POWER_SUCCESS) {
        ps_print_err("uart loop test, power on err! error = %d\n", error);
        goto power_on_fail;
    }

    if (prepare_to_visit_node(ps_core_d) != BFGX_POWER_SUCCESS) {
        ps_print_err("uart loop test, prepare work fail\n");
        error = BFGX_POWER_WAKEUP_FAIL;
        goto wakeup_fail;
    }

    post_to_visit_node(ps_core_d);

    return BFGX_POWER_SUCCESS;

wakeup_fail:
    ps_core_d->ps_pm->bfg_power_set(BFGX_GNSS, BFG_POWER_GPIO_DOWN);
power_on_fail:
    free_uart_loop_test();
alloc_mem_fail:
    return BFGX_POWER_FAILED;
}

int32 uart_loop_test_close(void)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s", __func__);

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (prepare_to_visit_node(ps_core_d) != 0) {
        ps_print_err("uart loop test, prepare work fail\n");
    }

    if (ps_core_d->ps_pm->bfg_power_set(BFGX_GNSS, BFG_POWER_GPIO_DOWN) != 0) {
        ps_print_err("uart loop test, power off err!");
    }

    free_uart_loop_test();

    post_to_visit_node(ps_core_d);

    return 0;
}

int32 uart_loop_test_set(uint8 flag)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;
    uint8 cmd;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EINVAL;
    }

    if (flag == UART_LOOP_SET_DEVICE_DATA_HANDLER) {
        cmd = SYS_CFG_SET_UART_LOOP_HANDLER;
    } else {
        cmd = SYS_CFG_SET_UART_LOOP_FINISH;
    }

    if (prepare_to_visit_node(ps_core_d) < 0) {
        ps_print_err("prepare wakeup fail\n");
        return -EFAULT;
    }

    oal_reinit_completion(g_uart_loop_test_info->set_done);
    ps_uart_state_pre(ps_core_d->tty);
    ps_tx_sys_cmd(ps_core_d, SYS_MSG, cmd);
    timeleft = wait_for_completion_timeout(&g_uart_loop_test_info->set_done, msecs_to_jiffies(5000));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait set uart loop ack timeout\n");
        post_to_visit_node(ps_core_d);
        return -ETIMEDOUT;
    }

    post_to_visit_node(ps_core_d);
    return 0;
}

int32 uart_loop_test_send_data(struct ps_core_s *ps_core_d, uint8 *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    uint16 tx_gnss_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    while (count > 0) {
        if (count > GNSS_TX_PACKET_LIMIT) {
            tx_gnss_len = GNSS_TX_PACKET_LIMIT;
        } else {
            tx_gnss_len = count;
        }
        /* curr tx skb total lenth */
        tx_skb_len = tx_gnss_len + sizeof(struct ps_packet_head);
        tx_skb_len = tx_skb_len + sizeof(struct ps_packet_end);

        skb = ps_alloc_skb(tx_skb_len);
        if (skb == NULL) {
            ps_print_err("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > GNSS_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, GNSS_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, GNSS_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, GNSS_LAST_MSG, tx_skb_len);
        }

        if (memcpy_s(&skb->data[sizeof(struct ps_packet_head)], tx_skb_len - sizeof(struct ps_packet_head),
                     buf, tx_gnss_len) != EOK) {
            ps_print_err("buf is not enough\n");
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_gnss_len;
        count = count - tx_gnss_len;
    }

    return 0;
}

int32 uart_loop_test_send_pkt(void)
{
    uint64 timeleft;
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (g_uart_loop_test_info == NULL) ||
                 (g_uart_loop_test_info->tx_buf == NULL))) {
        ps_print_err("para is invalided\n");
        return -EFAULT;
    }

    /* if low queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_low_seq.qlen > TX_LOW_QUE_MAX_NUM) {
        ps_print_err("uart loop test, tx low seq is too large [%d]\n", ps_core_d->tx_low_seq.qlen);
        return 0;
    }

    if (prepare_to_visit_node(ps_core_d) < 0) {
        ps_print_err("prepare work fail\n");
        return -EFAULT;
    }

    oal_reinit_completion(g_uart_loop_test_info->loop_test_done);

    /* to divide up packet function and tx to tty work */
    if (uart_loop_test_send_data(ps_core_d, g_uart_loop_test_info->tx_buf,
                                 g_uart_loop_test_cfg.pkt_len) < 0) {
        ps_print_err("uart loop test pkt send is err\n");
        post_to_visit_node(ps_core_d);
        return -EFAULT;
    }

    timeleft = wait_for_completion_timeout(&g_uart_loop_test_info->loop_test_done, msecs_to_jiffies(5000));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait uart loop done timeout\n");
        post_to_visit_node(ps_core_d);
        return -ETIMEDOUT;
    }

    post_to_visit_node(ps_core_d);

    return 0;
}

int32 uart_loop_test_data_check(uint8 *puc_src, uint8 *puc_dest, uint16 us_data_len)
{
    uint16 us_index;

    for (us_index = 0; us_index < us_data_len; us_index++) {
        if (puc_src[us_index] != puc_dest[us_index]) {
            return false;
        }
    }

    return true;
}

int32 uart_loop_test_recv_pkt(struct ps_core_s *ps_core_d, const uint8 *buf_ptr, uint16 pkt_len)
{
    uint16 expect_pkt_len;
    uint8 *rx_buf = NULL;
    uint16 recvd_len;

    if (unlikely((ps_core_d == NULL) || (g_uart_loop_test_info == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    expect_pkt_len = g_uart_loop_test_info->test_cfg->pkt_len;
    rx_buf = g_uart_loop_test_info->rx_buf;
    recvd_len = g_uart_loop_test_info->rx_pkt_len;

    if ((uint32)recvd_len + (uint32)pkt_len <= expect_pkt_len) {
        if (memcpy_s(&rx_buf[recvd_len], expect_pkt_len - recvd_len, buf_ptr, pkt_len) != EOK) {
            ps_print_err("memcpy_s error, destlen=%d, srclen=%d\n ", expect_pkt_len - recvd_len, pkt_len);
        }
        g_uart_loop_test_info->rx_pkt_len += pkt_len;
    } else {
        ps_print_err("pkt len err! pkt_len=[%d], recvd_len=[%d], max len=[%d]\n", pkt_len, recvd_len, expect_pkt_len);
    }

    if (expect_pkt_len == g_uart_loop_test_info->rx_pkt_len) {
        if (uart_loop_test_data_check(rx_buf, g_uart_loop_test_info->tx_buf, expect_pkt_len)) {
            ps_print_info("uart loop recv pkt SUCC\n");
        }
        g_uart_loop_test_info->rx_pkt_len = 0;
        complete(&g_uart_loop_test_info->loop_test_done);
    }

    return 0;
}

int32 uart_loop_test(void)
{
    uint32 i, count;
    uint16 pkt_len;
    unsigned long long tx_total_len;
    unsigned long long total_time = 0;
    unsigned long long throughout;
    unsigned long long effect;
    ktime_t start_time, end_time, trans_time;
    uint8 *puc_buf = NULL;

    if (uart_loop_test_open() < 0) {
        goto open_fail;
    }

    if (uart_loop_test_set(UART_LOOP_SET_DEVICE_DATA_HANDLER) < 0) {
        goto test_set_fail;
    }

    count = g_uart_loop_test_info->test_cfg->loop_count;
    pkt_len = g_uart_loop_test_info->test_cfg->pkt_len;
    tx_total_len = ((unsigned long long)count) * ((unsigned long long)pkt_len);
    puc_buf = g_uart_loop_test_info->tx_buf;

    for (i = 0; i < count; i++) {
        if (g_uart_loop_test_info->test_cfg->uart_loop_tx_random_enable) {
            uart_loop_test_tx_buf_init(puc_buf, pkt_len);  // 初始化tx_buf为随机数
        }

        start_time = ktime_get();

        if (uart_loop_test_send_pkt() != SUCCESS) {
            ps_print_err("uart loop test fail, i=[%d]\n", i);
            goto send_pkt_fail;
        }

        end_time = ktime_get();
        trans_time = ktime_sub(end_time, start_time);
        total_time += (unsigned long long)ktime_to_us(trans_time);
    }

    if (uart_loop_test_set(UART_LOOP_RESUME_DEVICE_DATA_HANDLER) < 0) {
        ps_print_err("uart loop test, resume device data handler failer\n");
    }

    uart_loop_test_close();

    if (total_time == 0) {
        ps_print_err("error!divisor is zero\n");
        return -FAILURE;
    }

    /*
     *  将传输字节数转换成bit数(B->b)以方便后文计算
     *  1000000 :1M波特率
     *   10=8+2 :uart每传输1字节数据需要另加2bit的开始位和结束位
     *     2    :因为uart是环回的，发1字节数据还会回1字节数据
     */
    throughout = tx_total_len * 1000000 * 10 * 2;
    do_div(throughout, total_time);
    effect = throughout;
    do_div(throughout, 8192); /* b->B->KB : x*8*1024 */
    do_div(effect, (g_default_baud_rate / 100)); /* 除以波特率算出百分比效率 */

    ps_print_info("[UART Test] pkt count      [%d] pkts sent\n", count);
    ps_print_info("[UART Test] pkt len        [%d] is pkt len\n", pkt_len);

    /* uart是环回的，发1字节数据还会回1字节数据 */
    ps_print_info("[UART Test] data lenth     [%llu]\n", tx_total_len * 2);
    ps_print_info("[UART Test] used time      [%llu] us\n", total_time);
    ps_print_info("[UART Test] throughout     [%llu] KBps\n", throughout);
    ps_print_info("[UART Test] effect         [%llu]%%\n", effect);

    return SUCCESS;

send_pkt_fail:
test_set_fail:
    uart_loop_test_close();
open_fail:
    return -FAILURE;
}

int conn_test_uart_loop(char *param)
{
    return uart_loop_test();
}
EXPORT_SYMBOL(conn_test_uart_loop);

OAL_STATIC const struct file_operations g_hw_bt_fops = {
    .owner = THIS_MODULE,
    .open = hw_bt_open,
    .write = hw_bt_write,
    .read = hw_bt_read,
    .poll = hw_bt_poll,
    .unlocked_ioctl = hw_bt_ioctl,
    .release = hw_bt_release,
};

OAL_STATIC const struct file_operations g_hw_fm_fops = {
    .owner = THIS_MODULE,
    .open = hw_fm_open,
    .write = hw_fm_write,
    .read = hw_fm_read,
    .unlocked_ioctl = hw_fm_ioctl,
    .release = hw_fm_release,
};

OAL_STATIC const struct file_operations g_hw_gnss_fops = {
    .owner = THIS_MODULE,
    .open = hw_gnss_open,
    .write = hw_gnss_write,
    .read = hw_gnss_read,
    .poll = hw_gnss_poll,
    .unlocked_ioctl = hw_gnss_ioctl,
    .release = hw_gnss_release,
};

OAL_STATIC const struct file_operations g_hw_ir_fops = {
    .owner = THIS_MODULE,
    .open = hw_ir_open,
    .write = hw_ir_write,
    .read = hw_ir_read,
    .release = hw_ir_release,
};

OAL_STATIC const struct file_operations g_hw_nfc_fops = {
    .owner = THIS_MODULE,
    .open = hw_nfc_open,
    .write = hw_nfc_write,
    .read = hw_nfc_read,
    .poll = hw_nfc_poll,
    .release = hw_nfc_release,
};

OAL_STATIC const struct file_operations g_hw_debug_fops = {
    .owner = THIS_MODULE,
    .open = hw_debug_open,
#ifdef PLATFORM_DEBUG_ENABLE
    .write = hw_debug_write,
#endif
    .read = hw_debug_read,
    .unlocked_ioctl = hw_debug_ioctl,
    .release = hw_debug_release,
};

#ifdef HI110X_HAL_MEMDUMP_ENABLE
OAL_STATIC const struct file_operations g_hw_bfgexcp_fops = {
    .owner = THIS_MODULE,
    .read = hw_bfgexcp_read,
    .unlocked_ioctl = hw_bfgexcp_ioctl,
};

OAL_STATIC const struct file_operations g_hw_wifiexcp_fops = {
    .owner = THIS_MODULE,
    .read = hw_wifiexcp_read,
    .unlocked_ioctl = hw_wifiexcp_ioctl,
};

#else
OAL_STATIC const struct file_operations g_hw_excp_fops = {
    .owner = THIS_MODULE,
    .open = hw_excp_open,
    .unlocked_ioctl = hw_excp_ioctl,
    .release = hw_excp_release,
};
#endif
#ifdef HAVE_HISI_BT
OAL_STATIC struct miscdevice g_hw_bt_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbt",
    .fops = &g_hw_bt_fops,
};
#endif

#ifdef HAVE_HISI_FM
OAL_STATIC struct miscdevice g_hw_fm_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwfm",
    .fops = &g_hw_fm_fops,
};
#endif

#ifdef HAVE_HISI_GNSS
OAL_STATIC struct miscdevice g_hw_gnss_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwgnss",
    .fops = &g_hw_gnss_fops,
};
#endif

#ifdef HAVE_HISI_IR
OAL_STATIC struct miscdevice g_hw_ir_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwir",
    .fops = &g_hw_ir_fops,
};
#endif

#ifdef HAVE_HISI_NFC
OAL_STATIC struct miscdevice g_hw_nfc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwnfc",
    .fops = &g_hw_nfc_fops,
};
#endif

OAL_STATIC struct miscdevice g_hw_debug_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbfgdbg",
    .fops = &g_hw_debug_fops,
};
#ifdef HI110X_HAL_MEMDUMP_ENABLE
OAL_STATIC struct miscdevice g_hw_bfgexcp_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwbfgexcp",
    .fops = &g_hw_bfgexcp_fops,
};
OAL_STATIC struct miscdevice g_hw_wifiexcp_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwwifiexcp",
    .fops = &g_hw_wifiexcp_fops,
};

#else
OAL_STATIC struct miscdevice g_hw_excp_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "hwexcp",
    .fops = &g_hw_excp_fops,
};
#endif

OAL_STATIC struct hw_ps_plat_data g_hisi_platform_data = {
    .dev_name = "/dev/ttyAMA4",
    .flow_cntrl = FLOW_CTRL_ENABLE,
    .baud_rate = DEFAULT_BAUD_RATE,
    .suspend = NULL,
    .resume = NULL,
    .set_bt_power = NULL,
    .set_fm_power = NULL,
    .set_gnss_power = NULL,
    .clear_bt_power = NULL,
    .clear_fm_power = NULL,
    .clear_gnss_power = NULL,
};

#ifdef HAVE_HISI_NFC
OAL_STATIC int plat_poweroff_notify_sys(struct notifier_block *this, unsigned long code, void *unused)
{
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_gnss_data = NULL;
    struct inode gnss_inode;
    struct file gnss_filp;
    int32 err;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("plat_poweroff_notify_sys get ps_core_d is NULL\n");
        return NOTIFY_BAD;
    }

    pst_gnss_data = &ps_core_d->bfgx_info[BFGX_GNSS];
    if (atomic_read(&pst_gnss_data->subsys_state) == POWER_STATE_OPEN) {
        err = hw_gnss_release(&gnss_inode, &gnss_filp);
        if (err != 0) {
            ps_print_warning("plat_poweroff_notify_sys call hw_gnss_release failed (err=%d)\n", err);
        }
    }
    return NOTIFY_OK;
}

OAL_STATIC struct notifier_block g_plat_poweroff_notifier = {
    .notifier_call = plat_poweroff_notify_sys,
};
#endif

#ifdef _PRE_CONFIG_USE_DTS
STATIC int32_t ps_platform_data_init(struct platform_device *pdev)
{
    board_info *bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -FAILURE;
    }

    strncpy_s(g_hisi_platform_data.dev_name, sizeof(g_hisi_platform_data.dev_name),
              bd_info->uart_port, sizeof(g_hisi_platform_data.dev_name) - 1);
    g_hisi_platform_data.dev_name[sizeof(g_hisi_platform_data.dev_name) - 1] = '\0';

    /* FPGA版本支持2M，动态修改 */
    if (!is_asic()) {
        g_hisi_platform_data.baud_rate = LOW_FREQ_BAUD_RATE;
        g_default_baud_rate = LOW_FREQ_BAUD_RATE;
    }

    pdev->dev.platform_data = &g_hisi_platform_data;

    g_hw_ps_device = pdev;

    return 0;
}

STATIC int32_t ps_plat_init(struct platform_device *pdev)
{
    int32 err;
    struct ps_plat_s *ps_plat_d = NULL;
    struct hw_ps_plat_data *pdata = NULL;

    ps_plat_d = kzalloc(sizeof(struct ps_plat_s), GFP_KERNEL);
    if (ps_plat_d == NULL) {
        ps_print_err("no mem to allocate\n");
        return -ENOMEM;
    }
    dev_set_drvdata(&pdev->dev, ps_plat_d);

    err = ps_core_init(&ps_plat_d->core_data);
    if (err != 0) {
        ps_print_err(" PS core init failed\n");
        goto err_core_init;
    }

    /* refer to itself */
    ps_plat_d->core_data->pm_data = ps_plat_d;
    /* get reference of pdev */
    ps_plat_d->pm_pdev = pdev;

    init_completion(&ps_plat_d->ldisc_uninstalled);
    init_completion(&ps_plat_d->ldisc_installed);
    init_completion(&ps_plat_d->ldisc_reconfiged);

    err = plat_bfgx_exception_rst_register(ps_plat_d);
    if (err < 0) {
        ps_print_err("bfgx_exception_rst_register failed\n");
        goto err_exception_rst_reg;
    }

    pdata = &g_hisi_platform_data;
    /* copying platform data */
    err = strcpy_s(ps_plat_d->dev_name, sizeof(ps_plat_d->dev_name), pdata->dev_name);
    if (err != EOK) {
        ps_print_err("plat dev name: %s copy failed.\n", pdata->dev_name);
    }
    ps_plat_d->flow_cntrl = pdata->flow_cntrl;
    ps_plat_d->baud_rate = pdata->baud_rate;

    return 0;

err_exception_rst_reg:
    ps_core_exit(ps_plat_d->core_data);
err_core_init:
    kfree(ps_plat_d);

    return err;
}

STATIC void ps_plat_exit(struct platform_device *pdev)
{
    struct ps_plat_s *ps_plat_d = NULL;

    ps_plat_d = dev_get_drvdata(&pdev->dev);
    if (ps_plat_d == NULL) {
        ps_print_err("ps_plat_d is null\n");
        return;
    }

    ps_core_exit(ps_plat_d->core_data);
    kfree(ps_plat_d);
}

STATIC int32_t ps_misc_dev_register(void)
{
    int32 err;

    PS_PRINT_FUNCTION_NAME;
#ifdef HAVE_HISI_BT
    err = misc_register(&g_hw_bt_device);
    if (err != 0) {
        ps_print_err("Failed to register bt inode\n ");
        goto err_register_bt;
    }
#endif

#ifdef HAVE_HISI_FM
    err = misc_register(&g_hw_fm_device);
    if (err != 0) {
        ps_print_err("Failed to register fm inode\n ");
        goto err_register_fm;
    }
#endif

#ifdef HAVE_HISI_GNSS
    err = misc_register(&g_hw_gnss_device);
    if (err != 0) {
        ps_print_err("Failed to register gnss inode\n ");
        goto err_register_gnss;
    }
#endif

#ifdef HAVE_HISI_IR
    err = misc_register(&g_hw_ir_device);
    if (err != 0) {
        ps_print_err("Failed to register ir inode\n");
        goto err_register_ir;
    }
#endif

    return 0;

#ifdef HAVE_HISI_IR
err_register_ir:
#endif
#ifdef HAVE_HISI_GNSS
    misc_deregister(&g_hw_gnss_device);
err_register_gnss:
#endif
#ifdef HAVE_HISI_FM
    misc_deregister(&g_hw_fm_device);
err_register_fm:
#endif
#ifdef HAVE_HISI_BT
    misc_deregister(&g_hw_bt_device);
err_register_bt:
#endif

    return err;
}

STATIC void ps_misc_dev_unregister(void)
{
#ifdef HAVE_HISI_BT
    misc_deregister(&g_hw_bt_device);
    ps_print_info("misc bt device have removed\n");
#endif
#ifdef HAVE_HISI_FM
    misc_deregister(&g_hw_fm_device);
    ps_print_info("misc fm device have removed\n");
#endif
#ifdef HAVE_HISI_GNSS
    misc_deregister(&g_hw_gnss_device);
    ps_print_info("misc gnss device have removed\n");
#endif
#ifdef HAVE_HISI_IR
    misc_deregister(&g_hw_ir_device);
    ps_print_info("misc ir have removed\n");
#endif
}

STATIC int32_t ps_exception_misc_register(void)
{
    int32 err;

    PS_PRINT_FUNCTION_NAME;

    err = misc_register(&g_hw_debug_device);
    if (err != 0) {
        ps_print_err("Failed to register debug inode\n");
        return err;
    }
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    err = misc_register(&g_hw_bfgexcp_device);
    if (err != 0) {
        ps_print_err("Failed to register g_hw_bfgexcp_device inode\n");
        goto err_register_bfgexcp;
    }

    err = misc_register(&g_hw_wifiexcp_device);
    if (err != 0) {
        ps_print_err("Failed to register g_hw_wifiexcp_device inode\n");
        goto err_register_wifiexcp;
    }
#else
    err = misc_register(&g_hw_excp_device);
    if (err != 0) {
        ps_print_err("Failed to register hw excp inode\n");
        goto err_register_excp;
    }
#endif

    return 0;
#ifdef HI110X_HAL_MEMDUMP_ENABLE
err_register_wifiexcp:
    misc_deregister(&g_hw_bfgexcp_device);
err_register_bfgexcp:
#else
err_register_excp:
#endif
    misc_deregister(&g_hw_debug_device);

    return err;
}

STATIC void ps_exception_misc_unregister(void)
{
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    misc_deregister(&g_hw_wifiexcp_device);
    misc_deregister(&g_hw_bfgexcp_device);
#else
    misc_deregister(&g_hw_excp_device);
#endif
    misc_deregister(&g_hw_debug_device);
}

STATIC int32_t ps_probe(struct platform_device *pdev)
{
    int32 err;

    err = ps_platform_data_init(pdev);
    if (err != 0) {
        return -EFAULT;
    }

    err = ps_plat_init(pdev);
    if (err != 0) {
        ps_print_err("ps_plat_init failed err %d\n", err);
        return -EFAULT;
    }

    err = bfgx_user_ctrl_init();
    if (err < 0) {
        ps_print_err("bfgx_user_ctrl_init failed\n");
        goto err_user_ctrl_init;
    }

    err = bfgx_customize_init();
    if (err < 0) {
        ps_print_err("bfgx_customize_init failed\n");
        goto err_bfgx_custmoize_exit;
    }

    tty_recv = ps_core_recv;

    err = ps_misc_dev_register();
    if (err != 0) {
        ps_print_err("ps_misc_dev_register failed err %d\n", err);
        goto err_misc_dev;
    }

    err = ps_exception_misc_register();
    if (err != 0) {
        ps_print_err("ps_exception_misc_register failed err %d\n", err);
        goto err_excep_dev;
    }

    ps_print_suc("%s is success!\n", __func__);

    return 0;

err_excep_dev:
    ps_misc_dev_unregister();
err_misc_dev:
    cali_data_buf_free();
err_bfgx_custmoize_exit:
    bfgx_user_ctrl_exit();
err_user_ctrl_init:
    ps_plat_exit(pdev);

    return -EFAULT;
}

/*
 * Prototype    : ps_suspend
 * Description  : called by kernel when kernel goto suspend
 */
int32 ps_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

/*
 * Prototype    : ps_resume
 * Description  : called by kernel when kernel resume from suspend
 */
int32 ps_resume(struct platform_device *pdev)
{
    return 0;
}

/*
 * Prototype    : ps_remove
 * Description  : called when user applation rmmod driver
 */
STATIC int32 ps_remove(struct platform_device *pdev)
{
    PS_PRINT_FUNCTION_NAME;

    bfgx_user_ctrl_exit();
    ps_print_info("sysfs user ctrl removed\n");

    cali_data_buf_free();

    ps_misc_dev_unregister();
    ps_exception_misc_unregister();
    ps_print_info("misc debug device have removed\n");

    ps_plat_exit(pdev);

    return 0;
}

OAL_STATIC struct of_device_id g_hi110x_ps_match_table[] = {
    {
        .compatible = DTS_COMP_HI110X_PS_NAME,
        .data = NULL,
    },
    {},
};
#endif

/*  platform_driver struct for PS module */
OAL_STATIC struct platform_driver g_ps_platform_driver = {
#ifdef _PRE_CONFIG_USE_DTS
    .probe = ps_probe,
    .remove = ps_remove,
    .suspend = ps_suspend,
    .resume = ps_resume,
#endif
    .driver = {
        .name = "hisi_bfgx",
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = g_hi110x_ps_match_table,
#endif
    },
};

int32 hw_ps_init(void)
{
    int32 ret;

    PS_PRINT_FUNCTION_NAME;

    ret = platform_driver_register(&g_ps_platform_driver);
    if (ret) {
        ps_print_err("Unable to register platform bfgx driver.\n");
    }
    return ret;
}

void hw_ps_exit(void)
{
    platform_driver_unregister(&g_ps_platform_driver);
}

MODULE_DESCRIPTION("Public serial Driver for huawei BT/FM/GNSS chips");
MODULE_LICENSE("GPL");
