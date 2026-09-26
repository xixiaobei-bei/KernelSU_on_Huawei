

#include "bfgx_dev.h"
#include "board.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "securec.h"

STATIC void uart_baudrate_set_by_ini(void)
{
    int32_t ret;
    int32_t ini_baudrate = 0;
    const int32_t baudrate_map[INI_UART_RATE_NUM] = {
        0, // 只占位（芯片寄存器默认为0 ，ini文件配置值从1开始）
        UART_BAUD_RATE_DBG,
        UART_BAUD_RATE_1M,
        UART_BAUD_RATE_2M,
        UART_BAUD_RATE_3M,
        UART_BAUD_RATE_4M,
        UART_BAUD_RATE_5M,
        UART_BAUD_RATE_6M,
        UART_BAUD_RATE_7M,
        UART_BAUD_RATE_8M
    };

    ret = get_cust_conf_int32(INI_MODU_PLAT, "uart_rate_value", &ini_baudrate);
    if (ret == INI_FAILED) {
        ps_print_info("can't found uart rate ini");
        return;
    }

    if ((ini_baudrate >= INI_UART_RATE_DBG) && (ini_baudrate < INI_UART_RATE_NUM)) {
        bfgx_uart_baud_rate_set(baudrate_map[ini_baudrate]);
    }
}

void baudrate_init(void)
{
    if (!hi110x_is_asic()) {
        bfgx_uart_baud_rate_set(UART_BAUD_RATE_2M);
    } else {
        if (get_hi110x_subchip_type() >= BOARD_VERSION_HI1106) {
            /* Hi1106回片首先使用2M，冒烟后提频 */
            bfgx_uart_baud_rate_set(UART_BAUD_RATE_2M);
        } else if (get_hi110x_subchip_type() >= BOARD_VERSION_HI1105) {
            /* 从hi1105开始，波特率支持到8M，其他项目为4M */
            bfgx_uart_baud_rate_set(UART_BAUD_RATE_8M);
        } else {
            bfgx_uart_baud_rate_set(UART_BAUD_RATE_4M);
        }
        uart_baudrate_set_by_ini();
    }
    g_uart_default_baud_rate = (uint32_t)bfgx_uart_baud_rate_get();
    ps_print_info("init baudrate=%d\n", g_uart_default_baud_rate);
    return;
}