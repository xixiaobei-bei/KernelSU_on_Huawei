#ifndef HI6930_UART_H
#define HI6930_UART_H 
#define UART_USE_FLAG 0x112233
#define AT_UART_USE_FLAG 0x116119
#define PRT_FLAG_EN_MAGIC_M 0x24680136
#define PRT_FLAG_EN_MAGIC_A 0x24680137
#if 1
#include "soc_memmap.h"
#include "soc_interrupts.h"
#define ARM_UART_PERIPHERAL_ID 0x00041011
#define SYNOPSYS_UART_PERIPHERAL_ID 0x000c21c0
#define MINI_SYSTEM_UART_ADDR HI_UART1_REGBASE_ADDR
#define UART0_BASE_ADDR HI_UART0_REGBASE_ADDR
#define UART1_BASE_ADDR HI_UART1_REGBASE_ADDR
#define UART2_BASE_ADDR HI_UART2_REGBASE_ADDR
#define UART3_BASE_ADDR HI_UART_MDM_REGBASE_ADDR
#define UART0_INT_LVL INT_LVL_UART0
#define UART1_INT_LVL INT_LVL_UART1
#define UART2_INT_LVL INT_LVL_UART2
#define UART3_INT_LVL INT_LVL_UART_MDM
#define UART0_M3_INT M3_UART0_INT
#define UART1_M3_INT M3_UART1_INT
#define UART2_M3_INT M3_UART2_INT
#define UART3_M3_INT 0
#define CCORE_SYS_UART_BASE 0xE0203000
#define CCORE_SYS_UART_INTID 50
#define UART_BALONG_ID ARM_UART_PERIPHERAL_ID
#define UART_ID_ARM 0x00041011
#define UART_BALONG_ID_MASK 0x000fffff
#define UART_PRIMCELL_CLOCK 19200000
#define UART_BAUDRATE 115200
#define UART_REGOFFSET_RBR 0x00
#define UART_REGOFFSET_THR 0x00
#define UART_REGOFFSET_RSR_ECR 0x04
#define UART_REGOFFSET_FR 0x18
#define UART_REGOFFSET_IBRD 0x24
#define UART_REGOFFSET_FBRD 0x28
#define UART_REGOFFSET_LCR_H 0x2C
#define UART_REGOFFSET_CR 0x30
#define UART_REGOFFSET_IFLS 0x34
#define UART_REGOFFSET_IMSC 0x38
#define UART_REGOFFSET_MIS 0x40
#define UART_REGOFFSET_ICR 0x44
#define UART_REGOFFSET_DMACR 0x48
#define UART_FIFO_SIZE 32
#define UART_FIFO_MASK 0xF0
#define PTIME (1<<7)
#define UART_LCR_BC_NORMAL 0x00
#define UART_LCR_BC_UNNORMAL 0x01
#define UART_IFLS_DEF_SET 0x12
#define UART_DISABLE 0x00
#define UART_ENABLE 0x01
#define UART_TX_ENABLE 0x100
#define UART_RX_ENABLE 0x200
#define UART_IER_IRQ_CLR 0xFFFF
#define UART_LCR_DLS_MASK 0x03
#define UART_LCR_DLS_5BITS 0x00
#define UART_LCR_DLS_6BITS 0x20
#define UART_LCR_DLS_7BITS 0x40
#define UART_LCR_DLS_8BITS 0x60
#define UART_LCR_STOP_2BITS 0x08
#define UART_LCR_STOP_1BITS 0x00
#define UART_LCR_PEN_NONE 0x00
#define UART_LCR_ODD 0x02
#define UART_LCR_EPS 0x06
#define UART_LCR_DEFAULTVALUE 0x00
#define UART_LCR_FIFO_ENABLE 0x10
#define UART_LCR_FIFO_DISABLE 0x00
#define UART_FR_UART_BUSY 0x08
#define UART_FR_RX_FIFO_EMP 0x10
#define UART_FR_TX_FIFO_FULL 0x20
#define UART_FR_RX_FIFO_FULL 0x40
#define UART_FR_TX_FIFO_EMP 0x80
#define UART_RX_IRQ_ENABLE 0x10
#define UART_TX_IRQ_ENABLE 0x20
#define UART_TIMEOUT_IRQ_ENABLE 0x40
#define UART_ALL_IRQ_DISABLE 0x00
#define UART_IIR_REV_VALID 0x10
#define UART_IIR_THR_INTERRUPT 0x20
#define UART_IIR_REV_TIMEOUT 0x40
#ifndef __ASSEMBLY__
struct uart_info{
 unsigned int uart_base_addr;
 unsigned int uart_intid;
};
extern struct uart_info g_uart_info;
static inline void print_info(const char *buffer)
{
    int loop;
    volatile unsigned int status = 0;
    while (*buffer != '\0')
    {
        loop = 0x10000;
        while ((status != 0x10) && loop--)
        {
            status = *(volatile unsigned *)(HI_UART0_REGBASE_ADDR + UART_REGOFFSET_FR) & 0x18;
        }
  *(volatile unsigned *)(HI_UART0_REGBASE_ADDR + UART_REGOFFSET_RBR) = (unsigned int)(*buffer);
        buffer++;
    }
}
#endif
static inline void modem_uart_gpio_config(void)
{
 *(volatile unsigned *)(0xE8A1F400) = (unsigned int)(0xFF);
 *(volatile unsigned *)(0xE8A1F3FC) = (unsigned int)(0x20);
}
#define print_uart_m3boot(value) \
 ldr r1, =HI_UART0_REGBASE_ADDR;\
 ldr r2, =value;\
 ldr r3, =UART_REGOFFSET_THR;\
 str r2, [r1,r3]
#define uart_init_m3boot \
 ldr r1, =HI_UART0_REGBASE_ADDR;\
 ldr r2, =0xE0;\
 ldr r3, =UART_REGOFFSET_RSR_ECR;\
 str r2, [r1, r3];\
 ldr r2, =0x0;\
 ldr r3, =UART_REGOFFSET_IMSC;\
 str r2, [r1, r3];\
 ldr r2, =0xFFFF;\
 ldr r3, =UART_REGOFFSET_ICR;\
 str r2, [r1, r3];\
 ldr r2, =0x1A;\
 ldr r3, =UART_REGOFFSET_IBRD;\
 str r2, [r1, r3];\
 ldr r2, =0x3;\
 ldr r3, =UART_REGOFFSET_FBRD;\
 str r2, [r1, r3];\
 ldr r2, =0x70;\
 ldr r3, =UART_REGOFFSET_LCR_H;\
 str r2, [r1, r3];\
 ldr r2, =0x164;\
 ldr r3, =UART_REGOFFSET_IFLS;\
 str r2, [r1, r3];\
 ldr r2, =0x0;\
 ldr r3, =UART_REGOFFSET_DMACR;\
 str r2, [r1, r3];\
 ldr r2, =0x301;\
 ldr r3, =UART_REGOFFSET_CR;\
 str r2, [r1, r3];\
 bx lr
#endif
#endif
