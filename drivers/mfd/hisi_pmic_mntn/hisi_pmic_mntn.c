/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_mntn_spmi.c
 *
 * Device driver for PMU DRIVER
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hisi_pmic_mntn_inner.h"
#include <linux/mfd/hisi_pmic_mntn.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/of_address.h>
#include <linux/version.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include <linux/regmap.h>
#include <securec.h>
#include <pr_log.h>
#define PR_LOG_TAG PMIC_MNTN_TAG

#include <chipset_common/hwpower/common_module/power_dsm.h>

#define PMIC_DSM_MASK_STATE 0xFF

#if defined CONFIG_HISI_HI6555V200_PMU || \
	defined CONFIG_HISI_HI6555V300_PMU || \
	defined CONFIG_HISI_HI6421V700_PMU || \
	defined CONFIG_HISI_HI6421V800_PMU
#define PMIC_DSM_IGNORE_NUM 99
#else
/* BALT and later platform */
#define PMIC_DSM_IGNORE_NUM 999
#endif

#ifdef CONFIG_GCOV_KERNEL
#define STATIC
#else
#define STATIC static
#endif

#define PMU0	0
#define PMU1	1
#define PMU_NUM	2
static struct pmic_mntn_desc *g_pmic_mntn[PMU_NUM];
static void __iomem *g_sysctrl_base;
#ifdef CONFIG_HISI_PMIC_DEBUG
static int g_mntn_notifier_flag;
#endif

static unsigned int pmic_mntn_reg_read(
	struct pmic_mntn_desc *pmic_mntn, int addr)
{
	struct device_node *np = NULL;
	unsigned int val = 0;
	int ret;

	if (pmic_mntn == NULL || pmic_mntn->dev == NULL)
		return 0;

	np = pmic_mntn->dev->of_node;

	if (pmic_mntn->regmap) {
		ret = regmap_read(pmic_mntn->regmap, addr, &val);
		if (ret < 0) {
			pr_err("%s addr 0x%x failed\n", __func__, addr);
			return 0;
		}
		return val;
	}

	if (of_device_is_compatible(np, PMU1_MNTN_COMP_STR))
		return hisi_sub_pmic_reg_read(addr);
	else
		return pmic_read_reg(addr);
}

static void pmic_mntn_reg_write(
	struct pmic_mntn_desc *pmic_mntn, int addr, int val)
{
	struct device_node *np = NULL;
	int ret;

	if (pmic_mntn == NULL || pmic_mntn->dev == NULL)
		return;

	np = pmic_mntn->dev->of_node;

	if (pmic_mntn->regmap) {
		ret = regmap_write(pmic_mntn->regmap, addr, val);
		if (ret < 0)
			pr_err("%s regmap write 0x%x failed\n", __func__, addr);

		return;
	}

	if (of_device_is_compatible(np, PMU1_MNTN_COMP_STR))
		hisi_sub_pmic_reg_write(addr, val);
	else
		pmic_write_reg(addr, val);
}

int zodiac_pmic_set_cold_reset(unsigned char status)
{
	unsigned char reg_value;
	struct pmic_mntn_desc *pmic_mntn = g_pmic_mntn[PMU0];

	if (pmic_mntn == NULL) {
		pr_err("%s:pmic mntn is null\n", __func__);
		return -EPERM;
	}

	reg_value = pmic_mntn_reg_read(
		pmic_mntn, pmic_mntn->otmp_hreset_pwrdown_reg.addr);
	pr_err("%s:Into:HRESET_PWRDOWN_CTRL register val is 0x%x!\n",
		__func__, reg_value);

	if ((status == PMIC_HRESET_COLD) || (status == PMIC_HRESET_HOT))
		pmic_write_reg(pmic_mntn->otmp_hreset_pwrdown_reg.addr,
			status);

	reg_value = pmic_mntn_reg_read(
		pmic_mntn, pmic_mntn->otmp_hreset_pwrdown_reg.addr);
	pr_err("%s:Out:HRESET_PWRDOWN_CTRL register val is 0x%x!\n",
		__func__, reg_value);

	return 0;
}

static BLOCKING_NOTIFIER_HEAD(hisi_pmic_mntn_notifier_list);
int pmic_mntn_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(
		&hisi_pmic_mntn_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(pmic_mntn_register_notifier);

int hisi_pmic_mntn_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(
		&hisi_pmic_mntn_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(hisi_pmic_mntn_register_notifier);

int pmic_mntn_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(
		&hisi_pmic_mntn_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(pmic_mntn_unregister_notifier);

int pmic_mntn_call_notifiers(int val, void *v)
{
	return blocking_notifier_call_chain(
		&hisi_pmic_mntn_notifier_list, (unsigned long)val, v);
}
EXPORT_SYMBOL_GPL(pmic_mntn_call_notifiers);

#ifdef CONFIG_HISI_PMIC_DEBUG
static int pmic_mntn_test_notifier_call(
	struct notifier_block *nb, unsigned long event, void *data)
{
	PMIC_MNTN_EXCP_INFO *ocp_ldo_msg = (PMIC_MNTN_EXCP_INFO *)data;

	if (ocp_ldo_msg == NULL) {
		pr_err("[%s] test pmic mnt ocp ldo msg is NULL!\n", __func__);
		return -EPERM;
	}

	if (event == PMIC_MNTN_OCP_EVENT)
		pr_err("[%s] test pmic mnt %s ocp event!\n",
			__func__, ocp_ldo_msg->ldo_num);
	else
		pr_err("[%s]invalid event %d!\n", __func__, (int)event);
	return 0;
}

static struct notifier_block pmic_mntn_test_nb = {
	.notifier_call = pmic_mntn_test_notifier_call,
	.priority = INT_MIN,
};
#endif

void pmic_mntn_panic_handler(void)
{
#ifdef CONFIG_HISI_BB
	rdr_syserr_process_for_ap(MODID_AP_S_PMU, 0, 0);
#else
	machine_restart("AP_S_PMU");
#endif
}

static struct pmic_mntn_desc *_find_pmic_by_power_search_for(
	const char *power_name, struct pmic_mntn_desc *pmic_mntn)
{
	struct pmic_exch_reg *exch_desc = NULL;
	unsigned int index, bit, reg_num;
	char *bit_name = NULL;
	int len;

	exch_desc = pmic_mntn->ocp_exch_desc;
	reg_num = pmic_mntn->ocp_reg_n;
	for (index = 0; index < reg_num; index++) {
		for (bit = 0; bit < pmic_mntn->data_width; bit++) {
			bit_name = exch_desc[index].event_bit_name[bit];
			len = (strlen(bit_name)) > (strlen(power_name)) ?
				(strlen(bit_name)) : (strlen(power_name));
			if (!strncmp(bit_name, power_name, len))
				return pmic_mntn;
		}
	}
	return NULL;
}

static struct pmic_mntn_desc *_find_pmic_by_power(const char *power_name)
{
	struct pmic_mntn_desc *pmic_mntn = NULL;
	struct pmic_mntn_desc *pmic_mntn_match = NULL;
	int pmu;

	if (power_name == NULL) {
		pr_err("[%s]power_name is null\n", __func__);
		return NULL;
	}

	for (pmu = 0; pmu < PMU_NUM; pmu++) {
		pmic_mntn = g_pmic_mntn[pmu];
		if (pmic_mntn == NULL)
			continue;

		pmic_mntn_match =
			_find_pmic_by_power_search_for(power_name, pmic_mntn);
		if (pmic_mntn_match != NULL)
			return pmic_mntn_match;
	}
	return NULL;
}

int pmic_special_ocp_register(
	char *power_name, pmic_ocp_callback handler)
{
	struct pmic_mntn_desc *pmic_mntn = g_pmic_mntn[PMU0];
	struct special_ocp_attr *head = NULL;
	struct special_ocp_attr *cur = NULL;

	if (power_name == NULL) {
		pr_err("[%s]power_name is null\n", __func__);
		return -EPERM;
	}
	if (!handler) {
		pr_err("[%s]handler is null\n", __func__);
		return -EPERM;
	}

	/* find pmic info */
	pmic_mntn = _find_pmic_by_power((const char *)power_name);
	if (pmic_mntn == NULL) {
		pr_err("[%s]error, power name %s\n", __func__, power_name);
		return -EPERM;
	}

	head = pmic_mntn->ocp_list_head;
	cur = pmic_mntn->ocp_list_tail;
	if (head == NULL) {
		head = kzalloc(sizeof(*head), GFP_KERNEL);
		if (head == NULL)
			return -ENOMEM;

		cur = head;
		pmic_mntn->ocp_list_head = head;
	} else {
		cur->next = kzalloc(sizeof(*cur), GFP_KERNEL);
		if (cur->next == NULL)
			return -ENOMEM;

		cur = cur->next;
	}
	pmic_mntn->ocp_list_tail = cur;

	cur->power_name = power_name;
	cur->handler = handler;

	pr_info("[%s]%s registered\n", __func__, power_name);

	return 0;
}
EXPORT_SYMBOL(pmic_special_ocp_register);

int hisi_pmic_special_ocp_register(
	char *power_name, pmic_ocp_callback handler)
{
	return pmic_special_ocp_register(power_name,handler); //for modem
}
EXPORT_SYMBOL(hisi_pmic_special_ocp_register);

static int pmic_mntn_ocp_special_handler(
	char *power_name, struct pmic_mntn_desc *pmic_mntn)
{
	struct special_ocp_attr *cur = pmic_mntn->ocp_list_head;
	int ret;

	if (cur == NULL) {
		pr_err("[%s]no power registered\n", __func__);
		return PMIC_OCP_NONE;
	}

	do {
		if (!strncmp(cur->power_name, power_name,
			    ((strlen(cur->power_name)) > (strlen(power_name)) ?
					    (strlen(cur->power_name)) :
					    (strlen(power_name))))) {
			pr_err("%s ocp special handler\n", power_name);
			ret = cur->handler(power_name);
			if (ret)
				return PMIC_OCP_NONE;
			else
				return PMIC_OCP_HANDLED;
		}
		cur = cur->next;
	} while (cur != NULL);

	return PMIC_OCP_NONE;
}

static void get_pmu_key_register_info(struct pmic_mntn_desc *pmic_mntn)
{
	unsigned int i;

	for (i = 0; i < pmic_mntn->pmu_record_reg_n; i++)
		pr_err("[%s]addr[0x%x] = value[0x%x]\n", __func__,
			pmic_mntn->pmu_record_regs[i],
			pmic_mntn_reg_read(
				pmic_mntn, pmic_mntn->pmu_record_regs[i]));
}

#ifdef CONFIG_HUAWEI_DSM
static void pmic_mntn_ocp_dsm_client(const char *bit_name, int dmd_offset)
{
	if (!dsm_client_ocuppy(power_dsm_get_dclient(POWER_DSM_PMU_OCP))) {
		pr_err("pmic %s_ocp happened, dmd is %d, please pay attention!\n",
			bit_name, dmd_offset);
		dsm_client_record(power_dsm_get_dclient(POWER_DSM_PMU_OCP),
			"pmic %s_ocp happened, please pay attention!\n",
			bit_name);
		dsm_client_notify(power_dsm_get_dclient(POWER_DSM_PMU_OCP),
			DSM_PMU_OCP_ERROR_NO_BASE + dmd_offset);
	} else {
		pr_err("pmic %s_ocp happened, dmd not record\n", bit_name);
	}
}

static void pmic_mntn_ocp_dsm_report(struct pmic_mntn_desc *pmic_mntn,
	unsigned int index, unsigned int bit, unsigned int reset)
{
	char *bit_name = NULL;
	struct pmic_exch_reg *exch_desc = NULL;
	int offset = pmic_mntn->data_width * index + bit;

	if (reset)
		exch_desc = pmic_mntn->record_exch_desc;
	else
		exch_desc = pmic_mntn->ocp_exch_desc;

	bit_name = exch_desc[index].event_bit_name[bit];
	if ((pmic_mntn->dsm_record_regs_mask[index] & BIT(bit)) &&
		(reset == (!!(pmic_mntn->dsm_ocp_reset_mask[index] &
				  BIT(bit)))) &&
		((unsigned int)offset < pmic_mntn->ocp_error_dmd_offset_n) &&
		(pmic_mntn->ocp_error_dmd_offset[offset] !=
			PMIC_DSM_IGNORE_NUM))
		pmic_mntn_ocp_dsm_client(
			bit_name, pmic_mntn->ocp_error_dmd_offset[offset]);
}

static int pmic_mntn_record_mntn_dsm_data_get(
	struct pmic_mntn_desc *pmic_mntn)
{
	s32 ret;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;

	pmic_mntn->dsm_record_regs_mask = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->record_reg_n, GFP_KERNEL);
	if (pmic_mntn->dsm_record_regs_mask == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,dsm-record-regs-mask",
		pmic_mntn->dsm_record_regs_mask, pmic_mntn->record_reg_n);
	if (ret) {
		dev_err(dev, "[%s]get dsm-record-regs-mask attribute failed\n",
			__func__);
		return -ENODEV;
	}

	pmic_mntn->dsm_ocp_reset_mask = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->record_reg_n, GFP_KERNEL);
	if (pmic_mntn->dsm_ocp_reset_mask == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,dsm-ocp-reset-mask",
		pmic_mntn->dsm_ocp_reset_mask, pmic_mntn->record_reg_n);
	if (ret) {
		dev_err(dev, "[%s]get dsm-ocp-reset-mask attribute failed\n",
			__func__);
		return -ENODEV;
	}
	ret = of_property_read_u32(np, "hisilicon,ocp-error-dmd-offset-number",
		(u32 *)&pmic_mntn->ocp_error_dmd_offset_n);
	if (ret) {
		dev_err(dev, "[%s]get ocp-error-dmd-offset-number attribute failed\n",
			__func__);
		return -ENODEV;
	}
	pmic_mntn->ocp_error_dmd_offset = (u32 *)devm_kzalloc(dev,
		sizeof(u32) * pmic_mntn->ocp_error_dmd_offset_n, GFP_KERNEL);
	if (pmic_mntn->ocp_error_dmd_offset == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,ocp-error-dmd-offset",
		pmic_mntn->ocp_error_dmd_offset,
		pmic_mntn->ocp_error_dmd_offset_n);
	if (ret) {
		dev_err(dev, "[%s]get ocp-error-dmd-offset attribute failed\n",
			__func__);
		return -ENODEV;
	}

	return 0;
}
#endif

static void pmic_mntn_ocp_normal_handler(struct pmic_mntn_desc *pmic_mntn,
	unsigned int care_bit, unsigned int index, unsigned int bit)
{
	char *bit_name = NULL;
	unsigned int bit_reg;
	struct pmic_exch_reg *exch_desc = pmic_mntn->ocp_exch_desc;
	PMIC_MNTN_EXCP_INFO ocp_ldo_msg;
	errno_t err;

	bit_name = exch_desc[index].event_bit_name[bit];
	bit_reg = exch_desc[index].event_ops_reg[bit];

	err = memset_s(
		&ocp_ldo_msg, sizeof(ocp_ldo_msg), 0, sizeof(ocp_ldo_msg));
	if (err != EOK) {
		pr_err("[%s]memset_s fail, err= %d\n", __func__, err);
		return;
	}

	if (exch_desc[index].check_count[bit] <
		exch_desc[index].check_ocp_num[bit]) {
		exch_desc[index].check_count[bit]++;
		/* open ldo */
		if (bit_reg != INVALID_REG_ADDR) {
			pmic_mntn_reg_write(
				pmic_mntn, bit_reg, OPEN_REGULATOR_CONTROL);
			/* clear record event */
			pmic_mntn_reg_write(pmic_mntn,
				pmic_mntn->record_regs[index],
				pmic_mntn->record_event_buff[index]);
		}
	} else if (care_bit & exch_desc[index].check_ocp_nofify) {
		exch_desc[index].check_count[bit] = 0;

		/* notify the terminal exception handling system */
		err = strncpy_s(ocp_ldo_msg.ldo_num, PMIC_OCP_LDO_NAME,
			bit_name, strlen(bit_name));
		if (err != EOK) {
			pr_err("[%s]strncpy_s fail, err= %d\n", __func__, err);
			return;
		}
		pmic_mntn_call_notifiers(
			PMIC_MNTN_OCP_EVENT, (void *)&ocp_ldo_msg);
	} else if (care_bit & exch_desc[index].check_ocp_reset) {
		pmic_mntn->health |= BIT(PMIC_HEALTH_OCP_EXCH_HAPPENED);
	} else {
		pr_err("%s: %s has been opened %u tiems,the time do nothing!\n",
			__func__, bit_name, exch_desc[index].check_count[bit]);
	}
}

static void pmic_mntn_ocp_event_proc(struct pmic_mntn_desc *pmic_mntn,
	unsigned int care_bit, unsigned int index, unsigned int bit)
{
	char *bit_name = NULL;
	char *cur_str = NULL;
	unsigned int bit_reg;
	struct pmic_exch_reg *exch_desc = pmic_mntn->ocp_exch_desc;
	int ret;
#if defined CONFIG_HISI_HI6555V200_PMU
	const int nov_flag = 1;
#else
	const int nov_flag = 0;
#endif
	bit_name = exch_desc[index].event_bit_name[bit];
	bit_reg = exch_desc[index].event_ops_reg[bit];
	cur_str = pmic_mntn->irq_log_show;

	snprintf(cur_str + strlen(cur_str), PMIC_PRINT_BUF_SIZE,
		"PMU EVENT TYPE:ocp_%s\n", bit_name);
	get_pmu_key_register_info(pmic_mntn);
	/* PMU irq event is 0, PMU reset event is 1 */
#ifdef CONFIG_HUAWEI_DSM
	pmic_mntn_ocp_dsm_report(pmic_mntn, index, bit, 0);
#endif
	/* close ldo */
	if (bit_reg != INVALID_REG_ADDR)
		pmic_mntn_reg_write(
			pmic_mntn, bit_reg, CLOSE_REGULATOR_CONTROL);

	/* clear interrupt */
	pmic_mntn_reg_write(
		pmic_mntn, pmic_mntn->ocp_regs[index], care_bit);
	ret = pmic_mntn_ocp_special_handler(bit_name, pmic_mntn);
	if (ret == PMIC_OCP_HANDLED)
		return;

	if (unlikely(strstr(saved_command_line, "androidboot.swtype=factory") ||
		nov_flag == 1)) {
		if (care_bit & exch_desc[index].inacceptable_event)
			pmic_mntn->health |=
				BIT(PMIC_HEALTH_OCP_EXCH_HAPPENED);
	} else {
		pmic_mntn_ocp_normal_handler(pmic_mntn, care_bit, index, bit);
	}
}

static void pmic_mntn_ocp_scan(struct pmic_mntn_desc *pmic_mntn)
{
	unsigned int index, bit;
	int *reg_buff = NULL;
	unsigned int reg_num;
	unsigned int care_bit;

	reg_buff = pmic_mntn->ocp_event_buff;
	reg_num = pmic_mntn->ocp_reg_n;
	for (index = 0; index < reg_num; index++) {
		if (reg_buff[index] == 0)
			continue;
		pr_err("[%s]reg = 0x%x, value = 0x%x\n", __func__,
			pmic_mntn->ocp_regs[index], reg_buff[index]);
		for (bit = 0; bit < pmic_mntn->data_width; bit++) {
			care_bit = ((unsigned int)reg_buff[index]) & BIT(bit);
			if (care_bit)
				pmic_mntn_ocp_event_proc(
					pmic_mntn, care_bit, index, bit);
		}
	}
}

static void pmic_mntn_record_events(struct pmic_mntn_desc *pmic_mntn)
{
	unsigned int index;
#ifdef CONFIG_HUAWEI_DSM
	unsigned int i;
#endif
	int len = PMIC_PRINT_BUF_SIZE;

	snprintf(pmic_mntn->init_log_show, PMIC_PRINT_BUF_SIZE,
		"\nPMIC REGISTER DUMP\n");
	len -= strlen(pmic_mntn->init_log_show);
	snprintf(pmic_mntn->init_log_show + strlen(pmic_mntn->init_log_show),
		len, "  addr     data\n");

	for (index = 0; index < pmic_mntn->record_reg_n; index++) {
		pmic_mntn->record_event_buff[index] = pmic_mntn_reg_read(
			pmic_mntn, pmic_mntn->record_regs[index]);

		pmic_mntn_reg_write(pmic_mntn,
			pmic_mntn->record_regs[index],
			pmic_mntn->record_event_buff[index]);
		len -= strlen(pmic_mntn->init_log_show);
		snprintf(pmic_mntn->init_log_show +
			strlen(pmic_mntn->init_log_show),
			len,
			"  0x%x    0x%x\n", pmic_mntn->record_regs[index],
			pmic_mntn->record_event_buff[index]);
		if ((pmic_mntn->record_event_buff[index] &
			    PMIC_DSM_MASK_STATE) == 0)
			continue;

#ifdef CONFIG_HUAWEI_DSM
		for (i = 0; i < pmic_mntn->data_width; i++) {
			if (pmic_mntn->record_event_buff[index] & BIT(i))
				/* PMU irq event is 0, PMU reset event is 1 */
				pmic_mntn_ocp_dsm_report(
					pmic_mntn, index, i, 1);
		}
#endif
	}

	pr_err("%s", pmic_mntn->init_log_show);
}

static void pmic_mntn_clear_ocp(struct pmic_mntn_desc *pmic_mntn)
{
	unsigned int index;

	for (index = 0; index < pmic_mntn->ocp_reg_n; index++) {
		pmic_mntn->ocp_event_buff[index] = pmic_mntn_reg_read(
			pmic_mntn, pmic_mntn->ocp_regs[index]);
		pmic_mntn_reg_write(pmic_mntn, pmic_mntn->ocp_regs[index],
			pmic_mntn->ocp_event_buff[index]);
	}

	memset((void *)pmic_mntn->ocp_event_buff, 0,
		sizeof(u32) * pmic_mntn->ocp_reg_n);
}

static irqreturn_t pmic_mntn_ocp_irq_handler(
	int irq, void *data)
{
	struct irq_desc *desc = NULL;
	struct pmic_mntn_desc *pmic_mntn = (struct pmic_mntn_desc *)data;

	dev_err(pmic_mntn->dev, "[%s] +\n", __func__);

	desc = irq_to_desc(irq);
	if (desc == NULL) {
		pr_err("[%s]irq_to_desc failed\n", __func__);
		return IRQ_NONE;
	}

	if (desc->irq_data.chip->irq_mask != NULL)
		desc->irq_data.chip->irq_mask(&desc->irq_data);

	queue_work(pmic_mntn->ocp_wq, &pmic_mntn->ocp_wk);

	dev_err(pmic_mntn->dev, "[%s] -\n", __func__);
	return IRQ_HANDLED;
}

static void pmic_mntn_ocp_wq_handler(struct work_struct *work)
{
	unsigned int index;
	struct irq_desc *desc = NULL;
	struct pmic_mntn_desc *pmic_mntn = NULL;

	pmic_mntn = container_of(work, struct pmic_mntn_desc, ocp_wk);

	dev_err(pmic_mntn->dev, "[%s] +\n", __func__);

	memset((void *)pmic_mntn->irq_log_show, '\0', PMIC_PRINT_BUF_SIZE);

	for (index = 0; index < pmic_mntn->ocp_reg_n; index++) {
		pmic_mntn->ocp_event_buff[index] = pmic_mntn_reg_read(
			pmic_mntn, pmic_mntn->ocp_regs[index]);
		pmic_mntn->record_event_buff[index] = pmic_mntn_reg_read(
			pmic_mntn, pmic_mntn->record_regs[index]);
	}

	pmic_mntn_ocp_scan(pmic_mntn);
	dev_err(pmic_mntn->dev, "\n%s\n", pmic_mntn->irq_log_show);

	desc = irq_to_desc(pmic_mntn->ocp_irq);
	if (desc == NULL) {
		pr_err("[%s]irq_to_desc(pmic_mntn->ocp_irq) error\n", __func__);
		return;
	}

	if (desc->irq_data.chip->irq_unmask != NULL)
		desc->irq_data.chip->irq_unmask(&desc->irq_data);

	if (pmic_mntn->health & BIT(PMIC_HEALTH_OCP_EXCH_HAPPENED))
		pmic_mntn_panic_handler();

	dev_err(pmic_mntn->dev, "[%s] -\n", __func__);
}

static void pmic_mntn_board_reboot(void)
{
	unsigned int value;

	pr_err("[%s]start\n", __func__);
	/* DDR bypass */
	writel(0x1 << SOC_SCTRL_SCPEREN1_wait_ddr_selfreflash_done_bypass_START,
		(void __iomem *)SOC_SCTRL_SCPEREN1_ADDR(g_sysctrl_base));
	while (1) {
		writel(0xdeadbeef, SOC_SCTRL_SCSYSSTAT_ADDR(g_sysctrl_base));
		value = readl((char *)SOC_SCTRL_SCPERCLKEN1_ADDR(
				g_sysctrl_base)) &
			(0x1 << SOC_SCTRL_SCPEREN1_wait_ddr_selfreflash_done_bypass_START);
		pr_err("[%s], 0xdeadbeef write fail , value = 0x%x\n", __func__,
			value);
	}
}

static irqreturn_t pmic_mntn_otmp_irq_handler(
	int irq, void *data)
{
	struct irq_desc *desc = NULL;
	struct pmic_mntn_desc *pmic_mntn = (struct pmic_mntn_desc *)data;

	desc = irq_to_desc(irq);
	if (desc == NULL) {
		pr_err("[%s]irq_to_desc failed\n", __func__);
		return IRQ_NONE;
	}

	if (desc->irq_data.chip->irq_mask != NULL)
		desc->irq_data.chip->irq_mask(&desc->irq_data);

	queue_work(pmic_mntn->otmp_wq, &pmic_mntn->otmp_wk);
	return IRQ_HANDLED;
}

static void pmic_mntn_otmp_wk_handler(struct work_struct *work)
{
	struct irq_desc *desc = NULL;
	struct pmic_mntn_desc *pmic_mntn = NULL;
	unsigned char reg_value;

	pmic_mntn = container_of(work, struct pmic_mntn_desc, otmp_wk);

	memset((void *)pmic_mntn->irq_log_show, '\0', PMIC_PRINT_BUF_SIZE);
	snprintf(pmic_mntn->irq_log_show, PMIC_PRINT_BUF_SIZE,
		"%s", "PMIC OTMP EVENT HAPPEN.\n");
	pr_info("\n%s\n", pmic_mntn->irq_log_show);

	desc = irq_to_desc(pmic_mntn->otmp_irq);
	if (desc == NULL) {
		pr_err("[%s]irq_to_desc err\n", __func__);
		return;
	}

	if (desc->irq_data.chip->irq_unmask != NULL)
		desc->irq_data.chip->irq_unmask(&desc->irq_data);

	/*
	 * hreset powerdown ctrl,
	 * if otmp_hreset_pwrdown_flag is not 0, do cold reset
	 */
	if (pmic_mntn->otmp_hreset_pwrdown_flag != 0) {
		reg_value = pmic_mntn_reg_read(
			pmic_mntn, pmic_mntn->otmp_hreset_pwrdown_reg.addr);
		set_reg_bit(reg_value, pmic_mntn->otmp_hreset_pwrdown_reg.shift,
			pmic_mntn->otmp_hreset_pwrdown_reg.mask,
			pmic_mntn->otmp_hreset_pwrdown_val);
		pmic_mntn_reg_write(pmic_mntn,
			pmic_mntn->otmp_hreset_pwrdown_reg.addr, reg_value);
	}
	pmic_mntn_board_reboot();
}

#if defined CONFIG_HUAWEI_VSYS_PWROFF_ABS_PD
int pmic_mntn_config_smpl(bool enable)
{
	unsigned int reg_value;
	unsigned int value;

	if (g_pmic_mntn[PMU0] == NULL)
		return 1;

	if (enable) {
		value = 1;
		pr_info("%s %d enable smpl in kernel\n", __func__, __LINE__);
	} else {
		value = 0;
		pr_info("%s %d disable smpl in kernel\n", __func__, __LINE__);
	}

	reg_value = pmic_read_reg(g_pmic_mntn[PMU0]->smpl_en_reg.addr);
	set_reg_bit(reg_value, g_pmic_mntn[PMU0]->smpl_en_reg.shift,
		g_pmic_mntn[PMU0]->smpl_en_reg.mask, value);
	pmic_write_reg(g_pmic_mntn[PMU0]->smpl_en_reg.addr, reg_value);

	return 0;
}

int pmic_mntn_config_vsys_pwroff_abs_pd(bool enable)
{
	unsigned int reg_value;
	unsigned int value;

	if (g_pmic_mntn[PMU0] == NULL)
		return 1;

	if (enable) {
		value = g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_en_val;
		pr_info("%s %d enable abs_pd in kernel\n", __func__, __LINE__);
	} else {
		value = g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_dis_val;
		pr_info("%s %d disable abs_pd in kernel\n", __func__, __LINE__);
	}

	reg_value = pmic_read_reg(
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.addr);
	set_reg_bit(reg_value,
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.shift,
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.mask, value);
	pmic_reg_write_lock(
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.addr, reg_value);

	return 0;
}

int pmic_mntn_vsys_pwroff_abs_pd_state(void)
{
	int ret;
	unsigned int reg_value;

	if (g_pmic_mntn[PMU0] == NULL)
		return -1;

	reg_value = pmic_read_reg(
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.addr);
	ret = get_reg_bit(reg_value,
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.shift,
		g_pmic_mntn[PMU0]->vsys_pwroff_abs_pd_tm_reg.mask);

	return ret;
}

static int pmic_mntn_vsys_pwroff_abs_pd_mntn_initial(
		struct pmic_mntn_desc *pmic_mntn)
{
	struct device_node *root = NULL;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	s32 ret = 0;

	root = of_find_compatible_node(
		np, NULL, "hisilicon-vsys-pwroff-abs-pd");
	if (root == NULL) {
		dev_err(dev, "[%s]no vsys-pwroff-abs-pd root node\n",
			__func__);
		return -ENODEV;
	}

	pmic_mntn->vsys_pwroff_abs_pd_en_val = 0;
	pmic_mntn->vsys_pwroff_abs_pd_dis_val = 0;
	memset(&pmic_mntn->vsys_pwroff_abs_pd_tm_reg, 0,
		sizeof(pmic_mntn->vsys_pwroff_abs_pd_tm_reg));

	ret |= of_property_read_u32_array(root,
		"hisilicon,vsys-pwroff-abs-pd-ctrl-en",
		&pmic_mntn->vsys_pwroff_abs_pd_en_val, 0x1); /* width:1 */
	ret |= of_property_read_u32_array(root,
		"hisilicon,vsys-pwroff-abs-pd-ctrl-dis",
		&pmic_mntn->vsys_pwroff_abs_pd_dis_val, 0x1); /* width:1 */
	ret |= of_property_read_u32_array(root,
		"hisilicon,vsys-pwroff-abs-pd-ctrl-reg",
		(u32 *)&pmic_mntn->vsys_pwroff_abs_pd_tm_reg, 0x3); /* width:3 */
	if (ret) {
		dev_err(dev,
			"[%s]get pmic vsys-pwroff-abs-pd attribute failed\n",
			__func__);
		return -ENODEV;
	}

	return 0;
}

#else
int pmic_mntn_config_smpl(bool enable)
{
	return 0;
}
int pmic_mntn_config_vsys_pwroff_abs_pd(bool enable)
{
	return 0;
}
int pmic_mntn_vsys_pwroff_abs_pd_state(void)
{
	return 0;
}
static int pmic_mntn_vsys_pwroff_abs_pd_mntn_initial(
		struct pmic_mntn_desc *pmic_mntn)
{
	return 0;
}
#endif /* CONFIG_VSYS_PWROFF_ABS_PD */

static int pmic_mntn_otmp_mntn_irq_init(
				struct pmic_mntn_desc *pmic_mntn)
{
	struct device *dev = pmic_mntn->dev;
	s32 ret;
	unsigned char reg_value;

	/* Set the otmp threshold */
	reg_value = pmic_mntn_reg_read(
		pmic_mntn, pmic_mntn->otmp_thshd_reg.addr);
	set_reg_bit(reg_value, pmic_mntn->otmp_thshd_reg.shift,
		pmic_mntn->otmp_thshd_reg.mask, pmic_mntn->otmp_thshd_val);
	pmic_mntn_reg_write(
		pmic_mntn, pmic_mntn->otmp_thshd_reg.addr, reg_value);

	pmic_mntn->otmp_irq = of_irq_get_byname(dev->of_node, "otmp");
	if (pmic_mntn->ocp_irq < 0) {
		dev_err(dev, "[%s]spmi_get_irq_byname otmp_irq failed\n",
			__func__);
		return -ENODEV;
	}

	pmic_mntn->otmp_wq = create_singlethread_workqueue("pmic-otmp-wq");
	if (pmic_mntn->otmp_wq == NULL) {
		dev_err(dev,
			"[%s]create_singlethread_workqueue otmp_wq failed\n",
			__func__);
		return -ENODEV;
	}

	INIT_WORK(&pmic_mntn->otmp_wk, pmic_mntn_otmp_wk_handler);

	ret = devm_request_irq(dev, pmic_mntn->otmp_irq,
		pmic_mntn_otmp_irq_handler, IRQF_NO_SUSPEND, "pmic-otmp-irq",
		(void *)pmic_mntn);
	if (ret) {
		dev_dbg(dev, "[%s] request_irq otmp_irq ret %d\n", __func__,
			ret);
		destroy_workqueue(pmic_mntn->otmp_wq);
		pmic_mntn->otmp_wq = NULL;
		return -ENODEV;
	}
	return 0;
}

static int pmic_mntn_sysctrl_get(struct pmic_mntn_desc *pmic_mntn)
{
	struct device_node *root = NULL;
	struct device *dev = pmic_mntn->dev;

	root = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (root == NULL) {
		dev_err(dev, "%s: sysctrl No compatible node found\n",
			__func__);
		return -ENODEV;
	}

	g_sysctrl_base = of_iomap(root, 0);
	if (g_sysctrl_base == NULL) {
		dev_err(dev, "%s: sysctrl_base is NULL\n", __func__);
		return -ENODEV;
	}
	return 0;
}
static int pmic_mntn_otmp_mntn_initial(
	struct pmic_mntn_desc *pmic_mntn)
{
	struct device_node *root = NULL;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	int ret0, ret1, ret2;
	s32 ret;

	root = of_get_child_by_name(np, "OTMP");
	if (root == NULL) {
		dev_err(dev, "[%s]no pmic-mntn-otmp root node\n",
			__func__);
		return -ENODEV;
	}

	dev_info(dev, "[%s]get pmic-mntn-otmp root node\n", __func__);
	ret0 = of_property_read_u32_array(root,
	    "hisilicon,otmp-threshold-val",
	    (u32 *)&pmic_mntn->otmp_thshd_val, 0x1); /* width:1 */
	ret1 = of_property_read_u32_array(root,
	    "hisilicon,otmp-threshold-reg",
	    (u32 *)&pmic_mntn->otmp_thshd_reg, 0x3); /* width:3 */
	ret = (ret0 || ret1);
	if (ret) {
		dev_err(dev, "[%s]get pmic otmp attribute failed\n", __func__);
		return -ENODEV;
	}

	ret0 = of_property_read_u32_array(root,
	    "hisilicon,otmp-hreset-pwrdown-flag",
	    (u32 *)&pmic_mntn->otmp_hreset_pwrdown_flag, 0x1); /* width:1 */
	ret1 = of_property_read_u32_array(root,
	    "hisilicon,otmp-hreset-pwrdown-val",
	    (u32 *)&pmic_mntn->otmp_hreset_pwrdown_val, 0x1); /* width:1 */
	ret2 = of_property_read_u32_array(root,
	    "hisilicon,otmp-hreset-pwrdown-reg",
	    (u32 *)&pmic_mntn->otmp_hreset_pwrdown_reg, 0x3); /* width:3 */
	ret = (ret0 || ret1 || ret2);
	if (ret) {
		dev_err(dev, "[%s]get pmic otmp attribute failed\n", __func__);
		return -ENODEV;
	}

	ret = pmic_mntn_sysctrl_get(pmic_mntn);
	if (ret)
		return ret;

	ret = pmic_mntn_otmp_mntn_irq_init(pmic_mntn);
	if (ret) {
		dev_dbg(dev, "[%s] pmic_mntn_otmp_mntn_irq_init failed\n",
			__func__);
		iounmap(g_sysctrl_base);
		g_sysctrl_base = NULL;
		return ret;
	}
	return 0;
}

static void pmic_mntn_set_smpl(
	struct pmic_mntn_desc *pmic_mntn)
{
	struct device *dev = pmic_mntn->dev;
	unsigned char reg_value;

	/* Set SMPL on/off */
	reg_value =
		pmic_mntn_reg_read(pmic_mntn, pmic_mntn->smpl_en_reg.addr);
	dev_info(dev, "read reg_value:0x%x\n", reg_value);
	set_reg_bit(reg_value, pmic_mntn->smpl_en_reg.shift,
		pmic_mntn->smpl_en_reg.mask, pmic_mntn->smpl_en_val);
	pmic_mntn_reg_write(
		pmic_mntn, pmic_mntn->smpl_en_reg.addr, reg_value);
	reg_value =
		pmic_mntn_reg_read(pmic_mntn, pmic_mntn->smpl_en_reg.addr);
	dev_info(dev, "write reg_value:0x%x\n", reg_value);

	/* Set SMPL effective time */
	reg_value =
		pmic_mntn_reg_read(pmic_mntn, pmic_mntn->smpl_tm_reg.addr);
	set_reg_bit(reg_value, pmic_mntn->smpl_tm_reg.shift,
		pmic_mntn->smpl_tm_reg.mask, pmic_mntn->smpl_tm_val);
	pmic_mntn_reg_write(
		pmic_mntn, pmic_mntn->smpl_tm_reg.addr, reg_value);
}

static int pmic_mntn_smpl_mntn_initial(
	struct pmic_mntn_desc *pmic_mntn)
{
	struct device_node *root = NULL;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	s32 ret, ret0, ret1, ret2, ret3;
	unsigned int smpl_en_val = 0;

	root = of_get_child_by_name(np, "SMPL");
	if (root == NULL) {
		dev_err(dev, "[%s]no pmic-mntn-smpl root node\n",
			__func__);
		return -ENODEV;
	}

	dev_info(dev, "[%s]get pmic-mntn-smpl root node\n", __func__);
	ret0 = of_property_read_u32_array(root, "hisilicon,smpl-ctrl-en",
	    &pmic_mntn->smpl_en_val, 0x1); /* width:1 */
	ret1 = of_property_read_u32_array(root, "hisilicon,smpl-ctrl-en-reg",
	    (u32 *)&pmic_mntn->smpl_en_reg, 0x3); /* width:3 */
	ret2 = of_property_read_u32_array(root, "hisilicon,smpl-ctrl-time",
	    &pmic_mntn->smpl_tm_val, 0x1); /* width:1 */
	ret3 = of_property_read_u32_array(root, "hisilicon,smpl-ctrl-time-reg",
	    (u32 *)&pmic_mntn->smpl_tm_reg, 0x3); /* width:3 */
	ret = (ret0 || ret1 || ret2 || ret3);
	if (ret) {
		dev_err(dev, "[%s]get pmic smpl attribute failed\n", __func__);
		return -ENODEV;
	}

	ret = of_property_read_u32_array(root,
		"hisilicon,poweroff-charging-smpl-ctrl-en",
		&smpl_en_val, 0x1); /* width:1 */
	if (!ret) {
		dev_info(dev, "poweroff-charging-smpl-ctrl-en: %u\n",
			smpl_en_val);
		if (strstr(saved_command_line, "androidboot.mode=charger")) {
			pmic_mntn->smpl_en_val = smpl_en_val;
			dev_info(dev, "pmic_mntn->smpl_en_val:%u\n",
				pmic_mntn->smpl_en_val);
		}
	}

	pmic_mntn_set_smpl(pmic_mntn);
	return 0;
}

static int pmic_mntn_ocp_mntn_spec_get_dts(struct pmic_mntn_desc *pmic_mntn,
	struct device_node *root, struct device *dev, unsigned int index)
{
	s32 ret;
	unsigned int bit;
	struct pmic_exch_reg *exch_reg_tmp = NULL;

	exch_reg_tmp = pmic_mntn->ocp_exch_desc;

	ret = of_property_read_u32(root, "hisilicon,inacceptable-event",
		(u32 *)&exch_reg_tmp[index].inacceptable_event);
	for (bit = 0; bit < pmic_mntn->data_width; bit++)
		ret = ret || of_property_read_string_index(root,
				     "hisilicon,event-bit-name", bit,
				     (const char **)&exch_reg_tmp[index]
					     .event_bit_name[bit]);

	ret = ret || of_property_read_u32(root, "hisilicon,check_ocp_nofify",
			     (u32 *)&exch_reg_tmp[index].check_ocp_nofify);
	if (ret)
		dev_err(dev, "[%s]read attribute of check_ocp_nofify\n",
			__func__);

	ret = of_property_read_u32(root, "hisilicon,check_ocp_reset",
		(u32 *)&exch_reg_tmp[index].check_ocp_reset);
	if (ret)
		dev_err(dev, "[%s]read attribute of check_ocp_reset\n",
			__func__);

	ret = of_property_read_u32_array(root, "hisilicon,check_ocp_num",
		(u32 *)exch_reg_tmp[index].check_ocp_num,
		pmic_mntn->data_width);
	if (ret)
		dev_err(dev, "[%s]read attribute of check_ocp_num\n", __func__);

	ret = of_property_read_u32_array(root, "hisilicon,event-ops-reg",
		(u32 *)exch_reg_tmp[index].event_ops_reg,
		pmic_mntn->data_width);
	if (ret) {
		dev_err(dev, "[%s]read attribute of event-ops-reg\n", __func__);
		return -ENODEV;
	}
	return ret;
}

static int pmic_mntn_ocp_mntn_spec_data_init(
	struct pmic_mntn_desc *pmic_mntn)
{
	s32 ret;
	unsigned int index;
	char compatible_string[PMIC_DTS_ATTR_LEN] = {0};
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	struct device_node *root = NULL;
	struct pmic_exch_reg *exch_reg_tmp = NULL;

	exch_reg_tmp = pmic_mntn->ocp_exch_desc;

	for (index = 0; index < pmic_mntn->ocp_reg_n; index++) {
		snprintf(compatible_string, PMIC_DTS_ATTR_LEN,
			"hisilicon-pmic-mntn-ocp-reg0x%x",
			pmic_mntn->ocp_regs[index]);
		root = of_find_compatible_node(np, NULL, compatible_string);
		if (root == NULL) {
			dev_err(dev, "[%s]no %s root node\n", __func__,
				compatible_string);
			return -ENODEV;
		}

		exch_reg_tmp[index].event_bit_name = (char **)devm_kzalloc(dev,
			pmic_mntn->data_width * sizeof(char *), GFP_KERNEL);
		if (exch_reg_tmp[index].event_bit_name == NULL)
			return -ENOMEM;

		exch_reg_tmp[index].event_ops_reg = (u32 *)devm_kzalloc(
			dev, pmic_mntn->data_width * sizeof(u32), GFP_KERNEL);
		if (exch_reg_tmp[index].event_ops_reg == NULL)
			return -ENOMEM;

		exch_reg_tmp[index].check_ocp_num = (u32 *)devm_kzalloc(
			dev, pmic_mntn->data_width * sizeof(u32), GFP_KERNEL);
		if (exch_reg_tmp[index].check_ocp_num == NULL)
			return -ENOMEM;

		exch_reg_tmp[index].check_count = (u32 *)devm_kzalloc(
			dev, pmic_mntn->data_width * sizeof(u32), GFP_KERNEL);
		if (exch_reg_tmp[index].check_count == NULL)
			return -ENOMEM;

		ret = pmic_mntn_ocp_mntn_spec_get_dts(
			pmic_mntn, root, dev, index);
		if (ret) {
			dev_err(dev,
				"[%s]pmic_ocp_mntn_spec_data_get_pro\n",
				__func__);
			return ret;
		}
	}
	return 0;
}

static int pmic_mntn_ocp_mntn_irq_initial(
	struct pmic_mntn_desc *pmic_mntn)
{
	struct device *dev = pmic_mntn->dev;
	s32 ret;

	pmic_mntn->ocp_irq = of_irq_get_byname(dev->of_node, "ocp");
	if (pmic_mntn->ocp_irq < 0) {
		dev_err(dev, "[%s] of_irq_get_byname ocp_irq failed\n",
			__func__);
		return -ENODEV;
	}
	dev_err(dev, "[%s] of_irq_get_byname ocp_irq %d\n",
		__func__, pmic_mntn->ocp_irq);

	pmic_mntn->ocp_wq = create_singlethread_workqueue("pmu-ocp-wq");
	if (pmic_mntn->ocp_wq == NULL) {
		dev_err(dev,
			"[%s] create_singlethread_workqueue ocp_wq failed\n",
			__func__);
		return -ENODEV;
	}

	INIT_WORK(&pmic_mntn->ocp_wk, pmic_mntn_ocp_wq_handler);

	ret = devm_request_irq(dev, pmic_mntn->ocp_irq,
			pmic_mntn_ocp_irq_handler, IRQF_NO_SUSPEND,
			"pmu-ocp-irq", (void *)pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]request_irq ocp_irq failed\n", __func__);
		destroy_workqueue(pmic_mntn->ocp_wq);
		pmic_mntn->ocp_wq = NULL;
		return -ENODEV;
	}

#ifdef CONFIG_HISI_PMIC_DEBUG
	if (!g_mntn_notifier_flag) {
		g_mntn_notifier_flag = 1;
		ret = pmic_mntn_register_notifier(&pmic_mntn_test_nb);
		if (ret)
			pr_err("%s:hisi pmic mntn test nb register fail!\n",
				__func__);
	}
#endif
	return 0;
}

static int pmic_mntn_ocp_mntn_initial(
	struct pmic_mntn_desc *pmic_mntn)
{
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	s32 ret;

	ret = of_property_read_u32(
		np, "hisilicon,ocp-reg-num", (u32 *)&pmic_mntn->ocp_reg_n);
	if (ret || !pmic_mntn->ocp_reg_n) {
		dev_err(dev, "[%s]get ocp-reg-num attribute failed\n",
			__func__);
		return -ENODEV;
	}

	pmic_mntn->ocp_regs = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->ocp_reg_n, GFP_KERNEL);
	if (pmic_mntn->ocp_regs == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,ocp-regs",
		pmic_mntn->ocp_regs, pmic_mntn->ocp_reg_n);
	if (ret) {
		dev_err(dev, "[%s]get ocp-regs attribute failed\n", __func__);
		return -ENODEV;
	}

	pmic_mntn->ocp_exch_desc = (struct pmic_exch_reg *)devm_kzalloc(
		dev, sizeof(*pmic_mntn->ocp_exch_desc) * pmic_mntn->ocp_reg_n,
			GFP_KERNEL);
	if (pmic_mntn->ocp_exch_desc == NULL)
		return -ENOMEM;

	ret = pmic_mntn_ocp_mntn_spec_data_init(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_ocp_mntn_spec_data_init failed\n",
			__func__);
		return ret;
	}

	pmic_mntn->ocp_event_buff = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->ocp_reg_n, GFP_KERNEL);
	if (pmic_mntn->ocp_event_buff == NULL)
		return -ENOMEM;

	pmic_mntn_clear_ocp(pmic_mntn);

	ret = pmic_mntn_ocp_mntn_irq_initial(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_ocp_mntn_irq_initial failed\n",
			__func__);
		return -ENODEV;
	}

	return 0;
}

static int pmic_mntn_record_buf_data_initial(
	struct device_node *root, struct pmic_mntn_desc *pmic_mntn)
{
	unsigned int index, bit;
	char compatible_string[PMIC_DTS_ATTR_LEN] = {0};
	s32 ret;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	struct pmic_exch_reg *exch_reg_tmp = NULL;

	exch_reg_tmp = pmic_mntn->record_exch_desc;

	for (index = 0; index < pmic_mntn->record_reg_n; index++) {
		snprintf(compatible_string, PMIC_DTS_ATTR_LEN,
			"hisilicon-pmic-mntn-record-reg0x%x",
			pmic_mntn->record_regs[index]);
		root = of_find_compatible_node(np, NULL, compatible_string);
		if (root == NULL) {
			dev_err(dev, "[%s]no %s root node.index %u\n", __func__,
				compatible_string, index);
			return -ENODEV;
		}

		exch_reg_tmp[index].event_bit_name = (char **)devm_kzalloc(dev,
			pmic_mntn->data_width * sizeof(char *), GFP_KERNEL);
		if (exch_reg_tmp[index].event_bit_name == NULL)
			return -ENOMEM;

		ret = of_property_read_u32(root, "hisilicon,inacceptable-event",
			(u32 *)&exch_reg_tmp[index].inacceptable_event);
		for (bit = 0; bit < pmic_mntn->data_width; bit++)
			ret = ret || of_property_read_string_index(root,
				 "hisilicon,event-bit-name", bit,
				(const char **)&exch_reg_tmp[index].event_bit_name[bit]);

		if (ret) {
			dev_err(dev, "[%s]read attribute of %s\n", __func__,
				compatible_string);
			return -ENODEV;
		}
	}
	return 0;
}

static int pmic_mntn_record_mntn_initial(
	struct pmic_mntn_desc *pmic_mntn)
{
	struct device_node *root = NULL;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	s32 ret;

	ret = of_property_read_u32(np, "hisilicon,record-reg-num",
		(u32 *)&pmic_mntn->record_reg_n);
	if (ret || !pmic_mntn->record_reg_n) {
		dev_err(dev, "[%s]get record-reg-num attribute failed\n",
			__func__);
		return -ENODEV;
	}

	pmic_mntn->record_regs = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->record_reg_n, GFP_KERNEL);
	if (pmic_mntn->record_regs == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,record-regs",
		pmic_mntn->record_regs, pmic_mntn->record_reg_n);
	if (ret) {
		dev_err(dev, "[%s]get record-regs attribute failed\n",
			__func__);
		return -ENODEV;
	}
#ifdef CONFIG_HUAWEI_DSM
	ret = pmic_mntn_record_mntn_dsm_data_get(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s] dsm_record_regs_mask buffer failed\n",
			__func__);
		return ret;
	}
#endif
	pmic_mntn->record_exch_desc = (struct pmic_exch_reg *)devm_kzalloc(dev,
		sizeof(*pmic_mntn->record_exch_desc) * pmic_mntn->record_reg_n,
			GFP_KERNEL);
	if (pmic_mntn->record_exch_desc == NULL)
		return -ENOMEM;

	ret = pmic_mntn_record_buf_data_initial(root, pmic_mntn);
	if (ret) {
		dev_err(dev,
			"[%s] pmic_mntn_record_buf_data_initial failed\n",
			__func__);
		return ret;
	}

	pmic_mntn->record_event_buff = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->record_reg_n, GFP_KERNEL);
	if (pmic_mntn->record_event_buff == NULL)
		return -ENOMEM;

	return 0;
}

static int pmic_mntn_key_register_record_initial(
				struct pmic_mntn_desc *pmic_mntn)
{
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;
	s32 ret;

	ret = of_property_read_u32(np, "hisilicon,pmu-record-reg-num",
		(u32 *)&pmic_mntn->pmu_record_reg_n);
	if (ret) {
		dev_err(dev, "[%s]get pmu-record-reg-num attribute failed\n",
		    __func__);
		return -ENODEV;
	}

	if (pmic_mntn->pmu_record_reg_n <= 0)
		return ret;

	pmic_mntn->pmu_record_regs = (u32 *)devm_kzalloc(
		dev, sizeof(u32) * pmic_mntn->pmu_record_reg_n, GFP_KERNEL);
	if (pmic_mntn->pmu_record_regs == NULL)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,pmu-record-regs",
		pmic_mntn->pmu_record_regs, pmic_mntn->pmu_record_reg_n);
	if (ret) {
		dev_err(dev,
			"[%s]get pmu-record-regs attribute failed\n",
			__func__);
		return -ENODEV;
	}

	return ret;
}

static int pmic_mntn_funs_initial(
				struct pmic_mntn_desc *pmic_mntn)
{
	int ret;
	struct device *dev = pmic_mntn->dev;

	ret = pmic_mntn_key_register_record_initial(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmu_key_register_record_initial error\n",
			__func__);
		return ret;
	}

	ret = pmic_mntn_otmp_mntn_initial(pmic_mntn);
	if (ret)
		dev_dbg(dev, "[%s]pmic_mntn_otmp_mntn_initial ret %d\n",
			__func__, ret);

	ret = pmic_mntn_smpl_mntn_initial(pmic_mntn);
	if (ret)
		dev_dbg(dev, "[%s]pmic_mntn_smpl_mntn_initial ret %d\n",
			__func__, ret);

	/* just read dtsi vsys_pwroff_abs_pd  property */
	ret = pmic_mntn_vsys_pwroff_abs_pd_mntn_initial(pmic_mntn);
	if (ret)
		dev_dbg(dev, "[%s]vsys_pwroff_abs_pd_mntn_initial ret %d\n",
			__func__, ret);

	ret = pmic_mntn_ocp_mntn_initial(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_ocp_mntn_initial error\n",
			__func__);
		return ret;
	}

	ret = pmic_mntn_record_mntn_initial(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_record_mntn_initial error\n",
			__func__);
		return ret;
	}

	return ret;
}

int pmic_mntn_drv_init(struct pmic_mntn_desc *pmic_mntn)
{
	int ret;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;

	pmic_mntn->health = (unsigned int)PMIC_HEALTH_STATUS_NORMAL;

	pmic_mntn->init_log_show =
		(char *)devm_kzalloc(dev, PMIC_PRINT_BUF_SIZE, GFP_KERNEL);
	if (pmic_mntn->init_log_show == NULL)
		return -ENOMEM;

	pmic_mntn->irq_log_show =
		(char *)devm_kzalloc(dev, PMIC_PRINT_BUF_SIZE, GFP_KERNEL);
	if (pmic_mntn->irq_log_show == NULL)
		return -ENOMEM;

	ret = of_property_read_u32(
		np, "hisilicon,data-width", &pmic_mntn->data_width);
	if (ret || !pmic_mntn->data_width) {
		dev_err(dev, "[%s]get pmic data-width failed\n", __func__);
		return -ENODEV;
	}

	ret = pmic_mntn_funs_initial(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_mntn_funs_initial failed\n",
			__func__);
		return ret;
	}

	pmic_mntn_record_events(pmic_mntn);

	if (of_device_is_compatible(np, PMU1_MNTN_COMP_STR))
		g_pmic_mntn[PMU1] = pmic_mntn;
	else if (of_device_is_compatible(np, PMU0_MNTN_COMP_STR))
		g_pmic_mntn[PMU0] = pmic_mntn;

	return ret;
}

static void pmic_mntn_vsys_surge_record_event(
	struct pmic_mntn_desc *pmic_mntn)
{
	unsigned int value;
#if defined CONFIG_HUAWEI_DSM
	const char *bit_name = "pmic vsys surge";
#endif

	value = pmic_read_reg(pmic_mntn->record_vsys_surge.vsys_surge_reg);
	if ((value & pmic_mntn->record_vsys_surge.vsys_surge_reg_mask) ==
	pmic_mntn->record_vsys_surge.vsys_surge_reg_mask) {
		pr_err("%s vsys surge reg 0x%x, value 0x%x\n", __func__,
			pmic_mntn->record_vsys_surge.vsys_surge_reg, value);
#if defined CONFIG_HUAWEI_DSM
		pmic_mntn_ocp_dsm_client(bit_name,
			pmic_mntn->record_vsys_surge.vsys_surge_dmd_offset);
#endif
	/* clear sys surge record */
		value = (value & (~pmic_mntn->record_vsys_surge.vsys_surge_reg_mask)) |
			(pmic_mntn->record_vsys_surge.vsys_surge_reg_mask);
		pmic_write_reg(pmic_mntn->record_vsys_surge.vsys_surge_reg, value);
	}
}

static int pmic_mntn_usb_notifier_call(struct notifier_block *usb_nb,
	unsigned long event, void *data)
{
	struct pmic_mntn_desc *pmic_mntn = container_of(usb_nb, struct pmic_mntn_desc, usb_nb);

	pmic_mntn_vsys_surge_record_event(pmic_mntn);
	return NOTIFY_OK;
}

void pmic_vsys_surge_init(struct pmic_mntn_desc *pmic_mntn)
{
	int ret;
	struct device *dev = pmic_mntn->dev;
	struct device_node *np = dev->of_node;

	ret = of_property_read_u32_array(np, "hisilicon,vsys_surge_event",
		(int *)&pmic_mntn->record_vsys_surge, 3); /* vsys surge parm is 3 */
	if (!ret) {
		pmic_mntn_vsys_surge_record_event(pmic_mntn);
		pmic_mntn->usb_nb.notifier_call = pmic_mntn_usb_notifier_call;
		ret = chip_charger_type_notifier_register(&pmic_mntn->usb_nb);
		if (ret < 0)
			dev_err(dev, "[%s]chip_charger_type_notifier_register failed\n",
				__func__);
	}
}

void pmic_mntn_drv_deinit(void)
{
#ifdef CONFIG_HISI_PMIC_DEBUG
		int ret;

		if (!g_mntn_notifier_flag)
			return;

		ret = pmic_mntn_unregister_notifier(&pmic_mntn_test_nb);
		if (ret)
			pr_err("%s: hisi pmic mntn test nb unregister fail!\n",
				__func__);
		g_mntn_notifier_flag = 0;
#endif
}

static int pmic_mntn_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pmic_mntn_desc *pmic_mntn = NULL;
	int ret;

	dev_err(dev, "[%s] +\n", __func__);
	pmic_mntn = (struct pmic_mntn_desc *)devm_kzalloc(
		dev, sizeof(*pmic_mntn), GFP_KERNEL);
	if (pmic_mntn == NULL)
		return -ENOMEM;

	pmic_mntn->dev = dev;

	pmic_mntn->regmap = dev_get_regmap(dev->parent, NULL);
	if (!pmic_mntn->regmap)
		return -ENODEV;

	ret = pmic_mntn_drv_init(pmic_mntn);
	if (ret) {
		dev_err(dev, "[%s]pmic_mntn_drv_init failed\n",
			__func__);
		return ret;
	}

	dev_err(dev, "[%s] succ\n", __func__);

	return 0;
}

#ifdef CONFIG_HISILICON_PLATFORM_MAINTAIN
static int pmic_mntn_sdcard_ocp_handler(char *power_name)
{
	static struct regulator *power_sd = NULL;
	static struct regulator *power_sdio = NULL;
	int ret;

	if (power_name)
		pr_err("%s ocp special handler\n", power_name);

	if (power_sd == NULL) {
		power_sd = regulator_get(NULL, SUPPLY_SD);
		if (IS_ERR(power_sd)) {
			pr_err("[%s]sd regulator found\n", __func__);
			return -ENODEV;
		}
	}

	ret = regulator_force_disable(power_sd);
	if (ret) {
		pr_err("[%s]disable sd regulator error\n", __func__);
		return ret;
	}

	if (power_sdio == NULL) {
		power_sdio = regulator_get(NULL, SUPPLY_SD_IO);
		if (IS_ERR(power_sdio)) {
			pr_err("[%s]sdio regulator found\n", __func__);
			return -ENODEV;
		}
	}

	ret = regulator_force_disable(power_sdio);
	if (ret) {
		pr_err("[%s]disable sdio regulator error\n", __func__);
		return ret;
	}

	return ret;
}
#endif

static int __init pmic_mntn_register_special_ocp(void)
{
#ifdef CONFIG_HISILICON_PLATFORM_MAINTAIN
	int ret;

	if (!check_himntn(HIMNTN_SD2JTAG) && !check_himntn(HIMNTN_SD2DJTAG)) {
		ret = pmic_special_ocp_register(
			SUPPLY_SD, pmic_mntn_sdcard_ocp_handler);
		if (ret) {
			pr_err("[%s]supply_sd register error\n", __func__);
			return ret;
		}

		ret = pmic_special_ocp_register(
			SUPPLY_SD_IO, pmic_mntn_sdcard_ocp_handler);
		if (ret) {
			pr_err("[%s]supply_sd_io register error\n", __func__);
			return ret;
		}
	}
#endif
	return 0;
}

/* The PMU can be registered only after all PMUs are initialized */
late_initcall(pmic_mntn_register_special_ocp);

static int pmic_mntn_remove(struct platform_device *pdev)
{
	pmic_mntn_drv_deinit();
	return 0;
}

const static struct of_device_id pmic_mntn_match_tbl[] = {
	{
		.compatible = "hisilicon,hisi-pmic-mntn",
	},

	{}
};


static struct platform_driver pmic_mntn_driver = {
	.driver = {
		.name = "hisi-pmic-mntn",
		.owner  = THIS_MODULE,
		.of_match_table = pmic_mntn_match_tbl,
	},
	.probe = pmic_mntn_probe,
	.remove = pmic_mntn_remove,

};

static int __init pmic_mntn_init(void)
{
	return platform_driver_register(&pmic_mntn_driver);
}

static void __exit pmic_mntn_exit(void)
{
	platform_driver_unregister(&pmic_mntn_driver);
}


module_init(pmic_mntn_init);
module_exit(pmic_mntn_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PMU MNTN Driver");
