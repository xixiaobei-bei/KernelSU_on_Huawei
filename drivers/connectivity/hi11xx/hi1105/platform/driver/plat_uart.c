

/* 头文件包含 */
/*lint -e322*/ /*lint -e7*/
#include "plat_uart.h"
#include "chr_user.h"
#include "oal_ext_if.h"
#include "plat_exception_rst.h"

#include <linux/tty.h>

#include "board.h"
#include "plat_debug.h"
#include "bfgx_dev.h"
#include "bfgx_data_parse.h"
/*lint +e322*/ /*lint +e7*/

/* 全局变量定义 */
uint32_t g_uart_default_baud_rate = DEFAULT_BAUD_RATE;

/* no lock while getting the state, just statistic */
/* call only in one place!!! */
void ps_tty_tx_cnt_add(struct ps_core_s *ps_core_d, uint32_t cnt)
{
    oal_atomic_add(&(ps_core_d->tty_tx_cnt), cnt);
}

/* call only in one place!!! */
STATIC void ps_tty_rx_cnt_add(struct ps_core_s *ps_core_d, uint32_t cnt)
{
    oal_atomic_add(&(ps_core_d->tty_rx_cnt), cnt);
}

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
void ps_uart_state_dump(struct ps_core_s *ps_core_d)
{
    return;
}

#else
STATIC void ps_uart_state_print(struct tty_struct *tty)
{
    struct serial_icounter_struct icount;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_UART, CHR_PLAT_DRV_ERROR_UART_PRINT);

    ps_print_info("stopped:%x  hw_stopped:%x\n", tty->stopped, tty->hw_stopped);

    if (tty->ops->get_icount(tty, &icount) < 0) {
        ps_print_err("get icount error\n");
        return;
    }

    ps_print_info("uart tx:%x    rx:%x\n", icount.tx, icount.rx);
    ps_print_info("uart cts:%x,dsr:%x,rng:%x,dcd:%x,frame:%x,overrun:%x,parity:%x,brk:%x,buf_overrun:%x\n",
                  icount.cts, icount.dsr, icount.rng, icount.dcd,
                  icount.frame, icount.overrun, icount.parity, icount.brk,
                  icount.buf_overrun);
    return;
}

STATIC void ps_tty_buffer_print(struct tty_struct *tty)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    struct tty_buffer *thead = NULL;
    struct tty_bufhead *buf = NULL;

    if (tty->port == NULL) {
        return;
    }

    buf = &(tty->port->buf);
    thead = buf->head;
    while (thead != NULL) {
        ps_print_info("tty rx buf:used=0x%x,size=0x%x,commit=0x%x,read=0x%x\n",
                      thead->used, thead->size, thead->commit, thead->read);
        thead = thead->next;
    }
#endif
}

void ps_uart_state_dump(struct ps_core_s *ps_core_d)
{
    struct tty_struct *tty = NULL;

    /* user版本受控 */
    if (is_hi110x_debug_type() == OAL_FALSE) {
        ps_print_info("uart state dump ignore\n");
        return;
    }

    tty = ps_core_d->tty;
    if (tty_kref_get(tty) == NULL) {
        ps_print_err("ps_core_d or tty is NULL\n");
        return;
    }

    ps_print_info("===uart&tty state===\n");
    ps_print_info("tty tx:%x, rx:%x\n", oal_atomic_read(&(ps_core_d->tty_tx_cnt)),
                  oal_atomic_read(&(ps_core_d->tty_rx_cnt)));
    ps_print_info("chars in tty tx buf len=%x\n", tty_chars_in_buffer(tty));

    ps_uart_state_print(tty);
    ps_tty_buffer_print(tty);
    tty_kref_put(tty);

    return;
}
#endif

STATIC char *get_tty_name(struct ps_plat_s *ps_plat_d)
{
    char *path = "/dev/";
    char *tty_dev_name = NULL;

    tty_dev_name = ps_plat_d->dev_name + strlen(path);

    return tty_dev_name;
}


/*
 * Prototype    : is_hisi_connetivity_tty
 * Description  : check the tty can match hisi connectivity ldisc
 */
STATIC bool is_hisi_connetivity_tty(struct tty_struct *tty, int type)
{
    struct ps_core_s *ps_core_d = NULL;
    struct ps_plat_s *ps_plat_d = NULL;
    const char *tty_name = NULL;

    if (tty == NULL) {
        ps_print_err("tty is NULL\n");
        return false;
    }

    ps_core_d = ps_get_core_reference(type);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return false;
    }

    ps_plat_d = (struct ps_plat_s *)(ps_core_d->ps_plat);
    if (unlikely(ps_plat_d == NULL)) {
        ps_print_err("ps_plat_d is NULL\n");
        return false;
    }

    tty_name = get_tty_name(ps_plat_d);
    if (strcmp(tty_name, tty->name) != 0) {
        return false;
    }

    return true;
}


/*
 * Prototype    : ps_tty_open
 * Description  : called by tty uart itself when open tty uart from octty
 * input        : tty -> have opened tty
 */
STATIC int32_t ps_tty_open(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s enter\n", __func__);

    if (!is_hisi_connetivity_tty(tty, BUART)) {
        return -EINVAL;
    }

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    ps_core_d->tty = tty;
    tty->disc_data = ps_core_d;

    /* don't do an wakeup for now */
    clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

    /* set mem already allocated */
    tty->receive_room = PUBLIC_BUF_MAX;
    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);

    return 0;
}

STATIC int32_t ps_me_tty_open(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s enter\n", __func__);

    if (!is_hisi_connetivity_tty(tty, GUART)) {
        return -EINVAL;
    }

    ps_core_d = ps_get_core_reference(GUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    ps_core_d->tty = tty;
    tty->disc_data = ps_core_d;

    /* don't do an wakeup for now */
    clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

    /* set mem already allocated */
    tty->receive_room = PUBLIC_BUF_MAX;
    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);

    return 0;
}

/*
 * Prototype    : ps_tty_close
 * Description  : called by tty uart when close tty uart from octty
 * input        : tty -> have opened tty
 */
STATIC void ps_tty_close(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s: entered\n", __func__);

    if ((!is_hisi_connetivity_tty(tty, BUART)) && (!is_hisi_connetivity_tty(tty, GUART))) {
        return;
    }

    if (tty->disc_data == NULL) {
        ps_print_err("tty or tty->disc_data is NULL\n");
        return;
    }

    ps_core_d = tty->disc_data;

    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);
    ps_core_d->tty = NULL;

    ps_print_info("tty close done!\n");
}

STATIC int32_t ps_tty_receive_check(struct tty_struct *tty, uint32_t type,
                                    int32_t count, const uint8_t *data)
{
#ifdef PLATFORM_DEBUG_ENABLE
    struct st_exception_info *pst_exception_data = NULL;
#endif

    if (!is_hisi_connetivity_tty(tty, type)) {
        ps_print_err("input type %d is error\n", type);
        return -EINVAL;
    }

    if (unlikely((tty->disc_data == NULL) || (st_tty_recv == NULL))) {
        ps_print_err("[%s] tty or tty->disc_data or st_tty_recv is NULL\n",
                     tty_name(tty));
        return -EINVAL;
    }

    if (unlikely(data == NULL)) {
        ps_print_err("[%s] received null from TTY\n", tty_name(tty));
        return -EINVAL;
    }

    if (unlikely(count < 0)) {
        ps_print_err("[%s] received count from TTY err\n", tty_name(tty));
        return -EINVAL;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    // 心跳超时DFR打桩验证
    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data != NULL) {
        if (pst_exception_data->debug_beat_flag == 0) {
            if (oal_print_rate_limit(PRINT_RATE_SECOND)) {
                ps_print_info("[%s]heartbeat dfr test, drop TTY data\n",
                              tty_name(tty));
            }
            return -EINVAL;
        }
    }
#endif

    return 0;
}

/*
 * Prototype    : ps_tty_receive
 * Description  : called by tty uart when recive data from tty uart
 * input        : tty   -> have opened tty
 *                data -> recive data ptr
 *                count-> recive data count
 */
STATIC void ps_tty_receive(struct tty_struct *tty, const uint8_t *data, char *tty_flags, int32_t count)
{
    struct ps_core_s *ps_core_d = NULL;

    if (ps_tty_receive_check(tty, BUART, count, data) < 0) {
        return;
    }

    ps_core_d = tty->disc_data;
    oal_spin_lock(&ps_core_d->rx_lock);

    ps_tty_rx_cnt_add(ps_core_d, count);
    st_tty_recv(tty->disc_data, data, count);

    oal_spin_unlock(&ps_core_d->rx_lock);
}

STATIC void ps_me_tty_receive(struct tty_struct *tty, const uint8_t *data, char *tty_flags, int32_t count)
{
    struct ps_core_s *ps_core_d = NULL;

    if (ps_tty_receive_check(tty, GUART, count, data) < 0) {
        return;
    }

    ps_core_d = tty->disc_data;
    oal_spin_lock(&ps_core_d->rx_lock);

    ps_tty_rx_cnt_add(ps_core_d, count);
    me_tty_recv(tty->disc_data, data, count);

    oal_spin_unlock(&ps_core_d->rx_lock);
}

/*
 * Prototype    : ps_tty_wakeup
 * Description  : called by tty uart when wakeup from suspend
 * input        : tty   -> have opened tty
 */
STATIC void ps_tty_wakeup(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    if ((!is_hisi_connetivity_tty(tty, BUART)) && (!is_hisi_connetivity_tty(tty, GUART))) {
        return;
    }

    if (tty->disc_data == NULL) {
        ps_print_err("tty or tty->disc_data is NULL\n");
        return;
    }
    ps_core_d = tty->disc_data;
    /* don't do an wakeup for now */
    clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

    /* call our internal wakeup */
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);
}

STATIC void ps_clean_tx_skb_buf(struct ps_core_s *ps_core_d)
{
#define WAIT_TX_WORK_DELAY 20
    uint8_t delay_times = RELEASE_DELAT_TIMES;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    ps_kfree_skb(ps_core_d, TX_HIGH_QUEUE);
    ps_kfree_skb(ps_core_d, TX_LOW_QUEUE);

    ps_print_info("free tx sbk buf done!\n");

    /* clean all tx sk_buff */
    while (((ps_core_d->tx_high_seq.qlen) || (ps_core_d->tx_low_seq.qlen)) && (delay_times)) {
        msleep(10); // sleep 10ms
        delay_times--;
    }

    if (oal_work_is_busy(&ps_core_d->tx_skb_work)) {
        ps_print_info("hisi bfgx notify tx work exit\n");
        atomic_set(&ps_core_d->force_tx_exit, 1);
        // wait for tx work exit
        msleep(WAIT_TX_WORK_DELAY);
    }

    if (!oal_is_err_or_null(ps_core_d->tty)) {
        if (tty_chars_in_buffer(ps_core_d->tty)) {
            ps_print_info("uart tx buf need clean, or it will block in uart close\n");
            tty_driver_flush_buffer(ps_core_d->tty);
        }
    }
}

/*
 * Prototype    : ps_tty_flush_buffer
 * Description  : called by tty uart when flush buffer
 * input        : tty   -> have opened tty
 */
STATIC void ps_tty_flush_buffer(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s: entered\n", __func__);

    if ((!is_hisi_connetivity_tty(tty, BUART)) && (!is_hisi_connetivity_tty(tty, GUART))) {
        return;
    }

    if (tty->disc_data == NULL) {
        ps_print_err("tty or tty->disc_data is NULL\n");
        return;
    }

    ps_core_d = tty->disc_data;

    reset_uart_rx_buf(ps_core_d);

    return;
}

static struct tty_struct *ps_tty_kopen(char* dev_name)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    struct tty_struct *tty = NULL;
    int ret;
    dev_t dev_no;

    ps_print_info("%s\n", __func__);

    ret = tty_dev_name_to_number(dev_name, &dev_no);
    if (ret != 0) {
        ps_print_err("can't found tty:%s ret=%d\n", dev_name, ret);
        return NULL;
    }

    /* open tty */
    tty = tty_kopen(dev_no);
    if (oal_is_err_or_null(tty)) {
        ps_print_err("open tty %s failed ret=%d\n", dev_name, PTR_RET(tty));
        return NULL;
    }

    if (tty->ops->open) {
        ret = tty->ops->open(tty, NULL);
    } else {
        ps_print_err("tty->ops->open is NULL\n");
        ret = -ENODEV;
    }

    if (ret) {
        tty_unlock(tty);
        return NULL;
    } else {
        return tty;
    }
#else
    return NULL;
#endif
}

static void ps_tty_kclose(struct ps_core_s *ps_core_d)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    struct tty_struct *tty = ps_core_d->tty;

    ps_print_info("%s\n", __func__);

    /* close tty */
    if (tty == NULL) {
        ps_print_err("tty is null, ignore\n");
        return;
    }

    tty_lock(tty);
    if (tty->ops->close) {
        tty->ops->close(tty, NULL);
    } else {
        ps_print_warning("tty->ops->close is null\n");
    }
    tty_unlock(tty);

    tty_kclose(tty);
#endif
}

static void ps_ktty_set_termios(struct tty_struct *tty, long baud_rate, uint8_t enable_flowctl)
{
    struct ktermios ktermios;

    ktermios = tty->termios;

    /* close soft flowctrl */
    ktermios.c_iflag &= ~IXON;

    /* set uart cts/rts flowctrl */
    ktermios.c_cflag &= ~CRTSCTS;
    if (enable_flowctl == FLOW_CTRL_ENABLE) {
        ktermios.c_cflag |= CRTSCTS;
    }

    /* set csize */
    ktermios.c_cflag &= ~(CSIZE);
    ktermios.c_cflag |= CS8;

    /* set uart baudrate */
    ktermios.c_cflag &= ~CBAUD;
    ktermios.c_cflag |= BOTHER;
    tty_termios_encode_baud_rate(&ktermios, baud_rate, baud_rate);
    tty_set_termios(tty, &ktermios);

    ps_print_info("set baud_rate=%d, except=%d\n", (int)tty_termios_baud_rate(&tty->termios), (int)baud_rate);
}

/* ps_set_tty_termios ttyUart in kernel driver */
/*
 * Prototype    : ps_set_tty_termios
 * Description  : set uart attr
 */
static int32_t ps_set_tty_termios(struct ps_core_s *ps_core_d, long baud_rate, uint8_t enable_flowctl)
{
    struct tty_struct *tty = NULL;

    ps_print_info("%s\n", __func__);

    mutex_lock(&ps_core_d->tty_mutex);

    tty = ps_core_d->tty;
    if (tty == NULL) {
        mutex_unlock(&ps_core_d->tty_mutex);
        ps_print_err("tty is closed\n");
        return -ENODEV;
    }

    ps_ktty_set_termios(tty, baud_rate, enable_flowctl);

    mutex_unlock(&ps_core_d->tty_mutex);

    return 0;
}

/* remove octty process access ttyUart in kernel driver */
/*
 * Prototype    : ps_change_uart_baud_rate
 * Description  : change arm platform uart baud rate to secend
 *                baud rate for high baud rate when download patch
 */
int32_t ps_change_uart_baud_rate(struct ps_core_s *ps_core_d, long baud_rate, uint8_t enable_flowctl)
{
    struct ps_plat_s *ps_plat_d = (struct ps_plat_s *)(ps_core_d->ps_plat);

    ps_print_info("%s:%s %lu\n", __func__, get_tty_name(ps_plat_d), baud_rate);

    /* for debug only */
    dump_uart_rx_buf(ps_core_d);

    if (wait_bfgx_memdump_complete() != EXCEPTION_SUCCESS) {
        ps_print_err("wait memdump complete failed\n");
    }

    if (!oal_is_err_or_null(ps_core_d->tty)) {
        if (tty_chars_in_buffer(ps_core_d->tty)) {
            ps_print_info("uart tx buf is not empty\n");
            tty_driver_flush_buffer(ps_core_d->tty);
        }
    }

    ps_plat_d->flow_cntrl = enable_flowctl;
    ps_plat_d->baud_rate = baud_rate;

    return ps_set_tty_termios(ps_core_d, ps_plat_d->baud_rate, ps_plat_d->flow_cntrl);
}

/*
 * Prototype    : open_tty_drv
 * Description  : called from PS Core when BT protocol stack drivers
 *                registration,or FM/GNSS hal stack open FM/GNSS inode
 */
int32_t open_tty_drv(struct ps_core_s *ps_core_d)
{
    int ret;
    struct tty_struct *tty = NULL;
    struct ps_plat_s *ps_plat_d = (struct ps_plat_s *)(ps_core_d->ps_plat);
    char *tty_name = NULL;

    tty_name = get_tty_name(ps_plat_d);

    ps_print_info("%s:%s\n", __func__, tty_name);
    mutex_lock(&ps_core_d->tty_mutex);

    if (ps_core_d->tty_have_open == true) {
        ps_print_warning("hisi bfgx line discipline have installed\n");
        mutex_unlock(&ps_core_d->tty_mutex);
        return 0;
    }

    reset_uart_rx_buf(ps_core_d);

    if (oal_atomic_read(&g_ir_only_mode) != 0) {
        /* ir only mode use baudrate 921600 */
        ps_plat_d->baud_rate = IR_ONLY_BAUD_RATE;
    }

    /* open tty */
    tty = ps_tty_kopen(tty_name);
    if (tty == NULL) {
        ps_print_err("failed to open ktty\n");
        mutex_unlock(&ps_core_d->tty_mutex);
        return -ENODEV;
    }

    ps_ktty_set_termios(tty, ps_plat_d->baud_rate, ps_plat_d->flow_cntrl);

    // tty_lock在tty_kopen时执行，成功时不释放锁，只有失败的时候才会释放
    tty_unlock(tty);

    ps_print_info("start tty set ldisc num=%d\n", ps_plat_d->ldisc_num);

    /* set line ldisc */
    ret = tty_set_ldisc(tty, ps_plat_d->ldisc_num); /* export after 4.13 */
    if (ret != 0) {
        ps_print_err("failed to set ldisc on tty, ret=%d\n", ret);
        mutex_unlock(&ps_core_d->tty_mutex);
        return ret;
    }

    ps_core_d->tty_have_open = true;

    mutex_unlock(&ps_core_d->tty_mutex);
    ps_print_info("open tty success\n");

    return 0;
}

/*
 * Prototype    : release_tty_drv
 * Description  : called from PS Core when BT protocol stack drivers
 *                  unregistration,or FM/GNSS hal stack close FM/GNSS inode
 */
int32_t release_tty_drv(struct ps_core_s *ps_core_d)
{
    struct ps_plat_s *ps_plat_d = (struct ps_plat_s *)(ps_core_d->ps_plat);

    ps_print_info("%s:%s\n", __func__, get_tty_name(ps_plat_d));

    // 置位tty发送中断标志位
    atomic_set(&ps_core_d->force_tx_exit, 1);

    mutex_lock(&ps_core_d->tty_mutex);
    if (ps_core_d->tty_have_open == false) {
        atomic_set(&ps_core_d->force_tx_exit, 0);
        ps_print_info("hisi bfgx line discipline have uninstalled, ignored\n");
        mutex_unlock(&ps_core_d->tty_mutex);
        return 0;
    }

    ps_clean_tx_skb_buf(ps_core_d);

    /* close tty */
    ps_tty_kclose(ps_core_d);
    atomic_set(&ps_core_d->force_tx_exit, 0);
    ps_core_d->tty_have_open = false;

    ps_plat_d->flow_cntrl = FLOW_CTRL_ENABLE;
    ps_plat_d->baud_rate = g_uart_default_baud_rate;

    ps_print_info("close tty success\n");
    mutex_unlock(&ps_core_d->tty_mutex);
    return 0;
}

STATIC struct tty_ldisc_ops g_ps_ldisc_ops = {
    .magic = TTY_LDISC_MAGIC,
    .name = "n_ps",
    .open = ps_tty_open,
    .close = ps_tty_close,
    .receive_buf = ps_tty_receive,
    .write_wakeup = ps_tty_wakeup,
    .flush_buffer = ps_tty_flush_buffer,
    .owner = THIS_MODULE
};

STATIC struct tty_ldisc_ops g_ps_me_ldisc_ops = {
    .magic = TTY_LDISC_MAGIC,
    .name = "n_ps_me",
    .open = ps_me_tty_open,
    .close = ps_tty_close,
    .receive_buf = ps_me_tty_receive,
    .write_wakeup = ps_tty_wakeup,
    .flush_buffer = ps_tty_flush_buffer,
    .owner = THIS_MODULE
};

int32_t plat_uart_init(int index)
{
#ifdef N_HW_BFG
    if (index == BUART) {
        return tty_register_ldisc(N_HW_BFG, &g_ps_ldisc_ops);
    } else if (index == GUART) {
        return tty_register_ldisc(N_HW_GNSS, &g_ps_me_ldisc_ops);
    } else {
        ps_print_err("uart init, index:%d not support\n", index);
        return -OAL_FAIL;
    }
#else
    return OAL_SUCC;
#endif
}

int32_t plat_uart_exit(int index)
{
#ifdef N_HW_BFG
    if (index == BUART) {
        return tty_unregister_ldisc(N_HW_BFG);
    } else if (index == GUART) {
        return tty_unregister_ldisc(N_HW_GNSS);
    } else {
        ps_print_err("uart exit, index:%d not support\n", index);
        return -OAL_FAIL;
    }
#else
    return OAL_SUCC;
#endif
}
