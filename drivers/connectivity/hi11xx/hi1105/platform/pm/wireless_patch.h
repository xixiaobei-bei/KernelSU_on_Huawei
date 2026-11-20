

#ifndef __WIRELESS_PATCH_H__
#define __WIRELESS_PATCH_H__

#include "hw_bfg_ps.h"
#include "plat_debug.h"
#include "plat_firmware.h"

#define os_init_waitqueue_head(wq)                               init_waitqueue_head(wq)
#define os_wait_event_interruptible(wq, condition)               wait_event_interruptible(wq, condition)
#define os_wait_event_interruptible_timeout(wq, condition, time) wait_event_interruptible_timeout(wq, condition, time)
#define os_waitqueue_active(wq)                                  waitqueue_active(wq)
#define os_wake_up_interruptible(wq)                             wake_up_interruptible(wq)

#define SOH 0x01 /* 开始字符 */
#define EOT 0x04 /* 发送完成 */
#define ACK 0x06 /* 正确接收应答 */
#define NAK 0x15 /* 校验错误重新发送，通讯开始时用于接收方协商累加校验 */
#define CAN 0x18 /* 结束下载 */

#define PATCH_INTEROP_TIMEOUT HZ

#define CRC_TABLE_SIZE 256

#define DATA_BUF_LEN 128

#define CFG_PACH_LEN 64

#define UART_CFG_FILE_FPGA_HI1105  "/vendor/firmware/hi1105/fpga/uart_cfg"
#define UART_CFG_FILE_PILOT_HI1105 "/vendor/firmware/hi1105/pilot/uart_cfg"

#define UART_CFG_FILE_FPGA_HI1106  "/vendor/firmware/hi1106/fpga/uart_cfg"
#define UART_CFG_FILE_PILOT_HI1106 "/vendor/firmware/hi1106/pilot/uart_cfg"

#define BRT_CMD_KEYWORD "BAUDRATE"

#define CMD_LEN  32
#define PARA_LEN 128

#define PATCH_SEND_CAN_UART                                \
    do {                                                   \
        patch_send_char(CAN, NO_RESPONSE);                 \
    } while (0)
#define PATCH_SEND_EOT_UART                                \
    do {                                                   \
        patch_send_char(EOT, WAIT_RESPONSE);               \
    } while (0)
#define PATCH_SEND_A_UART                                  \
    do {                                                   \
        patch_send_char('a', NO_RESPONSE);                 \
    } while (0)
#define PATCH_SEND_N_UART                                  \
    do {                                                   \
        patch_send_char('n', NO_RESPONSE);                 \
    } while (0)

#define MSG_FORM_DRV_G 'G'
#define MSG_FORM_DRV_C 'C'
#define MSG_FORM_DRV_A 'a'
#define MSG_FORM_DRV_N 'n'

#define READ_PATCH_BUF_LEN        (1024 * 32)
#define READ_DATA_BUF_LEN         (1024 * 50)
#define READ_DATA_REALLOC_BUF_LEN (1024 * 4)

#define XMODE_DATA_LEN 1024

#define XMODEM_PAK_LEN (XMODE_DATA_LEN + 5)

/* Enum Type Definition */
enum PATCH_WAIT_RESPONSE_ENUM {
    NO_RESPONSE = 0, /* 不等待device响应 */
    WAIT_RESPONSE    /* 等待device响应 */
};

typedef wait_queue_head_t os_wait_queue_head;

typedef struct {
    uint8_t cfg_path[CFG_PACH_LEN];
    uint8_t cfg_version[VERSION_LEN];
    uint8_t dev_version[VERSION_LEN];

    uint8_t resv_buf1[RECV_BUF_LEN];
    int32_t resv_buf1_len;

    uint8_t resv_buf2[RECV_BUF_LEN];
    int32_t resv_buf2_len;

    int32_t count;
    struct cmd_type_st *cmd;

    os_wait_queue_head *wait;
} patch_globals;

/* xmodem每包数据的结构，CRC校验 */
typedef struct {
    char head;      /* 开始字符 */
    char packet_num; /* 包序号 */
    char packet_ant; /* 包序号补码 */
} xmodem_head_pkt;

typedef struct {
    uint8_t *pbufstart;
    uint8_t *pbufend;
    uint8_t *phead;
    uint8_t *ptail;
} ring_buf;

/* Global Variable Declaring */
extern ring_buf g_stringbuf;
extern uint8_t *g_data_buf;
extern patch_globals g_patch_ctrl;

/* Function Declare */
extern int bfg_patch_download_function(void);
extern int32_t patch_send_char(char num, int32_t wait);
extern int32_t bfg_patch_recv(const uint8_t *data, int32_t count);
extern int32_t ps_patch_write(uint8_t *data, int32_t count);
extern int32_t ps_recv_patch(void *disc_data, const uint8_t *data, int32_t count);

#endif /* end of oam_kernel.h */
