

/* 头文件包含 */
#include <linux/sched.h>

#include "plat_debug.h"
#include "plat_uart.h"
#include "bfgx_exception_rst.h"
#include "plat_pm.h"
#include "bfgx_platform_msg_handle.h"


/* 全局变量定义 */
typedef void (*platform_msg_handler)(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void platform_unused_msg(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void wcpu_message_handle(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_uart_msg_handle(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_thread_msg_handle(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_agree_host_sleep(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_not_agree_host_sleep(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_read_mem_complete(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_set_uart_loop_complete(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void bfgx_recv_device_chr(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC void gnss_trickle_request_sleep(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);

STATIC platform_msg_handler g_sys_msg_handle[SYS_INF_BUTT] = {
    platform_unused_msg,         // SYS_INF_PF_INIT = 0x00,                平台软件初始化完成
    bfgx_thread_msg_handle,      // SYS_INF_BT_INIT = 0x01,                BT软件初始化完成
    bfgx_thread_msg_handle,      // SYS_INF_GNSS_INIT = 0x02,              GNSS软件初始化完成
    bfgx_thread_msg_handle,      // SYS_INF_FM_INIT = 0x03,                FM软件初始化完成
    platform_unused_msg,         // SYS_INF_BT_DISABLE = 0x04,             BT禁能
    platform_unused_msg,         // SYS_INF_GNSS_DISABLE = 0x05,           GNSS禁能
    platform_unused_msg,         // SYS_INF_FM_DISABLE = 0x06,             FM禁能
    bfgx_thread_msg_handle,      // SYS_INF_BT_EXIT = 0x07,                BT退出
    bfgx_thread_msg_handle,      // SYS_INF_GNSS_EXIT = 0x08,              GNSS退出
    bfgx_thread_msg_handle,      // SYS_INF_FM_EXIT = 0x09,                FM退出
    platform_unused_msg,         // SYS_INF_GNSS_WAIT_DOWNLOAD = 0x0A,     等待GNSS代码下载
    platform_unused_msg,         // SYS_INF_GNSS_DOWNLOAD_COMPLETE = 0x0B, GNSS代码下载完毕
    platform_unused_msg,         // SYS_INF_BFG_HEART_BEAT = 0x0C,         心跳信号
    bfgx_agree_host_sleep,       // SYS_INF_DEV_AGREE_HOST_SLP = 0x0D,     device回复host可睡
    bfgx_not_agree_host_sleep,   // SYS_INF_DEV_NOAGREE_HOST_SLP = 0x0E,   device回复host不可睡
    wcpu_message_handle,         // SYS_INF_WIFI_OPEN = 0x0F,              WCPU上电完成
    bfgx_thread_msg_handle,      // SYS_INF_IR_INIT = 0x10,                IR软件初始化完成
    bfgx_thread_msg_handle,      // SYS_INF_IR_EXIT = 0x11,                IR退出
    bfgx_thread_msg_handle,      // SYS_INF_NFC_INIT = 0x12,               NFC软件初始化完成
    bfgx_thread_msg_handle,      // SYS_INF_NFC_EXIT = 0x13,               NFC退出
    wcpu_message_handle,         // SYS_INF_WIFI_CLOSE = 0x14,             WCPU下电完成
    platform_unused_msg,         // SYS_INF_RF_TEMP_NORMAL = 0x15,         RF温度正常
    platform_unused_msg,         // SYS_INF_RF_TEMP_OVERHEAT = 0x16,       RF温度过热
    bfgx_read_mem_complete,      // SYS_INF_MEM_DUMP_COMPLETE = 0x17,      bfgx异常时，MEM DUMP已完成
    wcpu_message_handle,         // SYS_INF_WIFI_MEM_DUMP_COMPLETE = 0X18, bfgx异常时，MEM DUMP已完成
    wcpu_message_handle,         // SYS_INF_UART_HALT_WCPU = 0x19,         uart halt wcpu ok
    bfgx_set_uart_loop_complete, // SYS_INF_UART_LOOP_SET_DONE = 0x1a,     device 设置uart环回ok
    bfgx_recv_device_chr,        // SYS_INF_CHR_ERRNO_REPORT = 0x1b,       device向host上报CHR异常码
    bfgx_uart_msg_handle,        // SYS_INF_BAUD_CHG_2M_REQ = 0x1c,        device向host请求切换2M波特率
    bfgx_uart_msg_handle,        // SYS_INF_BAUD_CHG_6M_REQ = 0x1d,        device向host请求切换6M波特率
    bfgx_uart_msg_handle,        // SYS_INF_BAUD_CHG_COMP_ACK = 0x1e,      device对host的波特率切换完成通知消息的ACK
    bfgx_thread_msg_handle,      // SYS_INF_GNSS_LPPE_INIT = 0x1F,         1103 GNSS 新增的线程初始化完成
    bfgx_thread_msg_handle,      // SYS_INF_GNSS_LPPE_EXIT = 0x20,         1103 GNSS 新增的线程退出
    gnss_trickle_request_sleep,  // SYS_INF_GNSS_TRICKLE_SLEEP = 0x21,     1103 GNSS TRICKLE Sleep
};


STATIC void ps_exe_heartbeat_func(struct ps_core_s *ps_core_d, uint8_t syschar)
{
    struct st_exception_info *pst_exception_data = NULL;
    struct pm_drv_data *pm_data;

    pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("%ds,tty_tx=%d,tty_rx=%d,uart_tx=%d,uart_rx=%d\n",
                  syschar - SYS_INF_HEARTBEAT_CMD_BASE - 1, ps_uart_state_cur(STATE_TTY_TX),
                  ps_uart_state_cur(STATE_TTY_RX), ps_uart_state_cur(STATE_UART_TX),
                  ps_uart_state_cur(STATE_UART_RX));
    ps_print_info("pkt_num_rx:BT=%lu,GNSS=%lu,FM=%lu,IR=%lu,SYS_MSG=%lu,OML=%lu,timer=%d,mod_cnt=%d,gnss_sleep_flag=%d\n",
                  ps_core_d->rx_pkt_num[BFGX_BT], ps_core_d->rx_pkt_num[BFGX_GNSS],
                  ps_core_d->rx_pkt_num[BFGX_FM], ps_core_d->rx_pkt_num[BFGX_IR],
                  ps_core_d->rx_pkt_sys, ps_core_d->rx_pkt_oml, timer_pending(&pm_data->bfg_timer),
                  pm_data->bfg_timer_mod_cnt, atomic_read(&pm_data->gnss_sleep_flag));
    ps_print_info("pkt_num_tx:BT=%lu,GNSS=%lu,FM=%lu,IR=%lu\n",
                  ps_core_d->tx_pkt_num[BFGX_BT], ps_core_d->tx_pkt_num[BFGX_GNSS],
                  ps_core_d->tx_pkt_num[BFGX_FM], ps_core_d->tx_pkt_num[BFGX_IR]);

    get_exception_info_reference(&pst_exception_data);
    if (pst_exception_data != NULL) {
        if (pst_exception_data->debug_beat_flag == 1) {
            atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_RECV_BEAT_INFO);
        }
    }
}

STATIC void platform_unused_msg(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    ps_print_warning("unused msg:%d, need to check\n", *buf_ptr);
}

STATIC void wcpu_message_handle(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t msg = *buf_ptr;
    struct st_exception_info *pst_exception_data = NULL;

    switch (msg) {
        case SYS_INF_WIFI_OPEN:
            ps_print_info("WIFI has opened by BFNGI\n");
            complete(&ps_core_d->wait_wifi_opened);
            break;
        case SYS_INF_WIFI_CLOSE:
            ps_print_info("WIFI has closed by BFNGI\n");
            complete(&ps_core_d->wait_wifi_closed);
            break;
        case SYS_INF_UART_HALT_WCPU:
            get_exception_info_reference(&pst_exception_data);
            if (pst_exception_data != NULL) {
                ps_print_info("uart halt wcpu ok\n");
                complete(&pst_exception_data->wait_uart_halt_wcpu);
            }
            break;
        case SYS_INF_WIFI_MEM_DUMP_COMPLETE:
            ps_print_info("uart wifi mem dump complete\n");
            store_wifi_mem_to_file();
            break;
        default:
            ps_print_err("wcpu msg error:%d\n", msg);
            break;
    }
}

STATIC void bfgx_uart_msg_handle(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t msg = *buf_ptr;

    switch (msg) {
        case SYS_INF_BAUD_CHG_2M_REQ:
        case SYS_INF_BAUD_CHG_6M_REQ:
            bfgx_uart_rcv_baud_change_req(msg);
            break;
        case SYS_INF_BAUD_CHG_COMP_ACK:
            bfgx_uart_rcv_baud_change_complete_ack();
            break;
        default:
            ps_print_err("uart msg error:%d\n", msg);
            break;
    }
}

STATIC void thread_completion_handle(struct ps_core_s *ps_core_d, uint8_t flag, uint8_t thread)
{
    if (thread >= BFGX_BUTT) {
        ps_print_err("thread index error:%d\n", thread);
        return;
    }

    if (flag == THREAD_FLAG_INIT) {
        ps_print_info("%s thread init success\n", ps_core_d->bfgx_info[thread].name);
        oal_complete(&ps_core_d->bfgx_info[thread].wait_opened);
    } else if (flag == THREAD_FLAG_EXIT) {
        ps_print_info("%s thread exit success\n", ps_core_d->bfgx_info[thread].name);
        oal_complete(&ps_core_d->bfgx_info[thread].wait_closed);
    } else {
        ps_print_err("bfgx thread handle flag error:%d, thread:%d", flag, thread);
    }
}

STATIC void gnss_thread_handle(struct ps_core_s *ps_core_d, uint8_t flag, uint8_t info)
{
    switch (info) {
        case SYS_INF_GNSS_INIT:
            ps_print_info("GNSS me thread has opened\n");
            g_gnss_me_thread_status = DEV_THREAD_INIT;
            break;
        case SYS_INF_GNSS_LPPE_INIT:
            ps_print_info("GNSS lppe thread has opened\n");
            g_gnss_lppe_thread_status = DEV_THREAD_INIT;
            break;
        case SYS_INF_GNSS_EXIT:
            ps_print_info("GNSS me thread has exit\n");
            g_gnss_me_thread_status = DEV_THREAD_EXIT;
            break;
        case SYS_INF_GNSS_LPPE_EXIT:
            ps_print_info("GNSS lppe thread has exit\n");
            g_gnss_lppe_thread_status = DEV_THREAD_EXIT;
            break;
        default:
            ps_print_warning("GNSS thread info is error %d\n", info);
            break;
    }

    if (g_gnss_me_thread_status == g_gnss_lppe_thread_status) {
        thread_completion_handle(ps_core_d, flag, BFGX_GNSS);
    }
}

STATIC void bfgx_thread_msg_handle(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t msg = *buf_ptr;

    switch (msg) {
        case SYS_INF_BT_INIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_INIT, BFGX_BT);
            break;
        case SYS_INF_FM_INIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_INIT, BFGX_FM);
            break;
        case SYS_INF_IR_INIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_INIT, BFGX_IR);
            break;
#ifdef HAVE_HISI_NFC
        case SYS_INF_NFC_INIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_INIT, BFGX_NFC);
            break;
        case SYS_INF_NFC_EXIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_EXIT, BFGX_NFC);
            break;
#endif
        case SYS_INF_GNSS_INIT:
        case SYS_INF_GNSS_LPPE_INIT:
            gnss_thread_handle(ps_core_d, THREAD_FLAG_INIT, msg);
            break;
        case SYS_INF_BT_EXIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_EXIT, BFGX_BT);
            break;
        case SYS_INF_FM_EXIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_EXIT, BFGX_FM);
            break;
        case SYS_INF_IR_EXIT:
            thread_completion_handle(ps_core_d, THREAD_FLAG_EXIT, BFGX_IR);
            break;
        case SYS_INF_GNSS_EXIT:
        case SYS_INF_GNSS_LPPE_EXIT:
            gnss_thread_handle(ps_core_d, THREAD_FLAG_EXIT, msg);
            break;
        default:
            ps_print_err("thread msg error:%d\n", msg);
            break;
    };
}

STATIC void bfgx_not_agree_host_sleep(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    struct pm_drv_data *pm_data = NULL;

    pm_data = pm_get_drvdata();;
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("bfgx dev not agree host sleep\n");

    del_timer_sync(&pm_data->dev_ack_timer);
    ps_core_d->ps_pm->bfgx_dev_state_set(BFGX_ACTIVE);
    ps_core_d->ps_pm->bfgx_uart_state_set(UART_READY);

    if (!bfgx_other_subsys_all_shutdown(BFGX_GNSS)) {
        mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(BT_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;
    }

    complete(&pm_data->dev_ack_comp);
}

STATIC void bfgx_agree_host_sleep(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    unsigned long flags;
    struct pm_drv_data *pm_data;

    pm_data = pm_get_drvdata();;
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("dev ack to agree to sleep\n");
    if (timer_pending(&pm_data->dev_ack_timer)) {
        ps_print_dbg("dev ack to agree to sleep in normal case\n");
        del_timer_sync(&pm_data->dev_ack_timer);
        complete(&pm_data->dev_ack_comp);
        if (unlikely(pm_data->rcvdata_bef_devack_flag == 1)) {
            ps_print_info("device send data to host before dev rcv allow slp msg\n");
            pm_data->rcvdata_bef_devack_flag = 0;
        } else if (atomic_read(&ps_core_d->node_visit_flag) > 0) {
            ps_print_info("someone visit dev node during waiting for dev_ack\n");
        } else {
            spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
            if (pm_data->bfgx_dev_state == BFGX_ACTIVE) {
                ps_print_info("dev wkup isr occur during waiting for dev_ack\n");
            } else {
                pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
                bfg_wake_unlock();
            }
            spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);
        }
    } else {
        ps_print_info("dev ack to agree to sleep after devack timer expired,gpio state:%d\n",
                      board_get_bwkup_gpio_val());
        spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
        ps_core_d->ps_pm->bfgx_uart_state_set(UART_NOT_READY);
        ps_core_d->ps_pm->bfgx_dev_state_set(BFGX_SLEEP);
        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
    }
}

STATIC void bfgx_read_mem_complete(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    struct st_exception_info *pst_exception_data = NULL;

#ifdef HI110X_HAL_MEMDUMP_ENABLE
    bfgx_memdump_finish();
#endif

    get_exception_info_reference(&pst_exception_data);
    if (pst_exception_data != NULL) {
        ps_print_info("plat bfgx mem dump complete\n");
        atomic_set(&pst_exception_data->is_memdump_runing, 0);
        complete_all(&pst_exception_data->wait_read_bfgx_stack);
#ifndef HI110X_HAL_MEMDUMP_ENABLE
        if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
            bfgx_store_stack_mem_to_file();
        }
#endif
    }
}

STATIC void bfgx_set_uart_loop_complete(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    if (g_uart_loop_test_info != NULL) {
        ps_print_info("bfgx uart loop test set ok\n");
        complete(&g_uart_loop_test_info->set_done);
    }
}

STATIC void bfgx_recv_device_chr(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
#ifdef CONFIG_HI1102_PLAT_HW_CHR
    chr_errno_with_arg_stru *pst_chr = (chr_errno_with_arg_stru *)(buf_ptr + 1);
#endif

    ps_print_info("rcv devcie chr report\n");

#ifdef CONFIG_HI1102_PLAT_HW_CHR
    if (pst_chr->errlen > CHR_ERR_DATA_MAX_LEN) {
        ps_print_err("bfgx recv chr wrong, errno:0x%x, errlen:%d\n", pst_chr->errno, pst_chr->errlen);
        return;
    }

    chr_dev_exception_callback((uint8_t *)pst_chr, (pst_chr->errlen + OAL_SIZEOF(chr_errno_with_arg_stru)));
#else
    ps_print_info("host chr not support, ignore errno:0x%x\n", *(uint32_t *)buf_ptr);
#endif
}


STATIC void gnss_trickle_request_sleep(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    struct pm_drv_data *pm_data;

    pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("gnss trickle sleep!\n");

    if (!timer_pending(&pm_data->bfg_timer)) {
        ps_print_suc("gnss low power request sleep!\n");

        if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
            ps_print_info("queue_work send_allow_sleep_work not return true\n");
        }
    }

    /* set the flag to 1 means gnss request sleep */
    atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
}


/*
 * Prototype    : ps_exe_sys_func
 * Description  : called by core when recive sys data event from device
 *              : decode rx sys packet function
 */
int32 ps_exe_sys_func(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint8 syschar;

    PS_PRINT_FUNCTION_NAME;

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }

    ps_print_dbg("recv system data\n");
    syschar = *buf_ptr;
    ps_core_d->rx_pkt_sys++;
    if (syschar < SYS_INF_BUTT) {
        if (g_sys_msg_handle[syschar] != NULL) {
            g_sys_msg_handle[syschar](ps_core_d, buf_ptr);
        }
    } else if (syschar >= SYS_INF_HEARTBEAT_CMD_BASE) {
        ps_exe_heartbeat_func(ps_core_d, syschar);
    } else {
        ps_print_warning("bfgx recv unsupport msg:0x%x\n", syschar);
    }

    return 0;
}
