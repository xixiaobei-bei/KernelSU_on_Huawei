#ifndef _FD_LOG_H_
#define _FD_LOG_H_

#define LOG_TAG "HISI_FD"

#define PRINT_DEBUG 0
#define PRINT_INFO  1
#define PRINT_WARN  1
#define PRINT_ERR   1

#if PRINT_DEBUG
#define fd_dbg(fmt, ...) printk(KERN_INFO "[" LOG_TAG "]" "[D] %s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define fd_dbg(fmt, ...) ((void)0)
#endif

#if PRINT_INFO
#define fd_info(fmt, ...) printk(KERN_INFO "[" LOG_TAG "]" "[I] %s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define fd_info(fmt, ...) ((void)0)
#endif

#if PRINT_WARN
#define fd_warn(fmt, ...) printk(KERN_WARNING "[" LOG_TAG "]" "[W] %s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define fd_warn(fmt, ...) ((void)0)
#endif

#if PRINT_ERR
#define fd_err(fmt, ...) printk(KERN_ERR "[" LOG_TAG "]" "[E] %s:" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define fd_err(fmt, ...) ((void)0)
#endif

#endif

