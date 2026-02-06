

/* 头文件包含 */
#include "bfgx_data_parse.h"

#include "plat_debug.h"
#include "plat_uart.h"
#include "bfgx_dev.h"
#include "plat_parse_changid.h"
#include "bfgx_platform_msg_handle.h"
#include "bfgx_exception_rst.h"
#include "plat_pm.h"
#include "plat_cali.h"
#include "securec.h"
#include "oam_ext_if.h"
#include "chr_user.h"

/* function pointer for rx data */
int32_t (*st_tty_recv)(void *, const uint8_t *, int32_t);
int32_t (*me_tty_recv)(void *, const uint8_t *, int32_t);

uint32_t g_bfgx_rx_max_frame[BFGX_BUTT] = {
    BT_RX_MAX_FRAME,
    FM_RX_MAX_FRAME,
    GNSS_RX_MAX_FRAME,
    IR_RX_MAX_FRAME,
    NFC_RX_MAX_FRAME,
};

uint32_t g_bfgx_rx_queue[BFGX_BUTT] = {
    RX_BT_QUEUE,
    RX_FM_QUEUE,
    RX_GNSS_QUEUE,
    RX_IR_QUEUE,
    RX_NFC_QUEUE,
};

STATIC uint32_t g_bfgx_rx_queue_max_num[BFGX_BUTT] = {
    RX_BT_QUE_MAX_NUM,
    RX_FM_QUE_MAX_NUM,
    RX_GNSS_QUE_MAX_NUM,
    RX_IR_QUE_MAX_NUM,
    RX_NFC_QUE_MAX_NUM,
};

STATIC struct ps_tx_manage_info g_ps_tx_info[BFGX_BUTT] = {
    {PKG_NOT_SEPRETED, TX_HIGH_QUEUE, BT_MSG, BT_MSG, BT_MSG},
    {PKG_SEPRETED, TX_LOW_QUEUE, FM_FIRST_MSG, FM_COMMON_MSG, FM_LAST_MSG},
    {PKG_SEPRETED, TX_LOW_QUEUE, GNSS_FIRST_MSG, GNSS_COMMON_MSG, GNSS_LAST_MSG},
    {PKG_SEPRETED, TX_LOW_QUEUE, IR_FIRST_MSG, IR_COMMON_MSG, IR_LAST_MSG},
    {PKG_SEPRETED, TX_LOW_QUEUE, NFC_FIRST_MSG, NFC_COMMON_MSG, NFC_LAST_MSG},
};

typedef int32_t (*ps_core_msg_handler)(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_no_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_debug_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_mem_dump_size_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_mem_dump_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);
STATIC int32_t ps_recv_bfgx_cali_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);

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
    NULL,                        // NFC_FIRST_MSG = 0x0B,   NFC串口消息，第一个分段消息
    NULL,                        // NFC_COMMON_MSG = 0x0C,  NFC串口消息，中间分段消息
    NULL,                        // NFC_LAST_MSG = 0x0D,    NFC串口消息，最后一个分段消息
    ps_recv_debug_data,          // OML_MSG = 0x0E,         可维可测消息
    ps_recv_mem_dump_size_data,  // MEM_DUMP_SIZE = 0x0f,   bfgx异常时，要dump的mem长度消息
    ps_recv_mem_dump_data,       // MEM_DUMP = 0x10,        bfgx异常时，内存dump消息
    NULL,                        // WIFI_MEM_DUMP = 0x11,   UART READ WIFI MEM，内存dump消息
    ps_recv_bfgx_cali_data,      // BFGX_CALI_MSG = 0x12,   BFGX 校准数据上报
};

STATIC void print_uart_decode_param(struct ps_core_s *ps_core_d)
{
    ps_print_warning("[%s]rx_pkt_type=%x,rx_have_recv_pkt_len=%d,rx_pkt_total_len=%d,rx_have_del_public_len=%d\n",
                     index2name(ps_core_d->pm_data->index), ps_core_d->rx_pkt_type, ps_core_d->rx_have_recv_pkt_len,
                     ps_core_d->rx_pkt_total_len, ps_core_d->rx_have_del_public_len);
    ps_print_warning("[%s]rx_decode_tty_ptr=%p,rx_decode_public_buf_ptr=%p,rx_public_buf_org_ptr=%p\n",
                     index2name(ps_core_d->pm_data->index), ps_core_d->rx_decode_tty_ptr,
                     ps_core_d->rx_decode_public_buf_ptr, ps_core_d->rx_public_buf_org_ptr);
    return;
}

/*
 * Prototype    : ps_write_tty
 * Description  : can be called when write data to uart
 *                This is the internal write function - a wrapper
 *                to tty->ops->write
 * input        : data -> the ptr of send data buf
 *                count -> the size of send data buf
 */
int32_t ps_write_tty(struct ps_core_s *ps_core_d, const uint8_t *data, int32_t count)
{
    int32_t tty_write_cnt;
    struct tty_struct *tty = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely((ps_core_d == NULL) || (ps_core_d->tty == NULL))) {
        ps_print_err("tty unavailable to perform write\n");
        return -EINVAL;
    }

    tty = ps_core_d->tty;
    tty_write_cnt = tty->ops->write(tty, data, count);
    if (tty_write_cnt > 0) {
        ps_tty_tx_cnt_add(ps_core_d, tty_write_cnt);
    }

    return tty_write_cnt;
}

/*
 * Prototype    : ps_push_skb_queue
 * Description  : alloc skb buf,and memcopy data to skb buff
 * input        : buf_ptr -> ptr of recived data buf
 */
int32_t ps_push_skb_queue(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint16_t pkt_len, uint8_t type)
{
    struct sk_buff *skb = NULL;
    int32_t ret;

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

STATIC void delete_gnss_head_skb_msg(struct ps_core_s *ps_core_d)
{
    struct sk_buff *skb = NULL;
    uint8_t seperate_tag;

    do {
        if ((skb = ps_skb_dequeue(ps_core_d, RX_GNSS_QUEUE)) == NULL) {
            ps_print_warning("gnss skb rx queue is empty\n");
            return;
        }

        ps_print_warning("[%s]gnss delete msg, skb->len=%d, qlen=%d\n", index2name(ps_core_d->pm_data->index),
                         skb->len, ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen);

        seperate_tag = skb->data[skb->len - 1];
        if ((seperate_tag != GNSS_SEPER_TAG_INIT) && (seperate_tag != GNSS_SEPER_TAG_LAST)) {
            ps_print_err("seperate_tag=%x not support\n", seperate_tag);
            seperate_tag = GNSS_SEPER_TAG_INIT;
        }
        kfree_skb(skb);
    } while (seperate_tag == GNSS_SEPER_TAG_INIT);
}

STATIC void delete_gnss_tail_skb_msg(struct ps_core_s *ps_core_d, int32_t msg_len)
{
    struct sk_buff *skb = NULL;
    int32_t delet_len = msg_len;

    ps_print_warning("delete gnss tail rx data, len=%d!\n", delet_len);

    while (delet_len > 0) {
        if ((skb = skb_dequeue_tail(&ps_core_d->bfgx_info[BFGX_GNSS].rx_queue)) == NULL) {
            ps_print_warning("gnss rx queue has no data!\n");
            return;
        }

        ps_print_warning("drop gnss tail seperate data, tag:%x, skb->len=%x\n", skb->data[skb->len - 1], skb->len);
        kfree_skb(skb);
        delet_len -= (GNSS_RX_SEPERATE_FRAME_MAX_LEN - 1);
    }
}

STATIC int32_t ps_push_skb_queue_gnss(struct ps_core_s *ps_core_d, uint8_t *buf, uint16_t pkt_len, uint8_t type)
{
    struct sk_buff *skb = NULL;
    uint8_t seperate_tag;
    uint16_t seperate_len;
    int32_t copy_cnt;
    int32_t ret;

    if (unlikely(buf == NULL)) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }

    oal_spin_lock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);
    copy_cnt = 0;
    do {
        if ((pkt_len - copy_cnt) < GNSS_RX_SEPERATE_FRAME_MAX_LEN) {
            seperate_tag = GNSS_SEPER_TAG_LAST;
            seperate_len = pkt_len - copy_cnt + 1;
        } else {
            seperate_tag = GNSS_SEPER_TAG_INIT;
            seperate_len = GNSS_RX_SEPERATE_FRAME_MAX_LEN;
        }

        skb = alloc_skb(seperate_len, GFP_ATOMIC);
        if (unlikely(skb == NULL)) {
            ps_print_err("alloc skb fail, tag:%x, slen=%x, pkt_len=%x\n", seperate_tag, seperate_len, pkt_len);
            delete_gnss_tail_skb_msg(ps_core_d, copy_cnt);
            oal_spin_unlock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);
            return -EINVAL;
        }

        skb_put(skb, seperate_len);
        ret = memcpy_s(skb->data, seperate_len, buf + copy_cnt, seperate_len - 1);
        if (unlikely(ret != EOK)) {
            ps_print_err("memcpy_s failed\n");
            kfree_skb(skb);
            oal_spin_unlock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);
            return -EINVAL;
        }
        skb->data[seperate_len - 1] = seperate_tag;
        ps_skb_enqueue(ps_core_d, skb, type);
        copy_cnt += (seperate_len - 1);
    } while (copy_cnt < pkt_len);

    /* 确保gnss缓存的skb不超过(RX_GNSS_QUE_MAX_NUM >> 1)个，防止一会删前面的，一会删后面的 */
    while (ps_core_d->bfgx_info[BFGX_GNSS].rx_queue.qlen > (RX_GNSS_QUE_MAX_NUM >> 1)) {
        delete_gnss_head_skb_msg(ps_core_d);
    }

    oal_spin_unlock(&ps_core_d->bfgx_info[BFGX_GNSS].rx_lock);
    return 0;
}

/*
 * Prototype    : ps_push_skb_debug_queue
 * Description  : alloc skb buf,and memcopy data to skb buff
 * input        : buf_ptr -> ptr of recived data buf
 */
STATIC int32_t ps_push_skb_debug_queue(
    struct ps_core_s *ps_core_d, const uint8_t *buf_ptr, uint16_t pkt_len, uint8_t type)
{
    struct sk_buff *skb = NULL;
    uint16_t count;

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

STATIC int32_t ps_recv_mem_dump_size_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint32_t dump_mem_size;
    uint16_t rx_pkt_total_len;

    if (!ps_is_device_log_enable()) {
        return 0;
    }

    dump_mem_size = *(uint32_t *)buf_ptr;
    ps_print_info("[%s]prepare to recv bfgx mem size [%d]!\n", index2name(ps_core_d->pm_data->index), dump_mem_size);

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    bfgx_notice_hal_memdump(ps_core_d);
    bfgx_memdump_enquenue(ps_core_d, buf_ptr, rx_pkt_total_len);

    return 0;
}

STATIC int32_t ps_recv_mem_dump_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint16_t rx_pkt_total_len;

    if (!ps_is_device_log_enable()) {
        return 0;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - PS_PKG_SIZE;
    if (rx_pkt_total_len <= MEM_DUMP_RX_MAX_FRAME) {
        ps_print_dbg("[%s]recv bfgx stack size [%d]!\n", index2name(ps_core_d->pm_data->index), rx_pkt_total_len);
        bfgx_memdump_enquenue(ps_core_d, buf_ptr, rx_pkt_total_len);
    }

    return 0;
}

#define BT_CALI_TX_PATH_CALI_HIGH_RESIDUAL_FLAG BIT31
STATIC int32_t ps_recv_bfgx_cali_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint16_t rx_pkt_total_len;
    uint8_t *cali_data_buf = NULL;
    uint32_t cali_data_len = 0;
    uint32_t copy_len;
    uint32_t bt_fem_sta;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }

    cali_data_buf = bfgx_get_cali_data_buf(&cali_data_len);
    if (cali_data_buf == NULL) {
        ps_print_err("bfgx cali data buf is NULL\n");
        return -EINVAL;
    }

    bt_fem_sta = (*(uint32_t *)cali_data_buf) & BT_CALI_TX_PATH_CALI_HIGH_RESIDUAL_FLAG;
    if (bt_fem_sta) {
        oam_error_log1(0, 0, "ps_recv_bfgx_cali_data::bt_fem error,status[%d]", bt_fem_sta);
#ifdef CONFIG_HUAWEI_DSM
        /* bt_fem dmd report */
        hw_110x_dsm_client_notify(SYSTEM_TYPE_BT, DSM_BT_FEMERROR, "%s: bt_fem error, status[%d]",
                                  __FUNCTION__, bt_fem_sta);
#endif
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
    copy_len = (rx_pkt_total_len > cali_data_len) ? cali_data_len : rx_pkt_total_len;

    ps_print_info("[%s]recv bfgx cali data, rx_len=%d,copy_len=%d\n",
                  index2name(ps_core_d->pm_data->index), rx_pkt_total_len, copy_len);
    if (unlikely(memcpy_s(cali_data_buf, cali_data_len, buf_ptr, copy_len) != EOK)) {
        ps_print_err("memcopy_s error, destlen=%d, srclen=%d\n ", cali_data_len, copy_len);
    }

    complete(&g_cali_recv_done);

    return 0;
}

STATIC int32_t ps_recv_uart_loop_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    int32_t ret;
    uint16_t rx_pkt_total_len;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }

    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);

    if (rx_pkt_total_len <= UART_LOOP_MAX_PKT_LEN) {
        ret = uart_loop_test_recv_pkt(ps_core_d, buf_ptr, rx_pkt_total_len);
        if (ret < 0) {
            ps_print_err("[%s]uart_loop_test_recv_pkt failed\n", index2name(ps_core_d->pm_data->index));
            return -EINVAL;
        }
    } else {
        ps_print_err("[%s]uart loop test, recv pkt len is too large!\n", index2name(ps_core_d->pm_data->index));
        return -EINVAL;
    }

    return 0;
}

STATIC int32_t ps_push_device_log_to_queue(struct ps_core_s *ps_core_d, const uint8_t *buf_ptr, uint16_t len)
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
    oal_wait_queue_wake_up_interrupt(&ps_core_d->rx_dbg_wait);
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
 */
STATIC int32_t ps_recv_debug_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint16_t rx_pkt_total_len;
    uint16_t dbg_pkt_lenth;
    struct st_exception_info *pst_exception_data = NULL;

    ps_core_d->rx_pkt_oml++;
    rx_pkt_total_len = ps_core_d->rx_pkt_total_len - PS_PKG_SIZE;

    if ((buf_ptr[START_SIGNAL_LAST_WORDS] == PACKET_START_SIGNAL) &&
        (buf_ptr[PACKET_RX_FUNC_INDEX_LAST_WORDS] == PACKET_RX_FUNC_LAST_WORDS)) {
        /* get FrameLen 2 bytes */
        dbg_pkt_lenth = *(uint16_t *)&buf_ptr[PACKET_FRAMELEN_INDEX];
        if ((dbg_pkt_lenth == rx_pkt_total_len) && (dbg_pkt_lenth == LAST_WORD_LEN) &&
            (buf_ptr[RPT_IND_INDEX_LAST_WORDS] == PACKET_RX_RPT_IND_LAST_WORDS)) {
            ps_print_err("[%s]recv device last words!Faulttype=0x%x,FaultReason=0x%x,PC=0x%x,LR=0x%x\n",
                         index2name(ps_core_d->pm_data->index),
                         *(uint32_t *)&buf_ptr[FAULT_TYPE_INDEX_LAST_WORDS],
                         *(uint32_t *)&buf_ptr[FAULT_REASON_INDEX_LAST_WORDS],
                         *(uint32_t *)&buf_ptr[PC_INDEX_LAST_WORDS],
                         *(uint32_t *)&buf_ptr[LR_INDEX_LAST_WORDS]);

            plat_changid_print();
            declare_dft_trace_key_info("bfgx_device_panic", OAL_DFT_TRACE_EXCEP);

            pst_exception_data = get_exception_info_reference();
            if (pst_exception_data != NULL) {
                atomic_set(&pst_exception_data->is_memdump_runing, 1);
            }

            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_DEV, CHR_PLAT_DRV_ERROR_BFG_DEV_PANIC);

            plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, BFGX_LASTWORD_PANIC);
        } else {
            ps_print_warning("[%s]recv wrong last words,[%x %x], pkg len:%d\n", index2name(ps_core_d->pm_data->index),
                             *(uint32_t *)&buf_ptr[START_SIGNAL_LAST_WORDS],
                             *(uint32_t *)&buf_ptr[PACKET_FRAMELEN_INDEX],
                             rx_pkt_total_len);
        }
    }

    return ps_push_device_log_to_queue(ps_core_d, buf_ptr, rx_pkt_total_len);
}

/*
 * Prototype    : ps_store_rx_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 */
STATIC int32_t ps_store_rx_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint8_t subsys)
{
    uint16_t rx_current_pkt_len;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;

    pst_sepreted_data = &ps_core_d->bfgx_info[subsys].sepreted_rx;

    rx_current_pkt_len = ps_core_d->rx_pkt_total_len - PS_PKG_SIZE;
    pst_sepreted_data->rx_pkt_len = rx_current_pkt_len;

    if (unlikely((uint32_t)(pst_sepreted_data->rx_buf_all_len + rx_current_pkt_len) > g_bfgx_rx_max_frame[subsys])) {
        ps_print_err("[%s]rx_current_pkt_len=%d,rx_pkt_total_len=%d,rx_buf_all_len=%d,subsys=%d\n",
                     index2name(ps_core_d->pm_data->index),
                     pst_sepreted_data->rx_pkt_len,
                     ps_core_d->rx_pkt_total_len,
                     pst_sepreted_data->rx_buf_all_len,
                     subsys);
        return RX_PACKET_ERR;
    }

    if (unlikely(pst_sepreted_data->rx_buf_ptr == NULL)) {
        ps_print_err("sepreted rx_buf_ptr is NULL\n");
        return RX_PACKET_ERR;
    }

    if (unlikely(memcpy_s(pst_sepreted_data->rx_buf_ptr, g_bfgx_rx_max_frame[subsys] -
                          pst_sepreted_data->rx_buf_all_len, buf_ptr, rx_current_pkt_len) != EOK)) {
        ps_print_err("dis space is not enough\n ");
        return RX_PACKET_ERR;
    }

    pst_sepreted_data->rx_buf_all_len += rx_current_pkt_len;
    pst_sepreted_data->rx_buf_ptr += rx_current_pkt_len;

    return RX_PACKET_CORRECT;
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
        ps_print_warning("[%s]%s rx queue too large! qlen=%d\n", index2name(ps_core_d->pm_data->index),
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
        ps_print_err("[%s]push %s rx data to skb failed!\n",
                     index2name(ps_core_d->pm_data->index), g_bfgx_subsys_name[subsys]);
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
    uint8_t sepreted_type;
    int32_t seq_correct;
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
        ps_print_err("[%s]%s rx seq is err! sepreted_type=%d, rx_prev_seq=%d\n",
                     index2name(ps_core_d->pm_data->index),
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
        ps_print_err("[%s]%s rx data lenth err! give up this total pkt\n",
                     index2name(ps_core_d->pm_data->index), g_bfgx_subsys_name[subsys]);
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
 *                memcpy recive data to mem buf
 * input        : buf_ptr -> ptr of recived data buf
 */
STATIC int32_t ps_recv_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    int32_t ret;
    const uint32_t ul_max_dump_len = 32;
    struct pm_drv_data *pm_data = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct bfgx_sepreted_rx_st *pst_sepreted_data = NULL;
    uint16_t len;
    uint8_t subsys;

    subsys = ps_get_sepreated_subsys_type(ps_core_d);
    if (subsys >= BFGX_BUTT) {
        ps_print_err("sepreted subsys error, subsys=%d\n", subsys);
        return -EINVAL;
    }

    pm_data = ps_core_d->pm_data;

    if (g_uart_loop_test_cfg.uart_loop_enable) {
        ps_recv_uart_loop_data(ps_core_d, buf_ptr);
        return 0;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];
    pst_bfgx_data->rx_pkt_num++;
    pst_sepreted_data = &pst_bfgx_data->sepreted_rx;

    oal_spin_lock(&pst_sepreted_data->sepreted_rx_lock);
    if (atomic_read(&pst_bfgx_data->subsys_state) != POWER_STATE_OPEN) {
        len = ps_core_d->rx_pkt_total_len - PS_PKG_SIZE;
        ps_print_warning("%s has closed, no need to recv data, len is %d\n", g_bfgx_subsys_name[subsys], len);
        oal_print_hex_dump(buf_ptr, (len < ul_max_dump_len ? len : ul_max_dump_len), HEX_DUMP_GROUP_SIZE, "rec data :");
        oal_spin_unlock(&pst_sepreted_data->sepreted_rx_lock);
        return 0;
    }

    if (subsys == BFGX_GNSS) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
        oal_wake_lock_timeout(&pm_data->gnss_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);
    }

    ret = ps_sepreated_data_recv(ps_core_d, buf_ptr, subsys);

    oal_spin_unlock(&pst_sepreted_data->sepreted_rx_lock);

    return ret;
}

// serial_data_type limit
#define BT_PACKET_TYPE_MIN 2
#define BT_PACKET_TYPE_MAX 4

/*
 * Prototype    : ps_recv_no_sepreated_data
 * Description  : called by core when recive gnss data from device,
 *                memcpy recive data to mem buf
 *                buf_ptr -> ptr of recived data buf
 * output       : return 0 -> have finish
 */
STATIC int32_t ps_recv_no_sepreated_data(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    int32_t ret;
    uint16_t rx_pkt_total_len = 0;
    struct st_bfgx_data *pst_bfgx_data = NULL;
    struct pm_drv_data *pm_data = NULL;

    pm_data = ps_core_d->pm_data;

    // check whether bt package type is right or not
    if ((*buf_ptr < BT_PACKET_TYPE_MIN) || (*buf_ptr > BT_PACKET_TYPE_MAX)) {
        ps_print_err("Wrong package type [%d] in BT data!\n", *buf_ptr);
    }

    oal_wake_lock_timeout(&pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    pst_bfgx_data = &ps_core_d->bfgx_info[BFGX_BT];
    pst_bfgx_data->rx_pkt_num++;

    /* 如果已经缓存的数据到达了最大值，则新来的数据被丢弃 */
    if (pst_bfgx_data->rx_queue.qlen >= g_bfgx_rx_queue_max_num[BFGX_BT]) {
        ps_print_warning("[%s]%s rx queue too large! qlen=%d\n", index2name(ps_core_d->pm_data->index),
                         g_bfgx_subsys_name[BFGX_BT], pst_bfgx_data->rx_queue.qlen);
        oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);
        return -EINVAL;
    }

    if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_OPEN) {
        rx_pkt_total_len = ps_core_d->rx_pkt_total_len - sizeof(struct ps_packet_head) - sizeof(struct ps_packet_end);
        ret = ps_push_skb_queue(ps_core_d, buf_ptr, rx_pkt_total_len, g_bfgx_rx_queue[BFGX_BT]);
        if (ret < 0) {
            ps_print_err("[%s]push %s rx data to skb failed!\n",
                         index2name(ps_core_d->pm_data->index), g_bfgx_subsys_name[BFGX_BT]);
            return -EINVAL;
        }

        ps_print_dbg("%s rx done! qlen=%d\n", g_bfgx_subsys_name[BFGX_BT], pst_bfgx_data->rx_queue.qlen);
        oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);
    }

    return 0;
}

STATIC void ps_core_updata_bfg_timer(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t *ptr = NULL;
    uint8_t pkt_type;
    struct pm_drv_data *pm_data = NULL;

    pm_data = ps_core_d->pm_data;

    ptr = buf_ptr + PS_HEAD_SIZE;
    pkt_type = ps_core_d->rx_pkt_type;

    /*
     * if data comes from dev which is not SYS_INF_DEV_AGREE_HOST_SLP &
     * not gnss data & not oml data, we should re-monitor uart transfer
     */
    if (pkt_type == SYS_MSG) {
        if ((*ptr != SYS_INF_DEV_AGREE_HOST_SLP) && (*ptr < SYS_INF_HEARTBEAT_CMD_BASE)) {
            if (!bfgx_other_subsys_all_shutdown(pm_data, BFGX_GNSS)) {
                mod_timer(&pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
                pm_data->bfg_timer_mod_cnt++;
            }
        }
    } else if ((pkt_type != GNSS_FIRST_MSG) &&
               (pkt_type != GNSS_COMMON_MSG) &&
               (pkt_type != GNSS_LAST_MSG) &&
               (pkt_type != OML_MSG) &&
               (oal_atomic_read(&g_ir_only_mode) == 0)) {
        mod_timer(&pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;
        pm_data->gnss_votesleep_check_cnt = 0;
        pm_data->rx_pkt_gnss_pre = 0;
    }
}

STATIC void ps_core_updata_pm_status(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t *ptr = NULL;
    uint8_t pkt_type;
    struct pm_drv_data *pm_data = NULL;

    pm_data = ps_core_d->pm_data;

    ptr = buf_ptr + PS_HEAD_SIZE;
    pkt_type = ps_core_d->rx_pkt_type;

    /*
     * if some data comes from device which is the normal protocol msg,
     * mark it as "received data before device ack".
     */
    if (unlikely(pm_data->bfgx_dev_state_get(pm_data) == BFGX_SLEEP) &&
        (pkt_type != OML_MSG) &&
        (pkt_type != MEM_DUMP_SIZE) &&
        (pkt_type != MEM_DUMP) &&
        (*ptr != SYS_INF_DEV_AGREE_HOST_SLP) &&
        (*ptr < SYS_INF_HEARTBEAT_CMD_BASE)) {
        pm_data->rcvdata_bef_devack_flag = 1;
        ps_print_info("[%s]recv data before dev_ack, type=[0x%x]\n", index2name(ps_core_d->pm_data->index), pkt_type);
    }

    if (pm_data->wakeup_src_debug) {
        ps_print_info("[%s]dev wakeup host pkt type is [%d]\n", index2name(ps_core_d->pm_data->index), pkt_type);
        pm_data->wakeup_src_debug = 0;
    }
}

STATIC void ps_core_status_updata(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data != NULL) {
        atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_RECV_BEAT_INFO);
    }

    ps_core_updata_bfg_timer(ps_core_d, buf_ptr);
    ps_core_updata_pm_status(ps_core_d, buf_ptr);
}

/*
 * Prototype    : ps_decode_packet
 * Description  : called by core when recive data from device is
 *                a complete packet
 *              : decode packet function,and push sk_buff head queue
 * input        : buf_ptr -> ptr of recived data buf
 */
STATIC int32_t ps_decode_packet(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    uint8_t *ptr = NULL;
    uint8_t msg_type;

    ps_core_status_updata(ps_core_d, buf_ptr);

    ptr = buf_ptr + PS_HEAD_SIZE;
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

STATIC int32_t ps_check_chr_packet(struct ps_core_s *ps_core_d, uint8_t* buf_ptr)
{
    /* if type is sys packet,and packet lenth large sys pkt lenth,is err packet */
    if ((ps_core_d->rx_pkt_type == SYS_MSG) &&
        (ps_core_d->rx_pkt_total_len > SYS_TOTAL_PACKET_LENTH)) {
        if ((ps_core_d->rx_pkt_total_len == SYS_TOTAL_PACKET_LENTH + DEV_SEND_CHR_ERRNO_LEN) ||
            (buf_ptr[sizeof(struct ps_packet_head)] == SYS_INF_CHR_ERRNO_REPORT)) {
            ps_print_dbg("[%s]Host recv CHR errno!", index2name(ps_core_d->pm_data->index));
        } else {
            ps_print_err("[%s]the pkt type and len err: %x, %x\n", index2name(ps_core_d->pm_data->index),
                         ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len);
            print_uart_decode_param(ps_core_d);
            return RX_PACKET_ERR;
        }
    }
    return RX_PACKET_CORRECT;
}

STATIC int32_t ps_check_msg_data_len(struct ps_core_s *ps_core_d, uint8_t *buf_ptr)
{
    switch (ps_core_d->rx_pkt_type) {
        case BT_MSG:
            if (ps_core_d->rx_pkt_total_len > BT_RX_MAX_FRAME) {
                ps_print_err("type=%d, the pkt len is ERR: 0x%x\n",
                             ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len);
                print_uart_decode_param(ps_core_d);
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
                if (ps_core_d->rx_pkt_total_len > MAX_LAST_WORD_FRAME_LEN) {
                    ps_print_err("type=%d, the pkt len is ERR: 0x%x,(max frame len is:0x%x)\n",
                                 ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len, MAX_LAST_WORD_FRAME_LEN);
                    print_uart_decode_param(ps_core_d);
                    return RX_PACKET_ERR;
                }
            } else if (ps_core_d->rx_pkt_total_len > MAX_NOBT_FRAME_LEN) {
                ps_print_err("type=%d, the pkt len is ERR: 0x%x,(max frame len is:0x%x)\n",
                             ps_core_d->rx_pkt_type, ps_core_d->rx_pkt_total_len, MAX_NOBT_FRAME_LEN);
                print_uart_decode_param(ps_core_d);
                return RX_PACKET_ERR;
            }
            break;
        default:
            break;
    }

    return RX_PACKET_CORRECT;
}

/*
 * Prototype    : ps_check_packet_head
 * Description  : check recived curr packet is or not a complete packet
 * input        : buf_ptr -> ptr of recived data buf
 *                count   -> size of recived data buf
 * output       : return 0 -> this packet is a complete packet
 *                return 1 -> this packet has not a complete packet,need continue recv
 *                return -1-> this packst is a err packet,del it       :
 */
STATIC int32_t ps_check_packet_head(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, int32_t count)
{
    uint16_t len;
    uint16_t lenbak;
    uint8_t *ptr = buf_ptr;

    if (unlikely(*ptr != PACKET_START_SIGNAL)) {
        ps_print_warning("pkt head is ERR:%x,count=[%d],buf_ptr=[%p]\n", *ptr, count, buf_ptr);
        print_uart_decode_param(ps_core_d);
        return RX_PACKET_ERR;
    }

    /* if count less then sys packet lenth and continue */
    if (count < SYS_TOTAL_PACKET_LENTH) {
        return RX_PACKET_CONTINUE;
    }

    ptr++;
    ps_core_d->rx_pkt_type = *ptr;
    if (oal_unlikely(ps_core_d->rx_pkt_type >= MSG_BUTT)) {
        ps_print_err("[%s]pkt type is ERR:%x\n", index2name(ps_core_d->pm_data->index), ps_core_d->rx_pkt_type);
        print_uart_decode_param(ps_core_d);
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
    if (oal_unlikely(len < SYS_TOTAL_PACKET_LENTH || len > PUBLIC_BUF_MAX)) {
        ps_print_err("the pkt len is ERR: %x\n", len);
        print_uart_decode_param(ps_core_d);
        ps_core_d->rx_pkt_total_len = 0;
        return RX_PACKET_ERR;
    }

    /* check all service frame length */
    if (oal_unlikely(ps_check_msg_data_len(ps_core_d, buf_ptr) == RX_PACKET_ERR)) {
        return RX_PACKET_ERR;
    }

    /* check whether the recv packet is CHR packet */
    if (oal_unlikely(ps_check_chr_packet(ps_core_d, buf_ptr) == RX_PACKET_ERR)) {
        return RX_PACKET_ERR;
    }

    /* the packet is breaked, need continue rx */
    if (ps_core_d->rx_pkt_total_len > count) {
        return RX_PACKET_CONTINUE;
    }

    ptr = buf_ptr + ps_core_d->rx_pkt_total_len - PS_END_SIZE;
    /* check the last byte yes or not 7e */
    if (oal_unlikely(*ptr != PACKET_END_SIGNAL)) {
        ps_print_warning("[%s]pkt end is ERR: %x\n", index2name(ps_core_d->pm_data->index), *ptr);
        print_uart_decode_param(ps_core_d);
        return RX_PACKET_ERR;
    }

    return RX_PACKET_CORRECT;
}

STATIC void ps_tty_rx_buf_set(struct ps_core_s *ps_core_d, int32_t data_len)
{
    ps_core_d->rx_have_del_public_len = 0;
    ps_core_d->rx_have_recv_pkt_len = data_len;
    ps_core_d->rx_decode_public_buf_ptr = ps_core_d->rx_public_buf_org_ptr;
}

void dump_uart_rx_buf(struct ps_core_s *ps_core_d)
{
#define UART_DUMP_RX_BUF_LENGTH 16
    uint32_t i = 0;
    uint32_t ul_dump_len;

    /* uart在接收数据时，不能flaush buffer */
    oal_spin_lock(&ps_core_d->rx_lock);

    if (ps_core_d->rx_have_recv_pkt_len > 0) {
        /* have decode all public buf data, reset ptr and lenth */
        ps_print_warning("[%s]uart rx buf has data, rx_have_recv_pkt_len=%d\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_have_recv_pkt_len);
        ps_print_warning("[%s]uart rx buf has data, rx_have_del_public_len=%d\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_have_del_public_len);
        ps_print_warning("[%s]uart rx buf has data, rx_decode_public_buf_ptr=%p\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_decode_public_buf_ptr);
        ps_print_warning("[%s]uart rx buf has data, rx_public_buf_org_ptr=%p\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_public_buf_org_ptr);

        ul_dump_len = ps_core_d->rx_have_recv_pkt_len > UART_DUMP_RX_BUF_LENGTH ?
            UART_DUMP_RX_BUF_LENGTH : ps_core_d->rx_have_recv_pkt_len;
        for (i = 0; i < ul_dump_len; i++) {
            ps_print_warning("[%s]uart rx buf has data, data[%d]=0x%x\n",
                             index2name(ps_core_d->pm_data->index), i,
                             *(ps_core_d->rx_decode_public_buf_ptr + i));
        }
    }

    oal_spin_unlock(&ps_core_d->rx_lock);
}

void reset_uart_rx_buf(struct ps_core_s *ps_core_d)
{
    uint32_t i = 0;

    /* uart在接收数据时，不能flaush buffer */
    oal_spin_lock(&ps_core_d->rx_lock);

    if (ps_core_d->rx_have_recv_pkt_len > 0) {
        /* have decode all public buf data, reset ptr and lenth */
        ps_print_warning("[%s]uart rx buf has data, rx_have_recv_pkt_len=%d\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_have_recv_pkt_len);
        ps_print_warning("[%s]uart rx buf has data, rx_have_del_public_len=%d\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_have_del_public_len);
        ps_print_warning("[%s]uart rx buf has data, rx_decode_public_buf_ptr=%p\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_decode_public_buf_ptr);
        ps_print_warning("[%s]uart rx buf has data, rx_public_buf_org_ptr=%p\n",
                         index2name(ps_core_d->pm_data->index), ps_core_d->rx_public_buf_org_ptr);
        for (i = 0; i < ps_core_d->rx_have_recv_pkt_len; i++) {
            ps_print_warning("[%s]uart rx buf has data, data[%d]=0x%x\n",
                             index2name(ps_core_d->pm_data->index), i,
                             *(ps_core_d->rx_decode_public_buf_ptr + i));
        }
        ps_tty_rx_buf_set(ps_core_d, 0);
    }

    oal_spin_unlock(&ps_core_d->rx_lock);
}

/*
 * Prototype    : ps_core_recv
 * Description  : PS's core receive function.Decodes received RAW data and
 *                forwards to corresponding client drivers (Bluetooth)
 *                or hal stack(FM,GNSS).
 * input        : data -> recive data ptr   from UART TTY
 *                count -> recive data count from UART TTY
 */
int32_t ps_core_recv(void *disc_data, const uint8_t *data, int32_t count)
{
    struct ps_core_s *ps_core_d = NULL;
    uint8_t *ptr = NULL;
    int32_t data_len;
    int32_t ret;

    ps_core_d = (struct ps_core_s *)disc_data;
    ps_core_d->rx_decode_tty_ptr = (uint8_t *)data;

    if (ps_core_d->rx_have_recv_pkt_len > 0) {
        /* if curr packet is breaked package, first copy all uart buf data to public buf */
        ptr = ps_core_d->rx_public_buf_org_ptr + ps_core_d->rx_have_recv_pkt_len;
        if (unlikely(memcpy_s(ptr, PUBLIC_BUF_MAX - ps_core_d->rx_have_recv_pkt_len,
                              ps_core_d->rx_decode_tty_ptr, count) != EOK)) {
            ps_tty_rx_buf_set(ps_core_d, 0);
            ps_print_warning("received count too large!, memcopy_s error, destlen=%d, srclen=%d\n ",
                             PUBLIC_BUF_MAX - ps_core_d->rx_have_recv_pkt_len, count);
            return 0;
        }
    } else {
        /* if not breaked package, direct decode in uart buf */
        ps_core_d->rx_decode_public_buf_ptr = ps_core_d->rx_decode_tty_ptr;
    }

    ps_core_d->rx_have_recv_pkt_len += count;
    data_len = ps_core_d->rx_have_recv_pkt_len;

    // decode public buf data
    while (data_len > 0) {
        ret = ps_check_packet_head(ps_core_d, ps_core_d->rx_decode_public_buf_ptr, data_len);
        if (ret == RX_PACKET_CORRECT) {
            ps_decode_packet(ps_core_d, ps_core_d->rx_decode_public_buf_ptr);
            ps_core_d->rx_have_del_public_len += ps_core_d->rx_pkt_total_len;
            ps_core_d->rx_decode_public_buf_ptr += ps_core_d->rx_pkt_total_len;
            data_len = data_len - ps_core_d->rx_pkt_total_len;
            continue;
        } else if (ret == RX_PACKET_CONTINUE) {
            /* copy breaked pkt to buf head */
            if (unlikely(memmove_s(ps_core_d->rx_public_buf_org_ptr, PUBLIC_BUF_MAX,
                                   ps_core_d->rx_decode_public_buf_ptr, data_len) != EOK)) {
                ps_tty_rx_buf_set(ps_core_d, 0);
                ps_print_err("memmove_s error, destlen=%d, srclen=%d\n ", PUBLIC_BUF_MAX, data_len);
                return 0;
            }
            ps_tty_rx_buf_set(ps_core_d, data_len);
            data_len = 0;
        } else {
            /* this packet is err! remove a byte and continue decode */
            ps_core_d->rx_decode_public_buf_ptr++;
            ps_core_d->rx_have_del_public_len++;
            data_len--;
        }
    }

    /* have decode all public buf data, reset ptr and lenth */
    if (ps_core_d->rx_have_del_public_len == ps_core_d->rx_have_recv_pkt_len) {
        ps_tty_rx_buf_set(ps_core_d, 0);
    }

    return 0;
}

/*
 * Prototype    : ps_bt_enqueue
 * Description  : push skb data to skb head queue from tail
 */
void ps_skb_enqueue(struct ps_core_s *ps_core_d, struct sk_buff *skb, uint8_t type)
{
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err(" ps_core_d is NULL\n");
        return;
    }

    if (unlikely(skb == NULL)) {
        ps_print_err(" skb is NULL\n");
        return;
    }

    switch (type) {
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
}

/*
 * Prototype    : ps_skb_dequeue
 * Description  : internal de sk_buff seq function. return top of txq.
 */
struct sk_buff *ps_skb_dequeue(struct ps_core_s *ps_core_d, uint8_t type)
{
    struct sk_buff *curr_skb = NULL;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return curr_skb;
    }

    switch (type) {
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

/*
 * Prototype    : ps_skb_queue_len
 * Description  : internal queue skb nums.
 */
int32_t ps_skb_queue_len(struct ps_core_s *ps_core_d, uint8_t type)
{
    struct sk_buff_head* curr_queue = NULL;

    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return 0;
    }

    switch (type) {
        case TX_HIGH_QUEUE:
            curr_queue = &ps_core_d->tx_high_seq;
            break;
        case TX_LOW_QUEUE:
            curr_queue = &ps_core_d->tx_low_seq;
            break;
        case RX_DBG_QUEUE:
            curr_queue = &ps_core_d->rx_dbg_seq;
            break;
        case RX_BT_QUEUE:
            curr_queue = &ps_core_d->bfgx_info[BFGX_BT].rx_queue;
            break;
        case RX_FM_QUEUE:
            curr_queue = &ps_core_d->bfgx_info[BFGX_FM].rx_queue;
            break;
        case RX_GNSS_QUEUE:
            curr_queue = &ps_core_d->bfgx_info[BFGX_GNSS].rx_queue;
            break;
        case RX_IR_QUEUE:
            curr_queue = &ps_core_d->bfgx_info[BFGX_IR].rx_queue;
            break;
        case RX_NFC_QUEUE:
            curr_queue = &ps_core_d->bfgx_info[BFGX_NFC].rx_queue;
            break;
        default:
            ps_print_err("queue type is error, type=%d\n", type);
            break;
    }

    if (curr_queue != NULL) {
        return skb_queue_len(curr_queue);
    } else {
        return 0;
    }
}


/* ready状态遍历所有队列 */
STATIC struct sk_buff *ps_skb_dequeue_core(struct ps_core_s *ps_core_d, uint32_t *dequeue_init_flag)
{
    struct sk_buff *skb = NULL;
    static uint8_t tx_high_times;

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

STATIC int32_t ps_core_tx_para_check(struct ps_core_s *ps_core_d)
{
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if (unlikely(ps_core_d->pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }
    return 0;
}

/*
 * Prototype    : ps_core_tx_attemper
 * Description  : adj tx skb buf data to tty, seq BT and FM or GNSS.
 */
STATIC int32_t ps_core_tx_attemper(struct ps_core_s *ps_core_d)
{
    struct sk_buff *skb = NULL;
    int32_t len;
    struct pm_drv_data *pm_data = NULL;
    uint32_t dequeue_init_flag = 1;

    if (ps_core_tx_para_check(ps_core_d) != 0) {
        return -EINVAL;
    }

    pm_data = ps_core_d->pm_data;

    while (1) {
        skb = ps_skb_dequeue_core(ps_core_d, &dequeue_init_flag);
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
 */
STATIC void ps_core_tx_work(oal_work_stru *work)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_core_d = container_of(work, struct ps_core_s, tx_skb_work);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    /* makesure tty is avilable. release_tty_drv race access tty */
    mutex_lock(&ps_core_d->tty_mutex);
    if (ps_core_d->tty != NULL) {
        /* into tx skb buff queue */
        ps_core_tx_attemper(ps_core_d);
    }
    mutex_unlock(&ps_core_d->tty_mutex);

    return;
}

/*
 * Prototype    : ps_add_packet_head
 * Description  : add packet head to recv buf from hal or bt driver.
 * input        : buf  -> ptr of buf
 *                type -> packet type，example bt,fm,or gnss
 *                lenth-> packet length
 */
int32_t ps_add_packet_head(uint8_t *buf, uint8_t type, uint16_t lenth)
{
    uint8_t *ptr = buf;
    uint16_t len;

    if (oal_unlikely(ptr == NULL)) {
        ps_print_err("buf is NULL\n");
        return -EINVAL;
    }

    if (oal_unlikely(lenth < PS_PKG_SIZE)) {
        ps_print_err("buf is not enough\n");
        return -EINVAL;
    }

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
    ptr = ptr - PS_END_SIZE;
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
STATIC void ps_set_sys_packet(uint8_t *buf, uint8_t type, uint8_t content)
{
    uint8_t *ptr = buf;

    if (oal_unlikely(ptr == NULL)) {
        ps_print_err("buf is NULL\n");
        return;
    }

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
}

/*
 * Prototype    : ps_tx_sys_cmd
 * Description  : tx sys commend to uart tty
 */
int32_t ps_tx_sys_cmd(struct ps_core_s *ps_core_d, uint8_t type, uint8_t content)
{
    struct sk_buff *skb = NULL;
    bool ret = false;

    skb = alloc_skb(SYS_TOTAL_PACKET_LENTH, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
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
        ps_print_info("[%s]queue sys msg work, ret = %d, type = %d\n",
                      index2name(ps_core_d->pm_data->index), ret, content);
    }

    return 0;
}

int32_t ps_h2d_tx_data(struct ps_core_s *ps_core_d, uint8_t subsys, const char __user *buf, size_t count)
{
    struct sk_buff *skb = NULL;
    uint16_t tx_skb_len;
    uint16_t tx_data_len;
    uint8_t start = 0;

    while (count > 0) {
        tx_data_len = (count > PS_TX_PACKET_LIMIT) ? PS_TX_PACKET_LIMIT : count;

        /* curr tx skb total lenth */
        tx_skb_len = tx_data_len + PS_PKG_SIZE;
        skb = ps_alloc_skb(tx_skb_len);
        if (skb == NULL) {
            ps_print_err("ps alloc skb mem fail\n");
            return -EFAULT;
        }

        if (count > PS_TX_PACKET_LIMIT) {
            if (start == false) { /* this is a start packet */
                ps_add_packet_head(skb->data, g_ps_tx_info[subsys].start_pkg, tx_skb_len);
                start = true;
            } else { /* this is a int packet */
                ps_add_packet_head(skb->data, g_ps_tx_info[subsys].common_pkg, tx_skb_len);
            }
        } else { /* this is the last packet */
            ps_add_packet_head(skb->data, g_ps_tx_info[subsys].last_pkg, tx_skb_len);
        }

        if (copy_from_user(&skb->data[PS_HEAD_SIZE], buf, tx_data_len)) {
            ps_print_err("can't copy_from_user for gnss\n");
            kfree_skb(skb);
            return -EFAULT;
        }

        /* push the skb to skb queue */
        ps_skb_enqueue(ps_core_d, skb, g_ps_tx_info[subsys].tx_queue);
        queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

        buf = buf + tx_data_len;
        count = count - tx_data_len;
    }

    return 0;
}

STATIC int32_t ps_core_kzalloc_check(struct ps_core_s **ps_core_d)
{
    uint8_t *ptr = NULL;
    *ps_core_d = kzalloc(sizeof(struct ps_core_s), GFP_KERNEL);

    if (*ps_core_d == NULL) {
        ps_print_err("memory allocation failed\n");
        return -ENOMEM;
    }

    ptr = kzalloc(PUBLIC_BUF_MAX, GFP_KERNEL);
    if (ptr == NULL) {
        kfree(*ps_core_d);
        ps_print_err("no mem to allocate to public buf!\n");
        return -ENOMEM;
    }

    memset_s(ptr, PUBLIC_BUF_MAX, 0, PUBLIC_BUF_MAX);
    (*ps_core_d)->rx_public_buf_org_ptr = ptr;
    (*ps_core_d)->rx_decode_public_buf_ptr = ptr;

    return 0;
}

/*
 * Prototype    : ps_core_init
 * Description  : init ps_core_d struct and kzalloc memery
 */
int32_t ps_core_init(struct ps_core_s **core_data, int index)
{
    struct ps_core_s *ps_core_d = NULL;
    const char *name = NULL;
    int32_t err;
    int32_t i = 0;

    if (ps_core_kzalloc_check(&ps_core_d) < 0) {
        return -ENOMEM;
    }

    err = ps_pm_register(ps_core_d, index);
    if (err) {
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d);
        ps_print_err("error registering ps_pm err = %d\n", err);
        return -EFAULT;
    }

    err = plat_uart_init(index);
    if (err) {
        ps_pm_unregister(ps_core_d, index);
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d);
        ps_print_err("error registering line discipline %d\n", err);
        return -EFAULT;
    }

    /*
     * Initialize PS tx and rx queue head. BT module skb's
     * will be pushed in this queue for actual transmission.
     */
    skb_queue_head_init(&ps_core_d->tx_high_seq);
    skb_queue_head_init(&ps_core_d->tx_low_seq);
    skb_queue_head_init(&ps_core_d->rx_dbg_seq);
    atomic_set(&ps_core_d->force_tx_exit, 0);

    // 开机过程接收bfgx日志需要配成1，后续oam_hisi退出会清0
    atomic_set(&ps_core_d->dbg_recv_dev_log, 1);

    oal_spin_lock_init(&ps_core_d->rx_lock);

    sema_init(&ps_core_d->sr_wake_sema, 1);
    atomic_set(&ps_core_d->sr_cnt, 0);

    /* create a singlethread work queue */
    name = (index == BUART) ? "ps_tx_queue" : "ps_me_tx_queue";
    ps_core_d->ps_tx_workqueue = create_singlethread_workqueue(name);
    /* init tx work */
    INIT_WORK(&ps_core_d->tx_skb_work, ps_core_tx_work);

    for (i = 0; i < BFGX_BUTT; i++) {
        /* 初始化接收队列头 */
        oal_netbuf_head_init(&ps_core_d->bfgx_info[i].rx_queue);
        /* 初始化BFGX接收等待队列 */
        oal_wait_queue_init_head(&ps_core_d->bfgx_info[i].rx_wait);
        /* 初始化分包接收数据结构，BT不使用分包接收 */
        oal_spin_lock_init(&ps_core_d->bfgx_info[i].sepreted_rx.sepreted_rx_lock);
        ps_core_d->bfgx_info[i].sepreted_rx.rx_prev_seq = RX_SEQ_NULL;
        ps_core_d->bfgx_info[i].sepreted_rx.rx_buf_all_len = 0;
        ps_core_d->bfgx_info[i].sepreted_rx.rx_buf_ptr = NULL;
        ps_core_d->bfgx_info[i].sepreted_rx.rx_buf_org_ptr = NULL;
        oal_spin_lock_init(&ps_core_d->bfgx_info[i].rx_lock);
        ps_core_d->bfgx_info[i].rx_pkt_num = 0;
        ps_core_d->bfgx_info[i].tx_pkt_num = 0;
        ps_core_d->bfgx_info[i].name = g_bfgx_subsys_name[i];
        /* 初始化开关业务完成量 */
        oal_init_completion(&ps_core_d->bfgx_info[i].wait_closed);
        oal_init_completion(&ps_core_d->bfgx_info[i].wait_opened);
        atomic_set(&ps_core_d->bfgx_info[i].subsys_state, POWER_STATE_SHUTDOWN);
    }

    atomic_set(&ps_core_d->node_visit_flag, 0);

    oal_wait_queue_init_head(&ps_core_d->rx_dbg_wait);

    ps_core_d->rx_pkt_total_len = 0;
    ps_core_d->rx_have_recv_pkt_len = 0;
    ps_core_d->rx_have_del_public_len = 0;

    ps_core_d->tty_have_open = false;
    ps_core_d->dev_index = index;
    *core_data = ps_core_d;

    ps_core_d->rx_pkt_sys = 0;
    ps_core_d->rx_pkt_oml = 0;

    mutex_init(&ps_core_d->tty_mutex);
    atomic_set(&ps_core_d->tty_tx_cnt, 0);
    atomic_set(&ps_core_d->tty_rx_cnt, 0);

    return 0;
}

/*
 * Prototype    : ps_core_exit
 * Description  : release have init ps_core_d struct and kfree memery:
 */
int32_t ps_core_exit(struct ps_core_s *ps_core_d, int index)
{
    int32_t err;

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
        err = plat_uart_exit(index);
        if (err) {
            ps_print_err("unable to un-register ldisc %d\n", err);
        }
        /* unregister pm */
        ps_pm_unregister(ps_core_d, index);
        /* free the global data pointer */
        kfree(ps_core_d->rx_public_buf_org_ptr);
        kfree(ps_core_d);
    }
    return 0;
}
