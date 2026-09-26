/*
 * da_combine_irq.h
 *
 * Interrupt controller support for DA_COMBINE
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DA_COMBINE_IRQ_H__
#define __DA_COMBINE_IRQ_H__

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/hisi/da_combine/hi_cdc_ctrl.h>

enum da_combine_irq_type {
	IRQ_BTNUP_COMP1 = 0,
	IRQ_BTNDOWN_COMP1 = 1,
	IRQ_BTNDOWN_COMP2 = 2,
	IRQ_BTNUP_COMP2 = 3,
	IRQ_BTNUP_ECO = 4,
	IRQ_BTNDOWN_ECO = 5,
	IRQ_PLUGIN = 6,
	IRQ_PLUGOUT = 7,
	IRQ_VAD = 12,
	IRQ_WTD = 13,
	IRQ_MAD = 14,
	IRQ_CMD_VALID = 16,
	IRQ_PLL_UNLOCK = 20,
	IRQ_PLL44K1_UNLOCK = 34,
	IRQ_PLLMAD_UNLOCK = 36,
	IRQ_BUNK1_OCP = 40,
	IRQ_BUNK1_SCP = 41,
	IRQ_BUNK2_OCP = 42,
	IRQ_BUNK2_SCP = 43,
	IRQ_CP1_SHORT = 44,
	IRQ_CP2_SHORT = 45,
	IRQ_MAX
};

#define DA_COMBINE_MAX_IRQS_NUM 64
#define DA_COMBINE_IRQ_REG_BITS_NUM 8
#define DA_COMBINE_MAX_IRQ_REGS_NUM (DA_COMBINE_MAX_IRQS_NUM / DA_COMBINE_IRQ_REG_BITS_NUM)

struct da_combine_irq_map {
	/* irq registers in codec */
	unsigned int irq_regs[DA_COMBINE_MAX_IRQ_REGS_NUM];
	/* irq mask register in codec */
	unsigned int irq_mask_regs[DA_COMBINE_MAX_IRQ_REGS_NUM];
	int irq_num;
};

struct da_combine_irq {
	struct device *dev;
};

int da_combine_irq_init_irq(struct da_combine_irq *irq_data,
	const struct da_combine_irq_map *irq_map);
void da_combine_irq_deinit_irq(struct da_combine_irq *irq_data);
int da_combine_irq_request_irq(struct da_combine_irq *irq_data, int phy_irq,
	irq_handler_t handler, const char *name, void *priv);
void da_combine_irq_free_irq(struct da_combine_irq *irq_data, int phy_irq, void *priv);
int da_combine_irq_enable_irq(struct da_combine_irq *irq_data, int phy_irq);
int da_combine_irq_disable_irq(struct da_combine_irq *irq_data, int phy_irq);
int da_combine_irq_enable_irqs(struct da_combine_irq *irq_data,
	int irq_num, const int *phy_irqs);
int da_combine_irq_disable_irqs(struct da_combine_irq *irq_data,
	int irq_num, const int *phy_irqs);
void da_combine_irq_resume_wait(struct da_combine_irq *irq_data);
#endif /* __DA_COMBINE_IRQ_H__ */

