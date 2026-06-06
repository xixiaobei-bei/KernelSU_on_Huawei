

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_kernel_file.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_KERNEL_FILE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*
 * 函 数 名  : oal_kernel_file_open
 * 功能描述  : 内核文件打开函数
 * 输入参数  : 参数为文件路径,操作file类型结构变量
 * 输出参数  : 无
 * 返 回 值  : OAL_SUCC | OAL_FAIL
 */
oal_file *oal_kernel_file_open(oal_uint8 *file_path, oal_int32 ul_attribute)
{
    oal_file *pst_file;
    if (OAL_UNLIKELY(file_path == NULL)) {
        OAL_WARN_ON(1);
        return NULL;
    }
    pst_file = filp_open((oal_int8 *)file_path, ul_attribute, 0777);
    if (IS_ERR(pst_file)) {
        return 0;
    }

    return pst_file;
}

/*
 * 函 数 名  : oal_kernel_file_size
 * 功能描述  : 内核文件大小获取
 * 输入参数  : 参数file类型结构变量
 * 输出参数  : 无
 * 返 回 值  : file size
 */
loff_t oal_kernel_file_size(oal_file *pst_file)
{
    struct inode *pst_inode;
    loff_t ul_fsize = 0;

    if (OAL_UNLIKELY(pst_file == NULL)) {
        OAL_WARN_ON(1);
        return 0;
    }

    if (((pst_file->f_path).dentry) != OAL_PTR_NULL) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 1, 0))
        pst_inode = file_inode(pst_file);
#else
        pst_inode = ((pst_file->f_path).dentry)->d_inode;
#endif
        ul_fsize = pst_inode->i_size;
    }

    return ul_fsize;
}

/*
 * 函 数 名  : oal_kernel_file_read
 * 功能描述  : 内核文件读函数
 * 输入参数  : 参数file类型结构变量
 * 输出参数  : 无
 * 返 回 值  : file size
 */
ssize_t oal_kernel_file_read(oal_file *pst_file, oal_uint8 *pst_buff, loff_t ul_fsize)
{
    loff_t *pos;

    if (OAL_UNLIKELY((pst_file == NULL) || (pst_buff == NULL))) {
        OAL_WARN_ON(1);
        return 0;
    }

    pos = &(pst_file->f_pos);

    return vfs_read(pst_file, (oal_int8 *)pst_buff, ul_fsize, pos);
}

/*
 * 函 数 名  : oal_kernel_file_write
 * 功能描述  : 内核文件读函数
 * 输入参数  : 参数file类型结构变量
 * 输出参数  : 无
 * 返 回 值  : file size
 */

oal_int oal_kernel_file_write(oal_file *pst_file, oal_uint8 *pst_buf, loff_t fsize)
{
    loff_t *pst_pos = &(pst_file->f_pos);

    if (OAL_UNLIKELY((pst_file == NULL) || (pst_buf == NULL))) {
        OAL_WARN_ON(1);
        return 0;
    }

    vfs_write(pst_file, (oal_int8 *)pst_buf, fsize, pst_pos);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_kernel_file_print
 * 功能描述  : 内核文件读函数
 * 输入参数  : 参数file类型结构变量
 * 输出参数  : 无
 * 返 回 值  : file size
 */
oal_int oal_kernel_file_print(oal_file *pst_file, const oal_int8 *pc_fmt, ...)
{
    oal_int8 auc_str_buf[OAL_PRINT_FORMAT_LENGTH]; /* 保存要打印的字符串 buffer used during I/O */
    OAL_VA_LIST pc_args;
    oal_int32 ret;

    if (OAL_ANY_NULL_PTR2(pst_file, pc_fmt)) {
        return OAL_FAIL;
    }

    OAL_VA_START(pc_args, pc_fmt);
    ret = vsnprintf_s(auc_str_buf, OAL_PRINT_FORMAT_LENGTH, OAL_PRINT_FORMAT_LENGTH - 1, pc_fmt, pc_args);
    OAL_VA_END(pc_args);

    if (ret < 0) {
        OAL_IO_PRINT("kernel file str format err\n");
        return OAL_FAIL;
    }
    return oal_kernel_file_write(pst_file, (oal_uint8 *)auc_str_buf, OAL_STRLEN(auc_str_buf));
}
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT

OAL_STATIC oal_kobject *conn_syfs_root_object = NULL;
OAL_STATIC oal_kobject *conn_syfs_root_boot_object = NULL;

oal_kobject *oal_get_sysfs_root_object(oal_void)
{
    if (conn_syfs_root_object != NULL) {
        return conn_syfs_root_object;
    }
    conn_syfs_root_object = kobject_create_and_add("hisys", OAL_PTR_NULL);
    return conn_syfs_root_object;
}

oal_kobject *oal_get_sysfs_root_boot_object(oal_void)
{
    OAL_STATIC oal_kobject *root_boot_object = NULL;
    if (conn_syfs_root_boot_object) {
        return conn_syfs_root_boot_object;
    }
    root_boot_object = oal_get_sysfs_root_object();
    if (root_boot_object == NULL) {
        return NULL;
    }
    conn_syfs_root_boot_object = kobject_create_and_add("boot", root_boot_object);
    return conn_syfs_root_boot_object;
}

oal_kobject *oal_conn_sysfs_root_obj_init(oal_void)
{
    return oal_get_sysfs_root_object();
}

oal_void oal_conn_sysfs_root_obj_exit(oal_void)
{
    if (conn_syfs_root_object != NULL) {
        kobject_del(conn_syfs_root_object);
        conn_syfs_root_object = NULL;
    }
}

oal_void oal_conn_sysfs_root_boot_obj_exit(oal_void)
{
    if (conn_syfs_root_boot_object != NULL) {
        kobject_del(conn_syfs_root_boot_object);
        conn_syfs_root_boot_object = NULL;
    }
}
oal_module_symbol(oal_get_sysfs_root_object);
oal_module_symbol(oal_get_sysfs_root_boot_object);
oal_module_symbol(oal_conn_sysfs_root_obj_exit);
oal_module_symbol(oal_conn_sysfs_root_boot_obj_exit);
#else
oal_kobject *oal_get_sysfs_root_object(oal_void)
{
    return NULL;
}

oal_kobject *oal_conn_sysfs_root_obj_init(oal_void)
{
    return NULL;
}

oal_void oal_conn_sysfs_root_obj_exit(oal_void)
{
    return;
}

oal_void oal_conn_sysfs_root_boot_obj_exit(oal_void)
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


