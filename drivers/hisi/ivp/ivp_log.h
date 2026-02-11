#ifndef _IVP_LOG_H_
#define _IVP_LOG_H_

#define LOG_TAG "HISI_IVP"
#define PRINT_DEBUG 0
#define PRINT_INFO  1
#define PRINT_WARN  1
#define PRINT_ERR   1

#if PRINT_DEBUG
#define ivp_dbg(fmt, ...) \
	pr_info("[" LOG_TAG "][D] %s:" fmt "\n", __func__, ##__VA_ARGS__)
#else
#define ivp_dbg(fmt, ...) ((void)0)
#endif

#if PRINT_INFO
#define ivp_info(fmt, ...) \
	pr_info("[" LOG_TAG "][I] %s:" fmt "\n", __func__, ##__VA_ARGS__)
#else
#define ivp_info(fmt, ...) ((void)0)
#endif

#if PRINT_WARN
#define ivp_warn(fmt, ...) \
	pr_warning("[" LOG_TAG "][W] %s:" fmt "\n", __func__, ##__VA_ARGS__)
#else
#define ivp_warn(fmt, ...) ((void)0)
#endif

#if PRINT_ERR
#define ivp_err(fmt, ...) \
	pr_err("[" LOG_TAG "][E] %s:" fmt "\n", __func__, ##__VA_ARGS__)
#else
#define ivp_err(fmt, ...) ((void)0)
#endif

#endif
