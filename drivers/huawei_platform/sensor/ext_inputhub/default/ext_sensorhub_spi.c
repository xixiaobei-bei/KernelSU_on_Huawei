/*
 * ext_sensorhub_spi.c
 *
 * code for external sensorhub spi driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "ext_sensorhub_spi.h"

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>
#include <linux/errno.h>
#include <linux/syscalls.h>
#include <uapi/linux/sched/types.h>
#include <linux/wait.h>

#include "securec.h"
#include "ext_sensorhub_commu.h"

#define SPI_COM_MODE_POLL 1
#define SPI_COM_MODE_DMA 2
#define DEFAULT_BITS_PER_WORD 8
#define BITS_PER_WORD_16 16
#define FORCE_UPGRADE_SPI_SPEED 1000000

static struct spi_config g_spi_config;
static struct mutex spi_op_lock;
static bool g_spi_resume = true;
static unsigned int g_normal_spi_speed;

static void spi_callback(void *arg)
{
	complete(arg);
}

static int ext_sensorhub_spi_sync(u8 *buf, u32 len, struct spi_transfer *xfer)
{
	int ret;
	struct sched_param s_parm;
	struct spi_message	m;
	struct spi_device *spidev = g_spi_config.spi_dev;
	struct spi_controller *ctlr = spidev->controller;
	DECLARE_COMPLETION_ONSTACK(done);

	if (!g_spi_resume) {
		pr_err("%s spi is in suspend\n", __func__);
		return -EACCES;
	}

	s_parm.sched_priority = g_spi_config.sched_priority;
	spi_message_init(&m);
	spi_message_add_tail(xfer, &m);
	m.complete = spi_callback;
	m.context = &done;
	m.spi = spidev;
	ctlr->cur_msg = &m;

	ctlr->prepare_transfer_hardware(ctlr);
	/* hisi feature */
	ret = pl022_runtime_resume(ctlr->dev.parent);
	if (ret < 0) {
		pr_err("%s pl022_runtime_resume err: %d", __func__, ret);
		goto err;
	}
	ret = ctlr->transfer_one_message(ctlr, &m);
	wait_for_completion(&done);
	if (ret < 0) {
		pr_err("%s spi transfer_one_message err: %d", __func__, ret);
		goto err;
	}
err:
	pl022_runtime_suspend(ctlr->dev.parent);
	ctlr->unprepare_transfer_hardware(ctlr);
	return ret;
}

int ext_sensorhub_spi_write(u8 *buf, u32 len)
{
	struct spi_transfer xfer = {
		.tx_buf = buf,
		.len = len,
		.rx_buf = NULL,
	};
	struct spi_device *spidev = g_spi_config.spi_dev;
	struct spi_controller *ctlr = spidev->controller;
	u8 temp;
	int i;

	if (len % 2 == 0) {
		/* spi send from high address */
		/* bits_per_word = 16 need to exchange each two words(bytes) */
		for (i = 0; i < len - 1; i += 2) {
			temp = buf[i];
			buf[i] = buf[i + 1];
			buf[i + 1] = temp;
		}
		spidev->bits_per_word = BITS_PER_WORD_16;
	} else {
		spidev->bits_per_word = DEFAULT_BITS_PER_WORD;
	}
	/* spi write(short pkg) use polling mode to get less schedule time */
	g_spi_config.pl022_spi_config.com_mode = SPI_COM_MODE_POLL;
	spidev->max_speed_hz = g_normal_spi_speed;

	ctlr->setup(spidev);
	return ext_sensorhub_spi_sync(buf, len, &xfer);
}

int ext_sensorhub_spi_update(u8 *buf, u32 len, u32 out_len, u8 *tx_buf)
{
	struct spi_transfer xfer = {
		.tx_buf = buf,
		.len = (len + out_len),
		.rx_buf = tx_buf,
	};
	struct spi_device *spidev = g_spi_config.spi_dev;
	struct spi_controller *ctlr = spidev->controller;
	u8 temp;
	int i;

	pr_info("%s spi update len: %d, out_len: %d", __func__, len, out_len);
	if (tx_buf < NULL || len < 1)
		return -ENOMEM;

	spidev->bits_per_word = DEFAULT_BITS_PER_WORD;
	spidev->max_speed_hz = FORCE_UPGRADE_SPI_SPEED;
	g_spi_config.pl022_spi_config.com_mode = SPI_COM_MODE_DMA;

	ctlr->setup(spidev);
	return ext_sensorhub_spi_sync(buf, len, &xfer);
}


int ext_sensorhub_spi_read(u8 *buf, u32 len)
{
	int ret;
	int i;
	u8 *tx_buff = NULL;
	u8 temp;
	struct spi_transfer xfer = {
		.tx_buf = NULL,
		.len = len,
		.rx_buf = buf,
	};
	struct spi_device *spidev = g_spi_config.spi_dev;
	struct spi_controller *ctlr = spidev->controller;

	tx_buff = kmalloc((len > 2 ? len : 2), GFP_KERNEL);
	if (!tx_buff)
		return -ENOMEM;

	if (len % 2 == 0) {
		/* BITS_PER_WORD_16 out 0x7f is on the first byte */
		spidev->bits_per_word = BITS_PER_WORD_16;
		tx_buff[1] = 0x7f;
	} else {
		spidev->bits_per_word = DEFAULT_BITS_PER_WORD;
		tx_buff[0] = 0x7f;
	}
	spidev->max_speed_hz = g_normal_spi_speed;
	g_spi_config.pl022_spi_config.com_mode = SPI_COM_MODE_POLL;
	ctlr->setup(spidev);

	xfer.tx_buf = tx_buff;
	memset_s(buf, len, 0, len);
	ret = ext_sensorhub_spi_sync(buf, len, &xfer);
	if (len % 2 == 0) {
		for (i = 0; i < len - 1; i += 2) {
			temp = buf[i];
			buf[i] = buf[i + 1];
			buf[i + 1] = temp;
		}
	}
	kfree(tx_buff);
	return ret;
}

static void ext_sensorhub_spi_cs_set(u32 control)
{
	gpio_direction_output(g_spi_config.cs_gpio, control);
}

static int ext_sensorhub_spi_config(struct device_node *spi_cfg_node,
				    struct spi_config *spi_config)
{
	int rc;
	unsigned int value = 0;

	rc = of_property_read_u32(spi_cfg_node, "spi-max-frequency", &value);
	if (!rc) {
		spi_config->max_speed_hz = value;
		g_normal_spi_speed = value;
		pr_info("%s:spi-max-frequency configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-bus-id", &value);
	if (!rc) {
		spi_config->bus_id = (u8)value;
		pr_info("%s:spi-bus-id configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "spi-mode", &value);
	if (!rc) {
		spi_config->mode = value;
		pr_info("%s:spi-mode configed %d\n", __func__, value);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "bits-per-word", &value);
	if (!rc) {
		spi_config->bits_per_word = value;
		pr_info("%s:bits-per-word configed %d\n", __func__, value);
	}

	/* cs gpio */
	value = of_get_named_gpio(spi_cfg_node, "cs_gpio", 0);
	if (!gpio_is_valid(value)) {
		pr_err("%s: get cs_gpio failed\n", __func__);
		return -ENODEV;
	}
	spi_config->cs_gpio = value;
	rc = gpio_request(spi_config->cs_gpio, "spi_cs");
	if (rc) {
		pr_err("%s:gpio_request %d failed\n", __func__,
		       spi_config->cs_gpio);
		return rc;
	}
	gpio_direction_output(spi_config->cs_gpio, GPIO_HIGH);
	pr_info("%s:set cs gpio %d deault hi\n", __func__,
		spi_config->cs_gpio);

	return 0;
}

static void ext_sensorhub_pl022_config(
	struct device_node *spi_cfg_node,
	struct pl022_config_chip *pl022_spi_config)
{
	int rc;
	unsigned int value = 0;

	rc = of_property_read_u32(spi_cfg_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		pr_info("%s: pl022,interface parsed\n", __func__);
	}
	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		pr_info("%s:com_mode parsed: %d\n", __func__, value);
	}
	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		pr_info("%s:rx-level-trig parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		pr_info("%s:tx-level-trig parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		pr_info("%s:ctrl-len parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		pr_info("%s:wait-state parsed\n", __func__);
	}

	value = 0;
	rc = of_property_read_u32(spi_cfg_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		pr_info("%s:duplex parsed\n", __func__);
	}
}

static int ext_sensorhub_spi_probe(struct spi_device *spi_dev)
{
	int rc;
	struct device_node *spi_cfg_node = NULL;
	struct spi_config *spi_config = &g_spi_config;
	struct pl022_config_chip *pl022_spi_config =
	&g_spi_config.pl022_spi_config;

	pr_info("%s:get in", __func__);
	if (!spi_dev)
		return -ENODEV;

	spi_config->spi_dev = spi_dev;
	spi_cfg_node = spi_dev->dev.of_node;
	if (!spi_cfg_node) {
		pr_err("%s:ext sensorhub spi not config in dts", __func__);
		return -ENODEV;
	}

	rc = ext_sensorhub_spi_config(spi_cfg_node, spi_config);
	if (rc < 0)
		return rc;

	ext_sensorhub_pl022_config(spi_cfg_node, pl022_spi_config);

	spi_config->sched_priority = sys_sched_get_priority_max(SCHED_FIFO);
	spi_config->pl022_spi_config.cs_control = ext_sensorhub_spi_cs_set;
	spi_config->pl022_spi_config.hierarchy = SSP_MASTER;
	/* spi default config */
	if (!spi_config->max_speed_hz) {
		spi_config->max_speed_hz = SPI_SPEED_DEFAULT;
		g_normal_spi_speed = SPI_SPEED_DEFAULT;
	}
	if (!spi_config->mode)
		spi_config->mode = SPI_MODE_0;
	if (!spi_config->bits_per_word)
		spi_config->bits_per_word = DEFAULT_BITS_PER_WORD;

	spi_dev->controller_data = &spi_config->pl022_spi_config;
	spi_dev->mode = spi_config->mode;
	spi_dev->max_speed_hz = spi_config->max_speed_hz;
	spi_dev->bits_per_word = spi_config->bits_per_word;

	rc = spi_setup(spi_dev);
	if (rc) {
		pr_err("%s: spi setup fail\n", __func__);
		return rc;
	}

	return 0;
}

static int ext_sensorhub_spi_remove(struct spi_device *spi_dev)
{
	/* do noting */
	pr_info("%s get in", __func__);
	return 0;
}

static const struct of_device_id g_psoc_match_table[] = {
	{ .compatible = "huawei,ext_sensorhub", },
	{ },
};

static const struct spi_device_id g_device_id[] = {
	{ EXT_SENSORHUB_SPI_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, g_device_id);

static int ext_sensorhub_spi_suspend(struct device *dev)
{
	pr_info("%s get in", __func__);
	g_spi_resume = false;
	commu_suspend(SPI_DRIVER);
	return 0;
}

static int ext_sensorhub_spi_resume(struct device *dev)
{
	pr_info("%s get in", __func__);
	g_spi_resume = true;
	commu_resume(SPI_DRIVER);

	return 0;
}

static const struct dev_pm_ops hisi_spi_dev_pm_ops = {
	.suspend = ext_sensorhub_spi_suspend,
	.resume = ext_sensorhub_spi_resume,
};

static struct spi_driver g_spi_driver = {
	.probe = ext_sensorhub_spi_probe,
	.remove = ext_sensorhub_spi_remove,
	.id_table = g_device_id,
	.driver = {
		.name = EXT_SENSORHUB_SPI_DEVICE_NAME,
		.owner = THIS_MODULE,
		.bus = &spi_bus_type,
		.of_match_table = g_psoc_match_table,
		.pm = &hisi_spi_dev_pm_ops,
	},
};

int ext_sensorhub_spi_init(void)
{
	int rc;

	mutex_init(&spi_op_lock);
	pr_info("%s get in", __func__);
	rc = spi_register_driver(&g_spi_driver);
	if (rc < 0) {
		pr_err("%s register spi driver error: %d", __func__, rc);
		return rc;
	}

	pr_info("%s success", __func__);
	return 0;
}

void ext_sensorhub_spi_exit(void)
{
	pr_info("%s get in", __func__);
	spi_unregister_driver(&g_spi_driver);
}
