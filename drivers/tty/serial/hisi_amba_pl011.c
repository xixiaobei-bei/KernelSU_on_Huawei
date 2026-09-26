/*
 * Hisilicon Amba-pl011.c. Driver for AMBA serial ports
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <securec.h>
#include <pr_log.h>

#define TX_WORK_TIMEOUT 1600000
#define PR_LOG_TAG AMBA_PL011_TAG
#define SET_CONSOLE_FIFO_SIZE_DEFAULT 262144
#define SET_CONSOLE_FIFO_CPUON_DEFAULT 3
#define FIFO_SIZE_ARM 64
#define MAX_DBG_FS_NAME_LEN 16
#define RESET_WAIT_TIMES 10
#define MAX_CONSOLE_UART_NAME_LEN 8
#define UAP_REG_CFG_NUM 4

static int console_fifo_enable_status = -1;
static int __init console_fifo_enable_setup(char *str)
{
	char buf[2] = {0}; /* 2 for "1 or 0" */
	size_t index;

	if (strncpy_s(buf, sizeof(buf), str, sizeof(buf) - 1)) {
		pr_err("%s: strncpy_s failed\n", __func__);
		return 0;
	}

	/*
	 * console fifo enable is 1
	 * console fifo disable is 0
	 */
	for (index = 0; index < sizeof(buf); index++) {
		if (buf[index] == '1') {
			console_fifo_enable_status = 1;
			break;
		}
		if (buf[index] == '0' || buf[index] == ' ') {
			console_fifo_enable_status = 0;
			break;
		}
	}

	return 1;
}
__setup("console_fifo_status=", console_fifo_enable_setup);

unsigned int get_fifosize_arm(struct amba_device *dev)
{
	return FIFO_SIZE_ARM;
}

unsigned int pl011_reg_to_offset(
	const struct uart_amba_port *uap, unsigned int reg)
{
	return uap->reg_offset[reg];
}

unsigned int pl011_read(
	const struct uart_amba_port *uap, unsigned int reg)
{
	void __iomem *addr = NULL;

	addr = uap->port.membase + pl011_reg_to_offset(uap, reg);
	return (uap->port.iotype == UPIO_MEM32) ? readl_relaxed(addr)
		: readw_relaxed(addr);
}

void pl011_write(
	unsigned int val, const struct uart_amba_port *uap, unsigned int reg)
{
	void __iomem *addr = NULL;

	addr = uap->port.membase + pl011_reg_to_offset(uap, reg);
	if (uap->port.iotype == UPIO_MEM32)
		writel_relaxed(val, addr);
	else
		writew_relaxed(val, addr);
}

bool pl011_split_lcrh(const struct uart_amba_port *uap)
{
	return pl011_reg_to_offset(uap, REG_LCRH_RX) !=
		pl011_reg_to_offset(uap, REG_LCRH_TX);
}

int pl011_tx_stat_show(struct seq_file *s, void *arg)
{
	unsigned int uart_tx_buf_size;
	struct uart_tx_unit *unit = (struct uart_tx_unit *)s->private;

	uart_tx_buf_size = kfifo_size(&unit->tx_fifo);
	seq_puts(s, "console transfer statics:\n");
	seq_printf(s, "fifo over cnt : %lu\n", unit->tx_buf_over_cnt);
	seq_printf(s, "fifo over len : %lu\n", unit->tx_buf_over_size);
	seq_printf(s, "fifo capacity : 0x%lx(%luk)\n",
		(unsigned long)uart_tx_buf_size,
		(unsigned long)uart_tx_buf_size / 1024); /* convert to K byte */
	seq_printf(s, "fifo required : 0x%lx(%luk)\n", unit->tx_buf_max,
		unit->tx_buf_max / 1024); /* convert to K byte */
	/* print fifo useage percent */
	seq_printf(s, "fifo useage : %lu%%\n",
		(unit->tx_buf_max * 100) / uart_tx_buf_size);

	/* print fifo overflow percent */
	if (unit->tx_buf_max > uart_tx_buf_size)
		seq_printf(s, "overflow : %lu%% ~ %lu%%\n",
			((unit->tx_buf_max - uart_tx_buf_size) * 100) /
			uart_tx_buf_size,
			((unit->tx_buf_max - uart_tx_buf_size) * 100) /
			uart_tx_buf_size + 1);

	seq_printf(s, "printk cnt    : %lu\n", unit->tx_buf_times);
	seq_printf(s, "printk data   : %lu\n", unit->tx_got);
	seq_printf(s, "fifo in       : %lu\n", unit->tx_queued);
	seq_printf(s, "fifo out      : %lu\n", unit->tx_out);
	seq_printf(s, "uart send     : %lu\n", unit->tx_sent);
	seq_printf(s, "tasklet run   : %lu\n", unit->tx_uart_tasklet_run);
	if (unit->tx_uart_tasklet_run)
		seq_printf(s, "p_cnt/t_run   : %lu:1\n",
			unit->tx_buf_times / unit->tx_uart_tasklet_run);

	return 0;
}

int pl011_tx_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, pl011_tx_stat_show, inode->i_private);
}

const struct file_operations pl011_tx_stat_ops = {
	.open = pl011_tx_stat_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

#ifdef CONFIG_SERIAL_AMBA_PL011_CONSOLE
static int pl011_uart_tx_work(void *arg)
{
	struct uart_tx_unit *unit = (struct uart_tx_unit *)arg;
	struct uart_amba_port *uap =
		container_of(unit, struct uart_amba_port, tx_unit);
	static char local_static_buf[PL011_TX_LOCAL_BUF_SIZE] = "";
	unsigned int out_len;
	unsigned int pos;
	unsigned int status;
	int timeout, ret;
	unsigned long flags;

	__set_current_state(TASK_RUNNING);
	while (!kthread_should_stop()) {
		/* Wait for data to actually write */
		ret = wait_event_interruptible(unit->waitqueue,
			atomic_read(&unit->tx_sig) || kthread_should_stop());
		if (ret && !atomic_read(&unit->tx_sig))
			pr_debug("%s wait event fail. sig:%d ret:%d\n",
				__func__, atomic_read(&unit->tx_sig), ret);

		if (kthread_should_stop())
			break;
print_retry:
		timeout = TX_WORK_TIMEOUT;
		unit->tx_uart_tasklet_run++;

		/* fifo out and send */
		spin_lock_irqsave(&unit->tx_lock_in, flags);
		out_len = kfifo_out(&unit->tx_fifo, local_static_buf,
			PL011_TX_LOCAL_BUF_SIZE);
		spin_unlock_irqrestore(&unit->tx_lock_in, flags);
		unit->tx_out += out_len;
		for (pos = 0; pos < out_len; pos++) {
			if ((local_static_buf[pos] == '\n') && pos &&
				(local_static_buf[pos - 1] != '\r'))
				pl011_console_putchar(&uap->port, '\r');

			pl011_console_putchar(
				&uap->port, local_static_buf[pos]);
			unit->tx_sent++;
		}
		/*
		 * Finally, wait for transmitter to become empty
		 * and restore the TCR
		 */
		do {
			status = pl011_read(uap, REG_FR);
		} while ((status & uap->vendor->fr_busy) && timeout--);

		if (timeout < 0)
			pr_err("%s: PL011 tx work timeout\n", __func__);

		if (kfifo_len(&unit->tx_fifo) > 0)
			goto print_retry;
		atomic_set(&unit->tx_sig, 0);
	}

	return 0;
}

static void pl011_unit_init(struct uart_tx_unit *unit, int cpuid)
{
	unit->tx_got = 0;
	unit->tx_uart_fifo_full = 0;
	unit->tx_uart_fifo_full_lost = 0;
	unit->tx_sent = 0;
	unit->tx_out = 0;
	unit->tx_queued = 0;
	unit->tx_valid = 1;
	unit->tx_cpuid = cpuid;
	unit->max_cpus = num_possible_cpus();
	unit->tx_buf_times = 0;
	unit->tx_buf_max = 0;
	unit->tx_buf_over_cnt = 0;
	unit->tx_buf_over_size = 0;
	unit->tx_uart_tasklet_run = 0;
}

static int pl011_tx_work_init(struct uart_amba_port *uap,
	unsigned int uart_tx_buf_size, int cpuid)
{
	struct uart_tx_unit *unit = &uap->tx_unit;
	char dbgfs_name[MAX_DBG_FS_NAME_LEN] = {'\0'};
	int ret;

	ret = kfifo_alloc(
		&unit->tx_fifo, uart_tx_buf_size, GFP_KERNEL | __GFP_ZERO);
	if (ret) {
		pr_err("%s: port:%u malloc fail\n", __func__, uap->port.line);
		unit->tx_valid = 0;
		return ret;
	}

	spin_lock_init(&unit->tx_lock_in);

	atomic_set(&unit->tx_sig, 0);
	init_waitqueue_head(&unit->waitqueue);
	unit->thread =
		kthread_create(pl011_uart_tx_work, unit, "uart console thread");
	if (IS_ERR(unit->thread)) {
		ret = PTR_ERR(unit->thread);
		pr_err("%s: Couldn't create kthread %d\n", __func__, ret);
		unit->tx_valid = 0;
		return ret;
	}

	pl011_unit_init(unit, cpuid);

	if ((unit->tx_cpuid != -1) || (unit->max_cpus >= unit->tx_cpuid))
		kthread_bind(unit->thread, unit->tx_cpuid);

	wake_up_process(unit->thread);

	ret = snprintf_s(dbgfs_name, sizeof(dbgfs_name), sizeof(dbgfs_name) - 1,
		"uart%u_stat", uap->port.line);
	if (ret < 0) {
		pr_err("%s:gen dbgfs_name fail %d\n", __func__, ret);
		return ret;
	}
#ifdef CONFIG_HISI_DEBUG_FS
	/* 0444 for file read-only */
	debugfs_create_file(dbgfs_name, 0444, NULL, unit, &pl011_tx_stat_ops);
#endif
	return 0;
}

int pl011_tx_work_uninit(struct uart_amba_port *uap)
{
	struct uart_tx_unit *unit = &uap->tx_unit;

	if (!unit->tx_valid)
		return -ENODEV;

	kthread_stop(unit->thread);
	unit->tx_valid = 0;
	unit->tx_cpuid = UART_TX_CPUON_NOTSET;
	kfifo_free(&unit->tx_fifo);
	unit->tx_got = 0;
	unit->tx_uart_fifo_full = 0;
	unit->tx_uart_fifo_full_lost = 0;
	unit->tx_queued = 0;
	unit->tx_sent = 0;
	unit->tx_out = 0;
	unit->tx_buf_times = 0;
	unit->tx_buf_max = 0;
	unit->tx_buf_over_cnt = 0;
	unit->tx_buf_over_size = 0;
	unit->tx_uart_tasklet_run = 0;

	return 0;
}
#endif

void uart_chip_reset_endisable(struct uart_amba_port *dev, unsigned int enable)
{
	unsigned int ret;
	int timeout = RESET_WAIT_TIMES;

	if (dev->reset) {
		if (enable) {
			writel(BIT(dev->reset_bit),
				dev->reset_reg_base + dev->reset_enable_off);
			do {
				ret = readl(dev->reset_reg_base +
					dev->reset_status_off);
				ret &= BIT(dev->reset_bit);
				udelay(1);
			} while (!ret && timeout--);
		} else {
			writel(BIT(dev->reset_bit),
				dev->reset_reg_base + dev->reset_disable_off);
			do {
				ret = readl(dev->reset_reg_base +
					dev->reset_status_off);
				ret &= BIT(dev->reset_bit);
				udelay(1);
			} while (ret && timeout--);
		}
	}
}

static void pl011_console_write_tx(
	struct console *co, const char *s, unsigned int count)
{
	struct uart_amba_port *uap = amba_ports[co->index];
	struct uart_tx_unit *unit = &uap->tx_unit;
	unsigned int copyed_len;
	unsigned int fifo_len;
	unsigned long flag;

	spin_lock_irqsave(&unit->tx_lock_in, flag);
	copyed_len = kfifo_in(&unit->tx_fifo, s, count);
	fifo_len = kfifo_len(&unit->tx_fifo);
	unit->tx_got += count;
	unit->tx_buf_times++;
	unit->tx_queued += copyed_len;
	if (copyed_len < count) {
		/* fifo over-flow */
		unit->tx_buf_over_cnt++;
		unit->tx_buf_over_size += (count - copyed_len);
		unit->tx_buf_max = max(unit->tx_buf_max,
			(unsigned long)(fifo_len + count - copyed_len));
	} else {
		unit->tx_buf_max =
			max(unit->tx_buf_max, (unsigned long)fifo_len);
	}
	atomic_inc(&unit->tx_sig);
	wake_up_interruptible(&unit->waitqueue);
	spin_unlock_irqrestore(&unit->tx_lock_in, flag);
}

int hisi_pl011_probe_get_clk_freq(
	struct amba_device *dev, struct uart_amba_port *uap, int i)
{
	int ret;
	u64 clk_rate = 0; /* high speed clk vaule */

	ret = of_property_read_u64(dev->dev.of_node, "clock-rate", &clk_rate);
	if (!ret && (clk_rate > 0)) {
		dev_info(&dev->dev, "%s:reset clock rate[%lld]!\n", __func__,
			clk_rate);
		/* reset the clock rate */
		ret = clk_set_rate(uap->clk, clk_rate);
		if (ret) {
			dev_err(&dev->dev,
				"%s:set clock rate:%lld fail! ret:0x%x\n",
				__func__, clk_rate, ret);
			devm_clk_put(&dev->dev, uap->clk);
		}
	} else {
		dev_info(&dev->dev,
			"UART%d not have clock-rate value in dts!\n", i);
	}

	return ret;
}

int hisi_pl011_probe_reset_func_enable(
	struct amba_device *dev, struct uart_amba_port *uap)
{
	struct device_node *node = NULL;
	int ret;
	u32 data[UAP_REG_CFG_NUM] = {0};
	u32 flag_reset_enable = 0;

	/* get uart reset base regester */
	node = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (node)
		uap->reset_reg_base = of_iomap(node, 0);

	/* read reset function */
	ret = of_property_read_u32(
		dev->dev.of_node, "reset-enable-flag", &flag_reset_enable);
	if (ret || !node) {
		uap->reset = false;
		dev_info(&dev->dev, "get reset-enable-flag fail\n");
	} else {
		if (flag_reset_enable) {
			/* get uart reset-base regester */
			ret = of_property_read_u32_array(dev->dev.of_node,
				"reset-reg-base", &data[0], ARRAY_SIZE(data));
			if (ret) {
				uap->reset_reg_base = NULL;
				uap->reset = false;
				dev_info(&dev->dev,
					"get reset-reg-base property fail\n");
			} else {
				uap->reset_reg_base = ioremap(data[1], data[3]);
			}

			/* get uart reset control regester */
			ret = of_property_read_u32_array(dev->dev.of_node,
				"reset-controller-reg", &data[0],
					ARRAY_SIZE(data));
			if (ret) {
				uap->reset = false;
				dev_info(&dev->dev,
					"get reset-controller-reg fail\n");
			} else {
				uap->reset_enable_off = data[0];
				uap->reset_disable_off = data[1];
				uap->reset_status_off = data[2];
				uap->reset_bit = data[3];

				uap->reset = uap->reset_reg_base ? true : false;
			}
		} else {
			uap->reset = false;
		}
	}

	return ret;
}

static int hisi_pl011_check_console_name(const char *amba_console_name,
	struct uart_amba_port *uap)
{
	char console_uart_name[MAX_CONSOLE_UART_NAME_LEN] = "";

	if (!get_console_name(console_uart_name, sizeof(console_uart_name)))
		if (!strncmp(console_uart_name, amba_console_name,
			MAX_CONSOLE_UART_NAME_LEN))
			console_uart_name_is_ttyAMA = 1;

	if (!console_uart_name_is_ttyAMA ||
		(get_console_index() != (int)uap->port.line))
		return -EINVAL;

	return 0;
}

static int hisi_pl011_check_fifo_status(struct amba_device *dev,
	struct uart_amba_port *uap)
{
	u32 console_fifo_size;
	int console_fifo_cpuon;

	/*
	 * console fifo enable is 1
	 * console fifo disable is 0
	 */
	if (console_fifo_enable_status == 0) {
		dev_info(&dev->dev, "cmdline disable console fifo!\n");
		return -EINVAL;
	}

	if (console_fifo_enable_status == 1) {
		uap->tx_unit.tx_valid = 1;
		console_fifo_size = SET_CONSOLE_FIFO_SIZE_DEFAULT;
		console_fifo_cpuon = SET_CONSOLE_FIFO_CPUON_DEFAULT;
		dev_info(&dev->dev, "cmdline enable console fifo!\n");
		pl011_tx_work_init(uap, console_fifo_size, console_fifo_cpuon);

		return -EINVAL;
	}

	return 0;
}

static int hisi_pl011_prepare_fifo_cfg(struct amba_device *dev,
	struct uart_amba_port *uap, u32 *console_fifo_size,
	int *console_fifo_cpuon)
{
	/* get console fifo enable flag */
	if (of_property_read_u32(dev->dev.of_node, "console-fifo-enable",
		&uap->tx_unit.tx_valid)) {
		uap->tx_unit.tx_valid = 0;
		dev_info(&dev->dev, "don't have console-fifo-enable!\n");
		return -EINVAL;
	}

	if (!uap->tx_unit.tx_valid) {
		dev_info(&dev->dev, "%s:disable serial console fifo!\n",
			__func__);
		return -EINVAL;
	}

	dev_info(&dev->dev, "enable serial console fifo!\n");
	/* get console fifo size */
	if (of_property_read_u32(dev->dev.of_node, "console-fifo-size",
			console_fifo_size)) {
		*console_fifo_size = UART_TX_BUF_SIZE_DEF;
		dev_err(&dev->dev,
			"%s:console-fifo-size not exist! set %d as default\n",
			__func__, UART_TX_BUF_SIZE_DEF);
	}
	/* get console fifo cpuon */
	if (of_property_read_u32(dev->dev.of_node,
			"console-fifo-cpuon", (u32 *)console_fifo_cpuon)) {
		*console_fifo_cpuon = UART_TX_CPUON_NOTSET;
		dev_err(&dev->dev,
			"%s:console-fifo-cpuon not exist!disable fifo on cpu\n",
			__func__);
	} else {
		dev_info(&dev->dev, "%s set fifo on cpu:%d\n",
			__func__, *console_fifo_cpuon);
	}

	return 0;
}

void hisi_pl011_probe_console_enable(struct amba_device *dev,
	struct uart_amba_port *uap, const char *amba_console_name)
{
	u32 console_fifo_size = 0;
	int console_fifo_cpuon = 0;

	if (hisi_pl011_check_console_name(amba_console_name, uap)) {
		dev_info(&dev->dev, "%s:check console name fail\n", __func__);
		return;
	}

	if (hisi_pl011_check_fifo_status(dev, uap)) {
		dev_info(&dev->dev, "%s:check fifo status fail\n", __func__);
		return;
	}

	if (hisi_pl011_prepare_fifo_cfg(dev, uap, &console_fifo_size,
		&console_fifo_cpuon)) {
		dev_info(&dev->dev, "%s:prepare fifo cfg fail\n", __func__);
		return;
	}

	if (pl011_tx_work_init(uap, console_fifo_size, console_fifo_cpuon)) {
		dev_info(&dev->dev, "%s:tx work init fail\n", __func__);
		return;
	}
}

static void pl011_pm(struct uart_port *port, unsigned int state,
	unsigned int oldstate)
{
	int retval;
	struct uart_amba_port *uap = (struct uart_amba_port *)port;

	(void)oldstate;
	switch (state) {
	case UART_PM_STATE_ON:
		retval = clk_prepare_enable(uap->clk);
		if (retval)
			dev_err(uap->port.dev,
				"%s: clk_prepare_enable failed!!\n", __func__);
		break;

	case UART_PM_STATE_OFF:
		clk_disable_unprepare(uap->clk);
		break;
	default:
		dev_err(uap->port.dev, "%s: pm state error!! state=%u\n",
			__func__, state);
		break;
	}
}

struct console *get_uart_console(void) /* add by hisi-balong */
{
	struct console *uart_console = amba_reg.cons;

	return uart_console;
}

static void hisi_pl011_console_write(
	struct console *co, const char *s, unsigned int count)
{
	if (amba_ports[co->index]->tx_unit.tx_valid)
		pl011_console_write_tx(co, s, count);
	else
		pl011_console_write(co, s, count);
}
#ifdef CONFIG_PM_SLEEP
static int hisi_pl011_suspend(struct device *dev)
{
	struct uart_amba_port *uap = dev_get_drvdata(dev);
	int ret;

	if (!uap) {
		pr_err("%s:uap is NULL\n", __func__);
		return -EINVAL;
	}

	uap->pm_op_in_progress = 1;
	dev_info(uap->port.dev, "%s: +\n", __func__);
	ret = uart_suspend_port(&amba_reg, &uap->port);
	dev_info(uap->port.dev, "%s: -\n", __func__);
	uap->pm_op_in_progress = 0;

	return ret;
}

static int hisi_pl011_resume(struct device *dev)
{
	struct uart_amba_port *uap = dev_get_drvdata(dev);
	int ret;

	if (!uap) {
		pr_err("%s:uap is NULL\n", __func__);
		return -EINVAL;
	}

	uap->pm_op_in_progress = 1;
	dev_info(uap->port.dev, "%s: +\n", __func__);
	ret = uart_resume_port(&amba_reg, &uap->port);
	dev_info(uap->port.dev, "%s: -\n", __func__);
	uap->pm_op_in_progress = 0;

	return ret;
}
#endif

static int hisi_pl011_pinctrl(
	struct amba_device *dev, struct uart_amba_port *uap)
{
	int ret = 0;

	uap->pinctrl = devm_pinctrl_get(&dev->dev);
	if (IS_ERR(uap->pinctrl)) {
		ret = PTR_ERR(uap->pinctrl);
		return ret;
	}

	uap->pins_default =
		pinctrl_lookup_state(uap->pinctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(uap->pins_default))
		dev_err(&dev->dev, "could not get default pinstate\n");

	uap->pins_idle = pinctrl_lookup_state(uap->pinctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(uap->pins_idle))
		dev_err(&dev->dev, "could not get sleep pinstate\n");

	return ret;
}

static void hisi_pl011_disable_ms(struct uart_port *port)
{
	struct uart_amba_port *uap =
		container_of(port, struct uart_amba_port, port);

	uap->im &= ~(UART011_RIMIM | UART011_CTSMIM | UART011_DCDMIM |
		UART011_DSRMIM);
	writew(uap->im, uap->port.membase + UART011_IMSC);
}
