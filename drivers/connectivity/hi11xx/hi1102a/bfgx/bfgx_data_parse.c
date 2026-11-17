

/* Include Head file */
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/tty.h>

#include "securec.h"
#include "board.h"
#include "plat_type.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "bfgx_user_ctrl.h"
#include "plat_pm.h"
#include "plat_cali.h"
#include "bfgx_exception_rst.h"
#include "bfgx_platform_msg_handle.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_int32 g_bfgx_panic_ssi_dump = 0;
oal_debug_module_param(g_bfgx_panic_ssi_dump, int, S_IRUGO | S_IWUSR);
#endif

uint8 g_wakeup_src_debug = 0;
static unsigned int g_reset_cnt = 0;

/* function pointer for rx data */
int32 (*tty_recv)(void *, const uint8 *, int32);

uint32 g_bfgx_rx_max_frame[BFGX_BUTT] = {
    BT_RX_MAX_FRAME,
    FM_RX_MAX_FRAME,
    GNSS_RX_MAX_FRAME,
    IR_RX_MAX_FRAME,
    NFC_RX_MAX_FRAME,
};

uint32 g_bfgx_rx_queue[BFGX_BUTT] = {
    RX_BT_QUEUE,
    RX_FM_QUEUE,
    RX_GNSS_QUEUE,
    RX_IR_QUEUE,
    RX_NFC_QUEUE,
};

OAL_STATIC uint32 g_bfgx_rx_queue_max_num[BFGX_BUTT] = {
    RX_BT_QUE_MAX_NUM,
    RX_FM_QUE_MAX_NUM,
    RX_GNSS_QUE_MAX_NUM,
    RX_IR_QUE_MAX_NUM,
    RX_NFC_QUE_MAX_NUM,
};

typedef int32_t (*ps_core_msg_handler)(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_no_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_debug_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_mem_dump_size_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_mem_dump_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);

STATIC ps_core_msg_handler g_ps_core_msg_handle[MSG_BUTT] = {
    ps_exe_sys_func,             // SYS_MSG = 0x00,         系统串口消息
    ps_recv_no_sepreated_data,   // BT_MSG = 0x01,          BT串口消息
    ps_recv_sepreated_data,      // GNSS_FIRST_MSG = 0x02,  GNSS串口消息，第一个分段消息
    ps_recv_sepreated_data,      // GNSS_COMMON_MSG = 0x03, GNSS串口消息，中间分段消息
    ps_recv_sepreated_data,      // GNSS_LAST_MSG = 0x04,   GNSS串口消息，最后一个分段消息
    ps_recv_sepreated_data,      // FM_FIRST_MSG = 0x05,    FM串口消息，第一个分段消息
    ps_recv_sepreated_data,      // FM_COMMON_MSG = 0x06,   FM串口消息，中间分段消息
    ps_recv_sepreated_data,      // FM_LAST_MSG = 0x07,     FM串口消息，最后一个分段消息
    ps_recv_sepreated_data,      // IR_FIRST_MSG = 0x08,    红外串口消息，第一个分段消息
    ps_recv_sepreated_data,      // IR_COMMON_MSG = 0x09,   红外串口消息，中间分段消息
    ps_recv_sepreated_data,      // IR_LAST_MSG = 0x0A,     红外串口消息，最后一个分段消息
    ps_recv_sepreated_data,      // NFC_FIRST_MSG = 0x0B,   NFC串口消息，第一个分段消息
    ps_recv_sepreated_data,      // NFC_COMMON_MSG = 0x0C,  NFC串口消息，中间分段消息
    ps_recv_sepreated_data,      // NFC_LAST_MSG = 0x0D,    NFC串口消息，最后一个分段消息
    ps_recv_debug_data,          // OML_MSG = 0x0E,         可维可测消息
    ps_recv_mem_dump_size_data,  // MEM_DUMP_SIZE = 0x0f,   bfgx异常时，要dump的mem长度消息
    ps_recv_mem_dump_data,       // MEM_DUMP = 0x10,        bfgx异常时，内存dump消息
    NULL,                        // WIFI_MEM_DUMP = 0x11,   UART READ WIFI MEM，内存dump消息
    NULL,                        // BFGX_CALI_MSG = 0x12,   BFGX 校准数据上报
};

/* Function implement */
void print_uart_decode_param(void)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    ps_print_warning("rx_pkt_type=%x,rx_have_recv_pkt_len=%d,rx_pkt_total_len=%d,rx_have_del_public_len=%d\n",
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
 */
int32 ps_write_tty(struct ps_core_s *ps_core_d, const uint8 *data, int32 count)
{
    struct tty_struct *tty = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely((ps_core_d == NULL) || (ps_core_d->tty == NULL))) {
        ps_print_err("tty unavailable to perform write\n");
        return -EINVAL;
    }

    tty = ps_core_d->tty;

    ps_uart_tty_tx_add(count);
    return tty->ops->write(tty, data, count);
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
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }

    skb = alloc_skb(pkt_len, GFP_ATOMIC);
    if (skb == NULL) {
        ps_print_err("can't allocate mem for new skb, len=%d\n", pkt_len);
        return -EINVAL;
    }

    skb_put(skb, pkt_len);
    ret = memcpy_s(skb->data, pkt_len, buf_ptr, pkt_len);
    if (ret != EOK) {
        ps_print_err("skb data copy failed\n");
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
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    do {
        if ((skb = ps_skb_dequeue(ps_core_d, RX_GNSS_QUEUE)) == NULL) {
            ps_print_warning("gnss skb rx queue is empty\n");
            return 0;
        }

        ps_print_warning("gnss delete msg, skb->len=%d, qlen=%d\n",
                         skb->len, ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen);

        seperate_tag = skb->data[skb->len - 1];
        switch (seperate_tag) {
            case GNSS_SEPER_TAG_INIT:
            case GNSS_SEPER_TAG_LAST:
                kfree_skb(skb);
                break;
            default:
                ps_print_err("seperate_tag=%x not support\n", seperate_tag);
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
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
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
            ps_print_err("alloc new skb fail [%u]time\n", i + 1);
        }
        if (skb == NULL) {
            ps_print_err("===can't alloc mem for new skb, tag:%x, slen=%x, pkt_len=%x\n",
                         seperate_tag, seperate_len, pkt_len);
            while (copy_cnt > 0) {
                if ((skb = skb_dequeue_tail(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue)) == NULL) {
                    ps_print_err("===gnss rx queue has no data!\n");
                    spin_unlock(&ps_core_d->gnss_rx_lock);
                    return -EINVAL;
                }

                ps_print_err("===drop gnss seperate data, tag:%x, slen=%x, skb->len=%x, pkt_len=%x\n",
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
            ps_print_err("skb_data memcpy_s failed\n");
            spin_unlock(&ps_core_d->gnss_rx_lock);
            kfree_skb(skb);
            return -EINVAL;
        }
        skb->data[seperate_len - 1] = seperate_tag;
        ps_print_dbg("===tag:%x, slen=%x, skb_len=%x, copy_cnt=%x, pkt_len=%x\n",
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
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }

    skb = skb_dequeue_tail(&ps_core_d->rx_dbg_seq);
    if (skb != NULL) {
        if (memcpy_s(skb_tail_pointer(skb), skb_tailroom(skb), buf_ptr, pkt_len) == EOK) {
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
        ps_print_err("can't allocate mem for new debug skb, len=%d\n", count);
        return -EINVAL;
    }

    if (unlikely(memcpy_s(skb_tail_pointer(skb), count, buf_ptr, pkt_len) != EOK)) {
        ps_print_err("memcopy_s error, destlen=%d, srclen=%d\n ", count, pkt_len);
    }
    skb_put(skb, pkt_len);
    ps_skb_enqueue(ps_core_d, skb, type);

    return 0;
}

STATIC int32 ps_recv_mem_dump_size_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint32 dump_mem_size;
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    uint16 rx_pkt_total_len;
#endif
    ps_print_dbg("recv MEM DUMP SIZE data\n");

    if (!ps_is_device_log_enable()) {
        return 0;
    }

#if ((_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_HOST) && (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_HOST))
    dump_mem_size = *(uint16 *)buf_ptr;
#else
    dump_mem_size = *(uint32 *)buf_ptr;
#endif

    ps_print_info("prepare to recv bfgx mem size [%d]!\n", dump_mem_size);
#ifdef HI110X_HAL_MEMDUMP_ENABLE
    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    bfgx_notice_hal_memdump();
    bfgx_memdump_enquenue(buf_ptr, rx_pkt_total_len);
#else
    prepare_to_recv_bfgx_stack(dump_mem_size);
#endif
    return 0;
}

STATIC int32 ps_recv_mem_dump_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint16 rx_pkt_total_len;

    if (!ps_is_device_log_enable()) {
        return 0;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if (rx_pkt_total_len <= MEM_DUMP_RX_MAX_FRAME) {
        ps_print_dbg("recv bfgx stack size [%d]!\n", rx_pkt_total_len);
#ifdef HI110X_HAL_MEMDUMP_ENABLE
        bfgx_memdump_enquenue(buf_ptr, rx_pkt_total_len);
#else
        bfgx_recv_dev_mem(buf_ptr, rx_pkt_total_len);
#endif
    }

    return 0;
}


STATIC int32 ps_recv_uart_loop_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    int32 ret;
    uint16 rx_pkt_total_len;

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if (rx_pkt_total_len <= UART_LOOP_MAX_PKT_LEN) {
        ret = uart_loop_test_recv_pkt(ps_core_d, buf_ptr, rx_pkt_total_len);
        if (ret < 0) {
            ps_print_err("uart_loop_test_recv_pkt failed\n");
            return -EINVAL;
        }
    } else {
        ps_print_err("uart loop test, recv pkt len is too large!\n");
        return -EINVAL;
    }

    return 0;
}

static int32 ps_push_device_log_to_queue(struct ps_core_s *ps_core_d, const uint8 *buf_ptr, uint16 len)
{
    struct sk_buff *skb = NULL;
    int ret;

    if (!ps_is_device_log_enable()) {
        return 0;
    }

    if (atomic_read(&ps_core_d->dbg_recv_dev_log) == 0) {
        return 0;
    }

    if (len > DBG_RX_MAX_FRAME) {
        return 0;
    }

    ret = ps_push_skb_debug_queue(ps_core_d, buf_ptr, len, RX_DBG_QUEUE);
    if (ret < 0) {
        ps_print_err("push debug data to skb queue failed\n");
        return -EINVAL;
    }

    ps_print_dbg("rx_dbg_seq.qlen = %d\n", ps_core_d->rx_dbg_seq.qlen);
    wake_up_interruptible(&ps_core_d->rx_dbg_wait);
    if (ps_core_d->rx_dbg_seq.qlen > RX_DBG_QUE_MAX_NUM) {
        ps_print_info("rx dbg queue too large!");
        /* if sdt data is large，remove the head skb data */
        skb = ps_skb_dequeue(ps_core_d, RX_DBG_QUEUE);
        kfree_skb(skb);
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
STATIC int32 ps_recv_debug_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint16 rx_pkt_total_len;
    uint16 dbg_pkt_lenth = 0;
    uint8 ptr_index = 0;

    ps_core_d->rx_pkt_oml++;
    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if ((buf_ptr[START_SIGNAL_LAST_WORDS] == PACKET_START_SIGNAL)
        && (buf_ptr[PACKET_RX_FUNC_INDEX_LAST_WORDS] == PACKET_RX_FUNC_LAST_WORDS)) {
        /* get FrameLen 2 bytes */
        dbg_pkt_lenth = *(uint16 *)&buf_ptr[PACKET_FRAMELEN_INDEX];
        if ((dbg_pkt_lenth == rx_pkt_total_len) && (dbg_pkt_lenth == LAST_WORD_LEN) &&
            (buf_ptr[RPT_IND_INDEX_LAST_WORDS] == PACKET_RX_RPT_IND_LAST_WORDS)) {
            ps_print_err("recv device last words!Faulttype=0x%x,FaultReason=0x%x,PC=0x%x,LR=0x%x\n",
                         *(uint32 *)&buf_ptr[FAULT_TYPE_INDEX_LAST_WORDS],
                         *(uint32 *)&buf_ptr[FAULT_REASON_INDEX_LAST_WORDS],
                         *(uint32 *)&buf_ptr[PC_INDEX_LAST_WORDS],
                         *(uint32 *)&buf_ptr[LR_INDEX_LAST_WORDS]);
            declare_dft_trace_key_info("bfgx_device_panic", OAL_DFT_TRACE_EXCEP);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            if (g_bfgx_panic_ssi_dump) {
                ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL);
            }
#endif
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_DEV, CHR_PLAT_DRV_ERROR_BFG_DEV_PANIC);

            plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, BFGX_LASTWORD_PANIC);
        } else {
            /* buf maybe less than log header len */
            if (rx_pkt_total_len > PACKET_HEADER_LEN) {
                ps_print_warning("recv wrong last words,[%x %x]\n",
                                 *(uint32 *)&buf_ptr[START_SIGNAL_LAST_WORDS],
                                 *(uint32 *)&buf_ptr[PACKET_FRAMELEN_INDEX]);
            } else {
                ps_print_warning("recv wrong last words,len less than head\n");
                for (ptr_index = 0; ptr_index < rx_pkt_total_len; ptr_index++) {
                    ps_print_warning("lastwords[%d]=%x\n", ptr_index, buf_ptr[ptr_index]);
                }
            }
        }
    }

    return ps_push_device_log_to_queue(ps_core_d, buf_ptr, rx_pkt_total_len);
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
        ps_print_err("ps_core_d is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL, subsys=%d\n", subsys);
        return -EINVAL;
    }

    if (subsys >= BFGX_BUTT) {
        ps_print_err("subsys index %d outof range\n", subsys);
        return -EINVAL;
    }

    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    rx_current_pkt_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    pst_sepreted_data->rx_pkt_len = rx_current_pkt_len;

    if (likely((uint32)(pst_sepreted_data->rx_buf_all_len + rx_current_pkt_len) <= g_bfgx_rx_max_frame[subsys])) {
        if (likely(pst_sepreted_data->rx_buf_ptr != NULL)) {
            if (unlikely(memcpy_s(pst_sepreted_data->rx_buf_ptr, g_bfgx_rx_max_frame[subsys] -
                                  pst_sepreted_data->rx_buf_all_len, buf_ptr, rx_current_pkt_len) != EOK)) {
                ps_print_err("dis space is not enough\n ");
                return RX_PACKET_ERR;
            }
            pst_sepreted_data->rx_buf_all_len += rx_current_pkt_len;
            pst_sepreted_data->rx_buf_ptr += rx_current_pkt_len;
            return RX_PACKET_CORRECT;
        } else {
            ps_print_err("sepreted rx_buf_ptr is NULL\n");
            return RX_PACKET_ERR;
        }
    } else {
        ps_print_err("rx_current_pkt_len=%d,rx_pkt_total_len=%d,rx_buf_all_len=%d,subsys=%d\n",
                     pst_sepreted_data->rx_pkt_len,
                     ps_core_d->rx_pkt_total_len,
                     pst_sepreted_data->rx_buf_all_len,
                     subsys);
        return RX_PACKET_ERR;
    }
}

STATIC uint8_t ps_get_sepreated_subsys_type(struct ps_core_s *ps_core_d)
{
    uint8_t subsys = BFGX_BUTT;
    uint8_t msg_type = ps_core_d->rx_pkt_type;

    switch (msg_type) {
        case GNSS_FIRST_MSG:
        case GNSS_COMMON_MSG:
        case GNSS_LAST_MSG:
            subsys = BFGX_GNSS;
            break;

        case FM_FIRST_MSG:
        case FM_COMMON_MSG:
        case FM_LAST_MSG:
            subsys = BFGX_FM;
            break;

        case IR_FIRST_MSG:
        case IR_COMMON_MSG:
        case IR_LAST_MSG:
            subsys = BFGX_IR;
            break;

        case NFC_FIRST_MSG:
        case NFC_COMMON_MSG:
        case NFC_LAST_MSG:
            subsys = BFGX_NFC;
            break;

        default:
            /* BT not use sepreted rx */
            subsys = BFGX_BUTT;
            break;
    }

    return subsys;
}


STATIC uint8_t ps_get_sepreated_data_type(struct ps_core_s *ps_core_d)
{
    uint8_t sepreted_type = RX_SEQ_BUTT;
    uint8_t msg_type = ps_core_d->rx_pkt_type;

    switch (msg_type) {
        case GNSS_FIRST_MSG:
        case FM_FIRST_MSG:
        case IR_FIRST_MSG:
        case NFC_FIRST_MSG:
            sepreted_type = RX_SEQ_START;
            break;

        case GNSS_COMMON_MSG:
        case FM_COMMON_MSG:
        case IR_COMMON_MSG:
        case NFC_COMMON_MSG:
            sepreted_type = RX_SEQ_INT;
            break;

        case GNSS_LAST_MSG:
        case FM_LAST_MSG:
        case IR_LAST_MSG:
        case NFC_LAST_MSG:
            sepreted_type = RX_SEQ_LAST;
            break;

        default:
            sepreted_type = RX_SEQ_BUTT;
            break;
    }

    return sepreted_type;
}

STATIC int32_t ps_sepreated_type_check(struct bfgx_sepreted_rx_st *pst_sepreted_data, uint8_t sepreted_type)
{
    int32_t seq_correct = SEPRETED_RX_PKT_SEQ_ERROR;

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

    return seq_correct;
}

STATIC void ps_sepreated_rx_buf_reset(struct bfgx_sepreted_rx_st *pst_sepreted_data)
{
    pst_sepreted_data->rx_buf_ptr = pst_sepreted_data->rx_buf_org_ptr;
    pst_sepreted_data->rx_buf_all_len = 0;
}

STATIC int32_t ps_push_sepreated_data_to_queue(struct ps_core_s *ps_core_d, uint8_t subsys)
{
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    int32_t ret;

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    pst_sepreted_data = &pst_bfgx_data->sepreted_rx;

    /* 如果已经缓存的数据到达了最大值，则新来的数据被丢弃 */
    if (pst_bfgx_data->rx_queue.qlen >= g_bfgx_rx_queue_max_num[subsys]) {
        ps_print_warning("%s rx queue too large! qlen=%d\n",
                         g_bfgx_subsys_name[subsys], pst_bfgx_data->rx_queue.qlen);
        oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);
        ps_sepreated_rx_buf_reset(pst_sepreted_data);
        return -EINVAL;
    }
    /* gnss packet may large than 8KB, skb_alloc may fail, so seperate the packet to 4KB at most */
    if (subsys == BFGX_GNSS) {
        ret = ps_push_skb_queue_gnss(ps_core_d, pst_sepreted_data->rx_buf_org_ptr,
                                     pst_sepreted_data->rx_buf_all_len, g_bfgx_rx_queue[subsys]);
    } else {
        ret = ps_push_skb_queue(ps_core_d, pst_sepreted_data->rx_buf_org_ptr,
                                pst_sepreted_data->rx_buf_all_len, g_bfgx_rx_queue[subsys]);
    }

    ps_sepreated_rx_buf_reset(pst_sepreted_data);
    if (ret < 0) {
        ps_print_err("push %s rx data to skb failed!\n", g_bfgx_subsys_name[subsys]);
        return -EINVAL;
    }

    /* 现在skb中已经有正确完整的数据，唤醒等待数据的进程 */
    ps_print_dbg("%s rx done! qlen=%d\n", g_bfgx_subsys_name[subsys], pst_bfgx_data->rx_queue.qlen);
    oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);

    return 0;
}

STATIC int32_t ps_sepreated_data_recv(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint8_t subsys)
{
    int32_t ret;
    int32_t seq_correct;
    uint8_t sepreted_type;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;

    sepreted_type = ps_get_sepreated_data_type(ps_core_d);
    if (sepreted_type >= RX_SEQ_BUTT) {
        ps_print_err("sepreted type error, type=%d\n", sepreted_type);
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    pst_sepreted_data = &pst_bfgx_data->sepreted_rx;

    /* 检查分包序列的正确性 */
    seq_correct = ps_sepreated_type_check(pst_sepreted_data, sepreted_type);
    if (seq_correct != SEPRETED_RX_PKT_SEQ_CORRECT) {
        ps_print_err("%s rx seq is err! sepreted_type=%d, rx_prev_seq=%d\n",
                     g_bfgx_subsys_name[subsys], sepreted_type, pst_sepreted_data->rx_prev_seq);
        return -EINVAL;
    }

    /* 接收到的分包数据都要先拷贝到组包buffer中 */
    ret = ps_store_rx_sepreated_data(ps_core_d, buf_ptr, subsys);
    /*
     * 当组包发生错误时，只有在收到LAST包的时候才重置组包buffer。
     * 因为只有收到LAST包，才能确保接收到的下一包数据的正确性。
     */
    if ((ret == RX_PACKET_ERR) && (sepreted_type == RX_SEQ_LAST)) {
        ps_print_err("%s rx data lenth err! give up this total pkt\n", g_bfgx_subsys_name[subsys]);
        ps_sepreated_rx_buf_reset(pst_sepreted_data);
        return -EINVAL;
    }

    /* 收到LAST包，说明组包完成，否则继续接收，直到收到完整的数据包，或者中途发生错误，丢弃该包。 */
    if (sepreted_type == RX_SEQ_LAST) {
        ret = ps_push_sepreated_data_to_queue(ps_core_d, subsys);
    }

    return ret;
}

/*
 * Prototype    : ps_recv_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                  memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
STATIC int32_t ps_recv_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t subsys;
    uint16_t len;
    int32_t ret;
    const uint32_t max_dump_len = 32;
    struct pm_drv_data *pm_data = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;

    subsys = ps_get_sepreated_subsys_type(ps_core_d);
    if (subsys >= BFGX_BUTT) {
        ps_print_err("sepreted subsys error, subsys=%d\n", subsys);
        return -EINVAL;
    }

    pm_data = pm_get_drvdata();
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is null\n");
        return -EINVAL;
    }

    if (g_uart_loop_test_cfg.uart_loop_enable) {
        ps_print_dbg("recv UART LOOP data\n");
        ps_recv_uart_loop_data(ps_core_d, buf_ptr);
        return 0;
    }

    ps_core_d->rx_pkt_num[subsys]++;
    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    pst_sepreted_data = &pst_bfgx_data->sepreted_rx;

    spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    if (atomic_read(&pst_bfgx_data->subsys_state) != POWER_STATE_OPEN) {
        len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
        ps_print_warning("%s has closed, no need to recv data, len is %d\n", g_bfgx_subsys_name[subsys], len);
        oal_print_hex_dump(buf_ptr, (len < max_dump_len ? len : max_dump_len), HEX_DUMP_GROUP_SIZE, "rec data :");
        spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
        return 0;
    }

    if (subsys == BFGX_GNSS) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
        oal_wake_lock_timeout(&pm_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);
    }

    ret = ps_sepreated_data_recv(ps_core_d, buf_ptr, subsys);

    spin_unlock(&pst_sepreted_data->sepreted_rx_lock);

    return ret;
}

/*
 * Prototype    : ps_recv_no_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                  memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
STATIC int32 ps_recv_no_sepreated_data(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    int32 ret;
    uint16 rx_pkt_total_len = 0;
    struct pm_drv_data *pm_data = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    pm_data = pm_get_drvdata();
    ps_core_d->rx_pkt_num[BFGX_BT]++;
    pst_bfgx_data = &ps_core_d->bfgx_info[BFGX_BT];

    oal_wake_lock_timeout(&pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* 如果已经缓存的数据到达了最大值，则新来的数据被丢弃 */
    if (pst_bfgx_data->rx_queue.qlen >= g_bfgx_rx_queue_max_num[BFGX_BT]) {
        ps_print_warning("%s rx queue too large! qlen=%d\n",
                         g_bfgx_subsys_name[BFGX_BT], pst_bfgx_data->rx_queue.qlen);
        oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);
        return -EINVAL;
    }

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
        ret = ps_push_skb_queue(ps_core_d, buf_ptr, rx_pkt_total_len, g_bfgx_rx_queue[BFGX_BT]);
        if (ret < 0) {
            ps_print_err("push %s rx data to skb failed!\n", g_bfgx_subsys_name[BFGX_BT]);
            return -EINVAL;
        }

        ps_print_dbg("%s rx done! qlen=%d\n", g_bfgx_subsys_name[BFGX_BT], pst_bfgx_data->rx_queue.qlen);
        oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);
    }

    return 0;
}

STATIC void ps_core_updata_bfg_timer(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t pkt_type;
    uint8_t *ptr = NULL;
    struct pm_drv_data *pm_data = NULL;

    pm_data = pm_get_drvdata();
    ptr = buf_ptr + sizeof(struct ps_packet_head);
    pkt_type = ps_core_d->rx_pkt_type;

    /* if data comes from dev which is not SYS_INF_DEV_AGREE_HOST_SLP &
        not gnss data & not oml data, we should re-monitor uart transfer
      */
    if (pkt_type == SYS_MSG) {
        if ((*ptr != SYS_INF_DEV_AGREE_HOST_SLP) && (*ptr < SYS_INF_HEARTBEAT_CMD_BASE)) {
            if (!bfgx_other_subsys_all_shutdown(BFGX_GNSS)) {
                mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(BT_SLEEP_TIME));
                pm_data->bfg_timer_mod_cnt++;
            }
        }
    } else if ((pkt_type != GNSS_FIRST_MSG) && (pkt_type != GNSS_COMMON_MSG) &&
               (pkt_type != GNSS_LAST_MSG) && (pkt_type != OML_MSG) &&
               (oal_atomic_read(&g_ir_only_mode) == 0)) {
        mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(BT_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;

        pm_data->gnss_votesleep_check_cnt = 0;
        pm_data->rx_pkt_gnss_pre = 0;
    }
}

STATIC void ps_core_updata_pm_status(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t pkt_type;
    uint8_t *ptr = NULL;
    struct pm_drv_data *pm_data = NULL;

    pm_data = pm_get_drvdata();
    ptr = buf_ptr + sizeof(struct ps_packet_head);
    pkt_type = ps_core_d->rx_pkt_type;
    /*
     * if some data comes from device which is the normal protocol msg,
     * mark it as "received data before device ack".
     */
    if (unlikely(ps_core_d->ps_pm->bfgx_dev_state_get() == BFGX_SLEEP) &&
        (pkt_type != OML_MSG) && (pkt_type != MEM_DUMP_SIZE) &&
        (pkt_type != MEM_DUMP) && (*ptr != SYS_INF_DEV_AGREE_HOST_SLP) &&
        (*ptr < SYS_INF_HEARTBEAT_CMD_BASE)) {
        ps_core_d->ps_pm->pm_priv_data->rcvdata_bef_devack_flag = 1;
        ps_print_info("recv data before dev_ack, type=[0x%x]\n", pkt_type);
    }

    if (g_wakeup_src_debug) {
        ps_print_info("dev wakeup host pkt type is [%d]\n", pkt_type);
        g_wakeup_src_debug = 0;
    }
}

STATIC void ps_core_status_updata(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    struct st_exception_info *exception_data = NULL;

    get_exception_info_reference(&exception_data);
    if (exception_data != NULL) {
        if (exception_data->debug_beat_flag == 1) {
            atomic_set(&exception_data->bfgx_beat_flag, BFGX_RECV_BEAT_INFO);
        }
    }

    ps_core_updata_bfg_timer(ps_core_d, buf_ptr);
    ps_core_updata_pm_status(ps_core_d, buf_ptr);
}

/*
 * Prototype    : ps_decode_packet
 * Description  : called by core when recive data from device is
 *                  a complete packet
 *              : decode packet function,and push sk_buff head queue
 * input        : buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
STATIC int32 ps_decode_packet(struct ps_core_s *ps_core_d, uint8 *buf_ptr)
{
    uint8_t msg_type;
    uint8 *ptr = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_core_status_updata(ps_core_d, buf_ptr);

    ptr = buf_ptr + sizeof(struct ps_packet_head);
    msg_type = ps_core_d->rx_pkt_type;
    if (msg_type >= MSG_BUTT) {
        ps_print_err("msg_type:0x%x error\n", msg_type);
        return -EINVAL;
    }

    if (g_ps_core_msg_handle[msg_type] != NULL) {
        g_ps_core_msg_handle[msg_type](ps_core_d, ptr);
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
            ps_print_dbg("Host recv CHR errno!");
        } else {
            ps_print_err("the pkt type and len err: %x, %x\n", ps_core_d->rx_pkt_type,
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
        uart_pkt_err_end_time = uart_pkt_err_start_time + OAL_SEC_PER_MIN * HZ;
    }
    if (uart_pkt_err_count == UART_DMD_MAX_PKT_ERR_CNT) {
        if (jiffies <= uart_pkt_err_end_time) {
            hw_1102a_bt_dsm_client_notify(DSM_1102A_UART_PKT_ERR, "%s: report dmd for uart pkt err\n", __FUNCTION__);
        }
        uart_pkt_err_start_time = 0;
        uart_pkt_err_count = 0;
    }
}

STATIC int32_t ps_check_msg_data_len(uint8_t rx_pkt_type, uint16_t rx_pkt_total_len, uint8_t *buf_ptr)
{
    switch (rx_pkt_type) {
        case BT_MSG:
            if (rx_pkt_total_len > BT_RX_MAX_FRAME) {
                ps_print_err("type=%d, the pkt len is ERR: 0x%x\n", rx_pkt_type, rx_pkt_total_len);
                print_uart_decode_param();
                return RX_PACKET_ERR;
            }
            break;
        case GNSS_FIRST_MSG:
        case GNSS_COMMON_MSG:
        case GNSS_LAST_MSG:
        case FM_FIRST_MSG:
        case FM_COMMON_MSG:
        case FM_LAST_MSG:
        case IR_FIRST_MSG:
        case IR_COMMON_MSG:
        case IR_LAST_MSG:
        case NFC_FIRST_MSG:
        case NFC_COMMON_MSG:
        case NFC_LAST_MSG:
        case OML_MSG:
            if (oal_atomic_read(&g_ir_only_mode) != 0) {
                break;  // 单红外消息不分包，不需要分包检查
            }
            if ((buf_ptr[PACKET_OFFSET_HEAD_INDEX] == PACKET_START_SIGNAL) &&
                (buf_ptr[PACKET_OFFSET_HEAD_NEXT_INDEX] == PACKET_RX_FUNC_LAST_WORDS)) {
                if (rx_pkt_total_len > MAX_LAST_WORD_FRAME_LEN) {
                    ps_print_err("type=%d, the pkt len is ERR: 0x%x,(max frame len is:0x%x)\n",
                                 rx_pkt_type, rx_pkt_total_len, MAX_LAST_WORD_FRAME_LEN);
                    print_uart_decode_param();
                    return RX_PACKET_ERR;
                }
            } else if (rx_pkt_total_len > MAX_NOBT_FRAME_LEN) {
                ps_print_err("type=%d, the pkt len is ERR: 0x%x,(max frame len is:0x%x)\n",
                             rx_pkt_type, rx_pkt_total_len, MAX_NOBT_FRAME_LEN);
                print_uart_decode_param();
                return RX_PACKET_ERR;
            }
            break;
        default:
            break;
    }

    return RX_PACKET_CORRECT;
}

STATIC int32_t ps_check_packet_head_start(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, int32_t count)
{
    if ((ps_core_d == NULL) || (buf_ptr == NULL)) {
        ps_print_err("ps_core_d or buf_ptr is NULL\n");
        return -EINVAL;
    }

    if (unlikely(*buf_ptr != PACKET_START_SIGNAL)) {
        dmd_report_uart_pkt_err();
        ps_print_err("pkt head is ERR:%x,count=[%d],buf_ptr=[%p]\n", *buf_ptr, count, buf_ptr);
        print_uart_decode_param();
        return RX_PACKET_ERR;
    }

    /* if count less then sys packet lenth and continue */
    if (count < SYS_TOTAL_PACKET_LENTH) {
            return RX_PACKET_CONTINUE;
    }

    return RX_PACKET_CORRECT;
}

/*
 * Prototype    : ps_check_packet_head_etc
 * Description  : check recived curr packet is or not a complete packet
 * input        : buf_ptr -> ptr of recived data buf
 *                count   -> size of recived data buf
 * output       : return 0 -> this packet is a complete packet
 *                return 1 -> this packet has not a complete packet,need continue recv
 *                return -1-> this packst is a err packet,del it
 */
int32_t ps_check_packet_head(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, int32_t count)
{
    uint16_t len;
    uint16_t lenbak;
    int32_t ret;
    uint8_t *ptr = buf_ptr;

    ret = ps_check_packet_head_start(ps_core_d, ptr, count);
    if (ret != RX_PACKET_CORRECT) {
        return ret;
    }

    ptr++;
    ps_core_d->rx_pkt_type = *ptr;
    /* check packet type */
    if (unlikely(ps_core_d->rx_pkt_type >= MSG_BUTT)) {
        ps_print_err("the pkt type is ERR: %x\n", ps_core_d->rx_pkt_type);
        print_uart_decode_param();
        return RX_PACKET_ERR;
    }

    ptr++;
    len = *ptr;
    ptr++;
    lenbak = *ptr;
    lenbak = lenbak * 0x100;
    len = len + lenbak;
    ps_core_d->rx_pkt_total_len = len;

    /* check packet lenth */
    if (unlikely(len < SYS_TOTAL_PACKET_LENTH || len > PUBLIC_BUF_MAX)) {
        ps_print_err("the pkt len is ERR: %x\n", ps_core_d->rx_pkt_total_len);
        ps_core_d->rx_pkt_total_len = 0;
        print_uart_decode_param();
        return RX_PACKET_ERR;
    }

    /* check all service frame length */
    if (unlikely(ps_check_msg_data_len(ps_core_d->rx_pkt_type,
                                       ps_core_d->rx_pkt_total_len, buf_ptr)) == RX_PACKET_ERR) {
        return RX_PACKET_ERR;
    }

    /* check whether the recv packet is CHR packet */
    if (ps_check_chr_packet(ps_core_d, buf_ptr) != 0) {
        return RX_PACKET_ERR;
    }

    /* the packet is breaked, need continue rx */
    if (ps_core_d->rx_pkt_total_len > count) {
        return RX_PACKET_CONTINUE;
    }

    ptr = buf_ptr + len - sizeof(struct ps_packet_end);
    /* the packet is a err packet */
    if (unlikely(*ptr != PACKET_END_SIGNAL)) {
        ps_print_err("the pkt end is ERR: %x\n", *ptr);
        print_uart_decode_param();
        return RX_PACKET_ERR;
    }

    return RX_PACKET_CORRECT;
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
        ps_print_warning("uart rx buf has data, rx_have_recv_pkt_len=%d\n", ps_core_d->rx_have_recv_pkt_len);
        ps_print_warning("uart rx buf has data, rx_have_del_public_len=%d\n", ps_core_d->rx_have_del_public_len);
        ps_print_warning("uart rx buf has data, rx_decode_public_buf_ptr=%p\n", ps_core_d->rx_decode_public_buf_ptr);
        ps_print_warning("uart rx buf has data, rx_public_buf_org_ptr=%p\n", ps_core_d->rx_public_buf_org_ptr);

        ul_dump_len = ps_core_d->rx_have_recv_pkt_len > UART_DUMP_RX_BUF_LENGTH ?
            UART_DUMP_RX_BUF_LENGTH : ps_core_d->rx_have_recv_pkt_len;
        for (i = 0; i < ul_dump_len; i++) {
            ps_print_warning("uart rx buf has data, data[%d]=0x%x\n", i, *(ps_core_d->rx_decode_public_buf_ptr + i));
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
        ps_print_warning("uart rx buf has data, rx_have_recv_pkt_len=%d\n", ps_core_d->rx_have_recv_pkt_len);
        ps_print_warning("uart rx buf has data, rx_have_del_public_len=%d\n", ps_core_d->rx_have_del_public_len);
        ps_print_warning("uart rx buf has data, rx_decode_public_buf_ptr=%p\n", ps_core_d->rx_decode_public_buf_ptr);
        ps_print_warning("uart rx buf has data, rx_public_buf_org_ptr=%p\n", ps_core_d->rx_public_buf_org_ptr);
        for (i = 0; i < ps_core_d->rx_have_recv_pkt_len; i++) {
            ps_print_warning("uart rx buf has data, data[%d]=0x%x\n", i, *(ps_core_d->rx_decode_public_buf_ptr + i));
        }
        g_reset_cnt++;
        ps_print_warning("reset uart rx buf, reset cnt=%d\n", g_reset_cnt);
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
        ps_print_err(" disc_data is null \n");
        return 0;
    }

    if (unlikely(data == NULL)) {
        ps_print_err(" received null from TTY \n");
        return 0;
    }

    if (count < 0) {
        ps_print_err(" received count from TTY err\n");
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
 */
int32 ps_core_recv(void *disc_data, const uint8 *data, int32 count)
{
    struct ps_core_s *ps_core_d = NULL;
    uint8 *ptr = NULL;
    int32 count1 = 0;
    int32 ret;

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
                ps_print_warning("received count too large! , memcopy_s error, destlen=%d, srclen=%d\n ",
                                 PUBLIC_BUF_MAX - ps_core_d->rx_have_recv_pkt_len, count);
                continue;
            }
            ps_core_d->rx_have_recv_pkt_len = ps_core_d->rx_have_recv_pkt_len + count;
            count = 0;
            /* if count less then sys packet lenth and continue */
            if (ps_core_d->rx_have_recv_pkt_len < SYS_TOTAL_PACKET_LENTH) { /* not decode,and continue rxd data */
                continue;
            }
            /* decode public buf data */ /* find needs decode data in buf */
            while (ps_core_d->rx_have_del_public_len < ps_core_d->rx_have_recv_pkt_len) {
                count1 = ps_core_d->rx_have_recv_pkt_len - ps_core_d->rx_have_del_public_len;
                /* check curr data */
                ret = ps_check_packet_head(ps_core_d, ps_core_d->rx_decode_public_buf_ptr, count1);
                if (ret == RX_PACKET_CORRECT) { /* this packet is correct, and push it to proto buf */
                    ps_decode_packet(ps_core_d, ps_core_d->rx_decode_public_buf_ptr);
                    ps_core_d->rx_have_del_public_len += ps_core_d->rx_pkt_total_len;
                    ps_core_d->rx_decode_public_buf_ptr += ps_core_d->rx_pkt_total_len;
                    ps_core_d->rx_pkt_total_len = 0;
                    continue;
                } else if (ret == RX_PACKET_CONTINUE) {
                    /* copy breaked pkt to buf from start */
                    if (unlikely(memmove_s(ps_core_d->rx_public_buf_org_ptr, PUBLIC_BUF_MAX,
                                           ps_core_d->rx_decode_public_buf_ptr, count1) != EOK)) {
                        ps_clear_rx_buf(ps_core_d);
                        ps_print_err("memmove_s error, destlen=%d, srclen=%d\n ", PUBLIC_BUF_MAX, count1);
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
            /* have decode all public buf data, reset ptr and lenth */
            if (ps_core_d->rx_have_del_public_len == ps_core_d->rx_have_recv_pkt_len) {
                ps_clear_rx_buf(ps_core_d);
            }
        } else { /* if not have recv data, direct decode data in uart buf */
            ret = ps_check_packet_head(ps_core_d, ps_core_d->rx_decode_tty_ptr, count);
            if (ret == RX_PACKET_CORRECT) { /* this packet is correct, and push it to proto buf */
                ps_decode_packet(ps_core_d, ps_core_d->rx_decode_tty_ptr);
                ps_core_d->rx_decode_tty_ptr = ps_core_d->rx_decode_tty_ptr + ps_core_d->rx_pkt_total_len;
                count = count - ps_core_d->rx_pkt_total_len;
                ps_core_d->rx_pkt_total_len = 0;
                continue;
            } else if (ret == RX_PACKET_CONTINUE) { /* this packet is not completed,need continue rx */
                ptr = ps_core_d->rx_public_buf_org_ptr;
                /* copy data to public buf,and continue rx */
                if (unlikely(memcpy_s(ptr, PUBLIC_BUF_MAX, ps_core_d->rx_decode_tty_ptr, count) != EOK)) {
                    ps_clear_rx_buf(ps_core_d);
                    ps_print_err("memcpy_s error, destlen=%d, srclen=%d\n ", PUBLIC_BUF_MAX, count);
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
 */
int32 ps_skb_enqueue(struct ps_core_s *ps_core_d, struct sk_buff *skb, uint8 type)
{
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err(" ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (unlikely(skb == NULL)) {
        ps_print_err(" skb is NULL\n");
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
            ps_print_err("queue type is error, type=%d\n", type);
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
        ps_print_err("ps_core_d is NULL\n");
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
            ps_print_err("queue type is error, type=%d\n", type);
            break;
    }

    return curr_skb;
}


static struct sk_buff *ps_skb_dequeue_core(struct ps_core_s *ps_core_d, uint32 *dequeue_init_flag)
{
    struct sk_buff *skb = NULL;
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

STATIC int32 ps_core_tx_para_check(struct ps_core_s *ps_core_d, struct pm_drv_data **pm_data)
{
    *pm_data = pm_get_drvdata();
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (unlikely(*pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -EFAULT;
    }
    return 0;
}

/*
 * Prototype    : ps_core_tx_attemper_etc
 * Description  : adj tx skb buf data to tty, seq BT and FM or GNSS.
 */
int32 ps_core_tx_attemper(struct ps_core_s *ps_core_d)
{
    struct sk_buff *skb = NULL;
    int32 len, ret;
    uint8 uart_state = UART_NOT_READY;
    struct pm_drv_data *pm_data = NULL;
    uint64 flags;
    uint32 dequeue_init_flag = 1;

    PS_PRINT_FUNCTION_NAME;

    ret = ps_core_tx_para_check(ps_core_d, &pm_data);
    if (ret < 0) {
        return ret;
    }

    ps_print_dbg("enter tx work\n");

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
            ps_print_dbg("use tty start to send data,skb->len=[%d]\n", skb->len);
        }

        while (skb->len) {
            if (atomic_read(&ps_core_d->force_tx_exit) > 0) {
                ps_print_err("tty tx work abort for tty need release\n");
                kfree_skb(skb);
                return 0;
            }
            /* len is have tx to uart byte number */
            len = ps_write_tty(ps_core_d, skb->data, skb->len);
            if (len < 0) {
                ps_print_err("tty have not opened!\n");
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
 */
void ps_core_tx_work(struct work_struct *work)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    /* makesure tty is avilable. release_tty_drv race access tty */
    mutex_lock(&g_ps_tty_mutex);
    if (ps_core_d->tty != NULL) {
        /* into tx skb buff queue */
        ps_core_tx_attemper(ps_core_d);
    }
    mutex_unlock(&g_ps_tty_mutex);

    return;
}

/*
 * Prototype    : ps_add_packet_head
 * Description  : add packet head to recv buf from hal or bt driver.
 * input        : buf  -> ptr of buf
 *                type -> packet type，example bt,fm,or gnss
 *                lenth-> packet length
 */
int32 ps_add_packet_head(uint8 *buf, uint8 type, uint16 lenth)
{
    int8 *ptr = NULL;
    uint16 len;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    ptr = buf;
    *ptr = PACKET_START_SIGNAL;
    ptr++;
    *ptr = type;
    len = lenth;
    ptr++;
    *ptr = len_low_byte(len);
    ptr++;
    *ptr = len_high_byte(len);

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
 */
int32 ps_set_sys_packet(uint8 *buf, uint8 type, uint8 content)
{
    int8 *ptr = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("ps_core_d is NULL\n");
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
 * 返 回 值  : 0为成功，小于0为失败
 */
int32 ps_set_sys_variable_length_packet(uint8 *buf, uint8 type, const uint8 *content, uint16 len)
{
    int8 *ptr = NULL;
    uint16 content_len;
    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("skb data buf is NULL\n");
        return -EINVAL;
    }

    if ((len > SYS_VARIABLE_LENGTH_PACKET_MAX_LEN) || (len < SYS_VARIABLE_LENGTH_PACKET_MIN_LEN)) {
        ps_print_err(
            "sys variable-length packet len wrong,should not less than %d or more than %d,pls check len=[%d]\n",
            SYS_VARIABLE_LENGTH_PACKET_MIN_LEN, SYS_VARIABLE_LENGTH_PACKET_MAX_LEN, len);
        return -EINVAL;
    }

    content_len = len - SYS_VARIABLE_LENGTH_PACKET_MIN_LEN;

    /*
     * | 7e | 00 |len | 00 | ff |    |    |    |            ... ...            | 7e |
     * |  sys packet head  |sys variable-length|sys variable-length packet data|sys packet end
     * |       4byte       | packet head 4byte | (x)byte [gnss timesync 8byte] |     1byte
     */
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
        ps_print_err("variable length cmd send failed\n");
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
    bool ret = false;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb = alloc_skb(SYS_TOTAL_PACKET_LENTH, oal_in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        ps_print_err("can't allocate mem for new sys skb, len=%d\n", SYS_TOTAL_PACKET_LENTH);
        return -EINVAL;
    }

    skb_put(skb, SYS_TOTAL_PACKET_LENTH);
    /* set sys packet head to skb */
    ps_set_sys_packet(skb->data, type, content);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    ret = queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
    if (type == SYS_MSG) {
        ps_print_info("queue sys msg work, ret = %d, type = %d\n", ret, content);
    }

    return 0;
}

/*
 * 函 数 名  : ps_tx_sys_variable_length_cmd
 * 功能描述  : 发送系统可变长消息接口
 * 输入参数  : 系统消息类型、消息内容和其长度
 * 返 回 值  : 0为成功，小于0为失败
 */
int32 ps_tx_sys_variable_length_cmd(struct ps_core_s *ps_core_d, uint8 type, const uint8 *content, uint16 len)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    bool ret = false;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (content == NULL) {
        ps_print_err("user data buf is NULL\n");
        return -EINVAL;
    }

    tx_skb_len = len + SYS_VARIABLE_LENGTH_PACKET_MIN_LEN;

    skb = alloc_skb(tx_skb_len, oal_in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        ps_print_err("can't allocate mem for new variable length sys skb, len=%d\n", tx_skb_len);
        return -EINVAL;
    }

    skb_put(skb, tx_skb_len);

    /* set sys packet head to skb */
    ps_set_sys_variable_length_packet(skb->data, type, content, tx_skb_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    ret = queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
    ps_print_info("queue sys variable-length msg work, ret = %d\n", ret);

    /* lint -save -e670 -specific(-e670) */ /* 屏蔽skb未释放告警 */
    return 0;
}

int32 ps_tx_urgent_cmd(struct ps_core_s *ps_core_d, uint8 type, uint8 content)
{
    struct sk_buff *skb = NULL;
    bool ret = false;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    skb = alloc_skb(SYS_TOTAL_PACKET_LENTH, oal_in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        ps_print_err("can't allocate mem for new sys skb, len=%d\n", SYS_TOTAL_PACKET_LENTH);
        return -EINVAL;
    }

    skb_put(skb, SYS_TOTAL_PACKET_LENTH);
    /* set sys packet head to skb */
    ps_set_sys_packet(skb->data, type, content);
    ps_skb_enqueue(ps_core_d, skb, TX_URGENT_QUEUE);
    ret = queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
    if (type == SYS_MSG) {
        ps_print_info("queue sys msg work, ret = %d, type = %d\n", ret, content);
    }

    return 0;
}

/*
 * Prototype    : ps_tx_fmbuf
 * Description  : add packet head to recv ir data buff from hal,and tx to tty uart
 */
int32 ps_tx_fmbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    uint16 tx_fm_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
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
        if (skb == NULL) {
            ps_print_err("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > FM_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, FM_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, FM_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, FM_LAST_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_fm_len)) {
            ps_print_err("can't copy_from_user for ir\n");
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
 */
int32 ps_tx_irbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    uint16 tx_ir_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
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
        if (skb == NULL) {
            ps_print_err("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > IR_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, IR_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, IR_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, IR_LAST_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_ir_len)) {
            ps_print_err("can't copy_from_user for ir\n");
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
 */
int32 ps_tx_nfcbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16 tx_skb_len;
    uint16 tx_nfc_len;
    uint8 start = 0;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
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
        if (skb == NULL) {
            ps_print_err("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > NFC_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start gnss packet */
                ps_add_packet_head(skb->data, NFC_FIRST_MSG, tx_skb_len);
                start = true;
            } else { /* this is a int gnss packet */
                ps_add_packet_head(skb->data, NFC_COMMON_MSG, tx_skb_len);
            }
        } else { /* this is the last gnss packet */
            ps_add_packet_head(skb->data, NFC_LAST_MSG, tx_skb_len);
        }

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_nfc_len)) {
            ps_print_err("can't copy_from_user for nfc\n");
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
 */
int32 ps_tx_gnssbuf(struct ps_core_s *ps_core_d, const int8 __user *buf, size_t count)
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

        if (copy_from_user(&skb->data[sizeof(struct ps_packet_head)], buf, tx_gnss_len)) {
            ps_print_err("can't copy_from_user for gnss\n");
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

STATIC int32 ps_core_kzalloc_check(struct ps_core_s **ps_core_d, uint8 **ptr)
{
    *ps_core_d = kzalloc(sizeof(struct ps_core_s), GFP_KERNEL);
    if (*ps_core_d == NULL) {
        ps_print_err("memory allocation failed\n");
        return -ENOMEM;
    }

    *ptr = kzalloc(PUBLIC_BUF_MAX, GFP_KERNEL);
    if (*ptr == NULL) {
        kfree(*ps_core_d);
        ps_print_err("no mem to allocate to public buf!\n");
        return -ENOMEM;
    }

    return 0;
}

/*
 * Prototype    : ps_core_init
 * Description  : init ps_core_d struct and kzalloc memery
 */
int32 ps_core_init(struct ps_core_s **core_data)
{
    struct ps_core_s *ps_core_d = NULL;
    struct ps_pm_s *ps_pm_d = NULL;
    uint8 *ptr = NULL;
    int32 err;
    int32 i = 0;

    PS_PRINT_FUNCTION_NAME;

    if (ps_core_kzalloc_check(&ps_core_d, &ptr) < 0) {
        return -ENOMEM;
    }

    memset_s(ptr, PUBLIC_BUF_MAX, 0, PUBLIC_BUF_MAX);
    ps_core_d->rx_public_buf_org_ptr = ptr;
    ps_core_d->rx_decode_public_buf_ptr = ptr;

    ps_pm_d = kzalloc(sizeof(struct ps_pm_s), GFP_KERNEL);
    if (ps_pm_d == NULL) {
        ps_print_err("ps_pm_d memory allocation failed\n");
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
        ps_print_err("error registering ps_pm err = %d\n", err);
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
        ps_print_err("error registering %d line discipline %d\n", N_HW_BFG, err);
#endif
        return -EFAULT;
    }

    /*
     * Initialize PS tx and rx queue head. BT module skb's
     * will be pushed in this queue for actual transmission.
     */
    skb_queue_head_init(&ps_core_d->tx_urgent_seq);
    skb_queue_head_init(&ps_core_d->tx_high_seq);
    skb_queue_head_init(&ps_core_d->tx_low_seq);
    skb_queue_head_init(&ps_core_d->rx_dbg_seq);
    atomic_set(&ps_core_d->force_tx_exit, 0);

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
            ps_print_err("unable to un-register ldisc %d\n", err);
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
