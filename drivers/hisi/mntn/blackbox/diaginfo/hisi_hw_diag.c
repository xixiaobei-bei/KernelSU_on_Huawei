/*
 * hardware diaginfo record module
 *
 * Copyright (c) 2018 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/memory.h>
#include <asm/cacheflush.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/dma-direction.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/hisi_hw_diag.h>
#include <linux/hisi/mntn_dump.h>
#include <mntn_public_interface.h>
#include "libhwsecurec/securec.h"
#include "rdr_field.h"

static struct hisi_hw_diag_dev *g_hisi_hw_diag_dev;

static void clear_diag_trace(struct hisi_hw_diag_trace *trace)
{
	int ret;

	ret = memset_s(trace->data, HISI_HWDIAG_TRACE_DATA_MAXLEN,
		0x0, HISI_HWDIAG_TRACE_DATA_MAXLEN);
	if (ret < 0) {
		pr_err("[%s]memset_s error\n", __func__);
	}
	trace->err_id = 0;
	trace->used = 0;
	__dma_map_area((const void *)trace,
			sizeof(struct hisi_hw_diag_trace),
			DMA_TO_DEVICE);

	return;
}

static void fill_hw_diag_data(struct hisi_hw_diag_dev *d,
		unsigned int err_id, hisi_hw_diag_info *diaginfo)
{
	struct hisi_hw_diag_trace *trace;
	unsigned long flags;
	int ret;
	char tmpbuf[HISI_HWDIAG_TRACE_DATA_MAXLEN];

	switch (err_id) {
		case CPU_PANIC_INFO :
			ret = snprintf_s(tmpbuf, HISI_HWDIAG_TRACE_DATA_MAXLEN,
				(HISI_HWDIAG_TRACE_DATA_MAXLEN - 1),
				"cpu_num=%d", diaginfo->cpu_info.cpu_num);
			if (ret < 0)
				pr_err("[%s]CPU_PANIC_INFO diaginfo length error\n", __func__);
			break;
		case NOC_FAULT_INFO :
			ret = snprintf_s(tmpbuf, HISI_HWDIAG_TRACE_DATA_MAXLEN,
				(HISI_HWDIAG_TRACE_DATA_MAXLEN - 1),
				"init_flow=%s, target_flow=%s",
				diaginfo->noc_info.init_flow,
				diaginfo->noc_info.target_flow);
			if (ret < 0)
				pr_err("[%s]NOC_FAULT_INFO diaginfo length error\n", __func__);
			break;
		default :
			pr_err("[%s]unsupport err_id(%d)\n",
				__func__, err_id);
			return;
	}

	trace = d->trace_addr;
	spin_lock_irqsave(&d->record_lock, flags);
	if (trace->used) {
		spin_unlock_irqrestore(&d->record_lock, flags);
		pr_err("[%s]diaginfo store doing\n", __func__);
		return;
	}
	ret = memcpy_s((void *)trace->data, HISI_HWDIAG_TRACE_DATA_MAXLEN,
			(void *)tmpbuf, HISI_HWDIAG_TRACE_DATA_MAXLEN);
	if (ret < 0) {
		spin_unlock_irqrestore(&d->record_lock, flags);
		pr_err("[%s]copy trace data error\n", __func__);
		return;
	}
	trace->used = 1;
	trace->err_id = err_id;
	__dma_map_area((const void *)trace,
			sizeof(struct hisi_hw_diag_trace),
			DMA_TO_DEVICE);
	spin_unlock_irqrestore(&d->record_lock, flags);

}

static void save_hw_diag_log(struct hisi_hw_diag_dev *d, const char *date)
{
	struct hisi_hw_diag_trace *trace;
	unsigned long flags;

	trace = d->trace_addr;
	spin_lock_irqsave(&d->record_lock, flags);
	if (!trace->used) {
		pr_err("[%s]no trace data\n", __func__);
		spin_unlock_irqrestore(&d->record_lock, flags);
		return;
	}
	spin_unlock_irqrestore(&d->record_lock, flags);

	switch (trace->err_id) {
		case CPU_PANIC_INFO :
		case NOC_FAULT_INFO :
			break;
		default :
			pr_err("[%s]error err_id = %d\n", __func__, trace->err_id);
			goto clear_trace;
	}

	/* In normal process, '\0' is set at the middle or end of the string,
	   force a '\0' at the end to avoid abnormal process */
	trace->data[HISI_HWDIAG_TRACE_DATA_MAXLEN - 1] = '\0';
	bbox_diaginfo_record(trace->err_id, date, "%s", trace->data);

	pr_err("[%s]err_id(%d)\n", __func__, trace->err_id);

clear_trace:
	clear_diag_trace(trace);
	return;
}

/**
 * hisi_hw_diaginfo_trace - trace diaginfo in mntn hwdiag memory which to
 * be saved to hisi_diaginfo.log by hisi_hw_diaginfo_record, trace failed
 * if last trace has not been handle.
 *
 * @param  err_id   : define in bbox_diaginfo_id_def.h
 * @param  diaginfo : for ap_s_panic, trace cpunum;
 *                    for noc, trace init_flow & target_flow
 * @return: none
 *
 */
void hisi_hw_diaginfo_trace(unsigned int err_id, hisi_hw_diag_info *diaginfo)
{
	struct hisi_hw_diag_dev *d = g_hisi_hw_diag_dev;

	if (!d) {
		pr_err("[%s]hisi_hw_diag not init\n", __func__);
		return;
	}
	if (!diaginfo) {
		pr_err("[%s]diag info is NULL\n", __func__);
		return;
	}
	pr_info("[%s]err_id(%d)\n", __func__, err_id);
	fill_hw_diag_data(d, err_id, diaginfo);

	return;
}

/**
 * hisi_hw_diaginfo_record - record the traced info to
 * hisi_diaginfo.log and clear the trace.
 *
 * @param  date : the rtc time of recording, if input is null,
 *         tag the current rtc time.
 *
 * @return none
 *
 */
void hisi_hw_diaginfo_record(const char *date)
{
	struct hisi_hw_diag_dev *d = g_hisi_hw_diag_dev;

	if (!d) {
		pr_err("[%s]hisi_hw_diag not init\n", __func__);
		return;
	}

	save_hw_diag_log(d, date);
}

void hisi_hw_diag_init(void)
{
	int ret = 0;
	struct hisi_hw_diag_dev *d;

	pr_info("[%s]init start\n", __func__);
	d = kzalloc(sizeof(struct hisi_hw_diag_dev), GFP_KERNEL);
	if (!d) {
		pr_err("[%s]kzalloc failed\n", __func__);
		return;
	}

	ret = register_mntn_dump(MNTN_DUMP_HWDIAG,
			MNTN_DUMP_HWDIAG_SIZE, (void **)&d->trace_addr);
	if (ret < 0) {
		pr_err("[%s]register_mntn_dump failed\n", __func__);
		goto error_handler;
	}
	if (!d->trace_addr) {
		pr_err("[%s]trace addr invalid\n", __func__);
		goto error_handler;
	}

	d->trace_size = MNTN_DUMP_HWDIAG_SIZE;
	d->trace_max_num = d->trace_size / sizeof(struct hisi_hw_diag_trace);
	if (!d->trace_max_num) {
		pr_err("[%s]trace memory size lack\n", __func__);
		goto error_handler;
	}
	pr_info("[%s]diag info size(%d, %d)\n", __func__,
			d->trace_size, d->trace_max_num);

	spin_lock_init(&d->record_lock);

	g_hisi_hw_diag_dev = d;
	pr_info("[%s]init ok\n", __func__);
	return;

error_handler:
	kfree(d);
	return;
}
