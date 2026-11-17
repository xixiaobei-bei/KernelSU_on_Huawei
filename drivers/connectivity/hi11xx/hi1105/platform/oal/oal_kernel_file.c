

/* 头文件包含 */
#include "oal_kernel_file.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_KERNEL_FILE_C

/* 全局变量定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

/*
 * 内核文件打开函数
 * 参数为文件路径
 * 操作file类型结构变量
 *
 */
/*
 * 函 数 名  : oal_kernel_file_open
 * 功能描述  : 内核文件打开函数
 * 输入参数  : 参数为文件路径,操作file类型结构变量
 * 返 回 值  : OAL_SUCC | OAL_FAIL
 */
oal_file *oal_kernel_file_open(const uint8_t *file_path, int32_t ul_attribute)
{
    oal_file *pst_file = NULL;
    mm_segment_t fs;

    if (oal_unlikely(file_path == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    pst_file = filp_open((char *)file_path, ul_attribute, 0777);
    set_fs(fs);
    if (IS_ERR(pst_file)) {
        return 0;
    }

    return pst_file;
}

/*
 * 函 数 名  : oal_kernel_file_size
 * 功能描述  : 内核文件大小获取
 * 输入参数  : 参数file类型结构变量
 * 返 回 值  : file size
 */
loff_t oal_kernel_file_size(oal_file *pst_file)
{
    struct inode *pst_inode = NULL;
    loff_t ul_fsize = 0;

#ifdef _PRE_WINDOWS_SUPPORT
    return ul_fsize;
#else
    if (oal_unlikely(pst_file == NULL)) {
        oal_warn_on(1);
        return 0;
    }

    if (((pst_file->f_path).dentry) != NULL) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 1, 0))
        pst_inode = file_inode(pst_file);
#else
        pst_inode = ((pst_file->f_path).dentry)->d_inode;
#endif
        ul_fsize = pst_inode->i_size;
    }

    return ul_fsize;
#endif
}

/*
 * 函 数 名  : oal_kernel_file_read
 * 功能描述  : 内核文件读函数
 * 输入参数  : 参数file类型结构变量
 * 返 回 值  : file size
 */
ssize_t oal_kernel_file_read(oal_file *pst_file, uint8_t *pst_buff, loff_t ul_fsize)
{
    loff_t *pos = NULL;

    if (oal_unlikely((pst_file == NULL) || (pst_buff == NULL))) {
        oal_warn_on(1);
        return 0;
    }

    pos = &(pst_file->f_pos);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,18,0))
    return kernel_read(pst_file, (char *)pst_buff, ul_fsize, pos);
#else
    return vfs_read(pst_file, (char *)pst_buff, ul_fsize, pos);
#endif
}

/*
 * 函 数 名  : oal_kernel_file_write
 * 功能描述  : 内核文件读函数
 * 输入参数  : 参数file类型结构变量
 * 返 回 值  : file size
 */
int oal_kernel_file_write(oal_file *pst_file, uint8_t *pst_buf, loff_t fsize)
{
    loff_t *pst_pos = &(pst_file->f_pos);

    if (oal_unlikely((pst_file == NULL) || (pst_buf == NULL))) {
        oal_warn_on(1);
        return 0;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,18,0))
    kernel_write(pst_file, (const void *)pst_buf, fsize, pst_pos);
#else
    vfs_write(pst_file, (const void *)pst_buf, fsize, pst_pos);
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_kernel_file_print
 * 功能描述  : 内核文件读函数
 * 输入参数  : 参数file类型结构变量
 * 返 回 值  : file size
 */
int oal_kernel_file_print(oal_file *pst_file, const char *pc_fmt, ...)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return OAL_SUCC;
#else
    char auc_str_buf[OAL_PRINT_FORMAT_LENGTH]; /* 保存要打印的字符串 buffer used during I/O */
    OAL_VA_LIST pc_args;
    int32_t ret;

    if (oal_any_null_ptr2(pst_file, pc_fmt)) {
        return OAL_FAIL;
    }

    OAL_VA_START(pc_args, pc_fmt);
    ret = vsnprintf_s(auc_str_buf, OAL_PRINT_FORMAT_LENGTH, OAL_PRINT_FORMAT_LENGTH - 1, pc_fmt, pc_args);
    OAL_VA_END(pc_args);

    if (ret < 0) {
        oal_io_print("kernel file str format err\n");
        return OAL_FAIL;
    }
    return oal_kernel_file_write(pst_file, (uint8_t *)auc_str_buf, OAL_STRLEN(auc_str_buf));
#endif
}
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

OAL_STATIC oal_kobject *g_conn_syfs_root_object = NULL;
OAL_STATIC oal_kobject *g_conn_syfs_root_boot_object = NULL;

oal_kobject *oal_get_sysfs_root_object(void)
{
    if (g_conn_syfs_root_object != NULL) {
        return g_conn_syfs_root_object;
    }
    g_conn_syfs_root_object = kobject_create_and_add("hisys", NULL);
    return g_conn_syfs_root_object;
}

oal_kobject *oal_get_sysfs_root_boot_object(void)
{
    OAL_STATIC oal_kobject *root_boot_object = NULL;
    if (g_conn_syfs_root_boot_object) {
        return g_conn_syfs_root_boot_object;
    }
    root_boot_object = oal_get_sysfs_root_object();
    if (root_boot_object == NULL) {
        return NULL;
    }
    g_conn_syfs_root_boot_object = kobject_create_and_add("boot", root_boot_object);
    return g_conn_syfs_root_boot_object;
}

oal_kobject *oal_conn_sysfs_root_obj_init(void)
{
    return oal_get_sysfs_root_object();
}

void oal_conn_sysfs_root_obj_exit(void)
{
    if (g_conn_syfs_root_object != NULL) {
        kobject_del(g_conn_syfs_root_object);
        g_conn_syfs_root_object = NULL;
    }
}

void oal_conn_sysfs_root_boot_obj_exit(void)
{
    if (g_conn_syfs_root_boot_object != NULL) {
        kobject_del(g_conn_syfs_root_boot_object);
        g_conn_syfs_root_boot_object = NULL;
    }
}
oal_module_symbol(oal_get_sysfs_root_object);
oal_module_symbol(oal_get_sysfs_root_boot_object);
oal_module_symbol(oal_conn_sysfs_root_obj_exit);
oal_module_symbol(oal_conn_sysfs_root_boot_obj_exit);
#else
oal_kobject *oal_get_sysfs_root_object(void)
{
    return NULL;
}

oal_kobject *oal_conn_sysfs_root_obj_init(void)
{
    return NULL;
}

void oal_conn_sysfs_root_obj_exit(void)
{
    return;
}

void oal_conn_sysfs_root_boot_obj_exit(void)
{
    return;
}
#endif

/*lint -e19*/
oal_module_symbol(oal_kernel_file_open);
oal_module_symbol(oal_kernel_file_size);
oal_module_symbol(oal_kernel_file_read);
oal_module_symbol(oal_kernel_file_write);
oal_module_symbol(oal_kernel_file_print);
oal_module_license("GPL");
