/*
 * rdr_audio_dump_socdsp.c
 *
 * audio dump socdsp driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/pm_wakeup.h>
#include <linux/of_irq.h>
#include <asm/tlbflush.h>
#include "rdr_audio_adapter.h"
#include "rdr_audio_soc.h"
#include "rdr_audio_dump_socdsp.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif
#include <linux/hisi/audio_log.h>
#include "dsp_misc.h"
#include "usbaudio_ioctl.h"
#include <linux/hisi/usb/hifi_usb.h>

#define LOG_TAG "rdr_audio_dump_socdsp"

#define ICC_DEBUG_ONE_INFO_LEN 76
#define MAX_NMI_COMPLTET_NUM 10
#define FLAG_ROW_LEN 64
#define FLAG_INFO_LEN 32
#define FLAG_COMMENT_LEN 128
#define PARSE_FLAG_LOG_SIZE (FLAG_ROW_LEN * ARRAY_SIZE(g_socdsp_flag) + FLAG_COMMENT_LEN)
#define PARSE_ICC_DEBUG_LOG_SIZE (ICC_DEBUG_ONE_INFO_LEN * ICC_STAT_MSG_NUM * 2 + 330)

#define RDR_DSP_DUMP_ADDR (DSP_DUMP_BIN_ADDR)
#define RDR_DSP_DUMP_SIZE (DSP_DUMP_BIN_SIZE)
#define CFG_DSP_NMI 0x3C /* dsp nmi, bit0-bit15 */
#define ASP_CFG_BASE SOC_ACPU_ASP_CFG_BASE_ADDR
#define OM_SOC_LOG_PATH "sochifi_logs"
#define PARSED_DATA_INFO "enLogID | uwTimeStamp | uhwFileID | uhwLineID | uwVal1 | uwVal2 | uwVal3\n"
struct rdr_socdsp_des_s {
	uint32_t modid;
	uint32_t wdt_irq_num;
	char *pathname;
	uint32_t *control_addr;
	uint32_t *lock_addr;
	uint32_t *intclr_addr;
	pfn_cb_dump_done dumpdone_cb;

	struct semaphore dump_sem;
	struct semaphore handler_sem;
	struct wakeup_source rdr_wl;
	struct task_struct *kdump_task;
	struct task_struct *khandler_task;
};

static struct rdr_socdsp_des_s g_dump_socdsp;

static const char * const g_mem_dyn_type[] = {
	"UCOM_MEM_DYN_TYPE_DDR",
	"UCOM_MEM_DYN_TYPE_TCM",
	"UCOM_MEM_DYN_TYPE_OCB",
	"UCOM_MEM_DYN_TYPE_USB_160K",
	"UCOM_MEM_DYN_TYPE_USB_96K",
	"UCOM_MEM_DYN_TYPE_BUTT",
};

static const char * const g_mem_dyn_om_enable[] = {
	"UCOM_MEM_DYN_OM_ENABLE_NO",
	"UCOM_MEM_DYN_OM_ENABLE_YES",
	"UCOM_MEM_DYN_OM_ENABLE_BUTT",
};

struct flag_info {
	char flag_name[FLAG_INFO_LEN];
	unsigned int flag_addr_offset;
};

static const struct flag_info g_socdsp_flag[] = {
	{ "DRV_DSP_PANIC_MARK",         (DRV_DSP_PANIC_MARK - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_UART_LOG_LEVEL",     (DRV_DSP_UART_LOG_LEVEL - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_UART_TO_MEM_CUR",    (DRV_DSP_UART_TO_MEM_CUR_ADDR - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_EXCEPTION_NO",       (DRV_DSP_EXCEPTION_NO - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_IDLE_COUNT",         (DRV_DSP_IDLE_COUNT_ADDR - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_LOADED_INDICATE",    (DRV_DSP_LOADED_INDICATE - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_POWER_STATUS",       (DRV_DSP_POWER_STATUS_ADDR - DSP_FLAG_DATA_ADDR) },
	{ "DRV_DSP_NMI_FLAG",           (DRV_DSP_NMI_FLAG_ADDR - DSP_FLAG_DATA_ADDR) }
};
/*lint -e446*/

void audio_rdr_nmi_notify_dsp(void)
{
	unsigned int value;
	void __iomem *rdr_aspcfg_base = NULL;

	rdr_aspcfg_base = ioremap(ASP_CFG_BASE, (unsigned long)SZ_4K);
	if (!rdr_aspcfg_base) {
		AUDIO_LOGE("rdr aspcfg base ioremap error");
		return;
	}

	value = (unsigned int)readl(rdr_aspcfg_base + CFG_DSP_NMI);
	value &= ~0x1;
	writel(value, (rdr_aspcfg_base + CFG_DSP_NMI));

	value |= 0x1;
	writel(value, (rdr_aspcfg_base + CFG_DSP_NMI));
	udelay(1);
	writel(0x0, (rdr_aspcfg_base + CFG_DSP_NMI));

	iounmap(rdr_aspcfg_base);
}

bool is_dsp_power_on(void)
{
	unsigned int *power_status_addr = NULL;
	int power_status;
	bool is_power_on = false;

	power_status_addr = (unsigned int *)ioremap_wc(DRV_DSP_POWER_STATUS_ADDR, 0x4);
	if (!power_status_addr) {
		AUDIO_LOGE("drv dsp power status addr ioremap failed");
		return false;
	}

	power_status = readl(power_status_addr);
	if (power_status == DRV_DSP_POWER_ON)
		is_power_on = true;
	else if (power_status == DRV_DSP_POWER_OFF)
		is_power_on = false;
	else
		AUDIO_LOGE("get dsp power status error 0x%x", power_status);

	iounmap(power_status_addr);

	return is_power_on;
}

static bool is_nmi_complete(void)
{
	unsigned int *nmi_flag_addr = NULL;
	bool is_complete = false;

	nmi_flag_addr = (unsigned int *)ioremap_wc(DRV_DSP_NMI_FLAG_ADDR, 0x4ul);
	if (!nmi_flag_addr) {
		AUDIO_LOGE("drv dsp nmi flag addr ioremap failed");
		return false;
	}

	if ((unsigned int)DRV_DSP_NMI_COMPLETE == (unsigned int)readl(nmi_flag_addr))
		is_complete = true;
	else
		is_complete = false;

	iounmap(nmi_flag_addr);

	return is_complete;
}

static int parse_socdsp_innerlog(const char *original_data, unsigned int original_data_size,
	char *parsed_data, unsigned int parsed_data_size, unsigned int core_type)
{
	int i;
	int ret = 0;
	unsigned int index;
	struct innerlog_obj *innerlog = NULL;

	if (!original_data || !parsed_data) {
		AUDIO_LOGE("input data buffer is null");
		return -EINVAL;
	}

	memset(parsed_data, 0, parsed_data_size);
	snprintf(parsed_data, parsed_data_size, "\n\n/*********[innerlog info begin]*********/\n\n");

	innerlog = (struct innerlog_obj *)original_data;
	index = innerlog->curr_idx;
	if (index < OM_LOG_INNER_MAX_NUM) {
		const char *title = PARSED_DATA_INFO;

		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data), "%s", title);
		for (i = 0; i < OM_LOG_INNER_MAX_NUM; i++) {
			snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
				"0x%-8x 0x%-13x %-11u %-10u 0x%-17x 0x%-17x 0x%-17x\n",
				innerlog->records[index].enlogid,
				innerlog->records[index].time_stamp,
				innerlog->records[index].fileid,
				innerlog->records[index].lineid,
				innerlog->records[index].value1,
				innerlog->records[index].value2,
				innerlog->records[index].value3);
			index++;
			if (index >= OM_LOG_INNER_MAX_NUM)
				index = 0;
		}
	} else {
		ret = -EINVAL;
		AUDIO_LOGE("innerlog info invalid");
	}
	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"\n\n/*********[innerlog info end]*********/\n\n");

	return ret;
}

static int parse_socdsp_dynmem(const char *original_data, unsigned int original_data_size,
	char *parsed_data, unsigned int parsed_data_size, unsigned int core_type)
{
	int i;
	struct mem_dyn_ctrl *dynmem = NULL;
	struct mem_dyn_status *status = NULL;
	struct mem_dyn_node *nodes = NULL;

	if (!original_data || !parsed_data) {
		AUDIO_LOGE("input data buffer is null");
		return -EINVAL;
	}

	memset(parsed_data, 0, parsed_data_size);
	snprintf(parsed_data, parsed_data_size, "\n\n/*********[dynmem info begin]*********/\n\n");
	dynmem = (struct mem_dyn_ctrl *)original_data;

	for (i = 0; i < UCOM_MEM_DYN_TYPE_BUTT; i++) {
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"astStatus[%s]\n", g_mem_dyn_type[i]);
		status = &dynmem->status[i];

		if (status->enable >= UCOM_MEM_DYN_OM_ENABLE_BUTT) {
			snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"|- enEnable: ERROR: 0x%-10x\n", status->enable);
		} else {
			snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"|- enEnable: %-28s\n", g_mem_dyn_om_enable[status->enable]);
		}

		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"|- uwTotalSize: %-10u\n |- uwUsedRate: %-10u\n |- astMemTrace\n |- uwCurrUsedRate: %-10u\n"
			"|- uwTimeStamp: 0x%-10x\n",
			status->total_size,
			status->used_rate,
			status->mem_trace.curr_used_rate,
			status->mem_trace.time_stamp);
	}

	for (i = 0; i < UCOM_MEM_DYN_NODE_MAX_NUM; i++) {
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"astNodes[%d]\n", i + 1);
		nodes = &dynmem->nodes[i];
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			" |- stBlk\n uwAddr: 0x%08x uwSize: %-10u uwFileId: %-10u uwLineId: %-10u\n",
			nodes->blk.addr, nodes->blk.size, nodes->blk.fileid, nodes->blk.lineid);
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			" |- pstNext: 0x%08x\n |- pstPrev: 0x%08x\n", nodes->next, nodes->prev);
	}

	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"\n\n/*********[dynmem info end]*********/\n\n");

	return 0;
}

static int parse_socdsp_flag(const char *original_data, unsigned int original_data_size,
	char *parsed_data, unsigned int parsed_data_size, unsigned int core_type)
{
	unsigned int i;

	if (!original_data || !parsed_data) {
		AUDIO_LOGE("invalid param:base_buf: %s, dump_flag_addr: %s, dump_flag_size: %u",
			original_data, parsed_data, (unsigned int)parsed_data_size);
		return -EINVAL;
	}

	memset(parsed_data, 0, parsed_data_size);
	snprintf(parsed_data, parsed_data_size, "\n\n/*********[flag info begin]*********/\n\n");

	for (i = 0; i < ARRAY_SIZE(g_socdsp_flag); i++)
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"%-26s 0x%08x\n", g_socdsp_flag[i].flag_name,
			*((unsigned int *)(original_data + g_socdsp_flag[i].flag_addr_offset)));

	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"\n\n/*********[flag info end]*********/\n\n");

	return 0;
}

static int parse_socdsp_iccdebug(const char *original_data, unsigned int original_data_size,
	char *parsed_data, unsigned int parsed_data_size, unsigned int core_type)
{
	unsigned int i;
	struct icc_dbg *dbg = NULL;
	struct icc_uni_msg_info *msg = NULL;

	if (!original_data || !parsed_data) {
		AUDIO_LOGE("input data buffer is null");
		return -EINVAL;
	}

	dbg = (struct icc_dbg *)original_data;
	memset(parsed_data, 0, parsed_data_size);
	snprintf(parsed_data, parsed_data_size, "\n\n/*********[iccdebug info begin]*********/\n\n");

	msg = &dbg->msg_stat.send.msg[0];
	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"SEND MSG FIFO  front: %-4urear: %-4usize: %-4u\nchannelid timestamp read write drop sendpid recvpid length msgid\n",
		dbg->msg_stat.send.front, dbg->msg_stat.send.rear, dbg->msg_stat.send.size);
	for (i = 0; i < ICC_STAT_MSG_NUM; i++)
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"%-11u0x%-10x%-7u%-7u%-6u%-9u%-9u%-8d0x%-4x\n",
			msg[i].channel_id, msg[i].time, msg[i].fifo_read_pos, msg[i].fifo_write_pos,
			msg[i].drop, msg[i].sender_pid, msg[i].receiver_pid, msg[i].packetlen, msg[i].msg_id);

	msg = &dbg->msg_stat.recv.msg[0];
	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"\nRECV MSG FIFO:  front: %-4urear: %-4usize: %-4u\nchannelid timestamp read write drop sendpid recvpid length msgid\n",
		dbg->msg_stat.recv.front, dbg->msg_stat.recv.rear, dbg->msg_stat.recv.size);
	for (i = 0; i < ICC_STAT_MSG_NUM; i++)
		snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
			"%-11u0x%-10x%-7u%-7u%-6u%-9u%-9u%-8u0x%-4x\n",
			msg[i].channel_id, msg[i].time, msg[i].fifo_read_pos, msg[i].fifo_write_pos,
			msg[i].drop, msg[i].sender_pid, msg[i].receiver_pid, msg[i].packetlen, msg[i].msg_id);

	snprintf(parsed_data + strlen(parsed_data), parsed_data_size - strlen(parsed_data),
		"\n\n/*********[iccdebug info end]*********/\n\n");

	return 0;
}

struct parse_log g_parse_socdsp_log[] = {
	{ DSP_OM_LOG_SIZE + DRV_DSP_UART_TO_MEM_SIZE, DRV_DSP_STACK_TO_MEM_SIZE, PARSER_SOCDSP_TRACE_SIZE, parse_dsp_trace },
	{ RDR_FALG_OFFSET, DSP_FLAG_DATA_SIZE, PARSE_FLAG_LOG_SIZE, parse_socdsp_flag },
	{ 0, SOCDSP_ORIGINAL_CPUVIEW_SIZE, PARSER_SOCDSP_CPUVIEW_LOG_SIZE, parse_dsp_cpuview },
	{ SOCDSP_ORIGINAL_CPUVIEW_SIZE, SOCDSP_ORIGINAL_INNERLOG_SIZE, PARSE_INNERLOG_SIZE, parse_socdsp_innerlog },
	{ SOCDSP_ORIGINAL_CPUVIEW_SIZE + SOCDSP_ORIGINAL_INNERLOG_SIZE, SOCDSP_ORIGINAL_DYNMEM_SIZE, PARSE_MEM_DYN_LOG_SIZE, parse_socdsp_dynmem },
	{ DSP_OM_LOG_SIZE + DRV_DSP_UART_TO_MEM_SIZE + DRV_DSP_STACK_TO_MEM_SIZE, DSP_ICC_DEBUG_SIZE, PARSE_ICC_DEBUG_LOG_SIZE, parse_socdsp_iccdebug },
};

static int socdsp_size_log(char **full_text, const char *buf, char **parse_text, size_t *socdsp_log_size)
{
	unsigned int i;
	int ret = 0;
	size_t full_text_size;
	char *text_ptr = NULL;
	char *parse_ptr = NULL;

	*socdsp_log_size = COMPILE_TIME_BUFF_SIZE + strlen("\n\n") +
		(DRV_DSP_UART_TO_MEM_SIZE - DRV_DSP_UART_TO_MEM_RESERVE_SIZE);
	full_text_size = *socdsp_log_size;
	for (i = 0; i < ARRAY_SIZE(g_parse_socdsp_log); i++)
		full_text_size += g_parse_socdsp_log[i].parse_log_size;
	text_ptr = vzalloc(full_text_size);
	if (!text_ptr) {
		*full_text = NULL;
		AUDIO_LOGE("error: alloc full text failed");
		return -ENOMEM;
	}

	/* socdsp log */
	memcpy(text_ptr + strlen(text_ptr), buf + DSP_OM_LOG_SIZE, COMPILE_TIME_BUFF_SIZE);
	memcpy(text_ptr + strlen(text_ptr), "\n\n", strlen("\n\n"));
	memcpy(text_ptr + COMPILE_TIME_BUFF_SIZE + strlen("\n\n"),
		buf + DSP_OM_LOG_SIZE + DRV_DSP_UART_TO_MEM_RESERVE_SIZE,
		DRV_DSP_UART_TO_MEM_SIZE - DRV_DSP_UART_TO_MEM_RESERVE_SIZE);

	/* start parse socdsp log */
	AUDIO_LOGI("start parse socdsp log");
	parse_ptr = text_ptr + *socdsp_log_size;
	for (i = 0; i < ARRAY_SIZE(g_parse_socdsp_log); i++) {
		if (*socdsp_log_size + strlen(parse_ptr) + g_parse_socdsp_log[i].parse_log_size > full_text_size) {
			AUDIO_LOGE("log size more than the full text size");
			break;
		}

		ret = g_parse_socdsp_log[i].parse_func(buf + g_parse_socdsp_log[i].original_offset,
			g_parse_socdsp_log[i].original_log_size,
			parse_ptr + strlen(parse_ptr),
			g_parse_socdsp_log[i].parse_log_size,
			SOCDSP);
		if (ret != 0)
			AUDIO_LOGE("error: parser module %u failed", i);
	}
	*full_text = text_ptr;
	*parse_text = parse_ptr;
	AUDIO_LOGI("end parser socdsp log");

	return ret;
}

static void check_nmi(void)
{
	int count = 0;
	char comment[RDR_COMMENT_LEN] = {0};

	while (count < MAX_NMI_COMPLTET_NUM) {
		if (is_nmi_complete())
			break;

		count++;
		msleep(1);
	}

	if (count == MAX_NMI_COMPLTET_NUM) {
		AUDIO_LOGE("NMI process is uncomplete in dsp, om log maybe inaccurate");
		snprintf(comment, RDR_COMMENT_LEN, "NMI process is uncomplete in dsp, OM log maybe inaccurate");
	} else {
		snprintf(comment, RDR_COMMENT_LEN, "OM log is fine");
	}
}

static int dump_socdsp_ddr(const char *filepath)
{
	char *buf = NULL;
	char *full_text = NULL;
	char *parse_text = NULL;
	char xn[RDR_FNAME_LEN] = {0};
	size_t socdsp_log_size = 0;
	int ret;

	if (!filepath) {
		AUDIO_LOGE("error: filepath is NULL");
		return -ENOENT;
	}

	check_nmi();

	snprintf(xn, RDR_FNAME_LEN, "%s%s/hifi_ddr.bin", filepath, OM_SOC_LOG_PATH);

	buf = (char *)ioremap_wc(RDR_DSP_DUMP_ADDR, RDR_DSP_DUMP_SIZE);
	if (!buf) {
		AUDIO_LOGE("error: ioremap socdsp dump addr fail");
		return -ENOMEM;
	}

	ret = socdsp_size_log(&full_text, buf, &parse_text, &socdsp_log_size);
	if (ret == -ENOMEM)
		goto alloc_err;

	if (!parse_text)
		goto alloc_err;

	ret = rdr_audio_write_file(xn, full_text, socdsp_log_size + strlen(parse_text));
	if (ret != 0)
		AUDIO_LOGE("rdr: dump %s fail", xn);

	vfree(full_text);
alloc_err:
	iounmap(buf);

	return ret;
}

static void recv_channel_fifo(struct icc_channel *channel, struct icc_dbg *icc_dbg_info)
{
	struct icc_channel_fifo *dst_fifo = NULL;
	struct icc_channel_fifo *src_fifo = NULL;
	/* save receive fifo */
	if (channel->fifo_recv != 0) {
		dst_fifo = (struct icc_channel_fifo *)(((char *)&(channel->fifo_recv)) +
			*(unsigned int *)(&channel->fifo_recv));
		src_fifo = (struct icc_channel_fifo *)ioremap_wc(*((unsigned int *)dst_fifo->data) +
			ICC_SHARE_FAMA_ADDR_OFFSET, dst_fifo->size + sizeof(struct icc_channel_fifo));
		if (!src_fifo) {
			AUDIO_LOGE("rdr: remap socdsp recv src fifo fail");
			iounmap(icc_dbg_info);
			return;
		}

		/* copy fifo channel data */
		memcpy((void *)((char *)dst_fifo + sizeof(*dst_fifo)),
			(void *)((char *)src_fifo + sizeof(*src_fifo)),
			(unsigned long)dst_fifo->size);
		/* update fifo header */
		dst_fifo->magic = src_fifo->magic;
		dst_fifo->read  = src_fifo->read;
		dst_fifo->write = src_fifo->write;
		iounmap(src_fifo);
	}
}

static void send_channel_fifo(struct icc_channel *channel, struct icc_dbg *icc_dbg_info)
{
	struct icc_channel_fifo *dst_fifo = NULL;
	struct icc_channel_fifo *src_fifo = NULL;
	/* save to send fifo */
	if (channel->fifo_send != 0) {
		dst_fifo = (struct icc_channel_fifo *)((char *)&(channel->fifo_send) +
			*(unsigned int *)(&channel->fifo_send));
		src_fifo = (struct icc_channel_fifo *)ioremap_wc(*((unsigned int *)dst_fifo->data) +
			ICC_SHARE_FAMA_ADDR_OFFSET, dst_fifo->size + sizeof(struct icc_channel_fifo));
		if (!src_fifo) {
			AUDIO_LOGE("rdr: remap socdsp send src fifo fail");
			iounmap(icc_dbg_info);
			return;
		}
		/* copy fifo channel data */
		memcpy((void *)((char *)dst_fifo + sizeof(struct icc_channel_fifo)),
			(void *)((char *)src_fifo + sizeof(struct icc_channel_fifo)),
			(unsigned long)(dst_fifo->size));
		/* update fifo header */
		dst_fifo->magic = src_fifo->magic;
		dst_fifo->read = src_fifo->read;
		dst_fifo->write = src_fifo->write;

		iounmap(src_fifo);
	}
}

/*lint -e548*/
static int save_icc_channel_fifo(void)
{
	int i;
	struct icc_dbg *icc_dbg_info = NULL;
	struct icc_channel *channel = NULL;

	icc_dbg_info = (struct icc_dbg *)ioremap_wc(DSP_ICC_DEBUG_LOCATION,
		(unsigned long)DSP_ICC_DEBUG_SIZE);
	if (!icc_dbg_info) {
		AUDIO_LOGE("ioremap socdsp icc dbg info fail");
		return -ENOMEM;
	}

	if ((icc_dbg_info->protectword1 != ICC_DEBUG_PROTECT_WORD1) ||
		(icc_dbg_info->protectword2 != ICC_DEBUG_PROTECT_WORD2)) {
		AUDIO_LOGE("check ICC_DEBUG_PROTECT_WORD fail 0x%x, 0x%x, do not save icc fifo",
			icc_dbg_info->protectword1, icc_dbg_info->protectword2);
		iounmap(icc_dbg_info);
		return -EINVAL;
	}

	for (i = 0; i < ICC_CHANNEL_NUM; i++) {
		channel = &icc_dbg_info->icc_channels[i];

		recv_channel_fifo(channel, icc_dbg_info);
		send_channel_fifo(channel, icc_dbg_info);
	}
	iounmap(icc_dbg_info);

	return 0;
}
/*lint +e446*/

static void dump_socdsp(const char *filepath)
{
	int ret;

	WARN_ON(filepath == NULL);

	ret = save_icc_channel_fifo();
	AUDIO_LOGI("rdr: save icc channel fifo, %s", ret ? "fail" : "success");

	ret = dump_socdsp_ddr(filepath);
	AUDIO_LOGI("rdr: dump socdsp ddr, %s", ret ? "fail" : "success");

}

int soc_dump_thread(void *arg)
{
	g_dump_socdsp.modid = ~0;
	g_dump_socdsp.pathname = NULL;
	g_dump_socdsp.dumpdone_cb = NULL;
	sema_init(&g_dump_socdsp.dump_sem, 0);

	AUDIO_LOGI("enter blackbox");

	while (!kthread_should_stop()) {
		if (down_interruptible(&g_dump_socdsp.dump_sem)) {
			AUDIO_LOGE("down sem fail");
			continue;
		}

		if (g_dump_socdsp.modid == RDR_AUDIO_NOC_MODID && is_dsp_power_on()) {
			AUDIO_LOGI("notify socdsp save tcm");
			audio_rdr_nmi_notify_dsp();
		}

		AUDIO_LOGD("begin to dump socdsp log");
		dump_socdsp(g_dump_socdsp.pathname);
		AUDIO_LOGD("end dump socdsp log");

		if (g_dump_socdsp.dumpdone_cb) {
			AUDIO_LOGD("begin dump socdsp done callback, modid: 0x%x", g_dump_socdsp.modid);
			g_dump_socdsp.dumpdone_cb(g_dump_socdsp.modid, (unsigned long long)RDR_HIFI);
			AUDIO_LOGD("end dump socdsp done callback");
		}
	}

	AUDIO_LOGI("exit blackbox");

	return 0;
}

void rdr_audio_soc_dump(unsigned int modid, char *pathname, pfn_cb_dump_done pfb)
{
	WARN_ON(pathname == NULL);
	WARN_ON(pfb == NULL);

	AUDIO_LOGI("enter blackbox");

	g_dump_socdsp.modid = modid;
	g_dump_socdsp.dumpdone_cb = pfb;
	g_dump_socdsp.pathname = pathname;

	up(&g_dump_socdsp.dump_sem);

	AUDIO_LOGI("exit  blackbox");
}
/*lint +e548*/

