#ifdef CONFIG_HISI_DEBUG_FS
#include "pcie-kirin-common.h"
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

/* PCIe0<->PCIe1 mode, PCIe1 is RC and PCIe0 is EP */
#define APR_RC_ID		1
#define APR_EP_ID		0

#define SLT_RANDOM_DATA		0x12
#define SLT_TEST_DATA_SIZE	0xc0000

#define SIZE_M			(0x100000)
#define PCIE_SLT_NAME		"pcie-slt"
#define RC_NUMS			2
#ifdef CONFIG_KIRIN_PCIE_APR
#define RC_ID		1
#else
#define RC_ID		0
#endif
#define PCIE_SLT_DATA_DFT_LOOP_TIMES	2
#define PCIE_SLT_DATA_MAX_LOOP_TIMES	10000
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

enum pcie_test_result {
	RESULT_OK = 0,
	ERR_DATA_TRANS,
	ERR_L0,
	ERR_L0S,
	ERR_L1,
	ERR_L0S_L1,
	ERR_L1_1,
	ERR_L1_2,
	ERR_EP_ON,
	ERR_EP_OFF,
	ERR_OTHER
};

enum pcie_voltage {
	NORMAL_VOL,
	LOW_VOL,
	HIGH_VOL,
};

#define PCIETESTCMD	        _IOWR('p', 0xc1, unsigned long)
#define PCIETESTCMD_NEW	        _IOWR('p', 0xc2, unsigned long)

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

struct kirin_pcie_scb_union {
	u32 rc_id;
	u32 loop_times;
	u32 test_result;
};

int pcie_slt_hook_register(u32 rc_id, u32 device_type, int (*init)(void *),
			int (*on)(void *), int (*off)(void *), int (*setup)(void *),
			int (*data_transfer)(void *, u32, u32))
{
	if (!data_transfer || rc_id > g_rc_num ||
		(g_kirin_pcie[rc_id].dtsinfo.ep_device_type != device_type)) {
		PCIE_PR_ERR("Hook func invalid");
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

static int32_t pcie_get_ldoinfo(struct kirin_pcie *pcie)
{
	struct device_node *np = NULL;
	u32 val[3] = {0, 0};

	np = pcie->pci->dev->of_node;

	if (of_property_read_u32_array(np, "ldo5", val, 3)) {
		PCIE_PR_ERR("Failed to get ldo5 info");
		return -1;
	}
	g_pcie_slt_cfg[pcie->rc_id].ldo5_offset = val[0];
	g_pcie_slt_cfg[pcie->rc_id].ldo5_normal = val[1];
	g_pcie_slt_cfg[pcie->rc_id].ldo5_low = val[2];

	if (of_property_read_u32_array(np, "ldo30", val, 3)) {
		PCIE_PR_ERR("Failed to get ldo30 info");
		return -1;
	}
	g_pcie_slt_cfg[pcie->rc_id].ldo30_offset = val[0];
	g_pcie_slt_cfg[pcie->rc_id].ldo30_normal = val[1];
	g_pcie_slt_cfg[pcie->rc_id].ldo30_low = val[2];

	return 0;
}

static void pcie_set_vlotage(struct kirin_pcie *pcie, enum pcie_voltage vol)
{

	if (pcie_get_ldoinfo(pcie))
		return;

	switch (vol) {
	case LOW_VOL:
		/* low voltage LDO5:1.72, LDO30:0.725 */
		hisi_pmic_reg_write(g_pcie_slt_cfg[pcie->rc_id].ldo5_offset,
				g_pcie_slt_cfg[pcie->rc_id].ldo5_low);
		hisi_pmic_reg_write(g_pcie_slt_cfg[pcie->rc_id].ldo30_offset,
				g_pcie_slt_cfg[pcie->rc_id].ldo30_low);
		break;
	case NORMAL_VOL:
	default:
		/* normal voltage LDO5:1.8, LDO30:0.75 */
		hisi_pmic_reg_write(g_pcie_slt_cfg[pcie->rc_id].ldo5_offset,
				g_pcie_slt_cfg[pcie->rc_id].ldo5_normal);
		hisi_pmic_reg_write(g_pcie_slt_cfg[pcie->rc_id].ldo30_offset,
				g_pcie_slt_cfg[pcie->rc_id].ldo30_normal);
	}
}

#define LDO30_OFFSET		0x74
#define LDO37_OFFSET		0x79
#define BUCK2_OFFSET		0x4d
#define BUCK9_OFFSET		0x57

/* normal, low, high */
static u32 ldo30_vals[3] = {0x19, 0x14, 0x20};
static u32 ldo37_vals[3] = {0xA, 0x4, 0x16};
static u32 buck2_vals[3] = {0x14, 0x14, 0x1F};
static u32 buck9_vals[3] = {0x28, 0x28, 0x31};

/* For Phoenix Only */
int pcie_slt_set_voltage(enum pcie_voltage volt)
{
	switch (volt) {
	case NORMAL_VOL:
		/* normal voltage LDO37:1.2, LDO30:0.75 */
		hisi_pmic_reg_write(BUCK9_OFFSET, buck9_vals[NORMAL_VOL]);
		hisi_pmic_reg_write(BUCK2_OFFSET, buck2_vals[NORMAL_VOL]);
		hisi_pmic_reg_write(LDO37_OFFSET, ldo37_vals[NORMAL_VOL]);
		hisi_pmic_reg_write(LDO30_OFFSET, ldo30_vals[NORMAL_VOL]);
		break;
	case LOW_VOL:
		/* low voltage LDO37:1.14, LDO30:0.7 */
		hisi_pmic_reg_write(BUCK9_OFFSET, buck9_vals[LOW_VOL]);
		hisi_pmic_reg_write(BUCK2_OFFSET, buck2_vals[LOW_VOL]);
		hisi_pmic_reg_write(LDO37_OFFSET, ldo37_vals[LOW_VOL]);
		hisi_pmic_reg_write(LDO30_OFFSET, ldo30_vals[LOW_VOL]);
		break;
	case HIGH_VOL:
		/* high voltage LDO37:1.32, LDO30:0.82 */
		hisi_pmic_reg_write(BUCK9_OFFSET, buck9_vals[HIGH_VOL]);
		hisi_pmic_reg_write(BUCK2_OFFSET, buck2_vals[HIGH_VOL]);
		hisi_pmic_reg_write(LDO37_OFFSET, ldo37_vals[HIGH_VOL]);
		hisi_pmic_reg_write(LDO30_OFFSET, ldo30_vals[HIGH_VOL]);
		break;
	default:
		PCIE_PR_ERR("Invalid Parameter");
		return -1;
	}

	return 0;
}


/*
 * wait_for_power_status - wait for link Entry lowpower mode
 * @mode: lowpower mode index
 */
int wait_for_power_status(struct kirin_pcie *pcie, enum aspm_stat mode)
{
	u32 status4 = 0;
	u32 status5 = 0;
	unsigned long prev_jffy;

	prev_jffy = jiffies;
	while (!(time_after(jiffies, prev_jffy + HZ / 10))) {
		status4 = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR);
		status5 = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE5_ADDR);
		switch (mode) {
		case L0_MODE:
			if ((status4 & 0x3f) == 0x11)
				goto LTSSM_OK;
			break;
		case L0S_MODE:
			if ((status4 & 0x3f) == 0x12)
				goto LTSSM_OK;
			break;
		case L1_MODE:
			if ((status4 & 0x3f) == 0x14)
				goto LTSSM_OK;
			break;
		case L1_1_MODE:
			if (((status5 & BIT(15)) == 0) && ((status5 & BIT(14)) == BIT(14)))
				goto LTSSM_OK;
			break;
		case L1_2_MODE:
			if (((status5 & BIT(15)) == BIT(15)) && ((status5 & BIT(14)) == BIT(14)))
				goto LTSSM_OK;
			break;
		default:
			PCIE_PR_ERR("unknown lowpower mode");
			break;
		}

		udelay(2);
	}

	PCIE_PR_ERR("PCIECTRL_STATE4: 0x%x, PCIECTRL_STATE5: 0x%x", status4, status5);
	return mode;

LTSSM_OK:
	return 0;
}

static int apr_setup(void *data)
{
	int ret = 0;
	struct kirin_pcie *pcie_rc = &g_kirin_pcie[APR_RC_ID];
	struct kirin_pcie *pcie_ep = &g_kirin_pcie[APR_EP_ID];

	/* Power on RC firstly */
	ret = kirin_pcie_power_ctrl(pcie_rc, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power RC");
		dsm_pcie_dump_info(pcie_rc, DSM_ERR_POWER_ON);
		return ret;
	}

	/* Power on EP since EP need RC's reference clock */
	ret = kirin_pcie_power_ctrl(pcie_ep, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power RC");
		dsm_pcie_dump_info(pcie_ep, DSM_ERR_POWER_ON);
		return ret;
	}

	/* EP MAC initialization */
	ret = kirin_pcie_ep_mac_init(pcie_ep->rc_id);
	if (ret) {
		PCIE_PR_ERR("Failed to initialize EP MAC");
		return ret;
	}

	if (kirin_pcie_enumerate(pcie_rc->rc_id)) {
		PCIE_PR_ERR("kirin_pcie_enumerate fail");
		return  ERR_OTHER;
	}

	if (pci_enable_device(pcie_rc->ep_dev)) {
		PCIE_PR_ERR("Failed to enable APR EP");
		return ERR_OTHER;
	}

	pci_set_master(pcie_rc->ep_dev);

	return RESULT_OK;
}

#define BAR_INDEX0			0
#define ARP_DATA_TRANSFER_MAX_SIZE	0x400000
#define APR_DEVICE_TARGET_ADDR		0x100000000
static int apr_datatransfer(void *ddr, u32 size, u32 dir)
{
	void __iomem *wl_cpu_base = NULL;
	int result = RESULT_OK;
	struct kirin_pcie *pcie_rc = &g_kirin_pcie[APR_RC_ID];
	struct kirin_pcie *pcie_ep = &g_kirin_pcie[APR_EP_ID];
	/* Get EP BAR0 base address */
	u64 axi_addr = pci_resource_start(pcie_rc->ep_dev, BAR_INDEX0);

	if (!axi_addr) {
		PCIE_PR_ERR("Failed to get APR device base_addr");
		return ERR_OTHER;
	}

	wl_cpu_base = ioremap_nocache(axi_addr, 4*SIZE_M);
	if (!wl_cpu_base) {
		PCIE_PR_ERR("Failed to ioremap loop_back_src");
		result = ERR_OTHER;
		goto TEST_FAIL_UNMAP;
	}

	if (size > ARP_DATA_TRANSFER_MAX_SIZE)
		size = ARP_DATA_TRANSFER_MAX_SIZE;

	result = size;
	kirin_pcie_outbound_atu(pcie_rc->rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, KIRIN_PCIE_ATU_TYPE_MEM,
				axi_addr, axi_addr, ARP_DATA_TRANSFER_MAX_SIZE);
	kirin_pcie_inbound_atu(pcie_ep->rc_id, KIRIN_PCIE_ATU_REGION_INDEX0, KIRIN_PCIE_ATU_TYPE_MEM,
				APR_DEVICE_TARGET_ADDR, axi_addr, ARP_DATA_TRANSFER_MAX_SIZE);

	/* Write to EP Ram */
	if (dir == H2D)
		pcie_data_cpy((uintptr_t)wl_cpu_base, (uintptr_t)ddr, size);
	else if (dir == D2H)
		pcie_data_cpy((uintptr_t)ddr, (uintptr_t)wl_cpu_base, size);
	else
		result = -EINVAL;

TEST_FAIL_UNMAP:
	iounmap(wl_cpu_base);
	return result;
}

#define DATA_ARR_SIZE	10
static int slt_data_transfer(struct kirin_pcie *pcie, void *slt_data_src,
		void *slt_data_cmp, u32 loop_times)
{
	int index = 0;
	u32 j = 0;
	u32 data_size[DATA_ARR_SIZE] = {0x4, 0x8, 0x10, 0x100, 0x1000, 0x4000, 0xc000, 0x10000, 0x20000, 0x40000};
	int ret0, ret1, ret2, ret3;

	if (!loop_times) {
		PCIE_PR_ERR("Data transfer loop times is 0, set default[1].");
		loop_times = 1;
	}

	for (j = 0; j < loop_times; j++) {
		for (index = 0; index < DATA_ARR_SIZE; index++) {
			ret0 = memset_s(slt_data_src, SLT_TEST_DATA_SIZE,
				SLT_RANDOM_DATA + index, SLT_TEST_DATA_SIZE);
			ret1 = g_pcie_slt_cfg[pcie->rc_id].ep_ops.data_transfer(slt_data_src, data_size[index], H2D);
			ret2 = g_pcie_slt_cfg[pcie->rc_id].ep_ops.data_transfer(slt_data_cmp, data_size[index], D2H);
			ret3 = memcmp(slt_data_src, slt_data_cmp, ret2);

			if (ret0 != EOK || ret1 != ret2 || ret3) {
				PCIE_PR_ERR("Data transfer failed[%d, %d], ret1: %d, ret2: %d, ret3: %d", loop_times,
							index, ret1, ret2, ret3);
				return ERR_DATA_TRANS;
			}

			udelay(50);
		}
	}

	return 0;
}

static int pcie_slt_prepare(struct kirin_pcie *pcie)
{
	u32 rc_idx = 0;

	rc_idx = pcie->rc_id;
	if (g_pcie_slt_cfg[rc_idx].ep_ops.on && g_pcie_slt_cfg[rc_idx].ep_ops.on(pcie)) {
		PCIE_PR_ERR("Device pwrup fail.");
		return ERR_EP_ON;
	}

	if (g_pcie_slt_cfg[rc_idx].ep_ops.setup && g_pcie_slt_cfg[rc_idx].ep_ops.setup(pcie)) {
		PCIE_PR_ERR("Setup fail");
		return ERR_OTHER;
	}

	if ((!atomic_read(&(pcie->is_power_on))) || (!g_pcie_slt_cfg[rc_idx].ep_ops.data_transfer)) {
		PCIE_PR_ERR("Not ready");
		return ERR_OTHER;
	}

	return RESULT_OK;
}

static int pcie_slt_clear(struct kirin_pcie *pcie)
{
	if (g_pcie_slt_cfg[pcie->rc_id].ep_ops.off)
		g_pcie_slt_cfg[pcie->rc_id].ep_ops.off(pcie);

	return RESULT_OK;
}

enum pcie_test_result set_loopback_test(struct kirin_pcie *pcie, u32 loop_times)
{
	int ret = 0;
	enum pcie_test_result result;
	void __iomem *loop_back_cmp;
	void __iomem *loop_back_src;

	loop_back_src = vmalloc(SLT_TEST_DATA_SIZE * 2);
	if (!loop_back_src) {
		PCIE_PR_ERR("Failed to alloc memory");
		return ERR_OTHER;
	}

	ret = memset_s(loop_back_src, SLT_TEST_DATA_SIZE * 2,
			0, SLT_TEST_DATA_SIZE * 2);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		result = ret;
		goto TEST_FREE_SRC;
	}

	loop_back_cmp = vmalloc(SLT_TEST_DATA_SIZE * 2);
	if (!loop_back_cmp) {
		PCIE_PR_ERR("Failed to alloc memory");
		result = ERR_OTHER;
		goto TEST_FREE_SRC;
	}

	ret = memset_s(loop_back_cmp, SLT_TEST_DATA_SIZE * 2,
		0, SLT_TEST_DATA_SIZE * 2);
	if (ret != EOK) {
		PCIE_PR_ERR("Default Val\n");
		result = ret;
		goto TEST_FAIL_CMP;
	}

	kirin_pcie_config_l1ss(pcie->rc_id, L1SS_CLOSE);
	kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_CLOSE);
	if (wait_for_power_status(pcie, L0_MODE)) {
		PCIE_PR_ERR("Enter L0 failed");
		result = ERR_L0;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Enter L0 successful");

	ret = slt_data_transfer(pcie, loop_back_src, loop_back_cmp, loop_times);
	if (ret) {
		PCIE_PR_ERR("Data Transfer failed[PM_ASPM_CLOSE]");
		result = ERR_DATA_TRANS;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Data Transfer successful[PM_ASPM_CLOSE]");

	kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_L0S);
	if (wait_for_power_status(pcie, L0S_MODE)) {
		PCIE_PR_ERR("Enter L0s failed");
		result = ERR_L0S;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Enter L0s successful");

	kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_L1);
	if (wait_for_power_status(pcie, L1_MODE)) {
		PCIE_PR_ERR("Enter L1 failed");
		result = ERR_L1;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Enter L1 successful");

	kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_L0S_L1);
	if (wait_for_power_status(pcie, L1_MODE)) {
		PCIE_PR_ERR("Enter L0s&L1 failed");
		result = ERR_L0S_L1;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Enter L0s&L1 successful");

	kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_L1);
	kirin_pcie_config_l1ss(pcie->rc_id, L1SS_ASPM_1_1);
	if (wait_for_power_status(pcie, L1_1_MODE)) {
		PCIE_PR_ERR("Enter L1_1 failed");
		result = ERR_L1_1;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Enter L1_1 successful");

	kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_L1);
	kirin_pcie_config_l1ss(pcie->rc_id, L1SS_PM_ASPM_ALL);
	if (wait_for_power_status(pcie, L1_2_MODE)) {
		PCIE_PR_ERR("Enter L1_2 failed");
		result = ERR_L1_2;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Enter L1_2 successful");

	ret = slt_data_transfer(pcie, loop_back_src, loop_back_cmp, loop_times);
	if (ret) {
		PCIE_PR_ERR("Data Transfer failed[PM_ASPM_ALL]");
		result = ERR_DATA_TRANS;
		goto TEST_FAIL_CMP;
	}
	PCIE_PR_INFO("Data Transfer successful[PM_ASPM_ALL]");

	result = RESULT_OK;

TEST_FAIL_CMP:
	vfree(loop_back_cmp);
TEST_FREE_SRC:
	vfree(loop_back_src);
	return result;
}
EXPORT_SYMBOL(set_loopback_test);

enum pcie_test_result pcie_slt_vary_voltage_test(struct kirin_pcie *pcie, u32 loop_times)
{
	enum pcie_test_result ret = RESULT_OK;

	PCIE_PR_INFO("++");

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_BCM ||
			pcie->dtsinfo.ep_device_type == EP_DEVICE_NODEV ||
			pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
		PCIE_PR_ERR("Bypass, ep_device_type: %d", pcie->dtsinfo.ep_device_type);
		return ret;
	}

	if (pcie_slt_prepare(pcie))
		return ERR_OTHER;

	PCIE_PR_INFO("Test under default voltage");
	ret = set_loopback_test(pcie, loop_times);
	if (ret) {
		PCIE_PR_ERR("Default voltage pcie slt test fail");
		goto SLT_REALEASE;
	}

	PCIE_PR_INFO("Test under lower voltage");
	pcie_set_vlotage(pcie, LOW_VOL);
	ret = set_loopback_test(pcie, loop_times);
	if (ret) {
		PCIE_PR_ERR("Low voltage pcie slt test fail");
		pcie_set_vlotage(pcie, NORMAL_VOL);
		goto SLT_REALEASE;
	}

	PCIE_PR_INFO("Test under normal voltage");
	pcie_set_vlotage(pcie, NORMAL_VOL);
	ret = set_loopback_test(pcie, loop_times);
	if (ret) {
		PCIE_PR_ERR("Normal voltage pcie slt test fail");
		goto SLT_REALEASE;
	}

SLT_REALEASE:
	(void)pcie_slt_clear(pcie);

	PCIE_PR_INFO("--");
	return ret;
}

static inline int pcie_slt_lock(atomic_t *excl)
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

void pcie_slt_resource_init(struct kirin_pcie *pcie)
{
	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_APR) {
		PCIE_PR_INFO("PCIe0<->PCIe1 loopback mode, Register callback func");
		if (pcie_slt_hook_register(APR_RC_ID, EP_DEVICE_APR, NULL, NULL,
			NULL, apr_setup, apr_datatransfer)) {
			PCIE_PR_ERR("Failed to register callback func");
			return;
		}
	}

	if (g_pcie_slt_cfg[pcie->rc_id].ep_ops.init && g_pcie_slt_cfg[pcie->rc_id].ep_ops.init(pcie))
		PCIE_PR_ERR("Init Device resouce Fail.");
}

static long pcie_slt_ioctl(struct file *file, u_int cmd, unsigned long result)
{
	int ret = 0;
	struct kirin_pcie_scb_union scb;

	(void)memset_s((void *)&scb, sizeof(struct kirin_pcie_scb_union), 0, sizeof(struct kirin_pcie_scb_union));

	if (pcie_slt_lock(&(g_pcie_slt.ioctl_excl)))
		return -EBUSY;

	switch (cmd) {
	case PCIETESTCMD:
		scb.rc_id = RC_ID;
		scb.loop_times = PCIE_SLT_DATA_DFT_LOOP_TIMES;
		scb.test_result = pcie_slt_vary_voltage_test(&g_kirin_pcie[scb.rc_id], scb.loop_times);
		ret = copy_to_user((void __user *)(uintptr_t)result, (const void *)&(scb.test_result), sizeof(unsigned int));
		break;

	case PCIETESTCMD_NEW:
		ret = copy_from_user((void *)&scb, (void __user *)(uintptr_t)result, sizeof(struct kirin_pcie_scb_union));
		if (ret)
			goto FAIL;

		if (scb.rc_id >= g_rc_num || 0 >= scb.loop_times || PCIE_SLT_DATA_MAX_LOOP_TIMES < scb.loop_times) {
			PCIE_PR_ERR("InputPara is invalid, rc_id[%d], loop_times[%d].", scb.rc_id, scb.loop_times);
			ret = -1;
			goto FAIL;
		}

		scb.test_result = pcie_slt_vary_voltage_test(&g_kirin_pcie[scb.rc_id], scb.loop_times);
		ret = copy_to_user((void __user *)(uintptr_t)result, (const void *)&scb, sizeof(struct kirin_pcie_scb_union));
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
	PCIE_PR_INFO("start");

	if (pcie_slt_lock(&(g_pcie_slt.open_excl)))
		return -EBUSY;

	PCIE_PR_INFO("success");
	return 0;
}

static int pcie_slt_release(struct inode *ip, struct file *fp)
{

	PCIE_PR_INFO("pcie_slt_release");

	pcie_slt_unlock(&(g_pcie_slt.open_excl));

	return 0;
}

static const struct file_operations pcie_slt_fops = {
	.unlocked_ioctl    = pcie_slt_ioctl,
	.open              = pcie_slt_open,
	.release           = pcie_slt_release,
};

static int __init pcie_slt_init(void)
{
	int error;
	struct device *pdevice = NULL;
	unsigned int i = 0;

	if (!is_running_kernel_slt() && !runmode_is_factory())
		return 0;

	for (i = 0; i < g_rc_num; i++)
		pcie_slt_resource_init(&g_kirin_pcie[i]);

	/* semaphore initial */
	g_pcie_slt.pcie_slt_major_number = register_chrdev(0, PCIE_SLT_NAME, &pcie_slt_fops);
	if (g_pcie_slt.pcie_slt_major_number < 0) {
		PCIE_PR_ERR("register_chrdev error: %d.", g_pcie_slt.pcie_slt_major_number);
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
		PCIE_PR_ERR("class_create error.");
		goto failed_register_pcie;
	}

	pdevice = device_create(g_pcie_slt.pcie_slt_class, NULL, MKDEV(g_pcie_slt.pcie_slt_major_number, 0), NULL, PCIE_SLT_NAME);
	if (IS_ERR(pdevice)) {
		class_destroy(g_pcie_slt.pcie_slt_class);
		unregister_chrdev(g_pcie_slt.pcie_slt_major_number, PCIE_SLT_NAME);
		g_pcie_slt.pcie_slt_class = NULL;
		g_pcie_slt.pcie_slt_major_number = 0;
		error = -EFAULT;
		PCIE_PR_ERR("device_create error.");
		goto failed_register_pcie;
	}

	PCIE_PR_INFO("pcie-slt init ok!");

	return 0;

failed_register_pcie:
	return error;
}
static void __exit pcie_slt_cleanup(void)
{
	device_destroy(g_pcie_slt.pcie_slt_class, MKDEV(g_pcie_slt.pcie_slt_major_number, 0));
	class_destroy(g_pcie_slt.pcie_slt_class);
	unregister_chrdev(g_pcie_slt.pcie_slt_major_number, PCIE_SLT_NAME);
}
module_init(pcie_slt_init);
module_exit(pcie_slt_cleanup);
MODULE_DESCRIPTION("Hisilicon Kirin pcie driver");
MODULE_LICENSE("GPL");
#endif
