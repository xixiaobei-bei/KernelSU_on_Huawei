

#ifndef _TZ_SPI_NOTIFY_H_
#define _TZ_SPI_NOTIFY_H_

int tz_spi_init(struct device *class_dev, struct device_node *np);
void tz_spi_exit(void);
int tc_ns_tst_cmd(tc_ns_dev_file *dev_id, void *argp);
#endif
