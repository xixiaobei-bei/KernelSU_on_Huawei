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
#define GPIO_MAX_GROUP 31
#define GPIO_MAX_NUMBER 8
#define GPIO_GROUP_BIT 3
#define GPIO_TOTAL 223
#define GPIODIR 0x400
#define GPIO_TEST_NUM (206)
#define HI_PINTRL_AO_REG 0xFFF11000
static inline void bsp_gpio_set_modem_uart_active(void)
{
    writel(2,(void*)HI_PINTRL_AO_REG+0x07c);
    writel(2,(void*)HI_PINTRL_AO_REG+0x078);
}
#endif
