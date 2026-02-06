
#ifndef HWSDP_UTILS_H
#define HWSDP_UTILS_H

#include <linux/kernel.h>

/* debug log tag */
#define HWSDP_DEBUG_TAG "HWSDP"
#define hwsdp_log_err(fmt, args...) pr_err(" HWSDP: " fmt "\n", ## args)
#define hwsdp_log_warn(fmt, args...) pr_warn(" HWSDP: " fmt "\n", ## args)
#define hwsdp_log_warn_once(fmt, args...) pr_warn_once(" HWSDP: " fmt "\n", ## args)
#define hwsdp_log_info(fmt, args...) pr_info(" HWSDP: " fmt "\n", ## args)
#define hwsdp_log_info_once(fmt, args...) pr_info_once(" HWSDP: " fmt "\n", ## args)
#define hwsdp_log_debug(fmt, args...) pr_debug(" HWSDP: " fmt "\n", ## args)

#endif

