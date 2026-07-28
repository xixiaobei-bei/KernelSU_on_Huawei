/*
 * A driver for the camera fpga.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
//lint -save -e529 -e542 -e429 -e501 -e527 -e559 -e568 -e570 -e563 -e593 -e613 -e668 -e679 -e574
#include "ice40_spi.h"
#include "cam_log.h"


#define FPGA_VERSION_CHIP_ID_REG (0x00)
#define FPGA_VERSION_ID_REG      (0x01)
#define FPGA_IRQ_STATE_REG       (0x02)
#define FPGA_STATE_CTRL1_REG     (0x04)
#define FPGA_STATE_CTRL2_REG     (0x05)
#define FPGA_M1_PLUS_WIDE_REG    (0x0E)
#define FPGA_M2_PLUS_WIDE_REG    (0x0F)
#define FPGA_SELF_CHECK_REG      (0x10)

#define FPGA_CHIP_ID_VALUE       (0x5A)

#define REG_E_LIMIT_VAL          (5)
#define REG_F_LIMIT_VAL          (11)

#define LOAD_FW_ERR       (0)
#define SPI_RW_ERR        (1)
#define SELF_TEST_ERR     (2)
#define AGE_PROJ_ERR      (3)
#define AGE_ILLU_ERR      (4)
#define OTHER_FILL_ERR    (5)

#define ARRAY_SZ(A)   (sizeof(A)/sizeof(A[0]))

#define MAX_BUF_SIZE   (128)

#define FPGA_GPIO_CLK_CFG (9600000)

/*
 * only support ov9282 ov9286,
 * if add, please modify here:
 */
#define LOADFW_MAX_TIME   (2)
#define POWERUP_MAX_TIME  (2)
#define ENABLE_MAX_TIME   (1)

#define ICE40_DEVICE_NAME        "ice40_spi"
#define ICE40_MISC_DEVICE_NAME   "ice40"

#define MAX_LOAD_TIMES        (3)
#define MAX_BUFF_SIZE         (128)

extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);

#ifdef FPGA_READ_REG_DEBUG
static u8 REG = 0x00;
static u8 VAL = 0x00;
#endif

static struct ice40_spi_priv_data *spi_drv_data = NULL;

static struct mutex gpio_power_ctrl_lock;
static bool ice40_enable_gpio_powerdown = false;

static bool color_set_gpio185_powerup = false;

static int ice40_spi_read_reg(struct ice40_spi_priv_data *devdata, u8 addr, u8 *val);
static int ice40_spi_write_reg(struct ice40_spi_priv_data *devdata, u8 addr, u8 val);


extern struct dsm_client *client_camera_user;

/*
0x02-reg
bit7：M2-2异常
bit6：M2-1异常
bit5：Y异常
bit4：M1-2异常
bit3：M1-1异常
bit2：X异常
bit1：bit1=bit7 | bit6 | bit5 | bit4 | bit3 | bit2
bit0：0
*/
static struct err_map fpga_irq_err_table[] =
{   // 927011003 927011004 927011005 927011006 927011007 927011008
    {0x04, "Projector over current", DSM_FPGA_PROJ_OVER_CURRENT},                        /*投射器电流过大 bit2*/
    {0x20, "Illuminator over current", DSM_FPGA_ILLU_OVER_CURRENT},                      /*补光灯电流过大 bit5*/
    {0x08, "Projector pulse too long", DSM_FPGA_PROJ_PLUS_TOO_LONG},                      /*投射器电流脉冲宽度异常 bit3*/
    {0x40, "Illuminator pulse too long", DSM_FPGA_ILLU_PLUS_TOO_LONG},                    /*补光灯电流脉冲宽度异常 bit6*/
    {0x10, "Projector pulse frequency over limit", DSM_FPGA_PROJ_FREQU_OVER_LIMIT},          /*投射器电流脉冲周期异常 bit4*/
    {0x80, "Illuminator pulse frequency over limit", DSM_FPGA_ILLU_FREQU_OVER_LIMIT},        /*补光灯电流脉冲周期异常 bit7*/
};

static struct err_map fpga_common_err_table[] =
{   // 927011000 927011001 927011002 927011009 927011010 927011011
    [LOAD_FW_ERR]    = {0x01, "FPGA load image error", DSM_FPGA_LOAD_FW_FAIL},
    [SPI_RW_ERR]     = {0x01, "FPGA SPI error", DSM_FPGA_SPI_RW_FAIL},
    [SELF_TEST_ERR]  = {0x01, "FPGA Self Test error", DSM_FPGA_SELT_TEST_FAIL},
    [AGE_PROJ_ERR]   = {0x01, "Ageing projector pulse frequency over limit", DSM_FPGA_AGEING_PROJ_FREQU_LIMIT},   /*投射器老化脉宽检测异常 0x0E-reg*/
    [AGE_ILLU_ERR]   = {0x01, "Ageing illuminator pulse frequency over limit", DSM_FPGA_AGEING_ILLU_FREQU_LIMIT}, /*补光灯老化脉宽检测异常 0x0F-reg*/
    [OTHER_FILL_ERR] = {0x01, "FPGA other failed", DSM_FPGA_OTHER_FAIL},                             /*未定义异常*/
};
static void fpga_dsm_client_notify(struct ice40_spi_priv_data *drv_data,
    char *err_name, u32 err_code)
{
    if ((drv_data == NULL) || (err_name == NULL)) {
        cam_err("%s drvdata or err_name is NULL.", __func__);
        return;
    }

    if(!dsm_client_ocuppy(drv_data->client_ice40)){
        dsm_client_record(drv_data->client_ice40, "[ice40] %s.\n", err_name);
        dsm_client_record(drv_data->client_ice40, "[ice40] 0x02 = 0x%02x.\n", drv_data->last_err_code);
        dsm_client_notify(drv_data->client_ice40, err_code);
        cam_info("[I/DSM] fpga dsm_client_notify success.");
    }
    else {
        cam_info("[I/DSM] fpga dsm_client_notify fail.");
    }
}

static void fpga_load_firmware_notify(struct ice40_spi_priv_data *drv_data)
{
    char buff[MAX_BUFF_SIZE] = {0};

    if (drv_data == NULL) {
        cam_err("%s drvdata is NULL.", __func__);
        return;
    }

    if (drv_data->load_fw_fail_time > 0) {
        snprintf_s(buff, MAX_BUFF_SIZE, MAX_BUFF_SIZE - 1, "%s, fail times: %d.",
            fpga_common_err_table[LOAD_FW_ERR].err_name, drv_data->load_fw_fail_time);
        fpga_dsm_client_notify(drv_data, buff, fpga_common_err_table[LOAD_FW_ERR].err_num);
    }
    if (drv_data->self_check_fail_time > 0) {
        memset_s(buff, MAX_BUFF_SIZE, 0, MAX_BUFF_SIZE);
        snprintf_s(buff, MAX_BUFF_SIZE, MAX_BUFF_SIZE - 1, "%s, fail times: %d.",
            fpga_common_err_table[SELF_TEST_ERR].err_name, drv_data->self_check_fail_time);
        fpga_dsm_client_notify(drv_data, buff, fpga_common_err_table[SELF_TEST_ERR].err_num);
    }
}

/*
for reg: 0x0E 0x0F
读出的数值:
0x0E < 5ms  寄存器内容正常
0x0F < 11ms 寄存器内容正常
计算方法:
bit7-bit4 ms数；bit3-bit0 80us； PW=(bit7-bit4)+(bit3-bit0)*0.08 ms
*/
int fpga_plus_wide_value_check(struct ice40_spi_priv_data *drvdata,
               u8 addr, u8 *value, u8 limitVal)
{
    int ret = 0;
    u8 val = 0;
    int regValue = 0;

    if (drvdata == NULL) {
        cam_err("%s drvdata is NULL.", __func__);
        return -1;
    }
    ret = ice40_spi_read_reg(drvdata, addr, &val);
    if (ret < 0) {
        cam_err("%s read reg:0x%x fail.", __func__, addr);
        return -1;
    }
    cam_info("%s reg = 0x%x, val = 0x%x, limitVal = %d.", __func__, addr, val, limitVal);

    regValue = (val >> 4) * 100 + (val & 0x0F) * 8;
    if (regValue > (limitVal * 100)) {
        return 1;
    }

    return 0;
}

void fpga_enable_gpio_power_up(u8 val)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    struct ice40_spi_plat_data *plat_data = NULL;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return;
    }

    cam_info("%s val = %d, ice40_enable_gpio_powerdown = %d",
        __func__, val, ice40_enable_gpio_powerdown);

    plat_data = &drv_data->plat_data;
    if (plat_data == NULL) {
        cam_err("%s plat_data is NULL.", __func__);
        return;
    }
    if (val) {
        gpio_direction_output(plat_data->enable_gpio, 1);
        mutex_lock(&gpio_power_ctrl_lock);
        color_set_gpio185_powerup = true;
        mutex_unlock(&gpio_power_ctrl_lock);
    }
    else {
        mutex_lock(&gpio_power_ctrl_lock);
        color_set_gpio185_powerup = false;
        mutex_unlock(&gpio_power_ctrl_lock);
        if (ice40_enable_gpio_powerdown) {
            gpio_direction_output(plat_data->enable_gpio, 0);
        }
    }
}
EXPORT_SYMBOL(fpga_enable_gpio_power_up);

int ice40_spi_init(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    struct ice40_spi_plat_data *plat_data = NULL;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }

    cam_info("%s enter", __func__);

    drv_data->power_up_times++;
    drv_data->power_number++;
    if (drv_data->power_number > 1) {
        cam_info("%s ice40 already power up.", __func__);
        return 0;
    }

    plat_data = &drv_data->plat_data;
    /* enable gpio */
    gpio_direction_output(plat_data->enable_gpio, 1);
    /* spi cs gpio */
    gpio_direction_output(plat_data->spi_cs_gpio, 1);
    /* reset gpio */
    gpio_direction_output(plat_data->reset_gpio, 1);

    return 0;
}
EXPORT_SYMBOL(ice40_spi_init);

int ice40_spi_exit(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    struct ice40_spi_plat_data *plat_data = NULL;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }

    cam_info("%s enter", __func__);

    drv_data->power_number--;
    cam_info("%s drv_data->power_number = %d.", __func__, drv_data->power_number);
    if (drv_data->power_number > 0) {
        cam_info("%s drv_data->power_number = %d, will return.", __func__, drv_data->power_number);
        return 0;
    }

    plat_data = &drv_data->plat_data;
    /* reset gpio */
    gpio_direction_output(plat_data->reset_gpio, 0);
    /* spi cs gpio */
    gpio_direction_output(plat_data->spi_cs_gpio, 0);
    /* disable gpio */
    mutex_lock(&gpio_power_ctrl_lock);
    if (color_set_gpio185_powerup != true) {
        cam_info("%s color_set_gpio185_powerup = %d, make gpio185 pull down.",
            __func__, color_set_gpio185_powerup);
        gpio_direction_output(plat_data->enable_gpio, 0);
    }
    mutex_unlock(&gpio_power_ctrl_lock);

    cam_info("%s drv_data->power_up_times = %d.", __func__, drv_data->power_up_times);
    if (ice40_enable_gpio_powerdown != true) {
        if (drv_data->power_up_times >= POWERUP_MAX_TIME) {
            cam_info("%s drv_data->power_up_times = %d, will support power down.",
                __func__, drv_data->power_up_times);
            ice40_enable_gpio_powerdown = true;
        }
    }

    drv_data->power_number = 0;
    drv_data->load_fw_state = LOAD_FW_NOT_START;
    return 0;
}
EXPORT_SYMBOL(ice40_spi_exit);

int ice40_spi_enable(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    int ret = 0;
    u8 val = 0;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }

    cam_info("%s enter", __func__);

    if (drv_data->load_fw_state == LOAD_FW_FAIL) {
        cam_info("%s load fpga firmware fail, will exit.", __func__);
        return -1;
    }

    if (drv_data->load_fw_state != LOAD_FW_DONE) {
        cam_info("%s load_fw_state = %d, will exit.", __func__, drv_data->load_fw_state);
        return 0;
    }

    drv_data->enable_number++;
    if (drv_data->enable_number > ENABLE_MAX_TIME) {
        cam_info("%s drv_data->enable_number = %d, will return.",
            __func__, drv_data->enable_number);
        return 0;
    }

    val = 0x80;
    ret = ice40_spi_write_reg(drv_data, FPGA_STATE_CTRL1_REG, val);
    if (ret) {
        cam_err("%s ice40_spi_read_reg error, ret = %d", __func__, ret);
    }

    ret = clk_prepare_enable(drv_data->fpga_clkout);
    if (ret) {
        cam_err("%s cloud not prepare_enalbe fpga_clkout.", __func__);
        return ret;
    }

    /* firstly clear the irq register */
    ret = ice40_spi_read_reg(drv_data, FPGA_IRQ_STATE_REG, &val);
    if (ret) {
        cam_err("%s read reg = 0x%x error, ret = %d", __func__, FPGA_IRQ_STATE_REG, ret);
        return ret;
    }
    /* enable fpga irq handle */
    enable_irq(drv_data->spi->irq);

    return 0;
}
EXPORT_SYMBOL(ice40_spi_enable);

int ice40_spi_disable(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    struct ice40_spi_plat_data *plat_data = NULL;
    int ret = 0;
    u8 val = 0x00;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }

    cam_info("%s enter", __func__);

    plat_data = &drv_data->plat_data;
    if (drv_data->load_fw_state != LOAD_FW_DONE) {
        cam_info("%s load_fw_state = %d, will exit.", __func__, drv_data->load_fw_state);
        return 0;
    }

    drv_data->enable_number--;
    if (drv_data->enable_number > 0) {
        cam_info("%s drv_data->enable_number = %d, will return.",
            __func__, drv_data->enable_number);
        return 0;
    }

    if (fpga_plus_wide_value_check(drv_data, FPGA_M1_PLUS_WIDE_REG, &val, REG_E_LIMIT_VAL) > 0) {
        cam_err("%s write reg = 0x%x error, ret = %d", __func__, FPGA_M1_PLUS_WIDE_REG, ret);
        fpga_dsm_client_notify(drv_data, fpga_common_err_table[AGE_PROJ_ERR].err_name,
            fpga_common_err_table[AGE_PROJ_ERR].err_num);
    }
    if (fpga_plus_wide_value_check(drv_data, FPGA_M2_PLUS_WIDE_REG, &val, REG_F_LIMIT_VAL) > 0) {
        cam_err("%s write reg = 0x%x error, ret = %d", __func__, FPGA_M2_PLUS_WIDE_REG, ret);
        fpga_dsm_client_notify(drv_data, fpga_common_err_table[AGE_ILLU_ERR].err_name,
            fpga_common_err_table[AGE_ILLU_ERR].err_num);
    }

    /* shutdown fpga irq handle */
    disable_irq(drv_data->spi->irq);

    val = 0x00;
    ret = ice40_spi_write_reg(drv_data, FPGA_STATE_CTRL1_REG, val);
    if (ret) {
        cam_err("%s write reg = 0x%x error, ret = %d", __func__, FPGA_STATE_CTRL1_REG, ret);
    }

    clk_disable_unprepare(drv_data->fpga_clkout);

    return 0;
}
EXPORT_SYMBOL(ice40_spi_disable);

static void ice40_spi_close_disable(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    int ret = 0;
    u8 val = 0x00;

    cam_info("%s enter", __func__);

    /* shutdown fpga irq handle */
    disable_irq(drv_data->spi->irq);

    ret = ice40_spi_write_reg(drv_data, FPGA_STATE_CTRL1_REG, val);
    if (ret) {
        cam_err("%s write reg = 0x%x error, ret = %d", __func__, FPGA_STATE_CTRL1_REG, ret);
    }

    clk_disable_unprepare(drv_data->fpga_clkout);
}

int ice40_spi_init_fun(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;

    cam_info("%s enter", __func__);

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }
    drv_data->enable_number = 0;
    drv_data->power_number = 0;
    drv_data->power_up_times = 0;
    drv_data->load_fw_state = LOAD_FW_NOT_START;

    ice40_enable_gpio_powerdown = false;
    return 0;
}
EXPORT_SYMBOL(ice40_spi_init_fun);

int ice40_spi_close_fun(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;

    cam_info("%s enter, drv_data->enable_number = %d.",
        __func__, drv_data->enable_number);

    if (drv_data->enable_number > 0) {
        ice40_spi_close_disable();
    }
    ice40_spi_exit();
    return 0;
}
EXPORT_SYMBOL(ice40_spi_close_fun);

static int ice40_spi_read_reg(struct ice40_spi_priv_data *devdata, u8 addr, u8 *val)
{
    int status;
    struct spi_transfer xfer;
    struct spi_message  m;

    if ((devdata == NULL) || (val == NULL)) {
        cam_err("%s no driver data, or val is NULL.", __func__);
        return -ENODEV;
    }

    /* 1bit r/w + 5bit addr + 2bit dont care */
    /* 8bit dont care */
    memset_s(&(devdata->tx_buf[0]), 2, 0, 2);
    devdata->tx_buf[0] = (u8)((addr << 2) + 0x80);

    memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
    xfer.tx_buf = devdata->tx_buf;
    xfer.rx_buf = devdata->rx_buf;
    xfer.len = 2;

    spi_message_init(&m);
    spi_message_add_tail(&xfer, &m);
    status = spi_sync(devdata->spi, &m);
    if (status) {
        cam_err("%s - sync error: status=%d", __func__, status);
        return status;
    }

    cam_info("%s devdata->rx_buf[0] = 0x%x, devdata->rx_buf[1] = 0x%x.", __func__, devdata->rx_buf[0], devdata->rx_buf[1]);
    *val = devdata->rx_buf[1];

    return status;
}

static int ice40_spi_write_reg(struct ice40_spi_priv_data *devdata, u8 addr, u8 val)
{
    int status;
    struct spi_transfer xfer;
    struct spi_message  m;

    if (devdata == NULL) {
        cam_err("%s devdata is NULL.",__func__);
        return -EINVAL;
    }

    memset_s(&(devdata->tx_buf[0]), 2, 0, 2);
    devdata->tx_buf[0] = (addr << 2);
    devdata->tx_buf[1] = val;

    memset_s(&xfer, sizeof(xfer), 0, sizeof(xfer));
    xfer.tx_buf = devdata->tx_buf;
    xfer.rx_buf = NULL;
    xfer.len = 2;

    /* 1bit r/w + 5bit addr + 2bit dont care */
    /* 8bit data */
    spi_message_init(&m);
    spi_message_add_tail(&xfer, &m);
    status = spi_sync(devdata->spi, &m);
    if (status) {
        cam_err("%s - sync error: status=%d", __func__, status);
        return status;
    }

    return status;
}

/*
AP需要使能0x10自检寄存器的bit7，等待2ms后，读取0x10的bit6，
如果为1，表示FPGA正常工作
如果为0，表示FPGA逻辑没有工作
退出自检，关闭bit7
*/
static int ice40_spi_self_check(struct ice40_spi_priv_data *devdata)
{
    u8 val = 0x80;
    int ret = 0;
    int self_flag = -1; /*0:success -1:failed*/

    if (devdata == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }
    ret = ice40_spi_write_reg(devdata, FPGA_SELF_CHECK_REG, val);
    if (ret) {
        cam_err("[%s][%d] ice40_spi_read_reg error, ret = %d", __func__, __LINE__, ret);
        return ret;
    }
    mdelay(2);
    ret = ice40_spi_read_reg(devdata, FPGA_SELF_CHECK_REG, &val);
    if ((ret == 0) && (val & 0x40) != 0) {
        cam_info("%s fpga self-checking success.", __func__);
        self_flag = 0;
    }
    cam_info("%s reg = 0x%x, val = 0x%x", __func__, FPGA_SELF_CHECK_REG, val);

    val = 0x00;
    ret = ice40_spi_write_reg(devdata, FPGA_SELF_CHECK_REG, val);
    if (ret) {
        cam_err("[%s][%d] ice40_spi_read_reg error, ret = %d", __func__, __LINE__, ret);
    }

    return self_flag;
}

static int ice40_spi_load_fw_code(struct ice40_spi_priv_data *devdata, char *name)
{
    mm_segment_t oldfs = get_fs();
    struct file *fp = NULL;
    int file_size, one_size, left, block;
    loff_t pos;
    int clock_cycle_num = 1000;
    int ret = 0;
    u8 val1 = 0xFF;
    u8 val2 = 0xFF;

    if ((devdata == NULL) || (name == NULL)) {
        cam_err("%s no driver data, or name is NULL.", __func__);
        return -ENODEV;
    }

    set_fs(KERNEL_DS);
    fp = filp_open(ICE40_FPGA_FW_FILE, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        cam_info("%s open file error", __func__);
        set_fs(oldfs);
        return -EINVAL;
    }

    file_size =(int)(vfs_llseek(fp, 0L, SEEK_END));
    cam_info("start sending boot code... size=0x%x", file_size);

    vfs_llseek(fp, 0L, SEEK_SET);
    pos = fp->f_pos;
    for (left = file_size, block = 0; left > 0; left -= SPI_BLOCK_BUF_SIZE) {
        if (left > SPI_BLOCK_BUF_SIZE) {
            one_size = SPI_BLOCK_BUF_SIZE;
        }
        else {
            one_size = left;
            memset_s(devdata->tx_buf, SPI_BLOCK_BUF_SIZE,0, SPI_BLOCK_BUF_SIZE);
        }

        ret = vfs_read(fp, (char *)devdata->tx_buf, one_size, &pos);
        if (ret < 0) {
            cam_err("%s vfs read error %d", __func__, ret);
            break;
        }

        ret = spi_write(devdata->spi, devdata->tx_buf, SPI_BLOCK_BUF_SIZE);
        if (ret < 0) {
            cam_err("%s spi send error, ret = %d, %d bytes, test.", __func__, ret, one_size);
            break;
        }
        block++;
    }
    cam_info("%s succeed to send %d blocks", __func__, block);

    /* go on output clock signal for spi SCLK */
    memset_s(devdata->tx_buf, SPI_BLOCK_BUF_SIZE,0, SPI_BLOCK_BUF_SIZE);
    ret = spi_write(devdata->spi, devdata->tx_buf, clock_cycle_num);
    if (ret < 0) {
            cam_err("%s spi send error %d", __func__, ret);
    }

    ret = ice40_spi_read_reg(devdata, FPGA_VERSION_CHIP_ID_REG, &val1);
    if (ret) {
        cam_err("[%s][%d] ice40_spi_read_reg error, ret = %d", __func__, __LINE__, ret);
    }
    ret = ice40_spi_read_reg(devdata, FPGA_VERSION_ID_REG, &val2);
    if (ret) {
        cam_err("[%s][%d] ice40_spi_read_reg error, ret = %d", __func__, __LINE__, ret);
    }

    if (val1 != FPGA_CHIP_ID_VALUE) {
        cam_err("%s fpga get chip_id failed.", __func__);
        devdata->load_fw_fail_time++;
        ret = -1; /*load fw fail*/
    }
    if ((ret == 0) && (ice40_spi_self_check(devdata) != 0)) {
        cam_err("%s fpga self-checking failed.", __func__);
        devdata->self_check_fail_time++;
        ret = -1; /*self-check fail*/
    }

    filp_close(fp, 0);
    set_fs(oldfs);
    return ret;
}

int ice40_spi_load_fw(void)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    struct ice40_spi_plat_data *plat_data = NULL;
    char *fw_file_path = ICE40_FPGA_FW_FILE;
    int ret = 0;
    int load_time = 0;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return -ENODEV;
    }

    cam_info("%s enter", __func__);

    if (drv_data->load_fw_state != LOAD_FW_NOT_START) {
        cam_info("%s load_fw_state = %d, will exit.", __func__, drv_data->load_fw_state);
        return 0;
    }
    drv_data->load_fw_state = LOAD_FW_DOING;

    plat_data = &drv_data->plat_data;
    if (plat_data == NULL) {
        cam_err("%s plat_data is NULL.", __func__);
        drv_data->load_fw_state = LOAD_FW_FAIL;
        return -ENODEV;
    }

    drv_data->load_fw_fail_time = 0;
    drv_data->self_check_fail_time = 0;
    while (load_time < MAX_LOAD_TIMES) {
        cam_info("%s, load_time = %d", __func__, load_time);
        gpio_direction_output(plat_data->reset_gpio, 0);
        mdelay(1);
        gpio_direction_output(plat_data->reset_gpio, 1);
        mdelay(2);

        ret = ice40_spi_load_fw_code(drv_data, fw_file_path);
        if (ret == 0) {
            break;
        }
        load_time++;
    }
    if (drv_data->load_fw_fail_time != 0 || drv_data->self_check_fail_time != 0) {
        cam_info("%s drv_data->load_fw_fail_time = %d, drv_data->self_check_fail_time = %d.",
            __func__, drv_data->load_fw_fail_time, drv_data->self_check_fail_time);
        fpga_load_firmware_notify(drv_data);
    }

    if (ret) {
        cam_err("%s - load firmware failed.", __func__);
        drv_data->load_fw_state = LOAD_FW_FAIL;
        return -1;
    }

    drv_data->load_fw_state = LOAD_FW_DONE;
    return 0;
}
EXPORT_SYMBOL(ice40_spi_load_fw);

static void fpga_parse_dmd_error(struct work_struct *work)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    u8   err = spi_drv_data->last_err_code;
    int  index = 0;
    bool find = false;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return;
    }

    cam_info("%s err = 0x%x", __func__, err);
    for (index = 0; index < ARRAY_SZ(fpga_irq_err_table); index++) {
        if ((err & fpga_irq_err_table[index].err_head) != 0) {
            cam_info("%s: index = %d.", __func__, index);
            fpga_dsm_client_notify(drv_data, fpga_irq_err_table[index].err_name,
                fpga_irq_err_table[index].err_num);
            find = true;;
        }
    }

    if (find == false) {
        cam_info("%s: index = %d, find is false.", __func__, index);
        fpga_dsm_client_notify(drv_data, fpga_common_err_table[OTHER_FILL_ERR].err_name,
            fpga_common_err_table[OTHER_FILL_ERR].err_num);
    }
}

static irqreturn_t fpga_irq_thread(int irq, void *handle)
{
    struct ice40_spi_priv_data *drv_data = (struct ice40_spi_priv_data *)handle;
    u8 val = 0x00;
    int ret = 0;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return IRQ_HANDLED;
    }
    cam_info("%s enter...", __func__);

    if (drv_data->load_fw_state != LOAD_FW_DONE) {
        cam_info("%s load_fw_state = %d, will exit.", __func__, drv_data->load_fw_state);
        return IRQ_HANDLED;
    }

    ret =  ice40_spi_read_reg(drv_data, FPGA_IRQ_STATE_REG, &val);
    if(ret)
    {
        cam_err("%s ice40_spi_read_reg error, ret = %d", __func__, ret);
    }
    if (val == 0x00) {
        cam_info("%s reg = 0x02, val = 0x00, will return.", __func__);
        return IRQ_HANDLED;
    }
    cam_info("%s reg = 0x02, val = 0x%02x", __func__, val);
    drv_data->last_err_code = val;
    queue_work(drv_data->work_queue, &drv_data->dump_err_work);

    cam_info("%s exit...", __func__);
    return IRQ_HANDLED;
}

#ifdef FPGA_READ_REG_DEBUG
static int hw_fpga_param_check(char *buf, unsigned long *param,
int num_of_par)
{
    char *token = NULL;
    unsigned int base = 0;
    int cnt = 0;
    if ((NULL == buf) || (NULL == param)) {
        cam_err("%s buf or param is NULL", __func__);
        return -1;
    }

    token = strsep(&buf, " ");
    for (cnt = 0; cnt < num_of_par; cnt++) {
        if (token != NULL) {
            if ((token[1] == 'x') || (token[1] == 'X')) {//parse 0x*
                base = 16;  //hex
            } else {
                base = 10;  //decimal
            }
            if (strict_strtoul(token, base, &param[cnt]) != 0) {
                return -EINVAL;
            }
            token = strsep(&buf, " ");
        } else{
            return -EINVAL;
        }
    }
    return 0;
}

static ssize_t hw_fpga_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    int rc=0;
    u8 val = 0x00;
    int ret = 0;

    if ((drv_data == NULL) || (buf == NULL)) {
        cam_err("%s drv_data or buf is NULL", __func__);
        return -1;
    }

    ret =  ice40_spi_read_reg(drv_data, REG, &val);
    if (ret)
    {
        cam_err("%s ice40_spi_read_reg error, ret = %d", __func__, ret);
    }
    rc = scnprintf(buf, 0x10, "0x%x\n", val);

    return rc;
}

static ssize_t hw_fpga_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    unsigned long param[2] = {0};
    int rc = 0;

    if ((drv_data == NULL) || (buf == NULL)) {
        cam_err("%s drv_data or buf is NULL", __func__);
        return -1;
    }

    rc = hw_fpga_param_check((char *)buf, param, 2);
    if (rc < 0) {
        cam_err("%s failed to check param.", __func__);
        return rc;
    }

    REG = (u8)param[0];
    VAL = (u8)param[1];
    cam_info("%s REG = %d VAL = %d", __func__, REG, VAL);

    if (0xFF != VAL) {
        ice40_spi_write_reg(drv_data, REG, VAL);
    }
    else {
        cam_info("%s not allow write.", __func__);
    }

    return (ssize_t)count;
}

extern int register_camerafs_attr(struct device_attribute *attr);
static struct device_attribute dev_attr_fpga =
    __ATTR(fpgareg, 0664, hw_fpga_show, hw_fpga_store);
#endif

static void ice40_spi_cs_set(u32 control)
{
    int ret = 0;
    struct ice40_spi_priv_data *drv_data = spi_drv_data;
    struct ice40_spi_plat_data *plat_data = NULL;

    if (drv_data == NULL) {
        cam_err("%s no driver data.", __func__);
        return;
    }

    plat_data = &drv_data->plat_data;
    if (plat_data == NULL) {
        cam_err("%s plat_data is NULL.", __func__);
        return;
    }

    /*cam_info("%s control = %s.", __func__, (control==0)?"select":"diselect");*/ /*too many logs*/
    if (SSP_CHIP_SELECT == control) {
        ret = gpio_direction_output(plat_data->spi_cs_gpio, control);
        /* cs steup time at least 10ns */
        ndelay(10);
    } else {
        /* cs hold time at least 4*40ns(@25MHz) */
        ret = gpio_direction_output(plat_data->spi_cs_gpio, control);
        ndelay(200);
    }

    if (ret < 0) {
        cam_err("%s: fail to set gpio cs, result = %d.", __func__, ret);
    }
}

static int ice40_parse_gpio_config(struct ice40_spi_plat_data *plat_data,
                     struct device_node *of_node)
{
    unsigned int value;

    if ((plat_data == NULL) || (of_node == NULL)) {
        cam_err("%s plat_data or of_node is NULL.", __func__);
        return -EINVAL;
    }

    value = of_get_named_gpio(of_node, "ice40_spi,cs_gpio", 0);
    if (!gpio_is_valid(value)) {
        cam_err("%s get cs_gpio failed.", __func__);
        return -1;
    }
    plat_data->spi_cs_gpio = value;
    cam_info("%s cs_gpio = %d.", __func__, plat_data->spi_cs_gpio);

    value = of_get_named_gpio(of_node, "ice40_spi,rst_gpio", 0);
    if (!gpio_is_valid(value)) {
        cam_err("%s get rst_gpio failed.", __func__);
        return -1;
    }
    plat_data->reset_gpio = value;
    cam_info("%s reset_gpio = %d.", __func__, plat_data->reset_gpio);

    value = of_get_named_gpio(of_node, "ice40_spi,en_gpio", 0);
    if (!gpio_is_valid(value)) {
        cam_err("%s get en_gpio failed.", __func__);
        return -1;
    }
    plat_data->enable_gpio = value;
    cam_info("%s enable_gpio = %d.", __func__, plat_data->enable_gpio);

    value = of_get_named_gpio(of_node, "ice40_spi,irq_gpio", 0);
    if (!gpio_is_valid(value)) {
        cam_err("%s get irq_gpio failed.", __func__);
        return -1;
    }
    plat_data->irq_gpio = value;
    cam_info("%s irq_gpio = %d.", __func__, plat_data->irq_gpio);

    return 0;
}

int ice40_parse_spi_config(struct ice40_spi_plat_data *plat_data,
                     struct device_node *of_node)
{
    int rc = 0;
    unsigned int value;
    struct pl022_config_chip *pl022_spi_config = NULL;
    if (!plat_data || !of_node) {
        cam_err("%s plat_data or of_node is NULL.", __func__);
        return -EINVAL;
    }
    pl022_spi_config = (struct pl022_config_chip *)&plat_data->spidev0_chip_info;

    pl022_spi_config->hierarchy = 0;
    pl022_spi_config->cs_control = ice40_spi_cs_set;


    rc = of_property_read_u32(of_node, "pl022,interface", &value);
    if (!rc) {
        pl022_spi_config->iface = value;
        cam_info("%s iface configed %d.", __func__, pl022_spi_config->iface);
    }

    rc = of_property_read_u32(of_node, "pl022,com-mode", &value);
    if (!rc) {
        pl022_spi_config->com_mode = value;
        cam_info("%s com_mode configed %d.", __func__, pl022_spi_config->com_mode);
    }

    rc = of_property_read_u32(of_node, "pl022,rx-level-trig", &value);
    if (!rc) {
        pl022_spi_config->rx_lev_trig = value;
        cam_info("%s rx_lev_trig configed %d.", __func__, pl022_spi_config->rx_lev_trig);
    }

    rc = of_property_read_u32(of_node, "pl022,tx-level-trig", &value);
    if (!rc) {
        pl022_spi_config->tx_lev_trig = value;
        cam_info("%s tx_lev_trig configed %d.", __func__, pl022_spi_config->tx_lev_trig);
    }

    rc = of_property_read_u32(of_node, "pl022,ctrl-len", &value);
    if (!rc) {
        pl022_spi_config->ctrl_len = value;
        cam_info("%s ctrl_len configed %d.", __func__, pl022_spi_config->ctrl_len);
    }

    rc = of_property_read_u32(of_node, "pl022,wait-state", &value);
    if (!rc) {
        pl022_spi_config->wait_state = value;
        cam_info("%s wait_stateconfiged %d.", __func__, pl022_spi_config->wait_state);
    }

    rc = of_property_read_u32(of_node, "pl022,duplex", &value);
    if (!rc) {
        pl022_spi_config->duplex = value;
        cam_info("%s duplex %d.", __func__, pl022_spi_config->duplex);
    }

#if defined(CONFIG_ARCH_HI3630FPGA) || defined(CONFIG_ARCH_HI3630)
    rc = of_property_read_u32(of_node, "pl022,slave-tx-disable", &value);
    if (!rc) {
        pl022_spi_config->slave_tx_disable = value;
        cam_info("%s slave_tx_disable %d.", __func__, pl022_spi_config->slave_tx_disable);
    }
#endif

    return 0;
}

static int ice40_spi_get_dt_data(struct device *dev, struct ice40_spi_plat_data *plat_data)
{
    int ret = 0;
    unsigned int value;
    struct device_node *of_node = NULL;

    if (!dev || !dev->of_node || !plat_data) {
        cam_err("%s dev or of_node or plat_data is NULL.", __func__);
        return -EINVAL;
    }
    of_node = dev->of_node;

    ret = ice40_parse_gpio_config(plat_data, of_node);
    if (ret) {
        cam_err("%s parse gpio config failed.", __func__);
        return -1;
    }

    ret = ice40_parse_spi_config(plat_data, of_node);
    if (ret) {
        cam_err("%s parse spi config failed.", __func__);
        return -1;
    }

    ret = of_property_read_u32(of_node, "ice40_spi,chip_type", &value);
    if (ret < 0) {
        cam_err("%s get chip_type failed.", __func__);
        return -1;
    }
    plat_data->chip_type = value;
    cam_info("%s chip_type = %d.", __func__, plat_data->chip_type);

    ret = of_property_read_string(of_node, "clock-names", &plat_data->fpga_clkname);
    if (ret < 0) {
        cam_err("%s get clock-names failed.", __func__);
        return -1;
    }

    return ret;
}

static int ice40_spi_probe(struct spi_device *spi)
{
    struct ice40_spi_priv_data *drv_data = NULL;
    struct ice40_spi_plat_data *plat_data = NULL;
    int ret = 0;

    cam_info("%s enter", __func__);

    drv_data = kmalloc(sizeof(*drv_data), GFP_KERNEL);
    if (!drv_data) {
        cam_err("probe - can not alloc driver data");
        return -ENOMEM;
    }
    memset_s(drv_data, sizeof(*drv_data), 0, sizeof(*drv_data));

    plat_data = &drv_data->plat_data;
    ret = ice40_spi_get_dt_data(&spi->dev, plat_data);
    if (ret < 0) {
        cam_err("%s failed to ice40_spi_get_dt_data", __func__);
        goto err_no_pinctrl;
    }

    drv_data->tx_buf  = (void *)__get_free_pages(GFP_KERNEL|GFP_DMA, get_order(2*PAGE_SIZE));
    drv_data->rx_buf  = (void *)__get_free_pages(GFP_KERNEL|GFP_DMA, get_order(2*PAGE_SIZE));
    drv_data->irq_buf = (void *)__get_free_pages(GFP_KERNEL|GFP_DMA, get_order(PAGE_SIZE));
    cam_info("%s tx_buf=0x%pK rx_buf=0x%pK irq_buf=0x%pK",
        __func__,
        drv_data->tx_buf, drv_data->rx_buf, drv_data->irq_buf);
    if ((!drv_data->tx_buf) || (!drv_data->rx_buf) ||
        (!drv_data->irq_buf)) {
        cam_err("%s can not alloc dma buf page", __func__);
        ret = -ENOMEM;
        goto err_alloc_buf;
    }

    drv_data->spi = spi;
    mutex_init(&gpio_power_ctrl_lock);
    mutex_init(&drv_data->busy_lock);
    INIT_WORK(&drv_data->dump_err_work, fpga_parse_dmd_error);
    drv_data->work_queue = create_singlethread_workqueue(dev_name(&spi->dev));
    if (!drv_data->work_queue) {
        cam_err("probe - create workqueue error");
        ret = -EBUSY;
        goto err_create_queue;
    }

    spi->mode = SPI_MODE_0;
    spi->max_speed_hz = ICE40_SPI_SPEED_10M;
    spi->bits_per_word = 8;
    spi->controller_data = &plat_data->spidev0_chip_info;
    ret = spi_setup(spi);
    if (ret<0) {
        cam_err("probe - setup spi error");
        goto err_spi_setup;
    }

    drv_data->fpga_clkout = devm_clk_get(&spi->dev, plat_data->fpga_clkname);
    if (IS_ERR_OR_NULL(drv_data->fpga_clkout)) {
        cam_err("[%s] Failed: phyclk.%s.%li\n", __func__, plat_data->fpga_clkname,
            PTR_ERR(drv_data->fpga_clkout));
        goto err_spi_setup;
    }
    ret = clk_set_rate(drv_data->fpga_clkout, FPGA_GPIO_CLK_CFG);
    if(ret < 0) {
        cam_err("%s failed set_rate fpga_clkout rate.\n", __func__);
        goto err_spi_setup;
    }

    plat_data->pinctrl = devm_pinctrl_get_select(&spi->dev,
        PINCTRL_STATE_DEFAULT);
    if (NULL == plat_data->pinctrl) {
        cam_err("%s failed to set pin.", __func__);
        goto err_dev_attr;
    }

    ret = gpio_request(plat_data->spi_cs_gpio, "ice40_spi_cs");
    if (ret) {
        cam_err("probe - request spi cs gpio error");
        goto err_cs_gpio;
    }
    gpio_direction_output(plat_data->spi_cs_gpio, 1);

    ret = gpio_request(plat_data->reset_gpio, "ice40_spi_reset");
    if (ret) {
        cam_err("probe - request reset gpio error");
        goto err_reset_gpio;
    }
    gpio_direction_output(plat_data->reset_gpio, 0);

#ifdef FPGA_READ_REG_DEBUG
    ret = register_camerafs_attr(&dev_attr_fpga);
    if (ret < 0) {
        cam_err("%s register_camerafs_attr failed line %d\n", __func__, __LINE__);
    }
#endif

    drv_data->client_ice40 = client_camera_user;

    /* set driver_data to device */
    spi_set_drvdata(spi, drv_data);
    spi_drv_data = drv_data;

    drv_data->enable_number = 0;
    drv_data->power_number = 0;
    drv_data->power_up_times = 0;
    drv_data->load_fw_state = LOAD_FW_NOT_START;

    ret = gpio_request(plat_data->irq_gpio, "ice40_spi_irq");
    if (ret) {
        cam_err("probe - request irq gpio error");
        goto err_irq_gpio;
    }
    gpio_direction_input(plat_data->irq_gpio);

    spi->irq = gpio_to_irq(plat_data->irq_gpio);
    ret = request_threaded_irq(spi->irq, NULL, fpga_irq_thread,
                            IRQF_ONESHOT | IRQF_TRIGGER_RISING,
                            "ice40_isp", drv_data);
    if (ret) {
        cam_err("probe - request irq error(%d)", spi->irq);
        goto err_irq_config;
    }
    disable_irq(spi->irq);

    ret = gpio_request(plat_data->enable_gpio, "ice40_spi_enable");
    if (ret) {
        cam_err("probe - request enable gpio error");
        goto err_enable_gpio;
    }

    cam_info("ice40 spi probe success");
    return ret;

err_enable_gpio:
    free_irq(spi->irq, drv_data);
err_irq_config:
    gpio_free(plat_data->irq_gpio);
err_irq_gpio:
    gpio_free(plat_data->reset_gpio);
err_reset_gpio:
    gpio_free(plat_data->spi_cs_gpio);
err_cs_gpio:
err_dev_attr:
err_spi_setup:
    destroy_workqueue(drv_data->work_queue);
err_create_queue:
    free_pages((unsigned long)drv_data->tx_buf,  get_order(2*PAGE_SIZE));
    free_pages((unsigned long)drv_data->rx_buf,  get_order(2*PAGE_SIZE));
    free_pages((unsigned long)drv_data->irq_buf, get_order(PAGE_SIZE));
err_alloc_buf:
err_no_pinctrl:
    kfree(drv_data);
    spi_drv_data = NULL;
    return ret;
}

static int ice40_spi_remove(struct spi_device *sdev)
{
    return 0;
}

static const struct of_device_id ice40_spi_dt_ids[] = {
    { .compatible = "ice40,spi" },
    {},
};

static const struct spi_device_id g_ice40_device_id[] = {
    { ICE40_DEVICE_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(spi, g_ice40_device_id);

static struct spi_driver ice40_spi_drv = {
    .probe = ice40_spi_probe,
    .remove = ice40_spi_remove,
    .id_table = g_ice40_device_id,
    .driver = {
        .name =   ICE40_DEVICE_NAME,
        .owner =  THIS_MODULE,
        .bus =    &spi_bus_type,
        .of_match_table = ice40_spi_dt_ids,
    },
};

module_spi_driver(ice40_spi_drv);

MODULE_LICENSE("Huawei/GPL");

