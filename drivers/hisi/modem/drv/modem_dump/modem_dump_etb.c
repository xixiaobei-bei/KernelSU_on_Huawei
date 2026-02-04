/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/delay.h>
#include "modem_dump_etb.h"
#include "bsp_dump_drv.h"
coresight_mem_ctrl_t g_coresight_etb_mem_ctrl;
coresight_mem_ctrl_t g_coresight_etm_mem_ctrl;
modem_sysctrl_cfg_t  g_modem_sysctrl_cfg;
PERCTRL2_REG g_perctrl2_reg = {0,};
u32 g_perctrl_reg_val = 0;

/*****************************************************************************
* 函 数 名  : dump_perctrl2_memmap
* 功能描述  : 映射perctrl2寄存器
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_perctrl2_memmap(void)
{
    if(dump_get_product_type() == DUMP_PHONE)
    {
        char* name = "coresight,extern_dep" ;
        struct device_node* perctrl2_node    = NULL;
        
        unsigned int reg_data[2] = {0,};
        
        perctrl2_node = of_find_compatible_node(NULL, NULL, name);
        if(NULL == perctrl2_node)
        {
            dump_fetal("perctrl2_node error: of_find_compatible_node failed.\r\n");
            return BSP_ERROR;
        }
        
        if(of_property_read_u32_array(perctrl2_node, "reg", reg_data, 2))
        {
                dump_fetal(" cs get dts reg error\n");
                return BSP_ERROR;
        }
        g_perctrl2_reg.phy_addr = reg_data[0];

        g_perctrl2_reg.virt_addr= ioremap(reg_data[0],reg_data[1]);
         
        (void)of_property_read_u32_index(perctrl2_node, "offset", 0,&g_perctrl2_reg.offset);
        
        (void)of_property_read_u32_index(perctrl2_node, "pclkdbg_clkoff_sys", 0,&g_perctrl2_reg.pclkdbg_clkoff_sys);
        
        (void)of_property_read_u32_index(perctrl2_node, "atclkoff_sys", 0,&g_perctrl2_reg.atclkoff_sys);
        
        (void)of_property_read_u32_index(perctrl2_node, "pclkdbg_to_modem_clk_off_sys", 0,&g_perctrl2_reg.pclkdbg_to_modem_clk_off_sys);
        
        (void)of_property_read_u32_index(perctrl2_node, "atclk_to_modem_clkoff_sys", 0,&g_perctrl2_reg.atclk_to_modem_clkoff_sys);
        
        (void)of_property_read_u32_index(perctrl2_node, "modem_cssys_rst_req", 0,&g_perctrl2_reg.modem_cssys_rst_req);
  
        dump_fetal("g_perctrl2_reg.phy_addr = 0x%x,g_perctrl2_reg.virt_addr = 0x%x,g_perctrl2_reg.offset = 0x%x",g_perctrl2_reg.phy_addr ,g_perctrl2_reg.virt_addr,g_perctrl2_reg.offset);
        dump_fetal("g_perctrl2_reg.pclkdbg_clkoff_sys = 0x%x",g_perctrl2_reg.pclkdbg_clkoff_sys);
        dump_fetal("g_perctrl2_reg.atclkoff_sys = 0x%x",g_perctrl2_reg.atclkoff_sys);
        dump_fetal("g_perctrl2_reg.pclkdbg_to_modem_clk_off_sys = 0x%x",g_perctrl2_reg.pclkdbg_to_modem_clk_off_sys);
        dump_fetal("g_perctrl2_reg.modem_cssys_rst_req = 0x%x",g_perctrl2_reg.modem_cssys_rst_req );    
        dump_fetal("g_perctrl2_reg.atclk_to_modem_clkoff_sys = 0x%x",g_perctrl2_reg.atclk_to_modem_clkoff_sys);
    }

    return BSP_OK;

}
/*****************************************************************************
* 函 数 名  : dump_get_perctrl2_status
* 功能描述  : 查询perctrl2寄存器状态
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_get_perctrl2_status(void)
{
    if(dump_get_product_type() == DUMP_PHONE)
    {
        u32 status = 0;
        if(g_perctrl2_reg.virt_addr == NULL)
        {     
            dump_fetal("base addr is  null.\r\n");
        }
        status = readl(g_perctrl2_reg.virt_addr + g_perctrl2_reg.offset);
        g_perctrl_reg_val = status;
        printk("status = 0x%x \n",status);
        if(!BVAL(status, g_perctrl2_reg.pclkdbg_clkoff_sys))
        {
            dump_fetal("g_perctrl2_reg.pclkdbg_clkoff_sys is 0\n");
            return BSP_ERROR;
        }
        if(!BVAL(status, g_perctrl2_reg.atclkoff_sys))
        {
            dump_fetal("g_perctrl2_reg.atclkoff_sys is 0\n");
            return BSP_ERROR;
        }
        if(!BVAL(status, g_perctrl2_reg.pclkdbg_to_modem_clk_off_sys))
        {
            dump_fetal("g_perctrl2_reg.pclkdbg_to_modem_clk_off_sys is 0\n");
            return BSP_ERROR;
        }
        if(!BVAL(status, g_perctrl2_reg.atclk_to_modem_clkoff_sys))
        {
            dump_fetal("g_perctrl2_reg.atclk_to_modem_clkoff_sys is 0\n");
            return BSP_ERROR;
        }
        if(BVAL(status, g_perctrl2_reg.modem_cssys_rst_req))
        {
            dump_fetal("g_perctrl2_reg.modem_cssys_rst_req is 1\n");
            return BSP_ERROR;
        }
    }
    return BSP_OK;

}

/*****************************************************************************
* 函 数 名  : dump_modem_sysctrl_memmap
* 功能描述  : 映射邋sys_ctrl寄存器
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_modem_sysctrl_memmap(void)
{
    if(DUMP_PHONE == dump_get_product_type())
    {
        struct device_node* sysctrl_node = NULL;
        unsigned int reg_data[2] = {0,};
        unsigned int rst_data[3] = {0,};
        char* name = "ap_modem,sysctrl_cfg";
        memset(&g_modem_sysctrl_cfg,'\0',sizeof(g_modem_sysctrl_cfg));
        sysctrl_node = of_find_compatible_node(NULL, NULL, name);
        if(sysctrl_node)
        {
            if(of_property_read_u32_array(sysctrl_node, "reg", reg_data, 2))
            {
                dump_fetal(" cs get dts reg error\n");
                return BSP_ERROR;
            }
            g_modem_sysctrl_cfg.mdm_ctrl_sys_phy_addr  = (reg_data[0]);
            g_modem_sysctrl_cfg.mdm_ctrl_sys_mem_size  = reg_data[1];
            g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr = bsp_sysctrl_addr_get((void*)g_modem_sysctrl_cfg.mdm_ctrl_sys_phy_addr);

            if(of_property_read_u32_array(sysctrl_node, "clk", rst_data, 3))
            {
                memset(&g_modem_sysctrl_cfg,0,sizeof(g_modem_sysctrl_cfg));
                dump_fetal(" cs get dts clk error\n");
                return BSP_ERROR;
            }
            g_modem_sysctrl_cfg.clk.offset = (rst_data[0]);
            g_modem_sysctrl_cfg.clk.mcpu_dbg_clk_status = (rst_data[1]);
            g_modem_sysctrl_cfg.clk.mcpu_pd_clk_status = (rst_data[2]);

            memset(rst_data,0,sizeof(rst_data));
            if(of_property_read_u32_array(sysctrl_node, "reset", rst_data, 3))
            {
                memset(&g_modem_sysctrl_cfg,0,sizeof(g_modem_sysctrl_cfg));
                dump_fetal(" cs get dts reset error\n");
                return BSP_ERROR;
            }

            g_modem_sysctrl_cfg.rst.offset = (rst_data[0]);
            g_modem_sysctrl_cfg.rst.mdma9_pd_srst_status = (rst_data[1]);
            g_modem_sysctrl_cfg.rst.moda9_cpu_srst_status = (rst_data[2]);

            if(of_property_read_u32_array(sysctrl_node, "mtcmos", reg_data, 2))
            {
                memset(&g_modem_sysctrl_cfg,0,sizeof(g_modem_sysctrl_cfg));
                dump_fetal(" cs get dts tcmos error\n");
                return BSP_ERROR;
            }
            g_modem_sysctrl_cfg.mtcmos.offset = (reg_data[0]);
            g_modem_sysctrl_cfg.mtcmos.cpu_mtcmos_strl_status = (reg_data[1]);
        }
        else
        {
            dump_fetal("can not find dts\n");
            return BSP_OK;
        }
        dump_fetal("g_modem_sysctrl_cfg.mdm_ctrl_sys_phy_addr = 0x%x,g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr = 0x%x",g_modem_sysctrl_cfg.mdm_ctrl_sys_phy_addr,g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr);
        dump_fetal("g_modem_sysctrl_cfg.clk.offset = 0x%x,g_modem_sysctrl_cfg.clk.mcpu_dbg_clk_en = %d,g_modem_sysctrl_cfg.clk.mcpu_pd_clk_en=%d\n",g_modem_sysctrl_cfg.clk.offset,g_modem_sysctrl_cfg.clk.mcpu_dbg_clk_status,g_modem_sysctrl_cfg.clk.mcpu_pd_clk_status);
        dump_fetal("g_modem_sysctrl_cfg.rst.offset = 0x%x,g_modem_sysctrl_cfg.rst.mdma9_pd_srst_en= %d,g_modem_sysctrl_cfg.rst.moda9_cpu_srst_en =%d\n",g_modem_sysctrl_cfg.rst.offset,g_modem_sysctrl_cfg.rst.mdma9_pd_srst_status,g_modem_sysctrl_cfg.rst.moda9_cpu_srst_status);
        dump_fetal("g_modem_sysctrl_cfg.mtcmos.offset=0x%x,g_modem_sysctrl_cfg.mtcmos.cpu_mtcmos_strl_dis =%d\n",g_modem_sysctrl_cfg.mtcmos.offset,g_modem_sysctrl_cfg.mtcmos.cpu_mtcmos_strl_status);
    }

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_get_sysctrl_status
* 功能描述  : 查询modem系统控制器状态
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_get_sysctrl_status(void)
{
    if(DUMP_PHONE == dump_get_product_type())
    {
        u32 clk = 0;
        u32 rst = 0;
        u32 tcmos = 0;

        if(g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr == NULL)
        {
            dump_fetal("sys ctrl base addr is null\n");
            return BSP_ERROR;
        }
        clk = readl(g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr + g_modem_sysctrl_cfg.clk.offset);

        rst = readl(g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr + g_modem_sysctrl_cfg.rst.offset);

        tcmos = readl(g_modem_sysctrl_cfg.mdm_ctrl_sys_virt_addr + g_modem_sysctrl_cfg.mtcmos.offset);

        dump_fetal("clk = %x,rst = %x,tcmos = %x\n",clk,rst,tcmos);

        if(!BVAL(clk, g_modem_sysctrl_cfg.clk.mcpu_dbg_clk_status))
        {
            dump_fetal("modem a9 mcpu_dbg_clk_status is disable,mcpu_dbg_clk_status=0x%x status=0x%x\n",g_modem_sysctrl_cfg.clk.mcpu_dbg_clk_status,clk);
            return BSP_ERROR;
        }
        if(!BVAL(clk, g_modem_sysctrl_cfg.clk.mcpu_pd_clk_status))
        {
            dump_fetal("modem a9 mcpu_pd_clk_status is disable,bit=0x%x status=0x%x\n",g_modem_sysctrl_cfg.clk.mcpu_pd_clk_status,clk);
            return BSP_ERROR;
        }
        if(BVAL(rst, g_modem_sysctrl_cfg.rst.mdma9_pd_srst_status))
        {
            dump_fetal("modem a9 mdma9_pd_srst_status is disable,bit=0x%x status =0x%x\n",g_modem_sysctrl_cfg.rst.mdma9_pd_srst_status,rst);
            return BSP_ERROR;
        }
        if(BVAL(rst, g_modem_sysctrl_cfg.rst.moda9_cpu_srst_status))
        {
            dump_fetal("modem a9 moda9_cpu_srst_status is disable,bit=0x%x status =0x%x\n",g_modem_sysctrl_cfg.rst.moda9_cpu_srst_status,rst);
            return BSP_ERROR;
        }
        if(!BVAL(tcmos, g_modem_sysctrl_cfg.mtcmos.cpu_mtcmos_strl_status))
        {
            dump_fetal("modem a9 moda9_cpu_srst_status is disable,bit=0x%x status = 0x%x",g_modem_sysctrl_cfg.mtcmos.cpu_mtcmos_strl_status,tcmos);
            return BSP_ERROR;
        }
    }
    return BSP_OK;

}

/*****************************************************************************
* 函 数 名  : dump_coresight_memmap
* 功能描述  : 映射modem cp 的etb以及相关控制寄存器地址
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_coresight_memmap(void)
{  

    struct device_node* cs_node = NULL;
    unsigned int reg_data[2] = {0,};
    char* cs_name[2] = { "coresight,modem-etb","coresight,modem-etm"};
    u32 i = 0;
    coresight_mem_ctrl_t cs_mem_ctrl[2] = {{0,},{0,}};

    if(dump_modem_sysctrl_memmap() == BSP_ERROR)
    {
        dump_fetal("remap sysctrl fail\n");
        return BSP_ERROR;
    }
    for(i = 0;i < 2; i++)
    {
        memset(&(cs_mem_ctrl[i]), 0, sizeof(coresight_mem_ctrl_t));
        cs_node = of_find_compatible_node(NULL, NULL, cs_name[i]);

        if(cs_node)
        {
            if(of_property_read_u32_array(cs_node, "reg", reg_data, 2))
            {
                dump_fetal(" cs get dts reg error\n");
                return BSP_ERROR;
            }
            cs_mem_ctrl[i].cp_cs_phy_addr  = (reg_data[0]);
            cs_mem_ctrl[i].cp_cs_mem_size  = reg_data[1];
            cs_mem_ctrl[i].cp_cs_virt_addr = ioremap(cs_mem_ctrl[i].cp_cs_phy_addr,cs_mem_ctrl[i].cp_cs_mem_size);
        }
        else
        {

            dump_fetal("cs_node not fund\n");
        }
    }
    memcpy(&g_coresight_etb_mem_ctrl, &cs_mem_ctrl[0], sizeof(g_coresight_etb_mem_ctrl));
    memcpy(&g_coresight_etm_mem_ctrl, &cs_mem_ctrl[1], sizeof(g_coresight_etm_mem_ctrl));

    dump_fetal("g_coresight_etb_mem_ctrl.cp_cs_virt_addr = 0x%x,g_coresight_etb_mem_ctrl.cp_cs_phy_addr = 0x%x",
        g_coresight_etm_mem_ctrl.cp_cs_virt_addr,g_coresight_etm_mem_ctrl.cp_cs_phy_addr);

    dump_fetal("g_coresight_etm_mem_ctrl.cp_cs_virt_addr = 0x%x,g_coresight_etm_mem_ctrl.cp_cs_phy_addr = 0x%x",
        g_coresight_etb_mem_ctrl.cp_cs_virt_addr,g_coresight_etb_mem_ctrl.cp_cs_phy_addr);

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_stop_etm
* 功能描述  : 停止modem 的etm
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_stop_etm(void)
{
    volatile void* etm_addr = NULL;
    uint32_t etmcr = 0;
    int count = 0;

    if(g_coresight_etm_mem_ctrl.cp_cs_virt_addr == NULL)
    {

        dump_fetal("the cp_cs_virt_addr is null\n");
        return ;
    }
    etm_addr = g_coresight_etm_mem_ctrl.cp_cs_virt_addr;

    CS_ETM_UNLOCK(etm_addr);

    etmcr = cs_etm_readl(etm_addr, ETMCR);
    etmcr |= BIT(10);
    cs_etm_writel(etm_addr, etmcr, ETMCR);
    for (count = 10000; BVAL(cs_etm_readl(etm_addr, ETMSR), 1) != 1 && count > 0; count--)
    {
        udelay(10);
    }
    if(count >= 0)
    {
        dump_fetal("the ETMSR register has writed\n");
    }

    cs_etm_writel(etm_addr, 0x6F | BIT(14), ETMTEEVR);

    etmcr = cs_etm_readl(etm_addr, ETMCR);
    etmcr |= BIT(0);
    cs_etm_writel(etm_addr, etmcr, ETMCR);

    CS_ETM_LOCK(etm_addr);

    dump_fetal("the etm has stopped\n");
 

}
/*****************************************************************************
* 函 数 名  : dump_cs_etb
* 功能描述  : 保存etb信息
*
* 输入参数  :            
* 输出参数  : 

* 返 回 值  : 
              
*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_cs_etb(WDT_CORE_ID core_id)
{
    u32     reg_value;
    u32     i;
    u32  *  data;
    void*     dst_addr   = 0;
    u32     dst_size   = 0;
    volatile void*     tmcRegAddr = 0;


    if(core_id == WDT_CCORE_ID)
    {
        tmcRegAddr = g_coresight_etb_mem_ctrl.cp_cs_virt_addr;
        dst_addr   = bsp_dump_get_field_addr(DUMP_CP_UTRACE);
        dst_size   = DUMP_CP_UTRACE_SIZE;
    }
    else
    {
        dump_fetal("not wdt exception\n");
        return;
    }
    
    if((tmcRegAddr==0) || (dst_addr==0))
    {
        dump_fetal("coresigt addr is null %d %d\n",tmcRegAddr,dst_addr);
        return;
    }

    if(*(u32*)dst_addr == 0x89ABCDEF)
    {
        dump_fetal("the etb has save no need save again\n");
        return;
    }
    if(dump_get_sysctrl_status() == BSP_ERROR)
    {
        dump_fetal("modem a9  is not accessle,no need save etb\n");
        return;
    }

    /*先停止etm*/
    dump_stop_etm();

    /* unlock etb, 配置ETF_LOCK_ACCESS */
    writel(0xC5ACCE55, tmcRegAddr + 0xFB0);

    /* stop etb, 配置ETF_FORMAT_FLUSH_CTRL */
    reg_value = readl(tmcRegAddr + 0x304);
    /* FFCR StopOnFl */
    reg_value |= 1 << 12;
    /* FFCR FlushMem */
    reg_value |= 1 << 6;
    writel(reg_value, tmcRegAddr + 0x304);

    for(i=0; i<10000; i++)
    {
        /* read etb status, 读取ETF_STATUS */
        reg_value = readl(tmcRegAddr + 0x304);
        /* bit2为TMCReady指示位 */
        if(0 != ((reg_value & (1 << 6)) >> 6))
        {
            break;
        }
        udelay(10);
    }
    if(i >= 10000)
    {
        dump_fetal("ETF_STATUS register is not ready\n");
    }
    /* 等待TMCReady */
    for(i=0; i<10000; i++)
    {
        /* read etb status, 读取ETF_STATUS */
        reg_value = readl(tmcRegAddr + 0xc);
        /* bit2为TMCReady指示位 */
        if(0 != (reg_value & 0x4))
        {
            break;
        }
        udelay(10);
    }

    /* 超时判断 */
    if(i >= 10000)
    {
        dump_fetal("save etb time out\n");
    }

    /* 导出etb数据 */
    memset((void *)dst_addr, 0x0, dst_size);
    data = (u32 *)(dst_addr + 8);
    for(i=0; i<(1024*8)/4; i++)
    {
        /* read etb, 读取ETF_RAM_RD_DATA */
        reg_value = readl(tmcRegAddr + 0x10);
        *data = reg_value;
        data++;
        if(reg_value == 0xffffffff)
        {
            break;
        }
    }

    /* 0-3字节存放标识码 */
    *((u32 *)dst_addr) = (u32)0x89ABCDEF;
    /* 4-7个字节存放ETB数据长度 */
    *((u32 *)dst_addr + 1) = i*4;

    /* lock etb, 配置ETF_LOCK_ACCESS */
    writel(0x1, tmcRegAddr + 0xFB0);

    dump_fetal("the magic has writed 0x%x\n",*((u32 *)dst_addr));

}
