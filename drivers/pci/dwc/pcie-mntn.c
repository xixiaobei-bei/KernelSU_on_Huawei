/*
 * pcie-mntn.c
 *
 * PCIe mntn
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "pcie-kport-common.h"

#if defined(CONFIG_HUAWEI_DSM)
#define DSM_LOG_BUFFER_SIZE 256
#define RC_AER_OFFSET 0x100
#include <dsm/dsm_pub.h>

static u32 dsm_record_info[DSM_LOG_BUFFER_SIZE];
static u32 g_info_size;

void dsm_pcie_dump_info(struct pcie_kport *pcie, enum dsm_err_id id)
{
	u32 i = 0;

	mutex_lock(&pcie->power_lock);
	if (!atomic_read(&(pcie->is_power_on)))
		goto MUTEX_UNLOCK;

	dsm_record_info[i++] = id;
	if (pcie->dtsinfo.ep_device_type != EP_DEVICE_HI110X && pcie->dtsinfo.ep_device_type != EP_DEVICE_MODEM)
		dsm_record_info[i++] = gpio_get_value(pcie->gpio_id_reset);
	if (id != DSM_ERR_POWER_ON) {
		dsm_record_info[i++] = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		dsm_record_info[i++] = pcie_apb_phy_readl(pcie, SOC_PCIEPHY_STATE34_ADDR);
		dsm_record_info[i++] = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);
		dsm_record_info[i++] = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE5_ADDR);
		dsm_record_info[i++] = pcie_apb_ctrl_readl(pcie, PCIE_APP_LTSSM_ENABLE);
	}
	g_info_size = i;

MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
}

void dsm_pcie_clear_info(void)
{
	u32 i;

	g_info_size = 0;
	for (i = 0; i < DSM_LOG_BUFFER_SIZE; i++)
		dsm_record_info[i] = 0;
}

/*
 * return pcie dsm log_buffer addr
 * log string is stroaged in this buffer
 * param:
 *      buf - storage register value for wifi dmd_log
 *      buflen - buf size reserved for PCIe, max 384
 */
void dsm_pcie_dump_reginfo(char *buf, u32 buflen)
{
	int ret;
	u32 i;
	u32 seglen = 9;

	if (buf && buflen > (seglen * g_info_size)) {
		for (i = 0; i < g_info_size; i++) {
			ret = snprintf_s(&buf[(u64)i * seglen],
					 buflen - (u64)i * seglen,
					 seglen, "%08x ", dsm_record_info[i]);
			if (ret < 0)
				PCIE_PR_E("snprintf_s fail");
		}
	}
}
EXPORT_SYMBOL_GPL(dsm_pcie_dump_reginfo);

#else

void dsm_pcie_dump_info(struct pcie_kport *pcie, enum dsm_err_id id)
{
}

void dsm_pcie_clear_info(void)
{
}

/*
 * return pcie dsm log_buffer addr
 * log string is stroaged in this buffer
 */
void dsm_pcie_dump_reginfo(void *addr)
{
}
EXPORT_SYMBOL_GPL(dsm_pcie_dump_reginfo);
#endif

void dump_apb_register(struct pcie_kport *pcie)
{
	u32 j;

	mutex_lock(&pcie->power_lock);
	if (!atomic_read(&pcie->is_power_on)) {
		PCIE_PR_E("PCIe is Poweroff");
		goto MUTEX_UNLOCK;
	}

	pcie_dump_ilde_hw_stat(pcie->rc_id);

	/* 4 register-value per line (base1:0x0 base2:0x400) */
	PCIE_PR_I("####DUMP APB CORE Register :");
	for (j = 0; j < 0x4; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n",
			0x10 * j,
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x0),
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x4),
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x8),
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0xC));

	/* 4 register-value per line (base1:0x0 base2:0x300000) */
	for (j = 0; j < 0x2; j++)
		pr_info("0x%-8x: %8x %8x %8x %8x\n",
			0x10 * j + 0x400,
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x0 + 0x400),
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x4 + 0x400),
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0x8 + 0x400),
			pcie_apb_ctrl_readl(pcie, 0x10 * j + 0xC + 0x400));

	/* credit info */
	for (j = 0; j < 0x3; j++)
		pr_info("0x%-8x: %8x %8x %8x\n",
			0x4 * j + 0x430,
			pcie_apb_ctrl_readl(pcie, 0x4 * j + 0x430),
			pcie_apb_ctrl_readl(pcie, 0x4 * j + 0x430),
			pcie_apb_ctrl_readl(pcie, 0x4 * j + 0x430));

	PCIE_PR_I("####DUMP APB PHY Register :");
	pr_info("0x%-8x: %8x %8x %8x %8x %8x\n",
		0x0,
		pcie_apb_phy_readl(pcie, 0x0),
		pcie_apb_phy_readl(pcie, 0x4),
		pcie_apb_phy_readl(pcie, 0x8),
		pcie_apb_phy_readl(pcie, 0xc),
		pcie_apb_phy_readl(pcie, 0x400));
	pr_info("PHY MPLL status[0x488]: 0x%x\n", pcie_apb_phy_readl(pcie, 0x488));

	pcie_phy_state(pcie);

MUTEX_UNLOCK:
	mutex_unlock(&pcie->power_lock);
}

typedef void (*WIFI_DUMP_FUNC) (void);
typedef void (*DEVICE_DUMP_FUNC) (void);
#ifdef CONFIG_KIRIN_PCIE_NOC_DBG
WIFI_DUMP_FUNC g_device_dump = NULL;

bool is_pcie_target(int target_id)
{
	u32 i;

	for (i = 0; i < g_rc_num; i++)
		if (g_pcie_port[i].dtsinfo.noc_target_id == target_id)
			return true;

	return false;
}
EXPORT_SYMBOL_GPL(is_pcie_target);

void set_pcie_dump_flag(int target_id)
{
	u32 i;

	for (i = 0; i < g_rc_num; i++)
		if (g_pcie_port[i].dtsinfo.noc_target_id == target_id) {
			g_pcie_port[i].dtsinfo.noc_mntn = 1;
			return;
		}
}
EXPORT_SYMBOL_GPL(set_pcie_dump_flag);

void clear_pcie_dump_flag(void)
{
	u32 i;

	for (i = 0; i < g_rc_num; i++)
		if (g_pcie_port[i].dtsinfo.noc_mntn)
			g_pcie_port[i].dtsinfo.noc_mntn = 0;
}

bool get_pcie_dump_flag(void)
{
	u32 i;

	for (i = 0; i < g_rc_num; i++)
		if (g_pcie_port[i].dtsinfo.noc_mntn)
			return true;

	return false;
}
EXPORT_SYMBOL_GPL(get_pcie_dump_flag);

void dump_pcie_apb_info(void)
{
	struct pcie_kport *pcie = NULL;
	u32 i;

	for (i = 0; i < g_rc_num; i++)
		if (g_pcie_port[i].dtsinfo.noc_mntn)
			pcie = &g_pcie_port[i];

	if (!pcie) {
		PCIE_PR_I("Not set!");
		return;
	}

	if (!atomic_read(&pcie->is_power_on)) {
		PCIE_PR_E("PCIe is Poweroff");
		return;
	}

	dump_apb_register(pcie);

	if (g_device_dump) {
		PCIE_PR_E("Dump wifi info");
		g_device_dump();
	}

	clear_pcie_dump_flag();
}
EXPORT_SYMBOL_GPL(dump_pcie_apb_info);

void register_device_dump_func(WIFI_DUMP_FUNC func)
{
	g_device_dump = func;
}
EXPORT_SYMBOL_GPL(register_device_dump_func);
#else
bool is_pcie_target(int target_id)
{
	return false;
}
EXPORT_SYMBOL_GPL(is_pcie_target);

void set_pcie_dump_flag(int target_id)
{
}
EXPORT_SYMBOL_GPL(set_pcie_dump_flag);

void clear_pcie_dump_flag(void)
{
}

bool get_pcie_dump_flag(void)
{
	return false;
}
EXPORT_SYMBOL_GPL(get_pcie_dump_flag);

void dump_pcie_apb_info(void)
{
}
EXPORT_SYMBOL_GPL(dump_pcie_apb_info);

void register_device_dump_func(WIFI_DUMP_FUNC func)
{
}
EXPORT_SYMBOL_GPL(register_device_dump_func);
#endif

void register_wifi_dump_func(WIFI_DUMP_FUNC func)
{
	register_device_dump_func(func);
}
EXPORT_SYMBOL_GPL(register_wifi_dump_func);
