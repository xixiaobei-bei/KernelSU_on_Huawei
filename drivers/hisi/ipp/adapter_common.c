

/*lint -e750
-esym(750,*)*/
/*lint -e40 -e64 -e826 -e838 -e515 -e516 -e613 -e528 -e78 -e530 -e835 -e737 -e701 -e834 -e705
-e713 -e774 -e413 -e715 -e732
-esym(40,*) -esym(64,*) -esym(826,*) -esym(838,*) -esym(515,*) -esym(516,*) -esym(613,*) -esym(528,*) -esym(78,*)
-esym(530,*) -esym(835,*) -esym(737,*) -esym(701,*) -esym(834,*) -esym(705,*) -esym(713,*) -esym(774,*) -esym(413,*)
-esym(715,*) -esym(732,*)*/

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

#define CMDLST_STRIPE_DATA_SIZE    (0x1000)

unsigned int frame_num = 0;
extern cmdlst_dev_t g_cmdlst_devs[];
extern cvdr_dev_t g_cvdr_devs[];
unsigned int g_cmdlst_list_is_init = 0;
unsigned int g_mfnr_read_stat_flag = 0;

extern int memset_s(void *dest, size_t destMax, int c, size_t count);

typedef struct _cmdlst_channel_t
{
    struct list_head ready_list;
}cmdlst_channel_t;

typedef struct _cmdlst_priv_t
{
    cmdlst_channel_t    cmdlst_chan[CMDLST_CHANNEL_MAX];
} cmdlst_priv_t;

static cmdlst_priv_t g_cmdlst_priv;

static int cmdlst_enqueue(unsigned int channel_id, cmdlst_para_t * cmdlst_para);

static int cmdlst_start(cfg_tab_cmdlst_t* cmdlst_cfg, unsigned int channel_id);

static int cmdlst_set_buffer_header(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t* cmdlst_para);

static int cmdlst_set_irq_mode(cpe_cmdlst_irq_mode irq_mode, cmd_buf_t *cmd_buf, unsigned int cpu_enable);

static int cmdlst_set_vpwr(cmd_buf_t *cmd_buf, cmdlst_rd_cfg_info_t* rd_cfg_info);
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
    if ((cmd_buf->data_size + 8) <= cmd_buf->buffer_size)
    {
        *(unsigned int *)(cmd_buf->data_addr) = ((reg) & 0x000fffff);
        cmd_buf->data_addr += 4;
        cmd_buf->data_size += 4;
        *(unsigned int *)(cmd_buf->data_addr) = (val);
        cmd_buf->data_addr += 4;
        cmd_buf->data_size += 4;
    }
    else
    {
        pr_err("cmdlst buffer is full: %u, @0x%x", cmd_buf->data_size, reg);
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

void cmdlst_update_buffer_header(cmd_buf_t *cmd_buf, unsigned int channel_id)
{
    D("[%s] +\n", __func__);

    unsigned int next_hw_prio = 0;
    unsigned int next_hw_resource = 0;
    unsigned int next_hw_token_nbr = 0;
    unsigned int next_hw_link_channel = 0;

    cmd_buf_t * next_buf = cmd_buf->next_buffer;
    if(next_buf == NULL)
    {
        unsigned int i  = 0;
        for(i = 0; i < 10; i++)
        {
            *(unsigned int *)(cmd_buf->start_addr + 0x4 * i) = CMDLST_PADDING_DATA; //lint !e647
        }
        //loge_if(os_cache_flush((void*)(cmd_buf->start_addr + 0x0), 32, 1));longhaixu
        return;
    }
    /* ignore 16 bytes of padding data at the beginning */
    *(unsigned int *)(cmd_buf->start_addr + 0x0) = 0x000050D0 + 0x80*channel_id;
    *(unsigned int *)(cmd_buf->start_addr + 0x4) = (next_hw_prio << 31) | (next_hw_resource << 8) | (next_hw_token_nbr << 4) | (next_hw_link_channel);

    *(unsigned int *)(cmd_buf->start_addr + 0x8) = 0x000050D0 + 0x80*channel_id;
    *(unsigned int *)(cmd_buf->start_addr + 0xC) = next_buf->start_addr_isp_map >> 2;

    *(unsigned int *)(cmd_buf->start_addr + 0x10) = 0x000050C4 + 0x80*channel_id;
    *(unsigned int *)(cmd_buf->start_addr + 0x14) = ((next_buf->start_addr_isp_map + next_buf->data_size)
                        & 0xffffe000) | 0x0000012E;

    *(unsigned int *)(cmd_buf->start_addr + 0x18) = 0x000050C8 + 0x80*channel_id;
    *(unsigned int *)(cmd_buf->start_addr + 0x1C) = (next_buf->data_size >> 3) - 1;

    *(unsigned int *)(cmd_buf->start_addr + 0x20) = 0x000050CC + 0x80*channel_id;
    *(unsigned int *)(cmd_buf->start_addr + 0x24) = 0;

    //loge_if(os_cache_flush((void*)(cmd_buf->start_addr + 0x0), 32, 1));
    D("[%s] -\n", __func__);
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
void cmdlst_set_reg_incr(cmd_buf_t *cmd_buf, unsigned int reg, int size, int incr)
{
    D("[%s] +\n", __func__);
    
    if ((cmd_buf->data_size + (size+1)*4) > cmd_buf->buffer_size)
    {
        pr_err("cmdlst buffer is full: %u, @0x%x", cmd_buf->data_size, reg);
        return;
    }

    *(unsigned int *)(cmd_buf->data_addr) = (reg & 0x000ffffc) | ((size-1) << 24) | (incr << 1);
    cmd_buf->data_addr += 4;
    cmd_buf->data_size += 4;
    
    D("[%s] -\n", __func__);
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
    //D("[%s] +\n", __func__);
    
    *(unsigned int *)(cmd_buf->data_addr) = data;
    cmd_buf->data_addr += 4;
    cmd_buf->data_size += 4;

    //D("[%s] -\n", __func__);
}

int df_cmdlst_config_vpwr(cmd_buf_t *cmd_buf, unsigned int stripe_index,unsigned int  fs)
{
    D("[%s] +\n", __func__);
    
    if((0 == stripe_index) || (stripe_index > MAX_STRIPE_NUM))
    {
        pr_err("stripe_index=%d, error", stripe_index);
        return CPE_FW_ERR;
    }

    cfg_tab_cvdr_t cmdlst_w3_1_table;
    memset_s(&cmdlst_w3_1_table,sizeof(cfg_tab_cvdr_t),0,sizeof(cfg_tab_cvdr_t));
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].to_use = 1;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].id     = get_cvdr_wr_port_num(WR_CMDLST);
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.fs_addr= fs;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.pix_fmt = DF_D64;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.pix_expan = 1;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.last_page = (cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.fs_addr+ 0x10000) >> 15;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.line_stride =0;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.line_wrap = 0x3FFF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter0 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter1 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter2 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter3 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter_reload= 0xF;
    loge_if_ret(g_cvdr_devs[0].ops->prepare_cmd(&g_cvdr_devs[0], cmd_buf, &cmdlst_w3_1_table));

    D("[%s] -\n", __func__);
    return CPE_FW_OK;
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
static int cmdlst_set_buffer_header(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t* cmdlst_para)
{
    D("[%s]+",__func__);
    unsigned int idx;
    unsigned int irq_mode   = 0;
    unsigned int cpu_enable = 0;
    schedule_cmdlst_link_t* cmdlst_stripe = (schedule_cmdlst_link_t*)cmdlst_para->cmd_entry;
    cmdlst_rd_cfg_info_t* rd_cfg_info = &cmdlst_para->cmd_stripe_info[stripe_index].rd_cfg_info;
    irq_mode    = cmdlst_stripe[stripe_index].irq_mode;
    cpu_enable  = cmdlst_para->cmd_stripe_info[stripe_index].is_last_stripe ? 3 : 1;

    /* reserve 10 words to update next buffer later */
    for (idx = 0; idx < 10; idx++)
    {
        *(unsigned int *)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
        cmd_buf->data_addr += 4;
        cmd_buf->data_size += 4;
    }
    
    cmdlst_set_irq_mode(irq_mode, cmd_buf, cpu_enable);
    pr_info("stripe_cnt =%d, fs =0x%x \n", cmdlst_para->stripe_cnt, rd_cfg_info->fs);
    if(0 != rd_cfg_info->fs)
    {
        cmdlst_set_vpwr(cmd_buf, rd_cfg_info);

        for(idx = 0; idx < rd_cfg_info->rd_cfg_num; idx++)
        {
            *(unsigned int *)(cmd_buf->data_addr) = rd_cfg_info->rd_cfg[idx];
            cmd_buf->data_addr += 4;
            cmd_buf->data_size += 4;
        }
    } 

    while (cmd_buf->data_size < CMDLST_HEADER_SIZE)
    {
        *(unsigned int *)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
        cmd_buf->data_addr += 4;
        cmd_buf->data_size += 4;
    }
    D("[%s]-",__func__);
    return CPE_FW_OK;
}


/**********************************************************
 * function name: cmdlst_set_irq_mode
 *
 * description:
 *               set irq mode according frame type.
 * input:
 *     cmd_buf      : current cmdlst buf
 *     irq          : irq mode.
 *     cpu_enable   : 1 to hardware,3to hardware and cpu
 * return:
 *      0;
 ***********************************************************/
static int cmdlst_set_irq_mode(cpe_cmdlst_irq_mode irq_mode, cmd_buf_t *cmd_buf, unsigned int cpu_enable)
{
    D("[%s] +\n", __func__);

    switch(irq_mode)
    {
        case CMD_IRQ_MCF_MFNR_FULL_MODE:
        case CMD_IRQ_MCF_FULL_MODE_TWO_INPUT:
        case CMD_IRQ_MCF_FULL_MODE_ONE_INPUT:
        case CMD_IRQ_MCF_PARTIAL_MODE:
        case CMD_IRQ_MCF_YGF_SINGLE_MODE:
        case CMD_IRQ_MCF_YGF_DUAL_MODE:
        case CMD_IRQ_MCF_YGF_DUAL_MODE_LF_ONLY:
        case CMD_IRQ_MCF_YGF_SINGLE_MODE_LF_ONLY:
        case CMD_IRQ_MFNR_FULL_COLOR_NO_MASK_MODE:
        case CMD_IRQ_MFNR_FULL_MONO_MODE:
        case CMD_IRQ_MFNR_FULL_MONO_NO_MASK_MODE:
        case CMD_IRQ_MFNR_PARTIAL_COLOR_MODE:
        case CMD_IRQ_MFNR_PARTIAL_MONO_MODE:

            {
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_IRQ1_REG0_REG, 0x1C710FC0);//clean all eof
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_COMP_IRQ_REG2_REG, cpu_enable);
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_COMP_IRQ_REG_REG, irq_mode);
                break;
            }
        case CMD_IRQ_MFNR_READ_STAT_MODE:

            {
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_IRQ1_REG0_REG, 0x2000);
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_COMP_IRQ_REG2_REG, cpu_enable);
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_COMP_IRQ_REG_REG, irq_mode);
                break;
            }
        case CMD_IRQ_SLAM_SCOREMAP_MODE:
        case CMD_IRQ_SLAM_PYRAMID_MODE:
        case CMD_IRQ_SLAM_NORMAL_MODE:
            {
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CVDR_IRQ_REG0_REG, 0x1D240);//clean all eof
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+0x194, cpu_enable);
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+0x190, irq_mode);
                break;
            }
        case CMD_IRQ_SLAM_READ_STAT_MODE:
            {
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+SUB_CTRL_CPE_IRQ1_REG0_REG, 0x2000);//clean all eof
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+0x194, cpu_enable);
                cmdlst_set_reg(cmd_buf, JPG_TOP_OFFSET+0x190, irq_mode);
                break;
            }
        default:
            pr_err("err irq mode %d",irq_mode);
    }

 
    D("[%s] -\n", __func__);
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
static int cmdlst_set_vpwr(cmd_buf_t *cmd_buf, cmdlst_rd_cfg_info_t* rd_cfg_info)
{
    D("[%s] +\n", __func__);
    
    cfg_tab_cvdr_t cmdlst_w3_1_table;

    if(NULL == cmd_buf)
    {
        pr_err("cmdlst buf is null!");

        return CPE_FW_ERR;
    }

    if(NULL == rd_cfg_info)
    {
        pr_err("cmdlst read cfg info is null!");

        return CPE_FW_ERR;
    }

    memset_s(&cmdlst_w3_1_table, sizeof(cfg_tab_cvdr_t),0, sizeof(cfg_tab_cvdr_t));

    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].to_use = 1;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].id = get_cvdr_wr_port_num(WR_CMDLST);
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.fs_addr = rd_cfg_info->fs;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.pix_fmt = DF_D64;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.pix_expan = 1;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.last_page = (rd_cfg_info->fs + CMDLST_32K_PAGE - 1) >> 15;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.line_stride = 0;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].fmt.line_wrap = 0x3FFF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter0 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter1 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter2 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter3 = 0xF;
    cmdlst_w3_1_table.vp_wr_cfg[WR_CMDLST].bw.bw_limiter_reload= 0xF;

    loge_if_ret(g_cvdr_devs[0].ops->prepare_cmd(&g_cvdr_devs[0], cmd_buf, &cmdlst_w3_1_table));
    D("[%s] -\n", __func__);

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
    D("[%s] +\n", __func__);
    
    /* Padding two words for D64 */
    *(unsigned int *)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
    cmd_buf->data_addr += 4;
    cmd_buf->data_size += 4;
    *(unsigned int *)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
    cmd_buf->data_addr += 4;
    cmd_buf->data_size += 4;
    //loge_if_ret(os_cache_flush((void*)cmd_buf->start_addr, cmd_buf->data_size, 1));

    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

static int cmdlst_set_branch(unsigned int channel_id)
{
    D("[%s] +\n", __func__);
    
    cmdlst_state_t st;
    st.ch_id = channel_id;
    loge_if_ret(g_cmdlst_devs[0].ops->set_branch(&g_cmdlst_devs[0], st.ch_id));
    udelay(1);/*lint !e778 !e774 !e747*/
    
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

/**********************************************************
 * function name: cmdlst_eop_handler
 *
 * description:
 *              eop handler to dequeue a done-frame
 * input:
 *     NULL
 * return:
 *      0;
 ***********************************************************/
int cmdlst_eop_handler(cmdlst_eof_mode_e mode)
{
    D("[%s] +\n", __func__);

    unsigned int channel_id = 0;
    if(mode == CMD_EOF_SLAM_MODE || mode == CMD_EOF_SLAMTEST_MODE)
    {
        channel_id = 1;
    }
    struct list_head *ready_list = NULL;
    ready_list = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;

    schedule_cmdlst_link_t *cur_cmdlst_link = container_of(ready_list->next, schedule_cmdlst_link_t, list_link);
    while(!list_empty(ready_list))
    {
        cur_cmdlst_link = container_of(ready_list->next, schedule_cmdlst_link_t, list_link);
        list_del(ready_list->next);
    }
    if(0 == channel_id)
    {
        cpe_mem_free(MEM_ID_CMDLST_BUF_MM);
        cpe_mem_free(MEM_ID_CMDLST_ENTRY_MM);
        cpe_mem_free(MEM_ID_CMDLST_PARA_MM);
    }
    else if(1 == channel_id)
    {
        cpe_mem_free(MEM_ID_CMDLST_BUF_SLAM);
        cpe_mem_free(MEM_ID_CMDLST_ENTRY_SLAM);
        cpe_mem_free(MEM_ID_CMDLST_PARA_SLAM);
    }


    D("[%s] -\n", __func__);
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
static int cmdlst_enqueue(unsigned int channel_id,cmdlst_para_t * cmdlst_para)
{
    D("[%s] +\n", __func__);
    
    struct list_head *ready_list = NULL;

    ready_list       = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;

    struct list_head *dump_list = NULL;
    for(dump_list = ready_list->next;dump_list != ready_list;dump_list = dump_list->next)
    {
        pr_info("list[0x%llx]",(unsigned long long) dump_list);     //lint !e559
    }
    schedule_cmdlst_link_t * pos = NULL;
    schedule_cmdlst_link_t * n = NULL;

    list_for_each_entry_safe(pos, n, ready_list, list_link)
    {
        if(pos->list_link.next != ready_list)
        {
            pos->cmd_buf.next_buffer = (void*)&n->cmd_buf;
            cmdlst_update_buffer_header(&pos->cmd_buf, channel_id);
        }
    }

    D("[%s] -\n", __func__);
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
static int cmdlst_start(cfg_tab_cmdlst_t* cmdlst_cfg, unsigned int channel_id)
{
    D("[%s] +\n", __func__);
    
    struct list_head *cmdlst_insert_queue = NULL;

    cmdlst_insert_queue       = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;

    if (NULL != cmdlst_cfg)
    {
        cmdlst_do_config(&g_cmdlst_devs[0], cmdlst_cfg);
    }

    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}
int df_sched_set_buffer_header(cmdlst_para_t * cmdlst_para)
{
    D("[%s] +\n", __func__);
    unsigned int i = 0;
    schedule_cmdlst_link_t* cmdlst_stripe = (schedule_cmdlst_link_t*)cmdlst_para->cmd_entry;
    for (i = 0; i < cmdlst_para->stripe_cnt; i++)
    {
        cmdlst_set_buffer_header(i, &cmdlst_stripe[i].cmd_buf, cmdlst_para);
    }
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}


int df_sched_prepare(cmdlst_para_t * cmdlst_para)
{
    D("[%s] +\n", __func__);
    
    unsigned int i;
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = 0;

    if(g_cmdlst_list_is_init == 0)
    {
        for(i = 0;i < CMDLST_CHANNEL_MAX;i++)
        {
           INIT_LIST_HEAD(&g_cmdlst_priv.cmdlst_chan[i].ready_list);
        }
        g_cmdlst_list_is_init = 1;
    }

    if(!list_empty(&g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list))
    {
        pr_err("ready list not clean out");
    }
    schedule_cmdlst_link_t* new_entry = NULL;
    if(MM_CHANNEL == cmdlst_para->channel_id)
    {
        if ((ret = cpe_mem_get(MEM_ID_CMDLST_ENTRY_MM, &va, &da)) != 0) {
            pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_ENTRY_MM, va, da, ret);
            return CPE_FW_ERR;
        }
        new_entry = (schedule_cmdlst_link_t*)va;
        if ((ret = cpe_mem_get(MEM_ID_CMDLST_BUF_MM, &va, &da)) != 0) {
            pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_BUF_MM, va, da, ret);
            return CPE_FW_ERR;
        }
    }
    else if(SLAM_CHANNEL == cmdlst_para->channel_id)
    {
        if ((ret = cpe_mem_get(MEM_ID_CMDLST_ENTRY_SLAM, &va, &da)) != 0) {
            pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_ENTRY_SLAM, va, da, ret);
            return CPE_FW_ERR;
        }
        new_entry = (schedule_cmdlst_link_t*)va;

        if ((ret = cpe_mem_get(MEM_ID_CMDLST_BUF_SLAM, &va, &da)) != 0) {
            pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_BUF_SLAM, va, da, ret);
            return CPE_FW_ERR;
        }
    }

    if (NULL == new_entry)
    {
        pr_err("fail to memory_alloc new entry!");
        return CPE_FW_ERR;
    }
   
    memset_s(new_entry,cmdlst_para->stripe_cnt*sizeof(schedule_cmdlst_link_t),0,cmdlst_para->stripe_cnt*sizeof(schedule_cmdlst_link_t));
    cmdlst_para->cmd_entry = (void*)new_entry;

    new_entry[0].cmd_buf.start_addr  = va;
    new_entry[0].cmd_buf.start_addr_isp_map = da;
    for (i = 0; i < cmdlst_para->stripe_cnt; i++)
    {
        pr_info("cmdlst_para->stripe_cnt =%d, i=%d \n", cmdlst_para->stripe_cnt, i);
        new_entry[i].data = (void*)cmdlst_para;
        list_add_tail(&new_entry[i].list_link, &g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list);
        new_entry[i].cmd_buf.start_addr  = new_entry[0].cmd_buf.start_addr + (unsigned int) (CMDLST_BUFFER_SIZE * i); //lint !e647
        new_entry[i].cmd_buf.start_addr_isp_map = new_entry[0].cmd_buf.start_addr_isp_map + CMDLST_BUFFER_SIZE * i;
        if (0 == new_entry[i].cmd_buf.start_addr)
        {
            pr_err("fail to get cmdlist buffer!");
            return CPE_FW_ERR;
        }
        new_entry[i].cmd_buf.buffer_size = CMDLST_BUFFER_SIZE;
        new_entry[i].cmd_buf.header_size = CMDLST_HEADER_SIZE;
        new_entry[i].cmd_buf.data_addr   = new_entry[i].cmd_buf.start_addr;
        pr_info("buffer_size=%d, data_addr=0x%llx\n",new_entry[i].cmd_buf.buffer_size, new_entry[i].cmd_buf.data_addr);

        new_entry[i].cmd_buf.data_size   = 0;
        new_entry[i].cmd_buf.next_buffer = NULL;
    }

    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

int df_sched_start(cfg_tab_cmdlst_t* cmdlst_cfg, cmdlst_para_t * cmdlst_para)
{
    D("[%s] +\n", __func__);
    
    unsigned int channel_id    = cmdlst_para->channel_id;

    loge_if_ret(cmdlst_set_branch(channel_id));
    loge_if_ret(cmdlst_enqueue(channel_id, cmdlst_para));

    schedule_cmdlst_link_t* cmd_link_entry = NULL;
    list_for_each_entry(cmd_link_entry, &g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list, list_link)
    {
      //  cmdlst_buff_dump(&cmd_link_entry->cmd_buf);
    }
    

    if(cmdlst_cfg)
    {
        loge_if_ret(cmdlst_start(cmdlst_cfg, channel_id));
    }

    frame_num++;
    pr_info("channel[%d] start to work",channel_id);
    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

void dump_addr(unsigned long long addr, int num, char *info)
{
    #define DATA_PERLINE    (16)
    int i = 0;
    D("%s Dump ......\n", info);
    for (i = 0; i < num; i += DATA_PERLINE)
        pr_info("0x%llx : 0x%08x 0x%08x 0x%08x 0x%08x\n", addr + i,
            readl((volatile void __iomem *)(addr + i + 0x00)),
            readl((volatile void __iomem *)(addr + i + 0x04)),
            readl((volatile void __iomem *)(addr + i + 0x08)),
            readl((volatile void __iomem *)(addr + i + 0x0C)));
}

void cmdlst_buff_dump(cmd_buf_t* cmd_buf)
{
    D("[%s] +\n", __func__);
    
    pr_info("CMDLST BUFF: Size:0x%x   Addr: 0x%llx\n", cmd_buf->data_size, cmd_buf->start_addr);
    dump_addr(cmd_buf->start_addr, cmd_buf->data_size, "cmdlst_buff");
    
    D("[%s] -\n", __func__);
}

void cmdlst_set_reg_by_cpu(unsigned int reg, unsigned int val)
{
    unsigned int temp_reg;
    temp_reg = ((reg) & 0x000fffff);
    if(temp_reg >= 0x4000 && temp_reg < 0x5000)
    {
        hispcpe_reg_set(CPE_TOP, ((temp_reg) & 0x00000fff),val);
    }
    else if(temp_reg >= 0x5000 && temp_reg < 0x6000)
    {
        hispcpe_reg_set(CMDLIST_REG, ((temp_reg) & 0x00000fff),val);
    }
    else if(temp_reg >= 0x6000 && temp_reg < 0x8000)
    {
        hispcpe_reg_set(CVDR_REG, ((temp_reg) & 0x00000fff),val);
    }
    else if(temp_reg >= 0xa000 && temp_reg < 0xb000)
    {
        hispcpe_reg_set(MCF_REG, ((temp_reg) & 0x00000fff),val);
    }
    else if(temp_reg >= 0xb000 && temp_reg < 0xc000)
    {
        hispcpe_reg_set(MFNR_REG, ((temp_reg) & 0x00000fff),val);
    }
    else if(temp_reg >= 0x20000 && temp_reg < 0x40000)
    {
        hispcpe_reg_set(SMMU_REG, ((temp_reg) & 0x0000ffff),val);
    }
}

static void df_size_dump_w_h(isp_size_t *ds, char *s)
{
    pr_info("%s: width = %u, height = %u", s, ds->width, ds->height);
}

static void df_size_dump_crop_region(crop_region_info_t *cr, char *s)
{
    pr_info("%s: x = %u, y = %u, width = %u, height = %u", s, cr->x, cr->y, cr->width, cr->height);
}

void df_size_dump_stripe_info(isp_stripe_info_t *p_stripe_info, char *s)
{
    unsigned int i = 0;
    char name[LOG_NAME_LEN];
    char type_name[LOG_NAME_LEN];
    if(p_stripe_info->stripe_cnt == 0)
    {
        return;
    }

    memset_s((void *)type_name, LOG_NAME_LEN,0, LOG_NAME_LEN); 
    pr_info("%s stripe_cnt = %d\n", type_name, p_stripe_info->stripe_cnt);

    memset_s((void *)name, sizeof(name),0, sizeof(name));
    df_size_dump_crop_region(&p_stripe_info->crop_region, name);

    memset_s((void *)name,sizeof(name), 0, sizeof(name)); 
    df_size_dump_w_h(&p_stripe_info->pipe_work_size, name);

    memset_s((void *)name, sizeof(name),0, sizeof(name));
    df_size_dump_w_h(&p_stripe_info->full_size, name);

    for(i = 0; i < p_stripe_info->stripe_cnt; i++)
    {
        memset_s((void *)name,sizeof(name), 0, sizeof(name));
        D("%s: stripe_width[%d] = %u, start_point[%d] = %d, end_point[%d] = %d, overlap_left[%d] = %d, overlap_right[%d] = %d\n",
            type_name, i, p_stripe_info->stripe_width[i],
            i, p_stripe_info->stripe_start_point[i], i, p_stripe_info->stripe_end_point[i],
            i, p_stripe_info->overlap_left[i], i, p_stripe_info->overlap_right[i]);
    }
    return;
}

static unsigned int df_size_calc_stripe_width(unsigned int active_stripe_width, unsigned int input_align,unsigned int overlap)
{
    if (active_stripe_width == UNSIGNED_INT_MAX)
    {
        return UNSIGNED_INT_MAX;
    }
    unsigned int tmp_active_stripe_width = active_stripe_width / (1<<16);
    if (active_stripe_width % (1 << 16))
    {
        tmp_active_stripe_width++;
    }
    unsigned int stripe_width = tmp_active_stripe_width + overlap*2;
    stripe_width = (stripe_width / input_align) * input_align;
    if (stripe_width % input_align)
    {
        stripe_width += input_align;
    }
    return stripe_width;
}

void df_size_split_stripe(unsigned int constrain_cnt, df_size_constrain_t* p_size_constrain,
 isp_stripe_info_t* p_stripe_info,unsigned int overlap, unsigned int width)
{
    D("[%s] +\n", __func__);
    unsigned int input_align = 16;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int  in_full_width = width;
    unsigned int active_stripe_width = UNSIGNED_INT_MAX;
    unsigned int max_in_stripe_align = 0;
    unsigned int max_frame_width     = 0;
    unsigned int stripe_width        = 0;

    for(i = 0; i < constrain_cnt; i++)
    {
        if (p_size_constrain[i].out_width == UNSIGNED_INT_MAX)
            continue;
        unsigned int max_out_width = p_size_constrain[i].pix_align * (p_size_constrain[i].out_width / p_size_constrain[i].pix_align);
        if (p_size_constrain[i].out_width != max_out_width)
        {
            p_size_constrain[i].out_width = max_out_width;
        }

        unsigned int tmp_input_width = max_out_width * p_size_constrain[i].hinc;
        if (tmp_input_width < active_stripe_width)
        {
            active_stripe_width = tmp_input_width;
        }
        if (p_size_constrain[i].hinc * p_size_constrain[i].pix_align > max_in_stripe_align)
        {
            max_in_stripe_align = p_size_constrain[i].hinc * p_size_constrain[i].pix_align;
            max_frame_width = p_size_constrain[i].hinc * ((unsigned int) (in_full_width << 16) / p_size_constrain[i].hinc);
        }
    }

    // aligning boundAR on the pixel limit to get rid of possible rounding issue
    if(active_stripe_width != UNSIGNED_INT_MAX && active_stripe_width%(1<<16))
    {
        active_stripe_width = ((active_stripe_width>>16)+1)<<16;
    }

    // dn_ar is the max ar of coarsest scaling device that fits in bound_ar
    if (0 == max_in_stripe_align )
    {
        printk("max_in_stripe_align is zero \n");
        return ;
    }

    unsigned int tmp_frame_width = max_in_stripe_align * (active_stripe_width / max_in_stripe_align); //lint !e414
    unsigned int stripe_cnt = 0;
    if (tmp_frame_width == 0)
    {
        printk("the most downscaling imposes a granularity that is larger than the maximum crop region of the most upsclaing, no solution possible\n");
        return;
    }

    if ((in_full_width <= MAX_STRIPE_WIDTH) && (max_frame_width <= tmp_frame_width))
    {
        stripe_cnt = 1;
    }
    else if ((in_full_width <= 2 * (MAX_STRIPE_WIDTH - overlap)) && (max_frame_width <= 2 * tmp_frame_width))
    {
        stripe_cnt = 2;
    }
    else
    {
        stripe_width = df_size_calc_stripe_width(active_stripe_width, input_align, overlap);

        if (stripe_width > MAX_STRIPE_WIDTH)
        {
            stripe_width = MAX_STRIPE_WIDTH;
            active_stripe_width = (stripe_width - overlap*2)<<16;
        }
        if (max_in_stripe_align > active_stripe_width)
        {
            printk("The most downscaler imposes an active region superior to the maximum allowable\n");
        }
        active_stripe_width = (active_stripe_width / max_in_stripe_align) * max_in_stripe_align;  //lint !e414
        stripe_cnt = max_frame_width / active_stripe_width;                                       //lint !e414
        if (max_frame_width % active_stripe_width)//lint !e414
        {
            stripe_cnt++;
        }
    }

    unsigned int is_again_calc = 0;
    do
    {
        is_again_calc = 0;
        unsigned int tmp_last_end    = 0;
        unsigned int last_stripe_end = 0;
        for (i = 0; i < stripe_cnt; i++)
        {
            unsigned long long int active_stripe_end = ((i+1) * (unsigned long long int)(in_full_width << 16)) / stripe_cnt;  //lint !e647
            if (i != stripe_cnt - 1)
            {
                active_stripe_end = (active_stripe_end / max_in_stripe_align) * max_in_stripe_align; //lint !e414
                if (active_stripe_end <= tmp_last_end)
                {
                    printk("The most downscaler imposes an active region superior to the maximum allowable\n");
                }
                tmp_last_end = active_stripe_end;
            }
            unsigned int stripe_end   = 0;
            unsigned int stripe_start = UNSIGNED_INT_MAX;
            for(j = 0; j != constrain_cnt; j++)
            {
                unsigned int active_pix_align = p_size_constrain[j].pix_align * p_size_constrain[j].hinc;
                unsigned int tmp_stripe_start = (i == 0) ? 0 : last_stripe_end;
                unsigned int tmp_stripe_end   = 0;

                if (tmp_stripe_start < stripe_start)
                {
                    stripe_start = tmp_stripe_start;
                }
                if (i == stripe_cnt-1)
                {
                    tmp_stripe_end = active_stripe_end;
                }
                else
                {
                    tmp_stripe_end = (active_stripe_end / active_pix_align) * active_pix_align;
                }

                if (active_stripe_end - tmp_stripe_end > active_pix_align / 2)
                {
                    tmp_stripe_end += active_pix_align;
                }

                if (tmp_stripe_end > stripe_end)
                {
                    stripe_end = tmp_stripe_end;
                }
            }
            last_stripe_end = stripe_end;

            p_stripe_info->stripe_start_point[i] = stripe_start;
            p_stripe_info->stripe_end_point[i]   = stripe_end;
            p_stripe_info->overlap_left[i]       = (i==0) ? 0 : overlap;
            p_stripe_info->overlap_right[i]      = (i==stripe_cnt-1) ? 0 : overlap;
            p_stripe_info->stripe_cnt            = stripe_cnt;

            unsigned int start_point = p_stripe_info->stripe_start_point[i] - (p_stripe_info->overlap_left[i] << 16);
            p_stripe_info->stripe_start_point[i] = (start_point / (input_align << 16)) * input_align;
            p_stripe_info->overlap_left[i]       = (i==0) ? 0 :
            (p_stripe_info->stripe_end_point[i-1]-p_stripe_info->overlap_right[i-1]-p_stripe_info->stripe_start_point[i]);

            unsigned int end_point   = p_stripe_info->stripe_end_point[i] + (p_stripe_info->overlap_right[i] << 16);
            p_stripe_info->stripe_end_point[i] = (end_point / (input_align << 16)) * input_align;
            if (end_point % (input_align << 16))
            {
                p_stripe_info->stripe_end_point[i] += input_align;
            }
            if(i == stripe_cnt-1)
            {
                p_stripe_info->stripe_end_point[i] = width;
            }
            p_stripe_info->overlap_right[i]    = (i==stripe_cnt-1) ? 0 :
            (p_stripe_info->stripe_end_point[i]-ALIGN_UP(p_stripe_info->stripe_end_point[i]-overlap, CVDR_ALIGN_BYTES/2));

            p_stripe_info->stripe_width[i] = p_stripe_info->stripe_end_point[i] - p_stripe_info->stripe_start_point[i];

            //printk("%d\n",p_stripe_info->stripe_width[i]);
            if (p_stripe_info->stripe_width[i] > MAX_STRIPE_WIDTH)
            {
                stripe_cnt++;
                is_again_calc = 1;
                printk("Need to increase number of stripes\n");
                break;
            }
        }
    }
    while(is_again_calc);
    
    D("[%s] +\n", __func__);
}

#pragma GCC diagnostic pop
