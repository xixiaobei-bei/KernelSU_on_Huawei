


#include <bsp_nvim.h>
#include <bsp_blk.h>
#include <bsp_version.h>
#include "nv_index.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_index.h"
#include "nv_debug.h"
#include "nv_partition_upgrade.h"

#define THIS_MODU mod_nv

#define NV_FILE_UPGRADE_FLAG    NV_FILE_UPDATA

u32 nv_cust_get_head_info(const s8 * file_name, nv_dload_head *dload_head)
{
    u32 ret;
    FILE *fp;

    /* check modem_nv.bin upgrade flag */
    fp = nv_file_open((s8*)file_name, (s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("Head_info: open %s file failed!\n", file_name);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    ret = (u32)nv_file_read((u8*)dload_head, 1, (u32)sizeof(nv_dload_head), fp);
    (void)nv_file_close(fp);
    if(ret != sizeof(nv_dload_head))
    {
        nv_record("Head_info: read %s file dload head err!ret=0x%x.\n", file_name, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    return NV_OK;
}

u32 nv_cust_get_tail_info(const s8 * file_name, nv_dload_tail *dload_tail)
{
    u32 ret;
    u32 offset;
    FILE *fp;

    nv_dload_head dload_head;

    /* check modem_nv.bin upgrade flag */
    ret = nv_cust_get_head_info((s8*)file_name, &dload_head);
    if(ret)
    {
        nv_record("R_Tail_info: get %s file head info failed!\n", file_name);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    if(dload_head.nv_bin.magic_num == NV_CTRL_FILE_MAGIC_NUM)
    {
        return NV_OK;
    }

    /*second count file total len*/
    offset = sizeof(nv_dload_head);
    offset += ((dload_head.nv_bin.magic_num == NV_FILE_EXIST)?dload_head.nv_bin.len:0);
    offset += ((dload_head.xnv_map.magic_num == NV_FILE_EXIST)?dload_head.xnv_map.len:0);
    offset += ((dload_head.xnv.magic_num == NV_FILE_EXIST)?dload_head.xnv.len:0);
    offset += ((dload_head.cust_map.magic_num == NV_FILE_EXIST)?dload_head.cust_map.len:0);
    offset += ((dload_head.cust.magic_num == NV_FILE_EXIST)?dload_head.cust.len:0);

    /* check modem_nv.bin upgrade flag */
    fp = nv_file_open((s8*)file_name, (s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("R_Tail_info: open %s file failed!\n", file_name);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    (void)nv_file_seek(fp, offset, SEEK_SET);
    ret = (u32)nv_file_read((u8*)dload_tail, 1, (u32)sizeof(nv_dload_tail), fp);
    (void)nv_file_close(fp);
    if(ret != sizeof(nv_dload_tail))
    {
        nv_record("R_Tail_info: read %s file dload tail err!ret=0x%x\n", file_name, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    return NV_OK;
}

u32 nv_cust_set_tail_info(const s8 * file_name, nv_dload_tail *dload_tail)
{
    u32 ret;
    u32 offset;
    FILE *fp;

    nv_dload_head dload_head;

    /* check modem_nv.bin upgrade flag */
    ret = nv_cust_get_head_info((s8*)file_name, &dload_head);
    if(ret)
    {
        nv_record("W_Tail_info: get %s file head info failed!\n", file_name);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    if(dload_head.nv_bin.magic_num == NV_CTRL_FILE_MAGIC_NUM)
    {
        return NV_OK;
    }

    /*second count file total len*/
    offset = sizeof(nv_dload_head);
    offset += ((dload_head.nv_bin.magic_num == NV_FILE_EXIST)?dload_head.nv_bin.len:0);
    offset += ((dload_head.xnv_map.magic_num == NV_FILE_EXIST)?dload_head.xnv_map.len:0);
    offset += ((dload_head.xnv.magic_num == NV_FILE_EXIST)?dload_head.xnv.len:0);
    offset += ((dload_head.cust_map.magic_num == NV_FILE_EXIST)?dload_head.cust_map.len:0);
    offset += ((dload_head.cust.magic_num == NV_FILE_EXIST)?dload_head.cust.len:0);

    /* check modem_nv.bin upgrade flag */
    fp = nv_file_open((s8*)file_name, (s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_record("W_Tail_info: open %s file failed!\n", file_name);
        return BSP_ERR_NV_OPEN_FILE_FAIL;
    }

    (void)nv_file_seek(fp, offset, SEEK_SET);
    ret = (u32)nv_file_write((u8*)dload_tail, 1, (u32)sizeof(nv_dload_tail), fp);
    (void)nv_file_close(fp);
    if(ret != sizeof(nv_dload_tail))
    {
        nv_record("W_Tail_info: write %s file dload tail err!ret=0x%x\n", file_name, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    return NV_OK;
}

u32 nv_cust_set_upgrade_flag(const s8 *file_name, bool flag)
{
    u32 ret;

    nv_dload_tail dload_tail;

    ret = nv_cust_get_tail_info(file_name, &dload_tail);
    if(ret)
    {
        nv_record("Get %s file tail info err!\n", file_name);
        return NV_ERROR;
    }

    if(flag)
    {
        dload_tail.upgrade_flag = NV_FILE_UPGRADE_FLAG;
    }
    else
    {
        dload_tail.upgrade_flag = NV_FILE_EXIST;
    }

    ret = nv_cust_set_tail_info(file_name, &dload_tail);
    if(ret)
    {
       nv_record("write %s file tail info err!\n", file_name);
       return NV_ERROR;
    }

    return NV_OK;
}

u32 nv_upgrade_file_updata(void)
{
    u32 ret;

    ret = nv_file_update((s8*)NV_DLOAD_PATH);
    if(ret)
    {
        nv_record("updata %s file info fail.\n", (s8*)NV_DLOAD_PATH);
        return NV_ERROR;
    }

    if(!nv_file_access((s8*)NV_DLOAD_CUST_PATH,0))
    {
        ret = nv_file_update(NV_DLOAD_CUST_PATH);
        if(ret)
        {
            nv_record("updata %s file info fail.\n", (s8*)NV_DLOAD_CUST_PATH);
            return NV_ERROR;
        }
    }

    return NV_OK;
}

u32 nv_set_coldpatch_upgrade_flag(bool flag)
{
    u32 ret = NV_ERROR;
    u32 flen;
    nv_dload_head phead;
    nv_dload_tail ptail;

    ret = nv_storage_read((char *)NV_COLD_SEC_NAME, 0, &phead, (u32)sizeof(nv_dload_head));
    if(ret)
    {
        nv_printf("coldpatch can't access, ret:0x%x\n", ret);
        return ret;
    }

    if(phead.nv_bin.magic_num != NV_FILE_EXIST)
    {
        nv_printf("coldpatch file error\n");
        return BSP_ERR_NV_NO_FILE;
    }

    flen = sizeof(nv_dload_head);
    flen += ((phead.nv_bin.magic_num == NV_FILE_EXIST)? phead.nv_bin.len:0);
    flen += ((phead.xnv_map.magic_num == NV_FILE_EXIST)? phead.xnv_map.len:0);
    flen += ((phead.xnv.magic_num == NV_FILE_EXIST)? phead.xnv.len:0);
    flen += ((phead.cust_map.magic_num == NV_FILE_EXIST)? phead.cust_map.len:0);
    flen += ((phead.cust.magic_num == NV_FILE_EXIST)? phead.cust_map.len:0);

    ret = nv_storage_read((char *)NV_COLD_SEC_NAME, flen, &ptail, (u32)sizeof(nv_dload_tail));
    if(ret)
    {
        nv_printf("read coldpatch tail failed, ret:0x%x\n", ret);
        return BSP_ERR_NV_READ_DATA_FAIL;
    }

    if(flag)
    {
        ptail.upgrade_flag = NV_FILE_UPGRADE_FLAG;
    }
    else
    {
        ptail.upgrade_flag = NV_FILE_EXIST;
    }

    ret = nv_storage_write((char *)NV_COLD_SEC_NAME, flen, &ptail, (u32)sizeof(nv_dload_tail));
    if(ret)
    {
        nv_printf("write coldpatch tail failed, ret:0x%x\n", ret);
        return BSP_ERR_NV_WRITE_DATA_FAIL;
    }

    return NV_OK;
}

u32 nv_upgrade_set_flag(bool flag)
{
    u32 ret = NV_ERROR;

    if(!nv_file_access((s8*)NV_DLOAD_PATH,0))
    {
        ret = nv_cust_set_upgrade_flag((s8*)NV_DLOAD_PATH, flag);
    }

    if(!nv_file_access((s8*)NV_DLOAD_CUST_PATH,0))
    {
        ret |= nv_cust_set_upgrade_flag((s8*)NV_DLOAD_CUST_PATH, flag);
    }

    return ret;
}

EXPORT_SYMBOL(nv_upgrade_file_updata);
EXPORT_SYMBOL(nv_upgrade_set_flag);

