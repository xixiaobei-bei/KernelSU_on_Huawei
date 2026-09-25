

#ifndef __CHR_DEVS_H__
#define __CHR_DEVS_H__

/* 头文件包含 */
#include <linux/debugfs.h>
#include "plat_type.h"

/* CHR性能配置 */
/* 宏定义 */
#define CHR_DEV_KMSG_PLAT "chrKmsgPlat"

#define CHR_LOG_ENABLE          1
#define CHR_LOG_DISABLE         0
#define CHR_ERRNO_QUEUE_MAX_LEN 20
#define CHR_DEV_FRAME_START     0x7E
#define CHR_DEV_FRAME_END       0x7E
#define CHR_READ_SEMA           1

#ifdef CHR_DEBUG
#define CHR_DBG(s, args...)                                                 \
    do {                                                                    \
        /*lint -e515*/                                                      \
        /*lint -e516*/                                                      \
        printk(KERN_INFO KBUILD_MODNAME ":D]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                      \
        /*lint +e516*/                                                      \
    } while (0)
#else
#define CHR_DBG(s, args...)
#endif
#define CHR_ERR(s, args...)                                               \
    do {                                                                  \
        /*lint -e515*/                                                    \
        /*lint -e516*/                                                    \
        printk(KERN_ERR KBUILD_MODNAME ":E]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                    \
        /*lint +e516*/                                                    \
    } while (0)
#define CHR_WARNING(s, args...)                                               \
    do {                                                                      \
        /*lint -e515*/                                                        \
        /*lint -e516*/                                                        \
        printk(KERN_WARNING KBUILD_MODNAME ":W]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                        \
        /*lint +e516*/                                                        \
    } while (0)
#define CHR_INFO(s, args...)                                                \
    do {                                                                    \
        /*lint -e515*/                                                      \
        /*lint -e516*/                                                      \
        printk(KERN_INFO KBUILD_MODNAME ":I]chr %s]" s, __func__, ##args); \
        /*lint +e515*/                                                      \
        /*lint +e516*/                                                      \
    } while (0)

#define CHR_MAGIC          'C'
#define CHR_MAX_NR         2
#define chr_ERRNO_WRITE_NR 1
#define CHR_ERRNO_WRITE    _IOW(CHR_MAGIC, 1, int32)
#define CHR_ERRNO_ASK      _IOW(CHR_MAGIC, 2, int32)

#define CHR_ID_MSK 1000000
#define CHR_HOST   1
#define CHR_DEVICE 0

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
} CHR_EVENT;
typedef struct {
    uint8 framehead;
    uint8 reserved[3];
    uint32 error;
    uint8 frametail;
} CHR_DEV_EXCEPTION_STRU;

typedef struct {
    uint32 errno;
    uint16 errlen;
    uint16 flag : 1;
    uint16 resv : 15;

} CHR_DEV_EXCEPTION_STRU_PARA;

typedef struct stru_callback {
    uint32 (*chr_get_wifi_info)(uint32);
} chr_callback_stru;

typedef struct {
    uint32 chr_errno;
    uint16 chr_len;
    uint8 *chr_ptr;
} CHR_HOST_EXCEPTION_STRU;

#ifdef CONFIG_HI1102_PLAT_HW_CHR
extern int chr_miscdevs_init_etc(void);
extern void chr_miscdevs_exit_etc(void);
#endif

#endif
