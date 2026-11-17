

/* 头文件包含 */
#include "wireless_patch.h"
#include "plat_type.h"
#include "plat_uart.h"
#include "bfgx_dev.h"
#include "oal_kernel_file.h"
#include "plat_pm.h"
#include "bfgx_data_parse.h"
#include "plat_cali.h"
#include "securec.h"

/* 全局变量定义 */
patch_globals g_patch_ctrl;
ring_buf g_stringbuf;
uint8_t *g_data_buf;

/* xmodem 索引 */
uint8_t g_xmodem_index = 1;

unsigned short g_crc_table[CRC_TABLE_SIZE] = {
    /* CRC 余式表 */
    0X0000, 0X1021, 0X2042, 0X3063, 0X4084, 0X50A5, 0X60C6, 0X70E7,
    0X8108, 0X9129, 0XA14A, 0XB16B, 0XC18C, 0XD1AD, 0XE1CE, 0XF1EF,
    0X1231, 0X0210, 0X3273, 0X2252, 0X52B5, 0X4294, 0X72F7, 0X62D6,
    0X9339, 0X8318, 0XB37B, 0XA35A, 0XD3BD, 0XC39C, 0XF3FF, 0XE3DE,
    0X2462, 0X3443, 0X0420, 0X1401, 0X64E6, 0X74C7, 0X44A4, 0X5485,
    0XA56A, 0XB54B, 0X8528, 0X9509, 0XE5EE, 0XF5CF, 0XC5AC, 0XD58D,
    0X3653, 0X2672, 0X1611, 0X0630, 0X76D7, 0X66F6, 0X5695, 0X46B4,
    0XB75B, 0XA77A, 0X9719, 0X8738, 0XF7DF, 0XE7FE, 0XD79D, 0XC7BC,
    0X48C4, 0X58E5, 0X6886, 0X78A7, 0X0840, 0X1861, 0X2802, 0X3823,
    0XC9CC, 0XD9ED, 0XE98E, 0XF9AF, 0X8948, 0X9969, 0XA90A, 0XB92B,
    0X5AF5, 0X4AD4, 0X7AB7, 0X6A96, 0X1A71, 0X0A50, 0X3A33, 0X2A12,
    0XDBFD, 0XCBDC, 0XFBBF, 0XEB9E, 0X9B79, 0X8B58, 0XBB3B, 0XAB1A,
    0X6CA6, 0X7C87, 0X4CE4, 0X5CC5, 0X2C22, 0X3C03, 0X0C60, 0X1C41,
    0XEDAE, 0XFD8F, 0XCDEC, 0XDDCD, 0XAD2A, 0XBD0B, 0X8D68, 0X9D49,
    0X7E97, 0X6EB6, 0X5ED5, 0X4EF4, 0X3E13, 0X2E32, 0X1E51, 0X0E70,
    0XFF9F, 0XEFBE, 0XDFDD, 0XCFFC, 0XBF1B, 0XAF3A, 0X9F59, 0X8F78,
    0X9188, 0X81A9, 0XB1CA, 0XA1EB, 0XD10C, 0XC12D, 0XF14E, 0XE16F,
    0X1080, 0X00A1, 0X30C2, 0X20E3, 0X5004, 0X4025, 0X7046, 0X6067,
    0X83B9, 0X9398, 0XA3FB, 0XB3DA, 0XC33D, 0XD31C, 0XE37F, 0XF35E,
    0X02B1, 0X1290, 0X22F3, 0X32D2, 0X4235, 0X5214, 0X6277, 0X7256,
    0XB5EA, 0XA5CB, 0X95A8, 0X8589, 0XF56E, 0XE54F, 0XD52C, 0XC50D,
    0X34E2, 0X24C3, 0X14A0, 0X0481, 0X7466, 0X6447, 0X5424, 0X4405,
    0XA7DB, 0XB7FA, 0X8799, 0X97B8, 0XE75F, 0XF77E, 0XC71D, 0XD73C,
    0X26D3, 0X36F2, 0X0691, 0X16B0, 0X6657, 0X7676, 0X4615, 0X5634,
    0XD94C, 0XC96D, 0XF90E, 0XE92F, 0X99C8, 0X89E9, 0XB98A, 0XA9AB,
    0X5844, 0X4865, 0X7806, 0X6827, 0X18C0, 0X08E1, 0X3882, 0X28A3,
    0XCB7D, 0XDB5C, 0XEB3F, 0XFB1E, 0X8BF9, 0X9BD8, 0XABBB, 0XBB9A,
    0X4A75, 0X5A54, 0X6A37, 0X7A16, 0X0AF1, 0X1AD0, 0X2AB3, 0X3A92,
    0XFD2E, 0XED0F, 0XDD6C, 0XCD4D, 0XBDAA, 0XAD8B, 0X9DE8, 0X8DC9,
    0X7C26, 0X6C07, 0X5C64, 0X4C45, 0X3CA2, 0X2C83, 0X1CE0, 0X0CC1,
    0XEF1F, 0XFF3E, 0XCF5D, 0XDF7C, 0XAF9B, 0XBFBA, 0X8FD9, 0X9FF8,
    0X6E17, 0X7E36, 0X4E55, 0X5E74, 0X2E93, 0X3EB2, 0X0ED1, 0X1EF0
};

/*
 * Prototype    : ps_patch_to_nomal
 * Description  : from download patch state to normal state
 */
static void ps_patch_to_nomal(void)
{
    struct ps_core_s *ps_core_d = NULL;
    ps_print_info("%s", __func__);
    ps_core_d = ps_get_core_reference(BUART);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL");
        return;
    }

    /* change function pointer, pointer to ps_core_recv */
    st_tty_recv = ps_core_recv;
    /* init variable for rx data */
    ps_core_d->rx_pkt_total_len = 0;
    ps_core_d->rx_have_recv_pkt_len = 0;
    ps_core_d->rx_have_del_public_len = 0;
    ps_core_d->rx_decode_public_buf_ptr = ps_core_d->rx_public_buf_org_ptr;
    return;
}

/*
 * Prototype    : ps_patch_write
 * Description  : functions called from pm drivers,used download patch data
 */
int32_t ps_patch_write(uint8_t *data, int32_t count)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t len;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->tty == NULL))) {
        ps_print_err(" tty unavailable to perform write");
        return -EINVAL;
    }
    /* write to uart */
    len = ps_write_tty(ps_core_d, data, count);
    return len;
}

/*
 * Prototype    : ps_recv_patch
 * Description  : PS's pm receive function.this function is called when download patch.
 * input        : data -> recive data ptr   from UART TTY
 *                count -> recive data count from UART TTY
 */
int32_t ps_recv_patch(void *disc_data, const uint8_t *data, int32_t count)
{
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = (struct ps_core_s *)disc_data;
    if (unlikely((data == NULL) || (disc_data == NULL) || (ps_core_d->pm_data == NULL) ||
                 (ps_core_d->pm_data->recv_patch == NULL))) {
        ps_print_err(" received null from TTY ");
        return -EINVAL;
    }

    /* tx data to hw-pm */
    if (ps_core_d->pm_data->recv_patch(data, count) < 0) {
        ps_print_err(" %s-err", __func__);
        return -EPERM;
    }
    return 0;
}

/*
 * Prototype    : pm_uart_set_baudrate
 * Description  : set baudrate of uart0 when download patch
 * Input        : long baudrate: the baudrate want to set
 * Return       : 0 means succeed,-1 means failed.
 */
int32_t pm_uart_set_baudrate(long baudrate)
{
    struct ps_core_s *ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL || ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -1;
    }

    ps_print_info("set uart baudrate to %ld\n", baudrate);

    /* call interface supplied by 3 in 1 */
    if (ps_core_d->pm_data->change_baud_rate != NULL) {
        /* set uart baudrate */
        return ps_core_d->pm_data->change_baud_rate(ps_core_d, baudrate, FLOW_CTRL_ENABLE);
    }

    ps_print_err("change_baud_rate is NULL!\n");

    return -1;
}

/*
 * Prototype    : do_crc_table_1
 * Description  : CRC校验
 */
unsigned short do_crc_table_1(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0;
    uint8_t data_tmp;
    while (length > 0) {
        data_tmp = (uint8_t)(crc >> 8); /* crc右移8位 */
        crc = crc << 8; /* crc左移8位 */
        crc = crc ^ g_crc_table[data_tmp ^ (*data)];
        length--;
        data++;
    }
    return crc;
}

static void ringbuf_flush(void)
{
    /* writing maybe still work when interrupt by flush  */
    g_stringbuf.ptail = g_stringbuf.phead;
}

static int32_t check_uart_bufflen(void)
{
    if ((g_stringbuf.pbufstart == NULL) || (g_stringbuf.pbufend < g_stringbuf.pbufstart)) {
        ps_print_err("buf is NULL, failed\n ");
        ps_print_err("pbufstart=%p,pbufend=%p\n", g_stringbuf.pbufstart, g_stringbuf.pbufend);
        return -EFAIL;
    }

    if ((g_stringbuf.phead < g_stringbuf.pbufstart) ||
        (g_stringbuf.phead > g_stringbuf.pbufend) ||
        (g_stringbuf.ptail < g_stringbuf.pbufstart) ||
        (g_stringbuf.ptail > g_stringbuf.pbufend)) {
        ps_print_err("phead or ptail is out of range, failed\n");
        ps_print_err("pbufstart=%p,pbufend=%p\n", g_stringbuf.pbufstart, g_stringbuf.pbufend);
        ps_print_err("phead=%p,ptail=%p\n", g_stringbuf.phead, g_stringbuf.ptail);
        return -EFAIL;
    }

    return SUCC;
}

/*
 * Prototype    : uart_recv_data
 * Description  : receiver data form device,by uart interface
 */
int32_t uart_recv_data(const uint8_t *data, int32_t len)
{
    uint32_t ulbuflen;
    uint32_t ulheadtoendlen;
    uint8_t *ptail = NULL;
    if (unlikely((data == NULL))) {
        ps_print_err("data is NULL\n ");
        return -EFAIL;
    }

    if (check_uart_bufflen() != SUCC) {
        return -EFAIL;
    }
    ulbuflen = g_stringbuf.pbufend - g_stringbuf.pbufstart;
    ptail = g_stringbuf.ptail;
    ps_print_dbg("len [%d],\n ", len);

    /* phead and ptail are in the same cycle */
    if (g_stringbuf.phead >= ptail) {
        /* still in the same cycle */
        if ((g_stringbuf.pbufend - g_stringbuf.phead) >= len) {
            memcpy_s(g_stringbuf.phead, g_stringbuf.pbufend - g_stringbuf.phead, data, len);
            g_stringbuf.phead += len;
            if (os_waitqueue_active(g_patch_ctrl.wait)) {
                os_wake_up_interruptible(g_patch_ctrl.wait);
                ps_print_dbg("wake up ok");
            }
        } else if ((ulbuflen - (g_stringbuf.phead - ptail)) > len) {
            ulheadtoendlen = g_stringbuf.pbufend - g_stringbuf.phead;
            memcpy_s(g_stringbuf.phead, ulheadtoendlen, data, ulheadtoendlen);
            memcpy_s(g_stringbuf.pbufstart, g_stringbuf.ptail - g_stringbuf.pbufstart,
                     data + ulheadtoendlen, len - ulheadtoendlen);
            g_stringbuf.phead = g_stringbuf.pbufstart + (len - ulheadtoendlen);
            ps_print_info("phead back\n");
            if (os_waitqueue_active(g_patch_ctrl.wait)) {
                os_wake_up_interruptible(g_patch_ctrl.wait);
                ps_print_dbg("wake up ok");
            }
        } else {
            ps_print_err("Not enough mem,len=%d.\n ", len);
        }
    } else if((ptail - g_stringbuf.phead - 1) > len) {
        /* phead is in the next cycle */
        /* "ptail - phead = 1" means the buffer is full */
        memcpy_s(g_stringbuf.phead, ptail - g_stringbuf.phead - 1, data, len);
        g_stringbuf.phead += len;
        if (os_waitqueue_active(g_patch_ctrl.wait)) {
            os_wake_up_interruptible(g_patch_ctrl.wait);
            ps_print_dbg("wake up ok");
        }
    } else {
        ps_print_err("Not enough mem,len=%d.\n ", len);
    }

    if (g_stringbuf.phead >= g_stringbuf.pbufend) {
        g_stringbuf.phead = g_stringbuf.pbufstart;
        ps_print_info("phead back\n");
    }
    return SUCC;
}

/*
 * Prototype    : bfg_patch_recv
 * Description  : function for bfg patch receive
 * Input        : uint8_t *data: address of data
 *                int32_t count: length of data
 */
int32_t bfg_patch_recv(const uint8_t *data, int32_t count)
{
    int32_t ret;

    /* this function should be called after patch_init(uart), otherwise maybe null pointer */
    ret = uart_recv_data(data, count);

    return ret;
}

int32_t read_msg_t(uint8_t *data, int32_t len)
{
    int32_t l_len;
    uint32_t ultailtoendlen;
    uint8_t *phead = NULL;

    if (unlikely((data == NULL))) {
        ps_print_err("data is NULL\n ");
        return -EFAIL;
    }

    if (check_uart_bufflen() != SUCC) {
        return -EFAIL;
    }

    os_wait_event_interruptible_timeout(*g_patch_ctrl.wait,
                                        (g_stringbuf.phead != g_stringbuf.ptail),
                                        PATCH_INTEROP_TIMEOUT);
    phead = g_stringbuf.phead;
    /* phead and ptail are in the same cycle */
    if (phead > g_stringbuf.ptail) {
        if ((phead - g_stringbuf.ptail) > len) {
            memcpy_s(data, len, g_stringbuf.ptail, len);
            l_len = len;
            g_stringbuf.ptail += len;
        } else {
            /* not enough data */
            l_len = phead - g_stringbuf.ptail;
            memcpy_s(data, len, g_stringbuf.ptail, l_len);
            g_stringbuf.ptail += l_len;
        }
    } else if (phead < g_stringbuf.ptail) {
        /* phead is in the next cycle */
        ultailtoendlen = g_stringbuf.pbufend - g_stringbuf.ptail;
        if (ultailtoendlen > len) {
            memcpy_s(data, len, g_stringbuf.ptail, len);
            l_len = len;
            g_stringbuf.ptail += len;
        } else {
            memcpy_s(data, len, g_stringbuf.ptail, ultailtoendlen);
            if ((phead - g_stringbuf.pbufstart) > (len - ultailtoendlen)) {
                memcpy_s(data + ultailtoendlen, len - ultailtoendlen,
                         g_stringbuf.pbufstart, len - ultailtoendlen);
                g_stringbuf.ptail = g_stringbuf.pbufstart + (len - ultailtoendlen);
                l_len = len;
                ps_print_info("ptail back\n");
            } else {
                memcpy_s(data + ultailtoendlen, len - ultailtoendlen,
                         g_stringbuf.pbufstart, phead - g_stringbuf.pbufstart);
                l_len = ultailtoendlen + (phead - g_stringbuf.pbufstart);
                g_stringbuf.ptail = phead;
            }
        }
    } else {
        l_len = -1;
        ps_print_warning("No data.\n");
    }
    if (g_stringbuf.ptail >= g_stringbuf.pbufend) {
        g_stringbuf.ptail = g_stringbuf.pbufstart;
    }

    return l_len;
}

/*
 * Prototype    : pm_uart_send
 * Description  : uart patch transmit function
 * Input        : uint8_t *date: address of data
 *                int32_t len: length of data
 * Return       : length which has been sent
 */
int32_t pm_uart_send(uint8_t *data, int32_t len)
{
    uint16_t count = 0;

    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -1;
    }

    if (pm_data->write_patch == NULL) {
        ps_print_err("bfg_write_patch is NULL!\n");
        return -1;
    }

    while (1) {
        /* this function return the length has been sent */
        count = pm_data->write_patch(data, len);
        /* data has been sent over and return */
        if (count == len) {
            return len;
        }

        /* data has not been sent over, we will send again */
        data = data + count;
        len = len - count;
        msleep(1);
    }
}

/*
 * Prototype    : send_msg
 * Description  : send message to device,by sdio or uart
 */
int32_t send_msg_t(uint8_t *data, int32_t len)
{
    return pm_uart_send(data, len);
}

/*
 * Prototype    : recv_expect_result
 * Description  : receive result form device
 */
int32_t recv_expect_result_t(uint8_t expect)
{
    uint8_t auc_buf[RECV_BUF_LEN];
    int32_t l_len;
    int32_t i;

    ps_print_dbg(" entry\n");

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_len = read_msg_t(auc_buf, 1);
        if (l_len < 0) {
            ps_print_err("recv result fail\n");
            return -EFAIL;
        }
        if (auc_buf[0] == expect) {
            ps_print_dbg(" send SUCC [%x]\n", expect);
            return SUCC;
        } else if ((auc_buf[0] == MSG_FORM_DRV_N) || (auc_buf[0] == NAK) || (auc_buf[0] == MSG_FORM_DRV_C)) {
            /*
             * NAK: 文件传输时重发标识
             * MSG_FORM_DRV_N:其他重发标识
             */
            ps_print_err(" send again [0x%x]\n", auc_buf[0]);
            return -EFAIL;
        } else {
            /* 对于错误的结果，有十次的机会， */
            PATCH_SEND_N_UART;
            ps_print_warning(" error result[0x%x], expect [0x%x], read result again\n", auc_buf[0], expect);
        }
    }
    return -EFAIL;
}

/*
 * Prototype    : patch_device_respond
 * Description  : wait respond form device
 */
int32_t patch_device_respond(void)
{
    int32_t l_ret;
    int32_t i;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        memset_s(g_patch_ctrl.dev_version, VERSION_LEN, 0, VERSION_LEN);
        msleep(1);
        l_ret = read_msg_t(g_patch_ctrl.dev_version, VERSION_LEN);
        if (l_ret < 0) {
            ps_print_err("read fail![%d]\n", i);
            continue;
        } else if (!os_mem_cmp((char *)g_patch_ctrl.dev_version,
                               (char *)g_patch_ctrl.cfg_version,
                               os_str_len(g_patch_ctrl.cfg_version))) {
            ps_print_info("Device Version = [%s], CfgVersion = [%s].\n",
                          g_patch_ctrl.dev_version, g_patch_ctrl.cfg_version);
            return SUCC;
        } else {
            ps_print_info("Device Version = [%s], CfgVersion = [%s].\n",
                          g_patch_ctrl.dev_version, g_patch_ctrl.cfg_version);
        }
    }

    ps_print_err("read device version fail![%d]\n", i);

    return -EFAIL;
}

/*
 * Prototype    : patch_send
 * Description  : send message to device,by sdio or uart
 */
int32_t patch_send(uint8_t *data, int32_t len, uint8_t expect)
{
    int32_t i;
    int32_t l_ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = send_msg_t(data, len);
        if (l_ret < 0) {
            continue;
        }

        l_ret = recv_expect_result_t(expect);
        if (l_ret == 0) {
            return SUCC;
        }
    }

    return -EFAIL;
}

/*
 * Prototype    : patch_xmodem_send
 * Description  : xmodem protocol encapsulation for down file
 */
int32_t patch_xmodem_send(uint8_t *data, int32_t len, uint8_t expect)
{
    xmodem_head_pkt st_patch_pkt;
    uint16_t crc_value;
    int32_t l_ret;
    int32_t l_sendlen;
    int32_t l_datalen;
    int32_t i;
    uint8_t *flag = NULL;
    uint8_t crc_value_h;
    uint8_t crc_value_l;

    if (data == NULL) {
        return -EFAIL;
    }

    l_datalen = len;
    flag = data;

    while (l_datalen > 0) {
        l_sendlen = oal_min(XMODE_DATA_LEN, l_datalen);
        l_datalen = l_datalen - l_sendlen;
        st_patch_pkt.head = SOH;
        st_patch_pkt.packet_num = g_xmodem_index;

        /* 数据长度不够128个 */
        if (l_sendlen < XMODE_DATA_LEN) {
            ps_print_dbg("data_len  %d\n", l_sendlen);
            memset_s(&flag[l_sendlen], (XMODE_DATA_LEN - l_sendlen), 0x00, (XMODE_DATA_LEN - l_sendlen));
        }
        crc_value = do_crc_table_1(flag, XMODE_DATA_LEN);
        crc_value_h = (crc_value & 0xFF00) >> 8; /* us_CRCValue与上0xFF00后右移8位 */
        crc_value_l = crc_value & 0xFF;

        st_patch_pkt.packet_ant = ~(st_patch_pkt.packet_num);

        for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
            l_ret = pm_uart_send((uint8_t *)&st_patch_pkt, sizeof(st_patch_pkt));
            l_ret = pm_uart_send(flag, XMODE_DATA_LEN);
            l_ret = pm_uart_send(&crc_value_h, 1);
            l_ret = pm_uart_send(&crc_value_l, 1);

            l_ret = recv_expect_result_t(expect);
            if (l_ret < 0) {
                continue;
            }

            if (l_ret == SUCC) {
                break;
            }
        }
        if (i >= HOST_DEV_TIMEOUT) {
            return -EFAIL;
        }
        flag = flag + l_sendlen;

        g_xmodem_index++;
    }

    return SUCC;
}

int32_t patch_string_to_num(uint8_t *string)
{
    int32_t i;
    int32_t l_num;

    if (string == NULL) {
        return -EFAIL;
    }

    l_num = 0;
    for (i = 0; (string[i] >= '0') && (string[i] <= '9'); i++) {
        l_num = (l_num * 10) + (string[i] - '0'); /* 字符串转数字的逻辑需要，l_num*10 */
    }

    return l_num;
}

/*
 * Prototype    : patch_wait_g_form_dev
 * Description  : wait go'command form device
 */
int32_t patch_wait_g_form_dev(void)
{
    int32_t l_ret;
    int32_t i;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = recv_expect_result_t(MSG_FORM_DRV_G);
        if (l_ret == 0) {
            ps_print_dbg(" device finish G\n");
            return SUCC;
        }
    }

    ps_print_err("recv g FAIL\n");
    return -EFAIL;
}

int32_t patch_wait_g_retry_form_dev(void)
{
    int32_t l_ret;
    int32_t i;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = recv_expect_result_t(MSG_FORM_DRV_G);
        if (l_ret == 0) {
            ps_print_dbg(" device finish G\n");
            return SUCC;
        } else {
            PATCH_SEND_N_UART;
            ps_print_warning("receive G failed\n");
        }
    }
    return -EFAIL;
}

/*
 * Prototype    : patch_send_char
 * Description  : send char to device
 */
int32_t patch_send_char(char num, int32_t wait)
{
    int32_t l_ret;
    const uint32_t ul_buf_size = 8;
    uint8_t auc_buf[ul_buf_size];
    int32_t i;

    memset_s(auc_buf, sizeof(auc_buf), num, sizeof(auc_buf));
    ps_print_dbg("send [0x%x], wait[%d]\n", num, wait);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        /*
         * sdio 接口发送时，会四字节对齐，发送四个
         * uart 接口发送时，只发送一个
         */
        l_ret = send_msg_t(auc_buf, 1);
        if (l_ret < 0) {
            ps_print_err("Send fail\n");
            return l_ret;
        }

        if (wait == WAIT_RESPONSE) {
            l_ret = recv_expect_result_t(ACK);
            if (l_ret < 0) {
                continue;
            }
        }

        return l_ret;
    }

    return -EFAIL;
}

/*
 * Prototype    : patch_read_patch
 * Description  : read patch
 */
int32_t patch_read_patch(char *buf, int32_t len, os_kernel_file *fp)
{
    int32_t rdlen;

    if ((IS_ERR(fp)) || (buf == NULL)) {
        fp = NULL;
        ps_print_err("buf/fp is NULL\n");
        return -EFAIL;
    }

    rdlen = oal_file_read_ext(fp, fp->f_pos, buf, len);
    if (rdlen > 0) {
        fp->f_pos += rdlen;
    }

    return rdlen;
}

/*
 * Prototype    : patch_down_file
 * Description  : begin download patch file
 */
int32_t patch_down_file(const uint8_t *puc_file)
{
    os_kernel_file *fp = NULL;
    uint8_t *auc_buf;
    int32_t l_len;
    int32_t l_ret;
    int32_t l_count;
    mm_segment_t fs;

    if (puc_file == NULL) {
        return -EFAIL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(puc_file, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        set_fs(fs);
        fp = NULL;
        ps_print_err("filp_open %s fail!!\n", puc_file);
        return -EFAIL;
    }

    if (g_data_buf == NULL) {
        filp_close(fp, NULL);
        set_fs(fs);
        fp = NULL;
        return -EFAIL;
    }

    l_count = 1;
    g_xmodem_index = 1;
    while (1) {
        l_len = patch_read_patch(g_data_buf, READ_PATCH_BUF_LEN, fp);
        ps_print_dbg("kernel_read len[%d] [%d]\n", l_len, l_count);
        /* 正常读取文件 */
        if ((l_len > 0) && (l_len <= READ_PATCH_BUF_LEN)) {
            l_ret = patch_xmodem_send(g_data_buf, l_len, ACK);
            if (l_ret == -EFAIL) {
                ps_print_err(" uart send data[%d] fail\n", l_count);
                break;
            }
        } else if (l_len == 0) {
            /* 文件已经读取完成 */
            PATCH_SEND_EOT_UART;
            g_xmodem_index = 1;
            l_ret = SUCC;

            ps_print_dbg("read file[%d] [%d] send EOT\n", l_count, l_len);
            break;
        } else {
            /* 读取文件出错 */
            PATCH_SEND_CAN_UART;
            g_xmodem_index = 1;
            l_ret = -EFAIL;
            ps_print_err("read file[%d] [%d]\n", l_count, l_len);
            break;
        }
        l_count++;
    }

    auc_buf = NULL;

    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;

    return l_ret;
}

/*
 * Prototype    : patch_readm_fileopen
 * Description  : creat and open file to save mem
 */
os_kernel_file *patch_readm_fileopen(void)
{
    os_kernel_file *fp = NULL;
    mm_segment_t fs;
    struct timeval tv;
    struct rtc_time tm;
    char filename[50]; /* readm_bfg 文件名长度最大50 */
    int ret;

    oal_do_gettimeofday(&tv);
    rtc_time_to_tm(tv.tv_sec, &tm);

    /* 转换成当前时间 */
    ps_print_info("%4d-%02d-%02d  %02d:%02d:%02d\n",
                  tm.tm_year + 1900, /* 1900为系统计时的起点 */
                  tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    ret = snprintf_s(filename, sizeof(filename), sizeof(filename) - 1,
                     "/data/log/hi110x/readm_bfg%04d%02d%02d%02d%02d%02d.bin",
                     tm.tm_year + 1900, /* 1900为系统计时的起点 */
                     tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (ret < 0) {
        ps_print_err("log str format err line[%d]\n", __LINE__);
        return NULL;
    }

    ps_print_info("filename = %s", filename);
    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(filename, O_RDWR | O_CREAT, 0644);
    set_fs(fs);

    return fp;
}

/*
 * Prototype    : patch_recv_mem
 * Description  : receive memory information form device
 */
int32_t patch_recv_mem(os_kernel_file *fp, int32_t len)
{
    uint8_t *pdatabuf = NULL;
    int32_t l_ret;
    mm_segment_t fs;
    int32_t lenbuf = 0;

    if (oal_is_err_or_null(fp)) {
        ps_print_err("fp is error,fp = 0x%p\n", fp);
        return -EFAIL;
    }

    if (len == 0) { /* 0 is invalid */
        ps_print_warning("len=%d is invalid, force to 4 bytes\n", len);
        len = 4; /* 4 bytes at least */
    }

    pdatabuf = os_kmalloc_gfp(len);
    if (pdatabuf == NULL) {
        return -EFAIL;
    }

    memset_s(pdatabuf, len, 0, len);

    fs = get_fs();
    set_fs(KERNEL_DS);

    while (len > lenbuf) {
        l_ret = read_msg_t(pdatabuf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            ps_print_err("time out\n");
            break;
        }
    }

    if (len <= lenbuf) {
        oal_kernel_file_write(fp, pdatabuf, len);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif

    set_fs(fs);
    ringbuf_flush();
    os_mem_kfree(pdatabuf);

    return l_ret;
}

/*
 * Prototype    : patch_int_para_send
 * Description  : down integer to device
 */
int32_t patch_int_para_send(uint8_t *name, uint8_t *value)
{
    int32_t l_ret;
    int32_t data_len;
    int32_t value_len;
    int32_t i;
    int32_t n;
    uint8_t num[INT32_STR_LEN];
    uint8_t data[DATA_BUF_LEN];

    value_len = os_str_len((char *)value);

    memset_s(num, INT32_STR_LEN, 0, INT32_STR_LEN);
    memset_s(data, DATA_BUF_LEN, 0, DATA_BUF_LEN);

    data_len = 0;
    data_len = os_str_len(name);
    l_ret = memcpy_s(data, sizeof(data), name, data_len);
    if (l_ret != EOK) {
        ps_print_err("data buff not enough\n");
        return -EFAIL;
    }

    data[data_len] = COMPART_KEYWORD;
    data_len = data_len + 1;

    for (i = 0, n = 0; (i <= value_len) && (n < INT32_STR_LEN); i++) {
        if ((value[i] == ',') || (value_len == i)) {
            ps_print_dbg("num = %s, i = %d, n = %d\n", num, i, n);
            if (n == 0) {
                continue;
            }
            l_ret = memcpy_s((uint8_t *)&data[data_len], sizeof(data) - data_len, num, n);
            if (l_ret != EOK) {
                ps_print_err("data buff not enough\n");
                return -EFAIL;
            }
            data_len = data_len + n;

            data[data_len] = COMPART_KEYWORD;
            data_len = data_len + 1;

            memset_s(num, INT32_STR_LEN, 0, INT32_STR_LEN);
            n = 0;
        } else if (value[i] == 0x20) {
            continue;
        } else {
            num[n] = value[i];
            n++;
        }
    }

    ps_print_dbg("data_len = %d, \n", data_len);
    ps_print_dbg("data = %s, \n", data);

    ringbuf_flush();

    l_ret = send_msg_t(data, data_len);

    return l_ret;
}

int32_t patch_number_version_cmd(uint8_t *key, uint8_t *value)
{
    int32_t l_ret;
    int32_t i;
    char version_buff[VERSION_LEN] = {0};

    l_ret = snprintf_s(version_buff, sizeof(version_buff), sizeof(version_buff) - 1, "%s%c",
                       VER_CMD_KEYWORD, COMPART_KEYWORD);
    if (l_ret < 0) {
        ps_print_err("log str format err line[%d]\n", __LINE__);
        return l_ret;
    }

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = send_msg_t(version_buff, VERSION_LEN);
        if (l_ret < 0) {
            continue;
        }

        l_ret = patch_device_respond();
        if (l_ret == 0) {
            return SUCC;
        }
    }

    return l_ret;
}

STATIC int32_t patch_number_wmem_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;

    ret = patch_int_para_send(key, value);
    if (ret < 0) {
        ps_print_err("send %s,%s fail \n", key, value);
        return ret;
    }

    /* G 是device 相应操作完成标志 */
    ret = patch_wait_g_form_dev();
    if (ret >= 0) {
        ps_print_info("patch write mem %s success!\n", value);
    } else {
        ps_print_err("patch write mem %s fail! ret=%d\n", value, ret);
    }

    return ret;
}

STATIC int32_t patch_number_jump_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;

    ret = patch_int_para_send(key, value);
    if (ret < 0) {
        ps_print_err("send %s,%s fail \n", key, value);
        return ret;
    }

    /* G 是device 相应操作完成标志 */
    ret = patch_wait_g_form_dev();
    if (ret >= 0) {
        ps_print_info("JUMP %s success!\n", value);
    } else {
        ps_print_err("JUMP %s timeout! ret=%d\n", value, ret);
    }

    return ret;
}

STATIC int32_t patch_number_rmem_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;
    int32_t num;
    uint8_t *flag = NULL;
    os_kernel_file *fp = NULL;

    fp = patch_readm_fileopen();
    if (IS_ERR(fp)) {
        return SUCC;
    }

    ret = patch_int_para_send(key, value);
    if (ret < 0) {
        ps_print_err("send %s,%s fail \n", key, value);
        oal_file_close(fp);
        return ret;
    }

    flag = os_str_chr(value, ',');
    if (flag == NULL) {
        ps_print_err("RECV LEN ERROR..\n");
        oal_file_close(fp);
        return -EFAIL;
    }
    flag++;
    while (*flag == COMPART_KEYWORD) {
        flag++;
    }
    num = patch_string_to_num(flag);

    ps_print_info("recv len [%d]\n", num);

    ret = patch_recv_mem(fp, num);
    oal_file_close(fp);

    return ret;
}

STATIC int32_t patch_number_baud_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;
    int32_t num;

    /* 修改波特率 */
    ret = patch_int_para_send(key, value);
    if (ret < 0) {
        ps_print_err("send %s,%s fail \n", key, value);
        return ret;
    }

    ret = recv_expect_result_t(MSG_FORM_DRV_A);
    if (ret < 0) {
        ps_print_err("send change baudrate fail\n");
        return ret;
    }

    num = patch_string_to_num(value);
    ps_print_info("change baudrate to:%d\n", num);

    /* 增加调用修改波特率函数 */
    ret = pm_uart_set_baudrate(num);
    if (ret < 0) {
        ps_print_err("modify baudrate fail!!\n");
        return -EFAIL;
    }

    ringbuf_flush();
    oal_usleep_range(10000, 11000); /* 睡眠时间在10000到11000us之间 */

    /* G 是device 相应操作完成标志 */
    ret = patch_wait_g_retry_form_dev();

    return ret;
}

STATIC int32_t patch_down_cali_data(void)
{
    int32_t l_len;
    int32_t l_ret;

    if (g_data_buf == NULL) {
        return -EFAIL;
    }

    /* 获取bfgx校准数据 */
    l_ret = get_bfgx_cali_data(g_data_buf, &l_len, READ_PATCH_BUF_LEN);
    if (l_ret < 0) {
        ps_print_err("get bfgx cali data failed, len=%d\n", l_len);
        return -EFAIL;
    }

    /* Wait at least 5 ms */
    oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

    l_ret = patch_xmodem_send(g_data_buf, l_len, ACK);
    if (l_ret == -EFAIL) {
        ps_print_err("uart send data fail\n");
        return l_ret;
    }

    /* 文件已经读取完成 */
    PATCH_SEND_EOT_UART;

    /* G 是 DEVICE 完成相应操作标志 */
    l_ret = patch_wait_g_form_dev();

    return l_ret;
}

STATIC int32_t bfgx_cali_data_download(uint8_t *key, uint8_t *value)
{
    int32_t l_ret;
    uint32_t len;
    uint32_t value_len;
    uint8_t *addr = NULL;
    uint8_t buff_tx[SEND_BUF_LEN];

    /* 重新组合Value字符串，入参Value只是一个地址，形式为"0xXXXXX" */
    /* 组合以后的形式为"FILES 文件个数 要写的地址"---"FILES 1 0xXXXX " */
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    /* buff_tx="" */
    len = os_str_len(key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), key, len);
    if (l_ret != EOK) {
        ps_print_err("buff_tx not enough\n");
        return -EFAIL;
    }
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    /* buff_tx="FILES " */
    value_len = os_str_len(value);
    addr = delete_space(value, &value_len);
    if (addr == NULL) {
        ps_print_err("addr is NULL, Value[%s] value_len[%d]", value, value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, value_len);
    if (l_ret != EOK) {
        ps_print_err("buff_tx not enough\n");
        return -EFAIL;
    }
    len += value_len;
    buff_tx[len] = COMPART_KEYWORD;
    len++;

    ps_print_info("download bfgx cali data addr:%s\n", addr);

    /* buff_tx="FILES 0xXXXX " */
    /* 发送地址 */
    l_ret = patch_send(buff_tx, len, MSG_FORM_DRV_C);
    if (l_ret < 0) {
        ps_print_err(" SEND file addr error\n");

        return -EFAIL;
    }

    /* 获取bfgx校准数据 */
    l_ret = patch_down_cali_data();
    if (l_ret < 0) {
        ps_print_err("send cali data error\n");

        return l_ret;
    }

    return SUCC;
}

STATIC int32_t patch_number_cali_bfgx_data(uint8_t *key, uint8_t *value)
{
    int32_t ret;

    /* 加载BFGX的校准数据 */
    ret = bfgx_cali_data_download(FILES_CMD_KEYWORD, value);
    if (ret < 0) {
        ps_print_err("download bfgx cali data fail\n");
    }

    return ret;
}

/* 根据命令解析 */
STATIC firmware_number g_number_cmd_type[] = {
    {VER_CMD_KEYWORD,   patch_number_version_cmd},
    {WMEM_CMD_KEYWORD,  patch_number_wmem_cmd},
    {BRT_CMD_KEYWORD,   patch_number_baud_cmd},
    {CALI_BFGX_DATA_CMD_KEYWORD, patch_number_cali_bfgx_data},
    {JUMP_CMD_KEYWORD,  patch_number_jump_cmd},
    {RMEM_CMD_KEYWORD,  patch_number_rmem_cmd}
};

int32_t patch_number_type(uint8_t *key, uint8_t *value)
{
    int32_t ret = -EFAIL;
    int32_t loop;

    for (loop = 0; loop < oal_array_size(g_number_cmd_type); loop++) {
        if (!os_str_cmp(key, g_number_cmd_type[loop].key)) {
            ret = (g_number_cmd_type[loop].cmd_exec)(key, value);
            break;
        }
    }

    return ret;
}

/*
 * Prototype    : patch_quit_type
 * Description  : down quit command
 */
int32_t patch_quit_type(void)
{
    int32_t l_ret;
    int32_t l_len;
    const uint32_t ul_buf_len = 8;
    uint8_t buf[ul_buf_len];
    ps_print_dbg("entry\n");

    memset_s(buf, sizeof(buf), 0, sizeof(buf));

    l_ret = memcpy_s(buf, sizeof(buf), (uint8_t *)QUIT_CMD_KEYWORD, os_str_len(QUIT_CMD_KEYWORD));
    if (l_ret != EOK) {
        ps_print_err("buf not enough\n");
        return -EFAIL;
    }
    l_len = os_str_len(QUIT_CMD_KEYWORD);

    buf[l_len] = COMPART_KEYWORD;
    l_len++;

    ps_patch_to_nomal();

    l_ret = send_msg_t(buf, l_len);

    return l_ret;
}

int32_t patch_file_addr_send(uint8_t *data, int32_t data_len, const char *file_path)
{
    int32_t l_ret;

    l_ret = patch_send(data, data_len, MSG_FORM_DRV_C);
    if (l_ret < 0) {
        ps_print_err(" SEND file addr error\n");

        return -EFAIL;
    }

    ps_print_dbg("file path is %s\n", file_path);

    l_ret = patch_down_file(file_path);
    ps_print_dbg("patch_down_file:%d", l_ret);
    if (l_ret < 0) {
        ps_print_err(" SEND file error\n");

        return l_ret;
    }
    /* G 是 DEVICE 完成相应操作标志 */
    l_ret = patch_wait_g_form_dev();

    return l_ret;
}

/*
 * Prototype    : patch_file_type
 * Description  : down addr and file
 */
int32_t patch_file_type(uint8_t *key, const char *value)
{
    int32_t i;
    int32_t n;
    int32_t l_ret;
    int32_t l_len;
    uint8_t addr[INT32_STR_LEN];
    uint8_t data[DATA_BUF_LEN];
    int32_t data_len;
    const char *tmp = NULL;
    const char *tmp1 = NULL;

    ps_print_dbg("key = %s, value = %s\n", key, value);

    /*
     * 根据关键字的最后一个字符，确定发送地址之后，device的返回值
     * 所以配置文件的关键字不能随意修改
     */
    memset_s(data, DATA_BUF_LEN, 0, DATA_BUF_LEN);
    data_len = os_str_len(key);
    l_ret = memcpy_s(data, sizeof(data), key, data_len);
    if (l_ret != EOK) {
        ps_print_err("data not enough\n");
        return -EFAIL;
    }

    data[data_len] = COMPART_KEYWORD;
    data_len++;

    /* 兼容wifibootloader配置设置 */
    tmp1 = value;
    while (*tmp1 == ' ') {
        tmp1++;
    }
    if (*tmp1 != '1') {
        ps_print_err("para [%s] not begin with 1", tmp1);
        return -EFAIL;
    }
    tmp = os_str_chr(tmp1, ',');
    if (tmp == NULL) {
        ps_print_err("has no ',' string:[%s]", tmp);
        return -EFAIL;
    }
    tmp++;
    ps_print_dbg("tmp is %s\n", tmp);

    memset_s(addr, INT32_STR_LEN, 0, INT32_STR_LEN);
    for (i = 0, n = 0; tmp[i] != ',' && n < INT32_STR_LEN; i++) {
        if ((tmp[i] == ',') || (tmp[i] == COMPART_KEYWORD)) {
            break;
        } else {
            addr[n] = tmp[i];
            n++;
        }
    }
    l_ret = memcpy_s((uint8_t *)&data[data_len], sizeof(data) - data_len, addr, n);
    if (l_ret != EOK) {
        ps_print_err("data buff not enough\n");
        return -EFAIL;
    }
    data_len = data_len + n;

    data[data_len] = COMPART_KEYWORD;
    data_len++;

    ps_print_dbg("data is %s\n", data);

    /* 删除头部的空格 */
    l_len = os_str_len((char *)tmp);
    for (i = i + 1; i < l_len; i++) {
        /* 兼容绝对路径和相对路径 */
        if ((tmp[i] == '/') || (tmp[i] == '.')) {
            break;
        }
    }

    l_ret = patch_file_addr_send(data, data_len, &tmp[i]);
    return l_ret;
}

void *patch_malloc_cmd_buf(uint8_t *buf)
{
    int32_t l_len;
    uint8_t *flag = NULL;
    uint8_t *p_buf = NULL;

    /* 统计命令个数 */
    flag = buf;
    g_patch_ctrl.count = 0;
    while (flag != NULL) {
        /* 一个正确的命令行结束符为 ; */
        flag = os_str_chr(flag, CMD_LINE_SIGN);
        if (flag == NULL) {
            break;
        }
        g_patch_ctrl.count++;
        flag++;
    }

    ps_print_info("l_count = %d\n", g_patch_ctrl.count);

    /* 申请存储命令空间 */
    l_len = (g_patch_ctrl.count + CFG_INFO_RESERVE_LEN) * sizeof(struct cmd_type_st);
    p_buf = os_kmalloc_gfp(l_len);
    if (p_buf == NULL) {
        ps_print_err("kmalloc cmd_type_st fail\n");
        return NULL;
    }
    memset_s((void *)p_buf, l_len, 0, l_len);

    return p_buf;
}

static int32_t parse_cfg_add_cmd(int32_t index, int32_t cmd_type, const uint8_t* cmd_name, const uint8_t* cmd_para)
{
    uint32_t cmd_para_len;
    int32_t ret;

    g_patch_ctrl.cmd[index].cmd_type = cmd_type;
    memcpy_s(g_patch_ctrl.cmd[index].cmd_name, DOWNLOAD_CMD_LEN, cmd_name, DOWNLOAD_CMD_LEN);
    memcpy_s(g_patch_ctrl.cmd[index].cmd_para, DOWNLOAD_CMD_PARA_LEN, cmd_para, DOWNLOAD_CMD_PARA_LEN);
    /* 获取配置版本号 */
    if (!os_mem_cmp(g_patch_ctrl.cmd[index].cmd_name, VER_CMD_KEYWORD, os_str_len(VER_CMD_KEYWORD))) {
        cmd_para_len = os_str_len(g_patch_ctrl.cmd[index].cmd_para);
        ret = memcpy_s(g_patch_ctrl.cfg_version, sizeof(g_patch_ctrl.cfg_version),
                       g_patch_ctrl.cmd[index].cmd_para, cmd_para_len);
        if (ret != EOK) {
            ps_print_err("cmd_para_len = %d over auc_CfgVersion length\n", cmd_para_len);
            return -EFAIL;
        }

        ps_print_dbg("g_CfgVersion = [%s],[%s]\n", g_patch_ctrl.cfg_version, g_patch_ctrl.cmd[index].cmd_para);
    }
    return SUCC;
}

/*
 * Prototype    : parse_cfg_get_oneline
 * Description  : 获取配置文件中的一行,配置文件必须是unix格式
 */
static int32_t parse_cfg_get_oneline(uint8_t *buf, int32_t buf_len, uint8_t* cur, uint8_t** next)
{
    uint8_t *begin = NULL;
    uint8_t *end = NULL;

    begin = cur;
    end = os_str_chr(cur, '\n');
    if (end == NULL) { /* 文件的最后一行，没有换行符 */
        ps_print_dbg("lost of new line!\n");
        end = &buf[buf_len];
    } else if (end == begin) { /* 该行只有一个换行符 */
        ps_print_dbg("blank line\n");
        *next = end + 1;
        return -EFAIL;
    }
    *end = '\0';
    *next = end + 1;

    ps_print_info("operation string is [%s]\n", begin);
    return SUCC;
}
/*
 * Prototype    : patch_parse_cfg
 * Description  : parse config file
 */
int32_t patch_parse_cfg(uint8_t *buf, int32_t buf_len)
{
    int32_t i;
    uint8_t *cur = NULL;
    uint8_t *next = NULL;
    int32_t cmd_type;
    uint8_t cmd_name[DOWNLOAD_CMD_LEN];
    uint8_t cmd_para[DOWNLOAD_CMD_PARA_LEN];

    if (buf == NULL) {
        ps_print_err("buf is NULL!\n");
        return -EFAIL;
    }

    g_patch_ctrl.cmd = (struct cmd_type_st *)patch_malloc_cmd_buf(buf);
    if (g_patch_ctrl.cmd == NULL) {
        ps_print_err(" patch_malloc_cmd_buf fail\n");
        return -EFAIL;
    }

    /* 解析CMD BUF */
    next = buf;
    i = 0;
    while ((i < g_patch_ctrl.count)) {
        cur = next;
        if (cur >= &buf[buf_len]) {
            break;
        }

        if (parse_cfg_get_oneline(buf, buf_len, cur, &next) != SUCC) {
            continue;
        }

        memset_s(cmd_name, DOWNLOAD_CMD_LEN, 0, DOWNLOAD_CMD_LEN);
        memset_s(cmd_para, DOWNLOAD_CMD_PARA_LEN, 0, DOWNLOAD_CMD_PARA_LEN);

        cmd_type = firmware_parse_cmd(cur, cmd_name, sizeof(cmd_name), cmd_para, sizeof(cmd_para));

        ps_print_info("cmd type=[%d],cmd_name=[%s],cmd_para=[%s]\n", cmd_type, cmd_name, cmd_para);

        if (cmd_type != ERROR_TYPE_CMD) { /* 正确的命令类型，增加 */
            if (parse_cfg_add_cmd(i, cmd_type, cmd_name, cmd_para) != SUCC) {
                return -EFAIL;
            }
            i++;
        }
    }

    /* 根据实际命令个数，修改最终的命令个数 */
    g_patch_ctrl.count = i;
    ps_print_info("cmd count[%d]\n", g_patch_ctrl.count);

    return SUCC;
}

/*
 * Prototype    : patch_get_cfg
 * Description  : get patch config command
 */
int32_t patch_get_cfg(const char *cfg)
{
    uint8_t *buf = NULL;
    int32_t l_readlen;
    int32_t l_ret;

    if (cfg == NULL) {
        ps_print_err("cfg file path null!\n");
        return -EFAIL;
    }

    buf = os_kmalloc_gfp(READ_CFG_BUF_LEN);
    if (buf == NULL) {
        ps_print_err("malloc READ_CFG_BUF fail!\n");
        return -EFAIL;
    }

    /* cfg文件限定在小于2048,如果cfg文件的大小确实大于2048，可以修改READ_CFG_BUF_LEN的值 */
    l_readlen = firmware_read_cfg(cfg, buf, READ_CFG_BUF_LEN);
    if (l_readlen < 0) {
        ps_print_err("read cfg error\n");
        os_mem_kfree(buf);
        buf = NULL;
        return -EFAIL;
    } else if (l_readlen > READ_CFG_BUF_LEN - 1) {
        /* 减1是为了确保cfg文件的长度不超过READ_CFG_BUF_LEN，因为firmware_read_cfg最多只会读取READ_CFG_BUF_LEN长度的内容 */
        ps_print_err("cfg file [%s] larger than %d\n", cfg, READ_CFG_BUF_LEN);
        os_mem_kfree(buf);
        buf = NULL;
        return -EFAIL;
    } else {
        ps_print_info("read cfg file [%s] ok, size is [%d]\n", cfg, l_readlen);
    }

    l_ret = patch_parse_cfg(buf, l_readlen);
    if (l_ret < 0) {
        ps_print_err("parse cfg error\n");
    }

    os_mem_kfree(buf);
    buf = NULL;

    return l_ret;
}

/*
 * Prototype    : patch_download_info
 * Description  : download patch
 */
int32_t patch_execute_cmd(int32_t cmd_type, uint8_t *cmd_name, uint8_t *cmd_para)
{
    int32_t l_ret;

    /* 清空上次操作遗留下来的数据，读取结果时以长度为判断，buf就不用清空了 */
    g_patch_ctrl.resv_buf1_len = 0;
    g_patch_ctrl.resv_buf2_len = 0;

    switch (cmd_type) {
        case FILE_TYPE_CMD:
            ps_print_dbg(" command type FILE_TYPE_CMD\n");
            l_ret = patch_file_type(cmd_name, cmd_para);
            break;
        case NUM_TYPE_CMD:
            ps_print_dbg(" command type NUM_TYPE_CMD\n");
            l_ret = patch_number_type(cmd_name, cmd_para);

            break;
        case QUIT_TYPE_CMD:
            ps_print_dbg(" command type QUIT_TYPE_CMD\n");
            l_ret = patch_quit_type();
            break;

        default:
            ps_print_err("command type error[%d]\n", cmd_type);
            l_ret = -EFAIL;
            break;
    }

    return l_ret;
}

/*
 * Prototype    : patch_download_patch
 * Description  : download patch
 */
int32_t patch_download_patch(void)
{
    int32_t l_ret;
    int32_t i;
    uint32_t ul_alloc_len = READ_DATA_BUF_LEN;

    g_stringbuf.pbufstart = kmalloc(ul_alloc_len, GFP_KERNEL);
    if (g_stringbuf.pbufstart == NULL) {
        ul_alloc_len = READ_DATA_REALLOC_BUF_LEN;
        g_stringbuf.pbufstart = kmalloc(ul_alloc_len, GFP_KERNEL);
        if (g_stringbuf.pbufstart == NULL) {
            ps_print_err("ringbuf KMALLOC SIZE(%d) failed.\n", ul_alloc_len);
            g_stringbuf.pbufstart = g_patch_ctrl.resv_buf1;
            g_stringbuf.pbufend = RECV_BUF_LEN + g_stringbuf.pbufstart;

            return -EFAIL;
        }
    }

    ps_print_info("ringbuf kmalloc size(%d) suc.\n", ul_alloc_len);
    g_stringbuf.pbufend = ul_alloc_len + g_stringbuf.pbufstart;

    g_stringbuf.phead = g_stringbuf.pbufstart;
    g_stringbuf.ptail = g_stringbuf.pbufstart;

    /* 执行条件:: 命令行没有读完，命令不是错误命令  */
    for (i = 0; i < g_patch_ctrl.count; i++) {
        ps_print_info("cmd[%d]type[%d], name[%s], para[%s]\n",
                      i,
                      g_patch_ctrl.cmd[i].cmd_type,
                      g_patch_ctrl.cmd[i].cmd_name,
                      g_patch_ctrl.cmd[i].cmd_para);

        l_ret = patch_execute_cmd(g_patch_ctrl.cmd[i].cmd_type,
                                  g_patch_ctrl.cmd[i].cmd_name,
                                  g_patch_ctrl.cmd[i].cmd_para);
        if (l_ret < 0) {
            kfree(g_stringbuf.pbufstart);
            g_stringbuf.pbufstart = NULL;
            g_stringbuf.pbufend = NULL;
            g_stringbuf.phead = NULL;
            g_stringbuf.ptail = NULL;
            return l_ret;
        }
    }

    kfree(g_stringbuf.pbufstart);
    g_stringbuf.pbufstart = NULL;
    g_stringbuf.pbufend = NULL;
    g_stringbuf.phead = NULL;
    g_stringbuf.ptail = NULL;

    return SUCC;
}

STATIC void uart_cfg_file_init(void)
{
    int32_t l_subchip_type = get_hi110x_subchip_type();
    if (l_subchip_type == BOARD_VERSION_HI1105) {
        if (!hi110x_is_asic()) {
            memcpy_s(g_patch_ctrl.cfg_path, sizeof(g_patch_ctrl.cfg_path),
                     UART_CFG_FILE_FPGA_HI1105, os_str_len(UART_CFG_FILE_FPGA_HI1105));
        } else {
            memcpy_s(g_patch_ctrl.cfg_path, sizeof(g_patch_ctrl.cfg_path),
                     UART_CFG_FILE_PILOT_HI1105, os_str_len(UART_CFG_FILE_PILOT_HI1105));
        }
    } else if (l_subchip_type == BOARD_VERSION_HI1106) {
        if (!hi110x_is_asic()) {
            memcpy_s(g_patch_ctrl.cfg_path, sizeof(g_patch_ctrl.cfg_path),
                     UART_CFG_FILE_FPGA_HI1106, os_str_len(UART_CFG_FILE_FPGA_HI1106));
        } else {
            memcpy_s(g_patch_ctrl.cfg_path, sizeof(g_patch_ctrl.cfg_path),
                     UART_CFG_FILE_PILOT_HI1106, os_str_len(UART_CFG_FILE_PILOT_HI1106));
        }
    } else {
        ps_print_err("subchip %d unsupport uart download\n", l_subchip_type);
    }
}

/*
 * Prototype    : patch_init
 * Description  : patch module initialization
 */
int32_t patch_init(void)
{
    int32_t l_ret;

    uart_cfg_file_init();

    g_patch_ctrl.wait = os_kmalloc_gfp(sizeof(os_wait_queue_head));
    if (g_patch_ctrl.wait == NULL) {
        ps_print_err("malloc wait queue fail, size:%lu\n", sizeof(os_wait_queue_head));
        return -EFAIL;
    }

    os_init_waitqueue_head(g_patch_ctrl.wait);

    l_ret = patch_get_cfg(g_patch_ctrl.cfg_path);
    if (l_ret < 0) {
        ps_print_err("get [%s] command is fail\n", g_patch_ctrl.cfg_path);
        return -EFAIL;
    }

    if (g_data_buf == NULL) {
        g_data_buf = os_kmalloc_gfp(READ_PATCH_BUF_LEN);
        if (g_data_buf == NULL) {
            ps_print_err("DataBuf KMALLOC failed");
            g_data_buf = NULL;
            return -EFAIL;
        } else {
            ps_print_dbg("DataBuf KMALLOC succ");
        }
    }

    return SUCC;
}

/*
 * Prototype    : patch_exit
 * Description  : patch module exit
 */
int32_t patch_exit(void)
{
    g_patch_ctrl.count = 0;
    if (g_patch_ctrl.cmd != NULL) {
        os_mem_kfree(g_patch_ctrl.cmd);
        g_patch_ctrl.cmd = NULL;
    }

    if (g_patch_ctrl.wait != NULL) {
        os_mem_kfree(g_patch_ctrl.wait);
        g_patch_ctrl.wait = NULL;
    }

    if (g_data_buf != NULL) {
        os_mem_kfree(g_data_buf);
        g_data_buf = NULL;
    }

    return SUCC;
}

/*
 * Prototype    : bfg_patch_download_function
 * Description  : download bfg patch
 * Return       : 0 means succeed,-1 means failed
 */
int bfg_patch_download_function(void)
{
#define RETRY_CNT 3
    int32_t ret;
    int32_t counter = 0;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_time = 0;
    static unsigned long long count = 0;
    unsigned long long total_time = 0;

    struct ps_core_s *ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -1;
    }

    ps_print_dbg("enter\n");
    start_time = ktime_get();

    /* patch_init(uart) function should just be called once */
    ret = patch_init();
    if (ret) {
        ps_print_err("patch modem init failed, ret:%d!\n", ret);
        patch_exit();
        return ret;
    }

    /* bfg patch download, three times for fail */
    for (counter = 0; counter < RETRY_CNT; counter++) {
        ret = patch_download_patch();
        if (ret) {
            ps_print_err("bfg patch download fail, and reset power!\n");
            ps_uart_state_dump(ps_core_d);
        } else {
            end_time = ktime_get();
            trans_time = ktime_sub(end_time, start_time);
            total_time = (unsigned long long)ktime_to_us(trans_time);
            if (total_time > max_time) {
                max_time = total_time;
            }

            ps_print_warning("download bfg patch succ,count [%llu], current time [%llu]us, max time [%llu]us\n",
                             count, total_time, max_time);
            /* download patch successfully */
            patch_exit();
            return ret;
        }
    }

    /* going to exception */
    ps_print_err("bfg patch download has failed finally!\n");
    patch_exit();
    return ret;
}

