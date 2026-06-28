

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of((i), sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define POWER_DELAY_0        0//delay 0 ms
#define POWER_DELAY_1        1//delay 1 ms
#define SENSOR_INDEX_0       0// sensor index is 0
#define SENSOR_INDEX_2       2// sensor index is 2

//lint -save -e846 -e514 -e866 -e30 -e84 -e785 -e64 -e826 -e838 -e715 -e747 -e778 -e774 -e732 -e650 -e31 -e731 -e528 -e753 -e737
extern struct hw_csi_pad hw_csi_pad;
char const* imx486hybird_get_name(hwsensor_intf_t* si);
int imx486hybird_config(hwsensor_intf_t* si, void  *argp);
int imx486hybird_power_up(hwsensor_intf_t* si);
int imx486hybird_power_down(hwsensor_intf_t* si);
int imx486hybird_match_id(hwsensor_intf_t* si, void * data);
int imx486hybird_csi_enable(hwsensor_intf_t* si);
int imx486hybird_csi_disable(hwsensor_intf_t* si);

static hwsensor_vtbl_t s_imx486hybird_vtbl =
{
    .get_name = imx486hybird_get_name,
    .config = imx486hybird_config,
    .power_up = imx486hybird_power_up,
    .power_down = imx486hybird_power_down,
    .match_id = imx486hybird_match_id,
    .csi_enable = imx486hybird_csi_enable,
    .csi_disable = imx486hybird_csi_disable,
};
static bool power_on_status = false;//false: power off, true:power on

static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;

struct sensor_power_setting hw_imx486hybird_power_up_setting[] = {
    //disable sub cam reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //sub cam AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //sub cam AVDD1_EN 1.8V
    {
        .seq_type = SENSOR_AVDD1_EN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //sub cam DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //sub and 2 main cam shared IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam0 AVDD2 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //main cam0 and cam1 shared DVDD2 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam0 and cam1 shared AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //MIPI Swith to main cam1
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //MCLK enable
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_0,/* clk for back main camera */
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //MCLK enable
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_2,/* clk for back sub camera */
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam0 RESET enable
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam1 RESET2 enable
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },
};

struct sensor_power_setting hw_imx486hybird_power_down_setting[] = {
    //main cam1 RESET2 disable
    {
        .seq_type = SENSOR_RST2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam0 RESET disable
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //MCLK disable
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_2,/* clk for back sub camera */
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },
    //MCLK disable
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_0,/* clk for back main camera */
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },
   
    //main cam0 and cam1 shared AFVDD 2.85V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V2P85V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam0 and cam1 shared DVDD2 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //main cam0 AVDD2 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //sub and 2 main cam shared IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //sub cam DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,/* meet sensor spec requirement dealy 1 ms */
    },

    //sub cam AVDD1_EN 1.8V
    {
        .seq_type = SENSOR_AVDD1_EN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //sub cam AVDD 2.8V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

    //disable sub cam reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_0,/* meet sensor spec requirement dealy 0 ms */
    },

};
atomic_t volatile imx486hybird_powered = ATOMIC_INIT(0);
struct mutex imx486hybird_power_lock;
sensor_t s_imx486hybird =
{
    .intf = { .vtbl = &s_imx486hybird_vtbl, },
    .power_setting_array = {
            .power_setting = hw_imx486hybird_power_up_setting,
            .size = ARRAY_SIZE(hw_imx486hybird_power_up_setting),
     },
    .power_down_setting_array = {
            .power_setting = hw_imx486hybird_power_down_setting,
            .size = ARRAY_SIZE(hw_imx486hybird_power_down_setting),
    },
     .p_atpowercnt = &imx486hybird_powered,
};

const struct of_device_id
s_imx486hybird_dt_match[] =
{
    {
        .compatible = "huawei,imx486hybird",
        .data = &s_imx486hybird.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_imx486hybird_dt_match);

struct platform_driver
s_imx486hybird_driver =
{
    .driver =
    {
        .name = "huawei,imx486hybird",
        .owner = THIS_MODULE,
        .of_match_table = s_imx486hybird_dt_match,
    },
};

char const* imx486hybird_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int imx486hybird_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    ret = hw_sensor_power_up_config(sensor->dev, sensor->board_info);
    if (0 == ret ) {
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
    if (0 == ret ) {
        cam_info("%s. power up sensor success.", __func__);
    }
    else {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

int imx486hybird_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -ENODEV;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret ) {
        cam_info("%s. power down sensor success.", __func__);
    }
    else {
        cam_err("%s. power down sensor fail.", __func__);
    }
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

int imx486hybird_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int imx486hybird_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

int imx486hybird_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    if (NULL == si) {
        cam_err("%s. si is NULL.", __func__);
        return -EINVAL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -ENODEV;
    }
    cdata = (struct sensor_cfg_data *)data;
    if (NULL == cdata) {
        cam_err("%s. cdata is NULL.", __func__);
        return -EINVAL;
    }
    memset(cdata->cfg.name, 0, DEVICE_NAME_SIZE);

    strncpy(cdata->cfg.name, sensor->board_info->name, DEVICE_NAME_SIZE - 1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

int
imx486hybird_config(
        hwsensor_intf_t* si,
        void  *argp)
{
    struct sensor_cfg_data *data = NULL;
    int ret =0;

    if ((NULL == si) || (NULL == si->vtbl) || (NULL == argp)){
        cam_err("%s : si or argp is null", __func__);
        return -EINVAL;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("imx486hybird cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            if (mutex_lock_interruptible(&imx486hybird_power_lock))
                return -ERESTARTSYS;
             if (!power_on_status){
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_up(si);
                    if (0 == ret) {
                        power_on_status = true;
                    } else {
                        cam_err("%s. power up fail.", __func__);
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&imx486hybird_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            if (mutex_lock_interruptible(&imx486hybird_power_lock))
                return -ERESTARTSYS;
            if (power_on_status){
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_down(si);
                    if (0 != ret) {
                        cam_err("%s. power_down fail.", __func__);
                    }
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&imx486hybird_power_lock);
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

int32_t
imx486hybird_platform_probe(
        struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;
    cam_notice("enter %s",__func__);

    if (NULL == pdev) {
        cam_err("%s pdev is NULL", __func__);
        return -EINVAL;
    }

    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_imx486hybird_dt_match, np);
    if (!id) {
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
    s_pdev = pdev;

    mutex_init(&imx486hybird_power_lock);
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (rc < 0) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
        cam_err("%s rpmsg_sensor_register failed rc %d\n", __func__, rc);
        return -ENODEV;
    }
    s_sensor = sensor;

    return rc;
}

int __init
imx486hybird_init_module(void)
{
    cam_notice("enter %s",__func__);
    return platform_driver_probe(&s_imx486hybird_driver,
            imx486hybird_platform_probe);
}

void __exit
imx486hybird_exit_module(void)
{
    if( NULL != s_sensor)
    {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_pdev) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
    }
    platform_driver_unregister(&s_imx486hybird_driver);
}
//lint -restore

/*lint -e528 -esym(528,*)*/
module_init(imx486hybird_init_module);
module_exit(imx486hybird_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("imx486hybird");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
