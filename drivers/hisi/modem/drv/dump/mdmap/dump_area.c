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


#include <product_config.h>
#include <bsp_ddr.h>
#include "bsp_dump_mem.h"
#include "dump_area.h"
#include "dump_config.h"
#undef	THIS_MODU
#define THIS_MODU mod_dump



struct dump_global_area_ctrl_s g_st_dump_area_ctrl;
struct dump_field_ctrl_info_s   g_st_field_ctrl;
u32    g_dump_mem_init_flag = 0;


/*****************************************************************************
* 函 数 名  : dump_get_load_info
* 功能描述  : 获取当前load的信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_get_load_info(dump_load_info_t* load_info)
{
    dump_load_info_t * dump_load = NULL;
    if(!g_st_dump_area_ctrl.init_state)
        return BSP_ERROR;
    if(!load_info)
        return BSP_ERROR;
    dump_load = (dump_load_info_t *)((u8*)g_st_dump_area_ctrl.virt_addr+(MNTN_AREA_RESERVE_ADDR-MNTN_BASE_ADDR));

    /* coverity[secure_coding] */
    (void)memcpy_s(load_info,sizeof(dump_load_info_t),dump_load,sizeof(dump_load_info_t));

    return BSP_OK;
}
/*****************************************************************************
* 函 数 名  : dump_get_level1_area_info
* 功能描述  : 获取一级area的信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_get_level1_area_info(DUMP_AREA_ID areaid,struct dump_area_mntn_addr_info_s* area_info)
{
    if(!g_st_dump_area_ctrl.init_state)
    {
        dump_error("dump area has not init\n");
        return BSP_ERROR;
    }

    if((areaid >= DUMP_AREA_BUTT)||(NULL == area_info))
    {
        dump_error("input param is error,areaid = %d\n",areaid);
        return BSP_ERROR;
    }

    area_info->vaddr    = (void*)((unsigned long)g_st_dump_area_ctrl.virt_addr + g_st_dump_area_ctrl.virt_addr->area_info[areaid].offset);
    area_info->paddr    = (void*)(g_st_dump_area_ctrl.phy_addr+g_st_dump_area_ctrl.virt_addr->area_info[areaid].offset);
    area_info->len      = g_st_dump_area_ctrl.virt_addr->area_info[areaid].length;
    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_get_level1_areaid_by_level2_areaid
* 功能描述  : 根据二级的area id查找一级的area id
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
DUMP_AREA_ID dump_get_level1_areaid_by_level2_areaid(DUMP_LEVLE2_AREA_ID area_id)
{

    if(area_id <= DUMP_NR_AREA_END)
    {
        return DUMP_AREA_NR;
    }
    else
    {
        return DUMP_AREA_BUTT;
    }
}
/*****************************************************************************
* 函 数 名  : dump_get_load_info
* 功能描述  : 获取当前load的信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

s32 dump_get_level2_area_info(DUMP_LEVLE2_AREA_ID level2_area_id,struct dump_area_mntn_addr_info_s* area_info)
{
    DUMP_AREA_ID area_id ;
    s32 ret;

    area_id = dump_get_level1_areaid_by_level2_areaid(level2_area_id);
    if(area_id == DUMP_AREA_NR)
    {
        ret = BSP_OK;
    }
    else
    {
        ret = BSP_ERROR;
    }

    return ret;


}
/*****************************************************************************
* 函 数 名  : dump_get_global_info
* 功能描述  : 获取rdr的global头信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_get_global_info(struct dump_global_area_ctrl_s * global_area)
{
    if(0 == g_st_dump_area_ctrl.init_state || global_area == NULL)
    {
        return BSP_ERROR;
    }
    global_area->virt_addr = g_st_dump_area_ctrl.virt_addr;
    global_area->phy_addr = g_st_dump_area_ctrl.phy_addr;
    global_area->length = g_st_dump_area_ctrl.length;
    return BSP_OK;

}

/*****************************************************************************
* 函 数 名  : dump_get_global_baseinfo
* 功能描述  : 获取rdr的global头信息的基本信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void* dump_get_global_baseinfo(void)
{
    if(!g_st_dump_area_ctrl.init_state)
    {
        dump_error("dump area has not init\n");

        return NULL;
    }

    return &(g_st_dump_area_ctrl.virt_addr->base_info);
}

/*****************************************************************************
* 函 数 名  : dump_level1_area_init
* 功能描述  : 一级area初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

s32 dump_level1_area_init(void)
{
    dump_load_info_t * dump_load = NULL;
    if(g_st_dump_area_ctrl.init_state)
    {
        dump_error("dump area has init\n");
        return BSP_OK;
    }
    g_st_dump_area_ctrl.phy_addr = MDDR_FAMA(MNTN_BASE_ADDR);
    g_st_dump_area_ctrl.length   = MNTN_BASE_SIZE;
    g_st_dump_area_ctrl.virt_addr= (struct dump_global_struct_s*)ioremap_wc(g_st_dump_area_ctrl.phy_addr,g_st_dump_area_ctrl.length);

    if(NULL == g_st_dump_area_ctrl.virt_addr)
    {
        dump_error("fail to remap area phy addr \n");
        return BSP_ERROR;
    }

/*如果没有balong的RDR说明对接的是手机，需要dump模块初始化area信息*/
    dump_ok("area init on phone platform\n");
    (void)memset_s(g_st_dump_area_ctrl.virt_addr,sizeof(*g_st_dump_area_ctrl.virt_addr),0,sizeof(*g_st_dump_area_ctrl.virt_addr));
    g_st_dump_area_ctrl.virt_addr->top_head.area_number     = DUMP_AREA_BUTT;
    g_st_dump_area_ctrl.virt_addr->top_head.magic           = DUMP_GLOBALE_TOP_HEAD_MAGIC;

    (void)strcpy_s((char*)g_st_dump_area_ctrl.virt_addr->top_head.build_time,sizeof(g_st_dump_area_ctrl.virt_addr->top_head.build_time),__DATE__","__TIME__);
    (void)strcpy_s((char*)g_st_dump_area_ctrl.virt_addr->top_head.product_name,sizeof(g_st_dump_area_ctrl.virt_addr->top_head.product_name),PRODUCT_NAME);
    (void)strcpy_s((char*)g_st_dump_area_ctrl.virt_addr->top_head.product_version,sizeof(g_st_dump_area_ctrl.virt_addr->top_head.product_version),PRODUCT_FULL_VERSION_STR);

    g_st_dump_area_ctrl.virt_addr->area_info[DUMP_AREA_MDMAP].offset     = MNTN_AREA_MDMAP_ADDR - MNTN_BASE_ADDR;
    g_st_dump_area_ctrl.virt_addr->area_info[DUMP_AREA_MDMAP].length     = MNTN_AREA_MDMAP_SIZE;

    g_st_dump_area_ctrl.virt_addr->area_info[DUMP_AREA_LR].offset     = MNTN_AREA_LR_ADDR - MNTN_BASE_ADDR;
    g_st_dump_area_ctrl.virt_addr->area_info[DUMP_AREA_LR].length     = MNTN_AREA_LR_SIZE;

    g_st_dump_area_ctrl.virt_addr->area_info[DUMP_AREA_NR].offset     = 0;
    g_st_dump_area_ctrl.virt_addr->area_info[DUMP_AREA_NR].length     = 0;

    dump_load = (dump_load_info_t *)((uintptr_t)g_st_dump_area_ctrl.virt_addr+(MNTN_AREA_RESERVE_ADDR-MNTN_BASE_ADDR));

    dump_load->mdm_ddr  = DDR_MCORE_ADDR;
    dump_load->mdm_share= DDR_SHARED_MEM_ADDR;
    dump_load->mdm_dump = DDR_MNTN_ADDR;
    dump_load->mdm_sram = HI_SRAM_MEM_ADDR;
    dump_load->mdm_dts  = DDR_MCORE_DTS_ADDR;
    dump_load->mdm_ddr_saveoff = 0;
    dump_load->mdm_share_ddr_size = HI_SHARED_DDR_SIZE;
    dump_load->magic_num= DUMP_LOAD_MAGIC;
    g_st_dump_area_ctrl.init_state = true;

    dump_ok("dump area init success\n");

    return BSP_OK;

}
/*****************************************************************************
* 函 数 名  : dump_get_top_head
* 功能描述  : 获取rdr的global头信息的基本信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void* dump_get_rdr_top_head(void)
{
    if(!g_st_dump_area_ctrl.init_state)
    {
        dump_error("dump area has not init\n");

        return NULL;
    }

    return &(g_st_dump_area_ctrl.virt_addr->top_head);
}

/*****************************************************************************
* 函 数 名  : dump_area_init
* 功能描述  : 初始化当前area信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_area_init(void)
{
    s32 ret = 0;
    ret  = dump_level1_area_init();
    if(ret == BSP_ERROR)
    {
        dump_error("fail to init dump level1 area\n");
    }

    return ret;
}

/*****************************************************************************
* 函 数 名  : dump_get_mntn_base_addr
* 功能描述  : 查询可维可测的基地址
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void* dump_get_mntn_base_addr(void)
{
    return g_st_dump_area_ctrl.virt_addr;
}
/*****************************************************************************
* 函 数 名  : dump_get_nr_areaid_by_fieldid
* 功能描述  : 查找nr的areaid
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
static void dump_get_nr_areaid_by_fieldid(u32 field_id,u32* level1_area_id,u32* level2_area_id)
{
    if((field_id>=DUMP_NRCCPU_FIELD_BEGIN) && (field_id<=DUMP_NRCCPU_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_NR;
        *level2_area_id = DUMP_NR_AREA_CCPU;
    }
    else if((field_id>=DUMP_NRL2HAC_FIELD_BEGIN) && (field_id<=DUMP_NRL2HAC_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_NR;
        *level2_area_id = DUMP_NR_AREA_L2HAC;
    }
    else if((field_id>=DUMP_NRHL1C_FIELD_BEGIN) && (field_id<=DUMP_NRHL1C_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_NR;
        *level2_area_id = DUMP_NR_AREA_HL1C;
    }
    else if((field_id>=DUMP_NRPHY_FIELD_BEGIN) && (field_id<DUMP_NRPHY_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_NR;
        *level2_area_id = DUMP_NR_AREA_PHY;
    }
    else if((field_id>=DUMP_RF_FIELD_BEGIN) && (field_id<DUMP_RF_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_NR;
        *level2_area_id = DUMP_NR_AREA_RF;
    }
    else
    {
        *level1_area_id = DUMP_AREA_BUTT;
        *level2_area_id = AREA_LEVEL2_AREA_BUTT;
    }

}
/*****************************************************************************
* 函 数 名  : dump_get_areaid_by_fieldid
* 功能描述  : 根据fieldid查找一级和二级的area id
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
static void dump_get_areaid_by_fieldid(u32 field_id,u32* level1_area_id,u32* level2_area_id)
{
    if((field_id>=DUMP_MODEMAP_FIELD_BEGIN) && (field_id<=DUMP_MODEMAP_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_MDMAP;
        *level2_area_id = AREA_LEVEL2_AREA_BUTT;
    }
    else if((field_id >=DUMP_CP_FIELD_BEGIN) && (field_id<=DUMP_CP_FIELD_END))
    {
        *level1_area_id = DUMP_AREA_LR;
        *level2_area_id = AREA_LEVEL2_AREA_BUTT;
    }
    else
    {
        dump_get_nr_areaid_by_fieldid(field_id,level1_area_id,level2_area_id);
    }

}
/*****************************************************************************
* 函 数 名  : dump_get_area_info_by_filed_id
* 功能描述  : 根据filed id 查找对应的area info,如果是二级area就返回二级area的信息
              如果是一级则返回一级area的信息
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

s32 dump_get_area_info_by_filed_id(u32 field_id,struct dump_area_mntn_addr_info_s* area_info)
{
    u32 level1_area_id = DUMP_AREA_BUTT;
    u32 level2_area_id = AREA_LEVEL2_AREA_BUTT;

    if(area_info == NULL)
    {
        return BSP_ERROR;
    }

    dump_get_areaid_by_fieldid(field_id,&level1_area_id,&level2_area_id);
    if(level1_area_id == DUMP_AREA_BUTT && level2_area_id == AREA_LEVEL2_AREA_BUTT)
    {
        dump_error("fail to find area id \n");
        return BSP_ERROR;
    }

    /*不包含二级area*/
    if(level2_area_id == AREA_LEVEL2_AREA_BUTT)
    {
        if(dump_get_level1_area_info((DUMP_AREA_ID)level1_area_id,area_info))
        {
            dump_error("fail to get area info \n");
            return BSP_ERROR;
        }
    }
    else
    {
       if(BSP_ERROR == dump_get_level2_area_info(level2_area_id,area_info))
       {
            dump_error("get level2 area info fail\n");
            return BSP_ERROR;
       }
    }
    return BSP_OK;

}
/*****************************************************************************
* 函 数 名  : bsp_dump_get_field_addr
* 功能描述  : 获取field的地址
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_get_field_addr(u32 field_id)
{
    u32 i = 0;
    u8 * addr = 0;
    dump_area_t * parea = NULL;
    struct dump_area_mntn_addr_info_s area_info;

    memset_s(&area_info,sizeof(area_info),0,sizeof(area_info));
    if(BSP_ERROR == dump_get_area_info_by_filed_id(field_id,&area_info))
    {
        return NULL;
    }

    if((!area_info.vaddr) || (!area_info.len) ||(!area_info.paddr))
    {
        dump_error("fail to get area info\n");
        return NULL;
    }
    parea = (dump_area_t *)area_info.vaddr;
    /* search field addr by field id */
    for(i=0; i<parea->area_head.field_num; i++)
    {
        if(field_id == parea->fields[i].field_id)
        {
            addr = (u8 *)parea + parea->fields[i].offset_addr;
            return addr;
        }
    }

    return NULL;
}
/*****************************************************************************
* 函 数 名  : bsp_dump_get_field_map
* 功能描述  : 获取field的field信息
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_get_field_map(u32 field_id)
{
    u32 i = 0 ;
    dump_area_t * parea = NULL;
    struct dump_area_mntn_addr_info_s area_info;

    if(BSP_ERROR == dump_get_area_info_by_filed_id(field_id,&area_info))
    {
        return NULL;
    }

    if((!area_info.vaddr) || (!area_info.len) ||(!area_info.paddr))
    {
        dump_error("fail to get area info\n");
        return NULL;
    }

    parea = (dump_area_t*)area_info.vaddr;

    /* search field map by field id */
    for(i=0; i<parea->area_head.field_num; i++)
    {
        if(field_id == parea->fields[i].field_id)
        {
            return (u8 * )&parea->fields[i];
        }
    }

    return NULL;
}

/*****************************************************************************
* 函 数 名  : bsp_dump_register_field
* 功能描述  : 申请field信息
            1. 不带地址注册，传入参数时virt_addr,phy_addr必须传0，成功返回dump注册地址
            2. 自带地址注册，传入参数时phy_addr为自带物理地址，virt_addr为虚拟地址，同时在dump内存中分配相同大小内存，成功返回邋virt_addr

            3. 两种注册方式，都将在dump划分内存，对于自带地址的注册方式，在系统异常时，由dump模块做数据拷贝
            4. 每个注册区域需要由使用者传入对应的版本号，高8位为主版本号，低8位为次版本号
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_register_field(u32 field_id, char * name, void * virt_addr, void * phy_addr, u32 length, u16 version_id)
{
    dump_area_t* area_info = NULL;
    u8*         ret = NULL;
    u32         index = 0;
    u32         i = 0;
    unsigned long flags = 0;
    struct dump_field_self_info_s*  self = NULL;
    dump_field_map_t* pfiled = NULL;

    struct dump_field_ctrl_info_s*  field_ctrl = &g_st_field_ctrl;

    if(!g_dump_mem_init_flag)
    {
        bsp_dump_mem_init();
    }

    if((!name) || (!((field_id >=DUMP_MODEMAP_FIELD_BEGIN) && (field_id <=DUMP_MODEMAP_FIELD_END))))
    {
        dump_error("error param\n");
        return NULL;
    }

    spin_lock_irqsave(&field_ctrl->lock, flags);

    area_info = (dump_area_t*)field_ctrl->virt_area_addr;
    /*注册field个数超出最大范围*/
    index = area_info->area_head.field_num;
    /*检查是否有重复注册*/
    for(i=0;i<index;i++)
    {
        if(area_info->fields[i].field_id == field_id)
        {
            spin_unlock_irqrestore(&field_ctrl->lock, flags);
            return (u8*)area_info + area_info->fields[i].offset_addr;
        }
    }

    if(index >= DUMP_FIELD_MAX_NUM)
    {
        spin_unlock_irqrestore(&field_ctrl->lock, flags);

        dump_error("there is not enough id\n");

        return NULL;
    }

    /*剩余空间不足*/
    if(field_ctrl->free_length < length)
    {
        spin_unlock_irqrestore(&field_ctrl->lock, flags);

        dump_error("dump not enough space\n");

        return NULL;
    }

    ret = (u8*)area_info + field_ctrl->free_offset;
    DUMP_FIXED_FIELD(((void*)&(area_info->fields[index])),(field_id),(name),(field_ctrl->free_offset),(length),version_id);
    area_info->area_head.field_num ++;
    pfiled = &(area_info->fields[index]);

    field_ctrl->free_length -= length;
    field_ctrl->free_offset += length;
    field_ctrl->field_num ++;

    area_info->area_head.field_num = field_ctrl->field_num;

    /*自带地址注册*/
    if( virt_addr || phy_addr )
    {
        self = (struct dump_field_self_info_s*)ret;
        self->magic_num = DUMP_FIELD_SELF_MAGICNUM;
        self->phy_addr  = phy_addr;
        self->virt_addr  = virt_addr;
        self->reserved  = 0;
    }
    spin_unlock_irqrestore(&field_ctrl->lock, flags);

    return ret;
}

void dump_save_save_self_addr(struct dump_field_ctrl_info_s* filed_ctrl_info)
{
    dump_area_t* area_info;
    u32         index;
    u32         i;
    unsigned long flags;
    struct dump_field_self_info_s*  self;
    void* self_virt;

    if(filed_ctrl_info == NULL)
    {
        return;
    }

    spin_lock_irqsave(&filed_ctrl_info->lock, flags);

    area_info = (dump_area_t*)filed_ctrl_info->virt_area_addr;
    index     = filed_ctrl_info->field_num;

    for(i=0;i<index;i++)
    {
        self = (struct dump_field_self_info_s*)((u8*)area_info+area_info->fields[i].offset_addr);

        if((DUMP_FIELD_SELF_MAGICNUM == self->magic_num)&&(self->virt_addr))
        {
            self_virt = self->virt_addr;
            /*coverity[secure_coding]*/
            memcpy_s(self,area_info->fields[i].length,self_virt,area_info->fields[i].length);
        }
    }
    spin_unlock_irqrestore(&filed_ctrl_info->lock, flags);

    dump_ok("dump save self addr ok\n");
}
/*****************************************************************************
* 函 数 名  : bsp_dump_save_self_addr
* 功能描述  : 保存自注册的空间
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void bsp_dump_save_self_addr(void)
{
    dump_save_save_self_addr(&g_st_field_ctrl);

}

/*****************************************************************************
* 函 数 名  : bsp_dump_get_field_phy_addr
* 功能描述  : 获取field的物理地址
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_get_field_phy_addr(u32 field_id)
{
    u8 * addr = 0;
    struct dump_area_mntn_addr_info_s area_info;
    dump_area_t * parea = NULL;

    addr = bsp_dump_get_field_addr(field_id);
    if(NULL == addr)
    {
        return NULL;
    }
    if(BSP_ERROR == dump_get_area_info_by_filed_id(field_id,&area_info))
    {
        return NULL;
    }

    if((!area_info.vaddr) || (!area_info.len) ||(!area_info.paddr))
    {
        return NULL;
    }
    parea = (dump_area_t *)area_info.vaddr;

    /*coverity[overflow] */
    return (u8 *)((unsigned long)addr - (unsigned long)area_info.vaddr + (unsigned long)area_info.paddr);
}

/*****************************************************************************
* 函 数 名  : dump_lr_filed_init
* 功能描述  : lr子系统下的field初始化
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_mdmap_filed_init(void)
{
    struct dump_area_mntn_addr_info_s area_info;

    if(g_st_field_ctrl.init_flag)
    {
        return BSP_OK;
    }

    spin_lock_init(&g_st_field_ctrl.lock);

    memset_s(&area_info,sizeof(area_info),0,sizeof(area_info));
    if(BSP_ERROR == dump_get_level1_area_info(DUMP_AREA_MDMAP,&area_info))
    {
        dump_error("fail to get DUMP_AREA_MDMAP area\n");
        return BSP_ERROR;
    }
    if((!area_info.vaddr) || (!area_info.len) ||(!area_info.paddr))
    {
        dump_error("area info is error\n");
        return BSP_ERROR;
    }


    g_st_field_ctrl.virt_area_addr = area_info.vaddr;
    g_st_field_ctrl.phy_area_addr  = area_info.paddr;
    g_st_field_ctrl.total_length   = area_info.len;
    g_st_field_ctrl.free_offset    = sizeof(dump_area_t);
    g_st_field_ctrl.free_length    = g_st_field_ctrl.total_length - g_st_field_ctrl.free_offset;

    memset_s(g_st_field_ctrl.virt_area_addr,sizeof(*g_st_field_ctrl.virt_area_addr),0,sizeof(*g_st_field_ctrl.virt_area_addr));

    g_st_field_ctrl.virt_area_addr->area_head.magic_num = DUMP_LEVEL1_AREA_MAGICNUM;

    memcpy_s(g_st_field_ctrl.virt_area_addr->area_head.name,sizeof(g_st_field_ctrl.virt_area_addr->area_head.name),MDMAP_AREA_NAME,strlen(MDMAP_AREA_NAME));

    g_st_field_ctrl.virt_area_addr->area_head.field_num = 0;

    g_st_field_ctrl.init_flag = true;

    dump_ok("mdm ap field init ok\n");

    return BSP_OK;

}


/*****************************************************************************
* 函 数 名  : bsp_dump_get_avaiable_size
* 功能描述  :查询本核剩余的可维可测空间
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

void bsp_dump_get_avaiable_size(u32* size)
{
    unsigned long flags = 0;
    if(size != NULL)
    {
        spin_lock_irqsave(&g_st_field_ctrl.lock, flags);
        *size = g_st_field_ctrl.free_length;
        spin_unlock_irqrestore(&g_st_field_ctrl.lock, flags);
    }
}


/*****************************************************************************
* 函 数 名  : bsp_dump_field_init
* 功能描述  : cp dump fileld初始化
*
* 输入参数  :field_id
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_dump_field_init(void)
{
    if(BSP_OK != dump_mdmap_filed_init())
    {
        dump_error("fail to init mdmap fields\n");
        return BSP_ERROR;
    }

    dump_ok("dump field init ok\n");
    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_dump_mem_init
* 功能描述  : dump可维可测空间初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_dump_mem_init(void)
{
    if(g_dump_mem_init_flag == 1)
    {
        return BSP_OK;
    }
    if(dump_area_init())
    {
        return BSP_ERROR;
    }

    if(bsp_dump_field_init())
    {
        return BSP_ERROR;
    }

    dump_ok("dump mem init ok\n");
    g_dump_mem_init_flag = 1;

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : dump_show_field
* 功能描述  : 调试接口，显示当前已经注册field使用情况
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_show_field(void)
{
    return;
}

EXPORT_SYMBOL_GPL(bsp_dump_register_field);
EXPORT_SYMBOL_GPL(bsp_dump_get_field_addr);
EXPORT_SYMBOL(dump_show_field);





