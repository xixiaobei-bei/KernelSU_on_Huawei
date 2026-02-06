

/* Header File Including */
/*lint -e322*/ /*lint -e7*/
#include "plat_uart.h"

#include <linux/tty.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>

#include "board.h"
#include "plat_debug.h"
#include "plat_pm.h"
#include "hw_bfg_ps.h"
#include "bfgx_user_ctrl.h"

#include "securec.h"
#include "oal_ext_if.h"

/* Global Variable Definition */
STATIC struct ps_uart_state_s g_uart_state = {ATOMIC_INIT(0), ATOMIC_INIT(0), 0, 0, {0}};
STATIC struct ps_uart_state_s g_uart_state_pre = {ATOMIC_INIT(0), ATOMIC_INIT(0), 0, 0, {0}};
uint32 g_default_baud_rate = DEFAULT_BAUD_RATE;
struct mutex g_ps_tty_mutex;
STATIC oal_spin_lock_stru g_uart_state_spinlock;


/* Function Definition */
/* no lock while getting the state, just statistic */
/* call only in one place!!! */
void ps_uart_tty_tx_add(uint32 cnt)
{
    oal_atomic_add(&(g_uart_state.tty_tx_cnt), cnt);
}

/* call only in one place!!! */
STATIC void ps_uart_tty_rx_add(uint32 cnt)
{
    oal_atomic_add(&(g_uart_state.tty_rx_cnt), cnt);
}

uint32 ps_uart_state_cur(uint32 index)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    struct ps_core_s *ps_core_d = NULL;
    struct uart_state *state = NULL;

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->tty == NULL) || (ps_core_d->tty->driver_data == NULL))) {
        ps_print_err("ps_core_d ot tty is NULL\n");
        return 0;
    }

    state = ps_core_d->tty->driver_data;
    if (unlikely(state->uart_port == NULL)) {
        ps_print_err("uart_port is NULL\n");
        return 0;
    }
    switch (index) {
        case STATE_TTY_TX:
            return oal_atomic_read(&(g_uart_state.tty_tx_cnt));
        case STATE_TTY_RX:
            return oal_atomic_read(&(g_uart_state.tty_rx_cnt));
        case STATE_UART_TX:
            return state->uart_port->icount.tx;
        case STATE_UART_RX:
            return state->uart_port->icount.rx;
        default:
            ps_print_err("not support index\n");
            break;
    }
#else
#warning ps_uart_state_cur need adapt
#endif
    return 0;
}

STATIC void ps_uart_state_get(struct tty_struct *tty)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
    oal_uint irq_flag;
    struct uart_state *state = NULL;

    if (unlikely((tty == NULL) || (tty->driver_data == NULL))) {
        ps_print_dbg("tty_struct or driver_data is NULL\n");
        return;
    }

    // 商用版本不打印uart状态
    if (is_hi110x_debug_type() == OAL_FALSE) {
        return;
    }

    oal_spin_lock_irq_save(&g_uart_state_spinlock, &irq_flag);
    state = tty->driver_data;
    if (unlikely(state->uart_port == NULL)) {
        oal_spin_unlock_irq_restore(&g_uart_state_spinlock, &irq_flag);
        ps_print_err("uart_port is NULL\n");
        return;
    }

    memcpy_s(&g_uart_state.uart_cnt, sizeof(struct uart_icount), &state->uart_port->icount, sizeof(struct uart_icount));
    g_uart_state.tty_stopped = tty->stopped;
    g_uart_state.tty_hw_stopped = tty->hw_stopped;
    memcpy_s(&g_uart_state_pre, sizeof(struct ps_uart_state_s), &g_uart_state, sizeof(struct ps_uart_state_s));
    oal_spin_unlock_irq_restore(&g_uart_state_spinlock, &irq_flag);
#else
#warning ps_uart_state_get need adapt
#endif
    return;
}

STATIC void ps_uart_state_print(struct ps_uart_state_s *state)
{
    oal_uint irq_flag;
    struct ps_uart_state_s cur_state;

    oal_spin_lock_irq_save(&g_uart_state_spinlock, &irq_flag);
    if (unlikely(state == NULL)) {
        oal_spin_unlock_irq_restore(&g_uart_state_spinlock, &irq_flag);
        ps_print_err("state is NULL\n");
        return;
    }

    memcpy_s(&cur_state, sizeof(struct ps_uart_state_s), state, sizeof(struct ps_uart_state_s));
    oal_spin_unlock_irq_restore(&g_uart_state_spinlock, &irq_flag);

    ps_print_info(" tty tx:%x    rx:%x\n", oal_atomic_read(&(cur_state.tty_tx_cnt)),
                  oal_atomic_read(&(cur_state.tty_rx_cnt)));
    ps_print_info("uart tx:%x    rx:%x\n", cur_state.uart_cnt.tx, cur_state.uart_cnt.rx);
    ps_print_info("stopped:%x  hw_stopped:%x\n", cur_state.tty_stopped, cur_state.tty_hw_stopped);
    ps_print_info("uart cts:%x,dsr:%x,rng:%x,dcd:%x,frame:%x,overrun:%x,parity:%x,brk:%x,buf_overrun:%x\n",
                  cur_state.uart_cnt.cts, cur_state.uart_cnt.dsr, cur_state.uart_cnt.rng,
                  cur_state.uart_cnt.dcd, cur_state.uart_cnt.frame,
                  cur_state.uart_cnt.overrun, cur_state.uart_cnt.parity,
                  cur_state.uart_cnt.brk, cur_state.uart_cnt.buf_overrun);

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_UART, CHR_PLAT_DRV_ERROR_UART_PRINT);

    return;
}

void ps_uart_state_pre(struct tty_struct *tty)
{
    ps_uart_state_get(tty);
    return;
}

void ps_uart_state_dump(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    struct uart_state *state = NULL;
#endif

    // 商用版本不打印uart状态
    if (is_hi110x_debug_type() == OAL_FALSE) {
        return;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (tty == NULL))) {
        ps_print_err("ps_core_d ot tty is NULL\n");
        return;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    state = tty->driver_data;
    if (unlikely(state->uart_port == NULL)) {
        ps_print_err("uart_port is NULL\n");
        return;
    }
#endif

    ps_print_info("===pre uart&tty state===\n");
    ps_uart_state_print(&g_uart_state_pre);
    ps_print_info("===cur uart&tty state===\n");
    ps_uart_state_get(tty);
    ps_uart_state_print(&g_uart_state);
    ps_print_info("chars in tty tx buf len=%x\n", tty_chars_in_buffer(ps_core_d->tty));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    ps_print_info("uart port mctrl:0x%x\n", state->uart_port->mctrl);
    ps_print_info("DR   :0x%x\n", readw(state->uart_port->membase + 0x00));
    ps_print_info("FR   :0x%x\n", readw(state->uart_port->membase + 0x18));
    ps_print_info("IBRD :0x%x\n", readw(state->uart_port->membase + 0x24));
    ps_print_info("FBRD :0x%x\n", readw(state->uart_port->membase + 0x28));
    ps_print_info("LCR_H:0x%x\n", readw(state->uart_port->membase + 0x2C));
    ps_print_info("CR   :0x%x\n", readw(state->uart_port->membase + 0x30));
    ps_print_info("IFLS :0x%x\n", readw(state->uart_port->membase + 0x34));
    ps_print_info("IMSC :0x%x\n", readw(state->uart_port->membase + 0x38));
    ps_print_info("RIS  :0x%x\n", readw(state->uart_port->membase + 0x3C));
    ps_print_info("MIS  :0x%x\n", readw(state->uart_port->membase + 0x40));
#endif

    return;
}

/*
 * Prototype    : ps_tty_complete
 * Description  : to signal completion of line discipline installation
 *                  called from PS Core, upon tty_open.
 */
int32 ps_tty_complete(void *pm_data, uint8 install)
{
#if defined(_PRE_NON_OCTTY) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    /* use ktty */
#else
    struct ps_plat_s *ps_plat_d = (struct ps_plat_s *)pm_data;

    switch (install) {
        case TTY_LDISC_UNINSTALL:
            complete(&ps_plat_d->ldisc_uninstalled);
            break;

        case TTY_LDISC_INSTALL:
            complete(&ps_plat_d->ldisc_installed);
            break;

        case TTY_LDISC_RECONFIG:
            complete(&ps_plat_d->ldisc_reconfiged);
            break;

        default:
            ps_print_err("unknown install type [%d]\n", install);
            break;
    }
#endif

    return 0;
}

/*
 * Prototype    : ps_tty_open
 * Description  : called by tty uart itself when open tty uart from octty
 * input        : tty -> have opened tty
 */
STATIC int32 ps_tty_open(struct tty_struct *tty)
{
    struct ps_core_s *ps_core_d = NULL;
#if defined(_PRE_NON_OCTTY) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    /* use ktty */
#else
    uint8 install;
    struct ps_plat_s *ps_plat_d = NULL;
#endif

    ps_print_info("%s enter\n", __func__);

    ps_get_core_reference(&ps_core_d);
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

#if defined(_PRE_NON_OCTTY) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    /* use ktty */
#else
    ps_plat_d = (struct ps_plat_s *)ps_core_d->pm_data;
    install = ps_plat_d->ldisc_install;
    if (install == TTY_LDISC_INSTALL) {
        ps_tty_complete(ps_core_d->pm_data, TTY_LDISC_INSTALL);
        ps_print_info("install complete done!\n");
    } else if (install == TTY_LDISC_RECONFIG) {
        ps_tty_complete(ps_core_d->pm_data, TTY_LDISC_RECONFIG);
        ps_print_info("reconfig complete done!\n");
    } else {
        ps_print_err("ldisc_install [%d] is error!\n", install);
    }
#endif

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

    ps_print_info("%s: entered!!!\n", __func__);

    if ((tty == NULL) || (tty->disc_data == NULL)) {
        ps_print_err("tty or tty->disc_data is NULL\n");
        return;
    }

    ps_core_d = tty->disc_data;

    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);
    ps_core_d->tty = NULL;

    /* signal to complate that N_HW_BFG ldisc is un-installed */
    ps_tty_complete(ps_core_d->pm_data, TTY_LDISC_UNINSTALL);

    ps_print_info("uninstall complete done!\n");
}

/*
 * Prototype    : ps_tty_receive
 * Description  : called by tty uart when recive data from tty uart
 * input        : tty   -> have opened tty
 *                data -> recive data ptr
 *                count-> recive data count
 */
STATIC void ps_tty_receive(struct tty_struct *tty, const uint8 *data,
                           int8 *tty_flags, int32 count)
{
#ifdef PLATFORM_DEBUG_ENABLE
    struct timeval tv;
    struct rtc_time tm;
    int32 ret;
    uint64 tmp;
    const uint32 ul_filename_len = 60;
    char filename[ul_filename_len];
    mm_segment_t fs = {0};
#endif
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (unlikely((tty == NULL) || (tty->disc_data == NULL) || (tty_recv == NULL))) {
        ps_print_err("tty or tty->disc_data or tty_recv is NULL\n");
        return;
    }
    ps_core_d = tty->disc_data;
    spin_lock(&ps_core_d->rx_lock);
#ifdef PLATFORM_DEBUG_ENABLE
    if (g_uart_rx_dump) {
        ps_core_d->curr_time = jiffies;
        tmp = ps_core_d->curr_time - ps_core_d->pre_time;
        if ((tmp > DBG_FILE_TIME * HZ) || (ps_core_d->pre_time == 0)) {
            if (!OAL_IS_ERR_OR_NULL(ps_core_d->rx_data_fp)) {
                filp_close(ps_core_d->rx_data_fp, NULL);
            }
            do_gettimeofday(&tv);
            rtc_time_to_tm(tv.tv_sec, &tm);
            ret = snprintf_s(filename, sizeof(filename), sizeof(filename) - 1,
                             "/data/hwlogdir/uart_rx/uart_rx-%04d-%02d-%02d:%02d-%02d-%02d",
                             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                             tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
            if (ret < 0) {
                ps_uart_tty_rx_add(count);
                tty_recv(tty->disc_data, data, count);

                spin_unlock(&ps_core_d->rx_lock);
                ps_print_err("log str format err line[%d]\n", __LINE__);
                return;
            }
            ps_print_info("[ps_uart_dump]filename = %s", filename);

            ps_core_d->rx_data_fp = filp_open(filename, O_RDWR | O_CREAT, 0777);

            if (OAL_IS_ERR_OR_NULL(ps_core_d->rx_data_fp)) {
                ps_core_d->rx_data_fp = NULL;
                ps_print_err("[ps_uart_dump]create file error,fp = 0x%p\n", ps_core_d->rx_data_fp);
            }

            ps_core_d->pre_time = ps_core_d->curr_time;
        }

        if (ps_core_d->rx_data_fp != NULL) {
            fs = get_fs();
            set_fs(KERNEL_DS);
            ret = vfs_write(ps_core_d->rx_data_fp, data, count, &ps_core_d->rx_data_fp->f_pos);
            if (ret < 0) {
                ps_print_err("[ps_uart_dump] vfs write failed \n");
            }
            set_fs(fs);
        } else {
            ps_print_warning("[ps_uart_dump]uart rx dump dir not make or uart not translate data\n");
        }
    }
#endif

    ps_uart_tty_rx_add(count);
    tty_recv(tty->disc_data, data, count);

    spin_unlock(&ps_core_d->rx_lock);
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
    if ((tty == NULL) || (tty->disc_data == NULL)) {
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
    uint8 delay_times = RELEASE_DELAT_TIMES;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    ps_kfree_skb(ps_core_d, TX_URGENT_QUEUE);
    ps_kfree_skb(ps_core_d, TX_HIGH_QUEUE);
    ps_kfree_skb(ps_core_d, TX_LOW_QUEUE);

    ps_print_info("free tx sbk buf done!\n");

    /* clean all tx sk_buff */
    while (((ps_core_d->tx_urgent_seq.qlen) || (ps_core_d->tx_high_seq.qlen) ||
            (ps_core_d->tx_low_seq.qlen)) && (delay_times)) {
        msleep(10);
        delay_times--;
    }

    if (oal_work_is_busy(&ps_core_d->tx_skb_work)) {
        ps_print_info("hisi bfgx notify tx work exit\n");
        atomic_set(&ps_core_d->force_tx_exit, 1);
        // wait for tx work exit
        msleep(WAIT_TX_WORK_DELAY);
    }

    if (ps_core_d->tty != NULL) {
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
    ps_print_info("time to %s\n", __func__);

    reset_uart_rx_buf();

    return;
}

#if defined(_PRE_NON_OCTTY) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
/* use ktty */
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
    if (OAL_IS_ERR_OR_NULL(tty)) {
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
#endif
}

static void ps_tty_kclose(struct ps_core_s *ps_core_d)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    struct tty_struct *tty = ps_core_d->tty;

    ps_print_info("%s\n", __func__);

    mutex_lock(&g_ps_tty_mutex);
    /* close tty */
    if (tty == NULL) {
        mutex_unlock(&g_ps_tty_mutex);
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
    mutex_unlock(&g_ps_tty_mutex);
#endif
}

/* ps_set_tty_termios ttyUart in kernel driver */
/*
 * Prototype    : ps_set_tty_termios
 * Description  : set uart attr
 */
static void ps_set_tty_termios(struct tty_struct *tty, int64 baud_rate, uint8 enable_flowctl)
{
    struct ktermios ktermios;

    ps_print_info("%s\n", __func__);

    mutex_lock(&g_ps_tty_mutex);

    if (tty == NULL) {
        mutex_unlock(&g_ps_tty_mutex);
        ps_print_err("tty is NULL\n");
        return;
    }

    ktermios = tty->termios;

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

    mutex_unlock(&g_ps_tty_mutex);

    ps_print_info("set baud_rate=%d, except=%d\n", (int)tty_termios_baud_rate(&tty->termios), (int)baud_rate);
}

/* remove octty process access ttyUart in kernel driver */
/*
 * Prototype    : ps_change_uart_baud_rate
 * Description  : change arm platform uart baud rate to secend
 *                baud rate for high baud rate when download patch
 */
int32 ps_change_uart_baud_rate(int64 baud_rate, uint8 enable_flowctl)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft = 0;
    struct st_exception_info *pst_exception_data = NULL;

    ps_print_info("%s %lu\n", __func__, baud_rate);

    ps_get_plat_reference(&ps_plat_d);
    if (unlikely(ps_plat_d == NULL)) {
        ps_print_err("ps_plat_d is NULL\n");
        return -EINVAL;
    }

    ps_core_d = ps_plat_d->core_data;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* for debug only */
    dump_uart_rx_buf();

    get_exception_info_reference(&pst_exception_data);

    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EINVAL;
    }

    if (atomic_read(&pst_exception_data->is_memdump_runing) == 1) {
        oal_reinit_completion(pst_exception_data->wait_read_bfgx_stack);
        /* wait for read stack completion */
        timeleft = wait_for_completion_timeout(&pst_exception_data->wait_read_bfgx_stack,
                                               msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
        if (!timeleft) {
            ps_uart_state_dump(ps_core_d->tty);
            atomic_set(&pst_exception_data->is_memdump_runing, 0);
            ps_print_err("read bfgx stack failed!\n");
        } else {
            ps_print_info("read bfgx stack success!\n");
        }
    }

    if (!OAL_IS_ERR_OR_NULL(ps_core_d->tty)) {
        if (tty_chars_in_buffer(ps_core_d->tty)) {
            ps_print_info("uart tx buf is not empty\n");
            tty_driver_flush_buffer(ps_core_d->tty);
        }
    }

    ps_plat_d->flow_cntrl = enable_flowctl;
    ps_plat_d->baud_rate = baud_rate;
    ps_plat_d->ldisc_install = TTY_LDISC_RECONFIG;

    ps_print_info("ldisc_install = %d\n", TTY_LDISC_RECONFIG);

    ps_set_tty_termios(ps_core_d->tty, ps_plat_d->baud_rate, ps_plat_d->flow_cntrl);

    ps_print_suc("hisi bfgx ldisc reconfig succ\n");

    return 0;
}

static char *get_tty_name(struct ps_plat_s *pm_data)
{
    struct ps_plat_s *ps_plat_d = NULL;
    char *path = "/dev/";
    char *tty_dev_name = NULL;

    ps_plat_d = (struct ps_plat_s *)pm_data;

    tty_dev_name = ps_plat_d->dev_name + strlen(path);

    ps_print_info("tty dev name is %s\n", tty_dev_name);

    return tty_dev_name;
}

/*
 * Prototype    : open_tty_drv_etc
 * Description  : called from PS Core when BT protocol stack drivers
 *                registration,or FM/GNSS hal stack open FM/GNSS inode
 */
int32 open_tty_drv(struct ps_plat_s *pm_data)
{
    int ret;
    struct tty_struct *tty = NULL;
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;
    char *tty_name = NULL;

    ps_print_info("%s\n", __func__);

    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL\n");
        return -EINVAL;
    }

    tty_name = get_tty_name(pm_data);

    ps_plat_d = (struct ps_plat_s *)pm_data;
    ps_core_d = ps_plat_d->core_data;
    if (ps_core_d->tty_have_open == true) {
        ps_print_warning("hisi bfgx line discipline have installed\n");
        return 0;
    }

    reset_uart_rx_buf();

    if (oal_atomic_read(&g_ir_only_mode) != 0) {
        /* ir only mode use baudrate 921600 */
        ps_plat_d->baud_rate = IR_ONLY_BAUD_RATE;
    }

    /* open tty */
    tty = ps_tty_kopen(tty_name);
    if (tty == NULL) {
        ps_print_err("failed to open N_HW_BFG tty\n");
        hw_1102a_bt_dsm_client_notify(DSM_1102A_UART_OPEN_FAIL, "%s: open uart failed\n", __FUNCTION__);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_UART, CHR_PLAT_DRV_ERROR_OPEN_TTY);
        return -ENODEV;
    }

    ps_set_tty_termios(tty, ps_plat_d->baud_rate, ps_plat_d->flow_cntrl);

    // tty_lock在tty_kopen时执行，成功时不释放锁，只有失败的时候才会释放
    tty_unlock(tty);

    ps_print_info("start tty set ldisc num=%d\n", N_HW_BFG);

    /* set line ldisc */
    ret = tty_set_ldisc(tty, N_HW_BFG); /* export after 4.13 */
    if (ret != 0) {
        ps_print_err("failed to set N_HW_BFG on tty, ret=%d\n", ret);
        return ret;
    }

    ps_core_d->tty_have_open = true;

    ps_print_info("open tty success\n");

    return 0;
}

/*
 * Prototype    : release_tty_drv_etc
 * Description  : called from PS Core when BT protocol stack drivers
 *                  unregistration,or FM/GNSS hal stack close FM/GNSS inode
 */
int32 release_tty_drv(struct ps_plat_s *pm_data)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;

    ps_print_info("%s\n", __func__);

    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL");
        return -EINVAL;
    }

    ps_plat_d = (struct ps_plat_s *)pm_data;
    ps_core_d = ps_plat_d->core_data;

    if (ps_core_d->tty_have_open == false) {
        ps_print_info("hisi bfgx line discipline have uninstalled, ignored\n");
        return 0;
    }

    ps_clean_tx_skb_buf(ps_core_d);

    /* close tty */
    ps_tty_kclose(ps_core_d);
    atomic_set(&ps_core_d->force_tx_exit, 0);

    ps_core_d->tty_have_open = false;
    ps_plat_d->flow_cntrl = FLOW_CTRL_ENABLE;
    ps_plat_d->baud_rate = g_default_baud_rate;

    ps_print_info("close tty success\n");

    return 0;
}
#else

/*
 * Prototype    : ps_change_uart_baud_rate
 * Description  : change arm platform uart baud rate to secend
 *                baud rate for high baud rate when download patch
 * input        : baud_rate，enable_flowctl
 */
int32 ps_change_uart_baud_rate(int64 baud_rate, uint8 enable_flowctl)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft = 0;
    struct st_exception_info *pst_exception_data = NULL;

    ps_print_info("%s %lu\n", __func__, baud_rate);

    ps_get_plat_reference(&ps_plat_d);
    if (unlikely(ps_plat_d == NULL)) {
        ps_print_err("ps_plat_d is NULL\n");
        return -EINVAL;
    }

    ps_core_d = ps_plat_d->core_data;
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* for debug only */
    dump_uart_rx_buf();

    get_exception_info_reference(&pst_exception_data);

    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EINVAL;
    }

    if (atomic_read(&pst_exception_data->is_memdump_runing) == 1) {
        oal_reinit_completion(pst_exception_data->wait_read_bfgx_stack);
        /* wait for read stack completion */
        timeleft = wait_for_completion_timeout(&pst_exception_data->wait_read_bfgx_stack,
                                               msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
        if (!timeleft) {
            ps_uart_state_dump(ps_core_d->tty);
            atomic_set(&pst_exception_data->is_memdump_runing, 0);
            ps_print_err("read bfgx stack failed!\n");
        } else {
            ps_print_info("read bfgx stack success!\n");
        }
    }

    if (!OAL_IS_ERR_OR_NULL(ps_core_d->tty)) {
        if (tty_chars_in_buffer(ps_core_d->tty)) {
            ps_print_info("uart tx buf is not empty\n");
            tty_driver_flush_buffer(ps_core_d->tty);
        }
    }

    oal_reinit_completion(ps_plat_d->ldisc_reconfiged);
    ps_plat_d->flow_cntrl = enable_flowctl;
    ps_plat_d->baud_rate = baud_rate;
    ps_plat_d->ldisc_install = TTY_LDISC_RECONFIG;

    ps_print_info("ldisc_install = %d\n", TTY_LDISC_RECONFIG);
    sysfs_notify(g_sysfs_hi110x_bfgx, NULL, "install");
    timeleft = wait_for_completion_timeout(&ps_plat_d->ldisc_reconfiged, msecs_to_jiffies(HISI_LDISC_TIME));
    if (!timeleft) {
        ps_print_err("hisi bfgx ldisc reconfig timeout\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_UART, CHR_PLAT_DRV_ERROR_UART_BAURD);

        return -EINVAL;
    }

    ps_print_suc("hisi bfgx ldisc reconfig succ\n");

    return 0;
}

/*
 * Prototype    : open_tty_drv
 * Description  : called from PS Core when BT protocol stack drivers
 *                  registration,or FM/GNSS hal stack open FM/GNSS inode
 * output       : return 0--> open tty uart is ok
 *                return !0-> open tty uart is false
 */
int32 open_tty_drv(struct ps_plat_s *pm_data)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;
    uint8 retry = OPEN_TTY_RETRY_COUNT;
    uint64 timeleft = 0;

    ps_print_dbg("%s\n", __func__);

    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL\n");
        return -EINVAL;
    }

    ps_plat_d = (struct ps_plat_s *)pm_data;
    ps_core_d = ps_plat_d->core_data;
    if (ps_core_d->tty_have_open == true) {
        ps_print_dbg("hisi bfgx line discipline have installed\n");
        return 0;
    }

    reset_uart_rx_buf();

    do {
        oal_reinit_completion(ps_plat_d->ldisc_installed);
        ps_plat_d->ldisc_install = TTY_LDISC_INSTALL;
        if (oal_atomic_read(&g_ir_only_mode) != 0) {
            /* ir only mode use baudrate 921600 */
            ps_plat_d->baud_rate = IR_ONLY_BAUD_RATE;
        }

        ps_print_info("ldisc_install = %d,baud=%lu\n", TTY_LDISC_INSTALL, ps_plat_d->baud_rate);
        sysfs_notify(g_sysfs_hi110x_bfgx, NULL, "install");
        timeleft = wait_for_completion_timeout(&ps_plat_d->ldisc_installed, msecs_to_jiffies(HISI_LDISC_TIME));
        if (!timeleft) {
            ps_print_err("hisi bfgx ldisc installation timeout\n");
            oal_warn_on(1);
            continue;
        } else {
            ps_print_suc("hisi bfgx line discipline install succ\n");
            ps_core_d->tty_have_open = true;
            return 0;
        }
    } while (retry--);

    hw_1102a_bt_dsm_client_notify(DSM_1102A_UART_OPEN_FAIL, "%s: open uart failed\n", __FUNCTION__);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_UART, CHR_PLAT_DRV_ERROR_OPEN_TTY);

    return -EPERM;
}

/*
 * Prototype    : release_tty_drv
 * Description  : called from PS Core when BT protocol stack drivers
 *                  unregistration,or FM/GNSS hal stack close FM/GNSS inode
 * input        : pm_data ptr
 * output       : return 0--> open tty uart is ok
 *                return !0-> open tty uart is false
 */
int32 release_tty_drv(struct ps_plat_s *pm_data)
{
    int32 error;
    struct ps_plat_s *ps_plat_d = NULL;
    struct tty_struct *tty = NULL;
    struct ps_core_s *ps_core_d = NULL;
    uint64 timeleft;

    ps_print_info("%s\n", __func__);

    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL");
        return -EINVAL;
    }

    ps_plat_d = (struct ps_plat_s *)pm_data;
    ps_core_d = ps_plat_d->core_data;

    if (ps_core_d->tty_have_open == false) {
        ps_print_info("hisi bfgx line discipline have uninstalled, ignored\n");
        return 0;
    }

    ps_clean_tx_skb_buf(ps_core_d);

    mutex_lock(&g_ps_tty_mutex);
    atomic_set(&ps_core_d->force_tx_exit, 0);
    tty = ps_core_d->tty;
    if (tty != NULL) { /* can be called before ldisc is installed */
        /* Flush any pending characters in the driver and discipline. */
        ps_print_info(" %s--> into flush_buffer\n", __func__);
        tty_ldisc_flush(tty);
        tty_driver_flush_buffer(tty);
    }
    mutex_unlock(&g_ps_tty_mutex);

    oal_reinit_completion(ps_plat_d->ldisc_uninstalled);
    ps_plat_d->ldisc_install = TTY_LDISC_UNINSTALL;

    ps_print_info("ldisc_install = %d\n", TTY_LDISC_UNINSTALL);
    sysfs_notify(g_sysfs_hi110x_bfgx, NULL, "install");

    timeleft = wait_for_completion_timeout(&ps_plat_d->ldisc_uninstalled, msecs_to_jiffies(HISI_LDISC_TIME));
    if (!timeleft) {
        ps_print_err("hisi bfgx ldisc uninstall timeout\n");
        error = -ETIMEDOUT;
    } else {
        ps_print_suc("hisi bfgx line discipline uninstall succ\n");
        error = 0;
    }

    ps_core_d->tty_have_open = false;
    ps_plat_d->flow_cntrl = FLOW_CTRL_ENABLE;
    ps_plat_d->baud_rate = g_default_baud_rate;

    return error;
}
#endif

int32 is_tty_open(void *pm_data)
{
    struct ps_plat_s *ps_plat_d = NULL;
    struct ps_core_s *ps_core_d = NULL;

    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL");
        return -EINVAL;
    }

    ps_plat_d = (struct ps_plat_s *)pm_data;
    ps_core_d = ps_plat_d->core_data;

    if (ps_core_d->tty_have_open == false) {
        ps_print_info("hisi bfgx line discipline have uninstalled\n");
        return -EINVAL;
    }
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

int32 plat_uart_init(void)
{
    mutex_init(&g_ps_tty_mutex);
    oal_spin_lock_init(&g_uart_state_spinlock);

#ifdef N_HW_BFG
    return tty_register_ldisc(N_HW_BFG, &g_ps_ldisc_ops);
#else
    return OAL_SUCC;
#endif
}

int32 plat_uart_exit(void)
{
#ifdef N_HW_BFG
    return tty_unregister_ldisc(N_HW_BFG);
#else
    return OAL_SUCC;
#endif
}
