
#ifndef __DUMP_LOGZIP_H__
#define __DUMP_LOGZIP_H__


#include <linux/miscdevice.h>
#include <linux/suspend.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/err.h>
#include <linux/syscalls.h>
#include "dump_log_agent.h"
#define COMPRESS_FILE_PATH_LEN 256
#define MAX_COMPRESS_FILES         10
#define MAX_COMPRESS_FILE_NAME 32

struct zipintf_info
{
    char mfilepath[COMPRESS_FILE_PATH_LEN];
    u32 mfilenum;
    char pfile_list[MAX_COMPRESS_FILES][MAX_COMPRESS_FILE_NAME];
    struct dump_file_save_info_s saveinfo;
};

struct comp_log {
    struct zipintf_info *zip_info;
    wait_queue_head_t   wq;            /* The wait queue for reader */
    struct miscdevice   misc;          /* The "misc" device representing the log */
    struct mutex        mutex;         /* The mutex that protects the @buffer */
    struct list_head    logs;          /* The list of log channels */
    u32 fopen_cnt;
    u32 trigger_flag;
};

struct dump_zip_stru
{
    struct zipintf_info zipintf_info;
    struct comp_log *comp_log_ctrl;
};
int dump_get_compress_list(void);
s32 dump_trigger_compress(const char * logpath, int pathlen, struct dump_file_save_info_s * datainfo);
void dump_wait_compress_done(const char *log_path);
s32 dump_comp_dev_register(struct zipintf_info *zip_info);
void dump_logzip_init(void);
#endif
