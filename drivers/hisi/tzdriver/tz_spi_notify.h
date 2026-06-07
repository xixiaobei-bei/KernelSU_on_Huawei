

#ifndef _TZ_SPI_NOTIFY_H_
#define _TZ_SPI_NOTIFY_H_
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include "teek_ns_client.h"

int tz_spi_init(struct device *class_dev, struct device_node *np);
void tz_spi_exit(void);
int tc_ns_tst_cmd(tc_ns_dev_file *dev_id, void *argp);
#endif
