 

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/pinctrl/consumer.h>

#include "securec.h"
#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
static char *sensor_dts_name = "hi846AGASSI2BACK_VENDOR";
static bool power_on_status = false;//false: power off, true:power on
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -save -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -specific(-e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715)*/
//lint -save -e747
char const* hi846AGASSI2BACK_get_name(hwsensor_intf_t* si);
#define GPIO_LOW_STATE       0
#define GPIO_HIGH_STATE      1
int hi846AGASSI2BACK_config(hwsensor_intf_t* si, void  *argp);
int hi846AGASSI2BACK_power_up(hwsensor_intf_t* si);
int hi846AGASSI2BACK_power_down(hwsensor_intf_t* si);
int hi846AGASSI2BACK_match_id(hwsensor_intf_t* si, void * data);
int hi846AGASSI2BACK_csi_enable(hwsensor_intf_t* si);
int hi846AGASSI2BACK_csi_disable(hwsensor_intf_t* si);


static hwsensor_vtbl_t s_hi846AGASSI2BACK_vtbl =
{
    .get_name = hi846AGASSI2BACK_get_name,
    .config = hi846AGASSI2BACK_config,
    .power_up = hi846AGASSI2BACK_power_up,
    .power_down = hi846AGASSI2BACK_power_down,
    .match_id = hi846AGASSI2BACK_match_id,
    .csi_enable = hi846AGASSI2BACK_csi_enable,
    .csi_disable = hi846AGASSI2BACK_csi_disable,
};

static struct sensor_power_setting hi846AGASSI2BACK_power_up_setting[] = {


    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //SCAM DVDD1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM XSHUTDOWN PWDN
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
static struct sensor_power_setting hi846AGASSI2BACK_power_down_setting[] = {
    //MCAM XSHUTDOWN PWDN
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM DVDD1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //disable sec camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
};

struct mutex hi846AGASSI2BACK_power_lock;
atomic_t volatile hi846AGASSI2BACK_powered = ATOMIC_INIT(0);
static sensor_t s_hi846AGASSI2BACK =
{
    .intf = { .vtbl = &s_hi846AGASSI2BACK_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hi846AGASSI2BACK_power_up_setting),
            .power_setting = hi846AGASSI2BACK_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hi846AGASSI2BACK_power_down_setting),
            .power_setting = hi846AGASSI2BACK_power_down_setting,
    },
    .p_atpowercnt = &hi846AGASSI2BACK_powered,
};

static const struct of_device_id s_hi846AGASSI2BACK_dt_match[] =
{
    {
        .compatible = "huawei,hi846AGASSI2BACK",
        .data = &s_hi846AGASSI2BACK.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_hi846AGASSI2BACK_dt_match);

static struct platform_driver s_hi846AGASSI2BACK_driver =
{
    .driver =
    {
        .name = "huawei,hi846AGASSI2BACK",
        .owner = THIS_MODULE,
        .of_match_table = s_hi846AGASSI2BACK_dt_match,
    },
};

char const* hi846AGASSI2BACK_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

int hi846AGASSI2BACK_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_hi846AGASSI2BACK.dev, sensor->board_info);
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

int hi846AGASSI2BACK_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("111enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
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

int hi846AGASSI2BACK_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int hi846AGASSI2BACK_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

int hi846AGASSI2BACK_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor              = NULL;
    struct sensor_cfg_data *cdata = NULL;
    uint32_t module_id_0 = 0;
    uint32_t module_id_1 = 0;
    struct pinctrl_state *pinctrl_def  = NULL;
    struct pinctrl_state *pinctrl_idle = NULL;
    struct pinctrl *p    = NULL;
    static const char * const sensor_name[] = { "hi846AGASSI2BACK_HOLITECH", "hi846AGASSI2BACK_OFILM", "hi846AGASSI2BACK_LITEARRAY"};
    int rc = 0;

    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -1;
    }

    cdata = (struct sensor_cfg_data *)data;
    memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE);
    if (!strncmp(sensor->board_info->name, sensor_dts_name, strlen(sensor_dts_name)))
    {

        p = devm_pinctrl_get(s_hi846AGASSI2BACK.dev);
        if (IS_ERR_OR_NULL(p)) {
            cam_err("could not get pinctrl.\n");
            rc = -1;
            goto matchID_exit;
        }

        rc = gpio_request(sensor->board_info->gpios[FSIN].gpio, NULL);

        if(rc < 0) {
            cam_err("%s failed to request gpio[%d]", __func__, sensor->board_info->gpios[FSIN].gpio);
            rc = -1;
            goto pinctrl_exit;
        }

        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);


        pinctrl_def = pinctrl_lookup_state(p, "default");
        if (IS_ERR_OR_NULL(pinctrl_def)) {
            cam_err("could not get defstate.\n");
            rc = -1;
            goto gpio_error;
        }

        pinctrl_idle = pinctrl_lookup_state(p, "idle");
        if (IS_ERR_OR_NULL(pinctrl_idle)) {
            pr_err("could not get idle defstate.\n");
            rc = -1;
            goto gpio_error;
        }

        /*PULL UP*/
        rc = pinctrl_select_state(p, pinctrl_def);
        if (rc) {
            cam_err("could not set pins to default state.\n");
            rc = -1;
            goto gpio_error;
        }

        mdelay(1);
        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);
        rc = gpio_direction_input(sensor->board_info->gpios[FSIN].gpio);
        if (rc < 0) {
            cam_err("%s failed to config gpio(%d) input.\n",__func__, sensor->board_info->gpios[FSIN].gpio);
            rc = -1;
            goto gpio_error;
        }

        module_id_1 = gpio_get_value(sensor->board_info->gpios[FSIN].gpio);

        /*PULL DOWN*/
        rc = pinctrl_select_state(p, pinctrl_idle);
        if (rc) {
            cam_err("could not set pins to idle state.\n");
            rc = -1;
            goto gpio_error;
        }

        mdelay(1);
        cam_info("%s gpio[%d].", __func__, sensor->board_info->gpios[FSIN].gpio);

        rc = gpio_direction_input(sensor->board_info->gpios[FSIN].gpio);
        if (rc < 0) {
            cam_err("%s failed to config gpio(%d) input.\n",__func__, sensor->board_info->gpios[FSIN].gpio);
            rc = -1;
            goto gpio_error;
        }

        module_id_0 = gpio_get_value(sensor->board_info->gpios[FSIN].gpio);

        cam_info("%s module_id_0 %d module_id_1 %d .\n",__func__, module_id_0, module_id_1);
        if((module_id_0 == GPIO_HIGH_STATE) && (module_id_1 == GPIO_HIGH_STATE)){
            //holitech module
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[0], strlen(sensor_name[0])+1);
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else if((module_id_0 == GPIO_LOW_STATE) && (module_id_1 == GPIO_LOW_STATE)){
            //ofilm module
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[1], strlen(sensor_name[1])+1);
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        } else if ((module_id_0 == GPIO_LOW_STATE) && (module_id_1 == GPIO_HIGH_STATE)) {
            /* latearray module */
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[2], strlen(sensor_name[2])+1);
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else{
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name, strlen(sensor->board_info->name)+1);
            cdata->data = sensor->board_info->sensor_index;
            cam_err("%s failed to get the module id value.\n",__func__);
            rc = 0;
        }

        gpio_free(sensor->board_info->gpios[FSIN].gpio);
        goto pinctrl_exit;
    } else {
        strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name, strlen(sensor->board_info->name)+1);
        cdata->data = sensor->board_info->sensor_index;
        rc = 0;
        goto matchID_exit;
    }

gpio_error:
    gpio_free(sensor->board_info->gpios[FSIN].gpio);
pinctrl_exit:
    devm_pinctrl_put(p);
matchID_exit:
    if (cdata->data != SENSOR_INDEX_INVALID) {/*lint !e650*/
        cam_info("%s, cdata->cfg.name = %s", __func__,cdata->cfg.name );
    }

    return rc;
}

int hi846AGASSI2BACK_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;

	if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
	}

    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi846AGASSI2BACK cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&hi846AGASSI2BACK_power_lock);
            if(false == power_on_status){
                ret = si->vtbl->power_up(si);
                if (ret == 0) {
                    power_on_status = true;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi846AGASSI2BACK_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&hi846AGASSI2BACK_power_lock);
            if(true == power_on_status){
                ret = si->vtbl->power_down(si);
                if (ret != 0) {
                    cam_err("%s. power_down fail.", __func__);
                }
                power_on_status = false;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi846AGASSI2BACK_power_lock);
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
            ret = si->vtbl->match_id(si,argp);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t hi846AGASSI2BACK_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    cam_notice("enter %s",__func__);

    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_hi846AGASSI2BACK);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto hi846AGASSI2BACK_sensor_probe_fail;
        }
    } else {
        cam_err("%s hi846AGASSI2BACK of_node is NULL.\n", __func__);
        goto hi846AGASSI2BACK_sensor_probe_fail;
    }
    s_hi846AGASSI2BACK.dev = &pdev->dev;
    mutex_init(&hi846AGASSI2BACK_power_lock);
    rc = hwsensor_register(pdev, &s_hi846AGASSI2BACK.intf);
    rc = rpmsg_sensor_register(pdev, (void*)&s_hi846AGASSI2BACK);

hi846AGASSI2BACK_sensor_probe_fail:
    return rc;
}

static int __init hi846AGASSI2BACK_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_hi846AGASSI2BACK_driver,
            hi846AGASSI2BACK_platform_probe);
}

static void __exit hi846AGASSI2BACK_exit_module(void)
{
    hwsensor_unregister(Sensor2Pdev(s_hi846AGASSI2BACK));
    platform_driver_unregister(&s_hi846AGASSI2BACK_driver);
}
/*lint -restore*/
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 +esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -e528 -esym(528,*)*/
module_init(hi846AGASSI2BACK_init_module);
module_exit(hi846AGASSI2BACK_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hi846AGASSI2BACK");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
