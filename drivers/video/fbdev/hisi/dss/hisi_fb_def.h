/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_FB_DEF_H
#define HISI_FB_DEF_H

#include <linux/delay.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/bug.h>
#include <linux/printk.h>
#include <linux/hisi/hisi_mdfx.h>

#define void_unused(x)    ((void)(x))

#ifdef CONFIG_HISI_FB_V600
#define DISP_PANEL_NUM 2
#else
#define DISP_PANEL_NUM 1
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/* align */
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, al)  ((val) & ~((typeof(val))(al)-1))
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, al)    (((val) + ((al)-1)) & ~((typeof(val))(al)-1))
#endif

#ifndef BIT
#define BIT(x)  (1<<(x))
#endif

#ifndef IS_EVEN
#define IS_EVEN(x)  ((x) % 2 == 0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef FB_ENABLE_BIT
#define FB_ENABLE_BIT(val, i) ((val) | (1 << (i)))
#endif

#ifndef FB_DISABLE_BIT
#define FB_DISABLE_BIT(val, i) ((val) & (~(1 << (i))))
#endif

#ifndef IS_BIT_ENABLE
#define IS_BIT_ENABLE(val, i) (!!((val) & (1 << (i))))
#endif

#define KHZ 1000
#define MHZ (1000 * 1000)

enum {
	WAIT_TYPE_US = 0,
	WAIT_TYPE_MS,
};
struct mask_delay_time {
	uint32_t delay_time_before_fp;
	uint32_t delay_time_after_fp;
};
/*--------------------------------------------------------------------------*/
extern uint32_t g_dpu_fb_msg_level;
extern int64_t g_dpufb_mdfx_id;
/*
 * Message printing priorities:
 * LEVEL 0 KERN_EMERG (highest priority)
 * LEVEL 1 KERN_ALERT
 * LEVEL 2 KERN_CRIT
 * LEVEL 3 KERN_ERR
 * LEVEL 4 KERN_WARNING
 * LEVEL 5 KERN_NOTICE
 * LEVEL 6 KERN_INFO
 * LEVEL 7 KERN_DEBUG (Lowest priority)
 */
static inline void dpufb_debug_create_mdfx_client(void)
{
	if (g_dpufb_mdfx_id == -1) {
		g_dpufb_mdfx_id = mdfx_create_visitor(VISITOR_GRAPHIC_DISPLAY, NULL);
		pr_info("dpufb visitor id=%lld", g_dpufb_mdfx_id);
	}
}

static inline void dpufb_debug_destroy_mdfx_client(void)
{
	if (g_dpufb_mdfx_id != -1) {
		mdfx_destroy_visitor(g_dpufb_mdfx_id);
		g_dpufb_mdfx_id = -1;
	}

	pr_info("dpufb visitor id=%lld", g_dpufb_mdfx_id);
}

#define DPU_FB_EMERG(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_EMERG(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 0) \
			pr_emerg("[dpufb E]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_ALERT(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_ALERT(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 1) \
			pr_alert("[dpufb A]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_CRIT(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_CRIT(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 2) \
			pr_crit("[dpufb C]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_ERR(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_ERR(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 3) \
			pr_err("[dpufb E]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_WARNING(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_WARNING(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 4) \
			pr_warning("[dpufb W]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_NOTICE(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_NOTICE(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 5) \
			pr_info("[dpufb N]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_INFO(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_INFO(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 6) \
			pr_info("[dpufb I]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_DEBUG(msg, ...) \
	do { \
		dpufb_debug_create_mdfx_client(); \
		MDFX_DEBUG(g_dpufb_mdfx_id, "[dpufb]"msg, ## __VA_ARGS__); \
		if (g_dpu_fb_msg_level > 7) \
			pr_info("[dpufb D]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define DPU_FB_ISR_INFO(msg, ...) \
	do { \
		if (g_dpu_fb_msg_level > 6) \
			pr_info("[dpufb I]:%s: "msg, __func__, ## __VA_ARGS__); \
	} while (0)

#define dpu_check_and_return(condition, ret, level, msg, ...) \
	do { \
		if (condition) { \
			DPU_FB_##level(msg, ##__VA_ARGS__);\
			return ret; \
		} \
	} while (0)

#define dpu_check_and_no_retval(condition, level, msg, ...) \
	do { \
		if (condition) { \
			DPU_FB_##level(msg, ##__VA_ARGS__);\
			return; \
		} \
	} while (0)

#define dpu_err_check(condition, level, msg, ...) \
	do { \
		if (condition) { \
			DPU_FB_##level(msg, ##__VA_ARGS__);\
		} \
	} while (0)

#define dpu_check_and_call(fun_ptr, ...) \
	do { \
		if ((fun_ptr) != NULL) { \
			fun_ptr(__VA_ARGS__); \
		} \
	} while (0)

#define dpu_ptr_check_and_return(ptr, ret, msg, ...) \
	do { \
		if (IS_ERR(ptr)) { \
			ret = PTR_ERR(ptr); \
			DPU_FB_ERR(msg, ##__VA_ARGS__); \
			return ret; \
		} \
	} while (0)

#define dev_check_and_return(dev, condition, ret, level, msg, ...) \
	do { \
		if (condition) { \
			dev_##level(dev, msg, ##__VA_ARGS__); \
			return ret; \
		} \
	} while (0)

#if defined(CONFIG_HISI_FB_V600)
#define DPU_FB_INFO_INTERIM(msg, ...) DPU_FB_INFO(msg, ## __VA_ARGS__)
#else
#define DPU_FB_INFO_INTERIM(msg, ...)
#endif

#define assert(expr) \
	if (!(expr)) { \
		pr_err("[dpufb]: assertion failed! %s,%s,%s,line=%d\n",\
			#expr, __FILE__, __func__, __LINE__); \
	}

#define DPU_FB_ASSERT(x)   assert(x)

#define dpu_log_enable_if(cond, msg, ...) \
	do { \
		if (cond) { \
			DPU_FB_INFO(msg, ## __VA_ARGS__); \
		} else { \
			DPU_FB_DEBUG(msg, ## __VA_ARGS__); \
		} \
	} while (0)


/*--------------------------------------------------------------------------*/

#ifdef CONFIG_HISI_FB_DUMP_DSS_REG
#define outp32(addr, val) \
	do {\
		writel(val, addr);\
		pr_info("writel(0x%x, 0x%x);\n", val, addr);\
	} while (0)
#else
#define outp32(addr, val) writel(val, addr)
#endif

#define outp16(addr, val) writew(val, addr)
#define outp8(addr, val) writeb(val, addr)
#define outp(addr, val) outp32(addr, val)

#define inp32(addr) readl(addr)
#define inp16(addr) readw(addr)
#define inp8(addr) readb(addr)
#define inp(addr) inp32(addr)

#define inpw(port) readw(port)
#define outpw(port, val) writew(val, port)
#define inpdw(port) readl(port)
#define outpdw(port, val) writel(val, port)


#endif /* HISI_FB_DEF_H */
