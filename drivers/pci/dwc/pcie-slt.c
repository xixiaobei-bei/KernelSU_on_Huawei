/*
 * pcie-slt.c
 *
 * PCIe SLT test functions
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

#ifdef CONFIG_HISI_DEBUG_FS

#include "pcie-kport-common.h"
#include "pcie-kport-idle.h"

#include <linux/file.h>
#include <linux/module.h>
#include <linux/compat.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/pci-aspm.h>
#include <linux/fs.h>
#include <linux/mfd/hisi_pmic.h>
#include <asm/memory.h>
#include <hitest_slt.h>
#include <securec.h>
#include <scsi/ufs/ufs.h>
#include <hw_cmdline_parse.h>

/* PCIe0<->PCIe1 mode, PCIe1 is RC and PCIe0 is EP */
#define APR_RC_ID 1
#define APR_EP_ID 0

#define SLT_RANDOM_DATA    0x12
#define SLT_TEST_DATA_SIZE 0xc0000

#define SIZE_M         0x400000
#define PCIE_SLT_NAME  "pcie-slt"
#define RC_NUMS        2
#ifdef CONFIG_PCIE_KPORT_APR
#define RC_ID 1
#else
#define RC_ID 0
#endif
#define PCIE_SLT_DATA_DFT_LOOP_TIMES 2
#define PCIE_SLT_DATA_MAX_LOOP_TIMES 10000
#define ASPM_STATE_SINGLE_NUM 2
#define ASPM_STATE_MAX (ASPM_L0S_L1 + 1)
#define ID_SHIFT		28
#define READY_SHIT	24
#define VOLTAGE_SHIT	20
#define TEST_SHIT	12
#define RESULT_OK	0

enum aspm_stat {
	L0_MODE = 0,
	L0S_MODE = 1,
	L1_MODE = 2,
	L1_1_MODE = 3,
	L1_2_MODE = 4,
};

enum {
	H2D = 1,
	D2H = 2,
};

enum pcie_ops_stat {
	OPS_OK = 0,
	ERR_EP_INIT,
	ERR_EP_ON,
	ERR_EP_OFF,
	ERR_EP_SETUP,
	ERR_EP_TRANS,
	ERR_EP_OTHER
};

enum pcie_test_result {
	DEFAULT_RESULT = 0,
	ERR_L0,
	ERR_L0S,
	ERR_L1,
	ERR_L0S_L1,
	ERR_L1_1,
	ERR_L1_2,
	ERR_DATA_TRANS,
	ERR_DATA_CMP,
	ERR_OTHER
};

enum pcie_voltage {
	DEFAULT_VOL = 0,
	NORMAL_VOL,
	LOW_VOL,
	HIGH_VOL
};

#define PCIETESTCMD      _IOWR('p', 0xc1, unsigned long)
#define PCIETESTCMD_NEW  _IOWR('p', 0xc2, unsigned long)

struct pcie_ep_ops {
	int (*init)(void *data);
	int (*on)(void *data);
	int (*off)(void *data);
	int (*setup)(void *data);
	int (*data_transfer)(void *axi_addr, u32 data_size, u32 dir);
};

struct pcie_slt_cfg {
	u32 ldo5_offset;
	u32 ldo30_offset;
	u32 ldo5_normal;
	u32 ldo5_low;
	u32 ldo30_normal;
	u32 ldo30_low;
	struct pcie_ep_ops ep_ops;
};
struct pcie_slt_cfg g_pcie_slt_cfg[RC_NUMS];

struct pcie_slt {
	atomic_t ioctl_excl;
	atomic_t open_excl;
	int pcie_slt_major_number;
	struct class *pcie_slt_class;
};
struct pcie_slt g_pcie_slt;

struct pcie_scb_union {
	u32 rc_id;
	u32 loop_times;
	u32 test_result;
};

static int g_aspm_state[ASPM_STATE_MAX][ASPM_STATE_SINGLE_NUM] = {
	{L0_MODE, ERR_L0},
	{L0S_MODE, ERR_L0S},
	{L1_MODE, ERR_L1},
	{L1_MODE, ERR_L0S_L1}
};

static inline u32 set_test_result(u32 rc_id, u32 ready, u32 voltage, u32 test_result)
{
	return (((rc_id) << ID_SHIFT) | ((ready) << READY_SHIT) | \
		((voltage) << VOLTAGE_SHIT) | ((test_result) << TEST_SHIT));
}

int pcie_slt_hook_register(u32 rc_id, u32 device_type, int (*init)(void *),
			   int (*on)(void *), int (*off)(void *),
			   int (*setup)(void *),
			   int (*data_transfer)(void *, u32, u32))
{
	struct pcie_kport *pcie = get_pcie_by_id(rc_id);

	if (!pcie) {
		PCIE_PR_E("rc_id invalid");
		return -EINVAL;
	}

	if (!data_transfer || (pcie->dtsinfo.ep_device_type != device_type)) {
		PCIE_PR_E("Hook func invalid");
		g_pcie_slt_cfg[rc_id].ep_ops.data_transfer = NULL;
		return -EINVAL;
	}

	g_pcie_slt_cfg[rc_id].ep_ops.init = init;
	g_pcie_slt_cfg[rc_id].ep_ops.on = on;
	g_pcie_slt_cfg[rc_id].ep_ops.off = off;
	g_pcie_slt_cfg[rc_id].ep_ops.setup = setup;
	g_pcie_slt_cfg[rc_id].ep_ops.data_transfer = data_transfer;

	return RESULT_OK;
}
EXPORT_SYMBOL(pcie_slt_hook_register);

static int32_t pcie_get_ldoinfo(struct pcie_kport *pcie)
{
	struct device_node *np = NULL;
	u32 val[3] = {0}; /* bias volt: offset, normal_val, low_val */

	np = pcie->pci->dev->of_node;

	/* bias volt: offset, normal_val, low_val */
	if (of_property_read_u32_array(np, "ldo5", val, 3)) {
		PCIE_PR_E("Failed to get ldo5 info");
		return -1;
	}
	g_pcie_slt_cfg[pcie->rc_id].ldo5_offset = val[0]; /* Element 0 */
	g_pcie_slt_cfg[pcie->rc_id].ldo5_normal = val[1]; /* Element 1 */
	g_pcie_slt_cfg[pcie->rc_id].ldo5_low = val[2]; /* Element 2 */

	/* bias volt: offset, normal_val, low_val */
	if (of_property_read_u32_array(np, "ldo30", val, 3)) {
		PCIE_PR_E("Failed to get ldo30 info");
		return -1;
	}
	g_pcie_slt_cfg[pcie->rc_id].ldo30_offset = val[0]; /* Element 0 */
	g_pcie_slt_cfg[pcie->rc_id].ldo30_normal = val[1]; /* Element 1 */
	g_pcie_slt_cfg[pcie->rc_id].ldo30_low = val[2]; /* Element 2 */

	return 0;
}

static void pcie_set_vlotage(struct pcie_kport *pcie, enum pcie_voltage vol)
{
	if (pcie_get_ldoinfo(pcie))
		return;

	/* pcie and ufs has common voltage, call ufs handle before change the voltage */
	ufs_change_vol_pre();

	switch (vol) {
	case LOW_VOL:
		/* low voltage LDO5:1.72, LDO30:0.725 */
		pmic_write_reg(g_pcie_slt_cfg[pcie->rc_id].ldo5_offset,
				    g_pcie_slt_cfg[pcie->rc_id].ldo5_low);
		pmic_write_reg(g_pcie_slt_cfg[pcie->rc_id].ldo30_offset,
				    g_pcie_slt_cfg[pcie->rc_id].ldo30_low);
		break;
	case NORMAL_VOL:
		/* fall-through */
	default:
		/* normal voltage LDO5:1.8, LDO30:0.75 */
		pmic_write_reg(g_pcie_slt_cfg[pcie->rc_id].ldo5_offset,
				    g_pcie_slt_cfg[pcie->rc_id].ldo5_normal);
		pmic_write_reg(g_pcie_slt_cfg[pcie->rc_id].ldo30_offset,
				    g_pcie_slt_cfg[pcie->rc_id].ldo30_normal);
	}

	ufs_change_vol_post();
}

/*
 * wait_for_power_status - wait for link Entry lowpower mode
 * @mode: lowpower mode index
 */
static int wait_for_power_status(struct pcie_kport *pcie, enum aspm_stat mode)
{
	u32 status4 = 0;
	u32 status5 = 0;
	unsigned long prev_jffy;

	prev_jffy = jiffies;
	while (!(time_after(jiffies, prev_jffy + HZ / 10))) { /* 100ms timer */
		status4 = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);
		status5 = pcie_apb_ctrl_readl(pcie, SOC_PCIECTRL_STATE5_ADDR);
		switch (mode) {
		case L0_MODE:
			if ((status4 & LTSSM_STATUE_MASK) == LTSSM_L0)
				goto LTSSM_OK;
			break;
		case L0S_MODE:
			if ((status4 & LTSSM_STATUE_MASK) == LTSSM_L0S)
				goto LTSSM_OK;
			break;
		case L1_MODE:
			if ((status4 & LTSSM_STATUE_MASK) == LTSSM_L1)
				goto LTSSM_OK;
			break;
		case L1_1_MODE:
			if ((status5 & LTSSM_L1SS_MASK) == LTSSM_L1_1)
				goto LTSSM_OK;
			break;
		case L1_2_MODE:
			if ((status5 & LTSSM_L1SS_MASK) == LTSSM_L1_2)
				goto LTSSM_OK;
			break;
		default:
			PCIE_PR_E("unknown lowpower mode");
			break;
		}

		udelay(2); /* Re-detection after 2 us delay */
	}

	PCIE_PR_E("PCIECTRL_STATE4: 0x%x, PCIECTRL_STATE5: 0x%x",
		    status4, status5);
	return mode;

LTSSM_OK:
	return 0;
}

static int loopback_poweroff(void *data)
{
	int ret;
	struct pcie_kport *pcie_rc = &g_pcie_port[APR_RC_ID];
	struct pcie_kport *pcie_ep = &g_pcie_port[APR_EP_ID];

	if (pcie_rc->phy_ops && pcie_rc->phy_ops->irq_disable)
		pcie_rc->phy_ops->irq_disable(pcie_rc->phy);

	if (ltssm_enable(pcie_rc->rc_id, DISABLE)) {
		PCIE_PR_E("ltssm_disable fail");
		return  ERR_OTHER;
	}

	ret = pcie_power_ctrl(pcie_ep, RC_POWER_OFF);
	if (ret) {
		PCIE_PR_E("Failed to power off EP");
		return ret;
	}

	ret = pcie_power_ctrl(pcie_rc, RC_POWER_OFF);
	if (ret) {
		PCIE_PR_E("Failed to power off RC");
		return ret;
	}

	return RESULT_OK;
}

static int loopback_setup(void *data)
{
	int ret;
	struct pcie_kport *pcie_rc = &g_pcie_port[APR_RC_ID];
	struct pcie_kport *pcie_ep = &g_pcie_port[APR_EP_ID];

	ret = pcie_set_ep_mode(pcie_ep->rc_id);
	if (ret) {
		PCIE_PR_E("Failed to set_ep_mode");
		return ret;
	}

	/* Power on RC firstly */
	ret = pcie_power_ctrl(pcie_rc, RC_POWER_ON);
	if (ret) {
		PCIE_PR_E("Failed to power RC");
		dsm_pcie_dump_info(pcie_rc, DSM_ERR_POWER_ON);
		return ret;
	}

	/* Power on EP since EP need RC's reference clock */
	ret = pcie_power_ctrl(pcie_ep, RC_POWER_ON);
	if (ret) {
		PCIE_PR_E("Failed to power EP");
		dsm_pcie_dump_info(pcie_ep, DSM_ERR_POWER_ON);
		return ret;
	}

	if (pcie_ep->phy_ops && pcie_ep->phy_ops->irq_disable)
		pcie_ep->phy_ops->irq_disable(pcie_ep->phy);

	/* EP MAC initialization */
	ret = pcie_ep_mac_init(pcie_ep->rc_id);
	if (ret) {
		PCIE_PR_E("Failed to initialize EP MAC");
		return ret;
	}

	if (IS_ENABLED(CONFIG_PCIE_KPORT_MAY))
		(void)set_link_speed(pcie_rc->rc_id, GEN2);

	if (atomic_read(&pcie_rc->is_enumerated)) {
		if (pcie_kport_pm_control(POWERON, pcie_rc->rc_id)) {
			PCIE_PR_E("pcie_kport_pm_control fail");
			return  ERR_OTHER;
		}
	} else {
		if (pcie_kport_enumerate(pcie_rc->rc_id)) {
			PCIE_PR_E("enumerate fail");
			return  ERR_OTHER;
		}
	}

	if (pci_enable_device(pcie_rc->ep_dev)) {
		PCIE_PR_E("Failed to enable APR EP");
		return ERR_OTHER;
	}

	pci_set_master(pcie_rc->ep_dev);

	return RESULT_OK;
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

#define BAR_INDEX0                  0
#define ARP_DATA_TRANSFER_MAX_SIZE  0x400000
#define APR_DEVICE_TARGET_ADDR      0x100000000
static int loopback_datatransfer(void *ddr, u32 size, u32 dir)
{
	void __iomem *wl_cpu_base = NULL;
	int result = 0;
	struct pcie_kport *pcie_rc = &g_pcie_port[APR_RC_ID];
	struct pcie_kport *pcie_ep = &g_pcie_port[APR_EP_ID];
	u64 axi_addr;

	/* Get EP BAR0 base address */
	axi_addr = pci_resource_start(pcie_rc->ep_dev, BAR_INDEX0);
	if (!axi_addr) {
		PCIE_PR_E("Failed to get APR device base_addr");
		return ERR_OTHER;
	}

	wl_cpu_base = ioremap_nocache(axi_addr, SIZE_M);
	if (!wl_cpu_base) {
		PCIE_PR_E("Failed to ioremap loop_back_src");
		result = ERR_OTHER;
		goto TEST_FAIL_UNMAP;
	}

	if (size > ARP_DATA_TRANSFER_MAX_SIZE)
		size = ARP_DATA_TRANSFER_MAX_SIZE;

	pcie_kport_outbound_atu(pcie_rc->rc_id, ATU_REGION_INDEX0,
				ATU_TYPE_MEM, axi_addr, axi_addr,
				ARP_DATA_TRANSFER_MAX_SIZE);
	pcie_inbound_atu(pcie_ep->rc_id, ATU_REGION_INDEX0,
			       ATU_TYPE_MEM, APR_DEVICE_TARGET_ADDR,
			       axi_addr, ARP_DATA_TRANSFER_MAX_SIZE);

	/* Write to EP Ram */
	if (dir == H2D)
		pcie_data_trans(wl_cpu_base, SIZE_M, ddr, size);
	else if (dir == D2H)
		pcie_data_trans(ddr, SIZE_M, wl_cpu_base, size);
	else
		result = -EINVAL;

TEST_FAIL_UNMAP:
	iounmap(wl_cpu_base);
	return result;
}

static void show_diff_addr(struct pcie_kport *pcie,
		const void *src_addr, const void *dst_addr, unsigned int size)
{
	u32 src_val, dst_val;
	unsigned int i;
	int aer_pos;
	u32 val1, val2, val3, val4;

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos) {
		PCIE_PR_E("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return;
	}

	for (i = 0; i < 0x2; i++) {
		pci_read_config_dword(pcie->rc_dev, aer_pos + 0x10 * i + 0x0, &val1);
		pci_read_config_dword(pcie->rc_dev, aer_pos + 0x10 * i + 0x4, &val2);
		pci_read_config_dword(pcie->rc_dev, aer_pos + 0x10 * i + 0x8, &val3);
		pci_read_config_dword(pcie->rc_dev, aer_pos + 0x10 * i + 0xC, &val4);
		pr_err("0x%-8x: %8x %8x %8x %8x\n",
			aer_pos + 0x10 * i, val1, val2, val3, val4);
	}

	for (i = 0; i < size; i += REG_DWORD_ALIGN) {
		src_val = readl(src_addr + i);
		dst_val = readl(dst_addr + i);
		if (src_val != dst_val)
			pr_err("offset[0x%x]:src_val[0x%8x]  dst_val[0x%8x]\n",
				i, src_val, dst_val);
	}
}

#define DATA_ARR_SIZE 10
static u32 slt_data_transfer(struct pcie_kport *pcie, void *slt_data_src,
			     void *slt_data_cmp, u32 loop_times)
{
	int index, ret;
	u32 j, size, len_h2d, len_d2h;
	const u32 data_size[DATA_ARR_SIZE] = {
		0x4, 0x8, 0x10, 0x100, 0x1000, 0x4000,
		0xc000, 0x10000, 0x20000, 0x40000
	};

	if (!loop_times) {
		PCIE_PR_E("Data transfer loop times is 0, set default[1]");
		loop_times = 1;
	}

	for (j = 0; j < loop_times; j++) {
		for (index = 0; index < DATA_ARR_SIZE; index++) {
			size = data_size[index];
			ret = memset_s(slt_data_src, SLT_TEST_DATA_SIZE, SLT_RANDOM_DATA + index, SLT_TEST_DATA_SIZE);
			if (ret != EOK) {
				PCIE_PR_E("[%u, %d]:memset for slt_data_src failed", loop_times, index);
				return ERR_OTHER;
			}

			len_h2d = g_pcie_slt_cfg[pcie->rc_id].ep_ops.data_transfer(slt_data_src, size, H2D);
			len_d2h = g_pcie_slt_cfg[pcie->rc_id].ep_ops.data_transfer(slt_data_cmp, size, D2H);
			if (len_h2d != size || len_h2d != len_d2h) {
				PCIE_PR_E("[%u, %d]:data_transfer failed[0x%x, 0x%x]", loop_times, index, len_h2d, len_d2h);
				return ERR_DATA_TRANS;
			}

			ret = memcmp(slt_data_src, slt_data_cmp, size);
			if (ret) {
				PCIE_PR_E("[%u, %d]:memcmp failed[%d]", loop_times, index, ret);
				show_diff_addr(pcie, slt_data_src, slt_data_cmp, size);
				return ERR_DATA_CMP;
			}
			udelay(50); /* Each transmission interval is 50 us */
		}
	}

	return 0;
}

static u32 pcie_slt_prepare(struct pcie_kport *pcie)
{
	const u32 rc_idx = pcie->rc_id;

	if (g_pcie_slt_cfg[rc_idx].ep_ops.on) {
		if (g_pcie_slt_cfg[rc_idx].ep_ops.on(pcie)) {
			PCIE_PR_E("Device pwrup fail");
			return ERR_EP_ON;
		}
	}

	if (g_pcie_slt_cfg[rc_idx].ep_ops.setup) {
		if (g_pcie_slt_cfg[rc_idx].ep_ops.setup(pcie)) {
			PCIE_PR_E("Setup fail");
			return ERR_EP_SETUP;
		}
	}

	if ((!atomic_read(&(pcie->is_power_on))) || (!g_pcie_slt_cfg[rc_idx].ep_ops.data_transfer)) {
		PCIE_PR_E("Not ready");
		return ERR_EP_TRANS;
	}

	pcie_refclk_host_vote(&pcie->pci->pp, 1);

	return RESULT_OK;
}

static void pcie_slt_clear(struct pcie_kport *pcie)
{
	if (g_pcie_slt_cfg[pcie->rc_id].ep_ops.off)
		g_pcie_slt_cfg[pcie->rc_id].ep_ops.off(pcie);

	pcie_refclk_host_vote(&pcie->pci->pp, 0);
}

static int config_check_l0sl1(struct pcie_kport *pcie, enum link_aspm_state state)
{
	pcie_config_l0sl1(pcie->rc_id, state);
	if (wait_for_power_status(pcie, g_aspm_state[state][0])) {
		PCIE_PR_E("Enter link_aspm_state [%d] failed", state);
		return g_aspm_state[state][1]; /* element 1 err code */
	}

	PCIE_PR_I("Enter link_aspm_state [%d] successful", state);
	return 0;
}

static int loop_back_data_prepare(void __iomem **loop_back_cmp, void __iomem **loop_back_src)
{
	int ret;
	void __iomem *cmp_mem = NULL;
	void __iomem *src_mem = NULL;

	src_mem = vmalloc(SLT_TEST_DATA_SIZE);
	if (!src_mem) {
		PCIE_PR_E("Failed to alloc memory");
		return ERR_OTHER;
	}

	ret = memset_s(src_mem, SLT_TEST_DATA_SIZE, 0, SLT_TEST_DATA_SIZE);
	if (ret != EOK) {
		vfree(src_mem);
		PCIE_PR_E("Failed to set default val[0x0]");
		return ret;
	}

	cmp_mem = vmalloc(SLT_TEST_DATA_SIZE);
	if (!cmp_mem) {
		vfree(src_mem);
		PCIE_PR_E("Failed to alloc memory");
		return ERR_OTHER;
	}

	ret = memset_s(cmp_mem, SLT_TEST_DATA_SIZE, 0, SLT_TEST_DATA_SIZE);
	if (ret != EOK) {
		vfree(src_mem);
		vfree(cmp_mem);
		PCIE_PR_E("Failed to set default val[0x0]");
		return ret;
	}

	*loop_back_cmp = cmp_mem;
	*loop_back_src = src_mem;

	return 0;
}

static u32 set_loopback_test(struct pcie_kport *pcie, u32 loop_times)
{
	int ret, state;
	u32 result;
	void __iomem *loop_back_cmp = NULL;
	void __iomem *loop_back_src = NULL;

	ret = loop_back_data_prepare(&loop_back_cmp, &loop_back_src);
	if (ret)
		return ERR_OTHER;

	pcie_config_l1ss(pcie->rc_id, L1SS_CLOSE);
	result = config_check_l0sl1(pcie, ASPM_CLOSE);
	if (result)
		goto TEST_FAIL_CMP;

	result = slt_data_transfer(pcie, loop_back_src, loop_back_cmp, loop_times);
	if (result) {
		PCIE_PR_E("Data Transfer failed[PM_ASPM_CLOSE]");
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_I("Data Transfer successful[PM_ASPM_CLOSE]");

	for (state = ASPM_L0S; state < ASPM_STATE_MAX; state++) {
		result = config_check_l0sl1(pcie, state);
		if (result)
			goto TEST_FAIL_CMP;
	}

	pcie_config_l0sl1(pcie->rc_id, ASPM_L1);
	pcie_config_l1ss(pcie->rc_id, L1SS_ASPM_1_1);
	if (wait_for_power_status(pcie, L1_1_MODE)) {
		PCIE_PR_E("Enter L1_1 failed");
		result = ERR_L1_1;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_I("Enter L1_1 successful");

	pcie_config_l0sl1(pcie->rc_id, ASPM_L1);
	pcie_config_l1ss(pcie->rc_id, L1SS_PM_ASPM_ALL);
	if (wait_for_power_status(pcie, L1_2_MODE)) {
		PCIE_PR_E("Enter L1_2 failed");
		result = ERR_L1_2;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_I("Enter L1_2 successful");

	result = slt_data_transfer(pcie, loop_back_src, loop_back_cmp, loop_times);
	if (result) {
		PCIE_PR_E("Data Transfer failed[PM_ASPM_ALL]");
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_I("Data Transfer successful[PM_ASPM_ALL]");

	result = RESULT_OK;

TEST_FAIL_CMP:
	vfree(loop_back_cmp);
	vfree(loop_back_src);
	return result;
}

static u32 pcie_slt_vary_voltage_test(struct pcie_kport *pcie, u32 loop_times)
{
	u32 ret = RESULT_OK;
	u32 id = pcie->rc_id;

	PCIE_PR_I("+%s+", __func__);

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_BCM ||
	    pcie->dtsinfo.ep_device_type == EP_DEVICE_NODEV ||
	    pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_E("Bypass, ep_device_type: %u", pcie->dtsinfo.ep_device_type);
		return ret;
	}

	ret = pcie_slt_prepare(pcie);
	if (ret)
		return set_test_result(id, ret, DEFAULT_VOL, DEFAULT_RESULT);

	PCIE_PR_I("Test under default voltage");
	ret = set_loopback_test(pcie, loop_times);
	if (ret) {
		PCIE_PR_E("Default voltage pcie slt test fail");
		ret = set_test_result(id, OPS_OK, DEFAULT_VOL, ret);
		goto SLT_REALEASE;
	}

	PCIE_PR_I("Test under lower voltage");
	pcie_set_vlotage(pcie, LOW_VOL);
	ret = set_loopback_test(pcie, loop_times);
	if (ret) {
		PCIE_PR_E("Low voltage pcie slt test fail");
		ret = set_test_result(id, OPS_OK, LOW_VOL, ret);
		pcie_set_vlotage(pcie, NORMAL_VOL);
		goto SLT_REALEASE;
	}

	PCIE_PR_I("Test under normal voltage");
	pcie_set_vlotage(pcie, NORMAL_VOL);
	ret = set_loopback_test(pcie, loop_times);
	if (ret) {
		PCIE_PR_E("Normal voltage pcie slt test fail");
		ret = set_test_result(id, OPS_OK, NORMAL_VOL, ret);
		goto SLT_REALEASE;
	}

SLT_REALEASE:
	pcie_slt_clear(pcie);

	PCIE_PR_I("-%s-", __func__);
	return ret;
}

static int pcie_slt_lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1)
		return 0;

	atomic_dec(excl);
	return -1;
}

static inline void pcie_slt_unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

static void pcie_slt_resource_init(struct pcie_kport *pcie)
{
	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_APR) {
		PCIE_PR_I("PCIe0<->PCIe1 loopback mode, Register callback func");
		if (pcie_slt_hook_register(APR_RC_ID, EP_DEVICE_APR, NULL, NULL,
					loopback_poweroff, loopback_setup,
					loopback_datatransfer)) {
			PCIE_PR_E("Failed to register callback func");
			return;
		}
	}

	if (g_pcie_slt_cfg[pcie->rc_id].ep_ops.init)
		if (g_pcie_slt_cfg[pcie->rc_id].ep_ops.init(pcie))
			PCIE_PR_E("Init Device resouce Fail");
}

static long pcie_slt_ioctl(struct file *file, u_int cmd, unsigned long result)
{
	int ret;
	struct pcie_scb_union scb;

	ret = memset_s((void *)&scb, sizeof(struct pcie_scb_union), 0, sizeof(struct pcie_scb_union));
	if (ret != EOK)
		return (long)ret;

	if (pcie_slt_lock(&(g_pcie_slt.ioctl_excl)))
		return -EBUSY;

	switch (cmd) {
	case PCIETESTCMD:
		scb.rc_id = RC_ID;
		scb.loop_times = PCIE_SLT_DATA_DFT_LOOP_TIMES;
		scb.test_result = pcie_slt_vary_voltage_test(&g_pcie_port[scb.rc_id], scb.loop_times);
		ret = copy_to_user((void __user *)(uintptr_t)result,
				   (const void *)&(scb.test_result),
				   sizeof(unsigned int));
		break;

	case PCIETESTCMD_NEW:
		ret = copy_from_user((void *)&scb, (void __user *)(uintptr_t)result,
				     sizeof(struct pcie_scb_union));
		if (ret)
			goto FAIL;

		if (scb.rc_id >= g_rc_num || scb.loop_times <= 0 || scb.loop_times > PCIE_SLT_DATA_MAX_LOOP_TIMES) {
			PCIE_PR_E("InputPara is invalid, rc_id[%u], loop_times[%u]", scb.rc_id, scb.loop_times);
			ret = -1;
			goto FAIL;
		}

		scb.test_result = pcie_slt_vary_voltage_test(&g_pcie_port[scb.rc_id], scb.loop_times);
		ret = copy_to_user((void __user *)(uintptr_t)result,
				   (const void *)&scb,
				   sizeof(struct pcie_scb_union));
		break;

	default:
		ret = -1;
		break;
	}
FAIL:
	pcie_slt_unlock(&(g_pcie_slt.ioctl_excl));
	return (long)ret;
}

static int pcie_slt_open(struct inode *ip, struct file *fp)
{
	PCIE_PR_I("start");

	if (pcie_slt_lock(&(g_pcie_slt.open_excl)))
		return -EBUSY;

	PCIE_PR_I("success");
	return 0;
}

static int pcie_slt_release(struct inode *ip, struct file *fp)
{
	PCIE_PR_I("pcie_slt_release");

	pcie_slt_unlock(&(g_pcie_slt.open_excl));

	return 0;
}

static const struct file_operations pcie_slt_fops = {
	.unlocked_ioctl = pcie_slt_ioctl,
	.open           = pcie_slt_open,
	.release        = pcie_slt_release,
};

static int __init pcie_slt_init(void)
{
	int error = 0;
	struct device *pdevice = NULL;
	unsigned int i;

	if (!is_running_kernel_slt() && !runmode_is_factory())
		return 0;

	for (i = 0; i < g_rc_num; i++)
		pcie_slt_resource_init(&g_pcie_port[i]);

	/* semaphore initial */
	g_pcie_slt.pcie_slt_major_number = register_chrdev(0, PCIE_SLT_NAME, &pcie_slt_fops);
	if (g_pcie_slt.pcie_slt_major_number < 0) {
		PCIE_PR_E("register_chrdev error: %d", g_pcie_slt.pcie_slt_major_number);
		error = -EAGAIN;
		goto failed_register_pcie;
	}
	atomic_set(&g_pcie_slt.open_excl, 0);
	atomic_set(&g_pcie_slt.ioctl_excl, 0);

	g_pcie_slt.pcie_slt_class = class_create(THIS_MODULE, PCIE_SLT_NAME);
	if (IS_ERR(g_pcie_slt.pcie_slt_class)) {
		unregister_chrdev(g_pcie_slt.pcie_slt_major_number, PCIE_SLT_NAME);
		g_pcie_slt.pcie_slt_major_number = 0;
		error = PTR_ERR(g_pcie_slt.pcie_slt_class);
		PCIE_PR_E("class_create error");
		goto failed_register_pcie;
	}

	pdevice = device_create(g_pcie_slt.pcie_slt_class, NULL,
			MKDEV((unsigned int)g_pcie_slt.pcie_slt_major_number, 0),
			NULL, PCIE_SLT_NAME);
	if (IS_ERR(pdevice)) {
		class_destroy(g_pcie_slt.pcie_slt_class);
		unregister_chrdev(g_pcie_slt.pcie_slt_major_number, PCIE_SLT_NAME);
		g_pcie_slt.pcie_slt_class = NULL;
		g_pcie_slt.pcie_slt_major_number = 0;
		error = -EFAULT;
		PCIE_PR_E("device_create error");
		goto failed_register_pcie;
	}

	PCIE_PR_I("pcie-slt init ok!");

	return 0;

failed_register_pcie:
	return error;
}
static void __exit pcie_slt_cleanup(void)
{
	device_destroy(g_pcie_slt.pcie_slt_class,
			MKDEV((unsigned int)g_pcie_slt.pcie_slt_major_number, 0));
	class_destroy(g_pcie_slt.pcie_slt_class);
	unregister_chrdev(g_pcie_slt.pcie_slt_major_number, PCIE_SLT_NAME);
}
module_init(pcie_slt_init);
module_exit(pcie_slt_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PCIe kport slt driver");
#endif /* CONFIG_HISI_DEBUG_FS */
