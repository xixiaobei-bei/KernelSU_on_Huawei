

#ifdef CONFIG_HI1102_PLAT_HW_CHR
/* 头文件包含 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <asm/atomic.h>
#include <stdarg.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/un.h>
#include <linux/skbuff.h>
#include "chr_devs.h"
#include "board.h"
#include "bfgx_dev.h"
#include "oal_schedule.h"
#include "chr_errno.h"
#include "chr_user.h"
#include "oal_hcc_host_if.h"
#include "frw_ext_if.h"
#include "frw_event_main.h"
#include "hw_bfg_ps.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "bfgx_data_parse.h"
#include "securec.h"


/* 函数声明 */
static int32_t chr_misc_open(struct inode *fd, struct file *fp);
static ssize_t chr_misc_read(struct file *fp, char __user *buff, size_t count, loff_t *loff);
static long chr_misc_ioctl(struct file *fp, uint32_t cmd, uintptr_t arg);
static int32_t chr_misc_release(struct inode *fd, struct file *fp);
static void chr_rx_errno_to_dispatch(uint32_t errno);
static int32_t chr_wifi_tx_handler(uint32_t errno);
static int32_t chr_bfg_dev_tx_handler(uint32_t errno);

/* 全局变量定义 */
static chr_event g_chr_event;
/* 本模块debug控制全局变量 */
static int32_t g_chr_log_enable = CHR_LOG_DISABLE;

static const struct file_operations g_chr_misc_fops = {
    .owner = THIS_MODULE,
    .open = chr_misc_open,
    .read = chr_misc_read,
    .release = chr_misc_release,
    .unlocked_ioctl = chr_misc_ioctl,
};

static struct miscdevice g_chr_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = CHR_DEV_KMSG_PLAT,
    .fops = &g_chr_misc_fops,
};

/*
 * 函 数 名  : chr_misc_open
 * 功能描述  : 打开设备节点接口
 */
static int32_t chr_misc_open(struct inode *fd, struct file *fp)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_err("chr %s open fail, module is disable\n", g_chr_misc_dev.name);
        return -EBUSY;
    }
    chr_dbg("chr %s open success\n", g_chr_misc_dev.name);
    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_misc_read
 * 功能描述  : 读取设备节点接口
 */
static ssize_t chr_misc_read(struct file *fp, char __user *buff, size_t count, loff_t *loff)
{
    int32_t ret;
    uint32_t __user *puser = (uint32_t __user *)buff;
    struct sk_buff *skb = NULL;
    uint16_t data_len = 0;

    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_err("chr %s read fail, module is disable\n", g_chr_misc_dev.name);
        return -EBUSY;
    }

    if (count < sizeof(chr_dev_exception_stru_para)) {
        chr_err("The user space buff is too small\n");
        return -CHR_EFAIL;
    }

    if (buff == NULL) {
        chr_err("chr %s read fail, user buff is NULL", g_chr_misc_dev.name);
        return -EAGAIN;
    }

    skb = skb_dequeue(&g_chr_event.errno_queue);
    if (skb == NULL) {
        if (fp->f_flags & O_NONBLOCK) {
            chr_dbg("Thread read chr with NONBLOCK mode\n");
            /* for no data with O_NONBOCK mode return 0 */
            return 0;
        } else {
            if (wait_event_interruptible(g_chr_event.errno_wait,
                                         (skb = skb_dequeue(&g_chr_event.errno_queue)) != NULL)) {
                if (skb != NULL) {
                    skb_queue_head(&g_chr_event.errno_queue, skb);
                }
                chr_dbg("Thread interrupt with signel\n");
                return -ERESTARTSYS;
            }
        }
    }

    data_len = min_t(size_t, skb->len, count);
    ret = copy_to_user(puser, skb->data, data_len);
    if (ret) {
        chr_warning("copy_to_user err!restore it, len=%d\n", data_len);
        skb_queue_head(&g_chr_event.errno_queue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, data_len);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&g_chr_event.errno_queue, skb);
    }

    return data_len;
}

/* 不作限制的chr上报事件 */
static uint32_t g_chr_unlimit_errno[] = {
    CHR_WIFI_DISCONNECT_QUERY_EVENTID,
    CHR_WIFI_CONNECT_FAIL_QUERY_EVENTID,
    CHR_WIFI_WEB_FAIL_QUERY_EVENTID,
    CHR_WIFI_WEB_SLOW_QUERY_EVENTID,
    CHR_BT_CHIP_SOFT_ERROR_EVENTID,
    CHR_PLATFORM_EXCEPTION_EVENTID
};

/*
 * 函 数 名  : chr_report_frequency_limit
 * 功能描述  : 限制chr上报频率，两条chr的间隔至少10s
 * 返回值    : 两条chr的间隔超过10s，返回SUCC，否则返回FAILED
 */
static int32_t chr_report_frequency_limit(uint32_t errno)
{
#define CHR_REPORT_LIMIT_TIME 5 /* second */
    static unsigned long long chr_report_last_time = 0;
    static uint32_t old_errno = 0;
    unsigned long long chr_report_current_time;
    unsigned long long chr_report_interval_time;
    uint32_t index;
    uint32_t len = sizeof(g_chr_unlimit_errno) / sizeof(uint32_t);

    /* 跳过不限制的chr no */
    for (index = 0; index < len; index++) {
        if (errno == g_chr_unlimit_errno[index]) {
            return CHR_SUCC;
        }
    }

    chr_report_current_time = oal_current_kernel_time();
    chr_report_interval_time = chr_report_current_time - chr_report_last_time;

    if ((chr_report_interval_time > CHR_REPORT_LIMIT_TIME) ||
        (chr_report_last_time == 0) || (old_errno != errno)) {
        chr_report_last_time = chr_report_current_time;
        old_errno = errno;
        return CHR_SUCC;
    } else {
        return CHR_EFAIL;
    }
}

/*
 * 函 数 名  : chr_errno_convert_to_bfg_id
 * 功能描述  : 将CHR 错误号，转成bfg id号
 */
static int32_t chr_errno_convert_to_bfg_id(uint32_t errno)
{
    uint32_t chr_id;

    chr_id = errno / CHR_ID_MSK;

    if (chr_id == CHR_BT) {
        return BFGX_BT;
    } else if (chr_id == CHR_GNSS) {
        return BFGX_GNSS;
    } else {
        chr_err("unsupport errno [%d]\n", errno);
        return -EINVAL;
    }
}

/*
 * 函 数 名  : chr_write_errno_to_queue
 * 功能描述  : 将异常码写入队列
 */
static int32_t chr_write_errno_to_queue(uint32_t errno, uint16_t us_flag, uint8_t *ptr_data, uint16_t len)
{
    struct sk_buff *skb = NULL;
    uint16_t sk_len;
    int32_t ret;

    if (chr_report_frequency_limit(errno)) {
        chr_warning("chr report limited, dispose errno=%d\n", errno);
        return CHR_SUCC;
    }

    if (skb_queue_len(&g_chr_event.errno_queue) > CHR_ERRNO_QUEUE_MAX_LEN) {
        chr_warning("chr errno queue is full, dispose errno=%d\n", errno);
        return CHR_SUCC;
    }

    // sk_len 最大为0xffff, 长度过大会出现溢出
    if (len > (USHRT_MAX - sizeof(chr_dev_exception_stru_para))) {
        chr_warning("chr errno %x len %d over limit\n", errno, len);
        return -EINVAL;
    }

    /* for code run in interrupt context */
    sk_len = sizeof(chr_dev_exception_stru_para) + len;
    skb = alloc_skb(sk_len, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        chr_err("chr errno alloc skbuff failed! len=%d, errno=%d\n", sk_len, errno);
        return -ENOMEM;
    }

    skb_put(skb, sk_len);
    *(uint32_t *)skb->data = errno;
    *((uint16_t *)(skb->data + 4)) = len;  /* 偏移存放errno的前4个字节 */
    *((uint16_t *)(skb->data + 6)) = us_flag; /* 偏移存放errno加长度的前6个字节 */

    if ((len > 0) && (ptr_data != NULL)) {
        ret = memcpy_s(((uint8_t *)skb->data + sizeof(chr_dev_exception_stru_para)),
                       sk_len - sizeof(chr_dev_exception_stru_para), ptr_data, len);
        if (ret != EOK) {
            chr_err("memcpy_s error, destlen=%lu, srclen=%d\n ",
                    sk_len - sizeof(chr_dev_exception_stru_para), len);
        }
    }

    skb_queue_tail(&g_chr_event.errno_queue, skb);
    oal_wait_queue_wake_up_interrupt(&g_chr_event.errno_wait);

    chr_warning("chr_write_errno_to_queue success errno=%d\n", errno);

    return CHR_SUCC;
}

static long chr_misc_errno_write(uint32_t __user *puser)
{
    uint32_t ret;
    uint8_t *pst_mem = NULL;
    chr_host_exception_stru chr_rx_data;
    const uint16_t chr_msg_max_len = 0x8fff;

    ret = copy_from_user(&chr_rx_data, puser, sizeof(chr_host_exception_stru));
    if (ret) {
        chr_err("chr %s ioctl fail, get data from user fail", g_chr_misc_dev.name);
        return -EINVAL;
    }

    if (chr_rx_data.chr_len > chr_msg_max_len) {
        chr_err("chr msg is too long to write, chr_len = %u", chr_rx_data.chr_len);
        return -EINVAL;
    }

    if (chr_rx_data.chr_len == 0) {
        chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, NULL, 0);
    } else {
        pst_mem = oal_memalloc(chr_rx_data.chr_len);
        if (pst_mem == NULL) {
            chr_err("chr mem alloc failed len %u\n", chr_rx_data.chr_len);
            return -EINVAL;
        }

        if (chr_rx_data.chr_ptr == NULL) {
            chr_err("chr input arg is invalid!\n");
            oal_free(pst_mem);
            return -EINVAL;
        }

        ret = copy_from_user(pst_mem, (void __user *)(chr_rx_data.chr_ptr), chr_rx_data.chr_len);
        if (ret) {
            chr_err("chr %s ioctl fail, get data from user fail", g_chr_misc_dev.name);
            oal_free(pst_mem);
            return -EINVAL;
        }

        chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, pst_mem, chr_rx_data.chr_len);
        oal_free(pst_mem);
    }

    return CHR_SUCC;
}
/*
 * 函 数 名  : chr_misc_ioctl
 * 功能描述  : 控制设备节点接口
 */
static long chr_misc_ioctl(struct file *fp, uint32_t cmd, uintptr_t arg)
{
    uint32_t __user *puser = (uint32_t __user *)arg;
    uint32_t ret;
    uint32_t value = 0;

    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_err("chr %s ioctl fail, module is disable\n", g_chr_misc_dev.name);
        return -EBUSY;
    }

    if (_IOC_TYPE(cmd) != CHR_MAGIC) {
        chr_err("chr %s ioctl fail, the type of cmd is error type is %d\n", g_chr_misc_dev.name, _IOC_TYPE(cmd));
        return -EINVAL;
    }

    if (_IOC_NR(cmd) > CHR_MAX_NR) {
        chr_err("chr %s ioctl fail, the nr of cmd is error, nr is %d\n", g_chr_misc_dev.name, _IOC_NR(cmd));
        return -EINVAL;
    }

    switch (cmd) {
        case CHR_ERRNO_WRITE:
            if (chr_misc_errno_write(puser) < 0) {
                return -EINVAL;
            }
            break;
        case CHR_ERRNO_ASK:
            ret = get_user(value, puser);
            if (ret) {
                chr_err("chr %s ioctl fail, get data from user fail", g_chr_misc_dev.name);
                return -EINVAL;
            }
            chr_rx_errno_to_dispatch(value);
            break;
        default:
            chr_warning("chr ioctl not support cmd=0x%x\n", cmd);
            return -EINVAL;
    }

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_misc_release
 * 功能描述  : 释放节点设备接口
 */
static int32_t chr_misc_release(struct inode *fd, struct file *fp)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_err("chr %s release fail, module is disable\n", g_chr_misc_dev.name);
        return -EBUSY;
    }
    chr_dbg("chr %s release success\n", g_chr_misc_dev.name);
    return CHR_SUCC;
}

int32_t __chr_exception_para(uint32_t chr_errno, uint8_t *chr_ptr, uint16_t chr_len)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_dbg("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(chr_errno, CHR_HOST, chr_ptr, chr_len);
    return CHR_SUCC;
}

EXPORT_SYMBOL(__chr_exception_para);

int32_t __chr_exception_para_q(uint32_t chr_errno, chr_report_flags_enum_uint16 chr_flag,
                               uint8_t *chr_ptr, uint16_t chr_len)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_dbg("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(chr_errno, chr_flag, chr_ptr, chr_len);
    return CHR_SUCC;
}

EXPORT_SYMBOL(__chr_exception_para_q);

/*
 * 函 数 名  : chr_dev_exception_callback
 * 功能描述  : device异常回调接口
 */
void chr_dev_exception_callback(void *buff, uint16_t len)
{
    chr_dev_exception_stru_para *chr_dev_exception_p = NULL;
    chr_dev_exception_stru *chr_dev_exception = NULL;
    uint32_t chr_len;
    uint8_t *chr_data = NULL;

    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_dbg("chr throw exception fail, module is disable\n");
        return;
    }

    if (buff == NULL) {
        chr_warning("chr recv device errno fail, buff is NULL\n");
        return;
    }

    chr_dev_exception = (chr_dev_exception_stru *)buff;

    /* mode select */
    if ((chr_dev_exception->framehead == CHR_DEV_FRAME_START) && (chr_dev_exception->frametail == CHR_DEV_FRAME_END)) {
        /* old interface: chr upload has only errno */
        chr_len = sizeof(chr_dev_exception_stru);

        if (len != chr_len) {
            chr_warning("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32_t)len, chr_len);
            return;
        }

        chr_write_errno_to_queue(chr_dev_exception->error, CHR_DEVICE, NULL, 0);
    } else {
        /* new interface:chr upload eigher has data or not */
        chr_dev_exception_p = (chr_dev_exception_stru_para *)buff;
        chr_len = sizeof(chr_dev_exception_stru_para) + chr_dev_exception_p->errlen;

        if (len != chr_len) {
            chr_warning("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32_t)len, chr_len);
            return;
        }

        if (chr_dev_exception_p->errlen == 0) {
            chr_write_errno_to_queue(chr_dev_exception_p->error, chr_dev_exception_p->flag, NULL, 0);
        } else {
            chr_data = (uint8_t *)buff + sizeof(chr_dev_exception_stru_para);
            chr_write_errno_to_queue(chr_dev_exception_p->error, chr_dev_exception_p->flag,
                                     chr_data, chr_dev_exception_p->errlen);
        }
    }
}
EXPORT_SYMBOL(chr_dev_exception_callback);

static chr_callback_stru g_chr_get_wifi_info_callback;

void chr_host_callback_register(chr_get_wifi_info pfunc)
{
    if (pfunc == NULL) {
        chr_err("chr_host_callback_register::pfunc is null !");
        return;
    }

    g_chr_get_wifi_info_callback.chr_get_wifi_info = pfunc;

    return;
}

void chr_host_callback_unregister(void)
{
    g_chr_get_wifi_info_callback.chr_get_wifi_info = NULL;

    return;
}

EXPORT_SYMBOL(chr_host_callback_register);
EXPORT_SYMBOL(chr_host_callback_unregister);

/*
 * 函 数 名  : chr_rx_errno_to_dispatch
 * 功能描述  : 将接收到的errno进行解析并分配
 */
static void chr_rx_errno_to_dispatch(uint32_t errno)
{
    uint32_t chr_num;
    chr_num = errno / CHR_ID_MSK;
    switch (chr_num) {
        case CHR_WIFI:

            if (chr_wifi_tx_handler(errno) != CHR_SUCC) {
                chr_err("wifi tx failed,0x%x", errno);
            }
            break;
        case CHR_BT:
        case CHR_GNSS:
            if (chr_bfg_dev_tx_handler(errno) != CHR_SUCC) {
                chr_err("bt/gnss tx failed,0x%x", errno);
            }
            break;
        default:
            chr_err("rcv error num 0x%x", errno);
    }
}

/*
 * 函 数 名  : chr_wifi_dev_tx_handler
 * 功能描述  : 通过hcc通道将errno下发到wifi device
 */
static int32_t chr_wifi_dev_tx_handler(uint32_t errno)
{
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    oal_netbuf_stru *pst_netbuf = NULL;
    int32_t l_ret;

    if (hcc == NULL) {
        oal_io_print("chr_wifi_dev_tx_handler::hcc is null\n");
        return -CHR_EFAIL;
    }

    pst_netbuf = hcc_netbuf_alloc(sizeof(uint32_t));
    if (pst_netbuf == NULL) {
        oal_io_print("hwifi alloc skb fail.\n");
        return -CHR_EFAIL;
    }

    l_ret = memcpy_s(oal_netbuf_put(pst_netbuf, sizeof(uint32_t)), sizeof(uint32_t),
                     &errno, sizeof(uint32_t));
    if (l_ret != EOK) {
        oal_io_print("chr_wifi errno copy failed\n");
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_CHR,
                       0,
                       0,
                       HCC_FC_NONE,
                       DATA_HI_QUEUE);
    l_ret = hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (l_ret != CHR_SUCC) {
        oal_io_print("chr_wifi_dev_tx_handler::hcc tx is fail,ret=%d\n", l_ret);
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_host_tx_handler
 * 功能描述  : 调用回调接口将errno传给hmac
 */
static int32_t chr_host_tx_handler(uint32_t errno)
{
    if (g_chr_get_wifi_info_callback.chr_get_wifi_info == NULL) {
        oal_io_print("{chr_host_tx_handler:: callback is null!}");
        return CHR_EFAIL;
    }
    if (g_chr_get_wifi_info_callback.chr_get_wifi_info(errno) != CHR_SUCC) {
        oal_io_print("{chr_host_tx_handler:: tx faild, errno = %u !}", errno);
        return CHR_EFAIL;
    }

    return CHR_SUCC;
}

static int32_t chr_wifi_tx_handler(uint32_t errno)
{
    int32_t ret1;
    uint32_t ret2;

    if (wlan_pm_is_poweron() == OAL_FALSE) {
        chr_info("handle error[%d] fail, wifi device not active\n", errno);
        return -CHR_EFAIL;
    }

    ret1 = chr_host_tx_handler(errno);
    ret2 = chr_wifi_dev_tx_handler(errno);
    if (ret1 != CHR_SUCC || ret2 != CHR_SUCC) {
        chr_err("wifi tx failed,errno[%u],host tx ret1[%u],device tx ret2[%u]", errno, ret1, ret2);
        return -CHR_EFAIL;
    }

    chr_info("tx is succ,errno %u\n", errno);

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_bfg_dev_tx_handler
 * 功能描述  : 利用uart通道将errno传给bfg
 */
static int32_t chr_bfg_dev_tx_handler(uint32_t errno)
{
    struct sk_buff *skb = NULL;
    uint16_t sk_len;
    int32_t ret, bfgx_subsys;

    struct ps_core_s *ps_core_d = ps_get_core_reference(BUART);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->pm_data == NULL))) {
        chr_err("ps_core_d is NULL\n");
        return CHR_EFAIL;
    }

    bfgx_subsys = chr_errno_convert_to_bfg_id(errno);
    if (bfgx_subsys < 0) {
        return CHR_EFAIL;
    }

    if (ps_bfg_subsys_active(bfgx_subsys) == false) {
        chr_info("handle error[%d] fail, bfgx device not active\n", errno);
        return CHR_EFAIL;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        chr_err("bt tx high seqlen large than MAXNUM\n");
        return CHR_EFAIL;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret != CHR_SUCC) {
        chr_err("prepare work fail, bring to reset work\n");
        plat_exception_handler(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
    ps_core_d->pm_data->bfg_timer_mod_cnt++;

    /* alloc skb buf */
    sk_len = sizeof(uint32_t) + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);
    skb = alloc_skb(sk_len, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        chr_err("alloc skbuff failed! len=%d, errno=0x%x\n", sk_len, errno);
        post_to_visit_node(ps_core_d);
        return -CHR_EFAIL;
    }

    skb_put(skb, sk_len);

    /* skb data init,reuse the type of mem_dump to prevent the change of rom */
    ps_add_packet_head(skb->data, MEM_DUMP, sk_len);

    /* put errno into skb_data */
    *(uint32_t *)(skb->data + sizeof(struct ps_packet_head)) = errno;

    ps_skb_enqueue(ps_core_d, skb, TX_HIGH_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    post_to_visit_node(ps_core_d);

    chr_info("tx is succ,errno %u\n", errno);

    return CHR_SUCC;
}

int32_t chr_miscdevs_init(void)
{
    int32_t ret;

    oal_wait_queue_init_head(&g_chr_event.errno_wait);
    skb_queue_head_init(&g_chr_event.errno_queue);

    ret = misc_register(&g_chr_misc_dev);
    if (ret != CHR_SUCC) {
        chr_err("chr module init fail\n");
        return -CHR_EFAIL;
    }
    g_chr_log_enable = CHR_LOG_ENABLE;
    chr_info("chr module init succ\n");

    return CHR_SUCC;
}

void chr_miscdevs_exit(void)
{
    if (g_chr_log_enable != CHR_LOG_ENABLE) {
        chr_info("chr module is diabled\n");
        return;
    }

    misc_deregister(&g_chr_misc_dev);
    g_chr_log_enable = CHR_LOG_DISABLE;
    chr_info("chr module exit succ\n");
}

MODULE_AUTHOR("Hisilicon platform Driver Group");
MODULE_DESCRIPTION("hi110x chr log driver");
MODULE_LICENSE("GPL");
#endif
