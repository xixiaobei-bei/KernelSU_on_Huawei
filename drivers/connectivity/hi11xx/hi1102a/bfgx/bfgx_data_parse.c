

/*****************************************************************************
  Include Head file
*****************************************************************************/
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/tty.h>

#include "board.h"
#include "hw_bfg_ps.h"
#include "plat_type.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "bfgx_exception_rst.h"
#include "bfgx_user_ctrl.h"
#include "plat_pm.h"
#include "oal_ext_if.h"
#include "plat_cali.h"
#include "securec.h"
#ifdef CONFIG_HI1102_PLAT_HW_CHR
#include "chr_user.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 bfgx_panic_ssi_dump = 0;
oal_debug_module_param(bfgx_panic_ssi_dump, int, S_IRUGO | S_IWUSR);
#endif

uint8 uc_wakeup_src_debug = 0;
static unsigned int reset_cnt = 0;

/* function pointer for rx data */
int32 (*tty_recv)(void *, const uint8 *, int32);

uint32 bfgx_rx_max_frame[BFGX_BUTT] = {
    BT_RX_MAX_FRAME,
    FM_RX_MAX_FRAME,
    GNSS_RX_MAX_FRAME,
    IR_RX_MAX_FRAME,
    NFC_RX_MAX_FRAME,
};

uint32 bfgx_rx_queue[BFGX_BUTT] = {
    RX_BT_QUEUE,
    RX_FM_QUEUE,
    RX_GNSS_QUEUE,
    RX_IR_QUEUE,
    RX_NFC_QUEUE,
};

uint32 bfgx_rx_queue_max_num[BFGX_BUTT] = {
    RX_BT_QUE_MAX_NUM,
    RX_FM_QUE_MAX_NUM,
    RX_GNSS_QUE_MAX_NUM,
    RX_IR_QUE_MAX_NUM,
    RX_NFC_QUE_MAX_NUM,
};

/*****************************************************************************
  Function implement
*****************************************************************************/

void print_uart_decode_param(void)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return;
    }

    PS_PRINT_WARNING("rx_pkt_type=%x,rx_have_recv_pkt_len=%d,rx_pkt_total_len=%d,rx_have_del_public_len=%d\n",
                     ps_core_d->rx_pkt_type, ps_core_d->rx_have_recv_pkt_len,
                     ps_core_d->rx_pkt_total_len, ps_core_d->rx_have_del_public_len);
    return;
}

/*
 * Prototype    : ps_write_tty
 * Description  : can be called when write data to uart
 *                  This is the internal write function - a wrapper
 *                  to tty->ops->write
 * input        : data -> the ptr of send data buf
 *                count -> the size of send data buf
 * output       : no
 */
int32 ps_write_tty(struct ps_core_s *ps_core_d, const uint8 *data, int32 count)
{
    struct tty_struct *tty;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely((ps_core_d == NULL) || (ps_core_d->tty == NULL))) {
        PS_PRINT_ERR("tty unavailable to perform write\n");
        return -EINVAL;
    }

    tty = ps_core_d->tty;

    ps_uart_tty_tx_add(count);
    return tty->ops->write(tty, data, count);
}

void gnss_thread_init(struct ps_core_s *ps_core_d, uint8 info)
{
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return;
    }

    if (info == SYS_INF_GNSS_INIT) {
        PS_PRINT_INFO("GNSS me thread has opened\n");
        gnss_me_thread_status = DEV_THREAD_INIT;
    } else if (info == SYS_INF_GNSS_LPPE_INIT) {
        PS_PRINT_INFO("GNSS lppe thread has opened\n");
        gnss_lppe_thread_status = DEV_THREAD_INIT;
    } else {
        PS_PRINT_WARNING("GNSS thread info is error %d\n", info);
        return;
    }

    if ((gnss_me_thread_status == DEV_THREAD_INIT) && (gnss_lppe_thread_status == DEV_THREAD_INIT)) {
        complete(&ps_core_d->bfgx_info[BFGX_GNSS].wait_opened);
    }

    return;
}

void gnss_thread_exit(struct ps_core_s *ps_core_d, uint8 info)
{
    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is null\n");
        return;
    }

    if (info == SYS_INF_GNSS_EXIT) {
        PS_PRINT_INFO("GNSS me thread has exit\n");
        gnss_me_thread_status = DEV_THREAD_EXIT;
    } else if (info == SYS_INF_GNSS_LPPE_EXIT) {
        PS_PRINT_INFO("GNSS lppe thread has exit\n");
        gnss_lppe_thread_status = DEV_THREAD_EXIT;
    } else {
        PS_PRINT_WARNING("GNSS thread info is error %d\n", info);
        return;
    }

    if ((gnss_me_thread_status == DEV_THREAD_EXIT) && (gnss_lppe_thread_status == DEV_THREAD_EXIT)) {
        complete(&ps_core_d->bfgx_info[BFGX_GNSS].wait_closed);
    }

    return;
}

static void ps_exe_heartbeat_func(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data, uint8 syschar)
{
    struct st_exception_info *pst_exception_data = NULL;

    if (syschar >= SYS_INF_HEARTBEAT_CMD_BASE) {
        PS_PRINT_INFO("%ds,tty_tx=%d,tty_rx=%d,uart_tx=%d,uart_rx=%d\n",
                      syschar - SYS_INF_HEARTBEAT_CMD_BASE - 1, ps_uart_state_cur(STATE_TTY_TX),
                      ps_uart_state_cur(STATE_TTY_RX), ps_uart_state_cur(STATE_UART_TX),
                      ps_uart_state_cur(STATE_UART_RX));
        PS_PRINT_INFO("pkt_num_rx:BT=%lu,GNSS=%lu,FM=%lu,IR=%lu,SYS_MSG=%lu,OML=%lu,timer=%d,mod_cnt=%d,gnss_sleep_flag=%d\n",
                      ps_core_d->rx_pkt_num[BFGX_BT], ps_core_d->rx_pkt_num[BFGX_GNSS],
                      ps_core_d->rx_pkt_num[BFGX_FM], ps_core_d->rx_pkt_num[BFGX_IR],
                      ps_core_d->rx_pkt_sys, ps_core_d->rx_pkt_oml, timer_pending(&pm_data->bfg_timer),
                      pm_data->bfg_timer_mod_cnt, atomic_read(&pm_data->gnss_sleep_flag));
        PS_PRINT_INFO("pkt_num_tx:BT=%lu,GNSS=%lu,FM=%lu,IR=%lu\n",
                      ps_core_d->tx_pkt_num[BFGX_BT], ps_core_d->tx_pkt_num[BFGX_GNSS],
                      ps_core_d->tx_pkt_num[BFGX_FM], ps_core_d->tx_pkt_num[BFGX_IR]);

        get_exception_info_reference(&pst_exception_data);
        if (NULL != pst_exception_data) {
            if (1 == pst_exception_data->debug_beat_flag) {
                atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_RECV_BEAT_INFO);
            }
        }
    }
}

/*
 * Prototype    : ps_exe_sys_func
 * Description  : called by core when recive sys data event from device
 *              : decode rx sys packet function
 */

int32 ps_exe_sys_func(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    struct pm_drv_data *pm_data = NULL;
    struct st_exception_info *pst_exception_data = NULL;
#ifdef CONFIG_HI1102_PLAT_HW_CHR
    CHR_ERRNO_WITH_ARG_STRU *pst_chr = NULL;
#endif
    uint8 syschar;
    uint64 flags;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EINVAL;
    }

    syschar = *buf_ptr;

    switch (syschar) {
        case SYS_INF_PF_INIT:
            PS_PRINT_INFO("plat bfgx init complete\n");
            break;
        case SYS_INF_BT_INIT:
            PS_PRINT_INFO("BT has opened\n");
            complete(&ps_core_d->bfgx_info[BFGX_BT].wait_opened);
            break;
        case SYS_INF_WIFI_OPEN:
            PS_PRINT_INFO("WIFI has opened by BFNGI\n");
            complete(&ps_core_d->wait_wifi_opened);
            break;
        case SYS_INF_WIFI_CLOSE:
            PS_PRINT_INFO("WIFI has closed by BFNGI\n");
            complete(&ps_core_d->wait_wifi_closed);
            break;
        case SYS_INF_GNSS_INIT:
        case SYS_INF_GNSS_LPPE_INIT:
            gnss_thread_init(ps_core_d, syschar);
            break;
        case SYS_INF_FM_INIT:
            PS_PRINT_INFO("FM has opened\n");
            complete(&ps_core_d->bfgx_info[BFGX_FM].wait_opened);
            break;
        case SYS_INF_IR_INIT:
            PS_PRINT_INFO("IR has opened\n");
            complete(&ps_core_d->bfgx_info[BFGX_IR].wait_opened);
            break;
        case SYS_INF_NFC_INIT:
            PS_PRINT_INFO("NFC has opened\n");
            complete(&ps_core_d->bfgx_info[BFGX_NFC].wait_opened);
            break;
        case SYS_INF_BT_DISABLE:
            break;
        case SYS_INF_GNSS_DISABLE:
            break;
        case SYS_INF_FM_DISABLE:
            break;
        case SYS_INF_BT_EXIT:
            PS_PRINT_INFO("BT has closed\n");
            complete(&ps_core_d->bfgx_info[BFGX_BT].wait_closed);
            break;
        case SYS_INF_GNSS_EXIT:
        case SYS_INF_GNSS_LPPE_EXIT:
            gnss_thread_exit(ps_core_d, syschar);
            break;
        case SYS_INF_FM_EXIT:
            PS_PRINT_INFO("FM has closed\n");
            complete(&ps_core_d->bfgx_info[BFGX_FM].wait_closed);
            break;
        case SYS_INF_IR_EXIT:
            PS_PRINT_INFO("IR has closed\n");
            complete(&ps_core_d->bfgx_info[BFGX_IR].wait_closed);
            break;
        case SYS_INF_NFC_EXIT:
            PS_PRINT_INFO("NFC has closed\n");
            complete(&ps_core_d->bfgx_info[BFGX_NFC].wait_closed);
            break;
        case SYS_INF_GNSS_WAIT_DOWNLOAD:
            break;
        case SYS_INF_GNSS_DOWNLOAD_COMPLETE:
            break;
        case SYS_INF_DEV_NOAGREE_HOST_SLP:
            PS_PRINT_INFO("dev ack to no agree to sleep\n");
            del_timer_sync(&pm_data->dev_ack_timer);
            ps_core_d->ps_pm->bfgx_dev_state_set(BFGX_ACTIVE);
            ps_core_d->ps_pm->bfgx_uart_state_set(UART_READY);

            if (!bfgx_other_subsys_all_shutdown(BFGX_GNSS)) {
                mod_timer(&pm_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
                pm_data->bfg_timer_mod_cnt++;
            }

            complete(&pm_data->dev_ack_comp);
            break;
        case SYS_INF_DEV_AGREE_HOST_SLP:
            PS_PRINT_INFO("dev ack to agree to sleep\n");
            if (timer_pending(&pm_data->dev_ack_timer)) {
                PS_PRINT_DBG("dev ack to agree to sleep in normal case\n");
                del_timer_sync(&pm_data->dev_ack_timer);
                complete(&pm_data->dev_ack_comp);
                if (unlikely(pm_data->rcvdata_bef_devack_flag == 1)) {
                    PS_PRINT_INFO("device send data to host before dev rcv allow slp msg\n");
                    pm_data->rcvdata_bef_devack_flag = 0;
                } else if (atomic_read(&ps_core_d->node_visit_flag) > 0) {
                    PS_PRINT_INFO("someone visit dev node during waiting for dev_ack\n");
                } else {
                    spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
                    if (pm_data->bfgx_dev_state == BFGX_ACTIVE) {
                        PS_PRINT_INFO("dev wkup isr occur during waiting for dev_ack\n");
                    } else {
                        pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
                        bfg_wake_unlock();
                    }
                    spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);
                }
            } else {
                PS_PRINT_INFO("dev ack to agree to sleep after devack timer expired,gpio state:%d\n",
                              board_get_bwkup_gpio_val());
                spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
                ps_core_d->ps_pm->bfgx_uart_state_set(UART_NOT_READY);
                ps_core_d->ps_pm->bfgx_dev_state_set(BFGX_SLEEP);
                spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
            }
            break;
        case SYS_INF_MEM_DUMP_COMPLETE:
#ifdef HI110X_HAL_MEMDUMP_ENABLE
            bfgx_memdump_finish();
#endif
            get_exception_info_reference(&pst_exception_data);
            if (NULL != pst_exception_data) {
                PS_PRINT_INFO("plat bfgx mem dump complete\n");
                complete(&pst_exception_data->wait_read_bfgx_stack);
#ifndef HI110X_HAL_MEMDUMP_ENABLE
                if (PLAT_EXCEPTION_ENABLE != pst_exception_data->exception_reset_enable) {
                    bfgx_store_stack_mem_to_file();
                }
#endif
            }
            break;
        case SYS_INF_WIFI_MEM_DUMP_COMPLETE:
            PS_PRINT_INFO("uart wifi mem dump complete\n");
            store_wifi_mem_to_file();
            break;
        case SYS_INF_UART_HALT_WCPU:
            get_exception_info_reference(&pst_exception_data);
            if (NULL != pst_exception_data) {
                PS_PRINT_INFO("uart halt wcpu ok\n");
                complete(&pst_exception_data->wait_uart_halt_wcpu);
            }
            break;
        case SYS_INF_UART_LOOP_SET_DONE:
            if (NULL != uart_loop_test_info) {
                PS_PRINT_INFO("device uart loop test set ok\n");
                complete(&uart_loop_test_info->set_done);
            }
            break;
        case SYS_INF_CHR_ERRNO_REPORT:
            PS_PRINT_INFO("rcv SYS_INF_CHR_ERRNO_REPORT");
#ifdef CONFIG_HI1102_PLAT_HW_CHR
            pst_chr = (CHR_ERRNO_WITH_ARG_STRU *)(buf_ptr + 1);

            if (pst_chr->errlen > CHR_ERR_DATA_MAX_LEN) {
                PS_PRINT_ERR("host CHR system get dev errno wrong, errno 0x%x\n", pst_chr->errno);
                break;
            }

            chr_dev_exception_callback((uint8 *)pst_chr, (pst_chr->errlen + OAL_SIZEOF(CHR_ERRNO_WITH_ARG_STRU)));
#else
            PS_PRINT_INFO("host chr not support,ignore errno:0x%x\n", *(uint32 *)buf_ptr);
#endif

            break;
        case SYS_INF_BAUD_CHG_2M_REQ:
        case SYS_INF_BAUD_CHG_6M_REQ:
            bfgx_uart_rcv_baud_change_req(syschar);
            break;
        case SYS_INF_BAUD_CHG_COMP_ACK:
            bfgx_uart_rcv_baud_change_complete_ack();
            break;
        case SYS_INF_GNSS_TRICKLE_SlEEP:
            PS_PRINT_INFO("gnss trickle sleep!\n");
            if (!timer_pending(&pm_data->bfg_timer)) {
                PS_PRINT_SUC("gnss low power request sleep!\n");
                if (true != queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work)) {
                    PS_PRINT_INFO("queue_work send_allow_sleep_work not return true\n");
                }
            }

            /* set the flag to 1 means gnss request sleep */
            atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
            break;
        default:
            /* 心跳信号 */
            ps_exe_heartbeat_func(ps_core_d, pm_data, syschar);
            break;
    }
    return 0;
}

/*
 * Prototype    : ps_push_skb_queue
 * Description  : alloc skb buf,and memcopy data to skb buff
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_push_skb_queue(struct ps_core_s *ps_core_d, uint8 *buf_ptr, uint16 pkt_len, uint8 type)
{
    struct sk_buff *skb = NULL;
    int32 ret;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    skb = alloc_skb(pkt_len, GFP_ATOMIC);
    if (skb == NULL) {
        PS_PRINT_ERR("can't allocate mem for new skb, len=%d\n", pkt_len);
        return -EINVAL;
    }

    skb_put(skb, pkt_len);
    ret = memcpy_s(skb->data, pkt_len, buf_ptr, pkt_len);
    if (ret != EOK) {
        PS_PRINT_ERR("skb data copy failed\n");
        kfree_skb(skb);
        return -EINVAL;
    }

    ps_skb_enqueue(ps_core_d, skb, type);

    return 0;
}

int32 delete_gnss_head_skb_msg(void)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint8 seperate_tag = 0;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    do {
        if ((skb = ps_skb_dequeue(ps_core_d, RX_GNSS_QUEUE)) == NULL) {
            PS_PRINT_WARNING("gnss skb rx queue is empty\n");
            return 0;
        }

        PS_PRINT_WARNING("gnss delete msg, skb->len=%d, qlen=%d\n",
                         skb->len, ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen);

        seperate_tag = skb->data[skb->len - 1];
        switch (seperate_tag) {
            case GNSS_SEPER_TAG_INIT:
            case GNSS_SEPER_TAG_LAST:
                kfree_skb(skb);
                break;
            default:
                PS_PRINT_ERR("seperate_tag=%x not support\n", seperate_tag);
                kfree_skb(skb);
                seperate_tag = GNSS_SEPER_TAG_INIT;
                break;
        }
    } while (seperate_tag == GNSS_SEPER_TAG_INIT);

    return 0;
}

#define ALLOC_GNSS_SKB_TIMEOUT  5
int32 ps_push_skb_queue_gnss(struct ps_core_s *ps_core_d, uint8 *buf_ptr, uint16 pkt_len, uint8 type)
{
    struct sk_buff *skb = NULL;

    uint8 seperate_tag = 0;
    uint16 seperate_len = 0;
    int32 copy_cnt = 0;
    int32 ret;
    uint32  i;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }
    spin_lock(&ps_core_d->gnss_rx_lock);
    do {
        if ((pkt_len - copy_cnt) < GNSS_RX_SEPERATE_FRAME_MAX_LEN) {
            seperate_tag = GNSS_SEPER_TAG_LAST;
            seperate_len = pkt_len - copy_cnt + 1;
        } else {
            seperate_tag = GNSS_SEPER_TAG_INIT;
            seperate_len = GNSS_RX_SEPERATE_FRAME_MAX_LEN;
        }

        for (i = 0; i < ALLOC_GNSS_SKB_TIMEOUT; i++) {
            skb = alloc_skb(seperate_len, GFP_ATOMIC);
            if (skb != NULL) {
                break;
            }
            PS_PRINT_ERR("alloc new skb fail [%u]time\n", i+1);
        }
        if (skb == NULL) {
            PS_PRINT_ERR("===can't alloc mem for new skb, tag:%x, slen=%x, pkt_len=%x\n",
                         seperate_tag, seperate_len, pkt_len);
            while (copy_cnt > 0) {
                if ((skb = skb_dequeue_tail(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue)) == NULL) {
                    PS_PRINT_ERR("===gnss rx queue has no data!\n");
                    spin_unlock(&ps_core_d->gnss_rx_lock);
                    return -EINVAL;
                }

                PS_PRINT_ERR("===drop gnss seperate data, tag:%x, slen=%x, skb->len=%x, pkt_len=%x\n",
                             seperate_tag, seperate_len, skb->len, pkt_len);
                skb_pull(skb, skb->len);
                kfree_skb(skb);
                copy_cnt -= (GNSS_RX_SEPERATE_FRAME_MAX_LEN - 1);
            }
            spin_unlock(&ps_core_d->gnss_rx_lock);
            return -EINVAL;
        }

        skb_put(skb, seperate_len);
        ret = memcpy_s(skb->data, seperate_len, buf_ptr + copy_cnt, seperate_len - 1);
        if (ret != EOK) {
            PS_PRINT_ERR("skb_data memcpy_s failed\n");
            spin_unlock(&ps_core_d->gnss_rx_lock);
            kfree_skb(skb);
            return -EINVAL;
        }
        skb->data[seperate_len - 1] = seperate_tag;
        PS_PRINT_DBG("===tag:%x, slen=%x, skb_len=%x, copy_cnt=%x, pkt_len=%x\n",
                     seperate_tag, seperate_len, skb->len, copy_cnt + seperate_len - 1, pkt_len);

        ps_skb_enqueue(ps_core_d, skb, type);
        copy_cnt += (seperate_len - 1);
    } while (copy_cnt < pkt_len);

    /* 确保gnss缓存的skb不超过(RX_GNSS_QUE_MAX_NUM >> 1)个，防止一会删前面的，一会删后面的 */
    while (ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen > (RX_GNSS_QUE_MAX_NUM >> 1)) {
        delete_gnss_head_skb_msg();
    }
    spin_unlock(&ps_core_d->gnss_rx_lock);

    return 0;
}

/*
 * Prototype    : ps_push_skb_debug_queue
 * Description  : alloc skb buf,and memcopy data to skb buff
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_push_skb_debug_queue(struct ps_core_s *ps_core_d, const uint8 *buf_ptr,
                              uint16 pkt_len, uint8 type)
{
    struct sk_buff *skb = NULL;
    uint16 count;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    skb = skb_dequeue_tail(&ps_core_d->rx_dbg_seq);
    if (skb != NULL) {
        if (EOK == memcpy_s(skb_tail_pointer(skb), skb_tailroom(skb), buf_ptr, pkt_len)) {
            skb_put(skb, pkt_len);
            ps_skb_enqueue(ps_core_d, skb, type);
            return 0;
        } else {
            ps_skb_enqueue(ps_core_d, skb, type);
        }
    }

    count = max_t(size_t, pkt_len, DEBUG_SKB_BUFF_LEN);
    skb = alloc_skb(count, GFP_ATOMIC);
    if (skb == NULL) {
        PS_PRINT_ERR("can't allocate mem for new debug skb, len=%d\n", count);
        return -EINVAL;
    }

    if (unlikely(memcpy_s(skb_tail_pointer(skb), count, buf_ptr, pkt_len) != EOK)) {
        PS_PRINT_ERR("memcopy_s error, destlen=%d, srclen=%d\n ", count, pkt_len);
    }
    skb_put(skb, pkt_len);
    ps_skb_enqueue(ps_core_d, skb, type);

    return 0;
}

int32 ps_recv_mem_dump_size_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint32 dump_mem_size;
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    uint16 rx_pkt_total_len;
#endif

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }
#if ((_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_HOST) && (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_HOST))
    dump_mem_size = *(uint16 *)buf_ptr;
#else
    dump_mem_size = *(uint32 *)buf_ptr;
#endif

    PS_PRINT_INFO("prepare to recv bfgx mem size [%d]!\n", dump_mem_size);
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    bfgx_notice_hal_memdump();
    bfgx_memdump_enquenue(buf_ptr, rx_pkt_total_len);
#else
    prepare_to_recv_bfgx_stack(dump_mem_size);
#endif
    return 0;
}

int32 ps_recv_mem_dump_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr, uint32 system)
{
    uint16 rx_pkt_total_len;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if (system == SUBSYS_BFGX) {
        if (rx_pkt_total_len <= MEM_DUMP_RX_MAX_FRAME) {
            PS_PRINT_DBG("recv bfgx stack size [%d]!\n", rx_pkt_total_len);
#ifdef HI110X_HAL_MEMDUMP_ENABLE
            bfgx_memdump_enquenue(buf_ptr, rx_pkt_total_len);
#else
            bfgx_recv_dev_mem(buf_ptr, rx_pkt_total_len);
#endif
        }
    } else if (system == SUBSYS_WIFI) {
        if (rx_pkt_total_len <= WIFI_MEM_RX_MAX_FRAME) {
            PS_PRINT_INFO("recv wifi mem size [%d]!\n", rx_pkt_total_len);
            uart_recv_wifi_mem(buf_ptr, rx_pkt_total_len);
        }
    } else {
        PS_PRINT_ERR("param system is error, system=[%d]\n", system);
    }

    return 0;
}

int32 ps_recv_bfgx_cali_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    uint16 rx_pkt_total_len;
    uint8 *cali_data_buf = NULL;
    uint32 cali_data_len = 0;
    uint32 copy_len = 0;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    cali_data_buf = bfgx_get_cali_data_buf(&cali_data_len);
    if (cali_data_buf == NULL) {
        PS_PRINT_ERR("bfgx cali data buf is NULL\n");
        return -EINVAL;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    copy_len = (rx_pkt_total_len > cali_data_len) ? cali_data_len : rx_pkt_total_len;

    PS_PRINT_INFO("recv bfgx cali data, rx_len=%d,copy_len=%d\n", rx_pkt_total_len, copy_len);
    if (unlikely(memcpy_s(cali_data_buf, cali_data_len, buf_ptr, copy_len) != EOK)) {
        PS_PRINT_ERR("memcopy_s error, destlen=%d, srclen=%d\n ", cali_data_len, copy_len);
    }

    complete(&g_st_cali_recv_done);
#endif

    return 0;
}

int32 ps_recv_uart_loop_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    int32 ret;
    uint16 rx_pkt_total_len;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if (rx_pkt_total_len <= UART_LOOP_MAX_PKT_LEN) {
        ret = uart_loop_test_recv_pkt(ps_core_d, buf_ptr, rx_pkt_total_len);
        if (ret < 0) {
            PS_PRINT_ERR("uart_loop_test_recv_pkt failed\n");
            return -EINVAL;
        }
    } else {
        PS_PRINT_ERR("uart loop test, recv pkt len is too large!\n");
        return -EINVAL;
    }

    return 0;
}

/*
 * Prototype    : ps_recv_debug_data
 * Description  : called by core when recive gnss data from device,
 *                memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_recv_debug_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    struct sk_buff *skb = NULL;
    int32 ret;
    uint16 rx_pkt_total_len;
    uint16 dbg_pkt_lenth = 0;
    uint8 ptr_index = 0;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if ((buf_ptr[START_SIGNAL_LAST_WORDS] == PACKET_START_SIGNAL)
        && (buf_ptr[PACKET_RX_FUNC_INDEX_LAST_WORDS] == PACKET_RX_FUNC_LAST_WORDS)) {
        /* get FrameLen 2 bytes */
        dbg_pkt_lenth = *(uint16 *)&buf_ptr[PACKET_FRAMELEN_INDEX];
        if ((dbg_pkt_lenth == rx_pkt_total_len) && (dbg_pkt_lenth == LAST_WORD_LEN) &&
            (buf_ptr[RPT_IND_INDEX_LAST_WORDS] == PACKET_RX_RPT_IND_LAST_WORDS)) {
            PS_PRINT_ERR("recv device last words!Faulttype=0x%x,FaultReason=0x%x,PC=0x%x,LR=0x%x\n",
                         *(uint32 *)&buf_ptr[FAULT_TYPE_INDEX_LAST_WORDS],
                         *(uint32 *)&buf_ptr[FAULT_REASON_INDEX_LAST_WORDS],
                         *(uint32 *)&buf_ptr[PC_INDEX_LAST_WORDS],
                         *(uint32 *)&buf_ptr[LR_INDEX_LAST_WORDS]);
            DECLARE_DFT_TRACE_KEY_INFO("bfgx_device_panic", OAL_DFT_TRACE_EXCEP);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            if (bfgx_panic_ssi_dump) {
                ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL);
            }
#endif
            plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, BFGX_LASTWORD_PANIC);
        } else {
            /* buf maybe less than log header len */
            if (rx_pkt_total_len > PACKET_HEADER_LEN) {
                PS_PRINT_WARNING("recv wrong last words,[%x %x]\n",
                                 *(uint32 *)&buf_ptr[START_SIGNAL_LAST_WORDS],
                                 *(uint32 *)&buf_ptr[PACKET_FRAMELEN_INDEX]);
            } else {
                PS_PRINT_WARNING("recv wrong last words,len less than head\n");
                for (ptr_index = 0; ptr_index < rx_pkt_total_len; ptr_index++) {
                    PS_PRINT_WARNING("lastwords[%d]=%x\n", ptr_index, buf_ptr[ptr_index]);
                }
            }
        }
    }

    if (atomic_read(&ps_core_d->dbg_func_has_open)) {
        if (rx_pkt_total_len <= DBG_RX_MAX_FRAME) {
            ret = ps_push_skb_debug_queue(ps_core_d, buf_ptr, rx_pkt_total_len, RX_DBG_QUEUE);
            if (ret < 0) {
                PS_PRINT_ERR("push debug data to skb queue failed\n");
                return -EINVAL;
            }

            PS_PRINT_DBG("rx_dbg_seq.qlen = %d\n", ps_core_d->rx_dbg_seq.qlen);
            wake_up_interruptible(&ps_core_d->rx_dbg_wait);
            if (ps_core_d->rx_dbg_seq.qlen > RX_DBG_QUE_MAX_NUM) {
                PS_PRINT_INFO("rx dbg queue too large!");
                /* if sdt data is large，remove the head skb data */
                skb = ps_skb_dequeue(ps_core_d, RX_DBG_QUEUE);
                kfree_skb(skb);
            }
        }
    }

    return 0;
}

/*
 * Prototype    : ps_store_rx_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                  memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_store_rx_sepreated_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr, uint8 subsys)
{
    uint16 rx_current_pkt_len;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys index %d outof range\n", subsys);
        return -EINVAL;
    }

    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    rx_current_pkt_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    pst_sepreted_data->rx_pkt_len = rx_current_pkt_len;

    if (likely((uint32)(pst_sepreted_data->rx_buf_all_len + rx_current_pkt_len) <= bfgx_rx_max_frame[subsys])) {
        if (likely(pst_sepreted_data->rx_buf_ptr != NULL)) {
            if (unlikely(memcpy_s(pst_sepreted_data->rx_buf_ptr, bfgx_rx_max_frame[subsys] -
                                  pst_sepreted_data->rx_buf_all_len, buf_ptr, rx_current_pkt_len) != EOK)) {
                PS_PRINT_ERR("dis space is not enough\n ");
                return RX_PACKET_ERR;
            }
            pst_sepreted_data->rx_buf_all_len += rx_current_pkt_len;
            pst_sepreted_data->rx_buf_ptr += rx_current_pkt_len;
            return RX_PACKET_CORRECT;
        } else {
            PS_PRINT_ERR("sepreted rx_buf_ptr is NULL\n");
            return RX_PACKET_ERR;
        }
    } else {
        PS_PRINT_ERR("rx_current_pkt_len=%d,rx_pkt_total_len=%d,rx_buf_all_len=%d,subsys=%d\n",
                     pst_sepreted_data->rx_pkt_len,
                     ps_core_d->rx_pkt_total_len,
                     pst_sepreted_data->rx_buf_all_len,
                     subsys);
        return RX_PACKET_ERR;
    }
}

/*
 * Prototype    : ps_recv_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                  memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_recv_sepreated_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr, uint8 subsys, uint8 sepreted_type)
{
    uint16 len;
    int32 ret;
    const uint32 ul_max_dump_len = 32;
    struct pm_drv_data *pm_data = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    int32 seq_correct = SEPRETED_RX_PKT_SEQ_ERROR;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        PS_PRINT_ERR("subsys index %d outof range\n", subsys);
        return -EINVAL;
    }

    if (subsys == BFGX_BT) {
        PS_PRINT_ERR("BT not use sepreted rx, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (sepreted_type >= RX_SEQ_BUTT) {
        PS_PRINT_ERR("septeted_type %d outof range\n", sepreted_type);
        return -EINVAL;
    }

    pm_data = pm_get_drvdata();
    ;
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is null\n");
        return -EINVAL;
    }

    if (uart_loop_test_cfg.uart_loop_enable) {
        PS_PRINT_DBG("recv UART LOOP data\n");
        ps_recv_uart_loop_data(ps_core_d, buf_ptr);
        return 0;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    pst_sepreted_data = &pst_bfgx_data->sepreted_rx;

    spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    if (POWER_STATE_OPEN != atomic_read(&pst_bfgx_data->subsys_state)) {
        len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
        PS_PRINT_WARNING("%s has closed, no need to recv data, len is %d\n", bfgx_subsys_name[subsys], len);
        oal_print_hex_dump(buf_ptr, (len < ul_max_dump_len ? len : ul_max_dump_len), HEX_DUMP_GROUP_SIZE, "rec data :");
        spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
        return 0;
    }

    if (subsys == BFGX_GNSS) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    }

    /* 检查分包序列的正确性 */
    switch (sepreted_type) {
        case RX_SEQ_START:
            if ((pst_sepreted_data->rx_prev_seq == RX_SEQ_NULL) || (pst_sepreted_data->rx_prev_seq == RX_SEQ_LAST)) {
                pst_sepreted_data->rx_prev_seq = RX_SEQ_START;
                seq_correct = SEPRETED_RX_PKT_SEQ_CORRECT;
            }
            break;

        case RX_SEQ_INT:
            if ((pst_sepreted_data->rx_prev_seq == RX_SEQ_START) || (pst_sepreted_data->rx_prev_seq == RX_SEQ_INT)) {
                pst_sepreted_data->rx_prev_seq = RX_SEQ_INT;
                seq_correct = SEPRETED_RX_PKT_SEQ_CORRECT;
            }
            break;

        case RX_SEQ_LAST:
            pst_sepreted_data->rx_prev_seq = RX_SEQ_LAST;
            seq_correct = SEPRETED_RX_PKT_SEQ_CORRECT;
            break;

        default:
            break;
    }

    if (seq_correct == SEPRETED_RX_PKT_SEQ_CORRECT) {
        /* 接收到的分包数据都要先拷贝到组包buffer中 */
        ret = ps_store_rx_sepreated_data(ps_core_d, buf_ptr, subsys);
        /* 当组包发生错误时，只有在收到LAST包的时候才重置组包buffer。因为只有收到LAST包，才能确保接收到的下一包数据的正确性。 */
        if ((ret == RX_PACKET_ERR) && (sepreted_type == RX_SEQ_LAST)) {
            PS_PRINT_ERR("%s rx data lenth err! give up this total pkt\n", bfgx_subsys_name[subsys]);
            pst_sepreted_data->rx_buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
            pst_sepreted_data->rx_buf_all_len = 0;
            spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
            return -EINVAL;
        }
    } else {
        PS_PRINT_ERR("%s rx seq is err! sepreted_type=%d, rx_prev_seq=%d\n",
                     bfgx_subsys_name[subsys], sepreted_type, pst_sepreted_data->rx_prev_seq);
        spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
        return -EINVAL;
    }

    /* 收到LAST包，说明组包完成，否则继续接收，直到收到完整的数据包，或者中途发生错误，丢弃该包。 */
    if (sepreted_type == RX_SEQ_LAST) {
        /* 如果已经缓存的数据到达了最大值，则新来的数据被丢弃 */
        if (pst_bfgx_data->rx_queue.qlen >= bfgx_rx_queue_max_num[subsys]) {
            PS_PRINT_WARNING("%s rx queue too large! qlen=%d\n",
                             bfgx_subsys_name[subsys], pst_bfgx_data->rx_queue.qlen);
            wake_up_interruptible(&pst_bfgx_data->rx_wait);
            pst_sepreted_data->rx_buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
            pst_sepreted_data->rx_buf_all_len = 0;
            spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
            return -EINVAL;
        }

        if (subsys == BFGX_GNSS) { /* gnss packet may large than 8KB, skb_alloc may fail, so seperate the packet to 4KB at most */
            ret = ps_push_skb_queue_gnss(ps_core_d, pst_sepreted_data->rx_buf_org_ptr,
                                         pst_sepreted_data->rx_buf_all_len, bfgx_rx_queue[subsys]);
        } else {
            ret = ps_push_skb_queue(ps_core_d, pst_sepreted_data->rx_buf_org_ptr,
                                    pst_sepreted_data->rx_buf_all_len, bfgx_rx_queue[subsys]);
        }
        pst_sepreted_data->rx_buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
        pst_sepreted_data->rx_buf_all_len = 0;
        if (ret < 0) {
            PS_PRINT_ERR("push %s rx data to skb failed!\n", bfgx_subsys_name[subsys]);
            spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
            return -EINVAL;
        }

        /* 现在skb中已经有正确完整的数据，唤醒等待数据的进程 */
        PS_PRINT_DBG("%s rx done! qlen=%d\n", bfgx_subsys_name[subsys], pst_bfgx_data->rx_queue.qlen);
        wake_up_interruptible(&pst_bfgx_data->rx_wait);
    }
    spin_unlock(&pst_sepreted_data->sepreted_rx_lock);

    return 0;
}

/*
 * Prototype    : ps_recv_no_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                  memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_recv_no_sepreated_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr, uint8 subsys)
{
    int32 ret;
    uint16 rx_pkt_total_len = 0;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys != BFGX_BT) {
        PS_PRINT_ERR("bfgx subsys %d use sepreted rx\n", subsys);
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    /* 如果已经缓存的数据到达了最大值，则新来的数据被丢弃 */
    if (pst_bfgx_data->rx_queue.qlen >= bfgx_rx_queue_max_num[subsys]) {
        PS_PRINT_WARNING("%s rx queue too large! qlen=%d\n",
                         bfgx_subsys_name[subsys], pst_bfgx_data->rx_queue.qlen);
        wake_up_interruptible(&pst_bfgx_data->rx_wait);
        return -EINVAL;
    }

    if (POWER_STATE_OPEN == atomic_read(&pst_bfgx_data->subsys_state)) {
        rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
        ret = ps_push_skb_queue(ps_core_d, buf_ptr, rx_pkt_total_len, bfgx_rx_queue[subsys]);
        if (ret < 0) {
            PS_PRINT_ERR("push %s rx data to skb failed!\n", bfgx_subsys_name[subsys]);
            return -EINVAL;
        }

        PS_PRINT_DBG("%s rx done! qlen=%d\n", bfgx_subsys_name[subsys], pst_bfgx_data->rx_queue.qlen);
        wake_up_interruptible(&pst_bfgx_data->rx_wait);
    }

    return 0;
}

/*
 * Prototype    : ps_decode_packet_func
 * Description  : called by core when recive data from device is
 *                  a complete packet
 *              : decode packet function,and push sk_buff head queue
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
int32 ps_decode_packet_func(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint8 *ptr;
    struct pm_drv_data *pm_data = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        PS_PRINT_ERR("buf_ptr is NULL\n");
        return -EINVAL;
    }

    pm_data = pm_get_drvdata();
    ;
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is null\n");
        return -EINVAL;
    }

    get_exception_info_reference(&pst_exception_data);
    if (NULL != pst_exception_data) {
        if (1 == pst_exception_data->debug_beat_flag) {
            atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_RECV_BEAT_INFO);
        }
    }

    ptr = buf_ptr + sizeof(struct ps_packet_head);

    /* if data comes from dev which is not SYS_INF_DEV_AGREE_HOST_SLP &
       not gnss data & not oml data, we should re-monitor uart transfer
     */
    if (ps_core_d->rx_pkt_type == SYS_MSG) {
        if ((*ptr != SYS_INF_DEV_AGREE_HOST_SLP) && (*ptr < SYS_INF_HEARTBEAT_CMD_BASE)) {
            if (!bfgx_other_subsys_all_shutdown(BFGX_GNSS)) {
                mod_timer(&pm_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
                pm_data->bfg_timer_mod_cnt++;
            }
        }
    } else if ((GNSS_First_MSG != ps_core_d->rx_pkt_type) &&
               (GNSS_Common_MSG != ps_core_d->rx_pkt_type) &&
               (GNSS_Last_MSG != ps_core_d->rx_pkt_type) &&
               (ps_core_d->rx_pkt_type != OML_MSG) &&
               (!ir_only_mode)) {
        mod_timer(&pm_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
        pm_data->bfg_timer_mod_cnt++;

        pm_data->gnss_votesleep_check_cnt = 0;
        pm_data->rx_pkt_gnss_pre = 0;
    }

    /* if some data comes from device which is the normal protocol msg,
     * mark it as "received data before device ack".
     */
    if (unlikely(ps_core_d->ps_pm->bfgx_dev_state_get() == BFGX_SLEEP) &&
        (ps_core_d->rx_pkt_type != OML_MSG) &&
        (ps_core_d->rx_pkt_type != MEM_DUMP_SIZE) &&
        (ps_core_d->rx_pkt_type != MEM_DUMP) &&
        (*ptr != SYS_INF_DEV_AGREE_HOST_SLP) &&
        (*ptr < SYS_INF_HEARTBEAT_CMD_BASE)) {
        ps_core_d->ps_pm->pm_priv_data->rcvdata_bef_devack_flag = 1;
        PS_PRINT_INFO("recv data before dev_ack, type=[0x%x]\n", ps_core_d->rx_pkt_type);
    }

    if (uc_wakeup_src_debug) {
        PS_PRINT_INFO("dev wakeup host pkt type is [%d]\n", ps_core_d->rx_pkt_type);
        uc_wakeup_src_debug = 0;
    }

    switch (ps_core_d->rx_pkt_type) {
        case SYS_MSG:
            PS_PRINT_DBG("recv system data\n");
            ps_core_d->rx_pkt_sys++;
            ps_exe_sys_func(ps_core_d, ptr);
            break;

        case BT_MSG:
            PS_PRINT_DBG("recv BT data\n");
            ps_core_d->rx_pkt_num[BFGX_BT]++;
            oal_wake_lock_timeout(&pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);
            ps_recv_no_sepreated_data(ps_core_d, ptr, BFGX_BT);
            break;

        case FM_FIRST_MSG:
            PS_PRINT_DBG("into ->fm-START\n");
            ps_core_d->rx_pkt_num[BFGX_FM]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_FM, RX_SEQ_START);
            break;

        case FM_COMMON_MSG:
            PS_PRINT_DBG("into ->fm-INT\n");
            ps_core_d->rx_pkt_num[BFGX_FM]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_FM, RX_SEQ_INT);
            break;

        case FM_LAST_MSG:
            PS_PRINT_DBG("recv FM data\n");
            ps_core_d->rx_pkt_num[BFGX_FM]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_FM, RX_SEQ_LAST);
            break;

        case GNSS_First_MSG:
            PS_PRINT_DBG("into ->gnss-START\n");
            ps_core_d->rx_pkt_num[BFGX_GNSS]++;
            oal_wake_lock_timeout(&pm_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_GNSS, RX_SEQ_START);
            break;

        case GNSS_Common_MSG:
            PS_PRINT_DBG("into ->gnss-INT\n");
            ps_core_d->rx_pkt_num[BFGX_GNSS]++;
            oal_wake_lock_timeout(&pm_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_GNSS, RX_SEQ_INT);
            break;

        case GNSS_Last_MSG:
            PS_PRINT_DBG("recv GNSS data\n");
            ps_core_d->rx_pkt_num[BFGX_GNSS]++;
            oal_wake_lock_timeout(&pm_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_GNSS, RX_SEQ_LAST);
            break;

        case OML_MSG:
            PS_PRINT_DBG("recv debug data\n");
            ps_core_d->rx_pkt_oml++;
            ps_recv_debug_data(ps_core_d, ptr);
            break;

        case NFC_First_MSG:
            PS_PRINT_DBG("into ->nfc-START\n");
            ps_core_d->rx_pkt_num[BFGX_NFC]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_NFC, RX_SEQ_START);
            break;

        case NFC_Common_MSG:
            PS_PRINT_DBG("into ->nfc-INT\n");
            ps_core_d->rx_pkt_num[BFGX_NFC]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_NFC, RX_SEQ_INT);
            break;

        case NFC_Last_MSG:
            PS_PRINT_DBG("recv NFC data\n");
            ps_core_d->rx_pkt_num[BFGX_NFC]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_NFC, RX_SEQ_LAST);
            break;

        case IR_FIRST_MSG:
            PS_PRINT_DBG("into ->ir-START\n");
            ps_core_d->rx_pkt_num[BFGX_IR]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_IR, RX_SEQ_START);
            break;

        case IR_COMMON_MSG:
            PS_PRINT_DBG("into ->ir-INT\n");
            ps_core_d->rx_pkt_num[BFGX_IR]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_IR, RX_SEQ_INT);
            break;

        case IR_LAST_MSG:
            PS_PRINT_DBG("recv IR data\n");
            ps_core_d->rx_pkt_num[BFGX_IR]++;
            ps_recv_sepreated_data(ps_core_d, ptr, BFGX_IR, RX_SEQ_LAST);
            break;

        case MEM_DUMP_SIZE:
            PS_PRINT_DBG("recv MEM DUMP SIZE data\n");
            ps_recv_mem_dump_size_data(ps_core_d, ptr);
            break;

        case MEM_DUMP:
            PS_PRINT_DBG("recv MEM DUMP data\n");
            ps_recv_mem_dump_data(ps_core_d, ptr, SUBSYS_BFGX);
            break;

        case WIFI_MEM_DUMP:
            PS_PRINT_DBG("recv MEM DUMP data\n");
            ps_recv_mem_dump_data(ps_core_d, ptr, SUBSYS_WIFI);
            break;

        case BFGX_CALI_MSG:
            PS_PRINT_DBG("recv BFGX CALI data\n");
            ps_recv_bfgx_cali_data(ps_core_d, ptr);
            break;

        default:
            break;
    }

    return 0;
}

static int32 ps_check_chr_packet(struct ps_core_s *ps_core_d, uint8* buf_ptr)
{
    /* if type is sys packet,and packet lenth large sys pkt lenth,is err packet */
    if ((ps_core_d->rx_pkt_type == SYS_MSG) &&
        (ps_core_d->rx_pkt_total_len > SYS_TOTAL_PACKET_LENTH)) {
        if ((ps_core_d->rx_pkt_total_len == SYS_TOTAL_PACKET_LENTH + DEV_SEND_CHR_ERRNO_LEN) ||
            (buf_ptr[sizeof(struct ps_packet_head)] == SYS_INF_CHR_ERRNO_REPORT)) {
            PS_PRINT_DBG("Host recv CHR errno!");
        } else {
            PS_PRINT_ERR("the pkt type and len err: %x, %x\n", ps_core_d->rx_pkt_type,
                         ps_core_d->rx_pkt_total_len);
            print_uart_decode_param();
            return RX_PACKET_ERR;
        }
    }
    return 0;
}

static void dmd_report_uart_pkt_err(void)
{
    static uint32 uart_pkt_err_count = 0;
    static uint32 uart_pkt_err_start_time = 0;
    static uint32 uart_pkt_err_end_time = 0;

    uart_pkt_err_count++;
    if (uart_pkt_err_count == 1) {
         uart_pkt_err_start_time = jiffies;
         uart_pkt_err_end_time = uart_pkt_err_start_time + 60*HZ;
    }
    if (uart_pkt_err_count == UART_DMD_MAX_PKT_ERR_CNT) {
         if (jiffies <= uart_pkt_err_end_time) {
             hw_1102a_bt_dsm_client_notify(DSM_1102A_UART_PKT_ERR, "%s: report dmd for uart pkt err\n", __FUNCTION__);
         }
         uart_pkt_err_start_time = 0;
         uart_pkt_err_count = 0;
    }
}

/*
 * Prototype    : ps_check_packet_head
 * Description  : check recived curr packet is or not a complete packet
 * input        : buf_ptr -> ptr of recived data buf
 *                count   -> size of recived data buf
 * output       : return 0 -> this packet is a complete packet
 *                return 1 -> this packet has not a complete packet,need continue recv
 *                return -1-> this packst is a err packet,del it
 */
int32 ps_check_packet_head(struct ps_core_s *ps_core_d, uint8 *buf_ptr, int32 count)
{
    uint8 *ptr;
    uint16 len;
    uint16 lenbak;

    PS_PRINT_FUNCTION_NAME;

    if ((ps_core_d == NULL) || (buf_ptr == NULL)) {
        PS_PRINT_ERR("ps_core_d or buf_ptr is NULL\n");
        return -EINVAL;
    }

    ptr = buf_ptr;
    if (*ptr == PACKET_START_SIGNAL) { /* if count less then sys packet lenth and continue */
        if (count < SYS_TOTAL_PACKET_LENTH) {
            return RX_PACKET_CONTINUE;
        }
        ptr++;
        ps_core_d->rx_pkt_type = *ptr;

        /* check packet type */
        if (MSG_BUTT > ps_core_d->rx_pkt_type) {
            ptr++;
            len = *ptr;
            ptr++;
            lenbak = *ptr;
            lenbak = lenbak * 0x100;
            len = len + lenbak;
            ps_core_d->rx_pkt_total_len = len;

            /* check packet lenth less then sys packet lenth */
            if (ps_core_d->rx_pkt_total_len < SYS_TOTAL_PACKET_LENTH) {
                PS_PRINT_ERR("the pkt len is ERR: %x\n", ps_core_d->rx_pkt_total_len);
                print_uart_decode_param();
                return RX_PACKET_ERR;
            }

            /* check packet lenth large than buf total lenth */
            if (ps_core_d->rx_pkt_total_len > PUBLIC_BUF_MAX) {
                PS_PRINT_ERR("the pkt len is too large: %x\n", ps_core_d->rx_pkt_total_len);
                print_uart_decode_param();
                ps_core_d->rx_pkt_total_len = 0;
                return RX_PACKET_ERR;
            }

            /* check all service frame length */
            switch (ps_core_d->rx_pkt_type) {
                case BT_MSG:
                    if (ps_core_d->rx_pkt_total_len > BT_RX_MAX_FRAME) {
                        PS_PRINT_ERR("type=%d, the pkt len is ERR: 0x%x\n",
                                     ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len);
                        print_uart_decode_param();
                        return RX_PACKET_ERR;
                    }
                    break;
                case GNSS_First_MSG:
                case GNSS_Common_MSG:
                case GNSS_Last_MSG:
                case FM_FIRST_MSG:
                case FM_COMMON_MSG:
                case FM_LAST_MSG:
                case IR_FIRST_MSG:
                case IR_COMMON_MSG:
                case IR_LAST_MSG:
                case NFC_First_MSG:
                case NFC_Common_MSG:
                case NFC_Last_MSG:
                case OML_MSG:
                    if (ir_only_mode) {
                        break;  // 单红外消息不分包，不需要分包检查
                    }
                    if ((buf_ptr[PACKET_OFFSET_HEAD_INDEX] == PACKET_START_SIGNAL) &&
                        (buf_ptr[PACKET_OFFSET_HEAD_NEXT_INDEX] == PACKET_RX_FUNC_LAST_WORDS)) {
                        if (ps_core_d->rx_pkt_total_len > MAX_LAST_WORD_FRAME_LEN) {
                            PS_PRINT_ERR("type=%d, the pkt len is ERR: 0x%x,(max frame len is:0x%x)\n",
                                         ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len, MAX_LAST_WORD_FRAME_LEN);
                            print_uart_decode_param();
                            return RX_PACKET_ERR;
                        }
                    } else if (ps_core_d->rx_pkt_total_len > MAX_NOBT_FRAME_LEN) {
                        PS_PRINT_ERR("type=%d, the pkt len is ERR: 0x%x,(max frame len is:0x%x)\n",
                                     ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len, MAX_NOBT_FRAME_LEN);
                        print_uart_decode_param();
                        return RX_PACKET_ERR;
                    }
                    break;
                default:
                    break;
            }

            /* check whether the recv packet is CHR packet */
            if (ps_check_chr_packet(ps_core_d, buf_ptr) != 0) {
                return RX_PACKET_ERR;
            }

            /* if pkt lenth less then sys packet lenth and continue */
            if (ps_core_d->rx_pkt_total_len > count) { /* the packet is breaked, need continue rx */
                return RX_PACKET_CONTINUE;
            }
            ptr = buf_ptr + len - sizeof(struct ps_packet_end);
            /* check the last byte yes or not 7e */
            if (*ptr == PACKET_END_SIGNAL) { /* the packet is a good packet */
                return RX_PACKET_CORRECT;
            } else { /* the packet is a err packet */
                PS_PRINT_WARNING("the pkt end is ERR: %x\n", *ptr);
                print_uart_decode_param();
                return RX_PACKET_ERR;
            }
        } else {
            PS_PRINT_ERR("the pkt type is ERR: %x\n", ps_core_d->rx_pkt_type);
            print_uart_decode_param();
            return RX_PACKET_ERR;
        }
    }
    dmd_report_uart_pkt_err();
    PS_PRINT_WARNING("the pkt head is ERR: %x, count=[%d], buf_ptr=[%p]\n", *ptr, count, buf_ptr);
    print_uart_decode_param();

    return RX_PACKET_ERR;
}

void dump_uart_rx_buf(void)
{
#define UART_DUMP_RX_BUF_LENGTH 16
    uint32 i = 0;
    struct ps_core_s *ps_core_d = NULL;
    uint32 ul_dump_len;
    ps_get_core_reference(&ps_core_d);

    /* uart在接收数据时，不能flaush buffer */
    spin_lock(&ps_core_d->rx_lock);

    if (ps_core_d->rx_have_recv_pkt_len > 0) {
        /* have decode all public buf data, reset ptr and lenth */
        PS_PRINT_WARNING("uart rx buf has data, rx_have_recv_pkt_len=%d\n", ps_core_d->rx_have_recv_pkt_len);
        PS_PRINT_WARNING("uart rx buf has data, rx_have_del_public_len=%d\n", ps_core_d->rx_have_del_public_len);
        PS_PRINT_WARNING("uart rx buf has data, rx_decode_public_buf_ptr=%p\n", ps_core_d->rx_decode_public_buf_ptr);
        PS_PRINT_WARNING("uart rx buf has data, rx_public_buf_org_ptr=%p\n", ps_core_d->rx_public_buf_org_ptr);

        ul_dump_len = ps_core_d->rx_have_recv_pkt_len > UART_DUMP_RX_BUF_LENGTH ?
                      UART_DUMP_RX_BUF_LENGTH : ps_core_d->rx_have_recv_pkt_len;
        for (i = 0; i < ul_dump_len; i++) {
            PS_PRINT_WARNING("uart rx buf has data, data[%d]=0x%x\n", i, *(ps_core_d->rx_decode_public_buf_ptr + i));
        }
    }

    spin_unlock(&ps_core_d->rx_lock);
}


void reset_uart_rx_buf(void)
{
    uint32 i = 0;
    struct ps_core_s *ps_core_d = NULL;
    ps_get_core_reference(&ps_core_d);

    /* uart在接收数据时，不能flaush buffer */
    spin_lock(&ps_core_d->rx_lock);

    if (ps_core_d->rx_have_recv_pkt_len > 0) {
        /* have decode all public buf data, reset ptr and lenth */
        PS_PRINT_WARNING("uart rx buf has data, rx_have_recv_pkt_len=%d\n", ps_core_d->rx_have_recv_pkt_len);
        PS_PRINT_WARNING("uart rx buf has data, rx_have_del_public_len=%d\n", ps_core_d->rx_have_del_public_len);
        PS_PRINT_WARNING("uart rx buf has data, rx_decode_public_buf_ptr=%p\n", ps_core_d->rx_decode_public_buf_ptr);
        PS_PRINT_WARNING("uart rx buf has data, rx_public_buf_org_ptr=%p\n", ps_core_d->rx_public_buf_org_ptr);
        for (i = 0; i < ps_core_d->rx_have_recv_pkt_len; i++) {
            PS_PRINT_WARNING("uart rx buf has data, data[%d]=0x%x\n", i, *(ps_core_d->rx_decode_public_buf_ptr + i));
        }
        reset_cnt++;
        PS_PRINT_WARNING("reset uart rx buf, reset cnt=%d\n", reset_cnt);
        ps_core_d->rx_have_del_public_len = 0;
        ps_core_d->rx_have_recv_pkt_len = 0;
        ps_core_d->rx_decode_public_buf_ptr = ps_core_d->rx_public_buf_org_ptr;
    }

    spin_unlock(&ps_core_d->rx_lock);
}

static int32 core_recv_prepare(void *disc_data, const uint8 *data, int32 count)
{
    PS_PRINT_FUNCTION_NAME;

    if (disc_data == NULL) {
        PS_PRINT_ERR(" disc_data is null \n");
        return 0;
    }

    if (unlikely(data == NULL)) {
        PS_PRINT_ERR(" received null from TTY \n");
        return 0;
    }

    if (count < 0) {
        PS_PRINT_ERR(" received count from TTY err\n");
        return 0;
    }

    return 1;
}

static void ps_clear_rx_buf(struct ps_core_s *ps_core_d)
{
    ps_core_d->rx_have_del_public_len = 0;
    ps_core_d->rx_have_recv_pkt_len = 0;
    ps_core_d->rx_decode_public_buf_ptr = ps_core_d->rx_public_buf_org_ptr;
}

/*
 * Prototype    : ps_core_recv
 * Description  : PS's core receive function.Decodes received RAW data and
 *                  forwards to corresponding client drivers (Bluetooth)
 *                  or hal stack(FM,GNSS).
 * input        : data -> recive data ptr   from UART TTY
 *                count -> recive data count from UART TTY
 * output       : not
 */
int32 ps_core_recv(void *disc_data, const uint8 *data, int32 count)
{
    struct ps_core_s *ps_core_d;
    uint8 *ptr;
    int32 count1 = 0;
    int32 ret = 0;

    PS_PRINT_FUNCTION_NAME;

    ret = core_recv_prepare(disc_data, data, count);
    if (!ret) {
        return ret;
    }

    ps_core_d = (struct ps_core_s *)disc_data;
    ps_core_d->rx_decode_tty_ptr = (uint8 *)data;
    while (count) {                            /* if curr packet is breaked packet,and first memcpy */
        if (ps_core_d->rx_have_recv_pkt_len) { /* this is a breaked packet */
            ptr = ps_core_d->rx_public_buf_org_ptr;
            ptr = ptr + ps_core_d->rx_have_recv_pkt_len; /* find public buf ptr */
            /* copy all uart buf data to public buf */
            if (unlikely(memcpy_s(ptr, PUBLIC_BUF_MAX - ps_core_d->rx_have_recv_pkt_len,
                                  ps_core_d->rx_decode_tty_ptr, count) != EOK)) {
                ps_clear_rx_buf(ps_core_d);
                PS_PRINT_WARNING("received count too large! , memcopy_s error, destlen=%d, srclen=%d\n ",
                                 PUBLIC_BUF_MAX - ps_core_d->rx_have_recv_pkt_len, count);
                continue;
            }
            ps_core_d->rx_have_recv_pkt_len = ps_core_d->rx_have_recv_pkt_len + count;
            count = 0;
            /* if count less then sys packet lenth and continue */
            if (ps_core_d->rx_have_recv_pkt_len < SYS_TOTAL_PACKET_LENTH) { /* not decode,and continue rxd data */
                continue;
            }
            /* decode public buf data */
            while (ps_core_d->rx_have_del_public_len < ps_core_d->rx_have_recv_pkt_len) { /* find needs decode data in buf */
                count1 = ps_core_d->rx_have_recv_pkt_len - ps_core_d->rx_have_del_public_len;
                /* check curr data */
                ret = ps_check_packet_head(ps_core_d, ps_core_d->rx_decode_public_buf_ptr, count1);
                if (ret == RX_PACKET_CORRECT) { /* this packet is correct, and push it to proto buf */
                    ps_decode_packet_func(ps_core_d, ps_core_d->rx_decode_public_buf_ptr);
                    ps_core_d->rx_have_del_public_len += ps_core_d->rx_pkt_total_len;
                    ps_core_d->rx_decode_public_buf_ptr += ps_core_d->rx_pkt_total_len;
                    ps_core_d->rx_pkt_total_len = 0;
                    continue;
                } else if (ret == RX_PACKET_CONTINUE) {
                    /* copy breaked pkt to buf from start */
                    if (unlikely(memmove_s(ps_core_d->rx_public_buf_org_ptr, PUBLIC_BUF_MAX,
                                           ps_core_d->rx_decode_public_buf_ptr, count1) != EOK)) {
                        ps_clear_rx_buf(ps_core_d);
                        PS_PRINT_ERR("memmove_s error, destlen=%d, srclen=%d\n ", PUBLIC_BUF_MAX, count1);
                        return 0;
                    }
                    ps_core_d->rx_have_recv_pkt_len = count1;
                    ps_core_d->rx_decode_public_buf_ptr = ps_core_d->rx_public_buf_org_ptr;
                    ps_core_d->rx_have_del_public_len = 0;
                    break;
                } else { /* this packet is err! remove a byte and continue decode */
                    ps_core_d->rx_decode_public_buf_ptr++;
                    ps_core_d->rx_have_del_public_len++;
                }
            }
            if (ps_core_d->rx_have_del_public_len == ps_core_d->rx_have_recv_pkt_len) { /* have decode all public buf data, reset ptr and lenth */
                ps_clear_rx_buf(ps_core_d);
            }
        } else { /* if not have recv data, direct decode data in uart buf */
            ret = ps_check_packet_head(ps_core_d, ps_core_d->rx_decode_tty_ptr, count);
            if (ret == RX_PACKET_CORRECT) { /* this packet is correct, and push it to proto buf */
                ps_decode_packet_func(ps_core_d, ps_core_d->rx_decode_tty_ptr);
                ps_core_d->rx_decode_tty_ptr = ps_core_d->rx_decode_tty_ptr + ps_core_d->rx_pkt_total_len;
                count = count - ps_core_d->rx_pkt_total_len;
                ps_core_d->rx_pkt_total_len = 0;
                continue;
            } else if (ret == RX_PACKET_CONTINUE) { /* this packet is not completed,need continue rx */
                ptr = ps_core_d->rx_public_buf_org_ptr;
                /* copy data to public buf,and continue rx */
                if (unlikely(memcpy_s(ptr, PUBLIC_BUF_MAX, ps_core_d->rx_decode_tty_ptr, count) != EOK)) {
                    ps_clear_rx_buf(ps_core_d);
                    PS_PRINT_ERR("memcpy_s error, destlen=%d, srclen=%d\n ", PUBLIC_BUF_MAX, count);
                    return 0;
                }
                ps_core_d->rx_have_recv_pkt_len = count;
                count = 0;
                continue;
            } else { /* this packet is err! remove a byte and continue decode */
                ps_core_d->rx_decode_tty_ptr++;
                count--;
            }
        }
    }
    return 0;
}

/*
 * Prototype    : ps_bt_enqueue
 * Description  : push skb data to skb head queue from tail
 * input        : ps_core_d, skb
 * output       : not
 */
int32 ps_skb_enqueue(struct ps_core_s *ps_core_d, struct sk_buff *skb, uint8 type)
{
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR(" ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (unlikely(skb == NULL)) {
        PS_PRINT_ERR(" skb is NULL\n");
        return -EINVAL;
    }

    switch (type) {
        case TX_URGENT_QUEUE:
            skb_queue_tail(&ps_core_d->tx_urgent_seq, skb);
            break;
        case TX_HIGH_QUEUE:
            skb_queue_tail(&ps_core_d->tx_high_seq, skb);
            break;
        case TX_LOW_QUEUE:
            skb_queue_tail(&ps_core_d->tx_low_seq, skb);
            break;
        case RX_GNSS_QUEUE:
            skb_queue_tail(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue, skb);
            break;
        case RX_FM_QUEUE:
            skb_queue_tail(&ps_core_d->bfgx_info[BFGX_FM].rx_queue, skb);
            break;
        case RX_BT_QUEUE:
            skb_queue_tail(&ps_core_d->bfgx_info[BFGX_BT].rx_queue, skb);
            break;
        case RX_DBG_QUEUE:
            skb_queue_tail(&ps_core_d->rx_dbg_seq, skb);
            break;
        case RX_NFC_QUEUE:
            skb_queue_tail(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue, skb);
            break;
        case RX_IR_QUEUE:
            skb_queue_tail(&ps_core_d->bfgx_info[BFGX_IR].rx_queue, skb);
            break;
        default:
            PS_PRINT_ERR("queue type is error, type=%d\n", type);
            break;
    }

    return 0;
}

/*
 * Prototype    : ps_skb_dequeue
 * Description  : internal de sk_buff seq function. return top of txq.
 */
struct sk_buff *ps_skb_dequeue(struct ps_core_s *ps_core_d, uint8 type)
{
    struct sk_buff *curr_skb = NULL;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return curr_skb;
    }

    switch (type) {
        case TX_URGENT_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->tx_urgent_seq);
            break;
        case TX_HIGH_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->tx_high_seq);
            break;
        case TX_LOW_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->tx_low_seq);
            break;
        case RX_DBG_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->rx_dbg_seq);
            break;
        case RX_BT_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->bfgx_info[BFGX_BT].rx_queue);
            break;
        case RX_FM_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->bfgx_info[BFGX_FM].rx_queue);
            break;
        case RX_GNSS_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue);
            break;
        case RX_IR_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->bfgx_info[BFGX_IR].rx_queue);
            break;
        case RX_NFC_QUEUE:
            curr_skb = skb_dequeue(&ps_core_d->bfgx_info[BFGX_NFC].rx_queue);
            break;
        default:
            PS_PRINT_ERR("queue type is error, type=%d\n", type);
            break;
    }

    return curr_skb;
}


static struct sk_buff *ps_skb_dequeue_core(struct ps_core_s *ps_core_d, uint32 *dequeue_init_flag)
{
    struct sk_buff *skb;
    static uint8 tx_high_times;

    skb = ps_skb_dequeue(ps_core_d, TX_URGENT_QUEUE);
    if (skb != NULL) {
        return skb;
    }

    if (*dequeue_init_flag) {
        tx_high_times = 0;
        *dequeue_init_flag = 0;
    }

    if (tx_high_times < BT_TX_TIMES) { /* first scan high queue, if not and read low queue skb */
        if ((skb = ps_skb_dequeue(ps_core_d, TX_HIGH_QUEUE))) {
            tx_high_times++;
        } else { /* high queue no skb, direct read low queue skb */
            skb = ps_skb_dequeue(ps_core_d, TX_LOW_QUEUE);
            tx_high_times = 0;
        }
    } else { /* first scan low queue, if not and read high queue skb */
        if ((skb = ps_skb_dequeue(ps_core_d, TX_LOW_QUEUE))) {
            tx_high_times = 0;
        } else { /* low queue no skb, direct read high queue skb */
            skb = ps_skb_dequeue(ps_core_d, TX_HIGH_QUEUE);
            tx_high_times++;
        }
    }
    return skb;
}

/**
 * Prototype    : ps_core_tx_attemper_etc
 * Description  : adj tx skb buf data to tty, seq BT and FM or GNSS.
 */
int32 ps_core_tx_attemper(struct ps_core_s *ps_core_d)
{
    struct sk_buff *skb = NULL;
    int32 len = 0;
    uint8 uart_state = UART_NOT_READY;
    struct pm_drv_data *pm_data = NULL;
    uint64 flags;
    uint32 dequeue_init_flag = 1;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }
    pm_data = pm_get_drvdata();
    if (unlikely(pm_data == NULL)) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -EFAULT;
    }

    PS_PRINT_DBG("enter tx work\n");

    while (1) {
        spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
        uart_state = ps_core_d->ps_pm->bfgx_uart_state_get();
        if (uart_state != UART_READY) {
            /* 非ready状态仅从urgent队列发送 */
            skb = ps_skb_dequeue(ps_core_d, TX_URGENT_QUEUE);
        } else {
            /* ready状态遍历所有队列 */
            skb = ps_skb_dequeue_core(ps_core_d, &dequeue_init_flag);
        }
        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

        if (skb == NULL) { /* no skb and exit */
            break;
        }

        if ((ps_core_d->tty != NULL)) {
            /* enable wake-up from TTY */
            set_bit(TTY_DO_WRITE_WAKEUP, &ps_core_d->tty->flags);
        }
        /* tx skb data to uart driver until the skb is NULL */
        if (skb->len) {
            PS_PRINT_DBG("use tty start to send data,skb->len=[%d]\n", skb->len);
        }

        while (skb->len) {
            /* len is have tx to uart byte number */
            len = ps_write_tty(ps_core_d, skb->data, skb->len);
            if (len < 0) {
                PS_PRINT_ERR("tty have not opened!\n");
                kfree_skb(skb);
                return 0;
            }
            skb_pull(skb, len);
            /* if skb->len = len as expected, skb->len=0 */
            if (skb->len) {
                msleep(1);
            }
        }
        kfree_skb(skb);
    }
    return 0;
}

/*
 * Prototype    : ps_core_tx_work
 * Description  : adj tx buf data to tty seq, BT and FM or GNSS.
 * input        : ps_core_d
 * output       : not
 */
void ps_core_tx_work(struct work_struct *work)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return;
    }

    /* into tx skb buff queue */
    ps_core_tx_attemper(ps_core_d);

    return;
}

/*
 * Prototype    : ps_add_packet_head
 * Description  : add packet head to recv buf from hal or bt driver.
 * input        : buf  -> ptr of buf
 *                type -> packet type，example bt,fm,or gnss
 *                lenth-> packet length
 * output       : not
 */
int32 ps_add_packet_head(uint8 *buf, uint8 type, uint16 lenth)
{
    int8 *ptr;
    uint16 len;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    ptr = buf;
    *ptr = PACKET_START_SIGNAL;
    ptr++;
    *ptr = type;
    len = lenth;
    ptr++;
    *ptr = LEN_LOW_BYTE(len);
    ptr++;
    *ptr = LEN_HIGH_BYTE(len);

    ptr = buf;
    ptr = ptr + lenth;
    ptr = ptr - sizeof(struct ps_packet_end);
    *ptr = PACKET_END_SIGNAL;

    return 0;
}

/*
 * Prototype    : ps_set_sys_packet
 * Description  : set sys packet head to buf
 * input        : buf  -> ptr of buf
 *                type -> packet type，example bt,fm,or gnss
 *                lenth-> packet length
 * output       : not
 */
int32 ps_set_sys_packet(uint8 *buf, uint8 type, uint8 content)
{
    int8 *ptr;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    ptr = buf;
    *ptr = PACKET_START_SIGNAL;
    ptr++;
    *ptr = type;
    ptr++;
    *ptr = SYS_TOTAL_PACKET_LENTH;
    ptr++;
    *ptr = 0;
    ptr++;
    *ptr = content;
    ptr++;
    *ptr = PACKET_END_SIGNAL;

    return 0;
}

/*
 * 函 数 名  : ps_set_sys_variable_length_packet
 * 功能描述  : 封装系统可变长消息内容接口
 * 输入参数  : 系统消息类型、消息内容和整条消息的长度
 * 输出参数  : 无
 * 返 回 值  : 0为成功，小于0为失败
 */
int32 ps_set_sys_variable_length_packet(uint8 *buf, uint8 type, const uint8 *content, uint16 len)
{
    int8 *ptr;
    uint16 content_len;
    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        PS_PRINT_ERR("skb data buf is NULL\n");
        return -EINVAL;
    }

    if ((len > SYS_VARIABLE_LENGTH_PACKET_MAX_LEN) || (len < SYS_VARIABLE_LENGTH_PACKET_MIN_LEN)) {
        PS_PRINT_ERR(
            "sys variable-length packet len wrong,should not less than %d or more than %d,pls check len=[%d]\n",
            SYS_VARIABLE_LENGTH_PACKET_MIN_LEN, SYS_VARIABLE_LENGTH_PACKET_MAX_LEN, len);
        return -EINVAL;
    }

    content_len = len - SYS_VARIABLE_LENGTH_PACKET_MIN_LEN;

    /**********************************************************************************************
    | 7e | 00 |len | 00 | ff |    |    |    |            ... ...            | 7e |
    |  sys packet head  |sys variable-length|sys variable-length packet data|sys packet end
    |       4byte       | packet head 4byte | (x)byte [gnss timesync 8byte] |     1byte
    **********************************************************************************************/
    /* filling sys packet head 4byte */
    ptr = buf;
    *ptr = PACKET_START_SIGNAL;
    ptr++;
    *ptr = type;
    ptr++;
    *ptr = len;
    ptr++;
    *ptr = 0;
    ptr++;
    /* filling sys variable-length packet head 4byte */
    *ptr = TIMESYNC_PACKET_SYS_MSG_TYPE;
    ptr += SYS_VARIABLE_PACKET_MSG_HEAD_LEN;
    /* lint -save -e670 -specific(-e670) */ /* 屏蔽memcpy可疑缓冲区溢出告警 */
    /* 代码前端已规定memcpy长度参数的范围 */
    /* filling sys variable-length packet real data */
    if (memcpy_s(ptr, content_len, content, content_len) != EOK) {
        PS_PRINT_ERR("variable length cmd send failed\n");
        return -EINVAL;
    }
    ptr += content_len;
    /* filling sys packet end 1byte */
    *ptr = PACKET_END_SIGNAL;

    return 0;
}

/*
 * Prototype    : ps_tx_sys_cmd
 * Description  : tx sys commend to uart tty
 */
int32 ps_tx_sys_cmd(struct ps_core_s *ps_core_d, uint8 type, uint8 content)
{
    struct sk_buff *skb = NULL;
    bool ret;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb = alloc_skb(SYS_TOTAL_PACKET_LENTH, oal_in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        PS_PRINT_ERR("can't allocate mem for new sys skb, len=%d\n", SYS_TOTAL_PACKET_LENTH);
        return -EINVAL;
    }

    skb_put(skb, SYS_TOTAL_PACKET_LENTH);
    /* set sys packet head to skb */
    ps_set_sys_packet(skb->data, type, content);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    ret = queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
    if (type == SYS_MSG) {
        PS_PRINT_INFO("queue sys msg work, ret = %d, type = %d\n", ret, content);
    }

    return 0;
}

/*
 * 函 数 名  : ps_tx_sys_variable_length_cmd
 * 功能描述  : 发送系统可变长消息接口
 * 输入参数  : 系统消息类型、消息内容和其长度
 * 输出参数  : 无
 * 返 回 值  : 0为成功，小于0为失败
 */
int32 ps_tx_sys_variable_length_cmd(struct ps_core_s *ps_core_d, uint8 type, const uint8 *content, uint16 len)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    bool ret;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (content == NULL) {
        PS_PRINT_ERR("user data buf is NULL\n");
        return -EINVAL;
    }

    tx_skb_len = len + SYS_VARIABLE_LENGTH_PACKET_MIN_LEN;

    skb = alloc_skb(tx_skb_len, oal_in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        PS_PRINT_ERR("can't allocate mem for new variable length sys skb, len=%d\n", tx_skb_len);
        return -EINVAL;
    }

    skb_put(skb, tx_skb_len);

    /* set sys packet head to skb */
    ps_set_sys_variable_length_packet(skb->data, type, content, tx_skb_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    ret = queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
    PS_PRINT_INFO("queue sys variable-length msg work, ret = %d\n", ret);

    /* lint -save -e670 -specific(-e670) */ /* 屏蔽skb未释放告警 */
    return 0;
}

int32 ps_tx_urgent_cmd(struct ps_core_s *ps_core_d, uint8 type, uint8 content)
{
    struct sk_buff *skb = NULL;
    bool ret;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb = alloc_skb(SYS_TOTAL_PACKET_LENTH, oal_in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        PS_PRINT_ERR("can't allocate mem for new sys skb, len=%d\n", SYS_TOTAL_PACKET_LENTH);
        return -EINVAL;
    }

    skb_put(skb, SYS_TOTAL_PACKET_LENTH);
    /* set sys packet head to skb */
    ps_set_sys_packet(skb->data, type, content);
    ps_skb_enqueue(ps_core_d, skb, TX_URGENT_QUEUE);
    ret = queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
    if (type == SYS_MSG) {
        PS_PRINT_INFO("queue sys msg work, ret = %d, type = %d\n", ret, content);
    }

    return 0;
}

/*
 * Prototype    : ps_tx_fmbuf
 * Description  : add packet head to recv ir data buff from hal,and tx to tty uart
 * input        : ps_core_d
 * output       : not
 */
int32 ps_tx_fmbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb;
    uint16 tx_skb_len;
    uint16 tx_fm_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(NULL == ps_core_d)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    while (count > 0) {
        if (count > FM_TX_PACKET_LIMIT) {
            tx_fm_len = FM_TX_PACKET_LIMIT;
        } else {
            tx_fm_len = count;
        }
        /* curr tx skb total lenth */
        tx_skb_len = tx_fm_len + sizeof(struct ps_packet_head);
        tx_skb_len = tx_skb_len + sizeof(struct ps_packet_end);

        skb = ps_alloc_skb(tx_skb_len);
        if (NULL == skb) {
            PS_PRINT_ERR("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > FM_TX_PACKET_LIMIT) {
            if (false == start) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, FM_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, FM_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, FM_LAST_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_fm_len)) {
            PS_PRINT_ERR("can't copy_from_user for ir\n");
            kfree_skb(skb);
            return -EFAULT;
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_fm_len;
        count = count - tx_fm_len;
    }

    return 0;
}

/*
 * Prototype    : ps_tx_irbuf
 * Description  : add packet head to recv ir data buff from hal,and tx to tty uart
 * input        : ps_core_d
 * output       : not
 */
int32 ps_tx_irbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb;
    uint16 tx_skb_len;
    uint16 tx_ir_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(NULL == ps_core_d)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    while (count > 0) {
        if (count > IR_TX_PACKET_LIMIT) {
            tx_ir_len = IR_TX_PACKET_LIMIT;
        } else {
            tx_ir_len = count;
        }
        /* curr tx skb total lenth */
        tx_skb_len = tx_ir_len + sizeof(struct ps_packet_head);
        tx_skb_len = tx_skb_len + sizeof(struct ps_packet_end);

        skb = ps_alloc_skb(tx_skb_len);
        if (NULL == skb) {
            PS_PRINT_ERR("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > IR_TX_PACKET_LIMIT) {
            if (false == start) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, IR_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, IR_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, IR_LAST_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_ir_len)) {
            PS_PRINT_ERR("can't copy_from_user for ir\n");
            kfree_skb(skb);
            return -EFAULT;
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_ir_len;
        count = count - tx_ir_len;
    }

    return 0;
}

/*
 * Prototype    : ps_tx_nfcbuf
 * Description  : add packet head to recv nfc data buff from hal,and tx to tty uart
 * input        : ps_core_d
 * output       : not
 */
int32 ps_tx_nfcbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb;
    uint16 tx_skb_len;
    uint16 tx_nfc_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(NULL == ps_core_d)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
        return -EINVAL;
    }

    while (count > 0) {
        if (count > NFC_TX_PACKET_LIMIT) {
            tx_nfc_len = NFC_TX_PACKET_LIMIT;
        } else {
            tx_nfc_len = count;
        }
        /* curr tx skb total lenth */
        tx_skb_len = tx_nfc_len + sizeof(struct ps_packet_head);
        tx_skb_len = tx_skb_len + sizeof(struct ps_packet_end);

        skb = ps_alloc_skb(tx_skb_len);
        if (NULL == skb) {
            PS_PRINT_ERR("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > NFC_TX_PACKET_LIMIT) {
            if (false == start) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, NFC_First_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, NFC_Common_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, NFC_Last_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_nfc_len)) {
            PS_PRINT_ERR("can't copy_from_user for nfc\n");
            kfree_skb(skb);
            return -EFAULT;
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_nfc_len;
        count = count - tx_nfc_len;
    }

    return 0;
}

/*
 * Prototype    : ps_tx_gnssbuf
 * Description  : add packet head to recv gnss data buff from hal,and tx to tty uart
 * input        : ps_core_d
 * output       : not
 */
int32 ps_tx_gnssbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb;
    uint16 tx_skb_len;
    uint16 tx_gnss_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(NULL == ps_core_d)) {
        PS_PRINT_ERR("ps_core_d is NULL\n");
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
        if (NULL == skb) {
            PS_PRINT_ERR("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > GNSS_TX_PACKET_LIMIT) {
            if (false == start) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, GNSS_First_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, GNSS_Common_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, GNSS_Last_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_gnss_len)) {
            PS_PRINT_ERR("can't copy_from_user for gnss\n");
            kfree_skb(skb);
            return -EFAULT;
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, TX_LOW_QUEUE);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_gnss_len;
        count = count - tx_gnss_len;
    }

    return 0;
}

void ps_core_bfgx_info_name_init(struct ps_core_s *ps_core_d, bfgx_subsys_type_enum subsys)
{
    switch (subsys) {
        case BFGX_BT:
            ps_core_d->bfgx_info[subsys].name = "bt";
            break;
        case BFGX_FM:
            ps_core_d->bfgx_info[subsys].name = "fm";
            break;
        case BFGX_GNSS:
            ps_core_d->bfgx_info[subsys].name = "gnss";
            break;
        case BFGX_IR:
            ps_core_d->bfgx_info[subsys].name = "ir";
            break;
        case BFGX_NFC:
            ps_core_d->bfgx_info[subsys].name = "nfc";
            break;
        default:
            ps_core_d->bfgx_info[subsys].name = "N/A";
            break;
    }
}

/*
 * Prototype    : ps_core_init
 * Description  : init ps_core_d struct and kzalloc memery
 * input        : no
 * output       : ps_core_d
 */
int32 ps_core_init(struct ps_core_s **core_data)
{
    struct ps_core_s *ps_core_d;
    struct ps_pm_s *ps_pm_d;
    uint8 *ptr;
    int32 err;
    int32 i = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = kzalloc(sizeof(struct ps_core_s), GFP_KERNEL);
    if (!ps_core_d) {
        PS_PRINT_ERR("memory allocation failed\n");
        return -ENOMEM;
    }

    ptr = kzalloc(PUBLIC_BUF_MAX, GFP_KERNEL);
    if (!ptr) {
        kfree(ps_core_d);
        PS_PRINT_ERR("no mem to allocate to public buf!\n");
        return -ENOMEM;
    }
    memset_s(ptr, PUBLIC_BUF_MAX, 0, PUBLIC_BUF_MAX);
    ps_core_d->rx_public_buf_org_ptr = ptr;
    ps_core_d->rx_decode_public_buf_ptr = ptr;

    ps_pm_d = kzalloc(sizeof(struct ps_pm_s), GFP_KERNEL);
    if (!ps_pm_d) {
        PS_PRINT_ERR("ps_pm_d memory allocation failed\n");
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d);
        return -ENOMEM;
    }

    ps_core_d->ps_pm = ps_pm_d;
    ps_pm_d->ps_core_data = ps_core_d;
    err = ps_pm_register(ps_pm_d);
    if (err) {
        kfree(ps_core_d->ps_pm);
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d);
        PS_PRINT_ERR("error registering ps_pm err = %d\n", err);
        return -EFAULT;
    }

    ps_pm_d->change_baud_rate = ps_change_uart_baud_rate;
    ps_pm_d->operate_beat_timer = mod_beat_timer;

    err = plat_uart_init();
    if (err) {
        ps_pm_unregister(ps_pm_d);
        kfree(ps_core_d->ps_pm);
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d);
#ifdef N_HW_BFG
        PS_PRINT_ERR("error registering %d line discipline %d\n", N_HW_BFG, err);
#endif
        return -EFAULT;
    }

    /* Initialize PS tx and rx queue head. BT module skb's
     * will be pushed in this queue for actual transmission.
     */
    skb_queue_head_init(&ps_core_d->tx_urgent_seq);
    skb_queue_head_init(&ps_core_d->tx_high_seq);
    skb_queue_head_init(&ps_core_d->tx_low_seq);
    skb_queue_head_init(&ps_core_d->rx_dbg_seq);

    spin_lock_init(&ps_core_d->rx_lock);
    spin_lock_init(&ps_core_d->gnss_rx_lock);
    init_completion(&ps_core_d->wait_wifi_opened);
    init_completion(&ps_core_d->wait_wifi_closed);
    /* create a singlethread work queue */
    ps_core_d->ps_tx_workqueue = create_singlethread_workqueue("ps_tx_queue");
    /* init tx work */
    INIT_WORK(&ps_core_d->tx_skb_work, ps_core_tx_work);

    for (i = 0; i < BFGX_BUTT; i++) {
        /* 初始化接收队列头 */
        skb_queue_head_init(&ps_core_d->bfgx_info[i].rx_queue);
        /* 初始化BFGX接收等待队列 */
        init_waitqueue_head(&ps_core_d->bfgx_info[i].rx_wait);
        /* 初始化分包接收数据结构，BT不使用分包接收 */
        spin_lock_init(&ps_core_d->bfgx_info[i].sepreted_rx.sepreted_rx_lock);
        ps_core_d->bfgx_info[i].sepreted_rx.rx_prev_seq = RX_SEQ_NULL;
        ps_core_d->bfgx_info[i].sepreted_rx.rx_buf_all_len = 0;
        ps_core_d->bfgx_info[i].sepreted_rx.rx_buf_ptr = NULL;
        ps_core_d->bfgx_info[i].sepreted_rx.rx_buf_org_ptr = NULL;
        /* 初始化开关业务完成量 */
        init_completion(&ps_core_d->bfgx_info[i].wait_closed);
        init_completion(&ps_core_d->bfgx_info[i].wait_opened);
        atomic_set(&ps_core_d->bfgx_info[i].subsys_state, POWER_STATE_SHUTDOWN);
        ps_core_bfgx_info_name_init(ps_core_d, i);
    }

    atomic_set(&ps_core_d->node_visit_flag, 0);

    init_waitqueue_head(&ps_core_d->rx_dbg_wait);

    ps_core_d->rx_pkt_total_len = 0;
    ps_core_d->rx_have_recv_pkt_len = 0;
    ps_core_d->rx_have_del_public_len = 0;

    ps_core_d->tty_have_open = false;
    *core_data = ps_core_d;

    ps_core_d->rx_pkt_num[BFGX_BT] = 0;
    ps_core_d->rx_pkt_num[BFGX_FM] = 0;
    ps_core_d->rx_pkt_num[BFGX_GNSS] = 0;
    ps_core_d->rx_pkt_num[BFGX_IR] = 0;
    ps_core_d->rx_pkt_num[BFGX_NFC] = 0;
    ps_core_d->tx_pkt_num[BFGX_BT] = 0;
    ps_core_d->tx_pkt_num[BFGX_FM] = 0;
    ps_core_d->tx_pkt_num[BFGX_GNSS] = 0;
    ps_core_d->tx_pkt_num[BFGX_IR] = 0;
    ps_core_d->rx_pkt_num[BFGX_NFC] = 0;
    ps_core_d->rx_pkt_sys = 0;
    ps_core_d->rx_pkt_oml = 0;

    return 0;
}

/*
 * Prototype    : ps_core_exit
 * Description  : release have init ps_core_d struct and kfree memeryo
 */
int32 ps_core_exit(struct ps_core_s *ps_core_d)
{
    int32 err;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d != NULL) {
        /* Free PS tx and rx queue */
        ps_kfree_skb(ps_core_d, TX_HIGH_QUEUE);
        ps_kfree_skb(ps_core_d, TX_LOW_QUEUE);
        ps_kfree_skb(ps_core_d, RX_GNSS_QUEUE);
        ps_kfree_skb(ps_core_d, RX_FM_QUEUE);
        ps_kfree_skb(ps_core_d, RX_BT_QUEUE);
        ps_kfree_skb(ps_core_d, RX_DBG_QUEUE);
        ps_kfree_skb(ps_core_d, RX_NFC_QUEUE);
        ps_kfree_skb(ps_core_d, RX_IR_QUEUE);

        /* free tx work queue */
        destroy_workqueue(ps_core_d->ps_tx_workqueue);

        /* TTY ldisc cleanup */
        err = plat_uart_exit();
        if (err) {
            PS_PRINT_ERR("unable to un-register ldisc %d\n", err);
        }
        /* unregister pm */
        ps_pm_unregister(ps_core_d->ps_pm);
        /* free the global data pointer */
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d->ps_pm);
        kfree(ps_core_d);
    }
    return 0;
}

/*
 * Prototype    : wifi_choose_bfgn_channel_send_log2sdt
 * Description  : sdio read bfgn log, send to bfgn log channels
 * input        : data -> ptr
                  len  -> size of data ptr
 * output       : no
 */
int32 wifi_choose_bfgn_channel_send_log2sdt(uint8 *data, uint16 len)
{
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;
    ps_get_core_reference(&ps_core_d);

    ret = ps_push_skb_debug_queue(ps_core_d, data, len, RX_DBG_QUEUE);
    if (ret < 0) {
        PS_PRINT_ERR("push sdio data to skb queue failed\n");
        return 0;
    }
    return 1;
}
