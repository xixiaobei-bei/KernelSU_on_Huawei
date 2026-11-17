/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#ifndef __SLIMBUS_CSMI_H__
#define __SLIMBUS_CSMI_H__

#include <linux/types.h>
#include "csmi.h"
#include "slimbus_types.h"
#include "slimbus_debug.h"

/* address of soc generic device */
#define SOC_EA_GENERIC_DEVICE						0xaaaac1000200
#define SOC_LA_GENERIC_DEVICE						0xD0

/* address of da_combine generic device */
#define DA_COMBINE_EA_GENERIC_DEVICE					0xaaaabbbb0601
#define DA_COMBINE_LA_GENERIC_DEVICE					0xD1

/* framer id */
#define SLIMBUS_FRAMER_SOC_ID						0
#define SLIMBUS_FRAMER_DA_COMBINE_ID					1
#define SLIMBUS_LOSTMS_COUNT						5

extern int slimbus_dev_init(enum platform_type platform_type);

extern int slimbus_drv_init(enum platform_type platform_type, const void *slimbus_reg, const void *asp_reg, const void *sctrl_reg, int irq);

extern int slimbus_drv_stop(void);

extern int slimbus_drv_release(int irq);

extern int slimbus_drv_bus_configure(const struct slimbus_bus_config *bus_config);

extern int slimbus_drv_switch_framer(uint8_t laif, uint16_t nco, uint16_t nci, const struct slimbus_bus_config *bus_config);

extern int slimbus_drv_element_read(uint8_t target_la, uint16_t address, enum slimbus_slice_size slice_size, void *value_read);

extern int slimbus_drv_element_write(uint8_t target_la, uint16_t address, enum slimbus_slice_size slice_size, void *value_write);

extern int slimbus_drv_request_info(uint8_t target_la, uint16_t address, enum slimbus_slice_size slice_size, void *value_read);

extern int slimbus_drv_track_activate(const struct slimbus_channel_property *channel, uint32_t ch_num);

extern int slimbus_drv_track_deactivate(const struct slimbus_channel_property *channel, uint32_t ch_num);

extern uint8_t slimbus_drv_get_framerla(int framer_id);

extern int slimbus_drv_pause_clock(enum slimbus_restart_time time);

extern int slimbus_drv_resume_clock(void);

extern int slimbus_drv_reset_bus(void);

extern int slimbus_drv_shutdown_bus(void);

extern volatile bool slimbus_int_need_clear_get(void);

extern void slimbus_int_need_clear_set(volatile bool flag);

extern int slimbus_drv_track_update(int cg, int sm, int track, struct slimbus_device_info *dev, uint32_t ch_num, struct slimbus_channel_property *channel);

extern void slimbus_drv_get_params_la(int track_type, uint8_t *source_la, uint8_t *sink_la, enum slimbus_transport_protocol *tp);

extern int64_t get_timeus(void);

extern spinlock_t *slimbus_drv_get_spinlock(void);

extern int slimbus_wait_codec_device_enum(void);

#endif /* __SLIMBUS_CSMI_H__ */
