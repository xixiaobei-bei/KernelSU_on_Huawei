 

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

static hwsensor_vtbl_t s_hi843BND_vtbl;
static bool power_on_status = false;//false: power off, true:power on
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -save -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -specific(-e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715)*/

static char *sensor_dts_name = "HI843BND_VENDOR";
enum rear_sensor_dvdd_type_t{
	DVDD105V=0,//1.05V
	DVDD120V=1,//1.20V
};

int hi843BND_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hi843BND_power_up_setting[] = {
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
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
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD 1.2V
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


    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
	
    //SCAM RST	
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting hi843BND_power_down_setting[] = {
    //SCAM RST
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
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
    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
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
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

struct sensor_power_setting hi843BND_power_up_setting_dvdd120[] = {
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
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
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
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
        .delay = 0,
    },

    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val =  LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
	
    //SCAM RST
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting hi843BND_power_down_setting_dvdd120[] = {
    //SCAM RST
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val =  LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 IOVDD 1.80V
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
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

struct mutex hi843BND_power_lock;
atomic_t volatile hi843BND_powered = ATOMIC_INIT(0);
static sensor_t s_hi843BND =
{
    .intf = { .vtbl = &s_hi843BND_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hi843BND_power_up_setting),
            .power_setting = hi843BND_power_up_setting,
    },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hi843BND_power_down_setting),
            .power_setting = hi843BND_power_down_setting,
    },
    .p_atpowercnt = &hi843BND_powered,
};

static const struct of_device_id s_hi843BND_dt_match[] =
{
    {
        .compatible = "huawei,hi843BND",
        .data = &s_hi843BND.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_hi843BND_dt_match);

static struct platform_driver s_hi843BND_driver =
{
    .driver =
    {
        .name = "huawei,hi843BND",
        .owner = THIS_MODULE,
        .of_match_table = s_hi843BND_dt_match,
    },
};

char const* hi843BND_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }
    
    sensor = I2S(si);
    return sensor->board_info->name;
}

int hi843BND_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_hi843BND.dev, sensor->board_info);
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
hi843BND_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
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

int hi843BND_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int hi843BND_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int hi843BND_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    uint32_t module_id_0 = 0;
    uint32_t module_id_1 = 0;
    struct pinctrl_state *pinctrl_def  = NULL;
    struct pinctrl_state *pinctrl_idle = NULL;
    struct pinctrl *p    = NULL;
    char *sensor_name [] = { "HI843BND_OFILM", "HI843BND_QTECH"};
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
    if(memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE)){
        cam_err("%s. memset failed.", __func__);
        return -1;
    }
    if (!strncmp(sensor->board_info->name, sensor_dts_name, strlen(sensor_dts_name)))
    {
        p = devm_pinctrl_get(s_hi843BND.dev);
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
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[0], strlen(sensor_name[0])+1);
            cdata->data = sensor->board_info->sensor_index;
            rc = 0;
        }
        else if((module_id_0 == GPIO_HIGH_STATE) && (module_id_1 == GPIO_HIGH_STATE)){
            //qtech module
            strncpy_s(cdata->cfg.name, sizeof(cdata->cfg.name), sensor_name[1], strlen(sensor_name[1])+1);
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

static hwsensor_vtbl_t s_hi843BND_vtbl =
{
    .get_name = hi843BND_get_name,
    .config = hi843BND_config,
    .power_up = hi843BND_power_up,
    .power_down = hi843BND_power_down,
    .match_id = hi843BND_match_id,
    .csi_enable = hi843BND_csi_enable,
    .csi_disable = hi843BND_csi_disable,
    .match_id = hi843BND_match_id,
};

int hi843BND_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;
	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}
    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi843BND cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&hi843BND_power_lock);
            if(false == power_on_status){
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    mutex_unlock(&hi843BND_power_lock);
                    return -1;
                }else{
                    ret = si->vtbl->power_up(si);
                    if(0 == ret){
                        power_on_status = true;
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi843BND_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&hi843BND_power_lock);
            if(true == power_on_status){
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    mutex_unlock(&hi843BND_power_lock);
                    return -1;
                }else{
                    ret = si->vtbl->power_down(si);
                    if(0 != ret){
                        cam_err("%s. power_down fail.", __func__);
                    }
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi843BND_power_lock);
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
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t hi843BND_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    cam_notice("enter %s",__func__);

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }

    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_hi843BND);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto hi843BND_sensor_probe_fail;
        }

		/*CASE 1.2v dvdd, revew;CASE 1.05v use default; Other cases need add*/
		if(DVDD120V == s_hi843BND.board_info->ldo_diverse){
			cam_info("%s change to relative power setting line %d\n", __func__, __LINE__);
			s_hi843BND.power_setting_array.size = ARRAY_SIZE(hi843BND_power_up_setting_dvdd120);
			s_hi843BND.power_setting_array.power_setting = hi843BND_power_up_setting_dvdd120;
			s_hi843BND.power_down_setting_array.size = ARRAY_SIZE(hi843BND_power_down_setting_dvdd120);
			s_hi843BND.power_down_setting_array.power_setting = hi843BND_power_down_setting_dvdd120;
		}
    } else {
        cam_err("%s hi843BND of_node is NULL.\n", __func__);
        goto hi843BND_sensor_probe_fail;
    }
    mutex_init(&hi843BND_power_lock);
    s_hi843BND.dev = &pdev->dev;
    rc = hwsensor_register(pdev, &s_hi843BND.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto hi843BND_sensor_probe_fail;
    }
    rc = rpmsg_sensor_register(pdev, (void*)&s_hi843BND);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(Sensor2Pdev(s_hi843BND));
        goto hi843BND_sensor_probe_fail;
    }
    hi843BND_sensor_probe_fail:
    return rc;
}

static int __init hi843BND_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_hi843BND_driver,
            hi843BND_platform_probe);
}
static void __exit hi843BND_exit_module(void)
{
    rpmsg_sensor_unregister((void *)&s_hi843BND);
    hwsensor_unregister(Sensor2Pdev(s_hi843BND));
    platform_driver_unregister(&s_hi843BND_driver);
}
/*lint -restore*/
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 +esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -e528 -esym(528,*)*/
module_init(hi843BND_init_module);
module_exit(hi843BND_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hi843BND");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

