#ifndef DUBAI_LOG_H
#define DUBAI_LOG_H

#include <linux/printk.h>

#define DUBAI_TAG				"DUBAI"
#define dubai_debug(fmt, ...)	pr_debug("[" DUBAI_TAG "][D] %s: " fmt "\n", __func__, ##__VA_ARGS__)
#define dubai_info(fmt, ...)	pr_info("[" DUBAI_TAG "][I] %s: " fmt "\n", __func__, ##__VA_ARGS__)
#define dubai_err(fmt, ...)		pr_err("[" DUBAI_TAG "][E] %s: " fmt "\n", __func__, ##__VA_ARGS__)

#endif // DUBAI_LOG_H
