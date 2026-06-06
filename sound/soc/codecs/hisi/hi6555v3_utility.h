/*
 * hi6555V3_driver codec driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HI6555V3_UTILITY_H__
#define __HI6555V3_UTILITY_H__

#include <linux/kernel.h>
#include <linux/pm_runtime.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <asm/io.h>

/* HI6555V3 REGISTER RESOURCE NUM (comply with dts) */
enum hi6555c_reg_resource {
	HI6555V3_SOCCODEC = 0,
	HI6555V3_ASPCFG,
	HI6555V3_AOIOC,
	HI6555V3_REG_CNT
};

/* #define HI6555V3_DEBUG */

#ifdef HI6555V3_DEBUG
#define IN_FUNCTION	\
	pr_info(LOG_TAG"[I]:%s:%d: Begin\n", __FUNCTION__, __LINE__);
#define OUT_FUNCTION  \
	pr_info(LOG_TAG"[I]:%s:%d: End.\n", __FUNCTION__, __LINE__);

#define HI6555V3_LOGD(fmt, ...) \
	pr_info(LOG_TAG"[D]:%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define HI6555V3_LOGE(fmt, ...) \
	pr_err(LOG_TAG"[E]:%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else

#define IN_FUNCTION
#define OUT_FUNCTION

#define HI6555V3_LOGD(fmt, ...)
#define HI6555V3_LOGE(fmt, ...) \
	pr_err(LOG_TAG"[E]:%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#endif

#define HI6555V3_LOGI(fmt, ...) \
	pr_info(LOG_TAG"[I]:%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define HI6555V3_LOGW(fmt, ...) \
	pr_warn(LOG_TAG"[W]:%s:%d: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);

/* ALSA dapm control reg bit[31] negative  = no direct dapm */
#define ALSA_REG_MASK                0x7FFFFFFF
#define ASP_CODEC_REG_BASE           0xE8052000
#define ASP_CFG_REG_BASE             0xE804E000
#define PAGE_SoCCODEC                (ASP_CODEC_REG_BASE & ALSA_REG_MASK)
#define PAGE_ASPCFG                  (0xE804E000 & ALSA_REG_MASK)
#define PAGE_AO_IOC                  0xFFF11000
#define PAGE_PMU_CODEC               0x0
#define PAGE_VIRCODEC                0x8000

#define PAGE_TYPE_MASK               0x7FFFF000
#define PAGE_VALUE_MASK              0x00000FFF

#define INVALID_REG_VALUE            0xFFFFFFFF
#define MAX_UINT32                   0xFFFFFFFF
#define MAX_INT32                    0x7FFFFFFF

#define HI6555V3_SoCCODEC_START      0x00
#define HI6555V3_SoCCODEC_END        0x198
#define HI6555V3_PMUCODEC_START      0x00
#define HI6555V3_PMUCODEC_END        0x32F

#define HI6555V3_VIRCODEC_START      0
#define HI6555V3_VIRCODEC_END        (HI6555V3_VIR_CNT - 1)
#define HI6555V3_ASPCFG_START        0x000
#define HI6555V3_ASPCFG_END          0x248
#define HI6555V3_AOIOC_START         0x000
#define HI6555V3_AOIOC_END           0xF9C
#define HI6555V3_IOC_START           0x000
#define HI6555V3_IOC_END             0xA30
#define HI6555V3_PERICRG_START       0x000
#define HI6555V3_PERICRG_END         0xE2C

extern int hi6555v3_base_addr_map(struct platform_device *pdev);
extern void hi6555v3_base_addr_unmap(void);
extern void hi6555v3_clr_reg_bits(unsigned int reg, unsigned int value);
extern unsigned int hi6555v3_reg_read(struct snd_soc_codec *codec, unsigned int reg);
extern int hi6555v3_reg_update(unsigned int reg, unsigned int mask, unsigned int value);
extern int hi6555v3_reg_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int value);
extern void hi6555v3_set_reg_bits(unsigned int reg, unsigned int value);
extern void hi6555v3_reg_write_stub(unsigned int reg, unsigned int value);

#endif /* __HI6555V3_UTILITY_H__ */

