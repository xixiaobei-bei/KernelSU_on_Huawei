#include <product_config.h>
#include <osl_types.h>
#include <osl_list.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <osl_malloc.h>
#include <osl_irq.h>
#include <securec.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/atomic.h>
#include <linux/miscdevice.h>
#include <linux/suspend.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/err.h>
#include <linux/syscalls.h>
#include <bsp_slice.h>
#include "dump_area.h"
#include "dump_core.h"
#include "dump_debug.h"
#include "dump_config.h"
#include "dump_logzip.h"

#undef  THIS_MODU
#define THIS_MODU mod_dump

struct dump_zip_stru g_zip_ctrl;

static unsigned int compdev_poll(struct file *file, poll_table *wait)
{
    unsigned int ret = 0;
    dump_error("poll waiting.\n");
    poll_wait(file, &g_zip_ctrl.comp_log_ctrl->wq, wait);
    ret = g_zip_ctrl.comp_log_ctrl->trigger_flag ? POLLIN : 0;

    dump_error("poll done once.\n");

    return ret;
}

static ssize_t compdev_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{/*lint --e{715} suppress pos not referenced*/
    ssize_t ret = 1; /*lint !e446 (Warning -- side effect in initializer)*/
    dump_error("read zipdev.\n");
    if(NULL == buf)
    {
        return BSP_ERROR;
    }
    mutex_lock(&g_zip_ctrl.comp_log_ctrl->mutex);
    ret = copy_to_user(buf, (void *)&g_zip_ctrl.zipintf_info, sizeof(g_zip_ctrl.zipintf_info));
    if(ret)
    {
        dump_error("copy to user failed.\n");
    }
    g_zip_ctrl.comp_log_ctrl->trigger_flag = 0;

    mutex_unlock(&g_zip_ctrl.comp_log_ctrl->mutex);

    dump_error("read zipdev done.\n");
    return ret;
}

static int compdev_open(struct inode *inode, struct file *file)
{
    dump_error("open zipdev.\n");
    mutex_lock(&g_zip_ctrl.comp_log_ctrl->mutex);
    g_zip_ctrl.comp_log_ctrl->fopen_cnt++;
    mutex_unlock(&g_zip_ctrl.comp_log_ctrl->mutex);
    dump_error("open zipdev.\n");
    return 0;
}


static int compdev_release(struct inode *inode, struct file *file)
{/*lint --e{715} suppress inode not referenced*/

    dump_error("%s entry\n", __func__);
    return 0;
}

static const struct file_operations comp_dev_fops = {
    .read    = compdev_read,
    .poll    = compdev_poll,
    .open    = compdev_open,
    .release = compdev_release,
};


s32 dump_comp_dev_register(struct zipintf_info *zip_info)
{
    s32 ret = 0;

    g_zip_ctrl.comp_log_ctrl = kzalloc(sizeof(*g_zip_ctrl.comp_log_ctrl), GFP_KERNEL);
    if (g_zip_ctrl.comp_log_ctrl == NULL)
    {
        ret = EINVAL;
        goto out;
    }

    g_zip_ctrl.comp_log_ctrl->zip_info = zip_info;

    g_zip_ctrl.comp_log_ctrl->misc.minor = MISC_DYNAMIC_MINOR;
    g_zip_ctrl.comp_log_ctrl->misc.name = kstrdup("zipdev", GFP_KERNEL);
    if (unlikely(g_zip_ctrl.comp_log_ctrl->misc.name == NULL)) /*lint !e730: (Info -- Boolean argument to function)*/
    {
        ret = EINVAL;
        goto out_free_log;
    }

    g_zip_ctrl.comp_log_ctrl->misc.fops = &comp_dev_fops;
    g_zip_ctrl.comp_log_ctrl->misc.parent = NULL;

    init_waitqueue_head(&g_zip_ctrl.comp_log_ctrl->wq);
    mutex_init(&g_zip_ctrl.comp_log_ctrl->mutex);

    /* finally, initialize the misc device for this log */
    ret = misc_register(&g_zip_ctrl.comp_log_ctrl->misc);
    if (unlikely(ret)) /*lint !e730: (Info -- Boolean argument to function)*/
    {
        dump_error("failed to register misc device for log '%s'!\n", g_zip_ctrl.comp_log_ctrl->misc.name); /*lint !e429*/
        goto out_free_log;
    }

    dump_error("created zip dev '%s'\n", g_zip_ctrl.comp_log_ctrl->misc.name);
    return 0; /*lint !e429*/


out_free_log:
    kfree(g_zip_ctrl.comp_log_ctrl);
        g_zip_ctrl.comp_log_ctrl = NULL;
out:
    return ret;
}



int dump_get_compress_list(void)
{
    struct device_node* dev = NULL;
    u32 listsize = 0;
    u32 index = 0;
    char * temp = NULL;

    dev = of_find_compatible_node(NULL,NULL,"hisilicon,dump_compress");
    if(NULL == dev)
    {
        dump_error("dump compress dts node no find!\n");
        return -1;
    }

    if(of_property_read_u32(dev, "listsize", &listsize))
    {
        listsize = 0;
        dump_error("fail to get listsize!\n");
        return 0;
    }
    g_zip_ctrl.zipintf_info.mfilenum = listsize;

    for(index = 0; index < listsize; index++)
    {
        if(of_property_read_string_index(dev, "dump_compress_name", index, (const char **)&temp))
        {
            dump_error("fail to read dump_compress_name\n");
        }
        else
        {
            if(0 > snprintf_s(g_zip_ctrl.zipintf_info.pfile_list[index], MAX_COMPRESS_FILE_NAME, MAX_COMPRESS_FILE_NAME - 1, "%s",(char *)temp))
            {
                dump_error("snprintf err\n");
            }
        }
    }

    return 0;
}

s32 dump_trigger_compress(const char * logpath, int pathlen, struct dump_file_save_info_s * datainfo)
{
    if(pathlen >= COMPRESS_FILE_PATH_LEN)
    {
        dump_error("pathlen %d too big.\n", pathlen);
        return BSP_ERROR;
    }
    dump_error("dump_trigger_compress here for path %s.\n", logpath);

    if(NULL == g_zip_ctrl.comp_log_ctrl || NULL == logpath || NULL == datainfo)
    {
        dump_error("param is NULL.\n");
        return BSP_ERROR;
    }

    if((&g_zip_ctrl.comp_log_ctrl->wq)!=NULL)
    {
        if(0 > snprintf_s(g_zip_ctrl.zipintf_info.mfilepath, COMPRESS_FILE_PATH_LEN, COMPRESS_FILE_PATH_LEN - 1, "%s", (char *)logpath))
        {
            dump_error("snprintf err\n");
            return BSP_ERROR;
        }
        if(memcpy_s(&g_zip_ctrl.zipintf_info.saveinfo, sizeof(struct dump_file_save_info_s), datainfo, sizeof(struct dump_file_save_info_s)))
        {
            dump_error("memcpy err\n");
            return BSP_ERROR;
        }
        dump_error("dump_trigger_compress wakeup for path %s, file %s.\n", g_zip_ctrl.zipintf_info.mfilepath, g_zip_ctrl.zipintf_info.pfile_list[0]);
        mutex_lock(&g_zip_ctrl.comp_log_ctrl->mutex);
        g_zip_ctrl.comp_log_ctrl->trigger_flag = 1;
        mutex_unlock(&g_zip_ctrl.comp_log_ctrl->mutex);
        wake_up_interruptible(&g_zip_ctrl.comp_log_ctrl->wq);
        dump_debug_record(DUMP_DEBUG_TRIGER_PRESS,bsp_get_slice_value());
        return BSP_OK;
    }
    else
    {
        return BSP_ERROR;
    }

}

void dump_wait_compress_done(const char *log_path)
{
    char compfilename[256] = {0};
    int count = 0;
    if(NULL == log_path)
    {
        dump_error("log_path err\n");
        return;
    }

    //wait compress done
    dump_error("[0x%x] wait compress done\n",bsp_get_slice_value());

    dump_debug_record(DUMP_DEBUG_START_PRESS,bsp_get_slice_value());
    if(g_zip_ctrl.zipintf_info.mfilenum > 0)
    {
        count = 0;
        if(0 > snprintf_s(compfilename, COMPRESS_FILE_PATH_LEN, COMPRESS_FILE_PATH_LEN - 1, "%sCOMPDONE", log_path))
        {
            dump_error("snprintf err.\n");
        }
        while(sys_access(compfilename,0))
        {
            if(count++ < 1000)
                msleep(20);
            else
                break;
        }
    }
    dump_error("[0x%x] compress done with count %d.\n",bsp_get_slice_value(), count);
}

void dump_logzip_init(void)
{
    if(dump_get_compress_list())
    {
        dump_error("get compress list failed !\n");
        return;
    }

    if(dump_comp_dev_register(&g_zip_ctrl.zipintf_info))
    {
        return;
    }
    else
    {
        dump_ok("[init]logzip ok\n");
        return;
    }
}

