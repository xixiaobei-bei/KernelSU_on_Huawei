/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description:bus_interface.h
 * Create:2019.11.11
 */

#ifndef _BUS_INTERFACE_H_
#define _BUS_INTERFACE_H_
#include <linux/hisi/contexthub/protocol_as.h>
struct sensor_combo_cfg {
	uint8_t bus_type;
	uint8_t bus_num;
	uint8_t disable_sample_thread;
	union {
		uint32_t data;
		uint32_t i2c_address;
		union spi_ctrl ctrl;
	};
} __packed;

/*
 * Function    : mcu_i2c_rw
 * Description : i2c rw
 * Input       : [bus_num] bus number
 *               [i2c_add] i2c address
 *               [tx] register address
 *               [tx_len] register address len
 *               [rx_out] output value(chip id)
 *               [rx_len] output value len
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int mcu_i2c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len,
	uint8_t *rx_out, uint32_t rx_len);

/*
 * Function    : mcu_i3c_rw
 * Description : i3c rw
 * Input       : [bus_num] bus number
 *               [i2c_add] i3c address
 *               [tx] register address
 *               [tx_len] register address len
 *               [rx_out] output value(chip id)
 *               [rx_len] output value len
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int mcu_i3c_rw(uint8_t bus_num, uint8_t i2c_add, uint8_t *tx, uint32_t tx_len,
	uint8_t *rx_out, uint32_t rx_len);


/*
 * Function    : get_combo_bus_tag
 * Description : according string of bus, get tag
 * Input       : [bus] bus type string
 *             : [tag] output tag
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int get_combo_bus_tag(const char *bus, uint8_t *tag);

/*
 * Function    : combo_bus_trans
 * Description : bus packet send to mcu
 * Input       : [p_cfg] bus info
 *             : [tx] regiseter address
 *             : [tx_len] register address len
 *             : [rx_out] output value(chip id)
 *             : [rx_len] output value len
 * Output      : none
 * Return      : 0 is ok, otherwise failed
 */
int combo_bus_trans(struct sensor_combo_cfg *p_cfg, uint8_t *tx,
		uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);


int mcu_spi_rw(uint8_t bus_num, union spi_ctrl ctrl, uint8_t *tx,
		uint32_t tx_len, uint8_t *rx_out, uint32_t rx_len);
#endif /* _BUS_INTERFACE_H_ */
