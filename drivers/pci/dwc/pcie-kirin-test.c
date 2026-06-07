/*
 * PCIe host controller driver for Kirin SoCs
 *
 * Copyright (C) 2015 Huawei Electronics Co., Ltd.
 *		http://www.huawei.com
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "pcie-kirin-common.h"
#include <linux/pci-aspm.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <asm/memory.h>
#include <asm/cacheflush.h>
#include <securec.h>

#define PCI_CONFIG_SPACE		0x1000
#define TEST_MEM_SIZE			0x100000
#define TEST_MEM_PCI_BASE_OFFSET	0x2000000
#define TRANSFER_TIMES			1
#define MAX_RC_NUM			2
#define CTRLBUS_TIMEOUT_ENABLE		1

enum phy_lb_dir {
	TX2RX = 0,
	RX2TX = 1,
};

struct pcie_test_st {
	struct kirin_pcie *pcie;
	u64 rc_rdmem_addr;
	u64 rc_wrmem_addr;
	u64 ep_rdmem_addr;
	u64 ep_wrmem_addr;
	u64 rc_inboundmem_addr;
	u64 ep_inboundmem_addr;
};

struct pcie_test_st g_test_kirin_pcie[MAX_RC_NUM];

/*
 * check_pcie_on_work - Check if the parameters are valid and host is working.
 * @rc_id: Host ID;
 */
int check_pcie_on_work(u32 rc_id)
{
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->pcie) {
		PCIE_PR_ERR("PCIe%d is null", rc_id);
		return -EINVAL;
	}

	if (!atomic_read(&(test_kirin_pcie->pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe%d is power off", rc_id);
		return -EINVAL;
	}

	return 0;
}

/*
 * show_trans_rate - Show data transmission rate.
 * @begin_time: The time before transmission;
 * @end_time: The time after transmission;
 * @size: The size of datas.
 */
long int show_trans_rate(u64 begin_time, u64 end_time, u64 size)
{
	u64 time_count = end_time - begin_time;
	long int rate = ((size * sizeof(char) * 8 * 100 * TRANSFER_TIMES)/time_count);

	PCIE_PR_INFO("Data size [%lld], time sapce [%lld]",
		(size * sizeof(char) * 8 * 100 * TRANSFER_TIMES), time_count);
	PCIE_PR_INFO("Time sapce [%lld]. Transferring Rate is :[%ld] Bit/s", time_count, rate);

	return rate;
}

/*
 * show_pci_dev_state - Show rc and ep dev state.
 * @rc_id: Host ID;
 */
void show_pci_dev_state(u32 rc_id)
{
	int pm;
	u32 reg_value;
	struct kirin_pcie *pcie = NULL;
	struct pci_dev *ep_dev = NULL;
	struct pci_dev *rc_dev = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	ep_dev = pcie->ep_dev;
	rc_dev = pcie->rc_dev;

	if ((!rc_dev) || (!ep_dev)) {
		PCIE_PR_ERR("Failed to get Device");
		return;
	}

	pm = pci_find_capability(rc_dev, PCI_CAP_ID_PM);
	if (pm) {
		kirin_pcie_rd_own_conf(&(pcie->pci->pp), pm + PCI_PM_CTRL, 4, &reg_value);
		PCIE_PR_INFO("RC: pci_pm_ctrl reg value = [0x%x]", reg_value & 0x3);
	}

	pm = pci_find_capability(ep_dev, PCI_CAP_ID_PM);
	if (pm) {
		pci_read_config_dword(ep_dev, pm + PCI_PM_CTRL, &reg_value);
		PCIE_PR_INFO("EP: pci_pm_ctrl reg value = [0x%x]", reg_value & 0x3);
	}
}

static void kirin_pcie_atu_disable(struct kirin_pcie *pcie, u32 index, u32 direct)
{
	u32 iatu_offset = pcie->dtsinfo.iatu_base_offset;

	if (iatu_offset != KIRIN_PCIE_ATU_VIEWPORT) {
		iatu_offset += index * 0x200;
		if (direct & KIRIN_PCIE_ATU_REGION_INBOUND)
			iatu_offset += INBOUNT_OFFSET;
	} else
		kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
				KIRIN_PCIE_ATU_VIEWPORT, 0x4, direct | index);

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base,
			iatu_offset + KIRIN_PCIE_ATU_CR2, 0x4, 0x0);
}

/*
 * disable_outbound_iatu - Disable outbound iatu region.
 * @rc_id: Host ID;
 * @index: Outbound region ID;
 */
void disable_outbound_iatu(u32 rc_id, int index)
{
	struct kirin_pcie *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];
	kirin_pcie_atu_disable(pcie, (u32)index, KIRIN_PCIE_ATU_REGION_OUTBOUND);
}

/*
 * disable_inbound_iatu - Disable inbound iatu region.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
void disable_inbound_iatu(u32 rc_id, int index)
{
	struct kirin_pcie *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pcie = &g_kirin_pcie[rc_id];
	kirin_pcie_atu_disable(pcie, (u32)index, KIRIN_PCIE_ATU_REGION_INBOUND);
}

u64 g_dma_begin_time;
u64 g_dma_end_time;

struct dmatest_done {
	bool			done;
	wait_queue_head_t	*wait;
};

static void dmatest_callback(void *arg)
{
	struct dmatest_done *done = (struct dmatest_done *)arg;

	g_dma_end_time = hisi_getcurtime();
	PCIE_PR_INFO("Dmatest callback, begin_time = %llu,  end_time = %llu",
		g_dma_begin_time, g_dma_end_time);
	done->done = true;
	wake_up_all(done->wait);
}

/*
 * rc_read_ep_by_dma - RC read EP mem by dma.
 * @addr: RC mem address;
 * @size: Data size;
 * @rc_id: Host ID;
 */
int rc_read_ep_by_dma(u64 addr, u32 size, u32 rc_id)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(done_wait);
	dma_cap_mask_t mask;
	struct dma_chan *chan = NULL;
	struct dma_device *dev = NULL;
	unsigned long flags;
	enum dma_status status;
	dma_cookie_t cookie;
	struct dmatest_done done = { .wait = &done_wait };
	struct dma_async_tx_descriptor *tx = NULL;
	u64 rc_phys_addr, ep_in_rc_cpu_phys_addr;
	u64 wait_jiffies = msecs_to_jiffies(20000);
	struct kirin_pcie *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_kirin_pcie[rc_id].pcie;

	rc_phys_addr = addr;
	ep_in_rc_cpu_phys_addr = (pcie->dtsinfo.dbi_phy_base + TEST_MEM_PCI_BASE_OFFSET);
	flags = (unsigned long)(DMA_CTRL_ACK | DMA_PREP_INTERRUPT);

	kirin_pcie_outbound_atu(rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, KIRIN_PCIE_ATU_TYPE_MEM,
		ep_in_rc_cpu_phys_addr, ep_in_rc_cpu_phys_addr, size);

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	chan = dma_request_channel(mask, NULL, NULL);
	if (chan) {
		dev = chan->device;
		tx = dev->device_prep_dma_memcpy(chan, rc_phys_addr, ep_in_rc_cpu_phys_addr,
			size, flags);
		if (!tx) {
			PCIE_PR_ERR("Tx is null");
			goto release_dma_channel;
		}
		done.done = false;
		tx->callback = dmatest_callback;
		tx->callback_param = &done;
		cookie = tx->tx_submit(tx);

		if (dma_submit_error(cookie)) {
			PCIE_PR_ERR("DMA submit error");
			goto release_dma_channel;
		}
		g_dma_begin_time = hisi_getcurtime();
		dma_async_issue_pending(chan);
		if (wait_event_freezable_timeout(done_wait, done.done, wait_jiffies)) {
			PCIE_PR_ERR("Fail, wait event freezable timeout");
			goto release_dma_channel;
		}
		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
		if (status != DMA_COMPLETE) {
			PCIE_PR_ERR("DMA transfer fail, dmatest_error_type = %d", status);
			goto release_dma_channel;
		}
	} else {
		PCIE_PR_ERR("Failed to get dma channel");
		return -1;
	}
	dma_release_channel(chan);
	return 0;
release_dma_channel:
	dma_release_channel(chan);
	return -1;
}

/*
 * rc_write_ep_by_dma - RC write EP mem by dma.
 * @addr: RC mem address;
 * @size: Data size;
 * @rc_id: Host ID;
 */
int rc_write_ep_by_dma(u64 addr, u32 size, u32 rc_id)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(done_wait);
	dma_cap_mask_t mask;
	struct kirin_pcie *pcie = NULL;
	struct dma_chan *chan = NULL;
	struct dma_device *dev = NULL;
	unsigned long flags;
	enum dma_status status;
	dma_cookie_t cookie;
	struct dmatest_done done = { .wait = &done_wait };
	struct dma_async_tx_descriptor *rx = NULL;
	u64 rc_phys_addr, ep_in_rc_cpu_phys_addr;
	u64 wait_jiffies = msecs_to_jiffies(20000);

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_kirin_pcie[rc_id].pcie;

	rc_phys_addr = addr;
	ep_in_rc_cpu_phys_addr = (pcie->dtsinfo.dbi_phy_base + TEST_MEM_PCI_BASE_OFFSET);

	kirin_pcie_outbound_atu(rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, KIRIN_PCIE_ATU_TYPE_MEM,
		ep_in_rc_cpu_phys_addr, ep_in_rc_cpu_phys_addr, size);

	flags = (unsigned long)(DMA_CTRL_ACK | DMA_PREP_INTERRUPT);

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	chan = dma_request_channel(mask, NULL, NULL);
	if (chan) {
		dev = chan->device;
		rx = dev->device_prep_dma_memcpy(chan, ep_in_rc_cpu_phys_addr,
				rc_phys_addr, size, flags);
		if (!rx) {
			PCIE_PR_ERR("Rx is null");
			goto release_dma_channel;
		}
		done.done = false;
		rx->callback = dmatest_callback;
		rx->callback_param = &done;
		cookie = rx->tx_submit(rx);

		if (dma_submit_error(cookie)) {
			PCIE_PR_ERR("DMA submit error");
			goto release_dma_channel;
		}
		g_dma_begin_time = hisi_getcurtime();
		dma_async_issue_pending(chan);
		if (wait_event_freezable_timeout(done_wait, done.done, wait_jiffies)) {
			PCIE_PR_ERR("Fail, wait event freezable timeout");
			goto release_dma_channel;
		}
		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
		if (status != DMA_COMPLETE) {
			PCIE_PR_ERR("DMA transfer fail, dmatest_error_type = %d", status);
			goto release_dma_channel;
		}
	} else {
		PCIE_PR_ERR("Failed to get dma channel");
		return -1;
	}
	dma_release_channel(chan);
	return 0;
release_dma_channel:
	dma_release_channel(chan);
	return -1;
}

static void calc_rate(char *desc, u32 size, u64 start_time, u64 end_time)
{
	 /* HZ: system timer interrupt number per seconds */
	u64 cost_ns = end_time - start_time;

	u64 kbyte_size = size >> 10;
	u64 mbyte_size = size >> 20;
	u64 gbyte_size = size >> 30;

	u64 kbit_rate = (cost_ns) ? (kbyte_size * 8 * 1000000000) / (cost_ns) : 0;
	u64 mbit_rate = (cost_ns) ? (mbyte_size * 8 * 1000000000) / (cost_ns) : 0;
	u64 gbit_rate = (cost_ns) ? (mbyte_size * 8 * 1000000) / (cost_ns) : 0;

	PCIE_PR_INFO("%s total size: %llu MB(%llu GB)(%llu KB), cost times: %llu ns, rate:%llu Mb/s(%llu Gb/s)(%llu Kb/s)\n",
		desc, mbyte_size, gbyte_size, kbyte_size, cost_ns, mbit_rate, gbit_rate, kbit_rate);
}

/*
 * dma_trans_rate - Calculate the dma rate of data transmision.
 * @rc_id: Host ID;
 * @direction: 0 -- RC read EP; 1 -- RC write EP;
 * @size: Data size;
 * @addr: RC mem address;
 */
int dma_trans_rate(u32 rc_id, int direction, u32 size, u64 addr)
{
	struct kirin_pcie *pcie = NULL;
	int ret = 0;

	g_dma_begin_time = 0;
	g_dma_end_time = 0;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, 0x1010, 0x4, 0xfffffff);

	switch (direction) {
	case 0:
		ret = rc_read_ep_by_dma(addr, size, rc_id);
		if (ret) {
			PCIE_PR_ERR("RC read EP by DMA  fail");
			return ret;
		}
		calc_rate("EP_TO_RC_BY_DMA rate :", size,
				g_dma_begin_time, g_dma_end_time);
		break;
	case 1:
		ret = rc_write_ep_by_dma(addr, size, rc_id);
		if (ret) {
			PCIE_PR_ERR("RC write EP by DMA  fail");
			return ret;
		}
		calc_rate("RC_TO_EP_BY_DMA rate :", size,
				g_dma_begin_time, g_dma_end_time);
		break;
	default:
		PCIE_PR_ERR("Invalid Param");
		return -1;
	}

	return 0;
}

/*
 * rc_read_ep_cfg - RC read EP configuration.
 * @rc_id: Host ID;
 */
int rc_read_ep_cfg(u32 rc_id)
{
	unsigned int i;
	u64 ep_vaddr_in_cpu, busdev;
	struct kirin_pcie *pcie = NULL;
	struct pci_dev *ep_dev = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	ep_dev = pcie->ep_dev;

	if (!ep_dev) {
		PCIE_PR_ERR("Failed to get EP device");
		return -1;
	}

	busdev = TEST_BUS1_OFFSET;
	ep_vaddr_in_cpu = pci_resource_start(ep_dev, 0);

	PCIE_PR_INFO("EP addr in cpu physical is [0x%pK]", (void *)(uintptr_t)ep_vaddr_in_cpu);

	kirin_pcie_outbound_atu(rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, KIRIN_PCIE_ATU_TYPE_CFG0,
		ep_vaddr_in_cpu, busdev, PCI_CONFIG_SPACE);

	ep_vaddr_in_cpu = (__force uintptr_t)ioremap(ep_vaddr_in_cpu, PCI_CONFIG_SPACE);
	if (!ep_vaddr_in_cpu) {
		PCIE_PR_ERR("Failed to ioremap ep_vaddr_in_cpu");
		return -1;
	}
	PCIE_PR_INFO("EP addr in cpu virtual is [0x%pK]", (void *)(uintptr_t)ep_vaddr_in_cpu);

	config_enable_dbi(rc_id, DISABLE);
	for (i = 0; i < PCI_CONFIG_SPACE; i += 4)
		PCIE_PR_INFO("Wifi cfg register: offset[%d] = [0x%x]", i,
			readl((void *)(uintptr_t)ep_vaddr_in_cpu + i));
	config_enable_dbi(rc_id, ENABLE);

	iounmap((void __iomem *)(uintptr_t)ep_vaddr_in_cpu);
	return 0;
}

static void show_diff(u64 rc_addr, u64 ep_addr, unsigned int size)
{
	u32 rc_val, ep_val;
	unsigned int i;

	for (i = 0; i < size; i += 4) {
		rc_val = readl((void *)(uintptr_t)rc_addr + i);
		ep_val = readl((void *)(uintptr_t)ep_addr + i);
		if ((rc_val != ep_val))
			PCIE_PR_INFO("[0x%x]: rc_val=[0x%x], ep_val=[0x%x]",
					i, rc_val, ep_val);
	}
}

/*
 * rc_read_ep_mem - RC read EP mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Inbound region ID;
 */
int rc_read_ep_mem(u32 rc_id, unsigned int size, int index)
{
	int i, ret;
	u64 cpu_addr, begin_time, end_time;
	struct kirin_pcie *pcie = NULL;
	char *temp_memcmp = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	if (!test_kirin_pcie->rc_rdmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_addr is null");
		return -1;
	}

	cpu_addr = (pcie->dtsinfo.dbi_phy_base + TEST_MEM_PCI_BASE_OFFSET);

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	PCIE_PR_INFO("EP addr in cpu physical is [0x%pK], size is [0x%x]", (void *)(uintptr_t)cpu_addr, size);

	kirin_pcie_outbound_atu(rc_id, index, KIRIN_PCIE_ATU_TYPE_MEM, cpu_addr,
		cpu_addr, TEST_MEM_SIZE);

	cpu_addr = (__force uintptr_t)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)vmalloc(size);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)(uintptr_t)cpu_addr);
		return -1;
	}
	ret = memset_s(temp_memcmp, size, 0xFF, size);
	ret += memset_s((void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr,
			size, 0x0, size);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		return ret;
	}

	PCIE_PR_INFO("Reading from EP mem");

	begin_time = jiffies;

	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr, TEST_MEM_SIZE);
	for (i = 0; i < TRANSFER_TIMES; i++)
		pcie_data_cpy(test_kirin_pcie->rc_rdmem_addr, cpu_addr, size);
	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr, TEST_MEM_SIZE);

	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);

	if (memcmp((void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr, (void *)(uintptr_t)cpu_addr, size) != 0 ||
		memcmp((void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr, temp_memcmp, size) == 0) {
		show_diff(test_kirin_pcie->rc_rdmem_addr, cpu_addr, size);
		ret = -1;
	} else {
		ret = 0;
	}

	iounmap((void __iomem *)(uintptr_t)cpu_addr);
	vfree(temp_memcmp);

	return ret;
}

/*
 * set_ep_mem_inbound - Set EP DDR mem inbound for loopback data transfer.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
int set_ep_mem_inbound(u32 rc_id, int index)
{
	u64 busdev, temp_addr;
	struct pcie_test_st *test_kirin_pcie = NULL;
	struct kirin_pcie *pcie = NULL;
	int i;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	set_bme(rc_id, ENABLE);
	set_mse(rc_id, ENABLE);
	busdev = (pcie->dtsinfo.dbi_phy_base + TEST_MEM_PCI_BASE_OFFSET);

	if (!test_kirin_pcie->ep_inboundmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_addr is null");
		return -1;
	}

	temp_addr = virt_to_phys((void *)(uintptr_t)test_kirin_pcie->ep_inboundmem_addr);

	PCIE_PR_INFO("Inbound pci_add [0x%pK] to cpu_addr[0x%pK]", (void *)(uintptr_t)busdev, (void *)(uintptr_t)temp_addr);

	kirin_pcie_inbound_atu(rc_id, index, KIRIN_PCIE_ATU_TYPE_MEM, temp_addr,
		busdev, TEST_MEM_SIZE);

	for (i = 0; i < TEST_MEM_SIZE; i += 4)
		writel(0x34, (void *)(uintptr_t)test_kirin_pcie->ep_inboundmem_addr + i);

	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->ep_inboundmem_addr, TEST_MEM_SIZE);

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
	struct pcie_test_st *test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (check_pcie_on_work(rc_id))
		return;

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	if (!test_kirin_pcie->ep_rdmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_rdmem_add is null");
		return;
	}
	if (!test_kirin_pcie->ep_wrmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_wrmem_add is null");
		return;
	}

	for (i = 0; i < size; i += 4) {
		val = readl((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr + offset + i);
		PCIE_PR_INFO("ep_rdmem::Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	for (i = 0; i < size; i += 4) {
		val = readl((void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr + offset + i);
		PCIE_PR_INFO("ep_wrmem::Offset[0x%x], value=[0x%x]", i + offset, val);
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
	u64 cpu_addr, begin_time, end_time;
	int i, ret;
	char *temp_memcmp = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;
	struct kirin_pcie *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	if (!test_kirin_pcie->rc_wrmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_addr is null");
		return -1;
	}

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	cpu_addr = (pcie->dtsinfo.dbi_phy_base + TEST_MEM_PCI_BASE_OFFSET);

	kirin_pcie_outbound_atu(rc_id, index, KIRIN_PCIE_ATU_TYPE_MEM, cpu_addr,
		cpu_addr, TEST_MEM_SIZE);

	cpu_addr = (__force uintptr_t)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)vmalloc(size);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)(uintptr_t)cpu_addr);
		return -1;
	}

	ret = memset_s((void *)temp_memcmp, size, 0xFF, size);
	ret += memset_s((void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr,
			size, 0x56, TEST_MEM_SIZE);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		return ret;
	}

	PCIE_PR_INFO("Writing 0x56 to EP mem");

	begin_time = jiffies;

	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr, TEST_MEM_SIZE);
	for (i = 0; i < TRANSFER_TIMES; i++)
		pcie_data_cpy(cpu_addr, test_kirin_pcie->rc_wrmem_addr, size);
	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr, TEST_MEM_SIZE);

	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);

	if (memcmp((void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr, (void *)(uintptr_t)cpu_addr, size) != 0 ||
		memcmp((void *)(uintptr_t)cpu_addr, temp_memcmp, size) == 0) {
		show_diff(test_kirin_pcie->rc_wrmem_addr, cpu_addr, size);
		ret = -1;
	} else {
		ret = 0;
	}
	iounmap((void __iomem *)(uintptr_t)cpu_addr);
	vfree(temp_memcmp);
	return ret;
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
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	switch (flag) {
	case RC_RD_WR_EP:
		if ((!test_kirin_pcie->ep_wrmem_addr) ||
			(!test_kirin_pcie->rc_rdmem_addr)) {
			PCIE_PR_ERR("Addr is NULL");
			return -1;
		}

		if (memcmp((void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr,
			(void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr, size) != 0) {
			PCIE_PR_ERR("Transfer Failed");
			show_diff(test_kirin_pcie->rc_rdmem_addr,
					test_kirin_pcie->ep_wrmem_addr, size);
			return -1;
		}
		PCIE_PR_INFO("Transfer Ok");
		return 0;
	case EP_RD_WR_RC:
		if ((!test_kirin_pcie->ep_wrmem_addr) ||
			(!test_kirin_pcie->rc_rdmem_addr)) {
			PCIE_PR_ERR("Addr is NULL");
			return -1;
		}

		if (memcmp((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr,
			(void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr, size) != 0) {
			PCIE_PR_ERR("Transfer Failed");
			show_diff(test_kirin_pcie->rc_wrmem_addr,
					test_kirin_pcie->ep_rdmem_addr, size);
			return -1;
		}
		PCIE_PR_INFO("Transfer Ok");
		return 0;
	default:
		PCIE_PR_ERR("Input invalid param");
		return 0;
	}
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
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	set_bme(rc_id, ENABLE);
	set_mse(rc_id, ENABLE);
	busdev = TEST_BUS0_OFFSET;
	pcie = test_kirin_pcie->pcie;

	if (!test_kirin_pcie->rc_inboundmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_addr is null");
		return -1;
	}

	temp_addr = virt_to_phys((void *)(uintptr_t)test_kirin_pcie->rc_inboundmem_addr);

	PCIE_PR_INFO("Inbound pci_add [0x%pK] to cpu_addr[0x%pK]", (void *)(uintptr_t)busdev, (void *)(uintptr_t)temp_addr);

	kirin_pcie_inbound_atu(rc_id, index, KIRIN_PCIE_ATU_TYPE_MEM, temp_addr,
		busdev, TEST_MEM_SIZE);

	for (i = 0; i < TEST_MEM_SIZE; i += 4)
		writel(0x78, (void *)(uintptr_t)test_kirin_pcie->rc_inboundmem_addr + i);

	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->rc_inboundmem_addr, TEST_MEM_SIZE);

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
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;


	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->rc_rdmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_rdmem_add is null");
		return -1;
	}

	if (!test_kirin_pcie->rc_wrmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_wrmem_add is null");
		return -1;
	}

	for (i = 0; i < size; i += 4) {
		val = readl((void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr + offset + i);
		PCIE_PR_INFO("rc_rdmem: Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	for (i = 0; i < size; i += 4) {
		val = readl((void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr + offset + i);
		PCIE_PR_INFO("rc_wrmem: Offset[0x%x], value=[0x%x]", i + offset, val);
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
	u64 cpu_addr, begin_time, end_time;
	u32 i;
	int ret;
	char *temp_memcmp = NULL;
	struct resource *dbi = NULL;
	struct platform_device *pdev = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	if (!test_kirin_pcie->ep_rdmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_addr is null");
		return -1;
	}

	pdev = to_platform_device(test_kirin_pcie->pcie->pci->dev);

	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi) {
		PCIE_PR_ERR("Failed to get dbi base");
		return -1;
	}

	cpu_addr = dbi->start + TEST_BUS0_OFFSET;

	kirin_pcie_outbound_atu(rc_id, index, KIRIN_PCIE_ATU_TYPE_MEM, cpu_addr,
		TEST_BUS0_OFFSET, TEST_MEM_SIZE);

	cpu_addr = (__force uintptr_t)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)vmalloc(size);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)(uintptr_t)cpu_addr);
		return -1;
	}
	ret = memset_s(temp_memcmp, size, 0xFF, size);
	ret += memset_s((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr,
			size, 0x0, size);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		return ret;
	}

	PCIE_PR_INFO("Reading from RX mem");

	begin_time = jiffies;

	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr, TEST_MEM_SIZE);
	for (i = 0; i < TRANSFER_TIMES; i++)
		pcie_data_cpy(test_kirin_pcie->ep_rdmem_addr, cpu_addr, size);
	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr, TEST_MEM_SIZE);

	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);

	if (memcmp((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr, (void *)(uintptr_t)cpu_addr, size) != 0 ||
		memcmp((void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr, temp_memcmp, size) == 0) {
		show_diff(test_kirin_pcie->ep_rdmem_addr, cpu_addr, size);
		ret = -1;
	} else {
		ret = 0;
	}

	iounmap((void __iomem *)(uintptr_t)cpu_addr);
	vfree(temp_memcmp);
	return ret;
}

/*
 * ep_write_rc_mem - EP write RC mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Outbound region ID;
 */
int ep_write_rc_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr, begin_time, end_time;
	int i, ret;
	char *temp_memcmp = NULL;
	struct resource *dbi = NULL;
	struct platform_device *pdev = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size <= 0)
		size = TEST_MEM_SIZE;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	if (!test_kirin_pcie->ep_wrmem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_addr is null");
		return -1;
	}

	pdev = to_platform_device(test_kirin_pcie->pcie->pci->dev);

	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi) {
		PCIE_PR_ERR("Failed to get dbi base");
		return -1;
	}

	cpu_addr = dbi->start + TEST_BUS0_OFFSET;

	kirin_pcie_outbound_atu(rc_id, index, KIRIN_PCIE_ATU_TYPE_MEM, cpu_addr,
		TEST_BUS0_OFFSET, TEST_MEM_SIZE);

	cpu_addr = (__force uintptr_t)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)vmalloc(size);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)(uintptr_t)cpu_addr);
		return -1;
	}
	ret = memset_s(temp_memcmp, size, 0xFF, size);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		return ret;
	}

	PCIE_PR_INFO("Writing 0xde to RC mem");
	ret = memset_s((void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr,
			size, 0xde, size);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		return ret;
	}

	begin_time = jiffies;

	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr, TEST_MEM_SIZE);
	for (i = 0; i < TRANSFER_TIMES; i++)
		pcie_data_cpy(cpu_addr, test_kirin_pcie->ep_wrmem_addr, size);
	__flush_dcache_area((void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr, TEST_MEM_SIZE);

	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);
	if (memcmp((void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr, (void *)(uintptr_t)cpu_addr, size) != 0 ||
		memcmp((void *)(uintptr_t)cpu_addr, temp_memcmp, size) == 0) {
		show_diff(test_kirin_pcie->ep_wrmem_addr, cpu_addr, size);
		ret = -1;
	} else {
		ret = 0;
	}

	iounmap((void __iomem *)(uintptr_t)cpu_addr);
	vfree(temp_memcmp);
	return ret;
}

/*
 * test_host_power_control - Power on host and scan bus.
 * @rc_id: Host ID;
 * @flag: RC power status;
 */
int test_host_power_control(u32 rc_id, u32 flag)
{
	struct kirin_pcie *pcie = NULL;
	int ret;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pcie = g_test_kirin_pcie[rc_id].pcie;

	if (!pcie) {
		PCIE_PR_ERR("PCIe%d is null", rc_id);
		return -EINVAL;
	}

	ret = kirin_pcie_power_ctrl(pcie, (enum rc_power_status)flag);

	return ret;
}

/*
 * Set PCIe CTRL bus timeout
 * For silicon, it is 15ms or 16ms(if 0 based)
 * For FPGA, it is 150ms because its clock is much slower,
 * val needs to be set to 0x99e for FPGA platform
 */
int pcie_set_ctrlbus_timeout(u32 rc_id, u32 val)
{
	struct kirin_pcie *pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL25_ADDR);
	return 0;
}

/* Enable CTRLBUS Timeout Functionality */
int pcie_enable_ctrlbus_timeout(u32 rc_id)
{
	struct kirin_pcie *pcie = NULL;
	u32 val;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL25_ADDR);
	val &= ~CTRLBUS_TIMEOUT_ENABLE;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL25_ADDR);
	return 0;
}

/* Disable CTRLBUS Timeout Functionality */
int pcie_disable_ctrlbus_timeout(u32 rc_id)
{
	struct kirin_pcie *pcie = NULL;
	u32 val;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = &g_kirin_pcie[rc_id];
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL25_ADDR);
	val |= CTRLBUS_TIMEOUT_ENABLE;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL25_ADDR);
	return 0;
}

/*
 * kirin_pcie_ep_mac_init - EP mac initialization.
 * @id: PCIe ID;
 */
int kirin_pcie_ep_mac_init(u32 pcie_id)
{
	struct kirin_pcie *pcie = NULL;
	u32 val;

	if (pcie_id >= g_rc_num) {
		PCIE_PR_ERR("There is no pcie_id = %d", pcie_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[pcie_id];

	/* change EP device ID */
	val = PCIE_VENDOR_ID_HUAWEI + (pcie_id << PCIE_DEV_ID_SHIFT);

	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, 0x0, 0x4, val);
	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, 0x0, 0x4);
	PCIE_PR_INFO("DevID&VendorID is [0x%x]", val);

	/* Modify EP device class from 0x0(unsupport) to net devices */
	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_CLASS_REVISION, 0x4);
	val |= (PCI_CLASS_NETWORK_OTHER << PCI_CLASS_CODE_SHIFT);
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, PCI_CLASS_REVISION, 0x4, val);

	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, PCI_CLASS_REVISION, 0x4);
	PCIE_PR_INFO("Device class is [0x%x]", val);

	/* For FPGA, Set link speed to Gen1 */
	if (pcie->dtsinfo.board_type == BOARD_FPGA) {
		val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
			KIRIN_PCIE_EXP_LNKCTL2, 0x4);
		val &= ~PCIE_TARGET_SPEED_MASK;
		val |= PCIE_TARGET_SPEED_GEN1;
		kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, KIRIN_PCIE_EXP_LNKCTL2, 0x4, val);
	}

	ltssm_enable(pcie_id, ENABLE);
	PCIE_PR_INFO("Enable Link Training");

	set_bme(pcie_id, ENABLE);
	set_mse(pcie_id, ENABLE);
	PCIE_PR_INFO("Enable bus master and memory decode");

	/* disable EP change configuration space */
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL6_ADDR);
	val |= APP_DBI_RO_WR_DISABLE;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL6_ADDR);

	enable_req_clk(pcie, DISABLE);

	if (kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base,
		KIRIN_PCIE_ATU_VIEWPORT, 0x4) == 0xffffffff)
		pcie->pci->iatu_unroll_enabled = 1;
	else
		pcie->pci->iatu_unroll_enabled = 0;

	return 0;
}

/*
 * test_compliance - Set compliance for test.
 * @rc_id: Host ID;
 * @entry: 0 -- exit compliance; others -- entry compliance;
 */
int test_compliance(u32 rc_id, u32 entry)
{
	u32 val, cap_pos;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pp = &(g_test_kirin_pcie[rc_id].pcie->pci->pp);
	cap_pos = kirin_pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	ltssm_enable(rc_id, DISABLE);
	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, &val);
	if (entry)
		val |= ENTER_COMPLIANCE;
	else
		val &= ~ENTER_COMPLIANCE;
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, val);

	ltssm_enable(rc_id, ENABLE);

	return 0;
}

/*
 * set_preset - set preset for gen3 test.
 * @rc_id: Host ID;
 * @preset: preset value;
 */
int set_preset(u32 rc_id, u32 preset)
{
	u32 val, cap_pos;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pp = &(g_test_kirin_pcie[rc_id].pcie->pci->pp);

	if (preset > 10) {
		PCIE_PR_ERR("Invalid Param");
		return -1;
	}

	ltssm_enable(rc_id, DISABLE);
	cap_pos = kirin_pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos) {
		PCIE_PR_ERR("Failed to get PCI_CAP_ID_EXP");
		return -1;
	}

	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, &val);
	val &= ~(0xf << 12);
	val |= (preset << 12);
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, val);

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
	u32 val;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pp = &(g_test_kirin_pcie[rc_id].pcie->pci->pp);

	kirin_pcie_rd_own_conf(pp, PORT_LINK_CTRL_REG, 4, &val);
	val |= (0x1 << 2);
	kirin_pcie_wr_own_conf(pp, PORT_LINK_CTRL_REG, 4, val);

	if (local) {
		kirin_pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, 4, &val);
		val |= (0x1 << 16);
		kirin_pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, 4, val);

		kirin_pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, &val);
		val |= (0x1 << 31);
		kirin_pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, val);
	}
}

/*
 * test_exit_loopback - Set exit loopabck.
 * @rc_id: Host ID;
 * @local: 0 -- remote loopback; others -- local loopback;
 */
void test_exit_loopback(u32 rc_id, u32 local)
{
	u32 val;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pp = &(g_test_kirin_pcie[rc_id].pcie->pci->pp);

	kirin_pcie_rd_own_conf(pp, PORT_LINK_CTRL_REG, 4, &val);
	val &= ~(0x1 << 2);
	kirin_pcie_wr_own_conf(pp, PORT_LINK_CTRL_REG, 4, val);

	if (local) {
		kirin_pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, 4, &val);
		val &= ~(0x1 << 16);
		kirin_pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, 4, val);

		kirin_pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, &val);
		val &= ~(0x1 << 31);
		kirin_pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, val);
	}
}

/*
 * bypass_detect - Bypass detect link state.
 * @rc_id: Host ID;
 */
void bypass_detect(u32 rc_id)
{
	u32 val = 0;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	pp = &(g_test_kirin_pcie[rc_id].pcie->pci->pp);

	kirin_pcie_rd_own_conf(pp, PROT_FORCE_LINK_REG, 4, &val);
	val |= (0x1 << 17);
	kirin_pcie_wr_own_conf(pp, PROT_FORCE_LINK_REG, 4, val);
	udelay(10);

	kirin_pcie_rd_own_conf(pp, PROT_FORCE_LINK_REG, 4, &val);
	val |= (0x1 << 15);
	kirin_pcie_wr_own_conf(pp, PROT_FORCE_LINK_REG, 4, val);
}

/*
 * phy_internal_loopback - Set phy internal loopback for test.
 * @rc_id: Host ID;
 * @dir: 0 -- TX2RX; 1 -- RX2TX;
 */
int phy_internal_loopback(u32 rc_id, enum phy_lb_dir dir, enum link_speed gen)
{
	u32 val = 0;
	struct kirin_pcie *pcie = NULL;
	struct pcie_port *pp = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_kirin_pcie[rc_id].pcie;
	pp = &(g_test_kirin_pcie[rc_id].pcie->pci->pp);

	set_link_speed(rc_id, gen);

	if (dir == TX2RX)
		kirin_natural_phy_writel(pcie, 0x5, 0x1000);
	else
		kirin_natural_phy_writel(pcie, 0x6, 0x1000);

	if (gen == GEN3) {
		kirin_pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, 4, &val);
		val |= (0x1 << 16);
		kirin_pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, 4, val);

		kirin_pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, &val);
		val |= (0x1 << 31);
		kirin_pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, val);
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
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, 0x8);
	value_temp |= (0x1 << 26);
	kirin_elb_writel(pcie, value_temp, 0x8);
	udelay(2);

	value_temp &= ~(0x1 << 26);
	kirin_elb_writel(pcie, value_temp, 0x8);
	PCIE_PR_INFO("Read after write, ctl2 is %x", kirin_elb_readl(pcie, 0x8));
	PCIE_PR_INFO("ep_triggle_msi_intr");
}

/*
 * ep_triggle_intA_intr - EP triggle INTa interrupt.
 * @rc_id: Host ID;
 */
void ep_triggle_intA_intr(u32 rc_id)
{
	u32 value_temp;
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	value_temp |= (0x1 << 5);
	kirin_elb_writel(pcie, value_temp, SOC_PCIECTRL_CTRL7_ADDR);

	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	PCIE_PR_INFO("read after write, ctl7 is %x", value_temp);
	PCIE_PR_INFO("ep_triggle_inta_intr");
}

/*
 * ep_clr_intA_intr - EP clear INTa interrupt.
 * @rc_id: Host ID;
 */
void ep_clr_intA_intr(u32 rc_id)
{
	u32 value_temp;
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	value_temp &= ~(0x1 << 5);
	kirin_elb_writel(pcie, value_temp, SOC_PCIECTRL_CTRL7_ADDR);

	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	PCIE_PR_INFO("read after write, ctl7 is %x", value_temp);
	PCIE_PR_INFO("ep_clr_inta_intr");
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
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	iatu_offset = test_kirin_pcie->pcie->dtsinfo.iatu_base_offset;

	if (msg_code == MSG_CODE_ASSERT_INTA || msg_code == MSG_CODE_ASSERT_INTB ||
	    msg_code == MSG_CODE_DEASSERT_INTA || msg_code == MSG_CODE_DEASSERT_INTB) {
		PCIE_PR_INFO("INTA and INTB is not supported");
		return;
	} else if (msg_code == MSG_CODE_PME_TURN_OFF) {
		msg_type = MSG_TYPE_ROUTE_BROADCAST;
	} else {
		msg_type = KIRIN_PCIE_ATU_TYPE_MSG;
	}
	kirin_pcie_generate_msg(rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, iatu_offset, msg_type, msg_code);
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
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;


	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	value_temp = kirin_elb_readl(pcie, offset);
	value_temp &= ~(0x1 << bit);
	kirin_elb_writel(pcie, value_temp, offset);
	value_temp |= (0x1 << bit);
	kirin_elb_writel(pcie, value_temp, offset);
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
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, offset);
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
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	if (local)
		dev = pcie->rc_dev;
	else
		dev = pcie->ep_dev;

	pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &val);

	switch (num) {
	case 13:
		PCIE_PR_INFO("Enable obff message");
		val |= PCI_EXP_DEVCTL2_OBFF_MSGA_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, val);
		break;
	case 10:
		PCIE_PR_INFO("Enable LTR message");
		val |= PCI_EXP_DEVCTL2_LTR_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, val);
		break;
	default:
		PCIE_PR_INFO("Unsupport function");
		break;
	}

}

/* Check Bit 0 of SOC_PCIECTRL_CTRL22_ADDR to see clkreq setting */
int kirin_test_clkreq_status(u32 rc_id)
{
	u32 val;
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL22_ADDR);
	val &= 0x1;
	return val;
}

static void get_pcie_credit_info(struct kirin_pcie *pcie, u32 type, u32 *header, u32 *data)
{
	u32 val;

	if (type == TYPE_POST) {
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE12_ADDR);
	} else if (type == TYPE_NONPOST) {
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE13_ADDR);
	} else if (type == TYPE_CPL) {
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE14_ADDR);
	} else {
		PCIE_PR_ERR("Invalid type parameter");
		return;
	}

	*header = (val & PCIE_CREDIT_HEADER_MASK) >> PCIE_CREDIT_HEADER_OFFS;
	*data = val & PCIE_CREDIT_DATA_MASK;
}

int wlan_on(u32 rc_id, int on)
{
	int ret;
	u32 wl_power;
	struct device_node *np = NULL;
	struct kirin_pcie *pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];
	if (!pcie->pci->dev) {
		PCIE_PR_ERR("NO PCIe device");
		return -1;
	}

	np = pcie->pci->dev->of_node;
	if (np) {
		if (!(of_property_read_u32(np, "wl_power", &wl_power))) {
			PCIE_PR_INFO("WL Power On Number is [%d]", wl_power);
		} else {
			PCIE_PR_INFO("Failed to get wl_power info");
			return -1;
		}
	} else {
		PCIE_PR_INFO("Failed to get kirin_pcie info");
		return -1;
	}

	if (!wl_power) {
		PCIE_PR_ERR("wl_power is invalid");
		return -1;
	}

	ret = gpio_request(wl_power, "wl_on");
	if (ret != 0) {
		PCIE_PR_ERR("Failed to request wl_on gpio");
		return -1;
	}
	if (on) {
		PCIE_PR_INFO("Power on Wlan");
		gpio_direction_output(wl_power, 1);
		mdelay(200);
	} else {
		PCIE_PR_INFO("Power down Wlan");
		gpio_direction_output(wl_power, 0);
		mdelay(100);
	}
	gpio_free(wl_power);

	return 0;
}

void print_credit_info(u32 rc_id)
{
	u32 val, header, data;
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	get_pcie_credit_info(pcie, TYPE_POST, &header, &data);
	PCIE_PR_INFO("POST Cre_dit info:	Header:%d	Data:%d", header, data);
	get_pcie_credit_info(pcie, TYPE_NONPOST, &header, &data);
	PCIE_PR_INFO("Non-POST Cre_dit info:	Header:%d	Data:%d", header, data);
	get_pcie_credit_info(pcie, TYPE_CPL, &header, &data);
	PCIE_PR_INFO("Completion Cre_dit info:	Header:%d	Data:%d", header, data);

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE15_ADDR);
	if (val & NO_PENDING_DLLP)
		PCIE_PR_INFO("There is no pending DLLP");
	else
		PCIE_PR_INFO("There is pending DLLP(s)");

	if (val & NO_EXPECTING_ACK)
		PCIE_PR_INFO("There is no expecting ACK");
	else
		PCIE_PR_INFO("There is expecting ACK(s)");

	if (val & HAD_ENOUGH_CREDIT)
		PCIE_PR_INFO("All types have enough credits");
	else
		PCIE_PR_INFO("Some type doesn't have enough credits");

	if (val & NO_PENDING_TLP)
		PCIE_PR_INFO("There is no pending TLP");
	else
		PCIE_PR_INFO("There is pending TLP");

	if (val & NO_FC)
		PCIE_PR_INFO("Some type has no credit");
	else
		PCIE_PR_INFO("All types have credit");
}

int kirin_pcie_set_ep_mode(u32 rc_id)
{
	int ret;
	struct kirin_pcie *pcie = NULL;
	struct kirin_pcie_dtsinfo *dtsinfo = NULL;
	struct platform_device *pdev = NULL;

	PCIE_PR_INFO("+[%u]+", rc_id);

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];
	dtsinfo = &pcie->dtsinfo;

	pdev = to_platform_device(pcie->pci->dev);

	PCIE_PR_INFO("set PCIe[%u] in EP mode", rc_id);
	dtsinfo->ep_flag = 1;

	ret = kirin_pcie_power_notifiy_register(pcie->rc_id, NULL, NULL, NULL);
	if (ret)
		PCIE_PR_INFO("Failed to register NULL");

	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_MSI].num, (void *)pcie);
	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_LINKDOWN].num, (void *)pcie);

#ifdef CONFIG_KIRIN_PCIE_CPLTIMEOUT_INT
	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_CPLTIMEOUT].num, (void *)pcie);
#endif

	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTD].num, (void *)pcie);
	devm_free_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTC].num, (void *)pcie);

	PCIE_PR_INFO("-[%u]-", rc_id);
	return ret;
}

static int kirin_pcie_test_alloc_mem(struct pcie_test_st *test_kirin_pcie)
{
	/* Check one memory address is enough to see if memories are alreay allocated */
	if (test_kirin_pcie->rc_wrmem_addr != 0) {
		PCIE_PR_ERR("Kirin PCIe test already done!");
		return 0;
	}

	test_kirin_pcie->rc_wrmem_addr = __get_free_pages(GFP_KERNEL, 8);
	if (test_kirin_pcie->rc_wrmem_addr == 0) {
		PCIE_PR_ERR("Failed to alloc RC write memory");
		goto RC_WR_MEM_FAIL;
	}
	PCIE_PR_INFO("rc_wrmem_addr is 0x%pK", (void *)(uintptr_t)test_kirin_pcie->rc_wrmem_addr);

	test_kirin_pcie->rc_rdmem_addr = __get_free_pages(GFP_KERNEL, 8);
	if (test_kirin_pcie->rc_rdmem_addr == 0) {
		PCIE_PR_ERR("Failed to alloc RC read memory");
		goto RC_RD_MEM_FAIL;
	}
	PCIE_PR_INFO("rc_rdmem_addr is 0x%pK", (void *)(uintptr_t)test_kirin_pcie->rc_rdmem_addr);

	test_kirin_pcie->ep_wrmem_addr = __get_free_pages(GFP_KERNEL, 8);
	if (test_kirin_pcie->ep_wrmem_addr == 0) {
		PCIE_PR_ERR("Failed to alloc EP write memory");
		goto EP_WR_MEM_FAIL;
	}
	PCIE_PR_INFO("ep_wrmem_addr is 0x%pK", (void *)(uintptr_t)test_kirin_pcie->ep_wrmem_addr);

	test_kirin_pcie->ep_rdmem_addr = __get_free_pages(GFP_KERNEL, 8);
	if (test_kirin_pcie->ep_rdmem_addr == 0) {
		PCIE_PR_ERR("Failed to alloc EP read memory");
		goto EP_RD_MEM_FAIL;
	}
	PCIE_PR_INFO("ep_rdmem_addr is 0x%pK", (void *)(uintptr_t)test_kirin_pcie->ep_rdmem_addr);

	test_kirin_pcie->rc_inboundmem_addr = __get_free_pages(GFP_KERNEL, 8);
	if (test_kirin_pcie->rc_inboundmem_addr == 0) {
		PCIE_PR_ERR("Failed to alloc rc_inbound memory");
		goto RC_INBOUND_MEM_FAIL;
	}
	PCIE_PR_INFO("rc_inboundmem_addr is 0x%pK", (void *)(uintptr_t)test_kirin_pcie->rc_inboundmem_addr);

	test_kirin_pcie->ep_inboundmem_addr = __get_free_pages(GFP_KERNEL, 8);
	if (test_kirin_pcie->ep_inboundmem_addr == 0) {
		PCIE_PR_ERR("Failed to alloc ep_inbound memory");
		goto EP_INBOUND_MEM_FAIL;
	}
	PCIE_PR_INFO("ep_inboundmem_addr is 0x%pK", (void *)(uintptr_t)test_kirin_pcie->ep_inboundmem_addr);

	return 0;

EP_INBOUND_MEM_FAIL:
	free_pages(test_kirin_pcie->rc_inboundmem_addr, 8);
	test_kirin_pcie->rc_inboundmem_addr = 0;
RC_INBOUND_MEM_FAIL:
	free_pages(test_kirin_pcie->ep_rdmem_addr, 8);
	test_kirin_pcie->ep_rdmem_addr = 0;
EP_RD_MEM_FAIL:
	free_pages(test_kirin_pcie->ep_wrmem_addr, 8);
	test_kirin_pcie->ep_wrmem_addr = 0;
EP_WR_MEM_FAIL:
	free_pages(test_kirin_pcie->rc_rdmem_addr, 8);
	test_kirin_pcie->rc_rdmem_addr = 0;
RC_RD_MEM_FAIL:
	free_pages(test_kirin_pcie->rc_wrmem_addr, 8);
	test_kirin_pcie->rc_wrmem_addr = 0;
RC_WR_MEM_FAIL:
	return -ENOMEM;
}

int kirin_pcie_test_init(u32 rc_id)
{
	struct kirin_pcie *pcie = NULL;
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];
	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	test_kirin_pcie->pcie = pcie;

	return kirin_pcie_test_alloc_mem(test_kirin_pcie);
}

int kirin_pcie_test_exist(u32 rc_id)
{
	struct pcie_test_st *test_kirin_pcie = NULL;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	if (test_kirin_pcie->rc_rdmem_addr) {
		free_pages(test_kirin_pcie->rc_rdmem_addr, 8);
		test_kirin_pcie->rc_rdmem_addr = 0;
	}

	if (test_kirin_pcie->rc_wrmem_addr) {
		free_pages(test_kirin_pcie->rc_wrmem_addr, 8);
		test_kirin_pcie->rc_wrmem_addr = 0;
	}

	if (test_kirin_pcie->ep_rdmem_addr) {
		free_pages(test_kirin_pcie->ep_rdmem_addr, 8);
		test_kirin_pcie->ep_rdmem_addr = 0;
	}

	if (test_kirin_pcie->ep_wrmem_addr) {
		free_pages(test_kirin_pcie->ep_wrmem_addr, 8);
		test_kirin_pcie->ep_wrmem_addr = 0;
	}

	if (test_kirin_pcie->rc_inboundmem_addr) {
		free_pages(test_kirin_pcie->rc_inboundmem_addr, 8);
		test_kirin_pcie->rc_inboundmem_addr = 0;
	}

	if (test_kirin_pcie->ep_inboundmem_addr) {
		free_pages(test_kirin_pcie->ep_inboundmem_addr, 8);
		test_kirin_pcie->ep_inboundmem_addr = 0;
	}

	test_kirin_pcie->pcie = NULL;

	return 0;
}

const struct dw_pcie_ops dw_pcie_ep_ops = {
	.read_dbi = kirin_pcie_read_dbi,
	.write_dbi = kirin_pcie_write_dbi,
};

static int kirin_pcie_ep_probe(struct platform_device *pdev)
{
	struct kirin_pcie *pcie = NULL;
	struct dw_pcie *pci = NULL;
	struct kirin_pcie_dtsinfo *dtsinfo = NULL;
	int ret;

	PCIE_PR_INFO("++");

	ret = kirin_pcie_get_pcie(&pcie, pdev);
	if (ret) {
		PCIE_PR_ERR("Failed to get kirin pcie from dts");
		return ret;
	}

	PCIE_PR_INFO("PCIe EP No.%d probe", pcie->rc_id);

	pci = devm_kzalloc(&pdev->dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;
	pcie->pci = pci;

	ret = kirin_pcie_get_dtsinfo(pcie, pdev);
	if (ret) {
		PCIE_PR_ERR("Failed to get dts info");
		goto FAIL;
	}

	pcie->pci->dev = &(pdev->dev);
	pcie->pci->ops = &dw_pcie_ep_ops;
	dtsinfo = &pcie->dtsinfo;

	ret = gpio_request((unsigned int)pcie->gpio_id_reset, "pcie_reset");
	if (ret) {
		PCIE_PR_ERR("Failed to request gpio-%d", pcie->gpio_id_reset);
		goto FAIL;
	}

	platform_set_drvdata(pdev, pcie);

	ret = pcie_plat_init(pdev, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to get platform info");
		goto FAIL;
	}

	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power on EP");
		goto FAIL;
	}

	ret = kirin_pcie_ep_mac_init(pcie->rc_id);
	if (ret) {
		PCIE_PR_ERR("Failed to initialize EP MAC");
		goto FAIL;
	}

	atomic_set(&(pcie->is_ready), 1);
	spin_lock_init(&pcie->ep_ltssm_lock);
	mutex_init(&pcie->power_lock);
	mutex_init(&pcie->pm_lock);

	PCIE_PR_INFO("--");
	return 0;
FAIL:
	devm_kfree(&pdev->dev, pcie->pci);

	return ret;
}

static struct of_device_id kirin_pcie_ep_match_table[] = {
	{ .compatible = "hisilicon,kirin-pcie-ep",
		.data = NULL,
	},
	{},
};

static struct platform_driver kirin_pcie_ep_driver = {
	.driver		= {
		.name		= "kirin-pcie-ep",
		.owner = THIS_MODULE,
	       .of_match_table	= kirin_pcie_ep_match_table,
	},
	.probe          = kirin_pcie_ep_probe,
};

/*
 * kirin_pcie_ep_init - kirin_pcie ep device init.
 * @rc_id: Host ID;
 * @num: 13 -- OBFF; 10 -- LTR; others -- not msg function
 * @local: 0 -- Set EP dev; others -- Set RC dev;
 */
static int __init kirin_pcie_ep_init(void)
{
	int ret;

	ret = platform_driver_register(&kirin_pcie_ep_driver);
	if (ret) {
		PCIE_PR_ERR("Failed to register kirin ep driver");
		return ret;
	}
	return 0;
}

device_initcall(kirin_pcie_ep_init);
MODULE_DESCRIPTION("Hisilicon Kirin pcie driver");
MODULE_LICENSE("GPL");
