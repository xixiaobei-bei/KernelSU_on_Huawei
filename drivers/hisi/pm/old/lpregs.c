/*
 * lpreg.c
 *
 * debug information of suspend
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
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

#include "lpregs.h"
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
#include <linux/debugfs.h>
#include <linux/seq_buf.h>
#include <linux/uaccess.h>
#ifdef CONFIG_HISI_PMIC_PMU_SPMI
#include <linux/mfd/hisi_pmic.h>
#endif
#include <securec.h>
#include <linux/hisi/hisi_gpio_auto_gen.h>

#ifdef CONFIG_HUAWEI_DUBAI
#include <huawei_platform/power/dubai/dubai_wakeup_stats.h>
#endif

#include <pm_def.h>
#include <m3_interrupts.h>
#ifdef CONFIG_HISI_LPMCU_BB
#include <m3_rdr_ddr_map.h>
#include <lpmcu_runtime.h>
#endif

#define WAKE_IRQ_NUM_MAX		32

#define IPC_PROCESSOR_MAX		6
#define NSIPC_MAILBOX_MAX		27
#define AP_IPC_PROC_MAX		2
#define PROCESSOR_IOMCU_LENGTH		5
#define IPC_MBXDATA_MIN		0
#define IPC_MBXDATA_MAX		8
#ifdef CONFIG_HUAWEI_DUBAI
#define IPC_MBXDATA_TAG		2
#define get_sharemem_data(m)		((m) & 0xFFFF)
#define get_sharemem_source(m)		((m) & 0xFF)
#endif
#define ipc_mbx_source_offset(m)		((m) << 6)
#define ipc_mbx_dstatus_offset(m)		(0x0C + ((m) << 6))
#define ipc_mbxdata_offset(m, idex)		(0x20 + 4 * (idex) + ((m) << 6))

#define NO_SEQFILE		0
#define lowpm_msg(seq_file, fmt, args ...) \
	do { \
		if (seq_file == NO_SEQFILE) \
			pr_info(fmt"\n", ##args); \
		else \
			seq_printf(seq_file, fmt"\n", ##args); \
	} while (0)
#define lowpm_msg_cont(seq_file, fmt, args ...) \
	do { \
		if (seq_file == NO_SEQFILE) \
			pr_cont(fmt, ##args); \
		else \
			seq_printf(seq_file, fmt, ##args); \
	} while (0)

#define CRG_PERRSTDIS2_ip_prst_uart0_BIT	SOC_CRGPERIPH_PERRSTDIS2_ip_prst_uart0_START

#define MODULE_NAME		"hisilicon,lowpm_func"

#define UART_LPM3_IDX		6
#define UART_CRGPERI_IDX		0
#define ADDR_SIZE		0x4
#define UART_BAUD_DIVINT_VAL		0xa
#define UART_BAUD_DIVFRAC_VAL		0x1b
#define UART_RXIFLSEL_VAL		0x92
#define UART_WLEN_VAL		0x3
#define TICKMARK_STEP		4
#define TICKMARK_US		31 /* 1000000 / 32768 */

/* struct g_sysreg_bases to hold the base address of some system registers. */
struct sysreg_base_addr {
	void __iomem *uart_base;
	void __iomem *pctrl_base;
	void __iomem *sysctrl_base;
	void __iomem **gpio_base;
	void __iomem *crg_base;
	void __iomem *pmic_base;
	void __iomem *reserved_base;
	void __iomem *nsipc_base;
#ifdef AP_AO_NSIPC_IRQ
	void __iomem *ao_nsipc_base;
#endif
};

static struct sysreg_base_addr g_sysreg_base;
static unsigned int g_lpmcu_irq_num;
static const char **g_lpmcu_irq_name = NULL;

static unsigned int g_lp_fpga_flag;

static int g_suspended;
static struct wakeup_source g_lowpm_wake_lock;

static char *g_processor_name[IPC_PROCESSOR_MAX] = {
	"gic1",
	"gic2",
	"iomcu",
	"lpm3",
	"hifi",
	"modem"
};


static int get_ipc_addr(void)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "lowpm,ipc");
	if (np == NULL) {
		pr_err("%s: dts[%s] node not found\n", __func__, "lowpm,ipc");
		ret = -ENODEV;
		goto err;
	}
	g_sysreg_base.nsipc_base = of_iomap(np, 0);
	pr_info("%s: nsipc_base:%pK\n", __func__, g_sysreg_base.nsipc_base);
	if (g_sysreg_base.nsipc_base == NULL) {
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
	g_sysreg_base.ao_nsipc_base = of_iomap(np, 0);
	pr_info("%s: ao_nsipc_base:%pK\n",
		__func__, g_sysreg_base.ao_nsipc_base);
	if (g_sysreg_base.ao_nsipc_base == NULL) {
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

static struct device_node *find_uart_device_node(void)
{
	unsigned int uart_idx;
	struct device_node *np = NULL;

	uart_idx = get_console_index();
	switch (uart_idx) {
	case UART_CRGPERI_IDX:
		np = of_find_compatible_node(NULL, NULL, "arm,pl011");
		if (np != NULL)
			return np;
		np = of_find_compatible_node(NULL, NULL, "hisilicon,uart");
		if (np == NULL)
			pr_err("%s: dts[%s] node not found\n",
				__func__, "arm,pl011");
		break;
	case UART_LPM3_IDX:
		np = of_find_compatible_node(NULL, NULL,
					     "hisilicon,lowpm_func");
		if (np == NULL)
			pr_err("%s: dts[%s] node not found\n",
				__func__, "hisilicon,lowpm_func");
		break;
	default:
		pr_err("%s:uart default:%d\n", __func__, uart_idx);
	}
	return np;
}

static int map_sysregs(void)
{
	int ret;
	struct device_node *np = NULL;
	np = find_uart_device_node();
	if (np == NULL) {
		ret = -ENODEV;
		goto err;
	}
	g_sysreg_base.uart_base = of_iomap(np, 0);
	pr_debug("%s: uart_base:%pK\n", __func__, g_sysreg_base.uart_base);
	if (g_sysreg_base.uart_base == NULL) {
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
	g_sysreg_base.sysctrl_base = of_iomap(np, 0);
	pr_debug("%s: sysctrl_base:%pK\n",
		 __func__, g_sysreg_base.sysctrl_base);
	if (g_sysreg_base.sysctrl_base == NULL) {
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
	g_sysreg_base.pctrl_base = of_iomap(np, 0);
	pr_debug("%s: pctrl_base:%pK\n", __func__, g_sysreg_base.pctrl_base);
	if (g_sysreg_base.pctrl_base == NULL) {
		pr_err("%s: pctrl_base of iomap fail!\n", __func__);
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
	g_sysreg_base.crg_base = of_iomap(np, 0);
	pr_debug("%s: crg_base:%pK\n", __func__, g_sysreg_base.crg_base);
	if (g_sysreg_base.crg_base == NULL) {
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
	g_sysreg_base.pmic_base = of_iomap(np, 0);
	pr_debug("%s: pmic_base:%pK\n", __func__, g_sysreg_base.pmic_base);
	if (g_sysreg_base.pmic_base == NULL) {
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

#define UART_FIFO_TX_LEVEL      3 /* FIFO≤3/4 full */
#define UART_FIFO_RX_LEVEL      0 /* FIFO≥1/8 full */
#define UART_FRAME_CFG_VAL  0x3
#define HM_EN(n)	(0x10001U << (n))
#define HM_DIS(n)	(0x10000U << (n))
void debuguart_reinit(void)
{
	unsigned int uart_idx, uregv;

	if (console_suspend_enabled == 1 ||
	    g_sysreg_base.uart_base == NULL ||
	    g_sysreg_base.crg_base == NULL)
		return;

	uart_idx = get_console_index();
	if (uart_idx == UART_LPM3_IDX) {
		/* enable clk */
		uregv = (unsigned int)readl(SOC_SCTRL_SCLPMCUCLKEN_ADDR(g_sysreg_base.sysctrl_base)) |
					    BIT(SOC_SCTRL_SCLPMCUCLKEN_uart_clk_clkoff_sys_n_START);
		writel(uregv, SOC_SCTRL_SCLPMCUCLKEN_ADDR(g_sysreg_base.sysctrl_base));

		/* reset undo */
		writel(BIT(SOC_SCTRL_SCLPMCURSTDIS_uart_soft_rst_req_START),
		       SOC_SCTRL_SCLPMCURSTDIS_ADDR(g_sysreg_base.sysctrl_base));
	} else if (uart_idx == UART_CRGPERI_IDX) {
		/* enable clk */
		writel(BIT(SOC_CRGPERIPH_PEREN2_gt_clk_uart0_START),
		       SOC_CRGPERIPH_PEREN2_ADDR(g_sysreg_base.crg_base));
		/* reset undo */
		writel(BIT(CRG_PERRSTDIS2_ip_prst_uart0_BIT),
		       SOC_CRGPERIPH_PERRSTDIS2_ADDR(g_sysreg_base.crg_base));
	} else {
		return;
	}

	/* disable uart */
	writel(0x0, SOC_UART_UARTCR_ADDR(g_sysreg_base.uart_base));

	/* set baudrate: 19.2M 115200 */
	writel(UART_BAUD_DIVINT_VAL, SOC_UART_UARTIBRD_ADDR(g_sysreg_base.uart_base));
	writel(UART_BAUD_DIVFRAC_VAL, SOC_UART_UARTFBRD_ADDR(g_sysreg_base.uart_base));

	/* disable FIFO */
	writel(0x0, SOC_UART_UARTLCR_H_ADDR(g_sysreg_base.uart_base));

	/* set FIFO depth: 1/2 */
	writel(UART_RXIFLSEL_VAL, SOC_UART_UARTIFLS_ADDR(g_sysreg_base.uart_base));

	/* set int mask */
	uregv = BIT(SOC_UART_UARTIMSC_rtim_START) | BIT(SOC_UART_UARTIMSC_rxim_START);
	writel(uregv, SOC_UART_UARTIMSC_ADDR(g_sysreg_base.uart_base));

	/* enable FIFO */
	writel(BIT(SOC_UART_UARTLCR_H_fen_START) |
	       (UART_WLEN_VAL << SOC_UART_UARTLCR_H_wlen_START),
	       SOC_UART_UARTLCR_H_ADDR(g_sysreg_base.uart_base));

	/* enable uart trans */
	uregv = BIT(SOC_UART_UARTCR_uarten_START) |
		BIT(SOC_UART_UARTCR_rxe_START) |
		BIT(SOC_UART_UARTCR_txe_START);
	writel(uregv, SOC_UART_UARTCR_ADDR(g_sysreg_base.uart_base));
}


static unsigned int proc_mask_to_id(unsigned int mask)
{
	unsigned int i;

	for (i = 0; i < IPC_PROCESSOR_MAX; i++) {
		if ((mask & BIT(i)) != 0)
			break;
	}

	return i;
}

static void ipc_mbx_irq_show(struct seq_file *s, const void __iomem *base, unsigned int mbx)
{
	unsigned int ipc_source, ipc_dest, ipc_data, src_id, dest_id;
	unsigned int i;
#ifdef CONFIG_HUAWEI_DUBAI
	unsigned int source;
	unsigned int mem = 0;
#endif

	ipc_source = readl(base + ipc_mbx_source_offset(mbx));
	src_id = proc_mask_to_id(ipc_source);
	if (src_id < AP_IPC_PROC_MAX) {
		/* is ack irq */
		ipc_dest = readl(base + ipc_mbx_dstatus_offset(mbx));
		dest_id = proc_mask_to_id(ipc_dest);
		if (dest_id < IPC_PROCESSOR_MAX)
			lowpm_msg(s, "SR:mailbox%u: ack from %s",
				  mbx, g_processor_name[dest_id]);
		else
			lowpm_msg(s, "SR:mailbox%u: ack from unknown(dest_id = %u)",
				  mbx, dest_id);
	} else if (src_id < IPC_PROCESSOR_MAX) {
		/* is receive irq */
		lowpm_msg(s, "SR:mailbox%u: send by %s",
			  mbx, g_processor_name[src_id]);
	} else {
		lowpm_msg(s, "SR:mailbox%u: src id unknown(src_id = %u)",
			  mbx, src_id);
	}

	for (i = 0; i < IPC_MBXDATA_MAX; i++) {
		ipc_data = readl(base + ipc_mbxdata_offset(mbx, i));
#ifdef CONFIG_HUAWEI_DUBAI
		if (src_id < IPC_PROCESSOR_MAX &&
		    strncmp(g_processor_name[src_id], "iomcu",
			    PROCESSOR_IOMCU_LENGTH) == 0) {
			if (i == IPC_MBXDATA_MIN)
				mem = get_sharemem_data(ipc_data);
			else if (i == IPC_MBXDATA_TAG) {
				source = get_sharemem_source(ipc_data);
				dubai_log_wakeup_info("DUBAI_TAG_SENSORHUB_WAKEUP",
							 "mem=%u source=%u",
							 mem, source);
			}
		}
#endif
		lowpm_msg(s, "SR:[MBXDATA%u]:0x%x", i, ipc_data);
	}
}

static void combo_ipc_irq_show(struct seq_file *s, unsigned int nsipc_state)
{
	unsigned int mbx;

	/* ns ipc irq detail show */
	lowpm_msg(s, "SR:nsipc irq state:0x%x", nsipc_state);
	if (g_sysreg_base.nsipc_base == NULL) {
		lowpm_msg(s, "SR:nsipc base is NULL");
		return;
	}
	for (mbx = 0; mbx <= NSIPC_MAILBOX_MAX; mbx++) {
		if ((nsipc_state & BIT(mbx)) > 0)
			ipc_mbx_irq_show(s, g_sysreg_base.nsipc_base, mbx);
	}
}

#ifdef AP_AO_NSIPC_IRQ
static char *g_aoipc_processor_name[AO_IPC_PROCESSOR_MAX] = {
	"iomcu",
	"gic",
	"isp"
};

static unsigned int ao_proc_mask_to_id(unsigned int mask)
{
	unsigned int i;

	for (i = 0; i < AO_IPC_PROCESSOR_MAX; i++) {
		if ((mask & BIT(i)) != 0)
			break;
	}

	return i;
}

static void ao_ipc_mbx_irq_show(struct seq_file *s, const void __iomem *base,
				unsigned int mbx)
{
	unsigned int ipc_source, ipc_dest, ipc_data, src_id, dest_id;
	unsigned int i;
#ifdef CONFIG_HUAWEI_DUBAI
	unsigned int source;
	unsigned int mem = 0;
#endif

	ipc_source = readl(base + ipc_mbx_source_offset(mbx));
	src_id = ao_proc_mask_to_id(ipc_source);
	if (src_id == AP_IPC_PROC_BIT) {
		/* is ack irq */
		ipc_dest = readl(base + ipc_mbx_dstatus_offset(mbx));
		dest_id = ao_proc_mask_to_id(ipc_dest);
		if (dest_id < AO_IPC_PROCESSOR_MAX)
			lowpm_msg(s, "SR:mailbox%u: ack from %s",
				  mbx, g_aoipc_processor_name[dest_id]);
		else
			lowpm_msg(s, "SR:mailbox%u: ack from unknown(dest_id = %u)",
				  mbx, dest_id);
	} else if (src_id < AO_IPC_PROCESSOR_MAX) {
		/* is receive irq */
		lowpm_msg(s, "SR:mailbox%u: send by %s",
			  mbx, g_aoipc_processor_name[src_id]);
	} else {
		lowpm_msg(s, "SR:mailbox%u: src id unknown(src_id = %u)",
			  mbx, src_id);
	}

	for (i = 0; i < IPC_MBXDATA_MAX; i++) {
		ipc_data = readl(base + ipc_mbxdata_offset(mbx, i));
#ifdef CONFIG_HUAWEI_DUBAI
		if (src_id < AO_IPC_PROCESSOR_MAX &&
		    strncmp(g_aoipc_processor_name[src_id], "iomcu",
			    PROCESSOR_IOMCU_LENGTH) == 0) {
			if (i == IPC_MBXDATA_MIN) {
				mem = get_sharemem_data(ipc_data);
			} else if (i == IPC_MBXDATA_TAG) {
				source = get_sharemem_source(ipc_data);
				dubai_log_wakeup_info("DUBAI_TAG_SENSORHUB_WAKEUP",
							 "mem=%u source=%u",
							 mem, source);
			}
		}
#endif
		lowpm_msg(s, "SR:[MBXDATA%u]:0x%x", i, ipc_data);
	}
}

static void combo_ao_ipc_irq_show(struct seq_file *s,
				  unsigned int ao_nsipc_state)
{
	unsigned int mbx;

	/* ns ipc irq detail show */
	lowpm_msg(s, "SR:ao nsipc irq state:0x%x", ao_nsipc_state);
	if (g_sysreg_base.ao_nsipc_base == NULL) {
		lowpm_msg(s, "SR:ao nsipc base is NULL");
		return;
	}
	for (mbx = 0; mbx <= AO_NSIPC_MAILBOX_MAX; mbx++)
		if ((ao_nsipc_state & BIT(mbx)) > 0)
			ao_ipc_mbx_irq_show(s, g_sysreg_base.ao_nsipc_base, mbx);
}
#endif

#ifdef CONFIG_HISI_LPMCU_BB


static int pm_mask_bit(unsigned int val, unsigned int mask)
{
	if ((val & mask) != 0)
		return 1;
	else
		return 0;
}

static void pm_show_wake_status(struct seq_file *s)
{
	unsigned int wake_status;

	wake_status = readl(g_sysreg_base.reserved_base + WAKE_STATUS_OFFSET);
	lowpm_msg(s,
		  "SR:wakelock status[0x%x], ap:%d, modem:%d, hifi:%d, iomcu:%d, general_see:%d, hotplug:%d,%d.",
		  wake_status,
		  pm_mask_bit(wake_status, AP_MASK),
		  pm_mask_bit(wake_status, MODEM_MASK),
		  pm_mask_bit(wake_status, HIFI_MASK),
		  pm_mask_bit(wake_status, IOMCU_MASK),
		  pm_mask_bit(wake_status, GENERAL_SEE_MASK),
		  pm_mask_bit(wake_status, HOTPLUG_MASK(0)),
		  pm_mask_bit(wake_status, HOTPLUG_MASK(1)));
#ifdef CONFIG_HUAWEI_DUBAI
	dubai_log_wakeup_info("DUBAI_TAG_WAKE_STATUS",
				 "ap=%d modem=%d hifi=%d iomcu=%d general_see=%d hotplug0=%d hotplug1=%d",
				  pm_mask_bit(wake_status, AP_MASK),
				  pm_mask_bit(wake_status, MODEM_MASK),
				  pm_mask_bit(wake_status, HIFI_MASK),
				  pm_mask_bit(wake_status, IOMCU_MASK),
				  pm_mask_bit(wake_status, GENERAL_SEE_MASK),
				  pm_mask_bit(wake_status, HOTPLUG_MASK(0)),
				  pm_mask_bit(wake_status, HOTPLUG_MASK(1)));
#endif
	lowpm_msg(s, "SR:system sleeped %u times.",
		  readl(g_sysreg_base.reserved_base + SYS_DSLEEP_CNT_OFFSET));
	lowpm_msg(s,
		  "SR:wake times, system:%u, woken up by ap:%u, modem:%u, hifi:%u, iomcu:%u, lpm3:%u.",
		  readl(g_sysreg_base.reserved_base + SYS_DWAKE_CNT_OFFSET),
		  readl(g_sysreg_base.reserved_base + AP_WAKE_CNT_OFFSET),
		  readl(g_sysreg_base.reserved_base + MODEM_WAKE_CNT_OFFSET),
		  readl(g_sysreg_base.reserved_base + HIFI_WAKE_CNT_OFFSET),
		  readl(g_sysreg_base.reserved_base + IOMCU_WAKE_CNT_OFFSET),
		  readl(g_sysreg_base.reserved_base + LPM3_WAKE_CNT_OFFSET));
}

static void pm_show_wake_irqs(struct seq_file *s)
{
	unsigned int wake_irq, wake_irq1, ap_irq, bit_shift;
	unsigned int i;

	wake_irq = readl(g_sysreg_base.reserved_base + WAKE_IRQ_OFFSET);
	wake_irq1 = readl(g_sysreg_base.reserved_base + WAKE_IRQ1_OFFSET);
	lowpm_msg(s, "SR:WAKE_IRQ:0x%x, WAKE_IRQ1:0x%x", wake_irq, wake_irq1);
	for (i = 0; i < WAKE_IRQ_NUM_MAX; i++) {
		bit_shift = 1 << i;
		if ((((wake_irq & AP_WAKE_INT_MASK) |
		      (wake_irq1 & AP_WAKE_INT_MASK1)) & bit_shift) != 0)
			lowpm_msg(s, "LAST DEEPSLEEP WAKE BIT: %d(AP)", i);

		if (((wake_irq & IOMCU_WAKE_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST DEEPSLEEP WAKE BIT: %d(IOMCU)", i);

		if (((wake_irq & MODEM_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST DEEPSLEEP WAKE BIT: %d(MODEM)", i);

		if (((wake_irq1 & MODEM_DRX_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST DEEPSLEEP WAKE DRX_BIT: %d(MODEM)", i);

		if (((wake_irq & HIFI_WAKE_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST DEEPSLEEP WAKE BIT: %d(HIFI)", i);
	}

	wake_irq = readl(g_sysreg_base.reserved_base + SLEEP_WAKE_IRQ_OFFSET);
	wake_irq1 = readl(g_sysreg_base.reserved_base + SLEEP_WAKE_IRQ1_OFFSET);
	for (i = 0; i < WAKE_IRQ_NUM_MAX; i++) {
		bit_shift = 1 << i;
		if ((((wake_irq & AP_WAKE_INT_MASK) |
		      (wake_irq1 & AP_WAKE_INT_MASK1)) & bit_shift) != 0)
			lowpm_msg(s, "LAST SLEEP WAKE BIT: %d(AP)", i);

		if (((wake_irq & IOMCU_WAKE_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST SLEEP WAKE BIT: %d(IOMCU)", i);

		if (((wake_irq & MODEM_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST SLEEP WAKE BIT: %d(MODEM)", i);

		if (((wake_irq1 & MODEM_DRX_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST SLEEP WAKE DRX_BIT: %d(MODEM)", i);

		if (((wake_irq & HIFI_WAKE_INT_MASK) & bit_shift) != 0)
			lowpm_msg(s, "LAST SLEEP WAKE BIT: %d(HIFI)", i);
	}

	ap_irq = readl(g_sysreg_base.reserved_base + AP_WAKE_IRQ_OFFSET);
	if (g_lpmcu_irq_num > ap_irq && g_lpmcu_irq_name != NULL) {
#ifdef CONFIG_HUAWEI_DUBAI
		dubai_log_irq_wakeup(DUBAI_IRQ_WAKEUP_TYPE_LPMCU, g_lpmcu_irq_name[ap_irq], -1);
#endif
		lowpm_msg(s, "SR:AP WAKE IRQ(LPM3 NVIC): %d (%s)",
			  ap_irq, g_lpmcu_irq_name[ap_irq]);
	} else {
		lowpm_msg(s, "SR:AP WAKE IRQ(LPM3 NVIC): %d (no exist)", ap_irq);
	}


	if (ap_irq == IRQ_COMB_GIC_IPC) {
		combo_ipc_irq_show(s, readl(g_sysreg_base.reserved_base +
					    AP_NSIPC_IRQ_OFFSET));
#ifdef AP_AO_NSIPC_IRQ
		combo_ao_ipc_irq_show(s, readl(g_sysreg_base.reserved_base +
					       AP_AO_NSIPC_IRQ_OFFSET));
#endif
	}
}
#endif

void pm_status_show(struct seq_file *s)
{
#ifdef CONFIG_HISI_LPMCU_BB
	if (g_sysreg_base.reserved_base == NULL) {
		if (M3_RDR_SYS_CONTEXT_BASE_ADDR != 0) {
			g_sysreg_base.reserved_base = M3_RDR_SYS_CONTEXT_RUNTIME_VAR_ADDR;
		} else{
			pr_err("%s: M3_RDR_SYS_CONTEXT_BASE_ADDR is NULL\n", __func__);
			return;
		}
	}

	pm_show_wake_status(s);

	lowpm_msg_cont(s,
		       "SR:sr times of sub system, ap:s-%u, r-%u, modem:s-%u, r-%u, hifi:s-%u, r-%u, iomcu:s-%u, r-%u.",
		       readl(g_sysreg_base.reserved_base + AP_SUSPEND_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + AP_RESUME_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + MODEM_SUSPEND_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + MODEM_RESUME_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + HIFI_SUSPEND_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + HIFI_RESUME_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + IOMCU_SUSPEND_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + IOMCU_RESUME_CNT_OFFSET));
#if defined(GENERAL_SEE_SUSPEND_CNT_OFFSET) && defined(GENERAL_SEE_RESUME_CNT_OFFSET)
	lowpm_msg_cont(s,
		       "general_see:s-%u, r-%u.",
		       readl(g_sysreg_base.reserved_base + GENERAL_SEE_SUSPEND_CNT_OFFSET),
		       readl(g_sysreg_base.reserved_base + GENERAL_SEE_RESUME_CNT_OFFSET));
#endif
	lowpm_msg_cont(s, "\n");


	if (g_sysreg_base.sysctrl_base != NULL) {
		lowpm_msg(s, "SR:SCINT_STAT:0x%x.",
			  readl(g_sysreg_base.sysctrl_base + SOC_SCTRL_SCINT_STAT_ADDR(0)));
		lowpm_msg(s, "SR:SCINT_STAT1:0x%x.",
			  readl(g_sysreg_base.sysctrl_base + SOC_SCTRL_SCINT_STAT1_ADDR(0)));
	}

	pm_show_wake_irqs(s);

	lowpm_msg(s, "sleep 0x%x times.",
		  readl(g_sysreg_base.reserved_base + SYS_SLEEP_CNT_OFFSET));




#endif
}

#if defined(CONFIG_SR_DEBUG_LPREG) || (!defined(CONFIG_SR_DEBUG_SLEEP))
static void set_wakelock(int iflag)
{
	if (iflag == 1 && g_lowpm_wake_lock.active == 0)
		__pm_stay_awake(&g_lowpm_wake_lock);
	else if (iflag == 0 && g_lowpm_wake_lock.active != 0)
		__pm_relax(&g_lowpm_wake_lock);
}
#endif



static int init_lowpm_data(void)
{
	int ret;
	u32 i;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s: lowpm_func No compatible node found\n",
		       __func__);
		ret = -ENODEV;
		goto err;
	}

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

	ret = init_lowpm_data();
	if (ret != 0) {
		pr_err("%s: %d init_lowpm_data failed.\n", __func__, __LINE__);
		goto err;
	}

	wakeup_source_init(&g_lowpm_wake_lock, "lowpm_func");

#ifndef CONFIG_SR_DEBUG_SLEEP
	set_wakelock(1);
#endif


	np = of_find_compatible_node(NULL, NULL, "hisilicon,lowpm_func");
	if (np == NULL) {
		pr_err("%s: lowpm_func No compatible node found\n",
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
#endif

static const struct of_device_id lowpm_func_match[] = {
	{ .compatible = MODULE_NAME },
	{},
};

static struct platform_driver lowpm_func_drv = {
	.probe = lowpm_func_probe,
	.remove = lowpm_func_remove,
#ifdef CONFIG_PM
	.suspend = lowpm_func_suspend,
	.resume = lowpm_func_resume,
#endif
	.driver = {
		.name = MODULE_NAME,
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
