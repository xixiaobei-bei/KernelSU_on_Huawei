

#ifndef __CHR_DEVS_H__
#define __CHR_DEVS_H__

/* 头文件包含 */
#include <linux/debugfs.h>

/* CHR性能配置 */
/* 宏定义 */
#define CHR_DEV_KMSG_PLAT "chrKmsgPlat"

#define CHR_LOG_ENABLE          1
#define CHR_LOG_DISABLE         0
#define CHR_ERRNO_QUEUE_MAX_LEN 20
#define CHR_DEV_FRAME_START     0x7E
#define CHR_DEV_FRAME_END       0x7E
#define CHR_READ_SEMA           1

#ifdef  _PRE_WINDOWS_SUPPORT
#define chr_dbg
#define chr_err
#define chr_warning
#define chr_info
#else
#ifdef CHR_DEBUG
#define chr_dbg(s, args...)                                                 \
    do {                                                                    \
        /*lint -e515*/                                                      \
        /*lint -e516*/                                                      \
        printk(KERN_INFO KBUILD_MODNAME ":D]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                      \
        /*lint +e516*/                                                      \
    } while (0)
#else
#define chr_dbg(s, args...)
#endif
#define chr_err(s, args...)                                               \
    do {                                                                  \
        /*lint -e515*/                                                    \
        /*lint -e516*/                                                    \
        printk(KERN_ERR KBUILD_MODNAME ":E]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                    \
        /*lint +e516*/                                                    \
    } while (0)
#define chr_warning(s, args...)                                               \
    do {                                                                      \
        /*lint -e515*/                                                        \
        /*lint -e516*/                                                        \
        printk(KERN_WARNING KBUILD_MODNAME ":W]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                        \
        /*lint +e516*/                                                        \
    } while (0)
#define chr_info(s, args...)                                                \
    do {                                                                    \
        /*lint -e515*/                                                      \
        /*lint -e516*/                                                      \
        printk(KERN_INFO KBUILD_MODNAME ":I]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                      \
        /*lint +e516*/                                                      \
    } while (0)
#endif

#define CHR_MAGIC          'C'
#define CHR_MAX_NR         2
#define CHR_ERRNO_WRITE_NR 1
#define CHR_ERRNO_WRITE    _IOW(CHR_MAGIC, 1, int32_t)
#define CHR_ERRNO_ASK      _IOW(CHR_MAGIC, 2, int32_t)

#define CHR_ID_MSK 1000000

enum CHR_ID_ENUM {
    CHR_WIFI = 909,
    CHR_BT = 913,
    CHR_GNSS = 910,
    CHR_ENUM
};

/* 枚举类型定义 */
enum return_type {
    CHR_SUCC = 0,
    CHR_EFAIL,
};

/* 结构体定义 */
typedef struct {
    wait_queue_head_t errno_wait;
    struct sk_buff_head errno_queue;
    struct semaphore errno_sem;
} chr_event;
typedef struct {
    uint8_t framehead;
    uint8_t reserved[3]; // resverved 3 Byte
    uint32_t error;
    uint8_t frametail;
} chr_dev_exception_stru;

typedef struct {
    uint32_t error;
    uint16_t errlen;
    uint16_t flag;
} chr_dev_exception_stru_para;

typedef struct stru_callback {
    uint32_t (*chr_get_wifi_info)(uint32_t);
} chr_callback_stru;

typedef struct {
    uint32_t chr_errno;
    uint16_t chr_len;
    uint8_t *chr_ptr;
} chr_host_exception_stru;

#ifdef CONFIG_HI1102_PLAT_HW_CHR
extern int chr_miscdevs_init(void);
extern void chr_miscdevs_exit(void);
#endif

#endif
