

/*lint -e750
-esym(750,*)*/
/*lint -e40 -e64 -e826 -e838 -e515 -e516 -e613 -e528 -e78 -e530 -e835 -e737 -e701 -e834 -e705
-e713 -e774 -e413 -e715 -e732 -e647
-esym(40,*) -esym(64,*) -esym(826,*) -esym(838,*) -esym(515,*) -esym(516,*) -esym(613,*) -esym(528,*) -esym(78,*)
-esym(530,*) -esym(835,*) -esym(737,*) -esym(701,*) -esym(834,*) -esym(705,*) -esym(713,*) -esym(774,*) -esym(413,*)
-esym(715,*) -esym(732,*)  -esym(647,*)*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#include <linux/types.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "adapter_common.h"
#include "memory.h"
#include "cmdlst_drv.h"
#include "cmdlst_reg_offset.h"
#include "cvdr_drv.h"
#include "sub_ctrl_reg_offset.h"

#define MAX_STRIPE_WIDTH      (1024)
#define SCALER_RATIO          (4)
#define UNSIGNED_INT_MAX      (0xffffffff)

unsigned int frame_num = 0;
extern cmdlst_dev_t g_cmdlst_devs[];
extern cvdr_dev_t g_cvdr_devs[];

typedef struct _cmdlst_channel_t {
	struct list_head ready_list;
} cmdlst_channel_t;

typedef struct _cmdlst_priv_t {
	cmdlst_channel_t    cmdlst_chan[CMDLST_CHANNEL_MAX];
} cmdlst_priv_t;

static cmdlst_priv_t g_cmdlst_priv;

static int cmdlst_enqueue(unsigned int channel_id, cmdlst_para_t *cmdlst_para);

static int cmdlst_start(cfg_tab_cmdlst_t *cmdlst_cfg, unsigned int channel_id);

static int cmdlst_set_buffer_header(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t *cmdlst_para);

static int cmdlst_set_irq_mode(cmdlst_para_t *cmdlst_para, unsigned int stripe_index);

static int cmdlst_set_vpwr(cmd_buf_t *cmd_buf, cmdlst_rd_cfg_info_t *rd_cfg_info);
void dump_addr(unsigned long long addr, int num, char *info);

/**********************************************************
 * function name: cmdlst_set_reg
 *
 * description:
 *              fill current cmdlst buf
 * input:
 *     cmd_buf: current cmdlst buf to config.
 *     reg    : reg_addr
 *     value  : reg_val
 * return:
 *      NULL
 ***********************************************************/
void cmdlst_set_reg(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int val)
{
	if ((cmd_buf->data_size + 8) <= cmd_buf->buffer_size) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = ((reg) & 0x000fffff);
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = (val);
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	} else {
		e("cmdlst buffer is full: %u, @0x%x", cmd_buf->data_size, reg);
	}
}

/**********************************************************
 * function name: cmdlst_update_buffer_header
 *
 * description:
 *              fill current cmdlst buf
 * input:
 *     cmd_buf: current cmdlst buf to config.
 * return:
 *      NULL
 ***********************************************************/

void cmdlst_update_buffer_header(schedule_cmdlst_link_t *cur_point, schedule_cmdlst_link_t *next_point,
								 unsigned int channel_id)
{
	cur_point->cmd_buf.next_buffer = (void *)&next_point->cmd_buf;
	unsigned int next_hw_prio = next_point->cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_priority;
	unsigned int next_hw_resource = next_point->cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_ressource_request;
	unsigned int next_hw_token_nbr = next_point->cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_link_token_nbr;
	unsigned int next_hw_link_channel = next_point->cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_link_channel;
	cmd_buf_t *cmd_buf = &cur_point->cmd_buf;
	cmd_buf_t *next_buf = cmd_buf->next_buffer;

	if (next_buf == NULL) {
		unsigned int i  = 0;

		for (i = 0; i < 10; i++)
			*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x4 * i) = CMDLST_PADDING_DATA; //lint !e647

		return;
	}

	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x0) = 0x000050BC + 0x80 * channel_id;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x4) = (next_hw_prio << 31) | (next_hw_resource << 8) |
			(next_hw_token_nbr << 4) | (next_hw_link_channel);
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x8) = 0x000050D0 + 0x80 * channel_id;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0xC) = next_buf->start_addr_isp_map >> 2;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x10) = 0x000050C4 + 0x80 * channel_id;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x14) = ((next_buf->start_addr_isp_map + next_buf->data_size)
			& 0xffffe000) | 0x0000012E;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x18) = 0x000050C8 + 0x80 * channel_id;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x1C) = (next_buf->data_size >> 3) - 1;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x20) = 0x000050CC + 0x80 * channel_id;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x24) = 0;
}

void cmdlst_update_buffer_header_last(schedule_cmdlst_link_t *cur_point, schedule_cmdlst_link_t *next_point,
									  unsigned int channel_id)
{
	cmd_buf_t *cmd_buf = &cur_point->cmd_buf;
	unsigned int i  = 0;

	for (i = 0; i < 10; i++)
		*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + 0x4 * i) = CMDLST_PADDING_DATA; //lint !e647

	return;
}


/**********************************************************
 * function name: cmdlst_set_reg_incr
 *
 * description:
 *              current cmdlst buf
 * input:
 *     cmd_buf: current cmdlst buf to config.
 *     reg    : register start address
 *     size   : register numbers
 *     incr   : register address increment or not
 * return:
 *      NULL
 ***********************************************************/
void cmdlst_set_reg_incr(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int size, unsigned int incr,
						 unsigned int is_read)
{
	if ((cmd_buf->data_size + (size + 1) * 4) > cmd_buf->buffer_size) {
		e("cmdlst buffer is full: %u, @0x%x", cmd_buf->data_size, reg);
		return;
	}

	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = (reg & 0x001ffffc) | (((size - 1) & 0xFF) << 24) | ((
				incr & 0x1) << 1) |
			(is_read & 0x1);
	cmd_buf->data_addr += 4;
	cmd_buf->data_size += 4;
}

/**********************************************************
 * function name: cmdlst_set_reg_data
 *
 * description:
 *              current cmdlst buf
 * input:
 *     cmd_buf: current cmdlst buf to config.
 *     data   : register value
 * return:
 *      NULL
 ***********************************************************/
void cmdlst_set_reg_data(cmd_buf_t *cmd_buf, unsigned int data)
{
	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = data;
	cmd_buf->data_addr += 4;
	cmd_buf->data_size += 4;
}

/**********************************************************
 * function name: cmdlst_set_buffer_header
 *
 * description:
 *               set current cmdlst buf header.
 * input:
 *     cmd_buf      : current cmdlst buf
 *     irq          : irq mode.
 *     cpu_enable   : 1 to hardware,3to hardware and cpu
 * return:
 *      0;
 ***********************************************************/
static int cmdlst_set_buffer_header(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t *cmdlst_para)
{
	unsigned int idx;

	/* reserve 10 words to update next buffer later */
	for (idx = 0; idx < 10; idx++) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}

	cmdlst_set_irq_mode(cmdlst_para, stripe_index);

	while (cmd_buf->data_size < CMDLST_HEADER_SIZE) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}

	return CPE_FW_OK;
}

int cmdlst_read_buffer(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t *cmdlst_para)
{
	d("[%s]+\n", __func__);
	unsigned int idx;
	cmdlst_rd_cfg_info_t *rd_cfg_info = &cmdlst_para->cmd_stripe_info[stripe_index].rd_cfg_info;
	d("stripe_cnt =%d, fs =0x%x \n", cmdlst_para->stripe_cnt, rd_cfg_info->fs);

	if (0 != rd_cfg_info->fs) {
		cmdlst_set_vpwr(cmd_buf, rd_cfg_info);

		for (idx = 0; idx < rd_cfg_info->rd_cfg_num; idx++) {
			*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = rd_cfg_info->rd_cfg[idx];
			cmd_buf->data_addr += 4;
			cmd_buf->data_size += 4;
		}
	}

	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
	cmd_buf->data_addr += 4;
	cmd_buf->data_size += 4;
	d("[%s]-\n", __func__);
	return CPE_FW_OK;
}

/**********************************************************
 * function name: cmdlst_set_irq_mode
 *
 * description:
 *               set irq mode according frame type.
 * input:
 * return:
 *      0;
 ***********************************************************/
static int cmdlst_set_irq_mode(cmdlst_para_t *cmdlst_para, unsigned int stripe_index)
{
	schedule_cmdlst_link_t *cmd_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;
	cmd_buf_t *cmd_buf = &cmd_entry[stripe_index].cmd_buf;
	unsigned int irq_mode = cmdlst_para->cmd_stripe_info[stripe_index].irq_mode;
	unsigned int channel_id = cmdlst_para->channel_id;
	unsigned int cpu_enable  = (unsigned int)(cmdlst_para->cmd_stripe_info[stripe_index].is_last_stripe ? 3 : 1);

	switch (channel_id) {
	case CPE_CHANNEL: {
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CPE_IRQ_REG0_REG, 0x03FFFFFF); //clear cpe eof
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG, 0x1 << CPE_CHANNEL);
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_CHN0_REG + CPE_CHANNEL * 0x4, cpu_enable);
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CPE_IRQ_REG1_REG, 0x0 << 16 | irq_mode);
		break;
	}

	case SLAM_CHANNEL: {
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_SLAM_IRQ_REG0_REG, 0x3FFF); //clear cpe eof
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG, 0x1 << SLAM_CHANNEL);
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_CHN0_REG + SLAM_CHANNEL * 0x4, cpu_enable);
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_SLAM_IRQ_REG1_REG, 0x0 << 16 | irq_mode);
		break;
	}

	case RDR_CHANNEL: {
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_RDR_IRQ_REG0_REG, 0x1F); //clear cpe eof
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG,
					   0x1 << RDR_CHANNEL); //clear acpu cmdlst channel0 irq eof
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_CHN0_REG + RDR_CHANNEL * 0x4, cpu_enable);
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_RDR_IRQ_REG1_REG, 0x0 << 16 | irq_mode);
		break;
	}

	case CMP_CHANNEL: {
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMP_IRQ_REG0_REG, 0x1F); //clear cpe eof
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_ACPU_IRQ_REG0_REG,
					   0x1 << CMP_CHANNEL); //clear acpu cmdlst channel0 irq eof
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMDLST_CHN0_REG + CMP_CHANNEL * 0x4, cpu_enable);
		cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET + SUB_CTRL_CMP_IRQ_REG1_REG, 0x0 << 16 | irq_mode);
		break;
	}

	default: {
		d("Fail, Invalid channel = %d\n", channel_id);
		break;
	}
	}

	d("cmdlst_channel = %d, irq_mode = 0x%x\n", channel_id, irq_mode);
	return CPE_FW_OK;
}


/**********************************************************
 * function name: cmdlst_set_vpwr
 *
 * description:
 *               set cmdlst read configuration
 * input:
 *     cmd_buf      : current cmdlst buf
 *     rd_cfg_info  : read configuration
 * return:
 *      0;
 ***********************************************************/
static int cmdlst_set_vpwr(cmd_buf_t *cmd_buf, cmdlst_rd_cfg_info_t *rd_cfg_info)
{
	cfg_tab_cvdr_t cmdlst_w3_1_table;

	if (NULL == cmd_buf) {
		e("cmdlst buf is null!");
		return CPE_FW_ERR;
	}

	if (NULL == rd_cfg_info) {
		e("cmdlst read cfg info is null!");
		return CPE_FW_ERR;
	}

	loge_if(memset_s(&cmdlst_w3_1_table, sizeof(cfg_tab_cvdr_t), 0, sizeof(cfg_tab_cvdr_t)));
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].to_use = 1;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].id = get_cvdr_vp_wr_port_num(WR_CMDLST);
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.fs_addr = rd_cfg_info->fs;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.pix_fmt = DF_D32;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.pix_expan = 1;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.last_page = (rd_cfg_info->fs + CMDLST_32K_PAGE - 1) >> 15;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.line_stride = 0;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.line_wrap = 0x3FFF;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter0 = 0xF;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter1 = 0xF;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter2 = 0xF;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter3 = 0xF;
	cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter_reload = 0xF;
	loge_if_ret(g_cvdr_devs[0].ops->prepare_cmd(&g_cvdr_devs[0], cmd_buf, &cmdlst_w3_1_table));
	return CPE_FW_OK;
}
/**********************************************************
 * function name: cmdlst_set_buffer_padding
 *
 * description:
 *               set cmd buf rest as padding data
 * input:
 *     cmd_buf      : current cmdlst buf
 * return:
 *      0;
 ***********************************************************/
int cmdlst_set_buffer_padding(cmd_buf_t *cmd_buf)
{
	/* Padding two words for D64 */
	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
	cmd_buf->data_addr += 4;
	cmd_buf->data_size += 4;
	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
	unsigned int aligned_data_size = 0;
	aligned_data_size = ipp_align_up(cmd_buf->data_size, 128);
	unsigned int i = 0;

	for (i = cmd_buf->data_size; i < aligned_data_size; i += 4) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
	}

	cmd_buf->data_size = aligned_data_size;
	return CPE_FW_OK;
}

static int cmdlst_set_branch(unsigned int channel_id)
{
	cmdlst_state_t st;
	st.ch_id = channel_id;
	loge_if_ret(g_cmdlst_devs[0].ops->set_branch(&g_cmdlst_devs[0], st.ch_id));
	udelay(1);/*lint !e778 !e774 !e747*/
	return CPE_FW_OK;
}

/**********************************************************
 * function name: ipp_eop_handler
 *
 * description:
 *              eop handler to dequeue a done-frame
 * input:
 *     NULL
 * return:
 *      0;
 ***********************************************************/
int ipp_eop_handler(cmdlst_eof_mode_e mode)
{
	unsigned int channel_id = CPE_CHANNEL;

	if (mode == CMD_EOF_SLAM_MODE)
		channel_id = SLAM_CHANNEL;
	else if (mode == CMD_EOF_REORDER_MODE)
		channel_id = RDR_CHANNEL;
	else if (mode == CMD_EOF_COMPARE_MODE)
		channel_id = CMP_CHANNEL;

	struct list_head *ready_list = NULL;
	ready_list = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;

	while (!list_empty(ready_list))
		list_del(ready_list->next);

	if (CPE_CHANNEL == channel_id) {
		cpe_mem_free(MEM_ID_CMDLST_BUF_MM);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_MM);
		cpe_mem_free(MEM_ID_CMDLST_PARA_MM);
	} else if (SLAM_CHANNEL == channel_id) {
		cpe_mem_free(MEM_ID_CMDLST_BUF_SLAM);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_SLAM);
		cpe_mem_free(MEM_ID_CMDLST_PARA_SLAM);
	} else if (RDR_CHANNEL == channel_id) {
		cpe_mem_free(MEM_ID_CMDLST_BUF_REORDER);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_REORDER);
		cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER);
	} else if (CMP_CHANNEL == channel_id) {
		cpe_mem_free(MEM_ID_CMDLST_BUF_COMPARE);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_COMPARE);
		cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE);
	} else {
		e("[%s] Invilid channel id = %d\n", __func__, channel_id);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

/**********************************************************
 * function name: cmdlst_enqueue
 *
 * description:
 *              a new frame to enqueue
 * input:
 *     last_exec:last exec stripe's start_addr
 *     cmd_buf:current cmdlst buf
 *     prio:priority of new frame
 *     stripe_cnt :total stripe of new frame
 *     frame_type:new frame type
 * return:
 *      0;
 ***********************************************************/
static int cmdlst_enqueue(unsigned int channel_id, cmdlst_para_t *cmdlst_para)
{
	struct list_head *ready_list = NULL;
	ready_list       = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;
#if FLAG_LOG_DEBUG
	struct list_head *dump_list = NULL;

	for (dump_list = ready_list->next; dump_list != ready_list; dump_list = dump_list->next)
		d("list[0x%llx]", (unsigned long long)(uintptr_t)dump_list);    //lint !e559

#endif
	schedule_cmdlst_link_t *pos = NULL;
	schedule_cmdlst_link_t *n = NULL;
	list_for_each_entry_safe(pos, n, ready_list, list_link) {
		if (pos->list_link.next != ready_list) {
			cmdlst_update_buffer_header(pos, n, channel_id);
		} else {
			cmdlst_update_buffer_header_last(pos, n, channel_id);
		}
	}
	return CPE_FW_OK;
}

/**********************************************************
 * function name: cmdlst_start
 *
 * description:
 *              start cmdlst when branch,or the first frame.
 * input:
 *     last_exec:last exec stripe's start_addr
 *     cmdlst_cfg: cmdlst config table
 * return:
 *      0;
 ***********************************************************/
static int cmdlst_start(cfg_tab_cmdlst_t *cmdlst_cfg, unsigned int channel_id)
{
	struct list_head *cmdlst_insert_queue = NULL;
	cmdlst_insert_queue       = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;

	if (NULL != cmdlst_cfg)
		cmdlst_do_config(&g_cmdlst_devs[0], cmdlst_cfg);

	return CPE_FW_OK;
}

int df_sched_set_buffer_header(cmdlst_para_t *cmdlst_para)
{
	unsigned int i = 0;
	schedule_cmdlst_link_t *cmd_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;

	for (i = 0; i < cmdlst_para->stripe_cnt; i++)
		cmdlst_set_buffer_header(i, &cmd_entry[i].cmd_buf, cmdlst_para);

	return CPE_FW_OK;
}

int cmdlst_priv_prepare(void)
{
	unsigned int i;

	for (i = 0; i < CMDLST_CHANNEL_MAX; i++)
		INIT_LIST_HEAD(&g_cmdlst_priv.cmdlst_chan[i].ready_list);

	return CPE_FW_OK;
}

static int df_sched_prepare_cpe_channel(schedule_cmdlst_link_t **new_entry, unsigned long long *va, unsigned int *da,
										int *ret)
{
	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_ENTRY_MM, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_ENTRY_MM);
		return CPE_FW_ERR;
	}

	*new_entry = (schedule_cmdlst_link_t *)(uintptr_t)(*va);

	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_BUF_MM, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_BUF_MM);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

static int df_sched_prepare_slam_channel(schedule_cmdlst_link_t **new_entry, unsigned long long *va, unsigned int *da,
		int *ret)
{
	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_ENTRY_SLAM, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_ENTRY_SLAM);
		return CPE_FW_ERR;
	}

	*new_entry = (schedule_cmdlst_link_t *)(uintptr_t)(*va);

	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_BUF_SLAM, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_BUF_SLAM);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

static int df_sched_prepare_rdr_channel(schedule_cmdlst_link_t **new_entry, unsigned long long *va, unsigned int *da,
										int *ret)
{
	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_ENTRY_REORDER, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_ENTRY_REORDER);
		return CPE_FW_ERR;
	}

	*new_entry = (schedule_cmdlst_link_t *)(uintptr_t)(*va);

	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_BUF_REORDER, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_BUF_REORDER);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

static int df_sched_prepare_cmp_channel(schedule_cmdlst_link_t **new_entry, unsigned long long *va, unsigned int *da,
										int *ret)
{
	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_ENTRY_COMPARE, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_ENTRY_COMPARE);
		return CPE_FW_ERR;
	}

	*new_entry = (schedule_cmdlst_link_t *)(uintptr_t)(*va);

	if ((*ret = cpe_mem_get(MEM_ID_CMDLST_BUF_COMPARE, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_BUF_COMPARE);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

int df_sched_prepare(cmdlst_para_t *cmdlst_para)
{
	unsigned int i;
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;

	if (!list_empty(&g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list))
		e("ready list not clean out");

	schedule_cmdlst_link_t *new_entry = NULL;

	if (CPE_CHANNEL == cmdlst_para->channel_id) {
		if (df_sched_prepare_cpe_channel(&new_entry, &va, &da, &ret) != CPE_FW_OK)
			return CPE_FW_ERR;
	} else if (SLAM_CHANNEL == cmdlst_para->channel_id) {
		if (df_sched_prepare_slam_channel(&new_entry, &va, &da, &ret) != CPE_FW_OK)
			return CPE_FW_ERR;
	} else if (RDR_CHANNEL == cmdlst_para->channel_id) {
		if (df_sched_prepare_rdr_channel(&new_entry, &va, &da, &ret) != CPE_FW_OK)
			return CPE_FW_ERR;
	} else if (CMP_CHANNEL == cmdlst_para->channel_id) {
		if (df_sched_prepare_cmp_channel(&new_entry, &va, &da, &ret) != CPE_FW_OK)
			return CPE_FW_ERR;
	}

	if (NULL == new_entry) {
		e("fail to memory_alloc new entry!");
		return CPE_FW_ERR;
	}

	loge_if(memset_s(new_entry,  cmdlst_para->stripe_cnt * sizeof(schedule_cmdlst_link_t), 0,
					 cmdlst_para->stripe_cnt * sizeof(schedule_cmdlst_link_t)));
	cmdlst_para->cmd_entry = (void *)new_entry;
	new_entry[0].cmd_buf.start_addr  = va;
	new_entry[0].cmd_buf.start_addr_isp_map = da;

	for (i = 0; i < cmdlst_para->stripe_cnt; i++) {
		new_entry[i].stripe_cnt = cmdlst_para->stripe_cnt;
		new_entry[i].stripe_index = i;
		new_entry[i].data = (void *)cmdlst_para;
		list_add_tail(&new_entry[i].list_link, &g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list);
		new_entry[i].cmd_buf.start_addr  = new_entry[0].cmd_buf.start_addr + (unsigned long long)(
											   CMDLST_BUFFER_SIZE * (unsigned long long)i);  //lint !e647
		new_entry[i].cmd_buf.start_addr_isp_map = new_entry[0].cmd_buf.start_addr_isp_map + CMDLST_BUFFER_SIZE * i;

		if (0 == new_entry[i].cmd_buf.start_addr) {
			e("fail to get cmdlist buffer!");
			return CPE_FW_ERR;
		}

		new_entry[i].cmd_buf.buffer_size = CMDLST_BUFFER_SIZE;
		new_entry[i].cmd_buf.header_size = CMDLST_HEADER_SIZE;
		new_entry[i].cmd_buf.data_addr   = new_entry[i].cmd_buf.start_addr;
		new_entry[i].cmd_buf.data_size   = 0;
		new_entry[i].cmd_buf.next_buffer = NULL;
	}

	return CPE_FW_OK;
}

#if FLAG_LOG_DEBUG
static void cmdlst_dump_queue(cmdlst_para_t *cmdlst_para)
{
	unsigned int channel_id    = cmdlst_para->channel_id;
	schedule_cmdlst_link_t *cmdlst_temp_link = NULL;
	schedule_cmdlst_link_t *n = NULL;
	struct list_head  *cmdlst_insert_queue       = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;
	list_for_each_entry_safe(cmdlst_temp_link, n, cmdlst_insert_queue, list_link) {
		d("@@@@ dump queue stripe_cnt =%d,stripe_index =%d, channel_id  =%d, cmd_buf.start_addr = 0x%llx,start_addr_isp_map = 0x%08x, cmd_buf.size = 0x%x",
		  cmdlst_temp_link->stripe_cnt, cmdlst_temp_link->stripe_index, channel_id, cmdlst_temp_link->cmd_buf.start_addr,
		  cmdlst_temp_link->cmd_buf.start_addr_isp_map, cmdlst_temp_link->cmd_buf.data_size);
		d("@@@@ dump queue en_link =%d, ch_link =%d, ch_link_act_nbr =%d, irq_mode = 0x%x",
		  cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].en_link,
		  cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].ch_link,
		  cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].ch_link_act_nbr,
		  cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].irq_mode);
	}
}
#endif

int df_sched_start(cmdlst_para_t *cmdlst_para)
{
	unsigned int channel_id    = cmdlst_para->channel_id;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;
	loge_if_ret(cmdlst_set_branch(channel_id));
	loge_if_ret(cmdlst_enqueue(channel_id, cmdlst_para));
#if FLAG_LOG_DEBUG
	cmdlst_dump_queue(cmdlst_para);
#endif
	loge_if_ret(cmdlst_start(&(cmd_link_entry[0].cmdlst_cfg_tab), channel_id));
	list_for_each_entry(cmd_link_entry, &g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list, list_link) {
		cmdlst_buff_dump(&cmd_link_entry->cmd_buf);
	}
	frame_num++;
	return CPE_FW_OK;
}

void dump_addr(unsigned long long addr, int num, char *info)
{
#define DATA_PERLINE    (16)
	int i = 0;
	d("%s Dump ......\n", info);

	for (i = 0; i < num; i += DATA_PERLINE)
		d("0x%llx : 0x%08x 0x%08x 0x%08x 0x%08x\n", addr + i,
		  readl((volatile void __iomem *)(uintptr_t)(addr + i + 0x00)),
		  readl((volatile void __iomem *)(uintptr_t)(addr + i + 0x04)),
		  readl((volatile void __iomem *)(uintptr_t)(addr + i + 0x08)),
		  readl((volatile void __iomem *)(uintptr_t)(addr + i + 0x0C)));
}

void cmdlst_buff_dump(cmd_buf_t *cmd_buf)
{
#if FLAG_LOG_DEBUG
	dump_addr(cmd_buf->start_addr, cmd_buf->data_size, "cmdlst_buff");
#endif
}

void cmdlst_set_reg_by_cpu(unsigned int reg, unsigned int val)
{
	unsigned int temp_reg;
	temp_reg = ((reg) & 0x000fffff);

	if (temp_reg >= 0x4000 && temp_reg < 0x5000)
		hispcpe_reg_set(CPE_TOP, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0x5000 && temp_reg < 0x6000)
		hispcpe_reg_set(CMDLIST_REG, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0x6000 && temp_reg < 0x8000)
		hispcpe_reg_set(CVDR_REG, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0xa000 && temp_reg < 0xb000)
		hispcpe_reg_set(MCF_REG, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0xb000 && temp_reg < 0xc000)
		hispcpe_reg_set(MFNR_REG, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0x20000 && temp_reg < 0x40000)
		hispcpe_reg_set(SMMU_REG, ((temp_reg) & 0x0000ffff), val);
}

void df_size_dump_stripe_info(isp_stripe_info_t *p_stripe_info, char *s)
{
	unsigned int i = 0;
	char type_name[LOG_NAME_LEN];

	if (p_stripe_info->stripe_cnt == 0)
		return;

	loge_if(memset_s((void *)type_name, LOG_NAME_LEN, 0, LOG_NAME_LEN));
	d("%s stripe_cnt = %d\n", type_name, p_stripe_info->stripe_cnt);

	for (i = 0; i < p_stripe_info->stripe_cnt; i++)
		d("%s: stripe_width[%d] = %u, start_point[%d] = %d, end_point[%d] = %d, overlap_left[%d] = %d, overlap_right[%d] = %d\n",
		  type_name, i, p_stripe_info->stripe_width[i],
		  i, p_stripe_info->stripe_start_point[i], i, p_stripe_info->stripe_end_point[i],
		  i, p_stripe_info->overlap_left[i], i, p_stripe_info->overlap_right[i]);

	return;
}

static unsigned int df_size_calc_stripe_width(unsigned int active_stripe_width, unsigned int input_align,
		unsigned int overlap)
{
	if (active_stripe_width == UNSIGNED_INT_MAX || input_align == 0)
		return UNSIGNED_INT_MAX;

	unsigned int tmp_active_stripe_width = active_stripe_width / (1 << 16);

	if (active_stripe_width % (1 << 16))
		tmp_active_stripe_width++;

	unsigned int stripe_width = tmp_active_stripe_width + overlap * 2;
	stripe_width = (stripe_width / input_align) * input_align;

	if (stripe_width % input_align)
		stripe_width += input_align;

	return stripe_width;
}

static int df_size_get_max_constrain(df_size_constrain_t *p_size_constrain, unsigned int *active_stripe_width,
									 unsigned int constrain_cnt, unsigned int *max_in_stripe_align, unsigned int *max_frame_width, unsigned int width)
{
	unsigned int i = 0;

	for (i = 0; i < constrain_cnt; i++) {
		if (p_size_constrain[i].out_width == UNSIGNED_INT_MAX)
			continue;

		unsigned int max_out_width = p_size_constrain[i].pix_align * (p_size_constrain[i].out_width /
									 p_size_constrain[i].pix_align);

		if (p_size_constrain[i].out_width != max_out_width)
			p_size_constrain[i].out_width = max_out_width;

		unsigned int tmp_input_width = max_out_width * p_size_constrain[i].hinc;

		if (tmp_input_width < *active_stripe_width)
			*active_stripe_width = tmp_input_width;

		if (p_size_constrain[i].hinc * p_size_constrain[i].pix_align > *max_in_stripe_align) {
			*max_in_stripe_align = p_size_constrain[i].hinc * p_size_constrain[i].pix_align;
			*max_frame_width     = p_size_constrain[i].hinc * ((unsigned int)(width << 16) / p_size_constrain[i].hinc);
		}
	}

	return CPE_FW_OK;
}

static int df_size_calc_stripe_info(isp_stripe_info_t *p_stripe_info, unsigned int stripe_end,
									unsigned int stripe_start, unsigned int stripe_cnt, unsigned int overlap, unsigned int input_align,
									unsigned int i, unsigned int width)
{
	p_stripe_info->stripe_start_point[i] = stripe_start;
	p_stripe_info->stripe_end_point[i]	 = stripe_end;
	p_stripe_info->overlap_left[i]		 = (i == 0) ? 0 : overlap;
	p_stripe_info->overlap_right[i] 	 = (i == stripe_cnt - 1) ? 0 : overlap;
	p_stripe_info->stripe_cnt			 = stripe_cnt;
	unsigned int start_point = p_stripe_info->stripe_start_point[i] - (p_stripe_info->overlap_left[i] << 16);
	p_stripe_info->stripe_start_point[i] = (start_point / (input_align << 16)) * input_align;
	p_stripe_info->overlap_left[i]		 = (i == 0) ? 0 : (p_stripe_info->stripe_end_point[i - 1] -
										   p_stripe_info->overlap_right[i - 1] - p_stripe_info->stripe_start_point[i]);
	unsigned int end_point	 = p_stripe_info->stripe_end_point[i] + (p_stripe_info->overlap_right[i] << 16);
	p_stripe_info->stripe_end_point[i] = (end_point / (input_align << 16)) * input_align;

	if (end_point % (input_align << 16))
		p_stripe_info->stripe_end_point[i] += input_align;

	if (i == stripe_cnt - 1)
		p_stripe_info->stripe_end_point[i] = width;

	p_stripe_info->overlap_right[i]    = (i == stripe_cnt - 1) ? 0 :
										 (p_stripe_info->stripe_end_point[i] - ipp_align_up(p_stripe_info->stripe_end_point[i] - overlap, CVDR_ALIGN_BYTES / 2));
	p_stripe_info->stripe_width[i] = p_stripe_info->stripe_end_point[i] - p_stripe_info->stripe_start_point[i];
	return CPE_FW_OK;
}

static int df_size_do_split_stripe(isp_stripe_info_t *p_stripe_info, df_size_constrain_t *p_size_constrain,
								   unsigned int constrain_cnt, unsigned int stripe_cnt, unsigned int max_in_stripe_align, unsigned int overlap,
								   unsigned int width, unsigned int input_align)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int is_again_calc = 0;

	do {
		is_again_calc = 0;
		unsigned int tmp_last_end    = 0;
		unsigned int last_stripe_end = 0;
		if (stripe_cnt > MAX_CPE_STRIPE_NUM || stripe_cnt == 0 || max_in_stripe_align == 0) {
			e("stripe_cnt error, stripe_cnt = %d, max_in_stripe_align = %d", stripe_cnt, max_in_stripe_align);
			return CPE_FW_ERR;
		}
		for (i = 0; i < stripe_cnt; i++) {
			unsigned long long int active_stripe_end = ((i + 1) * ((unsigned long long int)(width) << 16)) / stripe_cnt; //lint !e414

			if (i != stripe_cnt - 1) {
				active_stripe_end = (active_stripe_end / max_in_stripe_align) * max_in_stripe_align; //lint !e414

				if (active_stripe_end <= tmp_last_end)
					printk("The most downscaler imposes an active region superior to the maximum allowable\n");

				tmp_last_end = active_stripe_end;
			}

			unsigned int stripe_end   = 0;
			unsigned int stripe_start = UNSIGNED_INT_MAX;

			for (j = 0; j != constrain_cnt; j++) {
				unsigned int active_pix_align = p_size_constrain[j].pix_align * p_size_constrain[j].hinc;
				unsigned int tmp_stripe_start = (i == 0) ? 0 : last_stripe_end;
				unsigned int tmp_stripe_end   = 0;

				if (tmp_stripe_start < stripe_start)
					stripe_start = tmp_stripe_start;

				tmp_stripe_end = (i == stripe_cnt - 1) ? active_stripe_end : (active_stripe_end / active_pix_align) * active_pix_align;

				if (active_stripe_end - tmp_stripe_end > active_pix_align / 2)
					tmp_stripe_end += active_pix_align;

				if (tmp_stripe_end > stripe_end)
					stripe_end = tmp_stripe_end;
			}

			last_stripe_end = stripe_end;
			df_size_calc_stripe_info(p_stripe_info, stripe_end, stripe_start, stripe_cnt, overlap, input_align, i, width);

			if (p_stripe_info->stripe_width[i] > MAX_STRIPE_WIDTH) {
				stripe_cnt++;
				is_again_calc = 1;
				printk("Need to increase number of stripes\n");
				break;
			}
		}
	} while (is_again_calc);

	return CPE_FW_OK;
}

void df_size_split_stripe(unsigned int constrain_cnt, df_size_constrain_t *p_size_constrain,
						  isp_stripe_info_t *p_stripe_info, unsigned int overlap, unsigned int width)
{
	unsigned int input_align = 16;
	unsigned int active_stripe_width = UNSIGNED_INT_MAX;
	unsigned int max_in_stripe_align = 0;
	unsigned int max_frame_width     = 0;
	unsigned int stripe_width        = 0;
	df_size_get_max_constrain(p_size_constrain, &active_stripe_width, constrain_cnt, &max_in_stripe_align, &max_frame_width,
							  width);

	// aligning boundAR on the pixel limit to get rid of possible rounding issue
	if (active_stripe_width != UNSIGNED_INT_MAX && active_stripe_width % (1 << 16))
		active_stripe_width = ((active_stripe_width >> 16) + 1) << 16;

	// dn_ar is the max ar of coarsest scaling device that fits in bound_ar
	if (0 == max_in_stripe_align) {
		printk("max_in_stripe_align is zero \n");
		return ;
	}

	unsigned int tmp_frame_width = max_in_stripe_align * (active_stripe_width / max_in_stripe_align); //lint !e414
	unsigned int stripe_cnt = 0;

	if (tmp_frame_width == 0) {
		printk("the most downscaling imposes a granularity that is larger than the maximum crop region of the most upsclaing, no solution possible\n");
		return;
	}

	if ((width <= MAX_STRIPE_WIDTH) && (max_frame_width <= tmp_frame_width)) {
		stripe_cnt = 1;
	} else if ((width <= 2 * (MAX_STRIPE_WIDTH - overlap)) && (max_frame_width <= 2 * tmp_frame_width)) {
		stripe_cnt = 2;
	} else {
		stripe_width = df_size_calc_stripe_width(active_stripe_width, input_align, overlap);

		if (stripe_width > MAX_STRIPE_WIDTH) {
			stripe_width = MAX_STRIPE_WIDTH;
			active_stripe_width = (stripe_width - overlap * 2) << 16;
		}

		if (max_in_stripe_align > active_stripe_width)
			printk("The most downscaler imposes an active region superior to the maximum allowable\n");

		active_stripe_width = (active_stripe_width / max_in_stripe_align) * max_in_stripe_align;  //lint !e414
		stripe_cnt = max_frame_width / active_stripe_width;                                       //lint !e414

		if (max_frame_width % active_stripe_width)   //lint !e414
			stripe_cnt++;
	}

	df_size_do_split_stripe(p_stripe_info, p_size_constrain, constrain_cnt, stripe_cnt, max_in_stripe_align, overlap, width,
							input_align);
}

int seg_src_set_cmdlst_wr_buf(cmd_buf_t *cmd_buf, unsigned int wr_addr, unsigned int rd_addr, unsigned int data_size)
{
	unsigned int line_shart_wstrb = CVDR_ALIGN_BYTES - wr_addr % CVDR_ALIGN_BYTES - 1;
	wr_addr = ipp_align_down(wr_addr, CVDR_ALIGN_BYTES);
	cfg_tab_cvdr_t cmdlst_wr_table;
	loge_if(memset_s(&cmdlst_wr_table, sizeof(cfg_tab_cvdr_t), 0, sizeof(cfg_tab_cvdr_t)));
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].to_use          = 1;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].id              = WR_CMDLST;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.fs_addr     = wr_addr;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.pix_fmt     = DF_D32;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.pix_expan   = 1;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.last_page   = (wr_addr + data_size) >> 15;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.line_stride = 0;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.line_shart_wstrb = line_shart_wstrb;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].fmt.line_wrap   = DEFAULT_LINE_WRAP;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter0  = 0xF;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter1  = 0xF;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter2  = 0xF;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter3  = 0xF;
	cmdlst_wr_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter_reload = 0xF;
	cvdr_prepare_cmd(&g_cvdr_devs[0], cmd_buf, &cmdlst_wr_table);
	unsigned int i = 0;
	unsigned int reg_size = 0;

	for (i = 0; i < (data_size / CMDLST_BURST_MAX_SIZE + 1); i++) {
		reg_size = ((data_size - i * CMDLST_BURST_MAX_SIZE) > CMDLST_BURST_MAX_SIZE) ?
				   CMDLST_BURST_MAX_SIZE : (data_size - i * CMDLST_BURST_MAX_SIZE);
		cmdlst_set_reg_incr(cmd_buf, rd_addr + 4 * i * CMDLST_BURST_MAX_SIZE, reg_size, 0, 1);
	}

	for (i = 0; i < 1; i++) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = (0xff1ffffd);
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}

	return CPE_FW_OK;
}

void ipp_update_cmdlst_cfg_tab(cmdlst_para_t *cmdlst_para)
{
	unsigned int i = 0;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;

	for (i = 0; i < cmdlst_para->stripe_cnt; i++) {
		cmd_link_entry[i].cmdlst_cfg_tab.cfg.to_use = 1;
		cmd_link_entry[i].cmdlst_cfg_tab.cfg.prefetch = 0;
		cmd_link_entry[i].cmdlst_cfg_tab.cfg.slowdown_nrt_channel = 0;
		cmd_link_entry[i].cmdlst_cfg_tab.ch_cfg.to_use = 1;
		cmd_link_entry[i].cmdlst_cfg_tab.ch_cfg.nrt_channel = 1;
		cmd_link_entry[i].cmdlst_cfg_tab.ch_cfg.active_token_nbr_en = 0;
		cmd_link_entry[i].cmdlst_cfg_tab.ch_cfg.active_token_nbr = 0;
		cmd_link_entry[i].cmdlst_cfg_tab.sw_ch_mngr_cfg.to_use = 1;
		cmd_link_entry[i].cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_link_channel = cmdlst_para->cmd_stripe_info[i].ch_link;
		cmd_link_entry[i].cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_link_token_nbr = cmdlst_para->cmd_stripe_info[i].ch_link_act_nbr;
		cmd_link_entry[i].cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_ressource_request = cmdlst_para->cmd_stripe_info[i].resource_share;
		cmd_link_entry[i].cmdlst_cfg_tab.sw_ch_mngr_cfg.sw_priority = cmdlst_para->cmd_stripe_info[i].hw_priority;
		cmd_link_entry[i].cmdlst_cfg_tab.vp_rd_cfg.to_use = 1;
		cmd_link_entry[i].cmdlst_cfg_tab.vp_rd_cfg.vp_rd_id = cmdlst_para->channel_id;
		cmd_link_entry[i].cmdlst_cfg_tab.vp_rd_cfg.rd_addr  = cmd_link_entry[i].cmd_buf.start_addr_isp_map;
		cmd_link_entry[i].cmdlst_cfg_tab.vp_rd_cfg.rd_size  = cmd_link_entry[i].cmd_buf.data_size;
	}
}

void cpe_set_cmdlst_stripe(cmdlst_stripe_info_t *stripe, unsigned int stripe_cnt)
{
	unsigned int i = 0;

	for (i = 0; i < stripe_cnt; i++) {
		if (i == stripe_cnt - 1)
			stripe[i].is_last_stripe   = 1;
		else
			stripe[i].is_last_stripe   = 0;
	}
}


#pragma GCC diagnostic pop
