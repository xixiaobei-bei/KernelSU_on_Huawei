
#include <linux/kernel.h>
#include <linux/module.h>

#include "bfgx_dev.h"
#include "bfgx_ioctrl.h"
#include "bfgx_bluez.h"
#include "plat_debug.h"
#include "bfgx_data_parse.h"
#include "chr_user.h"
#include "plat_pm.h"
#include "oal_ext_if.h"

STATIC struct bt_data_combination g_bt_data_combination = {0};

struct hbt_recv_pkt {
    uint8_t  type;    /* Packet type */
    uint8_t  hlen;    /* Header length */
    uint8_t  loff;    /* Data length offset in header */
    uint8_t  lsize;   /* Data length field size */
    uint16_t maxlen;  /* Max overall packet length */
    int (*recv)(struct hci_dev *hdev, struct sk_buff *skb);
};

#define MAC_CHR_LEN 12
#define MAC_BYTE_LEN (MAC_CHR_LEN / 2)

STATIC const struct hbt_recv_pkt g_hisi_recv_pkts[] = {
    {
        .type = HCI_ACLDATA_PKT,
        .hlen = HCI_ACL_HDR_SIZE,
        .loff = 2,
        .lsize = 2,
        .maxlen = HCI_MAX_FRAME_SIZE,
         .recv = hci_recv_frame },
    {
        .type = HCI_SCODATA_PKT,
        .hlen = HCI_SCO_HDR_SIZE,
        .loff = 2,
        .lsize = 1,
        .maxlen = HCI_MAX_SCO_SIZE,
        .recv = hci_recv_frame },
    {
        .type = HCI_EVENT_PKT,
        .hlen = HCI_EVENT_HDR_SIZE,
        .loff = 1,
        .lsize = 1,
        .maxlen = HCI_MAX_EVENT_SIZE,
        .recv = hci_recv_frame },
};

STATIC int hisi_recv_buf_update_len(struct hci_dev *hdev, struct sk_buff **skb,
    const struct hbt_recv_pkt *pkts, unsigned int pkts_index)
{
    if ((*skb)->len == (&pkts[pkts_index])->hlen) {
        u16 dlen;

        switch ((&pkts[pkts_index])->lsize) {
            case 0: // size 0
                /* No variable data length */
                dlen = 0;
                break;
            case 1: // size 1
                /* Single octet variable length */
                dlen = (*skb)->data[(&pkts[pkts_index])->loff];
                hci_skb_expect(*skb) += dlen;

                if (skb_tailroom(*skb) < dlen) {
                    kfree_skb(*skb);
                    return -1; /* ERR_PTR(-EMSGSIZE) */
                }
                break;
            case 2: // size 2
                /* Double octet variable length */
                dlen = get_unaligned_le16((*skb)->data +
                                (&pkts[pkts_index])->loff);
                hci_skb_expect(*skb) += dlen;

                if (skb_tailroom(*skb) < dlen) {
                    kfree_skb(*skb);
                    return -1; /* ERR_PTR(-EMSGSIZE) */
                }
                break;
            default:
                /* Unsupported variable length */
                kfree_skb(*skb);
                return -1; /* ERR_PTR(-EILSEQ) */
        }

        if (!dlen) {
            /* No more data, complete frame */
            (&pkts[pkts_index])->recv(hdev, (*skb));
            (*skb) = NULL;
        }
    } else {
        /* Complete frame */
        (&pkts[pkts_index])->recv(hdev, (*skb));
        (*skb) = NULL;
    }

    return 0;
}

STATIC struct sk_buff *hisi_get_bt_head(const unsigned char *buffer, const struct hbt_recv_pkt *pkts,
    int pkts_count)
{
    struct sk_buff *skb = NULL;
    int i;

    for (i = 0; i < pkts_count; i++) {
        if (buffer[0] != (&pkts[i])->type) {
            continue;
        }

        skb = bt_skb_alloc((&pkts[i])->maxlen, GFP_ATOMIC);
        if (!skb) {
            return NULL;
        }

        hci_skb_pkt_type(skb) = (&pkts[i])->type;
        hci_skb_expect(skb) = (&pkts[i])->hlen;
        break;
    }

    return skb;
}

struct sk_buff *hisi_recv_buf(struct hci_dev *hdev, struct sk_buff *skb,
    const unsigned char *buffer, int count)
{
    int err;
    int pkts_count = oal_array_size(g_hisi_recv_pkts);

    while (count) {
        int i, len;

        if (!count) {
            break;
        }

        if (skb == NULL) {
            skb = hisi_get_bt_head(buffer, g_hisi_recv_pkts, pkts_count);
            /* Check for invalid packet type */
            if (skb == NULL) {
                return ERR_PTR(-EILSEQ);
            }

            count -= 1;
            buffer += 1;
        }

        len = min_t(uint, hci_skb_expect(skb) - skb->len, count);
        skb_put_data(skb, buffer, len);

        count -= len;
        buffer += len;

        /* Check for partial packet */
        if (skb->len < hci_skb_expect(skb))
            continue;

        for (i = 0; i < pkts_count; i++) {
            if (hci_skb_pkt_type(skb) == (&g_hisi_recv_pkts[i])->type)
                break;
        }

        if (i >= pkts_count) {
            kfree_skb(skb);
            return ERR_PTR(-EILSEQ);
        }

        err = hisi_recv_buf_update_len(hdev, &skb, g_hisi_recv_pkts, i);
        if (err != 0) {
            return ERR_PTR(-EMSGSIZE);
        }
    }

    return skb;
}

STATIC int hisi_recv(struct hci_dev *hdev, const void *data, int count)
{
    struct sk_buff *rx_skb;
    rx_skb = hisi_recv_buf(hdev, NULL, (const unsigned char *)data, count);
    if (IS_ERR(rx_skb)) {
        int err = PTR_ERR(rx_skb);
        ps_print_info("ERROR rx_skb=%d\n", err);
        return err;
    }

    hdev->stat.byte_rx += count;

    return count;
}

STATIC uint32_t char2byte_bt(const char *strori, char *outbuf)
{
    int i;
    uint8_t temp = 0;
    uint32_t sum = 0;
    uint8_t *ptr_out = (uint8_t *)outbuf;
    const int l_loop_times = MAC_CHR_LEN; /* 单字节遍历是不是正确的mac地址:xx:xx:xx:xx:xx:xx */

    for (i = 0; i < l_loop_times; i++) {
        switch (strori[i]) {
            case '0' ... '9':
                temp = strori[i] - '0';
                break;

            case 'a' ... 'f':
                temp = (strori[i] - 'a') + 10; /* 加10为了保证'a'~'f'分别对应10~15 */
                break;

            case 'A' ... 'F':
                temp = (strori[i] - 'A') + 10; /* 加10为了保证'A'~'F'分别对应10~15 */
                break;
            default:
                break;
        }

        sum += temp;
        /* 为了组成正确的mac地址:xx:xx:xx:xx:xx:xx */
        if (i % 2 == 0) {
            ptr_out[i / 2] |= (temp << 4);
        } else {
            ptr_out[i / 2] |= temp;
        }
    }

    return sum;
}

#define HCI_SET_BD_ADDR 0xFC32
#define CHECK_MAX_CNT    3
#define CHECK_WAIT_TIME  50
STATIC int hci_bt_bdaddr(struct hci_dev *hdev)
{
    struct sk_buff *skb = NULL;
    int ret = INI_FAILED;
    int i;
    unsigned int offset = 0;
    unsigned int buf_len = MAC_CHR_LEN;
    uint8_t puc_buf[MAC_BYTE_LEN] = { 0 };
    bdaddr_t bt_bdaddr = { { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab } };
    char original_mac_addr[MAC_CHR_LEN] = { 0 };
    uint32_t ul_sum;

    /* READ MAC frome Eeprom */
    for (i = 0; i < CHECK_MAX_CNT; i++) {
        if (ini_eeprom_read("MACBT", offset, original_mac_addr, buf_len) == INI_SUCC) {
            ret = INI_SUCC;
            break;
        }
        msleep(CHECK_WAIT_TIME);
    }

    if (ret == 0) {
        oal_io_print("ini_eeprom_read MACBT return success\n");
        ul_sum = char2byte_bt(original_mac_addr, (char *)puc_buf);
        if (ul_sum != 0) {
            ini_warning("hci_bt_bdaddr get MAC from EEPROM: mac= %02x:%s:%s:%s:%02x:%02x \n",
                        puc_buf[0], "**", "**", "**", puc_buf[0x4], puc_buf[0x5]);
            for (i = 0; i < MAC_BYTE_LEN; i++) {
                bt_bdaddr.b[MAC_BYTE_LEN - i - 1] = puc_buf[i]; // 反向拷贝
            }
        }

        /* send macaddr to 1103 */
        for (i = 0; i < CHECK_MAX_CNT; i++) {
            skb = __hci_cmd_sync(hdev, HCI_SET_BD_ADDR, sizeof(bdaddr_t), &bt_bdaddr, HCI_INIT_TIMEOUT);
            if (!IS_ERR(skb)) {
                oal_io_print("retry %d times, MACBT from eeprom send success to device\n", i);
                break;
            }
            msleep(CHECK_WAIT_TIME);
        }

        if (IS_ERR(skb)) {
            int err = PTR_ERR(skb);
            ps_print_err("__hci_cmd_sync changing bt device address failed,%d\n", err);
            return 0;
        }
        kfree_skb(skb);
    } else {
        ps_print_err("ini_eeprom_read read error,use device default bt_bdaddr\n");
        chr_exception_report(CHR_READ_EEPROM_ERROR_EVENTID, CHR_SYSTEM_BT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_EEPROM_READ_INIT);
    }

    return 0;
}

STATIC int hci_bt_init(struct hci_dev *hdev)
{
    int ret;
    PS_PRINT_FUNCTION_NAME;
    set_bit(HCI_RUNNING, &hdev->flags);
    atomic_set(&hdev->cmd_cnt, 1);
    set_bit(HCI_INIT, &hdev->flags);

    ret = hci_bt_bdaddr(hdev);
    return ret;
}

/* Initialize device */
STATIC int hci_bt_open(struct hci_dev *hdev)
{
    int ret;
    struct pm_drv_data *pm_data = (struct pm_drv_data *)hci_get_drvdata(hdev);
    struct pm_top* pm_top_data = pm_get_top();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_print_info("hci_bt_open %p, glb:%p\n", pm_data, pm_get_drvdata(BUART));

    /* Undo clearing this from hci_uart_close() */
    mutex_lock(&(pm_top_data->host_mutex));

    ret = hw_bfgx_open(BFGX_BT);

    mutex_unlock(&(pm_top_data->host_mutex));

    if (ret == 0) {
        ret = hci_bt_init(hdev);
    }

    return ret;
}

/* Close device */
STATIC int hci_bt_close(struct hci_dev *hdev)
{
    int ret;
    struct pm_drv_data *pm_data = (struct pm_drv_data *)hci_get_drvdata(hdev);
    struct pm_top* pm_top_data = pm_get_top();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_print_info("hci_bt_close\n");

    // hci_uart_flush(hdev)
    hdev->flush = NULL;

    /* bt power off */
    mutex_lock(&(pm_top_data->host_mutex));

    ret = hw_bfgx_close(BFGX_BT);

    oal_wake_unlock_force(&pm_data->bt_wake_lock);

    mutex_unlock(&(pm_top_data->host_mutex));
    return ret;
}

STATIC void hci_bt_tx_complete(struct hci_dev *hdev, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
    int pkt_type = hci_skb_pkt_type(skb);
#else
    int pkt_type = bt_cb(skb)->pkt_type;
#endif

    /* Update HCI stat counters */
    switch (pkt_type) {
        case HCI_COMMAND_PKT:
            hdev->stat.cmd_tx++;
            break;

        case HCI_ACLDATA_PKT:
            hdev->stat.acl_tx++;
            break;

        case HCI_SCODATA_PKT:
            hdev->stat.sco_tx++;
            break;

        default:
            /* debug */
            ps_print_info("hci default type=%d\n", pkt_type);
            break;
    }

    ps_print_dbg("hci bt tx type=%d\n", pkt_type);
}

STATIC int hci_bt_send(struct hci_dev *hdev, struct ps_core_s *ps_core_d,
    struct sk_buff *skb, uint32_t count)
{
    uint16_t total_len;
    int32_t ret;

    int32_t headroom_add, headroom, tailroom, tailroom_add;

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        g_bt_data_combination.len = 0;
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    ps_core_d->pm_data->bfg_timer_mod_cnt++;

    headroom = g_bt_data_combination.len + sizeof(struct ps_packet_head) + 1;
    tailroom = sizeof(struct ps_packet_end);

    headroom_add = (headroom > oal_netbuf_headroom(skb) ? (headroom - oal_netbuf_headroom(skb)) : 0);
    tailroom_add = (tailroom > oal_netbuf_tailroom(skb) ? (tailroom - oal_netbuf_headroom(skb)) : 0);

    total_len = count + headroom + tailroom;

    if ((headroom_add > 0) || (tailroom_add > 0)) {
        /* head rom too small */
        ret = oal_netbuf_expand_head(skb, headroom_add, tailroom_add, GFP_ATOMIC);
        if (ret != 0) {
            ps_print_err("relloc skb failed, add=%d\n", headroom_add);
            post_to_visit_node(ps_core_d);
            g_bt_data_combination.len = 0;
            return ret;
        }
    }

    oal_netbuf_push(skb, headroom);
    oal_netbuf_put(skb, tailroom);

    skb->data[sizeof(struct ps_packet_head)] = hci_skb_pkt_type(skb);
    if (g_bt_data_combination.len == BT_TYPE_DATA_LEN) {
        skb->data[sizeof(struct ps_packet_head) + 1] = g_bt_data_combination.type;
    }

    g_bt_data_combination.len = 0;

    hdev->stat.byte_tx += (count + 1);

    hci_bt_tx_complete(hdev, skb);

    ps_add_packet_head(skb->data, BT_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    ps_core_d->bfgx_info[BFGX_BT].tx_pkt_num++;

    post_to_visit_node(ps_core_d);

    return 0;
}

/* Send frames from HCI layer */
STATIC int hci_bt_send_frame(struct hci_dev *hdev, struct sk_buff *skb)
{
    uint32_t count;
    struct ps_core_s *ps_core_d = NULL;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely((ps_core_d == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        g_bt_data_combination.len = 0;
        return -EINVAL;
    }

    count = skb->len;

    if (count > BT_TX_MAX_FRAME) {
        ps_print_err("bt skb len is too large, len=%u\n", skb->len);
        g_bt_data_combination.len = 0;
        return -EINVAL;
    }

    /* 适配O，BT数据分两次下发，先发数据类型，长度固定为1Byte，然后发数据，需要在驱动中组合起来发给device */
    if (count == BT_TYPE_DATA_LEN) {
        g_bt_data_combination.type = skb->data[0];
        g_bt_data_combination.len = count;
        kfree_skb(skb);
        return 0;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        ps_print_err("bt tx high seqlen large than MAXNUM\n");
        g_bt_data_combination.len = 0;
        kfree_skb(skb);
        return 0;
    }

    return hci_bt_send(hdev, ps_core_d, skb, count);
}

/* Reset device */
STATIC int hci_bt_flush(struct hci_dev *hdev)
{
    PS_PRINT_FUNCTION_NAME;

    return 0;
}


STATIC int hci_bt_setup(struct hci_dev *hdev)
{
    ps_print_info("hci_bt_setup\n");
    return 0;
}


STATIC int hci_bt_recv_thread(void *data)
{
    int ret;
    struct hci_dev *hdev = NULL;
    struct sk_buff *skb = NULL;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = (struct pm_drv_data *)data;

    if (pm_data == NULL) {
        return 0;
    }

    allow_signal(SIGTERM);

    for (;;) {
        ps_core_d = ps_get_core_reference(BUART);
        if (unlikely(ps_core_d == NULL)) {
            ps_print_err("ps_core_d is NULL\n");
            break;
        }

        if (oal_unlikely(kthread_should_stop())) {
            ps_print_info("hci_bt_recv_thread stop\n");
            break;
        }

        ret = oal_wait_event_interruptible_m(ps_core_d->bfgx_info[BFGX_BT].rx_wait,
                                             (ps_skb_queue_len(ps_core_d, RX_BT_QUEUE) != 0));
        if (ret != 0) {
            if (ret == -ERESTARTSYS) {
                ps_print_info("hci_bt_recv_thread interrupt\n");
                break;
            }
            ps_print_warning("hci_bt_recv_thread error ret=%d\n", ret);
        }

        skb = ps_skb_dequeue(ps_core_d, RX_BT_QUEUE);
        if (skb == NULL) {
            ps_print_warning("bt read skb queue is null!\n");
            continue;
        }

        hdev = pm_data->st_bt_dev.hdev;
        if (hdev == NULL) {
            ps_print_warning("hdev is null, drop recv pkt\n");
            kfree_skb(skb);
            continue;
        }

        hdev->stat.byte_rx += skb->len;

        hisi_recv(hdev, skb->data, skb->len);
        kfree_skb(skb);
    }

    return 0;
}

STATIC int bt_register_hci_dev(struct pm_drv_data *pm_data)
{
    int err;
    struct hci_dev *hdev;

    /* Initialize and register HCI device */
    hdev = hci_alloc_dev();
    if (hdev == NULL) {
        ps_print_err("can't allocate Hisi HCI device\n");
        return -ENOMEM;
    }

    hdev->bus = HCI_UART;

    hci_set_drvdata(hdev, pm_data);

    hdev->manufacturer = 16; /* 16 is hci manufacture */

    hdev->open  = hci_bt_open;
    hdev->close = hci_bt_close;
    hdev->flush = hci_bt_flush;
    hdev->send  = hci_bt_send_frame;
    hdev->setup = hci_bt_setup;

    err = hci_register_dev(hdev);
    if (err < 0) {
        ps_print_err("can't register hisi HCI device, err=%d\n", err);
        hci_free_dev(hdev);
        return -ENODEV;
    }

    /* create recv thread */
    pm_data->st_bt_dev.bt_recv_task = oal_thread_create(hci_bt_recv_thread,
                                                        (void *)pm_data,
                                                        NULL,
                                                        "hisi_hci_recv",
                                                        SCHED_FIFO,
                                                        0,
                                                        -1);
    if (IS_ERR(pm_data->st_bt_dev.bt_recv_task)) {
        ps_print_err("create hci_recv thread failed, err=%p\n", pm_data->st_bt_dev.bt_recv_task);
        hci_unregister_dev(hdev);
        hci_free_dev(hdev);
        return -ENOMEM;
    }

    pm_data->st_bt_dev.hdev = hdev;

    return 0;
}

STATIC void bt_unregister_hci_dev(struct pm_drv_data *pm_data)
{
    struct hci_dev *hdev = NULL;

    if (pm_data->st_bt_dev.hdev == NULL) {
        ps_print_info("hdev is null\n");
        return;
    }

    hdev = pm_data->st_bt_dev.hdev;

    if (!IS_ERR(pm_data->st_bt_dev.bt_recv_task)) {
        oal_thread_stop(pm_data->st_bt_dev.bt_recv_task, NULL);
        pm_data->st_bt_dev.bt_recv_task = NULL;
    }

    pm_data->st_bt_dev.hdev = NULL;
    hci_unregister_dev(hdev);

    hci_free_dev(hdev);

    ps_print_info("bt_unregister_hci_dev\n");
}
/*
 * Prototype    : hw_bt_open
 * Description  : functions called from above bt hal,when open bt file
 * input        : "/dev/hwbt"
 * output       : return 0 --> open is ok
 *              : return !0--> open is false
 */
int32_t hw_bt_open(struct inode *inode, struct file *filp)
{
    int32_t ret = 0;

    ps_print_info("hw_bt_open\n");

    return ret;
}

/*
 * Prototype    : hw_bt_read
 * Description  : functions called from above bt hal,read count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual read byte size
 */
ssize_t hw_bt_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16_t count1;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    if ((skb = ps_skb_dequeue(ps_core_d, RX_BT_QUEUE)) == NULL) {
        ps_print_warning("bt read skb queue is null!\n");
        return 0;
    }

    /* read min value from skb->len or count */
    count1 = min_t(size_t, skb->len, count);
    if (copy_to_user(buf, skb->data, count1)) {
        ps_print_err("copy_to_user is err!\n");
        ps_restore_skbqueue(ps_core_d, skb, RX_BT_QUEUE);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, count1);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        ps_restore_skbqueue(ps_core_d, skb, RX_BT_QUEUE);
    }

    return count1;
}

STATIC ssize_t hw_bt_parse_user_data(const char __user *buf, size_t count)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16_t total_len;

    total_len = count + g_bt_data_combination.len + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);
    ps_core_d = ps_get_core_reference(BUART);

    skb = ps_alloc_skb(total_len);
    if (skb == NULL) {
        ps_print_err("ps alloc skb mem fail\n");
        post_to_visit_node(ps_core_d);
        g_bt_data_combination.len = 0;
        return -EFAULT;
    }

    if (copy_from_user(&skb->data[sizeof(struct ps_packet_head) + g_bt_data_combination.len], buf, count)) {
        ps_print_err("copy_from_user from bt is err\n");
        kfree_skb(skb);
        post_to_visit_node(ps_core_d);
        g_bt_data_combination.len = 0;
        return -EFAULT;
    }

    if (g_bt_data_combination.len == BT_TYPE_DATA_LEN) {
        skb->data[sizeof(struct ps_packet_head)] = g_bt_data_combination.type;
    }

    g_bt_data_combination.len = 0;

    ps_add_packet_head(skb->data, BT_MSG, total_len);
    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    return 0;
}
/*
 * Prototype    : hw_bt_write
 * Description  : functions called from above bt hal,write count data to buf
 * input        : file handle, buf, count
 * output       : return size --> actual write byte size
 */
ssize_t hw_bt_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct ps_core_s *ps_core_d = NULL;
    int32_t ret = 0;
    uint8_t __user *puser = (uint8_t __user *)buf;
    uint8_t type = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely((ps_core_d == NULL) || (buf == NULL) || (filp == NULL) || (ps_core_d->pm_data == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        g_bt_data_combination.len = 0;
        return -EINVAL;
    }

    if (count > BT_TX_MAX_FRAME) {
        ps_print_err("bt skb len is too large!\n");
        g_bt_data_combination.len = 0;
        return -EINVAL;
    }

    if (count == BT_TYPE_DATA_LEN) {
        get_user(type, puser);
        g_bt_data_combination.type = type;
        g_bt_data_combination.len = count;
        return count;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        ps_print_err("bt tx high seqlen large than MAXNUM\n");
        g_bt_data_combination.len = 0;
        return 0;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work fail, bring to reset work\n");
        g_bt_data_combination.len = 0;
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    oal_wake_lock_timeout(&ps_core_d->pm_data->bt_wake_lock, DEFAULT_WAKELOCK_TIMEOUT);

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    ps_core_d->pm_data->bfg_timer_mod_cnt++;

    ret = hw_bt_parse_user_data(buf, count);
    if (ret < 0) {
        ps_print_err("hw_bt_write pase data fail.\n");
        return ret;
    }

    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    ps_core_d->bfgx_info[BFGX_BT].tx_pkt_num++;

    post_to_visit_node(ps_core_d);

    return count;
}

/*
 * Prototype    : hw_bt_poll
 * Description  : called by bt func from hal;
 *                check whether or not allow read and write
 */
uint32_t hw_bt_poll(struct file *filp, poll_table *wait)
{
    struct ps_core_s *ps_core_d = NULL;
    uint32_t mask = 0;

    PS_PRINT_FUNCTION_NAME;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely((ps_core_d == NULL) || (filp == NULL))) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    /* push curr wait event to wait queue */
    poll_wait(filp, &ps_core_d->bfgx_info[BFGX_BT].rx_wait, wait);

    if (ps_core_d->bfgx_info[BFGX_BT].rx_queue.qlen) { /* have data to read */
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

/*
 * Prototype    : hw_bt_ioctl
 * Description  : called by bt func from hal; default not use
 */
long hw_bt_ioctl(struct file *file, uint32_t cmd, unsigned long arg)
{
    long ret = 0;

    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);
    struct pm_top* pm_top_data = pm_get_top();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_print_info("cmd is %d,arg is %lu", cmd, arg);

    mutex_lock(&(pm_top_data->host_mutex));

    /* hal下发cmd,控制不实际关闭close bt */
    switch (cmd) {
        case BT_IOCTL_FAKE_CLOSE_CMD:
            if (arg == BT_FAKE_CLOSE) {
                pm_data->bt_fake_close_flag = OAL_TRUE;
            } else {
                pm_data->bt_fake_close_flag = OAL_FALSE;
            }
            break;
        case BT_IOCTL_OPEN:
            ret = hw_bfgx_open(BFGX_BT);
            break;
        case BT_IOCTL_RELEASE:
            ret = hw_bfgx_close(BFGX_BT);
            oal_wake_unlock_force(&pm_data->bt_wake_lock);
            break;
        case BT_IOCTL_HCISETPROTO:
            if (pm_data->st_bt_dev.hdev != NULL) {
                ret = -EEXIST;
                break;
            }
            ret = (long)bt_register_hci_dev(pm_data);
            if (ret != 0) {
                ps_print_err("bt_register_hci_dev failed =%ld\n", ret);
                break;
            }
            break;
        case BT_IOCTL_HCIUNSETPROTO:
            mutex_unlock(&(pm_top_data->host_mutex));
            bt_unregister_hci_dev(pm_data);
            mutex_lock(&(pm_top_data->host_mutex));
            ps_print_info("bt_unregister_hci_dev BT_IOCTL_HCIUNSETPROTO has unregister");
            break;
        default:
            ps_print_info("\n");
    }

    mutex_unlock(&(pm_top_data->host_mutex));

    return ret;
}


/*
 * Prototype    : hw_bt_release
 * Description  : called by bt func from hal when close bt inode
 * input        : "/dev/hwbt"
 * output       : return 0 --> close is ok
 *                return !0--> close is false
 */
int32_t hw_bt_release(struct inode *inode, struct file *filp)
{
    return 0;
}

