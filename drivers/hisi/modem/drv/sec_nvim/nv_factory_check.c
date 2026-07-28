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


#include <linux/vmalloc.h>
#include <product_nv_id.h>
#include <product_nv_def.h>
#include <osl_common.h>
#include <mdrv_nvim.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_index.h"
#include "nv_factory_check.h"
#include "nv_ctrl.h"

#define THIS_MODU mod_nv

/* 计算字符串的CRC */
u32 nv_cal_crc32(u8 *Packet, u32 dwLength)
{
    static u32 CRC32_TABLE[256] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
    };

    u32 CRC32 = 0;
    u32 i     = 0;


    for(i=0; i<dwLength; i++)
    {
        CRC32 = ((CRC32<<8)|Packet[i]) ^ (CRC32_TABLE[(CRC32>>24)&0xFF]); /* [false alarm]:fortify  */
    }

    for(i=0; i<4; i++)
    {
        CRC32 = ((CRC32<<8)|0x00) ^ (CRC32_TABLE[(CRC32>>24)&0xFF]); /* [false alarm]:fortify  */
    }

    return CRC32;
}

/*****************************************************************************
 函 数 名  : nv_make_single_band_nv_crc
 功能描述  : 计算单个band的校准nv的CRC校验码
 输入参数  : singal_band_nvid_s* check_item_temp : 待计算的band的nvid列表的起始地址
             u32 modem_num : 待计算的band所属的modem num
             u32 valild_nvid_count : 该band中可用nvid的最大数量
             u32 *last_crc_value : 上一次生成的CRC校验码
 输出参数  : u32 *gen_crc_value : 生成的CRC校验码
 返 回 值  : NV_OK 计算CRC校验码成功 其他:计算错误
*****************************************************************************/
u32 nv_make_single_band_nv_crc(singal_band_nvid_t* check_item_temp, u32 modem_num, u32 valild_nvid_count, u32 *last_crc_value, u32 *gen_crc_value)
{
    u8 * temp_nv_data;
    u32 * pdata;
    u32 ret = 0;
    u32 data_len;
    u32 nv_len;
    u32 nvid_index;
    u32 crc_value = 0;

    /*配置的可用nvid的数量不能大于可用nvid数量的最大值*/
    if(check_item_temp->uhwValidCount > valild_nvid_count)
    {
        nv_printf("nv list size is larger than stuct, uhwValidCount: 0x%x , struct support is:0x%x\n", check_item_temp->uhwValidCount, valild_nvid_count);
        return BSP_ERR_NV_OVER_MEM_ERR;
    }

    pdata = (u32 *)vmalloc(sizeof(u32) + 2*NV_MAX_UNIT_SIZE);
    if(!pdata)
    {
        nv_printf("malloc pdata fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    nv_len = 0;
    if(last_crc_value != NULL)
    {
        *pdata = *last_crc_value;
        data_len = sizeof(u32);
    }
    else
    {
        data_len = 0;
    }

    temp_nv_data = (u8 *)((unsigned long)pdata + data_len);
    for(nvid_index = 0; nvid_index < check_item_temp->uhwValidCount; nvid_index++)
    {
        /*读取待校验nv*/
        ret = bsp_nvm_get_len(check_item_temp->auhwNeedCheckID[nvid_index], &nv_len);
        if(ret)
        {
            nv_printf("get nv len error nvid: 0x%x fail, ret = 0x%x\n", check_item_temp->auhwNeedCheckID[nvid_index], ret);
            vfree((void *)pdata);
            return ret;
        }
        ret = bsp_nvm_dcread(modem_num, check_item_temp->auhwNeedCheckID[nvid_index], temp_nv_data, nv_len);
        if(ret)
        {
            nv_printf("read nv 0x%x fail, ret = 0x%x\n", check_item_temp->auhwNeedCheckID[nvid_index], ret);
            vfree((void *)pdata);
            return ret;
        }

        data_len += nv_len;

        nv_debug_printf("to cal crc data len 0x%x\n", data_len);
        nv_debug_printf("to cal crc data content:0x%x 0x%x 0x%x 0x%x 0x%x\n", pdata[0], pdata[1], pdata[2], pdata[3], pdata[4]);
        /*计算CRC校验码*/
        crc_value = nv_cal_crc32((u8 *)pdata, data_len);
        nv_debug_printf("after cal nvid: 0x%x crc value 0x%x\n", check_item_temp->auhwNeedCheckID[nvid_index] ,crc_value);

        *pdata = crc_value;
        data_len = sizeof(u32);
        temp_nv_data = (u8*)((unsigned long)pdata + data_len);
    }

    *gen_crc_value = crc_value;
    vfree((void *)pdata);
    return NV_OK;
}


u32 nv_check_mode_crc(factory_crc_check_info_t check_info)
{
    u32 index;
    u32 ret;
    singal_band_nvid_t * check_item_buff;
    singal_band_nvid_t * check_item_temp;
    u32 nv_len = 0;
    u32 last_crc_value = 0;
    u32 gen_crc_value = 0;
    u32 first_cal = 0;

    /*获取存放nv列表的nv的长度*/
    ret = bsp_nvm_get_len(check_info.list_nvid, &nv_len);
    if(ret)
    {
        nv_printf("get nv:0x%x len fail\n", check_info.list_nvid);
        return ret;
    }

    check_item_buff = (singal_band_nvid_t *)vmalloc((size_t)nv_len);
    if(!check_item_buff)
    {
        nv_printf("malloc fail,can not to read item nv\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    ret = bsp_nvm_read(check_info.list_nvid, (u8 *)check_item_buff, nv_len);
    if(ret)
    {
        nv_printf("read check nv item fail nvid:0x%x\n", check_info.list_nvid);
        vfree(check_item_buff);
        return ret;
    }

    check_item_temp = check_item_buff;
    for(index = 0; index < check_info.valild_band_count; index++)
    {

        nv_debug_printf("next single band nv addr = %pK\n", check_item_temp);

        /*如未使能该band的检查 或者配置可用数量为0，则不做检查*/
        if((SINGAL_NV_ENABLE != check_item_temp->uhwEnable)||(0 == check_item_temp->uhwValidCount))
        {
            nv_printf("index:0x%x single nv is not invalid, enable:0x%x validcount:0x%x\n", index, check_item_temp->uhwEnable, check_item_temp->uhwValidCount);
            check_item_temp = (singal_band_nvid_t *)((unsigned long)check_item_temp + check_info.singal_group_size);
            continue;
        }

        last_crc_value = gen_crc_value;
        if(0 == first_cal)
        {
            nv_debug_printf("cal first single band nv crc\n");
            ret = nv_make_single_band_nv_crc(check_item_temp, check_info.modem_num, check_info.valild_nvid_count, NULL, &gen_crc_value);
            first_cal = 1;
        }
        else
        {
            nv_debug_printf("cal not first single band nv crc\n");
            ret = nv_make_single_band_nv_crc(check_item_temp, check_info.modem_num, check_info.valild_nvid_count, &last_crc_value, &gen_crc_value);
        }
        if(ret)
        {
            nv_printf("make single band nv crc fail, ret = 0x%x\n", ret);
            vfree(check_item_buff);
            return ret;
        }

        check_item_temp = (singal_band_nvid_t *)((unsigned long)check_item_temp + check_info.singal_group_size);
    }

    vfree(check_item_buff);
    if(gen_crc_value == check_info.old_crc_value)
    {
        return NV_OK;
    }
    else
    {
        nv_printf("check crc fail\nnew crc:0x%x, old crc:0x%x\n", gen_crc_value, check_info.old_crc_value);
        return BSP_ERR_NV_CRC_CODE_ERR;
    }
}

/*****************************************************************************
 函 数 名  : nv_check_factory_nv_status
 功能描述  : 校验校准NV的CRC校验码
 输入参数  : u32 mode取值0: GU主卡 1:TL主卡 2:GU副卡
 输出参数  : 无
 返 回 值  : 0:校验通过
             0xxxxx027:CRC校验出错
             其他错误码执行过程中出错
*****************************************************************************/
u32 nv_check_factory_nv_status(u32 mode)
{
    NV_CRC_CHECK_RESULT_STRU crc_result_stru = {0};
    factory_crc_check_info_t check_info = {0};
    u32 ret;

    /*read crc value,nv 50000 is defined by product line*/
    ret = bsp_nvm_read(NV_ID_CRC_CHECK_RESULT, (u8*)&crc_result_stru, (u32)sizeof(NV_CRC_CHECK_RESULT_STRU));
    if(ret)
    {
        nv_printf("read crc result nv fail\n nv id:0x%x\n", NV_ID_CRC_CHECK_RESULT);
        return ret;
    }

    switch(mode)
    {
        case NV_FACTORY_CHECK_M1_GU:
            check_info.list_nvid = NV_ID_GUC_CHECK_ITEM;
            check_info.modem_num = NV_USIMM_CARD_1;
            check_info.old_crc_value = crc_result_stru.uwGucCrcResult;
            check_info.valild_band_count = MAX_NV_GUC_CHECK_ITEM_NUM;
            check_info.valild_nvid_count = MAX_SINGLE_GUC_BAND_CHECK_NUM;
            check_info.singal_group_size = sizeof(SINGLE_GUC_BAND_NV_ID_STRU);
            break;
        case NV_FACTORY_CHECK_M1_TL:
            check_info.list_nvid = NV_ID_TL_CHECK_ITEM;
            check_info.modem_num = NV_USIMM_CARD_1;
            check_info.old_crc_value = crc_result_stru.uwTlCrcResult;
            check_info.valild_band_count = MAX_NV_TL_CHECK_ITEM_NUM;
            check_info.valild_nvid_count = MAX_SINGLE_TL_BAND_CHECK_NUM;
            check_info.singal_group_size = sizeof(SINGLE_TL_BAND_NV_ID_STRU);
            break;
        case NV_FACTORY_CHECK_M2_GU:
            check_info.list_nvid = NV_ID_GUC_M2_CHECK_ITEM;
            check_info.modem_num = NV_USIMM_CARD_2;
            check_info.old_crc_value = crc_result_stru.uwGucM2CrcResult;
            check_info.valild_band_count = MAX_NV_GUC_CHECK_ITEM_NUM;
            check_info.valild_nvid_count = MAX_SINGLE_GUC_BAND_CHECK_NUM;
            check_info.singal_group_size = sizeof(SINGLE_GUC_BAND_NV_ID_STRU);
            break;
        default:
            nv_printf("para error, mode = 0x%x\n", mode);
            return BSP_ERR_NV_INVALID_PARAM;
    }

    return nv_check_mode_crc(check_info);
}

u32 nv_read_item_from_factory(u32 modem_id, u32 itemid, u32 offset, u8 *pdata, u32 datalen)
{
    u32 ret;
    nv_ctrl_info_s ctrl_info;
    nv_item_info_s *item_info;
    nv_file_info_s *file_info;
    nv_ctrl_info_s *ctrl_info_ptr = NULL;
    u32 item_offset = 0;
    FILE * fp;

    fp = nv_file_open((s8 *)NV_DEFAULT_PATH, (s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("open %s file failed!\n", NV_DEFAULT_PATH);
        return BSP_ERR_NV_NO_FILE;
    }

    /*read head info */
    ret = nv_file_read((u8*)(&ctrl_info),1,(u32)sizeof(nv_ctrl_info_s),fp);
    if(ret != sizeof(nv_ctrl_info_s))
    {
        nv_record("nv read factory read ctrl info err ret %d ctrl_size %d !\n", ret, (u32)sizeof(nv_ctrl_info_s));
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto nv_readEx_err;
    }
    nv_printf("nv read size ctrl %x , ctrl size %x \n", sizeof(nv_ctrl_info_s), ctrl_info.ctrl_size);

    ctrl_info_ptr = (nv_ctrl_info_s*)nv_malloc(ctrl_info.ctrl_size);
    if(NULL == ctrl_info_ptr)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        nv_record("nv read factory malloc size %d failed!\n", ctrl_info.ctrl_size);
        goto nv_readEx_err;
    }

    /*read ctrl info */
    nv_file_seek(fp,0,SEEK_SET);
    ret = nv_file_read((u8*)ctrl_info_ptr,1, ctrl_info.ctrl_size,fp);
    if(ret != ctrl_info.ctrl_size)
    {
        nv_record("nv read factory read ctrl err ret %d ctrl_size %d !\n", ret, ctrl_info.ctrl_size);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto nv_readEx_err;
    }
    ret = nv_search_iteminfo(itemid, ctrl_info_ptr, &item_info);
    if(ret)
    {
        nv_record("nv read factory can't find item id %d modem_id %d ret %d !\n", itemid, modem_id, ret);
        goto nv_readEx_err;
    }
    if((offset + datalen) > item_info->nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_READ_EX,5,offset,datalen,item_info->nv_len);
        nv_record("nv read factory over length offset %d datalen %d nvlen %d !\n", offset, datalen, item_info->nv_len);
        goto nv_readEx_err;
    }

    if((modem_id == 0) || (modem_id > ctrl_info.modem_num))
    {
        ret = BSP_ERR_NV_INVALID_MDMID_ERR;
        nv_debug(NV_FUN_READ_EX,6,ret,itemid,modem_id);
        nv_record("nv read factory ctrl mod num error modem_id %d mod num %d!\n", (u32)modem_id, ctrl_info.modem_num);
        goto nv_readEx_err;
    }

    if( modem_id > item_info->modem_num)
    {
        modem_id = 1;
    }

    /*calc item offset */
    file_info = (nv_file_info_s *)((unsigned long)ctrl_info_ptr + ctrl_info.file_offset);
    item_offset = file_info[item_info->file_id - 1].file_offset + item_info->nv_off[modem_id- 1] + offset;

    /*read item */
    nv_file_seek(fp,(s32)item_offset,SEEK_SET);
    ret = nv_file_read((u8*)(pdata),1,(u32)datalen,fp);
    
    (void)nv_file_close(fp);
    nv_free(ctrl_info_ptr);
    ctrl_info_ptr = NULL;
    if(ret != datalen)
    {
        nv_record("nv_file_read len error req len 0x%x  return len 0x%x!\n", (u32)datalen, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }
    
    return NV_OK;

nv_readEx_err:
    (void)nv_file_close(fp);
    if (ctrl_info_ptr)
        nv_free(ctrl_info_ptr);
    ctrl_info_ptr = NULL;
    return ret;
}

EXPORT_SYMBOL(nv_check_factory_nv_status);


