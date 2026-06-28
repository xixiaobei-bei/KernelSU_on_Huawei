#ifndef __HI6250_M3_INTERRUPTS_H__
#define __HI6250_M3_INTERRUPTS_H__ 
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define IRQ_MAX 237
#define IRQ_TIMER2 237
#define IRQ_TIMER1 236
#define IRQ_GIC_OUT(cluster,core) (211 + ((cluster) << 2) + (core))
#define IRQ_MDM_IPF_INTR1 209
#define IRQ_DDRC_ERR 102
#define IRQ_DDRC_FATAL 103
#define IRQ_SOCP1 193
#define IRQ_SOCP0 192
#define IRQ_ASP_WD 188
#define IRQ_ASPIPC 185
#define IRQ_COMB_IOM3_IPC 182
#define IRQ_COMB_GIC_IPC 181
#define IRQ_IPC_MBX(n,m) (((m) < 5) ? (m + 158+ n*13) : (m + 160+ n*14))
#define IRQ_GPIO30 228
#define IRQ_GPIO29 227
#define IRQ_GPIO28 226
#define IRQ_IPC_NS_MBX28 180
#define IRQ_IPC_NS_MBX27 179
#define IRQ_IPC_NS_MBX_19_22 177
#define IRQ_IPC_NS_MBX18 176
#define IRQ_IPC_NS_MBX17 175
#define IRQ_IPC_NS_MBX16 174
#define IRQ_IPC_NS_MBX15 173
#define IRQ_IPC_NS_MBX14 172
#define IRQ_IPC_NS_MBX13 171
#define IRQ_IPC_NS_INT3 167
#define IRQ_IPC_NS_INT4 168
#define IRQ_IPC_S_MBX24 166
#define IRQ_IPC_S_MBX23 165
#define IRQ_IPC_S_MBX18 163
#define IRQ_IPC_S_MBX17 162
#define IRQ_IPC_S_MBX16 161
#define IRQ_IPC_S_MBX15 160
#define IRQ_IPC_S_MBX14 159
#define IRQ_IPC_S_MBX13 158
#define IRQ_IPC_S_INT3 155
#define IRQ_WATCHDOG 153
#define IRQ_IPCM0_2 132
#define IRQ_IPCM1_2 133
#define IRQ_CLUSTER1_PWR 125
#define IRQ_CLUSTER0_PWR 124
#define IRQ_ASP_ARM 123
#define IRQ_MCPU_HIFI 106
#define IRQ_IOMCU_WD 77
#define IRQ_GPIO27 75
#define IRQ_GPIO26 74
#define IRQ_GPIO25 73
#define IRQ_GPIO24 72
#define IRQ_GPIO23 71
#define IRQ_GPIO22 70
#define IRQ_GPIO21 69
#define IRQ_GPIO20 68
#define IRQ_GPIO19 67
#define IRQ_GPIO18 66
#define IRQ_GPIO17 65
#define IRQ_GPIO16 64
#define IRQ_GPIO15 63
#define IRQ_GPIO14 62
#define IRQ_GPIO13 61
#define IRQ_GPIO12 60
#define IRQ_GPIO11 59
#define IRQ_GPIO10 58
#define IRQ_GPIO9 57
#define IRQ_GPIO8 56
#define IRQ_GPIO7 55
#define IRQ_GPIO6 54
#define IRQ_GPIO5 53
#define IRQ_GPIO4 52
#define IRQ_GPIO3 51
#define IRQ_GPIO2 50
#define IRQ_GPIO1 49
#define IRQ_GPIO0 48
#define IRQ_UART6 43
#define IRQ_UART5 42
#define IRQ_UART4 41
#define IRQ_UART3 81
#define IRQ_IOMCU_WAKEUP 78
#define IRQ_UART2 40
#define IRQ_UART1 39
#define IRQ_UART0 38
#define IRQ_TIME71 27
#define IRQ_TIME50 22
#define IRQ_TIME41 21
#define IRQ_TIME40 20
#define IRQ_TIME31 19
#define IRQ_TIME30 18
#define IRQ_TIME21 17
#define IRQ_TIME20 16
#define IRQ_TIME11 15
#define IRQ_TIME01 13
#define IRQ_RTC1 11
#define IRQ_RTC0 10
#define IRQ_APWDOG 8
#define IRQ_MDM2AP3 7
#define IRQ_MDM2AP2 6
#define M3_IPC_M2C_INT0 1
#define M3_IPC_M2C_INT1 2
#define M3_TIMER20_INT 16
#define M3_TIMER30_INT 18
#define M3_TIMER31_INT 19
#define M3_LTE_ARM_WAKEUP_INT 195
#define M3_TDS_DRX_ARM_WAKEUP_INT 196
#define M3_G1_INT_BBP_TO_CPU_32K 199
#define M3_G2_INT_BBP_TO_CPU_32K 202
#define M3_W_ARM_INT_02 204
#define CBBP_INT01 219
#define M3_IPF_INT0 208
#define IPCNORIRQ IRQ_IPC_S_INT3
#define NSIPCNORIRQ IRQ_IPC_NS_INT3
#define ASPIPCIRQ IRQ_ASPIPC
#define VDM_IRQ(cluster) (127 + (cluster))
#define GPU_VDM_IRQ 126
#define FAULT_SVC_ID 11
#define FAULT_PENDSV_ID 14
#define FAULT_SYSTICK_ID 15
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
