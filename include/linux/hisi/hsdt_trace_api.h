/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: HSDT Trace driver interface for Kirin SoCs
 * Author: Hisilicon
 * Create: 2020-5-13
 */
#ifndef _HSDT_TRACE_API_H
#define _HSDT_TRACE_API_H

#ifdef CONFIG_HSDTTRACE
int hsdt_trace_trg_cmd(u32 bus_type, u32 trg_cmd);
int hsdt_trace_trg_event(u32 bus_type, u32 trg_event);
int hsdt_trace_trg_data1(u32 bus_type, u32 trg_data_low, u32 trg_data_high);
int hsdt_trace_trg_data2(u32 bus_type, u32 trg_data_low, u32 trg_data_high);
int hsdt_trace_trg_data1_mask(u32 bus_type, u32 trg_data_mask_l, u32 trg_data_mask_h);
int hsdt_trace_trg_data2_mask(u32 bus_type, u32 trg_data_mask_l, u32 trg_data_mask_h);
int hsdt_trace_trg_timeout(u32 bus_type, u32 trg_timeout);
int hsdt_trace_trg_wait_num(u32 bus_type, u32 trg_wait_num);
int hsdt_trace_trigger_manual_cfg(u32 bus_type, u32 trg_manual_content);
int hsdt_trace_eof_int(u32 bus_type, u32 eof_int_content);
int hsdt_trace_cfg_event_pri(u32 bus_type, u32 event_id, u32 priority);
int hsdt_trace_event_cfg_ex(u32 bus_type, u32 event_id_start, u32 event_id_end, u32 priority);
int hsdt_trace_dump(u32 bus_type);
int hsdt_trace_start(u32 bus_type);
int hsdt_trace_stop(u32 bus_type);
int hsdt_trace_init(u32 bus_type);
#else
static inline int hsdt_trace_trg_cmd(u32 bus_type, u32 trg_cmd)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_event(u32 bus_type, u32 trg_event)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_data1(u32 bus_type, u32 trg_data_low, u32 trg_data_high)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_data2(u32 bus_type, u32 trg_data_low, u32 trg_data_high)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_data1_mask(u32 bus_type, u32 trg_data_mask_l, u32 trg_data_mask_h)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_data2_mask(u32 bus_type, u32 trg_data_mask_l, u32 trg_data_mask_h)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_timeout(u32 bus_type, u32 trg_timeout)
{
	return -EINVAL;
}

static inline int hsdt_trace_trg_wait_num(u32 bus_type, u32 trg_wait_num)
{
	return -EINVAL;
}

static inline int hsdt_trace_trigger_manual_cfg(u32 bus_type, u32 trg_manual_content)
{
	return -EINVAL;
}

static inline int hsdt_trace_eof_int(u32 bus_type, u32 eof_int_content)
{
	return -EINVAL;
}

static inline int hsdt_trace_cfg_event_pri(u32 bus_type, u32 event_id, u32 priority)
{
	return -EINVAL;
}

static inline int hsdt_trace_event_cfg_ex(u32 bus_type,
					  u32 event_id_start,
					  u32 event_id_end,
					  u32 priority)
{
	return -EINVAL;
}

static inline int hsdt_trace_dump(u32 bus_type)
{
	return -EINVAL;
}

static inline int hsdt_trace_start(u32 bus_type)
{
	return -EINVAL;
}

static inline int hsdt_trace_stop(u32 bus_type)
{
	return -EINVAL;
}

static inline int hsdt_trace_init(u32 bus_type)
{
	return -EINVAL;
}

#endif
#endif
