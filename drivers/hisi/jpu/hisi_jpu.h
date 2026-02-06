/*
 * jpeg jpu
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef HISI_JPU_H
#define HISI_JPU_H

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/clk.h>
#include <linux/regulator/driver.h>
#include <linux/platform_device.h>
#include <linux/compat.h>
#include <linux/irqreturn.h>
#include <soc_jpgdec_interface.h>
#ifdef CONFIG_HISI_FB_V600
#include <linux/hisi/hipp_common.h>
#else
#include <linux/hisi/hipp.h>
#endif
#include "hisi_jpu_common.h"

#define jpu_err_if_cond(cond, msg, ...) \
	do { \
		if (cond) { \
			hisi_jpu_err(msg, ##__VA_ARGS__); \
		} \
	} while (0)

#define jpu_check_null_return(in, out) do { \
	if ((in) == NULL) { \
		hisi_jpu_err("check null ptr at line %d\n", __LINE__); \
		return (out); \
	} \
} while (0)

#define JPG_CLK_ENABLE_DELAY 1
#define JPG_MEMORY_REMPAIR_DELAY 400

#define CONFIG_FB_DEBUG_USED

typedef enum {
	HISI_DSS_V400 = 1,
	HISI_DSS_V500,
	HISI_DSS_V501,
	HISI_DSS_V510,
	HISI_DSS_V510_CS,
	HISI_DSS_V600,
	UNSUPPORT_PLATFORM,
} jpeg_dec_platform;

typedef enum {
	JPEG_DECODER_REG = 0,
	JPEG_TOP_REG,
	JPEG_CVDR_REG,
	JPEG_SMMU_REG,
	JPEG_MEDIA1_REG,
	JPEG_PERI_REG,
	JPEG_PMCTRL_REG,
	JPEG_SCTRL_REG,
	JPEG_SECADAPT_REG,
} jpeg_reg_base;

typedef enum {
	JPEG_MERGED_IRQ = 0,
	JPEG_ERR_IRQ = 0,
	JPEG_DONE_IRQ,
	JPEG_OTHER_IRQ,
} jpeg_merged_irq;

struct hisi_jpu_data_type {
	uint32_t index;
	uint32_t ref_cnt;
	uint32_t fpga_flag;

	struct platform_device *pdev;
	uint32_t jpu_major;
	struct class *jpu_class;
	struct device *jpu_dev;

	char __iomem *jpu_cvdr_base;
	char __iomem *jpu_top_base;
	char __iomem *jpu_dec_base;
	char __iomem *jpu_smmu_base;
	char __iomem *media1_crg_base;
	char __iomem *peri_crg_base;
	char __iomem *pmctrl_base;
	char __iomem *sctrl_base;
#if defined(CONFIG_HISI_FB_V600)
	char __iomem *secadapt_base;
#endif

	uint32_t jpu_done_irq;
	uint32_t jpu_err_irq;
	uint32_t jpu_other_irq;
	uint32_t jpu_merged_irq;

	wait_queue_head_t jpu_dec_done_wq;
	uint32_t jpu_dec_done_flag;

	struct regulator *jpu_regulator;
	struct regulator *media1_regulator;

	const char *jpg_func_clk_name;
	struct clk *jpg_func_clk;

	const char *jpg_platform_name;
	jpeg_dec_platform jpu_support_platform;

	struct sg_table *lb_sg_table;
	uint32_t lb_buf_base;

	uint32_t lb_addr; /* line buffer addr */
	bool power_on;
	struct semaphore blank_sem;

	jpu_dec_reg_t jpu_dec_reg_default;
	jpu_dec_reg_t jpu_dec_reg;
	bool jpu_res_initialized;

	struct jpu_data_t jpu_req;

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
	struct hipp_common_s *jpgd_drv;
#endif

#if defined(CONFIG_HISI_FB_V600)
	jpgd_secadapt_prop_t jpgd_secadapt_prop;
#endif
};

extern int g_debug_jpu_dec;
extern int g_debug_jpu_dec_job_timediff;

int hisi_jpu_register(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_unregister(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_on(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_off(struct hisi_jpu_data_type *hisijd);

void hisi_jpu_dec_normal_reset(const struct hisi_jpu_data_type *hisijd);
void hisi_jpu_dec_error_reset(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_dec_done_config(struct hisi_jpu_data_type *hisijd,
	const struct jpu_data_t *jpu_req);
int hisi_jpu_dec_err_config(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_dec_other_config(struct hisi_jpu_data_type *hisijd);

#ifdef JPGDEC_USE_MERGED_IRQ
irqreturn_t hisi_jpu_dec_merged_isr(int irq, void *ptr);
#else
irqreturn_t hisi_jpu_dec_done_isr(int irq, void *ptr);
irqreturn_t hisi_jpu_dec_err_isr(int irq, void *ptr);
irqreturn_t hisi_jpu_dec_other_isr(int irq, void *ptr);
#endif

void hisi_jpu_dec_interrupt_unmask(const struct hisi_jpu_data_type *hisijd);
void hisi_jpu_dec_interrupt_mask(const struct hisi_jpu_data_type *hisijd);
void hisi_jpu_dec_interrupt_clear(const struct hisi_jpu_data_type *hisijd);

int hisijpu_job_exec(struct hisi_jpu_data_type *hisijd,
	const void __user *argp);

#endif /* HISI_JPU_H */
