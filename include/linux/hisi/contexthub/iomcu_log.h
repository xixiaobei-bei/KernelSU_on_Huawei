/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_log.h.
 * Create: 2019/11/05
 */
#ifndef __IOMCU_LOG_H
#define __IOMCU_LOG_H
#include <linux/printk.h>

#define ctxhub_info(fmt, ...) \
	pr_info("[iomcu_log][info]" fmt, ##__VA_ARGS__)

#define ctxhub_dbg(fmt, ...) \
	pr_debug("[iomcu_log][debug]" fmt, ##__VA_ARGS__)

#define ctxhub_err(fmt, ...) \
	pr_err("[iomcu_log][error]" fmt, ##__VA_ARGS__)

#define ctxhub_warn(fmt, ...) \
	pr_warn("[iomcu_log][warn]" fmt, ##__VA_ARGS__)

#endif /* __IOMCU_LOG_H */
