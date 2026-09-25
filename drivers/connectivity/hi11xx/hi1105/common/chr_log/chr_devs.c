

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
#ifdef CONFIG_HWCONNECTIVITY
#include "hisi_oneimage.h"
#endif
#include "chr_devs.h"
#include "oneimage.h"
#include "board.h"
#include "oal_schedule.h"
#include "chr_errno.h"
#include "oal_hcc_host_if.h"
#include "frw_ext_if.h"
#include "hal_commom_ops.h"
#include "frw_event_main.h"
#include "hw_bfg_ps.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "securec.h"

/* 函数声明 */
static int32 chr_misc_open(struct inode *fd, struct file *fp);
static ssize_t chr_misc_read(struct file *fp, int8 __user *buff, size_t count, loff_t *loff);
static int64 chr_misc_ioctl(struct file *fp, uint32 cmd, uintptr_t arg);
static int32 chr_misc_release(struct inode *fd, struct file *fp);
void chr_rx_errno_to_dispatch(uint32 errno);
int32 chr_wifi_tx_handler(uint32 errno);
int32 chr_bfg_dev_tx_handler(uint32 ul_errno);
uint32 chr_rx_proc_test(uint32 errno);

/* 全局变量定义 */
static CHR_EVENT chr_event;
/* 本模块debug控制全局变量 */
static int32 log_enable = CHR_LOG_DISABLE;

static const struct file_operations chr_misc_fops = {
    .owner = THIS_MODULE,
    .open = chr_misc_open,
    .read = chr_misc_read,
    .release = chr_misc_release,
    .unlocked_ioctl = chr_misc_ioctl,
};

static struct miscdevice chr_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = CHR_DEV_KMSG_PLAT,
    .fops = &chr_misc_fops,
};

/*
 * 函 数 名  : chr_misc_open
 * 功能描述  : 打开设备节点接口
 */
static int32 chr_misc_open(struct inode *fd, struct file *fp)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s open fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }
    CHR_DBG("chr %s open success\n", chr_misc_dev.name);
    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_misc_read
 * 功能描述  : 读取设备节点接口
 */
static ssize_t chr_misc_read(struct file *fp, int8 __user *buff, size_t count, loff_t *loff)
{
    int32 ret;
    uint32 __user *puser = (uint32 __user *)buff;
    struct sk_buff *skb = NULL;
    uint16 data_len = 0;

    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s read fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }

    if (count < sizeof(CHR_DEV_EXCEPTION_STRU_PARA)) {
        CHR_ERR("The user space buff is too small\n");
        return -CHR_EFAIL;
    }

    if (buff == NULL) {
        CHR_ERR("chr %s read fail, user buff is NULL", chr_misc_dev.name);
        return -EAGAIN;
    }

    skb = skb_dequeue(&chr_event.errno_queue);
    if (skb == NULL) {
        if (fp->f_flags & O_NONBLOCK) {
            CHR_DBG("Thread read chr with NONBLOCK mode\n");
            /* for no data with O_NONBOCK mode return 0 */
            return 0;
        } else {
            if (wait_event_interruptible(chr_event.errno_wait, (skb = skb_dequeue(&chr_event.errno_queue)) != NULL)) {
                if (skb != NULL) {
                    skb_queue_head(&chr_event.errno_queue, skb);
                }
                CHR_DBG("Thread interrupt with signel\n");
                return -ERESTARTSYS;
            }
        }
    }

    data_len = min_t(size_t, skb->len, count);
    ret = copy_to_user(puser, skb->data, data_len);
    if (ret) {
        CHR_WARNING("copy_to_user err!restore it, len=%d\n", data_len);
        skb_queue_head(&chr_event.errno_queue, skb);
        return -EFAULT;
    }

    /* have read count1 byte */
    skb_pull(skb, data_len);

    /* if skb->len = 0: read is over */
    if (skb->len == 0) { /* curr skb data have read to user */
        kfree_skb(skb);
    } else { /* if don,t read over; restore to skb queue */
        skb_queue_head(&chr_event.errno_queue, skb);
    }

    return data_len;
}

/* 不作限制的chr上报事件 */
uint32 unlimit_errno[] = {
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
static int32 chr_report_frequency_limit(uint32 ul_errno)
{
#define CHR_REPORT_LIMIT_TIME 5 /* second */
    static unsigned long long chr_report_last_time = 0;
    static uint32 ul_old_errno = 0;
    unsigned long long chr_report_current_time;
    unsigned long long chr_report_interval_time;
    struct timespec64 chr_report_time;
    uint32 index = 0;
    uint32 len = sizeof(unlimit_errno) / sizeof(uint32);

    /* 跳过不限制的chr no */
    for (index = 0; index < len; index++) {
        if (ul_errno == unlimit_errno[index]) {
            return CHR_SUCC;
        }
    }

    chr_report_time = current_kernel_time64();
    chr_report_current_time = chr_report_time.tv_sec;
    chr_report_interval_time = chr_report_current_time - chr_report_last_time;

    if ((chr_report_interval_time > CHR_REPORT_LIMIT_TIME) ||
        (chr_report_last_time == 0) || (ul_old_errno != ul_errno)) {
        chr_report_last_time = chr_report_current_time;
        ul_old_errno = ul_errno;
        return CHR_SUCC;
    } else {
        return CHR_EFAIL;
    }
}

/*
 * 函 数 名  : chr_errno_convert_to_bfg_id
 * 功能描述  : 将CHR 错误号，转成bfg id号
 */
static int32 chr_errno_convert_to_bfg_id(uint32 errno)
{
    uint32 chr_id;

    chr_id = errno / CHR_ID_MSK;

    if (chr_id == CHR_BT) {
        return BFGX_BT;
    } else if (chr_id == CHR_GNSS) {
        return BFGX_GNSS;
    } else {
        CHR_ERR("unsupport errno [%d]\n", errno);
        return -EINVAL;
    }
}

/*
 * 函 数 名  : chr_write_errno_to_queue
 * 功能描述  : 将异常码写入队列
 */
static int32 chr_write_errno_to_queue(uint32 ul_errno, uint16 us_flag, uint8 *ptr_data, uint16 ul_len)
{
    struct sk_buff *skb = NULL;
    uint16 sk_len;
    int32 ret = 0;

    if (chr_report_frequency_limit(ul_errno)) {
        CHR_WARNING("chr report limited, dispose errno=%d\n", ul_errno);
        return CHR_SUCC;
    }

    if (skb_queue_len(&chr_event.errno_queue) > CHR_ERRNO_QUEUE_MAX_LEN) {
        CHR_WARNING("chr errno queue is full, dispose errno=%d\n", ul_errno);
        return CHR_SUCC;
    }

    /* for code run in interrupt context */
    sk_len = sizeof(CHR_DEV_EXCEPTION_STRU_PARA) + ul_len;
    skb = alloc_skb(sk_len, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        CHR_ERR("chr errno alloc skbuff failed! len=%d, errno=%d\n", sk_len, ul_errno);
        return -ENOMEM;
    }

    skb_put(skb, sk_len);
    *(uint32 *)skb->data = ul_errno;
    *((uint16 *)(skb->data + 4)) = ul_len;  /* 偏移存放errno的前4个字节 */
    *((uint16 *)(skb->data + 6)) = us_flag; /* 偏移存放errno加长度的前6个字节 */

    if ((ul_len > 0) && (ptr_data != NULL)) {
        ret = memcpy_s(((uint8 *)skb->data + OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA)),
                       sk_len - OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA), ptr_data, ul_len);
        if (ret != EOK) {
            CHR_ERR("memcpy_s error, destlen=%lu, srclen=%d\n ",
                    sk_len - OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA), ul_len);
        }
    }

    skb_queue_tail(&chr_event.errno_queue, skb);
    wake_up_interruptible(&chr_event.errno_wait);

    CHR_WARNING("chr_write_errno_to_queue success errno=%d\n", ul_errno);

    return CHR_SUCC;
}

static int64 chr_misc_errno_write(uint32 __user *puser)
{
    uint32 ret;
    uint8 *pst_mem = NULL;
    CHR_HOST_EXCEPTION_STRU chr_rx_data;

    ret = copy_from_user(&chr_rx_data, puser, OAL_SIZEOF(CHR_HOST_EXCEPTION_STRU));
    if (ret) {
        CHR_ERR("chr %s ioctl fail, get data from user fail", chr_misc_dev.name);
        return -EINVAL;
    }

    if (chr_rx_data.chr_len == 0) {
        chr_write_errno_to_queue(chr_rx_data.chr_errno, CHR_HOST, NULL, 0);
    } else {
        pst_mem = oal_memalloc(chr_rx_data.chr_len);
        if (pst_mem == NULL) {
            CHR_ERR("chr mem alloc failed len %u\n", chr_rx_data.chr_len);
            return -EINVAL;
        }

        if (chr_rx_data.chr_ptr == NULL) {
            CHR_ERR("chr input arg is invalid!\n");
            oal_free(pst_mem);
            return -EINVAL;
        }

        ret = copy_from_user(pst_mem, (void __user *)(chr_rx_data.chr_ptr), chr_rx_data.chr_len);
        if (ret) {
            CHR_ERR("chr %s ioctl fail, get data from user fail", chr_misc_dev.name);
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
static int64 chr_misc_ioctl(struct file *fp, uint32 cmd, uintptr_t arg)
{
    uint32 __user *puser = (uint32 __user *)arg;
    uint32 ret;
    uint32 value = 0;

    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s ioctl fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }

    if (_IOC_TYPE(cmd) != CHR_MAGIC) {
        CHR_ERR("chr %s ioctl fail, the type of cmd is error type is %d\n", chr_misc_dev.name, _IOC_TYPE(cmd));
        return -EINVAL;
    }

    if (_IOC_NR(cmd) > CHR_MAX_NR) {
        CHR_ERR("chr %s ioctl fail, the nr of cmd is error, nr is %d\n", chr_misc_dev.name, _IOC_NR(cmd));
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
                CHR_ERR("chr %s ioctl fail, get data from user fail", chr_misc_dev.name);
                return -EINVAL;
            }

            chr_rx_errno_to_dispatch(value);
            break;
        default:
            CHR_WARNING("chr ioctl not support cmd=0x%x\n", cmd);
            return -EINVAL;
    }

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_misc_release
 * 功能描述  : 释放节点设备接口
 */
static int32 chr_misc_release(struct inode *fd, struct file *fp)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_ERR("chr %s release fail, module is disable\n", chr_misc_dev.name);
        return -EBUSY;
    }
    CHR_DBG("chr %s release success\n", chr_misc_dev.name);
    return CHR_SUCC;
}

int32 __chr_printLog_etc(CHR_LOGPRIORITY prio, CHR_DEV_INDEX dev_index, const int8 *fmt, ...)
{
    return CHR_SUCC;
}
EXPORT_SYMBOL(__chr_printLog_etc);

/*
 * 函 数 名  : __chr_exception_etc
 * 功能描述  : 内核空间抛异常码接口
 */
int32 __chr_exception_etc(uint32 errno)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(errno, CHR_HOST, NULL, 0);
    return CHR_SUCC;
}

int32 __chr_exception_para(uint32 chr_errno, uint8 *chr_ptr, uint16 chr_len)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return -CHR_EFAIL;
    }

    chr_write_errno_to_queue(chr_errno, CHR_HOST, chr_ptr, chr_len);
    return CHR_SUCC;
}

typedef struct {
    char ApErrCode;
    char arpTxErrCode;
    char macHardErrCode;
} dhcpFailChipInfo_STRU;

#define CHR_WIFI_TEST_ERRNO  909050004
#define CHR_BT_TEST_ERRNO    913050006
void chr_test(void)
{
    dhcpFailChipInfo_STRU aa = {0};
    aa.ApErrCode = 'j';
    aa.arpTxErrCode = 'p';
    aa.macHardErrCode = 'p';

    __chr_exception_para(CHR_WIFI_TEST_ERRNO, (oal_uint8 *)&aa, OAL_SIZEOF(dhcpFailChipInfo_STRU));

    chr_rx_errno_to_dispatch(CHR_BT_TEST_ERRNO);
    chr_rx_errno_to_dispatch(CHR_WIFI_TEST_ERRNO);

    OAL_IO_PRINT("{chr_test:: end !}\r\n");
}

uint32 chr_rx_proc_test(uint32 errno)
{
    OAL_IO_PRINT("{chr_rx_proc_test:: errno = %u !}", errno);
    CHR_EXCEPTION_P(errno, NULL, 0);
    return CHR_SUCC;
}

EXPORT_SYMBOL(__chr_exception_etc);
EXPORT_SYMBOL(__chr_exception_para);
/*
 * 函 数 名  : chr_dev_exception_callback_etc
 * 功能描述  : device异常回调接口
 */
void chr_dev_exception_callback_etc(void *buff, uint16 len)
{
    CHR_DEV_EXCEPTION_STRU_PARA *chr_dev_exception_p = NULL;
    CHR_DEV_EXCEPTION_STRU *chr_dev_exception = NULL;
    oal_uint32 chr_len = 0;
    oal_uint8 *chr_data = NULL;

    if (log_enable != CHR_LOG_ENABLE) {
        CHR_DBG("chr throw exception fail, module is disable\n");
        return;
    }

    if (buff == NULL) {
        CHR_WARNING("chr recv device errno fail, buff is NULL\n");
        return;
    }

    chr_dev_exception = (CHR_DEV_EXCEPTION_STRU *)buff;

    /* mode select */
    if ((chr_dev_exception->framehead == CHR_DEV_FRAME_START) && (chr_dev_exception->frametail == CHR_DEV_FRAME_END)) {
        /* old interface: chr upload has only errno */
        chr_len = sizeof(CHR_DEV_EXCEPTION_STRU);

        if (len != chr_len) {
            CHR_WARNING("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32)len, chr_len);
            return;
        }

        chr_write_errno_to_queue(chr_dev_exception->error, CHR_DEVICE, NULL, 0);
    } else {
        /* new interface:chr upload eigher has data or not */
        chr_dev_exception_p = (CHR_DEV_EXCEPTION_STRU_PARA *)buff;
        chr_len = sizeof(CHR_DEV_EXCEPTION_STRU_PARA) + chr_dev_exception_p->errlen;

        if (len != chr_len) {
            CHR_WARNING("chr recv device errno fail, len %d is unavailable,chr_len %d\n", (int32)len, chr_len);
            return;
        }

        if (chr_dev_exception_p->errlen == 0) {
            chr_write_errno_to_queue(chr_dev_exception_p->errno, chr_dev_exception_p->flag, NULL, 0);
        } else {
            chr_data = (oal_uint8 *)buff + OAL_SIZEOF(CHR_DEV_EXCEPTION_STRU_PARA);
            chr_write_errno_to_queue(chr_dev_exception_p->errno, chr_dev_exception_p->flag,
                                     chr_data, chr_dev_exception_p->errlen);
        }
    }
}
EXPORT_SYMBOL(chr_dev_exception_callback_etc);

chr_callback_stru gst_chr_get_wifi_info_callback;

void chr_host_callback_register(chr_get_wifi_info pfunc)
{
    if (pfunc == NULL) {
        CHR_ERR("chr_host_callback_register::pfunc is null !");
        return;
    }

    gst_chr_get_wifi_info_callback.chr_get_wifi_info = pfunc;

    return;
}

void chr_host_callback_unregister(void)
{
    gst_chr_get_wifi_info_callback.chr_get_wifi_info = OAL_PTR_NULL;

    return;
}

EXPORT_SYMBOL(chr_host_callback_register);
EXPORT_SYMBOL(chr_host_callback_unregister);

/*
 * 函 数 名  : chr_rx_errno_to_dispatch
 * 功能描述  : 将接收到的errno进行解析并分配
 */
void chr_rx_errno_to_dispatch(uint32 errno)
{
    uint32 chr_num;
    chr_num = errno / CHR_ID_MSK;
    switch (chr_num) {
        case CHR_WIFI:

            if (chr_wifi_tx_handler(errno) != CHR_SUCC) {
                CHR_ERR("wifi tx failed,0x%x", errno);
            }
            break;

        case CHR_BT:
        case CHR_GNSS:

            if (chr_bfg_dev_tx_handler(errno) != CHR_SUCC) {
                CHR_ERR("bt/gnss tx failed,0x%x", errno);
            }
            break;

        default:
            CHR_ERR("rcv error num 0x%x", errno);
    }
}

/*
 * 函 数 名  : chr_wifi_dev_tx_handler
 * 功能描述  : 通过hcc通道将errno下发到wifi device
 */
int32 chr_wifi_dev_tx_handler(uint32 errno)
{
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    oal_netbuf_stru *pst_netbuf = NULL;
    int32 l_ret;

    if (hcc == NULL) {
        OAL_IO_PRINT("chr_wifi_dev_tx_handler::hcc is null\n");
        return -CHR_EFAIL;
    }

    pst_netbuf = hcc_netbuf_alloc(OAL_SIZEOF(uint32));
    if (pst_netbuf == NULL) {
        OAL_IO_PRINT("hwifi alloc skb fail.\n");
        return -CHR_EFAIL;
    }

    l_ret = memcpy_s(oal_netbuf_put(pst_netbuf, OAL_SIZEOF(uint32)), OAL_SIZEOF(uint32), &errno, OAL_SIZEOF(uint32));
    if (l_ret != EOK) {
        OAL_IO_PRINT("chr_wifi errno copy failed\n");
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    hcc_hdr_param_init(&st_hcc_transfer_param,
                       HCC_ACTION_TYPE_CHR,
                       0,
                       0,
                       HCC_FC_NONE,
                       DATA_HI_QUEUE);
    l_ret = hcc_tx_etc(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (l_ret != CHR_SUCC) {
        OAL_IO_PRINT("chr_wifi_dev_tx_handler::hcc tx is fail,ret=%d\n", l_ret);
        oal_netbuf_free(pst_netbuf);
        return -CHR_EFAIL;
    }

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_host_tx_handler
 * 功能描述  : 调用回调接口将errno传给hmac
 */
int32 chr_host_tx_handler(uint32 errno)
{
    if (gst_chr_get_wifi_info_callback.chr_get_wifi_info == OAL_PTR_NULL) {
        OAL_IO_PRINT("{chr_host_tx_handler:: callback is null!}");
        return CHR_EFAIL;
    }
    if (gst_chr_get_wifi_info_callback.chr_get_wifi_info(errno) != CHR_SUCC) {
        OAL_IO_PRINT("{chr_host_tx_handler:: tx faild, errno = %u !}", errno);
        return CHR_EFAIL;
    }

    return CHR_SUCC;
}

int32 chr_wifi_tx_handler(uint32 errno)
{
    int32 ret1;
    uint32 ret2;

    if (wlan_pm_is_poweron_etc() == OAL_FALSE) {
        CHR_INFO("handle error[%d] fail, wifi device not active\n", errno);
        return -CHR_EFAIL;
    }

    ret1 = chr_host_tx_handler(errno);
    ret2 = chr_wifi_dev_tx_handler(errno);

    if (ret1 != CHR_SUCC || ret2 != CHR_SUCC) {
        CHR_ERR("wifi tx failed,errno[%u],host tx ret1[%u],device tx ret2[%u]", errno, ret1, ret2);
        return -CHR_EFAIL;
    }

    CHR_INFO("tx is succ,errno %u\n", errno);

    return CHR_SUCC;
}

/*
 * 函 数 名  : chr_bfg_dev_tx_handler
 * 功能描述  : 利用uart通道将errno传给bfg
 */
int32 chr_bfg_dev_tx_handler(uint32 ul_errno)
{
    struct ps_core_s *ps_core_d = NULL;
    struct sk_buff *skb = NULL;
    uint16 sk_len;
    int32 ret, bfgx_subsys;

    ps_get_core_reference_etc(&ps_core_d);
    if (unlikely((ps_core_d == NULL) || (ps_core_d->ps_pm == NULL))) {
        CHR_ERR("ps_core_d is NULL\n");
        return CHR_EFAIL;
    }

    bfgx_subsys = chr_errno_convert_to_bfg_id(ul_errno);
    if (bfgx_subsys < 0) {
        return CHR_EFAIL;
    }

    if (ps_bfg_subsys_active_etc(bfgx_subsys) == false) {
        CHR_INFO("handle error[%d] fail, bfgx device not active\n", ul_errno);
        return CHR_EFAIL;
    }

    /* if high queue num > MAX_NUM and don't write */
    if (ps_core_d->tx_high_seq.qlen > TX_HIGH_QUE_MAX_NUM) {
        CHR_ERR("bt tx high seqlen large than MAXNUM\n");
        return CHR_EFAIL;
    }

    ret = prepare_to_visit_node_etc(ps_core_d);
    if (ret != CHR_SUCC) {
        CHR_ERR("prepare work fail, bring to reset work\n");
        plat_exception_handler_etc(SUBSYS_BFGX, THREAD_BT, BFGX_WAKEUP_FAIL);
        return ret;
    }

    /* modify expire time of uart idle timer */
    mod_timer(&ps_core_d->ps_pm->pm_priv_data->bfg_timer, jiffies + (BT_SLEEP_TIME * HZ / 1000));
    ps_core_d->ps_pm->pm_priv_data->bfg_timer_mod_cnt++;

    /* alloc skb buf */
    sk_len = sizeof(uint32) + sizeof(struct ps_packet_head) + sizeof(struct ps_packet_end);
    skb = alloc_skb(sk_len, (oal_in_interrupt() || oal_in_atomic()) ? GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        CHR_ERR("alloc skbuff failed! len=%d, errno=0x%x\n", sk_len, ul_errno);
        post_to_visit_node_etc(ps_core_d);
        return -CHR_EFAIL;
    }

    skb_put(skb, sk_len);

    /* skb data init,reuse the type of mem_dump to prevent the change of rom */
    ps_add_packet_head_etc(skb->data, MEM_DUMP, sk_len);

    /* put errno into skb_data */
    *(uint32 *)(skb->data + sizeof(struct ps_packet_head)) = ul_errno;

    ps_skb_enqueue_etc(ps_core_d, skb, TX_HIGH_QUEUE);
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    post_to_visit_node_etc(ps_core_d);

    CHR_INFO("tx is succ,errno %u\n", ul_errno);

    return CHR_SUCC;
}

int32 chr_miscdevs_init_etc(void)
{
    int32 ret;

    init_waitqueue_head(&chr_event.errno_wait);
    skb_queue_head_init(&chr_event.errno_queue);

    ret = misc_register(&chr_misc_dev);
    if (ret != CHR_SUCC) {
        CHR_ERR("chr module init fail\n");
        return -CHR_EFAIL;
    }
    log_enable = CHR_LOG_ENABLE;
    CHR_INFO("chr module init succ\n");

    return CHR_SUCC;
}

void chr_miscdevs_exit_etc(void)
{
    if (log_enable != CHR_LOG_ENABLE) {
        CHR_INFO("chr module is diabled\n");
        return;
    }

    misc_deregister(&chr_misc_dev);
    log_enable = CHR_LOG_DISABLE;
    CHR_INFO("chr module exit succ\n");
}

MODULE_AUTHOR("Hisilicon platform Driver Group");
MODULE_DESCRIPTION("hi110x chr log driver");
MODULE_LICENSE("GPL");
#endif
