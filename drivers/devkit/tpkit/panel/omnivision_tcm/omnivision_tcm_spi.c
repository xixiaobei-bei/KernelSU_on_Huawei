/*
 * omnivision TCM touchscreen driver

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND omnivision
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL omnivision BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF omnivision WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, omnivision'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include "omnivision_tcm_core.h"

static unsigned char *buf;

static unsigned int buf_size;

static struct spi_transfer *xfer;

static struct ovt_tcm_bus_io bus_io;

struct ovt_tcm_hw_interface hw_if;

struct platform_device *ovt_tcm_spi_platform_device;

#ifdef CONFIG_OF
static int parse_dt(struct device *dev, struct ovt_tcm_board_data *bdata)
{
	int retval;
	u32 value = 0;
	struct property *prop;
	struct device_node *np = dev->of_node;
	struct device_node *child = NULL;
	const char *name = NULL;
	const char *str_value = NULL;

	prop = of_find_property(np, "omnivision,irq-gpio", NULL);
	if (prop && prop->length) {
		bdata->irq_gpio = of_get_named_gpio_flags(np,
				"omnivision,irq-gpio", 0,
				(enum of_gpio_flags *)&bdata->irq_flags);
	} else {
		bdata->irq_gpio = -1;
	}

	retval = of_property_read_u32(np, "omnivision,irq-on-state", &value);
	if (retval < 0)
		bdata->irq_on_state = 0;
	else
		bdata->irq_on_state = value;

	retval = of_property_read_string(np, "omnivision,pwr-reg-name", &name);
	if (retval < 0)
		bdata->pwr_reg_name = NULL;
	else
		bdata->pwr_reg_name = name;

	retval = of_property_read_string(np, "omnivision,bus-reg-name", &name);
	if (retval < 0)
		bdata->bus_reg_name = NULL;
	else
		bdata->bus_reg_name = name;

	prop = of_find_property(np, "omnivision,power-gpio", NULL);
	if (prop && prop->length) {
		bdata->power_gpio = of_get_named_gpio_flags(np,
				"omnivision,power-gpio", 0, NULL);
	} else {
		bdata->power_gpio = -1;
	}

	prop = of_find_property(np, "omnivision,power-on-state", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,power-on-state",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to read omnivision,power-on-state property");
			return retval;
		} else {
			bdata->power_on_state = value;
		}
	} else {
		bdata->power_on_state = 0;
	}

	prop = of_find_property(np, "omnivision,power-delay-ms", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,power-delay-ms",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to read omnivision,power-delay-ms property");
			return retval;
		} else {
			bdata->power_delay_ms = value;
		}
	} else {
		bdata->power_delay_ms = 0;
	}

	prop = of_find_property(np, "omnivision,reset-gpio", NULL);
	if (prop && prop->length) {
		bdata->reset_gpio = of_get_named_gpio_flags(np,
				"omnivision,reset-gpio", 0, NULL);
	} else {
		bdata->reset_gpio = -1;
	}

	prop = of_find_property(np, "omnivision,reset-on-state", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,reset-on-state",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to read omnivision,reset-on-state property");
			return retval;
		} else {
			bdata->reset_on_state = value;
		}
	} else {
		bdata->reset_on_state = 0;
	}

	prop = of_find_property(np, "omnivision,reset-active-ms", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,reset-active-ms",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to read omnivision,reset-active-ms property");
			return retval;
		} else {
			bdata->reset_active_ms = value;
		}
	} else {
		bdata->reset_active_ms = 0;
	}

	prop = of_find_property(np, "omnivision,reset-delay-ms", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,reset-delay-ms",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Unable to read omnivision,reset-delay-ms property");
			return retval;
		} else {
			bdata->reset_delay_ms = value;
		}
	} else {
		bdata->reset_delay_ms = 0;
	}

	prop = of_find_property(np, "omnivision,tpio-reset-gpio", NULL);
	if (prop && prop->length) {
		bdata->tpio_reset_gpio = of_get_named_gpio_flags(np,
				"omnivision,tpio-reset-gpio", 0, NULL);
	} else {
		bdata->tpio_reset_gpio = -1;
	}

	prop = of_find_property(np, "omnivision,x-flip", NULL);
	bdata->x_flip = prop > 0 ? true : false;

	prop = of_find_property(np, "omnivision,y-flip", NULL);
	bdata->y_flip = prop > 0 ? true : false;

	prop = of_find_property(np, "omnivision,swap-axes", NULL);
	bdata->swap_axes = prop > 0 ? true : false;

	prop = of_find_property(np, "omnivision,byte-delay-us", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,byte-delay-us",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Unable to read omnivision,byte-delay-us property");
			return retval;
		} else {
			bdata->byte_delay_us = value;
		}
	} else {
		bdata->byte_delay_us = 0;
	}

	prop = of_find_property(np, "omnivision,block-delay-us", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,block-delay-us",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Unable to read omnivision,block-delay-us property");
			return retval;
		} else {
			bdata->block_delay_us = value;
		}
	} else {
		bdata->block_delay_us = 0;
	}

	prop = of_find_property(np, "omnivision,spi-mode", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,spi-mode",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Unable to read omnivision,spi-mode property");
			return retval;
		} else {
			bdata->spi_mode = value;
		}
	} else {
		bdata->spi_mode = 0;
	}

	prop = of_find_property(np, "omnivision,ubl-max-freq", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,ubl-max-freq",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Unable to read omnivision,ubl-max-freq property");
			return retval;
		} else {
			bdata->ubl_max_freq = value;
		}
	} else {
		bdata->ubl_max_freq = 0;
	}

	prop = of_find_property(np, "omnivision,ubl-byte-delay-us", NULL);
	if (prop && prop->length) {
		retval = of_property_read_u32(np, "omnivision,ubl-byte-delay-us",
				&value);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Unable to read omnivision,ubl-byte-delay-us property");
			return retval;
		} else {
			bdata->ubl_byte_delay_us = value;
		}
	} else {
		bdata->ubl_byte_delay_us = 0;
	}
	retval = of_property_read_string(np, "tp_test_type", &str_value);
	if (retval < 0) {
		OVT_LOG_ERR("get tp_test_type failed, use default: fail\n");
		str_value = "tp_test_type";
	}
	strncpy(bdata->tp_test_type, str_value, TS_CAP_TEST_TYPE_LEN - 1);
	OVT_LOG_ERR("ovt tp_test_type = %s\n", bdata->tp_test_type);
#if 0
	child = of_find_node_by_name(np, bdata->project_id);
	if (IS_ERR_OR_NULL(child)) {
		OVT_LOG_ERR("find %s node failed\n", bdata->project_id);
		return -ENODATA;
	}

	prop = of_property_read_string(child, "vendor", &str_value);
	if (prop) {
		OVT_LOG_ERR("get module_name failed, use default");
		str_value = "unknown";
	}
	strncpy(bdata->module_name, str_value, MAX_STR_LEN);

	OVT_LOG_ERR("ovt module_name = %s\n", bdata->module_name);
#endif
	return 0;
}
#endif

static int ovt_tcm_spi_alloc_mem(struct ovt_tcm_hcd *tcm_hcd,
		unsigned int count, unsigned int size)
{
	static unsigned int xfer_count;
	struct spi_device *spi = tcm_hcd->ovt_tcm_chip_data->ts_platform_data->spi;

	if (count > xfer_count) {
		kfree(xfer);
		xfer = kcalloc(count, sizeof(*xfer), GFP_KERNEL);
		if (!xfer) {
			OVT_LOG_ERR(
					"Failed to allocate memory for xfer");
			xfer_count = 0;
			return -ENOMEM;
		}
		xfer_count = count;
	} else {
		memset(xfer, 0, count * sizeof(*xfer));
	}

	if (size > buf_size) {
		if (buf_size)
			kfree(buf);
		buf = kmalloc(size, GFP_KERNEL);
		if (!buf) {
			OVT_LOG_ERR(
					"Failed to allocate memory for buf");
			buf_size = 0;
			return -ENOMEM;
		}
		buf_size = size;
	}

	return 0;
}

static int ovt_tcm_spi_rmi_read(struct ovt_tcm_hcd *tcm_hcd,
		unsigned short addr, unsigned char *data, unsigned int length)
{
	int retval;
	unsigned int idx;
	unsigned int mode;
	unsigned int byte_count;
	struct spi_message msg;
	struct spi_device *spi = tcm_hcd->ovt_tcm_chip_data->ts_platform_data->spi;
	const struct ovt_tcm_board_data *bdata = tcm_hcd->hw_if->bdata;

	mutex_lock(&tcm_hcd->io_ctrl_mutex);

	spi_message_init(&msg);

	byte_count = length + 2;

	if (bdata->ubl_byte_delay_us == 0)
		retval = ovt_tcm_spi_alloc_mem(tcm_hcd, 2, byte_count);
	else
		retval = ovt_tcm_spi_alloc_mem(tcm_hcd, byte_count, 3);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory");
		goto exit;
	}

	buf[0] = (unsigned char)(addr >> 8) | 0x80;
	buf[1] = (unsigned char)addr;

	if (bdata->ubl_byte_delay_us == 0) {
		xfer[0].len = 2;
		xfer[0].tx_buf = buf;
		xfer[0].speed_hz = bdata->ubl_max_freq;
		spi_message_add_tail(&xfer[0], &msg);
		memset(&buf[2], 0xff, length);
		xfer[1].len = length;
		xfer[1].tx_buf = &buf[2];
		xfer[1].rx_buf = data;
		if (bdata->block_delay_us)
			xfer[1].delay_usecs = bdata->block_delay_us;
		xfer[1].speed_hz = bdata->ubl_max_freq;
		spi_message_add_tail(&xfer[1], &msg);
	} else {
		buf[2] = 0xff;
		for (idx = 0; idx < byte_count; idx++) {
			xfer[idx].len = 1;
			if (idx < 2) {
				xfer[idx].tx_buf = &buf[idx];
			} else {
				xfer[idx].tx_buf = &buf[2];
				xfer[idx].rx_buf = &data[idx - 2];
			}
			xfer[idx].delay_usecs = bdata->ubl_byte_delay_us;
			if (bdata->block_delay_us && (idx == byte_count - 1))
				xfer[idx].delay_usecs = bdata->block_delay_us;
			xfer[idx].speed_hz = bdata->ubl_max_freq;
			spi_message_add_tail(&xfer[idx], &msg);
		}
	}

	mode = spi->mode;
	spi->mode = SPI_MODE_3;

	retval = spi_sync(spi, &msg);
	if (retval == 0) {
		retval = length;
	} else {
		OVT_LOG_ERR(
				"Failed to complete SPI transfer, error = %d\n",
				retval);
	}

	spi->mode = mode;

exit:
	mutex_unlock(&tcm_hcd->io_ctrl_mutex);

	return retval;
}

static int ovt_tcm_spi_rmi_write(struct ovt_tcm_hcd *tcm_hcd,
		unsigned short addr, unsigned char *data, unsigned int length)
{
	int retval;
	unsigned int mode;
	unsigned int byte_count;
	struct spi_message msg;
	struct spi_device *spi = tcm_hcd->ovt_tcm_chip_data->ts_platform_data->spi;
	const struct ovt_tcm_board_data *bdata = tcm_hcd->hw_if->bdata;

	mutex_lock(&tcm_hcd->io_ctrl_mutex);

	spi_message_init(&msg);

	byte_count = length + 2;

	retval = ovt_tcm_spi_alloc_mem(tcm_hcd, 1, byte_count);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory");
		goto exit;
	}

	buf[0] = (unsigned char)(addr >> 8) & ~0x80;
	buf[1] = (unsigned char)addr;
	retval = secure_cpydata(&buf[2],
			buf_size - 2,
			data,
			length,
			length);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy write data");
		goto exit;
	}

	xfer[0].len = byte_count;
	xfer[0].tx_buf = buf;
	if (bdata->block_delay_us)
		xfer[0].delay_usecs = bdata->block_delay_us;
	spi_message_add_tail(&xfer[0], &msg);

	mode = spi->mode;
	spi->mode = SPI_MODE_3;

	retval = spi_sync(spi, &msg);
	if (retval == 0) {
		retval = length;
	} else {
		OVT_LOG_ERR(
				"Failed to complete SPI transfer, error = %d\n",
				retval);
	}

	spi->mode = mode;

exit:
	mutex_unlock(&tcm_hcd->io_ctrl_mutex);

	return retval;
}

static int ovt_tcm_spi_read(struct ovt_tcm_hcd *tcm_hcd, unsigned char *data,
		unsigned int length)
{
	int retval;
	unsigned int idx;
	struct spi_message msg;

	struct spi_device *spi;
	const struct ovt_tcm_board_data *bdata;

	spi = tcm_hcd->ovt_tcm_chip_data->ts_platform_data->spi;
	bdata = tcm_hcd->hw_if->bdata;

	mutex_lock(&tcm_hcd->io_ctrl_mutex);

	spi_message_init(&msg);

	if (bdata->byte_delay_us == 0)
		retval = ovt_tcm_spi_alloc_mem(tcm_hcd, 1, length);
	else
		retval = ovt_tcm_spi_alloc_mem(tcm_hcd, length, 1);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory");
		goto exit;
	}

	if (bdata->byte_delay_us == 0) {
		memset(buf, 0xff, length);
		xfer[0].len = length;
		xfer[0].tx_buf = buf;
		xfer[0].rx_buf = data;
		if (bdata->block_delay_us)
			xfer[0].delay_usecs = bdata->block_delay_us;
		spi_message_add_tail(&xfer[0], &msg);
	} else {
		buf[0] = 0xff;
		for (idx = 0; idx < length; idx++) {
			xfer[idx].len = 1;
			xfer[idx].tx_buf = buf;
			xfer[idx].rx_buf = &data[idx];
			xfer[idx].delay_usecs = bdata->byte_delay_us;
			if (bdata->block_delay_us && (idx == length - 1))
				xfer[idx].delay_usecs = bdata->block_delay_us;
			spi_message_add_tail(&xfer[idx], &msg);
		}
	}

	if (tcm_hcd->use_dma_download_firmware)
		spi->controller_data = &tcm_hcd->ovt_tcm_platform_data->spidev0_chip_info;

	retval = spi_setup(spi);
	if (retval) {
		TS_LOG_ERR("%s spi setup failed, retval = %d.\n", __func__, retval);
		return retval;
	}

	retval = spi_sync(spi, &msg);
	if (retval == 0) {
		retval = length;
	} else {
		OVT_LOG_ERR(
				"Failed to complete SPI transfer, error = %d\n",
				retval);
	}

exit:
	mutex_unlock(&tcm_hcd->io_ctrl_mutex);

	return retval;
}

static int ovt_tcm_spi_write(struct ovt_tcm_hcd *tcm_hcd, unsigned char *data,
		unsigned int length)
{
	int retval;
	unsigned int idx;
	struct spi_message msg;
	struct spi_device *spi = tcm_hcd->ovt_tcm_chip_data->ts_platform_data->spi;
	const struct ovt_tcm_board_data *bdata = tcm_hcd->hw_if->bdata;

	mutex_lock(&tcm_hcd->io_ctrl_mutex);

	spi_message_init(&msg);

	if (bdata->byte_delay_us == 0)
		retval = ovt_tcm_spi_alloc_mem(tcm_hcd, 1, 0);
	else
		retval = ovt_tcm_spi_alloc_mem(tcm_hcd, length, 0);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory");
		goto exit;
	}

	if (bdata->byte_delay_us == 0) {
		xfer[0].len = length;
		xfer[0].tx_buf = data;
		if (bdata->block_delay_us)
			xfer[0].delay_usecs = bdata->block_delay_us;
		spi_message_add_tail(&xfer[0], &msg);
	} else {
		for (idx = 0; idx < length; idx++) {
			xfer[idx].len = 1;
			xfer[idx].tx_buf = &data[idx];
			xfer[idx].delay_usecs = bdata->byte_delay_us;
			if (bdata->block_delay_us && (idx == length - 1))
				xfer[idx].delay_usecs = bdata->block_delay_us;
			spi_message_add_tail(&xfer[idx], &msg);
		}
	}

	if (tcm_hcd->use_dma_download_firmware)
		spi->controller_data = &tcm_hcd->ovt_tcm_platform_data->spidev0_chip_info;

	retval = spi_setup(spi);
	if (retval) {
		TS_LOG_ERR("%s spi setup failed, retval = %d.\n", __func__, retval);
		return retval;
	}

	retval = spi_sync(spi, &msg);
	if (retval == 0) {
		retval = length;
	} else {
		OVT_LOG_ERR(
				"Failed to complete SPI transfer, error = %d\n",
				retval);
	}

exit:
	mutex_unlock(&tcm_hcd->io_ctrl_mutex);

	return retval;
}

int ovt_tcm_spi_probe(struct spi_device *spi)
{
	int retval;

	if (spi->master->flags & SPI_MASTER_HALF_DUPLEX) {
		OVT_LOG_ERR(
				"Full duplex not supported by host");
		return -EIO;
	}

	ovt_tcm_spi_platform_device = platform_device_alloc(PLATFORM_DRIVER_NAME, 0);
	if (!ovt_tcm_spi_platform_device) {
		OVT_LOG_ERR(
				"Failed to allocate platform device");
		return -ENOMEM;
	}

#ifdef CONFIG_OF
	hw_if.bdata = devm_kzalloc(&spi->dev, sizeof(*hw_if.bdata), GFP_KERNEL);
	if (!hw_if.bdata) {
		OVT_LOG_ERR(
				"Failed to allocate memory for board data");
		return -ENOMEM;
	}
	parse_dt(&spi->dev, hw_if.bdata);
#else
	hw_if.bdata = spi->dev.platform_data;
#endif

	switch (hw_if.bdata->spi_mode) {
	case 0:
		spi->mode = SPI_MODE_0;
		break;
	case 1:
		spi->mode = SPI_MODE_1;
		break;
	case 2:
		spi->mode = SPI_MODE_2;
		break;
	case 3:
		spi->mode = SPI_MODE_3;
		break;
	}

	bus_io.type = BUS_SPI;
	bus_io.read = ovt_tcm_spi_read;
	bus_io.write = ovt_tcm_spi_write;
	bus_io.rmi_read = ovt_tcm_spi_rmi_read;
	bus_io.rmi_write = ovt_tcm_spi_rmi_write;

	hw_if.bus_io = &bus_io;

	spi->bits_per_word = 8;
	dev_set_name(&(spi->dev), "ovt_tcm_spi_device_TS_KIT");
	retval = spi_setup(spi);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to set up SPI protocol driver");
		return retval;
	}

	ovt_tcm_spi_platform_device->dev.parent = &spi->dev;
	ovt_tcm_spi_platform_device->dev.platform_data = &hw_if;

	retval = platform_device_add(ovt_tcm_spi_platform_device);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to add platform device");
		return retval;
	}

	return 0;
}

