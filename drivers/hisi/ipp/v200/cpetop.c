
//#include "hispcpe_sub_ctrl_drv.h"
#include "sub_ctrl_drv_priv.h"
#include "sub_ctrl_reg_offset.h"
#include "sub_ctrl_reg_offset_field.h"
#include "ipp_top_drv.h"
#include "mcf_drv.h"
#include <linux/printk.h>

#define LOG_TAG LOG_MODULE_TOP_DRV

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static cpe_top_ops_t cpe_top_ops =
{
    .prepare_cmd   = cpe_top_prepare_cmd,
};

cpe_top_dev_t g_cpe_top_devs[] =
{
    [0] =
    {
        .base_addr = JPG_TOP_ADDR,
        .ops = (struct _mcf_ops_t *)&cpe_top_ops,
    },
};

/**********************************************************
 * function name: cpe_top_mem_config
 *
 * description:
 *    Config para for cpe_top mem config
 *
 * input:
 *    dev  : cpe_top device
 *    para : para for cpe_top mem config
 *
 * output:
 *        0 : success
 *       -1 : fail
 ***********************************************************/
static int cpe_top_mem_config(cpe_top_dev_t* dev, cpe_top_mem_cfg_t* cpe_top_mem_cfg)
{
    pr_info("[%s] +\n", __func__);
    U_CPE_MEM_CFG0  temp_cpe_mem_cfg0;
    U_CPE_MEM_CFG1  temp_cpe_mem_cfg1;
    U_CPE_MEM_CFG2  temp_cpe_mem_cfg2;
    temp_cpe_mem_cfg0.u32 = 0;
    temp_cpe_mem_cfg1.u32 = 0;
    temp_cpe_mem_cfg2.u32 = 0;

    temp_cpe_mem_cfg0.bits.common_mem_ctrl_sp   = cpe_top_mem_cfg->mem_cfg0.common_mem_ctrl_sp;
    temp_cpe_mem_cfg1.bits.mcf_pwr_mem_ctrl_sp  = cpe_top_mem_cfg->mem_cfg1.mcf_pwr_mem_ctrl_sp;
    temp_cpe_mem_cfg2.bits.mfnr_pwr_mem_ctrl_sp = cpe_top_mem_cfg->mem_cfg2.mfnr_pwr_mem_ctrl_sp;

    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CPE_MEM_CFG0_REG , temp_cpe_mem_cfg0.u32);
    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CPE_MEM_CFG1_REG , temp_cpe_mem_cfg1.u32);
    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CPE_MEM_CFG2_REG , temp_cpe_mem_cfg2.u32);
    pr_info("[%s] -\n", __func__);
    return CPE_FW_OK;
 }

/**********************************************************
 * function name: cpe_top_crop_vpwr_config
 *
 * description:
 *    Config para for cpe_top crop_vpwr config
 *
 * input:
 *    dev  : cpe_top device
 *    para : para for cpe_top crop_vpwr config
 *
 * output:
 *        0 : success
 *       -1 : fail
 ***********************************************************/
static int cpe_top_crop_vpwr_config(cpe_top_dev_t* dev, cpe_top_crop_vpwr_t* cpe_top_crop_vpwr)
{
    pr_info("[%s] +\n", __func__);
    U_CROP_VPWR_0  temp_crop_vpwr_0;
    U_CROP_VPWR_1  temp_crop_vpwr_1;
    U_CROP_VPWR_2  temp_crop_vpwr_2;
    temp_crop_vpwr_0.u32 = 0;
    temp_crop_vpwr_1.u32 = 0;
    temp_crop_vpwr_2.u32 = 0;

    temp_crop_vpwr_0.bits.cpe_vpwr0_ihleft   = cpe_top_crop_vpwr->crop_vpwr_0.vpwr_ihleft;
    temp_crop_vpwr_0.bits.cpe_vpwr0_ihright  = cpe_top_crop_vpwr->crop_vpwr_0.vpwr_ihright - 1;
    temp_crop_vpwr_1.bits.cpe_vpwr1_ihleft   = cpe_top_crop_vpwr->crop_vpwr_1.vpwr_ihleft;
    temp_crop_vpwr_1.bits.cpe_vpwr1_ihright  = cpe_top_crop_vpwr->crop_vpwr_1.vpwr_ihright - 1;
    temp_crop_vpwr_2.bits.cpe_vpwr2_ihleft   = cpe_top_crop_vpwr->crop_vpwr_2.vpwr_ihleft;
    temp_crop_vpwr_2.bits.cpe_vpwr2_ihright  = cpe_top_crop_vpwr->crop_vpwr_2.vpwr_ihright - 1;

    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CROP_VPWR_0_REG , temp_crop_vpwr_0.u32);
    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CROP_VPWR_1_REG , temp_crop_vpwr_1.u32);
    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CROP_VPWR_2_REG , temp_crop_vpwr_2.u32);
    pr_info("[%s] -\n", __func__);
    return CPE_FW_OK;
 }

/**********************************************************
 * function name: cpe_top_mode_config
 *
 * description:
 *    Config para for cpe_top mode config
 *
 * input:
 *    dev  : cpe_top device
 *    para : para for cpe_top mode config
 *
 * output:
 *        0 : success
 *       -1 : fail
 ***********************************************************/
static int cpe_top_mode_config(cpe_top_dev_t* dev, cpe_top_mode_cfg_t* cpe_top_mode_cfg)
{
    U_CPE_mode_CFG  temp_cpe_mode_cfg;
    temp_cpe_mode_cfg.u32 = 0;
    temp_cpe_mode_cfg.bits.cpe_op_mode  = cpe_top_mode_cfg->cpe_op_mode;
//    CMDLST_SET_REG(dev->base_addr + SUB_CTRL_CPE_MODE_CFG_REG , cpe_top_mode_cfg->u32);

    return CPE_FW_OK;
 }


/**********************************************************
 * function name: cpe_top_do_config
 *
 * description:
 *    Config cpe_top register
 *
 * input:
 *            dev  : cpe_top device
 *         cfg_tab : all cpe_top config register
 *
 * output:
 *        0 : success
 *       -1 : fail
 ***********************************************************/
static int cpe_top_do_config(cpe_top_dev_t* dev, cpe_top_config_table_t* cfg_tab)
{
/*    if(dev == NULL || cfg_tab == NULL)
    {
        return CPE_FW_ERR;
    }*/

    if(1 == cfg_tab->to_use)
    {
        cfg_tab->to_use = 0;

        if(1 == cfg_tab->mode_cfg.to_use)
        {
            cfg_tab->mode_cfg.to_use = 0;
            cpe_top_mode_config(dev,&cfg_tab->mode_cfg);
        }

        if(1 == cfg_tab->mem_cfg.to_use)
        {
            cfg_tab->mem_cfg.to_use = 0;
            cpe_top_mem_config(dev,&cfg_tab->mem_cfg);
        }

        if(1 == cfg_tab->crop_vpwr.to_use)
        {
            cfg_tab->crop_vpwr.to_use = 0;
            cpe_top_crop_vpwr_config(dev,&cfg_tab->crop_vpwr);
        }
    }
        
    return CPE_FW_OK;
}


int cpe_top_prepare_cmd(cpe_top_dev_t * dev, cmd_buf_t * cmd_buf, cpe_top_config_table_t * table)
{
    dev->cmd_buf = cmd_buf;

    loge_if_ret(cpe_top_do_config(dev, table));

    return 0;
}

#pragma GCC diagnostic pop

/********************************** END **********************************/



