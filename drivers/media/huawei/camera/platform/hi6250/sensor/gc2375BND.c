 

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "securec.h"
#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define GPIO_LOW_STATE       0
#define GPIO_HIGH_STATE      1

static hwsensor_vtbl_t s_gc2375BND_vtbl;
static bool power_on_status = false;//false: power off, true:power on
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -save -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -specific(-e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715)*/

static char *sensor_dts_name = "GC2375BND_VENDOR";
int gc2375BND_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting gc2375BND_power_up_setting[] = {
    {
        .seq_type = SENSOR_PWDN2,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 5,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM2 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM2 AVDD 2.80V
    {
       .seq_type = SENSOR_AVDD,
       .data = (void*)"main-sensor-avdd",
       .config_val = LDO_VOLTAGE_V2P8V,
       .sensor_index = SENSOR_INDEX_INVALID,
       .delay = 0,
   },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_PWDN2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_RST3,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

struct sensor_power_setting gc2375BND_power_down_setting[] = {
    {
        .seq_type = SENSOR_PWDN2,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_RST3,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM2 AVDD 2.80V
    {
       .seq_type = SENSOR_AVDD,
       .data = (void*)"main-sensor-avdd",
       .config_val = LDO_VOLTAGE_V2P8V,
       .sensor_index = SENSOR_INDEX_INVALID,
       .delay = 1,
    },
    //MCAM2 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

struct mutex gc2375BND_power_lock;
atomic_t volatile gc2375BND_powered = ATOMIC_INIT(0);
static sensor_t s_gc2375BND =
{
    .intf = { .vtbl = &s_gc2375BND_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(gc2375BND_power_up_setting),
            .power_setting = gc2375BND_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(gc2375BND_power_down_setting),
            .power_setting = gc2375BND_power_down_setting,
    },
     .p_atpowercnt = &gc2375BND_powered,
};

static const struct of_device_id s_gc2375BND_dt_match[] =
{
    {
        .compatible = "huawei,gc2375BND",
        .data = &s_gc2375BND.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_gc2375BND_dt_match);

static struct platform_driver s_gc2375BND_driver =
{
    .driver =
    {
        .name = "huawei,gc2375BND",
        .owner = THIS_MODULE,
        .of_match_table = s_gc2375BND_dt_match,
    },
};

char const* gc2375BND_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int gc2375BND_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -1;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_gc2375BND.dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
        return ret;
    }
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

int
gc2375BND_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -1;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

int gc2375BND_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int gc2375BND_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int gc2375BND_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor              = NULL;
    struct sensor_cfg_data *cdata = NULL;
    int32_t module_id = 0;
    char *sensor_name [] = { "GC2375BND_OFILM", "GC2375BND_LITEON"};
    int rc = 0;

    if (!si || !data){
            cam_err("%s : si or data is null", __func__);
            return -1;
    }

    sensor = I2S(si);
    cdata  = (struct sensor_cfg_data *)data;
        memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE);
	if (!strncmp(sensor->board_info->name, sensor_dts_name, strlen(sensor_dts_name))) {
                rc = gpio_request(sensor->board_info->gpios[FSIN].gpio, NULL);
                if(rc < 0) {
                     cam_err("%s failed to request gpio[%d]", __func__, sensor->board_info->gpios[FSIN].gpio);
                     return rc;
                }

                cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);
                rc = gpio_direction_input(sensor->board_info->gpios[FSIN].gpio);
                if (rc < 0) {
                     cam_err("%s failed to config gpio(%d) input.\n",__func__, sensor->board_info->gpios[FSIN].gpio);
                     rc = -1;
                     goto gpio_error;
                }

                mdelay(1);
                module_id = gpio_get_value_cansleep(sensor->board_info->gpios[FSIN].gpio);
                if (module_id < 0) {
                     cam_err("%s failed to get gpio(%d) value(%d).\n",__func__, sensor->board_info->gpios[FSIN].gpio, module_id);
                     rc = -1;
                     goto gpio_error;
                }

                cam_info("%s module_id = %d", __func__,module_id);

                gpio_free(sensor->board_info->gpios[FSIN].gpio);

                if (0 == module_id) {//ofilm
                     strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[0], strlen(sensor_name[0])+1);
                     cdata->data = sensor->board_info->sensor_index;
                }
                else if(1 == module_id){//liteon
                     strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[1], strlen(sensor_name[1])+1);
                     cdata->data = sensor->board_info->sensor_index;
                }else{
                     strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name, strlen(sensor->board_info->name)+1);
                     cam_err("%s module unknown", __func__);
                }
                goto matchID_exit;
    }
    else
    {
        strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name, strlen(sensor->board_info->name)+1);
        cdata->data = sensor->board_info->sensor_index;
        goto matchID_exit;
    }

gpio_error:
    gpio_free(sensor->board_info->gpios[FSIN].gpio);
matchID_exit:
    if (cdata->data != (int)SENSOR_INDEX_INVALID) {
        cam_info("%s, cdata->cfg.name = %s", __func__,cdata->cfg.name );
    }

    return rc;
}

static hwsensor_vtbl_t s_gc2375BND_vtbl =
{
    .get_name = gc2375BND_get_name,
    .config = gc2375BND_config,
    .power_up = gc2375BND_power_up,
    .power_down = gc2375BND_power_down,
    .match_id = gc2375BND_match_id,
    .csi_enable = gc2375BND_csi_enable,
    .csi_disable = gc2375BND_csi_disable,
};

int gc2375BND_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;

    int ret =0;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("gc2375BND cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&gc2375BND_power_lock);
            if(false == power_on_status){
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_up(si);
                    if(0 == ret){
                        power_on_status = true;
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&gc2375BND_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&gc2375BND_power_lock);
            if(true == power_on_status){
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_down(si);
                    if(0 != ret){
                        cam_err("%s. power_down fail.", __func__);
                    }
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&gc2375BND_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            break;
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            if(NULL == si->vtbl->match_id){
                cam_err("%s. si->vtbl->match_id is null.", __func__);
                ret=-EINVAL;
            }else{
                ret = si->vtbl->match_id(si,argp);
            }
            break;
        case SEN_CONFIG_RESET_HOLD:
            cam_warn("%s cfgtype(%d) does not support", __func__, data->cfgtype);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            cam_warn("%s cfgtype(%d) does not support", __func__, data->cfgtype);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t gc2375BND_platform_probe(struct platform_device* pdev)
{
    int rc = 0;

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }

    cam_notice("enter %s",__func__);

    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_gc2375BND);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto gc2375BND_sensor_probe_fail;
        }
    } else {
        cam_err("%s gc2375BND of_node is NULL.\n", __func__);
        goto gc2375BND_sensor_probe_fail;
    }
    s_gc2375BND.dev = &pdev->dev;
    mutex_init(&gc2375BND_power_lock);

    rc = hwsensor_register(pdev, &s_gc2375BND.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto gc2375BND_sensor_probe_fail;
    }

    rc = rpmsg_sensor_register(pdev, (void *)&s_gc2375BND);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(Sensor2Pdev(s_gc2375BND));
        goto gc2375BND_sensor_probe_fail;
    }

gc2375BND_sensor_probe_fail:
    return rc;
}

static int __init gc2375BND_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_gc2375BND_driver,
            gc2375BND_platform_probe);
}

static void __exit gc2375BND_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_gc2375BND);
    hwsensor_unregister(Sensor2Pdev(s_gc2375BND));
    platform_driver_unregister(&s_gc2375BND_driver);
}
/*lint -restore*/
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 +esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -e528 -esym(528,*)*/
module_init(gc2375BND_init_module);
module_exit(gc2375BND_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("gc2375BND");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
