/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * Header file for device driverPMIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */
#ifndef _PMIC_MNTN_H_
#define _PMIC_MNTN_H_

#define PMIC_OCP_LDO_NAME        (16)
#define PMIC_EVENT_RESERVED      (240)

typedef  enum {
	PMIC_MNTN_OCP_EVENT = 0,
	PMIC_MNTN_EVENT_MAX,
}PMIC_MNTN_EVENT_TYPE;

typedef struct {
	char ldo_num[PMIC_OCP_LDO_NAME];
	char reserved[PMIC_EVENT_RESERVED];
}PMIC_MNTN_EXCP_INFO;

typedef enum {
	PMIC_HRESET_COLD = 0,
	PMIC_HRESET_HOT,
}PMIC_HREST_TYPE;

#if defined(CONFIG_HISI_PMIC_MNTN) || defined(CONFIG_HISI_PMIC_MNTN_SPMI)
/* Open/Close smpl Function
 * SYS_CTRL1,0x0DC, np_smpl_open_en(bit 0), SMPL function enable bit;
 * enable:true, support smpl
 */
extern int hisi_pmic_mntn_config_smpl(bool enable);

/* config vsys_pwroff_abs_pd
 * 'SYS_CTRL0, 0x0DB,vsys_pwroff_abs_pd_mask(bit 0),
 * when vsys < 2.3v,wether auto poweroff bit
 * enable:true, 1£ºdo not auto poweroff¡
 */
extern int hisi_pmic_mntn_config_vsys_pwroff_abs_pd(bool enable);

extern int hisi_pmic_mntn_vsys_pwroff_abs_pd_state(void);
#else
static inline int hisi_pmic_mntn_config_smpl(bool enable)
{
	return 0;
}
static inline int hisi_pmic_mntn_config_vsys_pwroff_abs_pd(bool enable)
{
	return 0;
}
static inline int hisi_pmic_mntn_vsys_pwroff_abs_pd_state(void)
{
	return 0;
}
#endif

#ifdef CONFIG_HISI_PMIC_MNTN_SPMI
extern int zodiac_pmic_set_cold_reset(unsigned char status);
/* pmic mntn notifier function */
int pmic_mntn_register_notifier(struct notifier_block *nb);
int hisi_pmic_mntn_register_notifier(struct notifier_block *nb);   //for modem & endpoint
int pmic_mntn_unregister_notifier(struct notifier_block *nb);
void pmic_mntn_panic_handler(void);
#else
static inline int zodiac_pmic_set_cold_reset(unsigned char status)
{
	return 0;
}
static inline int pmic_mntn_register_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline int hisi_pmic_mntn_register_notifier(struct notifier_block *nb)    //for modem & endpoint
{
	return 0;
}
static inline int pmic_mntn_unregister_notifier(struct notifier_block *nb)
{
	return 0;
}
static inline void pmic_mntn_panic_handler(void)
{
	return;
}
#endif
#endif /* _PMIC_MNTN_H_ */
