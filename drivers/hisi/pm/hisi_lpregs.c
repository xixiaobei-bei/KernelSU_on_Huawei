/*
 * Copyright (C) 2015 Hisilicon
 * License terms: GNU General Public License (GPL) version 2
 */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpu_pm.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/clockchips.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/irqflags.h>
#include <linux/arm-cci.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/slab.h>
#include <linux/console.h>
#include <linux/pm_wakeup.h>
#include <soc_crgperiph_interface.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_uart_interface.h>
#include <soc_sctrl_interface.h>
#include "hisi_lpregs.h"
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/seq_buf.h>
#include <linux/uaccess.h>
#ifdef CONFIG_HISI_PMIC_PMU_SPMI
#include <linux/mfd/hisi_pmic.h>
#endif
#include <securec.h>
#include <linux/hisi/hisi_gpio_auto_gen.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"

#ifdef CONFIG_HISI_LPMCU_BB
#include "m3_rdr_ddr_map.h"
#include <lpmcu_runtime.h>
#endif

#pragma GCC diagnostic pop

#ifdef CONFIG_HUAWEI_DUBAI
#include <chipset_common/dubai/dubai.h>
#endif

#define PM_BUFFER_SIZE						(256)

#include <pm_def.h>
#include <m3_interrupts.h>

#define WAKE_IRQ_NUM_MAX 32

#define DEBG_SUSPEND_PRINTK		(1<<0)
#define DEBG_SUSPEND_IO_SHOW		(1<<1)
#define DEBG_SUSPEND_PMU_SHOW	(1<<2)
#define DEBG_SUSPEND_IO_SET		(1<<3)
#define DEBG_SUSPEND_PMU_SET		(1<<4)
#define DEBG_SUSPEND_IO_S_SET		(1<<5)
#define DEBG_SUSPEND_RTC_EN		(1<<6)
#define DEBG_SUSPEND_TIMER_EN		(1<<7)
#define DEBG_SUSPEND_WAKELOCK	(1<<8)
#define DEBG_SUSPEND_AUDIO		(1<<9)
#define DEBG_SUSPEND_CLK_SHOW	(1<<10)
#define DEBG_SUSPEND_IPC_DATA_SHOW	(1<<11)
#define DEBUG_INFO				(1<<31)

/* pmu */
#define PMUSSI_REG(x) (sysreg_base.pmic_base + ((x)<<2))

/* ipc processor max */
#define IPC_PROCESSOR_MAX		6
/* ipc mailbox max */
#define NSIPC_MAILBOX_MAX		27

#define AP_IPC_PROC_MAX			2

#define PROCESSOR_IOMCU_LENGTH  5
#define IPC_MBXDATA_MIN			0
#define IPC_MBXDATA_MAX			8
#ifdef CONFIG_HUAWEI_DUBAI
#define IPC_MBXDATA_TAG			2
#define get_sharemem_data(m)	((m) & 0xFFFF)
#define get_sharemem_source(m)	((m) & 0xFF)
#endif
#define ipc_mbx_source_offset(m)	((m) << 6)
#define ipc_mbx_dstatus_offset(m)	(0x0C + ((m) << 6))
#define ipc_mbxdata_offset(m, idex)	(0x20 + 4 * (idex) + ((m) << 6))

#define CLK_DEBUG_FILE_MODE		0400
#define PMU_DEBUG_FILE_MODE		0400
#define IO_DEBUG_FILE_MODE		0400
#define CFG_DEBUG_FILE_MODE		(0400 | 0200)
#define DEBUGSR_DEBUG_FILE_MODE	0400
#define NO_SEQFILE 0
#define LOWPM_MSG(seq_file, fmt, args ...) \
	do { \
		if (seq_file == NO_SEQFILE) \
			pr_info(fmt"\n", ##args); \
		else \
			seq_printf(seq_file, fmt"\n", ##args); \
	} while (0)
#define LOWPM_MSG_CONT(seq_file, fmt, args ...) \
	do { \
		if (seq_file == NO_SEQFILE) \
			pr_cont(fmt, ##args); \
		else \
			seq_printf(seq_file, fmt, ##args); \
	} while (0)

#define CRG_PERRSTDIS2_ip_prst_uart0_BIT    SOC_CRGPERIPH_PERRSTDIS2_ip_prst_uart0_START

/* for hi6250 and hi3660 */
#ifndef SOC_ACPU_PMC_BASE_ADDR
#define SOC_ACPU_PMC_BASE_ADDR	SOC_ACPU_PMCTRL_BASE_ADDR
#endif
#define CTRL_NAME_MAX_LEN		(sizeof(char) * 10)

/* struct sysreg_bases to hold the base address of some system registers. */
struct sysreg_base_addr {
	void __iomem *uart_base;
	void __iomem *pctrl_base;
	void __iomem *pmctrl_base;
	void __iomem *sysctrl_base;
	void __iomem **gpio_base;
	void __iomem *crg_base;
	void __iomem *pmic_base;
	void __iomem *reserved_base;
	void __iomem *nsipc_base;
#ifdef	AP_AO_NSIPC_IRQ
	void __iomem *ao_nsipc_base;
#endif
};

struct sysreg_base_addr sysreg_base;

struct lp_clk {
	unsigned int reg_base;
	unsigned int reg_off;
	unsigned int bit_idx;
	const char *clk_name;
	unsigned int status;
};

unsigned int g_io_group_num;
unsigned int g_lpmcu_irq_num;
unsigned int g_lp_pmu_num;
unsigned int g_pmu_addr_end;
unsigned int g_lp_clk_num;
const char **g_lpmcu_irq_name;
static const char *g_plat_name;
#ifdef CONFIG_PM_SEC_IO
void __iomem **g_mg_reg;
void __iomem **g_cg_reg;
int g_mg_sec_reg_num;
int g_cg_sec_reg_num;
#endif


unsigned int g_lp_fpga_flag;

char *processor_name[IPC_PROCESSOR_MAX] = {
	"gic1",
	"gic2",
	"iomcu",
	"lpm3",
	"hifi",
	"modem"
};

#define PMU_NAME(a, b, c, d) \
	{ .name = a, .module = b, .status = c, .status_name = d}

/* note: g_hisi_pmu_idx[k] must be matched with g_pmuregs_lookups[k],
 * otherwise the array may be out of range.
 */
static struct pmu_idx **g_hisi_pmu_idx;
#define PMU_LOW(x, y, z) \
		{ .ucoffset = (x), .cval = (y), .cmask = (z), .old_val = 0,}
static struct pmuregs **g_pmuregs_lookups;

static struct lp_clk **g_clk_lookups;

static unsigned int g_usavedcfg;
static int g_suspended;

static struct wakeup_source lowpm_wake_lock;


#define BUFFER_LENGTH		40
#define DEBUG_LOG_BUF_SIZE	(1024 * 1024)

unsigned int g_boardid;
/* HISI_CHIPID_DIG_NUMS is defined in  linux/hisi/hisi_gpio_auto_gen.h */
int g_chipid[HISI_CHIPID_DIG_NUMS];

#define lowpm_kfree_array(arr, size, iter) \
	do { \
		for (iter = 0; iter < size; iter++) { \
			if (arr[iter] != NULL) { \
				kfree(arr[iter]); \
				arr[iter] = NULL; \
			} \
		} \
		kfree(arr); \
		arr = NULL; \
	} while (0)

static void hisi_lp_read_chipid(void)
{
	int ret, i;

	for (i = 0; i < HISI_CHIPID_DIG_NUMS; i++)
		g_chipid[i] = 0;
	ret = of_property_read_u32_array(of_find_node_by_path("/"),
			"hisi,chipid", &g_chipid[0], HISI_CHIPID_DIG_NUMS);
	if (ret != 0) {
		pr_err("%s: not find chipid, but may not wrong.\n", __func__);
		return;
	}

	LOWPM_MSG(NO_SEQFILE, "%s: chipid is:\t", __func__);
	for (i = 0; i < HISI_CHIPID_DIG_NUMS; i++)
		LOWPM_MSG_CONT(NO_SEQFILE, "%x ", g_chipid[i]);

	LOWPM_MSG_CONT(NO_SEQFILE, "\n");
}

static int compare_chipid(int *chipid)
{
	int i;

	for (i = 0; i < HISI_CHIPID_DIG_NUMS; i++) {
		if (chipid[i] != g_chipid[i])
			return -1;
	}

	return 0;
}

static int hisi_lp_read_boardid(void)
{
	int ret;
	unsigned int boardid[4];

	ret = of_property_read_u32_array(of_find_node_by_path("/"),
			"hisi,boardid", &boardid[0], 4);
	if (ret != 0) {
		pr_err("%s: not find boardid.\n", __func__);
		return -ENODEV;
	}

	g_boardid = boardid[0] * 1000 + boardid[1] * 100 + boardid[2] * 10 + boardid[3];

	pr_info("%s: boardid %x\n", __func__, g_boardid);

	return 0;
}

static int map_io_regs(void)
{
	unsigned int i;
	int ret;
	struct device_node *np = NULL;
	char *io_buffer = NULL;

	hisi_lp_read_chipid();
	ret = hisi_lp_read_boardid();
	if (ret != 0)
		goto err;

	io_buffer = kcalloc(BUFFER_LENGTH, sizeof(char), GFP_KERNEL);
	if (io_buffer == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	/* find io group max num */
	i = 0;
	while (1) {
		ret = memset_s(io_buffer, BUFFER_LENGTH * sizeof(char), 0, BUFFER_LENGTH * sizeof(char));
		if (ret != EOK) {
			pr_err("%s: memset_s io_buffer err!\n", __func__);
			goto err_free_buffer;
		}
		ret = snprintf_s(io_buffer, BUFFER_LENGTH * sizeof(char),
				BUFFER_LENGTH * sizeof(char) - 1, "arm,primecell%d", i);
		if (ret < 0)
			pr_err("%s: snprintf_s io_buffer err.\n", __func__);
		np = of_find_compatible_node(NULL, NULL, io_buffer);
		if (np == NULL) {
			pr_err("%s[%d]: gpio%d not find.\n", __func__, __LINE__, i);
			if (i == 0) {
				pr_err("%s: no group num!\n", __func__);
				ret = -ENODEV;
				goto err_free_buffer;
			}
			break;
		}
		i++;
		of_node_put(np);
	}
	g_io_group_num = i;
	pr_info("%s: find gpio group num: %u\n", __func__, g_io_group_num);

	sysreg_base.gpio_base = kcalloc(g_io_group_num, sizeof(void __iomem *), GFP_KERNEL);
	if (sysreg_base.gpio_base == NULL) {
		ret = -ENOMEM;
		goto err_free_buffer;
	}

	for (i = 0; i < g_io_group_num; i++) {
		ret = memset_s(io_buffer, BUFFER_LENGTH * sizeof(char), 0, BUFFER_LENGTH * sizeof(char));
		if (ret != EOK) {
			pr_err("%s: memset_s io_buffer err!\n", __func__);
			goto err_free_io;
		}
		ret = snprintf_s(io_buffer, BUFFER_LENGTH * sizeof(char),
				BUFFER_LENGTH * sizeof(char) - 1, "arm,primecell%d", i);
		if (ret < 0)
			pr_err("%s: snprintf_s io_buffer err.\n", __func__);
		np = of_find_compatible_node(NULL, NULL, io_buffer);
		if (np == NULL) {
			pr_err("%s[%d]: gpio%d not find.\n", __func__, __LINE__, i);
			ret = -ENODEV;
			goto err_free_io;
		}
		sysreg_base.gpio_base[i] = of_iomap(np, 0);
		pr_debug("%s: sysreg_base.gpio_base[%d] %pK\n",
				__func__, i, sysreg_base.gpio_base[i]);
		if (sysreg_base.gpio_base[i] == NULL) {
			pr_err("%s: gpio%d iomap err.\n", __func__, i);
			ret = -ENOMEM;
			goto err_put_node;
		}
		of_node_put(np);
	}

	kfree(io_buffer);
	return 0;

err_put_node:
	of_node_put(np);
err_free_io:
	kfree(sysreg_base.gpio_base);
	sysreg_base.gpio_base = NULL;
err_free_buffer:
	kfree(io_buffer);
err:
	pr_err("%s: failed!\n", __func__);
	return ret;
}

static int get_ipc_addr(void)
{
	int ret;
	struct device_node *np = NULL;

	/* FIXME */
	np = of_find_compatible_node(NULL, NULL, "lowpm,ipc");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "lowpm,ipc");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.nsipc_base = of_iomap(np, 0);
	pr_info("%s: nsipc_base:%pK\n", __func__, sysreg_base.nsipc_base);
	if (sysreg_base.nsipc_base == NULL) {
		pr_err("%s: nsipc_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

#ifdef AP_AO_NSIPC_IRQ
	np = of_find_compatible_node(NULL, NULL, "lowpm,ao_ipc");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "lowpm,ao_ipc");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.ao_nsipc_base = of_iomap(np, 0);
	pr_info("%s: ao_nsipc_base:%pK\n", __func__, sysreg_base.ao_nsipc_base);
	if (sysreg_base.ao_nsipc_base == NULL) {
		pr_err("%s: ao_nsipc_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);
#endif

	return 0;

err_put_node:
	of_node_put(np);
err:
	pr_err("%s[%d]: fail!\n", __func__, __LINE__);
	return ret;
}


/* map system registers */
static int map_sysregs(void)
{
	int ret;
	unsigned int uart_idx = 0;
	struct device_node *np = NULL;

	uart_idx = get_console_index();
	switch (uart_idx) {
	case 0:
		np = of_find_compatible_node(NULL, NULL, "arm,pl011");
		if (np == NULL) {
			pr_err("%s: dts[%s] node not found\n",
					__func__, "arm,pl011");
			ret = -ENODEV;
			goto err;
		}
		break;
	case 6:
		/* FIXME */
		np = of_find_compatible_node(NULL, NULL,
						"hisilicon,lowpm_func");
		if (np == NULL) {
			pr_err("%s: dts[%s] node not found\n",
					__func__, "hisilicon,lowpm_func");
			ret = -ENODEV;
			goto err;
		}
		break;
	default:
		pr_err("%s:uart default:%d\n", __func__, uart_idx);
		ret = -ENODEV;
		goto err;
	}

	sysreg_base.uart_base = of_iomap(np, 0);
	pr_debug("%s: uart_base:%pK\n", __func__, sysreg_base.uart_base);
	if (sysreg_base.uart_base == NULL) {
		pr_err("%s: uart_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "hisilicon,sysctrl");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.sysctrl_base = of_iomap(np, 0);
	pr_debug("%s: sysctrl_base:%pK\n", __func__, sysreg_base.sysctrl_base);
	if (sysreg_base.sysctrl_base == NULL) {
		pr_err("%s: sysctrl_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pctrl");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "hisilicon,pctrl");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.pctrl_base = of_iomap(np, 0);
	pr_debug("%s: pctrl_base:%pK\n", __func__, sysreg_base.pctrl_base);
	if (sysreg_base.pctrl_base == NULL) {
		pr_err("%s: pctrl_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "hisilicon,pmctrl");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.pmctrl_base = of_iomap(np, 0);
	pr_debug("%s: pmctrl_base:%pK\n", __func__, sysreg_base.pmctrl_base);
	if (sysreg_base.pmctrl_base == NULL) {
		pr_err("%s: pmctrl_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "hisilicon,crgctrl");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.crg_base = of_iomap(np, 0);
	pr_debug("%s: crg_base:%pK\n", __func__, sysreg_base.crg_base);
	if (sysreg_base.crg_base == NULL) {
		pr_err("%s: crg_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmu");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n",
				__func__, "hisilicon,pmu");
		ret = -ENODEV;
		goto err;
	}
	sysreg_base.pmic_base = of_iomap(np, 0);
	pr_debug("%s: pmic_base:%pK\n", __func__, sysreg_base.pmic_base);
	if (sysreg_base.pmic_base == NULL) {
		pr_err("%s: pmic_base of iomap fail!\n", __func__);
		ret = -ENOMEM;
		goto err_put_node;
	}
	of_node_put(np);

	ret = get_ipc_addr();
	if (ret != 0) {
		pr_err("%s: %d get_ipc_addr failed.\n", __func__, __LINE__);
		goto err;
	}
	return 0;

err_put_node:
	of_node_put(np);
err:
	pr_err("%s[%d]: fail!\n", __func__, __LINE__);
	return ret;
}

/****************************************
 * function: debuguart_reinit
 * description:
 * reinit debug uart.
 *****************************************/
void debuguart_reinit(void)
{
	unsigned int uart_idx, uregv;

	if ((console_suspend_enabled == 1) ||
	     (sysreg_base.uart_base == NULL) ||
	     (sysreg_base.crg_base == NULL))
		return;

	uart_idx = get_console_index();

	if (uart_idx == 6) {
		/* enable clk */
		uregv = (unsigned int)readl(SOC_SCTRL_SCLPMCUCLKEN_ADDR(sysreg_base.sysctrl_base))
				| BIT(SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_START);
		writel(uregv,
			SOC_SCTRL_SCLPMCUCLKEN_ADDR(sysreg_base.sysctrl_base));

		/* reset undo */
		writel(BIT(SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_START),
			SOC_SCTRL_SCLPMCURSTDIS_ADDR(sysreg_base.sysctrl_base));
	} else if (uart_idx == 0) {
		/* enable clk */
		writel(BIT(SOC_CRGPERIPH_PEREN2_gt_clk_uart0_START),
				SOC_CRGPERIPH_PEREN2_ADDR(sysreg_base.crg_base));

		/* reset undo */
		writel(BIT(CRG_PERRSTDIS2_ip_prst_uart0_BIT),
				SOC_CRGPERIPH_PERRSTDIS2_ADDR(sysreg_base.crg_base));
	} else {
		return;
	}

	/* disable uart */
	writel(0x0, SOC_UART_UARTCR_ADDR(sysreg_base.uart_base));

	/* set baudrate: 19.2M 115200 */
	writel(0xa, SOC_UART_UARTIBRD_ADDR(sysreg_base.uart_base));
	writel(0x1b, SOC_UART_UARTFBRD_ADDR(sysreg_base.uart_base));

	/* disable FIFO */
	writel(0x0, SOC_UART_UARTLCR_H_ADDR(sysreg_base.uart_base));

	/* set FIFO depth: 1/2 */
	writel(0x92, SOC_UART_UARTIFLS_ADDR(sysreg_base.uart_base));

	/* set int mask */
	uregv = BIT(SOC_UART_UARTIMSC_rtim_START)
		| BIT(SOC_UART_UARTIMSC_rxim_START);
	writel(uregv, SOC_UART_UARTIMSC_ADDR(sysreg_base.uart_base));

	/* enable FIFO */
	writel(0x70, SOC_UART_UARTLCR_H_ADDR(sysreg_base.uart_base));

	/* enable uart trans */
	uregv = BIT(SOC_UART_UARTCR_uarten_START)
			| BIT(SOC_UART_UARTCR_rxe_START)
			| BIT(SOC_UART_UARTCR_txe_START);
	writel(uregv, SOC_UART_UARTCR_ADDR(sysreg_base.uart_base));
}


unsigned int proc_mask_to_id(unsigned int mask)
{
	unsigned int i;

	for (i = 0; i < IPC_PROCESSOR_MAX; i++) {
		if (mask & BIT(i))
			break;
	}

	return i;
}

void ipc_mbx_irq_show(struct seq_file *s, const void __iomem *base, unsigned int mbx)
{
	unsigned int ipc_source, ipc_dest, ipc_data, src_id, dest_id;
	unsigned int i;
#ifdef CONFIG_HUAWEI_DUBAI
	unsigned int source = 0;
	unsigned int mem = 0;
#endif

	ipc_source = readl(base + ipc_mbx_source_offset(mbx));
	src_id = proc_mask_to_id(ipc_source);
	if (src_id < AP_IPC_PROC_MAX) {
		/* is ack irq */
		ipc_dest = readl(base + ipc_mbx_dstatus_offset(mbx));
		dest_id = proc_mask_to_id(ipc_dest);
		if (dest_id < IPC_PROCESSOR_MAX)
			LOWPM_MSG(s, "SR:mailbox%u: ack from %s", mbx, processor_name[dest_id]);
		else
			LOWPM_MSG(s, "SR:mailbox%u: ack from unknown(dest_id = %u)", mbx, dest_id);
	} else if (src_id < IPC_PROCESSOR_MAX) {
		/* is receive irq */
		LOWPM_MSG(s, "SR:mailbox%u: send by %s", mbx, processor_name[src_id]);
	} else {
		LOWPM_MSG(s, "SR:mailbox%u: src id unknown(src_id = %u)", mbx, src_id);
	}

	for (i = 0; i < IPC_MBXDATA_MAX; i++) {
		ipc_data = readl(base + ipc_mbxdata_offset(mbx, i));
#ifdef CONFIG_HUAWEI_DUBAI
		if ((src_id < IPC_PROCESSOR_MAX) &&
			(strncmp(processor_name[src_id], "iomcu", PROCESSOR_IOMCU_LENGTH) == 0)) {
			if (i == IPC_MBXDATA_MIN)
				mem = get_sharemem_data(ipc_data);
			else if (i == IPC_MBXDATA_TAG) {
				source = get_sharemem_source(ipc_data);
				dubai_update_wakeup_info("DUBAI_TAG_SENSORHUB_WAKEUP", "mem=%u source=%u", mem, source);
			}
		}
#endif
		LOWPM_MSG(s, "SR:[MBXDATA%u]:0x%x", i, ipc_data); //lint !e666
	}
}

void combo_ipc_irq_show(struct seq_file *s, unsigned int nsipc_state)
{
	unsigned int mbx;

	/* ns ipc irq detail show */
	LOWPM_MSG(s, "SR:nsipc irq state:0x%x", nsipc_state);
	if (sysreg_base.nsipc_base == NULL) {
		LOWPM_MSG(s, "SR:nsipc base is NULL");
		return;
	}
	for (mbx = 0; mbx <= NSIPC_MAILBOX_MAX; mbx++) {
		if ((nsipc_state & BIT(mbx)) > 0)
			ipc_mbx_irq_show(s, sysreg_base.nsipc_base, mbx);
	}

}

#ifdef AP_AO_NSIPC_IRQ
/* ao ipc */
char *aoipc_processor_name[AO_IPC_PROCESSOR_MAX] = {
	"iomcu",
	"gic",
	"isp"
};

unsigned int ao_proc_mask_to_id(unsigned int mask)
{
	unsigned int i;

	for (i = 0; i < AO_IPC_PROCESSOR_MAX; i++) {
		if (mask & BIT(i))
			break;
	}

	return i;
}

void ao_ipc_mbx_irq_show(struct seq_file *s, const void __iomem *base, unsigned int mbx)
{
	unsigned int ipc_source, ipc_dest, ipc_data, src_id, dest_id;
	unsigned int i;
#ifdef CONFIG_HUAWEI_DUBAI
	unsigned int source = 0;
	unsigned int mem = 0;
#endif

	ipc_source = readl(base + ipc_mbx_source_offset(mbx));
	src_id = ao_proc_mask_to_id(ipc_source);
	if (src_id == AP_IPC_PROC_BIT) {
		/* is ack irq */
		ipc_dest = readl(base + ipc_mbx_dstatus_offset(mbx));
		dest_id = ao_proc_mask_to_id(ipc_dest);
		if (dest_id < AO_IPC_PROCESSOR_MAX)
			LOWPM_MSG(s, "SR:mailbox%u: ack from %s", mbx, aoipc_processor_name[dest_id]);
		else
			LOWPM_MSG(s, "SR:mailbox%u: ack from unknown(dest_id = %u)", mbx, dest_id);
	} else if (src_id < AO_IPC_PROCESSOR_MAX) {
		/* is receive irq */
		LOWPM_MSG(s, "SR:mailbox%u: send by %s", mbx, aoipc_processor_name[src_id]);
	} else {
		LOWPM_MSG(s, "SR:mailbox%u: src id unknown(src_id = %u)", mbx, src_id);
	}

	for (i = 0; i < IPC_MBXDATA_MAX; i++) {
		ipc_data = readl(base + ipc_mbxdata_offset(mbx, i));
#ifdef CONFIG_HUAWEI_DUBAI
		if ((src_id < AO_IPC_PROCESSOR_MAX) &&
			(strncmp(aoipc_processor_name[src_id], "iomcu", PROCESSOR_IOMCU_LENGTH) == 0)) {
			if (i == IPC_MBXDATA_MIN) {
				mem = get_sharemem_data(ipc_data);
			} else if (i == IPC_MBXDATA_TAG) {
				source = get_sharemem_source(ipc_data);
				dubai_update_wakeup_info("DUBAI_TAG_SENSORHUB_WAKEUP", "mem=%u source=%u", mem, source);
			}
		}
#endif
		LOWPM_MSG(s, "SR:[MBXDATA%u]:0x%x", i, ipc_data); //lint !e666
	}
}

void combo_ao_ipc_irq_show(struct seq_file *s, unsigned int ao_nsipc_state)
{
	unsigned int mbx;

	/* ns ipc irq detail show */
	LOWPM_MSG(s, "SR:ao nsipc irq state:0x%x", ao_nsipc_state);
	if (sysreg_base.ao_nsipc_base == NULL) {
		LOWPM_MSG(s, "SR:ao nsipc base is NULL");
		return;
	}
	for (mbx = 0; mbx <= AO_NSIPC_MAILBOX_MAX; mbx++)
		if ((ao_nsipc_state & BIT(mbx)) > 0)
			ao_ipc_mbx_irq_show(s, sysreg_base.ao_nsipc_base, mbx);
}
#endif

/****************************************
 * function: pm_status_show
 * description:
 * show pm status.
 *****************************************/
void pm_status_show(struct seq_file *s)
{
#ifdef CONFIG_HISI_LPMCU_BB
	unsigned int wake_status, wake_irq, wake_irq1, ap_irq, bit_shift;
	unsigned int i;

	if (sysreg_base.reserved_base == NULL) {
		if (M3_RDR_SYS_CONTEXT_BASE_ADDR) {
			sysreg_base.reserved_base = M3_RDR_SYS_CONTEXT_RUNTIME_VAR_ADDR;
		} else{
			pr_err("%s: M3_RDR_SYS_CONTEXT_BASE_ADDR is NULL\n", __func__);
			return;
		}
	}

	wake_status = readl(sysreg_base.reserved_base + WAKE_STATUS_OFFSET); //lint !e578
	LOWPM_MSG(s,
		"SR:wakelock status[0x%x], ap:%d, modem:%d, hifi:%d, iomcu:%d, hisee:%d, hotplug:%d,%d.",
		wake_status,
		(wake_status & AP_MASK) ? 1 : 0,
		(wake_status & MODEM_MASK) ? 1 : 0,
		(wake_status & HIFI_MASK) ? 1 : 0,
		(wake_status & IOMCU_MASK) ? 1 : 0,
		(wake_status & HISEE_MASK) ? 1 : 0,
		(wake_status & HOTPLUG_MASK(0)) ? 1 : 0,
		(wake_status & HOTPLUG_MASK(1)) ? 1 : 0);
#ifdef CONFIG_HUAWEI_DUBAI
	dubai_update_wakeup_info("DUBAI_TAG_WAKE_STATUS",
		"ap=%d modem=%d hifi=%d iomcu=%d hisee=%d hotplug0=%d hotplug1=%d",
		(wake_status & AP_MASK) ? 1 : 0,
		(wake_status & MODEM_MASK) ? 1 : 0,
		(wake_status & HIFI_MASK) ? 1 : 0,
		(wake_status & IOMCU_MASK) ? 1 : 0,
		(wake_status & HISEE_MASK) ? 1 : 0,
		(wake_status & HOTPLUG_MASK(0)) ? 1 : 0,
		(wake_status & HOTPLUG_MASK(1)) ? 1 : 0);
#endif

	LOWPM_MSG(s, "SR:system sleeped %u times.",
		readl(sysreg_base.reserved_base + SYS_DSLEEP_CNT_OFFSET)); //lint !e666
	LOWPM_MSG(s,
		"SR:wake times, system:%u, woken up by ap:%u, modem:%u, hifi:%u, iomcu:%u, lpm3:%u.",
		readl(sysreg_base.reserved_base + SYS_DWAKE_CNT_OFFSET),
		readl(sysreg_base.reserved_base + AP_WAKE_CNT_OFFSET),
		readl(sysreg_base.reserved_base + MODEM_WAKE_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HIFI_WAKE_CNT_OFFSET),
		readl(sysreg_base.reserved_base + IOMCU_WAKE_CNT_OFFSET),
		readl(sysreg_base.reserved_base + LPM3_WAKE_CNT_OFFSET)); //lint !e666

#if defined(HISEE_SUSPEND_CNT_OFFSET) && defined(HISEE_RESUME_CNT_OFFSET)
	LOWPM_MSG(s,
		"SR:sr times of sub system, ap:s-%u, r-%u, modem:s-%u, r-%u, hifi:s-%u, r-%u, iomcu:s-%u, r-%u, hisee:s-%u, r-%u.",
		readl(sysreg_base.reserved_base + AP_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + AP_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + MODEM_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + MODEM_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HIFI_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HIFI_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + IOMCU_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + IOMCU_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HISEE_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HISEE_RESUME_CNT_OFFSET)); //lint !e666
#else
	LOWPM_MSG(s,
		"SR:sr times of sub system, ap:s-%u, r-%u, modem:s-%u, r-%u, hifi:s-%u, r-%u, iomcu:s-%u, r-%u.",
		readl(sysreg_base.reserved_base + AP_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + AP_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + MODEM_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + MODEM_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HIFI_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + HIFI_RESUME_CNT_OFFSET),
		readl(sysreg_base.reserved_base + IOMCU_SUSPEND_CNT_OFFSET),
		readl(sysreg_base.reserved_base + IOMCU_RESUME_CNT_OFFSET)); //lint !e666
#endif


	if (sysreg_base.sysctrl_base != NULL) {
		LOWPM_MSG(s, "SR:SCINT_STAT:0x%x.",
				readl(sysreg_base.sysctrl_base + SOC_SCTRL_SCINT_STAT_ADDR(0))); //lint !e666
		LOWPM_MSG(s, "SR:SCINT_STAT1:0x%x.",
				readl(sysreg_base.sysctrl_base + SOC_SCTRL_SCINT_STAT1_ADDR(0))); //lint !e666
	}

	wake_irq = readl(sysreg_base.reserved_base + WAKE_IRQ_OFFSET);
	wake_irq1 = readl(sysreg_base.reserved_base + WAKE_IRQ1_OFFSET);
	LOWPM_MSG(s, "SR:WAKE_IRQ:0x%x, WAKE_IRQ1:0x%x", wake_irq, wake_irq1);
	for (i = 0; i < WAKE_IRQ_NUM_MAX; i++) {
		bit_shift = (1 << i);
		if (((wake_irq & AP_WAKE_INT_MASK) | (wake_irq1 & AP_WAKE_INT_MASK1)) & bit_shift)
			LOWPM_MSG(s, "LAST DEEPSLEEP WAKE BIT: %d(AP)", i);

		if ((wake_irq & IOMCU_WAKE_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST DEEPSLEEP WAKE BIT: %d(IOMCU)", i);

		if ((wake_irq & MODEM_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST DEEPSLEEP WAKE BIT: %d(MODEM)", i);

		if ((wake_irq1 & MODEM_DRX_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST DEEPSLEEP WAKE DRX_BIT: %d(MODEM)", i);

		if ((wake_irq & HIFI_WAKE_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST DEEPSLEEP WAKE BIT: %d(HIFI)", i);
	}

	wake_irq = readl(sysreg_base.reserved_base + SLEEP_WAKE_IRQ_OFFSET);
	wake_irq1 = readl(sysreg_base.reserved_base + SLEEP_WAKE_IRQ1_OFFSET);
	for (i = 0; i < WAKE_IRQ_NUM_MAX; i++) {
		bit_shift = (1 << i);
		if (((wake_irq & AP_WAKE_INT_MASK) | (wake_irq1 & AP_WAKE_INT_MASK1)) & bit_shift)
			LOWPM_MSG(s, "LAST SLEEP WAKE BIT: %d(AP)", i);

		if ((wake_irq & IOMCU_WAKE_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST SLEEP WAKE BIT: %d(IOMCU)", i);

		if ((wake_irq & MODEM_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST SLEEP WAKE BIT: %d(MODEM)", i);

		if ((wake_irq1 & MODEM_DRX_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST SLEEP WAKE DRX_BIT: %d(MODEM)", i);

		if ((wake_irq & HIFI_WAKE_INT_MASK) & bit_shift)
			LOWPM_MSG(s, "LAST SLEEP WAKE BIT: %d(HIFI)", i);
	}

	ap_irq = readl(sysreg_base.reserved_base + AP_WAKE_IRQ_OFFSET);
	if ((g_lpmcu_irq_num > ap_irq) && (g_lpmcu_irq_name != NULL)) {
#ifdef CONFIG_HUAWEI_DUBAI
		dubai_update_wakeup_info("DUBAI_TAG_AP_WAKE_IRQ", "name=%s", g_lpmcu_irq_name[ap_irq]);
#endif
		LOWPM_MSG(s, "SR:AP WAKE IRQ(LPM3 NVIC): %d (%s)",
				ap_irq, g_lpmcu_irq_name[ap_irq]);
	} else {
		LOWPM_MSG(s, "SR:AP WAKE IRQ(LPM3 NVIC): %d (no exist)", ap_irq);
	}


	if (ap_irq == IRQ_COMB_GIC_IPC) {
		combo_ipc_irq_show(s, readl(sysreg_base.reserved_base + AP_NSIPC_IRQ_OFFSET));
#ifdef AP_AO_NSIPC_IRQ
		combo_ao_ipc_irq_show(s, readl(sysreg_base.reserved_base + AP_AO_NSIPC_IRQ_OFFSET));
#endif
	}

	LOWPM_MSG(s, "light sleep 0x%x times,resume 0x%x times.",
		readl(sysreg_base.reserved_base + LIGHT_SLEEP_CNT_OFFSET),
		readl(sysreg_base.reserved_base + LIGHT_SLEEP_RESUME_CNT_OFFSET)); //lint !e666

	LOWPM_MSG(s, "sleep 0x%x times.",
		readl(sysreg_base.reserved_base + SYS_SLEEP_CNT_OFFSET)); //lint !e666



#ifdef LPMCU_TICKMARK_FEATURE
	LOWPM_MSG(s, "lpm suspend time[%dus]: 0x%x, 0x%x.",
		(readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_DEEPSLEEP_WFI_ENTER * 4)
		- readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_SYS_SUSPEND_ENTRY * 4)) * 31,
		readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_SYS_SUSPEND_ENTRY * 4),
		readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_DEEPSLEEP_WFI_ENTER * 4)); //lint !e666

	LOWPM_MSG(s, "lpm resume time[%dus]: 0x%x, 0x%x.",
		(readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_SYS_WAKEUP_END * 4)
		- readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_SYS_WAKEUP * 4)) * 31,
		readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_SYS_WAKEUP * 4),
		readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + TICK_SYS_WAKEUP_END * 4)); //lint !e666

	LOWPM_MSG(s, "lpm sr time: ");
	for (i = 0; i < TICK_MARK_END_FLAG; i++) {
		LOWPM_MSG_CONT(s, "[%u]0x%x ", i, readl(M3_RDR_SYS_CONTEXT_LPMCU_STAT_ADDR + i * 4)); //lint !e666
		if (i % 10 == 0)
			LOWPM_MSG_CONT(s, "\n");
	}
	LOWPM_MSG_CONT(s, "\n");
#endif
#endif /* CONFIG_HISI_LPMCU_BB */
}

void set_wakelock(int iflag)
{
	if ((iflag == 1) && (lowpm_wake_lock.active == 0))
		__pm_stay_awake(&lowpm_wake_lock);
	else if ((iflag == 0) && (lowpm_wake_lock.active != 0))
		__pm_relax(&lowpm_wake_lock); //lint !e455
} //lint !e454 !e456

#define GPIO_DIR(x)		((x) + 0x400)
#define GPIO_DATA(x, y)		((x) + (1 << (2 + y)))
#define GPIO_BIT(x, y)		((x) << (y))
#define GPIO_IS_SET(x, y)	(((x) >> (y)) & 0x1)

static int show_iomg_sec(struct seq_file *s, struct iocfg_lp *iocfg)
{
#ifdef CONFIG_PM_SEC_IO
	void __iomem *addr = NULL;
	int j;

	if ((g_mg_sec_reg_num > 0) && (g_mg_reg != NULL)) {
		addr = (void __iomem *)(uintptr_t)(iocfg->iomg_base + iocfg->iomg_offset);
		for (j = 1; j < g_mg_sec_reg_num; j += 2) {
			if ((addr >= g_mg_reg[j - 1]) && (addr <= g_mg_reg[j])) {
				LOWPM_MSG_CONT(s, "sec iomg");
				return 1;
			}
		}
	}
	return 0;
#else
	return 0;
#endif
}
static void show_iomg_reg(struct seq_file *s, struct iocfg_lp *iocfg)
{
	void __iomem *addr = NULL;
	int value;

	LOWPM_MSG_CONT(s, "iomg:");

	if (iocfg->iomg_offset == -1) {
		LOWPM_MSG_CONT(s, "null");
		goto end;
	}

	if (show_iomg_sec(s, iocfg) == 1)
		goto end;

	addr = ioremap(iocfg->iomg_base + iocfg->iomg_offset, 0x4);
	if (addr == NULL) {
		LOWPM_MSG_CONT(s, "ioremap fail");
		goto end;
	}
	value = readl(addr);
	iounmap(addr);

	LOWPM_MSG_CONT(s, "Func-%d", value);
	if (value == FUNC1)
		LOWPM_MSG_CONT(s, "(%s)", iocfg->gpio_func1);
	else if (value == FUNC2)
		LOWPM_MSG_CONT(s, "(%s)", iocfg->gpio_func2);
	else if (value == FUNC3)
		LOWPM_MSG_CONT(s, "(%s)", iocfg->gpio_func3);

	LOWPM_MSG_CONT(s, "\t");

	/* check conflict */
	if (value != iocfg->iomg_val) {
		LOWPM_MSG_CONT(s, " -E [Func-%d]", iocfg->iomg_val);
		if (iocfg->iomg_val == FUNC1)
			LOWPM_MSG_CONT(s, "(%s)", iocfg->gpio_func1);
		if (iocfg->iomg_val == FUNC2)
			LOWPM_MSG_CONT(s, "(%s)", iocfg->gpio_func2);
		if (iocfg->iomg_val == FUNC3)
			LOWPM_MSG_CONT(s, "(%s)", iocfg->gpio_func3);
	}
end:
	LOWPM_MSG_CONT(s, "\t");
}

static int show_iocg_sec(struct seq_file *s, struct iocfg_lp *iocfg)
{
#ifdef CONFIG_PM_SEC_IO
	void __iomem *addr = NULL;
	int j;

	if ((g_cg_sec_reg_num > 0) && (g_cg_reg != NULL)) {
		addr = (void __iomem *)(uintptr_t)(iocfg->iocg_base + iocfg->iocg_offset);
		for (j = 1; j < g_cg_sec_reg_num; j += 2) {
			if ((addr >= g_cg_reg[j - 1]) && (addr <= g_cg_reg[j])) {
				LOWPM_MSG_CONT(s, "sec iocg");
				return 1;
			}
		}
	}
	return 0;
#else
	return 0;
#endif
}

static void show_iocg_reg(struct seq_file *s, struct iocfg_lp *iocfg)
{
	void __iomem *addr = NULL;
	int value;

	LOWPM_MSG_CONT(s, "iocg:");

	if (show_iocg_sec(s, iocfg) == 1)
		goto end;

	addr = ioremap(iocfg->iocg_base + iocfg->iocg_offset, 0x4);
	if (addr == NULL) {
		LOWPM_MSG_CONT(s, "ioremap fail");
		goto end;
	}
	value = readl(addr);
	iounmap(addr);

	LOWPM_MSG_CONT(s, "[0x%x]iocg = %s", value, pulltype[((unsigned int)value) & 0x3]);

	if ((value != iocfg->iocg_val) && (iocfg->iocg_val != -1))
		LOWPM_MSG_CONT(s, " -E [%s]", pulltype[iocfg->iocg_val]);

end:
	LOWPM_MSG_CONT(s, "\t");
}

static int show_gpio_sec(struct seq_file *s, struct iocfg_lp *iocfg)
{
	return 0;
}

static void show_gpio_info(struct seq_file *s, struct iocfg_lp *iocfg)
{
	void __iomem *addr = NULL;
	void __iomem **addr1 = NULL;
	int value, data;

	LOWPM_MSG_CONT(s, "gpio:");

	if (show_gpio_sec(s, iocfg) == 1)
		return;

	if (iocfg->iomg_val == FUNC0) {
		addr = GPIO_DIR(sysreg_base.gpio_base[iocfg->gpio_group_id]);
		addr1 = GPIO_DATA(sysreg_base.gpio_base[iocfg->gpio_group_id],
				iocfg->ugpio_bit); //lint !e679

		value = GPIO_IS_SET((u32)readl(addr), iocfg->ugpio_bit);
		data = GPIO_IS_SET((u32)readl(addr1), iocfg->ugpio_bit);
		LOWPM_MSG_CONT(s, "[0x%x 0x%x]gpio-%s", value, data, value ? "O" : "I");

		if (value != 0)
			LOWPM_MSG_CONT(s, "%s", data ? "H" : "L");

		if (value != iocfg->gpio_dir) {
			LOWPM_MSG_CONT(s, "\t-E [%s", iocfg->gpio_dir ? "O" : "I");
			if (iocfg->gpio_dir && (data != iocfg->gpio_val))
				LOWPM_MSG_CONT(s, "%s", iocfg->gpio_val ? "H" : "L");
			LOWPM_MSG_CONT(s, "]");
		}
	}
}

static struct iocfg_lp *dbg_get_iocfg_by_name(struct seq_file *s)
{
	struct iocfg_lp *hisi_iocfg_lookups = NULL;
	unsigned int i;

	if (g_lp_fpga_flag == 0) {
		for (i = 0; i < ARRAY_SIZE(iocfg_tables); i++) {
			if (strnstr(g_plat_name, "es", strlen(g_plat_name)) &&
				strnstr(iocfg_tables[i].boardname, "chip_es", sizeof(iocfg_tables[i].boardname))) {
				hisi_iocfg_lookups = iocfg_tables[i].iocfg;
				LOWPM_MSG(s, "chip es board");
				break;
			}
			if (strnstr(g_plat_name, "cs2", strlen(g_plat_name)) &&
				strnstr(iocfg_tables[i].boardname, "chip_cs2", sizeof(iocfg_tables[i].boardname))) {
				hisi_iocfg_lookups = iocfg_tables[i].iocfg;
				LOWPM_MSG(s, "chip cs2 board");
				break;
			}
			if (strnstr(g_plat_name, "cs", strlen(g_plat_name)) &&
				strnstr(iocfg_tables[i].boardname, "chip_cs", sizeof(iocfg_tables[i].boardname)) &&
				!strnstr(iocfg_tables[i].boardname, "chip_cs2", sizeof(iocfg_tables[i].boardname))) {
				hisi_iocfg_lookups = iocfg_tables[i].iocfg;
				LOWPM_MSG(s, "chip cs board");
				break;
			}
			if (strnstr(iocfg_tables[i].boardname, "chip", sizeof(iocfg_tables[i].boardname))) {
				hisi_iocfg_lookups = iocfg_tables[i].iocfg;
				LOWPM_MSG(s, "chip board");
				break;
			}

		}
	} else {
		for (i = 0; i < ARRAY_SIZE(iocfg_tables); i++) {
			if (strnstr(iocfg_tables[i].boardname, "fpga", sizeof(iocfg_tables[i].boardname))) {
				hisi_iocfg_lookups = iocfg_tables[i].iocfg;
				LOWPM_MSG(s, "fpga board");
				break;
			}
		}
	}

	return hisi_iocfg_lookups;
}

static void dbg_disp_iocfg_tables(struct seq_file *s)
{
	int i, j;

	LOWPM_MSG(s, "iocfg table length is %d: ", ARRAY_SIZE(iocfg_tables));
	for (i = 0; i < ARRAY_SIZE(iocfg_tables); i++) {
		LOWPM_MSG_CONT(s, "chipid:< ");
		for (j = 0; j < HISI_CHIPID_DIG_NUMS; j++)
			LOWPM_MSG_CONT(s, "%X ", iocfg_tables[i].chipid[j]);
		LOWPM_MSG_CONT(s, ">; boardname:%s\n", iocfg_tables[i].boardname);
	}
}

static struct iocfg_lp *dbg_get_iocfg_by_chipid(struct seq_file *s)
{
	struct iocfg_lp *hisi_iocfg_lookups = NULL;
	int *chipid = NULL;
	unsigned int i;

	dbg_disp_iocfg_tables(s);
	/* get iocfg table */
	for (i = 0; i < ARRAY_SIZE(iocfg_tables); i++) {
		chipid = iocfg_tables[i].chipid;
		if (compare_chipid(chipid) == 0) {
			hisi_iocfg_lookups = iocfg_tables[i].iocfg;
			LOWPM_MSG(s, "find by chipid, board name is %s.\n",
					iocfg_tables[i].boardname);
			break;
		}
	}

	return hisi_iocfg_lookups;
}

/* check all io status */
void io_status_show(struct seq_file *s)
{
	unsigned int i, gpio_id, len;
	struct iocfg_lp *hisi_iocfg_lookups = NULL;
	struct iocfg_lp *iocfg = NULL;

	if ((g_usavedcfg & DEBG_SUSPEND_IO_SHOW) != DEBG_SUSPEND_IO_SHOW)
		return;

	hisi_iocfg_lookups = dbg_get_iocfg_by_chipid(s);
	if (hisi_iocfg_lookups == NULL)
		hisi_iocfg_lookups = dbg_get_iocfg_by_name(s);

	if (hisi_iocfg_lookups == NULL) {
		LOWPM_MSG(s, "canot find iocfg table.");
		return;
	}

	/* calc hisi_iocfg_lookups[] length */
	len = 0;
	for (i = 0; hisi_iocfg_lookups[i].iomg_offset != CFG_END; i++)
		len = i;

	LOWPM_MSG(s, "io list length is %u :", len);

	for (i = 0; i < len; i++) {
		iocfg = &hisi_iocfg_lookups[i];
		gpio_id = ((iocfg->gpio_group_id << 3) + iocfg->ugpio_bit);

		LOWPM_MSG_CONT(s, "gpio_id:%d\tgpio_logic:%s\t", gpio_id, iocfg->gpio_logic);

		/* show iomg register's value */
		show_iomg_reg(s, iocfg);
		/* show iocg register */
		show_iocg_reg(s, iocfg);
		/* if this is gpio pin */
		show_gpio_info(s, iocfg);

		LOWPM_MSG_CONT(s, "\n");
	}
}


/*****************************************************************
 * function: pmu_status_show
 * description:
 * show the pmu status.
 ******************************************************************/
void pmu_status_show(struct seq_file *s)
{
	unsigned int i, uregv;
	unsigned int k = 0;
	struct pmu_idx *pmuidx = NULL;
	struct pmuregs *pmureg = NULL;

	if ((g_usavedcfg & DEBG_SUSPEND_PMU_SHOW) != DEBG_SUSPEND_PMU_SHOW)
		return;

	LOWPM_MSG(s, "PMU_CTRL register list length is %d:", g_pmu_addr_end);

	for (i = 0x0; i <= g_pmu_addr_end; i++) {
#ifdef	CONFIG_HISI_PMIC_PMU_SPMI
		uregv = hisi_pmic_reg_read(i);
#else
		uregv = readl(PMUSSI_REG(i));
#endif
		LOWPM_MSG_CONT(s, "PMU_CTRL 0x%02X=0x%02X\t", i, uregv);
		/* NOTE!! g_pmuregs_lookups[k] must be matched with g_hisi_pmu_idx[k],
		 * otherwise the array may be out of range.
		 */
		if ((k < g_lp_pmu_num) && (g_hisi_pmu_idx != NULL) && (g_pmuregs_lookups != NULL)) {
			pmuidx = g_hisi_pmu_idx[k];
			pmureg = g_pmuregs_lookups[k];
			if (pmuidx == NULL || pmureg == NULL || pmureg->ucoffset != i) {
				LOWPM_MSG_CONT(s, "\n");
				continue;
			}
			LOWPM_MSG_CONT(s, "[%s]", pmuidx->name);
			if ((uregv & pmureg->cmask) == pmureg->cval)
				LOWPM_MSG_CONT(s, "enabled  %s\t", (pmuidx->status & AP_DIS) ? "-E" : "");
			LOWPM_MSG_CONT(s, "[%s]owner:%s", pmuidx->status_name, pmuidx->module);
			k++;
		}
		LOWPM_MSG_CONT(s, "\n");
	}
}


#define NULL_CLOSE	0
#define AP_CLOSE	1
#define LPM3_CLOSE	2
#define AP_ON		3
#define AO_ON		4
#define ON_OFF		5
#define MODEM_ON	6

#define LP_CLK(_base, _offset, _bit, _clk_name, _status) \
{                               \
	.reg_base = _base, \
	.reg_off = _offset,  \
	.bit_idx    = _bit, \
	.clk_name = _clk_name,  \
	.status = _status,	\
}

static void clk_showone(struct seq_file *s, struct lp_clk *clk)
{
	u32 regval, bitval, regoff, bit_idx;
	void __iomem *regbase = NULL;

	if (clk == NULL) {
		LOWPM_MSG_CONT(s, "null ptr clk");
		return;
	}

	switch (clk->reg_base) {
	case SOC_ACPU_SCTRL_BASE_ADDR:
		LOWPM_MSG_CONT(s, "[%s]", "SYSCTRL");
		regbase = sysreg_base.sysctrl_base;
		break;
	case SOC_ACPU_PERI_CRG_BASE_ADDR:
		LOWPM_MSG_CONT(s, "[%s]", "CRGPERI");
		regbase = sysreg_base.crg_base;
		break;
	case SOC_ACPU_PMC_BASE_ADDR:
		LOWPM_MSG_CONT(s, "[%s]", "PMCCTRL");
		regbase = sysreg_base.pctrl_base;
		break;
	}

	regoff = clk->reg_off;
	bit_idx = clk->bit_idx;
	regval = readl(regbase + regoff);
	bitval = regval & BIT(bit_idx);

	LOWPM_MSG_CONT(s, " offset:0x%x regval:0x%x bit_idx:%s%d state:%d %s",
		regoff, regval, bit_idx > 9 ? "" : "0",
		bit_idx, bitval ? 1 : 0, clk->clk_name);

	if (bitval != 0) {
		if ((clk->status == NULL_CLOSE) || (clk->status == AP_CLOSE))
			LOWPM_MSG_CONT(s, " -E[OFF]");
	} else {
		if ((clk->status == AP_ON) || (clk->status == AO_ON))
			LOWPM_MSG_CONT(s, " -E[ON]");
	}
	if (clk->status == ON_OFF)
		LOWPM_MSG_CONT(s, "[on/off]");
	else if (clk->status == MODEM_ON)
		LOWPM_MSG_CONT(s, "[MODEM: on or off]");
	LOWPM_MSG_CONT(s, "\n");
}

/*****************************************************************
 * function:    clk_status_show
 * description:
 *  show the clk status.
 ******************************************************************/
void clk_status_show(struct seq_file *s)
{
	unsigned int i;

	if ((g_usavedcfg & DEBG_SUSPEND_CLK_SHOW) != DEBG_SUSPEND_CLK_SHOW)
		return;

	LOWPM_MSG(s, "clk list length is %u:", g_lp_clk_num);

	for (i = 0; i < g_lp_clk_num; i++)
		clk_showone(s, g_clk_lookups[i]);
}


static int init_pmu_lookup_table(void)
{
	int ret = 0;
	u32 i;
	struct device_node *np = NULL;
	char *buffer = NULL;

	buffer = kcalloc(BUFFER_LENGTH, sizeof(char), GFP_KERNEL);
	if (buffer == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	g_hisi_pmu_idx = kzalloc(g_lp_pmu_num * sizeof(struct pmu_idx *), GFP_KERNEL);
	if (g_hisi_pmu_idx == NULL) {
		ret = -ENOMEM;
		goto err_free_buffer;
	}
	g_pmuregs_lookups = kzalloc(g_lp_pmu_num * sizeof(struct pmuregs *), GFP_KERNEL);
	if (g_pmuregs_lookups == NULL) {
		ret = -ENOMEM;
		goto err_free_pmu_idx;
	}

	/* init pmu idx and lookup table */
	for (i = 0; i < g_lp_pmu_num; i++) {
		ret = memset_s(buffer, BUFFER_LENGTH * sizeof(char), 0, BUFFER_LENGTH * sizeof(char));
		if (ret != EOK) {
			pr_err("%s: memset_s buffer err!\n", __func__);
			goto err_free_lookups;
		}
		ret = snprintf_s(buffer, BUFFER_LENGTH * sizeof(char),
				BUFFER_LENGTH * sizeof(char) - 1, "hisilicon, lp_pmu%d", i);
		if (ret < 0)
			pr_err("%s: snprintf_s buffer err.\n", __func__);
		np = of_find_compatible_node(NULL, NULL, buffer);
		if (np == NULL) {
			pr_err("%s[%d]: lp_pmu%d not find.\n", __func__, __LINE__, i);
			ret = -ENODEV;
			goto err_free_lookups;
		}
		g_hisi_pmu_idx[i] = kzalloc(sizeof(struct pmu_idx), GFP_KERNEL);
		if (!g_hisi_pmu_idx[i]) {
			ret = -ENOMEM;
			goto err_put_node;
		}
		ret = of_property_read_string(np, "lp-pmu-name", &(g_hisi_pmu_idx[i]->name));
		if (ret != 0) {
			pr_err("%s, [%d]no lp-pmu-name!\n", __func__, i);
			ret = -ENODEV;
			goto err_put_node;
		}
		ret = of_property_read_string(np, "lp-pmu-module", &(g_hisi_pmu_idx[i]->module));
		if (ret != 0) {
			pr_err("%s, [%d]no lp-pmu-module!\n", __func__, i);
			ret = -ENODEV;
			goto err_put_node;
		}
		ret = of_property_read_u32(np, "lp-pmu-status-value", &(g_hisi_pmu_idx[i]->status));
		if (ret != 0) {
			pr_err("%s, [%d]no lp-pmu-status-value!\n", __func__, i);
			ret = -ENODEV;
			goto err_put_node;
		}
		ret = of_property_read_string(np, "lp-pmu-status", &(g_hisi_pmu_idx[i]->status_name));
		if (ret != 0) {
			pr_err("%s, [%d]no lp-pmu-status!\n", __func__, i);
			ret = -ENODEV;
			goto err_put_node;
		}
		g_pmuregs_lookups[i] = kzalloc(sizeof(struct pmuregs), GFP_KERNEL);
		if (g_pmuregs_lookups[i] == NULL) {
			ret = -ENOMEM;
			goto err_put_node;
		}
		ret = of_property_read_u32(np, "offset", &(g_pmuregs_lookups[i]->ucoffset));
		if (ret != 0) {
			pr_err("%s, [%d]no offset!\n", __func__, i);
			ret = -ENODEV;
			goto err_put_node;
		}
		ret = of_property_read_u32(np, "mask", &(g_pmuregs_lookups[i]->cmask));
		if (ret != 0) {
			pr_err("%s, [%d]no mask!\n", __func__, i);
			ret = -ENODEV;
			goto err_put_node;
		}
		g_pmuregs_lookups[i]->cval = g_pmuregs_lookups[i]->cmask;
		of_node_put(np);
	}

	kfree(buffer);
	pr_info("%s: %d init success.\n", __func__, __LINE__);
	return ret;

err_put_node:
	of_node_put(np);
err_free_lookups:
	lowpm_kfree_array(g_pmuregs_lookups, g_lp_pmu_num, i);
err_free_pmu_idx:
	lowpm_kfree_array(g_hisi_pmu_idx, g_lp_pmu_num, i);
err_free_buffer:
	kfree(buffer);
err:
	pr_err("%s init failed.\n", __func__);
	return ret;
}

static int init_pmu_table(void)
{
	int ret = 0;
	u32 i;
	struct device_node *np = NULL;
	char *buffer = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s[%d]: hisilicon,lowpm_func No compatible node found\n",
				__func__, __LINE__);
		ret = -ENODEV;
		goto err;
	}

	ret = of_property_read_u32(np, "pmu-addr-end", &g_pmu_addr_end);
	if (ret != 0) {
		pr_err("%s[%d], no pmu-addr-end!\n", __func__, __LINE__);
		ret = -ENODEV;
		of_node_put(np);
		goto err;
	}
	of_node_put(np);

	buffer = kcalloc(BUFFER_LENGTH, sizeof(char), GFP_KERNEL);
	if (buffer == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	/* find lp_pmu max num */
	i = 0;
	while (1) {
		ret = memset_s(buffer, BUFFER_LENGTH * sizeof(char), 0, BUFFER_LENGTH * sizeof(char));
		if (ret != EOK) {
			pr_err("%s: memset_s buffer err.\n", __func__);
			goto err_free_buffer;
		}
		ret = snprintf_s(buffer, BUFFER_LENGTH * sizeof(char),
			BUFFER_LENGTH * sizeof(char) - 1, "hisilicon, lp_pmu%d", i);
		if (ret < 0)
			pr_err("%s: snprintf_s buffer err.\n", __func__);
		np = of_find_compatible_node(NULL, NULL, buffer);
		if (np == NULL) {
			pr_err("%s[%d]: lp_pmu%d not find.\n", __func__, __LINE__, i);
			ret = -ENODEV;
			break;
		}
		i++;
		of_node_put(np);
	}
	g_lp_pmu_num = i;
	pr_info("%s: find lp pmu num: %u\n", __func__, g_lp_pmu_num);

	ret = init_pmu_lookup_table();
	if (ret != 0) {
		pr_err("%s[%d]: init_pmu_lookup_table failed!!\n", __func__, __LINE__);
		goto err_free_buffer;
	}

	kfree(buffer);
	pr_info("%s, init pmu table success.\n", __func__);
	return ret;

err_free_buffer:
	kfree(buffer);
err:
	pr_err("%s init failed.\n", __func__);
	return ret;
}

static int init_clk_lookup_table(u32 reg_num)
{
	int ret = 0;
	u32 i, j, k;
	struct device_node *np = NULL;
	u32 reg_base, reg_offset, bit_num;
	char *buffer = NULL;

	buffer = kcalloc(BUFFER_LENGTH, sizeof(char), GFP_KERNEL);
	if (buffer == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	g_clk_lookups = kzalloc(g_lp_clk_num * sizeof(struct lp_clk *), GFP_KERNEL);
	if (g_clk_lookups == NULL) {
		ret = -ENOMEM;
		goto err_free_buffer;
	}

	/* init lp clk table */
	k = 0;
	for (i = 0; i < reg_num; i++) {
		if (k >= g_lp_clk_num) {
			pr_err("%s[%d]: k >= g_lp_clk_num, k:%d,i:%d!\n",
					__func__, __LINE__, k, i);
			goto err_free_lookups;
		}
		ret = memset_s(buffer, BUFFER_LENGTH * sizeof(char),
				0, BUFFER_LENGTH * sizeof(char));
		if (ret != EOK) {
			pr_err("%s: memset_s buffer err.\n", __func__);
			goto err_free_lookups;
		}
		ret = snprintf_s(buffer, BUFFER_LENGTH * sizeof(char),
				BUFFER_LENGTH * sizeof(char) - 1, "hisilicon, lp_clk%d", i);
		if (ret < 0)
			pr_err("%s: snprintf_s buffer err.\n", __func__);
		np = of_find_compatible_node(NULL, NULL, buffer);
		if (np == NULL) {
			pr_err("%s[%d]: lp_clk%d not find.\n", __func__, __LINE__, i);
			goto err_free_lookups;
		}
		ret = of_property_read_u32(np, "lp-clk-base", &reg_base);
		if (ret != 0) {
			pr_err("%s, [%d]no lp-clk-base!\n", __func__, i);
			goto err_put_node;
		}
		ret = of_property_read_u32(np, "offset", &reg_offset);
		if (ret != 0) {
			pr_err("%s, [%d]no offset!\n", __func__, i);
			goto err_put_node;
		}
		ret = of_property_read_u32(np, "bit-num", &bit_num);
		if (ret != 0) {
			pr_err("%s, [%d]no bit-num!\n", __func__, i);
			goto err_put_node;
		}

		for (j = 0; j < bit_num; j++) {
			g_clk_lookups[k] = kzalloc(sizeof(struct lp_clk), GFP_KERNEL);
			if (g_clk_lookups[k] == NULL) {
				ret = -ENOMEM;
				goto err_put_node;
			}

			g_clk_lookups[k]->reg_base = reg_base;
			g_clk_lookups[k]->reg_off = reg_offset;
			g_clk_lookups[k]->bit_idx = j;

			ret = of_property_read_string_index(np, "lp-clk-name", j, &(g_clk_lookups[k]->clk_name));
			if (ret != 0) {
				pr_err("%s, [%d %d]no lp-clk-name!\n", __func__, i, j);
				goto err_put_node;
			}

			ret = of_property_read_u32_index(np, "lp-clk-status-value", j, &(g_clk_lookups[k]->status));
			if (ret != 0) {
				pr_err("%s, [%d %d]no lp-clk-status-value!\n", __func__, i, j);
				goto err_put_node;
			}

			k++;
		}
		of_node_put(np);
	}

	kfree(buffer);
	pr_info("%s: %d init success.\n", __func__, __LINE__);
	return ret;

err_put_node:
	of_node_put(np);
err_free_lookups:
	lowpm_kfree_array(g_clk_lookups, g_lp_clk_num, i);
err_free_buffer:
	kfree(buffer);
err:
	pr_err("%s init failed.\n", __func__);
	return ret;
}

static int init_clk_table(void)
{
	int ret;
	u32 i, bit_num, clk_reg_num;
	struct device_node *np = NULL;
	char *buffer = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s: hisilicon,lowpm_func No compatible node found\n",
				__func__);
		ret = -ENODEV;
		goto err;
	}

	buffer = kmalloc(BUFFER_LENGTH * sizeof(char), GFP_KERNEL);
	if (buffer == NULL) {
		ret = -ENOMEM;
		of_node_put(np);
		goto err;
	}
	of_node_put(np);

	/* find lp_clk max num */
	i = 0;
	while (1) {
		ret = memset_s(buffer, BUFFER_LENGTH * sizeof(char),
				0, BUFFER_LENGTH * sizeof(char));
		if (ret != EOK) {
			pr_err("%s: memset_s buffer err.\n", __func__);
			goto err_free_buffer;
		}
		ret = snprintf_s(buffer, BUFFER_LENGTH * sizeof(char),
				BUFFER_LENGTH * sizeof(char) - 1, "hisilicon, lp_clk%d", i);
		if (ret < 0)
			pr_err("%s: snprintf_s buffer err.\n", __func__);
		np = of_find_compatible_node(NULL, NULL, buffer);
		if (np == NULL) {
			pr_err("%s[%d]: lp_clk%d not find.\n", __func__, __LINE__, i);
			break;
		}
		ret = of_property_read_u32(np, "bit-num", &bit_num);
		if (ret != 0) {
			pr_err("%s, [%d]no bit-num!\n", __func__, i);
			of_node_put(np);
			goto err_free_buffer;
		}
		g_lp_clk_num += bit_num;
		i++;
		of_node_put(np);
	}
	clk_reg_num = i;
	pr_info("%s: find lp clk num: %u, lp clk reg num: %u\n",
				__func__, g_lp_clk_num, clk_reg_num);

	ret = init_clk_lookup_table(clk_reg_num);
	if (ret != 0) {
		pr_err("%s[%d]: init_clk_lookup_table failed!!\n", __func__, __LINE__);
		goto err_free_buffer;
	}

	kfree(buffer);
	pr_info("%s, init clk table success.\n", __func__);

	return ret;

err_free_buffer:
	kfree(buffer);
err:
	pr_err("%s init failed.\n", __func__);
	return ret;
}

#ifdef CONFIG_PM_SEC_IO
static int init_sec_io(struct device_node *np)
{
	int ret = 0;
	int i = 0;

	/* find iomg sec reg range */
	g_mg_sec_reg_num = of_property_count_u32_elems(np, "mg-sec-reg");
	if (g_mg_sec_reg_num < 0) {
		pr_err("%s[%d], no mg-sec-reg!\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err;
	}
	pr_info("%s[%d], find mg-sec-reg num: %d\n", __func__, __LINE__, g_mg_sec_reg_num);
	g_mg_reg = kcalloc(g_mg_sec_reg_num, sizeof(void __iomem *), GFP_KERNEL);
	if (g_mg_reg == NULL) {
		ret = -ENOMEM;
		goto err;
	}
	for (i = 0; i < g_mg_sec_reg_num; i++) {
		ret = of_property_read_u32_index(np, "mg-sec-reg", i, (u32 *) &g_mg_reg[i]);
		if (ret != 0) {
			pr_err("%s[%d], i = %d, no mg-sec-reg!\n", __func__, __LINE__, i);
			ret = -ENODEV;
			goto err_free_iomg;
		}
	}

	/* find iocg sec reg range */
	g_cg_sec_reg_num = of_property_count_u32_elems(np, "cg-sec-reg");
	if (g_cg_sec_reg_num < 0) {
		pr_err("%s[%d], no mg-sec-reg!\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err_free_iomg;
	}
	pr_info("%s[%d], find cg-sec-reg num: %d\n", __func__, __LINE__, g_cg_sec_reg_num);
	g_cg_reg = kcalloc(g_cg_sec_reg_num, sizeof(void __iomem *), GFP_KERNEL);
	if (g_cg_reg == NULL) {
		ret = -ENOMEM;
		goto err_free_iomg;
	}
	for (i = 0; i < g_cg_sec_reg_num; i++) {
		ret = of_property_read_u32_index(np, "cg-sec-reg", i, (u32 *) &g_cg_reg[i]);
		if (ret != 0) {
			pr_err("%s[%d], i = %d, no cg-sec-reg!\n", __func__, __LINE__, i);
			ret = -ENODEV;
			goto err_free_iocg;
		}
	}

	pr_info("%s:%d init sec ioc success!\n", __func__, __LINE__);
	return ret;

err_free_iocg:
	kfree(g_cg_reg);
	g_cg_reg = NULL;
err_free_iomg:
	kfree(g_mg_reg);
	g_mg_reg = NULL;
err:
	pr_err("%s: init fail!\n", __func__);
	return ret;

}
#endif



static int init_lowpm_data(void)
{
	int ret;
	u32 i;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s: hisilicon,lowpm_func No compatible node found\n",
				__func__);
		ret = -ENODEV;
		goto err;
	}

	ret = of_property_read_string(np, "plat-name", &g_plat_name);
	if (ret != 0) {
		pr_err("%s[%d], no plat-name!\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err_put_node;
	}

#ifdef CONFIG_PM_SEC_IO
	/* init sec IO */
	ret = init_sec_io(np);
	if (ret < 0) {
		pr_err("%s:%d init sec IO failed!\n", __func__, __LINE__);
		goto err_put_node;
	}
#endif


	/* init lpmcu irq table */
	ret = of_property_count_strings(np, "lpmcu-irq-table");
	if (ret < 0) {
		pr_err("%s, not find irq-table property!\n", __func__);
		goto err_put_node;
	}

	g_lpmcu_irq_num = ret;
	pr_info("%s, lpmcu-irq-table num: %d!\n", __func__, g_lpmcu_irq_num);

	g_lpmcu_irq_name = kcalloc(g_lpmcu_irq_num, sizeof(char *), GFP_KERNEL);
	if (g_lpmcu_irq_name == NULL) {
		ret = -ENOMEM;
		goto err_put_node;
	}

	for (i = 0; i < g_lpmcu_irq_num; i++) {
		ret = of_property_read_string_index(np, "lpmcu-irq-table",
				i, &g_lpmcu_irq_name[i]);
		if (ret != 0) {
			pr_err("%s, no lpmcu-irq-table %d!\n", __func__, i);
			goto err_free;
		}
	}
	of_node_put(np);

	ret = init_pmu_table();
	if (ret != 0)
		goto err_free;

	ret = init_clk_table();
	if (ret != 0)
		goto err_free;

	pr_info("%s, init lowpm data success.\n", __func__);

	return ret;

err_free:
	kfree(g_lpmcu_irq_name);
	g_lpmcu_irq_name = NULL;
err_put_node:
	of_node_put(np);
err:
	pr_err("%s: init fail!\n", __func__);
	return ret;
}

/*****************************************************************
 * function:    lowpm_func_probe
 * description:
 * driver interface.
 ******************************************************************/
static int lowpm_func_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret;


	pr_info("[%s] %d enter.\n", __func__, __LINE__);

	g_suspended = 0;

	ret = map_sysregs();
	if (ret != 0) {
		pr_err("%s: %d map_sysregs failed.\n", __func__, __LINE__);
		goto err;
	}

	ret = map_io_regs();
	if (ret != 0) {
		pr_err("%s: %d map_io_regs failed.\n", __func__, __LINE__);
		goto err;
	}

	ret = init_lowpm_data();
	if (ret != 0) {
		pr_err("%s: %d init_lowpm_data failed.\n", __func__, __LINE__);
		goto err;
	}

	wakeup_source_init(&lowpm_wake_lock, "lowpm_func");

#ifndef CONFIG_HISI_SR_DEBUG_SLEEP
	set_wakelock(1);
#endif


	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s: hisilicon,lowpm_func No compatible node found\n",
				__func__);
		return -ENODEV;
	}

	ret = of_property_read_u32_array(np, "fpga_flag", &g_lp_fpga_flag, 1);
	if (ret != 0)
		pr_err("%s , no fpga_flag.\n", __func__);
	of_node_put(np);

	pr_info("[%s] %d leave.\n", __func__, __LINE__);

	return ret;

err:
	pr_err("%s: fail!\n", __func__);
	return ret;
}

/*****************************************************************
 * function:    lowpm_func_remove
 * description:
 *  driver interface.
 ******************************************************************/
static int lowpm_func_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM
static int lowpm_func_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	g_suspended = 1;
	return 0;
}

static int lowpm_func_resume(struct platform_device *pdev)
{
	g_suspended = 0;
	return 0;
}
#else
#define lowpm_func_suspend	NULL
#define lowpm_func_resume	NULL
#endif

#define MODULE_NAME		"hisilicon,lowpm_func"

static const struct of_device_id lowpm_func_match[] = {
	{ .compatible = MODULE_NAME },
	{},
};

static struct platform_driver lowpm_func_drv = {
	.probe		= lowpm_func_probe,
	.remove		= lowpm_func_remove,
	.suspend	= lowpm_func_suspend,
	.resume		= lowpm_func_resume,
	.driver = {
		.name	= MODULE_NAME,
		.of_match_table = of_match_ptr(lowpm_func_match),
	},
};

static int __init lowpmreg_init(void)
{
	return platform_driver_register(&lowpm_func_drv);
}

static void __exit lowpmreg_exit(void)
{
	platform_driver_unregister(&lowpm_func_drv);
}


module_init(lowpmreg_init);
module_exit(lowpmreg_exit);
