

#include "gps_refclk_src_3.h"
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/errno.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/consumer.h>
#endif
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/stat.h>
#include "oal_schedule.h"

#define DTS_COMP_GPS_POWER_NAME "hisilicon,hisi_gps"
#define HI_GPS_REF_CLK_FREQ     49152000

typedef struct {
    struct clk *refclk;
    struct clk *muxclk;
    struct clk *mdmclk0;
    struct clk *mdmclk1;
    struct clk *mdmclk2;
    struct pinctrl *pctrl;
    struct pinctrl_state *pins_normal;
    struct pinctrl_state *pins_idle;
} hi_gps_info;

static struct clk *g_gps_ref_clk = NULL;
static struct clk *g_gps_mux_clk = NULL;
static struct clk *g_mdm2gps_clk0 = NULL;
static struct clk *g_mdm2gps_clk1 = NULL;
static struct clk *g_mdm2gps_clk2 = NULL;
static hi_gps_info *g_hi_gps_info = NULL;

static ssize_t gps_write_proc_nstandby(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
    char gps_nstandby = '0';
    printk(KERN_INFO "[GPS] gps_write_proc_nstandby \n");

    if ((len < 1) || (g_hi_gps_info == NULL)) {
        printk(KERN_ERR "[GPS] gps_write_proc_nstandby g_hi_gps_info is NULL or read length = 0.\n");
        return -EINVAL;
    }

    if (copy_from_user(&gps_nstandby, buffer, sizeof(gps_nstandby))) {
        printk(KERN_ERR "[GPS] gps_write_proc_nstandby copy_from_user failed!\n");
        return -EFAULT;
    }

    if (gps_nstandby == '0') {
        printk(KERN_INFO "[GPS] refclk disable.\n");
        /*lint -e119 */ /* 屏蔽pclint错误识别参数转换 */
        set_gps_ref_clk_enable_hi110x(false, (gps_modem_id_enum)0, (gps_rat_mode_enum)0);
    } else if (gps_nstandby == '1') {
        printk(KERN_INFO "[GPS] refclk SCPLL0 enable.\n");
        set_gps_ref_clk_enable_hi110x(true, (gps_modem_id_enum)0, (gps_rat_mode_enum)0);
    } else if (gps_nstandby == '2') {
        printk(KERN_INFO "[GPS] refclk SCPLL1 enable.\n");
        set_gps_ref_clk_enable_hi110x(true, (gps_modem_id_enum)0, (gps_rat_mode_enum)4);
    } else if (gps_nstandby == '3') {
        printk(KERN_INFO "[GPS] refclk SCPLL2 enable.\n");
        set_gps_ref_clk_enable_hi110x(true, (gps_modem_id_enum)1, (gps_rat_mode_enum)2); /*lint +e119 */
    } else {
        printk(KERN_ERR "[GPS] gps nstandby write error code[%d].\n", gps_nstandby);
    }

    return len;
}

static const struct file_operations g_gps_fops_nstandby = {
    .owner = THIS_MODULE,
    .write = gps_write_proc_nstandby,
};

static int create_gps_proc_file(void)
{
    int ret = 0;
    struct proc_dir_entry *gps_dir = NULL;
    struct proc_dir_entry *gps_nstandby_file = NULL;
    gps_dir = proc_mkdir("hi1102_gps", NULL);
    if (gps_dir == NULL) {
        printk(KERN_ERR "[GPS] proc dir create failed\n");
        ret = -ENOMEM;
        return ret;
    }

    gps_nstandby_file = proc_create("ref_clk", S_IRUGO | S_IWUSR | S_IWGRP | S_IFREG, gps_dir, &g_gps_fops_nstandby);
    if (gps_nstandby_file == NULL) {
        printk(KERN_ERR "[GPS] proc nstandby file create failed\n");
        ret = -ENOMEM;
        return ret;
    }
    printk(KERN_INFO "[GPS] gps create proc file ok. \n");

    return ret;
}

static int hi_gps_pinctrl_set(hi_gps_info *hi_gps, struct device *gps_power_dev)
{
    int ret;
    hi_gps->pctrl = devm_pinctrl_get(gps_power_dev); //lint !e64 屏蔽pclint无法识别返回值类型
    if (oal_is_err_or_null(hi_gps->pctrl)) {
        printk(KERN_ERR "[GPS] pinctrl get error! %p\n", hi_gps->pctrl);
        return -1;
    }

    hi_gps->pins_normal = pinctrl_lookup_state(hi_gps->pctrl, "default"); //lint !e64
    if (oal_is_err_or_null(hi_gps->pins_normal)) {
        printk(KERN_ERR "[GPS] hi_gps->pins_normal lookup error! %p\n", hi_gps->pins_normal);
        devm_pinctrl_put(hi_gps->pctrl);
        return -1;
    }

    hi_gps->pins_idle = pinctrl_lookup_state(hi_gps->pctrl, "idle"); //lint !e64 屏蔽pclint无法识别返回值类型
    if (oal_is_err_or_null(hi_gps->pins_idle)) {
        printk(KERN_ERR "[GPS] hi_gps->pins_idle lookup error! %p\n", hi_gps->pins_idle);
        devm_pinctrl_put(hi_gps->pctrl);
        return -1;
    }

    ret = pinctrl_select_state(hi_gps->pctrl, hi_gps->pins_normal);
    if (ret) {
        printk(KERN_ERR "[GPS] pinctrl_select_state error! %d\n", ret);
        devm_pinctrl_put(hi_gps->pctrl);
        return ret;
    }
    printk(KERN_INFO "[GPS] pinctrl is finish\n");

    return ret;
}

static void hi_gps_pinctrl_deset(hi_gps_info *hi_gps)
{
    devm_pinctrl_put(hi_gps->pctrl);
    hi_gps->pctrl =  NULL;
    hi_gps->pins_normal = NULL;
    hi_gps->pins_idle = NULL;
}

static int hi_gps_clk_set(hi_gps_info *hi_gps, struct device *gps_power_dev)
{
    int ret = 0;
    hi_gps->refclk = devm_clk_get(gps_power_dev, "ref_clk");
    if (oal_is_err_or_null(hi_gps->refclk)) {
        printk(KERN_ERR "[GPS] ref_clk get failed!\n");
        ret = PTR_ERR(hi_gps->refclk);
        goto err_refclk_get;
    }
    g_gps_ref_clk = hi_gps->refclk;

    hi_gps->muxclk = devm_clk_get(gps_power_dev, "mux_clk");
    if (oal_is_err_or_null(hi_gps->muxclk)) {
        printk(KERN_ERR "[GPS] mux_clk get failed!\n");
        ret = PTR_ERR(hi_gps->muxclk);
        goto err_muxclk_get;
    }
    g_gps_mux_clk = hi_gps->muxclk;

    hi_gps->mdmclk0 = devm_clk_get(gps_power_dev, "mdm_clk0");
    if (oal_is_err_or_null(hi_gps->mdmclk0)) {
        printk(KERN_ERR "[GPS] mdm_clk0 get failed!\n");
        ret = PTR_ERR(hi_gps->mdmclk0);
        goto err_mdmclk0_get;
    }
    g_mdm2gps_clk0 = hi_gps->mdmclk0;

    hi_gps->mdmclk1 = devm_clk_get(gps_power_dev, "mdm_clk1");
    if (oal_is_err_or_null(hi_gps->mdmclk1)) {
        printk(KERN_ERR "[GPS] mdm_clk1 get failed!\n");
        ret = PTR_ERR(hi_gps->mdmclk1);
        goto err_mdmclk1_get;
    }
    g_mdm2gps_clk1 = hi_gps->mdmclk1;

    hi_gps->mdmclk2 = devm_clk_get(gps_power_dev, "mdm_clk2");
    if (oal_is_err_or_null(hi_gps->mdmclk2)) {
        printk(KERN_ERR "[GPS] mdm_clk2 get failed!\n");
        ret = PTR_ERR(hi_gps->mdmclk2);
        goto err_mdmclk2_get;
    }
    g_mdm2gps_clk2 = hi_gps->mdmclk2;

    return ret;

err_mdmclk2_get:
    devm_clk_put(gps_power_dev, hi_gps->mdmclk1);
err_mdmclk1_get:
    devm_clk_put(gps_power_dev, hi_gps->mdmclk0);
err_mdmclk0_get:
    devm_clk_put(gps_power_dev, hi_gps->muxclk);
err_muxclk_get:
    devm_clk_put(gps_power_dev, hi_gps->refclk);
err_refclk_get:
    return ret;
}

static void hi_gps_clk_deset(hi_gps_info *hi_gps, struct device *gps_power_dev)
{
    devm_clk_put(gps_power_dev, hi_gps->mdmclk2);
    g_mdm2gps_clk2 = NULL;
    devm_clk_put(gps_power_dev, hi_gps->mdmclk1);
    g_mdm2gps_clk1 = NULL;
    devm_clk_put(gps_power_dev, hi_gps->mdmclk0);
    g_mdm2gps_clk0 = NULL;
    devm_clk_put(gps_power_dev, hi_gps->muxclk);
    g_gps_mux_clk = NULL;
    devm_clk_put(gps_power_dev, hi_gps->refclk);
    g_gps_ref_clk = NULL;
}

static int hi_gps_probe(struct platform_device *pdev)
{
    hi_gps_info *hi_gps = NULL;
    struct device *gps_power_dev = &pdev->dev;
    int ret;

    printk(KERN_INFO "[GPS] start find gps_power\n");
    hi_gps = kzalloc(sizeof(hi_gps_info), GFP_KERNEL);
    if (hi_gps == NULL) {
        printk(KERN_ERR "[GPS] Alloc memory failed\n");
        return -ENOMEM;
    }

    ret = hi_gps_pinctrl_set(hi_gps, gps_power_dev);
    if (ret != 0) {
        goto err_pinctrl_set;
    }

    ret = hi_gps_clk_set(hi_gps, gps_power_dev);
    if (ret != 0) {
        goto err_clk_set;
    }
    printk(KERN_INFO "[GPS] ref clk is finished!\n");

    ret = create_gps_proc_file();
    if (ret) {
        printk(KERN_ERR "[GPS] gps create proc file failed.\n");
        goto err_create_proc_file;
    }

    platform_set_drvdata(pdev, hi_gps);
    g_hi_gps_info = hi_gps;

#ifdef CONFIG_HI110X_GPS_REFCLK_INTERFACE
    /*lint -e611*/ /* 屏蔽可疑强转告警 */
    register_gps_set_ref_clk_func((void *)set_gps_ref_clk_enable_hi110x);
    /*lint +e611*/
    printk(KERN_INFO "[GPS] gps register func pointer succ.\n");
#endif
    return 0;

err_create_proc_file:
     hi_gps_clk_deset(hi_gps, gps_power_dev);
err_clk_set:  
    hi_gps_pinctrl_deset(hi_gps);
err_pinctrl_set:
    kfree(hi_gps);
    hi_gps = NULL;
    g_hi_gps_info = NULL;
    return ret;
}

static void hi_gps_shutdown(struct platform_device *pdev)
{
    hi_gps_info *hi_gps = platform_get_drvdata(pdev);
    printk(KERN_INFO "[GPS] hi_gps_shutdown!\n");

    if (hi_gps == NULL) {
        printk(KERN_ERR "[GPS] hi_gps is NULL,just return.\n");
        return;
    }

#ifdef CONFIG_HI110X_GPS_REFCLK_INTERFACE
    register_gps_set_ref_clk_func(NULL);
#endif

    platform_set_drvdata(pdev, NULL);
    kfree(hi_gps);
    hi_gps = NULL;
    g_hi_gps_info = NULL;
    return;
}

static const struct of_device_id g_gps_power_match_table[] = {
    {
        .compatible = DTS_COMP_GPS_POWER_NAME,  // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};

MODULE_DEVICE_TABLE(of, g_gps_power_match_table);

static struct platform_driver g_hisi_gps_plat_driver = {
    .probe = hi_gps_probe,
    .suspend = NULL,
    .remove = NULL,
    .shutdown = hi_gps_shutdown,
    .driver = {
        .name = "hisi_gps",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(g_gps_power_match_table),  // dts required code
    },
};

int hi_gps_plat_init(void)
{
    int ret = -1;

    if (isMyConnectivityChip(CHIP_TYPE_HI110X)) {
        ret = platform_driver_register(&g_hisi_gps_plat_driver);
        if (ret) {
            printk(KERN_ERR "[GPS] ERROR: unable to register hisi gps plat driver! \n");
        } else {
            printk(KERN_INFO "[GPS] hi_gps_plat_init ok! \n");
        }
    } else {
        printk(KERN_ERR "[GPS] chip type not hi110x");
    }

    return ret;
}

void hi_gps_plat_exit(void)
{
    platform_driver_unregister(&g_hisi_gps_plat_driver);
}

/*lint -e49 -e601 */ /* 屏蔽pclint无法识别gps_modem_id_enum和gps_rat_mode_enum */
int set_gps_ref_clk_enable_hi110x(bool enable, gps_modem_id_enum modem_id, gps_rat_mode_enum rat_mode)
{
    /*lint +e49 +e601 */
    int ret = 0;
    struct clk *parent = NULL;

    printk(KERN_INFO "[GPS] set_gps_ref_clk_enable(%d,%d,%d) \n", enable, modem_id, rat_mode);
    if (oal_is_err_or_null(g_gps_ref_clk) || oal_is_err_or_null(g_gps_mux_clk) ||
        oal_is_err_or_null(g_mdm2gps_clk0) || oal_is_err_or_null(g_mdm2gps_clk1) ||
        oal_is_err_or_null(g_mdm2gps_clk2)) {
        printk(KERN_ERR "[GPS] ERROR: refclk is invalid! \n");
        return -1;
    }

    if (enable) {
        if (gps_rat_mode_cdma == rat_mode) {
            /* SCPLL1 */
            parent = g_mdm2gps_clk1;
            printk(KERN_INFO "[GPS] select CLK1 \n");
        } else if (gps_modem_id_0 == modem_id) {
            /* SCPLL0 */
            parent = g_mdm2gps_clk0;
            printk(KERN_INFO "[GPS] select CLK0 \n");
        } else {
            /* SCPLL2 */
            parent = g_mdm2gps_clk2;
            printk(KERN_INFO "[GPS] select CLK2 \n");
        }

        ret = clk_set_parent(g_gps_mux_clk, parent);
        if (ret < 0) {
            printk(KERN_ERR "[GPS] ERROR: muxclk set parent failed! \n");
            return ret;
        }

        ret = clk_prepare_enable(g_gps_ref_clk);
        if (ret < 0) {
            printk(KERN_ERR "[GPS] ERROR: refclk enable failed! \n");
            return -1;
        }
        printk(KERN_INFO "[GPS] set_g_gps_ref_clk enable finish \n");
    } else {
        clk_disable_unprepare(g_gps_ref_clk);
        printk(KERN_INFO "[GPS] set_gps_ref_clk disable finish \n");
    }

    return 0;
}

MODULE_AUTHOR("DRIVER_AUTHOR");
MODULE_DESCRIPTION("GPS Hi110X Platfrom driver");
MODULE_LICENSE("GPL");

