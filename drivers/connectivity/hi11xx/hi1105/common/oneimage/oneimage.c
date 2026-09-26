

/* 头文件包含 */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/stat.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#include <linux/namei.h>
#include "oneimage.h"
#ifdef CONFIG_HWCONNECTIVITY
#include "hisi_oneimage.h"
#endif
#include "board.h"
#include "plat_debug.h"
#include "securec.h"

#ifdef CONFIG_HWCONNECTIVITY
/* global variables */
STATIC char g_hi110x_chip_type[HW_PROC_BUTT][BUFF_LEN];
STATIC struct hisi_proc_info g_hisi_proc_info[] = {
    { .proc_type = HW_PROC_CHIPTYPE,
      .proc_node_name = DTS_COMP_HW_CONNECTIVITY_NAME,
      .proc_pro_name = HW_CONN_PROC_CHIPTYPE_FILE },
    { .proc_type = HW_PROC_SUPP,
      .proc_node_name = DTS_COMP_HW_HISI_SUPP_CONFIG_NAME,
      .proc_pro_name = HW_CONN_PROC_SUPP_FILE },
    { .proc_type = HW_PROC_P2P,
      .proc_node_name = DTS_COMP_HW_HISI_P2P_CONFIG_NAME,
      .proc_pro_name = HW_CONN_PROC_P2P_FILE },
    { .proc_type = HW_PROC_HOSTAPD,
      .proc_node_name = DTS_COMP_HW_HISI_HOSTAPD_CONFIG_NAME,
      .proc_pro_name = HW_CONN_PROC_HOSTAPD_FILE },
    { .proc_type = HW_PROC_FIRMWARE,
      .proc_node_name = DTS_COMP_HW_HISI_FIRMWARE_CONFIG_NAME,
      .proc_pro_name = HW_CONN_PROC_FRIMWARE },
    {},
};

/* 全局变量定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_HWCONNECTIVITY_PC
int is_my_chip(void)
{
    return true;
}

int is_hisi_chiptype(int32_t chip)
{
    return true;
}
#else
int is_my_chip(void)
{
    if (!isMyConnectivityChip(CHIP_TYPE_HI110X)) {
        ps_print_err("cfg dev board chip type is not match hisi, skip driver init\n");
        return false;
    } else {
        ps_print_info("cfg dev board type is matched with hisi, continue\n");
    }
    return true;
}

int is_hisi_chiptype(int32_t chip)
{
    int32_t ret;
    hi110x_board_info *bd_info = NULL;

    if (is_my_chip() == false) {
        return false;
    }

    ret = board_chiptype_init();
    if (ret == BOARD_FAIL) {
        ps_print_err("sub chip type init fail\n");
        return false;
    }

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is null\n");
        return false;
    }

    if (chip != bd_info->chip_nr) {
        return false;
    } else {
        return true;
    }
}
#endif

EXPORT_SYMBOL(is_hisi_chiptype);
#endif

/* Note:symbol printk() has arg.count conflict */
/*lint -e515*/
STATIC int read_from_dts(char *buf, int buf_len, char *node_name, char *property_name)
{
    struct device_node *np = NULL;
    char *dts_pro_name = NULL;
    int ret;

    if (buf == NULL || buf_len <= 0) {
        printk(KERN_ERR "[HW_CONN] invalid input parameters in %s\n", __func__);
        return -EINVAL;
    }

    np = of_find_compatible_node(NULL, NULL, node_name);  // should be the same as dts node compatible property
    if (np == NULL) {
        /* if read fail, default is broadcom */
        printk(KERN_ERR "Unable to find hw_connectivity, using default {%s}\n", node_name);
        ret = strcpy_s(buf, buf_len, CHIP_TYPE_BCM);
        return ((ret == EOK) ? 0 : -ret);
    }

    ret = of_property_read_string(np, (const char *)property_name, (const char **)&dts_pro_name);
    if (ret < 0 || dts_pro_name == NULL) {
        /* if read fail, default is broadcom */
        printk(KERN_ERR "[HW_CONN] %s: get {%s}, ret:%d, using default\n", __func__, property_name, ret);
        ret = strcpy_s(buf, buf_len, CHIP_TYPE_BCM);
    } else {
        ret = strcpy_s(buf, buf_len, dts_pro_name);
    }

    return ((ret == EOK) ? 0 : -ret);
}

STATIC ssize_t hwconn_read_proc_chiptype(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    size_t size = 0;

    size = strlen(g_hi110x_chip_type[HW_PROC_CHIPTYPE]);
    if (size <= 0) {
        printk(KERN_ERR "[HW_CONN] g_hi110x_chip_type is blank\n");
        return 0;
    }

    if (len < size) {
        printk(KERN_ERR "[HW_CONN] chiptype read length = %lu.\n", (unsigned long)len);
        return -EINVAL;
    }

    printk(KERN_INFO "[HW_CONN] chiptype off = %lu\n", (unsigned long)*off);
    if ((unsigned long)*off > 0) {
        return 0;
    }

    printk(KERN_INFO "[HW_CONN] chip type[%s] \n", g_hi110x_chip_type[HW_PROC_CHIPTYPE]);

    if (copy_to_user(buffer, g_hi110x_chip_type[HW_PROC_CHIPTYPE], size)) {
        printk(KERN_ERR "[HW_CONN] chiptype copy_to_user failed!\n");
        return -EFAULT;
    }

    *off += size;
    return size;
}

STATIC ssize_t hwconn_read_proc_supp(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    size_t size = 0;

    size = strlen(g_hi110x_chip_type[HW_PROC_SUPP]);
    if (size <= 0) {
        printk(KERN_ERR "[HW_CONN] supp is blank\n");
        return 0;
    }

    if (len < size) {
        printk(KERN_ERR "[HW_CONN] supp read length = %lu.\n", (unsigned long)len);
        return -EINVAL;
    }

    printk(KERN_INFO "[HW_CONN] supp off = %lu\n", (unsigned long)*off);
    if ((unsigned long)*off > 0) {
        return 0;
    }

    printk(KERN_INFO "[HW_CONN] supp[%s] \n", g_hi110x_chip_type[HW_PROC_SUPP]);

    if (copy_to_user(buffer, g_hi110x_chip_type[HW_PROC_SUPP], size)) {
        printk(KERN_ERR "[HW_CONN] supp copy_to_user failed!\n");
        return -EFAULT;
    }

    *off += size;
    return size;
}

STATIC ssize_t hwconn_read_proc_p2p(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    size_t size = 0;

    size = strlen(g_hi110x_chip_type[HW_PROC_P2P]);
    if (size <= 0) {
        printk(KERN_ERR "[HW_CONN] p2p is blank\n");
        return 0;
    }

    if (len < size) {
        printk(KERN_ERR "[HW_CONN] p2p read length = %lu.\n", (unsigned long)len);
        return -EINVAL;
    }

    printk(KERN_INFO "[HW_CONN] p2p off = %lu\n", (unsigned long)*off);
    if ((unsigned long)*off > 0) {
        return 0;
    }

    printk(KERN_INFO "[HW_CONN] p2p[%s] \n", g_hi110x_chip_type[HW_PROC_P2P]);

    if (copy_to_user(buffer, g_hi110x_chip_type[HW_PROC_P2P], size)) {
        printk(KERN_ERR "[HW_CONN] p2p copy_to_user failed!\n");
        return -EFAULT;
    }

    *off += size;
    return size;
}

STATIC ssize_t hwconn_read_proc_apd(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    size_t size = 0;

    size = strlen(g_hi110x_chip_type[HW_PROC_HOSTAPD]);
    if (size <= 0) {
        printk(KERN_ERR "[HW_CONN] hostapd is blank\n");
        return 0;
    }

    if (len < size) {
        printk(KERN_ERR "[HW_CONN] hostapd read length = %lu.\n", (unsigned long)len);
        return -EINVAL;
    }

    printk(KERN_INFO "[HW_CONN] hostapd off = %lu\n", (unsigned long)*off);
    if ((unsigned long)*off > 0) {
        return 0;
    }

    printk(KERN_INFO "[HW_CONN] hostapd[%s]\n", g_hi110x_chip_type[HW_PROC_HOSTAPD]);

    if (copy_to_user(buffer, g_hi110x_chip_type[HW_PROC_HOSTAPD], size)) {
        printk(KERN_ERR "[HW_CONN] hostapd copy_to_user failed!\n");
        return -EFAULT;
    }

    *off += size;
    return size;
}

STATIC ssize_t hwconn_read_proc_firmware(struct file *filp, char __user *buffer, size_t len, loff_t *off)
{
    size_t size = 0;

    size = strlen(g_hi110x_chip_type[HW_PROC_FIRMWARE]);
    if (size <= 0) {
        printk(KERN_ERR "[HW_CONN] firmware is blank\n");
        return 0;
    }

    if (len < size) {
        printk(KERN_ERR "[HW_CONN] firmware read length = %lu.\n", (unsigned long)len);
        return -EINVAL;
    }

    printk(KERN_INFO "[HW_CONN] firmware off = %lu\n", (unsigned long)*off);
    if ((unsigned long)*off > 0) {
        return 0;
    }

    printk(KERN_INFO "[HW_CONN] firmware[%s]\n", g_hi110x_chip_type[HW_PROC_FIRMWARE]);

    if (copy_to_user(buffer, g_hi110x_chip_type[HW_PROC_FIRMWARE], size)) {
        printk(KERN_ERR "[HW_CONN] firmware copy_to_user failed!\n");
        return -EFAULT;
    }

    *off += size;
    return size;
}

STATIC const struct file_operations g_hwconn_proc_fops[] = {
    {
        .owner = THIS_MODULE,
        .read = hwconn_read_proc_chiptype,
    },
    {
        .owner = THIS_MODULE,
        .read = hwconn_read_proc_supp,
    },
    {
        .owner = THIS_MODULE,
        .read = hwconn_read_proc_p2p,
    },
    {
        .owner = THIS_MODULE,
        .read = hwconn_read_proc_apd,
    },
    {
        .owner = THIS_MODULE,
        .read = hwconn_read_proc_firmware,
    },
    {},
};

STATIC int create_hwconn_proc_file(void)
{
    int ret;
    int index;
    char node_path[NODE_PATH_LEN];
    struct path path;
    struct proc_dir_entry *hwconn_dir = NULL;
    struct proc_dir_entry *hwconn_chiptype_file = NULL;

    ret = kern_path(PROC_CONN_DIR, LOOKUP_FOLLOW, &path);
    if (ret) {
        hwconn_dir = proc_mkdir(HW_CONN_PROC_DIR, NULL);
        if (hwconn_dir == NULL) {
            printk(KERN_ERR "[HW_CONN] proc dir create failed\n");
            ret = -ENOMEM;
            return ret;
        }

        index = HW_PROC_CHIPTYPE;
    } else {
        index = HW_PROC_SUPP;
    }

    for (; index < HW_PROC_BUTT; index++) {
        memset_s((void *)g_hi110x_chip_type[index], sizeof(g_hi110x_chip_type[index]),
            0, sizeof(g_hi110x_chip_type[index]));
        ret = read_from_dts(g_hi110x_chip_type[index], sizeof(g_hi110x_chip_type[index]) - 1,
            g_hisi_proc_info[index].proc_node_name, g_hisi_proc_info[index].proc_pro_name);
        if (ret < 0) {
            printk(KERN_INFO "[HW_CONN read info from dts fail\n");
            continue;
        }

        if (hwconn_dir != NULL) {
            hwconn_chiptype_file = proc_create(g_hisi_proc_info[index].proc_pro_name,
                                               S_IRUGO, hwconn_dir, &g_hwconn_proc_fops[index]);
        } else {
            memset_s(node_path, sizeof(node_path), 0, sizeof(node_path));
            if (snprintf_s(node_path, sizeof(node_path), sizeof(node_path) - 1, "%s/%s",
                           HW_CONN_PROC_DIR, g_hisi_proc_info[index].proc_pro_name) < 0) {
                printk(KERN_ERR "[HW_CONN] snprintf_s failed\n");
            }
            printk(KERN_INFO "[HW_CONN node_path[%s]\n", node_path);
            hwconn_chiptype_file = proc_create(node_path, S_IRUGO, NULL, &g_hwconn_proc_fops[index]);
        }

        if (hwconn_chiptype_file == NULL) {
            printk(KERN_ERR "[HW_CONN] proc info file create failed\n");
            ret = -ENOMEM;
            return ret;
        }
    }

    printk(KERN_INFO "[HW_CONN] hwconn create proc file ok.\n");

    return ret;
}

STATIC int hw_connectivity_probe(struct platform_device *pdev)
{
    int ret;

    unref_param(pdev);
    ret = create_hwconn_proc_file();
    if (ret) {
        printk(KERN_ERR "[HW_CONN] create proc file failed.\n");
        return ret;
    }

    return 0;
}

STATIC void hw_connectivity_shutdown(struct platform_device *pdev)
{
    unref_param(pdev);
    printk(KERN_INFO "[HW_CONN] hw_connectivity_shutdown!\n");
    return;
}

STATIC const struct of_device_id g_hw_connectivity_match_table[] = {
    {
        .compatible = DTS_COMP_HW_CONNECTIVITY_NAME,  // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};

STATIC struct platform_driver g_hw_connectivity_driver = {
    .probe = hw_connectivity_probe,
    .suspend = NULL,
    .remove = NULL,
    .shutdown = hw_connectivity_shutdown,
    .driver = {
        .name = DTS_COMP_HW_CONNECTIVITY_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(g_hw_connectivity_match_table),  // dts required code
    },
};

int hw_misc_connectivity_init(void)
{
    printk(KERN_ERR "[HW_CONN] hw_connectivity_init enter\n");
    return platform_driver_register(&g_hw_connectivity_driver);
}

void hw_misc_connectivity_exit(void)
{
    platform_driver_unregister(&g_hw_connectivity_driver);
}
/*lint +e515*/
#else
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_HWCONNECTIVITY_PC
int is_my_chip(void)
{
    return true;
}

int is_hisi_chiptype(int32_t chip)
{
    int32_t ret;
    hi110x_board_info *bd_info = NULL;

    ret = board_chiptype_init();
    if (ret == BOARD_FAIL) {
        ps_print_err("sub chip type init fail\n");
        return false;
    }

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is null\n");
        return false;
    }

    if (chip != bd_info->chip_nr) {
        return false;
    } else {
        return true;
    }
}

EXPORT_SYMBOL(is_hisi_chiptype);
#else
int is_my_chip(void)
{
    return true;
}

int is_hisi_chiptype(int32_t chip)
{
    return true;
}
#endif
#endif
#endif

