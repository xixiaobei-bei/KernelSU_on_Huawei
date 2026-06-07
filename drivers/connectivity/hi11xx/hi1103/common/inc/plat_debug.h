

#ifndef __PLAT_DEBUG_H__
#define __PLAT_DEBUG_H__

/* 其他头文件包含 */
#include "bfgx_user_ctrl.h"
#include "chr_user.h"

/* 宏定义 */
#define PS_PRINT_FUNCTION_NAME                                    \
    do {                                                          \
        if (plat_loglevel_etc >= PLAT_LOG_DEBUG) {              \
            printk(KERN_INFO KBUILD_MODNAME ":D]%s]", __func__); \
        }                                                         \
    } while (0)

#define PS_PRINT_DBG(s, args...)                                            \
    do {                                                                    \
        if (plat_loglevel_etc >= PLAT_LOG_DEBUG) {                        \
            printk(KERN_INFO KBUILD_MODNAME ":D]%s]" s, __func__, ##args); \
        }                                                                   \
    } while (0)

#define PS_PRINT_INFO(s, args...)                                           \
    do {                                                                    \
        if (plat_loglevel_etc >= PLAT_LOG_INFO) {                         \
            printk(KERN_INFO KBUILD_MODNAME ":I]%s]" s, __func__, ##args); \
            CHR_LOG(CHR_LOG_INFO, CHR_LOG_TAG_PLAT, s, ##args);             \
        }                                                                   \
    } while (0)

#define PS_PRINT_SUC(s, args...)                                            \
    do {                                                                    \
        if (plat_loglevel_etc >= PLAT_LOG_INFO) {                         \
            printk(KERN_INFO KBUILD_MODNAME ":S]%s]" s, __func__, ##args); \
            CHR_LOG(CHR_LOG_INFO, CHR_LOG_TAG_PLAT, s, ##args);             \
        }                                                                   \
    } while (0)

#define PS_PRINT_WARNING(s, args...)                                          \
    do {                                                                      \
        if (plat_loglevel_etc >= PLAT_LOG_WARNING) {                        \
            printk(KERN_WARNING KBUILD_MODNAME ":W]%s]" s, __func__, ##args); \
            CHR_LOG(CHR_LOG_WARN, CHR_LOG_TAG_PLAT, s, ##args);               \
        }                                                                     \
    } while (0)

#define PS_PRINT_ERR(s, args...)                                          \
    do {                                                                  \
        if (plat_loglevel_etc >= PLAT_LOG_ERR) {                        \
            printk(KERN_ERR KBUILD_MODNAME ":E]%s]" s, __func__, ##args); \
            CHR_LOG(CHR_LOG_ERROR, CHR_LOG_TAG_PLAT, s, ##args);          \
        }                                                                 \
    } while (0)

#define PS_PRINT_ALERT(s, args...)                                              \
    do {                                                                        \
        if (plat_loglevel_etc >= PLAT_LOG_ALERT) {                            \
            printk(KERN_ALERT KBUILD_MODNAME ":ALERT]%s]" s, __func__, ##args); \
            CHR_LOG(CHR_LOG_ERROR, CHR_LOG_TAG_PLAT, s, ##args);                \
        }                                                                       \
    } while (0)

#define PS_BUG_ON(s)                                  \
    do {                                              \
        if ((bug_on_enable_etc == BUG_ON_ENABLE)) { \
            BUG_ON(s);                                \
        }                                             \
    } while (0)

#endif /* PLAT_DEBUG_H */
