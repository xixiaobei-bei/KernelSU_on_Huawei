

#ifndef __PLAT_UART_H__
#define __PLAT_UART_H__

/* 其他头文件包含 */
#include <linux/serial_core.h>
#include "plat_type.h"

/* 宏定义 */
typedef enum {
    STATE_TTY_TX = 0,
    STATE_TTY_RX = 1,
    STATE_UART_TX = 2,
    STATE_UART_RX = 3,
} UART_STATE_INDEX;

/* STRUCT 定义 */
struct ps_uart_state_s {
    uint32 tty_tx_cnt;
    uint32 tty_rx_cnt;
    uint32 tty_stopped;    /* tty 软件流控标志位 */
    uint32 tty_hw_stopped; /* tty 硬件流控标志位 */
    struct uart_icount uart_cnt;
};

/* EXTERN VARIABLE */
extern uint32 default_baud_rate;
extern struct mutex tty_mutex_etc;

/* 函数声明 */
extern int32 plat_uart_init_etc(void);
extern int32 plat_uart_exit_etc(void);
extern int32 open_tty_drv_etc(void *pm_data);
extern int32 release_tty_drv_etc(void *pm_data);
extern int32 ps_change_uart_baud_rate_etc(int64 baud_rate, uint8 enable_flowctl);
extern void ps_uart_tty_tx_add_etc(uint32 cnt);
extern void ps_uart_state_pre_etc(struct tty_struct *tty);
extern void ps_uart_state_dump_etc(struct tty_struct *tty);
extern uint32 ps_uart_state_cur_etc(uint32 index);
extern int32 is_tty_open(void *pm_data);
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
extern int32 bfg_patch_recv(const uint8 *data, int32 count);
#endif
extern int32 ps_recv_patch(void *disc_data, const uint8 *data, int32 count);
#endif
