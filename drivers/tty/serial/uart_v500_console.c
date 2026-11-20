/*
 * Console Driver for UART serial ports
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
#include "uart_v500_console.h"
static int console_fifo_cmd_status = -1;

static int __init console_fifo_cmd_setup(char *str)
{
	char buffer[2] = {0}; /* 2 for "1 or 0" */
	size_t index;

	if (strncpy_s(buffer, sizeof(buffer), str, sizeof(buffer) - 1)) {
		pr_err("%s: strncpy_s failed\n", __func__);
		return 0;
	}
	/*
	 * console fifo enable is 1 disable is 0
	 */
	for (index = 0; index < sizeof(buffer); index++) {
		if (buffer[index] == '1') {
			console_fifo_cmd_status = 1;
			break;
		}
		if (buffer[index] == '0' || buffer[index] == ' ') {
			console_fifo_cmd_status = 0;
			break;
		}
	}

	return 1;
}
__setup("console_fifo_status=", console_fifo_cmd_setup);

static int uart_v500_async_print_stat_show(struct seq_file *s, void *arg)
{
	unsigned int uart_async_print_buf_size;
	struct uart_async_print_unit *unit =
		(struct uart_async_print_unit *)s->private;

	uart_async_print_buf_size = kfifo_size(&unit->async_print_fifo);
	seq_puts(s, "console transfer statics:\n");
	seq_printf(s, "fifo over cnt : %lu\n", unit->async_print_over_cnt);
	seq_printf(s, "fifo over len : %lu\n", unit->async_print_over_size);
	seq_printf(s, "fifo capacity : 0x%lx(%luk)\n",
		(unsigned long)uart_async_print_buf_size,
		(unsigned long)uart_async_print_buf_size / 1024);
		/* convert to Kbyte */
	seq_printf(s, "fifo required : 0x%lx(%luk)\n",
		unit->async_print_buf_max, unit->async_print_buf_max / 1024);
	/* print fifo useage percent */
	seq_printf(s, "fifo useage : %lu%%\n", (unit->async_print_buf_max *
		100) / uart_async_print_buf_size);

	/* print fifo overflow percent */
	if (unit->async_print_buf_max > uart_async_print_buf_size)
		seq_printf(s, "overflow : %lu%% ~ %lu%%\n",
			((unit->async_print_buf_max -
			uart_async_print_buf_size) * 100) /
			uart_async_print_buf_size, ((unit->async_print_buf_max -
			uart_async_print_buf_size) * 100) /
			uart_async_print_buf_size + 1);

	seq_printf(s, "printk cnt    : %lu\n", unit->async_print_buf_times);
	seq_printf(s, "printk data   : %lu\n", unit->async_print_got);
	seq_printf(s, "async fifo in : %lu\n", unit->async_print_queued);
	seq_printf(s, "async fifo out: %lu\n", unit->async_print_out);
	seq_printf(s, "uart send     : %lu\n", unit->async_print_sent);
	seq_printf(s, "tasklet run   : %lu\n", unit->async_print_tasklet_run);
	if (unit->async_print_tasklet_run)
		seq_printf(s, "p_cnt/t_run   : %lu:1\n",
			unit->async_print_buf_times /
			unit->async_print_tasklet_run);

	return 0;
}

int uart_v500_async_print_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, uart_v500_async_print_stat_show,
		inode->i_private);
}

const struct file_operations uart_v500_async_print_stat_ops = {
	.open = uart_v500_async_print_stat_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

#ifdef CONFIG_SERIAL_UART_V500_CONSOLE
static int uart_v500_async_print_work(void *arg)
{
	struct uart_async_print_unit *unit =
		(struct uart_async_print_unit *)arg;
	struct uart_v500_port *uap = container_of(unit, struct uart_v500_port,
		async_print_unit);
	static char local_static_buf[UART_ASYNC_PRINT_LOCAL_BUF_SIZE] = "";
	unsigned int out_len, pos, status;
	int timeout, temp;
	unsigned long flags;

	__set_current_state(TASK_RUNNING);
	while (!kthread_should_stop()) {
		/* Wait for data to actually write */
		/*lint -e578 */
		temp = wait_event_interruptible(unit->waitqueue,
			atomic_read(&unit->async_print_sig) ||
			kthread_should_stop());
		/*lint +e578 */
		if (temp && !atomic_read(&unit->async_print_sig))
			pr_debug("%s wait event fail. sig: %d temp:%d\n",
				__func__, atomic_read(&unit->async_print_sig),
				temp);

		if (kthread_should_stop())
			break;
async_print_retry:
		timeout = UART_ASYNC_PRINT_WORK_TIMEOUT;
		unit->async_print_tasklet_run++;

		/* fifo out and send */
		spin_lock_irqsave(&unit->async_print_lock_in, flags);
		out_len = kfifo_out(&unit->async_print_fifo, local_static_buf,
			UART_ASYNC_PRINT_LOCAL_BUF_SIZE);
		spin_unlock_irqrestore(&unit->async_print_lock_in, flags);
		unit->async_print_out += out_len;
		for (pos = 0; pos < out_len; pos++) {
			if ((local_static_buf[pos] == '\n') && pos &&
				(local_static_buf[pos - 1] != '\r'))
				uart_v500_console_putchar(&uap->port, '\r');

			uart_v500_console_putchar(&uap->port, local_static_buf[pos]);
			unit->async_print_sent++;
		}
		/*
		 * Finally, wait for transmitter to become empty
		 */
		do {
			status = uart_v500_read(uap, REG_UART_STAT);
			timeout--;
		} while ((status & uap->vendor->stat_busy) && timeout);

		if (timeout < 0)
			pr_err("%s: uart async print work timeout\n", __func__);

		if (kfifo_len(&unit->async_print_fifo) > 0)
			goto async_print_retry;
		atomic_set(&unit->async_print_sig, 0);
	}

	return 0;
}

static void uart_v500_async_unit_init(struct uart_async_print_unit *unit,
	int cpuid)
{
	unit->async_print_got = 0;
	unit->async_print_sent = 0;
	unit->async_print_out = 0;
	unit->async_print_queued = 0;
	unit->async_valid = 1;
	unit->async_print_cpuid = cpuid;
	unit->max_cpus = num_possible_cpus();
	unit->async_print_buf_times = 0;
	unit->async_print_buf_max = 0;
	unit->async_print_over_cnt = 0;
	unit->async_print_over_size = 0;
	unit->async_print_tasklet_run = 0;
}

static int uart_v500_async_print_work_init(struct uart_v500_port *uap,
	unsigned int uart_async_print_buf_size, int cpu_id)
{
	struct uart_async_print_unit *unit = &uap->async_print_unit;
	char debug_fs_name[MAX_DEBUG_FS_NAME_LEN] = {'\0'};
	int ret;

	ret = kfifo_alloc(&unit->async_print_fifo, uart_async_print_buf_size,
		GFP_KERNEL | __GFP_ZERO);
	if (ret) {
		pr_err("%s: port:%u malloc fail\n", __func__, uap->port.line);
		unit->async_valid = 0;
		return ret;
	}

	spin_lock_init(&unit->async_print_lock_in);

	atomic_set(&unit->async_print_sig, 0);
	init_waitqueue_head(&unit->waitqueue);
	unit->thread = kthread_create(uart_v500_async_print_work, unit,
		"uart console thread");
	if (IS_ERR(unit->thread)) {
		ret = PTR_ERR(unit->thread);
		pr_err("%s: Couldn't create kthread %d\n", __func__, ret);
		unit->async_valid = 0;
		return ret;
	}
	uart_v500_async_unit_init(unit, cpu_id);
	if ((unit->async_print_cpuid != -1) || (unit->max_cpus >=
		unit->async_print_cpuid))
		kthread_bind(unit->thread, unit->async_print_cpuid);

	wake_up_process(unit->thread);

	ret = snprintf_s(debug_fs_name, sizeof(debug_fs_name),
		sizeof(debug_fs_name) - 1, "uart%u_stat", uap->port.line);
	if (ret < 0) {
		pr_err("%s:gen debug_fs_name fail %d\n", __func__, ret);
		return ret;
	}
#ifdef CONFIG_HISI_DEBUG_FS
	/* 0444 for file read-only */
	debugfs_create_file(debug_fs_name, 0444, NULL, unit,
		&uart_v500_async_print_stat_ops);
#endif
	return 0;
}

int uart_v500_async_print_work_uninit(struct uart_v500_port *uap)
{
	struct uart_async_print_unit *unit = &uap->async_print_unit;

	if (!unit->async_valid)
		return -ENODEV;

	kthread_stop(unit->thread);
	unit->async_valid = 0;
	unit->async_print_cpuid = UART_ASYNC_PRINT_CPUON_NOTSET;
	kfifo_free(&unit->async_print_fifo);
	unit->async_print_got = 0;
	unit->async_print_queued = 0;
	unit->async_print_sent = 0;
	unit->async_print_out = 0;
	unit->async_print_buf_times = 0;
	unit->async_print_buf_max = 0;
	unit->async_print_over_cnt = 0;
	unit->async_print_over_size = 0;
	unit->async_print_tasklet_run = 0;

	return 0;
}
#endif

static int uart_v500_check_console_name(const char *console_name,
	struct uart_v500_port *uap)
{
	char console_uart_name[MAX_UART_CONSOLE_NAME_LEN] = "";

	if (!get_console_name(console_uart_name, sizeof(console_uart_name)))
		if (!strncmp(console_uart_name, console_name,
			MAX_UART_CONSOLE_NAME_LEN))
			console_uart_name_is_ttyhw = 1;

	if (!console_uart_name_is_ttyhw ||
		(get_console_index() != (int)uap->port.line))
		return -EINVAL;

	return 0;
}

static int uart_v500_check_async_print_status(struct platform_device *dev,
	struct uart_v500_port *uap)
{
	unsigned int fifo_size;
	int fifo_cpuon;

	/*
	 * console fifo enable is 1 disable is 0
	 */
	if (console_fifo_cmd_status == 0) {
		dev_info(&dev->dev, "cmdline disable console fifo!\n");
		return -EINVAL;
	}

	if (console_fifo_cmd_status == 1) {
		uap->async_print_unit.async_valid = 1;
		fifo_size = CONSOLE_FIFO_SIZE_DEFAULT;
		fifo_cpuon = CONSOLE_FIFO_SIZE_DEFAULT;
		dev_info(&dev->dev, "cmdline enable console fifo!\n");
		uart_v500_async_print_work_init(uap, fifo_size, fifo_cpuon);
		return -EINVAL;
	}

	return 0;
}

static int uart_v500_prepare_async_print_cfg(struct platform_device *dev,
	struct uart_v500_port *uap, unsigned int *fifo_size, int *fifo_cpuon)
{
	/* get console fifo enable flag */
	if (of_property_read_u32(dev->dev.of_node, "console-fifo-enable",
		&uap->async_print_unit.async_valid)) {
		uap->async_print_unit.async_valid = 0;
		dev_info(&dev->dev, "don't have console-fifo-enable!\n");
		return -EINVAL;
	}

	if (!uap->async_print_unit.async_valid) {
		dev_info(&dev->dev, "%s:disable serial console fifo!\n",
			__func__);
		return -EINVAL;
	}

	dev_info(&dev->dev, "enable serial console fifo!\n");
	/* get console fifo size */
	if (of_property_read_u32(dev->dev.of_node, "console-fifo-size",
		fifo_size)) {
		*fifo_size = UART_ASYNC_PRINT_BUF_SIZE_DEF;
		dev_err(&dev->dev, "%s:fifo-size not exist set %d as default\n",
			__func__, UART_ASYNC_PRINT_BUF_SIZE_DEF);
	}
	/* get console fifo cpuon */
	if (of_property_read_u32(dev->dev.of_node, "console-fifo-cpuon",
		(unsigned int *)fifo_cpuon)) {
		*fifo_cpuon = UART_ASYNC_PRINT_CPUON_NOTSET;
		dev_err(&dev->dev,
			"%s:console-fifo-cpuon not exist!disable fifo on cpu\n",
			__func__);
	} else {
		dev_info(&dev->dev, "%s set fifo on cpu:%d\n", __func__,
			*fifo_cpuon);
	}

	return 0;
}

void uart_v500_probe_console_enable(struct platform_device *dev,
	struct uart_v500_port *uap, const char *console_name)
{
	unsigned int fifo_size = 0;
	int fifo_cpuon = 0;

	if (uart_v500_check_console_name(console_name, uap)) {
		dev_info(&dev->dev, "%s uart%d not register console\n",
			__func__, uap->port.line);
		return;
	}

	if (uart_v500_check_async_print_status(dev, uap)) {
		dev_info(&dev->dev, "%s:check fifo status fail\n", __func__);
		return;
	}

	if (uart_v500_prepare_async_print_cfg(dev, uap, &fifo_size,
		&fifo_cpuon)) {
		dev_info(&dev->dev, "%s:prepare fifo cfg fail\n", __func__);
		return;
	}

	if (uart_v500_async_print_work_init(uap, fifo_size, fifo_cpuon)) {
		dev_info(&dev->dev, "%s:tx work init fail\n", __func__);
		return;
	}
}

void uart_chip_reset_endisable(struct uart_v500_port *dev, unsigned int enable)
{
	unsigned int ret;
	int timeout = UART_RESET_WAIT_TIME;

	if (dev->reset_val) {
		if (enable) {
			writel(BIT(dev->reset_bit), dev->reset_base +
				dev->reset_enable);
			do {
				ret = readl(dev->reset_base +
					dev->reset_status);
				ret &= BIT(dev->reset_bit);
				udelay(1);
			} while (!ret && timeout--);
		} else {
			writel(BIT(dev->reset_bit), dev->reset_base +
				dev->reset_disable);
			do {
				ret = readl(dev->reset_base +
					dev->reset_status);
				ret &= BIT(dev->reset_bit);
				udelay(1);
			} while (ret && timeout--);
		}
	}
}

void uart_v500_console_write_tx(struct console *co, const char *s,
	unsigned int count)
{
	struct uart_v500_port *uap = get_uart_ports_uap((int)co->index);
	struct uart_async_print_unit *unit = &uap->async_print_unit;
	unsigned long flag;
	unsigned int copyed_len;
	unsigned int fifo_len;

	spin_lock_irqsave(&unit->async_print_lock_in, flag);
	copyed_len = kfifo_in(&unit->async_print_fifo, s, count);
	fifo_len = kfifo_len(&unit->async_print_fifo);
	unit->async_print_got += count;
	unit->async_print_buf_times++;
	unit->async_print_queued += copyed_len;
	if (copyed_len < count) {
		/* fifo over-flow */
		unit->async_print_over_cnt++;
		unit->async_print_over_size += (count - copyed_len);
		unit->async_print_buf_max = max(unit->async_print_buf_max,
			(unsigned long)(fifo_len + count - copyed_len));
	} else {
		unit->async_print_buf_max = max(unit->async_print_buf_max,
			(unsigned long)fifo_len);
	}
	atomic_inc(&unit->async_print_sig);
	wake_up_interruptible(&unit->waitqueue);
	spin_unlock_irqrestore(&unit->async_print_lock_in, flag);
}

#ifdef CONFIG_SERIAL_UART_V500_CONSOLE
void uart_v500_console_putchar(struct uart_port *port, int data)
{
	struct uart_v500_port *uap = container_of(port, struct uart_v500_port,
		port);
	unsigned int count = 0;

	while ((uart_v500_read(uap, REG_UART_STAT) & UART_V500_STAT_TXFF) &&
		(count < TXFF_TIMEOUT)) {
		udelay(1);
		barrier();
		count++;
	}

	uart_v500_write(data, uap, REG_UART_TX_FIFO);
}

static void __init uart_v500_console_get_options(struct uart_v500_port *uap,
	int *baud, int *parity, int *bits)
{
	unsigned int frame_cfg, ibrd, fbrd;

	if (uart_v500_read(uap, REG_UART_CONFIG) & UART_V500_CFG_UART_EN) {
		frame_cfg = uart_v500_read(uap, REG_UART_FRAME);
		*parity = 'n';
		if ((frame_cfg & UART_V500_FRAME_PARITY) == UART_V500_FRAME_PES)
			*parity = 'e';
		else if ((frame_cfg & UART_V500_FRAME_PARITY) ==
			UART_V500_FRAME_POS)
			*parity = 'o';

		if ((frame_cfg & UART_FRAME_DATA_BIT) == UART_V500_FRAME_LEN_7)
			*bits = 7; /* set data 7 bit */
		else
			*bits = 8; /* set data 8 bit */

		ibrd = uart_v500_read(uap, REG_UART_BAUD) & UART_BAUD_INT_MASK;
		fbrd = (uart_v500_read(uap, REG_UART_BAUD) >> 16) &
			UART_BAUD_FRAC_MASK;
		/*lint -e414 */
		/*
		 * Set baud rate
		 *
		 * div_int = UART_CLK / (16 * BAUD_RATE)
		 * div_frac = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) /
		 *           (16 * BAUD_RATE))
		 */
		if (ibrd || fbrd)
			*baud = uap->port.uartclk * 4 / (64 * ibrd + fbrd);
		/*lint +e414 */
	}
}

int __init uart_v500_console_setup(struct console *co, char *options)
{
	struct uart_v500_port *uap = NULL;
	int baud = 38400; /* default baud */
	int bits = 8; /* default data bits */
	int parity = 'n';
	int flow = 'n';
	int ret;

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index >= MAX_UART_IDX)
		co->index = 0;

	uap = get_uart_ports_uap((int)co->index);
	if (!uap)
		return -ENODEV;
	/* Allow pins to be muxed in and configured */
	if (!IS_ERR(uap->pinctrls_default)) {
		ret = pinctrl_select_state(uap->pinctrl, uap->pinctrls_idle);
		if (ret)
			dev_err(uap->port.dev,
				"could not set default pins,ret:%d\n", ret);

		ret = pinctrl_select_state(uap->pinctrl, uap->pinctrls_default);
		if (ret) {
			dev_err(uap->port.dev, "could not set default pins\n");
			return ret;
		}
	}

	ret = clk_prepare(uap->clk);
	if (ret)
		return ret;

	uart_chip_reset_endisable(uap, 1);
	uart_chip_reset_endisable(uap, 0);

	if (dev_get_platdata(uap->port.dev)) {
		struct uart_v500_dma_data *plat;

		plat = dev_get_platdata(uap->port.dev);
		if (plat->init)
			plat->init();
	}

	uap->port.uartclk = clk_get_rate(uap->clk);

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		uart_v500_console_get_options(uap, &baud, &parity, &bits);

	ret = uart_set_options(&uap->port, co, baud, parity, bits, flow);

	return ret;
}

static void uart_v500_putc(struct uart_port *port, int c)
{
	while (readl(port->membase + UART_STAT) & UART_V500_STAT_TXFF)
		cpu_relax();
	if (port->iotype == UPIO_MEM32)
		writel(c, port->membase + UART_TX_FIFO);
	else
		writeb(c, port->membase + UART_TX_FIFO);

	while (readl(port->membase + UART_STAT) & UART_V500_STAT_TX_BUSY)
		cpu_relax();
}

static void uart_v500_early_write(struct console *con,
			const char *s, unsigned n)
{
	struct earlycon_device *dev = con->data;

	uart_console_write(&dev->port, s, n, uart_v500_putc);
}

/*
 * On non-ACPI systems, earlycon is enabled by specifying
 * "earlycon=uart_v500,<address>" on the kernel command line.
 *
 * To get the traditional early console that starts before ACPI is parsed,
 * specify the full "earlycon=uart_v500,<address>" option.
 */
static int __init uart_v500_early_console_setup(struct earlycon_device *device,
	const char *opt)
{
	if (!device->port.membase)
		return -ENODEV;

	device->con->write = uart_v500_early_write;

	return 0;
}
OF_EARLYCON_DECLARE(uart_v500, "hisilicon,uart", uart_v500_early_console_setup);
#endif

int uart_v500_probe_reset_func_enable(struct platform_device *dev,
	struct uart_v500_port *uap)
{
	struct device_node *node = NULL;
	int ret;
	unsigned int data[UAP_REG_CFG_NUM] = {0};
	unsigned int flag_reset_enable = 0;

	/* get uart reset base regester */
	node = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (node)
		uap->reset_base = of_iomap(node, 0);

	/* read reset function */
	ret = of_property_read_u32(dev->dev.of_node, "reset-enable-flag",
		&flag_reset_enable);

	if (ret || !node) {
		uap->reset_val = false;
		dev_info(&dev->dev, "get reset-enable-flag fail\n");
	} else {
		if (flag_reset_enable) {
			/* get uart reset-base regester */
			ret = of_property_read_u32_array(dev->dev.of_node,
				"reset-reg-base", &data[0], ARRAY_SIZE(data));
			if (ret) {
				uap->reset_base = NULL;
				uap->reset_val = false;
				dev_info(&dev->dev,
					"get reset-reg-base property fail\n");
			} else {
				/*lint -e446 */
				uap->reset_base = ioremap(data[1], data[3]);
				/*lint +e446 */
			}

			/* get uart reset control regester */
			ret = of_property_read_u32_array(dev->dev.of_node,
				"reset-controller-reg", &data[0],
				ARRAY_SIZE(data));
			if (ret) {
				uap->reset_val = false;
				dev_info(&dev->dev,
					"get reset-controller-reg fail\n");
			} else {
				uap->reset_enable = data[0];
				uap->reset_disable = data[1];
				uap->reset_status = data[2];
				uap->reset_bit = data[3];
				uap->reset_val = uap->reset_base ? true : false;
			}
		} else {
			uap->reset_val = false;
		}
	}

	return ret;
}

struct console *get_uart_console(void)
{
	struct console *uart_console = g_uart_v500.cons;

	return uart_console;
}

static void uart_v500_console_sync_write(struct console *co, const char *s,
	unsigned int count)
{
	struct uart_v500_port *uap = get_uart_ports_uap((int)co->index);
	unsigned int old_cfg = 0;
	unsigned int new_cfg = 0;
	unsigned long flags;
	int locked = 1;

	if (clk_enable(uap->clk)) {
		printk(KERN_ERR "%s Can't enable clock\n", __func__);
		return;
	}

	local_irq_save(flags);
	if (uap->port.sysrq)
		locked = 0;
	else if (oops_in_progress)
		locked = spin_trylock(&uap->port.lock);
	else
		spin_lock(&uap->port.lock);

	/*
	 * First save the CR then disable the interrupts
	 */

	old_cfg = uart_v500_read(uap, REG_UART_CONFIG);
	old_cfg |= (old_cfg << UART_CONFIG_BIT_MASK);
	new_cfg = old_cfg | UART_V500_CFG_UART_EN | UART_V500_CFG_TX_EN;
	uart_v500_write(new_cfg, uap, REG_UART_CONFIG);
	uart_v500_write(UART_V500_CFG_CTS_DIS, uap, REG_UART_CONFIG);
	uart_console_write(&uap->port, s, count, uart_v500_console_putchar);

	/*
	 * Finally, wait for transmitter to become empty and restore the
	 * TCR. Allow feature register bits to be inverted to work around
	 * errata.
	 */
	while (uart_v500_read(uap, REG_UART_STAT) & uap->vendor->stat_busy)
		cpu_relax();
	uart_v500_write(old_cfg, uap, REG_UART_CONFIG);

	if (locked)
		spin_unlock(&uap->port.lock);
	local_irq_restore(flags);

	clk_disable(uap->clk);
}

void uart_v500_console_write(struct console *co, const char *s,
	unsigned int count)
{
	struct uart_v500_port *uart_ports = NULL;

	uart_ports = get_uart_ports_uap((int)co->index);
	if (uart_ports->async_print_unit.async_valid)
		uart_v500_console_write_tx(co, s, count);
	else
		uart_v500_console_sync_write(co, s, count);
}

int uart_probe_dt_alias(int index, struct device *dev)
{
	struct device_node *np = NULL;
	static bool seen_dev_alias;
	static bool seen_dev_no_alias;
	int val = index;

	if (!IS_ENABLED(CONFIG_OF))
		return val;

	np = dev->of_node;
	if (!np)
		return val;

	val = of_alias_get_id(np, "serial");
	if (val < 0) {
		seen_dev_no_alias = true;
		val = index;
	} else {
		seen_dev_alias = true;
		if (val >= MAX_UART_IDX || get_uart_ports_uap(val) != NULL) {
			dev_warn(dev,
				"requested serial port %d not available\n",
				val);
			val = index;
		}
	}

	if (seen_dev_alias && seen_dev_no_alias)
		dev_warn(dev, "aliased and non-aliased serial devices found in "
			"device tree. Serial port may be unpredictable\n");

	return val;
}

int uart_v500_find_free_port(void)
{
	int i;
	struct uart_v500_port *uart_ports = NULL;

	for (i = 0; i < MAX_UART_IDX; i++) {
		uart_ports = get_uart_ports_uap(i);
		if (uart_ports == NULL)
			return i;
	}
	return -EBUSY;
}
