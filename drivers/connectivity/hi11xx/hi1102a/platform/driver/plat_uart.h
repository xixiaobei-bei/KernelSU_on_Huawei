

#ifndef __PLAT_UART_H__
#define __PLAT_UART_H__

/* Include other Head file */
#include <linux/serial_core.h>
#include "plat_type.h"
#include "hw_bfg_ps.h"

typedef enum {
    STATE_TTY_TX = 0,
    STATE_TTY_RX = 1,
    STATE_UART_TX = 2,
    STATE_UART_RX = 3,
} uart_state_index;

/* STRUCT DEFINE */
struct ps_uart_state_s {
    atomic_t tty_tx_cnt;
    atomic_t tty_rx_cnt;
    uint32 tty_stopped;    /* tty 软件流控标志位 */
    uint32 tty_hw_stopped; /* tty 硬件流控标志位 */
    struct uart_icount uart_cnt;
};

/* EXTERN VARIABLE */
extern uint32 g_default_baud_rate;
extern struct mutex g_ps_tty_mutex;

/* EXTERN FUNCTION */
extern int32 plat_uart_init(void);
extern int32 plat_uart_exit(void);
extern int32 open_tty_drv(struct ps_plat_s *pm_data);
extern int32 release_tty_drv(struct ps_plat_s *pm_data);
extern int32 ps_change_uart_baud_rate(int64 baud_rate, uint8 enable_flowctl);
extern void ps_uart_tty_tx_add(uint32 cnt);
extern void ps_uart_state_pre(struct tty_struct *tty);
extern void ps_uart_state_dump(struct tty_struct *tty);
extern uint32 ps_uart_state_cur(uint32 index);
extern int32 is_tty_open(void *pm_data);
#endif
