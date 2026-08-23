
 
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"
#include <linux/pinctrl/consumer.h>
#include "../pmic/hw_pmic.h"

//lint -save -e846 -e866 -e826 -e785 -e838 -e715 -e747 -e774 -e778 -e732 -e731 -e569 -e650 -e31
#define DELAY_1MS            (1)
#define DELAY_0MS            (0)
#define I2S(i) container_of((i), sensor_t, intf)
#define CTL_RESET_HOLD       (0)
#define CTL_RESET_RELEASE    (1)

static hwsensor_vtbl_t s_imx576_vtbl;
static bool power_on_status = false;//false: power off, true:power on
static int imx576_config(hwsensor_intf_t* si, void  *argp);
struct mutex imx576_vce_power_lock;
static hwsensor_intf_t *s_intf = NULL;
static sensor_t *s_sensor = NULL;

static struct sensor_power_setting imx576_power_setting[] = {
    //SEL=0 SWITCH MIPI to S1
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW, //pull down for power down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
//S0 VCM 2.80V [PMU-LDO025]
    {
        .seq_type = SENSOR_AVDD0,
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },	

    //S0 AVDD 2.8V [PMU-LDO013]
    {
        .seq_type = SENSOR_AVDD,
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S0 DVDD 1.05V [PMU-LDO032]
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S0 IOVDD 1.80V [PMU-LDO021]
    {
        .seq_type = SENSOR_IOVDD,
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
	
    //SEL=0 SWITCH MIPI to S1
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH, //pull down
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_0MS,
    },
    //S1 MCLK [ISP-CLK0]
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = 0,
        .delay = DELAY_1MS,
    },
    //S1 RESET [GPIO_032]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = DELAY_1MS,
    },
};

static char const* imx576_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;
    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return NULL;
    }
    sensor = I2S(si);

    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or sensor->board_info is NULL.", __func__);
        return NULL;
    }

    return sensor->board_info->name;
}

static int imx576_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }
    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or sensor->board_info is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }

    if (0 == ret) {
        cam_info("%s. power up sensor success.", __func__);
    } else {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

static int imx576_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }
    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info) {
        cam_err("%s. sensor or sensor->board_info is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }

    if (0 == ret) {
        cam_info("%s. power down sensor success.", __func__);
    } else {
        cam_err("%s. power down sensor fail.", __func__);
    }

    return ret;
}

static int imx576_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

static int imx576_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int imx576_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if(NULL == si || NULL == data) {
        cam_err("%s. si or data is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);

    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info or name is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("%s name:%s", __func__, sensor->board_info->name);

    cdata = (struct sensor_cfg_data *)data;
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static int imx576_config_power_on(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&imx576_vce_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_up)
    {
        cam_err("%s. si,si->vtbl or si->vtbl->power_up is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&imx576_vce_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (!power_on_status){
        ret = si->vtbl->power_up(si);
        if (0 == ret) {
            power_on_status = true;
        } else {
            cam_err("%s. power up fail.", __func__);
        }
    } else {
        cam_err("%s camera has powered on",__func__);
    }

    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&imx576_vce_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int imx576_config_power_off(hwsensor_intf_t* si)
{
    int ret = 0;
    mutex_lock(&imx576_vce_power_lock);

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->power_down)
    {
        cam_err("%s. si,si->vtbl or si->vtbl->power_down is null", __func__);
        /*lint -e455 -esym(455,*)*/
        mutex_unlock(&imx576_vce_power_lock);
        /*lint -e455 +esym(455,*)*/
        return -EINVAL;
    }

    if (power_on_status){
        ret = si->vtbl->power_down(si);
        if (0 != ret) {
            cam_err("%s. power down fail.", __func__);
        }
        power_on_status = false;
    } else {
        cam_err("%s camera has powered off",__func__);
    }
    /*lint -e455 -esym(455,*)*/
    mutex_unlock(&imx576_vce_power_lock);
    /*lint -e455 +esym(455,*)*/

    return ret;
}

static int imx576_config_match_id(hwsensor_intf_t* si, void *argp)
{
    int ret = 0;

    if (NULL == si || NULL == si->vtbl || NULL == si->vtbl->match_id)
    {
        cam_err("%s. si power_up is null", __func__);
        ret = -EINVAL;
    } else {
        ret = si->vtbl->match_id(si,argp);
    }

    return ret;
}

enum camera_metadata_enum_android_hw_dual_primary_mode{
    ANDROID_HW_DUAL_PRIMARY_FIRST  = 0,
    ANDROID_HW_DUAL_PRIMARY_SECOND = 2,
    ANDROID_HW_DUAL_PRIMARY_BOTH   = 3,
};
static int imx576_do_hw_reset(hwsensor_intf_t* si, int ctl, int id)
{
//lint -save -e826 -e778 -e774 -e747
    sensor_t* sensor = I2S(si);
    hwsensor_board_info_t *b_info;
    int ret = 0;

    if((NULL == sensor) || (NULL == sensor->board_info)) {
        cam_warn("%s invalid sensor or board info", __func__);
        return 0;
    }

    b_info = sensor->board_info;
    ret = gpio_request(b_info->gpios[PWDN].gpio, "imx576_pwdn-1");
    if (ret) {
        cam_err("%s requeset pwdn pin failed", __func__);
        return ret;
    }

    if (CTL_RESET_HOLD == ctl) {
        if ((ANDROID_HW_DUAL_PRIMARY_SECOND == id) || (ANDROID_HW_DUAL_PRIMARY_BOTH == id)) {
            ret = gpio_direction_output(b_info->gpios[PWDN].gpio, SENSOR_GPIO_HIGH);
            cam_info("PWDN = GPIO_HIGH");
        }
    } else if (CTL_RESET_RELEASE == ctl) {
        if ((ANDROID_HW_DUAL_PRIMARY_SECOND == id) || (ANDROID_HW_DUAL_PRIMARY_BOTH == id)) {
            ret = gpio_direction_output(b_info->gpios[PWDN].gpio, SENSOR_GPIO_LOW);
            cam_info("PWDN = GPIO_LOW");
            hw_camdrv_msleep(5);
        }else if (ANDROID_HW_DUAL_PRIMARY_FIRST == id) {
            ret = gpio_direction_output(b_info->gpios[PWDN].gpio, SENSOR_GPIO_HIGH);
            cam_info("PWDN = GPIO_HIGH");
        }
    }

    gpio_free(b_info->gpios[PWDN].gpio);

    if (ret) {
        cam_err("%s set pwdn pin failed", __func__);
    } else {
        cam_info("%s: set standby state=%d, mode=%d", __func__, ctl, id);
    }
    //lint -restore
    return ret;
}

static int imx576_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret = 0;

    if ((NULL == si) || (NULL == argp)) {
        cam_err("%s : si or argp is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx576 cfgtype = %d",data->cfgtype);
    switch(data->cfgtype) {
        case SEN_CONFIG_POWER_ON:
            ret = imx576_config_power_on(si);
            break;
        case SEN_CONFIG_POWER_OFF:
            ret = imx576_config_power_off(si);
            break;
        case SEN_CONFIG_WRITE_REG:
        case SEN_CONFIG_READ_REG:
        case SEN_CONFIG_WRITE_REG_SETTINGS:
        case SEN_CONFIG_READ_REG_SETTINGS:
        case SEN_CONFIG_ENABLE_CSI:
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = imx576_config_match_id(si,argp);
            break;
        case SEN_CONFIG_RESET_HOLD:
            ret = imx576_do_hw_reset(si, CTL_RESET_HOLD, data->mode);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            ret = imx576_do_hw_reset(si, CTL_RESET_RELEASE, data->mode);
            break;
        default:
            cam_warn("%s cfgtype(%d) is unknow", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static hwsensor_vtbl_t s_imx576_vtbl =
{
    .get_name = imx576_get_name,
    .config = imx576_config,
    .power_up = imx576_power_up,
    .power_down = imx576_power_down,
    .match_id = imx576_match_id,
    .csi_enable = imx576_csi_enable,
    .csi_disable = imx576_csi_disable,
};

atomic_t volatile imx576VCE_powered = ATOMIC_INIT(0);

static sensor_t s_imx576 =
{
    .intf = { .vtbl = &s_imx576_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(imx576_power_setting),
            .power_setting = imx576_power_setting,
     },
    .p_atpowercnt = &imx576VCE_powered,
};

static const struct of_device_id s_imx576_dt_match[] =
{
    {
        .compatible = "huawei,imx576_vce",
        .data = &s_imx576.intf,
    },
    {

    },/* terminate list */
};

MODULE_DEVICE_TABLE(of, s_imx576_dt_match);

static int32_t imx576_platform_probe(struct platform_device* pdev);
static int32_t imx576_platform_remove(struct platform_device* pdev);

static struct platform_driver s_imx576_driver =
{
    .probe = imx576_platform_probe,
    .remove = imx576_platform_remove,
    .driver =
    {
        .name = "huawei,imx576",
        .owner = THIS_MODULE,
        .of_match_table = s_imx576_dt_match,
    },
};

static int32_t imx576_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_info("enter %s ",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    mutex_init(&imx576_vce_power_lock);
    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_imx576_dt_match, np);
    if (NULL == id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    if (NULL == intf) {
        cam_err("%s intf is NULL", __func__);
        return -ENODEV;
    }

    sensor = I2S(intf);
    if(NULL == sensor){
        cam_err("%s sensor is NULL rc %d", __func__, rc);
        return -ENODEV;
    }
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data rc %d", __func__, rc);
        return -ENODEV;
    }
    sensor->dev = &pdev->dev;

    rc = hwsensor_register(pdev, intf);
    if (rc < 0) {
        cam_err("%s hwsensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_intf = intf;
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(intf);
        s_intf = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;

    return rc;
}

static int32_t imx576_platform_remove(
    struct platform_device * pdev)
{
    if( NULL != s_sensor) {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_intf) {
        hwsensor_unregister(s_intf);
        s_intf = NULL;
    }
    return 0;
}

static int __init imx576_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_imx576_driver,
            imx576_platform_probe);
}

static void __exit imx576_exit_module(void)
{
    if (NULL != s_intf) {
        hwsensor_unregister(s_intf);
        s_intf = NULL;
    }

    platform_driver_unregister(&s_imx576_driver);
}

module_init(imx576_init_module);
module_exit(imx576_exit_module);
MODULE_DESCRIPTION("imx576");
MODULE_LICENSE("GPL v2");
//lint -restore
