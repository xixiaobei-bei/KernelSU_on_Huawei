

#ifdef _PRE_COLDBOOT_FEATURE
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include "plat_firmware_flash.h"
#include "plat_firmware.h"
#include "plat_firmware_util.h"
#include "plat_debug.h"
#include "plat_cali.h"
#include "board.h"

#define DOWNLOAD_CMD_COUNT      64
#define DOWNLOAD_FILE_PATH_LEN  512
#define DOWNLOAD_FILE_COUNT     2
#define FIRMWARE_PARTION        "/dev/block/by-name/reserved2"

typedef struct firmware_cmd_globals {
    int32_t count;                               /* 存储每个cfg文件解析后有效的命令个数 */
    cmd_type_info cmd[DOWNLOAD_CMD_COUNT];       /* 存储每个cfg文件的有效命令 */
} firmware_cmd_globals;

typedef struct firmware_file_desc {
    uint8_t file_path[DOWNLOAD_FILE_PATH_LEN];
    int32_t flash_offset;
    int32_t file_len;
} firmware_file_desc;

typedef struct firmware_file_globals {
    int32_t count;
    firmware_file_desc file_desc[DOWNLOAD_FILE_COUNT];
} firmware_file_globals;

typedef struct firmware_flash_map {
    firmware_cmd_globals cmd_cfg;
    firmware_file_globals file_cfg;
    bfgx_cali_data_stru cali_data;
} firmware_flash_map;

static firmware_flash_map g_firmware_map;

STATIC const char* firmware_get_partion(void)
{
    hi110x_board_info *bd_info = get_hi110x_board_info();
    if (bd_info->coldboot_partion != NULL) {
        return bd_info->coldboot_partion;
    } else {
        return FIRMWARE_PARTION;
    }
}

STATIC int32_t firmware_partion_write(const char *partion, loff_t from, uint8_t *buf, size_t len)
{
    int ret;
    int fd;
    /*lint --e{501,48}*/
    mm_segment_t oldfs = get_fs();
    set_fs(get_ds());
    fd = (int)sys_open(partion, O_RDWR, (int)S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        ps_print_err("sys_open %s block device failed, and fd = %d!\n",
                     partion, fd);
        ret = -ENODEV;
        goto out;
    }

    ret = (int)sys_lseek((unsigned int)fd, from, SEEK_SET);
    if (ret == -1) {
        ps_print_err("sys_seek error, from = 0x%llx, len = 0x%zx, ret = 0x%x.\n",
                     from, len, ret);
        ret = -EIO;
        goto out;
    }

    ret = (int)sys_write((unsigned int)fd, (char *)buf, len);
    if (ret == -1) {
        ps_print_err("sys_write error, from = 0x%llx, len = 0x%zx, ret = 0x%x.\n",
                     from, len, ret);
        ret = -EIO;
        goto out;
    }

    ret = sys_fsync(fd);
    if (ret < 0) {
        ps_print_err("sys_sync error, from = 0x%llx, len = 0x%zx, ret = 0x%x.\n",
                     from, len, ret);
        ret = -EIO;
        goto out;
    }
    sys_close((unsigned int)fd);
    set_fs(oldfs);
    return SUCC;
out:
    if (fd >= 0) {
        sys_close((unsigned int)fd);
    }
    set_fs(oldfs);
    return ret;
}

STATIC int32_t firmware_file_save(const char *partion, os_kernel_file *fp, uint32_t file_len, uint32_t flash_offset)
{
    uint32_t per_send_len;
    uint32_t send_count;
    int32_t rdlen;
    int32_t ret;
    uint32_t i;
    uint32_t offset = 0;

    uint8_t *buf = NULL;
    uint32_t buf_len = MIN_FIRMWARE_FILE_TX_BUF_LEN;

    buf = oal_memalloc(buf_len);
    if (buf == NULL) {
        return -EFAIL;
    }

    per_send_len = (buf_len > file_len) ? file_len : buf_len;
    send_count = (file_len + per_send_len - 1) / per_send_len;

    for (i = 0; i < send_count; i++) {
        rdlen = oal_file_read_ext(fp, fp->f_pos, buf, per_send_len);
        if (rdlen > 0) {
            ps_print_dbg("len of kernel_read is [%d], i=%d\n", rdlen, i);
            fp->f_pos += rdlen;
        } else {
            oal_free(buf);
            ps_print_err("len of kernel_read is error! ret=[%d], i=%d\n", rdlen, i);
            return (rdlen < 0) ? rdlen : -EFAIL;
        }

        ret = firmware_partion_write(partion, flash_offset + offset, buf, rdlen);
        if (ret < 0) {
            oal_free(buf);
            ps_print_err("firmware partion write file data fail\n");
            return -EFAIL;
        }
        ps_print_dbg("firmware_partion_write ret[%d] offset[%d]:write_len[%d]\n",
                     ret, flash_offset + offset, rdlen);
        offset += rdlen;
    }

    if (offset != file_len) {
        oal_free(buf);
        ps_print_err("file write len is err! write len is [%d], file len is [%d]\n", offset, file_len);
        return -EFAIL;
    }

    oal_free(buf);
    return SUCC;
}

STATIC int32_t firmware_set_firmware_file_desc(const char *partion, cmd_type_info *cfg_cmd_info)
{
    unsigned long addr;
    char *path = NULL;
    int32_t ret;
    uint32_t file_len;
    uint32_t i;
    uint32_t flash_offset = sizeof(firmware_flash_map);
    firmware_file_desc *file_desc = NULL;
    uint32_t file_count;
    os_kernel_file *fp = NULL;

    ret = parse_file_cmd(cfg_cmd_info->cmd_para, &addr, &path);
    if (ret < 0) {
        ps_print_err("parse file cmd fail, cmd[%s]!\n", cfg_cmd_info->cmd_para);
        return -EFAIL;
    }
    ret = file_open_get_len(path, &fp, &file_len);
    if (ret < 0) {
        ps_print_err("parse file len fail, path[%s]!\n", path);
        return -EFAIL;
    }

    file_desc = g_firmware_map.file_cfg.file_desc;
    file_count = g_firmware_map.file_cfg.count;
    for (i = 0; i < file_count; i++) {
        flash_offset += (file_desc + i)->file_len;
    }

    ret = firmware_file_save(partion, fp, file_len, flash_offset);
    if (ret == SUCC) {
        (file_desc + i)->flash_offset = flash_offset;
        (file_desc + i)->file_len = file_len;
        (void)strncpy_s((file_desc + i)->file_path, DOWNLOAD_FILE_PATH_LEN, path, strlen(path));
        g_firmware_map.file_cfg.count++;
        ps_print_info("firmware save file desc[%d]:flash_offset[%d], file_len[%d], file_path[%s]\n",
                      i, flash_offset, file_len, path);
    } else {
        ps_print_info("firmware save file fail, file_path[%s]\n", path);
    }

    oal_file_close(fp);
    return ret;
}

STATIC void firmware_cfg_cmd_save(const char *partion)
{
    int32_t ret;
    int32_t i;
    uint32_t cmd_count;
    cmd_type_info *cfg_cmd_info = NULL;
    cmd_type_info *save_cmd_info = NULL;

    cmd_count = g_cfg_info.count[BFGX_CFG];
    if (cmd_count >= DOWNLOAD_CMD_COUNT) {
        ps_print_err("cmd save error cfg_count: %d, max_save_count: %d\n", cmd_count, DOWNLOAD_CMD_COUNT);
        return;
    }

    cfg_cmd_info = g_cfg_info.cmd[BFGX_CFG];
    save_cmd_info = g_firmware_map.cmd_cfg.cmd;

    for (i = 0; i < cmd_count; i++) {
        ret = memcpy_s(save_cmd_info + i, sizeof(cmd_type_info), cfg_cmd_info + i, sizeof(cmd_type_info));
        if (unlikely(ret != EOK)) {
            ps_print_err("cmd cfg info copy failed\n");
            return;
        }

        if ((cfg_cmd_info + i)->cmd_type == FILE_TYPE_CMD) {
            ret = firmware_set_firmware_file_desc(partion, cfg_cmd_info + i);
            if (ret != SUCC) {
                return;
            }
        }
        ps_print_info("firmware save cmd[%d]:type[%d], name[%s], para[%s]\n",
                      i, (cfg_cmd_info + i)->cmd_type, (cfg_cmd_info + i)->cmd_name, (cfg_cmd_info + i)->cmd_para);
    }
    g_firmware_map.cmd_cfg.count = cmd_count;
    ps_print_info("firmware save cmd count:%d\n", cmd_count);
}

STATIC void firmware_cali_data_save(void)
{
    int32_t ret;
    uint32_t len;
    bfgx_cali_data_stru *cali_data = &g_firmware_map.cali_data;

    ret = get_bfgx_cali_data((uint8_t *)cali_data, &len, sizeof(bfgx_cali_data_stru));
    if (ret == SUCC) {
        ps_print_info("firmware bfgx cali data get succ\n");
    } else {
        ps_print_info("firmware bfgx cali data get fail\n");
    }
}

void firmware_coldboot_partion_save(void)
{
    int32_t ret;
    const char *partion = firmware_get_partion();

    firmware_cfg_cmd_save(partion);
    firmware_cali_data_save();

    ret = firmware_partion_write(partion, 0, (uint8_t *)&g_firmware_map, sizeof(g_firmware_map));
    if (ret == SUCC) {
        ps_print_info("firmware partion write firmware map data succ\n");
    } else {
        ps_print_info("firmware partion write firmware map data fail\n");
    }
}
#endif