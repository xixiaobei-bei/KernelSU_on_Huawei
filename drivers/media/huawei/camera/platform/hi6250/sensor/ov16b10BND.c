 


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <securec.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define SENSOR_REG_SETTING_DELAY_1_MS 1
#define SENSOR_REG_SETTING_DELAY_5_MS 5
#define SENSOR_REG_SETTING_DELAY_10_MS 10

#define GPIO_LOW_STATE       0
#define GPIO_HIGH_STATE      1

extern struct hw_csi_pad hw_csi_pad;
static hwsensor_vtbl_t s_ov16b10BND_vtbl;

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737

static char *sensor_dts_name = "OV16B10BND_VENDOR";

int ov16b10BND_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hw_ov16b10BND_power_up_setting[] = {

    //disable sec camera reset
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM1 AFVDD 3V
    {
        .seq_type      = SENSOR_VCM_AVDD,
        .data          = (void*)"cameravcm-vcc",
        .config_val    = LDO_VOLTAGE_V3PV,
        .sensor_index  = SENSOR_INDEX_INVALID,
        .delay         = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM1 VCM PD Enable
    {
        .seq_type      = SENSOR_VCM_PWDN,
        .config_val    = SENSOR_GPIO_HIGH,
        .sensor_index  = SENSOR_INDEX_INVALID,
        .delay          = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM1 IOVDD 1.80V
    {
        .seq_type     = SENSOR_IOVDD,
        .data         = (void*)"main-sensor-iovdd",
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    
    //MCAM1 AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD2,
        .data         = (void*)"main-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    
    //MCAM1 DVDD 1.1V
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD,
        .data         = (void*)"slave-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //SCAM DVDD1.2V
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_5_MS,
    },
    
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },


};

struct sensor_power_setting hw_ov16b10BND_power_down_setting[] = {
    {
        .seq_type     = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_10_MS,
    },
    
    {
        .seq_type     = SENSOR_RST,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //SCAM DVDD1.2V
    {
        .seq_type     = SENSOR_DVDD,
        .config_val   = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD,
        .data         = (void*)"slave-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //MCAM1 DVDD 1.1V
    {
        .seq_type     = SENSOR_DVDD2,
        .config_val   = LDO_VOLTAGE_1P1V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type     = SENSOR_AVDD2,
        .data         = (void*)"main-sensor-avdd",
        .config_val   = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type     = SENSOR_IOVDD,
        .data         = (void*)"main-sensor-iovdd",
        .config_val   = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },

    //MCAM1 VCM Enable
    {
        .seq_type      = SENSOR_VCM_PWDN,
        .config_val    = SENSOR_GPIO_LOW,
        .sensor_index  = SENSOR_INDEX_INVALID,
        .delay         = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = SENSOR_REG_SETTING_DELAY_1_MS,
    },
    //disable sec camera reset
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
};

atomic_t volatile ov16b10BND_powered = ATOMIC_INIT(0);
struct mutex ov16b10BND_power_lock;

static sensor_t s_ov16b10BND =
{
    .intf = { .vtbl = &s_ov16b10BND_vtbl, },
    .power_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_ov16b10BND_power_up_setting),
        .power_setting = hw_ov16b10BND_power_up_setting,
    },
    .power_down_setting_array = 
    {
        .size          = ARRAY_SIZE(hw_ov16b10BND_power_down_setting),
        .power_setting = hw_ov16b10BND_power_down_setting,
    },
    .p_atpowercnt      = &ov16b10BND_powered,
};


static const struct of_device_id s_ov16b10BND_dt_match[] =
{
    {
        .compatible = "huawei,ov16b10BND",
        .data       = &s_ov16b10BND.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov16b10BND_dt_match);

static struct platform_driver s_ov16b10BND_driver =
{
    .driver =
    {
        .name           = "huawei,ov16b10BND",
        .owner          = THIS_MODULE,
        .of_match_table = s_ov16b10BND_dt_match,
    },
};

char const* ov16b10BND_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }
    
    sensor = I2S(si);
    
    return sensor->board_info->name;
}

int ov16b10BND_power_up(hwsensor_intf_t* si)
{
    int ret          = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_ov16b10BND.dev, sensor->board_info);
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
    
    if (0 == ret){
        cam_info("%s. power up sensor success.", __func__);
    }
    else{
        cam_err("%s. power up sensor fail.", __func__);
    }
    
    return ret;
}

int ov16b10BND_power_down(hwsensor_intf_t* si)
{
    int ret          = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }

    if (0 == ret ){
        cam_info("%s. power down sensor success.", __func__);
    }
    else{
        cam_err("%s. power down sensor fail.", __func__);
    }
    
    hw_sensor_power_down_config(sensor->board_info);
    
    return ret;
}

int ov16b10BND_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int ov16b10BND_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int ov16b10BND_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor              = NULL;
    struct sensor_cfg_data *cdata = NULL;
    uint32_t module_id_0 = 0;
    uint32_t module_id_1 = 0;
    struct pinctrl_state *pinctrl_def  = NULL;
    struct pinctrl_state *pinctrl_idle = NULL;
    struct pinctrl *p    = NULL;
    char *sensor_name [] = { "OV16B10BND_OFILM" , "OV16B10BND_LITEON"};
    int rc = 0;

    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    sensor = I2S(si);
    cdata  = (struct sensor_cfg_data *)data;
    rc = memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE);
    if (rc != EOK) {
        cam_err("%s. failed to memset_s write_data.", __func__);
    }
    if (!strncmp(sensor->board_info->name, sensor_dts_name, strlen(sensor_dts_name)))
    {
        p = devm_pinctrl_get(s_ov16b10BND.dev);
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
        if((module_id_0 == GPIO_LOW_STATE) && (module_id_1 == GPIO_LOW_STATE)){
            //ofilm module
	     strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name),  sensor_name[0], strlen(sensor_name[0]));
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else if((module_id_0 == GPIO_HIGH_STATE) && (module_id_1 == GPIO_HIGH_STATE)){
            //liteon module
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[1], strlen(sensor_name[1]));
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else{
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name,
	         strlen(sensor->board_info->name));
            cdata->data = sensor->board_info->sensor_index;
            cam_err("%s failed to get the module id value.\n",__func__);
            rc = 0;
        }

        gpio_free(sensor->board_info->gpios[FSIN].gpio);
        goto pinctrl_exit;
    } else {
        strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor->board_info->name, 
	     strlen(sensor->board_info->name));
        cdata->data = sensor->board_info->sensor_index;
        rc = 0;
        goto matchID_exit;
    }

gpio_error:
    gpio_free(sensor->board_info->gpios[FSIN].gpio);
pinctrl_exit:
    devm_pinctrl_put(p);
matchID_exit:
    if (cdata->data != SENSOR_INDEX_INVALID) {
        cam_info("%s, cdata->cfg.name = %s", __func__,cdata->cfg.name );
    }

    return rc;
}

static hwsensor_vtbl_t s_ov16b10BND_vtbl =
{
    .get_name    = ov16b10BND_get_name,
    .config      = ov16b10BND_config,
    .power_up    = ov16b10BND_power_up,
    .power_down  = ov16b10BND_power_down,
    .match_id    = ov16b10BND_match_id,
    .csi_enable  = ov16b10BND_csi_enable,
    .csi_disable = ov16b10BND_csi_disable,
};

int ov16b10BND_config(hwsensor_intf_t* si, void  *argp)
{
    int ret =0;
    struct sensor_cfg_data *data = NULL;
    static bool ov16b10BND_power_on = false;

    if (NULL == si || NULL == argp){
        cam_err("%s si or argp is null.\n", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov16b10BND cfgtype = %d",data->cfgtype);

    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&ov16b10BND_power_lock);
            if (!ov16b10BND_power_on) {
                ret = si->vtbl->power_up(si);
                if(ret == 0){
                    ov16b10BND_power_on = true;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov16b10BND_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&ov16b10BND_power_lock);
            if (ov16b10BND_power_on) {
                ret = si->vtbl->power_down(si);
                if(ret != 0){
                    cam_err("%s. power_down fail.", __func__);
                }
                ov16b10BND_power_on = false;
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov16b10BND_power_lock);
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

    cam_debug("%s exit %d",__func__, ret);
    return ret;
}

static int32_t ov16b10BND_platform_probe(struct platform_device* pdev)
{
    int rc = 0;

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }
    
    cam_info("enter %s",__func__);
    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_ov16b10BND);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto ov16b10BND_sensor_probe_fail;
        }
    } else {
        cam_err("%s ov16b10BND of_node is NULL.\n", __func__);
        goto ov16b10BND_sensor_probe_fail;
    }

    s_ov16b10BND.dev = &pdev->dev;
    mutex_init(&ov16b10BND_power_lock);

    rc = hwsensor_register(pdev, &s_ov16b10BND.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto ov16b10BND_sensor_probe_fail;
    }
    
    rc = rpmsg_sensor_register(pdev, (void *)&s_ov16b10BND);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(Sensor2Pdev(s_ov16b10BND));
        goto ov16b10BND_sensor_probe_fail;
    }

ov16b10BND_sensor_probe_fail:
    return rc;
}

static int __init ov16b10BND_init_module(void)
{
    cam_info("enter %s",__func__);
    return platform_driver_probe(&s_ov16b10BND_driver, ov16b10BND_platform_probe);
}

static void __exit ov16b10BND_exit_module(void)
{
    rpmsg_sensor_unregister((void *)&s_ov16b10BND);
    hwsensor_unregister(Sensor2Pdev(s_ov16b10BND));
    platform_driver_unregister(&s_ov16b10BND_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(ov16b10BND_init_module);
module_exit(ov16b10BND_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov16b10BND");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

