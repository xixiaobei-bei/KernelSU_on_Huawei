

/* 头文件包含 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/syscalls.h>
#include <linux/dma-mapping.h>
#include <linux/stat.h>
#include <linux/string.h>

#include "wireless_patch.h"
#include "plat_pm.h"
#include "securec.h"

/* 全局变量定义 */
PATCH_GLOBALS_STUR global;
RINGBUF_STRU stringbuf;
uint8 *DataBuf_t;

/* xmodem 索引 */
uint8 xmodem_index = 1;

unsigned short CRC_table[CRC_TABLE_SIZE] = {
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
 * Prototype    : pm_uart_set_baudrate
 * Description  : set baudrate of uart0 when download patch
 * Input        : int64 baudrate: the baudrate want to set
 * Return       : 0 means succeed,-1 means failed.
 */
int32 pm_uart_set_baudrate(int64 baudrate)
{
    /* get platform driver data */
    struct pm_drv_data *pm_data = pm_get_drvdata_etc();
    if (pm_data == NULL) {
        PS_PRINT_ERR("pm_data is NULL!\n");
        return -1;
    }

    PS_PRINT_DBG("set uart baudrate to %ld\n", baudrate);

    /* call interface supplied by 3 in 1 */
    if (pm_data->ps_pm_interface->change_baud_rate != NULL) {
        /* set uart baudrate */
        return pm_data->ps_pm_interface->change_baud_rate(baudrate, FLOW_CTRL_ENABLE);
    }

    PS_PRINT_ERR("change_baud_rate is NULL!\n");

    return -1;
}

/*
 * Prototype    : do_crc_table_1
 * Description  : CRC校验
 */
unsigned short do_crc_table_1(uint8 *data, uint16 length)
{
    uint16 crc = 0;
    uint8 dataTmp;
    while (length > 0) {
        dataTmp = (uint8)(crc >> 8);
        crc = crc << 8;
        crc = crc ^ CRC_table[dataTmp ^ (*data)];
        length--;
        data++;
    }
    return crc;
}

/*
 * Prototype    : patch_send
 * Description  : send message to device,by sdio or uart
 */
int32 patch_send(uint8 *data, int32 len, uint8 expect, int32 type)
{
    int32 i;
    int32 l_ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = send_msg_t(data, len, type);
        if (l_ret < 0) {
            continue;
        }

        l_ret = recv_expect_result_t(expect, type);
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
int32 patch_xmodem_send(uint8 *data, int32 len, uint8 expect)
{
    XMODEM_HEAD_PKT_STRU st_patch_pkt;
    uint16 us_CRCValue;
    int32 l_ret;
    int32 l_sendlen;
    int32 l_datalen;
    int32 i;
    uint8 *flag = NULL;
    uint8 CRCValue_H;
    uint8 CRCValue_L;

    if (data == NULL) {
        return -EFAIL;
    }

    l_datalen = len;
    flag = data;

    while (l_datalen > 0) {
        l_sendlen = MIN(XMODE_DATA_LEN, l_datalen);
        l_datalen = l_datalen - l_sendlen;
        st_patch_pkt.Head = SOH;
        st_patch_pkt.PacketNum = xmodem_index;

        /* 数据长度不够128个 */
        if (l_sendlen < XMODE_DATA_LEN) {
            PS_PRINT_DBG("data_len  %d\n", l_sendlen);
            memset_s(&flag[l_sendlen], (XMODE_DATA_LEN - l_sendlen), 0x00, (XMODE_DATA_LEN - l_sendlen));
        }
        us_CRCValue = do_crc_table_1(flag, XMODE_DATA_LEN);
        CRCValue_H = (us_CRCValue & 0xFF00) >> 8;
        CRCValue_L = us_CRCValue & 0xFF;

        st_patch_pkt.PacketAnt = ~(st_patch_pkt.PacketNum);

        for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
            l_ret = pm_uart_send((uint8 *)&st_patch_pkt, sizeof(st_patch_pkt));
            l_ret = pm_uart_send(flag, XMODE_DATA_LEN);
            l_ret = pm_uart_send(&CRCValue_H, 1);
            l_ret = pm_uart_send(&CRCValue_L, 1);

            l_ret = recv_expect_result_t(expect, ENUM_INFO_UART);
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

        xmodem_index++;
    }

    return SUCC;
}

static void ringbuf_flush(void)
{
    /* writing maybe still work when interrupt by flush  */
    stringbuf.ptail = stringbuf.phead;
}

/*
 * Prototype    : uart_recv_data
 * Description  : receiver data form device,by uart interface
 */
int32 uart_recv_data(const uint8 *data, int32 len)
{
    uint32 ulbuflen;
    uint32 ulheadtoendlen = 0;
    uint8 *ptail = NULL;
    if (unlikely((data == NULL))) {
        PS_PRINT_ERR("data is NULL\n ");
        return -EFAIL;
    }
    if ((stringbuf.pbufstart == NULL) || (stringbuf.pbufend < stringbuf.pbufstart)) {
        PS_PRINT_ERR("buf is NULL,write failed\n ");
        PS_PRINT_ERR("pbufstart=%p,pbufend=%p\n", stringbuf.pbufstart, stringbuf.pbufend);
        return -EFAIL;
    }
    ptail = stringbuf.ptail;
    if ((stringbuf.phead < stringbuf.pbufstart) ||
        (stringbuf.phead > stringbuf.pbufend) ||
        (ptail < stringbuf.pbufstart) ||
        (ptail > stringbuf.pbufend)) {
        PS_PRINT_ERR("phead or ptail is out of range,write failed\n");
        PS_PRINT_ERR("pbufstart=%p,pbufend=%p\n", stringbuf.pbufstart, stringbuf.pbufend);
        PS_PRINT_ERR("phead=%p,ptail=%p\n", stringbuf.phead, stringbuf.ptail);
        return -EFAIL;
    }
    ulbuflen = stringbuf.pbufend - stringbuf.pbufstart;
    PS_PRINT_DBG("len [%d],\n ", len);

    /* phead and ptail are in the same cycle */
    if (stringbuf.phead >= ptail) {
        /* still in the same cycle */
        if ((stringbuf.pbufend - stringbuf.phead) >= len) {
            memcpy_s(stringbuf.phead, stringbuf.pbufend - stringbuf.phead, data, len);
            stringbuf.phead += len;
            if (OS_WAITQUEUE_ACTIVE(global.pst_wait)) {
                OS_WAKE_UP_INTERRUPTIBLE(global.pst_wait);
                PS_PRINT_DBG("wake up ok");
            }
        } else if ((ulbuflen - (stringbuf.phead - ptail)) > len) {
            ulheadtoendlen = stringbuf.pbufend - stringbuf.phead;
            memcpy_s(stringbuf.phead, ulheadtoendlen, data, ulheadtoendlen);
            memcpy_s(stringbuf.pbufstart, stringbuf.ptail - stringbuf.pbufstart,
                     data + ulheadtoendlen, len - ulheadtoendlen);
            stringbuf.phead = stringbuf.pbufstart + (len - ulheadtoendlen);
            PS_PRINT_INFO("phead back\n");
            if (OS_WAITQUEUE_ACTIVE(global.pst_wait)) {
                OS_WAKE_UP_INTERRUPTIBLE(global.pst_wait);
                PS_PRINT_DBG("wake up ok");
            }
        } else {
            PS_PRINT_ERR("Not enough mem,len=%d.\n ", len);
        }
    }
    /* phead is in the next cycle */
    /* "ptail - phead = 1" means the buffer is full */
    else if ((ptail - stringbuf.phead - 1) > len) {
        memcpy_s(stringbuf.phead, ptail - stringbuf.phead - 1, data, len);
        stringbuf.phead += len;
        if (OS_WAITQUEUE_ACTIVE(global.pst_wait)) {
            OS_WAKE_UP_INTERRUPTIBLE(global.pst_wait);
            PS_PRINT_DBG("wake up ok");
        }
    } else {
        PS_PRINT_ERR("Not enough mem,len=%d.\n ", len);
    }

    if (stringbuf.phead >= stringbuf.pbufend) {
        stringbuf.phead = stringbuf.pbufstart;
        PS_PRINT_INFO("phead back\n");
    }
    return SUCC;
}

int32 read_msg_t(uint8 *data, int32 len, int32 type)
{
    int32 l_len = -1;
    uint32 ultailtoendlen;
    uint8 *phead = NULL;

    if (unlikely((data == NULL))) {
        PS_PRINT_ERR("data is NULL\n ");
        return -EFAIL;
    }

    if ((stringbuf.pbufstart == NULL) || (stringbuf.pbufstart > stringbuf.pbufend)) {
        PS_PRINT_ERR("buf is NULL,read failed\n ");
        PS_PRINT_ERR("pbufstart=%p,pbufend=%p\n", stringbuf.pbufstart, stringbuf.pbufend);
        return -EFAIL;
    }

    if ((stringbuf.phead < stringbuf.pbufstart) ||
        (stringbuf.phead > stringbuf.pbufend) ||
        (stringbuf.ptail < stringbuf.pbufstart) ||
        (stringbuf.ptail > stringbuf.pbufend)) {
        PS_PRINT_ERR("phead or ptail is out of range, read failed\n");
        PS_PRINT_ERR("pbufstart=%p,pbufend=%p\n", stringbuf.pbufstart, stringbuf.pbufend);
        PS_PRINT_ERR("phead=%p,ptail=%p\n", stringbuf.phead, stringbuf.ptail);
        return -EFAIL;
    }

    OS_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(*global.pst_wait,
                                        (stringbuf.phead != stringbuf.ptail),
                                        PATCH_INTEROP_TIMEOUT);
    phead = stringbuf.phead;
    /* phead and ptail are in the same cycle */
    if (phead > stringbuf.ptail) {
        if ((phead - stringbuf.ptail) > len) {
            memcpy_s(data, len, stringbuf.ptail, len);
            l_len = len;
            stringbuf.ptail += len;
        }
        /* not enough data */
        else {
            l_len = phead - stringbuf.ptail;
            memcpy_s(data, len, stringbuf.ptail, l_len);
            stringbuf.ptail += l_len;
        }
    }
    /* phead is in the next cycle */
    else if (phead < stringbuf.ptail) {
        ultailtoendlen = stringbuf.pbufend - stringbuf.ptail;
        if (ultailtoendlen > len) {
            memcpy_s(data, len, stringbuf.ptail, len);
            l_len = len;
            stringbuf.ptail += len;
        } else {
            memcpy_s(data, len, stringbuf.ptail, ultailtoendlen);
            if ((phead - stringbuf.pbufstart) > (len - ultailtoendlen)) {
                memcpy_s(data + ultailtoendlen, len - ultailtoendlen,
                         stringbuf.pbufstart, len - ultailtoendlen);
                stringbuf.ptail = stringbuf.pbufstart + (len - ultailtoendlen);
                l_len = len;
                PS_PRINT_INFO("ptail back\n");
            } else {
                memcpy_s(data + ultailtoendlen, len - ultailtoendlen,
                         stringbuf.pbufstart, phead - stringbuf.pbufstart);
                l_len = ultailtoendlen + (phead - stringbuf.pbufstart);
                stringbuf.ptail = phead;
            }
        }
    } else {
        l_len = -1;
        PS_PRINT_WARNING("No data.\n");
    }
    if (stringbuf.ptail >= stringbuf.pbufend) {
        stringbuf.ptail = stringbuf.pbufstart;
    }

    return l_len;
}

/*
 * Prototype    : send_msg_etc
 * Description  : send message to device,by sdio or uart
 */
int32 send_msg_t(uint8 *data, int32 len, int32 type)
{
    return pm_uart_send(data, len);
}

/*
 * Prototype    : recv_expect_result_etc
 * Description  : receive result form device
 */
int32 recv_expect_result_t(uint8 expect, int32 type)
{
    uint8 auc_buf[RECV_BUF_LEN];
    int32 l_len;
    int32 i;

    PS_PRINT_DBG(" entry\n");

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_len = read_msg_t(auc_buf, 1, type);
        if (l_len < 0) {
            PS_PRINT_ERR("recv result fail\n");
            return -EFAIL;
        }
        if (auc_buf[0] == expect) {
            PS_PRINT_DBG(" send SUCC [%x]\n", expect);
            return SUCC;
        }
        /*
         * NAK: 文件传输时重发标识
         * MSG_FORM_DRV_N:其他重发标识
         */
        else if ((auc_buf[0] == MSG_FORM_DRV_N) || (auc_buf[0] == NAK) || (auc_buf[0] == MSG_FORM_DRV_C)) {
            PS_PRINT_ERR(" send again [0x%x]\n", auc_buf[0]);
            return -EFAIL;
        } else {
            /* 对于错误的结果，有十次的机会， */
            PATCH_SEND_N_UART;
            PS_PRINT_WARNING(" error result[0x%x], expect [0x%x], read result again\n", auc_buf[0], expect);
        }
    }
    return -EFAIL;
}

int32 patch_string_to_num(uint8 *string)
{
    int32 i;
    int32 l_num;

    if (string == NULL) {
        return -EFAIL;
    }

    l_num = 0;
    for (i = 0; (string[i] >= '0') && (string[i] <= '9'); i++) {
        l_num = (l_num * 10) + (string[i] - '0'); /* 字符串转数字的逻辑需要 */
    }

    return l_num;
}

/*
 * Prototype    : patch_wait_g_form_dev
 * Description  : wait go'command form device
 */
int32 patch_wait_g_form_dev(int32 type)
{
    int32 l_ret;
    int32 i;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = recv_expect_result_t(MSG_FORM_DRV_G, type);
        if (l_ret == 0) {
            PS_PRINT_DBG(" device finish G\n");
            return SUCC;
        }
    }

    PS_PRINT_ERR("recv g FAIL\n");
    return -EFAIL;
}

/*
 * Prototype    : patch_wait_g_form_dev
 * Description  : wait go'command form device
 */
int32 patch_wait_g_retry_form_dev(int32 type)
{
    int32 l_ret;
    int32 i;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = recv_expect_result_t(MSG_FORM_DRV_G, type);
        if (l_ret == 0) {
            PS_PRINT_DBG(" device finish G\n");
            return SUCC;
        } else {
            PATCH_SEND_N_UART;
            PS_PRINT_WARNING("receive G failed\n");
        }
    }
    return -EFAIL;
}

/*
 * Prototype    : patch_send_char
 * Description  : send char to device
 */
int32 patch_send_char(int8 num, int32 wait, int32 type)
{
    int32 l_ret;
    const uint32 ul_buf_size = 8;
    uint8 auc_buf[ul_buf_size];
    int32 i;

    memset_s(auc_buf, sizeof(auc_buf), num, sizeof(auc_buf));
    PS_PRINT_DBG("send [0x%x], wait[%d]\n", num, wait);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        /*
         * sdio 接口发送时，会四字节对齐，发送四个
         * uart 接口发送时，只发送一个
         */
        l_ret = send_msg_t(auc_buf, 1, type);
        if (l_ret < 0) {
            PS_PRINT_ERR("Send fail\n");
            return l_ret;
        }

        if (wait == WAIT_RESPONSE) {
            l_ret = recv_expect_result_t(ACK, type);
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
int32 patch_read_patch(int8 *buf, int32 len, OS_KERNEL_FILE_STRU *fp)
{
    int32 rdlen;

    if ((IS_ERR(fp)) || (buf == NULL)) {
        fp = NULL;
        PS_PRINT_ERR("buf/fp is NULL\n");
        return -EFAIL;
    }

    rdlen = kernel_read(fp, fp->f_pos, buf, len);
    if (rdlen > 0) {
        fp->f_pos += rdlen;
    }

    return rdlen;
}

/*
 * Prototype    : patch_down_file
 * Description  : begin download patch file
 */
int32 patch_down_file(const uint8 *puc_file, int32 type)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    uint8 *auc_buf;
    int32 l_len;
    int32 l_ret;
    int32 l_count;
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
        PS_PRINT_ERR("filp_open %s fail!!\n", puc_file);
        return -EFAIL;
    }

    if (DataBuf_t == NULL) {
        filp_close(fp, NULL);
        set_fs(fs);
        fp = NULL;
        return -EFAIL;
    }

    l_count = 1;
    xmodem_index = 1;
    while (1) {
        l_len = patch_read_patch(DataBuf_t, READ_PATCH_BUF_LEN, fp);
        PS_PRINT_DBG("kernel_read len[%d] [%d]\n", l_len, l_count);
        /* 正常读取文件 */
        if ((l_len > 0) && (l_len <= READ_PATCH_BUF_LEN)) {
            l_ret = patch_xmodem_send(DataBuf_t, l_len, ACK);
            if (l_ret == -EFAIL) {
                PS_PRINT_ERR(" uart send data[%d] fail\n", l_count);
                break;
            }
        }
        /* 文件已经读取完成 */
        else if (l_len == 0) {
            PATCH_SEND_EOT_UART;
            xmodem_index = 1;
            l_ret = SUCC;

            PS_PRINT_DBG("read file[%d] [%d] send EOT\n", l_count, l_len);
            break;
        }
        /* 读取文件出错 */
        else {
            PATCH_SEND_CAN_UART;
            xmodem_index = 1;
            l_ret = -EFAIL;
            PS_PRINT_ERR("read file[%d] [%d]\n", l_count, l_len);
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
OS_KERNEL_FILE_STRU *patch_readm_fileopen(int32 type)
{
    OS_KERNEL_FILE_STRU *fp = NULL;
    mm_segment_t fs;
    struct timeval tv;
    struct rtc_time tm;
    const uint32 ul_filename_len = 50;
    char filename[ul_filename_len];
    int ret;

    do_gettimeofday(&tv);
    rtc_time_to_tm(tv.tv_sec, &tm);

    PS_PRINT_INFO("%4d-%02d-%02d  %02d:%02d:%02d\n",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
    ret = snprintf_s(filename, sizeof(filename), sizeof(filename) - 1,
                     "/data/log/hi110x/readm_bfg%04d%02d%02d%02d%02d%02d.bin",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
    if (ret < 0) {
        PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
        return NULL;
    }

    PS_PRINT_INFO("filename = %s", filename);
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
int32 patch_recv_mem(OS_KERNEL_FILE_STRU *fp, int32 len, int32 type)
{
    uint8 *pdatabuf = NULL;
    int32 l_ret;
    mm_segment_t fs;
    int32 lenbuf = 0;

    if (OAL_IS_ERR_OR_NULL(fp)) {
        PS_PRINT_ERR("fp is error,fp = 0x%p\n", fp);
        return -EFAIL;
    }

    pdatabuf = OS_KMALLOC_GFP(len);
    if (pdatabuf == NULL) {
        return -EFAIL;
    }

    memset_s(pdatabuf, len, 0, len);

    fs = get_fs();
    set_fs(KERNEL_DS);

    while (len > lenbuf) {
        l_ret = read_msg_t(pdatabuf + lenbuf, len - lenbuf, type);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            PS_PRINT_ERR("time out\n");
            break;
        }
    }

    if (len <= lenbuf) {
        vfs_write(fp, pdatabuf, len, &fp->f_pos);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif

    set_fs(fs);
    ringbuf_flush();
    OS_MEM_KFREE(pdatabuf);

    return l_ret;
}

/*
 * Prototype    : patch_int_para_send
 * Description  : down integer to device
 */
int32 patch_int_para_send(uint8 *name, uint8 *Value, int32 type)
{
    int32 l_ret;
    int32 data_len;
    int32 Value_len;
    int32 i;
    int32 n;
    uint8 auc_num[INT32_STR_LEN];
    uint8 data[DATA_BUF_LEN];

    Value_len = OS_STR_LEN((int8 *)Value);

    memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
    memset_s(data, DATA_BUF_LEN, 0, DATA_BUF_LEN);

    data_len = 0;
    data_len = OS_STR_LEN(name);
    l_ret = memcpy_s(data, sizeof(data), name, data_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("data buff not enough\n");
        return -EFAIL;
    }

    data[data_len] = COMPART_KEYWORD;
    data_len = data_len + 1;

    for (i = 0, n = 0; (i <= Value_len) && (n < INT32_STR_LEN); i++) {
        if ((Value[i] == ',') || (Value_len == i)) {
            PS_PRINT_DBG("auc_num = %s, i = %d, n = %d\n", auc_num, i, n);
            if (n == 0) {
                continue;
            }
            l_ret = memcpy_s((uint8 *)&data[data_len], sizeof(data) - data_len, auc_num, n);
            if (l_ret != EOK) {
                PS_PRINT_ERR("data buff not enough\n");
                return -EFAIL;
            }
            data_len = data_len + n;

            data[data_len] = COMPART_KEYWORD;
            data_len = data_len + 1;

            memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
            n = 0;
        } else if (Value[i] == 0x20) {
            continue;
        } else {
            auc_num[n] = Value[i];
            n++;
        }
    }

    PS_PRINT_DBG("data_len = %d, \n", data_len);
    PS_PRINT_DBG("data = %s, \n", data);

    ringbuf_flush();

    l_ret = send_msg_t(data, data_len, type);

    return l_ret;
}

int32 patch_check_version(int32 type)
{
    int32 l_ret;
    int32 i = 0;
    int8 version_buff[VERSION_LEN] = {0};

    l_ret = snprintf_s(version_buff, sizeof(version_buff), sizeof(version_buff) - 1, "%s%c",
                       VER_CMD_KEYWORD, COMPART_KEYWORD);
    if (l_ret < 0) {
        PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
        return l_ret;
    }

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = send_msg_t(version_buff, VERSION_LEN, type);
        if (l_ret < 0) {
            continue;
        }

        l_ret = patch_device_respond(type);
        if (l_ret == 0) {
            return SUCC;
        }
    }

    return l_ret;
}

/*
 * Prototype    : patch_file_type
 * Description  : down parameter
 */
int32 patch_number_type(uint8 *Key, uint8 *Value, int32 type)
{
    int32 l_ret = -EFAIL;
    int32 num;
    uint8 *flag = NULL;
    OS_KERNEL_FILE_STRU *fp = NULL;

    if (!OS_MEM_CMP(Key, VER_CMD_KEYWORD, OS_STR_LEN(VER_CMD_KEYWORD))) {
        l_ret = patch_check_version(type);
        if (l_ret < 0) {
            PS_PRINT_ERR("version check fail\n");
            return l_ret;
        }
    } else if (!OS_STR_CMP((int8 *)Key, JUMP_CMD_KEYWORD) || !OS_STR_CMP((int8 *)Key, WMEM_CMD_KEYWORD)) {
        l_ret = patch_int_para_send(Key, Value, type);
        if (l_ret < 0) {
            PS_PRINT_ERR("send %s,%s fail \n", Key, Value);
            return l_ret;
        }
        /* G 是device 相应操作完成标志 */
        l_ret = patch_wait_g_form_dev(type);
    } else if (!OS_STR_CMP((int8 *)Key, BRT_CMD_KEYWORD)) {
        /* 修改波特率 */
        l_ret = patch_int_para_send(Key, Value, type);
        if (l_ret < 0) {
            PS_PRINT_ERR("send %s,%s fail \n", Key, Value);
            return l_ret;
        }

        l_ret = recv_expect_result_t(MSG_FORM_DRV_A, type);
        if (l_ret < 0) {
            PS_PRINT_ERR("send change baudrate fail\n");
            return l_ret;
        }

        num = patch_string_to_num(Value);
        PS_PRINT_INFO("change baudrate to:%d\n", num);

        /* 增加调用修改波特率函数 */
        l_ret = pm_uart_set_baudrate(num);
        if (l_ret < 0) {
            PS_PRINT_ERR(" modify baudrate fail!!\n");
            return -EFAIL;
        }

        ringbuf_flush();
        oal_usleep_range(10000, 11000);

        /* G 是device 相应操作完成标志 */
        l_ret = patch_wait_g_retry_form_dev(type);
    } else if (!OS_STR_CMP((int8 *)Key, RMEM_CMD_KEYWORD)) {
        fp = patch_readm_fileopen(type);
        if (IS_ERR(fp)) {
            return SUCC;
        }

        l_ret = patch_int_para_send(Key, Value, type);
        if (l_ret < 0) {
            PS_PRINT_ERR("send %s,%s fail \n", Key, Value);
            oal_file_close(fp, NULL);
            return l_ret;
        }

        flag = OS_STR_CHR(Value, ',');
        if (flag == NULL) {
            PS_PRINT_ERR("RECV LEN ERROR..\n");
            oal_file_close(fp, NULL);
            return -EFAIL;
        }
        flag++;
        while (*flag == COMPART_KEYWORD) {
            flag++;
        }
        num = patch_string_to_num(flag);

        PS_PRINT_INFO("recv len [%d]\n", num);

        l_ret = patch_recv_mem(fp, num, type);
        oal_file_close(fp, NULL);
    }
    return l_ret;
}

/*
 * Prototype    : patch_quit_type
 * Description  : down quit command
 */
int32 patch_quit_type(int32 type)
{
    int32 l_ret;
    int32 l_len;
    const uint32 ul_buf_len = 8;
    uint8 buf[ul_buf_len];
    PS_PRINT_DBG("entry\n");

    memset_s(buf, sizeof(buf), 0, sizeof(buf));

    l_ret = memcpy_s(buf, sizeof(buf), (uint8 *)QUIT_CMD, OS_STR_LEN(QUIT_CMD));
    if (l_ret != EOK) {
        PS_PRINT_ERR("buf not enough\n");
        return -EFAIL;
    }
    l_len = OS_STR_LEN(QUIT_CMD);

    buf[l_len] = COMPART_KEYWORD;
    l_len++;

    ps_patch_to_nomal();

    l_ret = send_msg_t(buf, l_len, type);

    return l_ret;
}

/*
 * Prototype    : patch_file_type
 * Description  : down addr and file
 */
int32 patch_file_addr_send(uint8 *data, int32 data_len, const char *file_path, int32 type)
{
    int32 l_ret;

    l_ret = patch_send(data, data_len, MSG_FORM_DRV_C, type);
    if (l_ret < 0) {
        PS_PRINT_ERR(" SEND file addr error\n");

        return -EFAIL;
    }

    PS_PRINT_DBG("file path is %s\n", file_path);

    l_ret = patch_down_file(file_path, type);
    PS_PRINT_DBG("patch_down_file:%d", l_ret);
    if (l_ret < 0) {
        PS_PRINT_ERR(" SEND file error\n");

        return l_ret;
    }
    /* G 是 DEVICE 完成相应操作标志 */
    l_ret = patch_wait_g_form_dev(type);

    return l_ret;
}

/*
 * Prototype    : patch_file_type
 * Description  : down addr and file
 */
int32 patch_file_type(uint8 *Key, const char *Value, int32 type)
{
    int32 i;
    int32 n;
    int32 l_ret;
    int32 l_len;
    uint8 auc_addr[INT32_STR_LEN];
    uint8 data[DATA_BUF_LEN];
    int32 data_len;
    const char *tmp = NULL;
    const char *tmp1 = NULL;

    PS_PRINT_DBG("Key = %s, Value = %s\n", Key, Value);

    /*
     * 根据关键字的最后一个字符，确定发送地址之后，device的返回值
     * 所以配置文件的关键字不能随意修改
     */
    memset_s(data, DATA_BUF_LEN, 0, DATA_BUF_LEN);
    data_len = OS_STR_LEN(Key);
    l_ret = memcpy_s(data, sizeof(data), Key, data_len);
    if (l_ret != EOK) {
        PS_PRINT_ERR("data not enough\n");
        return -EFAIL;
    }

    data[data_len] = COMPART_KEYWORD;
    data_len++;

    /* 兼容wifibootloader配置设置 */
    tmp1 = Value;
    while (*tmp1 == ' ') {
        tmp1++;
    }
    if (*tmp1 != '1') {
        PS_PRINT_ERR("para [%s] not begin with 1", tmp1);
        return -EFAIL;
    }
    tmp = OS_STR_CHR(tmp1, ',');
    if (tmp == NULL) {
        PS_PRINT_ERR("has no ',' string:[%s]", tmp);
        return -EFAIL;
    }
    tmp++;
    PS_PRINT_DBG("tmp is %s\n", tmp);

    memset_s(auc_addr, INT32_STR_LEN, 0, INT32_STR_LEN);
    for (i = 0, n = 0; tmp[i] != ',' && n < INT32_STR_LEN; i++) {
        if ((tmp[i] == ',') || (tmp[i] == COMPART_KEYWORD)) {
            break;
        } else {
            auc_addr[n] = tmp[i];
            n++;
        }
    }
    l_ret = memcpy_s((uint8 *)&data[data_len], sizeof(data) - data_len, auc_addr, n);
    if (l_ret != EOK) {
        PS_PRINT_ERR("data buff not enough\n");
        return -EFAIL;
    }
    data_len = data_len + n;

    data[data_len] = COMPART_KEYWORD;
    data_len++;

    PS_PRINT_DBG("data is %s\n", data);

    /* 删除头部的空格 */
    l_len = OS_STR_LEN((int8 *)tmp);
    for (i = i + 1; i < l_len; i++) {
        /* 兼容绝对路径和相对路径 */
        if ((tmp[i] == '/') || (tmp[i] == '.')) {
            break;
        }
    }

    l_ret = patch_file_addr_send(data, data_len, &tmp[i], type);
    return l_ret;
}
/*
 * Prototype    : patch_device_respond
 * Description  : wait respond form device
 */
int32 patch_device_respond(int32 type)
{
    int32 l_ret;
    int32 i;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        memset_s(global.auc_DevVersion, VERSION_LEN, 0, VERSION_LEN);
        msleep(1);
        l_ret = read_msg_t(global.auc_DevVersion, VERSION_LEN, type);
        if (l_ret < 0) {
            PS_PRINT_ERR("read fail![%d]\n", i);
            continue;
        } else if (!OS_MEM_CMP((int8 *)global.auc_DevVersion,
                               (int8 *)global.auc_CfgVersion,
                               OS_STR_LEN(global.auc_CfgVersion))) {
            PS_PRINT_INFO("Device Version = [%s], CfgVersion = [%s].\n",
                          global.auc_DevVersion, global.auc_CfgVersion);
            return SUCC;
        } else {
            PS_PRINT_INFO("Device Version = [%s], CfgVersion = [%s].\n",
                          global.auc_DevVersion, global.auc_CfgVersion);
        }
    }

    PS_PRINT_ERR("read device version fail![%d]\n", i);

    return -EFAIL;
}

/*
 * Prototype    : patch_parse_cfg
 * Description  : parse config file
 */
void *patch_malloc_cmd_buf(uint8 *buf, int32 type)
{
    int32 l_len;
    uint8 *flag = NULL;
    uint8 *p_buf = NULL;

    /* 统计命令个数 */
    flag = buf;
    global.l_count = 0;
    while (flag != NULL) {
        /* 一个正确的命令行结束符为 ; */
        flag = OS_STR_CHR(flag, CMD_LINE_SIGN);
        if (flag == NULL) {
            break;
        }
        global.l_count++;
        flag++;
    }

    PS_PRINT_INFO("l_count = %d\n", global.l_count);

    /* 申请存储命令空间 */
    l_len = (global.l_count + CFG_INFO_RESERVE_LEN) * sizeof(struct cmd_type_st);
    p_buf = OS_KMALLOC_GFP(l_len);
    if (p_buf == NULL) {
        PS_PRINT_ERR("kmalloc cmd_type_st fail\n");
        return NULL;
    }
    memset_s((void *)p_buf, l_len, 0, l_len);

    return p_buf;
}

/*
 * Prototype    : patch_parse_cfg
 * Description  : parse config file
 */
int32 patch_parse_cfg(uint8 *buf, int32 buf_len, int32 type)
{
    int32 i;
    int32 l_len;
    uint8 *flag = NULL;
    uint8 *begin = NULL;
    uint8 *end = NULL;
    int32 cmd_type;
    uint8 cmd_name[DOWNLOAD_CMD_LEN];
    uint8 cmd_para[DOWNLOAD_CMD_PARA_LEN];
    uint32 cmd_para_len = 0;
    int32 ret;

    if (buf == NULL) {
        PS_PRINT_ERR("buf is NULL!\n");
        return -EFAIL;
    }

    global.pst_cmd = (struct cmd_type_st *)patch_malloc_cmd_buf(buf, type);
    if (global.pst_cmd == NULL) {
        PS_PRINT_ERR(" patch_malloc_cmd_buf fail\n");
        return -EFAIL;
    }

    /* 解析CMD BUF */
    flag = buf;
    l_len = buf_len;
    i = 0;
    while ((i < global.l_count) && (flag < &buf[l_len])) {
        /*
         * 获取配置文件中的一行,配置文件必须是unix格式.
         * 配置文件中的某一行含有字符 @ 则认为该行为注释行
         */
        begin = flag;
        end = OS_STR_CHR(flag, '\n');
        if (end == NULL) { /* 文件的最后一行，没有换行符 */
            PS_PRINT_DBG("lost of new line!\n");
            end = &buf[l_len];
        } else if (end == begin) { /* 该行只有一个换行符 */
            PS_PRINT_DBG("blank line\n");
            flag = end + 1;
            continue;
        }
        *end = '\0';

        PS_PRINT_INFO("operation string is [%s]\n", begin);

        memset_s(cmd_name, DOWNLOAD_CMD_LEN, 0, DOWNLOAD_CMD_LEN);
        memset_s(cmd_para, DOWNLOAD_CMD_PARA_LEN, 0, DOWNLOAD_CMD_PARA_LEN);

        cmd_type = firmware_parse_cmd_etc(begin, cmd_name, sizeof(cmd_name), cmd_para, sizeof(cmd_para));

        PS_PRINT_INFO("cmd type=[%d],cmd_name=[%s],cmd_para=[%s]\n", cmd_type, cmd_name, cmd_para);

        if (cmd_type != ERROR_TYPE_CMD) { /* 正确的命令类型，增加 */
            global.pst_cmd[i].cmd_type = cmd_type;
            memcpy_s(global.pst_cmd[i].cmd_name, DOWNLOAD_CMD_LEN, cmd_name, DOWNLOAD_CMD_LEN);
            memcpy_s(global.pst_cmd[i].cmd_para, DOWNLOAD_CMD_PARA_LEN,
                     cmd_para, DOWNLOAD_CMD_PARA_LEN);
            /* 获取配置版本号 */
            if (!OS_MEM_CMP(global.pst_cmd[i].cmd_name,
                            VER_CMD_KEYWORD,
                            OS_STR_LEN(VER_CMD_KEYWORD))) {
                cmd_para_len = OS_STR_LEN(global.pst_cmd[i].cmd_para);
                ret = memcpy_s(global.auc_CfgVersion, sizeof(global.auc_CfgVersion),
                               global.pst_cmd[i].cmd_para, cmd_para_len);
                if (ret != EOK) {
                    PS_PRINT_ERR("cmd_para_len = %d over auc_CfgVersion length\n", cmd_para_len);
                    return -EFAIL;
                }

                PS_PRINT_DBG("g_CfgVersion = [%s],[%s]\n",
                             global.auc_CfgVersion, global.pst_cmd[i].cmd_para);
            }
            i++;
        }
        flag = end + 1;
    }

    /* 根据实际命令个数，修改最终的命令个数 */
    global.l_count = i;
    PS_PRINT_INFO("type[%d], cmd count[%d]\n", type, global.l_count);

    return SUCC;
}

/*
 * Prototype    : patch_get_cfg
 * Description  : get patch config command
 */
int32 patch_get_cfg(const char *cfg, int32 type)
{
    uint8 *buf = NULL;
    int32 l_readlen;
    int32 l_ret;

    if (cfg == NULL) {
        PS_PRINT_ERR("cfg file path null!\n");
        return -EFAIL;
    }

    buf = OS_KMALLOC_GFP(READ_CFG_BUF_LEN);
    if (buf == NULL) {
        PS_PRINT_ERR("malloc READ_CFG_BUF fail!\n");
        return -EFAIL;
    }

    /* cfg文件限定在小于2048,如果cfg文件的大小确实大于2048，可以修改READ_CFG_BUF_LEN的值 */
    l_readlen = firmware_read_cfg_etc(cfg, buf);
    if (l_readlen < 0) {
        PS_PRINT_ERR("read cfg error\n");
        OS_MEM_KFREE(buf);
        buf = NULL;
        return -EFAIL;
    }
    /* 减1是为了确保cfg文件的长度不超过READ_CFG_BUF_LEN，因为firmware_read_cfg最多只会读取READ_CFG_BUF_LEN长度的内容 */
    else if (l_readlen > READ_CFG_BUF_LEN - 1) {
        PS_PRINT_ERR("cfg file [%s] larger than %d\n", cfg, READ_CFG_BUF_LEN);
        OS_MEM_KFREE(buf);
        buf = NULL;
        return -EFAIL;
    } else {
        PS_PRINT_INFO("read cfg file [%s] ok, size is [%d]\n", cfg, l_readlen);
    }

    l_ret = patch_parse_cfg(buf, l_readlen, type);
    if (l_ret < 0) {
        PS_PRINT_ERR("parse cfg error\n");
    }

    OS_MEM_KFREE(buf);
    buf = NULL;

    return l_ret;
}

/*
 * Prototype    : patch_download_info
 * Description  : download patch
 */
int32 patch_execute_cmd(int32 cmd_type, uint8 *cmd_name, uint8 *cmd_para, int32 type)
{
    int32 l_ret;

    /* 清空上次操作遗留下来的数据，读取结果时以长度为判断，buf就不用清空了 */
    global.l_Recvbuf1_len = 0;
    global.l_Recvbuf2_len = 0;

    switch (cmd_type) {
        case FILE_TYPE_CMD:
            PS_PRINT_DBG(" command type FILE_TYPE_CMD\n");
            l_ret = patch_file_type(cmd_name, cmd_para, type);
            break;
        case NUM_TYPE_CMD:
            PS_PRINT_DBG(" command type NUM_TYPE_CMD\n");
            l_ret = patch_number_type(cmd_name, cmd_para, type);

            break;
        case QUIT_TYPE_CMD:
            PS_PRINT_DBG(" command type QUIT_TYPE_CMD\n");
            l_ret = patch_quit_type(type);
            break;

        default:
            PS_PRINT_ERR("command type error[%d]\n", cmd_type);
            l_ret = -EFAIL;
            break;
    }

    return l_ret;
}

/*
 * Prototype    : patch_download_patch
 * Description  : download patch
 */
int32 patch_download_patch(int32 type)
{
    int32 l_ret;
    int32 i;
    uint32 ul_alloc_len = READ_DATA_BUF_LEN;

    stringbuf.pbufstart = kmalloc(ul_alloc_len, GFP_KERNEL);
    if (stringbuf.pbufstart == NULL) {
        ul_alloc_len = READ_DATA_REALLOC_BUF_LEN;
        stringbuf.pbufstart = kmalloc(ul_alloc_len, GFP_KERNEL);
        if (stringbuf.pbufstart == NULL) {
            PS_PRINT_ERR("ringbuf KMALLOC SIZE(%d) failed.\n", ul_alloc_len);
            stringbuf.pbufstart = global.auc_Recvbuf1;
            stringbuf.pbufend = RECV_BUF_LEN + stringbuf.pbufstart;

            return -EFAIL;
        }
    }

    PS_PRINT_INFO("ringbuf kmalloc size(%d) suc.\n", ul_alloc_len);
    stringbuf.pbufend = ul_alloc_len + stringbuf.pbufstart;

    stringbuf.phead = stringbuf.pbufstart;
    stringbuf.ptail = stringbuf.pbufstart;

    /* 执行条件:: 命令行没有读完，命令不是错误命令  */
    for (i = 0; i < global.l_count; i++) {
        PS_PRINT_INFO("cmd[%d]type[%d], name[%s], para[%s]\n",
                      i,
                      global.pst_cmd[i].cmd_type,
                      global.pst_cmd[i].cmd_name,
                      global.pst_cmd[i].cmd_para);

        l_ret = patch_execute_cmd(global.pst_cmd[i].cmd_type,
                                  global.pst_cmd[i].cmd_name,
                                  global.pst_cmd[i].cmd_para,
                                  type);
        if (l_ret < 0) {
            kfree(stringbuf.pbufstart);
            stringbuf.pbufstart = NULL;
            stringbuf.pbufend = NULL;
            stringbuf.phead = NULL;
            stringbuf.ptail = NULL;
            return l_ret;
        }
    }

    kfree(stringbuf.pbufstart);
    stringbuf.pbufstart = NULL;
    stringbuf.pbufend = NULL;
    stringbuf.phead = NULL;
    stringbuf.ptail = NULL;

    return SUCC;
}

/*
 * Prototype    : patch_init
 * Description  : patch module initialization
 */
int32 patch_init(int32 type)
{
    int32 l_ret;

    memcpy_s(global.auc_Cfgpath, sizeof(global.auc_Cfgpath),
             UART_CFG_FILE, OS_STR_LEN(UART_CFG_FILE));

    global.pst_wait = OS_KMALLOC_GFP(sizeof(OS_WAIT_QUEUE_HEAD_T_STRU));
    if (global.pst_wait == NULL) {
        PS_PRINT_ERR("malloc wait queue fail, size:%lu\n", sizeof(OS_WAIT_QUEUE_HEAD_T_STRU));
        return -EFAIL;
    }

    OS_INIT_WAITQUEUE_HEAD(global.pst_wait);

    l_ret = patch_get_cfg(global.auc_Cfgpath, type);
    if (l_ret < 0) {
        PS_PRINT_ERR("get [%s] command is fail\n", global.auc_Cfgpath);
        return -EFAIL;
    }

    if (DataBuf_t == NULL) {
        DataBuf_t = OS_KMALLOC_GFP(READ_PATCH_BUF_LEN);
        if (DataBuf_t == NULL) {
            PS_PRINT_ERR("DataBuf_etc KMALLOC failed");
            DataBuf_t = NULL;
            return -EFAIL;
        } else {
            PS_PRINT_DBG("DataBuf_etc KMALLOC succ");
        }
    }

    return SUCC;
}

/*
 * Prototype    : patch_exit
 * Description  : patch module exit
 */
int32 patch_exit(void)
{
    global.l_count = 0;
    if (global.pst_cmd != NULL) {
        OS_MEM_KFREE(global.pst_cmd);
        global.pst_cmd = NULL;
    }

    if (global.pst_wait != NULL) {
        OS_MEM_KFREE(global.pst_wait);
        global.pst_wait = NULL;
    }

    if (DataBuf_t != NULL) {
        OS_MEM_KFREE(DataBuf_t);
        DataBuf_t = NULL;
    }

    return SUCC;
}
