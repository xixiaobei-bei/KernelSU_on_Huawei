/*
 *
 * PCIe test interface
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include <linux/pci-aspm.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <asm/memory.h>
#include <asm/cacheflush.h>
#include <securec.h>
#include <linux/hisi/pcie-kport-api.h>
#include "pcie-kport-common.h"
#include "pcie-kport-idle.h"

#define PCI_CONFIG_SPACE         0x100
#define TEST_MEM_SIZE            0x100000
#define TEST_MEM_PCI_BASE_OFFSET 0x1000000
#define MAX_RC_NUM               2
#define EP_INBOUND_INIT_VAL   0x34
#define RC_INBOUND_INIT_VAL   0x78
#define PCIE_SYS_INT          (0x1 << 5)
#define LOOPBACK_ENABLE       (0x1 << 2)
#define EQUALIZATION_DISABLE  (0x1 << 16)
#define PIPELOOPBACK_ENABLE   (0x1u << 31)
#define SIZE_16M	0x1000000ul
#define MEM_16M_ALIGN(a)	(((a) + (SIZE_16M)) & ~((SIZE_16M) - 1))

enum phy_lb_dir {
	TX2RX = 0,
	RX2TX = 1,
};

enum mem_trans_type {
	MEM_READ = 0,
	MEM_WRITE = 1,
};

struct pcie_test_st {
	struct pcie_kport *pcie;
	u64 rc_rdmem_addr;
	u64 rc_wrmem_addr;
	u64 ep_rdmem_addr;
	u64 ep_wrmem_addr;
	u64 rc_inboundmem_addr;
	u64 ep_inboundmem_addr;
};

struct pcie_test_st g_test_pcie[MAX_RC_NUM];

/*
 * check_pcie_on_work - Check if the parameters are valid and host is working.
 * input: rc_id: Host ID
 * return: 0: on_work
 */
int check_pcie_on_work(u32 rc_id)
{
	struct pcie_test_st *test_pcie = NULL;

	if (!pcie_check_rcid(rc_id)) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	test_pcie = &g_test_pcie[rc_id];

	if (!test_pcie->pcie) {
		PCIE_PR_E("PCIe%u is null", rc_id);
		return -EINVAL;
	}

	if (!atomic_read(&(test_pcie->pcie->is_power_on))) {
		PCIE_PR_E("PCIe%u is power off", rc_id);
		return -EINVAL;
	}

	return 0;
}

static void pcie_atu_disable(struct pcie_kport *pcie, u32 index, u32 direct)
{
	u32 iatu_offset = pcie->dtsinfo.iatu_base_offset;

	if (iatu_offset != ATU_VIEWPORT) {
		iatu_offset += index * PER_ATU_SIZE;
		if (direct & ATU_REGION_INBOUND)
			iatu_offset += INBOUNT_OFFSET;
	} else {
		pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
				     ATU_VIEWPORT, REG_DWORD_ALIGN,
				     direct | index);
	}

	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
			     iatu_offset + ATU_CR2, REG_DWORD_ALIGN, 0x0);
}

/*
 * disable_outbound_iatu - Disable outbound iatu region.
 * @rc_id: Host ID;
 * @index: Outbound region ID;
 */
void disable_outbound_iatu(u32 rc_id, u32 index)
{
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id) || (index >= PCIE_INBOUND_OUTBOND_MAX))
		return;

	pcie = &g_pcie_port[rc_id];
	pcie_atu_disable(pcie, (u32)index, ATU_REGION_OUTBOUND);
}

/*
 * disable_inbound_iatu - Disable inbound iatu region.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
void disable_inbound_iatu(u32 rc_id, u32 index)
{
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id) || (index >= PCIE_INBOUND_OUTBOND_MAX))
		return;

	pcie = &g_pcie_port[rc_id];
	pcie_atu_disable(pcie, (u32)index, ATU_REGION_INBOUND);
}

/*
 * rc_read_ep_cfg - RC read EP configuration.
 * @rc_id: Host ID;
 */
int rc_read_ep_cfg(u32 rc_id)
{
	unsigned int i;
	struct pcie_kport *pcie = NULL;
	u32 val1, val2, val3, val4;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_pcie[rc_id].pcie;

	if (!pcie->ep_dev) {
		PCIE_PR_E("Failed to get EP device");
		return -1;
	}

	for (i = 0; i < PCI_CONFIG_SPACE; i++) {
			pci_read_config_dword(pcie->ep_dev, 0x10 * i + 0x0, &val1);
			pci_read_config_dword(pcie->ep_dev, 0x10 * i + 0x4, &val2);
			pci_read_config_dword(pcie->ep_dev, 0x10 * i + 0x8, &val3);
			pci_read_config_dword(pcie->ep_dev, 0x10 * i + 0xC, &val4);
			pr_info("0x%-8x: %8x %8x %8x %8x\n",
				0x10 * i, val1, val2, val3, val4);
	}

	return 0;
}

static void show_diff(u64 rc_addr, u64 ep_addr, unsigned int size)
{
	u32 rc_val, ep_val;
	unsigned int i;

	for (i = 0; i < size; i += REG_DWORD_ALIGN) {
		rc_val = readl((void *)(uintptr_t)rc_addr + i);
		ep_val = readl((void *)(uintptr_t)ep_addr + i);
		if (rc_val != ep_val)
			PCIE_PR_I("[0x%x]: rc_val=[0x%x], ep_val=[0x%x]", i, rc_val, ep_val);
	}
}

static void pcie_data_trans(void *dst, u32 dst_size, const void *src, u32 src_size)
{
	u32 i, val;
	u32 size = src_size < dst_size ? src_size : dst_size;

	for (i = 0; i < size; i += 0x4) {
		val = readl(src + i);
		writel(val, dst + i);
	}
}

static int temp_mem_prepare(char **memcmp, unsigned int size)
{
	char *temp_memcmp = NULL;
	int ret;

	if ((size > TEST_MEM_SIZE) || (size == 0))
		return -1;

	temp_memcmp = vmalloc(size);
	if (!temp_memcmp) {
		PCIE_PR_E("Failed to alloc temp_memcmp");
		return -1;
	}

	ret = memset_s(temp_memcmp, size, 0xFF, size);
	if (ret != EOK) {
		PCIE_PR_E("Failed to set default val[0xFF]");
		vfree(temp_memcmp);
		return -1;
	}

	*memcmp = temp_memcmp;

	return 0;
}

static int data_trans_and_check(u64 cpu_addr, unsigned int size,
	u64 wrmem_addr, int wrmem_val, enum mem_trans_type trans_type)
{
	int ret;
	u64 local_cpu_addr, dst, src;
	char *temp_memcmp = NULL;

	ret = temp_mem_prepare(&temp_memcmp, size);
	if (ret)
		return ret;

	local_cpu_addr = (__force uintptr_t)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!local_cpu_addr) {
		PCIE_PR_E("Failed to ioremap cpu addr");
		ret = -1;
		goto FREE_TEMP_MEM;
	}

	ret = memset_s((void *)(uintptr_t)wrmem_addr, TEST_MEM_SIZE, wrmem_val, size);
	if (ret != EOK) {
		PCIE_PR_E("Failed to set default val[0x0]");
		goto FAIL;
	}

	if (trans_type == MEM_READ) {
		dst = wrmem_addr;
		src = local_cpu_addr;
	} else if (trans_type == MEM_WRITE) {
		dst = local_cpu_addr;
		src = wrmem_addr;
	} else {
		PCIE_PR_E("Error trans type");
		goto FAIL;
	}

	__flush_dcache_area((void *)(uintptr_t)wrmem_addr, TEST_MEM_SIZE);
	pcie_data_trans((void *)(uintptr_t)dst,
			TEST_MEM_SIZE, (const void *)(uintptr_t)src, size);
	__flush_dcache_area((void *)(uintptr_t)wrmem_addr, TEST_MEM_SIZE);

	if (memcmp((void *)(uintptr_t)wrmem_addr, (void *)(uintptr_t)local_cpu_addr, size) != 0 ||
	    memcmp((void *)(uintptr_t)dst, temp_memcmp, size) == 0) {
		show_diff(wrmem_addr, local_cpu_addr, size);
		ret = -1;
	} else {
		ret = 0;
	}

FAIL:
	iounmap((void __iomem *)(uintptr_t)local_cpu_addr);
FREE_TEMP_MEM:
	vfree(temp_memcmp);
	return ret;
}

/*
 * rc_read_ep_mem - RC read EP mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Inbound region ID;
 */
int rc_read_ep_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	struct pcie_kport *pcie = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_pcie = &g_test_pcie[rc_id];
	pcie = test_pcie->pcie;

	if (!test_pcie->rc_rdmem_addr) {
		PCIE_PR_E("The test_pcie->rc_mem_addr is null");
		return -1;
	}

	cpu_addr = (MEM_16M_ALIGN(pcie->pci->pp.cfg0_base) + TEST_MEM_PCI_BASE_OFFSET);

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	PCIE_PR_I("EP addr in cpu physical is [0x%pK], size is [0x%x]", (void *)(uintptr_t)cpu_addr, size);

	pcie_kport_outbound_atu(rc_id, index, ATU_TYPE_MEM, cpu_addr, cpu_addr, TEST_MEM_SIZE);

	return data_trans_and_check(cpu_addr, size, test_pcie->rc_rdmem_addr, 0x0, MEM_READ);
}

/*
 * set_ep_mem_inbound - Set EP DDR mem inbound for loopback data transfer.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
int set_ep_mem_inbound(u32 rc_id, int index)
{
	u64 busdev, temp_addr;
	struct pcie_test_st *test_pcie = NULL;
	struct pcie_kport *pcie = NULL;
	int i;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_pcie = &g_test_pcie[rc_id];
	pcie = test_pcie->pcie;
	set_bme(rc_id, ENABLE);
	set_mse(rc_id, ENABLE);
	busdev = MEM_16M_ALIGN(pcie->pci->pp.cfg0_base);

	if (!test_pcie->ep_inboundmem_addr) {
		PCIE_PR_E("The test_pcie->ep_mem_addr is null");
		return -1;
	}

	temp_addr = virt_to_phys((void *)(uintptr_t)test_pcie->ep_inboundmem_addr);

	PCIE_PR_I("Inbound pci_add [0x%pK] to cpu_addr[0x%pK]",
		     (void *)(uintptr_t)busdev, (void *)(uintptr_t)temp_addr);

	pcie_inbound_atu(rc_id, index, ATU_TYPE_MEM, temp_addr, busdev, TEST_MEM_SIZE);

	for (i = 0; i < TEST_MEM_SIZE; i += REG_DWORD_ALIGN)
		writel(EP_INBOUND_INIT_VAL, (void *)(uintptr_t)test_pcie->ep_inboundmem_addr + i);

	__flush_dcache_area((void *)(uintptr_t)test_pcie->ep_inboundmem_addr, TEST_MEM_SIZE);

	return 0;
}

/*
 * read_ep_addr - Read EP MEM value.
 * @rc_id: Host ID;
 * @offset: The offset which you want to read;
 * @size: Data size;
 */
void read_ep_addr(u32 rc_id, u32 offset, u32 size)
{
	u32 i, val;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_pcie = &g_test_pcie[rc_id];

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	if (!test_pcie->ep_rdmem_addr) {
		PCIE_PR_E("The test_pcie->ep_rdmem_add is null");
		return;
	}
	if (!test_pcie->ep_wrmem_addr) {
		PCIE_PR_E("The test_pcie->ep_wrmem_add is null");
		return;
	}

	for (i = 0; i < size; i += REG_DWORD_ALIGN) {
		val = readl((void *)(uintptr_t)test_pcie->ep_rdmem_addr + offset + i);
		PCIE_PR_I("ep_rdmem::Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	for (i = 0; i < size; i += REG_DWORD_ALIGN) {
		val = readl((void *)(uintptr_t)test_pcie->ep_wrmem_addr + offset + i);
		PCIE_PR_I("ep_wrmem::Offset[0x%x], value=[0x%x]", i + offset, val);
	}
}

/*
 * rc_write_ep_mem - RC write EP mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Inbound region ID;
 */
int rc_write_ep_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	struct pcie_test_st *test_pcie = NULL;
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_pcie = &g_test_pcie[rc_id];
	pcie = test_pcie->pcie;

	if (!test_pcie->rc_wrmem_addr) {
		PCIE_PR_E("The test_pcie->rc_mem_addr is null");
		return -1;
	}

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	cpu_addr = (MEM_16M_ALIGN(pcie->pci->pp.cfg0_base) + TEST_MEM_PCI_BASE_OFFSET);

	pcie_kport_outbound_atu(rc_id, index, ATU_TYPE_MEM, cpu_addr, cpu_addr, TEST_MEM_SIZE);

	return data_trans_and_check(cpu_addr, size, test_pcie->rc_wrmem_addr,  0x56, MEM_WRITE);
}

enum data_cmp {
	RC_RD_WR_EP = 0,
	EP_RD_WR_RC = 1,
};

/*
 * data_trans_ok - Check if RC data EQU EP data.
 * @rc_id: Host ID;
 * @size: Data size;
 */
int data_trans_ok(u32 rc_id, unsigned int size, enum data_cmp flag)
{
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_pcie = &g_test_pcie[rc_id];

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	switch (flag) {
	case RC_RD_WR_EP:
		if ((!test_pcie->ep_wrmem_addr) || (!test_pcie->rc_rdmem_addr)) {
			PCIE_PR_E("Addr is NULL");
			return -1;
		}

		if (memcmp((void *)(uintptr_t)test_pcie->ep_wrmem_addr,
			   (void *)(uintptr_t)test_pcie->rc_rdmem_addr, size) != 0) {
			PCIE_PR_E("Transfer Failed");
			show_diff(test_pcie->rc_rdmem_addr, test_pcie->ep_wrmem_addr, size);
			return -1;
		}
		PCIE_PR_I("Transfer Ok");
		return 0;
	case EP_RD_WR_RC:
		if ((!test_pcie->ep_wrmem_addr) || (!test_pcie->rc_rdmem_addr)) {
			PCIE_PR_E("Addr is NULL");
			return -1;
		}

		if (memcmp((void *)(uintptr_t)test_pcie->ep_rdmem_addr,
			   (void *)(uintptr_t)test_pcie->rc_wrmem_addr, size) != 0) {
			PCIE_PR_E("Transfer Failed");
			show_diff(test_pcie->rc_wrmem_addr, test_pcie->ep_rdmem_addr, size);
			return -1;
		}
		PCIE_PR_I("Transfer Ok");
		return 0;
	default:
		PCIE_PR_E("Input invalid param");
		return 0;
	}
}

u64 pcie_set_mem_outbound(u32 rc_id, struct pci_dev *dev, int bar, u64 target)
{
	u64 bar_addr;
	u32 reg_val;

	if (!dev || !pcie_check_rcid(rc_id) || bar >= DEVICE_COUNT_RESOURCE || bar < 0) {
		PCIE_PR_E("%s, rcid:%u, bar:%d", __func__, rc_id, bar);
		return 0;
	}
	bar_addr = pci_resource_start(dev, bar);
	pcie_kport_outbound_atu(rc_id, 0, PCIE_ATU_TYPE_MEM, bar_addr, target, TEST_BUS1_OFFSET);
	PCIE_PR_I("%s, baraddr:0x%llx", __func__, bar_addr);

	reg_val = dword_low(target);
	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + 0x4 * bar, reg_val); /* dword size */
	reg_val = dword_high(target);
	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + 0x4 * (bar + 1), reg_val); /* write next dword size */

	return bar_addr;
}

u64 pcie_set_mem_outbound_test(u32 rc_id, int bar, u64 target)
{
	struct pcie_kport *pcie = NULL;
	struct pci_dev *pdev = NULL;

	if (!pcie_check_rcid(rc_id) || bar >= DEVICE_COUNT_RESOURCE || bar < 0) {
		PCIE_PR_E("%s, rcid:%u, bar:%d", __func__, rc_id, bar);
		return 0;
	}
	pcie = &g_pcie_port[rc_id];
	pdev = pcie->ep_dev;
	return pcie_set_mem_outbound(rc_id, pdev, bar, target);
}

void pcie_enable_device_test(u32 rc_id)
{
	struct pcie_kport *pcie = NULL;
	struct pci_dev *pdev = NULL;

	if (!pcie_check_rcid(rc_id)) {
		PCIE_PR_E("%s, rcid:%u", __func__, rc_id);
		return;
	}
	pcie = &g_pcie_port[rc_id];
	pdev = pcie->ep_dev;
	pci_enable_device(pdev);
	pci_set_master(pdev);
}

/*
 * set_rc_mem_inbound - Set RC DDR mem inbound for loopback data transfer.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
int set_rc_mem_inbound(u32 rc_id, int index)
{
	u64 busdev, temp_addr;
	int i;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_pcie = &g_test_pcie[rc_id];
	set_bme(rc_id, ENABLE);
	set_mse(rc_id, ENABLE);
	busdev = TEST_BUS0_OFFSET;

	if (!test_pcie->rc_inboundmem_addr) {
		PCIE_PR_E("The test_pcie->rc_mem_addr is null");
		return -1;
	}

	temp_addr = virt_to_phys((void *)(uintptr_t)test_pcie->rc_inboundmem_addr);

	PCIE_PR_I("Inbound pci_add [0x%pK] to cpu_addr[0x%pK]",
		     (void *)(uintptr_t)busdev, (void *)(uintptr_t)temp_addr);

	pcie_inbound_atu(rc_id, index, ATU_TYPE_MEM, temp_addr, busdev, TEST_MEM_SIZE);

	for (i = 0; i < TEST_MEM_SIZE; i += REG_DWORD_ALIGN)
		writel(RC_INBOUND_INIT_VAL, (void *)(uintptr_t)test_pcie->rc_inboundmem_addr + i);

	__flush_dcache_area((void *)(uintptr_t)test_pcie->rc_inboundmem_addr, TEST_MEM_SIZE);

	return 0;
}

/*
 * read_rc_addr - Read RC MEM value.
 * @rc_id: Host ID;
 * @offset: The offset which you want to read;
 * @size: Data size;
 */
int read_rc_addr(u32 rc_id, u32 offset, u32 size)
{
	u32 i, val;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	test_pcie = &g_test_pcie[rc_id];

	if (!test_pcie->rc_rdmem_addr) {
		PCIE_PR_E("The test_pcie->rc_rdmem_add is null");
		return -1;
	}

	if (!test_pcie->rc_wrmem_addr) {
		PCIE_PR_E("The test_pcie->rc_wrmem_add is null");
		return -1;
	}

	for (i = 0; i < size; i += REG_DWORD_ALIGN) {
		val = readl((void *)(uintptr_t)test_pcie->rc_rdmem_addr + offset + i);
		PCIE_PR_I("rc_rdmem: Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	for (i = 0; i < size; i += REG_DWORD_ALIGN) {
		val = readl((void *)(uintptr_t)test_pcie->rc_wrmem_addr + offset + i);
		PCIE_PR_I("rc_wrmem: Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	return 0;
}

/*
 * ep_read_rc_mem - EP read RC mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Outbound region ID;
 */
int ep_read_rc_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	struct resource *config = NULL;
	struct platform_device *pdev = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	test_pcie = &g_test_pcie[rc_id];
	if (!test_pcie->ep_rdmem_addr) {
		PCIE_PR_E("The test_pcie->ep_mem_addr is null");
		return -1;
	}

	pdev = to_platform_device(test_pcie->pcie->pci->dev);

	config = platform_get_resource_byname(pdev, IORESOURCE_MEM, "config");
	if (!config) {
		PCIE_PR_E("Failed to get config base");
		return -1;
	}

	cpu_addr = config->start + TEST_BUS0_OFFSET;

	pcie_kport_outbound_atu(rc_id, index, ATU_TYPE_MEM, cpu_addr, TEST_BUS0_OFFSET, TEST_MEM_SIZE);

	return data_trans_and_check(cpu_addr, size, test_pcie->ep_rdmem_addr, 0x0, MEM_READ);
}

/*
 * ep_write_rc_mem - EP write RC mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Outbound region ID;
 */
int ep_write_rc_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	struct resource *config = NULL;
	struct platform_device *pdev = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size <= 0)
		size = TEST_MEM_SIZE;

	test_pcie = &g_test_pcie[rc_id];
	if (!test_pcie->ep_wrmem_addr) {
		PCIE_PR_E("The test_pcie->ep_mem_addr is null");
		return -1;
	}

	pdev = to_platform_device(test_pcie->pcie->pci->dev);

	config = platform_get_resource_byname(pdev, IORESOURCE_MEM, "config");
	if (!config) {
		PCIE_PR_E("Failed to get config base");
		return -1;
	}

	cpu_addr = config->start + TEST_BUS0_OFFSET;

	pcie_kport_outbound_atu(rc_id, index, ATU_TYPE_MEM, cpu_addr, TEST_BUS0_OFFSET, TEST_MEM_SIZE);

	return data_trans_and_check(cpu_addr, size, test_pcie->ep_wrmem_addr, 0xDE, MEM_WRITE);
}

/*
 * test_host_power_control - Power on host and scan bus.
 * @rc_id: Host ID;
 * @flag: RC power status;
 */
int test_host_power_control(u32 rc_id, u32 flag)
{
	struct pcie_kport *pcie = NULL;
	int ret;

	if (rc_id >= g_rc_num) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	pcie = g_test_pcie[rc_id].pcie;

	if (!pcie) {
		PCIE_PR_E("PCIe%u is null", rc_id);
		return -EINVAL;
	}

	ret = pcie_power_ctrl(pcie, (enum rc_power_status)flag);

	return ret;
}

/*
 * pcie_ep_mac_init - EP mac initialization.
 * @id: PCIe ID;
 */
int pcie_ep_mac_init(u32 pcie_id)
{
	struct pcie_kport *pcie = NULL;
	u32 val;

	if (pcie_id >= g_rc_num) {
		PCIE_PR_E("There is no pcie_id = %u", pcie_id);
		return -EINVAL;
	}

	pcie = &g_pcie_port[pcie_id];

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_E("PCIe%u is power off", pcie_id);
		return -EINVAL;
	}

	/* change EP device ID */
	val = PCIE_VENDOR_ID_HUAWEI + (pcie_id << PCIE_DEV_ID_SHIFT);

	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, 0x0, REG_DWORD_ALIGN, val);
	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, 0x0, REG_DWORD_ALIGN);
	PCIE_PR_I("DevID&VendorID is [0x%x]", val);

	/* Modify EP device class from 0x0(unsupport) to net devices */
	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_CLASS_REVISION, REG_DWORD_ALIGN);
	val |= (PCI_CLASS_NETWORK_OTHER << PCI_CLASS_CODE_SHIFT);
	pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_CLASS_REVISION, REG_DWORD_ALIGN, val);

	val = pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_CLASS_REVISION, REG_DWORD_ALIGN);
	PCIE_PR_I("Device class is [0x%x]", val);

	ltssm_enable(pcie_id, ENABLE);
	PCIE_PR_I("Enable Link Training");

	set_bme(pcie_id, ENABLE);
	set_mse(pcie_id, ENABLE);
	PCIE_PR_I("Enable bus master and memory decode");

	/* disable EP change configuration space */
	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL6_ADDR);
	val |= APP_DBI_RO_WR_DISABLE;
	pcie_apb_ctrl_writel(pcie, val, SOC_PCIECTRL_CTRL6_ADDR);

	enable_req_clk(pcie, DISABLE);

	if (pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, ATU_VIEWPORT, REG_DWORD_ALIGN) == 0xffffffff)
		pcie->pci->iatu_unroll_enabled = 1;
	else
		pcie->pci->iatu_unroll_enabled = 0;

	return 0;
}

static u32 pcie_find_capability(struct pcie_port *pp, int cap)
{
	u8 id;
	int ttl = 0x30;
	u32 pos = 0;
	u32 ent = 0;

	pcie_rd_own_conf(pp, PCI_CAPABILITY_LIST, REG_BYTE_ALIGN, &pos);

	/* bit[1~0]:capability-id, bit[15~8]:offset */
	while (ttl--) {
		if (pos < 0x40)
			break;
		pos &= ~3;
		pcie_rd_own_conf(pp, pos, REG_WORD_ALIGN, &ent);
		id = ent & 0xff;
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos = (ent >> 8);
	}
	return 0;
}

int retrain_link(u32 rc_id)
{
	u32 val = 0;
	u32 cap_pos;
	unsigned long start_jiffies;
	struct pcie_port *pp = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pp = &(g_pcie_port[rc_id].pci->pp);

	cap_pos = pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL, REG_DWORD_ALIGN, &val);
	/* Retrain link */
	val |= PCI_EXP_LNKCTL_RL;
	pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL, REG_DWORD_ALIGN, val);

	/* Wait for link training end. Break out after waiting for timeout */
	start_jiffies = jiffies;
	for (;;) {
		pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKSTA, REG_DWORD_ALIGN, &val);
		val &= 0xffff;
		if (!(val & PCI_EXP_LNKSTA_LT))
			break;
		if (time_after(jiffies, start_jiffies + HZ))
			break;
		msleep(1);
	}
	if (!(val & PCI_EXP_LNKSTA_LT))
		return 0;
	return -1;
}

int set_link_speed(u32 rc_id, enum link_speed gen)
{
	u32 val = 0x1;
	u32 reg_val = 0x0;
	int ret = 0;
	u32 cap_pos;
	struct pcie_port *pp = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pp = &(g_pcie_port[rc_id].pci->pp);

	/* link speed:gen1(0x1) gen2(0x2) gen3(0x3) */
	switch (gen) {
	case GEN1:
		val = 0x1;
		break;
	case GEN2:
		val = 0x2;
		break;
	case GEN3:
		val = 0x3;
		break;
	default:
		ret = -1;
	}

	cap_pos = pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	/* PCIE_CAP_TARGET_LINK_SPEED bit[3~0] */
	pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, REG_DWORD_ALIGN, &reg_val);
	reg_val &= ~(0x3 << 0);
	reg_val |= val;
	pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, REG_DWORD_ALIGN, reg_val);

	if (!ret)
		return retrain_link(rc_id);
	return ret;
}

/*
 * test_compliance - Set compliance for test.
 * @rc_id: Host ID;
 * @entry: 0 -- exit compliance; others -- entry compliance;
 */
int test_compliance(u32 rc_id, u32 entry)
{
	u32 cap_pos;
	u32 val = 0;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pp = &(g_test_pcie[rc_id].pcie->pci->pp);
	cap_pos = pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	ltssm_enable(rc_id, DISABLE);
	pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, REG_DWORD_ALIGN, &val);
	if (entry)
		val |= ENTER_COMPLIANCE;
	else
		val &= ~ENTER_COMPLIANCE;
	pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, REG_DWORD_ALIGN, val);

	ltssm_enable(rc_id, ENABLE);

	return 0;
}

/*
 * test_entry_loopback - Set entry loopabck for test.
 * @rc_id: Host ID;
 * @local: 0 -- remote loopback; others -- local loopback;
 */
void test_entry_loopback(u32 rc_id, u32 local)
{
	u32 val = 0;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pp = &(g_test_pcie[rc_id].pcie->pci->pp);

	pcie_rd_own_conf(pp, PORT_LINK_CTRL_REG, REG_DWORD_ALIGN, &val);
	val |= LOOPBACK_ENABLE;
	pcie_wr_own_conf(pp, PORT_LINK_CTRL_REG, REG_DWORD_ALIGN, val);

	if (local) {
		pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, REG_DWORD_ALIGN, &val);
		val |= EQUALIZATION_DISABLE;
		pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, REG_DWORD_ALIGN, val);

		pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, REG_DWORD_ALIGN, &val);
		val |= PIPELOOPBACK_ENABLE;
		pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, REG_DWORD_ALIGN, val);
	}
}

/*
 * test_exit_loopback - Set exit loopabck.
 * @rc_id: Host ID;
 * @local: 0 -- remote loopback; others -- local loopback;
 */
void test_exit_loopback(u32 rc_id, u32 local)
{
	u32 val = 0;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pp = &(g_test_pcie[rc_id].pcie->pci->pp);

	pcie_rd_own_conf(pp, PORT_LINK_CTRL_REG, REG_DWORD_ALIGN, &val);
	val &= ~LOOPBACK_ENABLE;
	pcie_wr_own_conf(pp, PORT_LINK_CTRL_REG, REG_DWORD_ALIGN, val);

	if (local) {
		pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, REG_DWORD_ALIGN, &val);
		val &= ~EQUALIZATION_DISABLE;
		pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, REG_DWORD_ALIGN, val);

		pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, REG_DWORD_ALIGN, &val);
		val &= ~PIPELOOPBACK_ENABLE;
		pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, REG_DWORD_ALIGN, val);
	}
}

/*
 * phy_internal_loopback - Set phy internal loopback for test.
 * @rc_id: Host ID;
 * @dir: 0 -- TX2RX; 1 -- RX2TX;
 */
int phy_internal_loopback(u32 rc_id, enum phy_lb_dir dir, enum link_speed gen)
{
	u32 val = 0;
	struct pcie_kport *pcie = NULL;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_pcie[rc_id].pcie;
	pp = &(g_test_pcie[rc_id].pcie->pci->pp);

	set_link_speed(rc_id, gen);

	if (dir == TX2RX)
		pcie_natural_phy_writel(pcie, 0x5, 0x1000);
	else
		pcie_natural_phy_writel(pcie, 0x6, 0x1000);

	if (gen == GEN3) {
		pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, REG_DWORD_ALIGN, &val);
		val |= EQUALIZATION_DISABLE;
		pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, REG_DWORD_ALIGN, val);

		pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, REG_DWORD_ALIGN, &val);
		val |= PIPELOOPBACK_ENABLE;
		pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, REG_DWORD_ALIGN, val);
	}

	return 0;
}

/*
 * ep_triggle_intr - EP triggle MSI interrupt.
 * @rc_id: Host ID;
 */
void ep_triggle_intr(u32 rc_id)
{
	u32 value_temp;
	struct pcie_kport *pcie = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_pcie = &g_test_pcie[rc_id];
	pcie = test_pcie->pcie;
	value_temp = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL2_ADDR);
	value_temp |= PCIE_VEN_MSI_REQ;
	pcie_apb_ctrl_writel(pcie, value_temp, SOC_PCIECTRL_CTRL2_ADDR);
	udelay(2); /* EP triger interrput then disable 2us latter */

	value_temp &= ~PCIE_VEN_MSI_REQ;
	pcie_apb_ctrl_writel(pcie, value_temp, SOC_PCIECTRL_CTRL2_ADDR);
	PCIE_PR_I("Read after write, ctl2 is %x", pcie_apb_ctrl_readl(pcie,
		     SOC_PCIECTRL_CTRL2_ADDR));
	PCIE_PR_I("ep_triggle_msi_intr");
}

/*
 * ep_triggle_inta_intr - EP triggle INTa interrupt.
 * @rc_id: Host ID;
 */
void ep_triggle_inta_intr(u32 rc_id)
{
	u32 value_temp;
	struct pcie_kport *pcie = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_pcie = &g_test_pcie[rc_id];
	pcie = test_pcie->pcie;
	value_temp = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	value_temp |= PCIE_SYS_INT;
	pcie_apb_ctrl_writel(pcie, value_temp, SOC_PCIECTRL_CTRL7_ADDR);

	value_temp = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	PCIE_PR_I("read after write, ctl7 is %x", value_temp);
	PCIE_PR_I("ep_triggle_inta_intr");
}

/*
 * ep_clr_inta_intr - EP clear INTa interrupt.
 * @rc_id: Host ID;
 */
void ep_clr_inta_intr(u32 rc_id)
{
	u32 value_temp;
	struct pcie_kport *pcie = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_pcie = &g_test_pcie[rc_id];
	pcie = test_pcie->pcie;
	value_temp = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	value_temp &= ~PCIE_SYS_INT;
	pcie_apb_ctrl_writel(pcie, value_temp, SOC_PCIECTRL_CTRL7_ADDR);

	value_temp = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	PCIE_PR_I("read after write, ctl7 is %x", value_temp);
	PCIE_PR_I("ep_clr_inta_intr");
}

/*
 * generate_msg - Generate MSG.
 * @rc_id: Host ID;
 * @msg_code: MSG code;
 */
void generate_msg(u32 rc_id, u32 msg_code)
{
	int msg_type;
	u32 iatu_offset;
	struct pcie_test_st *test_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_pcie = &g_test_pcie[rc_id];
	iatu_offset = test_pcie->pcie->dtsinfo.iatu_base_offset;

	if (msg_code == MSG_CODE_ASSERT_INTA ||
	    msg_code == MSG_CODE_ASSERT_INTB ||
	    msg_code == MSG_CODE_DEASSERT_INTA ||
	    msg_code == MSG_CODE_DEASSERT_INTB) {
		PCIE_PR_I("INTA and INTB is not supported");
		return;
	} else if (msg_code == MSG_CODE_PME_TURN_OFF) {
		msg_type = MSG_TYPE_ROUTE_BROADCAST;
	} else {
		msg_type = ATU_TYPE_MSG;
	}
	(void)pcie_generate_msg(rc_id, ATU_REGION_INDEX0,
				iatu_offset, msg_type, msg_code);
}

/*
 * msg_triggle_clr - Triggle or clear MSG.
 * @rc_id: Host ID;
 * @offset: Register offset;
 * @bit: Register bit;
 */
void msg_triggle_clr(u32 rc_id, u32 offset, u32 bit)
{
	u32 value_temp;
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pcie = g_test_pcie[rc_id].pcie;

	value_temp = pcie_apb_ctrl_readl(pcie, offset);
	value_temp &= ~(0x1 << bit);
	pcie_apb_ctrl_writel(pcie, value_temp, offset);
	value_temp |= (0x1 << bit);
	pcie_apb_ctrl_writel(pcie, value_temp, offset);
}

/*
 * msg_received - Whether received MSG or not.
 * @rc_id: Host ID;
 * @offset: Register offset;
 * @bit: Register bit;
 */
int msg_received(u32 rc_id, u32 offset, u32 bit)
{
	u32 value_temp;
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_pcie[rc_id].pcie;
	value_temp = pcie_apb_ctrl_readl(pcie, offset);
	if (value_temp & (0x1 << bit))
		return 1;
	else
		return 0;
}

/*
 * pcie_enable_msg_num - Enable MSG function,include LTR and OBFF.
 * @rc_id: Host ID;
 * @num: 13 -- OBFF; 10 -- LTR; others -- not msg function
 * @local: 0 -- Set EP dev; others -- Set RC dev;
 */
void pcie_enable_msg_num(u32 rc_id, int num, int local)
{
	u32 val;
	struct pci_dev *dev = NULL;
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pcie = g_test_pcie[rc_id].pcie;

	if (local)
		dev = pcie->rc_dev;
	else
		dev = pcie->ep_dev;

	pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &val);

	switch (num) {
	case 13: /* OBFF */
		PCIE_PR_I("Enable obff message");
		val |= PCI_EXP_DEVCTL2_OBFF_MSGA_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, val);
		break;
	case 10: /* LTR */
		PCIE_PR_I("Enable LTR message");
		val |= PCI_EXP_DEVCTL2_LTR_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, val);
		break;
	default:
		PCIE_PR_I("Unsupport function");
		break;
	}
}

static void get_pcie_credit_info(struct pcie_kport *pcie, u32 type, u32 *header, u32 *data)
{
	u32 val;

	if (type == TYPE_POST) {
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE12_ADDR);
	} else if (type == TYPE_NONPOST) {
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE13_ADDR);
	} else if (type == TYPE_CPL) {
		val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE14_ADDR);
	} else {
		PCIE_PR_E("Invalid type parameter");
		return;
	}

	*header = (val & PCIE_CREDIT_HEADER_MASK) >> PCIE_CREDIT_HEADER_OFFS;
	*data = val & PCIE_CREDIT_DATA_MASK;
}

void print_credit_info(u32 rc_id)
{
	u32 val, header, data;
	struct pcie_kport *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pcie = g_test_pcie[rc_id].pcie;
	get_pcie_credit_info(pcie, TYPE_POST, &header, &data);
	PCIE_PR_I("POST Cre_dit info:	Header:%u	Data:%u", header, data);
	get_pcie_credit_info(pcie, TYPE_NONPOST, &header, &data);
	PCIE_PR_I("Non-POST Cre_dit info:	Header:%u	Data:%u", header, data);
	get_pcie_credit_info(pcie, TYPE_CPL, &header, &data);
	PCIE_PR_I("Completion Cre_dit info:	Header:%u	Data:%u", header, data);

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE15_ADDR);
	if (val & NO_PENDING_DLLP)
		PCIE_PR_I("There is no pending DLLP");
	else
		PCIE_PR_I("There is pending DLLP(s)");

	if (val & NO_EXPECTING_ACK)
		PCIE_PR_I("There is no expecting ACK");
	else
		PCIE_PR_I("There is expecting ACK(s)");

	if (val & HAD_ENOUGH_CREDIT)
		PCIE_PR_I("All types have enough credits");
	else
		PCIE_PR_I("Some type doesn't have enough credits");

	if (val & NO_PENDING_TLP)
		PCIE_PR_I("There is no pending TLP");
	else
		PCIE_PR_I("There is pending TLP");

	if (val & NO_FC)
		PCIE_PR_I("Some type has no credit");
	else
		PCIE_PR_I("All types have credit");
}

int show_link_speed(u32 rc_id)
{
	unsigned int val;
	struct pcie_kport *pcie = NULL;

	if (!is_pcie_pwr_on(rc_id))
		return -1;

	pcie = &g_pcie_port[rc_id];

	val = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);
	val = val & 0xc0;
	switch (val) {
	case 0x0:
		PCIE_PR_I("Link speed: gen1");
		break;
	case 0x40:
		PCIE_PR_I("Link speed: gen2");
		break;
	case 0x80:
		PCIE_PR_I("Link speed: gen3");
		break;
	default:
		PCIE_PR_I("Link speed info unknow");
	}

	return val;
}

int pcie_set_ep_mode(u32 rc_id)
{
	int ret;
	struct pcie_kport *pcie = NULL;
	struct platform_device *pdev = NULL;

	PCIE_PR_I("+[%u]+", rc_id);

	if (rc_id >= g_rc_num) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	pcie = &g_pcie_port[rc_id];

	pdev = to_platform_device(pcie->pci->dev);

	PCIE_PR_I("set PCIe[%u] in EP mode", rc_id);
	pcie->dtsinfo.ep_flag = 1;

	ret = pcie_kport_power_notifiy_register(pcie->rc_id, NULL, NULL, NULL);
	if (ret)
		PCIE_PR_I("Failed to register NULL");

	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_MSI].num, (void *)pcie);
	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_LINKDOWN].num, (void *)pcie);

#ifdef CONFIG_PCIE_KPORT_CPLTIMEOUT_INT
	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_CPLTIMEOUT].num, (void *)pcie);
#endif

#ifdef CONFIG_PCIE_KPORT_IDLE
	if (pcie->idle_sleep) {
		devm_free_irq(&pdev->dev, pcie->idle_sleep->irq[0], (void *)pcie);
		devm_free_irq(&pdev->dev, pcie->idle_sleep->irq[1], (void *)pcie);
	}
#endif

	PCIE_PR_I("-[%u]-", rc_id);
	return ret;
}

static int pcie_test_alloc_mem(struct pcie_test_st *test_pcie)
{
	const u32 order = get_order(TEST_MEM_SIZE);

	/*
	 * Check one memory address is enough
	 * to see if memories are alreay allocated
	 */
	if (test_pcie->rc_wrmem_addr != 0) {
		PCIE_PR_E("Memories alloced already done!");
		return 0;
	}

	test_pcie->rc_wrmem_addr = __get_free_pages(GFP_KERNEL, order);
	if (test_pcie->rc_wrmem_addr == 0) {
		PCIE_PR_E("Failed to alloc RC write memory");
		goto RC_WR_MEM_FAIL;
	}
	PCIE_PR_I("rc_wrmem_addr is 0x%pK", (void *)(uintptr_t)test_pcie->rc_wrmem_addr);

	test_pcie->rc_rdmem_addr = __get_free_pages(GFP_KERNEL, order);
	if (test_pcie->rc_rdmem_addr == 0) {
		PCIE_PR_E("Failed to alloc RC read memory");
		goto RC_RD_MEM_FAIL;
	}
	PCIE_PR_I("rc_rdmem_addr is 0x%pK", (void *)(uintptr_t)test_pcie->rc_rdmem_addr);

	test_pcie->ep_wrmem_addr = __get_free_pages(GFP_KERNEL, order);
	if (test_pcie->ep_wrmem_addr == 0) {
		PCIE_PR_E("Failed to alloc EP write memory");
		goto EP_WR_MEM_FAIL;
	}
	PCIE_PR_I("ep_wrmem_addr is 0x%pK", (void *)(uintptr_t)test_pcie->ep_wrmem_addr);

	test_pcie->ep_rdmem_addr = __get_free_pages(GFP_KERNEL, order);
	if (test_pcie->ep_rdmem_addr == 0) {
		PCIE_PR_E("Failed to alloc EP read memory");
		goto EP_RD_MEM_FAIL;
	}
	PCIE_PR_I("ep_rdmem_addr is 0x%pK", (void *)(uintptr_t)test_pcie->ep_rdmem_addr);

	test_pcie->rc_inboundmem_addr = __get_free_pages(GFP_KERNEL, order);
	if (test_pcie->rc_inboundmem_addr == 0) {
		PCIE_PR_E("Failed to alloc rc_inbound memory");
		goto RC_INBOUND_MEM_FAIL;
	}
	PCIE_PR_I("rc_inboundmem_addr is 0x%pK", (void *)(uintptr_t)test_pcie->rc_inboundmem_addr);

	test_pcie->ep_inboundmem_addr = __get_free_pages(GFP_KERNEL, order);
	if (test_pcie->ep_inboundmem_addr == 0) {
		PCIE_PR_E("Failed to alloc ep_inbound memory");
		goto EP_INBOUND_MEM_FAIL;
	}
	PCIE_PR_I("ep_inboundmem_addr is 0x%pK", (void *)(uintptr_t)test_pcie->ep_inboundmem_addr);

	return 0;

EP_INBOUND_MEM_FAIL:
	free_pages(test_pcie->rc_inboundmem_addr, order);
	test_pcie->rc_inboundmem_addr = 0;
RC_INBOUND_MEM_FAIL:
	free_pages(test_pcie->ep_rdmem_addr, order);
	test_pcie->ep_rdmem_addr = 0;
EP_RD_MEM_FAIL:
	free_pages(test_pcie->ep_wrmem_addr, order);
	test_pcie->ep_wrmem_addr = 0;
EP_WR_MEM_FAIL:
	free_pages(test_pcie->rc_rdmem_addr, order);
	test_pcie->rc_rdmem_addr = 0;
RC_RD_MEM_FAIL:
	free_pages(test_pcie->rc_wrmem_addr, order);
	test_pcie->rc_wrmem_addr = 0;
RC_WR_MEM_FAIL:
	return -ENOMEM;
}

int pcie_test_init(u32 rc_id)
{
	struct pcie_kport *pcie = NULL;
	struct pcie_test_st *test_pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	pcie = &g_pcie_port[rc_id];
	test_pcie = &g_test_pcie[rc_id];
	test_pcie->pcie = pcie;

	return pcie_test_alloc_mem(test_pcie);
}

int pcie_test_exist(u32 rc_id)
{
	struct pcie_test_st *test_pcie = NULL;
	const u32 order = get_order(TEST_MEM_SIZE);

	if (rc_id >= g_rc_num) {
		PCIE_PR_E("There is no rc_id = %u", rc_id);
		return -EINVAL;
	}

	test_pcie = &g_test_pcie[rc_id];
	if (test_pcie->rc_rdmem_addr) {
		free_pages(test_pcie->rc_rdmem_addr, order);
		test_pcie->rc_rdmem_addr = 0;
	}

	if (test_pcie->rc_wrmem_addr) {
		free_pages(test_pcie->rc_wrmem_addr, order);
		test_pcie->rc_wrmem_addr = 0;
	}

	if (test_pcie->ep_rdmem_addr) {
		free_pages(test_pcie->ep_rdmem_addr, order);
		test_pcie->ep_rdmem_addr = 0;
	}

	if (test_pcie->ep_wrmem_addr) {
		free_pages(test_pcie->ep_wrmem_addr, order);
		test_pcie->ep_wrmem_addr = 0;
	}

	if (test_pcie->rc_inboundmem_addr) {
		free_pages(test_pcie->rc_inboundmem_addr, order);
		test_pcie->rc_inboundmem_addr = 0;
	}

	if (test_pcie->ep_inboundmem_addr) {
		free_pages(test_pcie->ep_inboundmem_addr, order);
		test_pcie->ep_inboundmem_addr = 0;
	}

	test_pcie->pcie = NULL;

	return 0;
}

const struct dw_pcie_ops dw_pcie_ep_ops = {
	.read_dbi  = pcie_read_dbi,
	.write_dbi = pcie_write_dbi,
};

static int pcie_ep_probe(struct platform_device *pdev)
{
	struct pcie_kport *pcie = NULL;
	struct dw_pcie *pci = NULL;
	int ret;

	PCIE_PR_I("+%s+", __func__);

	ret = pcie_get_port(&pcie, pdev);
	if (ret) {
		PCIE_PR_E("Failed to get pcie from dts");
		return ret;
	}

	PCIE_PR_I("PCIe EP No.%u probe", pcie->rc_id);

	pci = devm_kzalloc(&pdev->dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;
	pcie->pci = pci;

	ret = pcie_get_dtsinfo(pcie, pdev);
	if (ret) {
		PCIE_PR_E("Failed to get dts info");
		goto FAIL;
	}

	pcie->pci->dev = &(pdev->dev);
	pcie->pci->ops = &dw_pcie_ep_ops;

	platform_set_drvdata(pdev, pcie);

	ret = pcie_plat_init(pdev, pcie);
	if (ret) {
		PCIE_PR_E("Failed to get platform info");
		goto FAIL;
	}

	ret = pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_E("Failed to power on EP");
		goto FAIL;
	}

	ret = pcie_ep_mac_init(pcie->rc_id);
	if (ret) {
		PCIE_PR_E("Failed to initialize EP MAC");
		goto FAIL;
	}

	atomic_set(&(pcie->is_ready), 1);
	spin_lock_init(&pcie->ep_ltssm_lock);
	mutex_init(&pcie->power_lock);
	mutex_init(&pcie->pm_lock);

	PCIE_PR_I("-%s-", __func__);
	return 0;
FAIL:
	devm_kfree(&pdev->dev, pcie->pci);

	return ret;
}

static const struct of_device_id pcie_ep_match_table[] = {
	{
		.compatible = "pcie-kport,ep",
		.data = NULL,
	},
	{},
};

static struct platform_driver pcie_ep_driver = {
	.driver = {
		.name           = "pcie-kport-ep",
		.owner          = THIS_MODULE,
		.of_match_table = pcie_ep_match_table,
	},
	.probe = pcie_ep_probe,
};

/*
 * pcie_ep_init -struct pcie ep device init.
 * @rc_id: Host ID;
 * @num: 13 -- OBFF; 10 -- LTR; others -- not msg function
 * @local: 0 -- Set EP dev; others -- Set RC dev;
 */
static int __init pcie_ep_init(void)
{
	int ret;

	ret = platform_driver_register(&pcie_ep_driver);
	if (ret) {
		PCIE_PR_E("Failed to register pcie ep driver");
		return ret;
	}
	return 0;
}

device_initcall(pcie_ep_init);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PCIe kport test driver");
