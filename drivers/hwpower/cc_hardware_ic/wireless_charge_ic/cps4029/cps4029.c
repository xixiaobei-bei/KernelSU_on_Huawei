/*
 * cps4029.c
 *
 * cps4029 driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "cps4029.h"

#define HWLOG_TAG wireless_cps4029
HWLOG_REGIST();

static int cps4029_i2c_read(struct cps4029_dev_info *di,
	u8 *cmd, int cmd_len, u8 *dat, int dat_len)
{
	int i;
	int ret;

	if (!di || !cmd || !dat) {
		hwlog_err("i2c_read: para null\n");
		return -EINVAL;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = power_i2c_read_block(di->client, cmd, cmd_len, dat, dat_len);
		if (!ret)
			return 0;
		power_usleep(DT_USLEEP_10MS); /* delay for i2c retry */
	}

	return -EINVAL;
}

static int cps4029_i2c_write(struct cps4029_dev_info *di, u8 *cmd, int cmd_len)
{
	int i;
	int ret;

	if (!di || !cmd) {
		hwlog_err("i2c_write: para null\n");
		return -EINVAL;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = power_i2c_write_block(di->client, cmd, cmd_len);
		if (!ret)
			return 0;
		power_usleep(DT_USLEEP_10MS); /* delay for i2c retry */
	}

	return -EINVAL;
}

int cps4029_read_block(struct cps4029_dev_info *di, u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 cmd[CPS4029_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("read_block: para null\n");
		return -EINVAL;
	}

	di->client->addr = CPS4029_SW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	ret = cps4029_i2c_read(di, cmd, CPS4029_ADDR_LEN, data, len);
	if (ret) {
		di->g_val.sram_i2c_ready = false;
		return ret;
	}

	return 0;
}

int cps4029_write_block(struct cps4029_dev_info *di, u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !di->client || !data) {
		hwlog_err("write_block: para null\n");
		return -EINVAL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS4029_ADDR_LEN + len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	di->client->addr = CPS4029_SW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[CPS4029_ADDR_LEN], data, len);

	ret = cps4029_i2c_write(di, cmd, CPS4029_ADDR_LEN + len);
	if (ret) {
		di->g_val.sram_i2c_ready = false;
		kfree(cmd);
		return ret;
	}

	kfree(cmd);
	return 0;
}

int cps4029_read_byte(struct cps4029_dev_info *di, u16 reg, u8 *data)
{
	return cps4029_read_block(di, reg, data, BYTE_LEN);
}

int cps4029_read_word(struct cps4029_dev_info *di, u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = { 0 };

	ret = cps4029_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -EINVAL;

	*data = buff[0] | (buff[1] << BITS_PER_BYTE);

	return 0;
}

int cps4029_write_byte(struct cps4029_dev_info *di, u16 reg, u8 data)
{
	return cps4029_write_block(di, reg, &data, BYTE_LEN);
}

int cps4029_write_word(struct cps4029_dev_info *di, u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return cps4029_write_block(di, reg, buff, WORD_LEN);
}

int cps4029_write_byte_mask(struct cps4029_dev_info *di, u16 reg, u8 mask, u8 shift, u8 data)
{
	int ret;
	u8 val = 0;

	ret = cps4029_read_byte(di, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps4029_write_byte(di, reg, val);
}

int cps4029_write_word_mask(struct cps4029_dev_info *di, u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = cps4029_read_word(di, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps4029_write_word(di, reg, val);
}

static int cps4029_aux_write_block(struct cps4029_dev_info *di, u16 reg, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data) {
		hwlog_err("write_block: para null\n");
		return -EINVAL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS4029_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	di->client->addr = CPS4029_HW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[CPS4029_ADDR_LEN], data, data_len);

	ret = cps4029_i2c_write(di, cmd, CPS4029_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int cps4029_aux_write_word(struct cps4029_dev_info *di, u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return cps4029_aux_write_block(di, reg, buff, WORD_LEN);
}

int cps4029_get_chip_id(struct cps4029_dev_info *di, u16 *chip_id)
{
	int ret;

	ret = cps4029_read_word(di, CPS4029_CHIP_ID_ADDR, chip_id);
	hwlog_info(" chipid =0x%x ret = %d\n", *chip_id, ret);
	if (ret)
		return ret;
	*chip_id = CPS4029_CHIP_ID;

	return 0;
}

static int cps4029_get_mtp_version(struct cps4029_dev_info *di, u16 *mtp_ver)
{
	return cps4029_read_word(di, CPS4029_MTP_VER_ADDR, mtp_ver);
}

int cps4029_get_chip_info(struct cps4029_dev_info *di, struct cps4029_chip_info *info)
{
	int ret;

	if (!info || !di)
		return -EINVAL;

	ret = cps4029_get_chip_id(di, &info->chip_id);
	ret += cps4029_get_mtp_version(di, &info->mtp_ver);
	if (ret) {
		hwlog_err("get_chip_info: failed\n");
		return ret;
	}

	return 0;
}

int cps4029_get_chip_info_str(struct cps4029_dev_info *di, char *info_str, int len)
{
	int ret;
	struct cps4029_chip_info chip_info = {0};

	if (!info_str || (len != WL_CHIP_INFO_STR_LEN))
		return -EINVAL;

	ret = cps4029_get_chip_info(di, &chip_info);
	if (ret)
		return ret;

	memset(info_str, 0, CPS4029_CHIP_INFO_STR_LEN);

	snprintf(info_str, len, "chip_id:0x%04x mtp_ver:0x%04x",
		chip_info.chip_id, chip_info.mtp_ver);

	return 0;
}

int cps4029_get_mode(struct cps4029_dev_info *di, u8 *mode)
{
	return cps4029_read_byte(di, CPS4029_OP_MODE_ADDR, mode);
}

static void cps4029_wake_lock(struct cps4029_dev_info *di)
{
	if (di->wakelock.active) {
		hwlog_info("[wake_lock] already locked\n");
		return;
	}

	__pm_stay_awake(&di->wakelock);
	hwlog_info("wake_lock\n");
}

static void cps4029_wake_unlock(struct cps4029_dev_info *di)
{
	if (!di->wakelock.active) {
		hwlog_info("[wake_unlock] already unlocked\n");
		return;
	}

	__pm_relax(&di->wakelock);
	hwlog_info("wake_unlock\n");
}

void cps4029_enable_irq(struct cps4029_dev_info *di)
{
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[enable_irq] ++\n");
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[enable_irq] --\n");
	mutex_unlock(&di->mutex_irq);
}

void cps4029_disable_irq_nosync(struct cps4029_dev_info *di)
{
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[disable_irq_nosync] ++\n");
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[disable_irq_nosync] --\n");
	mutex_unlock(&di->mutex_irq);
}

void cps4029_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct cps4029_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[chip_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

static void cps4029_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u8 mode = 0;
	u16 chip_id = 0;
	struct cps4029_dev_info *di =
		container_of(work, struct cps4029_dev_info, irq_work);

	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[irq_work] gpio %s\n", gpio_val ? "high" : "low");
		goto exit;
	}
	/* init i2c */
	ret = cps4029_read_word(di, CPS4029_CHIP_ID_ADDR, &chip_id);
	if (!di->g_val.sram_i2c_ready || (chip_id != CPS4029_CHIP_ID)) {
		ret = cps4029_fw_sram_i2c_init(di, CPS4029_BYTE_INC);
		if (ret) {
			hwlog_err("irq_work: i2c init failed\n");
			goto exit;
		}
	}
	/* get System Operating Mode */
	ret = cps4029_get_mode(di, &mode);
	if (!ret)
		hwlog_info("[irq_work] get mode fail. mode=0x%x\n", mode);

	/* handler irq */
	if ((mode == CPS4029_OP_MODE_TX) || (mode == CPS4029_OP_MODE_BP))
		cps4029_tx_mode_irq_handler(di);
	else
		hwlog_info("[irq_work] mode=0x%x\n", mode);

exit:
	if (di) {
		cps4029_enable_irq(di);
		cps4029_wake_unlock(di);
	}
}

static irqreturn_t cps4029_interrupt(int irq, void *_di)
{
	struct cps4029_dev_info *di = _di;

	if (!di) {
		hwlog_err("interrupt: di null\n");
		return IRQ_HANDLED;
	}

	cps4029_wake_lock(di);
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("[interrupt] irq is not enable\n");
		cps4029_wake_unlock(di);
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int cps4029_dev_check(struct cps4029_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	power_usleep(DT_USLEEP_10MS); /* delay for power on */
	ret = cps4029_get_chip_id(di, &chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);

	hwlog_info("[dev_check] chip_id=0x%04x\n", chip_id);
	if (chip_id != CPS4029_CHIP_ID)
		hwlog_err("dev_check: rx_chip not match\n");

	return 0;
}

struct device_node *cps4029_dts_dev_node(void *dev_data)
{
	struct cps4029_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int cps4029_gpio_init(struct cps4029_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_output(np, "gpio_en", "cps4029_en",
		&di->gpio_en, di->gpio_en_valid_val))
		return -EINVAL;

	return 0;
}

static int cps4029_irq_init(struct cps4029_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "cps4029_int",
		&di->gpio_int, &di->irq_int))
		return -EINVAL;

	if (request_irq(di->irq_int, cps4029_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "cps4029_irq", di)) {
		hwlog_err("irq_init: request cps4029_irq failed\n");
		gpio_free(di->gpio_int);
		return -EINVAL;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, cps4029_irq_work);

	return 0;
}

static void cps4029_register_pwr_dev_info(struct cps4029_dev_info *di)
{
	int ret;
	u16 chip_id = 0;
	struct power_devices_info_data *pwr_dev_info = NULL;

	ret = cps4029_get_chip_id(di, &chip_id);
	if (ret)
		return;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static void cps4029_ops_unregister(struct wltrx_ic_ops *ops)
{
	if (!ops)
		return;

	kfree(ops->tx_ops);
	kfree(ops->fw_ops);
	kfree(ops->qi_ops);
	kfree(ops);
}

static int cps4029_ops_register(struct wltrx_ic_ops *ops, struct cps4029_dev_info *di)
{
	int ret;

	ret = cps4029_fw_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}

	ret = cps4029_tx_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}

	ret = cps4029_qi_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	return 0;
}

static void cps4029_fw_mtp_check(struct cps4029_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode()) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->mtp_check_work, cps4029_fw_mtp_check_work);
	schedule_delayed_work(&di->mtp_check_work,
		msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static int cps4029_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct cps4029_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct wltrx_ic_ops *ops = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	ops = kzalloc(sizeof(*ops), GFP_KERNEL);
	if (!ops) {
		devm_kfree(&client->dev, di);
		return -ENOMEM;
	}

	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->ic_type = id->driver_data;
	i2c_set_clientdata(client, di);

	ret = cps4029_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = cps4029_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = cps4029_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	ret = cps4029_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&di->wakelock, "cps4029_wakelock");
	mutex_init(&di->mutex_irq);
	ret = cps4029_ops_register(ops, di);
	if (ret)
		goto ops_regist_fail;

	cps4029_fw_mtp_check(di);
	cps4029_register_pwr_dev_info(di);

	hwlog_info("[ic_type:%u]wireless_chip probe ok\n", di->ic_type);
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
irq_init_fail:
	gpio_free(di->gpio_en);
gpio_init_fail:
parse_dts_fail:
dev_ck_fail:
	cps4029_ops_unregister(ops);
	devm_kfree(&client->dev, di);
	return ret;
}

static void cps4029_shutdown(struct i2c_client *client)
{
	hwlog_info("[shutdown]\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_cps4029);
static const struct i2c_device_id cps4029_i2c_id[] = {
	{ "cps4029", WLTRX_IC_MAIN },
	{ "cps4029_aux", WLTRX_IC_AUX },
	{}
};

static struct i2c_driver cps4029_driver = {
	.probe = cps4029_probe,
	.shutdown = cps4029_shutdown,
	.id_table = cps4029_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_cps4029",
	},
};

static int __init cps4029_init(void)
{
	return i2c_add_driver(&cps4029_driver);
}

static void __exit cps4029_exit(void)
{
	i2c_del_driver(&cps4029_driver);
}

device_initcall(cps4029_init);
module_exit(cps4029_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cps4029 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
