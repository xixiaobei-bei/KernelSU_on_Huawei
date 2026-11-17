#ifndef __HI_GPIO_H__
#define __HI_GPIO_H__ 
#include <osl_bio.h>
#define HI_K3_GPIO 
#define GPIO_PL061 
#define MODEM_UART_NEED_SELECT_BY_GPIO 
#define GPIODIR 0x400
#define GPIOIS 0x404
#define GPIOIBE 0x408
#define GPIOIEV 0x40C
#define GPIOIE 0x410
#define GPIOIE1 0x500
#define GPIOIE2 0x504
#define GPIOIE3 0x508
#define GPIOIE4 0x50c
#define GPIORIS 0x414
#define GPIOMIS 0x418
#define GPIOMIS1 0x530
#define GPIOMIS2 0x534
#define GPIOIC 0x41C
#define GPIO_MIS_CCORE GPIOMIS2
#define GPIO_IE_CCORE GPIOIE2
#define PL061_GPIO_NR 8
#define GPIO_MAX_GROUP 34
#define GPIO_MAX_NUMBER 8
#define GPIO_GROUP_BIT 3
#define GPIO_TOTAL 223
#define GPIO_TEST_NUM (206)
#define MODEM_UART_ACTIVE (1 << 5)
#define UART_GPIODIR_ADDR (0xe8a0c400)
#define UART_GPIODATA_ADDR (0xe8a0c3FC)
static inline void bsp_gpio_set_modem_uart_active(void)
{
    writel(0x7 << 4,(void*)UART_GPIODIR_ADDR);
    writel(MODEM_UART_ACTIVE, (void*)UART_GPIODATA_ADDR);
}
#endif
